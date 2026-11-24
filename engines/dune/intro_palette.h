/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DUNE_INTRO_PALETTE_H
#define DUNE_INTRO_PALETTE_H

#include "common/scummsys.h"

namespace Dune {

inline uint introPaletteFrameCount(uint16 kind) {
	return kind == 0x36 ? 512 : kind == 0x38 ? 128 : 0;
}

// DNVGA 2628/264D and 26B0/26E3. Inputs and output are six-bit DAC values.
inline void introPaletteFrame(const byte *previous, const byte *next,
		byte *current, uint16 kind, uint frame) {
	if (frame >= introPaletteFrameCount(kind))
		return;
	for (uint i = 0; i < 768; i++) {
		if (kind == 0x36) {
			// 2628 clears colours 0..254, then 264D interpolates eight
			// chunks of 96 components over 64 rounds. Colour 255 is
			// already at its target in the palette being interpolated.
			if (i >= 765) {
				current[i] = frame < 7 ? previous[i] : next[i];
				continue;
			}
			const int steps = frame / 8 + (i / 96 <= frame % 8 ? 1 : 0);
			const int value = next[i] - (64 - steps);
			current[i] = value > 0 ? value : 0;
		} else {
			// 26E3 subtracts two from four chunks of 64 colours per
			// round. Until its first write a chunk still shows the DAC's
			// previous palette, which can differ from the staged target.
			const int steps = frame / 4 + (i / 192 <= frame % 4 ? 1 : 0);
			const int value = next[i] - 2 * steps;
			current[i] = steps ? (value > 0 ? value : 0) : previous[i];
		}
	}
}

} // End of namespace Dune

#endif
