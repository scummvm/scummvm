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
#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "hopkins/talk.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

namespace Hopkins {

TalkManager::TalkManager() {
	_characterBuffer = NULL;
	_characterPalette = NULL;
	_characterSprite = NULL;
	_characterAnim = NULL;
	_characterSize = 0;
	STATI = false;
	_dialogueMesgId1 = _dialogueMesgId2 = _dialogueMesgId3 = _dialogueMesgId4 = 0;
	_paletteBufferIdx = 0;
}

void TalkManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void TalkManager::PARLER_PERSO(const Common::String &filename) {
	Common::String spriteFilename;

	int answer = 0;
	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(9);
	_vm->_eventsManager.VBL();
	_vm->_graphicsManager._scrollStatus = 1;
	bool oldDisableInventFl = _vm->_globals._disableInventFl;
	_vm->_globals._disableInventFl = true;
	_characterBuffer = _vm->_fileManager.searchCat(filename, 5);
	_characterSize = _vm->_globals._catalogSize;
	if (_characterBuffer == g_PTRNUL) {
		_characterBuffer = _vm->_fileManager.loadFile(filename);
		_characterSize = _vm->_fileManager.fileSize(filename);
	}
	_vm->_globals._saveData->_data[svField4] = 0;
	getStringFromBuffer(40, spriteFilename, (const char *)_characterBuffer);
	getStringFromBuffer(0, _questionsFilename, (const char *)_characterBuffer);
	getStringFromBuffer(20, _answersFilename, (const char *)_characterBuffer);
	if (_vm->_globals._language == LANG_FR) {
		_answersFilename = _questionsFilename = "RUE.TXT";
	} else if (_vm->_globals._language == LANG_EN) {
		_answersFilename = _questionsFilename = "RUEAN.TXT";
	} else if (_vm->_globals._language == LANG_SP) {
		_answersFilename = _questionsFilename = "RUEES.TXT";
	}
	_dialogueMesgId1 = (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 40);
	_paletteBufferIdx = 20 * (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 42) + 110;
	_characterSprite = _vm->_fileManager.searchCat(spriteFilename, 7);
	if (_characterSprite) {
		_characterSprite = _vm->_objectsManager.loadSprite(spriteFilename);
	} else {
		_characterSprite = _vm->_objectsManager.loadSprite("RES_SAN.RES");
	}

	if (_vm->_graphicsManager._lineNbr == SCREEN_WIDTH)
		_vm->_saveLoadManager.saveFile("TEMP.SCR", _vm->_graphicsManager._vesaScreen, 307200);
	else if (_vm->_graphicsManager._lineNbr == (SCREEN_WIDTH * 2))
		_vm->_saveLoadManager.saveFile("TEMP.SCR", _vm->_graphicsManager._vesaScreen, 614400);

	if (!_vm->_graphicsManager._lineNbr)
		_vm->_graphicsManager._scrollOffset = 0;
	_vm->_graphicsManager.NB_SCREEN(true);
	_vm->_objectsManager.PERSO_ON = true;
	searchCharacterPalette(_paletteBufferIdx, false);
	startCharacterAnim0(_paletteBufferIdx, false);
	initCharacterAnim();
	_dialogueMesgId2 = _dialogueMesgId1 + 1;
	_dialogueMesgId3 = _dialogueMesgId1 + 2;
	_dialogueMesgId4 = _dialogueMesgId1 + 3;
	int v14 = _vm->_eventsManager._mouseCursorId;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(0);
	if (!_vm->_globals.NOPARLE) {
		int v5;
		do {
			v5 = DIALOGUE();
			if (v5 != _dialogueMesgId4)
				answer = DIALOGUE_REP(v5);
			if (answer == -1)
				v5 = _dialogueMesgId4;
			_vm->_eventsManager.VBL();
		} while (v5 != _dialogueMesgId4);
	}
	if (_vm->_globals.NOPARLE) {
		int v6 = 1;
		int v7;
		do
			v7 = DIALOGUE_REP(v6++);
		while (v7 != -1);
	}
	clearCharacterAnim();
	_vm->_globals.NOPARLE = false;
	_characterBuffer = _vm->_globals.freeMemory(_characterBuffer);
	_characterSprite = _vm->_globals.freeMemory(_characterSprite);
	_vm->_graphicsManager.NB_SCREEN(false);

	_vm->_saveLoadManager.load("TEMP.SCR", _vm->_graphicsManager._vesaScreen);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	_vm->_objectsManager.PERSO_ON = false;
	_vm->_eventsManager._mouseCursorId = v14;

	_vm->_eventsManager.changeMouseCursor(v14);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);

	if (_vm->getIsDemo() == false)
		_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);

	_vm->_graphicsManager.initColorTable(145, 150, _vm->_graphicsManager._palette);
	_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	_vm->_graphicsManager.lockScreen();
	_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	_vm->_graphicsManager.unlockScreen();
	memcpy(_vm->_graphicsManager._vesaBuffer, _vm->_graphicsManager._vesaScreen, 614399);
	_vm->_globals._disableInventFl = oldDisableInventFl;
	_vm->_graphicsManager.DD_VBL();
	for (int i = 0; i <= 4; i++)
		_vm->_eventsManager.VBL();
	_vm->_graphicsManager._scrollStatus = 0;
}

void TalkManager::PARLER_PERSO2(const Common::String &filename) {
	// TODO: The original disables the mouse cursor here
	STATI = true;
	bool v7 = _vm->_globals._disableInventFl;
	_vm->_globals._disableInventFl = true;
	_characterBuffer = _vm->_fileManager.searchCat(filename, 5);
	_characterSize = _vm->_globals._catalogSize;
	if (_characterBuffer == g_PTRNUL) {
		_characterBuffer = _vm->_fileManager.loadFile(filename);
		_characterSize = _vm->_fileManager.fileSize(filename);
	}

	_vm->_globals._saveData->_data[svField4] = 0;
	getStringFromBuffer(0, _questionsFilename, (const char *)_characterBuffer);
	getStringFromBuffer(20, _answersFilename, (const char *)_characterBuffer);

	switch (_vm->_globals._language) {
	case LANG_EN:
		_questionsFilename = "RUEAN.TXT";
		_answersFilename = "RUEAN.TXT";
		break;
	case LANG_FR:
		_questionsFilename = "RUE.TXT";
		_answersFilename = "RUE.TXT";
		break;
	case LANG_SP:
		_questionsFilename = "RUEES.TXT";
		_answersFilename = "RUEES.TXT";
		break;
	}

	_dialogueMesgId1 = (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 40);
	_paletteBufferIdx = 20 * (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 42) + 110;
	searchCharacterPalette(_paletteBufferIdx, false);
	_dialogueMesgId2 = _dialogueMesgId1 + 1;
	_dialogueMesgId3 = _dialogueMesgId1 + 2;
	_dialogueMesgId4 = _dialogueMesgId1 + 3;
	int oldMouseCursorId = _vm->_eventsManager._mouseCursorId;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(0);

	if (!_vm->_globals.NOPARLE) {
		int v3;
		do {
			v3 = DIALOGUE();
			if (v3 != _dialogueMesgId4) {
				if (DIALOGUE_REP(v3) == -1)
					v3 = _dialogueMesgId4;
			}
		} while (v3 != _dialogueMesgId4);
	}

	if (_vm->_globals.NOPARLE) {
		int v4 = 1;
		int v5;
	    do
			v5 = DIALOGUE_REP(v4++);
		while (v5 != -1);
	}

	_characterBuffer = _vm->_globals.freeMemory(_characterBuffer);
	_vm->_eventsManager._mouseCursorId = oldMouseCursorId;

	_vm->_eventsManager.changeMouseCursor(oldMouseCursorId);
	_vm->_graphicsManager.initColorTable(145, 150, _vm->_graphicsManager._palette);
	_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	// TODO: The original reenables the mouse cursor here
	_vm->_globals._disableInventFl = v7;
	STATI = false;
}

void TalkManager::getStringFromBuffer(int srcStart, Common::String &dest, const char *srcData) {
	dest = Common::String(srcData + srcStart);
}

int TalkManager::DIALOGUE() {
	if (STATI) {
		uint16 *bufPtr = (uint16 *)_characterBuffer + 48;
		int curVal = (int16)READ_LE_UINT16(bufPtr);
		if (curVal != 0)
			_vm->_objectsManager.setBobAnimation(curVal);
		if (curVal != 1)
			_vm->_objectsManager.setBobAnimation((int16)READ_LE_UINT16(bufPtr + 1));
		if (curVal != 2)
			_vm->_objectsManager.setBobAnimation((int16)READ_LE_UINT16(bufPtr + 2));
		if (curVal != 3)
			_vm->_objectsManager.setBobAnimation((int16)READ_LE_UINT16(bufPtr + 3));
		if (curVal != 4)
			_vm->_objectsManager.setBobAnimation((int16)READ_LE_UINT16(bufPtr + 4));
	} else {
		dialogWait();
	}

	int sentence1LineNumb = VERIF_BOITE(_dialogueMesgId1, _questionsFilename, 65);
	int sentence2LineNumb = VERIF_BOITE(_dialogueMesgId2, _questionsFilename, 65);
	int sentence3LineNumb = VERIF_BOITE(_dialogueMesgId3, _questionsFilename, 65);
	int sentence4LineNumb = VERIF_BOITE(_dialogueMesgId4, _questionsFilename, 65);

	int sentence4PosY = 420 - 20 * sentence4LineNumb;
	int sentence3PosY = sentence4PosY - 20 * sentence3LineNumb;
	int sentence2PosY = sentence3PosY - 20 * sentence2LineNumb;
	int sentence1PosY = sentence2PosY - 20 * sentence1LineNumb;

	_vm->_fontManager.initTextBuffers(5, _dialogueMesgId1, _questionsFilename, 5, sentence1PosY, 0, 65, 255);
	_vm->_fontManager.initTextBuffers(6, _dialogueMesgId2, _questionsFilename, 5, sentence2PosY, 0, 65, 255);
	_vm->_fontManager.initTextBuffers(7, _dialogueMesgId3, _questionsFilename, 5, sentence3PosY, 0, 65, 255);
	_vm->_fontManager.initTextBuffers(8, _dialogueMesgId4, _questionsFilename, 5, sentence4PosY, 0, 65, 255);
	_vm->_fontManager.showText(5);
	_vm->_fontManager.showText(6);
	_vm->_fontManager.showText(7);
	_vm->_fontManager.showText(8);

	int retVal = -1;
	bool v6 = false;
  	do {
		int mousePosY = _vm->_eventsManager.getMouseY();
		if (sentence1PosY < mousePosY && mousePosY < (sentence2PosY - 1)) {
			_vm->_fontManager.setOptimalColor(6, 7, 8, 5);
			retVal = _dialogueMesgId1;
		}
		if (sentence2PosY < mousePosY && mousePosY < (sentence3PosY - 1)) {
			_vm->_fontManager.setOptimalColor(5, 7, 8, 6);
			retVal = _dialogueMesgId2;
		}
		if (sentence3PosY < mousePosY && mousePosY < (sentence4PosY - 1)) {
			_vm->_fontManager.setOptimalColor(5, 6, 8, 7);
			retVal = _dialogueMesgId3;
		}
		if (sentence4PosY < mousePosY && mousePosY < 419) {
			_vm->_fontManager.setOptimalColor(5, 6, 7, 8);
			retVal = _dialogueMesgId4;
		}

		_vm->_eventsManager.VBL();
		if (_vm->_eventsManager.getMouseButton())
			v6 = true;
		if (retVal == -1)
			v6 = false;
	} while (!_vm->shouldQuit() && !v6);

	_vm->_soundManager.mixVoice(retVal, 1);
	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(6);
	_vm->_fontManager.hideText(7);
	_vm->_fontManager.hideText(8);

	if (STATI) {
		uint16 *bufPtr = (uint16 *)_characterBuffer + 48;

		int curVal = (int16)READ_LE_UINT16(bufPtr);
		if (curVal != 0)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 1);
		if (curVal != 1)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 2);
		if (curVal != 2)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 3);
		if (curVal != 3)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 4);
		if (curVal != 4)
			_vm->_objectsManager.stopBobAnimation(curVal);
	} else {
		dialogTalk();
	}

	_vm->_eventsManager.VBL();
  return retVal;
}

int TalkManager::DIALOGUE_REP(int idx) {
	int v1;
	byte *v3;
	int v6;
	int v7;
	int v21;
	int v22;
	int v23;
	int v24;
	int v25;

	v1 = 0;
	v3 = _characterBuffer + 110;
	for (; (int16)READ_LE_UINT16(v3) != idx; v3 = _characterBuffer + 20 * v1 + 110) {
		++v1;
		if ((int16)READ_LE_UINT16((uint16 *)_characterBuffer + 42) < v1)
			return -1;
	}

	v22 = (int16)READ_LE_UINT16((uint16 *)v3 + 1);
	v25 = (int16)READ_LE_UINT16((uint16 *)v3 + 2);
	v24 = (int16)READ_LE_UINT16((uint16 *)v3 + 3);
	v23 = (int16)READ_LE_UINT16((uint16 *)v3 + 4);
	_dialogueMesgId1 = (int16)READ_LE_UINT16((uint16 *)v3 + 5);
	_dialogueMesgId2 = (int16)READ_LE_UINT16((uint16 *)v3 + 6);
	_dialogueMesgId3 = (int16)READ_LE_UINT16((uint16 *)v3 + 7);
	v6 = (int16)READ_LE_UINT16((uint16 *)v3 + 8);
	v7 = (int16)READ_LE_UINT16((uint16 *)v3 + 9);

	if (v7)
		_vm->_globals._saveData->_data[svField4] = v7;

	if (!v6)
		v6 = 10;
	if (STATI) {
		uint16 *bufPtr = (uint16 *)_characterBuffer + 43;
		int curVal = (int16)READ_LE_UINT16(bufPtr);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 1);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 2);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 3);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 4);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);
	} else {
		VISU_PARLE();
	}

	if (!_vm->_soundManager._textOffFl) {
		_vm->_fontManager.initTextBuffers(9, v22, _answersFilename, v25, v24, 5, v23, 252);
		_vm->_fontManager.showText(9);
	}
	if (!_vm->_soundManager.mixVoice(v22, 1)) {
		_vm->_eventsManager._curMouseButton = 0;
		_vm->_eventsManager._mouseButton = 0;

		if (_vm->getIsDemo()) {
			for (int i = 0; i < v6; i++) {
				_vm->_eventsManager.VBL();
			}
		} else {
			for (int i = 0; i < v6; i++) {
				_vm->_eventsManager.VBL();
				if (_vm->_eventsManager._mouseButton || _vm->_eventsManager._curMouseButton)
					break;
				if (_vm->_eventsManager.getMouseButton() && i + 1 > abs(v6 / 5))
					break;
			}
		}
	}

	if (!_vm->_soundManager._textOffFl)
		_vm->_fontManager.hideText(9);
	if (STATI) {
		uint16 *bufPtr = (uint16 *)_characterBuffer + 43;
		int curVal = (int16)READ_LE_UINT16(bufPtr);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 1);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 2);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 3);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = (int16)READ_LE_UINT16(bufPtr + 4);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);
	} else {
		dialogEndTalk();
	}
	v21 = 0;
	if (!_dialogueMesgId1)
		v21 = -1;

	return v21;
}

void TalkManager::searchCharacterPalette(int startIdx, bool dark) {
	int palettePos = 0;
	size_t curIdx = startIdx;
	for (;;) {
		if (_characterBuffer[curIdx] == 'P' && _characterBuffer[curIdx + 1] == 'A' && _characterBuffer[curIdx + 2] == 'L') {
			palettePos = curIdx;
			break;
		}
		++curIdx;
		if (_characterSize == curIdx)
			return;
	}

	_characterPalette = _characterBuffer + palettePos + 5;
	_characterPalette[0] = 0;
	_characterPalette[1] = 0;
	_characterPalette[2] = 0;
	_characterPalette[759] = 255;
	_characterPalette[760] = 255;
	_characterPalette[762] = 0;
	_characterPalette[763] = 0;
	_characterPalette[764] = 0;
	_characterPalette[765] = 224;
	_characterPalette[766] = 224;
	_characterPalette[767] = 255;

	if (!dark)
		_characterPalette[761] = 86;
	else
		_characterPalette[761] = 255;

	_vm->_graphicsManager.setPaletteVGA256(_characterPalette);
	_vm->_graphicsManager.initColorTable(145, 150, _characterPalette);
}

void TalkManager::dialogWait() {
	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			BOB_VISU_PARLE(idx);
	}
}

void TalkManager::dialogTalk() {
	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_OFF(idx);
	}

	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_ZERO(idx);
	}
}

void TalkManager::dialogEndTalk() {
	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_OFF(idx);
	}

	_vm->_eventsManager.VBL();
	_vm->_eventsManager.VBL();

	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			_vm->_objectsManager.BOB_ZERO(idx);
	}
}

int TalkManager::VERIF_BOITE(int idx, const Common::String &file, int a3) {
	int v9;
	int v10;
	char v11;
	char v13;
	int v15;
	byte *ptr;
	int v17;
	byte *v19;
	uint32 indexData[4047];
	Common::String filename;
	Common::String dest;
	Common::File f;
	int filesize;

	_vm->_globals.police_l = 11;

	// Build up the filename
	filename = dest = file;
	while (filename.lastChar() != '.')
		filename.deleteLastChar();
	filename += "IND";

	if (!f.open(filename))
		error("Could not open file - %s", filename.c_str());
	filesize = f.size();
	assert(filesize < 16188);

	for (int i = 0; i < (filesize / 4); ++i)
		indexData[i] = f.readUint32LE();
	f.close();

	if (!f.open(dest))
		error("Error opening file - %s", dest.c_str());

	f.seek(indexData[idx]);
	ptr = _vm->_globals.allocMemory(2058);
	if (ptr == g_PTRNUL)
		error("temporary TEXT");
	f.read(ptr, 2048);
	f.close();

	v19 = ptr;
	for (int i = 0; i < 2048; i++) {
		v13 = *v19;
		if ((byte)(*v19 + 46) > 27) {
			if ((byte)(v13 + 80) > 27) {
				if ((byte)(v13 - 65) <= 25 || (byte)(v13 - 97) <= 25)
					v13 = 32;
			} else {
				v13 -= 79;
			}
		} else {
			v13 = *v19 + 111;
		}
		*v19 = v13;
		v19++;
	}

	for (int i = 0; i < 2048; i++) {
		if ( ptr[i] == 10 || ptr[i] == 13 )
			ptr[i] = 0;
	}

	v9 = 0;
	v15 = (11 * a3) - 4;
	int lineCount = 0;
	do {
		v10 = 0;
		for (;;) {
			v17 = v10;
			do {
				v11 = ptr[v9 + v10];
				++v10;
			} while (v11 != ' ' && v11 != '%');

			if (v10 >= v15 / _vm->_globals.police_l) {
				if (v11 == '%')
					v11 = ' ';
				break;
			}

			if (v11 == '%') {
				v17 = v10;
				break;
			}
		}
		++lineCount;
		v9 += v17;
	} while (v11 != '%');
	free(ptr);
	return lineCount;
}

void TalkManager::VISU_PARLE() {
	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx]._enabledFl)
			BOB_VISU_PARLE(idx);
	}
}

void TalkManager::BOB_VISU_PARLE(int idx) {
	int v4;
	byte *v5;

	_vm->_objectsManager._priorityFl = true;
	if (!_vm->_globals._bob[idx].field0) {
		_vm->_objectsManager.BOB_ZERO(idx);
		v5 = _vm->_globals.Bqe_Anim[idx]._data;
		v4 = (int16)READ_LE_UINT16(v5 + 2);
		if (!v4)
			v4 = 1;
		if ((int16)READ_LE_UINT16(v5 + 24)) {
			_vm->_globals._bob[idx]._isSpriteFl = true;
			_vm->_globals._bob[idx].field36 = 0;
			_vm->_globals._bob[idx]._flipFl = false;
			_vm->_globals._bob[idx]._animData = _vm->_globals.Bqe_Anim[idx]._data;
			_vm->_globals._bob[idx].field0 = 10;
			v5 = _characterSprite;
			_vm->_globals._bob[idx]._spriteData = _characterSprite;
			_vm->_globals._bob[idx].field1E = v4;
			_vm->_globals._bob[idx].field20 = -1;
			_vm->_globals._bob[idx].field22 = 0;
			_vm->_globals._bob[idx]._offsetY = 0;
		}
	}
}

void TalkManager::startCharacterAnim0(int startIdx, bool readOnlyFl) {
	int animIdx = 0;
	size_t curIdx = startIdx;
	for (;;) {
		if (_characterBuffer[curIdx] == 'A' && _characterBuffer[curIdx + 1] == 'N' && _characterBuffer[curIdx + 2] == 'I' && _characterBuffer[curIdx + 3] == 'M' && _characterBuffer[curIdx + 4] == 1) {
			animIdx = curIdx;
			break;
		}
		++curIdx;
		if (_characterSize == curIdx)
			return;
	}
	_characterAnim = _characterBuffer + animIdx + 25;
	if (!readOnlyFl) {
		int idx = 0;
		int v7;
		do {
			v7 = (int16)READ_LE_UINT16(&_characterAnim[2 * idx + 4]);
			if (v7 && _vm->_globals._speed != 501)
				_vm->_graphicsManager.fastDisplay(_characterSprite, _vm->_eventsManager._startPos.x + (int16)READ_LE_UINT16(&_characterAnim[2 * idx]),
				    (int16)READ_LE_UINT16(&_characterAnim[2 * idx + 2]), _characterAnim[2 * idx + 8]);
			idx += 5;
		} while (_vm->_globals._speed != 501 && v7);
	}
}

/**
 * Initialize character animation
 */
void TalkManager::initCharacterAnim() {
	uint16 *bufPtr = (uint16 *)_characterBuffer + 43;
	byte *animPtr = _characterBuffer + 110;
	int curVal = (int16)READ_LE_UINT16(bufPtr);
	if (curVal)
		searchCharacterAnim(21, animPtr, curVal, _characterSize);

	curVal = (int16)READ_LE_UINT16(bufPtr + 1);
	if (curVal)
		searchCharacterAnim(22, animPtr, curVal, _characterSize);

	curVal = (int16)READ_LE_UINT16(bufPtr + 2);
	if (curVal)
		searchCharacterAnim(23, animPtr, curVal, _characterSize);

	curVal = (int16)READ_LE_UINT16(bufPtr + 3);
	if (curVal)
		searchCharacterAnim(24, animPtr, curVal, _characterSize);

	curVal = (int16)READ_LE_UINT16(bufPtr + 4);
	if (curVal)
		searchCharacterAnim(25, animPtr, curVal, _characterSize);

	curVal = (int16)READ_LE_UINT16(bufPtr + 5);
	if (curVal)
		searchCharacterAnim(26, animPtr, curVal, _characterSize);

	curVal = (int16)READ_LE_UINT16(bufPtr + 6);
	if (curVal)
		searchCharacterAnim(27, animPtr, curVal, _characterSize);

	curVal = (int16)READ_LE_UINT16(bufPtr + 7);
	if (curVal)
		searchCharacterAnim(28, animPtr, curVal, _characterSize);

	curVal = (int16)READ_LE_UINT16(bufPtr + 8);
	if (curVal)
		searchCharacterAnim(29, animPtr, curVal, _characterSize);

	curVal = (int16)READ_LE_UINT16(bufPtr + 9);
	if (curVal)
		searchCharacterAnim(30, animPtr, curVal, _characterSize);
}

void TalkManager::clearCharacterAnim() {
	for (int idx = 21; idx <= 34; ++idx) {
		_vm->_globals.Bqe_Anim[idx]._data = _vm->_globals.freeMemory(_vm->_globals.Bqe_Anim[idx]._data);
		_vm->_globals.Bqe_Anim[idx]._enabledFl = false;
	}
}

bool TalkManager::searchCharacterAnim(int idx, const byte *bufPerso, int a3, int a4) {
	bool result;
	const byte *v5;
	int v6;
	byte *v8;
	byte *v9;
	int v10;
	int v11;
	int v12;
	int v14;
	int v15;
	int v16;
	int v18;
	int v22;
	int v23;
	const byte *v24;

	v22 = 0;
	result = false;

	do {
		if (bufPerso[v22] == 'A' && bufPerso[v22 + 1] == 'N' && bufPerso[v22 + 2] == 'I' && bufPerso[v22 + 3] == 'M' && bufPerso[v22 + 4] == a3) {
			v5 = v22 + bufPerso + 5;
			v18 = v22 + 5;
			v6 = 0;
			bool v7 = false;
			do {
				if ((v5[0] == 'A' && v5[1] == 'N' && v5[2] == 'I' && v5[3] == 'M') ||
				    (v5[0] == 'F' && v5[1] == 'I' && v5[2] == 'N'))
					v7 = true;
				if (v18 > a4) {
					_vm->_globals.Bqe_Anim[idx]._enabledFl = false;
					result = false;
					_vm->_globals.Bqe_Anim[idx]._data = g_PTRNUL;
				}
				++v18;
				++v6;
				++v5;
			} while (!v7);
			_vm->_globals.Bqe_Anim[idx]._data = _vm->_globals.allocMemory(v6 + 50);
			_vm->_globals.Bqe_Anim[idx]._enabledFl = true;
			memcpy(_vm->_globals.Bqe_Anim[idx]._data, (const byte *)(v22 + bufPerso + 5), 20);
			v8 = _vm->_globals.Bqe_Anim[idx]._data;

			v9 = v8 + 20;
			v24 = v22 + bufPerso + 25;
			v10 = (int16)READ_LE_UINT16(v22 + bufPerso + 25);
			v11 = (int16)READ_LE_UINT16(v22 + bufPerso + 27);
			v23 = (int16)READ_LE_UINT16(v22 + bufPerso + 29);
			v12 = (int16)READ_LE_UINT16(v22 + bufPerso + 31);
			v8[28] = bufPerso[v22 + 33];
			v8[29] = bufPerso[v22 + 34];
			WRITE_LE_UINT16(v8 + 20, v10);
			WRITE_LE_UINT16(v8 + 22, v11);
			WRITE_LE_UINT16(v8 + 24, v23);
			WRITE_LE_UINT16(v8 + 26, v12);
			for (int i = 1; i < 5000; i++) {
				v9 += 10;
				v24 += 10;
				if (!v23)
					break;
				v14 = (int16)READ_LE_UINT16(v24);
				v15 = (int16)READ_LE_UINT16(v24 + 2);
				v23 = (int16)READ_LE_UINT16(v24 + 4);
				v16 = (int16)READ_LE_UINT16(v24 + 6);
				v9[8] = v24[8];
				v9[9] = v24[9];
				WRITE_LE_UINT16(v9, v14);
				WRITE_LE_UINT16(v9 + 2, v15);
				WRITE_LE_UINT16(v9 + 4, v23);
				WRITE_LE_UINT16(v9 + 6, v16);
			}
			result = true;
		}
		if (bufPerso[v22] == 'F' && bufPerso[v22 + 1] == 'I' && bufPerso[v22 + 2] == 'N')
			result = true;
		++v22;
	} while (v22 <= a4 && !result);

	return result;
}

void TalkManager::REPONSE(int zone, int verb) {
	byte *v5;
	byte *v6;
	uint16 v7;
	byte *v8;
	int v10;
	uint16 v11;
	int v12;
	int v13;
	bool v15;
	bool v16;
	bool loopCond;
	byte *ptr;

	byte zoneObj = zone;
	byte verbObj = verb;
LABEL_2:
	v15 = false;
	if (_vm->_globals.COUCOU != g_PTRNUL) {
		v5 = _vm->_globals.COUCOU;
		for (;;) {
			if (v5[0] == 'C' && v5[1] == 'O' && v5[2] == 'D') {
				if (v5[3] == zoneObj && v5[4] == verbObj)
					v15 = true;
			}
			if (v5[0] == 'F' && v5[1] == 'I' && v5[2] == 'N')
				break;
			if (!v15)
				v5++;
			else if (v15) {
				v6 = v5 + 5;
				ptr = _vm->_globals.allocMemory(620);
				if (g_PTRNUL == ptr)
					error("TRADUC");
				memset(ptr, 0, 620);
				v7 = 0;
				v12 = 0;
				loopCond = false;
				do {
					v16 = false;
					if (v6[v7] == 'F' && v6[v7 + 1] == 'C') {
						++v12;
						assert(v12 < (620 / 20));

						v8 = (ptr + 20 * v12);
						v11 = 0;
						do {
							assert(v11 < 20);
							v8[v11++] = v6[v7++];
							if (v6[v7] == 'F' && v6[v7 + 1] == 'F') {
								v16 = true;
								v8[v11] = 'F';
								v8[v11 + 1] = 'F';
								++v7;
							}
						} while (!v16);
					}
					if (!v16) {
						if ((v6[v7] == 'C' && v6[v7 + 1] == 'O' && v6[v7 + 2] == 'D') || (v6[v7] == 'F' && v6[v7 + 1] == 'I' && v6[v7 + 2] == 'N'))
							loopCond = true;
					}
					v6 += v7 + 1;
					v7 = 0;
				} while (!loopCond);
				loopCond = false;
				v13 = 1;
				do {
					v10 = _vm->_scriptManager.handleOpcode(ptr + 20 * v13);
					if (_vm->shouldQuit())
						return;

					if (v10 == 2)
						// GOTO
						v13 =  _vm->_scriptManager.handleGoto(ptr + 20 * v13);
					else if (v10 == 3)
						// IF
						v13 =  _vm->_scriptManager.handleIf(ptr, v13);

					if (v13 == -1)
						error("Invalid IFF function");

					if (v10 == 1 || v10 == 4)
						// Already handled opcode or END IF
						++v13;
					else if (!v10 || v10 == 5)
						// EXIT
						loopCond = true;
					else if (v10 == 6) {
						// JUMP
						_vm->_globals.freeMemory(ptr);
						zoneObj = _vm->_objectsManager._jumpZone;
						verbObj = _vm->_objectsManager._jumpVerb;
						goto LABEL_2;
					}
				} while (!loopCond);
				_vm->_globals.freeMemory(ptr);
				_vm->_globals._saveData->_data[svField2] = 0;
				return;
			}
		}
	}
}

void TalkManager::REPONSE2(int a1, int a2) {
	int indx = 0;
	if (a2 != 5 || _vm->_globals._saveData->_data[svField3] != 4)
		return;

	if (a1 == 22 || a1 == 23) {
		_vm->_objectsManager.setFlipSprite(0, false);
		_vm->_objectsManager.setSpriteIndex(0, 62);
		_vm->_objectsManager.SPACTION(_vm->_globals._forestSprite, "2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 4, false);
		if (a1 == 22) {
			_vm->_objectsManager.lockAnimX(6, _vm->_objectsManager.getBobPosX(3));
			_vm->_objectsManager.lockAnimX(8, _vm->_objectsManager.getBobPosX(3));
		} else { // a1 == 23
			_vm->_objectsManager.lockAnimX(6, _vm->_objectsManager.getBobPosX(4));
			_vm->_objectsManager.lockAnimX(8, _vm->_objectsManager.getBobPosX(4));
		}
		_vm->_objectsManager.stopBobAnimation(3);
		_vm->_objectsManager.stopBobAnimation(4);
		_vm->_objectsManager.setBobAnimation(6);
		_vm->_soundManager.PLAY_SAMPLE2(1);
		_vm->_objectsManager.SPACTION1(_vm->_globals._forestSprite, "13,14,15,14,13,12,13,14,15,16,-1,", 0, 0, 4);
		do
			_vm->_eventsManager.VBL();
		while (_vm->_objectsManager.BOBPOSI(6) < 12);
		_vm->_objectsManager.stopBobAnimation(6);
		_vm->_objectsManager.setBobAnimation(8);
		
		switch (_vm->_globals._screenId) {
		case 35:
			indx = 201;
			break;
		case 36:
			indx = 203;
			break;
		case 37:
			indx = 205;
			break;
		case 38:
			indx = 207;
			break;
		case 39:
			indx = 209;
			break;
		case 40:
			indx = 211;
			break;
		case 41:
			indx = 213;
			break;
		}
		_vm->_globals._saveData->_data[indx] = 2;
		_vm->_objectsManager.disableZone(22);
		_vm->_objectsManager.disableZone(23);
	} else if (a1 == 20 || a1 == 21) {
		_vm->_objectsManager.setFlipSprite(0, true);
		_vm->_objectsManager.setSpriteIndex(0, 62);
		_vm->_objectsManager.SPACTION(_vm->_globals._forestSprite, "2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 4, true);
		if (a1 == 20) {
			_vm->_objectsManager.lockAnimX(5, _vm->_objectsManager.getBobPosX(1));
			_vm->_objectsManager.lockAnimX(7, _vm->_objectsManager.getBobPosX(1));
		} else { // a1 == 21
			_vm->_objectsManager.lockAnimX(5, _vm->_objectsManager.getBobPosX(2));
			_vm->_objectsManager.lockAnimX(7, _vm->_objectsManager.getBobPosX(2));
		}
		_vm->_objectsManager.stopBobAnimation(1);
		_vm->_objectsManager.stopBobAnimation(2);
		_vm->_objectsManager.setBobAnimation(5);
		_vm->_soundManager.PLAY_SAMPLE2(1);
		_vm->_objectsManager.SPACTION1(_vm->_globals._forestSprite, "13,14,15,14,13,12,13,14,15,16,-1,", 0, 0, 4);
		do
			_vm->_eventsManager.VBL();
		while (_vm->_objectsManager.BOBPOSI(5) < 12);
		_vm->_objectsManager.stopBobAnimation(5);
		_vm->_objectsManager.setBobAnimation(7);
		switch (_vm->_globals._screenId) {
		case 35:
			indx = 200;
			break;
		case 36:
			indx = 202;
			break;
		case 37:
			indx = 204;
			break;
		case 38:
			indx = 206;
			break;
		case 39:
			indx = 208;
			break;
		case 40:
			indx = 210;
			break;
		case 41:
			indx = 212;
			break;
		}
		_vm->_globals._saveData->_data[indx] = 2;
		_vm->_objectsManager.disableZone(21);
		_vm->_objectsManager.disableZone(20);
	}
}

void TalkManager::OBJET_VIVANT(const Common::String &a2) {
	byte *v11;
	Common::String s;
	Common::String v20;
	Common::String v22;
	Common::String v23;

	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(9);
	_vm->_eventsManager.VBL();
	_vm->_graphicsManager._scrollStatus = 1;
	_vm->_linesManager.clearAllZones();
	_vm->_linesManager.resetLines();
	_vm->_globals.resetCache();
	
	for (int i = 0; i <= 44; i++)
		_vm->_globals.BOBZONE[i] = 0;

	_vm->_objectsManager._zoneNum = -1;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(0);
	_characterBuffer = _vm->_fileManager.searchCat(a2, 5);
	_characterSize = _vm->_globals._catalogSize;
	if (_characterBuffer == g_PTRNUL) {
		_characterBuffer = _vm->_fileManager.loadFile(a2);
		_characterSize = _vm->_fileManager.fileSize(a2);
	}
	getStringFromBuffer(40, v23, (const char *)_characterBuffer);
	getStringFromBuffer(0, v22, (const char *)_characterBuffer);
	getStringFromBuffer(20, v20, (const char *)_characterBuffer);

	if (v20 == "NULL")
		v20 = Common::String::format("IM%d", _vm->_globals._screenId);

	_characterSprite = _vm->_fileManager.searchCat(v23, 7);
	if (_characterSprite)
		_characterSprite = _vm->_objectsManager.loadSprite(v23);
	else
		_characterSprite = _vm->_objectsManager.loadSprite("RES_SAN.RES");

	if (_vm->_graphicsManager._lineNbr == SCREEN_WIDTH)
		_vm->_saveLoadManager.saveFile("TEMP.SCR", _vm->_graphicsManager._vesaScreen, 307200);
	else if (_vm->_graphicsManager._lineNbr == (SCREEN_WIDTH * 2))
		_vm->_saveLoadManager.saveFile("TEMP.SCR", _vm->_graphicsManager._vesaScreen, 614400);

	if (!_vm->_graphicsManager._lineNbr)
		_vm->_graphicsManager._scrollOffset = 0;
	_vm->_graphicsManager.NB_SCREEN(true);
	_paletteBufferIdx = 20 * (int16)READ_LE_UINT16((uint16 *)_characterBuffer + 42) + 110;
	_vm->_graphicsManager.NB_SCREEN(true);
	_vm->_objectsManager.PERSO_ON = true;
	searchCharacterPalette(_paletteBufferIdx, true);
	startCharacterAnim0(_paletteBufferIdx, false);
	v11 = _vm->_globals.COUCOU;
	_vm->_globals.COUCOU = g_PTRNUL;
	_vm->_globals.NOMARCHE = true;
	_vm->_objectsManager.INILINK(v22);
	_vm->_objectsManager.PERSO_ON = true;
	_vm->_globals.GOACTION = false;
	_vm->_objectsManager._zoneNum = -1;
	initCharacterAnim();
	VISU_PARLE();
	dialogWait();
	_vm->_graphicsManager.INI_ECRAN2(v22, true);
	_vm->_globals.NOMARCHE = true;
	_vm->_objectsManager._forceZoneFl = true;
	_vm->_objectsManager._zoneNum = -1;
	do {
		int mouseButton = _vm->_eventsManager.getMouseButton();
		if (mouseButton == 1)
			_vm->_objectsManager.handleLeftButton();
		else if (mouseButton == 2)
			_vm->_objectsManager.handleRightButton();

		_vm->_objectsManager.checkZone();
		if (_vm->_globals.GOACTION)
			_vm->_objectsManager.PARADISE();
		_vm->_eventsManager.VBL();
	} while (!_vm->_globals._exitId);
	dialogEndTalk();
	dialogTalk();
	clearCharacterAnim();
	clearCharacterAnim();
	_vm->_globals.NOPARLE = false;
	_characterBuffer = _vm->_globals.freeMemory(_characterBuffer);
	_characterSprite = _vm->_globals.freeMemory(_characterSprite);
	_vm->_graphicsManager.NB_SCREEN(false);
	_vm->_linesManager.clearAllZones();
	_vm->_linesManager.resetLines();
	_vm->_globals.resetCache();
	for (int i = 0; i <= 44; i++)
		_vm->_globals.BOBZONE[i] = 0;

	_vm->_globals.freeMemory(_vm->_globals.COUCOU);
	_vm->_globals.COUCOU = v11;
	_vm->_objectsManager._disableFl = true;
	_vm->_objectsManager.INILINK(v20);
	_vm->_graphicsManager.INI_ECRAN2(v20, true);
	_vm->_objectsManager._disableFl = false;
	_vm->_globals.NOMARCHE = false;
	if (_vm->_globals._exitId == 101)
		_vm->_globals._exitId = 0;

	_vm->_saveLoadManager.load("TEMP.SCR", _vm->_graphicsManager._vesaScreen);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	_vm->_objectsManager.PERSO_ON = false;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(4);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);

	if (!_vm->getIsDemo())
		_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);

	_vm->_graphicsManager.initColorTable(145, 150, _vm->_graphicsManager._palette);
	_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	_vm->_graphicsManager.lockScreen();
	_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager._vesaScreen, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	_vm->_graphicsManager.unlockScreen();
	_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	memcpy(_vm->_graphicsManager._vesaBuffer, _vm->_graphicsManager._vesaScreen, 614399);
	_vm->_globals._disableInventFl = false;
	_vm->_graphicsManager.DD_VBL();
	for (int i = 0; i <= 4; i++)
		_vm->_eventsManager.VBL();
	_vm->_graphicsManager._scrollStatus = 0;
}

} // End of namespace Hopkins
