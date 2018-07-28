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
 * $URL: https://scummvm-startrek.googlecode.com/svn/trunk/lzss.cpp $
 * $Id: lzss.cpp 18 2010-12-17 01:22:54Z clone2727 $
 *
 */

#include "common/textconsole.h"
#include "common/memstream.h"
#include "common/util.h"

#include "startrek/lzss.h"

namespace StarTrek {

Common::SeekableReadStream *decodeLZSS(Common::SeekableReadStream *indata, uint32 uncompressedSize) {
	uint32 N = 0x1000; /* History buffer size */
	byte *histbuff = new byte[N]; /* History buffer */
	memset(histbuff, 0, N);
	uint32 outstreampos = 0;
	uint32 bufpos = 0;
	byte *outLzssBufData = (byte *)malloc(uncompressedSize);

	for (;;) {
		byte flagbyte = indata->readByte();

		if (indata->eos())
			break;

		for (byte i = 0; i < 8; i++) {
			if ((flagbyte & (1 << i)) == 0) {
				uint32 offsetlen = indata->readUint16LE();

				if (indata->eos())
					break;

				uint32 length = (offsetlen & 0xF) + 3;
				uint32 offset = (bufpos - (offsetlen >> 4)) & (N - 1);
				for (uint32 j = 0; j < length; j++) {
					byte tempa = histbuff[(offset + j) & (N - 1)];
					outLzssBufData[outstreampos++] = tempa;
					histbuff[bufpos] = tempa;
					bufpos = (bufpos + 1) & (N - 1);
				}
			} else {
				byte tempa = indata->readByte();

				if (indata->eos())
					break;

				outLzssBufData[outstreampos++] = tempa;
				histbuff[bufpos] = tempa;
				bufpos = (bufpos + 1) & (N - 1);
			}
		}
	}

	delete[] histbuff;

	if (outstreampos != uncompressedSize)
		error("Size mismatch in LZSS decompression; expected %d bytes, got %d bytes", uncompressedSize, outstreampos);

	return new Common::MemoryReadStream(outLzssBufData, uncompressedSize, DisposeAfterUse::YES);
}

} // End of namespace StarTrek

