/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef STARK_XMG_H
#define STARK_XMG_H

#include "common/sys.h"
#include "common/stream.h"

#include "graphics/surface.h"

namespace Stark {

inline static void YUV2RGB(byte y, byte u, byte v, byte &r, byte &g, byte &b) {
	r = CLIP<int>(y + ((1357 * (v - 128)) >> 10), 0, 255);
	g = CLIP<int>(y - (( 691 * (v - 128)) >> 10) - ((333 * (u - 128)) >> 10), 0, 255);
	b = CLIP<int>(y + ((1715 * (u - 128)) >> 10), 0, 255);
}

struct XMGHeader {
	uint32 version;
	uint32 transColor;
	uint8  unknown;
	uint32 width;
	uint32 height;
	uint32 scanLen;
	uint32 unknown2;
	uint32 unknown3;
};

class XMGDecoder {
public:
	XMGDecoder() {}
	~XMGDecoder() {}
	
	Graphics::Surface *decodeImage(Common::SeekableReadStream *stream);

private:
	void processYCrCb(uint16 count);
	void processTrans(uint16 count);
	void processRGB(uint16 count);

	byte *_pixels;
	uint32 _width;
	Common::SeekableReadStream *_stream;

	XMGHeader header;

	uint32 _currX, _currY;
};

} // end of namespace Stark

#endif

