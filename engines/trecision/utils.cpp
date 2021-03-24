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
#include "trecision/nl/struct.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/message.h"

#include "common/textconsole.h"
#include "logic.h"

namespace Trecision {

void TrecisionEngine::initNames() {
	_sysText[1] = "NightLong was not properly installed!\nRun Autorun.exe from the CD-Rom.";
	_sysText[2] = "Not enough memory!\nYou need %d bytes more.\n";
	_sysText[3] = "Unknown error\n";
	_sysText[4] = "Please insert CD number %c and press return.";
	_sysText[5] = "Unknown error";
	_sysText[6] = "VESA Not Supported.\n";
	_sysText[7] = "Error reading file.";
	_sysText[8] = "Mouse not found error!\n";
	_sysText[9] = "SAVE POSITION";
	_sysText[10] = "EMPTY SLOT";
	_sysText[11] = "LOAD POSITION";
	_sysText[12] = "Error reading saved-game";
	_sysText[13] = "Are you sure that you want to quit (y/n)?";
	_sysText[14] = "Unknown error\n";
	_sysText[15] = "Sample handle not available\n";
	_sysText[16] = "Run NL to select an appropriate digital audio driver\n";
	_sysText[17] = "This demo is over.";
	_sysText[18] = "NightLong";
	_sysText[19] = "ERROR!";
	_sysText[20] = "Unsupported pixel format.";
	_sysText[21] = "DirectX Error";
	_sysText[22] = "NightLong Warning";
	_sysText[23] = "Use ";
	_sysText[24] = " with ";
	_sysText[25] = "Go to ";
	_sysText[26] = "Go to ... ";

	_sentence[0] = "          "; // Use it like a buffer !!!!
	_objName[0] = " ";
}

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
/*                                   addText           					   */
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

/* ------------------------------------------------
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

/* ------------------------------------------------
					drawString
 --------------------------------------------------*/
void TrecisionEngine::drawString() {
	for (int16 i = 0; i <= _textStackTop; i++) {
		if (_textStack[i].Clear)
			doClearText();
		else
			_textStack[i].doText();
	}
}

/*-------------------------------------------------------------------------*/
/*                           redrawString		        				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::redrawString() {
	if (!FlagDialogActive && !FlagDialogMenuActive && !FlagSomeOneSpeak && !_flagscriptactive && _flagMouseEnabled) {
		if (isInventoryArea(my))
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, 0, 0, 0, 0);
		else {
			CheckMask(mx, my);
			ShowObjName(_curObj, true);
		}
	}
}

/* ------------------------------------------------
						doText
 --------------------------------------------------*/
void StackText::doText() {
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

/* ------------------------------------------------
					doClearText
 --------------------------------------------------*/
void TrecisionEngine::doClearText() {
	if ((oldString.sign == nullptr) && (curString.sign)) {
		oldString.set(curString);
		curString.sign = nullptr;

		TextStatus |= TEXT_DEL;
	}
}

/* --------------------------------------------------
 * 					setRoom
 * --------------------------------------------------*/
void TrecisionEngine::setRoom(uint16 r, bool b) {
	_logicMgr->setRoom(r, b);
	RegenRoom();
}


} // End of namespace Trecision
