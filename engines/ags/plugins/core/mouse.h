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

#ifndef AGS_PLUGINS_CORE_MOUSE_H
#define AGS_PLUGINS_CORE_MOUSE_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Mouse : public ScriptContainer {
	BUILT_IN_HASH(Mouse)
public:
	virtual ~Mouse() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void ChangeCursorGraphic(ScriptMethodParams &params);
	void ChangeCursorHotspot(ScriptMethodParams &params);
	void Mouse_ChangeModeView2(ScriptMethodParams &params);
	void disable_cursor_mode(ScriptMethodParams &params);
	void enable_cursor_mode(ScriptMethodParams &params);
	void Mouse_GetModeGraphic(ScriptMethodParams &params);
	void IsButtonDown(ScriptMethodParams &params);
	void IsModeEnabled(ScriptMethodParams &params);
	void SaveCursorForLocationChange(ScriptMethodParams &params);
	void SetNextCursor(ScriptMethodParams &params);
	void SetPreviousCursor(ScriptMethodParams &params);
	void SetMouseBounds(ScriptMethodParams &params);
	void SetMousePosition(ScriptMethodParams &params);
	void RefreshMouse(ScriptMethodParams &params);
	void set_default_cursor(ScriptMethodParams &params);
	void set_mouse_cursor(ScriptMethodParams &params);
	void GetCursorMode(ScriptMethodParams &params);
	void set_cursor_mode(ScriptMethodParams &params);
	void Mouse_GetVisible(ScriptMethodParams &params);
	void Mouse_SetVisible(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
