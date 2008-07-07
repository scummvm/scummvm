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

#include "common/image-map.h"

namespace Common {

ImageMap::~ImageMap() {
	HashMap<String, Shape*>::iterator it;
	for (it = _areas.begin(); it != _areas.end(); it++) {
		delete it->_value;
	}
}

Rect *ImageMap::createRectArea(const String& id) {
	if (_areas.contains(id)) {
		warning("Image map already contains an area with target of '%s'");
		return 0;
	}
	Rect *r = new Rect();
	_areas[id] = r;
	return r;
}

Polygon *ImageMap::createPolygonArea(const String& id) {
	if (_areas.contains(id)) {
		warning("Image map already contains an area with target of '%s'");
		return 0;
	}
	Polygon *p = new Polygon();
	_areas[id] = p;
	return p;
}

/*
void ImageMap::addMapArea(Shape *shape, const String& target) {
	if (_areas.contains(target)) {
		warning("Image map already contains an area with target of '%s'");
		return;
	}
	_areas[target] = shape;
}
void ImageMap::addRectMapArea(const Rect& rect, const String& target) {
	areas.push_back(MapArea(rect, target));
}

void ImageMap::addPolygonMapArea(const Polygon& poly, const String& target) {
	areas.push_back(MapArea(poly, target));
}
*/
String ImageMap::findMapArea(int16 x, int16 y) {
	HashMap<String, Shape*>::iterator it;
	for (it = _areas.begin(); it != _areas.end(); it++) {
		if (it->_value->contains(x, y))
			return it->_key;
	}
	return "";
}


} // End of namespace Common
