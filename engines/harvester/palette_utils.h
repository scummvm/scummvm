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

#ifndef HARVESTER_PALETTE_UTILS_H
#define HARVESTER_PALETTE_UTILS_H

#include <cstring>

#include "common/scummsys.h"

namespace Harvester {

inline byte expandHarvesterVgaDacColor(byte value) {
	// Match VGA/DOSBox-X DAC expansion: replicate the high 2 bits into the low 2 bits.
	return (value << 2) | (value >> 4);
}

inline void buildHarvesterDisplayPalette(const byte *source, float brightness, byte *dest) {
	memset(dest, 0, 256 * 3);
	if (!source)
		return;

	if (brightness < 0.0f)
		brightness = 0.0f;

	for (uint color = 1; color < 256; ++color) {
		for (uint channel = 0; channel < 3; ++channel) {
			const uint index = color * 3 + channel;
			const int vgaDacValue = source[index] >> 2;
			int scaled = (int)(vgaDacValue * brightness + 0.5f);
			if (scaled > 63)
				scaled = 63;
			dest[index] = expandHarvesterVgaDacColor((byte)scaled);
		}
	}
}

} // End of namespace Harvester

#endif // HARVESTER_PALETTE_UTILS_H
