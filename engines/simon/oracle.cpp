/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/savefile.h"

#include "simon/simon.h"
#include "simon/intern.h"
#include "simon/vga.h"

namespace Simon {

void SimonEngine::checkLinkBox() {	// Check for boxes spilling over to next row of text
	if (_hyperLink != 0) {
		_variableArray[52] = _textWindow->x + _textWindow->textColumn - _variableArray[50];
		if (_variableArray[52] != 0) {
			defineBox(_variableArray[53], _variableArray[50], _variableArray[51], _variableArray[52], 15, 145, 208, _dummyItem2);
			_variableArray[53]++;
		}
		_variableArray[50] = _textWindow->x;
		_variableArray[51] = _textWindow->textRow + _textWindow->y + (_oracleMaxScrollY-_textWindow->scrollY) * 15;
	}
}

void SimonEngine::hyperLinkOn(uint16 x) {
	if (!getBitFlag(51))
		return;

	_hyperLink = x;
	_variableArray[50] = _textWindow->textColumn + _textWindow->x;
	_variableArray[51] = _textWindow->textRow + _textWindow->y + (_oracleMaxScrollY - _textWindow->scrollY) * 15;
}


void SimonEngine::hyperLinkOff() {
	if (!getBitFlag(51))
		return;

	_variableArray[52] = _textWindow->x + _textWindow->textColumn - _variableArray[50];
	defineBox(_variableArray[53], _variableArray[50], _variableArray[51], _variableArray[52], 15, 145, 208, _dummyItem2);
	_variableArray[53]++;
	_hyperLink = 0;
}

void SimonEngine::linksUp() {	// Scroll Oracle Links
	uint16 j;
	for (j = 700; j < _variableArray[53]; j++) {
		moveBox(j, 0, -15);
	}
}

void SimonEngine::linksDown() {
	uint16 i;
	for (i = 700; i < _variableArray[53]; i++) {
		moveBox(i,0, 15);
	}
}

void SimonEngine::scrollOracle() {
	int i;

	for (i = 0; i < 5; i++) {
		scrollOracleUp();
		bltOracleText();
	}
}

void SimonEngine::oracleTextUp() {
	Subroutine *sub;
	int i = 0;
	changeWindow(3);
	_noOracleScroll = 0;

	if (_textWindow->scrollY > _oracleMaxScrollY) 		// For scroll up
		_oracleMaxScrollY = _textWindow->scrollY;
	while(1) {
		if (_textWindow->scrollY == _oracleMaxScrollY)
			break;
		_textWindow->textRow = 105;
		for (i = 0; i < 5; i++) {
			_newLines = 0;
			_textWindow->textColumn = 0;
			_textWindow->textRow -= 3;
			if (i == 2) {
				_textWindow->scrollY += 1;
				_textWindow->textRow += 15;
				linksUp();
			}
			scrollOracleUp();
			setBitFlag(94, true);
			sub = getSubroutineByID(_variableArray[104]);
			if(sub)
				startSubroutineEx(sub);
			setBitFlag(94, false);
			bltOracleText();
		}
		if (_currentBoxNumber != 601 || !getBitFlag(89))
			break;
		delay(100);
	}
}

void SimonEngine::oracleTextDown() {
	Subroutine *sub;
	int i = 0;
	changeWindow(3);
	_noOracleScroll = 0;

	if (_textWindow->scrollY > _oracleMaxScrollY) 		// For scroll up
		_oracleMaxScrollY = _textWindow->scrollY;
	while(1) {
		if (_textWindow->scrollY == 0)
			break;

		for (i = 0; i < 5; i++) {
			_newLines = 0;
			_textWindow->textColumn = 0;
			_textWindow->textRow = (i + 1) * 3;
			if (i == 4) {
				_textWindow->scrollY -= 1;
				_textWindow->textRow = 0;
				linksDown();
			}
			scrollOracleDown();
			setBitFlag(93, true);
			sub = getSubroutineByID(_variableArray[104]);
			if (sub)
				startSubroutineEx(sub);
			setBitFlag(93, false);
			bltOracleText();
		}
		if (_currentBoxNumber != 600 || !getBitFlag(89))
			break;
		delay(100);
	}
}

void SimonEngine::scrollOracleUp() {
	byte *src, *dst;
	uint16 w, h;

	dst = getFrontBuf() + 103 * _screenWidth + 136;
	src = getFrontBuf() + 106 * _screenWidth + 136;

	for (h = 0; h < 21; h++) {
		for (w = 0; w < 360; w++) {
			if (dst[w] == 0 || dst[w] == 113  || dst[w] == 116 || dst[w] == 252)
				dst[w] = src[w];
		}
		dst += _screenWidth;
		src += _screenWidth;
	}

	for (h = 0; h < 80; h++) {
		memcpy(dst, src, 360);
		dst += _screenWidth;
		src += _screenWidth;
	}

	for (h = 0; h < 3; h++) {
		memset(dst, 0, 360);
		dst += _screenWidth;
		src += _screenWidth;
	}
}

void SimonEngine::scrollOracleDown() {
	byte *src, *dst;
	uint16 w, h;

	src = getFrontBuf() + 203 * _screenWidth + 136;
	dst = getFrontBuf() + 206 * _screenWidth + 136;

	for (h = 0; h < 77; h++) {
		memcpy(dst, src, 360);
		dst -= _screenWidth;
		src -= _screenWidth;
	}

	for (h = 0; h < 24; h++) {
		for (w = 0; w < 360; w++) {
			if (src[w] == 0)
				dst[w] = src[w];

			if (src[w] == 113  || src[w] == 116 || src[w] == 252) {
				dst[w] = src[w];
				src[w] = 0;
			}
		}
		dst -= _screenWidth;
		src -= _screenWidth;
	}
}

void SimonEngine::bltOracleText() {
	byte *src, *dst1, *dst2;
	uint16 h;

	src = getFrontBuf() + 103 * _screenWidth + 136;
	dst1 = getFrontBuf() + 103 * _screenWidth + 136;
	dst2 = getBackBuf() + 103 * _screenWidth + 136;

	for (h = 0; h < 104; h++) {
		memcpy(dst1, src, 360);
		memcpy(dst2, src, 360);
		dst1 += _screenWidth;
		dst2 += _screenWidth;
		src += _screenWidth;
	}
}

void SimonEngine::oracleLogo() {
	Common::Rect srcRect, dstRect;
	byte *src, *dst;
	uint16 w, h;

	dstRect.left = 16;
	dstRect.top = 16;
	dstRect.right = 58;
	dstRect.bottom = 59;

	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = 42;
	srcRect.bottom = 43;

	src = _iconFilePtr;
	dst = getBackBuf() + _screenWidth * dstRect.top + dstRect.left;

	for (h = 0; h < dstRect.height(); h++) {
		for (w = 0; w < dstRect.width(); w++) {
			if (src[w])
				dst[w] = src[w];
		}
		src += 336;
		dst += _screenWidth;
	}
}

void SimonEngine::swapCharacterLogo() {
	Common::Rect srcRect, dstRect;
	byte *src, *dst;
	uint16 w, h;
	int x;

	dstRect.left = 64;
	dstRect.top = 16;
	dstRect.right = 106;
	dstRect.bottom = 59;

	srcRect.top = 0;
	srcRect.bottom = 43;

	x = _variableArray[91];
	if (x > _variableArray[90])
		x--;
	if (x < _variableArray[90])
		x++;
	_variableArray[91] = x;

	x++;
	x *= 42;

	srcRect.left = x;
	srcRect.right = srcRect.left + 42;

	src = _iconFilePtr + srcRect.top * 336 + srcRect.left;
	dst = getBackBuf() + _screenWidth * dstRect.top + dstRect.left;

	for (h = 0; h < dstRect.height(); h++) {
		for (w = 0; w < dstRect.width(); w++) {
			if (src[w])
				dst[w] = src[w];
		}
		src += 336;
		dst += _screenWidth;
	}
}

void SimonEngine::listSaveGames(int n) {
	char b[108];
	Common::InSaveFile *in;
	uint16 j, k, z, maxFiles;
	int OK;
	memset(b, 0, 108);

	maxFiles = countSaveGames() - 1;
	j = maxFiles - n + 1;
	k = maxFiles - j + 1;
	z = maxFiles;
	if (getBitFlag(95)) {
		j++;
		z++;
	}

	while(1) {
		OK=1;
		if (getBitFlag(93) || getBitFlag(94)) {
			OK = 0;
			if(j > z)
				break;
		}

		if (getBitFlag(93)) {
			if (((_newLines + 1) >= _textWindow->scrollY) && ((_newLines + 1) < (_textWindow->scrollY + 3)))
				OK = 1;
		}

		if (getBitFlag(94)) {
			if ((_newLines + 1) == (_textWindow->scrollY + 7))
				OK = 1;
		}


		if (OK == 1) {
			if (j == maxFiles + 1) {
				showMessageFormat("\n");
				hyperLinkOn(j + 400);
				setTextColor(116);
	    			showMessageFormat(" %d. ",1);
				hyperLinkOff();
				setTextColor(113);
				k++;
				j--;
			}

			if (!(in = _saveFileMan->openForLoading(genSaveName(j))))
				break;
			in->read(b, 100);
			delete in;
		}

		showMessageFormat("\n");
		hyperLinkOn(j + 400);
		setTextColor(116);
		if (k < 10)
			showMessageFormat(" ");
		showMessageFormat("%d. ",k);
		setTextColor(113);
		showMessageFormat("%s ",b);
		hyperLinkOff();
		j--;
		k++;
	}
}

void SimonEngine::saveUserGame(int slot) {
	WindowBlock *window;
	Common::InSaveFile *in;
	char name[108];
	int len;
	memset(name, 0, 108);

	window = _windowArray[3];

	window->textRow = (slot + 1 - window->scrollY) * 15;
	window->textColumn = 26;

	if ((in = _saveFileMan->openForLoading(genSaveName(readVariable(55))))) {
		in->read(name, 100);
		delete in;
	}

	len = 0;
	while (name[len]) {
		byte chr = name[len];
		window->textColumn += getFeebleFontSize(chr);
		len++;
	}

	windowPutChar(window, 0x7f);
	for (;;) {
		_keyPressed = 0;
		delay(1);

		if (_keyPressed == 0 || _keyPressed >= 127)
			continue;

		window->textColumn -= getFeebleFontSize(127);
		name[len] = 0;
		windowBackSpace(_windowArray[3]);

		if (_keyPressed == 27) {
			writeVariable(55, _keyPressed);
			break;
		}
		if (_keyPressed == 10 || _keyPressed == 13) {
			if (!saveGame(readVariable(55), name))
				writeVariable(55, 0xFFFF);
			else
				writeVariable(55, 0);
			break;
		}
		if (_keyPressed == 8 && len != 0) {
			len--;
			byte chr = name[len];
			window->textColumn -= getFeebleFontSize(chr);
			name[len] = 0;
			windowBackSpace(_windowArray[3]);
		}
		if (_keyPressed >= 32 && window->textColumn + 26 <= window->width) {
			name[len++] = _keyPressed;
			windowPutChar(_windowArray[3], _keyPressed);
		}

		windowPutChar(window, 0x7f);
	}
}

void SimonEngine::windowBackSpace(WindowBlock *window) {
	byte *dst;
	uint x, y, h, w;

	_lockWord |= 0x8000;

	x = window->x + window->textColumn;
	y = window->y + window->textRow;

	dst = getFrontBuf() + _dxSurfacePitch * y + x;

	for (h = 0; h < 13; h++) {
		for (w = 0; w < 8; w++) {
			if (dst[w] == 113  || dst[w] == 116 || dst[w] == 252)
				dst[w] = 0;
		}
		dst += _screenWidth;
	}

	_lockWord &= ~0x8000;
}

} // End of namespace Simon
