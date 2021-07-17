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

#ifndef AGS_PLUGINS_CORE_MOUSE_H
#define AGS_PLUGINS_CORE_MOUSE_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Mouse : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void ChangeCursorGraphic(ScriptMethodParams &params);
	static void ChangeCursorHotspot(ScriptMethodParams &params);
	static void Mouse_ChangeModeView(ScriptMethodParams &params);
	static void disable_cursor_mode(ScriptMethodParams &params);
	static void enable_cursor_mode(ScriptMethodParams &params);
	static void Mouse_GetModeGraphic(ScriptMethodParams &params);
	static void IsButtonDown(ScriptMethodParams &params);
	static void IsModeEnabled(ScriptMethodParams &params);
	static void SaveCursorForLocationChange(ScriptMethodParams &params);
	static void SetNextCursor(ScriptMethodParams &params);
	static void SetPreviousCursor(ScriptMethodParams &params);
	static void SetMouseBounds(ScriptMethodParams &params);
	static void SetMousePosition(ScriptMethodParams &params);
	static void RefreshMouse(ScriptMethodParams &params);
	static void set_default_cursor(ScriptMethodParams &params);
	static void set_mouse_cursor(ScriptMethodParams &params);
	static void GetCursorMode(ScriptMethodParams &params);
	static void set_cursor_mode(ScriptMethodParams &params);
	static void Mouse_GetVisible(ScriptMethodParams &params);
	static void Mouse_SetVisible(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
