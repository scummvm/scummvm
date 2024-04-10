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

#include "ags/plugins/core/mouse.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/global_game.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Mouse::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Mouse::ChangeModeGraphic^2, Mouse::ChangeCursorGraphic);
	SCRIPT_METHOD(Mouse::ChangeModeHotspot^3, Mouse::ChangeCursorHotspot);
	SCRIPT_METHOD(Mouse::ChangeModeView^2, Mouse::Mouse_ChangeModeView2);
	SCRIPT_METHOD(Mouse::DisableMode^1, Mouse::disable_cursor_mode);
	SCRIPT_METHOD(Mouse::EnableMode^1, Mouse::enable_cursor_mode);
	SCRIPT_METHOD(Mouse::GetModeGraphic^1, Mouse::Mouse_GetModeGraphic);
	SCRIPT_METHOD(Mouse::IsButtonDown^1, Mouse::IsButtonDown);
	SCRIPT_METHOD(Mouse::IsModeEnabled^1, Mouse::IsModeEnabled);
	SCRIPT_METHOD(Mouse::SaveCursorUntilItLeaves^0, Mouse::SaveCursorForLocationChange);
	SCRIPT_METHOD(Mouse::SelectNextMode^0, Mouse::SetNextCursor);
	SCRIPT_METHOD(Mouse::SelectPreviousMode^0, Mouse::SetPreviousCursor);
	SCRIPT_METHOD(Mouse::SetBounds^4, Mouse::SetMouseBounds);
	SCRIPT_METHOD(Mouse::SetPosition^2, Mouse::SetMousePosition);
	SCRIPT_METHOD(Mouse::Update^0, Mouse::RefreshMouse);
	SCRIPT_METHOD(Mouse::UseDefaultGraphic^0, Mouse::set_default_cursor);
	SCRIPT_METHOD(Mouse::UseModeGraphic^1, Mouse::set_mouse_cursor);
	SCRIPT_METHOD(Mouse::get_Mode, Mouse::GetCursorMode);
	SCRIPT_METHOD(Mouse::set_Mode, Mouse::set_cursor_mode);
	SCRIPT_METHOD(Mouse::get_Visible, Mouse::Mouse_GetVisible);
	SCRIPT_METHOD(Mouse::set_Visible, Mouse::Mouse_SetVisible);
}

void Mouse::ChangeCursorGraphic(ScriptMethodParams &params) {
	PARAMS2(int, curs, int, newslot);
	AGS3::ChangeCursorGraphic(curs, newslot);
}

void Mouse::ChangeCursorHotspot(ScriptMethodParams &params) {
	PARAMS3(int, curs, int, x, int, y);
	AGS3::ChangeCursorHotspot(curs, x, y);
}

void Mouse::Mouse_ChangeModeView2(ScriptMethodParams &params) {
	PARAMS2(int, curs, int, newview);
	AGS3::Mouse_ChangeModeView2(curs, newview);
}

void Mouse::disable_cursor_mode(ScriptMethodParams &params) {
	PARAMS1(int, modd);
	AGS3::disable_cursor_mode(modd);
}

void Mouse::enable_cursor_mode(ScriptMethodParams &params) {
	PARAMS1(int, modd);
	AGS3::enable_cursor_mode(modd);
}

void Mouse::Mouse_GetModeGraphic(ScriptMethodParams &params) {
	PARAMS1(int, curs);
	params._result = AGS3::Mouse_GetModeGraphic(curs);
}

void Mouse::IsButtonDown(ScriptMethodParams &params) {
	PARAMS1(int, curs);
	params._result = AGS3::IsButtonDown(curs);
}

void Mouse::IsModeEnabled(ScriptMethodParams &params) {
	PARAMS1(int, curs);
	params._result = AGS3::IsModeEnabled(curs);
}

void Mouse::SaveCursorForLocationChange(ScriptMethodParams &params) {
	AGS3::SaveCursorForLocationChange();
}

void Mouse::SetNextCursor(ScriptMethodParams &params) {
	AGS3::SetNextCursor();
}

void Mouse::SetPreviousCursor(ScriptMethodParams &params) {
	AGS3::SetPreviousCursor();
}

void Mouse::SetMouseBounds(ScriptMethodParams &params) {
	PARAMS4(int, x1, int, y1, int, x2, int, y2);
	AGS3::SetMouseBounds(x1, y1, x2, y2);
}

void Mouse::SetMousePosition(ScriptMethodParams &params) {
	PARAMS2(int, newx, int, newy);
	AGS3::SetMousePosition(newx, newy);
}

void Mouse::RefreshMouse(ScriptMethodParams &params) {
	AGS3::RefreshMouse();
}

void Mouse::set_default_cursor(ScriptMethodParams &params) {
	AGS3::set_default_cursor();
}

void Mouse::set_mouse_cursor(ScriptMethodParams &params) {
	PARAMS1(int, newcurs);
	AGS3::set_mouse_cursor(newcurs);
}

void Mouse::GetCursorMode(ScriptMethodParams &params) {
	params._result = AGS3::GetCursorMode();
}

void Mouse::set_cursor_mode(ScriptMethodParams &params) {
	PARAMS1(int, newmode);
	AGS3::set_cursor_mode(newmode);
}

void Mouse::Mouse_GetVisible(ScriptMethodParams &params) {
	params._result = AGS3::Mouse_GetVisible();
}

void Mouse::Mouse_SetVisible(ScriptMethodParams &params) {
	PARAMS1(int, isOn);
	AGS3::Mouse_SetVisible(isOn);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
