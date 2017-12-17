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

#ifndef AGDS_SCREEN_H
#define AGDS_SCREEN_H

#include "common/scummsys.h"
#include "common/list.h"
#include "common/str.h"
#include "common/rect.h"

namespace Graphics {
	struct Surface;
}

namespace AGDS {

class Object;
struct Region;

struct MouseRegion {
	int			id;
	Region *	region;
	int			enabled;
	bool		currentlyIn;

	Common::String onEnter;
	Common::String onLeave;

	void enable() {
		++enabled;
	}

	void disable() {
		if (enabled > 0)
			--enabled;
	}

	MouseRegion(Region * reg, const Common::String &enter, const Common::String &leave):
		id(-1), region(reg), enabled(1), currentlyIn(false), onEnter(enter), onLeave(leave) {
	}
};

//fixme: move me away
class MouseMap {
	typedef Common::List<MouseRegion> MouseRegionsType;
	MouseRegionsType	_mouseRegions;
	int					_nextId;
	bool				_disabled;

public:
	MouseMap(): _nextId(0), _disabled(false) { }

	void disable(bool disabled) {
		_disabled = disabled;
	}

	int add(const MouseRegion & area) {
		_mouseRegions.push_back(area);
		_mouseRegions.back().id = _nextId++;
		return _mouseRegions.back().id;
	}
//	void clear() {
//		_mouseRegions.clear();
//	}
	MouseRegion * find(Common::Point pos);
	MouseRegion * find(int id);
	void remove(int id);
};

class Screen {
	Common::String _name;
	typedef Common::List<Object *> ChildrenType;
	ChildrenType _children;

public:
	Screen(Object *object);

	const Common::String &getName() const {
		return _name;
	}

	void add(Object *object);
	void remove(const Common::String &name);
	void paint(Graphics::Surface &backbuffer);
	Object *find(Common::Point pos) const;
};


} // End of namespace AGDS

#endif /* AGDS_SCREEN_H */
