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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

int16 scen_spriteResId[20];
char scen_spriteRefs[20];

Scen_Static scen_statics[10];
int16 scen_staticPictCount[10];
char scen_staticFromExt[10];
int16 scen_staticResId[10];
char scen_staticPictToSprite[70];

Scen_Animation scen_animations[10];
int16 scen_animPictCount[10];
char scen_animFromExt[10];
int16 scen_animResId[10];
char scen_animPictToSprite[70];

int16 scen_curStatic;
int16 scen_curStaticLayer;

int16 scen_toRedrawLeft;
int16 scen_toRedrawRight;
int16 scen_toRedrawTop;
int16 scen_toRedrawBottom;

int16 scen_animTop;
int16 scen_animLeft;

int16 *scen_pCaptureCounter;

int16 scen_loadStatic(char search) {
	int16 tmp;
	int16 *backsPtr;
	int16 picsCount;
	int16 resId;
	int16 i;
	int16 sceneryIndex;
	char *dataPtr;
	Scen_Static *ptr;
	int16 offset;
	int16 pictDescId;
	int16 width;
	int16 height;
	int16 sprResId;
	int16 sprIndex;

	inter_evalExpr(&sceneryIndex);
	tmp = inter_load16();
	backsPtr = (int16 *)inter_execPtr;
	inter_execPtr += tmp * 2;
	picsCount = inter_load16();
	resId = inter_load16();
	if (search) {
		for (i = 0; i < 10; i++) {
			if (scen_staticPictCount[i] != -1 && scen_staticResId[i] == resId) {
				inter_execPtr += 8 * scen_staticPictCount[i];
				return i;
			}

			if (scen_staticPictCount[i] == -1 && i < sceneryIndex)
				sceneryIndex = i;
		}
	}

	scen_staticPictCount[sceneryIndex] = picsCount;
	scen_staticResId[sceneryIndex] = resId;

	if (resId >= 30000) {
		scen_staticFromExt[sceneryIndex] = 1;
		dataPtr = game_loadExtData(resId, 0, 0);
	} else {
		scen_staticFromExt[sceneryIndex] = 0;
		dataPtr = game_loadTotResource(resId);
	}

	ptr = &scen_statics[sceneryIndex];
	ptr->dataPtr = dataPtr;

	ptr->layersCount = (int16)READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	ptr->layers = (Scen_StaticLayer **)malloc(sizeof(Scen_StaticLayer *) * ptr->layersCount);
	ptr->pieces = (Scen_PieceDesc **)malloc(sizeof(Scen_PieceDesc *) * picsCount);
	ptr->piecesFromExt = (int8 *)malloc(picsCount);

	for (i = 0; i < ptr->layersCount; i++) {
		offset = (int16)READ_LE_UINT16(&((int16 *)dataPtr)[i]);
		ptr->layers[i] = (Scen_StaticLayer *)(dataPtr + offset - 2);

		ptr->layers[i]->planeCount = (int16)READ_LE_UINT16(&ptr->layers[i]->planeCount);

		for (int j = 0; j < ptr->layers[i]->planeCount; ++j) {
			ptr->layers[i]->planes[j].destX = (int16)READ_LE_UINT16(&ptr->layers[i]->planes[j].destX);
			ptr->layers[i]->planes[j].destY = (int16)READ_LE_UINT16(&ptr->layers[i]->planes[j].destY);
		}

		ptr->layers[i]->backResId = (int16)READ_LE_UINT16(backsPtr);
		backsPtr++;
	}

	for (i = 0; i < picsCount; i++) {
		pictDescId = inter_load16();
		if (pictDescId >= 30000) {
			ptr->pieces[i] =
			    (Scen_PieceDesc *) game_loadExtData(pictDescId, 0,
			    0);
			ptr->piecesFromExt[i] = 1;
		} else {
			ptr->pieces[i] =
			    (Scen_PieceDesc *)
			    game_loadTotResource(pictDescId);
			ptr->piecesFromExt[i] = 0;
		}

		width = inter_load16();
		height = inter_load16();
		sprResId = inter_load16();
		for (sprIndex = 0; sprIndex < 20; sprIndex++) {
			if (scen_spriteResId[sprIndex] == sprResId)
				break;
		}

		if (sprIndex < 20) {
			scen_staticPictToSprite[7 * sceneryIndex + i] =
			    sprIndex;
			scen_spriteRefs[sprIndex]++;
		} else {
			for (sprIndex = 19; draw_spritesArray[sprIndex] != 0;
			    sprIndex--);

			scen_staticPictToSprite[7 * sceneryIndex + i] =
			    sprIndex;
			scen_spriteRefs[sprIndex] = 1;
			scen_spriteResId[sprIndex] = sprResId;
			draw_spritesArray[sprIndex] =
			    vid_initSurfDesc(videoMode, width, height, 2);

			vid_clearSurf(draw_spritesArray[sprIndex]);
			draw_destSurface = sprIndex;
			draw_spriteLeft = sprResId;
			draw_transparency = 0;
			draw_destSpriteX = 0;
			draw_destSpriteY = 0;
			draw_spriteOperation(DRAW_LOADSPRITE);
		}
	}
	return sceneryIndex + 100;
}

void scen_freeStatic(int16 index) {
	int16 i;
	int16 spr;

	if (index == -1)
		inter_evalExpr(&index);

	if (scen_staticPictCount[index] == -1)
		return;

	for (i = 0; i < scen_staticPictCount[index]; i++) {
		if (scen_statics[index].piecesFromExt[i] == 1)
			free(scen_statics[index].pieces[i]);

		spr = scen_staticPictToSprite[index * 7 + i];
		scen_spriteRefs[spr]--;
		if (scen_spriteRefs[spr] == 0) {
			vid_freeSurfDesc(draw_spritesArray[spr]);
			draw_spritesArray[spr] = 0;
			scen_spriteResId[spr] = -1;
		}
	}

	free(scen_statics[index].layers);
	free(scen_statics[index].pieces);
	free(scen_statics[index].piecesFromExt);
	if (scen_staticFromExt[index] == 1)
		free(scen_statics[index].dataPtr);

	scen_staticFromExt[index] = 0;
	scen_staticPictCount[index] = -1;
}

void scen_renderStatic(int16 scenery, int16 layer) {
	Scen_Static *ptr;
	Scen_StaticLayer *layerPtr;
	Scen_StaticPlane *planePtr;
	int16 planeCount;
	int16 order;
	int16 plane;

	int16 pieceIndex;
	int16 pictIndex;

	int16 left;
	int16 right;
	int16 top;
	int16 bottom;

	ptr = &scen_statics[scenery];
	if (layer >= ptr->layersCount)
		return;

	layerPtr = ptr->layers[layer];

	draw_spriteLeft = layerPtr->backResId;
	if (draw_spriteLeft != -1) {
		draw_destSpriteX = 0;
		draw_destSpriteY = 0;
		draw_destSurface = 21;
		draw_transparency = 0;
		draw_spriteOperation(DRAW_LOADSPRITE);
	}

	planeCount = layerPtr->planeCount;
	for (order = 0; order < 10; order++) {
		for (plane = 0, planePtr = layerPtr->planes;
		    plane < planeCount; plane++, planePtr++) {
			if (planePtr->drawOrder != order)
				continue;

			pieceIndex = planePtr->pieceIndex;
			pictIndex = planePtr->pictIndex - 1;

			draw_destSpriteX = planePtr->destX;
			draw_destSpriteY = planePtr->destY;
			left = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].left);
			right = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].right);
			top = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].top);
			bottom = FROM_LE_16(ptr->pieces[pictIndex][pieceIndex].bottom);

			draw_sourceSurface =
			    scen_staticPictToSprite[scenery * 7 + pictIndex];
			draw_destSurface = 21;
			draw_spriteLeft = left;
			draw_spriteTop = top;
			draw_spriteRight = right - left + 1;
			draw_spriteBottom = bottom - top + 1;
			draw_transparency = planePtr->transp ? 3 : 0;
			draw_spriteOperation(DRAW_BLITSURF);
		}
	}
}

void scen_interRenderStatic(void) {
	int16 layer;
	int16 index;

	inter_evalExpr(&index);
	inter_evalExpr(&layer);
	scen_renderStatic(index, layer);
}

void scen_interLoadCurLayer(void) {
	inter_evalExpr(&scen_curStatic);
	inter_evalExpr(&scen_curStaticLayer);
}

void scen_updateStatic(int16 orderFrom) {
	Scen_StaticLayer *layerPtr;
	Scen_PieceDesc **pictPtr;
	Scen_StaticPlane *planePtr;
	int16 planeCount;
	int16 order;
	int16 plane;
	int16 pieceIndex;
	int16 pictIndex;

	int16 left;
	int16 right;
	int16 top;
	int16 bottom;

	if (scen_curStatic == -1)
		return;

	if (scen_curStaticLayer >= scen_statics[scen_curStatic].layersCount)
		return;

	layerPtr = scen_statics[scen_curStatic].layers[scen_curStaticLayer];
	pictPtr = scen_statics[scen_curStatic].pieces;

	planeCount = layerPtr->planeCount;

	for (order = orderFrom; order < 10; order++) {
		for (planePtr = layerPtr->planes, plane = 0;
		    plane < planeCount; plane++, planePtr++) {
			if (planePtr->drawOrder != order)
				continue;

			pieceIndex = planePtr->pieceIndex;
			pictIndex = planePtr->pictIndex - 1;
			draw_destSpriteX = planePtr->destX;
			draw_destSpriteY = planePtr->destY;

			left = FROM_LE_16(pictPtr[pictIndex][pieceIndex].left);
			right = FROM_LE_16(pictPtr[pictIndex][pieceIndex].right);
			top = FROM_LE_16(pictPtr[pictIndex][pieceIndex].top);
			bottom = FROM_LE_16(pictPtr[pictIndex][pieceIndex].bottom);

			if (draw_destSpriteX > scen_toRedrawRight)
				continue;

			if (draw_destSpriteY > scen_toRedrawBottom)
				continue;

			if (draw_destSpriteX < scen_toRedrawLeft) {
				left += scen_toRedrawLeft - draw_destSpriteX;
				draw_destSpriteX = scen_toRedrawLeft;
			}

			if (draw_destSpriteY < scen_toRedrawTop) {
				top += scen_toRedrawTop - draw_destSpriteY;
				draw_destSpriteY = scen_toRedrawTop;
			}

			draw_spriteLeft = left;
			draw_spriteTop = top;
			draw_spriteRight = right - left + 1;
			draw_spriteBottom = bottom - top + 1;

			if (draw_spriteRight <= 0 || draw_spriteBottom <= 0)
				continue;

			if (draw_destSpriteX + draw_spriteRight - 1 >
			    scen_toRedrawRight)
				draw_spriteRight =
				    scen_toRedrawRight - draw_destSpriteX + 1;

			if (draw_destSpriteY + draw_spriteBottom - 1 >
			    scen_toRedrawBottom)
				draw_spriteBottom =
				    scen_toRedrawBottom - draw_destSpriteY + 1;

			draw_sourceSurface =
			    scen_staticPictToSprite[scen_curStatic * 7 +
			    pictIndex];
			draw_destSurface = 21;
			draw_transparency = planePtr->transp ? 3 : 0;
			draw_spriteOperation(DRAW_BLITSURF);
		}
	}
}

int16 scen_loadAnim(char search) {
	int16 picsCount;
	int16 resId;
	int16 i;
	int16 sceneryIndex;
	char *dataPtr;
	Scen_Animation *ptr;
	int16 offset;
	int16 pictDescId;
	int16 width;
	int16 height;
	int16 sprResId;
	int16 sprIndex;

	if (cd_globFlag) {
		while (cd_getTrackPos() != -1);

		cd_globFlag = false;
	}

	inter_evalExpr(&sceneryIndex);
	picsCount = inter_load16();
	resId = inter_load16();

	if (search) {
		for (i = 0; i < 10; i++) {
			if (scen_animPictCount[i] != 0
			    && scen_animResId[i] == resId) {
				inter_execPtr += 8 * scen_animPictCount[i];
				return i;
			}

			if (scen_animPictCount[i] == 0 && i < sceneryIndex)
				sceneryIndex = i;
		}
	}

	scen_animPictCount[sceneryIndex] = picsCount;
	scen_animResId[sceneryIndex] = resId;

	if (resId >= 30000) {
		scen_animFromExt[sceneryIndex] = 1;
		dataPtr = game_loadExtData(resId, 0, 0);
	} else {
		scen_animFromExt[sceneryIndex] = 0;
		dataPtr = game_loadTotResource(resId);
	}

	ptr = &scen_animations[sceneryIndex];
	ptr->dataPtr = dataPtr;

	ptr->layersCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	ptr->layers =
	    (Scen_AnimLayer **) malloc(sizeof(Scen_AnimLayer *) *
	    ptr->layersCount);
	ptr->pieces =
	    (Scen_PieceDesc **) malloc(sizeof(Scen_PieceDesc *) *
	    picsCount);
	ptr->piecesFromExt = (int8 *) malloc(picsCount);

	for (i = 0; i < ptr->layersCount; i++) {
		offset = (int16)READ_LE_UINT16(&((int16 *)dataPtr)[i]);
		ptr->layers[i] = (Scen_AnimLayer *) (dataPtr + offset - 2);

		ptr->layers[i]->unknown0 = (int16)READ_LE_UINT16(&ptr->layers[i]->unknown0);
		ptr->layers[i]->posX = (int16)READ_LE_UINT16(&ptr->layers[i]->posX);
		ptr->layers[i]->posY = (int16)READ_LE_UINT16(&ptr->layers[i]->posY);
		ptr->layers[i]->animDeltaX = (int16)READ_LE_UINT16(&ptr->layers[i]->animDeltaX);
		ptr->layers[i]->animDeltaY = (int16)READ_LE_UINT16(&ptr->layers[i]->animDeltaY);
		ptr->layers[i]->framesCount = (int16)READ_LE_UINT16(&ptr->layers[i]->framesCount);
	}

	for (i = 0; i < picsCount; i++) {
		pictDescId = inter_load16();
		if (pictDescId >= 30000) {
			ptr->pieces[i] =
			    (Scen_PieceDesc *) game_loadExtData(pictDescId, 0,
			    0);
			ptr->piecesFromExt[i] = 1;
		} else {
			ptr->pieces[i] =
			    (Scen_PieceDesc *)
			    game_loadTotResource(pictDescId);
			ptr->piecesFromExt[i] = 0;
		}

		width = inter_load16();
		height = inter_load16();
		sprResId = inter_load16();
		for (sprIndex = 0; sprIndex < 20; sprIndex++) {
			if (scen_spriteResId[sprIndex] == sprResId)
				break;
		}

		if (sprIndex < 20) {
			scen_animPictToSprite[7 * sceneryIndex + i] = sprIndex;
			scen_spriteRefs[sprIndex]++;
		} else {
			for (sprIndex = 19; draw_spritesArray[sprIndex] != 0;
			    sprIndex--);

			scen_animPictToSprite[7 * sceneryIndex + i] = sprIndex;
			scen_spriteRefs[sprIndex] = 1;
			scen_spriteResId[sprIndex] = sprResId;
			draw_spritesArray[sprIndex] =
			    vid_initSurfDesc(videoMode, width, height, 2);

			vid_clearSurf(draw_spritesArray[sprIndex]);
			draw_destSurface = sprIndex;
			draw_spriteLeft = sprResId;
			draw_transparency = 0;
			draw_destSpriteX = 0;
			draw_destSpriteY = 0;
			draw_spriteOperation(DRAW_LOADSPRITE);
		}
	}
	return sceneryIndex + 100;
}

// flags & 1 - do capture all area animation is occupying
// flags & 4 == 0 - calculate animation final size
// flags & 2 != 0 - don't check with "toRedraw"'s
// flags & 4 != 0 - checkk view toRedraw
void scen_updateAnim(int16 layer, int16 frame, int16 animation, int16 flags,
	    int16 drawDeltaX, int16 drawDeltaY, char doDraw) {
	Scen_AnimLayer *layerPtr;
	Scen_PieceDesc **pictPtr;
	Scen_AnimFramePiece *framePtr;

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

	if (layer >= scen_animations[animation].layersCount)
		return;

	layerPtr = scen_animations[animation].layers[layer];

	if (frame >= layerPtr->framesCount)
		return;

	if (flags & 1)		// Do capture
	{
		scen_updateAnim(layer, frame, animation, 0, drawDeltaX,
		    drawDeltaY, 0);

		if (scen_toRedrawLeft == -12345)	// Some magic number?
			return;

		game_capturePush(scen_toRedrawLeft, scen_toRedrawTop,
		    scen_toRedrawRight - scen_toRedrawLeft + 1,
		    scen_toRedrawBottom - scen_toRedrawTop + 1);

		*scen_pCaptureCounter = *scen_pCaptureCounter + 1;
	}
	pictPtr = scen_animations[animation].pieces;
	framePtr = layerPtr->frames;

	for (i = 0; i < frame; i++, framePtr++) {
		while (framePtr->notFinal == 1)
			framePtr++;
	}

	if ((flags & 4) == 0) {
		scen_toRedrawLeft = -12345;
	} else {
		scen_toRedrawLeft =
		    MAX(scen_toRedrawLeft, anim_animAreaLeft);
		scen_toRedrawTop =
		    MAX(scen_toRedrawTop, anim_animAreaTop);
		scen_toRedrawRight =
		    MIN(scen_toRedrawRight,
		    (int16)(anim_animAreaLeft + anim_animAreaWidth - 1));
		scen_toRedrawBottom =
		    MIN(scen_toRedrawBottom,
		    (int16)(anim_animAreaTop + anim_animAreaHeight - 1));
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
			if (destX < anim_animAreaLeft) {
				left += anim_animAreaLeft - destX;
				destX = anim_animAreaLeft;
			}

			if (left <= right
			    && destX + right - left >=
			    anim_animAreaLeft + anim_animAreaWidth)
				right -=
				    (destX + right - left) -
				    (anim_animAreaLeft + anim_animAreaWidth) +
				    1;

			if (destY < anim_animAreaTop) {
				top += anim_animAreaTop - destY;
				destY = anim_animAreaTop;
			}

			if (top <= bottom
			    && destY + bottom - top >=
			    anim_animAreaTop + anim_animAreaHeight)
				bottom -=
				    (destY + bottom - top) -
				    (anim_animAreaTop + anim_animAreaHeight) +
				    1;

		} else if (flags & 4) {
			if (destX < scen_toRedrawLeft) {
				left += scen_toRedrawLeft - destX;
				destX = scen_toRedrawLeft;
			}

			if (left <= right
			    && destX + right - left > scen_toRedrawRight)
				right -=
				    destX + right - left - scen_toRedrawRight;

			if (destY < scen_toRedrawTop) {
				top += scen_toRedrawTop - destY;
				destY = scen_toRedrawTop;
			}

			if (top <= bottom
			    && destY + bottom - top > scen_toRedrawBottom)
				bottom -=
				    destY + bottom - top - scen_toRedrawBottom;
		}

		if (left > right || top > bottom)
			continue;

		if (doDraw) {
			draw_sourceSurface =
			    scen_animPictToSprite[animation * 7 + pictIndex];
			draw_destSurface = 21;

			draw_spriteLeft = left;
			draw_spriteTop = top;
			draw_spriteRight = right - left + 1;
			draw_spriteBottom = bottom - top + 1;
			draw_destSpriteX = destX;
			draw_destSpriteY = destY;
			draw_transparency = transp;
			draw_spriteOperation(DRAW_BLITSURF);
		}

		if ((flags & 4) == 0) {
			if (scen_toRedrawLeft == -12345) {
				scen_toRedrawLeft = destX;
				scen_animLeft = destX;
				scen_toRedrawTop = destY;
				scen_animTop = destY;
				scen_toRedrawRight = destX + right - left;
				scen_toRedrawBottom = destY + bottom - top;
			} else {
				scen_toRedrawLeft =
				    MIN(scen_toRedrawLeft, destX);
				scen_toRedrawTop =
				    MIN(scen_toRedrawTop, destY);
				scen_toRedrawRight =
				    MAX(scen_toRedrawRight,
				    (int16)(destX + right - left));
				scen_toRedrawBottom =
				    MAX(scen_toRedrawBottom,
				    (int16)(destY + bottom - top));
			}
		}
	} while (framePtr->notFinal == 1);
}

void scen_freeAnim(int16 animation) {
	int16 i;
	int16 spr;

	if (animation == -1)
		inter_evalExpr(&animation);

	if (scen_animPictCount[animation] == 0)
		return;

	for (i = 0; i < scen_animPictCount[animation]; i++) {
		if (scen_animations[animation].piecesFromExt[i] == 1)
			free(scen_animations[animation].pieces[i]);

		spr = scen_animPictToSprite[animation * 7 + i];
		scen_spriteRefs[spr]--;
		if (scen_spriteRefs[spr] == 0) {
			vid_freeSurfDesc(draw_spritesArray[spr]);

			draw_spritesArray[spr] = 0;
			scen_spriteResId[spr] = -1;
		}
	}

	free(scen_animations[animation].layers);
	free(scen_animations[animation].pieces);
	free(scen_animations[animation].piecesFromExt);
	if (scen_animFromExt[animation] == 1)
		free(scen_animations[animation].dataPtr);

	scen_animFromExt[animation] = 0;
	scen_animPictCount[animation] = 0;
}

void scen_interUpdateAnim(void) {
	int16 deltaX;
	int16 deltaY;
	int16 flags;
	int16 frame;
	int16 layer;
	int16 animation;

	inter_evalExpr(&deltaX);
	inter_evalExpr(&deltaY);
	inter_evalExpr(&animation);
	inter_evalExpr(&layer);
	inter_evalExpr(&frame);
	flags = inter_load16();
	scen_updateAnim(layer, frame, animation, flags, deltaX, deltaY, 1);
}

void scen_interStoreParams(void) {
	Scen_AnimLayer *layerPtr;
	int16 animation;
	int16 layer;
	int16 var;

	warning("scen_interStoreParams: Storing...");

	inter_evalExpr(&animation);
	inter_evalExpr(&layer);
	layerPtr = scen_animations[animation].layers[layer];

	var = parse_parseVarIndex();
	WRITE_VAR_OFFSET(var, layerPtr->animDeltaX);

	var = parse_parseVarIndex();
	WRITE_VAR_OFFSET(var, layerPtr->animDeltaY);

	var = parse_parseVarIndex();
	WRITE_VAR_OFFSET(var, layerPtr->unknown0);

	var = parse_parseVarIndex();
	WRITE_VAR_OFFSET(var, layerPtr->framesCount);
}

}				// End of namespace Gob
