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

#include "common/textconsole.h"
#include "common/file.h"
#include "hopkins/globals.h"
#include "hopkins/files.h"
#include "hopkins/font.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

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

	// Initialise array properties
	for (int i = 0; i < 6; ++i)
		CACHE_BANQUE[i] = g_PTRNUL;
	for (int i = 0; i < 106; ++i)
		Common::fill((byte *)&ZONEP[i], (byte *)&ZONEP[i] + sizeof(ZonePItem), 0);
	for (int i = 0; i < 35; ++i)
		Common::fill((byte *)&Bqe_Anim[i], (byte *)&Bqe_Anim[i] + sizeof(BqeAnimItem), 0);
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
		Common::fill((byte *)&ObjetW[i], (byte *)&ObjetW[i] + sizeof(ObjetWItem), 0);
	for (int i = 0; i < 25; ++i)
		Common::fill((byte *)&Cache[i], (byte *)&Cache[i] + sizeof(CacheItem), 0);

	for (int i = 0; i < 105; ++i) {
		BOBZONE[i] = 0;
		BOBZONE_FLAG[i] = false;
	}
	for (int i = 0; i < 500; ++i)
		_spriteSize[i] = 0;
	for (int i = 0; i < 70; ++i)
		Common::fill((byte *)&Hopkins[i], (byte *)&Hopkins[i] + sizeof(HopkinsItem), 0);

	for (int i = 0; i < 36; ++i)
		_inventory[i] = 0;
	for (int i = 0; i < 51; ++i)
		Common::fill((byte *)&_sortedDisplay[i], (byte *)&_sortedDisplay[i] + sizeof(SortItem), 0);

	// Initialize fields
	_language = LANG_EN;

	_linuxEndDemoFl = false;
	_speed = 1;
	_oldFrameIndex = 0;
	_oldDirection = 0;
	_oldDirectionSpriteIdx = 59;
	_lastDirection = 0;
	TETE = NULL;
	NUM_FICHIER_OBJ = 0;
	nbrligne = 0;
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
	NOT_VERIF = false;
	PERSO_TYPE = 0;
	GOACTION = false;
	Compteur = 0;
	_actionDirection = 0;
	_actionDirection = 0;

	Credit_bx = -1;
	Credit_bx1 = -1;
	Credit_by = -1;
	Credit_by1 = -1;
	_creditsPosY = 0;
	_creditsLineNumb = 0;
	memset(_creditsItem, 0, 12000);
	_creditsStep = 0;

	_oceanDirection = 0;

	// Initialize pointers
	ICONE = NULL;
	BUF_ZONE = NULL;
	for (int idx = 0; idx < 6; ++idx)
		CACHE_BANQUE[idx] = NULL;
	SPRITE_ECRAN = NULL;
	_saveData = NULL;
	GESTE = NULL;
	_inventoryObject = NULL;
	_answerBuffer = g_PTRNUL;
	ADR_FICHIER_OBJ = NULL;
	PERSO = NULL;
	_optionDialogSpr = NULL;

	// Reset flags
	_censorshipFl = false;
	GESTE_FLAG = 0;
	_disableInventFl = false;
	NOMARCHE = false;
	_optionDialogFl = false;
	_cacheFl = false;
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
	j_104 = 0;
}

Globals::~Globals() {
	free(ICONE);
	freeMemory(TETE);
	freeMemory(BUF_ZONE);
	for (int idx = 0; idx < 6; ++idx)
		CACHE_BANQUE[idx] = freeMemory(CACHE_BANQUE[idx]);
	freeMemory(SPRITE_ECRAN);
	freeMemory((byte *)_saveData);
	freeMemory(GESTE);
	freeMemory(_inventoryObject);
	freeMemory(_answerBuffer);
	freeMemory(ADR_FICHIER_OBJ);
	freeMemory(PERSO);

	CLEAR_VBOB();

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
		FICH_ZONE = "ZONEAN.TXT";
		FICH_TEXTE = "TEXTEAN.TXT";
		break;
	case LANG_FR:
		FICH_ZONE = "ZONE01.TXT";
		FICH_TEXTE = "TEXTE01.TXT";
		break;
	case LANG_SP:
		FICH_ZONE = "ZONEES.TXT";
		FICH_TEXTE = "TEXTEES.TXT";
		break;
	}
}

void Globals::clearAll() {
	for (int idx = 0; idx < 6; ++idx)
		CACHE_BANQUE[idx] = g_PTRNUL;

	nbrligne = 80;
	INIT_ANIM();

	_boxWidth = 0;

	_vm->_fontManager.clearAll();

	INIT_VBOB();
	ADR_FICHIER_OBJ = g_PTRNUL;
	NUM_FICHIER_OBJ = 0;
	_vm->_eventsManager._objectBuf = g_PTRNUL;
	_vm->_dialogsManager._inventWin1 = g_PTRNUL;
	_vm->_dialogsManager._inventBuf2 = g_PTRNUL;
	_answerBuffer = g_PTRNUL;
	SPRITE_ECRAN = g_PTRNUL;
	_saveData = (Sauvegarde *)g_PTRNUL;
	_vm->_objectsManager._curObjectIndex = 0;

	for (int idx = 0; idx < 105; ++idx) {
		ZONEP[idx]._destX = 0;
		ZONEP[idx]._destY = 0;
		ZONEP[idx]._spriteIndex = 0;
	}

	_vm->_linesManager.clearAll();
	_vm->_objectsManager.clearAll();

	_saveData = (Sauvegarde *)malloc(sizeof(Sauvegarde));
	memset(_saveData, 0, sizeof(Sauvegarde));

	_boxWidth = 240;

	_vm->_eventsManager._objectBuf = allocMemory(2500);
	_inventoryObject = allocMemory(2500);

	ADR_FICHIER_OBJ = g_PTRNUL;

	GESTE = g_PTRNUL;
	GESTE_FLAG = 0;
}

void Globals::loadCharacterData() {
	assert(PERSO_TYPE >= 0 && PERSO_TYPE <= 2);

	const int *srcList[] = { HOPKINS_PERSO_0, HOPKINS_PERSO_1, HOPKINS_PERSO_2 };
	const int *srcP = srcList[PERSO_TYPE];

	for (int idx = 0; idx < 240 / 4; ++idx) {
		Hopkins[idx].field0 = *srcP++;
		Hopkins[idx].field2 = *srcP++;
	}

	_oldFrameIndex = -1;
	_oldDirection = -1;
}

void Globals::INIT_ANIM() {
	for (int idx = 0; idx < 35; ++idx) {
		Bqe_Anim[idx]._data = g_PTRNUL;
		Bqe_Anim[idx]._enabledFl = false;
	}

	for (int idx = 0; idx < 8; ++idx) {
		Bank[idx]._data = g_PTRNUL;
		Bank[idx]._loadedFl = false;
		Bank[idx]._filename = "";
		Bank[idx]._fileHeader = 0;
	}
}

void Globals::INIT_VBOB() {
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

void Globals::CLEAR_VBOB() {
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
		ObjetW[idx].field0 = *srcP++;
		ObjetW[idx]._idx = *srcP++;
		ObjetW[idx].field2 = *srcP++;
		ObjetW[idx].field3 = *srcP++;
		ObjetW[idx].field4 = *srcP++;
		ObjetW[idx].field5 = *srcP++;
		ObjetW[idx].field6 = *srcP++;
		ObjetW[idx].field7 = *srcP++;
	}

	free(data);
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

// Reset Cache
void Globals::resetCache() {

	for (int idx = 1; idx <= 5; ++idx) {
		CACHE_BANQUE[idx] = freeMemory(CACHE_BANQUE[idx]);
	}

	for (int idx = 0; idx <= 20; ++idx) {
		Cache[idx]._spriteData = g_PTRNUL;
		Cache[idx]._x = 0;
		Cache[idx]._y = 0;
		Cache[idx]._spriteIndex = 0;
		Cache[idx]._useCount = 0;
		Cache[idx]._width = 0;
		Cache[idx]._height = 0;
		Cache[idx].field10 = false;
		Cache[idx].field14 = 0;
	}

	_cacheFl = false;
}

void Globals::CACHE_ON() {
	_cacheFl = true;
}

void Globals::CACHE_OFF() {
	_cacheFl = false;
}

void Globals::B_CACHE_OFF(int idx) {
	assert(idx < 36);
	_vm->_objectsManager._bob[idx].field34 = true;
}

void Globals::CACHE_SUB(int idx) {
	Cache[idx]._useCount = 0;
}

void Globals::CACHE_ADD(int idx) {
	Cache[idx]._useCount = 1;
}

// Load Cache
void Globals::loadCache(const Common::String &file) {
	resetCache();
	byte *ptr = _vm->_fileManager.loadFile(file);
	Common::String filename = Common::String((const char *)ptr);

	Common::File f;
	if (!f.exists(filename))
		return;

	byte *spriteData = _vm->_fileManager.loadFile(filename);
	CACHE_BANQUE[1] = spriteData;
	int curBufIdx = 60;
	for (int i = 0; i <= 21; i++) {
		Cache[i]._spriteIndex = (int16)READ_LE_UINT16((uint16 *)ptr + curBufIdx);
		Cache[i]._x = (int16)READ_LE_UINT16((uint16 *)ptr + curBufIdx + 1);
		Cache[i]._y = (int16)READ_LE_UINT16((uint16 *)ptr + curBufIdx + 2);
		Cache[i].field14 = (int16)READ_LE_UINT16((uint16 *)ptr + curBufIdx + 4);
		if (spriteData == g_PTRNUL) {
			Cache[i]._useCount = 0;
		} else {
			Cache[i]._spriteData = spriteData;
			Cache[i]._width = _vm->_objectsManager.getWidth(spriteData, Cache[i]._spriteIndex);
			Cache[i]._height = _vm->_objectsManager.getHeight(spriteData, Cache[i]._spriteIndex);
			Cache[i]._useCount = 1;
		}

		if ( !Cache[i]._x && !Cache[i]._y && !Cache[i]._spriteIndex)
			Cache[i]._useCount = 0;
		curBufIdx += 5;
	}
	CACHE_ON();
	freeMemory(ptr);
}

} // End of namespace Hopkins
