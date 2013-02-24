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

#include "hopkins/talk.h"

#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

#include "common/system.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"

namespace Hopkins {

TalkManager::TalkManager() {
	_characterBuffer = NULL;
	_characterPalette = NULL;
	_characterSprite = NULL;
	_characterAnim = NULL;
	_characterSize = 0;
	_dialogueMesgId1 = _dialogueMesgId2 = _dialogueMesgId3 = _dialogueMesgId4 = 0;
	_paletteBufferIdx = 0;
}

void TalkManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void TalkManager::startAnimatedCharacterDialogue(const Common::String &filename) {
	Common::String spriteFilename;

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
	// CHECKME:_data[svField4] is useless?
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
	_dialogueMesgId1 = READ_LE_INT16((uint16 *)_characterBuffer + 40);
	_paletteBufferIdx = 20 * READ_LE_INT16((uint16 *)_characterBuffer + 42) + 110;
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
	int oldMouseCursorId = _vm->_eventsManager._mouseCursorId;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(0);
	if (!_vm->_globals._introSpeechOffFl) {
		int answer = 0;
		int dlgAnswer;
		do {
			dlgAnswer = dialogQuestion(false);
			if (dlgAnswer != _dialogueMesgId4)
				answer = dialogAnswer(dlgAnswer, false);
			if (answer == -1)
				dlgAnswer = _dialogueMesgId4;
			_vm->_eventsManager.VBL();
		} while (dlgAnswer != _dialogueMesgId4);
	}
	if (_vm->_globals._introSpeechOffFl) {
		int idx = 1;
		int answer;
		do
			answer = dialogAnswer(idx++, false);
		while (answer != -1);
	}
	clearCharacterAnim();
	_vm->_globals._introSpeechOffFl = false;
	_characterBuffer = _vm->_globals.freeMemory(_characterBuffer);
	_characterSprite = _vm->_globals.freeMemory(_characterSprite);
	_vm->_graphicsManager.NB_SCREEN(false);

	_vm->_saveLoadManager.load("TEMP.SCR", _vm->_graphicsManager._vesaScreen);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	_vm->_objectsManager.PERSO_ON = false;
	_vm->_eventsManager._mouseCursorId = oldMouseCursorId;

	_vm->_eventsManager.changeMouseCursor(oldMouseCursorId);
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

void TalkManager::startStaticCharacterDialogue(const Common::String &filename) {
	// TODO: The original disables the mouse cursor here
	bool oldDisableInventFl = _vm->_globals._disableInventFl;
	_vm->_globals._disableInventFl = true;
	_characterBuffer = _vm->_fileManager.searchCat(filename, 5);
	_characterSize = _vm->_globals._catalogSize;
	if (_characterBuffer == g_PTRNUL) {
		_characterBuffer = _vm->_fileManager.loadFile(filename);
		_characterSize = _vm->_fileManager.fileSize(filename);
	}

	// CHECKME:_data[svField4] is useless?
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

	_dialogueMesgId1 = READ_LE_INT16((uint16 *)_characterBuffer + 40);
	_paletteBufferIdx = 20 * READ_LE_INT16((uint16 *)_characterBuffer + 42) + 110;
	searchCharacterPalette(_paletteBufferIdx, false);
	_dialogueMesgId2 = _dialogueMesgId1 + 1;
	_dialogueMesgId3 = _dialogueMesgId1 + 2;
	_dialogueMesgId4 = _dialogueMesgId1 + 3;
	int oldMouseCursorId = _vm->_eventsManager._mouseCursorId;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(0);

	if (!_vm->_globals._introSpeechOffFl) {
		int answer;
		do {
			answer = dialogQuestion(true);
			if (answer != _dialogueMesgId4) {
				if (dialogAnswer(answer, true) == -1)
					answer = _dialogueMesgId4;
			}
		} while (answer != _dialogueMesgId4);
	}

	if (_vm->_globals._introSpeechOffFl) {
		int idx = 1;
		int answer;
		do
			answer = dialogAnswer(idx++, true);
		while (answer != -1);
	}

	_characterBuffer = _vm->_globals.freeMemory(_characterBuffer);
	_vm->_eventsManager._mouseCursorId = oldMouseCursorId;

	_vm->_eventsManager.changeMouseCursor(oldMouseCursorId);
	_vm->_graphicsManager.initColorTable(145, 150, _vm->_graphicsManager._palette);
	_vm->_graphicsManager.setPaletteVGA256(_vm->_graphicsManager._palette);
	// TODO: The original re-enables the mouse cursor here
	_vm->_globals._disableInventFl = oldDisableInventFl;
}

void TalkManager::getStringFromBuffer(int srcStart, Common::String &dest, const char *srcData) {
	dest = Common::String(srcData + srcStart);
}

int TalkManager::dialogQuestion(bool animatedFl) {
	if (animatedFl) {
		uint16 *bufPtr = (uint16 *)_characterBuffer + 48;
		int curVal = READ_LE_INT16(bufPtr);
		if (curVal != 0)
			_vm->_objectsManager.setBobAnimation(curVal);
		if (curVal != 1)
			_vm->_objectsManager.setBobAnimation(READ_LE_INT16(bufPtr + 1));
		if (curVal != 2)
			_vm->_objectsManager.setBobAnimation(READ_LE_INT16(bufPtr + 2));
		if (curVal != 3)
			_vm->_objectsManager.setBobAnimation(READ_LE_INT16(bufPtr + 3));
		if (curVal != 4)
			_vm->_objectsManager.setBobAnimation(READ_LE_INT16(bufPtr + 4));
	} else {
		dialogWait();
	}

	int sentence1LineNumb = countBoxLines(_dialogueMesgId1, _questionsFilename);
	int sentence2LineNumb = countBoxLines(_dialogueMesgId2, _questionsFilename);
	int sentence3LineNumb = countBoxLines(_dialogueMesgId3, _questionsFilename);
	int sentence4LineNumb = countBoxLines(_dialogueMesgId4, _questionsFilename);

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
	bool loopCond = false;
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
			loopCond = true;
		if (retVal == -1)
			loopCond = false;
	} while (!_vm->shouldQuit() && !loopCond);

	_vm->_soundManager.mixVoice(retVal, 1);
	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(6);
	_vm->_fontManager.hideText(7);
	_vm->_fontManager.hideText(8);

	if (animatedFl) {
		uint16 *bufPtr = (uint16 *)_characterBuffer + 48;

		int curVal = READ_LE_INT16(bufPtr);
		if (curVal != 0)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 1);
		if (curVal != 1)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 2);
		if (curVal != 2)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 3);
		if (curVal != 3)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 4);
		if (curVal != 4)
			_vm->_objectsManager.stopBobAnimation(curVal);
	} else {
		dialogTalk();
	}

	_vm->_eventsManager.VBL();
  return retVal;
}

int TalkManager::dialogAnswer(int idx, bool animatedFl) {
	int charIdx;
	byte *charBuf;
	for (charBuf = _characterBuffer + 110, charIdx = 0; READ_LE_INT16(charBuf) != idx; charBuf += 20) {
		++charIdx;
		if (READ_LE_INT16((uint16 *)_characterBuffer + 42) < charIdx)
			return -1;
	}

	int mesgId = READ_LE_INT16((uint16 *)charBuf + 1);
	int mesgPosX = READ_LE_INT16((uint16 *)charBuf + 2);
	int mesgPosY = READ_LE_INT16((uint16 *)charBuf + 3);
	int mesgLength = READ_LE_INT16((uint16 *)charBuf + 4);
	_dialogueMesgId1 = READ_LE_INT16((uint16 *)charBuf + 5);
	_dialogueMesgId2 = READ_LE_INT16((uint16 *)charBuf + 6);
	_dialogueMesgId3 = READ_LE_INT16((uint16 *)charBuf + 7);
	int frameNumb = READ_LE_INT16((uint16 *)charBuf + 8);

	// CHECKME:_data[svField4] is useless?
	int v7 = READ_LE_INT16((uint16 *)charBuf + 9);
	if (v7)
		_vm->_globals._saveData->_data[svField4] = v7;

	if (!frameNumb)
		frameNumb = 10;
	if (animatedFl) {
		uint16 *bufPtr = (uint16 *)_characterBuffer + 43;
		int curVal = READ_LE_INT16(bufPtr);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 1);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 2);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 3);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 4);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);
	} else {
		VISU_PARLE();
	}

	bool displayedTxtFl = false;
	if (!_vm->_soundManager._textOffFl) {
		_vm->_fontManager.initTextBuffers(9, mesgId, _answersFilename, mesgPosX, mesgPosY, 5, mesgLength, 252);
		_vm->_fontManager.showText(9);
		displayedTxtFl = true;
	}
	if (!_vm->_soundManager.mixVoice(mesgId, 1, displayedTxtFl)) {
		_vm->_eventsManager._curMouseButton = 0;
		_vm->_eventsManager._mouseButton = 0;

		if (_vm->getIsDemo()) {
			for (int i = 0; i < frameNumb; i++) {
				_vm->_eventsManager.VBL();
			}
		} else {
			for (int i = 0; i < frameNumb; i++) {
				_vm->_eventsManager.VBL();
				if (_vm->_eventsManager._mouseButton || _vm->_eventsManager._curMouseButton)
					break;
				if (_vm->_eventsManager.getMouseButton() && i + 1 > abs(frameNumb / 5))
					break;
			}
		}
	}

	if (!_vm->_soundManager._textOffFl)
		_vm->_fontManager.hideText(9);
	if (animatedFl) {
		uint16 *bufPtr = (uint16 *)_characterBuffer + 43;
		int curVal = READ_LE_INT16(bufPtr);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 1);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 2);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 3);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);

		curVal = READ_LE_INT16(bufPtr + 4);
		if (curVal)
			_vm->_objectsManager.stopBobAnimation(curVal);
	} else {
		dialogEndTalk();
	}
	int result = 0;
	if (!_dialogueMesgId1)
		result = -1;

	return result;
}

void TalkManager::searchCharacterPalette(int startIdx, bool dark) {
	int palettePos = 0;
	size_t curIdx = startIdx;
	for (;;) {
		if (READ_BE_UINT24(&_characterBuffer[curIdx]) == MKTAG24('P', 'A', 'L')) {
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
		if (_vm->_globals._animBqe[idx]._enabledFl)
			BOB_VISU_PARLE(idx);
	}
}

void TalkManager::dialogTalk() {
	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals._animBqe[idx]._enabledFl)
			_vm->_objectsManager.hideBob(idx);
	}

	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals._animBqe[idx]._enabledFl)
			_vm->_objectsManager.resetBob(idx);
	}
}

void TalkManager::dialogEndTalk() {
	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals._animBqe[idx]._enabledFl)
			_vm->_objectsManager.hideBob(idx);
	}

	_vm->_eventsManager.VBL();
	_vm->_eventsManager.VBL();

	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals._animBqe[idx]._enabledFl)
			_vm->_objectsManager.resetBob(idx);
	}
}

int TalkManager::countBoxLines(int idx, const Common::String &file) {
	_vm->_fontManager._fontFixedWidth = 11;

	// Build up the filename
	Common::String filename;
	Common::String dest;
	filename = dest = file;
	while (filename.lastChar() != '.')
		filename.deleteLastChar();
	filename += "IND";

	Common::File f;
	if (!f.open(filename))
		error("Could not open file - %s", filename.c_str());
	int filesize = f.size();
	assert(filesize < 16188);

	uint32 indexData[4047];
	for (int i = 0; i < (filesize / 4); ++i)
		indexData[i] = f.readUint32LE();
	f.close();

	if (!f.open(dest))
		error("Error opening file - %s", dest.c_str());

	f.seek(indexData[idx]);
	byte *decryptBuf = _vm->_globals.allocMemory(2058);
	assert(decryptBuf != g_PTRNUL);

	f.read(decryptBuf, 2048);
	f.close();

	// Decrypt buffer
	byte *curDecryptPtr = decryptBuf;
	for (int i = 0; i < 2048; i++) {
		char curByte = *curDecryptPtr;
		if ((byte)(curByte + 46) > 27) {
			if ((byte)(curByte + 80) > 27) {
				if ((curByte >= 'A' && curByte <= 'Z') || (curByte >= 'a' && curByte <= 'z'))
					curByte = ' ';
			} else {
				curByte -= 79;
			}
		} else {
			curByte += 111;
		}
		*curDecryptPtr = curByte;
		curDecryptPtr++;
	}

	// Separate strings
	for (int i = 0; i < 2048; i++) {
		if ( decryptBuf[i] == 10 || decryptBuf[i] == 13 )
			decryptBuf[i] = 0;
	}

	// Check size of each strings in order to compute box width
	int curBufIndx = 0;
	int lineCount = 0;
	int lineSize = 0;
	char curChar;
	do {
		int curLineSize = 0;
		for (;;) {
			lineSize = curLineSize;
			do {
				curChar = decryptBuf[curBufIndx + curLineSize];
				++curLineSize;
			} while (curChar != ' ' && curChar != '%');

			if (curLineSize >= MIN_LETTERS_PER_LINE - 1) {
				if (curChar == '%')
					curChar = ' ';
				break;
			}

			if (curChar == '%') {
				lineSize = curLineSize;
				break;
			}
		}
		++lineCount;
		curBufIndx += lineSize;
	} while (curChar != '%');
	_vm->_globals.freeMemory(decryptBuf);
	return lineCount;
}

void TalkManager::VISU_PARLE() {
	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals._animBqe[idx]._enabledFl)
			BOB_VISU_PARLE(idx);
	}
}

void TalkManager::BOB_VISU_PARLE(int idx) {
	_vm->_objectsManager._priorityFl = true;
	if (!_vm->_objectsManager._bob[idx].field0) {
		_vm->_objectsManager.resetBob(idx);
		byte *v5 = _vm->_globals._animBqe[idx]._data;
		int v4 = READ_LE_INT16(v5 + 2);
		if (!v4)
			v4 = 1;
		if (READ_LE_INT16(v5 + 24)) {
			_vm->_objectsManager._bob[idx]._isSpriteFl = true;
			_vm->_objectsManager._bob[idx]._zoomFactor = 0;
			_vm->_objectsManager._bob[idx]._flipFl = false;
			_vm->_objectsManager._bob[idx]._animData = _vm->_globals._animBqe[idx]._data;
			_vm->_objectsManager._bob[idx].field0 = 10;
			v5 = _characterSprite;
			_vm->_objectsManager._bob[idx]._spriteData = _characterSprite;
			_vm->_objectsManager._bob[idx].field1E = v4;
			_vm->_objectsManager._bob[idx].field20 = -1;
			_vm->_objectsManager._bob[idx].field22 = 0;
		}
	}
}

void TalkManager::startCharacterAnim0(int startIdx, bool readOnlyFl) {
	int animIdx = 0;
	size_t curIdx = startIdx;
	for (;;) {
		if (READ_BE_UINT32(&_characterBuffer[curIdx]) == MKTAG('A', 'N', 'I', 'M') && _characterBuffer[curIdx + 4] == 1) {
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
			v7 = READ_LE_INT16(&_characterAnim[2 * idx + 4]);
			if (v7 && _vm->_globals._speed != 501)
				_vm->_graphicsManager.fastDisplay(_characterSprite, _vm->_eventsManager._startPos.x + READ_LE_INT16(&_characterAnim[2 * idx]),
				    READ_LE_INT16(&_characterAnim[2 * idx + 2]), _characterAnim[2 * idx + 8]);
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
	int curVal = READ_LE_INT16(bufPtr);
	if (curVal)
		searchCharacterAnim(21, animPtr, curVal, _characterSize);

	curVal = READ_LE_INT16(bufPtr + 1);
	if (curVal)
		searchCharacterAnim(22, animPtr, curVal, _characterSize);

	curVal = READ_LE_INT16(bufPtr + 2);
	if (curVal)
		searchCharacterAnim(23, animPtr, curVal, _characterSize);

	curVal = READ_LE_INT16(bufPtr + 3);
	if (curVal)
		searchCharacterAnim(24, animPtr, curVal, _characterSize);

	curVal = READ_LE_INT16(bufPtr + 4);
	if (curVal)
		searchCharacterAnim(25, animPtr, curVal, _characterSize);

	curVal = READ_LE_INT16(bufPtr + 5);
	if (curVal)
		searchCharacterAnim(26, animPtr, curVal, _characterSize);

	curVal = READ_LE_INT16(bufPtr + 6);
	if (curVal)
		searchCharacterAnim(27, animPtr, curVal, _characterSize);

	curVal = READ_LE_INT16(bufPtr + 7);
	if (curVal)
		searchCharacterAnim(28, animPtr, curVal, _characterSize);

	curVal = READ_LE_INT16(bufPtr + 8);
	if (curVal)
		searchCharacterAnim(29, animPtr, curVal, _characterSize);

	curVal = READ_LE_INT16(bufPtr + 9);
	if (curVal)
		searchCharacterAnim(30, animPtr, curVal, _characterSize);
}

void TalkManager::clearCharacterAnim() {
	for (int idx = 21; idx <= 34; ++idx) {
		_vm->_globals._animBqe[idx]._data = _vm->_globals.freeMemory(_vm->_globals._animBqe[idx]._data);
		_vm->_globals._animBqe[idx]._enabledFl = false;
	}
}

bool TalkManager::searchCharacterAnim(int idx, const byte *bufPerso, int animId, int bufferSize) {
	bool result = false;

	for (int bufPos = 0; bufPos <= bufferSize; bufPos++) {
		if (READ_BE_UINT32(bufPerso + bufPos) == MKTAG('A', 'N', 'I', 'M') && bufPerso[bufPos + 4] == animId) {
			int bufIndx = bufPos + 5;
			const byte *curPtr = bufPerso + bufIndx;
			int animLength = 0;
			bool loopCond = false;
			do {
				if (READ_BE_UINT32(curPtr) == MKTAG('A', 'N', 'I', 'M') || READ_BE_UINT24(curPtr) == MKTAG24('F', 'I', 'N'))
					loopCond = true;
				if (bufIndx > bufferSize) {
					_vm->_globals._animBqe[idx]._enabledFl = false;
					_vm->_globals._animBqe[idx]._data = g_PTRNUL;
					return false;
				}
				++bufIndx;
				++animLength;
				++curPtr;
			} while (!loopCond);
			_vm->_globals._animBqe[idx]._data = _vm->_globals.allocMemory(animLength + 50);
			_vm->_globals._animBqe[idx]._enabledFl = true;
			memcpy(_vm->_globals._animBqe[idx]._data, (const byte *)(bufPerso + bufPos + 5), 20);
			int v23 = READ_LE_INT16(bufPos + bufPerso + 29);
			WRITE_LE_UINT16(_vm->_globals._animBqe[idx]._data + 20, READ_LE_INT16(bufPos + bufPerso + 25));
			WRITE_LE_UINT16(_vm->_globals._animBqe[idx]._data + 22, READ_LE_INT16(bufPos + bufPerso + 27));
			WRITE_LE_UINT16(_vm->_globals._animBqe[idx]._data + 24, v23);
			WRITE_LE_UINT16(_vm->_globals._animBqe[idx]._data + 26, READ_LE_INT16(bufPos + bufPerso + 31));
			_vm->_globals._animBqe[idx]._data[28] = bufPerso[bufPos + 33];
			_vm->_globals._animBqe[idx]._data[29] = bufPerso[bufPos + 34];
			byte *bqeCurData = _vm->_globals._animBqe[idx]._data + 20;
			const byte *curBufPerso = bufPos + bufPerso + 25;
			for (int i = 1; i < 5000; i++) {
				bqeCurData += 10;
				curBufPerso += 10;
				if (!v23)
					break;
				v23 = READ_LE_INT16(curBufPerso + 4);
				WRITE_LE_UINT16(bqeCurData, READ_LE_INT16(curBufPerso));
				WRITE_LE_UINT16(bqeCurData + 2, READ_LE_INT16(curBufPerso + 2));
				WRITE_LE_UINT16(bqeCurData + 4, v23);
				WRITE_LE_UINT16(bqeCurData + 6, READ_LE_INT16(curBufPerso + 6));
				bqeCurData[8] = curBufPerso[8];
				bqeCurData[9] = curBufPerso[9];
			}
			result = true;
		}
		if (READ_BE_UINT24(&bufPerso[bufPos]) == MKTAG24('F', 'I', 'N'))
			result = true;

		if (result)
			break;
	}

	return result;
}

void TalkManager::REPONSE(int zone, int verb) {
	byte zoneObj = zone;
	byte verbObj = verb;
	
	bool outerLoopFl;
	byte *ptr = g_PTRNUL;
	do {
		outerLoopFl = false;
		bool tagFound = false;
		if (_vm->_globals._answerBuffer == g_PTRNUL)
			return;

		byte *curAnswerBuf = _vm->_globals._answerBuffer;
		for (;;) {
			if (READ_BE_UINT24(curAnswerBuf) == MKTAG24('F', 'I', 'N'))
				return;
			if (READ_BE_UINT24(curAnswerBuf) == MKTAG24('C', 'O', 'D')) {
				if (curAnswerBuf[3] == zoneObj && curAnswerBuf[4] == verbObj)
					tagFound = true;
			}
			if (!tagFound)
				curAnswerBuf++;
			else
				break;
		}

		// 'COD' tag found
		curAnswerBuf += 5;
		ptr = _vm->_globals.allocMemory(620);
		assert(ptr != g_PTRNUL);
		memset(ptr, 0, 620);
		uint16 v7 = 0;
		int v12 = 0;
		bool innerLoopCond = false;
		do {
			tagFound = false;
			if (READ_BE_UINT16(&curAnswerBuf[v7]) == MKTAG16('F', 'C')) {
				++v12;
				assert(v12 < (620 / 20));

				byte *v8 = (ptr + 20 * v12);
				uint16 anwerIdx = 0;
				do {
					assert(anwerIdx < 20);
					v8[anwerIdx++] = curAnswerBuf[v7++];
					if (READ_BE_UINT16(&curAnswerBuf[v7]) == MKTAG16('F', 'F')) {
						tagFound = true;
						v8[anwerIdx] = 'F';
						v8[anwerIdx + 1] = 'F';
						++v7;
					}
				} while (!tagFound);
			}
			if (!tagFound) {
				uint32 signature24 = READ_BE_UINT24(&curAnswerBuf[v7]);
				if (signature24 == MKTAG24('C', 'O', 'D') || signature24 == MKTAG24('F', 'I', 'N'))
					innerLoopCond = true;
			}
			curAnswerBuf += v7 + 1;
			v7 = 0;
		} while (!innerLoopCond);
		innerLoopCond = false;
		int lastOpcodeResult = 1;
		do {
			int opcodeType = _vm->_scriptManager.handleOpcode(ptr + 20 * lastOpcodeResult);
			if (_vm->shouldQuit())
				return;

			if (opcodeType == 2)
				// GOTO
				lastOpcodeResult =  _vm->_scriptManager.handleGoto(ptr + 20 * lastOpcodeResult);
			else if (opcodeType == 3)
				// IF
				lastOpcodeResult =  _vm->_scriptManager.handleIf(ptr, lastOpcodeResult);

			if (lastOpcodeResult == -1)
				error("Invalid IFF function");

			if (opcodeType == 1 || opcodeType == 4)
				// Already handled opcode or END IF
				++lastOpcodeResult;
			else if (!opcodeType || opcodeType == 5)
				// EXIT
				innerLoopCond = true;
			else if (opcodeType == 6) {
				// JUMP
				_vm->_globals.freeMemory(ptr);
				zoneObj = _vm->_objectsManager._jumpZone;
				verbObj = _vm->_objectsManager._jumpVerb;
				outerLoopFl = true;
				break;
			}
		} while (!innerLoopCond);
	} while (outerLoopFl);
	_vm->_globals.freeMemory(ptr);
	_vm->_globals._saveData->_data[svField2] = 0;
	return;
}

void TalkManager::REPONSE2(int zone, int verb) {
	int indx = 0;
	if (verb != 5 || _vm->_globals._saveData->_data[svField3] != 4)
		return;

	if (zone == 22 || zone == 23) {
		_vm->_objectsManager.setFlipSprite(0, false);
		_vm->_objectsManager.setSpriteIndex(0, 62);
		_vm->_objectsManager.SPACTION(_vm->_objectsManager._forestSprite, "2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 4, false);
		if (zone == 22) {
			_vm->_objectsManager.lockAnimX(6, _vm->_objectsManager.getBobPosX(3));
			_vm->_objectsManager.lockAnimX(8, _vm->_objectsManager.getBobPosX(3));
		} else { // zone == 23
			_vm->_objectsManager.lockAnimX(6, _vm->_objectsManager.getBobPosX(4));
			_vm->_objectsManager.lockAnimX(8, _vm->_objectsManager.getBobPosX(4));
		}
		_vm->_objectsManager.stopBobAnimation(3);
		_vm->_objectsManager.stopBobAnimation(4);
		_vm->_objectsManager.setBobAnimation(6);
		_vm->_soundManager.playSample(1);
		_vm->_objectsManager.SPACTION1(_vm->_objectsManager._forestSprite, "13,14,15,14,13,12,13,14,15,16,-1,", 0, 0, 4);
		do
			_vm->_eventsManager.VBL();
		while (_vm->_objectsManager.getBobAnimDataIdx(6) < 12);
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
		_vm->_linesManager.disableZone(22);
		_vm->_linesManager.disableZone(23);
	} else if (zone == 20 || zone == 21) {
		_vm->_objectsManager.setFlipSprite(0, true);
		_vm->_objectsManager.setSpriteIndex(0, 62);
		_vm->_objectsManager.SPACTION(_vm->_objectsManager._forestSprite, "2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 4, true);
		if (zone == 20) {
			_vm->_objectsManager.lockAnimX(5, _vm->_objectsManager.getBobPosX(1));
			_vm->_objectsManager.lockAnimX(7, _vm->_objectsManager.getBobPosX(1));
		} else { // zone == 21
			_vm->_objectsManager.lockAnimX(5, _vm->_objectsManager.getBobPosX(2));
			_vm->_objectsManager.lockAnimX(7, _vm->_objectsManager.getBobPosX(2));
		}
		_vm->_objectsManager.stopBobAnimation(1);
		_vm->_objectsManager.stopBobAnimation(2);
		_vm->_objectsManager.setBobAnimation(5);
		_vm->_soundManager.playSample(1);
		_vm->_objectsManager.SPACTION1(_vm->_objectsManager._forestSprite, "13,14,15,14,13,12,13,14,15,16,-1,", 0, 0, 4);
		do
			_vm->_eventsManager.VBL();
		while (_vm->_objectsManager.getBobAnimDataIdx(5) < 12);
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
		_vm->_linesManager.disableZone(21);
		_vm->_linesManager.disableZone(20);
	}
}

void TalkManager::animateObject(const Common::String &a2) {
	_vm->_fontManager.hideText(5);
	_vm->_fontManager.hideText(9);
	_vm->_eventsManager.VBL();
	_vm->_graphicsManager._scrollStatus = 1;
	_vm->_linesManager.clearAllZones();
	_vm->_linesManager.resetLines();
	_vm->_globals.resetHidingItems();
	
	for (int i = 0; i <= 44; i++)
		_vm->_linesManager.BOBZONE[i] = 0;

	_vm->_objectsManager._zoneNum = -1;
	_vm->_eventsManager._mouseCursorId = 4;
	_vm->_eventsManager.changeMouseCursor(0);
	_characterBuffer = _vm->_fileManager.searchCat(a2, 5);
	_characterSize = _vm->_globals._catalogSize;
	if (_characterBuffer == g_PTRNUL) {
		_characterBuffer = _vm->_fileManager.loadFile(a2);
		_characterSize = _vm->_fileManager.fileSize(a2);
	}
	Common::String screenFilename;
	Common::String spriteFilename;
	Common::String curScreenFilename;
	getStringFromBuffer(40, spriteFilename, (const char *)_characterBuffer);
	getStringFromBuffer(0, screenFilename, (const char *)_characterBuffer);
	getStringFromBuffer(20, curScreenFilename, (const char *)_characterBuffer);

	if (curScreenFilename == "NULL")
		curScreenFilename = Common::String::format("IM%d", _vm->_globals._screenId);

	_characterSprite = _vm->_fileManager.searchCat(spriteFilename, 7);
	if (_characterSprite)
		_characterSprite = _vm->_objectsManager.loadSprite(spriteFilename);
	else
		_characterSprite = _vm->_objectsManager.loadSprite("RES_SAN.RES");

	if (_vm->_graphicsManager._lineNbr == SCREEN_WIDTH)
		_vm->_saveLoadManager.saveFile("TEMP.SCR", _vm->_graphicsManager._vesaScreen, 307200);
	else if (_vm->_graphicsManager._lineNbr == (SCREEN_WIDTH * 2))
		_vm->_saveLoadManager.saveFile("TEMP.SCR", _vm->_graphicsManager._vesaScreen, 614400);

	if (!_vm->_graphicsManager._lineNbr)
		_vm->_graphicsManager._scrollOffset = 0;
	_vm->_graphicsManager.NB_SCREEN(true);
	_paletteBufferIdx = 20 * READ_LE_INT16((uint16 *)_characterBuffer + 42) + 110;
	_vm->_graphicsManager.NB_SCREEN(true);
	_vm->_objectsManager.PERSO_ON = true;
	searchCharacterPalette(_paletteBufferIdx, true);
	startCharacterAnim0(_paletteBufferIdx, false);
	byte *oldAnswerBufferPtr = _vm->_globals._answerBuffer;
	_vm->_globals._answerBuffer = g_PTRNUL;
	_vm->_globals._freezeCharacterFl = true;
	_vm->_objectsManager.loadLinkFile(screenFilename);
	_vm->_objectsManager.PERSO_ON = true;
	_vm->_globals._actionMoveTo = false;
	_vm->_objectsManager._zoneNum = -1;
	initCharacterAnim();
	VISU_PARLE();
	dialogWait();
	_vm->_graphicsManager.initScreen(screenFilename, 2, true);
	_vm->_globals._freezeCharacterFl = true;
	_vm->_objectsManager._forceZoneFl = true;
	_vm->_objectsManager._zoneNum = -1;
	do {
		int mouseButton = _vm->_eventsManager.getMouseButton();
		if (mouseButton == 1)
			_vm->_objectsManager.handleLeftButton();
		else if (mouseButton == 2)
			_vm->_objectsManager.handleRightButton();

		_vm->_linesManager.checkZone();
		if (_vm->_globals._actionMoveTo)
			_vm->_objectsManager.PARADISE();
		_vm->_eventsManager.VBL();
	} while (!_vm->_globals._exitId);
	dialogEndTalk();
	dialogTalk();
	clearCharacterAnim();
	clearCharacterAnim();
	_vm->_globals._introSpeechOffFl = false;
	_characterBuffer = _vm->_globals.freeMemory(_characterBuffer);
	_characterSprite = _vm->_globals.freeMemory(_characterSprite);
	_vm->_graphicsManager.NB_SCREEN(false);
	_vm->_linesManager.clearAllZones();
	_vm->_linesManager.resetLines();
	_vm->_globals.resetHidingItems();
	for (int i = 0; i <= 44; i++)
		_vm->_linesManager.BOBZONE[i] = 0;

	_vm->_globals.freeMemory(_vm->_globals._answerBuffer);
	_vm->_globals._answerBuffer = oldAnswerBufferPtr;
	_vm->_objectsManager._disableFl = true;
	_vm->_objectsManager.loadLinkFile(curScreenFilename);
	_vm->_graphicsManager.initScreen(curScreenFilename, 2, true);
	_vm->_objectsManager._disableFl = false;
	_vm->_globals._freezeCharacterFl = false;
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
