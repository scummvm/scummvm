/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/text.h"
#include "kyra/animator.h"
#include "kyra/sprites.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine::waitForChatToFinish(int16 chatDuration, const char *chatStr, uint8 charNum) {
	debugC(9, kDebugLevelMain, "KyraEngine::waitForChatToFinish(%i, %s, %i)", chatDuration, chatStr, charNum); 
	bool hasUpdatedNPCs = false;
	bool runLoop = true;
	uint8 currPage;
	OSystem::Event event;
	int16 delayTime;

	//while( towns_isEscKeyPressed() )
		//towns_getKey();

	uint32 timeToEnd = strlen(chatStr) * 8 * _tickLength + _system->getMillis();

	if (_configVoice == 0 && chatDuration != -1) {
		switch (_configTextspeed) {
		case 0:
			chatDuration *= 2;
			break;
		case 2:
			chatDuration /= 4;
			break;
		case 3:
			chatDuration = -1;
			break;
		}
	}

	if (chatDuration != -1)
		chatDuration *= _tickLength;

	disableTimer(14);
	disableTimer(18);
	disableTimer(19);

	uint32 timeAtStart = _system->getMillis();
	uint32 loopStart;
	while (runLoop) {
		loopStart = _system->getMillis();
		if (_currentCharacter->sceneId == 210)
			if (seq_playEnd())
				break;

		if (_system->getMillis() > timeToEnd && !hasUpdatedNPCs) {
			hasUpdatedNPCs = true;
			disableTimer(15);
			_currHeadShape = 4;
			_animator->animRefreshNPC(0);
			_animator->animRefreshNPC(_talkingCharNum);

			if (_charSayUnk2 != -1) {
				_animator->sprites()[_charSayUnk2].active = 0;
				_sprites->_anims[_charSayUnk2].play = false;
				_charSayUnk2 = -1;
			}
		}

		updateGameTimers();
		_sprites->updateSceneAnims();
		_animator->restoreAllObjectBackgrounds();
		_animator->preserveAnyChangedBackgrounds();
		_animator->prepDrawAllObjects();

		currPage = _screen->_curPage;
		_screen->_curPage = 2;
		_text->printCharacterText(chatStr, charNum, _characterList[charNum].x1);
		_animator->_updateScreen = true;
		_screen->_curPage = currPage;

		_animator->copyChangedObjectsForward(0);
		updateTextFade();

		if ((chatDuration < (int16)(_system->getMillis() - timeAtStart)) && chatDuration != -1)
			break;

		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode == '.')
					_skipFlag = true;
				break;
			case OSystem::EVENT_QUIT:
				quitGame();
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				runLoop = false;
				break;
			default:
				break;
			}
		}
		
		if (_skipFlag)
			runLoop = false;

		delayTime = (loopStart + _gameSpeed) - _system->getMillis();
		if (delayTime > 0)
			_system->delayMillis(delayTime);
	}

	enableTimer(14);
	enableTimer(15);
	enableTimer(18);
	enableTimer(19);
	//clearKyrandiaButtonIO();
}

void KyraEngine::endCharacterChat(int8 charNum, int16 convoInitialized) {
	_charSayUnk3 = -1;

	if (charNum > 4 && charNum < 11) {
		//TODO: weird _game_inventory stuff here
		warning("STUB: endCharacterChat() for high charnums");
	}

	if (convoInitialized != 0) {
		_talkingCharNum = -1;
		_currentCharacter->currentAnimFrame = 7;
		_animator->animRefreshNPC(0);
		_animator->updateAllObjectShapes();
	}
}

void KyraEngine::restoreChatPartnerAnimFrame(int8 charNum) {
	_talkingCharNum = -1;

	if (charNum > 0 && charNum < 5) {
		_characterList[charNum].currentAnimFrame = _currentChatPartnerBackupFrame;
		_animator->animRefreshNPC(charNum);
	}

	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	_animator->updateAllObjectShapes();
}

void KyraEngine::backupChatPartnerAnimFrame(int8 charNum) {
	_talkingCharNum = 0;

	if (charNum < 5 && charNum > 0) 
		_currentChatPartnerBackupFrame = _characterList[charNum].currentAnimFrame;

	if (_scaleMode != 0)
		_currentCharacter->currentAnimFrame = 7;
	else
		_currentCharacter->currentAnimFrame = _currentCharAnimFrame;

	_animator->animRefreshNPC(0);
	_animator->updateAllObjectShapes();
}

int8 KyraEngine::getChatPartnerNum() {
	uint8 sceneTable[] = {0x2, 0x5, 0x2D, 0x7, 0x1B, 0x8, 0x22, 0x9, 0x30, 0x0A};
	int pos = 0;
	int partner = -1;

	for (int i = 1; i < 6; i++) {
		if (_currentCharacter->sceneId == sceneTable[pos]) {
			partner = sceneTable[pos+1];
			break;
		}
		pos += 2;
	}

	for (int i = 1; i < 5; i++) {
		if (_characterList[i].sceneId == _currentCharacter->sceneId) {
			partner = i;
			break;
		}
	}
	return partner;
}

int KyraEngine::initCharacterChat(int8 charNum) {
	if (_talkingCharNum == -1) {
		_talkingCharNum = 0;

		if (_scaleMode != 0)
			_currentCharacter->currentAnimFrame = 7;
		else
			_currentCharacter->currentAnimFrame = 16;

		_animator->animRefreshNPC(0);
		_animator->updateAllObjectShapes();
	}

	_charSayUnk2 = -1;
	_animator->flagAllObjectsForBkgdChange();
	_animator->restoreAllObjectBackgrounds();

	if (charNum > 4 && charNum < 11) {
		// TODO: Fill in weird _game_inventory stuff here
		warning("STUB: initCharacterChat() for high charnums");
	}

	_animator->flagAllObjectsForRefresh();
	_animator->flagAllObjectsForBkgdChange();
	_animator->preserveAnyChangedBackgrounds();
	_charSayUnk3 = charNum;

	return 1;
}

void KyraEngine::characterSays(const char *chatStr, int8 charNum, int8 chatDuration) {
	debugC(9, kDebugLevelMain, "KyraEngine::characterSays('%s', %i, %d)", chatStr, charNum, chatDuration);
	uint8 startAnimFrames[] =  { 0x10, 0x32, 0x56, 0x0, 0x0, 0x0 };

	uint16 chatTicks;
	int16 convoInitialized;
	int8 chatPartnerNum;

	if (_currentCharacter->sceneId == 210)
		return;

	convoInitialized = initCharacterChat(charNum);	
	chatPartnerNum = getChatPartnerNum();

	if (chatPartnerNum != -1 && chatPartnerNum < 5)
		backupChatPartnerAnimFrame(chatPartnerNum);

	if (charNum < 5) {
		_characterList[charNum].currentAnimFrame = startAnimFrames[charNum];
		_charSayUnk3 = charNum;
		_talkingCharNum = charNum;
		_animator->animRefreshNPC(charNum);
	}

	char *processedString = _text->preprocessString(chatStr);
	int lineNum = _text->buildMessageSubstrings(processedString);

	int16 yPos = _characterList[charNum].y1;
	yPos -= _scaleTable[charNum] * _characterList[charNum].height;
	yPos -= 8;
	yPos -= lineNum * 10;

	if (yPos < 11)
		yPos = 11;

	if (yPos > 100)
		yPos = 100;

	_text->_talkMessageY = yPos;
	_text->_talkMessageH = lineNum * 10;
	_animator->restoreAllObjectBackgrounds();

	_screen->copyRegion(12, _text->_talkMessageY, 12, 136, 296, _text->_talkMessageH, 2, 2);
	_screen->hideMouse();

	_text->printCharacterText(processedString, charNum, _characterList[charNum].x1);
	_screen->showMouse();

	if (chatDuration == -2)
		chatTicks = strlen(processedString) * 9;
	else
		chatTicks = chatDuration;

	waitForChatToFinish(chatTicks, chatStr, charNum);

	_animator->restoreAllObjectBackgrounds();

	_screen->copyRegion(12, 136, 12, _text->_talkMessageY, 296, _text->_talkMessageH, 2, 2);
	_animator->preserveAllBackgrounds();
	_animator->prepDrawAllObjects();
	_screen->hideMouse();

	_screen->copyRegion(12, _text->_talkMessageY, 12, _text->_talkMessageY, 296, _text->_talkMessageH, 2, 0);
	_screen->showMouse();
	_animator->flagAllObjectsForRefresh();
	_animator->copyChangedObjectsForward(0);

	if (chatPartnerNum != -1 && chatPartnerNum < 5)
		restoreChatPartnerAnimFrame(chatPartnerNum);

	endCharacterChat(charNum, convoInitialized);
}

void KyraEngine::drawSentenceCommand(const char *sentence, int color) {
	debugC(9, kDebugLevelMain, "KyraEngine::drawSentenceCommand('%s', %i)", sentence, color);
	_screen->hideMouse();
	_screen->fillRect(8, 143, 311, 152, 12);

	if (_startSentencePalIndex != color || _fadeText != false) {
		_currSentenceColor[0] = _screen->_currentPalette[765] = _screen->_currentPalette[color*3];
		_currSentenceColor[1] = _screen->_currentPalette[766] = _screen->_currentPalette[color*3+1];
		_currSentenceColor[2] = _screen->_currentPalette[767] = _screen->_currentPalette[color*3+2];
	
		_screen->setScreenPalette(_screen->_currentPalette);
		_startSentencePalIndex = 0;
	}

	_text->printText(sentence, 8, 143, 0xFF, 12, 0);
	_screen->showMouse();
	setTextFadeTimerCountdown(15);
	_fadeText = false;
}

void KyraEngine::updateSentenceCommand(const char *str1, const char *str2, int color) {
	debugC(9, kDebugLevelMain, "KyraEngine::updateSentenceCommand('%s', '%s', %i)", str1, str2, color);
	char sentenceCommand[500];
	strncpy(sentenceCommand, str1, 500);
	if (str2)
		strncat(sentenceCommand, str2, 500 - strlen(sentenceCommand));

	drawSentenceCommand(sentenceCommand, color);
	_screen->updateScreen();
}

void KyraEngine::updateTextFade() {
	debugC(9, kDebugLevelMain, "KyraEngine::updateTextFade()");
	if (!_fadeText)
		return;
	
	bool finished = false;
	for (int i = 0; i < 3; i++)
		if (_currSentenceColor[i] > 4)
			_currSentenceColor[i] -= 4;
		else
			if (_currSentenceColor[i]) {
				_currSentenceColor[i] = 0;
				finished = true;
			}
		
	_screen->_currentPalette[765] = _currSentenceColor[0];
	_screen->_currentPalette[766] = _currSentenceColor[1];
	_screen->_currentPalette[767] = _currSentenceColor[2];
	_screen->setScreenPalette(_screen->_currentPalette);

	if (finished) {
		_fadeText = false;
		_startSentencePalIndex = -1;
	}
}

TextDisplayer::TextDisplayer(Screen *screen) {
	_screen = screen;

	_talkCoords.y = 0x88;
	_talkCoords.x = 0;
	_talkCoords.w = 0;
	_talkMessageY = 0xC;
	_talkMessageH = 0;
	_talkMessagePrinted = false;
}

void TextDisplayer::setTalkCoords(uint16 y) {
	debugC(9, kDebugLevelMain, "TextDisplayer::setTalkCoords(%d)", y);
	_talkCoords.y = y;
}

int TextDisplayer::getCenterStringX(const char *str, int x1, int x2) {
	debugC(9, kDebugLevelMain, "TextDisplayer::getCenterStringX('%s', %d, %d)", str, x1, x2);
	_screen->_charWidth = -2;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	int strWidth = _screen->getTextWidth(str);
	_screen->setFont(curFont);
	_screen->_charWidth = 0;
	int w = x2 - x1 + 1;
	return x1 + (w - strWidth) / 2;
}

int TextDisplayer::getCharLength(const char *str, int len) {
	debugC(9, kDebugLevelMain, "TextDisplayer::getCharLength('%s', %d)", str, len);
	int charsCount = 0;
	if (*str) {
		_screen->_charWidth = -2;
		Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
		int i = 0;
		while (i <= len && *str) {
			i += _screen->getCharWidth(*str++);
			++charsCount;
		}
		_screen->setFont(curFont);
		_screen->_charWidth = 0;
	}
	return charsCount;
}

int TextDisplayer::dropCRIntoString(char *str, int offs) {
	debugC(9, kDebugLevelMain, "TextDisplayer::dropCRIntoString('%s', %d)", str, offs);
	int pos = 0;
	str += offs;
	while (*str) {
		if (*str == ' ') {
			*str = '\r';
			return pos;
		}
		++str;
		++pos;
	}
	return 0;
}

char *TextDisplayer::preprocessString(const char *str) {
	debugC(9, kDebugLevelMain, "TextDisplayer::preprocessString('%s')", str);
	if (str != _talkBuffer) {
		assert(strlen(str) < sizeof(_talkBuffer) - 1);
		strcpy(_talkBuffer, str);
	}
	char *p = _talkBuffer;
	while (*p) {
		if (*p == '\r') {
			return _talkBuffer;
		}
		++p;
	}
	p = _talkBuffer;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	int textWidth = _screen->getTextWidth(p);
	_screen->_charWidth = 0;
	if (textWidth > 176) {
		if (textWidth > 352) {
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

int TextDisplayer::buildMessageSubstrings(const char *str) {
	debugC(9, kDebugLevelMain, "TextDisplayer::buildMessageSubstrings('%s')", str);
	int currentLine = 0;
	int pos = 0;
	while (*str) {
		if (*str == '\r') {
			assert(currentLine < TALK_SUBSTRING_NUM);
			_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = '\0';
			++currentLine;
			pos = 0;
		} else {
			_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = *str;
			++pos;
			if (pos > TALK_SUBSTRING_LEN - 2) {
				pos = TALK_SUBSTRING_LEN - 2;
			}
		}
		++str;
	}
	_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = '\0';
	return currentLine + 1;
}

int TextDisplayer::getWidestLineWidth(int linesCount) {
	debugC(9, kDebugLevelMain, "TextDisplayer::getWidestLineWidth(%d)", linesCount);
	int maxWidth = 0;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	for (int l = 0; l < linesCount; ++l) {
		int w = _screen->getTextWidth(&_talkSubstrings[l * TALK_SUBSTRING_LEN]);
		if (maxWidth < w) {
			maxWidth = w;
		}
	}
	_screen->setFont(curFont);
	_screen->_charWidth = 0;
	return maxWidth;
}

void TextDisplayer::calcWidestLineBounds(int &x1, int &x2, int w, int cx) {
	debugC(9, kDebugLevelMain, "TextDisplayer::calcWidestLineBounds(%d, %d)", w, cx);
	x1 = cx - w / 2;
	if (x1 + w >= Screen::SCREEN_W - 12) {
		x1 = Screen::SCREEN_W - 12 - w - 1;
	} else if (x1 < 12) {
		x1 = 12;
	}
	x2 = x1 + w + 1;
}

void TextDisplayer::restoreTalkTextMessageBkgd(int srcPage, int dstPage) {
	debugC(9, kDebugLevelMain, "TextDisplayer::restoreTalkTextMessageBkgd(%d, %d)", srcPage, dstPage);
	if (_talkMessagePrinted) {
		_talkMessagePrinted = false;
		_screen->copyRegion(_talkCoords.x, _talkCoords.y, _talkCoords.x, _talkMessageY, _talkCoords.w, _talkMessageH, srcPage, dstPage);
	}
}

void TextDisplayer::printTalkTextMessage(const char *text, int x, int y, uint8 color, int srcPage, int dstPage) {
	debugC(9, kDebugLevelMain, "TextDisplayer::printTalkTextMessage('%s', %d, %d, %d, %d, %d)", text, x, y, color, srcPage, dstPage);
	char *str = preprocessString(text);
	int lineCount = buildMessageSubstrings(str);
	int top = y - lineCount * 10;
	if (top < 0) {
		top = 0;
	}
	_talkMessageY = top;
	_talkMessageH = lineCount * 10;
	int w = getWidestLineWidth(lineCount);
	int x1, x2;
	calcWidestLineBounds(x1, x2, w, x);
	_talkCoords.x = x1;
	_talkCoords.w = w + 2;
	_screen->copyRegion(_talkCoords.x, _talkMessageY, _talkCoords.x, _talkCoords.y, _talkCoords.w, _talkMessageH, srcPage, dstPage);
	int curPage = _screen->_curPage;
	_screen->_curPage = srcPage;
	for (int i = 0; i < lineCount; ++i) {
		top = i * 10 + _talkMessageY;
		char *msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
		int left = getCenterStringX(msg, x1, x2);
		printText(msg, left, top, color, 0xC, 0);
	}
	_screen->_curPage = curPage;
	_talkMessagePrinted = true;
}

void TextDisplayer::printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2) {
	debugC(9, kDebugLevelMain, "TextDisplayer::printText('%s', %d, %d, %d, %d, %d)", str, x, y, c0, c1, c2);
	uint8 colorMap[] = { 0, 15, 12, 12 };
	colorMap[3] = c1;
	_screen->setTextColor(colorMap, 0, 3);
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	_screen->printText(str, x, y, c0, c2);
	_screen->_charWidth = 0;
	_screen->setFont(curFont);
}

void TextDisplayer::printCharacterText(const char *text, int8 charNum, int charX) {
	debugC(9, kDebugLevelMain, "TextDisplayer::printCharacterText('%s', %d, %d)", text, charNum, charX);
	uint8 colorTable[] = {0x0F, 0x9, 0x0C9, 0x80, 0x5, 0x81, 0x0E, 0xD8, 0x55, 0x3A, 0x3a};
	int top, left, x1, x2, w, x;
	char *msg;

	uint8 color = colorTable[charNum];
	text = preprocessString(text);
	int lineCount = buildMessageSubstrings(text);
	w = getWidestLineWidth(lineCount);
	x = charX;
	calcWidestLineBounds(x1, x2, w, x);

	for (int i = 0; i < lineCount; ++i) {
		top = i * 10 + _talkMessageY;
		msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
		left = getCenterStringX(msg, x1, x2);
		printText(msg, left, top, color, 0xC, 0);
	}
}
} // end of namespace Kyra
