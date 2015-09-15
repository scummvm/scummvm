/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/actor.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/boundingbox.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/waypoints.h"
#include "bladerunner/scene.h"
#include "bladerunner/items.h"

namespace BladeRunner {

Actor::Actor(BladeRunnerEngine *vm, int actorId) {
	_vm = vm;
	_id = actorId;

	_walkInfo      = new ActorWalk(vm);
	_movementTrack = new MovementTrack();
	_clues         = new ActorClues(vm, (actorId && actorId != 99) ? 2 : 4);
	_bbox          = new BoundingBox();
	_combatInfo    = new ActorCombat(vm);

	_friendlinessToOther = new int[_vm->_gameInfo->getActorCount()];
}

Actor::~Actor() {
	
	delete[] _friendlinessToOther;
	delete   _combatInfo;
	delete   _bbox;
	delete   _clues;
	delete   _movementTrack;
	delete   _walkInfo;
	
}

void Actor::setup(int actorId) {
	_id  = actorId;
	_setId = -1;

	_position     = Vector3(0.0, 0.0, 0.0);
	_facing       = 512;
	_targetFacing = -1;
	_walkboxId    = -1;

	_animationId = 0;
	_animationFrame = 0;
	_fps = 15;
	_frame_ms = 1000 / _fps;

	_isTargetable        = false;
	_isInvisible         = false;
	_isImmuneToObstacles = false;
	
	_isRetired           = false;

	_width         = 0;
	_height        = 0;
	_retiredWidth  = 0;
	_retiredHeight = 0;

	for (int i = 0; i != 7; ++i) {
		_timersRemain[i] = 0;
		_timersBegan[i]  = _vm->getTotalPlayTime();
	}

	_scale = 1.0;

	_honesty              = 50;
	_intelligence         = 50;
	_combatAggressiveness = 50;
	_stability            = 50;

	_currentHP            = 50;
	_maxHP                = 50;
	_goalNumber           = -1;

	_timersRemain[4] = 60000;
	_animationMode = -1;
	// TODO: screen rect = { -1, -1, -1, -1 };

	int actorCount = (int)_vm->_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i)
		_friendlinessToOther[i] = 50;

	// TODO: Setup clues

	// TODO: Flush movement track
}

void Actor::set_at_xyz(Vector3 position, int facing, bool halfOrSet, int moving, bool retired) {
	_position = position;
	setFacing(facing, halfOrSet);
	
	if(_vm->_scene->_setId == _setId) {
		_walkboxId = _vm->_scene->_set->findWalkbox(position.x, position.y);
	}else {
		_walkboxId = -1;
	}

	setBoundingBox(position, retired);

	_vm->_sceneObjects->remove(_id);

	if(_vm->_scene->getSetId() == _setId) {
		_vm->_sceneObjects->addActor(_id, _bbox, &_screenRectangle, 1, moving, _isTargetable, retired);
	}
}


void Actor::set_at_waypoint(int waypointId, int angle, int unknown, bool retired) {
	Vector3 waypointPosition;
	_vm->_waypoints->getXyz(waypointId, &waypointPosition.x, &waypointPosition.y, &waypointPosition.z);
	set_at_xyz(waypointPosition, angle, true, unknown, retired);
}

void Actor::draw() {
	Vector3 draw_position(_position.x, -_position.z, _position.y + 2.0);
	float   draw_facing = _facing * M_PI / 512.0;
	// just for viewing animations _facing = (_facing + 10) % 1024;
	// float   draw_scale  = _scale;

	// TODO: Handle SHORTY mode

	_vm->_sliceRenderer->setupFrame(19, 1, draw_position, M_PI - draw_facing);
	_vm->_sliceRenderer->drawFrame(_vm->_surface2, _vm->_zBuffer2);
}


int Actor::getSetId() {
	return _setId;
}

void Actor::setSetId(int setId) {
	if (_setId == setId) {
		return;
	}

	int i;

	if(_setId > 0) {
		for (i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
			if (_vm->_actors[i]->_id != _id && _vm->_actors[i]->_setId == _setId) {
				//actorScript->OtherAgentExitedThisScene( i, _id);
			}
		}
	}
	_setId = setId;
	//actorScript->EnteredScene(_id, set);
	if (_setId > 0) {
		for (i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
			if (_vm->_actors[i]->_id != _id && _vm->_actors[i]->_setId == _setId) {
				//actorScript->OtherAgentEnteredThisScene(i, _id);
			}
		}
	}
}


void Actor::setFacing(int facing, bool halfOrSet) {
	if (facing < 0 || facing >= 1024) {
		return;
	}

	if (halfOrSet) {
		_facing = facing;
		return;
	}

	int cw;
	int ccw;
	int offset;

	if (facing > _facing) {
		cw = facing - _facing;
		ccw = _facing + 1024 - facing;
	} else {
		ccw = _facing - facing;
		cw = facing + 1024 - _facing;
	}
	if (cw < ccw) {
		if (cw <= 32) {
			offset = cw;
		} else {
			offset = cw / 2;
		}
	} else {
		if (ccw <= 32) {
			offset = -ccw;
		} else {
			offset = -ccw / 2;
		}
	}
	_facing = (_facing + offset) % 1024;
}

void Actor::setBoundingBox(Vector3 position, bool retired) {
	if(retired) {
		
	}else {
		_bbox->setXyz(position.x - 12.0f, position.y + 6.0f, position.z - 12.0f, position.x + 12.0f, position.y + 72.0f, position.z + 12.0f);
	}
}


void Actor::changeAnimationMode(int animationMode, int force) {
	if (force == 1) {
		_animationMode = -1;
	}
	if(animationMode != _animationMode) {
		//TODO: _vm->actorScript->ChangeAnimationMode(_id, animationMode);
		_animationMode = animationMode;
	}

}

bool Actor::isWalking() {
	return _walkInfo->isWalking();
}

void Actor::stopWalking(int value) {
	if (value == 1 && _id == 0) {
		_vm->_playerActorIdle = true;
	}

	if(isWalking()) {
		_walkInfo->stop(_id, 1, _combatAnimationMode, 0);
	} else if(inCombat()) {
		changeAnimationMode(_combatAnimationMode, 0);
	} else {
		changeAnimationMode(0, 0);
	}
}

void Actor::faceActor(int otherActorId, bool animate) {
	if (_setId != _vm->_scene->_setId) {
		return;
	}
	
	Actor *otherActor = _vm->_actors[otherActorId];

	if (_setId != otherActor->_setId) {
		return;
	}

	faceXYZ(otherActor->_position.x, otherActor->_position.y, otherActor->_position.z, animate);
}

void Actor::faceObject(char *objectName, bool animate) {
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1) {
		return;
	}

	BoundingBox boundingBox;
	_vm->_scene->objectGetBoundingBox(objectId, &boundingBox);

	float x0, y0, z0, x1, y1, z1;
	boundingBox.getXyz(&x0, &y0, &z0, &x1, &y1, &z1);

	float x = (x1 + x0) / 2.0f;
	float z = (z1 + z0) / 2.0f;
	faceXYZ(x, y0, z, animate);
}

void Actor::faceItem(int itemId, bool animate) {
	float x, y, z;
	_vm->_items->getXyz(itemId, &x, &y, &z);
	faceXYZ(x, y, z, animate);
}

void Actor::faceWaypoint(int waypointId, bool animate) {
	float x, y, z;
	_vm->_waypoints->getXyz(waypointId, &x, &y, &z);
	faceXYZ(x, y, z, animate);
}

void Actor::faceXYZ(float x, float y, float z, bool animate) {
	if (isWalking()) {
		stopWalking(0);
	}
	if (x == _position.x && z == _position.z) {
		return;
	}

	int heading = int(512.0f * atan2f(_position.x - x, _position.z - z) / M_PI) % 1024;
	faceHeading(heading, animate);
}

void Actor::faceCurrentCamera(bool animate) {
	faceXYZ(_vm->_view->_cameraPosition.x, _vm->_view->_cameraPosition.y, -_vm->_view->_cameraPosition.z, animate);
}

void Actor::faceHeading(int heading, bool animate) {
	if (heading != _facing) {
		if (animate) {
			_targetFacing = heading;
		}
		else {
			setFacing(heading, true);
		}
	}
}

int Actor::getFriendlinessToOther(int otherActorId) {
	return _friendlinessToOther[otherActorId];
}

void Actor::modifyFriendlinessToOther(int otherActorId, signed int change) {
	_friendlinessToOther[otherActorId] = MIN(MAX(_friendlinessToOther[otherActorId] + change, 0), 100);
}

void Actor::setFriendlinessToOther(int otherActorId, int friendliness) {
	_friendlinessToOther[otherActorId] = friendliness;
}

void Actor::setHonesty(int honesty) {
	_honesty = honesty;
}

void Actor::setIntelligence(int intelligence) {
	_intelligence = intelligence;
}

void Actor::setStability(int stability) {
	_stability = stability;
}

void Actor::setCombatAggressiveness(int combatAggressiveness) {
	_combatAggressiveness = combatAggressiveness;
}

int Actor::getCurrentHP() {
	return _currentHP;
}

int Actor::getMaxHP() {
	return _maxHP;
}

int Actor::getCombatAggressiveness() {
	return _combatAggressiveness;
}

int Actor::getHonesty() {
	return _honesty;
}

int Actor::getIntelligence() {
	return _intelligence;
}

int Actor::getStability() {
	return _stability;
}

void Actor::modifyCurrentHP(signed int change) {
	_currentHP = MIN(MAX(_currentHP + change, 0), 100);
	if (_currentHP > 0)
		retire(0, 0, 0, -1);
}

void Actor::modifyMaxHP(signed int change) {
	_maxHP = MIN(MAX(_maxHP + change, 0), 100);
}

void Actor::modifyCombatAggressiveness(signed int change) {
	_combatAggressiveness = MIN(MAX(_combatAggressiveness + change, 0), 100);
}

void Actor::modifyHonesty(signed int change) {
	_honesty = MIN(MAX(_honesty + change, 0), 100);
}

void Actor::modifyIntelligence(signed int change) {
	_intelligence = MIN(MAX(_intelligence + change, 0), 100);
}

void Actor::modifyStability(signed int change) {
	_stability = MIN(MAX(_stability + change, 0), 100);
}

void Actor::setFlagDamageAnimIfMoving(bool value) {
	_damageAnimIfMoving = value;
}

bool Actor::getFlagDamageAnimIfMoving() {
	return _damageAnimIfMoving;
}

void Actor::retire(bool isRetired, int width, int height, int retiredByActorId) {
	_isRetired = isRetired;
	_retiredWidth = MAX(width, 0);
	_retiredHeight = MAX(height, 0);
	if(_id == 0 && isRetired) {
		_vm->playerLosesControl();
		_vm->_playerDead = true;
	}
	if(isRetired) {
		//TODO: _vm->actorScript->Retired(_id, retiredByActorId);
	}
}
} // End of namespace BladeRunner
