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

#include "bladerunner/actor_clues.h"
#include "bladerunner/actor_combat.h"
#include "bladerunner/actor_walk.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/boundingbox.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/items.h"
#include "bladerunner/mouse.h"
#include "bladerunner/movement_track.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/scene.h"
#include "bladerunner/script/ai.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/waypoints.h"
#include "bladerunner/zbuffer.h"

namespace BladeRunner {

Actor::Actor(BladeRunnerEngine *vm, int actorId) {
	_vm = vm;
	_id = actorId;

	_walkInfo = new ActorWalk(vm);
	_movementTrack = new MovementTrack();
	_clues = new ActorClues(vm, (actorId && actorId != 99) ? 2 : 4);
	_bbox = new BoundingBox();
	_combatInfo = new ActorCombat(vm);

	_friendlinessToOther = new int[_vm->_gameInfo->getActorCount()];
}

Actor::~Actor() {
	delete[] _friendlinessToOther;
	delete _combatInfo;
	delete _bbox;
	delete _clues;
	delete _movementTrack;
	delete _walkInfo;
}

void Actor::setup(int actorId) {
	_id = actorId;
	_setId = -1;

	_position = Vector3(0.0, 0.0, 0.0);
	_facing = 512;
	_targetFacing = -1;
	_walkboxId = -1;

	_animationId = 0;
	_animationFrame = 0;
	_fps = 15;
	_frame_ms = 1000 / _fps;

	_isMoving = false;
	_isTargetable = false;
	_inCombat = false;
	_isInvisible = false;
	_isImmuneToObstacles = false;

	_isRetired = false;

	_width = 0;
	_height = 0;
	_retiredWidth = 0;
	_retiredHeight = 0;

	_movementTrackWalkingToWaypointId = -1;
	_movementTrackDelayOnNextWaypoint = -1;

	for (int i = 0; i != 7; ++i) {
		_timersRemain[i] = 0;
		_timersStart[i] = _vm->getTotalPlayTime();
	}

	_scale = 1.0;

	_honesty = 50;
	_intelligence = 50;
	_combatAggressiveness = 50;
	_stability = 50;

	_currentHP = 50;
	_maxHP = 50;
	_goalNumber = -1;

	_movementTrackPaused = false;
	_movementTrackNextWaypointId = -1;
	_movementTrackNextDelay = -1;
	_movementTrackNextAngle = -1;
	_movementTrackNextRunning = false;

	_timersRemain[4] = 60000;
	_animationMode = -1;
	_screenRectangle = Common::Rect(-1, -1, -1, -1);

	_animationModeCombatIdle = kAnimationModeCombatIdle;
	_animationModeCombatWalk = kAnimationModeCombatWalk;
	_animationModeCombatRun = kAnimationModeCombatRun;

	int actorCount = (int)_vm->_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i)
		_friendlinessToOther[i] = 50;

	_combatInfo->setup();
	_clues->removeAll();
	_movementTrack->flush();

	_actorSpeed = Vector3();
}

void Actor::changeAnimationMode(int animationMode, bool force) {
	if (force) {
		_animationMode = -1;
	}

	if (animationMode != _animationMode) {
		_vm->_aiScripts->ChangeAnimationMode(_id, animationMode);
		_animationMode = animationMode;
	}
}

void Actor::setFPS(int fps) {
	_fps = fps;

	if (fps == 0) {
		_frame_ms = 0;
	} else if (fps == -1) {
		_frame_ms = -1000;
	} else if (fps == -2) {
		_fps = _vm->_sliceAnimations->getFPS(_animationId);
		_frame_ms = 1000 / _fps;
	} else {
		_frame_ms = 1000 / fps;
	}
}

void Actor::countdownTimerStart(int timerId, int interval) {
	assert(timerId >= 0 && timerId < 7);
	_timersRemain[timerId] = interval;
	_timersStart[timerId] = _vm->getTotalPlayTime();
}

void Actor::countdownTimerReset(int timerId) {
	assert(timerId >= 0 && timerId < 7);
	_timersRemain[timerId] = 0;
}

int Actor::countdownTimerGetRemainingTime(int timerId) {
	assert(timerId >= 0 && timerId < 7);
	return _timersRemain[timerId];
}

void Actor::countdownTimersUpdate() {
	for (int i = 0; i <= 6; i++) {
		countdownTimerUpdate(i);
	}
}

void Actor::countdownTimerUpdate(int timerId) {
	if (_timersRemain[timerId] == 0) {
		return;
	}

	uint32 now = _vm->getTotalPlayTime();
	int tickInterval = now - _timersStart[timerId];
	_timersStart[timerId] = now;

	//warning("tickInterval: %d", tickInterval);
	_timersRemain[timerId] -= tickInterval;

	if (_timersRemain[timerId] <= 0) {
		switch (timerId) {
		case 0:
		case 1:
		case 2:
			if (!_vm->_aiScripts->IsInsideScript() && !_vm->_sceneScript->IsInsideScript()) {
				_vm->_aiScripts->TimerExpired(this->_id, timerId);
				this->_timersRemain[timerId] = 0;
			} else {
				this->_timersRemain[timerId] = 1;
			}
			break;
		case 3:
			_timersRemain[3] = 0;
			if (_movementTrack->isPaused()) {
				_timersRemain[3] = 1;
			} else {
				movementTrackNext(false);
			}
			break;
		case 4:
			// Something timer
			break;
		case 5:
			// Actor animation frame timer
			break;
		case 6:
			if (isRunning()) {
				if (_fps > 15) {
					int newFps = _fps - 2;
					if (newFps < 15) {
						newFps = 15;
					}
					setFPS(newFps);
				}
			}
			_timersRemain[6] = 200;
			break;
		}
	}
}

void Actor::movementTrackNext(bool omitAiScript) {
	bool hasNextMovement;
	int waypointSetId;
	int running;
	int angle;
	int delay;
	int waypointId;
	Vector3 waypointPosition;
	bool arrived;

	hasNextMovement = _movementTrack->next(&waypointId, &delay, &angle, &running);
	_movementTrackNextWaypointId = waypointId;
	_movementTrackNextDelay = delay;
	_movementTrackNextAngle = angle;
	_movementTrackNextRunning = running;
	if (hasNextMovement) {
		if (angle == -1) {
			angle = 0;
		}
		waypointSetId = _vm->_waypoints->getSetId(waypointId);
		_vm->_waypoints->getXYZ(waypointId, &waypointPosition.x, &waypointPosition.y, &waypointPosition.z);
		if (_setId == waypointSetId && waypointSetId == _vm->_actors[0]->_setId) {
			stopWalking(false);
			_walkInfo->setup(_id, running, _position, waypointPosition, false, &arrived);

			_movementTrackWalkingToWaypointId = waypointId;
			_movementTrackDelayOnNextWaypoint = delay;
			if (arrived) {
				movementTrackWaypointReached();
			}
		} else {
			setSetId(waypointSetId);
			setAtXYZ(waypointPosition, angle, true, false, false);

			if (!delay) {
				delay = 1;
			}
			if (delay > 1) {
				changeAnimationMode(kAnimationModeIdle, false);
			}
			countdownTimerStart(3, delay);
		}
		//return true;
	} else {
		if (!omitAiScript) {
			_vm->_aiScripts->CompletedMovementTrack(_id);
		}
		//return false;
	}
}

void Actor::movementTrackPause() {
	_movementTrack->pause();
	if (isWalking()) {
		_movementTrackPaused = true;
		stopWalking(false);
	} else {
		_movementTrackPaused = false;
	}
}

void Actor::movementTrackUnpause() {
	Vector3 waypointPosition;
	bool arrived;

	_movementTrack->unpause();
	if (_movementTrackNextWaypointId >= 0 && _movementTrackPaused) {
		_vm->_waypoints->getXYZ(_movementTrackNextWaypointId, &waypointPosition.x, &waypointPosition.y, &waypointPosition.z);
		_walkInfo->setup(_id, _movementTrackNextRunning, _position, waypointPosition, false, &arrived);
		_movementTrackPaused = false;
	}
}

void Actor::movementTrackWaypointReached() {
	int seconds;
	if (!_movementTrack->isPaused() && _id != 0) {
		if (_movementTrackWalkingToWaypointId >= 0 && _movementTrackDelayOnNextWaypoint) {
			if (!_movementTrackDelayOnNextWaypoint) {
				_movementTrackDelayOnNextWaypoint = 1;
			}
			if (_vm->_aiScripts->ReachedMovementTrackWaypoint(_id, _movementTrackWalkingToWaypointId)) {
				seconds = _movementTrackDelayOnNextWaypoint;
				if (seconds > 1) {
					changeAnimationMode(kAnimationModeIdle, false);
					seconds = _movementTrackDelayOnNextWaypoint; // todo: analyze if movement is changed in some aiscript->ChangeAnimationMode?
				}
				countdownTimerStart(3, seconds);
			}
		}
		_movementTrackWalkingToWaypointId = -1;
		_movementTrackDelayOnNextWaypoint = 0;
	}
}

bool Actor::loopWalkToActor(int otherActorId, int destinationOffset, int a3, bool run, bool a5, bool *flagIsRunning) {
	return loopWalk(_vm->_actors[otherActorId]->_position, destinationOffset, a3, run, _position, 24.0f, 24.0f, a5, flagIsRunning, false);
}

bool Actor::loopWalkToItem(int itemId, int destinationOffset, int a3, bool run, bool a5, bool *flagIsRunning) {
	float x, y, z;
	int width, height;
	_vm->_items->getXYZ(itemId, &x, &y, &z);
	_vm->_items->getWidthHeight(itemId, &width, &height);
	Vector3 itemPosition(x, y, z);

	return loopWalk(itemPosition, destinationOffset, a3, run, _position, width, 24.0f, a5, flagIsRunning, false);
}

void Actor::setAtXYZ(const Vector3 &position, int facing, bool snapFacing, bool moving, bool retired) {
	_position = position;
	setFacing(facing, snapFacing);

	if (_vm->_scene->getSetId() == _setId) {
		_walkboxId = _vm->_scene->_set->findWalkbox(_position.x, _position.y);
	} else {
		_walkboxId = -1;
	}

	setBoundingBox(_position, retired);

	_vm->_sceneObjects->remove(_id + SCENE_OBJECTS_ACTORS_OFFSET);

	if (_vm->_scene->getSetId() == _setId) {
		_vm->_sceneObjects->addActor(_id + SCENE_OBJECTS_ACTORS_OFFSET, _bbox, &_screenRectangle, 1, moving, _isTargetable, retired);
	}
}

void Actor::setAtWaypoint(int waypointId, int angle, int moving, bool retired) {
	Vector3 waypointPosition;
	_vm->_waypoints->getXYZ(waypointId, &waypointPosition.x, &waypointPosition.y, &waypointPosition.z);
	setAtXYZ(waypointPosition, angle, true, moving, retired);
}

bool Actor::loopWalk(const Vector3 &destination, int destinationOffset, bool a3, bool run, const Vector3 &start, float targetWidth, float targetSize, bool a8, bool *flagIsRunning, bool async) {
	if (true) { // simple walking
		*flagIsRunning = false;
		bool arrived;
		_walkInfo->setup(_id, false, _position, destination, false, &arrived);

		for (;;) {
			_vm->gameTick();
			if (!_walkInfo->isWalking() && !_walkInfo->isRunning())
				break;
			if (!_vm->_gameIsRunning)
				break;
		}
		return false;
	} else {
		//TODO:
		// original code, not yet working
		*flagIsRunning = false;

		if (destinationOffset > 0) {
			float dist = distance(_position, destination);
			if (dist - targetSize <= destinationOffset) {
				return false;
			}
		}

		if (a8 && !async && _id && destinationOffset <= 24) {
			if (distance(_vm->_playerActor->_position, destination) <= 24.0f) {
				_vm->_playerActor->walkToNearestPoint(destination, 48.0f);
			}
		}

		if (_id) {
			a3 = false;
		}

		Vector3 destinationX(destination);

		if (destinationOffset > 0) {
			walkFindU2(&destinationX, targetWidth, destinationOffset, targetSize, _position, destination);
		}

		bool walking = walkTo(run, destinationX, a8);

		if (async) {
			return false;
		}

		if (!walking && destinationOffset > 0) {
			walking = walkTo(run, destination, a8);
		}

		if (!walking) {
			return false;
		}

		if (async) {
			return false;
		}
		if (_id) {
			_vm->_mouse->disable();
		}
		if (a3) {
			//			TODO:
			//			dword_482990 = 1;
			//			dword_482994 = 0;
		} else {
			_vm->playerLosesControl();
		}

		if (a8) {
			_inWalkLoop = true;
		}

		bool v46 = false;
		while (_walkInfo->isWalking() && _vm->_gameIsRunning) {
			if (_walkInfo->isRunning()) {
				*flagIsRunning = true;
			}
			_vm->gameTick();
			if (_id == 0 && a3 /*&& dword_482994*/) {
				stopWalking(false);
				v46 = true;
			}
		}
		if (a8) {
			_inWalkLoop = false;
		}
		if (a3) {
			//			dword_482990 = 1;
		} else {
			_vm->playerGainsControl();
		}
		if (!v46 && destinationOffset == 0 /* && !PlayerActorIdle*/) {
			setAtXYZ(destination, _facing, true, false, false);
		}
		if (_id) {
			_vm->_mouse->enable();
		}
		return v46;
	}
}

bool Actor::walkTo(bool run, const Vector3 &destination, bool a3) {
	bool arrived;

	return _walkInfo->setup(_id, run, _position, destination, a3, &arrived);
}

bool Actor::loopWalkToXYZ(const Vector3 &destination, int destinationOffset, bool a3, bool run, bool a5, bool *flagIsRunning) {
	return loopWalk(destination, destinationOffset, a3, run, _position, 0.0f, 24.0f, a5, flagIsRunning, false);
}

bool Actor::loopWalkToSceneObject(const char *objectName, int destinationOffset, bool a3, bool run, bool a5, bool *flagIsRunning) {
	int sceneObject = _vm->_scene->_set->findObject(objectName);
	if (sceneObject < 0) {
		return true;
	}

	BoundingBox bbox;
	if (!_vm->_scene->_set->objectGetBoundingBox(sceneObject, &bbox)) {
		return true;
	}

	float x0, y0, z0, x1, y1, z1;
	bbox.getXYZ(&x0, &y0, &z0, &x1, &y1, &z1);

	float closestDistance = distance(_position.x, _position.z, x0, z0);
	float closestX = x0;
	float closestZ = z0;

	float d = distance(_position.x, _position.z, x1, z0);
	if (d < closestDistance) {
		closestX = x1;
		closestZ = z0;
		closestDistance = d;
	}

	d = distance(_position.x, _position.z, x1, z1);
	if (d < closestDistance) {
		closestX = x1;
		closestZ = z1;
		closestDistance = d;
	}

	d = distance(_position.x, _position.z, x0, z1);
	if (d < closestDistance) {
		closestX = x0;
		closestZ = z1;
		closestDistance = d;
	}
	bool inWalkbox;
	float y = _vm->_scene->_set->getAltitudeAtXZ(closestX, closestZ, &inWalkbox);
	Vector3 destination(closestX, y, closestZ);

	return loopWalk(destination, destinationOffset, a3, run, _position, 0.0f, 24.0f, a5, flagIsRunning, false);
}

bool Actor::loopWalkToWaypoint(int waypointId, int destinationOffset, int a3, bool run, bool a5, bool *flagIsRunning) {
	Vector3 waypointPosition;
	_vm->_waypoints->getXYZ(waypointId, &waypointPosition.x, &waypointPosition.y, &waypointPosition.z);
	return loopWalk(waypointPosition, destinationOffset, a3, run, _position, 0.0f, 24.0f, a5, flagIsRunning, false);
}

bool Actor::tick(bool forceDraw, Common::Rect *screenRect) {
	int remain = 0;
	bool needsUpdate = false;
	if (_fps > 0) {
		countdownTimerUpdate(5);
		remain = countdownTimerGetRemainingTime(5);
		needsUpdate = remain <= 0;
	} else if (forceDraw) {
		needsUpdate = true;
		remain = 0;
	}

	if (needsUpdate) {
		int newAnimation = 0, newFrame = 0;
		_vm->_aiScripts->UpdateAnimation(_id, &newAnimation, &newFrame);

		if (_animationId != newAnimation) {
			if (_fps != 0 && _fps != -1) {
				_animationId = newAnimation;
				setFPS(-2);
			}
		}
		_animationId = newAnimation;
		_animationFrame = newFrame;

		Vector3 positionChange = _vm->_sliceAnimations->getPositionChange(_animationId);
		float angleChange = _vm->_sliceAnimations->getFacingChange(_animationId);

		if (_id == 47) {
			positionChange.x = 0.0f;
			positionChange.y = 0.0f;
			positionChange.z = 0.0f;
		}

		if (isWalking()) {
			if (0.0f <= positionChange.y) {
				positionChange.y = -4.0f;
			}

			this->_targetFacing = -1;

			bool walked = _walkInfo->tick(_id, -positionChange.y, false);

			Vector3 pos;
			int facing;
			_walkInfo->getCurrentPosition(_id, &pos, &facing);
			setAtXYZ(pos, facing, false, this->_isMoving, false);
			if (walked) {
				_vm->_actors[_id]->changeAnimationMode(kAnimationModeIdle);

				this->movementTrackWaypointReached();
				if (this->inCombat()) {
					this->changeAnimationMode(this->_animationModeCombatIdle, false);
				} else {
					this->changeAnimationMode(kAnimationModeIdle, false);
				}
			}
		} else {
			if (angleChange != 0.0f) {
				int facingChange = angleChange * (512.0f / M_PI);
				if (facingChange != 0) {
					this->_facing = this->_facing - facingChange;
					if (this->_facing < 0) {
						this->_facing += 1024;
					}

					if (this->_facing >= 1024) {
						this->_facing = this->_facing - 1024;
					}
				}
			}

			if (0.0f != positionChange.x || 0.0f != positionChange.y || 0.0f != positionChange.z) {
				if (this->_actorSpeed.x != 0.0f) {
					positionChange.x = positionChange.x * this->_actorSpeed.x;
				}
				if (this->_actorSpeed.y != 0.0f) {
					positionChange.y = positionChange.y * this->_actorSpeed.y;
				}
				if (this->_actorSpeed.z != 0.0f) {
					positionChange.z = positionChange.z * this->_actorSpeed.z;
				}

				float angle = _facing * (M_PI / 512.0f);
				float sinx = sin(angle);
				float cosx = cos(angle);

				float originalX = this->_position.x;
				float originalY = this->_position.y;
				float originalZ = this->_position.z;

				this->_position.x = this->_position.x + positionChange.x * cosx - positionChange.y * sinx;
				this->_position.z = this->_position.z + positionChange.x * sinx + positionChange.y * cosx;
				this->_position.y = this->_position.y + positionChange.z;

				if (_vm->_sceneObjects->existsOnXZ(this->_id + SCENE_OBJECTS_ACTORS_OFFSET, this->_position.x, this->_position.z, false, false) == 1 && !this->_isImmuneToObstacles) {
					this->_position.x = originalX;
					this->_position.y = originalY;
					this->_position.z = originalZ;
				}
				setAtXYZ(this->_position, this->_facing, true, this->_isMoving, this->_isRetired);
			}
		}
	}

	bool isVisible = false;
	if (!_isInvisible) {
		isVisible = draw(screenRect);
		if (isVisible) {
			_screenRectangle = *screenRect;
		}
	}

	if (needsUpdate) {
		int nextFrameTime = remain + _frame_ms;
		if (nextFrameTime <= 0)
			nextFrameTime = 1;
		countdownTimerStart(5, nextFrameTime);
	}
	if (this->_targetFacing >= 0) {
		if (this->_targetFacing == this->_facing) {
			this->_targetFacing = -1;
		} else {
			this->setFacing(this->_targetFacing, false);
		}
	}
	return isVisible;
}

bool Actor::draw(Common::Rect *screenRect) {
	Vector3 drawPosition(_position.x, -_position.z, _position.y + 2.0);
	float drawAngle = M_PI - _facing * (M_PI / 512.0f);
	float drawScale = _scale;

	// TODO: Handle SHORTY mode

	_vm->_sliceRenderer->drawInWorld(_animationId, _animationFrame, drawPosition, drawAngle, drawScale, _vm->_surface2, _vm->_zbuffer->getData());
	_vm->_sliceRenderer->getScreenRectangle(screenRect, _animationId, _animationFrame, drawPosition, drawAngle, drawScale);

	return !screenRect->isEmpty();
}

int Actor::getSetId() {
	return _setId;
}

void Actor::setSetId(int setId) {
	if (_setId == setId) {
		return;
	}

	int i;

	if (_setId > 0) {
		for (i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
			if (_vm->_actors[i]->_id != _id && _vm->_actors[i]->_setId == _setId) {
				_vm->_aiScripts->OtherAgentExitedThisScene(i, _id);
			}
		}
	}
	_setId = setId;
	_vm->_aiScripts->EnteredScene(_id, _setId);
	if (_setId > 0) {
		for (i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
			if (_vm->_actors[i]->_id != _id && _vm->_actors[i]->_setId == _setId) {
				_vm->_aiScripts->OtherAgentEnteredThisScene(i, _id);
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

void Actor::setBoundingBox(const Vector3 &position, bool retired) {
	if (retired) {
		_bbox->setXYZ(position.x - (_retiredWidth / 2.0f),
		              position.y,
		              position.z - (_retiredWidth / 2.0f),

		              position.x + (_retiredWidth / 2.0f),
		              position.y + _retiredHeight,
		              position.z + (_retiredWidth / 2.0f));
	} else {
		_bbox->setXYZ(position.x - 12.0f,
		              position.y + 6.0f,
		              position.z - 12.0f,

		              position.x + 12.0f,
		              position.y + 72.0f,
		              position.z + 12.0f);
	}
}

float Actor::distanceFromView(View *view) const{
	float xDist = this->_position.x - view->_cameraPosition.x;
	float zDist = this->_position.z - view->_cameraPosition.z;
	return sqrt(xDist * xDist + zDist * zDist);
}

bool Actor::isWalking() const {
	return _walkInfo->isWalking();
}

bool Actor::isRunning() const {
	return _walkInfo->isRunning();
}

void Actor::stopWalking(bool value) {
	if (value && _id == 0) {
		_vm->_playerActorIdle = true;
	}

	if (isWalking()) {
		_walkInfo->stop(_id, true, _animationModeCombatIdle, 0);
	} else if (inCombat()) {
		changeAnimationMode(_animationModeCombatIdle, false);
	} else {
		changeAnimationMode(kAnimationModeIdle, false);
	}
}

void Actor::faceActor(int otherActorId, bool animate) {
	if (_setId != _vm->_scene->getSetId()) {
		return;
	}

	Actor *otherActor = _vm->_actors[otherActorId];

	if (_setId != otherActor->_setId) {
		return;
	}

	faceXYZ(otherActor->_position.x, otherActor->_position.y, otherActor->_position.z, animate);
}

void Actor::faceObject(const char *objectName, bool animate) {
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1) {
		return;
	}

	BoundingBox boundingBox;
	_vm->_scene->objectGetBoundingBox(objectId, &boundingBox);

	float x0, y0, z0, x1, y1, z1;
	boundingBox.getXYZ(&x0, &y0, &z0, &x1, &y1, &z1);

	float x = (x1 + x0) / 2.0f;
	float z = (z1 + z0) / 2.0f;
	faceXYZ(x, y0, z, animate);
}

void Actor::faceItem(int itemId, bool animate) {
	float x, y, z;
	_vm->_items->getXYZ(itemId, &x, &y, &z);
	faceXYZ(x, y, z, animate);
}

void Actor::faceWaypoint(int waypointId, bool animate) {
	float x, y, z;
	_vm->_waypoints->getXYZ(waypointId, &x, &y, &z);
	faceXYZ(x, y, z, animate);
}

void Actor::faceXYZ(float x, float y, float z, bool animate) {
	if (isWalking()) {
		stopWalking(false);
	}
	if (x == _position.x && z == _position.z) {
		return;
	}

	int heading = angle_1024(_position.x, _position.z, x, z);
	faceHeading(heading, animate);
}

void Actor::faceCurrentCamera(bool animate) {
	faceXYZ(_vm->_view->_cameraPosition.x, _vm->_view->_cameraPosition.y, -_vm->_view->_cameraPosition.z, animate);
}

void Actor::faceHeading(int heading, bool animate) {
	if (heading != _facing) {
		if (animate) {
			_targetFacing = heading;
		} else {
			setFacing(heading, true);
		}
	}
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

void Actor::setInvisible(bool isInvisible) {
	_isInvisible = isInvisible;
}

void Actor::setImmunityToObstacles(bool isImmune) {
	_isImmuneToObstacles = isImmune;
}

void Actor::modifyCurrentHP(signed int change) {
	_currentHP = MIN(MAX(_currentHP + change, 0), 100);
	if (_currentHP > 0)
		retire(false, 0, 0, -1);
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

void Actor::retire(bool retired, int width, int height, int retiredByActorId) {
	_isRetired = retired;
	_retiredWidth = MAX(width, 0);
	_retiredHeight = MAX(height, 0);
	if (_id == 0 && _isRetired) {
		_vm->playerLosesControl();
		_vm->_playerDead = true;
	}
	if (_isRetired) {
		_vm->_aiScripts->Retired(_id, retiredByActorId);
	}
}

void Actor::setTargetable(bool targetable) {
	_isTargetable = targetable;
}

void Actor::setHealth(int hp, int maxHp) {
	_currentHP = hp;
	_maxHP = maxHp;
	if (hp > 0) {
		retire(false, 0, 0, -1);
	}
}

void Actor::combatModeOn(int a2, int a3, int otherActorId, int a5, int animationModeCombatIdle, int animationModeCombatWalk, int animationModeCombatRun, int a9, int a10, int a11, int ammoDamage, int a13, int a14) {
	_animationModeCombatIdle = animationModeCombatIdle;
	_animationModeCombatWalk = animationModeCombatWalk;
	_animationModeCombatRun = animationModeCombatRun;
	_inCombat = true;
	if (_id > 0)
		_combatInfo->combatOn(_id, a2, a3, otherActorId, a5, a9, a10, a11, ammoDamage, a13, a14);
	stopWalking(false);
	changeAnimationMode(_animationModeCombatIdle, false);
	int i;
	for (i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
		Actor *otherActor = _vm->_actors[i];
		if (i != _id && otherActor->_setId == _setId && !otherActor->_isRetired) {
			//TODO: _vm->actorScript->OtherAgentEnteredCombatMode(i, _id, 1);
		}
	}
}

void Actor::combatModeOff() {
	if (_id > 0)
		_combatInfo->combatOff();
	_inCombat = false;
	stopWalking(false);
	changeAnimationMode(kAnimationModeIdle, false);
	int i;
	for (i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
		Actor *otherActor = _vm->_actors[i];
		if (i != _id && otherActor->_setId == _setId && !otherActor->_isRetired) {
			//TODO: _vm->actorScript->OtherAgentEnteredCombatMode(i, _id, 0);
		}
	}
}

float Actor::distanceFromActor(int otherActorId) {
	return (_position - _vm->_actors[otherActorId]->_position).length();
}

float Actor::getX() {
	return _position.x;
}

float Actor::getY() {
	return _position.y;
}

float Actor::getZ() {
	return _position.z;
}

void Actor::getXYZ(float *x, float *y, float *z) {
	*x = _position.x;
	*y = _position.y;
	*z = _position.z;
}

int Actor::getFacing() {
	return _facing;
}

int Actor::getAnimationMode() {
	return _animationMode;
}

void Actor::setGoal(int goalNumber) {
	int oldGoalNumber = _goalNumber;
	_goalNumber = goalNumber;
	if (goalNumber == oldGoalNumber) {
		return;
	}

	_vm->_aiScripts->GoalChanged(_id, oldGoalNumber, goalNumber);
	_vm->_sceneScript->ActorChangedGoal(_id, goalNumber, oldGoalNumber, _vm->_scene->getSetId() == _setId);
}

int Actor::getGoal() {
	return _goalNumber;
}

void Actor::speechPlay(int sentenceId, bool voiceOver) {
	char name[13];
	sprintf(name, "%02d-%04d.AUD", _id, sentenceId); //TODO somewhere here should be also language code
	int balance;

	if (voiceOver || _id == VOICEOVER_ACTOR) {
		balance = 0;
	} else {
		// Vector3 pos = _vm->_view->_frameViewMatrix * _position;
		int screenX = 320; //, screenY = 0;
		//TODO: transform to screen space using fov;
		balance = 127 * (2 * screenX - 640) / 640;
		balance = MIN(127, MAX(-127, balance));
	}

	_vm->_audioSpeech->playSpeech(name, balance);
}

void Actor::speechStop() {
	_vm->_audioSpeech->stopSpeech();
}

bool Actor::isSpeeching() {
	return _vm->_audioSpeech->isPlaying();
}

void Actor::addClueToDatabase(int clueId, int weight, bool clueAcquired, bool unknownFlag, int fromActorId) {
	_clues->add(_id, clueId, weight, clueAcquired, unknownFlag, fromActorId);
}

void Actor::acquireClue(int clueId, bool unknownFlag, int fromActorId) {
	_clues->acquire(clueId, unknownFlag, fromActorId);
}

void Actor::loseClue(int clueId) {
	_clues->lose(clueId);
}

bool Actor::hasClue(int clueId) {
	return _clues->isAcquired(clueId);
}

void Actor::copyClues(int actorId) {
	Actor *otherActor = _vm->_actors[actorId];
	for (int i = 0; i < (int)_vm->_gameInfo->getClueCount(); i++) {
		if (hasClue(i) && !_clues->isFlag4(i) && !otherActor->hasClue(i)) {
			int fromActorId = _id;
			if (_id == VOICEOVER_ACTOR) {
				fromActorId = _clues->getFromActorId(i);
			}
			otherActor->acquireClue(i, false, fromActorId);
		}
	}
}

int Actor::soundVolume() const {
	float dist = distanceFromView(_vm->_view);
	return 255.0f * MAX(MIN(dist / 1200.0f, 1.0f), 0.0f);
}

int Actor::soundBalance() const {
	Vector3 screenPosition = _vm->_view->calculateScreenPosition(_position);
	return 127.0f * (MAX(MIN(screenPosition.x / 640.0f, 1.0f), 0.0f) * 2.0f - 1.0f);
}

bool Actor::walkFindU1(const Vector3 &startPosition, const Vector3 &targetPosition, float size, Vector3 *newDestination) {
	newDestination->x = 0.0f;
	newDestination->y = 0.0f;
	newDestination->z = 0.0f;
	int facing = angle_1024(targetPosition, startPosition);
	int facing1 = 0;

	int facing2 = facing;
	int facing3 = 0;
	while (true) {
		float rotatedX = targetPosition.x + size * sin_1024(facing);
		float rotatedZ = targetPosition.z - size * cos_1024(facing);

		if (!_walkInfo->isXYZEmpty(rotatedX, targetPosition.y, rotatedZ, _id)) {
			if (_vm->_scene->_set->findWalkbox(rotatedX, rotatedZ) >= 0) {
				newDestination->x = rotatedX;
				newDestination->y = targetPosition.y;
				newDestination->z = rotatedZ;
				return true;
			}
		} else {
			facing += 20;
			if (facing > 1024) {
				facing -= 1024;
			}
			facing3 += 20;
		}

		rotatedX = size * sin_1024(facing2) + targetPosition.x;
		rotatedZ = size * cos_1024(facing2) + targetPosition.z;

		if (!_walkInfo->isXYZEmpty(rotatedX, targetPosition.y, rotatedZ, _id)) {
			if (_vm->_scene->_set->findWalkbox(rotatedX, rotatedZ) >= 0) {
				newDestination->x = rotatedX;
				newDestination->y = targetPosition.y;
				newDestination->z = rotatedZ;
				return true;
			}
		} else {
			facing2 -= 20;
			if (facing2 < 0) {
				facing2 += 1024;
			}
			facing1 += 20;
		}

		if (facing3 > 1024 && facing1 > 1024) {
			return false;
		}
	}
}

bool Actor::walkFindU2(Vector3 *newDestination, float targetWidth, int destinationOffset, float targetSize, const Vector3 &startPosition, const Vector3 &targetPosition) {
	newDestination->x = 0.0f;
	newDestination->y = 0.0f;
	newDestination->z = 0.0f;
	float size = destinationOffset + targetSize * 0.5f + targetWidth * 0.5f;
	float distance = (startPosition - targetPosition).length() - targetWidth * 0.5f - targetSize * 0.5f;
	if (size < distance) {
		return walkFindU1(startPosition, targetPosition, size, newDestination);
	} else {
		*newDestination = targetPosition;
		return true;
	}
}

bool Actor::walkToNearestPoint(const Vector3 &destination, float distance) {
	Vector3 out;
	bool flagIsRunning;
	if (_walkInfo->findNearestEmptyPosition(_id, destination, distance, out)) {
		loopWalk(out, 0, false, false, _position, 0.0f, 24.0f, false, &flagIsRunning, false);
		return true;
	}
	return false;
}

} // End of namespace BladeRunner
