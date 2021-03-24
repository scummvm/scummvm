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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "trecision/trecision.h"

#include "common/textconsole.h"
#include "nl/extern.h"
#include "nl/message.h"

namespace Trecision {
/* --------------------------------------------------
 * 					getNextSentence
 * --------------------------------------------------*/
char *TrecisionEngine::getNextSentence() {
	while (*_textPtr) {
		*_textPtr = ~(*_textPtr);
		_textPtr++;
	}

	_textPtr++;
	return _textPtr;
}

/*-------------------------------------------------------------------------*/
/*                                   TEXT              					   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::addText(uint16 x, uint16 y, const char *sign, uint16 tcol, uint16 scol) {
	_textStackTop++;
	if (_textStackTop >= MAXTEXTSTACK) {
		warning("MAXTEXTSTACK Reached!");
		return;
	}

	_textStack[_textStackTop].x = x;
	_textStack[_textStackTop].y = y;
	_textStack[_textStackTop].tcol = tcol;
	_textStack[_textStackTop].scol = scol;
	_textStack[_textStackTop].Clear = false;
	strcpy(_textStack[_textStackTop].sign, sign);
}

/* -----------------08/07/97 22.13-------------------
						clearText
 --------------------------------------------------*/
void TrecisionEngine::clearText() {
	if (_textStackTop >= 0) {
		// The stack isn't empty
		if (!_textStack[_textStackTop].Clear)
			// The previous is a string to write, return
			_textStackTop--;
	} else {
		// the stack is empty
		_textStackTop = 0;
		_textStack[_textStackTop].Clear = true;
	}
}

/* -----------------08/07/97 22.14-------------------
					drawString
 --------------------------------------------------*/
void TrecisionEngine::drawString() {
	for (int16 i = 0; i <= _textStackTop; i++) {
		if (_textStack[i].Clear)
			doClearText();
		else
			_textStack[i].DoText();
	}
}

/*-------------------------------------------------------------------------*/
/*                           redrawString		        				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::redrawString() {
	if (!FlagDialogActive && !FlagDialogMenuActive && !FlagSomeOneSpeak && !_flagscriptactive && _flagMouseEnabled) {
		if (INVAREA(my))
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, 0, 0, 0, 0);
		else {
			CheckMask(mx, my);
			ShowObjName(_curObj, true);
		}
	}
}

/* -----------------08/07/97 22.15-------------------
						DoText
 --------------------------------------------------*/
void StackText::DoText() {
	curString.x = x;
	curString.y = y;
	curString.dx = TextLength(sign, 0);
	if ((y == MAXY - CARHEI) && (curString.dx > 600))
		curString.dx = curString.dx * 3 / 5;
	else if ((y != MAXY - CARHEI) && (curString.dx > 960))
		curString.dx = curString.dx * 2 / 5;
	else if ((y != MAXY - CARHEI) && (curString.dx > 320))
		curString.dx = curString.dx * 3 / 5;

	curString.sign = sign;
	curString.l[0] = 0;
	curString.l[1] = 0;
	curString.l[2] = curString.dx;
	uint16 hstring = curString.checkDText();
	curString.l[3] = hstring;
	curString.dy = hstring;
	curString.tcol = tcol;
	curString.scol = scol;

	if (curString.y <= hstring)
		curString.y += hstring;
	else
		curString.y -= hstring;

	if (curString.y <= VIDEOTOP)
		curString.y = VIDEOTOP + 1;

	TextStatus |= TEXT_DRAW;
}

/* -----------------08/07/97 22.15-------------------
					DoClearString
 --------------------------------------------------*/
void TrecisionEngine::doClearText() {
	if ((oldString.sign == nullptr) && (curString.sign)) {
		oldString.set(curString);
		curString.sign = nullptr;

		TextStatus |= TEXT_DEL;
	}
}

} // End of namespace Trecision
