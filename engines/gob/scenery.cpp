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
#include "common/stream.h"

#include "gob/gob.h"
#include "gob/scenery.h"
#include "gob/global.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/inter.h"

namespace Gob {

Scenery::Scenery(GobEngine *vm) : _vm(vm) {
	for (int i = 0; i < 20; i++) {
		_spriteRefs[i] = 0;
		_spriteResId[i] = 0;
	}

	for (int i = 0; i < 10; i++) {
		_staticPictCount[i] = 0;
		_staticResId[i] = 0;
		_animPictCount[i] = 0;
		_animResId[i] = 0;
	}

	_curStatic = 0;
	_curStaticLayer = 0;

	_toRedrawLeft = 0;
	_toRedrawRight = 0;
	_toRedrawTop = 0;
	_toRedrawBottom = 0;

	_animTop = 0;
	_animLeft = 0;

	_pCaptureCounter = 0;

	for (int i = 0; i < 70; i++ ) {
		_staticPictToSprite[i] = 0;
		_animPictToSprite[i] = 0;
	}
}

Scenery::~Scenery() {
	for (int i = 0; i < 10; i++) {
		freeStatic(i);
		freeAnim(i);
	}
}

void Scenery::init() {
	for (int i = 0; i < 10; i++) {
		_animPictCount[i] = 0;
		_staticPictCount[i] = -1;
	}

	for (int i = 0; i < 20; i++) {
		_spriteRefs[i] = 0;
		_spriteResId[i] = -1;
	}

	_curStaticLayer = -1;
	_curStatic = -1;
}

int16 Scenery::loadStatic(char search) {
	int16 size;
	int16 *backsPtr;
	int16 picsCount;
	int16 resId;
	int16 sceneryIndex;
	byte *extData = 0;
	byte *dataPtr;
	Static *ptr;
	int16 pictDescId;
	int16 width;
	int16 height;
	int16 sprResId;
	int16 sprIndex;

	_vm->_inter->evalExpr(&sceneryIndex);

	size = _vm->_inter->load16();
	backsPtr = (int16 *) _vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr += size * 2;
	picsCount = _vm->_inter->load16();
	resId = _vm->_inter->load16();

	if (search) {
		int i;

		for (i = 0; i < 10; i++) {
			if ((_staticPictCount[i] != -1) && (_staticResId[i] == resId)) {
				_vm->_global->_inter_execPtr += 8 * _staticPictCount[i];
				return i;
			}

			if (_staticPictCount[i] == -1 && i < sceneryIndex)
				sceneryIndex = i;
		}
	}

	_staticPictCount[sceneryIndex] = picsCount;
	_staticResId[sceneryIndex] = resId;

	if (resId >= 30000) {
		extData = _vm->_game->loadExtData(resId, 0, 0);
		dataPtr = extData;
	} else
		dataPtr = _vm->_game->loadTotResource(resId);

	ptr = &_statics[sceneryIndex];

	ptr->layersCount = (int16) READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	ptr->layers = new StaticLayer[ptr->layersCount];
	ptr->pieces = new PieceDesc*[picsCount];
	ptr->piecesFromExt = new bool[picsCount];

	for (int i = 0; i < ptr->layersCount; i++) {
		int16 offset = READ_LE_UINT16(dataPtr + i * 2);
		Common::MemoryReadStream layerData(dataPtr + offset, 4294967295U);

		ptr->layers[i].planeCount = layerData.readSint16LE();

		ptr->layers[i].planes = new StaticPlane[ptr->layers[i].planeCount];
		for (int j = 0; j < ptr->layers[i].planeCount; ++j) {
			ptr->layers[i].planes[j].pictIndex = layerData.readSByte();
			ptr->layers[i].planes[j].pieceIndex = layerData.readSByte();
			ptr->layers[i].planes[j].drawOrder = layerData.readSByte();
			ptr->layers[i].planes[j].destX = layerData.readSint16LE();
			ptr->layers[i].planes[j].destY = layerData.readSint16LE();
			ptr->layers[i].planes[j].transp = layerData.readSByte();
		}

		ptr->layers[i].backResId = (int16) READ_LE_UINT16(backsPtr);
		backsPtr++;
	}

	for (int i = 0; i < picsCount; i++) {
		pictDescId = _vm->_inter->load16();

		if (pictDescId >= 30000) {
			ptr->pieces[i] =
				(PieceDesc *) _vm->_game->loadExtData(pictDescId, 0, 0);
			ptr->piecesFromExt[i] = true;
		} else {
			ptr->pieces[i] =
				(PieceDesc *) _vm->_game->loadTotResource(pictDescId);
			ptr->piecesFromExt[i] = false;
		}

		width = _vm->_inter->load16();
		height = _vm->_inter->load16();
		sprResId = _vm->_inter->load16();
		for (sprIndex = 0; sprIndex < 20; sprIndex++) {
			if (_spriteResId[sprIndex] == sprResId)
				break;
		}

		if (sprIndex < 20) {
			_staticPictToSprite[7 * sceneryIndex + i] = sprIndex;
			_spriteRefs[sprIndex]++;
		} else {
			for (sprIndex = 19; _vm->_draw->_spritesArray[sprIndex] != 0;
				sprIndex--);

			_staticPictToSprite[7 * sceneryIndex + i] = sprIndex;
			_spriteRefs[sprIndex] = 1;
			_spriteResId[sprIndex] = sprResId;
			_vm->_draw->initSpriteSurf(sprIndex, width, height, 2);

			_vm->_video->clearSurf(_vm->_draw->_spritesArray[sprIndex]);
			_vm->_draw->_destSurface = sprIndex;
			_vm->_draw->_spriteLeft = sprResId;
			_vm->_draw->_transparency = 0;
			_vm->_draw->_destSpriteX = 0;
			_vm->_draw->_destSpriteY = 0;
			_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
		}
	}

	delete[] extData;

	return sceneryIndex + 100;
}

void Scenery::freeStatic(int16 index) {
	int16 spr;

	if (index == -1)
		_vm->_inter->evalExpr(&index);

	if (_staticPictCount[index] == -1)
		return;

	for (int i = 0; i < _staticPictCount[index]; i++) {
		if (_statics[index].piecesFromExt[i])
			delete[] _statics[index].pieces[i];

		spr = _staticPictToSprite[index * 7 + i];
		_spriteRefs[spr]--;
		if (_spriteRefs[spr] == 0) {
			_vm->_draw->freeSprite(spr);
			_spriteResId[spr] = -1;
		}
	}

	for (int i = 0; i < _statics[index].layersCount; i++)
		delete[] _statics[index].layers[i].planes;
	delete[] _statics[index].layers;
	delete[] _statics[index].pieces;
	delete[] _statics[index].piecesFromExt;

	_statics[index].layersCount = 0;
	_staticPictCount[index] = -1;
}

void Scenery::renderStatic(int16 scenery, int16 layer) {
	Static *ptr;
	StaticLayer *layerPtr;
	StaticPlane *planePtr;
	int16 planeCount;
	int16 order;
	int16 plane;

	int16 pieceIndex;
	int16 pictIndex;

	int16 left;
	int16 right;
	int16 top;
	int16 bottom;

	ptr = &_statics[scenery];
	if (layer >= ptr->layersCount)
		return;

	layerPtr = &ptr->layers[layer];

	_vm->_draw->_spriteLeft = layerPtr->backResId;
	if (_vm->_draw->_spriteLeft != -1) {
		_vm->_draw->_destSpriteX = 0;
		_vm->_draw->_destSpriteY = 0;
		_vm->_draw->_destSurface = 21;
		_vm->_draw->_transparency = 0;
		_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
	}

	planeCount = layerPtr->planeCount;
	for (order = 0; order < 40; order++) {
		for (plane = 0, planePtr = layerPtr->planes;
		    plane < planeCount; plane++, planePtr++) {
			if (planePtr->drawOrder != order)
				continue;

			pieceIndex = planePtr->pieceIndex;
			pictIndex = planePtr->pictIndex - 1;

			_vm->_draw->_destSpriteX = planePtr->destX;
			_vm->_draw->_destSpriteY = planePtr->destY;
			left = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].left);
			right = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].right);
			top = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].top);
			bottom = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].bottom);

			_vm->_draw->_sourceSurface =
			    _staticPictToSprite[scenery * 7 + pictIndex];
			_vm->_draw->_destSurface = 21;
			_vm->_draw->_spriteLeft = left;
			_vm->_draw->_spriteTop = top;
			_vm->_draw->_spriteRight = right - left + 1;
			_vm->_draw->_spriteBottom = bottom - top + 1;
			_vm->_draw->_transparency = planePtr->transp ? 3 : 0;
			_vm->_draw->spriteOperation(DRAW_BLITSURF);
		}
	}
}

void Scenery::updateStatic(int16 orderFrom, byte index, byte layer) {
	StaticLayer *layerPtr;
	PieceDesc **pictPtr;
	StaticPlane *planePtr;
	int16 planeCount;
	int16 order;
	int16 plane;
	int16 pieceIndex;
	int16 pictIndex;

	int16 left;
	int16 right;
	int16 top;
	int16 bottom;

	if (layer >= _statics[index].layersCount)
		return;

	layerPtr = &_statics[index].layers[layer];
	pictPtr = _statics[index].pieces;

	planeCount = layerPtr->planeCount;

	for (order = orderFrom; order < 40; order++) {
		for (planePtr = layerPtr->planes, plane = 0;
		    plane < planeCount; plane++, planePtr++) {
			if (planePtr->drawOrder != order)
				continue;

			pieceIndex = planePtr->pieceIndex;
			pictIndex = planePtr->pictIndex - 1;
			_vm->_draw->_destSpriteX = planePtr->destX;
			_vm->_draw->_destSpriteY = planePtr->destY;

			left = FROM_LE_16(pictPtr[pictIndex][pieceIndex].left);
			right = FROM_LE_16(pictPtr[pictIndex][pieceIndex].right);
			top = FROM_LE_16(pictPtr[pictIndex][pieceIndex].top);
			bottom = FROM_LE_16(pictPtr[pictIndex][pieceIndex].bottom);

			if (_vm->_draw->_destSpriteX > _toRedrawRight)
				continue;

			if (_vm->_draw->_destSpriteY > _toRedrawBottom)
				continue;

			if (_vm->_draw->_destSpriteX < _toRedrawLeft) {
				left += _toRedrawLeft - _vm->_draw->_destSpriteX;
				_vm->_draw->_destSpriteX = _toRedrawLeft;
			}

			if (_vm->_draw->_destSpriteY < _toRedrawTop) {
				top += _toRedrawTop - _vm->_draw->_destSpriteY;
				_vm->_draw->_destSpriteY = _toRedrawTop;
			}

			_vm->_draw->_spriteLeft = left;
			_vm->_draw->_spriteTop = top;
			_vm->_draw->_spriteRight = right - left + 1;
			_vm->_draw->_spriteBottom = bottom - top + 1;

			if ((_vm->_draw->_spriteRight <= 0) ||
			    (_vm->_draw->_spriteBottom <= 0))
				continue;

			if ((_vm->_draw->_destSpriteX + _vm->_draw->_spriteRight - 1) >
			    _toRedrawRight)
				_vm->_draw->_spriteRight =
				    _toRedrawRight - _vm->_draw->_destSpriteX + 1;

			if ((_vm->_draw->_destSpriteY + _vm->_draw->_spriteBottom - 1) >
			    _toRedrawBottom)
				_vm->_draw->_spriteBottom =
				    _toRedrawBottom - _vm->_draw->_destSpriteY + 1;

			_vm->_draw->_sourceSurface =
			    _staticPictToSprite[index * 7 + pictIndex];
			_vm->_draw->_destSurface = 21;
			_vm->_draw->_transparency = planePtr->transp ? 3 : 0;
			_vm->_draw->spriteOperation(DRAW_BLITSURF);
		}
	}
}

void Scenery::updateStatic(int16 orderFrom) {
	if (_curStatic == -1)
		return;

	updateStatic(orderFrom, _curStatic & 0xFF, _curStaticLayer & 0xFF);

	if (_curStatic & 0xFF00)
		updateStatic(orderFrom, ((_curStatic >> 8) & 0xFF) - 1,
				(_curStaticLayer >> 8) & 0xFF);
}

int16 Scenery::loadAnim(char search) {
	int16 picsCount;
	int16 resId;
	int16 i;
	int16 j;
	int16 sceneryIndex;
	int16 framesCount;
	byte *extData;
	byte *dataPtr;
	Animation *ptr;
	int16 pictDescId;
	int16 width;
	int16 height;
	int16 sprResId;
	int16 sprIndex;
	uint32 layerPos;

	extData = 0;
	_vm->_inter->evalExpr(&sceneryIndex);
	picsCount = _vm->_inter->load16();
	resId = _vm->_inter->load16();

	if (search) {
		for (i = 0; i < 10; i++) {
			if ((_animPictCount[i] != 0) && (_animResId[i] == resId)) {
				_vm->_global->_inter_execPtr += 8 * _animPictCount[i];
				return i;
			}

			if ((_animPictCount[i] == 0) && (i < sceneryIndex))
				sceneryIndex = i;
		}
	}

	_animPictCount[sceneryIndex] = picsCount;
	_animResId[sceneryIndex] = resId;

	if (resId >= 30000) {
		extData = _vm->_game->loadExtData(resId, 0, 0);
		dataPtr = extData;
	} else
		dataPtr = _vm->_game->loadTotResource(resId);

	ptr = &_animations[sceneryIndex];

	ptr->layersCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	ptr->layers = new AnimLayer[ptr->layersCount];
	ptr->pieces = new PieceDesc*[picsCount];
	ptr->piecesFromExt = new bool[picsCount];

	for (i = 0; i < ptr->layersCount; i++) {
		int16 offset = READ_LE_UINT16(dataPtr + i * 2);
		Common::MemoryReadStream layerData(dataPtr + offset - 2, 4294967295U);

		ptr->layers[i].unknown0 = layerData.readSint16LE();
		ptr->layers[i].posX = layerData.readSint16LE();
		ptr->layers[i].posY = layerData.readSint16LE();
		ptr->layers[i].animDeltaX = layerData.readSint16LE();
		ptr->layers[i].animDeltaY = layerData.readSint16LE();
		ptr->layers[i].transp = layerData.readSByte();
		ptr->layers[i].framesCount = layerData.readSint16LE();

		layerPos = layerData.pos();
		framesCount = 0;
		layerData.seek(4, SEEK_CUR);
		for (j = 0; j < ptr->layers[i].framesCount;
				j++, framesCount++, layerData.seek(4, SEEK_CUR)) {
			while (layerData.readByte() == 1) {
				framesCount++;
				layerData.seek(4, SEEK_CUR);
			}
		}
		layerData.seek(layerPos);

		ptr->layers[i].frames = new AnimFramePiece[framesCount];
		for (j = 0; j < framesCount; j++) {
			ptr->layers[i].frames[j].pictIndex = layerData.readByte();
			ptr->layers[i].frames[j].pieceIndex = layerData.readByte();
			ptr->layers[i].frames[j].destX = layerData.readSByte();
			ptr->layers[i].frames[j].destY = layerData.readSByte();
			ptr->layers[i].frames[j].notFinal = layerData.readSByte();
		}
	}

	for (i = 0; i < picsCount; i++) {
		pictDescId = _vm->_inter->load16();
		if (pictDescId >= 30000) {
			ptr->pieces[i] =
				(PieceDesc *) _vm->_game->loadExtData(pictDescId, 0, 0);
			ptr->piecesFromExt[i] = true;
		} else {
			ptr->pieces[i] =
				(PieceDesc *) _vm->_game->loadTotResource(pictDescId);
			ptr->piecesFromExt[i] = false;
		}

		width = _vm->_inter->load16();
		height = _vm->_inter->load16();
		sprResId = _vm->_inter->load16();
		for (sprIndex = 0; sprIndex < 20; sprIndex++)
			if (_spriteResId[sprIndex] == sprResId)
				break;

		if (sprIndex < 20) {
			_animPictToSprite[7 * sceneryIndex + i] = sprIndex;
			_spriteRefs[sprIndex]++;
		} else {
			for (sprIndex = 19; _vm->_draw->_spritesArray[sprIndex] != 0;
				sprIndex--);

			_animPictToSprite[7 * sceneryIndex + i] = sprIndex;
			_spriteRefs[sprIndex] = 1;
			_spriteResId[sprIndex] = sprResId;
			_vm->_draw->initSpriteSurf(sprIndex, width, height, 2);

			_vm->_video->clearSurf(_vm->_draw->_spritesArray[sprIndex]);
			_vm->_draw->_destSurface = sprIndex;
			_vm->_draw->_spriteLeft = sprResId;
			_vm->_draw->_transparency = 0;
			_vm->_draw->_destSpriteX = 0;
			_vm->_draw->_destSpriteY = 0;
			_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
		}
	}

	delete[] extData;

	return sceneryIndex + 100;
}

void Scenery::freeAnim(int16 index) {
	int16 spr;

	if (index == -1)
		_vm->_inter->evalExpr(&index);

	if (_animPictCount[index] == 0)
		return;

	for (int i = 0; i < _animPictCount[index]; i++) {
		if (_animations[index].piecesFromExt[i])
			delete[] _animations[index].pieces[i];

		spr = _animPictToSprite[index * 7 + i];
		_spriteRefs[spr]--;
		if (_spriteRefs[spr] == 0) {
			_vm->_draw->freeSprite(spr);
			_spriteResId[spr] = -1;
		}
	}
	
	for (int i = 0; i < _animations[index].layersCount; i++)
		delete[] _animations[index].layers[i].frames;
	delete[] _animations[index].layers;
	delete[] _animations[index].pieces;
	delete[] _animations[index].piecesFromExt;

	_animPictCount[index] = 0;
}

// flags & 1 - do capture all area animation is occupying
// flags & 4 == 0 - calculate animation final size
// flags & 2 != 0 - don't check with "toRedraw"'s
// flags & 4 != 0 - checkk view toRedraw
void Scenery::updateAnim(int16 layer, int16 frame, int16 animation, int16 flags,
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

	if (flags & 1) { // Do capture
		updateAnim(layer, frame, animation, 0, drawDeltaX, drawDeltaY, 0);

		if (_toRedrawLeft == -12345)
			return;

		_vm->_game->capturePush(_toRedrawLeft, _toRedrawTop,
		    _toRedrawRight - _toRedrawLeft + 1,
		    _toRedrawBottom - _toRedrawTop + 1);

		*_pCaptureCounter = *_pCaptureCounter + 1;
	}

	pictPtr = _animations[animation].pieces;
	framePtr = layerPtr->frames;

	for (i = 0; i < frame; i++, framePtr++)
		while (framePtr->notFinal == 1)
			framePtr++;

	if (flags & 4) {
		_toRedrawLeft = MAX(_toRedrawLeft, _vm->_mult->_animLeft);
		_toRedrawTop = MAX(_toRedrawTop, _vm->_mult->_animTop);
		_toRedrawRight = MIN(_toRedrawRight,
		    (int16)(_vm->_mult->_animLeft + _vm->_mult->_animWidth - 1));
		_toRedrawBottom = MIN(_toRedrawBottom,
		    (int16)(_vm->_mult->_animTop + _vm->_mult->_animHeight - 1));
	} else
		_toRedrawLeft = -12345;

	transp = layerPtr->transp ? 3 : 0;

	framePtr--;
	do {
		framePtr++;

		pieceIndex = framePtr->pieceIndex;
		pictIndex = framePtr->pictIndex;

		destX = framePtr->destX;
		destY = framePtr->destY;

		highX = pictIndex & 0xC0;
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

		left = READ_LE_UINT16(&pictPtr[pictIndex][pieceIndex].left);
		right = READ_LE_UINT16(&pictPtr[pictIndex][pieceIndex].right);
		top = READ_LE_UINT16(&pictPtr[pictIndex][pieceIndex].top);
		bottom = READ_LE_UINT16(&pictPtr[pictIndex][pieceIndex].bottom);

		if (flags & 2) {
			if (destX < _vm->_mult->_animLeft) {
				left += _vm->_mult->_animLeft - destX;
				destX = _vm->_mult->_animLeft;
			}

			if ((left <= right) && ((destX + right - left) >=
			    (_vm->_mult->_animLeft + _vm->_mult->_animWidth)))
				right -= (destX + right - left) -
					(_vm->_mult->_animLeft + _vm->_mult->_animWidth) + 1;

			if (destY < _vm->_mult->_animTop) {
				top += _vm->_mult->_animTop - destY;
				destY = _vm->_mult->_animTop;
			}

			if ((top <= bottom) && ((destY + bottom - top) >=
						(_vm->_mult->_animTop + _vm->_mult->_animHeight)))
				bottom -= (destY + bottom - top) -
					(_vm->_mult->_animTop + _vm->_mult->_animHeight) + 1;

		} else if (flags & 4) {
			if (destX < _toRedrawLeft) {
				left += _toRedrawLeft - destX;
				destX = _toRedrawLeft;
			}

			if ((left <= right) && ((destX + right - left) > _toRedrawRight))
				right -= destX + right - left - _toRedrawRight;

			if (destY < _toRedrawTop) {
				top += _toRedrawTop - destY;
				destY = _toRedrawTop;
			}

			if ((top <= bottom) && ((destY + bottom - top) > _toRedrawBottom))
				bottom -= destY + bottom - top - _toRedrawBottom;
		}

		if ((left > right) || (top > bottom))
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
			_vm->_draw->spriteOperation(DRAW_BLITSURF);
		}

		if (!(flags & 4)) {
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
				_toRedrawLeft = MIN(_toRedrawLeft, destX);
				_toRedrawTop = MIN(_toRedrawTop, destY);
				_toRedrawRight =
					MAX(_toRedrawRight, (int16)(destX + right - left));
				_toRedrawBottom =
					MAX(_toRedrawBottom, (int16)(destY + bottom - top));
			}
		}

	} while (framePtr->notFinal == 1);
}

void Scenery::writeAnimLayerInfo(uint16 index, uint16 layer,
		int16 varDX, int16 varDY, int16 varUnk0, int16 varFrames) {

	assert(index < 10);
	assert(layer < _animations[index].layersCount);

	AnimLayer &animLayer = _animations[index].layers[layer];
	WRITE_VAR_OFFSET(varDX, animLayer.animDeltaX);
	WRITE_VAR_OFFSET(varDY, animLayer.animDeltaY);
	WRITE_VAR_OFFSET(varUnk0, animLayer.unknown0);
	WRITE_VAR_OFFSET(varFrames, animLayer.framesCount);
}

int16 Scenery::getStaticLayersCount(uint16 index) {
	assert(index < 10);

	return _statics[index].layersCount;
}

int16 Scenery::getAnimLayersCount(uint16 index) {
	assert(index < 10);

	return _animations[index].layersCount;
}

Scenery::StaticLayer *Scenery::getStaticLayer(uint16 index, uint16 layer) {
	assert(index < 10);
	assert(layer < _statics[index].layersCount);

	return &_statics[index].layers[layer];
}

Scenery::AnimLayer *Scenery::getAnimLayer(uint16 index, uint16 layer) {
	assert(index < 10);
	assert(layer < _animations[index].layersCount);

	return &_animations[index].layers[layer];
}

} // End of namespace Gob
