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
 * $URL$
 * $Id$
 *
 */

#include "kyra/text_v2.h"
#include "kyra/kyra_v2.h"
#include "kyra/resource.h"

namespace Kyra {

TextDisplayer_v2::TextDisplayer_v2(KyraEngine_v2 *vm, Screen_v2 *screen)
	: TextDisplayer(vm, screen), _vm(vm) {
}

void TextDisplayer_v2::backupTalkTextMessageBkgd(int srcPage, int dstPage) {
	_screen->copyRegion(_talkCoords.x, _talkMessageY, 0, 144, _talkCoords.w, _talkMessageH, srcPage, dstPage);
}

void TextDisplayer_v2::restoreScreen() {
	_vm->restorePage3();
	_vm->drawAnimObjects();
	_screen->hideMouse();
	_screen->copyRegion(_talkCoords.x, _talkMessageY, _talkCoords.x, _talkMessageY, _talkCoords.w, _talkMessageH, 2, 0);
	_screen->showMouse();
	_vm->flagAnimObjsForRefresh();
	_vm->refreshAnimObjects(0);
}

char *TextDisplayer_v2::preprocessString(const char *str) {
	debugC(9, kDebugLevelMain, "TextDisplayer_v2::preprocessString('%s')", str);

	if (str != _talkBuffer) {
		assert(strlen(str) < sizeof(_talkBuffer) - 1);
		strcpy(_talkBuffer, str);
	}

	char *p = _talkBuffer;
	while (*p) {
		if (*p == '\r')
			return _talkBuffer;
		++p;
	}

	p = _talkBuffer;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	int textWidth = _screen->getTextWidth(p);
	_screen->_charWidth = 0;

	// longer text strings for German versions
	int maxTextWidth = (_vm->language() == 2 ? 240 : 176);

	if (textWidth > maxTextWidth) {
		if (textWidth > (maxTextWidth*2)) {
			int count = getCharLength(p, textWidth / 3);
			int offs = dropCRIntoString(p, count);
			p += count + offs;
			_screen->_charWidth = -2;
			textWidth = _screen->getTextWidth(p);
			_screen->_charWidth = 0;
			count = getCharLength(p, textWidth / 2);
			dropCRIntoString(p, count);
		} else {
			int count = getCharLength(p, textWidth / 2);
			dropCRIntoString(p, count);
		}
	}
	_screen->setFont(curFont);
	return _talkBuffer;
}

void TextDisplayer_v2::calcWidestLineBounds(int &x1, int &x2, int w, int x) {
	debugC(9, kDebugLevelMain, "TextDisplayer_v2::calcWidestLineBounds(%d, %d)", w, x);
	x1 = x;
	x1 -= (w >> 1);
	x2 = x1 + w + 1;

	if (x1 + w >= 311)
		x1 = 311 - w - 1;

	if (x1 < 8)
		x1 = 8;

	x2 = x1 + w + 1;
}

#pragma mark -

int KyraEngine_v2::chatGetType(const char *str) {
	str += strlen(str);
	--str;
	switch (*str) {
	case '!':
		return 2;

	case ')':
		return -1;

	case '?':
		return 1;

	default:
		return 0;
	}
}

int KyraEngine_v2::chatCalcDuration(const char *str) {
	static const uint8 durationMultiplicator[] = { 16, 14, 12, 10, 8, 8, 7, 6, 5, 4 };

	// TODO / HACK: imlement this correctly
	const int _configTextspeed = 50;

	int duration = strlen(str);
	duration *= _flags.isTalkie ? 8 : durationMultiplicator[(_configTextspeed / 10)];
	return MAX<int>(duration, 120);
}

void KyraEngine_v2::objectChat(const char *str, int object, int vocHigh, int vocLow) {
	setNextIdleAnimTimer();

	_chatVocHigh = _chatVocLow = -1;

	objectChatInit(str, object, vocHigh, vocLow);
	_chatText = str;
	_chatObject = object;
	_chatIsNote = (chatGetType(str) == -1);

	if (_mainCharacter.facing > 7)
		_mainCharacter.facing = 5;

	static const uint8 talkScriptTable[] = {
		6, 7, 8,
		3, 4, 5,
		3, 4, 5,
		0, 1, 2,
		0, 1, 2,
		0, 1, 2,
		3, 4, 5,
		3, 4, 5
	};

	assert(_mainCharacter.facing * 3 + object < ARRAYSIZE(talkScriptTable));
	int script = talkScriptTable[_mainCharacter.facing * 3 + object];

	static const char *chatScriptFilenames[] = {
		"_Z1FSTMT.EMC",
		"_Z1FQUES.EMC",
		"_Z1FEXCL.EMC",
		"_Z1SSTMT.EMC",
		"_Z1SQUES.EMC",
		"_Z1SEXCL.EMC",
		"_Z1BSTMT.EMC",
		"_Z1BQUES.EMC",
		"_Z1BEXCL.EMC"
	};

	objectChatProcess(chatScriptFilenames[script]);
	_chatIsNote = false;

	_text->restoreScreen();

	_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	updateCharacterAnim(0);

	_chatText = 0;
	_chatObject = -1;

	setNextIdleAnimTimer();
}

void KyraEngine_v2::objectChatInit(const char *str, int object, int vocHigh, int vocLow) {
	str = _text->preprocessString(str);
	int lineNum = _text->buildMessageSubstrings(str);

	int yPos = 0, xPos = 0;

	if (!object) {
		int scale = getScale(_mainCharacter.x1, _mainCharacter.y1);
		yPos = _mainCharacter.y1 - ((_mainCharacter.height * scale) >> 8) - 8;
		xPos = _mainCharacter.x1;
	} else {
		yPos = _talkObjectList[object].y;
		xPos = _talkObjectList[object].x;
	}

	yPos -= lineNum * 10;
	yPos = MAX(yPos, 0);
	_text->_talkMessageY = yPos;
	_text->_talkMessageH = lineNum*10;

	int width = _text->getWidestLineWidth(lineNum);
	_text->calcWidestLineBounds(xPos, yPos, width, xPos);
	_text->_talkCoords.x = xPos;
	_text->_talkCoords.w = width + 2;

	restorePage3();
	_text->backupTalkTextMessageBkgd(2, 2);

	_screen->hideMouse();

	if (textEnabled()) {
		objectChatPrintText(str, object);
		_chatEndTime = _system->getMillis() + chatCalcDuration(str) * _tickLength;
	} else {
		_chatEndTime = _system->getMillis();
	}

	if (speechEnabled()) {
		_chatVocHigh = vocHigh;
		_chatVocLow = vocLow;
	} else {
		_chatVocHigh = _chatVocLow = -1;
	}

	_screen->showMouse();
}

void KyraEngine_v2::objectChatPrintText(const char *str, int object) {
	int c1 = _talkObjectList[object].color;
	str = _text->preprocessString(str);
	int lineNum = _text->buildMessageSubstrings(str);
	int maxWidth = _text->getWidestLineWidth(lineNum);
	int x = (object == 0) ? _mainCharacter.x1 : _talkObjectList[object].x;
	int cX1 = 0, cX2 = 0;
	_text->calcWidestLineBounds(cX1, cX2, maxWidth, x);

	for (int i = 0; i < lineNum; ++i) {
		str = &_text->_talkSubstrings[i*_text->maxSubstringLen()];

		int y = _text->_talkMessageY + i * 10;
		x = _text->getCenterStringX(str, cX1, cX2);

		_text->printText(str, x, y, c1, 0xCF, 0);
	}
}

void KyraEngine_v2::objectChatProcess(const char *script) {
	memset(&_chatScriptData, 0, sizeof(_chatScriptData));
	memset(&_chatScriptState, 0, sizeof(_chatScriptState));

	_scriptInterpreter->loadScript(script, &_chatScriptData, &_opcodesTemporary);
	_scriptInterpreter->initScript(&_chatScriptState, &_chatScriptData);
	_scriptInterpreter->startScript(&_chatScriptState, 0);
	while (_scriptInterpreter->validScript(&_chatScriptState))
		_scriptInterpreter->runScript(&_chatScriptState);

	_newShapeFilename[2] = _loadedZTable + '0';
	uint8 *shapeBuffer = _res->fileData(_newShapeFilename, 0);
	if (shapeBuffer) {
		int shapeCount = initNewShapes(shapeBuffer);

		if (_chatVocHigh >= 0) {
			playVoice(_chatVocHigh, _chatVocLow);
			_chatVocHigh = _chatVocLow = -1;
		}

		objectChatWaitToFinish();

		resetNewShapes(shapeCount, shapeBuffer);
	} else {
		warning("couldn't load file '%s'", _newShapeFilename);
	}

	_scriptInterpreter->unloadScript(&_chatScriptData);
}

void KyraEngine_v2::objectChatWaitToFinish() {
	int charAnimFrame = _mainCharacter.animFrame;
	setCharacterAnimDim(_newShapeWidth, _newShapeHeight);

	_scriptInterpreter->initScript(&_chatScriptState, &_chatScriptData);
	_scriptInterpreter->startScript(&_chatScriptState, 1);

	bool running = true;
	const uint32 endTime = _chatEndTime;

	while (running && !_quitFlag) {
		if (!_scriptInterpreter->validScript(&_chatScriptState))
			_scriptInterpreter->startScript(&_chatScriptState, 1);

		_temporaryScriptExecBit = false;
		while (!_temporaryScriptExecBit && _scriptInterpreter->validScript(&_chatScriptState))
			_scriptInterpreter->runScript(&_chatScriptState);

		int curFrame = _newShapeAnimFrame;
		uint32 delayTime = _newShapeDelay;

		if (!_chatIsNote)
			_mainCharacter.animFrame = 33 + curFrame;

		updateCharacterAnim(0);

		uint32 nextFrame = _system->getMillis() + delayTime * _tickLength;

		while (_system->getMillis() < nextFrame && !_quitFlag) {
			updateWithText();

			int inputFlag = checkInput(0);
			removeInputTop();
			if (inputFlag == 198 || inputFlag == 199) {
				//XXX
				_skipFlag = true;
				snd_stopVoice();
			}

			const uint32 curTime = _system->getMillis();
			if ((textEnabled() && curTime > endTime) || (speechEnabled() && !snd_voiceIsPlaying()) || _skipFlag) {
				_skipFlag = false;
				nextFrame = curTime;
				running = false;
			}

			delay(10);
		}
	}

	_mainCharacter.animFrame = charAnimFrame;
	updateCharacterAnim(0);
	resetCharacterAnimDim();
}

void KyraEngine_v2::initTalkObject(int initObject) {
	TalkObject &object = _talkObjectList[initObject];

	char STAFilename[13];
	char TLKFilename[13];
	char ENDFilename[13];

	strcpy(STAFilename, object.filename);
	strcpy(TLKFilename, object.filename);
	strcpy(ENDFilename, object.filename);

	strcpy(STAFilename + 4, "_STA.TIM");
	strcpy(TLKFilename + 4, "_TLK.TIM");
	strcpy(ENDFilename + 4, "_END.TIM");

	_currentTalkSections.STATim = loadTIMFile(STAFilename, NULL, 0);
	_currentTalkSections.TLKTim = loadTIMFile(TLKFilename, NULL, 0);
	_currentTalkSections.ENDTim = loadTIMFile(ENDFilename, NULL, 0);

	if (object.scriptId != -1) {
		_specialSceneScriptStateBackup[object.scriptId] = _specialSceneScriptState[object.scriptId];
		_specialSceneScriptState[object.scriptId] = 1;
	}

	/*if (_currentTalkObject.STATim) {
		_objectChatFinished = false;
		while (!_objectChatFinished) {
			processTalkObject(_currentTalkObject.STATim, 0);
			if (_chatText)
				updateWithText();
			else
				update();
		}
	}*/
}

void KyraEngine_v2::deinitTalkObject(int initObject) {
	TalkObject &object = _talkObjectList[initObject];

	/*if (_currentTalkObject.ENDTim) {
		_objectChatFinished = false;
		while (!_objectChatFinished) {
			processTalkObject(_currentTalkObject.ENDTim, 0);
			if (_chatText)
				updateWithText();
			else
				update();
		}
	}*/

	if (object.scriptId != -1) {
		_specialSceneScriptState[object.scriptId] = _specialSceneScriptStateBackup[object.scriptId];
	}

	if (_currentTalkSections.STATim != NULL) {
		freeTIM(_currentTalkSections.STATim);
		_currentTalkSections.STATim = NULL;
	}

	if (_currentTalkSections.TLKTim != NULL) {
		freeTIM(_currentTalkSections.TLKTim);
		_currentTalkSections.TLKTim = NULL;
	}

	if (_currentTalkSections.ENDTim != NULL) {
		freeTIM(_currentTalkSections.ENDTim);
		_currentTalkSections.ENDTim = NULL;
	}
}

byte *KyraEngine_v2::loadTIMFile(const char *filename, byte *buffer, int32 bufferSize) {
	ScriptFileParser file(filename, _res);
	if (!file) {
		error("Couldn't open script file '%s'", filename);
		return NULL;
	}

	int32 formBlockSize = file.getFORMBlockSize();
	if (formBlockSize == -1) {
		error("No FORM chunk found in file: '%s'", filename);
		return NULL;
	}

	if (formBlockSize < 20) {
		return NULL;
	}

	formBlockSize += sizeof(TIMHeader) + 120 + sizeof(TIMStructUnk1) * 10;

	TIMHeader *timHeader;
	if (buffer == NULL || bufferSize < formBlockSize) {
		buffer = new byte[formBlockSize];
		timHeader = (TIMHeader *)buffer;
		timHeader->deleteBufferFlag = 0xBABE;
	} else {
		timHeader = (TIMHeader *)buffer;
		timHeader->deleteBufferFlag = 0x0;
	}

	int32 chunkSize = file.getIFFBlockSize(AVTL_CHUNK);
	timHeader->unkFlag = -1;
	timHeader->unkFlag2 = 0;
	timHeader->unkOffset = sizeof(TIMHeader);
	timHeader->unkOffset2 = timHeader->unkOffset + sizeof(TIMStructUnk1) * 10;
	timHeader->AVTLOffset = timHeader->unkOffset2 + 120;
	timHeader->TEXTOffset = timHeader->AVTLOffset + chunkSize;

	_TIMBuffers.AVTLChunk = (uint16 *)(buffer + timHeader->AVTLOffset);
	_TIMBuffers.TEXTChunk = buffer + timHeader->TEXTOffset;

	if (!file.loadIFFBlock(AVTL_CHUNK, _TIMBuffers.AVTLChunk, chunkSize)) {
		error("Couldn't load AVTL chunk from file: '%s'", filename);
		return NULL;
	}

	_TIMBuffers.UnkChunk = (TIMStructUnk1 *)(buffer + timHeader->unkOffset);

	for (int i = 0; i < 10; i++) {
		_TIMBuffers.UnkChunk[i].unk_0 = 0;
		_TIMBuffers.UnkChunk[i].unk_2 = 0;
		_TIMBuffers.UnkChunk[i].unk_20 = &_TIMBuffers.AVTLChunk[ _TIMBuffers.AVTLChunk[i] ];
		_TIMBuffers.UnkChunk[i].unk_4 = 0;
		_TIMBuffers.UnkChunk[i].unk_8 = 0;
	}

	chunkSize = file.getIFFBlockSize(TEXT_CHUNK);
	if (chunkSize > 0) {
		if (!file.loadIFFBlock(TEXT_CHUNK, _TIMBuffers.TEXTChunk, chunkSize)) {
			error("Couldn't load TEXT chunk from file: '%s'", filename);
			return NULL;
		}
	}

	return buffer;
}

void KyraEngine_v2::freeTIM(byte *buffer) {
	TIMHeader *timHeader = (TIMHeader *)buffer;

	if (timHeader->deleteBufferFlag == 0xBABE) {
		delete[] buffer;
	}
}

} // end of namespace Kyra


