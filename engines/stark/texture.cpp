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

#include "engines/stark/texture.h"
#include "engines/stark/stark.h"

#include "common/stream.h"
#include "common/str.h"

#include <SDL_opengl.h> // HACK: I just want to see something - ideally we get _gfx from _scene

namespace Stark {

Texture::Texture() : _palette(NULL) {

}

Texture::~Texture() {
	if (_palette)
		delete[] _palette;

	for (Common::HashMap<Common::String, uint32>::iterator it = _texMap.begin(); it != _texMap.end(); ++it)
		glDeleteTextures(1, &it->_value);
}

bool Texture::createFromStream(Common::ReadStream *stream) {
	uint32 id = stream->readUint32LE();
	uint32 format = stream->readUint32LE();
	uint32 u1 = stream->readUint32LE();
	uint32 u2 = stream->readUint32LE();

	uint32 len = stream->readUint32LE();
	for (uint32 i = 0; i < len; ++i) {
		if (!readChunk(stream, format))
			return false;

	}
	return true;
}

bool Texture::readChunk(Common::ReadStream *stream, uint32 format) {
	uint32 marker = stream->readUint32LE();
	if (marker != 0xf0f0f0f0)
		return false;

	uint32 type = stream->readUint32LE();
	uint32 u3 = stream->readUint32LE();

	uint32 size = stream->readUint32LE();

	if (format == 2)
		uint u4 = stream->readUint32LE();

	if (type == 0x02faf082) {
		// Palette
		if (_palette)
			delete[] _palette;

		int entries = stream->readUint32LE();
		_palette = new uint32[entries * 3];
		byte *ptr = (byte *)_palette;
		for (int i = 0; i < entries; ++i) {
			*ptr++ = (byte)stream->readUint16LE();
			*ptr++ = (byte)stream->readUint16LE();
			*ptr++ = (byte)stream->readUint16LE();
			*ptr++ = 0xff;
		}
	} else if (type == 0x02faf080) {
		// Img
		uint16 nameLength = stream->readUint16LE();
		char *name = new char[nameLength];
		stream->read(name, nameLength);
		Common::String nameStr = Common::String(name, nameLength);
		delete[] name;
		byte u = stream->readByte();

		uint32 texIdx;
		glGenTextures(1, &texIdx);
		_texMap.setVal(nameStr, texIdx);

		glBindTexture(GL_TEXTURE_2D, texIdx);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		uint32 w = stream->readUint32LE();
		uint32 h = stream->readUint32LE();
		int levels = stream->readUint32LE();
		for (int i = 0; i < levels; ++i) {
			uint32 *img = new uint32[w * h];

			for (int j = 0; j < w * h; ++j)
				img[j] = _palette[stream->readByte()];

			if (i == 0)
				glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);		// For whatever reason, this doesn't work...
				//gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img);


			delete[] img;

			w /= 2;
			h /= 2;
		}

	} else {
		byte *data = new byte[size];
		stream->read(data, size);
		delete[] data;
	}

	marker = stream->readUint32LE();
	if (marker != 0x0f0f0f0f)
		return false;

	uint32 len = stream->readUint32LE();
	for (int i = 0; i < len; ++i)
		if (!readChunk(stream, format))
			return false;

	return true;
}

uint32 Texture::getTexture(Common::String name) const {
	Common::HashMap<Common::String, uint32>::const_iterator it = _texMap.find(name);
	if (it != _texMap.end())
		return it->_value;

	return 0;
}

} // End of namespace Stark
