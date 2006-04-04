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

#include <sys/stat.h>

namespace Simon {

void SimonEngine::checkLinkBox() {	// Check for boxes spilling over to next row of text
	if (_hyperLink != 0) {
		_variableArray[52] = _textWindow->x + _textWindow->textColumn - _variableArray[50];
		if (_variableArray[52] != 0) {
			addNewHitArea(_variableArray[53], _variableArray[50], _variableArray[51], _variableArray[52], 15, 145, 208, _dummyItem1);
			_variableArray[53]++;
		}
		_variableArray[50] = _textWindow->x;
		_variableArray[51] = _textWindow->textRow + _textWindow->y + (_oracleMaxScrollY-_textWindow->scrollY) * 15;
	}
}

void SimonEngine::hyperLinkOn(uint16 x) {
	if ((_bitArray[3] & (1 << 3)) == 0)
		return;

	_hyperLink = x;
	_variableArray[50] = _textWindow->textColumn+_textWindow->x;
	_variableArray[51] = _textWindow->textRow+_textWindow->y + (_oracleMaxScrollY - _textWindow->scrollY) * 15;
}


void SimonEngine::hyperLinkOff() {
	if ((_bitArray[3] & (1 << 3)) == 0)
		return;

	_variableArray[52] = _textWindow->x + _textWindow->textColumn - _variableArray[50];
	addNewHitArea(_variableArray[53], _variableArray[50], _variableArray[51], _variableArray[52], 15, 145, 208, _dummyItem1);
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

	if(_textWindow->scrollY > _oracleMaxScrollY) 		// For scroll up
		_oracleMaxScrollY = _textWindow->scrollY;

	while(1) {
		if(_textWindow->scrollY == _oracleMaxScrollY)
			break;
		_textWindow->textRow = 105;
		for (i = 0; i < 5; i++) {
			_newLines = 0;
			_textWindow->textColumn = 0;
			_textWindow->textRow -= 3;
			if(i == 2) {
				_textWindow->scrollY += 1;
				_textWindow->textRow += 15;
				linksUp();
			}
			scrollOracleUp();
			_bitArray[5] |= (1 << 14);
			sub = getSubroutineByID(_variableArray[104]);
			if(sub)
				startSubroutineEx(sub);
			_bitArray[5] &= ~(1 << 14);
			bltOracleText();
		}
		if (_currentBoxNumber != 600 || _leftButtonDown)
			break;
	}
}

void SimonEngine::oracleTextDown() {
	Subroutine *sub;
	int i = 0;
	changeWindow(3);
	_noOracleScroll = 0;

	if(_textWindow->scrollY > _oracleMaxScrollY) 		// For scroll up
		_oracleMaxScrollY = _textWindow->scrollY;

	while(1) {
		if(_textWindow->scrollY == 0)
			break;
		for (i = 0; i < 5; i++) {
			_newLines = 0;
			_textWindow->textColumn = 0;
			_textWindow->textRow = (i + 1) * 3;
			if(i == 4) {
				_textWindow->scrollY -= 1;
				_textWindow->textRow = 0;
				linksDown();
			}
			scrollOracleDown();
			_bitArray[5] |= (1 << 13);
			sub = getSubroutineByID(_variableArray[104]);
			if (sub)
				startSubroutineEx(sub);
			_bitArray[5] &= ~(1 << 13);
			bltOracleText();
		}
		if (_currentBoxNumber != 600 || _leftButtonDown)
			break;
	}
}

void SimonEngine::listSaveGames(int n) {
	char b[108];
	Common::InSaveFile *in;
	uint16 j, k, z, maxFiles;
	int OK;
	memset(b, 0, 108);

	maxFiles = countSaveGames();
	j = maxFiles - n + 1;
	k = maxFiles - j + 1;
	z = maxFiles;
	if (getBitFlag(95)) {
		j++;
		z++;
	}

	while(1) {
		OK=1;
		if (getBitFlag(93)) {
			OK = 0;
			if(j > z)
				break;
		}

		if (getBitFlag(93)) {
			if (((_newLines + 1) >= _textWindow->scrollY) && (
						(_newLines + 1) < (_textWindow->scrollY + 3)))
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
				o_setTextColor(116);
	    			showMessageFormat(" %d. ",1);
				hyperLinkOff();
				o_setTextColor(113);
				k++;
				j--;
			}

			if (!(in = _saveFileMan->openForLoading(gen_savename(j))))
				break;
			in->read(b, 100);
			delete in;
		}

		showMessageFormat("\n");
		hyperLinkOn(j + 400);
		o_setTextColor(116);
		if (k < 10)
			showMessageFormat(" ");
		showMessageFormat("%d. ",k);
		o_setTextColor(113);
		showMessageFormat("%s ",b);
		hyperLinkOff();
		j--;
		k++;
	}
}

void SimonEngine::scrollOracleUp() {
	// TODO
}

void SimonEngine::scrollOracleDown() {
	// TODO
}

void SimonEngine::bltOracleText() {
	// TODO
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

} // End of namespace Simon
