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

#include "image/cel_3do.h"

#include "common/file.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "image/bmp.h"

namespace Image {

enum CCBFlags {
	kCCBPacked = 1 << 9,
	kCCBNoPre0 = 1 << 22
};

Cel3DODecoder::Cel3DODecoder() {
	_surface = 0;
	_palette = 0;
	_paletteColorCount = 0;
}

Cel3DODecoder::~Cel3DODecoder() {
	destroy();
}

void Cel3DODecoder::destroy() {
	_surface = 0;

	delete[] _palette;
	_palette = 0;

	_paletteColorCount = 0;
}

bool Cel3DODecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	// This is not a full implementaion of CEL support,
	// just what is currently needed for following games:
	// * Plumbers don't wear ties
	// TODO: support paletted

	if (stream.readUint32BE() != MKTAG('C', 'C', 'B', ' '))
		return false;

	if (stream.readUint32BE() != 0x50) // block size
		return false;

	if (stream.readUint32BE() != 0) // CCB version
		return false;

	uint32 flags = stream.readUint32BE();

	stream.skip(0x30);
	uint32 pre0 = stream.readUint32BE();
	/* pre1 = */ stream.readUint32BE();
	uint32 width = stream.readUint32BE();
	uint32 height = stream.readUint32BE();

	while (!stream.eos()) {
		if (stream.readUint32BE() == MKTAG('P', 'D', 'A', 'T'))
			break;
		stream.skip(stream.readUint32BE() - 8);
	}

	if (stream.eos())
		return false;

	if (width == 0 || height == 0)
		return false;

	/* pdat_size = */ stream.readUint32BE();

	Graphics::PixelFormat format(2, 5, 5, 5, 1, 10, 5, 0, 15);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, format);

	uint16 *dst = (uint16 *)surface->getBasePtr(0, 0);

	if(!(flags & kCCBNoPre0)) {
		pre0 = stream.readUint32BE();
		if(!(flags & kCCBPacked)) {
			/* pre1 = */ stream.readUint32BE();
		}
	}

	// Only RGB555 is supported
	if ((pre0 & 0x17) != 0x16)
		return false;

	if(!(flags & kCCBPacked)) {
		// RAW
		// TODO: this can be optimized, especially on BE systems, but do we care?
		for (uint xy = 0; xy < width * height; xy++)
			*dst++ = stream.readUint16BE();
	} else {
		// RLE
		for (uint y = 0; y < height; y++) {
			int linecomprem = (stream.readUint16BE() + 2) * 4 - 2;
			int linerem = width;
			bool stopLine = false;
			while (linerem > 0 && linecomprem > 0 && !stopLine) {
				byte lead = stream.readByte();
				linecomprem--;
				switch (lead >> 6) {
				case 0: // end of the line
					stopLine = true;
					break;
				case 1: // copy
					for (uint i = 0; i <= (lead & 0x3f) && linerem > 0 && linecomprem > 0;
					     i++, linerem--, linecomprem -= 2)
						*dst++ = stream.readUint16BE();
					break;
				case 2: // black
					for (uint i = 0; i <= (lead & 0x3f) && linerem > 0; i++, linerem--)
						*dst++ = 0;
					break;
				case 3: { // RLE multiply
					uint16 rleval = stream.readUint16BE();
					linecomprem -= 2;
					for (uint i = 0; i <= (lead & 0x3f) && linerem > 0; i++, linerem--)
						*dst++ = rleval;
					break;
				}
				}
			}
			if (linecomprem > 0)
				stream.skip(linecomprem);
			if (linerem > 0) {
				memset(dst, 0, 2 * linerem);
				dst += linerem;
			}
		}
	}

	_surface = surface;

	return true;
}

} // End of namespace Image
