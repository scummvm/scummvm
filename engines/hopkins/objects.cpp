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

#include "hopkins/objects.h"

#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"

#include "common/system.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"

namespace Hopkins {

ObjectsManager::ObjectsManager(HopkinsEngine *vm) {
	_vm = vm;

	for (int i = 0; i < 6; ++i)
		Common::fill((byte *)&_sprite[i], (byte *)&_sprite[i] + sizeof(SpriteItem), 0);

	for (int i = 0; i < 36; ++i)
		Common::fill((byte *)&_bob[i], (byte *)&_bob[i] + sizeof(BobItem), 0);

	_helicopterFl = false;
	_priorityFl = false;
	_oldBorderPos = Common::Point(0, 0);
	_oldBorderSpriteIndex = 0;
	_borderPos = Common::Point(0, 0);
	_borderSpriteIndex = 0;
	_saveLoadX = _saveLoadY = 0;
	_oldInventoryPosX = _oldInventoryPosY = 0;
	_oldCharacterPosX = _oldCharacterPosY = 0;
	_eraseVisibleCounter = 0;
	_saveLoadSprite = g_PTRNUL;
	_saveLoadSprite2 = g_PTRNUL;
	_spritePtr = g_PTRNUL;
	_oldSpriteData = g_PTRNUL;
	PERSO_ON = false;
	_saveLoadFl = false;
	_visibleFl = false;
	BOBTOUS = false;
	_zoneNum = 0;
	_forceZoneFl = false;
	_changeVerbFl = false;
	_verb = 0;
	_changeHeadFl = false;
	_disableFl = false;
	_twoCharactersFl = false;
	_characterPos = Common::Point(0, 0);
	_startSpriteIndex = 0;
	OBSSEUL = false;
	_jumpVerb = 0;
	_jumpZone = 0;
	_oldSpriteIndex = 0;
	_oldFlipFl = false;
	_curObjectIndex = 0;
	_forestFl = false;
	_mapCarPosX = _mapCarPosY = 0;
	_forestSprite = NULL;
	_gestureBuf = NULL;
	_curGestureFile = 0;
	_headSprites = NULL;
	_homeRateCounter = 0;
}

ObjectsManager::~ObjectsManager() {
	_vm->_globals->freeMemory(_forestSprite);
	_vm->_globals->freeMemory(_gestureBuf);
	_vm->_globals->freeMemory(_headSprites);
}

void ObjectsManager::clearAll() {
	_forestFl = false;
	_forestSprite = _vm->_globals->freeMemory(_forestSprite);
	_curGestureFile = 0;
	_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
}

/**
 * Change Object
 */
void ObjectsManager::changeObject(int objIndex) {
	_vm->_eventsManager->_objectBuf = loadObjectFromFile(objIndex, true);
	_curObjectIndex = objIndex;
}

byte *ObjectsManager::loadObjectFromFile(int objIndex, bool mode) {
	byte *dataP = NULL;
	int objectFileNum = _vm->_globals->_objectAuthIcons[objIndex]._objectFileNum;
	int idx = _vm->_globals->_objectAuthIcons[objIndex]._idx;

	if (mode)
		++idx;

	if (objectFileNum != _vm->_globals->_curObjectFileNum) {
		if (_vm->_globals->_objectDataBuf != g_PTRNUL)
			ObjectsManager::removeObjectDataBuf();
		if (objectFileNum == 1) {
			_vm->_globals->_objectDataBuf = ObjectsManager::loadSprite("OBJET1.SPR");
		}
		_vm->_globals->_curObjectFileNum = objectFileNum;
	}

	int width = ObjectsManager::getWidth(_vm->_globals->_objectDataBuf, idx);
	int height = ObjectsManager::getHeight(_vm->_globals->_objectDataBuf, idx);
	_vm->_globals->_objectWidth = width;
	_vm->_globals->_objectHeight = height;

	if (mode) {
		sprite_alone(_vm->_globals->_objectDataBuf, _vm->_eventsManager->_objectBuf, idx);
		dataP = _vm->_eventsManager->_objectBuf;
	} else { 
		dataP = _vm->_globals->allocMemory(height * width);
		if (dataP == g_PTRNUL)
			error("CAPTURE_OBJET");

		capture_mem_sprite(_vm->_globals->_objectDataBuf, dataP, idx);
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
	_vm->_globals->_curObjectFileNum = 0;
	_vm->_globals->_objectDataBuf = _vm->_globals->freeMemory(_vm->_globals->_objectDataBuf);
}

/**
 * Load Sprite from file
 */
byte *ObjectsManager::loadSprite(const Common::String &file) {
	return _vm->_fileManager->loadFile(file);
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
	bool loopCondFl;
	uint16 arr[50];

	// Handle copying any background areas that text are going to be drawn on
	_vm->_globals->_sortedDisplayCount = 0;
	for (int idx = 0; idx <= 10; ++idx) {
		if (_vm->_fontManager->_textList[idx]._enabledFl && _vm->_fontManager->_text[idx]._textType != 2) {
			clipX = _vm->_fontManager->_textList[idx]._pos.x - 2;

			if (clipX < _vm->_graphicsManager->_minX)
				clipX = _vm->_graphicsManager->_minX;
	
			clipY = _vm->_fontManager->_textList[idx]._pos.y - 2;
			if (clipY < _vm->_graphicsManager->_minY)
				clipY = _vm->_graphicsManager->_minY;

			_vm->_graphicsManager->copySurface(_vm->_graphicsManager->_vesaScreen, clipX, clipY,
				_vm->_fontManager->_textList[idx]._width + 4, _vm->_fontManager->_textList[idx]._height + 4,
				_vm->_graphicsManager->_vesaBuffer, clipX, clipY);
			_vm->_fontManager->_textList[idx]._enabledFl = false;
		}
	}

	if (!PERSO_ON) {
		for (int idx = 0; idx < MAX_SPRITE; ++idx) {
			if (_vm->_globals->Liste[idx]._visibleFl) {
				clipX = _vm->_globals->Liste[idx]._posX - 2;
				if (clipX < _vm->_graphicsManager->_minX)
					clipX = _vm->_graphicsManager->_minX;

				clipY = _vm->_globals->Liste[idx]._posY - 2;
				if (clipY < _vm->_graphicsManager->_minY)
					clipY = _vm->_graphicsManager->_minY;

				_vm->_graphicsManager->copySurface(_vm->_graphicsManager->_vesaScreen, clipX, clipY,
					_vm->_globals->Liste[idx]._width + 4, _vm->_globals->Liste[idx]._height + 4,
					_vm->_graphicsManager->_vesaBuffer, clipX, clipY);
				_vm->_globals->Liste[idx]._visibleFl = false;
			}
		}
	}

	displayBobAnim();
	displayVBob();

	if (!PERSO_ON) {
		// Handle drawing characters on the screen
		for (int idx = 0; idx < MAX_SPRITE; ++idx) {
			_vm->_globals->Liste[idx]._visibleFl = false;
			if (_sprite[idx]._animationType == 1) {
				computeSprite(idx);
				if (_sprite[idx]._activeFl)
					beforeSort(SORT_SPRITE, idx, _sprite[idx]._height + _sprite[idx]._destY);
			}
		}

		if (_vm->_globals->_hidingActiveFl)
			checkHidingItem();
	}

	if (_priorityFl && _vm->_globals->_sortedDisplayCount) {
		for (int i = 1; i <= 48; i++) 
			arr[i] = i;

		do {
			loopCondFl = false;
			for (int sortIdx = 1; sortIdx < _vm->_globals->_sortedDisplayCount; sortIdx++) {
				if (_vm->_globals->_sortedDisplay[arr[sortIdx]]._priority > _vm->_globals->_sortedDisplay[arr[sortIdx + 1]]._priority) {
					SWAP(arr[sortIdx], arr[sortIdx + 1]);
					loopCondFl = true;
				}
			}
		} while (loopCondFl);

		for (int sortIdx = 1; sortIdx < _vm->_globals->_sortedDisplayCount + 1; sortIdx++) {
			int idx = arr[sortIdx];
			switch (_vm->_globals->_sortedDisplay[idx]._sortMode) {
			case SORT_BOB:
				setBobInfo(_vm->_globals->_sortedDisplay[idx]._index);
				break;
			case SORT_SPRITE:
				DEF_SPRITE(_vm->_globals->_sortedDisplay[idx]._index);
				break;
			case SORT_HIDING:
				displayHiding(_vm->_globals->_sortedDisplay[idx]._index);
				break;
			default:
				break;
			}
			_vm->_globals->_sortedDisplay[idx]._sortMode = SORT_NONE;
		}
	} else {
		for (int idx = 1; idx < _vm->_globals->_sortedDisplayCount + 1; ++idx) {
			switch (_vm->_globals->_sortedDisplay[idx]._sortMode) {
			case SORT_BOB:
				setBobInfo(_vm->_globals->_sortedDisplay[idx]._index);
				break;
			case SORT_SPRITE:
				DEF_SPRITE(_vm->_globals->_sortedDisplay[idx]._index);
				break;
			case SORT_HIDING:
				displayHiding(_vm->_globals->_sortedDisplay[idx]._index);
				break;
			default:
				break;
			}
			_vm->_globals->_sortedDisplay[idx]._sortMode = SORT_NONE;
		}
	}

	// Reset the Sort array
	for (int idx = 0; idx < 50; ++idx) {
		_vm->_globals->_sortedDisplay[idx]._sortMode = SORT_NONE;
		_vm->_globals->_sortedDisplay[idx]._index = 0;
		_vm->_globals->_sortedDisplay[idx]._priority = 0;
	}

	_vm->_globals->_sortedDisplayCount = 0;

	_vm->_dialogsManager->drawInvent(_oldBorderPos, _oldBorderSpriteIndex, _borderPos, _borderSpriteIndex);

	if (_saveLoadFl) {
		_vm->_graphicsManager->restoreSurfaceRect(_vm->_graphicsManager->_vesaBuffer, _saveLoadSprite, _vm->_eventsManager->_startPos.x + 183, 60, 274, 353);
		if (_saveLoadX && _saveLoadY)
			_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _saveLoadSprite2, _saveLoadX + _vm->_eventsManager->_startPos.x + 300, _saveLoadY + 300, 0);

		_vm->_graphicsManager->addDirtyRect(_vm->_eventsManager->_startPos.x + 183, 60, _vm->_eventsManager->_startPos.x + 457, 413);
	}

	// If the Options dialog is activated, draw the elements
	if (_vm->_globals->_optionDialogFl) {
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _vm->_globals->_optionDialogSpr,
			_vm->_eventsManager->_startPos.x + 464, 407, 0);
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _vm->_globals->_optionDialogSpr,
			_vm->_eventsManager->_startPos.x + 657, 556, _vm->_globals->_menuSpeed);
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _vm->_globals->_optionDialogSpr,
			_vm->_eventsManager->_startPos.x + 731, 495, _vm->_globals->_menuTextOff);
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _vm->_globals->_optionDialogSpr,
			_vm->_eventsManager->_startPos.x + 731, 468, _vm->_globals->_menuVoiceOff);
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _vm->_globals->_optionDialogSpr,
			_vm->_eventsManager->_startPos.x + 731, 441, _vm->_globals->_menuSoundOff);
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _vm->_globals->_optionDialogSpr,
			_vm->_eventsManager->_startPos.x + 731, 414, _vm->_globals->_menuMusicOff);
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _vm->_globals->_optionDialogSpr,
			_vm->_eventsManager->_startPos.x + 600, 522, _vm->_globals->_menuDisplayType);
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _vm->_globals->_optionDialogSpr,
			_vm->_eventsManager->_startPos.x + 611, 502, _vm->_globals->_menuScrollSpeed);
		_vm->_graphicsManager->addDirtyRect(_vm->_eventsManager->_startPos.x + 164, 107, _vm->_eventsManager->_startPos.x + 498, 320);
	}

	// Loop to draw any on-screen text
	for (int idx = 0; idx <= 10; ++idx) {
		if (_vm->_fontManager->_text[idx]._textOnFl) {
			if ((_vm->_fontManager->_text[idx]._textType < 2) || (_vm->_fontManager->_text[idx]._textType > 3))
				_vm->_fontManager->box(idx,
					_vm->_fontManager->_text[idx]._messageId, _vm->_fontManager->_text[idx]._filename,
					_vm->_eventsManager->_startPos.x + _vm->_fontManager->_text[idx]._pos.x, _vm->_fontManager->_text[idx]._pos.y);
			else
				_vm->_fontManager->box(idx,
					_vm->_fontManager->_text[idx]._messageId, _vm->_fontManager->_text[idx]._filename,
					_vm->_fontManager->_text[idx]._pos.x, _vm->_fontManager->_text[idx]._pos.y);
			_vm->_fontManager->_textList[idx]._enabledFl = true;

			if ((_vm->_fontManager->_text[idx]._textType < 2) || (_vm->_fontManager->_text[idx]._textType > 3))
				_vm->_fontManager->_textList[idx]._pos.x = _vm->_eventsManager->_startPos.x + _vm->_fontManager->_text[idx]._pos.x;
			else
				_vm->_fontManager->_textList[idx]._pos.x = _vm->_fontManager->_text[idx]._pos.x;

			_vm->_fontManager->_textList[idx]._pos.y = _vm->_fontManager->_text[idx]._pos.y;
			_vm->_fontManager->_textList[idx]._width = _vm->_fontManager->_text[idx]._width;
			_vm->_fontManager->_textList[idx]._height = _vm->_fontManager->_text[idx]._height;

			if (_vm->_fontManager->_textList[idx]._pos.x < _vm->_graphicsManager->_minX)
				_vm->_fontManager->_textList[idx]._pos.x = _vm->_graphicsManager->_minX - 1;
			if (_vm->_fontManager->_textList[idx]._pos.y < _vm->_graphicsManager->_minY)
				_vm->_fontManager->_textList[idx]._pos.y = _vm->_graphicsManager->_minY - 1;

			int posX = _vm->_fontManager->_textList[idx]._pos.x;
			if (_vm->_fontManager->_textList[idx]._width + posX > _vm->_graphicsManager->_maxX)
				_vm->_fontManager->_textList[idx]._width = _vm->_graphicsManager->_maxX - posX;
			int posY = _vm->_fontManager->_textList[idx]._pos.y;
			if (_vm->_fontManager->_textList[idx]._height + posY > _vm->_graphicsManager->_maxY)
				_vm->_fontManager->_textList[idx]._height = _vm->_graphicsManager->_maxY - posY;
			if (_vm->_fontManager->_textList[idx]._width <= 0 || _vm->_fontManager->_textList[idx]._height <= 0)
				_vm->_fontManager->_textList[idx]._enabledFl = false;
		}
	}

	_vm->_dialogsManager->inventAnim();
}

void ObjectsManager::initBob() {
	for (int idx = 0; idx < 35; ++idx)
		resetBob(idx);
}

void ObjectsManager::resetBob(int idx) {
	BobItem &bob = _bob[idx];
	ListeItem &item = _vm->_globals->Liste2[idx];

	bob._bobMode = 0;
	bob._spriteData = g_PTRNUL;
	bob._xp = 0;
	bob._yp = 0;
	bob._frameIndex = 0;
	bob._animDataIdx = 0;
	bob._moveChange1 = 0;
	bob._moveChange2 = 0;
	bob._disabledAnimationFl = false;
	bob._animData = g_PTRNUL;
	bob._bobMode10 = false;
	bob._bobModeChange = 0;
	bob.field20 = 0;
	bob.field22 = 0;
	bob.field34 = false;
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
	if (!_bob[idx]._activeFl)
		return;

	int xp = _bob[idx]._oldX;
	int yp = _bob[idx]._oldY;

	if (_bob[idx]._isSpriteFl)
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _bob[idx]._spriteData,
			xp + 300, yp + 300, _bob[idx]._frameIndex);
	else
		_vm->_graphicsManager->Affiche_Perfect(_vm->_graphicsManager->_vesaBuffer,
			_bob[idx]._spriteData, xp + 300, yp + 300, _bob[idx]._frameIndex,
			_bob[idx]._zoomOutFactor, _bob[idx]._zooInmFactor,
			_bob[idx]._flipFl);

	_vm->_globals->Liste2[idx]._visibleFl = true;
	_vm->_globals->Liste2[idx]._posX = xp;
	_vm->_globals->Liste2[idx]._posY = yp;

	_vm->_globals->Liste2[idx]._width = _bob[idx]._oldWidth;
	_vm->_globals->Liste2[idx]._height = _bob[idx]._oldHeight;

	if (_vm->_globals->Liste2[idx]._posX < _vm->_graphicsManager->_minX) {
		_vm->_globals->Liste2[idx]._width -= _vm->_graphicsManager->_minX - _vm->_globals->Liste2[idx]._posX;
		_vm->_globals->Liste2[idx]._posX = _vm->_graphicsManager->_minX;
	}

	if (_vm->_globals->Liste2[idx]._posY < _vm->_graphicsManager->_minY) {
		_vm->_globals->Liste2[idx]._height -= _vm->_graphicsManager->_minY - _vm->_globals->Liste2[idx]._posY;
		_vm->_globals->Liste2[idx]._posY = _vm->_graphicsManager->_minY;
	}

	if (_vm->_globals->Liste2[idx]._width + _vm->_globals->Liste2[idx]._posX > _vm->_graphicsManager->_maxX)
		_vm->_globals->Liste2[idx]._width = _vm->_graphicsManager->_maxX - _vm->_globals->Liste2[idx]._posX;

	if (_vm->_globals->Liste2[idx]._height + _vm->_globals->Liste2[idx]._posY > _vm->_graphicsManager->_maxY)
		_vm->_globals->Liste2[idx]._height = _vm->_graphicsManager->_maxY - _vm->_globals->Liste2[idx]._posY;

	if (_vm->_globals->Liste2[idx]._width <= 0 || _vm->_globals->Liste2[idx]._height <= 0)
		_vm->_globals->Liste2[idx]._visibleFl = false;

	if (_vm->_globals->Liste2[idx]._visibleFl)
		_vm->_graphicsManager->addDirtyRect(
             _vm->_globals->Liste2[idx]._posX,
             _vm->_globals->Liste2[idx]._posY,
             _vm->_globals->Liste2[idx]._posX + _vm->_globals->Liste2[idx]._width,
             _vm->_globals->Liste2[idx]._posY + _vm->_globals->Liste2[idx]._height);
}

void ObjectsManager::displayBob(int idx) {
	_priorityFl = true;

	if (_bob[idx]._bobMode)
		return;

	resetBob(idx);

	const byte *data = _vm->_globals->_animBqe[idx]._data;
	int bankIdx = READ_LE_INT16(data);
	if (!bankIdx)
		return;
	if ((!_vm->_globals->Bank[bankIdx]._loadedFl) || (!READ_LE_UINT16(data + 24)))
		return;


	int16 bobModeChange = READ_LE_INT16(data + 2);
	int16 newField22 = READ_LE_INT16(data + 4);
	// data[6] isn't used, read skipped
	int16 newField20 = READ_LE_INT16(data + 8);

	if (!bobModeChange)
		bobModeChange = 1;
	if (!newField20)
		newField20 = -1;

	_bob[idx]._isSpriteFl = false;

	if (_vm->_globals->Bank[bankIdx]._fileHeader == 1) {
		_bob[idx]._isSpriteFl = true;
		_bob[idx]._zoomFactor = 0;
		_bob[idx]._flipFl = false;
	}

	_bob[idx]._animData = _vm->_globals->_animBqe[idx]._data;
	_bob[idx]._bobMode = 10;
	_bob[idx]._spriteData = _vm->_globals->Bank[bankIdx]._data;

	_bob[idx]._bobModeChange = bobModeChange;
	_bob[idx].field20 = newField20;
	_bob[idx].field22 = newField22;
}

void ObjectsManager::hideBob(int idx) {
	if ((_bob[idx]._bobMode == 3) || (_bob[idx]._bobMode == 10))
		_bob[idx]._bobMode++;
}

void ObjectsManager::setBobOffset(int idx, int offset) {
	_bob[idx]._oldX2 = offset;
}

void ObjectsManager::SCBOB(int idx) {
	HidingItem *hid = &_vm->_globals->_hidingItem[idx];
	if (hid->_useCount == 0)
		return;

	for (int i = 0; i <= 20; i++) {
		if ((_bob[i]._bobMode) && (!_bob[i]._disabledAnimationFl) && (!_bob[i].field34) && (_bob[i]._frameIndex != 250)) {
			int oldRight = _bob[i]._oldX + _bob[i]._oldWidth;
			int oldBottom = _bob[i]._oldY + _bob[i]._oldHeight;
			int cachedRight = hid->_width + hid->_x;

			if ((oldBottom > hid->_y) && (oldBottom < hid->_yOffset +hid->_height + hid->_y)) {
				if ((oldRight >= hid->_x && oldRight <= cachedRight)
				 || (cachedRight >= _bob[i]._oldWidth && _bob[i]._oldWidth >= hid->_x)
				 || (cachedRight >= _bob[i]._oldWidth && _bob[i]._oldWidth >= hid->_x)
				 || (_bob[i]._oldWidth >= hid->_x && oldRight <= cachedRight)
				 || (_bob[i]._oldWidth <= hid->_x && oldRight >= cachedRight))
					++hid->_useCount;
			}
		}
	}
}

void ObjectsManager::CALCUL_BOB(int idx) {
	_bob[idx]._activeFl = false;
	if (_bob[idx]._isSpriteFl) {
		_bob[idx]._flipFl = false;
		_bob[idx]._zoomFactor = 0;
	}

	int spriteIdx = _bob[idx]._frameIndex;
	if (spriteIdx == 250)
		return;

	int deltaY, deltaX;
	if (_bob[idx]._flipFl) {
		deltaX = getOffsetX(_bob[idx]._spriteData, spriteIdx, true);
		deltaY = getOffsetY(_bob[idx]._spriteData, _bob[idx]._frameIndex, true);
	} else {
		deltaX = getOffsetX(_bob[idx]._spriteData, spriteIdx, false);
		deltaY = getOffsetY(_bob[idx]._spriteData, _bob[idx]._frameIndex, false);
	}

	int negZoom = 0;
	int posZoom = 0;
	if (_bob[idx]._zoomFactor < 0) {
		negZoom = -_bob[idx]._zoomFactor;
		if (negZoom > 95)
			negZoom = 95;
	} else
		posZoom = _bob[idx]._zoomFactor;

	if (posZoom) {
		if (deltaX >= 0)
			deltaX = _vm->_graphicsManager->zoomIn(deltaX, posZoom);
		else
			deltaX = -_vm->_graphicsManager->zoomIn(-deltaX, posZoom);

		if (deltaY >= 0)
			deltaY = _vm->_graphicsManager->zoomIn(deltaY, posZoom);
		else
			deltaY = -_vm->_graphicsManager->zoomIn(abs(deltaX), posZoom);
	}

	if (negZoom) {
		if (deltaX >= 0)
			deltaX = _vm->_graphicsManager->zoomOut(deltaX, negZoom);
		else
			deltaX = -_vm->_graphicsManager->zoomOut(-deltaX, negZoom);

		if (deltaY >= 0)
			deltaY = _vm->_graphicsManager->zoomOut(deltaY, negZoom);
		else
			deltaY = -_vm->_graphicsManager->zoomOut(abs(deltaX), negZoom);
	}

	int newX = _bob[idx]._xp - deltaX;
	int newY = _bob[idx]._yp - deltaY;
	_bob[idx]._activeFl = true;
	_bob[idx]._oldX = newX;
	_bob[idx]._oldY = newY;
	_bob[idx]._zooInmFactor = posZoom;
	_bob[idx]._zoomOutFactor = negZoom;

	_vm->_globals->Liste2[idx]._visibleFl = true;
	_vm->_globals->Liste2[idx]._posX = newX;
	_vm->_globals->Liste2[idx]._posY = newY;

	int width = getWidth(_bob[idx]._spriteData, _bob[idx]._frameIndex);
	int height = getHeight(_bob[idx]._spriteData, _bob[idx]._frameIndex);

	if (posZoom) {
		width = _vm->_graphicsManager->zoomIn(width, posZoom);
		height = _vm->_graphicsManager->zoomIn(height, posZoom);
	}
	if (negZoom) {
		height = _vm->_graphicsManager->zoomOut(height, negZoom);
		width = _vm->_graphicsManager->zoomOut(width, negZoom);
	}

	_vm->_globals->Liste2[idx]._width = width;
	_vm->_globals->Liste2[idx]._height = height;
	_bob[idx]._oldWidth = width;
	_bob[idx]._oldHeight = height;
}

void ObjectsManager::checkHidingItem() {
	for (int hidingItemIdx = 0; hidingItemIdx <= 19; hidingItemIdx++) {
		HidingItem *hid = &_vm->_globals->_hidingItem[hidingItemIdx];
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
					 || (hidingRight >= spr->_destX && hid->_x <= spr->_destX)
					 || (hidingRight >= spr->_destX && hid->_x <= spr->_destX)
					 || (hid->_x <= spr->_destX && right <= hidingRight)
					 || (hid->_x >= spr->_destX && right >= hidingRight))
						++hid->_useCount;
				}
			}
		}

		SCBOB(hidingItemIdx);
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

void ObjectsManager::DEF_SPRITE(int idx) {
	SpriteItem *spr = &_sprite[idx];
	if (!spr->_activeFl)
		return;

	if (spr->_rleFl)
		_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, spr->_spriteData,
		    spr->_destX + 300, spr->_destY + 300, spr->_spriteIndex);
	else
		_vm->_graphicsManager->Affiche_Perfect(_vm->_graphicsManager->_vesaBuffer, spr->_spriteData,
		    spr->_destX + 300, spr->_destY + 300,  spr->_spriteIndex, spr->_reducePct, spr->_zoomPct, spr->_flipFl);

	ListeItem *list = &_vm->_globals->Liste[idx];
	list->_width = spr->_width;
	list->_height = spr->_height;

	if (list->_posX < _vm->_graphicsManager->_minX) {
		list->_width -= _vm->_graphicsManager->_minX - list->_posX;
		list->_posX = _vm->_graphicsManager->_minX;
	}

	if (list->_posY < _vm->_graphicsManager->_minY) {
		list->_height -= _vm->_graphicsManager->_minY - list->_posY;
		list->_posY = _vm->_graphicsManager->_minY;
	}

	if (list->_width + list->_posX > _vm->_graphicsManager->_maxX)
		list->_width = _vm->_graphicsManager->_maxX - list->_posX;

	if (list->_height + list->_posY > _vm->_graphicsManager->_maxY)
		list->_height = _vm->_graphicsManager->_maxY - list->_posY;

	if (list->_width <= 0 || list->_height <= 0)
		list->_visibleFl = false;

	if (list->_visibleFl)
		_vm->_graphicsManager->addDirtyRect( list->_posX, list->_posY, list->_posX + list->_width, list->_posY + list->_height);
}

void ObjectsManager::displayHiding(int idx) {
	HidingItem *hid = &_vm->_globals->_hidingItem[idx];

	_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, _vm->_globals->_hidingItemData[1], 
		hid->_x + 300, hid->_y + 300, hid->_spriteIndex);
	_vm->_graphicsManager->addDirtyRect(hid->_x, hid->_y, hid->_x + hid->_width, hid->_y + hid->_height);
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
			deltaX = _vm->_graphicsManager->zoomIn(tmpX, zoomPercent);
		else
			deltaX = -_vm->_graphicsManager->zoomIn(-tmpX, zoomPercent);

		if (tmpY >= 0) {
			deltaY = _vm->_graphicsManager->zoomIn(tmpY, zoomPercent);
		} else {
			tmpY = abs(tmpX);
			deltaY = -_vm->_graphicsManager->zoomIn(tmpY, zoomPercent);
		}
	} else if (reducePercent) {
		if (tmpX >= 0)
			deltaX = _vm->_graphicsManager->zoomOut(tmpX, reducePercent);
		else
			deltaX = -_vm->_graphicsManager->zoomOut(-tmpX, reducePercent);

		if (tmpY >= 0) {
			deltaY = _vm->_graphicsManager->zoomOut(tmpY, reducePercent);
		} else {
			tmpY = abs(tmpX);
			deltaY = -_vm->_graphicsManager->zoomOut(tmpY, reducePercent);
		}
	}

	int newPosX = spr->_spritePos.x - deltaX;
	int newPosY = spr->_spritePos.y - deltaY;
	spr->_destX = newPosX;
	spr->_destY = newPosY;
	spr->_activeFl = true;
	spr->_zoomPct = zoomPercent;
	spr->_reducePct = reducePercent;

	_vm->_globals->Liste[idx]._visibleFl = true;
	_vm->_globals->Liste[idx]._posX = newPosX;
	_vm->_globals->Liste[idx]._posY = newPosY;

	int width = getWidth(spr->_spriteData, spr->_spriteIndex);
	int height = getHeight(spr->_spriteData, spr->_spriteIndex);

	if (zoomPercent) {
		width = _vm->_graphicsManager->zoomIn(width, zoomPercent);
		height = _vm->_graphicsManager->zoomIn(height, zoomPercent);
	} else if (reducePercent) {
		height = _vm->_graphicsManager->zoomOut(height, reducePercent);
		width = _vm->_graphicsManager->zoomOut(width, reducePercent);
	}

	spr->_width = width;
	spr->_height = height;
}

// Before Sort
void ObjectsManager::beforeSort(SortMode sortMode, int index, int priority) {
	++_vm->_globals->_sortedDisplayCount;
	assert (_vm->_globals->_sortedDisplayCount <= 48);

	_vm->_globals->_sortedDisplay[_vm->_globals->_sortedDisplayCount]._sortMode = sortMode;
	_vm->_globals->_sortedDisplay[_vm->_globals->_sortedDisplayCount]._index = index;
	_vm->_globals->_sortedDisplay[_vm->_globals->_sortedDisplayCount]._priority = priority;
}

// Display BOB Anim
void ObjectsManager::displayBobAnim() {
	for (int idx = 1; idx <= 35; idx++) {
		if (idx <= 20 && PERSO_ON) {
			_bob[idx]._bobMode10 = false;
			continue;
		}

		if (_bob[idx]._bobMode != 10) 
			continue;

		_bob[idx]._bobMode10 = false;
		if (_bob[idx]._animData == g_PTRNUL || _bob[idx]._disabledAnimationFl || _bob[idx].field20 == 0 || _bob[idx].field20 < -1) {
			if (_bob[idx]._bobModeChange == 1 || _bob[idx]._bobModeChange == 2)
				_bob[idx]._bobMode10 = true;
			continue;
		}

		if (_bob[idx]._moveChange1 == _bob[idx]._moveChange2) {
			_bob[idx]._bobMode10 = true;
		} else {
			_bob[idx]._moveChange2++;
			_bob[idx]._bobMode10 = false;
		}

		if (!_bob[idx]._bobMode10) {
			if (_bob[idx]._bobModeChange == 1 || _bob[idx]._bobModeChange == 2)
				_bob[idx]._bobMode10 = true;
			continue;
		}

		byte *dataPtr = _bob[idx]._animData + 20;
		int dataIdx = _bob[idx]._animDataIdx;
		_bob[idx]._xp = READ_LE_INT16(dataPtr + 2 * dataIdx);
		if (_vm->_globals->_lockedAnims[idx]._enableFl)
			_bob[idx]._xp = _vm->_globals->_lockedAnims[idx]._posX;
		if ( PERSO_ON && idx > 20 )
			_bob[idx]._xp += _vm->_eventsManager->_startPos.x;

		_bob[idx]._yp = READ_LE_INT16(dataPtr + 2 * dataIdx + 2);
		_bob[idx]._moveChange1 = READ_LE_INT16(dataPtr + 2 * dataIdx + 4);
		_bob[idx]._zoomFactor = READ_LE_INT16(dataPtr + 2 * dataIdx + 6);
		_bob[idx]._frameIndex = dataPtr[2 * dataIdx + 8];
		_bob[idx]._flipFl = (dataPtr[2 * dataIdx + 9] != 0);
		_bob[idx]._animDataIdx += 5;

		if (_bob[idx]._moveChange1 > 0) {
			_bob[idx]._moveChange1 /= _vm->_globals->_speed;
			if (_bob[idx]._moveChange1 > 0) {
				_bob[idx]._moveChange2 = 1;
				if (_bob[idx]._bobModeChange == 1 || _bob[idx]._bobModeChange == 2)
					_bob[idx]._bobMode10 = true;
				continue;
			}

			_bob[idx]._moveChange1 = 1;
		}
		if (!_bob[idx]._moveChange1) {
			if (_bob[idx].field20 > 0)
				_bob[idx].field20--;
			if (_bob[idx].field20 != -1 && _bob[idx].field20 <= 0) {
				_bob[idx]._bobMode = 11;
			} else {
				_bob[idx]._animDataIdx = 0;
				byte *bobData = _bob[idx]._animData + 20;
				_bob[idx]._xp = READ_LE_INT16(bobData);

				if (_vm->_globals->_lockedAnims[idx]._enableFl)
					_bob[idx]._xp = _vm->_globals->_lockedAnims[idx]._posX;
				if (PERSO_ON && idx > 20)
					_bob[idx]._xp += _vm->_eventsManager->_startPos.x;

				_bob[idx]._yp = READ_LE_INT16(bobData + 2);
				_bob[idx]._moveChange1 = READ_LE_INT16(bobData + 4);
				_bob[idx]._zoomFactor = READ_LE_INT16(bobData + 6);
				_bob[idx]._frameIndex = bobData[8];
				_bob[idx]._flipFl = (bobData[9] != 0);
				_bob[idx]._animDataIdx += 5;

				if (_bob[idx]._moveChange1 > 0) {
					_bob[idx]._moveChange1 /= _vm->_globals->_speed;
					// Original code. It can't be negative, so the check is on == 0
					if (_bob[idx]._moveChange1 <= 0)
						_bob[idx]._moveChange1 = 1;
				}
			}
		}

		_bob[idx]._moveChange2 = 1;
		if (_bob[idx]._bobModeChange == 1 || _bob[idx]._bobModeChange == 2)
			_bob[idx]._bobMode10 = true;
	}

	if (!PERSO_ON && BOBTOUS) {
		for (int i = 0; i < 35; i++) {
			if (_bob[i]._bobMode == 10 && !_bob[i]._disabledAnimationFl)
				_bob[i]._bobMode10 = true;
		}
	}

	BOBTOUS = false;

	for (int i = 1; i <= 35; i++) {
		if (i > 20 || !PERSO_ON) {
			if ((_bob[i]._bobMode == 10) && (_bob[i]._bobMode10)) {
				if ((_bob[i]._bobModeChange != 2) && (_bob[i]._bobModeChange != 4)) {
					if (_vm->_globals->Liste2[i]._visibleFl) {
						_vm->_graphicsManager->copySurface(_vm->_graphicsManager->_vesaScreen,
							_vm->_globals->Liste2[i]._posX, _vm->_globals->Liste2[i]._posY,
							_vm->_globals->Liste2[i]._width, _vm->_globals->Liste2[i]._height,
							_vm->_graphicsManager->_vesaBuffer, _vm->_globals->Liste2[i]._posX,
							_vm->_globals->Liste2[i]._posY);
						_vm->_globals->Liste2[i]._visibleFl = false;
					}
				}
			}

			if (_bob[i]._bobMode == 11) {
				if (_vm->_globals->Liste2[i]._visibleFl) {
					_vm->_graphicsManager->copySurface(_vm->_graphicsManager->_vesaScreen,
						_vm->_globals->Liste2[i]._posX, _vm->_globals->Liste2[i]._posY,
						_vm->_globals->Liste2[i]._width, _vm->_globals->Liste2[i]._height,
						_vm->_graphicsManager->_vesaBuffer,
						_vm->_globals->Liste2[i]._posX, _vm->_globals->Liste2[i]._posY);
					_vm->_globals->Liste2[i]._visibleFl = false;
				}

				_bob[i]._bobMode = 0;
			}
		}
	}

	for (int i = 1; i <= 35; i++) {
		_bob[i]._oldY = 0;
		if (_bob[i]._bobMode == 10 && !_bob[i]._disabledAnimationFl && _bob[i]._bobMode10) {
			CALCUL_BOB(i);
			int priority = _bob[i]._oldX2 + _bob[i]._oldHeight + _bob[i]._oldY;

			if (priority > 450)
				priority = 600;

			if (_bob[i]._activeFl)
				beforeSort(SORT_BOB, i, priority);
		}
	}
}

// Display VBOB
void ObjectsManager::displayVBob() {
	int width, height;

	for (int idx = 0; idx <= 29; idx++) {
		VBobItem *vbob = &_vm->_globals->VBob[idx];
		if (vbob->_displayMode == 4) {
			width = getWidth(vbob->_spriteData, vbob->_frameIndex);
			height = getHeight(vbob->_spriteData, vbob->_frameIndex);

			_vm->_graphicsManager->restoreSurfaceRect(_vm->_graphicsManager->_vesaScreen, vbob->_surface,
				vbob->_xp, vbob->_yp, width, height);

			_vm->_graphicsManager->restoreSurfaceRect(_vm->_graphicsManager->_vesaBuffer, vbob->_surface,
				vbob->_xp, vbob->_yp, width, height);

			_vm->_graphicsManager->addDirtyRect(vbob->_xp, vbob->_yp, vbob->_xp + width, height + vbob->_yp);
			vbob->_surface = _vm->_globals->freeMemory(vbob->_surface);

			vbob->_displayMode = 0;
			vbob->_spriteData = g_PTRNUL;
			vbob->_xp = 0;
			vbob->_yp = 0;
			vbob->_oldX = 0;
			vbob->_oldY = 0;
			vbob->_frameIndex = 0;
			vbob->_oldFrameIndex = 0;
			vbob->_oldSpriteData = g_PTRNUL;
		}

		if (vbob->_displayMode == 3) {
			width = getWidth(vbob->_oldSpriteData, vbob->_oldFrameIndex);
			height = getHeight(vbob->_oldSpriteData, vbob->_oldFrameIndex);

			_vm->_graphicsManager->restoreSurfaceRect(_vm->_graphicsManager->_vesaScreen, vbob->_surface,
				vbob->_oldX, vbob->_oldY, width, height);

			_vm->_graphicsManager->restoreSurfaceRect(_vm->_graphicsManager->_vesaBuffer, vbob->_surface,
				vbob->_oldX, vbob->_oldY, width, height);

			_vm->_graphicsManager->addDirtyRect(vbob->_oldX, vbob->_oldY, vbob->_oldX + width, vbob->_oldY + height);

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

			_vm->_graphicsManager->copySurfaceRect(_vm->_graphicsManager->_vesaScreen, surface,
				vbob->_xp, vbob->_yp, width, height);

			if (*vbob->_spriteData == 78) {
				_vm->_graphicsManager->Affiche_Perfect(_vm->_graphicsManager->_vesaScreen, vbob->_spriteData,
					vbob->_xp + 300, vbob->_yp + 300, vbob->_frameIndex, 0, 0, false);

				_vm->_graphicsManager->Affiche_Perfect(_vm->_graphicsManager->_vesaBuffer, vbob->_spriteData,
					vbob->_xp + 300, vbob->_yp + 300, vbob->_frameIndex, 0, 0, false);
			} else {
				_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaBuffer, vbob->_spriteData, 
					vbob->_xp + 300, vbob->_yp + 300, vbob->_frameIndex);

				_vm->_graphicsManager->Sprite_Vesa(_vm->_graphicsManager->_vesaScreen, vbob->_spriteData,
					vbob->_xp + 300, vbob->_yp + 300, vbob->_frameIndex);
			}

			_vm->_graphicsManager->addDirtyRect(vbob->_xp, vbob->_yp , vbob->_xp + width, vbob->_yp + height);
			vbob->_displayMode = 2;
		}
	}
}

/**
 * Get Sprite X coordinate
 */
int ObjectsManager::getSpriteX(int idx) {
	assert (idx  <= MAX_SPRITE);
	return _sprite[idx]._spritePos.x;
}

/**
 * Get Sprite Y coordinate
 */
int ObjectsManager::getSpriteY(int idx) {
	assert (idx  <= MAX_SPRITE);
	return _sprite[idx]._spritePos.y;
}

/**
 * Clear sprite structure
 */
void ObjectsManager::clearSprite() {
	for (int idx = 0; idx < MAX_SPRITE; idx++) {
		_sprite[idx]._spriteData = g_PTRNUL;
		_sprite[idx]._animationType = 0;
	}

	for (int idx = 0; idx < MAX_SPRITE; idx++) {
		ListeItem *list = &_vm->_globals->Liste[idx];
		list->_visibleFl = false;
		list->_posX = 0;
		list->_posY = 0;
		list->_width = 0;
		list->_height = 0;
	}
}

void ObjectsManager::animateSprite(int idx) {
	assert (idx  <= MAX_SPRITE);
	_sprite[idx]._animationType = 1;
}

void ObjectsManager::addStaticSprite(const byte *spriteData, Common::Point pos, int idx, int spriteIndex, int zoomFactor, bool flipFl, int deltaX, int deltaY) {
	assert (idx  <= MAX_SPRITE);

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
	assert (idx  <= MAX_SPRITE);
	_sprite[idx]._spritePos.x = xp;
}

/**
 * Set Sprite Y coordinate
 */
void ObjectsManager::setSpriteY(int idx, int yp) {
	assert (idx  <= MAX_SPRITE);
	_sprite[idx]._spritePos.y = yp;
}

/**
 * Set Sprite Index
 */
void ObjectsManager::setSpriteIndex(int idx, int spriteIndex) {
	assert (idx  <= MAX_SPRITE);
	_sprite[idx]._spriteIndex = spriteIndex;
}

// Set Sprite Size
void ObjectsManager::setSpriteZoom(int idx, int zoomFactor) {
	assert (idx  <= MAX_SPRITE);
	if (!_sprite[idx]._rleFl)
		_sprite[idx]._zoomFactor = zoomFactor;
}

void ObjectsManager::setFlipSprite(int idx, bool flipFl) {
	assert (idx  <= MAX_SPRITE);
	if (!_sprite[idx]._rleFl)
		_sprite[idx]._flipFl = flipFl;
}

void ObjectsManager::GOHOME() {
	if (_vm->_linesManager->_route == (RouteItem *)g_PTRNUL)
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
	if (_vm->_globals->_oldDirection == DIR_NONE) {
		computeAndSetSpriteSize();
		newPosX = _vm->_linesManager->_route->_x;
		newPosY = _vm->_linesManager->_route->_y;
		newDirection = _vm->_linesManager->_route->_dir;
		_vm->_linesManager->_route++;

		if (newPosX != -1 || newPosY != -1) {
			_vm->_globals->_oldDirection = newDirection;
			_vm->_globals->_oldDirectionSpriteIdx = newDirection + 59;
			_vm->_globals->_oldFrameIndex = 0;
			_oldCharacterPosX = newPosX;
			_oldCharacterPosY = newPosY;
		} else {
			setSpriteIndex(0, _vm->_globals->_oldDirection + 59);
			_vm->_globals->_actionDirection = DIR_NONE;
			int zoneId;
			if (_vm->_globals->_actionMoveTo)
				zoneId = _vm->_globals->_saveData->_data[svLastZoneNum];
			else
				zoneId = _zoneNum;
			_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
			computeAndSetSpriteSize();
			setFlipSprite(0, false);
			_homeRateCounter = 0;
			_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
			_vm->_globals->_oldDirection = DIR_NONE;
			if (zoneId > 0) {
				if (_vm->_linesManager->ZONEP[zoneId]._destX && _vm->_linesManager->ZONEP[zoneId]._destY && _vm->_linesManager->ZONEP[zoneId]._destY != 31) {
					if (_vm->_linesManager->ZONEP[zoneId]._spriteIndex == -1) {
						_vm->_linesManager->ZONEP[zoneId]._destX = 0;
						_vm->_linesManager->ZONEP[zoneId]._destY = 0;
						_vm->_linesManager->ZONEP[zoneId]._spriteIndex = 0;
					} else {
						setSpriteIndex(0, _vm->_linesManager->ZONEP[zoneId]._spriteIndex);
						_vm->_globals->_actionDirection = _vm->_linesManager->ZONEP[zoneId]._spriteIndex - 59;
					}
				}
			}
		}
		_homeRateCounter = 0;
		return;
	}
	if (_vm->_globals->_oldDirection == DIR_RIGHT) {
		if (_vm->_globals->_oldFrameIndex < 24 || _vm->_globals->_oldFrameIndex > 35) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 24;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedY;

			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX + deltaX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _vm->_globals->_oldFrameIndex + 1;
			if (oldFrameIdx > 35)
				oldFrameIdx = 24;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_vm->_globals->_oldDirection == DIR_LEFT) {
		if (_vm->_globals->_oldFrameIndex < 24 || _vm->_globals->_oldFrameIndex > 35) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 24;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX - deltaX;
			oldPosY = _oldCharacterPosY - deltaY;
			oldFrameIdx = _vm->_globals->_oldFrameIndex + 1;
			if (oldFrameIdx > 35)
				oldFrameIdx = 24;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_vm->_globals->_oldDirection == DIR_UP) {
		if (_vm->_globals->_oldFrameIndex > 11) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 0;
		} else {
			int deltaY = abs(_vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedY);
			if (_sprite[0]._zoomFactor < 0) {
				deltaY = _vm->_graphicsManager->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaY = _vm->_graphicsManager->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY - deltaY;
			oldFrameIdx = _vm->_globals->_oldFrameIndex + 1;
			if (oldFrameIdx > 11)
				oldFrameIdx = 0;
		}
		_homeRateCounter = 4 / _vm->_globals->_speed;
	}

	if (_vm->_globals->_oldDirection == DIR_DOWN) {
		if (_vm->_globals->_oldFrameIndex < 48 || _vm->_globals->_oldFrameIndex > 59) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 48;
		} else {
			int deltaY = abs(_vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedY);
			if (_sprite[0]._zoomFactor < 0) {
				deltaY = _vm->_graphicsManager->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaY = _vm->_graphicsManager->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX;
			oldPosY = deltaY + _oldCharacterPosY;
			oldFrameIdx = _vm->_globals->_oldFrameIndex + 1;
			if (oldFrameIdx > 59)
				oldFrameIdx = 48;
		}
		_homeRateCounter = 4 / _vm->_globals->_speed;
	}
	if (_vm->_globals->_oldDirection == DIR_UP_RIGHT) {
		if (_vm->_globals->_oldFrameIndex < 12 || _vm->_globals->_oldFrameIndex > 23) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 12;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			}
			if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = deltaX + _oldCharacterPosX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _vm->_globals->_oldFrameIndex + 1;
			if (oldFrameIdx > 23)
				oldFrameIdx = 12;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_vm->_globals->_oldDirection == DIR_UP_LEFT) {
		if (_vm->_globals->_oldFrameIndex < 12 || _vm->_globals->_oldFrameIndex > 23) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 12;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX - deltaX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _vm->_globals->_oldFrameIndex + 1;
			if (oldFrameIdx > 23)
				oldFrameIdx = 12;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_vm->_globals->_oldDirection == DIR_DOWN_RIGHT) {
		if (_vm->_globals->_oldFrameIndex < 36 || _vm->_globals->_oldFrameIndex > 47) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 36;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			}
			if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = deltaX + _oldCharacterPosX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _vm->_globals->_oldFrameIndex + 1;
			if (oldFrameIdx > 47)
				oldFrameIdx = 36;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	if (_vm->_globals->_oldDirection == DIR_DOWN_LEFT) {
		if (_vm->_globals->_oldFrameIndex < 36 || _vm->_globals->_oldFrameIndex > 47) {
			oldPosX = _oldCharacterPosX;
			oldPosY = _oldCharacterPosY;
			oldFrameIdx = 36;
		} else {
			int deltaX = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedX;
			int deltaY = _vm->_globals->_hopkinsItem[_vm->_globals->_oldFrameIndex]._speedY;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager->zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomOut(deltaY, -_sprite[0]._zoomFactor);
			}
			if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager->zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager->zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			oldPosX = _oldCharacterPosX - deltaX;
			oldPosY = _oldCharacterPosY + deltaY;
			oldFrameIdx = _vm->_globals->_oldFrameIndex + 1;
			if (oldFrameIdx > 47)
				oldFrameIdx = 36;
		}
		_homeRateCounter = 5 / _vm->_globals->_speed;
	}
	bool loopCond = false;
	do {
		newPosX = _vm->_linesManager->_route->_x;
		newPosY = _vm->_linesManager->_route->_y;
		newDirection = (Directions)_vm->_linesManager->_route->_dir;
		_vm->_linesManager->_route++;

		if (newPosX == -1 && newPosY == -1) {
			int zoneId;
			if (_vm->_globals->_actionMoveTo)
				zoneId = _vm->_globals->_saveData->_data[svLastZoneNum];
			else
				zoneId = _zoneNum;
			setSpriteIndex(0, _vm->_globals->_oldDirection + 59);
			_vm->_globals->_actionDirection = DIR_NONE;
			_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
			computeAndSetSpriteSize();
			setFlipSprite(0, false);
			_homeRateCounter = 0;
			_vm->_globals->_oldDirection = DIR_NONE;
			_oldCharacterPosX = getSpriteX(0);
			_oldCharacterPosY = getSpriteY(0);

			if (zoneId > 0) {
				if (_vm->_linesManager->ZONEP[zoneId]._destX && _vm->_linesManager->ZONEP[zoneId]._destY && _vm->_linesManager->ZONEP[zoneId]._destY != 31) {
					if ( _vm->_linesManager->ZONEP[zoneId]._spriteIndex == -1) {
						_vm->_linesManager->ZONEP[zoneId]._destX = 0;
						_vm->_linesManager->ZONEP[zoneId]._destY = 0;
						_vm->_linesManager->ZONEP[zoneId]._spriteIndex = 0;
					} else {
						setSpriteIndex(0,  _vm->_linesManager->ZONEP[zoneId]._spriteIndex);
						_vm->_globals->_actionDirection = _vm->_linesManager->ZONEP[zoneId]._spriteIndex - 59;
					}
				}
			}
			_homeRateCounter = 0;
			return;
		}
		if (_vm->_globals->_oldDirection != newDirection)
			break;
		if ((newDirection == DIR_RIGHT && newPosX >= oldPosX) || (_vm->_globals->_oldDirection == DIR_LEFT && newPosX <= oldPosX) ||
		    (_vm->_globals->_oldDirection == DIR_UP && newPosY <= oldPosY) || (_vm->_globals->_oldDirection == DIR_DOWN && newPosY >= oldPosY) ||
		    (_vm->_globals->_oldDirection == DIR_UP_RIGHT && newPosX >= oldPosX)  || (_vm->_globals->_oldDirection == DIR_UP_LEFT && newPosX <= oldPosX) ||
		    (_vm->_globals->_oldDirection == DIR_DOWN_RIGHT && newPosX >= oldPosX) || (_vm->_globals->_oldDirection == DIR_DOWN_LEFT && newPosX <= oldPosX))
			loopCond = true;
	} while (!loopCond);
	if (loopCond) {
		computeAndSetSpriteSize();
		if ((_vm->_globals->_oldDirection == DIR_DOWN_LEFT) || (_vm->_globals->_oldDirection == DIR_LEFT) || (_vm->_globals->_oldDirection == DIR_UP_LEFT))
			setFlipSprite(0, true);

		if ((_vm->_globals->_oldDirection == DIR_UP) || (_vm->_globals->_oldDirection == DIR_UP_RIGHT) || (_vm->_globals->_oldDirection == DIR_RIGHT) ||
		    (_vm->_globals->_oldDirection == DIR_DOWN_RIGHT) || (_vm->_globals->_oldDirection == DIR_DOWN))
			setFlipSprite(0, false);

		setSpriteX(0, newPosX);
		setSpriteY(0, newPosY);
		setSpriteIndex(0, oldFrameIdx);
	} else {
		if ((_vm->_globals->_oldDirection == DIR_DOWN_LEFT) || (_vm->_globals->_oldDirection == DIR_LEFT) || (_vm->_globals->_oldDirection == DIR_UP_LEFT))
			setFlipSprite(0, true);

		if ((_vm->_globals->_oldDirection == DIR_UP) || (_vm->_globals->_oldDirection == DIR_UP_RIGHT) || (_vm->_globals->_oldDirection == DIR_RIGHT) ||
		    (_vm->_globals->_oldDirection == DIR_DOWN_RIGHT) || (_vm->_globals->_oldDirection == DIR_DOWN))
			setFlipSprite(0, false);
		_homeRateCounter = 0;
	}
	_vm->_globals->_oldDirection = newDirection;
	_vm->_globals->_oldDirectionSpriteIdx = newDirection + 59;
	_vm->_globals->_oldFrameIndex = oldFrameIdx;
	_oldCharacterPosX = newPosX;
	_oldCharacterPosY = newPosY;
}

void ObjectsManager::GOHOME2() {
	if (_vm->_linesManager->_route == (RouteItem *)g_PTRNUL)
		return;

	int realSpeed = 2;
	if (_vm->_globals->_speed == 2)
		realSpeed = 4;
	else if (_vm->_globals->_speed == 3)
		realSpeed = 6;

	int countColisionPixel = 0;

	for (;;) {
		int nexPosX = _vm->_linesManager->_route->_x;
		int newPosY = _vm->_linesManager->_route->_y;
		Directions newDirection = (Directions)_vm->_linesManager->_route->_dir;
		_vm->_linesManager->_route++;

		if ((nexPosX == -1) && (newPosY == -1))
			break;

		++countColisionPixel;
		if (countColisionPixel >= realSpeed) {
			_vm->_globals->_lastDirection = newDirection;
			setSpriteX(0, nexPosX);
			setSpriteY(0, newPosY);
			switch (_vm->_globals->_lastDirection) {
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

	switch (_vm->_globals->_lastDirection) {
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

	_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
}

/**
 * Load Zone
 */
void ObjectsManager::loadZone(const Common::String &file) {
	for (int i = 1; i <= 100; i++) {
		_vm->_linesManager->ZONEP[i]._destX = 0;
		_vm->_linesManager->ZONEP[i]._destY = 0;
		_vm->_linesManager->ZONEP[i]._spriteIndex = 0;
		_vm->_linesManager->ZONEP[i]._verbFl1 = 0;
		_vm->_linesManager->ZONEP[i]._verbFl2 = 0;
		_vm->_linesManager->ZONEP[i]._verbFl3 = 0;
		_vm->_linesManager->ZONEP[i]._verbFl4 = 0;
		_vm->_linesManager->ZONEP[i]._verbFl5 = 0;
		_vm->_linesManager->ZONEP[i]._verbFl6 = 0;
		_vm->_linesManager->ZONEP[i]._verbFl7 = 0;
		_vm->_linesManager->ZONEP[i]._verbFl8 = 0;
		_vm->_linesManager->ZONEP[i]._verbFl9 = 0;
		_vm->_linesManager->ZONEP[i]._verbFl10 = 0;
		_vm->_linesManager->ZONEP[i]._messageId = 0;
		_vm->_linesManager->ZONEP[i]._enabledFl = false;
	}

	Common::File f;
	if (!f.exists(file))
		error("File not found : %s", file.c_str());

	byte *ptr = _vm->_fileManager->loadFile(file);
	int bufId = 0;
	int zoneLineIdx = 0;
	int bobZoneIdx;
	do {
		bobZoneIdx = READ_LE_INT16((uint16 *)ptr + bufId);
		if (bobZoneIdx != -1) {
			_vm->_linesManager->addZoneLine(
			    zoneLineIdx,
			    READ_LE_UINT16((uint16 *)ptr + bufId + 1),
			    READ_LE_UINT16((uint16 *)ptr + bufId + 2),
			    READ_LE_UINT16((uint16 *)ptr + bufId + 3),
			    READ_LE_UINT16((uint16 *)ptr + bufId + 4),
			    bobZoneIdx);
			_vm->_linesManager->ZONEP[bobZoneIdx]._enabledFl = true;
		}
		bufId += 5;
		++zoneLineIdx;
	} while (bobZoneIdx != -1);

	for (int i = 1; i <= 100; i++) {
		_vm->_linesManager->ZONEP[i]._destX = READ_LE_INT16((uint16 *)ptr + bufId);
		_vm->_linesManager->ZONEP[i]._destY = READ_LE_INT16((uint16 *)ptr + bufId + 1);
		_vm->_linesManager->ZONEP[i]._spriteIndex = READ_LE_INT16((uint16 *)ptr + bufId + 2);
		bufId += 3;
	}

	byte *verbData = (ptr + 10 * zoneLineIdx + 606);
	bufId = 0;
	for (int i = 1; i <= 100; i++) {
		_vm->_linesManager->ZONEP[i]._verbFl1 = verbData[bufId];
		_vm->_linesManager->ZONEP[i]._verbFl2 = verbData[bufId + 1];
		_vm->_linesManager->ZONEP[i]._verbFl3 = verbData[bufId + 2];
		_vm->_linesManager->ZONEP[i]._verbFl4 = verbData[bufId + 3];
		_vm->_linesManager->ZONEP[i]._verbFl5 = verbData[bufId + 4];
		_vm->_linesManager->ZONEP[i]._verbFl6 = verbData[bufId + 5];
		_vm->_linesManager->ZONEP[i]._verbFl7 = verbData[bufId + 6];
		_vm->_linesManager->ZONEP[i]._verbFl8 = verbData[bufId + 7];
		_vm->_linesManager->ZONEP[i]._verbFl9 = verbData[bufId + 8];
		_vm->_linesManager->ZONEP[i]._verbFl10 = verbData[bufId + 9];

		bufId += 10;
	}
	verbData += 1010;
	for (int i = 0; i < 100; i++)
		_vm->_linesManager->ZONEP[i + 1]._messageId = READ_LE_UINT16(verbData + 2 * i);

	_vm->_globals->freeMemory(ptr);
	_vm->_linesManager->CARRE_ZONE();
}

void ObjectsManager::handleCityMap() {
	_vm->_dialogsManager->_inventFl = false;
	_vm->_eventsManager->_gameKey = KEY_NONE;
	_vm->_linesManager->setMaxLineIdx(1);
	_vm->_globals->_characterMaxPosY = 440;
	_vm->_globals->_cityMapEnabledFl = true;
	_vm->_graphicsManager->_noFadingFl = false;
	_vm->_globals->_freezeCharacterFl = false;
	_spritePtr = g_PTRNUL;
	_vm->_globals->_exitId = 0;
	_vm->_globals->_checkDistanceFl = true;
	_vm->_soundManager->playSound(31);
	_vm->_globals->iRegul = 1;
	_vm->_graphicsManager->loadImage("PLAN");
	_vm->_linesManager->loadLines("PLAN.OB2");
	_vm->_globals->loadHidingItems("PLAN.CA2");
	loadZone("PLAN.ZO2");
	_spritePtr = _vm->_fileManager->loadFile("VOITURE.SPR");
	_vm->_animationManager->loadAnim("PLAN");
	_vm->_graphicsManager->displayAllBob();
	_vm->_graphicsManager->initScreen("PLAN", 2, false);
	for (int i = 0; i <= 15; i++)
		_vm->_globals->B_CACHE_OFF(i);
	_vm->_globals->B_CACHE_OFF(19);
	_vm->_globals->B_CACHE_OFF(20);
	_vm->_globals->enableHiding();

	if (!_mapCarPosX && !_mapCarPosY) {
		_mapCarPosX = 900;
		_mapCarPosY = 319;
	}
	addStaticSprite(_spritePtr, Common::Point(_mapCarPosX, _mapCarPosY), 0, 1, 0, false, 5, 5);
	_vm->_eventsManager->setMouseXY(_mapCarPosX, _mapCarPosY);
	_vm->_eventsManager->mouseOn();
	_vm->_graphicsManager->scrollScreen(getSpriteX(0) - 320);
	_vm->_graphicsManager->_scrollOffset = getSpriteX(0) - 320;
	animateSprite(0);
	_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
	_vm->_graphicsManager->SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager->SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager->SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager->SETCOLOR3(254, 0, 0, 0);

	for (int i = 0; i <= 4; i++)
		_vm->_eventsManager->refreshScreenAndEvents();

	_vm->_globals->iRegul = 1;
	_vm->_graphicsManager->fadeInLong();
	_vm->_eventsManager->changeMouseCursor(4);
	_vm->_graphicsManager->_noFadingFl = false;

	bool loopCond = false;
	do {
		int mouseButton = _vm->_eventsManager->getMouseButton();
		if (mouseButton) {
			// First cop call : Go to the bank and free the hostages
			if (_vm->_globals->_saveData->_data[svBankAttackAnimPlayedFl] == 1 && !_vm->_globals->_saveData->_data[svCopCall1PlayedFl]) {
				_vm->_globals->_saveData->_data[svCopCall1PlayedFl] = 1;
				_vm->_globals->_introSpeechOffFl = true;
				_vm->_talkManager->startAnimatedCharacterDialogue("APPEL1.pe2");
				_vm->_globals->_introSpeechOffFl = false;
				mouseButton = 0;
			}
			// Second cop call: Helico has been found in the empty lot
			if (_vm->_globals->_saveData->_data[svFreedHostageFl] == 1 && !_vm->_globals->_saveData->_data[svCopCall2PlayedFl]) {
				_vm->_globals->_saveData->_data[svCopCall2PlayedFl] = 1;
				_vm->_globals->_introSpeechOffFl = true;
				_vm->_talkManager->startAnimatedCharacterDialogue("APPEL2.pe2");
				_vm->_globals->_introSpeechOffFl = false;
				mouseButton = 0;
				_vm->_eventsManager->_curMouseButton = 0;
			}
			if (mouseButton == 1)
				handleLeftButton();
		}

		_vm->_linesManager->checkZone();
		GOHOME2();

		if (_vm->_linesManager->_route == (RouteItem *)g_PTRNUL && _vm->_globals->_actionMoveTo)
			PARADISE();
		_vm->_eventsManager->refreshScreenAndEvents();

		if (_vm->_globals->_exitId)
			loopCond = true;
	} while (!_vm->shouldQuit() && !loopCond);

	if (!_vm->_graphicsManager->_noFadingFl)
		_vm->_graphicsManager->fadeOutLong();
	_vm->_globals->iRegul = 0;
	_vm->_graphicsManager->_noFadingFl = false;
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
	_vm->_fontManager->hideText(9);
	int destX = _vm->_eventsManager->getMouseX();
	int destY = _vm->_eventsManager->getMouseY();

	if (!_vm->_dialogsManager->_inventFl && !_vm->_globals->_cityMapEnabledFl && 
		destX > _vm->_graphicsManager->_scrollOffset - 30 && destX < _vm->_graphicsManager->_scrollOffset + 50 && 
		destY > -30 && destY < 50) {
		int oldMouseCursor = _vm->_eventsManager->_mouseCursorId;
		_vm->_dialogsManager->_inventFl = true;
		_vm->_dialogsManager->showInventory();
		_vm->_dialogsManager->_inventFl = false;
		_vm->_eventsManager->_gameKey = KEY_NONE;
		if (!_vm->_globals->_exitId) {
			_vm->_dialogsManager->_inventFl = false;
			_vm->_eventsManager->_mouseCursorId = oldMouseCursor;
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
		_vm->_linesManager->checkZone();
		if (_zoneNum <= 0)
			return;
		int routeIdx = 0;
		do {
			_vm->_linesManager->_testRoute2[routeIdx] = _vm->_linesManager->_route[routeIdx];
			++routeIdx;
		} while (_vm->_linesManager->_route[routeIdx]._x != -1);

		_vm->_linesManager->_testRoute2[routeIdx].invalidate();
	}

	if (_vm->_globals->_actionMoveTo) {
		_vm->_linesManager->checkZone();
		_vm->_globals->_actionMoveTo = false;
		_vm->_globals->_saveData->_data[svLastMouseCursor] = 0;
		_vm->_globals->_saveData->_data[svLastZoneNum] = 0;
	}

	if (_vm->_globals->_cityMapEnabledFl && (_vm->_eventsManager->_mouseCursorId != 4 || _zoneNum <= 0))
		return;
	if (_zoneNum != -1 && _zoneNum != 0) {
		if (_vm->_linesManager->ZONEP[_zoneNum]._destX && _vm->_linesManager->ZONEP[_zoneNum]._destY && _vm->_linesManager->ZONEP[_zoneNum]._destY != 31) {
			destX = _vm->_linesManager->ZONEP[_zoneNum]._destX;
			destY = _vm->_linesManager->ZONEP[_zoneNum]._destY;
		}
	}
	_vm->_globals->_actionMoveTo = false;
	RouteItem *oldRoute = _vm->_linesManager->_route;
	_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
	if (_forestFl && _zoneNum >= 20 && _zoneNum <= 23) {
		if (getSpriteY(0) > 374 && getSpriteY(0) <= 410) {
			_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
			setSpriteIndex(0, _vm->_globals->_oldDirectionSpriteIdx);
			_vm->_globals->_actionDirection = DIR_NONE;
			_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
			computeAndSetSpriteSize();
			setFlipSprite(0, false);
			_homeRateCounter = 0;
			_vm->_globals->_oldDirection = DIR_NONE;
		} else {
			_vm->_linesManager->_route = _vm->_linesManager->PARCOURS2(getSpriteX(0), getSpriteY(0), getSpriteX(0), 390);
			if (_vm->_linesManager->_route != (RouteItem *)g_PTRNUL)
				_vm->_linesManager->PACOURS_PROPRE(_vm->_linesManager->_route);
			_oldCharacterPosX = getSpriteX(0);
			_oldCharacterPosY = getSpriteY(0);
			_homeRateCounter = 0;
			if (_vm->_linesManager->_route != (RouteItem *)g_PTRNUL || oldRoute == _vm->_linesManager->_route) {
				_vm->_globals->_oldDirection = DIR_NONE;
			} else {
				_vm->_linesManager->_route = oldRoute;
			}
		}
	} else {
		if (!_vm->_globals->_freezeCharacterFl && !_vm->_globals->_cityMapEnabledFl) {
			_vm->_linesManager->_route = _vm->_linesManager->PARCOURS2(getSpriteX(0), getSpriteY(0), destX, destY);
			if (_vm->_linesManager->_route != (RouteItem *)g_PTRNUL)
				_vm->_linesManager->PACOURS_PROPRE(_vm->_linesManager->_route);
			_oldCharacterPosX = getSpriteX(0);
			_oldCharacterPosY = getSpriteY(0);
			_homeRateCounter = 0;
			if (_vm->_linesManager->_route != (RouteItem *)g_PTRNUL || oldRoute == _vm->_linesManager->_route)
				_vm->_globals->_oldDirection = DIR_NONE;
			else
				_vm->_linesManager->_route = oldRoute;
		}
	}

	if (!_vm->_globals->_freezeCharacterFl && _vm->_globals->_cityMapEnabledFl)
		_vm->_linesManager->_route = _vm->_linesManager->cityMapCarRoute(getSpriteX(0), getSpriteY(0), destX, destY);

	if (_zoneNum != -1 && _zoneNum != 0) {
		if (_vm->_eventsManager->_mouseCursorId == 23)
			_vm->_globals->_saveData->_data[svLastMouseCursor] = 5;
		else 
			_vm->_globals->_saveData->_data[svLastMouseCursor] = _vm->_eventsManager->_mouseCursorId;

		if (_vm->_globals->_cityMapEnabledFl)
			_vm->_globals->_saveData->_data[svLastMouseCursor] = 6;
		_vm->_globals->_saveData->_data[svLastZoneNum] = _zoneNum;
		_vm->_globals->_saveData->_data[svLastObjectIndex] = _curObjectIndex;
		_vm->_globals->_actionMoveTo = true;
	}
	_vm->_fontManager->hideText(5);
	_vm->_graphicsManager->SETCOLOR4(251, 100, 100, 100);
	if (_vm->_globals->_screenId == 20 && _vm->_globals->_saveData->_data[svField132] == 1
				&& _curObjectIndex == 20 && _zoneNum == 12
				&& _vm->_eventsManager->_mouseCursorId == 23) {
		// Special case for throwing darts at the switch in Purgatory - the player shouldn't move
		_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
		getSpriteX(0);
		getSpriteY(0);
	}
}

void ObjectsManager::PARADISE() {
	char result = _vm->_globals->_saveData->_data[svLastMouseCursor];
	if (result && _vm->_globals->_saveData->_data[svLastZoneNum] && result != 4 && result > 3) {
		_vm->_fontManager->hideText(5);
		if (!_forestFl || _zoneNum < 20 || _zoneNum > 23) {
			if (_vm->_graphicsManager->_largeScreenFl) {
				_vm->_graphicsManager->_scrollStatus = 2;
				if (_vm->_eventsManager->_startPos.x + 320 - getSpriteX(0) > 160) {
					bool loopCond = false;
					do {
						_vm->_graphicsManager->_scrollPosX -= _vm->_graphicsManager->_scrollSpeed;
						if (_vm->_graphicsManager->_scrollPosX < 0) {
							_vm->_graphicsManager->_scrollPosX = 0;
							loopCond = true;
						}
						if (_vm->_graphicsManager->_scrollPosX > SCREEN_WIDTH) {
							_vm->_graphicsManager->_scrollPosX = SCREEN_WIDTH;
							loopCond = true;
						}
						if (_vm->_eventsManager->getMouseX() > _vm->_graphicsManager->_scrollPosX + 620)
							_vm->_eventsManager->setMouseXY(_vm->_eventsManager->_mousePos.x - 4, _vm->_eventsManager->getMouseY());

						_vm->_eventsManager->refreshScreenAndEvents();
					} while (!loopCond && _vm->_eventsManager->_startPos.x > getSpriteX(0) - 320);
				} else if (_vm->_eventsManager->_startPos.x + 320 - getSpriteX(0) < -160) {
					bool loopCond = false;
					do {
						_vm->_graphicsManager->_scrollPosX += _vm->_graphicsManager->_scrollSpeed;
						if (_vm->_graphicsManager->_scrollPosX < 0) {
							_vm->_graphicsManager->_scrollPosX = 0;
							loopCond = true;
						}
						if (_vm->_graphicsManager->_scrollPosX > SCREEN_WIDTH) {
							_vm->_graphicsManager->_scrollPosX = SCREEN_WIDTH;
							loopCond = true;
						}
						if (_vm->_eventsManager->getMouseX() < _vm->_graphicsManager->_scrollPosX + 10)
							_vm->_eventsManager->setMouseXY(_vm->_eventsManager->_mousePos.x + 4, _vm->_eventsManager->getMouseY());

						_vm->_eventsManager->refreshScreenAndEvents();
					} while (!loopCond && _vm->_eventsManager->_startPos.x < getSpriteX(0) - 320);
				}
				if (_vm->_eventsManager->getMouseX() > _vm->_graphicsManager->_scrollPosX + 620)
					_vm->_eventsManager->setMouseXY(_vm->_graphicsManager->_scrollPosX + 610, 0);
				if (_vm->_eventsManager->getMouseX() < _vm->_graphicsManager->_scrollPosX + 10)
					_vm->_eventsManager->setMouseXY(_vm->_graphicsManager->_scrollPosX + 10, 0);
				_vm->_eventsManager->refreshScreenAndEvents();
				_vm->_graphicsManager->_scrollStatus = 0;
			}
			_vm->_talkManager->REPONSE(_vm->_globals->_saveData->_data[svLastZoneNum], _vm->_globals->_saveData->_data[svLastMouseCursor]);
		} else {
			_vm->_talkManager->REPONSE2(_vm->_globals->_saveData->_data[svLastZoneNum], _vm->_globals->_saveData->_data[svLastMouseCursor]);
		}
		_vm->_eventsManager->changeMouseCursor(4);
		if (_zoneNum != -1 && _zoneNum != 0 && !_vm->_linesManager->ZONEP[_zoneNum]._enabledFl) {
			_zoneNum = -1;
			_forceZoneFl = true;
		}
		if (_zoneNum != _vm->_globals->_saveData->_data[svLastZoneNum] || _zoneNum == -1 || _zoneNum == 0) {
			_vm->_eventsManager->_mouseCursorId = 4;
			_changeVerbFl = false;
		} else {
			_vm->_eventsManager->_mouseCursorId = _vm->_globals->_saveData->_data[svLastMouseCursor];
			if (_changeVerbFl) {
				nextVerbIcon();
				_changeVerbFl = false;
			}
			if (_vm->_eventsManager->_mouseCursorId == 5)
				_vm->_eventsManager->_mouseCursorId = 4;
		}
		if (_vm->_eventsManager->_mouseCursorId != 23)
			_vm->_eventsManager->changeMouseCursor(_vm->_eventsManager->_mouseCursorId);
		_zoneNum = 0;
		_vm->_globals->_saveData->_data[svLastMouseCursor] = 0;
		_vm->_globals->_saveData->_data[svLastZoneNum] = 0;
	}
	if (_vm->_globals->_cityMapEnabledFl) {
		_vm->_eventsManager->_mouseCursorId = 0;
		_vm->_eventsManager->changeMouseCursor(0);
	}
	if (_vm->_globals->_freezeCharacterFl && _vm->_eventsManager->_mouseCursorId == 4) {
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
	_vm->_graphicsManager->endDisplayBob();
	_vm->_fontManager->hideText(5);
	_vm->_fontManager->hideText(9);
	_vm->_globals->clearVBob();
	_vm->_animationManager->clearAnim();
	_vm->_linesManager->clearAllZones();
	_vm->_linesManager->resetLines();
	_vm->_globals->resetHidingItems();

	for (int i = 0; i <= 48; i++) {
		_vm->_linesManager->BOBZONE[i] = 0;
		_vm->_linesManager->BOBZONE_FLAG[i] = false;
	}
	_vm->_eventsManager->_mouseCursorId = 4;
	_verb = 4;
	_zoneNum = 0;
	_forceZoneFl = true;
	_vm->_linesManager->resetLinesNumb();
	_vm->_linesManager->resetLastLine();
	_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
	_vm->_globals->_answerBuffer = _vm->_globals->freeMemory(_vm->_globals->_answerBuffer);
	_vm->_globals->SPRITE_ECRAN = _vm->_globals->freeMemory(_vm->_globals->SPRITE_ECRAN);
	_vm->_eventsManager->_startPos.x = 0;
	_vm->_eventsManager->_mouseSpriteId = 0;
	_vm->_globals->_saveData->_data[svLastMouseCursor] = 0;
	_vm->_globals->_saveData->_data[svLastZoneNum] = 0;
	_vm->_globals->_actionMoveTo = false;
	_forceZoneFl = true;
	_changeVerbFl = false;
	_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
	_vm->_globals->_oldDirection = DIR_NONE;
	_vm->_graphicsManager->resetDirtyRects();
}

/**
 * Change the currently active player face / Head
 * @param oldCharacter		Previously played character
 * @param newCharacter		New character to play
 */
void ObjectsManager::changeCharacterHead(PlayerCharacter oldCharacter, PlayerCharacter newCharacter) {
	CharacterLocation *loc;

	_changeHeadFl = true;
	_vm->_graphicsManager->copySurface(_vm->_graphicsManager->_vesaScreen, 532, 25, 65, 40, _vm->_graphicsManager->_vesaBuffer, 532, 25);
	_vm->_graphicsManager->addDirtyRect(532, 25, 597, 65);
	_vm->_globals->_checkDistanceFl = true;
	_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;

	if (oldCharacter == CHARACTER_SAMANTHA && newCharacter == CHARACTER_HOPKINS
		&& _vm->_globals->_saveData->_realHopkins._location == _vm->_globals->_screenId) {
		_changeHeadFl = false;
		loc = &_vm->_globals->_saveData->_samantha;
		loc->_pos.x = getSpriteX(0);
		loc->_pos.y = getSpriteY(0);
		loc->_startSpriteIndex = 64;
		loc->_location = _vm->_globals->_screenId;
		loc->_zoomFactor = _sprite[0]._animationType;

		removeSprite(1);
		addStaticSprite(_headSprites, loc->_pos, 1, 3, loc->_zoomFactor, false, 20, 127);
		animateSprite(1);
		removeSprite(0);

		_vm->_globals->_saveData->_data[svField354] = 0;
		_vm->_globals->_saveData->_data[svField356] = 0;
		_vm->_globals->_saveData->_data[svField357] = 1;

		loc = &_vm->_globals->_saveData->_realHopkins;
		_vm->_globals->PERSO = _vm->_fileManager->loadFile("PERSO.SPR");
		_vm->_globals->_characterType = 0;
		addStaticSprite(_vm->_globals->PERSO, loc->_pos, 0, 64, loc->_zoomFactor, false, 34, 190);
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
		_vm->_globals->PERSO = _vm->_fileManager->loadFile("PSAMAN.SPR");
		_vm->_globals->_characterType = 2;
		addStaticSprite(_vm->_globals->PERSO, loc->_pos, 0, 64, loc->_zoomFactor, false, 20, 127);
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
		}
	}
}

// Check Size
void ObjectsManager::computeAndSetSpriteSize() {
	int size = _vm->_globals->_spriteSize[getSpriteY(0)];
	if (_vm->_globals->_characterType == 1) {
		size = 20 * (5 * abs(size) - 100) / -80;
	} else if (_vm->_globals->_characterType == 2) {
		size = 20 * (5 * abs(size) - 165) / -67;
	}
	setSpriteZoom(0, size);
}

/**
 * Get next verb icon (or text)
 */
void ObjectsManager::nextVerbIcon() {
	_vm->_eventsManager->_mouseCursorId++;

	for(;;) {
		if (_vm->_eventsManager->_mouseCursorId == 4) {
			if (!_vm->_globals->_freezeCharacterFl || _zoneNum == -1 || _zoneNum == 0)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 5 || _vm->_eventsManager->_mouseCursorId == 6) {
			_vm->_eventsManager->_mouseCursorId = 6;
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl1 == 1)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 7) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl2 == 1)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 8) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl3 == 1)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 9) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl4 == 1)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 10) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl5 == 1)
				return;
			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 11) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl6 == 1)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 12) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl7 == 1)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 13) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl8 == 1)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 14) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl9 == 1)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 15) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl10 == 1)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 16) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl1 == 2)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 17) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl4 == 2)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 18) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl5 == 2)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 19) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl6 == 2)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 20) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl7 == 2)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 21) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl10 == 2)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 22) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl8 == 2)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 23) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl3 == 2)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 24) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl4 == 3)
				return;

			++_vm->_eventsManager->_mouseCursorId;
		}

		if (_vm->_eventsManager->_mouseCursorId == 25) {
			if (_vm->_linesManager->ZONEP[_zoneNum]._verbFl9 == 2)
				return;
		}
		_vm->_eventsManager->_mouseCursorId = 4;
	}
}

/**
 * Handle Right button
 */
void ObjectsManager::handleRightButton() {
	if (_zoneNum != -1 && _zoneNum != 0) {
		nextVerbIcon();
		if (_vm->_eventsManager->_mouseCursorId != 23)
			_vm->_eventsManager->changeMouseCursor(_vm->_eventsManager->_mouseCursorId);
		_verb = _vm->_eventsManager->_mouseCursorId;
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
		_borderPos.x = _vm->_graphicsManager->_scrollOffset + 158;
	else if (zoneIdx == 2 || zoneIdx == 8 || zoneIdx == 14 || zoneIdx == 20 || zoneIdx == 26)
		_borderPos.x = _vm->_graphicsManager->_scrollOffset + 212;
	else if (zoneIdx == 3 || zoneIdx == 9 || zoneIdx == 15 || zoneIdx == 21 || zoneIdx == 27)
		_borderPos.x = _vm->_graphicsManager->_scrollOffset + 266;
	else if (zoneIdx == 4 || zoneIdx == 10 || zoneIdx == 16 || zoneIdx == 22 || zoneIdx == 28)
		_borderPos.x = _vm->_graphicsManager->_scrollOffset + 320;
	else if (zoneIdx == 5 || zoneIdx == 11 || zoneIdx == 17 || zoneIdx == 23 || zoneIdx == 29)
		_borderPos.x = _vm->_graphicsManager->_scrollOffset + 374;
	else if (zoneIdx == 6 || zoneIdx == 12 || zoneIdx == 18 || zoneIdx == 24 || zoneIdx == 30 || zoneIdx == 31)
		_borderPos.x = _vm->_graphicsManager->_scrollOffset + 428;

	if (zoneIdx >= 1 && zoneIdx <= 29)
		_borderSpriteIndex = 0;
	else if (zoneIdx == 30 || zoneIdx == 31)
		_borderSpriteIndex = 2;
	else if (!zoneIdx || zoneIdx == 32) {
		_borderPos = Common::Point(0, 0);
		_borderSpriteIndex = 0;
	}

	if (!zoneIdx)
		_vm->_eventsManager->_mouseCursorId = 0;
	else if (zoneIdx >= 1 && zoneIdx <= 28)
		_vm->_eventsManager->_mouseCursorId = 8;
	else if (zoneIdx == 29)
		_vm->_eventsManager->_mouseCursorId = 1;
	else if (zoneIdx == 30)
		_vm->_eventsManager->_mouseCursorId = 2;
	else if (zoneIdx == 31)
		_vm->_eventsManager->_mouseCursorId = 3;
	else if (zoneIdx == 32)
		_vm->_eventsManager->_mouseCursorId = 16;

	if (zoneIdx >= 1 && zoneIdx <= 28 && !_vm->_globals->_inventory[zoneIdx]) {
		_vm->_eventsManager->_mouseCursorId = 0;
		_borderPos = Common::Point(0, 0);
		_borderSpriteIndex = 0;
	}

	if (_vm->_eventsManager->_mouseCursorId != 23)
		_vm->_eventsManager->changeMouseCursor(_vm->_eventsManager->_mouseCursorId);
	_vm->_eventsManager->getMouseX();
	_vm->_eventsManager->getMouseY();
}

/**
 * Get next icon for an object in the inventory
 */
void ObjectsManager::nextObjectIcon(int idx) {
	if (_vm->_eventsManager->_mouseCursorId == 0 || _vm->_eventsManager->_mouseCursorId == 2 ||
	    _vm->_eventsManager->_mouseCursorId == 3 || _vm->_eventsManager->_mouseCursorId == 16)
		return;
		
	int nextCursorId = _vm->_eventsManager->_mouseCursorId + 1;
	if (nextCursorId > 25)
		nextCursorId = 6;

	do {
		if (nextCursorId == 2 || nextCursorId == 5 || nextCursorId == 6) {
			_vm->_eventsManager->_mouseCursorId = 6;
			if (_vm->_globals->_objectAuthIcons[_vm->_globals->_inventory[idx]]._flag1 == 1)
				return;
			nextCursorId++;
		}
		if (nextCursorId == 7) {
			_vm->_eventsManager->_mouseCursorId = 7;
			if (_vm->_globals->_objectAuthIcons[_vm->_globals->_inventory[idx]]._flag2 == 1)
				return;
			nextCursorId++;
		}	
		if (nextCursorId == 8) {
			_vm->_eventsManager->_mouseCursorId = 8;
			return;
		}
		if (nextCursorId == 9 || nextCursorId == 10) {
			_vm->_eventsManager->_mouseCursorId = 10;
			if (_vm->_globals->_objectAuthIcons[_vm->_globals->_inventory[idx]]._flag6 == 1)
				return;
			nextCursorId = 11;
		}

		if (nextCursorId == 11) {
			_vm->_eventsManager->_mouseCursorId = 11;
			if (_vm->_globals->_objectAuthIcons[_vm->_globals->_inventory[idx]]._flag3 == 1)
				return;
			nextCursorId++;
		}

		if (nextCursorId == 12 || nextCursorId == 13) {
			_vm->_eventsManager->_mouseCursorId = 13;
			if (_vm->_globals->_objectAuthIcons[_vm->_globals->_inventory[idx]]._flag4 == 1)
				return;
			nextCursorId = 14;
		}

		if (nextCursorId == 14 || nextCursorId == 15) {
			_vm->_eventsManager->_mouseCursorId = 15;
			if (_vm->_globals->_objectAuthIcons[_vm->_globals->_inventory[idx]]._flag5 == 1)
				return;
			nextCursorId = 23;
		}

		if (nextCursorId >= 16 && nextCursorId <= 23) {
			_vm->_eventsManager->_mouseCursorId = 23;
			if (_vm->_globals->_objectAuthIcons[_vm->_globals->_inventory[idx]]._flag5 == 2)
				return;
			nextCursorId = 24;
		}

		if (nextCursorId == 24 || nextCursorId == 25) {
			_vm->_eventsManager->_mouseCursorId = 25;
		}
		
		nextCursorId = 6;
	} while (_vm->_globals->_objectAuthIcons[_vm->_globals->_inventory[idx]]._flag6 != 2);
}

void ObjectsManager::takeInventoryObject(int idx) {
	if (_vm->_eventsManager->_mouseCursorId == 8)
		changeObject(idx);
}

void ObjectsManager::OPTI_OBJET() {
	byte *data;
	Common::String file;
	int lastOpcodeResult = 1;

	file = "OBJET1.ini";
	data = _vm->_fileManager->searchCat(file, RES_INI);
	if (data == g_PTRNUL) {
		data = _vm->_fileManager->loadFile(file);
		if (data == g_PTRNUL)
			error("INI file %s not found", file.c_str());
	}

	if (READ_BE_UINT24(data) != MKTAG24('I', 'N', 'I'))
		error("File %s is not an INI file", file.c_str());

	for (;;) {
		int opcodeType = _vm->_scriptManager->handleOpcode(data + 20 * lastOpcodeResult);
		if (_vm->shouldQuit())
			return;

		if (opcodeType == 2)
			lastOpcodeResult = _vm->_scriptManager->handleGoto(data + 20 * lastOpcodeResult);
		else if (opcodeType == 3)
			lastOpcodeResult = _vm->_scriptManager->handleIf(data, lastOpcodeResult);

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
		_vm->_talkManager->startAnimatedCharacterDialogue("flicspe1.pe2");
		_vm->_globals->_introSpeechOffFl = false;

		if (_vm->_globals->_censorshipFl)
			break;

		oldPalette = _vm->_globals->allocMemory(1000);
		memcpy(oldPalette, _vm->_graphicsManager->_palette, 769);

		_vm->_saveLoadManager->saveFile("TEMP1.SCR", _vm->_graphicsManager->_vesaScreen, 307200);

		if (!_vm->_graphicsManager->_lineNbr)
			_vm->_graphicsManager->_scrollOffset = 0;
		_vm->_graphicsManager->NB_SCREEN(true);
		_vm->_soundManager->_specialSoundNum = 198;
		PERSO_ON = true;
		_vm->_animationManager->unsetClearAnimFlag();
		_vm->_animationManager->playAnim("otage.ANM", 1, 24, 500, true);
		_vm->_soundManager->_specialSoundNum = 0;
		_vm->_graphicsManager->NB_SCREEN(false);

		_vm->_saveLoadManager->load("TEMP1.SCR", _vm->_graphicsManager->_vesaScreen);
		g_system->getSavefileManager()->removeSavefile("TEMP1.SCR");

		PERSO_ON = false;
		memcpy(_vm->_graphicsManager->_palette, oldPalette, 769);
		_vm->_graphicsManager->setPaletteVGA256(_vm->_graphicsManager->_palette);
		_vm->_globals->freeMemory(oldPalette);
		_vm->_graphicsManager->lockScreen();
		_vm->_graphicsManager->m_scroll16(_vm->_graphicsManager->_vesaScreen, _vm->_eventsManager->_startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsManager->unlockScreen();
		memcpy(_vm->_graphicsManager->_vesaBuffer, _vm->_graphicsManager->_vesaScreen, 614399);

		_vm->_graphicsManager->_scrollStatus = 0;
		_vm->_graphicsManager->updateScreen();
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
			_vm->_linesManager->disableZone(15);
			_vm->_soundManager->playSoundFile("SOUND75.WAV");
		}
		if (_vm->_globals->_saveData->_data[svField261] == 2 && getBobAnimDataIdx(6) == 6) {
			stopBobAnimation(6);
			setBobAnimDataIdx(6, 0);
			setBobAnimation(7);
			_vm->_linesManager->enableZone(14);
			_vm->_globals->_saveData->_data[svField261] = 3;
		}
		_vm->_globals->_disableInventFl = false;
		break;
	case 93:
		if (_vm->_globals->_saveData->_data[svField333])
			break;

		_vm->_globals->_disableInventFl = true;
		do
			_vm->_eventsManager->refreshScreenAndEvents();
		while (getBobAnimDataIdx(8) != 3);
		_vm->_globals->_introSpeechOffFl = true;
		_vm->_talkManager->startAnimatedCharacterDialogue("GM3.PE2");
		stopBobAnimation(8);
		_vm->_globals->_saveData->_data[svField333] = 1;
		_vm->_globals->_disableInventFl = false;
		break;
	}
}

void ObjectsManager::BOB_VIVANT(int idx) {
	int startPos = 10 * idx;
	if (!READ_LE_UINT16(_vm->_talkManager->_characterAnim + startPos + 4))
		return;

	int xp = READ_LE_INT16(_vm->_talkManager->_characterAnim + startPos);
	int yp = READ_LE_INT16(_vm->_talkManager->_characterAnim + startPos + 2);
	int spriteIndex = _vm->_talkManager->_characterAnim[startPos + 8];

	_vm->_graphicsManager->fastDisplay(_vm->_talkManager->_characterSprite, xp, yp, spriteIndex);
}

void ObjectsManager::VBOB(byte *src, int idx, int xp, int yp, int frameIndex) {
	if (idx > 29)
		error("MAX_VBOB exceeded");

	VBobItem *vbob = &_vm->_globals->VBob[idx];
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

void ObjectsManager::VBOB_OFF(int idx) {
	if (idx > 29)
		error("MAX_VBOB exceeded");

	VBobItem *vbob = &_vm->_globals->VBob[idx];
	if (vbob->_displayMode <= 1)
		vbob->_displayMode = 0;
	else
		vbob->_displayMode = 4;
}

void ObjectsManager::doActionBack(int idx) {
	if (_curGestureFile != 1) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 1;
		_gestureBuf = _vm->_fileManager->loadFile("DOS.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_gestureBuf, "0,1,2,3,4,5,6,7,8,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 2:
		SPACTION(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,-1,", 8, false);
		break;
	case 3:
		SPACTION1(_gestureBuf, "12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8);
		break;
	case 4:
		ACTION(_gestureBuf, "0,1,2,3,4,5,6,7,8,8,8,8,8,8,9,10,11,12,13,12,11,12,13,12,11,12,13,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 5:
		SPACTION(_gestureBuf, "15,16,17,18,19,20,21,-1,", 8, false);
		break;
	case 6:
		SPACTION1(_gestureBuf, "20,19,18,17,16,15,-1,", 8);
		break;
	case 7:
		SPACTION(_gestureBuf, "15,16,17,18,19,20,21,22,23,24,-1,", 8, false);
		break;
	case 8:
		SPACTION1(_gestureBuf, "23,22,21,20,19,18,17,16,15,-1,", 8);
		break;
	case 9:
		SPACTION(_gestureBuf, "15,16,17,18,19,20,21,22,23,24,-1,", 8, false);
		break;
	case 10:
		SPACTION1(_gestureBuf, "23,22,21,20,19,18,17,16,15,-1,", 8);
		break;
	}
}

void ObjectsManager::doActionRight(int idx) {
	if (_curGestureFile != 3) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 3;
		_gestureBuf = _vm->_fileManager->loadFile("PROFIL.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_gestureBuf, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 8, false);
		break;
	case 2:
		SPACTION(_gestureBuf, "1,2,3,4,5,6,7,8,-1,", 8, false);
		break;
	case 3:
		SPACTION1(_gestureBuf, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 8);
		break;
	case 4:
		ACTION(_gestureBuf, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 8, false);
		break;
	case 5:
		SPACTION(_gestureBuf, "23,24,25,-1,", 8, false);
		break;
	case 6:
		SPACTION1(_gestureBuf, "24,,23,-1,", 8);
		break;
	case 7:
		SPACTION(_gestureBuf, "23,24,25,26,27,-1,", 8, false);
		break;
	case 8:
		SPACTION1(_gestureBuf, "26,25,24,23,-1,", 8);
		break;
	case 9:
		SPACTION(_gestureBuf, "23,24,25,26,27,28,29,-1,", 8, false);
		break;
	case 10:
		SPACTION1(_gestureBuf, "28,27,26,25,24,23,-1,", 8);
		break;
	}
}

void ObjectsManager::doActionDiagRight(int idx) {
	if (_curGestureFile != 4) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 4;
		_gestureBuf = _vm->_fileManager->loadFile("3Q.SPR");
	}

	switch (idx) {
	case 1:
		ACTION(_gestureBuf, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 2:
		SPACTION(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 8, false);
		break;
	case 3:
		SPACTION1(_gestureBuf, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8);
		break;
	case 4:
		ACTION(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 5:
		SPACTION(_gestureBuf, "15,16,17,18,-1,", 8, false);
		break;
	case 6:
		SPACTION1(_gestureBuf, "17,16,15,-1,", 8);
		break;
	case 7:
		SPACTION(_gestureBuf, "15,16,17,18,19,20-1,", 8, false);
		break;
	case 8:
		SPACTION1(_gestureBuf, "19,18,17,16,15,-1,", 8);
		break;
	case 9:
		SPACTION(_gestureBuf, "15,16,17,18,19,20,21,-1,", 8, false);
		break;
	case 10:
		SPACTION1(_gestureBuf, "20,19,18,17,15,-1,", 8);
		break;
	}
}

void ObjectsManager::doActionFront(int idx) {
	if (_curGestureFile != 2) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 2;
		_gestureBuf = _vm->_fileManager->loadFile("FACE.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_gestureBuf, "0,1,2,3,4,5,6,7,9,9,9,9,9,9,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	case 2:
		SPACTION(_gestureBuf, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,15,-1,", 8, false);
		break;
	case 3:
		SPACTION1(_gestureBuf, "14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 8);
		break;
	case 4:
		ACTION(_gestureBuf, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 8, false);
		break;
	}
}

void ObjectsManager::doActionDiagLeft(int idx) {
	if (_curGestureFile != 4) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 4;
		_gestureBuf = _vm->_fileManager->loadFile("3Q.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_gestureBuf, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 8, true);
		break;
	case 2:
		SPACTION(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 8, true);
		break;
	case 3:
		SPACTION1(_gestureBuf, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8);
		break;
	case 4:
		ACTION(_gestureBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 8, true);
		break;
	case 5:
		SPACTION(_gestureBuf, "15,16,17,18,-1,", 8, true);
		break;
	case 6:
		SPACTION1(_gestureBuf, "17,16,15,-1,", 8);
		break;
	case 7:
		SPACTION(_gestureBuf, "15,16,17,18,19,20,-1,", 8, true);
		break;
	case 8:
		SPACTION1(_gestureBuf, "19,18,17,16,15,-1,", 8);
		break;
	case 9:
		SPACTION(_gestureBuf, "15,16,17,18,19,20,21,-1,", 8, true);
		break;
	case 10:
		SPACTION1(_gestureBuf, "20,19,18,17,15,-1,", 8);
		break;
	}
}

void ObjectsManager::doActionLeft(int idx) {
	if (_curGestureFile != 3) {
		_gestureBuf = _vm->_globals->freeMemory(_gestureBuf);
		_curGestureFile = 3;
		_gestureBuf = _vm->_fileManager->loadFile("PROFIL.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_gestureBuf, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 8, true);
		break;
	case 2:
		SPACTION(_gestureBuf, "1,2,3,4,5,6,7,8,-1,", 8, true);
		break;
	case 3:
		SPACTION1(_gestureBuf, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 8);
		break;
	case 4:
		ACTION(_gestureBuf, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 8, true);
		break;
	case 5:
		SPACTION(_gestureBuf, "23,24,25,-1,", 8, true);
		break;
	case 6:
		SPACTION1(_gestureBuf, "24,,23,-1,", 8);
		break;
	case 7:
		SPACTION(_gestureBuf, "23,24,25,26,27,-1,", 8, true);
		break;
	case 8:
		SPACTION1(_gestureBuf, "26,25,24,23,-1,", 8);
		break;
	case 9:
		SPACTION(_gestureBuf, "23,24,25,26,27,28,29,-1,", 8, true);
		break;
	case 10:
		SPACTION1(_gestureBuf, "28,27,26,25,24,23,-1,", 8);
		break;
	}
}

void ObjectsManager::OPTI_ONE(int idx, int animIdx, int destPosi, int animAction) {
	// Set Hopkins animation and position
	if (animAction != 3) {
		setBobAnimation(idx);
		setBobAnimDataIdx(idx, animIdx);
	}

	// Make Hopkins walk to the expected place
	do {
		_vm->_eventsManager->refreshScreenAndEvents();
	} while (destPosi != getBobAnimDataIdx(idx));

	if (!animAction)
		stopBobAnimation(idx);
	else if (animAction == 4) {
		_vm->_graphicsManager->fastDisplay(_bob[idx]._spriteData,
			_bob[idx]._oldX, _bob[idx]._oldY, _bob[idx]._frameIndex);
		stopBobAnimation(idx);
		_vm->_eventsManager->refreshScreenAndEvents();
	}
}

int ObjectsManager::getBobAnimDataIdx(int idx) {
	return _bob[idx]._animDataIdx / 5;
}

void ObjectsManager::setBobAnimDataIdx(int idx, int animIdx) {
	_bob[idx]._animDataIdx = 5 * animIdx;
	_bob[idx]._moveChange1 = 0;
	_bob[idx]._moveChange2 = 0;
}

/**
 * Set Hopkins animation
 */
void ObjectsManager::setBobAnimation(int idx) {
	if (!_bob[idx]._disabledAnimationFl)
		return;

	_bob[idx]._disabledAnimationFl = false;
	_bob[idx]._animDataIdx = 5;
	_bob[idx]._frameIndex = 250;
	_bob[idx]._moveChange1 = 0;
	_bob[idx]._moveChange2 = 0;
}

/**
 * Stop Hopkins animation
 */
void ObjectsManager::stopBobAnimation(int idx) {
	_bob[idx]._disabledAnimationFl = true;
}

/**
 * Get X position
 */
int ObjectsManager::getBobPosX(int idx) {
	return _bob[idx]._xp;
}

int ObjectsManager::getBobPosY(int idx) {
	return _bob[idx]._yp;
}

int ObjectsManager::getBobFrameIndex(int idx) {
	return _bob[idx]._frameIndex;
}

void ObjectsManager::loadLinkFile(const Common::String &file) {
	Common::File f;
	Common::String filename = file + ".LNK";
	byte *ptr = _vm->_fileManager->searchCat(filename, RES_LIN);
	size_t nbytes = _vm->_globals->_catalogSize;
	if (ptr == g_PTRNUL) {
		if (!f.open(filename))
			error("Error opening file - %s", filename.c_str());

		nbytes = f.size();
		ptr = _vm->_globals->allocMemory(nbytes);
		if (g_PTRNUL == ptr)
			error("INILINK");
		_vm->_fileManager->readStream(f, ptr, nbytes);
		f.close();
	}
	if (!OBSSEUL) {
		for (int idx = 0; idx < 500; ++idx)
			_vm->_globals->_spriteSize[idx] = READ_LE_INT16((uint16 *)ptr + idx);

		_vm->_globals->resetHidingItems();

		Common::String filename2 = Common::String((const char *)ptr + 1000);
		if (!filename2.empty()) {
			_vm->_globals->_hidingItemData[1] = _vm->_fileManager->searchCat(filename2, RES_SLI);

			if (_vm->_globals->_hidingItemData[1] || _vm->_globals->_hidingItemData[1] == g_PTRNUL) {
				_vm->_globals->_hidingItemData[1] = _vm->_fileManager->loadFile(filename2);
			} else {
				_vm->_globals->_hidingItemData[1] = _vm->_fileManager->loadFile("RES_SLI.RES");
			}

			int curDataCacheId = 60;
			byte *curDataPtr = ptr + 1000;
			for (int hidingIdx = 0; hidingIdx <= 21; hidingIdx++) {
				HidingItem *hid = &_vm->_globals->_hidingItem[hidingIdx];
				int curSpriteId = READ_LE_INT16(curDataPtr + 2 * curDataCacheId);
				hid->_spriteIndex = curSpriteId;
				hid->_x = READ_LE_INT16(curDataPtr + 2 * curDataCacheId + 2);
				hid->_y = READ_LE_INT16(curDataPtr + 2 * curDataCacheId + 4);
				hid->_yOffset = READ_LE_INT16(curDataPtr + 2 * curDataCacheId + 8);

				if (!_vm->_globals->_hidingItemData[1]) {
					hid->_useCount = 0;
				} else {
					hid->_spriteData = _vm->_globals->_hidingItemData[1];
					hid->_width = getWidth(_vm->_globals->_hidingItemData[1], curSpriteId);
					hid->_height = getHeight(_vm->_globals->_hidingItemData[1], curSpriteId);
					hid->_useCount = 1;
				}
				if (!hid->_x && !hid->_y && !hid->_spriteIndex)
					hid->_useCount = 0;

				curDataCacheId += 5;
			}
			_vm->_globals->enableHiding();
		}
	}

	_vm->_linesManager->resetLines();
	for (size_t idx = 0; idx < nbytes - 3; idx++) {
		if (READ_BE_UINT24(&ptr[idx]) == MKTAG24('O', 'B', '2')) {
			byte *curDataPtr = &ptr[idx + 4];
			int lineDataIdx = 0;
			int curLineIdx = 0;
			_vm->_linesManager->resetLinesNumb();
			Directions curDirection;
			do {
				curDirection = (Directions)READ_LE_INT16(curDataPtr + 2 * lineDataIdx);
				if (curDirection != DIR_NONE) {
					_vm->_linesManager->addLine(
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
			_vm->_linesManager->initRoute();
		}
	}

	if (!OBSSEUL) {
		for (size_t idx = 0; idx < nbytes - 3; idx++) {
			if (READ_BE_UINT24(&ptr[idx]) == MKTAG24('Z', 'O', '2')) {
				byte *curDataPtr = &ptr[idx + 4];
				int curDataIdx = 0;
				for (int i = 1; i <= 100; i++) {
					_vm->_linesManager->ZONEP[i]._destX = 0;
					_vm->_linesManager->ZONEP[i]._destY = 0;
					_vm->_linesManager->ZONEP[i]._spriteIndex = 0;
					_vm->_linesManager->ZONEP[i]._verbFl1 = 0;
					_vm->_linesManager->ZONEP[i]._verbFl2 = 0;
					_vm->_linesManager->ZONEP[i]._verbFl3 = 0;
					_vm->_linesManager->ZONEP[i]._verbFl4 = 0;
					_vm->_linesManager->ZONEP[i]._verbFl5 = 0;
					_vm->_linesManager->ZONEP[i]._verbFl6 = 0;
					_vm->_linesManager->ZONEP[i]._verbFl7 = 0;
					_vm->_linesManager->ZONEP[i]._verbFl8 = 0;
					_vm->_linesManager->ZONEP[i]._verbFl9 = 0;
					_vm->_linesManager->ZONEP[i]._verbFl10 = 0;
					_vm->_linesManager->ZONEP[i]._messageId = 0;
				}

				int curLineIdx = 0;
				for (;;) {
					int bobZoneId = READ_LE_INT16(curDataPtr + 2 * curDataIdx);
					if (bobZoneId != -1) {
						_vm->_linesManager->addZoneLine(
						    curLineIdx,
						    READ_LE_INT16(curDataPtr + 2 * curDataIdx + 2),
						    READ_LE_INT16(curDataPtr + 2 * curDataIdx + 4),
						    READ_LE_INT16(curDataPtr + 2 * curDataIdx + 6),
						    READ_LE_INT16(curDataPtr + 2 * curDataIdx + 8),
						    bobZoneId);
						_vm->_linesManager->ZONEP[bobZoneId]._enabledFl = true;
					}
					curDataIdx += 5;
					++curLineIdx;
					if (bobZoneId == -1)
						break;
				}
				for (int i = 1; i <= 100; i++) {
					_vm->_linesManager->ZONEP[i]._destX = READ_LE_INT16(curDataPtr + 2 * curDataIdx);
					_vm->_linesManager->ZONEP[i]._destY = READ_LE_INT16(curDataPtr + 2 * curDataIdx + 2);
					_vm->_linesManager->ZONEP[i]._spriteIndex = READ_LE_INT16(curDataPtr + 2 * curDataIdx + 4);
					curDataIdx += 3;
				}

				byte *verbData = ptr + idx + (10 * curLineIdx + 606) + 4;
				for (int i = 1; i <= 100; i++) {
					int j = (i - 1) * 10;
					_vm->_linesManager->ZONEP[i]._verbFl1 = verbData[j];
					_vm->_linesManager->ZONEP[i]._verbFl2 = verbData[j + 1];
					_vm->_linesManager->ZONEP[i]._verbFl3 = verbData[j + 2];
					_vm->_linesManager->ZONEP[i]._verbFl4 = verbData[j + 3];
					_vm->_linesManager->ZONEP[i]._verbFl5 = verbData[j + 4];
					_vm->_linesManager->ZONEP[i]._verbFl6 = verbData[j + 5];
					_vm->_linesManager->ZONEP[i]._verbFl7 = verbData[j + 6];
					_vm->_linesManager->ZONEP[i]._verbFl8 = verbData[j + 7];
					_vm->_linesManager->ZONEP[i]._verbFl9 = verbData[j + 8];
					_vm->_linesManager->ZONEP[i]._verbFl10 = verbData[j + 9];
				}
				int dep = 1010;
				for (int i = 1; i <= 100; i++) {
					_vm->_linesManager->ZONEP[i]._messageId = READ_LE_INT16(verbData + dep);
					dep += 2;
				}
				_vm->_linesManager->CARRE_ZONE();
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
			_vm->_graphicsManager->SETCOLOR3(252, 100, 100, 100);
			_vm->_graphicsManager->SETCOLOR3(253, 100, 100, 100);
			_vm->_graphicsManager->SETCOLOR3(251, 100, 100, 100);
			_vm->_graphicsManager->SETCOLOR3(254, 0, 0, 0);
			for (int i = 0; i <= 4; i++)
				_vm->_eventsManager->refreshScreenAndEvents();
			_vm->_graphicsManager->fadeInLong();
			animateSprite(0);
			for (int i = 0; i <= 4; i++)
				_vm->_eventsManager->refreshScreenAndEvents();
			VBOB(_vm->_globals->SPRITE_ECRAN, 5, 15, 28, 1);
			_vm->_fontManager->hideText(9);
			bool displayedTxtFl = false;
			if (!_vm->_soundManager->_textOffFl) {
				_vm->_fontManager->initTextBuffers(9, 383, _vm->_globals->_textFilename, 220, 72, 6, 36, 253);
				_vm->_fontManager->showText(9);
				displayedTxtFl = true;
			}
			if (!_vm->_soundManager->_voiceOffFl)
				_vm->_soundManager->mixVoice(383, 4, displayedTxtFl);
			_vm->_globals->_saveData->_data[svField270] = 1;
			_vm->_globals->_saveData->_data[svField300] = 1;
			_vm->_globals->_saveData->_data[svField320] = 1;
			if (_vm->_soundManager->_voiceOffFl) {
				for (int i = 0; i <= 199; i++)
					_vm->_eventsManager->refreshScreenAndEvents();
			}
			_vm->_fontManager->hideText(9);
			VBOB_OFF(5);
			for (int i = 0; i <= 3; i++)
				_vm->_eventsManager->refreshScreenAndEvents();
			_vm->_graphicsManager->_noFadingFl = true;
			_vm->_globals->_disableInventFl = false;
		}
		break;

	case 18:
		if (_vm->_globals->_prevScreenId == 17) {
			_vm->_eventsManager->_mouseSpriteId = 4;
			for (int i = 0; i <= 4; i++)
				_vm->_eventsManager->refreshScreenAndEvents();
			_vm->_graphicsManager->fadeInLong();
			_vm->_globals->iRegul = 1;
			_vm->_globals->_disableInventFl = false;
			_vm->_graphicsManager->_noFadingFl = true;
			_vm->_globals->_introSpeechOffFl = true;
			_vm->_talkManager->startAnimatedCharacterDialogue("MAGE1.pe2");
			_vm->_graphicsManager->_noFadingFl = true;
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
		_vm->_linesManager->BOBZONE[20] = 1;
		_vm->_linesManager->BOBZONE[21] = 2;
		_vm->_linesManager->BOBZONE[22] = 3;
		_vm->_linesManager->BOBZONE[23] = 4;
		_vm->_linesManager->BOBZONE_FLAG[20] = true;
		_vm->_linesManager->BOBZONE_FLAG[21] = true;
		_vm->_linesManager->BOBZONE_FLAG[22] = true;
		_vm->_linesManager->BOBZONE_FLAG[23] = true;
		enableVerb(20, 5);
		enableVerb(21, 5);
		enableVerb(22, 5);
		enableVerb(23, 5);
		_vm->_linesManager->ZONEP[20]._messageId = 30;
		_vm->_linesManager->ZONEP[21]._messageId = 30;
		_vm->_linesManager->ZONEP[22]._messageId = 30;
		_vm->_linesManager->ZONEP[23]._messageId = 30;
		for (int i = svField200; i <= svField214; i++) {
			if (_vm->_globals->_saveData->_data[i] != 2)
				_vm->_globals->_saveData->_data[i] = 0;
		}
		break;

	case 73:
		if (!_vm->_globals->_saveData->_data[svSecondElevatorAvailableFl]) {
			_vm->_globals->resetHidingUseCount(0);
			_vm->_globals->resetHidingUseCount(1);
		}
		break;

	case 93:
		if (!_vm->_globals->_saveData->_data[svField333])
			setBobAnimation(8);
		break;
	}
}

void ObjectsManager::OPTI_BOBON(int idx1, int idx2, int idx3, int anim1Idx, int anim2Idx, int anim3Idx) {
	if (idx1 != -1)
		setBobAnimation(idx1);
	if (idx2 != -1)
		setBobAnimation(idx2);
	if (idx3 != -1)
		setBobAnimation(idx3);
	if (idx1 != -1)
		setBobAnimDataIdx(idx1, anim1Idx);
	if (idx2 != -1)
		setBobAnimDataIdx(idx2, anim2Idx);
	if (idx3 != -1)
		setBobAnimDataIdx(idx3, anim3Idx);
}

void ObjectsManager::SCI_OPTI_ONE(int idx, int animIdx, int animDataIdx, int a4) {
	_vm->_eventsManager->_curMouseButton = 0;
	_vm->_eventsManager->_mouseButton = 0;

	if (a4 != 3) {
		setBobAnimation(idx);
		setBobAnimDataIdx(idx, animIdx);
	}

	do {
		_vm->_eventsManager->refreshScreenAndEvents();
		if (_vm->_eventsManager->_curMouseButton)
			break;
	} while (animDataIdx != getBobAnimDataIdx(idx));
	if (!a4)
		stopBobAnimation(idx);
}

void ObjectsManager::disableVerb(int idx, int a2) {
	switch (a2) {
	case 6:
	case 16:
		_vm->_linesManager->ZONEP[idx]._verbFl1 = 0;
		break;
	case 7:
		_vm->_linesManager->ZONEP[idx]._verbFl2 = 0;
		break;
	case 5:
	case 8:
		_vm->_linesManager->ZONEP[idx]._verbFl3 = 0;
		break;
	case 9:
	case 17:
	case 24:
		_vm->_linesManager->ZONEP[idx]._verbFl4 = 0;
		break;
	case 10:
	case 18:
		_vm->_linesManager->ZONEP[idx]._verbFl5 = 0;
		break;
	case 11:
	case 19:
		_vm->_linesManager->ZONEP[idx]._verbFl6 = 0;
		break;
	case 12:
	case 20:
		_vm->_linesManager->ZONEP[idx]._verbFl7 = 0;
		break;
	case 13:
	case 22:
		_vm->_linesManager->ZONEP[idx]._verbFl8 = 0;
	case 14:
	case 21:
	case 25:
		_vm->_linesManager->ZONEP[idx]._verbFl9 = 0;
		break;
	case 15:
		_vm->_linesManager->ZONEP[idx]._verbFl10 = 0;
		break;
	}
	_changeVerbFl = true;
}

void ObjectsManager::enableVerb(int idx, int a2) {
	switch (a2) {
	case 5:
		_vm->_linesManager->ZONEP[idx]._verbFl3 = 2;
		break;
	case 6:
		_vm->_linesManager->ZONEP[idx]._verbFl1 = 1;
		break;
	case 7:
		_vm->_linesManager->ZONEP[idx]._verbFl2 = 1;
		break;
	case 8:
		_vm->_linesManager->ZONEP[idx]._verbFl3 = 1;
		break;
	case 9:
		_vm->_linesManager->ZONEP[idx]._verbFl4 = 1;
		break;
	case 10:
		_vm->_linesManager->ZONEP[idx]._verbFl5 = 1;
		break;
	case 11:
		_vm->_linesManager->ZONEP[idx]._verbFl6 = 1;
		break;
	case 12:
		_vm->_linesManager->ZONEP[idx]._verbFl7 = 1;
		break;
	case 13:
		_vm->_linesManager->ZONEP[idx]._verbFl8 = 1;
		break;
	case 14:
		_vm->_linesManager->ZONEP[idx]._verbFl8 = 1;
		break;
	case 15:
		_vm->_linesManager->ZONEP[idx]._verbFl9 = 1;
		break;
	case 16:
		_vm->_linesManager->ZONEP[idx]._verbFl1 = 2;
		break;
	case 17:
		_vm->_linesManager->ZONEP[idx]._verbFl4 = 2;
		break;
	case 18:
		_vm->_linesManager->ZONEP[idx]._verbFl5 = 2;
		break;
	case 19:
		_vm->_linesManager->ZONEP[idx]._verbFl6 = 2;
		break;
	case 20:
		_vm->_linesManager->ZONEP[idx]._verbFl7 = 2;
		break;
	case 21:
		_vm->_linesManager->ZONEP[idx]._verbFl9 = 2;
		break;
	case 22:
		_vm->_linesManager->ZONEP[idx]._verbFl8 = 2;
		break;
	case 24:
		_vm->_linesManager->ZONEP[idx]._verbFl4 = 3;
		break;
	case 25:
		_vm->_linesManager->ZONEP[idx]._verbFl9 = 2;
		break;
	}
}

void ObjectsManager::ACTION(const byte *spriteData, const Common::String &actionStr, int speed, bool flipFl) {
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
				_vm->_eventsManager->refreshScreenAndEvents();
			if (idx == -1)
				break;
		}
	}
}

void ObjectsManager::SPACTION(byte *spriteData, const Common::String &animationSeq, int speed, bool flipFl) {
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
			_vm->_eventsManager->refreshScreenAndEvents();
	} while (spriteIndex != -1);
}

void ObjectsManager::SPACTION1(byte *spriteData, const Common::String &animString, int speed) {
	Common::String tmpStr = "";
	int realSpeed = speed;
	if (_vm->_globals->_speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals->_speed == 3)
		realSpeed = speed / 3;

	int spriteIndex = 0;
	bool completeTokenFl;
	char nextChar;

	for (int idx = 0; ; idx++) {
		completeTokenFl = false;
		nextChar = animString[idx];
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
				_vm->_eventsManager->refreshScreenAndEvents();

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
	}

	if (_vm->_globals->_saveData->_data[savegameIdx] == 2)
		return;

	if (_vm->_globals->_saveData->_data[savegameIdx]) {
		if (_vm->_globals->_saveData->_data[savegameIdx] == 1) {
			if (((idx == 1 || idx == 2) && getBobAnimDataIdx(idx) == 26) || ((idx == 3 || idx == 4) && getBobAnimDataIdx(idx) == 27)) {
				_vm->_dialogsManager->disableInvent();
				_vm->_soundManager->playSample(1);
				_vm->_globals->_saveData->_data[savegameIdx] = 4;
			}
		}
		if (_vm->_globals->_saveData->_data[savegameIdx] == 4) {
			if (idx >= 1 && idx <= 4 && getBobAnimDataIdx(idx) > 30)
				_vm->_globals->_saveData->_data[savegameIdx] = 3;
		}
		if (_vm->_globals->_saveData->_data[savegameIdx] == 3) {
			_vm->_graphicsManager->FADE_LINUX = 2;
			_vm->_animationManager->playAnim("CREVE2.ANM", 100, 24, 500);
			_vm->_globals->_exitId = 150;
			_vm->_graphicsManager->_noFadingFl = true;
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
	_vm->_globals->_lockedAnims[idx]._enableFl = true;
	_vm->_globals->_lockedAnims[idx]._posX = x;
}

/**
 * Game scene control method
 */
void ObjectsManager::PERSONAGE(const Common::String &backgroundFile, const Common::String &linkFile,
							   const Common::String &animFile, const Common::String &s4, int soundNum, bool initializeScreen) {
	_vm->_dialogsManager->_inventFl = false;
	_vm->_eventsManager->_gameKey = KEY_NONE;
	_vm->_dialogsManager->enableInvent();
	_vm->_graphicsManager->_scrollOffset = 0;
	_vm->_globals->_cityMapEnabledFl = false;
	_vm->_globals->iRegul = 1;
	_vm->_soundManager->playSound(soundNum);
	_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
	_vm->_globals->_freezeCharacterFl = true;
	_vm->_globals->_exitId = 0;
	if (!backgroundFile.empty())
		_vm->_graphicsManager->loadImage(backgroundFile);
	if (!linkFile.empty())
		loadLinkFile(linkFile);
	if (!animFile.empty())
		_vm->_animationManager->loadAnim(animFile);
	_vm->_graphicsManager->displayAllBob();
	if (!s4.empty()) {
		if (initializeScreen)
			_vm->_graphicsManager->initScreen(s4, 0, initializeScreen);
		else
			_vm->_graphicsManager->initScreen(s4, 2, initializeScreen);
	}
	_vm->_eventsManager->mouseOn();
	if (_vm->_globals->_screenId == 61) {
		addStaticSprite(_vm->_globals->PERSO, Common::Point(330, 418), 0, 60, 0, false, 34, 190);
		animateSprite(0);
		_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
		computeAndSetSpriteSize();
	}
	_vm->_graphicsManager->SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager->SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager->SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager->SETCOLOR3(254, 0, 0, 0);
	_vm->_eventsManager->changeMouseCursor(4);
	for (int i = 0; i <= 4; i++)
		_vm->_eventsManager->refreshScreenAndEvents();
	_vm->_graphicsManager->fadeInLong();
	if (_vm->_globals->_screenId == 61) {
		_vm->_animationManager->playSequence("OUVRE.SEQ", 10, 4, 10, false, false);
		stopBobAnimation(3);
		_vm->_globals->_checkDistanceFl = true;
		_oldCharacterPosX = getSpriteX(0);
		_vm->_globals->_oldDirection = DIR_NONE;
		_homeRateCounter = 0;
		_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
		_vm->_linesManager->_route = _vm->_linesManager->PARCOURS2(getSpriteX(0), getSpriteY(0), 330, 345);
		_vm->_globals->_checkDistanceFl = true;
		do {
			GOHOME();
			_vm->_eventsManager->refreshScreenAndEvents();
		} while (_vm->_linesManager->_route != (RouteItem *)g_PTRNUL);
		setSpriteIndex(0, 64);
	}
	do {
		int mouseButton = _vm->_eventsManager->getMouseButton();
		if (mouseButton == 1) {
			handleLeftButton();
			mouseButton = 1;
		} else if (mouseButton == 2)
			handleRightButton();
		_vm->_dialogsManager->testDialogOpening();
		_vm->_linesManager->checkZone();
		if (_vm->_globals->_actionMoveTo)
			PARADISE();
		if (!_vm->_globals->_exitId)
			_vm->_eventsManager->refreshScreenAndEvents();

		if (_vm->_globals->_exitId)
			break;
	} while (!_vm->shouldQuit());
	if (_vm->shouldQuit())
		return;

	_vm->_graphicsManager->fadeOutLong();
	if (!animFile.empty())
		_vm->_graphicsManager->endDisplayBob();
	if (_vm->_globals->_screenId == 61)
		removeSprite(0);
	clearScreen();
	_vm->_globals->iRegul = 0;
}

/**
 * Game scene control method
 */
void ObjectsManager::PERSONAGE2(const Common::String &backgroundFile, const Common::String &linkFile,
								const Common::String &animFile, const Common::String &s4, int soundNum, bool initializeScreen) {
	_vm->_dialogsManager->_inventFl = false;
	_vm->_eventsManager->_gameKey = KEY_NONE;
	_verb = 4;
	_vm->_graphicsManager->_scrollOffset = 0;
	_vm->_dialogsManager->enableInvent();
	_vm->_globals->_cityMapEnabledFl = false;
	_vm->_graphicsManager->_noFadingFl = false;
	_vm->_globals->_freezeCharacterFl = false;
	_vm->_globals->_exitId = 0;
	_vm->_globals->_checkDistanceFl = true;
	_vm->_soundManager->playSound(soundNum);
	_vm->_globals->iRegul = 1;
	if (!backgroundFile.empty())
		_vm->_graphicsManager->loadImage(backgroundFile);
	if (!linkFile.empty())
		loadLinkFile(linkFile);
	if (!animFile.empty()) {
		_vm->_animationManager->loadAnim(animFile);
		_vm->_graphicsManager->displayAllBob();
	}
	if (!s4.empty()) {
		if (initializeScreen)
			_vm->_graphicsManager->initScreen(s4, 0, initializeScreen);
		else
			_vm->_graphicsManager->initScreen(s4, 2, initializeScreen);
	}
	_vm->_eventsManager->mouseOn();
	_vm->_eventsManager->_mouseCursorId = 4;
	_vm->_graphicsManager->SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager->SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager->SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager->SETCOLOR3(254, 0, 0, 0);
	if (_vm->_globals->_characterType) {
		if (!_vm->_globals->_saveData->_data[svAlternateSpriteFl] && !_vm->_globals->_saveData->_data[svField356]) {
			_vm->_globals->PERSO = _vm->_fileManager->loadFile("PERSO.SPR");
			_vm->_globals->_characterType = 0;
		}
	}
	if (!_vm->_globals->_characterType && _vm->_globals->_saveData->_data[svAlternateSpriteFl] == 1) {
		_vm->_globals->PERSO = _vm->_fileManager->loadFile("HOPFEM.SPR");
		_vm->_globals->_characterType = 1;
	}

	if (_vm->_globals->_characterType != 2 && _vm->_globals->_saveData->_data[svField356] == 1) {
		_vm->_globals->PERSO = _vm->_fileManager->loadFile("PSAMAN.SPR");
		_vm->_globals->_characterType = 2;
	}
	_vm->_globals->loadCharacterData();
	switch (_vm->_globals->_characterType) {
	case 0:
		addStaticSprite(_vm->_globals->PERSO, _characterPos, 0, _startSpriteIndex, 0, false, 34, 190);
		break;
	case 1:
		addStaticSprite(_vm->_globals->PERSO, _characterPos, 0, _startSpriteIndex, 0, false, 28, 155);
		break;
	case 2:
		addStaticSprite(_vm->_globals->PERSO, _characterPos, 0, _startSpriteIndex, 0, false, 20, 127);
		break;
	}
	_vm->_eventsManager->setMouseXY(_characterPos);
	if (_vm->_graphicsManager->_largeScreenFl)
		_vm->_graphicsManager->_scrollPosX = (int16)getSpriteX(0) - 320;
	computeAndSetSpriteSize();
	animateSprite(0);
	_vm->_globals->enableHiding();
	_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
	computeAndSetSpriteSize();
	sceneSpecialIni();
	_vm->_eventsManager->_mouseSpriteId = 4;
	_oldCharacterPosX = _characterPos.x;
	_oldCharacterPosY = _characterPos.y;
	_vm->_globals->_oldDirection = DIR_NONE;
	_homeRateCounter = 0;

	for (int idx = 0; idx < 5; ++idx)
		_vm->_eventsManager->refreshScreenAndEvents();

	_vm->_globals->iRegul = 1;
	if (!_vm->_graphicsManager->_noFadingFl)
		_vm->_graphicsManager->fadeInLong();
	_vm->_graphicsManager->_noFadingFl = false;
	_vm->_eventsManager->changeMouseCursor(4);

	int xCheck = 0;
	int yCheck = 0;

	bool breakFlag = false;
	while (!_vm->shouldQuit() && !breakFlag) {
		int mouseButtons = _vm->_eventsManager->getMouseButton();
		if (mouseButtons) {
			if (mouseButtons == 1) {
				if (_verb == 16 && _vm->_eventsManager->_mouseCursorId == 16) {
					int xp = _vm->_eventsManager->getMouseX();
					int yp = _vm->_eventsManager->getMouseY();

					if ((xCheck == xp) && (yCheck == yp)) {
						_vm->_linesManager->_route = (RouteItem *)g_PTRNUL;
						PARADISE();
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
			_vm->_dialogsManager->testDialogOpening();
			_vm->_linesManager->checkZone();
			if (_vm->_linesManager->_route == (RouteItem *)g_PTRNUL
					|| (GOHOME(), _vm->_linesManager->_route == (RouteItem *)g_PTRNUL)) {
				if (_vm->_globals->_actionMoveTo)
					PARADISE();
			}
			handleSpecialGames();
			_vm->_eventsManager->refreshScreenAndEvents();
			if (!_vm->_globals->_exitId)
				continue;
		}
		breakFlag = true;
	}

	if (_vm->_globals->_exitId != 8 || _vm->_globals->_screenId != 5 || !_helicopterFl) {
		if (!_vm->_graphicsManager->_noFadingFl)
			_vm->_graphicsManager->fadeOutLong();
		_vm->_graphicsManager->_noFadingFl = false;
		removeSprite(0);
		if (_twoCharactersFl) {
			removeSprite(1);
			_twoCharactersFl = false;
		}
		if (!animFile.empty())
			_vm->_graphicsManager->endDisplayBob();
		clearScreen();
	} else {
		_helicopterFl = false;
	}
	_vm->_globals->iRegul = 0;
}

void ObjectsManager::setVerb(int id) {
	_verb = id;
}
} // End of namespace Hopkins
