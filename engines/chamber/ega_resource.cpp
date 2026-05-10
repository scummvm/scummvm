/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/error.h"
#include "graphics/surface.h"

#include "chamber/ega_resource.h"

namespace Chamber {

// Global instances (defined here, declared extern in ega_resource.h)
EgaSpriteResource *ega_sprit_res = nullptr;
EgaSpriteResource *ega_puzzl_res = nullptr;
EgaSpriteResource *ega_perso_res = nullptr;

// ---------------------------------------------------------------------------
// EgaSpriteResource
// ---------------------------------------------------------------------------

EgaSpriteResource::EgaSpriteResource() {
}

EgaSpriteResource::~EgaSpriteResource() {
	for (uint i = 0; i < _sprites.size(); i++) {
		_sprites[i]->free();
		delete _sprites[i];
	}
}

void EgaSpriteResource::appendFromFile(const char *filename) {
	Common::File fd;
	if (!fd.open(filename))
		error("EgaSpriteResource::appendFromFile: cannot open %s", filename);
	appendFromStream(fd);
}

void EgaSpriteResource::appendFromStream(Common::SeekableReadStream &stream) {
	stream.skip(4); // skip 4-byte junk header

	while (!stream.eos()) {
		uint16 size = stream.readUint16LE();
		if (stream.eos())
			break;

		byte w = stream.readByte(); // width in 4-pixel units
		byte h = stream.readByte(); // height in pixels

		if (size < 4)
			break;

		uint16 dataSize = size - 4; // bytes of packed 4-bpp pixel data

		// Allocate CLUT8 surface: actual pixel width = w * 4
		Graphics::Surface *sprite = new Graphics::Surface();
		sprite->create(w * 4, h, Graphics::PixelFormat::createFormatCLUT8());

		byte *dst = (byte *)sprite->getPixels();

		// Unpack packed 4-bpp: 2 pixels per byte
		// Reference: kult/resource.cpp:100-103
		for (uint16 i = 0; i < dataSize; i++) {
			byte packed = stream.readByte();
			*dst++ = (packed >> 4) & 0x0F;  // high nibble = first pixel
			*dst++ =  packed       & 0x0F;  // low  nibble = second pixel
		}

		_sprites.push_back(sprite);
	}
}

} // End of namespace Chamber
