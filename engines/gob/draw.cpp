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
#include "gob/video.h"
#include "gob/game.h"
#include "gob/util.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/video.h"
#include "gob/palanim.h"
#include "gob/cdrom.h"

namespace Gob {

Draw::Draw(GobEngine *vm) : _vm(vm) {
	int i;
	int j;

	_fontIndex = 0;
	_spriteLeft = 0;
	_spriteTop = 0;
	_spriteRight = 0;
	_spriteBottom = 0;
	_destSpriteX = 0;
	_destSpriteY = 0;
	_backColor = 0;
	_frontColor = 0;
	_letterToPrint = 0;

	_destSurface = 0;
	_sourceSurface = 0;
	_renderFlags = 0;
	_backDeltaX = 0;
	_backDeltaY = 0;

	for (i = 0; i < 8; i++)
		_fonts[i] = 0;
	
	_textToPrint = 0;
	_transparency = 0;

	for (i = 0; i < 50; i++) {
		_spritesArray[i] = 0;
		_spritesHeights[i] = 0;
		for (j = 0; j < 3; j++) {
			_bigSpritesParts[i][j] = 0;
		}
	}

	_invalidatedCount = 0;
	for (i = 0; i < 30; i++) {
		_invalidatedTops[i] = 0;
		_invalidatedLefts[i] = 0;
		_invalidatedRights[i] = 0;
		_invalidatedBottoms[i] = 0;
	}

	_noInvalidated = 0;
	_applyPal = 0;
	_paletteCleared = 0;

	_backSurface = 0;
	_frontSurface = 0;

	for (i = 0; i < 18; i++)
		_unusedPalette1[i] = 0;
	for (i = 0; i < 16; i++)
		_unusedPalette2[i] = 0;
	for (i = 0; i < 256; i++) {
		_vgaPalette[i].red = 0;
		_vgaPalette[i].blue = 0;
		_vgaPalette[i].green = 0;
	}
	for (i = 0; i < 16; i++) {
		_vgaSmallPalette[i].red = 0;
		_vgaSmallPalette[i].blue = 0;
		_vgaSmallPalette[i].green = 0;
	}

	_cursorX = 0;
	_cursorY = 0;
	_cursorWidth = 0;
	_cursorHeight = 0;

	_cursorXDeltaVar = -1;
	_cursorYDeltaVar = -1;

	for (i = 0; i < 40; i++) {
		_cursorAnimLow[i] = 0;
		_cursorAnimHigh[i] = 0;
		_cursorAnimDelays[i] = 0;
	}

	_cursorIndex = 0;
	_transparentCursor = 0;
	_cursorSprites = 0;
	_cursorBack = 0;
	_cursorAnim = 0;

	_palLoadData1[0] = 0;
	_palLoadData1[1] = 17;
	_palLoadData1[2] = 34;
	_palLoadData1[3] = 51;
	_palLoadData2[0] = 0;
	_palLoadData2[1] = 68;
	_palLoadData2[2] = 136;
	_palLoadData2[3] = 204;

	_cursorTimeKey = 0;

	warning("GOB2 Stub! _word_2E8E2");
	_word_2E8E2 = 2;
}

void Draw::invalidateRect(int16 left, int16 top, int16 right, int16 bottom) {
	int16 temp;
	int16 rect;
	int16 i;

	if (_renderFlags & RENDERFLAG_NOINVALIDATE)
		return;

	if (left > right) {
		temp = left;
		left = right;
		right = temp;
	}
	if (top > bottom) {
		temp = top;
		top = bottom;
		bottom = temp;
	}

	if (left > 319 || right < 0 || top > 199 || bottom < 0)
		return;

	_noInvalidated = 0;

	if (_invalidatedCount >= 30) {
		_invalidatedLefts[0] = 0;
		_invalidatedTops[0] = 0;
		_invalidatedRights[0] = 319;
		_invalidatedBottoms[0] = 199;
		_invalidatedCount = 1;
		return;
	}

	if (left < 0)
		left = 0;

	if (right > 319)
		right = 319;

	if (top < 0)
		top = 0;

	if (bottom > 199)
		bottom = 199;

	left &= 0xfff0;
	right |= 0x000f;

	for (rect = 0; rect < _invalidatedCount; rect++) {

		if (_invalidatedTops[rect] > top) {
			if (_invalidatedTops[rect] > bottom) {
				for (i = _invalidatedCount; i > rect; i--) {
					_invalidatedLefts[i] =
					    _invalidatedLefts[i - 1];
					_invalidatedTops[i] =
					    _invalidatedTops[i - 1];
					_invalidatedRights[i] =
					    _invalidatedRights[i - 1];
					_invalidatedBottoms[i] =
					    _invalidatedBottoms[i - 1];
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
	return;
}

void Draw::blitInvalidated(void) {
	int16 i;

	if (_cursorIndex == 4)
		blitCursor();

	if (_vm->_inter->_terminate)
		return;

	if (_noInvalidated && _applyPal == 0)
		return;

	if (_noInvalidated) {
		setPalette();
		_applyPal = 0;
		return;
	}

	if (_applyPal) {
		clearPalette();

		_vm->_video->drawSprite(_backSurface, _frontSurface, 0, 0, 319,
		    199, 0, 0, 0);
		setPalette();
		_invalidatedCount = 0;
		_noInvalidated = 1;
		_applyPal = 0;
		return;
	}

	_vm->_global->_doRangeClamp = 0;
	for (i = 0; i < _invalidatedCount; i++) {
		_vm->_video->drawSprite(_backSurface, _frontSurface,
		    _invalidatedLefts[i], _invalidatedTops[i],
		    _invalidatedRights[i], _invalidatedBottoms[i],
		    _invalidatedLefts[i], _invalidatedTops[i], 0);
	}
	_vm->_global->_doRangeClamp = 1;

	_invalidatedCount = 0;
	_noInvalidated = 1;
	_applyPal = 0;
}

void Draw::setPalette(void) {
	if (_vm->_global->_videoMode != 0x13)
		error("setPalette: Video mode 0x%x is not supported!\n",
		    _vm->_global->_videoMode);

	_vm->_global->_pPaletteDesc->unused1 = _unusedPalette1;
	_vm->_global->_pPaletteDesc->unused2 = _unusedPalette2;
	_vm->_global->_pPaletteDesc->vgaPal = _vgaPalette;
	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	_paletteCleared = 0;
}

void Draw::clearPalette(void) {
	if (_paletteCleared == 0) {
		_paletteCleared = 1;
		_vm->_util->clearPalette();
	}
}

void Draw::blitCursor(void) {
	if (_cursorIndex == -1)
		return;

	_cursorIndex = -1;

	if (_noInvalidated) {
		_vm->_video->drawSprite(_backSurface, _frontSurface,
		    _cursorX, _cursorY,
		    _cursorX + _cursorWidth - 1,
		    _cursorY + _cursorHeight - 1, _cursorX,
		    _cursorY, 0);
	} else {
		invalidateRect(_cursorX, _cursorY,
		    _cursorX + _cursorWidth - 1,
		    _cursorY + _cursorHeight - 1);
	}
}

void Draw::animateCursor(int16 cursor) {
	int16 newX = 0;
	int16 newY = 0;
	Game::Collision *ptr;
	int16 minX;
	int16 minY;
	int16 maxX;
	int16 maxY;
	int16 cursorIndex;

	cursorIndex = cursor;

	if (cursorIndex == -1) {
		cursorIndex = 0;
		for (ptr = _vm->_game->_collisionAreas; ptr->left != -1; ptr++) {
			if (ptr->flags & 0xfff0)
				continue;

			if (ptr->left > _vm->_global->_inter_mouseX)
				continue;

			if (ptr->right < _vm->_global->_inter_mouseX)
				continue;

			if (ptr->top > _vm->_global->_inter_mouseY)
				continue;

			if (ptr->bottom < _vm->_global->_inter_mouseY)
				continue;

			if ((ptr->flags & 0xf) < 3)
				cursorIndex = 1;
			else
				cursorIndex = 3;
			break;
		}
		if (_cursorAnimLow[cursorIndex] == -1)
			cursorIndex = 1;
	}

	if (_cursorAnimLow[cursorIndex] != -1) {
		if (cursorIndex == _cursorIndex) {
			if (_cursorAnimDelays[_cursorIndex] != 0 &&
			    _cursorAnimDelays[_cursorIndex] * 10 +
			    _cursorTimeKey <= _vm->_util->getTimeKey()) {
				_cursorAnim++;
				_cursorTimeKey = _vm->_util->getTimeKey();
			} else {
/*				if (_noInvalidated &&
					inter_mouseX == _cursorX &&	inter_mouseY == _cursorY)
						return;*/
			}
		} else {
			_cursorIndex = cursorIndex;
			if (_cursorAnimDelays[_cursorIndex] != 0) {
				_cursorAnim =
				    _cursorAnimLow[_cursorIndex];
				_cursorTimeKey = _vm->_util->getTimeKey();
			} else {
				_cursorAnim = _cursorIndex;
			}
		}

		if (_cursorAnimDelays[_cursorIndex] != 0 &&
		    (_cursorAnimHigh[_cursorIndex] < _cursorAnim ||
			_cursorAnimLow[_cursorIndex] >
			_cursorAnim)) {
			_cursorAnim = _cursorAnimLow[_cursorIndex];
		}

		newX = _vm->_global->_inter_mouseX;
		newY = _vm->_global->_inter_mouseY;
		if (_cursorXDeltaVar != -1) {
			newX -= (uint16)VAR_OFFSET(_cursorIndex * 4 + (_cursorXDeltaVar / 4) * 4);
			newY -= (uint16)VAR_OFFSET(_cursorIndex * 4 + (_cursorYDeltaVar / 4) * 4);
		}

		minX = MIN(newX, _cursorX);
		minY = MIN(newY, _cursorY);
		maxX = MAX(_cursorX, newX) + _cursorWidth - 1;
		maxY = MAX(_cursorY, newY) + _cursorHeight - 1;
		_vm->_video->drawSprite(_backSurface, _cursorBack,
		    newX, newY, newX + _cursorWidth - 1,
		    newY + _cursorHeight - 1, 0, 0, 0);

		_vm->_video->drawSprite(_cursorSprites, _backSurface,
		    _cursorWidth * _cursorAnim, 0,
		    _cursorWidth * (_cursorAnim + 1) - 1,
		    _cursorHeight - 1, newX, newY, _transparentCursor);

		if (_noInvalidated == 0) {
			cursorIndex = _cursorIndex;
			_cursorIndex = -1;
			blitInvalidated();
			_cursorIndex = cursorIndex;
		} else {
			_vm->_video->waitRetrace(_vm->_global->_videoMode);
		}

		_vm->_video->drawSprite(_backSurface, _frontSurface,
		    minX, minY, maxX, maxY, minX, minY, 0);

		_vm->_video->drawSprite(_cursorBack, _backSurface,
		    0, 0, _cursorWidth - 1, _cursorHeight - 1,
		    newX, newY, 0);
	} else {
		blitCursor();
	}

	_cursorX = newX;
	_cursorY = newY;
}

void Draw::freeSprite(int16 index) {
	int i;

	// .-- sub_CD84 ---
	if (_spritesArray[index] == 0)
		return;
	_vm->_video->freeSurfDesc(_spritesArray[index]);

	for (i = 0; i < 3; i++)
		if (_bigSpritesParts[index][i] != 0)
			_vm->_video->freeSurfDesc(_bigSpritesParts[index][i]);
	// '------

	_spritesArray[index] = 0;
}

void Draw::adjustCoords(char adjust, int16 *coord1, int16 *coord2) {
	if (_word_2E8E2 == 2)
		return;

	if (adjust == 0) {
		if (coord2 != 0)
			*coord2 *= 2;
		if (coord1 != 0)
			*coord2 *= 2;
	}
	else if (adjust == 1) {
		if (coord2 != 0)
			*coord2 = (signed) ((unsigned) (*coord2 + 1) / 2);
		if (coord1 != 0)
			*coord1 = (signed) ((unsigned) (*coord1 + 1) / 2);
	}
	else if (adjust == 2) {
		if (coord2 != 0)
			*coord2 = *coord2 * 2 + 1;
		if (coord1 != 0)
			*coord1 = *coord1 * 2 + 1;
	}
}

void Draw::fillRect(int16 index, int16 left, int16 top, int16 right,
		int16 bottom, int16 color) {
	int i;
	int16 newbottom;

	if (bottom < _spritesHeights[index]) {
		_vm->_video->fillRect(_spritesArray[index], left, top, right, bottom, color);
		return;
	}

	if (top < _spritesHeights[index]) {
		_vm->_video->fillRect(_spritesArray[index], left, top, right,
				_spritesHeights[index]-1, color);
	}

	for (i = 1; i < 4; i++) {
		if ((_spritesHeights[index] * i) > bottom)
			continue;
		if (_bigSpritesParts[index][i-1] == 0)
			return;
		newbottom = MIN(bottom - (_spritesHeights[index] * i), (_spritesHeights[index] * i) - 1);
		_vm->_video->fillRect(_bigSpritesParts[index][i-1], left, 0, right, newbottom, color);
	}
}

void Draw::drawSprite(int16 source, int16 dest, int16 left,
		int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) {
	int i;
	int16 topS;
	int16 yS;
	int16 newbottom;

	if (bottom < _spritesHeights[source]) {
		drawSprite(_spritesArray[source], dest, left, top, right, bottom, x, y, transp);
		return;
	}

	topS = top;
	yS = y;

	if (top < _spritesHeights[source]) {
		drawSprite(_spritesArray[source], dest, left, top, right,
				_spritesHeights[source], x, y, transp);
		yS = y + _spritesHeights[source] - top;
		topS = _spritesHeights[source];
	}
	for (i = 1; i < 4; i++) {
		if ((_spritesHeights[source] * i) > topS)
			continue;
		if ((_spritesHeights[source] * (i+1)) <= topS)
			continue;
		if (_bigSpritesParts[source][i-1] == 0)
			break;
		newbottom = MIN(bottom - (_spritesHeights[source] * i), _spritesHeights[source] - 1);
		drawSprite(_bigSpritesParts[source][i-1], dest, left,
				topS - _spritesHeights[source], right, newbottom, x, yS, transp);
		yS += newbottom - (topS - (_spritesHeights[source] * i)) + 1;
		topS += newbottom - (topS - (_spritesHeights[source] * i)) + 1;
	}
}

void Draw::drawSprite(Video::SurfaceDesc * source, int16 dest, int16 left,
		int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) {
	int i;
	int16 topS;
	int16 yS;
	int16 newbottom;

	if ((y + bottom - top) < _spritesHeights[dest]) {
		_vm->_video->drawSprite(source, _spritesArray[dest], left, top,
				right, bottom, x, y, transp);
		return;
	}

	topS = top;
	yS = y;

	if (y < _spritesHeights[dest]) {
		_vm->_video->drawSprite(source, _spritesArray[dest], left, top, right,
				top + _spritesHeights[dest] - y - 1, x, y, transp);
		yS = _spritesHeights[dest];
		topS += _spritesHeights[dest] - y;
	}

	for (i = 1; i < 4; i++) {
		if ((_spritesHeights[dest] * i) > yS)
			continue;
		if ((_spritesHeights[dest] * (i+1)) <= yS)
			continue;
		if (_bigSpritesParts[dest][i-1] == 0)
			break;
		newbottom = MIN(bottom, (int16) (topS + _spritesHeights[dest] - 1));
		_vm->_video->drawSprite(source, _bigSpritesParts[dest][i-1], left, topS,
				right, newbottom, x, yS - (_spritesHeights[dest] * i), transp);
		yS += newbottom - topS + 1;
		topS += newbottom - topS + 1;
	}
}

void Draw::drawSprite(int16 source, Video::SurfaceDesc * dest, int16 left,
		int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) {
	int i;
	int16 topS;
	int16 yS;
	int16 newbottom;

	if (bottom < _spritesHeights[source]) {
		_vm->_video->drawSprite(_spritesArray[source], dest, left, top, right,
				bottom, x, y, transp);
		return;
	}

	topS = top;
	yS = y;

	if (top < _spritesHeights[source]) {
		_vm->_video->drawSprite(_spritesArray[source], dest, left, top, right,
				_spritesHeights[source] - 1, x, y, transp);
		yS = y + _spritesHeights[source] - top;
		topS = _spritesHeights[source];
	}

	for (i = 1; i < 4; i++) {
		if ((_spritesHeights[source] * i) > topS)
			continue;
		if ((_spritesHeights[source] * (i+1)) <= topS)
			continue;
		if (_bigSpritesParts[source][i-1] == 0)
			break;
		newbottom = MIN(bottom - (_spritesHeights[source] * i), _spritesHeights[source] - 1);
		_vm->_video->drawSprite(_bigSpritesParts[source][i-1], dest, left,
				topS - (_spritesHeights[source] * i), right, newbottom, x, y, transp);
		yS += newbottom - (topS - (_spritesHeights[source] * i)) + 1;
		topS += newbottom - (topS - (_spritesHeights[source] * i)) + 1;
	}
}

void Draw::drawString(char *str, int16 x, int16 y, int16 color1, int16 color2,
		int16 transp, Video::SurfaceDesc *dest, Video::FontDesc *font) {
	while (*str != '\0') {
		_vm->_video->drawLetter(*str, x, y, font, transp, color1, color2, dest);
		if (font->extraData == 0)
			x += font->itemWidth;
		else
			x += *(((char *)font->extraData) + (*str - font->startItem));
		str++;
	}
}

void Draw::printTextCentered(int16 arg_0, int16 left, int16 top, int16 right,
		int16 bottom, char *str, int16 fontIndex, int16 color) {
	char *storedIP;
	int i;
	int length;
	int16 width;
	
	adjustCoords(1, &left, &top);
	adjustCoords(1, &right, &bottom);

	if (_vm->_game->_totFileData[0x7E] != 0) {
		storedIP = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr = _vm->_game->_totFileData + 0x7E;
		WRITE_VAR(17, (uint32) arg_0);
		WRITE_VAR(18, (uint32) left);
		WRITE_VAR(19, (uint32) top);
		WRITE_VAR(20, (uint32) right-left+1);
		WRITE_VAR(21, (uint32) bottom-top+1);
		_vm->_inter->funcBlock(0);
		_vm->_global->_inter_execPtr = storedIP;
	}
	
	if (str[0] == '\0')
		return;

	_transparency = 1;
	_destSpriteX = left;
	_destSpriteY = top;
	_fontIndex = fontIndex;
	_frontColor = color;
	_textToPrint = str;
	width = 0;
	if (_fonts[fontIndex]->extraData == 0)
		width = strlen(str) * _fonts[fontIndex]->itemWidth;
	else {
		length = strlen(str);
		for (i = 0; i < length; i++)
			width +=
				*(((char*)_fonts[fontIndex]->extraData) + (str[i] - _fonts[_fontIndex]->startItem));
	}

	adjustCoords(1, &width, 0);
	_destSpriteX += (right - left + 1 - width) / 2;

	spriteOperation(DRAW_PRINTTEXT);
}

} // End of namespace Gob
