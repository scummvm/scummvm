/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */
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
	
	for (int i = 0; i < 4; i++)
		_fonts[i] = 0;
	
	_textToPrint = 0;
	_transparency = 0;

	for (int i = 0; i < 50; i++)
		_spritesArray[i] = 0;

	_invalidatedCount = 0;
	for (int i = 0; i < 30; i++) {
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

	_cursorX = 0;
	_cursorY = 0;
	_cursorWidth = 0;
	_cursorHeight = 0;

	_cursorXDeltaVar = -1;
	_cursorYDeltaVar = -1;

	for (int i = 0; i < 40; i++) {
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
	if (_cursorX + _cursorWidth > 320)
		_cursorWidth = 320 - _cursorX;

	if (_cursorY + _cursorHeight > 200)
		_cursorHeight = 200 - _cursorY;

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

void Draw::spriteOperation(int16 operation) {
	uint16 id;
	char *dataBuf;
	Game::TotResItem *itemPtr;
	int32 offset;
	int16 len;
	int16 i;
	int16 x;
	int16 y;
	int16 perLine;

	if (_sourceSurface >= 100)
		_sourceSurface -= 80;

	if (_destSurface >= 100)
		_destSurface -= 80;

	if (_renderFlags & RENDERFLAG_USEDELTAS) {
		if (_sourceSurface == 21) {
			_spriteLeft += _backDeltaX;
			_spriteTop += _backDeltaY;
		}

		if (_destSurface == 21) {
			_destSpriteX += _backDeltaX;
			_destSpriteY += _backDeltaY;
			if (operation == DRAW_DRAWLINE ||
			    (operation >= DRAW_DRAWBAR
				&& operation <= DRAW_FILLRECTABS)) {
				_spriteRight += _backDeltaX;
				_spriteBottom += _backDeltaY;
			}
		}
	}

	switch (operation) {
	case DRAW_BLITSURF:
		_vm->_video->drawSprite(_spritesArray[_sourceSurface],
		    _spritesArray[_destSurface],
		    _spriteLeft, _spriteTop,
		    _spriteLeft + _spriteRight - 1,
		    _spriteTop + _spriteBottom - 1,
		    _destSpriteX, _destSpriteY, _transparency);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX + _spriteRight - 1,
			    _destSpriteY + _spriteBottom - 1);
		}
		break;

	case DRAW_PUTPIXEL:
		_vm->_video->putPixel(_destSpriteX, _destSpriteY,
		    _frontColor, _spritesArray[_destSurface]);
		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX, _destSpriteY);
		}
		break;

	case DRAW_FILLRECT:
		_vm->_video->fillRect(_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _destSpriteX + _spriteRight - 1,
		    _destSpriteY + _spriteBottom - 1, _backColor);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX + _spriteRight - 1,
			    _destSpriteY + _spriteBottom - 1);
		}
		break;

	case DRAW_DRAWLINE:
		_vm->_video->drawLine(_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _frontColor);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom);
		}
		break;

	case DRAW_INVALIDATE:
		if (_destSurface == 21) {
			invalidateRect(_destSpriteX - _spriteRight, _destSpriteY - _spriteBottom,	// !!
			    _destSpriteX + _spriteRight,
			    _destSpriteY + _spriteBottom);
		}
		break;

	case DRAW_LOADSPRITE:
		id = _spriteLeft;
		if (id >= 30000) {
			dataBuf =
			    _vm->_game->loadExtData(id, &_spriteRight,
			    &_spriteBottom);
			_vm->_video->drawPackedSprite((byte *)dataBuf, _spriteRight,
			    _spriteBottom, _destSpriteX,
			    _destSpriteY, _transparency,
			    _spritesArray[_destSurface]);
			if (_destSurface == 21) {
				invalidateRect(_destSpriteX,
				    _destSpriteY,
				    _destSpriteX + _spriteRight - 1,
				    _destSpriteY + _spriteBottom - 1);
			}
			free(dataBuf);
			break;
		}
		// Load from .TOT resources
		itemPtr = &_vm->_game->_totResourceTable->items[id];
		offset = itemPtr->offset;
		if (offset >= 0) {
			dataBuf =
			    ((char *)_vm->_game->_totResourceTable) +
			    szGame_TotResTable + szGame_TotResItem *
			    _vm->_game->_totResourceTable->itemsCount + offset;
		} else {
			dataBuf =
			    _vm->_game->_imFileData +
			    (int32)READ_LE_UINT32(&((int32 *)_vm->_game->_imFileData)[-offset - 1]);
		}

		_spriteRight = itemPtr->width;
		_spriteBottom = itemPtr->height;
		_vm->_video->drawPackedSprite((byte *)dataBuf,
		    _spriteRight, _spriteBottom,
		    _destSpriteX, _destSpriteY,
		    _transparency, _spritesArray[_destSurface]);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX + _spriteRight - 1,
			    _destSpriteY + _spriteBottom - 1);
		}
		break;

	case DRAW_PRINTTEXT:
		len = strlen(_textToPrint);
		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX +
			    len * _fonts[_fontIndex]->itemWidth - 1,
			    _destSpriteY +
			    _fonts[_fontIndex]->itemHeight - 1);
		}

		for (i = 0; i < len; i++) {
			_vm->_video->drawLetter(_textToPrint[i],
			    _destSpriteX, _destSpriteY,
			    _fonts[_fontIndex],
			    _transparency,
			    _frontColor, _backColor,
			    _spritesArray[_destSurface]);

			_destSpriteX += _fonts[_fontIndex]->itemWidth;
		}
		break;

	case DRAW_DRAWBAR:
		_vm->_video->drawLine(_spritesArray[_destSurface],
		    _destSpriteX, _spriteBottom,
		    _spriteRight, _spriteBottom, _frontColor);

		_vm->_video->drawLine(_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _destSpriteX, _spriteBottom, _frontColor);

		_vm->_video->drawLine(_spritesArray[_destSurface],
		    _spriteRight, _destSpriteY,
		    _spriteRight, _spriteBottom, _frontColor);

		_vm->_video->drawLine(_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _destSpriteY, _frontColor);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom);
		}
		break;

	case DRAW_CLEARRECT:
		if (_backColor < 16) {
			_vm->_video->fillRect(_spritesArray[_destSurface],
			    _destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom,
			    _backColor);
		}
		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom);
		}
		break;

	case DRAW_FILLRECTABS:
		_vm->_video->fillRect(_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _backColor);

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom);
		}
		break;

	case DRAW_DRAWLETTER:
		if (_fontToSprite[_fontIndex].sprite == -1) {
			if (_destSurface == 21) {
				invalidateRect(_destSpriteX,
				    _destSpriteY,
				    _destSpriteX +
				    _fonts[_fontIndex]->itemWidth - 1,
				    _destSpriteY +
				    _fonts[_fontIndex]->itemHeight -
				    1);
			}
			_vm->_video->drawLetter(_letterToPrint,
			    _destSpriteX, _destSpriteY,
			    _fonts[_fontIndex],
			    _transparency,
			    _frontColor, _backColor,
			    _spritesArray[_destSurface]);
			break;
		}

		perLine =
		    _spritesArray[(int16)_fontToSprite[_fontIndex].
		    sprite]->width / _fontToSprite[_fontIndex].width;

		y = (_letterToPrint -
		    _fontToSprite[_fontIndex].base) / perLine *
		    _fontToSprite[_fontIndex].height;

		x = (_letterToPrint -
		    _fontToSprite[_fontIndex].base) % perLine *
		    _fontToSprite[_fontIndex].width;

		if (_destSurface == 21) {
			invalidateRect(_destSpriteX, _destSpriteY,
			    _destSpriteX +
			    _fontToSprite[_fontIndex].width,
			    _destSpriteY +
			    _fontToSprite[_fontIndex].height);
		}

		_vm->_video->drawSprite(_spritesArray[(int16)_fontToSprite
			[_fontIndex].sprite],
		    _spritesArray[_destSurface], x, y,
		    x + _fontToSprite[_fontIndex].width,
		    y + _fontToSprite[_fontIndex].height,
		    _destSpriteX, _destSpriteY, _transparency);

		break;
	}

	if (_renderFlags & RENDERFLAG_USEDELTAS) {
		if (_sourceSurface == 21) {
			_spriteLeft -= _backDeltaX;
			_spriteTop -= _backDeltaY;
		}

		if (_destSurface == 21) {
			_destSpriteX -= _backDeltaX;
			_destSpriteY -= _backDeltaY;
		}
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

void Draw::interPalLoad(void) {
	int16 i;
	int16 ind1;
	int16 ind2;
	byte cmd;
	char *palPtr;

	cmd = *_vm->_global->_inter_execPtr++;
	_applyPal = 0;
	if (cmd & 0x80)
		cmd &= 0x7f;
	else
		_applyPal = 1;

	if (cmd == 49) {
		warning("inter_palLoad: cmd == 49 is not supported");
		//var_B = 1;
		for (i = 0; i < 18; i++, _vm->_global->_inter_execPtr++) {
			if (i < 2) {
				if (_applyPal == 0)
					continue;

				_unusedPalette1[i] = *_vm->_global->_inter_execPtr;
				continue;
			}
			//if (*inter_execPtr != 0)
			//      var_B = 0;

			ind1 = *_vm->_global->_inter_execPtr >> 4;
			ind2 = (*_vm->_global->_inter_execPtr & 0xf);

			_unusedPalette1[i] =
			    ((_palLoadData1[ind1] + _palLoadData2[ind2]) << 8) +
			    (_palLoadData2[ind1] + _palLoadData1[ind2]);
		}

		_vm->_global->_pPaletteDesc->unused1 = _unusedPalette1;
	}

	switch (cmd) {
	case 52:
		for (i = 0; i < 16; i++, _vm->_global->_inter_execPtr += 3) {
			_vgaSmallPalette[i].red = _vm->_global->_inter_execPtr[0];
			_vgaSmallPalette[i].green = _vm->_global->_inter_execPtr[1];
			_vgaSmallPalette[i].blue = _vm->_global->_inter_execPtr[2];
		}
		break;

	case 50:
		for (i = 0; i < 16; i++, _vm->_global->_inter_execPtr++)
			_unusedPalette2[i] = *_vm->_global->_inter_execPtr;
		break;

	case 53:
		palPtr = _vm->_game->loadTotResource(_vm->_inter->load16());
		memcpy((char *)_vgaPalette, palPtr, 768);
		break;

	case 54:
		memset((char *)_vgaPalette, 0, 768);
		break;
	}
	if (!_applyPal) {
		_vm->_global->_pPaletteDesc->unused2 = _unusedPalette2;
		_vm->_global->_pPaletteDesc->unused1 = _unusedPalette1;

		if (_vm->_global->_videoMode != 0x13)
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *)_vgaSmallPalette;
		else
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *)_vgaPalette;

		_vm->_palanim->fade((Video::PalDesc *) _vm->_global->_pPaletteDesc, 0, 0);
	}
}

void Draw::printText(void) {
	int16 savedFlags;
	int16 ldestSpriteX;
	char *dataPtr;
	char *ptr;
	char *ptr2;
	int16 index;
	int16 destX;
	int16 destY;
	char cmd;
	int16 val;
	char buf[20];

	index = _vm->_inter->load16();

	_vm->_cdrom->playMultMusic();

	dataPtr = (char *)_vm->_game->_totTextData + _vm->_game->_totTextData->items[index].offset;
	ptr = dataPtr;

	if (_renderFlags & RENDERFLAG_CAPTUREPUSH) {
		_destSpriteX = READ_LE_UINT16(ptr);
		_destSpriteY = READ_LE_UINT16(ptr + 2);
		_spriteRight = READ_LE_UINT16(ptr + 4) - _destSpriteX + 1;
		_spriteBottom = READ_LE_UINT16(ptr + 6) - _destSpriteY + 1;
		_vm->_game->capturePush(_destSpriteX, _destSpriteY,
						 _spriteRight, _spriteBottom);
		(*_vm->_scenery->_pCaptureCounter)++;
	}
	_destSpriteX = READ_LE_UINT16(ptr);
	destX = _destSpriteX;

	_destSpriteY = READ_LE_UINT16(ptr + 2);
	destY = _destSpriteY;

	_spriteRight = READ_LE_UINT16(ptr + 4);
	_spriteBottom = READ_LE_UINT16(ptr + 6);
	_destSurface = 21;

	ptr += 8;

	_backColor = *ptr++;
	_transparency = 1;
	spriteOperation(DRAW_CLEARRECT);

	_backColor = 0;
	savedFlags = _renderFlags;

	_renderFlags &= ~RENDERFLAG_NOINVALIDATE;
	for (; (_destSpriteX = READ_LE_UINT16(ptr)) != -1; ptr++) {
		_destSpriteX += destX;
		_destSpriteY = READ_LE_UINT16(ptr + 2) + destY;
		_spriteRight = READ_LE_UINT16(ptr + 4) + destX;
		_spriteBottom = READ_LE_UINT16(ptr + 6) + destY;
		ptr += 8;

		cmd = (*ptr & 0xf0) >> 4;
		if (cmd == 0) {
			_frontColor = *ptr & 0xf;
			spriteOperation(DRAW_DRAWLINE);
		} else if (cmd == 1) {
			_frontColor = *ptr & 0xf;
			spriteOperation(DRAW_DRAWBAR);
		} else if (cmd == 2) {
			_backColor = *ptr & 0xf;
			spriteOperation(DRAW_FILLRECTABS);
		}
	}
	ptr += 2;

	for (ptr2 = ptr; *ptr2 != 1; ptr2++) {
		if (*ptr2 == 3)
			ptr2++;

		if (*ptr2 == 2)
			ptr2 += 4;
	}

	ptr2++;

	while (*ptr != 1) {
		cmd = *ptr;
		if (cmd == 3) {
			ptr++;
			_fontIndex = (*ptr & 0xf0) >> 4;
			_frontColor = *ptr & 0xf;
			ptr++;
			continue;
		} else if (cmd == 2) {
			ptr++;
			_destSpriteX = destX + READ_LE_UINT16(ptr);
			_destSpriteY = destY + READ_LE_UINT16(ptr + 2);
			ptr += 4;
			continue;
		}

		if ((byte)*ptr != 0xba) {
			_letterToPrint = *ptr;
			spriteOperation(DRAW_DRAWLETTER);
			_destSpriteX +=
			    _fonts[_fontIndex]->itemWidth;
			ptr++;
		} else {
			cmd = ptr2[17] & 0x7f;
			if (cmd == 0) {
				val = READ_LE_UINT16(ptr2 + 18) * 4;
				sprintf(buf, "%d",  VAR_OFFSET(val));
			} else if (cmd == 1) {
				val = READ_LE_UINT16(ptr2 + 18) * 4;

				strcpy(buf, _vm->_global->_inter_variables + val);
			} else {
				val = READ_LE_UINT16(ptr2 + 18) * 4;

				sprintf(buf, "%d",  VAR_OFFSET(val));
				if (buf[0] == '-') {
					while (strlen(buf) - 1 < (uint32)ptr2[17]) {
						_vm->_util->insertStr("0", buf, 1);
					}
				} else {
					while (strlen(buf) - 1 < (uint32)ptr2[17]) {
						_vm->_util->insertStr("0", buf, 0);
					}
				}

				_vm->_util->insertStr(",", buf, strlen(buf) + 1 - ptr2[17]);
			}

			_textToPrint = buf;
			ldestSpriteX = _destSpriteX;
			spriteOperation(DRAW_PRINTTEXT);
			if (ptr2[17] & 0x80) {
				if (ptr[1] == ' ') {
					_destSpriteX += _fonts[_fontIndex]->itemWidth;
					while (ptr[1] == ' ')
						ptr++;
					if (ptr[1] == 2) {
						if (READ_LE_UINT16(ptr + 4) == _destSpriteY)
							ptr += 5;
					}
				} else if (ptr[1] == 2 && READ_LE_UINT16(ptr + 4) == _destSpriteY) {
					ptr += 5;
					_destSpriteX += _fonts[_fontIndex]->itemWidth;
				}
			} else {
				_destSpriteX = ldestSpriteX + _fonts[_fontIndex]->itemWidth;
			}
			ptr2 += 23;
			ptr++;
		}
	}

	_renderFlags = savedFlags;
	if (_renderFlags & 4) {
		warning("printText: Input not supported!");
//              xor     ax, ax
//              loc_436_1391:
//              xor     dx, dx
//              push    ax
//              push    dx
//              push    ax
//              push    dx
//              push    ax
//              mov     al, 0
//              push    ax
//              call    sub_9FF_1E71
//              add     sp, 0Ch
	}

	if ((_renderFlags & RENDERFLAG_CAPTUREPOP) && *_vm->_scenery->_pCaptureCounter != 0) {
		(*_vm->_scenery->_pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
}

}				// End of namespace Gob
