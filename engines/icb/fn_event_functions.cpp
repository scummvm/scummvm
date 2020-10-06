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

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/event_manager.h"
#include "engines/icb/line_of_sight.h"
#include "engines/icb/sound_logic.h"
#include "engines/icb/mission.h"

#define EVENT_FNS_LOG "event_functions_log.txt"

namespace ICB {

// These are to do with the normal events (i.e. not line-of-sight or sound events).

mcodeFunctionReturnCodes fn_register_for_event(int32 &result, int32 *params) { return (MS->fn_register_for_event(result, params)); }

mcodeFunctionReturnCodes fn_unregister_for_event(int32 &result, int32 *params) { return (MS->fn_unregister_for_event(result, params)); }

mcodeFunctionReturnCodes fn_register_object_for_event(int32 &result, int32 *params) { return (MS->fn_register_object_for_event(result, params)); }

mcodeFunctionReturnCodes fn_unregister_object_for_event(int32 &result, int32 *params) { return (MS->fn_unregister_object_for_event(result, params)); }

mcodeFunctionReturnCodes fn_post_event(int32 &result, int32 *params) { return (MS->fn_post_event(result, params)); }

mcodeFunctionReturnCodes fn_post_future_event(int32 &result, int32 *params) { return (MS->fn_post_future_event(result, params)); }

mcodeFunctionReturnCodes fn_post_repeating_event(int32 &result, int32 *params) { return (MS->fn_post_repeating_event(result, params)); }

mcodeFunctionReturnCodes fn_clear_all_events(int32 &result, int32 *params) { return (MS->fn_clear_all_events(result, params)); }

mcodeFunctionReturnCodes fn_check_event_waiting(int32 &result, int32 *params) { return (MS->fn_check_event_waiting(result, params)); }

mcodeFunctionReturnCodes fn_event_check_last_sender(int32 &result, int32 *params) { return (MS->fn_event_check_last_sender(result, params)); }

mcodeFunctionReturnCodes fn_get_last_event_sender_id(int32 &result, int32 *params) { return (MS->fn_get_last_event_sender_id(result, params)); }

mcodeFunctionReturnCodes fn_post_named_event_to_object(int32 &result, int32 *params) { return (MS->fn_post_named_event_to_object(result, params)); }

mcodeFunctionReturnCodes fn_post_named_event_to_object_id(int32 &result, int32 *params) { return (MS->fn_post_named_event_to_object_id(result, params)); }

mcodeFunctionReturnCodes fn_shut_down_event_processing(int32 &result, int32 *params) { return (MS->fn_shut_down_event_processing(result, params)); }

mcodeFunctionReturnCodes fn_is_registered_for_event(int32 &result, int32 *params) { return (MS->fn_is_registered_for_event(result, params)); }

mcodeFunctionReturnCodes fn_is_object_registered_for_event(int32 &result, int32 *params) { return (MS->fn_is_object_registered_for_event(result, params)); }

mcodeFunctionReturnCodes fn_suspend_events(int32 &result, int32 *params) { return (MS->fn_suspend_events(result, params)); }

mcodeFunctionReturnCodes fn_unsuspend_events(int32 &result, int32 *params) { return (MS->fn_unsuspend_events(result, params)); }

// These are to do with line-of-sight.
mcodeFunctionReturnCodes fn_register_for_line_of_sight(int32 &result, int32 *params) { return (MS->fn_register_for_line_of_sight(result, params)); }

mcodeFunctionReturnCodes fn_unregister_for_line_of_sight(int32 &result, int32 *params) { return (MS->fn_unregister_for_line_of_sight(result, params)); }

mcodeFunctionReturnCodes fn_register_object_for_line_of_sight(int32 &result, int32 *params) { return (MS->fn_register_object_for_line_of_sight(result, params)); }

mcodeFunctionReturnCodes fn_unregister_object_for_line_of_sight(int32 &result, int32 *params) { return (MS->fn_unregister_object_for_line_of_sight(result, params)); }

mcodeFunctionReturnCodes fn_can_see(int32 &result, int32 *params) { return (MS->fn_can_see(result, params)); }

mcodeFunctionReturnCodes fn_can_object_see(int32 &result, int32 *params) { return (MS->fn_can_object_see(result, params)); }

mcodeFunctionReturnCodes fn_set_mega_field_of_view(int32 &result, int32 *params) { return (MS->fn_set_mega_field_of_view(result, params)); }

mcodeFunctionReturnCodes fn_set_object_sight_range(int32 &result, int32 *params) { return (MS->fn_set_object_sight_range(result, params)); }

mcodeFunctionReturnCodes fn_set_object_sight_height(int32 &result, int32 *params) { return (MS->fn_set_object_sight_height(result, params)); }

mcodeFunctionReturnCodes fn_line_of_sight_suspend(int32 &result, int32 *params) { return (MS->fn_line_of_sight_suspend(result, params)); }

mcodeFunctionReturnCodes fn_line_of_sight_unsuspend(int32 &result, int32 *params) { return (MS->fn_line_of_sight_unsuspend(result, params)); }

mcodeFunctionReturnCodes fn_line_of_sight_now(int32 &result, int32 *params) { return (MS->fn_line_of_sight_now(result, params)); }

mcodeFunctionReturnCodes fn_can_see_in_dark(int32 &result, int32 *params) { return (MS->fn_can_see_in_dark(result, params)); }

mcodeFunctionReturnCodes fn_mega_never_in_shadow(int32 &result, int32 *params) { return (MS->fn_mega_never_in_shadow(result, params)); }

// These are to do with the logic sound engine.
mcodeFunctionReturnCodes fn_sound_set_hearing_sensitivity(int32 &result, int32 *params) { return (MS->fn_sound_set_hearing_sensitivity(result, params)); }

mcodeFunctionReturnCodes fn_sound_heard_this(int32 &result, int32 *params) { return (MS->fn_sound_heard_this(result, params)); }

mcodeFunctionReturnCodes fn_sound_heard_something(int32 &result, int32 *params) { return (MS->fn_sound_heard_something(result, params)); }

mcodeFunctionReturnCodes fn_sound_new_entry(int32 &result, int32 *params) { return (MS->fn_sound_new_entry(result, params)); }

mcodeFunctionReturnCodes fn_sound_remove_entry(int32 &result, int32 *params) { return (MS->fn_sound_remove_entry(result, params)); }

mcodeFunctionReturnCodes fn_sound_simulate(int32 &result, int32 *params) { return (MS->fn_sound_simulate(result, params)); }

mcodeFunctionReturnCodes fn_sound_get_x(int32 &result, int32 *params) { return (MS->fn_sound_get_x(result, params)); }

mcodeFunctionReturnCodes fn_sound_get_z(int32 &result, int32 *params) { return (MS->fn_sound_get_z(result, params)); }

mcodeFunctionReturnCodes fn_sound_fast_face(int32 &result, int32 *params) { return (MS->fn_sound_fast_face(result, params)); }

mcodeFunctionReturnCodes fn_sound_route_to_near(int32 &result, int32 *params) { return (MS->fn_sound_route_to_near(result, params)); }

mcodeFunctionReturnCodes fn_sound_suspend(int32 &result, int32 *params) { return (MS->fn_sound_suspend(result, params)); }

mcodeFunctionReturnCodes fn_sound_unsuspend(int32 &result, int32 *params) { return (MS->fn_sound_unsuspend(result, params)); }

mcodeFunctionReturnCodes fn_sound_link_floors(int32 &result, int32 *params) { return (MS->fn_sound_link_floors(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_register_for_event(int32 &, int32 *params) {
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work in the event manager.
	g_oEventManager->RegisterForEvent(cur_id, event_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_unregister_for_event(int32 &, int32 *params) {
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work in the event manager.
	g_oEventManager->UnregisterForEvent(cur_id, event_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_register_object_for_event(int32 &, int32 *params) {
	uint32 nObjectID;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Find the target object's ID.
	nObjectID = objects->Fetch_item_number_by_name(object_name);

	// Call the function that does the work in the event manager.
	g_oEventManager->RegisterForEvent(nObjectID, event_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_unregister_object_for_event(int32 &, int32 *params) {
	uint32 nObjectID;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Find the target object's ID.
	nObjectID = objects->Fetch_item_number_by_name(object_name);

	// Call the function that does the work in the event manager.
	g_oEventManager->UnregisterForEvent(nObjectID, event_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_post_event(int32 &, int32 *params) {
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work in the event manager.
	g_oEventManager->PostNamedEvent(event_name, cur_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_post_future_event(int32 &, int32 *params) {
	uint32 nTime;

	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	nTime = (uint32)params[1];
	g_oEventManager->PostRepeatingEvent(event_name, nTime, 0, nTime);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_post_repeating_event(int32 &, int32 *params) {
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work in the event manager.
	g_oEventManager->PostRepeatingEvent(event_name, (uint32)params[1], (uint32)params[2], (uint32)params[3]);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_clear_all_events(int32 &, int32 *) {
	// Call the function that does the work in the event manager.
	g_oEventManager->ClearAllEventsForObject(cur_id);

	// Call the function that does the work in the event manager.
	g_oSoundLogicEngine->ClearHeardFlag(cur_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_check_event_waiting(int32 &result, int32 *params) {
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work in the event manager.
	result = g_oEventManager->CheckEventWaitingForObject(cur_id, event_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_event_check_last_sender(int32 &result, int32 *params) {
	uint32 nObjectID;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Find the target object's ID.
	nObjectID = objects->Fetch_item_number_by_name(object_name);

	// Find the sender of the named event.
	result = g_oEventManager->DidObjectSendLastNamedEvent(cur_id, nObjectID, event_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_get_last_event_sender_id(int32 &result, int32 *params) {
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Find the sender of the named event.
	result = g_oEventManager->GetIDOfLastObjectToPostEvent(cur_id, event_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_post_named_event_to_object(int32 &, int32 *params) {
	uint32 nTargetID;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Get ID of target and make sure it is valid.
	nTargetID = objects->Fetch_item_number_by_name(object_name);

	// Post the event.
	g_oEventManager->PostNamedEventToObject(event_name, nTargetID, cur_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_post_named_event_to_object_id(int32 &, int32 *params) {
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Post the event.
	g_oEventManager->PostNamedEventToObject(event_name, (uint32)(params[0]), cur_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_shut_down_event_processing(int32 &, int32 *) {
	// Shut down event processing for the object.
	g_oEventManager->ShutDownEventProcessingForObject(cur_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_is_registered_for_event(int32 &result, int32 *params) {
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Make the engine call.
	result = g_oEventManager->IsObjectRegisteredForEvent(cur_id, event_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_is_object_registered_for_event(int32 &result, int32 *params) {
	uint32 nObjectID;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *event_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Get ID of object and make sure it is valid.
	nObjectID = objects->Fetch_item_number_by_name(object_name);

	// Make the engine call.
	result = g_oEventManager->IsObjectRegisteredForEvent(nObjectID, event_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_suspend_events(int32 &, int32 *) {
	// Make the call to the event manager.
	g_oEventManager->SetSuspendFlagForObject(cur_id, TRUE8);

	// Must turn off sound events separately.
	g_oSoundLogicEngine->SetSuspendedFlag(cur_id, TRUE8);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_unsuspend_events(int32 &, int32 *) {
	// Make the call to the event manager.
	g_oEventManager->SetSuspendFlagForObject(cur_id, FALSE8);

	// Must turn off sound events separately.
	g_oSoundLogicEngine->SetSuspendedFlag(cur_id, FALSE8);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_register_for_line_of_sight(int32 &, int32 *params) {
	uint32 nObjectID;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Find the target object's ID.
	nObjectID = objects->Fetch_item_number_by_name(object_name);

	// Now we can make the actual call to the line-of-sight object.
	PXTRY

	g_oLineOfSight->Subscribe(cur_id, nObjectID);

	PXCATCH

	Fatal_error("Exception in _line_of_sight::Subscribe()");

	PXENDCATCH

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_unregister_for_line_of_sight(int32 &, int32 *params) {
	uint32 nObjectID;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Find the target object's ID.
	nObjectID = objects->Fetch_item_number_by_name(object_name);

	// Now we can make the actual call to the line-of-sight object.
	PXTRY

	g_oLineOfSight->UnSubscribe(cur_id, nObjectID);

	PXCATCH

	Fatal_error("Exception in _line_of_sight::UnSubscribe()");

	PXENDCATCH

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_register_object_for_line_of_sight(int32 &, int32 *params) {
	uint32 nObserverID;
	uint32 nTargetID;

	const char *observer_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *target_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Find the objects' IDs.
	nObserverID = objects->Fetch_item_number_by_name(observer_name);

	nTargetID = objects->Fetch_item_number_by_name(target_name);

	// Now we can make the actual call to the line-of-sight object.
	if ((nTargetID != PX_LINKED_DATA_FILE_ERROR) && (nObserverID != PX_LINKED_DATA_FILE_ERROR)) {
		PXTRY

		g_oLineOfSight->Subscribe(nObserverID, nTargetID);

		PXCATCH

		Fatal_error("Exception in _line_of_sight::Subscribe()");

		PXENDCATCH
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_unregister_object_for_line_of_sight(int32 &, int32 *params) {
	uint32 nObserverID;
	uint32 nTargetID;

	const char *observer_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *target_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Find the objects' IDs.
	nObserverID = objects->Fetch_item_number_by_name(observer_name);

	nTargetID = objects->Fetch_item_number_by_name(target_name);

	// Now we can make the actual call to the line-of-sight object.
	if ((nTargetID != PX_LINKED_DATA_FILE_ERROR) && (nObserverID != PX_LINKED_DATA_FILE_ERROR)) {
		PXTRY

		g_oLineOfSight->UnSubscribe(nObserverID, nTargetID);

		PXCATCH

		Fatal_error("Exception in _line_of_sight::UnSubscribe()");

		PXENDCATCH
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_can_see(int32 &result, int32 *params) {
	uint32 nTargetID;

	const char *target_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Find ID of target object.
	nTargetID = objects->Fetch_item_number_by_name(target_name);

	// Don't call line-of-sight for an invalid ID.
	if (nTargetID != PX_LINKED_DATA_FILE_ERROR) {
		PXTRY

		result = g_oLineOfSight->LineOfSight(cur_id, nTargetID);

		PXCATCH

		Fatal_error("Exception in _line_of_sight::LineOfSight()");

		PXENDCATCH
	} else {
		result = FALSE8;
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_can_object_see(int32 &result, int32 *params) {
	uint32 nObserverID;
	uint32 nTargetID;

	const char *observer_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *target_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Find the objects' IDs.
	nObserverID = objects->Fetch_item_number_by_name(observer_name);

	if (nObserverID == PX_LINKED_DATA_FILE_ERROR)
		Fatal_error("Object %s not found in fn_can_object_see()", observer_name);

	nTargetID = objects->Fetch_item_number_by_name(target_name);

	if (nTargetID == PX_LINKED_DATA_FILE_ERROR)
		Fatal_error("Object %s not found in fn_can_object_see()", target_name);

	// Don't call line-of-sight for invalid IDs.
	if ((nTargetID != PX_LINKED_DATA_FILE_ERROR) && (nObserverID != PX_LINKED_DATA_FILE_ERROR)) {
		PXTRY

		result = g_oLineOfSight->LineOfSight(nObserverID, nTargetID);

		PXCATCH

		Fatal_error("Exception in _line_of_sight::LineOfSight()");

		PXENDCATCH
	} else {
		result = FALSE8;
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_mega_field_of_view(int32 &, int32 *params) {
	// Make the call to do the work.
	PXTRY

	g_oLineOfSight->SetFieldOfView(cur_id, (uint32)(params[0]));

	PXCATCH

	Fatal_error("Exception in _line_of_sight::SetFieldOfView()");

	PXENDCATCH

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_object_sight_range(int32 &, int32 *params) {
	// Make the call to do the work.
	PXTRY

	g_oLineOfSight->SetSightRange(cur_id, (uint32)(params[0]));

	PXCATCH

	Fatal_error("Exception in _line_of_sight::SetSightRange()");

	PXENDCATCH

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_object_sight_height(int32 &, int32 *params) {
	// Make the call to do the work.
	PXTRY

	g_oLineOfSight->SetSightHeight(cur_id, (uint32)(params[0]));

	PXCATCH

	Fatal_error("Exception in _line_of_sight::SetSightHeight()");

	PXENDCATCH

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_line_of_sight_suspend(int32 &, int32 *) {
	// Make the call to the line-of-sight object.
	g_oLineOfSight->Suspend(cur_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_line_of_sight_unsuspend(int32 &, int32 *) {
	// Make the call to the line-of-sight object.
	g_oLineOfSight->Unsuspend(cur_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_line_of_sight_now(int32 &result, int32 *params) {
	uint32 nTargetID;

	const char *target_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	nTargetID = objects->Fetch_item_number_by_name(target_name);

	if (nTargetID == PX_LINKED_DATA_FILE_ERROR)
		Fatal_error("Object %s not found in fn_line_of_sight_now()", target_name);

	// Make the call to the line-of-sight object, overriding any height checks.
	result = g_oLineOfSight->ObjectToObject(cur_id, nTargetID, LIGHT, 0, (_line_of_sight::ActorEyeMode)params[1], TRUE8);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_can_see_in_dark(int32 &, int32 *params) {
	bool8 bOnOff;

	// Make the call to the line-of-sight object.
	bOnOff = ((int32)params[0] == 0) ? FALSE8 : TRUE8;
	g_oLineOfSight->SetCanSeeInDarkFlag(cur_id, bOnOff);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_mega_never_in_shadow(int32 &, int32 *params) {
	bool8 bOnOff;

	// Make the call to the line-of-sight object.
	bOnOff = ((int32)params[0] == 0) ? FALSE8 : TRUE8;
	g_oLineOfSight->SetNeverInShadowFlag(cur_id, bOnOff);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_set_hearing_sensitivity(int32 &, int32 *params) {
	// Pass call on to the sound logic engine.
	g_oSoundLogicEngine->SetHearingSensitivity(cur_id, (int32)params[0]);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_heard_something(int32 &result, int32 *) {
	// Write the call in the debug file.
	Zdebug("fn_sound_heard_something()");

	// Get the value from the sound logic engine.
	result = g_oSoundLogicEngine->MegaHeardSomething(cur_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_heard_this(int32 &result, int32 *params) {
	const char *sound_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Write the call in the debug file.
	Zdebug("fn_sound_heard_this(%s)", sound_name);

	// Get the value from the sound logic engine.
	result = g_oSoundLogicEngine->MegaHeardThis(cur_id, sound_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_new_entry(int32 &, int32 *params) {
	const char *sound_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Write the call in the debug file.
	Zdebug("fn_sound_new_entry(%s)", sound_name);

	// Pass call on to the sound logic engine.
	g_oSoundLogicEngine->AddSubscription(cur_id, sound_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_remove_entry(int32 &, int32 *params) {
	const char *sound_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Write the call in the debug file.
	Zdebug("fn_sound_remove_entry(%s)", sound_name);

	// Pass call on to the sound logic engine.
	g_oSoundLogicEngine->RemoveSubscription(cur_id, sound_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_suspend(int32 &, int32 *) {
	// Write the call in the debug file.
	Zdebug("fn_sound_suspend() called by mega %d", cur_id);

	// Turn sound events off
	g_oSoundLogicEngine->SetSuspendedFlag(cur_id, TRUE8);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_unsuspend(int32 &, int32 *) {
	// Write the call in the debug file.
	Zdebug("fn_sound_unsuspend() called by mega %d", cur_id);

	// Turn sound events on.
	g_oSoundLogicEngine->SetSuspendedFlag(cur_id, FALSE8);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_get_x(int32 &result, int32 *params) {
	uint32 nHashedSoundID;

	const char *sound_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Pass call on to the sound logic engine.
	nHashedSoundID = HashString(sound_name);
	result = g_oSoundLogicEngine->GetSoundX(nHashedSoundID);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_get_z(int32 &result, int32 *params) {
	uint32 nHashedSoundID;

	const char *sound_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Pass call on to the sound logic engine.
	nHashedSoundID = HashString(sound_name);
	result = g_oSoundLogicEngine->GetSoundZ(nHashedSoundID);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_fast_face(int32 &, int32 *params) {
	uint32 nHashedSoundID;
	int32 nX, nZ;

	const char *sound_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// See if this is the first time this function has been called from script.
	if (!L->looping) {
		// Work out the hashed name of the sound.
		nHashedSoundID = HashString(sound_name);

		// Get the coordinates of the sound.
		nX = g_oSoundLogicEngine->GetSoundX(nHashedSoundID);
		nZ = g_oSoundLogicEngine->GetSoundZ(nHashedSoundID);

		// Check that a valid coordinate was found.
		if ((nX == SL_UNDEFINED_COORDINATE) || (nZ == SL_UNDEFINED_COORDINATE)) {
			// Invalid coordinate, so ignore the script function.
			return (IR_CONT);
		}

		if (Calc_target_pan((PXreal)nX, (PXreal)nZ, M->actor_xyz.x, M->actor_xyz.z)) {
			// The mega needs to do a turn.
			L->looping = TRUE8;
		} else {
			// No animated turn required.
			return (IR_CONT);
		}
	}

	// We are running an animation to face the sound.
	if (M->target_pan) {
		// Still animating the turn.
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, (uint32)params[1]);
		return (IR_REPEAT);
	} else {
		// Finished the turn.
		L->looping = FALSE8;

		// Set back to stand.
		L->cur_anim_type = __STAND;
		L->anim_pc = 0;

		// Calling script can continue.
		return (IR_CONT);
	}

	// I think this is needed to stop PSX warning.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_route_to_near(int32 &result, int32 *params) {
	int32 nDeltaX, nDeltaZ;
	int32 nX, nZ;
	int32 nSquaredDistance;
	uint32 nHashedSoundID;

	const char *sound_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Work out the hashed name of the sound.
	nHashedSoundID = HashString(sound_name);

	// Get the coordinates of the sound.
	nX = g_oSoundLogicEngine->GetSoundX(nHashedSoundID);
	nZ = g_oSoundLogicEngine->GetSoundZ(nHashedSoundID);

	// Check that a valid coordinate was found.
	if ((nX == SL_UNDEFINED_COORDINATE) || (nZ == SL_UNDEFINED_COORDINATE)) {
		// Invalid coordinate, so ignore the script function.
		return (IR_CONT);
	}

	// Work out squared difference between where we are and where we need to be.
	nDeltaX = nX - (int32)logic_structs[cur_id]->mega->actor_xyz.x;
	nDeltaZ = nZ - (int32)logic_structs[cur_id]->mega->actor_xyz.z;
	nSquaredDistance = nDeltaX * nDeltaX + nDeltaZ * nDeltaZ;

	if (!L->looping) {
		// Don't build a route if mega is already within the required distance.
		if (nSquaredDistance < ((int32)params[3] * (int32)params[3])) {
			result = TRUE8;
			return (IR_CONT);
		}

		// Not close enough, so we need to build a route.
		if (!Setup_route(result, nX, nZ, (int32)params[1], __FULL, TRUE8)) {
			return (IR_CONT);
		}
	}

	// Run the route.
	if (Process_route()) {
		L->looping = 0;
		result = TRUE8;
		return (IR_CONT);
	}

	if (nSquaredDistance < ((int32)params[3] * (int32)params[3])) {
		M->m_main_route.dist_left = FLOAT_ZERO;
		M->m_main_route.current_position = (M->m_main_route.total_points - 1);
	}

	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_link_floors(int32 &, int32 *params) {
	const char *sound_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *sound_name2 = (const char *)MemoryUtil::resolvePtr(params[1]);

	Tdebug(EVENT_FNS_LOG, "fn_sound_link_floors( %s, %s )", sound_name, sound_name2);

	// Make the call to the sound logic engine.
	g_oSoundLogicEngine->LinkFloorsForSoundEvents(sound_name, sound_name2);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sound_simulate(int32 &, int32 *) { return (IR_CONT); }

} // End of namespace ICB
