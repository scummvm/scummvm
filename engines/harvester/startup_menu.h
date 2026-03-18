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

#ifndef HARVESTER_STARTUP_MENU_H
#define HARVESTER_STARTUP_MENU_H

#include "common/array.h"
#include "common/error.h"
#include "common/rect.h"
#include "common/str.h"

namespace Graphics {
class Font;
}

namespace Harvester {

class HarvesterEngine;
class StartupFlow;
struct IndexedBitmap;

class StartupMenuSystem {
public:
	StartupMenuSystem(HarvesterEngine &engine, Common::Point &mousePos,
		const Common::Array<Common::String> &menuItems);

	Common::Error runMainMenuStub(StartupFlow &startupFlow);
	Common::Error runRoomMenuStub(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, StartupFlow &startupFlow);

private:
	Common::Error runOptionsMenu(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, StartupFlow &startupFlow);
	Common::Error runHelpScreen(const byte *palette, float paletteBrightness, StartupFlow &startupFlow);
	Common::Error runSaveGameMenu(const byte *palette, float paletteBrightness, StartupFlow &startupFlow);
	Common::Error runQuitGameConfirm(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, StartupFlow &startupFlow);
	Common::String runModalTextEntryDialog(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const Graphics::Font &titleFont, const Graphics::Font &entryFont,
		const Common::String &title, const Common::String &initialText, StartupFlow &startupFlow);
	void renderMainMenuStub(int selectedItem, const Common::String &statusMessage) const;
	void renderRoomMenuStub(const IndexedBitmap &backdrop, int selectedItem) const;
	int getMenuItemAt(const Common::Point &mousePos) const;
	int getRoomMenuItemAt(const Common::Point &mousePos) const;

	HarvesterEngine &_engine;
	Common::Point &_mousePos;
	const Common::Array<Common::String> &_menuItems;
};

} // End of namespace Harvester

#endif // HARVESTER_STARTUP_MENU_H
