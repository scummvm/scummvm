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
#include "actorresource.h"
#include "actor.h"

namespace Dragons {

ActorManager::ActorManager(ActorResourceLoader *actorResourceLoader) : _actorResourceLoader(actorResourceLoader) {
	for (uint16 i = 0; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
		_actors.push_back(Actor(i));
	}
}


Actor *ActorManager::loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y, uint16 field16) {
	Actor *actor = loadActor(resourceId, sequenceId, x, y);
	if(actor) {
		actor->field16 = field16;
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

Actor::Actor(uint16 id) : _actorID(id) {
	_actorResource = NULL;
	resourceID = -1;
	_seqCodeIp = 0;
	frame_pointer_maybe = NULL;
	field16 = 3;
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
	_actorResource = resource;
	x_pos = x;
	y_pos = y;
	sequenceTimer = 0;
	target_x_pos = x;
	target_y_pos = y;
	var_e = 0x100;
	_sequenceID2 = 0;
	flags = (Dragons::ACTOR_FLAG_40 | Dragons::ACTOR_FLAG_4);
	frame_width = 0;
	frame_height = 0;
	frame_flags = 4;
	//TODO sub_80017010();

	updateSequence((uint16)sequenceID);
}

Graphics::Surface *Actor::getCurrentFrame() {
	frame_vram_x = _actorResource->getFrameHeader(sequenceTimer)->xOffset;
	frame_vram_y = _actorResource->getFrameHeader(sequenceTimer)->yOffset;
	return _actorResource->loadFrame(sequenceTimer);
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
	surface = _actorResource->loadFrame(*frame);

	debug(3, "load frame header: (%d,%d)", frame->width, frame->height);

}

byte *Actor::getSeqIpAtOffset(uint32 offset) {
	return _actorResource->getSequenceDataAtOffset(offset);
}

} // End of namespace Dragons
