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

#include "agds/mouseMap.h"
#include "agds/agds.h"
#include "agds/object.h"
#include "agds/region.h"

namespace AGDS {

int MouseMap::findFree() const {
	for (int i = 0, n = _mouseRegions.size(); i != n; ++i) {
		auto &region = _mouseRegions[i];
		if (!region)
			return i;
	}
	error("no mouse region available");
}

int MouseMap::add(MouseRegion area) {
	auto id = findFree();
	auto &region = _mouseRegions[id];
	region.reset(new MouseRegion(Common::move(area)));
	region->id = id;
	return id;
}

void MouseMap::hideInactive(AGDSEngine *engine, Common::Point pos) {
	for (auto &region : _mouseRegions) {
		if (!region || !region->enabled)
			continue;

		if (_disabled) {
			region->hide(engine);
		} else if (engine->userEnabled()) {
			if (!region->region->pointIn(pos)) {
				region->hide(engine);
			} else if (!region->visible) {
				region->show(engine);
				return;
			}
		}
	}
}

MouseRegion *MouseMap::find(int id) {
	for (auto &region : _mouseRegions) {
		if (region && region->id == id)
			return region.get();
	}
	return nullptr;
}

void MouseMap::remove(int id) {
	auto &region = _mouseRegions[id];
	if (!region) {
		warning("removing non-existent mouse region %d", id);
		return;
	}
	region->disable();
	region.reset();
}

void MouseRegion::show(AGDSEngine *engine) {
	if (visible)
		return;

	visible = true;
	debug("calling mouseArea[%d].onEnter: %s", id, onEnter.c_str());
	engine->runObject(onEnter);
}

void MouseRegion::hide(AGDSEngine *engine) {
	if (!visible)
		return;

	visible = false;
	debug("calling mouseArea[%d].onLeave: %s...", id, onLeave.c_str());
	engine->runObject(onLeave);
}

void MouseMap::hideAll(AGDSEngine *engine) {
	for (auto &region : _mouseRegions)
		if (region)
			region->hide(engine);
}

} // End of namespace AGDS
