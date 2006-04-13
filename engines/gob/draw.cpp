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
	
	int i;

	for (i = 0; i < 4; i++)
		_fonts[i] = 0;
	
	_textToPrint = 0;
	_transparency = 0;

	for (i = 0; i < 50; i++) {
		_spritesArray[i] = 0;
		_sprites1[i] = 0;
		_sprites2[i] = 0;
		_sprites3[i] = 0;
		_spritesWidths[i] = 0;
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
	// .-- sub_CD84 ---
	if (_spritesArray[index] == 0)
		return;
	_vm->_video->freeSurfDesc(_spritesArray[index]);

//	warning("GOB2 Stub! freeSprite: dword_2EFB4, dword_2EFB8, dword_2EFBC");

	if (_sprites1[index] != 0)
		_vm->_video->freeSurfDesc(_sprites1[index]);
	if (_sprites2[index] != 0)
		_vm->_video->freeSurfDesc(_sprites2[index]);
	if (_sprites3[index] != 0)
		_vm->_video->freeSurfDesc(_sprites3[index]);

	// '------

	_spritesArray[index] = 0;
}

void Draw::adjustCoords(int16 *coord1, int16 *coord2, char adjust) {
	warning("GOB2 Stub! if (word_2E8E2 == 2) return;");
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

//			sub_EDFC(0x16, _anim_animAreaWidth, _anim_animAreaHeight, 0);
void Draw::initBigSprite(int16 index, int16 height, int16 width, int16 flags) {
	int16 realwidth;
	int16 widthdiff;
	Gob::Video::SurfaceDesc **fragment;

	if (flags != 0)
		flags = 2;

	// .-- sub_CBD0 ---

	_sprites1[index] = 0;
	_sprites2[index] = 0;
	_sprites3[index] = 0;
	_spritesWidths[index] = width;

	if (_vm->_video->getRectSize(width, height, flags, _vm->_global->_videoMode) > 6500) {
		_spritesWidths[index] = width & 0xFFFE;
		while (_vm->_video->getRectSize(_spritesWidths[index],
					height, flags, _vm->_global->_videoMode) > 6500)
			_spritesWidths[index] -= 2;

		realwidth = _spritesWidths[index];
		_spritesArray[index] =
			_vm->_video->initSurfDesc(realwidth, height, flags, _vm->_global->_videoMode);
		
		fragment = _sprites1 + index;
		while (realwidth < width) {
			widthdiff = width - realwidth;
			if (_spritesWidths[index] >= widthdiff) {
				*fragment = _vm->_video->initSurfDesc(widthdiff, height, flags, _vm->_global->_videoMode);
				realwidth = width;
			}
			else {
				*fragment = _vm->_video->initSurfDesc(_spritesWidths[index], height,
					flags, _vm->_global->_videoMode);
				realwidth += _spritesWidths[index];
			}
			_vm->_video->clearSurf(*fragment++);
		}
	} else
		_spritesArray[index] =
			_vm->_video->initSurfDesc(width, height, flags, _vm->_global->_videoMode);

	_vm->_video->clearSurf(_spritesArray[index]);
	
	// '------
}

}				// End of namespace Gob
