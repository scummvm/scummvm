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

class ActorManager {
private:
	ActorResourceLoader *_actorResourceLoader;
public:
	ActorManager(ActorResourceLoader *actorResourceLoader);

public:
	Actor *loadActor(uint32 resourceId, uint32 sequenceId, int16 x, int16 y, uint16 field16);
};

class Actor {
public:
	ActorResourceLoader *_actorResourceLoader;
	uint16 actorFileDictionaryIndex;
	uint16 resourceID;
	int16 _seqCodeIp;
	void* frame_pointer_maybe;
	uint16 field_c;
	int16 var_e;
	uint16 frameIndex_maybe;
	uint16 _sequenceID;
	uint16 _sequenceID2;
	uint16 field16;
	uint16 field_18_flags_maybe;
	int16 x_pos;
	int16 y_pos;
	uint16 target_x_pos;
	uint16 target_y_pos;
	uint32 field_24;
	uint32 field_28;
	uint32 field_2c;
	uint32 field_30;
	uint16 field_34[32];
	uint16 field_74;
	uint16 field_76;
	uint16 field_78;
	uint16 field_7a;
	uint32 field_7c;
	uint16 field_80;
	uint16 frame_vram_x;
	uint16 frame_vram_y;
	uint16 frame_width;
	uint16 frame_height;
	uint16 frame_flags;
	uint16 clut;
public:

	Actor(ActorResourceLoader *actorResourceLoader, uint32 resourceID, int16 x, int16 y, uint16 sequenceID);
	Graphics::Surface *getCurrentFrame();
	void updateSequence(uint16 newSequenceID);
};

} // End of namespace Dragons

#endif //SCUMMVM_ACTOR_H
