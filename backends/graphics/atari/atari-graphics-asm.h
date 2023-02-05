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
 * Set Atari TT palette.
 * @param pPalette 256 palette entries (0000 RRRR GGGG BBBB)
 */
void asm_screen_set_tt_palette(const uint16 pPalette[256]);
/**
 * Set Atari Falcon palette.
 * @param pPalette 256 palette entries (RRRRRRrr GGGGGGgg 00000000 BBBBBBbb)
 */
void asm_screen_set_falcon_palette(const uint32 pPalette[256]);

/**
 * Set Atari TT/Falcon video base.
 */
void asm_screen_set_vram(const void* pScreen);

/**
 * Set Atari Falcon Videl resolution (Screenspain's SCP format).
 */
void asm_screen_set_scp_res(const void* pScp);

}

#endif
