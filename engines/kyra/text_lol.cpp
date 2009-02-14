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

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/util.h"

namespace Kyra {

TextDisplayer_LoL::TextDisplayer_LoL(LoLEngine *vm, Screen_LoL *screen) : _vm(vm), _screen(screen),
	_scriptParameter(0), _stringLength(0), _animWidth(0), _animColour1(0), _animColour2(0), _animFlag(true),
	_printFlag(false), _lineWidth(0), _numChars(0), _numCharsPrinted(0), _posX(0), _posY(0), _colour1(0), _colour2(0) {
	
	memset(_stringParameters, 0, 15 * sizeof(char*));
	_buffer = new char[600];
	memset(_buffer, 0, 600);

	_out = new char[1024];
	memset(_out, 0, 1024);

	_backupBuffer = new byte[20];
	memset(_backupBuffer, 0, 20);

	_currentLine = new char[85];
	memset(_currentLine, 0, 85);

	_pageBuffer1 = new uint8[0xfa00];
	_pageBuffer2 = new uint8[0xfa00];
}

TextDisplayer_LoL::~TextDisplayer_LoL() {
	delete[] _buffer;
	delete[] _out;
	delete[] _backupBuffer;
	delete[] _currentLine;
	delete[] _pageBuffer1;
	delete[] _pageBuffer2;
}

void TextDisplayer_LoL::setupField(bool mode) {
	if (_vm->textEnabled()) {
		if (mode) {
			_screen->copyRegionToBuffer(3, 0, 0, 320, 200, _pageBuffer1);
			_screen->copyRegion(80, 142, 0, 0, 240, 37, 0, 3, Screen::CR_NO_P_CHECK);
			_screen->copyRegionToBuffer(3, 0, 0, 320, 200, _pageBuffer2);
			_screen->copyBlockToPage(3, 0, 0, 320, 200, _pageBuffer1);
		} else {
			_screen->clearDim(4);
			int cp = _screen->setCurPage(2);
			_screen->copyRegionToBuffer(3, 0, 0, 320, 200, _pageBuffer1);
			_screen->copyBlockToPage(3, 0, 0, 320, 200, _pageBuffer2);
			_screen->copyRegion(80, 142, 0, 0, 240, 37, 3, 2, Screen::CR_NO_P_CHECK);

			for (int i = 177; i > 141; i--) {
				uint32 endTime = _vm->_system->getMillis() + _vm->_tickLength;
				_screen->hideMouse();
				_screen->copyRegion(83, i, 83, i - 1, 235, 3, 0, 0, Screen::CR_NO_P_CHECK);
				_screen->copyRegion(83, i + 1, 83, i + 1, 235, 1, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->updateScreen();
				_screen->showMouse();
				_vm->delayUntil(endTime);
			}

			_screen->copyBlockToPage(3, 0, 0, 320, 200, _pageBuffer1);
			_screen->setCurPage(cp);

			_vm->_updateFlags &= 0xfffd;
		}
	} else {
		if (!mode)
			_screen->clearDim(4);
		_vm->toggleSelectedCharacterFrame(1);
	}
}

void TextDisplayer_LoL::expandField() {
	if (_vm->textEnabled()) {
		_vm->_fadeText = false;
		_vm->_textColourFlag = 0;
		//_vm->toggleGuiUnk(11, 0);
		_screen->clearDim(3);
		_screen->copyRegionToBuffer(3, 0, 0, 320, 200, _pageBuffer1);
		_screen->copyRegion(83, 140, 0, 0, 235, 3, 0, 2, Screen::CR_NO_P_CHECK);

		for (int i = 140; i < 177; i++) {
			uint32 endTime = _vm->_system->getMillis() + _vm->_tickLength;
			_screen->hideMouse();
			_screen->copyRegion(0, 0, 83, i, 235, 3, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			_screen->showMouse();
			_vm->delayUntil(endTime);
		}

		_screen->copyBlockToPage(3, 0, 0, 320, 200, _pageBuffer1);
		_vm->_updateFlags |= 2;

	} else {
		_screen->clearDim(3);
		_vm->toggleSelectedCharacterFrame(0);
	}
}
void TextDisplayer_LoL::setAnimParameters(const char *str, int x, uint8 col1, uint8 col2) {
	static const char defaultStr[] = "<MORE>";

	if (str) {
		_animString = str;
		_animWidth = x;
		_animColour1 = col1;
		_animColour2 = col2;
	} else {
		_animString = defaultStr;
		_animWidth = 7;
		_animColour1 = 0;
		_animColour2 = 0;
	}
}

void TextDisplayer_LoL::playDialogue(int dim, char *str, EMCState *script, int16 *paramList, int16 paramIndex) {
	memcpy(_curPara, _stringParameters, 15 * sizeof(char*));
	//char *cmds = _curPara[0];
	_colour1prot = false;

	if (dim == 3) {
		if (_vm->_updateFlags & 2) {
			_screen->clearDim(4);
			dim = _screen->curDimIndex();
			_colour1 = 254;
			_colour1prot = true;
		} else {
			_screen->clearDim(3);
			dim = _screen->curDimIndex();
			_colour1 = 192;
			_colour1prot = true;
			_screen->copyColour(192, 254);
			//toggleGuiUnk(11, 1);
			_vm->_textColourFlag = 0;
			_vm->_fadeText = false;
		}

	} else {
		_screen->setScreenDim(dim);
		_colour1 = 254;
		_colour1prot = true;
	}

	int cp = _screen->setCurPage(0);
	Screen::FontId of = _screen->setFont(Screen::FID_9_FNT);

	memset(_backupBuffer, 0, 20);

	if (preprocessString(str, script, paramList, paramIndex)) {
		//vsnprintf(_out, 1024, str, cmds);
		_stringLength = strlen(_out);
		displayText(_out);
	} else {
		_stringLength = strlen(str);
		displayText(str);
		displayText(str);
	}

	for (int i = 0; i < 10; i++) {
		if (!_backupBuffer[i << 1])
			break;
		str[_backupBuffer[(i << 1) + 1]] = _backupBuffer[i << 1];
	}

	_screen->setScreenDim(dim);
	_screen->setCurPage(cp);
	_screen->setFont(of);

	_vm->_fadeText = false;
}

void TextDisplayer_LoL::printMessage(uint16 flags, char *str, ...) {
	static uint8 textColours[] = { 0xfe, 0xa2, 0x84, 0x97, 0x9F };
	static uint8 soundEffect[] = { 0x0B, 0x00, 0x2B, 0x1B, 0x00 };
	if (flags & 4)
		flags ^= 4;
	else
		_vm->updatePortraits();

	uint16 col = textColours[flags & 0x7fff];

	int od = _screen->curDimIndex();

	if (_vm->_updateFlags & 2) {
		_screen->setScreenDim(4);
		_screen->clearCurDim();		
	} else {
		_screen->setScreenDim(3);
		_screen->clearCurDim();
		_screen->copyColour(192, col);
		//toggleGuiUnk(11, 1);
	}

	_colour1 = 192;
	_colour1prot = true;

	va_list args;
	va_start(args, str);

	vsnprintf((char*) _buffer, 240, str, args);

	va_end(args);
	
	displayText(str);

	_screen->setScreenDim(od);

	if (!(flags & 0x8000) && soundEffect[flags])
		_vm->sound()->playSoundEffect(soundEffect[flags]);

	_vm->_textColourFlag = flags & 0x7fff;
	_vm->_fadeText = false;
}

bool TextDisplayer_LoL::preprocessString(char *str, EMCState *script, int16 *paramList, int16 paramIndex) {
	int cnt = 0;
	bool res = false;
	char *tmpd = _buffer;
	char **cmds = _curPara;

	for (char *s = str; *s;) {
		if (*s++ != '%')
			continue;

		char pos = *s;
		char para1 = 0;
		bool eos = false;

		switch (pos) {
			case '\0':
				eos = true;
				break;
			case '#':
				para1 = *++s;
				switch (para1) {
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
						break;
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

		char para2 = *s;

		switch (para2) {
			case '\0':
				eos = true;
				break;
			case '0':
				++s;
				break;
			default:
				while(para2 && para2 > 47 && para2 < 58)
					para2 = *++s;
				break;
		}

		if (eos)
			continue;

		char para3 = *++s;

		switch (para3) {
			case 'a':
				_backupBuffer[cnt++] = para3;
				_backupBuffer[cnt++] = (int16) (s - str);
				snprintf(tmpd, 7, "%d", _scriptParameter);
				*cmds++ = tmpd;
				tmpd += strlen(tmpd) + 1;
				res = true;
				*s++ = 's';
				break;

			case 'n':
				_backupBuffer[cnt++] = para3;
				_backupBuffer[cnt++] = (int16) (s - str);				
				*cmds++ = _vm->_characters[script ? script->stack[script->sp + paramIndex] : paramList[paramIndex]].name;
				paramIndex++;
				res = true;
				*s++ = 's';		
				break;

			case 's':
				*cmds++ = _vm->getLangString(script ? script->stack[script->sp + paramIndex] : paramList[paramIndex]);
				paramIndex++;
				res = true;
				s++;	
				break;

			case 'X':
			case 'd':
			case 'u':
			case 'x':
				snprintf(tmpd, 7, "%d", script ? script->stack[script->sp + paramIndex] : paramList[paramIndex]);
				*cmds++ = tmpd;
				tmpd += strlen(tmpd) + 1;
				paramIndex++;
				res = true;
				*s++ = 's';
				break;

			case '\0':
			default:				
				continue;
		}
	}

	return res;
}

void TextDisplayer_LoL::displayText(char *str, ...) {
	_printFlag = false;
	
	_lineWidth = 0;
	_numChars = 0;
	_numCharsPrinted = 0;

	_tempString1 = str;
	_tempString2 = 0;
	
	_currentLine[0] = 0;

	memset(_ctrl, 0, 3);

	if (!_colour1prot)
		_colour1 = _screen->_curDim->unk8;
	_colour2 = _screen->_curDim->unkA;
	_posX = _screen->_curDim->unkC;
	_posY = _screen->_curDim->unkE;
	
	char c = parseCommand();
	
	va_list args;
	va_start(args, str);

	const ScreenDim *sd = _screen->_curDim;

	while (c) {
		char a = tolower(_ctrl[1]);

		if (!_tempString2 && c == '%' ) {
			if (a == 'd') {
				snprintf(_scriptParaString, 11, "%d", va_arg(args, int));
				_tempString2 = _scriptParaString;
			} else if (a == 's') {
				_tempString2 = va_arg(args, char*);
			} else {
				break;
			}
			
			_ctrl[0] = _ctrl[2];
			_ctrl[2] = _ctrl[1] = 0;
			c = parseCommand();
		}

		switch (c - 1) {
			case 0:
				printLine(_currentLine);
				//if (!_dlgAnimCallback)
				//	break;

				portraitAnimation2();
				_numCharsPrinted = 0;
				break;

			case 1:
				printLine(_currentLine);
				_colour2 = parseCommand();
				break;

			case 5:
				printLine(_currentLine);
				_colour1 = parseCommand();
				break;

			case 8:
				//TODO
				break;

			case 11:
				//TODO
				break;

			case 12:
				printLine(_currentLine);
				_screen->_dimLineCount++;
				_posX = 0;
				_posY++;
				break;

			case 18:
				//TODO
				break;

			case 23:
				//TODO
				break;

			case 24:
				//TODO
				break;

			case 26:
				//TODO
				break;

			case 28:
				//TODO
				break;

			default:
				_lineWidth += _screen->getCharWidth(c);
				_currentLine[_numChars++] = c;
				_currentLine[_numChars] = 0;

				if ((_posX + _lineWidth) > (sd->w << 3))
					printLine(_currentLine);
				
				break;
		}		

		c = parseCommand();
	}

	va_end(args);

	if (_numChars)
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
	const ScreenDim *sd = _screen->_curDim;
	
	int fh = (_screen->getFontHeight() + _screen->_charOffset);
	int lines = (sd->h - _screen->_charOffset) / fh;
	
	while (_posY >= lines) {
		if (lines <= _screen->_dimLineCount && _animFlag) {
			_screen->_dimLineCount = 0;
			//if (_dlgAnimCallback) {
				portraitAnimation2();
				_numCharsPrinted = 0;
			//}
		}
		
		int h1 = ((sd->h / fh) - 1) * fh;
		int h2 = sd->h - fh;

		if (h2)
			_screen->copyRegion(sd->sx << 3, sd->sy + fh, sd->sx << 3, sd->sy, sd->w << 3, h2, _screen->_curPage, _screen->_curPage, Screen::CR_NO_P_CHECK);

		_screen->fillRect(sd->sx << 3, sd->sy + h1, (sd->sx + sd->w - 1) << 3, sd->sy + sd->h - 1, _colour2);

		_posY--;
	}

	int x1 = (sd->sx << 3) + _posX;
	int y = sd->sy + fh * _posY;
	int w = sd->w << 3;
	int lw = _lineWidth;
	int s = _numChars;
	char c = 0;

	if ((lw + _posX) > w) {
		if ((lines - 1) <= _screen->_dimLineCount && _animFlag)
			w -= (_animWidth * (_screen->getFontWidth() + _screen->_charWidth));

		w -= _posX;

		int n2 = 0;
		int n1 = s - 1;

		while (n1 > 0) {
			//cut off line after last space
			c = str[n1];
			lw -= _screen->getCharWidth(c);
			
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
			if (_posX && !_printFlag) {
				s = lw = 0;
				_printFlag = true;
			} else {
				s = n2;
			}
		}
	}

	c = str[s];
	str[s] = 0;
	
	_screen->printText(str, x1, y, _colour1, _colour2);
	_posX += lw;
	_numCharsPrinted += strlen(str);
	
	str[s] = c;

	if (c == ' ')
		s++;

	if (str[s] == ' ')
		s++;

	strcpy(str, &str[s]);
	_numChars = strlen(str);
	_lineWidth = _screen->getTextWidth(str);

	if (!_numChars && _posX < (sd->w << 3))
		return;

	_posX = 0;
	_posY++;
	_screen->_dimLineCount++;

	printLine(str);
}

/*void TextDisplayer_LoL::portraitAnimation1(const char *str, uint16 lineWidth, uint8 col1, uint8 col2, uint16 numCharsPrinted) {
	
}*/

void TextDisplayer_LoL::portraitAnimation2() {
	// TODO
}

} // end of namespace Kyra
