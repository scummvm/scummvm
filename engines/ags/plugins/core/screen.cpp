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

#include "ags/plugins/core/screen.h"
#include "ags/engine/ac/screen.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Screen::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Screen::get_Height, Screen::GetScreenHeight);
	SCRIPT_METHOD(Screen::get_Width, Screen::GetScreenWidth);
	SCRIPT_METHOD(Screen::get_AutoSizeViewportOnRoomLoad, Screen::GetAutoSizeViewport);
	SCRIPT_METHOD(Screen::set_AutoSizeViewportOnRoomLoad, Screen::SetAutoSizeViewport);
	SCRIPT_METHOD(Screen::get_Viewport, Screen::GetViewport);
	SCRIPT_METHOD(Screen::get_ViewportCount, Screen::GetViewportCount);
	SCRIPT_METHOD(Screen::geti_Viewports, Screen::GetAnyViewport);
	SCRIPT_METHOD(Screen::RoomToScreenPoint, Screen::RoomToScreenPoint);
}

void Screen::GetScreenHeight(ScriptMethodParams &params) {
	params._result = AGS3::Screen_GetScreenHeight();
}

void Screen::GetScreenWidth(ScriptMethodParams &params) {
	params._result = AGS3::Screen_GetScreenWidth();
}

void Screen::GetAutoSizeViewport(ScriptMethodParams &params) {
	params._result = AGS3::Screen_GetAutoSizeViewport();
}

void Screen::SetAutoSizeViewport(ScriptMethodParams &params) {
	PARAMS1(bool, on);
	AGS3::Screen_SetAutoSizeViewport(on);
}

void Screen::GetViewport(ScriptMethodParams &params) {
	params._result = AGS3::Screen_GetAnyViewport();
}

void Screen::GetViewportCount(ScriptMethodParams &params) {
	params._result = AGS3::Screen_GetViewportCount();
}

void Screen::GetAnyViewport(ScriptMethodParams &params) {
	PARAMS1(int, index);
	params._result = AGS3::Room_GetColorDepth(index);
}

void Screen::RoomToScreenPoint(ScriptMethodParams &params) {
	PARAMS2(int, roomx, int, roomy);
	params._result = AGS3::Room_GetHeight(roomx, roomy);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
