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

#include "ags/engine/ac/region.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_region.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/dynobj/cc_region.h"
#include "ags/engine/ac/dynobj/script_drawing_surface.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

ScriptRegion *GetRegionAtRoom(int xx, int yy) {
	return &_G(scrRegion)[GetRegionIDAtRoom(xx, yy)];
}

ScriptRegion *GetRegionAtScreen(int x, int y) {
	VpPoint vpt = _GP(play).ScreenToRoomDivDown(x, y);
	if (vpt.second < 0)
		return &_G(scrRegion)[0]; // return region[0] for consistency and backwards compatibility
	return GetRegionAtRoom(vpt.first.X, vpt.first.Y);
}

void Region_SetLightLevel(ScriptRegion *ssr, int brightness) {
	SetAreaLightLevel(ssr->id, brightness);
}

int Region_GetLightLevel(ScriptRegion *ssr) {
	return _GP(thisroom).GetRegionLightLevel(ssr->id);
}

int Region_GetTintEnabled(ScriptRegion *srr) {
	if (_GP(thisroom).Regions[srr->id].Tint & 0xFF000000)
		return 1;
	return 0;
}

int Region_GetTintRed(ScriptRegion *srr) {

	return _GP(thisroom).Regions[srr->id].Tint & 0x000000ff;
}

int Region_GetTintGreen(ScriptRegion *srr) {

	return (_GP(thisroom).Regions[srr->id].Tint >> 8) & 0x000000ff;
}

int Region_GetTintBlue(ScriptRegion *srr) {

	return (_GP(thisroom).Regions[srr->id].Tint >> 16) & 0x000000ff;
}

int Region_GetTintSaturation(ScriptRegion *srr) {

	return (_GP(thisroom).Regions[srr->id].Tint >> 24) & 0xFF;
}

int Region_GetTintLuminance(ScriptRegion *srr) {
	return _GP(thisroom).GetRegionTintLuminance(srr->id);
}

void Region_Tint(ScriptRegion *srr, int red, int green, int blue, int amount, int luminance) {
	SetRegionTint(srr->id, red, green, blue, amount, luminance);
}

void Region_TintNoLum(ScriptRegion *srr, int red, int green, int blue, int amount) {
	SetRegionTint(srr->id, red, green, blue, amount);
}

void Region_SetEnabled(ScriptRegion *ssr, int enable) {
	if (enable)
		EnableRegion(ssr->id);
	else
		DisableRegion(ssr->id);
}

int Region_GetEnabled(ScriptRegion *ssr) {
	return _G(croom)->region_enabled[ssr->id];
}

int Region_GetID(ScriptRegion *ssr) {
	return ssr->id;
}

void Region_RunInteraction(ScriptRegion *ssr, int mood) {
	RunRegionInteraction(ssr->id, mood);
}

//=============================================================================

void generate_light_table() {
	if (_GP(game).color_depth == 1 && _G(color_map) == nullptr) {
		create_light_table(&_GP(maincoltable), _G(palette), 0, 0, 0, nullptr);
		_G(color_map) = &_GP(maincoltable);
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// ScriptRegion *(int xx, int yy)
RuntimeScriptValue Sc_GetRegionAtRoom(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(ScriptRegion, _GP(ccDynamicRegion), GetRegionAtRoom);
}

RuntimeScriptValue Sc_GetRegionAtScreen(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT2(ScriptRegion, _GP(ccDynamicRegion), GetRegionAtScreen);
}

RuntimeScriptValue Sc_Region_GetDrawingSurface(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptDrawingSurface, Region_GetDrawingSurface);
}

RuntimeScriptValue Sc_Region_Tint(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT5(ScriptRegion, Region_Tint);
}

// void (ScriptRegion *srr, int red, int green, int blue, int amount)
RuntimeScriptValue Sc_Region_TintNoLum(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(ScriptRegion, Region_TintNoLum);
}

// void (ScriptRegion *ssr, int mood)
RuntimeScriptValue Sc_Region_RunInteraction(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptRegion, Region_RunInteraction);
}

// int (ScriptRegion *ssr)
RuntimeScriptValue Sc_Region_GetEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptRegion, Region_GetEnabled);
}

// void (ScriptRegion *ssr, int enable)
RuntimeScriptValue Sc_Region_SetEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptRegion, Region_SetEnabled);
}

// int (ScriptRegion *ssr)
RuntimeScriptValue Sc_Region_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptRegion, Region_GetID);
}

// int (ScriptRegion *ssr)
RuntimeScriptValue Sc_Region_GetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptRegion, Region_GetLightLevel);
}

// void (ScriptRegion *ssr, int brightness)
RuntimeScriptValue Sc_Region_SetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptRegion, Region_SetLightLevel);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintEnabled(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptRegion, Region_GetTintEnabled);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintBlue(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptRegion, Region_GetTintBlue);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintGreen(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptRegion, Region_GetTintGreen);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintRed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptRegion, Region_GetTintRed);
}

// int (ScriptRegion *srr)
RuntimeScriptValue Sc_Region_GetTintSaturation(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptRegion, Region_GetTintSaturation);
}

RuntimeScriptValue Sc_Region_GetTintLuminance(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptRegion, Region_GetTintLuminance);
}

void RegisterRegionAPI() {
	ScFnRegister region_api[] = {
		{"Region::GetAtRoomXY^2", API_FN_PAIR(GetRegionAtRoom)},
		{"Region::GetAtScreenXY^2", API_FN_PAIR(GetRegionAtScreen)},
		{"Region::GetDrawingSurface", API_FN_PAIR(Region_GetDrawingSurface)},

		{"Region::Tint^4", API_FN_PAIR(Region_TintNoLum)},
		{"Region::Tint^5", API_FN_PAIR(Region_Tint)},
		{"Region::RunInteraction^1", API_FN_PAIR(Region_RunInteraction)},
		{"Region::get_Enabled", API_FN_PAIR(Region_GetEnabled)},
		{"Region::set_Enabled", API_FN_PAIR(Region_SetEnabled)},
		{"Region::get_ID", API_FN_PAIR(Region_GetID)},
		{"Region::get_LightLevel", API_FN_PAIR(Region_GetLightLevel)},
		{"Region::set_LightLevel", API_FN_PAIR(Region_SetLightLevel)},
		{"Region::get_TintEnabled", API_FN_PAIR(Region_GetTintEnabled)},
		{"Region::get_TintBlue", API_FN_PAIR(Region_GetTintBlue)},
		{"Region::get_TintGreen", API_FN_PAIR(Region_GetTintGreen)},
		{"Region::get_TintRed", API_FN_PAIR(Region_GetTintRed)},
		{"Region::get_TintSaturation", API_FN_PAIR(Region_GetTintSaturation)},
		{"Region::get_TintLuminance", API_FN_PAIR(Region_GetTintLuminance)},
	};

	ccAddExternalFunctions361(region_api);
}

} // namespace AGS3
