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

#include "agds/screen.h"
#include "agds/object.h"
#include "agds/region.h"

namespace AGDS {

Screen::Screen(Object *object) {
	add(object);
}

void Screen::add(Object *object) {
	for(ChildrenType::iterator i = _children.begin(); i != _children.end(); ++i) {
		if (*i == object)
			return;
	}
	_children.push_back(object);
}

void Screen::remove(const Common::String &name) {
	for(ChildrenType::iterator i = _children.begin(); i != _children.end(); ) {
		if ((*i)->getName() == name)
			i = _children.erase(i);
		else
			++i;
	}
}


void Screen::paint(Graphics::Surface &backbuffer) {
	for(ChildrenType::iterator i = _children.begin(); i != _children.end(); ++i) {
		Object *object = *i;
		object->paint(backbuffer);
	}
}

Object *Screen::find(Common::Point pos) const {
	for(ChildrenType::const_iterator i = _children.begin(); i != _children.end(); ++i) {
		Object *object = *i;
		Region *region = object->getRegion();
		if (region && region->pointIn(pos))
			return object;
	}
	return NULL;
}

const MouseRegion * MouseMap::find(Common::Point pos) const {
	for(MouseRegionsType::const_iterator i = _mouseRegions.begin(); i != _mouseRegions.end(); ++i) {
		const MouseRegion &mouse = *i;
		if (mouse.region->pointIn(pos))
			return &mouse;
	}
	return NULL;

}



}
