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

#ifndef NUVIE_CORE_EGG_MANAGER_H
#define NUVIE_CORE_EGG_MANAGER_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/core/obj_manager.h"

namespace Ultima {
namespace Nuvie {

struct Egg {
	bool seen_egg;
	Obj *obj;
	Egg() {
		seen_egg = false;
		obj = NULL;
	};
};

class Configuration;
class ActorManager;
class Actor;
class Map;

class EggManager {
	Configuration *config;
	Map *map;
	ActorManager *actor_manager;
	ObjManager *obj_manager;
	nuvie_game_t gametype; // what game is being played?

	Std::list<Egg *> egg_list;

public:

	EggManager(Configuration *cfg, nuvie_game_t type, Map *m);
	~EggManager();

	void set_actor_manager(ActorManager *am) {
		actor_manager = am;
	}
	void set_obj_manager(ObjManager *om) {
		obj_manager = om;
	}

	void clean(bool keep_obj = true);

	void add_egg(Obj *egg);
	void remove_egg(Obj *egg, bool keep_egg = true);
	void set_egg_visibility(bool show_eggs);
	bool spawn_egg(Obj *egg, uint8 hatch_probability);
	void spawn_eggs(uint16 x, uint16 y, uint8 z, bool teleport = false);
	Std::list<Egg *> *get_egg_list() {
		return &egg_list;
	};
	bool is_spawning_actors() {
		return !not_spawning_actors;
	}
	void set_spawning_actors(bool spawning) {
		not_spawning_actors = !spawning;
	}

protected:

	uint8 get_worktype(Obj *embryo);
	bool not_spawning_actors;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
