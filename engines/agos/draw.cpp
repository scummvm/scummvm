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

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

byte *AGOSEngine::getFrontBuf() {
	if (getGameType() != GType_PP && getGameType() != GType_FF)
		_updateScreen = true;

	_dxSurfacePitch = _screenWidth;
	return _frontBuf;
}

byte *AGOSEngine::getBackBuf() {
	_dxSurfacePitch = _screenWidth;
	return _useBackGround ? _backGroundBuf : _backBuf;
}

byte *AGOSEngine::getBackGround() {
	_dxSurfacePitch = _screenWidth;
	return _backGroundBuf;
}

byte *AGOSEngine::getScaleBuf() {
	_dxSurfacePitch = _screenWidth;
	return _scaleBuf;
}

void AGOSEngine_Feeble::animateSpritesByY() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	int16 spriteTable[180][2];
	
	byte *src;
	int height, slot, y;
	uint i, numSprites = 0;

	vsp = _vgaSprites;
	while (vsp->id) {
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

	while (1) {
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

		drawImage_init(vsp->image, vsp->palette, vsp->x, vsp->y, vsp->flags);
	}

	_displayScreen = true;
}

void AGOSEngine_Feeble::animateSprites() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;

	if (_paletteFlag == 2)
		_paletteFlag = 1;

	if (_scrollCount) {
		scrollEvent();
	}

	if (getBitFlag(84)) {
		animateSpritesByY();
		return;
	}

	vsp = _vgaSprites;
	while (vsp->id) {
		vsp->windowNum &= 0x7FFF;

		vpe = &_vgaBufferPointers[vsp->zoneNum];
		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		_curSfxFile = vpe->sfxFile;
		_windowNum = vsp->windowNum;
		_vgaCurSpriteId = vsp->id;
		_vgaCurSpritePriority = vsp->priority;

		drawImage_init(vsp->image, vsp->palette, vsp->x, vsp->y, vsp->flags);
		vsp++;
	}

	_displayScreen = true;
}

void AGOSEngine::animateSprites() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;

	if (_copyScnFlag) {
		_copyScnFlag--;
		_vgaSpriteChanged++;
	}

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
		const uint8 var = (getGameType() == GType_ELVIRA1) ? 293 : 71;
		if (_wallOn && !_variableArray[var]) {
			_wallOn--;

			VC10_state state;
			state.srcPtr  = getBackGround() + 3 * _screenWidth + 3 * 16;
			state.height = state.draw_height = 127;
			state.width = state.draw_width = 14;
			state.y = 0;
			state.x = 0;
			state.palette = 0;
			state.paletteMod = 0;
			state.flags = kDFNonTrans;

			_windowNum = 4;

			_backFlag = 1;
			drawImage(&state);
			_backFlag = 0;

			_vgaSpriteChanged++;
		}
	}

	if (!_scrollFlag && !_vgaSpriteChanged) {
		return;
	}

	_vgaSpriteChanged = 0;

	if (_paletteFlag == 2)
		_paletteFlag = 1;

	if (getGameType() == GType_SIMON2 && _scrollFlag) {
		scrollScreen();
	}

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		dirtyClips();
	}

	restoreBackGround();

	vsp = _vgaSprites;
	while (vsp->id) {
		vsp->windowNum &= 0x7FFF;

		vpe = &_vgaBufferPointers[vsp->zoneNum];
		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		_curSfxFile = vpe->sfxFile;
		_windowNum = vsp->windowNum;
		_vgaCurSpriteId = vsp->id;
		_vgaCurSpritePriority = vsp->priority;

		saveBackGround(vsp);

		drawImage_init(vsp->image, vsp->palette, vsp->x, vsp->y, vsp->flags);
		vsp++;
	}

	if (getGameType() == GType_ELVIRA1 && _variableArray[293]) {
		// Used by the Fire Wall and Ice Wall spells
		debug(0, "Using special wall");

		uint8 color, h, len;
		byte *dst = _window4BackScn;

		color = (_variableArray[293] & 1) ? 13 : 15;
		_wallOn = 2;

		h = 127;
		while (h) {
			len = 112;
			while (len--) {
				*dst++ = color;
				dst++;
			}

			h--;
			if (h == 0)
				break;

			len = 112;
			while (len--) {
				dst++;
				*dst++ = color;
			}
			h--;
		}			

		_window4Flag = 1;
		setMoveRect(0, 0, 224, 127);
	} else if (getGameType() == GType_ELVIRA2 && _variableArray[71] & 2) {
		// Used by the Unholy Barrier spell
		uint8 color, h, len;
		byte *dst = _window4BackScn;

		color = 1;
		_wallOn = 2;

		h = 43;
		while (h) {
			len = 56;
			while (len--) {
				*dst++ = color;
				dst += 3;
			}

			h--;
			if (h == 0)
				break;

			dst += 448;

			len = 56;
			while (len--) {
				dst += 2;
				*dst++ = color;
				dst += 1;
			}
			dst += 448;
			h--;
		}			

		_window4Flag = 1;
		setMoveRect(0, 0, 224, 127);
	}

	if (_window6Flag == 1)
		_window6Flag++;

	if (_window4Flag == 1)
		_window4Flag++;

	_displayScreen = true;
}

void AGOSEngine::dirtyClips() {
	// TODO
}

void AGOSEngine::restoreBackGround() {
	if (getGameType() == GType_ELVIRA1 && getPlatform() == Common::kPlatformAtariST &&
		(getFeatures() & GF_DEMO)) {
		return;
	}

	AnimTable *animTable;
	uint images = 0;

	animTable = _screenAnim1;
	while (animTable->srcPtr) {
		animTable++;
		images++;
	}

	while (images--) {
		animTable--;

		if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
			!(animTable->window & 0x8000)) {
			//continue;
		}

		animTable->window &= 0x7FFF;
		_windowNum = animTable->window;	

		VC10_state state;
		state.srcPtr  = animTable->srcPtr;
		state.height = state.draw_height = animTable->height;
		state.width = state.draw_width = animTable->width;
		state.y = animTable->y;
		state.x = animTable->x;
		state.palette = 0;
		state.paletteMod = 0;
		state.flags = kDFNonTrans;

		_backFlag = 1;
		drawImage(&state);

		//if (getGameType() != GType_SIMON1 && getGameType() != GType_SIMON2) {
			animTable->srcPtr = 0;
		//}
	}
	_backFlag = 0;

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		AnimTable *animTableTmp;

		animTable = animTableTmp = _screenAnim1;
		while (animTable->srcPtr) {
			if (!(animTable->window & 8000)) {
				memcpy(animTableTmp, animTable, sizeof(AnimTable));
				animTableTmp++;
			}
			animTable++;
		}
		animTableTmp->srcPtr = 0;
	}
}

void AGOSEngine::saveBackGround(VgaSprite *vsp) {
	if (getGameType() == GType_ELVIRA1 && getPlatform() == Common::kPlatformAtariST &&
		(getFeatures() & GF_DEMO)) {
		return;
	}

	if ((vsp->flags & kDFSkipStoreBG) || !vsp->image)
		return;

	AnimTable *animTable = _screenAnim1;

	while (animTable->srcPtr)
		animTable++;

	const byte *ptr = _curVgaFile2 + vsp->image * 8;
	int16 x = vsp->x - _scrollX;
	int16 y = vsp->y - _scrollY;

	if (_window3Flag == 1) {
		animTable->srcPtr = (const byte *)_window4BackScn;
	} else {
		uint xoffs = (_videoWindows[vsp->windowNum * 4 + 0] * 2 + x) * 8;
		uint yoffs = (_videoWindows[vsp->windowNum * 4 + 1] + y);
		animTable->srcPtr = getBackGround() + xoffs + yoffs * _screenWidth;
	}

	animTable->x = x;
	animTable->y = y;

	animTable->width = READ_BE_UINT16(ptr + 6) / 16;
	if (vsp->flags & 40) {
		animTable->width++;
	}

	animTable->height = ptr[5];
	animTable->window = vsp->windowNum;
	animTable->id = vsp->id;
}

void AGOSEngine::displayBoxStars() {
	HitArea *ha, *dha;
	uint count;
	uint y_, x_;
	byte *dst;
	uint b, color;

	o_haltAnimation();

	if (getGameType() == GType_SIMON2)
		color = 236;
	else
		color = 225;

	uint curHeight = (getGameType() == GType_SIMON2) ? _boxStarHeight : 134;

	for (int i = 0; i < 5; i++) {
		ha = _hitAreas;
		count = ARRAYSIZE(_hitAreas);

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

				if (ha->y >= curHeight)
					continue;

				y_ = (ha->height / 2) - 4 + ha->y;

				x_ = (ha->width / 2) - 4 + ha->x - (_scrollX * 8);

				if (x_ >= 311)
					continue;

				dst = getFrontBuf();

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

		delay(100);

		setMoveRect(0, 0, 320, curHeight);
		_window4Flag = 2;

		displayScreen();
		delay(100);
	}

	o_restartAnimation();
}

void AGOSEngine::scrollScreen() {
	byte *dst;
	const byte *src;
	uint x, y;

	if (getGameType() == GType_SIMON2) {
		dst = getBackGround();
	} else {
		dst = getFrontBuf();
	}

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

		memcpy(_backBuf, _frontBuf, _screenWidth * _screenHeight);
		memcpy(_backGroundBuf, _backBuf, _screenHeight * _scrollWidth);
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

		if (getGameType() == GType_SIMON2) {
			memcpy(_window4BackScn, _backGroundBuf, _scrollHeight * _screenWidth);
		} else {
			memcpy(_backBuf, _frontBuf, _screenWidth * _screenHeight);
			memcpy(_backGroundBuf, _backBuf, _scrollHeight * _screenWidth);
		}

		setMoveRect(0, 0, 320, _scrollHeight);

		_window4Flag = 1;
	}

	_scrollFlag = 0;
}

void AGOSEngine::clearBackFromTop(uint lines) {
	memset(_backBuf, 0, lines * _screenWidth);
}

void AGOSEngine::clearSurfaces(uint num_lines) {
	memset(_backBuf, 0, num_lines * _screenWidth);

	_system->copyRectToScreen(_backBuf, _screenWidth, 0, 0, _screenWidth, num_lines);

	if (_useBackGround) {
		memset(_frontBuf, 0, num_lines * _screenWidth);
		memset(_backGroundBuf, 0, num_lines * _screenWidth);
	}
}

void AGOSEngine::fillFrontFromBack(uint x, uint y, uint w, uint h) {
	uint offs = x + y * _screenWidth;
	byte *s = _backBuf + offs;
	byte *d = _frontBuf + offs;

	do {
		memcpy(d, s, w);
		d += _screenWidth;
		s += _screenWidth;
	} while (--h);
}

void AGOSEngine::fillBackFromFront(uint x, uint y, uint w, uint h) {
	uint offs = x + y * _screenWidth;
	byte *s = _frontBuf + offs;
	byte *d = _backBuf + offs;

	do {
		memcpy(d, s, w);
		d += _screenWidth;
		s += _screenWidth;
	} while (--h);
}

void AGOSEngine::fillBackGroundFromBack(uint lines) {
	memcpy(_backGroundBuf, _backBuf, lines * _screenWidth);
}

void AGOSEngine::setMoveRect(uint16 x, uint16 y, uint16 width, uint16 height) {
	if (x < _moveXMin)
		_moveXMin = x;

	if (y < _moveYMin)
		_moveYMin = y;

	if (width > _moveXMax)
		_moveXMax = width;

	if (height > _moveYMax)
		_moveYMax = height;
}

void AGOSEngine::displayScreen() {
	if (_fastFadeInFlag == 0 && _paletteFlag == 1) {
		_paletteFlag = 0;
		if (memcmp(_displayPalette, _currentPalette, 1024)) {
			memcpy(_currentPalette, _displayPalette, 1024);
			_system->setPalette(_displayPalette, 0, 256);
		}
	}

	if (getGameType() == GType_PP || getGameType() == GType_FF) {
		_system->copyRectToScreen(getBackBuf(), _screenWidth, 0, 0, _screenWidth, _screenHeight);
		_system->updateScreen();

		if (getGameId() != GID_DIMP)
			memcpy(getBackBuf(), getFrontBuf(), _screenWidth * _screenHeight);
	} else {
		if (_window4Flag == 2) {
			_window4Flag = 0;

			uint16 srcWidth, width, height;
			byte *dst = getFrontBuf();

			const byte *src = _window4BackScn;
			if (_window3Flag == 1) {
				src = getBackGround();
			}

			dst += (_moveYMin + _videoWindows[17]) * _screenWidth;
			dst += (_videoWindows[16] * 16) + _moveXMin;

			src += (_videoWindows[18] * 16 * _moveYMin);
			src += _moveXMin;

			srcWidth = _videoWindows[18] * 16;
		
			width = _moveXMax - _moveXMin;
			height = _moveYMax - _moveYMin;

			for (; height > 0; height--) {
				memcpy(dst, src, width);
				dst += _screenWidth;
				src += srcWidth;
			}

			_moveXMin = 0xFFFF;
			_moveYMin = 0xFFFF;
			_moveXMax = 0;
			_moveYMax = 0;
		}
		
		if (_window6Flag == 2) {
			_window6Flag = 0;

			byte *src = _window6BackScn;
			byte *dst = getFrontBuf() + 16320;
			for (int i = 0; i < 80; i++) {
				memcpy(dst, src, 48);
				dst += _screenWidth;
				src += 48;
			}
		}

		_system->copyRectToScreen(getFrontBuf(), _screenWidth, 0, 0, _screenWidth, _screenHeight);
		_system->updateScreen();
	}

	if (getGameType() == GType_FF && _scrollFlag) {
		scrollScreen();
	}

	if (_fastFadeInFlag) {
		fastFadeIn();
	}
}

void AGOSEngine::fastFadeIn() {
	if (_fastFadeInFlag & 0x8000) {
		slowFadeIn();
	} else {
		_paletteFlag = false;
		memcpy(_currentPalette, _displayPalette, 1024);
		_system->setPalette(_displayPalette, 0, _fastFadeInFlag);
		_fastFadeInFlag = 0;
	}
}

void AGOSEngine::slowFadeIn() {
	uint8 *src, *dst;
	int c, p;

	_fastFadeInFlag &= 0x7fff;
	_paletteFlag = false;

	memset(_videoBuf1, 0, 1024);
	memcpy(_currentPalette, _displayPalette, 1024);
	memcpy(_videoBuf1 + 1024, _displayPalette, 1024);

	for (c = 255; c >= 0; c -= 4) {
	  	src = _videoBuf1 + 1024;
 		dst = _videoBuf1;

		for (p = _fastFadeInFlag; p !=0 ; p -= 3) {
			if (src[0] >= c)
				dst[0] += 4;
			if (src[1] >= c)
				dst[1] += 4;
			if (src[2] >= c)
				dst[2] += 4;
			src += 4;
			dst += 4;
 		}
 		_system->setPalette(_videoBuf1, 0, _fastFadeCount);
 		delay(5);
 	}
	_fastFadeInFlag = 0;
}

} // End of namespace AGOS
