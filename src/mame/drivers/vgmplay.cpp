// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// A "virtual" driver to play vgm files
// Use with mame vgmplay -quik file.vgm

#include "emu.h"

#define QSOUND_LLE

#include "imagedev/snapquik.h"

#include "cpu/h6280/h6280.h"
#include "cpu/m6502/n2a03.h"
#include "sound/2203intf.h"
#include "sound/2608intf.h"
#include "sound/2610intf.h"
#include "sound/2612intf.h"
#include "sound/262intf.h"
#include "sound/3526intf.h"
#include "sound/3812intf.h"
#include "sound/ay8910.h"
#include "sound/8950intf.h"
#include "sound/c352.h"
#include "sound/c6280.h"
#include "sound/gb.h"
#include "sound/iremga20.h"
#include "sound/k051649.h"
#include "sound/k053260.h"
#include "sound/k054539.h"
#include "sound/multipcm.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "sound/qsound.h"
#include "sound/rf5c68.h"
#include "sound/segapcm.h"
#include "sound/sn76496.h"
#include "sound/x1_010.h"
#include "sound/ym2151.h"
#include "sound/ym2413.h"
#include "sound/ymf271.h"
#include "sound/ymf278b.h"
#include "sound/ymz280b.h"

#include "vgmplay.lh"
#include "debugger.h"
#include "speaker.h"

#include <zlib.h>

#include <list>
#include <memory>
#include <utility>
#include <vector>

#define AS_IO16             1
#define MCFG_CPU_IO16_MAP   MCFG_DEVICE_DATA_MAP

class vgmplay_disassembler : public util::disasm_interface
{
public:
	vgmplay_disassembler() = default;
	virtual ~vgmplay_disassembler() = default;

	virtual uint32_t opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

class vgmplay_device : public cpu_device
{
public:
	enum io8_t
	{
		REG_SIZE     = 0x00000000,
		A_YM2612_0   = 0x00000010,
		A_YM2612_1   = 0x00000018,
		A_YM2151_0   = 0x00000020,
		A_YM2151_1   = 0x00000028,
		A_YM2413_0   = 0x00000030,
		A_YM2413_1   = 0x00000038,
		A_YM2203_0   = 0x00000040,
		A_YM2203_1   = 0x00000050,
		A_YM3526_0   = 0x00000060,
		A_YM3526_1   = 0x00000068,
		A_YM3812_0   = 0x00000070,
		A_YM3812_1   = 0x00000078,
		A_AY8910A    = 0x00000080,
		A_AY8910B    = 0x00000090,
		A_SN76496_0  = 0x000000a0,
		A_SN76496_1  = 0x000000a8,
		A_K053260    = 0x000000b0,
		A_C6280      = 0x000000e0,
		A_OKIM6295A  = 0x000000f0,
		A_OKIM6295B  = 0x00000110,
		A_SEGAPCM    = 0x00001000,
		A_GAMEBOY    = 0x00002000,
		A_NESAPU     = 0x00002030,
		A_NESRAM     = 0x00003000,
		A_MULTIPCMA  = 0x00013000,
		A_MULTIPCMB  = 0x00013010,
		A_POKEYA     = 0x00013020,
		A_POKEYB     = 0x00013030,
		A_YM2608_0   = 0x00013060,
		A_YM2608_1   = 0x00013068,
		A_YM2610_0   = 0x00013070,
		A_YM2610_1   = 0x00013078,
		A_Y8950_0    = 0x00013080,
		A_Y8950_1    = 0x00013088,
		A_YMF262_0   = 0x00013090,
		A_YMF262_1   = 0x00013098,
		A_YMF278B_0  = 0x000130a0,
		A_YMF278B_1  = 0x000130b0,
		A_YMF271_0   = 0x000130c0,
		A_YMF271_1   = 0x000130d0,
		A_YMZ280B_0  = 0x000130e0,
		A_YMZ280B_1  = 0x000130f0,
		A_K054539A   = 0x00013400,
		A_K054539B   = 0x00013800,
		A_QSOUND     = 0x00013c00,
		A_K051649    = 0x00013c10,
		A_GA20       = 0x00013c20,
		A_RF5C68     = 0x00013c40,
		A_RF5C164    = 0x00013c50,
		A_RF5C68RAM  = 0x00014000,
		A_RF5C164RAM = 0x00024000,
		A_X1_010     = 0x00034000
	};

	enum io16_t
	{
		A_C352       = 0x00000000
	};

	vgmplay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t execute_min_cycles() const override;
	virtual uint32_t execute_max_cycles() const override;
	virtual uint32_t execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	virtual space_config_vector memory_space_config() const override;

	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	template<int Chip> DECLARE_READ8_MEMBER(segapcm_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(ymf278b_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(ymf271_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(ymz280b_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(multipcm_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(k053260_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(okim6295_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(k054539_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(c352_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(qsound_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(ga20_rom_r);
	template<int Chip> DECLARE_READ8_MEMBER(x1_010_rom_r);

	template<int Chip> DECLARE_WRITE8_MEMBER(multipcm_bank_hi_w);
	template<int Chip> DECLARE_WRITE8_MEMBER(multipcm_bank_lo_w);

	template<int Chip> DECLARE_WRITE8_MEMBER(okim6295_nmk112_enable_w);
	template<int Chip> DECLARE_WRITE8_MEMBER(okim6295_bank_w);
	template<int Chip> DECLARE_WRITE8_MEMBER(okim6295_nmk112_bank_w);

	void stop();
	void pause();
	bool paused() const { return m_paused; }
	void play();
	void toggle_loop() { m_loop = !m_loop; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum { ACT_LED_PERSIST_MS = 100 };

	enum act_led {
		LED_AY8910 = 0,

		LED_SN76496,

		LED_OKIM6295,

		LED_YM2151,
		LED_YM2203,
		LED_YM2413,
		LED_YM2608,
		LED_YM2612,
		LED_YM3526,
		LED_YM3812,
		LED_YMF271,
		LED_YMZ280B,

		LED_MULTIPCM,
		LED_SEGAPCM,

		LED_QSOUND,

		LED_POKEY,

		LED_NESAPU,
		LED_GAMEBOY,

		LED_C352,

		LED_C6280,

		LED_K051649,
		LED_K053260,
		LED_K054539,

		LED_GA20,

		LED_RF5C68,
		LED_RF5C164,

		LED_X1_010,
		LED_YM2610,
		LED_Y8950,
		LED_YMF262,
		LED_YMF278B,

		LED_COUNT
	};

	enum { RESET, RUN, DONE };

	using led_expiry = std::pair<act_led, attotime>;
	using led_expiry_list = std::list<led_expiry>;
	using led_expiry_iterator = led_expiry_list::iterator;

	struct rom_block {
		offs_t start_address;
		offs_t end_address;
		std::unique_ptr<uint8_t[]> data;

		rom_block(rom_block &&) = default;
		rom_block(offs_t start, offs_t end, std::unique_ptr<uint8_t[]> &&d) : start_address(start), end_address(end), data(std::move(d)) {}
	};

	void pulse_act_led(act_led led);
	TIMER_CALLBACK_MEMBER(act_led_expired);

	uint8_t rom_r(int chip, uint8_t type, offs_t offset);
	uint32_t handle_data_block(uint32_t address);
	uint32_t handle_pcm_write(uint32_t address);
	void blocks_clear();

	output_finder<LED_COUNT> m_act_leds;
	led_expiry_list m_act_led_expiries;
	std::unique_ptr<led_expiry_iterator []> m_act_led_index;
	led_expiry_iterator m_act_led_off;
	emu_timer *m_act_led_timer = nullptr;

	address_space_config m_file_config, m_io_config, m_io16_config;
	address_space *m_file = nullptr, *m_io = nullptr, *m_io16 = nullptr;

	int m_icount = 0;
	int m_state = RESET;
	bool m_paused = false;
	bool m_loop = false;

	uint32_t m_pc = 0U;

	std::list<rom_block> m_rom_blocks[2][0x40];

	std::vector<uint8_t> m_data_streams[0x40];
	std::vector<uint32_t> m_data_stream_starts[0x40];

	uint32_t m_ym2612_stream_offset = 0U;

	uint32_t m_multipcm_bank_l[2];
	uint32_t m_multipcm_bank_r[2];
	uint32_t m_multipcm_banked[2];

	uint32_t m_okim6295_nmk112_enable[2];
	uint32_t m_okim6295_bank[2];
	uint32_t m_okim6295_nmk112_bank[2][4];
};

DEFINE_DEVICE_TYPE(VGMPLAY, vgmplay_device, "vgmplay_core", "VGM Player engine")

enum vgmplay_inputs : uint8_t
{
	VGMPLAY_STOP,
	VGMPLAY_PAUSE,
	VGMPLAY_PLAY,
	VGMPLAY_RESTART,
	VGMPLAY_LOOP,
};

class vgmplay_state : public driver_device
{
public:
	vgmplay_state(const machine_config &mconfig, device_type type, const char *tag);

	DECLARE_QUICKLOAD_LOAD_MEMBER(load_file);

	DECLARE_READ8_MEMBER(file_r);
	DECLARE_READ8_MEMBER(file_size_r);
	DECLARE_INPUT_CHANGED_MEMBER(key_pressed);

	template<int Chip> DECLARE_WRITE8_MEMBER(okim6295_clock_w);
	template<int Chip> DECLARE_WRITE8_MEMBER(okim6295_pin7_w);
	DECLARE_WRITE8_MEMBER(scc_w);

	void vgmplay(machine_config &config);
	void file_map(address_map &map);
	void soundchips16_map(address_map &map);
	void soundchips_map(address_map &map);
	template<int Chip> void segapcm_map(address_map &map);
	template<int Chip> void ymf278b_map(address_map &map);
	template<int Chip> void ymf271_map(address_map &map);
	template<int Chip> void ymz280b_map(address_map &map);
	template<int Chip> void c352_map(address_map &map);
	template<int Chip> void ga20_map(address_map &map);
	template<int Chip> void h6280_io_map(address_map &map);
	template<int Chip> void h6280_map(address_map &map);
	template<int Chip> void k053260_map(address_map &map);
	template<int Chip> void k054539_map(address_map &map);
	template<int Chip> void multipcm_map(address_map &map);
	template<int Chip> void nescpu_map(address_map &map);
	template<int Chip> void okim6295_map(address_map &map);
	template<int Chip> void qsound_map(address_map &map);
	template<int Chip> void rf5c68_map(address_map &map);
	template<int Chip> void rf5c164_map(address_map &map);
	template<int Chip> void x1_010_map(address_map &map);

private:
	std::vector<uint8_t> m_file_data;
	required_device<vgmplay_device> m_vgmplay;
	required_device<speaker_device> m_lspeaker;
	required_device<speaker_device> m_rspeaker;
	required_device_array<sn76496_device, 2> m_sn76496;
	required_device_array<ym2413_device, 2> m_ym2413;
	required_device_array<ym2612_device, 2> m_ym2612;
	required_device_array<ym2151_device, 2> m_ym2151;
	required_device<segapcm_device> m_segapcm;
	required_device<rf5c68_device> m_rf5c68;
	required_shared_ptr<uint8_t> m_rf5c68_ram;
	required_device_array<ym2203_device, 2> m_ym2203;
	required_device_array<ym2608_device, 2> m_ym2608;
	required_device_array<ym2610_device, 2> m_ym2610;
	required_device_array<ym3812_device, 2> m_ym3812;
	required_device_array<ym3526_device, 2> m_ym3526;
	required_device_array<y8950_device, 2> m_y8950;
	required_device_array<ymf262_device, 2> m_ymf262;
	required_device_array<ymf278b_device, 2> m_ymf278b;
	required_device_array<ymf271_device, 2> m_ymf271;
	required_device_array<ymz280b_device, 2> m_ymz280b;
	required_device_array<ay8910_device, 2> m_ay8910;
	required_device_array<multipcm_device, 2> m_multipcm;
	required_device<gameboy_sound_device> m_dmg;
	required_device<n2a03_device> m_nescpu;
	required_shared_ptr<uint8_t> m_nesram;
	required_device<k053260_device> m_k053260;
	required_device_array<k054539_device, 2> m_k054539;
	required_device<c6280_device> m_c6280;
	required_device<h6280_device> m_h6280;
	required_device_array<pokey_device, 2> m_pokey;
	required_device<c352_device> m_c352;
	required_device_array<okim6295_device, 2> m_okim6295;
	required_device<qsound_device> m_qsound;
	required_device<k051649_device> m_k051649;
	required_device<iremga20_device> m_ga20;
	required_device<rf5c68_device> m_rf5c164; // TODO : !!RF5C164!!
	required_shared_ptr<uint8_t> m_rf5c164_ram;
	required_device<x1_010_device> m_x1_010;

	uint32_t m_okim6295_clock[2];
	uint32_t m_okim6295_pin7[2];
	uint8_t m_scc_reg;

	uint32_t r32(int offset) const;
	uint8_t r8(int offset) const;
};

vgmplay_device::vgmplay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, VGMPLAY, tag, owner, clock),
	m_act_leds(*this, "led_act_%u", 0U),
	m_file_config("file", ENDIANNESS_LITTLE, 8, 32),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 32),
	m_io16_config("io16", ENDIANNESS_LITTLE, 16, 32)
{
}

void vgmplay_device::device_start()
{
	set_icountptr(m_icount);
	m_file = &space(AS_PROGRAM);
	m_io   = &space(AS_IO);
	m_io16  = &space(AS_IO16);

	m_act_leds.resolve();
	m_act_led_index = std::make_unique<led_expiry_iterator []>(LED_COUNT);
	for (act_led led = act_led(0); LED_COUNT != led; led = act_led(led + 1))
		m_act_led_index[led] = m_act_led_expiries.emplace(m_act_led_expiries.end(), led, attotime::never);
	m_act_led_off = m_act_led_expiries.begin();
	m_act_led_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vgmplay_device::act_led_expired), this));

	save_item(NAME(m_pc));

	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();
}

void vgmplay_device::device_reset()
{
	m_pc = 0;
	m_state = RESET;
	m_paused = false;

	m_ym2612_stream_offset = 0;
	blocks_clear();
}

void vgmplay_device::pulse_act_led(act_led led)
{
	m_act_leds[led] = 1;

	bool const was_first(m_act_led_expiries.begin() == m_act_led_index[led]);
	bool const all_off(m_act_led_expiries.begin() == m_act_led_off);
	attotime const now(machine().time());

	m_act_led_index[led]->second = now + attotime::from_msec(ACT_LED_PERSIST_MS);
	if (m_act_led_off != m_act_led_index[led])
		m_act_led_expiries.splice(m_act_led_off, m_act_led_expiries, m_act_led_index[led]);
	else
		++m_act_led_off;
	if (all_off)
		m_act_led_timer->adjust(attotime::from_msec(ACT_LED_PERSIST_MS));
	else if (was_first)
		m_act_led_timer->adjust(m_act_led_expiries.begin()->second - now);
}

TIMER_CALLBACK_MEMBER(vgmplay_device::act_led_expired)
{
	attotime const now(machine().time());

	while ((now + attotime::from_msec(1)) >= m_act_led_expiries.begin()->second)
	{
		led_expiry_iterator const expired(m_act_led_expiries.begin());
		m_act_leds[expired->first] = 0;
		expired->second = attotime::never;
		if (expired != m_act_led_off)
		{
			m_act_led_expiries.splice(m_act_led_off, m_act_led_expiries, expired);
			m_act_led_off = expired;
		}
	}

	if (m_act_led_expiries.begin() != m_act_led_off)
		m_act_led_timer->adjust(m_act_led_expiries.begin()->second - now);
}

void vgmplay_device::stop()
{
	device_reset();
	m_paused = true;
}

void vgmplay_device::pause()
{
	m_paused = !m_paused;
}

void vgmplay_device::play()
{
	if (m_paused && m_state != DONE)
		m_paused = false;
	else
		device_reset();
}

uint32_t vgmplay_device::execute_min_cycles() const
{
	return 0;
}

uint32_t vgmplay_device::execute_max_cycles() const
{
	return 65536;
}

uint32_t vgmplay_device::execute_input_lines() const
{
	return 0;
}

void vgmplay_device::blocks_clear()
{
	for(int i = 0; i < 0x40; i++) {
		m_rom_blocks[0][i].clear();
		m_rom_blocks[1][i].clear();
		m_data_streams[i].clear();
		m_data_stream_starts[i].clear();
	}
}

uint32_t vgmplay_device::handle_data_block(uint32_t address)
{
	uint32_t size = m_file->read_dword(m_pc+3);
	int second = (size & 0x80000000) ? 1 : 0;
	size &= 0x7fffffff;

	uint8_t type = m_file->read_byte(m_pc+2);
	if(type < 0x40) {
		uint32_t start = m_data_streams[type].size();
		m_data_stream_starts[type].push_back(start);
		m_data_streams[type].resize(start + size);
		for(uint32_t i=0; i<size; i++)
			m_data_streams[type][start+i] = m_file->read_byte(m_pc+7+i);

	} else if(type < 0x7f)
		logerror("ignored compressed stream size %x type %02x\n", size, type);

	else if (type < 0x80)
		logerror("ignored compression table size %x\n", size);

	else if (type < 0xc0) {
		//uint32_t rs = m_file->read_dword(m_pc+7);
		uint32_t start = m_file->read_dword(m_pc+11);
		std::unique_ptr<uint8_t[]> block = std::make_unique<uint8_t[]>(size - 8);
		for(uint32_t i=0; i<size-8; i++)
			block[i] = m_file->read_byte(m_pc+15+i);
		m_rom_blocks[second][type - 0x80].emplace_front(start, start+size-9, std::move(block));
	} else if(type <= 0xc2) {
		uint16_t start = m_file->read_word(m_pc+7);
		uint32_t data_size = size - 2;
		if (type == 0xc0) {
			for (int i = 0; i < data_size; i++)
				m_io->write_byte(A_RF5C68RAM + start + i, m_file->read_byte(m_pc + 9 + i));
		} else if (type == 0xc1) {
			for (int i = 0; i < data_size; i++)
				m_io->write_byte(A_RF5C164RAM + start + i, m_file->read_byte(m_pc + 9 + i));
		} else if (type == 0xc2) {
			for (int i = 0; i < data_size; i++)
				m_io->write_byte(A_NESRAM + start + i, m_file->read_byte(m_pc + 9 + i));
		}
	} else {
		logerror("ignored ram block size %x type %02x\n", size, type);
	}
	return 7+size;
}

uint32_t vgmplay_device::handle_pcm_write(uint32_t address)
{
	uint8_t type = m_file->read_byte(m_pc+2);
	uint32_t src = m_file->read_dword(m_pc+3) & 0xffffff;
	uint32_t dst = m_file->read_dword(m_pc+6) & 0xffffff;
	uint32_t size = m_file->read_dword(m_pc+9) & 0xffffff;

	if (m_data_streams->size() <= type || m_data_streams[type].size() <= src + size)
		logerror("ignored pcm ram writes src %x dst %x size %x type %02x\n", src, dst, size, type);
	else if (type == 0x01) {
		for (int i = 0; i < size; i++)
			m_io->write_byte(A_RF5C68RAM + dst + i, m_data_streams[type][src + i]);
	} else if (type == 0x02) {
		for (int i = 0; i < size; i++)
			m_io->write_byte(A_RF5C164RAM + dst + i, m_data_streams[type][src + i]);
	} else if (type == 0x07) {
		for (int i = 0; i < size; i++)
			m_io->write_byte(A_NESRAM + dst + i, m_data_streams[type][src + i]);
	} else {
		logerror("ignored pcm ram writes src %x dst %x size %x type %02x\n", src, dst, size, type);
	}
	return 12;
}

void vgmplay_device::execute_run()
{
	while(m_icount > 0) {
		switch(m_state) {
		case RESET: {
			uint32_t size = m_io->read_dword(REG_SIZE);
			if(!size) {
				m_pc = 0;
				m_state = DONE;
				break;
			}
			uint32_t version = m_file->read_dword(8);
			m_pc = version < 0x150 ? 0x40 : 0x34 + m_file->read_dword(0x34);
			m_state = RUN;
			break;
		}
		case RUN: {
			if (m_paused)
			{
				machine().sound().system_mute(1);
				m_icount = 0;
				return;
			}
			else
			{
				machine().sound().system_mute(0);
			}
			if(machine().debug_flags & DEBUG_FLAG_ENABLED)
				debugger_instruction_hook(m_pc);
			uint8_t code = m_file->read_byte(m_pc);
			switch(code) {
			case 0x30:
				pulse_act_led(LED_SN76496);
				m_io->write_byte(A_SN76496_1 + 0, m_file->read_byte(m_pc + 1));
				m_pc += 2;
				break;

			case 0x3f:
				pulse_act_led(LED_SN76496);
				m_io->write_byte(A_SN76496_1 + 1, m_file->read_byte(m_pc + 1));
				m_pc += 2;
				break;

			case 0x4f:
				pulse_act_led(LED_SN76496);
				m_io->write_byte(A_SN76496_0 + 1, m_file->read_byte(m_pc + 1));
				m_pc += 2;
				break;

			case 0x50:
				pulse_act_led(LED_SN76496);
				m_io->write_byte(A_SN76496_0 + 0, m_file->read_byte(m_pc + 1));
				m_pc += 2;
				break;

			case 0x51:
				pulse_act_led(LED_YM2413);
				m_io->write_byte(A_YM2413_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2413_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x52:
			case 0x53:
				pulse_act_led(LED_YM2612);
				m_io->write_byte(A_YM2612_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2612_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x54:
				pulse_act_led(LED_YM2151);
				m_io->write_byte(A_YM2151_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2151_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x55:
				pulse_act_led(LED_YM2203);
				m_io->write_byte(A_YM2203_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2203_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x56:
			case 0x57:
				pulse_act_led(LED_YM2608);
				m_io->write_byte(A_YM2608_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2608_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x58:
			case 0x59:
				pulse_act_led(LED_YM2610);
				m_io->write_byte(A_YM2610_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2610_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5a:
				pulse_act_led(LED_YM3812);
				m_io->write_byte(A_YM3812_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM3812_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5b:
				pulse_act_led(LED_YM3526);
				m_io->write_byte(A_YM3526_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM3526_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5c:
				pulse_act_led(LED_Y8950);
				m_io->write_byte(A_Y8950_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_Y8950_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5d:
				pulse_act_led(LED_YMZ280B);
				m_io->write_byte(A_YMZ280B_0 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YMZ280B_0 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x5e:
			case 0x5f:
				pulse_act_led(LED_YMF262);
				m_io->write_byte(A_YMF262_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YMF262_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0x61: {
				uint32_t duration = m_file->read_word(m_pc+1);
				m_icount -= duration;
				m_pc += 3;
				break;
			}

			case 0x62:
				m_icount -= 735;
				m_pc++;
				break;

			case 0x63:
				m_icount -= 882;
				m_pc++;
				break;

			case 0x66:
			{
				uint32_t loop_offset = m_file->read_dword(0x1c);
				if (!loop_offset)
				{
					m_state = DONE;
					break;
				}

				m_pc = 0x1c + loop_offset;
				break;
			}

			case 0x67:
				m_pc += handle_data_block(m_pc);
				break;

			case 0x68:
				m_pc += handle_pcm_write(m_pc);
				break;

			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
				m_icount -= 1+(code & 0xf);
				m_pc += 1;
				break;

			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				pulse_act_led(LED_YM2612);
				if(!m_data_streams[0].empty()) {
					if(m_ym2612_stream_offset >= int(m_data_streams[0].size()))
						m_ym2612_stream_offset = 0;

					m_io->write_byte(A_YM2612_0+0, 0x2a);
					m_io->write_byte(A_YM2612_0+1, m_data_streams[0][m_ym2612_stream_offset]);
					m_ym2612_stream_offset++;
				}
				m_pc += 1;
				m_icount -= code & 0xf;
				break;

			case 0xa0: {
				pulse_act_led(LED_AY8910);
				uint8_t reg = m_file->read_byte(m_pc+1);
				if(reg & 0x80) {
					m_io->write_byte(A_AY8910B+1, reg & 0x7f);
					m_io->write_byte(A_AY8910B+0, m_file->read_byte(m_pc+2));
				} else {
					m_io->write_byte(A_AY8910A+1, reg & 0x7f);
					m_io->write_byte(A_AY8910A+0, m_file->read_byte(m_pc+2));
				}
				m_pc += 3;
				break;
			}

			case 0xa1:
				pulse_act_led(LED_YM2413);
				m_io->write_byte(A_YM2413_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2413_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa2:
			case 0xa3:
				pulse_act_led(LED_YM2612);
				m_io->write_byte(A_YM2612_1 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2612_1 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa4:
				pulse_act_led(LED_YM2151);
				m_io->write_byte(A_YM2151_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2151_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa5:
				pulse_act_led(LED_YM2203);
				m_io->write_byte(A_YM2203_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2203_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa6:
			case 0xa7:
				pulse_act_led(LED_YM2608);
				m_io->write_byte(A_YM2608_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2608_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xa8:
			case 0xa9:
				pulse_act_led(LED_YM2610);
				m_io->write_byte(A_YM2610_0 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM2610_0 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xaa:
				pulse_act_led(LED_YM3812);
				m_io->write_byte(A_YM3812_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM3812_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xab:
				pulse_act_led(LED_YM3526);
				m_io->write_byte(A_YM3526_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YM3526_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xac:
				pulse_act_led(LED_Y8950);
				m_io->write_byte(A_Y8950_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_Y8950_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xad:
				pulse_act_led(LED_YMZ280B);
				m_io->write_byte(A_YMZ280B_1 + 0, m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YMZ280B_1 + 1, m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xae:
			case 0xaf:
				pulse_act_led(LED_YMF262);
				m_io->write_byte(A_YMF262_1 + 0 + ((code & 1) << 1), m_file->read_byte(m_pc + 1));
				m_io->write_byte(A_YMF262_1 + 1 + ((code & 1) << 1), m_file->read_byte(m_pc + 2));
				m_pc += 3;
				break;

			case 0xb0:
				pulse_act_led(LED_RF5C68);
				m_io->write_byte(A_RF5C68 + m_file->read_byte(m_pc+1), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;

			case 0xb1:
				pulse_act_led(LED_RF5C164);
				m_io->write_byte(A_RF5C164 + m_file->read_byte(m_pc+1), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;

			case 0xb3:
				pulse_act_led(LED_GAMEBOY);
				m_io->write_byte(A_GAMEBOY + m_file->read_byte(m_pc+1), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;

			case 0xb4:
				pulse_act_led(LED_NESAPU);
				m_io->write_byte(A_NESAPU + m_file->read_byte(m_pc+1), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;

			case 0xb5: {
				pulse_act_led(LED_MULTIPCM);
				uint8_t offset = m_file->read_byte(m_pc+1);
				if(offset & 0x80)
					m_io->write_byte(A_MULTIPCMB + (offset & 0x7f), m_file->read_byte(m_pc+2));
				else
					m_io->write_byte(A_MULTIPCMA + (offset & 0x7f), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;
			}

			case 0xb8: {
				pulse_act_led(LED_OKIM6295);
				uint8_t offset = m_file->read_byte(m_pc+1);
				if(offset & 0x80)
					m_io->write_byte(A_OKIM6295B + (offset & 0x7f), m_file->read_byte(m_pc+2));
				else
					m_io->write_byte(A_OKIM6295A + (offset & 0x7f), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;
			}

			case 0xb9:
				pulse_act_led(LED_C6280);
				m_io->write_byte(A_C6280 + m_file->read_byte(m_pc+1), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;

			case 0xba:
				pulse_act_led(LED_K053260);
				m_io->write_byte(A_K053260 + m_file->read_byte(m_pc+1), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;

			case 0xbb: {
				pulse_act_led(LED_POKEY);
				uint8_t offset = m_file->read_byte(m_pc+1);
				if (offset & 0x80)
					m_io->write_byte(A_POKEYB + (offset & 0x7f), m_file->read_byte(m_pc+2));
				else
					m_io->write_byte(A_POKEYA + (offset & 0x7f), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;
			}

			case 0xbf: {
				pulse_act_led(LED_GA20);
				m_io->write_byte(A_GA20 + m_file->read_byte(m_pc+1), m_file->read_byte(m_pc+2));
				m_pc += 3;
				break;
			}

			case 0xc0:
				pulse_act_led(LED_SEGAPCM);
				m_io->write_byte(A_SEGAPCM + (m_file->read_word(m_pc+1) & 0x7ff), m_file->read_byte(m_pc+3));
				m_pc += 4;
				break;

			case 0xc1:
				pulse_act_led(LED_RF5C68);
				m_io->write_byte(A_RF5C68RAM + m_file->read_word(m_pc+1), m_file->read_byte(m_pc+3));
				m_pc += 4;
				break;

			case 0xc2:
				pulse_act_led(LED_RF5C164);
				m_io->write_byte(A_RF5C164RAM + m_file->read_word(m_pc+1), m_file->read_byte(m_pc+3));
				m_pc += 4;
				break;

			case 0xc3: {
				pulse_act_led(LED_MULTIPCM);
				uint8_t offset = m_file->read_byte(m_pc+1);
				if (offset & 0x80) {
					m_io->write_byte(A_MULTIPCMB + 4 + (offset & 0x7f), m_file->read_byte(m_pc+3));
					m_io->write_byte(A_MULTIPCMB + 8 + (offset & 0x7f), m_file->read_byte(m_pc+2));
				} else {
					m_io->write_byte(A_MULTIPCMA + 4 + (offset & 0x7f), m_file->read_byte(m_pc+3));
					m_io->write_byte(A_MULTIPCMA + 8 + (offset & 0x7f), m_file->read_byte(m_pc+2));
				}
				m_pc += 4;
				break;
			}

			case 0xc4:
				pulse_act_led(LED_QSOUND);
				m_io->write_byte(A_QSOUND + 0, m_file->read_byte(m_pc+1));
				m_io->write_byte(A_QSOUND + 1, m_file->read_byte(m_pc+2));
				m_io->write_byte(A_QSOUND + 2, m_file->read_byte(m_pc+3));
				m_pc += 4;
				break;

			case 0xc8: {
				pulse_act_led(LED_X1_010);
				m_io->write_byte(A_X1_010 + ((m_file->read_byte(m_pc+1) << 8) | m_file->read_byte(m_pc+2)), m_file->read_byte(m_pc+3));
				m_pc += 4;
				break;
			}

			case 0xd0:
			{
				pulse_act_led(LED_YMF278B);
				uint8_t offset = m_file->read_byte(m_pc + 1);
				if (offset & 0x80)
				{
					m_io->write_byte(A_YMF278B_1 + (offset & 7) * 2, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_YMF278B_1 + (offset & 7) * 2 + 1, m_file->read_byte(m_pc + 3));
				}
				else
				{
					m_io->write_byte(A_YMF278B_0 + (offset & 7) * 2, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_YMF278B_0 + (offset & 7) * 2 + 1, m_file->read_byte(m_pc + 3));
				}
				m_pc += 4;
				break;
			}

			case 0xd1: {
				pulse_act_led(LED_YMF271);
				uint8_t offset = m_file->read_byte(m_pc+1);
				if (offset & 0x80)
				{
					m_io->write_byte(A_YMF271_1 + (offset & 7) * 2, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_YMF271_1 + (offset & 7) * 2 + 1, m_file->read_byte(m_pc + 3));
				}
				else
				{
					m_io->write_byte(A_YMF271_0 + (offset & 7) * 2, m_file->read_byte(m_pc + 2));
					m_io->write_byte(A_YMF271_0 + (offset & 7) * 2 + 1, m_file->read_byte(m_pc + 3));
				}
				m_pc += 4;
				break;
			}

			case 0xd2: {
				pulse_act_led(LED_K051649);
				uint32_t offset = m_file->read_byte(m_pc+1) << 1;
				m_io->write_byte(A_K051649 + (offset | 0), m_file->read_byte(m_pc+2));
				m_io->write_byte(A_K051649 + (offset | 1), m_file->read_byte(m_pc+3));
				m_pc += 4;
				break;
			}

			case 0xd3: {
				pulse_act_led(LED_K054539);
				uint16_t offset = m_file->read_byte(m_pc+1) << 8 | m_file->read_byte(m_pc+2);
				if (offset & 0x8000)
					m_io->write_byte(A_K054539B + (offset & 0x3ff), m_file->read_byte(m_pc+3));
				else
					m_io->write_byte(A_K054539A + (offset & 0x3ff), m_file->read_byte(m_pc+3));
				m_pc += 4;
				break;
			}

			case 0xe0:
				pulse_act_led(LED_YM2612);
				m_ym2612_stream_offset = m_file->read_dword(m_pc+1);
				m_pc += 5;
				break;

			case 0xe1: {
				pulse_act_led(LED_C352);
				uint32_t addr = (m_file->read_byte(m_pc+1) << 8) | m_file->read_byte(m_pc+2);
				uint16_t data = (m_file->read_byte(m_pc+3) << 8) | m_file->read_byte(m_pc+4);
				m_io16->write_word(A_C352 + (addr << 1), data);
				m_pc += 5;
				break;
			}

			default:
				logerror("unhandled code %02x (%02x %02x %02x %02x)\n", code, m_file->read_byte(m_pc+1), m_file->read_byte(m_pc+2), m_file->read_byte(m_pc+3), m_file->read_byte(m_pc+4));
				m_state = DONE;
				m_icount = 0;
				break;
			}
			break;
		}
		case DONE:
		{
			static bool done = false;
			if(!done && !m_loop)
			{
				logerror("done\n");
				done = true;
			}
			else if (m_loop)
			{
				device_reset();
			}
			else
			{
				if(machine().debug_flags & DEBUG_FLAG_ENABLED)
					debugger_instruction_hook(m_pc);
			}
			m_icount = 0;
			break;
		}
		}
	}
}

void vgmplay_device::execute_set_input(int inputnum, int state)
{
}

device_memory_interface::space_config_vector vgmplay_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_file_config),
		std::make_pair(AS_IO,      &m_io_config),
		std::make_pair(AS_IO16,    &m_io16_config)
	};
}

void vgmplay_device::state_import(const device_state_entry &entry)
{
}

void vgmplay_device::state_export(const device_state_entry &entry)
{
}

void vgmplay_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
}

std::unique_ptr<util::disasm_interface> vgmplay_device::create_disassembler()
{
	return std::make_unique<vgmplay_disassembler>();
}

uint32_t vgmplay_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t vgmplay_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	switch(opcodes.r8(pc)) {
	case 0x30:
		util::stream_format(stream, "psg.1 write %02x", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x3f:
		util::stream_format(stream, "psg.1 r06 = %02x", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x4f:
		util::stream_format(stream, "psg.0 r06 = %02x", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x50:
		util::stream_format(stream, "psg.0 write %02x", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x51:
		util::stream_format(stream, "ym2413.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x52:
	case 0x53:
		util::stream_format(stream, "ym2612.0 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x54:
		util::stream_format(stream, "ym2151.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x55:
		util::stream_format(stream, "ym2203.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x56:
	case 0x57:
		util::stream_format(stream, "ym2608.0 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x58:
	case 0x59:
		util::stream_format(stream, "ym2610.0 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x5a:
		util::stream_format(stream, "ym3812.0 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x5b:
		util::stream_format(stream, "ym3526.0 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0x5c:
		util::stream_format(stream, "y8950.0 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0x5d:
		util::stream_format(stream, "ymz280b.0 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0x5e:
	case 0x5f:
		util::stream_format(stream, "ymf262.0 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0x61: {
		uint32_t duration = opcodes.r8(pc+1) | (opcodes.r8(pc+2) << 8);
		util::stream_format(stream, "wait %d", duration);
		return 3 | SUPPORTED;
	}

	case 0x62:
		util::stream_format(stream, "wait 735");
		return 1 | SUPPORTED;

	case 0x63:
		util::stream_format(stream, "wait 882");
		return 1 | SUPPORTED;

	case 0x66:
		util::stream_format(stream, "end");
		return 1 | SUPPORTED;

	case 0x67: {
		static const char *const basic_types[8] = {
			"ym2612 pcm",
			"rf5c68 pcm",
			"rf5c164 pcm",
			"pwm pcm",
			"okim6258 adpcm",
			"huc6280 pcm",
			"scsp pcm",
			"nes apu dpcm"
		};

		static const char *const rom_types[20] = {
			"sega pcm rom",
			"ym2608 delta-t rom",
			"ym2610 adpcm rom",
			"ym2610 delta-t rom",
			"ymf278b rom",
			"ymf271 rom",
			"ymz280b rom",
			"ymf278b rom",
			"y8950 delta-t rom",
			"multipcm rom",
			"upd7759 rom",
			"okim6295 rom",
			"k054539 rom",
			"c140 rom",
			"k053260 rom",
			"qsound rom",
			"es5505/es5506 rom",
			"x1-010 rom",
			"c352 rom",
			"ga20 rom"
		};

		static const char *const ram_types[3] = {
			"rf5c68 ram",
			"rf5c164 ram",
			"nes apu ram"
		};

		static const char *const ram2_types[2] = {
			"scsp ram",
			"es5503 ram"
		};

		uint8_t type = opcodes.r8(pc+2);
		uint32_t size = opcodes.r8(pc+3) | (opcodes.r8(pc+4) << 8) | (opcodes.r8(pc+5) << 16) | (opcodes.r8(pc+6) << 24);
		if(type < 0x8)
			util::stream_format(stream, "data-block %x, %s", size, basic_types[type]);
		else if(type < 0x40)
			util::stream_format(stream, "data-block %x, %02x", size, type);
		else if(type < 0x48)
			util::stream_format(stream, "data-block %x comp., %s", size, basic_types[type & 0x3f]);
		else if(type < 0x7f)
			util::stream_format(stream, "data-block %x comp., %02x", size, type & 0x3f);
		else if(type < 0x80)
			util::stream_format(stream, "decomp-table %x, %02x/%02x", size, opcodes.r8(pc+7), opcodes.r8(pc+8));
		else if(type < 0x94)
			util::stream_format(stream, "data-block %x, %s", size, rom_types[type & 0x7f]);
		else if(type < 0xc0)
			util::stream_format(stream, "data-block %x, rom %02x", size, type);
		else if(type < 0xc3)
			util::stream_format(stream, "data-block %x, %s", size, ram_types[type & 0x1f]);
		else if(type < 0xe0)
			util::stream_format(stream, "data-block %x, ram %02x", size, type);
		else if(type < 0xe2)
			util::stream_format(stream, "data-block %x, %s", size, ram2_types[type & 0x1f]);
		else
			util::stream_format(stream, "data-block %x, ram %02x", size, type);
		return (7+size) | SUPPORTED;
	}

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		util::stream_format(stream, "wait %d", 1+(opcodes.r8(pc) & 0x0f));
		return 1 | SUPPORTED;

	case 0x80:
		util::stream_format(stream, "ym2612.0 r2a = rom++");
		return 1 | SUPPORTED;

	case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		util::stream_format(stream, "ym2612.0 r2a = rom++; wait %d", opcodes.r8(pc) & 0xf);
		return 1 | SUPPORTED;

	case 0xa0:
		util::stream_format(stream, "ay8910 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xa1:
		util::stream_format(stream, "ym2413.1 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xa2:
	case 0xa3:
		util::stream_format(stream, "ym2612.1 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xa4:
		util::stream_format(stream, "ym2151.1 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xa5:
		util::stream_format(stream, "ym2203.1 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xa6:
	case 0xa7:
		util::stream_format(stream, "ym2608.1 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xaa:
		util::stream_format(stream, "ym3812.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xab:
		util::stream_format(stream, "ym3526.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xac:
		util::stream_format(stream, "y8950.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xad:
		util::stream_format(stream, "ymz280b.1 r%02x = %02x", opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xae:
	case 0xaf:
		util::stream_format(stream, "ymf262.1 %d r%02x = %02x", opcodes.r8(pc) & 1, opcodes.r8(pc + 1), opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xb0:
		util::stream_format(stream, "rf5c68 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xb1:
		util::stream_format(stream, "rf5c164 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xb2:
		util::stream_format(stream, "pwm r%x = %03x", opcodes.r8(pc+1) >> 4, opcodes.r8(pc+2) | ((opcodes.r8(pc+1) & 0xf) << 8));
		return 3 | SUPPORTED;

	case 0xb3:
		util::stream_format(stream, "dmg r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xb4:
		util::stream_format(stream, "nesapu r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xb5:
		util::stream_format(stream, "multipcm r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xb6:
		util::stream_format(stream, "upd7759 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xb7:
		util::stream_format(stream, "okim6258 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xb8:
		util::stream_format(stream, "okim6295 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xb9:
		util::stream_format(stream, "huc6280 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xba:
		util::stream_format(stream, "k053260 r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xbb:
		util::stream_format(stream, "pokey r%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2));
		return 3 | SUPPORTED;

	case 0xc0:
		util::stream_format(stream, "segapcm %04x = %02x", opcodes.r8(pc+1) | (opcodes.r8(pc+2) << 8), opcodes.r8(pc+3));
		return 4 | SUPPORTED;

	case 0xc1:
		util::stream_format(stream, "rf5c68 %04x = %02x", opcodes.r8(pc+1) | (opcodes.r8(pc+2) << 8), opcodes.r8(pc+3));
		return 4 | SUPPORTED;

	case 0xc2:
		util::stream_format(stream, "rf5c164 %04x = %02x", opcodes.r8(pc+1) | (opcodes.r8(pc+2) << 8), opcodes.r8(pc+3));
		return 4 | SUPPORTED;

	case 0xc3:
		util::stream_format(stream, "multipcm c%02x.off = %04x", opcodes.r8(pc+1), opcodes.r8(pc+2) | (opcodes.r8(pc+3) << 8));
		return 4 | SUPPORTED;

	case 0xc4:
		util::stream_format(stream, "qsound %02x = %04x", opcodes.r8(pc+3), opcodes.r8(pc+2) | (opcodes.r8(pc+1) << 8));
		return 4 | SUPPORTED;

	case 0xc8:
		util::stream_format(stream, "x1-010 %04x = %02x", opcodes.r8(pc+2) | (opcodes.r8(pc+1) << 8), opcodes.r8(pc+3));
		return 4 | SUPPORTED;

	case 0xd0:
		util::stream_format(stream, "ymf278b.%d r%02x.%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc+1) & 0x7f, opcodes.r8(pc+2), opcodes.r8(pc+3));
		return 4 | SUPPORTED;

	case 0xd1:
		util::stream_format(stream, "ymf271.%d r%02x.%02x = %02x", BIT(opcodes.r8(pc + 1), 7), opcodes.r8(pc+1), opcodes.r8(pc+2), opcodes.r8(pc+3));
		return 4 | SUPPORTED;

	case 0xd2:
		util::stream_format(stream, "scc1 r%02x.%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2), opcodes.r8(pc+3));
		return 4 | SUPPORTED;

	case 0xd3:
		util::stream_format(stream, "k054539 r%02x.%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2), opcodes.r8(pc+3));
		return 4 | SUPPORTED;

	case 0xd4:
		util::stream_format(stream, "c140 r%02x.%02x = %02x", opcodes.r8(pc+1), opcodes.r8(pc+2), opcodes.r8(pc+3));
		return 4 | SUPPORTED;

	case 0xe0: {
		uint32_t off = opcodes.r8(pc+1) | (opcodes.r8(pc+2) << 8) | (opcodes.r8(pc+3) << 16) | (opcodes.r8(pc+4) << 24);
		util::stream_format(stream, "ym2612 offset = %x", off);
		return 5 | SUPPORTED;
	}

	case 0xe1: {
		uint16_t addr = (opcodes.r8(pc+1) << 8) | opcodes.r8(pc+2);
		uint16_t data = (opcodes.r8(pc+3) << 8) | opcodes.r8(pc+4);
		util::stream_format(stream, "c352 r%04x = %04x", addr, data);
		return 5 | SUPPORTED;
	}

	default:
		util::stream_format(stream, "?? %02x", opcodes.r8(pc));
		return 1 | SUPPORTED;
	}
}

uint8_t vgmplay_device::rom_r(int chip, uint8_t type, offs_t offset)
{
	for(const auto &b : m_rom_blocks[chip][type - 0x80])
	{
		if(offset >= b.start_address && offset <= b.end_address)
		{
			return b.data[offset - b.start_address];
		}
	}
	return 0;
}

template<int Chip>
READ8_MEMBER(vgmplay_device::segapcm_rom_r)
{
	return rom_r(Chip, 0x80, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::ymf278b_rom_r)
{
	return rom_r(Chip, 0x84, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::ymf271_rom_r)
{
	return rom_r(Chip, 0x85, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::ymz280b_rom_r)
{
	return rom_r(Chip, 0x86, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::multipcm_rom_r)
{
	if (m_multipcm_banked[Chip] == 1)
	{
		offset &= 0x1fffff;
		if (offset & 0x100000)
		{
			if (m_multipcm_bank_l[Chip] == m_multipcm_bank_r[Chip])
			{
				offset = ((m_multipcm_bank_r[Chip] & ~0xf) << 16) | (offset & 0xfffff);
			}
			else
			{
				if (offset & 0x80000)
				{
					offset = ((m_multipcm_bank_l[Chip] & ~0x7) << 16) | (offset & 0x7ffff);
				}
				else
				{
					offset = ((m_multipcm_bank_r[Chip] & ~0x7) << 16) | (offset & 0x7ffff);
				}
			}
		}
	}
	return rom_r(Chip, 0x89, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::okim6295_rom_r)
{
	if (m_okim6295_nmk112_enable[Chip])
	{
		if ((offset < 0x400) && (m_okim6295_nmk112_enable[Chip] & 0x80))
		{
			offset = (m_okim6295_nmk112_bank[Chip][(offset >> 8) & 0x3] << 16) | (offset & 0x3ff);
		}
		else
		{
			offset = (m_okim6295_nmk112_bank[Chip][(offset >> 16) & 0x3] << 16) | (offset & 0xffff);
		}
	}
	else
	{
		offset = (m_okim6295_bank[Chip] * 0x40000) | offset;
	}
	return rom_r(Chip, 0x8b, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::k054539_rom_r)
{
	return rom_r(Chip, 0x8c, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::k053260_rom_r)
{
	return rom_r(Chip, 0x8e, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::qsound_rom_r)
{
	return rom_r(Chip, 0x8f, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::x1_010_rom_r)
{
	return rom_r(Chip, 0x91, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::c352_rom_r)
{
	return rom_r(Chip, 0x92, offset);
}

template<int Chip>
READ8_MEMBER(vgmplay_device::ga20_rom_r)
{
	return rom_r(Chip, 0x93, offset);
}

vgmplay_state::vgmplay_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_vgmplay(*this, "vgmplay")
	, m_lspeaker(*this, "lspeaker")
	, m_rspeaker(*this, "rspeaker")
	, m_sn76496(*this, "sn76496.%d", 0)
	, m_ym2413(*this, "ym2413.%d", 0)
	, m_ym2612(*this, "ym2612.%d", 0)
	, m_ym2151(*this, "ym2151.%d", 0)
	, m_segapcm(*this, "segapcm")
	, m_rf5c68(*this, "rf5c68")
	, m_rf5c68_ram(*this, "rf5c68_ram")
	, m_ym2203(*this, "ym2203.%d", 0)
	, m_ym2608(*this, "ym2608.%d", 0)
	, m_ym2610(*this, "ym2610.%d", 0)
	, m_ym3812(*this, "ym3812.%d", 0)
	, m_ym3526(*this, "ym3526.%d", 0)
	, m_y8950(*this, "y8950.%d", 0)
	, m_ymf262(*this, "ymf262.%d", 0)
	, m_ymf278b(*this, "ymf278b.%d", 0)
	, m_ymf271(*this, "ymf271.%d", 0)
	, m_ymz280b(*this, "ymz280b.%d", 0)
	, m_ay8910(*this, "ay8910%c", 'a')
	, m_multipcm(*this, "multipcm%c", 'a')
	, m_dmg(*this, "dmg")
	, m_nescpu(*this, "nescpu")
	, m_nesram(*this, "nesapu_ram")
	, m_k053260(*this, "k053260")
	, m_k054539(*this, "k054539%c", 'a')
	, m_c6280(*this, "c6280")
	, m_h6280(*this, "h6280")
	, m_pokey(*this, "pokey%c", 'a')
	, m_c352(*this, "c352")
	, m_okim6295(*this, "okim6295%c", 'a')
	, m_qsound(*this, "qsound")
	, m_k051649(*this, "k051649")
	, m_ga20(*this, "ga20")
	, m_rf5c164(*this, "rf5c164")
	, m_rf5c164_ram(*this, "rf5c164_ram")
	, m_x1_010(*this, "x1_010")
{
}

uint32_t vgmplay_state::r32(int off) const
{
	if(off + 3 < int(m_file_data.size()))
		return m_file_data[off] | (m_file_data[off+1] << 8) | (m_file_data[off+2] << 16) | (m_file_data[off+3] << 24);
	return 0;
}

uint8_t vgmplay_state::r8(int off) const
{
	if(off < int(m_file_data.size()))
		return m_file_data[off];
	return 0;
}

static const ay8910_device::psg_type_t vgm_ay8910_type(uint8_t vgm_type)
{
	return (vgm_type & 0x10) ? ay8910_device::PSG_TYPE_YM : ay8910_device::PSG_TYPE_AY;
}

static const uint8_t vgm_ay8910_flags(uint8_t vgm_flags)
{
	uint8_t flags = 0;
	if (vgm_flags & 1) flags |= AY8910_LEGACY_OUTPUT;
	if (vgm_flags & 2) flags |= AY8910_SINGLE_OUTPUT;
	if (vgm_flags & 4) flags |= AY8910_DISCRETE_OUTPUT;
	return flags;
}

QUICKLOAD_LOAD_MEMBER(vgmplay_state, load_file)
{
	m_vgmplay->stop();

	m_file_data.resize(quickload_size);

	if (image.fread(&m_file_data[0], quickload_size) != quickload_size)
	{
		m_file_data.clear();
		return image_init_result::FAIL;
	}
	else
	{
		// Decompress gzip-compressed files (aka vgz)
		if(m_file_data[0] == 0x1f && m_file_data[1] == 0x8b) {
			std::vector<uint8_t> decomp;
			int bs = m_file_data.size();
			decomp.resize(2*bs);
			z_stream str;
			str.zalloc = nullptr;
			str.zfree = nullptr;
			str.opaque = nullptr;
			str.data_type = 0;
			str.next_in = &m_file_data[0];
			str.avail_in = m_file_data.size();
			str.total_in = 0;
			str.total_out = 0;
			int err = inflateInit2(&str, 31);
			if(err != Z_OK) {
				logerror("gzip header but not a gzip file\n");
				m_file_data.clear();
				return image_init_result::FAIL;
			}
			do {
				if(str.total_out >= decomp.size())
					decomp.resize(decomp.size() + bs);
				str.next_out = &decomp[str.total_out];
				str.avail_out = decomp.size() - str.total_out;
				err = inflate(&str, Z_SYNC_FLUSH);
			} while(err == Z_OK);
			if(err != Z_STREAM_END) {
				logerror("broken gzip file\n");
				m_file_data.clear();
				return image_init_result::FAIL;
			}
			m_file_data.resize(str.total_out);
			memcpy(&m_file_data[0], &decomp[0], str.total_out);
		}

		if(m_file_data.size() < 0x40 || r32(0) != 0x206d6756) {
			logerror("Not a vgm/vgz file\n");
			m_file_data.clear();
			return image_init_result::FAIL;
		}

		uint32_t version = r32(8);
		logerror("File version %x.%02x\n", version >> 8, version & 0xff);

		uint32_t data_start = version >= 0x150 ? r32(0x34) + 0x34 : 0x40;

		// Parse clocks
		m_sn76496[0]->set_unscaled_clock(r32(0x0c) & ~0xc0000000);
		m_sn76496[1]->set_unscaled_clock(r32(0x0c) & 0x40000000 ? r32(0x0c) & ~0xc0000000 : 0);
		if (r32(0x0c) & 0x80000000)
			logerror("Warning: file requests an unsupported T6W28");

		m_ym2413[0]->set_unscaled_clock(r32(0x10) & ~0x40000000);
		m_ym2413[1]->set_unscaled_clock(r32(0x10) & 0x40000000 ? r32(0x10) & ~0x40000000 : 0);

		m_ym2612[0]->set_unscaled_clock((version >= 0x110 ? r32(0x2c) : r32(0x10)) & ~0xc0000000);
		m_ym2612[1]->set_unscaled_clock((version >= 0x110 ? r32(0x2c) : r32(0x10)) & 0x40000000 ? (version >= 0x110 ? r32(0x2c) : r32(0x10)) & ~0xc0000000 : 0);
		if (version >= 0x110 && (r32(0x2c) & 0x80000000))
			logerror("Warning: file requests an unsupported YM3438\n");

		m_ym2151[0]->set_unscaled_clock((version >= 0x110 ? r32(0x30) : r32(0x10)) & ~0x40000000);
		m_ym2151[1]->set_unscaled_clock((version >= 0x110 ? r32(0x30) : r32(0x10)) & 0x40000000 ? (version >= 0x110 ? r32(0x30) : r32(0x10)) & ~0x40000000 : 0);

		m_segapcm->set_unscaled_clock(version >= 0x151 && data_start >= 0x3c ? r32(0x38) : 0);
		m_segapcm->set_bank(version >= 0x151 && data_start >= 0x40 ? r32(0x3c) : 0);

		m_rf5c68->set_unscaled_clock(version >= 0x151 && data_start >= 0x44 ? r32(0x40) : 0);
		m_ym2203[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x48 ? r32(0x44) & ~0x40000000 : 0);
		m_ym2203[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x48 && (r32(0x44) & 0x40000000) ? r32(0x44) & ~0x40000000 : 0);
		m_ym2608[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x4c ? r32(0x48) & ~0x40000000 : 0);
		m_ym2608[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x4c && r32(0x48) & 0x40000000 ? r32(0x48) & ~0x40000000 : 0);

		m_ym2610[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x50 ? r32(0x4c) & ~0xc0000000 : 0);
		m_ym2610[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x50 && r32(0x4c) & 0x40000000 ? r32(0x4c) & ~0xc0000000 : 0);
		if (version >= 0x151 && data_start >= 0x50 && (r32(0x4c) & 0x80000000))
			logerror("Warning: file requests an unsupported YM2610B\n");

		m_ym3812[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x54 ? r32(0x50) & ~0xc0000000 : 0);
		m_ym3812[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x54 && r32(0x50) & 0x40000000 ? r32(0x50) & ~0xc0000000 : 0);
		if (version >= 0x151 && data_start >= 0x54 && (r32(0x50) & 0x80000000))
			logerror("Warning: file requests an unsupported SoundBlaster Pro\n");

		m_ym3526[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x58 ? r32(0x54) & ~0x40000000 : 0);
		m_ym3526[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x58 && r32(0x54) & 0x40000000 ? r32(0x54) & ~0x40000000 : 0);
		m_y8950[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x5c ? r32(0x58) & ~0x40000000 : 0);
		m_y8950[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x5c && r32(0x58) & 0x40000000 ? r32(0x58) & ~0x40000000 : 0);
		m_ymf262[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x60 ? r32(0x5c) & ~0x40000000 : 0);
		m_ymf262[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x60 && r32(0x5c) & 0x40000000 ? r32(0x5c) & ~0x40000000 : 0);
		m_ymf278b[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x64 ? r32(0x60) & ~0x40000000 : 0);
		m_ymf278b[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x64 && r32(0x60) & 0x40000000 ? r32(0x60) & ~0x40000000 : 0);
		m_ymf271[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x68 ? r32(0x64) & ~0x40000000 : 0);
		m_ymf271[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x68 && r32(0x64) & 0x40000000 ? r32(0x64) & ~0x40000000 : 0);
		m_ymz280b[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x6c ? r32(0x68) & ~0x40000000 : 0);
		m_ymz280b[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x6c && r32(0x68) & 0x40000000 ? r32(0x68) & ~0x40000000 : 0);

		m_rf5c164->set_unscaled_clock(version >= 0x151 && data_start >= 0x70 ? r32(0x6c) : 0);

		if(version >= 0x151 && data_start >= 0x74 && r32(0x70))
			logerror("Warning: file requests an unsupported PWM\n");

		m_ay8910[0]->set_unscaled_clock(version >= 0x151 && data_start >= 0x78 ? r32(0x74) & ~0x40000000 : 0);
		m_ay8910[1]->set_unscaled_clock(version >= 0x151 && data_start >= 0x78 && (r32(0x74) & 0x40000000) ? r32(0x74) & ~0x40000000 : 0);
		m_ay8910[0]->set_psg_type(vgm_ay8910_type(version >= 0x151 && data_start >= 0x7c ? r8(0x78) : 0));
		m_ay8910[1]->set_psg_type(vgm_ay8910_type(version >= 0x151 && data_start >= 0x7c ? r8(0x78) : 0));
		m_ay8910[0]->set_flags(vgm_ay8910_flags(version >= 0x151 && data_start >= 0x7a ? r8(0x79) : 0));
		m_ay8910[1]->set_flags(vgm_ay8910_flags(version >= 0x151 && data_start >= 0x7a ? r8(0x79) : 0));
		m_ym2203[0]->set_flags(vgm_ay8910_flags(version >= 0x151 && data_start >= 0x7b ? r8(0x7a) : 0));
		m_ym2203[1]->set_flags(vgm_ay8910_flags(version >= 0x151 && data_start >= 0x7b ? r8(0x7a) : 0));
		m_ym2608[0]->set_flags(vgm_ay8910_flags(version >= 0x151 && data_start >= 0x7c ? r8(0x7b) : 0));
		m_ym2608[1]->set_flags(vgm_ay8910_flags(version >= 0x151 && data_start >= 0x7c ? r8(0x7b) : 0));

		m_dmg->set_unscaled_clock(version >= 0x161 && data_start >= 0x84 ? r32(0x80) & ~0x40000000 : 0);
		if (version >= 0x161 && data_start >= 0x84 && (r32(0x80) & 0x40000000))
			logerror("Warning: file requests an unsupported 2nd DMG\n");

		m_nescpu->set_unscaled_clock(version >= 0x161 && data_start >= 0x88 ? r32(0x84) & ~0xc0000000 : 0);
		if (version >= 0x161 && data_start >= 0x88 && (r32(0x84) & 0x80000000))
			logerror("Warning: file requests an unsupported FDS sound addon\n");
		if (version >= 0x161 && data_start >= 0x88 && (r32(0x84) & 0x40000000))
			logerror("Warning: file requests an unsupported 2nd NES APU\n");

		m_multipcm[0]->set_unscaled_clock(version >= 0x161 && data_start >= 0x8c ? r32(0x88) & ~0x40000000 : 0);
		m_multipcm[1]->set_unscaled_clock(version >= 0x161 && data_start >= 0x8c && (r32(0x88) & 0x40000000) ? r32(0x88) & ~0x40000000 : 0);

		if (version >= 0x161 && data_start >= 0x90 && r32(0x8c))
			logerror("Warning: file requests an unsupported uPD7759\n");
		if (version >= 0x161 && data_start >= 0x94 && r32(0x90))
			logerror("Warning: file requests an unsupported OKIM6258\n");

		m_k054539[0]->init_flags(version >= 0x161 && data_start >= 0x96 ? r8(0x95) : 0);
		m_k054539[1]->init_flags(version >= 0x161 && data_start >= 0x96 ? r8(0x95) : 0);

		m_okim6295_clock[0] = version >= 0x161 && data_start >= 0x9c ? r32(0x98) & ~0xc0000000 : 0;
		m_okim6295_clock[1] = version >= 0x161 && data_start >= 0x9c && (r32(0x98) & 0x40000000) ? r32(0x98) & ~0xc0000000 : 0;
		m_okim6295[0]->set_unscaled_clock(m_okim6295_clock[0]);
		m_okim6295[1]->set_unscaled_clock(m_okim6295_clock[1]);

		m_okim6295_pin7[0] = version >= 0x161 && data_start >= 0x9c && (r32(0x98) & 0x80000000) ? 1 : 0;
		m_okim6295_pin7[1] = version >= 0x161 && data_start >= 0x9c && (r32(0x98) & 0x40000000) && (r32(0x98) & 0x80000000) ? 1 : 0;
		m_okim6295[0]->set_pin7(m_okim6295_pin7[0] ? okim6295_device::PIN7_HIGH : okim6295_device::PIN7_LOW);
		m_okim6295[1]->set_pin7(m_okim6295_pin7[1] ? okim6295_device::PIN7_HIGH : okim6295_device::PIN7_LOW);

		m_k051649->set_unscaled_clock(version >= 0x161 && data_start >= 0xa0 ? r32(0x9c) & ~0xc0000000 : 0);
		if (version >= 0x161 && data_start >= 0xa0 && (r32(0x9c) & 0x80000000))
			logerror("Warning: file requests an unsupported Konami SCC\n");
		if (version >= 0x161 && data_start >= 0xa0 && (r32(0x9c) & 0x40000000))
			logerror("Warning: file requests an unsupported 2nd K051649\n");

		// HACK: Some VGMs contain 48,000 instead of 18,432,000
		if (version >= 0x161 && data_start >= 0xa4 && (r32(0xa0) & ~0x40000000) == 48000)
		{
			m_k054539[0]->set_clock_scale(384);
			m_k054539[1]->set_clock_scale(384);
		}

		m_k054539[0]->set_unscaled_clock(version >= 0x161 && data_start >= 0xa4 ? r32(0xa0) & ~0x40000000 : 0);
		m_k054539[1]->set_unscaled_clock(version >= 0x161 && data_start >= 0xa4 && (r32(0xa0) & 0x40000000) ? r32(0xa0) & ~0x40000000 : 0);

		m_c6280->set_unscaled_clock(version >= 0x161 && data_start >= 0xa8 ? r32(0xa4) & ~0x40000000 : 0);
		if (version >= 0x161 && data_start >= 0xa8 && (r32(0xa4) & 0x40000000))
			logerror("Warning: file requests an unsupported 2nd C6280\n");

		if (version >= 0x161 && data_start >= 0xac && r32(0xa8))
			logerror("Warning: file requests an unsupported C140\n");

		m_k053260->set_unscaled_clock(version >= 0x161 && data_start >= 0xb0 ? r32(0xac) & ~0x40000000 : 0);
		if (version >= 0x161 && data_start >= 0xb0 && (r32(0xac) & 0x40000000))
			logerror("Warning: file requests an unsupported 2nd K053260\n");

		m_pokey[0]->set_unscaled_clock(version >= 0x161 && data_start >= 0xb4 ? r32(0xb0) & ~0x40000000 : 0);
		m_pokey[1]->set_unscaled_clock(version >= 0x161 && data_start >= 0xb4 && (r32(0xb0) & 0x40000000) ? r32(0xb0) & ~0x40000000 : 0);

		// HACK: VGMs contain 4,000,000 instead of 60,000,000
		if (version >= 0x161 && data_start >= 0xb8 && r32(0xb4) == 4000000)
			m_qsound->set_clock_scale(15);

		m_qsound->set_unscaled_clock(version >= 0x161 && data_start >= 0xb8 ? r32(0xb4) : 0);

		if (version >= 0x171 && data_start >= 0xbc && r32(0xb8))
			logerror("Warning: file requests an unsupported SCSP\n");

		if (version >= 0x170 && data_start >= 0xc0 && r32(0xbc))
			logerror("Warning: file requests an unsupported Extra Header\n");

		if (version >= 0x171 && data_start >= 0xc4 && r32(0xc0))
			logerror("Warning: file requests an unsupported WonderSwan\n");
		if (version >= 0x171 && data_start >= 0xc8 && r32(0xc4))
			logerror("Warning: file requests an unsupported VSU\n");
		if (version >= 0x171 && data_start >= 0xcc && r32(0xc8))
			logerror("Warning: file requests an unsupported SAA1099\n");
		if (version >= 0x171 && data_start >= 0xd0 && r32(0xcc))
			logerror("Warning: file requests an unsupported ES5503\n");
		if (version >= 0x171 && data_start >= 0xd4 && r32(0xd0))
			logerror("Warning: file requests an unsupported %s\n", r32(0xd0) & 0x80000000 ? "ES5506" : "ES5505");

		m_c352->set_divider(version >= 0x171 && data_start >= 0xd7 && r8(0xd6) ? r8(0xd6) * 4 : 1);

		m_x1_010->set_unscaled_clock(version >= 0x171 && data_start >= 0xdc ? r32(0xd8) & ~0x40000000 : 0);
		if (version >= 0x171 && data_start >= 0xdc && (r32(0xd8) & 0x40000000))
			logerror("Warning: file requests an unsupported 2nd X1-010\n");

		m_c352->set_unscaled_clock(version >= 0x171 && data_start >= 0xe0 ? r32(0xdc) & ~0x40000000 : 0);
		if (version >= 0x171 && data_start >= 0xe0 && (r32(0xdc) & 0x40000000))
			logerror("Warning: file requests an unsupported 2nd C352\n");

		m_ga20->set_unscaled_clock(version >= 0x171 && data_start >= 0xe4 ? r32(0xe0) & ~0x40000000 : 0);
		if (version >= 0x171 && data_start >= 0xe4 && (r32(0xe0) & 0x40000000))
			logerror("Warning: file requests an unsupported 2nd GA20\n");

		machine().schedule_soft_reset();

		return image_init_result::PASS;
	}
}

READ8_MEMBER(vgmplay_state::file_r)
{
	if(offset < m_file_data.size())
		return m_file_data[offset];
	return 0;
}

READ8_MEMBER(vgmplay_state::file_size_r)
{
	uint32_t size = m_file_data.size();
	return size >> (8*offset);
}

template<int Chip>
WRITE8_MEMBER(vgmplay_device::multipcm_bank_hi_w)
{
	if (offset & 1)
		m_multipcm_bank_l[Chip] = (m_multipcm_bank_l[Chip] & 0xff) | (data << 16);
	if (offset & 2)
		m_multipcm_bank_r[Chip] = (m_multipcm_bank_r[Chip] & 0xff) | (data << 16);
}

template<int Chip>
WRITE8_MEMBER(vgmplay_device::multipcm_bank_lo_w)
{
	if (offset & 1)
		m_multipcm_bank_l[Chip] = (m_multipcm_bank_l[Chip] & 0xff00) | data;
	if (offset & 2)
		m_multipcm_bank_r[Chip] = (m_multipcm_bank_r[Chip] & 0xff00) | data;

	m_multipcm_banked[Chip] = 1;
}

template<int Chip>
WRITE8_MEMBER(vgmplay_state::okim6295_clock_w)
{
	uint32_t old = m_okim6295_clock[Chip];
	int shift = ((offset & 3) << 3);
	m_okim6295_clock[Chip] = (m_okim6295_clock[Chip] & ~(mem_mask << shift)) | ((data & mem_mask) << shift);
	if (old != m_okim6295_clock[Chip])
		m_okim6295[Chip]->set_unscaled_clock(m_okim6295_clock[Chip]);

}

template<int Chip>
WRITE8_MEMBER(vgmplay_state::okim6295_pin7_w)
{
	if ((data & mem_mask) != (m_okim6295_pin7[Chip] & mem_mask))
	{
		COMBINE_DATA(&m_okim6295_pin7[Chip]);
		m_okim6295[Chip]->set_pin7(m_okim6295_pin7[Chip]);
	}
}

template<int Chip>
WRITE8_MEMBER(vgmplay_device::okim6295_nmk112_enable_w)
{
	COMBINE_DATA(&m_okim6295_nmk112_enable[Chip]);
}

template<int Chip>
WRITE8_MEMBER(vgmplay_device::okim6295_bank_w)
{
	if ((data & mem_mask) != (m_okim6295_bank[Chip] & mem_mask))
	{
		COMBINE_DATA(&m_okim6295_bank[Chip]);
	}
}

template<int Chip>
WRITE8_MEMBER(vgmplay_device::okim6295_nmk112_bank_w)
{
	offset &= 3;
	if ((data & mem_mask) != (m_okim6295_nmk112_bank[Chip][offset] & mem_mask))
	{
		COMBINE_DATA(&m_okim6295_nmk112_bank[Chip][offset]);
	}
}

WRITE8_MEMBER(vgmplay_state::scc_w)
{
	switch(offset & 1)
	{
	case 0x00:
		m_scc_reg = data;
		break;
	case 0x01:
		switch(offset >> 1)
		{
		case 0x00:
			m_k051649->k051649_waveform_w(space, m_scc_reg, data);
			break;
		case 0x01:
			m_k051649->k051649_frequency_w(space, m_scc_reg, data);
			break;
		case 0x02:
			m_k051649->k051649_volume_w(space, m_scc_reg, data);
			break;
		case 0x03:
			m_k051649->k051649_keyonoff_w(space, m_scc_reg, data);
			break;
		case 0x04:
			m_k051649->k052539_waveform_w(space, m_scc_reg, data);
			break;
		case 0x05:
			m_k051649->k051649_test_w(space, m_scc_reg, data);
			break;
		}
		break;
	}
}

INPUT_CHANGED_MEMBER(vgmplay_state::key_pressed)
{
	if (!newval)
		return;

	int val = (uint8_t)(uintptr_t)param;
	switch (val)
	{
		case VGMPLAY_STOP:
			m_vgmplay->stop();
			break;
		case VGMPLAY_PAUSE:
			m_vgmplay->pause();
			break;
		case VGMPLAY_PLAY:
			m_vgmplay->play();
			break;
		case VGMPLAY_RESTART:
			m_vgmplay->reset();
			break;
		case VGMPLAY_LOOP:
			m_vgmplay->toggle_loop();
			break;
	}
}

static INPUT_PORTS_START( vgmplay )
	PORT_START("CONTROLS")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_STOP)        PORT_NAME("Stop")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_PAUSE)       PORT_NAME("Pause")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_PLAY)        PORT_NAME("Play")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_RESTART)     PORT_NAME("Restart")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_CHANGED_MEMBER(DEVICE_SELF, vgmplay_state, key_pressed, VGMPLAY_LOOP)        PORT_NAME("Loop")
INPUT_PORTS_END

void vgmplay_state::file_map(address_map &map)
{
	map(0x00000000, 0xffffffff).r(FUNC(vgmplay_state::file_r));
}

void vgmplay_state::soundchips16_map(address_map &map)
{
	map(vgmplay_device::A_C352, vgmplay_device::A_C352+0x7fff).w(m_c352, FUNC(c352_device::write));
}

void vgmplay_state::soundchips_map(address_map &map)
{
	map(vgmplay_device::REG_SIZE, vgmplay_device::REG_SIZE + 3).r(FUNC(vgmplay_state::file_size_r));
	map(vgmplay_device::A_SN76496_0 + 0, vgmplay_device::A_SN76496_0 + 0).w(m_sn76496[0], FUNC(sn76496_device::command_w));
//  map(vgmplay_device::A_SN76496_0 + 1, vgmplay_device::A_SN76496_0 + 1).w(m_sn76496[0], FUNC(sn76496_device::stereo_w)); // TODO: GG stereo
	map(vgmplay_device::A_SN76496_1 + 0, vgmplay_device::A_SN76496_1 + 0).w(m_sn76496[1], FUNC(sn76496_device::command_w));
//  map(vgmplay_device::A_SN76496_1 + 1, vgmplay_device::A_SN76496_1 + 1).w(m_sn76496[1], FUNC(sn76496_device::stereo_w)); // TODO: GG stereo
	map(vgmplay_device::A_YM2413_0, vgmplay_device::A_YM2413_0 + 1).w(m_ym2413[0], FUNC(ym2413_device::write));
	map(vgmplay_device::A_YM2413_1, vgmplay_device::A_YM2413_1 + 1).w(m_ym2413[1], FUNC(ym2413_device::write));
	map(vgmplay_device::A_YM2612_0, vgmplay_device::A_YM2612_0 + 3).w(m_ym2612[0], FUNC(ym2612_device::write));
	map(vgmplay_device::A_YM2612_1, vgmplay_device::A_YM2612_1 + 3).w(m_ym2612[1], FUNC(ym2612_device::write));
	map(vgmplay_device::A_YM2151_0, vgmplay_device::A_YM2151_0 + 1).w(m_ym2151[0], FUNC(ym2151_device::write));
	map(vgmplay_device::A_YM2151_1, vgmplay_device::A_YM2151_1 + 1).w(m_ym2151[1], FUNC(ym2151_device::write));
	map(vgmplay_device::A_SEGAPCM, vgmplay_device::A_SEGAPCM + 0x7ff).w(m_segapcm, FUNC(segapcm_device::sega_pcm_w));
	map(vgmplay_device::A_RF5C68, vgmplay_device::A_RF5C68 + 0xf).w(m_rf5c68, FUNC(rf5c68_device::rf5c68_w));
	map(vgmplay_device::A_RF5C68RAM, vgmplay_device::A_RF5C68RAM + 0xffff).w(m_rf5c68, FUNC(rf5c68_device::rf5c68_mem_w));
	map(vgmplay_device::A_YM2203_0, vgmplay_device::A_YM2203_0 + 1).w(m_ym2203[0], FUNC(ym2203_device::write));
	map(vgmplay_device::A_YM2203_1, vgmplay_device::A_YM2203_1 + 1).w(m_ym2203[1], FUNC(ym2203_device::write));
	map(vgmplay_device::A_YM2608_0, vgmplay_device::A_YM2608_0 + 0x3).w(m_ym2608[0], FUNC(ym2608_device::write));
	map(vgmplay_device::A_YM2608_1, vgmplay_device::A_YM2608_1 + 0x3).w(m_ym2608[1], FUNC(ym2608_device::write));
	map(vgmplay_device::A_YM2610_0, vgmplay_device::A_YM2610_0 + 0x3).w(m_ym2610[0], FUNC(ym2610_device::write));
	map(vgmplay_device::A_YM2610_1, vgmplay_device::A_YM2610_1 + 0x3).w(m_ym2610[1], FUNC(ym2610_device::write));
	map(vgmplay_device::A_YM3812_0, vgmplay_device::A_YM3812_0 + 1).w(m_ym3812[0], FUNC(ym3812_device::write));
	map(vgmplay_device::A_YM3812_1, vgmplay_device::A_YM3812_1 + 1).w(m_ym3812[1], FUNC(ym3812_device::write));
	map(vgmplay_device::A_YM3526_0, vgmplay_device::A_YM3526_0 + 1).w(m_ym3526[0], FUNC(ym3526_device::write));
	map(vgmplay_device::A_YM3526_1, vgmplay_device::A_YM3526_1 + 1).w(m_ym3526[1], FUNC(ym3526_device::write));
	map(vgmplay_device::A_Y8950_0, vgmplay_device::A_Y8950_0 + 1).w(m_y8950[0], FUNC(y8950_device::write));
	map(vgmplay_device::A_Y8950_1, vgmplay_device::A_Y8950_1 + 1).w(m_y8950[1], FUNC(y8950_device::write));
	map(vgmplay_device::A_YMF262_0, vgmplay_device::A_YMF262_0 + 1).w(m_ymf262[0], FUNC(ymf262_device::write));
	map(vgmplay_device::A_YMF262_1, vgmplay_device::A_YMF262_1 + 1).w(m_ymf262[1], FUNC(ymf262_device::write));
	map(vgmplay_device::A_YMF278B_0, vgmplay_device::A_YMF278B_0 + 0xf).w(m_ymf278b[0], FUNC(ymf278b_device::write));
	map(vgmplay_device::A_YMF278B_1, vgmplay_device::A_YMF278B_1 + 0xf).w(m_ymf278b[1], FUNC(ymf278b_device::write));
	map(vgmplay_device::A_YMF271_0, vgmplay_device::A_YMF271_0 + 0xf).w(m_ymf271[0], FUNC(ymf271_device::write));
	map(vgmplay_device::A_YMF271_1, vgmplay_device::A_YMF271_1 + 0xf).w(m_ymf271[1], FUNC(ymf271_device::write));
	map(vgmplay_device::A_YMZ280B_0, vgmplay_device::A_YMZ280B_0 + 0x1).w(m_ymz280b[0], FUNC(ymz280b_device::write));
	map(vgmplay_device::A_YMZ280B_1, vgmplay_device::A_YMZ280B_1 + 0x1).w(m_ymz280b[1], FUNC(ymz280b_device::write));
	map(vgmplay_device::A_AY8910A, vgmplay_device::A_AY8910A).w("ay8910a", FUNC(ay8910_device::data_w));
	map(vgmplay_device::A_AY8910A+1, vgmplay_device::A_AY8910A+1).w("ay8910a", FUNC(ay8910_device::address_w));
	map(vgmplay_device::A_AY8910B, vgmplay_device::A_AY8910B).w("ay8910b", FUNC(ay8910_device::data_w));
	map(vgmplay_device::A_AY8910B+1, vgmplay_device::A_AY8910B+1).w("ay8910b", FUNC(ay8910_device::address_w));
	map(vgmplay_device::A_K053260, vgmplay_device::A_K053260+0x2f).w(m_k053260, FUNC(k053260_device::write));
	map(vgmplay_device::A_C6280, vgmplay_device::A_C6280+0xf).w(m_c6280, FUNC(c6280_device::c6280_w));
	map(vgmplay_device::A_OKIM6295A, vgmplay_device::A_OKIM6295A).w("okim6295a", FUNC(okim6295_device::write));
	map(vgmplay_device::A_OKIM6295A+0x8, vgmplay_device::A_OKIM6295A+0xb).w(FUNC(vgmplay_state::okim6295_clock_w<0>));
	map(vgmplay_device::A_OKIM6295A+0xc, vgmplay_device::A_OKIM6295A+0xc).w(FUNC(vgmplay_state::okim6295_pin7_w<0>));
	map(vgmplay_device::A_OKIM6295A+0xe, vgmplay_device::A_OKIM6295A+0xe).w("vgmplay", FUNC(vgmplay_device::okim6295_nmk112_enable_w<0>));
	map(vgmplay_device::A_OKIM6295A+0xf, vgmplay_device::A_OKIM6295A+0xf).w("vgmplay", FUNC(vgmplay_device::okim6295_bank_w<0>));
	map(vgmplay_device::A_OKIM6295A+0x10, vgmplay_device::A_OKIM6295A+0x13).w("vgmplay", FUNC(vgmplay_device::okim6295_nmk112_bank_w<0>));
	map(vgmplay_device::A_OKIM6295B, vgmplay_device::A_OKIM6295B).w("okim6295b", FUNC(okim6295_device::write));
	map(vgmplay_device::A_OKIM6295B+0x8, vgmplay_device::A_OKIM6295B+0xb).w(FUNC(vgmplay_state::okim6295_clock_w<1>));
	map(vgmplay_device::A_OKIM6295B+0xc, vgmplay_device::A_OKIM6295B+0xc).w(FUNC(vgmplay_state::okim6295_pin7_w<1>));
	map(vgmplay_device::A_OKIM6295B+0xe, vgmplay_device::A_OKIM6295B+0xe).w("vgmplay", FUNC(vgmplay_device::okim6295_nmk112_enable_w<1>));
	map(vgmplay_device::A_OKIM6295B+0xf, vgmplay_device::A_OKIM6295B+0xf).w("vgmplay", FUNC(vgmplay_device::okim6295_bank_w<1>));
	map(vgmplay_device::A_OKIM6295B+0x10, vgmplay_device::A_OKIM6295B+0x13).w("vgmplay", FUNC(vgmplay_device::okim6295_nmk112_bank_w<1>));
	map(vgmplay_device::A_GAMEBOY, vgmplay_device::A_GAMEBOY+0x16).w(m_dmg, FUNC(gameboy_sound_device::sound_w));
	map(vgmplay_device::A_GAMEBOY+0x20, vgmplay_device::A_GAMEBOY+0x2f).w(m_dmg, FUNC(gameboy_sound_device::wave_w));
	map(vgmplay_device::A_NESAPU, vgmplay_device::A_NESAPU+0x1f).w("nescpu:nesapu", FUNC(nesapu_device::write));
	map(vgmplay_device::A_NESRAM, vgmplay_device::A_NESRAM+0xffff).ram().share("nesapu_ram");
	map(vgmplay_device::A_MULTIPCMA, vgmplay_device::A_MULTIPCMA+3).w("multipcma", FUNC(multipcm_device::write));
	map(vgmplay_device::A_MULTIPCMA+4, vgmplay_device::A_MULTIPCMA+7).w("vgmplay", FUNC(vgmplay_device::multipcm_bank_hi_w<0>));
	map(vgmplay_device::A_MULTIPCMA+8, vgmplay_device::A_MULTIPCMA+11).w("vgmplay", FUNC(vgmplay_device::multipcm_bank_lo_w<0>));
	map(vgmplay_device::A_MULTIPCMB, vgmplay_device::A_MULTIPCMB+3).w("multipcmb", FUNC(multipcm_device::write));
	map(vgmplay_device::A_MULTIPCMB+4, vgmplay_device::A_MULTIPCMB+7).w("vgmplay", FUNC(vgmplay_device::multipcm_bank_hi_w<1>));
	map(vgmplay_device::A_MULTIPCMB+8, vgmplay_device::A_MULTIPCMB+11).w("vgmplay", FUNC(vgmplay_device::multipcm_bank_lo_w<1>));
	map(vgmplay_device::A_POKEYA, vgmplay_device::A_POKEYA+0xf).w("pokeya", FUNC(pokey_device::write));
	map(vgmplay_device::A_POKEYB, vgmplay_device::A_POKEYB+0xf).w("pokeyb", FUNC(pokey_device::write));
	map(vgmplay_device::A_K054539A, vgmplay_device::A_K054539A+0x22f).w("k054539a", FUNC(k054539_device::write));
	map(vgmplay_device::A_K054539B, vgmplay_device::A_K054539B+0x22f).w("k054539b", FUNC(k054539_device::write));
	map(vgmplay_device::A_QSOUND, vgmplay_device::A_QSOUND+0x2).w(m_qsound, FUNC(qsound_device::qsound_w));
	map(vgmplay_device::A_K051649, vgmplay_device::A_K051649+0xf).w(FUNC(vgmplay_state::scc_w));
	map(vgmplay_device::A_GA20, vgmplay_device::A_GA20+0x1f).w(m_ga20, FUNC(iremga20_device::irem_ga20_w));
	map(vgmplay_device::A_RF5C164, vgmplay_device::A_RF5C164+0xf).w(m_rf5c164, FUNC(rf5c68_device::rf5c68_w));
	map(vgmplay_device::A_RF5C164RAM, vgmplay_device::A_RF5C164RAM+0xffff).w(m_rf5c164, FUNC(rf5c68_device::rf5c68_mem_w));
	map(vgmplay_device::A_X1_010, vgmplay_device::A_X1_010+0x1fff).w(m_x1_010, FUNC(x1_010_device::write));
}

template<int Chip>
void vgmplay_state::segapcm_map(address_map &map)
{
	map(0, 0x1fffff).r("vgmplay", FUNC(vgmplay_device::segapcm_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::ymf278b_map(address_map &map)
{
	map(0, 0x3fffff).r("vgmplay", FUNC(vgmplay_device::ymf278b_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::ymf271_map(address_map &map)
{
	map(0, 0x7fffff).r("vgmplay", FUNC(vgmplay_device::ymf271_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::ymz280b_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::ymz280b_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::multipcm_map(address_map &map)
{
	map(0, 0x3fffff).r("vgmplay", FUNC(vgmplay_device::multipcm_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::k053260_map(address_map &map)
{
	map(0, 0x1fffff).r("vgmplay", FUNC(vgmplay_device::k053260_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::okim6295_map(address_map &map)
{
	map(0, 0x3ffff).r("vgmplay", FUNC(vgmplay_device::okim6295_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::k054539_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::k054539_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::c352_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::c352_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::qsound_map(address_map &map)
{
	map(0, 0xffffff).r("vgmplay", FUNC(vgmplay_device::qsound_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::ga20_map(address_map &map)
{
	map(0, 0xfffff).r("vgmplay", FUNC(vgmplay_device::ga20_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::x1_010_map(address_map &map)
{
	map(0, 0xfffff).r("vgmplay", FUNC(vgmplay_device::x1_010_rom_r<Chip>));
}

template<int Chip>
void vgmplay_state::nescpu_map(address_map &map)
{
	map(0, 0xffff).ram().share("nesapu_ram");
}

template<int Chip>
void vgmplay_state::rf5c68_map(address_map &map)
{
	map(0, 0xffff).ram().share("rf5c68_ram");
}

template<int Chip>
void vgmplay_state::rf5c164_map(address_map &map)
{
	map(0, 0xffff).ram().share("rf5c164_ram");
}

template<int Chip>
void vgmplay_state::h6280_map(address_map &map)
{
	map(0, 0xffff).noprw();
}

template<int Chip>
void vgmplay_state::h6280_io_map(address_map &map)
{
	map(0, 3).noprw();
}

MACHINE_CONFIG_START(vgmplay_state::vgmplay)
	MCFG_DEVICE_ADD("vgmplay", VGMPLAY, 44100)
	MCFG_DEVICE_PROGRAM_MAP( file_map )
	MCFG_DEVICE_IO_MAP( soundchips_map )
	MCFG_CPU_IO16_MAP( soundchips16_map )

	MCFG_QUICKLOAD_ADD("quickload", vgmplay_state, load_file, "vgm,vgz", 0)
	MCFG_QUICKLOAD_INTERFACE("vgm_quik")

	MCFG_SOFTWARE_LIST_ADD("vgm_list", "vgmplay")

	config.set_default_layout(layout_vgmplay);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SN76496(config, m_sn76496[0], 0);
	m_sn76496[0]->add_route(0, "lspeaker", 0.5);
	m_sn76496[0]->add_route(0, "rspeaker", 0.5);

	SN76496(config, m_sn76496[1], 0);
	m_sn76496[1]->add_route(0, "lspeaker", 0.5);
	m_sn76496[1]->add_route(0, "rspeaker", 0.5);

	YM2413(config, m_ym2413[0], 0);
	m_ym2413[0]->add_route(ALL_OUTPUTS, "lspeaker", 1);
	m_ym2413[0]->add_route(ALL_OUTPUTS, "rspeaker", 1);

	YM2413(config, m_ym2413[1], 0);
	m_ym2413[1]->add_route(0, "lspeaker", 1);
	m_ym2413[1]->add_route(1, "rspeaker", 1);

	YM2612(config, m_ym2612[0], 0);
	m_ym2612[0]->add_route(0, "lspeaker", 1);
	m_ym2612[0]->add_route(1, "rspeaker", 1);

	YM2612(config, m_ym2612[1], 0);
	m_ym2612[1]->add_route(0, "lspeaker", 1);
	m_ym2612[1]->add_route(1, "rspeaker", 1);

	YM2151(config, m_ym2151[0], 0);
	m_ym2151[0]->add_route(0, "lspeaker", 1);
	m_ym2151[0]->add_route(1, "rspeaker", 1);

	YM2151(config, m_ym2151[1], 0);
	m_ym2151[1]->add_route(0, "lspeaker", 1);
	m_ym2151[1]->add_route(1, "rspeaker", 1);

	MCFG_DEVICE_ADD("segapcm", SEGAPCM, 0)
	MCFG_SEGAPCM_BANK(BANK_512) // Should be configurable for yboard...
	MCFG_DEVICE_ADDRESS_MAP(0, segapcm_map<0>)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	RF5C68(config, m_rf5c68, 0);
	m_rf5c68->set_addrmap(0, &vgmplay_state::rf5c68_map<0>);
	m_rf5c68->add_route(0, "lspeaker", 1);
	m_rf5c68->add_route(1, "rspeaker", 1);

	YM2203(config, m_ym2203[0], 0);
	m_ym2203[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_ym2203[0]->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	YM2203(config, m_ym2203[1], 0);
	m_ym2203[1]->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_ym2203[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	YM2608(config, m_ym2608[0], 0);
	m_ym2608[0]->add_route(ALL_OUTPUTS, "lspeaker", 1);
	m_ym2608[0]->add_route(ALL_OUTPUTS, "rspeaker", 1);

	YM2608(config, m_ym2608[1], 0);
	m_ym2608[1]->add_route(ALL_OUTPUTS, "lspeaker", 1);
	m_ym2608[1]->add_route(ALL_OUTPUTS, "rspeaker", 1);

	YM2610(config, m_ym2610[0], 0);
	m_ym2610[0]->add_route(0, "lspeaker", 0.25);
	m_ym2610[0]->add_route(0, "rspeaker", 0.25);
	m_ym2610[0]->add_route(1, "lspeaker", 0.50);
	m_ym2610[0]->add_route(2, "rspeaker", 0.50);

	YM2610(config, m_ym2610[1], 0);
	m_ym2610[1]->add_route(0, "lspeaker", 0.25);
	m_ym2610[1]->add_route(0, "rspeaker", 0.25);
	m_ym2610[1]->add_route(1, "lspeaker", 0.50);
	m_ym2610[1]->add_route(2, "rspeaker", 0.50);

	YM3812(config, m_ym3812[0], 0);
	m_ym3812[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_ym3812[0]->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	YM3812(config, m_ym3812[1], 0);
	m_ym3812[1]->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_ym3812[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	YM3526(config, m_ym3526[0], 0);
	m_ym3526[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_ym3526[0]->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	YM3526(config, m_ym3526[1], 0);
	m_ym3526[1]->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_ym3526[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	Y8950(config, m_y8950[0], 0);
	m_y8950[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.40);
	m_y8950[0]->add_route(ALL_OUTPUTS, "rspeaker", 0.40);

	Y8950(config, m_y8950[1], 0);
	m_y8950[1]->add_route(ALL_OUTPUTS, "lspeaker", 0.40);
	m_y8950[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.40);

	YMF262(config, m_ymf262[0], 0);
	m_ymf262[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.00);
	m_ymf262[0]->add_route(ALL_OUTPUTS, "rspeaker", 1.00);

	YMF262(config, m_ymf262[1], 0);
	m_ymf262[1]->add_route(ALL_OUTPUTS, "lspeaker", 1.00);
	m_ymf262[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.00);

	YMF278B(config, m_ymf278b[0], 0);
	m_ymf278b[0]->set_addrmap(0, &vgmplay_state::ymf278b_map<0>);
	m_ymf278b[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_ymf278b[0]->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	YMF278B(config, m_ymf278b[1], 0);
	m_ymf278b[1]->set_addrmap(0, &vgmplay_state::ymf278b_map<1>);
	m_ymf278b[1]->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_ymf278b[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	YMF271(config, m_ymf271[0], 0);
	m_ymf271[0]->set_addrmap(0, &vgmplay_state::ymf271_map<0>);
	m_ymf271[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_ymf271[0]->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	YMF271(config, m_ymf271[1], 0);
	m_ymf271[1]->set_addrmap(0, &vgmplay_state::ymf271_map<0>);
	m_ymf271[1]->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_ymf271[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	YMZ280B(config, m_ymz280b[0], 0);
	m_ymz280b[0]->set_addrmap(0, &vgmplay_state::ymz280b_map<0>);
	m_ymz280b[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_ymz280b[0]->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	YMZ280B(config, m_ymz280b[1], 0);
	m_ymz280b[1]->set_addrmap(0, &vgmplay_state::ymz280b_map<1>);
	m_ymz280b[1]->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_ymz280b[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	MCFG_DEVICE_ADD("multipcma", MULTIPCM, 0)
	MCFG_DEVICE_ADDRESS_MAP(0, multipcm_map<0>)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	MCFG_DEVICE_ADD("multipcmb", MULTIPCM, 0)
	MCFG_DEVICE_ADDRESS_MAP(0, multipcm_map<1>)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	MCFG_DEVICE_ADD("dmg", DMG_APU, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	MCFG_DEVICE_ADD("ay8910a", AY8910, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.33)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33)

	MCFG_DEVICE_ADD("ay8910b", AY8910, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.33)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33)

	MCFG_DEVICE_ADD("nescpu", N2A03, 0)
	MCFG_DEVICE_PROGRAM_MAP(nescpu_map<0>)
	MCFG_DEVICE_DISABLE()

	MCFG_DEVICE_MODIFY("nescpu:nesapu")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, ":lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, ":rspeaker", 0.50)

	MCFG_DEVICE_ADD("h6280", H6280, 1000000)
	MCFG_DEVICE_PROGRAM_MAP(h6280_map<0>)
	MCFG_DEVICE_IO_MAP(h6280_io_map<0>)
	MCFG_DEVICE_DISABLE()

	MCFG_DEVICE_ADD("c6280", C6280, 0)
	MCFG_C6280_CPU("h6280")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	MCFG_K053260_ADD("k053260", 0)
	MCFG_DEVICE_ADDRESS_MAP(0, k053260_map<0>)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	MCFG_DEVICE_ADD("pokeya", POKEY, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.5)

	MCFG_DEVICE_ADD("pokeyb", POKEY, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.5)

	MCFG_DEVICE_ADD("c352", C352, 0, 1)
	MCFG_DEVICE_ADDRESS_MAP(0, c352_map<0>)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	MCFG_DEVICE_ADD("okim6295a", OKIM6295, 0, okim6295_device::PIN7_HIGH)
	MCFG_DEVICE_ADDRESS_MAP(0, okim6295_map<0>)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)

	MCFG_DEVICE_ADD("okim6295b", OKIM6295, 0, okim6295_device::PIN7_HIGH)
	MCFG_DEVICE_ADDRESS_MAP(0, okim6295_map<1>)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)

	MCFG_DEVICE_ADD("k054539a", K054539, 0)
	MCFG_DEVICE_ADDRESS_MAP(0, k054539_map<0>)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	MCFG_DEVICE_ADD("k054539b", K054539, 0)
	MCFG_DEVICE_ADDRESS_MAP(0, k054539_map<0>)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	MCFG_DEVICE_ADD("qsound", QSOUND, 0)
	MCFG_DEVICE_ADDRESS_MAP(0, qsound_map<0>)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1)

	MCFG_K051649_ADD("k051649", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.33)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33)

	IREMGA20(config, m_ga20, 0);
	m_ga20->set_addrmap(0, &vgmplay_state::ga20_map<0>);
	m_ga20->add_route(0, "lspeaker", 1);
	m_ga20->add_route(1, "rspeaker", 1);

	RF5C68(config, m_rf5c164, 0); // TODO : !!RF5C164!!
	m_rf5c164->set_addrmap(0, &vgmplay_state::rf5c164_map<0>);
	m_rf5c164->add_route(0, "lspeaker", 1);
	m_rf5c164->add_route(1, "rspeaker", 1);

	X1_010(config, m_x1_010, 0);
	m_x1_010->set_addrmap(0, &vgmplay_state::x1_010_map<0>);
	m_x1_010->add_route(0, "lspeaker", 1);
	m_x1_010->add_route(1, "rspeaker", 1);
MACHINE_CONFIG_END

ROM_START( vgmplay )
	ROM_REGION( 0x80000, "ym2608.0", ROMREGION_ERASE00 )
	ROM_REGION( 0x80000, "ym2608.1", ROMREGION_ERASE00 )
	ROM_REGION( 0x80000, "ym2610.0", ROMREGION_ERASE00 )
	ROM_REGION( 0x80000, "ym2610.1", ROMREGION_ERASE00 )
	ROM_REGION( 0x80000, "y8950.0", ROMREGION_ERASE00 )
	ROM_REGION( 0x80000, "y8950.1", ROMREGION_ERASE00 )
ROM_END

CONS( 2016, vgmplay, 0, 0, vgmplay, vgmplay, vgmplay_state, empty_init, "MAME", "VGM player", MACHINE_CLICKABLE_ARTWORK )
