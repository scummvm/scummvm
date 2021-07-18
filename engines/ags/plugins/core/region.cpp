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

#include "ags/plugins/core/region.h"
#include "ags/engine/ac/region.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Region::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Region::GetAtRoomXY^2, GetRegionAtRoom);
	SCRIPT_METHOD_EXT(Region::GetAtScreenXY^2, GetRegionAtScreen);
	SCRIPT_METHOD_EXT(Region::Tint^4, TintNoLum);
	SCRIPT_METHOD_EXT(Region::RunInteraction^1, RunInteraction);
	SCRIPT_METHOD_EXT(Region::get_Enabled, GetEnabled);
	SCRIPT_METHOD_EXT(Region::set_Enabled, SetEnabled);
	SCRIPT_METHOD_EXT(Region::get_ID, GetID);
	SCRIPT_METHOD_EXT(Region::get_LightLevel, GetLightLevel);
	SCRIPT_METHOD_EXT(Region::set_LightLevel, SetLightLevel);
	SCRIPT_METHOD_EXT(Region::get_TintEnabled, GetTintEnabled);
	SCRIPT_METHOD_EXT(Region::get_TintBlue, GetTintBlue);
	SCRIPT_METHOD_EXT(Region::get_TintGreen, GetTintGreen);
	SCRIPT_METHOD_EXT(Region::get_TintRed, GetTintRed);
	SCRIPT_METHOD_EXT(Region::get_TintSaturation, GetTintSaturation);
}

void Region::GetRegionAtRoom(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetRegionAtRoom(xx, yy);
}

void Region::GetRegionAtScreen(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = AGS3::GetRegionAtScreen(x, y);
}

void Region::TintNoLum(ScriptMethodParams &params) {
	PARAMS5(ScriptRegion *, srr, int, red, int, green, int, blue, int, amount);
	AGS3::Region_TintNoLum(srr, red, green, blue, amount);
}

void Region::RunInteraction(ScriptMethodParams &params) {
	PARAMS2(ScriptRegion *, ssr, int, mood);
	AGS3::Region_RunInteraction(ssr, mood);
}

void Region::GetEnabled(ScriptMethodParams &params) {
	PARAMS1(ScriptRegion *, ssr);
	params._result = AGS3::Region_GetEnabled(ssr);
}

void Region::SetEnabled(ScriptMethodParams &params) {
	PARAMS2(ScriptRegion *, ssr, int, enable);
	AGS3::Region_SetEnabled(ssr, enable);
}

void Region::GetID(ScriptMethodParams &params) {
	PARAMS1(ScriptRegion *, ssr);
	params._result = AGS3::Region_GetID(ssr);
}

void Region::GetLightLevel(ScriptMethodParams &params) {
	PARAMS1(ScriptRegion *, ssr);
	params._result = AGS3::Region_GetLightLevel(ssr);
}

void Region::SetLightLevel(ScriptMethodParams &params) {
	PARAMS2(ScriptRegion *, ssr, int, brightness);
	AGS3::Region_SetLightLevel(ssr, brightness);
}

void Region::GetTintEnabled(ScriptMethodParams &params) {
	PARAMS1(ScriptRegion *, ssr);
	params._result = AGS3::Region_GetTintEnabled(ssr);
}

void Region::GetTintBlue(ScriptMethodParams &params) {
	PARAMS1(ScriptRegion *, ssr);
	params._result = AGS3::Region_GetTintBlue(ssr);
}

void Region::GetTintGreen(ScriptMethodParams &params) {
	PARAMS1(ScriptRegion *, ssr);
	params._result = AGS3::Region_GetTintGreen(ssr);
}

void Region::GetTintRed(ScriptMethodParams &params) {
	PARAMS1(ScriptRegion *, ssr);
	params._result = AGS3::Region_GetTintRed(ssr);
}

void Region::GetTintSaturation(ScriptMethodParams &params) {
	PARAMS1(ScriptRegion *, ssr);
	params._result = AGS3::Region_GetTintSaturation(ssr);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
