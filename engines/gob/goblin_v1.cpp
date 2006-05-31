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
#include "gob/goblin.h"
#include "gob/scenery.h"
#include "gob/map.h"

namespace Gob {

Goblin_v1::Goblin_v1(GobEngine *vm) : Goblin(vm) {
}

void Goblin_v1::freeObjects(void) {
	int16 i;
	int16 state;
	int16 col;

	for (i = 0; i < 16; i++) {
		if (_soundData[i] == 0)
			continue;

		_vm->_snd->freeSoundDesc(_soundData[i]);
		_soundData[i] = 0;
	}

	for (i = 0; i < 4; i++) {
		if (_goblins[i] == 0)
			continue;

		_goblins[i]->stateMach = _goblins[i]->realStateMach;

		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				delete _goblins[i]->stateMach[state][col];
				_goblins[i]->stateMach[state][col] = 0;
			}
		}

		if (i == 3) {
			for (state = 40; state < 70; state++) {
				delete _goblins[3]->stateMach[state][0];
				_goblins[3]->stateMach[state][0] = 0;
			}
		}

		delete[] _goblins[i]->stateMach;
		delete _goblins[i];
		_goblins[i] = 0;
	}

	for (i = 0; i < 20; i++) {
		if (_objects[i] == 0)
			continue;

		_objects[i]->stateMach = _objects[i]->realStateMach;

		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				delete _objects[i]->stateMach[state][col];
				_objects[i]->stateMach[state][col] = 0;
			}
		}

		delete[] _objects[i]->stateMach;
		delete _objects[i];
		_objects[i] = 0;
	}
}

void Goblin_v1::placeObject(Gob_Object *objDesc, char animated,
		int16 index, int16 x, int16 y, int16 state) {
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

		objDesc->order = _vm->_scenery->_toRedrawBottom / 24 + 3;

		objDesc->left = objDesc->xPos;
		objDesc->right = objDesc->xPos;
		objDesc->dirtyLeft = objDesc->xPos;
		objDesc->dirtyRight = objDesc->xPos;

		objDesc->top = objDesc->yPos;
		objDesc->bottom = objDesc->yPos;
		objDesc->dirtyTop = objDesc->yPos;
		objDesc->dirtyBottom = objDesc->yPos;

		_vm->_util->listInsertBack(_objList, objDesc);
	}
}

void Goblin_v1::initiateMove(int16 index) {
	_vm->_map->findNearestToDest(0);
	_vm->_map->findNearestToGob(0);
	_vm->_map->optimizePoints(0, 0, 0);

	_pathExistence = _vm->_map->checkDirectPath(-1, _vm->_map->_curGoblinX, _vm->_map->_curGoblinY,
	    _pressedMapX, _pressedMapY);

	if (_pathExistence == 3) {
		if (_vm->_map->checkLongPath(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY,
			_pressedMapX, _pressedMapY,
			_vm->_map->_nearestWayPoint, _vm->_map->_nearestDest) == 0) {
			_pathExistence = 0;
		} else {
			_vm->_map->_destX = _vm->_map->_wayPoints[_vm->_map->_nearestWayPoint].x;
			_vm->_map->_destY = _vm->_map->_wayPoints[_vm->_map->_nearestWayPoint].y;
		}
	}
}

} // End of namespace Gob
