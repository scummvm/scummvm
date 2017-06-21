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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common/scummsys.h"
#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Supernova {

class MSNImageDecoder : public Image::ImageDecoder {
public:
	MSNImageDecoder();
	virtual ~MSNImageDecoder();

	virtual void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	virtual const byte *getPalette() const { return _palette; }

	bool loadSection(int _section);

	static const int kMaxSections = 50;
	static const int kMaxClickFields = 80;

	Graphics::Surface *_surface;
	byte *_palette;
	byte *_encodedImage;

	struct Section {
		int16  x1;
		int16  x2;
		byte   y1;
		byte   y2;
		byte   next;
		uint16 addressLow;
		byte   addressHigh;
	} _section[kMaxSections];

	struct ClickField {
		int16  x1;
		int16  x2;
		byte   y1;
		byte   y2;
		byte   next;
	} _clickField[kMaxClickFields];
};

}
#endif
