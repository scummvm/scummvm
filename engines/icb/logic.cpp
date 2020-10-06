/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/debug.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/px_game_object.h"
#include "engines/icb/common/px_maths.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/mission.h"
#include "engines/icb/session.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/res_man.h"
#include "engines/icb/light.h"

namespace ICB {

mcodeFunctionReturnCodes fn_get_weapon(int32 &result, int32 *params) { return (MS->fn_get_weapon(result, params)); }

mcodeFunctionReturnCodes fn_pass_flag_to_engine(int32 &result, int32 *params) { return (MS->fn_pass_flag_to_engine(result, params)); }

mcodeFunctionReturnCodes fn_restart_object(int32 &result, int32 *params) { return (MS->fn_restart_object(result, params)); }

mcodeFunctionReturnCodes fn_get_persons_weapon(int32 &result, int32 *params) { return (MS->fn_get_persons_weapon(result, params)); }

mcodeFunctionReturnCodes fn_kill_me(int32 &result, int32 *params) { return (MS->fn_kill_me(result, params)); }

mcodeFunctionReturnCodes fn_kill_object(int32 &result, int32 *params) { return (MS->fn_kill_object(result, params)); }

mcodeFunctionReturnCodes fn_new_script(int32 &result, int32 *params) { return (MS->fn_new_script(result, params)); }

mcodeFunctionReturnCodes fn_gosub(int32 &result, int32 *params) { return (MS->fn_gosub(result, params)); }

mcodeFunctionReturnCodes fn_set_to_exlusive_coords(int32 &result, int32 *params) { return (MS->fn_set_to_exlusive_coords(result, params)); }

mcodeFunctionReturnCodes fn_object_rerun_logic_context(int32 &result, int32 *params) { return (MS->fn_object_rerun_logic_context(result, params)); }

mcodeFunctionReturnCodes fn_set_strike_overide(int32 &result, int32 *params) { return (MS->fn_set_strike_overide(result, params)); }

mcodeFunctionReturnCodes fn_set_shoot_overide(int32 &result, int32 *params) { return (MS->fn_set_shoot_overide(result, params)); }

mcodeFunctionReturnCodes fn_is_player_running(int32 &result, int32 *params) { return (MS->fn_is_player_running(result, params)); }

mcodeFunctionReturnCodes fn_is_player_walking(int32 &result, int32 *params) { return (MS->fn_is_player_walking(result, params)); }

mcodeFunctionReturnCodes fn_set_dynamic_light(int32 &result, int32 *params) { return (MS->fn_set_dynamic_light(result, params)); }

mcodeFunctionReturnCodes speak_set_dynamic_light(int32 &result, int32 *params) { return (MS->speak_set_dynamic_light(result, params)); }

mcodeFunctionReturnCodes fn_activate_sparkle(int32 &result, int32 *params) { return (MS->fn_activate_sparkle(result, params)); }

mcodeFunctionReturnCodes fn_deactivate_sparkle(int32 &result, int32 *params) { return (MS->fn_deactivate_sparkle(result, params)); }

// fn_activate_sparkle(x,y,z);
mcodeFunctionReturnCodes _game_session::fn_activate_sparkle(int32 &, int32 *params) {
	PXreal rx, ry, rz;

	L->GetPosition(rx, ry, rz);

	int x, y, z;
	x = (int)rx;
	y = (int)ry;
	z = (int)rz;

	L->sparkleX = (short)(params[0] - x);
	L->sparkleY = (short)(params[1] - y);
	L->sparkleZ = (short)(params[2] - z);

	L->sparkleOn = TRUE8;

	return IR_CONT;
}

// fn_deactivate_sparkle(x,y,z);
mcodeFunctionReturnCodes _game_session::fn_deactivate_sparkle(int32 &, int32 *) {
	L->sparkleOn = FALSE8;

	return IR_CONT;
}

// these are the con and des-tructors for the game object objects
// they are homeless and so stay here
void _logic::___init(const char *name) {
	int j;

	// set objects name
	Set_string(name, ob_name);

	// This object is active
	ob_status = OB_STATUS_NOT_HELD;

	// give full run rights
	big_mode = __SCRIPT;

	// clear the pointer to the voxel object specific structure
	// if these are NULL then the object is non mega
	voxel_info = NULL;
	mega = NULL; // clear mega info

	// defaults to a prop
	image_type = PROP;

	// defaults to no-type-set
	object_type = __NO_TYPE_SET;

	// defaults to responding to events, LOS etc.
	do_not_disturb = FALSE8;

	// set crude switch to off
	context_request = FALSE8;

	// this is important - reset the looping flag
	looping = 0;
	pause = 0;

	// an object must register itself for interaction
	player_can_interact = FALSE8;

	// reset pan adjust value
	pan_adjust = ZERO_TURN;

	// reset auto pan stuff
	auto_display_pan = ZERO_TURN;
	auto_panning = FALSE8;

	// clear the logic tree
	for (j = 0; j < TREE_SIZE; j++) {
		logic[j] = 0;
		logic_ref[j] = 0;
	}

	// set owner rect to something safe - for fn_on_screen calls by props
	owner_floor_rect = 0;

	cur_anim_type = __NO_ANIM;

	// reset the custom script list to 0 items
	total_list = 0;

	// defualt to tight pan interact type
	three_sixty_interact = FALSE8;

	// defaul to no prop coords set
	prop_coords_set = FALSE8;

	// not in a conversation
	conversation_uid = NO_SPEECH_REQUEST;

	// height is straight ahead...
	look_height = -1;

	// logic culling
	hold_mode = none;
	camera_held = FALSE8;

	// all sound effects to default
	sfxVars[0] = sfxVars[1] = sfxVars[2] = 0;

	// sparkle off by default
	sparkleOn = FALSE8;
}

void _mega::___init() {
	// need to reset target_pan in the same way as we need to reset looping in _logic()
	auto_target_pan = ZERO_TURN; // auto target

	target_pan = ZERO_TURN; // reset turn-on-spot-to pan
	cur_parent = NULL;
	cur_slice = 0;
	number_of_barriers = 0;  // number of local barriers associated with mega position
	number_of_nudge = 0;     // number of local barriers associated with mega position
	number_of_animating = 0; // animating barriers
	target_id = 0;
	interacting = FALSE8;

	// set some chr$ defaults
	weapon = __NOT_SET;
	motion = __MOTION_WALK;
	custom = FALSE8;
	has_exclusive_coords = FALSE8; // cord and chi may overide this
	is_evil = FALSE8;
	make_remora_beep = FALSE8;
	m_phase = 0;

	m_main_route.total_points = 0; // final route size
	m_main_route.diag_bars = 0;
	m_main_route.number_of_diag_bars = 0;

	use_strike_script = 0;
	use_fire_script = 0;
	on_players_floor = FALSE8;
	anim_speed = 1;
	pushed = 0; // coords not pushed
	reverse_route = 0;

	// Set a default speech colour for megas.
	speech_red = 0;
	speech_green = 230;
	speech_blue = 230;

	next_anim_type = __NO_ANIM;

	// initialise the dynamic light...
	InitDynamicLight();

	// Not currently shooting
	is_shooting = FALSE8;
	drawShadow = TRUE8; // shadows on

	// async
	asyncing = 0;       // not loading file
	async_list_pos = 0; // start of list
	async_weapon = __NOT_SET;

	// generic stair info for shadow correction
	on_stairs = TRUE8;

	inShadePercentage = DEFAULT_INSHADE_PERCENTAGE; // default shade value

	// footstep stuff
	footstep_status = 0;       // current foot left/right status...
	footstep_weight = 100;     // weight of footstep (100 is Cord) 255 maximum...
	footstep_special = FALSE8; // whether special or not...

	// default router extrapolation size
	extrap_size = 25;

	// set to draw
	display_me = TRUE8;

	dead = FALSE8; // still alive!

	// camera control
	y_locked = FALSE8;

	// breath off as default
	breath.on = 0;
}

mcodeFunctionReturnCodes _game_session::fn_set_to_exlusive_coords(int32 &, int32 *) {
	if (!logic_structs[cur_id]->mega)
		Fatal_error("terminal misuse of fn_set_to_exclusive_coords");

	logic_structs[cur_id]->mega->has_exclusive_coords = TRUE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_get_persons_weapon(int32 &result, int32 *params) {
	// return the weapon type to the script

	// params        0   name of mega

	uint32 id;

	const char *mega_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	id = objects->Fetch_item_number_by_name(mega_name);
	if (id == 0xffffffff)
		Fatal_error("fn_get_persons_weapon: object [%s] does not exist", mega_name);

	if (!logic_structs[id]->mega)
		Fatal_error("fn_get_persons_weapon: object [%s] not a mega", mega_name);

	result = logic_structs[id]->mega->Fetch_armed_status();

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_get_weapon(int32 &result, int32 *) {
	// return the weapon type to the script
	// no params

	result = Fetch_cur_megas_armed_status();

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_is_player_running(int32 &result, int32 *) {
	if (player.player_status == RUNNING)
		result = 1;
	else
		result = 0;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_player_walking(int32 &result, int32 *) {
	if (player.player_status == WALKING)
		result = 1;
	else
		result = 0;

	return IR_CONT;
}

bool8 _game_session::Fetch_cur_megas_armed_status() {
	// return a megas armed status

	if (!M)
		Fatal_error("%d not a mega but called Fetch_megas_weapon_type", cur_id);

	return (M->Fetch_armed_status());
}

__weapon _game_session::Fetch_cur_megas_pose() {
	// return a megas weapon type - can be called from outside of _game_session

	if (!M)
		Fatal_error("%d not a mega but called Fetch_megas_weapon_type", cur_id);

	return (M->Fetch_pose());
}

const char *_game_session::Fetch_cur_megas_custom_text() {
	// return pointer to megas custom ascii - mine, object, etc

	if (!M)
		Fatal_error("%d not a mega but called Fetch_megas_weapon_type", cur_id);

	return (M->custom_set);
}

bool8 _game_session::Fetch_custom() {
	// return a megas weapon type - can be called from outside of _game_session

	if (!M)
		Fatal_error("%d not a mega but called Fetch_cur_megas_custom_type", cur_id);

	return (M->Fetch_custom());
}

bool8 _mega::Fetch_custom(void) {
	// return custom anim type from _mega struct

	return (custom);
}

void _game_session::Reset_cur_megas_custom_type() {
	// resets to __NONE the current custom type
	// this is probably desireable and will save scripters doing it - or not and forgetting

	if (!M)
		Fatal_error("%d not a mega but called Reset_cur_megas_custom_type", cur_id);

	M->custom = FALSE8; //__NONE;
}

bool8 _mega::Fetch_armed_status() {
	// return weapon type from _mega struct
	// see also _game_session::Fetch_cur_megas_weapon_type

	return (armed_state_table[weapon]);
}

bool8 _mega::Is_crouched(void) {
	// is the mega croucing - called by LOS

	return (crouch_state_table[weapon]);
}

__weapon _mega::Fetch_pose(void) {
	// fetch custume subset - i.e crouch, crouch_gun, unarmed, etc.

	return (weapon);
}

void _game_session::Set_script(const char *script_name) {
	// set the script on the current level
	char *ad;

	ad = (char *)scripts->Fetch_item_by_name(script_name);

	L->logic[L->logic_level] = ad;

	// write reference for change script checks later - i.e. FN_context_chosen_script
	L->logic_ref[L->logic_level] = ad;
}

void _game_session::Context_check(uint32 script_name) {
	// we have been passed a script name - we need to check if this script is the same or
	// different from the one currently running on logic level 1. If the same then we do
	// nothing. If different then we set the level 1 logic to this new script

	char *ad;

	Zdebug("context check");

	ad = (char *)scripts->Try_fetch_item_by_hash(script_name);

	Zdebug("context_check ad=%d ref=%d", ad, L->logic_ref[1]);

	if (L->logic_ref[1] != ad) {
		// write actual offset
		L->logic[1] = ad;

		// write reference for change script checks later - i.e. FN_context_chosen_script
		L->logic_ref[1] = ad;

		L->logic_level = 1; // reset to level 1

		L->looping = 0; // reset the logic state flag

		if (L->mega)
			M->custom = FALSE8; // reset

		L->pause = 0;
	}
}

mcodeFunctionReturnCodes fn_context_chosen_logic(int32 &result, int32 *params) {
	// the logic context script has chosen a logic to set up but we do nothing if the script is running already
	// this function is used for an immediate logic change - i.e. it wont wait for animations to finish first

	//	params[0]    ascii name of new script

	return (MS->fn_context_chosen_logic(result, params));
}

mcodeFunctionReturnCodes _game_session::fn_context_chosen_logic(int32 &, int32 *params) {
	// pass the (hashed) name
	Context_check(params[0]); // now # number

	return (IR_TERMINATE); // get us back
}

void _game_session::Shut_down_object() {
	// lock out an object

	// Tell the event manager to stop handling events for this object.  [PS 09/12/98]
	g_oEventManager->ShutDownEventProcessingForObject(cur_id);

	logic_structs[cur_id]->ob_status = OB_STATUS_HELD; // lock out

	prop_state_table[cur_id] = 0; // set to state 0 - in case killed because of illegal frame

	Tdebug("objects_that_died.txt", "**OBJECT '%s' [id=%d] has been shut down**", object->GetName(), cur_id);
}

mcodeFunctionReturnCodes _game_session::fn_kill_me(int32 &, int32 *) {
	// kill this object

	Shut_down_object("fn_kill_me");

	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_kill_object(int32 &, int32 *params) {
	// kill this object
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	uint32 id = objects->Fetch_item_number_by_name(object_name);

	if (id == 0xffffffff)
		Fatal_error("fn_kill_object finds [%s] does not exist", object_name);

	if (id == cur_id)
		Fatal_error("fn_kill_object - dont use this function to shut self down [%s]", object_name);

	// Tell the event manager to stop handling events for this object.
	g_oEventManager->ShutDownEventProcessingForObject(id);

	prop_state_table[id] = 0; // set to state 0 - in case killed because of illegal frame

	logic_structs[id]->ob_status = OB_STATUS_HELD; // lock out

	Tdebug("objects_that_died.txt", "**OBJECT '%s' shut down by fn_kill_object", object_name);

	return (IR_CONT);
}

void _game_session::Shut_down_object(const char *ascii) {
	// lock out an object

	// Tell the event manager to stop handling events for this object.
	g_oEventManager->ShutDownEventProcessingForObject(cur_id);

	logic_structs[cur_id]->ob_status = OB_STATUS_HELD; // lock out

	prop_state_table[cur_id] = 0; // set to state 0 - in case killed because of illegal frame

	Tdebug("objects_that_died.txt", "**OBJECT '%s' [id=%d] has been shut down** %s", object->GetName(), cur_id, ascii);
}

bool8 _game_session::Console_shut_down_object(const char *name) {
	// we have name of object

	uint32 id = objects->Fetch_item_number_by_name(name);
	if (id == 0xffffffff)
		return (FALSE8);

	// Tell the event manager to stop handling events for this object.
	g_oEventManager->ShutDownEventProcessingForObject(id);

	prop_state_table[id] = 0; // set to state 0 - in case killed because of illegal frame

	logic_structs[id]->ob_status = OB_STATUS_HELD; // lock out

	Tdebug("objects_that_died.txt", "**OBJECT '%s' [id=%d] has been shut down** %s", name, id, "Console_shut_down_object");

	return (TRUE8);
}

bool8 _game_session::Free_object(const char *name) {
	// we have name of object
	uint32 id = objects->Fetch_item_number_by_name(name);

	if (id == 0xffffffff)
		return (FALSE8);

	logic_structs[id]->ob_status = OB_STATUS_NOT_HELD; // lock out

	Tdebug("objects_that_died.txt", "**OBJECT '%s' [id=%d] has been free'd by user ** %s", name, id, "Free_object");

	return (TRUE8);
}

void _game_session::Console_shut_down_all_mega_objects() {
	// how many objs in the mission
	uint32 tot_obs = Fetch_number_of_objects();

	if (tot_obs) {
		Tdebug("objects_that_died.txt", "\n\nuser shutting down all mega objects");

		for (uint32 j = 0; j < tot_obs; j++)
			if (logic_structs[j]->mega)
				Shut_down_id(j);
	}

	Tdebug("objects_that_died.txt", "\n\n");
}

void _game_session::Console_shut_down_all_objects() {
	// how many objs in the mission
	uint32 tot_obs = Fetch_number_of_objects();

	if (tot_obs) {
		Tdebug("objects_that_died.txt", "\n\nuser shutting down all objects");

		for (uint32 j = 0; j < tot_obs; j++)
			Shut_down_id(j);
	}

	Tdebug("objects_that_died.txt", "\n\n");
}

void _game_session::Shut_down_id(uint32 id) {
	// we have id of object

	// must be legal id
	_ASSERT(id < MS->Fetch_number_of_objects());

	logic_structs[id]->ob_status = OB_STATUS_HELD; // lock out

	Tdebug("objects_that_died.txt", "**OBJECT %s [id=%d] has been shut down** %s", (const char *)logic_structs[id]->GetName(), id, "Shut_down_id");
}

mcodeFunctionReturnCodes fn_shut_down_object(int32 &result, int32 *params) {
	// shut down current object - wont be logic processed any int32er
	return (MS->fn_shut_down_object(result, params));
}

mcodeFunctionReturnCodes _game_session::fn_shut_down_object(int32 & /*result*/, int32 * /*params*/) {
	//	params   none

	Shut_down_object();

	return (IR_STOP);
}

mcodeFunctionReturnCodes fn_pause(int32 &result, int32 *params) {
	// shut down current object - wont be logic processed any int32er
	return (MS->fn_pause(result, params));
}

mcodeFunctionReturnCodes _game_session::fn_pause(int32 &, int32 *params) {
	//	params   1 = pause value
	// params:  0 pointer to object's logic structure
	//          1 number of game-cycles to pause

	// NB. Pause-value of 0 causes script to continue, 1 causes a 1-cycle quit, 2 gives 2 cycles, etc.

	if (!L->looping) { // start the pause
		L->looping = 1;
		L->pause = params[0]; // no. of game cycles
	}

	if (L->pause) {             // if non-zero
		L->pause--;         // decrement the pause count
		return (IR_REPEAT); // drop out of script, but call this again next cycle
	} else {                    // pause count is zerp
		L->looping = 0;
		return (IR_CONT); // continue script
	}
}

mcodeFunctionReturnCodes fn_missing_routine(int32 &, int32 *) {
	// shut down current object - wont be logic processed any int32er
	Message_box("fn_missing_routine shutting down [%s]", MS->Fetch_object_name(MS->Fetch_cur_id()));

	MS->Shut_down_object(" - fn_missing_routine");

	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_pass_flag_to_engine(int32 & /*result*/, int32 *params) {
	// script passed a value to the engine

	// NOTE this is an fn_function which may be used in either normal scripts or engine called socket style scrips - that is why it is
	// fn- not socket_

	Fatal_error("never use fn_pass_flag_to_engine");

	script_var_value = params[0];

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_object_rerun_logic_context(int32 &, int32 *params) {
	// reset the named object so it reruns its logic context

	// params    0   name of object

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	uint32 id = objects->Fetch_item_number_by_name(object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_object_rerun_logic_context cant find object [%s]", object_name);

	logic_structs[id]->context_request = TRUE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_restart_object(int32 &, int32 *) {
	// force an object to return to its logic context

	Zdebug("fn_restart_object");

	L->logic_level = 0; // force back down

	return (IR_TERMINATE); // script to go around
}

void _game_session::Reset_all_objects() {
	// force all game objects back to level 0 where they will rerun logic contexts
	// if they are held this will have no effect

	uint32 tot_obs = Fetch_number_of_objects();

	for (uint32 j = 0; j < tot_obs; j++) {
		logic_structs[j]->logic_level = 0;
		logic_structs[j]->logic_ref[1] = 0;
	}
}

mcodeFunctionReturnCodes _game_session::fn_new_script(int32 &, int32 *params) {
	// change to a new **local** script

	// params    0   name of new script
	uint32 k;
	char *ad;
	uint32 script_hash;
	const char *script_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	script_hash = HashString(script_name);

	// try and find a script with the passed extention i.e. ???::looping
	for (k = 0; k < object->GetNoScripts(); k++) {
		if (script_hash == object->GetScriptNamePartHash(k)) {
			// script k is the one to run
			// get the address of the script we want to run
			ad = (char *)scripts->Try_fetch_item_by_hash(object->GetScriptNameFullHash(k));

			// write actual offset
			L->logic[1] = ad;

			// write reference for change script checks later - i.e. FN_context_chosen_script
			L->logic_ref[1] = ad;

			L->logic_level = 1; //

			L->looping = 0; // reset to 0 for new logics

			if (L->mega)
				M->custom = FALSE8; // reset

			// script interpretter shouldnt write a pc back
			return (IR_TERMINATE);
		}
	}

	Fatal_error("fn_new_script - cant find script [%s] in object [%s]", script_name, object->GetName());
	return IR_CONT; // keep daft compiler happy
}

mcodeFunctionReturnCodes _game_session::fn_gosub(int32 &, int32 *params) {
	// gosub to a new **local** script

	// params    0   name of new script
	uint32 k;
	char *ad;
	uint32 script_hash;
	const char *script_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	script_hash = HashString(script_name);

	if (L->logic_level != 1)
		Fatal_error("object [%s] has performed an illegal gosub", object->GetName());

	// try and find a script with the passed extention i.e. ???::looping
	for (k = 0; k < object->GetNoScripts(); k++) {
		// now check for actual script name
		if (script_hash == object->GetScriptNamePartHash(k)) {
			// script k is the one to run
			// get the address of the script we want to run

			ad = (char *)scripts->Try_fetch_item_by_hash(object->GetScriptNameFullHash(k));

			// write actual offset
			L->logic[2] = ad;

			L->logic_level = 2; //

			L->looping = 0; // reset to 0 for new logics

			if (L->mega)
				M->custom = FALSE8; // reset

			L->old_looping = 0; // gets popped on dropoff

			// script interpretter shouldnt write a pc back
			return (IR_GOSUB);
		}
	}

	Fatal_error("fn_gosub - cant find script [%s] in object [%s]", script_name, object->GetName());
	return IR_CONT; // keep daft compiler happy
}

mcodeFunctionReturnCodes _game_session::fn_set_strike_overide(int32 &, int32 *params) {
	// set a mega to strike overide - i.e. run a script instead of hitting them

	// params    0   name of mega
	//			1  0 off 1 on
	const char *mega_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	uint32 tar = MS->objects->Fetch_item_number_by_name(mega_name);
	if (tar == 0xffffffff)
		Fatal_error("fn_set_strike_overide finds object [%s] does not exist", mega_name);

	if (logic_structs[tar]->image_type == PROP)
		Fatal_error("fn_set_strike_overide called on non mega");

	logic_structs[tar]->mega->use_strike_script = (uint8)params[1];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_shoot_overide(int32 &, int32 *params) {
	// set a mega to strike overide - i.e. run a script instead of hitting them

	// params    0   name of mega
	//			1  0 off 1 on

	const char *mega_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	uint32 tar = MS->objects->Fetch_item_number_by_name(mega_name);
	if (tar == 0xffffffff)
		Fatal_error("fn_set_shoot_overide finds object [%s] does not exist", mega_name);

	if (logic_structs[tar]->image_type == PROP)
		Fatal_error("fn_set_shoot_overide called on non mega");

	logic_structs[tar]->mega->use_fire_script = (uint8)params[1];

	return IR_CONT;
}

void _mega::InitDynamicLight(void) {
	dynLight.nStates = 1;       // one state
	dynLight.w = 0;             // zero width
	dynLight.b = 0;             // zero bounce
	dynLight.anu = 0;           // don't use it
	dynLight.type = OMNI_LIGHT; // OMNI
	dynLight.ba = 0;            // means nothing for an OMNI
	dynLight.bs = 0;            // means nothing for an OMNI

	dynLight.states[0].ans2 = 0; // dont think these things are used...
	dynLight.states[0].ane2 = (100 * 1) * (100 * 1);

	dynLight.states[0].m = 128; // no shade...

	// direction don't matter it's an OMNI light
	dynLight.states[0].vx = 4096; // ignored for an OMNI light
	dynLight.states[0].vy = 0;    // ignored for an OMNI light
	dynLight.states[0].vz = 0;    // ignored for an OMNI light    }
}

// fn_set_dynamic_light(cycles,r,g,b,x,y,z,falloff);
mcodeFunctionReturnCodes _game_session::fn_set_dynamic_light(int32 &, int32 *params) {
	M->SetDynamicLight(params[0],                       // cycles
	                   params[1], params[2], params[3], // rgb
	                   params[4], params[5], params[6], // xyz
	                   params[7]);                      // falloff

	return IR_CONT;
}

// speak_set_dynamic_light("object",cycles,r,g,b,x,y,z,falloff);
mcodeFunctionReturnCodes _game_session::speak_set_dynamic_light(int32 &, int32 *params) {
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	int obj_id = objects->Fetch_item_number_by_name(object_name);

	logic_structs[obj_id]->mega->SetDynamicLight(params[1],                       // cycles
	                                             params[2], params[3], params[4], // rgb
	                                             params[5], params[6], params[7], // xyz
	                                             params[8]);                      // falloff

	return IR_CONT;
}

// SetDynamicLight(cycles,r,g,b,x,y,z);
// where cycles is number of cycles to stay on (-1 for constant, 0 for off)
void _mega::SetDynamicLight(int32 in_cycles, int32 in_r, int32 in_g, int32 in_b, int32 in_x, int32 in_y, int32 in_z, int32 falloff) {
	// set cycles (on)
	if (in_cycles != 0)
		dynLightOn = in_cycles + 1; // +1 cycles for first update (before drawing...)
	else
		dynLightOn = 0; // 0 cycles

	// check colours are 0-255
	if ((in_r > 255) || (in_r < 0) || (in_g > 255) || (in_g < 0) || (in_b > 255) || (in_b < 0))
		Fatal_error("Dynamic light rgb %d,%d,%d out of range (0-255)", in_r, in_g, in_b);

	// set colours (scale 0-255 to 0-4095)
	dynLight.states[0].c.r = (short)((in_r * 4096) / 256);
	dynLight.states[0].c.g = (short)((in_g * 4096) / 256);
	dynLight.states[0].c.b = (short)((in_b * 4096) / 256);

	// set the v field of colour to be the maximum of r,g,b

	dynLight.states[0].c.v = dynLight.states[0].c.r;         // start at red
	if (dynLight.states[0].c.g > dynLight.states[0].c.v)     // if green bigger
		dynLight.states[0].c.v = dynLight.states[0].c.g; // set to green
	if (dynLight.states[0].c.b > dynLight.states[0].c.v)     // if blue bigger
		dynLight.states[0].c.v = dynLight.states[0].c.b; // set to blue

	// setup positions
	dynLightX = (int16)in_x;
	dynLightY = (int16)in_y;
	dynLightZ = (int16)in_z;

	// falloff
	if (falloff == 0) {
		dynLight.afu = 0; // don't use it
	} else {
		dynLight.states[0].afs2 = (falloff * falloff) / 100; // (d/10)^2     = (d*d)/100
		dynLight.states[0].afe2 = falloff * falloff;         // d^2          = (d*d)

		dynLight.afu = 1; // use it
	}
}

void AddDynamicLight(PSXLampList &lamplist, _logic *log) {
	_mega *mega = log->mega;

	if (mega->dynLightOn == 0)
		return;

	int xx, yy, zz;

	xx = mega->dynLightX;
	yy = mega->dynLightY;
	zz = mega->dynLightZ;

	// rotate around character...

	PXfloat ss, cc;
	PXfloat angle = -log->pan * TWO_PI;

	ss = (PXfloat)PXsin(angle);
	cc = (PXfloat)PXcos(angle);

// rotate xx and zz around act.trueRot.vy       ONE
	mega->dynLight.states[0].pos.vx = (int32)(xx * cc - zz * ss);
	mega->dynLight.states[0].pos.vz = (int32)(xx * ss + zz * cc);
	mega->dynLight.states[0].pos.vy = (int32)yy; // no rotation

	// and add the players position

	mega->dynLight.states[0].pos.vx += (int32)mega->actor_xyz.x;
	mega->dynLight.states[0].pos.vy += (int32)mega->actor_xyz.y;
	mega->dynLight.states[0].pos.vz += (int32)mega->actor_xyz.z;

	// for each lamp to add
	lamplist.lamps[lamplist.n] = &(mega->dynLight);
	lamplist.states[lamplist.n] = 0;
	lamplist.n++;
}

void _game_session::UpdateMegaFX() {
	// first do things which are done for all megas
	// next do things that are only done for visable ones...

	// do the check
	if (!Object_visible_to_camera(cur_id))
		return;

	// now do on screen only things

	// dynamic light
	// if >0 then reduce it (only stay on for certain number of cycles... (THIS NEDS MOVING TO SOME MEGA UPDATE BIT)
	if (M->dynLightOn > 0)
		M->dynLightOn--;

	// breathing
	M->breath.Update();

	// bullet
	UpdateCartridgeCase();

	// if talking then update talking
	if ((cur_id == (uint)speech_info[CONV_ID].current_talker) && // we are the one talking
	    (speech_info[CONV_ID].total_subscribers > 1) &&          // not talking to myself
	    (speech_info[CONV_ID].state == __SAYING)                 // are definately saying, not just getting ready to...
	    ) {
		// get rap
		rap_API *pose = (rap_API *)rs_anims->Res_open(I->get_pose_name(), I->pose_hash, I->base_path, I->base_path_hash);

		// use it
		UpdateTalking(L, pose); // update jaw and neck bone
	} else {
		I->neckBone.Target0(); // update towards <0,0,0>
		I->jawBone.Target0();  // same
	}

	I->jawBone.Update();
	I->neckBone.Update();
	I->lookBone.Update();

	// next do ones that are only for player

	// return here...
	if (cur_id != player.Fetch_player_id())
		return;

	// now do player only things

	// shot deformation
	player.shotDeformation.Update();

	// if counter is full then jerk body back
	if (player.being_shot == 3)
		SetPlayerShotBone(player.shot_by_id); // set to maximum push now...

	// reduce counter (when it's zero we can shoot again)
	if (player.being_shot)
		player.being_shot--;

	UpdatePlayerLook();
}

} // End of namespace ICB
