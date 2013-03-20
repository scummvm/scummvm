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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "hopkins/menu.h"

#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/hopkins.h"
#include "hopkins/globals.h"
#include "hopkins/events.h"
#include "hopkins/graphics.h"
#include "hopkins/sound.h"

#include "common/scummsys.h"
#include "common/events.h"
#include "common/file.h"
#include "common/util.h"

namespace Hopkins {

enum MenuSelection { MENU_NONE = 0, PLAY_GAME = 1, LOAD_GAME = 2, OPTIONS = 3, INTRODUCTION = 4, QUIT = 5 };

MenuManager::MenuManager(HopkinsEngine *vm) {
	_vm = vm;
}

int MenuManager::menu() {
	byte *spriteData = NULL;
	MenuSelection menuIndex;
	Common::Point mousePos;
	signed int result;
	int frameIndex[] = { 0, 0, 0, 0, 0 };

	if (g_system->getEventManager()->shouldQuit())
		return -1;

	result = 0;
	while (!g_system->getEventManager()->shouldQuit()) {
		_vm->_objectsManager->_forestFl = false;
		_vm->_eventsManager->_breakoutFl = false;
		_vm->_globals->_disableInventFl = true;
		_vm->_globals->_exitId = 0;

		for (int idx = 0; idx < 31; ++idx)
			_vm->_globals->_inventory[idx] = 0;

		memset(_vm->_globals->_saveData, 0, 2000);
		_vm->_objectsManager->addObject(14);
		memset(frameIndex, 0, sizeof(int) * ARRAYSIZE(frameIndex));

		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			_vm->_graphicsManager->loadImage("MENU");
		else if (_vm->_globals->_language == LANG_EN)
			_vm->_graphicsManager->loadImage("MENUAN");
		else if (_vm->_globals->_language == LANG_FR)
			_vm->_graphicsManager->loadImage("MENUFR");
		else if (_vm->_globals->_language == LANG_SP)
			_vm->_graphicsManager->loadImage("MENUES");

		_vm->_graphicsManager->fadeInLong();

		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			spriteData = _vm->_objectsManager->loadSprite("MENU.SPR");
		else if (_vm->_globals->_language == LANG_EN)
			spriteData = _vm->_objectsManager->loadSprite("MENUAN.SPR");
		else if (_vm->_globals->_language == LANG_FR)
			spriteData = _vm->_objectsManager->loadSprite("MENUFR.SPR");
		else if (_vm->_globals->_language == LANG_SP)
			spriteData = _vm->_objectsManager->loadSprite("MENUES.SPR");

		_vm->_eventsManager->mouseOn();
		_vm->_eventsManager->changeMouseCursor(0);
		_vm->_eventsManager->_mouseCursorId = 0;
		_vm->_eventsManager->_mouseSpriteId = 0;

		_vm->_soundManager->playSound(28);

		// Loop to make menu selection
		bool selectionMade = false;
		do {
			if (g_system->getEventManager()->shouldQuit())
				return -1;

			menuIndex = MENU_NONE;
			mousePos = Common::Point(_vm->_eventsManager->getMouseX(), _vm->_eventsManager->getMouseY());

			if (mousePos.x >= 232 && mousePos.x <= 408) {
				if (mousePos.y >= 261 && mousePos.y <= 284)
					menuIndex = PLAY_GAME;
				else if (mousePos.y >= 293 && mousePos.y <= 316)
					menuIndex = LOAD_GAME;
				else if (mousePos.y >= 325 && mousePos.y <= 347)
					menuIndex = OPTIONS;
				else if (mousePos.y >= 356 && mousePos.y <= 379)
					menuIndex = INTRODUCTION;
				else if (mousePos.y >= 388 && mousePos.y <= 411)
					menuIndex = QUIT;
			}

			memset(frameIndex, 0, sizeof(int) * ARRAYSIZE(frameIndex));
			if (menuIndex > MENU_NONE)
				frameIndex[menuIndex - 1] = 1;

			_vm->_graphicsManager->fastDisplay(spriteData, 230, 259, frameIndex[0]);
			_vm->_graphicsManager->fastDisplay(spriteData, 230, 291, frameIndex[1] + 2);
			_vm->_graphicsManager->fastDisplay(spriteData, 230, 322, frameIndex[2] + 4);
			_vm->_graphicsManager->fastDisplay(spriteData, 230, 354, frameIndex[3] + 6);
			_vm->_graphicsManager->fastDisplay(spriteData, 230, 386, frameIndex[4] + 8);
			_vm->_eventsManager->refreshScreenAndEvents();

			if (_vm->_eventsManager->getMouseButton() == 1 && menuIndex != MENU_NONE)
				selectionMade = true;
		} while (!selectionMade);

		if (menuIndex > MENU_NONE) {
			_vm->_graphicsManager->fastDisplay(spriteData, 230, 259 + 32 * (menuIndex - 1), 10 + (menuIndex - 1));
			_vm->_eventsManager->refreshScreenAndEvents();
			_vm->_eventsManager->delay(200);
		}

		if (menuIndex == PLAY_GAME) {
			result = 1;
			break;
		} else if (menuIndex == LOAD_GAME) {
			_vm->_globals->_exitId = -1;
			_vm->_dialogsManager->showLoadGame();

			if (_vm->_globals->_exitId != -1) {
				result = _vm->_globals->_exitId;
				break;
			}
			_vm->_globals->_exitId = 0;
		} else if (menuIndex == OPTIONS) {
			_vm->_dialogsManager->showOptionsDialog();
		} else if (menuIndex == INTRODUCTION) {
			_vm->playIntro();
		} else if (menuIndex == QUIT) {
			result = -1;
			break;
		}
	}

	_vm->_globals->freeMemory(spriteData);
	_vm->_globals->_disableInventFl = false;
	_vm->_graphicsManager->fadeOutLong();
	return result;
}

} // End of namespace Hopkins
