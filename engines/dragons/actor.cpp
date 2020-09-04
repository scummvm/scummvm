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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "common/debug.h"
#include "dragons/dragons.h"
#include "dragons/dragonini.h"
#include "dragons/actorresource.h"
#include "dragons/actor.h"
#include "dragons/scene.h"
#include "dragons/screen.h"

namespace Dragons {

static const int kPathPointsCount = 32;

ActorManager::ActorManager(ActorResourceLoader *actorResourceLoader) : _actorResourceLoader(actorResourceLoader) {
	for (uint16 i = 0; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
		_actors.push_back(Actor(i));
	}
	resetDisplayOrder();
}


Actor *ActorManager::loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y, uint16 priorityLayer) {
	Actor *actor = loadActor(resourceId, sequenceId, x, y);
	if (actor) {
		actor->_priorityLayer = priorityLayer;
	}
	return actor;
}

Actor *ActorManager::loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y) {
	debug(1, "Load actor: resourceId: %d, SequenceId: %d, position: (%d,%d)", resourceId, sequenceId, x, y);
	ActorResource *resource = _actorResourceLoader->load(resourceId);
	//Actor *actor = new Actor(_actorResourceLoader->load(resourceId), x, y, sequenceId);
	Actor *actor = findFreeActor((int16)resourceId);
	if (actor) {
		actor->init(resource, x, y, sequenceId);
	} else {
		//TODO run find by resource and remove from mem logic here. @0x800358c8
		debug("Unable to find free actor slot!!");
		delete resource;
	}
	resetDisplayOrder();
	return actor;
}

Actor *ActorManager::findFreeActor(int16 resourceId) {
	int i = 0;
	for (ActorsIterator it = _actors.begin(); it != _actors.end() && i < 23; ++it, i++) {
		Actor *actor = it;
		if (!(actor->_flags & ACTOR_FLAG_40)) {
			actor->_resourceID = resourceId;
			actor->_walkSpeed = 0x100000;
			return actor;
		}
	}
	return nullptr;
}

Actor *ActorManager::getActor(uint16 actorId) {
	assert(actorId < DRAGONS_ENGINE_NUM_ACTORS);
	return &_actors[actorId];
}

void ActorManager::clearActorFlags(uint16 startingActorId) {
	assert(startingActorId < DRAGONS_ENGINE_NUM_ACTORS);
	for (uint16 i = startingActorId; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
		_actors[i]._flags = 0;
	}
}

Actor *ActorManager::loadActor(uint32 resourceId, uint16 actorId) { //TODO should we rename this. loadActorResource or updateActorResource
	Actor *actor = getActor(actorId);
	actor->_actorResource = _actorResourceLoader->load(resourceId);
	return actor;
}

ActorResource *ActorManager::getActorResource(uint32 resourceId) {
	return _actorResourceLoader->load(resourceId);
}

void ActorManager::updateActorDisplayOrder() {
	bool shouldContinue = true;

	while (shouldContinue) {
		shouldContinue = false;
		for (int i = 0; i < DRAGONS_ENGINE_NUM_ACTORS - 1; i++) {
			Actor *curActor = getActor(_displayOrder[i]);
			Actor *nextActor = getActor(_displayOrder[i + 1]);
			int16 curY = curActor->_y_pos > 0 ? curActor->_y_pos : 0;
			int16 nextY = nextActor->_y_pos > 0 ? nextActor->_y_pos : 0;
			if (nextActor->_priorityLayer * 0x1000000 + nextY * 0x100 + nextActor->_actorID <
					curActor->_priorityLayer * 0x1000000 + curY * 0x100 + curActor->_actorID) {
				_displayOrder[i] = nextActor->_actorID;
				_displayOrder[i + 1] = curActor->_actorID;
				shouldContinue = true;
			}
		}
	}
}

void ActorManager::resetDisplayOrder() {
	for (uint16 i = 0; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
		Actor *actor = getActor(i);
		_displayOrder[i] = i;
		if (!actor->isFlagSet(ACTOR_FLAG_40)) {
			actor->_priorityLayer = 0;
		}
	}
}

Actor *ActorManager::getActorByDisplayOrder(uint16 position) {
	return getActor(_displayOrder[position]);
}

Actor::Actor(uint16 id) : _actorID(id) {
	_actorResource = nullptr;
	_resourceID = -1;
	_seqCodeIp = 0;
	_priorityLayer = 3;
	_x_pos = 160;
	_y_pos = 110;
	_walkDestX = 0;
	_walkDestY = 0;
	_walkSpeed = 0;
	_flags = 0;
	_frame_flags = 0;
	_frame = nullptr;
	_surface = nullptr;

	_actorFileDictionaryIndex = 0;
	_sequenceTimerMaxValue = 0;
	_scale = 0x100;
	_sequenceTimer = 0;
	_sequenceID = 0;
	_direction = 0;
	_xShl16 = 0;
	_yShl16 = 0;
	_walkSlopeX = 0;
	_walkSlopeY = 0;
	_walkPointsIndex = 0;
	_finalWalkDestX = 0;
	_finalWalkDestY = 0;
	_field_7a = 0;
}

void Actor::init(ActorResource *resource, int16 x, int16 y, uint32 sequenceID) {
	debug(3, "actor %d Init", _actorID);
	delete _actorResource;
	_actorResource = resource;
	_x_pos = x;
	_y_pos = y;
	_sequenceTimer = 0;
	_walkDestX = x;
	_walkDestY = y;
	_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	_direction = 0;
	_flags = (ACTOR_FLAG_40 | Dragons::ACTOR_FLAG_4);
	_frame_flags = 4;
	//TODO sub_80017010();
	freeFrame();

	updateSequence((uint16)sequenceID);
}

void Actor::updateSequence(uint16 newSequenceID) {
	_sequenceID = newSequenceID;
	_flags &= 0xfbf1;
	_flags |= ACTOR_FLAG_1;
}

void Actor::resetSequenceIP() {
	_seqCodeIp = _actorResource->getSequenceData(_sequenceID);
}

void Actor::loadFrame(uint16 frameOffset) {
	freeFrame();

	_frame = _actorResource->loadFrameHeader(frameOffset);

	if (_frame->flags & 0x800) {
		_frame_flags |= ACTOR_FRAME_FLAG_2;
	} else {
		_frame_flags &= ~ACTOR_FRAME_FLAG_2;
	}

	_surface = _actorResource->loadFrame(*_frame, nullptr); // TODO paletteId == 0xf1 ? getEngine()->getBackgroundPalette() : nullptr);

	debug(5, "ActorId: %d load frame header: (%d,%d)", _actorID, _frame->width, _frame->height);

	_flags |= ACTOR_FLAG_8; //TODO check if this is the right spot. engine sets it at 0x800185b0

}

void Actor::freeFrame() {
	delete _frame;
	delete _surface;
	_frame = nullptr;
	_surface = nullptr;
}

byte *Actor::getSeqIpAtOffset(uint32 offset) {
	return _actorResource->getSequenceDataAtOffset(offset);
}

void Actor::reset_maybe() {
	_flags = 0;
	//TODO actor_find_by_resourceId_and_remove_resource_from_mem_maybe(resourceID);
	freeFrame();
	delete _actorResource;
	_actorResource = nullptr;
}

uint32 calcDistance(int32 x1, int32 y1, int32 x2, int32 y2) {
	return ABS(x2 - x1) * ABS(x2 - x1) + ABS(y2 - y1) * ABS(y2 - y1);
}

bool Actor::startWalk(int16 destX, int16 destY, uint16 flags) {
	static const int kCosTbl[40] = {
		// cos table
		256, 251, 236, 212, 181, 142, 97, 49,
		0, -49, -97, -142, -181, -212, -236, -251,
		-255, -251, -236, -212, -181, -142, -97, -49,
		0, 49, 97, 142, 181, 212, 236, 251,
		11, 0, 0, 0, 0, 0, 0, 0
	};
	static const int kAdjustXTbl[8] = {
		1, -1, 0, 0, 1, -1, 1, -1
	};
	static const int kAdjustYTbl[8] = {
		0, 0, 1, -1, 1, 1, -1, -1
	};

	debug(1, "startWalk(%d, %d, %d)", _actorID, destX, destY);
	bool wasAlreadyWalking = isFlagSet(ACTOR_FLAG_10);

	clearFlag(ACTOR_FLAG_10);

	// Check if the actor already is at the destination
	if (_x_pos == destX && _y_pos == destY) {
		if (wasAlreadyWalking) {
			stopWalk();
		}
		return true;
	}

	int xorflagsl = 0;
	int flag4 = 0;
	int origDestX = 0, origDestY = 0;

	int destPriority = 0;
	if (flags < 2) {
		destPriority = getEngine()->_scene->getPriorityAtPosition(Common::Point(destX, destY));
		if (destPriority < 0)
			destPriority = 1;
	}

	if ((flags == 0 && destPriority - 1 >= 8) || (flags == 1 && destPriority - 1 >= 16)) {
		// Destination point is not walkable so it has to be adjusted
		// Try to find a walkable destination point by testing the 32 corner points of a circle
		// in increasing radius steps.
		bool foundDestPos = false;
		xorflagsl = 1;
		origDestX = destX;
		origDestY = destY;
		for (int testRadius = 1; testRadius < 320 && !foundDestPos; ++testRadius) {
			for (int octant = 0; octant < 32; ++octant) {
				int testDestX = destX + ((testRadius * kCosTbl[octant % 32]) >> 8);
				int testDestY = destY + ((testRadius * kCosTbl[(octant + 8) % 32]) >> 8);
				if (testDestX >= 0 && testDestY >= 0) {
					int testDestPriority = getEngine()->_scene->getPriorityAtPosition(Common::Point(testDestX, testDestY));
					if ((flags == 0 && testDestPriority - 1 < 8) || (flags == 1 && testDestPriority - 1 < 16)) {
						destX = testDestX;
						destY = testDestY;
						foundDestPos = true;
						break;
					}
				}
			}
		}
		if (!foundDestPos) {
			if (wasAlreadyWalking) {
				stopWalk();
			}
			return false;
		}
	} else {
		// TODO Clean up
		xorflagsl = (flags ^ 2) < 1 ? 1 : 0;
	}

	// Check if the actor already is at the adjusted destination
	if (_x_pos == destX && _y_pos == destY) {
		if (wasAlreadyWalking) {
			stopWalk();
		}
		return true;
	}

	int tempDestX1 = destX, tempDestY1 = destY;
	int actorX1 = _x_pos, actorY1 = _y_pos;
	bool pathPointProcessed[kPathPointsCount];

	for (int pointIndex = 0; pointIndex < kPathPointsCount; ++pointIndex) {
		pathPointProcessed[pointIndex] = false;
	}

	_finalWalkDestX = destX;
	_finalWalkDestY = destY;

	if (!canWalkLine(actorX1, actorY1, tempDestX1, tempDestY1, flags)) {
		// Adjust source/dest positions
		for (int sxd = -1; sxd <= 1; ++sxd) {
			for (int syd = -1; syd <= 1; ++syd) {
				for (int dxd = -1; dxd <= 1; ++dxd) {
					for (int dyd = -1; dyd <= 1; ++dyd) {
						if (canWalkLine(actorX1 + sxd, actorY1 + syd, tempDestX1 + dxd, tempDestY1 + dyd, flags | 0x8000)) {
							sxd = 2;
							syd = 2;
							dxd = 2;
							dyd = 2;
							actorX1 += sxd;
							actorY1 += syd;
							tempDestX1 += dxd;
							tempDestY1 += dyd;
							_x_pos += sxd;
							_y_pos += syd;
						}
					}
				}
			}
		}
	}

	if (flag4 == 0) {
		// More adjusting of the source position
		bool needAdjustSourcePoint = true;
		for (int pointIndex = 0; pointIndex < kPathPointsCount; ++pointIndex) {
			const Common::Point pt = getEngine()->_scene->getPoint(pointIndex);
			if (pt.x != -1 && canWalkLine(actorX1, actorY1, pt.x, pt.y, flags)) {
				needAdjustSourcePoint = false;
				break;
			}
		}
		if (needAdjustSourcePoint) {
			for (int pointIndex = 0; needAdjustSourcePoint && pointIndex < kPathPointsCount; ++pointIndex) {
				const Common::Point pt = getEngine()->_scene->getPoint(pointIndex);
				for (int deltaIndex = 0; needAdjustSourcePoint && deltaIndex < 8; ++deltaIndex) {
					const int deltaX = kAdjustXTbl[deltaIndex];
					const int deltaY = kAdjustYTbl[deltaIndex];
					if (canWalkLine(actorX1 + deltaX, actorY1 + deltaY, pt.x, pt.y, flags)) {
						actorX1 += deltaX;
						actorY1 += deltaY;
						_x_pos += deltaX;
						_y_pos += deltaY;
						needAdjustSourcePoint = false;
					}
				}
			}
		}
		// More adjusting of the destination position
		bool needAdjustDestPoint = true;
		for (int pointIndex = 0; pointIndex < kPathPointsCount; ++pointIndex) {
			const Common::Point pt = getEngine()->_scene->getPoint(pointIndex);
			if (pt.x != -1 && canWalkLine(destX, destY, pt.x, pt.y, flags)) {
				needAdjustDestPoint = false;
				break;
			}
		}
		if (needAdjustDestPoint) {
			for (int pointIndex = 0; needAdjustDestPoint && pointIndex < kPathPointsCount; ++pointIndex) {
				const Common::Point pt = getEngine()->_scene->getPoint(pointIndex);
				for (int deltaIndex = 0; needAdjustDestPoint && deltaIndex < 8; ++deltaIndex) {
					const int deltaX = kAdjustXTbl[deltaIndex];
					const int deltaY = kAdjustYTbl[deltaIndex];
					if (canWalkLine(destX + deltaX, destY + deltaY, pt.x, pt.y, flags)) {
						destX += deltaX;
						destY += deltaY;
						needAdjustDestPoint = false;
					}
				}
			}
		}
	}

	// Build the actual path. The path is constructed backwards from the destination to the source.
	int pathPointsIndex = 0;
	while (!canWalkLine(actorX1, actorY1, tempDestX1, tempDestY1, flags) && pathPointsIndex < kPathPointsCount) {
		int foundPointIndex = pathfindingFindClosestPoint(actorX1, actorY1, tempDestX1, tempDestY1, flags, pathPointProcessed);
		if (foundPointIndex < 0) {
			if (wasAlreadyWalking) {
				stopWalk();
			}
			return false;
		}
		pathPointProcessed[foundPointIndex] = true;
		const Common::Point pt = getEngine()->_scene->getPoint(foundPointIndex);
		tempDestX1 = pt.x;
		tempDestY1 = pt.y;
		if (pathPointsIndex >= 2) {
			const Common::Point prevPt = getEngine()->_scene->getPoint(_walkPointsTbl[pathPointsIndex - 2]);
			if (canWalkLine(pt.x, pt.y, prevPt.x, prevPt.y, flags)) {
				--pathPointsIndex;
			}
		} else if (pathPointsIndex == 1) {
			if (canWalkLine(pt.x, pt.y, destX, destY, flags)) {
				--pathPointsIndex;
			}
		}
		_walkPointsTbl[pathPointsIndex] = foundPointIndex;
		++pathPointsIndex;
	}

	// Direction/post-processing
	if (xorflagsl != 0) {
		uint destDistance = calcDistance(destX, destY, tempDestX1, tempDestY1);
		uint sourceDistance = calcDistance(actorX1, actorY1, destX, destY);
		if (sourceDistance < 625 && ((actorX1 == destX && actorY1 == destY) || (sourceDistance < destDistance))) {
			int newDirection;
			int dx = origDestX - actorX1;
			int dy = origDestY - actorY1;
			if (dx != 0) {
				int slope = dy / dx;
				if (slope == 0) {
					newDirection = (dx < 1) ? 4 : 0;
				} else if (slope > 0) {
					newDirection = (dx <= 0) ? 3 : 7;
				} else {
					newDirection = (dx <= 0) ? 5 : 1;
				}
			} else {
				newDirection = (dy <= 0)  ? 2 : 6;
			}
			_direction = newDirection;
			if (wasAlreadyWalking) {
				stopWalk();
			}
			return false;
		}
	}

	_walkPointsIndex = pathPointsIndex - 1;
	if (pathPointsIndex == 0) {
		_walkDestX = tempDestX1;
		_walkDestY = tempDestY1;
		_finalWalkDestX = -1;
		_finalWalkDestY = -1;
	} else {
		const Common::Point pt = getEngine()->_scene->getPoint(_walkPointsTbl[_walkPointsIndex]);
		_walkDestX = pt.x;
		_walkDestY = pt.y;
	}
	int direction = startMoveToPoint(_walkDestX, _walkDestY);
	if (direction != -1 && !isFlagSet(ACTOR_FLAG_800)) {
		_direction = direction;
	}
	if (_sequenceID != _direction + 8 && !isFlagSet(ACTOR_FLAG_800)) {
		updateSequence(_direction + 8);
	}
	setFlag(ACTOR_FLAG_10);
	return true;
}

void Actor::stopWalk() {
	clearFlag(ACTOR_FLAG_10);
	_walkPointsIndex = 0;
	_walkDestX = _x_pos;
	_walkDestY = _y_pos;
	_finalWalkDestX = -1;
	_finalWalkDestY = -1;
	setFlag(ACTOR_FLAG_4);

	if (_flags & ACTOR_FLAG_200) {
		clearFlag(ACTOR_FLAG_800);
	}
}

void Actor::waitUntilFlag4IsSet() {
	while (!isFlagSet(ACTOR_FLAG_4) && !Engine::shouldQuit()) {
		getEngine()->waitForFrames(1);
	}
}

void Actor::waitUntilFlag8IsSet() {
	if (_flags & ACTOR_FLAG_8) {
		return;
	}

	while (!(_flags & ACTOR_FLAG_8) && !Engine::shouldQuit()) {
		getEngine()->waitForFrames(1);
	}
}

void Actor::waitUntilFlag8And4AreSet() {
	waitUntilFlag8IsSet();
	waitUntilFlag4IsSet();
}

void Actor::waitUntilFlag8SetThenSet1000() {
	waitUntilFlag8IsSet();
	setFlag(ACTOR_FLAG_1000);
}

void Actor::waitUntilFlag8SetThenSet1000AndWaitFor4() {
	waitUntilFlag8SetThenSet1000();
	waitUntilFlag4IsSet();
}

void Actor::clearFlag(uint32 flag) {
	_flags &= ~flag;
}

void Actor::setFlag(uint32 flag) {
	_flags |= flag;
}

bool Actor::isFlagSet(uint32 flag) {
	return (_flags & flag) == flag;
}

uint16 Actor::canWalkLine(int16 actor_x, int16 actor_y, int16 target_x, int16 target_y, uint16 walkFlags) {
	debug(1, "canWalkLine. (%X,%X) -> (%X,%X) %d", _x_pos, _y_pos, target_x, target_y, walkFlags);

	if (walkFlags == 2) {
		return 1;
	}
	uint16 width = getEngine()->_scene->getStageWidth();
	uint16 height = getEngine()->_scene->getStageHeight();

	if (walkFlags & 0x8000) {
		if (actor_x < 0
			|| width - 1 < actor_x
			|| actor_y < 0
			|| height - 1 < actor_y
			|| target_x < 0
			|| width - 1 < target_x
			|| target_y < 0
			|| height - 1 < target_y) {
			return 0;
		}
	}

	int32 x_increment = 0;
	int32 y_increment = 0;
	if (target_y == actor_y && target_x == target_y) {
		return 1;
	}

	int16 deltaX = target_x - actor_x;
	int16 deltaY = target_y - actor_y;

	if (target_y != actor_y && target_x == actor_x) {
		y_increment = deltaY > 0 ? 0x10000 : -0x10000;
	} else {
		if (target_y == actor_y) {
			if (target_x == actor_x) {
				x_increment = 0;
				y_increment = deltaY > 0 ? 0x10000 : -0x10000;
			} else {
				x_increment = deltaX > 0 ? 0x10000 : -0x10000;
				y_increment = 0;
			}
		} else {
			if (ABS(deltaY) < ABS(deltaX)) {
				x_increment = deltaX > 0 ? 0x10000 : -0x10000;
				y_increment = ((deltaY) << 0x10) / (deltaX);
				if ((deltaY > 0 && y_increment < 0) || (deltaY < 0 && y_increment > 0)) {
					y_increment = -y_increment;
				}
			} else {
				y_increment = deltaY > 0 ? 0x10000 : -0x10000;
				x_increment = ((deltaX) << 0x10) / (deltaY);
				if ((deltaX > 0 && x_increment < 0) || (deltaX < 0 && x_increment > 0)) {
					x_increment = -x_increment;
				}
			}
		}
	}

	// 0x80034d28
	int32 x = actor_x << 0x10;
	int32 y = actor_y << 0x10;
	for (;;) {
		if ((x+0x8000) >> 0x10 == target_x && (y+0x8000) >> 0x10 == target_y) {
			return 1;
		}
		int16 priority = getEngine()->_scene->getPriorityAtPosition(Common::Point(x>>0x10, y>>0x10));
		if (priority < 0) {
			priority = 1;
		}
		if (!(walkFlags & 0x7fff) && (priority == 0 || priority >= 8)) {
			return 0;
		}

		if ((walkFlags & 0x7fff) == 1) {
			if (priority == 0 || priority >= 0x10) {
				return 0;
			}
		}
		x += x_increment;
		y += y_increment;
	}
}

int Actor::startMoveToPoint(int destX, int destY) {
	int direction = 0;
	int quadrant = 0;
	int deltaX = destX - _x_pos;
	int deltaY = (destY - _y_pos) * 2;
	int absDeltaX = ABS(deltaX);
	int absDeltaY = ABS(deltaY);
	// debug("from: (%d, %d); to: (%d, %d); d: (%d, %d); actor._walkSpeed: %08X", _x_pos, actor._y, destX, destY, deltaX, deltaY, actor._walkSpeed);

	_xShl16 = _x_pos << 16;
	_yShl16 = _y_pos << 16;

	// Walk slope is a fixed point value, where the upper 16 bits are the integral part,
	// and the lower 16 bits the fractional part. 0x10000 is 1.0.

	if (deltaX != 0 && deltaY != 0) {
		// Walk along both X and Y axis
		if (absDeltaX < absDeltaY) {
			_walkSlopeX = (absDeltaX << 16) / absDeltaY;
			_walkSlopeY = 0x10000;
		} else {
			_walkSlopeX = 0x10000;
			_walkSlopeY = (absDeltaY << 16) / absDeltaX;
		}
	} else if (deltaX != 0) {
		// Walk along X only
		_walkSlopeX = 0x10000;
		_walkSlopeY = 0;
	} else if (deltaY != 0) {
		// Walk along Y only
		_walkSlopeX = 0;
		_walkSlopeY = 0x10000;
	} else {
		// Already at dest
		return -1;
	}

	_walkSlopeX = (_walkSlopeX / 32) * (_walkSpeed / 0x800);
	_walkSlopeY = (_walkSlopeY / 32) * (_walkSpeed / 0x800);

	if (deltaX < 0) {
		_walkSlopeX = -_walkSlopeX;
		quadrant += 2;
	}

	if (deltaY < 0) {
		_walkSlopeY = -_walkSlopeY;
		quadrant += 1;
	}

	switch (quadrant) {
	case 0:
		direction = (absDeltaX < absDeltaY) ? 2 : 0;
		break;
	case 1:
		direction = (absDeltaX < absDeltaY) ? 6 : 0;
		break;
	case 2:
		direction = (absDeltaX < absDeltaY) ? 2 : 4;
		break;
	case 3:
		direction = (absDeltaX < absDeltaY) ? 6 : 4;
		break;
	default:
		break;
	}

	_walkSlopeY /= 2;
	_walkDestX = destX;
	_walkDestY = destY;

	if (getEngine()->_dragonINIResource->isFlicker(_actorID)) {
		// Adjust walk slope for the main actor
		_walkSlopeX = _walkSlopeX * 3 / 2;
		_walkSlopeY = _walkSlopeY * 3 / 2;
	}

	return direction;
}

void Actor::walkPath() {
	if (isFlagClear(ACTOR_FLAG_400) && isFlagSet(Dragons::ACTOR_FLAG_40) && isFlagSet(Dragons::ACTOR_FLAG_10)) {
		_xShl16 += (((_scale * _walkSlopeX) / DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE) * 5) / 4;
		_yShl16 += (((_scale * _walkSlopeY) / DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE) * 5) / 4;

		if ((_walkSlopeX >= 0 && _walkDestX < (_xShl16 >> 0x10))
				|| (_walkSlopeX < 0 && (_xShl16 >> 0x10) < _walkDestX)) {
			_xShl16 = _walkDestX << 0x10;
		}

		if ((_walkSlopeY >= 0 && _walkDestY < (_yShl16 >> 0x10))
				|| (_walkSlopeY < 0 && (_yShl16 >> 0x10) < _walkDestY)) {
			_yShl16 = _walkDestY << 0x10;
		}

		_x_pos = _xShl16 >> 0x10;
		_y_pos = _yShl16 >> 0x10;

		if (_x_pos == _walkDestX && _y_pos == _walkDestY) {
			if (_walkPointsIndex <= 0) {
				if (_finalWalkDestX < 0) {
					clearFlag(ACTOR_FLAG_10);
					if (isFlagClear(ACTOR_FLAG_200)) {
						clearFlag(ACTOR_FLAG_800);
					}
					setFlag(ACTOR_FLAG_4);
					clearFlag(ACTOR_FLAG_1);
					return;
				} else {
					_walkDestX = _finalWalkDestX;
					_walkDestY = _finalWalkDestY;
					_finalWalkDestX = -1;
					_finalWalkDestY = -1;
				}
			} else {
				_walkPointsIndex--;
				Common::Point point = getEngine()->_scene->getPoint(_walkPointsTbl[_walkPointsIndex]);
				_walkDestX = point.x;
				_walkDestY = point.y;
			}
			// 0x8001bcc8
			int direction = startMoveToPoint(_walkDestX, _walkDestY);
			if (direction != -1 && !isFlagSet(ACTOR_FLAG_800)) {
				_direction = direction;
			}
			if (_sequenceID != _direction + 8 && _direction != -1 && !isFlagSet(ACTOR_FLAG_800)) {
				updateSequence(_direction + 8);
			}
			setFlag(ACTOR_FLAG_10);
		}
	}
}

// 0x80034930
int16 Actor::pathfindingFindClosestPoint(int16 actor_x, int16 actor_y, int16 target_x, int16 target_y,
										 int16 unkType, bool *pointsInUseTbl) {
	int16 pointId = -1;
	uint32 minDist = 0xffffffff;

	for (int i = 0; i < kPathPointsCount; i++) {
		Common::Point point = getEngine()->_scene->getPoint(i);
		if (point.x != -1 && !pointsInUseTbl[i]) {
			if (canWalkLine(point.x, point.y, target_x, target_y, unkType)) {
				uint32 dist = abs(point.x - actor_x) * abs(point.x - actor_x) + abs(point.y - actor_y) * abs(point.y - actor_y);
				if (dist < minDist) {
					minDist = dist;
					pointId = i;
				}
			}
		}
	}
	return pointId;
}

bool Actor::actorSetSequenceAndWaitAllowSkip(uint16 newSequenceID) {
	updateSequence(newSequenceID);
	waitUntilFlag8IsSet();
	return waitUntilFlag4IsSetAllowSkip();
}

bool Actor::waitUntilFlag4IsSetAllowSkip() {
	while (!isFlagSet(ACTOR_FLAG_4) && !Engine::shouldQuit()) {
		getEngine()->waitForFrames(1);
		if (getEngine()->checkForActionButtonRelease()) {
			return true;
		}
	}

	return false;
}

byte *Actor::getPalette() {
	if (!isFlagSet(ACTOR_FLAG_4000)) {
		if (!isFlagSet(ACTOR_FLAG_8000)) {
			if ((_frame_flags & 0x30) != 0) {
				return _actorResource->getPalette();
			}
			return getEngine()->_screen->getPalette(1);
		} else {
			return getEngine()->_screen->getPalette(0);
		}
	}
	return getEngine()->_screen->getPalette(4);
}

int16 Actor::getFrameYOffset() {
	return _frame ? _frame->yOffset : 0;
}

void Actor::waitForWalkToFinish() {
	DragonsEngine *vm = getEngine();
	do {
		vm->waitForFrames(1);
	} while (!Engine::shouldQuit() && isFlagSet(ACTOR_FLAG_10));
}

} // End of namespace Dragons
