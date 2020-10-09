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

#ifndef STARK_FORMATS_TM_H
#define STARK_FORMATS_TM_H

#include "engines/stark/formats/biff.h"

#include "common/str.h"
#include "graphics/surface.h"

namespace Stark {

class ArchiveReadStream;

namespace Gfx {
class TextureSet;
class Texture;
}

namespace Formats {

/**
 * A texture set loader able to read '.tm' files
 */
class TextureSetReader {
public:
	/**
	 * Load a texture set from the provided stream.
	 *
	 * The caller is responsible for freeing the texture set.
	 */
	static Gfx::TextureSet *read(ArchiveReadStream *stream);

	/** Read the texture set archive from the provided stream */
	static BiffArchive *readArchive(ArchiveReadStream *stream);

private:
	static BiffObject *biffObjectBuilder(uint32 type);

};

enum TextureSetType {
	kTextureSetGroup   = 0x02faf082,
	kTextureSetTexture = 0x02faf080
};

/**
 * A texture contained in a '.tm' texture set archive
 *
 * Textures have mipmaps.
 */
class Texture : public BiffObject {
public:
	static const uint32 TYPE = kTextureSetTexture;

	Texture();
	~Texture() override;

	Common::String getName() const {
		return _name;
	}

	/**
	 * Return a pointer to a texture ready for rendering
	 *
	 * The caller takes ownership of the texture.
	 * This method can only be called successfully once
	 * per texture. Subsequent calls return a null pointer.
	 */
	Gfx::Texture *acquireTexturePointer();

	/** Return a RGBA copy of the pixel data */
	Graphics::Surface *getSurface() const;

	// BiffObject API
	void readData(ArchiveReadStream *stream, uint32 dataLength) override;

private:
	Common::String _name;
	Gfx::Texture *_texture;
	Graphics::Surface _surface;
	byte _u;
};

} // End of namespace Formats
} // End of namespace Stark

#endif // STARK_FORMATS_TM_H
