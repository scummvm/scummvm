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
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/video.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/anim.h"
#include "gob/parse.h"
#include "gob/cdrom.h"

namespace Gob {

Scenery::Scenery(GobEngine *vm) : _vm(vm) {
	for (int i = 0; i < 20; i++) {
		spriteRefs[i] = 0;
		spriteResId[i] = 0;
	}
	for (int i = 0; i < 70; i++ ) {
		staticPictToSprite[i] = 0;
		animPictToSprite[i] = 0;
	}
	for (int i = 0; i < 10; i++) {
		staticPictCount[i] = 0;
		staticFromExt[i] = 0;
		staticResId[i] = 0;
		animPictCount[i] = 0;
		animFromExt[i] = 0;
		animResId[i] = 0;
	}

	curStatic = 0;
	curStaticLayer = 0;

	toRedrawLeft = 0;
	toRedrawRight = 0;
	toRedrawTop = 0;
	toRedrawBottom = 0;

	animTop = 0;
	animLeft = 0;

	pCaptureCounter = 0;
}

int16 Scenery::loadStatic(char search) {
	int16 tmp;
	int16 *backsPtr;
	int16 picsCount;
	int16 resId;
	int16 i;
	int16 sceneryIndex;
	char *dataPtr;
	Static *ptr;
	int16 offset;
	int16 pictDescId;
	int16 width;
	int16 height;
	int16 sprResId;
	int16 sprIndex;

	_vm->_inter->evalExpr(&sceneryIndex);
	tmp = _vm->_inter->load16();
	backsPtr = (int16 *)_vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr += tmp * 2;
	picsCount = _vm->_inter->load16();
	resId = _vm->_inter->load16();
	if (search) {
		for (i = 0; i < 10; i++) {
			if (staticPictCount[i] != -1 && staticResId[i] == resId) {
				_vm->_global->_inter_execPtr += 8 * staticPictCount[i];
				return i;
			}

			if (staticPictCount[i] == -1 && i < sceneryIndex)
				sceneryIndex = i;
		}
	}

	staticPictCount[sceneryIndex] = picsCount;
	staticResId[sceneryIndex] = resId;

	if (resId >= 30000) {
		staticFromExt[sceneryIndex] = 1;
		dataPtr = _vm->_game->loadExtData(resId, 0, 0);
	} else {
		staticFromExt[sceneryIndex] = 0;
		dataPtr = _vm->_game->loadTotResource(resId);
	}

	ptr = &statics[sceneryIndex];
	ptr->dataPtr = dataPtr;

	ptr->layersCount = (int16)READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	ptr->layers = (StaticLayer **)malloc(sizeof(StaticLayer *) * ptr->layersCount);
	ptr->pieces = (PieceDesc **)malloc(sizeof(PieceDesc *) * picsCount);
	ptr->piecesFromExt = (int8 *)malloc(picsCount);

	for (i = 0; i < ptr->layersCount; i++) {
		offset = (int16)READ_LE_UINT16(&((int16 *)dataPtr)[i]);
		ptr->layers[i] = (StaticLayer *)(dataPtr + offset - 2);

		ptr->layers[i]->planeCount = (int16)READ_LE_UINT16(&ptr->layers[i]->planeCount);

		for (int j = 0; j < ptr->layers[i]->planeCount; ++j) {
			ptr->layers[i]->planes[j].destX = (int16)READ_LE_UINT16(&ptr->layers[i]->planes[j].destX);
			ptr->layers[i]->planes[j].destY = (int16)READ_LE_UINT16(&ptr->layers[i]->planes[j].destY);
		}

		ptr->layers[i]->backResId = (int16)READ_LE_UINT16(backsPtr);
		backsPtr++;
	}

	for (i = 0; i < picsCount; i++) {
		pictDescId = _vm->_inter->load16();
		if (pictDescId >= 30000) {
			ptr->pieces[i] =
			    (PieceDesc *) _vm->_game->loadExtData(pictDescId, 0,
			    0);
			ptr->piecesFromExt[i] = 1;
		} else {
			ptr->pieces[i] =
			    (PieceDesc *)
			    _vm->_game->loadTotResource(pictDescId);
			ptr->piecesFromExt[i] = 0;
		}

		width = _vm->_inter->load16();
		height = _vm->_inter->load16();
		sprResId = _vm->_inter->load16();
		for (sprIndex = 0; sprIndex < 20; sprIndex++) {
			if (spriteResId[sprIndex] == sprResId)
				break;
		}

		if (sprIndex < 20) {
			staticPictToSprite[7 * sceneryIndex + i] =
			    sprIndex;
			spriteRefs[sprIndex]++;
		} else {
			for (sprIndex = 19; _vm->_draw->spritesArray[sprIndex] != 0;
			    sprIndex--);

			staticPictToSprite[7 * sceneryIndex + i] =
			    sprIndex;
			spriteRefs[sprIndex] = 1;
			spriteResId[sprIndex] = sprResId;
			_vm->_draw->spritesArray[sprIndex] =
			    _vm->_video->initSurfDesc(_vm->_global->_videoMode, width, height, 2);

			_vm->_video->clearSurf(_vm->_draw->spritesArray[sprIndex]);
			_vm->_draw->destSurface = sprIndex;
			_vm->_draw->spriteLeft = sprResId;
			_vm->_draw->transparency = 0;
			_vm->_draw->destSpriteX = 0;
			_vm->_draw->destSpriteY = 0;
			_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
		}
	}
	return sceneryIndex + 100;
}

void Scenery::freeStatic(int16 index) {
	int16 i;
	int16 spr;

	if (index == -1)
		_vm->_inter->evalExpr(&index);

	if (staticPictCount[index] == -1)
		return;

	for (i = 0; i < staticPictCount[index]; i++) {
		if (statics[index].piecesFromExt[i] == 1)
			free(statics[index].pieces[i]);

		spr = staticPictToSprite[index * 7 + i];
		spriteRefs[spr]--;
		if (spriteRefs[spr] == 0) {
			_vm->_video->freeSurfDesc(_vm->_draw->spritesArray[spr]);
			_vm->_draw->spritesArray[spr] = 0;
			spriteResId[spr] = -1;
		}
	}

	free(statics[index].layers);
	free(statics[index].pieces);
	free(statics[index].piecesFromExt);
	if (staticFromExt[index] == 1)
		free(statics[index].dataPtr);

	staticFromExt[index] = 0;
	staticPictCount[index] = -1;
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

	ptr = &statics[scenery];
	if (layer >= ptr->layersCount)
		return;

	layerPtr = ptr->layers[layer];

	_vm->_draw->spriteLeft = layerPtr->backResId;
	if (_vm->_draw->spriteLeft != -1) {
		_vm->_draw->destSpriteX = 0;
		_vm->_draw->destSpriteY = 0;
		_vm->_draw->destSurface = 21;
		_vm->_draw->transparency = 0;
		_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
	}

	planeCount = layerPtr->planeCount;
	for (order = 0; order < 10; order++) {
		for (plane = 0, planePtr = layerPtr->planes;
		    plane < planeCount; plane++, planePtr++) {
			if (planePtr->drawOrder != order)
				continue;

			pieceIndex = planePtr->pieceIndex;
			pictIndex = planePtr->pictIndex - 1;

			_vm->_draw->destSpriteX = planePtr->destX;
			_vm->_draw->destSpriteY = planePtr->destY;
			left = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].left);
			right = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].right);
			top = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].top);
			bottom = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].bottom);

			_vm->_draw->sourceSurface =
			    staticPictToSprite[scenery * 7 + pictIndex];
			_vm->_draw->destSurface = 21;
			_vm->_draw->spriteLeft = left;
			_vm->_draw->spriteTop = top;
			_vm->_draw->spriteRight = right - left + 1;
			_vm->_draw->spriteBottom = bottom - top + 1;
			_vm->_draw->transparency = planePtr->transp ? 3 : 0;
			_vm->_draw->spriteOperation(DRAW_BLITSURF);
		}
	}
}

void Scenery::interRenderStatic(void) {
	int16 layer;
	int16 index;

	_vm->_inter->evalExpr(&index);
	_vm->_inter->evalExpr(&layer);
	renderStatic(index, layer);
}

void Scenery::interLoadCurLayer(void) {
	_vm->_inter->evalExpr(&curStatic);
	_vm->_inter->evalExpr(&curStaticLayer);
}

void Scenery::updateStatic(int16 orderFrom) {
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

	if (curStatic == -1)
		return;

	if (curStaticLayer >= statics[curStatic].layersCount)
		return;

	layerPtr = statics[curStatic].layers[curStaticLayer];
	pictPtr = statics[curStatic].pieces;

	planeCount = layerPtr->planeCount;

	for (order = orderFrom; order < 10; order++) {
		for (planePtr = layerPtr->planes, plane = 0;
		    plane < planeCount; plane++, planePtr++) {
			if (planePtr->drawOrder != order)
				continue;

			pieceIndex = planePtr->pieceIndex;
			pictIndex = planePtr->pictIndex - 1;
			_vm->_draw->destSpriteX = planePtr->destX;
			_vm->_draw->destSpriteY = planePtr->destY;

			left = FROM_LE_16(pictPtr[pictIndex][pieceIndex].left);
			right = FROM_LE_16(pictPtr[pictIndex][pieceIndex].right);
			top = FROM_LE_16(pictPtr[pictIndex][pieceIndex].top);
			bottom = FROM_LE_16(pictPtr[pictIndex][pieceIndex].bottom);

			if (_vm->_draw->destSpriteX > toRedrawRight)
				continue;

			if (_vm->_draw->destSpriteY > toRedrawBottom)
				continue;

			if (_vm->_draw->destSpriteX < toRedrawLeft) {
				left += toRedrawLeft - _vm->_draw->destSpriteX;
				_vm->_draw->destSpriteX = toRedrawLeft;
			}

			if (_vm->_draw->destSpriteY < toRedrawTop) {
				top += toRedrawTop - _vm->_draw->destSpriteY;
				_vm->_draw->destSpriteY = toRedrawTop;
			}

			_vm->_draw->spriteLeft = left;
			_vm->_draw->spriteTop = top;
			_vm->_draw->spriteRight = right - left + 1;
			_vm->_draw->spriteBottom = bottom - top + 1;

			if (_vm->_draw->spriteRight <= 0 || _vm->_draw->spriteBottom <= 0)
				continue;

			if (_vm->_draw->destSpriteX + _vm->_draw->spriteRight - 1 >
			    toRedrawRight)
				_vm->_draw->spriteRight =
				    toRedrawRight - _vm->_draw->destSpriteX + 1;

			if (_vm->_draw->destSpriteY + _vm->_draw->spriteBottom - 1 >
			    toRedrawBottom)
				_vm->_draw->spriteBottom =
				    toRedrawBottom - _vm->_draw->destSpriteY + 1;

			_vm->_draw->sourceSurface =
			    staticPictToSprite[curStatic * 7 +
			    pictIndex];
			_vm->_draw->destSurface = 21;
			_vm->_draw->transparency = planePtr->transp ? 3 : 0;
			_vm->_draw->spriteOperation(DRAW_BLITSURF);
		}
	}
}

int16 Scenery::loadAnim(char search) {
	int16 picsCount;
	int16 resId;
	int16 i;
	int16 sceneryIndex;
	char *dataPtr;
	Animation *ptr;
	int16 offset;
	int16 pictDescId;
	int16 width;
	int16 height;
	int16 sprResId;
	int16 sprIndex;

	if (_vm->_cdrom->_cdPlaying) {
		while (_vm->_cdrom->getTrackPos() != -1)
		    _vm->_util->longDelay(50);

		_vm->_cdrom->_cdPlaying = false;
	}

	_vm->_inter->evalExpr(&sceneryIndex);
	picsCount = _vm->_inter->load16();
	resId = _vm->_inter->load16();

	if (search) {
		for (i = 0; i < 10; i++) {
			if (animPictCount[i] != 0
			    && animResId[i] == resId) {
				_vm->_global->_inter_execPtr += 8 * animPictCount[i];
				return i;
			}

			if (animPictCount[i] == 0 && i < sceneryIndex)
				sceneryIndex = i;
		}
	}

	animPictCount[sceneryIndex] = picsCount;
	animResId[sceneryIndex] = resId;

	if (resId >= 30000) {
		animFromExt[sceneryIndex] = 1;
		dataPtr = _vm->_game->loadExtData(resId, 0, 0);
	} else {
		animFromExt[sceneryIndex] = 0;
		dataPtr = _vm->_game->loadTotResource(resId);
	}

	ptr = &animations[sceneryIndex];
	ptr->dataPtr = dataPtr;

	ptr->layersCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	ptr->layers =
	    (AnimLayer **) malloc(sizeof(AnimLayer *) *
	    ptr->layersCount);
	ptr->pieces =
	    (PieceDesc **) malloc(sizeof(PieceDesc *) *
	    picsCount);
	ptr->piecesFromExt = (int8 *) malloc(picsCount);

	for (i = 0; i < ptr->layersCount; i++) {
		offset = (int16)READ_LE_UINT16(&((int16 *)dataPtr)[i]);
		ptr->layers[i] = (AnimLayer *) (dataPtr + offset - 2);

		ptr->layers[i]->unknown0 = (int16)READ_LE_UINT16(&ptr->layers[i]->unknown0);
		ptr->layers[i]->posX = (int16)READ_LE_UINT16(&ptr->layers[i]->posX);
		ptr->layers[i]->posY = (int16)READ_LE_UINT16(&ptr->layers[i]->posY);
		ptr->layers[i]->animDeltaX = (int16)READ_LE_UINT16(&ptr->layers[i]->animDeltaX);
		ptr->layers[i]->animDeltaY = (int16)READ_LE_UINT16(&ptr->layers[i]->animDeltaY);
		ptr->layers[i]->framesCount = (int16)READ_LE_UINT16(&ptr->layers[i]->framesCount);
	}

	for (i = 0; i < picsCount; i++) {
		pictDescId = _vm->_inter->load16();
		if (pictDescId >= 30000) {
			ptr->pieces[i] =
			    (PieceDesc *) _vm->_game->loadExtData(pictDescId, 0,
			    0);
			ptr->piecesFromExt[i] = 1;
		} else {
			ptr->pieces[i] =
			    (PieceDesc *)
			    _vm->_game->loadTotResource(pictDescId);
			ptr->piecesFromExt[i] = 0;
		}

		width = _vm->_inter->load16();
		height = _vm->_inter->load16();
		sprResId = _vm->_inter->load16();
		for (sprIndex = 0; sprIndex < 20; sprIndex++) {
			if (spriteResId[sprIndex] == sprResId)
				break;
		}

		if (sprIndex < 20) {
			animPictToSprite[7 * sceneryIndex + i] = sprIndex;
			spriteRefs[sprIndex]++;
		} else {
			for (sprIndex = 19; _vm->_draw->spritesArray[sprIndex] != 0;
			    sprIndex--);

			animPictToSprite[7 * sceneryIndex + i] = sprIndex;
			spriteRefs[sprIndex] = 1;
			spriteResId[sprIndex] = sprResId;
			_vm->_draw->spritesArray[sprIndex] =
			    _vm->_video->initSurfDesc(_vm->_global->_videoMode, width, height, 2);

			_vm->_video->clearSurf(_vm->_draw->spritesArray[sprIndex]);
			_vm->_draw->destSurface = sprIndex;
			_vm->_draw->spriteLeft = sprResId;
			_vm->_draw->transparency = 0;
			_vm->_draw->destSpriteX = 0;
			_vm->_draw->destSpriteY = 0;
			_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
		}
	}
	return sceneryIndex + 100;
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

	if (layer >= animations[animation].layersCount)
		return;

	layerPtr = animations[animation].layers[layer];

	if (frame >= layerPtr->framesCount)
		return;

	if (flags & 1)		// Do capture
	{
		updateAnim(layer, frame, animation, 0, drawDeltaX,
		    drawDeltaY, 0);

		if (toRedrawLeft == -12345)	// Some magic number?
			return;

		_vm->_game->capturePush(toRedrawLeft, toRedrawTop,
		    toRedrawRight - toRedrawLeft + 1,
		    toRedrawBottom - toRedrawTop + 1);

		*pCaptureCounter = *pCaptureCounter + 1;
	}
	pictPtr = animations[animation].pieces;
	framePtr = layerPtr->frames;

	for (i = 0; i < frame; i++, framePtr++) {
		while (framePtr->notFinal == 1)
			framePtr++;
	}

	if ((flags & 4) == 0) {
		toRedrawLeft = -12345;
	} else {
		toRedrawLeft =
		    MAX(toRedrawLeft, _vm->_anim->_areaLeft);
		toRedrawTop =
		    MAX(toRedrawTop, _vm->_anim->_areaTop);
		toRedrawRight =
		    MIN(toRedrawRight,
		    (int16)(_vm->_anim->_areaLeft + _vm->_anim->_areaWidth - 1));
		toRedrawBottom =
		    MIN(toRedrawBottom,
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
			if (destX < toRedrawLeft) {
				left += toRedrawLeft - destX;
				destX = toRedrawLeft;
			}

			if (left <= right
			    && destX + right - left > toRedrawRight)
				right -=
				    destX + right - left - toRedrawRight;

			if (destY < toRedrawTop) {
				top += toRedrawTop - destY;
				destY = toRedrawTop;
			}

			if (top <= bottom
			    && destY + bottom - top > toRedrawBottom)
				bottom -=
				    destY + bottom - top - toRedrawBottom;
		}

		if (left > right || top > bottom)
			continue;

		if (doDraw) {
			_vm->_draw->sourceSurface =
			    animPictToSprite[animation * 7 + pictIndex];
			_vm->_draw->destSurface = 21;

			_vm->_draw->spriteLeft = left;
			_vm->_draw->spriteTop = top;
			_vm->_draw->spriteRight = right - left + 1;
			_vm->_draw->spriteBottom = bottom - top + 1;
			_vm->_draw->destSpriteX = destX;
			_vm->_draw->destSpriteY = destY;
			_vm->_draw->transparency = transp;
			_vm->_draw->spriteOperation(DRAW_BLITSURF);
		}

		if ((flags & 4) == 0) {
			if (toRedrawLeft == -12345) {
				toRedrawLeft = destX;
				animLeft = destX;
				toRedrawTop = destY;
				animTop = destY;
				toRedrawRight = destX + right - left;
				toRedrawBottom = destY + bottom - top;
			} else {
				toRedrawLeft =
				    MIN(toRedrawLeft, destX);
				toRedrawTop =
				    MIN(toRedrawTop, destY);
				toRedrawRight =
				    MAX(toRedrawRight,
				    (int16)(destX + right - left));
				toRedrawBottom =
				    MAX(toRedrawBottom,
				    (int16)(destY + bottom - top));
			}
		}
	} while (framePtr->notFinal == 1);
}

void Scenery::freeAnim(int16 animation) {
	int16 i;
	int16 spr;

	if (animation == -1)
		_vm->_inter->evalExpr(&animation);

	if (animPictCount[animation] == 0)
		return;

	for (i = 0; i < animPictCount[animation]; i++) {
		if (animations[animation].piecesFromExt[i] == 1)
			free(animations[animation].pieces[i]);

		spr = animPictToSprite[animation * 7 + i];
		spriteRefs[spr]--;
		if (spriteRefs[spr] == 0) {
			_vm->_video->freeSurfDesc(_vm->_draw->spritesArray[spr]);

			_vm->_draw->spritesArray[spr] = 0;
			spriteResId[spr] = -1;
		}
	}

	free(animations[animation].layers);
	free(animations[animation].pieces);
	free(animations[animation].piecesFromExt);
	if (animFromExt[animation] == 1)
		free(animations[animation].dataPtr);

	animFromExt[animation] = 0;
	animPictCount[animation] = 0;
}

void Scenery::interUpdateAnim(void) {
	int16 deltaX;
	int16 deltaY;
	int16 flags;
	int16 frame;
	int16 layer;
	int16 animation;

	_vm->_inter->evalExpr(&deltaX);
	_vm->_inter->evalExpr(&deltaY);
	_vm->_inter->evalExpr(&animation);
	_vm->_inter->evalExpr(&layer);
	_vm->_inter->evalExpr(&frame);
	flags = _vm->_inter->load16();
	updateAnim(layer, frame, animation, flags, deltaX, deltaY, 1);
}

void Scenery::interStoreParams(void) {
	AnimLayer *layerPtr;
	int16 animation;
	int16 layer;
	int16 var;

	warning("interStoreParams: Storing...");

	_vm->_inter->evalExpr(&animation);
	_vm->_inter->evalExpr(&layer);
	layerPtr = animations[animation].layers[layer];

	var = _vm->_parse->parseVarIndex();
	WRITE_VAR_OFFSET(var, layerPtr->animDeltaX);

	var = _vm->_parse->parseVarIndex();
	WRITE_VAR_OFFSET(var, layerPtr->animDeltaY);

	var = _vm->_parse->parseVarIndex();
	WRITE_VAR_OFFSET(var, layerPtr->unknown0);

	var = _vm->_parse->parseVarIndex();
	WRITE_VAR_OFFSET(var, layerPtr->framesCount);
}

}				// End of namespace Gob
