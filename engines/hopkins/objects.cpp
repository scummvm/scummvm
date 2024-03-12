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

#include "hopkins/objects.h"

#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"

#include "common/system.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"

namespace Hopkins {

ObjectsManager::ObjectsManager(HopkinsEngine *vm) {
	_vm = vm;

	for (int i = 0; i < 6; ++i)
		Common::fill((byte *)&_sprite[i], (byte *)&_sprite[i] + sizeof(SpriteItem), 0);

	for (int i = 0; i < 36; ++i) {
		Common::fill((byte *)&_bob[i], (byte *)&_bob[i] + sizeof(BobItem), 0);
		Common::fill((byte *)&_lockedAnims[i], (byte *)&_lockedAnims[i] + sizeof(LockAnimItem), 0);
	}

	for (int i = 0; i < 30; ++i) {
		Common::fill((byte *)&_vBob[i], (byte *)&_vBob[i] + sizeof(VBobItem), 0);
	}

	for (int i = 0; i < 300; ++i)
		Common::fill((byte *)&_objectAuthIcons[i], (byte *)&_objectAuthIcons[i] + sizeof(ObjectAuthIcon), 0);

	_sortedDisplayCount = 0;
	for (int i = 0; i < 51; ++i)
		Common::fill((byte *)&_sortedDisplay[i], (byte *)&_sortedDisplay[i] + sizeof(SortItem), 0);

	for (int i = 0; i < 25; ++i)
		Common::fill((byte *)&_hidingItem[i], (byte *)&_hidingItem[i] + sizeof(HidingItem), 0);

	for (int i = 0; i < 6; ++i)
		_hidingItemData[i] = nullptr;

	for (int i = 0; i < 6; ++i)
		Common::fill((byte *)&_liste[i], (byte *)&_liste[i] + sizeof(ListeItem), 0);

	for (int i = 0; i < 35; ++i)
		Common::fill((byte *)&_liste2[i], (byte *)&_liste2[i] + sizeof(ListeItem), 0);

	_helicopterFl = false;
	_priorityFl = false;
	_oldBorderPos = Common::Point(0, 0);
	_oldBorderSpriteIndex = 0;
	_borderPos = Common::Point(0, 0);
	_borderSpriteIndex = 0;
	_saveLoadX = _saveLoadY = 0;
	_oldCharacterPosX = _oldCharacterPosY = 0;
	_eraseVisibleCounter = 0;
	_saveLoadSprite = nullptr;
	_saveLoadSprite2 = nullptr;
	_spritePtr = nullptr;
	_oldSpriteData = nullptr;
	_saveLoadFl = false;
	_visibleFl = false;
	_zoneNum = 0;
	_forceZoneFl = false;
	_changeVerbFl = false;
	_verb = 0;
	_changeHeadFl = false;
	_disableFl = false;
	_twoCharactersFl = false;
	_characterPos = Common::Point(0, 0);
	_startSpriteIndex = 0;
	_jumpVerb = 0;
	_jumpZone = 0;
	_oldSpriteIndex = 0;
	_oldFrameIndex = 0;
	_oldFlipFl = false;
	_curObjectIndex = 0;
	_forestFl = false;
	_mapCarPosX = _mapCarPosY = 0;
	_forestSprite = nullptr;
	_gestureBuf = nullptr;
	_curGestureFile = 0;
	_headSprites = nullptr;
	_homeRateCounter = 0;
	_lastDirection = DIR_NONE;
	_oldDirection = DIR_NONE;
	_oldDirectionSpriteIdx = 59;
	_objectWidth = _objectHeight = 0;
	_hidingActiveFl = false;
	_curObjectFileNum = 0;
	_objectDataBuf = nullptr;
	_charactersEnabledFl = false;
	_refreshBobMode10Fl = false;
}

ObjectsManager::~ObjectsManager() {
	_vm->_globals->freeMemory(_forestSprite);
	_vm->_globals->freeMemory(_gestureBuf);
	_vm->_globals->freeMemory(_headSprites);
	_vm->_globals->freeMemory(_objectDataBuf);
	clearVBob();

	for (int idx = 0; idx < 6; ++idx)
		_hidingItemData[idx] = _vm->_globals->freeMemory(_hidingItemData[idx]);
}

void ObjectsManager::clearAll() {
	_forestFl = false;
	_forestSprite = _vm->_globals->freeMemory(_forestSprite);
	_curGestureFile = 0;
	_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
	_curObjectFileNum = 0;

	for (int idx = 0; idx < 6; ++idx)
		_hidingItemData[idx] = _vm->_globals->freeMemory(_hidingItemData[idx]);

	_objectDataBuf = _vm->_globals->freeMemory(_objectDataBuf);
	initVBob();
}

// Load Object
void ObjectsManager::loadObjects() {
	byte *data = _vm->_fileIO->loadFile("OBJET.DAT");
	byte *srcP = data;

	for (int idx = 0; idx < 300; ++idx) {
		ObjectAuthIcon *objectAuthIcon = &_objectAuthIcons[idx];
		objectAuthIcon->_objectFileNum = *srcP++;
		objectAuthIcon->_idx = *srcP++;
		objectAuthIcon->_flag1 = *srcP++;
		objectAuthIcon->_flag2 = *srcP++;
		objectAuthIcon->_flag3 = *srcP++;
		objectAuthIcon->_flag4 = *srcP++;
		objectAuthIcon->_flag5 = *srcP++;
		objectAuthIcon->_flag6 = *srcP++;
	}

	_vm->_globals->freeMemory(data);
}

// Reset Hiding Items
void ObjectsManager::resetHidingItems() {
	for (int idx = 1; idx <= 5; ++idx) {
		_hidingItemData[idx] = _vm->_globals->freeMemory(_hidingItemData[idx]);
	}

	for (int idx = 0; idx <= 20; ++idx) {
		HidingItem *hid = &_hidingItem[idx];
		hid->_spriteData = nullptr;
		hid->_x = 0;
		hid->_y = 0;
		hid->_spriteIndex = 0;
		hid->_useCount = 0;
		hid->_width = 0;
		hid->_height = 0;
		hid->_resetUseCount = false;
		hid->_yOffset = 0;
	}

	_hidingActiveFl = false;
}

/**
 * Change Object
 */
void ObjectsManager::changeObject(int objIndex) {
	_vm->_events->_objectBuf = loadObjectFromFile(objIndex, true);
	_curObjectIndex = objIndex;
}

byte *ObjectsManager::loadObjectFromFile(int objIndex, bool mode) {
	byte *dataP = nullptr;
	int objectFileNum = _objectAuthIcons[objIndex]._objectFileNum;
	int idx = _objectAuthIcons[objIndex]._idx;

	if (mode)
		++idx;

	if (objectFileNum != _curObjectFileNum) {
		if (_objectDataBuf)
			removeObjectDataBuf();
		if (objectFileNum == 1) {
			_objectDataBuf = loadSprite("OBJET1.SPR");
		}
		_curObjectFileNum = objectFileNum;
	}

	int width = getWidth(_objectDataBuf, idx);
	int height = getHeight(_objectDataBuf, idx);
	_objectWidth = width;
	_objectHeight = height;

	if (mode) {
		sprite_alone(_objectDataBuf, _vm->_events->_objectBuf, idx);
		dataP = _vm->_events->_objectBuf;
	} else {
		dataP = _vm->_globals->allocMemory(height * width);
		if (dataP == nullptr)
			error("CAPTURE_OBJET");

		capture_mem_sprite(_objectDataBuf, dataP, idx);
	}

	return dataP;
}

/**
 * Remove an Object from the inventory
 */
void ObjectsManager::removeObject(int objIndex) {
	int idx;
	for (idx = 1; idx <= 32; ++idx) {
		if (_vm->_globals->_inventory[idx] == objIndex)
			break;
	}

	if (idx <= 32) {
		if (idx == 32) {
			_vm->_globals->_inventory[32] = 0;
		} else {
			for (int i = idx; i < 32; ++i)
				_vm->_globals->_inventory[i] = _vm->_globals->_inventory[i + 1];
		}
	}
	changeObject(14);

}

/**
 * Set Offset XY
 */
void ObjectsManager::setOffsetXY(byte *data, int idx, int xp, int yp, bool isSize) {
	byte *startP = data + 3;
	for (int i = idx; i; --i)
		startP += READ_LE_UINT32(startP) + 16;

	byte *rectP = startP + 8;
	if (isSize) {
		// Set size
		byte *pointP = rectP + 4;
		WRITE_LE_UINT16(pointP, xp);
		WRITE_LE_UINT16(pointP + 2, yp);
	} else {
		// Set position
		WRITE_LE_UINT16(rectP, xp);
		WRITE_LE_UINT16(rectP + 2, yp);
	}
}

int ObjectsManager::getOffsetX(const byte *spriteData, int spriteIndex, bool isSize) {
	const byte *data = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		data += READ_LE_UINT32(data) + 16;

	int result;
	if (isSize)
		result = READ_LE_INT16(data + 12);
	else
		result = READ_LE_INT16(data + 8);

	return result;
}

int ObjectsManager::getOffsetY(const byte *spriteData, int spriteIndex, bool isSize) {
	const byte *data = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		data += READ_LE_UINT32(data) + 16;

	int result;
	if (isSize)
		result = READ_LE_INT16(data + 14);
	else
		result = READ_LE_INT16(data + 10);

	return result;
}

/**
 * Get Width
 */
int ObjectsManager::getWidth(const byte *objectData, int idx) {
	const byte *rectP = objectData + 3;
	for (int i = idx; i; --i)
		rectP += READ_LE_UINT32(rectP) + 16;

	return READ_LE_INT16(rectP + 4);
}

/**
 * Get height
 */
int ObjectsManager::getHeight(const byte *objectData, int idx) {
	const byte *rectP = objectData + 3;
	for (int i = idx; i; --i)
		rectP += READ_LE_UINT32(rectP) + 16;

	return READ_LE_INT16(rectP + 6);
}

void ObjectsManager::sprite_alone(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = READ_LE_INT16(objP) * READ_LE_INT16(objP + 2);

	memcpy(sprite + 3, objP - 4, result + 16);
}

void ObjectsManager::capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = READ_LE_INT16(objP) * READ_LE_INT16(objP + 2);
	memcpy(sprite, objP + 12, result);
}

void ObjectsManager::removeObjectDataBuf() {
	_curObjectFileNum = 0;
	_objectDataBuf = _vm->_globals->freeMemory(_objectDataBuf);
}

/**
 * Load Sprite from file
 */
byte *ObjectsManager::loadSprite(const Common::Path &file) {
	return _vm->_fileIO->loadFile(file);
}

/**
 * Add Object
 */
void ObjectsManager::addObject(int objIndex) {
	int arrIndex = 0;
	for (;;) {
		++arrIndex;
		if ((!_vm->_globals->_inventory[arrIndex]) || (arrIndex == 32))
			break;
	}

	_vm->_globals->_inventory[arrIndex] = objIndex;
}

/**
 * Display Sprite
 */
void ObjectsManager::displaySprite() {
	int clipX;
	int clipY;
	uint16 arr[50];

	// Handle copying any background areas that text are going to be drawn on
	_sortedDisplayCount = 0;
	for (int idx = 0; idx <= 10; ++idx) {
		TxtItemList *curTxtList = &_vm->_fontMan->_textList[idx];
		if (curTxtList->_enabledFl && _vm->_fontMan->_text[idx]._textType != 2) {
			clipX = curTxtList->_pos.x - 2;

			if (clipX < _vm->_graphicsMan->_minX)
				clipX = _vm->_graphicsMan->_minX;

			clipY = curTxtList->_pos.y - 2;
			if (clipY < _vm->_graphicsMan->_minY)
				clipY = _vm->_graphicsMan->_minY;

			_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, clipX, clipY,
				curTxtList->_width + 4, curTxtList->_height + 4, _vm->_graphicsMan->_frontBuffer, clipX, clipY);
			curTxtList->_enabledFl = false;
		}
	}

	if (!_charactersEnabledFl) {
		for (int idx = 0; idx < MAX_SPRITE; ++idx) {
			ListeItem *curList = &_liste[idx];
			if (curList->_visibleFl) {
				clipX = curList->_posX - 2;
				if (clipX < _vm->_graphicsMan->_minX)
					clipX = _vm->_graphicsMan->_minX;

				clipY = curList->_posY - 2;
				if (clipY < _vm->_graphicsMan->_minY)
					clipY = _vm->_graphicsMan->_minY;

				_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, clipX, clipY,
					curList->_width + 4, curList->_height + 4, _vm->_graphicsMan->_frontBuffer, clipX, clipY);
				curList->_visibleFl = false;
			}
		}
	}

	displayBobAnim();
	displayVBob();

	if (!_charactersEnabledFl) {
		// Handle drawing characters on the screen
		for (int idx = 0; idx < MAX_SPRITE; ++idx) {
			_liste[idx]._visibleFl = false;
			SpriteItem *curSpr = &_sprite[idx];
			if (curSpr->_animationType == 1) {
				computeSprite(idx);
				if (curSpr->_activeFl)
					beforeSort(SORT_SPRITE, idx, curSpr->_height + curSpr->_destY);
			}
		}

		if (_hidingActiveFl)
			checkHidingItem();
	}

	if (_priorityFl && _sortedDisplayCount) {
		for (int i = 1; i <= 48; i++)
			arr[i] = i;

		bool loopCondFl;
		do {
			loopCondFl = false;
			for (int sortIdx = 1; sortIdx < _sortedDisplayCount; sortIdx++) {
				if (_sortedDisplay[arr[sortIdx]]._priority > _sortedDisplay[arr[sortIdx + 1]]._priority) {
					SWAP(arr[sortIdx], arr[sortIdx + 1]);
					loopCondFl = true;
				}
			}
		} while (loopCondFl);

		for (int sortIdx = 1; sortIdx < _sortedDisplayCount + 1; sortIdx++) {
			int idx = arr[sortIdx];
			switch (_sortedDisplay[idx]._sortMode) {
			case SORT_BOB:
				setBobInfo(_sortedDisplay[idx]._index);
				break;
			case SORT_SPRITE:
				showSprite(_sortedDisplay[idx]._index);
				break;
			case SORT_HIDING:
				displayHiding(_sortedDisplay[idx]._index);
				break;
			default:
				break;
			}
			_sortedDisplay[idx]._sortMode = SORT_NONE;
		}
	} else {
		for (int idx = 1; idx < _sortedDisplayCount + 1; ++idx) {
			switch (_sortedDisplay[idx]._sortMode) {
			case SORT_BOB:
				setBobInfo(_sortedDisplay[idx]._index);
				break;
			case SORT_SPRITE:
				showSprite(_sortedDisplay[idx]._index);
				break;
			case SORT_HIDING:
				displayHiding(_sortedDisplay[idx]._index);
				break;
			default:
				break;
			}
			_sortedDisplay[idx]._sortMode = SORT_NONE;
		}
	}

	// Reset the Sort array
	for (int idx = 0; idx < 50; ++idx) {
		SortItem *disp = &_sortedDisplay[idx];
		disp->_sortMode = SORT_NONE;
		disp->_index = 0;
		disp->_priority = 0;
	}

	_sortedDisplayCount = 0;

	_vm->_dialog->drawInvent(_oldBorderPos, _oldBorderSpriteIndex, _borderPos, _borderSpriteIndex);

	if (_saveLoadFl) {
		int16 posX = _vm->_events->_startPos.x;
		_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, _saveLoadSprite, posX + 183, 60, 274, 353);
		if (_saveLoadX && _saveLoadY)
			_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _saveLoadSprite2, _saveLoadX + posX + 300, _saveLoadY + 300, 0);

		_vm->_graphicsMan->addDirtyRect(posX + 183, 60, posX + 457, 413);
	}

	// If the Options dialog is activated, draw the elements
	if (_vm->_globals->_optionDialogFl) {
		int16 posX = _vm->_events->_startPos.x;
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_globals->_optionDialogSpr,
			posX + 464, 407, 0);
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_globals->_optionDialogSpr,
			posX + 657, 556, _vm->_globals->_menuSpeed);
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_globals->_optionDialogSpr,
			posX + 731, 495, _vm->_globals->_menuTextOff);
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_globals->_optionDialogSpr,
			posX + 731, 468, _vm->_globals->_menuVoiceOff);
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_globals->_optionDialogSpr,
			posX + 731, 441, _vm->_globals->_menuSoundOff);
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_globals->_optionDialogSpr,
			posX + 731, 414, _vm->_globals->_menuMusicOff);
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_globals->_optionDialogSpr,
			posX + 600, 522, _vm->_globals->_menuDisplayType);
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _vm->_globals->_optionDialogSpr,
			posX + 611, 502, _vm->_globals->_menuScrollSpeed);
		_vm->_graphicsMan->addDirtyRect(posX + 164, 107, posX + 498, 320);
	}

	// Loop to draw any on-screen text
	for (int idx = 0; idx <= 10; ++idx) {
		TxtItem *curTxt = &_vm->_fontMan->_text[idx];
		if (curTxt->_textOnFl) {
			TxtItemList *curTxtList = &_vm->_fontMan->_textList[idx];
			if ((curTxt->_textType < 2) || (curTxt->_textType > 3))
				_vm->_fontMan->box(idx, curTxt->_messageId, curTxt->_filename, _vm->_events->_startPos.x + curTxt->_pos.x, curTxt->_pos.y);
			else
				_vm->_fontMan->box(idx, curTxt->_messageId, curTxt->_filename, curTxt->_pos.x, curTxt->_pos.y);
			curTxtList->_enabledFl = true;

			if ((curTxt->_textType < 2) || (curTxt->_textType > 3))
				curTxtList->_pos.x = _vm->_events->_startPos.x + curTxt->_pos.x;
			else
				curTxtList->_pos.x = curTxt->_pos.x;

			curTxtList->_pos.y = curTxt->_pos.y;
			curTxtList->_width = curTxt->_width;
			curTxtList->_height = curTxt->_height;

			if (curTxtList->_pos.x < _vm->_graphicsMan->_minX)
				curTxtList->_pos.x = _vm->_graphicsMan->_minX - 1;
			if (curTxtList->_pos.y < _vm->_graphicsMan->_minY)
				curTxtList->_pos.y = _vm->_graphicsMan->_minY - 1;

			int posX = curTxtList->_pos.x;
			if (curTxtList->_width + posX > _vm->_graphicsMan->_maxX)
				curTxtList->_width = _vm->_graphicsMan->_maxX - posX;
			int posY = curTxtList->_pos.y;
			if (curTxtList->_height + posY > _vm->_graphicsMan->_maxY)
				curTxtList->_height = _vm->_graphicsMan->_maxY - posY;
			if (curTxtList->_width <= 0 || curTxtList->_height <= 0)
				curTxtList->_enabledFl = false;
		}
	}

	_vm->_dialog->inventAnim();
}

void ObjectsManager::resetBob(int idx) {
	BobItem &bob = _bob[idx];
	ListeItem &item = _liste2[idx];

	bob._bobMode = 0;
	bob._spriteData = nullptr;
	bob._xp = 0;
	bob._yp = 0;
	bob._frameIndex = 0;
	bob._animDataIdx = 0;
	bob._moveChange1 = 0;
	bob._moveChange2 = 0;
	bob._disabledAnimationFl = false;
	bob._animData = nullptr;
	bob._bobMode10 = false;
	bob._bobModeChange = 0;
	bob._modeChangeCtr = 0;
	bob._modeChangeUnused = 0;
	bob._disableFl = false;
	bob._zoomFactor = 0;
	bob._flipFl = false;
	bob._oldX2 = 0;

	item._visibleFl = false;
	item._posX = 0;
	item._posY = 0;
	item._width = 0;
	item._height = 0;
}

void ObjectsManager::setBobInfo(int idx) {
	BobItem *curBob = &_bob[idx];

	if (!curBob->_activeFl)
		return;

	int xp = curBob->_oldX;
	int yp = curBob->_oldY;

	if (curBob->_isSpriteFl)
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, curBob->_spriteData,
			xp + 300, yp + 300, curBob->_frameIndex);
	else
		_vm->_graphicsMan->drawCompressedSprite(_vm->_graphicsMan->_frontBuffer,
			curBob->_spriteData, xp + 300, yp + 300, curBob->_frameIndex,
			curBob->_zoomOutFactor, curBob->_zooInmFactor, curBob->_flipFl);

	ListeItem *curLst = &_liste2[idx];
	curLst->_visibleFl = true;
	curLst->_posX = xp;
	curLst->_posY = yp;

	curLst->_width = curBob->_oldWidth;
	curLst->_height = curBob->_oldHeight;

	if (curLst->_posX < _vm->_graphicsMan->_minX) {
		curLst->_width -= _vm->_graphicsMan->_minX - curLst->_posX;
		curLst->_posX = _vm->_graphicsMan->_minX;
	}

	if (curLst->_posY < _vm->_graphicsMan->_minY) {
		curLst->_height -= _vm->_graphicsMan->_minY - curLst->_posY;
		curLst->_posY = _vm->_graphicsMan->_minY;
	}

	if (curLst->_width + curLst->_posX > _vm->_graphicsMan->_maxX)
		curLst->_width = _vm->_graphicsMan->_maxX - curLst->_posX;

	if (curLst->_height + curLst->_posY > _vm->_graphicsMan->_maxY)
		curLst->_height = _vm->_graphicsMan->_maxY - curLst->_posY;

	if (curLst->_width <= 0 || curLst->_height <= 0)
		curLst->_visibleFl = false;

	if (curLst->_visibleFl)
		_vm->_graphicsMan->addDirtyRect(curLst->_posX, curLst->_posY, curLst->_posX + curLst->_width, curLst->_posY + curLst->_height);
}

void ObjectsManager::displayBob(int idx) {
	BobItem *curBob = &_bob[idx];

	_priorityFl = true;

	if (curBob->_bobMode)
		return;

	resetBob(idx);

	const byte *data = _vm->_animMan->_animBqe[idx]._data;
	int bankIdx = READ_LE_INT16(data);
	if (!bankIdx)
		return;
	if ((!_vm->_animMan->Bank[bankIdx]._loadedFl) || (!READ_LE_UINT16(data + 24)))
		return;


	int16 bobModeChange = READ_LE_INT16(data + 2);
	int16 modeChangeUnused = READ_LE_INT16(data + 4);
	// data[6] isn't used, read skipped
	int16 newModeChangeCtr = READ_LE_INT16(data + 8);

	if (!bobModeChange)
		bobModeChange = 1;
	if (!newModeChangeCtr)
		newModeChangeCtr = -1;

	curBob->_isSpriteFl = false;

	if (_vm->_animMan->Bank[bankIdx]._fileHeader == 1) {
		curBob->_isSpriteFl = true;
		curBob->_zoomFactor = 0;
		curBob->_flipFl = false;
	}

	curBob->_animData = _vm->_animMan->_animBqe[idx]._data;
	curBob->_bobMode = 10;
	curBob->_spriteData = _vm->_animMan->Bank[bankIdx]._data;

	curBob->_bobModeChange = bobModeChange;
	curBob->_modeChangeCtr = newModeChangeCtr;
	curBob->_modeChangeUnused = modeChangeUnused;
}

void ObjectsManager::hideBob(int idx) {
	BobItem *curBob = &_bob[idx];
	if ((curBob->_bobMode == 3) || (curBob->_bobMode == 10))
		curBob->_bobMode++;
}

void ObjectsManager::setBobOffset(int idx, int offset) {
	_bob[idx]._oldX2 = offset;
}

void ObjectsManager::computeHideCounter(int idx) {
	HidingItem *hid = &_hidingItem[idx];
	if (hid->_useCount == 0)
		return;

	for (int i = 0; i <= 20; i++) {
		BobItem *curBob = &_bob[i];
		if ((curBob->_bobMode) && (!curBob->_disabledAnimationFl) && (!curBob->_disableFl) && (curBob->_frameIndex != 250)) {
			int oldRight = curBob->_oldX + curBob->_oldWidth;
			int oldBottom = curBob->_oldY + curBob->_oldHeight;
			int hiddenRight = hid->_x + hid->_width;

			if ((oldBottom > hid->_y) && (oldBottom < hid->_yOffset + hid->_height + hid->_y)) {
				if ((oldRight >= hid->_x && oldRight <= hiddenRight)
				// CHECKME: The original was doing the test two times. This looks like an
				// original bug
				// || (cachedRight >= curBob->_oldWidth && curBob->_oldWidth >= hid->_x)
				 || (hiddenRight >= curBob->_oldWidth && curBob->_oldWidth >= hid->_x)
				 || (curBob->_oldWidth >= hid->_x && oldRight <= hiddenRight)
				 || (curBob->_oldWidth <= hid->_x && oldRight >= hiddenRight))
					++hid->_useCount;
			}
		}
	}
}

void ObjectsManager::initBobVariables(int idx) {
	BobItem *bob = &_bob[idx];

	bob->_activeFl = false;
	if (bob->_isSpriteFl) {
		bob->_flipFl = false;
		bob->_zoomFactor = 0;
	}

	int spriteIdx = bob->_frameIndex;
	if (spriteIdx == 250)
		return;

	int deltaY, deltaX;
	if (bob->_flipFl) {
		deltaX = getOffsetX(bob->_spriteData, spriteIdx, true);
		deltaY = getOffsetY(bob->_spriteData, bob->_frameIndex, true);
	} else {
		deltaX = getOffsetX(bob->_spriteData, spriteIdx, false);
		deltaY = getOffsetY(bob->_spriteData, bob->_frameIndex, false);
	}

	int negZoom = 0;
	int posZoom = 0;
	if (bob->_zoomFactor < 0)
		negZoom = CLIP(-bob->_zoomFactor, 0, 95);
	else
		posZoom = bob->_zoomFactor;

	if (posZoom) {
		if (deltaX >= 0)
			deltaX = _vm->_graphicsMan->zoomIn(deltaX, posZoom);
		else
			deltaX = -_vm->_graphicsMan->zoomIn(-deltaX, posZoom);

		if (deltaY >= 0)
			deltaY = _vm->_graphicsMan->zoomIn(deltaY, posZoom);
		else
			deltaY = -_vm->_graphicsMan->zoomIn(abs(deltaX), posZoom);
	}

	if (negZoom) {
		if (deltaX >= 0)
			deltaX = _vm->_graphicsMan->zoomOut(deltaX, negZoom);
		else
			deltaX = -_vm->_graphicsMan->zoomOut(-deltaX, negZoom);

		if (deltaY >= 0)
			deltaY = _vm->_graphicsMan->zoomOut(deltaY, negZoom);
		else
			deltaY = -_vm->_graphicsMan->zoomOut(abs(deltaX), negZoom);
	}

	int newX = bob->_xp - deltaX;
	int newY = bob->_yp - deltaY;
	bob->_activeFl = true;
	bob->_oldX = newX;
	bob->_oldY = newY;
	bob->_zooInmFactor = posZoom;
	bob->_zoomOutFactor = negZoom;

	ListeItem *curList = &_liste2[idx];
	curList->_visibleFl = true;
	curList->_posX = newX;
	curList->_posY = newY;

	int width = getWidth(bob->_spriteData, bob->_frameIndex);
	int height = getHeight(bob->_spriteData, bob->_frameIndex);

	if (posZoom) {
		width = _vm->_graphicsMan->zoomIn(width, posZoom);
		height = _vm->_graphicsMan->zoomIn(height, posZoom);
	} else if (negZoom) {
		width = _vm->_graphicsMan->zoomOut(width, negZoom);
		height = _vm->_graphicsMan->zoomOut(height, negZoom);
	}

	curList->_width = width;
	curList->_height = height;
	bob->_oldWidth = width;
	bob->_oldHeight = height;
}

void ObjectsManager::checkHidingItem() {
	for (int hidingItemIdx = 0; hidingItemIdx <= 19; hidingItemIdx++) {
		HidingItem *hid = &_hidingItem[hidingItemIdx];
		if (hid->_useCount == 0)
			continue;

		int _oldUseCount = hid->_useCount;
		for (int spriteIdx = 0; spriteIdx <= 4; spriteIdx++) {
			const SpriteItem *spr = &_sprite[spriteIdx];
			if (spr->_animationType == 1 && spr->_spriteIndex != 250) {
				int right = spr->_width + spr->_destX;
				int bottom = spr->_height + spr->_destY;
				int hidingRight = hid->_width + hid->_x;

				if (bottom > hid->_y && bottom < (hid->_yOffset + hid->_height + hid->_y)) {
					if ((right >= hid->_x && right <= hidingRight)
					// CHECKME: The original was doing the test two times. This looks like an
					// original bug
					// || (hidingRight >= spr->_destX && hid->_x <= spr->_destX)
					 || (hidingRight >= spr->_destX && hid->_x <= spr->_destX)
					 || (hid->_x <= spr->_destX && right <= hidingRight)
					 || (hid->_x >= spr->_destX && right >= hidingRight))
						++hid->_useCount;
				}
			}
		}

		computeHideCounter(hidingItemIdx);
		if (hid->_useCount != _oldUseCount) {
			int priority = hid->_yOffset + hid->_height + hid->_y;
			if (priority > 440)
				priority = 500;

			beforeSort(SORT_HIDING, hidingItemIdx, priority);
			hid->_useCount = 1;
			hid->_resetUseCount = true;
		} else if (hid->_resetUseCount) {
			hid->_resetUseCount = false;
			hid->_useCount = 1;
		}

	}
}

void ObjectsManager::showSprite(int idx) {
	SpriteItem *spr = &_sprite[idx];
	if (!spr->_activeFl)
		return;

	if (spr->_rleFl)
		_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, spr->_spriteData,
		    spr->_destX + 300, spr->_destY + 300, spr->_spriteIndex);
	else
		_vm->_graphicsMan->drawCompressedSprite(_vm->_graphicsMan->_frontBuffer, spr->_spriteData,
		    spr->_destX + 300, spr->_destY + 300,  spr->_spriteIndex, spr->_reducePct, spr->_zoomPct, spr->_flipFl);

	ListeItem *list = &_liste[idx];
	list->_width = spr->_width;
	list->_height = spr->_height;

	if (list->_posX < _vm->_graphicsMan->_minX) {
		list->_width -= _vm->_graphicsMan->_minX - list->_posX;
		list->_posX = _vm->_graphicsMan->_minX;
	}

	if (list->_posY < _vm->_graphicsMan->_minY) {
		list->_height -= _vm->_graphicsMan->_minY - list->_posY;
		list->_posY = _vm->_graphicsMan->_minY;
	}

	list->_width = MIN(list->_width, _vm->_graphicsMan->_maxX - list->_posX);
	list->_height = MIN(list->_height, _vm->_graphicsMan->_maxY - list->_posY);

	if (list->_width <= 0 || list->_height <= 0)
		list->_visibleFl = false;

	if (list->_visibleFl)
		_vm->_graphicsMan->addDirtyRect( list->_posX, list->_posY, list->_posX + list->_width, list->_posY + list->_height);
}

void ObjectsManager::displayHiding(int idx) {
	HidingItem *hid = &_hidingItem[idx];

	_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, _hidingItemData[1],
		hid->_x + 300, hid->_y + 300, hid->_spriteIndex);
	_vm->_graphicsMan->addDirtyRect(hid->_x, hid->_y, hid->_x + hid->_width, hid->_y + hid->_height);
}

// Compute Sprite
void ObjectsManager::computeSprite(int idx) {
	SpriteItem *spr = &_sprite[idx];

	spr->_activeFl = false;
	int spriteIndex = spr->_spriteIndex;
	if (spriteIndex == 250)
		return;

	int offX;
	int offY;
	if (spr->_flipFl) {
		offX = getOffsetX(spr->_spriteData, spriteIndex, true);
		offY = getOffsetY(spr->_spriteData, spr->_spriteIndex, true);
	} else {
		offX = getOffsetX(spr->_spriteData, spriteIndex, false);
		offY = getOffsetY(spr->_spriteData, spr->_spriteIndex, false);
	}

	int tmpX = spr->_deltaX + offX;
	int deltaX = tmpX;
	int tmpY = spr->_deltaY + offY;
	int deltaY = tmpY;
	int zoomPercent = 0;
	int reducePercent = 0;

	if (spr->_zoomFactor < 0) {
		reducePercent = -spr->_zoomFactor;
		if (reducePercent > 95)
			reducePercent = 95;
	} else
		zoomPercent = spr->_zoomFactor;

	if (zoomPercent) {
		if (tmpX >= 0)
			deltaX = _vm->_graphicsMan->zoomIn(tmpX, zoomPercent);
		else
			deltaX = -_vm->_graphicsMan->zoomIn(-tmpX, zoomPercent);

		if (tmpY >= 0) {
			deltaY = _vm->_graphicsMan->zoomIn(tmpY, zoomPercent);
		} else {
			tmpY = abs(tmpX);
			deltaY = -_vm->_graphicsMan->zoomIn(tmpY, zoomPercent);
		}
	} else if (reducePercent) {
		if (tmpX >= 0)
			deltaX = _vm->_graphicsMan->zoomOut(tmpX, reducePercent);
		else
			deltaX = -_vm->_graphicsMan->zoomOut(-tmpX, reducePercent);

		if (tmpY >= 0) {
			deltaY = _vm->_graphicsMan->zoomOut(tmpY, reducePercent);
		} else {
			tmpY = abs(tmpX);
			deltaY = -_vm->_graphicsMan->zoomOut(tmpY, reducePercent);
		}
	}

	int newPosX = spr->_spritePos.x - deltaX;
	int newPosY = spr->_spritePos.y - deltaY;
	spr->_destX = newPosX;
	spr->_destY = newPosY;
	spr->_activeFl = true;
	spr->_zoomPct = zoomPercent;
	spr->_reducePct = reducePercent;

	_liste[idx]._visibleFl = true;
	_liste[idx]._posX = newPosX;
	_liste[idx]._posY = newPosY;

	int width = getWidth(spr->_spriteData, spr->_spriteIndex);
	int height = getHeight(spr->_spriteData, spr->_spriteIndex);

	if (zoomPercent) {
		width = _vm->_graphicsMan->zoomIn(width, zoomPercent);
		height = _vm->_graphicsMan->zoomIn(height, zoomPercent);
	} else if (reducePercent) {
		height = _vm->_graphicsMan->zoomOut(height, reducePercent);
		width = _vm->_graphicsMan->zoomOut(width, reducePercent);
	}

	spr->_width = width;
	spr->_height = height;
}

// Before Sort
void ObjectsManager::beforeSort(SortMode sortMode, int index, int priority) {
	++_sortedDisplayCount;
	assert(_sortedDisplayCount <= 48);

	_sortedDisplay[_sortedDisplayCount]._sortMode = sortMode;
	_sortedDisplay[_sortedDisplayCount]._index = index;
	_sortedDisplay[_sortedDisplayCount]._priority = priority;
}

// Display BOB Anim
void ObjectsManager::displayBobAnim() {
	for (int idx = 1; idx <= 35; idx++) {
		BobItem *bob = &_bob[idx];
		if (idx <= 20 && _charactersEnabledFl) {
			bob->_bobMode10 = false;
			continue;
		}

		if (bob->_bobMode != 10)
			continue;

		bob->_bobMode10 = false;
		if (bob->_animData == nullptr || bob->_disabledAnimationFl || bob->_modeChangeCtr == 0 || bob->_modeChangeCtr < -1) {
			if (bob->_bobModeChange == 1 || bob->_bobModeChange == 2)
				bob->_bobMode10 = true;
			continue;
		}

		if (bob->_moveChange1 == bob->_moveChange2) {
			bob->_bobMode10 = true;
		} else {
			bob->_moveChange2++;
			bob->_bobMode10 = false;
		}

		if (!bob->_bobMode10) {
			if (bob->_bobModeChange == 1 || bob->_bobModeChange == 2)
				bob->_bobMode10 = true;
			continue;
		}

		byte *dataPtr = bob->_animData + 20;
		int dataIdx = bob->_animDataIdx;
		bob->_xp = READ_LE_INT16(dataPtr + 2 * dataIdx);
		if (_lockedAnims[idx]._enableFl)
			bob->_xp = _lockedAnims[idx]._posX;
		if ( _charactersEnabledFl && idx > 20)
			bob->_xp += _vm->_events->_startPos.x;

		bob->_yp = READ_LE_INT16(dataPtr + 2 * dataIdx + 2);
		bob->_moveChange1 = READ_LE_INT16(dataPtr + 2 * dataIdx + 4);
		bob->_zoomFactor = READ_LE_INT16(dataPtr + 2 * dataIdx + 6);
		bob->_frameIndex = dataPtr[2 * dataIdx + 8];
		bob->_flipFl = (dataPtr[2 * dataIdx + 9] != 0);
		bob->_animDataIdx += 5;

		if (bob->_moveChange1 > 0) {
			bob->_moveChange1 /= _vm->_globals->_speed;
			if (bob->_moveChange1 > 0) {
				bob->_moveChange2 = 1;
				if (bob->_bobModeChange == 1 || bob->_bobModeChange == 2)
					bob->_bobMode10 = true;
				continue;
			}

			bob->_moveChange1 = 1;
		}
		if (!bob->_moveChange1) {
			if (bob->_modeChangeCtr > 0)
				bob->_modeChangeCtr--;
			if (bob->_modeChangeCtr != -1 && bob->_modeChangeCtr <= 0) {
				bob->_bobMode = 11;
			} else {
				bob->_animDataIdx = 0;
				byte *bobData = bob->_animData + 20;
				bob->_xp = READ_LE_INT16(bobData);

				if (_lockedAnims[idx]._enableFl)
					bob->_xp = _lockedAnims[idx]._posX;
				if (_charactersEnabledFl && idx > 20)
					bob->_xp += _vm->_events->_startPos.x;

				bob->_yp = READ_LE_INT16(bobData + 2);
				bob->_moveChange1 = READ_LE_INT16(bobData + 4);
				bob->_zoomFactor = READ_LE_INT16(bobData + 6);
				bob->_frameIndex = bobData[8];
				bob->_flipFl = (bobData[9] != 0);
				bob->_animDataIdx += 5;

				if (bob->_moveChange1 > 0) {
					bob->_moveChange1 /= _vm->_globals->_speed;
					// Original code. It can't be negative, so the check is on == 0
					if (bob->_moveChange1 <= 0)
						bob->_moveChange1 = 1;
				}
			}
		}

		bob->_moveChange2 = 1;
		if (bob->_bobModeChange == 1 || bob->_bobModeChange == 2)
			bob->_bobMode10 = true;
	}

	if (!_charactersEnabledFl && _refreshBobMode10Fl) {
		for (int i = 0; i < 35; i++) {
			BobItem *curBob = &_bob[i];
			if (curBob->_bobMode == 10 && !curBob->_disabledAnimationFl)
				curBob->_bobMode10 = true;
		}
	}

	_refreshBobMode10Fl = false;

	for (int i = 1; i <= 35; i++) {
		BobItem *curBob = &_bob[i];
		ListeItem *curList = &_liste2[i];
		if (i > 20 || !_charactersEnabledFl) {
			if ((curBob->_bobMode == 10) && (curBob->_bobMode10)) {
				if ((curBob->_bobModeChange != 2) && (curBob->_bobModeChange != 4)) {
					if (curList->_visibleFl) {
						_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, curList->_posX, curList->_posY,
							curList->_width, curList->_height, _vm->_graphicsMan->_frontBuffer, curList->_posX, curList->_posY);
						curList->_visibleFl = false;
					}
				}
			}

			if (curBob->_bobMode == 11) {
				if (curList->_visibleFl) {
					_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, curList->_posX, curList->_posY,
						curList->_width, curList->_height, _vm->_graphicsMan->_frontBuffer, curList->_posX, curList->_posY);
					curList->_visibleFl = false;
				}

				curBob->_bobMode = 0;
			}
		}
	}

	for (int i = 1; i <= 35; i++) {
		BobItem *curBob = &_bob[i];
		curBob->_oldY = 0;
		if (curBob->_bobMode == 10 && !curBob->_disabledAnimationFl && curBob->_bobMode10) {
			initBobVariables(i);
			int priority = curBob->_oldX2 + curBob->_oldHeight + curBob->_oldY;

			if (priority > 450)
				priority = 600;

			if (curBob->_activeFl)
				beforeSort(SORT_BOB, i, priority);
		}
	}
}

// Display VBOB
void ObjectsManager::displayVBob() {
	int width, height;

	for (int idx = 0; idx <= 29; idx++) {
		VBobItem *vbob = &_vBob[idx];
		if (vbob->_displayMode == 4) {
			width = getWidth(vbob->_spriteData, vbob->_frameIndex);
			height = getHeight(vbob->_spriteData, vbob->_frameIndex);

			_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_backBuffer, vbob->_surface,
				vbob->_xp, vbob->_yp, width, height);

			_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, vbob->_surface,
				vbob->_xp, vbob->_yp, width, height);

			_vm->_graphicsMan->addDirtyRect(vbob->_xp, vbob->_yp, vbob->_xp + width, height + vbob->_yp);
			vbob->_surface = _vm->_globals->freeMemory(vbob->_surface);

			vbob->_displayMode = 0;
			vbob->_spriteData = nullptr;
			vbob->_xp = 0;
			vbob->_yp = 0;
			vbob->_oldX = 0;
			vbob->_oldY = 0;
			vbob->_frameIndex = 0;
			vbob->_oldFrameIndex = 0;
			vbob->_oldSpriteData = nullptr;
		}

		if (vbob->_displayMode == 3) {
			width = getWidth(vbob->_oldSpriteData, vbob->_oldFrameIndex);
			height = getHeight(vbob->_oldSpriteData, vbob->_oldFrameIndex);

			_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_backBuffer, vbob->_surface,
				vbob->_oldX, vbob->_oldY, width, height);

			_vm->_graphicsMan->restoreSurfaceRect(_vm->_graphicsMan->_frontBuffer, vbob->_surface,
				vbob->_oldX, vbob->_oldY, width, height);

			_vm->_graphicsMan->addDirtyRect(vbob->_oldX, vbob->_oldY, vbob->_oldX + width, vbob->_oldY + height);

			vbob->_displayMode = 1;
			vbob->_oldSpriteData = vbob->_spriteData;

			vbob->_surface = _vm->_globals->freeMemory(vbob->_surface);

			vbob->_oldX = vbob->_xp;
			vbob->_oldY = vbob->_yp;
			vbob->_oldFrameIndex = vbob->_frameIndex;
		}

		if (vbob->_displayMode == 1) {
			width = getWidth(vbob->_spriteData, vbob->_frameIndex);
			height = getHeight(vbob->_spriteData, vbob->_frameIndex);

			vbob->_surface = _vm->_globals->freeMemory(vbob->_surface);

			byte *surface = _vm->_globals->allocMemory(height * width);
			vbob->_surface = surface;

			_vm->_graphicsMan->copySurfaceRect(_vm->_graphicsMan->_backBuffer, surface,
				vbob->_xp, vbob->_yp, width, height);

			if (*vbob->_spriteData == 78) {
				_vm->_graphicsMan->drawCompressedSprite(_vm->_graphicsMan->_backBuffer, vbob->_spriteData,
					vbob->_xp + 300, vbob->_yp + 300, vbob->_frameIndex, 0, 0, false);

				_vm->_graphicsMan->drawCompressedSprite(_vm->_graphicsMan->_frontBuffer, vbob->_spriteData,
					vbob->_xp + 300, vbob->_yp + 300, vbob->_frameIndex, 0, 0, false);
			} else {
				_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_frontBuffer, vbob->_spriteData,
					vbob->_xp + 300, vbob->_yp + 300, vbob->_frameIndex);

				_vm->_graphicsMan->drawVesaSprite(_vm->_graphicsMan->_backBuffer, vbob->_spriteData,
					vbob->_xp + 300, vbob->_yp + 300, vbob->_frameIndex);
			}

			_vm->_graphicsMan->addDirtyRect(vbob->_xp, vbob->_yp , vbob->_xp + width, vbob->_yp + height);
			vbob->_displayMode = 2;
		}
	}
}

/**
 * Get Sprite X coordinate
 */
int ObjectsManager::getSpriteX(int idx) {
	assert(idx  <= MAX_SPRITE);
	return _sprite[idx]._spritePos.x;
}

/**
 * Get Sprite Y coordinate
 */
int ObjectsManager::getSpriteY(int idx) {
	assert(idx  <= MAX_SPRITE);
	return _sprite[idx]._spritePos.y;
}

/**
 * Clear sprite structure
 */
void ObjectsManager::clearSprite() {
	for (int idx = 0; idx < MAX_SPRITE; idx++) {
		_sprite[idx]._spriteData = nullptr;
		_sprite[idx]._animationType = 0;
	}

	for (int idx = 0; idx < MAX_SPRITE; idx++) {
		ListeItem *list = &_liste[idx];
		list->_visibleFl = false;
		list->_posX = 0;
		list->_posY = 0;
		list->_width = 0;
		list->_height = 0;
	}
}

void ObjectsManager::animateSprite(int idx) {
	assert(idx  <= MAX_SPRITE);
	_sprite[idx]._animationType = 1;
}

void ObjectsManager::addStaticSprite(const byte *spriteData, Common::Point pos, int idx, int spriteIndex, int zoomFactor, bool flipFl, int deltaX, int deltaY) {
	assert(idx  <= MAX_SPRITE);

	SpriteItem *spr = &_sprite[idx];
	spr->_spriteData = spriteData;
	spr->_spritePos = pos;
	spr->_spriteIndex = spriteIndex;
	spr->_zoomFactor = zoomFactor;
	spr->_flipFl = flipFl;
	spr->_deltaX = deltaX;
	spr->_deltaY = deltaY;
	spr->_animationType = 0;

	if (READ_BE_UINT24(spriteData) == MKTAG24('R', 'L', 'E')) {
		spr->_rleFl = true;
		spr->_zoomFactor = 0;
		spr->_flipFl = false;
	} else
		spr->_rleFl = false;

}

/**
 * Freeze sprite animation and free its memory
 */
void ObjectsManager::removeSprite(int idx) {
	// Type 3 was also used by freeSprite(), which has been removed as it wasn't used
	_sprite[idx]._animationType = 3;
}

/**
 * Set Sprite X coordinate
 */
void ObjectsManager::setSpriteX(int idx, int xp) {
	assert(idx  <= MAX_SPRITE);
	_sprite[idx]._spritePos.x = xp;
}

/**
 * Set Sprite Y coordinate
 */
void ObjectsManager::setSpriteY(int idx, int yp) {
	assert(idx  <= MAX_SPRITE);
	_sprite[idx]._spritePos.y = yp;
}

/**
 * Set Sprite Index
 */
void ObjectsManager::setSpriteIndex(int idx, int spriteIndex) {
	assert(idx  <= MAX_SPRITE);
	_sprite[idx]._spriteIndex = spriteIndex;
}

// Set Sprite Size
void ObjectsManager::setSpriteZoom(int idx, int zoomFactor) {
	assert(idx  <= MAX_SPRITE);
	if (!_sprite[idx]._rleFl)
		_sprite[idx]._zoomFactor = zoomFactor;
}

void ObjectsManager::setFlipSprite(int idx, bool flipFl) {
	assert(idx  <= MAX_SPRITE);
	if (!_sprite[idx]._rleFl)
		_sprite[idx]._flipFl = flipFl;
}

void ObjectsManager::goHome() {
	if (_vm->_linesMan->_route == nullptr)
		return;

	if (_homeRateCounter > 1) {
		--_homeRateCounter;
		return;
	}

	int newPosX;
	int newPosY;
	Directions newDirection;

	int oldPosX = 0;
	int oldPosY = 0;
	int oldFrameIdx = 0;
	_homeRateCounter = 0;
	if (_oldDirection == DIR_NONE) {
		computeAndSetSpriteSize();
		newPosX = _vm->_linesMan->_route->_x;
		newPosY = _vm->_linesMan->_route->_y;
		newDirection = _vm->_linesMan->_route->_dir;
		_vm->_linesMan->_route++;

		if (newPosX != -1 || newPosY != -1) {
			_oldDirection = newDirection;
			_oldDirectionSpriteIdx = newDirection + 59;
			_oldFrameIndex = 0;
			_oldCharacterPosX = newPosX;
			_oldCharacterPosY = newPosY;
		} else {
			setSpriteIndex(0, _oldDirection + 59);
			_vm->_globals->_actionDirection = DIR_NONE;
			int zoneId;
			if (_vm->_globals->_actionMoveTo)
				zoneId = _vm->_globals->_saveData->_data[svLastZoneNum];
			else
				zoneId = _zoneNum;
			_vm->_linesMan->_route = nullptr;
			computeAndSetSpriteSize();
			setFlipSprite(0, false);
			_homeRateCounter = 0;
			_vm->_linesMan->_route = nullptr;
			_oldDirection = DIR_NONE;
			if (zoneId > 0) {
				ZoneItem *curZone = &_vm->_linesMan->_zone[zoneId];
				if (curZone->_destX && curZone->_destY && curZone->_destY != 31) {
					if (curZone->_spriteIndex == -1) {
						curZone->_destX = 0;
						curZone->_destY = 0;
						curZone->_spriteIndex = 0;
					} else {
						setSpriteIndex(0, curZone->_spriteIndex);
						_vm->_globals->_actionDirection = curZone->_spriteIndex - 59;
					}
				}
			}
		}
		_homeRateCounter = 0;
		return;
	}
	if (_oldDirection == DIR_RIGHT) {
		if (_oldFrameIndex < 24 || _oldFrameIndex > 35) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 24;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedY;

			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsMan->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsMan->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX + deltaX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _oldFrameIndex + 1;
			if (oldFrameIdx > 35)
				oldFrameIdx = 24;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_oldDirection == DIR_LEFT) {
		if (_oldFrameIndex < 24 || _oldFrameIndex > 35) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 24;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsMan->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsMan->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX - deltaX;
			oldPosY = _oldCharacterPosY - deltaY;
			oldFrameIdx = _oldFrameIndex + 1;
			if (oldFrameIdx > 35)
				oldFrameIdx = 24;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_oldDirection == DIR_UP) {
		if (_oldFrameIndex > 11) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 0;
		} else {
			int deltaY = abs(_vm->_globals->_hopkinsItem[_oldFrameIndex]._speedY);
			if (_sprite[0]._zoomFactor < 0) {
				deltaY = _vm->_graphicsMan->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaY = _vm->_graphicsMan->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY - deltaY;
			oldFrameIdx = _oldFrameIndex + 1;
			if (oldFrameIdx > 11)
				oldFrameIdx = 0;
		}
		_homeRateCounter = 4 / _vm->_globals->_speed;
	}

	if (_oldDirection == DIR_DOWN) {
		if (_oldFrameIndex < 48 || _oldFrameIndex > 59) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 48;
		} else {
			int deltaY = abs(_vm->_globals->_hopkinsItem[_oldFrameIndex]._speedY);
			if (_sprite[0]._zoomFactor < 0) {
				deltaY = _vm->_graphicsMan->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaY = _vm->_graphicsMan->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX;
			oldPosY = deltaY + _oldCharacterPosY;
			oldFrameIdx = _oldFrameIndex + 1;
			if (oldFrameIdx > 59)
				oldFrameIdx = 48;
		}
		_homeRateCounter = 4 / _vm->_globals->_speed;
	}
	if (_oldDirection == DIR_UP_RIGHT) {
		if (_oldFrameIndex < 12 || _oldFrameIndex > 23) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 12;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsMan->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			}
			if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsMan->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = deltaX + _oldCharacterPosX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _oldFrameIndex + 1;
			if (oldFrameIdx > 23)
				oldFrameIdx = 12;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_oldDirection == DIR_UP_LEFT) {
		if (_oldFrameIndex < 12 || _oldFrameIndex > 23) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 12;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsMan->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsMan->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX - deltaX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _oldFrameIndex + 1;
			if (oldFrameIdx > 23)
				oldFrameIdx = 12;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_oldDirection == DIR_DOWN_RIGHT) {
		if (_oldFrameIndex < 36 || _oldFrameIndex > 47) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 36;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsMan->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			}
			if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsMan->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = deltaX + _oldCharacterPosX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _oldFrameIndex + 1;
			if (oldFrameIdx > 47)
				oldFrameIdx = 36;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_oldDirection == DIR_DOWN_LEFT) {
		if (_oldFrameIndex < 36 || _oldFrameIndex > 47) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 36;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsMan->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			}
			if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsMan->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsMan->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX - deltaX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _oldFrameIndex + 1;
			if (oldFrameIdx > 47)
				oldFrameIdx = 36;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	bool loopCond = false;
	do {
		newPosX = _vm->_linesMan->_route->_x;
		newPosY = _vm->_linesMan->_route->_y;
		newDirection = (Directions)_vm->_linesMan->_route->_dir;
		_vm->_linesMan->_route++;

		if (newPosX == -1 && newPosY == -1) {
			int zoneId;
			if (_vm->_globals->_actionMoveTo)
				zoneId = _vm->_globals->_saveData->_data[svLastZoneNum];
			else
				zoneId = _zoneNum;
			setSpriteIndex(0, _oldDirection + 59);
			_vm->_globals->_actionDirection = DIR_NONE;
			_vm->_linesMan->_route = nullptr;
			computeAndSetSpriteSize();
			setFlipSprite(0, false);
			_homeRateCounter = 0;
			_oldDirection = DIR_NONE;
			_oldCharacterPosX = getSpriteX(0);
			_oldCharacterPosY = getSpriteY(0);

			if (zoneId > 0) {
				ZoneItem *curZone = &_vm->_linesMan->_zone[zoneId];
				if (curZone->_destX && curZone->_destY && curZone->_destY != 31) {
					if ( curZone->_spriteIndex == -1) {
						curZone->_destX = 0;
						curZone->_destY = 0;
						curZone->_spriteIndex = 0;
					} else {
						setSpriteIndex(0,  curZone->_spriteIndex);
						_vm->_globals->_actionDirection = curZone->_spriteIndex - 59;
					}
				}
			}
			_homeRateCounter = 0;
			return;
		}
		if (_oldDirection != newDirection)
			break;
		if ((newDirection == DIR_RIGHT && newPosX >= oldPosX) || (_oldDirection == DIR_LEFT && newPosX <= oldPosX) ||
		    (_oldDirection == DIR_UP && newPosY <= oldPosY)   || (_oldDirection == DIR_DOWN && newPosY >= oldPosY) ||
		    (_oldDirection == DIR_UP_RIGHT && newPosX >= oldPosX)   || (_oldDirection == DIR_UP_LEFT && newPosX <= oldPosX) ||
		    (_oldDirection == DIR_DOWN_RIGHT && newPosX >= oldPosX) || (_oldDirection == DIR_DOWN_LEFT && newPosX <= oldPosX))
			loopCond = true;
	} while (!loopCond);
	if (loopCond) {
		computeAndSetSpriteSize();
		if ((_oldDirection == DIR_DOWN_LEFT) || (_oldDirection == DIR_LEFT) || (_oldDirection == DIR_UP_LEFT))
			setFlipSprite(0, true);

		if ((_oldDirection == DIR_UP) || (_oldDirection == DIR_UP_RIGHT) || (_oldDirection == DIR_RIGHT) ||
		    (_oldDirection == DIR_DOWN_RIGHT) || (_oldDirection == DIR_DOWN))
			setFlipSprite(0, false);

		setSpriteX(0, newPosX);
		setSpriteY(0, newPosY);
		setSpriteIndex(0, oldFrameIdx);
	} else {
		if ((_oldDirection == DIR_DOWN_LEFT) || (_oldDirection == DIR_LEFT) || (_oldDirection == DIR_UP_LEFT))
			setFlipSprite(0, true);

		if ((_oldDirection == DIR_UP) || (_oldDirection == DIR_UP_RIGHT) || (_oldDirection == DIR_RIGHT) ||
		    (_oldDirection == DIR_DOWN_RIGHT) || (_oldDirection == DIR_DOWN))
			setFlipSprite(0, false);
		_homeRateCounter = 0;
	}
	_oldDirection = newDirection;
	_oldDirectionSpriteIdx = newDirection + 59;
	_oldFrameIndex = oldFrameIdx;
	_oldCharacterPosX = newPosX;
	_oldCharacterPosY = newPosY;
}

void ObjectsManager::goHome2() {
	if (_vm->_linesMan->_route == nullptr)
		return;

	int realSpeed = 2;
	if (_vm->_globals->_speed == 2)
		realSpeed = 4;
	else if (_vm->_globals->_speed == 3)
		realSpeed = 6;

	int countColisionPixel = 0;

	for (;;) {
		int nexPosX = _vm->_linesMan->_route->_x;
		int newPosY = _vm->_linesMan->_route->_y;
		Directions newDirection = (Directions)_vm->_linesMan->_route->_dir;
		_vm->_linesMan->_route++;

		if ((nexPosX == -1) && (newPosY == -1))
			break;

		++countColisionPixel;
		if (countColisionPixel >= realSpeed) {
			_lastDirection = newDirection;
			setSpriteX(0, nexPosX);
			setSpriteY(0, newPosY);
			switch (_lastDirection) {
			case DIR_UP:
				setSpriteIndex(0, 4);
				break;
			case DIR_RIGHT:
				setSpriteIndex(0, 5);
				break;
			case DIR_DOWN:
				setSpriteIndex(0, 6);
				break;
			case DIR_LEFT:
				setSpriteIndex(0, 7);
				break;
			default:
				break;
			}

			return;
		}
	}

	switch (_lastDirection) {
	case DIR_UP:
		setSpriteIndex(0, 0);
		break;
	case DIR_RIGHT:
		setSpriteIndex(0, 1);
		break;
	case DIR_DOWN:
		setSpriteIndex(0, 2);
		break;
	case DIR_LEFT:
		setSpriteIndex(0, 3);
		break;
	default:
		break;
	}

	_vm->_linesMan->_route = nullptr;
}

/**
 * Load Zone
 */
void ObjectsManager::loadZone(const Common::Path &file) {
	for (int i = 1; i <= 100; i++) {
		ZoneItem *curZone = &_vm->_linesMan->_zone[i];
		curZone->_destX = 0;
		curZone->_destY = 0;
		curZone->_spriteIndex = 0;
		curZone->_verbFl1 = 0;
		curZone->_verbFl2 = 0;
		curZone->_verbFl3 = 0;
		curZone->_verbFl4 = 0;
		curZone->_verbFl5 = 0;
		curZone->_verbFl6 = 0;
		curZone->_verbFl7 = 0;
		curZone->_verbFl8 = 0;
		curZone->_verbFl9 = 0;
		curZone->_verbFl10 = 0;
		curZone->_messageId = 0;
		curZone->_enabledFl = false;
	}

	Common::File f;
	if (!f.exists(file))
		error("File not found : %s", file.toString().c_str());

	byte *ptr = _vm->_fileIO->loadFile(file);
	int bufId = 0;
	int zoneLineIdx = 0;
	int bobZoneIdx;
	do {
		bobZoneIdx = READ_LE_INT16((uint16 *)ptr + bufId);
		if (bobZoneIdx != -1) {
			_vm->_linesMan->addZoneLine(
			    zoneLineIdx,
			    READ_LE_UINT16((uint16 *)ptr + bufId + 1),
			    READ_LE_UINT16((uint16 *)ptr + bufId + 2),
			    READ_LE_UINT16((uint16 *)ptr + bufId + 3),
			    READ_LE_UINT16((uint16 *)ptr + bufId + 4),
			    bobZoneIdx);
			_vm->_linesMan->_zone[bobZoneIdx]._enabledFl = true;
		}
		bufId += 5;
		++zoneLineIdx;
	} while (bobZoneIdx != -1);

	for (int i = 1; i <= 100; i++) {
		ZoneItem *curZone = &_vm->_linesMan->_zone[i];
		curZone->_destX = READ_LE_INT16((uint16 *)ptr + bufId);
		curZone->_destY = READ_LE_INT16((uint16 *)ptr + bufId + 1);
		curZone->_spriteIndex = READ_LE_INT16((uint16 *)ptr + bufId + 2);
		bufId += 3;
	}

	byte *verbData = (ptr + 10 * zoneLineIdx + 606);
	bufId = 0;
	for (int i = 1; i <= 100; i++) {
		ZoneItem *curZone = &_vm->_linesMan->_zone[i];
		curZone->_verbFl1 = verbData[bufId];
		curZone->_verbFl2 = verbData[bufId + 1];
		curZone->_verbFl3 = verbData[bufId + 2];
		curZone->_verbFl4 = verbData[bufId + 3];
		curZone->_verbFl5 = verbData[bufId + 4];
		curZone->_verbFl6 = verbData[bufId + 5];
		curZone->_verbFl7 = verbData[bufId + 6];
		curZone->_verbFl8 = verbData[bufId + 7];
		curZone->_verbFl9 = verbData[bufId + 8];
		curZone->_verbFl10 = verbData[bufId + 9];

		bufId += 10;
	}
	verbData += 1010;
	for (int i = 0; i < 100; i++)
		_vm->_linesMan->_zone[i + 1]._messageId = READ_LE_UINT16(verbData + 2 * i);

	_vm->_globals->freeMemory(ptr);
	_vm->_linesMan->initSquareZones();
}

void ObjectsManager::handleCityMap() {
	_vm->_dialog->_inventFl = false;
	_vm->_events->_gameKey = KEY_NONE;
	_vm->_linesMan->setMaxLineIdx(1);
	_vm->_globals->_characterMaxPosY = 440;
	_vm->_globals->_cityMapEnabledFl = true;
	_vm->_graphicsMan->_noFadingFl = false;
	_vm->_globals->_freezeCharacterFl = false;
	_spritePtr = nullptr;
	_vm->_globals->_exitId = 0;
	_vm->_globals->_checkDistanceFl = true;
	_vm->_soundMan->playSound(31);
	_vm->_globals->_eventMode = EVENTMODE_IGNORE;
	_vm->_graphicsMan->loadImage("PLAN");
	_vm->_linesMan->loadLines("PLAN.OB2");
	loadHidingItems("PLAN.CA2");
	loadZone("PLAN.ZO2");
	_spritePtr = _vm->_fileIO->loadFile("VOITURE.SPR");
	_vm->_animMan->loadAnim("PLAN");
	_vm->_graphicsMan->displayAllBob();
	_vm->_graphicsMan->initScreen("PLAN", 2, false);
	for (int i = 0; i <= 15; i++)
		disableHidingItem(i);
	disableHidingItem(19);
	disableHidingItem(20);
	enableHidingBehavior();

	if (!_mapCarPosX && !_mapCarPosY) {
		_mapCarPosX = 900;
		_mapCarPosY = 319;
	}
	addStaticSprite(_spritePtr, Common::Point(_mapCarPosX, _mapCarPosY), 0, 1, 0, false, 5, 5);
	_vm->_events->setMouseXY(_mapCarPosX, _mapCarPosY);
	_vm->_events->mouseOn();
	_vm->_graphicsMan->scrollScreen(getSpriteX(0) - 320);
	_vm->_graphicsMan->_scrollOffset = getSpriteX(0) - 320;
	animateSprite(0);
	_vm->_linesMan->_route = nullptr;
	_vm->_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(254, 0, 0, 0);

	for (int i = 0; i <= 4; i++)
		_vm->_events->refreshScreenAndEvents();

	_vm->_globals->_eventMode = EVENTMODE_IGNORE;
	_vm->_graphicsMan->fadeInLong();
	_vm->_events->changeMouseCursor(4);
	_vm->_graphicsMan->_noFadingFl = false;

	bool loopCond = false;
	do {
		int mouseButton = _vm->_events->getMouseButton();
		if (mouseButton) {
			// First cop call : Go to the bank and free the hostages
			if (_vm->_globals->_saveData->_data[svBankAttackAnimPlayedFl] == 1 && !_vm->_globals->_saveData->_data[svCopCall1PlayedFl]) {
				_vm->_globals->_saveData->_data[svCopCall1PlayedFl] = 1;
				_vm->_globals->_introSpeechOffFl = true;
				_vm->_talkMan->startAnimatedCharacterDialogue("APPEL1.pe2");
				_vm->_globals->_introSpeechOffFl = false;
				mouseButton = 0;
			}
			// Second cop call: Helico has been found in the empty lot
			if (_vm->_globals->_saveData->_data[svFreedHostageFl] == 1 && !_vm->_globals->_saveData->_data[svCopCall2PlayedFl]) {
				_vm->_globals->_saveData->_data[svCopCall2PlayedFl] = 1;
				_vm->_globals->_introSpeechOffFl = true;
				_vm->_talkMan->startAnimatedCharacterDialogue("APPEL2.pe2");
				_vm->_globals->_introSpeechOffFl = false;
				mouseButton = 0;
				_vm->_events->_curMouseButton = 0;
			}
			if (mouseButton == 1)
				handleLeftButton();
		}

		_vm->_linesMan->checkZone();
		goHome2();

		if (_vm->_linesMan->_route == nullptr && _vm->_globals->_actionMoveTo)
			paradise();
		_vm->_events->refreshScreenAndEvents();

		if (_vm->_globals->_exitId)
			loopCond = true;
	} while (!_vm->shouldQuit() && !loopCond);

	if (!_vm->_graphicsMan->_noFadingFl)
		_vm->_graphicsMan->fadeOutLong();
	_vm->_globals->_eventMode = EVENTMODE_DEFAULT;
	_vm->_graphicsMan->_noFadingFl = false;
	_mapCarPosX = getSpriteX(0);
	_mapCarPosY = getSpriteY(0);
	removeSprite(0);
	_spritePtr = _vm->_globals->freeMemory(_spritePtr);
	clearScreen();
	_vm->_globals->_cityMapEnabledFl = false;
}

/**
 * Handle Left button
 */
void ObjectsManager::handleLeftButton() {
	_vm->_fontMan->hideText(9);
	int destX = _vm->_events->getMouseX();
	int destY = _vm->_events->getMouseY();

	if (!_vm->_dialog->_inventFl && !_vm->_globals->_cityMapEnabledFl &&
		destX > _vm->_graphicsMan->_scrollOffset - 30 && destX < _vm->_graphicsMan->_scrollOffset + 50 &&
		destY > -30 && destY < 50) {
		int oldMouseCursor = _vm->_events->_mouseCursorId;
		_vm->_dialog->_inventFl = true;
		_vm->_dialog->showInventory();
		_vm->_dialog->_inventFl = false;
		_vm->_events->_gameKey = KEY_NONE;
		if (!_vm->_globals->_exitId) {
			_vm->_dialog->_inventFl = false;
			_vm->_events->_mouseCursorId = oldMouseCursor;
		}
		return;
	}
	if (_vm->_globals->_saveData->_data[svField354] == 1 && !_vm->_globals->_cityMapEnabledFl
	    && destX >= 533 && destX <= 559 && destY >= 26 && destY <= 59) {
		changeCharacterHead(CHARACTER_HOPKINS_CLONE, CHARACTER_HOPKINS);
		return;
	}
	if (_vm->_globals->_saveData->_data[svField356] == 1 && !_vm->_globals->_cityMapEnabledFl
	    && destX >= 533 && destX <= 559 && destY >= 26 && destY <= 48) {
		changeCharacterHead(CHARACTER_SAMANTHA, CHARACTER_HOPKINS);
		return;
	}
	if (_vm->_globals->_saveData->_data[svField357] == 1) {
		if (_vm->_globals->_saveData->_data[svField353] == 1 && !_vm->_globals->_cityMapEnabledFl
		    && destX >= 533 && destX <= 559 && destY >= 26 && destY <= 59) {
			changeCharacterHead(CHARACTER_HOPKINS, CHARACTER_HOPKINS_CLONE);
			return;
		}
		if (_vm->_globals->_saveData->_data[svField355] == 1 && !_vm->_globals->_cityMapEnabledFl
		    && destX >= 567 && destX <= 593 && destY >= 26 && destY <= 59) {
			changeCharacterHead(CHARACTER_HOPKINS, CHARACTER_SAMANTHA);
			return;
		}
	}
	if (_vm->_globals->_cityMapEnabledFl && _vm->_globals->_actionMoveTo) {
		_vm->_linesMan->checkZone();
		if (_zoneNum <= 0)
			return;
		int routeIdx = 0;
		do {
			_vm->_linesMan->_testRoute2[routeIdx] = _vm->_linesMan->_route[routeIdx];
			++routeIdx;
		} while (_vm->_linesMan->_route[routeIdx]._x != -1);

		_vm->_linesMan->_testRoute2[routeIdx].invalidate();
	}

	if (_vm->_globals->_actionMoveTo) {
		_vm->_linesMan->checkZone();
		_vm->_globals->_actionMoveTo = false;
		_vm->_globals->_saveData->_data[svLastMouseCursor] = 0;
		_vm->_globals->_saveData->_data[svLastZoneNum] = 0;
	}

	if (_vm->_globals->_cityMapEnabledFl && (_vm->_events->_mouseCursorId != 4 || _zoneNum <= 0))
		return;
	if (_zoneNum != -1 && _zoneNum != 0) {
		ZoneItem *curZone = &_vm->_linesMan->_zone[_zoneNum];
		if (curZone->_destX && curZone->_destY && curZone->_destY != 31) {
			destX = curZone->_destX;
			destY = curZone->_destY;
		}
	}
	_vm->_globals->_actionMoveTo = false;
	RouteItem *oldRoute = _vm->_linesMan->_route;
	_vm->_linesMan->_route = nullptr;
	if (_forestFl && _zoneNum >= 20 && _zoneNum <= 23) {
		if (getSpriteY(0) > 374 && getSpriteY(0) <= 410) {
			_vm->_linesMan->_route = nullptr;
			setSpriteIndex(0, _oldDirectionSpriteIdx);
			_vm->_globals->_actionDirection = DIR_NONE;
			_vm->_linesMan->_route = nullptr;
			computeAndSetSpriteSize();
			setFlipSprite(0, false);
			_homeRateCounter = 0;
			_oldDirection = DIR_NONE;
		} else {
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(getSpriteX(0), getSpriteY(0), getSpriteX(0), 390);
			if (_vm->_linesMan->_route)
				_vm->_linesMan->optimizeRoute(_vm->_linesMan->_route);
			_oldCharacterPosX = getSpriteX(0);
			_oldCharacterPosY = getSpriteY(0);
			_homeRateCounter = 0;
			if (_vm->_linesMan->_route || oldRoute == _vm->_linesMan->_route) {
				_oldDirection = DIR_NONE;
			} else {
				_vm->_linesMan->_route = oldRoute;
			}
		}
	} else {
		if (!_vm->_globals->_freezeCharacterFl && !_vm->_globals->_cityMapEnabledFl) {
			_vm->_linesMan->_route = _vm->_linesMan->findRoute(getSpriteX(0), getSpriteY(0), destX, destY);
			if (_vm->_linesMan->_route)
				_vm->_linesMan->optimizeRoute(_vm->_linesMan->_route);
			_oldCharacterPosX = getSpriteX(0);
			_oldCharacterPosY = getSpriteY(0);
			_homeRateCounter = 0;
			if (_vm->_linesMan->_route || oldRoute == _vm->_linesMan->_route)
				_oldDirection = DIR_NONE;
			else
				_vm->_linesMan->_route = oldRoute;
		}
	}

	if (!_vm->_globals->_freezeCharacterFl && _vm->_globals->_cityMapEnabledFl)
		_vm->_linesMan->_route = _vm->_linesMan->cityMapCarRoute(getSpriteX(0), getSpriteY(0), destX, destY);

	if (_zoneNum != -1 && _zoneNum != 0) {
		if (_vm->_events->_mouseCursorId == 23)
			_vm->_globals->_saveData->_data[svLastMouseCursor] = 5;
		else
			_vm->_globals->_saveData->_data[svLastMouseCursor] = _vm->_events->_mouseCursorId;

		if (_vm->_globals->_cityMapEnabledFl)
			_vm->_globals->_saveData->_data[svLastMouseCursor] = 6;
		_vm->_globals->_saveData->_data[svLastZoneNum] = _zoneNum;
		_vm->_globals->_saveData->_data[svLastObjectIndex] = _curObjectIndex;
		_vm->_globals->_actionMoveTo = true;
	}
	_vm->_fontMan->hideText(5);
	_vm->_graphicsMan->setColorPercentage2(251, 100, 100, 100);
	if (_vm->_globals->_screenId == 20 && _vm->_globals->_saveData->_data[svField132] == 1
				&& _curObjectIndex == 20 && _zoneNum == 12
				&& _vm->_events->_mouseCursorId == 23) {
		// Special case for throwing darts at the switch in Purgatory - the player shouldn't move
		_vm->_linesMan->_route = nullptr;
		getSpriteX(0);
		getSpriteY(0);
	}
}

void ObjectsManager::paradise() {
	char result = _vm->_globals->_saveData->_data[svLastMouseCursor];
	if (result && _vm->_globals->_saveData->_data[svLastZoneNum] && result != 4 && result > 3) {
		_vm->_fontMan->hideText(5);
		if (!_forestFl || _zoneNum < 20 || _zoneNum > 23) {
			if (_vm->_graphicsMan->_largeScreenFl) {
				_vm->_graphicsMan->_scrollStatus = 2;
				if (_vm->_events->_startPos.x + 320 - getSpriteX(0) > 160) {
					bool loopCond = false;
					do {
						_vm->_graphicsMan->_scrollPosX -= _vm->_graphicsMan->_scrollSpeed;
						if (_vm->_graphicsMan->_scrollPosX < 0) {
							_vm->_graphicsMan->_scrollPosX = 0;
							loopCond = true;
						}
						if (_vm->_graphicsMan->_scrollPosX > SCREEN_WIDTH) {
							_vm->_graphicsMan->_scrollPosX = SCREEN_WIDTH;
							loopCond = true;
						}
						if (_vm->_events->getMouseX() > _vm->_graphicsMan->_scrollPosX + 620)
							_vm->_events->setMouseXY(_vm->_events->_mousePos.x - 4, _vm->_events->getMouseY());

						_vm->_events->refreshScreenAndEvents();
					} while (!loopCond && _vm->_events->_startPos.x > getSpriteX(0) - 320);
				} else if (_vm->_events->_startPos.x + 320 - getSpriteX(0) < -160) {
					bool loopCond = false;
					do {
						_vm->_graphicsMan->_scrollPosX += _vm->_graphicsMan->_scrollSpeed;
						if (_vm->_graphicsMan->_scrollPosX < 0) {
							_vm->_graphicsMan->_scrollPosX = 0;
							loopCond = true;
						}
						if (_vm->_graphicsMan->_scrollPosX > SCREEN_WIDTH) {
							_vm->_graphicsMan->_scrollPosX = SCREEN_WIDTH;
							loopCond = true;
						}
						if (_vm->_events->getMouseX() < _vm->_graphicsMan->_scrollPosX + 10)
							_vm->_events->setMouseXY(_vm->_events->_mousePos.x + 4, _vm->_events->getMouseY());

						_vm->_events->refreshScreenAndEvents();
					} while (!loopCond && _vm->_events->_startPos.x < getSpriteX(0) - 320);
				}
				if (_vm->_events->getMouseX() > _vm->_graphicsMan->_scrollPosX + 620)
					_vm->_events->setMouseXY(_vm->_graphicsMan->_scrollPosX + 610, 0);
				if (_vm->_events->getMouseX() < _vm->_graphicsMan->_scrollPosX + 10)
					_vm->_events->setMouseXY(_vm->_graphicsMan->_scrollPosX + 10, 0);
				_vm->_events->refreshScreenAndEvents();
				_vm->_graphicsMan->_scrollStatus = 0;
			}
			_vm->_talkMan->handleAnswer(_vm->_globals->_saveData->_data[svLastZoneNum], _vm->_globals->_saveData->_data[svLastMouseCursor]);
		} else {
			_vm->_talkMan->handleForestAnswser(_vm->_globals->_saveData->_data[svLastZoneNum], _vm->_globals->_saveData->_data[svLastMouseCursor]);
		}
		_vm->_events->changeMouseCursor(4);
		if (_zoneNum != -1 && _zoneNum != 0 && !_vm->_linesMan->_zone[_zoneNum]._enabledFl) {
			_zoneNum = -1;
			_forceZoneFl = true;
		}
		if (_zoneNum != _vm->_globals->_saveData->_data[svLastZoneNum] || _zoneNum == -1 || _zoneNum == 0) {
			_vm->_events->_mouseCursorId = 4;
			_changeVerbFl = false;
		} else {
			_vm->_events->_mouseCursorId = _vm->_globals->_saveData->_data[svLastMouseCursor];
			if (_changeVerbFl) {
				nextVerbIcon();
				_changeVerbFl = false;
			}
			if (_vm->_events->_mouseCursorId == 5)
				_vm->_events->_mouseCursorId = 4;
		}
		if (_vm->_events->_mouseCursorId != 23)
			_vm->_events->changeMouseCursor(_vm->_events->_mouseCursorId);
		_zoneNum = 0;
		_vm->_globals->_saveData->_data[svLastMouseCursor] = 0;
		_vm->_globals->_saveData->_data[svLastZoneNum] = 0;
	}
	if (_vm->_globals->_cityMapEnabledFl) {
		_vm->_events->_mouseCursorId = 0;
		_vm->_events->changeMouseCursor(0);
	}
	if (_vm->_globals->_freezeCharacterFl && _vm->_events->_mouseCursorId == 4) {
		if (_zoneNum != -1 && _zoneNum != 0)
			handleRightButton();
	}
	_vm->_globals->_actionMoveTo = false;
}

/**
 * Clear Screen
 */
void ObjectsManager::clearScreen() {
	clearSprite();
	_vm->_graphicsMan->endDisplayBob();
	_vm->_fontMan->hideText(5);
	_vm->_fontMan->hideText(9);
	clearVBob();
	_vm->_animMan->clearAnim();
	_vm->_linesMan->clearAllZones();
	_vm->_linesMan->resetLines();
	resetHidingItems();

	for (int i = 0; i <= 48; i++) {
		_vm->_linesMan->_bobZone[i] = 0;
		_vm->_linesMan->_bobZoneFl[i] = false;
	}
	_vm->_events->_mouseCursorId = 4;
	_verb = 4;
	_zoneNum = 0;
	_forceZoneFl = true;
	_vm->_linesMan->resetLinesNumb();
	_vm->_linesMan->resetLastLine();
	_vm->_linesMan->_route = nullptr;
	_vm->_globals->_answerBuffer = _vm->_globals->freeMemory(_vm->_globals->_answerBuffer);
	_vm->_globals->_levelSpriteBuf = _vm->_globals->freeMemory(_vm->_globals->_levelSpriteBuf);
	_vm->_events->_startPos.x = 0;
	_vm->_events->_mouseSpriteId = 0;
	_vm->_globals->_saveData->_data[svLastMouseCursor] = 0;
	_vm->_globals->_saveData->_data[svLastZoneNum] = 0;
	_vm->_globals->_actionMoveTo = false;
	_forceZoneFl = true;
	_changeVerbFl = false;
	_vm->_linesMan->_route = nullptr;
	_oldDirection = DIR_NONE;
	_vm->_graphicsMan->resetDirtyRects();
}

/**
 * Change the currently active player face / Head
 * @param oldCharacter		Previously played character
 * @param newCharacter		New character to play
 */
void ObjectsManager::changeCharacterHead(PlayerCharacter oldCharacter, PlayerCharacter newCharacter) {
	CharacterLocation *loc;

	_changeHeadFl = true;
	_vm->_graphicsMan->copySurface(_vm->_graphicsMan->_backBuffer, 532, 25, 65, 40, _vm->_graphicsMan->_frontBuffer, 532, 25);
	_vm->_graphicsMan->addDirtyRect(532, 25, 597, 65);
	_vm->_globals->_checkDistanceFl = true;
	_vm->_linesMan->_route = nullptr;

	if (oldCharacter == CHARACTER_SAMANTHA && newCharacter == CHARACTER_HOPKINS
		&& _vm->_globals->_saveData->_realHopkins._location == _vm->_globals->_screenId) {
		_changeHeadFl = false;
		loc = &_vm->_globals->_saveData->_samantha;
		loc->_pos.x = getSpriteX(0);
		loc->_pos.y = getSpriteY(0);
		loc->_startSpriteIndex = 64;
		loc->_location = _vm->_globals->_screenId;
		loc->_zoomFactor = _sprite[0]._zoomFactor;

		removeSprite(1);
		addStaticSprite(_headSprites, loc->_pos, 1, 3, loc->_zoomFactor, false, 20, 127);
		animateSprite(1);
		removeSprite(0);

		_vm->_globals->_saveData->_data[svField354] = 0;
		_vm->_globals->_saveData->_data[svField356] = 0;
		_vm->_globals->_saveData->_data[svField357] = 1;

		loc = &_vm->_globals->_saveData->_realHopkins;
		_vm->_globals->_characterSpriteBuf = _vm->_fileIO->loadFile("PERSO.SPR");
		_vm->_globals->_characterType = CHARACTER_HOPKINS;
		addStaticSprite(_vm->_globals->_characterSpriteBuf, loc->_pos, 0, 64, loc->_zoomFactor, false, 34, 190);
		animateSprite(0);
		_vm->_globals->loadCharacterData();
	} else if (oldCharacter == CHARACTER_HOPKINS && newCharacter == CHARACTER_SAMANTHA
			&& _vm->_globals->_saveData->_samantha._location == _vm->_globals->_screenId) {
		_changeHeadFl = false;
		loc = &_vm->_globals->_saveData->_realHopkins;
		loc->_pos.x = getSpriteX(0);
		loc->_pos.y = getSpriteY(0);
		loc->_startSpriteIndex = 64;
		loc->_location = _vm->_globals->_screenId;
		loc->_zoomFactor = _sprite[0]._zoomFactor;

		removeSprite(1);
		addStaticSprite(_headSprites, loc->_pos, 1, 2, loc->_zoomFactor, false, 34, 190);
		animateSprite(1);
		removeSprite(0);

		_vm->_globals->_saveData->_data[svField354] = 0;
		_vm->_globals->_saveData->_data[svField356] = 1;
		_vm->_globals->_saveData->_data[svField357] = 0;

		loc = &_vm->_globals->_saveData->_samantha;
		_vm->_globals->_characterSpriteBuf = _vm->_fileIO->loadFile("PSAMAN.SPR");
		_vm->_globals->_characterType = CHARACTER_SAMANTHA;
		addStaticSprite(_vm->_globals->_characterSpriteBuf, loc->_pos, 0, 64, loc->_zoomFactor, false, 20, 127);
		animateSprite(0);
		_vm->_globals->loadCharacterData();
	} else {
		switch (oldCharacter) {
		case CHARACTER_HOPKINS:
			loc = &_vm->_globals->_saveData->_realHopkins;
			loc->_pos.x = getSpriteX(0);
			loc->_pos.y = getSpriteY(0);
			loc->_startSpriteIndex = 64;
			loc->_location = _vm->_globals->_screenId;
			loc->_zoomFactor = _sprite[0]._zoomFactor;
			break;
		case CHARACTER_HOPKINS_CLONE:
			loc = &_vm->_globals->_saveData->_cloneHopkins;
			loc->_pos.x = getSpriteX(0);
			loc->_pos.y = getSpriteY(0);
			loc->_startSpriteIndex = 64;
			loc->_location = _vm->_globals->_screenId;
			loc->_zoomFactor = _sprite[0]._zoomFactor;
			break;
		case CHARACTER_SAMANTHA:
			loc = &_vm->_globals->_saveData->_samantha;
			loc->_pos.x = getSpriteX(0);
			loc->_pos.y = getSpriteY(0);
			loc->_startSpriteIndex = 64;
			loc->_location = _vm->_globals->_screenId;
			loc->_zoomFactor = _sprite[0]._zoomFactor;
			break;
		default:
			break;
		}

		switch (newCharacter) {
		case CHARACTER_HOPKINS:
			_vm->_globals->_saveData->_data[svHopkinsCloneFl] = 0;
			_vm->_globals->_saveData->_data[svField354] = 0;
			_vm->_globals->_saveData->_data[svField356] = 0;
			_vm->_globals->_saveData->_data[svField357] = 1;
			_vm->_globals->_exitId = _vm->_globals->_saveData->_realHopkins._location;
			break;
		case CHARACTER_HOPKINS_CLONE:
			_vm->_globals->_saveData->_data[svHopkinsCloneFl] = 1;
			_vm->_globals->_saveData->_data[svField354] = 1;
			_vm->_globals->_saveData->_data[svField356] = 0;
			_vm->_globals->_saveData->_data[svField357] = 0;
			_vm->_globals->_exitId = _vm->_globals->_saveData->_cloneHopkins._location;
			break;
		case CHARACTER_SAMANTHA:
			_vm->_globals->_saveData->_data[svHopkinsCloneFl] = 0;
			_vm->_globals->_saveData->_data[svField354] = 0;
			_vm->_globals->_saveData->_data[svField356] = 1;
			_vm->_globals->_saveData->_data[svField357] = 0;
			_vm->_globals->_exitId = _vm->_globals->_saveData->_samantha._location;
			break;
		default:
			break;
		}
	}
}

// Check Size
void ObjectsManager::computeAndSetSpriteSize() {
	int size = _vm->_globals->_spriteSize[getSpriteY(0)];
	if (_vm->_globals->_characterType == CHARACTER_HOPKINS_CLONE) {
		size = 20 * (5 * abs(size) - 100) / -80;
	} else if (_vm->_globals->_characterType == CHARACTER_SAMANTHA) {
		size = 20 * (5 * abs(size) - 165) / -67;
	}
	setSpriteZoom(0, size);
}

/**
 * Get next verb icon (or text)
 */
void ObjectsManager::nextVerbIcon() {
	_vm->_events->_mouseCursorId++;

	for(;;) {
		if (_vm->_events->_mouseCursorId == 4) {
			if (!_vm->_globals->_freezeCharacterFl || _zoneNum == -1 || _zoneNum == 0)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 5 || _vm->_events->_mouseCursorId == 6) {
			_vm->_events->_mouseCursorId = 6;
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl1 == 1)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 7) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl2 == 1)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 8) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl3 == 1)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 9) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl4 == 1)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 10) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl5 == 1)
				return;
			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 11) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl6 == 1)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 12) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl7 == 1)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 13) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl8 == 1)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 14) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl9 == 1)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 15) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl10 == 1)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 16) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl1 == 2)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 17) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl4 == 2)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 18) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl5 == 2)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 19) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl6 == 2)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 20) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl7 == 2)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 21) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl10 == 2)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 22) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl8 == 2)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 23) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl3 == 2)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 24) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl4 == 3)
				return;

			++_vm->_events->_mouseCursorId;
		}

		if (_vm->_events->_mouseCursorId == 25) {
			if (_vm->_linesMan->_zone[_zoneNum]._verbFl9 == 2)
				return;
		}
		_vm->_events->_mouseCursorId = 4;
	}
}

/**
 * Handle Right button
 */
void ObjectsManager::handleRightButton() {
	if (_zoneNum != -1 && _zoneNum != 0) {
		nextVerbIcon();
		if (_vm->_events->_mouseCursorId != 23)
			_vm->_events->changeMouseCursor(_vm->_events->_mouseCursorId);
		_verb = _vm->_events->_mouseCursorId;
	}
}

/**
 * Prepare border used to highlight the place below mouse cursor, in the inventory.
 * Also set the mouse cursor
 */
void ObjectsManager::initBorder(int zoneIdx) {
	_oldBorderPos = _borderPos;
	_oldBorderSpriteIndex = _borderSpriteIndex;
	if (zoneIdx >= 1 && zoneIdx <= 6)
		_borderPos.y = 120;
	else if (zoneIdx >= 7 && zoneIdx <= 12)
		_borderPos.y = 158;
	else if (zoneIdx >= 13 && zoneIdx <= 18)
		_borderPos.y = 196;
	else if (zoneIdx >= 19 && zoneIdx <= 24)
		_borderPos.y = 234;
	else if (zoneIdx >= 25 && zoneIdx <= 29)
		_borderPos.y = 272;
	else if (zoneIdx == 30)
		_borderPos.y = 272;
	else if (zoneIdx == 31)
		_borderPos.y = 290;

	if (zoneIdx == 1 || zoneIdx == 7 || zoneIdx == 13 || zoneIdx == 19 || zoneIdx == 25)
		_borderPos.x = _vm->_graphicsMan->_scrollOffset + 158;
	else if (zoneIdx == 2 || zoneIdx == 8 || zoneIdx == 14 || zoneIdx == 20 || zoneIdx == 26)
		_borderPos.x = _vm->_graphicsMan->_scrollOffset + 212;
	else if (zoneIdx == 3 || zoneIdx == 9 || zoneIdx == 15 || zoneIdx == 21 || zoneIdx == 27)
		_borderPos.x = _vm->_graphicsMan->_scrollOffset + 266;
	else if (zoneIdx == 4 || zoneIdx == 10 || zoneIdx == 16 || zoneIdx == 22 || zoneIdx == 28)
		_borderPos.x = _vm->_graphicsMan->_scrollOffset + 320;
	else if (zoneIdx == 5 || zoneIdx == 11 || zoneIdx == 17 || zoneIdx == 23 || zoneIdx == 29)
		_borderPos.x = _vm->_graphicsMan->_scrollOffset + 374;
	else if (zoneIdx == 6 || zoneIdx == 12 || zoneIdx == 18 || zoneIdx == 24 || zoneIdx == 30 || zoneIdx == 31)
		_borderPos.x = _vm->_graphicsMan->_scrollOffset + 428;

	if (zoneIdx >= 1 && zoneIdx <= 29)
		_borderSpriteIndex = 0;
	else if (zoneIdx == 30 || zoneIdx == 31)
		_borderSpriteIndex = 2;
	else if (!zoneIdx || zoneIdx == 32) {
		_borderPos = Common::Point(0, 0);
		_borderSpriteIndex = 0;
	}

	if (!zoneIdx)
		_vm->_events->_mouseCursorId = 0;
	else if (zoneIdx >= 1 && zoneIdx <= 28)
		_vm->_events->_mouseCursorId = 8;
	else if (zoneIdx == 29)
		_vm->_events->_mouseCursorId = 1;
	else if (zoneIdx == 30)
		_vm->_events->_mouseCursorId = 2;
	else if (zoneIdx == 31)
		_vm->_events->_mouseCursorId = 3;
	else if (zoneIdx == 32)
		_vm->_events->_mouseCursorId = 16;

	if (zoneIdx >= 1 && zoneIdx <= 28 && !_vm->_globals->_inventory[zoneIdx]) {
		_vm->_events->_mouseCursorId = 0;
		_borderPos = Common::Point(0, 0);
		_borderSpriteIndex = 0;
	}

	if (_vm->_events->_mouseCursorId != 23)
		_vm->_events->changeMouseCursor(_vm->_events->_mouseCursorId);
	_vm->_events->getMouseX();
	_vm->_events->getMouseY();
}

/**
 * Get next icon for an object in the inventory
 */
void ObjectsManager::nextObjectIcon(int idx) {
	int cursorId = _vm->_events->_mouseCursorId;
	ObjectAuthIcon *curAuthIco = &_objectAuthIcons[_vm->_globals->_inventory[idx]];

	if (cursorId == 0 || cursorId == 2 || cursorId == 3 || cursorId == 16)
		return;

	int nextCursorId = cursorId + 1;
	if (nextCursorId > 25)
		nextCursorId = 6;

	do {
		if (nextCursorId == 2 || nextCursorId == 5 || nextCursorId == 6) {
			_vm->_events->_mouseCursorId = 6;
			if (curAuthIco->_flag1 == 1)
				return;
			nextCursorId++;
		}
		if (nextCursorId == 7) {
			_vm->_events->_mouseCursorId = 7;
			if (curAuthIco->_flag2 == 1)
				return;
			nextCursorId++;
		}
		if (nextCursorId == 8) {
			_vm->_events->_mouseCursorId = 8;
			return;
		}
		if (nextCursorId == 9 || nextCursorId == 10) {
			_vm->_events->_mouseCursorId = 10;
			if (curAuthIco->_flag6 == 1)
				return;
			nextCursorId = 11;
		}

		if (nextCursorId == 11) {
			_vm->_events->_mouseCursorId = 11;
			if (curAuthIco->_flag3 == 1)
				return;
			nextCursorId++;
		}

		if (nextCursorId == 12 || nextCursorId == 13) {
			_vm->_events->_mouseCursorId = 13;
			if (curAuthIco->_flag4 == 1)
				return;
			nextCursorId = 14;
		}

		if (nextCursorId == 14 || nextCursorId == 15) {
			_vm->_events->_mouseCursorId = 15;
			if (curAuthIco->_flag5 == 1)
				return;
			nextCursorId = 23;
		}

		if (nextCursorId >= 16 && nextCursorId <= 23) {
			_vm->_events->_mouseCursorId = 23;
			if (curAuthIco->_flag5 == 2)
				return;
			nextCursorId = 24;
		}

		if (nextCursorId == 24 || nextCursorId == 25) {
			_vm->_events->_mouseCursorId = 25;
		}

		nextCursorId = 6;
	} while (curAuthIco->_flag6 != 2);
}

void ObjectsManager::takeInventoryObject(int idx) {
	if (_vm->_events->_mouseCursorId == 8)
		changeObject(idx);
}

void ObjectsManager::loadObjectIniFile() {
	byte *data;
	Common::Path file;
	int lastOpcodeResult = 1;

	file = "OBJET1.ini";
	bool fileFoundFl = false;
	data = _vm->_fileIO->searchCat(file, RES_INI, fileFoundFl);
	if (!fileFoundFl) {
		data = _vm->_fileIO->loadFile(file);
		if (data == nullptr)
			error("INI file %s not found", file.toString().c_str());
	}

	if (READ_BE_UINT24(data) != MKTAG24('I', 'N', 'I'))
		error("File %s is not an INI file", file.toString().c_str());

	for (;;) {
		int opcodeType = _vm->_script->handleOpcode(data + 20 * lastOpcodeResult);
		if (opcodeType == -1 || _vm->shouldQuit())
			return;

		if (opcodeType == 2)
			lastOpcodeResult = _vm->_script->handleGoto(data + 20 * lastOpcodeResult);
		else if (opcodeType == 3)
			lastOpcodeResult = _vm->_script->handleIf(data, lastOpcodeResult);

		if (lastOpcodeResult == -1)
			error("defective IFF function");

		if (opcodeType == 1 || opcodeType == 4)
			++lastOpcodeResult;
		else if (!opcodeType || opcodeType == 5)
			break;
	}

	_vm->_globals->freeMemory(data);
}

void ObjectsManager::handleSpecialGames() {
	byte *oldPalette;

	switch (_vm->_globals->_screenId) {
	case 5:
		if ((getSpriteY(0) > 399) || _vm->_globals->_saveData->_data[svField173])
			break;

		_vm->_globals->_saveData->_data[svField173] = 1;
		_vm->_globals->_introSpeechOffFl = true;
		_vm->_talkMan->startAnimatedCharacterDialogue("flicspe1.pe2");
		_vm->_globals->_introSpeechOffFl = false;

		if (_vm->_globals->_censorshipFl)
			break;

		oldPalette = _vm->_globals->allocMemory(1000);
		memcpy(oldPalette, _vm->_graphicsMan->_palette, 769);

		_vm->_graphicsMan->backupScreen();

		if (!_vm->_graphicsMan->_lineNbr)
			_vm->_graphicsMan->_scrollOffset = 0;
		_vm->_graphicsMan->displayScreen(true);
		_vm->_soundMan->_specialSoundNum = 198;
		_charactersEnabledFl = true;
		_vm->_animMan->unsetClearAnimFlag();
		_vm->_animMan->playAnim("OTAGE.ANM", "OTAGE.ANM", 1, 24, 500, true);
		_vm->_soundMan->_specialSoundNum = 0;
		_vm->_graphicsMan->displayScreen(false);

		_vm->_graphicsMan->restoreScreen();

		_charactersEnabledFl = false;
		memcpy(_vm->_graphicsMan->_palette, oldPalette, 769);
		_vm->_graphicsMan->setPaletteVGA256(_vm->_graphicsMan->_palette);
		_vm->_globals->freeMemory(oldPalette);
		_vm->_graphicsMan->display8BitRect(_vm->_graphicsMan->_backBuffer, _vm->_events->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		memcpy(_vm->_graphicsMan->_frontBuffer, _vm->_graphicsMan->_backBuffer, 614399);

		_vm->_graphicsMan->_scrollStatus = 0;
		_vm->_graphicsMan->updateScreen();
		break;
	case 20:
		_vm->_globals->_saveData->_data[svField132] = (getSpriteX(0) > 65 && getSpriteX(0) <= 124 && getSpriteY(0) > 372 && getSpriteY(0) <= 398) ? 1 : 0;
		break;
	case 35:
		if (_vm->_globals->_prevScreenId == 16)
			handleForest(35, 500, 555, 100, 440, 1);
		else if (_vm->_globals->_prevScreenId == 36)
			handleForest(35, 6, 84, 100, 440, 4);
		break;
	case 36:
		if (_vm->_globals->_prevScreenId == 35)
			handleForest(36, 551, 633, 100, 440, 2);
		else if (_vm->_globals->_prevScreenId == 37)
			handleForest(36, 6, 84, 100, 440, 4);
		break;
	case 37:
		if (_vm->_globals->_prevScreenId == 36)
			handleForest(37, 551, 633, 100, 440, 1);
		else if (_vm->_globals->_prevScreenId == 38)
			handleForest(37, 392, 529, 100, 440, 2);
		break;
	case 38:
		if (_vm->_globals->_prevScreenId == 37)
			handleForest(38, 133, 252, 100, 440, 4);
		else if (_vm->_globals->_prevScreenId == 39)
			handleForest(38, 6, 84, 100, 440, 3);
		break;
	case 39:
		if (_vm->_globals->_prevScreenId == 38)
			handleForest(39, 551, 633, 100, 440, 2);
		else if (_vm->_globals->_prevScreenId == 40)
			handleForest(39, 6, 84, 100, 440, 3);
		break;
	case 40:
		if (_vm->_globals->_prevScreenId == 39)
			handleForest(40, 133, 252, 100, 440, 4);
		else if (_vm->_globals->_prevScreenId == 41)
			handleForest(40, 392, 529, 100, 440, 2);
		break;
	case 41:
		if (_vm->_globals->_prevScreenId == 40)
			handleForest(41, 551, 633, 100, 440, 1);
		else if (_vm->_globals->_prevScreenId == 17)
			handleForest(41, 6, 84, 100, 440, 3);
		break;
	case 57:
		_vm->_globals->_disableInventFl = true;
		if (_vm->_globals->_saveData->_data[svField261] == 1 && getBobAnimDataIdx(5) == 37) {
			stopBobAnimation(5);
			setBobAnimDataIdx(5, 0);
			setBobAnimation(6);
			_vm->_globals->_saveData->_data[svField261] = 2;
			_vm->_linesMan->disableZone(15);
			_vm->_soundMan->playSoundFile("SOUND75.WAV");
		}
		if (_vm->_globals->_saveData->_data[svField261] == 2 && getBobAnimDataIdx(6) == 6) {
			stopBobAnimation(6);
			setBobAnimDataIdx(6, 0);
			setBobAnimation(7);
			_vm->_linesMan->enableZone(14);
			_vm->_globals->_saveData->_data[svField261] = 3;
		}
		_vm->_globals->_disableInventFl = false;
		break;
	case 93:
		if (_vm->_globals->_saveData->_data[svField333])
			break;

		_vm->_globals->_disableInventFl = true;
		do {
			_vm->_events->refreshScreenAndEvents();
		} while (getBobAnimDataIdx(8) != 3);
		_vm->_globals->_introSpeechOffFl = true;
		_vm->_talkMan->startAnimatedCharacterDialogue("GM3.PE2");
		stopBobAnimation(8);
		_vm->_globals->_saveData->_data[svField333] = 1;
		_vm->_globals->_disableInventFl = false;
		break;
	default:
		break;
	}
}

void ObjectsManager::quickDisplayBobSprite(int idx) {
	int startPos = 10 * idx;
	if (!READ_LE_UINT16(_vm->_talkMan->_characterAnim + startPos + 4))
		return;

	int xp = READ_LE_INT16(_vm->_talkMan->_characterAnim + startPos);
	int yp = READ_LE_INT16(_vm->_talkMan->_characterAnim + startPos + 2);
	int spriteIndex = _vm->_talkMan->_characterAnim[startPos + 8];

	_vm->_graphicsMan->fastDisplay(_vm->_talkMan->_characterSprite, xp, yp, spriteIndex);
}

void ObjectsManager::initVbob(const byte *src, int idx, int xp, int yp, int frameIndex) {
	if (idx > 29)
		error("MAX_VBOB exceeded");

	VBobItem *vbob = &_vBob[idx];
	if (vbob->_displayMode <= 1) {
		vbob->_displayMode = 1;
		vbob->_xp = xp;
		vbob->_yp = yp;
		vbob->_frameIndex = frameIndex;
		vbob->_oldX = xp;
		vbob->_oldY = yp;
		vbob->_oldFrameIndex = frameIndex;
		vbob->_spriteData = src;
		vbob->_oldSpriteData = src;
		vbob->_surface = _vm->_globals->freeMemory(vbob->_surface);
	} else if (vbob->_displayMode == 2 || vbob->_displayMode == 4) {
		vbob->_displayMode = 3;
		vbob->_oldX = vbob->_xp;
		vbob->_oldY = vbob->_yp;
		vbob->_oldSpriteData = vbob->_spriteData;
		vbob->_oldFrameIndex = vbob->_frameIndex;
		vbob->_xp = xp;
		vbob->_yp = yp;
		vbob->_frameIndex = frameIndex;
		vbob->_spriteData = src;
	}
}

void ObjectsManager::disableVbob(int idx) {
	if (idx > 29)
		error("MAX_VBOB exceeded");

	VBobItem *vbob = &_vBob[idx];
	if (vbob->_displayMode <= 1)
		vbob->_displayMode = 0;
	else
		vbob->_displayMode = 4;
}

void ObjectsManager::doActionBack(int idx) {
	if (_curGestureFile != 1) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 1;
		_gestureBuf = _vm->_fileIO->loadFile("DOS.SPR");
	}

	switch (idx) {
	case 1:
		showActionAnimation(_gestureBuf, "0,1,2,3,4,5,6,7,8,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 2:
		showSpecialActionAnimationWithFlip(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,-1,", 8, false);
		break;
	case 3:
		showSpecialActionAnimation(_gestureBuf, "12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8);
		break;
	case 4:
		showActionAnimation(_gestureBuf, "0,1,2,3,4,5,6,7,8,8,8,8,8,8,9,10,11,12,13,12,11,12,13,12,11,12,13,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 5:
		showSpecialActionAnimationWithFlip(_gestureBuf, "15,16,17,18,19,20,21,-1,", 8, false);
		break;
	case 6:
		showSpecialActionAnimation(_gestureBuf, "20,19,18,17,16,15,-1,", 8);
		break;
	case 7:
		showSpecialActionAnimationWithFlip(_gestureBuf, "15,16,17,18,19,20,21,22,23,24,-1,", 8, false);
		break;
	case 8:
		showSpecialActionAnimation(_gestureBuf, "23,22,21,20,19,18,17,16,15,-1,", 8);
		break;
	case 9:
		showSpecialActionAnimationWithFlip(_gestureBuf, "15,16,17,18,19,20,21,22,23,24,-1,", 8, false);
		break;
	case 10:
		showSpecialActionAnimation(_gestureBuf, "23,22,21,20,19,18,17,16,15,-1,", 8);
		break;
	default:
		break;
	}
}

void ObjectsManager::doActionRight(int idx) {
	if (_curGestureFile != 3) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 3;
		_gestureBuf = _vm->_fileIO->loadFile("PROFIL.SPR");
	}

	switch (idx) {
	case 1:
		showActionAnimation(_gestureBuf, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 8, false);
		break;
	case 2:
		showSpecialActionAnimationWithFlip(_gestureBuf, "1,2,3,4,5,6,7,8,-1,", 8, false);
		break;
	case 3:
		showSpecialActionAnimation(_gestureBuf, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 8);
		break;
	case 4:
		showActionAnimation(_gestureBuf, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 8, false);
		break;
	case 5:
		showSpecialActionAnimationWithFlip(_gestureBuf, "23,24,25,-1,", 8, false);
		break;
	case 6:
		showSpecialActionAnimation(_gestureBuf, "24,23,-1,", 8);
		break;
	case 7:
		showSpecialActionAnimationWithFlip(_gestureBuf, "23,24,25,26,27,-1,", 8, false);
		break;
	case 8:
		showSpecialActionAnimation(_gestureBuf, "26,25,24,23,-1,", 8);
		break;
	case 9:
		showSpecialActionAnimationWithFlip(_gestureBuf, "23,24,25,26,27,28,29,-1,", 8, false);
		break;
	case 10:
		showSpecialActionAnimation(_gestureBuf, "28,27,26,25,24,23,-1,", 8);
		break;
	default:
		break;
	}
}

void ObjectsManager::doActionDiagRight(int idx) {
	if (_curGestureFile != 4) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 4;
		_gestureBuf = _vm->_fileIO->loadFile("3Q.SPR");
	}

	switch (idx) {
	case 1:
		showActionAnimation(_gestureBuf, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 2:
		showSpecialActionAnimationWithFlip(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 8, false);
		break;
	case 3:
		showSpecialActionAnimation(_gestureBuf, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8);
		break;
	case 4:
		showActionAnimation(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 5:
		showSpecialActionAnimationWithFlip(_gestureBuf, "15,16,17,18,-1,", 8, false);
		break;
	case 6:
		showSpecialActionAnimation(_gestureBuf, "17,16,15,-1,", 8);
		break;
	case 7:
		showSpecialActionAnimationWithFlip(_gestureBuf, "15,16,17,18,19,20,-1,", 8, false);
		break;
	case 8:
		showSpecialActionAnimation(_gestureBuf, "19,18,17,16,15,-1,", 8);
		break;
	case 9:
		showSpecialActionAnimationWithFlip(_gestureBuf, "15,16,17,18,19,20,21,-1,", 8, false);
		break;
	case 10:
		showSpecialActionAnimation(_gestureBuf, "20,19,18,17,15,-1,", 8);
		break;
	default:
		break;
	}
}

void ObjectsManager::doActionFront(int idx) {
	if (_curGestureFile != 2) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 2;
		_gestureBuf = _vm->_fileIO->loadFile("FACE.SPR");
	}

	switch (idx) {
	case 1:
		showActionAnimation(_gestureBuf, "0,1,2,3,4,5,6,7,9,9,9,9,9,9,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 2:
		showSpecialActionAnimationWithFlip(_gestureBuf, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,15,-1,", 8, false);
		break;
	case 3:
		showSpecialActionAnimation(_gestureBuf, "14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 8);
		break;
	case 4:
		showActionAnimation(_gestureBuf, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	default:
		break;
	}
}

void ObjectsManager::doActionDiagLeft(int idx) {
	if (_curGestureFile != 4) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 4;
		_gestureBuf = _vm->_fileIO->loadFile("3Q.SPR");
	}

	switch (idx) {
	case 1:
		showActionAnimation(_gestureBuf, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 8, true);
		break;
	case 2:
		showSpecialActionAnimationWithFlip(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 8, true);
		break;
	case 3:
		showSpecialActionAnimation(_gestureBuf, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8);
		break;
	case 4:
		showActionAnimation(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8, true);
		break;
	case 5:
		showSpecialActionAnimationWithFlip(_gestureBuf, "15,16,17,18,-1,", 8, true);
		break;
	case 6:
		showSpecialActionAnimation(_gestureBuf, "17,16,15,-1,", 8);
		break;
	case 7:
		showSpecialActionAnimationWithFlip(_gestureBuf, "15,16,17,18,19,20,-1,", 8, true);
		break;
	case 8:
		showSpecialActionAnimation(_gestureBuf, "19,18,17,16,15,-1,", 8);
		break;
	case 9:
		showSpecialActionAnimationWithFlip(_gestureBuf, "15,16,17,18,19,20,21,-1,", 8, true);
		break;
	case 10:
		showSpecialActionAnimation(_gestureBuf, "20,19,18,17,15,-1,", 8);
		break;
	default:
		break;
	}
}

void ObjectsManager::doActionLeft(int idx) {
	if (_curGestureFile != 3) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 3;
		_gestureBuf = _vm->_fileIO->loadFile("PROFIL.SPR");
	}

	switch (idx) {
	case 1:
		showActionAnimation(_gestureBuf, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 8, true);
		break;
	case 2:
		showSpecialActionAnimationWithFlip(_gestureBuf, "1,2,3,4,5,6,7,8,-1,", 8, true);
		break;
	case 3:
		showSpecialActionAnimation(_gestureBuf, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 8);
		break;
	case 4:
		showActionAnimation(_gestureBuf, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 8, true);
		break;
	case 5:
		showSpecialActionAnimationWithFlip(_gestureBuf, "23,24,25,-1,", 8, true);
		break;
	case 6:
		showSpecialActionAnimation(_gestureBuf, "24,23,-1,", 8);
		break;
	case 7:
		showSpecialActionAnimationWithFlip(_gestureBuf, "23,24,25,26,27,-1,", 8, true);
		break;
	case 8:
		showSpecialActionAnimation(_gestureBuf, "26,25,24,23,-1,", 8);
		break;
	case 9:
		showSpecialActionAnimationWithFlip(_gestureBuf, "23,24,25,26,27,28,29,-1,", 8, true);
		break;
	case 10:
		showSpecialActionAnimation(_gestureBuf, "28,27,26,25,24,23,-1,", 8);
		break;
	default:
		break;
	}
}

void ObjectsManager::setAndPlayAnim(int idx, int animIdx, int destPosi, bool animAction) {
	// Set Hopkins animation and position
	setBobAnimation(idx);
	setBobAnimDataIdx(idx, animIdx);

	// Make Hopkins walk to the expected place
	do {
		_vm->_events->refreshScreenAndEvents();
	} while (destPosi != getBobAnimDataIdx(idx));

	if (!animAction)
		stopBobAnimation(idx);
	else {
		BobItem *bob = &_bob[idx];
		_vm->_graphicsMan->fastDisplay(bob->_spriteData, bob->_oldX, bob->_oldY, bob->_frameIndex);
		stopBobAnimation(idx);
		_vm->_events->refreshScreenAndEvents();
	}
}

int ObjectsManager::getBobAnimDataIdx(int idx) {
	return _bob[idx]._animDataIdx / 5;
}

void ObjectsManager::setBobAnimDataIdx(int idx, int animIdx) {
	BobItem *bob = &_bob[idx];
	bob->_animDataIdx = 5 * animIdx;
	bob->_moveChange1 = 0;
	bob->_moveChange2 = 0;
}

/**
 * Set Hopkins animation
 */
void ObjectsManager::setBobAnimation(int idx) {
	assert(idx < 36);
	BobItem *bob = &_bob[idx];
	if (!bob->_disabledAnimationFl)
		return;

	bob->_disabledAnimationFl = false;
	bob->_animDataIdx = 5;
	bob->_frameIndex = 250;
	bob->_moveChange1 = 0;
	bob->_moveChange2 = 0;
}

/**
 * Stop Hopkins animation
 */
void ObjectsManager::stopBobAnimation(int idx) {
	assert(idx < 36);
	_bob[idx]._disabledAnimationFl = true;
}

/**
 * Get X position
 */
int ObjectsManager::getBobPosX(int idx) {
	return _bob[idx]._xp;
}

void ObjectsManager::loadLinkFile(const Common::Path &file, bool skipDetails) {
	Common::File f;
	Common::Path filename = file.append(".LNK");
	bool fileFoundFl = false;
	byte *ptr = _vm->_fileIO->searchCat(filename, RES_LIN, fileFoundFl);
	size_t nbytes = _vm->_fileIO->_catalogSize;
	if (!fileFoundFl) {
		if (!f.open(filename))
			error("Error opening file - %s", filename.toString().c_str());

		nbytes = f.size();
		ptr = _vm->_globals->allocMemory(nbytes);
		if (ptr == nullptr)
			error("INILINK");
		_vm->_fileIO->readStream(f, ptr, nbytes);
		f.close();
	}
	if (!skipDetails) {
		for (int idx = 0; idx < 500; ++idx)
			_vm->_globals->_spriteSize[idx] = READ_LE_INT16((uint16 *)ptr + idx);

		resetHidingItems();

		Common::Path filename2(Common::String((const char *)ptr + 1000));
		if (!filename2.empty()) {
			fileFoundFl = false;
			_hidingItemData[1] = _vm->_fileIO->searchCat(filename2, RES_SLI, fileFoundFl);

			if (!fileFoundFl) {
				_hidingItemData[1] = _vm->_fileIO->loadFile(filename2);
			} else {
				_hidingItemData[1] = _vm->_fileIO->loadFile("RES_SLI.RES");
			}

			int curDataCacheId = 60;
			byte *curDataPtr = ptr + 1000;
			for (int hidingIdx = 0; hidingIdx <= 21; hidingIdx++) {
				HidingItem *hid = &_hidingItem[hidingIdx];
				int curSpriteId = READ_LE_INT16(curDataPtr + 2 * curDataCacheId);
				hid->_spriteIndex = curSpriteId;
				hid->_x = READ_LE_INT16(curDataPtr + 2 * curDataCacheId + 2);
				hid->_y = READ_LE_INT16(curDataPtr + 2 * curDataCacheId + 4);
				hid->_yOffset = READ_LE_INT16(curDataPtr + 2 * curDataCacheId + 8);

				if (!_hidingItemData[1]) {
					hid->_useCount = 0;
				} else {
					hid->_spriteData = _hidingItemData[1];
					hid->_width = getWidth(_hidingItemData[1], curSpriteId);
					hid->_height = getHeight(_hidingItemData[1], curSpriteId);
					hid->_useCount = 1;
				}
				if (!hid->_x && !hid->_y && !hid->_spriteIndex)
					hid->_useCount = 0;

				curDataCacheId += 5;
			}
			enableHidingBehavior();
		}
	}

	_vm->_linesMan->resetLines();
	for (size_t idx = 0; idx < nbytes - 3; idx++) {
		if (READ_BE_UINT24(&ptr[idx]) == MKTAG24('O', 'B', '2')) {
			byte *curDataPtr = &ptr[idx + 4];
			int lineDataIdx = 0;
			int curLineIdx = 0;
			_vm->_linesMan->resetLinesNumb();
			Directions curDirection;
			do {
				curDirection = (Directions)READ_LE_INT16(curDataPtr + 2 * lineDataIdx);
				if (curDirection != DIR_NONE) {
					_vm->_linesMan->addLine(
					    curLineIdx,
					    curDirection,
					    READ_LE_INT16(curDataPtr + 2 * lineDataIdx + 2),
					    READ_LE_INT16(curDataPtr + 2 * lineDataIdx + 4),
					    READ_LE_INT16(curDataPtr + 2 * lineDataIdx + 6),
					    READ_LE_INT16(curDataPtr + 2 * lineDataIdx + 8));
				}
				lineDataIdx += 5;
				++curLineIdx;
			} while (curDirection != DIR_NONE);
			_vm->_linesMan->initRoute();
		}
	}

	if (!skipDetails) {
		for (size_t idx = 0; idx < nbytes - 3; idx++) {
			if (READ_BE_UINT24(&ptr[idx]) == MKTAG24('Z', 'O', '2')) {
				byte *curDataPtr = &ptr[idx + 4];
				int curDataIdx = 0;
				for (int i = 1; i <= 100; i++) {
					ZoneItem *curZone = &_vm->_linesMan->_zone[i];
					curZone->_destX = 0;
					curZone->_destY = 0;
					curZone->_spriteIndex = 0;
					curZone->_verbFl1 = 0;
					curZone->_verbFl2 = 0;
					curZone->_verbFl3 = 0;
					curZone->_verbFl4 = 0;
					curZone->_verbFl5 = 0;
					curZone->_verbFl6 = 0;
					curZone->_verbFl7 = 0;
					curZone->_verbFl8 = 0;
					curZone->_verbFl9 = 0;
					curZone->_verbFl10 = 0;
					curZone->_messageId = 0;
				}

				int curLineIdx = 0;
				for (;;) {
					int bobZoneId = READ_LE_INT16(curDataPtr + 2 * curDataIdx);
					if (bobZoneId != -1) {
						_vm->_linesMan->addZoneLine(
						    curLineIdx,
						    READ_LE_INT16(curDataPtr + 2 * curDataIdx + 2),
						    READ_LE_INT16(curDataPtr + 2 * curDataIdx + 4),
						    READ_LE_INT16(curDataPtr + 2 * curDataIdx + 6),
						    READ_LE_INT16(curDataPtr + 2 * curDataIdx + 8),
						    bobZoneId);
						_vm->_linesMan->_zone[bobZoneId]._enabledFl = true;
					}
					curDataIdx += 5;
					++curLineIdx;
					if (bobZoneId == -1)
						break;
				}
				for (int i = 1; i <= 100; i++) {
					ZoneItem *curZone = &_vm->_linesMan->_zone[i];
					curZone->_destX = READ_LE_INT16(curDataPtr + 2 * curDataIdx);
					curZone->_destY = READ_LE_INT16(curDataPtr + 2 * curDataIdx + 2);
					curZone->_spriteIndex = READ_LE_INT16(curDataPtr + 2 * curDataIdx + 4);
					curDataIdx += 3;
				}

				byte *verbData = ptr + idx + (10 * curLineIdx + 606) + 4;
				for (int i = 1; i <= 100; i++) {
					int j = (i - 1) * 10;
					ZoneItem *curZone = &_vm->_linesMan->_zone[i];
					curZone->_verbFl1 = verbData[j];
					curZone->_verbFl2 = verbData[j + 1];
					curZone->_verbFl3 = verbData[j + 2];
					curZone->_verbFl4 = verbData[j + 3];
					curZone->_verbFl5 = verbData[j + 4];
					curZone->_verbFl6 = verbData[j + 5];
					curZone->_verbFl7 = verbData[j + 6];
					curZone->_verbFl8 = verbData[j + 7];
					curZone->_verbFl9 = verbData[j + 8];
					curZone->_verbFl10 = verbData[j + 9];
				}
				int dep = 1010;
				for (int i = 1; i <= 100; i++) {
					_vm->_linesMan->_zone[i]._messageId = READ_LE_INT16(verbData + dep);
					dep += 2;
				}
				_vm->_linesMan->initSquareZones();
			}
		}
	}
	_vm->_globals->freeMemory(ptr);
}

void ObjectsManager::sceneSpecialIni() {
	switch (_vm->_globals->_screenId) {
	case 17:
		if (_vm->_globals->_prevScreenId == 20) {
			_vm->_globals->_disableInventFl = true;
			_vm->_graphicsMan->setColorPercentage(252, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage(253, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage(251, 100, 100, 100);
			_vm->_graphicsMan->setColorPercentage(254, 0, 0, 0);
			for (int i = 0; i <= 4; i++)
				_vm->_events->refreshScreenAndEvents();
			_vm->_graphicsMan->fadeInLong();
			animateSprite(0);
			for (int i = 0; i <= 4; i++)
				_vm->_events->refreshScreenAndEvents();
			initVbob(_vm->_globals->_levelSpriteBuf, 5, 15, 28, 1);
			_vm->_fontMan->hideText(9);
			bool displayedTxtFl = false;
			if (!_vm->_soundMan->_textOffFl) {
				_vm->_fontMan->initTextBuffers(9, 383, _vm->_globals->_textFilename, 220, 72, 6, 36, 253);
				_vm->_fontMan->showText(9);
				displayedTxtFl = true;
			}
			if (!_vm->_soundMan->_voiceOffFl)
				_vm->_soundMan->mixVoice(383, 4, displayedTxtFl);
			_vm->_globals->_saveData->_data[svField270] = 1;
			_vm->_globals->_saveData->_data[svField300] = 1;
			_vm->_globals->_saveData->_data[svField320] = 1;
			if (_vm->_soundMan->_voiceOffFl) {
				for (int i = 0; i <= 199; i++)
					_vm->_events->refreshScreenAndEvents();
			}
			_vm->_fontMan->hideText(9);
			disableVbob(5);
			for (int i = 0; i <= 3; i++)
				_vm->_events->refreshScreenAndEvents();
			_vm->_graphicsMan->_noFadingFl = true;
			_vm->_globals->_disableInventFl = false;
		}
		break;

	case 18:
		if (_vm->_globals->_prevScreenId == 17) {
			_vm->_events->_mouseSpriteId = 4;
			for (int i = 0; i <= 4; i++)
				_vm->_events->refreshScreenAndEvents();
			_vm->_graphicsMan->fadeInLong();
			_vm->_globals->_eventMode = EVENTMODE_IGNORE;
			_vm->_globals->_disableInventFl = false;
			_vm->_graphicsMan->_noFadingFl = true;
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkMan->startAnimatedCharacterDialogue("MAGE1.pe2");
			_vm->_graphicsMan->_noFadingFl = true;
			_vm->_globals->_disableInventFl = false;
		}
		break;

	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
	case 41:
		_vm->_linesMan->_bobZone[20] = 1;
		_vm->_linesMan->_bobZone[21] = 2;
		_vm->_linesMan->_bobZone[22] = 3;
		_vm->_linesMan->_bobZone[23] = 4;
		_vm->_linesMan->_bobZoneFl[20] = true;
		_vm->_linesMan->_bobZoneFl[21] = true;
		_vm->_linesMan->_bobZoneFl[22] = true;
		_vm->_linesMan->_bobZoneFl[23] = true;
		enableVerb(20, 5);
		enableVerb(21, 5);
		enableVerb(22, 5);
		enableVerb(23, 5);
		_vm->_linesMan->_zone[20]._messageId = 30;
		_vm->_linesMan->_zone[21]._messageId = 30;
		_vm->_linesMan->_zone[22]._messageId = 30;
		_vm->_linesMan->_zone[23]._messageId = 30;
		for (int i = svField200; i <= svField214; i++) {
			if (_vm->_globals->_saveData->_data[i] != 2)
				_vm->_globals->_saveData->_data[i] = 0;
		}
		break;

	case 73:
		if (!_vm->_globals->_saveData->_data[svSecondElevatorAvailableFl]) {
			resetHidingUseCount(0);
			resetHidingUseCount(1);
		}
		break;

	case 93:
		if (!_vm->_globals->_saveData->_data[svField333])
			setBobAnimation(8);
		break;

	default:
		break;
	}
}

void ObjectsManager::setMultiBobAnim(int idx1, int idx2, int anim1Idx, int anim2Idx) {
	if (idx1 != -1)
		setBobAnimation(idx1);
	if (idx2 != -1)
		setBobAnimation(idx2);
	if (idx1 != -1)
		setBobAnimDataIdx(idx1, anim1Idx);
	if (idx2 != -1)
		setBobAnimDataIdx(idx2, anim2Idx);
}

void ObjectsManager::checkEventBobAnim(int idx, int animIdx, int animDataIdx, int a4) {
	_vm->_events->_curMouseButton = 0;
	_vm->_events->_mouseButton = 0;

	if (a4 != 3) {
		setBobAnimation(idx);
		setBobAnimDataIdx(idx, animIdx);
	}

	do {
		_vm->_events->refreshScreenAndEvents();
		if (_vm->_events->_curMouseButton)
			break;
	} while (animDataIdx != getBobAnimDataIdx(idx));
	if (!a4)
		stopBobAnimation(idx);
}

void ObjectsManager::disableVerb(int idx, int a2) {
	ZoneItem *curZone = &_vm->_linesMan->_zone[idx];
	switch (a2) {
	case 6:
	case 16:
		curZone->_verbFl1 = 0;
		break;
	case 7:
		curZone->_verbFl2 = 0;
		break;
	case 5:
	case 8:
		curZone->_verbFl3 = 0;
		break;
	case 9:
	case 17:
	case 24:
		curZone->_verbFl4 = 0;
		break;
	case 10:
	case 18:
		curZone->_verbFl5 = 0;
		break;
	case 11:
	case 19:
		curZone->_verbFl6 = 0;
		break;
	case 12:
	case 20:
		curZone->_verbFl7 = 0;
		break;
	case 13:
	case 22:
		curZone->_verbFl8 = 0;
		break;
	case 14:
	case 21:
	case 25:
		curZone->_verbFl9 = 0;
		break;
	case 15:
		curZone->_verbFl10 = 0;
		break;
	default:
		break;
	}
	_changeVerbFl = true;
}

void ObjectsManager::enableVerb(int idx, int a2) {
	ZoneItem *curZone = &_vm->_linesMan->_zone[idx];

	switch (a2) {
	case 5:
		curZone->_verbFl3 = 2;
		break;
	case 6:
		curZone->_verbFl1 = 1;
		break;
	case 7:
		curZone->_verbFl2 = 1;
		break;
	case 8:
		curZone->_verbFl3 = 1;
		break;
	case 9:
		curZone->_verbFl4 = 1;
		break;
	case 10:
		curZone->_verbFl5 = 1;
		break;
	case 11:
		curZone->_verbFl6 = 1;
		break;
	case 12:
		curZone->_verbFl7 = 1;
		break;
	case 13:
		curZone->_verbFl8 = 1;
		break;
	case 14:
		curZone->_verbFl8 = 1;
		break;
	case 15:
		curZone->_verbFl9 = 1;
		break;
	case 16:
		curZone->_verbFl1 = 2;
		break;
	case 17:
		curZone->_verbFl4 = 2;
		break;
	case 18:
		curZone->_verbFl5 = 2;
		break;
	case 19:
		curZone->_verbFl6 = 2;
		break;
	case 20:
		curZone->_verbFl7 = 2;
		break;
	case 21:
		curZone->_verbFl9 = 2;
		break;
	case 22:
		curZone->_verbFl8 = 2;
		break;
	case 24:
		curZone->_verbFl4 = 3;
		break;
	case 25:
		curZone->_verbFl9 = 2;
		break;
	default:
		break;
	}
}

void ObjectsManager::showActionAnimation(const byte *spriteData, const Common::String &actionStr, int speed, bool flipFl) {
	Common::String tmpStr = "";
	int realSpeed = speed;
	if (_vm->_globals->_speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals->_speed == 3)
		realSpeed = speed / 3;
	const byte *oldSpriteData = _sprite[0]._spriteData;
	int spriteIndex = _sprite[0]._spriteIndex;
	bool oldFlipFl = _sprite[0]._flipFl;
	_sprite[0]._flipFl = flipFl;

	int idx = 0;
	for (int strPos = 0; ; strPos++) {
		bool tokenCompleteFl = false;
		char curChar = actionStr[strPos];
		if (curChar == ',') {
			idx = atoi(tmpStr.c_str());
			tmpStr = "";
			tokenCompleteFl = true;
		} else {
			tmpStr += curChar;
		}

		if (tokenCompleteFl) {
			if (idx == -1) {
				_sprite[0]._spriteData = oldSpriteData;
				_sprite[0]._spriteIndex = spriteIndex;
				_sprite[0]._flipFl = oldFlipFl;
			} else {
				_sprite[0]._spriteData = spriteData;
				_sprite[0]._spriteIndex = idx;
			}
			for (int i = 0; i < realSpeed; i++)
				_vm->_events->refreshScreenAndEvents();
			if (idx == -1)
				break;
		}
	}
}

void ObjectsManager::showSpecialActionAnimationWithFlip(const byte *spriteData, const Common::String &animationSeq, int speed, bool flipFl) {
	Common::String tmpStr = "";

	int realSpeed = speed;
	if (_vm->_globals->_speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals->_speed == 3)
		realSpeed = speed / 3;

	_oldSpriteData = _sprite[0]._spriteData;
	_oldSpriteIndex = _sprite[0]._spriteIndex;
	_oldFlipFl = _sprite[0]._flipFl;
	_sprite[0]._flipFl = flipFl;

	uint strPos = 0;
	int spriteIndex = 0;
	do {
		bool completeTokenFl = false;
		do {
			char nextChar = animationSeq[strPos];
			if ((animationSeq[strPos] == ',') || (strPos == animationSeq.size() - 1)) {
				// Safeguard: if the sequence doesn't end with a coma, simulate it's present.
				if (animationSeq[strPos] != ',')
					tmpStr += nextChar;
				spriteIndex = atoi(tmpStr.c_str());
				tmpStr = "";
				completeTokenFl = true;
			} else {
				tmpStr += nextChar;
			}
			++strPos;
		} while (!completeTokenFl);

		if (spriteIndex != -1) {
			_sprite[0]._spriteData = spriteData;
			_sprite[0]._spriteIndex = spriteIndex;
		}
		for (int i = 0; i < realSpeed; i++)
			_vm->_events->refreshScreenAndEvents();
	} while (spriteIndex != -1);
}

void ObjectsManager::showSpecialActionAnimation(const byte *spriteData, const Common::String &animString, int speed) {
	Common::String tmpStr = "";
	int realSpeed = speed;
	if (_vm->_globals->_speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals->_speed == 3)
		realSpeed = speed / 3;

	int spriteIndex = 0;

	for (int idx = 0; ; idx++) {
		bool completeTokenFl = false;
		char nextChar = animString[idx];
		if (nextChar == ',') {
			spriteIndex = atoi(tmpStr.c_str());
			tmpStr = "";
			completeTokenFl = true;
		} else {
			tmpStr += nextChar;
		}

		if (completeTokenFl) {
			if (spriteIndex == -1) {
				_sprite[0]._spriteData = _oldSpriteData;
				_sprite[0]._spriteIndex = _oldSpriteIndex;
				_sprite[0]._flipFl = _oldFlipFl;
			} else {
				_sprite[0]._spriteData = spriteData;
				_sprite[0]._spriteIndex = spriteIndex;
			}

			for (int i = 0; i < realSpeed; i++)
				_vm->_events->refreshScreenAndEvents();

			if (spriteIndex == -1)
				break;
		}
	}
}

void ObjectsManager::handleForest(int screenId, int minX, int maxX, int minY, int maxY, int idx) {
	int savegameIdx = screenId;
	if (_vm->_globals->_screenId != screenId)
		return;

	switch (_vm->_globals->_screenId) {
	case 35:
		if (idx > 2)
			savegameIdx = 201;
		else
			savegameIdx = 200;
		break;
	case 36:
		if (idx > 2)
			savegameIdx = 203;
		else
			savegameIdx = 202;
		break;
	case 37:
		if (idx > 2)
			savegameIdx = 205;
		else
			savegameIdx = 204;
		break;
	case 38:
		if (idx > 2)
			savegameIdx = 207;
		else
			savegameIdx = 206;
		break;
	case 39:
		if (idx > 2)
			savegameIdx = 209;
		else
			savegameIdx = 208;
		break;
	case 40:
		if (idx > 2)
			savegameIdx = 211;
		else
			savegameIdx = 210;
		break;
	case 41:
		if (idx > 2)
			savegameIdx = 213;
		else
			savegameIdx = 212;
		break;
	default:
		break;
	}

	if (_vm->_globals->_saveData->_data[savegameIdx] == 2)
		return;

	if (_vm->_globals->_saveData->_data[savegameIdx]) {
		if (_vm->_globals->_saveData->_data[savegameIdx] == 1) {
			if (((idx == 1 || idx == 2) && getBobAnimDataIdx(idx) == 26) || ((idx == 3 || idx == 4) && getBobAnimDataIdx(idx) == 27)) {
				_vm->_dialog->disableInvent();
				_vm->_soundMan->playSample(1);
				_vm->_globals->_saveData->_data[savegameIdx] = 4;
			}
		}
		if (_vm->_globals->_saveData->_data[savegameIdx] == 4) {
			if (idx >= 1 && idx <= 4 && getBobAnimDataIdx(idx) > 30)
				_vm->_globals->_saveData->_data[savegameIdx] = 3;
		}
		if (_vm->_globals->_saveData->_data[savegameIdx] == 3) {
			_vm->_graphicsMan->_fadingFl = true;
			_vm->_animMan->playAnim("CREVE2.ANM", "CREVE2.ANM", 100, 24, 500);
			_vm->_globals->_exitId = 150;
			_vm->_graphicsMan->_noFadingFl = true;
			hideBob(1);
			hideBob(2);
			hideBob(3);
			hideBob(4);
		}
	} else if (minX < getSpriteX(0)
	           && maxX > getSpriteX(0)
	           && minY < getSpriteY(0)
	           && maxY > getSpriteY(0)) {
		if (idx >= 1 && idx <= 4)
			setBobAnimation(idx);
		_vm->_globals->_saveData->_data[savegameIdx] = 1;
	}
}

void ObjectsManager::lockAnimX(int idx, int x) {
	_lockedAnims[idx]._enableFl = true;
	_lockedAnims[idx]._posX = x;
}

/**
 * Game scene control method
 */
void ObjectsManager::sceneControl(const Common::Path &backgroundFile, const Common::Path &linkFile,
							   const Common::Path &animFile, const Common::Path &s4, int soundNum, bool initializeScreen) {
	_vm->_dialog->_inventFl = false;
	_vm->_events->_gameKey = KEY_NONE;
	_vm->_dialog->enableInvent();
	_vm->_graphicsMan->_scrollOffset = 0;
	_vm->_globals->_cityMapEnabledFl = false;
	_vm->_globals->_eventMode = EVENTMODE_IGNORE;
	_vm->_soundMan->playSound(soundNum);
	_vm->_linesMan->_route = nullptr;
	_vm->_globals->_freezeCharacterFl = true;
	_vm->_globals->_exitId = 0;
	if (!backgroundFile.empty())
		_vm->_graphicsMan->loadImage(backgroundFile);
	if (!linkFile.empty())
		loadLinkFile(linkFile);
	if (!animFile.empty())
		_vm->_animMan->loadAnim(animFile);
	_vm->_graphicsMan->displayAllBob();
	if (!s4.empty()) {
		if (initializeScreen)
			_vm->_graphicsMan->initScreen(s4, 0, initializeScreen);
		else
			_vm->_graphicsMan->initScreen(s4, 2, initializeScreen);
	}
	_vm->_events->mouseOn();
	if (_vm->_globals->_screenId == 61) {
		addStaticSprite(_vm->_globals->_characterSpriteBuf, Common::Point(330, 418), 0, 60, 0, false, 34, 190);
		animateSprite(0);
		_vm->_linesMan->_route = nullptr;
		computeAndSetSpriteSize();
	}
	_vm->_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(254, 0, 0, 0);
	_vm->_events->changeMouseCursor(4);
	for (int i = 0; i <= 4; i++)
		_vm->_events->refreshScreenAndEvents();
	_vm->_graphicsMan->fadeInLong();
	if (_vm->_globals->_screenId == 61) {
		_vm->_animMan->playSequence("OUVRE.SEQ", 10, 4, 10, false, false);
		stopBobAnimation(3);
		_vm->_globals->_checkDistanceFl = true;
		_oldCharacterPosX = getSpriteX(0);
		_oldDirection = DIR_NONE;
		_homeRateCounter = 0;
		_vm->_linesMan->_route = nullptr;
		_vm->_linesMan->_route = _vm->_linesMan->findRoute(getSpriteX(0), getSpriteY(0), 330, 345);
		_vm->_globals->_checkDistanceFl = true;
		do {
			goHome();
			_vm->_events->refreshScreenAndEvents();
		} while (_vm->_linesMan->_route);
		setSpriteIndex(0, 64);
	}
	do {
		int mouseButton = _vm->_events->getMouseButton();
		if (mouseButton == 1) {
			handleLeftButton();
			mouseButton = 1;
		} else if (mouseButton == 2)
			handleRightButton();
		_vm->_dialog->testDialogOpening();
		_vm->_linesMan->checkZone();
		if (_vm->_globals->_actionMoveTo)
			paradise();
		if (!_vm->_globals->_exitId)
			_vm->_events->refreshScreenAndEvents();

		if (_vm->_globals->_exitId)
			break;
	} while (!_vm->shouldQuit());
	if (_vm->shouldQuit())
		return;

	_vm->_graphicsMan->fadeOutLong();
	if (!animFile.empty())
		_vm->_graphicsMan->endDisplayBob();
	if (_vm->_globals->_screenId == 61)
		removeSprite(0);
	clearScreen();
	_vm->_globals->_eventMode = EVENTMODE_DEFAULT;
}

/**
 * Game scene control method
 */
void ObjectsManager::sceneControl2(const Common::Path &backgroundFile, const Common::Path &linkFile,
								const Common::Path &animFile, const Common::Path &s4, int soundNum, bool initializeScreen) {
	_vm->_dialog->_inventFl = false;
	_vm->_events->_gameKey = KEY_NONE;
	_verb = 4;
	_vm->_graphicsMan->_scrollOffset = 0;
	_vm->_dialog->enableInvent();
	_vm->_globals->_cityMapEnabledFl = false;
	_vm->_graphicsMan->_noFadingFl = false;
	_vm->_globals->_freezeCharacterFl = false;
	_vm->_globals->_exitId = 0;
	_vm->_globals->_checkDistanceFl = true;
	_vm->_soundMan->playSound(soundNum);
	_vm->_globals->_eventMode = EVENTMODE_IGNORE;
	if (!backgroundFile.empty())
		_vm->_graphicsMan->loadImage(backgroundFile);
	if (!linkFile.empty())
		loadLinkFile(linkFile);
	if (!animFile.empty()) {
		_vm->_animMan->loadAnim(animFile);
		_vm->_graphicsMan->displayAllBob();
	}
	if (!s4.empty()) {
		if (initializeScreen)
			_vm->_graphicsMan->initScreen(s4, 0, initializeScreen);
		else
			_vm->_graphicsMan->initScreen(s4, 2, initializeScreen);
	}
	_vm->_events->mouseOn();
	_vm->_events->_mouseCursorId = 4;
	_vm->_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(254, 0, 0, 0);
	if (_vm->_globals->_characterType != CHARACTER_HOPKINS && !_vm->_globals->_saveData->_data[svAlternateSpriteFl] && !_vm->_globals->_saveData->_data[svField356]) {
		_vm->_globals->_characterSpriteBuf = _vm->_fileIO->loadFile("PERSO.SPR");
		_vm->_globals->_characterType = CHARACTER_HOPKINS;
	}

	if (_vm->_globals->_characterType == CHARACTER_HOPKINS && _vm->_globals->_saveData->_data[svAlternateSpriteFl] == 1) {
		_vm->_globals->_characterSpriteBuf = _vm->_fileIO->loadFile("HOPFEM.SPR");
		_vm->_globals->_characterType = CHARACTER_HOPKINS_CLONE;
	}

	if (_vm->_globals->_characterType != CHARACTER_SAMANTHA && _vm->_globals->_saveData->_data[svField356] == 1) {
		_vm->_globals->_characterSpriteBuf = _vm->_fileIO->loadFile("PSAMAN.SPR");
		_vm->_globals->_characterType = CHARACTER_SAMANTHA;
	}
	_vm->_globals->loadCharacterData();
	switch (_vm->_globals->_characterType) {
	case CHARACTER_HOPKINS:
		addStaticSprite(_vm->_globals->_characterSpriteBuf, _characterPos, 0, _startSpriteIndex, 0, false, 34, 190);
		break;
	case CHARACTER_HOPKINS_CLONE:
		addStaticSprite(_vm->_globals->_characterSpriteBuf, _characterPos, 0, _startSpriteIndex, 0, false, 28, 155);
		break;
	case CHARACTER_SAMANTHA:
		addStaticSprite(_vm->_globals->_characterSpriteBuf, _characterPos, 0, _startSpriteIndex, 0, false, 20, 127);
		break;
	default:
		break;
	}
	_vm->_events->setMouseXY(_characterPos);
	if (_vm->_graphicsMan->_largeScreenFl)
		_vm->_graphicsMan->_scrollPosX = (int16)getSpriteX(0) - 320;
	computeAndSetSpriteSize();
	animateSprite(0);
	enableHidingBehavior();
	_vm->_linesMan->_route = nullptr;
	computeAndSetSpriteSize();
	sceneSpecialIni();
	_vm->_events->_mouseSpriteId = 4;
	_oldCharacterPosX = _characterPos.x;
	_oldCharacterPosY = _characterPos.y;
	_oldDirection = DIR_NONE;
	_homeRateCounter = 0;

	for (int idx = 0; idx < 5; ++idx)
		_vm->_events->refreshScreenAndEvents();

	_vm->_globals->_eventMode = EVENTMODE_IGNORE;
	if (!_vm->_graphicsMan->_noFadingFl)
		_vm->_graphicsMan->fadeInLong();
	_vm->_graphicsMan->_noFadingFl = false;
	_vm->_events->changeMouseCursor(4);

	int xCheck = 0;
	int yCheck = 0;

	bool breakFlag = false;
	while (!_vm->shouldQuit() && !breakFlag) {
		int mouseButtons = _vm->_events->getMouseButton();
		if (mouseButtons) {
			if (mouseButtons == 1) {
				if (_verb == 16 && _vm->_events->_mouseCursorId == 16) {
					int xp = _vm->_events->getMouseX();
					int yp = _vm->_events->getMouseY();

					if ((xCheck == xp) && (yCheck == yp)) {
						_vm->_linesMan->_route = nullptr;
						paradise();
						if (_vm->_globals->_exitId)
							breakFlag = true;
					}
					xCheck = xp;
					yCheck = yp;
				}
				handleLeftButton();
			} else if (mouseButtons == 2) {
				handleRightButton();
			}
		}
		if (!_vm->_globals->_exitId) {
			_vm->_dialog->testDialogOpening();
			_vm->_linesMan->checkZone();
			if (_vm->_linesMan->_route == nullptr
					|| (goHome(), _vm->_linesMan->_route == nullptr)) {
				if (_vm->_globals->_actionMoveTo)
					paradise();
			}
			handleSpecialGames();
			_vm->_events->refreshScreenAndEvents();
			if (!_vm->_globals->_exitId)
				continue;
		}
		breakFlag = true;
	}

	if (_vm->_globals->_exitId != 8 || _vm->_globals->_screenId != 5 || !_helicopterFl) {
		if (!_vm->_graphicsMan->_noFadingFl)
			_vm->_graphicsMan->fadeOutLong();
		_vm->_graphicsMan->_noFadingFl = false;
		removeSprite(0);
		if (_twoCharactersFl) {
			removeSprite(1);
			_twoCharactersFl = false;
		}
		if (!animFile.empty())
			_vm->_graphicsMan->endDisplayBob();
		clearScreen();
	} else {
		_helicopterFl = false;
	}
	_vm->_globals->_eventMode = EVENTMODE_DEFAULT;
}

void ObjectsManager::setVerb(int id) {
	_verb = id;
}

void ObjectsManager::resetHidingUseCount(int idx) {
	_hidingItem[idx]._useCount = 0;
}

void ObjectsManager::setHidingUseCount(int idx) {
	_hidingItem[idx]._useCount = 1;
}

// Load Hiding Items
void ObjectsManager::loadHidingItems(const Common::Path &file) {
	resetHidingItems();
	byte *ptr = _vm->_fileIO->loadFile(file);
	Common::Path filename((const char *)ptr);

	Common::File f;
	if (!f.exists(filename))
		return;

	byte *spriteData = _vm->_fileIO->loadFile(filename);
	_hidingItemData[1] = spriteData;
	int curBufIdx = 60;
	for (int i = 0; i <= 21; i++) {
		HidingItem *hid = &_hidingItem[i];
		hid->_spriteIndex = READ_LE_INT16((uint16 *)ptr + curBufIdx);
		hid->_x = READ_LE_INT16((uint16 *)ptr + curBufIdx + 1);
		hid->_y = READ_LE_INT16((uint16 *)ptr + curBufIdx + 2);
		hid->_yOffset = READ_LE_INT16((uint16 *)ptr + curBufIdx + 4);
		if (spriteData == nullptr) {
			hid->_useCount = 0;
		} else {
			hid->_spriteData = spriteData;
			hid->_width = getWidth(spriteData, hid->_spriteIndex);
			hid->_height = getHeight(spriteData, hid->_spriteIndex);
			hid->_useCount = 1;
		}

		if ( !hid->_x && !hid->_y && !hid->_spriteIndex)
			hid->_useCount = 0;
		curBufIdx += 5;
	}
	enableHidingBehavior();
	_vm->_globals->freeMemory(ptr);
}

void ObjectsManager::initVBob() {
	for (int idx = 0; idx < 30; ++idx) {
		VBobItem *vbob = &_vBob[idx];
		vbob->_displayMode = 0;
		vbob->_xp = 0;
		vbob->_yp = 0;
		vbob->_frameIndex = 0;
		vbob->_surface = nullptr;
		vbob->_spriteData = nullptr;
		vbob->_oldSpriteData = nullptr;
	}
}

void ObjectsManager::clearVBob() {
	for (int idx = 0; idx < 30; ++idx) {
		VBobItem *vbob = &_vBob[idx];
		vbob->_displayMode = 0;
		vbob->_xp = 0;
		vbob->_yp = 0;
		vbob->_frameIndex = 0;
		vbob->_surface = _vm->_globals->freeMemory(vbob->_surface);
		vbob->_spriteData = nullptr;
		vbob->_oldSpriteData = nullptr;
	}
}

void ObjectsManager::disableHidingItem(int idx) {
	assert(idx < 36);
	_bob[idx]._disableFl = true;
}

void ObjectsManager::enableHidingBehavior() {
	_hidingActiveFl = true;
}

void ObjectsManager::disableHidingBehavior() {
	_hidingActiveFl = false;
}

} // End of namespace Hopkins
