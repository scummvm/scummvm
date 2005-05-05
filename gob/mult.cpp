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

Mult_Object *mult_objects;
int16 *mult_renderData;
int16 mult_objCount;
int16 mult_frame;

char *mult_multData;
char mult_doPalSubst;
int16 mult_counter;
int16 mult_frameRate;

int32 *mult_animArrayX;
int32 *mult_animArrayY;

Mult_AnimData *mult_animArrayData;

int16 mult_index;

// Static keys
int16 mult_staticKeysCount;
Mult_StaticKey *mult_staticKeys;
int16 mult_staticIndices[10];

// Anim keys
Mult_AnimKey *mult_animKeys[4];
int16 mult_animKeysCount[4];
int16 mult_animLayer;
int16 mult_animIndices[10];

// Text keys
int16 mult_textKeysCount;
Mult_TextKey *mult_textKeys;

int16 mult_frameStart;

// Palette keys
int16 mult_palKeyIndex;
int16 mult_palKeysCount;
Mult_PalKey *mult_palKeys;
Color *mult_oldPalette;
Color mult_palAnimPalette[256];
int16 mult_palAnimKey;
int16 mult_palAnimIndices[4];
int16 mult_palAnimRed[4];
int16 mult_palAnimGreen[4];
int16 mult_palAnimBlue[4];

// Palette fading
Mult_PalFadeKey *mult_palFadeKeys;
int16 mult_palFadeKeysCount;
char mult_palFadingRed;
char mult_palFadingGreen;
char mult_palFadingBlue;

Color mult_fadePal[5][16];

// Sounds 
int16 mult_sndKeysCount;
Mult_SndKey *mult_sndKeys;

char mult_animDataAllocated;

char *mult_dataPtr;
int16 mult_staticLoaded[10];
int16 mult_animLoaded[10];
int16 mult_sndSlotsCount;

void mult_animate(void) {
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

	if (mult_renderData == 0)
		return;

	pDirtyLefts = mult_renderData;
	pDirtyRights = pDirtyLefts + mult_objCount;
	pDirtyTops = pDirtyRights + mult_objCount;
	pDirtyBottoms = pDirtyTops + mult_objCount;
	pNeedRedraw = pDirtyBottoms + mult_objCount;
	pCurLefts = pNeedRedraw + mult_objCount;
	pCurRights = pCurLefts + mult_objCount;
	pCurTops = pCurRights + mult_objCount;
	pCurBottoms = pCurTops + mult_objCount;
	minOrder = 100;
	maxOrder = 0;

	//Find dirty areas
	for (i = 0; i < mult_objCount; i++) {
		pNeedRedraw[i] = 0;
		pDirtyTops[i] = 1000;
		pDirtyLefts[i] = 1000;
		pDirtyBottoms[i] = 1000;
		pDirtyRights[i] = 1000;
		pAnimData = mult_objects[i].pAnimData;

		if (pAnimData->isStatic == 0 && pAnimData->isPaused == 0 &&
		    mult_objects[i].tick == pAnimData->maxTick) {
			if (pAnimData->order < minOrder)
				minOrder = pAnimData->order;

			if (pAnimData->order > maxOrder)
				maxOrder = pAnimData->order;

			pNeedRedraw[i] = 1;
			scen_updateAnim(pAnimData->layer, pAnimData->frame,
			    pAnimData->animation, 0,
			    *(mult_objects[i].pPosX), *(mult_objects[i].pPosY),
			    0);

			if (mult_objects[i].lastLeft != -1) {
				pDirtyLefts[i] =
				    MIN(mult_objects[i].lastLeft,
				    scen_toRedrawLeft);
				pDirtyTops[i] =
				    MIN(mult_objects[i].lastTop,
				    scen_toRedrawTop);
				pDirtyRights[i] =
				    MAX(mult_objects[i].lastRight,
				    scen_toRedrawRight);
				pDirtyBottoms[i] =
				    MAX(mult_objects[i].lastBottom,
				    scen_toRedrawBottom);
			} else {
				pDirtyLefts[i] = scen_toRedrawLeft;
				pDirtyTops[i] = scen_toRedrawTop;
				pDirtyRights[i] = scen_toRedrawRight;
				pDirtyBottoms[i] = scen_toRedrawBottom;
			}
			pCurLefts[i] = scen_toRedrawLeft;
			pCurRights[i] = scen_toRedrawRight;
			pCurTops[i] = scen_toRedrawTop;
			pCurBottoms[i] = scen_toRedrawBottom;
		} else {
			if (mult_objects[i].lastLeft != -1) {
				if (pAnimData->order < minOrder)
					minOrder = pAnimData->order;

				if (pAnimData->order > maxOrder)
					maxOrder = pAnimData->order;

				if (pAnimData->isStatic)
					*pNeedRedraw = 1;

				pCurLefts[i] = mult_objects[i].lastLeft;
				pDirtyLefts[i] = mult_objects[i].lastLeft;

				pCurTops[i] = mult_objects[i].lastTop;
				pDirtyTops[i] = mult_objects[i].lastTop;

				pCurRights[i] = mult_objects[i].lastRight;
				pDirtyRights[i] = mult_objects[i].lastRight;

				pCurBottoms[i] = mult_objects[i].lastBottom;
				pDirtyBottoms[i] = mult_objects[i].lastBottom;
			}
		}
	}

	// Find intersections
	for (i = 0; i < mult_objCount; i++) {
		pAnimData = mult_objects[i].pAnimData;
		pAnimData->intersected = 200;

		if (pAnimData->isStatic)
			continue;

		for (j = 0; j < mult_objCount; j++) {
			if (i == j)
				continue;

			if (mult_objects[j].pAnimData->isStatic)
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
	for (i = 0; i < mult_objCount; i++) {

		if (pNeedRedraw[i] == 0 || mult_objects[i].lastLeft == -1)
			continue;

		draw_sourceSurface = 22;
		draw_destSurface = 21;
		draw_spriteLeft = pDirtyLefts[i] - anim_animAreaLeft;
		draw_spriteTop = pDirtyTops[i] - anim_animAreaTop;
		draw_spriteRight = pDirtyRights[i] - pDirtyLefts[i] + 1;
		draw_spriteBottom = pDirtyBottoms[i] - pDirtyTops[i] + 1;
		draw_destSpriteX = pDirtyLefts[i];
		draw_destSpriteY = pDirtyTops[i];
		draw_transparency = 0;
		draw_spriteOperation(DRAW_BLITSURF);
		mult_objects[i].lastLeft = -1;
	}

	// Update view
	for (order = minOrder; order <= maxOrder; order++) {
		for (i = 0; i < mult_objCount; i++) {
			pAnimData = mult_objects[i].pAnimData;
			if (pAnimData->order != order)
				continue;

			if (pNeedRedraw[i]) {
				if (pAnimData->isStatic == 0) {

					scen_updateAnim(pAnimData->layer,
					    pAnimData->frame,
					    pAnimData->animation, 2,
					    *(mult_objects[i].pPosX),
					    *(mult_objects[i].pPosY), 1);

					if (scen_toRedrawLeft != -12345) {
						mult_objects[i].lastLeft =
						    scen_toRedrawLeft;
						mult_objects[i].lastTop =
						    scen_toRedrawTop;
						mult_objects[i].lastRight =
						    scen_toRedrawRight;
						mult_objects[i].lastBottom =
						    scen_toRedrawBottom;
					} else {
						mult_objects[i].lastLeft = -1;
					}
				}
				scen_updateStatic(order + 1);
			} else if (pAnimData->isStatic == 0) {
				for (j = 0; j < mult_objCount; j++) {
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

					scen_toRedrawLeft = pDirtyLefts[j];
					scen_toRedrawRight = pDirtyRights[j];
					scen_toRedrawTop = pDirtyTops[j];
					scen_toRedrawBottom = pDirtyBottoms[j];

					scen_updateAnim(pAnimData->layer,
					    pAnimData->frame,
					    pAnimData->animation, 4,
					    *(mult_objects[i].pPosX),
					    *(mult_objects[i].pPosY), 1);

					scen_updateStatic(order + 1);
				}
			}
		}
	}

	// Advance animations
	for (i = 0; i < mult_objCount; i++) {
		pAnimData = mult_objects[i].pAnimData;
		if (pAnimData->isStatic || pAnimData->isPaused)
			continue;

		if (mult_objects[i].tick == pAnimData->maxTick) {
			mult_objects[i].tick = 0;
			if (pAnimData->animType == 4) {
				pAnimData->isPaused = 1;
				pAnimData->frame = 0;
			} else {
				pAnimData->frame++;
				if (pAnimData->frame >=
				    scen_animations[(int)pAnimData->animation].layers[pAnimData->layer]->framesCount) {
					switch (pAnimData->animType) {
					case 0:
						pAnimData->frame = 0;
						break;

					case 1:
						pAnimData->frame = 0;

						*(mult_objects[i].pPosX) =
						    *(mult_objects[i].pPosX) +
						    scen_animations[(int)pAnimData->animation].layers[pAnimData->layer]->animDeltaX;

						*(mult_objects[i].pPosY) =
						    *(mult_objects[i].pPosY) +
						    scen_animations[(int)pAnimData->animation].layers[pAnimData->layer]->animDeltaY;
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
			mult_objects[i].tick++;
		}
	}
}

void mult_interGetObjAnimSize(void) {
	Mult_AnimData *pAnimData;
	int16 objIndex;

	inter_evalExpr(&objIndex);
	pAnimData = mult_objects[objIndex].pAnimData;
	if (pAnimData->isStatic == 0) {
		scen_updateAnim(pAnimData->layer, pAnimData->frame,
		    pAnimData->animation, 0, *(mult_objects[objIndex].pPosX),
		    *(mult_objects[objIndex].pPosY), 0);
	}
	WRITE_VAR_OFFSET(parse_parseVarIndex(), scen_toRedrawLeft);
	WRITE_VAR_OFFSET(parse_parseVarIndex(), scen_toRedrawTop);
	WRITE_VAR_OFFSET(parse_parseVarIndex(), scen_toRedrawRight);
	WRITE_VAR_OFFSET(parse_parseVarIndex(), scen_toRedrawBottom);
}

void mult_interInitMult(void) {
	int16 oldAnimHeight;
	int16 oldAnimWidth;
	int16 oldObjCount;
	int16 i;
	int16 posXVar;
	int16 posYVar;
	int16 animDataVar;

	oldAnimWidth = anim_animAreaWidth;
	oldAnimHeight = anim_animAreaHeight;
	oldObjCount = mult_objCount;

	anim_animAreaLeft = inter_load16();
	anim_animAreaTop = inter_load16();
	anim_animAreaWidth = inter_load16();
	anim_animAreaHeight = inter_load16();
	mult_objCount = inter_load16();
	posXVar = parse_parseVarIndex();
	posYVar = parse_parseVarIndex();
	animDataVar = parse_parseVarIndex();

	if (mult_objects == 0) {
		mult_renderData = (int16 *)malloc(sizeof(int16) * mult_objCount * 9);
		mult_objects = (Mult_Object *)malloc(sizeof(Mult_Object) * mult_objCount);

		for (i = 0; i < mult_objCount; i++) {
			mult_objects[i].pPosX = (int32 *)(inter_variables + i * 4 + (posXVar / 4) * 4);
			mult_objects[i].pPosY = (int32 *)(inter_variables + i * 4 + (posYVar / 4) * 4);
			mult_objects[i].pAnimData =
			    (Mult_AnimData *) (inter_variables + animDataVar +
			    i * 4 * inter_animDataSize);

			mult_objects[i].pAnimData->isStatic = 1;
			mult_objects[i].tick = 0;
			mult_objects[i].lastLeft = -1;
			mult_objects[i].lastRight = -1;
			mult_objects[i].lastTop = -1;
			mult_objects[i].lastBottom = -1;
		}
	} else if (oldObjCount != mult_objCount) {
		error("mult_interInitMult: Object count changed, but storage didn't (old count = %d, new count = %d)",
		    oldObjCount, mult_objCount);
	}

	if (anim_underAnimSurf != 0 &&
	    (oldAnimWidth != anim_animAreaWidth
		|| oldAnimHeight != anim_animAreaHeight)) {
		vid_freeSurfDesc(anim_underAnimSurf);
		anim_underAnimSurf = 0;
	}

	if (anim_underAnimSurf == 0) {
		anim_underAnimSurf = vid_initSurfDesc(videoMode,
		    anim_animAreaWidth, anim_animAreaHeight, 0);

		draw_spritesArray[22] = anim_underAnimSurf;
	}

	vid_drawSprite(draw_backSurface, anim_underAnimSurf,
	    anim_animAreaLeft, anim_animAreaTop,
	    anim_animAreaLeft + anim_animAreaWidth - 1,
	    anim_animAreaTop + anim_animAreaHeight - 1, 0, 0, 0);

	debug(0, "mult_interInitMult: x = %d, y = %d, w = %d, h = %d",
		  anim_animAreaLeft, anim_animAreaTop, anim_animAreaWidth, anim_animAreaHeight);
	debug(0, "    objCount = %d, animation data size = %d", mult_objCount, inter_animDataSize);
}

void mult_freeMult(void) {
	if (anim_underAnimSurf != 0)
		vid_freeSurfDesc(anim_underAnimSurf);

	free(mult_objects);
	free(mult_renderData);

	mult_objects = 0;
	mult_renderData = 0;
	anim_underAnimSurf = 0;
}

void mult_interLoadMult(void) {
	int16 val;
	int16 objIndex;
	int16 i;
	char *multData;

	debug(0, "mult_interLoadMult: Loading...");

	inter_evalExpr(&objIndex);
	inter_evalExpr(&val);
	*mult_objects[objIndex].pPosX = val;
	inter_evalExpr(&val);
	*mult_objects[objIndex].pPosY = val;

	multData = (char *)mult_objects[objIndex].pAnimData;
	for (i = 0; i < 11; i++) {
		if ((char)READ_LE_UINT16(inter_execPtr) == (char)99) {
			inter_evalExpr(&val);
			multData[i] = val;
		} else {
			inter_execPtr++;
		}
	}
}

void mult_freeAll(void) {
	int16 i;

	mult_freeMult();
	for (i = 0; i < 10; i++)
		scen_freeAnim(i);

	for (i = 0; i < 10; i++)
		scen_freeStatic(i);
}

void mult_initAll(void) {
	int16 i;

	mult_objects = 0;
	anim_underAnimSurf = 0;
	mult_renderData = 0;

	for (i = 0; i < 10; i++)
		scen_animPictCount[i] = 0;

	for (i = 0; i < 20; i++) {
		scen_spriteRefs[i] = 0;
		scen_spriteResId[i] = -1;
	}

	for (i = 0; i < 10; i++)
		scen_staticPictCount[i] = -1;

	scen_curStaticLayer = -1;
	scen_curStatic = -1;
}

void mult_playSound(Snd_SoundDesc * soundDesc, int16 repCount, int16 freq,
	    int16 channel) {
	snd_playSample(soundDesc, repCount, freq);
}

char mult_drawStatics(char stop) {
	if (mult_staticKeys[mult_staticKeysCount - 1].frame > mult_frame)
		stop = 0;

	for (mult_counter = 0; mult_counter < mult_staticKeysCount;
	    mult_counter++) {
		if (mult_staticKeys[mult_counter].frame != mult_frame
		    || mult_staticKeys[mult_counter].layer == -1)
			continue;

		for (scen_curStatic = 0, scen_curStaticLayer = mult_staticKeys[mult_counter].layer;
			 scen_curStaticLayer >= scen_statics[mult_staticIndices[scen_curStatic]].layersCount; 
			 scen_curStatic++) {
			scen_curStaticLayer -=
			    scen_statics[mult_staticIndices[scen_curStatic]].layersCount;
		}

		scen_curStatic = mult_staticIndices[scen_curStatic];
		scen_renderStatic(scen_curStatic, scen_curStaticLayer);
		vid_drawSprite(draw_backSurface, anim_underAnimSurf,
		    0, 0, 319, 199, 0, 0, 0);
	}
	return stop;
}

void mult_drawAnims(void) {
	Mult_AnimKey *key;
	Mult_Object *animObj;
	int16 i;
	int16 count;

	for (mult_index = 0; mult_index < 4; mult_index++) {
		for (mult_counter = 0; mult_counter < mult_animKeysCount[mult_index]; mult_counter++) {
			key = &mult_animKeys[mult_index][mult_counter];
			animObj = &mult_objects[mult_index];
			if (key->frame != mult_frame)
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

				count = scen_animations[mult_animIndices[0]].layersCount;
				i = 0;
				while (animObj->pAnimData->layer >= count) {
					animObj->pAnimData->layer -= count;
					i++;

					count = scen_animations[mult_animIndices[i]].layersCount;
				}
				animObj->pAnimData->animation = mult_animIndices[i];
			} else {
				animObj->pAnimData->isStatic = 1;
			}
		}
	}
}

void mult_drawText(char *pStop, char *pStopNoClear) {
	char *savedIP;

	int16 cmd;
	for (mult_index = 0; mult_index < mult_textKeysCount; mult_index++) {
		if (mult_textKeys[mult_index].frame != mult_frame)
			continue;

		cmd = mult_textKeys[mult_index].cmd;
		if (cmd == 0) {
			*pStop = 0;
		} else if (cmd == 1) {
			*pStopNoClear = 1;
			mult_frameStart = 0;
		} else if (cmd == 3) {
			*pStop = 0;
			savedIP = inter_execPtr;
			inter_execPtr = (char *)(&mult_textKeys[mult_index].index);
			inter_execPtr = savedIP;
		}
	}
}

char mult_prepPalAnim(char stop) {
	mult_palKeyIndex = -1;
	do {
		mult_palKeyIndex++;
		if (mult_palKeyIndex >= mult_palKeysCount)
			return stop;
	} while (mult_palKeys[mult_palKeyIndex].frame != mult_frame);

	if (mult_palKeys[mult_palKeyIndex].cmd == -1) {
		stop = 0;
		mult_doPalSubst = 0;
		pPaletteDesc->vgaPal = mult_oldPalette;

		memcpy((char *)mult_palAnimPalette, (char *)pPaletteDesc->vgaPal, 768);

		vid_setFullPalette(pPaletteDesc);
	} else {
		stop = 0;
		mult_doPalSubst = 1;
		mult_palAnimKey = mult_palKeyIndex;

		mult_palAnimIndices[0] = 0;
		mult_palAnimIndices[1] = 0;
		mult_palAnimIndices[2] = 0;
		mult_palAnimIndices[3] = 0;

		pPaletteDesc->vgaPal = mult_palAnimPalette;
	}
	return stop;
}

void mult_doPalAnim(void) {
	int16 off;
	int16 off2;
	Color *palPtr;
	Mult_PalKey *palKey;

	if (mult_doPalSubst == 0)
		return;

	for (mult_index = 0; mult_index < 4; mult_index++) {
		palKey = &mult_palKeys[mult_palAnimKey];

		if ((mult_frame % palKey->rates[mult_index]) != 0)
			continue;

		mult_palAnimRed[mult_index] =
		    pPaletteDesc->vgaPal[palKey->subst[0][mult_index] - 1].red;
		mult_palAnimGreen[mult_index] =
		    pPaletteDesc->vgaPal[palKey->subst[0][mult_index] - 1].green;
		mult_palAnimBlue[mult_index] =
		    pPaletteDesc->vgaPal[palKey->subst[0][mult_index] - 1].blue;

		while (1) {
			off = palKey->subst[(mult_palAnimIndices[mult_index] + 1) % 16][mult_index];
			if (off == 0) {
				off = palKey->subst[mult_palAnimIndices[mult_index]][mult_index] - 1;

				pPaletteDesc->vgaPal[off].red = mult_palAnimRed[mult_index];
				pPaletteDesc->vgaPal[off].green = mult_palAnimGreen[mult_index];
				pPaletteDesc->vgaPal[off].blue = mult_palAnimBlue[mult_index];
			} else {
				off = palKey->subst[(mult_palAnimIndices[mult_index] + 1) % 16][mult_index] - 1;
				off2 = palKey->subst[mult_palAnimIndices[mult_index]][mult_index] - 1;

				pPaletteDesc->vgaPal[off2].red = pPaletteDesc->vgaPal[off].red;
				pPaletteDesc->vgaPal[off2].green = pPaletteDesc->vgaPal[off].green;
				pPaletteDesc->vgaPal[off2].blue = pPaletteDesc->vgaPal[off].blue;
			}

			mult_palAnimIndices[mult_index] = (mult_palAnimIndices[mult_index] + 1) % 16;

			off = palKey->subst[mult_palAnimIndices[mult_index]][mult_index];

			if (off == 0) {
				mult_palAnimIndices[mult_index] = 0;
				off = palKey->subst[0][mult_index] - 1;

				mult_palAnimRed[mult_index] = pPaletteDesc->vgaPal[off].red;
				mult_palAnimGreen[mult_index] = pPaletteDesc->vgaPal[off].green;
				mult_palAnimBlue[mult_index] = pPaletteDesc->vgaPal[off].blue;
			}
			if (mult_palAnimIndices[mult_index] == 0)
				break;
		}
	}

	if (colorCount == 256) {
		vid_waitRetrace(videoMode);

		palPtr = pPaletteDesc->vgaPal;
		for (mult_counter = 0; mult_counter < 16; mult_counter++) {
			vid_setPalElem(mult_counter, palPtr->red, palPtr->green, palPtr->blue, 0, 0x13);
			palPtr++;
		}

		palPtr = pPaletteDesc->vgaPal;
		for (mult_counter = 0; mult_counter < 16; mult_counter++) {
			redPalette[mult_counter] = palPtr->red;
			greenPalette[mult_counter] = palPtr->green;
			bluePalette[mult_counter] = palPtr->blue;
			palPtr++;
		}
	} else {
		vid_setFullPalette(pPaletteDesc);
	}
}

char mult_doFadeAnim(char stop) {
	Mult_PalFadeKey *fadeKey;

	for (mult_index = 0; mult_index < mult_palFadeKeysCount; mult_index++) {
		fadeKey = &mult_palFadeKeys[mult_index];

		if (fadeKey->frame != mult_frame)
			continue;

		stop = 0;
		if ((fadeKey->flag & 1) == 0) {
			if (fadeKey->fade == 0) {
				pPaletteDesc->vgaPal = mult_fadePal[fadeKey->palIndex];
				vid_setFullPalette(pPaletteDesc);
			} else {
				pPaletteDesc->vgaPal = mult_fadePal[fadeKey->palIndex];
				pal_fade(pPaletteDesc, fadeKey->fade, 0);
			}
		} else {
			pPaletteDesc->vgaPal = mult_fadePal[fadeKey->palIndex];
			pal_fade(pPaletteDesc, fadeKey->fade, -1);

			mult_palFadingRed = (fadeKey->flag >> 1) & 1;
			mult_palFadingGreen = (fadeKey->flag >> 2) & 1;
			mult_palFadingBlue = (fadeKey->flag >> 3) & 1;
		}
	}

	if (mult_palFadingRed) {
		mult_palFadingRed = !pal_fadeStep(1);
		stop = 0;
	}
	if (mult_palFadingGreen) {
		mult_palFadingGreen = !pal_fadeStep(2);
		stop = 0;
	}
	if (mult_palFadingBlue) {
		mult_palFadingBlue = !pal_fadeStep(3);
		stop = 0;
	}
	return stop;
}

char mult_doSoundAnim(char stop) {
	Mult_SndKey *sndKey;
	for (mult_index = 0; mult_index < mult_sndKeysCount; mult_index++) {
		sndKey = &mult_sndKeys[mult_index];
		if (sndKey->frame != mult_frame)
			continue;

		if (sndKey->cmd != -1) {
			if (sndKey->cmd == 1) {
				snd_stopSound(0);
				stop = 0;
				mult_playSound(game_soundSamples[sndKey->soundIndex], sndKey->repCount,
				    sndKey->freq, sndKey->channel);

			} else if (sndKey->cmd == 4) {
				snd_stopSound(0);
				stop = 0;
				mult_playSound(game_soundSamples[sndKey->soundIndex], sndKey->repCount,
				    sndKey->freq, sndKey->channel);
			}
		} else {
			if (snd_playingSound)
				snd_stopSound(sndKey->channel);
		}
	}
	return stop;
}

void mult_playMult(int16 startFrame, int16 endFrame, char checkEscape,
	    char handleMouse) {
	char stopNoClear;
	char stop;
	Mult_Object *multObj;
	Mult_AnimData *animData;

	if (mult_multData == 0)
		return;

	stopNoClear = 0;
	mult_frame = startFrame;
	if (endFrame == -1)
		endFrame = 32767;

	if (mult_frame == -1) {
		mult_doPalSubst = 0;
		mult_palFadingRed = 0;
		mult_palFadingGreen = 0;
		mult_palFadingBlue = 0;

		mult_oldPalette = pPaletteDesc->vgaPal;
		memcpy((char *)mult_palAnimPalette, (char *)pPaletteDesc->vgaPal, 768);

		if (anim_underAnimSurf == 0) {
			util_setFrameRate(mult_frameRate);
			anim_animAreaTop = 0;
			anim_animAreaLeft = 0;
			anim_animAreaWidth = 320;
			anim_animAreaHeight = 200;
			mult_objCount = 4;

			mult_objects = (Mult_Object *)malloc(sizeof(Mult_Object) * mult_objCount);
			mult_renderData = (int16 *)malloc(sizeof(int16) * 9 * mult_objCount);

			mult_animArrayX = (int32 *)malloc(sizeof(int32) * mult_objCount);
			mult_animArrayY = (int32 *)malloc(sizeof(int32) * mult_objCount);

			mult_animArrayData = (Mult_AnimData *)malloc(sizeof(Mult_AnimData) * mult_objCount);

			for (mult_counter = 0; mult_counter < mult_objCount; mult_counter++) {
				multObj = &mult_objects[mult_counter];

				multObj->pPosX = (int32 *)&mult_animArrayX[mult_counter];
				multObj->pPosY = (int32 *)&mult_animArrayY[mult_counter];

				multObj->pAnimData = &mult_animArrayData[mult_counter];

				animData = multObj->pAnimData;
				animData->isStatic = 1;

				multObj->tick = 0;
				multObj->lastLeft = -1;
				multObj->lastTop = -1;
				multObj->lastRight = -1;
				multObj->lastBottom = -1;
			}

			anim_underAnimSurf =
			    vid_initSurfDesc(videoMode, 320, 200, 0);
			draw_spritesArray[22] = anim_underAnimSurf;

			vid_drawSprite(draw_backSurface, anim_underAnimSurf,
			    0, 0, 319, 199, 0, 0, 0);

			mult_animDataAllocated = 1;
		} else
			mult_animDataAllocated = 0;
		mult_frame = 0;
	}

	do {
		stop = 1;

		if (VAR(58) == 0) {
			stop = mult_drawStatics(stop);
			mult_drawAnims();
		}

		mult_animate();
		if (handleMouse) {
			draw_animateCursor(-1);
		} else {
			draw_blitInvalidated();
		}

		if (VAR(58) == 0) {
			mult_drawText(&stop, &stopNoClear);
		}

		stop = mult_prepPalAnim(stop);
		mult_doPalAnim();

		stop = mult_doFadeAnim(stop);
		stop = mult_doSoundAnim(stop);

		if (mult_frame >= endFrame)
			stopNoClear = 1;

		if (snd_playingSound)
			stop = 0;

		util_processInput();
		if (checkEscape && util_checkKey() == 0x11b)	// Esc
			stop = 1;

		mult_frame++;
		util_waitEndFrame();
	} while (stop == 0 && stopNoClear == 0);

	if (stopNoClear == 0) {
		if (mult_animDataAllocated) {
			free(mult_objects);
			mult_objects = 0;

			free(mult_renderData);
			mult_renderData = 0;

			free(mult_animArrayX);
			mult_animArrayX = 0;

			free(mult_animArrayY);
			mult_animArrayY = 0;

			free(mult_animArrayData);
			mult_animArrayData = 0;

			if (anim_underAnimSurf)
				vid_freeSurfDesc(anim_underAnimSurf);
			anim_underAnimSurf = 0;

			mult_animDataAllocated = 0;
		}

		if (snd_playingSound != 0)
			snd_stopSound(10);

		WRITE_VAR(57, (uint32)-1);
	} else {
		WRITE_VAR(57, mult_frame - 1 - mult_frameStart);
	}
}

void mult_zeroMultData(void) {
	mult_multData = 0;
}

void mult_loadMult(int16 resId) {
	char animCount;
	char staticCount;
	int16 palIndex;
	int16 i, j;

	mult_sndSlotsCount = 0;
	mult_frameStart = 0;
	mult_multData = game_loadExtData(resId, 0, 0);
	mult_dataPtr = mult_multData;

	staticCount = mult_dataPtr[0];
	animCount = mult_dataPtr[1];
	mult_dataPtr += 2;
	staticCount++;
	animCount++;

	for (i = 0; i < staticCount; i++, mult_dataPtr += 14) {
		mult_staticIndices[i] = scen_loadStatic(1);

		if (mult_staticIndices[i] >= 100) {
			mult_staticIndices[i] -= 100;
			mult_staticLoaded[i] = 1;
		} else {
			mult_staticLoaded[i] = 0;
		}
	}

	for (i = 0; i < animCount; i++, mult_dataPtr += 14) {
		mult_animIndices[i] = scen_loadAnim(1);

		if (mult_animIndices[i] >= 100) {
			mult_animIndices[i] -= 100;
			mult_animLoaded[i] = 1;
		} else {
			mult_animLoaded[i] = 0;
		}
	}

	mult_frameRate = READ_LE_UINT16(mult_dataPtr);
	mult_dataPtr += 2;

	mult_staticKeysCount = READ_LE_UINT16(mult_dataPtr);
	mult_dataPtr += 2;

	mult_staticKeys = (Mult_StaticKey *)malloc(sizeof(Mult_StaticKey) *
	    mult_staticKeysCount);
	for (i = 0; i < mult_staticKeysCount; i++, mult_dataPtr += 4) {
		mult_staticKeys[i].frame = (int16)READ_LE_UINT16(mult_dataPtr);
		mult_staticKeys[i].layer = (int16)READ_LE_UINT16(mult_dataPtr + 2);
	}

	for (j = 0; j < 4; j++) {
		mult_animKeysCount[j] = READ_LE_UINT16(mult_dataPtr);
		mult_dataPtr += 2;

		mult_animKeys[j] = (Mult_AnimKey *) malloc(sizeof(Mult_AnimKey) * mult_animKeysCount[j]);
		for (i = 0; i < mult_animKeysCount[j]; i++, mult_dataPtr += 10) {
			mult_animKeys[j][i].frame = (int16)READ_LE_UINT16(mult_dataPtr);
			mult_animKeys[j][i].layer = (int16)READ_LE_UINT16(mult_dataPtr + 2);
			mult_animKeys[j][i].posX = (int16)READ_LE_UINT16(mult_dataPtr + 4);
			mult_animKeys[j][i].posY = (int16)READ_LE_UINT16(mult_dataPtr + 6);
			mult_animKeys[j][i].order = (int16)READ_LE_UINT16(mult_dataPtr + 8);
		}
	}

	for (palIndex = 0; palIndex < 5; palIndex++) {
		for (i = 0; i < 16; i++) {
			mult_fadePal[palIndex][i].red = mult_dataPtr[0];
			mult_fadePal[palIndex][i].green = mult_dataPtr[1];
			mult_fadePal[palIndex][i].blue = mult_dataPtr[2];
			mult_dataPtr += 3;
		}
	}

	mult_palFadeKeysCount = READ_LE_UINT16(mult_dataPtr);
	mult_dataPtr += 2;
	mult_palFadeKeys = (Mult_PalFadeKey *)malloc(sizeof(Mult_PalFadeKey) * mult_palFadeKeysCount);

	for (i = 0; i < mult_palFadeKeysCount; i++, mult_dataPtr += 7) {
		mult_palFadeKeys[i].frame = (int16)READ_LE_UINT16(mult_dataPtr);
		mult_palFadeKeys[i].fade = (int16)READ_LE_UINT16(mult_dataPtr + 2);
		mult_palFadeKeys[i].palIndex = (int16)READ_LE_UINT16(mult_dataPtr + 4);
		mult_palFadeKeys[i].flag = *(mult_dataPtr + 6);
	}

	mult_palKeysCount = READ_LE_UINT16(mult_dataPtr);
	mult_dataPtr += 2;

	mult_palKeys = (Mult_PalKey *)malloc(sizeof(Mult_PalKey) * mult_palKeysCount);
	for (i = 0; i < mult_palKeysCount; i++, mult_dataPtr += 80) {
		mult_palKeys[i].frame = (int16)READ_LE_UINT16(mult_dataPtr);
		mult_palKeys[i].cmd = (int16)READ_LE_UINT16(mult_dataPtr + 2);
		mult_palKeys[i].rates[0] = (int16)READ_LE_UINT16(mult_dataPtr + 4);
		mult_palKeys[i].rates[1] = (int16)READ_LE_UINT16(mult_dataPtr + 6);
		mult_palKeys[i].rates[2] = (int16)READ_LE_UINT16(mult_dataPtr + 8);
		mult_palKeys[i].rates[3] = (int16)READ_LE_UINT16(mult_dataPtr + 10);
		mult_palKeys[i].unknown0 = (int16)READ_LE_UINT16(mult_dataPtr + 12);
		mult_palKeys[i].unknown1 = (int16)READ_LE_UINT16(mult_dataPtr + 14);
		memcpy(mult_palKeys[i].subst, mult_dataPtr + 16, 64);
	}

	mult_textKeysCount = READ_LE_UINT16(mult_dataPtr);
	mult_dataPtr += 2;
	mult_textKeys = (Mult_TextKey *) malloc(sizeof(Mult_TextKey) * mult_textKeysCount);

	for (i = 0; i < mult_textKeysCount; i++, mult_dataPtr += 28) {
		mult_textKeys[i].frame = (int16)READ_LE_UINT16(mult_dataPtr);
		mult_textKeys[i].cmd = (int16)READ_LE_UINT16(mult_dataPtr + 2);
		for (int k = 0; k < 9; ++k)
			mult_textKeys[i].unknown0[k] = (int16)READ_LE_UINT16(mult_dataPtr + 4 + (k * 2));
		mult_textKeys[i].index = (int16)READ_LE_UINT16(mult_dataPtr + 22);
		mult_textKeys[i].unknown1[0] = (int16)READ_LE_UINT16(mult_dataPtr + 24);
		mult_textKeys[i].unknown1[1] = (int16)READ_LE_UINT16(mult_dataPtr + 26);
	}

	mult_sndKeysCount = READ_LE_UINT16(mult_dataPtr);
	mult_dataPtr += 2;

	mult_sndKeys = (Mult_SndKey *)malloc(sizeof(Mult_SndKey) * mult_sndKeysCount);
	for (i = 0; i < mult_sndKeysCount; i++) {
		mult_sndKeys[i].frame = (int16)READ_LE_UINT16(mult_dataPtr);
		mult_sndKeys[i].cmd = (int16)READ_LE_UINT16(mult_dataPtr + 2);
		mult_sndKeys[i].freq = (int16)READ_LE_UINT16(mult_dataPtr + 4);
		mult_sndKeys[i].channel = (int16)READ_LE_UINT16(mult_dataPtr + 6);
		mult_sndKeys[i].repCount = (int16)READ_LE_UINT16(mult_dataPtr + 8);
		mult_sndKeys[i].resId = (int16)READ_LE_UINT16(mult_dataPtr + 10);
		mult_sndKeys[i].soundIndex = (int16)READ_LE_UINT16(mult_dataPtr + 12);
		
		mult_sndKeys[i].soundIndex = -1;
		mult_sndKeys[i].resId = -1;
		mult_dataPtr += 36;
		switch (mult_sndKeys[i].cmd) {
		case 1:
		case 4:
			mult_sndKeys[i].resId = READ_LE_UINT16(inter_execPtr);

			for (j = 0; j < i; j++) {
				if (mult_sndKeys[i].resId ==
				    mult_sndKeys[j].resId) {
					mult_sndKeys[i].soundIndex =
					    mult_sndKeys[j].soundIndex;
					inter_execPtr += 2;
					break;
				}
			}
			if (i == j) {
				game_interLoadSound(19 - mult_sndSlotsCount);
				mult_sndKeys[i].soundIndex =
				    19 - mult_sndSlotsCount;
				mult_sndSlotsCount++;
			}
			break;

		case 3:
			inter_execPtr += 6;
			break;

		case 5:
			inter_execPtr += mult_sndKeys[i].freq * 2;
			break;
		}
	}
}

void mult_freeMultKeys(void) {
	int i;
	char animCount;
	char staticCount;

	mult_dataPtr = mult_multData;
	staticCount = mult_dataPtr[0];
	animCount = mult_dataPtr[1];

	free(mult_dataPtr);

	staticCount++;
	animCount++;
	for (i = 0; i < staticCount; i++) {

		if (mult_staticLoaded[i] != 0)
			scen_freeStatic(mult_staticIndices[i]);
	}

	for (i = 0; i < animCount; i++) {
		if (mult_animLoaded[i] != 0)
			scen_freeAnim(mult_animIndices[i]);
	}

	free(mult_staticKeys);

	for (i = 0; i < 4; i++)
		free(mult_animKeys[i]);

	free(mult_palFadeKeys);
	free(mult_palKeys);
	free(mult_textKeys);

	for (i = 0; i < mult_sndSlotsCount; i++) {
		game_freeSoundSlot(19 - i);
	}

	free(mult_sndKeys);

	mult_multData = 0;

	if (mult_animDataAllocated != 0) {
		free(mult_objects);
		mult_objects = 0;

		free(mult_renderData);
		mult_renderData = 0;

		free(mult_animArrayX);
		mult_animArrayX = 0;

		free(mult_animArrayY);
		mult_animArrayY = 0;

		free(mult_animArrayData);
		mult_animArrayData = 0;

		if (anim_underAnimSurf)
			vid_freeSurfDesc(anim_underAnimSurf);
		anim_underAnimSurf = 0;

		mult_animDataAllocated = 0;
	}
}

void mult_checkFreeMult(void) {
	if (mult_multData != 0)
		mult_freeMultKeys();
}

}				// End of namespace Gob
