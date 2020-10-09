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

#include "engines/icb/p4.h" //for machine version
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/mission.h"
#include "engines/icb/session.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/debug.h"
#include "engines/icb/player.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/barriers.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/animation_mega_set.h"
#include "engines/icb/mission.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/ptr_util.h"

namespace ICB {

mcodeFunctionReturnCodes fn_mega_interacts(int32 &result, int32 *params) { return (MS->fn_mega_interacts(result, params)); }

mcodeFunctionReturnCodes fn_mega_generic_interact(int32 &result, int32 *params) { return (MS->fn_mega_generic_interact(result, params)); }

mcodeFunctionReturnCodes fn_interact_with_id(int32 &result, int32 *params) { return (MS->fn_interact_with_id(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_interact_with_id(int32 &, int32 *params) {
	// mega character interacts with an object via its id

	// do check to see if script running

	// if not set it up on level 2 and change script level

	// params        0   id of target object
	//				1  name of script
	const char *script_name = (const char *)MemoryUtil::resolvePtr(params[1]);
	c_game_object *target_object;
	uint32 script_hash;

	Zdebug("fn_interact_with_id with object");
	Zdebug("fn_interact_with_id with object [%d], script [%s]", params[0], script_name);

	script_hash = HashString(script_name);

	// get target object
	target_object = (c_game_object *)MS->objects->Fetch_item_by_number(params[0]);

	// set socket_id ready for any special socket functions
	M->target_id = params[0];

	// set this flag to avoid interact with id=0 based problems
	M->interacting = TRUE8;

	// now try and find a script with the passed extention i.e. ???::looping
	for (uint32 k = 0; k < target_object->GetNoScripts(); k++) {
		if (script_hash == target_object->GetScriptNamePartHash(k)) {
			Zdebug("found target interact script", k);
			// script k is the one to run
			// get the address of the script we want to run

			char *ad = (char *)scripts->Try_fetch_item_by_hash(target_object->GetScriptNameFullHash(k));

			// write actual offset
			L->logic[2] = ad;

			// write reference for change script checks later - i.e. FN_context_chosen_script
			L->logic_ref[2] = ad;

			L->logic_level = 2; // reset to level 2
			// action script will fall back to looping level 1

			// script interpretter shouldnt write a pc back
			return (IR_GOSUB);
		}
	}

	Fatal_error("fn_interact_with_id - target object [%d] has not got a [%s] script", params[0], object->GetName());

	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_mega_interacts(int32 &, int32 *params) {
	// mega character interacts with an object

	// do check to see if script running

	// if not set it up on level 2 and change script level

	// params        0   name of target object
	//				1  name of script

	c_game_object *target_object;
	uint32 script_hash;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *script_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	script_hash = HashString(script_name);

	Zdebug("fn_mega_interacts with object [%s], script [%s]", object_name, script_name);

	// get target object
	target_object = (c_game_object *)MS->objects->Try_fetch_item_by_name(object_name);
	if (!target_object)
		Fatal_error("fn_mega_interacts - named object [%s] dont exist", object_name);

	// set socket_id ready for any special socket functions
	M->target_id = objects->Fetch_item_number_by_name(object_name);

	// set this flag to avoid interact with id=0 based problems
	M->interacting = TRUE8;

	// now try and find a script with the passed extention i.e. ???::looping
	for (uint32 k = 0; k < target_object->GetNoScripts(); k++) {
		if (script_hash == target_object->GetScriptNamePartHash(k)) {
			Zdebug("found target interact script", k);
			// script k is the one to run
			// get the address of the script we want to run
			char *ad = (char *)scripts->Try_fetch_item_by_hash(target_object->GetScriptNameFullHash(k));

			// write actual offset
			L->logic[2] = ad;

			// write reference for change script checks later - i.e. FN_context_chosen_script
			L->logic_ref[2] = ad;

			L->logic_level = 2; // reset to level 2
			// action script will fall back to looping level 1

			// script interpretter shouldnt write a pc back
			return (IR_GOSUB);
		}
	}

	Fatal_error("fn_mega_interacts - target object [%s] has not got a [%s] script", object_name, object->GetName());

	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_mega_generic_interact(int32 &, int32 *params) {
	// mega character interacts with an object using the INTERACT script that the player also uses

	// params    0   ascii name of object

	c_game_object *target_object;
	uint32 script_hash;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	script_hash = HashString("interact");

	Zdebug("fn_mega_generic_interact with [%s]", object_name);

	// get target object
	target_object = (c_game_object *)MS->objects->Try_fetch_item_by_name(object_name);
	if (!target_object)
		Fatal_error("fn_mega_generic_interact - named object [%s] dont exist", object_name);

	// set socket_id ready for any special socket functions
	M->target_id = objects->Fetch_item_number_by_name(object_name);

	// set this flag to avoid interact with id=0 based problems
	M->interacting = TRUE8;

	// now try and find a script with the 'interact' extention i.e. ???::interact
	for (uint32 k = 0; k < target_object->GetNoScripts(); k++) {
		if (script_hash == target_object->GetScriptNamePartHash(k)) {
			Zdebug("found target interact script", k);
			// script k is the one to run
			// get the address of the script we want to run

			char *ad = (char *)scripts->Try_fetch_item_by_hash(target_object->GetScriptNameFullHash(k));

			// write actual offset
			L->logic[2] = ad;

			// write reference for change script checks later - i.e. FN_context_chosen_script
			L->logic_ref[2] = ad;

			L->logic_level = 2; // reset to level 2
			// action script will fall back to looping level 1

			// script interpretter shouldnt write a pc back
			return (IR_GOSUB);
		}
	}

	Fatal_error("fn_mega_generic_interact - [%s] finds target object [%s] has not got an 'interact' script", object->GetName(), object_name);

	return (IR_STOP);
}

bool8 _game_session::chi_interacts(int32 id, const char *script_name) {
	// chi interacts with a follow object

	// set it up on level 2 and change script level

	c_game_object *target_object;
	uint32 script_hash;

	script_hash = HashString(script_name);

	// get target object
	target_object = (c_game_object *)MS->objects->Fetch_item_by_number(id);
	if (!target_object)
		Fatal_error("chi_interacts - object [%d] dont exist", id);

	// set socket_id ready for any special socket functions
	M->target_id = id;

	// set this flag to avoid interact with id=0 based problems
	M->interacting = TRUE8;

	// now try and find a script with the passed extention i.e. ???::looping
	for (uint32 k = 0; k < target_object->GetNoScripts(); k++) {
		if (script_hash == target_object->GetScriptNamePartHash(k)) {
			// script k is the one to run
			// get the address of the script we want to run
			char *ad = (char *)scripts->Try_fetch_item_by_hash(target_object->GetScriptNameFullHash(k));

			// write actual offset
			L->logic[2] = ad;

			// write reference for change script checks later - i.e. FN_context_chosen_script
			L->logic_ref[2] = ad;

			L->logic_level = 2; // reset to level 2
			// action script will fall back to looping level 1

			// script interpretter shouldnt write a pc back
			return TRUE8;
		}
	}

	// couldnt find chi script

	return FALSE8;
}

} // End of namespace ICB
