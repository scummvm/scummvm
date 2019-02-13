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
#ifndef SCUMMVM_ACTOR_H
#define SCUMMVM_ACTOR_H

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
	ACTOR_FLAG_10 = 0x10,
	ACTOR_FLAG_20 = 0x20,
	ACTOR_FLAG_40 = 0x40,
	ACTOR_FLAG_80 = 0x80,
	ACTOR_FLAG_100 = 0x100,
	ACTOR_FLAG_200 = 0x200,
	ACTOR_FLAG_400 = 0x400,
	ACTOR_FLAG_800 = 0x800,
	ACTOR_FLAG_1000 = 0x1000,
	ACTOR_FLAG_2000 = 0x2000,
	ACTOR_FLAG_4000 = 0x4000,
	ACTOR_FLAG_8000 = 0x8000  //Seems turn off semi trans mode when selected.
};

class ActorManager {
public:
	typedef Common::Array<Actor> Actors;
	typedef Actors::iterator ActorsIterator;

private:
	ActorResourceLoader *_actorResourceLoader;
	Actors _actors;
public:
	ActorManager(ActorResourceLoader *actorResourceLoader);

public:
	Actor *loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y);
	Actor *loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y, uint16 priorityLayer);
	Actor *loadActor(uint32 resourceId, uint16 actorId);
	Actor *getActor(uint16 actorId);
	void clearActorFlags(uint16 startingActorId);

private:
	Actor *findFreeActor(int16 resourceID);
};

class Actor {
public:
	uint16 _actorID;
	ActorResource*_actorResource;
	uint16 actorFileDictionaryIndex;
	int16 resourceID;
	byte *_seqCodeIp;
	void* frame_pointer_maybe;
	ActorFrame *frame;
	Graphics::Surface *surface;
	uint16 field_c;
	int16 field_e;
	uint16 sequenceTimer;
	uint16 _sequenceID;
	uint16 _sequenceID2;
	int16 priorityLayer;
	uint16 flags;
	int16 x_pos;
	int16 y_pos;
	int16 target_x_pos;
	int16 target_y_pos;
	uint32 field_24;
	uint32 field_28;
	uint32 field_2c;
	uint32 field_30;
	uint16 field_34[32];
	uint16 field_74;
	int16 field_76;
	int16 field_78;
	uint16 field_7a;
	int32 field_7c;
	uint16 field_80;
	uint16 frame_vram_x;
	uint16 frame_vram_y;
	uint16 frame_width;
	uint16 frame_height;
	uint16 frame_flags;
	uint16 clut;
public:

	Actor(uint16 id);
	void init(ActorResource *resource, int16 x, int16 y, uint32 sequenceID);
	void updateSequence(uint16 newSequenceID);
	void resetSequenceIP();
	byte *getSeqIpAtOffset(uint32 offset);
	void loadFrame(uint16 frameOffset);
	void reset_maybe();
	bool pathfinding_maybe(int16 target_x, int16 target_y, int16 isNotFlicker);
	void waitUntilFlag8IsSet();
	void waitUntilFlag8And4AreSet();

	void clearFlag(uint32 flag);
	void setFlag(uint32 flag);
private:
	void pathfindingCleanup();
	uint16 pathfindingUnk(int16 actor_x, int16 actor_y, int16 target_x, int16 target_y, int16 unkType);
};

} // End of namespace Dragons

#endif //SCUMMVM_ACTOR_H
