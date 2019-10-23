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

	if (_vm->shouldQuit())
		return -1;

	result = 0;
	while (!_vm->shouldQuit()) {
		_vm->_objectsMan->_forestFl = false;
		_vm->_events->_breakoutFl = false;
		_vm->_globals->_disableInventFl = true;
		_vm->_globals->_exitId = 0;

		for (int idx = 0; idx < 31; ++idx)
			_vm->_globals->_inventory[idx] = 0;

		memset(_vm->_globals->_saveData->_data, 0, 2050-50);
		_vm->_objectsMan->addObject(14);
		memset(frameIndex, 0, sizeof(int) * ARRAYSIZE(frameIndex));

		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			_vm->_graphicsMan->loadImage("MENU");
		else {
			switch (_vm->_globals->_language) {
			case LANG_EN:
				_vm->_graphicsMan->loadImage("MENUAN");
				break;
			case LANG_FR:
				_vm->_graphicsMan->loadImage("MENUFR");
				break;
			case LANG_SP:
				_vm->_graphicsMan->loadImage("MENUES");
				break;
			default:
				break;
			}
		}

		_vm->_graphicsMan->fadeInLong();

		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			spriteData = _vm->_objectsMan->loadSprite("MENU.SPR");
		else {
			switch (_vm->_globals->_language) {
			case LANG_EN:
				spriteData = _vm->_objectsMan->loadSprite("MENUAN.SPR");
				break;
			case LANG_FR:
				spriteData = _vm->_objectsMan->loadSprite("MENUFR.SPR");
				break;
			case LANG_SP:
				spriteData = _vm->_objectsMan->loadSprite("MENUES.SPR");
				break;
			default:
				break;
			}
		}

		_vm->_events->mouseOn();
		_vm->_events->changeMouseCursor(0);
		_vm->_events->_mouseCursorId = 0;
		_vm->_events->_mouseSpriteId = 0;

		_vm->_soundMan->playSound(28);

		// Loop to make menu selection
		bool selectionMade = false;
		do {
			if (_vm->shouldQuit())
				return -1;

			menuIndex = MENU_NONE;
			mousePos = Common::Point(_vm->_events->getMouseX(), _vm->_events->getMouseY());

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

			_vm->_graphicsMan->fastDisplay(spriteData, 230, 259, frameIndex[0]);
			_vm->_graphicsMan->fastDisplay(spriteData, 230, 291, frameIndex[1] + 2);
			_vm->_graphicsMan->fastDisplay(spriteData, 230, 322, frameIndex[2] + 4);
			_vm->_graphicsMan->fastDisplay(spriteData, 230, 354, frameIndex[3] + 6);
			_vm->_graphicsMan->fastDisplay(spriteData, 230, 386, frameIndex[4] + 8);
			_vm->_events->refreshScreenAndEvents();

			if (_vm->_events->getMouseButton() == 1 && menuIndex != MENU_NONE)
				selectionMade = true;
		} while (!selectionMade);

		if (menuIndex > MENU_NONE) {
			_vm->_graphicsMan->fastDisplay(spriteData, 230, 259 + 32 * (menuIndex - 1), 10 + (menuIndex - 1));
			_vm->_events->refreshScreenAndEvents();
			_vm->_events->delay(200);
		}

		if (menuIndex == PLAY_GAME) {
			result = 1;
			break;
		} else if (menuIndex == LOAD_GAME) {
			_vm->_globals->_exitId = -1;
			_vm->_dialog->showLoadGame();

			if (_vm->_globals->_exitId != -1) {
				result = _vm->_globals->_exitId;
				break;
			}
			_vm->_globals->_exitId = 0;
		} else if (menuIndex == OPTIONS) {
			_vm->_dialog->showOptionsDialog();
		} else if (menuIndex == INTRODUCTION) {
			_vm->playIntro();
		} else if (menuIndex == QUIT) {
			result = -1;
			break;
		}
	}

	_vm->_globals->freeMemory(spriteData);
	_vm->_globals->_disableInventFl = false;
	_vm->_graphicsMan->fadeOutLong();
	return result;
}

} // End of namespace Hopkins
