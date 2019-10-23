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

#include "hopkins/globals.h"

#include "hopkins/files.h"
#include "hopkins/font.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

#include "common/textconsole.h"
#include "common/file.h"

namespace Hopkins {

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

Globals::Globals(HopkinsEngine *vm) {
	_vm = vm;

	// Initialize array properties
	for (int i = 0; i < 500; ++i)
		_spriteSize[i] = 0;
	for (int i = 0; i < 70; ++i)
		Common::fill((byte *)&_hopkinsItem[i], (byte *)&_hopkinsItem[i] + sizeof(HopkinsItem), 0);

	for (int i = 0; i < 36; ++i)
		_inventory[i] = 0;

	// Initialize fields
	_language = LANG_EN;

	_linuxEndDemoFl = false;
	_speed = 1;
	_eventMode = EVENTMODE_DEFAULT;
	_exitId = 0;
	_characterSpriteBuf = 0;
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
	_checkDistanceFl = false;
	_characterType = CHARACTER_HOPKINS;
	_actionMoveTo = false;
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
	_levelSpriteBuf = NULL;
	_saveData = NULL;
	_answerBuffer = NULL;
	_characterSpriteBuf = NULL;
	_optionDialogSpr = NULL;

	// Reset flags
	_censorshipFl = false;
	_disableInventFl = false;
	_freezeCharacterFl = false;
	_optionDialogFl = false;
	_introSpeechOffFl = false;
	_cityMapEnabledFl = false;

	_baseMapColor = 50;
	_curRoomNum = 0;
}

Globals::~Globals() {
	freeMemory(_levelSpriteBuf);
	freeMemory((byte *)_saveData);
	freeMemory(_answerBuffer);
	freeMemory(_characterSpriteBuf);
	free(NULL);
}

void Globals::setConfig() {
	// CHECKME: Should be in Globals() but it doesn't work
	// The Polish version is a translation of the English version. The filenames are the same.
	// The Russian version looks like a translation of the English version, based on the filenames.
	switch (_vm->getLanguage()) {
	case Common::EN_ANY:
	case Common::PL_POL:
	case Common::RU_RUS:
		_language = LANG_EN;
		break;
	case Common::FR_FRA:
		_language = LANG_FR;
		break;
	case Common::ES_ESP:
		_language = LANG_SP;
		break;
	default:
		error("Hopkins - SetConfig(): Unknown language in internal language mapping");
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
	default:
		break;
	}
}

void Globals::clearAll() {
	_vm->_fontMan->clearAll();
	_vm->_dialog->clearAll();
	_answerBuffer = NULL;
	_levelSpriteBuf = NULL;
	_saveData = NULL;
	_vm->_objectsMan->_curObjectIndex = 0;

	_vm->_linesMan->clearAll();
	_vm->_objectsMan->clearAll();

	_saveData = (Savegame *)malloc(sizeof(Savegame));
	_saveData->reset();

	_vm->_events->clearAll();
}

void Globals::loadCharacterData() {
	const int *srcList[] = { HOPKINS_PERSO_0, HOPKINS_PERSO_1, HOPKINS_PERSO_2 };
	const int *srcP = srcList[_characterType];

	for (int idx = 0; idx < 240 / 4; ++idx) {
		_hopkinsItem[idx]._speedX = *srcP++;
		_hopkinsItem[idx]._speedY = *srcP++;
	}

	_vm->_objectsMan->resetOldFrameIndex();
	_vm->_objectsMan->resetOldDirection();
}

byte *Globals::allocMemory(int count) {
	byte *result = (byte *)malloc(count);
	if (!result)
		result = NULL;
	return result;
}

byte *Globals::freeMemory(byte *p) {
	if (p)
		free(p);
	return NULL;
}

} // End of namespace Hopkins
