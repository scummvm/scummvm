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

#include "bladerunner/actor_clues.h"
#include "bladerunner/actor_combat.h"
#include "bladerunner/actor_walk.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/boundingbox.h"
#include "bladerunner/game_info.h"
#include "bladerunner/items.h"
#include "bladerunner/mouse.h"
#include "bladerunner/movement_track.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/scene_script.h"
#include "bladerunner/script/ai_script.h"
#include "bladerunner/set.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/time.h"
#if SUBTITLES_SUPPORT
#include "bladerunner/subtitles.h"
#endif // SUBTITLES_SUPPORT
#include "bladerunner/waypoints.h"
#include "bladerunner/zbuffer.h"

namespace BladeRunner {

Actor::Actor(BladeRunnerEngine *vm, int actorId) {
	_vm = vm;
	_id = actorId;

	_walkInfo      = new ActorWalk(vm);
	_movementTrack = new MovementTrack();
	_cluesLimit    = (actorId == 0 || actorId == 99) ? 4 : 2;
	_clues         = new ActorClues(vm, _cluesLimit);
	_combatInfo    = new ActorCombat(vm);

	_friendlinessToOther.resize(_vm->_gameInfo->getActorCount());

	_inWalkLoop                       = false;
	_damageAnimIfMoving               = false;

	setup(actorId);
}

Actor::~Actor() {
	delete _combatInfo;
	delete _clues;
	delete _movementTrack;
	delete _walkInfo;
}

void Actor::setup(int actorId) {
	_id    = actorId;
	_setId = -1;

	_position     = Vector3(0.0, 0.0, 0.0);
	_facing       = 512;
	_targetFacing = -1;
	_walkboxId    = -1;

	_animationId    = 0;
	_animationFrame = 0;
	_fps            = 15;
	_frameMs       = 1000 / _fps;

	_isMoving            = false;
	_isTarget            = false;
	_inCombat            = false;
	_isInvisible         = false;
	_isImmuneToObstacles = false;
	_isRetired           = false;

	_width         = 0;
	_height        = 0;
	_retiredWidth  = 0;
	_retiredHeight = 0;
	_scale         = 1.0f;

	_timer4RemainDefault = 60000;

	_movementTrackWalkingToWaypointId = -1;
	_movementTrackDelayOnNextWaypoint = -1;

	for (int i = 0; i != 7; ++i) {
		_timersLeft[i] = 0;
		_timersLast[i] = _vm->_time->current();
	}

	_honesty              = 50;
	_intelligence         = 50;
	_combatAggressiveness = 50;
	_stability            = 50;

	_currentHP  = 50;
	_maxHP      = 50;
	_goalNumber = -1;

	_movementTrackPaused         = false;
	_movementTrackNextWaypointId = -1;
	_movementTrackNextDelay      = -1;
	_movementTrackNextAngle      = -1;
	_movementTrackNextRunning    = false;

	_animationMode   = -1;
	_screenRectangle = Common::Rect(-1, -1, -1, -1);

	_animationModeCombatIdle = kAnimationModeCombatIdle;
	_animationModeCombatWalk = kAnimationModeCombatWalk;
	_animationModeCombatRun  = kAnimationModeCombatRun;

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
		_vm->_aiScripts->changeAnimationMode(_id, animationMode);
		_animationMode = animationMode;
	}
}

void Actor::setFPS(int fps) {
	_fps = fps;

	if (fps == 0) {
		_frameMs = 0;
	} else if (fps == -1) {
		_frameMs = -1000;
	} else if (fps == -2) {
		_fps = _vm->_sliceAnimations->getFPS(_animationId);
		_frameMs = 1000 / _fps;
	} else {
		_frameMs = 1000 / fps;
	}
}

void Actor::increaseFPS() {
	int fps = MIN(_fps + 3, 30);
	setFPS(fps);
}

void Actor::timerStart(int timerId, int interval) {
	assert(timerId >= 0 && timerId < 7);
	_timersLeft[timerId] = interval;
	_timersLast[timerId] = _vm->_time->current();
}

void Actor::timerReset(int timerId) {
	assert(timerId >= 0 && timerId < 7);
	_timersLeft[timerId] = 0;
}

int Actor::timerLeft(int timerId) {
	assert(timerId >= 0 && timerId < 7);
	return _timersLeft[timerId];
}

void Actor::timersUpdate() {
	for (int i = 0; i <= 6; i++) {
		timerUpdate(i);
	}
}

void Actor::timerUpdate(int timerId) {
	if (_timersLeft[timerId] == 0) {
		return;
	}

	uint32 timeNow = _vm->_time->current();
	int timeDiff = timeNow - _timersLast[timerId];
	_timersLast[timerId] = timeNow;
	_timersLeft[timerId] -= timeDiff;

	if (_timersLeft[timerId] <= 0) {
		switch (timerId) {
		case 0:
		case 1:
		case 2:
			if (!_vm->_aiScripts->isInsideScript() && !_vm->_sceneScript->isInsideScript()) {
				_vm->_aiScripts->timerExpired(_id, timerId);
				_timersLeft[timerId] = 0;
			} else {
				_timersLeft[timerId] = 1;
			}
			break;
		case 3:
			_timersLeft[3] = 0;
			if (_movementTrack->isPaused()) {
				_timersLeft[3] = 1;
			} else {
				movementTrackNext(false);
			}
			break;
		case 4:
			// Exchange clues between actors
			acquireCluesByRelations();
			_timersLeft[4] = _timer4RemainDefault;
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
			_timersLeft[6] = 200;
			break;
		}
	}
}

void Actor::movementTrackNext(bool omitAiScript) {
	bool hasNextMovement;
	int waypointSetId;
	bool running;
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
			timerStart(3, delay);
		}
		//return true;
	} else {
		if (!omitAiScript) {
			_vm->_aiScripts->completedMovementTrack(_id);
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
	if (!_movementTrack->isPaused() && _id != kActorMcCoy) {
		if (_movementTrackWalkingToWaypointId >= 0 && _movementTrackDelayOnNextWaypoint >= 0) {
			if (!_movementTrackDelayOnNextWaypoint) {
				_movementTrackDelayOnNextWaypoint = 1;
			}
			if (_vm->_aiScripts->reachedMovementTrackWaypoint(_id, _movementTrackWalkingToWaypointId)) {
				int delay = _movementTrackDelayOnNextWaypoint;
				if (delay > 1) {
					changeAnimationMode(kAnimationModeIdle, false);
					delay = _movementTrackDelayOnNextWaypoint; // todo: analyze if movement is changed in some aiscript->ChangeAnimationMode?
				}
				timerStart(3, delay);
			}
		}
		_movementTrackWalkingToWaypointId = -1;
		_movementTrackDelayOnNextWaypoint = 0;
	}
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

	_vm->_sceneObjects->remove(_id + kSceneObjectOffsetActors);

	if (_vm->_scene->getSetId() == _setId) {
		_vm->_sceneObjects->addActor(_id + kSceneObjectOffsetActors, _bbox, _screenRectangle, true, moving, _isTarget, retired);
	}
}

void Actor::setAtWaypoint(int waypointId, int angle, int moving, bool retired) {
	Vector3 waypointPosition;
	_vm->_waypoints->getXYZ(waypointId, &waypointPosition.x, &waypointPosition.y, &waypointPosition.z);
	setAtXYZ(waypointPosition, angle, true, moving, retired);
}

bool Actor::loopWalk(const Vector3 &destination, int destinationOffset, bool interruptible, bool runFlag, const Vector3 &start, float targetWidth, float targetSize, bool a8, bool *isRunningFlag, bool async) {
	*isRunningFlag = false;

	if (destinationOffset > 0) {
		float dist = distance(_position, destination);
		if (dist - targetSize <= destinationOffset) {
			return false;
		}
	}

	if (a8 && !async && _id != kActorMcCoy && destinationOffset <= 24) {
		if (distance(_vm->_playerActor->_position, destination) <= 24.0f) {
			_vm->_playerActor->walkToNearestPoint(destination, 48.0f);
		}
	}

	if (_id != kActorMcCoy) {
		interruptible = false;
	}

	Vector3 destinationX(destination);

	if (destinationOffset > 0) {
		walkFindU2(&destinationX, targetWidth, destinationOffset, targetSize, _position, destination);
	}

	bool walking = walkTo(runFlag, destinationX, a8);

	if (async) {
		return false;
	}

	if (!walking && destinationOffset > 0) {
		walking = walkTo(runFlag, destination, a8);
	}

	if (!walking) {
		faceXYZ(destination, false);
		return false;
	}

	if (_id != kActorMcCoy) {
		_vm->_mouse->disable();
	}

	if (interruptible) {
		_vm->_isWalkingInterruptible = true;
		_vm->_interruptWalking = false;
	} else {
		_vm->playerLosesControl();
	}

	if (a8) {
		_inWalkLoop = true;
	}

	bool wasInterrupted = false;
	while (_walkInfo->isWalking() && _vm->_gameIsRunning) {
		if (_walkInfo->isRunning()) {
			*isRunningFlag = true;
		}
		_vm->gameTick();
		if (_id == kActorMcCoy && interruptible && _vm->_interruptWalking) {
			stopWalking(false);
			wasInterrupted = true;
		}
	}
	if (a8) {
		_inWalkLoop = false;
	}
	if (interruptible) {
		_vm->_isWalkingInterruptible = false;
	} else {
		_vm->playerGainsControl();
	}
	if (!wasInterrupted && destinationOffset == 0 && !_vm->_playerActorIdle) {
		setAtXYZ(destination, _facing, true, false, false);
	}
	if (_id != kActorMcCoy) {
		_vm->_mouse->enable();
	}
	return wasInterrupted;
}

bool Actor::walkTo(bool runFlag, const Vector3 &destination, bool a3) {
	bool arrived;

	return _walkInfo->setup(_id, runFlag, _position, destination, a3, &arrived);
}

bool Actor::loopWalkToActor(int otherActorId, int destinationOffset, int interruptible, bool runFlag, bool a5, bool *isRunningFlag) {
	return loopWalk(_vm->_actors[otherActorId]->_position, destinationOffset, interruptible, runFlag, _position, 24.0f, 24.0f, a5, isRunningFlag, false);
}

bool Actor::loopWalkToItem(int itemId, int destinationOffset, int interruptible, bool runFlag, bool a5, bool *isRunningFlag) {
	float x, y, z;
	int width, height;
	_vm->_items->getXYZ(itemId, &x, &y, &z);
	_vm->_items->getWidthHeight(itemId, &width, &height);
	Vector3 itemPosition(x, y, z);

	return loopWalk(itemPosition, destinationOffset, interruptible, runFlag, _position, width, 24.0f, a5, isRunningFlag, false);
}

bool Actor::loopWalkToSceneObject(const Common::String &objectName, int destinationOffset, bool interruptible, bool runFlag, bool a5, bool *isRunningFlag) {
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

	return loopWalk(destination, destinationOffset, interruptible, runFlag, _position, 0.0f, 24.0f, a5, isRunningFlag, false);
}

bool Actor::loopWalkToWaypoint(int waypointId, int destinationOffset, int interruptible, bool runFlag, bool a5, bool *isRunningFlag) {
	Vector3 waypointPosition;
	_vm->_waypoints->getXYZ(waypointId, &waypointPosition.x, &waypointPosition.y, &waypointPosition.z);
	return loopWalk(waypointPosition, destinationOffset, interruptible, runFlag, _position, 0.0f, 24.0f, a5, isRunningFlag, false);
}

bool Actor::loopWalkToXYZ(const Vector3 &destination, int destinationOffset, bool interruptible, bool runFlag, bool a5, bool *isRunningFlag) {
	return loopWalk(destination, destinationOffset, interruptible, runFlag, _position, 0.0f, 24.0f, a5, isRunningFlag, false);
}

bool Actor::asyncWalkToWaypoint(int waypointId, int destinationOffset, bool runFlag, bool a5) {
	bool running;
	Vector3 waypointPosition;
	_vm->_waypoints->getXYZ(waypointId, &waypointPosition.x, &waypointPosition.y, &waypointPosition.z);
	return loopWalk(waypointPosition, destinationOffset, false, runFlag, _position, 0.0f, 24.0f, a5, &running, true);
}

void Actor::asyncWalkToXYZ(const Vector3 &destination, int destinationOffset, bool runFlag, int a6) {
	bool running;
	loopWalk(destination, destinationOffset, false, runFlag, _position, 0.0f, 24.0f, a6, &running, true);
}

void Actor::run() {
	_walkInfo->run(_id);
}

bool Actor::tick(bool forceDraw, Common::Rect *screenRect) {
	int timeLeft = 0;
	bool needsUpdate = false;
	if (_fps > 0) {
		timerUpdate(5);
		timeLeft = timerLeft(5);
		needsUpdate = timeLeft <= 0;
	} else if (forceDraw) {
		needsUpdate = true;
		timeLeft = 0;
	}
#if SUBTITLES_SUPPORT
	if(!isSpeeching()) {
	    _vm->_subtitles->hide();
	}
#endif // SUBTITLES_SUPPORT
	if (needsUpdate) {
		int newAnimation = 0, newFrame = 0;
		_vm->_aiScripts->updateAnimation(_id, &newAnimation, &newFrame);

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

		if (_id == kActorHysteriaPatron1) {
			positionChange.x = 0.0f;
			positionChange.y = 0.0f;
			positionChange.z = 0.0f;
		}

		if (isWalking()) {
			if (0.0f <= positionChange.y) {
				positionChange.y = -4.0f;
			}

			_targetFacing = -1;

			bool walked = _walkInfo->tick(_id, -positionChange.y, false);

			Vector3 pos;
			int facing;
			_walkInfo->getCurrentPosition(_id, &pos, &facing);
			setAtXYZ(pos, facing, false, _isMoving, false);
			if (walked) {
				_vm->_actors[_id]->changeAnimationMode(kAnimationModeIdle);

				movementTrackWaypointReached();
				if (inCombat()) {
					changeAnimationMode(_animationModeCombatIdle, false);
				} else {
					changeAnimationMode(kAnimationModeIdle, false);
				}
			}
		} else {
			if (angleChange != 0.0f) {
				int facingChange = angleChange * (512.0f / M_PI);
				if (facingChange != 0) {
					_facing = _facing - facingChange;
					if (_facing < 0) {
						_facing += 1024;
					}

					if (_facing >= 1024) {
						_facing = _facing - 1024;
					}
				}
			}

			if (0.0f != positionChange.x || 0.0f != positionChange.y || 0.0f != positionChange.z) {
				if (_actorSpeed.x != 0.0f) {
					positionChange.x = positionChange.x * _actorSpeed.x;
				}
				if (_actorSpeed.y != 0.0f) {
					positionChange.y = positionChange.y * _actorSpeed.y;
				}
				if (_actorSpeed.z != 0.0f) {
					positionChange.z = positionChange.z * _actorSpeed.z;
				}

				float sinx = _vm->_sinTable1024->at(_facing);
				float cosx = _vm->_cosTable1024->at(_facing);

				float originalX = _position.x;
				float originalY = _position.y;
				float originalZ = _position.z;

				// Yes, Z & Y are switched between world space and model space. X is also negated for some unknown reason (wrong dirertion for angles?)

				_position.x = _position.x - positionChange.x * cosx - positionChange.y * sinx;
				_position.z = _position.z - positionChange.x * sinx + positionChange.y * cosx;
				_position.y = _position.y + positionChange.z;

				if (_vm->_sceneObjects->existsOnXZ(_id + kSceneObjectOffsetActors, _position.x, _position.z, false, false) == 1 && !_isImmuneToObstacles) {
					_position.x = originalX;
					_position.y = originalY;
					_position.z = originalZ;
				}
				setAtXYZ(_position, _facing, true, _isMoving, _isRetired);
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
		int nextFrameTime = timeLeft + _frameMs;
		if (nextFrameTime <= 0) {
			nextFrameTime = 1;
		}
		timerStart(5, nextFrameTime);
	}
	if (_targetFacing >= 0) {
		if (_targetFacing == _facing) {
			_targetFacing = -1;
		} else {
			setFacing(_targetFacing, false);
		}
	}
	return isVisible;
}

void Actor::tickCombat() {
	if (_id != kActorMcCoy && !_isRetired && _inCombat) {
		_combatInfo->tick();
	}
}

bool Actor::draw(Common::Rect *screenRect) {
	Vector3 drawPosition(_position.x, -_position.z, _position.y + 2.0);
	float drawAngle = M_PI - _facing * (M_PI / 512.0f);
	float drawScale = _scale;

	// TODO: Handle SHORTY mode

	_vm->_sliceRenderer->drawInWorld(_animationId, _animationFrame, drawPosition, drawAngle, drawScale, _vm->_surfaceFront, _vm->_zbuffer->getData());
	_vm->_sliceRenderer->getScreenRectangle(screenRect, _animationId, _animationFrame, drawPosition, drawAngle, drawScale);

	return !screenRect->isEmpty();
}

int Actor::getSetId() const {
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
				_vm->_aiScripts->otherAgentExitedThisScene(i, _id);
			}
		}
	}
	_setId = setId;
	_vm->_aiScripts->enteredScene(_id, _setId);
	if (_setId > 0) {
		for (i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
			if (_vm->_actors[i]->_id != _id && _vm->_actors[i]->_setId == _setId) {
				_vm->_aiScripts->otherAgentEnteredThisScene(i, _id);
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
		_bbox.setXYZ(position.x - (_retiredWidth / 2.0f),
		             position.y,
		             position.z - (_retiredWidth / 2.0f),

		             position.x + (_retiredWidth / 2.0f),
		             position.y + _retiredHeight,
		             position.z + (_retiredWidth / 2.0f));
	} else {
		_bbox.setXYZ(position.x - 12.0f,
		             position.y + 6.0f,
		             position.z - 12.0f,

		             position.x + 12.0f,
		             position.y + 72.0f,
		             position.z + 12.0f);
	}
}

float Actor::distanceFromView(View *view) const{
	float xDist = _position.x - view->_cameraPosition.x;
	float zDist = _position.z + view->_cameraPosition.y; // y<->z is intentional, not a bug
	return sqrt(xDist * xDist + zDist * zDist);
}

bool Actor::isWalking() const {
	return _walkInfo->isWalking();
}

bool Actor::isRunning() const {
	return _walkInfo->isRunning();
}

void Actor::stopWalking(bool value) {
	if (value && _id == kActorMcCoy) {
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

	faceXYZ(otherActor->_position, animate);
}

void Actor::faceObject(const Common::String &objectName, bool animate) {
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

void Actor::faceXYZ(const Vector3 &pos, bool animate) {
	faceXYZ(pos.x, pos.y, pos.z, animate);
}

void Actor::faceCurrentCamera(bool animate) {
	faceXYZ(_vm->_view->_cameraPosition.x, _vm->_view->_cameraPosition.z, -_vm->_view->_cameraPosition.y, animate); // y<->z is intentional, not a bug
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
	_friendlinessToOther[otherActorId] = CLIP(_friendlinessToOther[otherActorId] + change, 0, 100);
}

void Actor::setFriendlinessToOther(int otherActorId, int friendliness) {
	_friendlinessToOther[otherActorId] = friendliness;
}

bool Actor::checkFriendlinessAndHonesty(int otherActorId) {
	int honestyDiff = 2 * _friendlinessToOther[otherActorId] - _honesty;
	uint friendlinessRange;

	if (honestyDiff > 30) {
		friendlinessRange = 100;
	} else if (honestyDiff >= 0 && honestyDiff <= 30) {
		friendlinessRange = 50;
	} else {
		friendlinessRange = 0;
	}

	return _vm->_rnd.getRandomNumberRng(1, 100) <= friendlinessRange;
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

void Actor::modifyCombatAggressiveness(signed int change) {
	_combatAggressiveness = CLIP(_combatAggressiveness + change, 0, 100);
}

void Actor::modifyHonesty(signed int change) {
	_honesty = CLIP(_honesty + change, 0, 100);
}

void Actor::modifyIntelligence(signed int change) {
	_intelligence = CLIP(_intelligence + change, 0, 100);
}

void Actor::modifyStability(signed int change) {
	_stability = CLIP(_stability + change, 0, 100);
}

void Actor::setFlagDamageAnimIfMoving(bool value) {
	_damageAnimIfMoving = value;
}

bool Actor::getFlagDamageAnimIfMoving() const {
	return _damageAnimIfMoving;
}

void Actor::retire(bool retired, int width, int height, int retiredByActorId) {
	_isRetired = retired;
	_retiredWidth = MAX(width, 0);
	_retiredHeight = MAX(height, 0);
	if (_id == kActorMcCoy && _isRetired) {
		_vm->playerLosesControl();
		_vm->_playerDead = true;
	}
	if (_isRetired) {
		_vm->_aiScripts->retired(_id, retiredByActorId);
	}
}

void Actor::setTarget(bool target) {
	_isTarget = target;
}

void Actor::setCurrentHP(int hp) {
	_currentHP = hp;
	if (hp > 0) {
		retire(false, 0, 0, -1);
	}
}

void Actor::setHealth(int hp, int maxHp) {
	_currentHP = hp;
	_maxHP = maxHp;
	if (hp > 0) {
		retire(false, 0, 0, -1);
	}
}

void Actor::modifyCurrentHP(signed int change) {
	_currentHP = CLIP(_currentHP + change, 0, 100);
	if (_currentHP > 0) {
		retire(false, 0, 0, -1);
	}
}

void Actor::modifyMaxHP(signed int change) {
	_maxHP = CLIP(_maxHP + change, 0, 100);
}


void Actor::combatModeOn(int initialState, bool rangedAttack, int enemyId, int waypointType, int animationModeCombatIdle, int animationModeCombatWalk, int animationModeCombatRun, int fleeRatio, int coverRatio, int actionRatio, int damage, int range, bool unstoppable) {
	_animationModeCombatIdle = animationModeCombatIdle;
	_animationModeCombatWalk = animationModeCombatWalk;
	_animationModeCombatRun = animationModeCombatRun;
	_inCombat = true;
	if (_id != kActorMcCoy) {
		_combatInfo->combatOn(_id, initialState, rangedAttack, enemyId, waypointType, fleeRatio, coverRatio, actionRatio, damage, range, unstoppable);
	}
	stopWalking(false);
	changeAnimationMode(_animationModeCombatIdle, false);
	for (int i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
		Actor *otherActor = _vm->_actors[i];
		if (i != _id && otherActor->_setId == _setId && !otherActor->_isRetired) {
			_vm->_aiScripts->otherAgentEnteredCombatMode(i, _id, true);
		}
	}
}

void Actor::combatModeOff() {
	if (_id != kActorMcCoy) {
		_combatInfo->combatOff();
	}
	_inCombat = false;
	stopWalking(false);
	changeAnimationMode(kAnimationModeIdle, false);
	for (int i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
		Actor *otherActor = _vm->_actors[i];
		if (i != _id && otherActor->_setId == _setId && !otherActor->_isRetired) {
			_vm->_aiScripts->otherAgentEnteredCombatMode(i, _id, false);
		}
	}
}

float Actor::distanceFromActor(int otherActorId) {
	return (_position - _vm->_actors[otherActorId]->_position).length();
}

int Actor::angleTo(const Vector3 &target) const {
	int angle = angle_1024(_position.x, _position.z, target.x, target.z) - _facing;
	if (angle < -512) {
		angle += 1024;
	} else if (angle > 512) {
		angle -= 1024;
	}
	return angle;
}

float Actor::getX() const {
	return _position.x;
}

float Actor::getY() const {
	return _position.y;
}

float Actor::getZ() const {
	return _position.z;
}

Vector3 Actor::getXYZ() const {
	return _position;
}

int Actor::getFacing() const {
	return _facing;
}

int Actor::getAnimationMode() const {
	return _animationMode;
}

void Actor::setGoal(int goalNumber) {
	int oldGoalNumber = _goalNumber;
	_goalNumber = goalNumber;
	if (goalNumber == oldGoalNumber) {
		return;
	}

	_vm->_aiScripts->goalChanged(_id, oldGoalNumber, goalNumber);
	_vm->_sceneScript->actorChangedGoal(_id, goalNumber, oldGoalNumber, _vm->_scene->getSetId() == _setId);
}

int Actor::getGoal() const {
	return _goalNumber;
}

void Actor::speechPlay(int sentenceId, bool voiceOver) {
	Common::String name = Common::String::format( "%02d-%04d%s.AUD", _id, sentenceId, _vm->_languageCode.c_str());

	int balance = 0;
	if (!voiceOver && _id != BladeRunnerEngine::kActorVoiceOver) {
		// Vector3 pos = _vm->_view->_frameViewMatrix * _position;
		int screenX = 320; //, screenY = 0;
		//TODO: transform to screen space using fov;
		balance = 127 * (2 * screenX - 640) / 640;
		balance = CLIP<int>(balance, -127, 127);
	}

#if SUBTITLES_SUPPORT
    _vm->_subtitles->getInGameSubsText(_id, sentenceId);
    _vm->_subtitles->show();
#endif // SUBTITLES_SUPPORT
	
	_vm->_audioSpeech->playSpeech(name, balance);
}

void Actor::speechStop() {
#if SUBTITLES_SUPPORT
    _vm->_subtitles->hide();
#endif // SUBTITLES_SUPPORT	
	_vm->_audioSpeech->stopSpeech();
}

bool Actor::isSpeeching() {
	return _vm->_audioSpeech->isPlaying();
}

void Actor::addClueToDatabase(int clueId, int weight, bool clueAcquired, bool unknownFlag, int fromActorId) {
	_clues->add(_id, clueId, weight, clueAcquired, unknownFlag, fromActorId);
}

bool Actor::canAcquireClue(int clueId) const {
	return _clues->exists(clueId);
}

void Actor::acquireClue(int clueId, bool unknownFlag, int fromActorId) {
	bool hasAlready = hasClue(clueId);
	_clues->acquire(clueId, unknownFlag, fromActorId);
	if (!hasAlready) {
		_vm->_aiScripts->receivedClue(_id, clueId, fromActorId);
	}
}

void Actor::loseClue(int clueId) {
	_clues->lose(clueId);
}

bool Actor::hasClue(int clueId) const {
	return _clues->isAcquired(clueId);
}

void Actor::copyClues(int actorId) {
	Actor *otherActor = _vm->_actors[actorId];
	for (int i = 0; i < (int)_vm->_gameInfo->getClueCount(); i++) {
		if (hasClue(i) && !_clues->isPrivate(i) && otherActor->canAcquireClue(i) && !otherActor->hasClue(i)) {
			int fromActorId = _id;
			if (_id == BladeRunnerEngine::kActorVoiceOver) {
				fromActorId = _clues->getFromActorId(i);
			}
			otherActor->acquireClue(i, false, fromActorId);
		}
	}
}

void Actor::acquireCluesByRelations() {
	if (_setId >= 0 && _setId != kSetFreeSlotG && _setId != _vm->_actors[0]->_setId) {
		for (int i = 0; i < (int)_vm->_gameInfo->getActorCount(); i++) {
			if (i != _id && _vm->_actors[i]->_setId == _setId && i && _id
					&& checkFriendlinessAndHonesty(i)
					&& _vm->_actors[i]->checkFriendlinessAndHonesty(_id)) {
				_clues->acquireCluesByRelations(_id, i);
			}
		}
	}
}

int Actor::soundVolume() const {
	float dist = distanceFromView(_vm->_view);
	return 35.0f * CLIP(1.0f - (dist / 1200.0f), 0.0f, 1.0f);
}

int Actor::soundBalance() const {
	Vector3 screenPosition = _vm->_view->calculateScreenPosition(_position);
	return 35.0f * (CLIP(screenPosition.x / 640.0f, 0.0f, 1.0f) * 2.0f - 1.0f);
}

bool Actor::isObstacleBetween(const Vector3 &target) {
	return _vm->_sceneObjects->isObstacleBetween(_position, target, -1);
}

int Actor::findTargetUnderMouse(BladeRunnerEngine *vm, int mouseX, int mouseY) {
	int setId = vm->_scene->getSetId();
	for (int i = 0; i < (int)vm->_gameInfo->getActorCount(); ++i) {
		if (vm->_actors[i]->isTarget() && vm->_actors[i]->getSetId() == setId) {
			if (vm->_actors[i]->_screenRectangle.contains(mouseX, mouseY)) {
				return i;
			}
		}
	}
	return -1;
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
		float rotatedX = targetPosition.x + size * _vm->_sinTable1024->at(facing);
		float rotatedZ = targetPosition.z - size * _vm->_cosTable1024->at(facing);

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

		rotatedX = size * _vm->_sinTable1024->at(facing2) + targetPosition.x;
		rotatedZ = size * _vm->_cosTable1024->at(facing2) + targetPosition.z;

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
	bool running;
	if (_walkInfo->findNearestEmptyPosition(_id, destination, distance, out)) {
		loopWalk(out, 0, false, false, _position, 0.0f, 24.0f, false, &running, false);
		return true;
	}
	return false;
}

void Actor::save(SaveFileWriteStream &f) {
	f.writeInt(_id);
	f.writeInt(_setId);
	f.writeVector3(_position);
	f.writeInt(_facing);
	f.writeInt(_targetFacing);
	f.writeInt(_timer4RemainDefault);

	f.writeInt(_honesty);
	f.writeInt(_intelligence);
	f.writeInt(_combatAggressiveness);
	f.writeInt(_stability);

	f.writeInt(_goalNumber);

	f.writeInt(_currentHP);
	f.writeInt(_maxHP);

	f.writeBool(_movementTrackPaused);
	f.writeInt(_movementTrackNextWaypointId);
	f.writeInt(_movementTrackNextDelay);
	f.writeInt(_movementTrackNextAngle);
	f.writeBool(_movementTrackNextRunning);

	f.writeInt(_cluesLimit);

	f.writeBool(_isMoving);
	f.writeBool(_isTarget);
	f.writeBool(_inCombat);
	f.writeBool(_isInvisible);
	f.writeBool(_isRetired);
	f.writeBool(_isImmuneToObstacles);

	f.writeInt(_animationMode);
	f.writeInt(_fps);
	f.writeInt(_frameMs);
	f.writeInt(_animationId);
	f.writeInt(_animationFrame);

	f.writeInt(_movementTrackWalkingToWaypointId);
	f.writeInt(_movementTrackDelayOnNextWaypoint);

	f.writeRect(_screenRectangle);
	f.writeInt(_retiredWidth);
	f.writeInt(_retiredHeight);
	f.writeInt(_damageAnimIfMoving);
	f.writeInt(0); // TODO: _actorFieldU6
	f.writeInt(0); // TODO: _actorFieldU7
	f.writeFloat(_scale);

	for (int i = 0; i < 7; ++i) {
		f.writeInt(_timersLeft[i]);
	}

	uint32 now = _vm->_time->getPauseStart();
	for (int i = 0; i < 7; ++i) {
		f.writeInt(now - _timersLast[i]);
	}

	int actorCount = _vm->_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i) {
		f.writeInt(_friendlinessToOther[i]);
	}

	_clues->save(f);

	_movementTrack->save(f);

	_walkInfo->save(f);

	f.writeBoundingBox(_bbox, false);

	_combatInfo->save(f);

	f.writeInt(_animationModeCombatIdle);
	f.writeInt(_animationModeCombatWalk);
	f.writeInt(_animationModeCombatRun);
}

void Actor::load(SaveFileReadStream &f) {
	_id = f.readInt();
	_setId = f.readInt();
	_position = f.readVector3();
	_facing = f.readInt();
	_targetFacing = f.readInt();
	_timer4RemainDefault = f.readInt();

	_honesty = f.readInt();
	_intelligence = f.readInt();
	_combatAggressiveness = f.readInt();
	_stability = f.readInt();

	_goalNumber = f.readInt();

	_currentHP = f.readInt();
	_maxHP = f.readInt();

	_movementTrackPaused = f.readBool();
	_movementTrackNextWaypointId = f.readInt();
	_movementTrackNextDelay = f.readInt();
	_movementTrackNextAngle = f.readInt();
	_movementTrackNextRunning = f.readBool();

	_cluesLimit = f.readInt();

	_isMoving = f.readBool();
	_isTarget = f.readBool();
	_inCombat = f.readBool();
	_isInvisible = f.readBool();
	_isRetired = f.readBool();
	_isImmuneToObstacles = f.readBool();

	_animationMode = f.readInt();
	_fps = f.readInt();
	_frameMs = f.readInt();
	_animationId = f.readInt();
	_animationFrame = f.readInt();

	_movementTrackWalkingToWaypointId = f.readInt();
	_movementTrackDelayOnNextWaypoint = f.readInt();

	_screenRectangle = f.readRect();
	_retiredWidth = f.readInt();
	_retiredHeight = f.readInt();
	_damageAnimIfMoving = f.readInt();
	f.skip(4); // TODO: _actorFieldU6
	f.skip(4); // TODO: _actorFieldU7
	_scale = f.readFloat();

	for (int i = 0; i < 7; ++i) {
		_timersLeft[i] = f.readInt();
	}

	uint32 now = _vm->_time->getPauseStart();
	for (int i = 0; i < 7; ++i) {
		_timersLast[i] = now - f.readInt();
	}

	int actorCount = _vm->_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i) {
		_friendlinessToOther[i] = f.readInt();
	}

	_clues->load(f);

	_movementTrack->load(f);

	_walkInfo->load(f);

	_bbox = f.readBoundingBox(false);

	_combatInfo->load(f);

	_animationModeCombatIdle = f.readInt();
	_animationModeCombatWalk = f.readInt();
	_animationModeCombatRun = f.readInt();
}

} // End of namespace BladeRunner
