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

#include "kyra/text_v3.h"

#include "kyra/screen_v3.h"

namespace Kyra {

TextDisplayer_v3::TextDisplayer_v3(KyraEngine_v3 *vm, Screen_v3 *screen)
	: TextDisplayer(vm, screen), _vm(vm), _screen(screen) {
}

char *TextDisplayer_v3::preprocessString(const char *str) {
	debugC(9, kDebugLevelMain, "TextDisplayer_v3::preprocessString('%s')", str);
	strcpy(_talkBuffer, str);

	char *p = _talkBuffer;
	while (*p) {
		if (*p++ == '\r')
			return _talkBuffer;
	}

	p = _talkBuffer;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;

	const int maxTextWidth = (_vm->language() == 0) ? 176 : 240;
	int textWidth = _screen->getTextWidth(p);

	if (textWidth > maxTextWidth) {
		int count = 0, offs = 0;
		if (textWidth > (3*maxTextWidth)) {
			count = getCharLength(p, textWidth/4);
			offs = dropCRIntoString(p, getCharLength(p, maxTextWidth), count);
			p += count + offs;
		}

		if (textWidth > (2*maxTextWidth)) {
			count = getCharLength(p, textWidth/3);
			offs = dropCRIntoString(p, getCharLength(p, maxTextWidth), count);
			p += count + offs;
			textWidth = _screen->getTextWidth(p);
		}

		count = getCharLength(p, textWidth/2);
		offs = dropCRIntoString(p, getCharLength(p, maxTextWidth), count);	
		p += count + offs;
		textWidth = _screen->getTextWidth(p);

		if (textWidth > maxTextWidth) {
			count = getCharLength(p, textWidth/2);
			offs = dropCRIntoString(p, getCharLength(p, maxTextWidth), count);	
		}
	}

	_screen->setFont(curFont);
	return _talkBuffer;
}

int TextDisplayer_v3::dropCRIntoString(char *str, int minOffs, int maxOffs) {
	debugC(9, kDebugLevelMain, "TextDisplayer_v3::dropCRIntoString('%s', %d, %d)", str, maxOffs, minOffs);

	int offset = 0;
	char *proc = str + minOffs;

	for (int i = minOffs; i < maxOffs; ++i) {
		if (*proc == ' ') {
			*proc = '\r';
			return offset;
		} else if (*proc == '-') {
			memmove(proc+1, proc, strlen(proc)+1);
			*(++proc) = '\r';
			++offset;
			return offset;
		}

		++offset;
		++proc;

		if (!*proc)
			return 0;
	}

	offset = 0;
	proc = str + minOffs;
	for (int i = minOffs; i >= 0; --i) {
		if (*proc == ' ') {
			*proc = '\r';
			return offset;
		} else if (*proc == '-') {
			memmove(proc+1, proc, strlen(proc)+1);
			*(++proc) = '\r';
			++offset;
			return offset;
		}

		--offset;
		--proc;

		if (!*proc)
			return 0;
	}

	*(str + minOffs) = '\r';
	return 0;
}

void TextDisplayer_v3::printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2, Screen::FontId font) {
	debugC(9, kDebugLevelMain, "TextDisplayer_v3::printText('%s', %d, %d, %d, %d, %d)", str, x, y, c0, c1, c2);
	uint8 colorMap[] = { 0, 255, 240, 240 };
	colorMap[3] = c1;
	_screen->setTextColor(colorMap, 0, 3);
	Screen::FontId curFont = _screen->setFont(font);
	_screen->_charWidth = -2;
	_screen->printText(str, x, y, c0, c2);
	_screen->_charWidth = 0;
	_screen->setFont(curFont);
}

void TextDisplayer_v3::restoreScreen() {
	debugC(9, kDebugLevelMain, "TextDisplayer_v3::restoreScreen()");
	_vm->restorePage3();
	_vm->drawAnimObjects();
	_screen->hideMouse();
	_screen->copyRegion(_talkCoords.x, _talkMessageY, _talkCoords.x, _talkMessageY, _talkCoords.w, _talkMessageH, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->showMouse();
	_vm->flagAnimObjsForRefresh();
	_vm->refreshAnimObjects(0);
}

void TextDisplayer_v3::calcWidestLineBounds(int &x1, int &x2, int w, int x) {
	debugC(9, kDebugLevelMain, "TextDisplayer_v3::calcWidestLineBounds(%d, %d)", w, x);
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

int KyraEngine_v3::chatGetType(const char *str) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::chatGetType('%s')", str);
	while (*str)
		++str;
	--str;
	switch (*str) {
	case '!':
		return 2;

	case ')':
		return 3;

	case '?':
		return 1;

	case '.':
	default:
		return 0;
	}
}

int KyraEngine_v3::chatCalcDuration(const char *str) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::chatCalcDuration('%s')", str);
	return MAX<int>(120, strlen(str)*6);
}

void KyraEngine_v3::objectChat(const char *str, int object, int vocHigh, int vocLow) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::objectChat('%s', %d, %d, %d)", str, object, vocHigh, vocLow);

	if (_mainCharacter.animFrame == 87 || _mainCharacter.animFrame == 0xFFFF || _mainCharacter.x1 <= 0 || _mainCharacter.y1 <= 0)
		return;

	_chatVocLow = _chatVocHigh = -1;
	objectChatInit(str, object, vocHigh, vocLow);
	_chatText = str;
	_chatObject = object;
	int chatType = chatGetType(str);

	if (_mainCharacter.facing > 7)
		_mainCharacter.facing = 5;

	static const uint8 talkScriptTable[] = {
		0x10, 0x11, 0x12, 0x13,
		0x0C, 0x0D, 0x0E, 0x0F,
		0x0C, 0x0D, 0x0E, 0x0F,
		0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03,
		0x00, 0x01, 0x02, 0x03,
		0x08, 0x09, 0x0A, 0x0B,
		0x08, 0x09, 0x0A, 0x0B
	};

	static const char *talkFilenameTable[] = {
		"MTFL00S.EMC", "MTFL00Q.EMC", "MTFL00E.EMC", "MTFL00T.EMC",
		"MTFR00S.EMC", "MTFR00Q.EMC", "MTFR00E.EMC", "MTRF00T.EMC",
		 "MTL00S.EMC",  "MTL00Q.EMC",  "MTL00E.EMC",  "MTL00T.EMC",
		 "MTR00S.EMC",  "MTR00Q.EMC",  "MTR00E.EMC",  "MTR00T.EMC",
		 "MTA00S.EMC",  "MTA00Q.EMC",  "MTA00Q.EMC",  "MTA00T.EMC"
	};

	int chat = talkScriptTable[chatType + _mainCharacter.facing * 4];
	objectChatProcess(talkFilenameTable[chat]);
	_text->restoreScreen();
	_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	updateCharacterAnim(0);
	_chatText = 0;
	_chatObject = -1;
	//setNextIdleAnimTimer();
}

void KyraEngine_v3::objectChatInit(const char *str, int object, int vocHigh, int vocLow) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::objectChatInit('%s', %d, %d, %d)", str, object, vocHigh, vocLow);
	str = _text->preprocessString(str);
	int lineNum = _text->buildMessageSubstrings(str);
	
	int xPos = 0, yPos = 0;

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

void KyraEngine_v3::objectChatPrintText(const char *str, int object) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::objectChatPrintText('%s', %d)", str, object);
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

		_text->printText(str, x, y, c1, 0xF0, 0);
	}
}

void KyraEngine_v3::objectChatProcess(const char *script) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::objectChatProcess('%s')", script);

	memset(&_chatScriptData, 0, sizeof(_chatScriptData));
	memset(&_chatScriptState, 0, sizeof(_chatScriptState));

	_scriptInterpreter->loadScript(script, &_chatScriptData, &_opcodesTemporary);
	_scriptInterpreter->initScript(&_chatScriptState, &_chatScriptData);
	_scriptInterpreter->startScript(&_chatScriptState, 0);
	while (_scriptInterpreter->validScript(&_chatScriptState))
		_scriptInterpreter->runScript(&_chatScriptState);

	if (_chatVocHigh >= 0) {
		playVoice(_chatVocHigh, _chatVocLow);
		_chatVocHigh = _chatVocLow = -1;
	}

	_useFrameTable = true;
	objectChatWaitToFinish();
	_useFrameTable = false;

	_scriptInterpreter->unloadScript(&_chatScriptData);
}

void KyraEngine_v3::objectChatWaitToFinish() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::objectChatWaitToFinish()");
	int charAnimFrame = _mainCharacter.animFrame;
	setCharacterAnimDim(_newShapeWidth, _newShapeHeight);

	_scriptInterpreter->initScript(&_chatScriptState, &_chatScriptData);
	_scriptInterpreter->startScript(&_chatScriptState, 1);

	bool running = true;
	const uint32 endTime = _chatEndTime;
	resetSkipFlag();

	while (running && !_quitFlag) {
		if (!_scriptInterpreter->validScript(&_chatScriptState))
			_scriptInterpreter->startScript(&_chatScriptState, 1);

		_temporaryScriptExecBit = false;
		while (!_temporaryScriptExecBit && _scriptInterpreter->validScript(&_chatScriptState)) {
			musicUpdate(0);
			_scriptInterpreter->runScript(&_chatScriptState);
		}

		int curFrame = _newShapeAnimFrame;
		uint32 delayTime = _newShapeDelay;

		_mainCharacter.animFrame = curFrame;
		updateCharacterAnim(0);

		uint32 nextFrame = _system->getMillis() + delayTime * _tickLength;

		while (_system->getMillis() < nextFrame && !_quitFlag) {
			updateWithText();

			const uint32 curTime = _system->getMillis();
			if ((textEnabled() && !speechEnabled() && curTime > endTime) || (speechEnabled() && !snd_voiceIsPlaying()) || skipFlag()) {
				snd_stopVoice();
				resetSkipFlag();
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

void KyraEngine_v3::badConscienceChat(const char *str, int vocHigh, int vocLow) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::badConscienceChat('%s', %d, %d)", str, vocHigh, vocLow);
	if (!_badConscienceShown)
		return;

	//setNextIdleAnimTimer();
	_chatVocHigh = _chatVocLow = -1;
	objectChatInit(str, 1, vocHigh, vocLow);
	_chatText = str;
	_chatObject = 1;
	badConscienceChatWaitToFinish();
	updateSceneAnim(0x0E, _badConscienceFrameTable[_badConscienceAnim+16]);
	_text->restoreScreen();
	update();
	_chatText = 0;
	_chatObject = -1;
}

void KyraEngine_v3::badConscienceChatWaitToFinish() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::badConscienceChatWaitToFinish()");
	if (_chatVocHigh) {
		playVoice(_chatVocHigh, _chatVocLow);
		_chatVocHigh = _chatVocLow = -1;
	}

	bool running = true;
	const uint32 endTime = _chatEndTime;
	resetSkipFlag();

	uint32 nextFrame = _system->getMillis() + _rnd.getRandomNumberRng(4, 8) * _tickLength;

	int frame = _badConscienceFrameTable[_badConscienceAnim+24];
	while (running && !_quitFlag) {
		if (nextFrame < _system->getMillis()) {
			++frame;
			if (_badConscienceFrameTable[_badConscienceAnim+32] < frame)
				frame = _badConscienceFrameTable[_badConscienceAnim+24];

			updateSceneAnim(0x0E, frame);
			updateWithText();

			nextFrame = _system->getMillis() + _rnd.getRandomNumberRng(4, 8) * _tickLength;
		}

		updateWithText();

		const uint32 curTime = _system->getMillis();
		if ((textEnabled() && !speechEnabled() && curTime > endTime) || (speechEnabled() && !snd_voiceIsPlaying()) || skipFlag()) {
			snd_stopVoice();
			resetSkipFlag();
			nextFrame = curTime;
			running = false;
		}

		delay(10);
	}
}

} // end of namespace Kyra

