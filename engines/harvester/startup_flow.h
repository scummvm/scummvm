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

#ifndef HARVESTER_STARTUP_FLOW_H
#define HARVESTER_STARTUP_FLOW_H

#include "common/array.h"
#include "common/error.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common {
struct Event;
}

namespace Harvester {

class HarvesterEngine;

class StartupFlow {
public:
	explicit StartupFlow(HarvesterEngine &engine);

	bool load();
	Common::Error run();

private:
	bool loadQuickTips();
	bool loadMenuItems();
	Common::Error runQuickTips();
	Common::Error runMainMenuStub();
	void renderQuickTipsScreen(const Common::String &tipText) const;
	void renderMainMenuStub(int selectedItem, const Common::String &statusMessage) const;
	bool handleSystemEvent(const Common::Event &event, Common::Error &result);
	int getMenuItemAt(const Common::Point &mousePos) const;

	HarvesterEngine &_engine;
	Common::Array<Common::String> _quickTips;
	Common::Array<Common::String> _menuItems;
	Common::Point _mousePos;
};

} // End of namespace Harvester

#endif // HARVESTER_STARTUP_FLOW_H
