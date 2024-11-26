/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/lua/lauxlib.h"

#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/script/script_actor.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/core/converse.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/views/portrait_view.h"

namespace Ultima {
namespace Nuvie {

///
//@module script

/***
An Actor object
@table Actor
@string[readonly] luatype This returns "actor"
@int[readonly] actor_num
@int align alignment

- 0 = DEFAULT
- 1 = NEUTRAL
- 2 = EVIL
- 3 = GOOD
- 4 = CHAOTIC

@bool[readonly] alive
@bool asleep
@int base_obj_n
@bool charmed
@bool cold (MD) bit 0 on the status flags. Set when the actor is cold.
@int combat_mode
@bool corpser_flag (U6) Has the actor been dragged underground by a corpser
@bool cursed
@int dex dexterity
@int direction The direction that the actor is facing.

- 0 = NORTH
- 1 = EAST
- 2 = SOUTH
- 3 = WEST
- 4 = NORTHEAST
- 5 = SOUTHEAST
- 6 = SOUTHWEST
- 7 = NORTHWEST
- 8 = NONE

@int exp experience
@int frame_n
@bool frenzy (MD) battle frenzy
@bool hit_flag Used by U6 to determine if an actor has been hit.
@int hp hit points
@bool hypoxia (MD) bit 6 on the obj flags. Tells if the actor is oxygen deprived
@bool[readonly] in_party Is the actor a mamber of the party?
@bool[readonly] in_vehicle Is the actor currently in a vehicle?
@int int intelligence
@int level
@int magic
@int[readonly] max_hp
@int mpts movement points
@string[readonly] name
@bool obj_flag_0 obj flag 0
@int obj_n
@int old_align Old alignment
@int[readonly] old_frame_n
@bool paralyzed
@bool poisoned
@bool protected
@int[readonly] sched_loc The location defined for the Actor's currently active schedule entry.
@int[readonly] sched_wt The worktype of the currently active schedule.
@int str strength
@bool[readonly] temp Is this a temporary actor?
@int[readonly] tile_num The tile number based on the obj_n + frame_n combination.
@bool visible Is the actor currently visible?
@int wt worktype
@field[readonly] xyz table containing location

 table format
```
{["x"]=x, ["y"]=y, ["z"]=z}
```
@int x
@int y
@int z

*/

extern bool nscript_get_location_from_args(lua_State *L, uint16 *x, uint16 *y, uint8 *z, int lua_stack_offset = 1);
extern Obj *nscript_get_obj_from_args(lua_State *L, int lua_stack_offset);
extern void nscript_new_obj_var(lua_State *L, Obj *obj);
extern int nscript_obj_new(lua_State *L, Obj *obj);
extern int nscript_u6llist_iter(lua_State *L);
extern int nscript_init_u6link_iter(lua_State *L, U6LList *list, bool is_recursive);

static int nscript_actor_new(lua_State *L);
static int nscript_actor_clone(lua_State *L);
static int nscript_get_actor_from_num(lua_State *L);

Actor *nscript_get_actor_from_args(lua_State *L, int lua_stack_offset = 1);
static int nscript_actor_set(lua_State *L);
static int nscript_actor_get(lua_State *L);
static int nscript_get_player_actor(lua_State *L);
static int nscript_actor_kill(lua_State *L);
static int nscript_actor_hit(lua_State *L);
static int nscript_actor_get_range(lua_State *L);
static int nscript_actor_resurrect(lua_State *L);
static int nscript_actor_inv_add_obj(lua_State *L);
static int nscript_actor_inv_remove_obj(lua_State *L);
static int nscript_actor_inv_remove_obj_qty(lua_State *L);
static int nscript_actor_inv_get_readied_obj_n(lua_State *L);
static int nscript_actor_inv_ready_obj(lua_State *L);
static int nscript_actor_inv_unready_obj(lua_State *L);
static int nscript_actor_inv_has_obj_n(lua_State *L);
static int nscript_actor_inv_get_obj_n(lua_State *L);
static int nscript_actor_inv_get_obj_total_qty(lua_State *L);
static int nscript_actor_move(lua_State *L);
static int nscript_actor_walk_path(lua_State *L);
static int nscript_actor_is_at_scheduled_location(lua_State *L);
static int nscript_actor_can_carry_obj(lua_State *L);
static int nscript_actor_can_carry_obj_weight(lua_State *L);
static int nscript_actor_black_fade_effect(lua_State *L);
static int nscript_actor_fade_out_effect(lua_State *L);
static int nscript_actor_show_portrait(lua_State *L);
static int nscript_actor_hide_portrait(lua_State *L);
static int nscript_actor_talk(lua_State *L);
static int nscript_actor_unlink_surrounding_objs(lua_State *L);
static int nscript_actor_use(lua_State *L);
static int nscript_actor_get_talk_flag(lua_State *L);
static int nscript_actor_set_talk_flag(lua_State *L);
static int nscript_actor_clear_talk_flag(lua_State *L);
static int nscript_actor_get_number_of_schedules(lua_State *L);
static int nscript_actor_get_schedule(lua_State *L);

static const struct luaL_Reg nscript_actorlib_f[] = {
	{ "new", nscript_actor_new },
	{ "clone", nscript_actor_clone },
	{ "kill", nscript_actor_kill },
	{ "hit", nscript_actor_hit },
	{ "get_range", nscript_actor_get_range },
	{ "resurrect", nscript_actor_resurrect },
	{ "move", nscript_actor_move },
	{ "walk_path", nscript_actor_walk_path },
	{ "get", nscript_get_actor_from_num },
	{ "get_player_actor", nscript_get_player_actor },
	{ "inv_add_obj", nscript_actor_inv_add_obj },
	{ "inv_remove_obj", nscript_actor_inv_remove_obj },
	{ "inv_remove_obj_qty", nscript_actor_inv_remove_obj_qty },
	{ "inv_get_readied_obj_n", nscript_actor_inv_get_readied_obj_n },
	{ "inv_ready_obj", nscript_actor_inv_ready_obj },
	{ "inv_unready_obj", nscript_actor_inv_unready_obj },
	{ "inv_has_obj_n", nscript_actor_inv_has_obj_n },
	{ "inv_get_obj_n", nscript_actor_inv_get_obj_n },
	{ "inv_get_obj_total_qty", nscript_actor_inv_get_obj_total_qty },
	{ "is_at_scheduled_location", nscript_actor_is_at_scheduled_location },
	{ "can_carry_obj", nscript_actor_can_carry_obj },
	{ "can_carry_obj_weight", nscript_actor_can_carry_obj_weight },
	{ "black_fade_effect", nscript_actor_black_fade_effect },
	{ "fade_out", nscript_actor_fade_out_effect },
	{ "show_portrait", nscript_actor_show_portrait },
	{ "hide_portrait", nscript_actor_hide_portrait },
	{ "talk", nscript_actor_talk },
	{ "unlink_surrounding_objs", nscript_actor_unlink_surrounding_objs },
	{ "use", nscript_actor_use },
	{ "get_talk_flag", nscript_actor_get_talk_flag },
	{ "set_talk_flag", nscript_actor_set_talk_flag },
	{ "clear_talk_flag", nscript_actor_clear_talk_flag },
	{ "get_number_of_schedules", nscript_actor_get_number_of_schedules },
	{ "get_schedule", nscript_actor_get_schedule },

	{ nullptr, nullptr }
};
static const struct luaL_Reg nscript_actorlib_m[] = {
	{ "__index", nscript_actor_get },
	{ "__newindex", nscript_actor_set },
	{ nullptr, nullptr }
};


//Actor variables - must be in alphabetical order
static const char *const actor_set_vars[] = {
	"align",
	"asleep",
	"base_obj_n",
	"charmed",
	"cold",
	"combat_mode",
	"corpser_flag",
	"cursed",
	"dex",
	"direction",
	"exp",
	"frame_n",
	"frenzy",
	"hit_flag",
	"hp",
	"hypoxia",
	"int",
	"level",
	"magic",
	"mpts",
	"obj_flag_0",
	"obj_n",
	"old_align",
	"paralyzed",
	"poisoned",
	"protected",
	"str",
	"visible",
	"wt",
	"x",
	"y",
	"z"
};

//Actor variables - must be in alphabetical order
static const char *const actor_get_vars[] = {
	"actor_num",
	"align",
	"alive",
	"asleep",
	"base_obj_n",
	"charmed",
	"cold",
	"combat_mode",
	"corpser_flag",
	"cursed",
	"dex",
	"direction",
	"exp",
	"frame_n",
	"frenzy",
	"hit_flag",
	"hp",
	"hypoxia",
	"in_party",
	"in_vehicle",
	"int",
	"level",
	"luatype",
	"magic",
	"max_hp",
	"mpts",
	"name",
	"obj_flag_0",
	"obj_n",
	"old_align",
	"old_frame_n",
	"paralyzed",
	"poisoned",
	"protected",
	"sched_loc",
	"sched_wt",
	"str",
	"temp",
	"tile_num",
	"visible",
	"wt",
	"x",
	"xyz",
	"y",
	"z"
};

//Actor set
static int nscript_actor_set_align(Actor *actor, lua_State *L);
static int nscript_actor_set_asleep_flag(Actor *actor, lua_State *L);
static int nscript_actor_set_base_obj_n(Actor *actor, lua_State *L);
static int nscript_actor_set_charmed_flag(Actor *actor, lua_State *L);
static int nscript_actor_set_cold_flag(Actor *actor, lua_State *L);
static int nscript_actor_set_combat_mode(Actor *actor, lua_State *L);
static int nscript_actor_set_corpser_flag(Actor *actor, lua_State *L);
static int nscript_actor_set_cursed_flag(Actor *actor, lua_State *L);
static int nscript_actor_set_dexterity(Actor *actor, lua_State *L);
static int nscript_actor_set_direction(Actor *actor, lua_State *L);
static int nscript_actor_set_exp(Actor *actor, lua_State *L);
static int nscript_actor_set_frame_n(Actor *actor, lua_State *L);
static int nscript_actor_set_frenzy(Actor *actor, lua_State *L);
static int nscript_actor_set_hit(Actor *actor, lua_State *L);
static int nscript_actor_set_hp(Actor *actor, lua_State *L);
static int nscript_actor_set_hypoxia(Actor *actor, lua_State *L);
static int nscript_actor_set_intelligence(Actor *actor, lua_State *L);
static int nscript_actor_set_level(Actor *actor, lua_State *L);
static int nscript_actor_set_magic(Actor *actor, lua_State *L);
static int nscript_actor_set_movement_pts(Actor *actor, lua_State *L);
static int nscript_actor_set_obj_flag_0(Actor *actor, lua_State *L);
static int nscript_actor_set_obj_n(Actor *actor, lua_State *L);
static int nscript_actor_set_old_align(Actor *actor, lua_State *L);
static int nscript_actor_set_paralyzed_flag(Actor *actor, lua_State *L);
static int nscript_actor_set_poisoned_flag(Actor *actor, lua_State *L);
static int nscript_actor_set_protected_flag(Actor *actor, lua_State *L);
static int nscript_actor_set_strength(Actor *actor, lua_State *L);
static int nscript_actor_set_visible_flag(Actor *actor, lua_State *L);
static int nscript_actor_set_worktype(Actor *actor, lua_State *L);
static int nscript_actor_set_x(Actor *actor, lua_State *L);
static int nscript_actor_set_y(Actor *actor, lua_State *L);
static int nscript_actor_set_z(Actor *actor, lua_State *L);

int (*const actor_set_func[])(Actor *, lua_State *) = {
	nscript_actor_set_align,
	nscript_actor_set_asleep_flag,
	nscript_actor_set_base_obj_n,
	nscript_actor_set_charmed_flag,
	nscript_actor_set_cold_flag,
	nscript_actor_set_combat_mode,
	nscript_actor_set_corpser_flag,
	nscript_actor_set_cursed_flag,
	nscript_actor_set_dexterity,
	nscript_actor_set_direction,
	nscript_actor_set_exp,
	nscript_actor_set_frame_n,
	nscript_actor_set_frenzy,
	nscript_actor_set_hit,
	nscript_actor_set_hp,
	nscript_actor_set_hypoxia,
	nscript_actor_set_intelligence,
	nscript_actor_set_level,
	nscript_actor_set_magic,
	nscript_actor_set_movement_pts,
	nscript_actor_set_obj_flag_0,
	nscript_actor_set_obj_n,
	nscript_actor_set_old_align,
	nscript_actor_set_paralyzed_flag,
	nscript_actor_set_poisoned_flag,
	nscript_actor_set_protected_flag,
	nscript_actor_set_strength,
	nscript_actor_set_visible_flag,
	nscript_actor_set_worktype,
	nscript_actor_set_x,
	nscript_actor_set_y,
	nscript_actor_set_z
};

//Actor get
static int nscript_actor_get_actor_num(Actor *actor, lua_State *L);
static int nscript_actor_get_align(Actor *actor, lua_State *L);
static int nscript_actor_get_alive(Actor *actor, lua_State *L);
static int nscript_actor_get_asleep_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_base_obj_n(Actor *actor, lua_State *L);
static int nscript_actor_get_charmed_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_cold_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_combat_mode(Actor *actor, lua_State *L);
static int nscript_actor_get_corpser_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_cursed_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_dexterity(Actor *actor, lua_State *L);
static int nscript_actor_get_direction(Actor *actor, lua_State *L);
static int nscript_actor_get_exp(Actor *actor, lua_State *L);
static int nscript_actor_get_frame_n(Actor *actor, lua_State *L);
static int nscript_actor_get_frenzy(Actor *actor, lua_State *L);
static int nscript_actor_get_hit_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_hp(Actor *actor, lua_State *L);
static int nscript_actor_get_hypoxia(Actor *actor, lua_State *L);
static int nscript_actor_get_in_party_status(Actor *actor, lua_State *L);
static int nscript_actor_get_in_vehicle(Actor *actor, lua_State *L);
static int nscript_actor_get_intelligence(Actor *actor, lua_State *L);
static int nscript_actor_get_level(Actor *actor, lua_State *L);
static int nscript_actor_get_luatype(Actor *actor, lua_State *L);
static int nscript_actor_get_magic(Actor *actor, lua_State *L);
static int nscript_actor_get_max_hp(Actor *actor, lua_State *L);
static int nscript_actor_get_movement_pts(Actor *actor, lua_State *L);
static int nscript_actor_get_name(Actor *actor, lua_State *L);
static int nscript_actor_get_obj_flag_0(Actor *actor, lua_State *L);
static int nscript_actor_get_obj_n(Actor *actor, lua_State *L);
static int nscript_actor_get_old_align(Actor *actor, lua_State *L);
static int nscript_actor_get_old_frame_n(Actor *actor, lua_State *L);
static int nscript_actor_get_paralyzed_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_poisoned_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_protected_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_sched_loc(Actor *actor, lua_State *L);
static int nscript_actor_get_sched_worktype(Actor *actor, lua_State *L);
static int nscript_actor_get_strength(Actor *actor, lua_State *L);
static int nscript_actor_get_temp_status(Actor *actor, lua_State *L);
static int nscript_actor_get_tile_num(Actor *actor, lua_State *L);
static int nscript_actor_get_visible_flag(Actor *actor, lua_State *L);
static int nscript_actor_get_worktype(Actor *actor, lua_State *L);
static int nscript_actor_get_x(Actor *actor, lua_State *L);
static int nscript_actor_get_xyz(Actor *actor, lua_State *L);
static int nscript_actor_get_y(Actor *actor, lua_State *L);
static int nscript_actor_get_z(Actor *actor, lua_State *L);

int (*const actor_get_func[])(Actor *, lua_State *) = {
	nscript_actor_get_actor_num,
	nscript_actor_get_align,
	nscript_actor_get_alive,
	nscript_actor_get_asleep_flag,
	nscript_actor_get_base_obj_n,
	nscript_actor_get_charmed_flag,
	nscript_actor_get_cold_flag,
	nscript_actor_get_combat_mode,
	nscript_actor_get_corpser_flag,
	nscript_actor_get_cursed_flag,
	nscript_actor_get_dexterity,
	nscript_actor_get_direction,
	nscript_actor_get_exp,
	nscript_actor_get_frame_n,
	nscript_actor_get_frenzy,
	nscript_actor_get_hit_flag,
	nscript_actor_get_hp,
	nscript_actor_get_hypoxia,
	nscript_actor_get_in_party_status,
	nscript_actor_get_in_vehicle,
	nscript_actor_get_intelligence,
	nscript_actor_get_level,
	nscript_actor_get_luatype,
	nscript_actor_get_magic,
	nscript_actor_get_max_hp,
	nscript_actor_get_movement_pts,
	nscript_actor_get_name,
	nscript_actor_get_obj_flag_0,
	nscript_actor_get_obj_n,
	nscript_actor_get_old_align,
	nscript_actor_get_old_frame_n,
	nscript_actor_get_paralyzed_flag,
	nscript_actor_get_poisoned_flag,
	nscript_actor_get_protected_flag,
	nscript_actor_get_sched_loc,
	nscript_actor_get_sched_worktype,
	nscript_actor_get_strength,
	nscript_actor_get_temp_status,
	nscript_actor_get_tile_num,
	nscript_actor_get_visible_flag,
	nscript_actor_get_worktype,
	nscript_actor_get_x,
	nscript_actor_get_xyz,
	nscript_actor_get_y,
	nscript_actor_get_z
};


static int nscript_map_get_actor(lua_State *L);
static int nscript_update_actor_schedules(lua_State *L);

static int nscript_actor_inv(lua_State *L);

void nscript_init_actor(lua_State *L) {
	luaL_newmetatable(L, "nuvie.Actor");

	luaL_register(L, nullptr, nscript_actorlib_m);

	luaL_register(L, "Actor", nscript_actorlib_f);

	lua_pushcfunction(L, nscript_map_get_actor);
	lua_setglobal(L, "map_get_actor");

	lua_pushcfunction(L, nscript_update_actor_schedules);
	lua_setglobal(L, "update_actor_schedules");

	lua_pushcfunction(L, nscript_actor_inv);
	lua_setglobal(L, "actor_inventory");
}

bool nscript_new_actor_var(lua_State *L, uint16 actor_num) {
	uint16 *userdata;

	userdata = (uint16 *)lua_newuserdata(L, sizeof(uint16));

	luaL_getmetatable(L, "nuvie.Actor");
	lua_setmetatable(L, -2);

	*userdata = actor_num;

	return true;
}

/***
Create a new temporary Actor.
@function Actor.new
@int obj_n
@int[opt=0] x
@int[opt=0] y
@int[opt=0] z
@int[opt=NEUTRAL] alignment
@int[opt=8] worktype
@treturn Actor|nil
@within Actor
 */
static int nscript_actor_new(lua_State *L) {
	Actor *actor;
	uint16 obj_n = 0;
	uint16 x = 0;
	uint16 y = 0;
	uint8 z = 0;
	ActorAlignment alignment = ACTOR_ALIGNMENT_NEUTRAL;
	uint8 worktype = ACTOR_WT_ASSAULT; //FIXME this may be U6 specific.

	int nargs = lua_gettop(L);

	if (nargs > 1) { // do we have arguments?
		uint8 i = nargs;

		if (i) {
			if (!lua_isnil(L, 1))
				obj_n = (uint16)lua_tointeger(L, 1);
			i--;
		}

		if (i) {
			if (!lua_isnil(L, 2))
				x = (uint16)lua_tointeger(L, 2);
			i--;
		}

		if (i) {
			if (!lua_isnil(L, 3))
				y = (uint16)lua_tointeger(L, 3);
			i--;
		}

		if (i) {
			if (!lua_isnil(L, 4))
				z = (uint8)lua_tointeger(L, 4);
			i--;
		}

		if (i) {
			if (!lua_isnil(L, 5))
				alignment = (ActorAlignment)lua_tointeger(L, 5);
			i--;
		}

		if (i) {
			if (!lua_isnil(L, 6))
				worktype = (uint8)lua_tointeger(L, 6);
			i--;
		}

		//init here.
		if (Game::get_game()->get_actor_manager()->create_temp_actor(obj_n, NO_OBJ_STATUS, x, y, z, alignment, worktype, &actor) == false)
			return 0;

		//create the new lua Actor variable
		if (nscript_new_actor_var(L, actor->get_actor_num()) == false)
			return 0;
	}

	return 1;
}

/***
Clone an actor as a new temporary actor.
@function Actor.clone
@tparam Actor actor Actor to clone
@tparam MapCoord|x,y,z location Location to place newly cloned actor.
@within Actor
 */
static int nscript_actor_clone(lua_State *L) {
	Actor *actor, *new_actor;
	uint16 x, y;
	uint8 z;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	if (nscript_get_location_from_args(L, &x, &y, &z, 2) == false)
		return 0;

	int stack_offset = lua_istable(L, 2) ? 3 : 5;

	if (lua_gettop(L) >= stack_offset) {
		//FIXME clone into existing actor here.
	}
	if (Game::get_game()->get_actor_manager()->clone_actor(actor, &new_actor, MapCoord(x, y, z))) {
		if (nscript_new_actor_var(L, actor->get_actor_num()) == true)
			return 1;
	}

	return 0;
}

/***
Get an Actor object from an actor number
@function Actor.get
@int actor_number
@treturn Actor
@within Actor
 */
static int nscript_get_actor_from_num(lua_State *L) {
	uint16 actor_num;
	actor_num = (uint16)lua_tointeger(L, 1);

	assert(actor_num < ACTORMANAGER_MAX_ACTORS);
	nscript_new_actor_var(L, actor_num);

	return 1;
}

Actor *nscript_get_actor_from_args(lua_State *L, int lua_stack_offset) {
	Actor *actor = nullptr;

	if (lua_isuserdata(L, lua_stack_offset)) {
		uint16 *actor_num = (uint16 *)luaL_checkudata(L, lua_stack_offset, "nuvie.Actor");
		if (actor_num != nullptr)
			actor = Game::get_game()->get_actor_manager()->get_actor(*actor_num);
	} else {
		actor = Game::get_game()->get_actor_manager()->get_actor((uint16)lua_tointeger(L, lua_stack_offset));
	}

	return actor;
}

/***
Get the current player Actor object
@function Actor.get_player_actor
@treturn Actor
@within Actor
 */
static int nscript_get_player_actor(lua_State *L) {
	Actor *player_actor = Game::get_game()->get_player()->get_actor();

	nscript_new_actor_var(L, player_actor->get_actor_num());
	return 1;
}

static int nscript_actor_set(lua_State *L) {
	Actor *actor;
	const char *key;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	key = lua_tostring(L, 2);

	int idx = str_bsearch(actor_set_vars, sizeof(actor_set_vars) / sizeof(actor_set_vars[0]), (const char *)key);
	if (idx == -1)
		return 0;

	(*actor_set_func[idx])(actor, L);

	return 0;
}

static int nscript_actor_set_align(Actor *actor, lua_State *L) {
	actor->set_alignment((ActorAlignment)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_asleep_flag(Actor *actor, lua_State *L) {
	actor->set_asleep(lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_charmed_flag(Actor *actor, lua_State *L) {
	actor->set_charmed(lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_cold_flag(Actor *actor, lua_State *L) {
	actor->set_status_flag(ACTOR_MD_STATUS_FLAG_COLD, lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_combat_mode(Actor *actor, lua_State *L) {
	actor->set_combat_mode((uint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_corpser_flag(Actor *actor, lua_State *L) {
	actor->set_corpser_flag(lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_cursed_flag(Actor *actor, lua_State *L) {
	actor->set_cursed(lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_dexterity(Actor *actor, lua_State *L) {
	actor->set_dexterity((uint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_direction(Actor *actor, lua_State *L) {
	actor->set_direction((NuvieDir)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_exp(Actor *actor, lua_State *L) {
	actor->set_exp((uint16)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_frame_n(Actor *actor, lua_State *L) {
	actor->set_frame_n((uint16)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_frenzy(Actor *actor, lua_State *L) {
	actor->set_obj_flag(ACTOR_MD_OBJ_FLAG_FRENZY, lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_hp(Actor *actor, lua_State *L) {
	actor->set_hp((uint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_hypoxia(Actor *actor, lua_State *L) {
	actor->set_obj_flag(ACTOR_MD_OBJ_FLAG_HYPOXIA, lua_toboolean(L, 3)); //MD uses object bit 6 which is cursed in U6.
	return 0;
}

static int nscript_actor_set_hit(Actor *actor, lua_State *L) {
	actor->set_hit_flag(lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_intelligence(Actor *actor, lua_State *L) {
	actor->set_intelligence((uint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_level(Actor *actor, lua_State *L) {
	actor->set_level((uint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_magic(Actor *actor, lua_State *L) {
	actor->set_magic((uint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_movement_pts(Actor *actor, lua_State *L) {
	actor->set_moves_left((sint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_obj_flag_0(Actor *actor, lua_State *L) {
	actor->set_obj_flag(0, (bool)lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_obj_n(Actor *actor, lua_State *L) {
	actor->set_obj_n((uint16)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_old_align(Actor *actor, lua_State *L) {
	actor->set_old_alignment((ActorAlignment)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_base_obj_n(Actor *actor, lua_State *L) {
	actor->change_base_obj_n((uint16)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_paralyzed_flag(Actor *actor, lua_State *L) {
	actor->set_paralyzed((bool)lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_poisoned_flag(Actor *actor, lua_State *L) {
	actor->set_poisoned(lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_protected_flag(Actor *actor, lua_State *L) {
	actor->set_protected(lua_toboolean(L, 3));
	return 0;
}

static int nscript_actor_set_strength(Actor *actor, lua_State *L) {
	actor->set_strength((uint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_visible_flag(Actor *actor, lua_State *L) {
	actor->set_invisible(!lua_toboolean(L, 3)); //negate value before passing back to actor.
	return 0;
}

static int nscript_actor_set_worktype(Actor *actor, lua_State *L) {
	actor->set_worktype((uint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_x(Actor *actor, lua_State *L) {
	//actor->set_x((uint16)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_y(Actor *actor, lua_State *L) {
	//actor->set_y((uint16)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_set_z(Actor *actor, lua_State *L) {
	//actor->set_z((uint8)lua_tointeger(L, 3));
	return 0;
}

static int nscript_actor_get(lua_State *L) {
	Actor *actor;
	const char *key;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	key = lua_tostring(L, 2);

	int idx = str_bsearch(actor_get_vars, sizeof(actor_get_vars) / sizeof(actor_get_vars[0]), key);
	if (idx == -1)
		return 0;

	return (*actor_get_func[idx])(actor, L);
}

static int nscript_actor_get_actor_num(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_actor_num());
	return 1;
}

static int nscript_actor_get_align(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_alignment());
	return 1;
}

static int nscript_actor_get_alive(Actor *actor, lua_State *L) {
	lua_pushboolean(L, actor->is_alive());
	return 1;
}

static int nscript_actor_get_asleep_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_sleeping());
	return 1;
}

static int nscript_actor_get_base_obj_n(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_base_obj_n());
	return 1;
}

static int nscript_actor_get_cursed_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_cursed());
	return 1;
}

static int nscript_actor_get_combat_mode(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_combat_mode());
	return 1;
}

static int nscript_actor_get_charmed_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_charmed());
	return 1;
}

static int nscript_actor_get_cold_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->get_status_flag(ACTOR_MD_STATUS_FLAG_COLD));
	return 1;
}

static int nscript_actor_get_corpser_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->get_corpser_flag());
	return 1;
}

static int nscript_actor_get_dexterity(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_dexterity());
	return 1;
}

static int nscript_actor_get_direction(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_direction());
	return 1;
}

static int nscript_actor_get_exp(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_exp());
	return 1;
}

static int nscript_actor_get_frame_n(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_frame_n());
	return 1;
}

static int nscript_actor_get_frenzy(Actor *actor, lua_State *L) {
	lua_pushboolean(L, actor->get_obj_flag(ACTOR_MD_OBJ_FLAG_FRENZY));
	return 1;
}

static int nscript_actor_get_hit_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_hit());
	return 1;
}

static int nscript_actor_get_hp(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_hp());
	return 1;
}

static int nscript_actor_get_hypoxia(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->get_obj_flag(ACTOR_MD_OBJ_FLAG_HYPOXIA));
	return 1;
}

static int nscript_actor_get_in_party_status(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_in_party());
	return 1;
}

static int nscript_actor_get_in_vehicle(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_in_vehicle());
	return 1;
}

static int nscript_actor_get_intelligence(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_intelligence());
	return 1;
}

static int nscript_actor_get_level(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_level());
	return 1;
}

static int nscript_actor_get_luatype(Actor *actor, lua_State *L) {
	lua_pushstring(L, "actor");
	return 1;
}

static int nscript_actor_get_magic(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_magic());
	return 1;
}

static int nscript_actor_get_max_hp(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_maxhp());
	return 1;
}

static int nscript_actor_get_movement_pts(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_moves_left());
	return 1;
}

static int nscript_actor_get_name(Actor *actor, lua_State *L) {
	lua_pushstring(L, actor->get_name());
	return 1;
}

static int nscript_actor_get_obj_flag_0(Actor *actor, lua_State *L) {
	lua_pushboolean(L, actor->get_obj_flag(0));
	return 1;
}

static int nscript_actor_get_obj_n(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_obj_n());
	return 1;
}

static int nscript_actor_get_old_frame_n(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_old_frame_n());
	return 1;
}

static int nscript_actor_get_old_align(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_old_alignment());
	return 1;
}

static int nscript_actor_get_paralyzed_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_paralyzed());
	return 1;
}

static int nscript_actor_get_poisoned_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_poisoned());
	return 1;
}

static int nscript_actor_get_protected_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_protected());
	return 1;
}

static int nscript_actor_get_sched_loc(Actor *actor, lua_State *L) {
	MapCoord sched_loc;

	if (actor->get_schedule_location(&sched_loc) == false)
		return 0;

	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushinteger(L, sched_loc.x);
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushinteger(L, sched_loc.y);
	lua_settable(L, -3);

	lua_pushstring(L, "z");
	lua_pushinteger(L, sched_loc.z);
	lua_settable(L, -3);

	return 1;
}

static int nscript_actor_get_sched_worktype(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_sched_worktype());
	return 1;
}

static int nscript_actor_get_strength(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_strength());
	return 1;
}

static int nscript_actor_get_temp_status(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)actor->is_temp());
	return 1;
}

static int nscript_actor_get_tile_num(Actor *actor, lua_State *L) {
	Tile *tile = actor->get_tile();

	lua_pushinteger(L, (int)tile->tile_num);
	return 1;
}

static int nscript_actor_get_visible_flag(Actor *actor, lua_State *L) {
	lua_pushboolean(L, (int)(actor->is_invisible() ? false : true));
	return 1;
}

static int nscript_actor_get_worktype(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_worktype());
	return 1;
}

static int nscript_actor_get_x(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_x());
	return 1;
}

static int nscript_actor_get_xyz(Actor *actor, lua_State *L) {
	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushinteger(L, actor->get_x());
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushinteger(L, actor->get_y());
	lua_settable(L, -3);

	lua_pushstring(L, "z");
	lua_pushinteger(L, actor->get_z());
	lua_settable(L, -3);

	return 1;
}

static int nscript_actor_get_y(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_y());
	return 1;
}

static int nscript_actor_get_z(Actor *actor, lua_State *L) {
	lua_pushinteger(L, actor->get_z());
	return 1;
}

/***
Call the C++ Actor::die() method
@function Actor.kill
@tparam Actor actor
@bool[opt=true] create_body Create a dead body on the map?
@within Actor
 */
static int nscript_actor_kill(lua_State *L) {
	Actor *actor;
	bool create_body = true;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	if (lua_gettop(L) >= 2)
		create_body = (bool)lua_toboolean(L, 2);

	actor->die(create_body);

	return 0;
}

/***
Call the C++ Actor::hit() method
@function ultima/nuvie/actors/actor.hit
@tparam Actor actor
@int damage
@within Actor
 */
static int nscript_actor_hit(lua_State *L) {
	Actor *actor;
	uint8 damage;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	damage = (uint8)luaL_checkinteger(L, 2);

	actor->hit(damage, true); //force hit

	return 0;
}

/***
Calls the get_combat_range script function with the wrapped absolute x,y distances to the target
@function Actor.get_range
@tparam Actor actor
@int target_x
@int target_y
@treturn int range
@within Actor
 */
static int nscript_actor_get_range(lua_State *L) {
	Actor *actor;
	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;
	uint16 target_x = (uint16) luaL_checkinteger(L, 2);
	uint16 target_y = (uint16) luaL_checkinteger(L, 3);
	lua_pushinteger(L, actor->get_range(target_x, target_y));
	return 1;
}

/***
Move actor to location.
@function Actor.move
@tparam Actor actor
@tparam MapCoord|x,y,z location
@treturn bool Returns true is the move was successful. False if the move fails.
@within Actor
 */
static int nscript_actor_move(lua_State *L) {
	Actor *actor;
	uint16 x, y;
	uint8 z;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	if (nscript_get_location_from_args(L, &x, &y, &z, 2) == false)
		return 0;

	lua_pushboolean(L, (int)actor->move(x, y, z));

	return 1;
}

/***
Move the actor one space along their pathfinding path.
@function Actor.walk_path
@tparam Actor actor
@within Actor
 */
static int nscript_actor_walk_path(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	actor->update(); //FIXME this should be specific to pathfinding.

	return 0;
}

/***
Checks to see if the actor is currently at their scheduled worktype location.
@function Actor.is_at_scheduled_location
@tparam Actor actor
@treturn bool
@within Actor
 */
static int nscript_actor_is_at_scheduled_location(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	lua_pushboolean(L, actor->is_at_scheduled_location());
	return 1;
}

/***
Checks to see if the actor can carry an object.
It first checks that the object is get-able then checks that
the actor can physically carry the object's weight.
@function Actor.can_carry_obj
@tparam Actor actor
@tparam Obj object
@treturn bool
@within Actor
 */
static int nscript_actor_can_carry_obj(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Obj *obj = nscript_get_obj_from_args(L, 2);
	if (obj == nullptr)
		return 0;

	lua_pushboolean(L, (int)actor->can_carry_object(obj));
	return 1;
}

/***
Checks to see if the actor can carry an object's weight.
@function Actor.can_carry_obj_weight
@tparam Actor actor
@tparam Obj object
@treturn bool
@within Actor
 */
static int nscript_actor_can_carry_obj_weight(lua_State *L) {
	if (Game::get_game()->using_hackmove())
		return 1;
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Obj *obj = nscript_get_obj_from_args(L, 2);
	if (obj == nullptr)
		return 0;

	lua_pushboolean(L, (int)actor->can_carry_weight(obj));
	return 1;
}

/***
Fade the black pixels in the actor's tile to a specified color.
@function Actor.black_fade_effect
@tparam Actor actor
@int fade_color
@int fade_speed
@within Actor
 */
static int nscript_actor_black_fade_effect(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	uint8 fade_color = (uint8)lua_tointeger(L, 2);
	uint16 fade_speed = (uint8)lua_tointeger(L, 3);

	if (actor != nullptr) {
		AsyncEffect *e = new AsyncEffect(new TileBlackFadeEffect(actor, fade_color, fade_speed));
		e->run();
	}

	return 0;
}

/***
Pixel fade out the actor's tile to nothing.
@function Actor.fade_out
@tparam Actor actor
@int fade_speed
@within Actor
 */
static int nscript_actor_fade_out_effect(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	uint16 fade_speed = (uint8)lua_tointeger(L, 2);

	if (actor != nullptr) {
		AsyncEffect *e = new AsyncEffect(new TileFadeEffect(actor, fade_speed));
		e->run();
	}

	return 0;
}

/***
Display the actor's portrait
@function Actor.show_portrait
@tparam Actor actor
@within Actor
 */
static int nscript_actor_show_portrait(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Game::get_game()->get_view_manager()->set_portrait_mode(actor, actor->get_name());

	return 0;
}

/***
Hide the actor's portrait.
The party view is shown if in original UI mode.
@function ultima/nuvie/actors/actor.hide_portrait
@tparam Actor actor
@within Actor
 */
static int nscript_actor_hide_portrait(lua_State *L) {
	if (Game::get_game()->is_new_style())
		Game::get_game()->get_view_manager()->get_portrait_view()->Hide();
	else
		Game::get_game()->get_view_manager()->set_party_mode();

	return 0;
}

/***
Talk to actor. The script will pause until the conversation has ended.
@function Actor.talk
@tparam Actor actor
@within Actor
 */
static int nscript_actor_talk(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Game::get_game()->get_converse()->start(actor);
	Game::get_game()->update_until_converse_finished();
	return 0;
}

/***
For multi-tile actors, disconnect their surrounding objects.
@function Actor.unlink_surrounding_objs
@tparam Actor actor
@bool make_temp Should the objects be made temporary?
@within Actor
 */
static int nscript_actor_unlink_surrounding_objs(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	bool make_temp_obj = lua_toboolean(L, 2);

	actor->unlink_surrounding_objects(make_temp_obj);

	return 0;
}

/***
Call the C++ actor usecode logic.
@function Actor.use
@tparam Actor actor
@within Actor
 */
static int nscript_actor_use(lua_State *L) {
	UseCode *usecode = Game::get_game()->get_usecode();
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Obj *my_obj = actor->make_obj();
	usecode->use_obj(my_obj, actor);
	delete_obj(my_obj);

	return 0;
}

/***
Resurrect a dead actor. If the actor's body is not on the map then the actor will
be placed at a random map location around the player.
@function Actor.resurrect
@tparam Actor actor
@tparam MapCoord|x,y,z location Location to put the resurrected actor on the map
@tparam Obj body_obj The dead actor's body object.
@within Actor
 */
static int nscript_actor_resurrect(lua_State *L) {
	Actor *actor;
	MapCoord loc;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	if (nscript_get_location_from_args(L, &loc.x, &loc.y, &loc.z, 2) == false)
		return 0;
	Obj *obj = nscript_get_obj_from_args(L, 5);
	bool toss = (obj->is_in_inventory() || obj->is_in_container());
	if (toss)
		loc = Game::get_game()->get_player()->get_actor()->get_location();

	actor->resurrect(loc, obj);
	if (toss) {
		ActorManager *actor_manager = Game::get_game()->get_actor_manager();
		if (!actor_manager->toss_actor(actor, 2, 2))
			actor_manager->toss_actor(actor, 4, 4);
	}
	return 0;
}

/***
Add an object to the actor's inventory.
@function Actor.inv_add_obj
@tparam Actor actor
@tparam Obj obj
@bool[opt=false] stack_objs Should we stack obj.qty into existing objects?
@within Actor
 */
static int nscript_actor_inv_add_obj(lua_State *L) {
	Actor *actor;
	bool stack_objs = false;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Obj **s_obj = (Obj **)luaL_checkudata(L, 2, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;

	if (lua_gettop(L) >= 3) {
		stack_objs = lua_toboolean(L, 3);
	}

	actor->inventory_add_object(obj, nullptr, stack_objs);

	return 0;
}

/***
Remove an object from the actor's inventory
@function Actor.inv_remove_obj
@tparam Actor actor
@tparam Obj obj
@within Actor
 */
static int nscript_actor_inv_remove_obj(lua_State *L) {
	Actor *actor;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Obj **s_obj = (Obj **)luaL_checkudata(L, 2, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;


	actor->inventory_remove_obj(obj);

	return 0;
}

/***
Remove a number of stackable objects from the actor's inventory
@function Actor.inv_remove_obj_qty
@tparam Actor actor
@int obj_n Type of object to remove.
@int qty Number of objects to remove.
@treturn int returns the number of objects actually removed. This may be less than were requested.
@within Actor
 */
static int nscript_actor_inv_remove_obj_qty(lua_State *L) {
	Actor *actor;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	uint16 obj_n = (uint16)lua_tointeger(L, 2);
	uint16 qty = (uint16)lua_tointeger(L, 3);


	lua_pushinteger(L, actor->inventory_del_object(obj_n, qty, 0));

	return 1;
}

/***
Returns the obj_n the object that is readied at a given location.
@function Actor.inv_get_readied_obj_n
@tparam Actor actor
@int location

- 0 = HEAD
- 1 = NECK
- 2 = BODY
- 3 = ARM
- 4 = ARM_2
- 5 = HAND
- 6 = HAND_2
- 7 = FOOT

@treturn int returns obj_n or -1 if nothing is readied at the specified location.
@within Actor
 */
static int nscript_actor_inv_get_readied_obj_n(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr) {
		lua_pushinteger(L, -1);
		return 1;
	}
	uint8 location = (uint8)lua_tointeger(L, 2);
	lua_pushinteger(L, actor->inventory_get_readied_obj_n(location));
	return 1;
}

/***
Ready an object from the actor's inventory.
@function Actor.inv_ready_obj
@tparam Actor actor
@tparam Obj obj
@within Actor
 */
static int nscript_actor_inv_ready_obj(lua_State *L) {
	Actor *actor;
	MapCoord loc;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Obj **s_obj = (Obj **)luaL_checkudata(L, 2, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;

	actor->add_readied_object(obj);

	return 0;
}

/***
Unready an object from the actor.
First attempt to unready object using usecode. If there is no unready usecode then just unready the object normally.
@function Actor.inv_unready_obj
@tparam Actor actor
@tparam Obj obj
@within Actor
 */
static int nscript_actor_inv_unready_obj(lua_State *L) {
	Actor *actor;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Obj **s_obj = (Obj **)luaL_checkudata(L, 2, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;

	UseCode *usecode = Game::get_game()->get_usecode();

	//try to unready via usecode.
	if (usecode->has_readycode(obj) && (usecode->ready_obj(obj, actor) == false)) {
		return 0;
	}

	actor->remove_readied_object(obj);

	return 0;
}

/***
Check if the actor has an object of type obj_n in their inventory.
@function Actor.inv_has_obj_n
@tparam Actor actor
@int obj_n object number to search for
@treturn bool
@within Actor
 */
static int nscript_actor_inv_has_obj_n(lua_State *L) {
	Actor *actor;
	uint16 obj_n;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	obj_n = (uint16)luaL_checkinteger(L, 2);

	lua_pushboolean(L, (int)actor->inventory_has_object(obj_n, 0, false));

	return 1;
}

/***
Get the first object of type obj_n from the actor's inventory.
@function Actor.inv_get_obj_n
@tparam Actor actor
@int obj_n object number to search for
@int[opt] frame_n search on frame number
@int[opt] quality search on object quality
@treturn Obj returns an object or nil if no match was found
@within Actor
 */
static int nscript_actor_inv_get_obj_n(lua_State *L) {
	Actor *actor;
	uint16 obj_n;
	uint8 frame_n = 0;
	uint8 quality = 0;
	bool match_frame_n = false;
	bool match_quality = false;
	Obj *obj;
	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	obj_n = (uint16)luaL_checkinteger(L, 2);

	if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
		frame_n = (uint8)luaL_checkinteger(L, 3);
		match_frame_n = true;
	}

	if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
		quality = (uint8)luaL_checkinteger(L, 4);
		match_quality = true;
	}

	obj = actor->inventory_get_object(obj_n, quality, match_quality, frame_n, match_frame_n);

	if (obj) {
		nscript_new_obj_var(L, obj);
		return 1;
	}

	return 0;
}

/***
Get the total number of objects of type obj_n in the actor's inventory
@function Actor.inv_get_obj_total_qty
@tparam Actor actor
@int obj_n object number to search for
@treturn int
@within Actor
 */
static int nscript_actor_inv_get_obj_total_qty(lua_State *L) {
	Actor *actor;
	uint16 obj_n;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	obj_n = (uint16)luaL_checkinteger(L, 2);

	lua_pushinteger(L, actor->inventory_count_object(obj_n));

	return 1;
}

/***
Get the actor at map location.
@function map_get_actor
@tparam MapCoord|x,y,z location
@tparam[opt] Actor excluded_actor Actor to be excluded from search results
@treturn Actor returns the actor or nil if no actor was found
@within Actor
 */
static int nscript_map_get_actor(lua_State *L) {
	ActorManager *actor_manager = Game::get_game()->get_actor_manager();
	Actor *actor;

	uint16 x, y;
	uint8 z;

	if (nscript_get_location_from_args(L, &x, &y, &z) == false)
		return 0;
	Actor *excluded_actor = nullptr;
	void *p = lua_touserdata(L, 4); // avoid error warnings when null
	if (p != nullptr)
		excluded_actor = nscript_get_actor_from_args(L, 4);

	actor = actor_manager->get_actor(x, y, z, true, excluded_actor);

	if (actor == nullptr)
		return 0;

	if (nscript_new_actor_var(L, actor->get_actor_num()) == false)
		return 0;

	return 1;
}

/***
Update all actor schedules.
Schedules change on the hour.
@function update_actor_schedules
@bool[opt=false] teleport_actors Should actors teleport to their new scheduled location?
@within Actor
 */
static int nscript_update_actor_schedules(lua_State *L) {
	bool teleport;
	if (lua_gettop(L) >= 1)
		teleport = (bool)lua_toboolean(L, 1);
	else
		teleport = false;
	ActorManager *actor_manager = Game::get_game()->get_actor_manager();
	actor_manager->updateSchedules(teleport);
	return 0;
}

/***
Iterate through objects in the actor's inventory.
@function actor_inventory
@tparam Actor actor
@bool[opt=false] is_recursive should we search containers inside the inventory?
@usage
   local ac = 0
   local obj

   for obj in actor_inventory(actor) do
	  if obj.readied then
		 local armour = armour_tbl[obj.obj_n]
		 if armour ~= nil then
			ac = ac + armour
		 end
	  end
   end
@within Actor
 */
static int nscript_actor_inv(lua_State *L) {
	Actor *actor;
	bool is_recursive = false;

	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	if (lua_gettop(L) >= 2)
		is_recursive = lua_toboolean(L, 2);

	U6LList *inv = actor->get_inventory_list();

	return nscript_init_u6link_iter(L, inv, is_recursive);
}

/***
Set one of the actor's talk flags
@function Actor.set_talk_flag
@tparam Actor actor
@int flag The flag to set. 0..7
@within Actor
 */
static int nscript_actor_set_talk_flag(lua_State *L) {
	Actor *actor;
	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;
	actor->set_flag((uint8)lua_tointeger(L, 2));
	return 0;
}

/***
Get the value of one of the actor's talk flags
@function Actor.get_talk_flag
@tparam Actor actor
@int flag The flag to get. 0..7
@treturn bool returns the flag value
@within Actor
 */
static int nscript_actor_get_talk_flag(lua_State *L) {
	Actor *actor;
	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;
	lua_pushboolean(L, actor->get_flag((uint8)lua_tointeger(L, 2)));

	return 1;
}

/***
Clear one of the actor's talk flags
@function Actor.clear_talk_flag
@tparam Actor actor
@int flag The flag to clear. 0..7
@within Actor
 */
static int nscript_actor_clear_talk_flag(lua_State *L) {
	Actor *actor;
	actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;
	actor->clear_flag((uint8)lua_tointeger(L, 2));
	return 0;
}

/***
Get the number of schedule entries
@function Actor.get_number_of_schedules
@tparam Actor actor
@treturn int
@within Actor
 */
static int nscript_actor_get_number_of_schedules(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	lua_pushinteger(L, actor->get_number_of_schedules());
	return 1;
}

/***
Get an Actor schedule entry
@function Actor.get_schedule
@tparam Actor actor
@int index The index of the schedule to retrieve
@treturn Schedule
@within Actor
 */
static int nscript_actor_get_schedule(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L);
	if (actor == nullptr)
		return 0;

	Schedule *schedule = actor->get_schedule((uint8)lua_tointeger(L, 2));

	lua_newtable(L);
	lua_pushstring(L, "day_of_week");
	lua_pushinteger(L, schedule->day_of_week);
	lua_settable(L, -3);

	lua_pushstring(L, "worktype");
	lua_pushinteger(L, schedule->worktype);
	lua_settable(L, -3);

	lua_pushstring(L, "x");
	lua_pushinteger(L, schedule->x);
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushinteger(L, schedule->y);
	lua_settable(L, -3);

	lua_pushstring(L, "z");
	lua_pushinteger(L, schedule->z);
	lua_settable(L, -3);

	return 1;
}

} // End of namespace Nuvie
} // End of namespace Ultima
