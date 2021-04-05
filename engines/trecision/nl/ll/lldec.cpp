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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/array.h"
#include "common/scummsys.h"

#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/trecision.h"

#define FAST_COOKIE 0xFA57F00D

namespace Trecision {

void decompress(const unsigned char *src, unsigned src_len, unsigned char *dst, unsigned dst_len) {
	unsigned short *sw = (unsigned short *)(src + src_len);
	unsigned char *d = dst;
	const unsigned char *s = src;
	unsigned short ctrl = 0, ctrl_cnt = 1;

	while (s < (const unsigned char *)sw) {
		if (!--ctrl_cnt) {
			ctrl = *--sw;
			ctrl_cnt = 16;
		} else {
			ctrl <<= 1;
		}

		if (ctrl & 0x8000) {
			unsigned foo = *--sw;
			const unsigned char *cs = d - (foo >> 4);

			switch (foo & 0xF) {
			case 0:
				*d++ = *cs++;
			case 1:
				*d++ = *cs++;
			case 2:
				*d++ = *cs++;
			case 3:
				*d++ = *cs++;
			case 4:
				*d++ = *cs++;
			case 5:
				*d++ = *cs++;
			case 6:
				*d++ = *cs++;
			case 7:
				*d++ = *cs++;
			case 8:
				*d++ = *cs++;
			case 9:
				*d++ = *cs++;
			case 10:
				*d++ = *cs++;
			case 11:
				*d++ = *cs++;
			case 12:
				*d++ = *cs++;
			case 13:
				*d++ = *cs++;
			case 14:
				*d++ = *cs++;
			case 15:
				*d++ = *cs++;

				*d++ = *cs++;
				*d++ = *cs++;
			}
		} else {
			*d++ = *s++;
		}
	}
}

uint32 DecCR(Common::String fileName, uint8 *DestArea, uint8 *DecArea) {
	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember(fileName);
	if (ff == nullptr)
		error("File not found %s", fileName.c_str());

	uint8 *ibuf = DecArea;
	uint8 *obuf = DestArea;

	int dataSize = ff->size() - 8;
	uint32 signature = ff->readUint32LE();
	if (signature != FAST_COOKIE)
		error("DecCR - %s has a bad signature and can't be loaded", fileName.c_str());

	uint32 decompSize = ff->readUint32LE();
	ff->read(ibuf, dataSize);
	delete ff;

	if (dataSize < decompSize)
		decompress(ibuf, dataSize, obuf, decompSize);
	else
		memcpy(obuf, ibuf, dataSize);

	return decompSize;
}

} // End of namespace Trecision
