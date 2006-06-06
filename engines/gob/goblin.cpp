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
#include "gob/music.h"
#include "gob/mult.h"

namespace Gob {

Goblin::Goblin(GobEngine *vm) : _vm(vm) {
	int i;

	_goesAtTarget = 0;
	_readyToAct = 0;
	_gobAction = 0;
	_itemIndInPocket = 5;
	_itemIdInPocket = 2;
	_itemByteFlag = 0;
	_destItemId = -1;
	_destActionItem = 0;
	_actDestItemDesc = 0;
	_forceNextState[0] = -1;
	_forceNextState[1] = -1;
	_forceNextState[2] = -1;
	_forceNextState[3] = -1;
	_forceNextState[4] = -1;
	_forceNextState[5] = -1;
	_forceNextState[6] = -1;
	_forceNextState[7] = 0;
	_forceNextState[8] = 0;
	_forceNextState[9] = 0;

	_rotStates[0][0] = 0; _rotStates[0][1] = 22; _rotStates[0][2] = 23; _rotStates[0][3] = 24;
	_rotStates[1][0] = 13; _rotStates[1][1] = 2; _rotStates[1][2] = 12; _rotStates[1][3] = 14;
	_rotStates[2][0] = 16; _rotStates[2][1] = 15; _rotStates[2][2] = 4; _rotStates[2][3] = 17;
	_rotStates[3][0] = 27; _rotStates[3][1] = 25; _rotStates[3][2] = 26; _rotStates[3][3] = 6;

	_boreCounter = 0;
	_positionedGob = 5;

	_noPick = 0;
	_objList = 0;

	for (i = 0; i < 4; i++)
		_goblins[i] = 0;
	_currentGoblin = 0;
	for (i = 0; i < 16; i++)
		_soundData[i] = 0;
	for (i = 0; i < 3; i++) {
		_gobPositions[i].x = 0;
		_gobPositions[i].y = 0;
	}
	_gobDestX = 0;
	_gobDestY = 0;
	_pressedMapX = 0;
	_pressedMapY = 0;
	_pathExistence = 0;

	_some0ValPtr = 0;

	_gobRetVarPtr = 0;
	_curGobVarPtr = 0;
	_curGobXPosVarPtr = 0;
	_curGobYPosVarPtr = 0;
	_itemInPocketVarPtr = 0;

	_curGobStateVarPtr = 0;
	_curGobFrameVarPtr = 0;
	_curGobMultStateVarPtr = 0;
	_curGobNextStateVarPtr = 0;
	_curGobScrXVarPtr = 0;
	_curGobScrYVarPtr = 0;
	_curGobLeftVarPtr = 0;
	_curGobTopVarPtr = 0;
	_curGobRightVarPtr = 0;
	_curGobBottomVarPtr = 0;
	_curGobDoAnimVarPtr = 0;
	_curGobOrderVarPtr = 0;
	_curGobNoTickVarPtr = 0;
	_curGobTypeVarPtr = 0;
	_curGobMaxTickVarPtr = 0;
	_curGobTickVarPtr = 0;
	_curGobActStartStateVarPtr = 0;
	_curGobLookDirVarPtr = 0;
	_curGobPickableVarPtr = 0;
	_curGobRelaxVarPtr = 0;
	_curGobMaxFrameVarPtr = 0;

	_destItemStateVarPtr = 0;
	_destItemFrameVarPtr = 0;
	_destItemMultStateVarPtr = 0;
	_destItemNextStateVarPtr = 0;
	_destItemScrXVarPtr = 0;
	_destItemScrYVarPtr = 0;
	_destItemLeftVarPtr = 0;
	_destItemTopVarPtr = 0;
	_destItemRightVarPtr = 0;
	_destItemBottomVarPtr = 0;
	_destItemDoAnimVarPtr = 0;
	_destItemOrderVarPtr = 0;
	_destItemNoTickVarPtr = 0;
	_destItemTypeVarPtr = 0;
	_destItemMaxTickVarPtr = 0;
	_destItemTickVarPtr = 0;
	_destItemActStartStVarPtr = 0;
	_destItemLookDirVarPtr = 0;
	_destItemPickableVarPtr = 0;
	_destItemRelaxVarPtr = 0;
	_destItemMaxFrameVarPtr = 0;

	_destItemType = 0;
	_destItemState = 0;
	for (i = 0; i < 20; i++) {
		_itemToObject[i] = 0;
		_objects[i] = 0;
	}
	_objCount = 0;
	_gobsCount = 0;

	_soundSlotsCount = 0;
	for (i = 0; i < 60; i++)
		_soundSlots[i] = -1;

	warning("GOB2 Stub! _word_2F9C0, _word_2F9BE, _word_2F9BC, _word_2F9BA, _dword_2F9B6, _dword_2F9B2");
	_word_2F9C0 = 0;
	_word_2F9BE = 0;
	_word_2F9BC = 0;
	_word_2F9BA = 0;
	_dword_2F9B6 = 0;
	_dword_2F9B2 = 0;
}

Goblin::~Goblin() {
	int i, state, col;

	if (_objList)
		_vm->_util->deleteList(_objList);

	for (i = 0; i < 4; i++) {
		if (_goblins[i]) {
			if (_goblins[i]->stateMach) {
				for (state = 0; state < (i == 3 ? 70 : 40); state++)
					for (col = 0; col < 6; col++)
						if (_goblins[i]->stateMach[state][col])
							delete _goblins[i]->stateMach[state][col];
				delete []_goblins[i]->stateMach;
			}
			delete _goblins[i];
		}
	}
	for (i = 0; i < 20; i++) {
		if (_objects[i]) {
			if (_objects[i]->stateMach) {
				for (state = 0; state < 40; state++)
					for (col = 0; col < 6; col++)
						if (_objects[i]->stateMach[state][col])
							delete _objects[i]->stateMach[state][col];
				delete []_objects[i]->stateMach;
			}
			delete _objects[i];
		}
	}

	for (i = 0; i < 16; i++)
		if (_soundData[i]) {
			if (_soundData[i]->data)
				delete[] _soundData[i]->data;
			delete _soundData[i];
		}
}

char Goblin::rotateState(int16 from, int16 to) {
	return _rotStates[from / 2][to / 2];
}

int16 Goblin::peekGoblin(Gob_Object *_curGob) {
	Util::ListNode *ptr;
	Gob_Object *desc;
	int16 index;
	int16 i;

	ptr = _objList->pHead;
	index = 0;
	while (ptr != 0) {
		desc = (Gob_Object *) ptr->pData;
		if (desc != _curGob) {
			for (i = 0; i < 3; i++) {
				if (desc != _goblins[i])
					continue;

				if (_vm->_global->_inter_mouseX < desc->right &&
				    _vm->_global->_inter_mouseX > desc->left &&
				    _vm->_global->_inter_mouseY < desc->bottom &&
				    _vm->_global->_inter_mouseY > desc->top) {
					index = i + 1;
				}
			}
		}
		ptr = ptr->pNext;
	}
	return index;
}

void Goblin::initList(void) {
	_objList = new Util::List;
	_objList->pHead = 0;
	_objList->pTail = 0;
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

					if (objDesc != _goblins[_currentGoblin])
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

	ptr = _objList->pHead;
	for (ptr = _objList->pHead; ptr != 0; ptr = ptr->pNext) {
		objDesc = (Gob_Object *) ptr->pData;

		if (objDesc->type == 3)
			objDesc->toRedraw = 1;
		else if (objDesc->type == 1)
			objDesc->toRedraw = 0;
	}

	for (ptr = _objList->pHead; ptr != 0; ptr = ptr->pNext) {
		objDesc = (Gob_Object *) ptr->pData;
		if (objDesc->toRedraw == 0)
			continue;

		_vm->_video->drawSprite(_vm->_anim->_animSurf, _vm->_draw->_backSurface,
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

		if (_vm->_scenery->_toRedrawLeft == -12345) {
			objDesc->dirtyLeft = objDesc->left;
			objDesc->dirtyRight = objDesc->right;
			objDesc->dirtyTop = objDesc->top;
			objDesc->dirtyBottom = objDesc->bottom;
		} else {
			objDesc->dirtyLeft =
			    MIN(objDesc->left, _vm->_scenery->_toRedrawLeft);
			objDesc->dirtyRight =
			    MAX(objDesc->right, _vm->_scenery->_toRedrawRight);
			objDesc->dirtyTop =
			    MIN(objDesc->top, _vm->_scenery->_toRedrawTop);
			objDesc->dirtyBottom =
			    MAX(objDesc->bottom, _vm->_scenery->_toRedrawBottom);
		}

		objDesc->dirtyLeft = 0;
		objDesc->dirtyRight = 319;
		objDesc->dirtyTop = 0;
		objDesc->dirtyBottom = 199;
	}

	sortByOrder(_objList);
	for (ptr = _objList->pHead; ptr != 0; ptr = ptr->pNext) {
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
				if (_vm->_scenery->_toRedrawLeft == -12345) {
					objDesc->left = 0;
					objDesc->top = 0;
					objDesc->right = 0;
					objDesc->bottom = 0;
				} else {
					_vm->_draw->invalidateRect(_vm->_scenery->_toRedrawLeft,
					    _vm->_scenery->_toRedrawTop,
					    _vm->_scenery->_toRedrawRight,
					    _vm->_scenery->_toRedrawBottom);

					objDesc->left = _vm->_scenery->_toRedrawLeft;
					objDesc->top = _vm->_scenery->_toRedrawTop;
					objDesc->right = _vm->_scenery->_toRedrawRight;
					objDesc->bottom = _vm->_scenery->_toRedrawBottom;
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
			for (ptr2 = _objList->pHead; ptr2 != 0;
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

				_vm->_scenery->_toRedrawLeft = gobDesc2->dirtyLeft;
				_vm->_scenery->_toRedrawRight = gobDesc2->dirtyRight;
				_vm->_scenery->_toRedrawTop = gobDesc2->dirtyTop;
				_vm->_scenery->_toRedrawBottom = gobDesc2->dirtyBottom;

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

	for (ptr = _objList->pHead; ptr != 0; ptr = ptr->pNext) {
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
					playSound(_soundData[sndItem],
					    repCount, freq);
				}
			}

			sndFrame = state->sndFrame & 0xff;
			sndItem = state->sndItem & 0xff;
			freq = 100 * (state->freq & 0xff);
			repCount = state->repCount & 0xff;

			if (objDesc->curFrame == sndFrame) {
				if (sndItem != 0xff) {
					playSound(_soundData[sndItem],
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
					playSound(_soundData[sndItem],
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

	for (node = _objList->pHead; node != 0; node = node->pNext) {
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
			    _vm->_scenery->_animations[objDesc->animation].layers[layer];

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
					objDesc->stateMach = _goblins[(int)(objDesc->multObjIndex)]->stateMach;
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

int16 Goblin::getObjMaxFrame(Gob_Object * objDesc) {
	int16 layer;

	layer = objDesc->stateMach[objDesc->state][0]->layer;
	return _vm->_scenery->_animations[objDesc->animation].layers[layer]->framesCount -
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
	gobDesc->stateMach = _goblins[(int)gobDesc->multObjIndex]->stateMach;
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

	gobDesc = _goblins[gobIndex];
	layer = gobDesc->stateMach[gobDesc->state][0]->layer;

	frameCount =
	    _vm->_scenery->_animations[gobDesc->animation].layers[layer]->framesCount;
	state = gobDesc->state;
	frame = gobDesc->curFrame;

	gobDesc->noTick = 0;
	gobDesc->toRedraw = 1;

	boreFlag = 1 << _vm->_util->getRandom(7);

	if (gobIndex != _currentGoblin && _vm->_util->getRandom(3) != 0) {
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

	debugC(4, DEBUG_GAMEFLOW, "switchGoblin");
	if (VAR(59) != 0)
		return;

	if (_goblins[_currentGoblin]->state <= 39 &&
	    _goblins[_currentGoblin]->curFrame != 0)
		return;

	if (index != 0 && _goblins[index - 1]->type != 0)
		return;

	if (index == 0)
		next = (_currentGoblin + 1) % 3;
	else
		next = index - 1;

	if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 3 ||
	    _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 6)
		return;

	if (_goblins[(_currentGoblin + 1) % 3]->type != 0 &&
	    _goblins[(_currentGoblin + 2) % 3]->type != 0)
		return;

	_gobPositions[_currentGoblin].x = _vm->_map->_curGoblinX;
	_gobPositions[_currentGoblin].y = _vm->_map->_curGoblinY;

	_goblins[_currentGoblin]->doAnim = 1;
	_goblins[_currentGoblin]->nextState = 21;

	nextLayer(_goblins[_currentGoblin]);

	_currentGoblin = next;
	if (_goblins[_currentGoblin]->type != 0)
		_currentGoblin = (_currentGoblin + 1) % 3;

	_goblins[_currentGoblin]->doAnim = 0;
	if (_goblins[_currentGoblin]->curLookDir == 4)
		_goblins[_currentGoblin]->nextState = 18;
	else
		_goblins[_currentGoblin]->nextState = 19;

	_goblins[_currentGoblin]->toRedraw = 1;
	nextLayer(_goblins[_currentGoblin]);

	tmp = _gobPositions[_currentGoblin].x;
	_pressedMapX = tmp;
	_vm->_map->_destX = tmp;
	_gobDestX = tmp;
	_vm->_map->_curGoblinX = tmp;

	tmp = _gobPositions[_currentGoblin].y;
	_pressedMapY = tmp;
	_vm->_map->_destY = tmp;
	_gobDestY = tmp;
	_vm->_map->_curGoblinY = tmp;

	*_curGobVarPtr = _currentGoblin;
	_pathExistence = 0;
	_readyToAct = 0;
}

void Goblin::adjustDest(int16 posX, int16 posY) {
	int16 resDelta;
	int16 resDeltaDir;
	int16 resDeltaPix;
	int16 deltaPix;
	int16 i;

	if (_vm->_map->getPass(_pressedMapX, _pressedMapY) == 0 &&
	    (_gobAction == 0
		|| _vm->_map->_itemsMap[_pressedMapY][_pressedMapX] == 0)) {

		resDelta = -1;
		resDeltaDir = 0;
		resDeltaPix = 0;

		for (i = 1;
		    i <= _pressedMapX
		    && _vm->_map->getPass(_pressedMapX - i, _pressedMapY) == 0;
		    i++);

		if (i <= _pressedMapX) {
			resDeltaPix = (i - 1) * 12 + (posX % 12) + 1;
			resDelta = i;
		}

		for (i = 1;
				(i + _pressedMapX) < _vm->_map->_mapWidth
		    && _vm->_map->getPass(_pressedMapX + i, _pressedMapY) == 0;
		    i++);

		if (_pressedMapX + i < _vm->_map->_mapWidth) {
			deltaPix = (i * 12) - (posX % 12);
			if (resDelta == -1 || deltaPix < resDeltaPix) {
				resDeltaPix = deltaPix;
				resDelta = i;
				resDeltaDir = 1;
			}
		}

		for (i = 1;
				(i + _pressedMapY) < _vm->_map->_mapHeight
		    && _vm->_map->getPass(_pressedMapX, _pressedMapY + i) == 0;
		    i++);

		if (_pressedMapY + i < _vm->_map->_mapHeight) {
			deltaPix = (i * 6) - (posY % 6);
			if (resDelta == -1 || deltaPix < resDeltaPix) {
				resDeltaPix = deltaPix;
				resDelta = i;
				resDeltaDir = 2;
			}
		}

		for (i = 1;
		    i <= _pressedMapY
		    && _vm->_map->getPass(_pressedMapX, _pressedMapY - i) == 0;
		    i++);

		if (i <= _pressedMapY) {
			deltaPix = (i * 6) + (posY % 6);
			if (resDelta == -1 || deltaPix < resDeltaPix) {
				resDeltaPix = deltaPix;
				resDelta = i;
				resDeltaDir = 3;
			}
		}

		switch (resDeltaDir) {
		case 0:
			_pressedMapX -= resDelta;
			break;

		case 1:
			_pressedMapX += resDelta;
			break;

		case 2:
			_pressedMapY += resDelta;
			break;

		case 3:
			_pressedMapY -= resDelta;
			break;
		}

	}
}

void Goblin::adjustTarget(void) {
	if (_gobAction == 4
	    && _vm->_map->_itemsMap[_pressedMapY][_pressedMapX] == 0) {

		if (_pressedMapY > 0
		    && _vm->_map->_itemsMap[_pressedMapY - 1][_pressedMapX] !=
		    0) {
			_pressedMapY--;
		} else if (_pressedMapX < _vm->_map->_mapWidth - 1
		    && _vm->_map->_itemsMap[_pressedMapY][_pressedMapX + 1] !=
		    0) {
			_pressedMapX++;
		} else if (_pressedMapX < _vm->_map->_mapWidth - 1 && _pressedMapY > 0
		    && _vm->_map->_itemsMap[_pressedMapY - 1][_pressedMapX +
			1] != 0) {
			_pressedMapY--;
			_pressedMapX++;
		}
	}
}

void Goblin::targetDummyItem(Gob_Object *gobDesc) {
	if (_vm->_map->_itemsMap[_pressedMapY][_pressedMapX] == 0 &&
	    _vm->_map->getPass(_pressedMapX, _pressedMapY) == 1) {
		if (gobDesc->curLookDir == 0) {
			_vm->_map->_itemPoses[0].x = _pressedMapX;
			_vm->_map->_itemPoses[0].y = _pressedMapY;
			_vm->_map->_itemPoses[0].orient = -4;
		} else {
			_vm->_map->_itemPoses[0].x = _pressedMapX;
			_vm->_map->_itemPoses[0].y = _pressedMapY;
			_vm->_map->_itemPoses[0].orient = -1;
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

	if (_gobAction == 3 || _gobAction == 4) {
		items = _vm->_map->_itemsMap[_pressedMapY][_pressedMapX];
		if (_gobAction == 4 && (items & 0xff00) != 0 &&
		    _objects[_itemToObject[(items & 0xff00) >> 8]]->
		    pickable == 1) {
			_destItemId = (items & 0xff00) >> 8;
			_destActionItem = (items & 0xff00) >> 8;
			_itemByteFlag = 1;
		} else if ((items & 0xff) == 0) {
			_destItemId = (items & 0xff00) >> 8;
			_destActionItem = (items & 0xff00) >> 8;
			_itemByteFlag = 1;
		} else if (_gobAction == 3 && _currentGoblin == 2 &&
		    (items & 0xff00) != 0) {
			_destItemId = (items & 0xff00) >> 8;
			_destActionItem = (items & 0xff00) >> 8;
			_itemByteFlag = 1;
		} else {
			_destItemId = items & 0xff;
			_destActionItem = items & 0xff;
			_itemByteFlag = 0;
		}

		_pressedMapY = _vm->_map->_itemPoses[_destItemId].y;
		_vm->_map->_destY = _vm->_map->_itemPoses[_destItemId].y;
		_gobDestY = _vm->_map->_itemPoses[_destItemId].y;

		if (_gobAction == 3 || _destActionItem == 0) {
			_pressedMapX = _vm->_map->_itemPoses[_destItemId].x;
			_vm->_map->_destX = _vm->_map->_itemPoses[_destItemId].x;
			_gobDestX = _vm->_map->_itemPoses[_destItemId].x;
		} else if ((items & 0xff00) != 0) {
			if (_vm->_map->_itemPoses[_destItemId].orient == 4) {
				if ((_vm->_map->_itemsMap[_pressedMapY]
					[_pressedMapX - 1] & 0xff00) ==
				    (_vm->_map->_itemsMap[_pressedMapY]
					[_pressedMapX] & 0xff00)) {
					_pressedMapX--;
					_vm->_map->_destX = _pressedMapX;
					_gobDestX = _pressedMapX;
				}
			} else if (_vm->_map->_itemPoses[_destItemId].orient == 0) {

				if ((_vm->_map->_itemsMap[_pressedMapY]
					[_pressedMapX + 1] & 0xff00) ==
				    (_vm->_map->_itemsMap[_pressedMapY]
					[_pressedMapX] & 0xff00)) {
					_pressedMapX++;
					_vm->_map->_destX = _pressedMapX;
					_gobDestX = _pressedMapX;
				}
			}

			if ((_vm->_map->_itemsMap[_pressedMapY +
				    1][_pressedMapX] & 0xff00) ==
			    (_vm->_map->_itemsMap[_pressedMapY][_pressedMapX] &
				0xff00)) {
				_pressedMapY++;
				_vm->_map->_destY = _pressedMapY;
				_gobDestY = _pressedMapY;
			}
		} else {
			if (_vm->_map->_itemPoses[_destItemId].orient == 4) {
				if ((_vm->_map->_itemsMap[_pressedMapY]
					[_pressedMapX - 1]) ==
				    (_vm->_map->_itemsMap[_pressedMapY]
					[_pressedMapX])) {
					_pressedMapX--;
					_vm->_map->_destX = _pressedMapX;
					_gobDestX = _pressedMapX;
				}
			} else if (_vm->_map->_itemPoses[_destItemId].orient == 0) {

				if ((_vm->_map->_itemsMap[_pressedMapY]
					[_pressedMapX + 1]) ==
				    (_vm->_map->_itemsMap[_pressedMapY]
					[_pressedMapX])) {
					_pressedMapX++;
					_vm->_map->_destX = _pressedMapX;
					_gobDestX = _pressedMapX;
				}
			}

			if (_pressedMapY < (_vm->_map->_mapHeight-1)) {
				if ((_vm->_map->_itemsMap[_pressedMapY +
							1][_pressedMapX]) ==
						(_vm->_map->_itemsMap[_pressedMapY][_pressedMapX])) {
					_pressedMapY++;
					_vm->_map->_destY = _pressedMapY;
					_gobDestY = _pressedMapY;
				}
			}

		}

		if (_gobAction == 4 && _destActionItem != 0 &&
			_itemToObject[_destActionItem] != -1 &&
		    _objects[_itemToObject[_destActionItem]]->
		    pickable == 1) {

			itemDesc =
			    _objects[_itemToObject[_destActionItem]];

			itemDesc->animation =
			    itemDesc->stateMach[itemDesc->state][0]->animation;
			layer =
			    itemDesc->stateMach[itemDesc->state][itemDesc->
			    stateColumn]->layer;

			_vm->_scenery->updateAnim(layer, 0, itemDesc->animation, 0,
			    itemDesc->xPos, itemDesc->yPos, 0);

			tmpX = (_vm->_scenery->_toRedrawRight + _vm->_scenery->_toRedrawLeft) / 2;
			tmpY = _vm->_scenery->_toRedrawBottom;

			tmpPosY = tmpY / 6;
			if ((tmpY % 3) < 3 && tmpPosY > 0)
				tmpPosY--;

			tmpPosX = tmpX / 12;
			if ((tmpX % 12) < 6 && tmpPosX > 0)
				tmpPosX--;

			if (_vm->_map->_itemPoses[_destActionItem].orient == 0 ||
			    _vm->_map->_itemPoses[_destActionItem].orient == -1) {
				tmpPosX++;
			}

			if (_vm->_map->getPass(tmpPosX, tmpPosY) == 1) {
				_pressedMapX = tmpPosX;
				_vm->_map->_destX = tmpPosX;
				_gobDestX = tmpPosX;

				_pressedMapY = tmpPosY;
				_vm->_map->_destY = tmpPosY;
				_gobDestY = tmpPosY;
			}
		}
	}
}

void Goblin::moveFindItem(int16 posX, int16 posY) {
	int16 i;
	if (_gobAction == 3 || _gobAction == 4) {
		for (i = 0; i < 20; i++) {
			if (_objects[i] == 0)
				continue;

			if (_objects[i]->type != 0)
				continue;

			if (_objects[i]->left > posX)
				continue;

			if (_objects[i]->right < posX)
				continue;

			if (_objects[i]->top > posY)
				continue;

			if (_objects[i]->bottom < posY)
				continue;

			if (_objects[i]->right - _objects[i]->left < 40)
				posX =
				    (_objects[i]->left +
				    _objects[i]->right) / 2;

			if (_objects[i]->bottom - _objects[i]->top < 40)
				posY =
				    (_objects[i]->top +
				    _objects[i]->bottom) / 2;

			break;
		}

		_pressedMapX = MIN(posX / 12, _vm->_map->_mapWidth - 1);
		_pressedMapY = MIN(posY / 6, _vm->_map->_mapHeight - 1);

		if (_vm->_map->_itemsMap[_pressedMapY][_pressedMapX] == 0
		    && i < 20) {

			if (_vm->_map->_itemsMap[_pressedMapY +
				1][_pressedMapX] != 0) {
				_pressedMapY++;
			} else if (_vm->_map->_itemsMap[_pressedMapY +
				1][_pressedMapX + 1] != 0) {
				_pressedMapX++;
				_pressedMapY++;
			} else
			    if (_vm->_map->_itemsMap[_pressedMapY][_pressedMapX +
				1] != 0) {
				_pressedMapX++;
			} else if (_vm->_map->_itemsMap[_pressedMapY -
				1][_pressedMapX + 1] != 0) {
				_pressedMapX++;
				_pressedMapY--;
			} else if (_vm->_map->_itemsMap[_pressedMapY -
				1][_pressedMapX] != 0) {
				_pressedMapY--;
			} else if (_vm->_map->_itemsMap[_pressedMapY -
				1][_pressedMapX - 1] != 0) {
				_pressedMapY--;
				_pressedMapX--;
			} else
			    if (_vm->_map->_itemsMap[_pressedMapY][_pressedMapX -
				1] != 0) {
				_pressedMapX--;
			} else if (_vm->_map->_itemsMap[_pressedMapY +
				1][_pressedMapX - 1] != 0) {
				_pressedMapX--;
				_pressedMapY++;
			}
		}
	} else {
		_pressedMapX = posX / 12;
		_pressedMapY = posY / 6;
	}
}

void Goblin::moveCheckSelect(int16 framesCount, Gob_Object * gobDesc, int16 *pGobIndex,
	    int16 *nextAct) {
	if (gobDesc->right > _vm->_global->_inter_mouseX &&
	    gobDesc->left < _vm->_global->_inter_mouseX &&
	    gobDesc->bottom > _vm->_global->_inter_mouseY &&
	    gobDesc->bottom - 10 < _vm->_global->_inter_mouseY && _gobAction == 0) {
		if (gobDesc->curLookDir & 4)
			*nextAct = 16;
		else
			*nextAct = 23;

		gobDesc->curFrame = framesCount - 1;
		_pathExistence = 0;
	} else {
		*pGobIndex = peekGoblin(gobDesc);

		if (*pGobIndex != 0) {
			_pathExistence = 0;
		} else if (_vm->_map->_curGoblinX == _pressedMapX &&
		    _vm->_map->_curGoblinY == _pressedMapY) {

			if (_gobAction != 0)
				_readyToAct = 1;

			_pathExistence = 0;
		}
	}
}

void Goblin::moveInitStep(int16 framesCount, int16 action, int16 cont,
	    Gob_Object *gobDesc, int16 *pGobIndex, int16 *pNextAct) {
	int16 posX;
	int16 posY;

	if (cont != 0 && _goesAtTarget == 0 &&
	    _readyToAct == 0 && VAR(59) == 0 &&
	    gobDesc->type != 1 &&
	    gobDesc->state != 10 && gobDesc->state != 11) {
		if (gobDesc->state >= 40) {
			gobDesc->curFrame = framesCount - 1;
		}

		_gobAction = action;
		_forceNextState[0] = -1;
		_forceNextState[1] = -1;
		_forceNextState[2] = -1;

		if (action == 3) {
			posX = _vm->_global->_inter_mouseX + 6;
			posY = _vm->_global->_inter_mouseY + 7;
		} else if (action == 4) {
			posX = _vm->_global->_inter_mouseX + 7;
			posY = _vm->_global->_inter_mouseY + 12;
		} else {
			posX = _vm->_global->_inter_mouseX;
			posY = _vm->_global->_inter_mouseY;
		}

		moveFindItem(posX, posY);
		adjustDest(posX, posY);
		adjustTarget();

		_vm->_map->_destX = _pressedMapX;
		_gobDestX = _pressedMapX;

		_vm->_map->_destY = _pressedMapY;
		_gobDestY = _pressedMapY;

		targetDummyItem(gobDesc);

		targetItem();
		initiateMove(0);

		moveCheckSelect(framesCount, gobDesc, pGobIndex, pNextAct);
	} else {

		if (_readyToAct != 0 &&
		    (_vm->_map->_curGoblinX != _pressedMapX ||
			_vm->_map->_curGoblinY != _pressedMapY))
			_readyToAct = 0;

		if (gobDesc->type == 1) {
			*pGobIndex = peekGoblin(gobDesc);
		}
	}
}

void Goblin::moveTreatRopeStairs(Gob_Object *gobDesc) {
	if (_currentGoblin != 1)
		return;

	if (gobDesc->nextState == 28
	    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY - 1) == 6) {
		_forceNextState[0] = 28;
		_forceNextState[1] = -1;
	}

	if (gobDesc->nextState == 29
	    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY + 1) == 6) {
		_forceNextState[0] = 29;
		_forceNextState[1] = -1;
	}

	if ((gobDesc->nextState == 28 || gobDesc->nextState == 29
		|| gobDesc->nextState == 20)
	    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 6) {
		if ((gobDesc->curLookDir == 0 || gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 2)
		    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY - 1) == 6) {
			_forceNextState[0] = 28;
			_forceNextState[1] = -1;
		} else if ((gobDesc->curLookDir == 0
			|| gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 6)
		    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY + 1) == 6) {
			_forceNextState[0] = 29;
			_forceNextState[1] = -1;
		}
	}

	if (gobDesc->nextState == 8
	    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY - 1) == 3) {
		_forceNextState[0] = 8;
		_forceNextState[1] = -1;
	}

	if (gobDesc->nextState == 9
	    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY + 1) == 3) {
		_forceNextState[0] = 9;
		_forceNextState[1] = -1;
	}

	if (gobDesc->nextState == 20
	    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 3) {
		if ((gobDesc->curLookDir == 0 || gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 2)
		    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY - 1) == 3) {
			_forceNextState[0] = 8;
			_forceNextState[1] = -1;
		} else if ((gobDesc->curLookDir == 0
			|| gobDesc->curLookDir == 4
			|| gobDesc->curLookDir == 6)
		    && _vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY + 1) == 3) {
			_forceNextState[0] = 9;
			_forceNextState[1] = -1;
		}
	}

}

void Goblin::movePathFind(Gob_Object *gobDesc, int16 nextAct) {
	if (_pathExistence == 1) {
		_vm->_map->_curGoblinX = _gobPositions[_currentGoblin].x;
		_vm->_map->_curGoblinY = _gobPositions[_currentGoblin].y;

		if (_vm->_map->_curGoblinX == _pressedMapX &&
		    _vm->_map->_curGoblinY == _pressedMapY && _gobAction != 0) {
			_readyToAct = 1;
			_pathExistence = 0;
		}

		nextAct = _vm->_map->getDirection(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY,
		    _vm->_map->_destX, _vm->_map->_destY);

		if (nextAct == 0)
			_pathExistence = 0;
	} else if (_pathExistence == 3) {
		_vm->_map->_curGoblinX = _gobPositions[_currentGoblin].x;
		_vm->_map->_curGoblinY = _gobPositions[_currentGoblin].y;

		if (_vm->_map->_curGoblinX == _gobDestX && _vm->_map->_curGoblinY == _gobDestY) {
			_pathExistence = 1;
			_vm->_map->_destX = _pressedMapX;
			_vm->_map->_destY = _pressedMapY;
		} else {

			if (_vm->_map->checkDirectPath(-1, _vm->_map->_curGoblinX, _vm->_map->_curGoblinY,
				_gobDestX, _gobDestY) == 1) {
				_vm->_map->_destX = _gobDestX;
				_vm->_map->_destY = _gobDestY;
			} else if (_vm->_map->_curGoblinX == _vm->_map->_destX && _vm->_map->_curGoblinY == _vm->_map->_destY) {

				if (_vm->_map->_nearestWayPoint > _vm->_map->_nearestDest) {
					_vm->_map->optimizePoints(0, 0, 0);

					_vm->_map->_destX =
					    _vm->_map->_wayPoints[_vm->_map->_nearestWayPoint].
					    x;
					_vm->_map->_destY =
					    _vm->_map->_wayPoints[_vm->_map->_nearestWayPoint].
					    y;

					if (_vm->_map->_nearestWayPoint > _vm->_map->_nearestDest)
						_vm->_map->_nearestWayPoint--;
				} else if (_vm->_map->_nearestWayPoint < _vm->_map->_nearestDest) {
					_vm->_map->optimizePoints(0, 0, 0);

					_vm->_map->_destX =
					    _vm->_map->_wayPoints[_vm->_map->_nearestWayPoint].
					    x;
					_vm->_map->_destY =
					    _vm->_map->_wayPoints[_vm->_map->_nearestWayPoint].
					    y;

					if (_vm->_map->_nearestWayPoint < _vm->_map->_nearestDest)
						_vm->_map->_nearestWayPoint++;
				} else {
					if (_vm->_map->checkDirectPath(-1, _vm->_map->_curGoblinX,
						_vm->_map->_curGoblinY, _gobDestX,
						_gobDestY) == 3 && _vm->_map->getPass(_pressedMapX, _pressedMapY) != 0) {
						_vm->_map->_destX = _vm->_map->_wayPoints[_vm->_map->_nearestWayPoint].x;
						_vm->_map->_destY = _vm->_map->_wayPoints[_vm->_map->_nearestWayPoint].y;
					} else {
						_pathExistence = 1;
						_vm->_map->_destX = _pressedMapX;
						_vm->_map->_destY = _pressedMapY;
					}
				}
			}
			nextAct =
			    _vm->_map->getDirection(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY,
			    _vm->_map->_destX, _vm->_map->_destY);
		}
	}

	if (_readyToAct != 0 && (_gobAction == 3 || _gobAction == 4))
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
		if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY - 1) == 6 &&
		    _currentGoblin != 1) {
			_pathExistence = 0;
			break;
		}

		if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 3) {
			gobDesc->nextState = 8;
			break;
		}

		if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 6 &&
		    _currentGoblin == 1) {
			gobDesc->nextState = 28;
			break;
		}

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 2);
		break;

	case Map::kDirS:
		if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY + 1) == 6 &&
		    _currentGoblin != 1) {
			_pathExistence = 0;
			break;
		}

		if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 3) {
			gobDesc->nextState = 9;
			break;
		}

		if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 6 &&
		    _currentGoblin == 1) {
			gobDesc->nextState = 29;
			break;
		}

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 6);
		break;

	case Map::kDirSE:
		if (_vm->_map->getPass(_vm->_map->_curGoblinX + 1, _vm->_map->_curGoblinY + 1) == 6 &&
		    _currentGoblin != 1) {
			_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 5;
		if (gobDesc->curLookDir == 4)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 4);
		break;

	case Map::kDirSW:
		if (_vm->_map->getPass(_vm->_map->_curGoblinX - 1, _vm->_map->_curGoblinY + 1) == 6 &&
		    _currentGoblin != 1) {
			_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 7;
		if (gobDesc->curLookDir == 0)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 0);
		break;

	case Map::kDirNW:
		if (_vm->_map->getPass(_vm->_map->_curGoblinX - 1, _vm->_map->_curGoblinY - 1) == 6 &&
		    _currentGoblin != 1) {
			_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 1;
		if (gobDesc->curLookDir == 0)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 0);
		break;

	case Map::kDirNE:
		if (_vm->_map->getPass(_vm->_map->_curGoblinX + 1, _vm->_map->_curGoblinY - 1) == 6 &&
		    _currentGoblin != 1) {
			_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 3;
		if (gobDesc->curLookDir == 4)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 4);
		break;

	case 0x4dc8:

		if (_currentGoblin == 0 && _gobAction == 3
		    && _itemIndInPocket == -1) {
			_destItemId = -1;
			_readyToAct = 0;
			break;
		}

		if (_currentGoblin == 0 && _gobAction == 4 &&
		    _itemIndInPocket == -1 && _destActionItem == 0) {
			gobDesc->multState = 104;
			_destItemId = -1;
			_readyToAct = 0;
			break;
		}

		if (_currentGoblin == 0 && _gobAction == 4 &&
		    _itemIndInPocket == -1 && _destActionItem != 0 &&
		    _itemToObject[_destActionItem] != -1 &&
		    _objects[_itemToObject[_destActionItem]]->
		    pickable == 0) {
			gobDesc->multState = 104;
			_destItemId = -1;
			_readyToAct = 0;
			break;
		}

		switch (_vm->_map->_itemPoses[_destActionItem].orient) {
		case 0:
		case -4:
			gobDesc->nextState = 10;
			gobDesc->curLookDir = 0;
			_destItemId = -1;
			break;

		case -1:
		case 4:
			gobDesc->nextState = 11;
			gobDesc->curLookDir = 4;
			_destItemId = -1;
			break;
		}
		break;

	default:
		if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 3 ||
		    (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 6
			&& _currentGoblin == 1)) {
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

	if (_goesAtTarget == 0
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
			if (_currentGoblin != 1)
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
			if (_currentGoblin != 1)
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

	if (_currentGoblin == 0
	    && gobDesc->stateMach == gobDesc->realStateMach
	    && (gobDesc->state == 10 || gobDesc->state == 11)
	    && gobDesc->curFrame == 9) {
		_vm->_snd->stopSound(0);
		if (_itemIndInPocket != -1) {
			_vm->_snd->playSample(_soundData[14], 1, 9000);
		}

		if (_itemIndInPocket == -1) {
			_vm->_snd->playSample(_soundData[14], 1, 5000);
		}
	}

	if (_boreCounter++ == 120) {
		_boreCounter = 0;
		for (i = 0; i < 3; i++)
			showBoredom(i);
	}

	if (gobDesc->multState != -1 && gobDesc->curFrame == framesCount &&
	    gobDesc->state != gobDesc->multState) {
		gobDesc->nextState = gobDesc->multState;
		gobDesc->multState = -1;

		newX =
		    _vm->_scenery->_animations[gobDesc->animation].
		    layers[_gobStateLayer]->animDeltaX + gobDesc->xPos;

		newY =
		    _vm->_scenery->_animations[gobDesc->animation].
		    layers[_gobStateLayer]->animDeltaY + gobDesc->yPos;

		_gobStateLayer = nextLayer(gobDesc);

		gobDesc->xPos = newX;
		gobDesc->yPos = newY;
	} else {
		if (gobDesc->curFrame == 3 &&
		    gobDesc->stateMach == gobDesc->realStateMach &&
		    (gobDesc->state < 10 ||
			(_currentGoblin == 1 && (gobDesc->state == 28
				|| gobDesc->state == 29))
		    )) {
			flag = 0;
			if (_forceNextState[0] != -1) {
				gobDesc->nextState = _forceNextState[0];
				for (i = 0; i < 9; i++)
					_forceNextState[i] =
					    _forceNextState[i + 1];
			}

			_vm->_map->_curGoblinX = _gobPositions[_currentGoblin].x;
			_vm->_map->_curGoblinY = _gobPositions[_currentGoblin].y;

			if (gobDesc->nextState != gobDesc->state) {
				_gobStateLayer = nextLayer(gobDesc);
				flag = 1;
			}

			switch (gobDesc->state) {
			case 0:
				_gobPositions[_currentGoblin].x--;
				break;

			case 2:
			case 8:
				_gobPositions[_currentGoblin].y--;
				break;

			case 4:
				_gobPositions[_currentGoblin].x++;
				break;

			case 6:
			case 9:
				_gobPositions[_currentGoblin].y++;
				break;

			case 1:
				_gobPositions[_currentGoblin].x--;
				_gobPositions[_currentGoblin].y--;
				break;

			case 3:
				_gobPositions[_currentGoblin].x++;
				_gobPositions[_currentGoblin].y--;
				break;

			case 5:
				_gobPositions[_currentGoblin].x++;
				_gobPositions[_currentGoblin].y++;
				break;

			case 7:
				_gobPositions[_currentGoblin].x--;
				_gobPositions[_currentGoblin].y++;
				break;

			case 38:
				_gobPositions[_currentGoblin].y++;
				break;
			}

			if (_currentGoblin == 1) {
				if (gobDesc->state == 28)
					_gobPositions[1].y--;

				if (gobDesc->state == 29)
					_gobPositions[1].y++;
			}

			if (flag != 0) {
				_vm->_scenery->updateAnim(_gobStateLayer, 0,
				    gobDesc->animation, 0, gobDesc->xPos,
				    gobDesc->yPos, 0);

				gobDesc->yPos =
				    (_vm->_map->_curGoblinY + 1) * 6 -
				    (_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
				gobDesc->xPos =
				    _vm->_map->_curGoblinX * 12 - (_vm->_scenery->_toRedrawLeft -
				    _vm->_scenery->_animLeft);
			}

			if ((gobDesc->state == 10 || gobDesc->state == 11)
			    && _currentGoblin != 0)
				_goesAtTarget = 1;
		}

		if (gobDesc->curFrame != framesCount)
			return;

		if (_forceNextState[0] != -1) {
			gobDesc->nextState = _forceNextState[0];
			for (i = 0; i < 10; i++)
				_forceNextState[i] =
				    _forceNextState[i + 1];
		}

		_vm->_map->_curGoblinX = _gobPositions[_currentGoblin].x;
		_vm->_map->_curGoblinY = _gobPositions[_currentGoblin].y;

		_gobStateLayer = nextLayer(gobDesc);
		if (gobDesc->stateMach == gobDesc->realStateMach) {

			switch (gobDesc->nextState) {
			case 0:
				_gobPositions[_currentGoblin].x--;
				break;

			case 2:
			case 8:
				_gobPositions[_currentGoblin].y--;
				break;

			case 4:
				_gobPositions[_currentGoblin].x++;
				break;

			case 6:
			case 9:
				_gobPositions[_currentGoblin].y++;
				break;

			case 1:
				_gobPositions[_currentGoblin].x--;
				_gobPositions[_currentGoblin].y--;
				break;

			case 3:
				_gobPositions[_currentGoblin].x++;
				_gobPositions[_currentGoblin].y--;
				break;

			case 5:
				_gobPositions[_currentGoblin].x++;
				_gobPositions[_currentGoblin].y++;
				break;

			case 7:
				_gobPositions[_currentGoblin].x--;
				_gobPositions[_currentGoblin].y++;
				break;

			case 38:
				_gobPositions[_currentGoblin].y++;
				break;
			}
			if (_currentGoblin == 1) {
				if (gobDesc->nextState == 28)
					_gobPositions[1].y--;

				if (gobDesc->nextState == 29)
					_gobPositions[1].y++;
			}
		}

		_vm->_scenery->updateAnim(_gobStateLayer, 0, gobDesc->animation, 0,
		    gobDesc->xPos, gobDesc->yPos, 0);

		gobDesc->yPos =
		    (_vm->_map->_curGoblinY + 1) * 6 - (_vm->_scenery->_toRedrawBottom -
		    _vm->_scenery->_animTop);
		gobDesc->xPos =
		    _vm->_map->_curGoblinX * 12 - (_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

		if ((gobDesc->state == 10 || gobDesc->state == 11)
		    && _currentGoblin != 0)
			_goesAtTarget = 1;
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
	    _vm->_scenery->_animations[gobDesc->animation].layers[layer]->framesCount;

	if (VAR(59) == 0 &&
	    gobDesc->state != 30 && gobDesc->state != 31) {
		gobDesc->order = (gobDesc->bottom) / 24 + 3;
	}

	if (_positionedGob != _currentGoblin) {
		_vm->_map->_curGoblinX = _gobPositions[_currentGoblin].x;
		_vm->_map->_curGoblinY = _gobPositions[_currentGoblin].y;
	}

	_positionedGob = _currentGoblin;

	gobDesc->animation =
	    gobDesc->stateMach[gobDesc->state][gobDesc->stateColumn]->
	    animation;

	_gobStateLayer =
	    gobDesc->stateMach[gobDesc->state][gobDesc->stateColumn]->layer;

	moveInitStep(framesCount, action, cont, gobDesc, &gobIndex,
	    &nextAct);
	moveTreatRopeStairs(gobDesc);
	moveAdvance(gobDesc, nextAct, framesCount);

	return gobIndex;
}

void Goblin::zeroObjects(void) {
	int16 i;

	for (i = 0; i < 4; i++)
		_goblins[i] = 0;

	for (i = 0; i < 20; i++)
		_objects[i] = 0;

	for (i = 0; i < 16; i++)
		_soundData[i] = 0;
}

void Goblin::freeAllObjects(void) {
	_vm->_util->deleteList(_objList);
	_objList = 0;
	freeObjects();
}

void Goblin::loadObjects(char *source) {
	int16 i;

	zeroObjects();
	for (i = 0; i < 20; i++)
		_itemToObject[i] = 100;

	freeObjects();
	initList();
	strcpy(_vm->_map->_sourceFile, source);

	_vm->_map->_sourceFile[strlen(_vm->_map->_sourceFile) - 4] = 0;
	_vm->_map->loadMapObjects(source);

	for (i = 0; i < _gobsCount; i++)
		placeObject(_goblins[i], 0, 0, 0, 0, 0);

	for (i = 0; i < _objCount; i++) {
		placeObject(_objects[i], 1, 0, 0, 0, 0);
	}

	initVarPointers();
	_actDestItemDesc = 0;
}

void Goblin::saveGobDataToVars(int16 xPos, int16 yPos, int16 someVal) {
	Gob_Object *obj;
	*_some0ValPtr = someVal;
	*_curGobXPosVarPtr = xPos;
	*_curGobYPosVarPtr = yPos;
	*_itemInPocketVarPtr = _itemIndInPocket;

	obj = _goblins[_currentGoblin];

	*_curGobStateVarPtr = obj->state;
	*_curGobFrameVarPtr = obj->curFrame;
	*_curGobMultStateVarPtr = obj->multState;
	*_curGobNextStateVarPtr = obj->nextState;
	*_curGobScrXVarPtr = obj->xPos;
	*_curGobScrYVarPtr = obj->yPos;
	*_curGobLeftVarPtr = obj->left;
	*_curGobTopVarPtr = obj->top;
	*_curGobRightVarPtr = obj->right;
	*_curGobBottomVarPtr = obj->bottom;
	*_curGobDoAnimVarPtr = obj->doAnim;
	*_curGobOrderVarPtr = obj->order;
	*_curGobNoTickVarPtr = obj->noTick;
	*_curGobTypeVarPtr = obj->type;
	*_curGobMaxTickVarPtr = obj->maxTick;
	*_curGobTickVarPtr = obj->tick;
	*_curGobActStartStateVarPtr = obj->actionStartState;
	*_curGobLookDirVarPtr = obj->curLookDir;
	*_curGobPickableVarPtr = obj->pickable;
	*_curGobRelaxVarPtr = obj->relaxTime;
	*_curGobMaxFrameVarPtr = getObjMaxFrame(obj);

	if (_actDestItemDesc == 0)
		return;

	obj = _actDestItemDesc;
	*_destItemStateVarPtr = obj->state;
	*_destItemFrameVarPtr = obj->curFrame;
	*_destItemMultStateVarPtr = obj->multState;
	*_destItemNextStateVarPtr = obj->nextState;
	*_destItemScrXVarPtr = obj->xPos;
	*_destItemScrYVarPtr = obj->yPos;
	*_destItemLeftVarPtr = obj->left;
	*_destItemTopVarPtr = obj->top;
	*_destItemRightVarPtr = obj->right;
	*_destItemBottomVarPtr = obj->bottom;
	*_destItemDoAnimVarPtr = obj->doAnim;
	*_destItemOrderVarPtr = obj->order;
	*_destItemNoTickVarPtr = obj->noTick;
	*_destItemTypeVarPtr = obj->type;
	*_destItemMaxTickVarPtr = obj->maxTick;
	*_destItemTickVarPtr = obj->tick;
	*_destItemActStartStVarPtr = obj->actionStartState;
	*_destItemLookDirVarPtr = obj->curLookDir;
	*_destItemPickableVarPtr = obj->pickable;
	*_destItemRelaxVarPtr = obj->relaxTime;
	*_destItemMaxFrameVarPtr = getObjMaxFrame(obj);

	_destItemState = obj->state;
	_destItemType = obj->type;
}

void Goblin::initVarPointers(void) {
	_gobRetVarPtr = (int32 *)VAR_ADDRESS(59);
	_curGobStateVarPtr = (int32 *)VAR_ADDRESS(60);
	_curGobFrameVarPtr = (int32 *)VAR_ADDRESS(61);
	_curGobMultStateVarPtr = (int32 *)VAR_ADDRESS(62);
	_curGobNextStateVarPtr = (int32 *)VAR_ADDRESS(63);
	_curGobScrXVarPtr = (int32 *)VAR_ADDRESS(64);
	_curGobScrYVarPtr = (int32 *)VAR_ADDRESS(65);
	_curGobLeftVarPtr = (int32 *)VAR_ADDRESS(66);
	_curGobTopVarPtr = (int32 *)VAR_ADDRESS(67);
	_curGobRightVarPtr = (int32 *)VAR_ADDRESS(68);
	_curGobBottomVarPtr = (int32 *)VAR_ADDRESS(69);
	_curGobDoAnimVarPtr = (int32 *)VAR_ADDRESS(70);
	_curGobOrderVarPtr = (int32 *)VAR_ADDRESS(71);
	_curGobNoTickVarPtr = (int32 *)VAR_ADDRESS(72);
	_curGobTypeVarPtr = (int32 *)VAR_ADDRESS(73);
	_curGobMaxTickVarPtr = (int32 *)VAR_ADDRESS(74);
	_curGobTickVarPtr = (int32 *)VAR_ADDRESS(75);
	_curGobActStartStateVarPtr = (int32 *)VAR_ADDRESS(76);
	_curGobLookDirVarPtr = (int32 *)VAR_ADDRESS(77);
	_curGobPickableVarPtr = (int32 *)VAR_ADDRESS(80);
	_curGobRelaxVarPtr = (int32 *)VAR_ADDRESS(81);
	_destItemStateVarPtr = (int32 *)VAR_ADDRESS(82);
	_destItemFrameVarPtr = (int32 *)VAR_ADDRESS(83);
	_destItemMultStateVarPtr = (int32 *)VAR_ADDRESS(84);
	_destItemNextStateVarPtr = (int32 *)VAR_ADDRESS(85);
	_destItemScrXVarPtr = (int32 *)VAR_ADDRESS(86);
	_destItemScrYVarPtr = (int32 *)VAR_ADDRESS(87);
	_destItemLeftVarPtr = (int32 *)VAR_ADDRESS(88);
	_destItemTopVarPtr = (int32 *)VAR_ADDRESS(89);
	_destItemRightVarPtr = (int32 *)VAR_ADDRESS(90);
	_destItemBottomVarPtr = (int32 *)VAR_ADDRESS(91);
	_destItemDoAnimVarPtr = (int32 *)VAR_ADDRESS(92);
	_destItemOrderVarPtr = (int32 *)VAR_ADDRESS(93);
	_destItemNoTickVarPtr = (int32 *)VAR_ADDRESS(94);
	_destItemTypeVarPtr = (int32 *)VAR_ADDRESS(95);
	_destItemMaxTickVarPtr = (int32 *)VAR_ADDRESS(96);
	_destItemTickVarPtr = (int32 *)VAR_ADDRESS(97);
	_destItemActStartStVarPtr = (int32 *)VAR_ADDRESS(98);
	_destItemLookDirVarPtr = (int32 *)VAR_ADDRESS(99);
	_destItemPickableVarPtr = (int32 *)VAR_ADDRESS(102);
	_destItemRelaxVarPtr = (int32 *)VAR_ADDRESS(103);
	_destItemMaxFrameVarPtr = (int32 *)VAR_ADDRESS(105);
	_curGobVarPtr = (int32 *)VAR_ADDRESS(106);
	_some0ValPtr = (int32 *)VAR_ADDRESS(107);
	_curGobXPosVarPtr = (int32 *)VAR_ADDRESS(108);
	_curGobYPosVarPtr = (int32 *)VAR_ADDRESS(109);
	_curGobMaxFrameVarPtr = (int32 *)VAR_ADDRESS(110);

	_itemInPocketVarPtr = (int32 *)VAR_ADDRESS(114);

	*_itemInPocketVarPtr = -2;
}

void Goblin::loadGobDataFromVars(void) {
	Gob_Object *obj;

	_itemIndInPocket = *_itemInPocketVarPtr;

	obj = _goblins[_currentGoblin];

	obj->state = *_curGobStateVarPtr;
	obj->curFrame = *_curGobFrameVarPtr;
	obj->multState = *_curGobMultStateVarPtr;
	obj->nextState = *_curGobNextStateVarPtr;
	obj->xPos = *_curGobScrXVarPtr;
	obj->yPos = *_curGobScrYVarPtr;
	obj->left = *_curGobLeftVarPtr;
	obj->top = *_curGobTopVarPtr;
	obj->right = *_curGobRightVarPtr;
	obj->bottom = *_curGobBottomVarPtr;
	obj->doAnim = *_curGobDoAnimVarPtr;
	obj->order = *_curGobOrderVarPtr;
	obj->noTick = *_curGobNoTickVarPtr;
	obj->type = *_curGobTypeVarPtr;
	obj->maxTick = *_curGobMaxTickVarPtr;
	obj->tick = *_curGobTickVarPtr;
	obj->actionStartState = *_curGobActStartStateVarPtr;
	obj->curLookDir = *_curGobLookDirVarPtr;
	obj->pickable = *_curGobPickableVarPtr;
	obj->relaxTime = *_curGobRelaxVarPtr;

	if (_actDestItemDesc == 0)
		return;

	obj = _actDestItemDesc;

	obj->state = *_destItemStateVarPtr;
	obj->curFrame = *_destItemFrameVarPtr;
	obj->multState = *_destItemMultStateVarPtr;
	obj->nextState = *_destItemNextStateVarPtr;
	obj->xPos = *_destItemScrXVarPtr;
	obj->yPos = *_destItemScrYVarPtr;
	obj->left = *_destItemLeftVarPtr;
	obj->top = *_destItemTopVarPtr;
	obj->right = *_destItemRightVarPtr;
	obj->bottom = *_destItemBottomVarPtr;
	obj->doAnim = *_destItemDoAnimVarPtr;
	obj->order = *_destItemOrderVarPtr;
	obj->noTick = *_destItemNoTickVarPtr;
	obj->type = *_destItemTypeVarPtr;
	obj->maxTick = *_destItemMaxTickVarPtr;
	obj->tick = *_destItemTickVarPtr;
	obj->actionStartState = *_destItemActStartStVarPtr;
	obj->curLookDir = *_destItemLookDirVarPtr;
	obj->pickable = *_destItemPickableVarPtr;
	obj->relaxTime = *_destItemRelaxVarPtr;

	if (obj->type != _destItemType)
		obj->toRedraw = 1;

	if (obj->state != _destItemState && obj->type == 0)
		obj->toRedraw = 1;
}

void Goblin::pickItem(int16 indexToPocket, int16 idToPocket) {
	int16 x;
	int16 y;

	if (_objects[indexToPocket]->pickable != 1)
		return;

	_objects[indexToPocket]->type = 3;

	_itemIndInPocket = indexToPocket;
	_itemIdInPocket = idToPocket;

	for (y = 0; y < _vm->_map->_mapHeight; y++) {
		for (x = 0; x < _vm->_map->_mapWidth; x++) {
			if (_itemByteFlag == 1) {
				if (((_vm->_map->_itemsMap[y][x] & 0xff00) >> 8) ==
				    idToPocket)
					_vm->_map->_itemsMap[y][x] &= 0xff;
			} else {
				if ((_vm->_map->_itemsMap[y][x] & 0xff) == idToPocket)
					_vm->_map->_itemsMap[y][x] &= 0xff00;
			}
		}
	}

	if (idToPocket >= 0 && idToPocket < 20) {
		_vm->_map->_itemPoses[_itemIdInPocket].x = 0;
		_vm->_map->_itemPoses[_itemIdInPocket].y = 0;
		_vm->_map->_itemPoses[_itemIdInPocket].orient = 0;
	}
}

void Goblin::placeItem(int16 indexInPocket, int16 idInPocket) {
	Gob_Object *itemDesc;
	int16 lookDir;
	int16 xPos;
	int16 yPos;
	int16 layer;

	itemDesc = _objects[indexInPocket];
	lookDir = _goblins[0]->curLookDir & 4;

	xPos = _gobPositions[0].x;
	yPos = _gobPositions[0].y;

	_itemIndInPocket = -1;
	_itemIdInPocket = 0;

	itemDesc->pickable = 1;
	itemDesc->type = 0;
	itemDesc->toRedraw = 1;
	itemDesc->curFrame = 0;
	itemDesc->order = _goblins[0]->order;
	itemDesc->animation =
	    itemDesc->stateMach[itemDesc->state][0]->animation;
	layer =
	    itemDesc->stateMach[itemDesc->state][itemDesc->stateColumn]->layer;

	_vm->_scenery->updateAnim(layer, 0, itemDesc->animation, 0,
	    itemDesc->xPos, itemDesc->yPos, 0);

	itemDesc->yPos +=
	    (_gobPositions[0].y * 6) + 5 - _vm->_scenery->_toRedrawBottom;

	if (lookDir == 4) {
		itemDesc->xPos += (_gobPositions[0].x * 12 + 14)
		    - (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	} else {
		itemDesc->xPos += (_gobPositions[0].x * 12)
		    - (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	}

	_vm->_map->placeItem(xPos, yPos, idInPocket);

	if (yPos > 0) {
		_vm->_map->placeItem(xPos, yPos - 1, idInPocket);
	}

	if (lookDir == 4) {
		if (xPos < _vm->_map->_mapWidth - 1) {
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
		_vm->_map->_itemPoses[idInPocket].x = _gobPositions[0].x;
		_vm->_map->_itemPoses[idInPocket].y = _gobPositions[0].y;
		_vm->_map->_itemPoses[idInPocket].orient = lookDir;
		if (_vm->_map->_itemPoses[idInPocket].orient == 0) {
//                      _vm->_map->_itemPoses[idInPocket].x++;
			if (_vm->_map->getPass(_vm->_map->_itemPoses[idInPocket].x + 1, (int)_vm->_map->_itemPoses[idInPocket].y) == 1)
				_vm->_map->_itemPoses[idInPocket].x++;
		} else {
			if (_vm->_map->getPass(_vm->_map->_itemPoses[idInPocket].x - 1, (int)_vm->_map->_itemPoses[idInPocket].y) == 1)
				_vm->_map->_itemPoses[idInPocket].x--;
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

	pickObj = _objects[indexToPick];
	placeObj = _objects[_itemIndInPocket];

	idToPlace = _itemIdInPocket;
	pickObj->type = 3;
	_itemIndInPocket = indexToPick;
	_itemIdInPocket = idToPick;

	if (_itemByteFlag == 0) {
		for (y = 0; y < _vm->_map->_mapHeight; y++) {
			for (x = 0; x < _vm->_map->_mapWidth; x++) {
				if ((_vm->_map->_itemsMap[y][x] & 0xff) == idToPick)
					_vm->_map->_itemsMap[y][x] =
					    (_vm->_map->_itemsMap[y][x] & 0xff00) +
					    idToPlace;
			}
		}
	} else {

		for (y = 0; y < _vm->_map->_mapHeight; y++) {
			for (x = 0; x < _vm->_map->_mapWidth; x++) {
				if (((_vm->_map->_itemsMap[y][x] & 0xff00) >> 8) ==
				    idToPick)
					_vm->_map->_itemsMap[y][x] =
					    (_vm->_map->_itemsMap[y][x] & 0xff) +
					    (idToPlace << 8);
			}
		}
	}

	if (idToPick >= 0 && idToPick < 20) {
		_vm->_map->_itemPoses[idToPlace].x =
		    _vm->_map->_itemPoses[_itemIdInPocket].x;
		_vm->_map->_itemPoses[idToPlace].y =
		    _vm->_map->_itemPoses[_itemIdInPocket].y;
		_vm->_map->_itemPoses[idToPlace].orient =
		    _vm->_map->_itemPoses[_itemIdInPocket].orient;

		_vm->_map->_itemPoses[_itemIdInPocket].x = 0;
		_vm->_map->_itemPoses[_itemIdInPocket].y = 0;
		_vm->_map->_itemPoses[_itemIdInPocket].orient = 0;
	}

	_itemIndInPocket = -1;
	_itemIdInPocket = 0;

	placeObj->type = 0;
	placeObj->nextState = -1;
	placeObj->multState = -1;
	placeObj->unk14 = 0;
	placeObj->toRedraw = 1;
	placeObj->curFrame = 0;
	placeObj->order = _goblins[0]->order;

	placeObj->animation =
	    placeObj->stateMach[placeObj->state][0]->animation;

	layer =
	    placeObj->stateMach[placeObj->state][placeObj->stateColumn]->layer;
	_vm->_scenery->updateAnim(layer, 0, placeObj->animation, 0, placeObj->xPos,
	    placeObj->yPos, 0);

	placeObj->yPos +=
	    (_gobPositions[0].y * 6) + 5 - _vm->_scenery->_toRedrawBottom;

	if (_vm->_map->_itemPoses[idToPlace].orient == 4) {
		placeObj->xPos += (_gobPositions[0].x * 12 + 14)
		    - (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	} else {
		placeObj->xPos += (_gobPositions[0].x * 12)
		    - (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	}
}

void Goblin::treatItemPick(int16 itemId) {
	int16 itemIndex;
	Gob_Object *gobDesc;

	gobDesc = _goblins[_currentGoblin];

	if (gobDesc->curFrame != 9)
		return;

	if (gobDesc->stateMach != gobDesc->realStateMach)
		return;

	_readyToAct = 0;
	_goesAtTarget = 0;

	itemIndex = _itemToObject[itemId];
	if (itemId != 0 && itemIndex != -1
	    && _objects[itemIndex]->pickable != 1)
		itemIndex = -1;

	if (_itemIndInPocket != -1 && _itemIndInPocket == itemIndex)
		itemIndex = -1;

	if (_itemIndInPocket != -1 && itemIndex != -1
	    && _objects[itemIndex]->pickable == 1) {
		swapItems(itemIndex, itemId);
		_itemIndInPocket = itemIndex;
		_itemIdInPocket = itemId;
		return;
	}

	if (_itemIndInPocket != -1 && itemIndex == -1) {
		placeItem(_itemIndInPocket, _itemIdInPocket);
		return;
	}

	if (_itemIndInPocket == -1 && itemIndex != -1) {
		pickItem(itemIndex, itemId);
		return;
	}
}

int16 Goblin::treatItem(int16 action) {
	int16 state;

	state = _goblins[_currentGoblin]->state;
	if ((state == 10 || state == 11) &&
	    _goblins[_currentGoblin]->curFrame == 0) {
		_readyToAct = 0;
	}

	if (action == 3 && _currentGoblin == 0 &&
	    (state == 10 || state == 11) && _goblins[0]->curFrame == 0) {
		saveGobDataToVars(_gobPositions[_currentGoblin].x,
		    _gobPositions[_currentGoblin].y, 0);
		_goesAtTarget = 1;
		return -1;
	}

	if (_noPick == 0 && _currentGoblin == 0 &&
	    (state == 10 || state == 11)) {
		treatItemPick(_destActionItem);

		saveGobDataToVars(_gobPositions[_currentGoblin].x,
		    _gobPositions[_currentGoblin].y, 0);
		return 0;
	}

	if (_goesAtTarget == 0) {
		saveGobDataToVars(_gobPositions[_currentGoblin].x,
		    _gobPositions[_currentGoblin].y, 0);
		return 0;
	} else {

		if (_itemToObject[_destActionItem] != 100 &&
		    _destActionItem != 0) {

			if (_itemToObject[_destActionItem] == -1) {
				_actDestItemDesc = 0;
			} else {
				_actDestItemDesc =
				    _objects[_itemToObject
				    [_destActionItem]];
			}
		}

		_goesAtTarget = 0;
		saveGobDataToVars(_gobPositions[_currentGoblin].x,
		    _gobPositions[_currentGoblin].y, 0);
		return _destActionItem;
	}
}

void Goblin::sub_19BD3(void) {
	Mult::Mult_Object *obj0;
	Mult::Mult_Object *obj1;
	Mult::Mult_AnimData *anim0;
	Mult::Mult_AnimData *anim1;
	int16 pass;
	int16 gob1X;
	int16 gob2X;
	int16 gob1Y;
	int16 gob2Y;
	int16 var_A;
	int16 var_C;
	int16 di;
	int16 si;

	obj0 = &(_vm->_mult->_objects[0]);
	obj1 = &(_vm->_mult->_objects[1]);
	anim0 = obj0->pAnimData;
	anim1 = obj1->pAnimData;

	si = anim0->state;
	di = anim1->state;

	if (anim0->someFlag == 0) {
		if ((_word_2F9BC == 0) && (anim0->isStatic == 0)) {
			if ((VAR(_dword_2F9B6) == 0) && (si == 28)) {
				si = _vm->_util->getRandom(3) + 24;
				warning("GOB2 Stub! sub_195C7(0, si);");
				WRITE_VAR(_dword_2F9B6, 100);
			} else
				WRITE_VAR(_dword_2F9B6, VAR(_dword_2F9B6) - 1);
		}
		if ((si == 8) || (si == 9) || (si == 29))
			anim0->field_10 = 6;
	}
	if (anim1->someFlag == 0) {
		if ((_word_2F9BA == 0) && (anim1->isStatic == 0)) {
			if ((VAR(_dword_2F9B2) == 0) && (di == 28)) {
				di = _vm->_util->getRandom(3) + 24;
				warning("GOB2 Stub! sub_195C7(1, di);");
				WRITE_VAR(_dword_2F9B2, 100);
			} else
				WRITE_VAR(_dword_2F9B2, VAR(_dword_2F9B2) - 1);
		}
		if ((di == 8) || (di == 9) || (di == 29))
			anim1->field_10 = 6;
	}

	if ((anim0->someFlag == 1) && (anim0->isStatic == 0) &&
			((anim0->state == 28) || (anim0->state == 29)))
		anim0->field_10 = 0;
	if ((anim1->someFlag == 1) && (anim1->isStatic == 0) &&
			((anim1->state == 28) || (anim1->state == 29)))
		anim1->field_10 = 0;

	if (VAR(18) != ((uint32) -1)) {
		if (anim0->layer == 44)
			anim0->field_10 = 4;
		else if (anim0->layer == 45)
			anim0->field_10 = 0;
		if (anim0->someFlag == 0)
			anim0->field_10 = 6;
	}
	if (VAR(19) != ((uint32) -1)) {
		if (anim1->layer == 48)
			anim1->field_10 = 4;
		else if (anim1->layer == 49)
			anim1->field_10 = 0;
		if (anim1->someFlag == 0)
			anim1->field_10 = 6;
	}

	if ((anim0->layer == 45) && (anim0->field_10 == 4) && (anim0->field_12 == 5) &&
			(VAR(18) == ((uint32) -1)) && (_word_2F9C0 == 0)) {
		warning("GOB2 Stub! sub_195C7(0, 19);");
	}
	if ((anim0->layer == 44) && (anim0->field_10 == 0) && (anim0->field_12 == 5) &&
			(VAR(18) == ((uint32) -1)) && (_word_2F9C0 == 0)) {
		warning("GOB2 Stub! sub_195C7(0, 16);");
	}
	if ((anim1->layer == 49) && (anim1->field_10 == 4) && (anim1->field_12 == 5) &&
			(VAR(19) == ((uint32) -1)) && (_word_2F9BE == 0)) {
		warning("GOB2 Stub! sub_195C7(1, 19);");
	}
	if ((anim1->layer == 48) && (anim1->field_10 == 0) && (anim1->field_12 == 5) &&
			(VAR(19) == ((uint32) -1)) && (_word_2F9BE == 0)) {
		warning("GOB2 Stub! sub_195C7(1, 16);");
	}

	gob1X = obj0->goblinX;
	gob2X = obj1->goblinX;
	gob1Y = obj0->goblinY;
	gob2Y = obj1->goblinY;
	di = anim0->field_13;
	si = anim0->field_14;
	var_A = anim1->field_13;
	var_C = anim1->field_14;

	pass = _vm->_map->getPass(gob1X, gob1Y, 40);
	if ((pass > 17) && (pass < 21))
		warning("GOB2 Stub! sub_19AB7(anim0);");
	pass = _vm->_map->getPass(gob2X, gob2Y, 40);
	if ((pass > 17) && (pass < 21))
		warning("GOB2 Stub! sub_19B45(anim1);");

	if ((di < 0) || (di > 39) || (si < 0) || (si > 39))
		return;

	if (gob1Y > si) {
		if (_vm->_map->getPass(di, si, 40) > 17) {
			do {
				si--;
			} while (_vm->_map->getPass(di, si, 40) > 17);
			si++;
			if (_vm->_map->getPass(di - 1, si, 40) == 0) {
				if (_vm->_map->getPass(di + 1, si, 40) != 0)
					di++;
			} else
				di--;
			warning("GOB2 Stub! sub_197A6(di (=%d), si (=%d), 0);", si, di);
		}
	} else {
		if (_vm->_map->getPass(di, si, 40) > 17) {
			do {
				si++;
			} while (_vm->_map->getPass(di, si, 40) > 17);
			si--;
			if (_vm->_map->getPass(di - 1, si, 40) == 0) {
				if (_vm->_map->getPass(di + 1, si, 40) != 0)
					di++;
			} else
				di--;
			warning("GOB2 Stub! sub_197A6(di (=%d), si (=%d), 0);", si, di);
		}
	}
	if (gob2Y > var_C) {
		if (_vm->_map->getPass(var_A, var_C, 40) > 17) {
			do {
				var_C--;
			} while (_vm->_map->getPass(var_A, var_C, 40) > 17);
			var_C++;
			if (_vm->_map->getPass(var_A - 1, var_C, 40) == 0) {
				if (_vm->_map->getPass(var_A + 1, var_C, 40) != 0)
					var_A++;
			} else
				var_A--;
			warning("GOB2 Stub! sub_197A6(var_A (=%d), var_C (=%d), 1);", var_A, var_C);
		}
	} else {
		if (_vm->_map->getPass(var_A, var_C, 40) > 17) {
			do {
				var_C++;
			} while (_vm->_map->getPass(var_A, var_C, 40) > 17);
			var_C--;
			if (_vm->_map->getPass(var_A - 1, var_C, 40) == 0) {
				if (_vm->_map->getPass(var_A + 1, var_C, 40) != 0)
					var_A++;
			} else
				var_A--;
			warning("GOB2 Stub! sub_197A6(var_A (=%d), var_C (=%d), 1);", var_A, var_C);
		}
	}
}

} // End of namespace Gob
