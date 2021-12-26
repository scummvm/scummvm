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

#ifndef AGS_PLUGINS_CORE_GUI_CONTROL_H
#define AGS_PLUGINS_CORE_GUI_CONTROL_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class GUIControl : public ScriptContainer {
	BUILT_IN_HASH(GUIControl)
public:
	virtual ~GUIControl() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void BringToFront(ScriptMethodParams &params);
	void GetGUIControlAtLocation(ScriptMethodParams &params);
	void SendToBack(ScriptMethodParams &params);
	void SetPosition(ScriptMethodParams &params);
	void SetSize(ScriptMethodParams &params);
	void GetAsButton(ScriptMethodParams &params);
	void GetAsInvWindow(ScriptMethodParams &params);
	void GetAsLabel(ScriptMethodParams &params);
	void GetAsListBox(ScriptMethodParams &params);
	void GetAsSlider(ScriptMethodParams &params);
	void GetAsTextBox(ScriptMethodParams &params);
	void GetClickable(ScriptMethodParams &params);
	void SetClickable(ScriptMethodParams &params);
	void GetEnabled(ScriptMethodParams &params);
	void SetEnabled(ScriptMethodParams &params);
	void GetHeight(ScriptMethodParams &params);
	void SetHeight(ScriptMethodParams &params);
	void GetID(ScriptMethodParams &params);
	void GetOwningGUI(ScriptMethodParams &params);
	void GetVisible(ScriptMethodParams &params);
	void SetVisible(ScriptMethodParams &params);
	void GetWidth(ScriptMethodParams &params);
	void SetWidth(ScriptMethodParams &params);
	void GetX(ScriptMethodParams &params);
	void SetX(ScriptMethodParams &params);
	void GetY(ScriptMethodParams &params);
	void SetY(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
