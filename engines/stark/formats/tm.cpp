/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/formats/tm.h"

#include "common/stream.h"
#include "common/str.h"

#include "graphics/surface.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

namespace Stark {
namespace Formats {

TextureSetReader::TextureSetReader(GfxDriver *driver) :
		_palette(nullptr),
		_driver(driver) {
}

TextureSetReader::~TextureSetReader() {
	delete[] _palette;
}

Gfx::TextureSet *TextureSetReader::read(Common::ReadStream *stream) {
	Gfx::TextureSet *textureSet = new Gfx::TextureSet();

	uint32 id = stream->readUint32LE();
	uint32 format = stream->readUint32LE();
	uint32 u1 = stream->readUint32LE();
	uint32 u2 = stream->readUint32LE();

	uint32 len = stream->readUint32LE();
	for (uint32 i = 0; i < len; ++i) {
		readChunk(stream, format, textureSet);
	}

	return textureSet;
}

void TextureSetReader::readChunk(Common::ReadStream *stream, uint32 format, Gfx::TextureSet *textureSet) {
	uint32 marker = stream->readUint32LE();
	if (marker != 0xf0f0f0f0) {
		error("Wrong magic while reading texture");
	}

	uint32 type = stream->readUint32LE();
	uint32 u3 = stream->readUint32LE();

	uint32 size = stream->readUint32LE();

	if (format == 2)
		uint u4 = stream->readUint32LE();

	if (type == 0x02faf082) {
		// Palette
		delete[] _palette;

		int entries = stream->readUint32LE();
		_palette = new byte[entries * 3];
		byte *ptr = _palette;
		for (int i = 0; i < entries; ++i) {
			*ptr++ = (byte)stream->readUint16LE();
			*ptr++ = (byte)stream->readUint16LE();
			*ptr++ = (byte)stream->readUint16LE();
		}
	} else if (type == 0x02faf080) {
		// Img
		uint16 nameLength = stream->readUint16LE();
		char *name = new char[nameLength];
		stream->read(name, nameLength);
		Common::String nameStr = Common::String(name, nameLength);
		delete[] name;
		byte u = stream->readByte();

		Gfx::MipMapTexture *texture = _driver->createMipMapTexture();

		uint32 w = stream->readUint32LE();
		uint32 h = stream->readUint32LE();
		uint32 levels = stream->readUint32LE();

		texture->setLevelCount(levels);

		for (uint32 i = 0; i < levels; ++i) {
			// Read the pixel data to a surface
			Graphics::Surface level;
			level.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
			stream->read(level.getPixels(), level.w * level.h);

			// Add the mipmap level to the texture
			texture->addLevel(i, &level, _palette);

			level.free();

			w /= 2;
			h /= 2;
		}

		textureSet->addTexture(nameStr, texture);
	} else {
		byte *data = new byte[size];
		stream->read(data, size);
		delete[] data;
	}

	marker = stream->readUint32LE();
	if (marker != 0x0f0f0f0f) {
		error("Wrong magic while reading texture");
	}

	uint32 len = stream->readUint32LE();
	for (uint32 i = 0; i < len; ++i) {
		readChunk(stream, format, textureSet);
	}
}

} // End of namespace Formats
} // End of namespace Stark
