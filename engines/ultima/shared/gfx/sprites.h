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

#ifndef ULTIMA_SPRITES_H
#define ULTIMA_SPRITES_H

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
	uint16 w() const { return _surface.w; }

	/**
	 * Return the height of the sprite
	 */
	uint16 h() const { return _surface.h; }

	/**
	 * Get a reference to the sprite surface
	 */
	byte *getPixels() { return (byte *)_surface.getPixels(); }

	/**
	 * Get a reference to a specified position within the sprite
	 */
	byte *getBasePtr(int16 x, int16 y) { return (byte *)_surface.getBasePtr(x, y); }

	/**
	 * Get a reference to a specified position within the sprite
	 */
	const byte *getBasePtr(int16 x, int16 y) const { return (const byte *)_surface.getBasePtr(x, y); }
};

class Sprites {
protected:
	Common::Array<Sprite> _data;
	Point _spriteSize;							// Size of individual sprites
protected:
	/**
	 * Load a raw data file containing sprite pixels
	 * @param name			Filename
	 * @param bpp			The bits per pixel for tiles in the file
	 * @param w				Width of tiles
	 * @param h				Height of files
	 */
	void load(const Common::String &name, uint bpp, uint16 w = 16, uint16 h = 16);
public:
	/**
	 * Destructor
	 */
	virtual ~Sprites() {}

	/**
	 * Return the default dimensions for tiles
	 */
	Point getSpriteSize() const { return _spriteSize; }

	/**
	 * Return the size of the sprites list
	 */
	size_t size() const { return _data.size(); }

	/**
	 * Returns true if the sprites list is empty
	 */
	bool empty() const { return size() == 0; }

	/**
	 * Return a specific sprite
	 */
	virtual Sprite &operator[](uint idx) { return _data[idx]; }
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
