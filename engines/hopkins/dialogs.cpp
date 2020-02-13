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

DialogsManager::DialogsManager(HopkinsEngine *vm) {
	_vm = vm;
	_inventFl = false;
	_inventDisplayedFl = false;
	_removeInventFl = false;
	_inventX = _inventY = 0;
	_oldInventX = 0;
	_inventWidth = _inventHeight = 0;
	_inventWin1 = NULL;
	_inventBuf2 = NULL;
	_inventoryIcons = NULL;
}

DialogsManager::~DialogsManager() {
	_vm->_globals->freeMemory(_inventWin1);
	_vm->_globals->freeMemory(_inventBuf2);
	_vm->_globals->freeMemory(_inventoryIcons);
}

void DialogsManager::clearAll() {
	_inventWin1 = NULL;
	_inventBuf2 = NULL;
}

void DialogsManager::loadIcons() {
	_inventoryIcons = _vm->_fileIO->loadFile("ICONE.SPR");
}

void DialogsManager::drawInvent(Common::Point oldBorder, int oldBorderSpriteIndex, Common::Point newBorder, int newBorderSpriteIndex) {
	if (!_inventDisplayedFl)
		return;

	_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, _inventWin1, _inventX, _inventY, _inventWidth, _inventHeight);
	if (oldBorder.x && oldBorder.y)
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _inventBuf2, oldBorder.x + 300, oldBorder.y + 300, oldBorderSpriteIndex + 1);
	if (newBorder.x && newBorder.y)
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _inventBuf2, newBorder.x + 300, newBorder.y + 300, newBorderSpriteIndex);
	_vm->_graphicsMan->addDirtyRect(_inventX, _inventY, _inventX + _inventWidth, _inventY + _inventHeight);
}

void DialogsManager::showOptionsDialog() {
	_vm->_events->changeMouseCursor(0);
	_vm->_events->refreshScreenAndEvents();
	Common::String filename;
	if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
		filename = "OPTION.SPR";
	else {
		switch (_vm->_globals->_language) {
		case LANG_FR:
			filename = "OPTIFR.SPR";
			break;
		case LANG_EN:
			filename = "OPTIAN.SPR";
			break;
		case LANG_SP:
			filename = "OPTIES.SPR";
			break;
		default:
			break;
		}
	}

	_vm->_globals->_optionDialogSpr = _vm->_fileIO->loadFile(filename);
	_vm->_globals->_optionDialogFl = true;

	int scrollOffset = _vm->_graphicsMan->_scrollOffset;
	bool doneFlag = false;
	do {
		if (_vm->_events->getMouseButton()) {
			Common::Point mousePos(_vm->_events->getMouseX(), _vm->_events->getMouseY());

			if (!_vm->_soundMan->_musicOffFl) {
				if (mousePos.x >= scrollOffset + 300 && mousePos.y > 113 && mousePos.x <= scrollOffset + 327 && mousePos.y <= 138) {
					// Change the music volume
					++_vm->_soundMan->_musicVolume;

					if (_vm->_soundMan->_musicVolume <= 12)
						_vm->_soundMan->playSoundFile("bruit2.wav");
					else
						_vm->_soundMan->_musicVolume = 12;
					_vm->_soundMan->setMODMusicVolume(_vm->_soundMan->_musicVolume);

					_vm->_soundMan->updateScummVMSoundSettings();
				}

				if (!_vm->_soundMan->_musicOffFl && mousePos.x >= scrollOffset + 331 && mousePos.y > 113 && mousePos.x <= scrollOffset + 358 && mousePos.y <= 138) {
					--_vm->_soundMan->_musicVolume;
					if (_vm->_soundMan->_musicVolume >= 0)
						_vm->_soundMan->playSoundFile("bruit2.wav");
					else
						_vm->_soundMan->_musicVolume = 0;

					_vm->_soundMan->setMODMusicVolume(_vm->_soundMan->_musicVolume);

					_vm->_soundMan->updateScummVMSoundSettings();
				}
			}
			if (!_vm->_soundMan->_soundOffFl) {
				// increase volume
				if (mousePos.x >= scrollOffset + 300 && mousePos.y > 140 && mousePos.x <= scrollOffset + 327 && mousePos.y <= 165) {
					++_vm->_soundMan->_soundVolume;
					if (_vm->_soundMan->_soundVolume <= 16)
						_vm->_soundMan->playSoundFile("bruit2.wav");
					else
						_vm->_soundMan->_soundVolume = 16;
					_vm->_soundMan->setMODSampleVolume();

					_vm->_soundMan->updateScummVMSoundSettings();
				}

				// Decrease volume
				if (!_vm->_soundMan->_soundOffFl && mousePos.x >= scrollOffset + 331 && mousePos.y > 140 && mousePos.x <= scrollOffset + 358 && mousePos.y <= 165) {
					--_vm->_soundMan->_soundVolume;
					if (_vm->_soundMan->_soundVolume >= 0)
						_vm->_soundMan->playSoundFile("bruit2.wav");
					else
						_vm->_soundMan->_soundVolume = 0;
					_vm->_soundMan->setMODSampleVolume();

					_vm->_soundMan->updateScummVMSoundSettings();
				}
			}

			if (!_vm->_soundMan->_voiceOffFl) {
				if (mousePos.x >= scrollOffset + 300 && mousePos.y > 167 && mousePos.x <= scrollOffset + 327 && mousePos.y <= 192) {
					++_vm->_soundMan->_voiceVolume;

					if (_vm->_soundMan->_voiceVolume <= 16)
						_vm->_soundMan->playSoundFile("bruit2.wav");
					else
						_vm->_soundMan->_voiceVolume = 16;
					_vm->_soundMan->setMODVoiceVolume();

					_vm->_soundMan->updateScummVMSoundSettings();
				}

				if (!_vm->_soundMan->_voiceOffFl && mousePos.x >= scrollOffset + 331 && mousePos.y > 167 && mousePos.x <= scrollOffset + 358 && mousePos.y <= 192) {
					--_vm->_soundMan->_voiceVolume;
					if (_vm->_soundMan->_voiceVolume >= 0)
						_vm->_soundMan->playSoundFile("bruit2.wav");
					else
						_vm->_soundMan->_voiceVolume = 0;
					_vm->_soundMan->setMODVoiceVolume();

					_vm->_soundMan->updateScummVMSoundSettings();
				}
			}

			if (mousePos.x >= scrollOffset + 431) {
				if (mousePos.y > 194 && mousePos.x <= scrollOffset + 489 && mousePos.y <= 219)
					_vm->_soundMan->_textOffFl = !_vm->_soundMan->_textOffFl;

				if (mousePos.x >= scrollOffset + 431) {
					if (mousePos.y > 167 && mousePos.x <= scrollOffset + 489 && mousePos.y <= 192) {
						_vm->_soundMan->_voiceOffFl = !_vm->_soundMan->_voiceOffFl;

						_vm->_soundMan->updateScummVMSoundSettings();
					}
					if (mousePos.x >= scrollOffset + 431) {
						if (mousePos.y > 113 && mousePos.x <= scrollOffset + 489 && mousePos.y <= 138) {
							if (_vm->_soundMan->_musicOffFl) {
								_vm->_soundMan->_musicOffFl = false;
								_vm->_soundMan->setMODMusicVolume(_vm->_soundMan->_musicVolume);
							} else {
								_vm->_soundMan->_musicOffFl = true;
								_vm->_soundMan->setMODMusicVolume(0);
							}

							_vm->_soundMan->updateScummVMSoundSettings();
						}

						if (mousePos.x >= scrollOffset + 431 && mousePos.y > 140 && mousePos.x <= scrollOffset + 489 && mousePos.y <= 165) {
							_vm->_soundMan->_soundOffFl = !_vm->_soundMan->_soundOffFl;

							_vm->_soundMan->updateScummVMSoundSettings();
						}
					}
				}
			}

			if (mousePos.x >= scrollOffset + 175 && mousePos.y > 285 && mousePos.x <= scrollOffset + 281 && mousePos.y <= 310) {
				_vm->_globals->_exitId = 300;
				doneFlag = true;
			}
			if (mousePos.x >= scrollOffset + 355 && mousePos.y > 285 && mousePos.x <= scrollOffset + 490 && mousePos.y <= 310)
				doneFlag = true;
			if (mousePos.x >= scrollOffset + 300 && mousePos.y > 194 && mousePos.x <= scrollOffset + 358 && mousePos.y <= 219) {
				switch (_vm->_graphicsMan->_scrollSpeed) {
				case 1:
					_vm->_graphicsMan->_scrollSpeed = 2;
					break;
				case 2:
					_vm->_graphicsMan->_scrollSpeed = 4;
					break;
				case 4:
					_vm->_graphicsMan->_scrollSpeed = 8;
					break;
				case 8:
					_vm->_graphicsMan->_scrollSpeed = 16;
					break;
				case 16:
					_vm->_graphicsMan->_scrollSpeed = 32;
					break;
				case 32:
					_vm->_graphicsMan->_scrollSpeed = 48;
					break;
				case 48:
					_vm->_graphicsMan->_scrollSpeed = 64;
					break;
				case 64:
					_vm->_graphicsMan->_scrollSpeed = 128;
					break;
				case 128:
					_vm->_graphicsMan->_scrollSpeed = 160;
					break;
				case 160:
					_vm->_graphicsMan->_scrollSpeed = 320;
					break;
				case 320:
					_vm->_graphicsMan->_scrollSpeed = 1;
					break;
				default:
					break;
				}
			}

			// Values are blocked, thus handling the zone is useless
			//if (mousePos.x >= _vm->_graphicsManager->ofscroll + 348 && mousePos.y > 248 && mousePos.x <= _vm->_graphicsManager->ofscroll + 394 && mousePos.y <= 273)
			//	_vm->_globals->_speed = 2;

			if (   mousePos.x < scrollOffset + 165 || mousePos.x > scrollOffset + 496
				|| mousePos.y < 107 || mousePos.y > 318)
				doneFlag = true;
		}

		if (_vm->_globals->_speed == 1)
			_vm->_globals->_menuSpeed = 6;
		else if (_vm->_globals->_speed == 2)
			_vm->_globals->_menuSpeed = 5;
		else if (_vm->_globals->_speed == 3)
			_vm->_globals->_menuSpeed = 4;

		_vm->_globals->_menuTextOff = !_vm->_soundMan->_textOffFl ? 7 : 8;
		_vm->_globals->_menuVoiceOff = !_vm->_soundMan->_voiceOffFl ? 7 : 8;
		_vm->_globals->_menuSoundOff = !_vm->_soundMan->_soundOffFl ? 7 : 8;
		_vm->_globals->_menuMusicOff = !_vm->_soundMan->_musicOffFl ? 7 : 8;

		_vm->_globals->_menuDisplayType = 9;

		switch (_vm->_graphicsMan->_scrollSpeed) {
		case 1:
			_vm->_globals->_menuScrollSpeed = 12;
			break;
		case 2:
			_vm->_globals->_menuScrollSpeed = 13;
			break;
		case 4:
			_vm->_globals->_menuScrollSpeed = 14;
			break;
		case 8:
			_vm->_globals->_menuScrollSpeed = 15;
			break;
		case 16:
			_vm->_globals->_menuScrollSpeed = 16;
			break;
		case 32:
			_vm->_globals->_menuScrollSpeed = 17;
			break;
		case 48:
			_vm->_globals->_menuScrollSpeed = 18;
			break;
		case 64:
			_vm->_globals->_menuScrollSpeed = 19;
			break;
		case 128:
			_vm->_globals->_menuScrollSpeed = 20;
			break;
		case 160:
			_vm->_globals->_menuScrollSpeed = 21;
			break;
		case 320:
			_vm->_globals->_menuScrollSpeed = 22;
			break;
		case 640:
			_vm->_globals->_menuScrollSpeed = 23;
			break;
		default:
			break;
		}

		_vm->_events->refreshScreenAndEvents();
	} while (!doneFlag);

	_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, scrollOffset + 164,
		107, 335, 215, _vm->_graphicsMan->_frontBuffer, scrollOffset + 164, 107);
	_vm->_graphicsMan->addDirtyRect(scrollOffset + 164, 107, scrollOffset + 498, 320);

	_vm->_globals->_optionDialogSpr = _vm->_globals->freeMemory(_vm->_globals->_optionDialogSpr);
	_vm->_globals->_optionDialogFl = false;
}

void DialogsManager::showInventory() {
	if (_removeInventFl || _inventDisplayedFl || _vm->_globals->_disableInventFl)
		return;

	_vm->_graphicsMan->_scrollStatus = 1;
	_vm->_objectsMan->_eraseVisibleCounter = 4;
	_vm->_objectsMan->_visibleFl = false;
	for (int i = 0; i <= 1; i++) {
		inventAnim();
		_vm->_events->getMouseX();
		_vm->_events->getMouseY();
		_vm->_events->refreshScreenAndEvents();
	}
	_inventWin1 = NULL;

	bool loopFl;
	do {
		loopFl = false;
		_vm->_events->_curMouseButton = 0;
		_vm->_events->_mouseButton = 0;
		_vm->_globals->_disableInventFl = true;
		_vm->_graphicsMan->setColorPercentage2(251, 100, 100, 100);

		Common::String filename;
		if (_vm->getPlatform() == Common::kPlatformOS2 || _vm->getPlatform() == Common::kPlatformBeOS)
			filename = "INVENT.SPR";
		else {
			switch (_vm->_globals->_language) {
			case LANG_EN:
				filename = "INVENTAN.SPR";
				break;
			case LANG_FR:
				filename = "INVENTFR.SPR";
				break;
			case LANG_SP:
				filename = "INVENTES.SPR";
				break;
			default:
				break;
			}
		}

		Common::File f;
		if (!f.open(filename))
			error("Error opening file - %s", filename.c_str());

		size_t filesize = f.size();
		_inventWin1 = _vm->_globals->allocMemory(filesize);
		_vm->_fileIO->readStream(f, _inventWin1, filesize);
		f.close();

		_inventBuf2 = _vm->_fileIO->loadFile("INVENT2.SPR");

		_inventX = _vm->_graphicsMan->_scrollOffset + 152;
		_inventY = 114;
		_inventWidth = _vm->_objectsMan->getWidth(_inventWin1, 0);
		_inventHeight = _vm->_objectsMan->getHeight(_inventWin1, 0);

		_vm->_graphicsMan->drawCompressedSprite(_vm->_graphicsMan->_frontBuffer, _inventWin1, _inventX + 300, 414, 0, 0, 0, false);
		int curPosY = 0;
		int inventCount = 0;
		for (int inventLine = 1; inventLine <= 5; inventLine++) {
			int curPosX = 0;
			for (int inventCol = 1; inventCol <= 6; inventCol++) {
				++inventCount;
				int inventIdx = _vm->_globals->_inventory[inventCount];
				// The last two zones are not reserved for the inventory: Options and Save/Load
				if (inventIdx && inventCount <= 29) {
					byte *obj = _vm->_objectsMan->loadObjectFromFile(inventIdx, false);
					_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, obj, _inventX + curPosX + 6,
						curPosY + 120, _vm->_objectsMan->getObjectWidth(), _vm->_objectsMan->getObjectHeight());
					_vm->_globals->freeMemory(obj);
				}
				curPosX += 54;
			};
			curPosY += 38;
		}
		_vm->_graphicsMan->copySurfaceRect(_vm->_graphicsMan->_frontBuffer, _inventWin1, _inventX, _inventY, _inventWidth, _inventHeight);
		_vm->_events->_curMouseButton = 0;
		int newInventoryItem = 0;

		// Main loop to select an inventory item
		while (!_vm->shouldQuit()) {
			// Turn on drawing the inventory dialog in the event manager
			_inventDisplayedFl = true;

			int mousePosX = _vm->_events->getMouseX();
			int mousePosY = _vm->_events->getMouseY();
			int mouseButton = _vm->_events->getMouseButton();
			int oldInventoryItem = newInventoryItem;
			newInventoryItem = _vm->_linesMan->checkInventoryHotspots(mousePosX, mousePosY);
			if (newInventoryItem != oldInventoryItem)
				_vm->_objectsMan->initBorder(newInventoryItem);
			int cursorId = _vm->_events->_mouseCursorId;
			if (cursorId != 1 && cursorId != 2 && cursorId != 3 && cursorId != 16) {
				if (mouseButton == 2) {
					_vm->_objectsMan->nextObjectIcon(newInventoryItem);
					cursorId = _vm->_events->_mouseCursorId;
					if (cursorId != 23)
						_vm->_events->changeMouseCursor(cursorId);
				}
			}
			cursorId = _vm->_events->_mouseCursorId;
			if (mouseButton == 1) {
				if (cursorId == 1 || cursorId == 2 || cursorId == 3 || cursorId == 16 || !cursorId)
					break;
				_vm->_objectsMan->takeInventoryObject(_vm->_globals->_inventory[newInventoryItem]);
				if (_vm->_events->_mouseCursorId == 8)
					break;

				_vm->_script->_tempObjectFl = true;
				_vm->_globals->_saveData->_data[svLastObjectIndex] = _vm->_objectsMan->_curObjectIndex;
				_vm->_globals->_saveData->_data[svLastInventoryItem] = _vm->_globals->_inventory[newInventoryItem];
				_vm->_globals->_saveData->_data[svLastInvMouseCursor] = _vm->_events->_mouseCursorId;
				_vm->_objectsMan->loadObjectIniFile();
				_vm->_script->_tempObjectFl = false;

				if (_vm->_soundMan->_voiceOffFl) {
					do {
						_vm->_events->refreshScreenAndEvents();
					} while (!_vm->_globals->_exitId && _vm->_events->getMouseButton() != 1);
					_vm->_fontMan->hideText(9);
				}
				if (_vm->_globals->_exitId) {
					if (_vm->_globals->_exitId == 2) {
						_vm->_globals->_exitId = 0;
						break;
					}

					_vm->_globals->_exitId = 0;
					_inventBuf2 = _vm->_globals->freeMemory(_inventBuf2);
					_inventWin1 = _vm->_globals->freeMemory(_inventWin1);
					loopFl = true;
					break;
				} else
					_inventDisplayedFl = true;
			}
			if (_removeInventFl)
				break;
			_vm->_events->refreshScreenAndEvents();
			if (_vm->_globals->_screenId >= 35 && _vm->_globals->_screenId <= 40)
				_vm->_objectsMan->handleSpecialGames();
		}
	} while (loopFl);

	_vm->_fontMan->hideText(9);
	if (_inventDisplayedFl) {
		_inventDisplayedFl = false;
		_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, _inventX, 114, _inventWidth, _inventHeight, _vm->_graphicsMan->_frontBuffer, _inventX, 114);
		_vm->_graphicsMan->addDirtyRect(_inventX, 114, _inventX + _inventWidth, _inventWidth + 114);
		_vm->_objectsMan->_refreshBobMode10Fl = true;
	}

	_inventWin1 = _vm->_globals->freeMemory(_inventWin1);
	_inventBuf2 = _vm->_globals->freeMemory(_inventBuf2);

	int cursorId = _vm->_events->_mouseCursorId;
	if (cursorId == 1)
		showOptionsDialog();
	else if (cursorId == 3)
		showLoadGame();
	else if (cursorId == 2)
		showSaveGame();

	_vm->_events->_mouseCursorId = 4;
	_vm->_events->changeMouseCursor(4);
	_vm->_objectsMan->_oldBorderPos = Common::Point(0, 0);
	_vm->_objectsMan->_borderPos = Common::Point(0, 0);
	_vm->_globals->_disableInventFl = false;
	_vm->_graphicsMan->_scrollStatus = 0;
}

/**
 * Inventory Animations
 */
void DialogsManager::inventAnim() {
	if (_vm->_globals->_disableInventFl)
		return;

	if (_vm->_objectsMan->_eraseVisibleCounter && !_vm->_objectsMan->_visibleFl) {
		_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, _oldInventX, 27, 48, 38,
			_vm->_graphicsMan->_frontBuffer, _oldInventX, 27);
		_vm->_graphicsMan->addDirtyRect(_oldInventX, 27, _oldInventX + 48, 65);
		--_vm->_objectsMan->_eraseVisibleCounter;
	}

	if (_vm->_objectsMan->_visibleFl) {
		if (_oldInventX <= 1)
			_oldInventX = 2;
		_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, _oldInventX, 27, 48, 38,
			_vm->_graphicsMan->_frontBuffer, _oldInventX, 27);

		_vm->_graphicsMan->addDirtyRect(_oldInventX, 27, _oldInventX + 48, 65);
		int newOffset = _vm->_graphicsMan->_scrollOffset + 2;
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _inventoryIcons, newOffset + 300, 327, 0);
		_vm->_graphicsMan->addDirtyRect(newOffset, 27, newOffset + 45, 62);
		_oldInventX = newOffset;
	}

	if (_vm->_globals->_saveData->_data[svField357] == 1) {
		if (_vm->_globals->_saveData->_data[svField353] == 1)
			_vm->_graphicsMan->drawCompressedSprite(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_headSprites, 832, 325, 0, 0, 0, false);
		if (_vm->_globals->_saveData->_data[svField355] == 1)
			_vm->_graphicsMan->drawCompressedSprite(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_headSprites, 866, 325, 1, 0, 0, false);
		_vm->_graphicsMan->addDirtyRect(532, 25, 560, 60);
		_vm->_graphicsMan->addDirtyRect(566, 25, 594, 60);
	}
	if (_vm->_globals->_saveData->_data[svField356] == 1) {
		_vm->_graphicsMan->drawCompressedSprite(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_headSprites, 832, 325, 0, 0, 0, false);
		_vm->_graphicsMan->addDirtyRect(532, 25, 560, 60);
	}

	if (_vm->_globals->_saveData->_data[svField354] == 1) {
		_vm->_graphicsMan->drawCompressedSprite(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_headSprites, 832, 325, 0, 0, 0, false);
		_vm->_graphicsMan->addDirtyRect(532, 25, 560, 60);
	}
}

/**
 * Test dialog opening
 */
void DialogsManager::testDialogOpening() {
	if (_vm->_globals->_cityMapEnabledFl)
		_vm->_events->_gameKey = KEY_NONE;

	if ((_vm->_events->_gameKey == KEY_NONE) || _inventFl)
		return;

	DIALOG_KEY key = _vm->_events->_gameKey;
	_vm->_events->_gameKey = KEY_NONE;
	_inventFl = true;

	switch (key) {
	case KEY_INVENTORY:
		showInventory();
		break;
	case KEY_OPTIONS:
		_vm->_graphicsMan->_scrollStatus = 1;
		showOptionsDialog();
		_vm->_graphicsMan->_scrollStatus = 0;
		break;
	case KEY_LOAD:
		_vm->_graphicsMan->_scrollStatus = 1;
		showLoadGame();
		_vm->_graphicsMan->_scrollStatus = 0;
		break;
	case KEY_SAVE:
		_vm->_graphicsMan->_scrollStatus = 1;
		showSaveGame();
		_vm->_graphicsMan->_scrollStatus = 0;
		break;
	default:
		break;
	}

	_inventFl = false;
	_vm->_events->_gameKey = KEY_NONE;
}

/**
 * Load Game dialog
 */
void DialogsManager::showLoadGame() {
	_vm->_events->refreshScreenAndEvents();
	showSaveLoad(MODE_LOAD);

	int slotNumber;
	do {
		slotNumber = searchSavegames();
		_vm->_events->refreshScreenAndEvents();
	} while (!_vm->shouldQuit() && (!slotNumber || _vm->_events->getMouseButton() != 1));
	_vm->_objectsMan->_saveLoadFl = false;
	int16 startPosX = _vm->_events->_startPos.x + 183;
	_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, startPosX, 60, 274, 353, _vm->_graphicsMan->_frontBuffer, startPosX, 60);
	_vm->_graphicsMan->addDirtyRect(startPosX, 60, startPosX + 274, 413);
	_vm->_objectsMan->_refreshBobMode10Fl = true;
	_vm->_objectsMan->_saveLoadSprite = _vm->_globals->freeMemory(_vm->_objectsMan->_saveLoadSprite);
	_vm->_objectsMan->_saveLoadSprite2 = _vm->_globals->freeMemory(_vm->_objectsMan->_saveLoadSprite2);
	_vm->_objectsMan->_saveLoadX = 0;
	_vm->_objectsMan->_saveLoadY = 0;

	if (slotNumber != 7) {
		_vm->_saveLoad->loadGame(slotNumber);
	}

	_vm->_objectsMan->changeObject(14);
}

/**
 * Save Game dialog
 */
void DialogsManager::showSaveGame() {
	_vm->_events->refreshScreenAndEvents();

	showSaveLoad(MODE_SAVE);
	int slotNumber;
	do {
		slotNumber = searchSavegames();
		_vm->_events->refreshScreenAndEvents();
	} while (!_vm->shouldQuit() && (!slotNumber || _vm->_events->getMouseButton() != 1));

	_vm->_objectsMan->_saveLoadFl = false;
	int16 startPosX = _vm->_events->_startPos.x + 183;
	_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, startPosX, 60, 274, 353, _vm->_graphicsMan->_frontBuffer, startPosX, 60);
	_vm->_graphicsMan->addDirtyRect(startPosX, 60, startPosX + 274, 413);
	_vm->_objectsMan->_refreshBobMode10Fl = true;
	_vm->_objectsMan->_saveLoadSprite = _vm->_globals->freeMemory(_vm->_objectsMan->_saveLoadSprite);
	_vm->_objectsMan->_saveLoadSprite2 = _vm->_globals->freeMemory(_vm->_objectsMan->_saveLoadSprite2);
	_vm->_objectsMan->_saveLoadX = 0;
	_vm->_objectsMan->_saveLoadY = 0;

	if (slotNumber != 7) {
		// Since the original GUI doesn't support save names, use a default name
		Common::String saveName = Common::String::format("Save #%d", slotNumber);

		_vm->_events->refreshScreenAndEvents();
		// Save the game
		_vm->_saveLoad->saveGame(slotNumber, saveName);
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
		switch (_vm->_globals->_language) {
		case LANG_EN:
			filename = "SAVEAN.SPR";
			break;
		case LANG_FR:
			filename = "SAVEFR.SPR";
			break;
		case LANG_SP:
			filename = "SAVEES.SPR";
			break;
		default:
			break;
		}
	}

	_vm->_objectsMan->_saveLoadSprite = _vm->_objectsMan->loadSprite(filename);
	_vm->_objectsMan->_saveLoadSprite2 = _vm->_objectsMan->loadSprite("SAVE2.SPR");
	int16 startPosX = _vm->_events->_startPos.x;
	_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_saveLoadSprite, startPosX + 483, 360, 0);

	if (_vm->_globals->_language == LANG_FR) {
		if (mode == MODE_SAVE)
			_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_saveLoadSprite, startPosX + 525, 375, 1);
		else if (mode == MODE_LOAD)
			_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_saveLoadSprite, startPosX + 515, 375, 2);
	} else {
		if (mode == MODE_SAVE)
			_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_saveLoadSprite, startPosX + 535, 372, 1);
		else if (mode == MODE_LOAD)
			_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_saveLoadSprite, startPosX + 539, 372, 2);
	}

	for (int slotNumber = 1; slotNumber <= 6; ++slotNumber) {
		hopkinsSavegameHeader header;
		if (_vm->_saveLoad->readSavegameHeader(slotNumber, header, false)) {
			Graphics::Surface thumb8;
			_vm->_saveLoad->convertThumb16To8(header._thumbnail, &thumb8);

			byte *thumb = (byte *)thumb8.getPixels();

			int16 startPosX_ = _vm->_events->_startPos.x;
			switch (slotNumber) {
			case 1:
				_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, thumb, startPosX_ + 190, 112, 128, 87);
				break;
			case 2:
				_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, thumb, startPosX_ + 323, 112, 128, 87);
				break;
			case 3:
				_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, thumb, startPosX_ + 190, 203, 128, 87);
				break;
			case 4:
				_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, thumb, startPosX_ + 323, 203, 128, 87);
				break;
			case 5:
				_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, thumb, startPosX_ + 190, 294, 128, 87);
				break;
			case 6:
				_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, thumb, startPosX_ + 323, 294, 128, 87);
				break;
			default:
				break;
			}

			thumb8.free();
			header._thumbnail->free();
			delete header._thumbnail;
		}
	}

	_vm->_graphicsMan->copySurfaceRect(_vm->_graphicsMan->_frontBuffer, _vm->_objectsMan->_saveLoadSprite, _vm->_events->_startPos.x + 183, 60, 274, 353);
	_vm->_objectsMan->_saveLoadFl = true;
	_vm->_objectsMan->_saveLoadX = 0;
	_vm->_objectsMan->_saveLoadY = 0;
}

/**
 * Search savegames
 */
int DialogsManager::searchSavegames() {
	int xp = _vm->_events->getMouseX();
	int yp = _vm->_events->getMouseY();

	int16 startPosX = _vm->_graphicsMan->_scrollOffset = _vm->_events->_startPos.x;

	int slotNumber = 0;
	if (yp >= 112 && yp <= 198) {
		if (xp > startPosX + 189 && xp < startPosX + 318) {
			slotNumber = 1;
			_vm->_objectsMan->_saveLoadX = 189;
			_vm->_objectsMan->_saveLoadY = 111;
		} else if (xp > startPosX + 322 && xp < startPosX + 452) {
			slotNumber = 2;
			_vm->_objectsMan->_saveLoadX = 322;
			_vm->_objectsMan->_saveLoadY = 111;
		}
	} else if (yp >= 203 && yp <= 289) {
		if (xp > startPosX + 189 && xp < startPosX + 318) {
			slotNumber = 3;
			_vm->_objectsMan->_saveLoadX = 189;
			_vm->_objectsMan->_saveLoadY = 202;
		} else if (xp > startPosX + 322 && xp < startPosX + 452) {
			slotNumber = 4;
			_vm->_objectsMan->_saveLoadX = 322;
			_vm->_objectsMan->_saveLoadY = 202;
		}
	} else if (yp >= 294 && yp <= 380) {
		if (xp > startPosX + 189 && xp < startPosX + 318) {
			slotNumber = 5;
			_vm->_objectsMan->_saveLoadX = 189;
			_vm->_objectsMan->_saveLoadY = 293;
		} else if (xp > startPosX + 322 && xp < startPosX + 452) {
			slotNumber = 6;
			_vm->_objectsMan->_saveLoadX = 322;
			_vm->_objectsMan->_saveLoadY = 293;
		}
	} else if (yp >= 388 && yp <= 404 && xp > startPosX + 273 && xp < startPosX + 355) {
		slotNumber = 7;
		_vm->_objectsMan->_saveLoadX = 0;
		_vm->_objectsMan->_saveLoadY = 0;
	} else {
		slotNumber = 0;
		_vm->_objectsMan->_saveLoadX = 0;
		_vm->_objectsMan->_saveLoadY = 0;
	}

	return slotNumber;
}

} // End of namespace Hopkins
