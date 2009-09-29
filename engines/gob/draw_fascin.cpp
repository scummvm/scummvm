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

//#include "common/endian.h"
//#include "graphics/cursorman.h"

#include "gob/draw.h"
#include "gob/game.h"
#include "gob/resources.h"

namespace Gob {

Draw_Fascination::Draw_Fascination(GobEngine *vm) : Draw_v1(vm) {
}

void Draw_Fascination::spriteOperation(int16 operation) {
	int16 len;
	int16 x, y;
	int16 perLine;
	Resource *resource;

	operation &= 0x0F;

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
			if ((operation == DRAW_DRAWLINE) ||
			   ((operation >= DRAW_DRAWBAR) &&
			    (operation <= DRAW_FILLRECTABS))) {
				_spriteRight += _backDeltaX;
				_spriteBottom += _backDeltaY;
			}
		}
	}

	if (_renderFlags & 0x20) {
		if (_destSurface == 21 || (operation == 0 && _sourceSurface == 21)) {
			winDraw(operation);
			return; 
		}
	}
	
	Font *font = 0;
	switch (operation) {
	case DRAW_BLITSURF:
		_vm->_video->drawSprite(*_spritesArray[_sourceSurface],
		    *_spritesArray[_destSurface],
		    _spriteLeft, _spriteTop,
		    _spriteLeft + _spriteRight  - 1,
		    _spriteTop  + _spriteBottom - 1,
		    _destSpriteX, _destSpriteY, _transparency);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
		break;

	case DRAW_PUTPIXEL:
		_vm->_video->putPixel(_destSpriteX, _destSpriteY,
		    _frontColor, *_spritesArray[_destSurface]);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _destSpriteX, _destSpriteY);
		break;

	case DRAW_FILLRECT:
		_vm->_video->fillRect(*_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _destSpriteX + _spriteRight - 1,
		    _destSpriteY + _spriteBottom - 1, _backColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
		break;

	case DRAW_DRAWLINE:
		_vm->_video->drawLine(*_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _frontColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_INVALIDATE:
		dirtiedRect(_destSurface, _destSpriteX - _spriteRight, _destSpriteY - _spriteBottom,
				_destSpriteX + _spriteRight, _destSpriteY + _spriteBottom);
		break;

	case DRAW_LOADSPRITE:
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
		break;

	case DRAW_PRINTTEXT:
		font = _fonts[_fontIndex];
		if (!font) {
			warning("Trying to print \"%s\" with undefined font %d", _textToPrint, _fontIndex);
			break;
		}

		len = strlen(_textToPrint);
		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + len * font->getCharWidth() - 1,
				_destSpriteY + font->getCharHeight() - 1);

		for (int i = 0; i < len; i++) {
			_vm->_video->drawLetter(_textToPrint[i],
			    _destSpriteX, _destSpriteY,
			    *font, _transparency,
			    _frontColor, _backColor,
			    *_spritesArray[_destSurface]);

			_destSpriteX += font->getCharWidth();
		}
		break;

	case DRAW_DRAWBAR:
		_vm->_video->drawLine(*_spritesArray[_destSurface],
		    _destSpriteX, _spriteBottom,
		    _spriteRight, _spriteBottom, _frontColor);

		_vm->_video->drawLine(*_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _destSpriteX, _spriteBottom, _frontColor);

		_vm->_video->drawLine(*_spritesArray[_destSurface],
		    _spriteRight, _destSpriteY,
		    _spriteRight, _spriteBottom, _frontColor);

		_vm->_video->drawLine(*_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _destSpriteY, _frontColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_CLEARRECT:
		if (_backColor < 16) {
			_vm->_video->fillRect(*_spritesArray[_destSurface],
			    _destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom,
			    _backColor);
		}
		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_FILLRECTABS:
		_vm->_video->fillRect(*_spritesArray[_destSurface],
		    _destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _backColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_DRAWLETTER:
		font = _fonts[_fontIndex];
		if (!font) {
			warning("Trying to print \'%c\' with undefined font %d", _letterToPrint, _fontIndex);
			break;
		}

		if (_fontToSprite[_fontIndex].sprite == -1) {
			dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
					_destSpriteX + font->getCharWidth()  - 1,
					_destSpriteY + font->getCharHeight() - 1);
			_vm->_video->drawLetter(_letterToPrint,
			    _destSpriteX, _destSpriteY,
			    *font, _transparency,
			    _frontColor, _backColor,
			    *_spritesArray[_destSurface]);
			break;
		}

		perLine =
			_spritesArray[(int16)_fontToSprite[_fontIndex].sprite]->getWidth() /
			_fontToSprite[_fontIndex].width;

		y = (_letterToPrint - _fontToSprite[_fontIndex].base) / perLine *
			_fontToSprite[_fontIndex].height;

		x = (_letterToPrint - _fontToSprite[_fontIndex].base) % perLine *
			_fontToSprite[_fontIndex].width;

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _fontToSprite[_fontIndex].width,
				_destSpriteY + _fontToSprite[_fontIndex].height);

		_vm->_video->drawSprite(*_spritesArray[(int16)_fontToSprite[_fontIndex].sprite],
		    *_spritesArray[_destSurface], x, y,
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
