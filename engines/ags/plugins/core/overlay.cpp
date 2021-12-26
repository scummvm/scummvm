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

#include "ags/plugins/core/overlay.h"
#include "ags/engine/ac/overlay.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Overlay::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Overlay::CreateGraphical^4, Overlay::CreateGraphical);
	SCRIPT_METHOD(Overlay::CreateTextual^106, Overlay::ScPl_CreateTextual);
	SCRIPT_METHOD(Overlay::SetText^104, Overlay::ScPl_SetText);
	SCRIPT_METHOD(Overlay::Remove^0, Overlay::Remove);
	SCRIPT_METHOD(Overlay::get_Valid, Overlay::GetValid);
	SCRIPT_METHOD(Overlay::get_X, Overlay::GetX);
	SCRIPT_METHOD(Overlay::set_X, Overlay::SetX);
	SCRIPT_METHOD(Overlay::get_Y, Overlay::GetY);
	SCRIPT_METHOD(Overlay::set_Y, Overlay::SetY);
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
