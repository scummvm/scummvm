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
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/anim.h"
#include "gob/scenery.h"
#include "gob/map.h"
#include "gob/sound.h"
#include "gob/game.h"
#include "gob/dataio.h"
#include "gob/cdrom.h"

namespace Gob {

Util_List *gob_objList;
Gob_Object *gob_goblins[4];
int16 gob_currentGoblin;
Snd_SoundDesc *gob_soundData[16];
int16 gob_gobStateLayer;
char gob_goesAtTarget = 0;
char gob_readyToAct = 0;
int16 gob_gobAction = 0;
Gob_Pos gob_gobPositions[3];
int16 gob_gobDestX;
int16 gob_gobDestY;
int16 gob_pressedMapX;
int16 gob_pressedMapY;
char gob_pathExistence;

int16 gob_itemIndInPocket = 5;
int16 gob_itemIdInPocket = 2;
char gob_itemByteFlag = 0;
int16 gob_destItemId = -1;
int16 gob_destActionItem = 0;
Gob_Object *gob_actDestItemDesc = 0;
int16 gob_forceNextState[10] = {
	-1, -1, -1, -1, -1, -1, -1, 0, 0, 0
};

char gob_rotStates[4][4] = {
	{0, 22, 23, 24},
	{13, 2, 12, 14},
	{16, 15, 4, 17},
	{27, 25, 26, 6}
};

int16 gob_destItemType;
int16 gob_destItemState;
// Pointers to interpreter variables
int32 *gob_some0ValPtr;

int32 *gob_gobRetVarPtr;
int32 *gob_curGobVarPtr;
int32 *gob_curGobXPosVarPtr;
int32 *gob_curGobYPosVarPtr;
int32 *gob_itemInPocketVarPtr;
int32 *gob_curGobStateVarPtr;
int32 *gob_curGobFrameVarPtr;
int32 *gob_curGobMultStateVarPtr;
int32 *gob_curGobNextStateVarPtr;
int32 *gob_curGobScrXVarPtr;
int32 *gob_curGobScrYVarPtr;
int32 *gob_curGobLeftVarPtr;
int32 *gob_curGobTopVarPtr;
int32 *gob_curGobRightVarPtr;
int32 *gob_curGobBottomVarPtr;
int32 *gob_curGobDoAnimVarPtr;
int32 *gob_curGobOrderVarPtr;
int32 *gob_curGobNoTickVarPtr;
int32 *gob_curGobTypeVarPtr;
int32 *gob_curGobMaxTickVarPtr;
int32 *gob_curGobTickVarPtr;
int32 *gob_curGobActStartStateVarPtr;
int32 *gob_curGobLookDirVarPtr;
int32 *gob_curGobPickableVarPtr;
int32 *gob_curGobRelaxVarPtr;
int32 *gob_curGobMaxFrameVarPtr;

int32 *gob_destItemStateVarPtr;
int32 *gob_destItemFrameVarPtr;
int32 *gob_destItemMultStateVarPtr;
int32 *gob_destItemNextStateVarPtr;
int32 *gob_destItemScrXVarPtr;
int32 *gob_destItemScrYVarPtr;
int32 *gob_destItemLeftVarPtr;
int32 *gob_destItemTopVarPtr;
int32 *gob_destItemRightVarPtr;
int32 *gob_destItemBottomVarPtr;
int32 *gob_destItemDoAnimVarPtr;
int32 *gob_destItemOrderVarPtr;
int32 *gob_destItemNoTickVarPtr;
int32 *gob_destItemTypeVarPtr;
int32 *gob_destItemMaxTickVarPtr;
int32 *gob_destItemTickVarPtr;
int32 *gob_destItemActStartStVarPtr;
int32 *gob_destItemLookDirVarPtr;
int32 *gob_destItemPickableVarPtr;
int32 *gob_destItemRelaxVarPtr;
int32 *gob_destItemMaxFrameVarPtr;

//
int16 gob_itemToObject[20];
Gob_Object *gob_objects[20];
int16 gob_objCount;
int16 gob_gobsCount;
char gob_boreCounter = 0;
int16 gob_positionedGob = 5;

char gob_noPick = 0;

char gob_rotateState(int16 from, int16 to) {
	return gob_rotStates[from / 2][to / 2];
}

int16 gob_peekGoblin(Gob_Object *curGob) {
	Util_ListNode *ptr;
	Gob_Object *desc;
	int16 index;
	int16 i;

	ptr = gob_objList->pHead;
	index = 0;
	while (ptr != 0) {
		desc = (Gob_Object *) ptr->pData;
		if (desc != curGob) {
			for (i = 0; i < 3; i++) {
				if (desc != gob_goblins[i])
					continue;

				if (inter_mouseX < desc->right &&
				    inter_mouseX > desc->left &&
				    inter_mouseY < desc->bottom &&
				    inter_mouseY > desc->top) {
					index = i + 1;
				}
			}
		}
		ptr = ptr->pNext;
	}
	return index;
}

void gob_initList(void) {
	gob_objList = (Util_List *) malloc(sizeof(Util_List));
	gob_objList->pHead = 0;
	gob_objList->pTail = 0;
}

void gob_sortByOrder(Util_List *list) {
	Util_ListNode *ptr;
	Util_ListNode *ptr2;
	void *tmp;

	ptr = list->pHead;
	while (ptr->pNext != 0) {
		for (ptr2 = ptr->pNext; ptr2 != 0; ptr2 = ptr2->pNext) {
			if (((Gob_Object *) ptr->pData)->order <=
			    ((Gob_Object *) ptr2->pData)->order) {
				if (((Gob_Object *) ptr->pData)->order !=
				    ((Gob_Object *) ptr2->pData)->order)
					continue;

				if (((Gob_Object *) ptr->pData)->bottom <=
				    ((Gob_Object *) ptr2->pData)->bottom) {
					if (((Gob_Object *) ptr->pData)->
					    bottom !=
					    ((Gob_Object *) ptr2->pData)->
					    bottom)
						continue;

					if ((Gob_Object *) ptr->pData !=
					    gob_goblins[gob_currentGoblin])
						continue;
				}
			}

			tmp = ptr->pData;
			ptr->pData = ptr2->pData;
			ptr2->pData = tmp;
		}
		ptr = ptr->pNext;
	}
}

void gob_playSound(Snd_SoundDesc *snd, int16 repCount, int16 freq) {
	if (snd != 0) {
		snd_stopSound(0);
		snd_playSample(snd, repCount, freq);
	}
}

void gob_drawObjects(void) {
	Util_ListNode *ptr;
	Util_ListNode *ptr2;

	Gob_Object *objDesc;
	Gob_Object *gobDesc2;
	int16 layer;

	ptr = gob_objList->pHead;
	for (ptr = gob_objList->pHead; ptr != 0; ptr = ptr->pNext) {
		objDesc = (Gob_Object *) ptr->pData;

		if (objDesc->type == 3)
			objDesc->toRedraw = 1;
		else if (objDesc->type == 1)
			objDesc->toRedraw = 0;
	}

	for (ptr = gob_objList->pHead; ptr != 0; ptr = ptr->pNext) {
		objDesc = (Gob_Object *) ptr->pData;
		if (objDesc->toRedraw == 0)
			continue;

		vid_drawSprite(anim_underAnimSurf, draw_backSurface,
		    objDesc->left, objDesc->top, objDesc->right,
		    objDesc->bottom, objDesc->left, objDesc->top, 0);

		draw_invalidateRect(objDesc->left, objDesc->top,
		    objDesc->right, objDesc->bottom);

		if (objDesc->type != 0)
			continue;

		layer =
		    objDesc->stateMach[objDesc->state][objDesc->stateColumn]->
		    layer;
		scen_updateAnim(layer, objDesc->curFrame, objDesc->animation,
		    0, objDesc->xPos, objDesc->yPos, 0);

		if (scen_toRedrawLeft == -12345) {
			objDesc->dirtyLeft = objDesc->left;
			objDesc->dirtyRight = objDesc->right;
			objDesc->dirtyTop = objDesc->top;
			objDesc->dirtyBottom = objDesc->bottom;
		} else {
			objDesc->dirtyLeft =
			    MIN(objDesc->left, scen_toRedrawLeft);
			objDesc->dirtyRight =
			    MAX(objDesc->right, scen_toRedrawRight);
			objDesc->dirtyTop =
			    MIN(objDesc->top, scen_toRedrawTop);
			objDesc->dirtyBottom =
			    MAX(objDesc->bottom, scen_toRedrawBottom);
		}

		objDesc->dirtyLeft = 0;
		objDesc->dirtyRight = 319;
		objDesc->dirtyTop = 0;
		objDesc->dirtyBottom = 199;
	}

	gob_sortByOrder(gob_objList);
	for (ptr = gob_objList->pHead; ptr != 0; ptr = ptr->pNext) {
		objDesc = (Gob_Object *) ptr->pData;
		if (objDesc->toRedraw) {
			layer =
			    objDesc->stateMach[objDesc->state][objDesc->
			    stateColumn]->layer;

			if (objDesc->type == 0) {
				if (objDesc->visible == 0) {
					scen_updateAnim(layer,
					    objDesc->curFrame,
					    objDesc->animation, 0,
					    objDesc->xPos, objDesc->yPos, 0);

				} else {
					scen_updateAnim(layer,
					    objDesc->curFrame,
					    objDesc->animation, 2,
					    objDesc->xPos, objDesc->yPos, 1);
				}
				if (scen_toRedrawLeft == -12345) {
					objDesc->left = 0;
					objDesc->top = 0;
					objDesc->right = 0;
					objDesc->bottom = 0;
				} else {
					draw_invalidateRect(scen_toRedrawLeft,
					    scen_toRedrawTop,
					    scen_toRedrawRight,
					    scen_toRedrawBottom);

					objDesc->left = scen_toRedrawLeft;
					objDesc->top = scen_toRedrawTop;
					objDesc->right = scen_toRedrawRight;
					objDesc->bottom = scen_toRedrawBottom;
					scen_updateStatic(objDesc->order);
				}
			} else {
				objDesc->left = 0;
				objDesc->top = 0;
				objDesc->right = 0;
				objDesc->bottom = 0;
				objDesc->type = 1;
			}
			continue;
		}

		if (objDesc->type == 0 && objDesc->visible != 0) {
			for (ptr2 = gob_objList->pHead; ptr2 != 0;
			    ptr2 = ptr2->pNext) {
				gobDesc2 = (Gob_Object *) ptr2->pData;

				if (gobDesc2->toRedraw == 0)
					continue;

				if (objDesc->right < gobDesc2->dirtyLeft)
					continue;

				if (gobDesc2->dirtyRight < objDesc->left)
					continue;

				if (objDesc->bottom < gobDesc2->dirtyTop)
					continue;

				if (gobDesc2->dirtyBottom < objDesc->top)
					continue;

				scen_toRedrawLeft = gobDesc2->dirtyLeft;
				scen_toRedrawRight = gobDesc2->dirtyRight;
				scen_toRedrawTop = gobDesc2->dirtyTop;
				scen_toRedrawBottom = gobDesc2->dirtyBottom;

				layer =
				    objDesc->stateMach[objDesc->
				    state][objDesc->stateColumn]->layer;

				scen_updateAnim(layer, objDesc->curFrame,
				    objDesc->animation, 4, objDesc->xPos,
				    objDesc->yPos, 1);

				scen_updateStatic(objDesc->order);
			}
		}
	}

	for (ptr = gob_objList->pHead; ptr != 0; ptr = ptr->pNext) {
		objDesc = (Gob_Object *) ptr->pData;
		if (objDesc->toRedraw == 0 || objDesc->type == 1)
			continue;

		Gob_State *state = objDesc->stateMach[objDesc->state][objDesc->stateColumn];
		int16 sndFrame;
		int16 sndItem;
		int16 freq;
		int16 repCount;

		if (state->sndFrame & 0xff00) {
			// There are two frames which trigger a sound effect,
			// so everything has to be encoded in one byte each.
			// Note that the frequency is multiplied by 100, not -
			// as I would have thought, 0x100.

			sndFrame = (state->sndFrame >> 8) & 0xff;
			sndItem = (state->sndItem >> 8) & 0xff;
			freq = 100 * ((state->freq >> 8) & 0xff);
			repCount = (state->repCount >> 8) & 0xff;

			if (objDesc->curFrame == sndFrame) {
				if (sndItem != 0xff) {
					gob_playSound(gob_soundData[sndItem],
					    repCount, freq);
				}
			}

			sndFrame = state->sndFrame & 0xff;
			sndItem = state->sndItem & 0xff;
			freq = 100 * (state->freq & 0xff);
			repCount = state->repCount & 0xff;

			if (objDesc->curFrame == sndFrame) {
				if (sndItem != 0xff) {
					gob_playSound(gob_soundData[sndItem],
					    repCount, freq);
				}
			}
		} else {
			// There is only one, so frequency etc. are used as is.
			sndFrame = state->sndFrame;
			sndItem = state->sndItem;
			freq = state->freq;
			repCount = state->repCount;

			if (objDesc->curFrame == sndFrame) {
				if (sndItem != -1) {
					gob_playSound(gob_soundData[sndItem],
					    repCount, freq);
				}
			}
		}
	}

//      scen_updateAnim(27, 0, 9, 2, 10, 10, 1);
}

void gob_animateObjects(void) {
	Util_ListNode *node;
	Gob_Object *objDesc;
	Scen_AnimLayer *pLayer;
	int16 layer;

	for (node = gob_objList->pHead; node != 0; node = node->pNext) {
		objDesc = (Gob_Object *) node->pData;
		if (objDesc->doAnim != 1 || objDesc->type != 0)
			continue;

		if (objDesc->noTick != 0)
			continue;

		if (objDesc->tick < objDesc->maxTick)
			objDesc->tick++;

		if (objDesc->tick >= objDesc->maxTick) {
			objDesc->tick = 1;
			objDesc->curFrame++;

			layer = objDesc->stateMach[objDesc->state][0]->layer;
			pLayer =
			    scen_animations[objDesc->animation].layers[layer];

			if (objDesc->curFrame < pLayer->framesCount)
				continue;

			objDesc->curFrame = 0;

			objDesc->xPos += pLayer->animDeltaX;
			objDesc->yPos += pLayer->animDeltaY;

			if (objDesc->nextState == -1
			    && objDesc->multState == -1
			    && objDesc->unk14 == 0) {
				objDesc->toRedraw = 0;
				objDesc->curFrame = pLayer->framesCount - 1;
			}

			if (objDesc->multState != -1) {
				if (objDesc->multState > 39) {
					objDesc->stateMach = gob_goblins[(int)(objDesc->multObjIndex)]->stateMach;
					objDesc->state = objDesc->multState - 40;
				} else {
					objDesc->stateMach = objDesc->realStateMach;
					objDesc->state = objDesc->multState;
				}
				objDesc->animation =
				    objDesc->stateMach[objDesc->state][0]->
				    animation;
				objDesc->multState = -1;
			} else {
				if (objDesc->nextState == -1)
					continue;

				objDesc->stateMach = objDesc->realStateMach;
				objDesc->state = objDesc->nextState;
				objDesc->animation =
				    objDesc->stateMach[objDesc->state][0]->
				    animation;
				objDesc->nextState = -1;
			}
			objDesc->toRedraw = 1;
		}
	}
}

void gob_placeObject(Gob_Object *objDesc, char animated) {
	int16 layer;

	if (objDesc->stateMach[objDesc->state][0] != 0) {
		objDesc->animation =
		    objDesc->stateMach[objDesc->state][0]->animation;

		objDesc->noTick = 0;
		objDesc->toRedraw = 1;
		objDesc->doAnim = animated;

		objDesc->maxTick = 1;
		objDesc->tick = 1;
		objDesc->curFrame = 0;
		objDesc->type = 0;
		objDesc->actionStartState = 0;
		objDesc->nextState = -1;
		objDesc->multState = -1;
		objDesc->stateColumn = 0;
		objDesc->curLookDir = 0;
		objDesc->visible = 1;
		objDesc->pickable = 0;
		objDesc->unk14 = 0;

		objDesc->relaxTime = util_getRandom(30);

		layer = objDesc->stateMach[objDesc->state][0]->layer;
		scen_updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->order = scen_toRedrawBottom / 24 + 3;

		objDesc->left = objDesc->xPos;
		objDesc->right = objDesc->xPos;
		objDesc->dirtyLeft = objDesc->xPos;
		objDesc->dirtyRight = objDesc->xPos;

		objDesc->top = objDesc->yPos;
		objDesc->bottom = objDesc->yPos;
		objDesc->dirtyTop = objDesc->yPos;
		objDesc->dirtyBottom = objDesc->yPos;

		util_listInsertBack(gob_objList, objDesc);
	}
}

int16 gob_getObjMaxFrame(Gob_Object * objDesc) {
	int16 layer;

	layer = objDesc->stateMach[objDesc->state][0]->layer;
	return scen_animations[objDesc->animation].layers[layer]->framesCount -
	    1;
}

int16 gob_objIntersected(Gob_Object *obj1, Gob_Object *obj2) {
	if (obj1->type == 1 || obj2->type == 1)
		return 0;

	if (obj1->right < obj2->left)
		return 0;

	if (obj1->left > obj2->right)
		return 0;

	if (obj1->bottom < obj2->top)
		return 0;

	if (obj1->top > obj2->bottom)
		return 0;

	return 1;
}

void gob_setMultStates(Gob_Object * gobDesc) {
	gobDesc->stateMach = gob_goblins[(int)gobDesc->multObjIndex]->stateMach;
}

int16 gob_nextLayer(Gob_Object *gobDesc) {
	if (gobDesc->nextState == 10)
		gobDesc->curLookDir = 0;

	if (gobDesc->nextState == 11)
		gobDesc->curLookDir = 4;

	if (gobDesc->nextState > 39) {
		gob_setMultStates(gobDesc);
	} else {
		gobDesc->stateMach = gobDesc->realStateMach;
	}

	gobDesc->curFrame = 0;
	if (gobDesc->nextState > 39)
		gobDesc->state = gobDesc->nextState - 40;
	else
		gobDesc->state = gobDesc->nextState;

	gobDesc->animation = gobDesc->stateMach[gobDesc->state][0]->animation;
	return gobDesc->stateMach[gobDesc->state][0]->layer;
}

void gob_showBoredom(int16 gobIndex) {
	Gob_Object *gobDesc;
	int16 frame;
	int16 frameCount;
	int16 layer;
	int16 state;
	int16 boreFlag;

	gobDesc = gob_goblins[gobIndex];
	layer = gobDesc->stateMach[gobDesc->state][0]->layer;

	frameCount =
	    scen_animations[gobDesc->animation].layers[layer]->framesCount;
	state = gobDesc->state;
	frame = gobDesc->curFrame;

	gobDesc->noTick = 0;
	gobDesc->toRedraw = 1;

	boreFlag = 1 << util_getRandom(7);

	if (gobIndex != gob_currentGoblin && util_getRandom(3) != 0) {
		if (state == 21) {
			if ((boreFlag & 16) || (boreFlag & 32)) {
				gobDesc->multState = 92 + gobIndex;
			} else if (boreFlag & 1) {
				gobDesc->multState = 86 + gobIndex;
			} else if (boreFlag & 2) {
				gobDesc->multState = 80 + gobIndex;
			} else if (boreFlag & 4) {
				gobDesc->multState = 89 + gobIndex;
			} else if (boreFlag & 8) {
				gobDesc->multState = 104 + gobIndex;
			}
		}
		gobDesc->nextState = 21;
	} else if (state >= 18 && state <= 21 && VAR(59) == 0) {
		if (state == 30 || state == 31)	// ???
			return;

		if (frame != frameCount)
			return;

		gobDesc->multState = 104 + gobIndex;
	}
}

// index - goblin to select+1
// index==0 - switch to next
void gob_switchGoblin(int16 index) {
	int16 next;
	int16 tmp;

	debug(0, "gob_switchGoblin");
	if (VAR(59) != 0)
		return;

	if (gob_goblins[gob_currentGoblin]->state <= 39 &&
	    gob_goblins[gob_currentGoblin]->curFrame != 0)
		return;

	if (index != 0 && gob_goblins[index - 1]->type != 0)
		return;

	if (index == 0)
		next = (gob_currentGoblin + 1) % 3;
	else
		next = index - 1;

	if (map_passMap[map_curGoblinY][map_curGoblinX] == 3 ||
	    map_passMap[map_curGoblinY][map_curGoblinX] == 6)
		return;

	if (gob_goblins[(gob_currentGoblin + 1) % 3]->type != 0 &&
	    gob_goblins[(gob_currentGoblin + 2) % 3]->type != 0)
		return;

	gob_gobPositions[gob_currentGoblin].x = map_curGoblinX;
	gob_gobPositions[gob_currentGoblin].y = map_curGoblinY;

	gob_goblins[gob_currentGoblin]->doAnim = 1;
	gob_goblins[gob_currentGoblin]->nextState = 21;

	gob_nextLayer(gob_goblins[gob_currentGoblin]);

	gob_currentGoblin = next;
	if (gob_goblins[gob_currentGoblin]->type != 0)
		gob_currentGoblin = (gob_currentGoblin + 1) % 3;

	gob_goblins[gob_currentGoblin]->doAnim = 0;
	if (gob_goblins[gob_currentGoblin]->curLookDir == 4)
		gob_goblins[gob_currentGoblin]->nextState = 18;
	else
		gob_goblins[gob_currentGoblin]->nextState = 19;

	gob_goblins[gob_currentGoblin]->toRedraw = 1;
	gob_nextLayer(gob_goblins[gob_currentGoblin]);

	tmp = gob_gobPositions[gob_currentGoblin].x;
	gob_pressedMapX = tmp;
	map_destX = tmp;
	gob_gobDestX = tmp;
	map_curGoblinX = tmp;

	tmp = gob_gobPositions[gob_currentGoblin].y;
	gob_pressedMapY = tmp;
	map_destY = tmp;
	gob_gobDestY = tmp;
	map_curGoblinY = tmp;

	*gob_curGobVarPtr = gob_currentGoblin;
	gob_pathExistence = 0;
	gob_readyToAct = 0;
}

void gob_adjustDest(int16 posX, int16 posY) {
	int16 resDelta;
	int16 resDeltaDir;
	int16 resDeltaPix;
	int16 deltaPix;
	int16 i;

	if (map_passMap[gob_pressedMapY][gob_pressedMapX] == 0 &&
	    (gob_gobAction == 0
		|| map_itemsMap[gob_pressedMapY][gob_pressedMapX] == 0)) {

		resDelta = -1;
		resDeltaDir = 0;
		resDeltaPix = 0;

		for (i = 1;
		    i <= gob_pressedMapX
		    && map_passMap[gob_pressedMapY][gob_pressedMapX - i] == 0;
		    i++);

		if (i <= gob_pressedMapX) {
			resDeltaPix = (i - 1) * 12 + (posX % 12) + 1;
			resDelta = i;
		}

		for (i = 1;
		    (i + gob_pressedMapX) < 26
		    && map_passMap[gob_pressedMapY][gob_pressedMapX + i] == 0;
		    i++);

		if (gob_pressedMapX + i < 26) {
			deltaPix = (i * 12) - (posX % 12);
			if (resDelta == -1 || deltaPix < resDeltaPix) {
				resDeltaPix = deltaPix;
				resDelta = i;
				resDeltaDir = 1;
			}
		}

		for (i = 1;
		    (i + gob_pressedMapY) < 28
		    && map_passMap[gob_pressedMapY + i][gob_pressedMapX] == 0;
		    i++);

		if (gob_pressedMapY + i < 28) {
			deltaPix = (i * 6) - (posY % 6);
			if (resDelta == -1 || deltaPix < resDeltaPix) {
				resDeltaPix = deltaPix;
				resDelta = i;
				resDeltaDir = 2;
			}
		}

		for (i = 1;
		    i <= gob_pressedMapY
		    && map_passMap[gob_pressedMapY - i][gob_pressedMapX] == 0;
		    i++);

		if (i <= gob_pressedMapY) {
			deltaPix = (i * 6) + (posY % 6);
			if (resDelta == -1 || deltaPix < resDeltaPix) {
				resDeltaPix = deltaPix;
				resDelta = i;
				resDeltaDir = 3;
			}
		}

		switch (resDeltaDir) {
		case 0:
			gob_pressedMapX -= resDelta;
			break;

		case 1:
			gob_pressedMapX += resDelta;
			break;

		case 2:
			gob_pressedMapY += resDelta;
			break;

		case 3:
			gob_pressedMapY -= resDelta;
			break;
		}

	}
}

void gob_adjustTarget(void) {
	if (gob_gobAction == 4
	    && map_itemsMap[gob_pressedMapY][gob_pressedMapX] == 0) {

		if (gob_pressedMapY > 0
		    && map_itemsMap[gob_pressedMapY - 1][gob_pressedMapX] !=
		    0) {
			gob_pressedMapY--;
		} else if (gob_pressedMapX < 25
		    && map_itemsMap[gob_pressedMapY][gob_pressedMapX + 1] !=
		    0) {
			gob_pressedMapX++;
		} else if (gob_pressedMapX < 25 && gob_pressedMapY > 0
		    && map_itemsMap[gob_pressedMapY - 1][gob_pressedMapX +
			1] != 0) {
			gob_pressedMapY--;
			gob_pressedMapX++;
		}
	}
}

void gob_targetDummyItem(Gob_Object *gobDesc) {
	if (map_itemsMap[gob_pressedMapY][gob_pressedMapX] == 0 &&
	    map_passMap[gob_pressedMapY][gob_pressedMapX] == 1) {
		if (gobDesc->curLookDir == 0) {
			map_itemPoses[0].x = gob_pressedMapX;
			map_itemPoses[0].y = gob_pressedMapY;
			map_itemPoses[0].orient = -4;
		} else {
			map_itemPoses[0].x = gob_pressedMapX;
			map_itemPoses[0].y = gob_pressedMapY;
			map_itemPoses[0].orient = -1;
		}
	}
}

void gob_targetItem(void) {
	int16 tmpX;
	int16 tmpY;
	int16 items;
	int16 layer;
	int16 tmpPosX;
	int16 tmpPosY;
	Gob_Object *itemDesc;

	if (gob_gobAction == 3 || gob_gobAction == 4) {
		items = map_itemsMap[gob_pressedMapY][gob_pressedMapX];
		if (gob_gobAction == 4 && (items & 0xff00) != 0 &&
		    gob_objects[gob_itemToObject[(items & 0xff00) >> 8]]->
		    pickable == 1) {
			gob_destItemId = (items & 0xff00) >> 8;
			gob_destActionItem = (items & 0xff00) >> 8;
			gob_itemByteFlag = 1;
		} else if ((items & 0xff) == 0) {
			gob_destItemId = (items & 0xff00) >> 8;
			gob_destActionItem = (items & 0xff00) >> 8;
			gob_itemByteFlag = 1;
		} else if (gob_gobAction == 3 && gob_currentGoblin == 2 &&
		    (items & 0xff00) != 0) {
			gob_destItemId = (items & 0xff00) >> 8;
			gob_destActionItem = (items & 0xff00) >> 8;
			gob_itemByteFlag = 1;
		} else {
			gob_destItemId = items & 0xff;
			gob_destActionItem = items & 0xff;
			gob_itemByteFlag = 0;
		}

		gob_pressedMapY = map_itemPoses[gob_destItemId].y;
		map_destY = map_itemPoses[gob_destItemId].y;
		gob_gobDestY = map_itemPoses[gob_destItemId].y;

		if (gob_gobAction == 3 || gob_destActionItem == 0) {
			gob_pressedMapX = map_itemPoses[gob_destItemId].x;
			map_destX = map_itemPoses[gob_destItemId].x;
			gob_gobDestX = map_itemPoses[gob_destItemId].x;
		} else if ((items & 0xff00) != 0) {
			if (map_itemPoses[gob_destItemId].orient == 4) {
				if ((map_itemsMap[gob_pressedMapY]
					[gob_pressedMapX - 1] & 0xff00) ==
				    (map_itemsMap[gob_pressedMapY]
					[gob_pressedMapX] & 0xff00)) {
					gob_pressedMapX--;
					map_destX = gob_pressedMapX;
					gob_gobDestX = gob_pressedMapX;
				}
			} else if (map_itemPoses[gob_destItemId].orient == 0) {

				if ((map_itemsMap[gob_pressedMapY]
					[gob_pressedMapX + 1] & 0xff00) ==
				    (map_itemsMap[gob_pressedMapY]
					[gob_pressedMapX] & 0xff00)) {
					gob_pressedMapX++;
					map_destX = gob_pressedMapX;
					gob_gobDestX = gob_pressedMapX;
				}
			}

			if ((map_itemsMap[gob_pressedMapY +
				    1][gob_pressedMapX] & 0xff00) ==
			    (map_itemsMap[gob_pressedMapY][gob_pressedMapX] &
				0xff00)) {
				gob_pressedMapY++;
				map_destY = gob_pressedMapY;
				gob_gobDestY = gob_pressedMapY;
			}
		} else {
			if (map_itemPoses[gob_destItemId].orient == 4) {
				if ((map_itemsMap[gob_pressedMapY]
					[gob_pressedMapX - 1]) ==
				    (map_itemsMap[gob_pressedMapY]
					[gob_pressedMapX])) {
					gob_pressedMapX--;
					map_destX = gob_pressedMapX;
					gob_gobDestX = gob_pressedMapX;
				}
			} else if (map_itemPoses[gob_destItemId].orient == 0) {

				if ((map_itemsMap[gob_pressedMapY]
					[gob_pressedMapX + 1]) ==
				    (map_itemsMap[gob_pressedMapY]
					[gob_pressedMapX])) {
					gob_pressedMapX++;
					map_destX = gob_pressedMapX;
					gob_gobDestX = gob_pressedMapX;
				}
			}

			if ((map_itemsMap[gob_pressedMapY +
				    1][gob_pressedMapX]) ==
			    (map_itemsMap[gob_pressedMapY][gob_pressedMapX])) {
				gob_pressedMapY++;
				map_destY = gob_pressedMapY;
				gob_gobDestY = gob_pressedMapY;
			}

		}

		if (gob_gobAction == 4 && gob_destActionItem != 0 &&
			gob_itemToObject[gob_destActionItem] != -1 &&
		    gob_objects[gob_itemToObject[gob_destActionItem]]->
		    pickable == 1) {

			itemDesc =
			    gob_objects[gob_itemToObject[gob_destActionItem]];

			itemDesc->animation =
			    itemDesc->stateMach[itemDesc->state][0]->animation;
			layer =
			    itemDesc->stateMach[itemDesc->state][itemDesc->
			    stateColumn]->layer;

			scen_updateAnim(layer, 0, itemDesc->animation, 0,
			    itemDesc->xPos, itemDesc->yPos, 0);

			tmpX = (scen_toRedrawRight + scen_toRedrawLeft) / 2;
			tmpY = scen_toRedrawBottom;

			tmpPosY = tmpY / 6;
			if ((tmpY % 3) < 3 && tmpPosY > 0)
				tmpPosY--;

			tmpPosX = tmpX / 12;
			if ((tmpX % 12) < 6 && tmpPosX > 0)
				tmpPosX--;

			if (map_itemPoses[gob_destActionItem].orient == 0 ||
			    map_itemPoses[gob_destActionItem].orient == -1) {
				tmpPosX++;
			}

			if (map_passMap[tmpPosY][tmpPosX] == 1) {
				gob_pressedMapX = tmpPosX;
				map_destX = tmpPosX;
				gob_gobDestX = tmpPosX;

				gob_pressedMapY = tmpPosY;
				map_destY = tmpPosY;
				gob_gobDestY = tmpPosY;
			}
		}
	}
}

void gob_initiateMove(void) {
	map_findNearestToDest();
	map_findNearestToGob();

	map_nearestWayPoint =
	    map_optimizePoints(map_curGoblinX, map_curGoblinY);

	gob_pathExistence = map_checkDirectPath(map_curGoblinX, map_curGoblinY,
	    gob_pressedMapX, gob_pressedMapY);

	if (gob_pathExistence == 3) {
		if (map_checkLongPath(map_curGoblinX, map_curGoblinY,
			gob_pressedMapX, gob_pressedMapY,
			map_nearestWayPoint, map_nearestDest) == 0) {
			gob_pathExistence = 0;
		} else {
			map_destX = map_wayPoints[map_nearestWayPoint].x;
			map_destY = map_wayPoints[map_nearestWayPoint].y;
		}
	}
}

void gob_moveFindItem(int16 posX, int16 posY) {
	int16 i;
	if (gob_gobAction == 3 || gob_gobAction == 4) {
		for (i = 0; i < 20; i++) {
			if (gob_objects[i] == 0)
				continue;

			if (gob_objects[i]->type != 0)
				continue;

			if (gob_objects[i]->left > posX)
				continue;

			if (gob_objects[i]->right < posX)
				continue;

			if (gob_objects[i]->top > posY)
				continue;

			if (gob_objects[i]->bottom < posY)
				continue;

			if (gob_objects[i]->right - gob_objects[i]->left < 40)
				posX =
				    (gob_objects[i]->left +
				    gob_objects[i]->right) / 2;

			if (gob_objects[i]->bottom - gob_objects[i]->top < 40)
				posY =
				    (gob_objects[i]->top +
				    gob_objects[i]->bottom) / 2;

			break;
		}

		gob_pressedMapX = posX / 12;
		gob_pressedMapY = posY / 6;

		if (map_itemsMap[gob_pressedMapY][gob_pressedMapX] == 0
		    && i < 20) {

			if (map_itemsMap[gob_pressedMapY +
				1][gob_pressedMapX] != 0) {
				gob_pressedMapY++;
			} else if (map_itemsMap[gob_pressedMapY +
				1][gob_pressedMapX + 1] != 0) {
				gob_pressedMapX++;
				gob_pressedMapY++;
			} else
			    if (map_itemsMap[gob_pressedMapY][gob_pressedMapX +
				1] != 0) {
				gob_pressedMapX++;
			} else if (map_itemsMap[gob_pressedMapY -
				1][gob_pressedMapX + 1] != 0) {
				gob_pressedMapX++;
				gob_pressedMapY--;
			} else if (map_itemsMap[gob_pressedMapY -
				1][gob_pressedMapX] != 0) {
				gob_pressedMapY--;
			} else if (map_itemsMap[gob_pressedMapY -
				1][gob_pressedMapX - 1] != 0) {
				gob_pressedMapY--;
				gob_pressedMapX--;
			} else
			    if (map_itemsMap[gob_pressedMapY][gob_pressedMapX -
				1] != 0) {
				gob_pressedMapX--;
			} else if (map_itemsMap[gob_pressedMapY +
				1][gob_pressedMapX - 1] != 0) {
				gob_pressedMapX--;
				gob_pressedMapY++;
			}
		}
	} else {
		gob_pressedMapX = posX / 12;
		gob_pressedMapY = posY / 6;
	}
}

void gob_moveCheckSelect(int16 framesCount, Gob_Object * gobDesc, int16 *pGobIndex,
	    int16 *nextAct) {
	if (gobDesc->right > inter_mouseX &&
	    gobDesc->left < inter_mouseX &&
	    gobDesc->bottom > inter_mouseY &&
	    gobDesc->bottom - 10 < inter_mouseY && gob_gobAction == 0) {
		if (gobDesc->curLookDir & 4)
			*nextAct = 16;
		else
			*nextAct = 23;

		gobDesc->curFrame = framesCount - 1;
		gob_pathExistence = 0;
	} else {
		*pGobIndex = gob_peekGoblin(gobDesc);

		if (*pGobIndex != 0) {
			gob_pathExistence = 0;
		} else if (map_curGoblinX == gob_pressedMapX &&
		    map_curGoblinY == gob_pressedMapY) {

			if (gob_gobAction != 0)
				gob_readyToAct = 1;

			gob_pathExistence = 0;
		}
	}
}

void gob_moveInitStep(int16 framesCount, int16 action, int16 cont,
	    Gob_Object *gobDesc, int16 *pGobIndex, int16 *pNextAct) {
	int16 posX;
	int16 posY;

	if (cont != 0 && gob_goesAtTarget == 0 &&
	    gob_readyToAct == 0 && VAR(59) == 0 &&
	    gobDesc->type != 1 &&
	    gobDesc->state != 10 && gobDesc->state != 11) {
		if (gobDesc->state >= 40) {
			gobDesc->curFrame = framesCount - 1;
		}

		gob_gobAction = action;
		gob_forceNextState[0] = -1;
		gob_forceNextState[1] = -1;
		gob_forceNextState[2] = -1;

		if (action == 3) {
			posX = inter_mouseX + 6;
			posY = inter_mouseY + 7;
		} else if (action == 4) {
			posX = inter_mouseX + 7;
			posY = inter_mouseY + 12;
		} else {
			posX = inter_mouseX;
			posY = inter_mouseY;
		}

		gob_moveFindItem(posX, posY);
		gob_adjustDest(posX, posY);
		gob_adjustTarget();

		map_destX = gob_pressedMapX;
		gob_gobDestX = gob_pressedMapX;

		map_destY = gob_pressedMapY;
		gob_gobDestY = gob_pressedMapY;

		gob_targetDummyItem(gobDesc);

		gob_targetItem();
		gob_initiateMove();

		gob_moveCheckSelect(framesCount, gobDesc, pGobIndex, pNextAct);
	} else {

		if (gob_readyToAct != 0 &&
		    (map_curGoblinX != gob_pressedMapX ||
			map_curGoblinY != gob_pressedMapY))
			gob_readyToAct = 0;

		if (gobDesc->type == 1) {
			*pGobIndex = gob_peekGoblin(gobDesc);
		}
	}
}

void gob_moveTreatRopeStairs(Gob_Object *gobDesc) {
	if (gob_currentGoblin != 1)
		return;

	if (gobDesc->nextState == 28
	    && map_passMap[map_curGoblinY - 1][map_curGoblinX] == 6) {
		gob_forceNextState[0] = 28;
		gob_forceNextState[1] = -1;
	}

	if (gobDesc->nextState == 29
	    && map_passMap[map_curGoblinY + 1][map_curGoblinX] == 6) {
		gob_forceNextState[0] = 29;
		gob_forceNextState[1] = -1;
	}

	if ((gobDesc->nextState == 28 || gobDesc->nextState == 29
		|| gobDesc->nextState == 20)
	    && map_passMap[map_curGoblinY][map_curGoblinX] == 6) {
		if ((gobDesc->curLookDir == 0 || gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 2)
		    && map_passMap[map_curGoblinY - 1][map_curGoblinX] == 6) {
			gob_forceNextState[0] = 28;
			gob_forceNextState[1] = -1;
		} else if ((gobDesc->curLookDir == 0
			|| gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 6)
		    && map_passMap[map_curGoblinY + 1][map_curGoblinX] == 6) {
			gob_forceNextState[0] = 29;
			gob_forceNextState[1] = -1;
		}
	}

	if (gobDesc->nextState == 8
	    && map_passMap[map_curGoblinY - 1][map_curGoblinX] == 3) {
		gob_forceNextState[0] = 8;
		gob_forceNextState[1] = -1;
	}

	if (gobDesc->nextState == 9
	    && map_passMap[map_curGoblinY + 1][map_curGoblinX] == 3) {
		gob_forceNextState[0] = 9;
		gob_forceNextState[1] = -1;
	}

	if (gobDesc->nextState == 20
	    && map_passMap[map_curGoblinY][map_curGoblinX] == 3) {
		if ((gobDesc->curLookDir == 0 || gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 2)
		    && map_passMap[map_curGoblinY - 1][map_curGoblinX] == 3) {
			gob_forceNextState[0] = 8;
			gob_forceNextState[1] = -1;
		} else if ((gobDesc->curLookDir == 0
			|| gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 6)
		    && map_passMap[map_curGoblinY + 1][map_curGoblinX] == 3) {
			gob_forceNextState[0] = 9;
			gob_forceNextState[1] = -1;
		}
	}

}

void gob_movePathFind(Gob_Object *gobDesc, int16 nextAct) {
	if (gob_pathExistence == 1) {
		map_curGoblinX = gob_gobPositions[gob_currentGoblin].x;
		map_curGoblinY = gob_gobPositions[gob_currentGoblin].y;

		if (map_curGoblinX == gob_pressedMapX &&
		    map_curGoblinY == gob_pressedMapY && gob_gobAction != 0) {
			gob_readyToAct = 1;
			gob_pathExistence = 0;
		}

		nextAct = map_getDirection(map_curGoblinX, map_curGoblinY,
		    map_destX, map_destY);

		if (nextAct == 0)
			gob_pathExistence = 0;
	} else if (gob_pathExistence == 3) {
		map_curGoblinX = gob_gobPositions[gob_currentGoblin].x;
		map_curGoblinY = gob_gobPositions[gob_currentGoblin].y;

		if (map_curGoblinX == gob_gobDestX &&
		    map_curGoblinY == gob_gobDestY) {
			gob_pathExistence = 1;
			map_destX = gob_pressedMapX;
			map_destY = gob_pressedMapY;
		} else {

			if (map_checkDirectPath(map_curGoblinX, map_curGoblinY,
				gob_gobDestX, gob_gobDestY) == 1) {
				map_destX = gob_gobDestX;
				map_destY = gob_gobDestY;
			} else if (map_curGoblinX == map_destX
			    && map_curGoblinY == map_destY) {

				if (map_nearestWayPoint > map_nearestDest) {
					map_nearestWayPoint =
					    map_optimizePoints(map_curGoblinX,
					    map_curGoblinY);

					map_destX =
					    map_wayPoints[map_nearestWayPoint].
					    x;
					map_destY =
					    map_wayPoints[map_nearestWayPoint].
					    y;

					if (map_nearestWayPoint >
					    map_nearestDest)
						map_nearestWayPoint--;
				} else if (map_nearestWayPoint <
				    map_nearestDest) {
					map_nearestWayPoint =
					    map_optimizePoints(map_curGoblinX,
					    map_curGoblinY);

					map_destX =
					    map_wayPoints[map_nearestWayPoint].
					    x;
					map_destY =
					    map_wayPoints[map_nearestWayPoint].
					    y;

					if (map_nearestWayPoint < map_nearestDest)
						map_nearestWayPoint++;
				} else {
					if (map_checkDirectPath(map_curGoblinX,
						map_curGoblinY, gob_gobDestX,
						gob_gobDestY) == 3 && map_passMap[gob_pressedMapY][gob_pressedMapX] != 0) {
						map_destX = map_wayPoints[map_nearestWayPoint].x;
						map_destY = map_wayPoints[map_nearestWayPoint].y;
					} else {
						gob_pathExistence = 1;
						map_destX = gob_pressedMapX;
						map_destY = gob_pressedMapY;
					}
				}
			}
			nextAct =
			    map_getDirection(map_curGoblinX, map_curGoblinY,
			    map_destX, map_destY);
		}
	}

	if (gob_readyToAct != 0 && (gob_gobAction == 3 || gob_gobAction == 4))
		nextAct = 0x4dc8;

	switch (nextAct) {
	case 0x4b00:
		gobDesc->nextState = gob_rotateState(gobDesc->curLookDir, 0);
		break;

	case 0x4d00:
		gobDesc->nextState = gob_rotateState(gobDesc->curLookDir, 4);
		break;

	case 16:
		gobDesc->nextState = 16;
		break;

	case 23:
		gobDesc->nextState = 23;
		break;

	case 0x4800:

		if (map_passMap[map_curGoblinY - 1][map_curGoblinX] == 6 &&
		    gob_currentGoblin != 1) {
			gob_pathExistence = 0;
			break;
		}

		if (map_passMap[map_curGoblinY][map_curGoblinX] == 3) {
			gobDesc->nextState = 8;
			break;
		}

		if (map_passMap[map_curGoblinY][map_curGoblinX] == 6 &&
		    gob_currentGoblin == 1) {
			gobDesc->nextState = 28;
			break;
		}

		gobDesc->nextState = gob_rotateState(gobDesc->curLookDir, 2);
		break;

	case 0x5000:
		if (map_passMap[map_curGoblinY + 1][map_curGoblinX] == 6 &&
		    gob_currentGoblin != 1) {
			gob_pathExistence = 0;
			break;
		}

		if (map_passMap[map_curGoblinY][map_curGoblinX] == 3) {
			gobDesc->nextState = 9;
			break;
		}

		if (map_passMap[map_curGoblinY][map_curGoblinX] == 6 &&
		    gob_currentGoblin == 1) {
			gobDesc->nextState = 29;
			break;
		}

		gobDesc->nextState = gob_rotateState(gobDesc->curLookDir, 6);
		break;

	case 0x5100:
		if (map_passMap[map_curGoblinY + 1][map_curGoblinX + 1] == 6 &&
		    gob_currentGoblin != 1) {
			gob_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 5;
		if (gobDesc->curLookDir == 4)
			break;

		gobDesc->nextState = gob_rotateState(gobDesc->curLookDir, 4);
		break;

	case 0x4f00:
		if (map_passMap[map_curGoblinY + 1][map_curGoblinX - 1] == 6 &&
		    gob_currentGoblin != 1) {
			gob_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 7;
		if (gobDesc->curLookDir == 0)
			break;

		gobDesc->nextState = gob_rotateState(gobDesc->curLookDir, 0);
		break;

	case 0x4700:
		if (map_passMap[map_curGoblinY - 1][map_curGoblinX - 1] == 6 &&
		    gob_currentGoblin != 1) {
			gob_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 1;
		if (gobDesc->curLookDir == 0)
			break;

		gobDesc->nextState = gob_rotateState(gobDesc->curLookDir, 0);
		break;

	case 0x4900:
		if (map_passMap[map_curGoblinY - 1][map_curGoblinX + 1] == 6 &&
		    gob_currentGoblin != 1) {
			gob_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 3;
		if (gobDesc->curLookDir == 4)
			break;

		gobDesc->nextState = gob_rotateState(gobDesc->curLookDir, 4);
		break;

	case 0x4dc8:

		if (gob_currentGoblin == 0 && gob_gobAction == 3
		    && gob_itemIndInPocket == -1) {
			gob_destItemId = -1;
			gob_readyToAct = 0;
			break;
		}

		if (gob_currentGoblin == 0 && gob_gobAction == 4 &&
		    gob_itemIndInPocket == -1 && gob_destActionItem == 0) {
			gobDesc->multState = 104;
			gob_destItemId = -1;
			gob_readyToAct = 0;
			break;
		}

		if (gob_currentGoblin == 0 && gob_gobAction == 4 &&
		    gob_itemIndInPocket == -1 && gob_destActionItem != 0 &&
		    gob_itemToObject[gob_destActionItem] != -1 &&
		    gob_objects[gob_itemToObject[gob_destActionItem]]->
		    pickable == 0) {
			gobDesc->multState = 104;
			gob_destItemId = -1;
			gob_readyToAct = 0;
			break;
		}

		switch (map_itemPoses[gob_destActionItem].orient) {
		case 0:
		case -4:
			gobDesc->nextState = 10;
			gobDesc->curLookDir = 0;
			gob_destItemId = -1;
			break;

		case -1:
		case 4:
			gobDesc->nextState = 11;
			gobDesc->curLookDir = 4;
			gob_destItemId = -1;
			break;
		}
		break;

	default:
		if (map_passMap[map_curGoblinY][map_curGoblinX] == 3 ||
		    (map_passMap[map_curGoblinY][map_curGoblinX] == 6
			&& gob_currentGoblin == 1)) {
			gobDesc->nextState = 20;
			break;
		}

		switch (gobDesc->curLookDir) {
		case 2:
		case 4:
			gobDesc->nextState = 18;
			break;

		case 6:
		case 0:
			gobDesc->nextState = 19;
			break;
		}
		break;
	}
	return;
}

void gob_moveAdvance(Gob_Object *gobDesc, int16 nextAct, int16 framesCount) {
	int16 i;
	int16 newX;
	int16 newY;
	int16 flag;

	gob_movePathFind(gobDesc, nextAct);

	gobDesc->curFrame++;
	if (gobDesc->curFrame == 1)
		gobDesc->actionStartState = gobDesc->state;

	if (gob_goesAtTarget == 0
	    && gobDesc->stateMach == gobDesc->realStateMach) {
		switch (gobDesc->state) {
		case 0:
		case 1:
		case 7:
		case 13:
		case 16:
		case 27:
			gobDesc->curLookDir = 0;
			break;

		case 3:
		case 4:
		case 5:
		case 12:
		case 23:
		case 26:
			gobDesc->curLookDir = 4;
			break;

		case 28:
			if (gob_currentGoblin != 1)
				break;
			gobDesc->curLookDir = 2;
			break;

		case 2:
		case 8:
		case 15:
		case 22:
		case 25:
			gobDesc->curLookDir = 2;
			break;

		case 29:
			if (gob_currentGoblin != 1)
				break;

			gobDesc->curLookDir = 6;
			break;

		case 6:
		case 9:
		case 14:
		case 17:
		case 24:
			gobDesc->curLookDir = 6;
			break;
		}
	}

	if (gobDesc->state >= 0 && gobDesc->state < 10 &&
	    gobDesc->stateMach == gobDesc->realStateMach &&
	    (gobDesc->curFrame == 3 || gobDesc->curFrame == 6)) {
		snd_speakerOn(10 * util_getRandom(3) + 50, 5);
	}

	if (gob_currentGoblin == 0
	    && gobDesc->stateMach == gobDesc->realStateMach
	    && (gobDesc->state == 10 || gobDesc->state == 11)
	    && gobDesc->curFrame == 9) {
		snd_stopSound(0);
		if (gob_itemIndInPocket != -1) {
			snd_playSample(gob_soundData[14], 1, 9000);
		}

		if (gob_itemIndInPocket == -1) {
			snd_playSample(gob_soundData[14], 1, 5000);
		}
	}

	if (gob_boreCounter++ == 120) {
		gob_boreCounter = 0;
		for (i = 0; i < 3; i++)
			gob_showBoredom(i);
	}

	if (gobDesc->multState != -1 && gobDesc->curFrame == framesCount &&
	    gobDesc->state != gobDesc->multState) {
		gobDesc->nextState = gobDesc->multState;
		gobDesc->multState = -1;

		newX =
		    scen_animations[gobDesc->animation].
		    layers[gob_gobStateLayer]->animDeltaX + gobDesc->xPos;

		newY =
		    scen_animations[gobDesc->animation].
		    layers[gob_gobStateLayer]->animDeltaY + gobDesc->yPos;

		gob_gobStateLayer = gob_nextLayer(gobDesc);

		gobDesc->xPos = newX;
		gobDesc->yPos = newY;
	} else {
		if (gobDesc->curFrame == 3 &&
		    gobDesc->stateMach == gobDesc->realStateMach &&
		    (gobDesc->state < 10 ||
			(gob_currentGoblin == 1 && (gobDesc->state == 28
				|| gobDesc->state == 29))
		    )) {
			flag = 0;
			if (gob_forceNextState[0] != -1) {
				gobDesc->nextState = gob_forceNextState[0];
				for (i = 0; i < 9; i++)
					gob_forceNextState[i] =
					    gob_forceNextState[i + 1];
			}

			map_curGoblinX = gob_gobPositions[gob_currentGoblin].x;
			map_curGoblinY = gob_gobPositions[gob_currentGoblin].y;

			if (gobDesc->nextState != gobDesc->state) {
				gob_gobStateLayer = gob_nextLayer(gobDesc);
				flag = 1;
			}

			switch (gobDesc->state) {
			case 0:
				gob_gobPositions[gob_currentGoblin].x--;
				break;

			case 2:
			case 8:
				gob_gobPositions[gob_currentGoblin].y--;
				break;

			case 4:
				gob_gobPositions[gob_currentGoblin].x++;
				break;

			case 6:
			case 9:
				gob_gobPositions[gob_currentGoblin].y++;
				break;

			case 1:
				gob_gobPositions[gob_currentGoblin].x--;
				gob_gobPositions[gob_currentGoblin].y--;
				break;

			case 3:
				gob_gobPositions[gob_currentGoblin].x++;
				gob_gobPositions[gob_currentGoblin].y--;
				break;

			case 5:
				gob_gobPositions[gob_currentGoblin].x++;
				gob_gobPositions[gob_currentGoblin].y++;
				break;

			case 7:
				gob_gobPositions[gob_currentGoblin].x--;
				gob_gobPositions[gob_currentGoblin].y++;
				break;

			case 38:
				gob_gobPositions[gob_currentGoblin].y++;
				break;
			}

			if (gob_currentGoblin == 1) {
				if (gobDesc->state == 28)
					gob_gobPositions[1].y--;

				if (gobDesc->state == 29)
					gob_gobPositions[1].y++;
			}

			if (flag != 0) {
				scen_updateAnim(gob_gobStateLayer, 0,
				    gobDesc->animation, 0, gobDesc->xPos,
				    gobDesc->yPos, 0);

				gobDesc->yPos =
				    (map_curGoblinY + 1) * 6 -
				    (scen_toRedrawBottom - scen_animTop);
				gobDesc->xPos =
				    map_curGoblinX * 12 - (scen_toRedrawLeft -
				    scen_animLeft);
			}

			if ((gobDesc->state == 10 || gobDesc->state == 11)
			    && gob_currentGoblin != 0)
				gob_goesAtTarget = 1;
		}

		if (gobDesc->curFrame != framesCount)
			return;

		if (gob_forceNextState[0] != -1) {
			gobDesc->nextState = gob_forceNextState[0];
			for (i = 0; i < 10; i++)
				gob_forceNextState[i] =
				    gob_forceNextState[i + 1];
		}

		map_curGoblinX = gob_gobPositions[gob_currentGoblin].x;
		map_curGoblinY = gob_gobPositions[gob_currentGoblin].y;

		gob_gobStateLayer = gob_nextLayer(gobDesc);
		if (gobDesc->stateMach == gobDesc->realStateMach) {

			switch (gobDesc->nextState) {
			case 0:
				gob_gobPositions[gob_currentGoblin].x--;
				break;

			case 2:
			case 8:
				gob_gobPositions[gob_currentGoblin].y--;
				break;

			case 4:
				gob_gobPositions[gob_currentGoblin].x++;
				break;

			case 6:
			case 9:
				gob_gobPositions[gob_currentGoblin].y++;
				break;

			case 1:
				gob_gobPositions[gob_currentGoblin].x--;
				gob_gobPositions[gob_currentGoblin].y--;
				break;

			case 3:
				gob_gobPositions[gob_currentGoblin].x++;
				gob_gobPositions[gob_currentGoblin].y--;
				break;

			case 5:
				gob_gobPositions[gob_currentGoblin].x++;
				gob_gobPositions[gob_currentGoblin].y++;
				break;

			case 7:
				gob_gobPositions[gob_currentGoblin].x--;
				gob_gobPositions[gob_currentGoblin].y++;
				break;

			case 38:
				gob_gobPositions[gob_currentGoblin].y++;
				break;
			}
			if (gob_currentGoblin == 1) {
				if (gobDesc->nextState == 28)
					gob_gobPositions[1].y--;

				if (gobDesc->nextState == 29)
					gob_gobPositions[1].y++;
			}
		}

		scen_updateAnim(gob_gobStateLayer, 0, gobDesc->animation, 0,
		    gobDesc->xPos, gobDesc->yPos, 0);

		gobDesc->yPos =
		    (map_curGoblinY + 1) * 6 - (scen_toRedrawBottom -
		    scen_animTop);
		gobDesc->xPos =
		    map_curGoblinX * 12 - (scen_toRedrawLeft - scen_animLeft);

		if ((gobDesc->state == 10 || gobDesc->state == 11)
		    && gob_currentGoblin != 0)
			gob_goesAtTarget = 1;
	}
	return;
}

int16 gob_doMove(Gob_Object *gobDesc, int16 cont, int16 action) {
	int16 framesCount;
	int16 nextAct;
	int16 gobIndex;
	int16 layer;

	nextAct = 0;
	gobIndex = 0;

	layer = gobDesc->stateMach[gobDesc->state][0]->layer;
	framesCount =
	    scen_animations[gobDesc->animation].layers[layer]->framesCount;

	if (VAR(59) == 0 &&
	    gobDesc->state != 30 && gobDesc->state != 31) {
		gobDesc->order = (gobDesc->bottom) / 24 + 3;
	}

	if (gob_positionedGob != gob_currentGoblin) {
		map_curGoblinX = gob_gobPositions[gob_currentGoblin].x;
		map_curGoblinY = gob_gobPositions[gob_currentGoblin].y;
	}

	gob_positionedGob = gob_currentGoblin;

	gobDesc->animation =
	    gobDesc->stateMach[gobDesc->state][gobDesc->stateColumn]->
	    animation;

	gob_gobStateLayer =
	    gobDesc->stateMach[gobDesc->state][gobDesc->stateColumn]->layer;

	gob_moveInitStep(framesCount, action, cont, gobDesc, &gobIndex,
	    &nextAct);
	gob_moveTreatRopeStairs(gobDesc);
	gob_moveAdvance(gobDesc, nextAct, framesCount);

	return gobIndex;
}

void gob_freeObjects(void) {
	int16 i;
	int16 state;
	int16 col;

	for (i = 0; i < 16; i++) {
		if (gob_soundData[i] == 0)
			continue;

		snd_freeSoundData(gob_soundData[i]);
		gob_soundData[i] = 0;
	}

	for (i = 0; i < 4; i++) {
		if (gob_goblins[i] == 0)
			continue;

		gob_goblins[i]->stateMach = gob_goblins[i]->realStateMach;

		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				free(gob_goblins[i]->stateMach[state][col]);
				gob_goblins[i]->stateMach[state][col] = 0;
			}
		}

		if (i == 3) {
			for (state = 40; state < 70; state++) {
				free(gob_goblins[3]->stateMach[state][0]);
				gob_goblins[3]->stateMach[state][0] = 0;
			}
		}

		free(gob_goblins[i]->stateMach);
		free(gob_goblins[i]);
		gob_goblins[i] = 0;
	}

	for (i = 0; i < 20; i++) {
		if (gob_objects[i] == 0)
			continue;

		gob_objects[i]->stateMach = gob_objects[i]->realStateMach;

		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				free(gob_objects[i]->stateMach[state][col]);
				gob_objects[i]->stateMach[state][col] = 0;
			}
		}

		free(gob_objects[i]->stateMach);
		free(gob_objects[i]);
		gob_objects[i] = 0;
	}
}

void gob_zeroObjects(void) {
	int16 i;

	for (i = 0; i < 4; i++)
		gob_goblins[i] = 0;

	for (i = 0; i < 20; i++)
		gob_objects[i] = 0;

	for (i = 0; i < 16; i++)
		gob_soundData[i] = 0;
}

void gob_freeAllObjects(void) {
	util_deleteList(gob_objList);
	gob_freeObjects();
}

void gob_loadObjects(char *source) {
	int16 i;

	gob_zeroObjects();
	for (i = 0; i < 20; i++)
		gob_itemToObject[i] = 100;

	gob_freeObjects();
	gob_initList();
	strcpy(map_sourceFile, source);

	map_sourceFile[strlen(map_sourceFile) - 4] = 0;
	map_loadMapObjects(source);

	for (i = 0; i < gob_gobsCount; i++)
		gob_placeObject(gob_goblins[i], 0);

	for (i = 0; i < gob_objCount; i++) {
		gob_placeObject(gob_objects[i], 1);
	}

	gob_initVarPointers();
	gob_actDestItemDesc = 0;
}

void gob_saveGobDataToVars(int16 xPos, int16 yPos, int16 someVal) {
	Gob_Object *obj;
	*gob_some0ValPtr = someVal;
	*gob_curGobXPosVarPtr = xPos;
	*gob_curGobYPosVarPtr = yPos;
	*gob_itemInPocketVarPtr = gob_itemIndInPocket;

	obj = gob_goblins[gob_currentGoblin];

	*gob_curGobStateVarPtr = obj->state;
	*gob_curGobFrameVarPtr = obj->curFrame;
	*gob_curGobMultStateVarPtr = obj->multState;
	*gob_curGobNextStateVarPtr = obj->nextState;
	*gob_curGobScrXVarPtr = obj->xPos;
	*gob_curGobScrYVarPtr = obj->yPos;
	*gob_curGobLeftVarPtr = obj->left;
	*gob_curGobTopVarPtr = obj->top;
	*gob_curGobRightVarPtr = obj->right;
	*gob_curGobBottomVarPtr = obj->bottom;
	*gob_curGobDoAnimVarPtr = obj->doAnim;
	*gob_curGobOrderVarPtr = obj->order;
	*gob_curGobNoTickVarPtr = obj->noTick;
	*gob_curGobTypeVarPtr = obj->type;
	*gob_curGobMaxTickVarPtr = obj->maxTick;
	*gob_curGobTickVarPtr = obj->tick;
	*gob_curGobActStartStateVarPtr = obj->actionStartState;
	*gob_curGobLookDirVarPtr = obj->curLookDir;
	*gob_curGobPickableVarPtr = obj->pickable;
	*gob_curGobRelaxVarPtr = obj->relaxTime;
	*gob_curGobMaxFrameVarPtr = gob_getObjMaxFrame(obj);

	if (gob_actDestItemDesc == 0)
		return;

	obj = gob_actDestItemDesc;
	*gob_destItemStateVarPtr = obj->state;
	*gob_destItemFrameVarPtr = obj->curFrame;
	*gob_destItemMultStateVarPtr = obj->multState;
	*gob_destItemNextStateVarPtr = obj->nextState;
	*gob_destItemScrXVarPtr = obj->xPos;
	*gob_destItemScrYVarPtr = obj->yPos;
	*gob_destItemLeftVarPtr = obj->left;
	*gob_destItemTopVarPtr = obj->top;
	*gob_destItemRightVarPtr = obj->right;
	*gob_destItemBottomVarPtr = obj->bottom;
	*gob_destItemDoAnimVarPtr = obj->doAnim;
	*gob_destItemOrderVarPtr = obj->order;
	*gob_destItemNoTickVarPtr = obj->noTick;
	*gob_destItemTypeVarPtr = obj->type;
	*gob_destItemMaxTickVarPtr = obj->maxTick;
	*gob_destItemTickVarPtr = obj->tick;
	*gob_destItemActStartStVarPtr = obj->actionStartState;
	*gob_destItemLookDirVarPtr = obj->curLookDir;
	*gob_destItemPickableVarPtr = obj->pickable;
	*gob_destItemRelaxVarPtr = obj->relaxTime;
	*gob_destItemMaxFrameVarPtr = gob_getObjMaxFrame(obj);

	gob_destItemState = obj->state;
	gob_destItemType = obj->type;
}

void gob_initVarPointers(void) {
	gob_gobRetVarPtr = (int32 *)VAR_ADDRESS(59);
	gob_curGobStateVarPtr = (int32 *)VAR_ADDRESS(60);
	gob_curGobFrameVarPtr = (int32 *)VAR_ADDRESS(61);
	gob_curGobMultStateVarPtr = (int32 *)VAR_ADDRESS(62);
	gob_curGobNextStateVarPtr = (int32 *)VAR_ADDRESS(63);
	gob_curGobScrXVarPtr = (int32 *)VAR_ADDRESS(64);
	gob_curGobScrYVarPtr = (int32 *)VAR_ADDRESS(65);
	gob_curGobLeftVarPtr = (int32 *)VAR_ADDRESS(66);	
	gob_curGobTopVarPtr = (int32 *)VAR_ADDRESS(67);
	gob_curGobRightVarPtr = (int32 *)VAR_ADDRESS(68);
	gob_curGobBottomVarPtr = (int32 *)VAR_ADDRESS(69);	
	gob_curGobDoAnimVarPtr = (int32 *)VAR_ADDRESS(70);
	gob_curGobOrderVarPtr = (int32 *)VAR_ADDRESS(71);
	gob_curGobNoTickVarPtr = (int32 *)VAR_ADDRESS(72);
	gob_curGobTypeVarPtr = (int32 *)VAR_ADDRESS(73);
	gob_curGobMaxTickVarPtr = (int32 *)VAR_ADDRESS(74);
	gob_curGobTickVarPtr = (int32 *)VAR_ADDRESS(75);
	gob_curGobActStartStateVarPtr = (int32 *)VAR_ADDRESS(76);
	gob_curGobLookDirVarPtr = (int32 *)VAR_ADDRESS(77);
	gob_curGobPickableVarPtr = (int32 *)VAR_ADDRESS(80);
	gob_curGobRelaxVarPtr = (int32 *)VAR_ADDRESS(81);
	gob_destItemStateVarPtr = (int32 *)VAR_ADDRESS(82);
	gob_destItemFrameVarPtr = (int32 *)VAR_ADDRESS(83);
	gob_destItemMultStateVarPtr = (int32 *)VAR_ADDRESS(84);
	gob_destItemNextStateVarPtr = (int32 *)VAR_ADDRESS(85);
	gob_destItemScrXVarPtr = (int32 *)VAR_ADDRESS(86);
	gob_destItemScrYVarPtr = (int32 *)VAR_ADDRESS(87);
	gob_destItemLeftVarPtr = (int32 *)VAR_ADDRESS(88);
	gob_destItemTopVarPtr = (int32 *)VAR_ADDRESS(89);
	gob_destItemRightVarPtr = (int32 *)VAR_ADDRESS(90);
	gob_destItemBottomVarPtr = (int32 *)VAR_ADDRESS(91);
	gob_destItemDoAnimVarPtr = (int32 *)VAR_ADDRESS(92);
	gob_destItemOrderVarPtr = (int32 *)VAR_ADDRESS(93);
	gob_destItemNoTickVarPtr = (int32 *)VAR_ADDRESS(94);
	gob_destItemTypeVarPtr = (int32 *)VAR_ADDRESS(95);
	gob_destItemMaxTickVarPtr = (int32 *)VAR_ADDRESS(96);
	gob_destItemTickVarPtr = (int32 *)VAR_ADDRESS(97);
	gob_destItemActStartStVarPtr = (int32 *)VAR_ADDRESS(98);
	gob_destItemLookDirVarPtr = (int32 *)VAR_ADDRESS(99);
	gob_destItemPickableVarPtr = (int32 *)VAR_ADDRESS(102);
	gob_destItemRelaxVarPtr = (int32 *)VAR_ADDRESS(103);
	gob_destItemMaxFrameVarPtr = (int32 *)VAR_ADDRESS(105);
	gob_curGobVarPtr = (int32 *)VAR_ADDRESS(106);
	gob_some0ValPtr = (int32 *)VAR_ADDRESS(107);
	gob_curGobXPosVarPtr = (int32 *)VAR_ADDRESS(108);
	gob_curGobYPosVarPtr = (int32 *)VAR_ADDRESS(109);
	gob_curGobMaxFrameVarPtr = (int32 *)VAR_ADDRESS(110);
	
	gob_itemInPocketVarPtr = (int32 *)VAR_ADDRESS(114);
	
	*gob_itemInPocketVarPtr = -2;
}

void gob_loadGobDataFromVars(void) {
	Gob_Object *obj;

	gob_itemIndInPocket = *gob_itemInPocketVarPtr;

	obj = gob_goblins[gob_currentGoblin];

	obj->state = *gob_curGobStateVarPtr;
	obj->curFrame = *gob_curGobFrameVarPtr;
	obj->multState = *gob_curGobMultStateVarPtr;
	obj->nextState = *gob_curGobNextStateVarPtr;
	obj->xPos = *gob_curGobScrXVarPtr;
	obj->yPos = *gob_curGobScrYVarPtr;
	obj->left = *gob_curGobLeftVarPtr;
	obj->top = *gob_curGobTopVarPtr;
	obj->right = *gob_curGobRightVarPtr;
	obj->bottom = *gob_curGobBottomVarPtr;
	obj->doAnim = *gob_curGobDoAnimVarPtr;
	obj->order = *gob_curGobOrderVarPtr;
	obj->noTick = *gob_curGobNoTickVarPtr;
	obj->type = *gob_curGobTypeVarPtr;
	obj->maxTick = *gob_curGobMaxTickVarPtr;
	obj->tick = *gob_curGobTickVarPtr;
	obj->actionStartState = *gob_curGobActStartStateVarPtr;
	obj->curLookDir = *gob_curGobLookDirVarPtr;
	obj->pickable = *gob_curGobPickableVarPtr;
	obj->relaxTime = *gob_curGobRelaxVarPtr;

	if (gob_actDestItemDesc == 0)
		return;

	obj = gob_actDestItemDesc;

	obj->state = *gob_destItemStateVarPtr;
	obj->curFrame = *gob_destItemFrameVarPtr;
	obj->multState = *gob_destItemMultStateVarPtr;
	obj->nextState = *gob_destItemNextStateVarPtr;
	obj->xPos = *gob_destItemScrXVarPtr;
	obj->yPos = *gob_destItemScrYVarPtr;
	obj->left = *gob_destItemLeftVarPtr;
	obj->top = *gob_destItemTopVarPtr;
	obj->right = *gob_destItemRightVarPtr;
	obj->bottom = *gob_destItemBottomVarPtr;
	obj->doAnim = *gob_destItemDoAnimVarPtr;
	obj->order = *gob_destItemOrderVarPtr;
	obj->noTick = *gob_destItemNoTickVarPtr;
	obj->type = *gob_destItemTypeVarPtr;
	obj->maxTick = *gob_destItemMaxTickVarPtr;
	obj->tick = *gob_destItemTickVarPtr;
	obj->actionStartState = *gob_destItemActStartStVarPtr;
	obj->curLookDir = *gob_destItemLookDirVarPtr;
	obj->pickable = *gob_destItemPickableVarPtr;
	obj->relaxTime = *gob_destItemRelaxVarPtr;

	if (obj->type != gob_destItemType)
		obj->toRedraw = 1;

	if (obj->state != gob_destItemState && obj->type == 0)
		obj->toRedraw = 1;
}

void gob_pickItem(int16 indexToPocket, int16 idToPocket) {
	int16 x;
	int16 y;

	if (gob_objects[indexToPocket]->pickable != 1)
		return;

	gob_objects[indexToPocket]->type = 3;

	gob_itemIndInPocket = indexToPocket;
	gob_itemIdInPocket = idToPocket;

	for (y = 0; y < 28; y++) {
		for (x = 0; x < 26; x++) {
			if (gob_itemByteFlag == 1) {
				if (((map_itemsMap[y][x] & 0xff00) >> 8) ==
				    idToPocket)
					map_itemsMap[y][x] &= 0xff;
			} else {
				if ((map_itemsMap[y][x] & 0xff) == idToPocket)
					map_itemsMap[y][x] &= 0xff00;
			}
		}
	}

	if (idToPocket >= 0 && idToPocket < 20) {
		map_itemPoses[gob_itemIdInPocket].x = 0;
		map_itemPoses[gob_itemIdInPocket].y = 0;
		map_itemPoses[gob_itemIdInPocket].orient = 0;
	}
}

void gob_placeItem(int16 indexInPocket, int16 idInPocket) {
	Gob_Object *itemDesc;
	int16 lookDir;
	int16 xPos;
	int16 yPos;
	int16 layer;

	itemDesc = gob_objects[indexInPocket];
	lookDir = gob_goblins[0]->curLookDir & 4;

	xPos = gob_gobPositions[0].x;
	yPos = gob_gobPositions[0].y;

	gob_itemIndInPocket = -1;
	gob_itemIdInPocket = 0;

	itemDesc->pickable = 1;
	itemDesc->type = 0;
	itemDesc->toRedraw = 1;
	itemDesc->curFrame = 0;
	itemDesc->order = gob_goblins[0]->order;
	itemDesc->animation =
	    itemDesc->stateMach[itemDesc->state][0]->animation;
	layer =
	    itemDesc->stateMach[itemDesc->state][itemDesc->stateColumn]->layer;

	scen_updateAnim(layer, 0, itemDesc->animation, 0,
	    itemDesc->xPos, itemDesc->yPos, 0);

	itemDesc->yPos +=
	    (gob_gobPositions[0].y * 6) + 5 - scen_toRedrawBottom;

	if (lookDir == 4) {
		itemDesc->xPos += (gob_gobPositions[0].x * 12 + 14)
		    - (scen_toRedrawLeft + scen_toRedrawRight) / 2;
	} else {
		itemDesc->xPos += (gob_gobPositions[0].x * 12)
		    - (scen_toRedrawLeft + scen_toRedrawRight) / 2;
	}

	map_placeItem(xPos, yPos, idInPocket);

	if (yPos > 0) {
		map_placeItem(xPos, yPos - 1, idInPocket);
	}

	if (lookDir == 4) {
		if (xPos < 25) {
			map_placeItem(xPos + 1, yPos, idInPocket);

			if (yPos > 0) {
				map_placeItem(xPos + 1, yPos - 1, idInPocket);
			}
		}
	} else {
		if (xPos > 0) {
			map_placeItem(xPos - 1, yPos, idInPocket);

			if (yPos > 0) {
				map_placeItem(xPos - 1, yPos - 1, idInPocket);
			}
		}
	}

	if (idInPocket >= 0 && idInPocket < 20) {
		map_itemPoses[idInPocket].x = gob_gobPositions[0].x;
		map_itemPoses[idInPocket].y = gob_gobPositions[0].y;
		map_itemPoses[idInPocket].orient = lookDir;
		if (map_itemPoses[idInPocket].orient == 0) {
//                      map_itemPoses[idInPocket].x++;
			if (map_passMap[(int)map_itemPoses[idInPocket].y][map_itemPoses[idInPocket].x + 1] == 1)
				map_itemPoses[idInPocket].x++;
		} else {
			if (map_passMap[(int)map_itemPoses[idInPocket].y][map_itemPoses[idInPocket].x - 1] == 1)
				map_itemPoses[idInPocket].x--;
		}
	}
}

void gob_swapItems(int16 indexToPick, int16 idToPick) {
	int16 layer;
	Gob_Object *pickObj;
	Gob_Object *placeObj;
	int16 idToPlace;
	int16 x;
	int16 y;

	pickObj = gob_objects[indexToPick];
	placeObj = gob_objects[gob_itemIndInPocket];

	idToPlace = gob_itemIdInPocket;
	pickObj->type = 3;
	gob_itemIndInPocket = indexToPick;
	gob_itemIdInPocket = idToPick;

	if (gob_itemByteFlag == 0) {
		for (y = 0; y < 28; y++) {
			for (x = 0; x < 26; x++) {
				if ((map_itemsMap[y][x] & 0xff) == idToPick)
					map_itemsMap[y][x] =
					    (map_itemsMap[y][x] & 0xff00) +
					    idToPlace;
			}
		}
	} else {

		for (y = 0; y < 28; y++) {
			for (x = 0; x < 26; x++) {
				if (((map_itemsMap[y][x] & 0xff00) >> 8) ==
				    idToPick)
					map_itemsMap[y][x] =
					    (map_itemsMap[y][x] & 0xff) +
					    (idToPlace << 8);
			}
		}
	}

	if (idToPick >= 0 && idToPick < 20) {
		map_itemPoses[idToPlace].x =
		    map_itemPoses[gob_itemIdInPocket].x;
		map_itemPoses[idToPlace].y =
		    map_itemPoses[gob_itemIdInPocket].y;
		map_itemPoses[idToPlace].orient =
		    map_itemPoses[gob_itemIdInPocket].orient;

		map_itemPoses[gob_itemIdInPocket].x = 0;
		map_itemPoses[gob_itemIdInPocket].y = 0;
		map_itemPoses[gob_itemIdInPocket].orient = 0;
	}

	gob_itemIndInPocket = -1;
	gob_itemIdInPocket = 0;

	placeObj->type = 0;
	placeObj->nextState = -1;
	placeObj->multState = -1;
	placeObj->unk14 = 0;
	placeObj->toRedraw = 1;
	placeObj->curFrame = 0;
	placeObj->order = gob_goblins[0]->order;

	placeObj->animation =
	    placeObj->stateMach[placeObj->state][0]->animation;

	layer =
	    placeObj->stateMach[placeObj->state][placeObj->stateColumn]->layer;
	scen_updateAnim(layer, 0, placeObj->animation, 0, placeObj->xPos,
	    placeObj->yPos, 0);

	placeObj->yPos +=
	    (gob_gobPositions[0].y * 6) + 5 - scen_toRedrawBottom;

	if (map_itemPoses[idToPlace].orient == 4) {
		placeObj->xPos += (gob_gobPositions[0].x * 12 + 14)
		    - (scen_toRedrawLeft + scen_toRedrawRight) / 2;
	} else {
		placeObj->xPos += (gob_gobPositions[0].x * 12)
		    - (scen_toRedrawLeft + scen_toRedrawRight) / 2;
	}
}

void gob_treatItemPick(int16 itemId) {
	int16 itemIndex;
	Gob_Object *gobDesc;

	gobDesc = gob_goblins[gob_currentGoblin];

	if (gobDesc->curFrame != 9)
		return;

	if (gobDesc->stateMach != gobDesc->realStateMach)
		return;

	gob_readyToAct = 0;
	gob_goesAtTarget = 0;

	itemIndex = gob_itemToObject[itemId];
	if (itemId != 0 && itemIndex != -1
	    && gob_objects[itemIndex]->pickable != 1)
		itemIndex = -1;

	if (gob_itemIndInPocket != -1 && gob_itemIndInPocket == itemIndex)
		itemIndex = -1;

	if (gob_itemIndInPocket != -1 && itemIndex != -1
	    && gob_objects[itemIndex]->pickable == 1) {
		gob_swapItems(itemIndex, itemId);
		gob_itemIndInPocket = itemIndex;
		gob_itemIdInPocket = itemId;
		return;
	}

	if (gob_itemIndInPocket != -1 && itemIndex == -1) {
		gob_placeItem(gob_itemIndInPocket, gob_itemIdInPocket);
		return;
	}

	if (gob_itemIndInPocket == -1 && itemIndex != -1) {
		gob_pickItem(itemIndex, itemId);
		return;
	}
}

int16 gob_treatItem(int16 action) {
	int16 state;

	state = gob_goblins[gob_currentGoblin]->state;
	if ((state == 10 || state == 11) &&
	    gob_goblins[gob_currentGoblin]->curFrame == 0) {
		gob_readyToAct = 0;
	}

	if (action == 3 && gob_currentGoblin == 0 &&
	    (state == 10 || state == 11) && gob_goblins[0]->curFrame == 0) {
		gob_saveGobDataToVars(gob_gobPositions[gob_currentGoblin].x,
		    gob_gobPositions[gob_currentGoblin].y, 0);
		gob_goesAtTarget = 1;
		return -1;
	}

	if (gob_noPick == 0 && gob_currentGoblin == 0 &&
	    (state == 10 || state == 11)) {
		gob_treatItemPick(gob_destActionItem);

		gob_saveGobDataToVars(gob_gobPositions[gob_currentGoblin].x,
		    gob_gobPositions[gob_currentGoblin].y, 0);
		return 0;
	}

	if (gob_goesAtTarget == 0) {
		gob_saveGobDataToVars(gob_gobPositions[gob_currentGoblin].x,
		    gob_gobPositions[gob_currentGoblin].y, 0);
		return 0;
	} else {

		if (gob_itemToObject[gob_destActionItem] != 100 &&
		    gob_destActionItem != 0) {

			if (gob_itemToObject[gob_destActionItem] == -1) {
				gob_actDestItemDesc = 0;
			} else {
				gob_actDestItemDesc =
				    gob_objects[gob_itemToObject
				    [gob_destActionItem]];
			}
		}

		gob_goesAtTarget = 0;
		gob_saveGobDataToVars(gob_gobPositions[gob_currentGoblin].x,
		    gob_gobPositions[gob_currentGoblin].y, 0);
		return gob_destActionItem;
	}
}

void gob_interFunc(void) {
	int16 cmd;
	int16 extraData = 0;
	Gob_Object *objDesc = NULL;
	Gob_Object *gobDesc;
	int16 xPos;
	int16 yPos;
	int16 x;
	int16 y;
	int16 item;
	int16 val;
	int16 layer;
	int16 state;
	int32 *retVarPtr;

	retVarPtr = (int32 *)VAR_ADDRESS(59);

	cmd = inter_load16();
	inter_execPtr += 2;
	if (cmd > 0 && cmd < 17) {
		extraData = inter_load16();
		objDesc = gob_objects[extraData];
		extraData = inter_load16();
	}

	if (cmd > 90 && cmd < 107) {
		extraData = inter_load16();
		objDesc = gob_goblins[extraData];
		extraData = inter_load16();
		cmd -= 90;
	}

	if (cmd > 110 && cmd < 128) {
		extraData = inter_load16();
		objDesc = gob_goblins[extraData];
		cmd -= 90;
	} else if (cmd > 20 && cmd < 38) {
		extraData = inter_load16();
		objDesc = gob_objects[extraData];
	}

	if (cmd < 40 && objDesc == 0)
		return;

	debug(5, "cmd = %d", cmd);
	switch (cmd) {
	case 1:
		objDesc->state = extraData;

		if (objDesc == gob_actDestItemDesc)
			*gob_destItemStateVarPtr = extraData;
		break;

	case 2:
		objDesc->curFrame = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemFrameVarPtr = extraData;
		break;

	case 3:
		objDesc->nextState = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemNextStateVarPtr = extraData;
		break;

	case 4:
		objDesc->multState = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemMultStateVarPtr = extraData;
		break;

	case 5:
		objDesc->order = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemOrderVarPtr = extraData;
		break;

	case 6:
		objDesc->actionStartState = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemActStartStVarPtr = extraData;
		break;

	case 7:
		objDesc->curLookDir = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemLookDirVarPtr = extraData;
		break;

	case 8:
		objDesc->type = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemTypeVarPtr = extraData;

		if (extraData == 0)
			objDesc->toRedraw = 1;
		break;

	case 9:
		objDesc->noTick = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemNoTickVarPtr = extraData;
		break;

	case 10:
		objDesc->pickable = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemPickableVarPtr = extraData;
		break;

	case 12:
		objDesc->xPos = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemScrXVarPtr = extraData;
		break;

	case 13:
		objDesc->yPos = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemScrYVarPtr = extraData;
		break;

	case 14:
		objDesc->doAnim = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemDoAnimVarPtr = extraData;
		break;

	case 15:
		objDesc->relaxTime = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemRelaxVarPtr = extraData;
		break;

	case 16:
		objDesc->maxTick = extraData;
		if (objDesc == gob_actDestItemDesc)
			*gob_destItemMaxTickVarPtr = extraData;
		break;

	case 21:
		*retVarPtr = objDesc->state;
		break;

	case 22:
		*retVarPtr = objDesc->curFrame;
		break;

	case 23:
		*retVarPtr = objDesc->nextState;
		break;

	case 24:
		*retVarPtr = objDesc->multState;
		break;

	case 25:
		*retVarPtr = objDesc->order;
		break;

	case 26:
		*retVarPtr = objDesc->actionStartState;
		break;

	case 27:
		*retVarPtr = objDesc->curLookDir;
		break;

	case 28:
		*retVarPtr = objDesc->type;
		break;

	case 29:
		*retVarPtr = objDesc->noTick;
		break;

	case 30:
		*retVarPtr = objDesc->pickable;
		break;

	case 32:
		*retVarPtr = gob_getObjMaxFrame(objDesc);
		break;

	case 33:
		*retVarPtr = objDesc->xPos;
		break;

	case 34:
		*retVarPtr = objDesc->yPos;
		break;

	case 35:
		*retVarPtr = objDesc->doAnim;
		break;

	case 36:
		*retVarPtr = objDesc->relaxTime;
		break;

	case 37:
		*retVarPtr = objDesc->maxTick;
		break;

	case 40:
	case 42:
		xPos = inter_load16();
		yPos = inter_load16();
		item = inter_load16();

		if (cmd == 42) {
			xPos = VAR(xPos);
			yPos = VAR(yPos);
			item = VAR(item);
		}

		for (y = 0; y < 28; y++) {
			for (x = 0; x < 26; x++) {
				if ((map_itemsMap[y][x] & 0xff) == item) {
					map_itemsMap[y][x] &= 0xff00;
				} else if (((map_itemsMap[y][x] & 0xff00) >> 8)
				    == item) {
					map_itemsMap[y][x] &= 0xff;
				}
			}
		}

		if (xPos < 25) {
			if (yPos > 0) {
				if ((map_itemsMap[yPos][xPos] & 0xff00) != 0 ||
				    (map_itemsMap[yPos - 1][xPos] & 0xff00) !=
				    0
				    || (map_itemsMap[yPos][xPos +
					    1] & 0xff00) != 0
				    || (map_itemsMap[yPos - 1][xPos +
					    1] & 0xff00) != 0) {

					map_itemsMap[yPos][xPos] =
					    (map_itemsMap[yPos][xPos] & 0xff00)
					    + item;

					map_itemsMap[yPos - 1][xPos] =
					    (map_itemsMap[yPos -
						1][xPos] & 0xff00) + item;

					map_itemsMap[yPos][xPos + 1] =
					    (map_itemsMap[yPos][xPos +
						1] & 0xff00) + item;

					map_itemsMap[yPos - 1][xPos + 1] =
					    (map_itemsMap[yPos - 1][xPos +
						1] & 0xff00) + item;
				} else {
					map_itemsMap[yPos][xPos] =
					    (map_itemsMap[yPos][xPos] & 0xff) +
					    (item << 8);

					map_itemsMap[yPos - 1][xPos] =
					    (map_itemsMap[yPos -
						1][xPos] & 0xff) + (item << 8);

					map_itemsMap[yPos][xPos + 1] =
					    (map_itemsMap[yPos][xPos +
						1] & 0xff) + (item << 8);

					map_itemsMap[yPos - 1][xPos + 1] =
					    (map_itemsMap[yPos - 1][xPos +
						1] & 0xff) + (item << 8);
				}
			} else {
				if ((map_itemsMap[yPos][xPos] & 0xff00) != 0 ||
				    (map_itemsMap[yPos][xPos + 1] & 0xff00) !=
				    0) {
					map_itemsMap[yPos][xPos] =
					    (map_itemsMap[yPos][xPos] & 0xff00)
					    + item;

					map_itemsMap[yPos][xPos + 1] =
					    (map_itemsMap[yPos][xPos +
						1] & 0xff00) + item;
				} else {
					map_itemsMap[yPos][xPos] =
					    (map_itemsMap[yPos][xPos] & 0xff) +
					    (item << 8);

					map_itemsMap[yPos][xPos + 1] =
					    (map_itemsMap[yPos][xPos +
						1] & 0xff) + (item << 8);
				}
			}
		} else {
			if (yPos > 0) {
				if ((map_itemsMap[yPos][xPos] & 0xff00) != 0 ||
				    (map_itemsMap[yPos - 1][xPos] & 0xff00) !=
				    0) {
					map_itemsMap[yPos][xPos] =
					    (map_itemsMap[yPos][xPos] & 0xff00)
					    + item;

					map_itemsMap[yPos - 1][xPos] =
					    (map_itemsMap[yPos -
						1][xPos] & 0xff00) + item;
				} else {
					map_itemsMap[yPos][xPos] =
					    (map_itemsMap[yPos][xPos] & 0xff) +
					    (item << 8);

					map_itemsMap[yPos - 1][xPos] =
					    (map_itemsMap[yPos -
						1][xPos] & 0xff) + (item << 8);
				}
			} else {
				if ((map_itemsMap[yPos][xPos] & 0xff00) != 0) {
					map_itemsMap[yPos][xPos] =
					    (map_itemsMap[yPos][xPos] & 0xff00)
					    + item;
				} else {
					map_itemsMap[yPos][xPos] =
					    (map_itemsMap[yPos][xPos] & 0xff) +
					    (item << 8);
				}
			}
		}

		if (item < 0 || item >= 20)
			break;

		if (xPos > 1 && map_passMap[yPos][xPos - 2] == 1) {
			map_itemPoses[item].x = xPos - 2;
			map_itemPoses[item].y = yPos;
			map_itemPoses[item].orient = 4;
			break;
		}

		if (xPos < 24 && map_passMap[yPos][xPos + 2] == 1) {
			map_itemPoses[item].x = xPos + 2;
			map_itemPoses[item].y = yPos;
			map_itemPoses[item].orient = 0;
			break;
		}

		if (xPos < 25 && map_passMap[yPos][xPos + 1] == 1) {
			map_itemPoses[item].x = xPos + 1;
			map_itemPoses[item].y = yPos;
			map_itemPoses[item].orient = 0;
			break;
		}

		if (xPos > 0 && map_passMap[yPos][xPos - 1] == 1) {
			map_itemPoses[item].x = xPos - 1;
			map_itemPoses[item].y = yPos;
			map_itemPoses[item].orient = 4;
			break;
		}
		break;

	case 41:
	case 43:
		xPos = inter_load16();
		yPos = inter_load16();

		if (cmd == 43) {
			xPos = VAR(xPos);
			yPos = VAR(yPos);
		}

		if ((map_itemsMap[yPos][xPos] & 0xff00) != 0) {
			*retVarPtr = (map_itemsMap[yPos][xPos] & 0xff00) >> 8;
		} else {
			*retVarPtr = map_itemsMap[yPos][xPos];
		}
		break;

	case 44:
		xPos = inter_load16();
		yPos = inter_load16();
		val = inter_load16();
		map_passMap[yPos][xPos] = val;
		break;

	case 50:
		item = inter_load16();
		xPos = inter_load16();
		yPos = inter_load16();

		gob_gobPositions[item].x = xPos * 2;
		gob_gobPositions[item].y = yPos * 2;

		objDesc = gob_goblins[item];
		objDesc->nextState = 21;

		gob_nextLayer(objDesc);

		layer = objDesc->stateMach[objDesc->state][0]->layer;

		scen_updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->yPos =
		    (gob_gobPositions[item].y * 6 + 6) - (scen_toRedrawBottom -
		    scen_animTop);
		objDesc->xPos =
		    gob_gobPositions[item].x * 12 - (scen_toRedrawLeft -
		    scen_animLeft);

		objDesc->curFrame = 0;
		objDesc->state = 21;
		if (gob_currentGoblin == item) {
			*gob_curGobScrXVarPtr = objDesc->xPos;
			*gob_curGobScrYVarPtr = objDesc->yPos;

			*gob_curGobFrameVarPtr = 0;
			*gob_curGobStateVarPtr = 18;
			gob_pressedMapX = gob_gobPositions[item].x;
			gob_pressedMapY = gob_gobPositions[item].y;
		}
		break;

	case 52:
		item = inter_load16();
		*retVarPtr = gob_gobPositions[item].x >> 1;
		break;

	case 53:
		item = inter_load16();
		*retVarPtr = gob_gobPositions[item].y >> 1;
		break;

	case 150:
		item = inter_load16();
		xPos = inter_load16();
		yPos = inter_load16();

		objDesc = gob_goblins[item];
		if (yPos == 0) {
			objDesc->multState = xPos;
			objDesc->nextState = xPos;
			gob_nextLayer(objDesc);

			layer = objDesc->stateMach[objDesc->state][0]->layer;

			objDesc->xPos =
			    scen_animations[objDesc->animation].layers[layer]->
			    posX;
			objDesc->yPos =
			    scen_animations[objDesc->animation].layers[layer]->
			    posY;

			*gob_curGobScrXVarPtr = objDesc->xPos;
			*gob_curGobScrYVarPtr = objDesc->yPos;
			*gob_curGobFrameVarPtr = 0;
			*gob_curGobStateVarPtr = objDesc->state;
			*gob_curGobNextStateVarPtr = objDesc->nextState;
			*gob_curGobMultStateVarPtr = objDesc->multState;
			*gob_curGobMaxFrameVarPtr =
			    gob_getObjMaxFrame(objDesc);
			gob_noPick = 1;
			break;
		}

		objDesc->multState = 21;
		objDesc->nextState = 21;
		objDesc->state = 21;
		gob_nextLayer(objDesc);
		layer = objDesc->stateMach[objDesc->state][0]->layer;

		scen_updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->yPos =
		    (yPos * 6 + 6) - (scen_toRedrawBottom - scen_animTop);
		objDesc->xPos =
		    xPos * 12 - (scen_toRedrawLeft - scen_animLeft);

		gob_gobPositions[item].x = xPos;
		gob_pressedMapX = xPos;
		map_curGoblinX = xPos;

		gob_gobPositions[item].y = yPos;
		gob_pressedMapY = yPos;
		map_curGoblinY = yPos;

		*gob_curGobScrXVarPtr = objDesc->xPos;
		*gob_curGobScrYVarPtr = objDesc->yPos;
		*gob_curGobFrameVarPtr = 0;
		*gob_curGobStateVarPtr = 21;
		*gob_curGobNextStateVarPtr = 21;
		*gob_curGobMultStateVarPtr = -1;
		gob_noPick = 0;
		break;

	case 250:
		item = inter_load16();
		xPos = inter_load16();
		yPos = inter_load16();

		gob_gobPositions[item].x = xPos;
		gob_gobPositions[item].y = yPos;

		objDesc = gob_goblins[item];
		objDesc->nextState = 21;
		gob_nextLayer(objDesc);

		layer = objDesc->stateMach[objDesc->state][0]->layer;

		scen_updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->yPos =
		    (yPos * 6 + 6) - (scen_toRedrawBottom - scen_animTop);
		objDesc->xPos =
		    xPos * 12 - (scen_toRedrawLeft - scen_animLeft);

		objDesc->curFrame = 0;
		objDesc->state = 21;

		if (gob_currentGoblin == item) {
			*gob_curGobScrXVarPtr = objDesc->xPos;
			*gob_curGobScrYVarPtr = objDesc->yPos;
			*gob_curGobFrameVarPtr = 0;
			*gob_curGobStateVarPtr = 18;

			gob_pressedMapX = gob_gobPositions[item].x;
			gob_pressedMapY = gob_gobPositions[item].y;
		}
		break;

	case 251:
		item = inter_load16();
		state = inter_load16();

		objDesc = gob_goblins[item];
		objDesc->nextState = state;

		gob_nextLayer(objDesc);
		layer = objDesc->stateMach[objDesc->state][0]->layer;

		objDesc->xPos =
		    scen_animations[objDesc->animation].layers[layer]->posX;
		objDesc->yPos =
		    scen_animations[objDesc->animation].layers[layer]->posY;

		if (item == gob_currentGoblin) {
			*gob_curGobScrXVarPtr = objDesc->xPos;
			*gob_curGobScrYVarPtr = objDesc->yPos;
			*gob_curGobFrameVarPtr = 0;
			*gob_curGobStateVarPtr = objDesc->state;
			*gob_curGobMultStateVarPtr = objDesc->multState;
		}
		break;

	case 252:
		item = inter_load16();
		state = inter_load16();
		objDesc = gob_objects[item];

		objDesc->nextState = state;

		gob_nextLayer(objDesc);
		layer = objDesc->stateMach[objDesc->state][0]->layer;
		objDesc->xPos =
		    scen_animations[objDesc->animation].layers[layer]->posX;
		objDesc->yPos =
		    scen_animations[objDesc->animation].layers[layer]->posY;

		objDesc->toRedraw = 1;
		objDesc->type = 0;
		if (objDesc == gob_actDestItemDesc) {
			*gob_destItemScrXVarPtr = objDesc->xPos;
			*gob_destItemScrYVarPtr = objDesc->yPos;

			*gob_destItemStateVarPtr = objDesc->state;
			*gob_destItemNextStateVarPtr = -1;
			*gob_destItemMultStateVarPtr = -1;
			*gob_destItemFrameVarPtr = 0;
		}
		break;

	case 152:
		item = inter_load16();
		val = inter_load16();
		objDesc = gob_objects[item];
		objDesc->unk14 = val;
		break;

	case 200:
		gob_itemIdInPocket = inter_load16();
		break;

	case 201:
		gob_itemIndInPocket = inter_load16();
		break;

	case 202:
		*retVarPtr = gob_itemIdInPocket;
		break;

	case 203:
		*retVarPtr = gob_itemIndInPocket;
		break;

	case 204:
		item = inter_load16();
		xPos = inter_load16();
		yPos = inter_load16();
		val = inter_load16();

		map_itemPoses[item].x = xPos;
		map_itemPoses[item].y = yPos;
		map_itemPoses[item].orient = val;
		break;

	case 500:
		extraData = inter_load16();
		objDesc = gob_objects[extraData];

		objDesc->relaxTime--;
		if (objDesc->relaxTime < 0 &&
		    gob_getObjMaxFrame(objDesc) == objDesc->curFrame) {
			objDesc->relaxTime = util_getRandom(100) + 50;
			objDesc->curFrame = 0;
			objDesc->toRedraw = 1;
		}
		break;

	case 502:
		item = inter_load16();
		*retVarPtr = gob_gobPositions[item].x;
		break;

	case 503:
		item = inter_load16();
		*retVarPtr = gob_gobPositions[item].y;
		break;

	case 600:
		gob_pathExistence = 0;
		break;

	case 601:
		extraData = inter_load16();
		gob_goblins[extraData]->visible = 1;
		break;

	case 602:
		extraData = inter_load16();
		gob_goblins[extraData]->visible = 0;
		break;

	case 603:
		extraData = inter_load16();
		item = inter_load16();

		objDesc = gob_objects[extraData];
		if (gob_objIntersected(objDesc, gob_goblins[item]) != 0)
			*retVarPtr = 1;
		else
			*retVarPtr = 0;
		break;

	case 604:
		extraData = inter_load16();
		item = inter_load16();

		objDesc = gob_goblins[extraData];
		if (gob_objIntersected(objDesc, gob_goblins[item]) != 0)
			*retVarPtr = 1;
		else
			*retVarPtr = 0;
		break;

	case 605:
		item = inter_load16();
		xPos = inter_load16();
		yPos = inter_load16();
		val = inter_load16();

		map_itemPoses[item].x = xPos;
		map_itemPoses[item].y = yPos;
		map_itemPoses[item].orient = val;
		break;

	case 1000:
		extraData = inter_load16();
		if (game_extHandle >= 0)
			data_closeData(game_extHandle);

		gob_loadObjects((char *)VAR_ADDRESS(extraData));
		game_extHandle = data_openData(game_curExtFile);
		break;

	case 1001:
		gob_freeAllObjects();
		break;

	case 1002:
		gob_animateObjects();
		break;

	case 1003:
		gob_drawObjects();

		if (cd_getTrackPos() == -1)
			cd_playBgMusic();
		break;

	case 1004:
		map_loadMapsInitGobs();
		break;

	case 1005:
		extraData = inter_load16();
		xPos = inter_load16();

		if ((uint16)VAR(xPos) == 0) {
			item =
			    gob_doMove(gob_goblins[gob_currentGoblin], 1,
			    (uint16)VAR(extraData));
		} else {
			item =
			    gob_doMove(gob_goblins[gob_currentGoblin], 1, 3);
		}

		if (item != 0)
			gob_switchGoblin(item);
		break;

	case 1006:
		gob_switchGoblin(0);
		break;

	case 1008:
		gob_loadGobDataFromVars();
		break;

	case 1009:
		extraData = inter_load16();
		cmd = inter_load16();
		xPos = inter_load16();

		if ((uint16)VAR(xPos) == 0) {
			WRITE_VAR(cmd, gob_treatItem((uint16)VAR(extraData)));
			break;
		}

		WRITE_VAR(cmd, gob_treatItem(3));
		break;

	case 1010:
		gob_doMove(gob_goblins[gob_currentGoblin], 0, 0);
		break;

	case 1011:
		extraData = inter_load16();
		if (VAR(extraData) != 0)
			gob_goesAtTarget = 1;
		else
			gob_goesAtTarget = 0;
		break;

	case 1015:
		extraData = inter_load16();
		extraData = VAR(extraData);
		gob_objects[10]->xPos = extraData;

		extraData = inter_load16();
		extraData = VAR(extraData);
		gob_objects[10]->yPos = extraData;
		break;

	case 2005:
		gobDesc = gob_goblins[0];
		if (gob_currentGoblin != 0) {
			gob_goblins[gob_currentGoblin]->doAnim = 1;
			gob_goblins[gob_currentGoblin]->nextState = 21;

			gob_nextLayer(gob_goblins[gob_currentGoblin]);
			gob_currentGoblin = 0;

			gobDesc->doAnim = 0;
			gobDesc->type = 0;
			gobDesc->toRedraw = 1;

			gob_pressedMapX = gob_gobPositions[0].x;
			map_destX = gob_gobPositions[0].x;
			gob_gobDestX = gob_gobPositions[0].x;

			gob_pressedMapY = gob_gobPositions[0].y;
			map_destY = gob_gobPositions[0].y;
			gob_gobDestY = gob_gobPositions[0].y;

			*gob_curGobVarPtr = 0;
			gob_pathExistence = 0;
			gob_readyToAct = 0;
		}

		if (gobDesc->state != 10 && gob_itemIndInPocket != -1 &&
		    gob_getObjMaxFrame(gobDesc) == gobDesc->curFrame) {

			gobDesc->stateMach = gobDesc->realStateMach;
			xPos = gob_gobPositions[0].x;
			yPos = gob_gobPositions[0].y;

			gobDesc->nextState = 10;
			layer = gob_nextLayer(gobDesc);

			scen_updateAnim(layer, 0, gobDesc->animation, 0,
			    gobDesc->xPos, gobDesc->yPos, 0);

			gobDesc->yPos =
			    (yPos * 6 + 6) - (scen_toRedrawBottom -
			    scen_animTop);
			gobDesc->xPos =
			    xPos * 12 - (scen_toRedrawLeft - scen_animLeft);
		}

		if (gobDesc->state != 10)
			break;

		if (gob_itemIndInPocket == -1)
			break;

		if (gobDesc->curFrame != 10)
			break;

		objDesc = gob_objects[gob_itemIndInPocket];
		objDesc->type = 0;
		objDesc->toRedraw = 1;
		objDesc->curFrame = 0;

		objDesc->order = gobDesc->order;
		objDesc->animation =
		    objDesc->stateMach[objDesc->state][0]->animation;

		layer = objDesc->stateMach[objDesc->state][0]->layer;

		scen_updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->yPos +=
		    (gob_gobPositions[0].y * 6 + 5) - scen_toRedrawBottom;

		if (gobDesc->curLookDir == 4) {
			objDesc->xPos += gob_gobPositions[0].x * 12 + 14
			    - (scen_toRedrawLeft + scen_toRedrawRight) / 2;
		} else {
			objDesc->xPos += gob_gobPositions[0].x * 12
			    - (scen_toRedrawLeft + scen_toRedrawRight) / 2;
		}

		gob_itemIndInPocket = -1;
		gob_itemIdInPocket = -1;
		util_beep(50);
		break;

	default:
		warning("gob_interFunc: Unknown command %d!", cmd);
		inter_execPtr -= 2;
		cmd = inter_load16();
		inter_execPtr += cmd * 2;
		break;
	}
	return;
}

} // End of namespace Gob
