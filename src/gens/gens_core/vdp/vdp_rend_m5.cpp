/***************************************************************************
 * Gens: VDP Renderer. (Mode 5)                                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "vdp_rend_m5.hpp"
#include "vdp_rend.h"
#include "vdp_io.h"

#include "emulator/g_main.hpp"

// C includes.
#include <stdint.h>
#include <string.h>


// Line buffer for current line.
// TODO: Mark as static once VDP_Render_Line_m5_asm is ported to C.
typedef union
{
	uint8_t  u8[336<<1];
	uint16_t u16[336];
	uint32_t u32[336>>1];
} LineBuf_t;
LineBuf_t LineBuf;


// asm rendering code.
extern "C" void VDP_Render_Line_m5_asm(void);

VDP_Data_Misc_t VDP_Data_Misc;


/**
 * T_Make_Sprite_Struct(): Fill Sprite_Struct[] with information from the Sprite Attribute Table.
 * @param interlaced If true, using Interlaced Mode 2. (2x res)
 * @param partial If true, only do a partial update. (X pos, X size)
 */
template<bool interlaced, bool partial>
static inline void T_Make_Sprite_Struct(void)
{
	uint16_t *CurSpr = (uint16_t*)Spr_Addr;
	unsigned int spr_num = 0;
	unsigned int link;
	
	// H40 allows 80 sprites; H32 allows 64 sprites.
	// TODO: Test 0x81 instead of 0x01?
	const unsigned int max_spr = (VDP_Reg.Set4 & 0x01) ? 80 : 64;
	
	do
	{
		// Sprite position.
		Sprite_Struct[spr_num].Pos_X = (*(CurSpr + 3) & 0x1FF) - 128;
		if (!partial)
		{
			if (interlaced)
			{
				// TODO: Don't do this!
				// Use proper interlaced mode instead.
				
				// Interlaced: Y-pos is divided by 2.
				Sprite_Struct[spr_num].Pos_Y = ((*CurSpr & 0x3FF) / 2) - 128;
			}
			else
			{
				// Non-Interlaced. Y-pos is kept as-is.
				Sprite_Struct[spr_num].Pos_Y = (*CurSpr & 0x1FF) - 128;
			}
		}
		
		// Sprite size.
		uint8_t sz = (*(CurSpr + 1) >> 8);
		Sprite_Struct[spr_num].Size_X = ((sz >> 2) & 3) + 1;	// 1 more than the original value.
		if (!partial)
			Sprite_Struct[spr_num].Size_Y = sz & 3;		// Exactly the original value.
		
		// Determine the maximum positions.
		Sprite_Struct[spr_num].Pos_X_Max =
				Sprite_Struct[spr_num].Pos_X +
				((Sprite_Struct[spr_num].Size_X * 8) - 1);
		
		if (!partial)
		{
			Sprite_Struct[spr_num].Pos_Y_Max =
					Sprite_Struct[spr_num].Pos_Y +
					((Sprite_Struct[spr_num].Size_Y * 8) + 7);
			
			// Tile number. (Also includes palette, priority, and flip bits.)
			Sprite_Struct[spr_num].Num_Tile = *(CurSpr + 2);
		}
		
		// Link number.
		link = (*(CurSpr + 1) & 0x7F);
		
		// Increment the sprite number.
		spr_num++;
		
		if (link == 0)
			break;
		
		// Go to the next sprite.
		CurSpr = ((uint16_t*)Spr_Addr) + (link * (8>>1));
		
		// Stop processing after:
		// - Link number is 0. (checked above)
		// - Link number exceeds maximum number of sprites.
		// - We've processed the maximum number of sprites.
	} while (link < max_spr && spr_num < max_spr);
	
	// Store the byte index of the last sprite.
	if (!partial)
		VDP_Data_Misc.Spr_End = (spr_num - 1) * sizeof(Sprite_Struct_t);
}


/**
 * C wrapper functions for T_Make_Sprite_Struct().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
void Make_Sprite_Struct(void);
void Make_Sprite_Struct_Interlaced(void);
void Make_Sprite_Struct_Partial(void);
}

void Make_Sprite_Struct(void)
{
	T_Make_Sprite_Struct<false, false>();
}
void Make_Sprite_Struct_Interlaced(void)
{
	T_Make_Sprite_Struct<true, false>();
}
void Make_Sprite_Struct_Partial(void)
{
	T_Make_Sprite_Struct<false, true>();
}


/**
 * T_Get_X_Offset(): Get the X offset for the line. (Horizontal Scroll Table)
 * @param plane True for Scroll A; false for Scroll B.
 * @return X offset.
 */
template<bool plane>
static inline uint16_t T_Get_X_Offset(void)
{
	const unsigned int H_Scroll_Offset = (VDP_Current_Line & H_Scroll_Mask) * 2;
	
	if (plane)
	{
		// Scroll A.
		return H_Scroll_Addr[H_Scroll_Offset];
	}
	else
	{
		// Scroll B.
		return H_Scroll_Addr[H_Scroll_Offset + 1];
	}
}

/**
 * C wrapper functions for T_Get_X_Offset().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	uint16_t Get_X_Offset_ScrollA(void);
	uint16_t Get_X_Offset_ScrollB(void);
}

uint16_t Get_X_Offset_ScrollA(void)
{
	return T_Get_X_Offset<true>();
}
uint16_t Get_X_Offset_ScrollB(void)
{
	return T_Get_X_Offset<false>();
}


/**
 * T_Update_Y_Offset(): Update the Y offset.
 * @param plane True for Scroll A; false for Scroll B.
 * @param interlaced True for interlaced; false for non-interlaced.
 * @param cur Current Y offset. (Returned if we're outside of VRam limits.)
 * @return Y offset.
 */
template<bool plane, bool interlaced>
static inline unsigned int T_Update_Y_Offset(unsigned int cur)
{
	// TODO: This function is untested!
	// Find a ROM that uses 2-cell VScroll to test it.
	
	if (VDP_Data_Misc.Cell & 0xFF81)
	{
		// Outside of VRam limits. Don't change anything.
		return cur;
	}
	
	// Get the vertical scroll offset.
	unsigned int VScroll_Offset = VDP_Data_Misc.Cell;
	if (plane)
	{
		// Scroll A.
		VScroll_Offset = VSRam.u16[VScroll_Offset];
	}
	else
	{
		// Scroll B.
		VScroll_Offset = VSRam.u16[VScroll_Offset + 1];
	}
	
	if (interlaced)
	{
		// Divide Y scroll by 2.
		// TODO: Don't do this! Handle interlaced mode properly.
		VScroll_Offset /= 2;
	}
	
	VScroll_Offset += VDP_Current_Line;
	VDP_Data_Misc.Line_7 = VScroll_Offset & 0x07;	// Pattern line.
	VScroll_Offset >>= 3;				// Get the V Cell offset.
	VScroll_Offset &= V_Scroll_CMask;		// Prevent V Cell offset from overflowing.
	return VScroll_Offset;
}

/**
 * C wrapper functions for T_Update_Y_Offset().
 * TODO: Remove these once vdp_rend_m5_x86.asm is fully ported to C++.
 */
extern "C" {
	unsigned int Update_Y_Offset_ScrollA(unsigned int cur);
	unsigned int Update_Y_Offset_ScrollB(unsigned int cur);
	unsigned int Update_Y_Offset_ScrollA_Interlaced(unsigned int cur);
	unsigned int Update_Y_Offset_ScrollB_Interlaced(unsigned int cur);
}

unsigned int Update_Y_Offset_ScrollA(unsigned int cur)
{
	return T_Update_Y_Offset<true, false>(cur);
}
unsigned int Update_Y_Offset_ScrollB(unsigned int cur)
{
	return T_Update_Y_Offset<false, false>(cur);
}
unsigned int Update_Y_Offset_ScrollA_Interlaced(unsigned int cur)
{
	return T_Update_Y_Offset<true, true>(cur);
}
unsigned int Update_Y_Offset_ScrollB_Interlaced(unsigned int cur)
{
	return T_Update_Y_Offset<false, true>(cur);
}


/**
 * T_Render_LineBuf(): Render the line buffer to the destination surface.
 * @param pixel Type of pixel.
 * @param md_palette MD palette buffer.
 * @param num_px Number of pixels to render.
 * @param src Source. (Line buffer)
 * @param dest Destination surface.
 */
template<typename pixel, pixel *md_palette>
static inline void T_Render_LineBuf(unsigned int num_px, uint8_t *src, pixel *dest)
{
	// Render the line buffer to the destination surface.
	// Line buffer is accessed using bytes for some reason.
	for (unsigned int i = (num_px / 4); i != 0; i--)
	{
		// TODO: Endianness conversions.
		*dest     = md_palette[*src];
		*(dest+1) = md_palette[*(src+2)];
		*(dest+2) = md_palette[*(src+4)];
		*(dest+3) = md_palette[*(src+6)];
			
		dest += 4;
		src += 8;
	}
}


/**
 * VDP_Render_Line_m5(): Render a line. (Mode 5)
 */
void VDP_Render_Line_m5(void)
{
	// Check if the VDP is active.
	if (!(VDP_Reg.Set2 & 0x40))
	{
		// VDP isn't active. Clear the line buffer.
		if (VDP_Reg.Set4 & 0x08)
		{
			// Shadow/Highlight is enabled. Clear with 0x40.
			memset(LineBuf.u8, 0x40, sizeof(LineBuf.u8));
		}
		else
		{
			// Shadow/Highlight is disabled. Clear with 0x00.
			memset(LineBuf.u8, 0x00, sizeof(LineBuf.u8));
		}
	}
	else
	{
		// VDP is active.
		VDP_Render_Line_m5_asm();
	}
	
	// Check if the palette was modified.
	if (VDP_Flags.CRam)
	{
		// Update the palette.
		if (VDP_Reg.Set4 & 0x08)
			VDP_Update_Palette_HS();
		else
			VDP_Update_Palette();
	}
	
	// Render the image.
	const unsigned int num_px = (160 - H_Pix_Begin) * 2;
	const unsigned int LineStart = (TAB336[VDP_Current_Line] + 8);
	
	if (bppMD == 32)
	{
		T_Render_LineBuf<uint32_t, MD_Palette32>
				(num_px, &LineBuf.u8[8<<1], &MD_Screen32[LineStart]);
	}
	else
	{
		T_Render_LineBuf<uint16_t, MD_Palette>
				(num_px, &LineBuf.u8[8<<1], &MD_Screen[LineStart]);
	}
}
