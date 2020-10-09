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

#ifndef GLK_PICTURE_H
#define GLK_PICTURE_H

#include "common/array.h"
#include "graphics/managed_surface.h"

namespace Glk {

/**
 * Picture/image class
 */
struct Picture : Graphics::ManagedSurface {
private:
	int _transColor;
public:
	int _refCount;
	Common::String _name;
	bool _scaled;

	/**
	 * Constructor
	 */
	Picture() : Graphics::ManagedSurface(), _refCount(0), _scaled(false), _transColor(0x7777) {}

	/**
	 * Constructor
	 */
	Picture(int width, int height, const Graphics::PixelFormat &fmt);
	/**
	 * Increment reference counter
	 */
	void increment();

	/**
	 * Decrement reference counter
	 */
	void decrement();

	/**
	 * Draw the picture
	 */
	void drawPicture(const Common::Point &destPos, const Common::Rect &box);

	/**
	 * Get the transparency color
	 */
	uint getTransparentColor() const { return _transColor; }

	/**
	 * Set the transparency color
	 */
	void setTransparentColor(uint color) { _transColor = color; }
};

/**
 * Picture entry in the in-memory store
 */
struct PictureEntry {
	Picture *_picture;
	Picture *_scaled;
	PictureEntry() : _picture(nullptr), _scaled(nullptr) {}
};

/**
 * Pictures manager
 */
class Pictures {
private:
	int _refCount;
	Common::Array<PictureEntry> _store;
	Common::Array<Common::String> _adaptivePics;
	Common::Array<byte> _savedPalette;
private:
	/**
	 * Stores an original picture in the store
	 */
	void storeOriginal(Picture *pic);

	/**
	 * Stores a scaled picture in the store
	 */
	void storeScaled(Picture *pic);
public:
	/**
	 * Constructor
	 */
	Pictures();

	/**
	 * Destructor
	 */
	~Pictures() { clear(); }

	/**
	 * Clear the picture list
	 */
	void clear();

	/**
	 * Increments the count of the number of pictures in use
	 */
	void increment();

	/**
	 * Decrements the count of the number of pictures in use
	 */
	void decrement();

	/**
	 * Searches for an existing picture entry
	 */
	PictureEntry *search(const Common::String &name);

	/**
	 * Stores a picture in the store
	 */
	void store(Picture *pic);

	/**
	 * Retrieves a picture from the store
	 */
	Picture *retrieve(const Common::String &name, bool scaled);

	/**
	 * Load a given picture
	 */
	Picture *load(const Common::String &name);

	/**
	 * Rescale the passed picture to a new picture of a given size
	 */
	Picture *scale(Picture *src, size_t sx, size_t sy);
};

} // End of namespace Glk

#endif
