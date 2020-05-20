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

#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"
#include "common/lua/lualib.h"

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/core/tile_manager.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/inventory_view.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/gui/widgets/console.h"
#include "ultima/nuvie/core/cursor.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/script/script_actor.h"
#include "ultima/nuvie/script/script_cutscene.h"
#include "ultima/nuvie/core/magic.h"
#include "ultima/nuvie/files/tmx_map.h"
#include "ultima/nuvie/files/u6_lib_n.h"

namespace Ultima {
namespace Nuvie {

///
// @module script
//

/***
An in-game object
@table Obj
@string[readonly] luatype This returns "obj"
@int x x position
@int y y position
@int z z position
@tparam[readonly] MapCoord| xyz The object's Location in a MapCoord table.
@int obj_n object number
@int frame_n frame number
@int quality
@int qty quantity
@string[readonly] name The object name from the 'look' table.
@string[readonly] look_string A printable look description
     a book
     an elephant
     5 torches
@bool[readonly] on_map Is the object on the map?
@bool[readonly] in_container Is the object in a container?
@field parent (Obj|Actor) The parent of this object. Either an object if this object is in a container. Or an Actor if this object is in an inventory.
@bool[readonly] readied Is this object readied in someone's inventory?
@bool[readonly] stackable Is this object able to be stacked together?
@number[readonly] weight The object's weight
@int[readonly] tile_num The tile number corresponding to the obj_n + frame_num values
@bool[readonly] getable Is this object getable by the player?
@bool ok_to_take Is it considered stealing if the player gets this object?
@int status Object status
@bool invisible Toggle object visibility
@bool[writeonly] temporary Toggle temporary status *writeonly*

 */

/***
A map coordinate
@table MapCoord
@int x
@int y
@int z 0 for the 1024x1024 surface level, 1..5 for the dungeon levels
 */

/***
An actor schedule entry
@table Schedule
@int day_of_week
@int worktype
@int x
@int y
@int z
 */

extern bool nscript_new_actor_var(lua_State *L, uint16 actor_num);

struct ScriptObjRef {
	uint16 refcount;
	iAVLKey key;

	ScriptObjRef() {
		refcount = 0;
		key._int = -1;
	};
};

static iAVLTree *script_obj_list;

static iAVLKey get_iAVLKey(const void *item) {
	return ((const ScriptObjRef *)item)->key;
}

static NuvieIO *g_objlist_file = NULL;

// used for garbage collection.
//returns current object reference count. Or -1 on error.
static sint32 nscript_inc_obj_ref_count(Obj *obj);
static sint32 nscript_dec_obj_ref_count(Obj *obj);

bool nscript_get_location_from_args(lua_State *L, uint16 *x, uint16 *y, uint8 *z, int lua_stack_offset = 1);
Obj *nscript_get_obj_from_args(lua_State *L, int lua_stack_offset);
extern Actor *nscript_get_actor_from_args(lua_State *L, int lua_stack_offset = 1);

void nscript_new_obj_var(lua_State *L, Obj *obj);

inline bool nscript_obj_init_from_obj(lua_State *L, Obj *dst_obj);
inline bool nscript_obj_init_from_args(lua_State *L, int nargs, Obj *s_obj);
static int nscript_obj_newobj(lua_State *L);
int nscript_obj_new(lua_State *L, Obj *obj);
static int nscript_obj_gc(lua_State *L);
static int nscript_obj_get(lua_State *L);
static int nscript_obj_set(lua_State *L);
static int nscript_obj_movetomap(lua_State *L);
static int nscript_obj_movetoinv(lua_State *L);
static int nscript_obj_movetocont(lua_State *L);
static int nscript_obj_use(lua_State *L);
static int nscript_obj_removefromengine(lua_State *L);

static int nscript_container_remove_obj(lua_State *L);

static const luaL_Reg nscript_objlib_f[] = {
	{ "new", nscript_obj_newobj },
	{ "moveToMap", nscript_obj_movetomap },
	{ "moveToInv", nscript_obj_movetoinv },
	{ "moveToCont", nscript_obj_movetocont },
	{ "removeFromCont", nscript_container_remove_obj },
	{ "removeFromEngine", nscript_obj_removefromengine },
	{ "use", nscript_obj_use },

	{ NULL, NULL }
};
static const luaL_Reg nscript_objlib_m[] = {
	{ "__index", nscript_obj_get },
	{ "__newindex", nscript_obj_set },
	{ "__gc", nscript_obj_gc },
	{ NULL, NULL }
};

static int nscript_u6link_gc(lua_State *L);

static const struct luaL_Reg nscript_u6linklib_m[] = {
	{ "__gc", nscript_u6link_gc },
	{ NULL, NULL }
};

static int nscript_u6link_recursive_gc(lua_State *L);

static const struct luaL_Reg nscript_u6linkrecursivelib_m[] = {
	{ "__gc", nscript_u6link_recursive_gc },
	{ NULL, NULL }
};

static int nscript_print(lua_State *L);
static int nscript_clear_scroll(lua_State *L);
static int nscript_display_prompt(lua_State *L);
//no longer used -- static int nscript_get_target(lua_State *L);
static int nscript_load(lua_State *L);

static int nscript_config_get_boolean_value(lua_State *L);
static int nscript_config_get_game_type(lua_State *L);
static int nscript_config_get_language(lua_State *L);

static int nscript_objlist_seek(lua_State *L);
static int nscript_objlist_read1(lua_State *L);
static int nscript_objlist_write1(lua_State *L);
static int nscript_objlist_read2(lua_State *L);
static int nscript_objlist_write2(lua_State *L);

static int nscript_game_get_ui_style(lua_State *L);
static int nscript_player_get_name(lua_State *L);
static int nscript_player_get_gender(lua_State *L);
static int nscript_player_get_location(lua_State *L);
static int nscript_player_get_karma(lua_State *L);
static int nscript_player_set_karma(lua_State *L);
static int nscript_player_dec_alcohol(lua_State *L);
static int nscript_player_move(lua_State *L);
static int nscript_player_set_actor(lua_State *L);
static int nscript_player_is_in_solo_mode(lua_State *L);

static int nscript_party_is_in_combat_mode(lua_State *L);
static int nscript_party_set_combat_mode(lua_State *L);
static int nscript_party_set_party_mode(lua_State *L);
static int nscript_party_move(lua_State *L);
static int nscript_party_use_entrance(lua_State *L);
static int nscript_party_get_size(lua_State *L);
static int nscript_party_get_member(lua_State *L);
static int nscript_party_update_leader(lua_State *L);
static int nscript_party_resurrect_dead_members(lua_State *L);
static int nscript_party_exit_vehicle(lua_State *L);
static int nscript_party_set_in_vehicle(lua_State *L);
static int nscript_party_dismount_from_horses(lua_State *L);
static int nscript_party_show_all(lua_State *L);
static int nscript_party_hide_all(lua_State *L);

static int nscript_timer_set(lua_State *L);
static int nscript_timer_get(lua_State *L);
static int nscript_timer_update_all(lua_State *L);

static int nscript_clock_get_year(lua_State *L);
static int nscript_clock_get_month(lua_State *L);
static int nscript_clock_get_day(lua_State *L);
static int nscript_clock_get_minute(lua_State *L);
static int nscript_clock_get_hour(lua_State *L);
static int nscript_clock_inc(lua_State *L);

static int nscript_wind_set(lua_State *L);
static int nscript_wind_get(lua_State *L);

static int nscript_input_select(lua_State *L);
static int nscript_input_select_integer(lua_State *L);

//obj manager
static int nscript_objs_at_loc(lua_State *L);
static int nscript_find_volcano_obj_near_player(lua_State *L);
static int nscript_map_get_obj(lua_State *L);
static int nscript_map_remove_obj(lua_State *L);
static int nscript_map_is_water(lua_State *L);
static int nscript_map_is_on_screen(lua_State *L);
static int nscript_map_get_impedence(lua_State *L);
static int nscript_map_get_tile_num(lua_State *L);
static int nscript_map_get_dmg_tile_num(lua_State *L);
static int nscript_map_line_test(lua_State *L);
static int nscript_map_line_hit_check(lua_State *L);

static int nscript_map_can_put_actor(lua_State *L);
static int nscript_map_can_put_obj(lua_State *L);
static int nscript_map_enable_temp_actor_cleaning(lua_State *L);

static int nscript_map_export_tmx_files(lua_State *L);

static int nscript_tileset_export(lua_State *L);

static int nscript_tile_get_flag(lua_State *L);
static int nscript_tile_get_description(lua_State *L);

static int nscript_anim_get_number_of_entries(lua_State *L);
static int nscript_anim_get_tile(lua_State *L);
static int nscript_anim_get_first_frame(lua_State *L);
static int nscript_anim_set_first_frame(lua_State *L);
static int nscript_anim_play(lua_State *L);
static int nscript_anim_stop(lua_State *L);

//Misc
static int nscript_new_hit_entities_tbl_var(lua_State *L, ProjectileEffect *effect);
static int nscript_quake_start(lua_State *L);
static int nscript_explosion_start(lua_State *L);
static int nscript_projectile_anim(lua_State *L);
static int nscript_projectile_anim_multi(lua_State *L);
static int nscript_hit_anim(lua_State *L);
static int nscript_usecode_look(lua_State *L);

static int nscript_fade_out(lua_State *L);
static int nscript_fade_in(lua_State *L);
static int nscript_fade_tile(lua_State *L);
static int nscript_black_fade_obj(lua_State *L);

static int nscript_xor_effect(lua_State *L);
static int nscript_xray_effect(lua_State *L);

static int nscript_peer_effect(lua_State *L);
static int nscript_wing_strike_effect(lua_State *L);
static int nscript_hail_storm_effect(lua_State *L);
static int nscript_wizard_eye_effect(lua_State *L);


static int nscript_play_end_sequence(lua_State *L);

static int nscript_play_sfx(lua_State *L);
static int nscript_is_god_mode_enabled(lua_State *L);
static int nscript_set_armageddon(lua_State *L);

static int nscript_mouse_cursor_show(lua_State *L);
static int nscript_mouse_cursor_set_pointer(lua_State *L);

static int nscript_wait(lua_State *L);

static int nscript_mapwindow_center_at_loc(lua_State *L);
static int nscript_mapwindow_get_loc(lua_State *L);
static int nscript_mapwindow_set_loc(lua_State *L);
static int nscript_mapwindow_set_enable_blacking(lua_State *L);

static int nscript_load_text_from_lzc(lua_State *L);

static int nscript_display_text_in_scroll_gump(lua_State *L);

static int nscript_lock_inventory_view(lua_State *L);
static int nscript_unlock_inventory_view(lua_State *L);

//Iterators
int nscript_u6llist_iter(lua_State *L);
int nscript_u6llist_iter_recursive(lua_State *L);
int nscript_party_iter(lua_State *L);

static int nscript_party(lua_State *L);
static int nscript_container(lua_State *L);
int nscript_init_u6link_iter(lua_State *L, U6LList *list, bool is_recursive);

static int nscript_find_obj(lua_State *L);
static int nscript_find_obj_from_area(lua_State *L);

Script *Script::script = NULL;

static int lua_error_handler(lua_State *L) {
	//lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	lua_getglobal(L, "debug");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);
	lua_pushinteger(L, 2);
	lua_call(L, 2, 1);
	return 1;
}

static bool get_tbl_field_sint16(lua_State *L, const char *index, sint16 *field, int table_stack_offset = -2) {
	lua_pushstring(L, index);
	lua_gettable(L, table_stack_offset);

	if (!lua_isnumber(L, -1))
		return false;

	*field = (sint16)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return true;
}

static bool get_tbl_field_as_wrapped_coord(lua_State *L, const char *index, uint16 *field, uint8 map_level, int table_stack_offset = -2) {
	sint16 coord;
	if (get_tbl_field_sint16(L, index, &coord, table_stack_offset) == false) {
		return false;
	}

	*field = wrap_signed_coord(coord, map_level);

	return true;
}

static bool get_tbl_field_uint16(lua_State *L, const char *index, uint16 *field, int table_stack_offset = -2) {
	lua_pushstring(L, index);
	lua_gettable(L, table_stack_offset);

	if (!lua_isnumber(L, -1))
		return false;

	*field = (uint16)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return true;
}

static bool get_tbl_field_uint8(lua_State *L, const char *index, uint8 *field, int table_stack_offset = -2) {
	lua_pushstring(L, index);
	lua_gettable(L, table_stack_offset);

	if (!lua_isnumber(L, -1))
		return false;

	*field = (uint8)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return true;
}

static bool get_tbl_field_string(lua_State *L, const char *index, char *field, uint16 max_len) {
	lua_pushstring(L, index);
	lua_gettable(L, -2);

	if (!lua_isstring(L, -1))
		return false;

	size_t size;
	const char *string = lua_tolstring(L, -1, &size);
	if (size > max_len)
		size = max_len;

	memcpy(field, string, size);
	field[size] = '\0';

	lua_pop(L, 1);
	return true;
}

uint8 ScriptThread::resume_with_location(MapCoord loc) {
	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushinteger(L, loc.x);
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushinteger(L, loc.y);
	lua_settable(L, -3);

	lua_pushstring(L, "z");
	lua_pushinteger(L, loc.z);
	lua_settable(L, -3);

	return resume(1);
}

uint8 ScriptThread::resume_with_direction(uint8 dir) {
	lua_pushinteger(L, dir);

	return resume(1);
}

uint8 ScriptThread::resume_with_spell_num(uint8 spell_num) {
	lua_pushinteger(L, spell_num);

	return resume(1);
}

uint8 ScriptThread::resume_with_obj(Obj *obj) {
	if (obj) {
		nscript_new_obj_var(L, obj);
		return resume(1);
	}

	return resume(0);
}

uint8 ScriptThread::resume_with_nil() {
	return resume(0);
}

uint8 ScriptThread::resume(int narg) {
	const char *s;
	int ret = lua_resume(L, /*NULL,*/ narg);

	state = NUVIE_SCRIPT_ERROR;

	if (ret == 0) {
		lua_gc(L, LUA_GCCOLLECT, 0); //FIXME! How often should we collect the garbage?
		state = NUVIE_SCRIPT_FINISHED;
	} else if (ret == LUA_YIELD) {
		if (lua_gettop(L) >= 1) {
			s = lua_tostring(L, 1);
			if (s) {
				if (!strcmp(s, "target")) {
					state = NUVIE_SCRIPT_GET_TARGET;
				} else if (!strcmp(s, "dir")) {
					state = NUVIE_SCRIPT_GET_DIRECTION;
				} else if (!strcmp(s, "need_dir")) { // input_really_needs_directon()
					Game::get_game()->get_event()->do_not_show_target_cursor = true;
					state = NUVIE_SCRIPT_GET_DIRECTION;
				} else if (!strcmp(s, "spell")) {
					state = NUVIE_SCRIPT_GET_SPELL;
				} else if (!strcmp(s, "inv_obj")) {
					Actor *actor = nscript_get_actor_from_args(L, 2);
					data = actor->get_actor_num();
					state = NUVIE_SCRIPT_GET_INV_OBJ;
				} else if (!strcmp(s, "obj")) {
					state = NUVIE_SCRIPT_GET_OBJ;
				} else if (!strcmp(s, "player_obj")) {
					state = NUVIE_SCRIPT_GET_PLAYER_OBJ;
				} else if (!strcmp(s, "talk")) {
					Actor *actor = nscript_get_actor_from_args(L, 2);
					data = actor->get_actor_num();
					return NUVIE_SCRIPT_TALK_TO_ACTOR;
				} else if (!strcmp(s, "adv_game_time")) {
					if (lua_gettop(L) < 2)
						data = 0;
					data = lua_tointeger(L, 2);
					state = NUVIE_SCRIPT_ADVANCE_GAME_TIME;
				}
			}
		}
	}

	if (state == NUVIE_SCRIPT_ERROR) {
		DEBUG(0, LEVEL_ERROR, "%s\n", lua_tostring(L, lua_gettop(L)));
	}

	return state;
}


Script::Script(Configuration *cfg, GUI *gui, SoundManager *sm, nuvie_game_t type) {
	const char *path;
	size_t len;
	config = cfg;
	gametype = type;
	script = this;
	soundManager = sm;

	script_obj_list = iAVLAllocTree(get_iAVLKey);

	L = luaL_newstate();
	luaL_openlibs(L);

	luaL_newmetatable(L, "nuvie.U6Link");
	luaL_register(L, NULL, nscript_u6linklib_m);

	luaL_newmetatable(L, "nuvie.U6LinkRecursive");
	luaL_register(L, NULL, nscript_u6linkrecursivelib_m);

	luaL_newmetatable(L, "nuvie.Obj");
	//lua_pushvalue(L, -1); //duplicate metatable
	//lua_setfield(L, -2, "__index"); // add __index to metatable
	luaL_register(L, NULL, nscript_objlib_m);

	luaL_register(L, "Obj", nscript_objlib_f);

	lua_pushcfunction(L, nscript_load);
	lua_setglobal(L, "nuvie_load");

	lua_pushcfunction(L, nscript_config_get_boolean_value);
	lua_setglobal(L, "config_get_boolean_value");

	lua_pushcfunction(L, nscript_config_get_game_type);
	lua_setglobal(L, "config_get_game_type");

	lua_pushcfunction(L, nscript_config_get_language);
	lua_setglobal(L, "config_get_language");

	nscript_init_actor(L);
	nscript_init_cutscene(L, cfg, gui, sm);

	lua_pushcfunction(L, nscript_objlist_seek);
	lua_setglobal(L, "objlist_seek");

	lua_pushcfunction(L, nscript_objlist_read1);
	lua_setglobal(L, "objlist_read1");

	lua_pushcfunction(L, nscript_objlist_write1);
	lua_setglobal(L, "objlist_write1");

	lua_pushcfunction(L, nscript_objlist_read2);
	lua_setglobal(L, "objlist_read2");

	lua_pushcfunction(L, nscript_objlist_write2);
	lua_setglobal(L, "objlist_write2");

	lua_pushcfunction(L, nscript_clear_scroll);
	lua_setglobal(L, "clear_scroll");

	lua_pushcfunction(L, nscript_print);
	lua_setglobal(L, "print");

	lua_pushcfunction(L, nscript_display_prompt);
	lua_setglobal(L, "display_prompt");

	lua_pushcfunction(L, nscript_input_select);
	lua_setglobal(L, "input_select");

	lua_pushcfunction(L, nscript_input_select_integer);
	lua_setglobal(L, "input_select_integer");

	lua_pushcfunction(L, nscript_play_end_sequence);
	lua_setglobal(L, "play_end_sequence");

	lua_pushcfunction(L, nscript_play_sfx);
	lua_setglobal(L, "play_sfx");

	lua_pushcfunction(L, nscript_party);
	lua_setglobal(L, "party_members");

	lua_pushcfunction(L, nscript_container);
	lua_setglobal(L, "container_objs");

	lua_pushcfunction(L, nscript_find_obj);
	lua_setglobal(L, "find_obj");

	lua_pushcfunction(L, nscript_find_obj_from_area);
	lua_setglobal(L, "find_obj_from_area");

	lua_pushcfunction(L, nscript_timer_set);
	lua_setglobal(L, "timer_set");

	lua_pushcfunction(L, nscript_timer_get);
	lua_setglobal(L, "timer_get");

	lua_pushcfunction(L, nscript_timer_update_all);
	lua_setglobal(L, "timer_update_all");

	lua_pushcfunction(L, nscript_clock_get_year);
	lua_setglobal(L, "clock_get_year");

	lua_pushcfunction(L, nscript_clock_get_month);
	lua_setglobal(L, "clock_get_month");

	lua_pushcfunction(L, nscript_clock_get_day);
	lua_setglobal(L, "clock_get_day");

	lua_pushcfunction(L, nscript_clock_get_minute);
	lua_setglobal(L, "clock_get_minute");

	lua_pushcfunction(L, nscript_clock_get_hour);
	lua_setglobal(L, "clock_get_hour");

	lua_pushcfunction(L, nscript_clock_inc);
	lua_setglobal(L, "clock_inc");

	lua_pushcfunction(L, nscript_wind_set);
	lua_setglobal(L, "wind_set_dir");

	lua_pushcfunction(L, nscript_wind_get);
	lua_setglobal(L, "wind_get_dir");

	lua_pushcfunction(L, nscript_tile_get_flag);
	lua_setglobal(L, "tile_get_flag");

	lua_pushcfunction(L, nscript_tile_get_description);
	lua_setglobal(L, "tile_get_description");

	lua_pushcfunction(L, nscript_anim_get_number_of_entries);
	lua_setglobal(L, "anim_get_number_of_entries");

	lua_pushcfunction(L, nscript_anim_get_tile);
	lua_setglobal(L, "anim_get_tile");

	lua_pushcfunction(L, nscript_anim_set_first_frame);
	lua_setglobal(L, "anim_set_first_frame");

	lua_pushcfunction(L, nscript_anim_get_first_frame);
	lua_setglobal(L, "anim_get_first_frame");

	lua_pushcfunction(L, nscript_anim_play);
	lua_setglobal(L, "anim_play");

	lua_pushcfunction(L, nscript_anim_stop);
	lua_setglobal(L, "anim_stop");

	lua_pushcfunction(L, nscript_objs_at_loc);
	lua_setglobal(L, "objs_at_loc");

	lua_pushcfunction(L, nscript_find_volcano_obj_near_player);
	lua_setglobal(L, "find_volcano_near_player");

	lua_pushcfunction(L, nscript_map_get_obj);
	lua_setglobal(L, "map_get_obj");

	lua_pushcfunction(L, nscript_map_remove_obj);
	lua_setglobal(L, "map_remove_obj");

	lua_pushcfunction(L, nscript_map_is_water);
	lua_setglobal(L, "map_is_water");

	lua_pushcfunction(L, nscript_map_is_on_screen);
	lua_setglobal(L, "map_is_on_screen");

	lua_pushcfunction(L, nscript_map_get_impedence);
	lua_setglobal(L, "map_get_impedence");

	lua_pushcfunction(L, nscript_map_get_tile_num);
	lua_setglobal(L, "map_get_tile_num");

	lua_pushcfunction(L, nscript_map_get_dmg_tile_num);
	lua_setglobal(L, "map_get_dmg_tile_num");

	lua_pushcfunction(L, nscript_map_can_put_actor);
	lua_setglobal(L, "map_can_put");

	lua_pushcfunction(L, nscript_map_can_put_obj);
	lua_setglobal(L, "map_can_put_obj");

	lua_pushcfunction(L, nscript_map_enable_temp_actor_cleaning);
	lua_setglobal(L, "map_enable_temp_actor_cleaning");

	lua_pushcfunction(L, nscript_map_line_test);
	lua_setglobal(L, "map_can_reach_point");

	lua_pushcfunction(L, nscript_map_line_hit_check);
	lua_setglobal(L, "map_line_hit_check");

	lua_pushcfunction(L, nscript_map_export_tmx_files);
	lua_setglobal(L, "map_export_tmx_files");

	lua_pushcfunction(L, nscript_tileset_export);
	lua_setglobal(L, "tileset_export");

	lua_pushcfunction(L, nscript_game_get_ui_style);
	lua_setglobal(L, "game_get_ui_style");

	lua_pushcfunction(L, nscript_player_get_name);
	lua_setglobal(L, "player_get_name");

	lua_pushcfunction(L, nscript_player_get_gender);
	lua_setglobal(L, "player_get_gender");

	lua_pushcfunction(L, nscript_player_get_location);
	lua_setglobal(L, "player_get_location");

	lua_pushcfunction(L, nscript_player_get_karma);
	lua_setglobal(L, "player_get_karma");

	lua_pushcfunction(L, nscript_player_set_karma);
	lua_setglobal(L, "player_set_karma");

	lua_pushcfunction(L, nscript_player_dec_alcohol);
	lua_setglobal(L, "player_dec_alcohol");

	lua_pushcfunction(L, nscript_player_move);
	lua_setglobal(L, "player_move");

	lua_pushcfunction(L, nscript_player_set_actor);
	lua_setglobal(L, "player_set_actor");

	lua_pushcfunction(L, nscript_player_is_in_solo_mode);
	lua_setglobal(L, "player_is_in_solo_mode");

	lua_pushcfunction(L, nscript_party_get_size);
	lua_setglobal(L, "party_get_size");

	lua_pushcfunction(L, nscript_party_get_member);
	lua_setglobal(L, "party_get_member");

	lua_pushcfunction(L, nscript_party_is_in_combat_mode);
	lua_setglobal(L, "party_is_in_combat_mode");

	lua_pushcfunction(L, nscript_party_set_combat_mode);
	lua_setglobal(L, "party_set_combat_mode");

	lua_pushcfunction(L, nscript_party_set_party_mode);
	lua_setglobal(L, "party_set_party_mode");

	lua_pushcfunction(L, nscript_party_move);
	lua_setglobal(L, "party_move");

	lua_pushcfunction(L, nscript_party_use_entrance);
	lua_setglobal(L, "party_use_entrance");

	lua_pushcfunction(L, nscript_party_update_leader);
	lua_setglobal(L, "party_update_leader");

	lua_pushcfunction(L, nscript_party_resurrect_dead_members);
	lua_setglobal(L, "party_resurrect_dead_members");

	lua_pushcfunction(L, nscript_party_exit_vehicle);
	lua_setglobal(L, "party_exit_vehicle");

	lua_pushcfunction(L, nscript_party_set_in_vehicle);
	lua_setglobal(L, "party_set_in_vehicle");

	lua_pushcfunction(L, nscript_party_dismount_from_horses);
	lua_setglobal(L, "party_dismount_from_horses");

	lua_pushcfunction(L, nscript_party_show_all);
	lua_setglobal(L, "party_show_all");

	lua_pushcfunction(L, nscript_party_hide_all);
	lua_setglobal(L, "party_hide_all");

	lua_pushcfunction(L, nscript_quake_start);
	lua_setglobal(L, "quake_start");

	lua_pushcfunction(L, nscript_explosion_start);
	lua_setglobal(L, "explosion_start");

	lua_pushcfunction(L, nscript_projectile_anim);
	lua_setglobal(L, "projectile_anim");

	lua_pushcfunction(L, nscript_projectile_anim_multi);
	lua_setglobal(L, "projectile_anim_multi");

	lua_pushcfunction(L, nscript_hit_anim);
	lua_setglobal(L, "hit_anim");

	lua_pushcfunction(L, nscript_usecode_look);
	lua_setglobal(L, "usecode_look");

	lua_pushcfunction(L, nscript_fade_out);
	lua_setglobal(L, "fade_out");

	lua_pushcfunction(L, nscript_fade_in);
	lua_setglobal(L, "fade_in");

	lua_pushcfunction(L, nscript_fade_tile);
	lua_setglobal(L, "fade_tile");

	lua_pushcfunction(L, nscript_black_fade_obj);
	lua_setglobal(L, "fade_obj");

	lua_pushcfunction(L, nscript_xor_effect);
	lua_setglobal(L, "xor_effect");

	lua_pushcfunction(L, nscript_xray_effect);
	lua_setglobal(L, "xray_effect");

	lua_pushcfunction(L, nscript_peer_effect);
	lua_setglobal(L, "peer_effect");

	lua_pushcfunction(L, nscript_wing_strike_effect);
	lua_setglobal(L, "wing_strike_effect");

	lua_pushcfunction(L, nscript_hail_storm_effect);
	lua_setglobal(L, "hail_storm_effect");

	lua_pushcfunction(L, nscript_wizard_eye_effect);
	lua_setglobal(L, "wizard_eye_effect");

	lua_pushcfunction(L, nscript_is_god_mode_enabled);
	lua_setglobal(L, "is_god_mode_enabled");

	lua_pushcfunction(L, nscript_set_armageddon);
	lua_setglobal(L, "set_armageddon");

	lua_pushcfunction(L, nscript_mouse_cursor_show);
	lua_setglobal(L, "mouse_cursor_visible");

	lua_pushcfunction(L, nscript_mouse_cursor_set_pointer);
	lua_setglobal(L, "mouse_cursor_set_pointer");

	lua_pushcfunction(L, nscript_wait);
	lua_setglobal(L, "script_wait");

	lua_pushcfunction(L, nscript_mapwindow_center_at_loc);
	lua_setglobal(L, "mapwindow_center_at_location");

	lua_pushcfunction(L, nscript_mapwindow_get_loc);
	lua_setglobal(L, "mapwindow_get_location");

	lua_pushcfunction(L, nscript_mapwindow_set_loc);
	lua_setglobal(L, "mapwindow_set_location");

	lua_pushcfunction(L, nscript_mapwindow_set_enable_blacking);
	lua_setglobal(L, "mapwindow_set_enable_blacking");

	lua_pushcfunction(L, nscript_load_text_from_lzc);
	lua_setglobal(L, "load_text_from_lzc");

	lua_pushcfunction(L, nscript_display_text_in_scroll_gump);
	lua_setglobal(L, "display_text_in_scroll_gump");

	lua_pushcfunction(L, nscript_lock_inventory_view);
	lua_setglobal(L, "lock_inventory_view");

	lua_pushcfunction(L, nscript_unlock_inventory_view);
	lua_setglobal(L, "unlock_inventory_view");

	seed_random();

	lua_getglobal(L, "package");
	lua_pushstring(L, "path");
	lua_gettable(L, -2);

	path = lua_tolstring(L, -1, &len);
	DEBUG(0, LEVEL_INFORMATIONAL, "lua path = %s\n", path);

}

Script::~Script() {
	if (L)
		lua_close(L);
}

bool Script::init() {
	Std::string dir, path;
	config->value("config/datadir", dir, "");
	build_path(dir, "scripts", path);
	dir = path;

	Std::string game_tag = get_game_tag(gametype);
	stringToLower(game_tag);

	build_path(dir, game_tag, path);

	dir = path;
	build_path(dir, "init.lua", path);
	ConsoleAddInfo("Loading init.lua");

	Std::string init_str = "init = nuvie_load(\"";
	init_str.append(game_tag);
	init_str.append("/init.lua\"); init()");

	if (run_script(init_str.c_str()) == false) {
		Std::string errorStr = "Loading ";
		errorStr.append(path);
		ConsoleAddError(errorStr);
		return false;
	}

	return true;
}

void Script::seed_random() {
	//Seed the lua random number generator.
	//seed with a random number from NUVIE_RAND()
	//this should be seeded at this point.

	//lua_getfield(L, LUA_GLOBALSINDEX, "math");
	lua_getglobal(L, "math");
	lua_getfield(L, -1, "randomseed");
	lua_remove(L, -2);
	lua_pushnumber(L, NUVIE_RAND());
	lua_pcall(L, 1, 0, 0);

	return;
}

bool Script::run_script(const char *scriptStr) {
	if (luaL_dostring(L, scriptStr) != 0) {
		DEBUG(0, LEVEL_ERROR, "Script Error: %s\n", luaL_checkstring(L, -1));
		return false;
	}

	return true;
}

bool Script::play_cutscene(const char *script_file) {
	string script_file_path = "";
	config->value("config/GameID", script_file_path);
	script_file_path += script_file;

	ConsoleHide();

	return run_lua_file(script_file_path.c_str());
}

MovementStatus Script::call_player_before_move_action(sint16 *rel_x, sint16 *rel_y) {
	lua_getglobal(L, "player_before_move_action");
	lua_pushinteger(L, *rel_x);
	lua_pushinteger(L, *rel_y);

	if (call_function("player_before_move_action", 2, 3)) {
		if (!lua_isnil(L, -2)) {
			*rel_x = (sint16)lua_tointeger(L, -2);
		}
		if (!lua_isnil(L, -1)) {
			*rel_y = (sint16)lua_tointeger(L, -1);
		}

		switch (lua_tointeger(L, -3)) {
		case 0 :
			return CAN_MOVE;
		case 1 :
			return BLOCKED;
		case 2 :
			return FORCE_MOVE;
		default :
			break;
		}
	}
	return CAN_MOVE;
}


bool Script::call_player_post_move_action(bool didMove) {
	lua_getglobal(L, "player_post_move_action");
	lua_pushboolean(L, didMove);

	return call_function("player_post_move_action", 1, 0);
}

bool Script::call_player_pass() {
	lua_getglobal(L, "player_pass");

	return call_function("player_pass", 0, 0);
}

bool Script::call_actor_update_all() {
	lua_getglobal(L, "actor_update_all");

	return call_function("actor_update_all", 0, 0);
}

bool Script::call_actor_init(Actor *actor, uint8 alignment) {
	lua_getglobal(L, "actor_init");
	nscript_new_actor_var(L, actor->get_actor_num());
	lua_pushinteger(L, alignment);

	return call_function("actor_init", 2, 0);
}

bool Script::call_actor_attack(Actor *actor, MapCoord location, Obj *weapon, Actor *foe) {
	lua_getglobal(L, "actor_attack");
	nscript_new_actor_var(L, actor->get_actor_num());
	//nscript_new_actor_var(L, foe->get_actor_num());
	uint8 num_arg = 6;
	lua_pushnumber(L, (lua_Number)location.x);
	lua_pushnumber(L, (lua_Number)location.y);
	lua_pushnumber(L, (lua_Number)location.z);
	if (weapon == NULL)
		nscript_new_actor_var(L, actor->get_actor_num());
	else
		nscript_obj_new(L, weapon);
	if (foe == NULL)
		num_arg = 5;
	else
		nscript_new_actor_var(L, foe->get_actor_num());

	if (call_function("actor_attack", num_arg, 0) == false) {
		return false;
	}

	Game::get_game()->get_map_window()->updateBlacking(); // the script might have updated the blocking objects. eg broken a door.
	return true;
}

bool Script::call_load_game(NuvieIO *objlist) {
	return call_loadsave_game("load_game", objlist);
}

bool Script::call_save_game(NuvieIO *objlist) {
	return call_loadsave_game("save_game", objlist);
}

bool Script::call_loadsave_game(const char *function, NuvieIO *objlist) {
	g_objlist_file = objlist;
	lua_getglobal(L, function);

	bool result = call_function(function, 0, 0);

	g_objlist_file = NULL;
	return result;
}

bool Script::call_actor_map_dmg(Actor *actor, MapCoord location) {

	lua_getglobal(L, "actor_map_dmg");
	nscript_new_actor_var(L, actor->get_actor_num());
	//nscript_new_actor_var(L, foe->get_actor_num());
	lua_pushnumber(L, (lua_Number)location.x);
	lua_pushnumber(L, (lua_Number)location.y);
	lua_pushnumber(L, (lua_Number)location.z);

	return call_function("actor_map_dmg", 4, 0);
}

bool Script::call_actor_tile_dmg(Actor *actor, uint16 tile_num) {

	lua_getglobal(L, "actor_tile_dmg");
	nscript_new_actor_var(L, actor->get_actor_num());
	lua_pushnumber(L, (lua_Number)tile_num);


	return call_function("actor_tile_dmg", 2, 0);
}

bool Script::call_actor_hit(Actor *actor, uint8 dmg, bool display_hit_msg) {
	lua_getglobal(L, "actor_hit");
	nscript_new_actor_var(L, actor->get_actor_num());
	lua_pushnumber(L, (lua_Number)dmg);

	if (call_function("actor_hit", 2, 0) == false)
		return false;

	if (display_hit_msg) {
		lua_getglobal(L, "actor_hit_msg");
		nscript_new_actor_var(L, actor->get_actor_num());

		return call_function("actor_hit_msg", 1, 0);
	}

	return true;
}

bool Script::call_look_obj(Obj *obj) {
	lua_getglobal(L, "look_obj");

	nscript_obj_new(L, obj);

	if (call_function("look_obj", 1, 1) == false)
		return false;

	return lua_toboolean(L, -1);
}

int Script::call_obj_get_readiable_location(Obj *obj) {
	lua_getglobal(L, "obj_get_readiable_location");

	nscript_obj_new(L, obj);

	if (call_function("obj_get_readiable_location", 1, 1) == false)
		return -1;

	return lua_tointeger(L, -1);
}

uint8 Script::actor_get_max_magic_points(Actor *actor) {
	lua_getglobal(L, "actor_get_max_magic_points");
	nscript_new_actor_var(L, actor->get_actor_num());

	if (call_function("actor_get_max_magic_points", 1, 1) == false)
		return 0;
	return (uint8)lua_tointeger(L, -1);
}

bool Script::call_actor_get_obj(Actor *actor, Obj *obj, Obj *container) {
	int num_args = 2;
	lua_getglobal(L, "actor_get_obj");
	nscript_new_actor_var(L, actor->get_actor_num());
	nscript_obj_new(L, obj);

	if (container) {
		nscript_obj_new(L, container);
		num_args++;
	}
	if (call_function("actor_get_obj", num_args, 1) == false)
		return false;

	return lua_toboolean(L, -1);
}

bool Script::call_actor_subtract_movement_points(Actor *actor, uint8 points) {
	lua_getglobal(L, "subtract_movement_pts");
	nscript_new_actor_var(L, actor->get_actor_num());
	lua_pushnumber(L, (lua_Number)points);

	if (call_function("subtract_movement_pts", 2, 0) == false)
		return false;

	return true;
}

bool Script::call_actor_resurrect(Actor *actor) {
	lua_getglobal(L, "actor_resurrect");
	nscript_new_actor_var(L, actor->get_actor_num());

	if (call_function("actor_resurrect", 1, 0) == false)
		return false;

	return true;
}

uint8 Script::call_actor_str_adj(Actor *actor) {
	lua_getglobal(L, "actor_str_adj");
	nscript_new_actor_var(L, actor->get_actor_num());

	if (call_function("actor_str_adj", 1, 1) == false)
		return 0;
	return (uint8)lua_tointeger(L, -1);
}

uint8 Script::call_actor_dex_adj(Actor *actor) {
	lua_getglobal(L, "actor_dex_adj");
	nscript_new_actor_var(L, actor->get_actor_num());

	if (call_function("actor_dex_adj", 1, 1) == false)
		return 0;
	return (uint8)lua_tointeger(L, -1);
}

uint8 Script::call_actor_int_adj(Actor *actor) {
	lua_getglobal(L, "actor_int_adj");
	nscript_new_actor_var(L, actor->get_actor_num());

	if (call_function("actor_int_adj", 1, 1) == false)
		return 0;
	return (uint8)lua_tointeger(L, -1);
}

bool Script::call_use_keg(Obj *obj) {
	lua_getglobal(L, "use_keg");

	nscript_obj_new(L, obj);

	if (call_function("use_keg", 1, 0) == false)
		return false;

	return true;
}

bool Script::call_has_usecode(Obj *obj, UseCodeEvent usecode_type) {
	lua_getglobal(L, "has_usecode");

	nscript_obj_new(L, obj);
	lua_pushnumber(L, (lua_Number)usecode_type);

	if (call_function("has_usecode", 2, 1) == false)
		return false;

	return lua_toboolean(L, -1);
}

ScriptThread *Script::call_use_obj(Obj *obj, Actor *actor) {
	ScriptThread *t = NULL;
	lua_State *s;

	s = lua_newthread(L);

	lua_getglobal(s, "use_obj");

	nscript_obj_new(s, obj);
	nscript_new_actor_var(s, actor->get_actor_num());

	//FIXME wrap stacktrace dumping logic here as per call_function method.

	t = new ScriptThread(s, 2);
	//if(nscript_call_function(L, "use_obj", 2, 0, true) == false)
	//  return false;

	return t;
}

bool Script::call_ready_obj(Obj *obj, Actor *actor) {
	lua_getglobal(L, "ready_obj");

	nscript_obj_new(L, obj);
	nscript_new_actor_var(L, actor->get_actor_num());

	if (call_function("ready_obj", 2, 1) == false)
		return false;

	return lua_toboolean(L, -1);
}

bool Script::call_move_obj(Obj *obj, sint16 rel_x, sint16 rel_y) {
	lua_getglobal(L, "move_obj");

	nscript_obj_new(L, obj);
	lua_pushnumber(L, (lua_Number)rel_x);
	lua_pushnumber(L, (lua_Number)rel_y);

	if (call_function("move_obj", 3, 1) == false)
		return false;

	return lua_toboolean(L, -1);
}

bool Script::call_handle_alt_code(uint16 altcode) {
	lua_getglobal(L, "handle_alt_code");
	lua_pushnumber(L, (lua_Number)altcode);

	if (call_function("handle_alt_code", 1, 0) == false)
		return false;

	return true;
}

bool Script::call_magic_get_spell_list(Spell **spell_list) {
	lua_getglobal(L, "magic_get_spell_list");

	if (call_function("magic_get_spell_list", 0, 1) == false)
		return false;

	for (int i = 1;; i++) {
		lua_pushinteger(L, i);
		lua_gettable(L, -2);

		if (!lua_istable(L, -1)) { //we've hit the end of our targets
			::debug(1, "end = %d", i);
			lua_pop(L, 1);
			break;
		}

		uint16 num;
		uint8 re;
		char name[13];
		char invocation[5];

		get_tbl_field_uint16(L, "spell_num", &num);
		get_tbl_field_uint8(L, "reagents", &re);
		get_tbl_field_string(L, "name", name, 12);
		get_tbl_field_string(L, "invocation", invocation, 4);

		if (num < 256 && spell_list[num] == NULL) {
			spell_list[num] = new Spell((uint8)num, (const char *)name, (const char *)invocation, re);
			::debug(1, "num = %d, reagents = %d, name = %s invocation = %s\n", num, re, name, invocation);
		}

		lua_pop(L, 1);
	}

	return true;
}

bool Script::call_actor_use_effect(Obj *effect_obj, Actor *actor) {
	lua_getglobal(L, "actor_use_effect");
	nscript_new_actor_var(L, actor->get_actor_num());
	nscript_obj_new(L, effect_obj);

	return call_function("actor_use_effect", 2, 0);
}

bool Script::call_can_get_obj_override(Obj *obj) {
	lua_getglobal(L, "can_get_obj_override");
	nscript_obj_new(L, obj);

	if (call_function("can_get_obj_override", 1, 1) == false)
		return false;

	return lua_toboolean(L, -1);
}

bool Script::call_out_of_ammo(Actor *attacker, Obj *weapon, bool print_message) {
	lua_getglobal(L, "out_of_ammo");
	nscript_new_actor_var(L, attacker->get_actor_num());
	if (weapon == NULL)
		nscript_new_actor_var(L, attacker->get_actor_num());
	else
		nscript_obj_new(L, weapon);
	lua_pushboolean(L, print_message);

	if (call_function("out_of_ammo", 3, 1) == false)
		return false;

	return lua_toboolean(L, -1);
}

bool Script::call_is_avatar_dead() {
	lua_getglobal(L, "is_avatar_dead");
	if (call_function("is_avatar_dead", 0, 1) == false)
		return false;
	return lua_toboolean(L, -1);
}

bool Script::call_is_ranged_select(UseCodeType operation) {
	lua_getglobal(L, "is_ranged_select");
	lua_pushstring(L, useCodeTypeToString(operation));

	if (call_function("is_ranged_select", 1, 1) == false)
		return false;
	return lua_toboolean(L, -1);
}

bool Script::call_function(const char *func_name, int num_args, int num_return, bool print_stacktrace) {
	int start_idx = lua_gettop(L);
	int error_index = 0;

	if (print_stacktrace) {
		error_index = lua_gettop(L) - num_args;
		lua_pushcfunction(L, lua_error_handler);
		lua_insert(L, error_index);
	}

	int result = lua_pcall(L, num_args, num_return, error_index);
	if (result != 0) {
		DEBUG(0, LEVEL_ERROR, "Script Error: %s(), %s\n", func_name, luaL_checkstring(L, -1));
		lua_pop(L, 1);
	}

	if (print_stacktrace) {
		lua_remove(L, error_index);
	}

	if (lua_gettop(L) + num_args + 1 != start_idx + num_return)
		DEBUG(0, LEVEL_ERROR, "lua stack error!");

	return (result != 0) ? false : true;
}

ScriptThread *Script::call_function_in_thread(const char *function_name) {
	ScriptThread *t = NULL;
	lua_State *s;

	s = lua_newthread(L);

	lua_getglobal(s, function_name);

	//FIXME wrap stacktrace dumping logic here as per call_function method.

	t = new ScriptThread(s, 0);

	return t;
}

bool Script::run_lua_file(const char *filename) {
	Std::string dir, path;
	Script::get_script()->get_config()->value("config/datadir", dir, "");

	build_path(dir, "scripts", path);
	dir = path;
	build_path(dir, filename, path);

	if (luaL_loadfile(L, path.c_str()) != 0) {
		DEBUG(0, LEVEL_ERROR, "loading script file %s", path.c_str());
		return false;
	}

	return call_function(path.c_str(), 0, 0);
}

bool Script::call_moonstone_set_loc(uint8 phase, MapCoord location) {
	lua_getglobal(L, "moonstone_set_loc");

	lua_pushnumber(L, (lua_Number)phase);
	lua_pushnumber(L, (lua_Number)location.x);
	lua_pushnumber(L, (lua_Number)location.y);
	lua_pushnumber(L, (lua_Number)location.z);

	return call_function("moonstone_set_loc", 4, 0);
}

MapCoord Script::call_moonstone_get_loc(uint8 phase) {
	MapCoord loc(0, 0, 0);

	lua_getglobal(L, "moonstone_get_loc");

	lua_pushnumber(L, (lua_Number)phase);

	if (call_function("moonstone_get_loc", 1, 1) == false)
		return loc;

	get_tbl_field_uint16(L, "x", &loc.x);
	get_tbl_field_uint16(L, "y", &loc.y);
	get_tbl_field_uint8(L, "z", &loc.z);

	return loc;
}

bool Script::call_update_moongates(bool visible) {
	lua_getglobal(L, "update_moongates");

	lua_pushboolean(L, visible);

	return call_function("update_moongates", 1, 0);
}

bool Script::call_advance_time(uint16 minutes) {
	lua_getglobal(L, "advance_time");

	lua_pushnumber(L, (lua_Number)minutes);

	return call_function("advance_time", 1, 0);
}

bool Script::call_set_g_show_stealing(bool stealing) {
	lua_getglobal(L, "set_g_show_stealing");
	lua_pushboolean(L, stealing);

	if (call_function("set_g_show_stealing", 1, 0) == false)
		return false;
	return true;
}

uint8 Script::call_get_combat_range(uint16 absx, uint16 absy) {
	lua_getglobal(L, "get_combat_range");
	lua_pushnumber(L, (lua_Number)absx);
	lua_pushnumber(L, (lua_Number)absy);
	if (call_function("get_combat_range", 2, 1) == false)
		return 9;
	return (uint8)lua_tointeger(L, -1);
}

uint8 Script::call_get_weapon_range(uint16 obj_n) {
	lua_getglobal(L, "get_weapon_range");
	lua_pushnumber(L, (lua_Number)obj_n);
	if (call_function("get_weapon_range", 1, 1) == false)
		return 1;
	return (uint8)lua_tointeger(L, -1);
}

uint8 Script::call_play_midgame_sequence(uint16 seq_num) {
	lua_getglobal(L, "play_midgame_sequence");
	lua_pushnumber(L, (lua_Number)seq_num);
	if (call_function("play_midgame_sequence", 1, 1) == false)
		return 1;
	return (uint8)lua_tointeger(L, -1);
}

bool Script::call_talk_script(uint8 script_number) {
	lua_getglobal(L, "talk_script");
	lua_pushnumber(L, (lua_Number)script_number);
	if (call_function("talk_script", 1, 0) == false)
		return false;
	return true;
}

bool Script::call_talk_to_obj(Obj *obj) {
	lua_getglobal(L, "talk_to_obj");

	nscript_obj_new(L, obj);

	if (call_function("talk_to_obj", 1, 1) == false)
		return false;

	return (bool)lua_toboolean(L, -1);
}

bool Script::call_talk_to_actor(Actor *actor) {
	lua_getglobal(L, "talk_to_actor");

	nscript_new_actor_var(L, actor->get_actor_num());

	if (call_function("talk_to_actor", 1, 1) == false)
		return false;

	return (bool)lua_toboolean(L, -1);
}

bool Script::call_is_container_obj(uint16 obj_n) {
	lua_getglobal(L, "is_container_obj");
	lua_pushnumber(L, (lua_Number)obj_n);
	call_function("is_container_object", 1, 1);
	return (bool)lua_toboolean(L, -1);
}

uint8 Script::call_get_portrait_number(Actor *actor) {
	lua_getglobal(L, "get_portrait_number");
	nscript_new_actor_var(L, actor->get_actor_num());
	if (call_function("get_portrait_number", 1, 1) == false)
		return 1;
	return (uint8)lua_tointeger(L, -1);
}

bool Script::call_player_attack() {
	lua_getglobal(L, "player_attack");

	return call_function("player_attack", 0, 0);
}

uint16 Script::call_get_tile_to_object_mapping(uint16 tile_n) {
	lua_getglobal(L, "get_tile_to_object_mapping");
	lua_pushnumber(L, (lua_Number)tile_n);
	call_function("get_tile_to_object_mapping", 1, 1);

	return ((uint)lua_tonumber(L, -1));
}

bool Script::call_is_tile_object(uint16 obj_n) {
	lua_getglobal(L, "is_tile_object");
	lua_pushnumber(L, (lua_Number)obj_n);
	call_function("is_tile_object", 1, 1);
	return (lua_toboolean(L, -1));
}

ScriptThread *Script::new_thread(const char *scriptfile) {
	ScriptThread *t = NULL;
	lua_State *s;

	s = lua_newthread(L);
	lua_getglobal(s, "run_script");
	lua_pushstring(s, scriptfile);

	t = new ScriptThread(s, 1);

	return t;
}

ScriptThread *Script::new_thread_from_string(const char *scriptStr) {
	ScriptThread *t = NULL;
	lua_State *s;

	s = lua_newthread(L);

	if (luaL_loadbuffer(s, scriptStr, strlen(scriptStr), "nuvie") != 0)
		return NULL;

	t = new ScriptThread(s, 0);

	return t;
}

bool nscript_get_location_from_args(lua_State *L, uint16 *x, uint16 *y, uint8 *z, int lua_stack_offset) {
	if (lua_istable(L, lua_stack_offset)) {
		if (!get_tbl_field_uint8(L, "z", z, lua_stack_offset)) return false;
		if (!get_tbl_field_as_wrapped_coord(L, "x", x, *z, lua_stack_offset)) return false;
		if (!get_tbl_field_as_wrapped_coord(L, "y", y, *z, lua_stack_offset)) return false;
	} else {
		if (lua_isnil(L, lua_stack_offset)) return false;
		*z = (uint8)luaL_checkinteger(L,  lua_stack_offset + 2);
		*x = wrap_signed_coord((sint16)luaL_checkinteger(L, lua_stack_offset), *z);
		*y = wrap_signed_coord((sint16)luaL_checkinteger(L, lua_stack_offset + 1), *z);
	}

	return true;
}

Obj *nscript_get_obj_from_args(lua_State *L, int lua_stack_offset) {
	Obj **s_obj = (Obj **)luaL_checkudata(L, lua_stack_offset, "nuvie.Obj");
	if (s_obj == NULL)
		return NULL;

	return *s_obj;
}

void nscript_new_obj_var(lua_State *L, Obj *obj) {
	Obj **p_obj;
	p_obj = (Obj **)lua_newuserdata(L, sizeof(Obj *));

	luaL_getmetatable(L, "nuvie.Obj");
	lua_setmetatable(L, -2);

	*p_obj = obj;

	nscript_inc_obj_ref_count(obj);
}

/***
Create a new Obj.
This function can clone and existing object or create a new object from one or more parameters.
@function Obj.new
@tparam[opt] Obj obj Object to clone
@int[opt] obj_n Object number
@int[opt] frame_n Frame number
@int[opt] quality Quality
@int[opt] qty Quantity
@int[opt] x x map position
@int[opt] y y map position
@int[opt] z z map position
@treturn Obj The newly created object
@within Object
 */
static int nscript_obj_newobj(lua_State *L) {
	return nscript_obj_new(L, NULL);
}

int nscript_obj_new(lua_State *L, Obj *obj) {
	Obj **p_obj;

	p_obj = (Obj **)lua_newuserdata(L, sizeof(Obj *));

	luaL_getmetatable(L, "nuvie.Obj");
	lua_setmetatable(L, -2);

	if (obj == NULL) {
		obj = new Obj();

		if (lua_gettop(L) > 1) { // do we have arguments?
			if (lua_isuserdata(L, 1)) { // do we have an obj
				if (nscript_obj_init_from_obj(L, obj) == false)
					return 0;
			} else { // init object from arguments
				if (nscript_obj_init_from_args(L, lua_gettop(L) - 1, obj) == false)
					return 0;
			}
		}
	}

	*p_obj = obj;

	nscript_inc_obj_ref_count(obj);

	return 1;
}

sint32 nscript_inc_obj_ref_count(Obj *obj) {
	ScriptObjRef *obj_ref;
	iAVLKey key;
	key._ptr = obj;

	obj_ref = (ScriptObjRef *)iAVLSearch(script_obj_list, key);
	if (obj_ref == NULL) {
		obj->set_in_script(true); // mark as being used by script engine.
		obj_ref =  new ScriptObjRef();
		obj_ref->key._ptr = obj;
		iAVLInsert(script_obj_list, obj_ref);
	}

	obj_ref->refcount++;

	return (sint32)obj_ref->refcount;
}

sint32 nscript_dec_obj_ref_count(Obj *obj) {
	ScriptObjRef *obj_ref;
	iAVLKey key;
	key._ptr = obj;

	obj_ref = (ScriptObjRef *)iAVLSearch(script_obj_list, key);
	if (obj_ref == NULL)
		return -1;


	obj_ref->refcount--;

	if (obj_ref->refcount == 0) {
		iAVLDelete(script_obj_list, key);
		delete obj_ref;
		obj->set_in_script(false); //nolonger being referenced by the script engine.
		return 0;
	}

	return obj_ref->refcount;
}

inline bool nscript_obj_init_from_obj(lua_State *L, Obj *s_obj) {
	Obj **tmp_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");
	if (tmp_obj == NULL)
		return false;

	Obj *ptr = *tmp_obj;
	if (ptr == NULL)
		return false;

	s_obj->obj_n = ptr->obj_n;
	s_obj->frame_n = ptr->frame_n;
	s_obj->quality = ptr->quality;
	s_obj->qty = ptr->qty;
	s_obj->x = ptr->x;
	s_obj->y = ptr->y;
	s_obj->z = ptr->z;

	return true;
}

inline bool nscript_obj_init_from_args(lua_State *L, int nargs, Obj *s_obj) {
	uint8 i = nargs;

	if (i) {
		if (!lua_isnil(L, 1))
			s_obj->obj_n = (uint16)lua_tointeger(L, 1);
		i--;
	}

	if (i) {
		if (!lua_isnil(L, 2))
			s_obj->frame_n = (uint8)lua_tointeger(L, 2);
		i--;
	}

	if (i) {
		if (!lua_isnil(L, 3))
			s_obj->quality = (uint8)lua_tointeger(L, 3);
		i--;
	}

	if (i) {
		if (!lua_isnil(L, 4))
			s_obj->qty = (uint16)lua_tointeger(L, 4);
		i--;
	}

	if (i) {
		if (!lua_isnil(L, 5))
			s_obj->x = (uint16)lua_tointeger(L, 5);
		i--;
	}

	if (i) {
		if (!lua_isnil(L, 6))
			s_obj->y = (uint16)lua_tointeger(L, 6);
		i--;
	}

	if (i) {
		if (!lua_isnil(L, 7))
			s_obj->z = (uint8)lua_tointeger(L, 7);
		i--;
	}

	return true;
}

static int nscript_obj_gc(lua_State *L) {
	//DEBUG(0, LEVEL_INFORMATIONAL, "\nObj garbage Collection!\n");

	Obj **p_obj = (Obj **)lua_touserdata(L, 1);
	Obj *obj;

	if (p_obj == NULL)
		return false;

	obj = *p_obj;

	if (nscript_dec_obj_ref_count(obj) == 0) { // no longer referenced by the script engine
		// remove object if it is not referenced by the game engine.
		if (obj->get_engine_loc() == OBJ_LOC_NONE)
			delete_obj(obj);
	}

	return 0;
}

/*
   static inline Obj *nscript_get_obj_ptr(ScriptObj *s_obj)
   {
   if(s_obj)
   {
    if(s_obj->obj_ptr)
      return s_obj->obj_ptr;
    else
      return &s_obj->script_obj;
   }

   return NULL;
   }
 */

static void nscript_update_obj_location_variables(Obj *obj, uint16 x, uint16 y, uint8 z) {
	if (obj->is_on_map()) {
		Game::get_game()->get_obj_manager()->move(obj, x, y, z);
	} else {
		obj->x = x;
		obj->y = y;
		obj->z = z;
	}
}

static int nscript_obj_set(lua_State *L) {
	Obj **s_obj;
	Obj *obj;
	//Obj *ptr;
	const char *key;

	s_obj = (Obj **)lua_touserdata(L, 1);
	if (s_obj == NULL)
		return 0;

	obj = *s_obj;
	if (obj == NULL)
		return 0;

	// ptr = nscript_get_obj_ptr(s_obj);

	key = lua_tostring(L, 2);

	if (!strcmp(key, "x")) {
		nscript_update_obj_location_variables(obj, (uint16)lua_tointeger(L, 3), obj->y, obj->z);
		return 0;
	}

	if (!strcmp(key, "y")) {
		nscript_update_obj_location_variables(obj, obj->x, (uint16)lua_tointeger(L, 3), obj->z);
		return 0;
	}

	if (!strcmp(key, "z")) {
		nscript_update_obj_location_variables(obj, obj->x, obj->y, (uint8)lua_tointeger(L, 3));
		return 0;
	}

	if (!strcmp(key, "obj_n")) {
		obj->obj_n = (uint16)lua_tointeger(L, 3);
		return 0;
	}

	if (!strcmp(key, "frame_n")) {
		obj->frame_n = (uint8)lua_tointeger(L, 3);
		return 0;
	}

	if (!strcmp(key, "quality")) {
		obj->quality = (uint8)lua_tointeger(L, 3);
		return 0;
	}

	if (!strcmp(key, "qty")) {
		obj->qty = (uint8)lua_tointeger(L, 3);
		return 0;
	}

	if (!strcmp(key, "status")) {
		obj->status = (uint8)lua_tointeger(L, 3);
		return 0;
	}

	if (!strcmp(key, "invisible")) {
		obj->set_invisible((bool)lua_toboolean(L, 3));
		return 0;
	}

	if (!strcmp(key, "ok_to_take")) {
		obj->set_ok_to_take((bool)lua_toboolean(L, 3));
		return 0;
	}

	if (!strcmp(key, "temporary")) {
		obj->set_temporary((bool)lua_toboolean(L, 3));
		return 0;
	}

	return 0;
}

static int nscript_obj_get(lua_State *L) {
	Obj **s_obj;
	Obj *obj;
	const char *key;

	s_obj = (Obj **)lua_touserdata(L, 1);
	if (s_obj == NULL)
		return 0;

	obj = *s_obj;
	if (obj == NULL)
		return 0;

	//ptr = nscript_get_obj_ptr(s_obj);

	key = lua_tostring(L, 2);

	if (!strcmp(key, "luatype")) {
		lua_pushstring(L, "obj");
		return 1;
	}

	if (!strcmp(key, "x")) {
		lua_pushinteger(L, obj->x);
		return 1;
	}

	if (!strcmp(key, "y")) {
		lua_pushinteger(L, obj->y);
		return 1;
	}

	if (!strcmp(key, "z")) {
		lua_pushinteger(L, obj->z);
		return 1;
	}

	if (!strcmp(key, "obj_n")) {
		lua_pushinteger(L, obj->obj_n);
		return 1;
	}

	if (!strcmp(key, "frame_n")) {
		lua_pushinteger(L, obj->frame_n);
		return 1;
	}

	if (!strcmp(key, "quality")) {
		lua_pushinteger(L, obj->quality);
		return 1;
	}

	if (!strcmp(key, "qty")) {
		lua_pushinteger(L, obj->qty);
		return 1;
	}

	if (!strcmp(key, "name")) {
		ObjManager *obj_manager = Game::get_game()->get_obj_manager();
		lua_pushstring(L, obj_manager->get_obj_name(obj->obj_n, obj->frame_n));
		return 1;
	}
	/*
	   if(!strcmp(key, "container"))
	   {
	       U6LList *obj_list = obj->container;
	       if(obj_list == NULL)
	          return 0;

	       U6Link *link = obj_list->start();

	       lua_pushcfunction(L, nscript_u6llist_iter);

	       U6Link **p_link = (U6Link **)lua_newuserdata(L, sizeof(U6Link *));
	       *p_link = link;

	       luaL_getmetatable(L, "nuvie.U6Link");
	       lua_setmetatable(L, -2);

	       return 2;
	   }
	*/
	if (!strcmp(key, "look_string")) {
		ObjManager *obj_manager = Game::get_game()->get_obj_manager();
		lua_pushstring(L, obj_manager->look_obj(obj, true));
		return 1;
	}

	if (!strcmp(key, "on_map")) {
		lua_pushboolean(L, (int)obj->is_on_map());
		return 1;
	}

	if (!strcmp(key, "in_container")) {
		lua_pushboolean(L, (int)obj->is_in_container());
		return 1;
	}

	if (!strcmp(key, "readied")) {
		lua_pushboolean(L, (int)obj->is_readied());
		return 1;
	}

	if (!strcmp(key, "stackable")) {
		ObjManager *obj_manager = Game::get_game()->get_obj_manager();
		lua_pushboolean(L, (int)obj_manager->is_stackable(obj));
		return 1;
	}

	if (!strcmp(key, "status")) {
		lua_pushnumber(L, obj->status);
		return 1;
	}

	if (!strcmp(key, "weight")) {
		ObjManager *obj_manager = Game::get_game()->get_obj_manager();
		float weight = obj_manager->get_obj_weight(obj, OBJ_WEIGHT_INCLUDE_CONTAINER_ITEMS, OBJ_WEIGHT_DONT_SCALE);
		weight = floorf(weight); //get rid of the tiny fraction
		weight /= 10; //now scale.
		lua_pushnumber(L, (lua_Number)weight);
		return 1;
	}

	if (!strcmp(key, "tile_num")) {
		ObjManager *obj_manager = Game::get_game()->get_obj_manager();
		Tile *tile = obj_manager->get_obj_tile(obj->obj_n, obj->frame_n);
		lua_pushinteger(L, (int)tile->tile_num);
		return 1;
	}

	if (!strcmp(key, "tile_num_original")) {
		ObjManager *obj_manager = Game::get_game()->get_obj_manager();
		TileManager *tile_manager = Game::get_game()->get_tile_manager();
		Tile *tile = tile_manager->get_original_tile(obj_manager->get_obj_tile_num(obj->obj_n) + obj->frame_n);
		lua_pushinteger(L, (int)tile->tile_num);
		return 1;
	}


	if (!strcmp(key, "getable")) {
		ObjManager *obj_manager = Game::get_game()->get_obj_manager();
		lua_pushboolean(L, (int)obj_manager->can_get_obj(obj));
		return 1;
	}

	if (!strcmp(key, "ok_to_take")) {
		lua_pushboolean(L, (int)obj->is_ok_to_take());
		return 1;
	}

	if (!strcmp(key, "parent")) {
		Obj *parent_container = obj->get_container_obj();
		if (parent_container) {
			nscript_new_obj_var(L, parent_container);
			return 1;
		} else if (obj->is_in_inventory()) {
			Actor *parent_actor = obj->get_actor_holding_obj();
			if (parent_actor) {
				nscript_new_actor_var(L, parent_actor->get_actor_num());
				return 1;
			}
		}
	}

	if (!strcmp(key, "xyz")) {
		lua_newtable(L);
		lua_pushstring(L, "x");
		lua_pushinteger(L, obj->x);
		lua_settable(L, -3);

		lua_pushstring(L, "y");
		lua_pushinteger(L, obj->y);
		lua_settable(L, -3);

		lua_pushstring(L, "z");
		lua_pushinteger(L, obj->z);
		lua_settable(L, -3);

		return 1;
	}

	if (!strcmp(key, "invisible")) {
		lua_pushboolean(L, (int)obj->is_invisible());
		return 1;
	}

	return 0;
}

/***
Move an object to the map.
@function Obj.moveToMap
@tparam Obj obj Object to move
@tparam[opt] MapCoord|x,y,z location Map location. If not supplied the location will be taken from the object's x,y and z variables
@within Object
 */
static int nscript_obj_movetomap(lua_State *L) {
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();

	Obj **s_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;

	MapCoord loc;
	if (lua_gettop(L) >= 2) {
		if (nscript_get_location_from_args(L, &loc.x, &loc.y, &loc.z, 2) == false)
			return 0;
	} else {
		loc.x = obj->x;
		loc.y = obj->y;
		loc.z = obj->z;
	}

	if (obj) {
		if (obj_manager->moveto_map(obj, loc) == false) {
			//delete map_obj;
			return luaL_error(L, "moving obj to map!");
		}

		//s_obj->obj_ptr = map_obj;
	}

	return 0;
}

/***
Move an object into an Actor's inventory
@function Obj.moveToInv
@tparam Obj obj Object to move
@int actor_num Actor number
@within Object
 */
static int nscript_obj_movetoinv(lua_State *L) {
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();
	ActorManager *actor_manager = Game::get_game()->get_actor_manager();
	Actor *actor;

	Obj **s_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;

	if (lua_gettop(L) < 2)
		return luaL_error(L, "You must supply an Actor # to Obj.moveToInv()");

	actor = actor_manager->get_actor(lua_tointeger(L, 2));

	if (actor == NULL)
		return luaL_error(L, "Getting Actor (%d)", lua_tointeger(L, 2));

	if (obj) {
		if (obj_manager->moveto_inventory(obj, actor) == false) {
			//delete inv_obj;
			return luaL_error(L, "moving obj to actor inventory!");
		}

		//s_obj->obj_ptr = inv_obj;
	}


	return 0;
}

/***
Move an object into a container
@function Obj.moveToCont
@tparam Obj obj Object to move
@tparam Obj container Container object to move into
@within Object
 */
static int nscript_obj_movetocont(lua_State *L) {
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();
	Obj **s_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");
	Obj *obj;
	Obj *container_obj;

	obj = *s_obj;

	if (obj == NULL)
		return 0;

	if (lua_gettop(L) < 2)
		return luaL_error(L, "You must supply an Object to move into in Obj.moveToCont()");

	s_obj = (Obj **)luaL_checkudata(L, 2, "nuvie.Obj");
	container_obj = *s_obj;

	if (container_obj) {
		if (obj_manager->moveto_container(obj, container_obj) == false) {
			return luaL_error(L, "moving obj into container!");
		}
	}

	//pos = lua_tointeger(L, 2);

	return 0;
}

/***
Remove an object from its container.
The object will be unlinked from the engine after this operation. It will be freed after it goes out of scope.
@function Obj.removeFromCont
@tparam Obj obj Object to move
@within Object
 */
static int nscript_container_remove_obj(lua_State *L) {
	Obj **s_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");
	Obj *obj;
	Obj *cont_obj;

	obj = *s_obj;

	if (obj == NULL)
		return luaL_error(L, "getting obj!");

	cont_obj = obj->get_container_obj();

	if (cont_obj == NULL)
		return luaL_error(L, "obj not in a container!");

	if (cont_obj->remove(obj) == false)
		return luaL_error(L, "removing obj from container!");

	return 0;
}

/***
Call the old C++ usecode logic for a given object. (U6)
@function Obj.use
@tparam Obj obj Object to use
@within Object
 */
static int nscript_obj_use(lua_State *L) {
	UseCode *usecode = Game::get_game()->get_usecode();
	Player *player = Game::get_game()->get_player();
	Actor *actor = player->get_actor();

	Obj **s_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;

	if (obj) {
		usecode->use_obj(obj, actor);
	}


	return 0;
}

/***
Remove an object from the game engine.
The object will be unlinked from the engine after this operation. It will be freed after it goes out of scope.
@function Obj.removeFromEngine
@tparam Obj obj Object to unlink
@within Object
 */
static int nscript_obj_removefromengine(lua_State *L) {
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();
	Obj **s_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");

	Obj *obj;

	obj = *s_obj;

	if (obj) {
		obj_manager->unlink_from_engine(obj);
	}

	return 0;
}

/* release last iter U6Link if required. */
static int nscript_u6link_gc(lua_State *L) {
	U6Link **s_link = (U6Link **)luaL_checkudata(L, 1, "nuvie.U6Link");
	U6Link *link = *s_link;

	if (link == NULL)
		return 0;

	releaseU6Link(link);

	::debug(1, "U6Link garbage collector!!");
	return 0;
}

/* free up resources for a recursive U6Link iterator. */
static int nscript_u6link_recursive_gc(lua_State *L) {
	Std::stack<U6Link *> **s_stack = (Std::stack<U6Link *> **)luaL_checkudata(L, 1, "nuvie.U6LinkRecursive");
	Std::stack<U6Link *> *s = *s_stack;

	if (s->empty() == false) {
		for (; !s->empty(); s->pop()) {
			U6Link *link = s->top();

			if (link != NULL)
				releaseU6Link(link);
		}
	}

	delete s;

	//printf("U6LinkResursive garbage collector!!\n");
	return 0;
}

/***
   Clear the message scroll
   @function clear_scroll
 */
static int nscript_clear_scroll(lua_State *L) {
	MsgScroll *scroll = Game::get_game()->get_scroll();
	if (scroll) {
		scroll->clear_scroll();
	}

	return 0;
}

/***
   Print a string to the message scroll
   @function print
   @param string the string to print
*/
static int nscript_print(lua_State *L) {
	MsgScroll *scroll = Game::get_game()->get_scroll();
	const char *string = luaL_checkstring(L, 1);
	if (scroll) {
		scroll->display_string(string);
	} else {
		::debug(1, "%s", string);
	}
	return 0;
}

/***
   Print the prompt string to the message scroll
   @function display_prompt
   @param string the string to print
*/
static int nscript_display_prompt(lua_State *L) {
	MsgScroll *scroll = Game::get_game()->get_scroll();

	if (!scroll->can_display_prompt())
		return 0;

	bool newline = lua_toboolean(L, 1);
	if (newline)
		scroll->display_string("\n");
	scroll->display_prompt();
	return 0;
}

/***
   Load a lua script from the data/scripts/ directory
   @function nuvie_load
   @param path lua file relative to data/scripts directory
   @return contents of the lua file as a function block on success.
           A string is returned on compilation failure.
           nil is returned if the file cannot be opened
 */
static int nscript_load(lua_State *L) {
	const char *file = luaL_checkstring(L, 1);
	string dir;
	string path;

	Script::get_script()->get_config()->value("config/datadir", dir, "");

	build_path(dir, "scripts", path);
	dir = path;
	build_path(dir, file, path);

	if (luaL_loadfile(L, path.c_str()) == LUA_ERRFILE) {
		lua_pop(L, 1);
		return 0;
	}

	return 1;
}

/***
   Get a boolean value for a given key from the config file
   @function config_get_boolean_value
   @param config_key config key to be retrieved
   @return boolean config value
 */
static int nscript_config_get_boolean_value(lua_State *L) {
	bool value;
	const char *config_key = luaL_checkstring(L, 1);
	Script::get_script()->get_config()->value(Std::string(config_key), value);

	lua_pushboolean(L, value);
	return 1;
}

/***
   Get the currently running game type
   @function config_get_game_type
   @return a two character string representing the current game type. "U6", "MD" or "SE"
 */
static int nscript_config_get_game_type(lua_State *L) {
	lua_pushstring(L, get_game_tag(Game::get_game()->get_game_type()));
	return 1;
}

/***
   Get the currently selected language
   @function config_get_language
   @return a two character string representing the currently selected language. "en" is the default if no language has been selected.
 */
static int nscript_config_get_language(lua_State *L) {
	Std::string value;
	Script::get_script()->get_config()->value(config_get_game_key(Script::get_script()->get_config()) + "/language", value, "en");
	lua_pushstring(L, value.c_str());
	return 1;
}

/***
   Seek to a given position in the objlist data
   @function objlist_seek
   @param position position to seek to in bytes relative to the start of the file
 */
static int nscript_objlist_seek(lua_State *L) {
	uint32 position = (uint32)lua_tointeger(L, 1);
	if (g_objlist_file)
		g_objlist_file->seek(position);

	return 0;
}

/***
   Read a 1 byte integer number from the current position in the objlist data.
   The current position is incremented by 1 after the read.
   @function objlist_read1
   @return value
 */
static int nscript_objlist_read1(lua_State *L) {
	if (g_objlist_file) {
		lua_pushinteger(L, g_objlist_file->read1());
		return 1;
	}

	return 0;
}

/***
   Overwrite objlist data at the current position with a 1 byte unsigned number.
   The current position is incremented by 1 after the write.
   @function objlist_write1
   @param value number to write. This number will be cast to uint8
   @return true on success false on failure

 */
static int nscript_objlist_write1(lua_State *L) {
	bool ret = false;
	uint8 value = (uint8)lua_tointeger(L, 1);
	if (g_objlist_file) {
		ret = g_objlist_file->write1(value);
	}

	lua_pushboolean(L, ret);
	return 1;
}

/***
   Read a 2 byte integer number from the current position in the objlist data.
   The current position is incremented by 2 after the read.
   @function objlist_read2
   @return value
 */
static int nscript_objlist_read2(lua_State *L) {
	if (g_objlist_file) {
		lua_pushinteger(L, g_objlist_file->read2());
		return 1;
	}

	return 0;
}

/***
   Overwrite objlist data at the current position with a 2 byte unsigned number.
   The current position is incremented by 2 after the write.
   @function objlist_write2
   @param value number to write. This number will be cast to uint16
   @return true on success false on failure

 */
static int nscript_objlist_write2(lua_State *L) {
	bool ret = false;
	uint16 value = (uint16)lua_tointeger(L, 1);
	if (g_objlist_file) {
		ret = g_objlist_file->write2(value);
	}

	lua_pushboolean(L, ret);
	return 1;
}

/***
Get the currently selected UI style
@function game_get_ui_style
@return The UI style

-   0 = Original style
-   1 = New style
-   2 = Original+ cutoff map
-   3 = Original+ full map
 */
static int nscript_game_get_ui_style(lua_State *L) {
	lua_pushinteger(L, Game::get_game()->get_game_style());
	return 1;
}

/***
Get the player name
@function player_get_name
@return string player name
@within player
 */
static int nscript_player_get_name(lua_State *L) {
	Player *player = Game::get_game()->get_player();
	if (player) {
		lua_pushstring(L, player->get_name());
		return 1;
	}

	return 0;
}

/***
Get the gender of the player
@function player_get_gender
@return

- 0 = Male
- 1 = Female
@within player
 */
static int nscript_player_get_gender(lua_State *L) {
	uint8 gender = 0;
	Player *player = Game::get_game()->get_player();
	if (player) {
		gender = player->get_gender();
	}

	lua_pushinteger(L, gender);
	return 1;
}

/***
Get the location of the player
@function player_get_location
@treturn MapCoord
@within player
 */
static int nscript_player_get_location(lua_State *L) {
	Player *player = Game::get_game()->get_player();

	uint16 x, y;
	uint8 z;

	player->get_actor()->get_location(&x, &y, &z);

	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushinteger(L, x);
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushinteger(L, y);
	lua_settable(L, -3);

	lua_pushstring(L, "z");
	lua_pushinteger(L, z);
	lua_settable(L, -3);

	return 1;
}

/***
Get the player's karma value (U6)
@function player_get_karma
@return karma value
@within player
 */
static int nscript_player_get_karma(lua_State *L) {
	Player *player = Game::get_game()->get_player();
	lua_pushinteger(L, player->get_karma());
	return 1;
}

/***
Set the karma value for the player (U6)
@function player_set_karma
@param value new karma value
@within player
 */
static int nscript_player_set_karma(lua_State *L) {
	Player *player = Game::get_game()->get_player();
	player->set_karma((uint8)lua_tointeger(L, 1));
	return 0;
}

/***
Decrement the player's alcohol counter (U6)

If value is greater than the counter the the counter is left at zero.
@function player_dec_alcohol
@param value number to decrement counter by
@within player
 */
static int nscript_player_dec_alcohol(lua_State *L) {
	Player *player = Game::get_game()->get_player();
	player->dec_alcohol((uint8)lua_tointeger(L, 1));
	return 0;
}

/***
Check to see if the party is currently in combat mode
@function party_is_in_combat_mode
@treturn boolean true if the party is in combat mode otherwise false
@within party
 */
static int nscript_party_is_in_combat_mode(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	lua_pushboolean(L, party->is_in_combat_mode());
	return 1;
}

/***
Toggle combat mode
@function party_set_combat_mode
@tparam boolean value
@within party
 */
static int nscript_party_set_combat_mode(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	party->set_in_combat_mode(lua_toboolean(L, 1));
	return 0;
}

/***
Set party mode. The first member in the party becomes the leader.
@function party_set_party_mode
@within party
 */
static int nscript_party_set_party_mode(lua_State *L) {
	Player *player = Game::get_game()->get_player();
	player->set_party_mode(player->get_party()->get_actor(0));

	return 0;
}

/***
Move party to a given map location
@function party_move
@tparam MapCoord|x,y,z location map location to move party to.
@within party
 */
static int nscript_party_move(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	uint16 x, y;
	uint8 z;

	if (nscript_get_location_from_args(L, &x, &y, &z) == false)
		return 0;

	party->move(x, y, z);

	return 0;
}

/***
Walk party members to an entrance and teleport them to the exit on the other side.

__Warning:__ This function uses an old timer based class. So it will execute synchronously on subsequent calls to the main update loop.

__FIXME:__ This logic should probably be done in pure script with pathfinder helper functions. ;-)

@function party_use_entrance
@tparam x,y,z entrance entrance location
@tparam MapCoord|x,y,z exit exit location
@within party
 */
static int nscript_party_use_entrance(lua_State *L) {
	Party *party = Game::get_game()->get_party();

	MapCoord entrance;
	MapCoord exit;

	if (nscript_get_location_from_args(L, &entrance.x, &entrance.y, &entrance.z) == false)
		return 0;
	if (nscript_get_location_from_args(L, &exit.x, &exit.y, &exit.z, 4) == false)
		return 0;

	party->walk(&entrance, &exit);

	return 0;
}

/***
Move player to given map location
@function player_move
@tparam MapCoord|x,y,z location map location to move player to.
@bool teleport spawn eggs if true
@within player
 */
static int nscript_player_move(lua_State *L) {
	Player *player = Game::get_game()->get_player();
	uint16 x, y;
	uint8 z;

	if (nscript_get_location_from_args(L, &x, &y, &z) == false)
		return 0;

	player->move(x, y, z, lua_toboolean(L, 4));

	return 0;
}

/***
Make the given actor the player controlled actor.
@function player_set_actor
@tparam Actor actor
@within player
 */
static int nscript_player_set_actor(lua_State *L) {
	Player *player = Game::get_game()->get_player();
	Actor *actor = nscript_get_actor_from_args(L, 1);

	if (actor && actor != player->get_actor())
		player->update_player(actor);

	return 0;
}

/***
Check if the player is currently in solo mode
@function player_is_in_solo_mode
@treturn bool true if the player is in solo mode otherwise false
@within player
 */
static int nscript_player_is_in_solo_mode(lua_State *L) {
	Player *player = Game::get_game()->get_player();
	lua_pushboolean(L, !player->in_party_mode());

	return 1;
}

/***
Returns the number of members in the party
@function party_get_size
@treturn int number of party members
@within party
 */
static int nscript_party_get_size(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	lua_pushinteger(L, party->get_party_size());
	return 1;
}

/***
Get the Actor object for a given party member
@function party_get_member
@int member_num index of party member to retrieve
@treturn Actor|nil returns nil if actor cannot be found for given index
@within party
 */
static int nscript_party_get_member(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	uint8 member_num = (uint8)lua_tointeger(L, 1);

	Actor *actor = party->get_actor(member_num);

	if (actor == NULL)
		return 0;

	nscript_new_actor_var(L, actor->get_actor_num());
	return 1;
}

/***
Get the current party leader and update player
@function party_update_leader
@within party
 */
static int nscript_party_update_leader(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	Player *player = Game::get_game()->get_player();

	Actor *leader = party->get_leader_actor();

	if (leader) {
		player->update_player(leader);
	}

	return 0;
}

/***
Resurrect dead party members
@function party_resurrect_dead_members
@within party
 */
static int nscript_party_resurrect_dead_members(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	party->resurrect_dead_members();

	return 0;
}

/***
Exit party members from vehicle
@function party_exit_vehicle
@tparam MapCoord|x,y,z location map location to move party to once they exit the vehicle
@within party
 */
static int nscript_party_exit_vehicle(lua_State *L) {
	Party *party = Game::get_game()->get_party();

	uint16 x, y;
	uint8 z;

	if (nscript_get_location_from_args(L, &x, &y, &z) == false)
		return 0;

	party->exit_vehicle(x, y, z);

	return 0;
}

/***
Dismount all party members from their horses (U6)
@function party_dismount_from_horses
@within party
 */
static int nscript_party_dismount_from_horses(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	party->dismount_from_horses();
	return 0;
}

/***
Toggle party vehicle mode
@function party_set_in_vehicle
@bool value
@within party
 */
static int nscript_party_set_in_vehicle(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	party->set_in_vehicle((bool) lua_toboolean(L, 1));
	return 0;
}

/***
Show all party members on the map.
@function party_show_all
@within party
 */
static int nscript_party_show_all(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	party->show();
	return 0;
}

/***
Hide all party members on the map.
@function party_hide_all
@within party
 */
static int nscript_party_hide_all(lua_State *L) {
	Party *party = Game::get_game()->get_party();
	party->hide();
	return 0;
}

/***
Get an object from the map
@function map_get_obj
@tparam MapCoord|x,y,z location
@param[opt] obj_n object number
@bool[opt=false] inc_multi_tile_objs Should this search also include surrounding multi-tile objects that cover this location?
@treturn Obj|nil
@within map
 */
static int nscript_map_get_obj(lua_State *L) {
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();
	Obj *obj;

	uint16 x, y;
	uint8 z;

	if (nscript_get_location_from_args(L, &x, &y, &z) == false)
		return 0;

	int top = lua_gettop(L);
	bool loc_is_table = lua_istable(L, 1);
	int stack_offset = loc_is_table ? 2 : 4;

	if ((loc_is_table && top > 1) || top > 3) {
		uint16 obj_n = lua_tointeger(L, stack_offset);
		bool include_multi_tile_objs = false;
		if (lua_gettop(L) > stack_offset) {
			include_multi_tile_objs = lua_toboolean(L, stack_offset + 1);
		}
		if (include_multi_tile_objs) {
			obj = obj_manager->get_obj_of_type_from_location_inc_multi_tile(obj_n, x, y, z);
		} else {
			obj = obj_manager->get_obj_of_type_from_location(obj_n, x, y, z);
		}
	} else {
		obj = obj_manager->get_obj(x, y, z);
	}

	if (obj) {
		nscript_new_obj_var(L, obj);
		return 1;
	}

	return 0;
}

/***
Remove an object from the map.

Once removed from the map the object has no engine location and will be cleaned up in the next GC
@function map_remove_obj
@tparam MapCoord|x,y,z location
@param[opt] obj_n object number
@treturn boolean success/failure
@within map
 */
static int nscript_map_remove_obj(lua_State *L) {
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();

	Obj **s_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;

	if (obj_manager->remove_obj_from_map(obj))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

/***
Can you put an actor at a given map location
@function map_can_put
@tparam MapCoord|x,y,z location
@treturn boolean true if actor an be placed at location otherwise false
@within map
 */
static int nscript_map_can_put_actor(lua_State *L) {
	ActorManager *actor_manager = Game::get_game()->get_actor_manager();
	uint16 x, y;
	uint8 z;

	if (nscript_get_location_from_args(L, &x, &y, &z) == false)
		return 0;

	lua_pushboolean(L, actor_manager->can_put_actor(MapCoord(x, y, z)));

	return 1;
}

/***
Can you put an object at a given map location
@function map_can_put_obj
@tparam MapCoord|x,y,z location
@treturn bool true if an object an be placed at location otherwise false
@within map
 */
static int nscript_map_can_put_obj(lua_State *L) {
	Map *map = Game::get_game()->get_game_map();
	uint16 x, y;
	uint8 z;

	if (nscript_get_location_from_args(L, &x, &y, &z, 1) == false)
		return 0;

	lua_pushboolean(L, map->can_put_obj(x, y, z));

	return 1;
}

/***
Toggle automatic cleaning of out of area temporary actors.
@function map_enable_temp_actor_cleaning
@tparam bool value
@within map
 */
static int nscript_map_enable_temp_actor_cleaning(lua_State *L) {
	ActorManager *actorManager = Game::get_game()->get_actor_manager();
	actorManager->enable_temp_actor_cleaning((bool)lua_toboolean(L, 1));
	return 0;
}

/***
Check map location for water
@function map_is_water
@tparam MapCoord|x,y,z location
@treturn bool true if the map at location is a water tile otherwise false
@within map
 */
static int nscript_map_is_water(lua_State *L) {
	Map *map = Game::get_game()->get_game_map();

	uint16 x, y;
	uint8 z;
	if (nscript_get_location_from_args(L, &x, &y, &z, 1) == false)
		return 0;

	lua_pushboolean(L, map->is_water(x, y, z));

	return 1;
}

/***
Checks if the map location is currently on screen
@function map_is_on_screen
@tparam MapCoord|x,y,z location
@treturn bool true if the map location is currently on screen otherwise false
@within map
 */
static int nscript_map_is_on_screen(lua_State *L) {
	MapWindow *map_window = Game::get_game()->get_map_window();

	uint16 x, y;
	uint8 z;
	if (nscript_get_location_from_args(L, &x, &y, &z, 1) == false)
		return 0;

	lua_pushboolean(L, map_window->is_on_screen(x, y, z));

	return 1;
}

/***
Get the impedance of a map location
@function map_get_impedence
@int x
@int y
@int z
@bool[opt=true] ignore_objects Ignore objects while calculating impedance
@treturn int impedance
@fixme Rename this function to map_get_impedance()
@within map
 */
static int nscript_map_get_impedence(lua_State *L) {
	Map *map = Game::get_game()->get_game_map();

	uint16 x, y;
	uint8 z;
	if (nscript_get_location_from_args(L, &x, &y, &z, 1) == false)
		return 0;

	bool ignore_objects = true;

	if (lua_gettop(L) >= 4)
		ignore_objects = (bool) lua_toboolean(L, 4);

	lua_pushinteger(L, map->get_impedance(x, y, z, ignore_objects));

	return 1;
}

/***
get the map tile number for a given map location
@function map_get_tile_num
@tparam MapCoord|x,y,z location
@bool[opt=false] get_original_tile_num return the original tile_num.
@treturn int|nil
@within map
 */
static int nscript_map_get_tile_num(lua_State *L) {
	bool original_tile = false;
	Map *map = Game::get_game()->get_game_map();

	uint16 x, y;
	uint8 z;
	if (nscript_get_location_from_args(L, &x, &y, &z, 1) == false)
		return 0;

	if (lua_istable(L, 1)) {
		if (lua_gettop(L) >= 2)
			original_tile = (bool) lua_toboolean(L, 2);
	} else {
		if (lua_gettop(L) >= 4)
			original_tile = (bool) lua_toboolean(L, 4);
	}

	Tile *t = map->get_tile(x, y, z, original_tile);
	if (t != NULL) {
		lua_pushinteger(L, t->tile_num);
		return 1;
	}

	return 0;
}

/***
get the tile number for given location if the map tile can do damage.
If the base map tile does not do damage then objects at the map location
are also searched and the tile number of the first object that can damage the player
is returned.
@function map_get_dmg_tile_num
@int x
@int y
@int z
@treturn int|nil tile number of the damaging tile or nil if no damaging tiles are found at the location
@within map
 */
static int nscript_map_get_dmg_tile_num(lua_State *L) {
	Map *map = Game::get_game()->get_game_map();

	uint16 x, y;
	uint8 z;
	if (nscript_get_location_from_args(L, &x, &y, &z, 1) == false)
		return 0;

	Tile *t = map->get_dmg_tile(x, y, z);
	if (t != NULL) {
		lua_pushinteger(L, t->tile_num);
		return 1;
	}

	return 0;
}

/***
Returns true if a line between points x,y and x1, y1 does not cross any missile boundary tiles
@function map_can_reach_point
@int x
@int y
@int x1
@int y1
@int z
@treturn bool
@within map
 */
static int nscript_map_line_test(lua_State *L) {
	Map *map = Game::get_game()->get_game_map();
	LineTestResult result;
	bool ret = false;

	uint16 x = (uint16) luaL_checkinteger(L, 1);
	uint16 y = (uint16) luaL_checkinteger(L, 2);

	uint16 x1 = (uint16) luaL_checkinteger(L, 3);
	uint16 y1 = (uint16) luaL_checkinteger(L, 4);
	uint8 level = (uint8) luaL_checkinteger(L, 5);

	//FIXME world wrapping for MD
	if (map->lineTest(x, y, x1, y1, level, LT_HitMissileBoundary, result) == false)
		ret = true;

	lua_pushboolean(L, (int)ret);
	return 1;
}

/***
Returns the first point on a line between x,y and x1, y1 where a missile boundary tile is crossed
If no boundary tiles are crossed on the line then x1, y1 are returned
@function map_line_hit_check
@int x
@int y
@int x1
@int y1
@int z
@treturn int,int an x,y coord
@within map
 */
static int nscript_map_line_hit_check(lua_State *L) {
	Map *map = Game::get_game()->get_game_map();
	LineTestResult result;

	uint16 x = (uint16) luaL_checkinteger(L, 1);
	uint16 y = (uint16) luaL_checkinteger(L, 2);

	uint16 x1 = (uint16) luaL_checkinteger(L, 3);
	uint16 y1 = (uint16) luaL_checkinteger(L, 4);
	uint8 level = (uint8) luaL_checkinteger(L, 5);

	//FIXME world wrapping for MD
	if (map->lineTest(x, y, x1, y1, level, LT_HitMissileBoundary, result)) {
		lua_pushinteger(L, result.hit_x);
		lua_pushinteger(L, result.hit_y);
	} else {
		lua_pushinteger(L, x1);
		lua_pushinteger(L, y1);
	}

	return 2;
}

/***
export map to Tiled TMX files. This creates 1 TMX file per map level. They are saved into the currently active save game directory.
@function map_export_tmx_files
@treturn bool returns true if the tmx files were created successfully. false on error
 */
static int nscript_map_export_tmx_files(lua_State *L) {
	Game *game = Game::get_game();
	TMXMap *tmxMap = new TMXMap(game->get_tile_manager(), game->get_game_map(), game->get_obj_manager());
	lua_pushboolean(L, tmxMap->exportTmxMapFiles("data", game->get_game_type()));

	delete tmxMap;
	return 1;
}

/***
export tileset to a bmp file 'data/images/tiles/nn/custom_tiles.bmp' in the current savegame directory.
@function tileset_export
@bool[opt=false] overWriteFile specifies if the output file should be overwritten if it already exists.
@treturn bool returns true if the was written to disk. false otherwise
 */
static int nscript_tileset_export(lua_State *L) {
	Game *game = Game::get_game();
	bool overwriteFile = false;

	if (lua_gettop(L) >= 1) {
		overwriteFile = (bool)lua_toboolean(L, 1);
	}

	Std::string path;
	path = "data";
	build_path(path, "images", path);
	build_path(path, "tiles", path);
	build_path(path, get_game_tag(game->get_game_type()), path);

	if (!directory_exists(path.c_str())) {
		mkdir_recursive(path.c_str(), 0700);
	}

	build_path(path, "custom_tiles.bmp", path);

	if (!overwriteFile && file_exists(path.c_str())) {
		lua_pushboolean(L, false);
	} else {
		game->get_tile_manager()->exportTilesetToBmpFile(path, false);
		lua_pushboolean(L, true);
	}

	return 1;
}

/***
get a tile flag for a given tile number
@function tile_get_flag
@int tile_number
@int flag_set either 1, 2 or 3
@int bit_number bit number of flag from flag_set 0..7
@treturn bool|nil return flag or nil on error
 */
static int nscript_tile_get_flag(lua_State *L) {
	uint16 tile_num = (uint16) luaL_checkinteger(L, 1);
	uint8 flag_set = (uint8) luaL_checkinteger(L, 2);
	uint8 bit = (uint8) luaL_checkinteger(L, 3);

	Tile *tile = Game::get_game()->get_tile_manager()->get_original_tile(tile_num);

	if (tile == NULL || flag_set < 1 || flag_set > 3 || bit > 7)
		return 0;

	uint8 bit_flags = 0;

	if (flag_set == 1) {
		bit_flags = tile->flags1;
	} else if (flag_set == 2) {
		bit_flags = tile->flags2;
	} else if (flag_set == 3) {
		bit_flags = tile->flags3;
	}

	lua_pushboolean(L, (bool)(bit_flags & (1 << bit)));
	return 1;
}

/***
get the description for a given tile number
@function tile_get_description
@int tile_number
@treturn string the descriptive string for the given tile number.
 */
static int nscript_tile_get_description(lua_State *L) {
	uint16 tile_num = (uint16) luaL_checkinteger(L, 1);
	lua_pushstring(L, Game::get_game()->get_tile_manager()->lookAtTile(tile_num, 1, false));
	return 1;
}

/***
get the number of tile animations
@function anim_get_number_of_entries
@treturn int number of animations
*/
static int nscript_anim_get_number_of_entries(lua_State *L) {
	lua_pushinteger(L, Game::get_game()->get_tile_manager()->get_number_of_animations());
	return 1;
}

/***
get the tile number for a given animation number
@function anim_get_tile
@int anim_index index of animation
@treturn int tile number
*/
static int nscript_anim_get_tile(lua_State *L) {
	uint16 anim_index = (uint16) luaL_checkinteger(L, 1);

	lua_pushinteger(L, Game::get_game()->get_tile_manager()->get_anim_tile(anim_index));
	return 1;
}

/***
set the starting animation frame for a given animation number
@function anim_set_first_frame
@int anim_index index of animation loop to change
@int anim_start_tile the tile number of the start animation
*/
static int nscript_anim_set_first_frame(lua_State *L) {
	uint16 anim_index = (uint16) luaL_checkinteger(L, 1);
	uint16 anim_start_tile = (uint16) luaL_checkinteger(L, 2);

	Game::get_game()->get_tile_manager()->set_anim_first_frame(anim_index, anim_start_tile);
	return 0;
}

/***
get the starting animation frame for a given animation number
@function anim_get_first_frame
@int anim_index index of animation
@treturn int tile number of first animation frame
*/
static int nscript_anim_get_first_frame(lua_State *L) {
	uint16 anim_index = (uint16) luaL_checkinteger(L, 1);

	lua_pushinteger(L, Game::get_game()->get_tile_manager()->get_anim_first_frame(anim_index));
	return 1;
}

/***
start playing animation
@function anim_play
@int anim_index index of animation to play
*/
static int nscript_anim_play(lua_State *L) {
	uint8 anim_index = (uint8) luaL_checkinteger(L, 1);
	Game::get_game()->get_tile_manager()->anim_play_repeated(anim_index);
	return 0;
}

/***
stop playing animation
@function anim_stop
@int anim_index index of animation
*/
static int nscript_anim_stop(lua_State *L) {
	uint8 anim_index = (uint8) luaL_checkinteger(L, 1);
	Game::get_game()->get_tile_manager()->anim_stop_playing(anim_index);
	return 0;
}

/***
start earthquake effect. This shakes the mapwindow and makes a sound.
@function quake_start
@int magnitude quake strength
@int duration duration of effect in milliseconds
@treturn bool always returns true
@within effects
 */
static int nscript_quake_start(lua_State *L) {
	Player *player = Game::get_game()->get_player();

	uint8 magnitude = (uint8)luaL_checkinteger(L, 1);
	uint32 duration = (uint32)luaL_checkinteger(L, 2);

	new QuakeEffect(magnitude, duration, player->get_actor());

	lua_pushboolean(L, true);
	return 1;
}

static int nscript_new_hit_entities_tbl_var(lua_State *L, ProjectileEffect *effect) {
	vector<MapEntity> *hit_items = effect->get_hit_entities();

	lua_newtable(L);

	for (uint16 i = 0; i < hit_items->size(); i++) {
		lua_pushinteger(L, i);

		MapEntity m = (*hit_items)[i];
		if (m.entity_type == ENT_OBJ)
			nscript_obj_new(L, m.obj);
		else if (m.entity_type == ENT_ACTOR) {
			nscript_new_actor_var(L, m.actor->get_actor_num());
		}

		lua_settable(L, -3);
	}

	return 1;
}

/***
Create an explosion at x,y on the current level. This function returns an iterator function
to iterate through a list of Actors and Objects that were hit by the explosion.
@function explosion_start
@int tile_number
@int x
@int y
@treturn table A table containing the hit Actor and Obj objects.
@usage
    local hit_items = explosion_start(0x17e, actor.x, actor.y)

    for k,v in pairs(hit_items) do
      if v.luatype == "actor" then
        actor_hit(v, random(1, 0x14))
      end
    end
@within effects
 */
static int nscript_explosion_start(lua_State *L) {
	uint16 tile_num = (uint16)luaL_checkinteger(L, 1);
	uint16 x = (uint16)luaL_checkinteger(L, 2);
	uint16 y = (uint16)luaL_checkinteger(L, 3);

	ExpEffect *effect = new ExpEffect(tile_num, MapCoord(x, y));
	AsyncEffect *e = new AsyncEffect(effect);
	e->run();

	return nscript_new_hit_entities_tbl_var(L, (ProjectileEffect *)effect);
}

/***
Creates a single tile projectile effect. This travels in a straight line from (startx,starty) to (targetx,targety)
the tile can leave a trail and rotate whilst moving.
@function projectile_anim
@int startx
@int starty
@int targetx
@int targety
@int speed how far the tile travels each update. The tile will move speed * 2 pixels along the line each iteration
@bool trail if true a copy of the tile will be left at intervals along the line
@int initial_tile_rotation in degrees
@int[opt=0] rotation_amount in degrees
@int[opt=0] src_tile_y_offset use by U6 projectile tiles which need to be centred vertically before rotation. eg arrows.
@treturn bool Always returns true
@within effects
 */
static int nscript_projectile_anim(lua_State *L) {
	uint16 tile_num = (uint16)luaL_checkinteger(L, 1);
	uint16 startx = (uint16)luaL_checkinteger(L, 2);
	uint16 starty = (uint16)luaL_checkinteger(L, 3);
	uint16 targetx = (uint16)luaL_checkinteger(L, 4);
	uint16 targety = (uint16)luaL_checkinteger(L, 5);
	uint16 speed = (uint16)luaL_checkinteger(L, 6);
	bool trail = (bool)lua_toboolean(L, 7);
	uint8 initial_tile_rotation = (uint8)luaL_checkinteger(L, 8);
	uint16 rotation_amount = 0;
	uint8 src_tile_y_offset = 0;

	if (lua_gettop(L) >= 9)
		rotation_amount = (uint16)luaL_checkinteger(L, 9);

	if (lua_gettop(L) >= 10)
		src_tile_y_offset = (uint8)luaL_checkinteger(L, 10);

	ProjectileEffect *projectile_effect = new ProjectileEffect(tile_num, MapCoord(startx, starty), MapCoord(targetx, targety), speed, trail, initial_tile_rotation, rotation_amount, src_tile_y_offset);
	AsyncEffect *e = new AsyncEffect(projectile_effect);
	e->run();

	lua_pushboolean(L, true);
	return 1;
}

/***
Create multiple projectile effects emanating from (startx,starty) going to multiple target locations
@function projectile_anim_multi
@int tile_number
@int startx
@int starty
@tparam {MapCoord,...} targets
@int speed
@int trail
@int initial_tile_rotation
@treturn table|bool A table containing the hit Actor and Obj objects. Or false if argument 4 isn't a table
@within effects
 */
static int nscript_projectile_anim_multi(lua_State *L) {
	uint16 tile_num = (uint16)luaL_checkinteger(L, 1);
	uint16 startx = (uint16)luaL_checkinteger(L, 2);
	uint16 starty = (uint16)luaL_checkinteger(L, 3);

	if (!lua_istable(L, 4)) {
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushvalue(L, 4); //push table containing targets to top of stack

	uint16 x = 0;
	uint16 y = 0;
	uint8 z = 0;

	vector<MapCoord> t;

	for (int i = 1;; i++) {
		lua_pushinteger(L, i);
		lua_gettable(L, -2);

		if (!lua_istable(L, -1)) { //we've hit the end of our targets
			::debug(1, "end = %d", i);
			lua_pop(L, 1);
			break;
		}
		//get target fields here.

		get_tbl_field_uint16(L, "x", &x);
		get_tbl_field_uint16(L, "y", &y);
		get_tbl_field_uint8(L, "z", &z);

		t.push_back(MapCoord(x, y, z));

		lua_pop(L, 1);
	}

	uint16 speed = (uint16)luaL_checkinteger(L, 5);
	bool trail = (bool)luaL_checkinteger(L, 6);
	uint8 initial_tile_rotation = (uint8)luaL_checkinteger(L, 7);

	ProjectileEffect *effect = new ProjectileEffect(tile_num, MapCoord(startx, starty), t, speed, trail, initial_tile_rotation);
	AsyncEffect *e = new AsyncEffect(effect);
	e->run();

	return nscript_new_hit_entities_tbl_var(L, effect);
}

/***
Hit effect. Displays the hit/damage tile over the map at a given location for a short period of time. The hit sfx is also played.
@function hit_anim
@int x
@int y
@treturn bool Always returns true
@within effects
 */
static int nscript_hit_anim(lua_State *L) {
	uint16 targetx = (uint16)luaL_checkinteger(L, 1);
	uint16 targety = (uint16)luaL_checkinteger(L, 2);


	AsyncEffect *e = new AsyncEffect(new HitEffect(MapCoord(targetx, targety)));
	e->run();

	lua_pushboolean(L, true);
	return 1;
}


/***
Call the C++ usecode look function for a given Obj
@function usecode_look
@tparam Obj object object to look at
@treturn bool returns true if the Obj has a C++ usecode look function
@within Object
 */
//FIXME need to move this into lua script.
static int nscript_usecode_look(lua_State *L) {
	Obj **s_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;

	UseCode *usecode = Game::get_game()->get_usecode();
	Player *player = Game::get_game()->get_player();

	lua_pushboolean(L, (int)usecode->look_obj(obj, player->get_actor()));
	return 1;
}

/***
execute the pixelated map window fade out effect
@function fade_out
@within effects
 */
static int nscript_fade_out(lua_State *L) {
	AsyncEffect *e = new AsyncEffect(new FadeEffect(FADE_PIXELATED, FADE_OUT));
	e->run();

	return 0;
}

/***
execute the pixelated map window fade in effect
@function fade_in
@within effects
 */
static int nscript_fade_in(lua_State *L) {
	AsyncEffect *e = new AsyncEffect(new FadeEffect(FADE_PIXELATED, FADE_IN));
	e->run();

	return 0;
}

/***
pixel fade from one tile to another. If to_tile is not supplied the fade to blank
@function fade_tile
@tparam MapCoord|x,y,z location location for the effect to take place
@int from_tile tile number of the tile to fade from
@int[opt] to_tile tile number of the tile to fade to
@within effects
 */
static int nscript_fade_tile(lua_State *L) {
	MapCoord loc;
	Tile *tile_from =  NULL;
	Tile *tile_to =  NULL;
	TileManager *tm = Game::get_game()->get_tile_manager();

	if (nscript_get_location_from_args(L, &loc.x, &loc.y, &loc.z) == false)
		return 0;

	if (lua_isnumber(L, 4))
		tile_from = tm->get_tile((uint16)luaL_checkinteger(L, 4));

	if (lua_gettop(L) > 4)
		tile_to = tm->get_tile((uint16)luaL_checkinteger(L, 5));


	AsyncEffect *e = new AsyncEffect(new TileFadeEffect(loc, tile_from, tile_to, FADE_PIXELATED, 10));
//	AsyncEffect *e = new AsyncEffect(new TileFadeEffect(loc, tile_from, 0, 4, false, 20));
	e->run();


	return 0;
}

/***
black fade effect on a given object
@function fade_obj
@tparam Obj obj object to fade
@int fade_color the colour (palette index) to fade the black pixels to.
@int fade_speed
@within effects
 */
static int nscript_black_fade_obj(lua_State *L) {
	Obj *obj = nscript_get_obj_from_args(L, 1);
	uint8 fade_color = (uint8)lua_tointeger(L, 2);
	uint16 fade_speed = (uint8)lua_tointeger(L, 3);

	if (obj != NULL) {
		AsyncEffect *e = new AsyncEffect(new TileBlackFadeEffect(obj, fade_color, fade_speed));
		e->run();
	}

	return 0;
}

/***
xor effect. XOR the colours on the mapwindow
@function xor_effect
@int duration in milliseconds
@within effects
 */
static int nscript_xor_effect(lua_State *L) {
	uint16 duration = (uint16)luaL_checkinteger(L, 1);

	AsyncEffect *e = new AsyncEffect(new XorEffect(duration));
	e->run();

	return 0;
}

/***
xray effect. Disable map blacking. (see through walls)
@function xray_effect
@int duration in milliseconds
@within effects
 */
static int nscript_xray_effect(lua_State *L) {
	uint16 duration = (uint16)luaL_checkinteger(L, 1);

	AsyncEffect *e = new AsyncEffect(new XRayEffect(duration));
	e->run();

	return 0;
}
/***
peer effect.
Display an overview of the current area in the MapWindow. Any new actions
cancel the effect and return to the script.
(area is 48x48 tiles around the player, regardless of MapWindow size)
@function peer_effect
@within effects
 */
static int nscript_peer_effect(lua_State *L) {
	uint16 x, y;
	uint8 z;

	Game::get_game()->get_player()->get_location(&x, &y, &z);

	AsyncEffect *e = new AsyncEffect(new PeerEffect((x - x % 8) - 18, (y - y % 8) - 18, z));
	e->run(EFFECT_PROCESS_GUI_INPUT);

	return 0;
}

/***
wing strike effect. A dragon flies across the screen. (U6)
@function wing_strike_effect
@within effects
 */
static int nscript_wing_strike_effect(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L, 1);

	if (actor != NULL) {
		AsyncEffect *e = new AsyncEffect(new WingStrikeEffect(actor));
		e->run();
	}

	return 0;
}

/***
hail storm effect (U6). Hail stones rain down on the mapwindow.
@function hail_storm_effect
@tparam MapCoord|x,y,z location
@within effects
 */
static int nscript_hail_storm_effect(lua_State *L) {
	MapCoord loc;
	if (nscript_get_location_from_args(L, &loc.x, &loc.y, &loc.z) == false)
		return 0;


	AsyncEffect *e = new AsyncEffect(new HailStormEffect(loc));
	e->run();


	return 0;
}

/***
wizard's eye effect (U6).
@function wizard_eye_effect
@tparam MapCoord|x,y,z start_location
@within effects
 */
static int nscript_wizard_eye_effect(lua_State *L) {
	MapCoord loc;
	uint16 duration = (uint16)luaL_checkinteger(L, 1);

	if (nscript_get_location_from_args(L, &loc.x, &loc.y, &loc.z, 2) == false)
		return 0;

	AsyncEffect *e = new AsyncEffect(new WizardEyeEffect(loc, duration));
	e->run(EFFECT_PROCESS_GUI_INPUT);


	return 0;
}

/***
play the game end sequence then quit.
The file ending.lua is used from the relevant game's script directory.
@function play_end_sequence
 */
static int nscript_play_end_sequence(lua_State *L) {
	get_cutscene()->moveToFront();

	Script::get_script()->play_cutscene("/ending.lua");

	Game::get_game()->quit();

	return 0;
}

/***
play a sound effect
@function play_sfx
@int sfx_id the sound effect index number.
@bool[opt=false] async_playback if true then the sfx is played back asynchronously.
 */
static int nscript_play_sfx(lua_State *L) {
	bool play_mode = SFX_PLAY_SYNC;
	uint16 sfx_id = (uint16)luaL_checkinteger(L, 1);
	if (lua_gettop(L) > 1) {
		if (lua_toboolean(L, 2) != 0 /*== true*/)
			play_mode = SFX_PLAY_ASYNC;
	}

	Script::get_script()->get_sound_manager()->playSfx(sfx_id, play_mode);

	return 0;
}

int nscript_u6llist_iter(lua_State *L) {
	U6Link **s_link = (U6Link **)luaL_checkudata(L, 1, "nuvie.U6Link");
	U6Link *link = *s_link;

	if (link == NULL || link->data == NULL)
		return 0;

	Obj *obj = (Obj *)link->data;
	nscript_obj_new(L, obj);

	retainU6Link(link->next);
	*s_link = link->next;

	releaseU6Link(link); // release old link object.

	return 1;
}

int nscript_u6llist_iter_recursive(lua_State *L) {
	Std::stack<U6Link *> **s_stack = (Std::stack<U6Link *> **)luaL_checkudata(L, 1, "nuvie.U6LinkRecursive");
	Std::stack<U6Link *> *s = *s_stack;

	if (s->empty() || s->top() == NULL)
		return 0;

	U6Link *link = s->top();

	Obj *obj = (Obj *)link->data;
	nscript_obj_new(L, obj);

	s->pop();
	if (link->next != NULL) {
		s->push(link->next);
		retainU6Link(link->next);
	}

	if (obj->container && obj->container->count() > 0) {
		s->push(obj->container->start());
		retainU6Link(obj->container->start());
	}

	releaseU6Link(link); // release old link object.

	return 1;
}

int nscript_party_iter(lua_State *L) {
	uint16 party_index = (uint16)lua_tointeger(L, lua_upvalueindex(1));

	if (party_index == Game::get_game()->get_party()->get_party_size())
		return 0;

	uint8 actor_num = Game::get_game()->get_party()->get_actor_num(party_index);

	lua_pushinteger(L, party_index + 1);
	lua_replace(L, lua_upvalueindex(1));

	nscript_new_actor_var(L, actor_num);

	return 1;
}

/***
Iterate through party members.
@function party_members
@usage
  local actor
  for actor in party_members() do
    actor.poisoned = false
  end
@within party
 */
static int nscript_party(lua_State *L) {
	lua_pushinteger(L, 0);
	lua_pushcclosure(L, &nscript_party_iter, 1);
	return 1;
}

int nscript_find_obj_iter(lua_State *L) {
	Obj *cur_obj = NULL;

	if (!lua_isnil(L, lua_upvalueindex(1)))
		cur_obj = nscript_get_obj_from_args(L, lua_upvalueindex(1));
	uint8 level = (uint8)lua_tointeger(L, lua_upvalueindex(2));
	bool match_frame_n = (bool)lua_toboolean(L, lua_upvalueindex(3));
	bool match_quality = (bool)lua_toboolean(L, lua_upvalueindex(4));

	if (cur_obj == NULL)
		return 0;

	ObjManager *obj_manager = Game::get_game()->get_obj_manager();
	Obj *next_obj = obj_manager->find_next_obj(level, cur_obj, match_frame_n, match_quality);

	if (next_obj == NULL) {
		lua_pushnil(L);
	} else {
		nscript_new_obj_var(L, next_obj);
	}
	lua_replace(L, lua_upvalueindex(1));

	lua_pushinteger(L, level);
	lua_replace(L, lua_upvalueindex(2));

	lua_pushboolean(L, match_frame_n);
	lua_replace(L, lua_upvalueindex(3));

	lua_pushboolean(L, match_quality);
	lua_replace(L, lua_upvalueindex(4));

	nscript_new_obj_var(L, cur_obj);

	return 1;
}

Obj *nscript_get_next_obj_from_area(U6Link **link, uint16 x, uint16 y, uint8 z, uint16 w, uint16 h, uint16 *xOffset, uint16 *yOffset) {
	if (*link != NULL) {
		Obj *obj = (Obj *)(*link)->data;
		*link = (*link)->next;
		return obj;
	}

	ObjManager *obj_manager = Game::get_game()->get_obj_manager();
	while (*yOffset < h) {
		U6LList *list = obj_manager->get_obj_list(x + *xOffset, y + *yOffset, z);

		(*xOffset)++;
		if (*xOffset == w) {
			(*yOffset)++;
			*xOffset = 0;
		}

		if (list) {
			*link = list->start();
			if (*link) {
				Obj *obj = (Obj *)(*link)->data;
				*link = (*link)->next;
				return obj;
			}
		}
	}

	return NULL;
}

int nscript_find_obj_from_area_iter(lua_State *L) {
	Obj *cur_obj = NULL;

	U6Link **s_link = (U6Link **)luaL_checkudata(L, lua_upvalueindex(1), "nuvie.U6Link");

	uint16 x = (uint16)lua_tointeger(L, lua_upvalueindex(2));
	uint16 y = (uint16)lua_tointeger(L, lua_upvalueindex(3));
	uint8 z = (uint8)lua_tointeger(L, lua_upvalueindex(4));
	uint16 width = (uint16)lua_tointeger(L, lua_upvalueindex(5));
	uint16 height = (uint16)lua_tointeger(L, lua_upvalueindex(6));
	uint16 xOffset = (uint16)lua_tointeger(L, lua_upvalueindex(7));
	uint16 yOffset = (uint16)lua_tointeger(L, lua_upvalueindex(8));

	releaseU6Link(*s_link); // release old link object.

	cur_obj = nscript_get_next_obj_from_area(s_link, x, y, z, width, height, &xOffset, &yOffset);

	retainU6Link(*s_link);

	if (cur_obj == NULL)
		return 0;

	lua_pushinteger(L, xOffset);
	lua_replace(L, lua_upvalueindex(7));

	lua_pushinteger(L, yOffset);
	lua_replace(L, lua_upvalueindex(8));

	nscript_new_obj_var(L, cur_obj);

	return 1;
}
/***
Iterate through all objects of a specific type on a given map level.
@function find_obj
@int z map level to search
@int obj_n object number to search for
@int[opt] frame_n filter search based on specific frame number
@int[opt] quality filter search based on specific quality value
@usage
  local loc = player_get_location()
  for obj in find_obj(loc.z, 223, 1) do
    if obj ~= nil then
      explode_obj(obj)
    end
  end
@within Object
 */
static int nscript_find_obj(lua_State *L) {
	uint8 level = (uint8)luaL_checkinteger(L, 1);
	uint16 obj_n = (uint16)luaL_checkinteger(L, 2);
	uint16 frame_n = 0;
	bool match_frame_n = OBJ_NOMATCH_FRAME_N;
	uint16 quality = 0;
	bool match_quality = OBJ_NOMATCH_QUALITY;

	if (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) {
		frame_n = (uint16)luaL_checkinteger(L, 3);
		match_frame_n = OBJ_MATCH_FRAME_N;
	}

	if (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) {
		quality = (uint16)luaL_checkinteger(L, 4);
		match_quality = OBJ_MATCH_QUALITY;
	}

	ObjManager *obj_manager = Game::get_game()->get_obj_manager();
	Obj *obj = obj_manager->find_obj(level, obj_n, quality, match_quality, frame_n, match_frame_n);
	if (obj != NULL) {
		nscript_new_obj_var(L, obj);
	} else {
		lua_pushnil(L);
	}

	lua_pushinteger(L, level);
	lua_pushboolean(L, match_frame_n);
	lua_pushboolean(L, match_quality);

	lua_pushcclosure(L, &nscript_find_obj_iter, 4);

	return 1;
}

/***
Iterate through all objects within a given area.
@function find_obj_from_area
@tparam MapCoord|x,y,z location location for the effect to take place
@int width width of area to search
@int height height of area to search
@within Object
 */
static int nscript_find_obj_from_area(lua_State *L) {
	MapCoord loc;
	int stackOffset = 4;
	if (nscript_get_location_from_args(L, &loc.x, &loc.y, &loc.z) == false)
		return 0;
	if (lua_istable(L, 1)) {
		stackOffset = 2;
	}

	uint16 width = (uint16)luaL_checkinteger(L, stackOffset++);
	uint16 height = (uint16)luaL_checkinteger(L, stackOffset);

	U6Link **p_link = (U6Link **)lua_newuserdata(L, sizeof(U6Link *));
	*p_link = NULL;

	luaL_getmetatable(L, "nuvie.U6Link");
	lua_setmetatable(L, -2);

	lua_pushinteger(L, loc.x);
	lua_pushinteger(L, loc.y);
	lua_pushinteger(L, loc.z);
	lua_pushinteger(L, width);
	lua_pushinteger(L, height);
	lua_pushinteger(L, 0); //cur x offset
	lua_pushinteger(L, 0); //cur y offset

	lua_pushcclosure(L, &nscript_find_obj_from_area_iter, 8);

	return 1;
}

/***
Set specific game timer counter (U6).
These counters are decremented each turn and are used for things like torch duration, eclipse etc.
@function timer_set
@int timer_idx
@int value
@within time
 */
static int nscript_timer_set(lua_State *L) {
	GameClock *clock = Game::get_game()->get_clock();

	uint8 timer_num = (uint8)luaL_checkinteger(L, 1);
	uint8 value = (uint8)luaL_checkinteger(L, 2);

	clock->set_timer(timer_num, value);

	return 0;
}

/***
Get specific game timer counter (U6).
These counters are decremented each turn and are used for things like torch duration, eclipse etc.
@function timer_get
@int timer_idx
@treturn integer value of the timer
@within time
 */
static int nscript_timer_get(lua_State *L) {
	GameClock *clock = Game::get_game()->get_clock();

	uint8 timer_num = (uint8)luaL_checkinteger(L, 1);

	lua_pushinteger(L, clock->get_timer(timer_num));

	return 1;
}

/***
Set all timer counters with a given value
@function timer_update_all
@int value
@within time
 */
static int nscript_timer_update_all(lua_State *L) {
	GameClock *clock = Game::get_game()->get_clock();

	uint8 value = (uint8)luaL_checkinteger(L, 1);

	clock->update_timers(value);

	return 0;
}

/***
Get the current year
@function clock_get_year
@treturn integer
@within time
 */
static int nscript_clock_get_year(lua_State *L) {
	GameClock *clock = Game::get_game()->get_clock();

	if (clock == NULL)
		return 0;

	lua_pushinteger(L, clock->get_year());

	return 1;
}

/***
Get the current month
@function clock_get_month
@treturn integer
@within time
 */
static int nscript_clock_get_month(lua_State *L) {
	GameClock *clock = Game::get_game()->get_clock();

	if (clock == NULL)
		return 0;

	lua_pushinteger(L, clock->get_month());

	return 1;
}

/***
Get the current day
@function clock_get_day
@treturn integer
@within time
 */
static int nscript_clock_get_day(lua_State *L) {
	GameClock *clock = Game::get_game()->get_clock();

	if (clock == NULL)
		return 0;

	lua_pushinteger(L, clock->get_day());

	return 1;
}

/***
Get the clock minute value
@function clock_get_minute
@treturn integer
@within time
 */
static int nscript_clock_get_minute(lua_State *L) {
	GameClock *clock = Game::get_game()->get_clock();

	lua_pushinteger(L, clock->get_minute());

	return 1;
}

/***
Get the clock hour value
@function clock_get_hour
@treturn integer the hour in 24 hour format
@within time
 */
static int nscript_clock_get_hour(lua_State *L) {
	GameClock *clock = Game::get_game()->get_clock();

	lua_pushinteger(L, clock->get_hour());

	return 1;
}

/***
Increase the game clock by a number of minutes
@function clock_inc
@int minutes the number of minutes to increment the clock by
@within time
 */
static int nscript_clock_inc(lua_State *L) {
	GameClock *clock = Game::get_game()->get_clock();

	uint16 minutes = (uint16)luaL_checkinteger(L, 1);

	clock->inc_minute(minutes);

	return 0;
}

/***
Set the current wind direction (U6).
@function wind_set_dir
@int direction new wind direction
 */
static int nscript_wind_set(lua_State *L) {
	Weather *weather = Game::get_game()->get_weather();
	uint8 wind_dir = (uint8)luaL_checkinteger(L, 1);

	weather->set_wind_dir(wind_dir);

	return 0;
}

/***
Get the current wind direction (U6).
@function wind_get_dir
@treturn integer
 */
static int nscript_wind_get(lua_State *L) {
	Weather *weather = Game::get_game()->get_weather();
	lua_pushinteger(L, weather->get_wind_dir());
	return 1;
}

/***
Get input from the keyboard
@function input_select
@string allowed_characters set to nil if all characters are permitted
@bool can_escape can the player use escape to skip input
@treturn string characters input by the player
@within io
 */
static int nscript_input_select(lua_State *L) {
	const char *allowed_chars = NULL;

	if (!lua_isnil(L, 1))
		allowed_chars = luaL_checkstring(L, 1);

	bool can_escape = lua_toboolean(L, 2);

	TextInputEffect *inputEffect = new TextInputEffect(allowed_chars, can_escape);
	AsyncEffect *e = new AsyncEffect(inputEffect);
	e->run(EFFECT_PROCESS_GUI_INPUT);

	Std::string input = inputEffect->get_input();

	lua_pushstring(L, input.c_str());

	return 1;
}

/***
Get an integer number as input from the player.
The characters input by the player are converted into an integer using the
C function strtol()
@function input_select_integer
@string allowed_characters set to nil if all characters are permitted
@bool can_escape can the player use escape to skip input
@treturn integer number input by the player
@within io
 */
static int nscript_input_select_integer(lua_State *L) {
	const char *allowed_chars = NULL;

	if (!lua_isnil(L, 1))
		allowed_chars = luaL_checkstring(L, 1);

	bool can_escape = lua_toboolean(L, 2);

	TextInputEffect *inputEffect = new TextInputEffect(allowed_chars, can_escape);
	AsyncEffect *e = new AsyncEffect(inputEffect);
	e->run(EFFECT_PROCESS_GUI_INPUT);

	Std::string input = inputEffect->get_input();

	int num = (int)strtol(input.c_str(), (char **)NULL, 10);
	lua_pushinteger(L, num);

	return 1;
}

/***
Iterate through objects at a given map location
@function objs_at_loc
@tparam MapCoord|x,y,z location
@within Object
 */
static int nscript_objs_at_loc(lua_State *L) {
	U6Link *link = NULL;
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();

	uint16 x, y;
	uint8 z;

	if (nscript_get_location_from_args(L, &x, &y, &z) == false)
		return 0;

	if (x < 1024 && y < 1024 && z <= 5) {
		U6LList *obj_list = obj_manager->get_obj_list(x, y, z);
		if (obj_list != NULL)
			link = obj_list->start();
	} else {
		DEBUG(0, LEVEL_ERROR, "objs_at_loc() Invalid coordinates (%d, %d, %d)\n", x, y, z);
	}

	lua_pushcfunction(L, nscript_u6llist_iter);

	U6Link **p_link = (U6Link **)lua_newuserdata(L, sizeof(U6Link *));
	*p_link = link;

	if (link)
		retainU6Link(link);

	luaL_getmetatable(L, "nuvie.U6Link");
	lua_setmetatable(L, -2);

	return 2;
}

/***
Find a volcano object near the player. This function searches an area +/- 5 tiles around the location specified.
The search starts top left and finishes bottom right. The first volcano/fumarole object found is returned.
@function find_volcano_near_player
@tparam MapCoord|x,y,z location centre of the search area
@treturn Obj|nil first volcano/fumarole object found or nil if none found.
@within Object
 */
static int nscript_find_volcano_obj_near_player(lua_State *L) {
	uint16 x, y;
	uint8 z;
	const uint8 range = 5;
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();

	Game::get_game()->get_player()->get_location(&x, &y, &z);

	for (uint16 i = y - range; i < y + range; i++) {
		for (uint16 j = x - range; j < x + range; j++) {
			U6LList *obj_list = obj_manager->get_obj_list(j, i, z);
			if (obj_list) {
				for (U6Link *link = obj_list->start(); link; link = link->next) {
					Obj *o = (Obj *)link->data;
					if (o->obj_n == OBJ_U6_VOLCANO || o->obj_n == OBJ_U6_FUMAROLE) {
						nscript_new_obj_var(L, o);
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

/***
Iterate through objects in a container.
@function container_objs
@tparam Obj container container object to search.
@bool[opt=false] is_recursive should we search containers inside the container?
@usage
  local child
  for child in container_objs(obj) do  -- look through container for effect object.
    if child.obj_n == 337 then --effect
      found = true
      print("\nIt's trapped.\n");
      break
    end
  end
@within Object
 */
static int nscript_container(lua_State *L) {
	bool is_recursive = false;
	Obj **s_obj = (Obj **)luaL_checkudata(L, 1, "nuvie.Obj");
	Obj *obj;

	obj = *s_obj;

	U6LList *obj_list = obj->container;

	if (lua_gettop(L) >= 2)
		is_recursive = lua_toboolean(L, 2);

	return nscript_init_u6link_iter(L, obj_list, is_recursive);
}

int nscript_init_u6link_iter(lua_State *L, U6LList *list, bool is_recursive) {
	U6Link *link = NULL;

	if (list != NULL)
		link = list->start();

	retainU6Link(link);

	if (is_recursive) {
		lua_pushcfunction(L, nscript_u6llist_iter_recursive);

		Std::stack<U6Link *> **p_stack = (Std::stack<U6Link *> **)lua_newuserdata(L, sizeof(Std::stack<U6Link *> *));
		*p_stack = new Std::stack<U6Link *>();
		(*p_stack)->push(link);

		luaL_getmetatable(L, "nuvie.U6LinkRecursive");
	} else {
		lua_pushcfunction(L, nscript_u6llist_iter);

		U6Link **p_link = (U6Link **)lua_newuserdata(L, sizeof(U6Link *));
		*p_link = link;

		luaL_getmetatable(L, "nuvie.U6Link");
	}

	lua_setmetatable(L, -2);

	return 2;
}

/***
Is the god mode cheat currently active?
@function is_god_mode_enabled
@treturn bool
 */
static int nscript_is_god_mode_enabled(lua_State *L) {
	bool god_mode = Game::get_game()->is_god_mode_enabled();
	lua_pushboolean(L, god_mode);
	return 1;
}

/***
Set armageddon flag
@function set_armageddon
@bool new_value
 */
static int nscript_set_armageddon(lua_State *L) {
	Game::get_game()->set_armageddon((bool)lua_toboolean(L, 1));
	return 0;
}

/***
Toggle mouse cursor visibility.
@function mouse_cursor_visible
@bool visible
@within io
 */
static int nscript_mouse_cursor_show(lua_State *L) {
	bool show_cursor = lua_toboolean(L, 1);
	Cursor *cursor = Game::get_game()->get_cursor();
	if (cursor) {
		if (show_cursor) {
			cursor->show();
		} else {
			cursor->hide();
		}
	}
	return 0;
}

/***
Select which mouse cursor pointer to use.
@function mouse_cursor_set_pointer
@int pointer_index
@within io
 */
static int nscript_mouse_cursor_set_pointer(lua_State *L) {
	uint8 new_pointer = lua_tointeger(L, 1);
	Cursor *cursor = Game::get_game()->get_cursor();
	if (cursor) {
		cursor->set_pointer(new_pointer);
	}

	return 0;
}

/***
Pause the script. Tile animation will continue while the script is paused.
@function script_wait
@int duration in milliseconds
@within io
 */
static int nscript_wait(lua_State *L) {
	uint32 duration = (uint32)luaL_checkinteger(L, 1);

	Game::get_game()->get_map_window()->updateAmbience();
	Game::get_game()->get_view_manager()->update();

	AsyncEffect *e = new AsyncEffect(new TimedEffect(duration));
	e->run(EFFECT_PROCESS_GUI_INPUT);


	return 0;
}

/***
Centre the mapwindow at a given location.
@function mapwindow_center_at_location
@int x
@int y
@int z
@within mapwindow
 */
static int nscript_mapwindow_center_at_loc(lua_State *L) {
	MapWindow *map_window = Game::get_game()->get_map_window();

	uint16 x = (uint16) luaL_checkinteger(L, 1);
	uint16 y = (uint16) luaL_checkinteger(L, 2);
	uint8 z = (uint8) luaL_checkinteger(L, 3);

	map_window->centerMap(x, y, z);

	return 0;
}

/***
Get the current location that the mapwindow is displaying. This is the top left corner
of the mapwindow.
@function mapwindow_get_location
@treturn MapCoord
@within mapwindow
 */
static int nscript_mapwindow_get_loc(lua_State *L) {
	MapWindow *map_window = Game::get_game()->get_map_window();

	uint16 x = map_window->get_cur_x();
	uint16 y = map_window->get_cur_y();
	uint8 z;
	map_window->get_level(&z);

	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushinteger(L, x);
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushinteger(L, y);
	lua_settable(L, -3);

	lua_pushstring(L, "z");
	lua_pushinteger(L, z);
	lua_settable(L, -3);

	return 1;
}

/***
Set the current location that the mapwindow is displaying. This is the top left corner
@function mapwindow_set_location
@int x
@int y
@int z
@within mapwindow
 */
static int nscript_mapwindow_set_loc(lua_State *L) {
	MapWindow *map_window = Game::get_game()->get_map_window();

	uint16 x = (uint16) luaL_checkinteger(L, 1);
	uint16 y = (uint16) luaL_checkinteger(L, 2);
	uint8 z = (uint8) luaL_checkinteger(L, 3);

	map_window->moveMap(x, y, z);

	map_window->set_enable_blacking(false);

	return 0;
}

/***
Toggle mapwindow 'blacking'. Blacking hides tiles that are not visible to the player because they are obscured
by a wall.
@function mapwindow_set_enable_blacking
@bool enable_blacking
@within mapwindow
 */
static int nscript_mapwindow_set_enable_blacking(lua_State *L) {
	MapWindow *map_window = Game::get_game()->get_map_window();

	bool enable_blacking = lua_toboolean(L, 1);

	map_window->set_enable_blacking(enable_blacking);

	return 0;
}

/***
Loads text from a given LZC file.
@function load_text_from_lzc
@string filename the lzc file to extract the text from
@int index offset in the lzc file to load the text from
@treturn string the extracted text
 */
static int nscript_load_text_from_lzc(lua_State *L) {
	unsigned char *buf = NULL;
	Std::string filename(lua_tostring(L, 1));
	U6Lib_n lib_n;

	Std::string path;

	config_get_path(Game::get_game()->get_config(), filename, path);

	if (!lib_n.open(path, 4, NUVIE_GAME_MD)) {
		return 0;
	}
	int idx = lua_tointeger(L, 2);
	if (idx >= (int)lib_n.get_num_items()) {
		return 0;
	}

	buf = lib_n.get_item(idx, NULL);
	if (!buf) {
		return 0;
	}

	int len = lib_n.get_item_size(idx);
	lib_n.close();

	if (len < 1 || buf[len - 1] != 0) {
		free(buf);
		return 0;
	}

	if (len >= 2 && buf[len - 2] == 0xff) {
		buf[len - 2] = 0x0;
	}

	lua_pushstring(L, (const char *)buf);
	free(buf);

	return 1;
}

/***
Display string in scroll gump if in new style. Otherwise display on regular message scroll.
@function display_text_in_scroll_gump
@string text the text to display in the scroll
@within UI
 */
static int nscript_display_text_in_scroll_gump(lua_State *L) {
	const char *text = lua_tostring(L, 1);
	if (text) {
		if (Game::get_game()->is_new_style())
			Game::get_game()->get_view_manager()->open_scroll_gump(text, strlen(text));
		else
			Game::get_game()->get_scroll()->message(text);
	}
	return 0;
}

/***
Lock the inventory view to a specific Actor.
@function lock_inventory_view
@tparam Actor actor
@within UI
 */
static int nscript_lock_inventory_view(lua_State *L) {
	Actor *actor = nscript_get_actor_from_args(L, 1);
	Game::get_game()->get_view_manager()->get_inventory_view()->set_actor(actor, true);
	Game::get_game()->get_view_manager()->get_inventory_view()->lock_to_actor(true);
	Game::get_game()->get_view_manager()->set_inventory_mode();
	return 0;
}

/***
Unlock the inventory view
@function unlock_inventory_view
@within UI
 */
static int nscript_unlock_inventory_view(lua_State *L) {
	Game::get_game()->get_view_manager()->get_inventory_view()->lock_to_actor(false);
	Game::get_game()->get_view_manager()->get_inventory_view()->set_party_member(0);
	Game::get_game()->get_view_manager()->set_inventory_mode();
	return 0;
}

} // End of namespace Nuvie
} // End of namespace Ultima
