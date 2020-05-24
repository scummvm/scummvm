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
#ifndef DRAGONS_ACTOR_H
#define DRAGONS_ACTOR_H

#include "common/system.h"

namespace Dragons {
class Actor;
class ActorResourceLoader;
class ActorResource;
struct ActorFrame;

#define DRAGONS_ENGINE_NUM_ACTORS 64

enum ActorFlags {
	ACTOR_FLAG_1 = 1,
	ACTOR_FLAG_2 = 2,
	ACTOR_FLAG_4 = 4,
	ACTOR_FLAG_8 = 8,
	ACTOR_FLAG_10 = 0x10, //actor is walking a path
	ACTOR_FLAG_20 = 0x20,
	ACTOR_FLAG_40 = 0x40,
	ACTOR_FLAG_80 = 0x80,
	ACTOR_FLAG_100 = 0x100,
	ACTOR_FLAG_200 = 0x200,  // Use screen coordinates not map coordinates.
	ACTOR_FLAG_400 = 0x400,  // Actor is hidden
	ACTOR_FLAG_800 = 0x800,
	ACTOR_FLAG_1000 = 0x1000,
	ACTOR_FLAG_2000 = 0x2000,
	ACTOR_FLAG_4000 = 0x4000,
	ACTOR_FLAG_8000 = 0x8000  //Seems turn off semi trans mode when selected.
};

enum ActorFrameFlags {
	ACTOR_FRAME_FLAG_2 = 0x2,
	ACTOR_FRAME_FLAG_10 = 0x10,
	ACTOR_FRAME_FLAG_20 = 0x20
};

class ActorManager {
public:
	typedef Common::Array<Actor> Actors;
	typedef Actors::iterator ActorsIterator;

private:
	ActorResourceLoader *_actorResourceLoader;
	Actors _actors;
	uint16 _displayOrder[DRAGONS_ENGINE_NUM_ACTORS];
public:
	ActorManager(ActorResourceLoader *actorResourceLoader);

public:
	Actor *loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y);
	Actor *loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y, uint16 priorityLayer);
	Actor *loadActor(uint32 resourceId, uint16 actorId);
	Actor *getActor(uint16 actorId);
	Actor *getActorByDisplayOrder(uint16 position);
	void clearActorFlags(uint16 startingActorId);
	ActorResource *getActorResource(uint32 resourceId);
	void updateActorDisplayOrder();
private:
	Actor *findFreeActor(int16 resourceID);
	void resetDisplayOrder();
};

class Actor {
public:
	uint16 _actorID;
	ActorResource* _actorResource;
	uint16 _actorFileDictionaryIndex;
	int16 _resourceID;
	byte *_seqCodeIp;
	ActorFrame *_frame;
	Graphics::Surface *_surface;
	uint16 _sequenceTimerMaxValue;
	int16 _scale; // scale factor 0x100 is 100%
	uint16 _sequenceTimer;
	uint16 _sequenceID;
	int16 _direction;
	int16 _priorityLayer;
	uint16 _flags;
	int16 _x_pos;
	int16 _y_pos;
	int16 _walkDestX;
	int16 _walkDestY;
	int32 _xShl16;
	int32 _yShl16;
	int32 _walkSlopeX;
	int32 _walkSlopeY;
	uint16 _walkPointsTbl[32];
	int16 _walkPointsIndex;
	int16 _finalWalkDestX;
	int16 _finalWalkDestY;
	uint16 _field_7a;
	int32 _walkSpeed;
	uint16 _frame_flags;
public:

	Actor(uint16 id);
	void init(ActorResource *resource, int16 x, int16 y, uint32 sequenceID);
	void updateSequence(uint16 newSequenceID);
	void resetSequenceIP();
	byte *getSeqIpAtOffset(uint32 offset);
	void loadFrame(uint16 frameOffset);
	void freeFrame();
	void reset_maybe();
	bool startWalk(int16 destX, int16 destY, uint16 flags);
	void walkPath();
	void waitUntilFlag4IsSet();
	void waitUntilFlag8IsSet();
	void waitUntilFlag8And4AreSet();
	void waitUntilFlag8SetThenSet1000();
	void waitUntilFlag8SetThenSet1000AndWaitFor4();
	void waitForWalkToFinish();

	bool waitUntilFlag4IsSetAllowSkip();
	bool actorSetSequenceAndWaitAllowSkip(uint16 newSequenceID);

	void clearFlag(uint32 flag);
	void setFlag(uint32 flag);
	bool isFlagSet(uint32 flag);
	bool isFlagClear(uint32 flag) { return !isFlagSet(flag); }

	byte *getPalette();
	int16 getFrameYOffset();
private:
	void stopWalk();
	uint16 canWalkLine(int16 actor_x, int16 actor_y, int16 target_x, int16 target_y, uint16 walkFlags);
	int16 pathfindingFindClosestPoint(int16 actor_x, int16 actor_y, int16 target_x, int16 target_y, int16 unkType,
									  bool *pointsInUseTbl);
	int startMoveToPoint(int destX, int destY);
};

} // End of namespace Dragons

#endif //DRAGONS_ACTOR_H
