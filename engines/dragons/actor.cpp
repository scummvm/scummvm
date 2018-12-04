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

ActorManager::ActorManager(ActorResourceLoader *actorResourceLoader) : _actorResourceLoader(actorResourceLoader) {}

Actor *ActorManager::loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y, uint16 field16) {
	debug("Load actor: resourceId: %d, SequenceId: %d, position: (%d,%d) field16: %d", resourceId, sequenceId, x, y, field16);
	//ActorResource *resource = _actorResourceLoader->load(resourceId);
	Actor *actor = new Actor(_actorResourceLoader, resourceId, x, y, sequenceId);
	return actor;
}

Actor::Actor(ActorResourceLoader *actorResourceLoader, uint32 resourceID, int16 x, int16 y, uint32 sequenceID): _actorResourceLoader(actorResourceLoader), resourceID(resourceID), x_pos(x), y_pos(y) {
	frameIndex_maybe = 0;
	target_x_pos = x;
	target_y_pos = y;
	var_e = 0x100;
	_sequenceID2 = 0;
	flags = (0x40 | 4);
	frame_width = 0;
	frame_height = 0;
	frame_flags = 4;
	//TODO sub_80017010();

	updateSequence((uint16)sequenceID);
}

Graphics::Surface *Actor::getCurrentFrame() {
	ActorResource *resource = _actorResourceLoader->load(resourceID);
	frame_vram_x = resource->getFrameHeader(frameIndex_maybe)->field_0;
	frame_vram_y = resource->getFrameHeader(frameIndex_maybe)->field_2;
	return resource->loadFrame(frameIndex_maybe);
}

void Actor::updateSequence(uint16 newSequenceID) {
	_sequenceID = newSequenceID;
	flags &= 0xfbf1;
	flags |= 1;
}

} // End of namespace Dragons
