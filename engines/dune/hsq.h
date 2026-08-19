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

#ifndef DUNE_HSQ_H
#define DUNE_HSQ_H

#include "common/stream.h"

namespace Dune {

static inline
byte getbit(uint16 &queue, Common::ReadStream *src) {
	byte bit;

	bit = queue & 1;
	queue >>= 1;

	if (queue == 0) {
		queue = src->readUint16LE();
		bit = queue & 1;
		queue = 0x8000 | (queue >> 1);
	}

	return bit;
}

template <typename T>
void decompressHSQ(Common::ReadStream *src, int packedLength, T buf, int unpackedLength) {
	uint16 queue = 0;
	int dst = 0;

	for (;;) {
		if (getbit(queue, src)) {
			assert(dst < unpackedLength);
			buf[dst++] = src->readByte();
		} else {
			int count;
			int offset;

			if (getbit(queue, src)) {
				uint16 word = src->readUint16LE();

				count  = word & 0x07;
				offset = (word >> 3) - 8192;
				if (count == 0) {
					count = src->readByte();
				}
				if (count == 0) {
					break;
				}
			} else {
				byte b0 = getbit(queue, src);
				byte b1 = getbit(queue, src);

				count  = 2*b0 + b1;
				offset = int(src->readByte()) - 256;
			}
			assert(dst+count+2 < unpackedLength);
			for (int i = 0; i != count+2; ++i) {
				buf[dst+i] = buf[dst+i+offset];
			}
			dst += count + 2;
		}
	}
}

} // End of namespace Dune

#endif
