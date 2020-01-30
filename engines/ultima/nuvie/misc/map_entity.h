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

#ifndef NUVIE_MISC_MAP_ENTITY_H
#define NUVIE_MISC_MAP_ENTITY_H

#include "ultima/nuvie/core/nuvie_defs.h"

namespace Ultima {
namespace Nuvie {

class Actor;
class MapCoord;
class NuvieAnim;
class Obj;

typedef enum {
	ENT_NOTHING = 0,
	ENT_ACTOR,
	ENT_OBJ,
	ENT_ANIM
} EntityType;

/* Any object on the map ("in the world") that isn't part of the map.
 * WARNING: It just points to another object, and doesn't copy it.
 */
typedef struct MapEntity_s {
	EntityType entity_type;
	union {
		char *data;
		Actor *actor;
		Obj *obj;
		NuvieAnim *anim;
	};
	MapEntity_s()             {
		entity_type = ENT_NOTHING;
		data = NULL;
	}
	MapEntity_s(Actor *a)     {
		entity_type = ENT_ACTOR;
		actor = a;
	}
	MapEntity_s(Obj *o)       {
		entity_type = ENT_OBJ;
		obj = o;
	}
	MapEntity_s(NuvieAnim *a) {
		entity_type = ENT_ANIM;
		anim = a;
	}
} MapEntity;

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
