//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "ac/region.h"
#include "ac/common_defines.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_region.h"
#include "ac/room.h"
#include "ac/roomstatus.h"
#include "ac/dynobj/cc_region.h"
#include "ac/dynobj/scriptdrawingsurface.h"
#include "game/roomstruct.h"
#include "script/runtimescriptvalue.h"

using namespace AGS::Common;

extern ScriptRegion scrRegion[MAX_ROOM_REGIONS];
extern RoomStruct thisroom;
extern RoomStatus*croom;
extern GameSetupStruct game;
extern COLOR_MAP maincoltable;
extern color palette[256];
extern CCRegion ccDynamicRegion;


ScriptRegion *GetRegionAtRoom(int xx, int yy) {
    return &scrRegion[GetRegionIDAtRoom(xx, yy)];
}

ScriptRegion *GetRegionAtScreen(int x, int y)
{
    VpPoint vpt = play.ScreenToRoomDivDown(x, y);
    if (vpt.second < 0)
        return nullptr;
    return GetRegionAtRoom(vpt.first.X, vpt.first.Y);
}

void Region_SetLightLevel(ScriptRegion *ssr, int brightness) {
    SetAreaLightLevel(ssr->id, brightness);
}

int Region_GetLightLevel(ScriptRegion *ssr) {
    return thisroom.GetRegionLightLevel(ssr->id);
}

int Region_GetTintEnabled(ScriptRegion *srr) {
    if (thisroom.Regions[srr->id].Tint & 0xFF000000)
        return 1;
    return 0;
}

int Region_GetTintRed(ScriptRegion *srr) {

    return thisroom.Regions[srr->id].Tint & 0x000000ff;
}

int Region_GetTintGreen(ScriptRegion *srr) {

    return (thisroom.Regions[srr->id].Tint >> 8) & 0x000000ff;
}

int Region_GetTintBlue(ScriptRegion *srr) {

    return (thisroom.Regions[srr->id].Tint >> 16) & 0x000000ff;
}

int Region_GetTintSaturation(ScriptRegion *srr) {

    return (thisroom.Regions[srr->id].Tint >> 24) & 0xFF;
}

int Region_GetTintLuminance(ScriptRegion *srr)
{
    return thisroom.GetRegionTintLuminance(srr->id);
}

void Region_Tint(ScriptRegion *srr, int red, int green, int blue, int amount, int luminance)
{
    SetRegionTint(srr->id, red, green, blue, amount, luminance);
}

void Region_TintNoLum(ScriptRegion *srr, int red, int green, int blue, int amount)
{
    SetRegionTint(srr->id, red, green, blue, amount);
}

void Region_SetEnabled(ScriptRegion *ssr, int enable) {
    if (enable)
        EnableRegion(ssr->id);
    else
        DisableRegion(ssr->id);
}

int Region_GetEnabled(ScriptRegion *ssr) {
    return croom->region_enabled[ssr->id];
}

int Region_GetID(ScriptRegion *ssr) {
    return ssr->id;
}

void Region_RunInteraction(ScriptRegion *ssr, int mood) {
    RunRegionInteraction(ssr->id, mood);
}

//=============================================================================

void generate_light_table()
{
    if (game.color_depth == 1 && color_map == nullptr)
    {
        create_light_table(&maincoltable, palette, 0, 0, 0, nullptr);
        color_map = &maincoltable;
    }
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

#include "debug/out.h"
#include "script/script_api.h"
#include "script/script_runtime.h"

// ScriptRegion *(int xx, int yy)
RuntimeScriptValue Sc_GetRegionAtRoom(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJ_PINT2(ScriptRegion, ccDynamicRegion, GetRegionAtRoom);
}

RuntimeScriptValue Sc_GetRegionAtScreen(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJ_PINT2(ScriptRegion, ccDynamicRegion, GetRegionAtScreen);
}

RuntimeScriptValue Sc_Region_GetDrawingSurface(const RuntimeScriptValue *params, int32_t param_count)
{
    ScriptDrawingSurface* ret_obj = Room_GetDrawingSurfaceForMask(kRoomAreaRegion);
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj);
}

RuntimeScriptValue Sc_Region_Tint(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT5(ScriptRegion, Region_Tint);
}

// void (ScriptRegion *srr, int red, int green, int blue, int amount)
RuntimeScriptValue Sc_Region_TintNoLum(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(ScriptRegion, Region_TintNoLum);
}

// void (ScriptRegion *ssr, int mood)
RuntimeScriptValue Sc_Region_RunInteraction(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptRegion, Region_RunInteraction);
}

// int (ScriptRegion *ssr)
RuntimeScriptValue Sc_Region_GetEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptRegion, Region_GetEnabled);
}

// void (ScriptRegion *ssr, int enable)
RuntimeScriptValue Sc_Region_SetEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptRegion, Region_SetEnabled);
}

// int (ScriptRegion *ssr)
RuntimeScriptValue Sc_Region_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptRegion, Region_GetID);
}

// int (ScriptRegion *ssr)
RuntimeScriptValue Sc_Region_GetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptRegion, Region_GetLightLevel);
}

// void (ScriptRegion *ssr, int brightness)
RuntimeScriptValue Sc_Region_SetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptRegion, Region_SetLightLevel);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptRegion, Region_GetTintEnabled);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintBlue(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptRegion, Region_GetTintBlue);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintGreen(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptRegion, Region_GetTintGreen);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintRed(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptRegion, Region_GetTintRed);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintSaturation(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptRegion, Region_GetTintSaturation);
}

RuntimeScriptValue Sc_Region_GetTintLuminance(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptRegion, Region_GetTintLuminance);
}



void RegisterRegionAPI()
{
    ccAddExternalStaticFunction("Region::GetAtRoomXY^2",        Sc_GetRegionAtRoom);
    ccAddExternalStaticFunction("Region::GetAtScreenXY^2",      Sc_GetRegionAtScreen);
    ccAddExternalStaticFunction("Region::GetDrawingSurface",    Sc_Region_GetDrawingSurface);
    ccAddExternalObjectFunction("Region::Tint^4",               Sc_Region_TintNoLum);
    ccAddExternalObjectFunction("Region::Tint^5",               Sc_Region_Tint);
    ccAddExternalObjectFunction("Region::RunInteraction^1",     Sc_Region_RunInteraction);
    ccAddExternalObjectFunction("Region::get_Enabled",          Sc_Region_GetEnabled);
    ccAddExternalObjectFunction("Region::set_Enabled",          Sc_Region_SetEnabled);
    ccAddExternalObjectFunction("Region::get_ID",               Sc_Region_GetID);
    ccAddExternalObjectFunction("Region::get_LightLevel",       Sc_Region_GetLightLevel);
    ccAddExternalObjectFunction("Region::set_LightLevel",       Sc_Region_SetLightLevel);
    ccAddExternalObjectFunction("Region::get_TintEnabled",      Sc_Region_GetTintEnabled);
    ccAddExternalObjectFunction("Region::get_TintBlue",         Sc_Region_GetTintBlue);
    ccAddExternalObjectFunction("Region::get_TintGreen",        Sc_Region_GetTintGreen);
    ccAddExternalObjectFunction("Region::get_TintRed",          Sc_Region_GetTintRed);
    ccAddExternalObjectFunction("Region::get_TintSaturation",   Sc_Region_GetTintSaturation);
    ccAddExternalObjectFunction("Region::get_TintLuminance",    Sc_Region_GetTintLuminance);

    /* ----------------------- Registering unsafe exports for plugins -----------------------*/

    ccAddExternalFunctionForPlugin("Region::GetAtRoomXY^2",        (void*)GetRegionAtRoom);
    ccAddExternalFunctionForPlugin("Region::GetAtScreenXY^2",      (void*)GetRegionAtScreen);
    ccAddExternalFunctionForPlugin("Region::Tint^4",               (void*)Region_TintNoLum);
    ccAddExternalFunctionForPlugin("Region::RunInteraction^1",     (void*)Region_RunInteraction);
    ccAddExternalFunctionForPlugin("Region::get_Enabled",          (void*)Region_GetEnabled);
    ccAddExternalFunctionForPlugin("Region::set_Enabled",          (void*)Region_SetEnabled);
    ccAddExternalFunctionForPlugin("Region::get_ID",               (void*)Region_GetID);
    ccAddExternalFunctionForPlugin("Region::get_LightLevel",       (void*)Region_GetLightLevel);
    ccAddExternalFunctionForPlugin("Region::set_LightLevel",       (void*)Region_SetLightLevel);
    ccAddExternalFunctionForPlugin("Region::get_TintEnabled",      (void*)Region_GetTintEnabled);
    ccAddExternalFunctionForPlugin("Region::get_TintBlue",         (void*)Region_GetTintBlue);
    ccAddExternalFunctionForPlugin("Region::get_TintGreen",        (void*)Region_GetTintGreen);
    ccAddExternalFunctionForPlugin("Region::get_TintRed",          (void*)Region_GetTintRed);
    ccAddExternalFunctionForPlugin("Region::get_TintSaturation",   (void*)Region_GetTintSaturation);
}
