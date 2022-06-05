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

#include "common/str.h"
#include "glk/scott/decompress_text.h"

namespace Glk {
namespace Scott {

int rotateLeftWithCarry(uint8_t *byte, int lastCarry) {
	int carry = ((*byte & 0x80) > 0);
	*byte = *byte << 1;
	if (lastCarry)
		*byte = *byte | 0x01;
	return carry;
}

int decompressOne(uint8_t *bytes) {
	uint8_t result = 0;
	int carry;
	for (int i = 0; i < 5; i++) {
		carry = 0;
		for (int j = 0; j < 5; j++) {
			carry = rotateLeftWithCarry(bytes + 4 - j, carry);
		}
		rotateLeftWithCarry(&result, carry);
	}
	return result;
}

char *decompressText(uint8_t *source, int stringIndex) {
	// Lookup table
	Common::String alphabet = " abcdefghijklmnopqrstuvwxyz'\x01,.\x00";

	int pos, c, uppercase, i, j;
	uint8_t decompressed[256];
	uint8_t buffer[5];
	int idx = 0;

	// Find the start of the compressed message
	for (i = 0; i < stringIndex; i++) {
		pos = *source;
		pos = pos & 0x7F;
		source += pos;
	};

	uppercase = ((*source & 0x40) == 0); // Test bit 6

	source++;
	do {
		// Get five compressed bytes
		for (i = 0; i < 5; i++) {
			buffer[i] = *source++;
		}
		for (j = 0; j < 8; j++) {
			// Decompress one character:
			int next = decompressOne(buffer);

			c = alphabet[next];

			if (c == 0x01) {
				uppercase = 1;
				c = ' ';
			}

			if (c >= 'a' && uppercase) {
				c = toupper(c);
				uppercase = 0;
			}
			decompressed[idx++] = c;

			if (idx > 255)
				return nullptr;

			if (idx == 255)
				c = 0; // We've gone too far, return

			if (c == 0) {
				char *result = new char[idx];
				memcpy(result, decompressed, idx);
				return result;
			} else if (c == '.' || c == ',') {
				if (c == '.')
					uppercase = 1;
				decompressed[idx++] = ' ';
			}
		}
	} while (idx < 0xff); // Chosen arbitrarily, might be too small
	return nullptr;
}

} // End of namespace Scott
} // End of namespace Glk
