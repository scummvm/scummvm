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

#ifndef BACKENDS_GRAPHICS_ATARI_C2P_ASM_H
#define BACKENDS_GRAPHICS_ATARI_C2P_ASM_H

#include "common/scummsys.h"

extern "C" {

/**
 * Chunky to planar conversion routine. Converts a chunky (byte) buffer into eight bitplanes.
 * Optimized for surface-to-surface copy with the same pitch.
 *
 * @param pChunky chunky buffer start
 * @param pChunkyEnd chunky buffer end (including the last byte)
 * @param pScreen bitplane screen start
 */
void asm_c2p1x1_8(const byte *pChunky, const byte *pChunkyEnd, byte *pScreen);

/**
 * Chunky to planar conversion routine. Converts a chunky (byte) buffer into eight bitplanes.
 * Optimized for arbitrary rectangle position and dimension (16px aligned).
 *
 * @param pChunky chunky buffer at rectangle's [X1, Y1] position
 * @param pChunkyEnd chunky buffer at rectangle's [X2, Y2] position (included)
 * @param chunkyWidth rectangle width
 * @param chunkyPitch chunky buffer width (in bytes)
 * @param pScreen bitplane screen at rectangle's [X1, Y1] position
 * @param screenPitch bitplane screen width (in bytes)
 */
void asm_c2p1x1_8_rect(const byte *pChunky, const byte *pChunkyEnd, uint32 chunkyWidth, uint32 chunkyPitch, byte *pScreen, uint32 screenPitch);

}

#endif
