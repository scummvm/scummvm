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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "image/codecs/indeo/get_bits.h"

namespace Image {
namespace Indeo {

int GetBits::getVLC2(int16 (*table)[2], int bits, int maxDepth) {
	int code;
	int n, nbBits;
	unsigned int index;

	index = peekBits(bits);
	code  = table[index][0];
	n     = table[index][1];

	if (maxDepth > 1 && n < 0) {
		skip(bits);
		nbBits = -n;

		index = peekBits(nbBits) + code;
		code = table[index][0];
		n = table[index][1];

		if (maxDepth > 2 && n < 0) {
			skip(nbBits);
			nbBits = -n;

			index = peekBits(nbBits) + code;
			code = table[index][0];
			n = table[index][1];
		}
	}

	skip(n);
	return code;
}

} // End of namespace Indeo
} // End of namespace Image
