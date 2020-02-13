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

#ifndef SUPERNOVA_GRAPHICS_H
#define SUPERNOVA_GRAPHICS_H

#include "common/scummsys.h"
#include "image/image_decoder.h"
#include "supernova/supernova.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Supernova {
class SupernovaEngine;

class MSNImage : public Image::ImageDecoder {
public:
	MSNImage(SupernovaEngine *vm);
	~MSNImage() override;

	void destroy() override;
	bool loadStream(Common::SeekableReadStream &stream) override;
	const Graphics::Surface *getSurface() const override { return _sectionSurfaces[0]; }
	const byte *getPalette() const override { return _palette; }

	bool init(int filenumber);

	static const int kMaxSections = 50;
	static const int kMaxClickFields = 80;
	static const uint32 kInvalidAddress = 0x00FFFFFF;

	int _filenumber;
	int _pitch;
	int _numSections;
	int _numClickFields;
	Common::Array<Graphics::Surface *> _sectionSurfaces;
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

private:
	SupernovaEngine *_vm;
	bool loadFromEngineDataFile();
	bool loadPbmFromEngineDataFile();
	bool loadSections();
};

}
#endif
