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

#include "hopkins/dialogs.h"

#include "hopkins/events.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/sound.h"

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/util.h"

namespace Hopkins {

DialogsManager::DialogsManager() {
	_inventFl = false;
	_inventDisplayedFl = false;
	_removeInventFl = false;
	_inventX = _inventY = 0;
	_inventWidth = _inventHeight = 0;
	_inventWin1 = g_PTRNUL;
	_inventBuf2 = g_PTRNUL;
	_inventoryIcons = g_PTRNUL;
}

DialogsManager::~DialogsManager() {
	_vm->_globals.freeMemory(_inventWin1);
	_vm->_globals.freeMemory(_inventoryIcons);
}

void DialogsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void DialogsManager::showOptionsDialog() {
	_vm->_eventsManager.changeMouseCursor(0);
	_vm->_eventsManager.refreshScreenAndEvents();
	Common::String filename;
	if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
		filename = "OPTION.SPR";
	else {
		if (_vm->_globals._language == LANG_FR)
			filename = "OPTIFR.SPR";
		else if (_vm->_globals._language == LANG_EN)
			filename = "OPTIAN.SPR";
		else if (_vm->_globals._language == LANG_SP)
			filename = "OPTIES.SPR";
	}

	_vm->_globals._optionDialogSpr = _vm->_fileManager.loadFile(filename);
	_vm->_globals._optionDialogFl = true;

	int scrollOffset = _vm->_graphicsManager._scrollOffset;
	bool doneFlag = false;
	do {
		if (_vm->_eventsManager.getMouseButton()) {
			Common::Point mousePos(_vm->_eventsManager.getMouseX(), _vm->_eventsManager.getMouseY());
			mousePos.x = _vm->_eventsManager.getMouseX();
			mousePos.y = _vm->_eventsManager.getMouseY();

			if (!_vm->_soundManager._musicOffFl) {
				if (mousePos.x >= scrollOffset + 300 && mousePos.y > 113 && mousePos.x <= scrollOffset + 327 && mousePos.y <= 138) {
					// Change the music volume
					++_vm->_soundManager._musicVolume;

					if (_vm->_soundManager._musicVolume <= 12)
						_vm->_soundManager.playSoundFile("bruit2.wav");
					else
						_vm->_soundManager._musicVolume = 12;
					_vm->_soundManager.setMODMusicVolume(_vm->_soundManager._musicVolume);

					_vm->_soundManager.updateScummVMSoundSettings();
				}

				if (!_vm->_soundManager._musicOffFl && mousePos.x >= scrollOffset + 331 && mousePos.y > 113 && mousePos.x <= scrollOffset + 358 && mousePos.y <= 138) {
					--_vm->_soundManager._musicVolume;
					if (_vm->_soundManager._musicVolume >= 0)
						_vm->_soundManager.playSoundFile("bruit2.wav");
					else
						_vm->_soundManager._musicVolume = 0;

					_vm->_soundManager.setMODMusicVolume(_vm->_soundManager._musicVolume);

					_vm->_soundManager.updateScummVMSoundSettings();
				}
			}
			if (!_vm->_soundManager._soundOffFl) {
				// increase volume
				if (mousePos.x >= scrollOffset + 300 && mousePos.y > 140 && mousePos.x <= scrollOffset + 327 && mousePos.y <= 165) {
					++_vm->_soundManager._soundVolume;
					if (_vm->_soundManager._soundVolume <= 16)
						_vm->_soundManager.playSoundFile("bruit2.wav");
					else
						_vm->_soundManager._soundVolume = 16;
					_vm->_soundManager.setMODSampleVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}

				// Decrease volume
				if (!_vm->_soundManager._soundOffFl && mousePos.x >= scrollOffset + 331 && mousePos.y > 140 && mousePos.x <= scrollOffset + 358 && mousePos.y <= 165) {
					--_vm->_soundManager._soundVolume;
					if (_vm->_soundManager._soundVolume >= 0)
						_vm->_soundManager.playSoundFile("bruit2.wav");
					else
						_vm->_soundManager._soundVolume = 0;
					_vm->_soundManager.setMODSampleVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}
			}

			if (!_vm->_soundManager._voiceOffFl) {
				if (mousePos.x >= scrollOffset + 300 && mousePos.y > 167 && mousePos.x <= scrollOffset + 327 && mousePos.y <= 192) {
					++_vm->_soundManager._voiceVolume;

					if (_vm->_soundManager._voiceVolume <= 16)
						_vm->_soundManager.playSoundFile("bruit2.wav");
					else
						_vm->_soundManager._voiceVolume = 16;
					_vm->_soundManager.setMODVoiceVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}

				if (!_vm->_soundManager._voiceOffFl && mousePos.x >= scrollOffset + 331 && mousePos.y > 167 && mousePos.x <= scrollOffset + 358 && mousePos.y <= 192) {
					--_vm->_soundManager._voiceVolume;
					if (_vm->_soundManager._voiceVolume >= 0)
						_vm->_soundManager.playSoundFile("bruit2.wav");
					else
						_vm->_soundManager._voiceVolume = 0;
					_vm->_soundManager.setMODVoiceVolume();

					_vm->_soundManager.updateScummVMSoundSettings();
				}
			}

			if (mousePos.x >= scrollOffset + 431) {
				if (mousePos.y > 194 && mousePos.x <= scrollOffset + 489 && mousePos.y <= 219)
					_vm->_soundManager._textOffFl = !_vm->_soundManager._textOffFl;

				if (mousePos.x >= scrollOffset + 431) {
					if (mousePos.y > 167 && mousePos.x <= scrollOffset + 489 && mousePos.y <= 192) {
						_vm->_soundManager._voiceOffFl = !_vm->_soundManager._voiceOffFl;

						_vm->_soundManager.updateScummVMSoundSettings();
					}
					if (mousePos.x >= scrollOffset + 431) {
						if (mousePos.y > 113 && mousePos.x <= scrollOffset + 489 && mousePos.y <= 138) {
							if (_vm->_soundManager._musicOffFl) {
								_vm->_soundManager._musicOffFl = false;
								_vm->_soundManager.setMODMusicVolume(_vm->_soundManager._musicVolume);
							} else {
								_vm->_soundManager._musicOffFl = true;
								_vm->_soundManager.setMODMusicVolume(0);
							}

							_vm->_soundManager.updateScummVMSoundSettings();
						}

						if (mousePos.x >= scrollOffset + 431 && mousePos.y > 140 && mousePos.x <= scrollOffset + 489 && mousePos.y <= 165) {
							_vm->_soundManager._soundOffFl = !_vm->_soundManager._soundOffFl;

							_vm->_soundManager.updateScummVMSoundSettings();
						}
					}
				}
			}

			if (mousePos.x >= scrollOffset + 175 && mousePos.y > 285 && mousePos.x <= scrollOffset + 281 && mousePos.y <= 310) {
				_vm->_globals._exitId = 300;
				doneFlag = true;
			}
			if (mousePos.x >= scrollOffset + 355 && mousePos.y > 285 && mousePos.x <= scrollOffset + 490 && mousePos.y <= 310)
				doneFlag = true;
			if (mousePos.x >= scrollOffset + 300 && mousePos.y > 194 && mousePos.x <= scrollOffset + 358 && mousePos.y <= 219) {
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

			if (   mousePos.x < scrollOffset + 165 || mousePos.x > scrollOffset + 496 
				|| mousePos.y < 107 || mousePos.y > 318)
				doneFlag = true;
		}

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

		switch (_vm->_graphicsManager._scrollSpeed) {
		case 1:
			_vm->_globals._menuScrollSpeed = 12;
			break;
		case 2:
			_vm->_globals._menuScrollSpeed = 13;
			break;
		case 4:
			_vm->_globals._menuScrollSpeed = 14;
			break;
		case 8:
			_vm->_globals._menuScrollSpeed = 15;
			break;
		case 16:
			_vm->_globals._menuScrollSpeed = 16;
			break;
		case 32:
			_vm->_globals._menuScrollSpeed = 17;
			break;
		case 48:
			_vm->_globals._menuScrollSpeed = 18;
			break;
		case 64:
			_vm->_globals._menuScrollSpeed = 19;
			break;
		case 128:
			_vm->_globals._menuScrollSpeed = 20;
			break;
		case 160:
			_vm->_globals._menuScrollSpeed = 21;
			break;
		case 320:
			_vm->_globals._menuScrollSpeed = 22;
			break;
		case 640:
			_vm->_globals._menuScrollSpeed = 23;
			break;
		}

		_vm->_eventsManager.refreshScreenAndEvents();
	} while (!doneFlag);

	_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen, scrollOffset + 164,
		107, 335, 215, _vm->_graphicsManager._vesaBuffer, scrollOffset + 164, 107);
	_vm->_graphicsManager.addVesaSegment(scrollOffset + 164, 107,
		scrollOffset + 498, 320);

	_vm->_globals._optionDialogSpr = _vm->_globals.freeMemory(_vm->_globals._optionDialogSpr);
	_vm->_globals._optionDialogFl = false;
}

void DialogsManager::showInventory() {
	if (_removeInventFl || _inventDisplayedFl || _vm->_globals._disableInventFl)
		return;

	_vm->_graphicsManager._scrollStatus = 1;
	_vm->_objectsManager._eraseVisibleCounter = 4;
	_vm->_objectsManager._visibleFl = false;
	for (int i = 0; i <= 1; i++) {
		inventAnim();
		_vm->_eventsManager.getMouseX();
		_vm->_eventsManager.getMouseY();
		_vm->_eventsManager.refreshScreenAndEvents();
	}
	_inventWin1 = g_PTRNUL;

	bool loopFl;
	do {
		loopFl = false;
		_vm->_eventsManager._curMouseButton = 0;
		_vm->_eventsManager._mouseButton = 0;
		_vm->_globals._disableInventFl = true;
		_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);

		Common::String filename;
		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			filename = "INVENT.SPR";
		else {
			switch (_vm->_globals._language) {
			case LANG_EN:
				filename = "INVENTAN.SPR";
				break;
			case LANG_FR:
				filename = "INVENTFR.SPR";
				break;
			case LANG_SP:
				filename = "INVENTES.SPR";
				break;
			}
		}

		Common::File f;
		if (!f.open(filename))
			error("Error opening file - %s", filename.c_str());

		size_t filesize = f.size();
		_inventWin1 = _vm->_globals.allocMemory(filesize);
		_vm->_fileManager.readStream(f, _inventWin1, filesize);
		f.close();

		_inventBuf2 = _vm->_fileManager.loadFile("INVENT2.SPR");

		_inventX = _vm->_graphicsManager._scrollOffset + 152;
		_inventY = 114;
		_inventWidth = _vm->_objectsManager.getWidth(_inventWin1, 0);
		_inventHeight = _vm->_objectsManager.getHeight(_inventWin1, 0);

		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _inventWin1, _inventX + 300, 414, 0, 0, 0, false);
		int curPosY = 0;
		int inventCount = 0;
		for (int inventLine = 1; inventLine <= 5; inventLine++) {
			int curPosX = 0;
			for (int inventCol = 1; inventCol <= 6; inventCol++) {
				++inventCount;
				int inventIdx = _vm->_globals._inventory[inventCount];
				// The last two zones are not reserved for the inventory: Options and Save/Load
				if (inventIdx && inventCount <= 29) {
					byte *obj = _vm->_objectsManager.loadObjectFromFile(inventIdx, false);
					_vm->_graphicsManager.restoreSurfaceRect(_vm->_graphicsManager._vesaBuffer, obj, _inventX + curPosX + 6,
						curPosY + 120, _vm->_globals._objectWidth, _vm->_globals._objectHeight);
					_vm->_globals.freeMemory(obj);
				}
				curPosX += 54;
			};
			curPosY += 38;
		}
		_vm->_graphicsManager.copySurfaceRect(_vm->_graphicsManager._vesaBuffer, _inventWin1, _inventX, _inventY, _inventWidth, _inventHeight);
		_vm->_eventsManager._curMouseButton = 0;
		int newInventoryItem = 0;

		// Main loop to select an inventory item
		while (!_vm->shouldQuit()) {
			// Turn on drawing the inventory dialog in the event manager
			_inventDisplayedFl = true;

			int mousePosX = _vm->_eventsManager.getMouseX();
			int mousePosY = _vm->_eventsManager.getMouseY();
			int mouseButton = _vm->_eventsManager.getMouseButton();
			int oldInventoryItem = newInventoryItem;
			newInventoryItem = _vm->_linesManager.checkInventoryHotspots(mousePosX, mousePosY);
			if (newInventoryItem != oldInventoryItem)
				_vm->_objectsManager.initBorder(newInventoryItem);
			if (_vm->_eventsManager._mouseCursorId != 1 && _vm->_eventsManager._mouseCursorId != 2 && _vm->_eventsManager._mouseCursorId != 3 && _vm->_eventsManager._mouseCursorId != 16) {
				if (mouseButton == 2) {
					_vm->_objectsManager.nextObjectIcon(newInventoryItem);
					if (_vm->_eventsManager._mouseCursorId != 23)
						_vm->_eventsManager.changeMouseCursor(_vm->_eventsManager._mouseCursorId);
				}
			}
			if (mouseButton == 1) {
				if (_vm->_eventsManager._mouseCursorId == 1 || _vm->_eventsManager._mouseCursorId == 2 || _vm->_eventsManager._mouseCursorId == 3 || _vm->_eventsManager._mouseCursorId == 16 || !_vm->_eventsManager._mouseCursorId)
					break;
				_vm->_objectsManager.takeInventoryObject(_vm->_globals._inventory[newInventoryItem]);
				if (_vm->_eventsManager._mouseCursorId == 8)
					break;

				_vm->_scriptManager._tempObjectFl = true;
				_vm->_globals._saveData->_data[svLastObjectIndex] = _vm->_objectsManager._curObjectIndex;
				_vm->_globals._saveData->_data[svLastInventoryItem] = _vm->_globals._inventory[newInventoryItem];
				_vm->_globals._saveData->_data[svLastInvMouseCursor] = _vm->_eventsManager._mouseCursorId;
				_vm->_objectsManager.OPTI_OBJET();
				_vm->_scriptManager._tempObjectFl = false;

				if (_vm->_soundManager._voiceOffFl) {
					do
						_vm->_eventsManager.refreshScreenAndEvents();
					while (!_vm->_globals._exitId && _vm->_eventsManager.getMouseButton() != 1);
					_vm->_fontManager.hideText(9);
				}
				if (_vm->_globals._exitId) {
					if (_vm->_globals._exitId == 2) {
						_vm->_globals._exitId = 0;
						break;
					}

					_vm->_globals._exitId = 0;
					_inventBuf2 = _vm->_globals.freeMemory(_inventBuf2);
					_inventWin1 = _vm->_globals.freeMemory(_inventWin1);
					loopFl = true;
					break;
				} else
					_inventDisplayedFl = true;
			}
			if (_removeInventFl)
				break;
			_vm->_eventsManager.refreshScreenAndEvents();
			if (_vm->_globals._screenId >= 35 && _vm->_globals._screenId <= 40)
				_vm->_objectsManager.handleSpecialGames();
		}
	} while (loopFl);

	_vm->_fontManager.hideText(9);
	if (_inventDisplayedFl) {
		_inventDisplayedFl = false;
		_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen, _inventX, 114, _inventWidth, _inventHeight, _vm->_graphicsManager._vesaBuffer, _inventX, 114);
		_vm->_graphicsManager.addVesaSegment(_inventX, 114, _inventX + _inventWidth, _inventWidth + 114);
		_vm->_objectsManager.BOBTOUS = true;
	}

	_inventWin1 = _vm->_globals.freeMemory(_inventWin1);
	_inventBuf2 = _vm->_globals.freeMemory(_inventBuf2);

	if (_vm->_eventsManager._mouseCursorId == 1)
		showOptionsDialog();
	else if (_vm->_eventsManager._mouseCursorId == 3)
		showLoadGame();
	else if (_vm->_eventsManager._mouseCursorId == 2)
		showSaveGame();

	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(4);
	_vm->_objectsManager._oldBorderPos = Common::Point(0, 0);
	_vm->_objectsManager._borderPos = Common::Point(0, 0);
	_vm->_globals._disableInventFl = false;
	_vm->_graphicsManager._scrollStatus = 0;
}

/**
 * Inventory Animations
 */
void DialogsManager::inventAnim() {
	if (_vm->_globals._disableInventFl)
		return;

	if (_vm->_objectsManager._eraseVisibleCounter && !_vm->_objectsManager._visibleFl) {
		_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen, _vm->_objectsManager._oldInventoryPosX, 27, 48, 38,
			_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._oldInventoryPosX, 27);
		_vm->_graphicsManager.addVesaSegment(_vm->_objectsManager._oldInventoryPosX, 27, _vm->_objectsManager._oldInventoryPosX + 48, 65);
		--_vm->_objectsManager._eraseVisibleCounter;
	}

	if (_vm->_objectsManager._visibleFl) {
		if (_vm->_objectsManager._oldInventoryPosX <= 1)
			_vm->_objectsManager._oldInventoryPosX = 2;
		_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen, _vm->_objectsManager._oldInventoryPosX, 27, 48, 38,
			_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._oldInventoryPosX, 27);

		_vm->_graphicsManager.addVesaSegment(_vm->_objectsManager._oldInventoryPosX, 27, _vm->_objectsManager._oldInventoryPosX + 48, 65);
		int newOffset = _vm->_graphicsManager._scrollOffset + 2;
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _inventoryIcons, newOffset + 300, 327, 0);
		_vm->_graphicsManager.addVesaSegment(newOffset, 27, newOffset + 45, 62);
		_vm->_objectsManager._oldInventoryPosX = newOffset;
	}

	if (_vm->_globals._saveData->_data[svField357] == 1) {
		if (_vm->_globals._saveData->_data[svField353] == 1)
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._headSprites, 832, 325, 0, 0, 0, false);
		if (_vm->_globals._saveData->_data[svField355] == 1)
			_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._headSprites, 866, 325, 1, 0, 0, false);
		_vm->_graphicsManager.addVesaSegment(532, 25, 560, 60);
		_vm->_graphicsManager.addVesaSegment(566, 25, 594, 60);
	}
	if (_vm->_globals._saveData->_data[svField356] == 1) {
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._headSprites, 832, 325, 0, 0, 0, false);
		_vm->_graphicsManager.addVesaSegment(532, 25, 560, 60);
	}

	if (_vm->_globals._saveData->_data[svField354] == 1) {
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._headSprites, 832, 325, 0, 0, 0, false);
		_vm->_graphicsManager.addVesaSegment(532, 25, 560, 60);
	}
}

/**
 * Test dialog opening
 */
void DialogsManager::testDialogOpening() {
	if (_vm->_globals._cityMapEnabledFl)
		_vm->_eventsManager._gameKey = KEY_NONE;

	if ((_vm->_eventsManager._gameKey == KEY_NONE) || _inventFl)
		return;

	DIALOG_KEY key = _vm->_eventsManager._gameKey;
	_vm->_eventsManager._gameKey = KEY_NONE;
	_inventFl = true;

	switch (key) {
	case KEY_INVENTORY:
		showInventory();
		break;
	case KEY_OPTIONS:
		_vm->_graphicsManager._scrollStatus = 1;
		showOptionsDialog();
		_vm->_graphicsManager._scrollStatus = 0;
		break;
	case KEY_LOAD:
		_vm->_graphicsManager._scrollStatus = 1;
		showLoadGame();
		_vm->_graphicsManager._scrollStatus = 0;
		break;
	case KEY_SAVE:
		_vm->_graphicsManager._scrollStatus = 1;
		showSaveGame();
		_vm->_graphicsManager._scrollStatus = 0;
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
	_vm->_eventsManager.refreshScreenAndEvents();
	showSaveLoad(MODE_LOAD);

	int slotNumber;
	do {
		slotNumber = searchSavegames();
		_vm->_eventsManager.refreshScreenAndEvents();
	} while (!_vm->shouldQuit() && (!slotNumber || _vm->_eventsManager.getMouseButton() != 1));
	_vm->_objectsManager._saveLoadFl = false;
	_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x + 183, 60, 274, 353, _vm->_graphicsManager._vesaBuffer, _vm->_eventsManager._startPos.x + 183, 60);
	_vm->_graphicsManager.addVesaSegment(_vm->_eventsManager._startPos.x + 183, 60, 457, 413);
	_vm->_objectsManager.BOBTOUS = true;
	_vm->_objectsManager._saveLoadSprite = _vm->_globals.freeMemory(_vm->_objectsManager._saveLoadSprite);
	_vm->_objectsManager._saveLoadSprite2 = _vm->_globals.freeMemory(_vm->_objectsManager._saveLoadSprite2);
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
	_vm->_eventsManager.refreshScreenAndEvents();

	showSaveLoad(MODE_SAVE);
	int slotNumber;
	do {
		slotNumber = searchSavegames();
		_vm->_eventsManager.refreshScreenAndEvents();
	} while (!_vm->shouldQuit() && (!slotNumber || _vm->_eventsManager.getMouseButton() != 1));

	_vm->_objectsManager._saveLoadFl = false;
	_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x + 183, 60, 274, 353, _vm->_graphicsManager._vesaBuffer, _vm->_eventsManager._startPos.x + 183, 60);
	_vm->_graphicsManager.addVesaSegment(_vm->_eventsManager._startPos.x + 183, 60, _vm->_eventsManager._startPos.x + 457, 413);
	_vm->_objectsManager.BOBTOUS = true;
	_vm->_objectsManager._saveLoadSprite = _vm->_globals.freeMemory(_vm->_objectsManager._saveLoadSprite);
	_vm->_objectsManager._saveLoadSprite2 = _vm->_globals.freeMemory(_vm->_objectsManager._saveLoadSprite2);
	_vm->_objectsManager._saveLoadX = 0;
	_vm->_objectsManager._saveLoadY = 0;

	if (slotNumber != 7) {
		// Since the original GUI doesn't support save names, use a default name
		Common::String saveName = Common::String::format("Save #%d", slotNumber);

		// Save the game
		_vm->_saveLoadManager.saveGame(slotNumber, saveName);
	}
}

/**
 * Load/Save dialog
 */
void DialogsManager::showSaveLoad(SaveLoadMode mode) {
	Common::String filename;

	if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
		filename = "SAVE.SPR";
	else {
		switch (_vm->_globals._language) {
		case LANG_EN:
			filename = "SAVEAN.SPR";
			break;
		case LANG_FR:
			filename = "SAVEFR.SPR";
			break;
		case LANG_SP:
			filename = "SAVEES.SPR";
			break;
		}
	}

	_vm->_objectsManager._saveLoadSprite = _vm->_objectsManager.loadSprite(filename);
	_vm->_objectsManager._saveLoadSprite2 = _vm->_objectsManager.loadSprite("SAVE2.SPR");
	_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._saveLoadSprite, _vm->_eventsManager._startPos.x + 483, 360, 0);

	if (_vm->_globals._language == LANG_FR) {
		if (mode == MODE_SAVE)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._saveLoadSprite, _vm->_eventsManager._startPos.x + 525, 375, 1);
		else if (mode == MODE_LOAD)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._saveLoadSprite, _vm->_eventsManager._startPos.x + 515, 375, 2);
	} else {
		if (mode == MODE_SAVE)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._saveLoadSprite, _vm->_eventsManager._startPos.x + 535, 372, 1);
		else if (mode == MODE_LOAD)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._saveLoadSprite, _vm->_eventsManager._startPos.x + 539, 372, 2);
	}

	for (int slotNumber = 1; slotNumber <= 6; ++slotNumber) {
		hopkinsSavegameHeader header;
		if (_vm->_saveLoadManager.readSavegameHeader(slotNumber, header)) {
			Graphics::Surface thumb8;
			_vm->_saveLoadManager.convertThumb16To8(header._thumbnail, &thumb8);

			byte *thumb = (byte *)thumb8.pixels;

			switch (slotNumber) {
			case 1:
				_vm->_graphicsManager.restoreSurfaceRect(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 190, 112, 128, 87);
				break;
			case 2:
				_vm->_graphicsManager.restoreSurfaceRect(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 323, 112, 128, 87);
				break;
			case 3:
				_vm->_graphicsManager.restoreSurfaceRect(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 190, 203, 128, 87);
				break;
			case 4:
				_vm->_graphicsManager.restoreSurfaceRect(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 323, 203, 128, 87);
				break;
			case 5:
				_vm->_graphicsManager.restoreSurfaceRect(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 190, 294, 128, 87);
				break;
			case 6:
				_vm->_graphicsManager.restoreSurfaceRect(_vm->_graphicsManager._vesaBuffer, thumb, _vm->_eventsManager._startPos.x + 323, 294, 128, 87);
				break;
			}

			thumb8.free();
			header._thumbnail->free();
			delete header._thumbnail;
		}
	}

	_vm->_graphicsManager.copySurfaceRect(_vm->_graphicsManager._vesaBuffer, _vm->_objectsManager._saveLoadSprite, _vm->_eventsManager._startPos.x + 183, 60, 274, 353);
	_vm->_objectsManager._saveLoadFl = true;
	_vm->_objectsManager._saveLoadX = 0;
	_vm->_objectsManager._saveLoadY = 0;
}

/**
 * Search savegames
 */
int DialogsManager::searchSavegames() {
	int xp = _vm->_eventsManager.getMouseX();
	int yp = _vm->_eventsManager.getMouseY();

	_vm->_graphicsManager._scrollOffset = _vm->_eventsManager._startPos.x;

	int slotNumber = 0;
	if (yp >= 112 && yp <= 198) {
		if (xp > _vm->_eventsManager._startPos.x + 189 && xp < _vm->_eventsManager._startPos.x + 318) {
			slotNumber = 1;
			_vm->_objectsManager._saveLoadX = 189;
			_vm->_objectsManager._saveLoadY = 111;
		} else if (xp > _vm->_graphicsManager._scrollOffset + 322 && xp < _vm->_graphicsManager._scrollOffset + 452) {
			slotNumber = 2;
			_vm->_objectsManager._saveLoadX = 322;
			_vm->_objectsManager._saveLoadY = 111;
		}
	} else if (yp >= 203 && yp <= 289) {
		if (xp > _vm->_graphicsManager._scrollOffset + 189 && xp < _vm->_graphicsManager._scrollOffset + 318) {
			slotNumber = 3;
			_vm->_objectsManager._saveLoadX = 189;
			_vm->_objectsManager._saveLoadY = 202;
		} else if (xp > _vm->_graphicsManager._scrollOffset + 322 && xp < _vm->_graphicsManager._scrollOffset + 452) {
			slotNumber = 4;
			_vm->_objectsManager._saveLoadX = 322;
			_vm->_objectsManager._saveLoadY = 202;
		}
	} else if (yp >= 294 && yp <= 380) {
		if (xp > _vm->_graphicsManager._scrollOffset + 189 && xp < _vm->_graphicsManager._scrollOffset + 318) {
			slotNumber = 5;
			_vm->_objectsManager._saveLoadX = 189;
			_vm->_objectsManager._saveLoadY = 293;
		} else if (xp > _vm->_graphicsManager._scrollOffset + 322 && xp < _vm->_graphicsManager._scrollOffset + 452) {
			slotNumber = 6;
			_vm->_objectsManager._saveLoadX = 322;
			_vm->_objectsManager._saveLoadY = 293;
		}
	} else if (yp >= 388 && yp <= 404 && xp > _vm->_graphicsManager._scrollOffset + 273 && xp < _vm->_graphicsManager._scrollOffset + 355) {
		slotNumber = 7;
		_vm->_objectsManager._saveLoadX = 0;
		_vm->_objectsManager._saveLoadY = 0;
	} else {
		slotNumber = 0;
		_vm->_objectsManager._saveLoadX = 0;
		_vm->_objectsManager._saveLoadY = 0;
	}

	return slotNumber;
}

} // End of namespace Hopkins
