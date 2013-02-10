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

#include "common/system.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"
#include "hopkins/objects.h"
#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

ObjectsManager::ObjectsManager() {
	for (int i = 0; i < 6; ++i) {
		Common::fill((byte *)&_sprite[i], (byte *)&_sprite[i] + sizeof(SpriteItem), 0);
	}
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
}

ObjectsManager::~ObjectsManager() {
	_vm->_globals.freeMemory(_forestSprite);
}

void ObjectsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void ObjectsManager::clearAll() {
	_forestFl = false;
	_forestSprite = g_PTRNUL;
}

/**
 * Change Object
 */
void ObjectsManager::changeObject(int objIndex) {
	_vm->_eventsManager._objectBuf = CAPTURE_OBJET(objIndex, true);
	_curObjectIndex = objIndex;
}

byte *ObjectsManager::CAPTURE_OBJET(int objIndex, bool mode) {
	byte *dataP = NULL;
	int val1 = _vm->_globals.ObjetW[objIndex].field0;
	int val2 = _vm->_globals.ObjetW[objIndex]._idx;

	if (mode)
		++val2;

	if (val1 != _vm->_globals.NUM_FICHIER_OBJ) {
		if (_vm->_globals.ADR_FICHIER_OBJ != g_PTRNUL)
			ObjectsManager::DEL_FICHIER_OBJ();
		if (val1 == 1) {
			_vm->_globals.ADR_FICHIER_OBJ = ObjectsManager::loadSprite("OBJET1.SPR");
		}
		_vm->_globals.NUM_FICHIER_OBJ = val1;
	}

	int width = ObjectsManager::getWidth(_vm->_globals.ADR_FICHIER_OBJ, val2);
	int height = ObjectsManager::getHeight(_vm->_globals.ADR_FICHIER_OBJ, val2);
	_vm->_globals._objectWidth = width;
	_vm->_globals._objectHeight = height;

	if (mode) {
		sprite_alone(_vm->_globals.ADR_FICHIER_OBJ, _vm->_eventsManager._objectBuf, val2);
		dataP = _vm->_eventsManager._objectBuf;
	} else { 
		dataP = _vm->_globals.allocMemory(height * width);
		if (dataP == g_PTRNUL)
			error("CAPTURE_OBJET");

		capture_mem_sprite(_vm->_globals.ADR_FICHIER_OBJ, dataP, val2);
	}

	return dataP;
}

/**
 * Remove an Object from the inventory
 */
void ObjectsManager::removeObject(int objIndex) {
	int idx;
	for (idx = 1; idx <= 32; ++idx) {
		if (_vm->_globals._inventory[idx] == objIndex)
			break;
	}

	if (idx <= 32) {
		if (idx == 32) {
			_vm->_globals._inventory[32] = 0;
		} else {
			for (int i = idx; i < 32; ++i)
				_vm->_globals._inventory[i] = _vm->_globals._inventory[i + 1];
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
	const byte *v3 = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		v3 += READ_LE_UINT32(v3) + 16;

	const byte *v5 = v3 + 8;
	int result = (int16)READ_LE_UINT16(v5);
	if (isSize)
		result = (int16)READ_LE_UINT16(v5 + 4);

	return result;
}

int ObjectsManager::getOffsetY(const byte *spriteData, int spriteIndex, bool isSize) {
	const byte *v3 = spriteData + 3;
	for (int i = spriteIndex; i; --i)
		v3 += READ_LE_UINT32(v3) + 16;

	const byte *v5 = v3 + 10;
	int result = (int16)READ_LE_UINT16(v5);
	if (isSize)
		result = (int16)READ_LE_UINT16(v5 + 4);

	return result;
}

/**
 * Get Width
 */
int ObjectsManager::getWidth(const byte *objectData, int idx) {
	const byte *rectP = objectData + 3;
	for (int i = idx; i; --i)
		rectP += READ_LE_UINT32(rectP) + 16;

	return (int16)READ_LE_UINT16(rectP + 4);
}

/**
 * Get height
 */
int ObjectsManager::getHeight(const byte *objectData, int idx) {
	const byte *rectP = objectData + 3;
	for (int i = idx; i; --i)
		rectP += READ_LE_UINT32(rectP) + 16;

	return (int16)READ_LE_UINT16(rectP + 6);
}

void ObjectsManager::sprite_alone(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = (int16)READ_LE_UINT16(objP) * (int16)READ_LE_UINT16(objP + 2);

	memcpy(sprite + 3, objP - 4, result + 16);
}

void ObjectsManager::capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = (int16)READ_LE_UINT16(objP) * (int16)READ_LE_UINT16(objP + 2);
	memcpy(sprite, objP + 12, result);
}

void ObjectsManager::DEL_FICHIER_OBJ() {
	_vm->_globals.NUM_FICHIER_OBJ = 0;
	_vm->_globals.ADR_FICHIER_OBJ = _vm->_globals.freeMemory(_vm->_globals.ADR_FICHIER_OBJ);
}

/**
 * Load Sprite from file
 */
byte *ObjectsManager::loadSprite(const Common::String &file) {
	return _vm->_fileManager.loadFile(file);
}

/**
 * Add Object
 */
void ObjectsManager::addObject(int objIndex) {
	int arrIndex = 0;
	for (;;) {
		++arrIndex;
		if ((!_vm->_globals._inventory[arrIndex]) || (arrIndex == 32))
			break;;
	}

	_vm->_globals._inventory[arrIndex] = objIndex;
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
	_vm->_globals._sortedDisplayCount = 0;
	for (int idx = 0; idx <= 10; ++idx) {
		if (_vm->_fontManager._textList[idx]._enabledFl && _vm->_fontManager._text[idx]._textType != 2) {
			clipX = _vm->_fontManager._textList[idx]._pos.x - 2;

			if (clipX < _vm->_graphicsManager._minX)
				clipX = _vm->_graphicsManager._minX;
	
			clipY = _vm->_fontManager._textList[idx]._pos.y - 2;
			if (clipY < _vm->_graphicsManager._minY)
				clipY = _vm->_graphicsManager._minY;

			_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen, clipX, clipY,
				_vm->_fontManager._textList[idx]._width + 4, _vm->_fontManager._textList[idx]._height + 4,
				_vm->_graphicsManager._vesaBuffer, clipX, clipY);
			_vm->_fontManager._textList[idx]._enabledFl = false;
		}
	}

	if (!PERSO_ON) {
		for (int idx = 0; idx < MAX_SPRITE; ++idx) {
			if (_vm->_globals.Liste[idx]._visibleFl) {
				clipX = _vm->_globals.Liste[idx]._posX - 2;
				if (clipX < _vm->_graphicsManager._minX)
					clipX = _vm->_graphicsManager._minX;

				clipY = _vm->_globals.Liste[idx]._posY - 2;
				if (clipY < _vm->_graphicsManager._minY)
					clipY = _vm->_graphicsManager._minY;

				_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen, clipX, clipY,
					_vm->_globals.Liste[idx]._width + 4, _vm->_globals.Liste[idx]._height + 4,
					_vm->_graphicsManager._vesaBuffer, clipX, clipY);
				_vm->_globals.Liste[idx]._visibleFl = false;
			}
		}
	}

	displayBobAnim();
	displayVBob();

	if (!PERSO_ON) {
		// Handle drawing characters on the screen
		for (int idx = 0; idx < MAX_SPRITE; ++idx) {
			_vm->_globals.Liste[idx]._visibleFl = false;
			if (_sprite[idx]._animationType == 1) {
				computeSprite(idx);
				if (_sprite[idx].field2A)
					beforeSort(SORT_SPRITE, idx, _sprite[idx]._height + _sprite[idx]._destY);
			}
		}

		if (_vm->_globals._cacheFl)
			checkCache();
	}

	if (_priorityFl && _vm->_globals._sortedDisplayCount) {
		for (int i = 1; i <= 48; i++) 
			arr[i] = i;

		do {
			loopCondFl = false;
			for (int sortIdx = 1; sortIdx < _vm->_globals._sortedDisplayCount; sortIdx++) {
				if (_vm->_globals._sortedDisplay[arr[sortIdx]]._priority > _vm->_globals._sortedDisplay[arr[sortIdx + 1]]._priority) {
					SWAP(arr[sortIdx], arr[sortIdx + 1]);
					loopCondFl = true;
				}
			}
		} while (loopCondFl);

		for (int sortIdx = 1; sortIdx < _vm->_globals._sortedDisplayCount + 1; sortIdx++) {
			int idx = arr[sortIdx];
			switch (_vm->_globals._sortedDisplay[idx]._sortMode) {
			case SORT_BOB:
				setBobInfo(_vm->_globals._sortedDisplay[idx]._index);
				break;
			case SORT_SPRITE:
				DEF_SPRITE(_vm->_globals._sortedDisplay[idx]._index);
				break;
			case SORT_CACHE:
				displayCache(_vm->_globals._sortedDisplay[idx]._index);
				break;
			default:
				break;
			}
			_vm->_globals._sortedDisplay[idx]._sortMode = SORT_NONE;
		}
	} else {
		for (int idx = 1; idx < (_vm->_globals._sortedDisplayCount + 1); ++idx) {
			switch (_vm->_globals._sortedDisplay[idx]._sortMode) {
			case SORT_BOB:
				setBobInfo(_vm->_globals._sortedDisplay[idx]._index);
				break;
			case SORT_SPRITE:
				DEF_SPRITE(_vm->_globals._sortedDisplay[idx]._index);
				break;
			case SORT_CACHE:
				displayCache(_vm->_globals._sortedDisplay[idx]._index);
				break;
			default:
				break;
			}
			_vm->_globals._sortedDisplay[idx]._sortMode = SORT_NONE;
		}
	}

	// Reset the Sort array
	for (int idx = 0; idx < 50; ++idx) {
		_vm->_globals._sortedDisplay[idx]._sortMode = SORT_NONE;
		_vm->_globals._sortedDisplay[idx]._index = 0;
		_vm->_globals._sortedDisplay[idx]._priority = 0;
	}

	_vm->_globals._sortedDisplayCount = 0;
	if (_vm->_dialogsManager._inventDisplayedFl) {
		_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, _vm->_dialogsManager._inventWin1, _vm->_dialogsManager._inventX, _vm->_dialogsManager._inventY, _vm->_dialogsManager._inventWidth, _vm->_dialogsManager._inventHeight);
		if (_oldBorderPos.x && _oldBorderPos.y)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_dialogsManager._inventBuf2, _oldBorderPos.x + 300, _oldBorderPos.y + 300, _oldBorderSpriteIndex + 1);
		if (_borderPos.x && _borderPos.y)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_dialogsManager._inventBuf2, _borderPos.x + 300, _borderPos.y + 300, _borderSpriteIndex);
		_vm->_graphicsManager.addVesaSegment(_vm->_dialogsManager._inventX, _vm->_dialogsManager._inventY, _vm->_dialogsManager._inventX + _vm->_dialogsManager._inventWidth, _vm->_dialogsManager._inventY + _vm->_dialogsManager._inventHeight);
	}

	if (_saveLoadFl) {
		_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, _saveLoadSprite, _vm->_eventsManager._startPos.x + 183, 60, 274, 353);
		if (_saveLoadX && _saveLoadY)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _saveLoadSprite2, _saveLoadX + _vm->_eventsManager._startPos.x + 300, _saveLoadY + 300, 0);

		_vm->_graphicsManager.addVesaSegment(_vm->_eventsManager._startPos.x + 183, 60, _vm->_eventsManager._startPos.x + 457, 413);
	}

	// If the Options dialog is activated, draw the elements
	if (_vm->_globals._optionDialogFl) {
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals._optionDialogSpr,
			_vm->_eventsManager._startPos.x + 464, 407, 0);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals._optionDialogSpr,
			_vm->_eventsManager._startPos.x + 657, 556, _vm->_globals._menuSpeed);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals._optionDialogSpr,
			_vm->_eventsManager._startPos.x + 731, 495, _vm->_globals._menuTextOff);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals._optionDialogSpr,
			_vm->_eventsManager._startPos.x + 731, 468, _vm->_globals._menuVoiceOff);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals._optionDialogSpr,
			_vm->_eventsManager._startPos.x + 731, 441, _vm->_globals._menuSoundOff);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals._optionDialogSpr,
			_vm->_eventsManager._startPos.x + 731, 414, _vm->_globals._menuMusicOff);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals._optionDialogSpr,
			_vm->_eventsManager._startPos.x + 600, 522, _vm->_globals._menuDisplayType);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals._optionDialogSpr,
			_vm->_eventsManager._startPos.x + 611, 502, _vm->_globals._menuScrollSpeed);
		_vm->_graphicsManager.addVesaSegment(_vm->_eventsManager._startPos.x + 164, 107, _vm->_eventsManager._startPos.x + 498, 320);
	}

	// Loop to draw any on-screen text
	for (int idx = 0; idx <= 10; ++idx) {
		if (_vm->_fontManager._text[idx]._textOnFl) {
			if ((_vm->_fontManager._text[idx]._textType < 2) || (_vm->_fontManager._text[idx]._textType > 3))
				_vm->_fontManager.box(idx,
					_vm->_fontManager._text[idx]._messageId, _vm->_fontManager._text[idx]._filename,
					_vm->_eventsManager._startPos.x + _vm->_fontManager._text[idx]._pos.x, _vm->_fontManager._text[idx]._pos.y);
			else
				_vm->_fontManager.box(idx,
					_vm->_fontManager._text[idx]._messageId, _vm->_fontManager._text[idx]._filename,
					_vm->_fontManager._text[idx]._pos.x, _vm->_fontManager._text[idx]._pos.y);
			_vm->_fontManager._textList[idx]._enabledFl = true;

			if ((_vm->_fontManager._text[idx]._textType < 2) || (_vm->_fontManager._text[idx]._textType > 3))
				_vm->_fontManager._textList[idx]._pos.x = _vm->_eventsManager._startPos.x + _vm->_fontManager._text[idx]._pos.x;
			else
				_vm->_fontManager._textList[idx]._pos.x = _vm->_fontManager._text[idx]._pos.x;

			_vm->_fontManager._textList[idx]._pos.y = _vm->_fontManager._text[idx]._pos.y;
			_vm->_fontManager._textList[idx]._width = _vm->_fontManager._text[idx]._width;
			_vm->_fontManager._textList[idx]._height = _vm->_fontManager._text[idx]._height;

			if (_vm->_fontManager._textList[idx]._pos.x < _vm->_graphicsManager._minX)
				_vm->_fontManager._textList[idx]._pos.x = _vm->_graphicsManager._minX - 1;
			if (_vm->_fontManager._textList[idx]._pos.y < _vm->_graphicsManager._minY)
				_vm->_fontManager._textList[idx]._pos.y = _vm->_graphicsManager._minY - 1;

			int posX = _vm->_fontManager._textList[idx]._pos.x;
			if (_vm->_fontManager._textList[idx]._width + posX > _vm->_graphicsManager._maxX)
				_vm->_fontManager._textList[idx]._width = _vm->_graphicsManager._maxX - posX;
			int posY = _vm->_fontManager._textList[idx]._pos.y;
			if (_vm->_fontManager._textList[idx]._height + posY > _vm->_graphicsManager._maxY)
				_vm->_fontManager._textList[idx]._height = _vm->_graphicsManager._maxY - posY;
			if (_vm->_fontManager._textList[idx]._width <= 0 || _vm->_fontManager._textList[idx]._height <= 0)
				_vm->_fontManager._textList[idx]._enabledFl = false;
		}
	}

	_vm->_dialogsManager.inventAnim();
}

void ObjectsManager::initBob() {
	for (int idx = 0; idx < 35; ++idx)
		resetBob(idx);
}

void ObjectsManager::resetBob(int idx) {
	BobItem &bob = _bob[idx];
	ListeItem &item = _vm->_globals.Liste2[idx];

	bob.field0 = 0;
	bob._spriteData = g_PTRNUL;
	bob._xp = 0;
	bob._yp = 0;
	bob._frameIndex = 0;
	bob._animDataIdx = 0;
	bob.field12 = 0;
	bob.field14 = 0;
	bob._disabledAnimationFl = false;
	bob._animData = g_PTRNUL;
	bob.field1C = false;
	bob.field1E = 0;
	bob.field20 = 0;
	bob.field22 = 0;
	bob._offsetY = 0;
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
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _bob[idx]._spriteData,
			xp + 300, yp + 300, _bob[idx]._frameIndex);
	else
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer,
			_bob[idx]._spriteData, xp + 300, yp + 300, _bob[idx]._frameIndex,
			_bob[idx]._zoomOutFactor, _bob[idx]._zooInmFactor,
			_bob[idx]._flipFl);

	_vm->_globals.Liste2[idx]._visibleFl = true;
	_vm->_globals.Liste2[idx]._posX = xp;
	_vm->_globals.Liste2[idx]._posY = yp;

	_vm->_globals.Liste2[idx]._width = _bob[idx]._oldWidth;
	_vm->_globals.Liste2[idx]._height = _bob[idx]._oldHeight;

	if (_vm->_globals.Liste2[idx]._posX < _vm->_graphicsManager._minX) {
		_vm->_globals.Liste2[idx]._width -= _vm->_graphicsManager._minX - _vm->_globals.Liste2[idx]._posX;
		_vm->_globals.Liste2[idx]._posX = _vm->_graphicsManager._minX;
	}

	if (_vm->_globals.Liste2[idx]._posY < _vm->_graphicsManager._minY) {
		_vm->_globals.Liste2[idx]._height -= _vm->_graphicsManager._minY - _vm->_globals.Liste2[idx]._posY;
		_vm->_globals.Liste2[idx]._posY = _vm->_graphicsManager._minY;
	}

	if (_vm->_globals.Liste2[idx]._width + _vm->_globals.Liste2[idx]._posX > _vm->_graphicsManager._maxX)
		_vm->_globals.Liste2[idx]._width = _vm->_graphicsManager._maxX - _vm->_globals.Liste2[idx]._posX;

	if (_vm->_globals.Liste2[idx]._height + _vm->_globals.Liste2[idx]._posY > _vm->_graphicsManager._maxY)
		_vm->_globals.Liste2[idx]._height = _vm->_graphicsManager._maxY - _vm->_globals.Liste2[idx]._posY;

	if (_vm->_globals.Liste2[idx]._width <= 0 || _vm->_globals.Liste2[idx]._height <= 0)
		_vm->_globals.Liste2[idx]._visibleFl = false;

	if (_vm->_globals.Liste2[idx]._visibleFl)
		_vm->_graphicsManager.addVesaSegment(
             _vm->_globals.Liste2[idx]._posX,
             _vm->_globals.Liste2[idx]._posY,
             _vm->_globals.Liste2[idx]._posX + _vm->_globals.Liste2[idx]._width,
             _vm->_globals.Liste2[idx]._posY + _vm->_globals.Liste2[idx]._height);
}

void ObjectsManager::BOB_VISU(int idx) {
	_priorityFl = true;

	if (_bob[idx].field0)
		return;

	resetBob(idx);

	const byte *data = _vm->_globals.Bqe_Anim[idx]._data;
	int bankIdx = (int16)READ_LE_UINT16(data);
	if (!bankIdx)
		return;
	if ((!_vm->_globals.Bank[bankIdx]._loadedFl) || (!READ_LE_UINT16(data + 24)))
		return;


	int16 v9 = (int16)READ_LE_UINT16(data + 2);
	int16 v8 = (int16)READ_LE_UINT16(data + 4);
	int16 offsetY = (int16)READ_LE_UINT16(data + 6);
	int16 v6 = (int16)READ_LE_UINT16(data + 8);

	if (!v9)
		v9 = 1;
	if (!v6)
		v6 = -1;

	_bob[idx]._isSpriteFl = false;

	if (_vm->_globals.Bank[bankIdx]._fileHeader == 1) {
		_bob[idx]._isSpriteFl = true;
		_bob[idx]._zoomFactor = 0;
		_bob[idx]._flipFl = false;
	}

	_bob[idx]._animData = _vm->_globals.Bqe_Anim[idx]._data;
	_bob[idx].field0 = 10;
	_bob[idx]._spriteData = _vm->_globals.Bank[bankIdx]._data;

	_bob[idx].field1E = v9;
	_bob[idx].field20 = v6;
	_bob[idx].field22 = v8;
	_bob[idx]._offsetY = offsetY;
}

void ObjectsManager::BOB_OFF(int idx) {
	if ((_bob[idx].field0 == 3) || (_bob[idx].field0 == 10))
		_bob[idx].field0++;
}

void ObjectsManager::BOB_OFFSET(int idx, int offset) {
	_bob[idx]._oldX2 = offset;
}

void ObjectsManager::SCBOB(int idx) {
	if (_vm->_globals.Cache[idx]._useCount == 0)
		return;

	for (int i = 0; i <= 20; i++) {
		if ((_bob[i].field0) && (!_bob[i]._disabledAnimationFl) && (!_bob[i].field34) && (_bob[i]._frameIndex != 250)) {
			int oldRight = _bob[i]._oldX + _bob[i]._oldWidth;
			int oldBottom = _bob[i]._oldY + _bob[i]._oldHeight;
			int cachedRight = _vm->_globals.Cache[idx]._width + _vm->_globals.Cache[idx]._x;

			if ((oldBottom > _vm->_globals.Cache[idx]._y) && (oldBottom < _vm->_globals.Cache[idx].field14 +_vm->_globals.Cache[idx]._height + _vm->_globals.Cache[idx]._y)) {
				if ((oldRight >= _vm->_globals.Cache[idx]._x && oldRight <= cachedRight)
				 || (cachedRight >= _bob[i]._oldWidth && _bob[i]._oldWidth >= _vm->_globals.Cache[idx]._x)
				 || (cachedRight >= _bob[i]._oldWidth && _bob[i]._oldWidth >= _vm->_globals.Cache[idx]._x)
				 || (_bob[i]._oldWidth >= _vm->_globals.Cache[idx]._x && oldRight <= cachedRight)
				 || (_bob[i]._oldWidth <= _vm->_globals.Cache[idx]._x && oldRight >= cachedRight))
					++_vm->_globals.Cache[idx]._useCount;
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
			deltaX = _vm->_graphicsManager.zoomIn(deltaX, posZoom);
		else
			deltaX = -_vm->_graphicsManager.zoomIn(-deltaX, posZoom);

		if (deltaY >= 0)
			deltaY = _vm->_graphicsManager.zoomIn(deltaY, posZoom);
		else
			deltaY = -_vm->_graphicsManager.zoomIn(abs(deltaX), posZoom);
	}

	if (negZoom) {
		if (deltaX >= 0)
			deltaX = _vm->_graphicsManager.zoomOut(deltaX, negZoom);
		else
			deltaX = -_vm->_graphicsManager.zoomOut(-deltaX, negZoom);

		if (deltaY >= 0)
			deltaY = _vm->_graphicsManager.zoomOut(deltaY, negZoom);
		else
			deltaY = -_vm->_graphicsManager.zoomOut(abs(deltaX), negZoom);
	}

	int newX = _bob[idx]._xp - deltaX;
	int newY = _bob[idx]._yp - deltaY;
	_bob[idx]._activeFl = true;
	_bob[idx]._oldX = newX;
	_bob[idx]._oldY = newY;
	_bob[idx]._zooInmFactor = posZoom;
	_bob[idx]._zoomOutFactor = negZoom;

	_vm->_globals.Liste2[idx]._visibleFl = true;
	_vm->_globals.Liste2[idx]._posX = newX;
	_vm->_globals.Liste2[idx]._posY = newY;

	int width = getWidth(_bob[idx]._spriteData, _bob[idx]._frameIndex);
	int height = getHeight(_bob[idx]._spriteData, _bob[idx]._frameIndex);

	if (posZoom) {
		width = _vm->_graphicsManager.zoomIn(width, posZoom);
		height = _vm->_graphicsManager.zoomIn(height, posZoom);
	}
	if (negZoom) {
		height = _vm->_graphicsManager.zoomOut(height, negZoom);
		width = _vm->_graphicsManager.zoomOut(width, negZoom);
	}

	_vm->_globals.Liste2[idx]._width = width;
	_vm->_globals.Liste2[idx]._height = height;
	_bob[idx]._oldWidth = width;
	_bob[idx]._oldHeight = height;
}

void ObjectsManager::checkCache() {
	for (int cacheIdx = 0; cacheIdx <= 19; cacheIdx++) {
		if (_vm->_globals.Cache[cacheIdx]._useCount == 0)
			continue;

		int _oldUseCount = _vm->_globals.Cache[cacheIdx]._useCount;
		for (int spriteIdx = 0; spriteIdx <= 4; spriteIdx++) {
			if (_sprite[spriteIdx]._animationType == 1 && _sprite[spriteIdx]._spriteIndex != 250) {
				int right = _sprite[spriteIdx]._width + _sprite[spriteIdx]._destX;
				int bottom = _sprite[spriteIdx]._height + _sprite[spriteIdx]._destY;
				int cachedRight = _vm->_globals.Cache[cacheIdx]._width + _vm->_globals.Cache[cacheIdx]._x;

				if (bottom > _vm->_globals.Cache[cacheIdx]._y && bottom < (_vm->_globals.Cache[cacheIdx].field14 + _vm->_globals.Cache[cacheIdx]._height + _vm->_globals.Cache[cacheIdx]._y)) {
					if ((right >= _vm->_globals.Cache[cacheIdx]._x && right <= cachedRight)
					 || (cachedRight >= _sprite[spriteIdx]._destX && _vm->_globals.Cache[cacheIdx]._x <= _sprite[spriteIdx]._destX)
					 || (cachedRight >= _sprite[spriteIdx]._destX && _vm->_globals.Cache[cacheIdx]._x <= _sprite[spriteIdx]._destX)
					 || (_vm->_globals.Cache[cacheIdx]._x <= _sprite[spriteIdx]._destX && right <= cachedRight)
					 || (_vm->_globals.Cache[cacheIdx]._x >= _sprite[spriteIdx]._destX && right >= cachedRight))
						++_vm->_globals.Cache[cacheIdx]._useCount;
				}
			}
		}

		SCBOB(cacheIdx);
		if (_vm->_globals.Cache[cacheIdx]._useCount == _oldUseCount) {
			if (_vm->_globals.Cache[cacheIdx].field10) {
				_vm->_globals.Cache[cacheIdx].field10 = false;
				_vm->_globals.Cache[cacheIdx]._useCount = 1;
			}
		} else {
			int priority = _vm->_globals.Cache[cacheIdx].field14 + _vm->_globals.Cache[cacheIdx]._height + _vm->_globals.Cache[cacheIdx]._y;
			if (priority > 440)
				priority = 500;

			beforeSort(SORT_CACHE, cacheIdx, priority);
			_vm->_globals.Cache[cacheIdx]._useCount = 1;
			_vm->_globals.Cache[cacheIdx].field10 = true;
		}
	}
}

void ObjectsManager::DEF_SPRITE(int idx) {
	if (!_sprite[idx].field2A)
		return;

	if (_sprite[idx]._rleFl)
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _sprite[idx]._spriteData,
		    _sprite[idx]._destX + 300, _sprite[idx]._destY + 300, _sprite[idx]._spriteIndex);
	else
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _sprite[idx]._spriteData,
		    _sprite[idx]._destX + 300, _sprite[idx]._destY + 300,  _sprite[idx]._spriteIndex, _sprite[idx]._reducePct, _sprite[idx]._zoomPct, _sprite[idx]._flipFl);

	_vm->_globals.Liste[idx]._width = _sprite[idx]._width;
	_vm->_globals.Liste[idx]._height = _sprite[idx]._height;

	if (_vm->_globals.Liste[idx]._posX < _vm->_graphicsManager._minX) {
		_vm->_globals.Liste[idx]._width -= _vm->_graphicsManager._minX - _vm->_globals.Liste[idx]._posX;
		_vm->_globals.Liste[idx]._posX = _vm->_graphicsManager._minX;
	}

	if (_vm->_globals.Liste[idx]._posY < _vm->_graphicsManager._minY) {
		_vm->_globals.Liste[idx]._height -= _vm->_graphicsManager._minY - _vm->_globals.Liste[idx]._posY;
		_vm->_globals.Liste[idx]._posY = _vm->_graphicsManager._minY;
	}

	if (_vm->_globals.Liste[idx]._width + _vm->_globals.Liste[idx]._posX > _vm->_graphicsManager._maxX)
		_vm->_globals.Liste[idx]._width = _vm->_graphicsManager._maxX - _vm->_globals.Liste[idx]._posX;

	if (_vm->_globals.Liste[idx]._height + _vm->_globals.Liste[idx]._posY > _vm->_graphicsManager._maxY)
		_vm->_globals.Liste[idx]._height = _vm->_graphicsManager._maxY - _vm->_globals.Liste[idx]._posY;

	if (_vm->_globals.Liste[idx]._width <= 0 || _vm->_globals.Liste[idx]._height <= 0)
		_vm->_globals.Liste[idx]._visibleFl = false;

	if (_vm->_globals.Liste[idx]._visibleFl)
		_vm->_graphicsManager.addVesaSegment( _vm->_globals.Liste[idx]._posX, _vm->_globals.Liste[idx]._posY,
		    _vm->_globals.Liste[idx]._posX + _vm->_globals.Liste[idx]._width, _vm->_globals.Liste[idx]._posY + _vm->_globals.Liste[idx]._height);
}

void ObjectsManager::displayCache(int idx) {
	_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.CACHE_BANQUE[1],
	    _vm->_globals.Cache[idx]._x + 300, _vm->_globals.Cache[idx]._y + 300,
	    _vm->_globals.Cache[idx]._spriteIndex);

	_vm->_graphicsManager.addVesaSegment(_vm->_globals.Cache[idx]._x, _vm->_globals.Cache[idx]._y,
	    _vm->_globals.Cache[idx]._x + _vm->_globals.Cache[idx]._width, _vm->_globals.Cache[idx]._y + _vm->_globals.Cache[idx]._height);
}

// Compute Sprite
void ObjectsManager::computeSprite(int idx) {
	_sprite[idx].field2A = false;
	int spriteIndex = _sprite[idx]._spriteIndex;
	if (spriteIndex == 250)
		return;

	int offX;
	int offY;
	if (_sprite[idx]._flipFl) {
		offX = getOffsetX(_sprite[idx]._spriteData, spriteIndex, true);
		offY = getOffsetY(_sprite[idx]._spriteData, _sprite[idx]._spriteIndex, true);
	} else {
		offX = getOffsetX(_sprite[idx]._spriteData, spriteIndex, false);
		offY = getOffsetY(_sprite[idx]._spriteData, _sprite[idx]._spriteIndex, false);
	}

	int tmpX = _sprite[idx].field12 + offX;
	int deltaX = tmpX;
	int tmpY = _sprite[idx].field14 + offY;
	int deltaY = tmpY;
	int zoomPercent = 0;
	int reducePercent = 0;

	if (_sprite[idx]._zoomFactor < 0) {
		reducePercent = -_sprite[idx]._zoomFactor;
		if (reducePercent > 95)
			reducePercent = 95;
	} else 
		zoomPercent = _sprite[idx]._zoomFactor;

	if (zoomPercent) {
		if (tmpX >= 0)
			deltaX = _vm->_graphicsManager.zoomIn(tmpX, zoomPercent);
		else
			deltaX = -_vm->_graphicsManager.zoomIn(-tmpX, zoomPercent);

		if (tmpY >= 0) {
			deltaY = _vm->_graphicsManager.zoomIn(tmpY, zoomPercent);
		} else {
			tmpY = abs(tmpX);
			deltaY = -_vm->_graphicsManager.zoomIn(tmpY, zoomPercent);
		}
	} else if (reducePercent) {
		if (tmpX >= 0)
			deltaX = _vm->_graphicsManager.zoomOut(tmpX, reducePercent);
		else
			deltaX = -_vm->_graphicsManager.zoomOut(-tmpX, reducePercent);

		if (tmpY >= 0) {
			deltaY = _vm->_graphicsManager.zoomOut(tmpY, reducePercent);
		} else {
			tmpY = abs(tmpX);
			deltaY = -_vm->_graphicsManager.zoomOut(tmpY, reducePercent);
		}
	}

	int newPosX = _sprite[idx]._spritePos.x - deltaX;
	int newPosY = _sprite[idx]._spritePos.y - deltaY;
	_sprite[idx]._destX = newPosX;
	_sprite[idx]._destY = newPosY;
	_sprite[idx].field2A = true;
	_sprite[idx]._zoomPct = zoomPercent;
	_sprite[idx]._reducePct = reducePercent;

	_vm->_globals.Liste[idx]._visibleFl = true;
	_vm->_globals.Liste[idx]._posX = newPosX;
	_vm->_globals.Liste[idx]._posY = newPosY;

	int width = getWidth(_sprite[idx]._spriteData, _sprite[idx]._spriteIndex);
	int height = getHeight(_sprite[idx]._spriteData, _sprite[idx]._spriteIndex);

	if (zoomPercent) {
		width = _vm->_graphicsManager.zoomIn(width, zoomPercent);
		height = _vm->_graphicsManager.zoomIn(height, zoomPercent);
	} else if (reducePercent) {
		height = _vm->_graphicsManager.zoomOut(height, reducePercent);
		width = _vm->_graphicsManager.zoomOut(width, reducePercent);
	}

	_sprite[idx]._width = width;
	_sprite[idx]._height = height;
}

// Before Sort
void ObjectsManager::beforeSort(SortMode sortMode, int index, int priority) {
	++_vm->_globals._sortedDisplayCount;
	assert (_vm->_globals._sortedDisplayCount <= 48);

	_vm->_globals._sortedDisplay[_vm->_globals._sortedDisplayCount]._sortMode = sortMode;
	_vm->_globals._sortedDisplay[_vm->_globals._sortedDisplayCount]._index = index;
	_vm->_globals._sortedDisplay[_vm->_globals._sortedDisplayCount]._priority = priority;
}

// Display BOB Anim
void ObjectsManager::displayBobAnim() {
	for (int idx = 1; idx <= 35; idx++) {
		if (idx <= 20 && PERSO_ON) {
			_bob[idx].field1C = false;
			continue;
		}

		if (_bob[idx].field0 != 10) 
			continue;

		_bob[idx].field1C = false;
		int v1 = _bob[idx].field20;
		if (v1 == -1)
			v1 = 50;
		if (_bob[idx]._animData == g_PTRNUL || _bob[idx]._disabledAnimationFl || v1 <= 0) {
			if (_bob[idx].field1E == 1 || _bob[idx].field1E == 2)
				_bob[idx].field1C = true;
			continue;
		}

		if (_bob[idx].field12 == _bob[idx].field14) {
			_bob[idx].field1C = true;
		} else {
			_bob[idx].field14++;
			_bob[idx].field1C = false;
		}

		if (!_bob[idx].field1C) {
			if (_bob[idx].field1E == 1 || _bob[idx].field1E == 2)
				_bob[idx].field1C = true;
			continue;
		}

		byte *dataPtr = _bob[idx]._animData + 20;
		int dataIdx = _bob[idx]._animDataIdx;
		_bob[idx]._xp = (int16)READ_LE_UINT16(dataPtr + 2 * dataIdx);
		if (_vm->_globals._lockedAnims[idx]._enableFl)
			_bob[idx]._xp = _vm->_globals._lockedAnims[idx]._posX;
		if ( PERSO_ON && idx > 20 )
			_bob[idx]._xp += _vm->_eventsManager._startPos.x;

		_bob[idx]._yp = (int16)READ_LE_UINT16(dataPtr + 2 * dataIdx + 2);
		_bob[idx].field12 = (int16)READ_LE_UINT16(dataPtr + 2 * dataIdx + 4);
		_bob[idx]._zoomFactor = (int16)READ_LE_UINT16(dataPtr + 2 * dataIdx + 6);
		_bob[idx]._frameIndex = dataPtr[2 * dataIdx + 8];
		_bob[idx]._flipFl = (dataPtr[2 * dataIdx + 9] != 0);
		_bob[idx]._animDataIdx += 5;

		int v5 = _bob[idx].field12;
		if (v5 > 0) {
			int v6 = v5 / _vm->_globals._speed;
			_bob[idx].field12 = v5 / _vm->_globals._speed;
			if (v6 > 0) {
				_bob[idx].field14 = 1;
				if (_bob[idx].field1E == 1 || _bob[idx].field1E == 2)
					_bob[idx].field1C = true;
				continue;
			}

			_bob[idx].field12 = 1;
		}
		if (!_bob[idx].field12) {
			if (_bob[idx].field20 > 0)
				_bob[idx].field20--;
			if (_bob[idx].field20 != -1 && _bob[idx].field20 <= 0) {
				_bob[idx].field0 = 11;
			} else {
				_bob[idx]._animDataIdx = 0;
				byte *v21 = _bob[idx]._animData + 20;
				_bob[idx]._xp = (int16)READ_LE_UINT16(v21);

				if (_vm->_globals._lockedAnims[idx]._enableFl)
					_bob[idx]._xp = _vm->_globals._lockedAnims[idx]._posX;
				if (PERSO_ON && idx > 20)
					_bob[idx]._xp += _vm->_eventsManager._startPos.x;

				_bob[idx]._yp = (int16)READ_LE_UINT16(v21 + 2);
				_bob[idx].field12 = (int16)READ_LE_UINT16(v21 + 4);
				_bob[idx]._zoomFactor = (int16)READ_LE_UINT16(v21 + 6);
				_bob[idx]._frameIndex = v21[8];
				_bob[idx]._flipFl = (v21[9] != 0);
				_bob[idx]._animDataIdx += 5;
				int v10 = _bob[idx].field12;

				if (v10 > 0) {
					int v11 = v10 / _vm->_globals._speed;
					_bob[idx].field12 = v11;
					// Original code. It can't be negative, so the check is on == 0
					if (v11 <= 0)
						_bob[idx].field12 = 1;
				}
			}
		}

		_bob[idx].field14 = 1;
		if (_bob[idx].field1E == 1 || _bob[idx].field1E == 2)
			_bob[idx].field1C = true;
	}

	if (!PERSO_ON && BOBTOUS) {
		for (int i = 0; i < 35; i++) {
			if (_bob[i].field0 == 10 && !_bob[i]._disabledAnimationFl)
				_bob[i].field1C = true;
		}
	}

	BOBTOUS = false;

	for (int i = 1; i <= 35; i++) {
		if (i > 20 || !PERSO_ON) {
			if ((_bob[i].field0 == 10) && (_bob[i].field1C)) {
				if ((_bob[i].field1E != 2) && (_bob[i].field1E != 4)) {
					if (_vm->_globals.Liste2[i]._visibleFl) {
						_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen,
							_vm->_globals.Liste2[i]._posX, _vm->_globals.Liste2[i]._posY,
							_vm->_globals.Liste2[i]._width, _vm->_globals.Liste2[i]._height,
							_vm->_graphicsManager._vesaBuffer, _vm->_globals.Liste2[i]._posX,
							_vm->_globals.Liste2[i]._posY);
						_vm->_globals.Liste2[i]._visibleFl = false;
					}
				}
			}

			if (_bob[i].field0 == 11) {
				if (_vm->_globals.Liste2[i]._visibleFl) {
					_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen,
						_vm->_globals.Liste2[i]._posX, _vm->_globals.Liste2[i]._posY,
						_vm->_globals.Liste2[i]._width, _vm->_globals.Liste2[i]._height,
						_vm->_graphicsManager._vesaBuffer,
						_vm->_globals.Liste2[i]._posX, _vm->_globals.Liste2[i]._posY);
					_vm->_globals.Liste2[i]._visibleFl = false;
				}

				_bob[i].field0 = 0;
			}
		}
	}

	for (int i = 1; i <= 35; i++) {
		_bob[i]._oldY = 0;
		if (_bob[i].field0 == 10 && !_bob[i]._disabledAnimationFl && _bob[i].field1C) {
			CALCUL_BOB(i);
			int v19 = _bob[i]._oldX2 + _bob[i]._oldHeight + _bob[i]._oldY;

			if (v19 > 450)
				v19 = 600;

			if (_bob[i]._activeFl)
				beforeSort(SORT_BOB, i, v19);
		}
	}
}

// Display VBOB
void ObjectsManager::displayVBob() {
	int width, height;

	for (int idx = 0; idx <= 29; idx++) {
		if (_vm->_globals.VBob[idx].field4 == 4) {
			width = getWidth(_vm->_globals.VBob[idx]._spriteData, _vm->_globals.VBob[idx]._frameIndex);
			height = getHeight(_vm->_globals.VBob[idx]._spriteData, _vm->_globals.VBob[idx]._frameIndex);

			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaScreen,
				_vm->_globals.VBob[idx]._surface, _vm->_globals.VBob[idx]._xp,
				_vm->_globals.VBob[idx]._yp, width, height);

			_vm->_graphicsManager.Restore_Mem(
				_vm->_graphicsManager._vesaBuffer, _vm->_globals.VBob[idx]._surface,
				_vm->_globals.VBob[idx]._xp, _vm->_globals.VBob[idx]._yp,
				width, height);

			_vm->_graphicsManager.addVesaSegment(
				_vm->_globals.VBob[idx]._xp, _vm->_globals.VBob[idx]._yp,
				_vm->_globals.VBob[idx]._xp + width, height + _vm->_globals.VBob[idx]._yp);

			_vm->_globals.VBob[idx]._surface = _vm->_globals.freeMemory(_vm->_globals.VBob[idx]._surface);

			_vm->_globals.VBob[idx].field4 = 0;
			_vm->_globals.VBob[idx]._spriteData = g_PTRNUL;
			_vm->_globals.VBob[idx]._xp = 0;
			_vm->_globals.VBob[idx]._yp = 0;
			_vm->_globals.VBob[idx]._oldX = 0;
			_vm->_globals.VBob[idx]._oldY = 0;
			_vm->_globals.VBob[idx]._frameIndex = 0;
			_vm->_globals.VBob[idx]._oldFrameIndex = 0;
			_vm->_globals.VBob[idx]._oldSpriteData = g_PTRNUL;
		}

		if (_vm->_globals.VBob[idx].field4 == 3) {
			width = getWidth(_vm->_globals.VBob[idx]._oldSpriteData, _vm->_globals.VBob[idx]._oldFrameIndex);
			height = getHeight(_vm->_globals.VBob[idx]._oldSpriteData, _vm->_globals.VBob[idx]._oldFrameIndex);

			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaScreen,
				_vm->_globals.VBob[idx]._surface, _vm->_globals.VBob[idx]._oldX,
				_vm->_globals.VBob[idx]._oldY,
				width, height);

			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer,
				_vm->_globals.VBob[idx]._surface, _vm->_globals.VBob[idx]._oldX,
				_vm->_globals.VBob[idx]._oldY, width, height);

			_vm->_graphicsManager.addVesaSegment(_vm->_globals.VBob[idx]._oldX,
				_vm->_globals.VBob[idx]._oldY, _vm->_globals.VBob[idx]._oldX + width,
				_vm->_globals.VBob[idx]._oldY + height);

			_vm->_globals.VBob[idx].field4 = 1;
			_vm->_globals.VBob[idx]._oldSpriteData = _vm->_globals.VBob[idx]._spriteData;

			_vm->_globals.VBob[idx]._surface = _vm->_globals.freeMemory(_vm->_globals.VBob[idx]._surface);

			_vm->_globals.VBob[idx]._oldX = _vm->_globals.VBob[idx]._xp;
			_vm->_globals.VBob[idx]._oldY = _vm->_globals.VBob[idx]._yp;
			_vm->_globals.VBob[idx]._oldFrameIndex = _vm->_globals.VBob[idx]._frameIndex;
		}

		if (_vm->_globals.VBob[idx].field4 == 1) {
			width = getWidth(_vm->_globals.VBob[idx]._spriteData, _vm->_globals.VBob[idx]._frameIndex);
			height = getHeight(_vm->_globals.VBob[idx]._spriteData, _vm->_globals.VBob[idx]._frameIndex);

			_vm->_globals.VBob[idx]._surface = _vm->_globals.freeMemory(_vm->_globals.VBob[idx]._surface);

			byte *surface = _vm->_globals.allocMemory(height * width);
			_vm->_globals.VBob[idx]._surface = surface;

			_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager._vesaScreen, surface,
				_vm->_globals.VBob[idx]._xp, _vm->_globals.VBob[idx]._yp, width, height);

			byte *v10 = _vm->_globals.VBob[idx]._spriteData;
			if (*v10 == 78) {
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaScreen, v10,
					_vm->_globals.VBob[idx]._xp + 300, _vm->_globals.VBob[idx]._yp + 300,
					_vm->_globals.VBob[idx]._frameIndex, 0, 0, false);

				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer,
					_vm->_globals.VBob[idx]._spriteData,
					_vm->_globals.VBob[idx]._xp + 300, _vm->_globals.VBob[idx]._yp + 300,
					_vm->_globals.VBob[idx]._frameIndex, 0, 0, false);
			} else {
				_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer,
					v10, _vm->_globals.VBob[idx]._xp + 300, _vm->_globals.VBob[idx]._yp + 300,
					_vm->_globals.VBob[idx]._frameIndex);

				_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaScreen, _vm->_globals.VBob[idx]._spriteData,
					_vm->_globals.VBob[idx]._xp + 300, _vm->_globals.VBob[idx]._yp + 300,
					_vm->_globals.VBob[idx]._frameIndex);
			}

			_vm->_graphicsManager.addVesaSegment(_vm->_globals.VBob[idx]._xp,
				_vm->_globals.VBob[idx]._yp , _vm->_globals.VBob[idx]._xp + width,
				_vm->_globals.VBob[idx]._yp + height);
			_vm->_globals.VBob[idx].field4 = 2;
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
		_vm->_globals.Liste[idx]._visibleFl = false;
		_vm->_globals.Liste[idx]._posX = 0;
		_vm->_globals.Liste[idx]._posY = 0;
		_vm->_globals.Liste[idx]._width = 0;
		_vm->_globals.Liste[idx]._height = 0;
	}
}

void ObjectsManager::animateSprite(int idx) {
	assert (idx  <= MAX_SPRITE);
	_sprite[idx]._animationType = 1;
}

void ObjectsManager::addStaticSprite(const byte *spriteData, Common::Point pos, int idx, int spriteIndex, int zoomFactor, bool flipFl, int a8, int a9) {
	assert (idx  <= MAX_SPRITE);
	_sprite[idx]._spriteData = spriteData;
	_sprite[idx]._spritePos = pos;
	_sprite[idx]._spriteIndex = spriteIndex;
	_sprite[idx]._zoomFactor = zoomFactor;
	_sprite[idx]._flipFl = flipFl;
	_sprite[idx].field12 = a8;
	_sprite[idx].field14 = a9;
	_sprite[idx]._animationType = 0;

	if (READ_BE_UINT24(spriteData) == MKTAG24('R', 'L', 'E')) {
		_sprite[idx]._rleFl = true;
		_sprite[idx]._zoomFactor = 0;
		_sprite[idx]._flipFl = false;
	} else
		_sprite[idx]._rleFl = false;

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

void ObjectsManager::checkZone() {
	int mouseX = _vm->_eventsManager.getMouseX();
	int mouseY = _vm->_eventsManager.getMouseY();
	int oldMouseY = mouseY;
	if (_vm->_globals._cityMapEnabledFl
	        || _vm->_eventsManager._startPos.x >= mouseX
	        || (mouseY = _vm->_graphicsManager._scrollOffset + 54, mouseX >= mouseY)
	        || (mouseY = oldMouseY - 1, mouseY < 0 || mouseY > 59)) {
		if (_visibleFl)
			_eraseVisibleCounter = 4;
		_visibleFl = false;
	} else {
		_visibleFl = true;
	}
	if (_forceZoneFl) {
		_vm->_globals.compteur_71 = 100;
		_vm->_globals._oldMouseZoneId = -1;
		_vm->_globals._oldMouseX = -200;
		_vm->_globals._oldMouseY = -220;
		_forceZoneFl = false;
	}

	_vm->_globals.compteur_71++;
	if (_vm->_globals.compteur_71 <= 1)
		return;

	if (_vm->_globals.NOMARCHE || (_vm->_linesManager._route == (int16 *)g_PTRNUL) || _vm->_globals.compteur_71 > 4) {
		_vm->_globals.compteur_71 = 0;
		int zoneId;
		if (_vm->_globals._oldMouseX != mouseX || _vm->_globals._oldMouseY != oldMouseY) {
			zoneId = _vm->_linesManager.MZONE();
		} else {
			zoneId = _vm->_globals._oldMouseZoneId;
		}
		if (_vm->_globals._oldMouseZoneId != zoneId) {
			_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
			_vm->_eventsManager._mouseCursorId = 4;
			_vm->_eventsManager.changeMouseCursor(4);
			if (_vm->_globals._forceHideText) {
				_vm->_fontManager.hideText(5);
				_vm->_globals._forceHideText = false;
				return;
			}
		}
		if (zoneId != -1) {
			if (_vm->_globals.ZONEP[zoneId].field6 || _vm->_globals.ZONEP[zoneId].field7 ||
			    _vm->_globals.ZONEP[zoneId].field8 || _vm->_globals.ZONEP[zoneId].field9 ||
			    _vm->_globals.ZONEP[zoneId].fieldA || _vm->_globals.ZONEP[zoneId].fieldB ||
			    _vm->_globals.ZONEP[zoneId].fieldC || _vm->_globals.ZONEP[zoneId].fieldD ||
			    _vm->_globals.ZONEP[zoneId].fieldE || _vm->_globals.ZONEP[zoneId].fieldF) {
				if (_vm->_globals._oldMouseZoneId != zoneId) {
					_vm->_fontManager.initTextBuffers(5, _vm->_globals.ZONEP[zoneId].field12, _vm->_globals.FICH_ZONE, 0, 430, 0, 0, 252);
					_vm->_fontManager.showText(5);
					_vm->_globals._forceHideText = true;
				}
				_vm->_globals._hotspotTextColor += 25;
				if (_vm->_globals._hotspotTextColor > 100)
					_vm->_globals._hotspotTextColor = 0;
				_vm->_graphicsManager.SETCOLOR4(251, _vm->_globals._hotspotTextColor, _vm->_globals._hotspotTextColor,
					_vm->_globals._hotspotTextColor);
				if (_vm->_eventsManager._mouseCursorId == 4) {
					if (_vm->_globals.ZONEP[zoneId].field6 == 2) {
						_vm->_eventsManager.changeMouseCursor(16);
						_vm->_eventsManager._mouseCursorId = 16;
						_verb = 16;
					}
				}
			} else {
				_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
				_vm->_eventsManager._mouseCursorId = 4;
				_vm->_eventsManager.changeMouseCursor(4);
			}
		}
		_zoneNum = zoneId;
		_vm->_globals._oldMouseX = mouseX;
		_vm->_globals._oldMouseY = oldMouseY;
		_vm->_globals._oldMouseZoneId = zoneId;
		if (_vm->_globals.NOMARCHE && (_vm->_eventsManager._mouseCursorId == 4)) {
			if (zoneId != -1 && zoneId != 0)
				handleRightButton();
		}
		if ((_vm->_globals._cityMapEnabledFl && zoneId == -1) || !zoneId) {
			_verb = 0;
			_vm->_eventsManager._mouseCursorId = 0;
			_vm->_eventsManager.changeMouseCursor(0);
		}
	}
}

void ObjectsManager::GOHOME() {
	int v0 = 0;
	int v58 = 0;
	int v1 = 0;

	if (_vm->_linesManager._route == (int16 *)g_PTRNUL)
		return;

	if (_vm->_globals.Compteur > 1) {
		--_vm->_globals.Compteur;
		return;
	}

	int newPosX;
	int newPosY;
	int newDirection;

	_vm->_globals.Compteur = 0;
	if (_vm->_globals._oldDirection == -1) {
		computeAndSetSpriteSize();
		newPosX = *_vm->_linesManager._route++;
		newPosY = *_vm->_linesManager._route++;
		newDirection = *_vm->_linesManager._route++;
		_vm->_linesManager._route++;

		if (newPosX != -1 || newPosY != -1) {
			_vm->_globals._oldDirection = newDirection;
			_vm->_globals._oldDirectionSpriteIdx = newDirection + 59;
			_vm->_globals._oldFrameIndex = 0;
			_oldCharacterPosX = newPosX;
			_oldCharacterPosY = newPosY;
		} else {
			setSpriteIndex(0, _vm->_globals._oldDirection + 59);
			_vm->_globals._actionDirection = 0;
			int zoneId;
			if (_vm->_globals.GOACTION)
				zoneId = _vm->_globals._saveData->_data[svField2];
			else
				zoneId = _zoneNum;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			computeAndSetSpriteSize();
			setFlipSprite(0, false);
			_vm->_globals.Compteur = 0;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			_vm->_globals._oldDirection = -1;
			if (zoneId > 0) {
				if (_vm->_globals.ZONEP[zoneId]._destX && _vm->_globals.ZONEP[zoneId]._destY && _vm->_globals.ZONEP[zoneId]._destY != 31) {
					if (_vm->_globals.ZONEP[zoneId]._spriteIndex == -1) {
						_vm->_globals.ZONEP[zoneId]._destX = 0;
						_vm->_globals.ZONEP[zoneId]._destY = 0;
						_vm->_globals.ZONEP[zoneId]._spriteIndex = 0;
					} else {
						setSpriteIndex(0, _vm->_globals.ZONEP[zoneId]._spriteIndex);
						_vm->_globals._actionDirection = _vm->_globals.ZONEP[zoneId]._spriteIndex - 59;
					}
				}
			}
		}
		_vm->_globals.Compteur = 0;
		return;
	}
	if (_vm->_globals._oldDirection == 3) {
		if (_vm->_globals._oldFrameIndex < 24 || _vm->_globals._oldFrameIndex > 35) {
			v0 = _oldCharacterPosX;
			v58 = _oldCharacterPosY;
			v1 = 24;
		} else {
			int deltaX = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field0;
			int deltaY = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field2;

			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager.zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager.zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			v0 = _oldCharacterPosX + deltaX;
			v58 = _oldCharacterPosY + deltaY;
			v1 = _vm->_globals._oldFrameIndex + 1;
			if (v1 > 35)
				v1 = 24;
		}
		_vm->_globals.Compteur = 5 / _vm->_globals._speed;
	}
	if (_vm->_globals._oldDirection == 7) {
		if (_vm->_globals._oldFrameIndex < 24 || _vm->_globals._oldFrameIndex > 35) {
			v0 = _oldCharacterPosX;
			v58 = _oldCharacterPosY;
			v1 = 24;
		} else {
			int deltaX = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field0;
			int deltaY = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field2;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager.zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager.zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			v0 = _oldCharacterPosX - deltaX;
			v58 = _oldCharacterPosY - deltaY;
			v1 = _vm->_globals._oldFrameIndex + 1;
			if (v1 > 35)
				v1 = 24;
		}
		_vm->_globals.Compteur = 5 / _vm->_globals._speed;
	}
	if (_vm->_globals._oldDirection == 1) {
		if (_vm->_globals._oldFrameIndex > 11) {
			v0 = _oldCharacterPosX;
			v58 = _oldCharacterPosY;
			v1 = 0;
		} else {
			int deltaY = abs(_vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field2);
			if (_sprite[0]._zoomFactor < 0) {
				deltaY = _vm->_graphicsManager.zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaY = _vm->_graphicsManager.zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			v0 = _oldCharacterPosX;
			v58 = _oldCharacterPosY - deltaY;
			v1 = _vm->_globals._oldFrameIndex + 1;
			if (v1 > 11)
				v1 = 0;
		}
		_vm->_globals.Compteur = 4 / _vm->_globals._speed;
	}

	if (_vm->_globals._oldDirection == 5) {
		if (_vm->_globals._oldFrameIndex < 48 || _vm->_globals._oldFrameIndex > 59) {
			v0 = _oldCharacterPosX;
			v58 = _oldCharacterPosY;
			v1 = 48;
		} else {
			int deltaY = abs(_vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field2);
			if (_sprite[0]._zoomFactor < 0) {
				deltaY = _vm->_graphicsManager.zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaY = _vm->_graphicsManager.zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			v0 = _oldCharacterPosX;
			v58 = deltaY + _oldCharacterPosY;
			v1 = _vm->_globals._oldFrameIndex + 1;
			if (v1 > 59)
				v1 = 48;
		}
		_vm->_globals.Compteur = 4 / _vm->_globals._speed;
	}
	if (_vm->_globals._oldDirection == 2) {
		if (_vm->_globals._oldFrameIndex < 12 || _vm->_globals._oldFrameIndex > 23) {
			v0 = _oldCharacterPosX;
			v58 = _oldCharacterPosY;
			v1 = 12;
		} else {
			int deltaX = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field0;
			int deltaY = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field2;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager.zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomOut(deltaY, -_sprite[0]._zoomFactor);
			}
			if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager.zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			v0 = deltaX + _oldCharacterPosX;
			v58 = _oldCharacterPosY + deltaY;
			v1 = _vm->_globals._oldFrameIndex + 1;
			if (v1 > 23)
				v1 = 12;
		}
		_vm->_globals.Compteur = 5 / _vm->_globals._speed;
	}
	if (_vm->_globals._oldDirection == 8) {
		if (_vm->_globals._oldFrameIndex < 12 || _vm->_globals._oldFrameIndex > 23) {
			v0 = _oldCharacterPosX;
			v58 = _oldCharacterPosY;
			v1 = 12;
		} else {
			int deltaX = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field0;
			int deltaY = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field2;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager.zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomOut(deltaY, -_sprite[0]._zoomFactor);
			} else if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager.zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			v0 = _oldCharacterPosX - deltaX;
			v58 = _oldCharacterPosY + deltaY;
			v1 = _vm->_globals._oldFrameIndex + 1;
			if (v1 > 23)
				v1 = 12;
		}
		_vm->_globals.Compteur = 5 / _vm->_globals._speed;
	}
	if (_vm->_globals._oldDirection == 4) {
		if (_vm->_globals._oldFrameIndex < 36 || _vm->_globals._oldFrameIndex > 47) {
			v0 = _oldCharacterPosX;
			v58 = _oldCharacterPosY;
			v1 = 36;
		} else {
			int deltaX = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field0;
			int deltaY = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field2;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager.zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomOut(deltaY, -_sprite[0]._zoomFactor);
			}
			if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager.zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			v0 = deltaX + _oldCharacterPosX;
			v58 = _oldCharacterPosY + deltaY;
			v1 = _vm->_globals._oldFrameIndex + 1;
			if (v1 > 47)
				v1 = 36;
		}
		_vm->_globals.Compteur = 5 / _vm->_globals._speed;
	}
	if (_vm->_globals._oldDirection == 6) {
		if (_vm->_globals._oldFrameIndex < 36 || _vm->_globals._oldFrameIndex > 47) {
			v0 = _oldCharacterPosX;
			v58 = _oldCharacterPosY;
			v1 = 36;
		} else {
			int deltaX = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field0;
			int deltaY = _vm->_globals.Hopkins[_vm->_globals._oldFrameIndex].field2;
			if (_sprite[0]._zoomFactor < 0) {
				deltaX = _vm->_graphicsManager.zoomOut(deltaX, -_sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomOut(deltaY, -_sprite[0]._zoomFactor);
			}
			if (_sprite[0]._zoomFactor > 0) {
				deltaX = _vm->_graphicsManager.zoomIn(deltaX, _sprite[0]._zoomFactor);
				deltaY = _vm->_graphicsManager.zoomIn(deltaY, _sprite[0]._zoomFactor);
			}
			v0 = _oldCharacterPosX - deltaX;
			v58 = _oldCharacterPosY + deltaY;
			v1 = _vm->_globals._oldFrameIndex + 1;
			if (v1 > 47)
				v1 = 36;
		}
		_vm->_globals.Compteur = 5 / _vm->_globals._speed;
	}
	bool loopCond = false;
	do {
		newPosX = *_vm->_linesManager._route++;
		newPosY = *_vm->_linesManager._route++;
		newDirection = *_vm->_linesManager._route++;
		_vm->_linesManager._route++;

		if (newPosX == -1 && newPosY == -1) {
			int zoneId;
			if (_vm->_globals.GOACTION)
				zoneId = _vm->_globals._saveData->_data[svField2];
			else
				zoneId = _zoneNum;
			setSpriteIndex(0, _vm->_globals._oldDirection + 59);
			_vm->_globals._actionDirection = 0;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			computeAndSetSpriteSize();
			setFlipSprite(0, false);
			_vm->_globals.Compteur = 0;
			_vm->_globals._oldDirection = -1;
			_oldCharacterPosX = getSpriteX(0);
			_oldCharacterPosY = getSpriteY(0);

			if (zoneId > 0) {
				if (_vm->_globals.ZONEP[zoneId]._destX && _vm->_globals.ZONEP[zoneId]._destY && _vm->_globals.ZONEP[zoneId]._destY != 31) {
					if ( _vm->_globals.ZONEP[zoneId]._spriteIndex == -1) {
						_vm->_globals.ZONEP[zoneId]._destX = 0;
						_vm->_globals.ZONEP[zoneId]._destY = 0;
						_vm->_globals.ZONEP[zoneId]._spriteIndex = 0;
					} else {
						setSpriteIndex(0,  _vm->_globals.ZONEP[zoneId]._spriteIndex);
						_vm->_globals._actionDirection = _vm->_globals.ZONEP[zoneId]._spriteIndex - 59;
					}
				}
			}
			_vm->_globals.Compteur = 0;
			return;
		}
		if (_vm->_globals._oldDirection != newDirection)
			break;
		if ((newDirection == 3 && newPosX >= v0) || (_vm->_globals._oldDirection == 7 && newPosX <= v0) ||
		    (_vm->_globals._oldDirection == 1 && newPosY <= v58) || (_vm->_globals._oldDirection == 5 && newPosY >= v58) ||
		    (_vm->_globals._oldDirection == 2 && newPosX >= v0)  || (_vm->_globals._oldDirection == 8 && newPosX <= v0) ||
		    (_vm->_globals._oldDirection == 4 && newPosX >= v0) || (_vm->_globals._oldDirection == 6 && newPosX <= v0))
			loopCond = true;
	} while (!loopCond);
	if (loopCond) {
		computeAndSetSpriteSize();
		if ((_vm->_globals._oldDirection == 6) || (_vm->_globals._oldDirection == 7) || (_vm->_globals._oldDirection == 8))
			setFlipSprite(0, true);

		if ((_vm->_globals._oldDirection == 1) || (_vm->_globals._oldDirection == 2) || (_vm->_globals._oldDirection == 3) ||
		    (_vm->_globals._oldDirection == 4) || (_vm->_globals._oldDirection == 5))
			setFlipSprite(0, false);

		setSpriteX(0, newPosX);
		setSpriteY(0, newPosY);
		setSpriteIndex(0, v1);
	} else {
		if ((_vm->_globals._oldDirection == 6) || (_vm->_globals._oldDirection == 7) || (_vm->_globals._oldDirection == 8))
			setFlipSprite(0, true);

		if ((_vm->_globals._oldDirection == 1) || (_vm->_globals._oldDirection == 2) || (_vm->_globals._oldDirection == 3) ||
		    (_vm->_globals._oldDirection == 4) || (_vm->_globals._oldDirection == 5))
			setFlipSprite(0, false);
		_vm->_globals.Compteur = 0;
	}
	_vm->_globals._oldDirection = newDirection;
	_vm->_globals._oldDirectionSpriteIdx = newDirection + 59;
	_vm->_globals._oldFrameIndex = v1;
	_oldCharacterPosX = newPosX;
	_oldCharacterPosY = newPosY;
}

void ObjectsManager::GOHOME2() {
	if (_vm->_linesManager._route == (int16 *)g_PTRNUL)
		return;

	int v0 = 2;
	if (_vm->_globals._speed == 2)
		v0 = 4;
	else if (_vm->_globals._speed == 3)
		v0 = 6;

	_vm->_globals.j_104 = 0;

	for (;;) {
		int nexPosX = *_vm->_linesManager._route++;
		int newPosY = *_vm->_linesManager._route++;
		int newDirection = *_vm->_linesManager._route++;
		_vm->_linesManager._route++;

		if ((nexPosX == -1) && (newPosY == -1))
			break;

		++_vm->_globals.j_104;
		if (_vm->_globals.j_104 >= v0) {
			_vm->_globals._lastDirection = newDirection;
			setSpriteX(0, nexPosX);
			setSpriteY(0, newPosY);
			if (_vm->_globals._lastDirection == 1)
				setSpriteIndex(0, 4);
			else if (_vm->_globals._lastDirection == 3)
				setSpriteIndex(0, 5);
			else if (_vm->_globals._lastDirection == 5)
				setSpriteIndex(0, 6);
			else if (_vm->_globals._lastDirection == 7)
				setSpriteIndex(0, 7);

			return;
		}
	}
	if (_vm->_globals._lastDirection == 1)
		setSpriteIndex(0, 0);
	else if (_vm->_globals._lastDirection == 3)
		setSpriteIndex(0, 1);
	else if (_vm->_globals._lastDirection == 5)
		setSpriteIndex(0, 2);
	else if (_vm->_globals._lastDirection == 7)
		setSpriteIndex(0, 3);

	_vm->_linesManager._route = (int16 *)g_PTRNUL;
}

/**
 * Load Zone
 */
void ObjectsManager::loadZone(const Common::String &file) {
	for (int i = 1; i <= 100; i++) {
		_vm->_globals.ZONEP[i]._destX = 0;
		_vm->_globals.ZONEP[i]._destY = 0;
		_vm->_globals.ZONEP[i]._spriteIndex = 0;
		_vm->_globals.ZONEP[i].field6 = 0;
		_vm->_globals.ZONEP[i].field7 = 0;
		_vm->_globals.ZONEP[i].field8 = 0;
		_vm->_globals.ZONEP[i].field9 = 0;
		_vm->_globals.ZONEP[i].fieldA = 0;
		_vm->_globals.ZONEP[i].fieldB = 0;
		_vm->_globals.ZONEP[i].fieldC = 0;
		_vm->_globals.ZONEP[i].fieldD = 0;
		_vm->_globals.ZONEP[i].fieldE = 0;
		_vm->_globals.ZONEP[i].fieldF = 0;
		_vm->_globals.ZONEP[i].field12 = 0;
		_vm->_globals.ZONEP[i]._enabledFl = false;
	}

	Common::File f;
	if (!f.exists(file))
		error("File not found : %s", file.c_str());

	byte *ptr = _vm->_fileManager.loadFile(file);
	int bufId = 0;
	int zoneLineIdx = 0;
	int bobZoneIdx;
	do {
		bobZoneIdx = (int16)READ_LE_UINT16((uint16 *)ptr + bufId);
		if (bobZoneIdx != -1) {
			_vm->_linesManager.addZoneLine(
			    zoneLineIdx,
			    READ_LE_UINT16((uint16 *)ptr + bufId + 1),
			    READ_LE_UINT16((uint16 *)ptr + bufId + 2),
			    READ_LE_UINT16((uint16 *)ptr + bufId + 3),
			    READ_LE_UINT16((uint16 *)ptr + bufId + 4),
			    bobZoneIdx);
			_vm->_globals.ZONEP[bobZoneIdx]._enabledFl = true;
		}
		bufId += 5;
		++zoneLineIdx;
	} while (bobZoneIdx != -1);

	for (int i = 1; i <= 100; i++) {
		_vm->_globals.ZONEP[i]._destX = (int16)READ_LE_UINT16((uint16 *)ptr + bufId);
		_vm->_globals.ZONEP[i]._destY = (int16)READ_LE_UINT16((uint16 *)ptr + bufId + 1);
		_vm->_globals.ZONEP[i]._spriteIndex = (int16)READ_LE_UINT16((uint16 *)ptr + bufId + 2);
		bufId += 3;
	}

	byte *v9 = (ptr + 10 * zoneLineIdx + 606);
	bufId = 0;
	for (int i = 1; i <= 100; i++) {
		_vm->_globals.ZONEP[i].field6 = v9[bufId];
		_vm->_globals.ZONEP[i].field7 = v9[bufId + 1];
		_vm->_globals.ZONEP[i].field8 = v9[bufId + 2];
		_vm->_globals.ZONEP[i].field9 = v9[bufId + 3];
		_vm->_globals.ZONEP[i].fieldA = v9[bufId + 4];
		_vm->_globals.ZONEP[i].fieldB = v9[bufId + 5];
		_vm->_globals.ZONEP[i].fieldC = v9[bufId + 6];
		_vm->_globals.ZONEP[i].fieldD = v9[bufId + 7];
		_vm->_globals.ZONEP[i].fieldE = v9[bufId + 8];
		_vm->_globals.ZONEP[i].fieldF = v9[bufId + 9];

		bufId += 10;
	}
	v9 += 1010;
	for (int i = 0; i < 100; i++)
		_vm->_globals.ZONEP[i + 1].field12 = READ_LE_UINT16(v9 + 2 * i);

	_vm->_globals.freeMemory(ptr);
	_vm->_linesManager.CARRE_ZONE();
}

void ObjectsManager::handleCityMap() {
	_vm->_dialogsManager._inventFl = false;
	_vm->_eventsManager._gameKey = KEY_NONE;
	_vm->_linesManager.setMaxLineIdx(1);
	_vm->_globals._characterMaxPosY = 440;
	_vm->_globals._cityMapEnabledFl = true;
	_vm->_graphicsManager._noFadingFl = false;
	_vm->_globals.NOMARCHE = false;
	_spritePtr = g_PTRNUL;
	_vm->_globals._exitId = 0;
	_vm->_globals.NOT_VERIF = true;
	_vm->_soundManager.WSOUND(31);
	_vm->_globals.iRegul = 1;
	_vm->_graphicsManager.loadImage("PLAN");
	_vm->_linesManager.loadLines("PLAN.OB2");
	_vm->_globals.loadCache("PLAN.CA2");
	loadZone("PLAN.ZO2");
	_spritePtr = _vm->_fileManager.loadFile("VOITURE.SPR");
	_vm->_animationManager.loadAnim("PLAN");
	_vm->_graphicsManager.VISU_ALL();
	_vm->_graphicsManager.INI_ECRAN2("PLAN", false);
	for (int i = 0; i <= 15; i++)
		_vm->_globals.B_CACHE_OFF(i);
	_vm->_globals.B_CACHE_OFF(19);
	_vm->_globals.B_CACHE_OFF(20);
	_vm->_globals.CACHE_ON();

	if (!_mapCarPosX && !_mapCarPosY) {
		_mapCarPosX = 900;
		_mapCarPosY = 319;
	}
	addStaticSprite(_spritePtr, Common::Point(_mapCarPosX, _mapCarPosY), 0, 1, 0, false, 5, 5);
	_vm->_eventsManager.setMouseXY(_mapCarPosX, _mapCarPosY);
	_vm->_eventsManager.mouseOn();
	_vm->_graphicsManager.scrollScreen(getSpriteX(0) - 320);
	_vm->_graphicsManager._scrollOffset = getSpriteX(0) - 320;
	animateSprite(0);
	_vm->_linesManager._route = (int16 *)g_PTRNUL;
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);

	for (int i = 0; i <= 4; i++)
		_vm->_eventsManager.VBL();

	_vm->_globals.iRegul = 1;
	_vm->_graphicsManager.fadeInLong();
	_vm->_eventsManager.changeMouseCursor(4);
	_vm->_graphicsManager._noFadingFl = false;

	bool loopCond = false;
	do {
		int mouseButton = _vm->_eventsManager.getMouseButton();
		if (mouseButton) {
			if (_vm->_globals._saveData->_data[svField170] == 1 && !_vm->_globals._saveData->_data[svField171]) {
				_vm->_globals._saveData->_data[svField171] = 1;
				_vm->_globals._introSpeechOffFl = true;
				_vm->_talkManager.PARLER_PERSO("APPEL1.pe2");
				_vm->_globals._introSpeechOffFl = false;
				mouseButton = 0;
			}
			if (_vm->_globals._saveData->_data[svField80] == 1 && !_vm->_globals._saveData->_data[svField172]) {
				_vm->_globals._saveData->_data[svField172] = 1;
				_vm->_globals._introSpeechOffFl = true;
				_vm->_talkManager.PARLER_PERSO("APPEL2.pe2");
				_vm->_globals._introSpeechOffFl = false;
				mouseButton = 0;
				_vm->_eventsManager._curMouseButton = 0;
			}
			if (mouseButton == 1)
				handleLeftButton();
		}

		checkZone();
		GOHOME2();

		if (_vm->_linesManager._route == (int16 *)g_PTRNUL && _vm->_globals.GOACTION)
			PARADISE();
		_vm->_eventsManager.VBL();

		if (_vm->_globals._exitId)
			loopCond = true;
	} while (!_vm->shouldQuit() && !loopCond);

	if (!_vm->_graphicsManager._noFadingFl)
		_vm->_graphicsManager.fadeOutLong();
	_vm->_globals.iRegul = 0;
	_vm->_graphicsManager._noFadingFl = false;
	_mapCarPosX = getSpriteX(0);
	_mapCarPosY = getSpriteY(0);
	removeSprite(0);
	_spritePtr = _vm->_globals.freeMemory(_spritePtr);
	clearScreen();
	_vm->_globals._cityMapEnabledFl = false;
}

/**
 * Handle Left button
 */
void ObjectsManager::handleLeftButton() {
	_vm->_fontManager.hideText(9);
	int destX = _vm->_eventsManager.getMouseX();
	int destY = _vm->_eventsManager.getMouseY();

	if (!_vm->_dialogsManager._inventFl && !_vm->_globals._cityMapEnabledFl && 
		destX > _vm->_graphicsManager._scrollOffset - 30 && destX < _vm->_graphicsManager._scrollOffset + 50 && 
		destY > -30 && destY < 50) {
		int oldMouseCursor = _vm->_eventsManager._mouseCursorId;
		_vm->_dialogsManager._inventFl = true;
		_vm->_dialogsManager.showInventory();
		_vm->_dialogsManager._inventFl = false;
		_vm->_eventsManager._gameKey = KEY_NONE;
		if (!_vm->_globals._exitId) {
			_vm->_dialogsManager._inventFl = false;
			_vm->_eventsManager._mouseCursorId = oldMouseCursor;
		}
		return;
	}
	if (_vm->_globals._saveData->_data[svField354] == 1 && !_vm->_globals._cityMapEnabledFl
	    && destX >= 533 && destX <= 559 && destY >= 26 && destY <= 59) {
		changeCharacterHead(CHARACTER_HOPKINS_CLONE, CHARACTER_HOPKINS);
		return;
	}
	if (_vm->_globals._saveData->_data[svField356] == 1 && !_vm->_globals._cityMapEnabledFl
	    && destX >= 533 && destX <= 559 && destY >= 26 && destY <= 48) {
		changeCharacterHead(CHARACTER_SAMANTHA, CHARACTER_HOPKINS);
		return;
	}
	if (_vm->_globals._saveData->_data[svField357] == 1) {
		if (_vm->_globals._saveData->_data[svField353] == 1 && !_vm->_globals._cityMapEnabledFl
		    && destX >= 533 && destX <= 559 && destY >= 26 && destY <= 59) {
			changeCharacterHead(CHARACTER_HOPKINS, CHARACTER_HOPKINS_CLONE);
			return;
		}
		if (_vm->_globals._saveData->_data[svField355] == 1 && !_vm->_globals._cityMapEnabledFl
		    && destX >= 567 && destX <= 593 && destY >= 26 && destY <= 59) {
			changeCharacterHead(CHARACTER_HOPKINS, CHARACTER_SAMANTHA);
			return;
		}
	}
	if (_vm->_globals._cityMapEnabledFl && _vm->_globals.GOACTION) {
		checkZone();
		if (_zoneNum <= 0)
			return;
		int routeIdx = 0;
		do {
			_vm->_linesManager.essai2[routeIdx] = _vm->_linesManager._route[routeIdx];
			++routeIdx;
		} while (_vm->_linesManager._route[routeIdx] != -1);

		_vm->_linesManager.essai2[routeIdx] = -1;
		_vm->_linesManager.essai2[routeIdx + 1] = -1;
		_vm->_linesManager.essai2[routeIdx + 2] = -1;
		_vm->_linesManager.essai2[routeIdx + 3] = -1;
	}

	if (_vm->_globals.GOACTION) {
		checkZone();
		_vm->_globals.GOACTION = false;
		_vm->_globals._saveData->_data[svField1] = 0;
		_vm->_globals._saveData->_data[svField2] = 0;
	}

	if (_vm->_globals._cityMapEnabledFl && (_vm->_eventsManager._mouseCursorId != 4 || _zoneNum <= 0))
		return;
	if (_zoneNum != -1 && _zoneNum != 0) {
		if (_vm->_globals.ZONEP[_zoneNum]._destX && _vm->_globals.ZONEP[_zoneNum]._destY && _vm->_globals.ZONEP[_zoneNum]._destY != 31) {
			destX = _vm->_globals.ZONEP[_zoneNum]._destX;
			destY = _vm->_globals.ZONEP[_zoneNum]._destY;
		}
	}
	_vm->_globals.GOACTION = false;
	int16 *oldRoute = _vm->_linesManager._route;
	_vm->_linesManager._route = (int16 *)g_PTRNUL;
	if (_forestFl && _zoneNum >= 20 && _zoneNum <= 23) {
		if (getSpriteY(0) > 374 && getSpriteY(0) <= 410) {
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			setSpriteIndex(0, _vm->_globals._oldDirectionSpriteIdx);
			_vm->_globals._actionDirection = 0;
			_vm->_linesManager._route = (int16 *)g_PTRNUL;
			computeAndSetSpriteSize();
			setFlipSprite(0, false);
			_vm->_globals.Compteur = 0;
			_vm->_globals._oldDirection = -1;
		} else {
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(getSpriteX(0), getSpriteY(0), getSpriteX(0), 390);
			if (_vm->_linesManager._route != (int16 *)g_PTRNUL)
				_vm->_linesManager.PACOURS_PROPRE(_vm->_linesManager._route);
			_oldCharacterPosX = getSpriteX(0);
			_oldCharacterPosY = getSpriteY(0);
			_vm->_globals.Compteur = 0;
			if (_vm->_linesManager._route != (int16 *)g_PTRNUL || oldRoute == _vm->_linesManager._route) {
				_vm->_globals._oldDirection = -1;
			} else {
				_vm->_linesManager._route = oldRoute;
			}
		}
	} else {
		if (!_vm->_globals.NOMARCHE && !_vm->_globals._cityMapEnabledFl) {
			_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(getSpriteX(0), getSpriteY(0), destX, destY);
			if (_vm->_linesManager._route != (int16 *)g_PTRNUL)
				_vm->_linesManager.PACOURS_PROPRE(_vm->_linesManager._route);
			_oldCharacterPosX = getSpriteX(0);
			_oldCharacterPosY = getSpriteY(0);
			_vm->_globals.Compteur = 0;
			if (_vm->_linesManager._route != (int16 *)g_PTRNUL || oldRoute == _vm->_linesManager._route)
				_vm->_globals._oldDirection = -1;
			else
				_vm->_linesManager._route = oldRoute;
		}
	}

	if (!_vm->_globals.NOMARCHE && _vm->_globals._cityMapEnabledFl)
		_vm->_linesManager._route = _vm->_linesManager.cityMapCarRoute(getSpriteX(0), getSpriteY(0), destX, destY);

	if (_zoneNum != -1 && _zoneNum != 0) {
		if (_vm->_eventsManager._mouseCursorId == 23)
			_vm->_globals._saveData->_data[svField1] = 5;
		else 
			_vm->_globals._saveData->_data[svField1] = _vm->_eventsManager._mouseCursorId;

		if (_vm->_globals._cityMapEnabledFl)
			_vm->_globals._saveData->_data[svField1] = 6;
		_vm->_globals._saveData->_data[svField2] = _zoneNum;
		_vm->_globals._saveData->_data[svField3] = _curObjectIndex;
		_vm->_globals.GOACTION = true;
	}
	_vm->_fontManager.hideText(5);
	_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
	if (_vm->_globals._screenId == 20 && _vm->_globals._saveData->_data[svField132] == 1
				&& _curObjectIndex == 20 && _zoneNum == 12
				&& _vm->_eventsManager._mouseCursorId == 23) {
		// Special case for throwing darts at the switch in Purgatory - the player shouldn't move
		_vm->_linesManager._route = (int16 *)g_PTRNUL;
		getSpriteX(0);
		getSpriteY(0);
	}
}

void ObjectsManager::PARADISE() {
	char result = _vm->_globals._saveData->_data[svField1];
	if (result && _vm->_globals._saveData->_data[svField2] && result != 4 && result > 3) {
		_vm->_fontManager.hideText(5);
		if (!_forestFl || _zoneNum < 20 || _zoneNum > 23) {
			if (_vm->_graphicsManager._largeScreenFl) {
				_vm->_graphicsManager._scrollStatus = 2;
				if (_vm->_eventsManager._startPos.x + 320 - getSpriteX(0) > 160) {
					bool loopCond = false;
					do {
						_vm->_graphicsManager._scrollPosX -= _vm->_graphicsManager._scrollSpeed;
						if (_vm->_graphicsManager._scrollPosX < 0) {
							_vm->_graphicsManager._scrollPosX = 0;
							loopCond = true;
						}
						if (_vm->_graphicsManager._scrollPosX > SCREEN_WIDTH) {
							_vm->_graphicsManager._scrollPosX = SCREEN_WIDTH;
							loopCond = true;
						}
						if (_vm->_eventsManager.getMouseX() > _vm->_graphicsManager._scrollPosX + 620)
							_vm->_eventsManager.setMouseXY(_vm->_eventsManager._mousePos.x - 4, _vm->_eventsManager.getMouseY());

						_vm->_eventsManager.VBL();
					} while (!loopCond && _vm->_eventsManager._startPos.x > getSpriteX(0) - 320);
				} else if (_vm->_eventsManager._startPos.x + 320 - getSpriteX(0) < -160) {
					bool loopCond = false;
					do {
						_vm->_graphicsManager._scrollPosX += _vm->_graphicsManager._scrollSpeed;
						if (_vm->_graphicsManager._scrollPosX < 0) {
							_vm->_graphicsManager._scrollPosX = 0;
							loopCond = true;
						}
						if (_vm->_graphicsManager._scrollPosX > SCREEN_WIDTH) {
							_vm->_graphicsManager._scrollPosX = SCREEN_WIDTH;
							loopCond = true;
						}
						if (_vm->_eventsManager.getMouseX() < _vm->_graphicsManager._scrollPosX + 10)
							_vm->_eventsManager.setMouseXY(_vm->_eventsManager._mousePos.x + 4, _vm->_eventsManager.getMouseY());

						_vm->_eventsManager.VBL();
					} while (!loopCond && _vm->_eventsManager._startPos.x < getSpriteX(0) - 320);
				}
				if (_vm->_eventsManager.getMouseX() > _vm->_graphicsManager._scrollPosX + 620)
					_vm->_eventsManager.setMouseXY(_vm->_graphicsManager._scrollPosX + 610, 0);
				if (_vm->_eventsManager.getMouseX() < _vm->_graphicsManager._scrollPosX + 10)
					_vm->_eventsManager.setMouseXY(_vm->_graphicsManager._scrollPosX + 10, 0);
				_vm->_eventsManager.VBL();
				_vm->_graphicsManager._scrollStatus = 0;
			}
			_vm->_talkManager.REPONSE(_vm->_globals._saveData->_data[svField2], _vm->_globals._saveData->_data[svField1]);
		} else {
			_vm->_talkManager.REPONSE2(_vm->_globals._saveData->_data[svField2], _vm->_globals._saveData->_data[svField1]);
		}
		_vm->_eventsManager.changeMouseCursor(4);
		if (_zoneNum != -1 && _zoneNum != 0 && !_vm->_globals.ZONEP[_zoneNum]._enabledFl) {
			_zoneNum = -1;
			_forceZoneFl = true;
		}
		if (_zoneNum != _vm->_globals._saveData->_data[svField2] || _zoneNum == -1 || _zoneNum == 0) {
			_vm->_eventsManager._mouseCursorId = 4;
			_changeVerbFl = false;
		} else {
			_vm->_eventsManager._mouseCursorId = _vm->_globals._saveData->_data[svField1];
			if (_changeVerbFl) {
				nextVerbIcon();
				_changeVerbFl = false;
			}
			if (_vm->_eventsManager._mouseCursorId == 5)
				_vm->_eventsManager._mouseCursorId = 4;
		}
		if (_vm->_eventsManager._mouseCursorId != 23)
			_vm->_eventsManager.changeMouseCursor(_vm->_eventsManager._mouseCursorId);
		_zoneNum = 0;
		_vm->_globals._saveData->_data[svField1] = 0;
		_vm->_globals._saveData->_data[svField2] = 0;
	}
	if (_vm->_globals._cityMapEnabledFl) {
		_vm->_eventsManager._mouseCursorId = 0;
		_vm->_eventsManager.changeMouseCursor(0);
	}
	if (_vm->_globals.NOMARCHE && _vm->_eventsManager._mouseCursorId == 4) {
		if (_zoneNum != -1 && _zoneNum != 0)
			handleRightButton();
	}
	_vm->_globals.GOACTION = false;
}

/**
 * Clear Screen
 */
void ObjectsManager::clearScreen() {
	clearSprite();
	_vm->_graphicsManager.FIN_VISU();
	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(9);
	_vm->_globals.CLEAR_VBOB();
	_vm->_animationManager.clearAnim();
	_vm->_linesManager.clearAllZones();
	_vm->_linesManager.resetLines();
	_vm->_globals.resetCache();

	for (int i = 0; i <= 48; i++) {
		_vm->_globals.BOBZONE[i] = 0;
		_vm->_globals.BOBZONE_FLAG[i] = false;
	}
	_vm->_eventsManager._mouseCursorId = 4;
	_verb = 4;
	_zoneNum = 0;
	_forceZoneFl = true;
	_vm->_linesManager.resetLinesNumb();
	_vm->_linesManager.resetLastLine();
	_vm->_linesManager._route = (int16 *)g_PTRNUL;
	_vm->_globals._answerBuffer = _vm->_globals.freeMemory(_vm->_globals._answerBuffer);
	_vm->_globals.SPRITE_ECRAN = _vm->_globals.freeMemory(_vm->_globals.SPRITE_ECRAN);
	_vm->_eventsManager._startPos.x = 0;
	_vm->_eventsManager._mouseSpriteId = 0;
	_vm->_globals._saveData->_data[svField1] = 0;
	_vm->_globals._saveData->_data[svField2] = 0;
	_vm->_globals.GOACTION = false;
	_forceZoneFl = true;
	_changeVerbFl = false;
	_vm->_linesManager._route = (int16 *)g_PTRNUL;
	_vm->_globals._oldDirection = -1;
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
}

/**
 * Change character Face / Head
 */
void ObjectsManager::changeCharacterHead(PlayerCharacter oldCharacter, PlayerCharacter newCharacter) {
	CharacterLocation *loc;

	_changeHeadFl = true;
	_vm->_graphicsManager.copySurface(_vm->_graphicsManager._vesaScreen, 532, 25, 65, 40, _vm->_graphicsManager._vesaBuffer, 532, 25);
	_vm->_graphicsManager.addVesaSegment(532, 25, 597, 65);
	_vm->_globals.NOT_VERIF = true;
	_vm->_linesManager._route = (int16 *)g_PTRNUL;

	if (oldCharacter == CHARACTER_SAMANTHA && newCharacter == CHARACTER_HOPKINS
		&& _vm->_globals._saveData->_realHopkins._location == _vm->_globals._screenId) {
		_changeHeadFl = false;
		loc = &_vm->_globals._saveData->_samantha;
		loc->_pos.x = getSpriteX(0);
		loc->_pos.y = getSpriteY(0);
		loc->_startSpriteIndex = 64;
		loc->_location = _vm->_globals._screenId;
		loc->_zoomFactor = _sprite[0]._animationType;

		removeSprite(1);
		addStaticSprite(_vm->_globals.TETE, loc->_pos, 1, 3, loc->_zoomFactor, false, 20, 127);
		animateSprite(1);
		removeSprite(0);

		_vm->_globals._saveData->_data[svField354] = 0;
		_vm->_globals._saveData->_data[svField356] = 0;
		_vm->_globals._saveData->_data[svField357] = 1;

		loc = &_vm->_globals._saveData->_realHopkins;
		_vm->_globals.PERSO = _vm->_fileManager.loadFile("PERSO.SPR");
		_vm->_globals.PERSO_TYPE = 0;
		addStaticSprite(_vm->_globals.PERSO, loc->_pos, 0, 64, loc->_zoomFactor, false, 34, 190);
		animateSprite(0);
		_vm->_globals.loadCharacterData();
	} else if (oldCharacter == CHARACTER_HOPKINS && newCharacter == CHARACTER_SAMANTHA
			&& _vm->_globals._saveData->_samantha._location == _vm->_globals._screenId) {
		_changeHeadFl = false;
		loc = &_vm->_globals._saveData->_realHopkins;
		loc->_pos.x = getSpriteX(0);
		loc->_pos.y = getSpriteY(0);
		loc->_startSpriteIndex = 64;
		loc->_location = _vm->_globals._screenId;
		loc->_zoomFactor = _sprite[0]._zoomFactor;

		removeSprite(1);
		addStaticSprite(_vm->_globals.TETE, loc->_pos, 1, 2, loc->_zoomFactor, false, 34, 190);
		animateSprite(1);
		removeSprite(0);

		_vm->_globals._saveData->_data[svField354] = 0;
		_vm->_globals._saveData->_data[svField356] = 1;
		_vm->_globals._saveData->_data[svField357] = 0;

		loc = &_vm->_globals._saveData->_samantha;
		_vm->_globals.PERSO = _vm->_fileManager.loadFile("PSAMAN.SPR");
		_vm->_globals.PERSO_TYPE = 2;
		addStaticSprite(_vm->_globals.PERSO, loc->_pos, 0, 64, loc->_zoomFactor, false, 20, 127);
		animateSprite(0);
		_vm->_globals.loadCharacterData();
	} else {
		switch (oldCharacter) {
		case CHARACTER_HOPKINS:
			loc = &_vm->_globals._saveData->_realHopkins;
			loc->_pos.x = getSpriteX(0);
			loc->_pos.y = getSpriteY(0);
			loc->_startSpriteIndex = 64;
			loc->_location = _vm->_globals._screenId;
			loc->_zoomFactor = _sprite[0]._zoomFactor;
			break;
		case CHARACTER_HOPKINS_CLONE:
			loc = &_vm->_globals._saveData->_cloneHopkins;
			loc->_pos.x = getSpriteX(0);
			loc->_pos.y = getSpriteY(0);
			loc->_startSpriteIndex = 64;
			loc->_location = _vm->_globals._screenId;
			loc->_zoomFactor = _sprite[0]._zoomFactor;
			break;
		case CHARACTER_SAMANTHA:
			loc = &_vm->_globals._saveData->_samantha;
			loc->_pos.x = getSpriteX(0);
			loc->_pos.y = getSpriteY(0);
			loc->_startSpriteIndex = 64;
			loc->_location = _vm->_globals._screenId;
			loc->_zoomFactor = _sprite[0]._zoomFactor;
			break;
		default:
			break;
		}

		switch (newCharacter) {
		case CHARACTER_HOPKINS:
			_vm->_globals._saveData->_data[svField121] = 0;
			_vm->_globals._saveData->_data[svField354] = 0;
			_vm->_globals._saveData->_data[svField356] = 0;
			_vm->_globals._saveData->_data[svField357] = 1;
			_vm->_globals._exitId = _vm->_globals._saveData->_realHopkins._location;
			break;
		case CHARACTER_HOPKINS_CLONE:
			_vm->_globals._saveData->_data[svField121] = 1;
			_vm->_globals._saveData->_data[svField354] = 1;
			_vm->_globals._saveData->_data[svField356] = 0;
			_vm->_globals._saveData->_data[svField357] = 0;
			_vm->_globals._exitId = _vm->_globals._saveData->_cloneHopkins._location;
			break;
		case CHARACTER_SAMANTHA:
			_vm->_globals._saveData->_data[svField121] = 0;
			_vm->_globals._saveData->_data[svField354] = 0;
			_vm->_globals._saveData->_data[svField356] = 1;
			_vm->_globals._saveData->_data[svField357] = 0;
			_vm->_globals._exitId = _vm->_globals._saveData->_samantha._location;
			break;
		}
	}
}

// Check Size
void ObjectsManager::computeAndSetSpriteSize() {
	int size = _vm->_globals._spriteSize[getSpriteY(0)];
	if (_vm->_globals.PERSO_TYPE == 1) {
		size = 20 * (5 * abs(size) - 100) / -80;
	} else if (_vm->_globals.PERSO_TYPE == 2) {
		size = 20 * (5 * abs(size) - 165) / -67;
	}
	setSpriteZoom(0, size);
}

/**
 * Get next verb icon (or text)
 */
void ObjectsManager::nextVerbIcon() {
	_vm->_eventsManager._mouseCursorId++;

	for(;;) {
		if (_vm->_eventsManager._mouseCursorId == 4) {
			if (!_vm->_globals.NOMARCHE || _zoneNum == -1 || _zoneNum == 0)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 5 || _vm->_eventsManager._mouseCursorId == 6) {
			_vm->_eventsManager._mouseCursorId = 6;
			if (_vm->_globals.ZONEP[_zoneNum].field6 == 1)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 7) {
			if (_vm->_globals.ZONEP[_zoneNum].field7 == 1)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 8) {
			if (_vm->_globals.ZONEP[_zoneNum].field8 == 1)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 9) {
			if (_vm->_globals.ZONEP[_zoneNum].field9 == 1)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 10) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldA == 1)
				return;
			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 11) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldB == 1)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 12) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldC == 1)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 13) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldD == 1)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 14) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldE == 1)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 15) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldF == 1)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 16) {
			if (_vm->_globals.ZONEP[_zoneNum].field6 == 2)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 17) {
			if (_vm->_globals.ZONEP[_zoneNum].field9 == 2)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 18) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldA == 2)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 19) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldB == 2)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 20) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldC == 2)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 21) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldF == 2)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 22) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldD == 2)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 23) {
			if (_vm->_globals.ZONEP[_zoneNum].field8 == 2)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 24) {
			if (_vm->_globals.ZONEP[_zoneNum].field9 == 3)
				return;

			++_vm->_eventsManager._mouseCursorId;
		}

		if (_vm->_eventsManager._mouseCursorId == 25) {
			if (_vm->_globals.ZONEP[_zoneNum].fieldE == 2)
				return;
		}
		_vm->_eventsManager._mouseCursorId = 4;
	}
}

/**
 * Handle Right button
 */
void ObjectsManager::handleRightButton() {
	if (_zoneNum != -1 && _zoneNum != 0) {
		nextVerbIcon();
		if (_vm->_eventsManager._mouseCursorId != 23)
			_vm->_eventsManager.changeMouseCursor(_vm->_eventsManager._mouseCursorId);
		_verb = _vm->_eventsManager._mouseCursorId;
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
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 158;
	else if (zoneIdx == 2 || zoneIdx == 8 || zoneIdx == 14 || zoneIdx == 20 || zoneIdx == 26)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 212;
	else if (zoneIdx == 3 || zoneIdx == 9 || zoneIdx == 15 || zoneIdx == 21 || zoneIdx == 27)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 266;
	else if (zoneIdx == 4 || zoneIdx == 10 || zoneIdx == 16 || zoneIdx == 22 || zoneIdx == 28)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 320;
	else if (zoneIdx == 5 || zoneIdx == 11 || zoneIdx == 17 || zoneIdx == 23 || zoneIdx == 29)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 374;
	else if (zoneIdx == 6 || zoneIdx == 12 || zoneIdx == 18 || zoneIdx == 24 || zoneIdx == 30 || zoneIdx == 31)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 428;

	if (zoneIdx >= 1 && zoneIdx <= 29)
		_borderSpriteIndex = 0;
	else if (zoneIdx == 30 || zoneIdx == 31)
		_borderSpriteIndex = 2;
	else if (!zoneIdx || zoneIdx == 32) {
		_borderPos = Common::Point(0, 0);
		_borderSpriteIndex = 0;
	}

	if (!zoneIdx)
		_vm->_eventsManager._mouseCursorId = 0;
	else if (zoneIdx == 32)
		_vm->_eventsManager._mouseCursorId = 16;
	else if (zoneIdx == 30)
		_vm->_eventsManager._mouseCursorId = 2;
	else if (zoneIdx == 31)
		_vm->_eventsManager._mouseCursorId = 3;
	else if (zoneIdx >= 1 && zoneIdx <= 28)
		_vm->_eventsManager._mouseCursorId = 8;
	else if (zoneIdx == 29)
		_vm->_eventsManager._mouseCursorId = 1;

	if (zoneIdx >= 1 && zoneIdx <= 28 && !_vm->_globals._inventory[zoneIdx]) {
		_vm->_eventsManager._mouseCursorId = 0;
		_borderPos = Common::Point(0, 0);
		_borderSpriteIndex = 0;
	}

	if (_vm->_eventsManager._mouseCursorId != 23)
		_vm->_eventsManager.changeMouseCursor(_vm->_eventsManager._mouseCursorId);
	_vm->_eventsManager.getMouseX();
	_vm->_eventsManager.getMouseY();
}

/**
 * Get next icon for an object in the inventory
 */
void ObjectsManager::nextObjectIcon(int idx) {
	if (_vm->_eventsManager._mouseCursorId == 0 || _vm->_eventsManager._mouseCursorId == 2 ||
	    _vm->_eventsManager._mouseCursorId == 3 || _vm->_eventsManager._mouseCursorId == 16)
		return;
		
	int nextCursorId = _vm->_eventsManager._mouseCursorId + 1;
	if (nextCursorId > 25)
		nextCursorId = 6;

	do {
		if (nextCursorId == 2 || nextCursorId == 5 || nextCursorId == 6) {
			_vm->_eventsManager._mouseCursorId = 6;
			if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field2 == 1)
				return;
			nextCursorId++;
		}
		if (nextCursorId == 7) {
			_vm->_eventsManager._mouseCursorId = 7;
			if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field3 == 1)
				return;
			nextCursorId++;
		}	
		if (nextCursorId == 8) {
			_vm->_eventsManager._mouseCursorId = 8;
			return;
		}
		if (nextCursorId == 9 || nextCursorId == 10) {
			_vm->_eventsManager._mouseCursorId = 10;
			if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field7 == 1)
				return;
			nextCursorId = 11;
		}

		if (nextCursorId == 11) {
			_vm->_eventsManager._mouseCursorId = 11;
			if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field4 == 1)
				return;
			nextCursorId++;
		}

		if (nextCursorId == 12 || nextCursorId == 13) {
			_vm->_eventsManager._mouseCursorId = 13;
			if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field5 == 1)
				return;
			nextCursorId = 14;
		}

		if (nextCursorId == 14 || nextCursorId == 15) {
			_vm->_eventsManager._mouseCursorId = 15;
			if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field6 == 1)
				return;
			nextCursorId = 23;
		}

		if (nextCursorId >= 16 && nextCursorId <= 23) {
			_vm->_eventsManager._mouseCursorId = 23;
			if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field6 == 2)
				return;
			nextCursorId = 24;
		}

		if (nextCursorId == 24 || nextCursorId == 25) {
			_vm->_eventsManager._mouseCursorId = 25;
		}
		
		nextCursorId = 6;
	} while (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field7 != 2);
}

void ObjectsManager::takeInventoryObject(int idx) {
	if (_vm->_eventsManager._mouseCursorId == 8)
		changeObject(idx);
}

void ObjectsManager::OPTI_OBJET() {
	byte *data;
	Common::String file;
	int lastOpcodeResult = 1;

	file = "OBJET1.ini";
	data = _vm->_fileManager.searchCat(file, 1);
	if (data == g_PTRNUL) {
		data = _vm->_fileManager.loadFile(file);
		if (data == g_PTRNUL)
			error("INI file %s not found", file.c_str());
	}

	if (READ_BE_UINT24(data) != MKTAG24('I', 'N', 'I'))
		error("File %s is not an INI file", file.c_str());

	for (;;) {
		int opcodeType = _vm->_scriptManager.handleOpcode(data + 20 * lastOpcodeResult);
		if (_vm->shouldQuit())
			return;

		if (opcodeType == 2)
			lastOpcodeResult = _vm->_scriptManager.handleGoto(data + 20 * lastOpcodeResult);
		else if (opcodeType == 3)
			lastOpcodeResult = _vm->_scriptManager.handleIf(data, lastOpcodeResult);

		if (lastOpcodeResult == -1)
			error("defective IFF function");

		if (opcodeType == 1 || opcodeType == 4)
			++lastOpcodeResult;
		if (!opcodeType || opcodeType == 5)
			break;
	}

	_vm->_globals.freeMemory(data);
}

void ObjectsManager::handleSpecialGames() {
	byte *oldPalette;

	switch (_vm->_globals._screenId) {
	case 5:
		if ((getSpriteY(0) > 399) || _vm->_globals._saveData->_data[svField173])
			break;

		_vm->_globals._saveData->_data[svField173] = 1;
		_vm->_globals._introSpeechOffFl = true;
		_vm->_talkManager.PARLER_PERSO("flicspe1.pe2");
		_vm->_globals._introSpeechOffFl = false;

		if (_vm->_globals._censorshipFl)
			break;

		oldPalette = _vm->_globals.allocMemory(1000);
		memcpy(oldPalette, _vm->_graphicsManager._palette, 769);

		_vm->_saveLoadManager.saveFile("TEMP1.SCR", _vm->_graphicsManager._vesaScreen, 307200);

		if (!_vm->_graphicsManager._lineNbr)
			_vm->_graphicsManager._scrollOffset = 0;
		_vm->_graphicsManager.NB_SCREEN(true);
		_vm->_soundManager._specialSoundNum = 198;
		PERSO_ON = true;
		_vm->_animationManager.NO_SEQ = true;
		_vm->_animationManager._clearAnimationFl = false;
		_vm->_animationManager.playAnim("otage.ANM", 1, 24, 500);
		_vm->_animationManager.NO_SEQ = false;
		_vm->_soundManager._specialSoundNum = 0;
		_vm->_graphicsManager.NB_SCREEN(false);

		_vm->_saveLoadManager.load("TEMP1.SCR", _vm->_graphicsManager._vesaScreen);
		g_system->getSavefileManager()->removeSavefile("TEMP1.SCR");

		PERSO_ON = false;
		memcpy(_vm->_graphicsManager._palette, oldPalette, 769);
		_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
		_vm->_globals.freeMemory(oldPalette);
		_vm->_graphicsManager.lockScreen();
		_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsManager.unlockScreen();
		memcpy(_vm->_graphicsManager._vesaBuffer, _vm->_graphicsManager._vesaScreen, 614399);

		_vm->_graphicsManager._scrollStatus = 0;
		_vm->_graphicsManager.DD_VBL();
		break;
	case 20:
		_vm->_globals._saveData->_data[svField132] = (getSpriteX(0) > 65 && getSpriteX(0) <= 124 && getSpriteY(0) > 372 && getSpriteY(0) <= 398) ? 1 : 0;
		break;
	case 35:
		if (_vm->_globals._prevScreenId == 16)
			handleForest(35, 500, 555, 100, 440, 1);
		else if (_vm->_globals._prevScreenId == 36)
			handleForest(35, 6, 84, 100, 440, 4);
		break;
	case 36:
		if (_vm->_globals._prevScreenId == 35)
			handleForest(36, 551, 633, 100, 440, 2);
		else if (_vm->_globals._prevScreenId == 37)
			handleForest(36, 6, 84, 100, 440, 4);
		break;
	case 37:
		if (_vm->_globals._prevScreenId == 36)
			handleForest(37, 551, 633, 100, 440, 1);
		else if (_vm->_globals._prevScreenId == 38)
			handleForest(37, 392, 529, 100, 440, 2);
		break;
	case 38:
		if (_vm->_globals._prevScreenId == 37)
			handleForest(38, 133, 252, 100, 440, 4);
		else if (_vm->_globals._prevScreenId == 39)
			handleForest(38, 6, 84, 100, 440, 3);
		break;
	case 39:
		if (_vm->_globals._prevScreenId == 38)
			handleForest(39, 551, 633, 100, 440, 2);
		else if (_vm->_globals._prevScreenId == 40)
			handleForest(39, 6, 84, 100, 440, 3);
		break;
	case 40:
		if (_vm->_globals._prevScreenId == 39)
			handleForest(40, 133, 252, 100, 440, 4);
		else if (_vm->_globals._prevScreenId == 41)
			handleForest(40, 392, 529, 100, 440, 2);
		break;
	case 41:
		if (_vm->_globals._prevScreenId == 40)
			handleForest(41, 551, 633, 100, 440, 1);
		else if (_vm->_globals._prevScreenId == 17)
			handleForest(41, 6, 84, 100, 440, 3);
		break;
	case 57:
		_vm->_globals._disableInventFl = true;
		if (_vm->_globals._saveData->_data[svField261] == 1 && getBobAnimDataIdx(5) == 37) {
			stopBobAnimation(5);
			setBobAnimDataIdx(5, 0);
			setBobAnimation(6);
			_vm->_globals._saveData->_data[svField261] = 2;
			disableZone(15);
			_vm->_soundManager.playSound("SOUND75.WAV");
		}
		if (_vm->_globals._saveData->_data[svField261] == 2 && getBobAnimDataIdx(6) == 6) {
			stopBobAnimation(6);
			setBobAnimDataIdx(6, 0);
			setBobAnimation(7);
			enableZone(14);
			_vm->_globals._saveData->_data[svField261] = 3;
		}
		_vm->_globals._disableInventFl = false;
		break;
	case 93:
		if (_vm->_globals._saveData->_data[svField333])
			break;

		_vm->_globals._disableInventFl = true;
		do
			_vm->_eventsManager.VBL();
		while (getBobAnimDataIdx(8) != 3);
		_vm->_globals._introSpeechOffFl = true;
		_vm->_talkManager.PARLER_PERSO("GM3.PE2");
		stopBobAnimation(8);
		_vm->_globals._saveData->_data[svField333] = 1;
		_vm->_globals._disableInventFl = false;
		break;
	}
}

void ObjectsManager::BOB_VIVANT(int idx) {
	int startPos = 10 * idx;
	if (!READ_LE_UINT16(_vm->_talkManager._characterAnim + startPos + 4))
		return;

	int xp = (int16)READ_LE_UINT16(_vm->_talkManager._characterAnim + startPos);
	int yp = (int16)READ_LE_UINT16(_vm->_talkManager._characterAnim + startPos + 2);
	int spriteIndex = _vm->_talkManager._characterAnim[startPos + 8];

	_vm->_graphicsManager.fastDisplay(_vm->_talkManager._characterSprite, xp, yp, spriteIndex);
}

void ObjectsManager::VBOB(byte *src, int idx, int xp, int yp, int frameIndex) {
	if (idx > 29)
		error("MAX_VBOB exceeded");

	if (_vm->_globals.VBob[idx].field4 <= 1) {
		_vm->_globals.VBob[idx].field4 = 1;
		_vm->_globals.VBob[idx]._xp = xp;
		_vm->_globals.VBob[idx]._yp = yp;
		_vm->_globals.VBob[idx]._frameIndex = frameIndex;
		_vm->_globals.VBob[idx]._oldX = xp;
		_vm->_globals.VBob[idx]._oldY = yp;
		_vm->_globals.VBob[idx]._oldFrameIndex = frameIndex;
		_vm->_globals.VBob[idx]._spriteData = src;
		_vm->_globals.VBob[idx]._oldSpriteData = src;
		_vm->_globals.VBob[idx]._surface = _vm->_globals.freeMemory(_vm->_globals.VBob[idx]._surface);
	} else if (_vm->_globals.VBob[idx].field4 == 2 || _vm->_globals.VBob[idx].field4 == 4) {
		_vm->_globals.VBob[idx].field4 = 3;
		_vm->_globals.VBob[idx]._oldX = _vm->_globals.VBob[idx]._xp;
		_vm->_globals.VBob[idx]._oldY = _vm->_globals.VBob[idx]._yp;
		_vm->_globals.VBob[idx]._oldSpriteData = _vm->_globals.VBob[idx]._spriteData;
		_vm->_globals.VBob[idx]._oldFrameIndex = _vm->_globals.VBob[idx]._frameIndex;
		_vm->_globals.VBob[idx]._xp = xp;
		_vm->_globals.VBob[idx]._yp = yp;
		_vm->_globals.VBob[idx]._frameIndex = frameIndex;
		_vm->_globals.VBob[idx]._spriteData = src;
	}
}

void ObjectsManager::VBOB_OFF(int idx) {
	if (idx > 29)
		error("MAX_VBOB exceeded");

	if (_vm->_globals.VBob[idx].field4 <= 1)
		_vm->_globals.VBob[idx].field4 = 0;
	else
		_vm->_globals.VBob[idx].field4 = 4;
}

void ObjectsManager::ACTION_DOS(int idx) {
	if (_vm->_globals.GESTE_FLAG != 1) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 1;
		_vm->_globals.GESTE = _vm->_fileManager.loadFile("DOS.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, false);
		break;
	case 2:
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,-1,", 0, 0, 8, false);
		break;
	case 3:
		SPACTION1(_vm->_globals.GESTE, "12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
		break;
	case 4:
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,8,9,10,11,12,13,12,11,12,13,12,11,12,13,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, false);
		break;
	case 5:
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,-1,", 0, 0, 8, false);
		break;
	case 6:
		SPACTION1(_vm->_globals.GESTE, "20,19,18,17,16,15,-1,", 0, 0, 8);
		break;
	case 7:
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,22,23,24,-1,", 0, 0, 8, false);
		break;
	case 8:
		SPACTION1(_vm->_globals.GESTE, "23,22,21,20,19,18,17,16,15,-1,", 0, 0, 8);
		break;
	case 9:
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,22,23,24,-1,", 0, 0, 8, false);
		break;
	case 10:
		SPACTION1(_vm->_globals.GESTE, "23,22,21,20,19,18,17,16,15,-1,", 0, 0, 8);
		break;
	}
}

void ObjectsManager::ACTION_DROITE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 3) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 3;
		_vm->_globals.GESTE = _vm->_fileManager.loadFile("PROFIL.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_vm->_globals.GESTE, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 0, 0, 8, false);
		break;
	case 2:
		SPACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,-1,", 0, 0, 8, false);
		break;
	case 3:
		SPACTION1(_vm->_globals.GESTE, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 0, 0, 8);
		break;
	case 4:
		ACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 0, 0, 8, false);
		break;
	case 5:
		SPACTION(_vm->_globals.GESTE, "23,24,25,-1,", 0, 0, 8, false);
		break;
	case 6:
		SPACTION1(_vm->_globals.GESTE, "24,,23,-1,", 0, 0, 8);
		break;
	case 7:
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,-1,", 0, 0, 8, false);
		break;
	case 8:
		SPACTION1(_vm->_globals.GESTE, "26,25,24,23,-1,", 0, 0, 8);
		break;
	case 9:
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,28,29,-1,", 0, 0, 8, false);
		break;
	case 10:
		SPACTION1(_vm->_globals.GESTE, "28,27,26,25,24,23,-1,", 0, 0, 8);
		break;
	}
}

void ObjectsManager::Q_DROITE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 4) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 4;
		_vm->_globals.GESTE = _vm->_fileManager.loadFile("3Q.SPR");
	}

	switch (idx) {
	case 1:
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, false);
		break;
	case 2:
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 8, false);
		break;
	case 3:
		SPACTION1(_vm->_globals.GESTE, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
		break;
	case 4:
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, false);
		break;
	case 5:
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,-1,", 0, 0, 8, false);
		break;
	case 6:
		SPACTION1(_vm->_globals.GESTE, "17,16,15,-1,", 0, 0, 8);
		break;
	case 7:
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20-1,", 0, 0, 8, false);
		break;
	case 8:
		SPACTION1(_vm->_globals.GESTE, "19,18,17,16,15,-1,", 0, 0, 8);
		break;
	case 9:
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,-1,", 0, 0, 8, false);
		break;
	case 10:
		SPACTION1(_vm->_globals.GESTE, "20,19,18,17,15,-1,", 0, 0, 8);
		break;
	}
}

void ObjectsManager::ACTION_FACE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 2) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 2;
		_vm->_globals.GESTE = _vm->_fileManager.loadFile("FACE.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,9,9,9,9,9,9,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, false);
		break;
	case 2:
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,15,-1,", 0, 0, 8, false);
		break;
	case 3:
		SPACTION1(_vm->_globals.GESTE, "14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
		break;
	case 4:
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, false);
		break;
	}
}

void ObjectsManager::Q_GAUCHE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 4) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 4;
		_vm->_globals.GESTE = _vm->_fileManager.loadFile("3Q.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, true);
		break;
	case 2:
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 8, true);
		break;
	case 3:
		SPACTION1(_vm->_globals.GESTE, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
		break;
	case 4:
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, true);
		break;
	case 5:
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,-1,", 0, 0, 8, true);
		break;
	case 6:
		SPACTION1(_vm->_globals.GESTE, "17,16,15,-1,", 0, 0, 8);
		break;
	case 7:
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,-1,", 0, 0, 8, true);
		break;
	case 8:
		SPACTION1(_vm->_globals.GESTE, "19,18,17,16,15,-1,", 0, 0, 8);
		break;
	case 9:
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,-1,", 0, 0, 8, true);
		break;
	case 10:
		SPACTION1(_vm->_globals.GESTE, "20,19,18,17,15,-1,", 0, 0, 8);
		break;
	}
}

void ObjectsManager::ACTION_GAUCHE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 3) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 3;
		_vm->_globals.GESTE = _vm->_fileManager.loadFile("PROFIL.SPR");
	}
	
	switch (idx) {
	case 1:
		ACTION(_vm->_globals.GESTE, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 0, 0, 8, true);
		break;
	case 2:
		SPACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,-1,", 0, 0, 8, true);
		break;
	case 3:
		SPACTION1(_vm->_globals.GESTE, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 0, 0, 8);
		break;
	case 4:
		ACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 0, 0, 8, true);
		break;
	case 5:
		SPACTION(_vm->_globals.GESTE, "23,24,25,-1,", 0, 0, 8, true);
		break;
	case 6:
		SPACTION1(_vm->_globals.GESTE, "24,,23,-1,", 0, 0, 8);
		break;
	case 7:
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,-1,", 0, 0, 8, true);
		break;
	case 8:
		SPACTION1(_vm->_globals.GESTE, "26,25,24,23,-1,", 0, 0, 8);
		break;
	case 9:
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,28,29,-1,", 0, 0, 8, true);
		break;
	case 10:
		SPACTION1(_vm->_globals.GESTE, "28,27,26,25,24,23,-1,", 0, 0, 8);
		break;
	}
}

void ObjectsManager::enableZone(int idx) {
	if (_vm->_globals.BOBZONE[idx]) {
		_vm->_globals.BOBZONE_FLAG[idx] = true;
	} else {
		_vm->_globals.ZONEP[idx]._enabledFl = true;
	}
}

void ObjectsManager::disableZone(int idx) {
	if (_vm->_globals.BOBZONE[idx]) {
		_vm->_globals.BOBZONE_FLAG[idx] = false;
	} else {
		_vm->_globals.ZONEP[idx]._enabledFl = false;
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
		_vm->_eventsManager.VBL();
	} while (destPosi != getBobAnimDataIdx(idx));

	if (!animAction)
		stopBobAnimation(idx);
	else if (animAction == 4) {
		_vm->_graphicsManager.fastDisplay(_bob[idx]._spriteData,
			_bob[idx]._oldX, _bob[idx]._oldY, _bob[idx]._frameIndex);
		stopBobAnimation(idx);
		_vm->_eventsManager.VBL();
	}
}

int ObjectsManager::getBobAnimDataIdx(int idx) {
	return _bob[idx]._animDataIdx / 5;
}

void ObjectsManager::setBobAnimDataIdx(int idx, int animIdx) {
	_bob[idx]._animDataIdx = 5 * animIdx;
	_bob[idx].field12 = 0;
	_bob[idx].field14 = 0;
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
	_bob[idx].field12 = 0;
	_bob[idx].field14 = 0;
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

void ObjectsManager::INILINK(const Common::String &file) {
	Common::File f;
	Common::String filename = file + ".LNK";
	byte *ptr = _vm->_fileManager.searchCat(filename, 3);
	size_t nbytes = _vm->_globals._catalogSize;
	if (ptr == g_PTRNUL) {
		if (!f.open(filename))
			error("Error opening file - %s", filename.c_str());

		nbytes = f.size();
		ptr = _vm->_globals.allocMemory(nbytes);
		if (g_PTRNUL == ptr)
			error("INILINK");
		_vm->_fileManager.readStream(f, ptr, nbytes);
		f.close();
	}
	if (!OBSSEUL) {
		for (int idx = 0; idx < 500; ++idx)
			_vm->_globals._spriteSize[idx] = (int16)READ_LE_UINT16((uint16 *)ptr + idx);

		_vm->_globals.resetCache();

		Common::String filename2 = Common::String((const char *)ptr + 1000);
		if (!filename2.empty()) {
			_vm->_globals.CACHE_BANQUE[1] = _vm->_fileManager.searchCat(filename2, 8);

			if (_vm->_globals.CACHE_BANQUE[1] || _vm->_globals.CACHE_BANQUE[1] == g_PTRNUL) {
				_vm->_globals.CACHE_BANQUE[1] = _vm->_fileManager.loadFile(filename2);
			} else {
				_vm->_globals.CACHE_BANQUE[1] = _vm->_fileManager.loadFile("RES_SLI.RES");
			}

			int curDataCacheId = 60;
			byte *curDataPtr = ptr + 1000;
			for (int cacheIdx = 0; cacheIdx <= 21; cacheIdx++) {
				int curSpriteId = (int16)READ_LE_UINT16(curDataPtr + 2 * curDataCacheId);
				_vm->_globals.Cache[cacheIdx]._spriteIndex = curSpriteId;
				_vm->_globals.Cache[cacheIdx]._x = (int16)READ_LE_UINT16(curDataPtr + 2 * curDataCacheId + 2);
				_vm->_globals.Cache[cacheIdx]._y = (int16)READ_LE_UINT16(curDataPtr + 2 * curDataCacheId + 4);
				_vm->_globals.Cache[cacheIdx].field14 = (int16)READ_LE_UINT16(curDataPtr + 2 * curDataCacheId + 8);

				if (!_vm->_globals.CACHE_BANQUE[1]) {
					_vm->_globals.Cache[cacheIdx]._useCount = 0;
				} else {
					_vm->_globals.Cache[cacheIdx]._spriteData = _vm->_globals.CACHE_BANQUE[1];
					_vm->_globals.Cache[cacheIdx]._width = getWidth(_vm->_globals.CACHE_BANQUE[1], curSpriteId);
					_vm->_globals.Cache[cacheIdx]._height = getHeight(_vm->_globals.CACHE_BANQUE[1], curSpriteId);
					_vm->_globals.Cache[cacheIdx]._useCount = 1;
				}
				if (!_vm->_globals.Cache[cacheIdx]._x && !_vm->_globals.Cache[cacheIdx]._y
							&& !_vm->_globals.Cache[cacheIdx]._spriteIndex)
					_vm->_globals.Cache[cacheIdx]._useCount = 0;

				curDataCacheId += 5;
			}
			_vm->_globals.CACHE_ON();
		}
	}

	_vm->_linesManager.resetLines();
	for (size_t idx = 0; idx < nbytes - 3; idx++) {
		if (READ_BE_UINT24(&ptr[idx]) == MKTAG24('O', 'B', '2')) {
			byte *curDataPtr = &ptr[idx + 4];
			int lineDataIdx = 0;
			int curLineIdx = 0;
			_vm->_linesManager.resetLinesNumb();
			int curDirection;
			do {
				curDirection = (int16)READ_LE_UINT16(curDataPtr + 2 * lineDataIdx);
				if (curDirection != -1) {
					_vm->_linesManager.addLine(
					    curLineIdx,
					    curDirection,
					    (int16)READ_LE_UINT16(curDataPtr + 2 * lineDataIdx + 2),
					    (int16)READ_LE_UINT16(curDataPtr + 2 * lineDataIdx + 4),
					    (int16)READ_LE_UINT16(curDataPtr + 2 * lineDataIdx + 6),
					    (int16)READ_LE_UINT16(curDataPtr + 2 * lineDataIdx + 8));
				}
				lineDataIdx += 5;
				++curLineIdx;
			} while (curDirection != -1);
			_vm->_linesManager.initRoute();
		}
	}

	if (!OBSSEUL) {
		for (size_t idx = 0; idx < nbytes - 3; idx++) {
			if (READ_BE_UINT24(&ptr[idx]) == MKTAG24('Z', 'O', '2')) {
				byte *curDataPtr = &ptr[idx + 4];
				int curDataIdx = 0;
				for (int i = 1; i <= 100; i++) {
					_vm->_globals.ZONEP[i]._destX = 0;
					_vm->_globals.ZONEP[i]._destY = 0;
					_vm->_globals.ZONEP[i]._spriteIndex = 0;
					_vm->_globals.ZONEP[i].field6 = 0;
					_vm->_globals.ZONEP[i].field7 = 0;
					_vm->_globals.ZONEP[i].field8 = 0;
					_vm->_globals.ZONEP[i].field9 = 0;
					_vm->_globals.ZONEP[i].fieldA = 0;
					_vm->_globals.ZONEP[i].fieldB = 0;
					_vm->_globals.ZONEP[i].fieldC = 0;
					_vm->_globals.ZONEP[i].fieldD = 0;
					_vm->_globals.ZONEP[i].fieldE = 0;
					_vm->_globals.ZONEP[i].fieldF = 0;
					_vm->_globals.ZONEP[i].field12 = 0;
				}

				int curLineIdx = 0;
				int v28;
				do {
					v28 = (int16)READ_LE_UINT16(curDataPtr + 2 * curDataIdx);
					if (v28 != -1) {
						_vm->_linesManager.addZoneLine(
						    curLineIdx,
						    (int16)READ_LE_UINT16(curDataPtr + 2 * curDataIdx + 2),
						    (int16)READ_LE_UINT16(curDataPtr + 2 * curDataIdx + 4),
						    (int16)READ_LE_UINT16(curDataPtr + 2 * curDataIdx + 6),
						    (int16)READ_LE_UINT16(curDataPtr + 2 * curDataIdx + 8),
						    v28);
						_vm->_globals.ZONEP[v28]._enabledFl = true;
					}
					curDataIdx += 5;
					++curLineIdx;
				} while (v28 != -1);
				for (int i = 1; i <= 100; i++) {
					_vm->_globals.ZONEP[i]._destX = (int16)READ_LE_UINT16(curDataPtr + 2 * curDataIdx);
					_vm->_globals.ZONEP[i]._destY = (int16)READ_LE_UINT16(curDataPtr + 2 * curDataIdx + 2);
					_vm->_globals.ZONEP[i]._spriteIndex = (int16)READ_LE_UINT16(curDataPtr + 2 * curDataIdx + 4);
					curDataIdx += 3;
				}

				byte *v22 = ptr + idx + (10 * curLineIdx + 606) + 4;
				for (int i = 1; i <= 100; i++) {
					int j = (i - 1) * 10;
					_vm->_globals.ZONEP[i].field6 = v22[j];
					_vm->_globals.ZONEP[i].field7 = v22[j + 1];
					_vm->_globals.ZONEP[i].field8 = v22[j + 2];
					_vm->_globals.ZONEP[i].field9 = v22[j + 3];
					_vm->_globals.ZONEP[i].fieldA = v22[j + 4];
					_vm->_globals.ZONEP[i].fieldB = v22[j + 5];
					_vm->_globals.ZONEP[i].fieldC = v22[j + 6];
					_vm->_globals.ZONEP[i].fieldD = v22[j + 7];
					_vm->_globals.ZONEP[i].fieldE = v22[j + 8];
					_vm->_globals.ZONEP[i].fieldF = v22[j + 9];
				}
				int dep = 1010;
				for (int i = 1; i <= 100; i++) {
					_vm->_globals.ZONEP[i].field12 = (int16)READ_LE_UINT16(v22 + dep);
					dep += 2;
				}
				_vm->_linesManager.CARRE_ZONE();
			}
		}
	}
	_vm->_globals.freeMemory(ptr);
}

void ObjectsManager::SPECIAL_INI() {
	switch (_vm->_globals._screenId) {
	case 17:
		if (_vm->_globals._prevScreenId == 20) {
			_vm->_globals._disableInventFl = true;
			_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
			for (int i = 0; i <= 4; i++)
				_vm->_eventsManager.VBL();
			_vm->_graphicsManager.fadeInLong();
			animateSprite(0);
			for (int i = 0; i <= 4; i++)
				_vm->_eventsManager.VBL();
			VBOB(_vm->_globals.SPRITE_ECRAN, 5, 15, 28, 1);
			_vm->_fontManager.hideText(9);
			if (!_vm->_soundManager._textOffFl) {
				_vm->_fontManager.initTextBuffers(9, 383, _vm->_globals.FICH_TEXTE, 220, 72, 6, 36, 253);
				if (!_vm->_soundManager._textOffFl)
					_vm->_fontManager.showText(9);
			}
			if (!_vm->_soundManager._voiceOffFl)
				_vm->_soundManager.mixVoice(383, 4);
			_vm->_globals._saveData->_data[svField270] = 1;
			_vm->_globals._saveData->_data[svField300] = 1;
			_vm->_globals._saveData->_data[svField320] = 1;
			if (_vm->_soundManager._voiceOffFl) {
				for (int i = 0; i <= 199; i++)
					_vm->_eventsManager.VBL();
			}
			_vm->_fontManager.hideText(9);
			VBOB_OFF(5);
			for (int i = 0; i <= 3; i++)
				_vm->_eventsManager.VBL();
			_vm->_graphicsManager._noFadingFl = true;
			_vm->_globals._disableInventFl = false;
		}
		break;

	case 18:
		if (_vm->_globals._prevScreenId == 17) {
			_vm->_eventsManager._mouseSpriteId = 4;
			for (int i = 0; i <= 4; i++)
				_vm->_eventsManager.VBL();
			_vm->_graphicsManager.fadeInLong();
			_vm->_globals.iRegul = 1;
			_vm->_globals._disableInventFl = false;
			_vm->_graphicsManager._noFadingFl = true;
			_vm->_globals._introSpeechOffFl = true;
			_vm->_talkManager.PARLER_PERSO("MAGE1.pe2");
			_vm->_graphicsManager._noFadingFl = true;
			_vm->_globals._disableInventFl = false;
		}
		break;

	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
	case 41:
		_vm->_globals.BOBZONE[20] = 1;
		_vm->_globals.BOBZONE[21] = 2;
		_vm->_globals.BOBZONE[22] = 3;
		_vm->_globals.BOBZONE[23] = 4;
		_vm->_globals.BOBZONE_FLAG[20] = true;
		_vm->_globals.BOBZONE_FLAG[21] = true;
		_vm->_globals.BOBZONE_FLAG[22] = true;
		_vm->_globals.BOBZONE_FLAG[23] = true;
		enableVerb(20, 5);
		enableVerb(21, 5);
		enableVerb(22, 5);
		enableVerb(23, 5);
		_vm->_globals.ZONEP[20].field12 = 30;
		_vm->_globals.ZONEP[21].field12 = 30;
		_vm->_globals.ZONEP[22].field12 = 30;
		_vm->_globals.ZONEP[23].field12 = 30;
		for (int i = 200; i <= 214; i++) {
			if (_vm->_globals._saveData->_data[i] != 2)
				_vm->_globals._saveData->_data[i] = 0;
		}
		break;

	case 73:
		if (!_vm->_globals._saveData->_data[svField318]) {
			_vm->_globals.CACHE_SUB(0);
			_vm->_globals.CACHE_SUB(1);
		}
		break;

	case 93:
		if (!_vm->_globals._saveData->_data[svField333])
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

void ObjectsManager::SCI_OPTI_ONE(int idx, int animIdx, int a3, int a4) {
	_vm->_eventsManager._curMouseButton = 0;
	_vm->_eventsManager._mouseButton = 0;

	if (a4 != 3) {
		setBobAnimation(idx);
		setBobAnimDataIdx(idx, animIdx);
	}

	do {
		_vm->_eventsManager.VBL();
		if (_vm->_eventsManager._curMouseButton)
			break;
	} while (a3 != getBobAnimDataIdx(idx));
	if (!a4)
		stopBobAnimation(idx);
}

void ObjectsManager::disableVerb(int idx, int a2) {
	switch (a2) {
	case 6:
	case 16:
		_vm->_globals.ZONEP[idx].field6 = 0;
		break;
	case 7:
		_vm->_globals.ZONEP[idx].field7 = 0;
		break;
	case 5:
	case 8:
		_vm->_globals.ZONEP[idx].field8 = 0;
		break;
	case 9:
	case 17:
	case 24:
		_vm->_globals.ZONEP[idx].field9 = 0;
		break;
	case 10:
	case 18:
		_vm->_globals.ZONEP[idx].fieldA = 0;
		break;
	case 11:
	case 19:
		_vm->_globals.ZONEP[idx].fieldB = 0;
		break;
	case 12:
	case 20:
		_vm->_globals.ZONEP[idx].fieldC = 0;
		break;
	case 13:
	case 22:
		_vm->_globals.ZONEP[idx].fieldD = 0;
	case 14:
	case 21:
	case 25:
		_vm->_globals.ZONEP[idx].fieldE = 0;
		break;
	case 15:
		_vm->_globals.ZONEP[idx].fieldF = 0;
		break;
	}
	_changeVerbFl = true;
}

void ObjectsManager::enableVerb(int idx, int a2) {
	switch (a2) {
	case 5:
		_vm->_globals.ZONEP[idx].field8 = 2;
		break;
	case 6:
		_vm->_globals.ZONEP[idx].field6 = 1;
		break;
	case 7:
		_vm->_globals.ZONEP[idx].field7 = 1;
		break;
	case 8:
		_vm->_globals.ZONEP[idx].field8 = 1;
		break;
	case 9:
		_vm->_globals.ZONEP[idx].field9 = 1;
		break;
	case 10:
		_vm->_globals.ZONEP[idx].fieldA = 1;
		break;
	case 11:
		_vm->_globals.ZONEP[idx].fieldB = 1;
		break;
	case 12:
		_vm->_globals.ZONEP[idx].fieldC = 1;
		break;
	case 13:
		_vm->_globals.ZONEP[idx].fieldD = 1;
		break;
	case 14:
		_vm->_globals.ZONEP[idx].fieldD = 1;
		break;
	case 15:
		_vm->_globals.ZONEP[idx].fieldE = 1;
		break;
	case 16:
		_vm->_globals.ZONEP[idx].field6 = 2;
		break;
	case 17:
		_vm->_globals.ZONEP[idx].field9 = 2;
		break;
	case 18:
		_vm->_globals.ZONEP[idx].fieldA = 2;
		break;
	case 19:
		_vm->_globals.ZONEP[idx].fieldB = 2;
		break;
	case 20:
		_vm->_globals.ZONEP[idx].fieldC = 2;
		break;
	case 21:
		_vm->_globals.ZONEP[idx].fieldE = 2;
		break;
	case 22:
		_vm->_globals.ZONEP[idx].fieldD = 2;
		break;
	case 24:
		_vm->_globals.ZONEP[idx].field9 = 3;
		break;
	case 25:
		_vm->_globals.ZONEP[idx].fieldE = 2;
		break;
	}
}

void ObjectsManager::ACTION(const byte *spriteData, const Common::String &actionStr, int a3, int a4, int speed, bool flipFl) {
	Common::String tmpStr = "";
	int realSpeed = speed;
	if (_vm->_globals._speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals._speed == 3)
		realSpeed = speed / 3;
	const byte *oldSpriteData = _sprite[0]._spriteData;
	int spriteIndex = _sprite[0]._spriteIndex;
	bool oldFlipFl = _sprite[0]._flipFl;
	_sprite[0].field12 += a3;
	_sprite[0].field14 += a4;
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
				_sprite[0].field12 -= a3;
				_sprite[0].field14 -= a4;
				_sprite[0]._flipFl = oldFlipFl;
			} else {
				_sprite[0]._spriteData = spriteData;
				_sprite[0]._spriteIndex = idx;
			}
			for (int i = 0; i < realSpeed; i++)
				_vm->_eventsManager.VBL();
			if (idx == -1)
				break;
		}
	}
}

void ObjectsManager::SPACTION(byte *spriteData, const Common::String &animationSeq, int a3, int a4, int speed, bool flipFl) {
	Common::String tmpStr = "";

	int realSpeed = speed;
	if (_vm->_globals._speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals._speed == 3)
		realSpeed = speed / 3;

	_oldSpriteData = _sprite[0]._spriteData;
	_oldSpriteIndex = _sprite[0]._spriteIndex;
	_oldFlipFl = _sprite[0]._flipFl;
	_sprite[0].field12 += a3;
	_sprite[0].field14 += a4;
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
			_vm->_eventsManager.VBL();
	} while (spriteIndex != -1);
}

void ObjectsManager::SPACTION1(byte *spriteData, const Common::String &animString, int a3, int a4, int speed) {
	Common::String tmpStr = "";
	int realSpeed = speed;
	if (_vm->_globals._speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals._speed == 3)
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
				_sprite[0].field12 -= a3;
				_sprite[0].field14 -= a4;
				_sprite[0]._flipFl = _oldFlipFl;
			} else {
				_sprite[0]._spriteData = spriteData;
				_sprite[0]._spriteIndex = spriteIndex;
			}

			for (int i = 0; i < realSpeed; i++)
				_vm->_eventsManager.VBL();

			if (spriteIndex == -1)
				break;
		}
	}
}

void ObjectsManager::handleForest(int screenId, int minX, int maxX, int minY, int maxY, int idx) {
	int savegameIdx = screenId;
	if (_vm->_globals._screenId != screenId)
		return;

	switch (_vm->_globals._screenId) {
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

	if (_vm->_globals._saveData->_data[savegameIdx] == 2)
		return;

	if (_vm->_globals._saveData->_data[savegameIdx]) {
		if (_vm->_globals._saveData->_data[savegameIdx] == 1) {
			if (((idx == 1 || idx == 2) && getBobAnimDataIdx(idx) == 26) || ((idx == 3 || idx == 4) && getBobAnimDataIdx(idx) == 27)) {
				_vm->_dialogsManager._removeInventFl = true;
				_vm->_soundManager.PLAY_SAMPLE2(1);
				_vm->_globals._saveData->_data[savegameIdx] = 4;
			}
		}
		if (_vm->_globals._saveData->_data[savegameIdx] == 4) {
			if (idx >= 1 && idx <= 4 && getBobAnimDataIdx(idx) > 30)
				_vm->_globals._saveData->_data[savegameIdx] = 3;
		}
		if (_vm->_globals._saveData->_data[savegameIdx] == 3) {
			_vm->_graphicsManager.FADE_LINUX = 2;
			_vm->_animationManager.playAnim("CREVE2.ANM", 100, 24, 500);
			_vm->_globals._exitId = 150;
			_vm->_graphicsManager._noFadingFl = true;
			BOB_OFF(1);
			BOB_OFF(2);
			BOB_OFF(3);
			BOB_OFF(4);
		}
	} else if (minX < getSpriteX(0)
	           && maxX > getSpriteX(0)
	           && minY < getSpriteY(0)
	           && maxY > getSpriteY(0)) {
		if (idx >= 1 && idx <= 4)
			setBobAnimation(idx);
		_vm->_globals._saveData->_data[savegameIdx] = 1;
	}
}

void ObjectsManager::lockAnimX(int idx, int x) {
	_vm->_globals._lockedAnims[idx]._enableFl = true;
	_vm->_globals._lockedAnims[idx]._posX = x;
}

/**
 * Game scene control method
 */
void ObjectsManager::PERSONAGE(const Common::String &backgroundFile, const Common::String &linkFile,
							   const Common::String &animFile, const Common::String &s4, int v, bool initializeScreen) {
	_vm->_dialogsManager._inventFl = false;
	_vm->_eventsManager._gameKey = KEY_NONE;
	_vm->_dialogsManager._removeInventFl = false;
	_vm->_graphicsManager._scrollOffset = 0;
	_vm->_globals._cityMapEnabledFl = false;
	_vm->_globals.iRegul = 1;
	_vm->_soundManager.WSOUND(v);
	_vm->_linesManager._route = (int16 *)g_PTRNUL;
	_vm->_globals.NOMARCHE = true;
	_vm->_globals._exitId = 0;
	if (!backgroundFile.empty())
		_vm->_graphicsManager.loadImage(backgroundFile);
	if (!linkFile.empty())
		INILINK(linkFile);
	if (!animFile.empty())
		_vm->_animationManager.loadAnim(animFile);
	_vm->_graphicsManager.VISU_ALL();
	if (!s4.empty()) {
		if (initializeScreen)
			_vm->_graphicsManager.INI_ECRAN(s4, initializeScreen);
		else
			_vm->_graphicsManager.INI_ECRAN2(s4, initializeScreen);
	}
	_vm->_eventsManager.mouseOn();
	if (_vm->_globals._screenId == 61) {
		addStaticSprite(_vm->_globals.PERSO, Common::Point(330, 418), 0, 60, 0, false, 34, 190);
		animateSprite(0);
		_vm->_linesManager._route = (int16 *)g_PTRNUL;
		computeAndSetSpriteSize();
	}
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_vm->_eventsManager.changeMouseCursor(4);
	for (int i = 0; i <= 4; i++)
		_vm->_eventsManager.VBL();
	_vm->_graphicsManager.fadeInLong();
	if (_vm->_globals._screenId == 61) {
		_vm->_animationManager.playSequence("OUVRE.SEQ", 10, 4, 10);
		stopBobAnimation(3);
		_vm->_globals.NOT_VERIF = true;
		_oldCharacterPosX = getSpriteX(0);
		_vm->_globals._oldDirection = -1;
		_vm->_globals.Compteur = 0;
		_vm->_linesManager._route = (int16 *)g_PTRNUL;
		_vm->_linesManager._route = _vm->_linesManager.PARCOURS2(getSpriteX(0), getSpriteY(0), 330, 345);
		_vm->_globals.NOT_VERIF = true;
		do {
			GOHOME();
			_vm->_eventsManager.VBL();
		} while (_vm->_linesManager._route != (int16 *)g_PTRNUL);
		setSpriteIndex(0, 64);
	}
	do {
		int mouseButton = _vm->_eventsManager.getMouseButton();
		if (mouseButton == 1) {
			handleLeftButton();
			mouseButton = 1;
		} else if (mouseButton == 2)
			handleRightButton();
		_vm->_dialogsManager.testDialogOpening();
		checkZone();
		if (_vm->_globals.GOACTION)
			PARADISE();
		if (!_vm->_globals._exitId)
			_vm->_eventsManager.VBL();

		if (_vm->_globals._exitId)
			break;
	} while (!_vm->shouldQuit());
	if (_vm->shouldQuit())
		return;

	_vm->_graphicsManager.fadeOutLong();
	if (!animFile.empty())
		_vm->_graphicsManager.FIN_VISU();
	if (_vm->_globals._screenId == 61)
		removeSprite(0);
	clearScreen();
	_vm->_globals.iRegul = 0;
}

/**
 * Game scene control method
 */
void ObjectsManager::PERSONAGE2(const Common::String &backgroundFile, const Common::String &linkFile,
								const Common::String &animFile, const Common::String &s4, int v, bool initializeScreen) {
	_vm->_dialogsManager._inventFl = false;
	_vm->_eventsManager._gameKey = KEY_NONE;
	_verb = 4;
	_vm->_graphicsManager._scrollOffset = 0;
	_vm->_dialogsManager._removeInventFl = false;
	_vm->_globals._cityMapEnabledFl = false;
	_vm->_graphicsManager._noFadingFl = false;
	_vm->_globals.NOMARCHE = false;
	_vm->_globals._exitId = 0;
	_vm->_globals.NOT_VERIF = true;
	_vm->_soundManager.WSOUND(v);
	_vm->_globals.iRegul = 1;
	if (!backgroundFile.empty())
		_vm->_graphicsManager.loadImage(backgroundFile);
	if (!linkFile.empty())
		INILINK(linkFile);
	if (!animFile.empty()) {
		_vm->_animationManager.loadAnim(animFile);
		_vm->_graphicsManager.VISU_ALL();
	}
	if (!s4.empty()) {
		if (initializeScreen)
			_vm->_graphicsManager.INI_ECRAN(s4, initializeScreen);
		else
			_vm->_graphicsManager.INI_ECRAN2(s4, initializeScreen);
	}
	_vm->_eventsManager.mouseOn();
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	if (_vm->_globals.PERSO_TYPE) {
		if (!_vm->_globals._saveData->_data[svField122] && !_vm->_globals._saveData->_data[svField356]) {
			_vm->_globals.PERSO = _vm->_fileManager.loadFile("PERSO.SPR");
			_vm->_globals.PERSO_TYPE = 0;
		}
	}
	if (!_vm->_globals.PERSO_TYPE) {
		if (_vm->_globals._saveData->_data[svField122] == 1) {
			_vm->_globals.PERSO = _vm->_fileManager.loadFile("HOPFEM.SPR");
			_vm->_globals.PERSO_TYPE = 1;
		}
	}
	if (_vm->_globals.PERSO_TYPE != 2 && _vm->_globals._saveData->_data[svField356] == 1) {
		_vm->_globals.PERSO = _vm->_fileManager.loadFile("PSAMAN.SPR");
		_vm->_globals.PERSO_TYPE = 2;
	}
	_vm->_globals.loadCharacterData();
	switch (_vm->_globals.PERSO_TYPE) {
	case 0:
		addStaticSprite(_vm->_globals.PERSO, _characterPos, 0, _startSpriteIndex, 0, false, 34, 190);
		break;
	case 1:
		addStaticSprite(_vm->_globals.PERSO, _characterPos, 0, _startSpriteIndex, 0, false, 28, 155);
		break;
	case 2:
		addStaticSprite(_vm->_globals.PERSO, _characterPos, 0, _startSpriteIndex, 0, false, 20, 127);
		break;
	}
	_vm->_eventsManager.setMouseXY(_characterPos);
	if (_vm->_graphicsManager._largeScreenFl)
		_vm->_graphicsManager._scrollPosX = (int16)getSpriteX(0) - 320;
	computeAndSetSpriteSize();
	animateSprite(0);
	_vm->_globals.CACHE_ON();
	_vm->_linesManager._route = (int16 *)g_PTRNUL;
	computeAndSetSpriteSize();
	SPECIAL_INI();
	_vm->_eventsManager._mouseSpriteId = 4;
	_oldCharacterPosX = _characterPos.x;
	_oldCharacterPosY = _characterPos.y;
	_vm->_globals._oldDirection = -1;
	_vm->_globals.Compteur = 0;

	for (int idx = 0; idx < 5; ++idx)
		_vm->_eventsManager.VBL();

	_vm->_globals.iRegul = 1;
	if (!_vm->_graphicsManager._noFadingFl)
		_vm->_graphicsManager.fadeInLong();
	_vm->_graphicsManager._noFadingFl = false;
	_vm->_eventsManager.changeMouseCursor(4);

	int xCheck = 0;
	int yCheck = 0;

	bool breakFlag = false;
	while (!_vm->shouldQuit() && !breakFlag) {
		int mouseButtons = _vm->_eventsManager.getMouseButton();
		if (mouseButtons) {
			if (mouseButtons == 1) {
				if (_verb == 16 && _vm->_eventsManager._mouseCursorId == 16) {
					int xp = _vm->_eventsManager.getMouseX();
					int yp = _vm->_eventsManager.getMouseY();

					if ((xCheck == xp) && (yCheck == yp)) {
						_vm->_linesManager._route = (int16 *)g_PTRNUL;
						PARADISE();
						if (_vm->_globals._exitId)
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
		if (!_vm->_globals._exitId) {
			_vm->_dialogsManager.testDialogOpening();
			checkZone();
			if (_vm->_linesManager._route == (int16 *)g_PTRNUL
					|| (GOHOME(), _vm->_linesManager._route == (int16 *)g_PTRNUL)) {
				if (_vm->_globals.GOACTION)
					PARADISE();
			}
			handleSpecialGames();
			_vm->_eventsManager.VBL();
			if (!_vm->_globals._exitId)
				continue;
		}
		breakFlag = true;
	}

	if (_vm->_globals._exitId != 8 || _vm->_globals._screenId != 5 || !_helicopterFl) {
		if (!_vm->_graphicsManager._noFadingFl)
			_vm->_graphicsManager.fadeOutLong();
		_vm->_graphicsManager._noFadingFl = false;
		removeSprite(0);
		if (_twoCharactersFl) {
			removeSprite(1);
			_twoCharactersFl = false;
		}
		if (!animFile.empty())
			_vm->_graphicsManager.FIN_VISU();
		clearScreen();
	} else {
		_helicopterFl = false;
	}
	_vm->_globals.iRegul = 0;
}

} // End of namespace Hopkins
