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

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "gui/saveload.h"
#include "graphics/thumbnail.h"

#include "gnap/gnap.h"
#include "gnap/datarchive.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"

namespace Gnap {

void GnapEngine::createMenuSprite() {
	_menuBackgroundSurface = _gameSys->createSurface(0x10002);
}

void GnapEngine::freeMenuSprite() {
	_gameSys->removeSpriteDrawItem(_menuBackgroundSurface, 260);
	delayTicksCursor(5);
	deleteSurface(&_menuBackgroundSurface);
}

void GnapEngine::initMenuHotspots1() {
	int curId = 0;

	for (int i = 0; i < 3; ++i) {
		int top = 74 * i + 69;
		for (int j = 0; j < 3; ++j) {
			int left = 87 * j + 262;
			_hotspots[curId]._rect = Common::Rect(left, top, left + 79, top + 66);
			_hotspots[curId]._flags = SF_NONE;
			++curId;
		}
	}

	_hotspots[curId]._rect = Common::Rect(330, 350, 430, 460);
	_hotspots[curId]._flags = SF_GRAB_CURSOR;

	++curId;
	_hotspots[curId]._rect = Common::Rect(180, 15, 620, 580);
	_hotspots[curId]._flags = SF_NONE;

	++curId;
	_hotspots[curId]._rect = Common::Rect(0, 0, 799, 599);
	_hotspots[curId]._flags = SF_NONE;

	_hotspotsCount = curId + 1;
}

void GnapEngine::initMenuHotspots2() {
	int curId = 0;

	for (int i = 0; i < 4; ++i) {
		int top = 48 * i + 85;
		_hotspots[curId]._rect = Common::Rect(312, top, 465, top + 37);
		_hotspots[curId]._flags = SF_GRAB_CURSOR;
		++curId;
	}

	_hotspots[curId]._rect = Common::Rect(500, 72, 527, 99);
	_hotspots[curId]._flags = SF_DISABLED;

	++curId;
	_hotspots[curId]._rect = Common::Rect(330, 350, 430, 460);
	_hotspots[curId]._flags = SF_GRAB_CURSOR;

	++curId;
	_hotspots[curId]._rect = Common::Rect(180, 15, 620, 580);
	_hotspots[curId]._flags = SF_NONE;

	++curId;
	_hotspots[curId]._rect = Common::Rect(0, 0, 799, 599);
	_hotspots[curId]._flags = SF_NONE;

	_hotspotsCount = curId + 1;
}

void GnapEngine::initMenuQuitQueryHotspots() {
	_hotspots[0]._rect = Common::Rect(311, 197, 377, 237);
	_hotspots[0]._flags = SF_GRAB_CURSOR;

	_hotspots[1]._rect = Common::Rect(403, 197, 469, 237);
	_hotspots[1]._flags = SF_GRAB_CURSOR;

	_hotspots[2]._rect = Common::Rect(330, 350, 430, 460);
	_hotspots[2]._flags = SF_GRAB_CURSOR;

	_hotspots[3]._rect = Common::Rect(180, 15, 620, 580);
	_hotspots[3]._flags = SF_NONE;

	_hotspots[4]._rect = Common::Rect(0, 0, 799, 599);
	_hotspots[4]._flags = SF_NONE;

	_hotspotsCount = 5;
}

void GnapEngine::initSaveLoadHotspots() {
	int curId = 0;

	for (int i = 0; i < 7; ++i ) {
		int top = 31 * i + 74;
		_hotspots[curId]._rect = Common::Rect(288, top, 379, top + 22);
		_hotspots[curId]._flags = SF_GRAB_CURSOR;
		++curId;
	}

	if (_menuStatus == 2) {
		_hotspots[curId]._rect = Common::Rect(416, 160, 499, 188);
		_hotspots[curId]._flags = SF_GRAB_CURSOR;
		++curId;
	}

	_hotspots[curId]._rect = Common::Rect(416, 213, 499, 241);
	_hotspots[curId]._flags = SF_GRAB_CURSOR;

	++curId;
	_hotspots[curId]._rect = Common::Rect(330, 350, 430, 460);
	_hotspots[curId]._flags = SF_GRAB_CURSOR;

	++curId;
	_hotspots[curId]._rect = Common::Rect(180, 15, 620, 580);
	_hotspots[curId]._flags = SF_NONE;

	++curId;
	_hotspots[curId]._rect = Common::Rect(0, 0, 799, 599);
	_hotspots[curId]._flags = SF_NONE;

	_hotspotsCount = curId + 1;
}

void GnapEngine::drawInventoryFrames() {
	for (int i = 0; i < 9; ++i)
		_gameSys->drawSpriteToSurface(_menuBackgroundSurface, _hotspots[i]._rect.left - 93, _hotspots[i]._rect.top, 0x10001);
}

void GnapEngine::insertInventorySprites() {
	for (int i = 0; i < 9; ++i) {
		_menuInventoryIndices[i] = -1;
		_gameSys->removeSpriteDrawItem(_menuInventorySprites[_sceneClickedHotspot], 261);
		_menuInventorySprites[i] = 0;
	}

	_menuSpritesIndex = 0;

	for (int index = 0; index < 30 && _menuSpritesIndex < 9; ++index) {
		if (invHas(index)) {
			_gameSys->drawSpriteToSurface(_menuBackgroundSurface,
				_hotspots[_menuSpritesIndex]._rect.left - 93, _hotspots[_menuSpritesIndex]._rect.top, 0x10000);
			_menuInventorySprites[_menuSpritesIndex] = _gameSys->createSurface(getInventoryItemSpriteNum(index) | 0x10000);
			if (index != _grabCursorSpriteIndex) {
				_menuInventoryIndices[_menuSpritesIndex] = index;
				_gameSys->insertSpriteDrawItem(_menuInventorySprites[_menuSpritesIndex],
					_hotspots[_menuSpritesIndex]._rect.left + ((79 - _menuInventorySprites[_menuSpritesIndex]->w) / 2),
					_hotspots[_menuSpritesIndex]._rect.top + ((66 - _menuInventorySprites[_menuSpritesIndex]->h) / 2),
					261);
			}
			_hotspots[_menuSpritesIndex]._flags = SF_GRAB_CURSOR;
			++_menuSpritesIndex;
		}
	}
}

void GnapEngine::removeInventorySprites() {
	for (int i = 0; i < _menuSpritesIndex; ++i)
		if (_menuInventorySprites[i])
			_gameSys->removeSpriteDrawItem(_menuInventorySprites[i], 261);
	delayTicksCursor(5);
	for (int j = 0; j < _menuSpritesIndex; ++j) {
		if (_menuInventorySprites[j]) {
			deleteSurface(&_menuInventorySprites[j]);
			_menuInventorySprites[j] = 0;
			_menuInventoryIndices[j] = -1;
		}
	}
	_menuSpritesIndex = 0;
}

void GnapEngine::runMenu() {
	_spriteHandle = nullptr;
	_cursorSprite = nullptr;
	_menuSprite1 = nullptr;
	_menuSprite2 = nullptr;
	_menuSaveLoadSprite = nullptr;
	_menuQuitQuerySprite = nullptr;

	_menuStatus = 0;
	_menuDone = false;

	delete _tempThumbnail;
	_tempThumbnail = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	Graphics::saveThumbnail(*_tempThumbnail);

	createMenuSprite();
	insertDeviceIconActive();

	for (int i = 0; i < 7; ++i) {
		_savegameFilenames[i][0] = 0;
		_savegameSprites[i] = nullptr;
	}

	if (_menuStatus == 0) {
		invAdd(kItemMagazine);
		setGrabCursorSprite(-1);
		hideCursor();
		initMenuHotspots1();
		drawInventoryFrames();
		insertInventorySprites();
		_gameSys->insertSpriteDrawItem(_menuBackgroundSurface, 93, 0, 260);
		showCursor();
		// SetCursorPos(400, 300);
		setVerbCursor(GRAB_CURSOR);
		// pollMessages();
	}

	_timers[2] = 10;

	while (!isKeyStatus1(Common::KEYCODE_BACKSPACE) && !isKeyStatus1(Common::KEYCODE_ESCAPE) && !_sceneDone && !_menuDone) {
		updateCursorByHotspot();

		switch (_menuStatus) {
		case 0:
			updateMenuStatusInventory();
			break;
		case 1:
			updateMenuStatusMainMenu();
			break;
		case 2:
			updateMenuStatusSaveGame();
			break;
		case 3:
			updateMenuStatusLoadGame();
			break;
		case 4:
			updateMenuStatusQueryQuit();
			break;
		default:
			break;
		}

		gameUpdateTick();
	}

	removeInventorySprites();
	if (_spriteHandle)
		_gameSys->removeSpriteDrawItem(_spriteHandle, 261);
	if (_menuSprite1)
		_gameSys->removeSpriteDrawItem(_menuSprite1, 262);
	if (_menuSprite2)
		_gameSys->removeSpriteDrawItem(_menuSprite2, 262);
	for (int i = 0; i < 7; ++i)
		if (_savegameSprites[i])
			_gameSys->removeSpriteDrawItem(_savegameSprites[i], 263);
	if (_cursorSprite)
		_gameSys->removeSpriteDrawItem(_cursorSprite, 264);
	if (_menuSaveLoadSprite)
		_gameSys->removeSpriteDrawItem(_menuSaveLoadSprite, 262);
	if (_menuQuitQuerySprite)
		_gameSys->removeSpriteDrawItem(_menuQuitQuerySprite, 262);
	if (_menuBackgroundSurface)
		_gameSys->removeSpriteDrawItem(_menuBackgroundSurface, 260);

	delayTicksCursor(5);

	deleteSurface(&_spriteHandle);
	deleteSurface(&_menuSprite1);
	deleteSurface(&_menuSprite2);
	for (int i = 0; i < 7; ++i)
		deleteSurface(&_savegameSprites[i]);
	deleteSurface(&_cursorSprite);
	deleteSurface(&_menuSaveLoadSprite);
	deleteSurface(&_menuQuitQuerySprite);

	_sceneClickedHotspot = -1;

	_timers[2] = getRandom(20) + 30;
	_timers[3] = getRandom(200) + 50;
	_timers[0] = getRandom(75) + 75;
	_timers[1] = getRandom(20) + 30;

	clearAllKeyStatus1();

	_mouseClickState._left = false;

	removeDeviceIconActive();

	freeMenuSprite();//??? CHECKME
}

void GnapEngine::updateMenuStatusInventory() {
	static const struct {
		int item1, item2, resultItem;
	} kCombineItems[] = {
		{kItemGrass, kItemMud, kItemDisguise},
		{kItemDice, kItemQuarterWithHole, kItemDiceQuarterHole},
		{kItemPill, kItemBucketWithBeer, kItemBucketWithPill}
	};

	updateGrabCursorSprite(0, 0);
	_hotspots[0]._rect = Common::Rect(262, 69, 341, 135);
	_sceneClickedHotspot = -1;
	if (_timers[2] == 0)
		_sceneClickedHotspot = getClickedHotspotId();
	if (_sceneClickedHotspot == -1 || _sceneClickedHotspot >= _menuSpritesIndex) {
		if (_sceneClickedHotspot == _hotspotsCount - 3) {
			if (_grabCursorSpriteIndex == -1) {
				_timers[2] = 10;
				playSound(0x108F4, false);
				_menuStatus = 1;
				Common::Rect dirtyRect(_hotspots[0]._rect.left, _hotspots[0]._rect.top, _hotspots[2]._rect.right, _hotspots[_hotspotsCount - 4]._rect.bottom);
				drawInventoryFrames();
				initMenuHotspots2();
				removeInventorySprites();
				if (!_menuSprite1)
					_menuSprite1 = _gameSys->createSurface(0x104F8);
				_gameSys->insertSpriteDrawItem(_menuSprite1, 288, 79, 262);
				_gameSys->insertDirtyRect(dirtyRect);
			} else {
				playSound(0x108F5, false);
			}
		} else if (_sceneClickedHotspot == _hotspotsCount - 1) {
			_timers[2] = 10;
			playSound(0x108F5, false);
			_menuDone = true;
		}
	} else if (_sceneClickedHotspot != -1 && _menuInventoryIndices[_sceneClickedHotspot] != -1 && _grabCursorSpriteIndex == -1) {
		_gameSys->removeSpriteDrawItem(_menuInventorySprites[_sceneClickedHotspot], 261);
		setGrabCursorSprite(_menuInventoryIndices[_sceneClickedHotspot]);
		_menuInventoryIndices[_sceneClickedHotspot] = -1;
	} else if (_sceneClickedHotspot != -1 && _menuInventoryIndices[_sceneClickedHotspot] == -1 && _grabCursorSpriteIndex != -1) {
		_menuInventoryIndices[_sceneClickedHotspot] = _grabCursorSpriteIndex;
		_gameSys->insertSpriteDrawItem(_menuInventorySprites[_sceneClickedHotspot],
			_hotspots[_sceneClickedHotspot]._rect.left + ((79 - _menuInventorySprites[_sceneClickedHotspot]->w) / 2),
			_hotspots[_sceneClickedHotspot]._rect.top + (66 - _menuInventorySprites[_sceneClickedHotspot]->h) / 2,
			261);
		setGrabCursorSprite(-1);
	} else if (_sceneClickedHotspot != -1 && _menuInventoryIndices[_sceneClickedHotspot] != -1 && _grabCursorSpriteIndex != -1) {
		int combineIndex = -1;
		for (int i = 0; i < ARRAYSIZE(kCombineItems); ++i) {
			if ((_grabCursorSpriteIndex == kCombineItems[i].item1 && _menuInventoryIndices[_sceneClickedHotspot] == kCombineItems[i].item2) ||
				(_grabCursorSpriteIndex == kCombineItems[i].item2 && _menuInventoryIndices[_sceneClickedHotspot] == kCombineItems[i].item1)) {
				combineIndex = i;
				break;
			}
		}
		if (combineIndex >= 0) {
			invRemove(kCombineItems[combineIndex].item1);
			invRemove(kCombineItems[combineIndex].item2);
			invAdd(kCombineItems[combineIndex].resultItem);
			playSound(0x108AE, false);
			deleteSurface(&_spriteHandle); // CHECKME
			_spriteHandle = _gameSys->createSurface(0x10001);
			_gameSys->insertSpriteDrawItem(_spriteHandle, _hotspots[_menuSpritesIndex - 1]._rect.left, _hotspots[_menuSpritesIndex - 1]._rect.top, 261);
			setGrabCursorSprite(kCombineItems[combineIndex].resultItem);
			removeInventorySprites();
			insertInventorySprites();
			delayTicksCursor(5);
		} else {
			playSound(0x108F5, false);
		}
	}
}

void GnapEngine::updateMenuStatusMainMenu() {
	_hotspots[0]._rect = Common::Rect(312, 85, 465, 122);
	_sceneClickedHotspot = -1;
	if (!_timers[2])
		_sceneClickedHotspot = getClickedHotspotId();

	if (_sceneClickedHotspot != 1 && _sceneClickedHotspot != 0) {
		if (_sceneClickedHotspot != 2 && _hotspotsCount - 1 != _sceneClickedHotspot) {
			if (_sceneClickedHotspot == 3) {
				// Quit
				_timers[2] = 10;
				playSound(0x108F4, false);
				_gameSys->removeSpriteDrawItem(_menuSprite1, 262);
				initMenuQuitQueryHotspots();
				_menuStatus = 4;
				if (!_menuQuitQuerySprite)
					_menuQuitQuerySprite = _gameSys->createSurface(0x104FC);
				_gameSys->insertSpriteDrawItem(_menuQuitQuerySprite, 254, 93, 262);
			} else if (_sceneClickedHotspot == 4) {
				// Pause ?
				playSound(0x108F4, false);
				Common::Rect dirtyRect(0, 0, 799, 599);
				hideCursor();
				_largeSprite = _gameSys->allocSurface(800, 600);

				for (int i = 0; i < 3; ++i) {
					_timers[2] = 10;

					if (i == 0) {
						_gameSys->drawSpriteToSurface(_largeSprite, 0, 0, 0x1078D);
						_gameSys->insertSpriteDrawItem(_largeSprite, 0, 0, 300);
						playMidi("pause.mid");
					} else if (i == 1) {
						_gameSys->drawSpriteToSurface(_largeSprite, 0, 0, 0x1078E);
						_gameSys->insertDirtyRect(dirtyRect);
					} else if (i == 2) {
						_gameSys->drawSpriteToSurface(_largeSprite, 0, 0, 0x1078F);
						_gameSys->insertDirtyRect(dirtyRect);
					}

					while (!_mouseClickState._left && !isKeyStatus1(Common::KEYCODE_ESCAPE) && !isKeyStatus1(Common::KEYCODE_RETURN)
							&& !isKeyStatus1(Common::KEYCODE_SPACE) && !_timers[2] && !_gameDone)
						gameUpdateTick();

					playSound(0x108F5, false);
					_mouseClickState._left = false;
					clearKeyStatus1(Common::KEYCODE_ESCAPE);
					clearKeyStatus1(Common::KEYCODE_RETURN);
					clearKeyStatus1(Common::KEYCODE_SPACE);
				}

				_gameSys->removeSpriteDrawItem(_largeSprite, 300);
				delayTicksCursor(5);
				deleteSurface(&_largeSprite);
				showCursor();
			} else if (_hotspotsCount - 3 == _sceneClickedHotspot) {
				// Button - Return to the inventory
				_timers[2] = 10;
				playSound(0x108F4, false);
				initMenuHotspots1();
				_menuStatus = 0;
				if (_menuSprite1)
					_gameSys->removeSpriteDrawItem(_menuSprite1, 262);
				insertInventorySprites();
				Common::Rect dirtyRect(_hotspots[0]._rect.left, _hotspots[0]._rect.top, _hotspots[2]._rect.right, _hotspots[_hotspotsCount - 4]._rect.bottom);
				_gameSys->insertDirtyRect(dirtyRect);
			}
		} else {
			// Resume
			playSound(0x108F5, false);
			_menuDone = true;
		}
	} else {
		// Save / Load
#if 1
		_timers[2] = 10;
		playSound(0x108F4, false);

		if (_sceneClickedHotspot == 1) {
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
			int16 savegameId = dialog->runModalWithCurrentTarget();
			Common::String savegameDescription = dialog->getResultString();
			delete dialog;

			if (savegameId != -1) {
				saveGameState(savegameId, savegameDescription);
			}
		} else {
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
			int16 savegameId = dialog->runModalWithCurrentTarget();
			delete dialog;

			if (savegameId != -1) {
				loadGameState(savegameId);
				_wasSavegameLoaded = true;
				_menuDone = true;
				_sceneDone = true;
				playSound(0x108F4, false);
			} else {
				playSound(0x108F5, false);
			}
		}
	}
#else
	// NOTE:
	// This is the code for the original behavior.
	// It's currently not working prolery, but could be
	// fixed to replace the ScummVM screens currently
	// used.
		_timers[2] = 10;
		playSound(0x108F4, false);
		_gameSys->removeSpriteDrawItem(_menuSprite1, 262);
		if (_menuSaveLoadSprite)
			deleteSurface(&_menuSaveLoadSprite);
		if (_sceneClickedHotspot == 1) {
			// Save
			_menuStatus = 2;
			initSaveLoadHotspots();
			_menuSaveLoadSprite = _gameSys->createSurface(0x104FB);
		} else {
			// Load
			_menuStatus = 3;
			initSaveLoadHotspots();
			_menuSaveLoadSprite = _gameSys->createSurface(0x104FA);
		}
		_gameSys->insertSpriteDrawItem(_menuSaveLoadSprite, 403, 72, 262);
		if (!_menuSprite2)
			_menuSprite2 = _gameSys->createSurface(0x104F9);
		_gameSys->insertSpriteDrawItem(_menuSprite2, 277, 66, 262);
		for (int i = 0; i < 7; ++i) {
			Common::String savegameDescription;
			if (!_savegameSprites[i])
				_savegameSprites[i] = _gameSys->allocSurface(111, 40);
			if (readSavegameDescription(i + 1, savegameDescription) == 0)
				strncpy(_savegameFilenames[i], savegameDescription.c_str(), 40);
			_gameSys->drawTextToSurface(_savegameSprites[i], 0, 0, 255, 0, 0, _savegameFilenames[i]);
			_gameSys->insertSpriteDrawItem(_savegameSprites[i], 288, _hotspots[i].top, 263);
		}
		_savegameIndex = -1;
	}
#endif
}

Common::Error GnapEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(
		getSaveStateName(slot));
	if (!out)
		return Common::kCreatingFileFailed;

	GnapSavegameHeader header;
	header._saveName = desc;
	writeSavegameHeader(out, header);

	Common::Serializer s(nullptr, out);
	synchronize(s);

	out->finalize();
	delete out;

	return Common::kNoError;
}

void GnapEngine::synchronize(Common::Serializer &s) {
	if (s.isSaving()) {
		s.syncAsSint32LE(_currentSceneNum);
		s.syncAsSint32LE(_prevSceneNum);
		s.syncAsSint32LE(_cursorValue);
		s.syncAsUint32LE(_inventory);
		s.syncAsUint32LE(_gameFlags);
	} else {
		s.syncAsSint32LE(_newSceneNum);
		s.syncAsSint32LE(_currentSceneNum);
		s.syncAsSint32LE(_newCursorValue);
		s.syncAsUint32LE(_inventory);
		s.syncAsUint32LE(_gameFlags);

		if (isFlag(kGFUnk24))
			_timers[9] = 600;
	}
}

const char *const SAVEGAME_STR = "GNAP";
#define SAVEGAME_STR_SIZE 4
void GnapEngine::writeSavegameHeader(Common::OutSaveFile *out, GnapSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(GNAP_SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(header._saveName);
	out->writeByte('\0');

	// This implies the menu is used
	// If we want to save/load at any time, then a check should be added
	out->write(_tempThumbnail->getData(), _tempThumbnail->size());

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
}

WARN_UNUSED_RESULT bool GnapEngine::readSavegameHeader(Common::InSaveFile *in, GnapSavegameHeader &header, bool skipThumbnail) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > GNAP_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header._saveName += ch;

	// Get the thumbnail, saved in v2 or later
	if (header._version == 1)
		header._thumbnail = nullptr;
	else {
		if (!Graphics::loadThumbnail(*in, header._thumbnail, skipThumbnail)) {
			return false;
		}
	}

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();

	return true;
}

Common::Error GnapEngine::loadGameState(int slot) {
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
		getSaveStateName(slot));
	if (!saveFile)
		return Common::kReadingFailed;

	Common::Serializer s(saveFile, nullptr);

	// Load the savegame header
	GnapSavegameHeader header;
	if (!readSavegameHeader(saveFile, header))
		error("Invalid savegame");

	if (header._thumbnail) {
		header._thumbnail->free();
		delete header._thumbnail;
	}

	synchronize(s);
	delete saveFile;

	_loadGameSlot = slot;
	return Common::kNoError;
}

void GnapEngine::updateMenuStatusSaveGame() {
#if 0
	// NOTE:
	// This is the code for the original screen game.
	// It could be eventually fixed and could replace
	// the ScummVM screens currently used.

	char v43[30];
	int v46;
	v43[0] = '\0';
	_hotspots[0]._x1 = 288;
	_hotspots[0]._y1 = 74;
	_hotspots[0]._x2 = 379;
	_hotspots[0]._y2 = 96;
	_sceneClickedHotspot = -1;

	if (!_timers[2])
		_sceneClickedHotspot = getClickedHotspotId();

	if (_hotspotsCount - 3 == _sceneClickedHotspot) {
		// Button
		_timers[2] = 10;
		playSound(0x108F4, false);
		_menuStatus = 1;
		warning("writeSavegame(_savegameIndex + 1, (int)&_savegameFilenames[30 * _savegameIndex], 1);");
	} else if (_hotspotsCount - 4 == _sceneClickedHotspot) {
		// Cancel
		_timers[2] = 10;
		playSound(0x108F5, false);
		_menuStatus = 1;
		if (strcmp(v43, _savegameFilenames[_savegameIndex]) && _savegameIndex != -1) {
			strcpy(_savegameFilenames[_savegameIndex], v43);
			if (_savegameSprites[_savegameIndex] != nullptr) {
				_gameSys->removeSpriteDrawItem(_savegameSprites[_savegameIndex], 263);
				delayTicksCursor(5);
				warning("memFreeHandle(_savegameSprites[_savegameIndex]);");
			}
			int v16 = _gameSys->getSpriteWidthById(0x104F9);
			warning("_savegameSprites[_savegameIndex] = allocSprite(v16, 40, 128, 0);");
		}
	} else if (_hotspotsCount - 5 == _sceneClickedHotspot) {
		// OK
		_timers[2] = 10;
		playSound(0x108F4, false);
		if (_savegameIndex != -1)
			warning("writeSavegame(_savegameIndex + 1, (int)&_savegameFilenames[30 * _savegameIndex], 1);");
		_menuStatus = 1;
	} else if (_hotspotsCount - 1 == _sceneClickedHotspot) {
		// in background
		_menuDone = true;
	} else if (_sceneClickedHotspot != -1 && _hotspotsCount - 2 != _sceneClickedHotspot) {
		// Savegame name
		_timers[2] = 10;
		playSound(0x108F4, false);
		if (strcmp(v43, _savegameFilenames[_savegameIndex]) & (_savegameIndex != -1)) {
			strcpy(_savegameFilenames[_savegameIndex], v43);
			if (_savegameSprites[_savegameIndex] != nullptr) {
				_gameSys->removeSpriteDrawItem(_savegameSprites[_savegameIndex], 263);
				delayTicksCursor(5);
				warning("memFreeHandle(_savegameSprites[_savegameIndex]);");
			}
			int v18 = _gameSys->getSpriteWidthById(0x104F9);
			_savegameSprites[_savegameIndex] = _gameSys->allocSurface(v18, 40);
			_gameSys->drawTextToSurface(_savegameSprites[_savegameIndex], 0, 0, 255, 0, 0, _savegameFilenames[_savegameIndex]);
			_gameSys->insertSpriteDrawItem(_savegameSprites[_savegameIndex], 288, _hotspots[_savegameIndex]._y1, 263);
		}
		_savegameIndex = _sceneClickedHotspot;
		v46 = strlen(_savegameFilenames[_sceneClickedHotspot]);
		strcpy(v43, _savegameFilenames[_sceneClickedHotspot]);
		if (_cursorSprite == nullptr) {
			int v19 = _gameSys->getTextHeight("_");
			int v20 = _gameSys->getTextWidth("_");
			_cursorSprite = _gameSys->allocSurface(v20, v19);
			_gameSys->drawTextToSurface(_cursorSprite, 0, 0, 255, 0, 0, "_");
		} else {
			_gameSys->removeSpriteDrawItem(_cursorSprite, 264);
		}
		int v21 = _hotspots[_savegameIndex]._x2;
		int v22 = v21 - _gameSys->getTextWidth("_");
		if (v22 > _gameSys->getTextWidth(_savegameFilenames[_savegameIndex]) + 288) {
			int v25 = _gameSys->getTextWidth(_savegameFilenames[_savegameIndex]) + 288;
			_gameSys->insertSpriteDrawItem(_cursorSprite, v25, _hotspots[_savegameIndex]._y1, 264);
		} else {
			int v23 = _hotspots[_savegameIndex]._x2;
			int v24 = v23 - _gameSys->getTextWidth("_");
			_gameSys->insertSpriteDrawItem(_cursorSprite, v24, _hotspots[_savegameIndex]._y1, 264);
		}
	}

	updateEvents();
	Common::Event event;
	_eventMan->pollEvent(event);

	Common::KeyCode keycode = event.kbd.keycode;
	if (_savegameIndex != -1 && keycode) {
		if ((keycode < Common::KEYCODE_a || keycode > Common::KEYCODE_z) && (keycode < Common::KEYCODE_0 || keycode > Common::KEYCODE_9) && keycode != Common::KEYCODE_SPACE) {
			if (keycode == Common::KEYCODE_BACKSPACE) {
				if (v46 > 0)
					--v46;
				_savegameFilenames[_savegameIndex][v46] = '\0';
				if (_savegameSprites[_savegameIndex] != nullptr) {
					_gameSys->removeSpriteDrawItem(_savegameSprites[_savegameIndex], 263);
					warning("memFreeHandle(_savegameSprites[_savegameIndex]);");
				}
				int v32 = _gameSys->getSpriteWidthById(0x104F9);
				_savegameSprites[_savegameIndex] = _gameSys->allocSurface(v32, 40);
				_gameSys->drawTextToSurface(_savegameSprites[_savegameIndex], 0, 0, 255, 0, 0, _savegameFilenames[_savegameIndex]);
				_gameSys->insertSpriteDrawItem(_savegameSprites[_savegameIndex], 288, _hotspots[_savegameIndex]._y1, 263);
				_gameSys->removeSpriteDrawItem(_cursorSprite, 264);
				int v33 = _hotspots[_savegameIndex]._y1;
				int v34 = _gameSys->getTextWidth(_savegameFilenames[_savegameIndex]);
				_gameSys->insertSpriteDrawItem(_cursorSprite, _hotspots[_savegameIndex]._x1 + v34, v33, 264);
			} else if (keycode == Common::KEYCODE_RETURN) {
				_menuStatus = 1;
				warning("writeSavegame(_savegameIndex + 1, (int)&_savegameFilenames[30 * _savegameIndex], 1);");
			}
		} else {
			_savegameFilenames[_savegameIndex][v46] = event.kbd.ascii;
			if (v46 < 28)
				++v46;
			_savegameFilenames[_savegameIndex][v46] = '\0';
			if (_gameSys->getTextWidth(_savegameFilenames[_savegameIndex]) > 91) {
				--v46;
				_savegameFilenames[_savegameIndex][v46] = '\0';
			}
			_gameSys->drawTextToSurface(_savegameSprites[_savegameIndex], 0, 0, 255, 0, 0, _savegameFilenames[_savegameIndex]);
			int v26 = _gameSys->getTextWidth(_savegameFilenames[_savegameIndex]);
			Common::Rect rect;
			rect.right = _hotspots[_savegameIndex]._x1 + v26;
			int v27 = rect.right;
			rect.left = v27 - 2 * _gameSys->getTextWidth("W");
			rect.top = _hotspots[_savegameIndex]._y1;
			rect.bottom = _hotspots[_savegameIndex]._y2;
			_gameSys->insertDirtyRect(rect);
			_gameSys->removeSpriteDrawItem(_cursorSprite, 264);
			int v28 = _hotspots[_savegameIndex]._x2;
			int v29 = _gameSys->getTextWidth("_");
			if (v28 - v29 > rect.right)
				_gameSys->insertSpriteDrawItem(_cursorSprite, rect.right, rect.top, 264);
			else {
				int v30 = _hotspots[_savegameIndex]._x2;
				int v31 = v30 - _gameSys->getTextWidth("_");
				_gameSys->insertSpriteDrawItem(_cursorSprite, v31, rect.top, 264);
			}
			clearKeyStatus1(8);
		}
	}

//	warning("keybChar = 0;");
	if (_menuStatus == 1 || _menuDone) {
		_gameSys->removeSpriteDrawItem(_menuSprite2, 262);
		_gameSys->removeSpriteDrawItem(_menuSaveLoadSprite, 262);
		for (int i = 0; i < 7; ++i)
			_gameSys->removeSpriteDrawItem(_savegameSprites[i], 263);
		_gameSys->removeSpriteDrawItem(_cursorSprite, 264);
		if (!_menuDone) {
			initMenuHotspots2();
			_gameSys->insertSpriteDrawItem(_menuSprite1, 288, 79, 262);
		}
	}
#endif
}

void GnapEngine::updateMenuStatusLoadGame() {
	_hotspots[0]._rect = Common::Rect(288, 74, 379, 96);
	_sceneClickedHotspot = -1;
	if (!_timers[2])
		_sceneClickedHotspot = getClickedHotspotId();
	if (_sceneClickedHotspot != -1 && _hotspotsCount - 2 != _sceneClickedHotspot) {
		_timers[2] = 10;
		if (_hotspotsCount - 4 <= _sceneClickedHotspot) {
			playSound(0x108F5, false);
			_gameSys->removeSpriteDrawItem(_menuSprite2, 262);
			_gameSys->removeSpriteDrawItem(_menuSaveLoadSprite, 262);
			for (int i = 0; i < 7; ++i)
				_gameSys->removeSpriteDrawItem(_savegameSprites[i], 263);
			if (_hotspotsCount - 1 == _sceneClickedHotspot) {
				_menuDone = true;
			} else {
				_menuStatus = 1;
				initMenuHotspots2();
				_gameSys->insertSpriteDrawItem(_menuSprite1, 288, 79, 262);
			}
		} else if (loadSavegame(_sceneClickedHotspot + 1)) {
			playSound(0x108F5, false);
		} else {
			playSound(0x108F4, false);
			_sceneDone = true;
		}
	}
}

void GnapEngine::updateMenuStatusQueryQuit() {
	_hotspots[0]._rect = Common::Rect(311, 197, 377, 237);
	_sceneClickedHotspot = -1;

	if (!_timers[2])
		_sceneClickedHotspot = getClickedHotspotId();

	/*	_sceneClickedHotspot
		0	Yes
		1	No
		2	Button
		3	Display
		4	Background
	*/

	if (_sceneClickedHotspot == 0) {
		// Quit the game
		playSound(0x108F5, false);
		_gameSys->removeSpriteDrawItem(_menuQuitQuerySprite, 262);
		_sceneDone = true;
		_gameDone = true;
	} else if (_sceneClickedHotspot == 4) {
		// Exit the device
		playSound(0x108F4, false);
		_gameSys->removeSpriteDrawItem(_menuQuitQuerySprite, 262);
		_menuDone = true;
	} else if (_sceneClickedHotspot != -1) {
		// Return to the main menu
		playSound(0x108F4, false);
		_gameSys->removeSpriteDrawItem(_menuQuitQuerySprite, 262);
		_timers[2] = 10;
		_menuStatus = 1;
		initMenuHotspots2();
		_gameSys->insertSpriteDrawItem(_menuSprite1, 288, 79, 262);
	}
}

} // End of namespace Gnap
