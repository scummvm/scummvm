/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/engine/kyra_rpg.h"
#include "kyra/engine/timer.h"

#include "common/system.h"

namespace Kyra {

enum {
	kEoBTextBufferSize = 2560
};

TextDisplayer_rpg::TextDisplayer_rpg(KyraRpgEngine *engine, Screen *scr) : _vm(engine), _screen(scr),
	_lineCount(0), _printFlag(false), _lineWidth(0), _numCharsTotal(0), _allowPageBreak(true), _dimCount(scr ? scr->screenDimTableCount() : 0),
	_numCharsLeft(0), _numCharsPrinted(0), _twoByteLineBreakFlag(false), _waitButtonMode(1),
	_pc98TextMode(engine->gameFlags().use16ColorMode && engine->game() == GI_LOL),
	_waitButtonFont(Screen::FID_6_FNT), _isChinese(_vm->gameFlags().lang == Common::Language::ZH_TWN || _vm->gameFlags().lang == Common::Language::ZH_CHN) {

	_dialogueBuffer = new char[kEoBTextBufferSize]();
	_currentLine = new char[85]();

	if (_pc98TextMode)
		_waitButtonFont = Screen::FID_SJIS_TEXTMODE_FNT;
	else if ((_vm->game() == GI_EOB2 && _vm->gameFlags().platform == Common::kPlatformFMTowns))
		_waitButtonFont = Screen::FID_8_FNT;
	else if (_vm->gameFlags().platform == Common::kPlatformPC98)
		_waitButtonFont = Screen::FID_SJIS_FNT;
	else if ((_vm->game() == GI_LOL && _vm->gameFlags().lang == Common::Language::ZH_TWN))
		_waitButtonFont = Screen::FID_CHINESE_FNT;

	_textDimData = new TextDimData[_dimCount];
	memset(_textDimData, 0, sizeof(TextDimData) * _dimCount);

	applySetting(-1, kNoHalfWidthLineEnd, ((_vm->gameFlags().lang == Common::JA_JPN && _vm->game() == GI_EOB1) || (_vm->gameFlags().lang == Common::Language::ZH_TWN && _vm->game() == GI_EOB2)) ? 1 : 0);

	for (int i = 0; i < 256; ++i)
		_colorMap[i] = i;

	for (int i = 0; i < _dimCount; ++i) {
		const ScreenDim *d = _screen->getScreenDim(i);
		_textDimData[i].color1= d->col1;
		_textDimData[i].color2 = d->col2;
		_textDimData[i].line = d->line;
		_textDimData[i].column = d->column;
	}

	_table1 = new char[128]();
	_table2 = new char[16]();

	_tempString1 = _tempString2 = 0;
	_ctrl[0] = _ctrl[1] = _ctrl[2] = '\0';
	_waitButtonSpace = 0;
}

TextDisplayer_rpg::~TextDisplayer_rpg() {
	setColorMapping(-1, 0 ,0);
	delete[] _dialogueBuffer;
	delete[] _currentLine;
	delete[] _textDimData;
	delete[] _table1;
	delete[] _table2;
}

void TextDisplayer_rpg::setupField(int dim, bool mode) {
	setPageBreakFlag();

	_textDimData[dim].color2 = _vm->guiSettings()->colors.fill;
	_screen->setScreenDim(dim);

	if (mode) {
		_screen->set16bitShadingLevel(4);
		clearCurDim();
		_screen->set16bitShadingLevel(0);
	} else {
		resetDimTextPositions(dim);
	}

	_vm->_dialogueFieldAmiga = false;
}

void TextDisplayer_rpg::resetDimTextPositions(int dim) {
	_textDimData[dim].column = 0;
	_textDimData[dim].line = 0;
}

void TextDisplayer_rpg::resetPageBreakString() {
	if (_vm->_moreStrings)
		_pageBreakString = _vm->_moreStrings[0];
}

void TextDisplayer_rpg::setPageBreakFlag() {
	_allowPageBreak = true;
	_lineCount = 0;
}

void TextDisplayer_rpg::removePageBreakFlag() {
	_allowPageBreak = false;
}

void TextDisplayer_rpg::setColorMapping(int sd, uint8 from, uint8 to) {
	if (sd < -1 || sd >= _dimCount)
		error("TextDisplayer_rpg::mapColor(): arg out of range");

	if (sd == -1) {
		for (int i = 0; i < _dimCount; ++i) {
			delete[] _textDimData[i].colorMap;
			_textDimData[i].colorMap = nullptr;
		}
		_colorMap[from] = to;
	} else {
		if (_textDimData[sd].colorMap == nullptr) {
			_textDimData[sd].colorMap = new uint8[256];
			for (int i = 0; i < 256; ++i)
				_textDimData[sd].colorMap[i] = i;
		}
		_textDimData[sd].colorMap[from] = to;
	}
}

void TextDisplayer_rpg::displayText(char *str, ...) {
	convertString(str);
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

	bool sjisTextMode = (_pc98TextMode && (sdx == 3 || sdx == 4 || sdx == 5 || sdx == 15)) ? true : false;
	Screen::FontId of = (_vm->game() == GI_EOB2 && _vm->gameFlags().platform == Common::kPlatformFMTowns) ? _screen->setFont(Screen::FID_8_FNT) : _screen->_currentFont;

	uint16 charsPerLine = (sd->w << 3) / (_screen->getFontWidth() + _screen->_charSpacing);

	while (c) {
		char a = tolower((unsigned char)_ctrl[1]);

		if (!_tempString2 && c == '%') {
			if (a == 'd') {
				_scriptParaString = Common::String::format("%d", va_arg(args, int));
				_tempString2 = _scriptParaString.c_str();
			} else if (a == 's') {
				_tempString2 = va_arg(args, const char*);
			} else {
				break;
			}

			_ctrl[0] = _ctrl[2];
			_ctrl[2] = _ctrl[1] = 0;
			c = parseCommand();
		}

		if (isTwoByteChar(c)) {
			char next = parseCommand();
			int cw = _screen->getCharWidth((uint8)c | (uint8)next << 8) + _textDimData[sdx].charSpacing;
			if (_textDimData[sdx].column + _lineWidth + cw > (sd->w << 3))
				printLine(_currentLine);

			_currentLine[_numCharsLeft++] = c;
			_currentLine[_numCharsLeft++] = next;
			_currentLine[_numCharsLeft] = '\0';

			_lineWidth += cw;
			if (_textDimData[sdx].noHalfWidthLineEnd && (_textDimData[sdx].column + _lineWidth + cw >= (sd->w << 3)))
				printLine(_currentLine);

			c = parseCommand();
			continue;
		}

		uint16 dv = _textDimData[sdx].column / (_screen->getFontWidth() + _screen->_charSpacing);

		switch (c - 1) {
		case 0:
			printLine(_currentLine);
			_screen->updateScreen();
			textPageBreak();
			_numCharsPrinted = 0;
			break;

		case 1:
			printLine(_currentLine);
			_textDimData[sdx].color2 = parseCommand();
			break;

		case 5:
			printLine(_currentLine);
			_textDimData[sdx].color1= parseCommand();
			break;

		case 8:
			printLine(_currentLine);
			dv = _textDimData[sdx].column / (_screen->getFontWidth() + _screen->_charSpacing);
			dv = ((dv + 8) & 0xFFF8) - 1;
			if (dv >= charsPerLine)
				dv = 0;
			_textDimData[sdx].column = (_screen->getFontWidth() + _screen->_charSpacing) * dv;
			break;

		case 12:
			if (sjisTextMode)
				_twoByteLineBreakFlag = true;
			printLine(_currentLine);
			_twoByteLineBreakFlag = false;
			//_lineWidth = 0;
			_lineCount++;
			_textDimData[sdx].column = 0;
			_textDimData[sdx].line++;
			break;

		case 11: case 18: case 23:
		case 24: case 26: case 28:
			// These are at the time of writing this comment not known to be
			// used. In case there is some use of them in some odd version
			// we display this warning here.
			warning("TextDisplayer_rpg::displayText: Triggered stub function %d", c - 1);
			break;

		default:
			if (_vm->game() == GI_EOB1 || _vm->game() == GI_LOL || (unsigned char)c > 30) {
				int cw = _screen->getCharWidth((uint8)c) + _textDimData[sdx].charSpacing;
				_lineWidth += cw;

				_currentLine[_numCharsLeft++] = c;
				_currentLine[_numCharsLeft] = 0;

				if ((_textDimData[sdx].column + _lineWidth) > (sd->w << 3))
					printLine(_currentLine);
			}
		}

		c = parseCommand();
	}

	va_end(args);

	if (_numCharsLeft)
		printLine(_currentLine);

	_screen->setFont(of);
	_screen->updateScreen();
}

char TextDisplayer_rpg::parseCommand() {
	if (!_ctrl[1])
		readNextPara();

	char res = _ctrl[1];
	_ctrl[1] = _ctrl[2];
	_ctrl[2] = 0;

	if (!_ctrl[1])
		readNextPara();

	return res;
}

void TextDisplayer_rpg::readNextPara() {
	char c = 0;
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

	// This seems to be some sort of character conversion mechanism. The original doesn't make any use of it, however.
	// All necessary conversions take place somewhere else. This code actually causes issues if the character conversions
	// don't take place before calling displayText(). So we disable it for now. If some (not yet supported) localized
	// versions depend on this code we'll have to look at this again.
#if 0
	if ((_vm->game() != GI_LOL) && (d & 0x80)) {
		d &= 0x7F;
		c = d & 7;
		d = (d & 0x78) >> 3;
		uint8 l = d;
		c = _table1[(l << 3) + c];
		d = _table2[l];
	}
#endif

	_ctrl[1] = d;
	_ctrl[2] = c;
}

void TextDisplayer_rpg::printLine(char *str) {
	const ScreenDim *sd = _screen->_curDim;
	int sdx = _screen->curDimIndex();
	bool sjisTextMode = _pc98TextMode && (sdx == 3 || sdx == 4 || sdx == 5 || sdx == 15) ? true : false;

	int fh = _screen->getFontHeight() + _screen->_lineSpacing + _textDimData[sdx].lineSpacing;
	int lines = (sd->h - _screen->_lineSpacing) / fh;

	// Another hack for Chinese EOB II...The original prints text at the very bottom of the text field,
	// even if there is a good risk of printing text over the dialogue buttons.
	if (_isChinese && _allowPageBreak)
		++lines;

	while (_textDimData[sdx].line >= lines) {
		if ((lines - _waitButtonSpace) <= _lineCount && _allowPageBreak) {
			_lineCount = 0;
			_screen->updateScreen();
			textPageBreak();
			_numCharsPrinted = 0;
		}

		int h1 = ((sd->h / fh) - 1) * fh;
		int h2 = sd->h - fh;
		int wOffs = (_textDimData[sdx].shadowColor && sd->sx > 0) ? 1 : 0;

		if (h2)
			_screen->copyRegion((sd->sx << 3) - wOffs, sd->sy + fh, (sd->sx << 3) - wOffs, sd->sy, (sd->w << 3) + wOffs, h2, _screen->_curPage, _screen->_curPage, Screen::CR_NO_P_CHECK);

		// HACK: In Chinese EOBII some characters overdraw the valid boundaries by one pixel
		// (at least the ',' does). So, the original redraws the border here. We do the same
		// since for now I don't have any good idea how to do this less ugly...
		if (_isChinese && _vm->_flags.gameID == GI_EOB2 && sdx == 7)
			_screen->drawBox(3, 170, 290, 199, _vm->guiSettings()->colors.fill);

		_screen->set16bitShadingLevel(4);
		_screen->fillRect((sd->sx << 3) - wOffs, sd->sy + h1, ((sd->sx + sd->w) << 3) - 1, sd->sy + sd->h - 1, remapColor(sdx, _textDimData[sdx].color2));
		_screen->set16bitShadingLevel(0);

		if (_textDimData[sdx].line)
			_textDimData[sdx].line--;
	}

	int x1 = (sd->sx << 3) + _textDimData[sdx].column;
	int y = sd->sy + (fh + _textDimData[sdx].visualLineSpacingAdjust) * _textDimData[sdx].line;
	int w = sd->w << 3;
	int lw = _lineWidth;
	int s = _numCharsLeft;
	uint8 twoByteCharOffs = 0;

	if (sjisTextMode) {
		bool ct = true;

		if ((lw + _textDimData[sdx].column) >= w) {
			if ((lines - 1 - (_waitButtonSpace << 1)) <= _lineCount)
				// cut off line to leave space for "MORE" button
				w -= _vm->guiSettings()->buttons.waitReserve;
		} else {
			if (!_twoByteLineBreakFlag || (_lineCount + 1 < lines - 1))
				ct = false;
			else
				// cut off line to leave space for "MORE" button
				w -= _vm->guiSettings()->buttons.waitReserve;
		}

		if (ct) {
			w -= _textDimData[sdx].column;

			int n2 = 0;
			int n1 = (w / 4) - 1;

			while (n2 < n1 && n2 < s) {
				if (isTwoByteChar(str[n2]))
					n2++;
				n2++;
			}
			s = n2;
		}
	} else {
		for (int i = 0; i < s; ++i) {
			if (isTwoByteChar(str[i]))
				twoByteCharOffs = (_vm->game() == GI_EOB1 || _isChinese) ? 16 : 8;
		}

		if (_isChinese) {
			s = strlen(str);
			if ((lw + _textDimData[sdx].column) >= w) {
				s -= ((lw + _textDimData[sdx].column - w) >> 3);
				w -= _textDimData[sdx].column;
			}

		} else if ((lw + _textDimData[sdx].column) >= w) {
			if ((lines - 1) <= _lineCount && _allowPageBreak)
				// cut off line to leave space for "MORE" button
				w -= _vm->guiSettings()->buttons.waitReserve;

			w -= _textDimData[sdx].column;

			int lineLastCharPos = 0;
			int strPos = s - 1;

			if (twoByteCharOffs) {
				lw = 0;
				int prevStrPos = 0;

				for (strPos = 0; strPos < s; ++strPos) {
					uint8 c = str[strPos];
					if (isTwoByteChar(c))
						lw += (_screen->getCharWidth(c | (uint8)str[++strPos] << 8) + _textDimData[sdx].charSpacing);
					else
						lw += _screen->getCharWidth(c);

					if (!lineLastCharPos && w < lw + twoByteCharOffs)
						lineLastCharPos = prevStrPos;

					if (lineLastCharPos && c == ' ') {
						s = strPos;
						_printFlag = false;
						break;
					}
					prevStrPos = strPos;
				}

				if (!lineLastCharPos) {
					lineLastCharPos = s - 1;
					if (lineLastCharPos && str[lineLastCharPos] == ' ') {
						s = strPos;
						_printFlag = false;
					}
				}

				lw = _lineWidth;

			} else {
				while (strPos > 0) {
					//cut off line after last space
					uint8 c = str[strPos];
					lw -= _screen->getCharWidth(c);

					if (!lineLastCharPos && lw <= w)
						lineLastCharPos = strPos;

					if (lineLastCharPos && c == ' ') {
						s = strPos;
						_printFlag = false;
						break;
					}
					strPos--;
				}
			}

			if (!strPos) {
				if (_textDimData[sdx].column && !_printFlag) {
					s = lw = 0;
					_printFlag = true;
				} else {
					s = lineLastCharPos;
				}
			}
		}
	}

	char lastChr = str[s];
	str[s] = 0;

	uint8 col1 = remapColor(sdx, _textDimData[sdx].color1);
	uint8 col2 = remapColor(sdx, _textDimData[sdx].color2);
	if (sjisTextMode && (sdx == 2 || sdx == 3 || sdx == 4 || sdx == 5 || sdx == 15)) {
		x1 &= ~3;
		y = (y + 8) & ~7;
		col2 = 0;
	}

	if (_textDimData[sdx].shadowColor) {
		_screen->printText(str, x1 - 1, y, _textDimData[sdx].shadowColor, col2);
		_screen->printText(str, x1, y + 1, _textDimData[sdx].shadowColor, 0);
		_screen->printText(str, x1 - 1, y + 1, _textDimData[sdx].shadowColor, 0);
		// Another hack for Chinese EOBII. Due to the reduced line spacing - while still drawing a shadow for the font - the
		// lines will overdraw by one pixel if we don't clear the bottom line. This will otherwise cause glitches when doing line feeds.
		for (int i = 0; i < -_textDimData[sdx].lineSpacing && y + fh + i < sd->sy + sd->h; ++i)
			_screen->drawClippedLine(x1 - 1, y + fh + i, x1 + lw, y + fh + i, col2);
		col2 = 0;
	}
	_screen->printText(str, x1, y, col1, col2);

	_textDimData[sdx].column += lw;
	_numCharsPrinted += strlen(str);

	str[s] = lastChr;
	if (lastChr == ' ')
		s++;
	if (str[s] == ' ')
		s++;

	uint32 len = strlen(&str[s]);
	for (uint32 i = 0; i < len; i++)
		str[i] = str[s + i];
	str[len] = 0;

	_numCharsLeft = strlen(str);
	_lineWidth = _screen->getTextWidth(str) + _textDimData[sdx].charSpacing * _numCharsLeft;

	if (!_numCharsLeft && (_textDimData[sdx].column + twoByteCharOffs) <= (sd->w << 3))
		return;

	_textDimData[sdx].column = 0;
	_textDimData[sdx].line++;
	_lineCount++;

	if (_numCharsLeft || !_isChinese)
		printLine(str);
}

void TextDisplayer_rpg::printDialogueText(int stringId, const char *pageBreakString, const char*) {
	const char *str = (const char *)(_screen->getCPagePtr(5) + READ_LE_UINT16(&_screen->getCPagePtr(5)[(stringId - 1) << 1]));
	assert(strlen(str) < kEoBTextBufferSize);
	Common::strlcpy(_dialogueBuffer, str, kEoBTextBufferSize);

	_screen->set16bitShadingLevel(4);
	int cs = (_vm->gameFlags().platform == Common::kPlatformPC98 && !_vm->gameFlags().use16ColorMode) ? _screen->setFontStyles(_screen->_currentFont, Font::kStyleFat) : -1;
	displayText(_dialogueBuffer);
	if (cs != -1)
		_screen->setFontStyles(_screen->_currentFont, cs);
	_screen->set16bitShadingLevel(0);

	if (pageBreakString) {
		if (pageBreakString[0]) {
			_pageBreakString = pageBreakString;
			displayWaitButton();
			resetPageBreakString();
		}
	}

	_vm->_dialogueFieldAmiga = true;
}

void TextDisplayer_rpg::printDialogueText(const char *str, bool wait) {
	assert(Common::strnlen(str, kEoBTextBufferSize) < kEoBTextBufferSize);
	Common::strlcpy(_dialogueBuffer, str, kEoBTextBufferSize);

	int cs = (_vm->gameFlags().platform == Common::kPlatformPC98 && !_vm->gameFlags().use16ColorMode) ? _screen->setFontStyles(_screen->_currentFont, Font::kStyleFat) : -1;
	displayText(_dialogueBuffer);
	if (cs != -1)
		_screen->setFontStyles(_screen->_currentFont, cs);
	if (wait)
		displayWaitButton();
}

void TextDisplayer_rpg::printMessage(const char *str, int textColor, ...) {
	int tc = _textDimData[_screen->curDimIndex()].color1;

	if (textColor != -1)
		_textDimData[_screen->curDimIndex()].color1= textColor;

	va_list args;
	va_start(args, textColor);
	vsnprintf(_dialogueBuffer, kEoBTextBufferSize - 1, str, args);
	va_end(args);

	displayText(_dialogueBuffer, textColor);

	if (_vm->game() != GI_EOB1)
		_textDimData[_screen->curDimIndex()].color1= tc;

	if (!_screen->_curPage)
		_screen->updateScreen();
}

int TextDisplayer_rpg::clearDim(int dim) {
	int res = _screen->curDimIndex();
	_screen->setScreenDim(dim);
	_textDimData[dim].color1 = _screen->_curDim->col1;
	_textDimData[dim].color2 = (_vm->game() == GI_LOL || _vm->gameFlags().platform == Common::kPlatformAmiga) ? _screen->_curDim->col2 : _vm->guiSettings()->colors.fill;

	clearCurDim();
	return res;
}

void TextDisplayer_rpg::clearCurDim() {
	int d = _screen->curDimIndex();
	const ScreenDim *tmp = _screen->getScreenDim(d);

	int xOffs = 0;
	int wOffs = 0;
	int hOffs = 0;

	if (_textDimData[d].shadowColor) {
		if (tmp->sx > 0)
			xOffs = wOffs = 1;
	}

	if (_pc98TextMode) {
		--wOffs;
		--hOffs;
	}

	_screen->fillRect((tmp->sx << 3) - xOffs, tmp->sy, ((tmp->sx + tmp->w) << 3) - 1 + wOffs, (tmp->sy + tmp->h) - 1 + hOffs, _textDimData[d].color2);

	_lineCount = 0;
	_textDimData[d].column = _textDimData[d].line = 0;
}

void TextDisplayer_rpg::textPageBreak() {
	if (_vm->game() != GI_LOL)
		SWAP(_vm->_dialogueButtonLabelColor1, _vm->_dialogueButtonLabelColor2);

	int cp = _screen->setCurPage(0);
	Screen::FontId cf = _screen->setFont(_waitButtonFont);
	int cs = (_vm->gameFlags().platform == Common::kPlatformPC98 && !_vm->gameFlags().use16ColorMode) ? _screen->setFontStyles(_waitButtonFont, Font::kStyleFat) : -1;

	if (_vm->game() == GI_LOL)
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

	int sdx = _screen->curDimIndex();
	const ScreenDim *dim = _screen->getScreenDim(sdx);

	int x = ((dim->sx + dim->w) << 3) - (_vm->_dialogueButtonWidth + 3);
	int y = 0;
	int w = _vm->_dialogueButtonWidth;

	if (_vm->game() == GI_LOL) {
		if (_isChinese) {
			y = dim->sy + dim->h - 15;
		} else if (_vm->_needSceneRestore && (_vm->_updateFlags & 2)) {
			if (_vm->_currentControlMode || !(_vm->_updateFlags & 2)) {
				y = dim->sy + dim->h - 5;
			} else {
				x += 6;
				y = dim->sy + dim->h - 2;
			}
		} else {
			y = dim->sy + dim->h - 10;
		}
	} else {
		y = _vm->guiSettings()->buttons.waitY[_waitButtonMode];
		x = _vm->guiSettings()->buttons.waitX[_waitButtonMode];
		w = _vm->guiSettings()->buttons.waitWidth[_waitButtonMode];
	}

	if (_vm->game() == GI_LOL && _vm->gameFlags().use16ColorMode) {
		_vm->gui_drawBox(x + 8, (y & ~7) - 1, 66, 10, 0xEE, 0xCC, -1);
		_screen->printText(_pageBreakString.c_str(), (x + 37 - (_pageBreakString.size() << 1) + 4) & ~3, (y + 2) & ~7, 0xC1, 0);
	} else {
		_screen->set16bitShadingLevel(4);
		_vm->gui_drawBox(x, y, w, _vm->guiSettings()->buttons.height, _vm->guiSettings()->colors.frame1, _vm->guiSettings()->colors.frame2, _vm->guiSettings()->colors.fill);
		_screen->set16bitShadingLevel(0);
#if defined(ENABLE_EOB)
		if (_vm->guiSettings()->buttons.labelShadow && _vm->game() != GI_LOL)
			((Screen_EoB*)screen())->printShadedText(_pageBreakString.c_str(), x + (w >> 1) - (_vm->screen()->getTextWidth(_pageBreakString.c_str()) >> 1), y + _vm->guiSettings()->buttons.txtOffsY, _vm->_dialogueButtonLabelColor1, 0, _vm->guiSettings()->colors.guiColorBlack);
		else
#endif
			_screen->printText(_pageBreakString.c_str(), x + (w >> 1) - (_vm->screen()->getTextWidth(_pageBreakString.c_str()) >> 1), y + _vm->guiSettings()->buttons.txtOffsY, _vm->_dialogueButtonLabelColor1, 0);
	}

	_vm->removeInputTop();

	bool loop = true;
	bool target = false;

	do {
		int inputFlag = _vm->checkInput(0, false) & 0xFF;
		_vm->removeInputTop();

		while (!inputFlag && !_vm->shouldQuit()) {
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
			if (_vm->posWithinRect(_vm->_mouseX, _vm->_mouseY, x, y, x + w, y + _vm->guiSettings()->buttons.height)) {
				if (_vm->game() == GI_LOL)
					target = true;
				else
					loop = false;
			}
		} else if (inputFlag == 200 || inputFlag == 202) {
			if (target)
				loop = false;
		}
	} while (loop && !_vm->shouldQuit());

	_screen->set16bitShadingLevel(4);
	if (_vm->game() == GI_LOL && _vm->gameFlags().use16ColorMode)
		_screen->fillRect(x + 8, y, x + 57, y + _vm->guiSettings()->buttons.height, _textDimData[sdx].color2);
	else
		_screen->fillRect(x, y, x + w - 1, y + _vm->guiSettings()->buttons.height - 1, _textDimData[sdx].color2);

	// Fix border overdraw glitch
	if (_vm->game() == GI_EOB2 && _isChinese && y + _vm->guiSettings()->buttons.height == 200)
		_screen->drawClippedLine(x, 199, x + w - 1, 199, _vm->guiSettings()->colors.frame1);

	clearCurDim();
	_screen->set16bitShadingLevel(0);
	_screen->updateScreen();

	if (_vm->game() == GI_LOL)
		_vm->_timer->pauseSingleTimer(11, false);

	if (_vm->_updateCharNum != -1) {
		_vm->_resetPortraitAfterSpeechAnim = resetPortraitAfterSpeechAnim;
		if (updatePortraitSpeechAnimDuration > 36)
			updatePortraitSpeechAnimDuration -= 36;
		else
			updatePortraitSpeechAnimDuration >>= 1;

		_vm->_updatePortraitSpeechAnimDuration = updatePortraitSpeechAnimDuration;
	}

	if (cs != -1)
		_screen->setFontStyles(_waitButtonFont, cs);
	_screen->setFont(cf);
	_screen->setCurPage(cp);

	if (_vm->game() != GI_LOL)
		SWAP(_vm->_dialogueButtonLabelColor1, _vm->_dialogueButtonLabelColor2);

	_vm->removeInputTop();
}

void TextDisplayer_rpg::displayWaitButton() {
	_vm->_dialogueNumButtons = 1;
	_vm->_dialogueButtonString[0] = _pageBreakString.c_str();
	_vm->_dialogueButtonString[1] = 0;
	_vm->_dialogueButtonString[2] = 0;
	_vm->_dialogueHighlightedButton = 0;

	_vm->_dialogueButtonPosX = &_vm->guiSettings()->buttons.waitX[_waitButtonMode];
	_vm->_dialogueButtonPosY = &_vm->guiSettings()->buttons.waitY[_waitButtonMode];
	_vm->_dialogueButtonWidth = _vm->guiSettings()->buttons.waitWidth[_waitButtonMode];
	_vm->_dialogueButtonYoffs = 0;

	SWAP(_vm->_dialogueButtonLabelColor1, _vm->_dialogueButtonLabelColor2);
	_vm->drawDialogueButtons();

	if (!_vm->shouldQuit())
		_vm->removeInputTop();

	while (!_vm->processDialogue() && !_vm->shouldQuit()) {}

	_screen->set16bitShadingLevel(4);
	_screen->fillRect(_vm->_dialogueButtonPosX[0], _vm->_dialogueButtonPosY[0], _vm->_dialogueButtonPosX[0] + _vm->_dialogueButtonWidth - 1, _vm->_dialogueButtonPosY[0] + _vm->guiSettings()->buttons.height - 1, _vm->guiSettings()->colors.fill);
	clearCurDim();
	_screen->set16bitShadingLevel(0);
	_screen->updateScreen();
	_vm->_dialogueButtonWidth = _vm->guiSettings()->buttons.width;
	SWAP(_vm->_dialogueButtonLabelColor1, _vm->_dialogueButtonLabelColor2);
}

void TextDisplayer_rpg::convertString(char *str) {
	static const char convertTable_EOB2_Amiga_DE[] = {
		'\x84', '\x7F', '\x8E', '\x7F', '\x81', '\x7D', '\x9A', '\x7D', '\x94', '\x7E', '\x99', '\x7E', '\0', '\0'
	};

	const char *table = 0;

	if (_vm->game() == GI_EOB2 && _vm->gameFlags().platform == Common::kPlatformAmiga && _vm->gameFlags().lang == Common::DE_DEU)
		table = convertTable_EOB2_Amiga_DE;

	if (!table)
		return;

	for (; *str; ++str) {
		for (const char *c = table; *c; c += 2) {
			if ((*str) == c[0])
				*str = c[1];
		}
	}
}

bool TextDisplayer_rpg::isTwoByteChar(uint8 c) const {
	if (_vm->gameFlags().lang == Common::JA_JPN)
		return (c >= 0xE0 || (c > 0x80 && c < 0xA0));
	else if (_isChinese)
		return (c & 0x80);
	return false;
}

void TextDisplayer_rpg::applySetting(int sd, int ix, int val) {
	if (sd < -1 || sd >= _dimCount || ix >= kOutOfRange)
		error("TextDisplayer_rpg::applySetting(): arg out of range");

	const int *memberAddr[] = {
		&_textDimData[0].lineSpacing,
		&_textDimData[0].visualLineSpacingAdjust,
		&_textDimData[0].charSpacing,
		&_textDimData[0].shadowColor,
		&_textDimData[0].noHalfWidthLineEnd
	};

	int offset = (const byte*)memberAddr[ix] - (const byte*)&_textDimData[0];

	if (sd == -1) {
		for (int i = 0; i < _dimCount; ++i)
			*(int*)((byte*)&_textDimData[i] + offset) = val;
	} else {
		*(int*)((byte*)&_textDimData[sd] + offset) = val;
	}
}

uint8 TextDisplayer_rpg::remapColor(int sd, uint8 color) const {
	if (sd < -1 || sd >= _dimCount)
		error("TextDisplayer_rpg::applySetting(): arg out of range");

	// HACK: Apparently, this needs a better implementation (allowing to set
	// mappings for col1 and col2 independently). But this will do for now...
	if (_vm->gameFlags().platform == Common::kPlatformAmiga && sd != 7 && color == _textDimData[sd].color2)
		return color;

	if (sd != -1 && _textDimData[sd].colorMap != nullptr)
		return _textDimData[sd].colorMap[color];

	return _colorMap[color];
}

} // End of namespace Kyra

#endif // (ENABLE_EOB || ENABLE_LOL)
