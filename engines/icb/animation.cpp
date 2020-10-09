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
#include "engines/icb/res_man.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_prop_anims.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/session.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/global_objects.h"

namespace ICB {

mcodeFunctionReturnCodes fn_prop_animate(int32 &result, int32 *params) { return (MS->fn_prop_animate(result, params)); }

mcodeFunctionReturnCodes fn_prop_set_to_last_frame(int32 &result, int32 *params) { return (MS->fn_prop_set_to_last_frame(result, params)); }

mcodeFunctionReturnCodes fn_prop_set_to_first_frame(int32 &result, int32 *params) { return (MS->fn_prop_set_to_first_frame(result, params)); }

mcodeFunctionReturnCodes fn_test_prop_anim(int32 &result, int32 *params) { return (MS->fn_test_prop_anim(result, params)); }

mcodeFunctionReturnCodes fn_inherit_prop_anim_height(int32 &result, int32 *params) { return (MS->fn_inherit_prop_anim_height(result, params)); }

mcodeFunctionReturnCodes fn_inherit_prop_anim_height_id(int32 &result, int32 *params) { return (MS->fn_inherit_prop_anim_height_id(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_prop_animate(int32 &, int32 *params) {
	// runs forward through a prop animation until it finishes
	// bungs a frame in first time called - lets last frame have a whole cycle then quits next go
	// note - this is for props only not actors

	// in
	//	param    0       anim name

	// out
	//			IR_CONT        anim finished
	//			IR_REPEAT  anim not finished yet - more frames remain to be played

	_animating_prop *index;
	_animation_entry *anim;
	uint32 j;

	// find entry for this object via its name
	index = (_animating_prop *)prop_anims->Try_fetch_item_by_name(object->GetName());

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// loop through all looking for our named anim
	for (j = 0; j < index->num_anims; j++) {
		// get each anim for this prop in turn until we match name
		anim = (_animation_entry *)(((char *)index) + index->anims[j]);

		if (!strcmp(((char *)((char *)index) + anim->name), anim_name)) {
			// found the anim

			// is this the first time in?
			if (!L->looping) {
				// first time so set up the anim

				// set first frame
				prop_state_table[cur_id] = anim->frames[0];

				// current pc
				L->anim_pc = 0;

				// now looping
				L->looping = 1;

				// come back again next cycle
				return (IR_REPEAT);
			} else {
				// just running through the animation
				// is it the end? - i.e. was the frame set up last cycle the last in the sequence - if so then continue with
				// the script

				if ((uint8)L->anim_pc == (anim->num_frames - 1)) {
					// not looping
					logic_structs[cur_id]->looping = 0;

					// last frame has been displayed so now continue with script
					return (IR_CONT);
				}

				// advance current pc
				++L->anim_pc;

				// set next frame
				prop_state_table[cur_id] = anim->frames[L->anim_pc];

				// come back again next cycle
				return (IR_REPEAT);
			}
		}
	}

	// didnt find the named anim so thats it
	Tdebug("objects_that_died.txt", "_game_session::fn_prop_animate object %s cant find anim %s", object->GetName(), anim_name);
	Shut_down_object("by fn_prop_animate");
	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_prop_set_to_last_frame(int32 &, int32 *params) {
	_animating_prop *index;
	_animation_entry *anim;
	uint32 j;

	// find entry for this object via its name
	index = (_animating_prop *)prop_anims->Try_fetch_item_by_name(object->GetName());

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// loop through all looking for our named anim
	for (j = 0; j < index->num_anims; j++) {
		// get each anim for this prop in turn until we match name
		anim = (_animation_entry *)(((char *)index) + index->anims[j]);

		if (!strcmp(((char *)((char *)index) + anim->name), anim_name)) {
			// found the anim
			// set to last frame in sequence
			prop_state_table[cur_id] = anim->frames[anim->num_frames - 1];

			return (IR_CONT);
		}
	}

	// didnt find the named anim so thats it
	Tdebug("objects_that_died.txt", "fn_prop_set_to_last_frame object %s cant find anim %s", object->GetName(), anim_name);
	Shut_down_object("by fn_prop_set_to_last_frame");

	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_prop_set_to_first_frame(int32 &, int32 *params) {
	_animating_prop *index;
	_animation_entry *anim;
	uint32 j;

	// find entry for this object via its name
	index = (_animating_prop *)prop_anims->Try_fetch_item_by_name(object->GetName());

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// loop through all looking for our named anim
	for (j = 0; j < index->num_anims; j++) {
		// get each anim for this prop in turn until we match name
		anim = (_animation_entry *)(((char *)index) + index->anims[j]);
		if (!strcmp(((char *)((char *)index) + anim->name), anim_name)) {
			// found the anim
			// set to last frame in sequence
			prop_state_table[cur_id] = anim->frames[0];

			return (IR_CONT);
		}
	}

	// didnt find the named anim so thats it
	Tdebug("objects_that_died.txt", "fn_prop_set_to_first_frame object %s cant find anim %s", object->GetName(), anim_name);
	Message_box("fn_prop_set_to_first_frame object %s cant find anim %s", object->GetName(), anim_name);
	Shut_down_object("by fn_prop_set_to_first_frame");
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_test_prop_anim(int32 &result, int32 *params) {
	// confirm that a prop anim actually exists
	// this is useful for objects that may be using custom engine logic

	_animating_prop *index;
	_animation_entry *anim;
	uint32 j;

	// find entry for this object via its name
	index = (_animating_prop *)prop_anims->Try_fetch_item_by_name(object->GetName());

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// loop through all looking for our named anim
	for (j = 0; j < index->num_anims; j++) {
		// get each anim for this prop in turn until we match name
		anim = (_animation_entry *)(((char *)index) + index->anims[j]);

		if (!strcmp(((char *)((char *)index) + anim->name), anim_name)) {
			// final check to see if there are actual frames in there
			if (anim->num_frames) {
				result = 1;
				return (IR_CONT);
			}

			Message_box("fn_test_prop_anim object [%s] has anim [%s] but it has no frames", object->GetName(), anim_name);
			result = 0;
			return IR_CONT;
		}
	}

	Message_box("fn_test_prop_anim didnt find anim [%s] for object [%s]", anim_name, object->GetName());

	result = 0;

	return IR_CONT;
}

uint32 _game_session::Validate_prop_anim(const char *anim_name) {
	// confirm that a prop anim actually exists
	// this is useful for objects that may be using custom engine logic
	// return the anim number

	_animating_prop *index;
	_animation_entry *anim;
	uint32 j;

	// find entry for this object via its name
	index = (_animating_prop *)prop_anims->Try_fetch_item_by_name(object->GetName());

	// loop through all looking for our named anim
	for (j = 0; j < index->num_anims; j++) {
		// get each anim for this prop in turn until we match name
		anim = (_animation_entry *)(((char *)index) + index->anims[j]);

		if (!strcmp(((char *)((char *)index) + anim->name), anim_name)) {
			// final check to see if there are actual frames in there
			if (anim->num_frames)
				return (j);

			Fatal_error("Validate_prop_anim object [%s] has anim [%s] but it has no frames", object->GetName(), anim_name);
		}
	}

	Fatal_error("Validate_prop_anim didnt find anim [%s] for object [%s]", anim_name, object->GetName());
	return (FALSE8);
}

mcodeFunctionReturnCodes _game_session::fn_inherit_prop_anim_height_id(int32 &, int32 *params) {
	// runs forward through a props animation until it finishes passing the height to the mega
	// so instead of playing the anim we're just gettings the y height each frame instead
	// the prop should therefore be playing the anim at the same time

	//	param    0       prop id
	//			1      anim name

	// out
	//			IR_CONT        anim finished
	//			IR_REPEAT  anim not finished yet - more frames remain to be played

	_animating_prop *index;
	_animation_entry *anim;
	uint32 j;

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Zdebug("fn_inherit_prop_anim_height_id");

	// find entry for this object via its name, which we find via its number :(
	index = (_animating_prop *)prop_anims->Fetch_item_by_name((const char *)objects->Fetch_items_name_by_number(params[0]));

	// loop through all looking for our named anim
	for (j = 0; j < index->num_anims; j++) {
		// get each anim for this prop in turn until we match name
		anim = (_animation_entry *)(((char *)index) + index->anims[j]);

		if (!strcmp(((char *)((char *)index) + anim->name), anim_name)) {
			// found the anim
			Zdebug(" found anim");

			// is this the first time in?
			if (!L->looping) {
				// first time so set up the anim
				if (!anim->offset_heights)
					return IR_CONT;

				// current pc
				L->list[0] = 0; // our prop pc!

				// now looping
				L->looping = 1;

				// protect the mega
				L->do_not_disturb = 1;

				// get first height
				int16 *heights = (int16 *)(((char *)index) + anim->offset_heights);
				PXreal one_height = (PXreal)(heights[0]);
				M->actor_xyz.y = one_height;

				// come back again next cycle
				return (IR_REPEAT);
			} else {
				// just running through the animation
				// is it the end? - i.e. was the frame set up last cycle the last in the sequence - if so then continue with
				// the script

				if (L->list[0] > (uint8)(anim->num_frames - 1))
					Fatal_error("%s corrupted in fn_inherit_prop_anim_height_id", object->GetName());

				if ((uint8)L->list[0] == (anim->num_frames - 1)) {
					// not looping
					logic_structs[cur_id]->looping = 0;

					// unprotect the mega
					L->do_not_disturb = 0;

					// last frame has been displayed so now continue with script
					return (IR_CONT);
				}

				// advance current pc
				L->list[0]++;

				Zdebug("pc = %d", L->list[0]);

				// get height
				int16 *heights = (int16 *)(((char *)index) + anim->offset_heights);
				PXreal one_height = (PXreal)(heights[L->list[0]]);
				M->actor_xyz.y = one_height;
				Zdebug("new height %3.1f", M->actor_xyz.y);
				// come back again next cycle
				return (IR_REPEAT);
			}
		}
	}

	// didnt find the named anim so thats it
	Fatal_error("fn_inherit_prop_anim_height_id object [%s] prop [%s] cant find anim [%s]", object->GetName(), objects->Fetch_items_name_by_number(params[0]), anim_name);
	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_inherit_prop_anim_height(int32 &, int32 *params) {
	// runs forward through a props animation until it finishes passing the height to the mega
	// so instead of playing the anim we're just gettings the y height each frame instead
	// the prop should therefore be playing the anim at the same time

	//	param    0       prop name
	//			1      anim name

	// out
	//			IR_CONT        anim finished
	//			IR_REPEAT  anim not finished yet - more frames remain to be played

	_animating_prop *index;
	_animation_entry *anim;
	uint32 j;

	const char *prop_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Zdebug("fn_inherit_prop_anim_height");
	Zdebug("ob %s", object->GetName());
	Zdebug("prop %s  anim %s", prop_name, anim_name);

	// find entry for this object via its name, which we find via its number :(
	index = (_animating_prop *)prop_anims->Fetch_item_by_name(prop_name);

	// loop through all looking for our named anim
	for (j = 0; j < index->num_anims; j++) {
		// get each anim for this prop in turn until we match name
		anim = (_animation_entry *)(((char *)index) + index->anims[j]);

		if (!strcmp(((char *)((char *)index) + anim->name), anim_name)) {
			// found the anim
			Zdebug(" found anim");

			// is this the first time in?
			if (!L->looping) {
				// first time so set up the anim

				// current pc
				L->list[0] = 0; // our prop pc!

				// now looping
				L->looping = 1;

				// protect the mega
				L->do_not_disturb = 1;

				// get first height
				int16 *heights = (int16 *)(((char *)index) + anim->offset_heights);
				PXreal one_height = (PXreal)(heights[0]);
				M->actor_xyz.y = one_height;

				// come back again next cycle
				return (IR_REPEAT);
			} else {
				// just running through the animation
				// is it the end? - i.e. was the frame set up last cycle the last in the sequence - if so then continue with
				// the script

				if ((uint8)L->list[0] == (anim->num_frames - 1)) {
					// not looping
					logic_structs[cur_id]->looping = 0;

					// unprotect the mega
					L->do_not_disturb = 0;

					// last frame has been displayed so now continue with script
					return (IR_CONT);
				}

				// advance current pc
				L->list[0]++;

				// get height
				int16 *heights = (int16 *)(((char *)index) + anim->offset_heights);
				PXreal one_height = (PXreal)(heights[L->list[0]]);

				M->actor_xyz.y = one_height;

				// come back again next cycle
				return (IR_REPEAT);
			}
		}
	}

	// didnt find the named anim so thats it
	Fatal_error("fn_inherit_prop_anim_height object [%s] prop [%s] cant find anim [%s]", object->GetName(), prop_name, anim_name);
	return (IR_STOP); //
}

} // End of namespace ICB
