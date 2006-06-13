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
#include "common/stream.h"

#include "gob/gob.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/video.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/parse.h"
#include "gob/cdrom.h"

namespace Gob {

Scenery::Scenery(GobEngine *vm) : _vm(vm) {
	int i;

	for (i = 0; i < 20; i++) {
		_spriteRefs[i] = 0;
		_spriteResId[i] = 0;
	}
	for (i = 0; i < 70; i++ ) {
		_staticPictToSprite[i] = 0;
		_animPictToSprite[i] = 0;
	}
	for (i = 0; i < 10; i++) {
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
}

int16 Scenery::loadStatic(char search) {
	int16 tmp;
	int16 *backsPtr;
	int16 picsCount;
	int16 resId;
	int16 i;
	int16 sceneryIndex;
	char *extData;
	char *dataPtr;
	Static *ptr;
	int16 offset;
	int16 pictDescId;
	int16 width;
	int16 height;
	int16 sprResId;
	int16 sprIndex;

	extData = 0;
	_vm->_inter->evalExpr(&sceneryIndex);
	tmp = _vm->_inter->load16();
	backsPtr = (int16 *)_vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr += tmp * 2;
	picsCount = _vm->_inter->load16();
	resId = _vm->_inter->load16();
	if (search) {
		for (i = 0; i < 10; i++) {
			if (_staticPictCount[i] != -1 && _staticResId[i] == resId) {
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

	ptr->layersCount = (int16)READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	ptr->layers = new StaticLayer[ptr->layersCount];
	ptr->pieces = new PieceDesc*[picsCount];
	ptr->piecesFromExt = new int8[picsCount];

	for (i = 0; i < ptr->layersCount; i++) {
		offset = (int16)READ_LE_UINT16(&((int16 *)dataPtr)[i]);
		Common::MemoryReadStream layerData((byte *) (dataPtr + offset), 4294967295U);

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

		ptr->layers[i].backResId = (int16)READ_LE_UINT16(backsPtr);
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
			if (_spriteResId[sprIndex] == sprResId)
				break;
		}

		if (sprIndex < 20) {
			_staticPictToSprite[7 * sceneryIndex + i] =
			    sprIndex;
			_spriteRefs[sprIndex]++;
		} else {
			for (sprIndex = 19; _vm->_draw->_spritesArray[sprIndex] != 0;
			    sprIndex--);

			_staticPictToSprite[7 * sceneryIndex + i] =
			    sprIndex;
			_spriteRefs[sprIndex] = 1;
			_spriteResId[sprIndex] = sprResId;
			_vm->_draw->_spritesArray[sprIndex] =
			    _vm->_video->initSurfDesc(_vm->_global->_videoMode, width, height, 2);

			_vm->_video->clearSurf(_vm->_draw->_spritesArray[sprIndex]);
			_vm->_draw->_destSurface = sprIndex;
			_vm->_draw->_spriteLeft = sprResId;
			_vm->_draw->_transparency = 0;
			_vm->_draw->_destSpriteX = 0;
			_vm->_draw->_destSpriteY = 0;
			_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
		}
	}
	if (extData != 0)
		delete[] extData;
	return sceneryIndex + 100;
}

void Scenery::freeStatic(int16 index) {
	int16 i;
	int16 spr;

	if (index == -1)
		_vm->_inter->evalExpr(&index);

	if (_staticPictCount[index] == -1)
		return;

	for (i = 0; i < _staticPictCount[index]; i++) {
		if (_statics[index].piecesFromExt[i] == 1)
			delete[] _statics[index].pieces[i];

		spr = _staticPictToSprite[index * 7 + i];
		_spriteRefs[spr]--;
		if (_spriteRefs[spr] == 0) {
			_vm->_video->freeSurfDesc(_vm->_draw->_spritesArray[spr]);
			_vm->_draw->_spritesArray[spr] = 0;
			_spriteResId[spr] = -1;
		}
	}

	for (i = 0; i < _statics[index].layersCount; i++)
		delete[] _statics[index].layers[i].planes;
	delete[] _statics[index].layers;
	delete[] _statics[index].pieces;
	delete[] _statics[index].piecesFromExt;

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
	for (order = 0; order < 10; order++) {
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

	if (_curStatic == -1)
		return;

	if (_curStaticLayer >= _statics[_curStatic].layersCount)
		return;

	layerPtr = &_statics[_curStatic].layers[_curStaticLayer];
	pictPtr = _statics[_curStatic].pieces;

	planeCount = layerPtr->planeCount;

	for (order = orderFrom; order < 10; order++) {
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

			if (_vm->_draw->_spriteRight <= 0 || _vm->_draw->_spriteBottom <= 0)
				continue;

			if (_vm->_draw->_destSpriteX + _vm->_draw->_spriteRight - 1 >
			    _toRedrawRight)
				_vm->_draw->_spriteRight =
				    _toRedrawRight - _vm->_draw->_destSpriteX + 1;

			if (_vm->_draw->_destSpriteY + _vm->_draw->_spriteBottom - 1 >
			    _toRedrawBottom)
				_vm->_draw->_spriteBottom =
				    _toRedrawBottom - _vm->_draw->_destSpriteY + 1;

			_vm->_draw->_sourceSurface =
			    _staticPictToSprite[_curStatic * 7 +
			    pictIndex];
			_vm->_draw->_destSurface = 21;
			_vm->_draw->_transparency = planePtr->transp ? 3 : 0;
			_vm->_draw->spriteOperation(DRAW_BLITSURF);
		}
	}
}

int16 Scenery::loadAnim(char search) {
	int16 picsCount;
	int16 resId;
	int16 i;
	int16 j;
	int16 sceneryIndex;
	int16 framesCount;
	char *extData;
	char *dataPtr;
	Animation *ptr;
	int16 offset;
	int16 pictDescId;
	int16 width;
	int16 height;
	int16 sprResId;
	int16 sprIndex;
	uint32 layerPos;

	extData = 0;
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
			if (_animPictCount[i] != 0
			    && _animResId[i] == resId) {
				_vm->_global->_inter_execPtr += 8 * _animPictCount[i];
				return i;
			}

			if (_animPictCount[i] == 0 && i < sceneryIndex)
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
	ptr->piecesFromExt = new int8[picsCount];

	for (i = 0; i < ptr->layersCount; i++) {
		offset = (int16)READ_LE_UINT16(&((int16 *)dataPtr)[i]);
		Common::MemoryReadStream layerData((byte *) (dataPtr + offset - 2), 4294967295U);

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
		for (j = 0; j < ptr->layers[i].framesCount; j++, framesCount++, layerData.seek(4, SEEK_CUR)) {
			while(layerData.readByte() == 1) {
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
			if (_spriteResId[sprIndex] == sprResId)
				break;
		}

		if (sprIndex < 20) {
			_animPictToSprite[7 * sceneryIndex + i] = sprIndex;
			_spriteRefs[sprIndex]++;
		} else {
			for (sprIndex = 19; _vm->_draw->_spritesArray[sprIndex] != 0;
			    sprIndex--);

			_animPictToSprite[7 * sceneryIndex + i] = sprIndex;
			_spriteRefs[sprIndex] = 1;
			_spriteResId[sprIndex] = sprResId;
			_vm->_draw->initBigSprite(sprIndex, width, height, 2);

			_vm->_video->clearSurf(_vm->_draw->_spritesArray[sprIndex]);
			_vm->_draw->_destSurface = sprIndex;
			_vm->_draw->_spriteLeft = sprResId;
			_vm->_draw->_transparency = 0;
			_vm->_draw->_destSpriteX = 0;
			_vm->_draw->_destSpriteY = 0;
			_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
		}
	}
	if (extData != 0)
		delete[] extData;
	return sceneryIndex + 100;
}

void Scenery::freeAnim(int16 animation) {
	int16 i;
	int16 spr;

	if (animation == -1)
		_vm->_inter->evalExpr(&animation);

	if (_animPictCount[animation] == 0)
		return;

	for (i = 0; i < _animPictCount[animation]; i++) {
		if (_animations[animation].piecesFromExt[i] == 1)
			delete[] _animations[animation].pieces[i];

		spr = _animPictToSprite[animation * 7 + i];
		_spriteRefs[spr]--;
		if (_spriteRefs[spr] == 0) {
			_vm->_video->freeSurfDesc(_vm->_draw->_spritesArray[spr]);

			_vm->_draw->_spritesArray[spr] = 0;
			_spriteResId[spr] = -1;
		}
	}

	
	for (i = 0; i < _animations[animation].layersCount; i++)
		delete[] _animations[animation].layers[i].frames;
	delete[] _animations[animation].layers;
	delete[] _animations[animation].pieces;
	delete[] _animations[animation].piecesFromExt;

	_animPictCount[animation] = 0;
}

void Scenery::interStoreParams(void) {
	AnimLayer *layerPtr;
	int16 animation;
	int16 layer;
	int16 var;

	warning("interStoreParams: Storing...");

	_vm->_inter->evalExpr(&animation);
	_vm->_inter->evalExpr(&layer);
	layerPtr = &_animations[animation].layers[layer];

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
