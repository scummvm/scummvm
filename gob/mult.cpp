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
#include "gob/video.h"
#include "gob/anim.h"
#include "gob/draw.h"
#include "gob/scenery.h"
#include "gob/mult.h"
#include "gob/util.h"
#include "gob/inter.h"
#include "gob/parse.h"
#include "gob/global.h"
#include "gob/sound.h"
#include "gob/palanim.h"
#include "gob/game.h"

namespace Gob {

Mult::Mult(GobEngine *vm) : _vm(vm) {
	objects = 0;
	renderData = 0;
	objCount = 0;
	underAnimSurf = 0;
	multData = 0;
	frame = 0;
	doPalSubst = 0;
	counter = 0;
	frameRate = 0;

	animArrayX = 0;
	animArrayY = 0;
	animArrayData = 0;

	index = 0;

	staticKeysCount = 0;
	staticKeys = 0;
	for (int i = 0; i < 10; i++)
		staticIndices[i] = 0;

	for (int i = 0; i < 4; i++) {
		animKeys[i] = 0;
		animKeysCount[i] = 0;
	}
	animLayer = 0;
	for (int i = 0; i < 10; i++)
		animIndices[i] = 0;

	textKeysCount = 0;
	textKeys = 0;

	frameStart = 0;

	palKeyIndex = 0;
	palKeysCount = 0;
	palKeys = 0;
	oldPalette = 0;
	palAnimKey = 0;
	for (int i = 0; i < 256; i++) {
		palAnimPalette[i].red = 0;
		palAnimPalette[i].green = 0;
		palAnimPalette[i].blue = 0;
	}
	for (int i = 0; i < 4; i++) {
		palAnimIndices[i] = 0;
		palAnimRed[i] = 0;
		palAnimGreen[i] = 0;
		palAnimBlue[i] = 0;
	}

	palFadeKeys = 0;
	palFadeKeysCount = 0;
	palFadingRed = 0;
	palFadingGreen = 0;
	palFadingBlue = 0;

	animDataAllocated = 0;

	dataPtr = 0;
	for (int i = 0; i < 10; i++) {
		staticLoaded[i] = 0;
		animLoaded[i] = 0;
	}
	sndSlotsCount = 0;

	sndKeysCount = 0;
	sndKeys = 0;

	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 16; j++) {
			fadePal[i][j].red = 0;
			fadePal[i][j].green = 0;
			fadePal[i][j].blue = 0;
		}
}

void Mult::animate(void) {
	int16 minOrder;
	int16 maxOrder;
	int16 *pCurLefts;
	int16 *pCurRights;
	int16 *pCurTops;
	int16 *pCurBottoms;
	int16 *pDirtyLefts;
	int16 *pDirtyRights;
	int16 *pDirtyTops;
	int16 *pDirtyBottoms;
	int16 *pNeedRedraw;
	Mult_AnimData *pAnimData;
	int16 i, j;
	int16 order;

	if (renderData == 0)
		return;

	pDirtyLefts = renderData;
	pDirtyRights = pDirtyLefts + objCount;
	pDirtyTops = pDirtyRights + objCount;
	pDirtyBottoms = pDirtyTops + objCount;
	pNeedRedraw = pDirtyBottoms + objCount;
	pCurLefts = pNeedRedraw + objCount;
	pCurRights = pCurLefts + objCount;
	pCurTops = pCurRights + objCount;
	pCurBottoms = pCurTops + objCount;
	minOrder = 100;
	maxOrder = 0;

	//Find dirty areas
	for (i = 0; i < objCount; i++) {
		pNeedRedraw[i] = 0;
		pDirtyTops[i] = 1000;
		pDirtyLefts[i] = 1000;
		pDirtyBottoms[i] = 1000;
		pDirtyRights[i] = 1000;
		pAnimData = objects[i].pAnimData;

		if (pAnimData->isStatic == 0 && pAnimData->isPaused == 0 &&
		    objects[i].tick == pAnimData->maxTick) {
			if (pAnimData->order < minOrder)
				minOrder = pAnimData->order;

			if (pAnimData->order > maxOrder)
				maxOrder = pAnimData->order;

			pNeedRedraw[i] = 1;
			_vm->_scenery->updateAnim(pAnimData->layer, pAnimData->frame,
			    pAnimData->animation, 0,
			    *(objects[i].pPosX), *(objects[i].pPosY),
			    0);

			if (objects[i].lastLeft != -1) {
				pDirtyLefts[i] =
				    MIN(objects[i].lastLeft,
				    _vm->_scenery->toRedrawLeft);
				pDirtyTops[i] =
				    MIN(objects[i].lastTop,
				    _vm->_scenery->toRedrawTop);
				pDirtyRights[i] =
				    MAX(objects[i].lastRight,
				    _vm->_scenery->toRedrawRight);
				pDirtyBottoms[i] =
				    MAX(objects[i].lastBottom,
				    _vm->_scenery->toRedrawBottom);
			} else {
				pDirtyLefts[i] = _vm->_scenery->toRedrawLeft;
				pDirtyTops[i] = _vm->_scenery->toRedrawTop;
				pDirtyRights[i] = _vm->_scenery->toRedrawRight;
				pDirtyBottoms[i] = _vm->_scenery->toRedrawBottom;
			}
			pCurLefts[i] = _vm->_scenery->toRedrawLeft;
			pCurRights[i] = _vm->_scenery->toRedrawRight;
			pCurTops[i] = _vm->_scenery->toRedrawTop;
			pCurBottoms[i] = _vm->_scenery->toRedrawBottom;
		} else {
			if (objects[i].lastLeft != -1) {
				if (pAnimData->order < minOrder)
					minOrder = pAnimData->order;

				if (pAnimData->order > maxOrder)
					maxOrder = pAnimData->order;

				if (pAnimData->isStatic)
					*pNeedRedraw = 1;

				pCurLefts[i] = objects[i].lastLeft;
				pDirtyLefts[i] = objects[i].lastLeft;

				pCurTops[i] = objects[i].lastTop;
				pDirtyTops[i] = objects[i].lastTop;

				pCurRights[i] = objects[i].lastRight;
				pDirtyRights[i] = objects[i].lastRight;

				pCurBottoms[i] = objects[i].lastBottom;
				pDirtyBottoms[i] = objects[i].lastBottom;
			}
		}
	}

	// Find intersections
	for (i = 0; i < objCount; i++) {
		pAnimData = objects[i].pAnimData;
		pAnimData->intersected = 200;

		if (pAnimData->isStatic)
			continue;

		for (j = 0; j < objCount; j++) {
			if (i == j)
				continue;

			if (objects[j].pAnimData->isStatic)
				continue;

			if (pCurRights[i] < pCurLefts[j])
				continue;

			if (pCurRights[j] < pCurLefts[i])
				continue;

			if (pCurBottoms[i] < pCurTops[j])
				continue;

			if (pCurBottoms[j] < pCurTops[i])
				continue;

			pAnimData->intersected = j;
			break;
		}
	}

	// Restore dirty areas
	for (i = 0; i < objCount; i++) {

		if (pNeedRedraw[i] == 0 || objects[i].lastLeft == -1)
			continue;

		_vm->_draw->_sourceSurface = 22;
		_vm->_draw->_destSurface = 21;
		_vm->_draw->_spriteLeft = pDirtyLefts[i] - _vm->_anim->_areaLeft;
		_vm->_draw->_spriteTop = pDirtyTops[i] - _vm->_anim->_areaTop;
		_vm->_draw->_spriteRight = pDirtyRights[i] - pDirtyLefts[i] + 1;
		_vm->_draw->_spriteBottom = pDirtyBottoms[i] - pDirtyTops[i] + 1;
		_vm->_draw->_destSpriteX = pDirtyLefts[i];
		_vm->_draw->_destSpriteY = pDirtyTops[i];
		_vm->_draw->_transparency = 0;
		_vm->_draw->spriteOperation(DRAW_BLITSURF);
		objects[i].lastLeft = -1;
	}

	// Update view
	for (order = minOrder; order <= maxOrder; order++) {
		for (i = 0; i < objCount; i++) {
			pAnimData = objects[i].pAnimData;
			if (pAnimData->order != order)
				continue;

			if (pNeedRedraw[i]) {
				if (pAnimData->isStatic == 0) {

					_vm->_scenery->updateAnim(pAnimData->layer,
					    pAnimData->frame,
					    pAnimData->animation, 2,
					    *(objects[i].pPosX),
					    *(objects[i].pPosY), 1);

					if (_vm->_scenery->toRedrawLeft != -12345) {
						objects[i].lastLeft =
						    _vm->_scenery->toRedrawLeft;
						objects[i].lastTop =
						    _vm->_scenery->toRedrawTop;
						objects[i].lastRight =
						    _vm->_scenery->toRedrawRight;
						objects[i].lastBottom =
						    _vm->_scenery->toRedrawBottom;
					} else {
						objects[i].lastLeft = -1;
					}
				}
				_vm->_scenery->updateStatic(order + 1);
			} else if (pAnimData->isStatic == 0) {
				for (j = 0; j < objCount; j++) {
					if (pNeedRedraw[j] == 0)
						continue;

					if (pDirtyRights[i] < pDirtyLefts[j])
						continue;

					if (pDirtyRights[j] < pDirtyLefts[i])
						continue;

					if (pDirtyBottoms[i] < pDirtyTops[j])
						continue;

					if (pDirtyBottoms[j] < pDirtyTops[i])
						continue;

					_vm->_scenery->toRedrawLeft = pDirtyLefts[j];
					_vm->_scenery->toRedrawRight = pDirtyRights[j];
					_vm->_scenery->toRedrawTop = pDirtyTops[j];
					_vm->_scenery->toRedrawBottom = pDirtyBottoms[j];

					_vm->_scenery->updateAnim(pAnimData->layer,
					    pAnimData->frame,
					    pAnimData->animation, 4,
					    *(objects[i].pPosX),
					    *(objects[i].pPosY), 1);

					_vm->_scenery->updateStatic(order + 1);
				}
			}
		}
	}

	// Advance animations
	for (i = 0; i < objCount; i++) {
		pAnimData = objects[i].pAnimData;
		if (pAnimData->isStatic || pAnimData->isPaused)
			continue;

		if (objects[i].tick == pAnimData->maxTick) {
			objects[i].tick = 0;
			if (pAnimData->animType == 4) {
				pAnimData->isPaused = 1;
				pAnimData->frame = 0;
			} else {
				pAnimData->frame++;
				if (pAnimData->frame >=
				    _vm->_scenery->animations[(int)pAnimData->animation].layers[pAnimData->layer]->framesCount) {
					switch (pAnimData->animType) {
					case 0:
						pAnimData->frame = 0;
						break;

					case 1:
						pAnimData->frame = 0;

						*(objects[i].pPosX) =
						    *(objects[i].pPosX) +
						    _vm->_scenery->animations[(int)pAnimData->animation].layers[pAnimData->layer]->animDeltaX;

						*(objects[i].pPosY) =
						    *(objects[i].pPosY) +
						    _vm->_scenery->animations[(int)pAnimData->animation].layers[pAnimData->layer]->animDeltaY;
						break;

					case 2:
						pAnimData->frame = 0;
						pAnimData->animation =
						    pAnimData->newAnimation;
						pAnimData->layer =
						    pAnimData->newLayer;
						break;

					case 3:
						pAnimData->animType = 4;
						pAnimData->frame = 0;
						break;

					case 5:
						pAnimData->isStatic = 1;
						pAnimData->frame = 0;
						break;

					case 6:
						pAnimData->frame--;
						pAnimData->isPaused = 1;
						break;
					}
					pAnimData->newCycle = 1;
				} else {
					pAnimData->newCycle = 0;
				}
			}
		} else {
			objects[i].tick++;
		}
	}
}

void Mult::interGetObjAnimSize(void) {
	Mult_AnimData *pAnimData;
	int16 objIndex;

	_vm->_inter->evalExpr(&objIndex);
	pAnimData = objects[objIndex].pAnimData;
	if (pAnimData->isStatic == 0) {
		_vm->_scenery->updateAnim(pAnimData->layer, pAnimData->frame,
		    pAnimData->animation, 0, *(objects[objIndex].pPosX),
		    *(objects[objIndex].pPosY), 0);
	}
	WRITE_VAR_OFFSET(_vm->_parse->parseVarIndex(), _vm->_scenery->toRedrawLeft);
	WRITE_VAR_OFFSET(_vm->_parse->parseVarIndex(), _vm->_scenery->toRedrawTop);
	WRITE_VAR_OFFSET(_vm->_parse->parseVarIndex(), _vm->_scenery->toRedrawRight);
	WRITE_VAR_OFFSET(_vm->_parse->parseVarIndex(), _vm->_scenery->toRedrawBottom);
}

void Mult::interInitMult(void) {
	int16 oldAnimHeight;
	int16 oldAnimWidth;
	int16 oldObjCount;
	int16 i;
	int16 posXVar;
	int16 posYVar;
	int16 animDataVar;

	oldAnimWidth = _vm->_anim->_areaWidth;
	oldAnimHeight = _vm->_anim->_areaHeight;
	oldObjCount = objCount;

	_vm->_anim->_areaLeft = _vm->_inter->load16();
	_vm->_anim->_areaTop = _vm->_inter->load16();
	_vm->_anim->_areaWidth = _vm->_inter->load16();
	_vm->_anim->_areaHeight = _vm->_inter->load16();
	objCount = _vm->_inter->load16();
	posXVar = _vm->_parse->parseVarIndex();
	posYVar = _vm->_parse->parseVarIndex();
	animDataVar = _vm->_parse->parseVarIndex();

	if (objects == 0) {
		renderData = (int16 *)malloc(sizeof(int16) * objCount * 9);
		objects = (Mult_Object *)malloc(sizeof(Mult_Object) * objCount);

		for (i = 0; i < objCount; i++) {
			objects[i].pPosX = (int32 *)(_vm->_global->_inter_variables + i * 4 + (posXVar / 4) * 4);
			objects[i].pPosY = (int32 *)(_vm->_global->_inter_variables + i * 4 + (posYVar / 4) * 4);
			objects[i].pAnimData =
			    (Mult_AnimData *) (_vm->_global->_inter_variables + animDataVar +
			    i * 4 * _vm->_global->_inter_animDataSize);

			objects[i].pAnimData->isStatic = 1;
			objects[i].tick = 0;
			objects[i].lastLeft = -1;
			objects[i].lastRight = -1;
			objects[i].lastTop = -1;
			objects[i].lastBottom = -1;
		}
	} else if (oldObjCount != objCount) {
		error("interInitMult: Object count changed, but storage didn't (old count = %d, new count = %d)",
		    oldObjCount, objCount);
	}

	if (_vm->_anim->_animSurf != 0 &&
	    (oldAnimWidth != _vm->_anim->_areaWidth
		|| oldAnimHeight != _vm->_anim->_areaHeight)) {
		_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);
		_vm->_anim->_animSurf = 0;
	}

	if (_vm->_anim->_animSurf == 0) {
		_vm->_anim->_animSurf = _vm->_video->initSurfDesc(_vm->_global->_videoMode,
		    _vm->_anim->_areaWidth, _vm->_anim->_areaHeight, 0);

		_vm->_draw->_spritesArray[22] = _vm->_anim->_animSurf;
	}

	_vm->_video->drawSprite(_vm->_draw->_backSurface, _vm->_anim->_animSurf,
	    _vm->_anim->_areaLeft, _vm->_anim->_areaTop,
	    _vm->_anim->_areaLeft + _vm->_anim->_areaWidth - 1,
	    _vm->_anim->_areaTop + _vm->_anim->_areaHeight - 1, 0, 0, 0);

	debug(4, "interInitMult: x = %d, y = %d, w = %d, h = %d",
		  _vm->_anim->_areaLeft, _vm->_anim->_areaTop, _vm->_anim->_areaWidth, _vm->_anim->_areaHeight);
	debug(4, "    objCount = %d, animation data size = %d", objCount, _vm->_global->_inter_animDataSize);
}

void Mult::freeMult(void) {
	if (_vm->_anim->_animSurf != 0)
		_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);

	free(objects);
	free(renderData);

	objects = 0;
	renderData = 0;
	_vm->_anim->_animSurf = 0;
}

void Mult::interLoadMult(void) {
	int16 val;
	int16 objIndex;
	int16 i;
	char *lmultData;

	debug(4, "interLoadMult: Loading...");

	_vm->_inter->evalExpr(&objIndex);
	_vm->_inter->evalExpr(&val);
	*objects[objIndex].pPosX = val;
	_vm->_inter->evalExpr(&val);
	*objects[objIndex].pPosY = val;

	lmultData = (char *)objects[objIndex].pAnimData;
	for (i = 0; i < 11; i++) {
		if ((char)READ_LE_UINT16(_vm->_global->_inter_execPtr) == (char)99) {
			_vm->_inter->evalExpr(&val);
			lmultData[i] = val;
		} else {
			_vm->_global->_inter_execPtr++;
		}
	}
}

void Mult::freeAll(void) {
	int16 i;

	freeMult();
	for (i = 0; i < 10; i++)
		_vm->_scenery->freeAnim(i);

	for (i = 0; i < 10; i++)
		_vm->_scenery->freeStatic(i);
}

void Mult::initAll(void) {
	int16 i;

	objects = 0;
	_vm->_anim->_animSurf = 0;
	renderData = 0;

	for (i = 0; i < 10; i++)
		_vm->_scenery->animPictCount[i] = 0;

	for (i = 0; i < 20; i++) {
		_vm->_scenery->spriteRefs[i] = 0;
		_vm->_scenery->spriteResId[i] = -1;
	}

	for (i = 0; i < 10; i++)
		_vm->_scenery->staticPictCount[i] = -1;

	_vm->_scenery->curStaticLayer = -1;
	_vm->_scenery->curStatic = -1;
}

void Mult::playSound(Snd::SoundDesc * soundDesc, int16 repCount, int16 freq,
	    int16 channel) {
	_vm->_snd->playSample(soundDesc, repCount, freq);
}

char Mult::drawStatics(char stop) {
	if (staticKeys[staticKeysCount - 1].frame > frame)
		stop = 0;

	for (counter = 0; counter < staticKeysCount;
	    counter++) {
		if (staticKeys[counter].frame != frame
		    || staticKeys[counter].layer == -1)
			continue;

		for (_vm->_scenery->curStatic = 0, _vm->_scenery->curStaticLayer = staticKeys[counter].layer;
			 _vm->_scenery->curStaticLayer >= _vm->_scenery->statics[staticIndices[_vm->_scenery->curStatic]].layersCount;
			 _vm->_scenery->curStatic++) {
			_vm->_scenery->curStaticLayer -=
			    _vm->_scenery->statics[staticIndices[_vm->_scenery->curStatic]].layersCount;
		}

		_vm->_scenery->curStatic = staticIndices[_vm->_scenery->curStatic];
		_vm->_scenery->renderStatic(_vm->_scenery->curStatic, _vm->_scenery->curStaticLayer);
		_vm->_video->drawSprite(_vm->_draw->_backSurface, _vm->_anim->_animSurf,
		    0, 0, 319, 199, 0, 0, 0);
	}
	return stop;
}

void Mult::drawAnims(void) {
	Mult_AnimKey *key;
	Mult_Object *animObj;
	int16 i;
	int16 count;

	for (index = 0; index < 4; index++) {
		for (counter = 0; counter < animKeysCount[index]; counter++) {
			key = &animKeys[index][counter];
			animObj = &objects[index];
			if (key->frame != frame)
				continue;

			if (key->layer != -1) {
				(*animObj->pPosX) = key->posX;
				(*animObj->pPosY) = key->posY;

				animObj->pAnimData->frame = 0;
				animObj->pAnimData->order = key->order;
				animObj->pAnimData->animType = 1;

				animObj->pAnimData->isPaused = 0;
				animObj->pAnimData->isStatic = 0;
				animObj->pAnimData->maxTick = 0;
				animObj->tick = 0;
				animObj->pAnimData->layer = key->layer;

				count = _vm->_scenery->animations[animIndices[0]].layersCount;
				i = 0;
				while (animObj->pAnimData->layer >= count) {
					animObj->pAnimData->layer -= count;
					i++;

					count = _vm->_scenery->animations[animIndices[i]].layersCount;
				}
				animObj->pAnimData->animation = animIndices[i];
			} else {
				animObj->pAnimData->isStatic = 1;
			}
		}
	}
}

void Mult::drawText(char *pStop, char *pStopNoClear) {
	char *savedIP;

	int16 cmd;
	for (index = 0; index < textKeysCount; index++) {
		if (textKeys[index].frame != frame)
			continue;

		cmd = textKeys[index].cmd;
		if (cmd == 0) {
			*pStop = 0;
		} else if (cmd == 1) {
			*pStopNoClear = 1;
			frameStart = 0;
		} else if (cmd == 3) {
			*pStop = 0;
			savedIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr = (char *)(&textKeys[index].index);
			_vm->_global->_inter_execPtr = savedIP;
		}
	}
}

char Mult::prepPalAnim(char stop) {
	palKeyIndex = -1;
	do {
		palKeyIndex++;
		if (palKeyIndex >= palKeysCount)
			return stop;
	} while (palKeys[palKeyIndex].frame != frame);

	if (palKeys[palKeyIndex].cmd == -1) {
		stop = 0;
		doPalSubst = 0;
		_vm->_global->_pPaletteDesc->vgaPal = oldPalette;

		memcpy((char *)palAnimPalette, (char *)_vm->_global->_pPaletteDesc->vgaPal, 768);

		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	} else {
		stop = 0;
		doPalSubst = 1;
		palAnimKey = palKeyIndex;

		palAnimIndices[0] = 0;
		palAnimIndices[1] = 0;
		palAnimIndices[2] = 0;
		palAnimIndices[3] = 0;

		_vm->_global->_pPaletteDesc->vgaPal = palAnimPalette;
	}
	return stop;
}

void Mult::doPalAnim(void) {
	int16 off;
	int16 off2;
	Video::Color *palPtr;
	Mult_PalKey *palKey;

	if (doPalSubst == 0)
		return;

	for (index = 0; index < 4; index++) {
		palKey = &palKeys[palAnimKey];

		if ((frame % palKey->rates[index]) != 0)
			continue;

		palAnimRed[index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][index] - 1].red;
		palAnimGreen[index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][index] - 1].green;
		palAnimBlue[index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][index] - 1].blue;

		while (1) {
			off = palKey->subst[(palAnimIndices[index] + 1) % 16][index];
			if (off == 0) {
				off = palKey->subst[palAnimIndices[index]][index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off].red = palAnimRed[index];
				_vm->_global->_pPaletteDesc->vgaPal[off].green = palAnimGreen[index];
				_vm->_global->_pPaletteDesc->vgaPal[off].blue = palAnimBlue[index];
			} else {
				off = palKey->subst[(palAnimIndices[index] + 1) % 16][index] - 1;
				off2 = palKey->subst[palAnimIndices[index]][index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off2].red = _vm->_global->_pPaletteDesc->vgaPal[off].red;
				_vm->_global->_pPaletteDesc->vgaPal[off2].green = _vm->_global->_pPaletteDesc->vgaPal[off].green;
				_vm->_global->_pPaletteDesc->vgaPal[off2].blue = _vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}

			palAnimIndices[index] = (palAnimIndices[index] + 1) % 16;

			off = palKey->subst[palAnimIndices[index]][index];

			if (off == 0) {
				palAnimIndices[index] = 0;
				off = palKey->subst[0][index] - 1;

				palAnimRed[index] = _vm->_global->_pPaletteDesc->vgaPal[off].red;
				palAnimGreen[index] = _vm->_global->_pPaletteDesc->vgaPal[off].green;
				palAnimBlue[index] = _vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}
			if (palAnimIndices[index] == 0)
				break;
		}
	}

	if (_vm->_global->_colorCount == 256) {
		_vm->_video->waitRetrace(_vm->_global->_videoMode);

		palPtr = _vm->_global->_pPaletteDesc->vgaPal;
		for (counter = 0; counter < 16; counter++) {
			_vm->_video->setPalElem(counter, palPtr->red, palPtr->green, palPtr->blue, 0, 0x13);
			palPtr++;
		}

		palPtr = _vm->_global->_pPaletteDesc->vgaPal;
		for (counter = 0; counter < 16; counter++) {
			_vm->_global->_redPalette[counter] = palPtr->red;
			_vm->_global->_greenPalette[counter] = palPtr->green;
			_vm->_global->_bluePalette[counter] = palPtr->blue;
			palPtr++;
		}
	} else {
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	}
}

char Mult::doFadeAnim(char stop) {
	Mult_PalFadeKey *fadeKey;

	for (index = 0; index < palFadeKeysCount; index++) {
		fadeKey = &palFadeKeys[index];

		if (fadeKey->frame != frame)
			continue;

		stop = 0;
		if ((fadeKey->flag & 1) == 0) {
			if (fadeKey->fade == 0) {
				_vm->_global->_pPaletteDesc->vgaPal = fadePal[fadeKey->palIndex];
				_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			} else {
				_vm->_global->_pPaletteDesc->vgaPal = fadePal[fadeKey->palIndex];
				_vm->_palanim->fade(_vm->_global->_pPaletteDesc, fadeKey->fade, 0);
			}
		} else {
			_vm->_global->_pPaletteDesc->vgaPal = fadePal[fadeKey->palIndex];
			_vm->_palanim->fade(_vm->_global->_pPaletteDesc, fadeKey->fade, -1);

			palFadingRed = (fadeKey->flag >> 1) & 1;
			palFadingGreen = (fadeKey->flag >> 2) & 1;
			palFadingBlue = (fadeKey->flag >> 3) & 1;
		}
	}

	if (palFadingRed) {
		palFadingRed = !_vm->_palanim->fadeStep(1);
		stop = 0;
	}
	if (palFadingGreen) {
		palFadingGreen = !_vm->_palanim->fadeStep(2);
		stop = 0;
	}
	if (palFadingBlue) {
		palFadingBlue = !_vm->_palanim->fadeStep(3);
		stop = 0;
	}
	return stop;
}

char Mult::doSoundAnim(char stop) {
	Mult_SndKey *sndKey;
	for (index = 0; index < sndKeysCount; index++) {
		sndKey = &sndKeys[index];
		if (sndKey->frame != frame)
			continue;

		if (sndKey->cmd != -1) {
			if (sndKey->cmd == 1) {
				_vm->_snd->stopSound(0);
				stop = 0;
				playSound(_vm->_game->_soundSamples[sndKey->soundIndex], sndKey->repCount,
				    sndKey->freq, sndKey->channel);

			} else if (sndKey->cmd == 4) {
				_vm->_snd->stopSound(0);
				stop = 0;
				playSound(_vm->_game->_soundSamples[sndKey->soundIndex], sndKey->repCount,
				    sndKey->freq, sndKey->channel);
			}
		} else {
			if (_vm->_snd->_playingSound)
				_vm->_snd->stopSound(sndKey->channel);
		}
	}
	return stop;
}

void Mult::playMult(int16 startFrame, int16 endFrame, char checkEscape,
	    char handleMouse) {
	char stopNoClear;
	char stop;
	Mult_Object *multObj;
	Mult_AnimData *animData;

	if (multData == 0)
		return;

	stopNoClear = 0;
	frame = startFrame;
	if (endFrame == -1)
		endFrame = 32767;

	if (frame == -1) {
		doPalSubst = 0;
		palFadingRed = 0;
		palFadingGreen = 0;
		palFadingBlue = 0;

		oldPalette = _vm->_global->_pPaletteDesc->vgaPal;
		memcpy((char *)palAnimPalette, (char *)_vm->_global->_pPaletteDesc->vgaPal, 768);

		if (_vm->_anim->_animSurf == 0) {
			_vm->_util->setFrameRate(frameRate);
			_vm->_anim->_areaTop = 0;
			_vm->_anim->_areaLeft = 0;
			_vm->_anim->_areaWidth = 320;
			_vm->_anim->_areaHeight = 200;
			objCount = 4;

			objects = (Mult_Object *)malloc(sizeof(Mult_Object) * objCount);
			renderData = (int16 *)malloc(sizeof(int16) * 9 * objCount);

			animArrayX = (int32 *)malloc(sizeof(int32) * objCount);
			animArrayY = (int32 *)malloc(sizeof(int32) * objCount);

			animArrayData = (Mult_AnimData *)malloc(sizeof(Mult_AnimData) * objCount);

			for (counter = 0; counter < objCount; counter++) {
				multObj = &objects[counter];

				multObj->pPosX = (int32 *)&animArrayX[counter];
				multObj->pPosY = (int32 *)&animArrayY[counter];

				multObj->pAnimData = &animArrayData[counter];

				animData = multObj->pAnimData;
				animData->isStatic = 1;

				multObj->tick = 0;
				multObj->lastLeft = -1;
				multObj->lastTop = -1;
				multObj->lastRight = -1;
				multObj->lastBottom = -1;
			}

			_vm->_anim->_animSurf =
			    _vm->_video->initSurfDesc(_vm->_global->_videoMode, 320, 200, 0);
			_vm->_draw->_spritesArray[22] = _vm->_anim->_animSurf;

			_vm->_video->drawSprite(_vm->_draw->_backSurface, _vm->_anim->_animSurf,
			    0, 0, 319, 199, 0, 0, 0);

			animDataAllocated = 1;
		} else
			animDataAllocated = 0;
		frame = 0;
	}

	do {
		stop = 1;

		if (VAR(58) == 0) {
			stop = drawStatics(stop);
			drawAnims();
		}

		animate();
		if (handleMouse) {
			_vm->_draw->animateCursor(-1);
		} else {
			_vm->_draw->blitInvalidated();
		}

		if (VAR(58) == 0) {
			drawText(&stop, &stopNoClear);
		}

		stop = prepPalAnim(stop);
		doPalAnim();

		stop = doFadeAnim(stop);
		stop = doSoundAnim(stop);

		if (frame >= endFrame)
			stopNoClear = 1;

		if (_vm->_snd->_playingSound)
			stop = 0;

		_vm->_util->processInput();
		if (checkEscape && _vm->_util->checkKey() == 0x11b)	// Esc
			stop = 1;

		frame++;
		_vm->_util->waitEndFrame();
	} while (stop == 0 && stopNoClear == 0);

	if (stopNoClear == 0) {
		if (animDataAllocated) {
			free(objects);
			objects = 0;

			free(renderData);
			renderData = 0;

			free(animArrayX);
			animArrayX = 0;

			free(animArrayY);
			animArrayY = 0;

			free(animArrayData);
			animArrayData = 0;

			if (_vm->_anim->_animSurf)
				_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);
			_vm->_anim->_animSurf = 0;

			animDataAllocated = 0;
		}

		if (_vm->_snd->_playingSound != 0)
			_vm->_snd->stopSound(10);

		WRITE_VAR(57, (uint32)-1);
	} else {
		WRITE_VAR(57, frame - 1 - frameStart);
	}
}

void Mult::zeroMultData(void) {
	multData = 0;
}

void Mult::loadMult(int16 resId) {
	char animCount;
	char staticCount;
	int16 palIndex;
	int16 i, j;

	sndSlotsCount = 0;
	frameStart = 0;
	multData = _vm->_game->loadExtData(resId, 0, 0);
	dataPtr = multData;

	staticCount = dataPtr[0];
	animCount = dataPtr[1];
	dataPtr += 2;
	staticCount++;
	animCount++;

	for (i = 0; i < staticCount; i++, dataPtr += 14) {
		staticIndices[i] = _vm->_scenery->loadStatic(1);

		if (staticIndices[i] >= 100) {
			staticIndices[i] -= 100;
			staticLoaded[i] = 1;
		} else {
			staticLoaded[i] = 0;
		}
	}

	for (i = 0; i < animCount; i++, dataPtr += 14) {
		animIndices[i] = _vm->_scenery->loadAnim(1);

		if (animIndices[i] >= 100) {
			animIndices[i] -= 100;
			animLoaded[i] = 1;
		} else {
			animLoaded[i] = 0;
		}
	}

	frameRate = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	staticKeysCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	staticKeys = (Mult_StaticKey *)malloc(sizeof(Mult_StaticKey) *
	    staticKeysCount);
	for (i = 0; i < staticKeysCount; i++, dataPtr += 4) {
		staticKeys[i].frame = (int16)READ_LE_UINT16(dataPtr);
		staticKeys[i].layer = (int16)READ_LE_UINT16(dataPtr + 2);
	}

	for (j = 0; j < 4; j++) {
		animKeysCount[j] = READ_LE_UINT16(dataPtr);
		dataPtr += 2;

		animKeys[j] = (Mult_AnimKey *) malloc(sizeof(Mult_AnimKey) * animKeysCount[j]);
		for (i = 0; i < animKeysCount[j]; i++, dataPtr += 10) {
			animKeys[j][i].frame = (int16)READ_LE_UINT16(dataPtr);
			animKeys[j][i].layer = (int16)READ_LE_UINT16(dataPtr + 2);
			animKeys[j][i].posX = (int16)READ_LE_UINT16(dataPtr + 4);
			animKeys[j][i].posY = (int16)READ_LE_UINT16(dataPtr + 6);
			animKeys[j][i].order = (int16)READ_LE_UINT16(dataPtr + 8);
		}
	}

	for (palIndex = 0; palIndex < 5; palIndex++) {
		for (i = 0; i < 16; i++) {
			fadePal[palIndex][i].red = dataPtr[0];
			fadePal[palIndex][i].green = dataPtr[1];
			fadePal[palIndex][i].blue = dataPtr[2];
			dataPtr += 3;
		}
	}

	palFadeKeysCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;
	palFadeKeys = (Mult_PalFadeKey *)malloc(sizeof(Mult_PalFadeKey) * palFadeKeysCount);

	for (i = 0; i < palFadeKeysCount; i++, dataPtr += 7) {
		palFadeKeys[i].frame = (int16)READ_LE_UINT16(dataPtr);
		palFadeKeys[i].fade = (int16)READ_LE_UINT16(dataPtr + 2);
		palFadeKeys[i].palIndex = (int16)READ_LE_UINT16(dataPtr + 4);
		palFadeKeys[i].flag = *(dataPtr + 6);
	}

	palKeysCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	palKeys = (Mult_PalKey *)malloc(sizeof(Mult_PalKey) * palKeysCount);
	for (i = 0; i < palKeysCount; i++, dataPtr += 80) {
		palKeys[i].frame = (int16)READ_LE_UINT16(dataPtr);
		palKeys[i].cmd = (int16)READ_LE_UINT16(dataPtr + 2);
		palKeys[i].rates[0] = (int16)READ_LE_UINT16(dataPtr + 4);
		palKeys[i].rates[1] = (int16)READ_LE_UINT16(dataPtr + 6);
		palKeys[i].rates[2] = (int16)READ_LE_UINT16(dataPtr + 8);
		palKeys[i].rates[3] = (int16)READ_LE_UINT16(dataPtr + 10);
		palKeys[i].unknown0 = (int16)READ_LE_UINT16(dataPtr + 12);
		palKeys[i].unknown1 = (int16)READ_LE_UINT16(dataPtr + 14);
		memcpy(palKeys[i].subst, dataPtr + 16, 64);
	}

	textKeysCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;
	textKeys = (Mult_TextKey *) malloc(sizeof(Mult_TextKey) * textKeysCount);

	for (i = 0; i < textKeysCount; i++, dataPtr += 28) {
		textKeys[i].frame = (int16)READ_LE_UINT16(dataPtr);
		textKeys[i].cmd = (int16)READ_LE_UINT16(dataPtr + 2);
		for (int k = 0; k < 9; ++k)
			textKeys[i].unknown0[k] = (int16)READ_LE_UINT16(dataPtr + 4 + (k * 2));
		textKeys[i].index = (int16)READ_LE_UINT16(dataPtr + 22);
		textKeys[i].unknown1[0] = (int16)READ_LE_UINT16(dataPtr + 24);
		textKeys[i].unknown1[1] = (int16)READ_LE_UINT16(dataPtr + 26);
	}

	sndKeysCount = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	sndKeys = (Mult_SndKey *)malloc(sizeof(Mult_SndKey) * sndKeysCount);
	for (i = 0; i < sndKeysCount; i++) {
		sndKeys[i].frame = (int16)READ_LE_UINT16(dataPtr);
		sndKeys[i].cmd = (int16)READ_LE_UINT16(dataPtr + 2);
		sndKeys[i].freq = (int16)READ_LE_UINT16(dataPtr + 4);
		sndKeys[i].channel = (int16)READ_LE_UINT16(dataPtr + 6);
		sndKeys[i].repCount = (int16)READ_LE_UINT16(dataPtr + 8);
		sndKeys[i].resId = (int16)READ_LE_UINT16(dataPtr + 10);
		sndKeys[i].soundIndex = (int16)READ_LE_UINT16(dataPtr + 12);

		sndKeys[i].soundIndex = -1;
		sndKeys[i].resId = -1;
		dataPtr += 36;
		switch (sndKeys[i].cmd) {
		case 1:
		case 4:
			sndKeys[i].resId = READ_LE_UINT16(_vm->_global->_inter_execPtr);

			for (j = 0; j < i; j++) {
				if (sndKeys[i].resId ==
				    sndKeys[j].resId) {
					sndKeys[i].soundIndex =
					    sndKeys[j].soundIndex;
					_vm->_global->_inter_execPtr += 2;
					break;
				}
			}
			if (i == j) {
				_vm->_game->interLoadSound(19 - sndSlotsCount);
				sndKeys[i].soundIndex =
				    19 - sndSlotsCount;
				sndSlotsCount++;
			}
			break;

		case 3:
			_vm->_global->_inter_execPtr += 6;
			break;

		case 5:
			_vm->_global->_inter_execPtr += sndKeys[i].freq * 2;
			break;
		}
	}
}

void Mult::freeMultKeys(void) {
	int i;
	char animCount;
	char staticCount;

	dataPtr = multData;
	staticCount = dataPtr[0];
	animCount = dataPtr[1];

	free(dataPtr);

	staticCount++;
	animCount++;
	for (i = 0; i < staticCount; i++) {

		if (staticLoaded[i] != 0)
			_vm->_scenery->freeStatic(staticIndices[i]);
	}

	for (i = 0; i < animCount; i++) {
		if (animLoaded[i] != 0)
			_vm->_scenery->freeAnim(animIndices[i]);
	}

	free(staticKeys);

	for (i = 0; i < 4; i++)
		free(animKeys[i]);

	free(palFadeKeys);
	free(palKeys);
	free(textKeys);

	for (i = 0; i < sndSlotsCount; i++) {
		_vm->_game->freeSoundSlot(19 - i);
	}

	free(sndKeys);

	multData = 0;

	if (animDataAllocated != 0) {
		free(objects);
		objects = 0;

		free(renderData);
		renderData = 0;

		free(animArrayX);
		animArrayX = 0;

		free(animArrayY);
		animArrayY = 0;

		free(animArrayData);
		animArrayData = 0;

		if (_vm->_anim->_animSurf)
			_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);
		_vm->_anim->_animSurf = 0;

		animDataAllocated = 0;
	}
}

void Mult::checkFreeMult(void) {
	if (multData != 0)
		freeMultKeys();
}

}				// End of namespace Gob
