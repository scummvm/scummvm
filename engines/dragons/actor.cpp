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
#include <common/debug.h>
#include "dragons.h"
#include "dragonini.h"
#include "actorresource.h"
#include "actor.h"
#include "scene.h"

namespace Dragons {

ActorManager::ActorManager(ActorResourceLoader *actorResourceLoader) : _actorResourceLoader(actorResourceLoader) {
	for (uint16 i = 0; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
		_actors.push_back(Actor(i));
	}
}


Actor *ActorManager::loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y, uint16 priorityLayer) {
	Actor *actor = loadActor(resourceId, sequenceId, x, y);
	if(actor) {
		actor->priorityLayer = priorityLayer;
	}
	return actor;
}

Actor *ActorManager::loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y) {
	debug("Load actor: resourceId: %d, SequenceId: %d, position: (%d,%d)", resourceId, sequenceId, x, y);
	ActorResource *resource = _actorResourceLoader->load(resourceId);
	//Actor *actor = new Actor(_actorResourceLoader->load(resourceId), x, y, sequenceId);
	Actor *actor = findFreeActor((int16)resourceId);
	if (actor) {
		actor->init(resource, x, y, sequenceId);
	} else {
		//TODO run find by resource and remove from mem logic here. @0x800358c8
		debug("Unable to find free actor slot!!");
	}
	return actor;
}

Actor *ActorManager::findFreeActor(int16 resourceId) {
	int i = 0;
	for (ActorsIterator it = _actors.begin(); it != _actors.end() && i < 23; ++it, i++) {
		Actor *actor = it;
		if (!(actor->flags & Dragons::ACTOR_FLAG_40)) {
			actor->resourceID = resourceId;
			actor->field_7c = 0x100000;
			return actor;
		}
	}
	return NULL;
}

Actor *ActorManager::getActor(uint16 actorId) {
	assert(actorId < DRAGONS_ENGINE_NUM_ACTORS);
	return &_actors[actorId];
}

void ActorManager::clearActorFlags(uint16 startingActorId) {
	assert(startingActorId < DRAGONS_ENGINE_NUM_ACTORS);
	for(uint16 i = startingActorId; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
		_actors[i].flags = 0;
	}
}

Actor *ActorManager::loadActor(uint32 resourceId, uint16 actorId) {
	Actor *actor = getActor(actorId);
	actor->_actorResource = _actorResourceLoader->load(resourceId);
	return actor;
}

Actor::Actor(uint16 id) : _actorID(id) {
	_actorResource = NULL;
	resourceID = -1;
	_seqCodeIp = 0;
	frame_pointer_maybe = NULL;
	priorityLayer = 3;
	x_pos = 160;
	y_pos = 110;
	target_x_pos = 0;
	target_y_pos = 0;
	field_7c = 0;
	flags = 0;
	frame_width = 0;
	frame_height = 0;
	frame_flags = 0;
	clut = 0;
	frame = NULL;
	surface = NULL;
}

void Actor::init(ActorResource *resource, int16 x, int16 y, uint32 sequenceID) {
	debug(3, "actor %d Init", _actorID);
	_actorResource = resource;
	x_pos = x;
	y_pos = y;
	sequenceTimer = 0;
	target_x_pos = x;
	target_y_pos = y;
	field_e = 0x100;
	_sequenceID2 = 0;
	flags = (Dragons::ACTOR_FLAG_40 | Dragons::ACTOR_FLAG_4);
	frame_width = 0;
	frame_height = 0;
	frame_flags = 4;
	//TODO sub_80017010();

	updateSequence((uint16)sequenceID);
}

void Actor::updateSequence(uint16 newSequenceID) {
	_sequenceID = newSequenceID;
	flags &= 0xfbf1;
	flags |= Dragons::ACTOR_FLAG_1;
}

void Actor::resetSequenceIP() {
	_seqCodeIp = _actorResource->getSequenceData(_sequenceID);
}

void Actor::loadFrame(uint16 frameOffset) {
	if (frame) {
		delete frame;
	}
	if (surface) {
		delete surface;
	}

	frame = _actorResource->loadFrameHeader(frameOffset);
	uint16 paletteId = 0;
	if (flags & Dragons::ACTOR_FLAG_4000) {
		paletteId = 0xf7;
	} else if (flags & Dragons::ACTOR_FLAG_8000) {
		paletteId = 0xf1;
	} else {
		paletteId = 0;
	}

	surface = _actorResource->loadFrame(*frame, NULL); // TODO paletteId == 0xf1 ? getEngine()->getBackgroundPalette() : NULL);

	debug(3, "ActorId: %d load frame header: (%d,%d) palette: %X", _actorID, frame->width, frame->height, paletteId);

	flags |= Dragons::ACTOR_FLAG_8; //TODO check if this is the right spot. engine sets it at 0x800185b0

}

byte *Actor::getSeqIpAtOffset(uint32 offset) {
	return _actorResource->getSequenceDataAtOffset(offset);
}

void Actor::reset_maybe() {
	flags = 0;
	//TODO actor_find_by_resourceId_and_remove_resource_from_mem_maybe(resourceID);
}

static const int32 pathfinderXYOffsetTbl[32] =
		{
				0x00000100,
				0x000000fb,
				0x000000ec,
				0x000000d4,

				0x000000b5,
				0x0000008e,
				0x00000061,
				0x00000031,

				-0x00000000,
				-0x31,
				-0x61,
				-0x8e,

				-0xb5,
				-0xd4,
				-0xec,
				-0xfb,

				-0xff,
				-0xfb,
				-0xec,
				-0xd4,

				-0xb5,
				-0x8e,
				-0x61,
				-0x31,

				0x00000000,
				0x00000031,
				0x00000061,
				0x0000008e,

				0x000000b5,
				0x000000d4,
				0x000000ec,
				0x000000fb
		};


bool Actor::pathfinding_maybe(int16 target_x, int16 target_y, int16 unkTypeMaybe) {
	uint8 pathfinderData[32];
	debug(1, "pathfinding. (%X,%X) -> (%X,%X)", x_pos, y_pos, target_x, target_y);
	int16 priority = 0;
	int16 var_90_1 = 0;
	int16 var88 = 0;

	bool isFlag0x10Set = flags & Dragons::ACTOR_FLAG_10;
	flags &= ~Dragons::ENGINE_FLAG_10;

	if (x_pos == target_x && y_pos == target_y) {
		if (isFlag0x10Set) {
			pathfindingCleanup();
		}
		return true;
	}

	if (unkTypeMaybe < 2) {
		priority = getEngine()->_scene->getPriorityAtPosition(Common::Point(target_x_pos, target_y_pos));
		if (priority < 0) {
			priority = 1;
		}
	}
	if ((unkTypeMaybe != 0 || priority - 1 < 8) && (unkTypeMaybe != 1 || priority - 1 < 16)) {
		var_90_1 = (unkTypeMaybe ^ 2) < 1 ? 1 : 0;

		int32 x_related_idx=1;
		for(; x_related_idx < 320; x_related_idx++) {

			int32 v0_18 = 0;
			for (int32 s3_1 = 0;s3_1 < 0x20; s3_1++) {
				int32 v0_19 = s3_1 + 8;
				if (v0_19 <  0) {
					v0_19 = s3_1 - 8;
				}
				int16 y_offset = (x_related_idx * pathfinderXYOffsetTbl[v0_19 & 0x1f]) / 16;
				int16 x_offset = (x_related_idx * pathfinderXYOffsetTbl[s3_1 & 0x1f]) / 16;
				if (target_x + x_offset >= 0 &&
					target_y + y_offset >= 0) {
					priority = getEngine()->_scene->getPriorityAtPosition(Common::Point(target_x + x_offset, target_y + y_offset));

					if ((unkTypeMaybe == 0 && priority - 1 < 8) || (unkTypeMaybe == 1 && priority -1 < 0x10)) {
						target_x += x_offset;
						target_y += y_offset;
						x_related_idx = -1;
						break;
					}
				}
			}
			if (x_related_idx == -1) {
				break;
			}
		}

		if (x_related_idx == 320) {
			if (isFlag0x10Set) {
				pathfindingCleanup();
			}
			return false;
		}
	} else {
		var_90_1 = 1;
	}

	if (x_pos == target_x && y_pos == target_y) {
		if (isFlag0x10Set) {
			pathfindingCleanup();
		}
		return true;
	}

	int16 numWalkPoints = 0;

	int16 newTargetX = target_x;
	int16 newTargetY = target_y;
	int16 newX = x_pos;
	int16 newY = y_pos;

	memset(pathfinderData, 0, 32);

	field_76 = target_x;
	field_78 = target_y;

	if(!pathfindingUnk(x_pos, y_pos, target_x, target_y, unkTypeMaybe)) {
		//  0x8003398c
		int16 xOffset = -1;
		//TODO convert to for loops
		do {
			int16 yOffset = -1;
			do {
				int16 targetXOffset = -1;
				do {
					int16 targetYOffset = -1;
					do {
						if(pathfindingUnk(newX + xOffset, newY + yOffset, newTargetX + targetXOffset, newTargetY + targetYOffset, unkTypeMaybe | 0x8000)) {
							targetXOffset = 2;
							newX += xOffset;
							newY += yOffset;
							newTargetX += targetXOffset;
							newTargetY += targetYOffset;
							var_90_1 = 0;
							var88 = 1;
							x_pos += xOffset;
							y_pos += yOffset;
							xOffset = 2;
							yOffset = 2;
							targetYOffset = 3;
						} else {
							targetYOffset++;
						}
					} while(targetYOffset < 2);
					targetXOffset++;
				} while(targetXOffset < 2);
				yOffset++;
			} while(yOffset < 2);
			xOffset++;
		} while(xOffset < 2);
	}

	if (var88 == 0) { //0x80033af0
		int16 i;
		for (i = 0; i < 0x20; i++) {
			Common::Point point = getEngine()->_scene->getPoint(i);
			if (point.x != -1) {
				if (pathfindingUnk(x_pos, x_pos, point.x, point.y, unkTypeMaybe)) {
					break;
				}
			}
		}

		if (i == 0x20) {
			// 0x80033b80
			int16 tempX = newX;
			int16 tempY = newY;
			for(int j = 0; j < 0x20; j++) {
				Common::Point point = getEngine()->_scene->getPoint(i);
				if (point.x == -1) {
					continue;
				}
				if (pathfindingUnk(newX + 1, tempY, point.x, point.y, unkTypeMaybe)) {
					newX++;
					x_pos++;
					break;
				} else if (pathfindingUnk(newX - 1, tempY, point.x, point.y, unkTypeMaybe)) {
					newX--;
					x_pos--;
					break;
				} else if (pathfindingUnk(tempX, newY + 1, point.x, point.y, unkTypeMaybe)) {
					newY++;
					y_pos++;
					break;
				} else if (pathfindingUnk(tempX, newY - 1, point.x, point.y, unkTypeMaybe)) {
					newY--;
					y_pos--;
					break;
				} else if (pathfindingUnk(newX + 1, newY + 1, point.x, point.y, unkTypeMaybe)) {
					newX++;
					x_pos++;
					newY++;
					y_pos++;
					break;
				} else if (pathfindingUnk(newX - 1, newY + 1, point.x, point.y, unkTypeMaybe)) {
					newX--;
					x_pos--;
					newY++;
					y_pos++;
					break;
				} else if (pathfindingUnk(newX + 1, newY - 1, point.x, point.y, unkTypeMaybe)) {
					newX++;
					x_pos++;
					newY--;
					y_pos--;
					break;
				} else if (pathfindingUnk(newX - 1, newY - 1, point.x, point.y, unkTypeMaybe)) {
					newX--;
					x_pos--;
					newY--;
					y_pos--;
					break;
				}
			}
		}

		if (var88 == 0) {
			// 0x80033e48
			for (i = 0; i < 0x20; i++) {
				Common::Point point = getEngine()->_scene->getPoint(i);
				if (point.x != -1) {
					if (pathfindingUnk(x_pos, x_pos, point.x, point.y, unkTypeMaybe)) {
						break;
					}
				}
			}

			if (i == 0x20) {
				// 0x80033ed0
				for(int j = 0; j < 0x20; j++) {
					Common::Point point = getEngine()->_scene->getPoint(i);
					if (point.x == -1) {
						continue;
					}
					if (pathfindingUnk(target_x + 1, target_y, point.x, point.y, unkTypeMaybe)) {
						target_x++;
						break;
					} else if (pathfindingUnk(target_x - 1, target_y, point.x, point.y, unkTypeMaybe)) {
						target_x--;
						break;
					} else if (pathfindingUnk(target_x, target_y + 1, point.x, point.y, unkTypeMaybe)) {
						target_y++;
						break;
					} else if (pathfindingUnk(target_x, target_y - 1, point.x, point.y, unkTypeMaybe)) {
						target_y--;
						break;
					} else if (pathfindingUnk(target_x + 1, target_y + 1, point.x, point.y, unkTypeMaybe)) {
						target_x++;
						target_y++;
						break;
					} else if (pathfindingUnk(target_x - 1, target_y + 1, point.x, point.y, unkTypeMaybe)) {
						target_x--;
						target_y++;
						break;
					} else if (pathfindingUnk(target_x + 1, target_y - 1, point.x, point.y, unkTypeMaybe)) {
						target_x++;
						target_y--;
						break;
					} else if (pathfindingUnk(target_x - 1, target_y - 1, point.x, point.y, unkTypeMaybe)) {
						target_x--;
						target_y--;
						break;
					}
				}
			}
		}
	}

	for (; !pathfindingUnk(newX, newY, newTargetX, newTargetY, unkTypeMaybe); ) {
		int16 pointId = pathfindingFindClosestPoint(newX, newY, newTargetX, newTargetX, unkTypeMaybe, pathfinderData);
		if (pointId == -1) {
			if (isFlag0x10Set) {
				pathfindingCleanup();
			}
			return false;
		}
		pathfinderData[pointId] = 1;
		Common::Point point = getEngine()->_scene->getPoint(pointId);
		if (numWalkPoints < 2) {
			if (numWalkPoints > 0 && pathfindingUnk(point.x, point.y, newTargetX, newTargetY, unkTypeMaybe)) {
				numWalkPoints--;
			}
		} else {
			Common::Point targetPoint = getEngine()->_scene->getPoint(walkPointsTbl[numWalkPoints - 2]);
			if (pathfindingUnk(point.x, point.y, targetPoint.x, targetPoint.y, unkTypeMaybe)) {
				numWalkPoints--;
			}
		}
		walkPointsTbl[numWalkPoints] = (uint16)pointId;
		numWalkPoints++;
	}

		//0x8003437c
		int16 origDistance = abs(target_x - x_pos) * abs(target_x - x_pos) + abs(target_y - y_pos) * abs(target_y - y_pos);
		int16 newTargetDiffDistance = abs(newTargetX - target_x) * abs(newTargetX - target_x) + abs(newTargetY - target_y) * abs(newTargetY - target_y);

		if (var_90_1 == 0
		|| origDistance >= 625
		|| ((target_x != x_pos || target_y != y_pos) && origDistance >= newTargetDiffDistance)) {
			//0x80034568
			debug(1, "0x80034568");
			walkPointsIndex = numWalkPoints - 1;

			if (numWalkPoints == 0) {
				target_x_pos = newTargetX;
				target_y_pos = newTargetY;
				field_76 = -1;
				field_78 = -1;
			} else {
				uint16 pointId = walkPointsTbl[walkPointsIndex];
				Common::Point point = getEngine()->_scene->getPoint(pointId);
				target_x_pos = point.x;
				target_y_pos = point.y;
			}
			int16 newSeqId = pathfindingUpdateTarget(target_x, target_y);
			if (newSeqId != -1 && !(flags & ACTOR_FLAG_800)) {
				_sequenceID2 = newSeqId;
			}
			if (_sequenceID != _sequenceID2 + 8 && !(flags & ACTOR_FLAG_800)) {
				updateSequence(_sequenceID2 + 8);
			}
			setFlag(ACTOR_FLAG_10);
			return true;
		} else {
			//0x80034470
			debug(1, "0x80034470");
		}


	return false;
}

void Actor::pathfindingCleanup() {
	clearFlag(Dragons::ACTOR_FLAG_10);
	walkPointsIndex = 0;
	target_x_pos = x_pos;
	target_y_pos = y_pos;
	field_76 = -1;
	field_78 = -1;
	setFlag(Dragons::ACTOR_FLAG_4);

	if (flags & Dragons::ACTOR_FLAG_200) {
		clearFlag(Dragons::ACTOR_FLAG_800);
	}
}

void Actor::waitUntilFlag8IsSet() {
	if (flags & Dragons::ACTOR_FLAG_8) {
		return;
	}

	while(!(flags & Dragons::ACTOR_FLAG_8)) {
		getEngine()->waitForFrames(1);
	}
}

void Actor::waitUntilFlag8And4AreSet() {
	waitUntilFlag8IsSet();

	if (flags & Dragons::ACTOR_FLAG_4) {
		return;
	}

	while(!(flags & Dragons::ACTOR_FLAG_4)) {
		getEngine()->waitForFrames(1);
	}
}

void Actor::clearFlag(uint32 flag) {
	flags &= ~flag;
}

void Actor::setFlag(uint32 flag) {
	flags |= flag;
}

bool Actor::isFlagSet(uint32 flag) {
	return (flags & flag) == flag;
}

uint16 Actor::pathfindingUnk(int16 actor_x, int16 actor_y, int16 target_x, int16 target_y, int16 unkType) {
	debug(1, "pathfindingUnk. (%X,%X) -> (%X,%X) %d", x_pos, y_pos, target_x, target_y, unkType);

	if (unkType == 2) {
		return 1;
	}
	uint16 width = getEngine()->_scene->getStageWidth();
	uint16 height = getEngine()->_scene->getStageHeight();

	if (unkType & 0x8000
	|| actor_x < 0
	|| width - 1 < actor_x
	|| actor_y < 0
	|| height - 1 < actor_y
	|| target_x < 0
	|| width - 1 < target_x
	|| target_y < 0
	|| height - 1 < target_y) {
		return 0;
	}

	int32 x_increment = 0;
	int32 y_increment = 0;
	if (target_y == actor_y && target_x == target_y) {
		return 1;
	}

	int16 diffX = target_x - actor_x;
	int16 diffY = target_y - actor_y;

	if (target_y != actor_y && target_x == actor_x) {
		y_increment = diffY > 0 ? 1 : -1;
	} else {
		if (target_y == actor_y) {
			if (target_x == actor_x) {
				x_increment = 0;
				y_increment = diffY > 0 ? 1 : -1;
			} else {
				x_increment = diffX > 0 ? 1 : -1;
				y_increment = 0;
			}
		} else {
			if (ABS(diffY) < ABS(diffX)) {
				x_increment = diffX > 0 ? 1 : -1;
				y_increment = ((diffY) /*<< 0x10*/) / (diffX);
				if ((diffY > 0 && y_increment < 0) || (diffY < 0 && y_increment > 0)) {
					y_increment = -y_increment;
				}
			} else {
				y_increment = diffY > 0 ? 1 : -1;
				x_increment = ((diffX) /*<< 0x10*/) / (diffY);
				if ((diffX > 0 && x_increment < 0) || (diffX < 0 && x_increment > 0)) {
					x_increment = -x_increment;
				}
			}
		}
	}

	// 0x80034d28
	int32 x = actor_x << 0x10;
	int32 y = actor_y << 0x10;
	for(;;) {
		if ((x+0x8000) >> 0x10 == target_x && (y+0x8000) >> 0x10 == target_y) {
			return 1;
		}
		int16 priority = getEngine()->_scene->getPriorityAtPosition(Common::Point(x>>0x10, y>>0x10));
		if ( priority < 0) {
			priority = 1;
		}
		if (!(unkType & 0x7fff) && (priority == 0 || priority >= 9)) {
			return 0;
		}

		if ((unkType & 0x7fff) == 1) {
			if (priority == 0 || priority >= 0x11) {
				return 0;
			}
		}
		x += x_increment;
		y += y_increment;
	}
}

	int16 Actor::pathfindingUpdateTarget(int16 newTargetX, int16 newTargetY) {
		field_24_x = x_pos << 0x10;
		field_28_y = y_pos << 0x10;

		int16 diffX = newTargetX - x_pos;
		int16 diffY = newTargetY - y_pos;
		int16 absDiffX = abs(diffX);
		int16 absDiffY = abs(diffY) * 2;

		int16 t2 = 0;
		int16 newSequenceId = -1;

		if (diffX == 0) {
			if (diffY == 0) {
				return -1;
			}
			field_2c = 0;
			field_30 = 0x10000;
		} else {
			if (diffY == 0) {
				field_2c = 0x10000;
				field_30 = 0;
			} else {
				if (absDiffX >= absDiffY) {
					field_2c = 0x10000;
					field_30 = (absDiffY << 0x10) / absDiffX;
				} else {
					field_2c = (absDiffX << 0x10) / absDiffY;
					field_30 = 0x10000;
				}
			}
		}
		field_30 = ((field_30 >> 5) * field_7c) >> 0xb;
		field_2c = ((field_2c >> 5) * field_7c) >> 0xb;

		if (diffX < 0) {
			field_2c = -field_2c;
			t2 = 2;
		}

		if (diffY < 0) {
			field_30 = -field_30;
			t2++;
		}

		switch (t2) {
			case 0 :
				newSequenceId = absDiffX < (absDiffY * 2) ? 2 : 0;
				break;
			case 1 :
				error("TODO t2 == 1 0x80035138");
			case 2 :
				newSequenceId = absDiffX < (absDiffY * 2) ? 2 : 4;
				break;
			case 3 :
				newSequenceId = absDiffX < (absDiffY * 2) ? 6 : 4;
				break;
			default :
				break;
		}

		field_30 = field_30 / 2;

		if (getEngine()->_dragonINIResource->isFlicker(_actorID)) {
			// TODO 0x8003523c
			error("FIXME 0x8003523c");
		}

		target_x_pos = newTargetX;
		target_y_pos = newTargetY;

		return newSequenceId;
	}

	void Actor::walkPath() {
		if (isFlagClear(Dragons::ACTOR_FLAG_400) && isFlagSet(Dragons::ACTOR_FLAG_40) && isFlagSet(Dragons::ACTOR_FLAG_10)) {
			field_24_x += (((field_e * field_2c) / 256) * 5) / 4;
			field_28_y += (((field_e * field_30) / 256) * 5) / 4;

			if ( (field_2c >= 0 && target_x_pos < (field_24_x >> 0x10))
			|| (field_2c < 0 && (field_24_x >> 0x10) < target_x_pos)) {
				field_24_x = target_x_pos << 0x10;
			}

			if ( (field_30 >= 0 && target_y_pos < (field_28_y >> 0x10))
				 || (field_30 < 0 && (field_28_y >> 0x10) < target_y_pos)) {
				field_28_y = target_y_pos << 0x10;
			}

			x_pos = field_24_x >> 0x10;
			y_pos = field_28_y >> 0x10;

			if (x_pos == target_x_pos && y_pos == target_y_pos) {
				if (walkPointsIndex <= 0) {
					if (field_76 < 0) {
						clearFlag(ACTOR_FLAG_10);
						if (isFlagClear(ACTOR_FLAG_200)) {
							clearFlag(ACTOR_FLAG_800);
						}
						setFlag(ACTOR_FLAG_4);
						clearFlag(ACTOR_FLAG_1);
						return;
					} else {
						target_x_pos = field_76;
						target_y_pos = field_78;
						field_76 = -1;
						field_78 = -1;
					}
				} else {
					walkPointsIndex--;
					Common::Point point = getEngine()->_scene->getPoint(walkPointsTbl[walkPointsIndex]);
					target_x_pos = point.x;
					target_y_pos = point.y;
				}
				// 0x8001bcc8
				if(pathfindingUpdateTarget(target_x_pos, target_y_pos) == -1) {
					_sequenceID2 = -1;
				} else {
					if (_sequenceID != _sequenceID2 + 8 && !(flags & ACTOR_FLAG_800)) {
						updateSequence(_sequenceID2 + 8);
					}
				}
				setFlag(ACTOR_FLAG_10);
			}
		}
	}

	int16 Actor::pathfindingFindClosestPoint(int16 actor_x, int16 actor_y, int16 target_x, int16 target_y,
											 int16 unkType, uint8 *pointsInUseTbl) {
		int16 pointId = -1;
		uint32 minDist = 0xffffffff;

		for (int i = 0; i < 0x22; i++) {
			Common::Point point = getEngine()->_scene->getPoint(i);
			if (point.x != -1 && pointsInUseTbl[i] == 0) {
				if (pathfindingUnk(point.x, point.y, target_x, target_y, unkType)) {
					uint32 dist = abs(point.x - actor_x) * abs(point.x - actor_x) + abs(point.y - actor_y) * abs(point.y - actor_y);
					if ( dist < minDist) {
						minDist = dist;
						pointId = i;
					}
				}
			}
		}
		return pointId;
	}

} // End of namespace Dragons
