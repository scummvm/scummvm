/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/draw.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/game.h"
#include "gob/inter.h"
#include "gob/video.h"

namespace Gob {

Draw::Draw(GobEngine *vm) : _vm(vm) {
	_renderFlags = 0;

	_fontIndex = 0;
	_spriteLeft = 0;
	_spriteTop = 0;
	_spriteRight = 0;
	_spriteBottom = 0;
	_destSpriteX = 0;
	_destSpriteY = 0;
	_backColor = 0;
	_frontColor = 0;
	_transparency = 0;

	_sourceSurface = 0;
	_destSurface = 0;

	_letterToPrint = 0;
	_textToPrint = 0;

	_backDeltaX = 0;
	_backDeltaY = 0;

	for (int i = 0; i < 8; i++)
		_fonts[i] = 0;
	
	for (int i = 0; i < SPRITES_COUNT; i++)
		_spritesArray[i] = 0;

	_invalidatedCount = 0;
	for (int i = 0; i < 30; i++) {
		_invalidatedTops[i] = 0;
		_invalidatedLefts[i] = 0;
		_invalidatedRights[i] = 0;
		_invalidatedBottoms[i] = 0;
	}

	_noInvalidated = false;
	_noInvalidated57 = false;
	_paletteCleared = false;
	_applyPal = false;

	_backSurface = 0;
	_frontSurface = 0;

	for (int i = 0; i < 18; i++)
		_unusedPalette1[i] = 0;
	for (int i = 0; i < 16; i++)
		_unusedPalette2[i] = 0;
	for (int i = 0; i < 256; i++) {
		_vgaPalette[i].red = 0;
		_vgaPalette[i].blue = 0;
		_vgaPalette[i].green = 0;
	}
	for (int i = 0; i < 16; i++) {
		_vgaSmallPalette[i].red = 0;
		_vgaSmallPalette[i].blue = 0;
		_vgaSmallPalette[i].green = 0;
	}

	_showCursor = 0;
	_cursorIndex = 0;
	_transparentCursor = 0;
	_cursorTimeKey = 0;

	_cursorX = 0;
	_cursorY = 0;
	_cursorWidth = 0;
	_cursorHeight = 0;

	_cursorHotspotXVar = -1;
	_cursorHotspotYVar = -1;

	_cursorSprites = 0;
	_cursorSpritesBack = 0;
	_scummvmCursor = 0;

	_cursorAnim = 0;
	for (int i = 0; i < 40; i++) {
		_cursorAnimLow[i] = 0;
		_cursorAnimHigh[i] = 0;
		_cursorAnimDelays[i] = 0;
	}

	_palLoadData1[0] = 0;
	_palLoadData1[1] = 17;
	_palLoadData1[2] = 34;
	_palLoadData1[3] = 51;
	_palLoadData2[0] = 0;
	_palLoadData2[1] = 68;
	_palLoadData2[2] = 136;
	_palLoadData2[3] = 204;

	_needAdjust = 2;
	_scrollOffsetX = 0;
	_scrollOffsetY = 0;
}

void Draw::invalidateRect(int16 left, int16 top, int16 right, int16 bottom) {
	if (_renderFlags & RENDERFLAG_NOINVALIDATE)
		return;

	if (left > right)
		SWAP(left, right);
	if (top > bottom)
		SWAP(top, bottom);

	if ((left > (_vm->_video->_surfWidth - 1)) || (right < 0) ||
	    (top > (_vm->_video->_surfHeight - 1)) || (bottom < 0))
		return;

	_noInvalidated = false;

	if (_invalidatedCount >= 30) {
		_invalidatedLefts[0] = 0;
		_invalidatedTops[0] = 0;
		_invalidatedRights[0] = _vm->_video->_surfWidth - 1;
		_invalidatedBottoms[0] = _vm->_video->_surfHeight - 1;
		_invalidatedCount = 1;
		return;
	}

	if (left < 0)
		left = 0;

	if (right > (_vm->_video->_surfWidth - 1))
		right = _vm->_video->_surfWidth - 1;

	if (top < 0)
		top = 0;

	if (bottom > (_vm->_video->_surfHeight - 1))
		bottom = _vm->_video->_surfHeight - 1;

	left &= 0xFFF0;
	right |= 0x000F;

	for (int rect = 0; rect < _invalidatedCount; rect++) {

		if (_invalidatedTops[rect] > top) {
			if (_invalidatedTops[rect] > bottom) {
				for (int i = _invalidatedCount; i > rect; i--) {
					_invalidatedLefts[i] = _invalidatedLefts[i - 1];
					_invalidatedTops[i] = _invalidatedTops[i - 1];
					_invalidatedRights[i] = _invalidatedRights[i - 1];
					_invalidatedBottoms[i] = _invalidatedBottoms[i - 1];
				}
				_invalidatedLefts[rect] = left;
				_invalidatedTops[rect] = top;
				_invalidatedRights[rect] = right;
				_invalidatedBottoms[rect] = bottom;
				_invalidatedCount++;
				return;
			}
			if (_invalidatedBottoms[rect] < bottom)
				_invalidatedBottoms[rect] = bottom;

			if (_invalidatedLefts[rect] > left)
				_invalidatedLefts[rect] = left;

			if (_invalidatedRights[rect] < right)
				_invalidatedRights[rect] = right;

			_invalidatedTops[rect] = top;
			return;
		}

		if (_invalidatedBottoms[rect] < top)
			continue;

		if (_invalidatedBottoms[rect] < bottom)
			_invalidatedBottoms[rect] = bottom;

		if (_invalidatedLefts[rect] > left)
			_invalidatedLefts[rect] = left;

		if (_invalidatedRights[rect] < right)
			_invalidatedRights[rect] = right;

		return;
	}

	_invalidatedLefts[_invalidatedCount] = left;
	_invalidatedTops[_invalidatedCount] = top;
	_invalidatedRights[_invalidatedCount] = right;
	_invalidatedBottoms[_invalidatedCount] = bottom;
	_invalidatedCount++;
}

void Draw::blitInvalidated() {
	if (_noInvalidated57 &&
			((_vm->_global->_videoMode == 5) || (_vm->_global->_videoMode == 7)))
		return;

	if (_cursorIndex == 4)
		blitCursor();

	if (_vm->_inter->_terminate)
		return;

	if (_noInvalidated && !_applyPal)
		return;

	if (_noInvalidated) {
		setPalette();
		_applyPal = false;
		return;
	}

	_showCursor = (_showCursor & ~2) | ((_showCursor & 1) << 1);
	if (_applyPal) {
		clearPalette();

		_vm->_video->drawSprite(_backSurface, _frontSurface,
				0, 0, _vm->_video->_surfWidth - 1,
				_vm->_video->_surfHeight - 1, 0, 0, 0);
		setPalette();
		_invalidatedCount = 0;
		_noInvalidated = true;
		_applyPal = false;
		return;
	}

	_vm->_video->_doRangeClamp = false;
	for (int i = 0; i < _invalidatedCount; i++) {
		_vm->_video->drawSprite(_backSurface, _frontSurface,
		    _invalidatedLefts[i], _invalidatedTops[i],
		    _invalidatedRights[i], _invalidatedBottoms[i],
		    _invalidatedLefts[i], _invalidatedTops[i], 0);
	}
	_vm->_video->_doRangeClamp = true;

	_invalidatedCount = 0;
	_noInvalidated = true;
	_applyPal = false;
}

void Draw::setPalette() {
	_vm->validateVideoMode(_vm->_global->_videoMode);

	_vm->_global->_pPaletteDesc->unused1 = _unusedPalette1;
	_vm->_global->_pPaletteDesc->unused2 = _unusedPalette2;
	_vm->_global->_pPaletteDesc->vgaPal = _vgaPalette;
	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	_paletteCleared = false;
}

void Draw::clearPalette() {
	if (!_paletteCleared) {
		_vm->_util->clearPalette();
		_paletteCleared = true;
	}
}

void Draw::initSpriteSurf(int16 index, int16 width, int16 height,
		int16 flags) {

	_spritesArray[index] =
		_vm->_video->initSurfDesc(_vm->_global->_videoMode, width, height, flags);
	_vm->_video->clearSurf(_spritesArray[index]);
}

void Draw::adjustCoords(char adjust, int16 *coord1, int16 *coord2) {
	if (_needAdjust == 2)
		return;

	switch (adjust) {
		case 0:
			if (coord2)
				*coord2 *= 2;
			if (coord1)
				*coord2 *= 2;
			break;

		case 1:
			if (coord2)
				*coord2 = (signed) ((unsigned) (*coord2 + 1) / 2);
			if (coord1)
				*coord1 = (signed) ((unsigned) (*coord1 + 1) / 2);
			break;

		case 2:
			if (coord2)
				*coord2 = *coord2 * 2 + 1;
			if (coord1)
				*coord1 = *coord1 * 2 + 1;
			break;
	}
}

void Draw::drawString(char *str, int16 x, int16 y, int16 color1, int16 color2,
		int16 transp, SurfaceDesc *dest, Video::FontDesc *font) {

	while (*str != '\0') {
		_vm->_video->drawLetter(*str, x, y, font, transp, color1, color2, dest);
		if (!font->extraData)
			x += font->itemWidth;
		else
			x += *(((char *)font->extraData) + (*str - font->startItem));
		str++;
	}
}

void Draw::printTextCentered(int16 id, int16 left, int16 top, int16 right,
		int16 bottom, char *str, int16 fontIndex, int16 color) {

	adjustCoords(1, &left, &top);
	adjustCoords(1, &right, &bottom);

	if (READ_LE_UINT16(_vm->_game->_totFileData + 0x7E) != 0) {
		char *storedIP = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr = _vm->_game->_totFileData +
			READ_LE_UINT16(_vm->_game->_totFileData + 0x7E);
		WRITE_VAR(17, (uint32) id);
		WRITE_VAR(18, (uint32) left);
		WRITE_VAR(19, (uint32) top);
		WRITE_VAR(20, (uint32) (right - left + 1));
		WRITE_VAR(21, (uint32) (bottom - top + 1));
		_vm->_inter->funcBlock(0);
		_vm->_global->_inter_execPtr = storedIP;
	}
	
	if (str[0] == '\0')
		return;

	int16 width = 0;

	_transparency = 1;
	_destSpriteX = left;
	_destSpriteY = top;
	_fontIndex = fontIndex;
	_frontColor = color;
	_textToPrint = str;
	if (_fonts[fontIndex]->extraData != 0) {
		char *data = (char *) _fonts[fontIndex]->extraData;
		int length = strlen(str);

		for (int i = 0; i < length; i++)
			width += *(data + (str[i] - _fonts[_fontIndex]->startItem));
	}
	else
		width = strlen(str) * _fonts[fontIndex]->itemWidth;

	adjustCoords(1, &width, 0);
	_destSpriteX += (right - left + 1 - width) / 2;

	spriteOperation(DRAW_PRINTTEXT);
}

int32 Draw::getSpriteRectSize(int16 index) {
	if (!_spritesArray[index])
		return 0;

	return _spritesArray[index]->getWidth() * _spritesArray[index]->getHeight();
}

} // End of namespace Gob
