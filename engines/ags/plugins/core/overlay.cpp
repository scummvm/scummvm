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

#include "ags/plugins/core/overlay.h"
#include "ags/engine/ac/overlay.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Overlay::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Overlay::CreateGraphical^4, CreateGraphical);
	SCRIPT_METHOD_EXT(Overlay::CreateTextual^106, ScPl_CreateTextual);
	SCRIPT_METHOD_EXT(Overlay::SetText^104, ScPl_SetText);
	SCRIPT_METHOD_EXT(Overlay::Remove^0, Remove);
	SCRIPT_METHOD_EXT(Overlay::get_Valid, GetValid);
	SCRIPT_METHOD_EXT(Overlay::get_X, GetX);
	SCRIPT_METHOD_EXT(Overlay::set_X, SetX);
	SCRIPT_METHOD_EXT(Overlay::get_Y, GetY);
	SCRIPT_METHOD_EXT(Overlay::set_Y, SetY);
}

void Overlay::CreateGraphical(ScriptMethodParams &params) {
	PARAMS4(int, x, int, y, int, slot, int, transparent);
	params._result = AGS3::Overlay_CreateGraphical(x, y, slot, transparent);
}

void Overlay::ScPl_CreateTextual(ScriptMethodParams &params) {
	PARAMS6(int, x, int, y, int, width, int, font, int, colour, const char *, text);
	params._result = AGS3::Overlay_CreateTextual(x, y, width, font, colour, text);
}

void Overlay::ScPl_SetText(ScriptMethodParams &params) {
	PARAMS5(int, x, int, y, int, width, int, font, int, colour);
	Common::String text = params.format(5);
	params._result = AGS3::Overlay_CreateTextual(x, y, width, font, colour, text.c_str());
}

void Overlay::Remove(ScriptMethodParams &params) {
	PARAMS1(ScriptOverlay *, sco);
	AGS3::Overlay_Remove(sco);
}

void Overlay::GetValid(ScriptMethodParams &params) {
	PARAMS1(ScriptOverlay *, sco);
	params._result = AGS3::Overlay_GetValid(sco);
}

void Overlay::GetX(ScriptMethodParams &params) {
	PARAMS1(ScriptOverlay *, sco);
	params._result = AGS3::Overlay_GetX(sco);
}

void Overlay::SetX(ScriptMethodParams &params) {
	PARAMS2(ScriptOverlay *, scover, int, newx);
	AGS3::Overlay_SetX(scover, newx);
}

void Overlay::GetY(ScriptMethodParams &params) {
	PARAMS1(ScriptOverlay *, sco);
	params._result = AGS3::Overlay_GetY(sco);
}

void Overlay::SetY(ScriptMethodParams &params) {
	PARAMS2(ScriptOverlay *, scover, int, newy);
	AGS3::Overlay_SetY(scover, newy);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
