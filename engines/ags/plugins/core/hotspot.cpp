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

#include "ags/plugins/core/hotspot.h"
#include "ags/engine/ac/hotspot.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Hotspot::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Hotspot::GetAtRoomXY^2, GetHotspotAtRoom);
	SCRIPT_METHOD_EXT(Hotspot::GetAtScreenXY^2, GetHotspotAtScreen);
	SCRIPT_METHOD_EXT(Hotspot::GetName^1, GetName);
	SCRIPT_METHOD_EXT(Hotspot::GetProperty^1, GetProperty);
	SCRIPT_METHOD_EXT(Hotspot::GetPropertyText^2, GetPropertyText);
	SCRIPT_METHOD_EXT(Hotspot::GetTextProperty^1, GetTextProperty);
	SCRIPT_METHOD_EXT(Hotspot::RunInteraction^1, RunInteraction);
	SCRIPT_METHOD_EXT(Hotspot::get_Enabled, GetEnabled);
	SCRIPT_METHOD_EXT(Hotspot::set_Enabled, SetEnabled);
	SCRIPT_METHOD_EXT(Hotspot::get_ID, GetID);
	SCRIPT_METHOD_EXT(Hotspot::get_Name, GetName_New);
	SCRIPT_METHOD_EXT(Hotspot::get_WalkToX, GetWalkToX);
	SCRIPT_METHOD_EXT(Hotspot::get_WalkToY, GetWalkToY);
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
