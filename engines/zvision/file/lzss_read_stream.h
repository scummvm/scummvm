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

#ifndef ZVISION_LZSS_STREAM_H
#define ZVISION_LZSS_STREAM_H

#include "common/stream.h"
#include "common/array.h"

namespace Common {
class SeekableReadStream;
}

namespace ZVision {

class LzssReadStream : public Common::ReadStream {
public:
	/**
	 * A class that decompresses LZSS data and implements ReadStream for easy access
	 * to the decompiled data.
	 *
	 * @param source              The source data
	 */
	LzssReadStream(Common::SeekableReadStream *source);

private:
	enum {
		BLOCK_SIZE = 0x1000
	};

private:
	Common::SeekableReadStream *_source;
	byte _window[BLOCK_SIZE];
	uint _windowCursor;
	bool _eosFlag;

public:
	bool eos() const override;
	uint32 read(void *dataPtr, uint32 dataSize) override;

private:
	/**
	 * Decompress the next <numberOfBytes> from the source stream. Or until EOS
	 *
	 * @param numberOfBytes    How many bytes to decompress. This is a count of source bytes, not destination bytes
	 */
	uint32 decompressBytes(byte *destination, uint32 numberOfBytes);
};

}

#endif
