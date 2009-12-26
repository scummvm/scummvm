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

#ifdef ENABLE_LOL

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/timer.h"
#include "kyra/util.h"

namespace Kyra {

TextDisplayer_LoL::TextDisplayer_LoL(LoLEngine *vm, Screen_LoL *screen) : _vm(vm), _screen(screen),
	_scriptTextParameter(0), _lineCount(0),	_printFlag(false), _lineWidth(0), _numCharsTotal(0),
	_numCharsLeft(0), _numCharsPrinted(0), _sjisLineBreakFlag(false) {

	memset(_stringParameters, 0, 15 * sizeof(char *));
	_buffer = new char[600];
	memset(_buffer, 0, 600);

	_dialogueBuffer = new char[1024];
	memset(_dialogueBuffer, 0, 1024);

	_currentLine = new char[85];
	memset(_currentLine, 0, 85);

	for (int i = 0; i < 14; i++){
		const ScreenDim *d = _screen->getScreenDim(i);
		_textDimData[i].color1 = d->unk8;
		_textDimData[i].color2 = d->unkA;
		_textDimData[i].line = d->unkC;
		_textDimData[i].column = d->unkE;
	}
}

TextDisplayer_LoL::~TextDisplayer_LoL() {
	delete[] _buffer;
	delete[] _dialogueBuffer;
	delete[] _currentLine;
}

void TextDisplayer_LoL::setupField(bool mode) {
	if (_vm->textEnabled()) {

		int y = 142;
		int h = 37;
		int stepY = 3;
		int stepH = 1;

		if (_vm->gameFlags().use16ColorMode) {
			y = 140;
			h = 39;
			stepY = 4;
			stepH = 2;
		}

		if (mode) {
			_screen->copyRegionToBuffer(3, 0, 0, 320, 40, _vm->_pageBuffer1);
			_screen->copyRegion(80, y, 0, 0, 240, h, 0, 3, Screen::CR_NO_P_CHECK);
			_screen->copyRegionToBuffer(3, 0, 0, 320, 40, _vm->_pageBuffer2);
			_screen->copyBlockToPage(3, 0, 0, 320, 40, _vm->_pageBuffer1);
		} else {
			_screen->setScreenDim(clearDim(4));
			int cp = _screen->setCurPage(2);
			_screen->copyRegionToBuffer(3, 0, 0, 320, 40, _vm->_pageBuffer1);
			_screen->copyBlockToPage(3, 0, 0, 320, 40, _vm->_pageBuffer2);
			_screen->copyRegion(0, 0, 80, y, 240, h, 3, _screen->_curPage, Screen::CR_NO_P_CHECK);

			for (int i = 177; i > 141; i--) {
				uint32 endTime = _vm->_system->getMillis() + _vm->_tickLength;
				_screen->copyRegion(83, i - stepH + 1, 83, i - stepH, 235, stepY, 0, 0, Screen::CR_NO_P_CHECK);
				_screen->copyRegion(83, i + 1, 83, i + 1, 235, 1, 2, 0, Screen::CR_NO_P_CHECK);
				_vm->updateInput();
				_screen->updateScreen();
				_vm->delayUntil(endTime);
			}

			_screen->copyBlockToPage(3, 0, 0, 320, 200, _vm->_pageBuffer1);
			_screen->setCurPage(cp);

			_vm->_updateFlags &= 0xfffd;
		}
	} else {
		if (!mode)
			_screen->setScreenDim(clearDim(4));
		_vm->toggleSelectedCharacterFrame(1);
	}
}

void TextDisplayer_LoL::expandField() {
	uint8 *tmp = _vm->_pageBuffer1 + 13000;

	if (_vm->textEnabled()) {
		_vm->_fadeText = false;
		_vm->_textColorFlag = 0;
		_vm->_timer->disable(11);
		_screen->setScreenDim(clearDim(3));
		_screen->copyRegionToBuffer(3, 0, 0, 320, 10, tmp);

		int y = 140;
		int h = 3;
		int stepH = 0;

		if (_vm->gameFlags().use16ColorMode) {
			y = 139;
			h = 4;
			stepH = 1;
		}
		
		_screen->copyRegion(83, y, 0, 0, 235, h, 0, 2, Screen::CR_NO_P_CHECK);		

		for (int i = 140; i < 177; i++) {
			uint32 endTime = _vm->_system->getMillis() + _vm->_tickLength;
			_screen->copyRegion(0, 0, 83, i - stepH, 235, h, 2, 0, Screen::CR_NO_P_CHECK);
			_vm->updateInput();
			_screen->updateScreen();
			_vm->delayUntil(endTime);
		}

		_screen->copyBlockToPage(3, 0, 0, 320, 10, tmp);
		_vm->_updateFlags |= 2;

	} else {
		clearDim(3);
		_vm->toggleSelectedCharacterFrame(0);
	}
}

int TextDisplayer_LoL::clearDim(int dim) {
	int res = _screen->curDimIndex();
	_screen->setScreenDim(dim);
	_textDimData[dim].color1 = _screen->_curDim->unk8;
	_textDimData[dim].color2 = _screen->_curDim->unkA;
	clearCurDim();
	return res;
}

void TextDisplayer_LoL::resetDimTextPositions(int dim) {
	_textDimData[dim].column = 0;
	_textDimData[dim].line = 0;
}

void TextDisplayer_LoL::printDialogueText(int dim, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex) {
	int oldDim = 0;

	const bool isPc98 = (_vm->gameFlags().platform == Common::kPlatformPC98);

	if (dim == 3) {
		if (_vm->_updateFlags & 2) {
			oldDim = clearDim(4);
			_textDimData[4].color1 = isPc98 ? 0x33 : 254;
			_textDimData[4].color2 = _screen->_curDim->unkA;
		} else {
			oldDim = clearDim(3);
			_textDimData[3].color1 = isPc98 ? 0x33 : 192;
			_textDimData[3].color2 = _screen->_curDim->unkA;
			if (!isPc98)
				_screen->copyColor(192, 254);
			_vm->enableTimer(11);
			_vm->_textColorFlag = 0;
			_vm->_fadeText = false;
		}
	} else {
		oldDim = _screen->curDimIndex();
		_screen->setScreenDim(dim);
		_textDimData[dim].color1 = isPc98 ? 0x33 : 254;
		_textDimData[dim].color2 = _screen->_curDim->unkA;
	}

	int cp = _screen->setCurPage(0);
	Screen::FontId of = _screen->setFont(_vm->gameFlags().use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_9_FNT);

	preprocessString(str, script, paramList, paramIndex);
	_numCharsTotal = strlen(_dialogueBuffer);
	displayText(_dialogueBuffer);

	_screen->setScreenDim(oldDim);
	_screen->setCurPage(cp);
	_screen->setFont(of);

	_vm->_fadeText = false;
}

void TextDisplayer_LoL::printMessage(uint16 type, const char *str, ...) {
	static uint8 textColors256[] = { 0xfe, 0xa2, 0x84, 0x97, 0x9F };
	static uint8 textColors16[] = { 0x33, 0xaa, 0x88, 0x55, 0x99 };
	static uint8 soundEffect[] = { 0x0B, 0x00, 0x2B, 0x1B, 0x00 };
	const uint8 *textColors = _vm->gameFlags().use16ColorMode ? textColors16 : textColors256;
	if (type & 4)
		type ^= 4;
	else
		_vm->stopPortraitSpeechAnim();

	uint16 col = textColors[type & 0x7fff];

	int od = _screen->curDimIndex();

	if (_vm->_updateFlags & 2) {
		clearDim(4);
		_textDimData[4].color1 = col;
	} else {
		clearDim(3);
		if (_vm->gameFlags().use16ColorMode) {
			_textDimData[3].color1 = col;
		} else {
			_screen->copyColor(192, col);
			_textDimData[3].color1 = 192;
		}
		_vm->enableTimer(11);
	}

	va_list args;
	va_start(args, str);

	vsnprintf((char *)_buffer, 240, str, args);

	va_end(args);

	displayText(_buffer);

	_screen->setScreenDim(od);

	if (!(type & 0x8000)) {
		if (soundEffect[type])
			_vm->sound()->playSoundEffect(soundEffect[type]);
	}

	_vm->_textColorFlag = type & 0x7fff;
	_vm->_fadeText = false;
}

void TextDisplayer_LoL::preprocessString(char *str, EMCState *script, const uint16 *paramList, int16 paramIndex) {
	char *dst = _dialogueBuffer;
	const bool isPc98 = (_vm->gameFlags().platform == Common::kPlatformPC98);

	for (char *s = str; *s;) {
		if (isPc98) {
			uint8 c = *s;
			if (c >= 0xE0 || (c > 0x80 && c < 0xA0)) {
				*dst++ = *s++;
				*dst++ = *s++;
				continue;
			}
		}

		if (*s != '%') {
			*dst++ = *s++;
			continue;
		}

		char para = *++s;
		bool eos = false;

		switch (para) {
		case '\0':
			eos = true;
			break;
		case '#':
			para = *++s;
			switch (para) {
			case 'E':
			case 'G':
			case 'X':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 's':
			case 'u':
			case 'x':
				break;
			default:
				eos = true;
			}
			break;
		case ' ':
		case '+':
		case '-':
			++s;
		default:
			break;
		}

		if (eos)
			continue;

		para = *s;

		switch (para) {
		case '\0':
			eos = true;
			break;
		case '0':
			++s;
			break;
		default:
			while (para && para > 47 && para < 58)
				para = *++s;
			break;
		}
		if (eos)
			continue;

		para = *s++;

		switch (para) {
		case 'a':
			snprintf(dst, 7, "%d", _scriptTextParameter);
			dst += strlen(dst);
			break;

		case 'n':
			strcpy(dst, _vm->_characters[script ? script->stack[script->sp + paramIndex] : paramList[paramIndex]].name);
			dst += strlen(dst);
			break;

		case 's':
			strcpy(dst, _vm->getLangString(script ? script->stack[script->sp + paramIndex] : paramList[paramIndex]));
			dst += strlen(dst);
			break;

		case 'X':
		case 'd':
		case 'u':
		case 'x':
			snprintf(dst, 7, "%d", script ? script->stack[script->sp + paramIndex] : paramList[paramIndex]);
			dst += strlen(dst);
			break;

		case '\0':
		default:
			continue;
		}
	}
	*dst = 0;
}

void TextDisplayer_LoL::displayText(char *str, ...) {
	const bool isPc98 = (_vm->gameFlags().platform == Common::kPlatformPC98);
	
	_printFlag = false;

	_lineWidth = 0;
	_numCharsLeft = 0;
	_numCharsPrinted = 0;

	_tempString1 = str;
	_tempString2 = 0;

	_currentLine[0] = 0;

	memset(_ctrl, 0, 3);

	char c = parseCommand();

	va_list args;
	va_start(args, str);

	const ScreenDim *sd = _screen->_curDim;
	int sdx = _screen->curDimIndex();

	bool pc98PrintFlag = (isPc98 && (sdx == 3 || sdx == 4 || sdx == 5 || sdx == 15)) ? true : false;	
	uint16 charsPerLine = (sd->w << 3) / (_screen->getFontWidth() + _screen->_charWidth);

	while (c) {
		char a = tolower(_ctrl[1]);

		if (!_tempString2 && c == '%') {
			if (a == 'd') {
				snprintf(_scriptParaString, 11, "%d", va_arg(args, int));
				_tempString2 = _scriptParaString;
			} else if (a == 's') {
				_tempString2 = va_arg(args, char *);
			} else {
				break;
			}

			_ctrl[0] = _ctrl[2];
			_ctrl[2] = _ctrl[1] = 0;
			c = parseCommand();
		}

		if (isPc98) {
			uint8 cu = (uint8) c;
			if (cu >= 0xE0 || (cu > 0x80 && cu < 0xA0)) {
				_currentLine[_numCharsLeft++] = c;
				_currentLine[_numCharsLeft++] = parseCommand();
				_currentLine[_numCharsLeft] = '\0';
				_lineWidth += 8;
				if ((_textDimData[sdx].column + _lineWidth) > (sd->w << 3))
					printLine(_currentLine);
				c = parseCommand();
				continue;
			}
		}

		uint16 dv = _textDimData[sdx].column / (_screen->getFontWidth() + _screen->_charWidth);

		switch (c - 1) {
		case 0:
			printLine(_currentLine);
			textPageBreak();
			_numCharsPrinted = 0;
			break;

		case 1:
			printLine(_currentLine);
			_textDimData[sdx].color2 = parseCommand();
			break;

		case 5:
			printLine(_currentLine);
			_textDimData[sdx].color1 = parseCommand();
			break;

		case 8:
			printLine(_currentLine);
			dv = _textDimData[sdx].column / (_screen->getFontWidth() + _screen->_charWidth);
			dv = ((dv + 8) & 0xfff8) - 1;
			if (dv >= charsPerLine)
				dv = 0;
			_textDimData[sdx].column = (_screen->getFontWidth() + _screen->_charWidth) * dv;
			break;

		case 11:
			// TODO (UNUSED)
			break;

		case 12:
			if (isPc98)
				_sjisLineBreakFlag = true;
			printLine(_currentLine);
			_sjisLineBreakFlag = false;
			_lineCount++;
			_textDimData[sdx].column = 0;
			_textDimData[sdx].line++;
			break;

		case 18:
			// TODO (UNUSED)
			break;

		case 23:
			// TODO (UNUSED)
			break;

		case 24:
			// TODO (UNUSED)
			break;

		case 26:
			// TODO (UNUSED)
			break;

		case 28:
			// TODO (UNUSED)
			break;

		default:
			_lineWidth += (pc98PrintFlag ? 4 : _screen->getCharWidth((uint8)c));
			_currentLine[_numCharsLeft++] = c;
			_currentLine[_numCharsLeft] = 0;

			if ((_textDimData[sdx].column + _lineWidth) > (sd->w << 3))
				printLine(_currentLine);
		}

		c = parseCommand();
	}

	va_end(args);

	if (_numCharsLeft)
		printLine(_currentLine);
}

char TextDisplayer_LoL::parseCommand() {
	if (!_ctrl[1])
		readNextPara();

	char res = _ctrl[1];
	_ctrl[1] = _ctrl[2];
	_ctrl[2] = 0;

	if (!_ctrl[1])
		readNextPara();

	return res;
}

void TextDisplayer_LoL::readNextPara() {
	char d = 0;

	if (_tempString2) {
		if (*_tempString2) {
			d = *_tempString2++;
		} else {
			_tempString2 = 0;
			d = _ctrl[0];
		}
	}

	if (!d && _tempString1) {
		if (*_tempString1)
			d = *_tempString1++;
		else
			_tempString1 = 0;
	}

	_ctrl[1] = d;
	_ctrl[2] = 0;
}

void TextDisplayer_LoL::printLine(char *str) {
	const bool isPc98 = (_vm->gameFlags().platform == Common::kPlatformPC98);
	const ScreenDim *sd = _screen->_curDim;
	int sdx = _screen->curDimIndex();
	bool pc98PrintFlag = (isPc98 && (sdx == 3 || sdx == 4 || sdx == 5 || sdx == 15)) ? true : false;

	int fh = (_screen->_currentFont == Screen::FID_SJIS_FNT) ? 9 : (_screen->getFontHeight() + _screen->_charOffset);
	int lines = (sd->h - _screen->_charOffset) / fh;

	while (_textDimData[sdx].line >= lines) {
		if (lines <= _lineCount) {
			_lineCount = 0;
			textPageBreak();
			_numCharsPrinted = 0;
		}

		int h1 = ((sd->h / fh) - 1) * fh;
		int h2 = sd->h - fh;

		if (h2)
			_screen->copyRegion(sd->sx << 3, sd->sy + fh, sd->sx << 3, sd->sy, sd->w << 3, h2, _screen->_curPage, _screen->_curPage, Screen::CR_NO_P_CHECK);

		_screen->fillRect(sd->sx << 3, sd->sy + h1, (sd->sx + sd->w - 1) << 3, sd->sy + sd->h - 1, _textDimData[sdx].color2);
		if (_textDimData[sdx].line)
			_textDimData[sdx].line--;
	}

	int x1 = (sd->sx << 3) + _textDimData[sdx].column;
	int y = sd->sy + (pc98PrintFlag ? (_textDimData[sdx].line << 3) : (fh * _textDimData[sdx].line));
	int w = sd->w << 3;
	int lw = _lineWidth;
	int s = _numCharsLeft;
	char c = 0;

	if (pc98PrintFlag) {
		bool ct = true;

		if ((lw + _textDimData[sdx].column) > w) {
			if ((lines - 1) <= _lineCount)
				// cut off line to leave space for "MORE" button
				w -= 80;
		} else {
			if (!_sjisLineBreakFlag || (_lineCount + 1 < lines - 1))
				ct = false;
			else
				// cut off line to leave space for "MORE" button
				w -= 80;
		}

		if (ct) {
			w -= _textDimData[sdx].column;

			int n2 = 0;
			int n1 = (w / 4) - 1;			

			do {
				c = str[n2];
				uint8 cu = (uint8) c;
				if (cu >= 0xE0 || (cu > 0x80 && cu < 0xA0))
					n2++;
				n2++;
			} while (n2 < n1 && n2 < s);
			s = n2;
		}
	} else {
		if ((lw + _textDimData[sdx].column) > w) {
			if ((lines - 1) <= _lineCount)
				// cut off line to leave space for "MORE" button
				w -= (10 * (_screen->getFontWidth() + _screen->_charWidth));

			w -= _textDimData[sdx].column;

			int n2 = 0;
			int n1 = s - 1;

			while (n1 > 0) {
				//cut off line after last space
				c = str[n1];
				
				lw -= _screen->getCharWidth((uint8)c);

				if (!n2 && lw <= w)
					n2 = n1;

				if (n2 && c == ' ') {
					s = n1;
					_printFlag = false;
					break;
				}
				n1--;								
			}

			if (!n1) {
				if (_textDimData[sdx].column && !_printFlag) {
					s = lw = 0;
					_printFlag = true;
				} else {
					s = n2;
				}
			}
		}
	}

	c = str[s];
	str[s] = 0;

	uint8 col = _textDimData[sdx].color1;
	if (isPc98 && (sdx == 2 || sdx == 3 || sdx == 4 || sdx == 5 || sdx == 15)) {		
		switch (_textDimData[sdx].color1) {
		case 0x88:
			col = 0x41;
			break;
		case 0x55:
			col = 0x81;
			break;
		case 0xaa:
			col = 0x21;
			break;
		case 0x99:
			col = 0xa1;
			break;
		case 0x33:
			col = 0xe1;
			break;
		case 0x18:
			col = 0x61;
			break;
		default:
			col = 1;
			break;
		}
		_screen->printText(str, x1 & ~3, (y + 8) & ~7, col, 0);
	} else {
		_screen->printText(str, x1, y, col, _textDimData[sdx].color2);
	}

	_textDimData[sdx].column += lw;
	_numCharsPrinted += strlen(str);

	str[s] = c;

	if (c == ' ')
		s++;

	if (str[s] == ' ')
		s++;

	uint32 len = strlen(&str[s]);
	for (uint32 i = 0; i < len; i++)
		str[i] = str[s + i];
	str[len] = 0;

	_numCharsLeft = strlen(str);
	_lineWidth = pc98PrintFlag ? (_numCharsLeft << 2) : _screen->getTextWidth(str);

	if (!_numCharsLeft && _textDimData[sdx].column < (sd->w << 3))
		return;

	_textDimData[sdx].column = 0;
	_textDimData[sdx].line++;
	_lineCount++;

	printLine(str);
}

void TextDisplayer_LoL::textPageBreak() {
	int cp = _screen->setCurPage(0);
	Screen::FontId cf = _screen->setFont(_vm->gameFlags().use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_6_FNT);

	_vm->_timer->pauseSingleTimer(11, true);

	_vm->_fadeText = false;
	int resetPortraitAfterSpeechAnim = 0;
	int updatePortraitSpeechAnimDuration = 0;

	if (_vm->_updateCharNum != -1)  {
		resetPortraitAfterSpeechAnim = _vm->_resetPortraitAfterSpeechAnim;
		_vm->_resetPortraitAfterSpeechAnim = 0;
		updatePortraitSpeechAnimDuration = _vm->_updatePortraitSpeechAnimDuration;
		if (_vm->_updatePortraitSpeechAnimDuration > 36)
			_vm->_updatePortraitSpeechAnimDuration = 36;
	}

	uint32 speechPartTime = 0;
	if (_vm->speechEnabled() && _vm->_activeVoiceFileTotalTime && _numCharsTotal)
		speechPartTime = _vm->_system->getMillis() + ((_numCharsPrinted * _vm->_activeVoiceFileTotalTime) / _numCharsTotal);

	const ScreenDim *dim = _screen->getScreenDim(_screen->curDimIndex());

	int x = ((dim->sx + dim->w) << 3) - 77;
	int y = 0;

	if (_vm->_needSceneRestore && (_vm->_updateFlags & 2)) {
		if (_vm->_currentControlMode || !(_vm->_updateFlags & 2)) {
			y = dim->sy + dim->h - 5;
		} else {
			x += 6;
			y = dim->sy + dim->h - 2;
		}
	} else {
		y = dim->sy + dim->h - 10;
	}

	char *txt = _vm->getLangString(0x4073);
	if (_vm->gameFlags().use16ColorMode) {
		_vm->gui_drawBox(x + 8, (y & ~7) - 1, 66, 10, 0xee, 0xcc, -1);	
		_vm->_screen->printText(txt, (x + 37 - (strlen(txt) << 1) + 4) & ~3, (y + 2) & ~7, 0xc1, 0);
	} else {
		_vm->gui_drawBox(x, y, 74, 9, 136, 251, -1);	
		_vm->_screen->printText(txt, x + 37 - (_vm->_screen->getTextWidth(txt) >> 1), y + 2, 144, 0);
	}	

	_vm->removeInputTop();

	bool loop = true;
	bool target = false;

	do {
		int inputFlag = _vm->checkInput(0, false) & 0xFF;
		_vm->removeInputTop();

		while (!inputFlag) {
			_vm->update();

			if (_vm->speechEnabled()) {
				if (((_vm->_system->getMillis() > speechPartTime) || (_vm->snd_updateCharacterSpeech() != 2)) && speechPartTime) {
					loop = false;
					inputFlag = _vm->_keyMap[Common::KEYCODE_RETURN];
					break;
				}
			}

			inputFlag = _vm->checkInput(0, false) & 0xFF;
			_vm->removeInputTop();
		}

		_vm->gui_notifyButtonListChanged();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN]) {
			loop = false;
		} else if (inputFlag == 199 || inputFlag == 201) {
			if (_vm->posWithinRect(_vm->_mouseX, _vm->_mouseY, x, y, x + 74, y + 9))
				target = true;
		} else if (inputFlag == 200 || inputFlag == 202) {
			if (target)
				loop = false;
		}
	} while (loop);


	if (_vm->gameFlags().use16ColorMode) 
		_screen->fillRect(x + 8, y, x + 57, y + 9, _textDimData[_screen->curDimIndex()].color2);
	else
		_screen->fillRect(x, y, x + 73, y + 8, _textDimData[_screen->curDimIndex()].color2);

	clearCurDim();

	_vm->_timer->pauseSingleTimer(11, false);

	if (_vm->_updateCharNum != -1) {
		_vm->_resetPortraitAfterSpeechAnim = resetPortraitAfterSpeechAnim;
		if (updatePortraitSpeechAnimDuration > 36)
			updatePortraitSpeechAnimDuration -= 36;
		else
			updatePortraitSpeechAnimDuration >>= 1;

		_vm->_updatePortraitSpeechAnimDuration = updatePortraitSpeechAnimDuration;
	}

	_screen->setFont(cf);
	_screen->setCurPage(cp);
	_vm->removeInputTop();
}

void TextDisplayer_LoL::clearCurDim() {
	int d = _screen->curDimIndex();
	const ScreenDim *tmp = _screen->getScreenDim(d);
	if (_vm->gameFlags().use16ColorMode) {
		_screen->fillRect(tmp->sx << 3, tmp->sy, ((tmp->sx + tmp->w) << 3) - 2, (tmp->sy + tmp->h) - 2, _textDimData[d].color2);
	} else
		_screen->fillRect(tmp->sx << 3, tmp->sy, ((tmp->sx + tmp->w) << 3) - 1, (tmp->sy + tmp->h) - 1, _textDimData[d].color2);
	
	_lineCount = 0;
	_textDimData[d].column = _textDimData[d].line = 0;
}

} // End of namespace Kyra

#endif // ENABLE_LOL

