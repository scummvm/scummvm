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

#include "ags/shared/ac/dynobj/cc_hotspot.h"
#include "ags/shared/ac/hotspot.h"
#include "ags/shared/ac/gamestate.h"
#include "ags/shared/ac/global_hotspot.h"
#include "ags/shared/ac/global_translation.h"
#include "ags/shared/ac/properties.h"
#include "ags/shared/ac/room.h"
#include "ags/shared/ac/roomstatus.h"
#include "ags/shared/ac/string.h"
#include "ags/shared/game/roomstruct.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/script/runtimescriptvalue.h"

namespace AGS3 {

using namespace AGS::Shared;

extern RoomStruct thisroom;
extern RoomStatus *croom;
extern ScriptHotspot scrHotspot[MAX_ROOM_HOTSPOTS];
extern CCHotspot ccDynamicHotspot;

void Hotspot_SetEnabled(ScriptHotspot *hss, int newval) {
	if (newval)
		EnableHotspot(hss->id);
	else
		DisableHotspot(hss->id);
}

int Hotspot_GetEnabled(ScriptHotspot *hss) {
	return croom->hotspot_enabled[hss->id];
}

int Hotspot_GetID(ScriptHotspot *hss) {
	return hss->id;
}

int Hotspot_GetWalkToX(ScriptHotspot *hss) {
	return GetHotspotPointX(hss->id);
}

int Hotspot_GetWalkToY(ScriptHotspot *hss) {
	return GetHotspotPointY(hss->id);
}

ScriptHotspot *GetHotspotAtScreen(int xx, int yy) {
	return &scrHotspot[GetHotspotIDAtScreen(xx, yy)];
}

ScriptHotspot *GetHotspotAtRoom(int x, int y) {
	return &scrHotspot[get_hotspot_at(x, y)];
}

void Hotspot_GetName(ScriptHotspot *hss, char *buffer) {
	GetHotspotName(hss->id, buffer);
}

const char *Hotspot_GetName_New(ScriptHotspot *hss) {
	return CreateNewScriptString(get_translation(thisroom.Hotspots[hss->id].Name));
}

bool Hotspot_IsInteractionAvailable(ScriptHotspot *hhot, int mood) {

	play.check_interaction_only = 1;
	RunHotspotInteraction(hhot->id, mood);
	int ciwas = play.check_interaction_only;
	play.check_interaction_only = 0;
	return (ciwas == 2);
}

void Hotspot_RunInteraction(ScriptHotspot *hss, int mood) {
	RunHotspotInteraction(hss->id, mood);
}

int Hotspot_GetProperty(ScriptHotspot *hss, const char *property) {
	return get_int_property(thisroom.Hotspots[hss->id].Properties, croom->hsProps[hss->id], property);
}

void Hotspot_GetPropertyText(ScriptHotspot *hss, const char *property, char *bufer) {
	get_text_property(thisroom.Hotspots[hss->id].Properties, croom->hsProps[hss->id], property, bufer);

}

const char *Hotspot_GetTextProperty(ScriptHotspot *hss, const char *property) {
	return get_text_property_dynamic_string(thisroom.Hotspots[hss->id].Properties, croom->hsProps[hss->id], property);
}

bool Hotspot_SetProperty(ScriptHotspot *hss, const char *property, int value) {
	return set_int_property(croom->hsProps[hss->id], property, value);
}

bool Hotspot_SetTextProperty(ScriptHotspot *hss, const char *property, const char *value) {
	return set_text_property(croom->hsProps[hss->id], property, value);
}

int get_hotspot_at(int xpp, int ypp) {
	int onhs = thisroom.HotspotMask->GetPixel(room_to_mask_coord(xpp), room_to_mask_coord(ypp));
	if (onhs <= 0 || onhs >= MAX_ROOM_HOTSPOTS) return 0;
	if (croom->hotspot_enabled[onhs] == 0) return 0;
	return onhs;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

#include "ags/shared/debug/out.h"
#include "ags/shared/script/script_api.h"
#include "ags/shared/script/script_runtime.h"
#include "ags/shared/ac/dynobj/scriptstring.h"

extern ScriptString myScriptStringImpl;

RuntimeScriptValue Sc_GetHotspotAtRoom(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(ScriptHotspot, ccDynamicHotspot, GetHotspotAtRoom);
}

// ScriptHotspot *(int xx, int yy)
RuntimeScriptValue Sc_GetHotspotAtScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(ScriptHotspot, ccDynamicHotspot, GetHotspotAtScreen);
}

RuntimeScriptValue Sc_Hotspot_GetDrawingSurface(const RuntimeScriptValue *params, int32_t param_count) {
	ScriptDrawingSurface *ret_obj = Room_GetDrawingSurfaceForMask(kRoomAreaHotspot);
	return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj);
}

// void (ScriptHotspot *hss, char *buffer)
RuntimeScriptValue Sc_Hotspot_GetName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(ScriptHotspot, Hotspot_GetName, char);
}

// int  (ScriptHotspot *hss, const char *property)
RuntimeScriptValue Sc_Hotspot_GetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(ScriptHotspot, Hotspot_GetProperty, const char);
}

// void  (ScriptHotspot *hss, const char *property, char *bufer)
RuntimeScriptValue Sc_Hotspot_GetPropertyText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ2(ScriptHotspot, Hotspot_GetPropertyText, const char, char);
}

// const char* (ScriptHotspot *hss, const char *property)
RuntimeScriptValue Sc_Hotspot_GetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_POBJ(ScriptHotspot, const char, myScriptStringImpl, Hotspot_GetTextProperty, const char);
}

RuntimeScriptValue Sc_Hotspot_SetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ_PINT(ScriptHotspot, Hotspot_SetProperty, const char);
}

RuntimeScriptValue Sc_Hotspot_SetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ2(ScriptHotspot, Hotspot_SetTextProperty, const char, const char);
}

RuntimeScriptValue Sc_Hotspot_IsInteractionAvailable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_PINT(ScriptHotspot, Hotspot_IsInteractionAvailable);
}

// void  (ScriptHotspot *hss, int mood)
RuntimeScriptValue Sc_Hotspot_RunInteraction(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptHotspot, Hotspot_RunInteraction);
}

// int (ScriptHotspot *hss)
RuntimeScriptValue Sc_Hotspot_GetEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptHotspot, Hotspot_GetEnabled);
}

// void (ScriptHotspot *hss, int newval)
RuntimeScriptValue Sc_Hotspot_SetEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptHotspot, Hotspot_SetEnabled);
}

// int (ScriptHotspot *hss)
RuntimeScriptValue Sc_Hotspot_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptHotspot, Hotspot_GetID);
}

// const char* (ScriptHotspot *hss)
RuntimeScriptValue Sc_Hotspot_GetName_New(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptHotspot, const char, myScriptStringImpl, Hotspot_GetName_New);
}

// int (ScriptHotspot *hss)
RuntimeScriptValue Sc_Hotspot_GetWalkToX(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptHotspot, Hotspot_GetWalkToX);
}

// int (ScriptHotspot *hss)
RuntimeScriptValue Sc_Hotspot_GetWalkToY(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptHotspot, Hotspot_GetWalkToY);
}



void RegisterHotspotAPI() {
	ccAddExternalStaticFunction("Hotspot::GetAtRoomXY^2", Sc_GetHotspotAtRoom);
	ccAddExternalStaticFunction("Hotspot::GetAtScreenXY^2", Sc_GetHotspotAtScreen);
	ccAddExternalStaticFunction("Hotspot::GetDrawingSurface", Sc_Hotspot_GetDrawingSurface);
	ccAddExternalObjectFunction("Hotspot::GetName^1", Sc_Hotspot_GetName);
	ccAddExternalObjectFunction("Hotspot::GetProperty^1", Sc_Hotspot_GetProperty);
	ccAddExternalObjectFunction("Hotspot::GetPropertyText^2", Sc_Hotspot_GetPropertyText);
	ccAddExternalObjectFunction("Hotspot::GetTextProperty^1", Sc_Hotspot_GetTextProperty);
	ccAddExternalObjectFunction("Hotspot::SetProperty^2", Sc_Hotspot_SetProperty);
	ccAddExternalObjectFunction("Hotspot::SetTextProperty^2", Sc_Hotspot_SetTextProperty);
	ccAddExternalObjectFunction("Hotspot::IsInteractionAvailable^1", Sc_Hotspot_IsInteractionAvailable);
	ccAddExternalObjectFunction("Hotspot::RunInteraction^1", Sc_Hotspot_RunInteraction);
	ccAddExternalObjectFunction("Hotspot::get_Enabled", Sc_Hotspot_GetEnabled);
	ccAddExternalObjectFunction("Hotspot::set_Enabled", Sc_Hotspot_SetEnabled);
	ccAddExternalObjectFunction("Hotspot::get_ID", Sc_Hotspot_GetID);
	ccAddExternalObjectFunction("Hotspot::get_Name", Sc_Hotspot_GetName_New);
	ccAddExternalObjectFunction("Hotspot::get_WalkToX", Sc_Hotspot_GetWalkToX);
	ccAddExternalObjectFunction("Hotspot::get_WalkToY", Sc_Hotspot_GetWalkToY);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("Hotspot::GetAtRoomXY^2", (void *)GetHotspotAtRoom);
	ccAddExternalFunctionForPlugin("Hotspot::GetAtScreenXY^2", (void *)GetHotspotAtScreen);
	ccAddExternalFunctionForPlugin("Hotspot::GetName^1", (void *)Hotspot_GetName);
	ccAddExternalFunctionForPlugin("Hotspot::GetProperty^1", (void *)Hotspot_GetProperty);
	ccAddExternalFunctionForPlugin("Hotspot::GetPropertyText^2", (void *)Hotspot_GetPropertyText);
	ccAddExternalFunctionForPlugin("Hotspot::GetTextProperty^1", (void *)Hotspot_GetTextProperty);
	ccAddExternalFunctionForPlugin("Hotspot::RunInteraction^1", (void *)Hotspot_RunInteraction);
	ccAddExternalFunctionForPlugin("Hotspot::get_Enabled", (void *)Hotspot_GetEnabled);
	ccAddExternalFunctionForPlugin("Hotspot::set_Enabled", (void *)Hotspot_SetEnabled);
	ccAddExternalFunctionForPlugin("Hotspot::get_ID", (void *)Hotspot_GetID);
	ccAddExternalFunctionForPlugin("Hotspot::get_Name", (void *)Hotspot_GetName_New);
	ccAddExternalFunctionForPlugin("Hotspot::get_WalkToX", (void *)Hotspot_GetWalkToX);
	ccAddExternalFunctionForPlugin("Hotspot::get_WalkToY", (void *)Hotspot_GetWalkToY);
}

} // namespace AGS3
