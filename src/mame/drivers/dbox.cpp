// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 * The Dbox-1 CPU board.
 *__________________________________________________________________________________________________________
 *                                                                                                          |
 *                                                                                                          |
 *                                                         _______________                                  |
 *                ___                                     |               |       ________    ________      |
 *               |   |                                    |_______________|      |        |  |        |     |
 *               |   |                                                           | FLASH  |  | FLASH  |     |
 *               |   |                                                           | 29F800 |  | 29F800 |     |
 *               |   |                                                           |        |  |        |     |
 *               |   |                 ____________                              |  1Mb   |  |  1Mb   |     |
 *               |   |                |            |                             |        |  |        |     |
 *               |___|                |   CPU32    |                             |        |  |        |     |
 *                                    | 68340PV16E |                             |________|  |________|     |
 *                                    |            |                                                        |
 *                                    |            |                                                        |
 *             _______________        |____________|                    _____                               |
 *            |               |                                        |     |                              |
 *            |  SCSI         |                                        |_____|         _____       _____    |
 *            |  CONTROLLER   |        +-+                              _____         |     |     |     |   |
 *            | LSI 53CF54-2  |        |X|                             |     |        |DRAM |     |DRAM |   |
 *            |               |        |T|                             |_____|        |42460|     |42460|   |
 *            |               |        |A|                                            |     |     |     |   |
 *            |               |        |L|                  ----------------          |512Kb|     |512Kb|   |
 *            |_______________|        +-+                  |              |          |     |     |     |   |
 *                                    3.6864                ----------------          |_____|     |_____|   |
 *                                                                                                          |
 *__________________________________________________________________________________________________________|
 *
 * History of Nokia Multimedia Division
 *-------------------------------------
 * Luxor AB was a swedish home electronics and computer manufacturer located in Motala from 1923 and aquired
 * by Nokia 1985. Luxor designed among other things TV setsm Radios and the famous ABC-80. The Nokia Multimedia 
 * Division was formed in Linköping as a result of the Luxor aquesition. Their main design was a satellite 
 * receiver, the first satellite in Europee was launched in 1988 and market was growing fast however it took
 * a long time, almost 10 years before the breakthrough came for Nokia, a deal with the Kirsch Gruppe was struck and 
 * in 1996 the 68340 based Dbox-1 was released in Germany. The original design was expensive, so soon a cost reduced 
 * version based on PPC, the Dbox-2, was released. The boxes sold in millions but the margins were negative or very 
 * low at best and the Kirsch Gruppe went bankrupt in 2002 and Nokia decided to shutdown the facility in Linköping.
 *
 * The heavily subsidiced Dbox was very popular in Holland since Kirsch Gruppe didn't lock use to themselfs. This was
 * corrected in a forced firmware upgrade leaving the "customers" in Holland without a working box. Pretty soon a
 * shareware software developed by Uli Hermann appeared called DVB98 and later DVB2000 reenabling the boxes in  Holland 
 * and blocking upgrades. Uli's software was by many considered better than the original software.
 *
 * Misc links about Nokia Multimedia Division and this board:
 * http://www.siliconinvestor.com/readmsg.aspx?msgid=5097482 
 * http://www.telecompaper.com/news/beta-research-publishes-dbox-specifications--163443
 * https://de.wikipedia.org/wiki/D-box
 * http://dvb2000.org/dvb2000/
 *
 * Misc findings
 *--------------
 * - Serial port on the back runs at 19200 at issues modem commands when attached to terminal
 * - It is possible to attach a BDM emulator and retrieve the ROM through it.
 * - It is possible to flash new firmware by adding jumper XP06 (under the modem board)
 * - The bootstrap is based on RTXC 3.2g RTOS 
 * - The bootstrap jumps to firmware from 0xb82 to RAM at 0x800000
 *
 * Identified chips/devices
 *-----------------
 * Motorola 68340 CPU
 * Philips SAA7124 Digital Video Encoder
 * LSI 53CF54-2 SCSI controller
 * Crystal CL4922-CL Mpeg Audio Decoder System
 * Rockwell R6653-16 Single Device Data/Fax Modem Data Pump
 * Philips 8020401TM100 <modem related, unknown purpose>
 * C-cube CL9100 MPEG2 decoder
 * Lucent AV6220A MPEG2 demultiplexer w. crypto interface
 * CI Common Interface module
 * LSI L2A0371 Tuner
 * 2 x 29F800-90 (2Mb FLASH)
 * 2 x 42260-60  (1Mb DRAM)
 * Siemens SDA5708 dot matrix display, SPI like connection 
 *  - http://arduinotehniq.blogspot.se/2015/07/sda5708-display-8-character-7x5-dot.html
 *  - charset stored at 0x808404 to 0x808780, 7 bytes per character
 * 
 *
 * Address Map
 * --------------------------------------------------------------------------
 * Address Range     Memory Space (physical)   Notes
 * --------------------------------------------------------------------------
 * 0xffffffff                                    (top of memory)
 * 0x00FFF780-0x00FFF7BF DMA                   offset to SIM40
 * 0x00FFF700-0x00FFF721 Serial devices        offset to SIM40
 * 0x00FFF600-0x00FFF67F Timers                offset to SIM40
 * 0x00FFF000-0x00FFF07F SIM40                 programmed base adress (MCR)
 * 0x00700000-0x008fffff RAM      
 * 0x00000000-0x0001ffff bootstrap 
 * --------------------------------------------------------------------------
 *
 * Init sequence
 * -------------
 *  MCR           : 0x6301     Timer/wd disabled, show cycles, ext arbit, 
 *                             user access to SIM40, IARB = 1
 *  MBAR          : 0x00FFF101 SIM40 base = 0x00fff000
 *  VBR           : 0x008096F8 VBR - Vector Base Register
 *  SIM40 + 0x0006: 0xE8       AVR - Auto Vector Register
 *  SIM40 + 0x0021: 0x0C       SWIV_SYPCR
 *  SIM40 + 0x001F: 0x00       PPARB - Port B pin assignment
 *  SIM40 + 0x0011: 0xFF       PORTA - Port A Data
 *  SIM40 + 0x0013: 0x16       DDRA  - Port A Data direction
 *  SIM40 + 0x0700: 0x00       Serial port
 *  SIM40 + 0x071F: 0xFF       Serial port
 *  SIM40 + 0x0004: 0x7F03     SYNCR
 *  SIM40 + 0x0040: 0x003FFFF5 CS0 mask 1
 *  SIM40 + 0x0044: 0x00000003 CS0 base 1
 *  SIM40 + 0x0048: 0x003FFFF5 CS1 mask 1
 *  SIM40 + 0x004C: 0x00800003 CS1 base 1
 *  SIM40 + 0x0050: 0x00007FFF CS2 mask 1
 *  SIM40 + 0x0054: 0x00700003 CS2 base 1
 *  SIM40 + 0x0058: 0x000007F2 CS3 mask 1
 *  SIM40 + 0x005C: 0x00700003 CS3 base 1
 *  SIM40 + 0x001F: 0x40       PBARB Port B Pin Assignment
 *
 *  -------------------------------------------------------------
 *  The bootstrap copies the firmware to RAM and jumps to it
 *  -------------------------------------------------------------
 *
 *  --- PC > 0x700000 so probably init of the RTXC tick timer setup?!
 *  SIM40 + 0x0022: 0x0140     PICR Periodic Interrupt Control Register
 *  SIM40 + 0x0024: 0x0029     PICR Periodic Interrupt Timer Register
 *
 *  Serial port setup
 * ------------------
 *  --- PC < 0x1FFFF so bootstrap code
 *  SIM40 + 0x0700: 0x00     Serial Module - MCR High Byte 
 *                            - The serial module is enabled
 *                            - ignore FREEZE
 *                            - The crystal clock is the clear-to-send input capture clock for both channels
 *  SIM40 + 0x071F: 0xFF     Serial Module - OUTPUT PORT (OP)4 BIT RESET - all cleared
 *  SIM40 + 0x0700: 0x00     Serial Module - MCR High Byte 
 *                            - The serial module is enabled
 *                            - ignore FREEZE
 *                            - The crystal clock is the clear-to-send input capture clock for both channels
 *  SIM40 + 0x0701: 0x8A     Serial Module - MCR Low Byte 
 *                            - The serial control regosters are only accessable from supervisor mode
 *                            - IARB = 0x0A - serial module has priority level 10d
 *  SIM40 + 0x0704: 0x01     Serial Module - ILR Interrupt Level
 *  SIM40 + 0x0705: 0x44     Serial Module - IVR Interrupt Vector
 *  SIM40 + 0x0714: 0x80     Serial Module - ACR Auxiliary Control
 *                                           - Set 2 of the available baud rates is selected.
 *                                           - CTS state change has no effect
 *  --- setup Channel A
 *  SIM40 + 0x0712: 0x20     Serial Module - CRA Command Register A
 *                                           - Reset the receiver
 *  SIM40 + 0x0712: 0x30     Serial Module - CRA Command Register A
 *                                           - Reset the transmitter
 *  SIM40 + 0x0710: 0x93     Serial Module - MR1A Mode Register 1A
 *                                           - Upon receipt of a valid start bit, RTS≈ is negated if the channel's FIFO is full.
 *                                             RTS≈ is reasserted when the FIFO has an empty position available.
 *                                           - No Parity
 *                                           - Eight bits
 *  SIM40 + 0x0720: 0x07     Serial Module - MR2A Mode Register 2A
 *                                           - No CTS or RTS controll
 *                                           - 1 STOP bit
 *  SIM40 + 0x0711: 0xCC     Serial Module - CSRA Clock Select Register A
 *                                           - 19200 baud TXc
 *                                           - 19200 baud Rxc
 *  SIM40 + 0x0712: 0x41     Serial Module - CRA Command Register A
 *                                           - Reset Error status
 *                                           - Enable Receiver
 *  - Check for charcters in channel A
 *  SIM40 + 0x0711: btst #0  Serial Module - SRA Status Register A
 *  --- if there is 
 *        store all characters in buffer at (A6) 0x88FFD0
 *  --- setup Channel B (See details as for channel A above)
 *  SIM40 + 0x071A: 0x20     Serial Module - CRB Command Register B
 *  SIM40 + 0x071A: 0x30     Serial Module - CRB Command Register B
 *  SIM40 + 0x0718: 0x93     Serial Module - MR1B Mode Register 1B
 *  SIM40 + 0x0721: 0x07     Serial Module - MR2B Mode Register 2B
 *  SIM40 + 0x0719: 0xCC     Serial Module - CSRB Clock Select Register B
 *  SIM40 + 0x071A: 0x41     Serial Module - CRB Command Register B
 *  - Check for characters in channel B
 *  SIM40 + 0x0719: btst #0  Serial Module - SRB Status Register B
 *  --- if there is 
 *        store all characters in buffer at (A6) 0x88FFD0
 *  --- 
 *  - Check bit 0 set on Input Port
 *  SIM40 + 0x071D: btst #0  Input Port - IP
 *  --- if bit 0 is set
 *      0x00801208: 0x80
 *  SIM40 + 0x071A: 0x81     Serial Module - CRB Command Register B
 *  --- 
 *  SIM40 + 0x0720: 0x41     Serial Module - MR2A Mode register 2A 
 *  SIM40 + 0x071D: 0x03     OPCR Output Port Control Register
 *  SIM40 + 0x0715: 0x03     IER Interrupt Enable Register
 *
 *  Timer setup
 * ------------
 *  SIM40 + 0x0640: 0x03     MCR Timer 2
 *  tbc...
 *
 *  -------------------------------------------------------------
 *  The bootstrap copies the firmware to RAM and jumps to it
 *  -------------------------------------------------------------
 *
 *  --- PC > 0x700000 so probably init of the RTXC tick timer setup?!
 *  SIM40 + 0x0022: 0x0140     PICR Periodic Interrupt Control Register
 *  SIM40 + 0x0024: 0x0029     PICR Periodic Interrupt Timer Register
 *
 *  Serial port setup
 * ------------------
 *  --- PC < 0x1FFFF so bootstrap code
 *  SIM40 + 0x0700: 0x00     Serial Module - MCR High Byte 
 *                            - The serial module is enabled
 *                            - ignore FREEZE
 *                            - The crystal clock is the clear-to-send input capture clock for both channels
 *  SIM40 + 0x071F: 0xFF     Serial Module - OUTPUT PORT (OP)4 BIT RESET - all cleared
 *  SIM40 + 0x0700: 0x00     Serial Module - MCR High Byte 
 *                            - The serial module is enabled
 *                            - ignore FREEZE
 *                            - The crystal clock is the clear-to-send input capture clock for both channels
 *  SIM40 + 0x0701: 0x8A     Serial Module - MCR Low Byte 
 *                            - The serial control regosters are only accessable from supervisor mode
 *                            - IARB = 0x0A - serial module has priority level 10d
 *  SIM40 + 0x0704: 0x01     Serial Module - ILR Interrupt Level
 *  SIM40 + 0x0705: 0x44     Serial Module - IVR Interrupt Vector
 *  SIM40 + 0x0714: 0x80     Serial Module - ACR Auxiliary Control
 *                                           - Set 2 of the available baud rates is selected.
 *                                           - CTS state change has no effect
 *  --- setup Channel A
 *  SIM40 + 0x0712: 0x20     Serial Module - CRA Command Register A
 *                                           - Reset the receiver
 *  SIM40 + 0x0712: 0x30     Serial Module - CRA Command Register A
 *                                           - Reset the transmitter
 *  SIM40 + 0x0710: 0x93     Serial Module - MR1A Mode Register 1A
 *                                           - Upon receipt of a valid start bit, RTS≈ is negated if the channel's FIFO is full.
 *                                             RTS≈ is reasserted when the FIFO has an empty position available.
 *                                           - No Parity
 *                                           - Eight bits
 *  SIM40 + 0x0720: 0x07     Serial Module - MR2A Mode Register 2A
 *                                           - No CTS or RTS controll
 *                                           - 1 STOP bit
 *  SIM40 + 0x0711: 0xCC     Serial Module - CSRA Clock Select Register A
 *                                           - 19200 baud TXc
 *                                           - 19200 baud Rxc
 *  SIM40 + 0x0712: 0x41     Serial Module - CRA Command Register A
 *                                           - Reset Error status
 *                                           - Enable Receiver
 *  - Check for charcters in channel A
 *  SIM40 + 0x0711: btst #0  Serial Module - SRA Status Register A
 *  --- if there is 
 *        store all characters in buffer at (A6) 0x88FFD0
 *  --- setup Channel B (See details as for channel A above)
 *  SIM40 + 0x071A: 0x20     Serial Module - CRB Command Register B
 *  SIM40 + 0x071A: 0x30     Serial Module - CRB Command Register B
 *  SIM40 + 0x0718: 0x93     Serial Module - MR1B Mode Register 1B
 *  SIM40 + 0x0721: 0x07     Serial Module - MR2B Mode Register 2B
 *  SIM40 + 0x0719: 0xCC     Serial Module - CSRB Clock Select Register B
 *  SIM40 + 0x071A: 0x41     Serial Module - CRB Command Register B
 *  - Check for characters in channel B
 *  SIM40 + 0x0719: btst #0  Serial Module - SRB Status Register B
 *  --- if there is 
 *        store all characters in buffer at (A6) 0x88FFD0
 *  --- 
 *  - Check bit 0 set on Input Port
 *  SIM40 + 0x071D: btst #0  Input Port - IP
 *  --- if bit 0 is set
 *      0x00801208: 0x80
 *  SIM40 + 0x071A: 0x81     Serial Module - CRB Command Register B
 *  --- 
 *  SIM40 + 0x0720: 0x41     Serial Module - MR2A Mode register 2A 
 *  SIM40 + 0x071D: 0x03     OPCR Output Port Control Register
 *  SIM40 + 0x0715: 0x03     IER Interrupt Enable Register
 *
 *  Timer setup
 * ------------
 *  SIM40 + 0x0640: 0x03     MCR Timer 2
 *  tbc...
 *
 *  Identified low level drivers in firmware
 *  ----------------------------------------
 *  800420..80046C : Some PORT A serialisation routine for the 
 *                   Siemens SDA5708 dot matrix display
 *
 * Interrupt sources
 * ----------------------------------------------------------
 * Description               Device  Lvl  IRQ
 *                           /Board      Vector 
 * ----------------------------------------------------------
 * On board Sources                       
 *  
 * Off board Sources (other boards)
 * ----------------------------------------------------------
 *
 * DMAC Channel Assignments
 * ----------------------------------------------------------
 * Channel         Device
 * ----------------------------------------------------------
 * ----------------------------------------------------------
 *
 *  TODO:
 *  - Dump the ROMs
 *  - Dump/Find bootable software
 *  - Setup a working address map
 *  - Fix debug terminal
 *  - TBC
 *
 ****************************************************************************/

#include "emu.h"
#include "machine/68340.h"

class dbox_state : public driver_device
{
 public:
 dbox_state(const machine_config &mconfig, device_type type, const char *tag)
	 : driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"){ }
	required_device<m68340_cpu_device> m_maincpu;

	virtual void machine_reset() override;
	DECLARE_DRIVER_INIT(dbox);
};

void dbox_state::machine_reset()
{
}


static ADDRESS_MAP_START( dbox_map, AS_PROGRAM, 32, dbox_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x700000, 0x8fffff) AM_RAM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dbox )
INPUT_PORTS_END

static MACHINE_CONFIG_START( dbox )
	MCFG_CPU_ADD("maincpu", M68340, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(dbox_map)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(dbox_state, dbox)
{
}

ROM_START( dbox )
	ROM_REGION(0x1000000, "maincpu", 0)
//	ROM_LOAD16_WORD( "dvb2000.bin", 0x000000, 0x8b742, CRC(5b21c455) SHA1(1e7654c37dfa65d1b8ac2469cdda82f91b47b3c7) )
	ROM_LOAD16_WORD( "nokboot.bin", 0x000000, 0x20000, CRC(0ff53e1f) SHA1(52002ee22c032775dac383d408c44abe9244724f) )
ROM_END

COMP( 1996, dbox,	0, 0, dbox, dbox, dbox_state, dbox, "Nokia Multimedia", "D-box 1, Kirsch gruppe", MACHINE_IS_SKELETON )
