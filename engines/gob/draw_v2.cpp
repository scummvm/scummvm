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
#include "gob/game.h"
#include "gob/util.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/cdrom.h"

namespace Gob {

Draw_v2::Draw_v2(GobEngine *vm) : Draw_v1(vm) {
}

void Draw_v2::printText(void) {
/*
	int16 savedFlags;
	int16 ldestSpriteX;
	char *dataPtr;
	char *ptr;
	char *ptr2;
	int16 destX;
	int16 destY;
	char cmd;
	int16 val;
	char buf[20];
*/
	int16 index;
	warning("GOB2 Stub! Draw_v2::printText()");

	index = _vm->_inter->load16();

	_vm->_cdrom->playMultMusic();

	return;
}

void Draw_v2::spriteOperation(int16 operation) {
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
			delete[] dataBuf;
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
		break;
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
		break;
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

} // End of namespace Gob
