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

#include "ags/engine/ac/inventoryitem.h"
#include "ags/shared/ac/characterinfo.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_inventoryitem.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/script/runtimescriptvalue.h"
#include "ags/engine/ac/dynobj/cc_inventory.h"

#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/scriptstring.h"

namespace AGS3 {

extern GameSetupStruct game;
extern ScriptInvItem scrInv[MAX_INV];
extern int cur_cursor;
extern CharacterInfo *playerchar;
extern CCInventory ccDynamicInv;


void InventoryItem_SetCursorGraphic(ScriptInvItem *iitem, int newSprite) {
	set_inv_item_cursorpic(iitem->id, newSprite);
}

int InventoryItem_GetCursorGraphic(ScriptInvItem *iitem) {
	return game.invinfo[iitem->id].cursorPic;
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

ScriptInvItem *GetInvAtLocation(int xx, int yy) {
	int hsnum = GetInvAt(xx, yy);
	if (hsnum <= 0)
		return nullptr;
	return &scrInv[hsnum];
}

void InventoryItem_GetName(ScriptInvItem *iitem, char *buff) {
	GetInvName(iitem->id, buff);
}

const char *InventoryItem_GetName_New(ScriptInvItem *invitem) {
	return CreateNewScriptString(get_translation(game.invinfo[invitem->id].name));
}

int InventoryItem_GetGraphic(ScriptInvItem *iitem) {
	return game.invinfo[iitem->id].pic;
}

void InventoryItem_RunInteraction(ScriptInvItem *iitem, int mood) {
	RunInventoryInteraction(iitem->id, mood);
}

int InventoryItem_CheckInteractionAvailable(ScriptInvItem *iitem, int mood) {
	return IsInventoryInteractionAvailable(iitem->id, mood);
}

int InventoryItem_GetProperty(ScriptInvItem *scii, const char *property) {
	return get_int_property(game.invProps[scii->id], play.invProps[scii->id], property);
}

void InventoryItem_GetPropertyText(ScriptInvItem *scii, const char *property, char *bufer) {
	get_text_property(game.invProps[scii->id], play.invProps[scii->id], property, bufer);
}

const char *InventoryItem_GetTextProperty(ScriptInvItem *scii, const char *property) {
	return get_text_property_dynamic_string(game.invProps[scii->id], play.invProps[scii->id], property);
}

bool InventoryItem_SetProperty(ScriptInvItem *scii, const char *property, int value) {
	return set_int_property(play.invProps[scii->id], property, value);
}

bool InventoryItem_SetTextProperty(ScriptInvItem *scii, const char *property, const char *value) {
	return set_text_property(play.invProps[scii->id], property, value);
}

//=============================================================================

void set_inv_item_cursorpic(int invItemId, int piccy) {
	game.invinfo[invItemId].cursorPic = piccy;

	if ((cur_cursor == MODE_USE) && (playerchar->activeinv == invItemId)) {
		update_inv_cursor(invItemId);
		set_mouse_cursor(cur_cursor);
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

extern ScriptString myScriptStringImpl;

// ScriptInvItem *(int xx, int yy)
RuntimeScriptValue Sc_GetInvAtLocation(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(ScriptInvItem, ccDynamicInv, GetInvAtLocation);
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
	API_CONST_OBJCALL_OBJ_POBJ(ScriptInvItem, const char, myScriptStringImpl, InventoryItem_GetTextProperty, const char);
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

// const char* (ScriptInvItem *invitem)
RuntimeScriptValue Sc_InventoryItem_GetName_New(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ(ScriptInvItem, const char, myScriptStringImpl, InventoryItem_GetName_New);
}



void RegisterInventoryItemAPI() {
	ccAddExternalStaticFunction("InventoryItem::GetAtScreenXY^2", Sc_GetInvAtLocation);
	ccAddExternalObjectFunction("InventoryItem::IsInteractionAvailable^1", Sc_InventoryItem_CheckInteractionAvailable);
	ccAddExternalObjectFunction("InventoryItem::GetName^1", Sc_InventoryItem_GetName);
	ccAddExternalObjectFunction("InventoryItem::GetProperty^1", Sc_InventoryItem_GetProperty);
	ccAddExternalObjectFunction("InventoryItem::GetPropertyText^2", Sc_InventoryItem_GetPropertyText);
	ccAddExternalObjectFunction("InventoryItem::GetTextProperty^1", Sc_InventoryItem_GetTextProperty);
	ccAddExternalObjectFunction("InventoryItem::SetProperty^2", Sc_InventoryItem_SetProperty);
	ccAddExternalObjectFunction("InventoryItem::SetTextProperty^2", Sc_InventoryItem_SetTextProperty);
	ccAddExternalObjectFunction("InventoryItem::RunInteraction^1", Sc_InventoryItem_RunInteraction);
	ccAddExternalObjectFunction("InventoryItem::SetName^1", Sc_InventoryItem_SetName);
	ccAddExternalObjectFunction("InventoryItem::get_CursorGraphic", Sc_InventoryItem_GetCursorGraphic);
	ccAddExternalObjectFunction("InventoryItem::set_CursorGraphic", Sc_InventoryItem_SetCursorGraphic);
	ccAddExternalObjectFunction("InventoryItem::get_Graphic", Sc_InventoryItem_GetGraphic);
	ccAddExternalObjectFunction("InventoryItem::set_Graphic", Sc_InventoryItem_SetGraphic);
	ccAddExternalObjectFunction("InventoryItem::get_ID", Sc_InventoryItem_GetID);
	ccAddExternalObjectFunction("InventoryItem::get_Name", Sc_InventoryItem_GetName_New);
	ccAddExternalObjectFunction("InventoryItem::set_Name", Sc_InventoryItem_SetName);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("InventoryItem::GetAtScreenXY^2", (void *)GetInvAtLocation);
	ccAddExternalFunctionForPlugin("InventoryItem::IsInteractionAvailable^1", (void *)InventoryItem_CheckInteractionAvailable);
	ccAddExternalFunctionForPlugin("InventoryItem::GetName^1", (void *)InventoryItem_GetName);
	ccAddExternalFunctionForPlugin("InventoryItem::GetProperty^1", (void *)InventoryItem_GetProperty);
	ccAddExternalFunctionForPlugin("InventoryItem::GetPropertyText^2", (void *)InventoryItem_GetPropertyText);
	ccAddExternalFunctionForPlugin("InventoryItem::GetTextProperty^1", (void *)InventoryItem_GetTextProperty);
	ccAddExternalFunctionForPlugin("InventoryItem::RunInteraction^1", (void *)InventoryItem_RunInteraction);
	ccAddExternalFunctionForPlugin("InventoryItem::SetName^1", (void *)InventoryItem_SetName);
	ccAddExternalFunctionForPlugin("InventoryItem::get_CursorGraphic", (void *)InventoryItem_GetCursorGraphic);
	ccAddExternalFunctionForPlugin("InventoryItem::set_CursorGraphic", (void *)InventoryItem_SetCursorGraphic);
	ccAddExternalFunctionForPlugin("InventoryItem::get_Graphic", (void *)InventoryItem_GetGraphic);
	ccAddExternalFunctionForPlugin("InventoryItem::set_Graphic", (void *)InventoryItem_SetGraphic);
	ccAddExternalFunctionForPlugin("InventoryItem::get_ID", (void *)InventoryItem_GetID);
	ccAddExternalFunctionForPlugin("InventoryItem::get_Name", (void *)InventoryItem_GetName_New);
	ccAddExternalFunctionForPlugin("InventoryItem::set_Name", (void *)InventoryItem_SetName);
}

} // namespace AGS3
