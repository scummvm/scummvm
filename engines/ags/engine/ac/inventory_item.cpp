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

#include "ags/engine/ac/inventory_item.h"
#include "ags/shared/ac/character_info.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_inventory_item.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/ac/dynobj/cc_inventory.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"

namespace AGS3 {

void InventoryItem_SetCursorGraphic(ScriptInvItem *iitem, int newSprite) {
	set_inv_item_cursorpic(iitem->id, newSprite);
}

int InventoryItem_GetCursorGraphic(ScriptInvItem *iitem) {
	return _GP(game).invinfo[iitem->id].cursorPic;
}

void InventoryItem_SetGraphic(ScriptInvItem *iitem, int piccy) {
	set_inv_item_pic(iitem->id, piccy);
}

void InventoryItem_SetName(ScriptInvItem *scii, const char *newname) {
	SetInvItemName(scii->id, newname);
}

int InventoryItem_GetID(ScriptInvItem *scii) {
	return scii->id;
}

const char *InventoryItem_GetScriptName(ScriptInvItem *scii) {
	return CreateNewScriptString(_GP(game).invScriptNames[scii->id]);
}

ScriptInvItem *GetInvAtLocation(int xx, int yy) {
	int hsnum = GetInvAt(xx, yy);
	if (hsnum <= 0)
		return nullptr;
	return &_G(scrInv)[hsnum];
}

void InventoryItem_GetName(ScriptInvItem *iitem, char *buff) {
	GetInvName(iitem->id, buff);
}

const char *InventoryItem_GetName_New(ScriptInvItem *invitem) {
	return CreateNewScriptString(get_translation(_GP(game).invinfo[invitem->id].name.GetCStr()));
}

int InventoryItem_GetGraphic(ScriptInvItem *iitem) {
	return _GP(game).invinfo[iitem->id].pic;
}

void InventoryItem_RunInteraction(ScriptInvItem *iitem, int mood) {
	RunInventoryInteraction(iitem->id, mood);
}

int InventoryItem_CheckInteractionAvailable(ScriptInvItem *iitem, int mood) {
	return IsInventoryInteractionAvailable(iitem->id, mood);
}

int InventoryItem_GetProperty(ScriptInvItem *scii, const char *property) {
	return get_int_property(_GP(game).invProps[scii->id], _GP(play).invProps[scii->id], property);
}

void InventoryItem_GetPropertyText(ScriptInvItem *scii, const char *property, char *bufer) {
	get_text_property(_GP(game).invProps[scii->id], _GP(play).invProps[scii->id], property, bufer);
}

const char *InventoryItem_GetTextProperty(ScriptInvItem *scii, const char *property) {
	return get_text_property_dynamic_string(_GP(game).invProps[scii->id], _GP(play).invProps[scii->id], property);
}

bool InventoryItem_SetProperty(ScriptInvItem *scii, const char *property, int value) {
	return set_int_property(_GP(play).invProps[scii->id], property, value);
}

bool InventoryItem_SetTextProperty(ScriptInvItem *scii, const char *property, const char *value) {
	return set_text_property(_GP(play).invProps[scii->id], property, value);
}

//=============================================================================

void set_inv_item_cursorpic(int invItemId, int piccy) {
	_GP(game).invinfo[invItemId].cursorPic = piccy;

	if ((_G(cur_cursor) == MODE_USE) && (_G(playerchar)->activeinv == invItemId)) {
		update_inv_cursor(invItemId);
		set_mouse_cursor(_G(cur_cursor));
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

ScriptInvItem *InventoryItem_GetByName(const char *name) {
	return static_cast<ScriptInvItem *>(ccGetScriptObjectAddress(name, _GP(ccDynamicInv).GetType()));
}

RuntimeScriptValue Sc_InventoryItem_GetByName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_POBJ(ScriptInvItem, _GP(ccDynamicInv), InventoryItem_GetByName, const char);
}

// ScriptInvItem *(int xx, int yy)
RuntimeScriptValue Sc_GetInvAtLocation(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(ScriptInvItem, _GP(ccDynamicInv), GetInvAtLocation);
}

// int (ScriptInvItem *iitem, int mood)
RuntimeScriptValue Sc_InventoryItem_CheckInteractionAvailable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT(ScriptInvItem, InventoryItem_CheckInteractionAvailable);
}

// void (ScriptInvItem *iitem, char *buff)
RuntimeScriptValue Sc_InventoryItem_GetName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(ScriptInvItem, InventoryItem_GetName, char);
}

// int (ScriptInvItem *scii, const char *property)
RuntimeScriptValue Sc_InventoryItem_GetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(ScriptInvItem, InventoryItem_GetProperty, const char);
}

// void (ScriptInvItem *scii, const char *property, char *bufer)
RuntimeScriptValue Sc_InventoryItem_GetPropertyText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ2(ScriptInvItem, InventoryItem_GetPropertyText, const char, char);
}

// const char* (ScriptInvItem *scii, const char *property)
RuntimeScriptValue Sc_InventoryItem_GetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_POBJ(ScriptInvItem, const char, _GP(myScriptStringImpl), InventoryItem_GetTextProperty, const char);
}

RuntimeScriptValue Sc_InventoryItem_SetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ_PINT(ScriptInvItem, InventoryItem_SetProperty, const char);
}

RuntimeScriptValue Sc_InventoryItem_SetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ2(ScriptInvItem, InventoryItem_SetTextProperty, const char, const char);
}

// void (ScriptInvItem *iitem, int mood)
RuntimeScriptValue Sc_InventoryItem_RunInteraction(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptInvItem, InventoryItem_RunInteraction);
}

// void (ScriptInvItem *scii, const char *newname)
RuntimeScriptValue Sc_InventoryItem_SetName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(ScriptInvItem, InventoryItem_SetName, const char);
}

// int (ScriptInvItem *iitem)
RuntimeScriptValue Sc_InventoryItem_GetCursorGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptInvItem, InventoryItem_GetCursorGraphic);
}

// void (ScriptInvItem *iitem, int newSprite)
RuntimeScriptValue Sc_InventoryItem_SetCursorGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptInvItem, InventoryItem_SetCursorGraphic);
}

// int (ScriptInvItem *iitem)
RuntimeScriptValue Sc_InventoryItem_GetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptInvItem, InventoryItem_GetGraphic);
}

// void (ScriptInvItem *iitem, int piccy)
RuntimeScriptValue Sc_InventoryItem_SetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptInvItem, InventoryItem_SetGraphic);
}

// int (ScriptInvItem *scii)
RuntimeScriptValue Sc_InventoryItem_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptInvItem, InventoryItem_GetID);
}

RuntimeScriptValue Sc_InventoryItem_GetScriptName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptInvItem, const char, _GP(myScriptStringImpl), InventoryItem_GetScriptName);
}

// const char* (ScriptInvItem *invitem)
RuntimeScriptValue Sc_InventoryItem_GetName_New(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptInvItem, const char, _GP(myScriptStringImpl), InventoryItem_GetName_New);
}

void RegisterInventoryItemAPI() {
	ScFnRegister invitem_api[] = {
		{"InventoryItem::GetAtScreenXY^2", API_FN_PAIR(GetInvAtLocation)},
		{"InventoryItem::GetByName", API_FN_PAIR(InventoryItem_GetByName)},

		{"InventoryItem::IsInteractionAvailable^1", API_FN_PAIR(InventoryItem_CheckInteractionAvailable)},
		{"InventoryItem::GetName^1", API_FN_PAIR(InventoryItem_GetName)},
		{"InventoryItem::GetProperty^1", API_FN_PAIR(InventoryItem_GetProperty)},
		{"InventoryItem::GetPropertyText^2", API_FN_PAIR(InventoryItem_GetPropertyText)},
		{"InventoryItem::GetTextProperty^1", API_FN_PAIR(InventoryItem_GetTextProperty)},
		{"InventoryItem::SetProperty^2", API_FN_PAIR(InventoryItem_SetProperty)},
		{"InventoryItem::SetTextProperty^2", API_FN_PAIR(InventoryItem_SetTextProperty)},
		{"InventoryItem::RunInteraction^1", API_FN_PAIR(InventoryItem_RunInteraction)},
		{"InventoryItem::SetName^1", API_FN_PAIR(InventoryItem_SetName)},
		{"InventoryItem::get_CursorGraphic", API_FN_PAIR(InventoryItem_GetCursorGraphic)},
		{"InventoryItem::set_CursorGraphic", API_FN_PAIR(InventoryItem_SetCursorGraphic)},
		{"InventoryItem::get_Graphic", API_FN_PAIR(InventoryItem_GetGraphic)},
		{"InventoryItem::set_Graphic", API_FN_PAIR(InventoryItem_SetGraphic)},
		{"InventoryItem::get_ID", API_FN_PAIR(InventoryItem_GetID)},
		{"InventoryItem::get_Name", API_FN_PAIR(InventoryItem_GetName_New)},
		{"InventoryItem::set_Name", API_FN_PAIR(InventoryItem_SetName)},
		{"InventoryItem::get_ScriptName", API_FN_PAIR(InventoryItem_GetScriptName)},
	};

	ccAddExternalFunctions361(invitem_api);
}

} // namespace AGS3
