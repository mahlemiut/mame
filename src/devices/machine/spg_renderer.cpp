// license:BSD-3-Clause
// copyright-holders:David Haywood, Ryan Holtz

#include "emu.h"
#include "spg_renderer.h"

DEFINE_DEVICE_TYPE(SPG_RENDERER, spg_renderer_device, "spg_renderer", "SunPlus / GeneralPlus video rendering")

spg_renderer_device::spg_renderer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
{
}

spg_renderer_device::spg_renderer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spg_renderer_device(mconfig, SPG_RENDERER, tag, owner, clock)
{
}

void spg_renderer_device::device_start()
{
	for (uint8_t i = 0; i < 32; i++)
	{
		m_rgb5_to_rgb8[i] = (i << 3) | (i >> 2);
	}
	for (uint16_t i = 0; i < 0x8000; i++)
	{
		m_rgb555_to_rgb888[i] = (m_rgb5_to_rgb8[(i >> 10) & 0x1f] << 16) |
								(m_rgb5_to_rgb8[(i >>  5) & 0x1f] <<  8) |
								(m_rgb5_to_rgb8[(i >>  0) & 0x1f] <<  0);
	}

	save_item(NAME(m_video_regs_1c));
	save_item(NAME(m_video_regs_1d));
	save_item(NAME(m_video_regs_1e));

	save_item(NAME(m_video_regs_22));
	save_item(NAME(m_video_regs_2a));

	save_item(NAME(m_video_regs_30));
	save_item(NAME(m_video_regs_3c));

	save_item(NAME(m_video_regs_42));

	save_item(NAME(m_ycmp_table));
}

void spg_renderer_device::device_reset()
{
	m_video_regs_1c = 0x0000;
	m_video_regs_1d = 0x0000;
	m_video_regs_1e = 0x0000;

	m_video_regs_22 = 0x0000;
	m_video_regs_2a = 0x0000;
	
	m_video_regs_30 = 0x0000;
	m_video_regs_3c = 0x0020;

	m_video_regs_42 = 0x0001;

	for (int i = 0; i < 480; i++)
	{
		m_ycmp_table[i] = 0xffffffff;
	}
}


// Perform a lerp between a and b
inline uint8_t spg_renderer_device::mix_channel(uint8_t bottom, uint8_t top)
{
	uint8_t alpha = (m_video_regs_2a & 3) << 6;
	return ((256 - alpha) * bottom + alpha * top) >> 8;
}

template<spg_renderer_device::blend_enable_t Blend, spg_renderer_device::flipx_t FlipX>
void spg_renderer_device::draw_tilestrip(const rectangle& cliprect, uint32_t* dst, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint16_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space &spc, uint16_t* paletteram)
{
	const uint32_t yflipmask = flip_y ? tile_h - 1 : 0;
	uint32_t m = tilegfxdata_addr + words_per_tile * tile + bits_per_row * (tile_scanline ^ yflipmask);
	uint32_t bits = 0;
	uint32_t nbits = 0;

	for (int32_t x = FlipX ? (tile_w - 1) : 0; FlipX ? x >= 0 : x < tile_w; FlipX ? x-- : x++)
	{
		int realdrawpos = (drawx + x) & 0x1ff;

		bits <<= nc_bpp;

		if (nbits < nc_bpp)
		{
			uint16_t b = spc.read_word(m++ & 0x3fffff);
			b = (b << 8) | (b >> 8);
			bits |= b << (nc_bpp - nbits);
			nbits += 16;
		}
		nbits -= nc_bpp;

		uint32_t pal = palette_offset + (bits >> 16);
		bits &= 0xffff;

		if (realdrawpos >= 0 && realdrawpos < 320)
		{
			uint16_t rgb = paletteram[pal];

			if (!(rgb & 0x8000))
			{
				if (Blend)
				{
					dst[realdrawpos] = (mix_channel((uint8_t)(dst[realdrawpos] >> 16), m_rgb5_to_rgb8[(rgb >> 10) & 0x1f]) << 16) |
						(mix_channel((uint8_t)(dst[realdrawpos] >> 8), m_rgb5_to_rgb8[(rgb >> 5) & 0x1f]) << 8) |
						(mix_channel((uint8_t)(dst[realdrawpos] >> 0), m_rgb5_to_rgb8[rgb & 0x1f]));
				}
				else
				{
					dst[realdrawpos] = m_rgb555_to_rgb888[rgb];
				}
			}
		}
	}
}


void spg_renderer_device::draw_linemap(const rectangle& cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t tilegfxdata_addr, uint16_t* scrollregs, uint16_t* tilemapregs, address_space &spc, uint16_t* paletteram)
{
	if ((scanline < 0) || (scanline >= 240))
		return;


	uint32_t tilemap = tilemapregs[2];
	uint32_t palette_map = tilemapregs[3];

	//printf("draw bitmap bases %04x %04x\n", tilemap, palette_map);

	//uint32_t xscroll = scrollregs[0];
	uint32_t yscroll = scrollregs[1];

	int realline = (scanline + yscroll) & 0xff;


	uint16_t tile = spc.read_word(tilemap + realline);
	uint16_t palette = 0;

	//if (!tile)
	//  continue;

	palette = spc.read_word(palette_map + realline / 2);
	if (scanline & 1)
		palette >>= 8;
	else
		palette &= 0x00ff;

	//const int linewidth = 320 / 2;
	int sourcebase = tile | (palette << 16);

	uint32_t ctrl = tilemapregs[1];

	if (ctrl & 0x80) // HiColor mode (rad_digi)
	{
		for (int i = 0; i < 320; i++)
		{
			const uint16_t data = spc.read_word(sourcebase + i);

			if (!(data & 0x8000))
			{
				dst[i] = m_rgb555_to_rgb888[data & 0x7fff];
			}
		}
	}
	else
	{
		for (int i = 0; i < 320 / 2; i++)
		{
			uint8_t palette_entry;
			uint16_t color;
			const uint16_t data = spc.read_word(sourcebase + i);

			palette_entry = (data & 0x00ff);
			color = paletteram[palette_entry];

			if (!(color & 0x8000))
			{
				dst[(i * 2) + 0] = m_rgb555_to_rgb888[color & 0x7fff];
			}

			palette_entry = (data & 0xff00) >> 8;
			color = paletteram[palette_entry];

			if (!(color & 0x8000))
			{
				dst[(i * 2) + 1] = m_rgb555_to_rgb888[color & 0x7fff];
			}
		}
	}
}


bool spg_renderer_device::get_tile_info(uint32_t tilemap_rambase, uint32_t palettemap_rambase, uint32_t x0, uint32_t y0, uint32_t tile_count_x, uint32_t ctrl, uint32_t attr, uint16_t& tile, spg_renderer_device::blend_enable_t& blend, spg_renderer_device::flipx_t& flip_x, bool& flip_y, uint32_t& palette_offset, address_space &spc)
{
	uint32_t tile_address = x0 + (tile_count_x * y0);

	tile = (ctrl & 0x0004) ? spc.read_word(tilemap_rambase) : spc.read_word(tilemap_rambase + tile_address);

	if (!tile)
		return false;

	uint32_t tileattr = attr;
	uint32_t tilectrl = ctrl;
	if ((ctrl & 2) == 0)
	{   // -(1) bld(1) flip(2) pal(4)

		uint16_t palette = (ctrl & 0x0004) ? spc.read_word(palettemap_rambase) : spc.read_word(palettemap_rambase + tile_address / 2);
		if (x0 & 1)
			palette >>= 8;
		else
			palette &= 0x00ff;


		tileattr &= ~0x000c;
		tileattr |= (palette >> 2) & 0x000c;    // flip

		tileattr &= ~0x0f00;
		tileattr |= (palette << 8) & 0x0f00;    // palette

		tilectrl &= ~0x0100;
		tilectrl |= (palette << 2) & 0x0100;    // blend
	}

	blend = ((tileattr & 0x4000 || tilectrl & 0x0100)) ? BlendOn : BlendOff;
	flip_x = (tileattr & 0x0004) ? FlipXOn : FlipXOff;
	flip_y= (tileattr & 0x0008);

	palette_offset = (tileattr & 0x0f00) >> 4;


	return true;
}


// this builds up a line table for the vcmp effect, this is not correct when step is used
void spg_renderer_device::update_vcmp_table()
{
	int currentline = 0;

	int step = m_video_regs_1e & 0xff;
	if (step & 0x80)
		step = step - 0x100;

	int current_inc_value = (m_video_regs_1c<<4);

	int counter = 0;

	for (int i = 0; i < 480; i++)
	{
		if (i < m_video_regs_1d)
		{
			m_ycmp_table[i] = 0xffffffff;
		}
		else
		{
			if ((currentline >= 0) && (currentline < 240))
			{
				m_ycmp_table[i] = currentline;
			}
			
			counter += current_inc_value;

			while (counter >= (0x20<<4))
			{
				currentline++;
				current_inc_value += step;

				counter -= (0x20<<4);
			}
		}
	}
}

void spg_renderer_device::draw_tilestrip(spg_renderer_device::blend_enable_t blend, spg_renderer_device::flipx_t flip_x, const rectangle& cliprect, uint32_t* dst, uint32_t tile_h, uint32_t tile_w, uint32_t tilegfxdata_addr, uint16_t tile, uint32_t tile_scanline, int drawx, bool flip_y, uint32_t palette_offset, const uint32_t nc_bpp, const uint32_t bits_per_row, const uint32_t words_per_tile, address_space& spc, uint16_t* paletteram)
{
	if (blend)
	{
		if (flip_x)
		{
			draw_tilestrip<BlendOn, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram);
		}
		else
		{
			draw_tilestrip<BlendOn, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram);
		}
	}
	else
	{
		if (flip_x)
		{
			draw_tilestrip<BlendOff, FlipXOn>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram);
		}
		else
		{
			draw_tilestrip<BlendOff, FlipXOff>(cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram);
		}
	}
}

void spg_renderer_device::draw_page(const rectangle& cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t tilegfxdata_addr, uint16_t* scrollregs, uint16_t* tilemapregs, address_space& spc, uint16_t* paletteram, uint16_t* scrollram)
{
	const uint32_t attr = tilemapregs[0];
	const uint32_t ctrl = tilemapregs[1];

	if (!(ctrl & 0x0008))
	{
		return;
	}

	if (((attr & 0x3000) >> 12) != priority)
	{
		return;
	}

	if (ctrl & 0x0001) // Bitmap / Linemap mode! (basically screen width tile mode)
	{
		draw_linemap(cliprect, dst, scanline, priority, tilegfxdata_addr, scrollregs, tilemapregs, spc, paletteram);
		return;
	}

	uint32_t logical_scanline = scanline;

	if (ctrl & 0x0040) // 'vertical compression feature' (later models only?)
	{
		if (m_video_regs_1e != 0x0000)
			popmessage("vertical compression mode with non-0 step amount %04x offset %04x step %04x\n", m_video_regs_1c, m_video_regs_1d, m_video_regs_1e);
		
		logical_scanline = m_ycmp_table[scanline];
		if (logical_scanline == 0xffffffff)
			return;
	}

	const uint32_t xscroll = scrollregs[0];
	const uint32_t yscroll = scrollregs[1];
	const uint32_t tilemap_rambase = tilemapregs[2];
	const uint32_t palettemap_rambase = tilemapregs[3];
	const int tile_width = (attr & 0x0030) >> 4;
	const uint32_t tile_h = 8 << ((attr & 0x00c0) >> 6);
	const uint32_t tile_w = 8 << (tile_width);
	const uint32_t tile_count_x = 512 / tile_w; // all tilemaps are 512 pixels wide
	const uint32_t bitmap_y = (logical_scanline + yscroll) & 0xff; // all tilemaps are 256 pixels high
	const uint32_t y0 = bitmap_y / tile_h;
	const uint32_t tile_scanline = bitmap_y % tile_h;
	const uint8_t bpp = attr & 0x0003;
	const uint32_t nc_bpp = ((bpp)+1) << 1;
	const uint32_t bits_per_row = nc_bpp * tile_w / 16;
	const uint32_t words_per_tile = bits_per_row * tile_h;
	const bool row_scroll = (ctrl & 0x0010);

	int realxscroll = xscroll;
	if (row_scroll)
	{
		// Tennis in My Wireless Sports confirms the need to add the scroll value here rather than rowscroll being screen-aligned
		realxscroll += (int16_t)scrollram[(logical_scanline+yscroll) & 0xff];
	}

	const int upperscrollbits = (realxscroll >> (tile_width + 3));
	const int endpos = (320 + tile_w) / tile_w;
	for (uint32_t x0 = 0; x0 < endpos; x0++)
	{
		spg_renderer_device::blend_enable_t blend;
		spg_renderer_device::flipx_t flip_x;
		bool flip_y;
		uint16_t tile;
		uint32_t palette_offset;

		if (!get_tile_info(tilemap_rambase, palettemap_rambase, (x0 + upperscrollbits) & (tile_count_x-1) , y0, tile_count_x, ctrl, attr, tile, blend, flip_x, flip_y, palette_offset, spc))
			continue;

		palette_offset >>= nc_bpp;
		palette_offset <<= nc_bpp;

		const int drawx = (x0 * tile_w) - (realxscroll & (tile_w-1));
		draw_tilestrip(blend,flip_x, cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, tile_scanline, drawx, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram);
	}
}

void spg_renderer_device::draw_sprite(const rectangle& cliprect, uint32_t* dst, uint32_t scanline, int priority, uint32_t base_addr, address_space &spc, uint16_t* paletteram, uint16_t* spriteram)
{
	uint32_t tilegfxdata_addr = 0x40 * m_video_regs_22;
	uint16_t tile = spriteram[base_addr + 0];
	int16_t x = spriteram[base_addr + 1];
	int16_t y = spriteram[base_addr + 2];
	uint16_t attr = spriteram[base_addr + 3];

	if (!tile)
	{
		return;
	}

	if (((attr & 0x3000) >> 12) != priority)
	{
		return;
	}

	const uint32_t tile_h = 8 << ((attr & 0x00c0) >> 6);
	const uint32_t tile_w = 8 << ((attr & 0x0030) >> 4);

	if (!(m_video_regs_42 & 0x0002))
	{
		x = (160 + x) - tile_w / 2;
		y = (120 - y) - (tile_h / 2) + 8;
	}

	x &= 0x01ff;
	y &= 0x01ff;

	int firstline = y;
	int lastline = y + (tile_h - 1);
	lastline &= 0x1ff;

	const spg_renderer_device::blend_enable_t blend = (attr & 0x4000) ? BlendOn : BlendOff;
	const spg_renderer_device::flipx_t flip_x = (attr & 0x0004) ? FlipXOn : FlipXOff;
	const uint8_t bpp = attr & 0x0003;
	const uint32_t nc_bpp = ((bpp)+1) << 1;
	const uint32_t bits_per_row = nc_bpp * tile_w / 16;
	const uint32_t words_per_tile = bits_per_row * tile_h;

	bool flip_y = (attr & 0x0008);
	uint32_t palette_offset = (attr & 0x0f00) >> 4;
	
	// the Circuit Racing game in PDC100 needs this or some graphics have bad colours at the edges when turning as it leaves stray lower bits set
	palette_offset >>= nc_bpp;
	palette_offset <<= nc_bpp;


	if (firstline < lastline)
	{
		int scanx = scanline - firstline;

		if ((scanx >= 0) && (scanline <= lastline))
		{
			draw_tilestrip(blend, flip_x, cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram);
		}
	}
	else
	{
		// clipped from top
		int tempfirstline = firstline - 0x200;
		int templastline = lastline;
		int scanx = scanline - tempfirstline;

		if ((scanx >= 0) && (scanline <= templastline))
		{
			draw_tilestrip(blend, flip_x, cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram);
		}
		// clipped against the bottom
		tempfirstline = firstline;
		templastline = lastline + 0x200;
		scanx = scanline - tempfirstline;

		if ((scanx >= 0) && (scanline <= templastline))
		{
			draw_tilestrip(blend, flip_x, cliprect, dst, tile_h, tile_w, tilegfxdata_addr, tile, scanx, x, flip_y, palette_offset, nc_bpp, bits_per_row, words_per_tile, spc, paletteram);
		}
	}
}

void spg_renderer_device::draw_sprites(const rectangle &cliprect, uint32_t* dst, uint32_t scanline, int priority, address_space &spc, uint16_t* paletteram, uint16_t* spriteram, int sprlimit)
{
	if (!(m_video_regs_42 & 0x0001))
	{
		return;
	}

	for (uint32_t n = 0; n < sprlimit; n++)
	{
		draw_sprite(cliprect, dst, scanline, priority, 4 * n, spc, paletteram, spriteram);
	}
}


void spg_renderer_device::apply_saturation_and_fade(bitmap_rgb32& bitmap, const rectangle& cliprect, int scanline)
{
	static const float s_u8_to_float = 1.0f / 255.0f;
	static const float s_gray_r = 0.299f;
	static const float s_gray_g = 0.587f;
	static const float s_gray_b = 0.114f;
	const float sat_adjust = (0xff - (m_video_regs_3c & 0x00ff)) / (float)(0xff - 0x20);

	const uint16_t fade_offset = m_video_regs_30;

	uint32_t* src = &bitmap.pix32(scanline, cliprect.min_x);

	for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
	{
		if ((m_video_regs_3c & 0x00ff) != 0x0020) // apply saturation
		{
			const uint32_t src_rgb = *src;
			const float src_r = (uint8_t)(src_rgb >> 16) * s_u8_to_float;
			const float src_g = (uint8_t)(src_rgb >> 8) * s_u8_to_float;
			const float src_b = (uint8_t)(src_rgb >> 0) * s_u8_to_float;
			const float luma = src_r * s_gray_r + src_g * s_gray_g + src_b * s_gray_b;
			const float adjusted_r = luma + (src_r - luma) * sat_adjust;
			const float adjusted_g = luma + (src_g - luma) * sat_adjust;
			const float adjusted_b = luma + (src_b - luma) * sat_adjust;
			const int integer_r = (int)floor(adjusted_r * 255.0f);
			const int integer_g = (int)floor(adjusted_g * 255.0f);
			const int integer_b = (int)floor(adjusted_b * 255.0f);
			*src = (integer_r > 255 ? 0xff0000 : (integer_r < 0 ? 0 : ((uint8_t)integer_r << 16))) |
				(integer_g > 255 ? 0x00ff00 : (integer_g < 0 ? 0 : ((uint8_t)integer_g << 8))) |
				(integer_b > 255 ? 0x0000ff : (integer_b < 0 ? 0 : (uint8_t)integer_b));

		}

		if (fade_offset != 0) // apply fade
		{
			const uint32_t src_rgb = *src;
			const uint8_t src_r = (src_rgb >> 16) & 0xff;
			const uint8_t src_g = (src_rgb >> 8) & 0xff;
			const uint8_t src_b = (src_rgb >> 0) & 0xff;
			const uint8_t r = src_r - fade_offset;
			const uint8_t g = src_g - fade_offset;
			const uint8_t b = src_b - fade_offset;
			*src = (r > src_r ? 0 : (r << 16)) |
				(g > src_g ? 0 : (g << 8)) |
				(b > src_b ? 0 : (b << 0));
		}

		src++;
	}
}
