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
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_IFF_H
#define PARALLACTION_IFF_H

#include "common/stream.h"
#include "common/iff_container.h"		// for IFF chunk names
#include "graphics/iff.h"				// for BMHD

// this IFF parser code is courtesy of the Kyra engine team ;)

namespace Parallaction {

class IFFParser {
public:
	IFFParser() : _stream(0), _startOffset(0), _endOffset(0) {}
	IFFParser(Common::SeekableReadStream *stream) : _stream(0), _startOffset(0), _endOffset(0) {
		setInputStream(stream);
	}
	~IFFParser() { destroy(); }

	void setInputStream(Common::SeekableReadStream *stream);

	operator bool() const { return (_startOffset != _endOffset) && _stream; }

	uint32 getFORMBlockSize();
	uint32 getIFFBlockSize(Common::IFF_ID chunk);
	bool loadIFFBlock(Common::IFF_ID chunk, void *loadTo, uint32 ptrSize);
	Common::SeekableReadStream *getIFFBlockStream(Common::IFF_ID chunkName);
private:
	void destroy();
	uint32 moveToIFFBlock(Common::IFF_ID chunkName);

	Common::SeekableReadStream *_stream;
	uint32 _startOffset;
	uint32 _endOffset;
};




class ILBMDecoder {
	Common::SeekableReadStream *_in;
	bool _disposeStream;

	void planarToChunky(byte *out, uint32 width, byte *in, uint32 planeWidth, uint32 nPlanes, bool packPlanes);

protected:
	IFFParser 	_parser;
	Graphics::BMHD 	_header;
	bool	_hasHeader;
	uint32	_bodySize;
	uint32	_paletteSize;


public:
	ILBMDecoder(Common::SeekableReadStream *input, bool disposeStream = false);

	virtual ~ILBMDecoder();

	uint32 getWidth();
	uint32 getHeight();
	uint32 getNumColors();
	byte *getPalette();

	byte *getBitmap(uint32 numPlanes, bool packPlanes);
	byte *getBitmap() {
		assert(_hasHeader);
		return getBitmap(_header.depth, false);
	}
};


}

#endif

