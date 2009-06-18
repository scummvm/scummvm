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

void Surface::clearDirtyRects() {
	_dirtyRects.clear();
}

void Surface::markDirty() {
	_fullUpdate = true;
}

void Surface::markClean() {
	_fullUpdate = false;
	_dirtyRects.clear();
}

bool Surface::needsFullUpdate() {
	return _fullUpdate;
}

Common::List<Common::Rect> *Surface::getDirtyRects() {
	return &_dirtyRects;
}

uint8 Surface::getTransparentColour() {
	return _transparentColour;
}

void Surface::setTransparentColour(uint8 colour) {
	_transparentColour = colour;
}

} // End of namespace Draci
