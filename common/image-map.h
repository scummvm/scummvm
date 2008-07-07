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
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_IMAGEMAP_H
#define COMMON_IMAGEMAP_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/rect.h"
#include "common/polygon.h"

namespace Common {

class MapArea {
public:
	MapArea() : _shape(0), _target() {}
	MapArea(const Rect& r, const String& t) : _target(t) {
		_shape = new Rect(r);
	}
	MapArea(const Polygon& p, const String& t) : _target(t) {
		_shape = new Polygon(p);
	}
	virtual ~MapArea() {
		delete _shape;
	}

	virtual bool contains(int x, int y) {
		return _shape->contains(x, y);
	}

	String getTarget() { return _target; }

protected:
	/* shape defining the MapArea's boundary */
	Shape *_shape;
	/* string describing the target of MapArea */
	String _target;
};

class ImageMap {

public:

	~ImageMap();
	
	Rect *createRectArea(const String& id);
	Polygon *createPolygonArea(const String& id);

	//void addMapArea(Shape *shape, const String& target);
	/*void addRectMapArea(const Rect& rect, const String& target);
	void addPolygonMapArea(const Polygon& poly, const String& target);
*/
	String findMapArea(int16 x, int16 y);

protected:
	HashMap<String, Shape*> _areas;
};


} // End of namespace Common

#endif
