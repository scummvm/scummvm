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
#include "gob/scenery.h"
#include "gob/anim.h"
#include "gob/draw.h"
#include "gob/game.h"

namespace Gob {

Scenery_v2::Scenery_v2(GobEngine *vm) : Scenery_v1(vm) {
}

// flags & 1 - do capture all area animation is occupying
// flags & 4 == 0 - calculate animation final size
// flags & 2 != 0 - don't check with "toRedraw"'s
// flags & 4 != 0 - checkk view toRedraw
void Scenery_v2::updateAnim(int16 layer, int16 frame, int16 animation, int16 flags,
	    int16 drawDeltaX, int16 drawDeltaY, char doDraw) {
	AnimLayer *layerPtr;
	PieceDesc **pictPtr;
	AnimFramePiece *framePtr;

	uint16 pieceIndex;
	uint16 pictIndex;

	int16 left;
	int16 right;
	int16 top;
	int16 bottom;

	byte highX;
	byte highY;

	int16 i;
	int16 transp;

	int16 destX;
	int16 destY;

	if ((_animPictCount[animation] == 0) || (layer < 0))
		return;
	if (layer >= _animations[animation].layersCount)
		return;

	layerPtr = &_animations[animation].layers[layer];

	if (frame >= layerPtr->framesCount)
		return;

	if (flags & 1)		// Do capture
	{
		updateAnim(layer, frame, animation, 0, drawDeltaX,
		    drawDeltaY, 0);

		if (_toRedrawLeft == -12345)	// Some magic number?
			return;

		_vm->_game->capturePush(_toRedrawLeft, _toRedrawTop,
		    _toRedrawRight - _toRedrawLeft + 1,
		    _toRedrawBottom - _toRedrawTop + 1);

		*_pCaptureCounter = *_pCaptureCounter + 1;
	}

	pictPtr = _animations[animation].pieces;
	framePtr = layerPtr->frames;

	for (i = 0; i < frame; i++, framePtr++) {
		while (framePtr->notFinal == 1)
			framePtr++;
	}

	if ((flags & 4) == 0) {
		_toRedrawLeft = -12345;
	} else {
		_toRedrawLeft =
		    MAX(_toRedrawLeft, _vm->_anim->_areaLeft);
		_toRedrawTop =
		    MAX(_toRedrawTop, _vm->_anim->_areaTop);
		_toRedrawRight =
		    MIN(_toRedrawRight,
		    (int16)(_vm->_anim->_areaLeft + _vm->_anim->_areaWidth - 1));
		_toRedrawBottom =
		    MIN(_toRedrawBottom,
		    (int16)(_vm->_anim->_areaTop + _vm->_anim->_areaHeight - 1));
	}

	transp = layerPtr->transp ? 3 : 0;

	framePtr--;
	do {
		framePtr++;

		pieceIndex = framePtr->pieceIndex;
		pictIndex = framePtr->pictIndex;

		destX = framePtr->destX;
		destY = framePtr->destY;

		highX = pictIndex & 0xc0;
		highY = pictIndex & 0x30;
		highX >>= 6;
		highY >>= 4;
		if (destX >= 0)
			destX += ((uint16)highX) << 7;
		else
			destX -= ((uint16)highX) << 7;

		if (destY >= 0)
			destY += ((uint16)highY) << 7;
		else
			destY -= ((uint16)highY) << 7;

		if (drawDeltaX == 1000)
			destX += layerPtr->posX;
		else
			destX += drawDeltaX;

		if (drawDeltaY == 1000)
			destY += layerPtr->posY;
		else
			destY += drawDeltaY;

		pictIndex = (pictIndex & 15) - 1;

		left = FROM_LE_16(pictPtr[pictIndex][pieceIndex].left);
		right = FROM_LE_16(pictPtr[pictIndex][pieceIndex].right);
		top = FROM_LE_16(pictPtr[pictIndex][pieceIndex].top);
		bottom = FROM_LE_16(pictPtr[pictIndex][pieceIndex].bottom);

		if (flags & 2) {
			if (destX < _vm->_anim->_areaLeft) {
				left += _vm->_anim->_areaLeft - destX;
				destX = _vm->_anim->_areaLeft;
			}

			if (left <= right
			    && destX + right - left >=
			    _vm->_anim->_areaLeft + _vm->_anim->_areaWidth)
				right -=
				    (destX + right - left) -
				    (_vm->_anim->_areaLeft + _vm->_anim->_areaWidth) +
				    1;

			if (destY < _vm->_anim->_areaTop) {
				top += _vm->_anim->_areaTop - destY;
				destY = _vm->_anim->_areaTop;
			}

			if (top <= bottom
			    && destY + bottom - top >=
			    _vm->_anim->_areaTop + _vm->_anim->_areaHeight)
				bottom -=
				    (destY + bottom - top) -
				    (_vm->_anim->_areaTop + _vm->_anim->_areaHeight) +
				    1;

		} else if (flags & 4) {
			if (destX < _toRedrawLeft) {
				left += _toRedrawLeft - destX;
				destX = _toRedrawLeft;
			}

			if (left <= right
			    && destX + right - left > _toRedrawRight)
				right -=
				    destX + right - left - _toRedrawRight;

			if (destY < _toRedrawTop) {
				top += _toRedrawTop - destY;
				destY = _toRedrawTop;
			}

			if (top <= bottom
			    && destY + bottom - top > _toRedrawBottom)
				bottom -=
				    destY + bottom - top - _toRedrawBottom;
		}

		// ---

		if (left > right || top > bottom)
			continue;

		if (doDraw) {
			_vm->_draw->_sourceSurface =
			    _animPictToSprite[animation * 7 + pictIndex];
			_vm->_draw->_destSurface = 21;

			_vm->_draw->_spriteLeft = left;
			_vm->_draw->_spriteTop = top;
			_vm->_draw->_spriteRight = right - left + 1;
			_vm->_draw->_spriteBottom = bottom - top + 1;
			_vm->_draw->_destSpriteX = destX;
			_vm->_draw->_destSpriteY = destY;
			_vm->_draw->_transparency = transp;
			_vm->_draw->spriteOperation(DRAW_DRAWLETTER);
		}

		if ((flags & 4) == 0) {
			if (_toRedrawLeft == -12345) {
				_toRedrawLeft = destX;
				_animLeft = destX;
				_toRedrawTop = destY;
				_animTop = destY;
				_toRedrawRight = destX + right - left;
				_animRight = destX + right - left;
				_toRedrawBottom = destY + bottom - top;
				_animBottom = destY + bottom - top;
			} else {
				_toRedrawLeft =
				    MIN(_toRedrawLeft, destX);
				_toRedrawTop =
				    MIN(_toRedrawTop, destY);
				_toRedrawRight =
				    MAX(_toRedrawRight,
				    (int16)(destX + right - left));
				_toRedrawBottom =
				    MAX(_toRedrawBottom,
				    (int16)(destY + bottom - top));
			}
		}
	} while (framePtr->notFinal == 1);
}

} // End of namespace Gob
