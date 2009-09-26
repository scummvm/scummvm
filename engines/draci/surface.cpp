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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "draci/screen.h"
#include "draci/surface.h"

namespace Draci {

Surface::Surface(int width, int height) {
	this->create(width, height, 1);
	this->markClean();
	_transparentColour = kDefaultTransparent;
}

Surface::~Surface() {
	this->free();
}

/**
 * @brief Marks a dirty rectangle on the surface
 * @param r The rectangle to be marked dirty
 */
void Surface::markDirtyRect(Common::Rect r) {
	Common::List<Common::Rect>::iterator it;

	r.clip(w, h);

	if (r.isEmpty())
		return;

	it = _dirtyRects.begin(); 
	while (it != _dirtyRects.end()) {

		if (it->contains(r))
			return;

		if (r.contains(*it))
			it = _dirtyRects.erase(it);
		else
			++it;
	}

	_dirtyRects.push_back(r);
}

/**
 * @brief Clears all dirty rectangles
 *
 */
void Surface::clearDirtyRects() {
	_dirtyRects.clear();
}

/**
 * @brief Marks the whole surface dirty
 */
void Surface::markDirty() {
	_fullUpdate = true;
}

/**
 * @brief Marks the whole surface clean
 */
void Surface::markClean() {
	_fullUpdate = false;
	_dirtyRects.clear();
}

/**
 * @brief Checks whether the surface needs a full update
 */
bool Surface::needsFullUpdate() const {
	return _fullUpdate;
}

/**
 * @brief Fetches the surface's dirty rectangles
 * @return A pointer a list of dirty rectangles
 */
const Common::List<Common::Rect> *Surface::getDirtyRects() const {
	return &_dirtyRects;
}

/**
 * @brief Returns the current transparent colour of the surface
 */
uint Surface::getTransparentColour() const {
	return _transparentColour;
}

/**
 * @brief Sets the surface's transparent colour
 */
void Surface::setTransparentColour(uint colour) {
	_transparentColour = colour;
}

/**
 * @brief Fills the surface with the specified colour
 */
void Surface::fill(uint colour) {
	byte *ptr = (byte *)getBasePtr(0, 0);

	memset(ptr, colour, w * h);
}

/**
 * @brief Calculates horizontal center of an object
 *
 * @param x 	The x coordinate of the center
 * @param width The width of the object to be centered (in pixels)
 *
 * @return The centered x coordinate
 */
uint Surface::centerOnX(uint x, uint width) const {

	int newX = x - width / 2;

	if (newX < 0)
		newX = 0;

	if (newX + width >= (uint)w - 1)
		newX = (w - 1) - width;

	if (newX < 0)
		newX = 0;

	return newX;
}

/**
 * @brief Calculates vertical center of an object
 *
 * @param y 	The y coordinate of the center
 * @param height The height of the object to be centered (in pixels)
 *
 * @return The centered y coordinate
 */
uint Surface::centerOnY(uint y, uint height) const {
	
	int newY = y - height / 2;
	
	if (newY < 0)
		newY = 0;

	if (newY + height >= (uint)h - 1)
		newY = (h - 1) - height;

	if (newY < 0)
		newY = 0;

	return newY;
}

/**
 * @brief Returns a Common::Rect corresponding to the surface.
 */

Common::Rect Surface::getRect() const {
	return Common::Rect(w, h);
}

} // End of namespace Draci
