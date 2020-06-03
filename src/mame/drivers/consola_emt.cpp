// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Indra / Amper "Consola EMT"

    On-board ticketing console used on public transport buses on Madrid
    from 1997 to 2007.

    CPU Intel N80C188XL25 with a 50.0 HMz xtal near it.
    1 Oki MSM62X42B real time clock with built-in xtal
    4 x HY62810DA 128kx8bit CMOS SRAM
    MCU (for ticket pinter?) Intel N87C51FA (unprotected) with 11.0592 MHz xtal near it
    2 x Zilog Z85230 with a 4.9152 MHZ xtal near them
    5 x 8 dips banks:
    -FUNCIONES = Functions
    -L-NUMBUS  = Low / Bus number
    -H-NUMBUS  = High / Bus number
    -L-NUMFAB  = Low / Manufacturer number
    -H-NUMFAB  = High / Manufacturer number
    LCD display = PC2002LRU-LSO-H

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/msm6242.h"
#include "machine/z80scc.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class consoemt_state : public driver_device
{
public:
	consoemt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
        m_mcu(*this, "mcu"),
        m_lcdc(*this, "lcdc")
	{ }

	void consoemt(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
    required_device<mcs51_cpu_device> m_mcu;
    required_device<hd44780_device> m_lcdc;

    void mem_map(address_map &map);
	void io_map(address_map &map);

	void consoemt_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void consoemt_state::mem_map(address_map &map)
{
    map(0x00000, 0x7ffff).ram();
    map(0xc0000, 0xfffff).rom().region("maincpu", 0);
}

void consoemt_state::io_map(address_map &map)
{
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( consoemt )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void consoemt_state::consoemt_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 92,  83,  88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

HD44780_PIXEL_UPDATE( consoemt_state::lcd_pixel_update )
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 20)
		bitmap.pix16(1 + y + line*8 + line, 1 + pos*6 + x) = state ? 1 : 2;
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void consoemt_state::machine_start()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void consoemt_state::consoemt(machine_config &config)
{
	I80188(config, m_maincpu, 50_MHz_XTAL / 4); // divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &consoemt_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &consoemt_state::io_map);

	I80C51(config, m_mcu, 11.0592_MHz_XTAL);

    MSM6242(config, "rtc", XTAL(32'768));

    SCC85230(config, "uart1", 4.9152_MHz_XTAL);

    SCC85230(config, "uart2", 4.9152_MHz_XTAL);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(6*20+1, 19);
	screen.set_visarea_full();
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(consoemt_state::consoemt_palette), 3);

	KS0066_F05(config, m_lcdc, 0);
	m_lcdc->set_lcd_size(2, 20);
	m_lcdc->set_pixel_update_cb(FUNC(consoemt_state::lcd_pixel_update));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( consoemt )
    ROM_REGION(0x40000, "maincpu", 0)
    ROM_LOAD("pupitre_emt_24_04_03_6adc.ic1", 0x00000, 0x40000, CRC(fbafc173) SHA1(c0366a553125d42f18c24faa71467144eae42972))

    ROM_REGION(0x2000, "mcu", 0)
    ROM_LOAD("v26_7caa_n87c51fa.ic20", 0x0000, 0x2000, CRC(37e6c202) SHA1(7b240ed6474240090c26de11048a40c5870886dd))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY          FULLNAME       FLAGS
COMP( 1997, consoemt, 0,      0,      consoemt, consoemt, consoemt_state, empty_init, "Indra / Amper", "Consola EMT", MACHINE_IS_SKELETON )
