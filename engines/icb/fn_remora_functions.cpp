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
#include "engines/icb/remora.h"
#include "engines/icb/text_sprites.h"
#include "engines/icb/icon_list_manager.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/mission.h"

namespace ICB {

// This defines the maximum number of locks in the one-off mission 01 interface.
#define MAX_LOCKS_IN_M08 12

// This variable works like the L->looping flag, but controls a handful of tri-state functions in this file.
uint32 nFunctionState;

void RemoraStandardRestart(uint32 nRemoraID);
const char *SkipLineNumber(const char *pcLine);

mcodeFunctionReturnCodes fn_remora_script_activate(int32 &result, int32 *params) { return (MS->fn_remora_script_activate(result, params)); }

mcodeFunctionReturnCodes fn_remora_script_deactivate(int32 &result, int32 *params) { return (MS->fn_remora_script_deactivate(result, params)); }

mcodeFunctionReturnCodes fn_remora_is_active(int32 &result, int32 *params) { return (MS->fn_remora_is_active(result, params)); }

mcodeFunctionReturnCodes fn_remora_get_mode(int32 &result, int32 *params) { return (MS->fn_remora_get_mode(result, params)); }

mcodeFunctionReturnCodes fn_remora_set_mode(int32 &result, int32 *params) { return (MS->fn_remora_set_mode(result, params)); }

// These handle menus within the Remora.
mcodeFunctionReturnCodes fn_remora_add_icon(int32 &result, int32 *params) { return (MS->fn_remora_add_icon(result, params)); }

mcodeFunctionReturnCodes fn_remora_remove_icon(int32 &result, int32 *params) { return (MS->fn_remora_remove_icon(result, params)); }

mcodeFunctionReturnCodes fn_remora_reset_icon_list(int32 &result, int32 *params) { return (MS->fn_remora_reset_icon_list(result, params)); }

mcodeFunctionReturnCodes fn_remora_choose(int32 &result, int32 *params) { return (MS->fn_remora_choose(result, params)); }

mcodeFunctionReturnCodes fn_remora_wait_on_icon(int32 &result, int32 *params) { return (MS->fn_remora_wait_on_icon(result, params)); }

mcodeFunctionReturnCodes fn_remora_new_menu(int32 &result, int32 *params) { return (MS->fn_remora_new_menu(result, params)); }

mcodeFunctionReturnCodes fn_remora_new_menu_on_icon(int32 &result, int32 *params) { return (MS->fn_remora_new_menu_on_icon(result, params)); }

mcodeFunctionReturnCodes fn_remora_menu_return(int32 &result, int32 *params) { return (MS->fn_remora_menu_return(result, params)); }

mcodeFunctionReturnCodes fn_remora_menu_return_on_icon(int32 &result, int32 *params) { return (MS->fn_remora_menu_return_on_icon(result, params)); }

// These handle text display within the Remora.
mcodeFunctionReturnCodes fn_remora_paragraph_text(int32 &result, int32 *params) { return (MS->fn_remora_paragraph_text(result, params)); }

mcodeFunctionReturnCodes fn_remora_main_heading(int32 &result, int32 *params) { return (MS->fn_remora_main_heading(result, params)); }

mcodeFunctionReturnCodes fn_remora_sub_heading(int32 &result, int32 *params) { return (MS->fn_remora_sub_heading(result, params)); }

mcodeFunctionReturnCodes fn_remora_option_text(int32 &result, int32 *params) { return (MS->fn_remora_option_text(result, params)); }

mcodeFunctionReturnCodes fn_remora_warning_text(int32 &result, int32 *params) { return (MS->fn_remora_warning_text(result, params)); }

mcodeFunctionReturnCodes fn_remora_blank_line(int32 &result, int32 *params) { return (MS->fn_remora_blank_line(result, params)); }

// These handle graphics within the Remora.
mcodeFunctionReturnCodes fn_remora_clear_screen(int32 &result, int32 *params) { return (MS->fn_remora_clear_screen(result, params)); }

mcodeFunctionReturnCodes fn_remora_picture(int32 &result, int32 *params) { return (MS->fn_remora_picture(result, params)); }

mcodeFunctionReturnCodes fn_remora_set_max_zoom(int32 &result, int32 *params) { return (MS->fn_remora_set_max_zoom(result, params)); }

mcodeFunctionReturnCodes fn_remora_set_min_zoom(int32 &result, int32 *params) { return (MS->fn_remora_set_min_zoom(result, params)); }

mcodeFunctionReturnCodes fn_remora_set_current_zoom(int32 &result, int32 *params) { return (MS->fn_remora_set_current_zoom(result, params)); }

mcodeFunctionReturnCodes fn_remora_emp_flash(int32 &result, int32 *params) { return (MS->fn_remora_emp_flash(result, params)); }

// These deal with the Remora's email system.
mcodeFunctionReturnCodes fn_remora_send_email(int32 &result, int32 *params) { return (MS->fn_remora_send_email(result, params)); }

mcodeFunctionReturnCodes fn_remora_is_email_waiting(int32 &result, int32 *params) { return (MS->fn_remora_is_email_waiting(result, params)); }

mcodeFunctionReturnCodes fn_remora_mark_email_read(int32 &result, int32 *params) { return (MS->fn_remora_mark_email_read(result, params)); }

mcodeFunctionReturnCodes fn_remora_check_email_id(int32 &result, int32 *params) { return (MS->fn_remora_check_email_id(result, params)); }

// Miscellaneous Remora functions.
mcodeFunctionReturnCodes fn_remora_fix_motion_scan_xz(int32 &result, int32 *params) { return (MS->fn_remora_fix_motion_scan_xz(result, params)); }

mcodeFunctionReturnCodes fn_remora_default_logic(int32 &result, int32 *params) { return (MS->fn_remora_default_logic(result, params)); }

mcodeFunctionReturnCodes fn_remora_mega_says(int32 &result, int32 *params) { return (MS->fn_remora_mega_says(result, params)); }

mcodeFunctionReturnCodes fn_remora_add_floor_range(int32 &result, int32 *params) { return (MS->fn_remora_add_floor_range(result, params)); }

mcodeFunctionReturnCodes fn_remora_reset_floor_ranges(int32 &result, int32 *params) { return (MS->fn_remora_reset_floor_ranges(result, params)); }

mcodeFunctionReturnCodes fn_remora_set_map_knowledge_level(int32 &result, int32 *params) { return (MS->fn_remora_set_map_knowledge_level(result, params)); }

mcodeFunctionReturnCodes fn_remora_update_player(int32 &result, int32 *params) { return (MS->fn_remora_update_player(result, params)); }

mcodeFunctionReturnCodes fn_remora_progress_bar(int32 &result, int32 *params) { return (MS->fn_remora_progress_bar(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_remora_update_player(int32 &, int32 *) {
	_input *state;

	// If the Remora is now shut down, we need to return to player-standing mode.
	if (!g_oRemora->IsActive()) {
		return (IR_CONT);
	}

	// check keys/pads/etc. to see what the user is trying to do
	player.Update_input_state();

	state = player.Fetch_input_state();

	// Now just run its logic.  It will process the 'deactivate' message if one was posted in previous line.
	g_oRemora->CycleRemoraLogic(*state);

	// Cycle the inventory logic if it is active.  Note though that the fact that the Remora is active does
	// not mean the inventory is, because there could be a gap between menus being displayed.
	if (g_oIconListManager->IsActive())
		g_oIconListManager->CycleInventoryLogic(*state);

	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_is_active(int32 &result, int32 *) {
	// Call the function that does the work.
	result = g_oRemora->IsActive();

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_get_mode(int32 &result, int32 *) {
	// First check that the Remora is active.
	if (g_oRemora->IsActive())
		result = (int32)g_oRemora->GetMode();
	else
		result = REMORA_NOT_ACTIVE;

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_set_mode(int32 &, int32 *params) {
	// Set the mode.
	g_oRemora->SetMode((_remora::RemoraMode)params[0]);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_picture(int32 &, int32 *params) {
	uint32 nXOffset;

	const char *picture_name = (const char *)MemoryUtil::resolvePtr(params[2]);

	// Write the call in the debug file.
	Zdebug("fn_remora_picture( %d, %d, %s )", (int32)params[0], (int32)params[1], picture_name);

	// Neither can pixel offset.
	nXOffset = ((int32)params[1] < 0) ? 0 : (uint32)params[1];

	// Make the call to the Remora.
	g_oRemora->SetupPicture(nXOffset, picture_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_clear_screen(int32 &, int32 *) {
	// Make the call to the Remora object.
	g_oRemora->ClearAllText();

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_add_icon(int32 &, int32 *params) {
	char pcIconPath[ENGINE_STRING_LEN];

	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Make the call to the Remora object.
	g_oIconListManager->AddIconToList(ICON_LIST_REMORA, icon_name);

	// Preload the icon for PSX smoothing.
	sprintf(pcIconPath, ICON_PATH);
	g_oIconMenu->PreloadIcon(pcIconPath, icon_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_remove_icon(int32 &, int32 *params) {
	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Make the call to the Remora object.
	g_oIconListManager->RemoveIconFromList(ICON_LIST_REMORA, icon_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_reset_icon_list(int32 &, int32 *) {
	// Make the call to the Remora object.
	g_oIconListManager->ResetList(ICON_LIST_REMORA);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_choose(int32 &, int32 *) {
	// This function must be used from inside the Remora.
	if (!g_oRemora->IsActive())
		Fatal_error("fn_remora_choose() cannot be used outside the Remora");

	// Make the call to the Remora object.
	g_oIconListManager->ActivateIconMenu(ICON_LIST_REMORA, TRUE8, FALSE8);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_paragraph_text(int32 &, int32 *params) {
	const char *pcText;

	// Find the text in the resources.
	pcText = g_oRemora->LocateTextFromReference((uint32)params[1]);

	if (pcText) {
		// Skip line numbers.
		pcText = SkipLineNumber(pcText);

		// Set the text.
		g_oRemora->SetText(pcText, REMORA_TEXT_PARAGRAPH, 1, PIN_AT_TOP_LEFT);
	} else {
		// We failed to find it.
		Fatal_error("Unable to find text for reference %x in fn_remora_paragraph_text()", (uint32)params[1]);
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_main_heading(int32 &, int32 *params) {
	const char *pcText;

	// Find the text in the resources.
	pcText = g_oRemora->LocateTextFromReference((uint32)params[0]);

	if (pcText) {
		// Skip line numbers.
		pcText = SkipLineNumber(pcText);

		// Set the text.
		g_oRemora->SetText(pcText, REMORA_TEXT_HEADING, 0, PIN_AT_CENTRE);
	} else {
		// We failed to find it.
		Fatal_error("Unable to find text for reference %x in fn_remora_main_heading()", (uint32)params[0]);
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_sub_heading(int32 &, int32 *params) {
	const char *pcText;

	// Find the text in the resources.
	pcText = g_oRemora->LocateTextFromReference((uint32)params[1]);

	if (pcText) {
		// Skip line numbers.
		pcText = SkipLineNumber(pcText);

		// Set the text.
		g_oRemora->SetText(pcText, REMORA_TEXT_HEADING, 0, PIN_AT_TOP_LEFT);
	} else {
		// We failed to find it.
		Fatal_error("Unable to find text for reference %x in fn_remora_sub_heading()", (uint32)params[1]);
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_option_text(int32 &, int32 *params) {
	const char *pcText;

	// Find the text in the resources.
	pcText = g_oRemora->LocateTextFromReference((uint32)params[1]);

	if (pcText) {
		// Skip line numbers.
		pcText = SkipLineNumber(pcText);

		// Set the text.
		g_oRemora->SetText(pcText, REMORA_TEXT_OPTION, 0, PIN_AT_TOP_LEFT);
	} else {
		// We failed to find it.
		Fatal_error("Unable to find text for reference %x in fn_remora_option_text()", (uint32)params[1]);
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_warning_text(int32 &, int32 *params) {
	const char *pcText;

	// Find the text in the resources.
	pcText = g_oRemora->LocateTextFromReference((uint32)params[1]);

	if (pcText) {
		// Skip line numbers.
		pcText = SkipLineNumber(pcText);

		// Set the text.
		g_oRemora->SetText(pcText, REMORA_TEXT_WARNING, 0, PIN_AT_CENTRE);
	} else {
		// We failed to find it.
		Fatal_error("Unable to find text for reference %x in fn_remora_warning_text()", (uint32)params[1]);
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_blank_line(int32 &, int32 *) {
	// Set the blank line.
	g_oRemora->SetText(NULL, 0, 0, PIN_AT_CENTRE);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_progress_bar(int32 &, int32 *params) {
	int32 nTargetCycles;
	int32 nCurrentValue;

	// Get cycles to run bar over.
	nTargetCycles = (int32)params[0];

	// Is this a new bar?
	if (!L->looping) {
		// We are now doing a script loop.
		L->looping = TRUE8;

		// Initially none of the bar is there.
		g_oRemora->SetProgressBarValue(0);

		// Set target value for bar.
		g_oRemora->SetProgressBarTotal(nTargetCycles);

		// And here I set a tri-state function control variable, to help with control.
		nFunctionState = 1;

		// Come back to this function next time.
		return (IR_REPEAT);
	}

	// Check if we are incrementing progress bar or holding it for a cycle at 100%.
	if (nFunctionState == 1) {
		// Still increasing it.  Get current bar position.
		nCurrentValue = g_oRemora->GetProgressBarValue();

		// Update bar count.
		++nCurrentValue;

		// Set current complete.
		g_oRemora->SetProgressBarValue(nCurrentValue);

		// Check if done yet.
		if (nCurrentValue == nTargetCycles) {
			// Finished, but we want to come back and hold it for a cycle.
			nFunctionState = 2;
			return (IR_REPEAT);
		} else {
			// Return to increment the bar some more next time.
			return (IR_REPEAT);
		}
	} else {
		// Now we have really finished.
		L->looping = FALSE8;
		g_oRemora->SetProgressBarValue(-1);
		return (IR_CONT);
	}
}

mcodeFunctionReturnCodes _game_session::fn_remora_fix_motion_scan_xz(int32 &, int32 *) {
	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_script_activate(int32 &, int32 *params) {
	_input *psInputState;

	// In case the script writer has forgotten to do it, drop any icon currently held.
	g_oIconListManager->Drop();

	// This tells the UI that the Remora is now active.
	MS->player.Push_control_mode(ACTOR_RELATIVE);
	MS->player.Set_player_status(REMORA);

	// This flag is used to indicate that the Remora has been activated in a specific mode (i.e. from script,
	// rather than the keyboard).  It is used to stop the Remora dropping into its default menu selection upon
	// activation; instead it will use the mode set here.
	g_oRemora->SetModeOverride((_remora::RemoraMode)params[0]);

	// This sets a flag which the Remora will pick up next cycle.
	g_oRemora->ActivateRemora((_remora::RemoraMode)params[0]);

	// Cycle the Remora's logic to make sure drawing code doesn't try to do a draw before the
	// Remora has had a chance to set itself up.
	MS->player.Update_input_state();
	psInputState = MS->player.Fetch_input_state();
	g_oRemora->CycleRemoraLogic(*psInputState);

	// Stop key bounce.
	MS->player.remora_lock = TRUE8;

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_script_deactivate(int32 &, int32 *) {
	_input sInputState;

	// Write the call in the debug files.
	Zdebug("fn_remora_script_deactivate();");

	// If the Remora is not active then there is nothing to do.
	if (!g_oRemora->IsActive())
		return (IR_CONT);

	// The script may be set up in such a way that the icon that was selected to cause this deactivate
	// to happen may not have cleared the currently-selected icon, so do it here to be safe.
	g_oIconListManager->Drop();

	// Deactivate the Remora.
	g_oRemora->SetMode(_remora::MOTION_SCAN);
	g_oRemora->DeactivateRemora(TRUE8);
	sInputState.UnSetButton((const ButtonEnums)(0xff));
	g_oRemora->CycleRemoraLogic(sInputState);

	// This tells the UI that the Remora is now gone and the player is back.
	MS->player.Pop_control_mode();

	logic_structs[player.Fetch_player_id()]->mega->weapon = __NOT_ARMED;
	logic_structs[player.Fetch_player_id()]->voxel_info->___init(logic_structs[player.Fetch_player_id()]->mega->chr_name,
	                                                             logic_structs[player.Fetch_player_id()]->mega->anim_set, __NOT_ARMED);

	MS->player.Set_player_status(STOOD);

	MS->Setup_prop_sleep_states(); // recompute prop sleep states once we leave remora

	// Stop key bounce.
	MS->player.remora_lock = TRUE8;

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_default_logic(int32 &, int32 *) {
	// Check to see if we are already looping
	if (!L->looping) {
		// Mark the fact that we are now looping.
		L->looping = 1;

		// Clear any events outstanding for the Remora.
		g_oEventManager->ClearAllEventsForObject(cur_id);

		// Make sure there are no icons in the list for the Remora.
		g_oIconListManager->ResetList(ICON_LIST_REMORA);

		// Make sure the Remora's screen is clear.
		g_oRemora->ClearAllText();

		// Must call this function again.
		return (IR_REPEAT);
	} else {
		// We are looping in this function call.  Simply drop out if the Remora is not active.
		if (!g_oRemora->IsActive()) {
			// Remora not currently active.
			return (IR_REPEAT);
		} else {
			// Remora has been activated.  Either drop into the default menu mode, or do the one specified
			// by script if there is one.
			g_oRemora->SetDefaultOrOverrideMode();

			// Make the Remora rerun its logic context.
			g_oEventManager->PostNamedEventToObject(EVENT_LOGIC_RERUN, cur_id, cur_id);

			// We are not looping from now on.
			L->looping = 0;

			// Calling script can continue now.
			return (IR_CONT);
		}
	}

	// To fix a GCC compiler warning
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_new_menu(int32 &, int32 *params) {
	// Set the variables so we jump to the right menu when we restart.
	g_oRemora->AccessMenuLevelVariables(params, _remora::SET);

	// Jump.
	RemoraStandardRestart(cur_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_new_menu_on_icon(int32 &, int32 *params) {
	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[5]);

	// Check to see if we are already looping
	if (!L->looping) {
		// Set the variables so we jump to the right menu when we restart.
		g_oRemora->AccessMenuLevelVariables(params, _remora::SET);

		// Remove any icons from the icon list.
		g_oIconListManager->ResetList(ICON_LIST_REMORA);

		// Add the given icon to the Remora icon list.
		g_oIconListManager->AddIconToList(ICON_LIST_REMORA, icon_name);

		// Activate the chooser menu.
		g_oIconListManager->ActivateIconMenu(ICON_LIST_REMORA, TRUE8, FALSE8);

		// Mark the fact that we are now looping.
		L->looping = 1;

		// And here I set the tri-state function control variable, so we know which bit to run next.
		nFunctionState = 1;

		// Drop out but call this function again next time.
		return (IR_REPEAT);
	} else {
		// We are looping on this function.  The set-up part must already have been done.  But the function has
		// two subsequent states.  See which one we're on.
		if (nFunctionState == 1) {
			// We are looping on this function.  The set-up part must already have been done.
			// Check for the icon being selected.
			if (g_oIconListManager->ItemHeld() && g_oIconListManager->Holding(icon_name)) {
				// Drop the item held.
				g_oIconListManager->Drop();

				// Cord speaks his instructions to the Remora.
				g_oRemora->DisplayCharacterSpeech((uint32)params[6]);

				// We are now moving to a new state in this function.
				nFunctionState = 2;
				return (IR_REPEAT);
			} else {
				// Icon has not been selected, so we need to come back to this function.
				return (IR_REPEAT);
			}
		} else {
			// Second stage of this function now: displaying mega speech.
			if (g_oRemora->GetSpeechTimer() == 0) {
				// Do the Remora script jump.
				RemoraStandardRestart(cur_id);

				// Allow calling script to continue.
				nFunctionState = 0;
				L->looping = 0;
				return (IR_CONT);
			} else {
				// The speech hasn't been displayed long enough, so hold inside this fn_function().
				return (IR_REPEAT);
			}
		}
	}
}

mcodeFunctionReturnCodes _game_session::fn_remora_wait_on_icon(int32 &, int32 *params) {
	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Check to see if we are already looping
	if (!L->looping) {
		// Remove any icons from the icon list.
		g_oIconListManager->ResetList(ICON_LIST_REMORA);

		// Add the given icon to the Remora icon list.
		g_oIconListManager->AddIconToList(ICON_LIST_REMORA, icon_name);

		// Activate the chooser menu.
		g_oIconListManager->ActivateIconMenu(ICON_LIST_REMORA, TRUE8, FALSE8);

		// Mark the fact that we are now looping.
		L->looping = 1;

		// And here I set the tri-state function control variable, so we know which bit to run next.
		nFunctionState = 1;

		// Drop out but call this function again next time.
		return (IR_REPEAT);
	} else {
		// We are looping on this function.  The set-up part must already have been done.  But the function has
		// two subsequent states.  See which one we're on.
		if (nFunctionState == 1) {
			if (g_oIconListManager->ItemHeld() && g_oIconListManager->Holding(icon_name)) {
				// Yes, user has selected the icon to start a new mode.  First drop item held.
				g_oIconListManager->Drop();

				// Cord speaks his instructions to the Remora.
				g_oRemora->DisplayCharacterSpeech((uint32)params[2]);

				// We are now moving to a new state in this function.
				nFunctionState = 2;

				// Drop out but call this function again next time.
				return (IR_REPEAT);
			} else {
				// Drop out but call this function again next time.
				return (IR_REPEAT);
			}
		} else {
			// Second stage of this function now: displaying mega speech.
			if (g_oRemora->GetSpeechTimer() == 0) {
				// Do the Remora new mode.
				g_oRemora->SetMode((_remora::RemoraMode)params[1]);
				RemoraStandardRestart(cur_id);

				// Allow calling script to continue.
				nFunctionState = 0;
				L->looping = 0;
				return (IR_CONT);
			} else {
				// The speech hasn't been displayed long enough, so hold inside this fn_function().
				return (IR_REPEAT);
			}
		}
	}
}

mcodeFunctionReturnCodes _game_session::fn_remora_menu_return(int32 &, int32 *params) {
	int32 i;
	uint32 nReturned;
	int32 pnMenuVariables[REMORA_MENU_DEPTH];

	// Get the current state of the menu variables.
	g_oRemora->AccessMenuLevelVariables(pnMenuVariables, _remora::GET);

	// Find out how deep we are already in the menu tree.
	i = REMORA_MENU_DEPTH - 1;
	while ((i >= 0) && (pnMenuVariables[i] == 0))
		--i;

	// If we ran off the end then we are already at the root, but there is little value in making this an error.
	if (i < 0)
		return (IR_CONT);

	// We are at a certain depth in the menu tree, so back up the required amount.  If we run off the end,
	// this means user has requested to go back more levels than we are deep, but just ignore this.
	nReturned = 0;
	while ((i >= 0) && (nReturned < (uint32)params[0])) {
		pnMenuVariables[i] = 0;
		--i;
		++nReturned;
	}

	// Put the new values back in the script object.
	g_oRemora->AccessMenuLevelVariables(pnMenuVariables, _remora::SET);

	// Jump.
	RemoraStandardRestart(cur_id);

	// Allow calling script to continue.
	return (IR_CONT);

}

mcodeFunctionReturnCodes _game_session::fn_remora_menu_return_on_icon(int32 &, int32 *params) {
	int32 i;
	uint32 nReturned;
	int32 pnMenuVariables[REMORA_MENU_DEPTH];

	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Check to see if we are already looping
	if (!L->looping) {
		// First time in, so display the icon.
		g_oIconListManager->ResetList(ICON_LIST_REMORA);
		g_oIconListManager->AddIconToList(ICON_LIST_REMORA, icon_name);
		g_oIconListManager->ActivateIconMenu(ICON_LIST_REMORA, TRUE8, FALSE8);

		// Mark the fact that we are now looping.
		L->looping = 1;

		// And here I set the tri-state function control variable, so we know which bit to run next.
		nFunctionState = 1;

		// Drop out but call this function again next time.
		return (IR_REPEAT);
	} else {
		// We are looping on this function.  The set-up part must already have been done.  But the function has
		// two subsequent states.  See which one we're on.
		if (nFunctionState == 1) {
			if (g_oIconListManager->ItemHeld() && g_oIconListManager->Holding(icon_name)) {
				// Clear the item held.
				g_oIconListManager->Drop();

				// Remove any icons from the icon list.
				g_oIconListManager->ResetList(ICON_LIST_REMORA);

				// Icon has been selected, so we are going to do the jump back up the menu tree.  Get
				// current state of the menu variables.
				g_oRemora->AccessMenuLevelVariables(pnMenuVariables, _remora::GET);

				// Find out how deep we are already in the menu tree.
				i = REMORA_MENU_DEPTH - 1;
				while ((i >= 0) && (pnMenuVariables[i] == 0))
					--i;

				// If we ran off the end then we are already at the root, but there is no value in
				// making this an error.
				if (i >= 0) {
					// We are at a certain depth in the menu tree, so back up the required amount.  If we run off the
					// end,
					// this means user has requested to go back more levels than we are deep, but just ignore this.
					nReturned = 0;
					while ((i >= 0) && (nReturned < (uint32)params[0])) {
						pnMenuVariables[i] = 0;
						--i;
						++nReturned;
					}
				}

				// Put the new values back in the script object.
				g_oRemora->AccessMenuLevelVariables(pnMenuVariables, _remora::SET);

				// Display what Cord needs to say.
				g_oRemora->DisplayCharacterSpeech((uint32)params[2]);

				// We are now moving to a new state in this function.
				nFunctionState = 2;

				// Drop out but call this function again next time.
				return (IR_REPEAT);
			} else {
				// Icon has not been chosen, so call this function again next time.
				return (IR_REPEAT);
			}
		} else {
			// Second stage of this function now: displaying mega speech.
			if (g_oRemora->GetSpeechTimer() == 0) {
				// Do the Remora script jump.
				RemoraStandardRestart(cur_id);

				// Allow calling script to continue.
				nFunctionState = 0;
				L->looping = 0;
				return (IR_CONT);
			} else {
				// The speech hasn't been displayed long enough, so hold inside this fn_function().
				return (IR_REPEAT);
			}
		}
	}
}

mcodeFunctionReturnCodes _game_session::fn_remora_mega_says(int32 &, int32 *params) {
	// Check to see if we are already looping
	if (!L->looping) {
		// Mark the fact that we are now looping.
		L->looping = 1;

		// Find the text in the resources.
		g_oRemora->DisplayCharacterSpeech((uint32)params[1]);

		// Drop out but call this function again next time.
		return (IR_REPEAT);
	} else {
		// Get the speech timer.  If it has run down to zero, the supplied line has been displayed long enough,
		// so we can go on to the next line of script.
		if (g_oRemora->GetSpeechTimer() == 0) {
			// Allow calling script to continue.
			L->looping = 0;
			return (IR_CONT);
		}

		// The speech hasn't been displayed long enough, so hold inside this fn_function().
		return (IR_REPEAT);
	}
}

mcodeFunctionReturnCodes _game_session::fn_remora_add_floor_range(int32 &, int32 *params) {
	// Simply pass the call on to the Remora.
	g_oRemora->AddFloorRange((uint32)params[0], (uint32)params[1]);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_reset_floor_ranges(int32 &, int32 *) {
	// Simply pass the call on to the Remora.
	g_oRemora->ResetFloorRanges();

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_send_email(int32 &, int32 *params) {
	const char *email_id = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Set the email in the Remora.
	g_oRemora->NewEmail(email_id);

	// Here we borrow the logic in the icon menu which flashes added medipacks etc.
	g_oIconListManager->SetEmailArrived();

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_is_email_waiting(int32 &result, int32 *) {
	// Make the call to the Remora.
	result = (int32)g_oRemora->IsEmailWaiting();

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_check_email_id(int32 &result, int32 *params) {
	const char *email_id = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Check with the Remora if the string matches.
	result = g_oRemora->IsThisEmailWaiting(email_id);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_mark_email_read(int32 &, int32 *) {
	// Turn off the email-waiting flag in the Remora.
	g_oRemora->MarkEmailRead();

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_set_max_zoom(int32 &, int32 *params) {
	// Call the Remora function to do it.
	g_oRemora->SetMaximumZoom((uint32)params[0]);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_set_min_zoom(int32 &, int32 *params) {
	// Call the Remora function to do it.
	g_oRemora->SetMinimumZoom((uint32)params[0]);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_set_current_zoom(int32 &, int32 *params) {
	// Call the Remora function to do it.
	g_oRemora->SetCurrentZoom((uint32)params[0]);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_emp_flash(int32 &, int32 *) {
	// If the Remora is not active, just ignore this function.
	if (g_oRemora->IsActive()) {
		// Check to see if we are already looping
		if (!L->looping) {
			// Mark the fact that we are now looping.
			L->looping = 1;
		}

		// See if the EMP effect is still running or not.
		if (g_oRemora->EMPEffect()) {
			// Menu still active, so we must continue cycling its logic next time through.
			return (IR_REPEAT);
		} else {
			// A selection has been made or the menu has been cancelled.  Calling script can now continue.
			L->looping = 0;
			return (IR_CONT);
		}
	}

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remora_set_map_knowledge_level(int32 &, int32 * /*params*/) {
	// Calling script can continue.
	return (IR_CONT);
}

void RemoraStandardRestart(uint32 nRemoraID) {
	// Clear the item held if any.
	g_oIconListManager->Drop();

	// This blockRemove any icons from the icon list.
	g_oIconListManager->ResetList(ICON_LIST_REMORA);

	// Clear all text.
	g_oRemora->ClearAllText();

	MS->logic_structs[nRemoraID]->context_request = TRUE8;
}

const char *SkipLineNumber(const char *pcLine) {
	const char *pcParsePos;

	// If line number display is turned on, don't do anything.
	if (px.speechLineNumbers)
		return (pcLine);

	// Initialise parse pointer.
	pcParsePos = pcLine;

	// If the first character is one that marks the line as being spoken or not, skip it.
	if ((*pcParsePos == TS_SPOKEN_LINE) || (*pcParsePos == TS_NON_SPOKEN_LINE))
		++pcParsePos;

	// The first character has to start the line number to be valid.
	if (*pcParsePos == TS_LINENO_OPEN) {
		// Okay, we appear to have a legal line number.  Find the close brace for it.
		while ((*pcParsePos != '\0') && (*pcParsePos != TS_LINENO_CLOSE))
			++pcParsePos;

		// If we didn't find one then this is an error.
		if (*pcParsePos == '\0')
			Fatal_error("Failed to find the end of the line number in [%s]", pcLine);

		// Skip to first non-space after the line number.
		++pcParsePos;
		while ((*pcParsePos != '\0') && (*pcParsePos == ' '))
			++pcParsePos;

		// If we got to the end of the string then we have a line number with no text following it.
		if (*pcParsePos == '\0')
			Fatal_error("Found line number [%s] with no text in SkipLineNumber()", pcLine);

		// Write the modified pointer back into the text block.
		return (pcParsePos);
	} else {
		// No line number so nothing to chop off.
		return (pcParsePos);
	}
}

} // End of namespace ICB
