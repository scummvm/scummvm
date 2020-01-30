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

#ifndef NUVIE_CORE_PLAYER_H
#define NUVIE_CORE_PLAYER_H

#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/actors/actor.h"

namespace Ultima {
namespace Nuvie {

#define MOVE_COST_USE 5

class Configuration;
class GameClock;
class Actor;
class ActorManager;
class MapWindow;
class Party;
class NuvieIO;

class Player {
	Configuration *config;
	int game_type;
	GameClock *clock;
	Party *party;
	bool party_mode;
	bool mapwindow_centered;
	Actor *actor;
	ActorManager *actor_manager;
	ObjManager *obj_manager;

// char name[14]; We can get the name from the player actor. --SB-X
	uint8 gender;

	uint8 questf;
	uint8 karma;
	uint8 gargishf; // learned Gargish
	uint8 alcohol; // number of alcoholic drinks consumed

	MapWindow *map_window;

	sint8 current_weapon;

public:

	Player(Configuration *cfg);

	bool init(ObjManager *om, ActorManager *am, MapWindow *mw, GameClock *c, Party *p);
	void init();
	bool load(NuvieIO *objlist);
	bool save(NuvieIO *objlist);

	Actor *find_actor();
	void update_player(Actor *next_player);

	bool is_mapwindow_centered()            {
		return (mapwindow_centered);
	}
	void set_mapwindow_centered(bool state);

	bool is_in_vehicle() {
		return (get_actor()->get_actor_num() == 0);
	}

	Party *get_party()      {
		return (party);
	}
	bool set_party_mode(Actor *new_actor);
	bool set_solo_mode(Actor *new_actor);
	bool in_party_mode()    {
		return (party_mode);
	}

	void set_karma(uint8 val) {
		karma = val;
	}
	uint8 get_karma()         {
		return (karma);
	}
	void add_karma(uint8 val = 1);
	void subtract_karma(uint8 val = 1);
	void subtract_movement_points(uint8 points);
	void add_alcohol(uint8 val = 1) {
		alcohol = clamp_max(alcohol + val, 255);
	}
	void dec_alcohol(uint8 val = 1) {
		if (alcohol > val) {
			alcohol -= val;
		} else {
			alcohol = 0;
		}
	}

	void set_quest_flag(uint8 val) {
		questf = val;
	}
	uint8 get_quest_flag()         {
		return (questf);
	}
	void set_gargish_flag(uint8 val) {
		gargishf = val;
	}
	uint8 get_gargish_flag()          {
		return (gargishf);
	}

	void set_actor(Actor *new_actor);
	Actor *get_actor();
	void get_location(uint16 *ret_x, uint16 *ret_y, uint8 *ret_level);
	uint8 get_location_level();

	const char *get_name();
	void set_gender(uint8 val) {
		gender = val;
	}
	const char *get_gender_title();
	uint8 get_gender()         {
		return (gender);
	}

	bool check_moveRelative(sint16 rel_x, sint16 rel_y);
	void moveRelative(sint16 rel_x, sint16 rel_y, bool mouse_movement = false);
	void try_open_door(uint16 x, uint16 y, uint8 z);
	void move(sint16 new_x, sint16 new_y, uint8 new_level, bool teleport);
	void moveLeft();
	void moveRight();
	void moveUp();
	void moveDown();
	void pass();
	void repairShip();

	uint32 get_walk_delay();
	bool check_walk_delay();

	bool weapon_can_hit(uint16 x, uint16 y);
	void attack_select_init(bool use_attack_text = true);
	bool attack_select_next_weapon(bool add_newline = false, bool use_attack_text = true);

	void attack(MapCoord target, Actor *target_actor);
	sint8 get_current_weapon() {
		return current_weapon;
	}

protected:

	bool attack_select_weapon_at_location(sint8 location, bool add_newline = false, bool use_attack_text = true);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
