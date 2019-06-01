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

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/formats/biff.h"

#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/services.h"

namespace Stark {
namespace Formats {

class TextureGroup : public BiffObject {
public:
	static const uint32 TYPE = kTextureSetGroup;

	TextureGroup() :
			BiffObject(),
			_palette(nullptr) {
		_type = TYPE;
	}

	~TextureGroup() override {
		delete[] _palette;
	}

	const byte *getPalette() {
		return _palette;
	}

	// BiffObject API
	void readData(ArchiveReadStream *stream, uint32 dataLength) override {
		int entries = stream->readUint32LE();
		_palette = new byte[entries * 3];

		byte *ptr = _palette;
		for (int i = 0; i < entries; ++i) {
			*ptr++ = (byte) stream->readUint16LE();
			*ptr++ = (byte) stream->readUint16LE();
			*ptr++ = (byte) stream->readUint16LE();
		}
	}

private:
	byte *_palette;
};

Texture::Texture() :
    BiffObject(),
    _texture(nullptr),
    _u(0) {
	_type = TYPE;
}

Texture::~Texture() {
	_surface.free();
	delete _texture;
}

void Texture::readData(ArchiveReadStream *stream, uint32 dataLength) {
	TextureGroup *textureGroup = static_cast<TextureGroup *>(_parent);

	_name = stream->readString16();
	_u = stream->readByte();

	uint32 w = stream->readUint32LE();
	uint32 h = stream->readUint32LE();
	uint32 levels = stream->readUint32LE();

	_texture = StarkGfx->createTexture();
	_texture->setLevelCount(levels);

	for (uint32 i = 0; i < levels; ++i) {
		// Read the pixel data to a surface
		Graphics::Surface level;
		Graphics::Surface *surface = i == 0 ? &_surface : &level;

		surface->create(w, h, Graphics::PixelFormat::createFormatCLUT8());
		stream->read(surface->getPixels(), surface->w * surface->h);

		// Add the mipmap level to the texture
		_texture->addLevel(i, surface, textureGroup->getPalette());

		level.free();

		w /= 2;
		h /= 2;
	}
}

Gfx::Texture *Texture::acquireTexturePointer() {
	Gfx::Texture *texture = _texture;
	_texture = nullptr;

	return texture;
}

Graphics::Surface *Texture::getSurface() const {
	TextureGroup *textureGroup = static_cast<TextureGroup *>(_parent);

	return _surface.convertTo(Gfx::Driver::getRGBAPixelFormat(), textureGroup->getPalette());
}

Gfx::TextureSet *TextureSetReader::read(ArchiveReadStream *stream) {
	BiffArchive archive = BiffArchive(stream, &biffObjectBuilder);

	Common::Array<Texture *> textures = archive.listObjectsRecursive<Texture>();

	Gfx::TextureSet *textureSet = new Gfx::TextureSet();
	for (uint i = 0; i < textures.size(); i++) {
		textureSet->addTexture(textures[i]->getName(), textures[i]->acquireTexturePointer());
	}

	return textureSet;
}

BiffArchive *TextureSetReader::readArchive(ArchiveReadStream *stream) {
	return new BiffArchive(stream, &biffObjectBuilder);
}

BiffObject *TextureSetReader::biffObjectBuilder(uint32 type) {
	switch (type) {
		case kTextureSetGroup:
			return new TextureGroup();
		case kTextureSetTexture:
			return new Texture();
		default:
			return nullptr;
	}
}

} // End of namespace Formats
} // End of namespace Stark
