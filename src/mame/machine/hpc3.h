// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HPC3 "High-performance Peripheral Controller" emulation

**********************************************************************/

#ifndef MAME_MACHINE_HPC3_H
#define MAME_MACHINE_HPC3_H

#pragma once

#include "cpu/mips/mips3.h"
#include "machine/ioc2.h"
#include "machine/wd33c93.h"
#include "sound/dac.h"

class hpc3_device : public device_t
{
public:
	template <typename T, typename U, typename V, typename W, typename X>
	hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner
		, T &&cpu_tag, U &&scsi_tag, V &&ioc2_tag, W &&ldac_tag, X &&rdac_tag)
		: hpc3_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_wd33c93.set_tag(std::forward<U>(scsi_tag));
		m_ioc2.set_tag(std::forward<V>(ioc2_tag));
		m_ldac.set_tag(std::forward<W>(ldac_tag));
		m_rdac.set_tag(std::forward<X>(rdac_tag));
	}

	template <typename T, typename U, typename V, typename W, typename X, typename Y>
	hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner
		, T &&cpu_tag, U &&scsi_tag, V &&scsi2_tag, W &&ioc2_tag, X &&ldac_tag, Y &&rdac_tag)
		: hpc3_device(mconfig, tag, owner
			, std::forward<T>(cpu_tag), std::forward<U>(scsi_tag), std::forward<W>(ioc2_tag), std::forward<X>(ldac_tag), std::forward<Y>(rdac_tag))
	{
		m_wd33c93_2.set_tag(std::forward<V>(scsi2_tag));
	}

	hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER(hd_enet_r);
	DECLARE_WRITE32_MEMBER(hd_enet_w);
	template <uint32_t index> DECLARE_READ32_MEMBER(hd_r);
	template <uint32_t index> DECLARE_WRITE32_MEMBER(hd_w);
	DECLARE_READ32_MEMBER(pbus4_r);
	DECLARE_WRITE32_MEMBER(pbus4_w);
	DECLARE_READ32_MEMBER(pbusdma_r);
	DECLARE_WRITE32_MEMBER(pbusdma_w);
	DECLARE_READ32_MEMBER(unkpbus0_r);
	DECLARE_WRITE32_MEMBER(unkpbus0_w);

	DECLARE_READ32_MEMBER(dma_config_r);
	DECLARE_WRITE32_MEMBER(dma_config_w);
	DECLARE_READ32_MEMBER(pio_config_r);
	DECLARE_WRITE32_MEMBER(pio_config_w);

	DECLARE_WRITE_LINE_MEMBER(scsi0_irq);
	DECLARE_WRITE_LINE_MEMBER(scsi0_drq);
	DECLARE_WRITE_LINE_MEMBER(scsi1_irq);
	DECLARE_WRITE_LINE_MEMBER(scsi1_drq);

protected:
	void device_start() override;
	void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void do_pbus_dma(uint32_t channel);

	void dump_chain(uint32_t base);
	void fetch_chain(int channel);
	void decrement_chain(int channel);
	void scsi_drq(bool state, int channel);
	void scsi_dma(int channel);

	static const device_timer_id TIMER_PBUS_DMA = 0;

	struct pbus_dma_t
	{
		bool m_active;
		uint32_t m_buf_ptr;
		uint32_t m_cur_ptr;
		uint32_t m_desc_ptr;
		uint32_t m_desc_flags;
		uint32_t m_next_ptr;
		uint32_t m_bytes_left;
		uint32_t m_config;
		emu_timer *m_timer;
	};

	enum
	{
		PBUS_CTRL_ENDIAN    = 0x00000002,
		PBUS_CTRL_RECV      = 0x00000004,
		PBUS_CTRL_FLUSH     = 0x00000008,
		PBUS_CTRL_DMASTART  = 0x00000010,
		PBUS_CTRL_LOAD_EN   = 0x00000020,
		PBUS_CTRL_REALTIME  = 0x00000040,
		PBUS_CTRL_HIGHWATER = 0x0000ff00,
		PBUS_CTRL_FIFO_BEG  = 0x003f0000,
		PBUS_CTRL_FIFO_END  = 0x3f000000,
	};

	enum
	{
		PBUS_DMADESC_EOX  = 0x80000000,
		PBUS_DMADESC_EOXP = 0x40000000,
		PBUS_DMADESC_XIE  = 0x20000000,
		PBUS_DMADESC_IPG  = 0x00ff0000,
		PBUS_DMADESC_TXD  = 0x00008000,
		PBUS_DMADESC_BC   = 0x00003fff,
	};

	enum
	{
		HPC3_DMACTRL_IRQ    = 0x01,
		HPC3_DMACTRL_ENDIAN = 0x02,
		HPC3_DMACTRL_DIR    = 0x04,
		HPC3_DMACTRL_ENABLE = 0x10,
	};

	required_device<mips3_device> m_maincpu;
	required_device<wd33c93_device> m_wd33c93;
	optional_device<wd33c93_device> m_wd33c93_2;
	required_device<ioc2_device> m_ioc2;
	required_device<dac_16bit_r2r_twos_complement_device> m_ldac;
	required_device<dac_16bit_r2r_twos_complement_device> m_rdac;
	required_shared_ptr<uint32_t> m_mainram;

	uint32_t m_enetr_nbdp;
	uint32_t m_enetr_cbp;

	struct scsi_dma_t
	{
		uint32_t m_desc;
		uint32_t m_addr;
		uint32_t m_ctrl;
		uint32_t m_length;
		uint32_t m_next;
		bool m_irq;
		bool m_big_endian;
		bool m_to_device;
		bool m_active;
	};

	scsi_dma_t m_scsi_dma[2];
	pbus_dma_t m_pbus_dma[8];
	uint32_t m_pio_config[10];

	address_space *m_cpu_space;

	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ... );
};

DECLARE_DEVICE_TYPE(SGI_HPC3, hpc3_device)

#endif // MAME_MACHINE_HAL2_H
