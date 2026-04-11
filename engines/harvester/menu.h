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

#ifndef HARVESTER_MENU_H
#define HARVESTER_MENU_H

#include "common/array.h"
#include "common/error.h"
#include "common/rect.h"
#include "common/str.h"
#include "harvester/art.h"

namespace Graphics {
class Font;
}

namespace Harvester {

class HarvesterEngine;
class Flow;

class MenuSystem {
public:
	MenuSystem(HarvesterEngine &engine, Common::Point &mousePos,
		const Common::Array<Common::String> &menuItems);

	Common::Error runMainMenuStub(Flow &flow);
	Common::Error runRoomMenuStub(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow, bool canSaveGame);

private:
	Common::Error runLoadGameMenu(const byte *palette, float paletteBrightness,
		Flow &flow, bool &loadedGame);
	Common::Error runOptionsMenu(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow);
	Common::Error runHelpScreen(const byte *palette, float paletteBrightness, Flow &flow);
	Common::Error runSaveGameMenu(const byte *palette, float paletteBrightness,
		Flow &flow, bool &savedGame);
	Common::Error restoreRoomBackdropAfterSave(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow) const;
	Common::Error runConfirmPrompt(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow, const Common::String &promptText,
		bool &confirmed);
	Common::Error runQuitGameConfirm(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow);
	Common::Error showGameOverBackdrop(Flow &flow);
	void clearMainMenuBackdrop();
	void renderMainMenuStub(const Common::Array<Common::String> &menuItems, int selectedItem,
		const Common::String &statusMessage) const;
	void renderMainMenuScreen(int selectedItem, const Common::String &statusMessage,
		bool drawCursor, const Common::Array<Common::String> &menuItems) const;
	void renderBackdropMenuScreen(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const Common::Array<Common::String> &menuItems,
		int selectedItem) const;
	int getMenuItemAt(const Common::Point &mousePos, const Common::Array<Common::String> &menuItems) const;
	int getBackdropMenuItemAt(const Common::Point &mousePos,
		const Common::Array<Common::String> &menuItems) const;

	HarvesterEngine &_engine;
	Common::Point &_mousePos;
	const Common::Array<Common::String> &_menuItems;
	IndexedBitmap _mainMenuBackdrop;
	byte _mainMenuBackdropPalette[256 * 3] = { 0 };
	bool _hasMainMenuBackdrop = false;
};

} // End of namespace Harvester

#endif // HARVESTER_MENU_H
