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

#include "common/scummsys.h"
#include "common/events.h"
#include "common/file.h"
#include "common/util.h"
#include "hopkins/menu.h"
#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/hopkins.h"
#include "hopkins/globals.h"
#include "hopkins/events.h"
#include "hopkins/graphics.h"
#include "hopkins/sound.h"

namespace Hopkins {

void MenuManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

enum MenuSelection { MENU_NONE = 0, PLAY_GAME = 1, LOAD_GAME = 2, OPTIONS = 3, INTRODUCTION = 4, QUIT = 5 };

int MenuManager::menu() {
	byte *spriteData = NULL;
	MenuSelection menuIndex;
	Common::Point mousePos;
	signed int result;
	int frame5Index;
	int frame4Index;
	int frame3Index;
	int frame2Index;
	int frame1Index;

	if (g_system->getEventManager()->shouldQuit())
		return -1;

	result = 0;
	while (!g_system->getEventManager()->shouldQuit()) {
		_vm->_globals._forestFl = false;
		_vm->_eventsManager._breakoutFl = false;
		_vm->_globals._disableInventFl = true;
		_vm->_globals._exitId = 0;

		for (int idx = 0; idx < 31; ++idx)
			_vm->_globals._inventory[idx] = 0;

		memset(_vm->_globals._saveData, 0, 2000);
		_vm->_objectsManager.addObject(14);
		frame5Index = 0;
		frame4Index = 0;
		frame3Index = 0;
		frame2Index = 0;
		frame1Index = 0;

		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			_vm->_graphicsManager.loadImage("MENU");
		else if (_vm->_globals._language == LANG_EN)
			_vm->_graphicsManager.loadImage("MENUAN");
		else if (_vm->_globals._language == LANG_FR)
			_vm->_graphicsManager.loadImage("MENUFR");
		else if (_vm->_globals._language == LANG_SP)
			_vm->_graphicsManager.loadImage("MENUES");

		_vm->_graphicsManager.fadeInLong();

		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			_vm->_fileManager.constructFilename("SYSTEM", "MENU.SPR");
		else if (_vm->_globals._language == LANG_EN)
			_vm->_fileManager.constructFilename("SYSTEM", "MENUAN.SPR");
		else if (_vm->_globals._language == LANG_FR)
			_vm->_fileManager.constructFilename("SYSTEM", "MENUFR.SPR");
		else if (_vm->_globals._language == LANG_SP)
			_vm->_fileManager.constructFilename("SYSTEM", "MENUES.SPR");

		spriteData = _vm->_objectsManager.loadSprite(_vm->_globals._curFilename);
		_vm->_eventsManager.mouseOn();
		_vm->_eventsManager.changeMouseCursor(0);
		_vm->_eventsManager._mouseCursorId = 0;
		_vm->_eventsManager._mouseSpriteId = 0;

		for (;;) {
			for (;;) {
				_vm->_soundManager.WSOUND(28);

				// Loop to make menu selection
				bool selectionMade = false;
				do {
					if (g_system->getEventManager()->shouldQuit())
						return -1;

					menuIndex = MENU_NONE;
					mousePos = Common::Point(_vm->_eventsManager.getMouseX(), _vm->_eventsManager.getMouseY());

					if ((uint16)(mousePos.x - 232) <= 176) {
						if ((uint16)(mousePos.y - 261) <= 23)
							menuIndex = PLAY_GAME;
						if ((uint16)(mousePos.y - 293) <= 23)
							menuIndex = LOAD_GAME;
						if ((uint16)(mousePos.y - 325) <= 22)
							menuIndex = OPTIONS;
						if ((uint16)(mousePos.y - 356) <= 23)
							menuIndex = INTRODUCTION;

						if ((uint16)(mousePos.y - 388) <= 23)
							menuIndex = QUIT;
					}

					switch (menuIndex) {
					case MENU_NONE:
						frame1Index = 0;
						frame2Index = 0;
						frame3Index = 0;
						frame4Index = 0;
						frame5Index = 0;
						break;
					case PLAY_GAME:
						frame1Index = 1;
						frame2Index = 0;
						frame3Index = 0;
						frame4Index = 0;
						frame5Index = 0;
						break;
					case LOAD_GAME:
						frame1Index = 0;
						frame2Index = 1;
						frame3Index = 0;
						frame4Index = 0;
						frame5Index = 0;
						break;
					case OPTIONS:
						frame1Index = 0;
						frame2Index = 0;
						frame3Index = 1;
						frame4Index = 0;
						frame5Index = 0;
						break;
					case INTRODUCTION:
						frame1Index = 0;
						frame2Index = 0;
						frame3Index = 0;
						frame4Index = 1;
						frame5Index = 0;
						break;
					case QUIT:
						frame1Index = 0;
						frame2Index = 0;
						frame3Index = 0;
						frame4Index = 0;
						frame5Index = 1;
					}

					_vm->_graphicsManager.fastDisplay(spriteData, 230, 259, frame1Index);
					_vm->_graphicsManager.fastDisplay(spriteData, 230, 291, frame2Index + 2);
					_vm->_graphicsManager.fastDisplay(spriteData, 230, 322, frame3Index + 4);
					_vm->_graphicsManager.fastDisplay(spriteData, 230, 354, frame4Index + 6);
					_vm->_graphicsManager.fastDisplay(spriteData, 230, 386, frame5Index + 8);
					_vm->_eventsManager.VBL();

					if (_vm->_eventsManager.getMouseButton() == 1 && menuIndex != MENU_NONE)
						selectionMade = true;
				} while (!selectionMade);

				if (menuIndex == PLAY_GAME) {
					_vm->_graphicsManager.fastDisplay(spriteData, 230, 259, 10);
					_vm->_eventsManager.VBL();
					_vm->_eventsManager.delay(200);
					result = 1;
				}
				if (menuIndex != LOAD_GAME)
					break;

				_vm->_graphicsManager.fastDisplay(spriteData, 230, 291, 11);
				_vm->_eventsManager.VBL();
				_vm->_eventsManager.delay(200);

				_vm->_globals._exitId = -1;
				_vm->_dialogsManager.showLoadGame();

				if (_vm->_globals._exitId != -1) {
					result = _vm->_globals._exitId;
					break;
				}
				_vm->_globals._exitId = 0;
			}

			if (menuIndex != OPTIONS)
				break;

			// Options menu item selected
			_vm->_graphicsManager.fastDisplay(spriteData, 230, 322, 12);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.delay(200);

			// Show the options dialog
			_vm->_dialogsManager.showOptionsDialog();
		}
		if (menuIndex == INTRODUCTION) {
			_vm->_graphicsManager.fastDisplay(spriteData, 230, 354, 13);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.delay(200);
			_vm->playIntro();
			continue;
		}

		if ( menuIndex == QUIT) {
			_vm->_graphicsManager.fastDisplay(spriteData, 230, 386, 14);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.delay(200);
			result = -1;
		}
		break;
	}

	_vm->_globals.freeMemory(spriteData);
	_vm->_globals._disableInventFl = false;
	_vm->_graphicsManager.fadeOutLong();
	return result;
}

} // End of namespace Hopkins
