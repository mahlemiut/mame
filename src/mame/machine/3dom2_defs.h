// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    3DO M2

***************************************************************************/

#pragma once

#ifndef MACHINE_3DOM2_DEFS_H
#define MACHINE_3DOM2_DEFS_H

/***************************************************************************
    ENUMERATIONS
***************************************************************************/

#define M2_SYSCFG_VIDEO_NTSC            (0x00000000U)
#define M2_SYSCFG_VIDEO_PAL             (0x00000001U)

#define M2_SYSCFG_VIDEO_ENCODER_MEIENC  (0x00000000U)   // NTSC by default
#define M2_SYSCFG_VIDEO_ENCODER_VP536   (0x00000004U)   // NTSC by default
#define M2_SYSCFG_VIDEO_ENCODER_BT9103  (0x00000008U)   // PAL by default
#define M2_SYSCFG_VIDEO_ENCODER_DENC    (0x0000000CU)   // PAL by default

#define M2_SYSCFG_REGION_UK             (0x00000800U)
#define M2_SYSCFG_REGION_JAPAN          (0x00001000U)
#define M2_SYSCFG_REGION_US             (0x00001800U)

#if 0 // Console
#define M2_SYSCFG_AUDIO_CS4216          (0xA0000000U)
#define M2_SYSCFG_AUDIO_ASASHI          (0xE0000000U)
#else
#define M2_SYSCFG_AUDIO_CS4216          (0x20000000U)
#define M2_SYSCFG_AUDIO_ASASHI          (0x60000000U)
#endif
#define M2_SYSCFG_BOARD_AC_DEVCARD      (0x00040000U)
#define M2_SYSCFG_BOARD_AC_COREBOARD    (0x00058000U)
#define M2_SYSCFG_BOARD_DEVCARD         (0x00060000U)
#define M2_SYSCFG_BOARD_UPGRADE         (0x00070000U)
#define M2_SYSCFG_BOARD_MULTIPLAYER     (0x00078000U)

#define M2_SYSCONFIG_ARCADE (0x03600000 | SYSCFG_BOARD_AC_COREBOARD | SYSCFG_AUDIO_ASASHI | SYSCFG_REGION_JAPAN | SYSCFG_VIDEO_ENCODER_MEIENC | SYSCFG_VIDEO_NTSCU)


#define M2_MEMBASE_POWERBUS         (0x00010000U)
#define M2_MEMBASE_MEMCTL           (0x00020000U)
#define M2_MEMBASE_VDU              (0x00030000U)
#define M2_MEMBASE_TE               (0x00040000U)
#define M2_MEMBASE_DSP              (0x00060000U)
#define M2_MEMBASE_CTRLPORT         (0x00070000U)
#define M2_MEMBASE_MPEG             (0x00080000U)
#define M2_MEMBASE_TE_TRAM          (0x000c0000U)
#define M2_MEMBASE_SLOT1            (0x01000000U)
#define M2_MEMBASE_SLOT2            (0x02000000U)
#define M2_MEMBASE_SLOT3            (0x03000000U)
#define M2_MEMBASE_SLOT4            (0x04000000U)
#define M2_MEMBASE_SLOT5            (0x05000000U)
#define M2_MEMBASE_SLOT6            (0x06000000U)
#define M2_MEMBASE_SLOT7            (0x07000000U)
#define M2_MEMBASE_SLOT8            (0x08000000U)
#define M2_MEMBASE_CPUID            (0x10000000U)
#define M2_MEMBASE_RAM              (0x40000000U)

enum dev_mask
{
	DEVICE_MASK     (0x0000ffffU)
	SLOT_MASK       (0x00ffffffU)
	TE_TRAM_MASK    (0x00003fffU)
};

	enum reg_offs
	{
#define M2_BDAPCTL_DEVID            (0x00U)
#define M2_BDAPCTL_PBCONTROL        (0x10U)
#define M2_BDAPCTL_PBINTENSET       (0x40U)
#define M2_BDAPCTL_PBINTSTAT        (0x50U)
#define M2_BDAPCTL_ERRSTAT          (0x60U)
#define M2_BDAPCTL_ERRADDR          (0x70U)


	enum reg_offs
	{
		MCTL_MCONFIG    (0x0U)
		MCTL_MREF       (0x4U)
		MCTL_MCNTL      (0x8U)
		MCTL_MRESET     (0xcU)
	};

#define M2_MCFG_LDIA_MASK       (0x07000000U)
#define M2_MCFG_LDIA_SHIFT      (24U)
#define M2_MCFG_LDOA_MASK       (0x00c00000U)
#define M2_MCFG_LDOA_SHIFT      (22U)
#define M2_MCFG_RC_MASK         (0x003c0000U)
#define M2_MCFG_RC_SHIFT        (18U)
#define M2_MCFG_RCD_MASK        (0x00030000U)
#define M2_MCFG_RCD_SHIFT       (16U)
#define M2_MCFG_SS1_MASK        (0x0000e000U)
#define M2_MCFG_SS1_SHIFT       (13U)
#define M2_MCFG_SS0_MASK        (0x00001c00U)
#define M2_MCFG_SS0_SHIFT       (10U)
#define M2_MCFG_CL_MASK         (0x00000030U)
#define M2_MCFG_CL_SHIFT        (4U)

#define M2_MREF_DEBUGADDR       (0x7F000000U)  /* Selector if GPIOx_GP =(0 */
#define M2_MREF_GPIO3_GP        (0x00800000U)  /* General purpose or debug out */
#define M2_MREF_GPIO3_OUT       (0x00400000U)  /* Output or input */
#define M2_MREF_GPIO3_VALUE     (0x00200000U)  /* Value if GPIOx_GP =(1 */
#define M2_MREF_GPIO2_GP        (0x00100000U)  /* General purpose or debug out */
#define M2_MREF_GPIO2_OUT       (0x00080000U)  /* Output or input */
#define M2_MREF_GPIO2_VALUE     (0x00040000U)  /* Value if GPIOx_GP =(1 */
#define M2_MREF_GPIO1_GP        (0x00020000U)  /* General purpose or debug out */
#define M2_MREF_GPIO1_OUT       (0x00010000U)  /* Output or input */
#define M2_MREF_GPIO1_VALUE     (0x00008000U)  /* Value if GPIOx_GP =(1 */
#define M2_MREF_GPIO0_GP        (0x00004000U)  /* General purpose or debug out */
#define M2_MREF_GPIO0_OUT       (0x00002000U)  /* Output or input */
#define M2_MREF_GPIO0_VALUE     (0x00001000U)  /* Value if GPIOx_GP =(1 */
#define M2_MREF_REFRESH         (0x00000FFFU)  /* Memory refresh count */


	enum CDE_REGS
		// Miscellaneous
		CDE_DEVICE_ID       (0x000U)
		CDE_VERSION         (0x004U)
		CDE_SDBG_CNTL       (0x00C,     // Serial debug control register
		CDE_SDBG_RD         (0x010,     // Serial debug read data
		CDE_SDBG_WRT        (0x014,     // Serial debug write data
		CDE_INT_STS         (0x018,     // offset for status reg
		CDE_INT_ENABLE      (0x01CU)
		CDE_RESET_CNTL      (0x020U)
		CDE_ROM_DISABLE     (0x024U)
		CDE_CD_CMD_WRT      (0x028U)
		CDE_CD_STS_RD       (0x02CU)
		CDE_GPIO1           (0x030,     // GPIO1 control register (UART interrupt)
		CDE_GPIO2           (0x034,     // GPIO1 control register

		// BIO Bus
		CDE_DEV_DETECT      (0x200U)
		CDE_BBLOCK          (0x204U)
		CDE_BBLOCK_EN       (0x208,     // Blocking enable register
		CDE_DEV5_CONF       (0x20CU)
		CDE_DEV_STATE       (0x210U)
		CDE_DEV6_CONF       (0x214U)
		CDE_DEV5_VISA_CONF  (0x218U)
		CDE_DEV6_VISA_CONF  (0x21CU)
		CDE_UNIQ_ID_CMD     (0x220U)
		CDE_UNIQ_ID_RD      (0x224U)
		CDE_DEV_ERROR       (0x228U)
		CDE_DEV7_CONF       (0x22CU)
		CDE_DEV7_VISA_CONF  (0x230U)
		CDE_DEV0_SETUP      (0x240U)
		CDE_DEV0_CYCLE_TIME (0x244U)
		CDE_DEV1_SETUP      (0x248U)
		CDE_DEV1_CYCLE_TIME (0x24CU)
		CDE_DEV2_SETUP      (0x250U)
		CDE_DEV2_CYCLE_TIME (0x254U)
		CDE_DEV3_SETUP      (0x258U)
		CDE_DEV3_CYCLE_TIME (0x25CU)
		CDE_DEV4_SETUP      (0x260U)
		CDE_DEV4_CYCLE_TIME (0x264U)
		CDE_DEV5_SETUP      (0x268U)
		CDE_DEV5_CYCLE_TIME (0x26CU)
		CDE_DEV6_SETUP      (0x270U)
		CDE_DEV6_CYCLE_TIME (0x274U)
		CDE_DEV7_SETUP      (0x278U)
		CDE_DEV7_CYCLE_TIME (0x27CU)
		CDE_SYSTEM_CONF     (0x280U)
		CDE_VISA_DIS        (0x284U)
		M2_CDE_MICRO_RWS        (0x290U)
		M2_CDE_MICRO_WI     (0x294U)
		M2_CDE_MICRO_WOB        (0x298U)
		M2_CDE_MICRO_WO     (0x29CU)
		M2_CDE_MICRO_STATUS (0x2A0U)

		// CD DMA
		M2_CDE_CD_DMA1_CNTL (0x300U)
		M2_CDE_CD_DMA1_CPAD (0x308U)
		M2_CDE_CD_DMA1_CCNT (0x30CU)
		M2_CDE_CD_DMA1_NPAD (0x318U)
		M2_CDE_CD_DMA1_NCNT (0x31CU)
		M2_CDE_CD_DMA2_CNTL (0x320U)
		M2_CDE_CD_DMA2_CPAD (0x328U)
		M2_CDE_CD_DMA2_CCNT (0x32CU)
		M2_CDE_CD_DMA2_NPAD (0x338U)
		M2_CDE_CD_DMA2_NCNT (0x33CU)

		// BioBus DMA
		M2_CDE_DMA1_CNTL        (0x1000U)
		M2_CDE_DMA1_CBAD        (0x1004U)
		M2_CDE_DMA1_CPAD        (0x1008U)
		M2_CDE_DMA1_CCNT        (0x100CU)
		M2_CDE_DMA1_NBAD        (0x1014U)
		M2_CDE_DMA1_NPAD        (0x1018U)
		M2_CDE_DMA1_NCNT        (0x101CU)
		M2_CDE_DMA2_CNTL        (0x1020U)
		M2_CDE_DMA2_CBAD        (0x1024U)
		M2_CDE_DMA2_CPAD        (0x1028U)
		M2_CDE_DMA2_CCNT        (0x102CU)
		M2_CDE_DMA2_NBAD        (0x1034U)
		M2_CDE_DMA2_NPAD        (0x1038U)
		M2_CDE_DMA2_NCNT        (0x103CU)
	};

	enum cde_int
	{
		M2_CDE_INT_SENT     (0x80000000U)
		M2_CDE_SDBG_WRT_DONE    (0x10000000U)
		M2_CDE_SDBG_RD_DONE (0x08000000U)
		M2_CDE_DIPIR            (0x04000000U)
		M2_CDE_ARM_BOUNDS       (0x01000000U)
		M2_CDE_DMA2_BLOCKED (0x00400000U)
		M2_CDE_DMA1_BLOCKED (0x00200000U)
		M2_CDE_ID_READY     (0x00100000U)
		M2_CDE_ARM_FENCE        (0x00080000U)
		M2_CDE_EXT_INT          (0x00040000, // Added by Phil
		M2_CDE_3DO_CARD_INT (0x00020000U)
		M2_CDE_ARM_INT          (0x00010000U)
		M2_CDE_CD_DMA2_OF       (0x00004000U)
		M2_CDE_CD_DMA1_OF       (0x00002000U)
		M2_CDE_ARM_ABORT        (0x00001000U)
		M2_CDE_CD_DMA2_DONE (0x00000800U)
		M2_CDE_CD_DMA1_DONE (0x00000400U)
		M2_CDE_DMA2_DONE        (0x00000100U)
		M2_CDE_DMA1_DONE        (0x00000080U)
		M2_CDE_PBUS_ERROR       (0x00000040U)
		M2_CDE_CD_CMD_WRT_DONE  (0x00000020U)
		M2_CDE_CD_STS_RD_DONE   (0x00000010U)
		M2_CDE_CD_STS_FL_DONE   (0x00000008U)
		M2_CDE_GPIO1_INT        (0x00000004U)
		M2_CDE_GPIO2_INT        (0x00000002U)
		M2_CDE_BBUS_ERROR       (0x00000001U)
	};

	enum cde_dma_cntl
	{
		M2_CDE_DMA_DIRECTION    (0x00000400,    /* PowerBus to BioBus if set */
		M2_CDE_DMA_RESET        (0x00000200,    /* Reset engine if set */
		M2_CDE_DMA_GLOBAL       (0x00000100,    /* snoopable trans if set */
		M2_CDE_DMA_CURR_VALID   (0x00000080,    /* current setup valid if set */
		M2_CDE_DMA_NEXT_VALID   (0x00000040,    /* next setup valid if set */
		M2_CDE_DMA_GO_FOREVER   (0x00000020,    /* copy next to current if set*/
		M2_CDE_PB_CHANNEL_MASK  (0x0000001F,    /* powerbus channel to use */
	};

// CDE
#define M2_CDE_WRITEN_HOLD      (0x00000003U)
#define M2_CDE_WRITEN_SETUP     (0x0000001CU)
#define M2_CDE_READ_HOLD        (0x00000060U)
#define M2_CDE_READ_SETUP       (0x00000380U)
#define M2_CDE_PAGEMODE         (0x00000400U)
#define M2_CDE_DATAWIDTH        (0x00001800U)
#define M2_CDE_DATAWIDTH_8      (0x00000000U)
#define M2_CDE_DATAWIDTH_16     (0x00000800U)
#define M2_CDE_READ_SETUP_IO    (0x0000E000U)
#define M2_CDE_MODEA            (0x00010000U)
#define M2_CDE_HIDEA            (0x00020000U)


// TE

/***************************************************************************
 REGISTER DEFINITIONS
 ***************************************************************************/
//-------------------------------------------------
//  General Control
//-------------------------------------------------

// Triangle Engine Master Mode
#define M2_TE_MASTER_MODE_RESET             (0x00000001U)
#define M2_TE_MASTER_MODE_DTEXT             (0x00000002U)
#define M2_TE_MASTER_MODE_DSHADE            (0x00000004U)
#define M2_TE_MASTER_MODE_DBLEND            (0x00000008U)
#define M2_TE_MASTER_MODE_DZBUF             (0x00000010U)
#define M2_TE_MASTER_MODE_DDITH             (0x00000020U)

// Triangle Engine Immediate Control
#define M2_TE_TEICNTL_INT                   (0x00000001U)
#define M2_TE_TEICNTL_STEP                  (0x00000002U)
#define M2_TE_TEICNTL_STPL                  (0x00000004U)
#define M2_TE_TEICNTL_STPI                  (0x00000008U)
#define M2_TE_TEICNTL_RSTRT                 (0x00000010U)
#define M2_TE_TEICNTL_STRT                  (0x00000020U)

#define M2_TE_TEDCNTL_TLD                   (0x00000001U)
#define M2_TE_TEDCNTL_JA                    (0x00000002U)
#define M2_TE_TEDCNTL_JR                    (0x00000004U)
#define M2_TE_TEDCNTL_INT                   (0x00000008U)
#define M2_TE_TEDCNTL_PSE                   (0x00000010U)
#define M2_TE_TEDCNTL_SYNC                  (0x00000020U)

#define M2_TE_INTSTAT_DEFERRED_INSTR        (0x00000100U)
#define M2_TE_INTSTAT_IMMEDIATE_INSTR       (0x00000200U)
#define M2_TE_INTSTAT_LIST_END              (0x00000400U)
#define M2_TE_INTSTAT_WINDOW_CLIP           (0x00000800U)
#define M2_TE_INTSTAT_SPECIAL_INSTR         (0x00001000U)
#define M2_TE_INTSTAT_UNIMPLEMENTED_INSTR   (0x00002000U)
#define M2_TE_INTSTAT_SUPERVISOR            (0x00004000U)
#define M2_TE_INTSTAT_ANY_RENDER            (0x00008000U)
#define M2_TE_INTSTAT_Z_FUNC                (0x00010000U)
#define M2_TE_INTSTAT_ALU_STATUS            (0x00020000U)
#define M2_TE_INTSTAT_FB_CLIP               (0x00040000U)
#define M2_TE_INTSTAT_IMMEDIATE             (0x00080000U)

// IWP
// IRP
// Interrupt Enable
// Interrupt Status
// Vertex Control



//-------------------------------------------------
//  Setup Engine
//-------------------------------------------------

		// Vertex State
#define M2_TE_VERTEXSTATE_TSORT_MASK        (0x00000007U)
#define M2_TE_VERTEXSTATE_TSORT_OMN         (0x00000001U)
#define M2_TE_VERTEXSTATE_TSORT_MNO         (0x00000002U)
#define M2_TE_VERTEXSTATE_TSORT_ONM         (0x00000003U)
#define M2_TE_VERTEXSTATE_TSORT_NOM         (0x00000004U)
#define M2_TE_VERTEXSTATE_TSORT_MON         (0x00000005U)
#define M2_TE_VERTEXSTATE_TSORT_NMO         (0x00000006U)

#define M2_TE_VERTEXSTATE_VCNT_SHIFT        (3U)
#define M2_TE_VERTEXSTATE_VCNT_MASK         (0x00000018U)


		//-------------------------------------------------
		//  Edge Walker
		//-------------------------------------------------

		// Edge and Span Walker Control
#define ESCNTL_DSPOFF                       (0x00000001U)
#define ESCNTL_DUSCAN                       (0x00000002U)
#define ESCNTL_PERSPECTIVEOFF               (0x00000004U)


		//-------------------------------------------------
		//  Texture Mapper
		//-------------------------------------------------

		// Texture Mapper Master Control (0x00046400)
#define M2_TE_TXTCNTL_MMDMA_TRAM_ON             (0x00000004U)
#define M2_TE_TXTCNTL_MMDMA_PIP_ON              (0x00000008U)
#define M2_TE_TXTCNTL_SNOOP_ON                  (0x00000020U)

		// Texture Load Control (0x00046404)
#define M2_TE_TXTLDCNTL_SRCBITOFFS              (0x00000007U)
#define M2_TE_TXTLDCNTL_LDMODE_MASK             (0x00000300U)
#define M2_TE_TXTLDCNTL_LDMODE_TEXLOAD          (0x00000000U)
#define M2_TE_TXTLDCNTL_LDMODE_MMDMA            (0x00000100U)
#define M2_TE_TXTLDCNTL_LDMODE_PIPLOAD          (0x00000200U)
#define M2_TE_TXTLDCNTL_LDMODE_RESERVED         (0x00000300U)
#define M2_TE_TXTLDCNTL_COMPRESSED              (0x00000400U)

		// Address Control (0x00046408)
#define M2_TE_TXTADDRCNTL_LODMAX_MASK           (0x0000000fU)
#define M2_TE_TXTADDRCNTL_FILTERSEL_MASK        (0x00000003U)
#define M2_TE_TXTADDRCNTL_FILTERSEL_POINT       (0x00000000U)
#define M2_TE_TXTADDRCNTL_FILTERSEL_LINEAR      (0x00000001U)
#define M2_TE_TXTADDRCNTL_FILTERSEL_BILINEAR    (0x00000002U)
#define M2_TE_TXTADDRCNTL_FILTERSEL_QUASITRI    (0x00000003U)
#define M2_TE_TXTADDRCNTL_R12FILTERSEL_SHIFT    (4U)
#define M2_TE_TXTADDRCNTL_R3FILTERSEL_SHIFT     (7U)
#define M2_TE_TXTADDRCNTL_R45FILTERSEL_SHIFT    (10U)
#define M2_TE_TXTADDRCNTL_LOOKUP_EN             (0x00002000U)

		// PIP Control (0x0004640C)
#define M2_TE_TXTPIPCNTL_INDEX_OFFSET           (0x000000ffU)
#define M2_TE_TXTPIPCNTL_COLORSEL_MASK          (0x00000700U)
#define M2_TE_TXTPIPCNTL_COLORSEL_SHIFT         (8U)
#define M2_TE_TXTPIPCNTL_ALPHASEL_MASK          (0x00003800U)
#define M2_TE_TXTPIPCNTL_ALPHASEL_SHIFT         (11U)
#define M2_TE_TXTPIPCNTL_SSBSEL_MASK            (0x0001c000U)
#define M2_TE_TXTPIPCNTL_SSBSEL_SHIFT           (14U)

#define M2_TE_TXTPIPCNTL_SEL_CONSTANT               0
#define M2_TE_TXTPIPCNTL_SEL_TRAM                   1
#define M2_TE_TXTPIPCNTL_SEL_PIP                    2

		// Texture Application Control (0x00046410)
#define M2_TE_TXTTABCNTL_C_ASEL_MASK                0x00000007
#define M2_TE_TXTTABCNTL_C_ASEL_SHIFT               0
#define M2_TE_TXTTABCNTL_C_BSEL_MASK                0x00000038
#define M2_TE_TXTTABCNTL_C_BSEL_SHIFT               3
#define M2_TE_TXTTABCNTL_C_TSEL_MASK                0x000001c0
#define M2_TE_TXTTABCNTL_C_TSEL_SHIFT               6

#define M2_TE_TXTTABCNTL_C_ABTSEL_AITER         0
#define M2_TE_TXTTABCNTL_C_ABTSEL_CITER         1
#define M2_TE_TXTTABCNTL_C_ABTSEL_AT                2
#define M2_TE_TXTTABCNTL_C_ABTSEL_CT                3
#define M2_TE_TXTTABCNTL_C_ABTSEL_ACONST            4
#define M2_TE_TXTTABCNTL_C_ABTSEL_CCONST            5

#define M2_TE_TXTTABCNTL_C_OSEL_MASK                0x00000600
#define M2_TE_TXTTABCNTL_C_OSEL_SHIFT               9

#define M2_TE_TXTTABCNTL_CO_SEL_CITER               0
#define M2_TE_TXTTABCNTL_CO_SEL_CT              1
#define M2_TE_TXTTABCNTL_CO_SEL_BLEND               2
#define M2_TE_TXTTABCNTL_CO_SEL_RESERVED            3

#define M2_TE_TXTTABCNTL_A_ASEL_MASK                0x00001800
#define M2_TE_TXTTABCNTL_A_ASEL_SHIFT               11
#define M2_TE_TXTTABCNTL_A_BSEL_MASK                0x00006000
#define M2_TE_TXTTABCNTL_A_BSEL_SHIFT               13

#define M2_TE_TXTTABCNTL_A_ABSEL_AITER          0
#define M2_TE_TXTTABCNTL_A_ABSEL_AT             1
#define M2_TE_TXTTABCNTL_A_ABSEL_ACONST         2

#define M2_TE_TXTTABCNTL_A_OSEL_MASK                0x00018000
#define M2_TE_TXTTABCNTL_A_OSEL_SHIFT               15

#define M2_TE_TXTTABCNTL_AO_SEL_AITER               0
#define M2_TE_TXTTABCNTL_AO_SEL_AT              1
#define M2_TE_TXTTABCNTL_AO_SEL_BLEND               2
#define M2_TE_TXTTABCNTL_AO_SEL_RESERVED            3

#define M2_TE_TXTTABCNTL_BLENDOP_MASK               0x00010000
#define M2_TE_TXTTABCNTL_BLENDOP_SHIFT          16
#define M2_TE_TXTTABCNTL_BLENDOP_LERP               0
#define M2_TE_TXTTABCNTL_BLENDOP_MULT               1

		// TAB Constants
#define M2_TE_TXTTABCONST_BLUE                  0x000000ff
#define M2_TE_TXTTABCONST_BLUE_SHIFT                0
#define M2_TE_TXTTABCONST_GREEN                 0x0000ff00
#define M2_TE_TXTTABCONST_GREEN_SHIFT               8
#define M2_TE_TXTTABCONST_RED                       0x00ff0000
#define M2_TE_TXTTABCONST_RED_SHIFT             16
#define M2_TE_TXTTABCONST_ALPHA                 0x7f000000
#define M2_TE_TXTTABCONST_ALPHA_SHIFT               24
#define M2_TE_TXTTABCONST_SSB                       0x80000000

		// Texture Loader Destination Base (0x00046414)
#define M2_TE_TXTLDDSTBASE_ADDR                 0x00003ffc

		// Texture Lod Base 0 (0x00046414)
		// Texture Lod Base 1 (0x00046418)
		// Texture Lod Base 2 (0x0004641C)
		// Texture Lod Base 3 (0x00046420)
#define M2_TE_TXTLODBASE_MASK                       0x00003ffc

		// Texture Loader Source Base (0x00046424)
#define M2_TE_TXTLDSRCBASE_ADDR                 0x00003fff

		// Texture Loader Counts (0x00046428)
#define M2_TE_TXTLDBYTECNT_COUNT                    0x0fffffff
#define M2_TE_TXTLDROWCNT_COUNT                 0x0fffffff
#define M2_TE_TXTLDTEXCNT_COUNT                 0x0fffffff

		// Texture Loader Width (0x00046428)
#define M2_TE_TxTLDWIDTH_SRCROW                 0x0000ffff
#define M2_TE_TxTLDWIDTH_DSTROW_SHIFT               16
#define M2_TE_TxTLDWIDTH_DSTROW                 0xffff0000

		// Texture Size (0x0004642C)
#define M2_TE_TXTUVMAX_VMAX_MASK                    0x000003ff
#define M2_TE_TXTUVMAX_VMAX_SHIFT                   0
#define M2_TE_TXTUVMAX_UMAX_MASK                    0x03ff0000
#define M2_TE_TXTUVMAX_UMAX_SHIFT                   16

		// Texture Mask (0x00046430)
#define M2_TE_TXTUVMASK_VMASK_MASK              0x000003ff
#define M2_TE_TXTUVMASK_VMASK_SHIFT             0
#define M2_TE_TXTUVMASK_UMASK_MASK              0x03ff0000
#define M2_TE_TXTUVMASK_UMASK_SHIFT             16

		// TRAM Format (0x0004643C)
		// TODO: Expansion formats
#define M2_TE_TXTEXPFORM_CDEPTH_MASK                0x0000000f
#define M2_TE_TXTEXPFORM_CDEPTH_SHIFT               0
#define M2_TE_TXTEXPFORM_IDEPTH_MASK                0x0000000f
#define M2_TE_TXTEXPFORM_IDEPTH_SHIFT               0
#define M2_TE_TXTEXPFORM_ADEPTH_MASK                0x000000f0
#define M2_TE_TXTEXPFORM_ADEPTH_SHIFT               4
#define M2_TE_TXTEXPFORM_TRANSPARENT                0x00000100
#define M2_TE_TXTEXPFORM_SSBON                  0x00000200
#define M2_TE_TXTEXPFORM_COLORON                    0x00000400
#define M2_TE_TXTEXPFORM_INDEXON                    0x00000400
#define M2_TE_TXTEXPFORM_ALPHAON                    0x00000800
#define M2_TE_TXTEXPFORM_LITERAL                    0x00001000


		// Format Registers


		//-------------------------------------------------
		//  Destination Blender
		//-------------------------------------------------

		// Snoop (0x0048000)
#define M2_TE_DBSNOOP_DESTWRSNOOP               0x00000001
#define M2_TE_DBSNOOP_SRCRDSNOOP                0x00000002
#define M2_TE_DBSNOOP_ZWRSNOOP              0x00000004
#define M2_TE_DBSNOOP_ZRDSNOOP              0x00000008

		// Supervisor General Control (0x0048004)
#define M2_TE_DBSUPERGENCTL_DESTOUTEN           0x00000001
#define M2_TE_DBSUPERGENCTL_DESTWR16BEN     0x00000002
#define M2_TE_DBSUPERGENCTL_ZWR16BEN            0x00000004

		// User General Control (0x0048008)
#define M2_TE_DBUSERGENCTL_DESTOUT_MASK     0x0000000f
#define M2_TE_DBUSERGENCTL_DITHEREN         0x00000010
#define M2_TE_DBUSERGENCTL_SRCINEN          0x00000020
#define M2_TE_DBUSERGENCTL_BLENDEN          0x00000040
#define M2_TE_DBUSERGENCTL_WCLIPOUTEN           0x00000080
#define M2_TE_DBUSERGENCTL_WCLIPINEN            0x00000100
#define M2_TE_DBUSERGENCTL_ZOUTEN               0x00000200
#define M2_TE_DBUSERGENCTL_ZBUFEN               0x00000400

		// Discard Control (0x004800C)
#define M2_TE_DBDISCARDCTL_ADISEN               0x00000001
#define M2_TE_DBDISCARDCTL_RGBDISEN         0x00000002
#define M2_TE_DBDISCARDCTL_SSBDISEN         0x00000004
#define M2_TE_DBDISCARDCTL_ZCLIPDISEN           0x00000008

		// Status (0x0048010)
#define M2_TE_DBSTATUS_ANYREND              0x00000001
#define M2_TE_DBSTATUS_ZFUNC_GT             0x00000002
#define M2_TE_DBSTATUS_ZFUNC_EQ             0x00000004
#define M2_TE_DBSTATUS_ZFUNC_LT             0x00000008
#define DBSTATUS_ALUSTAT_BLUE_GT        0x00000010
#define DBSTATUS_ALUSTAT_BLUE_EQ        0x00000020
#define DBSTATUS_ALUSTAT_BLUE_LT        0x00000040
#define DBSTATUS_ALUSTAT_GREEN_GT       0x00000080
#define DBSTATUS_ALUSTAT_GREEN_EQ       0x00000100
#define DBSTATUS_ALUSTAT_GREEN_LT       0x00000200
#define DBSTATUS_ALUSTAT_RED_GT         0x00000400
#define DBSTATUS_ALUSTAT_RED_EQ         0x00000800
#define DBSTATUS_ALUSTAT_RED_LT         0x00001000
#define M2_TE_DBSTATUS_ZCLIP                    0x00002000
#define M2_TE_DBSTATUS_WINCLIP              0x00004000
#define M2_TE_DBSTATUS_FBCLIP                   0x00008000

		// Interrupt Control (0x00048014)
#define M2_TE_DBINTCNTL_ZFUNCSTATINTEN_MASK 0x00000003
#define M2_TE_DBINTCNTL_ZFUNCSTATINTEN_MASK 0x00000003

		// Framebuffer XY Clip Control (0x00048018)
#define M2_TE_DBFBCLIP_YFBCLIP_MASK         0x000007ff
#define M2_TE_DBFBCLIP_YFBCLIP_SHIFT            0
#define M2_TE_DBFBCLIP_XFBCLIP_MASK         0x07ff0000
#define M2_TE_DBFBCLIP_XFBCLIP_SHIFT            16

		// Window X Clip Control (0x0004801C)
#define M2_TE_DBFBXWINCLIP_XMAX_MASK            0x000007ff
#define M2_TE_DBFBXWINCLIP_XMAX_SHIFT           0
#define M2_TE_DBFBXWINCLIP_XMIN_MASK            0x07ff0000
#define M2_TE_DBFBXWINCLIP_XMIN_SHIFT           16

		// Window Y Clip Control (0x00048020)
#define M2_TE_DBFBYWINCLIP_YMAX_MASK            0x000007ff
#define M2_TE_DBFBYWINCLIP_YMAX_SHIFT           0
#define M2_TE_DBFBYWINCLIP_YMIN_MASK            0x07ff0000
#define M2_TE_DBFBYWINCLIP_YMIN_SHIFT           16

		// Destination Write Control (0x0048024)
#define M2_TE_DBDESTCNTL_32BPP              0x00000001

		// Destination Write Base Address (0x0048028)

		// Destination X Stride (0x004802C)
#define M2_TE_DBDEST_XSTRIDE                    0x000007ff

		// Source Read Control (0x00048030)
#define M2_TE_DBSRCCNTL_32BPP                   0x00000001
#define M2_TE_DBSRCCNTL_MSBREP              0x00000002

		// Source Read Base Address (0x00048034)

		// Source X Stride (0x00048038)
#define M2_TE_DBSRCXSTRIDE                  0x000007ff

		// Source XY Offset (0x0004803C)
#define M2_TE_DBSRCOFFS_YOFFS_MASK          0x00000fff
#define M2_TE_DBSRCOFFS_YOFFS_SHIFT         0
#define M2_TE_DBSRCOFFS_XOFFS_MASK          0x0fff0000
#define M2_TE_DBSRCOFFS_XOFFS_SHIFT         16

		// Z Buffer Control (0x00048040)
#define M2_TE_DBZCNTL_ZFUNCCNTL_MASK            0x0000003f
#define M2_TE_DBZCNTL_ZPIXOUT_LT                0x00000001
#define M2_TE_DBZCNTL_ZBUFOUT_LT                0x00000002
#define M2_TE_DBZCNTL_ZPIXOUT_EQ                0x00000004
#define M2_TE_DBZCNTL_ZBUFOUT_EQ                0x00000008
#define M2_TE_DBZCNTL_ZPIXOUT_GT                0x00000010
#define M2_TE_DBZCNTL_ZBUFOUT_GT                0x00000020

		// Z Buffer Base Address (0x00048044)
#define M2_TE_DBZBASEADDR_MASK              0x00ffffff

		// Z Buffer XY Offset (0x00048048)
#define M2_TE_DBZOFFS_YOFFS_MASK                0x00000fff
#define M2_TE_DBZOFFS_YOFFS_SHIFT               0
#define M2_TE_DBZOFFS_XOFFS_MASK                0x0fff0000
#define M2_TE_DBZOFFS_XOFFS_SHIFT               16

		// Z Buffer Clip (0x0004804C)
#define M2_TE_DBZCLIP_YCLIP_MASK                0x000007ff
#define M2_TE_DBZCLIP_YCLIP_SHIFT               0
#define M2_TE_DBZCLIP_XCLIP_MASK                0x07ff0000
#define M2_TE_DBZCLIP_XCLIP_SHIFT               16

		// SSB/DSB Control (0x00048050)
#define M2_TE_DBSSBDSBCNTL_DSBSELECT_MASK       0x00000007
#define M2_TE_DBSSBDSBCNTL_DSBSELECT_SHIFT  0
#define M2_TE_DBSSBDSBCNTL_DSBSELECT_SSB        0
#define M2_TE_DBSSBDSBCNTL_DSBSELECT_CONSTANT   1
#define M2_TE_DBSSBDSBCNTL_DSBSELECT_SRC        2

#define M2_TE_DBSSBDSBCNTL_DSBCONST         0x00000004

		// RGB constants (0x00048054)
#define M2_TE_DBCONSTIN_B_MASK              0x000000ff
#define M2_TE_DBCONSTIN_B_SHIFT             0
#define M2_TE_DBCONSTIN_G_MASK              0x0000ff00
#define M2_TE_DBCONSTIN_G_SHIFT             8
#define M2_TE_DBCONSTIN_R_MASK              0x00ff0000
#define M2_TE_DBCONSTIN_R_SHIFT             16

		// Texture Multiplication Control (0x00048058)
#define M2_TE_DBTXTMULTCNTL_TXTRJUST                    0x00000001
#define M2_TE_DBTXTMULTCNTL_TXTCOEFCMP              0x00000002

#define M2_TE_DBTXTMULTCNTL_TXTCONSTCNTL_MASK           0x0000000c
#define M2_TE_DBTXTMULTCNTL_TXTCONSTCNTL_SHIFT      2
#define M2_TE_DBTXTMULTCNTL_TXTCONSTCNTL_TEXSSB     0
#define M2_TE_DBTXTMULTCNTL_TXTCONSTCNTL_SRCDSB     1

#define M2_TE_DBTXTMULTCNTL_COEFSEL_MASK                0x00000030
#define M2_TE_DBTXTMULTCNTL_COEFSEL_SHIFT               4
#define M2_TE_DBTXTMULTCNTL_COEFSEL_ATI             0
#define M2_TE_DBTXTMULTCNTL_COEFSEL_ASRC                1
#define M2_TE_DBTXTMULTCNTL_COEFSEL_CONSTANT            2
#define M2_TE_DBTXTMULTCNTL_COEFSEL_CSRC                3

#define M2_TE_DBTXTMULTCNTL_INSEL_MASK              0x000000c0
#define M2_TE_DBTXTMULTCNTL_INSEL_SHIFT             6
#define M2_TE_DBTXTMULTCNTL_INSEL_CTI                   0
#define M2_TE_DBTXTMULTCNTL_INSEL_CONSTANT          1
#define M2_TE_DBTXTMULTCNTL_INSEL_COMPSRC               2
#define M2_TE_DBTXTMULTCNTL_INSEL_ATI                   3

		// Source Multiplication Control (0x00048058)
#define M2_TE_DBSRCMULTCNTL_SRCRJUST                    0x00000001
#define M2_TE_DBSRCMULTCNTL_SRCCOEFCMP              0x00000002

#define M2_TE_DBSRCMULTCNTL_SRCCONSTCNTL_MASK           0x0000000c
#define M2_TE_DBSRCMULTCNTL_SRCCONSTCNTL_SHIFT      2
#define M2_TE_DBSRCMULTCNTL_SRCCONSTCNTL_TEXSSB     0
#define M2_TE_DBSRCMULTCNTL_SRCCONSTCNTL_SRCDSB     1

#define M2_TE_DBSRCMULTCNTL_COEFSEL_MASK                0x00000030
#define M2_TE_DBSRCMULTCNTL_COEFSEL_SHIFT               4
#define M2_TE_DBSRCMULTCNTL_COEFSEL_ATI             0
#define M2_TE_DBSRCMULTCNTL_COEFSEL_ASRC                1
#define M2_TE_DBSRCMULTCNTL_COEFSEL_CONSTANT            2
#define M2_TE_DBSRCMULTCNTL_COEFSEL_CTI             3

#define M2_TE_DBSRCMULTCNTL_INSEL_MASK              0x000000c0
#define M2_TE_DBSRCMULTCNTL_INSEL_SHIFT             6
#define M2_TE_DBSRCMULTCNTL_INSEL_SRC                   0
#define M2_TE_DBSRCMULTCNTL_INSEL_CONSTANT          1
#define M2_TE_DBSRCMULTCNTL_INSEL_COMPCTI               2
#define M2_TE_DBSRCMULTCNTL_INSEL_TEXALPHA          3

// ALU Control (0x00048070)
#define M2_TE_DBALUCNTL_FINALDIVIDE_MASK            0x00000007
#define M2_TE_DBALUCNTL_FINALDIVIDE_SHIFT           0

#define M2_TE_DBALUCNTL_ALUOP_MASK                  0x000000f8
#define M2_TE_DBALUCNTL_ALUOP_SHIFT                 5

// Source Alpha Control (0x00048074)
#define M2_TE_DBDSTACNTL_ADESTSEL_MASK              0x00000003
#define M2_TE_DBDSTACNTL_ADESTSEL_SHIFT             0
#define M2_TE_DBDSTACNTL_ADESTCONSTCNTL_MASK        0x0000000c
#define M2_TE_DBDSTACNTL_ADESTCONSTCNTL_SHIFT       2

#define M2_TE_DBDSTALPHACONST_CONST1_MASK           0x000000ff
#define M2_TE_DBDSTALPHACONST_CONST1_SHIFT          0
#define M2_TE_DBDSTALPHACONST_CONST0_MASK           0x00ff0000
#define M2_TE_DBDSTALPHACONST_CONST0_SHIFT          16

#define M2_TE_DBSSBDSBCNTL_DSBSEL_MASK              0x00000003
#define M2_TE_DBSSBDSBCNTL_DSBSEL_SHIFT             0
#define M2_TE_DBSSBDSBCNTL_DSBCONST_MASK            0x00000004
#define M2_TE_DBSSBDSBCNTL_DSBCONST_SHIFT           2

#endif // MACHINE_3DOM2_DEFS_H
