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

#ifndef AGDS_MOUSE_MAP_H
#define AGDS_MOUSE_MAP_H

#include "common/list.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/str.h"

namespace AGDS {

class AGDSEngine;
struct Region;
typedef Common::SharedPtr<Region> RegionPtr;

struct MouseRegion {
	int			id;
	RegionPtr	region;
	bool		enabled;
	bool		visible;

	Common::String onEnter;
	Common::String onLeave;

	void enable() {
		enabled = true;
	}

	void disable(AGDSEngine *engine) {
		enabled = false;
		hide(engine);
	}

	MouseRegion(RegionPtr reg, const Common::String &enter, const Common::String &leave):
		id(-1), region(reg), enabled(1), visible(false), onEnter(enter), onLeave(leave) {
	}

	void hide(AGDSEngine * engine);
	void show(AGDSEngine * engine);
};

class MouseMap {
	typedef Common::List<MouseRegion> MouseRegionsType;
	MouseRegionsType	_mouseRegions;
	int					_nextId;
	bool				_disabled;

public:
	MouseMap(): _nextId(0), _disabled(false) { }

	void disable(AGDSEngine * engine, bool disabled) {
		_disabled = disabled;
		if (disabled)
			hideAll(engine);
	}

	bool disabled() const {
		return _disabled;
	}

	int add(const MouseRegion & area);
	void remove(AGDSEngine *engine, int id);

	void hideAll(AGDSEngine *engine);

	void clear() {
		_mouseRegions.clear();
	}
	MouseRegion * find(Common::Point pos);
	MouseRegion * find(int id);
};

} // End of namespace AGDS

#endif /* AGDS_SCREEN_H */
