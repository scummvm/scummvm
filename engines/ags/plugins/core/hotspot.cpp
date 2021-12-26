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

#include "ags/plugins/core/hotspot.h"
#include "ags/engine/ac/hotspot.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Hotspot::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Hotspot::GetAtRoomXY^2, Hotspot::GetHotspotAtRoom);
	SCRIPT_METHOD(Hotspot::GetAtScreenXY^2, Hotspot::GetHotspotAtScreen);
	SCRIPT_METHOD(Hotspot::GetName^1, Hotspot::GetName);
	SCRIPT_METHOD(Hotspot::GetProperty^1, Hotspot::GetProperty);
	SCRIPT_METHOD(Hotspot::GetPropertyText^2, Hotspot::GetPropertyText);
	SCRIPT_METHOD(Hotspot::GetTextProperty^1, Hotspot::GetTextProperty);
	SCRIPT_METHOD(Hotspot::RunInteraction^1, Hotspot::RunInteraction);
	SCRIPT_METHOD(Hotspot::get_Enabled, Hotspot::GetEnabled);
	SCRIPT_METHOD(Hotspot::set_Enabled, Hotspot::SetEnabled);
	SCRIPT_METHOD(Hotspot::get_ID, Hotspot::GetID);
	SCRIPT_METHOD(Hotspot::get_Name, Hotspot::GetName_New);
	SCRIPT_METHOD(Hotspot::get_WalkToX, Hotspot::GetWalkToX);
	SCRIPT_METHOD(Hotspot::get_WalkToY, Hotspot::GetWalkToY);
}

void Hotspot::GetHotspotAtRoom(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = AGS3::GetHotspotAtRoom(x, y);
}

void Hotspot::GetHotspotAtScreen(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetHotspotAtScreen(xx, yy);
}

void Hotspot::GetName(ScriptMethodParams &params) {
	PARAMS2(ScriptHotspot *, hss, char *, buffer);
	AGS3::Hotspot_GetName(hss, buffer);
}

void Hotspot::GetProperty(ScriptMethodParams &params) {
	PARAMS2(ScriptHotspot *, hss, const char *, property);
	params._result = AGS3::Hotspot_GetProperty(hss, property);
}

void Hotspot::GetPropertyText(ScriptMethodParams &params) {
	PARAMS3(ScriptHotspot *, hss, const char *, property, char *, bufer);
	AGS3::Hotspot_GetPropertyText(hss, property, bufer);
}

void Hotspot::GetTextProperty(ScriptMethodParams &params) {
	PARAMS2(ScriptHotspot *, hss, const char *, property);
	params._result = AGS3::Hotspot_GetTextProperty(hss, property);
}

void Hotspot::RunInteraction(ScriptMethodParams &params) {
	PARAMS2(ScriptHotspot *, hss, int, mood);
	AGS3::Hotspot_RunInteraction(hss, mood);
}

void Hotspot::GetEnabled(ScriptMethodParams &params) {
	PARAMS1(ScriptHotspot *, hss);
	params._result = AGS3::Hotspot_GetEnabled(hss);
}

void Hotspot::SetEnabled(ScriptMethodParams &params) {
	PARAMS2(ScriptHotspot *, hss, int, newval);
	AGS3::Hotspot_SetEnabled(hss, newval);
}

void Hotspot::GetID(ScriptMethodParams &params) {
	PARAMS1(ScriptHotspot *, hss);
	params._result = AGS3::Hotspot_GetID(hss);
}

void Hotspot::GetName_New(ScriptMethodParams &params) {
	PARAMS1(ScriptHotspot *, hss);
	params._result = AGS3::Hotspot_GetName_New(hss);
}

void Hotspot::GetWalkToX(ScriptMethodParams &params) {
	PARAMS1(ScriptHotspot *, hss);
	params._result = AGS3::Hotspot_GetWalkToX(hss);
}

void Hotspot::GetWalkToY(ScriptMethodParams &params) {
	PARAMS1(ScriptHotspot *, hss);
	params._result = AGS3::Hotspot_GetWalkToY(hss);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
