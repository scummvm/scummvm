/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MOUSE_MAP_H
#define MOUSE_MAP_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/str.h"

namespace AGDS {

class AGDSEngine;
struct Region;
using RegionPtr = Common::SharedPtr<Region>;

struct MouseRegion {
	int id = -1;
	RegionPtr region = nullptr;
	bool enabled = true;
	bool visible = false;

	Common::String onEnter;
	Common::String onLeave;

	void enable() {
		enabled = true;
	}

	void disable() {
		enabled = false;
	}

	MouseRegion() {}

	MouseRegion(RegionPtr reg, const Common::String &enter, const Common::String &leave) : region(reg), onEnter(enter), onLeave(leave) {
	}

	void hide(AGDSEngine *engine);
	void show(AGDSEngine *engine);
};
using MouseRegionPtr = Common::ScopedPtr<MouseRegion>;

class MouseMap {
	Common::Array<MouseRegionPtr> _mouseRegions;
	bool _disabled;

public:
	MouseMap() : _mouseRegions(100), _disabled(false) {}

	void disable(bool disabled) {
		_disabled = disabled;
	}

	bool disabled() const {
		return _disabled;
	}

	int findFree() const;
	int add(MouseRegion area);
	void remove(int id);

	void hideAll(AGDSEngine *engine);

	void hideInactive(AGDSEngine *engine, Common::Point pos);
	MouseRegion *find(int id);
};

} // End of namespace AGDS

#endif /* AGDS_SCREEN_H */
