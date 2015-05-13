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

#include "sherlock/decompress.h"

namespace Sherlock {

/**
 * Decompresses an LZW compressed resource. If no outSize is specified, it will
 * decompress the entire resource. If, however, an explicit size is specified,
 * then it means we're already within a resource, and only want to decompress
 * part of it.
 */
Common::SeekableReadStream *decompressLZ(Common::SeekableReadStream &source, int32 outSize) {
	if (outSize == -1) {
		source.seek(5);
		outSize = source.readSint32LE();
	}

	byte lzWindow[4096];
	uint16 lzWindowPos;
	uint16 cmd;

	byte *outBuffer = new byte[outSize];
	byte *outBufferEnd = outBuffer + outSize;
	Common::MemoryReadStream *outS = new Common::MemoryReadStream(outBuffer, outSize, DisposeAfterUse::YES);

	memset(lzWindow, 0xFF, 0xFEE);
	lzWindowPos = 0xFEE;
	cmd = 0;

	do {
		cmd >>= 1;
		if (!(cmd & 0x100))
			cmd = source.readByte() | 0xFF00;

		if (cmd & 1) {
			byte literal = source.readByte();
			*outBuffer++ = literal;
			lzWindow[lzWindowPos] = literal;
			lzWindowPos = (lzWindowPos + 1) & 0x0FFF;
		} else {
			int copyPos, copyLen;
			copyPos = source.readByte();
			copyLen = source.readByte();
			copyPos = copyPos | ((copyLen & 0xF0) << 4);
			copyLen = (copyLen & 0x0F) + 3;
			while (copyLen--) {
				byte literal = lzWindow[copyPos];
				copyPos = (copyPos + 1) & 0x0FFF;
				*outBuffer++ = literal;
				lzWindow[lzWindowPos] = literal;
				lzWindowPos = (lzWindowPos + 1) & 0x0FFF;
			}
		}
	} while (outBuffer < outBufferEnd);

	return outS;
}


/**
 * Decompresses a Rose Tattoo resource
 *
Common::SeekableReadStream *decompress32(Common::SeekableReadStream &source, int32 outSize) {
	if (outSize == -1) {
		outSize = source.readSint32LE();
	}

	byte lzWindow[8192];
	byte *outBuffer = new byte[outSize];
	byte *outBufferEnd = outBuffer + outSize;
	Common::MemoryReadStream *outS = new Common::MemoryReadStream(outBuffer, outSize, DisposeAfterUse::YES);

	memset(lzWindow, 0xFF, 8192);
	int lzWindowPos = 0xFEE;
	int cmd = 0;

	do {
		cmd >>= 1;
		if (!(cmd & 0x100))
			cmd = source.readByte() | 0xFF00;

		if (cmd & 1) {
			byte literal = source.readByte();
			*outBuffer++ = literal;
			lzWindow[lzWindowPos] = literal;
			lzWindowPos = (lzWindowPos + 1) & 0x0FFF;
		} else {
			int copyPos, copyLen;
			copyPos = source.readByte();
			copyLen = source.readByte();
			copyPos = copyPos | ((copyLen & 0xF0) << 4);
			copyLen = (copyLen & 0x0F) + 3;
			while (copyLen--) {
				byte literal = lzWindow[copyPos];
				copyPos = (copyPos + 1) & 0x0FFF;
				*outBuffer++ = literal;
				lzWindow[lzWindowPos] = literal;
				lzWindowPos = (lzWindowPos + 1) & 0x0FFF;
			}
		}
	} while (outBuffer < outBufferEnd);

	return outS;
}
*/

} // namespace Sherlock
