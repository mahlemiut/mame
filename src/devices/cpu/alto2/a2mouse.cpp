// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII mouse interface
 *
 *****************************************************************************/
#include "alto2cpu.h"
#include "a2roms.h"

enum {
	MX1     = (1<<0),       //!< MX1 signal is bit 0 (latch bit 1)
	LMX1    = (1<<1),
	MX2     = (1<<2),       //!< MX2 signal is bit 2 (latch bit 3)
	LMX2    = (1<<3),
	MY1     = (1<<4),       //!< MY1 signal is bit 4 (latch bit 5)
	LMY1    = (1<<5),
	MY2     = (1<<6),       //!< MY2 signal is bit 6 (latch bit 7)
	LMY2    = (1<<7),
	MACTIVE = (MX1|MX2|MY1|MY2),    //!< mask for the active bits
	MLATCH  = (LMX1|LMX2|LMY1|LMY2) //!< mask for the latched bits
};

/**
 * <PRE>
 * The mouse inputs from the shutters are connected to a quad
 * 2/3 input RS flip flop (SN74279).
 *
 *          74279
 *       +---+--+---+
 *       |   +--+   |
 *   R1 -|1       16|- Vcc
 *       |          |
 *  S1a -|2       15|- S4
 *       |          |
 *  S1b -|3       14|- R4
 *       |          |
 *   Q1 -|4       13|- Q4
 *       |          |
 *   R2 -|5       12|- S3a
 *       |          |
 *   S2 -|6       11|- S3b
 *       |          |
 *   Q2 -|7       10|- R3
 *       |          |
 *  GND -|8        9|- Q3
 *       |          |
 *       +----------+
 *
 * The 'Y' Encoder signals are connected to IC1:
 *  shutter pin(s)  R/S     output
 *  ------------------------------------
 *  0       2,3     S1a,b   Q1 MX2 -> 1
 *  1       1       R1      Q1 MX2 -> 0
 *  2       5       R2      Q2 MX1 -> 0
 *  3       6       S2      Q2 MX1 -> 1
 *
 * The 'X' Encoder signals are connected to IC2:
 *  shutter pin(s)  R/S     output
 *  ------------------------------------
 *  0       2,3     S1a,b   Q1 MY2 -> 1
 *  1       1       R1      Q1 MY2 -> 0
 *  2       5       R2      Q2 MY1 -> 0
 *  3       6       S2      Q2 MY1 -> 1
 *
 *
 * The pulse train generated by a left or up rotation is:
 *
 *             +---+   +---+   +---+
 * MX1/MY1     |   |   |   |   |   |
 *          ---+   +---+   +---+   +---
 *
 *           +---+   +---+   +---+   +-
 * MX2/MY2   |   |   |   |   |   |   |
 *          -+   +---+   +---+   +---+
 *
 *
 * The pulse train generated by a right or down rotation is:
 *
 *           +---+   +---+   +---+   +-
 * MX1/MY1   |   |   |   |   |   |   |
 *          -+   +---+   +---+   +---+
 *
 *             +---+   +---+   +---+
 * MX2/MY2     |   |   |   |   |   |
 *          ---+   +---+   +---+   +---
 *
 * In order to simulate the shutter sequence for the mouse motions
 * we have to generate a sequence of pulses on MX1/MX2 and MY1/MY2
 * that have their phases shifted by 90 degree.
 * </PRE>
 */


#define MOVEX(x) ((((x) < 0) ? MY2 : ((x) > 0) ? MY1 : 0))
#define MOVEY(y) ((((y) < 0) ? MX2 : ((y) > 0) ? MX1 : 0))
#define SIGN(a) ((a) < 0 ? -1 : (a) > 0 ? 1 : 0)

/**
 * @brief return the mouse motion flags
 *
 * Advance the mouse x and y coordinates to the dx and dy
 * coordinates by either toggling MX2 or MX1 first for a
 * y movement, or MY2 or MY1 for x movement.
 * There are four read phases counted by m_mouse.phase
 *
 * @return lookup value from madr_a32
 */
UINT16 alto2_cpu_device::mouse_read()
{
	UINT16 data;

	m_mouse.latch = (m_mouse.latch << 1) & MLATCH;
	data = m_madr_a32[m_mouse.latch];

	switch (m_mouse.phase) {
	case 0:
		m_mouse.latch ^= MOVEX(m_mouse.dx - m_mouse.x);
		m_mouse.latch ^= MOVEY(m_mouse.dy - m_mouse.y);
		break;
	case 1:
		m_mouse.latch |= MACTIVE;
		m_mouse.x += SIGN(m_mouse.dx - m_mouse.x);
		m_mouse.y += SIGN(m_mouse.dy - m_mouse.y);
		break;
	case 2:
		m_mouse.latch ^= MOVEX(m_mouse.dx - m_mouse.x);
		m_mouse.latch ^= MOVEY(m_mouse.dy - m_mouse.y);
		break;
	case 3:
		m_mouse.latch |= MACTIVE;
		m_mouse.x += SIGN(m_mouse.dx - m_mouse.x);
		m_mouse.y += SIGN(m_mouse.dy - m_mouse.y);
	}
	m_mouse.phase = (m_mouse.phase + 1) % 4;
	return data;
}

/**
 * @brief register a mouse motion in x direction
 * @param ioport_field reference to the field
 * @param param pointer passed in PORT_CHANGED_MEMBER last parameter
 * @param oldval the old ioport_value
 * @param newval the new ioport_value
 */
INPUT_CHANGED_MEMBER( alto2_cpu_device::mouse_motion_x )
{
	INT16 ox = static_cast<INT16>(oldval);
	INT16 nx = static_cast<INT16>(newval);
	m_mouse.dx = std::min(std::max(0, m_mouse.dx + (nx - ox)), 639);
}

/**
 * @brief register a mouse motion in y direction
 * @param ioport_field reference to the field
 * @param param pointer passed in PORT_CHANGED_MEMBER last parameter
 * @param oldval the old ioport_value
 * @param newval the new ioport_value
 */
INPUT_CHANGED_MEMBER( alto2_cpu_device::mouse_motion_y )
{
	INT16 oy = static_cast<INT16>(oldval);
	INT16 ny = static_cast<INT16>(newval);
	m_mouse.dy = std::min(std::max(0, m_mouse.dy + (ny - oy)), 824);
}

/**
 * @brief register a mouse button change
 *
 * convert button bit to UTILIN[13-15]
 *
 * @param ioport_field reference to the field
 * @param param pointer passed in PORT_CHANGED_MEMBER last parameter
 * @param oldval the old ioport_value
 * @param newval the new ioport_value
 */
INPUT_CHANGED_MEMBER( alto2_cpu_device::mouse_button_0 )
{
	X_WRBITS(m_hw.utilin,16,13,13,newval);
}

INPUT_CHANGED_MEMBER( alto2_cpu_device::mouse_button_1 )
{
	X_WRBITS(m_hw.utilin,16,14,14,newval);
}

INPUT_CHANGED_MEMBER( alto2_cpu_device::mouse_button_2 )
{
	X_WRBITS(m_hw.utilin,16,15,15,newval);
}

static const prom_load_t pl_madr_a32 =
{
	"madr.a32",
	nullptr,
	"a0e3b4a7",
	"24e50afdeb637a6a8588f8d3a3493c9188b8da2c",
	/* size */  0400,
	/* amap */  AMAP_DEFAULT,
	/* axor */  0,
	/* dxor */  017,                        // invert D0-D3
	/* width */ 4,
	/* shift */ 0,
	/* dmap */  DMAP_REVERSE_0_3,           // reverse D0-D3 to D3-D0
	/* dand */  ZERO,
	/* type */  sizeof(UINT8)
};

/**
 * @brief initialize the mouse context to useful values
 *
 * From the Alto Hardware Manual:
 * <PRE>
 * The mouse is a hand-held pointing device which contains two encoders
 * which digitize its position as it is rolled over a table-top. It also
 * has three buttons which may be read as the three low order bits of
 * memory location UTILIN (0177030), iin the manner of the keyboard.
 * The bit/button correspondence in UTILIN are (depressed keys
 * correspond to 0's in memory):
 *
 *      UTILIN[13]      TOP or LEFT button (RED)
 *      UTILIN[14]      BOTTOM or RIGHT button (BLUE)
 *      UTILIN[15]      MIDDLE button (YELLOW)
 *
 * The mouse coordinates are maintained by the MRT microcode in locations
 * MOUSELOC(0424)=X and MOUSELOC+1(0425)=Y in page one of the Alto memory.
 * These coordinates are relative, i.e., the hardware only increments and
 * decrements them. The resolution of the mouse is approximately 100 points
 * per inch.
 * </PRE>
 */
void alto2_cpu_device::init_mouse()
{
	memset(&m_mouse, 0, sizeof(m_mouse));
	m_madr_a32 = prom_load(machine(), &pl_madr_a32, memregion("madr_a32")->base());
}

void alto2_cpu_device::exit_mouse()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_mouse()
{
	m_mouse.x = 0;
	m_mouse.y = 0;
	m_mouse.dx = 0;
	m_mouse.dy = 0;
	m_mouse.latch = 0;
	m_mouse.phase = 0;
}
