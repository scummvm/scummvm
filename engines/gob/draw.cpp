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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/draw.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/resources.h"
#include "gob/script.h"
#include "gob/inter.h"
#include "gob/video.h"
#include "gob/palanim.h"

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
	_hotspotText = 0;

	_backDeltaX = 0;
	_backDeltaY = 0;

	for (int i = 0; i < kFontCount; i++)
		_fonts[i] = 0;

	_spritesArray.resize(SPRITES_COUNT);

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

	_pattern = 0;
}

Draw::~Draw() {
	for (int i = 0; i < kFontCount; i++)
		delete _fonts[i];
}

void Draw::invalidateRect(int16 left, int16 top, int16 right, int16 bottom) {
	if (_renderFlags & RENDERFLAG_NOINVALIDATE) {
		_vm->_video->dirtyRectsAll();
		return;
	}

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
		forceBlit();
		setPalette();
		_invalidatedCount = 0;
		_noInvalidated = true;
		_applyPal = false;
		return;
	}

	_vm->_video->_doRangeClamp = false;
	for (int i = 0; i < _invalidatedCount; i++) {
		_vm->_video->drawSprite(*_backSurface, *_frontSurface,
		    _invalidatedLefts[i], _invalidatedTops[i],
		    _invalidatedRights[i], _invalidatedBottoms[i],
		    _invalidatedLefts[i], _invalidatedTops[i], 0);
		_vm->_video->dirtyRectsAdd(_invalidatedLefts[i], _invalidatedTops[i],
				_invalidatedRights[i], _invalidatedBottoms[i]);
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

void Draw::dirtiedRect(int16 surface,
		int16 left, int16 top, int16 right, int16 bottom) {

	dirtiedRect(_spritesArray[surface], left, top, right, bottom);
}

void Draw::dirtiedRect(SurfaceDescPtr surface,
		int16 left, int16 top, int16 right, int16 bottom) {

	if (surface == _backSurface)
		invalidateRect(left, top, right, bottom);
	else if (surface == _frontSurface)
		_vm->_video->dirtyRectsAdd(left, top, right, bottom);
	else if (_vm->_video->_splitSurf && (surface == _vm->_video->_splitSurf))
		_vm->_video->retrace();
}

void Draw::initSpriteSurf(int16 index, int16 width, int16 height,
		int16 flags) {

	_spritesArray[index] =
		_vm->_video->initSurfDesc(_vm->_global->_videoMode, width, height, flags);
	_vm->_video->clearSurf(*_spritesArray[index]);
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

int Draw::stringLength(const char *str, int16 fontIndex) {
	static const int8 japaneseExtraCharLen[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	if ((fontIndex < 0) || (fontIndex > 7) || !_fonts[fontIndex])
		return 0;

	Font &font = *_fonts[fontIndex];

	int len = 0;

	if (_vm->_global->_language == 10) {

		for (int i = 0; str[i] != 0; i++) {
			if (((unsigned char) str[i+1]) < 128) {
				len += japaneseExtraCharLen[4];
				i++;
			} else
				len += font.getCharWidth();
		}

	} else {

		if (!font.isMonospaced())
			while (*str != '\0')
				len += font.getCharWidth(*str++);
		else
			len = strlen(str) * font.getCharWidth();

	}

	return len;
}

void Draw::drawString(const char *str, int16 x, int16 y, int16 color1, int16 color2,
		int16 transp, SurfaceDesc &dest, const Font &font) {

	while (*str != '\0') {
		_vm->_video->drawLetter(*str, x, y, font, transp, color1, color2, dest);
		x += font.getCharWidth(*str);
		str++;
	}
}

void Draw::printTextCentered(int16 id, int16 left, int16 top, int16 right,
		int16 bottom, const char *str, int16 fontIndex, int16 color) {

	adjustCoords(1, &left, &top);
	adjustCoords(1, &right, &bottom);

	uint16 centerOffset = _vm->_game->_script->getFunctionOffset(TOTFile::kFunctionCenter);
	if (centerOffset != 0) {
		_vm->_game->_script->call(centerOffset);

		WRITE_VAR(17, (uint32) id);
		WRITE_VAR(18, (uint32) left);
		WRITE_VAR(19, (uint32) top);
		WRITE_VAR(20, (uint32) (right - left + 1));
		WRITE_VAR(21, (uint32) (bottom - top + 1));
		_vm->_inter->funcBlock(0);

		_vm->_game->_script->pop();
	}

	if (str[0] == '\0')
		return;

	_transparency = 1;
	_destSpriteX = left;
	_destSpriteY = top;
	_fontIndex = fontIndex;
	_frontColor = color;
	_textToPrint = str;

	Font &font = *_fonts[fontIndex];

	int16 width = 0;
	if (!font.isMonospaced()) {
		const char *s = str;
		while (*s != '\0')
			width += font.getCharWidth(*s++);
	}
	else
		width = strlen(str) * font.getCharWidth();

	adjustCoords(1, &width, 0);
	_destSpriteX += (right - left + 1 - width) / 2;

	spriteOperation(DRAW_PRINTTEXT);
}

void Draw::oPlaytoons_sub_F_1B( uint16 id, int16 left, int16 top, int16 right, int16 bottom, char *paramStr, int16 fontIndex, int16 var4, int16 shortId) {
	int16 i, j, width;
	char tmpStr[128];

	strcpy( tmpStr, paramStr);
	adjustCoords(1, &left, &top);
	adjustCoords(1, &right,  &bottom);

	uint16 centerOffset = _vm->_game->_script->getFunctionOffset(TOTFile::kFunctionCenter);
	if (centerOffset != 0) {
		_vm->_game->_script->call(centerOffset);

		WRITE_VAR(17, (uint32) id & 0x7FFF);
		WRITE_VAR(18, (uint32) left);
		WRITE_VAR(19, (uint32) top);
		WRITE_VAR(20, (uint32) (right - left + 1));
		WRITE_VAR(21, (uint32) (bottom - top + 1));

		if (_vm->_game->_script->peekUint16(41) >= '4') {
			WRITE_VAR(22, (uint32) fontIndex);
			WRITE_VAR(23, (uint32) var4);
			if (id & 0x8000)
				WRITE_VAR(24, (uint32) 1);
			else
				WRITE_VAR(24, (uint32) 0);
			WRITE_VAR(25, (uint32) shortId);
			if (_hotspotText) {
				strncpy(_hotspotText, paramStr, 40);
				_hotspotText[39] = 0;
			}
		}
		_vm->_inter->funcBlock(0);
		_vm->_game->_script->pop();
	}
	strcpy(paramStr, tmpStr);
	if (*paramStr) {
		_transparency = 1;
		_fontIndex = fontIndex;
		_frontColor = var4;
		if (_vm->_game->_script->peekUint16(41) >= '4' && strchr(paramStr, 92)) {
			char str[80];
			char *str2;
			int16 strLen= 0;
			int16 offY, deltaY;

			str2 = paramStr;
			do {
				strLen++;
				str2++;
				str2 = strchr(str2, 92);
			} while (str2);
			deltaY = (bottom - right + 1 - (strLen * _fonts[fontIndex]->getCharHeight())) / (strLen + 1);
			offY = right + deltaY;
			for (i = 0; paramStr[i]; i++) {
				j = 0;
				while (paramStr[i] && paramStr[i] != 92)
					str[j++] = paramStr[i++];
				str[j] = 0;
				_destSpriteX = left;
				_destSpriteY = offY;
				_textToPrint = str;
				width = stringLength(str, fontIndex);
				adjustCoords(1, &width, NULL);
				_destSpriteX += (top - left + 1 - width) / 2;
				spriteOperation(DRAW_PRINTTEXT);
				offY += deltaY + _fonts[fontIndex]->getCharHeight();
			}
		} else {
			_destSpriteX = left;
			if (_vm->_game->_script->peekUint16(41) >= '4')
				_destSpriteY = right + (bottom - right + 1 - _fonts[fontIndex]->getCharHeight()) / 2;
			else
				_destSpriteY = right;
			_textToPrint = paramStr;
			width = stringLength(paramStr, fontIndex);
			adjustCoords(1, &width, NULL);
			_destSpriteX += (top - left + 1 - width) / 2;
			spriteOperation(DRAW_PRINTTEXT);
		}
	}
	return;
}

void Draw::activeWin (int16 id) {
	int i, j;
	bool found = false;
	int16 t[10], t2[10];
	int nextId = -1;
	int oldId  = -1;
	SurfaceDescPtr tempSrf;
	SurfaceDescPtr oldSrf[10];

	warning ("activeWindow %d", id);

	if (_fascinWin[id].id == -1)
		return;
	warning("swap screen ?");
	forceBlit();
	_vm->_video->dirtyRectsAll();
	for (i = 0; i < 10; i++) {
		t[i] = -1;
		t2[i] = -1;
//		oldSrf[i] = 0;
	}
	for (i = 0; i < 10; i++)
		if ((i != id) && (_fascinWin[i].id > _fascinWin[id].id) && (winOverlap(i, id))) {
			t[_fascinWin[i].id] = i;
			found = true;
		}
	if (found) {
		for (j = 10 - 1; j >= 0; j--)
			if (t[j] != -1) {
				if (nextId != -1)
					_vm->_video->drawSprite(*_spritesArray[_destSurface], *_fascinWin[nextId].savedSurface,
											 _fascinWin[t[j]].left, _fascinWin[t[j]].top,
											 _fascinWin[t[j]].left + _fascinWin[t[j]].width  - 1,
											 _fascinWin[t[j]].top  + _fascinWin[t[j]].height - 1,
											 _fascinWin[t[j]].left & 15, 0, 0);
				t2[j] = nextId;
				restoreWin(t[j]);
				nextId = t[j];
			}
			oldId = nextId;
			_vm->_video->drawSprite(*_spritesArray[_destSurface], *_fascinWin[nextId].savedSurface,
									 _fascinWin[id].left, _fascinWin[id].top,
									 _fascinWin[id].left + _fascinWin[id].width  - 1,
									 _fascinWin[id].top  + _fascinWin[id].height - 1,
									 _fascinWin[id].left & 15, 0, 0);
			restoreWin(id);
			nextId = id;
			for (j = 0; j < 10; j++)
				if (t[j] != -1) {
					_vm->_video->drawSprite(*_spritesArray[_destSurface], *_fascinWin[nextId].savedSurface,
											 _fascinWin[t[j]].left, _fascinWin[t[j]].top,
											 _fascinWin[t[j]].left + _fascinWin[t[j]].width  - 1,
											 _fascinWin[t[j]].top  + _fascinWin[t[j]].height - 1,
											 _fascinWin[t[j]].left & 15, 0, 0);
					oldSrf[t[j]] = _fascinWin[nextId].savedSurface;
					if (t2[j] != -1)
						_vm->_video->drawSprite(*_fascinWin[t2[j]].savedSurface, *_spritesArray[_destSurface],
												 _fascinWin[t[j]].left & 15, 0,
												(_fascinWin[t[j]].left & 15) + _fascinWin[t[j]].width - 1,
												 _fascinWin[t[j]].height - 1, _fascinWin[t[j]].left,
												 _fascinWin[t[j]].top, 0);
					else {
// Shift skipped as always set to zero (?)
						_vm->_video->drawSprite(*_spritesArray[_destSurface], *_spritesArray[_destSurface],
												 _fascinWin[t[j]].left, _fascinWin[t[j]].top,
												 _fascinWin[t[j]].left + _fascinWin[t[j]].width  - 1,
												 _fascinWin[t[j]].top  + _fascinWin[t[j]].height - 1,
												 _fascinWin[t[j]].left, _fascinWin[t[j]].top, 0);
					}
					dirtiedRect(_destSurface, _fascinWin[t[j]].left, _fascinWin[t[j]].top,
								_fascinWin[t[j]].left + _fascinWin[t[j]].width  - 1,
								_fascinWin[t[j]].top  + _fascinWin[t[j]].height - 1);
					nextId = t2[j];
				}
			tempSrf = _vm->_video->initSurfDesc(_vm->_global->_videoMode, _winMaxWidth + 15, _winMaxHeight, 0);
			_vm->_video->drawSprite(*_spritesArray[_destSurface], *tempSrf,
									 _fascinWin[id].left, _fascinWin[id].top,
									 _fascinWin[id].left + _fascinWin[id].width  - 1,
									 _fascinWin[id].top  + _fascinWin[id].height - 1,
									 _fascinWin[id].left & 15, 0, 0);
			_vm->_video->drawSprite(*_fascinWin[oldId].savedSurface, *_spritesArray[_destSurface],
									 _fascinWin[id].left & 15, 0,
									(_fascinWin[id].left & 15) + _fascinWin[id].width - 1,
									 _fascinWin[id].height - 1,
									 _fascinWin[id].left, _fascinWin[id].top, 0);
			_fascinWin[oldId].savedSurface.reset();
			_fascinWin[oldId].savedSurface = tempSrf;
			oldSrf[id] = _fascinWin[oldId].savedSurface;
// Strangerke not sure concerning the use of _destSurface
			dirtiedRect(_destSurface, _fascinWin[id].left, _fascinWin[id].top,
						_fascinWin[id].left + _fascinWin[id].width  - 1,
						_fascinWin[id].top  + _fascinWin[id].height - 1);
			nextId = id;
			for (j = 0; j < 10; j++)
				if (oldSrf[j])
					_fascinWin[j].savedSurface = oldSrf[j];
	}
	for (i = 0; i < 10; i++)
		if ((i != id) && (_fascinWin[i].id > _fascinWin[id].id))
			_fascinWin[i].id--;
	_fascinWin[id].id = _winCount - 1;
}

bool Draw::winOverlap(int16 idWin1, int16 idWin2) {
	if ((_fascinWin[idWin1].left + _fascinWin[idWin1].width  <= _fascinWin[idWin2].left) ||
		(_fascinWin[idWin2].left + _fascinWin[idWin2].width  <= _fascinWin[idWin1].left) ||
		(_fascinWin[idWin1].top  + _fascinWin[idWin1].height <= _fascinWin[idWin2].top ) ||
		(_fascinWin[idWin2].top  + _fascinWin[idWin2].height <= _fascinWin[idWin1].top ))
		return(false);
 return(true);
}

void Draw::closeWin (int16 i) {
	warning("closeWin %d", i);
	if (_fascinWin[i].id == -1)
		return;
	WRITE_VAR((_winVarArrayStatus / 4) + i, VAR((_winVarArrayStatus / 4) + i) | 1);
	restoreWin(i);
	_fascinWin[i].id = -1;
	_fascinWin[i].savedSurface.reset();
	_winCount--;
}

int16 Draw::openWin(int16 id) {
	if (_fascinWin[id].id != -1)
		return(0);
	_fascinWin[id].id = _winCount;
	_winCount++;
	_fascinWin[id].left   = VAR((_winVarArrayLeft   / 4) + id);
	_fascinWin[id].top    = VAR((_winVarArrayTop    / 4) + id);
	_fascinWin[id].width  = VAR((_winVarArrayWidth  / 4) + id);
	_fascinWin[id].height = VAR((_winVarArrayHeight / 4) + id);
	_fascinWin[id].savedSurface = _vm->_video->initSurfDesc(_vm->_global->_videoMode, _winMaxWidth + 15, _winMaxHeight, 0);

	warning("Draw::openWin id %d- left %d top %d l %d h%d", id, _fascinWin[id].left, _fascinWin[id].top, _fascinWin[id].width, _fascinWin[id].height);

	saveWin(id);
	WRITE_VAR((_winVarArrayStatus / 4) + id, VAR((_winVarArrayStatus / 4) + id) & 0xFFFFFFFE);
	return(1);
}

void Draw::restoreWin(int16 i) {
	warning("restoreWin");
	_vm->_video->drawSprite(*_fascinWin[i].savedSurface, *_spritesArray[_destSurface],
							 _fascinWin[i].left & 15, 0,
							(_fascinWin[i].left & 15) + _fascinWin[i].width - 1, _fascinWin[i].height - 1,
							 _fascinWin[i].left, _fascinWin[i].top, 0);
	dirtiedRect(_fascinWin[i].savedSurface, _fascinWin[i].left, _fascinWin[i].top,
				_fascinWin[i].left + _fascinWin[i].width  - 1,
				_fascinWin[i].top  + _fascinWin[i].height - 1);
}

void Draw::saveWin(int16 id) {
	warning("saveWin");
	_vm->_video->drawSprite(*_spritesArray[_destSurface], *_fascinWin[id].savedSurface,
							 _fascinWin[id].left,  _fascinWin[id].top,
							 _fascinWin[id].left + _fascinWin[id].width  - 1,
							 _fascinWin[id].top  + _fascinWin[id].height - 1,
							 _fascinWin[id].left & 15, 0, 0);	
	dirtiedRect(_destSurface, _fascinWin[id].left, 0,
				_fascinWin[id].left + _fascinWin[id].width  - 1,
				_fascinWin[id].top  + _fascinWin[id].height - 1);
}

void Draw::winMove(int16 id) {
	int oldLeft, oldTop;

	oldLeft = _fascinWin[id].left;
	oldTop = _fascinWin[id].top;
	restoreWin(id);

	_fascinWin[id].left = _vm->_global->_inter_mouseX;
	_fascinWin[id].top = _vm->_global->_inter_mouseY;
	WRITE_VAR((_winVarArrayLeft / 4) + id, _fascinWin[id].left);
	WRITE_VAR((_winVarArrayTop  / 4) + id, _fascinWin[id].top);
	saveWin(id);

// Shift skipped as always set to zero (?)
	_vm->_video->drawSprite(*_frontSurface, *_spritesArray[_destSurface],
							oldLeft, oldTop,
							oldLeft + _fascinWin[id].width  - 1,
							oldTop  + _fascinWin[id].height - 1,
							_fascinWin[id].left, _fascinWin[id].top, 0);
	dirtiedRect(_frontSurface, _fascinWin[id].left, _fascinWin[id].top,
				_fascinWin[id].left + _fascinWin[id].width  - 1,
				_fascinWin[id].top  + _fascinWin[id].height - 1);
}

void Draw::winTrace(int16 left, int16 top, int16 width, int16 height) {
// TODO : Implement correct the trace of the Window. In short,
//  - drawline currently use the wrong surface, to be fixed
//  - dirtiedRect should be put after the 4 drawlines when the surface is fixed
//  - drawline should be replaced by a drawline with palette inversion

	int16 right, bottom;

	right  = left + width  - 1;
	bottom = top  + height - 1;

	_vm->_video->drawLine(*_backSurface, left,  top,    right, top,    0);
	_vm->_video->drawLine(*_backSurface, left,  top,    left,  bottom, 0);
	_vm->_video->drawLine(*_backSurface, left,  bottom, right, bottom, 0);
	_vm->_video->drawLine(*_backSurface, right, top,    right, bottom, 0);
}

void Draw::handleWinBorder(int16 id) {
	int16 minX = 0;
	int16 maxX = 320;
	int16 minY = 0;
	int16 maxY = 200;
	warning("handleWinBorder");

	if (VAR((_winVarArrayStatus / 4) + id) & 8)
		minX = (int16)(VAR((_winVarArrayLimitsX / 4) + id) >> 16L);
	if (VAR((_winVarArrayStatus / 4) + id) & 16)
		maxX = (int16)(VAR((_winVarArrayLimitsX / 4) + id) & 0xFFFFL);
	if (VAR((_winVarArrayStatus / 4) + id) & 32)
		minY = (int16)(VAR((_winVarArrayLimitsY / 4) + id) >> 16L);
	if (VAR((_winVarArrayStatus / 4) + id) & 64)
		maxY = (int16)(VAR((_winVarArrayLimitsY / 4) + id) & 0xFFFFL);
	_vm->_global->_inter_mouseX = _fascinWin[id].left;
	_vm->_global->_inter_mouseY = _fascinWin[id].top;
	if (_vm->_global->_mousePresent)
		_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
	winTrace(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY, _fascinWin[id].width, _fascinWin[id].height);
	_cursorX = _vm->_global->_inter_mouseX;
	_cursorY = _vm->_global->_inter_mouseY;

	do {
// TODO: Usage of checkKeys to be confirmed. A simple refresh of the mouse buttons is required
		_vm->_game->checkKeys(&_vm->_global->_inter_mouseX, &_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, 1);
		if (_vm->_global->_inter_mouseX != _cursorX || _vm->_global->_inter_mouseY != _cursorY) {
			if (_vm->_global->_inter_mouseX < minX) {
				_vm->_global->_inter_mouseX = minX;
				if (_vm->_global->_mousePresent)
					_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
			}
			if (_vm->_global->_inter_mouseY < minY) {
				_vm->_global->_inter_mouseY = minY;
				if (_vm->_global->_mousePresent)
					_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
			}
			if (_vm->_global->_inter_mouseX + _fascinWin[id].width > maxX) {
				_vm->_global->_inter_mouseX = maxX - _fascinWin[id].width;
				if (_vm->_global->_mousePresent)
					_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
			}
			if (_vm->_global->_inter_mouseY + _fascinWin[id].height > maxY) {
				_vm->_global->_inter_mouseY = maxY - _fascinWin[id].height;
				if (_vm->_global->_mousePresent)
					_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
			}
			winTrace(_cursorX,_cursorY, _fascinWin[id].width, _fascinWin[id].height);
			winTrace(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY, _fascinWin[id].width, _fascinWin[id].height);
			_cursorX = _vm->_global->_inter_mouseX;
			_cursorY = _vm->_global->_inter_mouseY;
		}
	} while (_vm->_game->_mouseButtons);
	winTrace(_cursorX, _cursorY, _fascinWin[id].width, _fascinWin[id].height);
	_cursorX = _vm->_global->_inter_mouseX;
	_cursorY = _vm->_global->_inter_mouseY;
}

int16 Draw::handleCurWin() {
	int8 i, matchNum;
	int16 bestMatch = -1;

	warning("handleCurWin");
	if (_vm->_game->_mouseButtons != 1 || ((_vm->_draw->_renderFlags & 128) == 0))
		return(0);
	for (i = 0; i < 10; i++)
		if (_fascinWin[i].id != -1)
			if ((_vm->_global->_inter_mouseX >= _fascinWin[i].left) &&
				(_vm->_global->_inter_mouseX <  _fascinWin[i].left + _fascinWin[i].width) &&
				(_vm->_global->_inter_mouseY >= _fascinWin[i].top) &&
				(_vm->_global->_inter_mouseY <  _fascinWin[i].top  + _fascinWin[i].height)) {
				if (_fascinWin[i].id == _winCount - 1) {
					if ((_vm->_global->_inter_mouseX < _fascinWin[i].left + 12) &&
						(_vm->_global->_inter_mouseY < _fascinWin[i].top  + 12) &&
						(VAR(_winVarArrayStatus / 4 + i) & 2)) {
						blitCursor();
						activeWin(i);
						closeWin(i);
						_vm->_util->waitMouseRelease(1);
						return(i);
					}
					if ((_vm->_global->_inter_mouseX >= _fascinWin[i].left + _fascinWin[i].width - 12) &&
						(_vm->_global->_inter_mouseY <  _fascinWin[i].top  + 12) &&
						(VAR(_winVarArrayStatus / 4 + i) & 4) &&
						(_vm->_global->_mousePresent) &&
						(_vm->_global->_videoMode != 0x07)) {
						blitCursor();
						handleWinBorder(i);
						winMove(i);
						_vm->_global->_inter_mouseX = _fascinWin[i].left + _fascinWin[i].width - 12 + 1;
						_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
						return(-i);
					}
					return(0);
				} else
					if (_fascinWin[i].id > bestMatch) {
						bestMatch = _fascinWin[i].id;
						matchNum = i;
					}
			}
	if (bestMatch != -1) {
		blitCursor();
		activeWin(matchNum);
	}
	return(0);
}

void Draw::winDecomp(int16 x, int16 y, SurfaceDescPtr bmp) {
// TODO: Implementation to be confirmed (used cut and paste from another part of the code)
	Resource *resource;

	resource = _vm->_game->_resources->getResource((uint16) _spriteLeft,
		                                             &_spriteRight, &_spriteBottom);

	if (!resource)
		return;

	_vm->_video->drawPackedSprite(resource->getData(),
			_spriteRight, _spriteBottom, _destSpriteX, _destSpriteY,
			_transparency, *_spritesArray[_destSurface]);

	dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
			_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);

	delete resource;
	return;
}

void Draw::winDraw(int16 fct) {
 int16 left;
 int16 top;
 int16 width;
 int16 height;

 bool found = false;
 int  i, j, k, l;
 int len;
 Resource *resource;
 int table[10];
 SurfaceDescPtr tempSrf;

	if (_destSurface == 21) {
		if (_vm->_global->_curWinId) {
			if (_fascinWin[_vm->_global->_curWinId].id == -1)
				return;
			else {
				_destSpriteX += _fascinWin[_vm->_global->_curWinId].left;
				_destSpriteY += _fascinWin[_vm->_global->_curWinId].top;
				if (fct == 3 || (fct >= 7 && fct <= 9)) {
					_spriteRight  += _fascinWin[_vm->_global->_curWinId].left;
					_spriteBottom += _fascinWin[_vm->_global->_curWinId].top;
				}
			}
		}
		left = _destSpriteX;
		top = _destSpriteY;
	} else {
		if (_vm->_global->_curWinId) {
			if (_fascinWin[_vm->_global->_curWinId].id == -1)
				return;
			else {
				_spriteLeft += _fascinWin[_vm->_global->_curWinId].left;
				_spriteTop  += _fascinWin[_vm->_global->_curWinId].top;
			}
		}
		left = _spriteLeft;
		top = _spriteTop;
	}
	for (i = 0; i < 10; i++)
		table[i] = 0;
	switch (fct) {
	case DRAW_BLITSURF:   // 0 - move
	case DRAW_FILLRECT:   // 2 - fill rectangle
		width  = left + _spriteRight - 1;
		height = top  + _spriteBottom - 1;
		break;
	case DRAW_PUTPIXEL:   // 1 - put a pixel
		width  = _destSpriteX;
		height = _destSpriteY;
		break;
	case DRAW_DRAWLINE:   // 3 - draw line
	case DRAW_DRAWBAR:    // 7 - draw border
	case DRAW_CLEARRECT:  // 8 - clear rectangle
	case DRAW_FILLRECTABS:// 9 - fill rectangle, with other coordinates
		width  = _spriteRight;
		height = _spriteBottom;
		break;
	case DRAW_INVALIDATE: // 4 - Draw a circle
		left   = _destSpriteX - _spriteRight;
		top    = _destSpriteY - _spriteRight;
		width  = _destSpriteX + _spriteRight;
		height = _destSpriteY + _spriteBottom;
		break;
	case DRAW_LOADSPRITE: // 5 - Uncompress and load a sprite

// TODO: check the implementation, currently dirty cut and paste of DRAW_SPRITE code
		resource = _vm->_game->_resources->getResource((uint16) _spriteLeft,
			                                             &_spriteRight, &_spriteBottom);

		if (!resource)
			break;

		_vm->_video->drawPackedSprite(resource->getData(),
				_spriteRight, _spriteBottom, _destSpriteX, _destSpriteY,
				_transparency, *_spritesArray[_destSurface]);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);

		delete resource;

		width  = _destSpriteX + _spriteRight - 1;
		height = _destSpriteY + _spriteBottom - 1;
		break;
	case DRAW_PRINTTEXT:  // 6 - Display string
		width  = _destSpriteX - 1 + strlen(_textToPrint) * _fonts[_fontIndex]->getCharWidth();
		height = _destSpriteY - 1 + _fonts[_fontIndex]->getCharHeight();
		break;
	case DRAW_DRAWLETTER: // 10 - Display a character
		if (_fontToSprite[_fontIndex].sprite == -1) {
			width  = _destSpriteX - 1 + _fonts[_fontIndex]->getCharWidth();
			height = _destSpriteY - 1 + _fonts[_fontIndex]->getCharHeight();
		} else {
			width  = _destSpriteX + _fontToSprite[_fontIndex].width - 1;
			height = _destSpriteY + _fontToSprite[_fontIndex].height - 1;
		}
		break;
	default:
		warning("Unexpected fct value");
		break;
	}
	for (i = 0; i < 10; i++)
		if ((i != _vm->_global->_curWinId) && (_fascinWin[i].id != -1))
			if (!_vm->_global->_curWinId || _fascinWin[i].id>_fascinWin[_vm->_global->_curWinId].id)
				if ((_fascinWin[i].left + _fascinWin[i].width  > left) && (width  >= _fascinWin[i].left) &&
					(_fascinWin[i].top  + _fascinWin[i].height > top ) && (height >= _fascinWin[i].top)) {
					found = true;
					table[_fascinWin[i].id] = i;
				}
	if ((_sourceSurface == 21) && (fct == 0)) {
		_vm->_video->drawSprite(*_spritesArray[_sourceSurface], *_spritesArray[_destSurface],
								 _spriteLeft, _spriteTop, _spriteLeft + _spriteRight - 1,
								 _spriteTop + _spriteBottom - 1, _destSpriteX, _destSpriteY, _transparency);
		if (!found)
			return;
		if (_vm->_global->_curWinId == 0)
			j = 0;
		else
			j = _fascinWin[_vm->_global->_curWinId].id + 1;
		for (i = 9; i >= j; i--) {
			if (table[i])
				_vm->_video->drawSprite(*_fascinWin[table[i]].savedSurface, *_spritesArray[_destSurface],
										 _fascinWin[table[i]].left & 15, 0,
										(_fascinWin[table[i]].left & 15) + _fascinWin[table[i]].width - 1,
										 _fascinWin[table[i]].height - 1, _fascinWin[table[i]].left - _spriteLeft + _destSpriteX,
										 _fascinWin[table[i]].top - _spriteTop + _destSpriteY, 0);
		}
		return;
	}
	if (found) {
		tempSrf = _vm->_video->initSurfDesc(_vm->_global->_videoMode, width - left + 1, height - top + 1, 0);
		_vm->_video->drawSprite(*_backSurface, *tempSrf, left, top, width, height, 0, 0, 0);
		if (_vm->_global->_curWinId == 0)
			j = 0;
		else
			j = _fascinWin[_vm->_global->_curWinId].id + 1;
		for (i = 9; i >= j; i--) {
			if (table[i])
				_vm->_video->drawSprite(*_fascinWin[table[i]].savedSurface, *tempSrf,
										 _fascinWin[table[i]].left & 15, 0,
										(_fascinWin[table[i]].left & 15) + _fascinWin[table[i]].width - 1,
										 _fascinWin[table[i]].height - 1,
										 _fascinWin[table[i]].left  - left,
										 _fascinWin[table[i]].top   - top , 0);
		}
// Strangerke not sure concerning the use of _destSurface
		dirtiedRect(_destSurface, left, top, width, height);
		switch (fct) {
		case DRAW_BLITSURF:   // 0 - move
			_vm->_video->drawSprite(*_spritesArray[_sourceSurface], *tempSrf,
									 _spriteLeft, _spriteTop, _spriteLeft + _spriteRight - 1,
									 _spriteTop + _spriteBottom - 1, 0, 0, _transparency);
			break;
		case DRAW_PUTPIXEL:   // 1 - put a pixel
			_vm->_video->putPixel(0, 0, _frontColor, *tempSrf);
			break;
		case DRAW_FILLRECT:   // 2 - fill rectangle
			_vm->_video->fillRect(*tempSrf, 0, 0, _spriteRight - 1, _spriteBottom - 1, _backColor);
			break;
		case DRAW_DRAWLINE:   // 3 - draw line
			_vm->_video->drawLine(*tempSrf, 0, 0, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _frontColor);
			break;
		case DRAW_INVALIDATE: // 4 - Draw a circle
			_vm->_video->drawCircle(*tempSrf, _spriteRight, _spriteRight, _spriteRight, _frontColor);
			break;
		case DRAW_LOADSPRITE: // 5 - Uncompress and load a sprite
			winDecomp(0, 0, tempSrf);
			break;
		case DRAW_PRINTTEXT:  // 6 - Display string
			len = strlen(_textToPrint);
			for (j = 0; j < len; j++)
				_vm->_video->drawLetter(_textToPrint[j], j * _fonts[_fontIndex]->getCharWidth(), 0,
										*_fonts[_fontIndex], _transparency, _frontColor, _backColor, *tempSrf);
			_destSpriteX += len * _fonts[_fontIndex]->getCharWidth();
			break;
		case DRAW_DRAWBAR:    // 7 - draw border
			_vm->_video->drawLine(*tempSrf, 0, _spriteBottom - _destSpriteY, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _frontColor);
			_vm->_video->drawLine(*tempSrf, 0, 0, 0, _spriteBottom - _destSpriteY, _frontColor);
			_vm->_video->drawLine(*tempSrf, _spriteRight - _destSpriteX, 0, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _frontColor);
			_vm->_video->drawLine(*tempSrf, 0, 0, _spriteRight - _destSpriteX, 0, _frontColor);
			break;
		case DRAW_CLEARRECT:  // 8 - clear rectangle
			if (_backColor < 16)
				_vm->_video->fillRect(*tempSrf, 0, 0, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _backColor);
			break;
		case DRAW_FILLRECTABS:// 9 - fill rectangle, with other coordinates
			_vm->_video->fillRect(*tempSrf, 0, 0, _spriteRight - _destSpriteX, _spriteBottom - _destSpriteY, _backColor);
			break;
		case DRAW_DRAWLETTER: // 10 - Display a character
			if (_fontToSprite[_fontIndex].sprite == -1)
				_vm->_video->drawLetter(_letterToPrint, 0, 0, *_fonts[_fontIndex], _transparency, _frontColor, _backColor, *tempSrf);
			else {
				int xx, yy, nn;
				nn = _spritesArray[_fontToSprite[_fontIndex].sprite]->getWidth() / _fontToSprite[_fontIndex].width;
				yy = ((_letterToPrint - _fontToSprite[_fontIndex].base) / nn) * _fontToSprite[_fontIndex].height;
				xx = ((_letterToPrint - _fontToSprite[_fontIndex].base) % nn) * _fontToSprite[_fontIndex].width;
				_vm->_video->drawSprite(*_spritesArray[_fontToSprite[_fontIndex].sprite], *tempSrf,
										 xx, yy, xx + _fontToSprite[_fontIndex].width - 1,
										 yy + _fontToSprite[_fontIndex].height - 1, 0, 0, _transparency);
			}
			break;
		default:
			warning("Unexpected fct value");
			break;
		}
		if (_vm->_global->_curWinId == 0)
			i = 0;
		else
			i = _fascinWin[_vm->_global->_curWinId].id + 1;
		for (;i < 10; i++) {
			if (table[i]) {
				k = table[i];
				_vm->_video->drawSprite(*tempSrf, *_fascinWin[k].savedSurface,
										0, 0, width - left, height - top,
										left - _fascinWin[k].left + (_fascinWin[k].left & 15),
										top  - _fascinWin[k].top, 0);
// Shift skipped as always set to zero (?)
				_vm->_video->drawSprite(*_frontSurface, *tempSrf,
										MAX(left  , _fascinWin[k].left),
										MAX(top   , _fascinWin[k].top),
										MIN(width , (int16) (_fascinWin[k].left + _fascinWin[k].width  - 1)),
										MIN(height, (int16) (_fascinWin[k].top  + _fascinWin[k].height - 1)),
										MAX(left  , _fascinWin[k].left) - left,
										MAX(top   , _fascinWin[k].top)  - top, 0);
				if (_cursorIndex != -1)
					_vm->_video->drawSprite(*_cursorSpritesBack, *tempSrf,
											0, 0, _cursorWidth - 1, _cursorHeight - 1,
											_cursorX - left, _cursorY - top, 0);
				for (j = 9; j > i; j--) {
					if (table[j] && winOverlap(k, table[j])) {
						l = table[j];
						_vm->_video->drawSprite(*_fascinWin[l].savedSurface, *tempSrf,
												MAX(_fascinWin[l].left, _fascinWin[k].left)
													- _fascinWin[l].left + (_fascinWin[l].left & 15),
												MAX(_fascinWin[l].top , _fascinWin[k].top ) - _fascinWin[l].top,
												MIN(_fascinWin[l].left + _fascinWin[l].width  - 1, _fascinWin[k].left + _fascinWin[k].width - 1)
													- _fascinWin[l].left + (_fascinWin[l].left & 15),
												MIN(_fascinWin[l].top  + _fascinWin[l].height - 1, _fascinWin[k].top  + _fascinWin[k].height - 1)
													- _fascinWin[l].top,
												MAX(_fascinWin[l].left, _fascinWin[k].left) - left,
												MAX(_fascinWin[l].top , _fascinWin[k].top ) - top, 0);
					}
				}
			}
		}
		_vm->_video->drawSprite(*tempSrf, *_backSurface, 0, 0, width - left, height - top, left, top, 0);
		tempSrf.reset();
	} else {
// Strangerke not sure concerning the use of _destSurface
		dirtiedRect(_destSurface, left, top, width, height);
		switch (fct) {
		case DRAW_BLITSURF:   // 0 - move
			_vm->_video->drawSprite(*_spritesArray[_sourceSurface], *_backSurface,
									 _spriteLeft, _spriteTop,
									 _spriteLeft + _spriteRight  - 1,
									 _spriteTop  + _spriteBottom - 1,
									 _destSpriteX, _destSpriteY, _transparency);
			break;
		case DRAW_PUTPIXEL:   // 1 - put a pixel
			_vm->_video->putPixel(_destSpriteX, _destSpriteY, _frontColor, *_backSurface);
			break;
		case DRAW_FILLRECT:   // 2 - fill rectangle
			_vm->_video->fillRect(*_backSurface, _destSpriteX, _destSpriteY, _destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1, _backColor);
			break;
		case DRAW_DRAWLINE:   // 3 - draw line
			_vm->_video->drawLine(*_backSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom, _frontColor);
			break;
		case DRAW_INVALIDATE: // 4 - Draw a circle
			_vm->_video->drawCircle(*_backSurface, _spriteRight, _spriteRight, _spriteRight, _frontColor);
			break;
		case DRAW_LOADSPRITE: // 5 - Uncompress and load a sprite
			winDecomp(_destSpriteX, _destSpriteY, _backSurface);
			break;
		case DRAW_PRINTTEXT:  // 6 - Display string
			len = strlen(_textToPrint);
			for ( j = 0; j < len; j++)
				_vm->_video->drawLetter(_textToPrint[j], _destSpriteX + j * _fonts[_fontIndex]->getCharWidth(),
										_destSpriteY, *_fonts[_fontIndex], _transparency, _frontColor, _backColor, *_backSurface);
				_destSpriteX += len * _fonts[_fontIndex]->getCharWidth();
			break;
		case DRAW_DRAWBAR:    // 7 - draw border
			_vm->_video->drawLine(*_backSurface, _destSpriteX, _spriteBottom, _spriteRight, _spriteBottom, _frontColor);
			_vm->_video->drawLine(*_backSurface, _destSpriteX, _destSpriteY,  _destSpriteX, _spriteBottom, _frontColor);
			_vm->_video->drawLine(*_backSurface, _spriteRight, _destSpriteY,  _spriteRight, _spriteBottom, _frontColor);
			_vm->_video->drawLine(*_backSurface, _destSpriteX, _destSpriteY,  _spriteRight, _destSpriteY,  _frontColor);
			break;
		case DRAW_CLEARRECT:  // 8 - clear rectangle
		if (_backColor < 16)
				_vm->_video->fillRect(*_backSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom, _backColor);
			break;
		case DRAW_FILLRECTABS:// 9 - fill rectangle, with other coordinates
			_vm->_video->fillRect(*_backSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom, _backColor);
			break;
		case DRAW_DRAWLETTER: // 10 - Display a character
			if (_fontToSprite[_fontIndex].sprite == -1)
				_vm->_video->drawLetter(_letterToPrint, _destSpriteX, _destSpriteY, *_fonts[_fontIndex], _transparency,
										_frontColor, _backColor, *_spritesArray[_destSurface]);
			else {
				int xx, yy, nn;
				nn = _spritesArray[_fontToSprite[_fontIndex].sprite]->getWidth() / _fontToSprite[_fontIndex].width;
				yy = ((_letterToPrint - _fontToSprite[_fontIndex].base) / nn) * _fontToSprite[_fontIndex].height;
				xx = ((_letterToPrint - _fontToSprite[_fontIndex].base) % nn) * _fontToSprite[_fontIndex].width;
				_vm->_video->drawSprite(*_spritesArray[_fontToSprite[_fontIndex].sprite], *_spritesArray[_destSurface],
										xx, yy,
										xx + _fontToSprite[_fontIndex].width  - 1,
										yy + _fontToSprite[_fontIndex].height - 1,
										_destSpriteX, _destSpriteY, _transparency);
			}
			break;
		default:
			warning("Unexpected fct value");
			break;
		}
	}
	if (_renderFlags & 16) {
		if (_sourceSurface == 21) {
			_spriteLeft -= _backDeltaX;
			_spriteTop -= _backDeltaY;
		}
		if (_destSurface == 21) {
			_destSpriteX -= _backDeltaX;
			_destSpriteY -= _backDeltaY;
		}
	}
	if (_vm->_global->_curWinId) {
		_destSpriteX -= _fascinWin[_vm->_global->_curWinId].left;
		_destSpriteY -= _fascinWin[_vm->_global->_curWinId].top;
	}
}

int32 Draw::getSpriteRectSize(int16 index) {
	if (!_spritesArray[index])
		return 0;

	return _spritesArray[index]->getWidth() * _spritesArray[index]->getHeight();
}

void Draw::forceBlit(bool backwards) {
	if (!_frontSurface || !_backSurface)
		return;
	if (_frontSurface == _backSurface)
		return;
	if (_spritesArray[20] != _frontSurface)
		return;
	if (_spritesArray[21] != _backSurface)
		return;

	if (!backwards) {
		_vm->_video->drawSprite(*_backSurface, *_frontSurface, 0, 0,
				_backSurface->getWidth() - 1, _backSurface->getHeight() - 1,
				0, 0, 0);
		_vm->_video->dirtyRectsAll();
	} else
		_vm->_video->drawSprite(*_frontSurface, *_backSurface, 0, 0,
				_frontSurface->getWidth() - 1, _frontSurface->getHeight() - 1,
				0, 0, 0);

}

const int16 Draw::_wobbleTable[360] = {
	 0x0000,  0x011D,  0x023B,  0x0359,  0x0476,  0x0593,  0x06B0,  0x07CC,  0x08E8,
	 0x0A03,  0x0B1D,  0x0C36,  0x0D4E,  0x0E65,  0x0F7B,  0x1090,  0x11A4,  0x12B6,
	 0x13C6,  0x14D6,  0x15E3,  0x16EF,  0x17F9,  0x1901,  0x1A07,  0x1B0C,  0x1C0E,
	 0x1D0E,  0x1E0B,  0x1F07,  0x2000,  0x20F6,  0x21EA,  0x22DB,  0x23C9,  0x24B5,
	 0x259E,  0x2684,  0x2766,  0x2846,  0x2923,  0x29FC,  0x2AD3,  0x2BA5,  0x2C75,
	 0x2D41,  0x2E09,  0x2ECE,  0x2F8F,  0x304D,  0x3106,  0x31BC,  0x326E,  0x331C,
	 0x33C6,  0x346C,  0x350E,  0x35AC,  0x3646,  0x36DB,  0x376C,  0x37F9,  0x3882,
	 0x3906,  0x3985,  0x3A00,  0x3A77,  0x3AE9,  0x3B56,  0x3BBF,  0x3C23,  0x3C83,
	 0x3CDE,  0x3D34,  0x3D85,  0x3DD1,  0x3E19,  0x3E5C,  0x3E99,  0x3ED2,  0x3F07,
	 0x3F36,  0x3F60,  0x3F85,  0x3FA6,  0x3FC1,  0x3FD8,  0x3FE9,  0x3FF6,  0x3FFD,
	 0x4000,  0x3FFD,  0x3FF6,  0x3FE9,  0x3FD8,  0x3FC1,  0x3FA6,  0x3F85,  0x3F60,
	 0x3F36,  0x3F07,  0x3ED2,  0x3E99,  0x3E5C,  0x3E19,  0x3DD1,  0x3D85,  0x3D34,
	 0x3CDE,  0x3C83,  0x3C23,  0x3BBF,  0x3B56,  0x3AE9,  0x3A77,  0x3A00,  0x3985,
	 0x3906,  0x3882,  0x37F9,  0x376C,  0x36DB,  0x3646,  0x35AC,  0x350E,  0x346C,
	 0x33C6,  0x331C,  0x326E,  0x31BC,  0x3106,  0x304D,  0x2F8F,  0x2ECE,  0x2E09,
	 0x2D41,  0x2C75,  0x2BA5,  0x2AD3,  0x29FC,  0x2923,  0x2846,  0x2766,  0x2684,
	 0x259E,  0x24B5,  0x23C9,  0x22DB,  0x21EA,  0x20F6,  0x1FFF,  0x1F07,  0x1E0B,
	 0x1D0E,  0x1C0E,  0x1B0C,  0x1A07,  0x1901,  0x17F9,  0x16EF,  0x15E3,  0x14D6,
	 0x13C6,  0x12B6,  0x11A4,  0x1090,  0x0F7B,  0x0E65,  0x0D4E,  0x0C36,  0x0B1D,
	 0x0A03,  0x08E8,  0x07CC,  0x06B0,  0x0593,  0x0476,  0x0359,  0x023B,  0x011D
	-0x0000, -0x011D, -0x023B, -0x0359, -0x0476, -0x0593, -0x06B0, -0x07CC, -0x08E8,
	-0x0A03, -0x0B1D, -0x0C36, -0x0D4E, -0x0E65, -0x0F7B, -0x1090, -0x11A4, -0x12B6,
	-0x13C6, -0x14D6, -0x15E3, -0x16EF, -0x17F9, -0x1901, -0x1A07, -0x1B0C, -0x1C0E,
	-0x1D0E, -0x1E0B, -0x1F07, -0x2000, -0x20F6, -0x21EA, -0x22DB, -0x23C9, -0x24B5,
	-0x259E, -0x2684, -0x2766, -0x2846, -0x2923, -0x29FC, -0x2AD3, -0x2BA5, -0x2C75,
	-0x2D41, -0x2E09, -0x2ECE, -0x2F8F, -0x304D, -0x3106, -0x31BC, -0x326E, -0x331C,
	-0x33C6, -0x346C, -0x350E, -0x35AC, -0x3646, -0x36DB, -0x376C, -0x37F9, -0x3882,
	-0x3906, -0x3985, -0x3A00, -0x3A77, -0x3AE9, -0x3B56, -0x3BBF, -0x3C23, -0x3C83,
	-0x3CDE, -0x3D34, -0x3D85, -0x3DD1, -0x3E19, -0x3E5C, -0x3E99, -0x3ED2, -0x3F07,
	-0x3F36, -0x3F60, -0x3F85, -0x3FA6, -0x3FC1, -0x3FD8, -0x3FE9, -0x3FF6, -0x3FFD,
	-0x4000, -0x3FFD, -0x3FF6, -0x3FE9, -0x3FD8, -0x3FC1, -0x3FA6, -0x3F85, -0x3F60,
	-0x3F36, -0x3F07, -0x3ED2, -0x3E99, -0x3E5C, -0x3E19, -0x3DD1, -0x3D85, -0x3D34,
	-0x3CDE, -0x3C83, -0x3C23, -0x3BBF, -0x3B56, -0x3AE9, -0x3A77, -0x3A00, -0x3985,
	-0x3906, -0x3882, -0x37F9, -0x376C, -0x36DB, -0x3646, -0x35AC, -0x350E, -0x346C,
	-0x33C6, -0x331C, -0x326E, -0x31BC, -0x3106, -0x304D, -0x2F8F, -0x2ECE, -0x2E09,
	-0x2D41, -0x2C75, -0x2BA5, -0x2AD3, -0x29FC, -0x2923, -0x2846, -0x2766, -0x2684,
	-0x259E, -0x24B5, -0x23C9, -0x22DB, -0x21EA, -0x20F6, -0x1FFF, -0x1F07, -0x1E0B,
	-0x1D0E, -0x1C0E, -0x1B0C, -0x1A07, -0x1901, -0x17F9, -0x16EF, -0x15E3, -0x14D6,
	-0x13C6, -0x12B6, -0x11A4, -0x1090, -0x0F7B, -0x0E65, -0x0D4E, -0x0C36, -0x0B1D,
	-0x0A03, -0x08E8, -0x07CC, -0x06B0, -0x0593, -0x0476, -0x0359, -0x023B, -0x011D
};

void Draw::wobble(SurfaceDesc &surfDesc) {
	int16 amplitude = 32;
	uint16 curFrame = 0;
	uint16 frameWobble = 0;
	uint16 rowWobble = 0;
	int8 *offsets = new int8[_vm->_height];

	_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, -1);

	while (amplitude > 0) {
		rowWobble = frameWobble;
		frameWobble = (frameWobble + 20) % 360;

		for (uint16 y = 0; y < _vm->_height; y++) {
			offsets[y] = amplitude +
				((_wobbleTable[rowWobble] * amplitude) / 0x4000);

			rowWobble = (rowWobble + 20) % 360;
		}

		if (curFrame++ & 16)
			amplitude--;

		for (uint16 y = 0; y < _vm->_height; y++)
			_vm->_video->drawSprite(surfDesc, *_frontSurface,
					0, y, _vm->_width - 1, y, offsets[y], y, 0);

		_vm->_palAnim->fadeStep(0);
		_vm->_video->dirtyRectsAll();
		_vm->_video->waitRetrace();
	}

	_vm->_video->drawSprite(surfDesc, *_frontSurface,
			0, 0, _vm->_width - 1, _vm->_height - 1, 0, 0, 0);

	_applyPal = false;
	_invalidatedCount = 0;
	_noInvalidated = true;
	_vm->_video->dirtyRectsAll();

	delete[] offsets;
}

Font *Draw::loadFont(const char *path) const {
	if (!_vm->_dataIO->existData(path))
		return 0;

	byte *data = _vm->_dataIO->getData(path);

	return new Font(data);
}

bool Draw::loadFont(int fontIndex, const char *path) {
	if ((fontIndex < 0) || (fontIndex >= kFontCount))
		return false;

	delete _fonts[fontIndex];

	_fonts[fontIndex] = loadFont(path);

	return _fonts[fontIndex] != 0;
}

} // End of namespace Gob
