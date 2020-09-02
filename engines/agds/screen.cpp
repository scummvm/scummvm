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
#include "agds/animation.h"
#include "agds/object.h"
#include "agds/region.h"

namespace AGDS {

int Screen::ObjectZCompare(const ObjectPtr &a, const ObjectPtr &b) {
	return b->z() - a->z();
}

int Screen::AnimationZCompare(const Animation *a, const Animation *b) {
	return b->z() - a->z();
}

Screen::Screen(ObjectPtr object, const MouseMap &mouseMap) : _object(object), _name(object->getName()), _mouseMap(mouseMap),
                                                             _children(&ObjectZCompare), _animations(&AnimationZCompare) {
	add(object);
}

Screen::~Screen() {
	_children.clear();
}

void Screen::add(ObjectPtr object) {
	if (object == NULL) {
		warning("refusing to add null to scene");
		return;
	}
	for (ChildrenType::iterator i = _children.begin(); i != _children.end(); ++i) {
		if (*i == object) {
			warning("double adding object %s", (*i)->getName().c_str());
			return;
		}
	}
	_children.insert(object);
}

ObjectPtr Screen::find(const Common::String &name) {
	for (ChildrenType::iterator i = _children.begin(); i != _children.end(); ++i) {
		if ((*i)->getName() == name)
			return *i;
	}
	return ObjectPtr();
}

bool Screen::remove(const ObjectPtr &object) {
	bool found = false;
	for (ChildrenType::iterator i = _children.begin(); i != _children.end();) {
		if (*i == object) {
			i = _children.erase(i);
			found = true;
		} else
			++i;
	}
	return found;
}

bool Screen::remove(const Common::String &name) {
	bool found = false;
	for (ChildrenType::iterator i = _children.begin(); i != _children.end();) {
		if ((*i)->getName() == name) {
			i = _children.erase(i);
			found = true;
		} else
			++i;
	}
	return found;
}

void Screen::paint(AGDSEngine &engine, Graphics::Surface &backbuffer) {
#if 0
	ChildrenType::iterator child = _children.begin();
	AnimationsType::iterator animation = _animations.begin();
	int idx = 0;
	while(child != _children.end() || animation != _animations.end()) {
		bool child_valid = child != _children.end();
		bool animation_valid = animation != _animations.end();
		if (child_valid && animation_valid) {
			if ((*child)->z() < (*animation)->z()) {
				debug("object %d, z: %d", idx++, (*child)->z());
				(*child)->paint(engine, backbuffer);
				++child;
			} else {
				debug("animatin %d, z: %d", idx++, (*animation)->z());
				(*animation)->paint(engine, backbuffer, Common::Point());
				++animation;
			}
		} else if (child_valid) {
			debug("object %d, z: %d", idx++, (*child)->z());
			(*child)->paint(engine, backbuffer);
			++child;
		} else {
			debug("animatin %d, z: %d", idx++, (*animation)->z());
			(*animation)->paint(engine, backbuffer, Common::Point());
			++animation;
		}
	}
#else
	ChildrenType::iterator i;
	for (i = _children.begin(); i != _children.end(); ++i) {
		ObjectPtr object = *i;
		if (object->z() < 1000)
			break;
		object->paint(engine, backbuffer);
	}
	for (AnimationsType::iterator j = _animations.begin(); j != _animations.end(); ++j) {
		Animation *animation = *j;
		animation->paint(engine, backbuffer, Common::Point());
	}
	for (; i != _children.end(); ++i) {
		ObjectPtr object = *i;
		object->paint(engine, backbuffer);
	}
#endif
}

ObjectPtr Screen::find(Common::Point pos) const {
	for (ChildrenType::const_iterator i = _children.begin(); i != _children.end(); ++i) {
		ObjectPtr object = *i;
		RegionPtr region = object->region();
		if (region && region->pointIn(pos))
			return object;
	}
	return ObjectPtr();
}

Screen::KeyHandler Screen::findKeyHandler(const Common::String &keyName) {
	KeyHandler keyHandler;
	for (ChildrenType::const_iterator i = _children.begin(); i != _children.end(); ++i) {
		ObjectPtr object = *i;
		uint ip = object->getKeyHandler(keyName);
		if (ip) {
			keyHandler.ip = ip;
			keyHandler.object = object;
			break;
		}
	}
	return keyHandler;
}

MouseRegion *MouseMap::find(Common::Point pos) {
	if (_disabled)
		return NULL;
	for (MouseRegionsType::iterator i = _mouseRegions.begin(); i != _mouseRegions.end(); ++i) {
		MouseRegion &mouse = *i;
		if (mouse.enabled && mouse.region->pointIn(pos))
			return &mouse;
	}
	return NULL;
}

MouseRegion *MouseMap::find(int id) {
	for (MouseRegionsType::iterator i = _mouseRegions.begin(); i != _mouseRegions.end(); ++i) {
		MouseRegion &mouse = *i;
		if (mouse.id == id)
			return &mouse;
	}
	return NULL;
}

void MouseMap::remove(int id) {
	for (MouseRegionsType::iterator i = _mouseRegions.begin(); i != _mouseRegions.end();) {
		MouseRegion &mouse = *i;
		if (mouse.id == id)
			i = _mouseRegions.erase(i);
		else
			++i;
	}
}

} // namespace AGDS
