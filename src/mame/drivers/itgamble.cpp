// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************

  Nazionale Elettronica + others (mostly Italian) Gambling games
  mostly based on H8/3048 + OKI 6295 or similar.
.
  These all use MCUs with internal ROM for their programs,
  they can't be dumped easily, and thus we can't emulate
  them at the moment because there is nothing to emulate

  This driver is just a placeholder for the graphic / sound
  ROM loading

*******************************************************************

  --- Hardware Notes ---

There are at least 3 viariants for this hardware.

---------  
The oldest revision (green) is marked H83048 with the following hardware:
(note that @IC2 is unpopulated)
  CPU:
    1x H8/3048 (HD64F3048F16)            @IC1
    (128KB ROM; 4KB RAM)
  Sound:
    1x OKI 6295 or clones                @IC24
	1x LM358N dual operational amplifier @IC27
    1x TDA2003 audio amplifier           @IC26
  PLDs:
    1x ispLSI2064-80LJ                   @IC12
  Clock:
    1x Xtal 30.000 MHz                   @OSC1
    1x Resonator ZTB1000J (1000 kHz)     @X1
  ROMs:
    1x 27C010 or 27C020 (sound)          @IC25
    2x or 4x 27C040 (graphics)           @IC17,18,19,20
  RAMs:
    2x SRAM 32k x 8                      @IC13,14
  Connectors:
    1x 28x2 edge connector
    1x 12 legs connector                 @CN1
    1x 50 legs flat cable connector      @CN4
  Other:
    1x battery                           @BAT1
    1x 12 DIP switches bank              @CN2,3
    2x trimmer (VOLUME, SPARK)           @P1,2

Known games on this hardware revision are:
200x        Book Theatre (ver 1.2)
2000.04.12  Capitan Uncino (Ver 1.2)
2001.02     Capitani Coraggiosi (Ver 1.3)

---------
A slightly newer revision (green) is marked H83048 and adds a very small piggyback @IC2 with:

  Timekeeping: 1x Dallas DS1302 Trickle Charge Timekeeping Chip
  Clock:       1x Xtal 32.768
  A jumper cable connects the piggyback with ICL7673 pin2 @IC31 Automatic battery back-up switch

Known games on this hardware revision are:
2001        Europa 2002 (Ver 2.0, set 1)
2001        Europa 2002 (Ver 2.0, set 2)
2002.01     La Perla Nera (Ver 2.0)
2001.11     La Perla Nera Gold (Ver 2.0)
2001.06     Labyrinth (Versione 1.5)
200x        Pin Ups (Ver 1.0 Rev.A)
2001.12     World Cup (Ver 1.4) - note ICs location are differently numbered
2001        World Cup (Ver 1.5) - note RAMS are smaller SRAM 8k x 8


---------
A more recent revision (red) is marked H83048 Rev 1.1 and removes the piggyback putting Timekeeping and Clock (X2) directly on the board.
  A jumper cable on the solder side connects Timekeeping pin5 to unpopulated IC2 pin11
		 
Known games on this hardware revision are:
2001.08.24  Abacus (Ver 1.0)
2002        Europa 2002 Space (Ver 3.0)
2002.12     UFO Robot (Ver. 1.0 Rev.A)


*******************************************************************/

#include "emu.h"
#include "cpu/h8/h83048.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define MAIN_CLOCK  XTAL(30'000'000)
#define SND_CLOCK   XTAL(1'000'000)

#define MNUMBER_MAIN_CLOCK  XTAL(24'000'000)
#define MNUMBER_SND_CLOCK   XTAL(16'000'000)

#define EJOLLYX5_MAIN_CLOCK XTAL(16'000'000)


class itgamble_state : public driver_device
{
public:
	itgamble_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_palette(*this, "palette")
	{ }

	void mnumber(machine_config &config);
	void itgamble(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void itgamble_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

	// driver_device overrides
	virtual void machine_reset() override;
	virtual void video_start() override;
};


/*************************
*     Video Hardware     *
*************************/

void itgamble_state::video_start()
{
}

uint32_t itgamble_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen());
	return 0;
}


/*************************
* Memory map information *
*************************/

void itgamble_state::itgamble_map(address_map &map)
{
	map.global_mask(0xffffff);
	map(0x000000, 0xffffff).rom();
}


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( itgamble )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout gfxlayout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( gfx_itgamble )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout_8x8x8,   0, 16  )
GFXDECODE_END


/**************************
*      Machine Reset      *
**************************/

void itgamble_state::machine_reset()
{
	/* stop the CPU, we have no code for it anyway */
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}


/**************************
*     Machine Drivers     *
**************************/

MACHINE_CONFIG_START(itgamble_state::itgamble)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", H83048, MAIN_CLOCK/2)
	MCFG_DEVICE_PROGRAM_MAP(itgamble_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(itgamble_state, screen_update)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_itgamble)
	MCFG_PALETTE_ADD("palette", 0x200)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("oki", OKIM6295, SND_CLOCK, okim6295_device::PIN7_HIGH) /* 1MHz resonator */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(itgamble_state::mnumber)
	itgamble(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(MNUMBER_MAIN_CLOCK/2)    /* probably the wrong CPU */

	MCFG_DEVICE_REPLACE("oki", OKIM6295, MNUMBER_SND_CLOCK/16, okim6295_device::PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


#ifdef UNUSED_CODE
static MACHINE_CONFIG_START( ejollyx5 )
	itgamble(config);
	/* wrong CPU. we need a Renesas M16/62A 16bit microcomputer core */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(EJOLLYX5_MAIN_CLOCK/2)   /* up to 10MHz.*/

	MCFG_DEVICE_REPLACE("oki", OKIM6295, MNUMBER_SND_CLOCK/16, okim6295_device::PIN7_HIGH) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
#endif


/*************************
*        Rom Load        *
*************************/

/* Book Theatre (ver 1.2)
PCB is marked: "CE H83048" on component side
PCB is marked: "H83048 bottom" on solder side
PCB is labeled: "BOOK THEATER Vers. 1.2" on component side
*/
ROM_START( bookthr )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "bookthr_ver1.2_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 ) //bigger than 8bpps?
	ROM_LOAD( "2.ic18", 0x000000, 0x80000, CRC(39433a74) SHA1(088944bfb43b4f239f22d0d2213efd19cea7db30) )
	ROM_LOAD( "3.ic17", 0x080000, 0x80000, CRC(893abdcc) SHA1(4dd28fd46bec8be5549d679d31c771888fcb1286) )

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1.ic25", 0x00000, 0x40000, CRC(4fe79e43) SHA1(7c154cb00e9b64fbdcc218280f2183b816cef20b) ) //same as Abacus
ROM_END

/* Capitan Uncino (Ver 1.2)
PCB is marked: "CE H83048" on component side
PCB is marked: "ET5" and "H83048 bottom" on solder side
PCB is labeled: "Capitan Uncino Vers. 1.3" and " PASSED 12/04/00" on component side
--
PCB is labeled Ver 1.3, while EPROMs are labeled Ver 1.2
*/
ROM_START( capunc )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "capunc.ver1.2.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "2.ver.1.2.ic18", 0x000000, 0x80000, CRC(5030f7eb) SHA1(106b61c84e3647b8d68d6c30ee7e63ec2df1f5fd) )
	ROM_LOAD( "3.ver.1.2.ic17", 0x080000, 0x80000, CRC(2b50e312) SHA1(dc901540a5e1a25fe6e7deb58b0fe01f116aaf63) )

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1.ic25", 0x00000, 0x40000, CRC(4fe79e43) SHA1(7c154cb00e9b64fbdcc218280f2183b816cef20b) )
ROM_END

/* Capitani Coraggiosi (Ver 1.3)
PCB is marked: "CE H83048" on component side
PCB is marked: "H83048 bottom" on solder side
PCB is labeled: "Capitani Coraggiosi Vers. 1.3" and "PASSED 02/2001" on component side
*/
ROM_START( capcor )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "capcor.ver1.3.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "2.ic18", 0x000000, 0x80000, CRC(342bea85) SHA1(885080a9b55d64f9a93e3d5e31e6b13f272bdb93) )
	ROM_LOAD( "3.ic17", 0x080000, 0x80000, CRC(ac530eff) SHA1(7c3a6e322311a1cd93801639a0498d5947fb14f2) )

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1.ic25", 0x00000, 0x40000, CRC(4fe79e43) SHA1(7c154cb00e9b64fbdcc218280f2183b816cef20b) )
ROM_END

/* Europa 2002 (Ver 2.0, set 1)
PCB is marked: "CE H83048" on component side
PCB is marked: "H83048 bottom" on solder side
PCB is labeled: "EUROPA 2002 Versione 2_0" and "Non rimuovere PASSED 11/2001 Garanzia 6 MESI" on component side
*/
ROM_START( euro2k2 )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "euro2k2_ver2.0_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "4a.ic18", 0x000000, 0x80000, CRC(5decae2d) SHA1(d918aad0e2a1249b18677833f743c92fb678050a) )
	ROM_LOAD( "5a.ic17", 0x080000, 0x80000, CRC(8f1bbbf3) SHA1(5efcf77674f8737fc1b98881acebacb26b10adc1) )
	ROM_LOAD( "2a.ic20", 0x100000, 0x40000, CRC(f9bffb07) SHA1(efba175189d99a4548739a72f8a1f03c2782a3d0) )
	ROM_LOAD( "3a.ic19", 0x140000, 0x40000, CRC(8b29cd56) SHA1(8a09e307271bceef6e9f863153d0f7a9bc6dc6bd) )

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1.ic25", 0x00000, 0x40000, CRC(b9b1aff0) SHA1(35622d7d099a10e5c6bcae152fded1f50692f740) )
ROM_END

/* Europa 2002 (Ver 2.0, set 2)
PCB is marked: "CE H83048" on component side
PCB is marked: "H83048 bottom" on solder side
PCB is labeled: "EUROPA 2002 Versione 2_0" and "PASSED 10/2001" on component side
*/
ROM_START( euro2k2a )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "euro2k2a_ver2.0_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x1c0000, "gfx1", 0 )
	ROM_LOAD( "4a.ic18", 0x000000, 0x80000, CRC(5decae2d) SHA1(d918aad0e2a1249b18677833f743c92fb678050a) )
	ROM_LOAD( "5a.ic17", 0x080000, 0x80000, CRC(8f1bbbf3) SHA1(5efcf77674f8737fc1b98881acebacb26b10adc1) )
	ROM_LOAD( "2a.ic20", 0x100000, 0x40000, CRC(f9bffb07) SHA1(efba175189d99a4548739a72f8a1f03c2782a3d0) )
	ROM_LOAD( "3a.ic19", 0x140000, 0x80000, CRC(56c8a73d) SHA1(49b44e5604cd8675d8f9770e5fb68dad4394e11d) ) /* identical halves */ // sldh

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1.ic25", 0x00000, 0x40000, CRC(4fe79e43) SHA1(7c154cb00e9b64fbdcc218280f2183b816cef20b) ) // sldh
ROM_END

/* La Perla Nera (Ver 2.0)
PCB is marked: "CE H83048" on component side
PCB is marked: "H83048 bottom" on solder side
PCB is labeled: "LA PERLA NERA Versione 2.0" and "Non Rimuovere PASSED 01/2002 garanzia 6 MESI" on component side
*/
ROM_START( laperla )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "laperla_ver2.0_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "2jolly.ic18", 0x000000, 0x80000, CRC(7bf3d5f2) SHA1(f3a51dd642358a20f6324f28fdf458e8ceaca7a1) )
	ROM_LOAD( "3jolly.ic17", 0x080000, 0x80000, CRC(c3a8d9a0) SHA1(cc95c56ebc6137e11c82ed17be7c9f83ed7b6cfc) )

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1.ic25", 0x00000, 0x40000, CRC(4fe79e43) SHA1(7c154cb00e9b64fbdcc218280f2183b816cef20b) )
ROM_END


/* La Perla Nera Gold (Ver 2.0)
PCB is marked: "CE H83048" on component side
PCB is marked: "H83048 bottom" on solder side
PCB is labeled: "LA PERLA NERA GOLD Versione 2.0" and "Non Rimuovere PASSED 11/2001 garanzia 6 MESI" on component side
*/
ROM_START( laperlag )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "laperlag_ver2.0_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ic18-laperlaneragold2.bin", 0x000000, 0x80000, CRC(ae37de44) SHA1(089f97678fa39aee1885d7c63c4bc7c88e7fe553) )
	ROM_LOAD( "ic17-laperlaneragold3.bin", 0x080000, 0x80000, CRC(86da6d11) SHA1(e6b7f9ccbf2e91a60fdf38067ec7ac7e73dea8cd) )

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "ic25-uno.bin", 0x00000, 0x20000, CRC(e6a0854b) SHA1(394e01bb24abd1e0d2c447b4d620fc5d02257d8a) )
ROM_END

/* World Cup (Ver 1.5)
PCB is marked: "CE H83048" on component side
PCB is marked: "H83048 bottom" on solder side
PCB is labeled: "WORLD CUP Versione 1.5" on component side
---
It is the same game as World Cup (Ver 1.4) but with less RAM
*/
ROM_START( wcup )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "wcup_ver1.5_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 ) //bigger than 8bpps?
	ROM_LOAD( "world cup 2.ic18", 0x000000, 0x80000, CRC(4524445b) SHA1(50ec31ac9e4cd807fd4bf3d667644ed662681782) )
	ROM_LOAD( "world cup 3.ic17", 0x080000, 0x80000, CRC(0df1af40) SHA1(f5050533e5a9cf2113e5aeffaeca23c7572cafae) )

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1.ic25", 0x00000, 0x20000, CRC(e6a0854b) SHA1(394e01bb24abd1e0d2c447b4d620fc5d02257d8a) ) // same as laperlag
ROM_END

/* World Cup (Ver 1.4)
PCB is marked: "CE ND2001" on component side
PCB is marked: "ND2001 Rev. 1.0" and "bottom" on solder side
PCB is labeled: "WORLD CUP Versione 1.4" and "Non Rimuovere PASSED 12/2001 Garanzia 6 MESI" on component side
---
It is the same game as World Cup (Ver 1.5) but ICs location are numbered differently due to a different PCB layout
*/

ROM_START( wcup14 )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "wcup_ver1.4_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 ) //bigger than 8bpps?
	ROM_LOAD( "world cup 2.ic18", 0x000000, 0x80000, CRC(4524445b) SHA1(50ec31ac9e4cd807fd4bf3d667644ed662681782) )
	ROM_LOAD( "world cup 3.ic17", 0x080000, 0x80000, CRC(0df1af40) SHA1(f5050533e5a9cf2113e5aeffaeca23c7572cafae) )

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1.ic25", 0x00000, 0x20000, CRC(e6a0854b) SHA1(394e01bb24abd1e0d2c447b4d620fc5d02257d8a) ) // same as laperlag
ROM_END

/* Abacus (Ver 1.0)
PCB is marked: "CE H83048" on component side
PCB is marked: "bottom" and "H83048 Rev. 1.1" on solder side
PCB is labeled: "ABACUS Vers. 1.0" and "FR 24.08.01" on component side
*/
ROM_START( abacus )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "abacus_ver1.0_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 ) //bigger than 8bpps?
	ROM_LOAD( "abacus2.ic18", 0x000000, 0x80000, CRC(9884ee09) SHA1(85875dbcd6821c8173457df0216145b4208d5c06) )
	ROM_LOAD( "abacus3.ic17", 0x080000, 0x80000, CRC(ec6473c4) SHA1(49980b94ccf77fbfdaa151fccaeb3c2ddad3c119) )

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1.ic25", 0x00000, 0x40000, CRC(4fe79e43) SHA1(7c154cb00e9b64fbdcc218280f2183b816cef20b) )
ROM_END


/* Europa 2002 Space (Ver 3.0)
PCB is marked: "CE H83048" on component side
PCB is marked: "bottom" and "H83048 Rev. 1.1" on solder side
PCB is labeled: "EUROPA 2002 SPACE Ver. 3.0" and "Non rimuovere PASSED 04/2002 Garanzia 6 MESI" on component side
*/
ROM_START( euro2k2s )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* all the program code is in here */
	ROM_LOAD( "euro2k2s_ver3.0_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "europa2002space4.ic18", 0x000000, 0x80000, CRC(cf4db4f1) SHA1(6c03e54e30eb83778d1cad5ade17c26a370ea8a3) )
	ROM_LOAD( "europa2002space5.ic17", 0x080000, 0x80000, CRC(1070b4ac) SHA1(3492de52cd0c784479d2774f6050b24cf4591484) )
	ROM_LOAD( "europa2002_2-a.ic20",   0x100000, 0x40000, CRC(971bc33b) SHA1(c385e5bef57cdb52a86c1e38fca471ef5ab3da7c) )
	ROM_LOAD( "europa2002space3.ic19", 0x140000, 0x40000, CRC(d82dba04) SHA1(63d407dd036d3c7f190ad7b6d694288e9a9e56d0) ) /* identical halves */

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "1-a.ic25", 0x00000, 0x20000, CRC(8fcb283d) SHA1(9e95c72967da13606eed6d16f84145273b9ffddf) )
ROM_END

/********** DIFFERENT HARDWARE **********/


/* Mystery Number

CPU:

1x HD64F3048F16 (main)(u2)
3x XC9572 (u29,u33,u34)
1x M6295 (u5)(sound)
1x oscillator 24.000 MHz.
1x oscillator 16.000 MHz.

ROMs:

4x M27C4001 (1,2,3,4)(main)
1x AM27C020 (5)(sound)

Note:

1x JAMMA edge connector
1x 8 legs jumper (jp1)
1x battery
1x 8x2 DIP switches
1x trimmer (volume)

*/

ROM_START( mnumber )    /* clocks should be changed for this game */
	ROM_REGION( 0x1000000, "maincpu", 0 )   /* all the program code is in here */
	ROM_LOAD( "mnumber_hd64f3048f16.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* different encoded gfx */
	ROM_LOAD( "mysterynumber3.u20", 0x000000, 0x80000, CRC(251f1e11) SHA1(e8c90b289e76cea6a541b701859be6465a381668) )
	ROM_LOAD( "mysterynumber4.u21", 0x080000, 0x80000, CRC(2b8744e4) SHA1(8a12c6f300818de3738e7c44c7df71c432cb9975) )
	ROM_LOAD( "mysterynumber1.u22", 0x100000, 0x80000, CRC(d2ce1f61) SHA1(8f30407050fc102191747996258d4b5da3a0d994) )
	ROM_LOAD( "mysterynumber2.u19", 0x180000, 0x80000, CRC(7b3a3b32) SHA1(9db46aa12077a48951056705491da1cce747c374) ) /* identical halves */

	ROM_REGION( 0x40000, "oki", 0 ) /* M6295 samples */
	ROM_LOAD( "mysterynumber5.u6", 0x00000, 0x40000, CRC(80aba466) SHA1(e9bf7e1c3d1c6b1b0dba43dd79a71f89e63df814) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME      PARENT   MACHINE   INPUT     STATE           INIT        ROT   COMPANY                  FULLNAME                         FLAGS
/* hardware green H83048*/
GAME( 200?, bookthr,  0,       itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "Book Theatre (Ver 1.2)",        MACHINE_IS_SKELETON )
GAME( 2000, capunc,   0,       itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "Capitan Uncino (Ver 1.2)",      MACHINE_IS_SKELETON )
GAME( 2001, capcor,   0,       itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "Capitani Coraggiosi (Ver 1.3)", MACHINE_IS_SKELETON )

/* hardware green H83048 + piggyback for timekeeping*/
GAME( 2001, euro2k2,  0,       itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "Europa 2002 (Ver 2.0, set 1)",  MACHINE_IS_SKELETON )
GAME( 2001, euro2k2a, euro2k2, itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "Europa 2002 (Ver 2.0, set 2)",  MACHINE_IS_SKELETON )
GAME( 2002, laperla,  0,       itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "La Perla Nera (Ver 2.0)",       MACHINE_IS_SKELETON )
GAME( 2001, laperlag, 0,       itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "La Perla Nera Gold (Ver 2.0)",  MACHINE_IS_SKELETON )
GAME( 2001, wcup,     0,       itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "World Cup (Ver 1.5)",           MACHINE_IS_SKELETON )
GAME( 2001, wcup14,   wcup,    itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "World Cup (Ver 1.4)",           MACHINE_IS_SKELETON )

/* hardware red H83048 Rev 1.1 + timekeeping on board*/
GAME( 2001, abacus,   0,       itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "Abacus (Ver 1.0)",              MACHINE_IS_SKELETON )
GAME( 2002, euro2k2s, euro2k2, itgamble, itgamble, itgamble_state, empty_init, ROT0, "Nazionale Elettronica", "Europa 2002 Space (Ver 3.0)",   MACHINE_IS_SKELETON )

/* different hardware */
GAME( 2000, mnumber,  0,       mnumber,  itgamble, itgamble_state, empty_init, ROT0, "MM / BRL Bologna",      "Mystery Number",                MACHINE_IS_SKELETON )
