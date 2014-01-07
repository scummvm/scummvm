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
	
	int v0, v2 = 0;

	for (int i = 0; i < 3; ++i) {
		int v1 = 0;
		while (v1 < 3) {
			_hotspots[v2].x1 = 87 * v1 + 262;
			_hotspots[v2].y1 = 74 * i + 69;
			_hotspots[v2].x2 = _hotspots[v2].x1 + 79;
			_hotspots[v2].y2 = _hotspots[v2].y1 + 66;
			_hotspots[v2].flags = 0;
			_hotspots[v2].id = v2;
			++v1;
			++v2;
		}
	}

	_hotspots[v2].x1 = 330;
	_hotspots[v2].y1 = 350;
	_hotspots[v2].x2 = 430;
	_hotspots[v2].y2 = 460;
	_hotspots[v2].flags = 2;
	_hotspots[v2].id = v2;

	v0 = v2 + 1;
	_hotspots[v0].x1 = 180;
	_hotspots[v0].y1 = 15;
	_hotspots[v0].x2 = 620;
	_hotspots[v0].y2 = 580;
	_hotspots[v0].flags = 0;
	_hotspots[v0].id = v0;

	++v0;
	_hotspots[v0].x1 = 0;
	_hotspots[v0].y1 = 0;
	_hotspots[v0].x2 = 799;
	_hotspots[v0].y2 = 599;
	_hotspots[v0].flags = 0;
	_hotspots[v0].id = v0;

	_hotspotsCount = v0 + 1;
}

void GnapEngine::initMenuHotspots2() {
	int i, v0;
	
	for (i = 0; i < 4; ++i) {
		_hotspots[i].x1 = 312;
		_hotspots[i].y1 = 48 * i + 85;
		_hotspots[i].x2 = _hotspots[i].x1 + 153;
		_hotspots[i].y2 = _hotspots[i].y1 + 37;
		_hotspots[i].flags = SF_GRAB_CURSOR;
		_hotspots[i].id = i;
	}
	_hotspots[i].x1 = 500;
	_hotspots[i].y1 = 72;
	_hotspots[i].x2 = 527;
	_hotspots[i].y2 = 99;
	_hotspots[i].flags = SF_DISABLED;
	_hotspots[i].id = i;
	v0 = i + 1;
	_hotspots[v0].x1 = 330;
	_hotspots[v0].y1 = 350;
	_hotspots[v0].x2 = 430;
	_hotspots[v0].y2 = 460;
	_hotspots[v0].flags = SF_GRAB_CURSOR;
	_hotspots[v0].id = v0;
	++v0;
	_hotspots[v0].x1 = 180;
	_hotspots[v0].y1 = 15;
	_hotspots[v0].x2 = 620;
	_hotspots[v0].y2 = 580;
	_hotspots[v0].flags = 0;
	_hotspots[v0].id = v0;
	++v0;
	_hotspots[v0].x1 = 0;
	_hotspots[v0].y1 = 0;
	_hotspots[v0].x2 = 799;
	_hotspots[v0].y2 = 599;
	_hotspots[v0].flags = 0;
	_hotspots[v0].id = v0;
	_hotspotsCount = v0 + 1;
}

void GnapEngine::initMenuQuitQueryHotspots() {
	_hotspots[0].x1 = 311;
	_hotspots[0].y1 = 197;
	_hotspots[0].x2 = 377;
	_hotspots[0].y2 = 237;
	_hotspots[0].flags = SF_GRAB_CURSOR;
	_hotspots[0].id = 0;
	_hotspots[1].x1 = 403;
	_hotspots[1].y1 = 197;
	_hotspots[1].x2 = 469;
	_hotspots[1].y2 = 237;
	_hotspots[1].flags = SF_GRAB_CURSOR;
	_hotspots[1].id = 1;
	_hotspots[2].x1 = 330;
	_hotspots[2].y1 = 350;
	_hotspots[2].x2 = 430;
	_hotspots[2].y2 = 460;
	_hotspots[2].flags = SF_GRAB_CURSOR;
	_hotspots[2].id = 2;
	_hotspots[3].x1 = 180;
	_hotspots[3].y1 = 15;
	_hotspots[3].x2 = 620;
	_hotspots[3].y2 = 580;
	_hotspots[3].flags = 0;
	_hotspots[3].id = 3;
	_hotspots[4].x1 = 0;
	_hotspots[4].y1 = 0;
	_hotspots[4].x2 = 799;
	_hotspots[4].y2 = 599;
	_hotspots[4].flags = 0;
	_hotspots[4].id = 4;
	_hotspotsCount = 5;
}

void GnapEngine::initSaveLoadHotspots() {
	int i, v0;
	for (i = 0; i < 7; ++i ) {
		_hotspots[i].x1 = 288;
		_hotspots[i].y1 = 31 * i + 74;
		_hotspots[i].x2 = _hotspots[i].x1 + 91;
		_hotspots[i].y2 = _hotspots[i].y1 + 22;
		_hotspots[i].flags = SF_GRAB_CURSOR;
		_hotspots[i].id = i;
	}
	if (_menuStatus == 2) {
		_hotspots[i].x1 = 416;
		_hotspots[i].y1 = 160;
		_hotspots[i].x2 = 499;
		_hotspots[i].y2 = 188;
		_hotspots[i].flags = SF_GRAB_CURSOR;
		_hotspots[i].id = i;
		++i;
	}
	_hotspots[i].x1 = 416;
	_hotspots[i].y1 = 213;
	_hotspots[i].x2 = 499;
	_hotspots[i].y2 = 241;
	_hotspots[i].flags = SF_GRAB_CURSOR;
	_hotspots[i].id = i;
	v0 = i + 1;
	_hotspots[v0].x1 = 330;
	_hotspots[v0].y1 = 350;
	_hotspots[v0].x2 = 430;
	_hotspots[v0].y2 = 460;
	_hotspots[v0].flags = SF_GRAB_CURSOR;
	_hotspots[v0].id = v0;
	++v0;
	_hotspots[v0].x1 = 180;
	_hotspots[v0].y1 = 15;
	_hotspots[v0].x2 = 620;
	_hotspots[v0].y2 = 580;
	_hotspots[v0].flags = 0;
	_hotspots[v0].id = v0;
	++v0;
	_hotspots[v0].x1 = 0;
	_hotspots[v0].y1 = 0;
	_hotspots[v0].x2 = 799;
	_hotspots[v0].y2 = 599;
	_hotspots[v0].flags = 0;
	_hotspots[v0].id = v0;
	_hotspotsCount = v0 + 1;
}

void GnapEngine::drawInventoryFrames() {
	for (int i = 0; i < 9; ++i)
		_gameSys->drawSpriteToSurface(_menuBackgroundSurface, _hotspots[i].x1 - 93, _hotspots[i].y1, 0x10001);
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
				_hotspots[_menuSpritesIndex].x1 - 93, _hotspots[_menuSpritesIndex].y1, 0x10000);
			_menuInventorySprites[_menuSpritesIndex] = _gameSys->createSurface(getInventoryItemSpriteNum(index) | 0x10000);
			if (index != _grabCursorSpriteIndex) {
				_menuInventoryIndices[_menuSpritesIndex] = index;
				_gameSys->insertSpriteDrawItem(_menuInventorySprites[_menuSpritesIndex],
					_hotspots[_menuSpritesIndex].x1 + ((79 - _menuInventorySprites[_menuSpritesIndex]->w) / 2),
					_hotspots[_menuSpritesIndex].y1 + ((66 - _menuInventorySprites[_menuSpritesIndex]->h) / 2),
					261);
			}
			_hotspots[_menuSpritesIndex].flags = SF_GRAB_CURSOR;
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

	int v43;

	_spriteHandle = 0;
	_cursorSprite = 0;
	_menuSprite1 = 0;
	_menuSprite2 = 0;
	_menuSaveLoadSprite = 0;
	_menuQuitQuerySprite = 0;

	_menuStatus = 0;
	_menuDone = false;

	createMenuSprite();//??? CHECKME Crashes when done in loadStockDat() find out why

	insertDeviceIconActive();

	for (int i = 0; i < 7; ++i) {
		_savegameFilenames[i][0] = 0;
		_savegameSprites[i] = 0;
	}

	v43 = 0;

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

	while (!isKeyStatus1(8) && !isKeyStatus1(28) && !_sceneDone && !_menuDone) {

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

	_mouseClickState.left = false;

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
	_hotspots[0].x1 = 262;
	_hotspots[0].y1 = 69;
	_hotspots[0].x2 = 341;
	_hotspots[0].y2 = 135;
	_sceneClickedHotspot = -1;
	if (_timers[2] == 0)
		_sceneClickedHotspot = getClickedHotspotId();
	if (_sceneClickedHotspot == -1 || _sceneClickedHotspot >= _menuSpritesIndex) {
		if (_sceneClickedHotspot == _hotspotsCount - 3) {
			if (_grabCursorSpriteIndex == -1) {
				_timers[2] = 10;
				playSound(0x108F4, 0);
				_menuStatus = 1;
				Common::Rect dirtyRect(_hotspots[0].x1, _hotspots[0].y1, _hotspots[2].x2, _hotspots[_hotspotsCount - 4].y2);
				drawInventoryFrames();
				initMenuHotspots2();
				removeInventorySprites();
				if (!_menuSprite1)
					_menuSprite1 = _gameSys->createSurface(0x104F8);
				_gameSys->insertSpriteDrawItem(_menuSprite1, 288, 79, 262);
				_gameSys->insertDirtyRect(dirtyRect);
			} else {
				playSound(0x108F5, 0);
			}
		} else if (_sceneClickedHotspot == _hotspotsCount - 1) {
			_timers[2] = 10;
			playSound(0x108F5, 0);
			_menuDone = true;
		}
	} else if (_sceneClickedHotspot != -1 && _menuInventoryIndices[_sceneClickedHotspot] != -1 && _grabCursorSpriteIndex == -1) {
		_gameSys->removeSpriteDrawItem(_menuInventorySprites[_sceneClickedHotspot], 261);
		setGrabCursorSprite(_menuInventoryIndices[_sceneClickedHotspot]);
		_menuInventoryIndices[_sceneClickedHotspot] = -1;
	} else if (_sceneClickedHotspot != -1 && _menuInventoryIndices[_sceneClickedHotspot] == -1 && _grabCursorSpriteIndex != -1) {
		_menuInventoryIndices[_sceneClickedHotspot] = _grabCursorSpriteIndex;
		_gameSys->insertSpriteDrawItem(_menuInventorySprites[_sceneClickedHotspot],
			_hotspots[_sceneClickedHotspot].x1 + ((79 - _menuInventorySprites[_sceneClickedHotspot]->w) / 2),
			_hotspots[_sceneClickedHotspot].y1 + (66 - _menuInventorySprites[_sceneClickedHotspot]->h) / 2,
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
			playSound(0x108AE, 0);
			deleteSurface(&_spriteHandle); // CHECKME
			_spriteHandle = _gameSys->createSurface(0x10001);
			_gameSys->insertSpriteDrawItem(_spriteHandle, _hotspots[_menuSpritesIndex - 1].x1, _hotspots[_menuSpritesIndex - 1].y1, 261);
			setGrabCursorSprite(kCombineItems[combineIndex].resultItem);
			removeInventorySprites();
			insertInventorySprites();
			delayTicksCursor(5);
		} else {
			playSound(0x108F5, 0);
		}
	}
}

void GnapEngine::updateMenuStatusMainMenu() {
	_hotspots[0].x1 = 312;
	_hotspots[0].y1 = 85;
	_hotspots[0].x2 = 465;
	_hotspots[0].y2 = 122;
	_sceneClickedHotspot = -1;
	if (!_timers[2])
		_sceneClickedHotspot = getClickedHotspotId();
		
	if (_sceneClickedHotspot != 1 && _sceneClickedHotspot != 0) {
		if (_sceneClickedHotspot != 2 && _hotspotsCount - 1 != _sceneClickedHotspot) {
			if (_sceneClickedHotspot == 3) {
				_timers[2] = 10;
				playSound(0x108F4, 0);
				_gameSys->removeSpriteDrawItem(_menuSprite1, 262);
				initMenuQuitQueryHotspots();
				_menuStatus = 4;
				if (!_menuQuitQuerySprite)
					_menuQuitQuerySprite = _gameSys->createSurface(0x104FC);
				_gameSys->insertSpriteDrawItem(_menuQuitQuerySprite, 254, 93, 262);
			} else if (_sceneClickedHotspot == 4) {
				playSound(0x108F4, 0);
				Common::Rect dirtyRect(0, 0, 799, 599);
				hideCursor();
				_largeSprite = _gameSys->allocSurface(800, 600);

				for (int i = 0; i < 3; ++i) {

					_timers[2] = 10;

					if (i == 0) {
						_gameSys->drawSpriteToSurface(_largeSprite, 0, 0, 0x1078D);
						_gameSys->insertSpriteDrawItem(_largeSprite, 0, 0, 300);
						/* TODO
						if (_midiFlag) {
							playMidi("pause.mid", 0);
						}
						*/
					} else if (i == 1) {
						_gameSys->drawSpriteToSurface(_largeSprite, 0, 0, 0x1078E);
						_gameSys->insertDirtyRect(dirtyRect);
					} else if (i == 2) {
						_gameSys->drawSpriteToSurface(_largeSprite, 0, 0, 0x1078F);
						_gameSys->insertDirtyRect(dirtyRect);
					}

					while (!_mouseClickState.left && !isKeyStatus1(28) && !isKeyStatus1(30) && !isKeyStatus1(29) && !_timers[2]) {
						gameUpdateTick();
					}

					playSound(0x108F5, 0);
					_mouseClickState.left = false;
					clearKeyStatus1(28);
					clearKeyStatus1(29);
					clearKeyStatus1(30);
				}
				
				_gameSys->removeSpriteDrawItem(_largeSprite, 300);
				delayTicksCursor(5);
				deleteSurface(&_largeSprite);
				showCursor();
			} else if (_hotspotsCount - 3 == _sceneClickedHotspot) {
				_timers[2] = 10;
				playSound(0x108F4, 0);
				initMenuHotspots1();
				/* TODO
				if (_mouseX < 93 || _mouseX > 638 || _mouseY < 0 || _mouseY > 600)
					SetCursorPos(400, 300);
					*/
				_menuStatus = 0;
				if (_menuSprite1)
					_gameSys->removeSpriteDrawItem(_menuSprite1, 262);
				insertInventorySprites();
				Common::Rect dirtyRect(_hotspots[0].x1, _hotspots[0].y1, _hotspots[2].x2, _hotspots[_hotspotsCount - 4].y2);
				_gameSys->insertDirtyRect(dirtyRect);
			}
		} else {
			playSound(0x108F5, 0);
			_menuDone = true;
		}
	} else {
		_timers[2] = 10;
		playSound(0x108F4, 0);
		_gameSys->removeSpriteDrawItem(_menuSprite1, 262);
		if (_menuSaveLoadSprite)
			deleteSurface(&_menuSaveLoadSprite);
		if (_sceneClickedHotspot == 1) {
			_menuStatus = 2;
			initSaveLoadHotspots();
			_menuSaveLoadSprite = _gameSys->createSurface(0x104FB);
		} else {
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
			_gameSys->insertSpriteDrawItem(_savegameSprites[i], 288, _hotspots[i].y1, 263);
		}
		_savegameIndex = -1;
	}
}

void GnapEngine::updateMenuStatusSaveGame() {
#if 0 // TODO
	_hotspots[0].x1 = 288;
	_hotspots[0].y1 = 74;
	_hotspots[0].x2 = 379;
	_hotspots[0].y2 = 96;
	_sceneClickedHotspot = -1;
	if (!_timers[2])
	_sceneClickedHotspot = getClickedHotspotId();
	if (_hotspotsCount - 3 == _sceneClickedHotspot)
	{
	_timers[2] = 10;
	playSound(0x108F4, 0);
	_menuStatus = 1;
	writeSavegame(_savegameIndex + 1, (int)&_savegameFilenames[30 * _savegameIndex], 1);
	}
	else if (_hotspotsCount - 4 == _sceneClickedHotspot)
	{
	_timers[2] = 10;
	playSound(0x108F5, 0);
	_menuStatus = 1;
	if (strcmp(&v43, &_savegameFilenames[30 * _savegameIndex]) && _savegameIndex != -1)
	{
	strcpy((char *)&_savegameFilenames[30 * _savegameIndex], &v43);
	if (_savegameSprites[_savegameIndex] != -1)
	{
	_gameSys->removeSpriteDrawItem(_savegameSprites[_savegameIndex], 263);
	delayTicksCursor(5);
	memFreeHandle(_savegameSprites[_savegameIndex]);
	}
	v16 = getSpriteWidthById(0x104F9);
	_savegameSprites[_savegameIndex] = allocSprite(v16, 40, 128, 0);
	}
	}
	else if (_hotspotsCount - 5 == _sceneClickedHotspot)
	{
	_timers[2] = 10;
	playSound(0x108F4, 0);
	if ((signed int)_savegameIndex > -1)
	writeSavegame(_savegameIndex + 1, (int)&_savegameFilenames[30 * _savegameIndex], 1);
	_menuStatus = 1;
	}
	else if (_hotspotsCount - 1 == _sceneClickedHotspot)
	{
	_menuDone = true;
	}
	else if (_sceneClickedHotspot != -1 && _hotspotsCount - 2 != _sceneClickedHotspot)
	{
	_timers[2] = 10;
	playSound(0x108F4, 0);
	v17 = strcmp(&v43, &_savegameFilenames[30 * _savegameIndex]);
	if (!v17)
	LOBYTE(v17) = 0;
	if ((unsigned int8)v17 & (_savegameIndex < 0xFFFFFFFF))
	{
	strcpy((char *)&_savegameFilenames[30 * _savegameIndex], &v43);
	if (_savegameSprites[_savegameIndex] != -1)
	{
	_gameSys->removeSpriteDrawItem(_savegameSprites[_savegameIndex], 263);
	delayTicksCursor(5);
	memFreeHandle(_savegameSprites[_savegameIndex]);
	}
	v18 = getSpriteWidthById(0x104F9);
	_savegameSprites[_savegameIndex] = allocSprite(v18, 40, 128, 0);
	drawTextToSurface(_savegameSprites[_savegameIndex], 0, 0, 255, 0, 0, &_savegameFilenames[30 * _savegameIndex]);
	insertSpriteDrawItem(_savegameSprites[_savegameIndex], 288, _hotspots[_savegameIndex].y1, 263);
	}
	_savegameIndex = _sceneClickedHotspot;
	v46 = strlen(&_savegameFilenames[30 * _sceneClickedHotspot]);
	strcpy(&v43, &_savegameFilenames[30 * _sceneClickedHotspot]);
	if (_cursorSprite == -1)
	{
	v19 = getTextHeight("_");
	v20 = getTextWidth("_");
	_cursorSprite = allocSprite(v20, v19, 128, 0);
	drawTextToSurface(_cursorSprite, 0, 0, 255, 0, 0, "_");
	}
	else
	{
	_gameSys->removeSpriteDrawItem(_cursorSprite, 264);
	}
	v21 = _hotspots[_savegameIndex].x2;
	v22 = v21 - getTextWidth("_");
	if (v22 > getTextWidth(&_savegameFilenames[30 * _savegameIndex]) + 288)
	{
	v25 = getTextWidth(&_savegameFilenames[30 * _savegameIndex]) + 288;
	insertSpriteDrawItem(_cursorSprite, v25, _hotspots[_savegameIndex].y1, 264);
	}
	else
	{
	v23 = _hotspots[_savegameIndex].x2;
	v24 = v23 - getTextWidth("_");
	insertSpriteDrawItem(_cursorSprite, v24, _hotspots[_savegameIndex].y1, 264);
	}
	}
	if ((signed int)_savegameIndex > -1 && keybChar)
	{
	if ((keybChar < 'A' || keybChar > 'Z') && (keybChar < '0' || keybChar > '9') && keybChar != ' ')
	{
	if (keybChar == 8)
	{
	if ((signed int)v46 > 0)
	--v46;
	*(&_savegameFilenames[30 * _savegameIndex] + v46) = 0;
	if (_savegameSprites[_savegameIndex] != -1)
	{
	_gameSys->removeSpriteDrawItem(_savegameSprites[_savegameIndex], 263);
	memFreeHandle(_savegameSprites[_savegameIndex]);
	}
	v32 = getSpriteWidthById(0x104F9);
	_savegameSprites[_savegameIndex] = allocSprite(v32, 40, 128, 0);
	drawTextToSurface(_savegameSprites[_savegameIndex], 0, 0, 255, 0, 0, &_savegameFilenames[30 * _savegameIndex]);
	insertSpriteDrawItem(_savegameSprites[_savegameIndex], 288, _hotspots[_savegameIndex].y1, 263);
	_gameSys->removeSpriteDrawItem(_cursorSprite, 264);
	v33 = _hotspots[_savegameIndex].y1;
	v34 = getTextWidth(&_savegameFilenames[30 * _savegameIndex]);
	insertSpriteDrawItem(_cursorSprite, LOWORD(_hotspots[_savegameIndex].x1) + v34, v33, 264);
	}
	else if (keybChar == 13)
	{
	_menuStatus = 1;
	writeSavegame(_savegameIndex + 1, (int)&_savegameFilenames[30 * _savegameIndex], 1);
	}
	}
	else
	{
	*(&_savegameFilenames[30 * _savegameIndex] + v46) = keybChar;
	if ((signed int)v46 < 28)
	++v46;
	*(&_savegameFilenames[30 * _savegameIndex] + v46) = 0;
	if (getTextWidth(&_savegameFilenames[30 * _savegameIndex]) > 91)
	{
	--v46;
	*(&_savegameFilenames[30 * _savegameIndex] + v46) = 0;
	}
	drawTextToSurface(_savegameSprites[_savegameIndex], 0, 0, 255, 0, 0, &_savegameFilenames[30 * _savegameIndex]);
	v26 = getTextWidth(&_savegameFilenames[30 * _savegameIndex]);
	rect.right = _hotspots[_savegameIndex].x1 + v26;
	v27 = rect.right;
	rect.left = v27 - 2 * getTextWidth("W");
	rect.top = _hotspots[_savegameIndex].y1;
	rect.bottom = _hotspots[_savegameIndex].y2;
	insertDirtyRect(&rect);
	_gameSys->removeSpriteDrawItem(_cursorSprite, 264);
	v28 = _hotspots[_savegameIndex].x2;
	v29 = getTextWidth("_");
	if (v28 - v29 > rect.right)
	{
	insertSpriteDrawItem(_cursorSprite, SLOWORD(rect.right), SLOWORD(rect.top), 264);
	}
	else
	{
	v30 = _hotspots[_savegameIndex].x2;
	v31 = v30 - getTextWidth("_");
	insertSpriteDrawItem(_cursorSprite, v31, SLOWORD(rect.top), 264);
	}
	clearKeyStatus1(8);
	}
	}
	keybChar = 0;
	if (_menuStatus == 1 || _menuDone)
	{
	_gameSys->removeSpriteDrawItem(_menuSprite2, 262);
	_gameSys->removeSpriteDrawItem(_menuSaveLoadSprite, 262);
	for (i = 0; i < 7; ++i)
	_gameSys->removeSpriteDrawItem(_savegameSprites[i], 263);
	_gameSys->removeSpriteDrawItem(_cursorSprite, 264);
	if (!_menuDone)
	{
	initMenuHotspots2();
	insertSpriteDrawItem(_menuSprite1, 288, 79, 262);
	}
	}
#endif
}

void GnapEngine::updateMenuStatusLoadGame() {
	_hotspots[0].x1 = 288;
	_hotspots[0].y1 = 74;
	_hotspots[0].x2 = 379;
	_hotspots[0].y2 = 96;
	_sceneClickedHotspot = -1;
	if (!_timers[2])
		_sceneClickedHotspot = getClickedHotspotId();
	if (_sceneClickedHotspot != -1 && _hotspotsCount - 2 != _sceneClickedHotspot) {
		_timers[2] = 10;
		if (_hotspotsCount - 4 <= _sceneClickedHotspot) {
			playSound(0x108F5, 0);
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
			playSound(0x108F5, 0);
		} else {
			playSound(0x108F4, 0);
			_sceneDone = true;
		}
	}
}

void GnapEngine::updateMenuStatusQueryQuit() {

	_hotspots[0].x1 = 311;
	_hotspots[0].y1 = 197;
	_hotspots[0].x2 = 377;
	_hotspots[0].y2 = 237;

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
		playSound(0x108F5, 0);
		_gameSys->removeSpriteDrawItem(_menuQuitQuerySprite, 262);
		_sceneDone = true;
		_gameDone = true;
	} else if (_sceneClickedHotspot == 4) {
		// Exit the device
		playSound(0x108F4, 0);
		_gameSys->removeSpriteDrawItem(_menuQuitQuerySprite, 262);
		_menuDone = true;
	} else if (_sceneClickedHotspot != -1) {
		// Return to the main menu
		playSound(0x108F4, 0);
		_gameSys->removeSpriteDrawItem(_menuQuitQuerySprite, 262);
		_timers[2] = 10;
		_menuStatus = 1;
		initMenuHotspots2();
		_gameSys->insertSpriteDrawItem(_menuSprite1, 288, 79, 262);
	}

}

} // End of namespace Gnap
