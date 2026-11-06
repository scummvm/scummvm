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

#include "tesoro/core/namekey.h"

namespace Tesoro {

static uint32 charBits(char c) {
	uint32 value = (unsigned char)c;
	if ((int8)c < '5')
		value += 11;
	return value & 0x1f;
}

uint64 nameKey64(const char *name) {
	uint32 half[2] = { 0, 0 };
	for (int h = 0; h < 2; ++h) {
		for (int i = 0; i < 6; ++i) {
			const char c = name[h * 6 + i];
			if (c == '\0') {
				half[h] <<= 30 - 5 * i;
				return (uint64(half[1]) << 32) | half[0];
			}
			half[h] = (half[h] << 5) | charBits(c);
		}
	}
	return (uint64(half[1]) << 32) | half[0];
}

void nameFromKey(uint64 key, char *out) {
	for (int h = 0; h < 2; ++h) {
		uint32 half = h ? uint32(key >> 32) : uint32(key);
		for (int i = 5; i >= 0; --i) {
			const uint32 bits = half & 0x1f;
			out[h * 6 + i] = bits == 0 ? ' ' : bits < 0x1b ? char(bits + 0x40) : char(bits + 0x15);
			half >>= 5;
		}
	}
	out[12] = '\0';
}

} // End of namespace Tesoro
