/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKENDS_GRAPHICS_ATARI_ASM_H
#define BACKENDS_GRAPHICS_ATARI_ASM_H

#include "common/scummsys.h"

extern "C" {

/**
 * Save Atari TT video registers.
 */
void asm_screen_tt_save(void);
/**
 * Save Atari Falcon video registers.
 */
void asm_screen_falcon_save(void);

/**
 * Restore Atari TT video registers.
 */
void asm_screen_tt_restore(void);
/**
 * Restore Atari Falcon video registers.
 */
void asm_screen_falcon_restore(void);

/**
 * Copy 4bpl sprite into 4bpl buffer. Sprite's width must be multiply of 16.
 *
 * @param dstBuffer destination buffer (four bitplanes)
 * @param srcBuffer source buffer (four bitplanes)
 * @param srcMask source mask (one bitplane)
 * @param destX sprite's X position (in pixels)
 * @param destY sprite's Y position (in pixels)
 * @param dstPitch destination buffer's pitch (in bytes)
 * @param w sprite's width (in pixels)
 * @param h sprite's height (in pixels)
 */
void asm_draw_4bpl_sprite(uint16 *dstBuffer, const uint16 *srcBuffer, const uint16 *srcMask,
						  uint destX, uint destY, uint dstPitch, uint w, uint h);
/**
 * Copy 8bpl sprite into 8bpl buffer. Sprite's width must be multiply of 16.
 *
 * @param dstBuffer destination buffer (eight bitplanes)
 * @param srcBuffer source buffer (eight bitplanes)
 * @param srcMask source mask (one bitplane)
 * @param destX sprite's X position (in pixels)
 * @param destY sprite's Y position (in pixels)
 * @param dstPitch destination buffer's pitch (in bytes)
 * @param w sprite's width (in pixels)
 * @param h sprite's height (in pixels)
 */
void asm_draw_8bpl_sprite(uint16 *dstBuffer, const uint16 *srcBuffer, const uint16 *srcMask,
						  uint destX, uint destY, uint dstPitch, uint w, uint h);

}

#endif
