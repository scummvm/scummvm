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

#include "ags/plugins/core/region.h"
#include "ags/engine/ac/region.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Region::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Region::GetAtRoomXY^2, Region::GetRegionAtRoom);
	SCRIPT_METHOD(Region::GetAtScreenXY^2, Region::GetRegionAtScreen);
	SCRIPT_METHOD(Region::Tint^4, Region::TintNoLum);
	SCRIPT_METHOD(Region::RunInteraction^1, Region::RunInteraction);
	SCRIPT_METHOD(Region::get_Enabled, Region::GetEnabled);
	SCRIPT_METHOD(Region::set_Enabled, Region::SetEnabled);
	SCRIPT_METHOD(Region::get_ID, Region::GetID);
	SCRIPT_METHOD(Region::get_LightLevel, Region::GetLightLevel);
	SCRIPT_METHOD(Region::set_LightLevel, Region::SetLightLevel);
	SCRIPT_METHOD(Region::get_TintEnabled, Region::GetTintEnabled);
	SCRIPT_METHOD(Region::get_TintBlue, Region::GetTintBlue);
	SCRIPT_METHOD(Region::get_TintGreen, Region::GetTintGreen);
	SCRIPT_METHOD(Region::get_TintRed, Region::GetTintRed);
	SCRIPT_METHOD(Region::get_TintSaturation, Region::GetTintSaturation);
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
