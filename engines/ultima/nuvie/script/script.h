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

#ifndef NUVIE_SCRIPT_SCRIPT_H
#define NUVIE_SCRIPT_SCRIPT_H

#include "common/lua/lua.h"

#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/usecode/usecode.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class MapCoord;
class Spell;
class Obj;
class Actor;
class NuvieIO;
class SoundManager;

#define NUVIE_SCRIPT_NOT_STARTED     255
#define NUVIE_SCRIPT_ERROR             0
#define NUVIE_SCRIPT_FINISHED          1
#define NUVIE_SCRIPT_GET_TARGET        2
#define NUVIE_SCRIPT_GET_DIRECTION     3
#define NUVIE_SCRIPT_GET_INV_OBJ       4
#define NUVIE_SCRIPT_ADVANCE_GAME_TIME 5
#define NUVIE_SCRIPT_ADVANCE_REAL_TIME 6
#define NUVIE_SCRIPT_TALK_TO_ACTOR     7
#define NUVIE_SCRIPT_GET_SPELL         8
#define NUVIE_SCRIPT_GET_OBJ           9
#define NUVIE_SCRIPT_GET_PLAYER_OBJ   10 //can get an object immediately surrounding the player or from their inventory.

#define NUVIE_SCRIPT_CB_ADV_GAME_TIME  1

class ScriptThread {
	lua_State *L;
	int start_nargs;
	uint32 data;
	uint8 state;

public:

	ScriptThread(lua_State *l, int nargs) {
		L = l;
		start_nargs = nargs;
		data = 0;
		state = NUVIE_SCRIPT_NOT_STARTED;
	}
	~ScriptThread() {  }
	uint32 get_data() {
		return data;
	}
	uint8 start() {
		return resume(start_nargs);
	}
	uint8 resume_with_location(MapCoord loc);
	uint8 resume_with_direction(uint8 dir);
	uint8 resume_with_spell_num(uint8 spell_num);
	uint8 resume_with_obj(Obj *obj);
	uint8 resume_with_nil();
	uint8 resume(int narg = 0);
	bool finished() {
		return lua_status(L) != LUA_YIELD ? true : false;
	}
	int get_error() {
		return lua_status(L);
	}

	bool is_running() {
		return !finished();
	}
	uint8 get_state() {
		return state;
	}
};

#define SCRIPT_DISPLAY_HIT_MSG true
class Script {
	static Script *script;
	Configuration *config;
	nuvie_game_t gametype; // what game is being played?
	SoundManager *soundManager;
	lua_State *L;

public:

	Script(Configuration *cfg, GUI *gui, SoundManager *sm, nuvie_game_t type);
	~Script();

	bool init();

	/* Return instance of self */
	static Script *get_script()           {
		return (script);
	}
	Configuration *get_config() {
		return (config);
	}
	SoundManager *get_sound_manager() {
		return soundManager;
	}

	bool run_script(const char *script);
	bool call_load_game(NuvieIO *objlist);
	bool call_save_game(NuvieIO *objlist);

	bool play_cutscene(const char *script_file);
	MovementStatus call_player_before_move_action(sint16 *rel_x, sint16 *rel_y);
	bool call_player_post_move_action(bool didMove);
	bool call_player_pass();
	bool call_actor_update_all();
	bool call_actor_init(Actor *actor, uint8 alignment);
	bool call_actor_attack(Actor *actor, MapCoord location, Obj *weapon, Actor *foe);
	bool call_actor_map_dmg(Actor *actor, MapCoord location);
	bool call_actor_tile_dmg(Actor *actor, uint16 tile_num);
	bool call_actor_hit(Actor *actor, uint8 dmg, bool display_hit_msg = false);
	uint8 call_actor_str_adj(Actor *actor);
	uint8 call_actor_dex_adj(Actor *actor);
	uint8 call_actor_int_adj(Actor *actor);
	bool call_look_obj(Obj *obj);
	int call_obj_get_readiable_location(Obj *obj);
	uint8 actor_get_max_magic_points(Actor *actor);
	bool call_actor_get_obj(Actor *actor, Obj *obj, Obj *container = NULL);
	bool call_actor_subtract_movement_points(Actor *actor, uint8 points);
	bool call_actor_resurrect(Actor *actor);
	bool call_use_keg(Obj *obj); //we need this until we move all usecode into script.
	bool call_has_usecode(Obj *obj, UseCodeEvent usecode_type);
	ScriptThread *call_use_obj(Obj *obj, Actor *actor);
	bool call_ready_obj(Obj *obj, Actor *actor);
	bool call_move_obj(Obj *obj, sint16 rel_x, sint16 rel_y);
	bool call_handle_alt_code(uint16 altcode);

	bool call_magic_get_spell_list(Spell **spell_list);
	bool call_actor_use_effect(Obj *effect_obj, Actor *actor);
	bool call_function(const char *func_name, int num_args, int num_return, bool print_stacktrace = true);
	ScriptThread *call_function_in_thread(const char *function_name);
	bool run_lua_file(const char *filename);
	bool call_moonstone_set_loc(uint8 phase, MapCoord location); //this is a hack until we have 'use' moonstone in script.
	bool call_advance_time(uint16 minutes);
	bool call_can_get_obj_override(Obj *obj);
	bool call_out_of_ammo(Actor *attacker, Obj *weapon, bool print_message);
	bool call_is_avatar_dead();
	bool call_is_ranged_select(UseCodeType operation);
	bool call_set_g_show_stealing(bool stealing);
	uint8 call_get_combat_range(uint16 absx, uint16 absy);
	uint8 call_get_weapon_range(uint16 obj_n);

	MapCoord call_moonstone_get_loc(uint8 phase);
	bool call_update_moongates(bool visible);

	uint8 call_play_midgame_sequence(uint16 seq_num);
	bool call_talk_script(uint8 script_number);
	bool call_talk_to_obj(Obj *obj);
	bool call_talk_to_actor(Actor *actor);
	bool call_is_container_obj(uint16 obj_n);
	uint8 call_get_portrait_number(Actor *actor);
	bool call_player_attack();

	uint16 call_get_tile_to_object_mapping(uint16 tile_n);
	bool call_is_tile_object(uint16 obj_n);

	ScriptThread *new_thread(const char *scriptfile);
	ScriptThread *new_thread_from_string(const char *script);

protected:
	bool call_loadsave_game(const char *function, NuvieIO *objlist);
	void seed_random();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
