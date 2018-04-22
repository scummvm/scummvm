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

#ifndef ULTIMA_SPRITE_H
#define ULTIMA_SPRITE_H

#include "common/array.h"
#include "graphics/managed_surface.h"
#include "ultima/shared/core/rect.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

class Sprite {
private:
	Graphics::ManagedSurface _surface;
	Graphics::ManagedSurface _transSurface;
public:
	/**
	 * Constructor
	 */
	Sprite();

	/**
	 * Constructor, copying from another sprite
	 */
	Sprite(const Sprite &src);

	/**
	 * Constructor, loading a sprite from passed data
	 * @param src			Source data
	 * @param bpp			The bits per pixel for tiles in the file
	 * @param w				Width of tiles
	 * @param h				Height of files
	 */
	Sprite(const byte *src, uint bpp, uint16 w = 16, uint16 h = 16);

	/**
	 * Draw a tile onto a passed surface
	 */
	void draw(Graphics::ManagedSurface &dest, const Common::Point &pt);

	/**
	 * Return the width of the sprite
	 */
	uint16 w() const {
		return _surface.w;
	}

	/**
	 * Return the height of the sprite
	 */
	uint16 h() const {
		return _surface.h;
	}
};

class Sprites : public Common::Array<Sprite> {
private:
	Point _spriteSize;							// Size of individual sprites
public:
	/**
	 * Load a raw data file containing sprite pixels
	 * @param name			Filename
	 * @param bpp			The bits per pixel for tiles in the file
	 * @param w				Width of tiles
	 * @param h				Height of files
	 */
	void load(const Common::String &name, uint bpp, uint16 w = 16, uint16 h = 16);

	/**
	 * Return the default dimensions for tiles
	 */
	Point getSpriteSize() const { return _spriteSize; }
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
