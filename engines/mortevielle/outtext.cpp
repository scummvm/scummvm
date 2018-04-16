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

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/graphics.h"

#include "common/file.h"
#include "common/str.h"

namespace Mortevielle {

TextHandler::TextHandler(MortevielleEngine *vm) {
	_vm = vm;
}

/**
 * Next word
 * @remarks	Originally called 'l_motsuiv'
 */
int TextHandler::nextWord(int p, const char *ch, int &tab) {
	int c = p;

	while ((ch[p] != ' ') && (ch[p] != '$') && (ch[p] != '@'))
		++p;

	return tab * (p - c);
}

/**
 * Engine function - Display Text
 * @remarks	Originally called 'afftex'
 */
void TextHandler::displayStr(Common::String inputStr, int x, int y, int dx, int dy, int typ) {
	Common::String s;
	int i, j;

	// Safeguard: add $ just in case
	inputStr += '$';

	_vm->_screenSurface->putxy(x, y);
	int tab = 6;
	dx *= 6;
	dy *= 6;
	int xc = x;
	int yc = y;
	int xf = x + dx;
	int yf = y + dy;
	int p = 0;
	bool stringParsed = (inputStr[p] == '$');
	s = "";
	while (!stringParsed) {
		switch (inputStr[p]) {
		case '@':
			_vm->_screenSurface->drawString(s, typ);
			s = "";
			++p;
			xc = x;
			yc += 6;
			_vm->_screenSurface->putxy(xc, yc);
			break;
		case ' ':
			s += ' ';
			xc += tab;
			++p;
			if (nextWord(p, inputStr.c_str(), tab) + xc > xf) {
				_vm->_screenSurface->drawString(s, typ);
				s = "";
				xc = x;
				yc += 6;
				if (yc > yf) {
					while (!_vm->keyPressed())
						;
					i = y;
					do {
						j = x;
						do {
							_vm->_screenSurface->putxy(j, i);
							_vm->_screenSurface->drawString(" ", 0);
							j += 6;
						} while (j <= xf);
						i += 6;
					} while (i <= yf);
					yc = y;
				}
				_vm->_screenSurface->putxy(xc, yc);
			}
			break;
		case '$':
			stringParsed = true;
			_vm->_screenSurface->drawString(s, typ);
			break;
		default:
			s += inputStr[p];
			++p;
			xc += tab;
			break;
		}
	}
}

/**
 * Load DES (picture container) file
 * @remarks	Originally called 'chardes'
 */
void TextHandler::loadPictureFile(Common::String filename, Common::String altFilename, int32 skipSize, int length) {
	Common::File f;
	if (!f.open(filename)) {
		if (!f.open(altFilename))
			error("Missing file: Either %s or %s", filename.c_str(), altFilename.c_str());
	}
	// HACK: The original game contains a bug in the 2nd intro screen, in German DOS version.
	// The size specified in the fxx array is wrong (too short). In order to fix it, we are using
	// the value -1 to force a variable read length.
	if (length == -1)
		length = f.size() - skipSize;

	assert(skipSize + length <= f.size());

	free(_vm->_curPict);
	_vm->_curPict = (byte *)malloc(sizeof(byte) * length);
	f.seek(skipSize);
	f.read(_vm->_curPict, length);
	f.close();
}

/**
 * Load ANI file
 * @remarks	Originally called 'charani'
 */
void TextHandler::loadAniFile(Common::String filename, int32 skipSize, int length) {
	Common::File f;
	if (!f.open(filename))
		error("Missing file - %s", filename.c_str());

	assert(skipSize + length <= f.size());

	free(_vm->_curAnim);
	_vm->_curAnim = (byte *)malloc(sizeof(byte) * length);
	f.seek(skipSize);
	f.read(_vm->_curAnim, length);
	f.close();
}

void TextHandler::taffich() {
	static const byte tran1[] = { 121, 121, 138, 139, 120 };
	static const byte tran2[] = { 150, 150, 152, 152, 100, 110, 159, 100, 100 };

	int cx, drawingSize, npal;
	int32 drawingStartPos;

	int a = _vm->_caff;
	if ((a >= 153) && (a <= 161))
		a = tran2[a - 153];
	else if ((a >= 136) && (a <= 140))
		a = tran1[a - 136];
	int b = a;
	if (_vm->_maff == a)
		return;

	switch (a) {
	case 16:
		_vm->_coreVar._pctHintFound[9] = '*';
		_vm->_coreVar._availableQuestion[42] = '*';
		break;
	case 20:
		_vm->_coreVar._availableQuestion[39] = '*';
		if (_vm->_coreVar._availableQuestion[36] == '*') {
			_vm->_coreVar._pctHintFound[3] = '*';
			_vm->_coreVar._availableQuestion[38] = '*';
		}
		break;
	case 24:
		_vm->_coreVar._availableQuestion[37] = '*';
		break;
	case 30:
		_vm->_coreVar._availableQuestion[9] = '*';
		break;
	case 31: // Coat of arms
		_vm->_coreVar._pctHintFound[4] = '*';
		_vm->_coreVar._availableQuestion[35] = '*';
		break;
	case 118:
		_vm->_coreVar._availableQuestion[41] = '*';
		break;
	case 143:
		_vm->_coreVar._pctHintFound[1] = '*';
		break;
	case 150:
		_vm->_coreVar._availableQuestion[34] = '*';
		break;
	case 151:
		_vm->_coreVar._pctHintFound[2] = '*';
		break;
	default:
		break;
	}

	_vm->_destinationOk = true;
	_vm->_mouse->hideMouse();
	drawingStartPos = 0;
	Common::String filename, altFilename;

	if ((a != 50) && (a != 51)) {
		int m = a + 2000;

		if ((m > 2001) && (m < 2010))
			m = 2001;
		else if (m == 2011)
			m = 2010;
		if (a == 32)
			m = 2034;
		else if ((a == 17) && (_vm->_maff == 14))
			m = 2018;
		else if (a > 99) {
			if ((_vm->_is == 1) || (_vm->_is == 0))
				m = 2031;
			else
				m = 2032;
		}

		if ( ((a > 69) && (a < 80)) || (a == 30) || (a == 31) || (a == 144) || (a == 147) || (a == 149) )
			m = 2030;
		else if ( ((a < 27) && ( ((_vm->_maff > 69) && (!_vm->_coreVar._alreadyEnteredManor)) || (_vm->_maff > 99) )) || ((_vm->_maff > 29) && (_vm->_maff < 33)) )
			m = 2033;

		_vm->displayInterScreenMessage(m);
		_vm->_maff = a;

		if (a == 159)
			a = 86;
		else if (a > 140)
			a -= 67;
		else if (a > 137)
			a -= 66;
		else if (a > 99)
			a -= 64;
		else if (a > 69)
			a -= 42;
		else if (a > 29)
			a -= 5;
		else if (a == 26)
			a = 24;
		else if (a > 18)
			--a;
		npal = a;

		for (cx = 0; cx <= (a - 1); ++cx)
			drawingStartPos += _vm->_drawingSizeArr[cx];
		drawingSize = _vm->_drawingSizeArr[a];

		altFilename = filename = "DXX.mor";
	} else {
		filename = "DZZ.mor";
		altFilename = "DZZALL";

		if (a == 50) {
			// First intro screen
			drawingStartPos = 0;
			drawingSize = _vm->_drawingSizeArr[87];
		} else { // a == 51
			// Second intro screen
			drawingStartPos = _vm->_drawingSizeArr[87];
			// HACK: Force a variable size in order to fix the wrong size used by the German version
			drawingSize = -1;
		}
		_vm->_maff = a;
		npal = a + 37;
	}
	loadPictureFile(filename, altFilename, drawingStartPos, drawingSize);
	_vm->_numpal = npal;
	_vm->setPal(npal);

	if ((b < 15) || (b == 16) || (b == 17) || (b == 24) || (b == 26) || (b == 50)) {
		drawingStartPos = 0;
		if ((b < 15) || (b == 16) || (b == 17) || (b == 24) || (b == 26)) {
			if (b == 26)
				b = 18;
			else if (b == 24)
				b = 17;
			else if (b > 15)
				--b;
			for (cx = 0; cx <= (b - 1); ++cx)
				drawingStartPos += _vm->_drawingSizeArr[cx + 89];
			drawingSize = _vm->_drawingSizeArr[b + 89];
			filename = "AXX.mor";
		} else { // b == 50
			// CHECKME: the size of AZZ.mor is 1280 for the DOS version
			//          and 1260 for the Amiga version. Maybe the 20 bytes
			//          are a filler (to get 10 blocks of 128 bytes),
			//          or the size should be variable.
			drawingSize = 1260;
			filename = "AZZ.mor";
		}
		loadAniFile(filename, drawingStartPos, drawingSize);
	}
	_vm->_mouse->showMouse();
	if ((a < COAT_ARMS) && ((_vm->_maff < COAT_ARMS) || (_vm->_coreVar._currPlace == LANDING)) && (_vm->_currAction != _vm->_menu->_opcodeEnter)) {
		if ((a == ATTIC) || (a == CELLAR))
			_vm->displayAloneText();
		else if (!_vm->_outsideOnlyFl)
			_vm->getPresence(_vm->_coreVar._currPlace);
		_vm->_savedBitIndex =  0;
	}
}

} // End of namespace Mortevielle
