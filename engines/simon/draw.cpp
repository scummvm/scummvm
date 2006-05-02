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

#include "common/system.h"

#include "simon/simon.h"
#include "simon/intern.h"

namespace Simon {

byte *SimonEngine::getFrontBuf() {
	_dxSurfacePitch = _screenWidth;
	return _frontBuf;
}

byte *SimonEngine::getBackBuf() {
	_dxSurfacePitch = _screenWidth;
	return _useBackGround ? _backGroundBuf : _backBuf;
}

byte *SimonEngine::getBackGround() {
	_dxSurfacePitch = _screenWidth;
	return _backGroundBuf;
}

byte *SimonEngine::getScaleBuf() {
	_dxSurfacePitch = _screenWidth;
	return _scaleBuf;
}

void SimonEngine::animateSprites() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	const byte *vc_ptr_org = _vcPtr;
	uint16 params[5];							// parameters to vc10

	if (_paletteFlag == 2)
		_paletteFlag = 1;

	if (getGameType() == GType_FF && _scrollCount) {
		scrollEvent();
	}
	if (getGameType() == GType_SIMON2 && _scrollFlag) {
		scrollScreen();
	}

	if (getGameType() == GType_FF && getBitFlag(84)) {
		animateSpritesByY();
		return;
	}

	vsp = _vgaSprites;

	while (vsp->id != 0) {
		vsp->windowNum &= 0x7FFF;

		vpe = &_vgaBufferPointers[vsp->zoneNum];
		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		_curSfxFile = vpe->sfxFile;
		_windowNum = vsp->windowNum;
		_vgaCurSpriteId = vsp->id;
		_vgaCurSpritePriority = vsp->priority;

		params[0] = readUint16Wrapper(&vsp->image);
		params[1] = readUint16Wrapper(&vsp->palette);
		params[2] = readUint16Wrapper(&vsp->x);
		params[3] = readUint16Wrapper(&vsp->y);

		if (getGameType() == GType_SIMON1) {
			params[4] = READ_BE_UINT16(&vsp->flags);
		} else {
			*(byte *)(&params[4]) = (byte)vsp->flags;
		}

		_vcPtr = (const byte *)params;
		vc10_draw();

		vsp++;
	}

	if (_drawImagesDebug)
		memset(_backBuf, 0, _screenWidth * _screenHeight);

	_updateScreen = true;
	_vcPtr = vc_ptr_org;
}

void SimonEngine::animateSpritesDebug() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	const byte *vc_ptr_org = _vcPtr;
	uint16 params[5];							// parameters to vc10_draw

	if (_paletteFlag == 2)
		_paletteFlag = 1;

	vsp = _vgaSprites;
	while (vsp->id != 0) {
		vsp->windowNum &= 0x7FFF;

		vpe = &_vgaBufferPointers[vsp->zoneNum];
		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		_curSfxFile = vpe->sfxFile;
		_windowNum = vsp->windowNum;
		_vgaCurSpriteId = vsp->id;

		if (vsp->image)
			fprintf(_dumpFile, "id:%5d image:%3d base-color:%3d x:%3d y:%3d flags:%x\n",
							vsp->id, vsp->image, vsp->palette, vsp->x, vsp->y, vsp->flags);
		params[0] = readUint16Wrapper(&vsp->image);
		params[1] = readUint16Wrapper(&vsp->palette);
		params[2] = readUint16Wrapper(&vsp->x);
		params[3] = readUint16Wrapper(&vsp->y);
		params[4] = readUint16Wrapper(&vsp->flags);
		_vcPtr = (const byte *)params;
		vc10_draw();

		vsp++;
	}

	_updateScreen = true;
	_vcPtr = vc_ptr_org;
}

void SimonEngine::animateSpritesByY() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	const byte *vc_ptr_org = _vcPtr;
	uint16 params[5];							// parameters to vc10
	int16 spriteTable[180][2];
	
	byte *src;
	int height, slot, y;
	uint i, numSprites = 0;

	vsp = _vgaSprites;
	while (vsp->id != 0) {
		if (vsp->flags & kDFScaled) {
			y = vsp->y;
		} else if (vsp->flags & kDFMasked) {
			vpe = &_vgaBufferPointers[vsp->zoneNum];
			src = vpe->vgaFile2 + vsp->image * 8;
			height = READ_LE_UINT16(src + 4) & 0x7FFF;
			y = vsp->y + height;
		} else {
			y = vsp->priority;
		}

		spriteTable[numSprites][0] = y;
		spriteTable[numSprites][1] = numSprites;
		numSprites++;
		vsp++;
	}

	while(1) {
		y = spriteTable[0][0];
		slot = spriteTable[0][1];

		for (i = 0; i < numSprites; i++) {
			if (y >= spriteTable[i][0]) {
				y = spriteTable[i][0];
				slot = spriteTable[i][1];
			}
		}

		if (y == 9999)
			break;

		for (i = 0; i < numSprites; i++) {
			if (slot == spriteTable[i][1]) {
				spriteTable[i][0] = 9999;
				break;
			}
		}

		vsp = &_vgaSprites[slot];
		vsp->windowNum &= 0x7FFF;

		vpe = &_vgaBufferPointers[vsp->zoneNum];
		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		_curSfxFile = vpe->sfxFile;
		_windowNum = vsp->windowNum;
		_vgaCurSpriteId = vsp->id;
		_vgaCurSpritePriority = vsp->priority;

		params[0] = readUint16Wrapper(&vsp->image);
		params[1] = readUint16Wrapper(&vsp->palette);
		params[2] = readUint16Wrapper(&vsp->x);
		params[3] = readUint16Wrapper(&vsp->y);
		*(byte *)(&params[4]) = (byte)vsp->flags;

		_vcPtr = (const byte *)params;
		vc10_draw();
	}

	_updateScreen = true;
	_vcPtr = vc_ptr_org;
}

void SimonEngine::displayBoxStars() {
	HitArea *ha, *dha;
	uint count;
	uint y_, x_;
	byte *dst;
	uint b, color;

	_lockWord |= 0x8000;

	if (getGameType() == GType_SIMON2)
		color = 236;
	else
		color = 225;

	uint limit = (getGameType() == GType_SIMON2) ? 200 : 134;

	for (int i = 0; i < 5; i++) {
		ha = _hitAreas;
		count = ARRAYSIZE(_hitAreas);

		animateSprites();

		do {
			if (ha->id != 0 && ha->flags & kBFBoxInUse && !(ha->flags & kBFBoxDead)) {

				dha = _hitAreas;
				if (ha->flags & kBFTextBox) {
					while (dha != ha && dha->flags != ha->flags)
						++dha;
					if (dha != ha && dha->flags == ha->flags)
						continue;
				} else {
					dha = _hitAreas;
					while (dha != ha && dha->item_ptr != ha->item_ptr)
						++dha;
					if (dha != ha && dha->item_ptr == ha->item_ptr)
						continue;
				}

				if (ha->y >= limit || ((getGameType() == GType_SIMON2) && ha->y >= _boxStarHeight))
					continue;

				y_ = (ha->height / 2) - 4 + ha->y;

				x_ = (ha->width / 2) - 4 + ha->x - (_scrollX * 8);

				if (x_ >= 311)
					continue;

				dst = getBackBuf();

				dst += (((_dxSurfacePitch / 4) * y_) * 4) + x_;

				b = _dxSurfacePitch;
				dst[4] = color;
				dst[b+1] = color;
				dst[b+4] = color;
				dst[b+7] = color;
				b += _dxSurfacePitch;
				dst[b+2] = color;
				dst[b+4] = color;
				dst[b+6] = color;
				b += _dxSurfacePitch;
				dst[b+3] = color;
				dst[b+5] = color;
				b += _dxSurfacePitch;
				dst[b] = color;
				dst[b+1] = color;
				dst[b+2] = color;
				dst[b+6] = color;
				dst[b+7] = color;
				dst[b+8] = color;
				b += _dxSurfacePitch;
				dst[b+3] = color;
				dst[b+5] = color;
				b += _dxSurfacePitch;
				dst[b+2] = color;
				dst[b+4] = color;
				dst[b+6] = color;
				b += _dxSurfacePitch;
				dst[b+1] = color;
				dst[b+4] = color;
				dst[b+7] = color;
				b += _dxSurfacePitch;
				dst[b+4] = color;
			}
		} while (ha++, --count);

		dx_update_screen_and_palette();
		delay(100);
		animateSprites();
		dx_update_screen_and_palette();
		delay(100);
	}

	_lockWord &= ~0x8000;
}

void SimonEngine::scrollScreen() {
	byte *dst = getFrontBuf();
	const byte *src;
	uint x, y;

	if (_scrollXMax == 0) {
		uint screenSize = 8 * _screenWidth;
		if (_scrollFlag < 0) {
			memmove(dst + screenSize, dst, _scrollWidth * _screenHeight - screenSize);
		} else {
			memmove(dst, dst + screenSize, _scrollWidth * _screenHeight - screenSize);
		}

		y = _scrollY - 8;

		if (_scrollFlag > 0) {
			dst += _screenHeight * _screenWidth - screenSize;
			y += 488;
		}

		src = _scrollImage + y / 2;
		decodeRow(dst, src + readUint32Wrapper(src), _scrollWidth);

		_scrollY += _scrollFlag;
		vcWriteVar(250, _scrollY);
	} else {
		if (_scrollFlag < 0) {
			memmove(dst + 8, dst, _screenWidth * _scrollHeight - 8);
		} else {
			memmove(dst, dst + 8, _screenWidth * _scrollHeight - 8);
		}

		x = _scrollX;
		x -= (getGameType() == GType_FF) ? 8 : 1;

		if (_scrollFlag > 0) {
			dst += _screenWidth - 8;
			x += (getGameType() == GType_FF) ? 648 : 41;
		}

		if (getGameType() == GType_FF)
			src = _scrollImage + x / 2;
		else
			src = _scrollImage + x * 4;
		decodeColumn(dst, src + readUint32Wrapper(src), _scrollHeight);

		_scrollX += _scrollFlag;
		vcWriteVar(251, _scrollX);
	}

	memcpy(_backBuf, _frontBuf, _screenWidth * _screenHeight);
	memcpy(_backGroundBuf, _backBuf, _scrollHeight * _screenWidth);

	_scrollFlag = 0;
}

void SimonEngine::clearBackFromTop(uint lines) {
	memset(_backBuf, 0, lines * _screenWidth);
}

void SimonEngine::dx_clear_surfaces(uint num_lines) {
	memset(_backBuf, 0, num_lines * _screenWidth);

	_system->copyRectToScreen(_backBuf, _screenWidth, 0, 0, _screenWidth, num_lines);

	if (_useBackGround) {
		memset(_frontBuf, 0, num_lines * _screenWidth);
		memset(_backGroundBuf, 0, num_lines * _screenWidth);
	}
}

void SimonEngine::fillFrontFromBack(uint x, uint y, uint w, uint h) {
	uint offs = x + y * _screenWidth;
	byte *s = _backBuf + offs;
	byte *d = _frontBuf + offs;

	do {
		memcpy(d, s, w);
		d += _screenWidth;
		s += _screenWidth;
	} while (--h);
}

void SimonEngine::fillBackFromFront(uint x, uint y, uint w, uint h) {
	uint offs = x + y * _screenWidth;
	byte *s = _frontBuf + offs;
	byte *d = _backBuf + offs;

	do {
		memcpy(d, s, w);
		d += _screenWidth;
		s += _screenWidth;
	} while (--h);
}

void SimonEngine::fillBackGroundFromBack(uint lines) {
	memcpy(_backGroundBuf, _backBuf, lines * _screenWidth);
}

void SimonEngine::dx_update_screen_and_palette() {
	_numScreenUpdates++;

	if (_paletteColorCount == 0 && _paletteFlag == 1) {
		_paletteFlag = 0;
		if (memcmp(_palette, _paletteBackup, 1024) != 0) {
			memcpy(_paletteBackup, _palette, 1024);
			_system->setPalette(_palette, 0, 256);
		}
	}

	_system->copyRectToScreen(_backBuf, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	_system->updateScreen();

	memcpy(_backBuf, _frontBuf, _screenWidth * _screenHeight);

	if (getGameType() == GType_FF && _scrollFlag) {
		scrollScreen();
	}

	if (_paletteColorCount != 0) {
		if (getGameType() == GType_SIMON1 && _usePaletteDelay) {
			delay(100);
			_usePaletteDelay = false;
		}
		fastFadeIn();
	}
}

void SimonEngine::fastFadeIn() {
	if (_paletteColorCount & 0x8000) {
		slowFadeIn();
	} else {
		_paletteFlag = false;
		memcpy(_paletteBackup, _palette, 1024);
		_system->setPalette(_palette, 0, _paletteColorCount);
		_paletteColorCount = 0;
	}
}

void SimonEngine::slowFadeIn() {
	uint8 paletteTmp[768];
	uint8 *src, *dst;
	int c, p;

	_paletteColorCount &= 0x7fff;
	_paletteFlag = false;

	memcpy(_videoBuf1, _palette, 1024); // Difference
	memset(_videoBuf1, 0, 768);

	memcpy(_paletteBackup, _palette, 768);
	memcpy(paletteTmp, _palette, 768);

	for (c = 255; c > 0; c -= 4) {
	  	src = paletteTmp;
 		dst = _videoBuf1;

		for (p = _paletteColorCount; p !=0 ; p--) {
			if (*src >= c)
				*dst = *dst + 4;
			
			src++;
			dst++;
 		}
 		_system->setPalette(_videoBuf1, 0, _videoNumPalColors);
		if (_fade)
			_system->updateScreen();
 		delay(5);
 	}
	_paletteColorCount = 0;
}

} // End of namespace Simon
