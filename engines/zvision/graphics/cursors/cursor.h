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

#ifndef ZVISION_CURSOR_H
#define ZVISION_CURSOR_H

#include "graphics/surface.h"
#include "zvision/zvision.h"

namespace Common {
class String;
}

namespace ZVision {

/**
 * Utility class to parse and hold cursor data
 * Modeled off Graphics::Cursor
 */
class ZorkCursor {
public:
	ZorkCursor();
	ZorkCursor(ZVision *engine, const Common::String &fileName);
	ZorkCursor(const ZorkCursor &other);
	~ZorkCursor();

private:
	uint16 _width;
	uint16 _height;
	uint16 _hotspotX;
	uint16 _hotspotY;
	Graphics::Surface _surface;

public:
	ZorkCursor &operator=(const ZorkCursor &other);

	uint16 getWidth() const {
		return _width;
	}
	uint16 getHeight() const {
		return _height;
	}
	uint16 getHotspotX() const {
		return _hotspotX;
	}
	uint16 getHotspotY() const {
		return _hotspotY;
	}
	byte getKeyColor() const {
		return 0;
	}
	const byte *getSurface() const {
		return (const byte *)_surface.getPixels();
	}
};

} // End of namespace ZVision

#endif
