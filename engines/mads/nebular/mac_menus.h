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

#ifndef MADS_NEBULAR_MAC_MENUS_H
#define MADS_NEBULAR_MAC_MENUS_H

#include "common/scummsys.h"

namespace Common {
struct Event;
class String;
}

namespace Graphics {
class MacMenu;
struct MacMenuItem;
struct MacMenuSubMenu;
class MacWindowManager;
class ManagedSurface;
}

namespace MADS {
namespace RexNebular {

class MacResourceProvider;
class RexNebularEngine;

class MacNebularMenu {
private:
	RexNebularEngine &_engine;
	MacResourceProvider &_resources;
	Graphics::ManagedSurface &_screen;
	Graphics::MacWindowManager *_windowManager = nullptr;
	Graphics::MacMenu *_menu = nullptr;
	byte _palette[256 * 3] = {};
	bool _paletteValid = false;
	bool _outerMenuActive = false;
	int _pendingCommand = -1;

	static void menuCallback(int commandId, Common::String &name, void *data);
	bool initializeWindowManager();
	bool loadMenuResource(uint16 resourceID,
		Graphics::MacMenuSubMenu *parent = nullptr, int parentItem = -1);
	Graphics::MacMenuItem *getMenuItem(int menu, int item) const;
	Graphics::MacMenuItem *getSubMenuItem(int menu, int parentItem, int item) const;
	void setItemState(Graphics::MacMenuItem *item, bool enabled, bool checked);
	void updateState();
	void dispatchCommand(int commandId);
	void syncPalette();

public:
	MacNebularMenu(RexNebularEngine &engine, MacResourceProvider &resources,
		Graphics::ManagedSurface &screen);
	~MacNebularMenu();

	bool initialize();
	bool processEvent(Common::Event &event);
	void draw();
	byte getBlackColor();
	void getMenuColors(byte &menuBlack, byte &menuWhite);
	void setOuterMenuActive(bool active) { _outerMenuActive = active; }
	int runDifficultyDialog();
	int selectResumeSlot();
};

void selectMacintoshDifficulty(MacNebularMenu *menus);
void macintoshGameMenu();

} // namespace RexNebular
} // namespace MADS

#endif
