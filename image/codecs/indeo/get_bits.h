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

#ifndef IMAGE_CODECS_INDEO_GET_BITS_H
#define IMAGE_CODECS_INDEO_GET_BITS_H

#include "common/bitstream.h"

namespace Image {
namespace Indeo {

/**
 * Intel Indeo Bitstream reader
 */
class GetBits : public Common::BitStreamMemory8LSB {
public:
	/**
	* Constructor
	*/
	GetBits(const byte *dataPtr, uint32 dataSize) : Common::BitStreamMemory8LSB(new Common::BitStreamMemoryStream(dataPtr, dataSize), DisposeAfterUse::YES) {}

	/**
	 * The number of bits left
	 */
	int getBitsLeft() const { return size() - pos(); }

	/**
	 * Parse a VLC code.
	 * @param bits is the number of bits which will be read at once, must be
	 *             identical to nbBits in init_vlc()
	 * @param maxDepth is the number of times bits bits must be read to completely
	 *                  read the longest vlc code
	 *                  = (max_vlc_length + bits - 1) / bits
	 */
	template <int maxDepth, int bits>
	int getVLC2(int16 (*table)[2]) {
		int code;
		int n, nbBits;
		unsigned int index;

		index = peekBits<bits>();
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
};

} // End of namespace Indeo
} // End of namespace Image

#endif
