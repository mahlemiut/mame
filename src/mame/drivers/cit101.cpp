// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Preliminary driver for first-generation C. Itoh video terminals.

CIT-101 (released December 1980)
    C. Itoh's first terminal, based on DEC VT100. ANSI X3.64 and V52 compatible.
    12-inch monochrome screen displaying 24 lines of 80 or 132 characters.
    8 x 10 character cell, 10 x 10 (80 columns)/9 x 10 (132 columns) display cell.
    15,600 Hz horizontal frequency; 50 Hz/60 Hz vertical frequency selectable.
    Cursor may be selected as blinking or solid block/underline, or invisible.
    7 or 8 bit ASCII characters.
    RS232-C or 20 mA current loop communications and auxiliary (printer) ports.
    85-key detachable keyboard with 7 LEDs and settable key click.
CIT-80 (released September 1981)
    "Entry-level version" of CIT-101.
    12-inch monochrome screen displaying 24 lines of 80 characters.
    7-bit characters only.
CIT-161 (released 1982)
    Colorized version of the CIT-101.
    12-inch color screen displaying 24 lines of 80 or 132 characters.
    64 combinations of 8 colors are programmable.
CIT-500 (released 1982)
    Word processing terminal with full page display.
    15-inch vertically oriented monochrome screen with tilt/swivel.
    64 lines of 80 characters (interlaced).
    105-key keyboard.
CIT-101e (released 1983)
    Ergonomic redesign of CIT-101.
    Competitive with DEC VT220 (which was released several months later).
    14-inch monochrome screen with tilt/swivel, 24 lines of 80 or 132 characters.
    85-key low-profile keyboard.
CIG-201
    Plug-in graphics card for CIT-101 and CIT-101e.
    Compatible with Tektronix 4010/4014.
CIG-261
    Plug-in color graphics card for CIT-161.
    Compatible with Tektronix 4010/4014.
CIG-267
    Plug-in color graphics card for CIT-161.
    Compatible with Tektronix 4027A.

Special SET-UP control codes:
* CTRL+S: Save settings to NVR
* CTRL+R: Recall settings from NVR
* CTRL+D: Restore default NVR settings
* CTRL+A: Set answerback message
* CTRL+X: Enable/disable Bidirectional Auxiliary I/O Channel and SET-UP D Mode
          (undocumented; SET-UP B Mode only)

************************************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/er2055.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "screen.h"

#include "machine/cit101_kbd.h"


class cit101_state : public driver_device
{
public:
	cit101_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_nvr(*this, "nvr")
		, m_chargen(*this, "chargen")
		, m_mainram(*this, "mainram")
		, m_extraram(*this, "extraram")
	{ }

	void cit101(machine_config &config);
protected:
	virtual void machine_start() override;
private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(c000_ram_r);
	DECLARE_WRITE8_MEMBER(c000_ram_w);
	DECLARE_READ8_MEMBER(e0_latch_r);
	DECLARE_WRITE8_MEMBER(e0_latch_w);

	DECLARE_WRITE8_MEMBER(screen_control_w);
	DECLARE_WRITE8_MEMBER(brightness_w);

	DECLARE_WRITE8_MEMBER(nvr_address_w);
	DECLARE_READ8_MEMBER(nvr_data_r);
	DECLARE_WRITE8_MEMBER(nvr_data_w);
	DECLARE_WRITE8_MEMBER(nvr_control_w);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	u8 m_e0_latch;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<er2055_device> m_nvr;
	required_region_ptr<u8> m_chargen;
	required_shared_ptr<u8> m_mainram;
	required_shared_ptr<u8> m_extraram;
};


void cit101_state::machine_start()
{
	subdevice<i8251_device>("comuart")->write_cts(0);
	subdevice<i8251_device>("kbduart")->write_cts(0);

	save_item(NAME(m_e0_latch));
}

u32 cit101_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const int char_width = BIT(m_extraram[0], 1) ? 10 : 9;
	const u16 ptrbase = m_mainram[0];

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const int row = y / 10;
		u16 rowaddr = m_mainram[row * 2 + ptrbase + 1] | (m_extraram[row * 2 + ptrbase + 1] & 0x3f) << 8;
		const u16 rowattr = m_mainram[row * 2 + ptrbase] | m_extraram[row * 2 + ptrbase] << 8;
		const int line = ((y % 10) / (BIT(rowattr, 8) ? 2 : 1) + (rowattr & 0x000f)) & 0xf;

		int c = 0;
		u8 attr = m_extraram[rowaddr];
		u8 char_data = m_chargen[(m_mainram[rowaddr] << 4) | line];
		if (line == 9 && BIT(attr, 0))
			char_data ^= 0xff;
		if (BIT(attr, 1))
			char_data ^= 0xff;
		for (int x = screen.visible_area().left(); x <= screen.visible_area().right(); x++)
		{
			if (x >= cliprect.left() && x <= cliprect.right())
				bitmap.pix32(y, x) = BIT(char_data, 7) ? rgb_t::white() : rgb_t::black();

			c++;
			if (!BIT(rowattr, 9) || !BIT(c, 0))
			{
				if (c < (BIT(rowattr, 9) ? char_width << 1 : char_width))
					char_data = (char_data << 1) | (char_data & 1);
				else
				{
					c = 0;
					rowaddr++;
					attr = m_extraram[rowaddr];
					char_data = m_chargen[(m_mainram[rowaddr] << 4) | line];
					if (line == 9 && BIT(attr, 0))
						char_data ^= 0xff;
					if (BIT(attr, 1))
						char_data ^= 0xff;
				}
			}
		}
	}

	return 0;
}


READ8_MEMBER(cit101_state::c000_ram_r)
{
	if (!machine().side_effects_disabled())
		m_e0_latch = m_extraram[offset];
	return m_mainram[offset];
}

WRITE8_MEMBER(cit101_state::c000_ram_w)
{
	m_extraram[offset] = m_e0_latch;
	m_mainram[offset] = data;
}

READ8_MEMBER(cit101_state::e0_latch_r)
{
	return m_e0_latch;
}

WRITE8_MEMBER(cit101_state::e0_latch_w)
{
	m_e0_latch = data;
}

WRITE8_MEMBER(cit101_state::screen_control_w)
{
	if ((m_extraram[0] & 0x06) != (data & 0x06))
	{
		const int height = BIT(data, 2) ? 312 : 260;
		const attoseconds_t frame_period = HZ_TO_ATTOSECONDS(BIT(data, 2) ? 50 : 60);
		if (BIT(data, 1))
		{
			const rectangle visarea(0, 799, 0, 239);
			m_screen->set_unscaled_clock(14.976_MHz_XTAL);
			m_screen->configure(960, height, visarea, frame_period);
		}
		else
		{
			const rectangle visarea(0, 1187, 0, 239);
			m_screen->set_unscaled_clock(22.464_MHz_XTAL);
			m_screen->configure(1440, height, visarea, frame_period);
		}
	}

	m_extraram[0] = data;
}

WRITE8_MEMBER(cit101_state::brightness_w)
{
}

WRITE8_MEMBER(cit101_state::nvr_address_w)
{
	m_nvr->set_address(data & 0x3f);
	m_nvr->set_clk(BIT(data, 6));
}

READ8_MEMBER(cit101_state::nvr_data_r)
{
	return m_nvr->data();
}

WRITE8_MEMBER(cit101_state::nvr_data_w)
{
	m_nvr->set_data(data);
}

WRITE8_MEMBER(cit101_state::nvr_control_w)
{
	m_nvr->set_control(BIT(data, 5), !BIT(data, 4), BIT(data, 7), BIT(data, 6));
}

void cit101_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x7fff).ram().share("mainram");
	map(0x8000, 0xbfff).ram().share("extraram"); // only 4 bits wide?
	map(0x8000, 0x8000).w(this, FUNC(cit101_state::screen_control_w));
	map(0xc000, 0xdfff).rw(this, FUNC(cit101_state::c000_ram_r), FUNC(cit101_state::c000_ram_w));
	map(0xfc00, 0xfc00).rw("auxuart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xfc01, 0xfc01).rw("auxuart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xfc20, 0xfc20).rw("comuart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xfc21, 0xfc21).rw("comuart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xfc40, 0xfc40).rw("kbduart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xfc41, 0xfc41).rw("kbduart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xfc60, 0xfc63).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfc80, 0xfc83).w("pit0", FUNC(pit8253_device::write));
	map(0xfcc0, 0xfcc3).w("pit1", FUNC(pit8253_device::write));
}

void cit101_state::io_map(address_map &map)
{
	map(0x00, 0x00).rw("auxuart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x01, 0x01).rw("auxuart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x20, 0x20).rw("comuart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x21, 0x21).rw("comuart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x40, 0x40).rw("kbduart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x41, 0x41).rw("kbduart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x60, 0x63).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa0, 0xa0).w(this, FUNC(cit101_state::brightness_w));
	map(0xe0, 0xe0).rw(this, FUNC(cit101_state::e0_latch_r), FUNC(cit101_state::e0_latch_w));
}


static INPUT_PORTS_START( cit101 )
INPUT_PORTS_END


MACHINE_CONFIG_START(cit101_state::cit101)
	MCFG_DEVICE_ADD("maincpu", I8085A, 6.144_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	//MCFG_SCREEN_RAW_PARAMS(14.976_MHz_XTAL, 960, 0, 800, 260, 0, 240)
	MCFG_SCREEN_RAW_PARAMS(22.464_MHz_XTAL, 1440, 0, 1188, 260, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(cit101_state, screen_update)
	MCFG_SCREEN_VBLANK_CALLBACK(INPUTLINE("maincpu", I8085_RST75_LINE))

	MCFG_DEVICE_ADD("comuart", I8251, 6.144_MHz_XTAL / 2)
	MCFG_I8251_TXD_HANDLER(WRITELINE("comm", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(WRITELINE("comm", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(WRITELINE("comm", rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE("uartint", input_merger_device, in_w<0>))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE("uartint", input_merger_device, in_w<2>))

	MCFG_DEVICE_ADD("comm", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("comuart", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(WRITELINE("comuart", i8251_device, write_dsr))
	// CTS can be disabled in SET-UP Mode C
	// DSR, CD, SI, RI are examined only during the modem test, not "always ignored" as the User's Manual claims

	MCFG_DEVICE_ADD("auxuart", I8251, 6.144_MHz_XTAL / 2)
	MCFG_I8251_TXD_HANDLER(WRITELINE("printer", rs232_port_device, write_txd))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE("uartint", input_merger_device, in_w<1>))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE("uartint", input_merger_device, in_w<3>))

	MCFG_DEVICE_ADD("printer", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("auxuart", i8251_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(WRITELINE("auxuart", i8251_device, write_cts))

	MCFG_INPUT_MERGER_ANY_HIGH("uartint")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", I8085_RST55_LINE))

	MCFG_DEVICE_ADD("kbduart", I8251, 6.144_MHz_XTAL / 2)
	MCFG_I8251_TXD_HANDLER(WRITELINE("keyboard", cit101_hle_keyboard_device, write_rxd))
	MCFG_I8251_RXRDY_HANDLER(INPUTLINE("maincpu", I8085_RST65_LINE))

	MCFG_DEVICE_ADD("keyboard", CIT101_HLE_KEYBOARD, 0)
	MCFG_CIT101_HLE_KEYBOARD_TXD_CALLBACK(WRITELINE("kbduart", i8251_device, write_rxd))

	MCFG_DEVICE_ADD("pit0", PIT8253, 0)
	MCFG_PIT8253_CLK0(6.144_MHz_XTAL / 4)
	MCFG_PIT8253_CLK1(6.144_MHz_XTAL / 4)
	MCFG_PIT8253_CLK2(6.144_MHz_XTAL / 4)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE("auxuart", i8251_device, write_txc))
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE("auxuart", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("pit1", PIT8253, 0)
	MCFG_PIT8253_CLK0(6.144_MHz_XTAL / 4)
	MCFG_PIT8253_CLK1(6.144_MHz_XTAL / 4)
	MCFG_PIT8253_CLK2(6.144_MHz_XTAL / 4)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE("comuart", i8251_device, write_txc))
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE("comuart", i8251_device, write_rxc))
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE("kbduart", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("kbduart", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("ppi", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(*this, cit101_state, nvr_address_w))
	MCFG_I8255_IN_PORTB_CB(READ8(*this, cit101_state, nvr_data_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(*this, cit101_state, nvr_data_w))
	MCFG_I8255_IN_PORTC_CB(READLINE("comm", rs232_port_device, cts_r)) MCFG_DEVCB_BIT(0)
	MCFG_DEVCB_CHAIN_INPUT(READLINE("comm", rs232_port_device, dcd_r)) MCFG_DEVCB_BIT(1) // tied to DSR for loopback test
	MCFG_DEVCB_CHAIN_INPUT(READLINE("comm", rs232_port_device, ri_r)) MCFG_DEVCB_BIT(2) // tied to CTS for loopback test
	MCFG_DEVCB_CHAIN_INPUT(READLINE("comm", rs232_port_device, si_r)) MCFG_DEVCB_BIT(3) // tied to CTS for loopback test
	MCFG_I8255_OUT_PORTC_CB(WRITE8(*this, cit101_state, nvr_control_w))

	MCFG_DEVICE_ADD("nvr", ER2055, 0)
MACHINE_CONFIG_END


// PCB ID: HAV-2P005B / CIT-101 / C. ITOH
// CPU: NEC D8085AC
// RAM: 12x NEC D416C-2 (16 positions labeled 8116E, including 4 unpopulated ones)
// Peripherals: 3x M5L8251AP-5 (2M, 7J, 7K); 2x NEC D8253C-2 (7I, 7L); NEC D8255AC-2 (7N); GI ER-2055 (8P)
// Oscillators: 6.144 (XTAL1), 14.976 (XTAL2), 22.464 (XTAL3)
ROM_START( cit101 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "ic1_=3g04=_v10a.bin", 0x0000, 0x1000, CRC(5601fcac) SHA1(cad0d0335d133dd43993bc718ff72d12b8445cd1) ) // TMM2732D-45
	ROM_LOAD( "ic2_=3h04=_v10a.bin", 0x1000, 0x1000, CRC(23d263e0) SHA1(586e8185f9804987e0a4081724c060e74769d41d) ) // TMM2732D-45
	ROM_LOAD( "ic3_=3i04=_v10a.bin", 0x2000, 0x1000, CRC(15994b1d) SHA1(6d125db4ef5e1dd4d5a4d2f4d6f6bdf574e5bad8) ) // TMM2732D-45
	ROM_LOAD( "ic4_=3j04=_v10a.bin", 0x3000, 0x0800, CRC(d786995f) SHA1(943b521dcc7abc0662d6e136169b7db480ae1e5c) ) // MB8516
	ROM_RELOAD(0x3800, 0x0800)

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "1h =5h 1 02= char rom.bin", 0x0000, 0x1000, CRC(ee0ff889) SHA1(a74ada19d19041b29e1b49aaf57ba7d9d54575e1) ) // TMM2332P

	ROM_REGION(0x180, "proms", 0)
	ROM_LOAD( "2i_=3a00=.bin", 0x000, 0x100, NO_DUMP ) // position labeled (MB)7052
	ROM_LOAD( "2f_=6g00=.bin", 0x100, 0x020, NO_DUMP ) // position labeled TBP18S030
	ROM_LOAD( "2e_=7i00=.bin", 0x120, 0x020, NO_DUMP ) // position labeled TBP18S030
	ROM_LOAD( "5d_=4l00=.bin", 0x140, 0x020, NO_DUMP ) // position labeled TBP18S030
	ROM_LOAD( "5g_=7f00=.bin", 0x160, 0x020, NO_DUMP ) // position labeled TBP18S030
ROM_END

COMP( 1980, cit101, 0, 0, cit101, cit101, cit101_state, 0, "C. Itoh Electronics", "CIT-101", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
