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

	_priorityFl = false;
	_oldBorderPos = Common::Point(0, 0);
	_oldBorderSpriteIndex = 0;
	_borderPos = Common::Point(0, 0);
	_borderSpriteIndex = 0;
	_saveLoadX = _saveLoadY = 0;
	I_old_x = I_old_y = 0;
	g_old_x = g_old_y = 0;
	FLAG_VISIBLE_EFFACE = 0;
	SL_SPR = g_PTRNUL;
	SL_SPR2 = g_PTRNUL;
	_spritePtr = g_PTRNUL;
	S_old_spr = g_PTRNUL;
	PERSO_ON = false;
	_saveLoadFl = false;
	SL_MODE = false;
	_visibleFl = false;
	_disableCursorFl = false;
	BOBTOUS = false;
	my_anim = 0;
	NUMZONE = 0;
	_forceZoneFl = false;
	_changeVerbFl = false;
	_verb = 0;
	Vold_taille = 0;
	SPEED_X = SPEED_Y = 0;
	SPEED_IMAGE = 0;
	SPEED_PTR = g_PTRNUL;
	_lastLine = 0;
	A_ANIM = 0;
	MA_ANIM = 0;
	MA_ANIM1 = 0;
	A_DEPA = 0;
	MAX_DEPA = 0;
	MAX_DEPA1 = 0;
	CH_TETE = false;
	T_RECTIF = 0;
	_disableFl = false;
	_twoCharactersFl = false;
	_characterPos = Common::Point(0, 0);
	PERI = 0;
	RECALL = 0;
	PTAILLE = 0;
	PEROFX = 0;
	PEROFY = 0;
	OBSSEUL = 0;
	NVVERBE = 0;
	NVZONE = 0;
	S_old_ani = 0;
	S_old_ret = 0;
	nouveau_x = nouveau_y = 0;
	nouveau_sens = 0;
	nouveau_anim = 0;
}

void ObjectsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

/**
 * Change Object
 */
void ObjectsManager::changeObject(int objIndex) {
	_vm->_eventsManager._objectBuf = CAPTURE_OBJET(objIndex, 1);
	_vm->_globals._newObjectFl = true;
	_vm->_globals._curObjectIndex = objIndex;
}

byte *ObjectsManager::CAPTURE_OBJET(int objIndex, int mode) {
	byte *dataP = NULL;
	int val1 = _vm->_globals.ObjetW[objIndex].field0;
	int val2 = _vm->_globals.ObjetW[objIndex]._idx;

	if (mode == 1)
		++val2;

	if (val1 != _vm->_globals.NUM_FICHIER_OBJ) {
		if (_vm->_globals.ADR_FICHIER_OBJ != g_PTRNUL)
			ObjectsManager::DEL_FICHIER_OBJ();
		if (val1 == 1) {
			_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "OBJET1.SPR");
			_vm->_globals.ADR_FICHIER_OBJ = ObjectsManager::loadSprite(_vm->_globals._curFilename);
		}
		_vm->_globals.NUM_FICHIER_OBJ = val1;
	}

	int width = ObjectsManager::getWidth(_vm->_globals.ADR_FICHIER_OBJ, val2);
	int height = ObjectsManager::getHeight(_vm->_globals.ADR_FICHIER_OBJ, val2);
	_vm->_globals._objectWidth = width;
	_vm->_globals._objectHeight = height;

	switch (mode) {
	case 0:
		dataP = _vm->_globals.allocMemory(height * width);
		if (dataP == g_PTRNUL)
			error("CAPTURE_OBJET");

		capture_mem_sprite(_vm->_globals.ADR_FICHIER_OBJ, dataP, val2);
		break;

	case 1:
		sprite_alone(_vm->_globals.ADR_FICHIER_OBJ, _vm->_eventsManager._objectBuf, val2);
		dataP = _vm->_eventsManager._objectBuf;
		break;

	case 3:
		capture_mem_sprite(_vm->_globals.ADR_FICHIER_OBJ, _vm->_globals._inventoryObject, val2);
		dataP = _vm->_globals._inventoryObject;
		break;

	default:
		break;
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

int ObjectsManager::sprite_alone(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = (int16)READ_LE_UINT16(objP) * (int16)READ_LE_UINT16(objP + 2);

	memcpy(sprite + 3, objP - 4, result + 16);
	return result;
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

int ObjectsManager::capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex) {
	const byte *objP = objectData + 3;
	for (int i = objIndex; i; --i) {
		objP += READ_LE_UINT32(objP) + 16;
	}

	objP += 4;
	int result = (int16)READ_LE_UINT16(objP) * (int16)READ_LE_UINT16(objP + 2);

	memcpy(sprite, objP + 12, result);
	return result;
}

/**
 * Add Object
 */
int ObjectsManager::addObject(int objIndex) {
	bool flag = false;
	int arrIndex = 0;
	do {
		++arrIndex;
		if (!_vm->_globals._inventory[arrIndex])
			flag = true;
		if (arrIndex == 32)
			flag = true;
	} while (!flag);

	_vm->_globals._inventory[arrIndex] = objIndex;
	return arrIndex;
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

			if (clipX < _vm->_graphicsManager.min_x)
				clipX = _vm->_graphicsManager.min_x;
	
			clipY = _vm->_fontManager._textList[idx]._pos.y - 2;
			if (clipY < _vm->_graphicsManager.min_y)
				clipY = _vm->_graphicsManager.min_y;

			_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen, clipX, clipY,
				_vm->_fontManager._textList[idx]._width + 4, _vm->_fontManager._textList[idx]._height + 4,
				_vm->_graphicsManager._vesaBuffer, clipX, clipY);
			_vm->_fontManager._textList[idx]._enabledFl = false;
		}
	}

	if (!PERSO_ON) {
		for (int idx = 0; idx < MAX_SPRITE; ++idx) {
			if (_vm->_globals.Liste[idx].field0) {
				clipX = _vm->_globals.Liste[idx].field2 - 2;
				if (clipX < _vm->_graphicsManager.min_x)
					clipX = _vm->_graphicsManager.min_x;

				clipY = _vm->_globals.Liste[idx].field4 - 2;
				if (clipY < _vm->_graphicsManager.min_y)
					clipY = _vm->_graphicsManager.min_y;

				_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen, clipX, clipY,
					_vm->_globals.Liste[idx]._width + 4, _vm->_globals.Liste[idx]._height + 4,
					_vm->_graphicsManager._vesaBuffer, clipX, clipY);
				_vm->_globals.Liste[idx].field0 = false;
			}
		}
	}

	displayBobAnim();
	displayVBob();

	if (!PERSO_ON) {
		// Handle drawing characters on the screen
		for (int idx = 0; idx < MAX_SPRITE; ++idx) {
			_vm->_globals.Liste[idx].field0 = false;
			if (_sprite[idx]._animationType == 1) {
				computeSprite(idx);
				if (_sprite[idx].field2A)
					beforeSort(SORT_SPRITE, idx, _sprite[idx]._height + _sprite[idx].field2E);
			}
		}

		if (_vm->_globals._cacheFl)
			checkCache();
	}

	if (_priorityFl && _vm->_globals._sortedDisplayCount) {
		for (int v33 = 1; v33 <= 48; v33++) 
			arr[v33] = v33;

		do {
			loopCondFl = false;
			for (int v34 = 1; v34 < _vm->_globals._sortedDisplayCount; v34++) {
				if (_vm->_globals._sortedDisplay[arr[v34]]._priority > _vm->_globals._sortedDisplay[arr[v34 + 1]]._priority) {
					int oldIdx = arr[v34];
					arr[v34] = arr[v34 + 1];
					arr[v34 + 1] = oldIdx;
					loopCondFl = true;
				}
			}
		} while (loopCondFl);

		for (int sortIdx = 1; sortIdx < _vm->_globals._sortedDisplayCount + 1; sortIdx++) {
			int idx = arr[sortIdx];
			switch (_vm->_globals._sortedDisplay[idx]._sortMode) {
			case SORT_BOB:
				DEF_BOB(_vm->_globals._sortedDisplay[idx]._index);
				break;
			case SORT_SPRITE:
				DEF_SPRITE(_vm->_globals._sortedDisplay[idx]._index);
				break;
			case SORT_CACHE:
				DEF_CACHE(_vm->_globals._sortedDisplay[idx]._index);
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
				DEF_BOB(_vm->_globals._sortedDisplay[idx]._index);
				break;
			case SORT_SPRITE:
				DEF_SPRITE(_vm->_globals._sortedDisplay[idx]._index);
				break;
			case SORT_CACHE:
				DEF_CACHE(_vm->_globals._sortedDisplay[idx]._index);
				break;
			default:
				break;
			}
			_vm->_globals._sortedDisplay[idx]._sortMode = SORT_NONE;
		}
	}

	// Reset the Tri array
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
		_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager._vesaBuffer, SL_SPR, _vm->_eventsManager._startPos.x + 183, 60, 274, 353);
		if (_saveLoadX && _saveLoadY)
			_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, SL_SPR2, _saveLoadX + _vm->_eventsManager._startPos.x + 300, _saveLoadY + 300, 0);

		_vm->_graphicsManager.addVesaSegment(_vm->_eventsManager._startPos.x + 183, 60, _vm->_eventsManager._startPos.x + 457, 413);
	}

	// If the Options dialog is activated, draw the elements
	if (_vm->_globals._optionDialogFl) {
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.OPTION_SPR,
			_vm->_eventsManager._startPos.x + 464, 407, 0);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.OPTION_SPR,
			_vm->_eventsManager._startPos.x + 657, 556, _vm->_globals._menuSpeed);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.OPTION_SPR,
			_vm->_eventsManager._startPos.x + 731, 495, _vm->_globals._menuTextOff);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.OPTION_SPR,
			_vm->_eventsManager._startPos.x + 731, 468, _vm->_globals._menuVoiceOff);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.OPTION_SPR,
			_vm->_eventsManager._startPos.x + 731, 441, _vm->_globals._menuSoundOff);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.OPTION_SPR,
			_vm->_eventsManager._startPos.x + 731, 414, _vm->_globals._menuMusicOff);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.OPTION_SPR,
			_vm->_eventsManager._startPos.x + 600, 522, _vm->_globals._menuDisplayType);
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals.OPTION_SPR,
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

			if (_vm->_fontManager._textList[idx]._pos.x < _vm->_graphicsManager.min_x)
				_vm->_fontManager._textList[idx]._pos.x = _vm->_graphicsManager.min_x - 1;
			if (_vm->_fontManager._textList[idx]._pos.y < _vm->_graphicsManager.min_y)
				_vm->_fontManager._textList[idx]._pos.y = _vm->_graphicsManager.min_y - 1;

			int posX = _vm->_fontManager._textList[idx]._pos.x;
			if (_vm->_fontManager._textList[idx]._width + posX > _vm->_graphicsManager.max_x)
				_vm->_fontManager._textList[idx]._width = _vm->_graphicsManager.max_x - posX;
			int posY = _vm->_fontManager._textList[idx]._pos.y;
			if (_vm->_fontManager._textList[idx]._height + posY > _vm->_graphicsManager.max_y)
				_vm->_fontManager._textList[idx]._height = _vm->_graphicsManager.max_y - posY;
			if (_vm->_fontManager._textList[idx]._width <= 0 || _vm->_fontManager._textList[idx]._height <= 0)
				_vm->_fontManager._textList[idx]._enabledFl = false;
		}
	}

	_vm->_dialogsManager.inventAnim();
}

void ObjectsManager::INIT_BOB() {
	for (int idx = 0; idx < 35; ++idx) {
		BOB_ZERO(idx);
	}
}

void ObjectsManager::BOB_ZERO(int idx) {
	BobItem &bob = _vm->_globals._bob[idx];
	Liste2Item &item = _vm->_globals.Liste2[idx];

	bob.field0 = 0;
	bob._spriteData = g_PTRNUL;
	bob._xp = 0;
	bob._yp = 0;
	bob._frameIndex = 0;
	bob.field10 = 0;
	bob.field12 = 0;
	bob.field14 = 0;
	bob._disabledAnimationFl = false;
	bob._animData = g_PTRNUL;
	bob.field1C = false;
	bob.field1E = 0;
	bob.field20 = 0;
	bob.field22 = 0;
	bob._offsetY = 0;
	bob.field30 = g_PTRNUL;
	bob.field34 = false;
	bob.field36 = 0;
	bob._modeFlag = 0;
	bob._oldX2 = 0;

	item._visibleFl = false;
	item._xp = 0;
	item._yp = 0;
	item._width = 0;
	item._height = 0;
}

void ObjectsManager::DEF_BOB(int idx) {
	if (!_vm->_globals._bob[idx]._activeFl)
		return;

	int xp = _vm->_globals._bob[idx]._oldX;
	int yp = _vm->_globals._bob[idx]._oldY;

	if (_vm->_globals._bob[idx]._isSpriteFl)
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _vm->_globals._bob[idx]._spriteData,
			xp + 300, yp + 300, _vm->_globals._bob[idx]._frameIndex);
	else
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer,
			_vm->_globals._bob[idx]._spriteData, xp + 300, yp + 300, _vm->_globals._bob[idx]._frameIndex,
			_vm->_globals._bob[idx].field4A, _vm->_globals._bob[idx]._oldY2,
			_vm->_globals._bob[idx]._modeFlag);

	_vm->_globals.Liste2[idx]._visibleFl = true;
	_vm->_globals.Liste2[idx]._xp = xp;
	_vm->_globals.Liste2[idx]._yp = yp;

	_vm->_globals.Liste2[idx]._width = _vm->_globals._bob[idx]._oldWidth;
	_vm->_globals.Liste2[idx]._height = _vm->_globals._bob[idx]._oldHeight;

	if (_vm->_globals.Liste2[idx]._xp < _vm->_graphicsManager.min_x) {
		_vm->_globals.Liste2[idx]._width -= _vm->_graphicsManager.min_x - _vm->_globals.Liste2[idx]._xp;
		_vm->_globals.Liste2[idx]._xp = _vm->_graphicsManager.min_x;
	}

	if (_vm->_globals.Liste2[idx]._yp < _vm->_graphicsManager.min_y) {
		_vm->_globals.Liste2[idx]._height -= _vm->_graphicsManager.min_y - _vm->_globals.Liste2[idx]._yp;
		_vm->_globals.Liste2[idx]._yp = _vm->_graphicsManager.min_y;
	}

	if (_vm->_globals.Liste2[idx]._width + _vm->_globals.Liste2[idx]._xp > _vm->_graphicsManager.max_x)
		_vm->_globals.Liste2[idx]._width = _vm->_graphicsManager.max_x - _vm->_globals.Liste2[idx]._xp;

	if (_vm->_globals.Liste2[idx]._height + _vm->_globals.Liste2[idx]._yp > _vm->_graphicsManager.max_y)
		_vm->_globals.Liste2[idx]._height = _vm->_graphicsManager.max_y - _vm->_globals.Liste2[idx]._yp;

	if (_vm->_globals.Liste2[idx]._width <= 0 || _vm->_globals.Liste2[idx]._height <= 0)
		_vm->_globals.Liste2[idx]._visibleFl = false;

	if (_vm->_globals.Liste2[idx]._visibleFl)
		_vm->_graphicsManager.addVesaSegment(
             _vm->_globals.Liste2[idx]._xp,
             _vm->_globals.Liste2[idx]._yp,
             _vm->_globals.Liste2[idx]._xp + _vm->_globals.Liste2[idx]._width,
             _vm->_globals.Liste2[idx]._yp + _vm->_globals.Liste2[idx]._height);
}

void ObjectsManager::BOB_VISU(int idx) {
	_priorityFl = true;

	if (_vm->_globals._bob[idx].field0)
		return;

	BOB_ZERO(idx);

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

	_vm->_globals._bob[idx]._isSpriteFl = false;

	if (_vm->_globals.Bank[bankIdx]._fileHeader == 1) {
		_vm->_globals._bob[idx]._isSpriteFl = true;
		_vm->_globals._bob[idx].field36 = 0;
		_vm->_globals._bob[idx]._modeFlag = 0;
	}

	_vm->_globals._bob[idx]._animData = _vm->_globals.Bqe_Anim[idx]._data;
	_vm->_globals._bob[idx].field0 = 10;
	_vm->_globals._bob[idx]._spriteData = _vm->_globals.Bank[bankIdx]._data;

	_vm->_globals._bob[idx].field1E = v9;
	_vm->_globals._bob[idx].field20 = v6;
	_vm->_globals._bob[idx].field22 = v8;
	_vm->_globals._bob[idx]._offsetY = offsetY;
}

void ObjectsManager::BOB_OFF(int idx) {
	if ((_vm->_globals._bob[idx].field0 == 3) || (_vm->_globals._bob[idx].field0 == 10))
		_vm->_globals._bob[idx].field0++;
}

void ObjectsManager::BOB_OFFSET(int idx, int v) {
	_vm->_globals._bob[idx]._oldX2 = v;
}

void ObjectsManager::BOB_ADJUST(int idx, int v) {
	_vm->_globals._bob[idx]._oldX2 = v;
}

void ObjectsManager::BOB_OFFSETY(int idx, int v) {
	_vm->_globals._bob[idx]._offsetY = v;
}

void ObjectsManager::SCBOB(int idx) {
	if (_vm->_globals.Cache[idx].fieldA <= 0)
		return;

	for (int v8 = 0; v8 <= 20; v8++) {
		if ((_vm->_globals._bob[v8].field0) && (!_vm->_globals._bob[v8]._disabledAnimationFl) && (!_vm->_globals._bob[v8].field34) && (_vm->_globals._bob[v8]._frameIndex != 250)) {
			int v9 = _vm->_globals._bob[v8]._oldX + _vm->_globals._bob[v8]._oldWidth;
			int v6 = _vm->_globals._bob[v8]._oldY + _vm->_globals._bob[v8]._oldHeight;
			int v7 = _vm->_globals.Cache[idx]._width + _vm->_globals.Cache[idx]._x;

			if ((v6 > _vm->_globals.Cache[idx]._y) && (v6 < _vm->_globals.Cache[idx].field14 +_vm->_globals.Cache[idx]._height + _vm->_globals.Cache[idx]._y)) {
				bool checkFl = false;
				if (v9 >= _vm->_globals.Cache[idx]._x && v9 <= v7) {
					++_vm->_globals.Cache[idx].fieldA;
					checkFl = true;
				}
				if (!checkFl && v7 >= _vm->_globals._bob[v8]._oldWidth && _vm->_globals._bob[v8]._oldWidth >= _vm->_globals.Cache[idx]._x) {
					++_vm->_globals.Cache[idx].fieldA;
					checkFl = true;
				}

				if (!checkFl && v7 >= _vm->_globals._bob[v8]._oldWidth && _vm->_globals._bob[v8]._oldWidth >= _vm->_globals.Cache[idx]._x ) {
					++_vm->_globals.Cache[idx].fieldA;
					checkFl = true;
				}

				if (!checkFl && _vm->_globals._bob[v8]._oldWidth >= _vm->_globals.Cache[idx]._x && v9 <= v7) {
					++_vm->_globals.Cache[idx].fieldA;
					checkFl = true;
				}

				if (!checkFl && _vm->_globals._bob[v8]._oldWidth <= _vm->_globals.Cache[idx]._x && v9 >= v7)
					++_vm->_globals.Cache[idx].fieldA;
			}
		}
	}
}

void ObjectsManager::CALCUL_BOB(int idx) {
	_vm->_globals._bob[idx]._activeFl = false;
	if (_vm->_globals._bob[idx]._isSpriteFl) {
		_vm->_globals._bob[idx]._modeFlag = 0;
		_vm->_globals._bob[idx].field36 = 0;
	}

	int result = _vm->_globals._bob[idx]._frameIndex;
	if (result == 250)
		return;

	int deltaY, deltaX;
	if (_vm->_globals._bob[idx]._modeFlag) {
		deltaX = getOffsetX(_vm->_globals._bob[idx]._spriteData, result, 1);
		deltaY = getOffsetY(_vm->_globals._bob[idx]._spriteData, _vm->_globals._bob[idx]._frameIndex, 1);
	} else {
		deltaX = getOffsetX(_vm->_globals._bob[idx]._spriteData, result, 0);
		deltaY = getOffsetY(_vm->_globals._bob[idx]._spriteData, _vm->_globals._bob[idx]._frameIndex, 0);
	}

	int v20 = 0;
	int v21 = 0;
	if (_vm->_globals._bob[idx].field36 < 0) {
		v20 = -_vm->_globals._bob[idx].field36;
		if (v20 > 95)
			v20 = 95;
	} else
		v21 = _vm->_globals._bob[idx].field36;

	if (v21) {
		if (deltaX >= 0)
			deltaX = _vm->_graphicsManager.zoomIn(deltaX, v21);
		else
			deltaX = -_vm->_graphicsManager.zoomIn(deltaX, v21);

		if (deltaY >= 0) {
			deltaY = _vm->_graphicsManager.zoomIn(deltaY, v21);
		} else {
			deltaY = deltaX;
			if (deltaX < 0)
				deltaY = -deltaX;
			deltaY = -_vm->_graphicsManager.zoomIn(deltaY, v21);
		}
	}

	if (v20) {
		if (deltaX >= 0)
			deltaX = _vm->_graphicsManager.zoomOut(deltaX, v20);
		else
			deltaX = -_vm->_graphicsManager.zoomOut(-deltaX, v20);

		if (deltaY >= 0) {
			deltaY = _vm->_graphicsManager.zoomOut(deltaY, v20);
		} else {
			if (deltaX < 0)
				deltaY = -_vm->_graphicsManager.zoomOut(-deltaX, v20);
			else 
				deltaY = -_vm->_graphicsManager.zoomOut(deltaX, v20);
		}
	}

	int v13 = _vm->_globals._bob[idx]._xp - deltaX;
	int v14 = _vm->_globals._bob[idx]._yp - deltaY;
	_vm->_globals._bob[idx]._activeFl = true;
	_vm->_globals._bob[idx]._oldX = v13;
	_vm->_globals._bob[idx]._oldY = v14;
	_vm->_globals._bob[idx]._oldY2 = v21;
	_vm->_globals._bob[idx].field4A = v20;

	_vm->_globals.Liste2[idx]._visibleFl = true;
	_vm->_globals.Liste2[idx]._xp = v13;
	_vm->_globals.Liste2[idx]._yp = v14;

	int width = getWidth(_vm->_globals._bob[idx]._spriteData, _vm->_globals._bob[idx]._frameIndex);
	int height = getHeight(_vm->_globals._bob[idx]._spriteData, _vm->_globals._bob[idx]._frameIndex);

	if (v21) {
		width = _vm->_graphicsManager.zoomIn(width, v21);
		height = _vm->_graphicsManager.zoomIn(height, v21);
	}
	if (v20) {
		height = _vm->_graphicsManager.zoomOut(height, v20);
		width = _vm->_graphicsManager.zoomOut(width, v20);
	}

	_vm->_globals.Liste2[idx]._width = width;
	_vm->_globals.Liste2[idx]._height = height;
	_vm->_globals._bob[idx]._oldWidth = width;
	_vm->_globals._bob[idx]._oldHeight = height;
}

void ObjectsManager::checkCache() {
	for (int v8 = 0; v8 <= 19; v8++) {
		if (_vm->_globals.Cache[v8].fieldA <= 0)
			continue;

		int oldFieldA = _vm->_globals.Cache[v8].fieldA;
		for (int v10 = 0; v10 <= 4; v10++) {
			if (_sprite[v10]._animationType == 1 && _sprite[v10]._spriteIndex != 250) {
				int v11 = _sprite[v10]._width + _sprite[v10].field2C;
				int v2 = _sprite[v10]._height + _sprite[v10].field2E;
				int v9 = _vm->_globals.Cache[v8]._width + _vm->_globals.Cache[v8]._x;

				if (v2 > _vm->_globals.Cache[v8]._y && v2 < (_vm->_globals.Cache[v8].field14 + _vm->_globals.Cache[v8]._height + _vm->_globals.Cache[v8]._y)) {
					bool v4 = false;
					if (v11 >= _vm->_globals.Cache[v8]._x && v11 <= v9) {
						++_vm->_globals.Cache[v8].fieldA;
						v4 = true;
					}

					if (!v4 && v9 >= _sprite[v10].field2C && _vm->_globals.Cache[v8]._x <= _sprite[v10].field2C) {
						++_vm->_globals.Cache[v8].fieldA;
						v4 = true;
					}

					if (!v4 && v9 >= _sprite[v10].field2C && _vm->_globals.Cache[v8]._x <= _sprite[v10].field2C) {
						++_vm->_globals.Cache[v8].fieldA;
						v4 = true;
					}
					
					if (!v4 && _vm->_globals.Cache[v8]._x <= _sprite[v10].field2C && v11 <= v9) {
						++_vm->_globals.Cache[v8].fieldA;
						v4 = true;
					}
					
					if (!v4 && _vm->_globals.Cache[v8]._x >= _sprite[v10].field2C && v11 >= v9)
						++_vm->_globals.Cache[v8].fieldA;
				}
			}
		}

		SCBOB(v8);
		if (_vm->_globals.Cache[v8].fieldA == oldFieldA) {
			if (_vm->_globals.Cache[v8].field10) {
				_vm->_globals.Cache[v8].field10 = false;
				_vm->_globals.Cache[v8].fieldA = 1;
			}
		} else {
			int v5 = _vm->_globals.Cache[v8].field14 + _vm->_globals.Cache[v8]._height + _vm->_globals.Cache[v8]._y;
			if (v5 > 440)
				v5 = 500;

			beforeSort(SORT_CACHE, v8, v5);
			_vm->_globals.Cache[v8].fieldA = 1;
			_vm->_globals.Cache[v8].field10 = true;
		}
	}
}

void ObjectsManager::DEF_SPRITE(int idx) {
	if (!_sprite[idx].field2A)
		return;

	if (_sprite[idx]._rleFl)
		_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager._vesaBuffer, _sprite[idx]._spriteData,
		    _sprite[idx].field2C + 300, _sprite[idx].field2E + 300, _sprite[idx]._spriteIndex);
	else
		_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer, _sprite[idx]._spriteData,
		    _sprite[idx].field2C + 300, _sprite[idx].field2E + 300,  _sprite[idx]._spriteIndex, _sprite[idx]._reducePct, _sprite[idx]._zoomPct, _sprite[idx].fieldE);

	_vm->_globals.Liste[idx]._width = _sprite[idx]._width;
	_vm->_globals.Liste[idx]._height = _sprite[idx]._height;

	if (_vm->_globals.Liste[idx].field2 < _vm->_graphicsManager.min_x) {
		_vm->_globals.Liste[idx]._width -= _vm->_graphicsManager.min_x - _vm->_globals.Liste[idx].field2;
		_vm->_globals.Liste[idx].field2 = _vm->_graphicsManager.min_x;
	}

	if (_vm->_globals.Liste[idx].field4 < _vm->_graphicsManager.min_y) {
		_vm->_globals.Liste[idx]._height -= _vm->_graphicsManager.min_y - _vm->_globals.Liste[idx].field4;
		_vm->_globals.Liste[idx].field4 = _vm->_graphicsManager.min_y;
	}

	if (_vm->_globals.Liste[idx]._width + _vm->_globals.Liste[idx].field2 > _vm->_graphicsManager.max_x)
		_vm->_globals.Liste[idx]._width = _vm->_graphicsManager.max_x - _vm->_globals.Liste[idx].field2;

	if (_vm->_globals.Liste[idx]._height + _vm->_globals.Liste[idx].field4 > _vm->_graphicsManager.max_y)
		_vm->_globals.Liste[idx]._height = _vm->_graphicsManager.max_y - _vm->_globals.Liste[idx].field4;

	if (_vm->_globals.Liste[idx]._width <= 0 || _vm->_globals.Liste[idx]._height <= 0)
		_vm->_globals.Liste[idx].field0 = false;

	if (_vm->_globals.Liste[idx].field0)
		_vm->_graphicsManager.addVesaSegment( _vm->_globals.Liste[idx].field2, _vm->_globals.Liste[idx].field4,
		    _vm->_globals.Liste[idx].field2 + _vm->_globals.Liste[idx]._width, _vm->_globals.Liste[idx].field4 + _vm->_globals.Liste[idx]._height);
}

void ObjectsManager::DEF_CACHE(int idx) {
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
	if (_sprite[idx].fieldE) {
		offX = getOffsetX(_sprite[idx]._spriteData, spriteIndex, 1);
		offY = getOffsetY(_sprite[idx]._spriteData, _sprite[idx]._spriteIndex, 1);
	} else {
		offX = getOffsetX(_sprite[idx]._spriteData, spriteIndex, 0);
		offY = getOffsetY(_sprite[idx]._spriteData, _sprite[idx]._spriteIndex, 0);
	}

	int tmpX = _sprite[idx].field12 + offX;
	int deltaX = tmpX;
	int tmpY = _sprite[idx].field14 + offY;
	int deltaY = tmpY;
	int zoomPercent = 0;
	int reducePercent = 0;

	if (_sprite[idx].fieldC < 0) {
		reducePercent = -_sprite[idx].fieldC;
		if (reducePercent > 95)
			reducePercent = 95;
	} else 
		zoomPercent = _sprite[idx].fieldC;

	if (zoomPercent) {
		if (tmpX >= 0)
			deltaX = _vm->_graphicsManager.zoomIn(tmpX, zoomPercent);
		else
			deltaX = -_vm->_graphicsManager.zoomIn(-tmpX, zoomPercent);

		if (tmpY >= 0) {
			deltaY = _vm->_graphicsManager.zoomIn(tmpY, zoomPercent);
		} else {
			if (tmpX < 0)
				tmpY = -tmpX;
			else
				tmpY = tmpX;

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
			if (tmpX < 0)
				tmpY = -tmpX;
			else
				tmpY = tmpX;

			deltaY = -_vm->_graphicsManager.zoomOut(tmpY, reducePercent);
		}
	}

	int v15 = _sprite[idx]._spritePos.x - deltaX;
	int v16 = _sprite[idx]._spritePos.y - deltaY;
	_sprite[idx].field2C = v15;
	_sprite[idx].field2E = v16;
	_sprite[idx].field2A = true;
	_sprite[idx]._zoomPct = zoomPercent;
	_sprite[idx]._reducePct = reducePercent;

	_vm->_globals.Liste[idx].field0 = true;
	_vm->_globals.Liste[idx].field2 = v15;
	_vm->_globals.Liste[idx].field4 = v16;

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
int ObjectsManager::beforeSort(SortMode triMode, int index, int priority) {
	int result;

	++_vm->_globals._sortedDisplayCount;
	if (_vm->_globals._sortedDisplayCount > 48)
		error("NBTRI too high");

	result = _vm->_globals._sortedDisplayCount;
	_vm->_globals._sortedDisplay[result]._sortMode = triMode;
	_vm->_globals._sortedDisplay[result]._index = index;
	_vm->_globals._sortedDisplay[result]._priority = priority;

	return result;
}

// Display BOB Anim
void ObjectsManager::displayBobAnim() {
	int v1;
	int v5;
	uint v6;
	int v10;
	int v11;
	int v14;
	byte *v20;
	byte *v21;
	int v24;

	int idx = 0;
	do {
		++idx;
		if (idx <= 20 && PERSO_ON) {
			_vm->_globals._bob[idx].field1C = false;
			continue;
		}

		if (_vm->_globals._bob[idx].field0 != 10) 
			continue;

		_vm->_globals._bob[idx].field1C = false;
		v1 = _vm->_globals._bob[idx].field20;
		if (v1 == -1)
			v1 = 50;
		if (_vm->_globals._bob[idx]._animData == g_PTRNUL || _vm->_globals._bob[idx]._disabledAnimationFl || v1 <= 0) {
			if (_vm->_globals._bob[idx].field1E == 1 || _vm->_globals._bob[idx].field1E == 2)
				_vm->_globals._bob[idx].field1C = true;
			continue;
		}

		if (_vm->_globals._bob[idx].field12 == _vm->_globals._bob[idx].field14) {
			_vm->_globals._bob[idx].field1C = true;
		} else {
			_vm->_globals._bob[idx].field14++;
			_vm->_globals._bob[idx].field1C = false;
		}

		if (!_vm->_globals._bob[idx].field1C) {
			if (_vm->_globals._bob[idx].field1E == 1 || _vm->_globals._bob[idx].field1E == 2)
				_vm->_globals._bob[idx].field1C = true;
			continue;
		}

		v20 = _vm->_globals._bob[idx]._animData + 20;
		v24 = _vm->_globals._bob[idx].field10;
		_vm->_globals._bob[idx]._xp = (int16)READ_LE_UINT16(v20 + 2 * v24);
		if (_vm->_globals._lockedAnims[idx]._enableFl)
			_vm->_globals._bob[idx]._xp = _vm->_globals._lockedAnims[idx]._posX;
		if ( PERSO_ON && idx > 20 )
			_vm->_globals._bob[idx]._xp += _vm->_eventsManager._startPos.x;

		_vm->_globals._bob[idx]._yp = (int16)READ_LE_UINT16(v20 + 2 * v24 + 2);
		_vm->_globals._bob[idx].field12 = (int16)READ_LE_UINT16(v20 + 2 * v24 + 4);
		_vm->_globals._bob[idx].field36 = (int16)READ_LE_UINT16(v20 + 2 * v24 + 6);
		_vm->_globals._bob[idx]._frameIndex = v20[2 * v24 + 8];
		_vm->_globals._bob[idx]._modeFlag = v20[2 * v24 + 9];
		_vm->_globals._bob[idx].field10 += 5;
		v5 = _vm->_globals._bob[idx].field12;

		if (v5 > 0) {
			v6 = v5 / _vm->_globals._speed;
			_vm->_globals._bob[idx].field12 = v5 / _vm->_globals._speed;
			if (v6 > 0) {
				_vm->_globals._bob[idx].field14 = 1;
				if (_vm->_globals._bob[idx].field1E == 1 || _vm->_globals._bob[idx].field1E == 2)
					_vm->_globals._bob[idx].field1C = true;
				continue;
			}

			_vm->_globals._bob[idx].field12 = 1;
		}
		if (!_vm->_globals._bob[idx].field12) {
			if (_vm->_globals._bob[idx].field20 > 0)
				_vm->_globals._bob[idx].field20--;
			if (_vm->_globals._bob[idx].field20 != -1 && _vm->_globals._bob[idx].field20 <= 0) {
				_vm->_globals._bob[idx].field0 = 11;
			} else {
				_vm->_globals._bob[idx].field10 = 0;
				v21 = _vm->_globals._bob[idx]._animData + 20;
				_vm->_globals._bob[idx]._xp = (int16)READ_LE_UINT16(v21);

				if (_vm->_globals._lockedAnims[idx]._enableFl)
					_vm->_globals._bob[idx]._xp = _vm->_globals._lockedAnims[idx]._posX;
				if (PERSO_ON && idx > 20)
					_vm->_globals._bob[idx]._xp += _vm->_eventsManager._startPos.x;

				_vm->_globals._bob[idx]._yp = (int16)READ_LE_UINT16(v21 + 2);
				_vm->_globals._bob[idx].field12 = (int16)READ_LE_UINT16(v21 + 4);
				_vm->_globals._bob[idx].field36 = (int16)READ_LE_UINT16(v21 + 6);
				_vm->_globals._bob[idx]._frameIndex = v21[8];
				_vm->_globals._bob[idx]._modeFlag = v21[9];
				_vm->_globals._bob[idx].field10 += 5;
				v10 = _vm->_globals._bob[idx].field12;

				if (v10 > 0) {
					v11 = v10 / _vm->_globals._speed;
					_vm->_globals._bob[idx].field12 = v10 / _vm->_globals._speed;
					// Original code. It can't be negative, so the check is on == 0
					if (v11 <= 0)
						_vm->_globals._bob[idx].field12 = 1;
				}
			}
		}

		_vm->_globals._bob[idx].field14 = 1;
		if (_vm->_globals._bob[idx].field1E == 1 || _vm->_globals._bob[idx].field1E == 2)
			_vm->_globals._bob[idx].field1C = true;
		continue;
	} while (idx != 35);

	if (!PERSO_ON && BOBTOUS) {
		for (int v26 = 0; v26 != 35; v26++) {
			if (_vm->_globals._bob[v26].field0 == 10 && !_vm->_globals._bob[v26]._disabledAnimationFl)
				_vm->_globals._bob[v26].field1C = true;
		}
	}

	BOBTOUS = false;

	for (int i = 1; i < 35; i++) {
		if (i > 20 || !PERSO_ON) {
			if ((_vm->_globals._bob[i].field0 == 10) && (_vm->_globals._bob[i].field1C)) {
				v14 = _vm->_globals._bob[i].field1E;

				if ((v14 != 2) && (v14 != 4)) {
					if (_vm->_globals.Liste2[i]._visibleFl) {
						_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen,
							_vm->_globals.Liste2[i]._xp, _vm->_globals.Liste2[i]._yp,
							_vm->_globals.Liste2[i]._width, _vm->_globals.Liste2[i]._height,
							_vm->_graphicsManager._vesaBuffer, _vm->_globals.Liste2[i]._xp,
							_vm->_globals.Liste2[i]._yp);
						_vm->_globals.Liste2[i]._visibleFl = false;
					}
				}
			}

			if (_vm->_globals._bob[i].field0 == 11) {
				if (_vm->_globals.Liste2[i]._visibleFl) {
					_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen,
						_vm->_globals.Liste2[i]._xp, _vm->_globals.Liste2[i]._yp,
						_vm->_globals.Liste2[i]._width, _vm->_globals.Liste2[i]._height,
						_vm->_graphicsManager._vesaBuffer,
						_vm->_globals.Liste2[i]._xp, _vm->_globals.Liste2[i]._yp);
					_vm->_globals.Liste2[i]._visibleFl = false;
				}

				_vm->_globals._bob[i].field0 = 0;
			}
		}
	}

	for (int i = 1; i < 25; i++) {
		_vm->_globals._bob[i]._oldY = 0;
		if (_vm->_globals._bob[i].field0 == 10 && !_vm->_globals._bob[i]._disabledAnimationFl && _vm->_globals._bob[i].field1C) {
			CALCUL_BOB(i);
			int v19 = _vm->_globals._bob[i]._oldX2 + _vm->_globals._bob[i]._oldHeight + _vm->_globals._bob[i]._oldY;

			if (v19 > 450)
				v19 = 600;

			if (_vm->_globals._bob[i]._activeFl)
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
					_vm->_globals.VBob[idx]._frameIndex,
					0, 0, 0);

				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager._vesaBuffer,
					_vm->_globals.VBob[idx]._spriteData,
					_vm->_globals.VBob[idx]._xp + 300, _vm->_globals.VBob[idx]._yp + 300,
					_vm->_globals.VBob[idx]._frameIndex,
					0, 0, 0);
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
		_sprite[idx].field1C = g_PTRNUL;
		_sprite[idx]._spriteData = g_PTRNUL;
		_sprite[idx]._animationType = 0;
	}

	for (int idx = 0; idx < MAX_SPRITE; idx++) {
		_vm->_globals.Liste[idx].field0 = false;
		_vm->_globals.Liste[idx].field2 = 0;
		_vm->_globals.Liste[idx].field4 = 0;
		_vm->_globals.Liste[idx]._width = 0;
		_vm->_globals.Liste[idx]._height = 0;
		_vm->_globals.Liste[idx].fieldA = 0;
	}
}

void ObjectsManager::SPRITE_ON(int idx) {
	assert (idx  <= MAX_SPRITE);
	_sprite[idx]._animationType = 1;
}

void ObjectsManager::SPRITE(const byte *spriteData, Common::Point pos, int idx, int spriteIndex, int a6, int a7, int a8, int a9) {
	assert (idx  <= MAX_SPRITE);
	_sprite[idx]._spriteData = spriteData;
	_sprite[idx]._spritePos = pos;
	_sprite[idx]._spriteIndex = spriteIndex;
	_sprite[idx].fieldC = a6;
	_sprite[idx].fieldE = a7;
	_sprite[idx].field12 = a8;
	_sprite[idx].field14 = a9;
	_sprite[idx].field1C = g_PTRNUL;
	_sprite[idx].field20 = 0;
	_sprite[idx].field24 = 0;
	_sprite[idx].field26 = 0;
	_sprite[idx].field22 = 0;
	_sprite[idx]._animationType = 0;

	_sprite[idx]._rleFl = false;
	if (spriteData[0] == 'R' && spriteData[1] == 'L' && spriteData[2] == 'E') {
		_sprite[idx]._rleFl = true;
		_sprite[idx].fieldC = 0;
		_sprite[idx].fieldE = 0;
	}
}

void ObjectsManager::SPRITE2(const byte *spriteData, int idx, byte *a3, int a4, int a5) {
	_sprite[idx]._spriteData = spriteData;
	_sprite[idx].field1C = a3;
	_sprite[idx].field20 = a4;
	_sprite[idx].field24 = 0;
	_sprite[idx].field26 = 0;
	_sprite[idx].fieldC = 0;
	_sprite[idx].fieldE = 0;
	_sprite[idx]._animationType = 1;
	_sprite[idx].field22 = 0;
	_sprite[idx].field14 = a5;

	_sprite[idx]._rleFl = false;
	if (spriteData[0] == 'R' && spriteData[1] == 'L' && spriteData[2] == 'E')
		_sprite[idx]._rleFl = true;

}

/**
 * Freeze sprite animation and free its memory
 */
void ObjectsManager::removeSprite(int idx) {
	// Type 3 was also used by freeSprite(), which has been removed as it wasn't used
	_sprite[idx]._animationType = 3;
	_sprite[idx].field1C = _vm->_globals.freeMemory(_sprite[idx].field1C);
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
void ObjectsManager::SETTAILLESPR(int idx, int a2) {
	assert (idx  <= MAX_SPRITE);
	if (!_sprite[idx]._rleFl)
		_sprite[idx].fieldC = a2;
}

void ObjectsManager::setFlipSprite(int idx, bool flip) {
	assert (idx  <= MAX_SPRITE);
	if (!_sprite[idx]._rleFl)
		_sprite[idx].fieldE = flip;
}

void ObjectsManager::checkZone() {
	int v0;
	int v1;
	int v2;
	uint16 v3;
	int v4;

	v0 = _vm->_eventsManager.getMouseX();
	v1 = _vm->_eventsManager.getMouseY();
	v2 = v1;
	if (_vm->_globals.PLAN_FLAG
	        || _vm->_eventsManager._startPos.x >= v0
	        || (v1 = _vm->_graphicsManager._scrollOffset + 54, v0 >= v1)
	        || (v1 = v2 - 1, (uint16)(v2 - 1) > 59)) {
		if (_visibleFl)
			FLAG_VISIBLE_EFFACE = 4;
		_visibleFl = false;
	} else {
		_visibleFl = true;
	}
	if (_forceZoneFl) {
		_vm->_globals.compteur_71 = 100;
		_vm->_globals.old_zone_68 = -1;
		_vm->_globals.old_x_69 = -200;
		_vm->_globals.old_y_70 = -220;
		_forceZoneFl = false;
	}
	v3 = _vm->_globals.compteur_71 + 1;
	_vm->_globals.compteur_71 = v3;

	if (v3 <= 1)
		return;

	if (_vm->_globals.NOMARCHE || (_vm->_globals.chemin == (int16 *)g_PTRNUL) || v3 > 4u) {
		_vm->_globals.compteur_71 = 0;
		if (_vm->_globals.old_x_69 != v0 || _vm->_globals.old_y_70 != v2) {
			v4 = MZONE();
		} else {
			v4 = _vm->_globals.old_zone_68;
		}
		if (_vm->_globals.old_zone_68 != v4) {
			_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
			_vm->_eventsManager._mouseCursorId = 4;
			_vm->_eventsManager.changeMouseCursor(4);
			if (_vm->_globals.zozo_73 == 1) {
				_vm->_fontManager.hideText(5);
				_vm->_globals.zozo_73 = 0;
				return;
			}
		}
		if (v4 != -1) {
			if (_vm->_globals.ZONEP[v4].field6 || _vm->_globals.ZONEP[v4].field7 ||
			    _vm->_globals.ZONEP[v4].field8 || _vm->_globals.ZONEP[v4].field9 ||
			    _vm->_globals.ZONEP[v4].fieldA || _vm->_globals.ZONEP[v4].fieldB ||
			    _vm->_globals.ZONEP[v4].fieldC || _vm->_globals.ZONEP[v4].fieldD ||
			    _vm->_globals.ZONEP[v4].fieldE || _vm->_globals.ZONEP[v4].fieldF) {
				if (_vm->_globals.old_zone_68 != v4) {
					_vm->_fontManager.initTextBuffers(5, _vm->_globals.ZONEP[v4].field12, _vm->_globals.FICH_ZONE, 0, 430, 0, 0, 252);
					_vm->_fontManager.showText(5);
					_vm->_globals.zozo_73 = 1;
				}
				_vm->_globals.force_to_data_0 += 25;
				if (_vm->_globals.force_to_data_0 > 100)
					_vm->_globals.force_to_data_0 = 0;
				_vm->_graphicsManager.SETCOLOR4(251, _vm->_globals.force_to_data_0, _vm->_globals.force_to_data_0,
					_vm->_globals.force_to_data_0);
				if (_vm->_eventsManager._mouseCursorId == 4) {
					v1 = 5 * v4;
					if (_vm->_globals.ZONEP[v4].field6 == 2) {
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
		NUMZONE = v4;
		_vm->_globals.old_x_69 = v0;
		_vm->_globals.old_y_70 = v2;
		_vm->_globals.old_zone_68 = v4;
		if (_vm->_globals.NOMARCHE && (_vm->_eventsManager._mouseCursorId == 4)) {
			v1 = v4 + 1;
			if ((uint16)(v4 + 1) > 1u)
				handleRightButton();
		}
		if ((_vm->_globals.PLAN_FLAG && v4 == -1) || !v4) {
			_verb = 0;
			_vm->_eventsManager._mouseCursorId = 0;
			_vm->_eventsManager.changeMouseCursor(0);
		}
	}
}

void ObjectsManager::GOHOME() {
	int16 v0;
	int16 v1;
	int16 v3;
	int16 v4;
	unsigned int v5;
	unsigned int v6;
	unsigned int v7;
	unsigned int v8;
	int16 v9;
	int16 v10;
	unsigned int v11;
	unsigned int v12;
	unsigned int v13;
	unsigned int v14;
	int v15;
	int16 v16;
	unsigned int v17;
	unsigned int v18;
	int v19;
	int16 v20;
	unsigned int v21;
	unsigned int v22;
	int16 v23;
	int16 v24;
	unsigned int v25;
	unsigned int v26;
	unsigned int v27;
	unsigned int v28;
	int16 v29;
	int16 v30;
	unsigned int v31;
	unsigned int v32;
	unsigned int v33;
	unsigned int v34;
	int16 v35;
	int16 v36;
	unsigned int v37;
	unsigned int v38;
	unsigned int v39;
	unsigned int v40;
	int16 v41;
	int16 v42;
	unsigned int v43;
	unsigned int v44;
	unsigned int v45;
	unsigned int v46;
	int16 v47;
	int16 v48;
	int16 v49;
	int v50;
	int16 v51;
	int16 v52;
	int16 v54;
	int v55;
	int16 v56;
	int16 v57;
	int16 v58;

	v0 = 0;
	v58 = 0;
	v1 = 0;

	if (_vm->_globals.chemin == (int16 *)g_PTRNUL)
		return;

	if (_vm->_globals.Compteur > 1) {
		--_vm->_globals.Compteur;
		return;
	}

	_vm->_globals.Compteur = 0;
	if (_vm->_globals.g_old_sens == -1) {
		VERIFTAILLE();
		nouveau_x = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_y = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_sens = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_anim = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		if (nouveau_x != -1 || nouveau_y != -1) {
			_vm->_globals.g_old_sens = nouveau_sens;
			_vm->_globals.g_old_sens2 = nouveau_sens;
			_vm->_globals.g_old_anim = 0;
			g_old_x = nouveau_x;
			g_old_y = nouveau_y;
		} else {
			setSpriteIndex(0, _vm->_globals.g_old_sens + 59);
			_vm->_globals._actionDirection = 0;
			if (_vm->_globals.GOACTION)
				v54 = _vm->_globals._saveData->data[svField2];
			else
				v54 = NUMZONE;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			VERIFTAILLE();
			setFlipSprite(0, false);
			_vm->_globals.Compteur = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			_vm->_globals.g_old_sens = -1;
			if (v54 > 0) {
				v55 = v54;
				if (_vm->_globals.ZONEP[v55]._destX) {
					v56 = _vm->_globals.ZONEP[v55]._destY;
					if (v56) {
						if (v56 != 31) {
							v57 = _vm->_globals.ZONEP[v55].field4;
							if (v57 == -1) {
								_vm->_globals.ZONEP[v55]._destX = 0;
								_vm->_globals.ZONEP[v55]._destY = 0;
								_vm->_globals.ZONEP[v55].field4 = 0;
							} else {
								setSpriteIndex(0, v57);
								_vm->_globals._actionDirection = _vm->_globals.ZONEP[v55].field4 - 59;
							}
						}
					}
				}
			}
		}
LABEL_241:
		_vm->_globals.Compteur = 0;
		return;
	}
	if (_vm->_globals.g_old_sens == 3) {
		if ((uint16)(_vm->_globals.g_old_anim - 24) > 11) {
			v0 = g_old_x;
			v58 = g_old_y;
		} else {
			v3 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
			v4 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;

			if (_sprite[0].fieldC < 0) {
				v5 = _sprite[0].fieldC;
				if (_sprite[0].fieldC < 0)
					v5 = -_sprite[0].fieldC;
				v3 = _vm->_graphicsManager.zoomOut(v3, v5);
				v6 = _sprite[0].fieldC;
				if (_sprite[0].fieldC < 0)
					v6 = -_sprite[0].fieldC;
				v4 = _vm->_graphicsManager.zoomOut(v4, v6);
			}
			if (_sprite[0].fieldC > 0) {
				v7 = _sprite[0].fieldC;
				if (_sprite[0].fieldC < 0)
					v7 = -_sprite[0].fieldC;
				v3 = _vm->_graphicsManager.zoomIn(v3, v7);
				v8 = _sprite[0].fieldC;
				if (_sprite[0].fieldC < 0)
					v8 = -_sprite[0].fieldC;
				v4 = _vm->_graphicsManager.zoomIn(v4, v8);
			}
			v0 = v3 + g_old_x;
			v58 = g_old_y + v4;
			v1 = _vm->_globals.g_old_anim + 1;
			if (_vm->_globals.g_old_anim != 35)
				goto LABEL_23;
		}
		v1 = 24;
LABEL_23:
		_vm->_globals.Compteur = 5 / _vm->_globals._speed;
	}
	if (_vm->_globals.g_old_sens != 7)
		goto LABEL_43;
	if ((uint16)(_vm->_globals.g_old_anim - 24) > 11) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v9 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v10 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (_sprite[0].fieldC < 0) {
			v11 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v11 = -_sprite[0].fieldC;
			v9 = _vm->_graphicsManager.zoomOut(v9, v11);
			v12 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v12 = -_sprite[0].fieldC;
			v10 = _vm->_graphicsManager.zoomOut(v10, v12);
		}
		if (_sprite[0].fieldC > 0) {
			v13 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v13 = -_sprite[0].fieldC;
			v9 = _vm->_graphicsManager.zoomIn(v9, v13);
			v14 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v14 = -_sprite[0].fieldC;
			v10 = _vm->_graphicsManager.zoomIn(v10, v14);
		}
		v0 = g_old_x - v9;
		v58 = g_old_y - v10;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 35)
			goto LABEL_42;
	}
	v1 = 24;
LABEL_42:
	_vm->_globals.Compteur = 5 / _vm->_globals._speed;
LABEL_43:
	if (_vm->_globals.g_old_sens != 1)
		goto LABEL_60;
	if (_vm->_globals.g_old_anim > 11) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v15 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (v15 < 0)
			v15 = -v15;
		v16 = v15;
		if (_sprite[0].fieldC < 0) {
			v17 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v17 = -_sprite[0].fieldC;
			v16 = _vm->_graphicsManager.zoomOut(v16, v17);
		}
		if (_sprite[0].fieldC > 0) {
			v18 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v18 = -_sprite[0].fieldC;
			v16 = _vm->_graphicsManager.zoomIn(v16, v18);
		}
		v0 = g_old_x;
		v58 = g_old_y - v16;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 11)
			goto LABEL_59;
	}
	v1 = 0;
LABEL_59:
	_vm->_globals.Compteur = 4 / _vm->_globals._speed;
LABEL_60:
	if (_vm->_globals.g_old_sens != 5)
		goto LABEL_77;
	if ((uint16)(_vm->_globals.g_old_anim - 48) > 11) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v19 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (v19 < 0)
			v19 = -v19;
		v20 = v19;
		if (_sprite[0].fieldC < 0) {
			v21 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v21 = -_sprite[0].fieldC;
			v20 = _vm->_graphicsManager.zoomOut(v20, v21);
		}
		if (_sprite[0].fieldC > 0) {
			v22 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v22 = -_sprite[0].fieldC;
			v20 = _vm->_graphicsManager.zoomIn(v20, v22);
		}
		v0 = g_old_x;
		v58 = v20 + g_old_y;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 59)
			goto LABEL_76;
	}
	v1 = 48;
LABEL_76:
	_vm->_globals.Compteur = 4 / _vm->_globals._speed;
LABEL_77:
	if (_vm->_globals.g_old_sens != 2)
		goto LABEL_96;
	if ((uint16)(_vm->_globals.g_old_anim - 12) > 11) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v23 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v24 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (_sprite[0].fieldC < 0) {
			v25 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v25 = -_sprite[0].fieldC;
			v23 = _vm->_graphicsManager.zoomOut(v23, v25);
			v26 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v26 = -_sprite[0].fieldC;
			v24 = _vm->_graphicsManager.zoomOut(v24, v26);
		}
		if (_sprite[0].fieldC > 0) {
			v27 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v27 = -_sprite[0].fieldC;
			v23 = _vm->_graphicsManager.zoomIn(v23, v27);
			v28 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v28 = -_sprite[0].fieldC;
			v24 = _vm->_graphicsManager.zoomIn(v24, v28);
		}
		v0 = v23 + g_old_x;
		v58 = g_old_y + v24;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 23)
			goto LABEL_95;
	}
	v1 = 12;
LABEL_95:
	_vm->_globals.Compteur = 5 / _vm->_globals._speed;
LABEL_96:
	if (_vm->_globals.g_old_sens != 8)
		goto LABEL_115;
	if ((uint16)(_vm->_globals.g_old_anim - 12) > 11) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v29 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v30 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (_sprite[0].fieldC < 0) {
			v31 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v31 = -_sprite[0].fieldC;
			v29 = _vm->_graphicsManager.zoomOut(v29, v31);
			v32 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v32 = -_sprite[0].fieldC;
			v30 = _vm->_graphicsManager.zoomOut(v30, v32);
		}
		if (_sprite[0].fieldC > 0) {
			v33 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v33 = -_sprite[0].fieldC;
			v29 = _vm->_graphicsManager.zoomIn(v29, v33);
			v34 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v34 = -_sprite[0].fieldC;
			v30 = _vm->_graphicsManager.zoomIn(v30, v34);
		}
		v0 = g_old_x - v29;
		v58 = g_old_y + v30;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 23)
			goto LABEL_114;
	}
	v1 = 12;
LABEL_114:
	_vm->_globals.Compteur = 5 / _vm->_globals._speed;
LABEL_115:
	if (_vm->_globals.g_old_sens != 4)
		goto LABEL_134;
	if ((uint16)(_vm->_globals.g_old_anim - 36) > 11) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v35 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v36 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (_sprite[0].fieldC < 0) {
			v37 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v37 = -_sprite[0].fieldC;
			v35 = _vm->_graphicsManager.zoomOut(v35, v37);
			v38 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v38 = -_sprite[0].fieldC;
			v36 = _vm->_graphicsManager.zoomOut(v36, v38);
		}
		if (_sprite[0].fieldC > 0) {
			v39 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v39 = -_sprite[0].fieldC;
			v35 = _vm->_graphicsManager.zoomIn(v35, v39);
			v40 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v40 = -_sprite[0].fieldC;
			v36 = _vm->_graphicsManager.zoomIn(v36, v40);
		}
		v0 = v35 + g_old_x;
		v58 = g_old_y + v36;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 47)
			goto LABEL_133;
	}
	v1 = 36;
LABEL_133:
	_vm->_globals.Compteur = 5 / _vm->_globals._speed;
LABEL_134:
	if (_vm->_globals.g_old_sens != 6)
		goto LABEL_153;
	if ((uint16)(_vm->_globals.g_old_anim - 36) > 11) {
		v0 = g_old_x;
		v58 = g_old_y;
	} else {
		v41 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field0;
		v42 = _vm->_globals.Hopkins[_vm->_globals.g_old_anim].field2;
		if (_sprite[0].fieldC < 0) {
			v43 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v43 = -_sprite[0].fieldC;
			v41 = _vm->_graphicsManager.zoomOut(v41, v43);
			v44 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v44 = -_sprite[0].fieldC;
			v42 = _vm->_graphicsManager.zoomOut(v42, v44);
		}
		if (_sprite[0].fieldC > 0) {
			v45 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v45 = -_sprite[0].fieldC;
			v41 = _vm->_graphicsManager.zoomIn(v41, v45);
			v46 = _sprite[0].fieldC;
			if (_sprite[0].fieldC < 0)
				v46 = -_sprite[0].fieldC;
			v42 = _vm->_graphicsManager.zoomIn(v42, v46);
		}
		v0 = g_old_x - v41;
		v58 = g_old_y + v42;
		v1 = _vm->_globals.g_old_anim + 1;
		if (_vm->_globals.g_old_anim != 47)
			goto LABEL_152;
	}
	v1 = 36;
LABEL_152:
	_vm->_globals.Compteur = 5 / _vm->_globals._speed;
LABEL_153:
	v47 = 0;
	do {
		nouveau_x = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		v48 = *_vm->_globals.chemin;
		nouveau_y = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_sens = *_vm->_globals.chemin;
		_vm->_globals.chemin++;
		nouveau_anim = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		if (nouveau_x == -1 && v48 == -1) {
			if (_vm->_globals.GOACTION)
				v49 = _vm->_globals._saveData->data[svField2];
			else
				v49 = NUMZONE;
			setSpriteIndex(0, _vm->_globals.g_old_sens + 59);
			_vm->_globals._actionDirection = 0;
			_vm->_globals.chemin = (int16 *)g_PTRNUL;
			VERIFTAILLE();
			setFlipSprite(0, false);
			_vm->_globals.Compteur = 0;
			_vm->_globals.g_old_sens = -1;
			g_old_x = getSpriteX(0);
			g_old_y = getSpriteY(0);

			if (v49 > 0) {
				v50 = v49;
				if (_vm->_globals.ZONEP[v50]._destX) {
					v51 = _vm->_globals.ZONEP[v50]._destY;
					if (v51) {
						if (v51 != 31) {
							v52 = _vm->_globals.ZONEP[v50].field4;
							if (v52 == -1) {
								_vm->_globals.ZONEP[v50]._destX = 0;
								_vm->_globals.ZONEP[v50]._destY = 0;
								_vm->_globals.ZONEP[v50].field4 = 0;
							} else {
								setSpriteIndex(0, v52);
								_vm->_globals._actionDirection = _vm->_globals.ZONEP[v50].field4 - 59;
							}
						}
					}
				}
			}
			goto LABEL_241;
		}
		if (_vm->_globals.g_old_sens != nouveau_sens)
			break;
		if ((nouveau_sens == 3 && nouveau_x >= v0) || (_vm->_globals.g_old_sens == 7 && nouveau_x <= v0) ||
		    (_vm->_globals.g_old_sens == 1 && nouveau_y <= v58) || (_vm->_globals.g_old_sens == 5 && nouveau_y >= v58) ||
		    (_vm->_globals.g_old_sens == 2 && nouveau_x >= v0)  || (_vm->_globals.g_old_sens == 8 && nouveau_x <= v0) ||
		    (_vm->_globals.g_old_sens == 4 && nouveau_x >= v0) || (_vm->_globals.g_old_sens == 6 && nouveau_x <= v0))
			v47 = 1;
	} while (v47 != 1);
	if (v47 == 1) {
		VERIFTAILLE();
		if ((_vm->_globals.g_old_sens == 6) || (_vm->_globals.g_old_sens == 7) || (_vm->_globals.g_old_sens == 8))
			setFlipSprite(0, true);

		if ((_vm->_globals.g_old_sens == 1) || (_vm->_globals.g_old_sens == 2) || (_vm->_globals.g_old_sens == 3) ||
		    (_vm->_globals.g_old_sens == 4) || (_vm->_globals.g_old_sens == 5))
			setFlipSprite(0, false);

		setSpriteX(0, nouveau_x);
		setSpriteY(0, nouveau_y);
		setSpriteIndex(0, v1);
	} else {
		if ((_vm->_globals.g_old_sens == 6) || (_vm->_globals.g_old_sens == 7) || (_vm->_globals.g_old_sens == 8))
			setFlipSprite(0, true);

		if ((_vm->_globals.g_old_sens == 1) || (_vm->_globals.g_old_sens == 2) || (_vm->_globals.g_old_sens == 3) ||
		    (_vm->_globals.g_old_sens == 4) || (_vm->_globals.g_old_sens == 5))
			setFlipSprite(0, false);
		_vm->_globals.Compteur = 0;
	}
	_vm->_globals.g_old_sens = nouveau_sens;
	_vm->_globals.g_old_sens2 = nouveau_sens;
	_vm->_globals.g_old_anim = v1;
	g_old_x = nouveau_x;
	g_old_y = nouveau_y;
}

void ObjectsManager::GOHOME2() {
	if (_vm->_globals.chemin == (int16 *)g_PTRNUL)
		return;

	int v0 = 2;
	if (_vm->_globals._speed == 2)
		v0 = 4;
	else if (_vm->_globals._speed == 3)
		v0 = 6;

	_vm->_globals.j_104 = 0;

	for (;;) {
		nouveau_x = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_y = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_sens = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		nouveau_anim = *_vm->_globals.chemin;
		_vm->_globals.chemin++;

		if ((nouveau_x == -1) && (nouveau_y == -1))
			break;

		++_vm->_globals.j_104;
		if (_vm->_globals.j_104 >= v0) {
			_vm->_globals._lastDirection = nouveau_sens;
			setSpriteX(0, nouveau_x);
			setSpriteY(0, nouveau_y);
			if (_vm->_globals._lastDirection == 1)
				setSpriteIndex(0, 4);
			else if (_vm->_globals._lastDirection == 3)
				setSpriteIndex(0, 5);
			else if (_vm->_globals._lastDirection == 5)
				setSpriteIndex(0, 6);
			else if (_vm->_globals._lastDirection == 7)
				setSpriteIndex(0, 7);

			if (my_anim++ > 1)
				my_anim = 0;

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

	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	my_anim = 0;
	A_ANIM = 0;
	A_DEPA = 0;
}

// Load Obstacle
void ObjectsManager::CHARGE_OBSTACLE(const Common::String &file) {
	int16 v1;
	byte *ptr;
	int16 v4;
	int16 v5;

	_vm->_linesManager.RESET_OBSTACLE();
	_vm->_linesManager._linesNumb = 0;
	_lastLine = 0;
	_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, file);
	ptr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	v4 = 0;
	v5 = 0;
	do {
		v1 = (int16)READ_LE_UINT16((uint16 *)ptr + v4);
		if (v1 != -1) {
			_vm->_linesManager.AJOUTE_LIGNE(
			    v5,
			    v1,
			    (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 1),
			    (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 2),
			    (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 3),
			    (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 4),
			    1);
			++_vm->_linesManager._linesNumb;
		}
		v4 += 5;
		++v5;
	} while (v1 != -1);
	_vm->_linesManager.INIPARCOURS();
	_vm->_globals.freeMemory(ptr);
}

/**
 * Load Zone
 */
void ObjectsManager::loadZone(const Common::String &file) {
	int bobZoneIdx;
	int v4;
	byte *v9;
	int v10;
	byte *v13;
	int16 v17;
	int16 v18;
	byte *ptr;

	for (int i = 1; i <= 100; i++) {
		_vm->_globals.ZONEP[i]._destX = 0;
		_vm->_globals.ZONEP[i]._destY = 0;
		_vm->_globals.ZONEP[i].field4 = 0;
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
		_vm->_globals.ZONEP[i].field10 = 0;
	}

	_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, file);

	Common::File f;
	if (!f.exists(_vm->_globals._curFilename))
		error("File not found : %s", _vm->_globals._curFilename.c_str());

	ptr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	v4 = 0;
	v18 = 0;
	v17 = 0;
	do {
		bobZoneIdx = (int16)READ_LE_UINT16((uint16 *)ptr + v4);
		if (bobZoneIdx != -1) {
			_vm->_linesManager.addZoneLine(
			    v18,
			    READ_LE_UINT16((uint16 *)ptr + v4 + 1), // CHECKME: Shouldn't it be a byte?
			    READ_LE_UINT16((uint16 *)ptr + v4 + 2),
			    READ_LE_UINT16((uint16 *)ptr + v4 + 3),
			    READ_LE_UINT16((uint16 *)ptr + v4 + 4),
			    bobZoneIdx);
			_vm->_globals.ZONEP[bobZoneIdx].field10 = 1;
		}
		v4 += 5;
		++v18;
		++v17;
	} while (bobZoneIdx != -1);

	for (int i = 1; i <= 100; i++) {
		 // CHECKME: Shouldn't it be a byte?
		_vm->_globals.ZONEP[i]._destX = (int16)READ_LE_UINT16((uint16 *)ptr + v4);
		_vm->_globals.ZONEP[i]._destY = (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 1);
		_vm->_globals.ZONEP[i].field4 = (int16)READ_LE_UINT16((uint16 *)ptr + v4 + 2);
		v4 += 3;
	}

	v9 = (ptr + 10 * v17 + 606);
	v10 = 0;
	for (int i = 1; i <= 100; i++) {
		_vm->_globals.ZONEP[i].field6 = v9[v10];
		_vm->_globals.ZONEP[i].field7 = v9[v10 + 1];
		_vm->_globals.ZONEP[i].field8 = v9[v10 + 2];
		_vm->_globals.ZONEP[i].field9 = v9[v10 + 3];
		_vm->_globals.ZONEP[i].fieldA = v9[v10 + 4];
		_vm->_globals.ZONEP[i].fieldB = v9[v10 + 5];
		_vm->_globals.ZONEP[i].fieldC = v9[v10 + 6];
		_vm->_globals.ZONEP[i].fieldD = v9[v10 + 7];
		_vm->_globals.ZONEP[i].fieldE = v9[v10 + 8];
		_vm->_globals.ZONEP[i].fieldF = v9[v10 + 9];

		v10 += 10;
	}
	v13 = v9 + 1010;
	for (int v14 = 0; v14 < 100; v14++)
		_vm->_globals.ZONEP[v14 + 1].field12 = READ_LE_UINT16(v13 + 2 * v14);

	_vm->_globals.freeMemory(ptr);
	CARRE_ZONE();
}

// Square Zone
void ObjectsManager::CARRE_ZONE() {
	int16 *dataP;
	int v4;
	int v5;
	int v10;
	int v11;
	int v12;
	int v14;

	for (int idx = 0; idx < 100; ++idx) {
		_vm->_globals.CarreZone[idx].field0 = 0;
		_vm->_globals.CarreZone[idx].fieldE = 0;
		_vm->_globals.CarreZone[idx].field2 = 1280;
		_vm->_globals.CarreZone[idx].field4 = 0;
		_vm->_globals.CarreZone[idx].field6 = 460;
		_vm->_globals.CarreZone[idx].field8 = 0;
		_vm->_globals.CarreZone[idx].fieldA = 401;
		_vm->_globals.CarreZone[idx].fieldC = 0;
	}

	for (int idx = 0; idx < 400; ++idx) {
		dataP = _vm->_linesManager._zoneLine[idx].zoneData;
		if (dataP != (int16 *)g_PTRNUL) {
			v4 = _vm->_linesManager._zoneLine[idx].field2;
			_vm->_globals.CarreZone[v4].field0 = 1;
			if (_vm->_globals.CarreZone[v4].fieldC < idx)
				_vm->_globals.CarreZone[v4].fieldC = idx;
			if (_vm->_globals.CarreZone[v4].fieldA > idx)
				_vm->_globals.CarreZone[v4].fieldA = idx;

			v12 = _vm->_linesManager._zoneLine[idx].count;
			if (v12 > 0) {
				for (int v13 = 0; v13 < v12; v13++) {
					v5 = *dataP++;
					v11 = *dataP++;

					if (_vm->_globals.CarreZone[v4].field2 >= v5)
						_vm->_globals.CarreZone[v4].field2 = v5;
					if (_vm->_globals.CarreZone[v4].field4 <= v5)
						_vm->_globals.CarreZone[v4].field4 = v5;
					if (_vm->_globals.CarreZone[v4].field6 >= v11)
						_vm->_globals.CarreZone[v4].field6 = v11;
					if (_vm->_globals.CarreZone[v4].field8 <= v11)
						_vm->_globals.CarreZone[v4].field8 = v11;
				}
			}
		}
	}

	for (int v7 = 0; v7 <= 99; v7++) {
		v10 = _vm->_globals.CarreZone[v7].field2 - _vm->_globals.CarreZone[v7].field4;
		if (v10 < 0)
			v10 = -v10;
		v14 = _vm->_globals.CarreZone[v7].field6 - _vm->_globals.CarreZone[v7].field8;
		if (v14 < 0)
			v14 = -v14;
		if (v10 == v14)
			_vm->_globals.CarreZone[v7].fieldE = 1;
	}
}

void ObjectsManager::PLAN_BETA() {
	int v1;
	int v3;
	int v5;

	v1 = 0;
	_vm->_dialogsManager._inventFl = false;
	_vm->_eventsManager._gameKey = KEY_NONE;
	_vm->_globals.Max_Propre = 1;
	_vm->_globals.Max_Ligne_Long = 1;
	_vm->_globals.Max_Perso_Y = 440;
	_vm->_globals.NOSPRECRAN = true;
	_vm->_globals.PLAN_FLAG = true;
	_vm->_graphicsManager._noFadingFl = false;
	_vm->_globals.NOMARCHE = false;
	_spritePtr = g_PTRNUL;
	_vm->_globals._exitId = 0;
	_vm->_globals.NOT_VERIF = true;
	_vm->_soundManager.WSOUND(31);
	_vm->_globals.iRegul = 1;
	_vm->_graphicsManager.loadImage("PLAN");
	CHARGE_OBSTACLE("PLAN.OB2");
	_vm->_globals.loadCache("PLAN.CA2");
	loadZone("PLAN.ZO2");
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "VOITURE.SPR");
	_spritePtr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	_vm->_animationManager.loadAnim("PLAN");
	_vm->_graphicsManager.VISU_ALL();
	_vm->_graphicsManager.INI_ECRAN2("PLAN");
	for (int v2 = 0; v2 <= 15; v2++)
		_vm->_globals.CACHE_OFF(v2);
	_vm->_globals.CACHE_OFF(19);
	_vm->_globals.CACHE_OFF(20);
	_vm->_globals.CACHE_ON();

	if (!_vm->_globals.PLANX && !_vm->_globals.PLANY) {
		_vm->_globals.PLANX = 900;
		_vm->_globals.PLANY = 319;
		_vm->_globals.PLANI = 1;
	}
	SPRITE(_spritePtr, Common::Point(_vm->_globals.PLANX, _vm->_globals.PLANY), 0, _vm->_globals.PLANI, 0, 0, 5, 5);
	_vm->_eventsManager.setMouseXY(_vm->_globals.PLANX, _vm->_globals.PLANY);
	my_anim = 0;
	_vm->_eventsManager.mouseOn();
	v3 = getSpriteX(0);
	_vm->_graphicsManager.scrollScreen(v3 - 320);
	_vm->_graphicsManager._scrollOffset = getSpriteX(0) - 320;
	SPRITE_ON(0);
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);

	for (int v4 = 0; v4 <= 4; v4++)
		_vm->_eventsManager.VBL();

	_vm->_globals.iRegul = 1;
	_vm->_graphicsManager.fadeInLong();
	_vm->_eventsManager.changeMouseCursor(4);
	_vm->_graphicsManager._noFadingFl = false;

	do {
		v5 = _vm->_eventsManager.getMouseButton();
		if (v5) {
			if (_vm->_globals._saveData->data[svField170] == 1 && !_vm->_globals._saveData->data[svField171]) {
				_vm->_globals._saveData->data[svField171] = 1;
				_vm->_globals.NOPARLE = true;
				_vm->_talkManager.PARLER_PERSO("APPEL1.pe2");
				_vm->_globals.NOPARLE = false;
				v5 = 0;
			}
			if (_vm->_globals._saveData->data[svField80] == 1 && !_vm->_globals._saveData->data[svField172]) {
				_vm->_globals._saveData->data[svField172] = 1;
				_vm->_globals.NOPARLE = true;
				_vm->_talkManager.PARLER_PERSO("APPEL2.pe2");
				_vm->_globals.NOPARLE = false;
				v5 = 0;
				_vm->_eventsManager._curMouseButton = 0;
			}
			if (v5 == 1)
				handleLeftButton();
		}

		checkZone();
		GOHOME2();

		if (_vm->_globals.chemin == (int16 *)g_PTRNUL && _vm->_globals.GOACTION)
			PARADISE();
		_vm->_eventsManager.VBL();

		if (_vm->_globals._exitId)
			v1 = 1;
	} while (!_vm->shouldQuit() && v1 != 1);

	if (!_vm->_graphicsManager._noFadingFl)
		_vm->_graphicsManager.fadeOutLong();
	_vm->_globals.iRegul = 0;
	_vm->_graphicsManager._noFadingFl = false;
	_vm->_globals.PLANX = getSpriteX(0);
	_vm->_globals.PLANY = getSpriteY(0);
	_vm->_globals.PLANI = 1;
	removeSprite(0);
	_spritePtr = _vm->_globals.freeMemory(_spritePtr);
	CLEAR_ECRAN();
	_vm->_globals.NOSPRECRAN = false;
	_vm->_globals.PLAN_FLAG = false;
}

/**
 * Handle Left button
 */
void ObjectsManager::handleLeftButton() {
	int srcX, srcY;
	int destX, destY;
	int v1;
	int v2;
	int16 *v3;
	int16 *v4;
	int16 *v5;
	int zoneCount;
	int zoneX;
	int zoneY;
	int16 *v9;
	int v10;
	int v11;
	int v12;
	int16 *v13;
	int16 *v16;
	int v17;
	int v18;

	_vm->_fontManager.hideText(9);
	destX = _vm->_eventsManager.getMouseX();
	destY = _vm->_eventsManager.getMouseY();

	if (!_vm->_dialogsManager._inventFl && !_vm->_globals.PLAN_FLAG && destX > _vm->_graphicsManager._scrollOffset - 30 && destX < _vm->_graphicsManager._scrollOffset + 50 && (uint16)(destY + 29) <= 78) {
		v1 = _vm->_eventsManager._mouseCursorId;
		_vm->_dialogsManager._inventFl = true;
		_vm->_dialogsManager.showInventory();
		_vm->_dialogsManager._inventFl = false;
		_vm->_eventsManager._gameKey = KEY_NONE;
		if (!_vm->_globals._exitId) {
			_vm->_dialogsManager._inventFl = false;
			_vm->_eventsManager._mouseCursorId = v1;
		}
		return;
	}
	if (_vm->_globals._saveData->data[svField354] == 1
	        && !_vm->_globals.PLAN_FLAG
	        && (uint16)(destX - 533) <= 26
	        && (uint16)(destY - 26) <= 33) {
		changeCharacterHead(CHARACTER_HOPKINS_CLONE, CHARACTER_HOPKINS);
		return;
	}
	if (_vm->_globals._saveData->data[svField356] == 1
	        && !_vm->_globals.PLAN_FLAG
	        && (uint16)(destX - 533) <= 26
	        && (uint16)(destY - 26) <= 22) {
		changeCharacterHead(CHARACTER_SAMANTHA, CHARACTER_HOPKINS);
		return;
	}
	if (_vm->_globals._saveData->data[svField357] == 1) {
		if (_vm->_globals._saveData->data[svField353] == 1
		        && !_vm->_globals.PLAN_FLAG
		        && (uint16)(destX - 533) <= 26
		        && (uint16)(destY - 26) <= 33) {
			changeCharacterHead(CHARACTER_HOPKINS, CHARACTER_HOPKINS_CLONE);
			return;
		}
		if (_vm->_globals._saveData->data[svField355] == 1
		        && !_vm->_globals.PLAN_FLAG
		        && (uint16)(destX - 567) <= 26
		        && (uint16)(destY - 26) <= 33) {
			changeCharacterHead(CHARACTER_HOPKINS, CHARACTER_SAMANTHA);
			return;
		}
	}
	if (_vm->_globals.PLAN_FLAG) {
		if (!_vm->_globals.GOACTION)
			goto LABEL_38;
		checkZone();
		if (NUMZONE <= 0)
			return;
		v2 = 0;
		v3 = _vm->_globals.essai2;
		v4 = _vm->_globals.chemin;
		do {
			v3[v2] = v4[v2];
			++v2;
		} while (v4[v2] != -1);

		v5 = _vm->_globals.essai2;
		v5[v2] = -1;
		v5[v2 + 1] = -1;
		v5[v2 + 2] = -1;
		v5[v2 + 3] = -1;
	}
	if (_vm->_globals.GOACTION) {
		checkZone();
		_vm->_globals.GOACTION = false;
		_vm->_globals._saveData->data[svField1] = 0;
		_vm->_globals._saveData->data[svField2] = 0;
	}
LABEL_38:
	if (_vm->_globals.PLAN_FLAG && (_vm->_eventsManager._mouseCursorId != 4 || NUMZONE <= 0))
		return;
	if ((uint16)(NUMZONE + 1) > 1u) {
		zoneCount = NUMZONE;
		zoneX = _vm->_globals.ZONEP[zoneCount]._destX;
		if (zoneX) {
			zoneY = _vm->_globals.ZONEP[zoneCount]._destY;
			if (zoneY) {
				if (zoneY != 31) {
					destX = zoneX;
					destY = zoneY;
				}
			}
		}
	}
	_vm->_globals.GOACTION = false;
	v9 = _vm->_globals.chemin;
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	if (_vm->_globals._forestFl && ((uint16)(NUMZONE - 20) <= 1u || (uint16)(NUMZONE - 22) <= 1u)) {
		if (getSpriteY(0) <= 374 || getSpriteY(0) > 410) {
			v10 = getSpriteX(0);
			v11 = getSpriteY(0);
			v12 = getSpriteX(0);
			v13 = _vm->_linesManager.PARCOURS2(v12, v11, v10, 390);
			_vm->_globals.chemin = v13;
			if (v13 != (int16 *)g_PTRNUL)
				PACOURS_PROPRE(v13);
			g_old_x = getSpriteX(0);
			g_old_y = getSpriteY(0);
			_vm->_globals.Compteur = 0;
			if (_vm->_globals.chemin != (int16 *)g_PTRNUL || v9 == _vm->_globals.chemin) {
				_vm->_globals.g_old_sens = -1;
				goto LABEL_65;
			}
			goto LABEL_63;
		}
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
		setSpriteIndex(0, _vm->_globals.g_old_sens2 + 59);
		_vm->_globals._actionDirection = 0;
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
		VERIFTAILLE();
		setFlipSprite(0, false);
		_vm->_globals.Compteur = 0;
		_vm->_globals.g_old_sens = -1;
		goto LABEL_65;
	}
	if (!_vm->_globals.NOMARCHE) {
		if (!_vm->_globals.PLAN_FLAG) {
			srcY = getSpriteY(0);
			srcX = getSpriteX(0);
			v16 = _vm->_linesManager.PARCOURS2(srcX, srcY, destX, destY);
			_vm->_globals.chemin = v16;
			if (v16 != (int16 *)g_PTRNUL)
				PACOURS_PROPRE(v16);
			g_old_x = getSpriteX(0);
			g_old_y = getSpriteY(0);
			_vm->_globals.Compteur = 0;
			if (_vm->_globals.chemin != (int16 *)g_PTRNUL || v9 == _vm->_globals.chemin)
				_vm->_globals.g_old_sens = -1;
			else
LABEL_63:
				_vm->_globals.chemin = v9;
		}
LABEL_65:
		if (!_vm->_globals.NOMARCHE && _vm->_globals.PLAN_FLAG) {
			v17 = getSpriteY(0);
			v18 = getSpriteX(0);
			_vm->_globals.chemin = PARC_VOITURE(v18, v17, destX, destY);
		}
	}
	if ((uint16)(NUMZONE + 1) > 1u) {
		// TODO: Reformat the weird if statement generated by the decompiler
		if (_vm->_eventsManager._mouseCursorId == 23 || (_vm->_globals._saveData->data[svField1] = _vm->_eventsManager._mouseCursorId, _vm->_eventsManager._mouseCursorId == 23))
			_vm->_globals._saveData->data[svField1] = 5;
		if (_vm->_globals.PLAN_FLAG)
			_vm->_globals._saveData->data[svField1] = 6;
		_vm->_globals._saveData->data[svField2] = NUMZONE;
		_vm->_globals._saveData->data[svField3] = _vm->_globals._curObjectIndex;
		_vm->_globals.GOACTION = true;
	}
	_vm->_fontManager.hideText(5);
	_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
	if (_vm->_globals._screenId == 20 && _vm->_globals._saveData->data[svField132] == 1
				&& _vm->_globals._curObjectIndex == 20 && NUMZONE == 12
				&& _vm->_eventsManager._mouseCursorId == 23) {
		// Special case for throwing darts at the switch in Purgatory - the player shouldn't move
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
		getSpriteX(0);
		getSpriteY(0);
	}
}

void ObjectsManager::PARADISE() {
	int v1;
	char result;
	int v3;
	int v5;

	v1 = 0;
	result = _vm->_globals._saveData->data[svField1];
	if (result && _vm->_globals._saveData->data[svField2] && result != 4 && result > 3) {
		_vm->_fontManager.hideText(5);
		if (!_vm->_globals._forestFl || ((uint16)(NUMZONE - 20) > 1u && (uint16)(NUMZONE - 22) > 1u)) {
			if (_vm->_graphicsManager._largeScreenFl) {
				_vm->_graphicsManager.no_scroll = 2;
				if (_vm->_eventsManager._startPos.x >= getSpriteX(0) - 320)
					goto LABEL_64;
				v3 = _vm->_eventsManager._startPos.x + 320 - getSpriteX(0);
				if (v3 < 0)
					v3 = -v3;
				if (v3 <= 160) {
LABEL_64:
					if (_vm->_eventsManager._startPos.x > getSpriteX(0) - 320) {
						v5 = _vm->_eventsManager._startPos.x + 320 - getSpriteX(0);
						if (v5 < 0)
							v5 = -v5;
						if (v5 > 160) {
							_vm->_graphicsManager.no_scroll = 2;
							do {
								_vm->_graphicsManager.SCROLL -= _vm->_graphicsManager._scrollSpeed;
								if (_vm->_graphicsManager.SCROLL < 0) {
									_vm->_graphicsManager.SCROLL = 0;
									v1 = 1;
								}
								if (_vm->_graphicsManager.SCROLL > SCREEN_WIDTH) {
									_vm->_graphicsManager.SCROLL = SCREEN_WIDTH;
									v1 = 1;
								}
								if (_vm->_eventsManager.getMouseX() > _vm->_graphicsManager.SCROLL + 620)
									_vm->_eventsManager.setMouseXY(_vm->_eventsManager._mousePos.x - 4, _vm->_eventsManager.getMouseY());

								_vm->_eventsManager.VBL();
							} while (v1 != 1 && _vm->_eventsManager._startPos.x > getSpriteX(0) - 320);
						}
					}
				} else {
					do {
						_vm->_graphicsManager.SCROLL += _vm->_graphicsManager._scrollSpeed;
						if (_vm->_graphicsManager.SCROLL < 0) {
							_vm->_graphicsManager.SCROLL = 0;
							v1 = 1;
						}
						if (_vm->_graphicsManager.SCROLL > SCREEN_WIDTH) {
							_vm->_graphicsManager.SCROLL = SCREEN_WIDTH;
							v1 = 1;
						}
						if (_vm->_eventsManager.getMouseX() < _vm->_graphicsManager.SCROLL + 10)
							_vm->_eventsManager.setMouseXY(_vm->_eventsManager._mousePos.x + 4, _vm->_eventsManager.getMouseY());

						_vm->_eventsManager.VBL();
					} while (v1 != 1 && _vm->_eventsManager._startPos.x < getSpriteX(0) - 320);
				}
				if (_vm->_eventsManager.getMouseX() > _vm->_graphicsManager.SCROLL + 620)
					_vm->_eventsManager.setMouseXY(_vm->_graphicsManager.SCROLL + 610, 0);
				if (_vm->_eventsManager.getMouseX() < _vm->_graphicsManager.SCROLL + 10)
					_vm->_eventsManager.setMouseXY(_vm->_graphicsManager.SCROLL + 10, 0);
				_vm->_eventsManager.VBL();
				_vm->_graphicsManager.no_scroll = 0;
			}
			_vm->_talkManager.REPONSE(_vm->_globals._saveData->data[svField2], _vm->_globals._saveData->data[svField1]);
		} else {
			_vm->_talkManager.REPONSE2(_vm->_globals._saveData->data[svField2], _vm->_globals._saveData->data[svField1]);
		}
		_vm->_eventsManager.changeMouseCursor(4);
		if ((uint16)(NUMZONE + 1) > 1u && !_vm->_globals.ZONEP[NUMZONE].field16) {
			NUMZONE = -1;
			_forceZoneFl = true;
		}
		if (NUMZONE != _vm->_globals._saveData->data[svField2] || (uint16)(NUMZONE + 1) <= 1u) {
			_vm->_eventsManager._mouseCursorId = 4;
			_changeVerbFl = false;
		} else {
			_vm->_eventsManager._mouseCursorId = _vm->_globals._saveData->data[svField1];
			if (_changeVerbFl) {
				VERBEPLUS();
				_changeVerbFl = false;
			}
			if (_vm->_eventsManager._mouseCursorId == 5)
				_vm->_eventsManager._mouseCursorId = 4;
		}
		if (_vm->_eventsManager._mouseCursorId != 23)
			_vm->_eventsManager.changeMouseCursor(_vm->_eventsManager._mouseCursorId);
		NUMZONE = 0;
		_vm->_globals._saveData->data[svField1] = 0;
		_vm->_globals._saveData->data[svField2] = 0;
	}
	if (_vm->_globals.PLAN_FLAG) {
		_vm->_eventsManager._mouseCursorId = 0;
		_vm->_eventsManager.changeMouseCursor(0);
	}
	if (_vm->_globals.NOMARCHE) {
		if (_vm->_eventsManager._mouseCursorId == 4) {
			result = NUMZONE + 1;
			if ((uint16)(NUMZONE + 1) > 1u)
				handleRightButton();
		}
	}
	_vm->_globals.GOACTION = false;
}

// Clear Screen
void ObjectsManager::CLEAR_ECRAN() {
	clearSprite();
	_vm->_graphicsManager.FIN_VISU();
	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(9);
	_vm->_globals.CLEAR_VBOB();
	_vm->_animationManager.clearAnim();
	_vm->_linesManager.CLEAR_ZONE();
	_vm->_linesManager.RESET_OBSTACLE();
	_vm->_globals.resetCache();

	for (int v1 = 0; v1 <= 48; v1++) {
		_vm->_globals.BOBZONE[v1] = 0;
		_vm->_globals.BOBZONE_FLAG[v1] = false;
	}
	_vm->_eventsManager._mouseCursorId = 4;
	_verb = 4;
	NUMZONE = 0;
	Vold_taille = 0;
	SPEED_PTR = g_PTRNUL;
	SPEED_X = 0;
	SPEED_Y = 0;
	SPEED_IMAGE = 0;
	_forceZoneFl = true;
	_vm->_linesManager._linesNumb = 0;
	_lastLine = 0;
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	_vm->_globals.COUCOU = _vm->_globals.freeMemory(_vm->_globals.COUCOU);
	_vm->_globals.SPRITE_ECRAN = _vm->_globals.freeMemory(_vm->_globals.SPRITE_ECRAN);
	_vm->_eventsManager._startPos.x = 0;
	_vm->_eventsManager._mouseSpriteId = 0;
	Vold_taille = 200;
	_vm->_globals._saveData->data[svField1] = 0;
	_vm->_globals._saveData->data[svField2] = 0;
	_vm->_globals.GOACTION = false;
	_forceZoneFl = true;
	_changeVerbFl = false;
	_vm->_globals.NOSPRECRAN = false;
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	_vm->_globals.g_old_sens = -1;
	my_anim = 1;
	A_ANIM = 0;
	MA_ANIM = 0;
	MA_ANIM1 = 0;
	A_DEPA = 0;
	MAX_DEPA = 0;
	MAX_DEPA1 = 0;
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
}

/**
 * Change character Face / Head
 */
void ObjectsManager::changeCharacterHead(PlayerCharacter oldCharacter, PlayerCharacter newCharacter) {
	CharacterLocation *loc;

	CH_TETE = true;
	_vm->_graphicsManager.SCOPY(_vm->_graphicsManager._vesaScreen, 532, 25, 65, 40, _vm->_graphicsManager._vesaBuffer, 532, 25);
	_vm->_graphicsManager.addVesaSegment(532, 25, 597, 65);
	_vm->_globals.NOT_VERIF = true;
	_vm->_globals.chemin = (int16 *)g_PTRNUL;

	if (oldCharacter == CHARACTER_SAMANTHA && newCharacter == CHARACTER_HOPKINS
		&& _vm->_globals._saveData->_realHopkins._location == _vm->_globals._screenId) {
		CH_TETE = false;
		loc = &_vm->_globals._saveData->_samantha;
		loc->_pos.x = getSpriteX(0);
		loc->_pos.y = getSpriteY(0);
		loc->field2 = 64;
		loc->_location = _vm->_globals._screenId;
		loc->field4 = _sprite[0]._animationType;

		removeSprite(1);
		SPRITE(_vm->_globals.TETE, loc->_pos, 1, 3, loc->field4, 0, 20, 127);
		SPRITE_ON(1);
		removeSprite(0);

		_vm->_globals._saveData->data[svField354] = 0;
		_vm->_globals._saveData->data[svField356] = 0;
		_vm->_globals._saveData->data[svField357] = 1;
		T_RECTIF = 0;

		loc = &_vm->_globals._saveData->_realHopkins;
		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "PERSO.SPR");
		_vm->_globals.PERSO = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
		_vm->_globals.PERSO_TYPE = 0;
		SPRITE(_vm->_globals.PERSO, loc->_pos, 0, 64, loc->field4, 0, 34, 190);
		SPRITE_ON(0);
		_vm->_globals.loadCharacterData();
	} else if (oldCharacter == CHARACTER_HOPKINS && newCharacter == CHARACTER_SAMANTHA
			&& _vm->_globals._saveData->_samantha._location == _vm->_globals._screenId) {
		CH_TETE = false;
		loc = &_vm->_globals._saveData->_realHopkins;
		loc->_pos.x = getSpriteX(0);
		loc->_pos.y = getSpriteY(0);
		loc->field2 = 64;
		loc->_location = _vm->_globals._screenId;
		loc->field4 = _sprite[0].fieldC;

		removeSprite(1);
		SPRITE(_vm->_globals.TETE, loc->_pos, 1, 2, loc->field4, 0, 34, 190);
		SPRITE_ON(1);
		removeSprite(0);

		_vm->_globals._saveData->data[svField354] = 0;
		_vm->_globals._saveData->data[svField356] = 1;
		_vm->_globals._saveData->data[svField357] = 0;

		loc = &_vm->_globals._saveData->_samantha;
		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "PSAMAN.SPR");
		_vm->_globals.PERSO = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
		_vm->_globals.PERSO_TYPE = 2;
		SPRITE(_vm->_globals.PERSO, loc->_pos, 0, 64, loc->field4, 0, 20, 127);
		SPRITE_ON(0);
		_vm->_globals.loadCharacterData();
	} else {
		switch (oldCharacter) {
		case CHARACTER_HOPKINS:
			loc = &_vm->_globals._saveData->_realHopkins;
			loc->_pos.x = getSpriteX(0);
			loc->_pos.y = getSpriteY(0);
			loc->field2 = 64;
			loc->_location = _vm->_globals._screenId;
			loc->field4 = _sprite[0].fieldC;
			break;
		case CHARACTER_HOPKINS_CLONE:
			loc = &_vm->_globals._saveData->_cloneHopkins;
			loc->_pos.x = getSpriteX(0);
			loc->_pos.y = getSpriteY(0);
			loc->field2 = 64;
			loc->_location = _vm->_globals._screenId;
			loc->field4 = _sprite[0].fieldC;
			break;
		case CHARACTER_SAMANTHA:
			loc = &_vm->_globals._saveData->_samantha;
			loc->_pos.x = getSpriteX(0);
			loc->_pos.y = getSpriteY(0);
			loc->field2 = 64;
			loc->_location = _vm->_globals._screenId;
			loc->field4 = _sprite[0].fieldC;
			break;
		default:
			break;
		}

		switch (newCharacter) {
		case CHARACTER_HOPKINS:
			_vm->_globals._saveData->data[svField121] = 0;
			_vm->_globals._saveData->data[svField354] = 0;
			_vm->_globals._saveData->data[svField356] = 0;
			_vm->_globals._saveData->data[svField357] = 1;
			_vm->_globals._exitId = _vm->_globals._saveData->_realHopkins._location;
			break;
		case CHARACTER_HOPKINS_CLONE:
			_vm->_globals._saveData->data[svField121] = 1;
			_vm->_globals._saveData->data[svField354] = 1;
			_vm->_globals._saveData->data[svField356] = 0;
			_vm->_globals._saveData->data[svField357] = 0;
			_vm->_globals._exitId = _vm->_globals._saveData->_cloneHopkins._location;
			break;
		case CHARACTER_SAMANTHA:
			_vm->_globals._saveData->data[svField121] = 0;
			_vm->_globals._saveData->data[svField354] = 0;
			_vm->_globals._saveData->data[svField356] = 1;
			_vm->_globals._saveData->data[svField357] = 0;
			_vm->_globals._exitId = _vm->_globals._saveData->_samantha._location;
			break;
		}
	}
}

// Check Size
void ObjectsManager::VERIFTAILLE() {
	int v0 = _vm->_globals.STAILLE[getSpriteY(0)];
	if (_vm->_globals.PERSO_TYPE == 1) {
		if (v0 < 0)
			v0 = -v0;
		v0 = 20 * (5 * v0 - 100) / -80;
	} else if (_vm->_globals.PERSO_TYPE == 2) {
		if (v0 < 0)
			v0 = -v0;
		v0 = 20 * (5 * v0 - 165) / -67;
	}
	SETTAILLESPR(0, v0);
}

void ObjectsManager::PACOURS_PROPRE(int16 *a1) {
	int v1;
	int v2;
	int v4;
	int v5;
	int v6;
	int v7;
	int v9;
	int v10;
	int v11;
	int v12;
	int v13;
	int v14;
	int v15;

	v1 = 0;
	v13 = 0;
	v14 = -1;
	v2 = a1[1];
	v15 = a1[2];
	if (a1[0] == -1 && v2 == -1)
		return;

	for (;;) {
		if (v14 != -1 && v15 != v14) {
			v11 = v1;
			v12 = 0;
			v10 = CALC_PROPRE(v2);
			v4 = a1[v1];
			v9 = a1[v1];
			v5 = a1[v1 + 1];
			v6 = 0;
			while (v4 != -1 || v5 != -1) {
				int idx = v1;
				v1 += 4;
				++v12;
				if (a1[idx + 2] != v15)
					v6 = 1;
				if (v6 == 1)
					break;
				v4 = a1[v1];
				v9 = a1[v1];
				v5 = a1[v1 + 1];
			}
			if (v12 < v10) {
				v7 = v11;
				for (int v8 = 0; v8 < v12; v8++) {
					a1[v7 + 2] = v14;
					v7 += 4;
				}
				v15 = v14;
			}
			v1 = v11;
			if (v9 == -1 && v5 == -1)
				v13 = 1;
		}
		v1 += 4;
		if (v13 == 1)
			break;
		v14 = v15;
		v2 = a1[v1 + 1];
		v15 = a1[v1 + 2];
		if (a1[v1] == -1) {
			if (v2 == -1)
				break;
		}
	}
}

int16 *ObjectsManager::PARC_VOITURE(int a1, int a2, int a3, int a4) {
	int v4;
	int v5;
	int v6;
	int v7;
	int v8;
	int v10;
	int v11;
	int v12;
	int v14;
	int v15;
	int v16;
	int v18;
	int v19;
	int v20;
	int16 *result;
	int v23;
	int v27;
	int v28;
	int v29;
	int v32;
	int16 *v33;
	int v34;
	int v35;
	int16 *v37;
	int v39;
	int16 *v41;
	int16 *v45;
	int v47;
	int v48;
	int16 *v50;
	int v52;
	int16 *v54;
	int16 *v58;
	int v62;
	int v63;
	int v64;
	int v65;
	int v66;
	int v67;
	int v68 = 0;
	int v69 = 0;
	int j;
	int v72 = 0;
	int v73 = 0;
	int v74;
	int v75;
	int v77[10];
	int v82[10];
	int v87[10];

	v4 = a3;
	v5 = a4;
	v67 = 0;
	if (a3 <= 14)
		v4 = 15;
	if (a4 <= 14)
		v5 = 15;
	if (v4 > _vm->_graphicsManager.max_x - 10)
		v4 = _vm->_graphicsManager.max_x - 10;
	if (v5 > 445)
		v5 = 440;
	v75 = v4;
	v74 = v5;
	v6 = 0;
	v7 = v74;
	if (_vm->_graphicsManager.max_y > v74) {
		v8 = 5;
		do {
			v62 = v8;
			v8 = v62;
			if (_vm->_linesManager.colision2_ligne(v75, v7, &v82[5], &v87[5], 0, _lastLine) && v87[v62] <= _lastLine)
				break;
			v82[v62] = 0;
			v87[v62] = -1;
			++v6;
			++v7;
		} while (_vm->_graphicsManager.max_y > v7);
	}
	v77[5] = v6;
	v10 = 0;
	v11 = v74;
	if (_vm->_graphicsManager.min_y < v74) {
		v12 = 1;
		do {
			v63 = v12;
			v12 = v63;
			if (_vm->_linesManager.colision2_ligne(v75, v11, &v82[1], &v87[1], 0, _lastLine) && v87[v63] <= _lastLine)
				break;
			v82[v63] = 0;
			v87[v63] = -1;
			if (v77[5] < v10) {
				if (v87[5] != -1)
					break;
			}
			++v10;
			--v11;
		} while (_vm->_graphicsManager.min_y < v11);
	}
	v77[1] = v10;
	v14 = 0;
	v15 = v75;
	if (_vm->_graphicsManager.max_x > v75) {
		v16 = 3;
		do {
			v64 = v16;
			v16 = v64;
			if (_vm->_linesManager.colision2_ligne(v15, v74, &v82[3], &v87[3], 0, _lastLine) && v87[v64] <= _lastLine)
				break;
			v82[v64] = 0;
			v87[v64] = -1;
			++v14;
			if (v77[1] < v14) {
				if (v87[1] != -1)
					break;
			}
			if (v77[5] < v14 && v87[5] != -1)
				break;
			++v15;
		} while (_vm->_graphicsManager.max_x > v15);
	}
	v77[3] = v14;
	v18 = 0;
	v19 = v75;
	if (_vm->_graphicsManager.min_x < v75) {
		v20 = 7;
		do {
			v65 = v20;
			v20 = v65;
			if (_vm->_linesManager.colision2_ligne(v19, v74, &v82[7], &v87[7], 0, _lastLine) && v87[v65] <= _lastLine)
				break;
			v82[v65] = 0;
			v87[v65] = -1;
			++v18;
			if (v77[1] < v18) {
				if (v87[1] != -1)
					break;
			}
			if (v77[5] < v18 && v87[5] != -1)
				break;
			if (v77[3] < v18 && v87[3] != -1)
				break;
			--v19;
		} while (_vm->_graphicsManager.min_x < v19);
	}
	v77[7] = v18;
	if (v87[1] == -1)
		v77[1] = 1300;
	if (v87[3] == -1)
		v77[3] = 1300;
	if (v87[5] == -1)
		v77[5] = 1300;
	if (v87[7] == -1)
		v77[7] = 1300;
	if (v87[1] != -1 || v87[3] != -1 || v87[5] != -1 || v87[7] != -1) {
		v23 = 0;
		if (v87[5] != -1 && v77[1] >= v77[5] && v77[3] >= v77[5] && v77[7] >= v77[5]) {
			v73 = v87[5];
			v72 = v82[5];
			v23 = 1;
		}
		if (v87[1] != -1 && !v23 && v77[5] >= v77[1] && v77[3] >= v77[1] && v77[7] >= v77[1]) {
			v73 = v87[1];
			v72 = v82[1];
			v23 = 1;
		}
		if (v87[3] != -1 && !v23 && v77[1] >= v77[3] && v77[5] >= v77[3] && v77[7] >= v77[3]) {
			v73 = v87[3];
			v72 = v82[3];
			v23 = 1;
		}
		if (v87[7] != -1 && !v23 && v77[5] >= v77[7] && v77[3] >= v77[7] && v77[1] >= v77[7]) {
			v73 = v87[7];
			v72 = v82[7];
		}
		for (int v24 = 0; v24 <= 8; v24++) {
			v87[v24] = -1;
			v82[v24] = 0;
			v77[v24] = 1300;
		}
		if (_vm->_linesManager.colision2_ligne(a1, a2, &v82[1], &v87[1], 0, _lastLine)) {
			v69 = v87[1];
			v68 = v82[1];
		} else if (_vm->_linesManager.colision2_ligne(a1, a2, &v82[1], &v87[1], 0, _vm->_linesManager._linesNumb)) {
			v27 = 0;
			for (;;) {
				v28 = _vm->_globals.essai2[v27];
				v29 = _vm->_globals.essai2[v27 + 1];
				v66 = _vm->_globals.essai2[v27 + 2];
				v27 += 4;
				if (_vm->_linesManager.colision2_ligne(v28, v29, &v82[1], &v87[1], 0, _lastLine))
					break;
				v32 = v67;
				_vm->_globals.super_parcours[v32] = v28;
				_vm->_globals.super_parcours[v32 + 1] = v29;
				_vm->_globals.super_parcours[v32 + 2] = v66;
				_vm->_globals.super_parcours[v32 + 3] = 0;

				v33 = _vm->_globals.essai0;
				_vm->_globals.essai0[v32] = v28;
				v33[v32 + 1] = v29;
				v33[v32 + 2] = v66;
				v33[v32 + 3] = 0;
				v67 += 4;
				if (v28 == -1)
					break;;
			}
			if (v28 != -1) {
				v69 = v87[1];
				v68 = v82[1];
			}
		} else {
			v69 = 1;
			v68 = 1;
			v67 = 0;
		}
LABEL_90:
		if (v69 < v73) {
			v34 = v68;
			v35 = v68;
			for (int i = _vm->_linesManager.Ligne[v69].field0; v35 < i - 2; i = _vm->_linesManager.Ligne[v69].field0) {
				v37 = _vm->_linesManager.Ligne[v69].lineData;
				v39 = v67;
				_vm->_globals.super_parcours[v39] = v37[2 * v35];
				_vm->_globals.super_parcours[v39 + 1] = v37[2 * v35 + 1];
				_vm->_globals.super_parcours[v39 + 2] = _vm->_linesManager.Ligne[v69].field6;
				_vm->_globals.super_parcours[v39 + 3] = 0;
				v67 += 4;
				++v34;
				v35 = v34;
			}
			for (j = v69 + 1; j < v73; ++j) {
				if (_vm->_linesManager.PLAN_TEST(
						_vm->_linesManager.Ligne[j].lineData[0],
						_vm->_linesManager.Ligne[j].lineData[1],
				        v67,
				        j,
				        v73,
				        0) == 1) {
LABEL_88:
					v69 = _vm->_linesManager.NV_LIGNEDEP;
					v68 = _vm->_linesManager.NV_LIGNEOFS;
					v67 = _vm->_linesManager.NV_POSI;
					goto LABEL_90;
				}
				if (_vm->_linesManager.Ligne[j].field0 - 2 > 0) {
					for (int v40 = 0; v40 < _vm->_linesManager.Ligne[j].field0 - 2; v40++) {
						v41 = _vm->_linesManager.Ligne[j].lineData;
						_vm->_globals.super_parcours[v67] = v41[2 * v40];
						_vm->_globals.super_parcours[v67 + 1] = v41[2 * v40 + 1];
						_vm->_globals.super_parcours[v67 + 2] = _vm->_linesManager.Ligne[j].field6;
						_vm->_globals.super_parcours[v67 + 3] = 0;
						v67 += 4;
					}
				}
			}
			v68 = 0;
			v69 = v73;
		}
		if (v69 > v73) {
			for (int k = v68; k > 0; --k) {
				v45 = _vm->_linesManager.Ligne[v69].lineData;
				v47 = v67;
				_vm->_globals.super_parcours[v47] = v45[2 * k];
				_vm->_globals.super_parcours[v47 + 1] = v45[2 * k + 1];
				_vm->_globals.super_parcours[v47 + 2] = _vm->_linesManager.Ligne[v69].field8;
				_vm->_globals.super_parcours[v47 + 3] = 0;
				v67 += 4;
			}
			for (int l = v69 - 1; l > v73; --l) {
				v48 = l;
				if (_vm->_linesManager.PLAN_TEST(
						_vm->_linesManager.Ligne[l].lineData[2 * _vm->_linesManager.Ligne[v48].field0 - 2],
						_vm->_linesManager.Ligne[l].lineData[2 * _vm->_linesManager.Ligne[v48].field0 - 1],
				        v67,
				        l,
				        v73,
				        0) == 1)
					goto LABEL_88;
				
				for (int v49 = _vm->_linesManager.Ligne[v48].field0 - 2; v49 > 0; v49 --) {
					v50 = _vm->_linesManager.Ligne[l].lineData;
					v52 = v67;
					_vm->_globals.super_parcours[v52] = v50[2 * v49];
					_vm->_globals.super_parcours[v52 + 1] = v50[2 * v49 + 1];
					_vm->_globals.super_parcours[v52 + 2] = _vm->_linesManager.Ligne[l].field8;
					_vm->_globals.super_parcours[v52 + 3] = 0;
					v67 += 4;
				}
			}
			v68 = _vm->_linesManager.Ligne[v73].field0 - 1;
			v69 = v73;
		}
		if (v69 == v73) {
			if (v68 <= v72) {
				for (int v57 = v68; v57 < v72; v57++) {
					v58 = _vm->_linesManager.Ligne[v73].lineData;
					_vm->_globals.super_parcours[v67] = v58[2 * v57];
					_vm->_globals.super_parcours[v67 + 1] = v58[2 * v57 + 1];
					_vm->_globals.super_parcours[v67 + 2] = _vm->_linesManager.Ligne[v73].field6;
					_vm->_globals.super_parcours[v67 + 3] = 0;
					v67 += 4;
				}
			} else {
				for (int v53 = v68; v53 > v72; v53--) {
					v54 = _vm->_linesManager.Ligne[v73].lineData;
					_vm->_globals.super_parcours[v67] = v54[2 * v53];
					_vm->_globals.super_parcours[v67 + 1] = v54[2 * v53 + 1];
					_vm->_globals.super_parcours[v67 + 2] = _vm->_linesManager.Ligne[v73].field8;
					_vm->_globals.super_parcours[v67 + 3] = 0;
					v67 += 4;
				}
			}
		}
		_vm->_globals.super_parcours[v67] = -1;
		_vm->_globals.super_parcours[v67 + 1] = -1;
		_vm->_globals.super_parcours[v67 + 2] = -1;
		_vm->_globals.super_parcours[v67 + 3] = -1;
		result = &_vm->_globals.super_parcours[0];
	} else {
		result = (int16 *)g_PTRNUL;
	}
	return result;
}

void ObjectsManager::VERBEPLUS() {
	int v;

	v = _vm->_eventsManager._mouseCursorId + 1;
	_vm->_eventsManager._mouseCursorId = v;
	if (v == 4)
		goto LABEL_24;
	if (v == 5)
		goto LABEL_28;
	if (v == 6)
		goto LABEL_29;
	if (v == 7)
		goto LABEL_31;
	if (v == 8)
		goto LABEL_33;
	if (v == 9)
		goto LABEL_35;
	if (v == 10)
		goto LABEL_37;
	if (v == 11)
		goto LABEL_39;
	if (v == 12)
		goto LABEL_41;
	if (v == 13)
		goto LABEL_43;
	if (v == 14)
		goto LABEL_45;
	if (v == 15)
		goto LABEL_47;
	if (v == 16)
		goto LABEL_49;
	if (v == 17)
		goto LABEL_51;
	if (v == 18)
		goto LABEL_53;
	if (v == 19)
		goto LABEL_55;
	if (v == 20)
		goto LABEL_57;
	if (v == 21)
		goto LABEL_59;
	if (v == 22)
		goto LABEL_61;
	if (v == 23)
		goto LABEL_63;
	if (v == 24)
		goto LABEL_65;
	if (v == 25)
		goto LABEL_67;
	do {
		do {
			_vm->_eventsManager._mouseCursorId = 4;
LABEL_24:
			if (!_vm->_globals.NOMARCHE || (v = NUMZONE + 1, (uint16)(NUMZONE + 1) <= 1u)) {
				if (_vm->_eventsManager._mouseCursorId == 4)
					return;
			} else {
				_vm->_eventsManager._mouseCursorId = 5;
			}
LABEL_28:
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 6) {
LABEL_29:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field6 == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 7) {
LABEL_31:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field7 == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 8) {
LABEL_33:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field8 == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 9) {
LABEL_35:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field9 == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 10) {
LABEL_37:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldA == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 11) {
LABEL_39:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldB == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 12) {
LABEL_41:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldC == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 13) {
LABEL_43:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldD == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 14) {
LABEL_45:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldE == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 15) {
LABEL_47:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldF == 1)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 16) {
LABEL_49:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field6 == 2)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 17) {
LABEL_51:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field9 == 2)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 18) {
LABEL_53:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldA == 2)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 19) {
LABEL_55:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldB == 2)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 20) {
LABEL_57:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldC == 2)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 21) {
LABEL_59:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldF == 2)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 22) {
LABEL_61:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].fieldD == 2)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 23) {
LABEL_63:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field8 == 2)
					return;
			}
			++_vm->_eventsManager._mouseCursorId;
			if (_vm->_eventsManager._mouseCursorId == 24) {
LABEL_65:
				v = 5 * NUMZONE;
				if (_vm->_globals.ZONEP[NUMZONE].field9 == 3)
					return;
			}
			v = _vm->_eventsManager._mouseCursorId + 1;
			_vm->_eventsManager._mouseCursorId = v;
		} while (v != 25);
LABEL_67:
		v = 5 * NUMZONE;
	} while (_vm->_globals.ZONEP[NUMZONE].fieldE != 2);
}

/**
 * Handle Right button
 */
void ObjectsManager::handleRightButton() {
	if ((uint16)(NUMZONE + 1) > 1u) {
		VERBEPLUS();
		if (_vm->_eventsManager._mouseCursorId != 23)
			_vm->_eventsManager.changeMouseCursor(_vm->_eventsManager._mouseCursorId);
		_verb = _vm->_eventsManager._mouseCursorId;
	}
}

int ObjectsManager::MZONE() {
	signed int result;
	int16 v2;
	int v3;
	int16 v4;
	int16 v5;
	int v6;
	int v8;
	int v9;
	int16 yCurrent;
	int16 v11;
	int16 j;
	int16 k;
	int16 xCurrent;
	int v15;
	int16 v16;
	int16 v17;
	int16 v18;
	int16 v19;
	int16 yp;
	int16 xp;

	v19 = 0;
	v18 = 0;
	v17 = 0;
	v16 = 0;
	xp = _vm->_eventsManager._mousePos.x + _vm->_eventsManager._mouseOffset.x;
	yp = _vm->_eventsManager._mousePos.y + _vm->_eventsManager._mouseOffset.y;
	if ((_vm->_eventsManager._mousePos.y + _vm->_eventsManager._mouseOffset.y) > 19) {
		for (int v1 = 0; v1 <= 48; v1++) {
			v2 = _vm->_globals.BOBZONE[v1];
			if (v2) {
				if (_vm->_globals.BOBZONE_FLAG[v1]) {
					v3 = v2;
					v15 = v3;
					if (_vm->_globals._bob[v3].field0) {
						if (_vm->_globals._bob[v3]._frameIndex != 250) {
							if (!_vm->_globals._bob[v3]._disabledAnimationFl) {
								v4 = _vm->_globals._bob[v3]._oldX;
								if (xp > v4) {
									if (xp < _vm->_globals._bob[v3]._oldWidth + v4) {
										v5 = _vm->_globals._bob[v3]._oldY;
										if (yp > v5) {
											if (yp < _vm->_globals._bob[v3]._oldHeight + v5) {
												v6 = v1;
												if (_vm->_globals.ZONEP[v1].field4 == -1) {
													_vm->_globals.ZONEP[v6]._destX = 0;
													_vm->_globals.ZONEP[v1]._destY = 0;
												}
												if (!_vm->_globals.ZONEP[v6]._destX) {
													if (!_vm->_globals.ZONEP[v1]._destY) {
														_vm->_globals.ZONEP[v6]._destX = _vm->_globals._bob[v15]._oldWidth + _vm->_globals._bob[v15]._oldX;
														_vm->_globals.ZONEP[v1]._destY = _vm->_globals._bob[v15]._oldHeight + _vm->_globals._bob[v15]._oldY + 6;
														_vm->_globals.ZONEP[v1].field4 = -1;
													}
												}
												return v1;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		_vm->_globals.SegmentEnCours = 0;
		for (int v7 = 0; v7 <= 99; v7++) {
			if (_vm->_globals.ZONEP[v7].field10 == 1) {
				v8 = v7;
				if (_vm->_globals.CarreZone[v8].field0 == 1) {
					if (_vm->_globals.CarreZone[v7].field2 <= xp
					        && _vm->_globals.CarreZone[v7].field4 >= xp
					        && _vm->_globals.CarreZone[v7].field6 <= yp
					        && _vm->_globals.CarreZone[v7].field8 >= yp) {
								if (_vm->_globals.CarreZone[v7].fieldE == 1) {
									_vm->_globals.oldzone_46 = _vm->_linesManager._zoneLine[_vm->_globals.CarreZone[v7].fieldA].field2;
							return _vm->_globals.oldzone_46;
						}
						v9 = _vm->_globals.SegmentEnCours;
						_vm->_globals.Segment[v9].field2 = _vm->_globals.CarreZone[v7].fieldA;
						_vm->_globals.Segment[v9].field4 = _vm->_globals.CarreZone[v7].fieldC;
						++_vm->_globals.SegmentEnCours;
					}
				}
			}
		}
		if (!_vm->_globals.SegmentEnCours)
			goto LABEL_58;

		for (yCurrent = yp; yCurrent >= 0; --yCurrent) {
			v11 = colision(xp, yCurrent);
			v19 = v11;
			if (v11 != -1 && _vm->_globals.ZONEP[v11].field10 == 1)
				break;
		}
		if (v19 == -1)
			goto LABEL_58;
		for (j = yp; j < _vm->_graphicsManager.max_y; ++j) {
			v18 = colision(xp, j);
			if (v18 != -1 && _vm->_globals.ZONEP[v19].field10 == 1)
				break;
		}
		if (v18 == -1)
			goto LABEL_58;
		for (k = xp; k >= 0; --k) {
			v16 = colision(k, yp);
			if (v16 != -1 && _vm->_globals.ZONEP[v19].field10 == 1)
				break;
		}
		if (v16 == -1)
			goto LABEL_58;

		for (xCurrent = xp; _vm->_graphicsManager.max_x > xCurrent; ++xCurrent) {
			v17 = colision(xCurrent, yp);
			if (v17 != -1 && _vm->_globals.ZONEP[v19].field10 == 1)
				break;
		}
		if (v19 == v18 && v19 == v16 && v19 == v17) {
			_vm->_globals.oldzone_46 = v19;
			result = v19;
		} else {
LABEL_58:
			_vm->_globals.oldzone_46 = -1;
			result = -1;
		}
	} else {
		result = 0;
	}
	return result;
}
/**
 * Prepare border used to highlight the place below mouse cursor, in the inventory.
 * Also set the mouse cursor
 */
void ObjectsManager::initBorder(int a1) {
	_oldBorderPos = _borderPos;
	_oldBorderSpriteIndex = _borderSpriteIndex;
	if ((uint16)(a1 - 1) <= 5u)
		_borderPos.y = 120;
	if ((uint16)(a1 - 7) <= 5u)
		_borderPos.y = 158;
	if ((uint16)(a1 - 13) <= 5u)
		_borderPos.y = 196;
	if ((uint16)(a1 - 19) <= 5u)
		_borderPos.y = 234;
	if ((uint16)(a1 - 25) <= 4u)
		_borderPos.y = 272;
	if (a1 == 1 || a1 == 7 || a1 == 13 || a1 == 19 || a1 == 25)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 158;
	if (a1 == 2 || a1 == 8 || a1 == 14 || a1 == 20 || a1 == 26)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 212;
	if (a1 == 3 || a1 == 9 || a1 == 15 || a1 == 21 || a1 == 27)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 266;
	if (a1 == 4 || a1 == 10 || a1 == 16 || a1 == 22 || a1 == 28)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 320;
	if (a1 == 5 || a1 == 11 || a1 == 17 || a1 == 23 || a1 == 29)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 374;
	if (a1 == 6 || a1 == 12 || a1 == 18 || a1 == 24 || (uint16)(a1 - 30) <= 1u)
		_borderPos.x = _vm->_graphicsManager._scrollOffset + 428;
	if ((uint16)(a1 - 1) <= 28)
		_borderSpriteIndex = 0;
	if ((uint16)(a1 - 30) <= 1)
		_borderSpriteIndex = 2;
	if (a1 == 30)
		_borderPos.y = 272;
	if (a1 == 31)
		_borderPos.y = 290;
	if (!a1 || a1 == 32) {
		_borderPos = Common::Point(0, 0);
		_borderSpriteIndex = 0;
	}
	if (!a1)
		_vm->_eventsManager._mouseCursorId = 0;
	if (a1 == 32)
		_vm->_eventsManager._mouseCursorId = 16;
	if (a1 == 30)
		_vm->_eventsManager._mouseCursorId = 2;
	if (a1 == 31)
		_vm->_eventsManager._mouseCursorId = 3;
	if ((uint16)(a1 - 1) <= 28)
		_vm->_eventsManager._mouseCursorId = 8;
	if (a1 == 29)
		_vm->_eventsManager._mouseCursorId = 1;
	if ((uint16)(a1 - 1) <= 27 && !_vm->_globals._inventory[a1]) {
		_vm->_eventsManager._mouseCursorId = 0;
		_borderPos = Common::Point(0, 0);
		_borderSpriteIndex = 0;
	}
	if (_vm->_eventsManager._mouseCursorId != 23)
		_vm->_eventsManager.changeMouseCursor(_vm->_eventsManager._mouseCursorId);
	_vm->_eventsManager.getMouseX();
	_vm->_eventsManager.getMouseY();
}

void ObjectsManager::OBJETPLUS(int idx) {
	int v1;
	int v2;
	int v3;

	v1 = _vm->_eventsManager._mouseCursorId;
	if (_vm->_eventsManager._mouseCursorId && _vm->_eventsManager._mouseCursorId != 16 && (uint16)(_vm->_eventsManager._mouseCursorId - 2) > 1u) {
		v2 = _vm->_eventsManager._mouseCursorId++ + 1;
		if (v1 == 5)
			goto LABEL_24;
		if (v2 == 7)
			goto LABEL_26;
		if (v2 != 8) {
			if (v2 == 9)
				_vm->_eventsManager._mouseCursorId = 10;
			if (_vm->_eventsManager._mouseCursorId == 10)
				goto LABEL_29;
			if (_vm->_eventsManager._mouseCursorId == 11)
				goto LABEL_31;
			if (_vm->_eventsManager._mouseCursorId == 12)
				_vm->_eventsManager._mouseCursorId = 13;
			if (_vm->_eventsManager._mouseCursorId == 13)
				goto LABEL_33;
			if (_vm->_eventsManager._mouseCursorId == 14)
				_vm->_eventsManager._mouseCursorId = 15;
			if (_vm->_eventsManager._mouseCursorId == 15)
				goto LABEL_35;
			if ((uint16)(_vm->_eventsManager._mouseCursorId - 16) <= 6u)
				_vm->_eventsManager._mouseCursorId = 23;
			if (_vm->_eventsManager._mouseCursorId == 23)
				goto LABEL_37;
			if (_vm->_eventsManager._mouseCursorId == 24)
				_vm->_eventsManager._mouseCursorId = 25;
			if (_vm->_eventsManager._mouseCursorId == 25)
				goto LABEL_39;
			do {
				_vm->_eventsManager._mouseCursorId = 6;
LABEL_24:
				if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field2 == 1)
					break;
				++_vm->_eventsManager._mouseCursorId;
				if (_vm->_eventsManager._mouseCursorId == 7) {
LABEL_26:
					if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field3 == 1)
						return;
				}
				v3 = _vm->_eventsManager._mouseCursorId++;
				if (_vm->_eventsManager._mouseCursorId == 8)
					break;
				_vm->_eventsManager._mouseCursorId = v3 + 3;
				if (v3 == 7) {
LABEL_29:
					if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field7 == 1)
						return;
				}
				++_vm->_eventsManager._mouseCursorId;
				if (_vm->_eventsManager._mouseCursorId == 11) {
LABEL_31:
					if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field4 == 1)
						return;
				}
				_vm->_eventsManager._mouseCursorId += 2;
				if (_vm->_eventsManager._mouseCursorId == 13) {
LABEL_33:
					if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field5 == 1)
						return;
				}
				_vm->_eventsManager._mouseCursorId += 2;
				if (_vm->_eventsManager._mouseCursorId == 15) {
LABEL_35:
					if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field6 == 1)
						return;
				}
				_vm->_eventsManager._mouseCursorId = 23;
LABEL_37:
				if (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field6 == 2)
					break;
				_vm->_eventsManager._mouseCursorId = 25;
LABEL_39:
				;
			} while (_vm->_globals.ObjetW[_vm->_globals._inventory[idx]].field7 != 2);
		}
	}
}

void ObjectsManager::VALID_OBJET(int a1) {
	if (_vm->_eventsManager._mouseCursorId == 8)
		changeObject(a1);
}

void ObjectsManager::OPTI_OBJET() {
	byte *data;
	Common::String file;
	int v0 = 1;
	int v5;

	file = "OBJET1.ini";
	data = _vm->_fileManager.searchCat(file, 1);
	if (data == g_PTRNUL) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, file);
		data = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
		if (data == g_PTRNUL)
			error("INI file %s not found", file.c_str());
	}

	if (data[0] != 'I' || data[1] != 'N' || data[2] != 'I')
		error("File %s is not an INI file", file.c_str());

	bool v7 = false;
	do {
		v5 = _vm->_scriptManager.handleOpcode(data + 20 * v0);
		if (_vm->shouldQuit())
			return;

		if (v5 == 2)
			v0 = _vm->_scriptManager.handleGoto(data + 20 * v0);
		if (v5 == 3)
			v0 = _vm->_scriptManager.handleIf(data, v0);
		if (v0 == -1)
			error("defective IFF function");
		if (v5 == 1 || v5 == 4)
			++v0;
		if (!v5 || v5 == 5)
			v7 = true;
	} while (!v7);

	_vm->_globals.freeMemory(data);
}

void ObjectsManager::handleSpecialGames() {
	byte *oldPalette;
	byte *v2;
	byte *v3;

	switch (_vm->_globals._screenId) {
	case 5:
		if ((getSpriteY(0) > 399) || _vm->_globals._saveData->data[svField173])
			break;

		_vm->_globals._saveData->data[svField173] = 1;
		_vm->_globals.NOPARLE = true;
		_vm->_talkManager.PARLER_PERSO("flicspe1.pe2");
		_vm->_globals.NOPARLE = false;

		if (_vm->_globals._censorshipFl)
			break;

		oldPalette = _vm->_globals.allocMemory(1000);
		memcpy(oldPalette, _vm->_graphicsManager._palette, 769);

		_vm->_saveLoadManager.saveFile("TEMP1.SCR", _vm->_graphicsManager._vesaScreen, 307200);

		if (!_vm->_graphicsManager._lineNbr)
			_vm->_graphicsManager._scrollOffset = 0;
		_vm->_graphicsManager.NB_SCREEN(true);
		_vm->_soundManager.SPECIAL_SOUND = 198;
		PERSO_ON = true;
		_vm->_animationManager.NO_SEQ = true;
		_vm->_animationManager._clearAnimationFl = false;
		_vm->_animationManager.playAnim("otage.ANM", 1, 24, 500);
		_vm->_animationManager.NO_SEQ = false;
		_vm->_soundManager.SPECIAL_SOUND = 0;
		_vm->_graphicsManager.NB_SCREEN(false);

		_vm->_saveLoadManager.load("TEMP1.SCR", _vm->_graphicsManager._vesaScreen);
		g_system->getSavefileManager()->removeSavefile("TEMP1.SCR");

		PERSO_ON = false;
		memcpy(_vm->_graphicsManager._palette, oldPalette, 769);
		_vm->_graphicsManager.SHOW_PALETTE();
		_vm->_globals.freeMemory(oldPalette);
		_vm->_graphicsManager.lockScreen();
		_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsManager.unlockScreen();
		v2 = _vm->_graphicsManager._vesaBuffer;
		v3 = _vm->_graphicsManager._vesaScreen;
		memcpy(_vm->_graphicsManager._vesaBuffer, _vm->_graphicsManager._vesaScreen, 614396);
		v3 = v3 + 614396;
		v2 = v2 + 614396;
		*v2 = *v3;
		v2 = v2 + 2;
		*v2 = v3[2];

		_vm->_graphicsManager.no_scroll = 0;
		_vm->_graphicsManager.DD_VBL();
		break;
	case 20:
		_vm->_globals._saveData->data[svField132] = (getSpriteX(0) > 65 && getSpriteX(0) <= 124 && getSpriteY(0) > 372 && getSpriteY(0) <= 398) ? 1 : 0;
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
		if (_vm->_globals._saveData->data[svField261] == 1 && BOBPOSI(5) == 37) {
			stopBobAnimation(5);
			SET_BOBPOSI(5, 0);
			setBobAnimation(6);
			_vm->_globals._saveData->data[svField261] = 2;
			disableZone(15);
			_vm->_soundManager.playSound("SOUND75.WAV");
		}
		if (_vm->_globals._saveData->data[svField261] == 2 && BOBPOSI(6) == 6) {
			stopBobAnimation(6);
			SET_BOBPOSI(6, 0);
			setBobAnimation(7);
			ZONE_ON(14);
			_vm->_globals._saveData->data[svField261] = 3;
		}
		_vm->_globals._disableInventFl = false;
		break;
	case 93:
		if (_vm->_globals._saveData->data[svField333])
			break;

		_vm->_globals._disableInventFl = true;
		do
			_vm->_eventsManager.VBL();
		while (BOBPOSI(8) != 3);
		_vm->_globals.NOPARLE = true;
		_vm->_talkManager.PARLER_PERSO("GM3.PE2");
		stopBobAnimation(8);
		_vm->_globals._saveData->data[svField333] = 1;
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

		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "DOS.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,-1,", 0, 0, 8, 0);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,8,9,10,11,12,13,12,11,12,13,12,11,12,13,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,-1,", 0, 0, 8, 0);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "20,19,18,17,16,15,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,22,23,24,-1,", 0, 0, 8, 0);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "23,22,21,20,19,18,17,16,15,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,22,23,24,-1,", 0, 0, 8, 0);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "23,22,21,20,19,18,17,16,15,-1,", 0, 0, 8);
}

void ObjectsManager::ACTION_DROITE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 3) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 3;
		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "PROFIL.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 0, 0, 8, 0);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,-1,", 0, 0, 8, 0);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 0, 0, 8, 0);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "23,24,25,-1,", 0, 0, 8, 0);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "24,,23,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,-1,", 0, 0, 8, 0);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "26,25,24,23,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,28,29,-1,", 0, 0, 8, 0);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "28,27,26,25,24,23,-1,", 0, 0, 8);
}

void ObjectsManager::Q_DROITE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 4) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 4;
		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "3Q.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 8, 0);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,-1,", 0, 0, 8, 0);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "17,16,15,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20-1,", 0, 0, 8, 0);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "19,18,17,16,15,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,-1,", 0, 0, 8, 0);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "20,19,18,17,15,-1,", 0, 0, 8);
}

void ObjectsManager::ACTION_FACE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 2) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 2;
		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "FACE.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,9,9,9,9,9,9,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,15,-1,", 0, 0, 8, 0);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,9,10,11,12,13,14,13,12,11,10,9,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 0);
}

void ObjectsManager::Q_GAUCHE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 4) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 4;
		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "3Q.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 1);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 8, 1);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "0,1,2,3,4,5,6,7,8,9,10,11,12,11,12,11,12,11,12,11,10,9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 8, 1);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,-1,", 0, 0, 8, 1);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "17,16,15,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,-1,", 0, 0, 8, 1);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "19,18,17,16,15,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "15,16,17,18,19,20,21,-1,", 0, 0, 8, 1);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "20,19,18,17,15,-1,", 0, 0, 8);
}

void ObjectsManager::ACTION_GAUCHE(int idx) {
	if (_vm->_globals.GESTE_FLAG != 3) {
		_vm->_globals.GESTE = _vm->_globals.freeMemory(_vm->_globals.GESTE);
		_vm->_globals.GESTE_FLAG = 3;
		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "PROFIL.SPR");
		_vm->_globals.GESTE = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	}
	if (idx == 1)
		ACTION(_vm->_globals.GESTE, "20,19,18,17,16,15,14,13,13,13,13,13,14,15,16,17,18,19,20,-1,", 0, 0, 8, 1);
	if (idx == 2)
		SPACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,-1,", 0, 0, 8, 1);
	if (idx == 3)
		SPACTION1(_vm->_globals.GESTE, "9,10,11,12,13,14,15,16,17,18,19,20,-1,", 0, 0, 8);
	if (idx == 4)
		ACTION(_vm->_globals.GESTE, "1,2,3,4,5,6,7,8,8,7,6,5,4,3,2,1,-1,", 0, 0, 8, 1);
	if (idx == 5)
		SPACTION(_vm->_globals.GESTE, "23,24,25,-1,", 0, 0, 8, 1);
	if (idx == 6)
		SPACTION1(_vm->_globals.GESTE, "24,,23,-1,", 0, 0, 8);
	if (idx == 7)
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,-1,", 0, 0, 8, 1);
	if (idx == 8)
		SPACTION1(_vm->_globals.GESTE, "26,25,24,23,-1,", 0, 0, 8);
	if (idx == 9)
		SPACTION(_vm->_globals.GESTE, "23,24,25,26,27,28,29,-1,", 0, 0, 8, 1);
	if (idx == 10)
		SPACTION1(_vm->_globals.GESTE, "28,27,26,25,24,23,-1,", 0, 0, 8);
}

void ObjectsManager::ZONE_ON(int idx) {
	if (_vm->_globals.BOBZONE[idx]) {
		_vm->_globals.BOBZONE_FLAG[idx] = true;
	} else {
		_vm->_globals.ZONEP[idx].field10 = 1;
	}
}

void ObjectsManager::disableZone(int idx) {
	if (_vm->_globals.BOBZONE[idx]) {
		_vm->_globals.BOBZONE_FLAG[idx] = false;
	} else {
		_vm->_globals.ZONEP[idx].field10 = 0;
	}

}

void ObjectsManager::OPTI_ONE(int idx, int fromPosi, int destPosi, int a4) {
	// Set Hopkins animation and position
	if (a4 != 3) {
		setBobAnimation(idx);
		SET_BOBPOSI(idx, fromPosi);
	}

	// Make Hopkins walk to the expected place
	do {
		_vm->_eventsManager.VBL();
	} while (destPosi != BOBPOSI(idx));

	if (!a4)
		stopBobAnimation(idx);
	else if (a4 == 4) {
		_vm->_graphicsManager.fastDisplay(_vm->_globals._bob[idx]._spriteData,
			_vm->_globals._bob[idx]._oldX, _vm->_globals._bob[idx]._oldY, _vm->_globals._bob[idx]._frameIndex);
		stopBobAnimation(idx);
		_vm->_eventsManager.VBL();
	}
}


void ObjectsManager::AFFICHE_SPEED1(byte *speedData, int xp, int yp, int img) {
	SPEED_PTR = speedData;
	SPEED_X = xp;
	SPEED_Y = yp;
	SPEED_IMAGE = img;
}

int ObjectsManager::BOBPOSI(int idx) {
	return _vm->_globals._bob[idx].field10 / 5;
}

/**
 * Set Hopkins animation
 */
void ObjectsManager::setBobAnimation(int idx) {
	if (!_vm->_globals._bob[idx]._disabledAnimationFl)
		return;

	_vm->_globals._bob[idx]._disabledAnimationFl = false;
	_vm->_globals._bob[idx].field10 = 5;
	_vm->_globals._bob[idx]._frameIndex = 250;
	_vm->_globals._bob[idx].field12 = 0;
	_vm->_globals._bob[idx].field14 = 0;
}

/**
 * Stop Hopkins animation
 */
void ObjectsManager::stopBobAnimation(int idx) {
	_vm->_globals._bob[idx]._disabledAnimationFl = true;
}

void ObjectsManager::SET_BOBPOSI(int idx, int a2) {
	_vm->_globals._bob[idx].field10 = 5 * a2;
	_vm->_globals._bob[idx].field12 = 0;
	_vm->_globals._bob[idx].field14 = 0;
}

/**
 * Get X position
 */
int ObjectsManager::getBobPosX(int idx) {
	return _vm->_globals._bob[idx]._xp;
}

int ObjectsManager::BOBY(int idx) {
	return _vm->_globals._bob[idx]._yp;
}

int ObjectsManager::BOBA(int idx) {
	return _vm->_globals._bob[idx]._frameIndex;
}

void ObjectsManager::INILINK(const Common::String &file) {
	int v8;
	int v9;
	int v10;
	int v11;
	int v12;
	byte *v13;
	int v14;
	int v15;
	byte *v16;
	byte *v17;
	byte *v22;
	int16 v27;
	int16 v28;
	int v31;
	int v32;
	int v33;
	int v34;
	int v35;
	int v36;
	byte *v37;
	size_t nbytes;
	byte *ptr;
	Common::String filename, filename2;
	Common::File f;

	filename = file + ".LNK";
	ptr = _vm->_fileManager.searchCat(filename, 3);
	nbytes = _vm->_globals._catalogSize;
	if (ptr == g_PTRNUL) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, filename);

		if (!f.open(_vm->_globals._curFilename))
			error("Error opening file - %s", _vm->_globals._curFilename.c_str());

		nbytes = f.size();
		ptr = _vm->_globals.allocMemory(nbytes);
		if (g_PTRNUL == ptr)
			error("INILINK");
		_vm->_fileManager.readStream(f, ptr, nbytes);
		f.close();
	}
	if (!OBSSEUL) {
		for (int idx = 0; idx < 500; ++idx)
			_vm->_globals.STAILLE[idx] = (int16)READ_LE_UINT16((uint16 *)ptr + idx);

		_vm->_globals.resetCache();

		filename2 = Common::String((const char *)ptr + 1000);
		if (!filename2.empty()) {
			_vm->_globals.CACHE_BANQUE[1] = _vm->_fileManager.searchCat(filename2, 8);

			if (_vm->_globals.CACHE_BANQUE[1] || _vm->_globals.CACHE_BANQUE[1] == g_PTRNUL) {
				_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, filename2);
			} else {
				_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "RES_SLI.RES");
			}

			_vm->_globals.CACHE_BANQUE[1] = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
			v36 = 60;
			v37 = ptr + 1000;
			for (int v40 = 0; v40 <= 21; v40++) {
				v8 = (int16)READ_LE_UINT16(v37 + 2 * v36);
				v9 = (int16)READ_LE_UINT16(v37 + 2 * v36 + 2);
				v10 = (int16)READ_LE_UINT16(v37 + 2 * v36 + 4);
				v11 = v40;

				_vm->_globals.Cache[v11].field14 = (int16)READ_LE_UINT16(v37 + 2 * v36 + 8);
				_vm->_globals.Cache[v11]._spriteIndex = v8;
				_vm->_globals.Cache[v11]._x = v9;
				_vm->_globals.Cache[v11]._y = v10;

				if (!_vm->_globals.CACHE_BANQUE[1]) {
					_vm->_globals.Cache[v40].fieldA = 0;
				} else {
					v12 = v8;
					v13 = _vm->_globals.CACHE_BANQUE[1];
					v14 = getWidth(v13, v8);
					v15 = getHeight(v13, v12);
					_vm->_globals.Cache[v40]._spriteData = v13;
					_vm->_globals.Cache[v40]._width = v14;
					_vm->_globals.Cache[v40]._height = v15;
					_vm->_globals.Cache[v40].fieldA = 1;
				}
				if (!_vm->_globals.Cache[v40]._x && !_vm->_globals.Cache[v40]._y
							&& !_vm->_globals.Cache[v40]._spriteIndex)
					_vm->_globals.Cache[v40].fieldA = 0;

				v36 += 5;
			}
			_vm->_globals.CACHE_ON();
		}
	}

	_vm->_linesManager.RESET_OBSTACLE();
	for (size_t idx = 0; idx < nbytes; idx++) {
		if (ptr[idx] == 'O' && ptr[idx + 1] == 'B' && ptr[idx + 2] == '2') {
			v16 = ptr + idx + 4;
			v32 = 0;
			v34 = 0;
			_vm->_linesManager._linesNumb = 0;
			do {
				v27 = (int16)READ_LE_UINT16(v16 + 2 * v32);
				if (v27 != -1) {
					_vm->_linesManager.AJOUTE_LIGNE(
					    v34,
					    v27,
					    (int16)READ_LE_UINT16(v16 + 2 * v32 + 2),
					    (int16)READ_LE_UINT16(v16 + 2 * v32 + 4),
					    (int16)READ_LE_UINT16(v16 + 2 * v32 + 6),
					    (int16)READ_LE_UINT16(v16 + 2 * v32 + 8),
					    1);
					++_vm->_linesManager._linesNumb;
				}
				v32 += 5;
				++v34;
			} while (v27 != -1);
			_vm->_linesManager.INIPARCOURS();
		}
	}

	if (!OBSSEUL) {
		for (size_t idx = 0; idx < nbytes; idx++) {
			if (ptr[idx] == 'Z' && ptr[idx + 1] == 'O' && ptr[idx + 2] == '2') {
				v17 = &ptr[idx + 4];
				v33 = 0;
				v35 = 0;
				for (int i = 1; i <= 100; i++) {
					_vm->_globals.ZONEP[i]._destX = 0;
					_vm->_globals.ZONEP[i]._destY = 0;
					_vm->_globals.ZONEP[i].field4 = 0;
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

				v31 = 0;
				do {
					v28 = (int16)READ_LE_UINT16(v17 + 2 * v33);
					if (v28 != -1) {
						_vm->_linesManager.addZoneLine(
						    v35,
						    (int16)READ_LE_UINT16(v17 + 2 * v33 + 2),
						    (int16)READ_LE_UINT16(v17 + 2 * v33 + 4),
						    (int16)READ_LE_UINT16(v17 + 2 * v33 + 6),
						    (int16)READ_LE_UINT16(v17 + 2 * v33 + 8),
						    v28);
						_vm->_globals.ZONEP[v28].field10 = 1;
					}
					v33 += 5;
					++v35;
					++v31;
				} while (v28 != -1);
				for (int i = 1; i <= 100; i++) {
					_vm->_globals.ZONEP[i]._destX = (int16)READ_LE_UINT16(v17 + 2 * v33);
					_vm->_globals.ZONEP[i]._destY = (int16)READ_LE_UINT16(v17 + 2 * v33 + 2);
					_vm->_globals.ZONEP[i].field4 = (int16)READ_LE_UINT16(v17 + 2 * v33 + 4);
					v33 += 3;
				}

				v22 = ptr + idx + (10 * v31 + 606) + 4;
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
				CARRE_ZONE();
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
			SPRITE_ON(0);
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
			_vm->_globals._saveData->data[svField270] = 1;
			_vm->_globals._saveData->data[svField300] = 1;
			_vm->_globals._saveData->data[svField320] = 1;
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
			_vm->_globals.NOPARLE = true;
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
			if (_vm->_globals._saveData->data[i] != 2)
				_vm->_globals._saveData->data[i] = 0;
		}
		break;

	case 73:
		if (!_vm->_globals._saveData->data[svField318]) {
			_vm->_globals.CACHE_SUB(0);
			_vm->_globals.CACHE_SUB(1);
		}
		break;

	case 93:
		if (!_vm->_globals._saveData->data[svField333])
			setBobAnimation(8);
		break;
	}
}

void ObjectsManager::OPTI_BOBON(int idx1, int idx2, int idx3, int a4, int a5, int a6, int a7) {
	if (idx1 != -1)
		setBobAnimation(idx1);
	if (idx2 != -1)
		setBobAnimation(idx2);
	if (idx3 != -1)
		setBobAnimation(idx3);
	if (!a7) {
		if (idx1 != -1)
			SET_BOBPOSI(idx1, a4);
		if (idx2 != -1)
			SET_BOBPOSI(idx2, a5);
		if (idx3 != -1)
			SET_BOBPOSI(idx3, a6);
	}
}

void ObjectsManager::SCI_OPTI_ONE(int idx, int a2, int a3, int a4) {
	_vm->_eventsManager._curMouseButton = 0;
	_vm->_eventsManager._mouseButton = 0;

	if (a4 != 3) {
		setBobAnimation(idx);
		SET_BOBPOSI(idx, a2);
	}

	do {
		_vm->_eventsManager.VBL();
		if (_vm->_eventsManager._curMouseButton)
			break;
	} while (a3 != BOBPOSI(idx));
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

int ObjectsManager::CALC_PROPRE(int idx) {
	int retVal = 25;
	int size = _vm->_globals.STAILLE[idx];
	if (_vm->_globals.PERSO_TYPE == 1) {
		if (size < 0)
			size = -size;
		size = 20 * (5 * size - 100) / -80;
	} else if (_vm->_globals.PERSO_TYPE == 2) {
		if (size < 0)
			size = -size;
		size = 20 * (5 * size - 165) / -67;
	}

	if (size < 0)
		retVal = _vm->_graphicsManager.zoomOut(25, -size);
	else if (size > 0)
		retVal = _vm->_graphicsManager.zoomIn(25, size);

	return retVal;
}

int ObjectsManager::colision(int xp, int yp) {
	if (_vm->_globals.SegmentEnCours <= 0)
		return -1;

	int xMax = xp + 4;
	int xMin = xp - 4;

	for (int idx = 0; idx <= _vm->_globals.SegmentEnCours; ++idx) {
		int field2 = _vm->_globals.Segment[idx].field2;
		if (_vm->_globals.Segment[idx].field4 < field2)
			continue;

		int yMax = yp + 4;
		int yMin = yp - 4;

		do {
			int16 *dataP = _vm->_linesManager._zoneLine[field2].zoneData;
			if (dataP != (int16 *)g_PTRNUL) {
				int count = _vm->_linesManager._zoneLine[field2].count;
				int v1 = dataP[0];
				int v2 = dataP[1];
				int v3 = dataP[count * 2 - 2];
				int v4 = dataP[count * 2 - 1];

				bool flag = true;
				if (v1 < v3 && (xMax < v1 || xMin > v3))
					flag = false;
				if (v1 >= v3 && (xMin > v1 || xMax < v3))
					flag = false;
				if (v2 < v4 && (yMax < v2 || yMin > v4))
					flag = false;
				if (v2 >= v4 && (yMin > v2 || yMax < v4))
					flag = false;

				if (flag && _vm->_linesManager._zoneLine[field2].count > 0) {
					for (int v5 = 0; v5 < count; ++v5) {
						int xCheck = *dataP++;
						int yCheck = *dataP++;

						if ((xp == xCheck || (xp + 1) == xCheck) && (yp == yCheck))
							return _vm->_linesManager._zoneLine[field2].field2;
					}
				}
			}
		} while (++field2 <= _vm->_globals.Segment[idx].field4);
	}

	return -1;
}

void ObjectsManager::ACTION(const byte *spriteData, const Common::String &a2, int a3, int a4, int speed, int a6) {
	bool tokenCompleteFl;
	char curChar;
	int spriteIndex;

	int strPos = 0;
	int idx = 0;
	Common::String tmpStr = "";
	int realSpeed = speed;
	if (_vm->_globals._speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals._speed == 3)
		realSpeed = speed / 3;
	const byte *oldSpriteData = _sprite[0]._spriteData;
	spriteIndex = _sprite[0]._spriteIndex;
	int oldFieldE = _sprite[0].fieldE;
	_sprite[0].field12 += a3;
	_sprite[0].field14 += a4;
	_sprite[0].fieldE = a6;

	for (;;) {
		tokenCompleteFl = false;
		curChar = a2[strPos];
		if (curChar == ',') {
			idx = atoi(tmpStr.c_str());
			tmpStr = "";
			tokenCompleteFl = true;
		} else {
			tmpStr += curChar;
		}
		++strPos;

		if (tokenCompleteFl) {
			if (idx == -1) {
				_sprite[0]._spriteData = oldSpriteData;
				_sprite[0]._spriteIndex = spriteIndex;
				_sprite[0].field12 -= a3;
				_sprite[0].field14 -= a4;
				_sprite[0].fieldE = oldFieldE;
			} else {
				_sprite[0]._spriteData = spriteData;
				_sprite[0]._spriteIndex = idx;
			}
			for (int v10 = 0; v10 < realSpeed; v10++)
				_vm->_eventsManager.VBL();
			if (idx == -1)
				break;
		}
	}
}

void ObjectsManager::SPACTION(byte *a1, const Common::String &animationSeq, int a3, int a4, int speed, int a6) {
	int spriteIndex = 0;
	Common::String tmpStr = "";

	int realSpeed = speed;
	if (_vm->_globals._speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals._speed == 3)
		realSpeed = speed / 3;

	S_old_spr = _sprite[0]._spriteData;
	S_old_ani = _sprite[0]._spriteIndex;
	S_old_ret = _sprite[0].fieldE;
	_sprite[0].field12 += a3;
	_sprite[0].field14 += a4;
	_sprite[0].fieldE = a6;

	uint strPos = 0;
	char nextChar;
	do {
		bool completeTokenFl = false;
		do {
			nextChar = animationSeq[strPos];
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
			_sprite[0]._spriteData = a1;
			_sprite[0]._spriteIndex = spriteIndex;
		}
		for (int i = 0; i < realSpeed; i++)
			_vm->_eventsManager.VBL();
	} while (spriteIndex != -1);
}

void ObjectsManager::SPACTION1(byte *spriteData, const Common::String &animString, int a3, int a4, int speed) {
	Common::String tmpStr = "";
	int realSpeed;
	if (_vm->_globals._speed == 2)
		realSpeed = speed / 2;
	else if (_vm->_globals._speed == 3)
		realSpeed = speed / 3;
	else
		realSpeed = speed;

	int idx = 0;
	int spriteIndex = 0;
	bool completeTokenFl;
	char nextChar;

	for (;;) {
		completeTokenFl = false;
		nextChar = animString[idx];
		if (nextChar == ',') {
			spriteIndex = atoi(tmpStr.c_str());
			tmpStr = "";
			completeTokenFl = true;
		} else {
			tmpStr += nextChar;
		}
		++idx;

		if (completeTokenFl) {
			if (spriteIndex == -1) {
				_sprite[0]._spriteData = S_old_spr;
				_sprite[0]._spriteIndex = S_old_ani;
				_sprite[0].field12 -= a3;
				_sprite[0].field14 -= a4;
				_sprite[0].fieldE = S_old_ret;
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

	if (_vm->_globals._saveData->data[savegameIdx] == 2)
		return;

	if (_vm->_globals._saveData->data[savegameIdx]) {
		if (_vm->_globals._saveData->data[savegameIdx] == 1) {
			if (((idx == 1 || idx == 2) && BOBPOSI(idx) == 26) || ((idx == 3 || idx == 4) && BOBPOSI(idx) == 27)) {
				_vm->_dialogsManager._removeInventFl = true;
				_vm->_soundManager.PLAY_SAMPLE2(1);
				_vm->_globals._saveData->data[savegameIdx] = 4;
			}
		}
		if (_vm->_globals._saveData->data[savegameIdx] == 4) {
			if (idx >= 1 && idx <= 4 && BOBPOSI(idx) > 30)
				_vm->_globals._saveData->data[savegameIdx] = 3;
		}
		if (_vm->_globals._saveData->data[savegameIdx] == 3) {
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
		_vm->_globals._saveData->data[savegameIdx] = 1;
	}
}

void ObjectsManager::lockAnimX(int idx, int a2) {
	_vm->_globals._lockedAnims[idx]._enableFl = true;
	_vm->_globals._lockedAnims[idx]._posX = a2;
}

void ObjectsManager::PERSONAGE(const Common::String &backgroundFile, const Common::String &linkFile,
							   const Common::String &animFile, const Common::String &s4, int v) {
	int v5;
	int v7;
	int v8;
	int v9;

	v5 = 0;
	_vm->_dialogsManager._inventFl = false;
	_vm->_eventsManager._gameKey = KEY_NONE;
	_vm->_dialogsManager._removeInventFl = false;
	_vm->_graphicsManager._scrollOffset = 0;
	_vm->_globals.PLAN_FLAG = false;
	_vm->_globals.iRegul = 1;
	_vm->_soundManager.WSOUND(v);
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
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
		if (!_vm->_globals.NOSPRECRAN)
			_vm->_graphicsManager.INI_ECRAN(s4);
		if (!s4.empty() && _vm->_globals.NOSPRECRAN)
			_vm->_graphicsManager.INI_ECRAN2(s4);
	}
	_vm->_eventsManager.mouseOn();
	if (_vm->_globals._screenId == 61) {
		SPRITE(_vm->_globals.PERSO, Common::Point(330, 418), 0, 60, 0, 0, 34, 190);
		SPRITE_ON(0);
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
		VERIFTAILLE();
	}
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_vm->_eventsManager.changeMouseCursor(4);
	for (int v6 = 0; v6 <= 4; v6++)
		_vm->_eventsManager.VBL();
	_vm->_graphicsManager.fadeInLong();
	if (_vm->_globals._screenId == 61) {
		_vm->_animationManager.playSequence("OUVRE.SEQ", 10, 4, 10);
		stopBobAnimation(3);
		_vm->_globals.NOT_VERIF = true;
		g_old_x = getSpriteX(0);
		_vm->_globals.g_old_sens = -1;
		_vm->_globals.Compteur = 0;
		_vm->_globals.chemin = (int16 *)g_PTRNUL;
		v7 = getSpriteY(0);
		v8 = getSpriteX(0);
		_vm->_globals.chemin = _vm->_linesManager.PARCOURS2(v8, v7, 330, 345);
		_vm->_globals.NOT_VERIF = true;
		do {
			GOHOME();
			_vm->_eventsManager.VBL();
		} while (_vm->_globals.chemin != (int16 *)g_PTRNUL);
		setSpriteIndex(0, 64);
	}
	do {
		v9 = _vm->_eventsManager.getMouseButton();
		if (v9 == 1) {
			handleLeftButton();
			v9 = 1;
		}
		if (v9 == 2)
			handleRightButton();
		_vm->_dialogsManager.testDialogOpening();
		checkZone();
		if (_vm->_globals.GOACTION)
			PARADISE();
		if (!_vm->_globals._exitId) {
			_vm->_eventsManager.VBL();
			if (!_vm->_globals._exitId)
				continue;
		}
		v5 = 1;
	} while (!_vm->shouldQuit() && v5 != 1);
	if (_vm->shouldQuit())
		return;

	_vm->_graphicsManager.fadeOutLong();
	if (!animFile.empty())
		_vm->_graphicsManager.FIN_VISU();
	if (_vm->_globals._screenId == 61)
		removeSprite(0);
	CLEAR_ECRAN();
	_vm->_globals.iRegul = 0;
}

void ObjectsManager::PERSONAGE2(const Common::String &backgroundFile, const Common::String &linkFile,
								const Common::String &animFile, const Common::String &s4, int v) {
	int mouseButtons;
	bool breakFlag;
	int xp, yp;

	_vm->_dialogsManager._inventFl = false;
	_vm->_eventsManager._gameKey = KEY_NONE;
	_verb = 4;
	_vm->_globals.MAX_COMPTE = 6;
	_vm->_graphicsManager._scrollOffset = 0;
	_vm->_dialogsManager._removeInventFl = false;
	_vm->_globals.PLAN_FLAG = false;
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
		if (!_vm->_globals.NOSPRECRAN)
			_vm->_graphicsManager.INI_ECRAN(s4);
		else
			_vm->_graphicsManager.INI_ECRAN2(s4);
	}
	_vm->_eventsManager.mouseOn();
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	if (_vm->_globals.PERSO_TYPE) {
		if (!_vm->_globals._saveData->data[svField122] && !_vm->_globals._saveData->data[svField356]) {
			_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "PERSO.SPR");
			_vm->_globals.PERSO = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
			_vm->_globals.PERSO_TYPE = 0;
		}
	}
	if (!_vm->_globals.PERSO_TYPE) {
		if (_vm->_globals._saveData->data[svField122] == 1) {
			_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "HOPFEM.SPR");
			_vm->_globals.PERSO = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
			_vm->_globals.PERSO_TYPE = 1;
		}
	}
	if (_vm->_globals.PERSO_TYPE != 2 && _vm->_globals._saveData->data[svField356] == 1) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "PSAMAN.SPR");
		_vm->_globals.PERSO = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
		_vm->_globals.PERSO_TYPE = 2;
	}
	_vm->_globals.loadCharacterData();
	switch (_vm->_globals.PERSO_TYPE) {
	case 0:
		SPRITE(_vm->_globals.PERSO, _characterPos, 0, PERI, 0, 0, 34, 190);
		break;
	case 1:
		SPRITE(_vm->_globals.PERSO, _characterPos, 0, PERI, 0, 0, 28, 155);
		break;
	case 2:
		SPRITE(_vm->_globals.PERSO, _characterPos, 0, PERI, 0, 0, 20, 127);
		break;
	}
	_vm->_eventsManager.setMouseXY(_characterPos);
	if (_vm->_graphicsManager._largeScreenFl)
		_vm->_graphicsManager.SCROLL = (int16)getSpriteX(0) - 320;
	VERIFTAILLE();
	SPRITE_ON(0);
	_vm->_globals.CACHE_ON();
	_vm->_globals.chemin = (int16 *)g_PTRNUL;
	VERIFTAILLE();
	SPECIAL_INI();
	_vm->_eventsManager._mouseSpriteId = 4;
	g_old_x = _characterPos.x;
	g_old_y = _characterPos.y;
	_vm->_globals.g_old_sens = -1;
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

	breakFlag = false;
	while (!_vm->shouldQuit() && !breakFlag) {
		mouseButtons = _vm->_eventsManager.getMouseButton();
		if (mouseButtons) {
			if (mouseButtons == 1) {
				if (_verb == 16 && _vm->_eventsManager._mouseCursorId == 16) {
					xp = _vm->_eventsManager.getMouseX();
					yp = _vm->_eventsManager.getMouseY();

					if ((xCheck == xp) && (yCheck == yp)) {
						_vm->_globals.chemin = (int16 *)g_PTRNUL;
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
			if (_vm->_globals.chemin == (int16 *)g_PTRNUL
					|| (GOHOME(), _vm->_globals.chemin == (int16 *)g_PTRNUL)) {
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

	if (_vm->_globals._exitId != 8 || _vm->_globals._screenId != 5 || !_vm->_globals._helicopterFl) {
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
		CLEAR_ECRAN();
	} else {
		_vm->_globals._helicopterFl = false;
	}
	_vm->_globals.iRegul = 0;
}

} // End of namespace Hopkins
