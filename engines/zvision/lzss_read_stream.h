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

#ifndef ZVISION_LZSS_STREAM_H
#define ZVISION_LZSS_STREAM_H

#include "common/types.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/array.h"

namespace ZVision {

class LzssReadStream : public Common::ReadStream {
public:
	/**
	 * A class that decompresses LZSS data and implements ReadStream for easy access 
	 * to the decompiled data. It can either decompress all the data in the beginning 
	 * or decompress as needed by read().
	 *
	 * @param source              The source data
	 * @param stream			  Decompress the data as needed (true) or all at once (false)
	 * @param decompressedSize    The size of the decompressed data. If npos, the class will choose a size and grow as needed
	 */
	LzssReadStream(Common::SeekableReadStream *source, bool stream = true, uint32 decompressedSize = npos);

public:
	static const uint32 npos = 0xFFFFFFFFu;
	static const uint16 blockSize = 0x1000u;

private:
	Common::SeekableReadStream *_source;
	Common::Array<char> _destination;
	char _window[blockSize];
	uint16 _windowCursor;
	uint32 _readCursor;

public:
	bool eos() const;
	uint32 read(void *dataPtr, uint32 dataSize);
	uint32 currentSize() const;

private:
	/**
	 * Decompress the next <numberOfBytes> from the source stream. Or until EOS
	 *
	 * @param numberOfBytes    How many bytes to decompress. This is a count of source bytes, not destination bytes
	 */
	void decompressBytes(uint32 numberOfBytes);
	/** Decompress all of the source stream. */
	void decompressAll();
};

}

#endif
