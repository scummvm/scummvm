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

void TrecisionEngine::decompress(const unsigned char *src, unsigned src_len, unsigned char *dst, unsigned dst_len) {
	uint16 *sw = (uint16 *)(src + src_len);
	uint8 *d = dst;
	const uint8 *s = src;
	unsigned short ctrl = 0, ctrl_cnt = 1;

	while (s < (const uint8 *)sw) {
		if (!--ctrl_cnt) {
			ctrl = *--sw;
			ctrl_cnt = 16;
		} else {
			ctrl <<= 1;
		}

		if (ctrl & 0x8000) {
			uint16 foo = *--sw;
			const uint8 *cs = d - (foo >> 4);

			uint num = 16 - (foo & 0xF);

			for (uint16 i = 0; i < num; ++i)
				*d++ = *cs++;

			*d++ = *cs++;
			*d++ = *cs;
		} else {
			*d++ = *s++;
		}
	}
}

#define FAST_COOKIE 0xFA57F00D
uint32 TrecisionEngine::DecCR(Common::String fileName, uint8 *DestArea) {
	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember(fileName);
	if (ff == nullptr)
		error("File not found %s", fileName.c_str());

	int32 dataSize = ff->size() - 8;
	uint8 *ibuf = new uint8[dataSize];
	uint8 *obuf = DestArea;

	uint32 signature = ff->readUint32LE();
	if (signature != FAST_COOKIE)
		error("DecCR - %s has a bad signature and can't be loaded", fileName.c_str());

	int32 decompSize = ff->readSint32LE();
	ff->read(ibuf, dataSize);
	delete ff;

	if (dataSize < decompSize)
		decompress(ibuf, dataSize, obuf, decompSize);
	else
		memcpy(obuf, ibuf, dataSize);

	delete[] ibuf;
	return decompSize;
}

} // End of namespace Trecision
