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

#include "common/file.h"
#include "common/memstream.h"
#include "bagel/hodjnpodj/hnplibs/lzss.h"

namespace Bagel {
namespace HodjNPodj {

Common::SeekableReadStream *makeLzssStream(const Common::Path &filename) {
	Common::File file;
	if (!file.open(filename))
		return nullptr;

	// First part of the signature
	if (file.readUint32BE() != MKTAG('S', 'Z', 'D', 'D'))
		return nullptr;

	// Second part of the signature
	if (file.readUint32BE() != 0x88F02733)
		return nullptr;

	// Compression mode must be 'A'
	if (file.readByte() != 'A')
		return nullptr;

	file.readByte(); // file name character change
	uint32 unpackedLength = file.readUint32LE();

	byte *window = new byte[0x1000]();
	int pos = 0x1000 - 16;

	byte *unpackedData = (byte *)malloc(unpackedLength);
	assert(unpackedData);
	byte *dataPos = unpackedData;

	uint32 remaining = unpackedLength;

	// Apply simple LZSS decompression
	for (;;) {
		byte controlByte = file.readByte();

		if (remaining == 0 || file.eos())
			break;

		for (byte i = 0; i < 8; i++) {
			if (controlByte & (1 << i)) {
				*dataPos++ = window[pos++] = file.readByte();
				pos &= 0xFFF;
				if (--remaining == 0)
					break;
			} else {
				int matchPos = file.readByte();
				int matchLen = file.readByte();
				matchPos |= (matchLen & 0xF0) << 4;
				matchLen = (matchLen & 0xF) + 3;
				if ((uint32)matchLen > remaining)
					matchLen = remaining;
				remaining -= matchLen;

				while (matchLen--) {
					*dataPos++ = window[pos++] = window[matchPos++];
					pos &= 0xFFF;
					matchPos &= 0xFFF;
				}

				if (remaining == 0)
					break;
			}
		}
	}

	delete[] window;
	return new Common::MemoryReadStream(unpackedData, unpackedLength);
}

} // namespace HodjNPodj
} // namespace Bagel
