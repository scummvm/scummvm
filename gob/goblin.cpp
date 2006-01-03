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

Goblin::Goblin(GobEngine *vm) : _vm(vm) {
	goesAtTarget = 0;
	readyToAct = 0;
	gobAction = 0;
	itemIndInPocket = 5;
	itemIdInPocket = 2;
	itemByteFlag = 0;
	destItemId = -1;
	destActionItem = 0;
	actDestItemDesc = 0;
	forceNextState[0] = -1;
	forceNextState[1] = -1;
	forceNextState[2] = -1;
	forceNextState[3] = -1;
	forceNextState[4] = -1;
	forceNextState[5] = -1;
	forceNextState[6] = -1;
	forceNextState[7] = 0;
	forceNextState[8] = 0;
	forceNextState[9] = 0;

	rotStates[0][0] = 0; rotStates[0][1] = 22; rotStates[0][2] = 23; rotStates[0][3] = 24;
	rotStates[1][0] = 13; rotStates[1][1] = 2; rotStates[1][2] = 12; rotStates[1][3] = 14;
	rotStates[2][0] = 16; rotStates[2][1] = 15; rotStates[2][2] = 4; rotStates[2][3] = 17;
	rotStates[3][0] = 27; rotStates[3][1] = 25; rotStates[3][2] = 26; rotStates[3][3] = 6;

	boreCounter = 0;
	positionedGob = 5;

	noPick = 0;
	objList = 0;
	for (int i = 0; i < 4; i++)
		goblins[i] = 0;
	currentGoblin = 0;
	for (int i = 0; i < 16; i++)
		soundData[i] = 0;
	for (int i = 0; i < 3; i++) {
		gobPositions[i].x = 0;
		gobPositions[i].y = 0;
	}
	gobDestX = 0;
	gobDestY = 0;
	pressedMapX = 0;
	pressedMapY = 0;
	pathExistence = 0;

	some0ValPtr = 0;

	gobRetVarPtr = 0;
	curGobVarPtr = 0;
	curGobXPosVarPtr = 0;
	curGobYPosVarPtr = 0;
	itemInPocketVarPtr = 0;

	curGobStateVarPtr = 0;
	curGobFrameVarPtr = 0;
	curGobMultStateVarPtr = 0;
	curGobNextStateVarPtr = 0;
	curGobScrXVarPtr = 0;
	curGobScrYVarPtr = 0;
	curGobLeftVarPtr = 0;
	curGobTopVarPtr = 0;
	curGobRightVarPtr = 0;
	curGobBottomVarPtr = 0;
	curGobDoAnimVarPtr = 0;
	curGobOrderVarPtr = 0;
	curGobNoTickVarPtr = 0;
	curGobTypeVarPtr = 0;
	curGobMaxTickVarPtr = 0;
	curGobTickVarPtr = 0;
	curGobActStartStateVarPtr = 0;
	curGobLookDirVarPtr = 0;
	curGobPickableVarPtr = 0;
	curGobRelaxVarPtr = 0;
	curGobMaxFrameVarPtr = 0;

	destItemStateVarPtr = 0;
	destItemFrameVarPtr = 0;
	destItemMultStateVarPtr = 0;
	destItemNextStateVarPtr = 0;
	destItemScrXVarPtr = 0;
	destItemScrYVarPtr = 0;
	destItemLeftVarPtr = 0;
	destItemTopVarPtr = 0;
	destItemRightVarPtr = 0;
	destItemBottomVarPtr = 0;
	destItemDoAnimVarPtr = 0;
	destItemOrderVarPtr = 0;
	destItemNoTickVarPtr = 0;
	destItemTypeVarPtr = 0;
	destItemMaxTickVarPtr = 0;
	destItemTickVarPtr = 0;
	destItemActStartStVarPtr = 0;
	destItemLookDirVarPtr = 0;
	destItemPickableVarPtr = 0;
	destItemRelaxVarPtr = 0;
	destItemMaxFrameVarPtr = 0;

	destItemType = 0;
	destItemState = 0;
	for (int i = 0; i < 20; i++) {
		itemToObject[i] = 0;
		objects[i] = 0;
	}
	objCount = 0;
	gobsCount = 0;
}

char Goblin::rotateState(int16 from, int16 to) {
	return rotStates[from / 2][to / 2];
}

int16 Goblin::peekGoblin(Gob_Object *curGob) {
	Util::ListNode *ptr;
	Gob_Object *desc;
	int16 index;
	int16 i;

	ptr = objList->pHead;
	index = 0;
	while (ptr != 0) {
		desc = (Gob_Object *) ptr->pData;
		if (desc != curGob) {
			for (i = 0; i < 3; i++) {
				if (desc != goblins[i])
					continue;

				if (_vm->_global->inter_mouseX < desc->right &&
				    _vm->_global->inter_mouseX > desc->left &&
				    _vm->_global->inter_mouseY < desc->bottom &&
				    _vm->_global->inter_mouseY > desc->top) {
					index = i + 1;
				}
			}
		}
		ptr = ptr->pNext;
	}
	return index;
}

void Goblin::initList(void) {
	objList = (Util::List *) malloc(sizeof(Util::List));
	objList->pHead = 0;
	objList->pTail = 0;
}

void Goblin::sortByOrder(Util::List *list) {
	Util::ListNode *ptr;
	Util::ListNode *ptr2;

	ptr = list->pHead;
	while (ptr->pNext != 0) {
		for (ptr2 = ptr->pNext; ptr2 != 0; ptr2 = ptr2->pNext) {
			Gob_Object *objDesc = (Gob_Object *)ptr->pData;
			Gob_Object *objDesc2 = (Gob_Object *)ptr2->pData;

			if (objDesc->order <= objDesc2->order) {
				if (objDesc->order != objDesc2->order)
					continue;

				if (objDesc->bottom <= objDesc2->bottom) {
					if (objDesc->bottom != objDesc2->bottom)
						continue;

					if (objDesc != goblins[currentGoblin])
						continue;
				}
			}

			SWAP(ptr->pData, ptr2->pData);
		}
		ptr = ptr->pNext;
	}
}

void Goblin::playSound(Snd::SoundDesc *snd, int16 repCount, int16 freq) {
	if (snd != 0) {
		_vm->_snd->stopSound(0);
		_vm->_snd->playSample(snd, repCount, freq);
	}
}

void Goblin::drawObjects(void) {
	Util::ListNode *ptr;
	Util::ListNode *ptr2;

	Gob_Object *objDesc;
	Gob_Object *gobDesc2;
	int16 layer;

	ptr = objList->pHead;
	for (ptr = objList->pHead; ptr != 0; ptr = ptr->pNext) {
		objDesc = (Gob_Object *) ptr->pData;

		if (objDesc->type == 3)
			objDesc->toRedraw = 1;
		else if (objDesc->type == 1)
			objDesc->toRedraw = 0;
	}

	for (ptr = objList->pHead; ptr != 0; ptr = ptr->pNext) {
		objDesc = (Gob_Object *) ptr->pData;
		if (objDesc->toRedraw == 0)
			continue;

		_vm->_video->drawSprite(_vm->_anim->_animSurf, _vm->_draw->backSurface,
		    objDesc->left, objDesc->top, objDesc->right,
		    objDesc->bottom, objDesc->left, objDesc->top, 0);

		_vm->_draw->invalidateRect(objDesc->left, objDesc->top,
		    objDesc->right, objDesc->bottom);

		if (objDesc->type != 0)
			continue;

		layer =
		    objDesc->stateMach[objDesc->state][objDesc->stateColumn]->
		    layer;
		_vm->_scenery->updateAnim(layer, objDesc->curFrame, objDesc->animation,
		    0, objDesc->xPos, objDesc->yPos, 0);

		if (_vm->_scenery->toRedrawLeft == -12345) {
			objDesc->dirtyLeft = objDesc->left;
			objDesc->dirtyRight = objDesc->right;
			objDesc->dirtyTop = objDesc->top;
			objDesc->dirtyBottom = objDesc->bottom;
		} else {
			objDesc->dirtyLeft =
			    MIN(objDesc->left, _vm->_scenery->toRedrawLeft);
			objDesc->dirtyRight =
			    MAX(objDesc->right, _vm->_scenery->toRedrawRight);
			objDesc->dirtyTop =
			    MIN(objDesc->top, _vm->_scenery->toRedrawTop);
			objDesc->dirtyBottom =
			    MAX(objDesc->bottom, _vm->_scenery->toRedrawBottom);
		}

		objDesc->dirtyLeft = 0;
		objDesc->dirtyRight = 319;
		objDesc->dirtyTop = 0;
		objDesc->dirtyBottom = 199;
	}

	sortByOrder(objList);
	for (ptr = objList->pHead; ptr != 0; ptr = ptr->pNext) {
		objDesc = (Gob_Object *) ptr->pData;
		if (objDesc->toRedraw) {
			layer =
			    objDesc->stateMach[objDesc->state][objDesc->
			    stateColumn]->layer;

			if (objDesc->type == 0) {
				if (objDesc->visible == 0) {
					_vm->_scenery->updateAnim(layer,
					    objDesc->curFrame,
					    objDesc->animation, 0,
					    objDesc->xPos, objDesc->yPos, 0);

				} else {
					_vm->_scenery->updateAnim(layer,
					    objDesc->curFrame,
					    objDesc->animation, 2,
					    objDesc->xPos, objDesc->yPos, 1);
				}
				if (_vm->_scenery->toRedrawLeft == -12345) {
					objDesc->left = 0;
					objDesc->top = 0;
					objDesc->right = 0;
					objDesc->bottom = 0;
				} else {
					_vm->_draw->invalidateRect(_vm->_scenery->toRedrawLeft,
					    _vm->_scenery->toRedrawTop,
					    _vm->_scenery->toRedrawRight,
					    _vm->_scenery->toRedrawBottom);

					objDesc->left = _vm->_scenery->toRedrawLeft;
					objDesc->top = _vm->_scenery->toRedrawTop;
					objDesc->right = _vm->_scenery->toRedrawRight;
					objDesc->bottom = _vm->_scenery->toRedrawBottom;
					_vm->_scenery->updateStatic(objDesc->order);
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
			for (ptr2 = objList->pHead; ptr2 != 0;
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

				_vm->_scenery->toRedrawLeft = gobDesc2->dirtyLeft;
				_vm->_scenery->toRedrawRight = gobDesc2->dirtyRight;
				_vm->_scenery->toRedrawTop = gobDesc2->dirtyTop;
				_vm->_scenery->toRedrawBottom = gobDesc2->dirtyBottom;

				layer =
				    objDesc->stateMach[objDesc->
				    state][objDesc->stateColumn]->layer;

				_vm->_scenery->updateAnim(layer, objDesc->curFrame,
				    objDesc->animation, 4, objDesc->xPos,
				    objDesc->yPos, 1);

				_vm->_scenery->updateStatic(objDesc->order);
			}
		}
	}

	for (ptr = objList->pHead; ptr != 0; ptr = ptr->pNext) {
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
					playSound(soundData[sndItem],
					    repCount, freq);
				}
			}

			sndFrame = state->sndFrame & 0xff;
			sndItem = state->sndItem & 0xff;
			freq = 100 * (state->freq & 0xff);
			repCount = state->repCount & 0xff;

			if (objDesc->curFrame == sndFrame) {
				if (sndItem != 0xff) {
					playSound(soundData[sndItem],
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
					playSound(soundData[sndItem],
					    repCount, freq);
				}
			}
		}
	}

//      _vm->_scenery->updateAnim(27, 0, 9, 2, 10, 10, 1);
}

void Goblin::animateObjects(void) {
	Util::ListNode *node;
	Gob_Object *objDesc;
	Scenery::AnimLayer *pLayer;
	int16 layer;

	for (node = objList->pHead; node != 0; node = node->pNext) {
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
			    _vm->_scenery->animations[objDesc->animation].layers[layer];

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
					objDesc->stateMach = goblins[(int)(objDesc->multObjIndex)]->stateMach;
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

void Goblin::placeObject(Gob_Object *objDesc, char animated) {
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

		objDesc->relaxTime = _vm->_util->getRandom(30);

		layer = objDesc->stateMach[objDesc->state][0]->layer;
		_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->order = _vm->_scenery->toRedrawBottom / 24 + 3;

		objDesc->left = objDesc->xPos;
		objDesc->right = objDesc->xPos;
		objDesc->dirtyLeft = objDesc->xPos;
		objDesc->dirtyRight = objDesc->xPos;

		objDesc->top = objDesc->yPos;
		objDesc->bottom = objDesc->yPos;
		objDesc->dirtyTop = objDesc->yPos;
		objDesc->dirtyBottom = objDesc->yPos;

		_vm->_util->listInsertBack(objList, objDesc);
	}
}

int16 Goblin::getObjMaxFrame(Gob_Object * objDesc) {
	int16 layer;

	layer = objDesc->stateMach[objDesc->state][0]->layer;
	return _vm->_scenery->animations[objDesc->animation].layers[layer]->framesCount -
	    1;
}

int16 Goblin::objIntersected(Gob_Object *obj1, Gob_Object *obj2) {
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

void Goblin::setMultStates(Gob_Object * gobDesc) {
	gobDesc->stateMach = goblins[(int)gobDesc->multObjIndex]->stateMach;
}

int16 Goblin::nextLayer(Gob_Object *gobDesc) {
	if (gobDesc->nextState == 10)
		gobDesc->curLookDir = 0;

	if (gobDesc->nextState == 11)
		gobDesc->curLookDir = 4;

	if (gobDesc->nextState > 39) {
		setMultStates(gobDesc);
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

void Goblin::showBoredom(int16 gobIndex) {
	Gob_Object *gobDesc;
	int16 frame;
	int16 frameCount;
	int16 layer;
	int16 state;
	int16 boreFlag;

	gobDesc = goblins[gobIndex];
	layer = gobDesc->stateMach[gobDesc->state][0]->layer;

	frameCount =
	    _vm->_scenery->animations[gobDesc->animation].layers[layer]->framesCount;
	state = gobDesc->state;
	frame = gobDesc->curFrame;

	gobDesc->noTick = 0;
	gobDesc->toRedraw = 1;

	boreFlag = 1 << _vm->_util->getRandom(7);

	if (gobIndex != currentGoblin && _vm->_util->getRandom(3) != 0) {
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
void Goblin::switchGoblin(int16 index) {
	int16 next;
	int16 tmp;

	debug(4, "switchGoblin");
	if (VAR(59) != 0)
		return;

	if (goblins[currentGoblin]->state <= 39 &&
	    goblins[currentGoblin]->curFrame != 0)
		return;

	if (index != 0 && goblins[index - 1]->type != 0)
		return;

	if (index == 0)
		next = (currentGoblin + 1) % 3;
	else
		next = index - 1;

	if (_vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 3 ||
	    _vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 6)
		return;

	if (goblins[(currentGoblin + 1) % 3]->type != 0 &&
	    goblins[(currentGoblin + 2) % 3]->type != 0)
		return;

	gobPositions[currentGoblin].x = _vm->_map->curGoblinX;
	gobPositions[currentGoblin].y = _vm->_map->curGoblinY;

	goblins[currentGoblin]->doAnim = 1;
	goblins[currentGoblin]->nextState = 21;

	nextLayer(goblins[currentGoblin]);

	currentGoblin = next;
	if (goblins[currentGoblin]->type != 0)
		currentGoblin = (currentGoblin + 1) % 3;

	goblins[currentGoblin]->doAnim = 0;
	if (goblins[currentGoblin]->curLookDir == 4)
		goblins[currentGoblin]->nextState = 18;
	else
		goblins[currentGoblin]->nextState = 19;

	goblins[currentGoblin]->toRedraw = 1;
	nextLayer(goblins[currentGoblin]);

	tmp = gobPositions[currentGoblin].x;
	pressedMapX = tmp;
	_vm->_map->destX = tmp;
	gobDestX = tmp;
	_vm->_map->curGoblinX = tmp;

	tmp = gobPositions[currentGoblin].y;
	pressedMapY = tmp;
	_vm->_map->destY = tmp;
	gobDestY = tmp;
	_vm->_map->curGoblinY = tmp;

	*curGobVarPtr = currentGoblin;
	pathExistence = 0;
	readyToAct = 0;
}

void Goblin::adjustDest(int16 posX, int16 posY) {
	int16 resDelta;
	int16 resDeltaDir;
	int16 resDeltaPix;
	int16 deltaPix;
	int16 i;

	if (_vm->_map->passMap[pressedMapY][pressedMapX] == 0 &&
	    (gobAction == 0
		|| _vm->_map->itemsMap[pressedMapY][pressedMapX] == 0)) {

		resDelta = -1;
		resDeltaDir = 0;
		resDeltaPix = 0;

		for (i = 1;
		    i <= pressedMapX
		    && _vm->_map->passMap[pressedMapY][pressedMapX - i] == 0;
		    i++);

		if (i <= pressedMapX) {
			resDeltaPix = (i - 1) * 12 + (posX % 12) + 1;
			resDelta = i;
		}

		for (i = 1;
				(i + pressedMapX) < Map::kMapWidth
		    && _vm->_map->passMap[pressedMapY][pressedMapX + i] == 0;
		    i++);

		if (pressedMapX + i < Map::kMapWidth) {
			deltaPix = (i * 12) - (posX % 12);
			if (resDelta == -1 || deltaPix < resDeltaPix) {
				resDeltaPix = deltaPix;
				resDelta = i;
				resDeltaDir = 1;
			}
		}

		for (i = 1;
				(i + pressedMapY) < Map::kMapHeight
		    && _vm->_map->passMap[pressedMapY + i][pressedMapX] == 0;
		    i++);

		if (pressedMapY + i < Map::kMapHeight) {
			deltaPix = (i * 6) - (posY % 6);
			if (resDelta == -1 || deltaPix < resDeltaPix) {
				resDeltaPix = deltaPix;
				resDelta = i;
				resDeltaDir = 2;
			}
		}

		for (i = 1;
		    i <= pressedMapY
		    && _vm->_map->passMap[pressedMapY - i][pressedMapX] == 0;
		    i++);

		if (i <= pressedMapY) {
			deltaPix = (i * 6) + (posY % 6);
			if (resDelta == -1 || deltaPix < resDeltaPix) {
				resDeltaPix = deltaPix;
				resDelta = i;
				resDeltaDir = 3;
			}
		}

		switch (resDeltaDir) {
		case 0:
			pressedMapX -= resDelta;
			break;

		case 1:
			pressedMapX += resDelta;
			break;

		case 2:
			pressedMapY += resDelta;
			break;

		case 3:
			pressedMapY -= resDelta;
			break;
		}

	}
}

void Goblin::adjustTarget(void) {
	if (gobAction == 4
	    && _vm->_map->itemsMap[pressedMapY][pressedMapX] == 0) {

		if (pressedMapY > 0
		    && _vm->_map->itemsMap[pressedMapY - 1][pressedMapX] !=
		    0) {
			pressedMapY--;
		} else if (pressedMapX < Map::kMapWidth - 1
		    && _vm->_map->itemsMap[pressedMapY][pressedMapX + 1] !=
		    0) {
			pressedMapX++;
		} else if (pressedMapX < Map::kMapWidth - 1 && pressedMapY > 0
		    && _vm->_map->itemsMap[pressedMapY - 1][pressedMapX +
			1] != 0) {
			pressedMapY--;
			pressedMapX++;
		}
	}
}

void Goblin::targetDummyItem(Gob_Object *gobDesc) {
	if (_vm->_map->itemsMap[pressedMapY][pressedMapX] == 0 &&
	    _vm->_map->passMap[pressedMapY][pressedMapX] == 1) {
		if (gobDesc->curLookDir == 0) {
			_vm->_map->itemPoses[0].x = pressedMapX;
			_vm->_map->itemPoses[0].y = pressedMapY;
			_vm->_map->itemPoses[0].orient = -4;
		} else {
			_vm->_map->itemPoses[0].x = pressedMapX;
			_vm->_map->itemPoses[0].y = pressedMapY;
			_vm->_map->itemPoses[0].orient = -1;
		}
	}
}

void Goblin::targetItem(void) {
	int16 tmpX;
	int16 tmpY;
	int16 items;
	int16 layer;
	int16 tmpPosX;
	int16 tmpPosY;
	Gob_Object *itemDesc;

	if (gobAction == 3 || gobAction == 4) {
		items = _vm->_map->itemsMap[pressedMapY][pressedMapX];
		if (gobAction == 4 && (items & 0xff00) != 0 &&
		    objects[itemToObject[(items & 0xff00) >> 8]]->
		    pickable == 1) {
			destItemId = (items & 0xff00) >> 8;
			destActionItem = (items & 0xff00) >> 8;
			itemByteFlag = 1;
		} else if ((items & 0xff) == 0) {
			destItemId = (items & 0xff00) >> 8;
			destActionItem = (items & 0xff00) >> 8;
			itemByteFlag = 1;
		} else if (gobAction == 3 && currentGoblin == 2 &&
		    (items & 0xff00) != 0) {
			destItemId = (items & 0xff00) >> 8;
			destActionItem = (items & 0xff00) >> 8;
			itemByteFlag = 1;
		} else {
			destItemId = items & 0xff;
			destActionItem = items & 0xff;
			itemByteFlag = 0;
		}

		pressedMapY = _vm->_map->itemPoses[destItemId].y;
		_vm->_map->destY = _vm->_map->itemPoses[destItemId].y;
		gobDestY = _vm->_map->itemPoses[destItemId].y;

		if (gobAction == 3 || destActionItem == 0) {
			pressedMapX = _vm->_map->itemPoses[destItemId].x;
			_vm->_map->destX = _vm->_map->itemPoses[destItemId].x;
			gobDestX = _vm->_map->itemPoses[destItemId].x;
		} else if ((items & 0xff00) != 0) {
			if (_vm->_map->itemPoses[destItemId].orient == 4) {
				if ((_vm->_map->itemsMap[pressedMapY]
					[pressedMapX - 1] & 0xff00) ==
				    (_vm->_map->itemsMap[pressedMapY]
					[pressedMapX] & 0xff00)) {
					pressedMapX--;
					_vm->_map->destX = pressedMapX;
					gobDestX = pressedMapX;
				}
			} else if (_vm->_map->itemPoses[destItemId].orient == 0) {

				if ((_vm->_map->itemsMap[pressedMapY]
					[pressedMapX + 1] & 0xff00) ==
				    (_vm->_map->itemsMap[pressedMapY]
					[pressedMapX] & 0xff00)) {
					pressedMapX++;
					_vm->_map->destX = pressedMapX;
					gobDestX = pressedMapX;
				}
			}

			if ((_vm->_map->itemsMap[pressedMapY +
				    1][pressedMapX] & 0xff00) ==
			    (_vm->_map->itemsMap[pressedMapY][pressedMapX] &
				0xff00)) {
				pressedMapY++;
				_vm->_map->destY = pressedMapY;
				gobDestY = pressedMapY;
			}
		} else {
			if (_vm->_map->itemPoses[destItemId].orient == 4) {
				if ((_vm->_map->itemsMap[pressedMapY]
					[pressedMapX - 1]) ==
				    (_vm->_map->itemsMap[pressedMapY]
					[pressedMapX])) {
					pressedMapX--;
					_vm->_map->destX = pressedMapX;
					gobDestX = pressedMapX;
				}
			} else if (_vm->_map->itemPoses[destItemId].orient == 0) {

				if ((_vm->_map->itemsMap[pressedMapY]
					[pressedMapX + 1]) ==
				    (_vm->_map->itemsMap[pressedMapY]
					[pressedMapX])) {
					pressedMapX++;
					_vm->_map->destX = pressedMapX;
					gobDestX = pressedMapX;
				}
			}

			if ((_vm->_map->itemsMap[pressedMapY +
				    1][pressedMapX]) ==
			    (_vm->_map->itemsMap[pressedMapY][pressedMapX])) {
				pressedMapY++;
				_vm->_map->destY = pressedMapY;
				gobDestY = pressedMapY;
			}

		}

		if (gobAction == 4 && destActionItem != 0 &&
			itemToObject[destActionItem] != -1 &&
		    objects[itemToObject[destActionItem]]->
		    pickable == 1) {

			itemDesc =
			    objects[itemToObject[destActionItem]];

			itemDesc->animation =
			    itemDesc->stateMach[itemDesc->state][0]->animation;
			layer =
			    itemDesc->stateMach[itemDesc->state][itemDesc->
			    stateColumn]->layer;

			_vm->_scenery->updateAnim(layer, 0, itemDesc->animation, 0,
			    itemDesc->xPos, itemDesc->yPos, 0);

			tmpX = (_vm->_scenery->toRedrawRight + _vm->_scenery->toRedrawLeft) / 2;
			tmpY = _vm->_scenery->toRedrawBottom;

			tmpPosY = tmpY / 6;
			if ((tmpY % 3) < 3 && tmpPosY > 0)
				tmpPosY--;

			tmpPosX = tmpX / 12;
			if ((tmpX % 12) < 6 && tmpPosX > 0)
				tmpPosX--;

			if (_vm->_map->itemPoses[destActionItem].orient == 0 ||
			    _vm->_map->itemPoses[destActionItem].orient == -1) {
				tmpPosX++;
			}

			if (_vm->_map->passMap[tmpPosY][tmpPosX] == 1) {
				pressedMapX = tmpPosX;
				_vm->_map->destX = tmpPosX;
				gobDestX = tmpPosX;

				pressedMapY = tmpPosY;
				_vm->_map->destY = tmpPosY;
				gobDestY = tmpPosY;
			}
		}
	}
}

void Goblin::initiateMove(void) {
	_vm->_map->findNearestToDest();
	_vm->_map->findNearestToGob();
	_vm->_map->optimizePoints();

	pathExistence = _vm->_map->checkDirectPath(_vm->_map->curGoblinX, _vm->_map->curGoblinY,
	    pressedMapX, pressedMapY);

	if (pathExistence == 3) {
		if (_vm->_map->checkLongPath(_vm->_map->curGoblinX, _vm->_map->curGoblinY,
			pressedMapX, pressedMapY,
			_vm->_map->nearestWayPoint, _vm->_map->nearestDest) == 0) {
			pathExistence = 0;
		} else {
			_vm->_map->destX = _vm->_map->wayPoints[_vm->_map->nearestWayPoint].x;
			_vm->_map->destY = _vm->_map->wayPoints[_vm->_map->nearestWayPoint].y;
		}
	}
}

void Goblin::moveFindItem(int16 posX, int16 posY) {
	int16 i;
	if (gobAction == 3 || gobAction == 4) {
		for (i = 0; i < 20; i++) {
			if (objects[i] == 0)
				continue;

			if (objects[i]->type != 0)
				continue;

			if (objects[i]->left > posX)
				continue;

			if (objects[i]->right < posX)
				continue;

			if (objects[i]->top > posY)
				continue;

			if (objects[i]->bottom < posY)
				continue;

			if (objects[i]->right - objects[i]->left < 40)
				posX =
				    (objects[i]->left +
				    objects[i]->right) / 2;

			if (objects[i]->bottom - objects[i]->top < 40)
				posY =
				    (objects[i]->top +
				    objects[i]->bottom) / 2;

			break;
		}

		pressedMapX = posX / 12;
		pressedMapY = posY / 6;

		if (_vm->_map->itemsMap[pressedMapY][pressedMapX] == 0
		    && i < 20) {

			if (_vm->_map->itemsMap[pressedMapY +
				1][pressedMapX] != 0) {
				pressedMapY++;
			} else if (_vm->_map->itemsMap[pressedMapY +
				1][pressedMapX + 1] != 0) {
				pressedMapX++;
				pressedMapY++;
			} else
			    if (_vm->_map->itemsMap[pressedMapY][pressedMapX +
				1] != 0) {
				pressedMapX++;
			} else if (_vm->_map->itemsMap[pressedMapY -
				1][pressedMapX + 1] != 0) {
				pressedMapX++;
				pressedMapY--;
			} else if (_vm->_map->itemsMap[pressedMapY -
				1][pressedMapX] != 0) {
				pressedMapY--;
			} else if (_vm->_map->itemsMap[pressedMapY -
				1][pressedMapX - 1] != 0) {
				pressedMapY--;
				pressedMapX--;
			} else
			    if (_vm->_map->itemsMap[pressedMapY][pressedMapX -
				1] != 0) {
				pressedMapX--;
			} else if (_vm->_map->itemsMap[pressedMapY +
				1][pressedMapX - 1] != 0) {
				pressedMapX--;
				pressedMapY++;
			}
		}
	} else {
		pressedMapX = posX / 12;
		pressedMapY = posY / 6;
	}
}

void Goblin::moveCheckSelect(int16 framesCount, Gob_Object * gobDesc, int16 *pGobIndex,
	    int16 *nextAct) {
	if (gobDesc->right > _vm->_global->inter_mouseX &&
	    gobDesc->left < _vm->_global->inter_mouseX &&
	    gobDesc->bottom > _vm->_global->inter_mouseY &&
	    gobDesc->bottom - 10 < _vm->_global->inter_mouseY && gobAction == 0) {
		if (gobDesc->curLookDir & 4)
			*nextAct = 16;
		else
			*nextAct = 23;

		gobDesc->curFrame = framesCount - 1;
		pathExistence = 0;
	} else {
		*pGobIndex = peekGoblin(gobDesc);

		if (*pGobIndex != 0) {
			pathExistence = 0;
		} else if (_vm->_map->curGoblinX == pressedMapX &&
		    _vm->_map->curGoblinY == pressedMapY) {

			if (gobAction != 0)
				readyToAct = 1;

			pathExistence = 0;
		}
	}
}

void Goblin::moveInitStep(int16 framesCount, int16 action, int16 cont,
	    Gob_Object *gobDesc, int16 *pGobIndex, int16 *pNextAct) {
	int16 posX;
	int16 posY;

	if (cont != 0 && goesAtTarget == 0 &&
	    readyToAct == 0 && VAR(59) == 0 &&
	    gobDesc->type != 1 &&
	    gobDesc->state != 10 && gobDesc->state != 11) {
		if (gobDesc->state >= 40) {
			gobDesc->curFrame = framesCount - 1;
		}

		gobAction = action;
		forceNextState[0] = -1;
		forceNextState[1] = -1;
		forceNextState[2] = -1;

		if (action == 3) {
			posX = _vm->_global->inter_mouseX + 6;
			posY = _vm->_global->inter_mouseY + 7;
		} else if (action == 4) {
			posX = _vm->_global->inter_mouseX + 7;
			posY = _vm->_global->inter_mouseY + 12;
		} else {
			posX = _vm->_global->inter_mouseX;
			posY = _vm->_global->inter_mouseY;
		}

		moveFindItem(posX, posY);
		adjustDest(posX, posY);
		adjustTarget();

		_vm->_map->destX = pressedMapX;
		gobDestX = pressedMapX;

		_vm->_map->destY = pressedMapY;
		gobDestY = pressedMapY;

		targetDummyItem(gobDesc);

		targetItem();
		initiateMove();

		moveCheckSelect(framesCount, gobDesc, pGobIndex, pNextAct);
	} else {

		if (readyToAct != 0 &&
		    (_vm->_map->curGoblinX != pressedMapX ||
			_vm->_map->curGoblinY != pressedMapY))
			readyToAct = 0;

		if (gobDesc->type == 1) {
			*pGobIndex = peekGoblin(gobDesc);
		}
	}
}

void Goblin::moveTreatRopeStairs(Gob_Object *gobDesc) {
	if (currentGoblin != 1)
		return;

	if (gobDesc->nextState == 28
	    && _vm->_map->passMap[_vm->_map->curGoblinY - 1][_vm->_map->curGoblinX] == 6) {
		forceNextState[0] = 28;
		forceNextState[1] = -1;
	}

	if (gobDesc->nextState == 29
	    && _vm->_map->passMap[_vm->_map->curGoblinY + 1][_vm->_map->curGoblinX] == 6) {
		forceNextState[0] = 29;
		forceNextState[1] = -1;
	}

	if ((gobDesc->nextState == 28 || gobDesc->nextState == 29
		|| gobDesc->nextState == 20)
	    && _vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 6) {
		if ((gobDesc->curLookDir == 0 || gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 2)
		    && _vm->_map->passMap[_vm->_map->curGoblinY - 1][_vm->_map->curGoblinX] == 6) {
			forceNextState[0] = 28;
			forceNextState[1] = -1;
		} else if ((gobDesc->curLookDir == 0
			|| gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 6)
		    && _vm->_map->passMap[_vm->_map->curGoblinY + 1][_vm->_map->curGoblinX] == 6) {
			forceNextState[0] = 29;
			forceNextState[1] = -1;
		}
	}

	if (gobDesc->nextState == 8
	    && _vm->_map->passMap[_vm->_map->curGoblinY - 1][_vm->_map->curGoblinX] == 3) {
		forceNextState[0] = 8;
		forceNextState[1] = -1;
	}

	if (gobDesc->nextState == 9
	    && _vm->_map->passMap[_vm->_map->curGoblinY + 1][_vm->_map->curGoblinX] == 3) {
		forceNextState[0] = 9;
		forceNextState[1] = -1;
	}

	if (gobDesc->nextState == 20
	    && _vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 3) {
		if ((gobDesc->curLookDir == 0 || gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 2)
		    && _vm->_map->passMap[_vm->_map->curGoblinY - 1][_vm->_map->curGoblinX] == 3) {
			forceNextState[0] = 8;
			forceNextState[1] = -1;
		} else if ((gobDesc->curLookDir == 0
			|| gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 6)
		    && _vm->_map->passMap[_vm->_map->curGoblinY + 1][_vm->_map->curGoblinX] == 3) {
			forceNextState[0] = 9;
			forceNextState[1] = -1;
		}
	}

}

void Goblin::movePathFind(Gob_Object *gobDesc, int16 nextAct) {
	if (pathExistence == 1) {
		_vm->_map->curGoblinX = gobPositions[currentGoblin].x;
		_vm->_map->curGoblinY = gobPositions[currentGoblin].y;

		if (_vm->_map->curGoblinX == pressedMapX &&
		    _vm->_map->curGoblinY == pressedMapY && gobAction != 0) {
			readyToAct = 1;
			pathExistence = 0;
		}

		nextAct = _vm->_map->getDirection(_vm->_map->curGoblinX, _vm->_map->curGoblinY,
		    _vm->_map->destX, _vm->_map->destY);

		if (nextAct == 0)
			pathExistence = 0;
	} else if (pathExistence == 3) {
		_vm->_map->curGoblinX = gobPositions[currentGoblin].x;
		_vm->_map->curGoblinY = gobPositions[currentGoblin].y;

		if (_vm->_map->curGoblinX == gobDestX && _vm->_map->curGoblinY == gobDestY) {
			pathExistence = 1;
			_vm->_map->destX = pressedMapX;
			_vm->_map->destY = pressedMapY;
		} else {

			if (_vm->_map->checkDirectPath(_vm->_map->curGoblinX, _vm->_map->curGoblinY,
				gobDestX, gobDestY) == 1) {
				_vm->_map->destX = gobDestX;
				_vm->_map->destY = gobDestY;
			} else if (_vm->_map->curGoblinX == _vm->_map->destX && _vm->_map->curGoblinY == _vm->_map->destY) {

				if (_vm->_map->nearestWayPoint > _vm->_map->nearestDest) {
					_vm->_map->optimizePoints();

					_vm->_map->destX =
					    _vm->_map->wayPoints[_vm->_map->nearestWayPoint].
					    x;
					_vm->_map->destY =
					    _vm->_map->wayPoints[_vm->_map->nearestWayPoint].
					    y;

					if (_vm->_map->nearestWayPoint > _vm->_map->nearestDest)
						_vm->_map->nearestWayPoint--;
				} else if (_vm->_map->nearestWayPoint < _vm->_map->nearestDest) {
					_vm->_map->optimizePoints();

					_vm->_map->destX =
					    _vm->_map->wayPoints[_vm->_map->nearestWayPoint].
					    x;
					_vm->_map->destY =
					    _vm->_map->wayPoints[_vm->_map->nearestWayPoint].
					    y;

					if (_vm->_map->nearestWayPoint < _vm->_map->nearestDest)
						_vm->_map->nearestWayPoint++;
				} else {
					if (_vm->_map->checkDirectPath(_vm->_map->curGoblinX,
						_vm->_map->curGoblinY, gobDestX,
						gobDestY) == 3 && _vm->_map->passMap[pressedMapY][pressedMapX] != 0) {
						_vm->_map->destX = _vm->_map->wayPoints[_vm->_map->nearestWayPoint].x;
						_vm->_map->destY = _vm->_map->wayPoints[_vm->_map->nearestWayPoint].y;
					} else {
						pathExistence = 1;
						_vm->_map->destX = pressedMapX;
						_vm->_map->destY = pressedMapY;
					}
				}
			}
			nextAct =
			    _vm->_map->getDirection(_vm->_map->curGoblinX, _vm->_map->curGoblinY,
			    _vm->_map->destX, _vm->_map->destY);
		}
	}

	if (readyToAct != 0 && (gobAction == 3 || gobAction == 4))
		nextAct = 0x4dc8;

	switch (nextAct) {
	case Map::kDirW:
		gobDesc->nextState = rotateState(gobDesc->curLookDir, 0);
		break;

	case Map::kDirE:
		gobDesc->nextState = rotateState(gobDesc->curLookDir, 4);
		break;

	case 16:
		gobDesc->nextState = 16;
		break;

	case 23:
		gobDesc->nextState = 23;
		break;

	case Map::kDirN:
		if (_vm->_map->passMap[_vm->_map->curGoblinY - 1][_vm->_map->curGoblinX] == 6 &&
		    currentGoblin != 1) {
			pathExistence = 0;
			break;
		}

		if (_vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 3) {
			gobDesc->nextState = 8;
			break;
		}

		if (_vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 6 &&
		    currentGoblin == 1) {
			gobDesc->nextState = 28;
			break;
		}

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 2);
		break;

	case Map::kDirS:
		if (_vm->_map->passMap[_vm->_map->curGoblinY + 1][_vm->_map->curGoblinX] == 6 &&
		    currentGoblin != 1) {
			pathExistence = 0;
			break;
		}

		if (_vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 3) {
			gobDesc->nextState = 9;
			break;
		}

		if (_vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 6 &&
		    currentGoblin == 1) {
			gobDesc->nextState = 29;
			break;
		}

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 6);
		break;

	case Map::kDirSE:
		if (_vm->_map->passMap[_vm->_map->curGoblinY + 1][_vm->_map->curGoblinX + 1] == 6 &&
		    currentGoblin != 1) {
			pathExistence = 0;
			break;
		}

		gobDesc->nextState = 5;
		if (gobDesc->curLookDir == 4)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 4);
		break;

	case Map::kDirSW:
		if (_vm->_map->passMap[_vm->_map->curGoblinY + 1][_vm->_map->curGoblinX - 1] == 6 &&
		    currentGoblin != 1) {
			pathExistence = 0;
			break;
		}

		gobDesc->nextState = 7;
		if (gobDesc->curLookDir == 0)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 0);
		break;

	case Map::kDirNW:
		if (_vm->_map->passMap[_vm->_map->curGoblinY - 1][_vm->_map->curGoblinX - 1] == 6 &&
		    currentGoblin != 1) {
			pathExistence = 0;
			break;
		}

		gobDesc->nextState = 1;
		if (gobDesc->curLookDir == 0)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 0);
		break;

	case Map::kDirNE:
		if (_vm->_map->passMap[_vm->_map->curGoblinY - 1][_vm->_map->curGoblinX + 1] == 6 &&
		    currentGoblin != 1) {
			pathExistence = 0;
			break;
		}

		gobDesc->nextState = 3;
		if (gobDesc->curLookDir == 4)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 4);
		break;

	case 0x4dc8:

		if (currentGoblin == 0 && gobAction == 3
		    && itemIndInPocket == -1) {
			destItemId = -1;
			readyToAct = 0;
			break;
		}

		if (currentGoblin == 0 && gobAction == 4 &&
		    itemIndInPocket == -1 && destActionItem == 0) {
			gobDesc->multState = 104;
			destItemId = -1;
			readyToAct = 0;
			break;
		}

		if (currentGoblin == 0 && gobAction == 4 &&
		    itemIndInPocket == -1 && destActionItem != 0 &&
		    itemToObject[destActionItem] != -1 &&
		    objects[itemToObject[destActionItem]]->
		    pickable == 0) {
			gobDesc->multState = 104;
			destItemId = -1;
			readyToAct = 0;
			break;
		}

		switch (_vm->_map->itemPoses[destActionItem].orient) {
		case 0:
		case -4:
			gobDesc->nextState = 10;
			gobDesc->curLookDir = 0;
			destItemId = -1;
			break;

		case -1:
		case 4:
			gobDesc->nextState = 11;
			gobDesc->curLookDir = 4;
			destItemId = -1;
			break;
		}
		break;

	default:
		if (_vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 3 ||
		    (_vm->_map->passMap[_vm->_map->curGoblinY][_vm->_map->curGoblinX] == 6
			&& currentGoblin == 1)) {
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

void Goblin::moveAdvance(Gob_Object *gobDesc, int16 nextAct, int16 framesCount) {
	int16 i;
	int16 newX;
	int16 newY;
	int16 flag;

	movePathFind(gobDesc, nextAct);

	gobDesc->curFrame++;
	if (gobDesc->curFrame == 1)
		gobDesc->actionStartState = gobDesc->state;

	if (goesAtTarget == 0
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
			if (currentGoblin != 1)
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
			if (currentGoblin != 1)
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
		_vm->_snd->speakerOn(10 * _vm->_util->getRandom(3) + 50, 5);
	}

	if (currentGoblin == 0
	    && gobDesc->stateMach == gobDesc->realStateMach
	    && (gobDesc->state == 10 || gobDesc->state == 11)
	    && gobDesc->curFrame == 9) {
		_vm->_snd->stopSound(0);
		if (itemIndInPocket != -1) {
			_vm->_snd->playSample(soundData[14], 1, 9000);
		}

		if (itemIndInPocket == -1) {
			_vm->_snd->playSample(soundData[14], 1, 5000);
		}
	}

	if (boreCounter++ == 120) {
		boreCounter = 0;
		for (i = 0; i < 3; i++)
			showBoredom(i);
	}

	if (gobDesc->multState != -1 && gobDesc->curFrame == framesCount &&
	    gobDesc->state != gobDesc->multState) {
		gobDesc->nextState = gobDesc->multState;
		gobDesc->multState = -1;

		newX =
		    _vm->_scenery->animations[gobDesc->animation].
		    layers[gobStateLayer]->animDeltaX + gobDesc->xPos;

		newY =
		    _vm->_scenery->animations[gobDesc->animation].
		    layers[gobStateLayer]->animDeltaY + gobDesc->yPos;

		gobStateLayer = nextLayer(gobDesc);

		gobDesc->xPos = newX;
		gobDesc->yPos = newY;
	} else {
		if (gobDesc->curFrame == 3 &&
		    gobDesc->stateMach == gobDesc->realStateMach &&
		    (gobDesc->state < 10 ||
			(currentGoblin == 1 && (gobDesc->state == 28
				|| gobDesc->state == 29))
		    )) {
			flag = 0;
			if (forceNextState[0] != -1) {
				gobDesc->nextState = forceNextState[0];
				for (i = 0; i < 9; i++)
					forceNextState[i] =
					    forceNextState[i + 1];
			}

			_vm->_map->curGoblinX = gobPositions[currentGoblin].x;
			_vm->_map->curGoblinY = gobPositions[currentGoblin].y;

			if (gobDesc->nextState != gobDesc->state) {
				gobStateLayer = nextLayer(gobDesc);
				flag = 1;
			}

			switch (gobDesc->state) {
			case 0:
				gobPositions[currentGoblin].x--;
				break;

			case 2:
			case 8:
				gobPositions[currentGoblin].y--;
				break;

			case 4:
				gobPositions[currentGoblin].x++;
				break;

			case 6:
			case 9:
				gobPositions[currentGoblin].y++;
				break;

			case 1:
				gobPositions[currentGoblin].x--;
				gobPositions[currentGoblin].y--;
				break;

			case 3:
				gobPositions[currentGoblin].x++;
				gobPositions[currentGoblin].y--;
				break;

			case 5:
				gobPositions[currentGoblin].x++;
				gobPositions[currentGoblin].y++;
				break;

			case 7:
				gobPositions[currentGoblin].x--;
				gobPositions[currentGoblin].y++;
				break;

			case 38:
				gobPositions[currentGoblin].y++;
				break;
			}

			if (currentGoblin == 1) {
				if (gobDesc->state == 28)
					gobPositions[1].y--;

				if (gobDesc->state == 29)
					gobPositions[1].y++;
			}

			if (flag != 0) {
				_vm->_scenery->updateAnim(gobStateLayer, 0,
				    gobDesc->animation, 0, gobDesc->xPos,
				    gobDesc->yPos, 0);

				gobDesc->yPos =
				    (_vm->_map->curGoblinY + 1) * 6 -
				    (_vm->_scenery->toRedrawBottom - _vm->_scenery->animTop);
				gobDesc->xPos =
				    _vm->_map->curGoblinX * 12 - (_vm->_scenery->toRedrawLeft -
				    _vm->_scenery->animLeft);
			}

			if ((gobDesc->state == 10 || gobDesc->state == 11)
			    && currentGoblin != 0)
				goesAtTarget = 1;
		}

		if (gobDesc->curFrame != framesCount)
			return;

		if (forceNextState[0] != -1) {
			gobDesc->nextState = forceNextState[0];
			for (i = 0; i < 10; i++)
				forceNextState[i] =
				    forceNextState[i + 1];
		}

		_vm->_map->curGoblinX = gobPositions[currentGoblin].x;
		_vm->_map->curGoblinY = gobPositions[currentGoblin].y;

		gobStateLayer = nextLayer(gobDesc);
		if (gobDesc->stateMach == gobDesc->realStateMach) {

			switch (gobDesc->nextState) {
			case 0:
				gobPositions[currentGoblin].x--;
				break;

			case 2:
			case 8:
				gobPositions[currentGoblin].y--;
				break;

			case 4:
				gobPositions[currentGoblin].x++;
				break;

			case 6:
			case 9:
				gobPositions[currentGoblin].y++;
				break;

			case 1:
				gobPositions[currentGoblin].x--;
				gobPositions[currentGoblin].y--;
				break;

			case 3:
				gobPositions[currentGoblin].x++;
				gobPositions[currentGoblin].y--;
				break;

			case 5:
				gobPositions[currentGoblin].x++;
				gobPositions[currentGoblin].y++;
				break;

			case 7:
				gobPositions[currentGoblin].x--;
				gobPositions[currentGoblin].y++;
				break;

			case 38:
				gobPositions[currentGoblin].y++;
				break;
			}
			if (currentGoblin == 1) {
				if (gobDesc->nextState == 28)
					gobPositions[1].y--;

				if (gobDesc->nextState == 29)
					gobPositions[1].y++;
			}
		}

		_vm->_scenery->updateAnim(gobStateLayer, 0, gobDesc->animation, 0,
		    gobDesc->xPos, gobDesc->yPos, 0);

		gobDesc->yPos =
		    (_vm->_map->curGoblinY + 1) * 6 - (_vm->_scenery->toRedrawBottom -
		    _vm->_scenery->animTop);
		gobDesc->xPos =
		    _vm->_map->curGoblinX * 12 - (_vm->_scenery->toRedrawLeft - _vm->_scenery->animLeft);

		if ((gobDesc->state == 10 || gobDesc->state == 11)
		    && currentGoblin != 0)
			goesAtTarget = 1;
	}
	return;
}

int16 Goblin::doMove(Gob_Object *gobDesc, int16 cont, int16 action) {
	int16 framesCount;
	int16 nextAct;
	int16 gobIndex;
	int16 layer;

	nextAct = 0;
	gobIndex = 0;

	layer = gobDesc->stateMach[gobDesc->state][0]->layer;
	framesCount =
	    _vm->_scenery->animations[gobDesc->animation].layers[layer]->framesCount;

	if (VAR(59) == 0 &&
	    gobDesc->state != 30 && gobDesc->state != 31) {
		gobDesc->order = (gobDesc->bottom) / 24 + 3;
	}

	if (positionedGob != currentGoblin) {
		_vm->_map->curGoblinX = gobPositions[currentGoblin].x;
		_vm->_map->curGoblinY = gobPositions[currentGoblin].y;
	}

	positionedGob = currentGoblin;

	gobDesc->animation =
	    gobDesc->stateMach[gobDesc->state][gobDesc->stateColumn]->
	    animation;

	gobStateLayer =
	    gobDesc->stateMach[gobDesc->state][gobDesc->stateColumn]->layer;

	moveInitStep(framesCount, action, cont, gobDesc, &gobIndex,
	    &nextAct);
	moveTreatRopeStairs(gobDesc);
	moveAdvance(gobDesc, nextAct, framesCount);

	return gobIndex;
}

void Goblin::freeObjects(void) {
	int16 i;
	int16 state;
	int16 col;

	for (i = 0; i < 16; i++) {
		if (soundData[i] == 0)
			continue;

		_vm->_snd->freeSoundData(soundData[i]);
		soundData[i] = 0;
	}

	for (i = 0; i < 4; i++) {
		if (goblins[i] == 0)
			continue;

		goblins[i]->stateMach = goblins[i]->realStateMach;

		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				free(goblins[i]->stateMach[state][col]);
				goblins[i]->stateMach[state][col] = 0;
			}
		}

		if (i == 3) {
			for (state = 40; state < 70; state++) {
				free(goblins[3]->stateMach[state][0]);
				goblins[3]->stateMach[state][0] = 0;
			}
		}

		free(goblins[i]->stateMach);
		free(goblins[i]);
		goblins[i] = 0;
	}

	for (i = 0; i < 20; i++) {
		if (objects[i] == 0)
			continue;

		objects[i]->stateMach = objects[i]->realStateMach;

		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				free(objects[i]->stateMach[state][col]);
				objects[i]->stateMach[state][col] = 0;
			}
		}

		free(objects[i]->stateMach);
		free(objects[i]);
		objects[i] = 0;
	}
}

void Goblin::zeroObjects(void) {
	int16 i;

	for (i = 0; i < 4; i++)
		goblins[i] = 0;

	for (i = 0; i < 20; i++)
		objects[i] = 0;

	for (i = 0; i < 16; i++)
		soundData[i] = 0;
}

void Goblin::freeAllObjects(void) {
	_vm->_util->deleteList(objList);
	freeObjects();
}

void Goblin::loadObjects(char *source) {
	int16 i;

	zeroObjects();
	for (i = 0; i < 20; i++)
		itemToObject[i] = 100;

	freeObjects();
	initList();
	strcpy(_vm->_map->sourceFile, source);

	_vm->_map->sourceFile[strlen(_vm->_map->sourceFile) - 4] = 0;
	_vm->_map->loadMapObjects(source);

	for (i = 0; i < gobsCount; i++)
		placeObject(goblins[i], 0);

	for (i = 0; i < objCount; i++) {
		placeObject(objects[i], 1);
	}

	initVarPointers();
	actDestItemDesc = 0;
}

void Goblin::saveGobDataToVars(int16 xPos, int16 yPos, int16 someVal) {
	Gob_Object *obj;
	*some0ValPtr = someVal;
	*curGobXPosVarPtr = xPos;
	*curGobYPosVarPtr = yPos;
	*itemInPocketVarPtr = itemIndInPocket;

	obj = goblins[currentGoblin];

	*curGobStateVarPtr = obj->state;
	*curGobFrameVarPtr = obj->curFrame;
	*curGobMultStateVarPtr = obj->multState;
	*curGobNextStateVarPtr = obj->nextState;
	*curGobScrXVarPtr = obj->xPos;
	*curGobScrYVarPtr = obj->yPos;
	*curGobLeftVarPtr = obj->left;
	*curGobTopVarPtr = obj->top;
	*curGobRightVarPtr = obj->right;
	*curGobBottomVarPtr = obj->bottom;
	*curGobDoAnimVarPtr = obj->doAnim;
	*curGobOrderVarPtr = obj->order;
	*curGobNoTickVarPtr = obj->noTick;
	*curGobTypeVarPtr = obj->type;
	*curGobMaxTickVarPtr = obj->maxTick;
	*curGobTickVarPtr = obj->tick;
	*curGobActStartStateVarPtr = obj->actionStartState;
	*curGobLookDirVarPtr = obj->curLookDir;
	*curGobPickableVarPtr = obj->pickable;
	*curGobRelaxVarPtr = obj->relaxTime;
	*curGobMaxFrameVarPtr = getObjMaxFrame(obj);

	if (actDestItemDesc == 0)
		return;

	obj = actDestItemDesc;
	*destItemStateVarPtr = obj->state;
	*destItemFrameVarPtr = obj->curFrame;
	*destItemMultStateVarPtr = obj->multState;
	*destItemNextStateVarPtr = obj->nextState;
	*destItemScrXVarPtr = obj->xPos;
	*destItemScrYVarPtr = obj->yPos;
	*destItemLeftVarPtr = obj->left;
	*destItemTopVarPtr = obj->top;
	*destItemRightVarPtr = obj->right;
	*destItemBottomVarPtr = obj->bottom;
	*destItemDoAnimVarPtr = obj->doAnim;
	*destItemOrderVarPtr = obj->order;
	*destItemNoTickVarPtr = obj->noTick;
	*destItemTypeVarPtr = obj->type;
	*destItemMaxTickVarPtr = obj->maxTick;
	*destItemTickVarPtr = obj->tick;
	*destItemActStartStVarPtr = obj->actionStartState;
	*destItemLookDirVarPtr = obj->curLookDir;
	*destItemPickableVarPtr = obj->pickable;
	*destItemRelaxVarPtr = obj->relaxTime;
	*destItemMaxFrameVarPtr = getObjMaxFrame(obj);

	destItemState = obj->state;
	destItemType = obj->type;
}

void Goblin::initVarPointers(void) {
	gobRetVarPtr = (int32 *)VAR_ADDRESS(59);
	curGobStateVarPtr = (int32 *)VAR_ADDRESS(60);
	curGobFrameVarPtr = (int32 *)VAR_ADDRESS(61);
	curGobMultStateVarPtr = (int32 *)VAR_ADDRESS(62);
	curGobNextStateVarPtr = (int32 *)VAR_ADDRESS(63);
	curGobScrXVarPtr = (int32 *)VAR_ADDRESS(64);
	curGobScrYVarPtr = (int32 *)VAR_ADDRESS(65);
	curGobLeftVarPtr = (int32 *)VAR_ADDRESS(66);
	curGobTopVarPtr = (int32 *)VAR_ADDRESS(67);
	curGobRightVarPtr = (int32 *)VAR_ADDRESS(68);
	curGobBottomVarPtr = (int32 *)VAR_ADDRESS(69);
	curGobDoAnimVarPtr = (int32 *)VAR_ADDRESS(70);
	curGobOrderVarPtr = (int32 *)VAR_ADDRESS(71);
	curGobNoTickVarPtr = (int32 *)VAR_ADDRESS(72);
	curGobTypeVarPtr = (int32 *)VAR_ADDRESS(73);
	curGobMaxTickVarPtr = (int32 *)VAR_ADDRESS(74);
	curGobTickVarPtr = (int32 *)VAR_ADDRESS(75);
	curGobActStartStateVarPtr = (int32 *)VAR_ADDRESS(76);
	curGobLookDirVarPtr = (int32 *)VAR_ADDRESS(77);
	curGobPickableVarPtr = (int32 *)VAR_ADDRESS(80);
	curGobRelaxVarPtr = (int32 *)VAR_ADDRESS(81);
	destItemStateVarPtr = (int32 *)VAR_ADDRESS(82);
	destItemFrameVarPtr = (int32 *)VAR_ADDRESS(83);
	destItemMultStateVarPtr = (int32 *)VAR_ADDRESS(84);
	destItemNextStateVarPtr = (int32 *)VAR_ADDRESS(85);
	destItemScrXVarPtr = (int32 *)VAR_ADDRESS(86);
	destItemScrYVarPtr = (int32 *)VAR_ADDRESS(87);
	destItemLeftVarPtr = (int32 *)VAR_ADDRESS(88);
	destItemTopVarPtr = (int32 *)VAR_ADDRESS(89);
	destItemRightVarPtr = (int32 *)VAR_ADDRESS(90);
	destItemBottomVarPtr = (int32 *)VAR_ADDRESS(91);
	destItemDoAnimVarPtr = (int32 *)VAR_ADDRESS(92);
	destItemOrderVarPtr = (int32 *)VAR_ADDRESS(93);
	destItemNoTickVarPtr = (int32 *)VAR_ADDRESS(94);
	destItemTypeVarPtr = (int32 *)VAR_ADDRESS(95);
	destItemMaxTickVarPtr = (int32 *)VAR_ADDRESS(96);
	destItemTickVarPtr = (int32 *)VAR_ADDRESS(97);
	destItemActStartStVarPtr = (int32 *)VAR_ADDRESS(98);
	destItemLookDirVarPtr = (int32 *)VAR_ADDRESS(99);
	destItemPickableVarPtr = (int32 *)VAR_ADDRESS(102);
	destItemRelaxVarPtr = (int32 *)VAR_ADDRESS(103);
	destItemMaxFrameVarPtr = (int32 *)VAR_ADDRESS(105);
	curGobVarPtr = (int32 *)VAR_ADDRESS(106);
	some0ValPtr = (int32 *)VAR_ADDRESS(107);
	curGobXPosVarPtr = (int32 *)VAR_ADDRESS(108);
	curGobYPosVarPtr = (int32 *)VAR_ADDRESS(109);
	curGobMaxFrameVarPtr = (int32 *)VAR_ADDRESS(110);

	itemInPocketVarPtr = (int32 *)VAR_ADDRESS(114);

	*itemInPocketVarPtr = -2;
}

void Goblin::loadGobDataFromVars(void) {
	Gob_Object *obj;

	itemIndInPocket = *itemInPocketVarPtr;

	obj = goblins[currentGoblin];

	obj->state = *curGobStateVarPtr;
	obj->curFrame = *curGobFrameVarPtr;
	obj->multState = *curGobMultStateVarPtr;
	obj->nextState = *curGobNextStateVarPtr;
	obj->xPos = *curGobScrXVarPtr;
	obj->yPos = *curGobScrYVarPtr;
	obj->left = *curGobLeftVarPtr;
	obj->top = *curGobTopVarPtr;
	obj->right = *curGobRightVarPtr;
	obj->bottom = *curGobBottomVarPtr;
	obj->doAnim = *curGobDoAnimVarPtr;
	obj->order = *curGobOrderVarPtr;
	obj->noTick = *curGobNoTickVarPtr;
	obj->type = *curGobTypeVarPtr;
	obj->maxTick = *curGobMaxTickVarPtr;
	obj->tick = *curGobTickVarPtr;
	obj->actionStartState = *curGobActStartStateVarPtr;
	obj->curLookDir = *curGobLookDirVarPtr;
	obj->pickable = *curGobPickableVarPtr;
	obj->relaxTime = *curGobRelaxVarPtr;

	if (actDestItemDesc == 0)
		return;

	obj = actDestItemDesc;

	obj->state = *destItemStateVarPtr;
	obj->curFrame = *destItemFrameVarPtr;
	obj->multState = *destItemMultStateVarPtr;
	obj->nextState = *destItemNextStateVarPtr;
	obj->xPos = *destItemScrXVarPtr;
	obj->yPos = *destItemScrYVarPtr;
	obj->left = *destItemLeftVarPtr;
	obj->top = *destItemTopVarPtr;
	obj->right = *destItemRightVarPtr;
	obj->bottom = *destItemBottomVarPtr;
	obj->doAnim = *destItemDoAnimVarPtr;
	obj->order = *destItemOrderVarPtr;
	obj->noTick = *destItemNoTickVarPtr;
	obj->type = *destItemTypeVarPtr;
	obj->maxTick = *destItemMaxTickVarPtr;
	obj->tick = *destItemTickVarPtr;
	obj->actionStartState = *destItemActStartStVarPtr;
	obj->curLookDir = *destItemLookDirVarPtr;
	obj->pickable = *destItemPickableVarPtr;
	obj->relaxTime = *destItemRelaxVarPtr;

	if (obj->type != destItemType)
		obj->toRedraw = 1;

	if (obj->state != destItemState && obj->type == 0)
		obj->toRedraw = 1;
}

void Goblin::pickItem(int16 indexToPocket, int16 idToPocket) {
	int16 x;
	int16 y;

	if (objects[indexToPocket]->pickable != 1)
		return;

	objects[indexToPocket]->type = 3;

	itemIndInPocket = indexToPocket;
	itemIdInPocket = idToPocket;

	for (y = 0; y < Map::kMapHeight; y++) {
		for (x = 0; x < Map::kMapWidth; x++) {
			if (itemByteFlag == 1) {
				if (((_vm->_map->itemsMap[y][x] & 0xff00) >> 8) ==
				    idToPocket)
					_vm->_map->itemsMap[y][x] &= 0xff;
			} else {
				if ((_vm->_map->itemsMap[y][x] & 0xff) == idToPocket)
					_vm->_map->itemsMap[y][x] &= 0xff00;
			}
		}
	}

	if (idToPocket >= 0 && idToPocket < 20) {
		_vm->_map->itemPoses[itemIdInPocket].x = 0;
		_vm->_map->itemPoses[itemIdInPocket].y = 0;
		_vm->_map->itemPoses[itemIdInPocket].orient = 0;
	}
}

void Goblin::placeItem(int16 indexInPocket, int16 idInPocket) {
	Gob_Object *itemDesc;
	int16 lookDir;
	int16 xPos;
	int16 yPos;
	int16 layer;

	itemDesc = objects[indexInPocket];
	lookDir = goblins[0]->curLookDir & 4;

	xPos = gobPositions[0].x;
	yPos = gobPositions[0].y;

	itemIndInPocket = -1;
	itemIdInPocket = 0;

	itemDesc->pickable = 1;
	itemDesc->type = 0;
	itemDesc->toRedraw = 1;
	itemDesc->curFrame = 0;
	itemDesc->order = goblins[0]->order;
	itemDesc->animation =
	    itemDesc->stateMach[itemDesc->state][0]->animation;
	layer =
	    itemDesc->stateMach[itemDesc->state][itemDesc->stateColumn]->layer;

	_vm->_scenery->updateAnim(layer, 0, itemDesc->animation, 0,
	    itemDesc->xPos, itemDesc->yPos, 0);

	itemDesc->yPos +=
	    (gobPositions[0].y * 6) + 5 - _vm->_scenery->toRedrawBottom;

	if (lookDir == 4) {
		itemDesc->xPos += (gobPositions[0].x * 12 + 14)
		    - (_vm->_scenery->toRedrawLeft + _vm->_scenery->toRedrawRight) / 2;
	} else {
		itemDesc->xPos += (gobPositions[0].x * 12)
		    - (_vm->_scenery->toRedrawLeft + _vm->_scenery->toRedrawRight) / 2;
	}

	_vm->_map->placeItem(xPos, yPos, idInPocket);

	if (yPos > 0) {
		_vm->_map->placeItem(xPos, yPos - 1, idInPocket);
	}

	if (lookDir == 4) {
		if (xPos < Map::kMapWidth - 1) {
			_vm->_map->placeItem(xPos + 1, yPos, idInPocket);

			if (yPos > 0) {
				_vm->_map->placeItem(xPos + 1, yPos - 1, idInPocket);
			}
		}
	} else {
		if (xPos > 0) {
			_vm->_map->placeItem(xPos - 1, yPos, idInPocket);

			if (yPos > 0) {
				_vm->_map->placeItem(xPos - 1, yPos - 1, idInPocket);
			}
		}
	}

	if (idInPocket >= 0 && idInPocket < 20) {
		_vm->_map->itemPoses[idInPocket].x = gobPositions[0].x;
		_vm->_map->itemPoses[idInPocket].y = gobPositions[0].y;
		_vm->_map->itemPoses[idInPocket].orient = lookDir;
		if (_vm->_map->itemPoses[idInPocket].orient == 0) {
//                      _vm->_map->itemPoses[idInPocket].x++;
			if (_vm->_map->passMap[(int)_vm->_map->itemPoses[idInPocket].y][_vm->_map->itemPoses[idInPocket].x + 1] == 1)
				_vm->_map->itemPoses[idInPocket].x++;
		} else {
			if (_vm->_map->passMap[(int)_vm->_map->itemPoses[idInPocket].y][_vm->_map->itemPoses[idInPocket].x - 1] == 1)
				_vm->_map->itemPoses[idInPocket].x--;
		}
	}
}

void Goblin::swapItems(int16 indexToPick, int16 idToPick) {
	int16 layer;
	Gob_Object *pickObj;
	Gob_Object *placeObj;
	int16 idToPlace;
	int16 x;
	int16 y;

	pickObj = objects[indexToPick];
	placeObj = objects[itemIndInPocket];

	idToPlace = itemIdInPocket;
	pickObj->type = 3;
	itemIndInPocket = indexToPick;
	itemIdInPocket = idToPick;

	if (itemByteFlag == 0) {
		for (y = 0; y < Map::kMapHeight; y++) {
			for (x = 0; x < Map::kMapWidth; x++) {
				if ((_vm->_map->itemsMap[y][x] & 0xff) == idToPick)
					_vm->_map->itemsMap[y][x] =
					    (_vm->_map->itemsMap[y][x] & 0xff00) +
					    idToPlace;
			}
		}
	} else {

		for (y = 0; y < Map::kMapHeight; y++) {
			for (x = 0; x < Map::kMapWidth; x++) {
				if (((_vm->_map->itemsMap[y][x] & 0xff00) >> 8) ==
				    idToPick)
					_vm->_map->itemsMap[y][x] =
					    (_vm->_map->itemsMap[y][x] & 0xff) +
					    (idToPlace << 8);
			}
		}
	}

	if (idToPick >= 0 && idToPick < 20) {
		_vm->_map->itemPoses[idToPlace].x =
		    _vm->_map->itemPoses[itemIdInPocket].x;
		_vm->_map->itemPoses[idToPlace].y =
		    _vm->_map->itemPoses[itemIdInPocket].y;
		_vm->_map->itemPoses[idToPlace].orient =
		    _vm->_map->itemPoses[itemIdInPocket].orient;

		_vm->_map->itemPoses[itemIdInPocket].x = 0;
		_vm->_map->itemPoses[itemIdInPocket].y = 0;
		_vm->_map->itemPoses[itemIdInPocket].orient = 0;
	}

	itemIndInPocket = -1;
	itemIdInPocket = 0;

	placeObj->type = 0;
	placeObj->nextState = -1;
	placeObj->multState = -1;
	placeObj->unk14 = 0;
	placeObj->toRedraw = 1;
	placeObj->curFrame = 0;
	placeObj->order = goblins[0]->order;

	placeObj->animation =
	    placeObj->stateMach[placeObj->state][0]->animation;

	layer =
	    placeObj->stateMach[placeObj->state][placeObj->stateColumn]->layer;
	_vm->_scenery->updateAnim(layer, 0, placeObj->animation, 0, placeObj->xPos,
	    placeObj->yPos, 0);

	placeObj->yPos +=
	    (gobPositions[0].y * 6) + 5 - _vm->_scenery->toRedrawBottom;

	if (_vm->_map->itemPoses[idToPlace].orient == 4) {
		placeObj->xPos += (gobPositions[0].x * 12 + 14)
		    - (_vm->_scenery->toRedrawLeft + _vm->_scenery->toRedrawRight) / 2;
	} else {
		placeObj->xPos += (gobPositions[0].x * 12)
		    - (_vm->_scenery->toRedrawLeft + _vm->_scenery->toRedrawRight) / 2;
	}
}

void Goblin::treatItemPick(int16 itemId) {
	int16 itemIndex;
	Gob_Object *gobDesc;

	gobDesc = goblins[currentGoblin];

	if (gobDesc->curFrame != 9)
		return;

	if (gobDesc->stateMach != gobDesc->realStateMach)
		return;

	readyToAct = 0;
	goesAtTarget = 0;

	itemIndex = itemToObject[itemId];
	if (itemId != 0 && itemIndex != -1
	    && objects[itemIndex]->pickable != 1)
		itemIndex = -1;

	if (itemIndInPocket != -1 && itemIndInPocket == itemIndex)
		itemIndex = -1;

	if (itemIndInPocket != -1 && itemIndex != -1
	    && objects[itemIndex]->pickable == 1) {
		swapItems(itemIndex, itemId);
		itemIndInPocket = itemIndex;
		itemIdInPocket = itemId;
		return;
	}

	if (itemIndInPocket != -1 && itemIndex == -1) {
		placeItem(itemIndInPocket, itemIdInPocket);
		return;
	}

	if (itemIndInPocket == -1 && itemIndex != -1) {
		pickItem(itemIndex, itemId);
		return;
	}
}

int16 Goblin::treatItem(int16 action) {
	int16 state;

	state = goblins[currentGoblin]->state;
	if ((state == 10 || state == 11) &&
	    goblins[currentGoblin]->curFrame == 0) {
		readyToAct = 0;
	}

	if (action == 3 && currentGoblin == 0 &&
	    (state == 10 || state == 11) && goblins[0]->curFrame == 0) {
		saveGobDataToVars(gobPositions[currentGoblin].x,
		    gobPositions[currentGoblin].y, 0);
		goesAtTarget = 1;
		return -1;
	}

	if (noPick == 0 && currentGoblin == 0 &&
	    (state == 10 || state == 11)) {
		treatItemPick(destActionItem);

		saveGobDataToVars(gobPositions[currentGoblin].x,
		    gobPositions[currentGoblin].y, 0);
		return 0;
	}

	if (goesAtTarget == 0) {
		saveGobDataToVars(gobPositions[currentGoblin].x,
		    gobPositions[currentGoblin].y, 0);
		return 0;
	} else {

		if (itemToObject[destActionItem] != 100 &&
		    destActionItem != 0) {

			if (itemToObject[destActionItem] == -1) {
				actDestItemDesc = 0;
			} else {
				actDestItemDesc =
				    objects[itemToObject
				    [destActionItem]];
			}
		}

		goesAtTarget = 0;
		saveGobDataToVars(gobPositions[currentGoblin].x,
		    gobPositions[currentGoblin].y, 0);
		return destActionItem;
	}
}

void Goblin::interFunc(void) {
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
	bool objDescSet = false;

	retVarPtr = (int32 *)VAR_ADDRESS(59);

	cmd = _vm->_inter->load16();
	_vm->_global->inter_execPtr += 2;
	if (cmd > 0 && cmd < 17) {
		extraData = _vm->_inter->load16();
		objDesc = objects[extraData];
		objDescSet = true;
		extraData = _vm->_inter->load16();
	}

	if (cmd > 90 && cmd < 107) {
		extraData = _vm->_inter->load16();
		objDesc = goblins[extraData];
		objDescSet = true;
		extraData = _vm->_inter->load16();
		cmd -= 90;
	}

	if (cmd > 110 && cmd < 128) {
		extraData = _vm->_inter->load16();
		objDesc = goblins[extraData];
		objDescSet = true;
		cmd -= 90;
	} else if (cmd > 20 && cmd < 38) {
		extraData = _vm->_inter->load16();
		objDesc = objects[extraData];
		objDescSet = true;
	}

/*
	NB: The original gobliiins engine did not initialize the objDesc
	variable, so we manually check if objDesc is properly set before
	checking if it is zero. If it was not set, we do not return. This
	fixes a crash in the EGA version if the life bar is depleted, because
	interFunc is called multiple times with cmd == 39.
	Bug #1324814
*/


	if (cmd < 40 && objDescSet && objDesc == 0)
		return;

	debug(5, "cmd = %d", cmd);
	switch (cmd) {
	case 1:
		objDesc->state = extraData;

		if (objDesc == actDestItemDesc)
			*destItemStateVarPtr = extraData;
		break;

	case 2:
		objDesc->curFrame = extraData;
		if (objDesc == actDestItemDesc)
			*destItemFrameVarPtr = extraData;
		break;

	case 3:
		objDesc->nextState = extraData;
		if (objDesc == actDestItemDesc)
			*destItemNextStateVarPtr = extraData;
		break;

	case 4:
		objDesc->multState = extraData;
		if (objDesc == actDestItemDesc)
			*destItemMultStateVarPtr = extraData;
		break;

	case 5:
		objDesc->order = extraData;
		if (objDesc == actDestItemDesc)
			*destItemOrderVarPtr = extraData;
		break;

	case 6:
		objDesc->actionStartState = extraData;
		if (objDesc == actDestItemDesc)
			*destItemActStartStVarPtr = extraData;
		break;

	case 7:
		objDesc->curLookDir = extraData;
		if (objDesc == actDestItemDesc)
			*destItemLookDirVarPtr = extraData;
		break;

	case 8:
		objDesc->type = extraData;
		if (objDesc == actDestItemDesc)
			*destItemTypeVarPtr = extraData;

		if (extraData == 0)
			objDesc->toRedraw = 1;
		break;

	case 9:
		objDesc->noTick = extraData;
		if (objDesc == actDestItemDesc)
			*destItemNoTickVarPtr = extraData;
		break;

	case 10:
		objDesc->pickable = extraData;
		if (objDesc == actDestItemDesc)
			*destItemPickableVarPtr = extraData;
		break;

	case 12:
		objDesc->xPos = extraData;
		if (objDesc == actDestItemDesc)
			*destItemScrXVarPtr = extraData;
		break;

	case 13:
		objDesc->yPos = extraData;
		if (objDesc == actDestItemDesc)
			*destItemScrYVarPtr = extraData;
		break;

	case 14:
		objDesc->doAnim = extraData;
		if (objDesc == actDestItemDesc)
			*destItemDoAnimVarPtr = extraData;
		break;

	case 15:
		objDesc->relaxTime = extraData;
		if (objDesc == actDestItemDesc)
			*destItemRelaxVarPtr = extraData;
		break;

	case 16:
		objDesc->maxTick = extraData;
		if (objDesc == actDestItemDesc)
			*destItemMaxTickVarPtr = extraData;
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
		*retVarPtr = getObjMaxFrame(objDesc);
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
		xPos = _vm->_inter->load16();
		yPos = _vm->_inter->load16();
		item = _vm->_inter->load16();

		if (cmd == 42) {
			xPos = VAR(xPos);
			yPos = VAR(yPos);
			item = VAR(item);
		}

		for (y = 0; y < Map::kMapHeight; y++) {
			for (x = 0; x < Map::kMapWidth; x++) {
				if ((_vm->_map->itemsMap[y][x] & 0xff) == item) {
					_vm->_map->itemsMap[y][x] &= 0xff00;
				} else if (((_vm->_map->itemsMap[y][x] & 0xff00) >> 8)
				    == item) {
					_vm->_map->itemsMap[y][x] &= 0xff;
				}
			}
		}

		if (xPos < Map::kMapWidth - 1) {
			if (yPos > 0) {
				if ((_vm->_map->itemsMap[yPos][xPos] & 0xff00) != 0 ||
				    (_vm->_map->itemsMap[yPos - 1][xPos] & 0xff00) !=
				    0
				    || (_vm->_map->itemsMap[yPos][xPos +
					    1] & 0xff00) != 0
				    || (_vm->_map->itemsMap[yPos - 1][xPos +
					    1] & 0xff00) != 0) {

					_vm->_map->itemsMap[yPos][xPos] =
					    (_vm->_map->itemsMap[yPos][xPos] & 0xff00)
					    + item;

					_vm->_map->itemsMap[yPos - 1][xPos] =
					    (_vm->_map->itemsMap[yPos -
						1][xPos] & 0xff00) + item;

					_vm->_map->itemsMap[yPos][xPos + 1] =
					    (_vm->_map->itemsMap[yPos][xPos +
						1] & 0xff00) + item;

					_vm->_map->itemsMap[yPos - 1][xPos + 1] =
					    (_vm->_map->itemsMap[yPos - 1][xPos +
						1] & 0xff00) + item;
				} else {
					_vm->_map->itemsMap[yPos][xPos] =
					    (_vm->_map->itemsMap[yPos][xPos] & 0xff) +
					    (item << 8);

					_vm->_map->itemsMap[yPos - 1][xPos] =
					    (_vm->_map->itemsMap[yPos -
						1][xPos] & 0xff) + (item << 8);

					_vm->_map->itemsMap[yPos][xPos + 1] =
					    (_vm->_map->itemsMap[yPos][xPos +
						1] & 0xff) + (item << 8);

					_vm->_map->itemsMap[yPos - 1][xPos + 1] =
					    (_vm->_map->itemsMap[yPos - 1][xPos +
						1] & 0xff) + (item << 8);
				}
			} else {
				if ((_vm->_map->itemsMap[yPos][xPos] & 0xff00) != 0 ||
				    (_vm->_map->itemsMap[yPos][xPos + 1] & 0xff00) !=
				    0) {
					_vm->_map->itemsMap[yPos][xPos] =
					    (_vm->_map->itemsMap[yPos][xPos] & 0xff00)
					    + item;

					_vm->_map->itemsMap[yPos][xPos + 1] =
					    (_vm->_map->itemsMap[yPos][xPos +
						1] & 0xff00) + item;
				} else {
					_vm->_map->itemsMap[yPos][xPos] =
					    (_vm->_map->itemsMap[yPos][xPos] & 0xff) +
					    (item << 8);

					_vm->_map->itemsMap[yPos][xPos + 1] =
					    (_vm->_map->itemsMap[yPos][xPos +
						1] & 0xff) + (item << 8);
				}
			}
		} else {
			if (yPos > 0) {
				if ((_vm->_map->itemsMap[yPos][xPos] & 0xff00) != 0 ||
				    (_vm->_map->itemsMap[yPos - 1][xPos] & 0xff00) !=
				    0) {
					_vm->_map->itemsMap[yPos][xPos] =
					    (_vm->_map->itemsMap[yPos][xPos] & 0xff00)
					    + item;

					_vm->_map->itemsMap[yPos - 1][xPos] =
					    (_vm->_map->itemsMap[yPos -
						1][xPos] & 0xff00) + item;
				} else {
					_vm->_map->itemsMap[yPos][xPos] =
					    (_vm->_map->itemsMap[yPos][xPos] & 0xff) +
					    (item << 8);

					_vm->_map->itemsMap[yPos - 1][xPos] =
					    (_vm->_map->itemsMap[yPos -
						1][xPos] & 0xff) + (item << 8);
				}
			} else {
				if ((_vm->_map->itemsMap[yPos][xPos] & 0xff00) != 0) {
					_vm->_map->itemsMap[yPos][xPos] =
					    (_vm->_map->itemsMap[yPos][xPos] & 0xff00)
					    + item;
				} else {
					_vm->_map->itemsMap[yPos][xPos] =
					    (_vm->_map->itemsMap[yPos][xPos] & 0xff) +
					    (item << 8);
				}
			}
		}

		if (item < 0 || item >= 20)
			break;

		if (xPos > 1 && _vm->_map->passMap[yPos][xPos - 2] == 1) {
			_vm->_map->itemPoses[item].x = xPos - 2;
			_vm->_map->itemPoses[item].y = yPos;
			_vm->_map->itemPoses[item].orient = 4;
			break;
		}

		if (xPos < Map::kMapWidth - 2 && _vm->_map->passMap[yPos][xPos + 2] == 1) {
			_vm->_map->itemPoses[item].x = xPos + 2;
			_vm->_map->itemPoses[item].y = yPos;
			_vm->_map->itemPoses[item].orient = 0;
			break;
		}

		if (xPos < Map::kMapWidth - 1 && _vm->_map->passMap[yPos][xPos + 1] == 1) {
			_vm->_map->itemPoses[item].x = xPos + 1;
			_vm->_map->itemPoses[item].y = yPos;
			_vm->_map->itemPoses[item].orient = 0;
			break;
		}

		if (xPos > 0 && _vm->_map->passMap[yPos][xPos - 1] == 1) {
			_vm->_map->itemPoses[item].x = xPos - 1;
			_vm->_map->itemPoses[item].y = yPos;
			_vm->_map->itemPoses[item].orient = 4;
			break;
		}
		break;

	case 41:
	case 43:
		xPos = _vm->_inter->load16();
		yPos = _vm->_inter->load16();

		if (cmd == 43) {
			xPos = VAR(xPos);
			yPos = VAR(yPos);
		}

		if ((_vm->_map->itemsMap[yPos][xPos] & 0xff00) != 0) {
			*retVarPtr = (_vm->_map->itemsMap[yPos][xPos] & 0xff00) >> 8;
		} else {
			*retVarPtr = _vm->_map->itemsMap[yPos][xPos];
		}
		break;

	case 44:
		xPos = _vm->_inter->load16();
		yPos = _vm->_inter->load16();
		val = _vm->_inter->load16();
		_vm->_map->passMap[yPos][xPos] = val;
		break;

	case 50:
		item = _vm->_inter->load16();
		xPos = _vm->_inter->load16();
		yPos = _vm->_inter->load16();

		gobPositions[item].x = xPos * 2;
		gobPositions[item].y = yPos * 2;

		objDesc = goblins[item];
		objDesc->nextState = 21;

		nextLayer(objDesc);

		layer = objDesc->stateMach[objDesc->state][0]->layer;

		_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->yPos =
		    (gobPositions[item].y * 6 + 6) - (_vm->_scenery->toRedrawBottom -
		    _vm->_scenery->animTop);
		objDesc->xPos =
		    gobPositions[item].x * 12 - (_vm->_scenery->toRedrawLeft -
		    _vm->_scenery->animLeft);

		objDesc->curFrame = 0;
		objDesc->state = 21;
		if (currentGoblin == item) {
			*curGobScrXVarPtr = objDesc->xPos;
			*curGobScrYVarPtr = objDesc->yPos;

			*curGobFrameVarPtr = 0;
			*curGobStateVarPtr = 18;
			pressedMapX = gobPositions[item].x;
			pressedMapY = gobPositions[item].y;
		}
		break;

	case 52:
		item = _vm->_inter->load16();
		*retVarPtr = gobPositions[item].x >> 1;
		break;

	case 53:
		item = _vm->_inter->load16();
		*retVarPtr = gobPositions[item].y >> 1;
		break;

	case 150:
		item = _vm->_inter->load16();
		xPos = _vm->_inter->load16();
		yPos = _vm->_inter->load16();

		objDesc = goblins[item];
		if (yPos == 0) {
			objDesc->multState = xPos;
			objDesc->nextState = xPos;
			nextLayer(objDesc);

			layer = objDesc->stateMach[objDesc->state][0]->layer;

			objDesc->xPos =
			    _vm->_scenery->animations[objDesc->animation].layers[layer]->
			    posX;
			objDesc->yPos =
			    _vm->_scenery->animations[objDesc->animation].layers[layer]->
			    posY;

			*curGobScrXVarPtr = objDesc->xPos;
			*curGobScrYVarPtr = objDesc->yPos;
			*curGobFrameVarPtr = 0;
			*curGobStateVarPtr = objDesc->state;
			*curGobNextStateVarPtr = objDesc->nextState;
			*curGobMultStateVarPtr = objDesc->multState;
			*curGobMaxFrameVarPtr =
			    getObjMaxFrame(objDesc);
			noPick = 1;
			break;
		}

		objDesc->multState = 21;
		objDesc->nextState = 21;
		objDesc->state = 21;
		nextLayer(objDesc);
		layer = objDesc->stateMach[objDesc->state][0]->layer;

		_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->yPos =
		    (yPos * 6 + 6) - (_vm->_scenery->toRedrawBottom - _vm->_scenery->animTop);
		objDesc->xPos =
		    xPos * 12 - (_vm->_scenery->toRedrawLeft - _vm->_scenery->animLeft);

		gobPositions[item].x = xPos;
		pressedMapX = xPos;
		_vm->_map->curGoblinX = xPos;

		gobPositions[item].y = yPos;
		pressedMapY = yPos;
		_vm->_map->curGoblinY = yPos;

		*curGobScrXVarPtr = objDesc->xPos;
		*curGobScrYVarPtr = objDesc->yPos;
		*curGobFrameVarPtr = 0;
		*curGobStateVarPtr = 21;
		*curGobNextStateVarPtr = 21;
		*curGobMultStateVarPtr = -1;
		noPick = 0;
		break;

	case 250:
		item = _vm->_inter->load16();
		xPos = _vm->_inter->load16();
		yPos = _vm->_inter->load16();

		gobPositions[item].x = xPos;
		gobPositions[item].y = yPos;

		objDesc = goblins[item];
		objDesc->nextState = 21;
		nextLayer(objDesc);

		layer = objDesc->stateMach[objDesc->state][0]->layer;

		_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->yPos =
		    (yPos * 6 + 6) - (_vm->_scenery->toRedrawBottom - _vm->_scenery->animTop);
		objDesc->xPos =
		    xPos * 12 - (_vm->_scenery->toRedrawLeft - _vm->_scenery->animLeft);

		objDesc->curFrame = 0;
		objDesc->state = 21;

		if (currentGoblin == item) {
			*curGobScrXVarPtr = objDesc->xPos;
			*curGobScrYVarPtr = objDesc->yPos;
			*curGobFrameVarPtr = 0;
			*curGobStateVarPtr = 18;

			pressedMapX = gobPositions[item].x;
			pressedMapY = gobPositions[item].y;
		}
		break;

	case 251:
		item = _vm->_inter->load16();
		state = _vm->_inter->load16();

		objDesc = goblins[item];
		objDesc->nextState = state;

		nextLayer(objDesc);
		layer = objDesc->stateMach[objDesc->state][0]->layer;

		objDesc->xPos =
		    _vm->_scenery->animations[objDesc->animation].layers[layer]->posX;
		objDesc->yPos =
		    _vm->_scenery->animations[objDesc->animation].layers[layer]->posY;

		if (item == currentGoblin) {
			*curGobScrXVarPtr = objDesc->xPos;
			*curGobScrYVarPtr = objDesc->yPos;
			*curGobFrameVarPtr = 0;
			*curGobStateVarPtr = objDesc->state;
			*curGobMultStateVarPtr = objDesc->multState;
		}
		break;

	case 252:
		item = _vm->_inter->load16();
		state = _vm->_inter->load16();
		objDesc = objects[item];

		objDesc->nextState = state;

		nextLayer(objDesc);
		layer = objDesc->stateMach[objDesc->state][0]->layer;
		objDesc->xPos =
		    _vm->_scenery->animations[objDesc->animation].layers[layer]->posX;
		objDesc->yPos =
		    _vm->_scenery->animations[objDesc->animation].layers[layer]->posY;

		objDesc->toRedraw = 1;
		objDesc->type = 0;
		if (objDesc == actDestItemDesc) {
			*destItemScrXVarPtr = objDesc->xPos;
			*destItemScrYVarPtr = objDesc->yPos;

			*destItemStateVarPtr = objDesc->state;
			*destItemNextStateVarPtr = -1;
			*destItemMultStateVarPtr = -1;
			*destItemFrameVarPtr = 0;
		}
		break;

	case 152:
		item = _vm->_inter->load16();
		val = _vm->_inter->load16();
		objDesc = objects[item];
		objDesc->unk14 = val;
		break;

	case 200:
		itemIdInPocket = _vm->_inter->load16();
		break;

	case 201:
		itemIndInPocket = _vm->_inter->load16();
		break;

	case 202:
		*retVarPtr = itemIdInPocket;
		break;

	case 203:
		*retVarPtr = itemIndInPocket;
		break;

	case 204:
		item = _vm->_inter->load16();
		xPos = _vm->_inter->load16();
		yPos = _vm->_inter->load16();
		val = _vm->_inter->load16();

		_vm->_map->itemPoses[item].x = xPos;
		_vm->_map->itemPoses[item].y = yPos;
		_vm->_map->itemPoses[item].orient = val;
		break;

	case 500:
		extraData = _vm->_inter->load16();
		objDesc = objects[extraData];

		objDesc->relaxTime--;
		if (objDesc->relaxTime < 0 &&
		    getObjMaxFrame(objDesc) == objDesc->curFrame) {
			objDesc->relaxTime = _vm->_util->getRandom(100) + 50;
			objDesc->curFrame = 0;
			objDesc->toRedraw = 1;
		}
		break;

	case 502:
		item = _vm->_inter->load16();
		*retVarPtr = gobPositions[item].x;
		break;

	case 503:
		item = _vm->_inter->load16();
		*retVarPtr = gobPositions[item].y;
		break;

	case 600:
		pathExistence = 0;
		break;

	case 601:
		extraData = _vm->_inter->load16();
		goblins[extraData]->visible = 1;
		break;

	case 602:
		extraData = _vm->_inter->load16();
		goblins[extraData]->visible = 0;
		break;

	case 603:
		extraData = _vm->_inter->load16();
		item = _vm->_inter->load16();

		objDesc = objects[extraData];
		if (objIntersected(objDesc, goblins[item]) != 0)
			*retVarPtr = 1;
		else
			*retVarPtr = 0;
		break;

	case 604:
		extraData = _vm->_inter->load16();
		item = _vm->_inter->load16();

		objDesc = goblins[extraData];
		if (objIntersected(objDesc, goblins[item]) != 0)
			*retVarPtr = 1;
		else
			*retVarPtr = 0;
		break;

	case 605:
		item = _vm->_inter->load16();
		xPos = _vm->_inter->load16();
		yPos = _vm->_inter->load16();
		val = _vm->_inter->load16();

		_vm->_map->itemPoses[item].x = xPos;
		_vm->_map->itemPoses[item].y = yPos;
		_vm->_map->itemPoses[item].orient = val;
		break;

	case 1000:
		extraData = _vm->_inter->load16();
		if (_vm->_game->extHandle >= 0)
			_vm->_dataio->closeData(_vm->_game->extHandle);

		loadObjects((char *)VAR_ADDRESS(extraData));
		_vm->_game->extHandle = _vm->_dataio->openData(_vm->_game->curExtFile);
		break;

	case 1001:
		freeAllObjects();
		break;

	case 1002:
		animateObjects();
		break;

	case 1003:
		drawObjects();

		if (_vm->_cdrom->getTrackPos() == -1)
			_vm->_cdrom->playBgMusic();
		break;

	case 1004:
		_vm->_map->loadMapsInitGobs();
		break;

	case 1005:
		extraData = _vm->_inter->load16();
		xPos = _vm->_inter->load16();

		if ((uint16)VAR(xPos) == 0) {
			item =
			    doMove(goblins[currentGoblin], 1,
			    (uint16)VAR(extraData));
		} else {
			item =
			    doMove(goblins[currentGoblin], 1, 3);
		}

		if (item != 0)
			switchGoblin(item);
		break;

	case 1006:
		switchGoblin(0);
		break;

	case 1008:
		loadGobDataFromVars();
		break;

	case 1009:
		extraData = _vm->_inter->load16();
		cmd = _vm->_inter->load16();
		xPos = _vm->_inter->load16();

		if ((uint16)VAR(xPos) == 0) {
			WRITE_VAR(cmd, treatItem((uint16)VAR(extraData)));
			break;
		}

		WRITE_VAR(cmd, treatItem(3));
		break;

	case 1010:
		doMove(goblins[currentGoblin], 0, 0);
		break;

	case 1011:
		extraData = _vm->_inter->load16();
		if (VAR(extraData) != 0)
			goesAtTarget = 1;
		else
			goesAtTarget = 0;
		break;

	case 1015:
		extraData = _vm->_inter->load16();
		extraData = VAR(extraData);
		objects[10]->xPos = extraData;

		extraData = _vm->_inter->load16();
		extraData = VAR(extraData);
		objects[10]->yPos = extraData;
		break;

	case 2005:
		gobDesc = goblins[0];
		if (currentGoblin != 0) {
			goblins[currentGoblin]->doAnim = 1;
			goblins[currentGoblin]->nextState = 21;

			nextLayer(goblins[currentGoblin]);
			currentGoblin = 0;

			gobDesc->doAnim = 0;
			gobDesc->type = 0;
			gobDesc->toRedraw = 1;

			pressedMapX = gobPositions[0].x;
			_vm->_map->destX = gobPositions[0].x;
			gobDestX = gobPositions[0].x;

			pressedMapY = gobPositions[0].y;
			_vm->_map->destY = gobPositions[0].y;
			gobDestY = gobPositions[0].y;

			*curGobVarPtr = 0;
			pathExistence = 0;
			readyToAct = 0;
		}

		if (gobDesc->state != 10 && itemIndInPocket != -1 &&
		    getObjMaxFrame(gobDesc) == gobDesc->curFrame) {

			gobDesc->stateMach = gobDesc->realStateMach;
			xPos = gobPositions[0].x;
			yPos = gobPositions[0].y;

			gobDesc->nextState = 10;
			layer = nextLayer(gobDesc);

			_vm->_scenery->updateAnim(layer, 0, gobDesc->animation, 0,
			    gobDesc->xPos, gobDesc->yPos, 0);

			gobDesc->yPos =
			    (yPos * 6 + 6) - (_vm->_scenery->toRedrawBottom -
			    _vm->_scenery->animTop);
			gobDesc->xPos =
			    xPos * 12 - (_vm->_scenery->toRedrawLeft - _vm->_scenery->animLeft);
		}

		if (gobDesc->state != 10)
			break;

		if (itemIndInPocket == -1)
			break;

		if (gobDesc->curFrame != 10)
			break;

		objDesc = objects[itemIndInPocket];
		objDesc->type = 0;
		objDesc->toRedraw = 1;
		objDesc->curFrame = 0;

		objDesc->order = gobDesc->order;
		objDesc->animation =
		    objDesc->stateMach[objDesc->state][0]->animation;

		layer = objDesc->stateMach[objDesc->state][0]->layer;

		_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->yPos +=
		    (gobPositions[0].y * 6 + 5) - _vm->_scenery->toRedrawBottom;

		if (gobDesc->curLookDir == 4) {
			objDesc->xPos += gobPositions[0].x * 12 + 14
			    - (_vm->_scenery->toRedrawLeft + _vm->_scenery->toRedrawRight) / 2;
		} else {
			objDesc->xPos += gobPositions[0].x * 12
			    - (_vm->_scenery->toRedrawLeft + _vm->_scenery->toRedrawRight) / 2;
		}

		itemIndInPocket = -1;
		itemIdInPocket = -1;
		_vm->_util->beep(50);
		break;

	default:
		warning("interFunc: Unknown command %d!", cmd);
		_vm->_global->inter_execPtr -= 2;
		cmd = _vm->_inter->load16();
		_vm->_global->inter_execPtr += cmd * 2;
		break;
	}
	return;
}

} // End of namespace Gob
