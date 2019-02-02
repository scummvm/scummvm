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
#include "actorresource.h"
#include "actor.h"

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

void Actor::pathfinding_maybe(int16 x, int16 y, int16 unk) {
	//TODO implement me.
	error("Implement pathfinding_maybe()");
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

} // End of namespace Dragons
