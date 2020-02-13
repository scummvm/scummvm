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

#ifndef NUVIE_ACTORS_U6_ACTOR_H
#define NUVIE_ACTORS_U6_ACTOR_H

#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/misc/actor_list.h"

namespace Ultima {
namespace Nuvie {

#define MOVETYPE_U6_NONE       0
#define MOVETYPE_U6_LAND       1
#define MOVETYPE_U6_WATER_LOW  2 // skiffs, rafts
#define MOVETYPE_U6_WATER_HIGH 3 // ships
#define MOVETYPE_U6_AIR_LOW    4 // balloon, birds... this movetype cannot cross mountain tops.
#define MOVETYPE_U6_AIR_HIGH   5 // dragons
#define MOVETYPE_U6_ETHEREAL   6

#define REMOVE_SURROUNDING_OBJS true

#define ACTOR_MOVEMENT_FLAGS_CORPSER  0x10

typedef struct {
	uint16 base_obj_n;
	uint8 frames_per_direction;
	uint8 tiles_per_direction;
	uint8 tiles_per_frame;
	uint8 tile_start_offset; //used for ships where the frame_n starts at 8
	uint16 dead_obj_n;
	uint8 dead_frame_n;
	bool can_laydown;
	bool can_sit;
	ActorTileType tile_type;
	uint8 movetype;
	uint16 twitch_rand; //used to control how frequently an actor twitches, lower numbers twitch more
	uint8 body_armor_class;
} U6ActorType;

class U6Actor: public Actor {
protected:

	const U6ActorType *actor_type;
	const U6ActorType *base_actor_type;
	uint8 current_movetype;

	sint8 walk_frame_inc; // added to walk_frame each step

public:

	U6Actor(Map *m, ObjManager *om, GameClock *c);
	~U6Actor() override;

	bool init(uint8 obj_status = NO_OBJ_STATUS) override;
	uint16 get_downward_facing_tile_num() override;
	bool updateSchedule(uint8 hour, bool teleport = false) override;
	void set_worktype(uint8 new_worktype, bool init = false) override;
	void revert_worktype() override;
	void change_base_obj_n(uint16 val) override;
	void set_direction(uint8 d) override;
	void face_location(uint16 lx, uint16 ly) override;
	void clear() override;
	bool move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags = 0) override;
	bool check_move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags = 0) override;
	void twitch() override;
	void do_twitch();
	void die(bool create_body = true) override;
	void set_paralyzed(bool paralyzed) override;
	void set_protected(bool val) override;
	void set_charmed(bool val) override;
	void set_corpser_flag(bool val) override;
	void set_cursed(bool val) override;
	void set_asleep(bool val) override;
	void set_ethereal(bool val) override {
		current_movetype = val ? MOVETYPE_U6_ETHEREAL : actor_type->movetype;
		ethereal = val;
	}

	uint8 get_object_readiable_location(Obj *obj) override;
	const CombatType *get_object_combat_type(uint16 objN) override;
	ActorTileType get_tile_type() override {
		return (actor_type->tile_type);
	}
	Obj *inventory_get_food(Obj *container = 0) override;
	uint8 get_maxhp() override {
		return (((level * 30) <= 255) ? (level * 30) : 255);    // U6
	}
	uint8 get_maxmagic() override;

	bool weapon_can_hit(const CombatType *weapon, Actor *target, uint16 *hit_x, uint16 *hit_y) override;

	bool is_immobile() override; // frozen by worktype or status
	bool can_twitch();

	bool get_corpser_flag() override {
		return (movement_flags & ACTOR_MOVEMENT_FLAGS_CORPSER);
	}
	bool can_be_passed(Actor *other) override;
	bool will_not_talk() override;

	void set_actor_obj_n(uint16 new_obj_n);
	void pathfind_to(MapCoord &d) override;
	void handle_lightsource(uint8 hour) override;

	uint8 get_hp_text_color() override;
	uint8 get_str_text_color() override {
		return 0x48;
	}
	uint8 get_dex_text_color() override {
		return 0x48;
	}

protected:
	bool init_ship();
	bool init_splitactor(uint8 obj_status); //cows, horses etc.
	bool init_dragon();
	bool init_hydra();
	bool init_silver_serpent();
	void init_new_silver_serpent();
	void gather_snake_objs_from_map(Obj *start_obj, uint16 ax, uint16 ay, uint16 az);
	inline bool check_move_silver_serpent(uint16 x, uint16 y);
	bool sit_on_chair(Obj *obj);

	inline void discover_direction();
	void setup_walk_to_location();
	void wt_sleep(bool init = false);
	void wt_play_lute();

	inline const U6ActorType *get_actor_type(uint16 new_obj_n);

	inline bool has_surrounding_objs();
	inline void remove_surrounding_objs_from_map();
	inline void add_surrounding_objs_to_map();
	inline void move_surrounding_objs_relative(sint16 rel_x, sint16 rel_y);
	inline void move_silver_serpent_objs_relative(sint16 rel_x, sint16 rel_y);
	inline void set_direction_of_surrounding_objs(uint8 new_direction);
	inline void set_direction_of_surrounding_ship_objs(uint8 new_direction);
	inline void set_direction_of_surrounding_splitactor_objs(uint8 new_direction);
	inline void set_direction_of_surrounding_dragon_objs(uint8 new_direction);

	inline void twitch_surrounding_objs();
	inline void twitch_surrounding_dragon_objs();
	inline void twitch_surrounding_hydra_objs();
	inline void twitch_obj(Obj *obj);

	inline void clear_surrounding_objs_list(bool delete_objs = false);
	inline void init_surrounding_obj(uint16 x, uint16 y, uint8 z, uint16 actor_obj_n, uint16 obj_frame_n);

	const CombatType *get_hand_combat_type() override;

	void print() override;
	const char *get_worktype_string(uint32 wt) override;
	void inventory_make_all_objs_ok_to_take();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
