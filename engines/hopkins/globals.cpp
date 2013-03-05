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

#include "hopkins/globals.h"

#include "hopkins/files.h"
#include "hopkins/font.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

#include "common/textconsole.h"
#include "common/file.h"

namespace Hopkins {

// Global null pointer. This is needed by the engine to recognise NULL pointers, since
// there are places that differentiate between it and a 0 'error' value
byte *g_PTRNUL;

// Default data for the Hopkins array

const int HOPKINS_PERSO_0[] = {
		0, -2, 0, -3, 0, -6, 0, -1, 0, -3, 0, -3, 0, -5, 0, -3, 0, -6, 0, -3, 0, -3, 0, -3,
		9, -4, 8, -4, 6, -2, 9, -2, 9, -3, 9, -3, 9, -4, 9, -2, 9, -2, 8, -2, 9, -3, 9, -2,
		13, 0, 13, 0, 13, 0, 13, 0, 14, 0, 13, 0, 13, 0, 12, 0, 12, 0, 14, 0, 13, 0, 14, 0,
		10, 3, 9, 3, 10, 4, 8, 2, 7, 1, 10, 2, 9, 2, 7, 4, 7, 3, 8, 0, 9, 1, 9, 1, 0, 4, 0,
		4, 0, 6, 0, 3, 0, 4, 0, 3, 0, 4, 0, 4, 0, 6, 0, 3, 0, 3, 0, 3
};

const int HOPKINS_PERSO_1[] = {
		0, -2, 0, -2, 0, -5, 0, -1, 0, -2, 0, -2, 0, -4, 0, -2, 0, -5, 0, -2, 0, -2, 0, -2,
		11, 0, 10, 0, 11, 0, 11, 0, 11, 0, 11, 0, 12, 0, 11, 0, 9, 0, 10, 0, 11, 0, 11, 0,
		11, 0, 10, 0, 11, 0, 11, 0, 11, 0, 11, 0, 12, 0, 11, 0, 9, 0, 10, 0, 11, 0, 11, 0,
		11, 0, 10, 0, 11, 0, 11, 0, 11, 0, 11, 0, 12, 0, 11, 0, 9, 0, 10, 0, 11, 0, 11, 0,
		0, 3, 0, 3, 0, 5, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 5, 0, 3, 0, 3, 0, 3
};

const int HOPKINS_PERSO_2[] = {
		0, -2, 0, 0, 0, -3, 0, -2, 0, -2, 0, -1, 0, -2, 0, -1, 0, -3, 0, -2, 0, -2, 0, -2,
		8, 0, 9, 0, 5, 0, 9, 0, 7, 0, 7, 0, 7, 0, 7, 0, 6, 0, 7, 0, 6, 0, 9, 0, 8, 0, 9, 0,
		5, 0, 9, 0, 7, 0, 7, 0, 7, 0, 7, 0, 6, 0, 7, 0, 6, 0, 9, 0, 8, 0, 9, 0, 5, 0, 9, 0,
		7, 0, 7, 0, 7, 0, 7, 0, 6, 0, 7, 0, 6, 0, 9, 0, 0, 2, 0, 0, 0, 2, 0, 1, 0, 2, 0, 2,
		0, 2, 0, 2, 0, 2, 0, 1, 0, 2, 0, 2
};

Globals::Globals() {
	// Set up the special g_PTRNUL variable
	g_PTRNUL = (byte *)malloc(16);
	strcpy((char *)g_PTRNUL, "POINTERNULL");

	// Initialize array properties
	for (int i = 0; i < 35; ++i)
		Common::fill((byte *)&_animBqe[i], (byte *)&_animBqe[i] + sizeof(BqeAnimItem), 0);
	for (int i = 0; i < 8; ++i)
		Common::fill((byte *)&Bank[i], (byte *)&Bank[i] + sizeof(BankItem), 0);
	for (int i = 0; i < 6; ++i)
		Common::fill((byte *)&Liste[i], (byte *)&Liste[i] + sizeof(ListeItem), 0);
	for (int i = 0; i < 35; ++i)
		Common::fill((byte *)&Liste2[i], (byte *)&Liste2[i] + sizeof(ListeItem), 0);
	for (int i = 0; i < 30; ++i) {
		Common::fill((byte *)&_lockedAnims[i], (byte *)&_lockedAnims[i] + sizeof(LockAnimItem), 0);
		Common::fill((byte *)&VBob[i], (byte *)&VBob[i] + sizeof(VBobItem), 0);
	}
	for (int i = 0; i < 300; ++i)
		Common::fill((byte *)&_objectAuthIcons[i], (byte *)&_objectAuthIcons[i] + sizeof(ObjectAuthIcon), 0);
	for (int i = 0; i < 25; ++i)
		Common::fill((byte *)&_hidingItem[i], (byte *)&_hidingItem[i] + sizeof(HidingItem), 0);

	for (int i = 0; i < 500; ++i)
		_spriteSize[i] = 0;
	for (int i = 0; i < 70; ++i)
		Common::fill((byte *)&_hopkinsItem[i], (byte *)&_hopkinsItem[i] + sizeof(HopkinsItem), 0);

	for (int i = 0; i < 36; ++i)
		_inventory[i] = 0;
	for (int i = 0; i < 51; ++i)
		Common::fill((byte *)&_sortedDisplay[i], (byte *)&_sortedDisplay[i] + sizeof(SortItem), 0);

	// Initialize fields
	_language = LANG_EN;

	_linuxEndDemoFl = false;
	_speed = 1;
	_oldFrameIndex = 0;
	_oldDirection = DIR_NONE;
	_oldDirectionSpriteIdx = 59;
	_lastDirection = DIR_NONE;
	_curObjectFileNum = 0;
	_boxWidth = 0;
	_objectWidth = _objectHeight = 0;
	_catalogPos = 0;
	_catalogSize = 0;
	iRegul = 0;
	_exitId = 0;
	PERSO = 0;
	_screenId = 0;
	_prevScreenId = 0;
	_characterMaxPosY = 0;
	_menuScrollSpeed = 0;
	_menuSpeed = 0;
	_menuSoundOff = 0;
	_menuVoiceOff = 0;
	_menuMusicOff = 0;
	_menuTextOff = 0;
	_menuDisplayType = 0;
	_sortedDisplayCount = 0;
	_checkDistanceFl = false;
	_characterType = 0;
	_actionMoveTo = false;
	Compteur = 0;
	_actionDirection = DIR_NONE;

	_creditsStartX = -1;
	_creditsEndX = -1;
	_creditsStartY = -1;
	_creditsEndY = -1;
	_creditsPosY = 0;
	_creditsLineNumb = 0;
	memset(_creditsItem, 0, 12000);
	_creditsStep = 0;

	_oceanDirection = DIR_NONE;

	// Initialize pointers
	for (int i = 0; i < 6; ++i)
		_hidingItemData[i] = g_PTRNUL;
	SPRITE_ECRAN = NULL;
	_saveData = NULL;
	_answerBuffer = g_PTRNUL;
	_objectDataBuf = NULL;
	PERSO = NULL;
	_optionDialogSpr = NULL;

	// Reset flags
	_censorshipFl = false;
	_disableInventFl = false;
	_freezeCharacterFl = false;
	_optionDialogFl = false;
	_hidingActiveFl = false;
	_introSpeechOffFl = false;
	_baseMapColor = 50;

	// Reset indexed variables
	_hotspotTextColor = 0;
	_oldRouteFromX = 0;
	_oldRouteFromY = 0;
	_oldRouteDestX = 0;
	_oldRouteDestY = 0;
	_oldMouseZoneId = 0;
	_oldZoneNum = 0;
	_oldMouseX = 0;
	_oldMouseY = 0;
	compteur_71 = 0;
	_forceHideText = false;
}

Globals::~Globals() {
	for (int idx = 0; idx < 6; ++idx)
		_hidingItemData[idx] = freeMemory(_hidingItemData[idx]);
	freeMemory(SPRITE_ECRAN);
	freeMemory((byte *)_saveData);
	freeMemory(_answerBuffer);
	freeMemory(_objectDataBuf);
	freeMemory(PERSO);

	clearVBob();

	free(g_PTRNUL);
}

void Globals::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void Globals::setConfig() {
	// CHECKME: Should be in Globals() but it doesn't work
	// The Polish version is a translation of the English version. The filenames are the same.
	switch (_vm->getLanguage()) {
	case Common::EN_ANY:
	case Common::PL_POL:
		_language = LANG_EN;
		break;
	case Common::FR_FRA:
		_language = LANG_FR;
		break;
	case Common::ES_ESP:
		_language = LANG_SP;
		break;
	default:
		warning("Unknown language in internal language mapping");
		break;
	}
	// End of CHECKME

	switch (_language) {
	case LANG_EN:
		_zoneFilename = "ZONEAN.TXT";
		_textFilename = "TEXTEAN.TXT";
		break;
	case LANG_FR:
		_zoneFilename = "ZONE01.TXT";
		_textFilename = "TEXTE01.TXT";
		break;
	case LANG_SP:
		_zoneFilename = "ZONEES.TXT";
		_textFilename = "TEXTEES.TXT";
		break;
	}
}

void Globals::clearAll() {
	for (int idx = 0; idx < 6; ++idx)
		_hidingItemData[idx] = g_PTRNUL;

	initAnimBqe();

	_boxWidth = 0;

	_vm->_fontManager.clearAll();

	initVBob();
	_objectDataBuf = g_PTRNUL;
	_curObjectFileNum = 0;
	_vm->_eventsManager._objectBuf = g_PTRNUL;
	_vm->_dialogsManager._inventWin1 = g_PTRNUL;
	_vm->_dialogsManager._inventBuf2 = g_PTRNUL;
	_answerBuffer = g_PTRNUL;
	SPRITE_ECRAN = g_PTRNUL;
	_saveData = (Savegame *)g_PTRNUL;
	_vm->_objectsManager._curObjectIndex = 0;

	_vm->_linesManager.clearAll();
	_vm->_objectsManager.clearAll();

	_saveData = (Savegame *)malloc(sizeof(Savegame));
	memset(_saveData, 0, sizeof(Savegame));

	_boxWidth = 240;

	_vm->_eventsManager._objectBuf = allocMemory(2500);

	_objectDataBuf = g_PTRNUL;
}

void Globals::loadCharacterData() {
	assert(_characterType >= 0 && _characterType <= 2);

	const int *srcList[] = { HOPKINS_PERSO_0, HOPKINS_PERSO_1, HOPKINS_PERSO_2 };
	const int *srcP = srcList[_characterType];

	for (int idx = 0; idx < 240 / 4; ++idx) {
		_hopkinsItem[idx]._speedX = *srcP++;
		_hopkinsItem[idx]._speedY = *srcP++;
	}

	_oldFrameIndex = -1;
	_oldDirection = DIR_NONE;
}

void Globals::initAnimBqe() {
	for (int idx = 0; idx < 35; ++idx) {
		_animBqe[idx]._data = g_PTRNUL;
		_animBqe[idx]._enabledFl = false;
	}

	for (int idx = 0; idx < 8; ++idx) {
		Bank[idx]._data = g_PTRNUL;
		Bank[idx]._loadedFl = false;
		Bank[idx]._filename = "";
		Bank[idx]._fileHeader = 0;
	}
}

void Globals::initVBob() {
	for (int idx = 0; idx < 30; ++idx) {
		VBob[idx].field4 = 0;
		VBob[idx]._xp = 0;
		VBob[idx]._yp = 0;
		VBob[idx]._frameIndex = 0;
		VBob[idx]._surface = g_PTRNUL;
		VBob[idx]._spriteData = g_PTRNUL;
		VBob[idx]._oldSpriteData = g_PTRNUL;
	}
}

void Globals::clearVBob() {
	for (int idx = 0; idx < 30; ++idx) {
		VBob[idx].field4 = 0;
		VBob[idx]._xp = 0;
		VBob[idx]._yp = 0;
		VBob[idx]._frameIndex = 0;
		VBob[idx]._surface = freeMemory(VBob[idx]._surface);
		VBob[idx]._spriteData = g_PTRNUL;
		VBob[idx]._oldSpriteData = g_PTRNUL;
	}
}

// Load Object
void Globals::loadObjects() {
	byte *data = _vm->_fileManager.loadFile("OBJET.DAT");
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

	freeMemory(data);
}

byte *Globals::allocMemory(int count) {
	byte *result = (byte *)malloc(count);
	if (!result)
		result = g_PTRNUL;
	return result;
}

byte *Globals::freeMemory(byte *p) {
	if (p != g_PTRNUL)
		free(p);
	return g_PTRNUL;
}

// Reset Hiding Items
void Globals::resetHidingItems() {

	for (int idx = 1; idx <= 5; ++idx) {
		_hidingItemData[idx] = freeMemory(_hidingItemData[idx]);
	}

	for (int idx = 0; idx <= 20; ++idx) {
		HidingItem *hid = &_hidingItem[idx];
		hid->_spriteData = g_PTRNUL;
		hid->_x = 0;
		hid->_y = 0;
		hid->_spriteIndex = 0;
		hid->_useCount = 0;
		hid->_width = 0;
		hid->_height = 0;
		hid->field10 = false;
		hid->field14 = 0;
	}

	_hidingActiveFl = false;
}

void Globals::enableHiding() {
	_hidingActiveFl = true;
}

void Globals::disableHiding() {
	_hidingActiveFl = false;
}

void Globals::B_CACHE_OFF(int idx) {
	assert(idx < 36);
	_vm->_objectsManager._bob[idx].field34 = true;
}

void Globals::resetHidingUseCount(int idx) {
	_hidingItem[idx]._useCount = 0;
}

void Globals::setHidingUseCount(int idx) {
	_hidingItem[idx]._useCount = 1;
}

// Load Hiding Items
void Globals::loadHidingItems(const Common::String &file) {
	resetHidingItems();
	byte *ptr = _vm->_fileManager.loadFile(file);
	Common::String filename = Common::String((const char *)ptr);

	Common::File f;
	if (!f.exists(filename))
		return;

	byte *spriteData = _vm->_fileManager.loadFile(filename);
	_hidingItemData[1] = spriteData;
	int curBufIdx = 60;
	for (int i = 0; i <= 21; i++) {
		_hidingItem[i]._spriteIndex = READ_LE_INT16((uint16 *)ptr + curBufIdx);
		_hidingItem[i]._x = READ_LE_INT16((uint16 *)ptr + curBufIdx + 1);
		_hidingItem[i]._y = READ_LE_INT16((uint16 *)ptr + curBufIdx + 2);
		_hidingItem[i].field14 = READ_LE_INT16((uint16 *)ptr + curBufIdx + 4);
		if (spriteData == g_PTRNUL) {
			_hidingItem[i]._useCount = 0;
		} else {
			_hidingItem[i]._spriteData = spriteData;
			_hidingItem[i]._width = _vm->_objectsManager.getWidth(spriteData, _hidingItem[i]._spriteIndex);
			_hidingItem[i]._height = _vm->_objectsManager.getHeight(spriteData, _hidingItem[i]._spriteIndex);
			_hidingItem[i]._useCount = 1;
		}

		if ( !_hidingItem[i]._x && !_hidingItem[i]._y && !_hidingItem[i]._spriteIndex)
			_hidingItem[i]._useCount = 0;
		curBufIdx += 5;
	}
	enableHiding();
	freeMemory(ptr);
}

} // End of namespace Hopkins
