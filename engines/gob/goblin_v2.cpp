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
#include "gob/mult.h"
#include "gob/game.h"
#include "gob/scenery.h"
#include "gob/map.h"

namespace Gob {

Goblin_v2::Goblin_v2(GobEngine *vm) : Goblin_v1(vm) {
	_gobsCount = -1;
}

void Goblin_v2::freeObjects(void) {
	int i;

	if (_gobsCount < 0)
		return;

	for (i = 0; i < _gobsCount; i++) {
		delete[] _vm->_mult->_objects[i].goblinStates[0];
		delete[] _vm->_mult->_objects[i].goblinStates;
	}
	for (i = 0; i < _vm->_goblin->_soundSlotsCount; i++)
		if ((_vm->_goblin->_soundSlots[i] & 0x8000) == 0)
			_vm->_game->freeSoundSlot(_vm->_goblin->_soundSlots[i]);
//	delete[] off_2F2AB;
	_gobsCount = -1;
}

void Goblin_v2::placeObject(Gob_Object *objDesc, char animated,
		int16 index, int16 x, int16 y, int16 state) {
	Mult::Mult_Object *obj;
	Mult::Mult_AnimData *objAnim;
	int16 layer;
	int16 animation;

	obj = &_vm->_mult->_objects[index];
	objAnim = obj->pAnimData;

	obj->goblinX = x;
	obj->goblinY = y;
	objAnim->order = y;

	if (state == -1) {
		objAnim->frame = 0;
		objAnim->isPaused = 0;
		objAnim->isStatic = 0;
		objAnim->newCycle = 0;
		_vm->_scenery->updateAnim(objAnim->layer, 0, objAnim->animation, 0,
				*obj->pPosX, *obj->pPosY, 0);
		if (!_vm->_map->_bigTiles)
			*obj->pPosY = (y + 1) * _vm->_map->_tilesHeight
				- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
		else
			*obj->pPosY = ((y + 1) / 2) * _vm->_map->_tilesHeight
				- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
		*obj->pPosX = x * _vm->_map->_tilesWidth;
	} else {
		if (obj->goblinStates[state] != 0) {
			layer = obj->goblinStates[state][0].layer;
			animation = obj->goblinStates[state][0].animation;
			objAnim->state = state;
			objAnim->layer = layer;
			objAnim->animation = animation;
			objAnim->frame = 0;
			objAnim->isPaused = 0;
			objAnim->isStatic = 0;
			objAnim->newCycle = _vm->_scenery->_animations[animation].layers[layer]->framesCount;
			_vm->_scenery->updateAnim(layer, 0, animation, 0, *obj->pPosX, *obj->pPosY, 0);
			if (!_vm->_map->_bigTiles)
				*obj->pPosY = (y + 1) * _vm->_map->_tilesHeight
					- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
			else
				*obj->pPosY = ((y + 1) / 2) * _vm->_map->_tilesHeight
					- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
			*obj->pPosX = x * _vm->_map->_tilesWidth;
			initiateMove(index);
		} else
			initiateMove(index);
	}
}

void Goblin_v2::initiateMove(int16 index) {
	Mult::Mult_Object *obj = &_vm->_mult->_objects[index];

	obj->destX = obj->gobDestX;
	obj->destY = obj->gobDestY;
	_vm->_map->findNearestToDest(index);
	_vm->_map->findNearestToGob(index);
	_vm->_map->optimizePoints(index, obj->goblinX, obj->goblinY);
	obj->pAnimData->field_12 = _vm->_map->checkDirectPath(index,
			obj->goblinX, obj->goblinY, obj->gobDestX, obj->gobDestY);
	if (obj->pAnimData->field_12 == 3) {
		obj->destX = _vm->_map->_wayPoints[obj->nearestWayPoint].x;
		obj->destY = _vm->_map->_wayPoints[obj->nearestWayPoint].y;
	}
}

} // End of namespace Gob
