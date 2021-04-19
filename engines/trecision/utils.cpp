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

#include "trecision/nl/extern.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/logic.h"
#include "trecision/trecision.h"

namespace Trecision {

void TrecisionEngine::initNames() {
	_sysText[kMessageSavePosition] = "SAVE POSITION";
	_sysText[kMessageEmptySpot] = "EMPTY SLOT";
	_sysText[kMessageLoadPosition] = "LOAD POSITION";
	_sysText[kMessageConfirmExit] = "Are you sure that you want to quit (y/n)?";
	_sysText[kMessageDemoOver] = "This demo is over.";
	_sysText[kMessageError] = "ERROR!";
	_sysText[kMessageUse] = "Use ";
	_sysText[kMessageWith] = " with ";
	_sysText[kMessageGoto] = "Go to ";
	_sysText[kMessageGoto2] = "Go to ... ";

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
void TrecisionEngine::addText(uint16 x, uint16 y, const char *text, uint16 tcol, uint16 scol) {
	_textStackTop++;
	if (_textStackTop >= MAXTEXTSTACK) {
		warning("MAXTEXTSTACK Reached!");
		return;
	}

	_textStack[_textStackTop].x = x;
	_textStack[_textStackTop].y = y;
	_textStack[_textStackTop].tcol = tcol;
	_textStack[_textStackTop].scol = scol;
	_textStack[_textStackTop].clear = false;
	strcpy(_textStack[_textStackTop].text, text);
}

/* ------------------------------------------------
						clearText
 --------------------------------------------------*/
void TrecisionEngine::clearText() {
	if (_textStackTop >= 0) {
		// The stack isn't empty
		if (!_textStack[_textStackTop].clear)
			// The previous is a string to write, return
			_textStackTop--;
	} else {
		// the stack is empty
		_textStackTop = 0;
		_textStack[_textStackTop].clear = true;
	}
}

/* ------------------------------------------------
					drawString
 --------------------------------------------------*/
void TrecisionEngine::drawString() {
	for (int16 i = 0; i <= _textStackTop; i++) {
		if (_textStack[i].clear)
			doClearText();
		else
			_textStack[i].doText();
	}
}

/*-------------------------------------------------------------------------*/
/*                           redrawString		        				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::redrawString() {
	if (!_flagDialogActive && !_flagDialogMenuActive && !_flagSomeoneSpeaks && !_flagscriptactive && g_vm->isCursorVisible()) {
		if (isInventoryArea(g_vm->_mouseY))
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, 0, 0, 0, 0);
		else {
			CheckMask(g_vm->_mouseX, g_vm->_mouseY);
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
	curString.dx = TextLength(text, 0);
	if ((y == MAXY - CARHEI) && (curString.dx > 600))
		curString.dx = curString.dx * 3 / 5;
	else if ((y != MAXY - CARHEI) && (curString.dx > 960))
		curString.dx = curString.dx * 2 / 5;
	else if ((y != MAXY - CARHEI) && (curString.dx > 320))
		curString.dx = curString.dx * 3 / 5;

	curString.text = text;
	curString._subtitleRect.left = 0;
	curString._subtitleRect.top = 0;
	curString._subtitleRect.right = curString.dx;
	uint16 hstring = curString.checkDText();
	curString._subtitleRect.bottom = hstring;
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
	if ((oldString.text == nullptr) && (curString.text)) {
		oldString.set(curString);
		curString.text = nullptr;

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
