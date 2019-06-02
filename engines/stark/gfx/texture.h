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

#ifndef STARK_GFX_TEXTURE_H
#define STARK_GFX_TEXTURE_H

#include "common/hash-str.h"

namespace Graphics {
	struct Surface;
}

namespace Stark {
namespace Gfx {

/**
 * An abstract texture
 */
class Texture {
public:
	Texture();
	virtual ~Texture();

	enum SamplingFilter {
		kNearest,
		kLinear
	};

	/** Make the texture active */
	virtual void bind() const = 0;

	/** Define or update the texture pixel data */
	virtual void update(const Graphics::Surface *surface, const byte *palette = nullptr) = 0;

	/** Set the filter used when sampling the texture */
	virtual void setSamplingFilter(SamplingFilter filter) = 0;

	/**
	 * Define the total number of levels of details
	 *
	 * Must be called before adding levels
	 */
	virtual void setLevelCount(uint32 count) = 0;

	/**
	 * Add a detail level to the texture
	 */
	virtual void addLevel(uint32 level, const Graphics::Surface *surface, const byte *palette = nullptr) = 0;

	/** Get the texture width */
	uint32 width() const { return _width; }

	/** Get the texture height */
	uint32 height() const { return _height; }

protected:
	uint32 _width;
	uint32 _height;
};

/**
 * A collection of textures referenced by their names
 */
class TextureSet {
public:
	TextureSet();
	~TextureSet();

	/**
	 * Add a texture to the set
	 */
	void addTexture(const Common::String &name, Texture *texture);

	/**
	 * Retrieve a texture from the set
	 */
	const Texture *getTexture(const Common::String &name) const;

private:
	typedef Common::HashMap<Common::String, Texture *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TextureMap;

	TextureMap _texMap;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_TEXTURE_H
