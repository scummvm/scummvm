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

#ifndef CINEPAK_H
#define CINEPAK_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"

#include "graphics/video/codecs/codec.h"

namespace Mohawk {

struct CinepakCodebook {
	byte y[4];
	byte u, v;
};

struct CinepakStrip {
	uint16 id;
	uint16 length;
	Common::Rect rect;
	CinepakCodebook v1_codebook[256], v4_codebook[256];
};

struct CinepakFrame {
	byte flags;
	uint32 length;
	uint16 width;
	uint16 height;
	uint16 stripCount;
	CinepakStrip *strips;
	
	Graphics::Surface *surface;
};

class CinepakDecoder : public Graphics::Codec {
public:
	CinepakDecoder();
	~CinepakDecoder();

	Graphics::Surface *decodeImage(Common::SeekableReadStream *stream);

private:
	CinepakFrame _curFrame;
	int32 _y;
	Graphics::PixelFormat _pixelFormat;
	
	void loadCodebook(Common::SeekableReadStream *stream, uint16 strip, byte codebookType, byte chunkID, uint32 chunkSize);
	void decodeVectors(Common::SeekableReadStream *stream, uint16 strip, byte chunkID, uint32 chunkSize);
};

}

#endif
