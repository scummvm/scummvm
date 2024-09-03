/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/icb.h"
#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/mission.h"
#include "engines/icb/remora.h"
#include "engines/icb/text_sprites.h"
#include "engines/icb/icon_list_manager.h"

namespace ICB {

// These are general icon list functions.
mcodeFunctionReturnCodes fn_is_carrying(int32 &result, int32 *params) { return (MS->fn_is_carrying(result, params)); }

mcodeFunctionReturnCodes fn_carrying_how_many(int32 &result, int32 *params) { return (MS->fn_carrying_how_many(result, params)); }

mcodeFunctionReturnCodes fn_is_holding(int32 &result, int32 *params) { return (MS->fn_is_holding(result, params)); }

mcodeFunctionReturnCodes fn_item_held(int32 &result, int32 *params) { return (MS->fn_item_held(result, params)); }

mcodeFunctionReturnCodes fn_drop(int32 &result, int32 *params) { return (MS->fn_drop(result, params)); }

mcodeFunctionReturnCodes fn_reset_icon_list(int32 &result, int32 *params) { return (MS->fn_reset_icon_list(result, params)); }

mcodeFunctionReturnCodes fn_add_icon_to_icon_list(int32 &result, int32 *params) { return (MS->fn_add_icon_to_icon_list(result, params)); }

mcodeFunctionReturnCodes fn_remove_icon_from_icon_list(int32 &result, int32 *params) { return (MS->fn_remove_icon_from_icon_list(result, params)); }

mcodeFunctionReturnCodes fn_interact_choose(int32 &result, int32 *params) { return (MS->fn_interact_choose(result, params)); }

mcodeFunctionReturnCodes fn_destroy_icon_list(int32 &result, int32 *params) { return (MS->fn_destroy_icon_list(result, params)); }

// These two are wrappers for dealing with the inventory (save having to pass in the name of the inventory icon list).
mcodeFunctionReturnCodes fn_add_inventory_item(int32 &result, int32 *params) { return (MS->fn_add_inventory_item(result, params)); }

mcodeFunctionReturnCodes fn_remove_inventory_item(int32 &result, int32 *params) { return (MS->fn_remove_inventory_item(result, params)); }

// These are wrappers for handling functions to do with ammo and health packs.
mcodeFunctionReturnCodes fn_add_medipacks(int32 &result, int32 *params) { return (MS->fn_add_medipacks(result, params)); }

mcodeFunctionReturnCodes fn_use_medipacks(int32 &result, int32 *params) { return (MS->fn_use_medipacks(result, params)); }

mcodeFunctionReturnCodes fn_add_ammo_clips(int32 &result, int32 *params) { return (MS->fn_add_ammo_clips(result, params)); }

mcodeFunctionReturnCodes fn_use_ammo_clips(int32 &result, int32 *params) { return (MS->fn_use_ammo_clips(result, params)); }

mcodeFunctionReturnCodes fn_shutdown_inventory(int32 &result, int32 *params) { return (MS->fn_shutdown_inventory(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_is_carrying(int32 &result, int32 *params) {
	const char *item_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work.
	result = g_oIconListManager->Carrying(ICON_LIST_INVENTORY, item_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_carrying_how_many(int32 &result, int32 *params) {
	const char *item_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work.
	result = g_oIconListManager->CarryingHowMany(ICON_LIST_INVENTORY, item_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_item_held(int32 &result, int32 *) {
	// Call the function that does the work.
	result = g_oIconListManager->ItemHeld();

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_is_holding(int32 &result, int32 *params) {
	const char *item_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work.
	result = g_oIconListManager->Holding(item_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_drop(int32 &, int32 *) {
	// Call the function that does the work.
	g_oIconListManager->Drop();

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_add_inventory_item(int32 &, int32 *params) {
	char pcIconPath[ENGINE_STRING_LEN];

	const char *item_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work.
	g_oIconListManager->AddIconToList(ICON_LIST_INVENTORY, item_name);

	// Preload the icon for PSX smoothing.
	Common::sprintf_s(pcIconPath, ICON_PATH);
	g_oIconMenu->PreloadIcon(pcIconPath, item_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remove_inventory_item(int32 &, int32 *params) {
	const char *item_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the function that does the work.
	g_oIconListManager->RemoveIconFromList(ICON_LIST_INVENTORY, item_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_add_medipacks(int32 &result, int32 *params) {
	if (g_icb->getGameType() == GType_ELDORADO) {
		result = 0;
		return IR_CONT;
	}

	char pcIconPath[ENGINE_STRING_LEN];
	bool8 bFlashIcons;

	// can only carry 2
	if (g_mission->num_medi >= 2) {
		result = 1; // not given
		return IR_CONT;
	}

	// Type-safe convert the integer second parameter to a bool8.
	bFlashIcons = ((int32)params[1] == 0) ? FALSE8 : TRUE8;

	// Call the function that does the work.
	player.AddMediPacks(1, bFlashIcons);

	// Preload the icon for PSX smoothing.
	Common::sprintf_s(pcIconPath, ICON_PATH);
	g_oIconMenu->PreloadIcon(pcIconPath, ARMS_HEALTH_NAME);
	// Calling script can continue.

	result = 0; // given
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_use_medipacks(int32 &, int32 *params) {
	// Remove the medipacks.
	player.UseMediPacks((uint32)params[0]);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_add_ammo_clips(int32 &result, int32 *params) {
	if (g_icb->getGameType() == GType_ELDORADO) {
		return IR_CONT;
	}

	char pcIconPath[ENGINE_STRING_LEN];
	bool8 bFlashIcons;

	// Type-safe convert the integer second parameter to a bool8.
	bFlashIcons = ((int32)params[1] == 0) ? FALSE8 : TRUE8;

	// how many we got already?
	int32 clips = player.GetNoAmmoClips();

	// work out max we can carry minus what we have
	int32 can_take = player.GetMaxClips() - clips;

	if (can_take >= params[0]) { // we have room for all we're being given
		// Call the function that does the work.
		player.AddAmmoClips((uint32)params[0], bFlashIcons);

		result = 0;
	} else { // can't take all that were offered
		// Call the function that does the work.
		player.AddAmmoClips(can_take, bFlashIcons); // take max we can take

		result = params[0] - can_take; // leave behind those we can't take
	}

	// Preload the icon for PSX smoothing.
	Common::sprintf_s(pcIconPath, ICON_PATH);
	g_oIconMenu->PreloadIcon(pcIconPath, ARMS_AMMO_NAME);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_use_ammo_clips(int32 &, int32 *params) {
	// Remove the medipacks.
	player.UseAmmoClips((uint32)params[0]);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_add_icon_to_icon_list(int32 &, int32 *params) {
	char pcIconPath[ENGINE_STRING_LEN];

	const char *list_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Call the engine function that does the work.
	g_oIconListManager->AddIconToList(list_name, icon_name);

	// Preload the icon for PSX smoothing.
	Common::sprintf_s(pcIconPath, ICON_PATH);
	g_oIconMenu->PreloadIcon(pcIconPath, icon_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_remove_icon_from_icon_list(int32 &, int32 *params) {
	const char *list_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *icon_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	// Call the engine function that does the work.
	g_oIconListManager->RemoveIconFromList(list_name, icon_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_reset_icon_list(int32 &, int32 *params) {
	const char *list_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Make the call to the Remora object.
	g_oIconListManager->ResetList(list_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_destroy_icon_list(int32 &, int32 *params) {
	const char *list_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Call the engine function that does the work.
	g_oIconListManager->DestroyList(list_name);

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_interact_choose(int32 &, int32 *params) {
	_input *psInputState;

	const char *list_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Check to see if we are already looping
	if (!L->looping) {
		// There has to be a current interact object.
		if (!MS->player.Fetch_player_interact_status())
			Fatal_error("fn_interact_choose() can only be called from an interact script");

		// Mark the fact that we are now looping.
		L->looping = 1;

		// Bring up the icon menu.
		g_oIconListManager->ActivateIconMenu(list_name, TRUE8, FALSE8);
		MS->player.Push_control_mode(ACTOR_RELATIVE);

		// Must call this function again as long as this interface is active.
		return (IR_REPEAT);
	} else {
		// This is a subsequent call to this function.  Cycle the icon menu logic.
		MS->player.Update_input_state();
		psInputState = MS->player.Fetch_input_state();
		if (g_oIconMenu->CycleIconMenu(*psInputState)) {
			// Menu still active, so we must continue cycling its logic next time through.
			return (IR_REPEAT);
		} else {
			// A selection has been made or the menu has been cancelled.  Calling script can now continue.
			MS->player.Pop_control_mode();
			L->looping = 0;
			return (IR_CONT);
		}
	}

	// To fix a GCC compiler warning.
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_shutdown_inventory(int32 &, int32 *) {
	g_oIconMenu->CloseDownIconMenu();

	// Calling script can continue.
	return IR_CONT;
}

} // End of namespace ICB
