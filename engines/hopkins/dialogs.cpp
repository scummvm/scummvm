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
#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/util.h"
#include "hopkins/dialogs.h"
#include "hopkins/events.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/sound.h"

namespace Hopkins {

DialogsManager::DialogsManager() {
	_inventFl = false;
	_inventDisplayedFl = false;
	_removeInventFl = false;
	_inventX = _inventY = 0;
	_inventWidth = _inventHeight = 0;
	_inventWin1 = g_PTRNUL;
	_inventBuf2 = g_PTRNUL;
}

DialogsManager::~DialogsManager() {
	_vm->_globals.freeMemory(_inventWin1);
}

void DialogsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void DialogsManager::showOptionsDialog() {
	bool doneFlag;

	doneFlag = false;
	_vm->_eventsManager.changeMouseCursor(0);
	_vm->_eventsManager.VBL();
	if (_vm->_globals._language == LANG_FR)
		_vm->_fileManager.constructFilename("SYSTEM", "OPTIFR.SPR");
	else if (_vm->_globals._language == LANG_EN)
		_vm->_fileManager.constructFilename("SYSTEM", "OPTIAN.SPR");
	else if (_vm->_globals._language == LANG_SP)
		_vm->_fileManager.constructFilename("SYSTEM", "OPTIES.SPR");

	_vm->_globals.OPTION_SPR = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	_vm->_globals._optionDialogFl = true;

	do {
		if (_vm->_eventsManager.getMouseButton()) {
			Common::Point mousePos(_vm->_eventsManager.getMouseX(), _vm->_eventsManager.getMouseY());
			mousePos.x = _vm->_eventsManager.getMouseX();
			mousePos.y = _vm->_eventsManager.getMouseY();

			if (!_vm->_soundManager._musicOffFl) {
				if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 300 && mousePos.y > 113 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 327 && mousePos.y <= 138) {
					// Change the music volume
					++_vm->_soundManager._musicVolume;

					if (_vm->_soundManager._musicVolume <= 12)
						_vm->_soundManager.playSound("bruit2.wav");
					else
						_vm->_soundManager._musicVolume = 12;
					_vm->_soundManager.MODSetMusicVolume(_vm->_soundManager._musicVolume);

					_vm->_soundManager.updateScummVMSoundSettings();
				}

				if (!_vm->_soundManager._musicOffFl && mousePos.x >= _vm->_graphicsManager._scrollOffset + 331 && mousePos.y > 113 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 358 && mousePos.y <= 138) {
					--_vm->_soundManager._musicVolume;
					if (_vm->_soundManager._musicVolume >= 0)
						_vm->_soundManager.playSound("bruit2.wav");
					else
						_vm->_soundManager._musicVolume = 0;

					_vm->_soundManager.MODSetMusicVolume(_vm->_soundManager._musicVolume);

					_vm->_soundManager.updateScummVMSoundSettings();
				}
			}
			if (!_vm->_soundManager._soundOffFl) {
				if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 300 && mousePos.y > 140 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 327 && mousePos.y <= 165) {
					++_vm->_soundManager._soundVolume;
					if (_vm->_soundManager._soundVolume <= 16)
						_vm->_soundManager.playSound("bruit2.wav");
					else
						_vm->_soundManager._soundVolume = 16;
					_vm->_soundManager.MODSetSampleVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}

				if (!_vm->_soundManager._soundOffFl && mousePos.x >= _vm->_graphicsManager._scrollOffset + 331 && mousePos.y > 140 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 358 && mousePos.y <= 165) {
					--_vm->_soundManager._soundVolume;
					if (_vm->_soundManager._soundVolume >= 0)
						_vm->_soundManager.playSound("bruit2.wav");
					else
						_vm->_soundManager._soundVolume = 0;
					_vm->_soundManager.MODSetSampleVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}
			}

			if (!_vm->_soundManager._voiceOffFl) {
				if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 300 && mousePos.y > 167 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 327 && mousePos.y <= 192) {
					++_vm->_soundManager._voiceVolume;

					if (_vm->_soundManager._voiceVolume <= 16)
						_vm->_soundManager.playSound("bruit2.wav");
					else
						_vm->_soundManager._voiceVolume = 16;
					_vm->_soundManager.MODSetVoiceVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}

				if (!_vm->_soundManager._voiceOffFl && mousePos.x >= _vm->_graphicsManager._scrollOffset + 331 && mousePos.y > 167 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 358 && mousePos.y <= 192) {
					--_vm->_soundManager._voiceVolume;
					if (_vm->_soundManager._voiceVolume >= 0)
						_vm->_soundManager.playSound("bruit2.wav");
					else
						_vm->_soundManager._voiceVolume = 0;
					_vm->_soundManager.MODSetVoiceVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}
			}

			if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 431) {
				if (mousePos.y > 194 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 489 && mousePos.y <= 219)
					_vm->_soundManager._textOffFl = !_vm->_soundManager._textOffFl;

				if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 431) {
					if (mousePos.y > 167 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 489 && mousePos.y <= 192) {
						_vm->_soundManager._voiceOffFl = !_vm->_soundManager._voiceOffFl;

						_vm->_soundManager.updateScummVMSoundSettings();
					}
					if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 431) {
						if (mousePos.y > 113 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 489 && mousePos.y <= 138) {
							if (_vm->_soundManager._musicOffFl) {
								_vm->_soundManager._musicOffFl = false;
								_vm->_soundManager.MODSetMusicVolume(_vm->_soundManager._musicVolume);
							} else {
								_vm->_soundManager._musicOffFl = true;
								_vm->_soundManager.MODSetMusicVolume(0);
							}

							_vm->_soundManager.updateScummVMSoundSettings();
						}

						if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 431 && mousePos.y > 140 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 489 && mousePos.y <= 165) {
							_vm->_soundManager._soundOffFl = !_vm->_soundManager._soundOffFl;

							_vm->_soundManager.updateScummVMSoundSettings();
						}
					}
				}
			}

			if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 175 && mousePos.y > 285 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 281 && mousePos.y <= 310) {
				_vm->_globals._exitId = 300;
				doneFlag = true;
			}
			if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 355 && mousePos.y > 285 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 490 && mousePos.y <= 310)
				doneFlag = true;
			if (mousePos.x >= _vm->_graphicsManager._scrollOffset + 300 && mousePos.y > 194 && mousePos.x <= _vm->_graphicsManager._scrollOffset + 358 && mousePos.y <= 219) {
				switch (_vm->_graphicsManager._scrollSpeed) {
				case 1:
					_vm->_graphicsManager._scrollSpeed = 2;
					break;
				case 2:
					_vm->_graphicsManager._scrollSpeed = 4;
					break;
				case 4:
					_vm->_graphicsManager._scrollSpeed = 8;
					break;
				case 8:
					_vm->_graphicsManager._scrollSpeed = 16;
					break;
				case 16:
					_vm->_graphicsManager._scrollSpeed = 32;
					break;
				case 32:
					_vm->_graphicsManager._scrollSpeed = 48;
					break;
				case 48:
					_vm->_graphicsManager._scrollSpeed = 64;
					break;
				case 64:
					_vm->_graphicsManager._scrollSpeed = 128;
					break;
				case 128:
					_vm->_graphicsManager._scrollSpeed = 160;
					break;
				case 160:
					_vm->_graphicsManager._scrollSpeed = 320;
					break;
				case 320:
					_vm->_graphicsManager._scrollSpeed = 1;
					break;
				}
			}

			// Values are blocked, thus handling the zone is useless
			//if (mousePos.x >= _vm->_graphicsManager.ofscroll + 348 && mousePos.y > 248 && mousePos.x <= _vm->_graphicsManager.ofscroll + 394 && mousePos.y <= 273)
			//	_vm->_globals._speed = 2;

			if (mousePos.x < _vm->_graphicsManager._scrollOffset + 165 || mousePos.x > _vm->_graphicsManager._scrollOffset + 496 || (uint)(mousePos.y - 107) > 211)
				doneFlag = true;
		}

		if (_vm->_graphicsManager.MANU_SCROLL)
			_vm->_globals._menuScrollType = 1;
		else
			_vm->_globals._menuScrollType = 2;

		if (_vm->_globals._speed == 1)
			_vm->_globals._menuSpeed = 6;
		else if (_vm->_globals._speed == 2)
			_vm->_globals._menuSpeed = 5;
		else if (_vm->_globals._speed == 3)
			_vm->_globals._menuSpeed = 4;

		_vm->_globals._menuTextOff = !_vm->_soundManager._textOffFl ? 7 : 8;
		_vm->_globals._menuVoiceOff = !_vm->_soundManager._voiceOffFl ? 7 : 8;
		_vm->_globals._menuSoundOff = !_vm->_soundManager._soundOffFl ? 7 : 8;
		_vm->_globals._menuMusicOff = !_vm->_soundManager._musicOffFl ? 7 : 8;

		_vm->_globals._menuDisplayType = 9;

		if (_vm->_graphicsManager._scrollSpeed == 1)
			_vm->_globals._menuScrollSpeed = 12;
		else if (_vm->_graphicsManager._scrollSpeed == 2)
			_vm->_globals._menuScrollSpeed = 13;
		else if (_vm->_graphicsManager._scrollSpeed == 4)
			_vm->_globals._menuScrollSpeed = 14;
		else if (_vm->_graphicsManager._scrollSpeed == 8)
			_vm->_globals._menuScrollSpeed = 15;
		else if (_vm->_graphicsManager._scrollSpeed == 16)
			_vm->_globals._menuScrollSpeed = 16;
		else if (_vm->_graphicsManager._scrollSpeed == 32)
			_vm->_globals._menuScrollSpeed = 17;
		else if (_vm->_graphicsManager._scrollSpeed == 48)
			_vm->_globals._menuScrollSpeed = 18;
		else if (_vm->_graphicsManager._scrollSpeed == 64)
			_vm->_globals._menuScrollSpeed = 19;
		else if (_vm->_graphicsManager._scrollSpeed == 128)
			_vm->_globals._menuScrollSpeed = 20;
		else if (_vm->_graphicsManager._scrollSpeed == 160)
			_vm->_globals._menuScrollSpeed = 21;
		else if (_vm->_graphicsManager._scrollSpeed == 320)
			_vm->_globals._menuScrollSpeed = 22;
		else if (_vm->_graphicsManager._scrollSpeed == 640)
			_vm->_globals._menuScrollSpeed = 23;

		_vm->_eventsManager.VBL();
	} while (!doneFlag);

	_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen, _vm->_graphicsManager._scrollOffset + 164,
		107, 335, 215, _vm->_graphicsManager._vesaBuffer, _vm->_graphicsManager._scrollOffset + 164, 107);
	_vm->_graphicsManager.addVesaSegment(_vm->_graphicsManager._scrollOffset + 164, 107,
		_vm->_graphicsManager._scrollOffset + 498, 320);

	_vm->_globals.OPTION_SPR = _vm->_globals.freeMemory(_vm->_globals.OPTION_SPR);
	_vm->_globals._optionDialogFl = false;
}

void DialogsManager::showInventory() {
	if (_removeInventFl || _inventDisplayedFl || _vm->_globals._disableInventFl)
		return;

	_vm->_graphicsManager.no_scroll = 1;
	_vm->_objectsManager.FLAG_VISIBLE_EFFACE = 4;
	_vm->_objectsManager._visibleFl = false;
	for (int v1 = 0; v1 <= 1; v1++) {
		inventAnim();
		_vm->_eventsManager.getMouseX();
		_vm->_eventsManager.getMouseY();
		_vm->_eventsManager.VBL();
	}
	_vm->_dialogsManager._inventWin1 = g_PTRNUL;

LABEL_7:
	_vm->_eventsManager._curMouseButton = 0;
	_vm->_eventsManager._mouseButton = 0;
	_vm->_globals._disableInventFl = true;
	_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);

	switch (_vm->_globals._language) {
		case LANG_EN:
			_vm->_fileManager.constructFilename("SYSTEM", "INVENTAN.SPR");
			break;
		case LANG_FR:
			_vm->_fileManager.constructFilename("SYSTEM", "INVENTFR.SPR");
			break;
		case LANG_SP:
			_vm->_fileManager.constructFilename("SYSTEM", "INVENTES.SPR");
			break;
	}

	Common::File f;
	if (!f.open(_vm->_globals._curFilename))
		error("Error opening file - %s", _vm->_globals._curFilename.c_str());

	size_t filesize = f.size();
	_vm->_dialogsManager._inventWin1 = _vm->_globals.allocMemory(filesize);
	_vm->_fileManager.readStream(f, _vm->_dialogsManager._inventWin1, filesize);
	f.close();

	_vm->_fileManager.constructFilename("SYSTEM", "INVENT2.SPR");
	_inventBuf2 = _vm->_fileManager.loadFile(_vm->_globals._curFilename);

	int v19 = _inventX = _vm->_graphicsManager._scrollOffset + 152;
	_inventY = 114;
	int v18 = _inventWidth = _vm->_objectsManager.getWidth(_vm->_dialogsManager._inventWin1, 0);
	int v17 = _inventHeight = _vm->_objectsManager.getHeight(_vm->_dialogsManager._inventWin1, 0);

	_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _vm->_dialogsManager._inventWin1,
		v19 + 300, 414, 0, 0, 0, 0);
	int v15 = 0;
	int v4 = 0;
	for (int v14 = 1; v14 <= 5; v14++) {
		int v16 = 0;
		for (int v5 = 1; v5 <= 6; v5++) {
			++v4;
			int v6 = _vm->_globals._inventory[v4];
			if (v6 && v4 <= 29) {
				byte *v7 = _vm->_objectsManager.CAPTURE_OBJET(v6, 0);
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, v7, v19 + v16 + 6,
					v15 + 120, _vm->_globals._objectWidth, _vm->_globals._objectHeight);
				_vm->_globals.freeMemory(v7);
			}
			v16 += 54;
		};
		v15 += 38;
	}
	_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager._vesaBuffer, _vm->_dialogsManager._inventWin1, _inventX, _inventY, _inventWidth, _inventHeight);
	_vm->_eventsManager._curMouseButton = 0;
	bool v20 = false;
	int v13 = 0;

	// Main loop to select an inventory item
	while (!_vm->shouldQuit()) {
		// Turn on drawing the inventory dialog in the event manager
		_inventDisplayedFl = true;

		int mousePosX = _vm->_eventsManager.getMouseX();
		int mousePosY = _vm->_eventsManager.getMouseY();
		int mouseButton = _vm->_eventsManager.getMouseButton();
		int v10 = v13;
		int v11 = _vm->_linesManager.ZONE_OBJET(mousePosX, mousePosY);
		v13 = v11;
		if (v11 != v10)
			_vm->_objectsManager.initBorder(v11);
		if (_vm->_eventsManager._mouseCursorId != 1 && _vm->_eventsManager._mouseCursorId != 2 && _vm->_eventsManager._mouseCursorId != 3 && _vm->_eventsManager._mouseCursorId != 16) {
			if (mouseButton == 2) {
				_vm->_objectsManager.OBJETPLUS(v13);
				if (_vm->_eventsManager._mouseCursorId != 23)
					_vm->_eventsManager.changeMouseCursor(_vm->_eventsManager._mouseCursorId);
			}
		}
		if (mouseButton == 1) {
			if (_vm->_eventsManager._mouseCursorId == 1 || _vm->_eventsManager._mouseCursorId == 2 || _vm->_eventsManager._mouseCursorId == 3 || _vm->_eventsManager._mouseCursorId == 16 || !_vm->_eventsManager._mouseCursorId)
				break;
			_vm->_objectsManager.VALID_OBJET(_vm->_globals._inventory[v13]);
			if (_vm->_eventsManager._mouseCursorId == 8)
				v20 = true;
			if (!v20) {
				_vm->_scriptManager.TRAVAILOBJET = true;
				_vm->_globals._saveData->data[svField3] = _vm->_globals._curObjectIndex;
				_vm->_globals._saveData->data[svField8] = _vm->_globals._inventory[v13];
				_vm->_globals._saveData->data[svField9] = _vm->_eventsManager._mouseCursorId;
				_vm->_objectsManager.OPTI_OBJET();
				_vm->_scriptManager.TRAVAILOBJET = false;

				if (_vm->_soundManager._voiceOffFl) {
					do
						_vm->_eventsManager.VBL();
					while (!_vm->_globals._exitId && _vm->_eventsManager.getMouseButton() != 1);
					_vm->_fontManager.hideText(9);
				}
				if (_vm->_globals._exitId) {
					if (_vm->_globals._exitId == 2)
						v20 = true;
					_vm->_globals._exitId = 0;
					if (!v20) {
						_inventBuf2 = _vm->_globals.freeMemory(_inventBuf2);
						_vm->_dialogsManager._inventWin1 = _vm->_globals.freeMemory(_vm->_dialogsManager._inventWin1);
						goto LABEL_7;
					}
				} else if (!v20) {
					_inventDisplayedFl = true;
				}
			}
		}
		if (_removeInventFl)
			v20 = true;
		if (v20)
			break;
		_vm->_eventsManager.VBL();
		if (_vm->_globals._screenId >= 35 && _vm->_globals._screenId <= 40)
			_vm->_objectsManager.handleSpecialGames();
	}
	_vm->_fontManager.hideText(9);
	if (_inventDisplayedFl) {
		_inventDisplayedFl = false;
		_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen, v19, 114, v18, v17, _vm->_graphicsManager._vesaBuffer, v19, 114);
		_vm->_graphicsManager.addVesaSegment(v19, 114, v19 + v18, v18 + 114);
		_vm->_objectsManager.BOBTOUS = true;
	}

	_vm->_dialogsManager._inventWin1 = _vm->_globals.freeMemory(_vm->_dialogsManager._inventWin1);
	_inventBuf2 = _vm->_globals.freeMemory(_inventBuf2);

	if (_vm->_eventsManager._mouseCursorId == 1)
		showOptionsDialog();
	else if (_vm->_eventsManager._mouseCursorId == 3)
		_vm->_dialogsManager.showLoadGame();
	else if (_vm->_eventsManager._mouseCursorId == 2)
		_vm->_dialogsManager.showSaveGame();

	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(4);
	_vm->_objectsManager._oldBorderPos = Common::Point(0, 0);
	_vm->_objectsManager._borderPos = Common::Point(0, 0);
	_vm->_globals._disableInventFl = false;
	_vm->_graphicsManager.no_scroll = 0;
}

/**
 * Inventory Animations
 */
void DialogsManager::inventAnim() {
	if (_vm->_globals._disableInventFl)
		return;

	if (_vm->_objectsManager.FLAG_VISIBLE_EFFACE && !_vm->_objectsManager._visibleFl) {
		_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen, _vm->_objectsManager.I_old_x, 27, 48, 38,
			_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager.I_old_x, 27);
		_vm->_graphicsManager.addVesaSegment(_vm->_objectsManager.I_old_x, 27, _vm->_objectsManager.I_old_x + 48, 65);
		--_vm->_objectsManager.FLAG_VISIBLE_EFFACE;
	}

	if (_vm->_objectsManager._visibleFl) {
		if (_vm->_objectsManager.I_old_x <= 1)
			_vm->_objectsManager.I_old_x = 2;
		_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen, _vm->_objectsManager.I_old_x, 27, 48, 38,
			_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager.I_old_x, 27);

		_vm->_graphicsManager.addVesaSegment(_vm->_objectsManager.I_old_x, 27, _vm->_objectsManager.I_old_x + 48, 65);
		int v0 = _vm->_graphicsManager._scrollOffset + 2;
		int v1 = _vm->_graphicsManager._scrollOffset + 2;
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.ICONE, v1 + 300, 327, 0);
		_vm->_graphicsManager.addVesaSegment(v1, 27, v1 + 45, 62);
		_vm->_objectsManager.I_old_x = v0;
	}

	if (_vm->_globals._saveData->data[svField357] == 1) {
		if (_vm->_globals._saveData->data[svField353] == 1)
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
		if (_vm->_globals._saveData->data[svField355] == 1)
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _vm->_globals.TETE, 866, 325, 1, 0, 0, 0);
		_vm->_graphicsManager.addVesaSegment(532, 25, 560, 60);
		_vm->_graphicsManager.addVesaSegment(566, 25, 594, 60);
	}
	if (_vm->_globals._saveData->data[svField356] == 1) {
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
		_vm->_graphicsManager.addVesaSegment(532, 25, 560, 60);
	}

	if (_vm->_globals._saveData->data[svField354] == 1) {
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _vm->_globals.TETE, 832, 325, 0, 0, 0, 0);
		_vm->_graphicsManager.addVesaSegment(532, 25, 560, 60);
	}
}

/**
 * Test dialog opening
 */
void DialogsManager::testDialogOpening() {
	if (_vm->_globals.PLAN_FLAG)
		_vm->_eventsManager._gameKey = KEY_NONE;

	if ((_vm->_eventsManager._gameKey == KEY_NONE) || _inventFl)
		return;

	DIALOG_KEY key = _vm->_eventsManager._gameKey;
	_vm->_eventsManager._gameKey = KEY_NONE;
	_inventFl = true;

	switch (key) {
	case KEY_INVENTORY:
		_vm->_dialogsManager.showInventory();
		break;
	case KEY_OPTIONS:
		_vm->_graphicsManager.no_scroll = 1;
		_vm->_dialogsManager.showOptionsDialog();
		_vm->_graphicsManager.no_scroll = 0;
		break;
	case KEY_LOAD:
		_vm->_graphicsManager.no_scroll = 1;
		_vm->_dialogsManager.showLoadGame();
		_vm->_graphicsManager.no_scroll = 0;
		break;
	case KEY_SAVE:
		_vm->_graphicsManager.no_scroll = 1;
		_vm->_dialogsManager.showSaveGame();
		_vm->_graphicsManager.no_scroll = 0;
		break;
	default:
		break;
	}

	_inventFl = false;
	_vm->_eventsManager._gameKey = KEY_NONE;
}

/**
 * Load Game dialog
 */
void DialogsManager::showLoadGame() {
	int slotNumber;

	_vm->_eventsManager.VBL();
	showSaveLoad(2);
	do {
		slotNumber = searchSavegames();
		_vm->_eventsManager.VBL();
	} while (!_vm->shouldQuit() && (!slotNumber || _vm->_eventsManager.getMouseButton() != 1));
	_vm->_objectsManager._saveLoadFl = false;
	_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x + 183, 60, 274, 353, _vm->_graphicsManager._vesaBuffer, _vm->_eventsManager._startPos.x + 183, 60);
	_vm->_graphicsManager.addVesaSegment(_vm->_eventsManager._startPos.x + 183, 60, 457, 413);
	_vm->_objectsManager.BOBTOUS = true;
	_vm->_objectsManager.SL_SPR = _vm->_globals.freeMemory(_vm->_objectsManager.SL_SPR);
	_vm->_objectsManager.SL_SPR2 = _vm->_globals.freeMemory(_vm->_objectsManager.SL_SPR2);
	_vm->_objectsManager._saveLoadX = 0;
	_vm->_objectsManager._saveLoadY = 0;

	if (slotNumber != 7) {
		_vm->_saveLoadManager.loadGame(slotNumber);
	}

	_vm->_objectsManager.changeObject(14);
}

/**
 * Save Game dialog
 */
void DialogsManager::showSaveGame() {
	int slotNumber;
	Common::String saveName;

	_vm->_eventsManager.VBL();

	showSaveLoad(1);
	do {
		slotNumber = searchSavegames();
		_vm->_eventsManager.VBL();
	} while (!_vm->shouldQuit() && (!slotNumber || _vm->_eventsManager.getMouseButton() != 1));

	_vm->_objectsManager._saveLoadFl = false;
	_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x + 183, 60, 274, 353, _vm->_graphicsManager._vesaBuffer, _vm->_eventsManager._startPos.x + 183, 60);
	_vm->_graphicsManager.addVesaSegment(_vm->_eventsManager._startPos.x + 183, 60, _vm->_eventsManager._startPos.x + 457, 413);
	_vm->_objectsManager.BOBTOUS = true;
	_vm->_objectsManager.SL_SPR = _vm->_globals.freeMemory(_vm->_objectsManager.SL_SPR);
	_vm->_objectsManager.SL_SPR2 = _vm->_globals.freeMemory(_vm->_objectsManager.SL_SPR2);
	_vm->_objectsManager._saveLoadX = 0;
	_vm->_objectsManager._saveLoadY = 0;

	if (slotNumber != 7) {
		// Since the original GUI doesn't support save names, use a default name
		saveName = Common::String::format("Save #%d", slotNumber);

		// Save the game
		_vm->_saveLoadManager.saveGame(slotNumber, saveName);
	}
}

/**
 * Load/Save dialog
 */
void DialogsManager::showSaveLoad(int a1) {
	int slotNumber;
	hopkinsSavegameHeader header;
	byte *thumb;

	switch (_vm->_globals._language) {
	case LANG_EN:
		_vm->_fileManager.constructFilename("SYSTEM", "SAVEAN.SPR");
		break;
	case LANG_FR:
		_vm->_fileManager.constructFilename("SYSTEM", "SAVEFR.SPR");
		break;
	case LANG_SP:
		_vm->_fileManager.constructFilename("SYSTEM", "SAVEES.SPR");
		break;
	}

	_vm->_objectsManager.SL_SPR = _vm->_objectsManager.loadSprite(_vm->_globals._curFilename);
	_vm->_fileManager.constructFilename("SYSTEM", "SAVE2.SPR");
	_vm->_objectsManager.SL_SPR2 = _vm->_objectsManager.loadSprite(_vm->_globals._curFilename);
	_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager.SL_SPR, _vm->_eventsManager._startPos.x + 483, 360, 0);

	if (_vm->_globals._language == LANG_FR) {
		if (a1 == 1)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager.SL_SPR, _vm->_eventsManager._startPos.x + 525, 375, 1);
		if (a1 == 2)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager.SL_SPR, _vm->_eventsManager._startPos.x + 515, 375, 2);
	} else {
		if (a1 == 1)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager.SL_SPR, _vm->_eventsManager._startPos.x + 535, 372, 1);
		if (a1 == 2)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager.SL_SPR, _vm->_eventsManager._startPos.x + 539, 372, 2);
	}

	for (slotNumber = 1; slotNumber <= 6; ++slotNumber) {
		if (_vm->_saveLoadManager.readSavegameHeader(slotNumber, header)) {
			Graphics::Surface thumb8;
			_vm->_saveLoadManager.convertThumb16To8(header._thumbnail, &thumb8);

			thumb = (byte *)thumb8.pixels;

			switch (slotNumber) {
			case 1:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 190, 112, 128, 87);
				break;
			case 2:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 323, 112, 128, 87);
				break;
			case 3:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 190, 203, 128, 87);
				break;
			case 4:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 323, 203, 128, 87);
				break;
			case 5:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 190, 294, 128, 87);
				break;
			case 6:
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 323, 294, 128, 87);
				break;
			}

			thumb8.free();
			header._thumbnail->free();
			delete header._thumbnail;
		}
	}

	_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager.SL_SPR, _vm->_eventsManager._startPos.x + 183, 60, 274, 353);
	_vm->_objectsManager._saveLoadFl = true;
	_vm->_objectsManager.SL_MODE = a1;
	_vm->_objectsManager._saveLoadX = 0;
	_vm->_objectsManager._saveLoadY = 0;
}

/**
 * Search savegames
 */
int DialogsManager::searchSavegames() {
	int slotNumber = 0;
	int xp = _vm->_eventsManager.getMouseX();
	int yp = _vm->_eventsManager.getMouseY();

	_vm->_graphicsManager._scrollOffset = _vm->_eventsManager._startPos.x;
	if ((uint16)(yp - 112) <= 86) {
		if (xp > _vm->_eventsManager._startPos.x + 189 && xp < _vm->_eventsManager._startPos.x + 318)
			slotNumber = 1;
		if ((uint16)(yp - 112) <= 86 && xp > _vm->_graphicsManager._scrollOffset + 322 && xp < _vm->_graphicsManager._scrollOffset + 452)
			slotNumber = 2;
	}
	if ((uint16)(yp - 203) <= 86) {
		if (xp > _vm->_graphicsManager._scrollOffset + 189 && xp < _vm->_graphicsManager._scrollOffset + 318)
			slotNumber = 3;
		if ((uint16)(yp - 203) <= 86 && xp > _vm->_graphicsManager._scrollOffset + 322 && xp < _vm->_graphicsManager._scrollOffset + 452)
			slotNumber = 4;
	}
	if ((uint16)(yp - 294) <= 86) {
		if (xp > _vm->_graphicsManager._scrollOffset + 189 && xp < _vm->_graphicsManager._scrollOffset + 318)
			slotNumber = 5;
		if ((uint16)(yp - 294) <= 86 && xp > _vm->_graphicsManager._scrollOffset + 322 && xp < _vm->_graphicsManager._scrollOffset + 452)
			slotNumber = 6;
	}
	if ((uint16)(yp - 388) <= 16 && xp > _vm->_graphicsManager._scrollOffset + 273 && xp < _vm->_graphicsManager._scrollOffset + 355)
		slotNumber = 7;

	switch (slotNumber) {
	case 1:
		_vm->_objectsManager._saveLoadX = 189;
		_vm->_objectsManager._saveLoadY = 111;
		break;
	case 2:
		_vm->_objectsManager._saveLoadX = 322;
		_vm->_objectsManager._saveLoadY = 111;
		break;
	case 3:
		_vm->_objectsManager._saveLoadX = 189;
		_vm->_objectsManager._saveLoadY = 202;
		break;
	case 4:
		_vm->_objectsManager._saveLoadX = 322;
		_vm->_objectsManager._saveLoadY = 202;
		break;
	case 5:
		_vm->_objectsManager._saveLoadX = 189;
		_vm->_objectsManager._saveLoadY = 293;
		break;
	case 6:
		_vm->_objectsManager._saveLoadX = 322;
		_vm->_objectsManager._saveLoadY = 293;
		break;
	case 0:
	case 7:
		_vm->_objectsManager._saveLoadX = 0;
		_vm->_objectsManager._saveLoadY = 0;
		break;
	}
	return slotNumber;
}

} // End of namespace Hopkins
