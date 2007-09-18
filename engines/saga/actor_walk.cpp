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

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/console.h"
#include "saga/events.h"
#include "saga/isomap.h"
#include "saga/objectmap.h"
#include "saga/sagaresnames.h"
#include "saga/script.h"
#include "saga/sound.h"
#include "saga/scene.h"

namespace Saga {

static const PathDirectionData pathDirectionLUT[8][3] = {
	{ { 0, Point( 0, -1) }, { 7, Point(-1, -1) }, { 4, Point( 1, -1) } },
	{ { 1, Point( 1,  0) }, { 4, Point( 1, -1) }, { 5, Point( 1,  1) } },
	{ { 2, Point( 0,  1) }, { 5, Point( 1,  1) }, { 6, Point(-1,  1) } },
	{ { 3, Point(-1,  0) }, { 6, Point(-1,  1) }, { 7, Point(-1, -1) } },
	{ { 0, Point( 0, -1) }, { 1, Point( 1,  0) }, { 4, Point( 1, -1) } },
	{ { 1, Point( 1,  0) }, { 2, Point( 0,  1) }, { 5, Point( 1,  1) } },
	{ { 2, Point( 0,  1) }, { 3, Point(-1,  0) }, { 6, Point(-1,  1) } },
	{ { 3, Point(-1,  0) }, { 0, Point( 0, -1) }, { 7, Point(-1, -1) } }
};

static const int pathDirectionLUT2[8][2] = {
	{  0, -1 },
	{  1,  0 },
	{  0,  1 },
	{ -1,  0 },
	{  1, -1 },
	{  1,  1 },
	{ -1,  1 },
	{ -1, -1 }
};

static const int angleLUT[16][2] = {
	{    0, -256 },
	{   98, -237 },
	{  181, -181 },
	{  237,  -98 },
	{  256,    0 },
	{  237,	  98 },
	{  181,  181 },
	{   98,  237 },
	{    0,  256 },
	{  -98,  237 },
	{ -181,  181 },
	{ -237,   98 },
	{ -256,    0 },
	{ -237,  -98 },
	{ -181, -181 },
	{  -98, -237 }
};

static const int directionLUT[8][2] = {
	{  0 * 2, -2 * 2 },
	{  2 * 2, -1 * 2 },
	{  3 * 2,  0 * 2 },
	{  2 * 2,  1 * 2 },
	{  0 * 2,  2 * 2 },
	{ -2 * 2,  1 * 2 },
	{ -4 * 2,  0 * 2 },
	{ -2 * 2, -1 * 2 }
};

static const int tileDirectionLUT[8][2] = {
	{  1,  1 },
	{  2,  0 },
	{  1, -1 },
	{  0, -2 },
	{ -1, -1 },
	{ -2,  0 },
	{ -1,  1 },
	{  0,  2 }
};

struct DragonMove {
	uint16 baseFrame;
	int16 offset[4][2];
};

static const DragonMove dragonMoveTable[12] = {
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{ 28, { { -0,  0 }, { -1,  6 }, {  -5,  11 }, { -10,  15 } } },
	{ 56, { {  0,  0 }, {  1,  6 }, {   5,  11 }, {  10,  15 } } },
	{ 40, { {  0,  0 }, {  6,  1 }, {  11,   5 }, {  15,  10 } } },
	{ 44, { {  0,  0 }, {  6, -1 }, {  11,  -5 }, {  15, -10 } } },
	{ 32, { { -0, -0 }, { -6, -1 }, { -11,  -5 }, { -15, -10 } } },
	{ 52, { { -0,  0 }, { -6,  1 }, { -11,   5 }, { -15,  10 } } },
	{ 36, { {  0, -0 }, {  1, -6 }, {   5, -11 }, {  10, -15 } } },
	{ 48, { { -0, -0 }, { -1, -6 }, {  -5, -11 }, { -10, -15 } } }
};

inline int16 quickDistance(const Point &point1, const Point &point2, int16 compressX) {
	Point delta;
	delta.x = ABS(point1.x - point2.x) / compressX;
	delta.y = ABS(point1.y - point2.y);
	return ((delta.x < delta.y) ? (delta.y + delta.x / 2) : (delta.x + delta.y / 2));
}

inline void calcDeltaS(const Point &point1, const Point &point2, Point &delta, Point &s) {

	delta.x = point2.x - point1.x;
	if (delta.x == 0) {
		s.x = 0;
	} else {
		if (delta.x > 0) {
			s.x = 1;
		} else {
			s.x = -1;
			delta.x = -delta.x;
		}
	}


	delta.y = point2.y - point1.y;
	if (delta.y == 0) {
		s.y = 0;
	} else {
		if (delta.y > 0) {
			s.y = 1;
		} else {
			s.y = -1;
			delta.y = -delta.y;
		}
	}
}

inline int16 int16Compare(int16 i1, int16 i2) {
	return ((i1) > (i2) ? 1 : ((i1) < (i2) ? -1 : 0));
}

bool Actor::validFollowerLocation(const Location &location) {
	Point point;
	location.toScreenPointXY(point);

	if ((point.x < 5) || (point.x >= _vm->getDisplayWidth() - 5) ||
		(point.y < 0) || (point.y > _vm->_scene->getHeight())) {
		return false;
	}

	return (_vm->_scene->canWalk(point));
}

void Actor::realLocation(Location &location, uint16 objectId, uint16 walkFlags) {
	int angle;
	int distance;
	ActorData *actor;
	ObjectData *obj;
	debug (8, "Actor::realLocation objectId=%i", objectId);
	if (walkFlags & kWalkUseAngle) {
		if (_vm->_scene->getFlags() & kSceneFlagISO) {
			angle = (location.x + 2) & 15;
			distance = location.y;

			location.u() = (angleLUT[angle][0] * distance) >> 8;
			location.v() = -(angleLUT[angle][1] * distance) >> 8;
		} else {
			angle = location.x & 15;
			distance = location.y;

			location.x = (angleLUT[angle][0] * distance) >> 6;
			location.y = (angleLUT[angle][1] * distance) >> 6;
		}
	}

	if (objectId != ID_NOTHING) {
		if (validActorId(objectId)) {
			actor = getActor(objectId);
			location.addXY(actor->_location);
		} else if (validObjId(objectId)) {
			obj = getObj(objectId);
			location.addXY(obj->_location);
		}
	}
}

void Actor::actorFaceTowardsObject(uint16 actorId, uint16 objectId) {
	ActorData *actor;
	ObjectData *obj;

	if (validActorId(objectId)) {
		actor = getActor(objectId);
		actorFaceTowardsPoint(actorId, actor->_location);
	} else if (validObjId(objectId)) {
		obj = getObj(objectId);
		actorFaceTowardsPoint(actorId, obj->_location);
	}
}

void Actor::actorFaceTowardsPoint(uint16 actorId, const Location &toLocation) {
	ActorData *actor;
	Location delta;
	//debug (8, "Actor::actorFaceTowardsPoint actorId=%i", actorId);
	actor = getActor(actorId);

	toLocation.delta(actor->_location, delta);

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		if (delta.u() > 0) {
			actor->_facingDirection = (delta.v() > 0) ? kDirUp : kDirRight;
		} else {
			actor->_facingDirection = (delta.v() > 0) ? kDirLeft : kDirDown;
		}
	} else {
		if (ABS(delta.y) > ABS(delta.x * 2)) {
			actor->_facingDirection = (delta.y > 0) ? kDirDown : kDirUp;
		} else {
			actor->_facingDirection = (delta.x > 0) ? kDirRight : kDirLeft;
		}
	}
}

void Actor::updateActorsScene(int actorsEntrance) {
	int i, j;
	int followerDirection;
	ActorData *actor;
	Location tempLocation;
	Location possibleLocation;
	Point delta;
	const SceneEntry *sceneEntry;

	if (_vm->_scene->currentSceneNumber() == 0) {
		error("Actor::updateActorsScene _vm->_scene->currentSceneNumber() == 0");
	}

	_vm->_sound->stopVoice();
	_activeSpeech.stringsCount = 0;
	_activeSpeech.playing = false;
	_protagonist = NULL;

	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i];
		actor->_inScene = false;
		actor->_spriteList.freeMem();
		if (actor->_disabled) {
			continue;
		}
		if ((actor->_flags & (kProtagonist | kFollower)) || (i == 0)) {
			if (actor->_flags & kProtagonist) {
				actor->_finalTarget = actor->_location;
				_centerActor = _protagonist = actor;
			} else if (_vm->getGameType() == GType_ITE && 
					   _vm->_scene->currentSceneResourceId() == RID_ITE_OVERMAP_SCENE) {
				continue;
			}

			actor->_sceneNumber = _vm->_scene->currentSceneNumber();
		}
		if (actor->_sceneNumber == _vm->_scene->currentSceneNumber()) {
			actor->_inScene = true;
			actor->_actionCycle = (_vm->_rnd.getRandomNumber(7) & 0x7) * 4; // 1/8th chance
		}
	}

	// _protagonist can be null while loading a game from the command line
	if (_protagonist == NULL)
		return;

	if ((actorsEntrance >= 0) && (_vm->_scene->_entryList.entryListCount > 0)) {
		if (_vm->_scene->_entryList.entryListCount <= actorsEntrance) {
			actorsEntrance = 0; //OCEAN bug
		}

		sceneEntry = _vm->_scene->_entryList.getEntry(actorsEntrance);
		if (_vm->_scene->getFlags() & kSceneFlagISO) {
			_protagonist->_location = sceneEntry->location;
		} else {
			_protagonist->_location.x = sceneEntry->location.x * ACTOR_LMULT;
			_protagonist->_location.y = sceneEntry->location.y * ACTOR_LMULT;
			_protagonist->_location.z = sceneEntry->location.z * ACTOR_LMULT;
		}
		// Workaround for bug #1328045:
		// "When entering any of the houses at the start of the
		// game if you click on anything inside the building you
		// start walking through the door, turn around and leave."
		//
		// After stepping on an action zone, Rif is trying to exit.
		// Shift Rif's entry position to a non action zone area.
		if (_vm->getGameType() == GType_ITE) {
			if ((_vm->_scene->currentSceneNumber() >= 53) && (_vm->_scene->currentSceneNumber() <= 66))
				_protagonist->_location.y += 10;
		}

		_protagonist->_facingDirection = _protagonist->_actionDirection = sceneEntry->facing;
	}

	_protagonist->_currentAction = kActionWait;

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		//nothing?
	} else {
		_vm->_scene->initDoorsState(); //TODO: move to _scene
	}

	followerDirection = _protagonist->_facingDirection + 3;
	calcScreenPosition(_protagonist);

	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i];
		if (actor->_flags & (kFollower)) {
			actor->_facingDirection = actor->_actionDirection = _protagonist->_facingDirection;
			actor->_currentAction = kActionWait;
			actor->_walkStepsCount = actor->_walkStepIndex = 0;
			actor->_location.z = _protagonist->_location.z;


			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				_vm->_isoMap->placeOnTileMap(_protagonist->_location, actor->_location, 3, followerDirection & 0x07);
			} else {
				followerDirection &= 0x07;

				possibleLocation = _protagonist->_location;

				delta.x = directionLUT[followerDirection][0];
				delta.y = directionLUT[followerDirection][1];

				for (j = 0; j < 30; j++) {
					tempLocation = possibleLocation;
					tempLocation.x += delta.x;
					tempLocation.y += delta.y;

					if (validFollowerLocation(tempLocation)) {
						possibleLocation = tempLocation;
					} else {
						tempLocation = possibleLocation;
						tempLocation.x += delta.x;
						if (validFollowerLocation(tempLocation)) {
							possibleLocation = tempLocation;
						} else {
							tempLocation = possibleLocation;
							tempLocation.y += delta.y;
							if (validFollowerLocation(tempLocation)) {
								possibleLocation = tempLocation;
							} else {
								break;
							}
						}
					}
				}

				actor->_location = possibleLocation;
			}
			followerDirection += 2;
		}

	}

	handleActions(0, true);
	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		_vm->_isoMap->adjustScroll(true);
	}
}

void Actor::handleActions(int msec, bool setup) {
	int i;
	ActorData *actor;
	ActorFrameRange *frameRange;
	int state;
	int speed;
	int32 framesLeft;
	Location delta;
	Location addDelta;
	int hitZoneIndex;
	const HitZone *hitZone;
	Point hitPoint;
	Location pickLocation;

	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i];
		if (!actor->_inScene)
			continue;

		if ((_vm->getGameType() == GType_ITE) && (i == ACTOR_DRAGON_INDEX)) {
			moveDragon(actor);
			continue;
		}

		switch (actor->_currentAction) {
		case kActionWait:
			if (!setup && (actor->_flags & kFollower)) {
				followProtagonist(actor);
				if (actor->_currentAction != kActionWait)
					break;
			}

			if (actor->_targetObject != ID_NOTHING) {
				actorFaceTowardsObject(actor->_id, actor->_targetObject);
			}

			if (actor->_flags & kCycle) {
				frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameStand));
				if (frameRange->frameCount > 0) {
					actor->_actionCycle++;
					actor->_actionCycle = (actor->_actionCycle) % frameRange->frameCount;
				} else {
					actor->_actionCycle = 0;
				}
				actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
				break;
			}

			if ((actor->_actionCycle & 3) == 0) {
				actor->cycleWrap(100);

				frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameWait));
				if ((frameRange->frameCount < 1 || actor->_actionCycle > 33))
					frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameStand));

				if (frameRange->frameCount) {
					actor->_frameNumber = frameRange->frameIndex + (uint16)_vm->_rnd.getRandomNumber(frameRange->frameCount - 1);
				} else {
					actor->_frameNumber = frameRange->frameIndex;
				}
			}
			actor->_actionCycle++;
			break;

		case kActionWalkToPoint:
		case kActionWalkToLink:
			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				actor->_partialTarget.delta(actor->_location, delta);

				while ((delta.u() == 0) && (delta.v() == 0)) {

					if ((actor == _protagonist) && (_vm->mouseButtonPressed())) {
						_vm->_isoMap->screenPointToTileCoords(_vm->mousePos(), pickLocation);

						if (!actorWalkTo(_protagonist->_id, pickLocation)) {
							break;
						}
					} else if (!_vm->_isoMap->nextTileTarget(actor) && !actorEndWalk(actor->_id, true)) {
						break;
					}

					actor->_partialTarget.delta(actor->_location, delta);
					actor->_partialTarget.z = 0;
				}

				if (actor->_flags & kFastest) {
					speed = 8;
				} else if (actor->_flags & kFaster) {
					speed = 6;
				} else {
					speed = 4;
				}

				if (_vm->_scene->currentSceneResourceId() == RID_ITE_OVERMAP_SCENE) {
					speed = 2;
				}

				if ((actor->_actionDirection == 2) || (actor->_actionDirection == 6)) {
					speed = speed / 2;
				}

				if (ABS(delta.v()) > ABS(delta.u())) {
					addDelta.v() = clamp(-speed, delta.v(), speed);
					if (addDelta.v() == delta.v()) {
						addDelta.u() = delta.u();
					} else {
						addDelta.u() = delta.u() * addDelta.v();
						addDelta.u() += (addDelta.u() > 0) ? (delta.v() / 2) : (-delta.v() / 2);
						addDelta.u() /= delta.v();
					}
				} else {
					addDelta.u() = clamp(-speed, delta.u(), speed);
					if (addDelta.u() == delta.u()) {
						addDelta.v() = delta.v();
					} else {
						addDelta.v() = delta.v() * addDelta.u();
						addDelta.v() += (addDelta.v() > 0) ? (delta.u() / 2) : (-delta.u() / 2);
						addDelta.v() /= delta.u();
					}
				}

				actor->_location.add(addDelta);
			} else {
				actor->_partialTarget.delta(actor->_location, delta);

				while ((delta.x == 0) && (delta.y == 0)) {

					if (actor->_walkStepIndex >= actor->_walkStepsCount) {
						actorEndWalk(actor->_id, true);
						break;
					}

					actor->_partialTarget.fromScreenPoint(actor->_walkStepsPoints[actor->_walkStepIndex++]);
					if (_vm->getGameType() == GType_ITE) {
						if (actor->_partialTarget.x > 224 * 2 * ACTOR_LMULT) {
							actor->_partialTarget.x -= 256 * 2 * ACTOR_LMULT;
						}
					} else {
						if (actor->_partialTarget.x > 224 * 4 * ACTOR_LMULT) {
							actor->_partialTarget.x -= 256 * 4 * ACTOR_LMULT;
						}
					}

					actor->_partialTarget.delta(actor->_location, delta);

					if (ABS(delta.y) > ABS(delta.x)) {
						actor->_actionDirection = delta.y > 0 ? kDirDown : kDirUp;
					} else {
						actor->_actionDirection = delta.x > 0 ? kDirRight : kDirLeft;
					}
				}

				if (_vm->getGameType() == GType_ITE)
					speed = (ACTOR_LMULT * 2 * actor->_screenScale + 63) / 256;
				else
					speed = (ACTOR_SPEED * actor->_screenScale + 128) >> 8;

				if (speed < 1)
					speed = 1;

				if (_vm->getGameType() == GType_IHNM)
					speed = speed / 2;

				if ((actor->_actionDirection == kDirUp) || (actor->_actionDirection == kDirDown)) {
					addDelta.y = clamp(-speed, delta.y, speed);
					if (addDelta.y == delta.y) {
						addDelta.x = delta.x;
					} else {
						addDelta.x = delta.x * addDelta.y;
						addDelta.x += (addDelta.x > 0) ? (delta.y / 2) : (-delta.y / 2);
						addDelta.x /= delta.y;
						actor->_facingDirection = actor->_actionDirection;
					}
				} else {
					addDelta.x = clamp(-2 * speed, delta.x, 2 * speed);
					if (addDelta.x == delta.x) {
						addDelta.y = delta.y;
					} else {
						addDelta.y = delta.y * addDelta.x;
						addDelta.y += (addDelta.y > 0) ? (delta.x / 2) : (-delta.x / 2);
						addDelta.y /= delta.x;
						actor->_facingDirection = actor->_actionDirection;
					}
				}

				actor->_location.add(addDelta);
			}

			if (actor->_actorFlags & kActorBackwards) {
				actor->_facingDirection = (actor->_actionDirection + 4) & 7;
				actor->_actionCycle--;
			} else {
				actor->_actionCycle++;
			}

			frameRange = getActorFrameRange(actor->_id, actor->_walkFrameSequence);

			if (actor->_actionCycle < 0) {
				actor->_actionCycle = frameRange->frameCount - 1;
			} else if (actor->_actionCycle >= frameRange->frameCount) {
				actor->_actionCycle = 0;
			}

			actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			break;

		case kActionWalkDir:
			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				actor->_location.u() += tileDirectionLUT[actor->_actionDirection][0];
				actor->_location.v() += tileDirectionLUT[actor->_actionDirection][1];

				frameRange = getActorFrameRange(actor->_id, actor->_walkFrameSequence);

				actor->_actionCycle++;
				actor->cycleWrap(frameRange->frameCount);
				actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			} else {
				if (_vm->getGameType() == GType_ITE) {
					actor->_location.x += directionLUT[actor->_actionDirection][0] * 2;
					actor->_location.y += directionLUT[actor->_actionDirection][1] * 2;
				} else {
					// FIXME: The original does not multiply by 8 here, but we do
					actor->_location.x += (directionLUT[actor->_actionDirection][0] * 8 * actor->_screenScale + 128) >> 8;
					actor->_location.y += (directionLUT[actor->_actionDirection][1] * 8 * actor->_screenScale + 128) >> 8;
				}

				frameRange = getActorFrameRange(actor->_id, actor->_walkFrameSequence);
				actor->_actionCycle++;
				actor->cycleWrap(frameRange->frameCount);
				actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			}
			break;

		case kActionSpeak:
			actor->_actionCycle++;
			actor->cycleWrap(64);

			frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameGesture));
			if (actor->_actionCycle >= frameRange->frameCount) {
				if (actor->_actionCycle & 1)
					break;
				frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameSpeak));

				state = (uint16)_vm->_rnd.getRandomNumber(frameRange->frameCount);

				if (state == 0) {
					frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameStand));
				} else {
					state--;
				}
			} else {
				state = actor->_actionCycle;
			}

			actor->_frameNumber = frameRange->frameIndex + state;
			break;

		case kActionAccept:
		case kActionStoop:
			break;

		case kActionCycleFrames:
		case kActionPongFrames:
			if (actor->_cycleTimeCount > 0) {
				actor->_cycleTimeCount--;
				break;
			}

			actor->_cycleTimeCount = actor->_cycleDelay;
			actor->_actionCycle++;

			frameRange = getActorFrameRange(actor->_id, actor->_cycleFrameSequence);

			if (actor->_currentAction == kActionPongFrames) {
				if (actor->_actionCycle >= frameRange->frameCount * 2 - 2) {
					if (actor->_actorFlags & kActorContinuous) {
						actor->_actionCycle = 0;
					} else {
						actor->_currentAction = kActionFreeze;
						break;
					}
				}

				state = actor->_actionCycle;
				if (state >= frameRange->frameCount) {
					state = frameRange->frameCount * 2 - 2 - state;
				}
			} else {
				if (actor->_actionCycle >= frameRange->frameCount) {
					if (actor->_actorFlags & kActorContinuous) {
						actor->_actionCycle = 0;
					} else {
						actor->_currentAction = kActionFreeze;
						break;
					}
				}
				state = actor->_actionCycle;
			}

			if (frameRange->frameCount && (actor->_actorFlags & kActorRandom)) {
				state = _vm->_rnd.getRandomNumber(frameRange->frameCount - 1);
			}

			if (actor->_actorFlags & kActorBackwards) {
				actor->_frameNumber = frameRange->frameIndex + frameRange->frameCount - 1 - state;
			} else {
				actor->_frameNumber = frameRange->frameIndex + state;
			}
			break;

		case kActionFall:
			if (actor->_actionCycle > 0) {
				framesLeft = actor->_actionCycle--;
				actor->_finalTarget.delta(actor->_location, delta);
				delta.x /= framesLeft;
				delta.y /= framesLeft;
				actor->_location.addXY(delta);
				actor->_fallVelocity += actor->_fallAcceleration;
				actor->_fallPosition += actor->_fallVelocity;
				actor->_location.z = actor->_fallPosition >> 4;
			} else {
				actor->_location = actor->_finalTarget;
				actor->_currentAction = kActionFreeze;
				_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
			}
			break;

		case kActionClimb:
			actor->_cycleDelay++;
			if (actor->_cycleDelay & 3) {
				break;
			}

			if (actor->_location.z >= actor->_finalTarget.z + ACTOR_CLIMB_SPEED) {
				actor->_location.z -= ACTOR_CLIMB_SPEED;
				actor->_actionCycle--;
			} else if (actor->_location.z <= actor->_finalTarget.z - ACTOR_CLIMB_SPEED) {
				actor->_location.z += ACTOR_CLIMB_SPEED;
				actor->_actionCycle++;
			} else {
				actor->_location.z = actor->_finalTarget.z;
				actor->_currentAction = kActionFreeze;
				_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
			}

			frameRange = getActorFrameRange(actor->_id, actor->_cycleFrameSequence);

			if (actor->_actionCycle < 0) {
				actor->_actionCycle = frameRange->frameCount - 1;
			}
			actor->cycleWrap(frameRange->frameCount);
			actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			break;
		}

		if ((actor->_currentAction >= kActionWalkToPoint) && (actor->_currentAction <= kActionWalkDir)) {
			hitZone = NULL;

			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				actor->_location.toScreenPointUV(hitPoint);
			} else {
				actor->_location.toScreenPointXY(hitPoint);
			}
			hitZoneIndex = _vm->_scene->_actionMap->hitTest(hitPoint);
			if (hitZoneIndex != -1) {
				hitZone = _vm->_scene->_actionMap->getHitZone(hitZoneIndex);
			}

			if (hitZone != actor->_lastZone) {
				if (actor->_lastZone)
					stepZoneAction(actor, actor->_lastZone, true, false);
				actor->_lastZone = hitZone;
				// WORKAROUND for graphics glitch in the rat caves. Don't do this step zone action in the rat caves
				// (room 51) for hitzone 24577 (the door with the copy protection) to avoid the glitch. This glitch
				// happens because the copy protection is supposed to kick in at this point, but it's bypassed
				// (with permission from Wyrmkeep Entertainment)
				if (hitZone && 
					!(_vm->getGameType() == GType_ITE && _vm->_scene->currentSceneNumber() == 51 && hitZone->getHitZoneId() == 24577))
					stepZoneAction(actor, hitZone, false, false);
			}
		}
	}
	// Update frameCount for sfWaitFrames in IHNM
	_vm->_frameCount++;
}

void Actor::direct(int msec) {

	if (_vm->_scene->_entryList.entryListCount == 0) {
		return;
	}

	if (_vm->_interface->_statusTextInput) {
		return;
	}

	// FIXME: HACK. This should be turned into cycle event.
	_lastTickMsec += msec;

	if (_lastTickMsec > 1000 / _handleActionDiv) {
		_lastTickMsec = 0;
		//process actions
		handleActions(msec, false);
	}

//process speech
	handleSpeech(msec);
}

bool Actor::followProtagonist(ActorData *actor) {
	Location protagonistLocation;
	Location newLocation;
	Location delta;
	int protagonistBGMaskType;
	Point prefer1;
	Point prefer2;
	Point prefer3;
	int16 prefU;
	int16 prefV;
	int16 newU;
	int16 newV;

	assert(_protagonist);

	actor->_flags &= ~(kFaster | kFastest);
	protagonistLocation = _protagonist->_location;
	calcScreenPosition(_protagonist);

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		prefU = 60;
		prefV = 60;


		actor->_location.delta(protagonistLocation, delta);

		if (actor->_id == actorIndexToId(2)) {
			prefU = prefV = 48;
		}

		if ((delta.u() > prefU) || (delta.u() < -prefU) || (delta.v() > prefV) || (delta.v() < -prefV)) {

			if ((delta.u() > prefU * 2) || (delta.u() < -prefU * 2) || (delta.v() > prefV * 2) || (delta.v() < -prefV * 2)) {
				actor->_flags |= kFaster;

				if ((delta.u() > prefU * 3) || (delta.u() < -prefU*3) || (delta.v() > prefV * 3) || (delta.v() < -prefV * 3)) {
					actor->_flags |= kFastest;
				}
			}

			prefU /= 2;
			prefV /= 2;

			newU = clamp(-prefU, delta.u(), prefU) + protagonistLocation.u();
			newV = clamp(-prefV, delta.v(), prefV) + protagonistLocation.v();

			newLocation.u() = newU + _vm->_rnd.getRandomNumber(prefU - 1) - prefU / 2;
			newLocation.v() = newV + _vm->_rnd.getRandomNumber(prefV - 1) - prefV / 2;
			newLocation.z = 0;

			return actorWalkTo(actor->_id, newLocation);
		}

	} else {
		prefer1.x = (100 * _protagonist->_screenScale) >> 8;
		prefer1.y = (50 * _protagonist->_screenScale) >> 8;

		if (_protagonist->_currentAction == kActionWalkDir) {
			prefer1.x /= 2;
		}

		if (prefer1.x < 8) {
			prefer1.x = 8;
		}

		if (prefer1.y < 8) {
			prefer1.y = 8;
		}

		prefer2.x = prefer1.x * 2;
		prefer2.y = prefer1.y * 2;
		prefer3.x = prefer1.x + prefer1.x / 2;
		prefer3.y = prefer1.y + prefer1.y / 2;

		actor->_location.delta(protagonistLocation, delta);

		protagonistBGMaskType = 0;
		if (_vm->_scene->isBGMaskPresent() && _vm->_scene->validBGMaskPoint(_protagonist->_screenPosition)) {
			protagonistBGMaskType = _vm->_scene->getBGMaskType(_protagonist->_screenPosition);
		}

		if ((_vm->_rnd.getRandomNumber(7) & 0x7) == 0) // 1/8th chance
			actor->_actorFlags &= ~kActorNoFollow;

		if (actor->_actorFlags & kActorNoFollow) {
			return false;
		}

		if ((delta.x > prefer2.x) || (delta.x < -prefer2.x) ||
			(delta.y > prefer2.y) || (delta.y < -prefer2.y) ||
			((_protagonist->_currentAction == kActionWait) &&
			(delta.x * 2 < prefer1.x) && (delta.x * 2 > -prefer1.x) &&
			(delta.y < prefer1.y) && (delta.y > -prefer1.y))) {

				if (ABS(delta.x) > ABS(delta.y)) {

					delta.x = (delta.x > 0) ? prefer3.x : -prefer3.x;

					newLocation.x = delta.x + protagonistLocation.x;
					newLocation.y = clamp(-prefer2.y, delta.y, prefer2.y) + protagonistLocation.y;
				} else {
					delta.y = (delta.y > 0) ? prefer3.y : -prefer3.y;

					newLocation.x = clamp(-prefer2.x, delta.x, prefer2.x) + protagonistLocation.x;
					newLocation.y = delta.y + protagonistLocation.y;
				}
				newLocation.z = 0;

				if (protagonistBGMaskType != 3) {
					newLocation.x += _vm->_rnd.getRandomNumber(prefer1.x - 1) - prefer1.x / 2;
					newLocation.y += _vm->_rnd.getRandomNumber(prefer1.y - 1) - prefer1.y / 2;
				}

				newLocation.x = clamp(-31 * 4, newLocation.x, (_vm->getDisplayWidth() + 31) * 4);

				return actorWalkTo(actor->_id, newLocation);
			}
	}
	return false;
}

bool Actor::actorWalkTo(uint16 actorId, const Location &toLocation) {
	ActorData *actor;
	ActorData *anotherActor;
	int	i;

	Rect testBox;
	Rect testBox2;
	Point anotherActorScreenPosition;
	Point collision;
	Point pointFrom, pointTo, pointBest, pointAdd;
	Point delta, bestDelta;
	Point tempPoint;
	bool extraStartNode;
	bool extraEndNode;

	actor = getActor(actorId);

	if (actor == _protagonist) {
		_vm->_scene->setDoorState(2, 0xff);
		_vm->_scene->setDoorState(3, 0);
	} else {
		_vm->_scene->setDoorState(2, 0);
		_vm->_scene->setDoorState(3, 0xff);
	}

	if (_vm->_scene->getFlags() & kSceneFlagISO) {

		if ((_vm->getGameType() == GType_ITE) && (actor->_index == ACTOR_DRAGON_INDEX)) {
			return false;
		}

		actor->_finalTarget = toLocation;
		actor->_walkStepsCount = 0;
		_vm->_isoMap->findTilePath(actor, actor->_location, toLocation);


		if ((actor->_walkStepsCount == 0) && (actor->_flags & kProtagonist)) {
			actor->_actorFlags |= kActorNoCollide;
			_vm->_isoMap->findTilePath(actor, actor->_location, toLocation);
		}

		actor->_walkStepIndex = 0;
		if (_vm->_isoMap->nextTileTarget(actor)) {
			actor->_currentAction = kActionWalkToPoint;
			actor->_walkFrameSequence = getFrameType(kFrameWalk);
		} else {
			actorEndWalk(actorId, false);
			return false;
		}
	} else {

		actor->_location.toScreenPointXY(pointFrom);
		pointFrom.x &= ~1;

		extraStartNode = _vm->_scene->offscreenPath(pointFrom);

		toLocation.toScreenPointXY(pointTo);
		pointTo.x &= ~1;

		extraEndNode = _vm->_scene->offscreenPath(pointTo);

		if (_vm->_scene->isBGMaskPresent()) {

			if ((((actor->_currentAction >= kActionWalkToPoint) &&
				(actor->_currentAction <= kActionWalkDir)) || (actor == _protagonist)) &&
				!_vm->_scene->canWalk(pointFrom)) {
				
				int max = _vm->getGameType() == GType_ITE ? 8 : 4;

				for (i = 1; i < max; i++) {
					pointAdd = pointFrom;
					pointAdd.y += i;
					if (_vm->_scene->canWalk(pointAdd)) {
						pointFrom = pointAdd;
						break;
					}
					pointAdd = pointFrom;
					pointAdd.y -= i;
					if (_vm->_scene->canWalk(pointAdd)) {
						pointFrom = pointAdd;
						break;
					}
					if (_vm->getGameType() == GType_ITE) {
						pointAdd = pointFrom;
						pointAdd.x += i;
						if (_vm->_scene->canWalk(pointAdd)) {
							pointFrom = pointAdd;
							break;
						}
						pointAdd = pointFrom;
						pointAdd.x -= i;
						if (_vm->_scene->canWalk(pointAdd)) {
							pointFrom = pointAdd;
							break;
						}
					}
				}
			}

			_barrierCount = 0;
			if (!(actor->_actorFlags & kActorNoCollide)) {
				collision.x = ACTOR_COLLISION_WIDTH * actor->_screenScale / (256 * 2);
				collision.y = ACTOR_COLLISION_HEIGHT * actor->_screenScale / (256 * 2);


				for (i = 0; (i < _actorsCount) && (_barrierCount < ACTOR_BARRIERS_MAX); i++) {
					anotherActor = _actors[i];
					if (!anotherActor->_inScene)
						continue;
					if (anotherActor == actor)
						continue;

					anotherActorScreenPosition = anotherActor->_screenPosition;
					testBox.left = (anotherActorScreenPosition.x - collision.x) & ~1;
					testBox.right = (anotherActorScreenPosition.x + collision.x) & ~1 + 1;
					testBox.top = anotherActorScreenPosition.y - collision.y;
					testBox.bottom = anotherActorScreenPosition.y + collision.y + 1;
					testBox2 = testBox;
					testBox2.right += 2;
					testBox2.left -= 2;
					testBox2.top -= 1;
					testBox2.bottom += 1;

					if (testBox2.contains(pointFrom)) {
						if (pointFrom.x > anotherActorScreenPosition.x + 4) {
							testBox.right = pointFrom.x - 1;
						} else if (pointFrom.x < anotherActorScreenPosition.x - 4) {
							testBox.left = pointFrom.x + 2;
						} else if (pointFrom.y > anotherActorScreenPosition.y) {
							testBox.bottom = pointFrom.y;
						} else {
							testBox.top = pointFrom.y + 1 ;
						}
					}

					if ((testBox.width() > 0) && (testBox.height() > 0)) {
						_barrierList[_barrierCount++] = testBox;
					}
				}
			}


			pointBest = pointTo;
			actor->_walkStepsCount = 0;
			findActorPath(actor, pointFrom, pointTo);

			if (actor->_walkStepsCount == 0) {
				error("actor->_walkStepsCount == 0");
			}

			if (extraStartNode) {
				actor->_walkStepIndex = 0;
			} else {
				actor->_walkStepIndex = 1;
			}

			if (extraEndNode) {
				toLocation.toScreenPointXY(tempPoint);
				actor->_walkStepsCount--;
				actor->addWalkStepPoint(tempPoint);
			}


			pointBest = actor->_walkStepsPoints[actor->_walkStepsCount - 1];

			pointBest.x &= ~1;
			delta.x = ABS(pointFrom.x - pointTo.x);
			delta.y = ABS(pointFrom.y - pointTo.y);

			bestDelta.x = ABS(pointBest.x - pointTo.x);
			bestDelta.y = ABS(pointBest.y - pointTo.y);

			if ((delta.x + delta.y <= bestDelta.x + bestDelta.y) && (actor->_flags & kFollower)) {
				actor->_actorFlags |= kActorNoFollow;
			}

			if (pointBest == pointFrom) {
				actor->_walkStepsCount = 0;
			}
		} else {
			actor->_walkStepsCount = 0;
			actor->addWalkStepPoint(pointTo);
			actor->_walkStepIndex = 0;
		}

		actor->_partialTarget = actor->_location;
		actor->_finalTarget = toLocation;
		if (actor->_walkStepsCount == 0) {
			actorEndWalk(actorId, false);
			return false;
		} else {
			if (actor->_flags & kProtagonist) {
				_actors[1]->_actorFlags &= ~kActorNoFollow; // TODO: mark all actors with kFollower flag, not only 1 and 2
				_actors[2]->_actorFlags &= ~kActorNoFollow;
			}
			actor->_currentAction = (actor->_walkStepsCount >= ACTOR_MAX_STEPS_COUNT) ? kActionWalkToLink : kActionWalkToPoint;
			actor->_walkFrameSequence = getFrameType(kFrameWalk);
		}
	}
	return true;
}

bool Actor::actorEndWalk(uint16 actorId, bool recurse) {
	bool walkMore = false;
	ActorData *actor;
	const HitZone *hitZone;
	int hitZoneIndex;
	Point testPoint;

	actor = getActor(actorId);
	actor->_actorFlags &= ~kActorBackwards;

	if (_vm->getGameType() == GType_ITE) {

		if (actor->_location.distance(actor->_finalTarget) > 8 && (actor->_flags & kProtagonist) && recurse && !(actor->_actorFlags & kActorNoCollide)) {
			actor->_actorFlags |= kActorNoCollide;
			return actorWalkTo(actorId, actor->_finalTarget);
		}
	}

	actor->_currentAction = kActionWait;
	if (actor->_actorFlags & kActorFinalFace) {
		actor->_facingDirection = actor->_actionDirection = (actor->_actorFlags >> 6) & 0x07; //?
	}

	actor->_actorFlags &= ~(kActorNoCollide | kActorCollided | kActorFinalFace | kActorFacingMask);
	actor->_flags &= ~(kFaster | kFastest);

	if (actor == _protagonist) {
		_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
		if (_vm->_script->_pendingVerb == _vm->_script->getVerbType(kVerbWalkTo)) {
			if (_vm->getGameType() == GType_ITE)
				actor->_location.toScreenPointUV(testPoint); // it's wrong calculation, but it is used in ITE
			else
				actor->_location.toScreenPointXY(testPoint);
				
			hitZoneIndex = _vm->_scene->_actionMap->hitTest(testPoint);
			if (hitZoneIndex != -1) {
				hitZone = _vm->_scene->_actionMap->getHitZone(hitZoneIndex);
				stepZoneAction(actor, hitZone, false, true);
			} else {
				_vm->_script->setNoPendingVerb();
			}
		} else if (_vm->_script->_pendingVerb != _vm->_script->getVerbType(kVerbNone)) {
			_vm->_script->doVerb();
		}
	} else {
		if (recurse && (actor->_flags & kFollower))
			walkMore = followProtagonist(actor);

		_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
	}
	return walkMore;
}

void Actor::moveDragon(ActorData *actor) {
	int16 dir0, dir1, dir2, dir3;
	int16 moveType;
	Event event;
	const DragonMove *dragonMove;

	if ((actor->_actionCycle < 0) ||
		((actor->_actionCycle == 0) && (actor->_dragonMoveType >= ACTOR_DRAGON_TURN_MOVES))) {

		moveType = kDragonMoveInvalid;
		if (actor->_location.distance(_protagonist->_location) < 24) {
			if (_dragonHunt && (_protagonist->_currentAction != kActionFall)) {
				event.type = kEvTOneshot;
				event.code = kScriptEvent;
				event.op = kEventExecNonBlocking;
				event.time = 0;
				event.param = _vm->_scene->getScriptModuleNumber(); // module number
				event.param2 = ACTOR_EXP_KNOCK_RIF;			// script entry point number
				event.param3 = -1;		// Action
				event.param4 = -1;		// Object
				event.param5 = -1;		// With Object
				event.param6 = -1;		// Actor
				_vm->_events->queue(&event);

				_dragonHunt = false;
			}
		} else {
			_dragonHunt = true;
		}

		if (actor->_walkStepIndex + 2 > actor->_walkStepsCount) {

			_vm->_isoMap->findDragonTilePath(actor, actor->_location, _protagonist->_location, actor->_actionDirection);

			if (actor->_walkStepsCount == 0) {
				_vm->_isoMap->findDragonTilePath(actor, actor->_location, _protagonist->_location, 0);
			}

			if (actor->_walkStepsCount < 2) {
				return;
			}

			actor->_partialTarget = actor->_location;
			actor->_finalTarget = _protagonist->_location;
			actor->_walkStepIndex = 0;
		}

		dir0 = actor->_actionDirection;
		dir1 = actor->_tileDirections[actor->_walkStepIndex++];
		dir2 = actor->_tileDirections[actor->_walkStepIndex];
		dir3 = actor->_tileDirections[actor->_walkStepIndex + 1];

		if (dir0 != dir1){
			actor->_actionDirection = dir0 = dir1;
		}

		actor->_location = actor->_partialTarget;

		if ((dir1 != dir2) && (dir1 == dir3)) {
			switch (dir1) {
			case kDirUpLeft:
				actor->_partialTarget.v() += 16;
				moveType = kDragonMoveUpLeft;
				break;
			case kDirDownLeft:
				actor->_partialTarget.u() -= 16;
				moveType = kDragonMoveDownLeft;
				break;
			case kDirDownRight:
				actor->_partialTarget.v() -= 16;
				moveType = kDragonMoveDownRight;
				break;
			case kDirUpRight:
				actor->_partialTarget.u() += 16;
				moveType = kDragonMoveUpRight;
				break;
			}

			switch (dir2) {
			case kDirUpLeft:
				actor->_partialTarget.v() += 16;
				break;
			case kDirDownLeft:
				actor->_partialTarget.u() -= 16;
				break;
			case kDirDownRight:
				actor->_partialTarget.v() -= 16;
				break;
			case kDirUpRight:
				actor->_partialTarget.u() += 16;
				break;
			}

			actor->_walkStepIndex++;
		} else {
			switch (dir1) {
			case kDirUpLeft:
				actor->_partialTarget.v() += 16;
				switch (dir2) {
				case kDirDownLeft:
					moveType = kDragonMoveUpLeft_Left;
					actor->_partialTarget.u() -= 16;
					break;
				case kDirUpLeft:
					moveType = kDragonMoveUpLeft;
					break;
				case kDirUpRight:
					actor->_partialTarget.u() += 16;
					moveType = kDragonMoveUpLeft_Right;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;
			case kDirDownLeft:
				actor->_partialTarget.u() -= 16;
				switch (dir2) {
				case kDirDownRight:
					moveType = kDragonMoveDownLeft_Left;
					actor->_partialTarget.v() -= 16;
					break;
				case kDirDownLeft:
					moveType = kDragonMoveDownLeft;
					break;
				case kDirUpLeft:
					moveType = kDragonMoveDownLeft_Right;
					actor->_partialTarget.v() += 16;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;
			case kDirDownRight:
				actor->_partialTarget.v() -= 16;
				switch (dir2) {
				case kDirUpRight:
					moveType = kDragonMoveDownRight_Left;
					actor->_partialTarget.u() += 16;
					break;
				case kDirDownRight:
					moveType = kDragonMoveDownRight;
					break;
				case kDirDownLeft:
					moveType = kDragonMoveDownRight_Right;
					actor->_partialTarget.u() -= 16;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;
			case kDirUpRight:
				actor->_partialTarget.u() += 16;
				switch (dir2) {
				case kDirUpLeft:
					moveType = kDragonMoveUpRight_Left;
					actor->_partialTarget.v() += 16;
					break;
				case kDirUpRight:
					moveType = kDragonMoveUpRight;
					break;
				case kDirDownRight:
					moveType = kDragonMoveUpRight_Right;
					actor->_partialTarget.v() -= 16;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;

			default:
				actor->_actionDirection = dir1;
				actor->_walkStepsCount = 0;
				break;
			}
		}

		actor->_dragonMoveType = moveType;

		if (moveType >= ACTOR_DRAGON_TURN_MOVES) {
			actor->_dragonStepCycle = 0;
			actor->_actionCycle = 4;
			actor->_walkStepIndex++;
		} else {
			actor->_actionCycle = 4;
		}
	}

	actor->_actionCycle--;

	if ((actor->_walkStepsCount < 1) || (actor->_actionCycle < 0)) {
		return;
	}

	if (actor->_dragonMoveType < ACTOR_DRAGON_TURN_MOVES) {

		actor->_dragonStepCycle++;
		if (actor->_dragonStepCycle >= 7) {
			actor->_dragonStepCycle = 0;
		}

		actor->_dragonBaseFrame = actor->_dragonMoveType * 7;

		if (actor->_location.u() > actor->_partialTarget.u() + 3) {
			actor->_location.u() -= 4;
		} else if (actor->_location.u() < actor->_partialTarget.u() - 3) {
			actor->_location.u() += 4;
		} else {
			actor->_location.u() = actor->_partialTarget.u();
		}

		if (actor->_location.v() > actor->_partialTarget.v() + 3) {
			actor->_location.v() -= 4;
		} else if (actor->_location.v() < actor->_partialTarget.v() - 3) {
			actor->_location.v() += 4;
		} else {
			actor->_location.v() = actor->_partialTarget.v();
		}
	} else {
		dragonMove = &dragonMoveTable[actor->_dragonMoveType];
		actor->_dragonBaseFrame = dragonMove->baseFrame;


		actor->_location.u() = actor->_partialTarget.u() - dragonMove->offset[actor->_actionCycle][0];
		actor->_location.v() = actor->_partialTarget.v() - dragonMove->offset[actor->_actionCycle][1];

		actor->_dragonStepCycle++;
		if (actor->_dragonStepCycle >= 3) {
			actor->_dragonStepCycle = 3;
		}
	}

	actor->_frameNumber = actor->_dragonBaseFrame + actor->_dragonStepCycle;
}

void Actor::findActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point iteratorPoint;
	Point bestPoint;
	int maskType;
	int i;
	Rect intersect;

#ifdef ACTOR_DEBUG
	_debugPointsCount = 0;
#endif

	actor->_walkStepsCount = 0;
	if (fromPoint == toPoint) {
		actor->addWalkStepPoint(toPoint);
		return;
	}

	for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
		for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
			if (_vm->_scene->validBGMaskPoint(iteratorPoint)) {
				maskType = _vm->_scene->getBGMaskType(iteratorPoint);
				setPathCell(iteratorPoint, _vm->_scene->getDoorState(maskType) ? kPathCellBarrier : kPathCellEmpty);
			} else {
				setPathCell(iteratorPoint, kPathCellBarrier);
			}
		}
	}

	for (i = 0; i < _barrierCount; i++) {
		intersect.left = MAX(_pathRect.left, _barrierList[i].left);
		intersect.top = MAX(_pathRect.top, _barrierList[i].top);
		intersect.right = MIN(_pathRect.right, _barrierList[i].right);
		intersect.bottom = MIN(_pathRect.bottom, _barrierList[i].bottom);

		for (iteratorPoint.y = intersect.top; iteratorPoint.y < intersect.bottom; iteratorPoint.y++) {
			for (iteratorPoint.x = intersect.left; iteratorPoint.x < intersect.right; iteratorPoint.x++) {
				setPathCell(iteratorPoint, kPathCellBarrier);
			}
		}
	}

#ifdef ACTOR_DEBUG
	for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
		for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
			if (getPathCell(iteratorPoint) == kPathCellBarrier) {
				addDebugPoint(iteratorPoint, 24);
			}
		}
	}
#endif

	if (scanPathLine(fromPoint, toPoint)) {
		actor->addWalkStepPoint(fromPoint);
		actor->addWalkStepPoint(toPoint);
		return;
	}

	i = fillPathArray(fromPoint, toPoint, bestPoint);

	if (fromPoint == bestPoint) {
		actor->addWalkStepPoint(bestPoint);
		return;
	}

	if (i == 0) {
		error("fillPathArray returns zero");
	}

	setActorPath(actor, fromPoint, bestPoint);
}

bool Actor::scanPathLine(const Point &point1, const Point &point2) {
	Point point;
	Point delta;
	Point s;
	Point fDelta;
	int16 errterm;

	calcDeltaS(point1, point2, delta, s);
	point = point1;

	fDelta.x = delta.x * 2;
	fDelta.y = delta.y * 2;

	if (delta.y > delta.x) {

		errterm = fDelta.x - delta.y;

		while (delta.y > 0) {
			while (errterm >= 0) {
				point.x += s.x;
				errterm -= fDelta.y;
			}
			
			point.y += s.y;		
			errterm += fDelta.x;

			if (!validPathCellPoint(point)) {
				return false;
			}
			if (getPathCell(point) == kPathCellBarrier) {
				return false;
			}
			delta.y--;
		}
	} else {

		errterm = fDelta.y - delta.x;

		while (delta.x > 0) {
			while (errterm >= 0) {
				point.y += s.y;
				errterm -= fDelta.x;
			}
			
			point.x += s.x;
			errterm += fDelta.y;

			if (!validPathCellPoint(point)) {
				return false;
			}
			if (getPathCell(point) == kPathCellBarrier) {
				return false;
			}
			delta.x--;
		}
	}
	return true;
}

int Actor::fillPathArray(const Point &fromPoint, const Point &toPoint, Point &bestPoint) {
	int bestRating;
	int currentRating;
	int i;
	Point bestPath;
	int pointCounter;
	int startDirection;
	PathDirectionData *pathDirection;
	PathDirectionData *newPathDirection;
	const PathDirectionData *samplePathDirection;
	Point nextPoint;
	int directionCount;
	int16 compressX = (_vm->getGameType() == GType_ITE) ? 2 : 1;

	_pathDirectionListCount = 0;
	pointCounter = 0;
	bestRating = quickDistance(fromPoint, toPoint, compressX);
	bestPath = fromPoint;

	for (startDirection = 0; startDirection < 4; startDirection++) {
		newPathDirection = addPathDirectionListData();
		newPathDirection->coord = fromPoint;
		newPathDirection->direction = startDirection;
	}

	if (validPathCellPoint(fromPoint)) {
		setPathCell(fromPoint, kDirUp);

#ifdef ACTOR_DEBUG
		addDebugPoint(fromPoint, 24+36);
#endif
	}

	i = 0;

	do {
		pathDirection = &_pathDirectionList[i];
		for (directionCount = 0; directionCount < 3; directionCount++) {
			samplePathDirection = &pathDirectionLUT[pathDirection->direction][directionCount];
			nextPoint = pathDirection->coord;
			nextPoint.x += samplePathDirection->coord.x;
			nextPoint.y += samplePathDirection->coord.y;

			if (!validPathCellPoint(nextPoint)) {
				continue;
			}

			if (getPathCell(nextPoint) != kPathCellEmpty) {
				continue;
			}

			setPathCell(nextPoint, samplePathDirection->direction);

#ifdef ACTOR_DEBUG
			addDebugPoint(nextPoint, samplePathDirection->direction + 96);
#endif
			newPathDirection = addPathDirectionListData();
			newPathDirection->coord = nextPoint;
			newPathDirection->direction = samplePathDirection->direction;
			++pointCounter;
			if (nextPoint == toPoint) {
				bestPoint = toPoint;
				return pointCounter;
			}
			currentRating = quickDistance(nextPoint, toPoint, compressX);
			if (currentRating < bestRating) {
				bestRating = currentRating;
				bestPath = nextPoint;
			}
			pathDirection = &_pathDirectionList[i];
		}
		++i;
	} while (i < _pathDirectionListCount);

	bestPoint = bestPath;
	return pointCounter;
}

void Actor::setActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point nextPoint;
	int8 direction;
	int i;

	_pathListIndex = -1;
	addPathListPoint(toPoint);
	nextPoint = toPoint;

	while (!(nextPoint == fromPoint)) {
		direction = getPathCell(nextPoint);
		if ((direction < 0) || (direction >= 8)) {
			error("Actor::setActorPath error direction 0x%X", direction);
		}
		nextPoint.x -= pathDirectionLUT2[direction][0];
		nextPoint.y -= pathDirectionLUT2[direction][1];
		addPathListPoint(nextPoint);

#ifdef ACTOR_DEBUG
		addDebugPoint(nextPoint, 0x8a);
#endif
	}

	pathToNode();
	removeNodes();
	nodeToPath();
	removePathPoints();

	for (i = 0; i <= _pathNodeListIndex; i++) {
		actor->addWalkStepPoint(_pathNodeList[i].point);
	}
}

void Actor::pathToNode() {
	Point point1, point2, delta;
	int direction;
	int i;
	Point *point;

	point= &_pathList[_pathListIndex];
	direction = 0;

	_pathNodeListIndex = -1;
	addPathNodeListPoint(*point);

	for (i = _pathListIndex; i > 0; i--) {
		point1 = *point;
		--point;
		point2 = *point;
		if (direction == 0) {
			delta.x = int16Compare(point2.x, point1.x);
			delta.y = int16Compare(point2.y, point1.y);
			direction++;
		}
		if ((point1.x + delta.x != point2.x) || (point1.y + delta.y != point2.y)) {
			addPathNodeListPoint(point1);
			direction--;
			i++;
			point++;
		}
	}
	addPathNodeListPoint(*_pathList);
}

int pathLine(Point *pointList, const Point &point1, const Point &point2) {
	Point point;
	Point delta;
	Point tempPoint;
	Point s;
	int16 errterm;
	int16 res;

	calcDeltaS(point1, point2, delta, s);

	point = point1;

	tempPoint.x = delta.x * 2;
	tempPoint.y = delta.y * 2;

	if (delta.y > delta.x) {
		
		errterm = tempPoint.x - delta.y;
		res = delta.y;

		while (delta.y > 0) {
			while (errterm >= 0) {
				point.x += s.x;
				errterm -= tempPoint.y;
			}

			point.y += s.y;
			errterm += tempPoint.x;

			*pointList = point;
			pointList++;
			delta.y--;
		}
	} else {

		errterm = tempPoint.y - delta.x;
		res = delta.x;

		while (delta.x > 0) {
			while (errterm >= 0) {
				point.y += s.y;
				errterm -= tempPoint.x;
			}

			point.x += s.x;
			errterm += tempPoint.y;

			*pointList = point;
			pointList++;
			delta.x--;
		}
	}
	return res;
}

void Actor::nodeToPath() {
	int i;
	Point point1, point2;
	PathNode *node;
	Point *point;

	for (i = 0, point = _pathList; i < _pathListAlloced; i++, point++) {
		point->x = point->y = PATH_NODE_EMPTY;
	}

	_pathListIndex = 1;
	_pathList[0] = _pathNodeList[0].point;
	_pathNodeList[0].link = 0;
	for (i = 0, node = _pathNodeList; i < _pathNodeListIndex; i++) {
		point1 = node->point;
		node++;
		point2 = node->point;
		_pathListIndex += pathLine(&_pathList[_pathListIndex], point1, point2);
		node->link = _pathListIndex - 1;
	}
	_pathListIndex--;
	_pathNodeList[_pathNodeListIndex].link = _pathListIndex;

}

void Actor::removeNodes() {
	int i, j, k;
	PathNode *iNode, *jNode, *kNode, *fNode;
	fNode = &_pathNodeList[_pathNodeListIndex];

	if (scanPathLine(_pathNodeList[0].point, fNode->point)) {
		_pathNodeList[1] = *fNode;
		_pathNodeListIndex = 1;
	}

	if (_pathNodeListIndex < 4) {
		return;
	}

	for (i = _pathNodeListIndex - 1, iNode = fNode-1; i > 1 ; i--, iNode--) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			continue;
		}

		if (scanPathLine(_pathNodeList[0].point, iNode->point)) {
			for (j = 1, jNode = _pathNodeList + 1; j < i; j++, jNode++) {
				jNode->point.x = PATH_NODE_EMPTY;
			}
		}
	}

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeListIndex - 1; i++, iNode++) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			continue;
		}

		if (scanPathLine(fNode->point, iNode->point)) {
			for (j = i + 1, jNode = iNode + 1; j < _pathNodeListIndex; j++, jNode++) {
				jNode->point.x = PATH_NODE_EMPTY;
			}
		}
	}
	condenseNodeList();

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeListIndex - 1; i++, iNode++) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			continue;
		}
		for (j = i + 2, jNode = iNode + 2; j < _pathNodeListIndex; j++, jNode++) {
			if (jNode->point.x == PATH_NODE_EMPTY) {
				continue;
			}

			if (scanPathLine(iNode->point, jNode->point)) {
				for (k = i + 1,kNode = iNode + 1; k < j; k++, kNode++) {
					kNode->point.x = PATH_NODE_EMPTY;
				}
			}
		}
	}
	condenseNodeList();
}

void Actor::condenseNodeList() {
	int i, j, count;
	PathNode *iNode, *jNode;

	count = _pathNodeListIndex;

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeListIndex; i++, iNode++) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			j = i + 1;
			jNode = iNode + 1;
			while (jNode->point.x == PATH_NODE_EMPTY) {
				j++;
				jNode++;
			}
			*iNode = *jNode;
			count = i;
			jNode->point.x = PATH_NODE_EMPTY;
			if (j == _pathNodeListIndex) {
				break;
			}
		}
	}
	_pathNodeListIndex = count;
}

void Actor::removePathPoints() {
	int i, j, k, l;
	PathNode *node;
	int start;
	int end;
	Point point1, point2;

	if (_pathNodeListIndex < 2)
		return;

	_newPathNodeListIndex = -1;
	addNewPathNodeListPoint(_pathNodeList[0]);

	for (i = 1, node = _pathNodeList + 1; i < _pathNodeListIndex; i++, node++) {
		addNewPathNodeListPoint(*node);

		for (j = 5; j > 0; j--) {
			start = node->link - j;
			end = node->link + j;

			if (start < 0 || end > _pathListIndex) {
				continue;
			}

			point1 = _pathList[start];
			point2 = _pathList[end];
			if ((point1.x == PATH_NODE_EMPTY) || (point2.x == PATH_NODE_EMPTY)) {
				continue;
			}

			if (scanPathLine(point1, point2)) {
				for (l = 1; l <= _newPathNodeListIndex; l++) {
					if (start <= _newPathNodeList[l].link) {
						_newPathNodeListIndex = l;
						_newPathNodeList[_newPathNodeListIndex].point = point1;
						_newPathNodeList[_newPathNodeListIndex].link = start;
						incrementNewPathNodeListIndex();
						break;
					}
				}
				_newPathNodeList[_newPathNodeListIndex].point = point2;
				_newPathNodeList[_newPathNodeListIndex].link = end;

				for (k = start + 1; k < end; k++) {
					_pathList[k].x = PATH_NODE_EMPTY;
				}
				break;
			}
		}
	}

	addNewPathNodeListPoint(_pathNodeList[_pathNodeListIndex]);

	for (i = 0, j = 0; i <= _newPathNodeListIndex; i++) {
		if (_newPathNodeListIndex == i || (_newPathNodeList[i].point != _newPathNodeList[i+1].point)) {
			_pathNodeList[j++] = _newPathNodeList[i];
		}
	}
	_pathNodeListIndex = j - 1;
}

void Actor::drawPathTest() {
#ifdef ACTOR_DEBUG
	int i;
	Surface *surface;
	surface = _vm->_gfx->getBackBuffer();
	if (_debugPoints == NULL) {
		return;
	}

	for (i = 0; i < _debugPointsCount; i++) {
		*((byte *)surface->pixels + (_debugPoints[i].point.y * surface->pitch) + _debugPoints[i].point.x) = _debugPoints[i].color;
	}
#endif
}

// Console wrappers - must be safe to run

void Actor::cmdActorWalkTo(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	Location location;
	Point movePoint;

	movePoint.x = atoi(argv[2]);
	movePoint.y = atoi(argv[3]);

	location.fromScreenPoint(movePoint);

	if (!validActorId(actorId)) {
		_vm->_console->DebugPrintf("Actor::cmActorWalkTo Invalid actorId 0x%X.\n", actorId);
		return;
	}

	actorWalkTo(actorId, location);
}

} // End of namespace Saga
