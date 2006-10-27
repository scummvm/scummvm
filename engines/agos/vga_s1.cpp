/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

#include "agos/agos.h"
#include "agos/intern.h"
#include "agos/vga.h"

namespace AGOS {

void AGOSEngine::setupSimon1VideoOpcodes(VgaOpcodeProc *op) {
	setupCommonVideoOpcodes(op);

	op[11] = &AGOSEngine::vc11_clearPathFinder;
	op[17] = &AGOSEngine::vc17_setPathfinderItem;
	op[22] = &AGOSEngine::vc22_setPaletteNew;
	op[32] = &AGOSEngine::vc32_copyVar;
	op[37] = &AGOSEngine::vc37_addToSpriteY;
	op[48] = &AGOSEngine::vc48_setPathFinder;
	op[59] = &AGOSEngine::vc59_ifSpeech;
	op[60] = &AGOSEngine::vc60_stopAnimation;
	op[61] = &AGOSEngine::vc61_setMaskImage;
	op[62] = &AGOSEngine::vc62_fastFadeOut;
	op[63] = &AGOSEngine::vc63_fastFadeIn;
}

void AGOSEngine::vc11_clearPathFinder() {
	memset(&_pathFindArray, 0, sizeof(_pathFindArray));
}

void AGOSEngine::vc17_setPathfinderItem() {
	uint16 a = vcReadNextWord();
	_pathFindArray[a - 1] = (const uint16 *)_vcPtr;

	int end = (getGameType() == GType_FF || getGameType() == GType_PP) ? 9999 : 999;
	while (readUint16Wrapper(_vcPtr) != end)
		_vcPtr += 4;
	_vcPtr += 2;
}

void AGOSEngine::vc22_setPaletteNew() {
	byte *offs, *palptr, *src;
	uint16 a = 0, b, num, palSize;

	a = vcReadNextWord();
	b = vcReadNextWord();

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		num = 256;
		palSize = 768;

		palptr = _displayPalette;
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		num = a == 0 ? 32 : 16;
		palSize = 96;

		palptr = &_displayPalette[(a * 64)];
	}

	offs = _curVgaFile1 + 6;
	src = offs + b * palSize;

	do {
		palptr[0] = src[0] * 4;
		palptr[1] = src[1] * 4;
		palptr[2] = src[2] * 4;
		palptr[3] = 0;

		palptr += 4;
		src += 3;
	} while (--num);

	_paletteFlag = 2;
	_vgaSpriteChanged++;
}

void AGOSEngine::vc32_copyVar() {
	uint16 a = vcReadVar(vcReadNextWord());
	vcWriteVar(vcReadNextWord(), a);
}

void AGOSEngine::vc37_addToSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y += vcReadVar(vcReadNextWord());
	_vgaSpriteChanged++;
}

void AGOSEngine::vc45_setSpriteX() {
	VgaSprite *vsp = findCurSprite();
	vsp->x = vcReadVar(vcReadNextWord());
	_vgaSpriteChanged++;
}

void AGOSEngine::vc46_setSpriteY() {
	VgaSprite *vsp = findCurSprite();
	vsp->y = vcReadVar(vcReadNextWord());
	_vgaSpriteChanged++;
}

void AGOSEngine::vc47_addToVar() {
	uint16 var = vcReadNextWord();
	vcWriteVar(var, vcReadVar(var) + vcReadVar(vcReadNextWord()));
}

void AGOSEngine::vc48_setPathFinder() {
	uint16 a = (uint16)_variableArrayPtr[12];
	const uint16 *p = _pathFindArray[a - 1];

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		VgaSprite *vsp = findCurSprite();
		int16 x, y, ydiff;
		int16 x1, y1, x2, y2;
		uint pos = 0;

		x = vsp->x;
		while (x >= (int16)readUint16Wrapper(p + 2)) {
			p += 2;
			pos++;
		}

		x1 = readUint16Wrapper(p);
		y1 = readUint16Wrapper(p + 1);
		x2 = readUint16Wrapper(p + 2);
		y2 = readUint16Wrapper(p + 3);

		if (x2 != 9999) {
			ydiff = y2 - y1;
			if (ydiff < 0) {
				ydiff = -ydiff;
				x = vsp->x & 7;
				ydiff *= x;
				ydiff /= 8;
				ydiff = -ydiff;	
			} else {
				x = vsp->x & 7;
				ydiff *= x;
				ydiff /= 8;
			}
			y1 += ydiff;
		}

		y = vsp->y;
		vsp->y = y1;
		checkScrollY(y1 - y, y1);

		_variableArrayPtr[11] = x1;
		_variableArrayPtr[13] = pos;
	} else {
		uint b = (uint16)_variableArray[13];
		p += b * 2 + 1;
		int c = _variableArray[14];

		int step;
		int y1, y2;
		int16 *vp;

		step = 2;
		if (c < 0) {
			c = -c;
			step = -2;
		}

		vp = &_variableArray[20];

		do {
			y2 = readUint16Wrapper(p);
			p += step;
			y1 = readUint16Wrapper(p) - y2;

			vp[0] = y1 / 2;
			vp[1] = y1 - (y1 / 2);

			vp += 2;
		} while (--c);
	}
}

void AGOSEngine::vc59_ifSpeech() {
	if (!_sound->isVoiceActive())
		vcSkipNextInstruction();
}

void AGOSEngine::vc61_setMaskImage() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadVarOrWord();
	vsp->x += vcReadNextWord();
	vsp->y += vcReadNextWord();
	vsp->flags = kDFMasked | kDFUseFrontBuf;

	_vgaSpriteChanged++;
}

} // End of namespace AGOS
