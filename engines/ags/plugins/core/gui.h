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

#ifndef AGS_PLUGINS_CORE_GUI_H
#define AGS_PLUGINS_CORE_GUI_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class GUI : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void Centre(ScriptMethodParams &params);
	static void GetGUIAtLocation(ScriptMethodParams &params);
	static void SetPosition(ScriptMethodParams &params);
	static void SetSize(ScriptMethodParams &params);
	static void GetBackgroundGraphic(ScriptMethodParams &params);
	static void SetBackgroundGraphic(ScriptMethodParams &params);
	static void GetClickable(ScriptMethodParams &params);
	static void SetClickable(ScriptMethodParams &params);
	static void GetControlCount(ScriptMethodParams &params);
	static void GetiControls(ScriptMethodParams &params);
	static void GetHeight(ScriptMethodParams &params);
	static void SetHeight(ScriptMethodParams &params);
	static void GetID(ScriptMethodParams &params);
	static void GetTransparency(ScriptMethodParams &params);
	static void SetTransparency(ScriptMethodParams &params);
	static void GetVisible(ScriptMethodParams &params);
	static void SetVisible(ScriptMethodParams &params);
	static void GetWidth(ScriptMethodParams &params);
	static void SetWidth(ScriptMethodParams &params);
	static void GetX(ScriptMethodParams &params);
	static void SetX(ScriptMethodParams &params);
	static void GetY(ScriptMethodParams &params);
	static void SetY(ScriptMethodParams &params);
	static void GetZOrder(ScriptMethodParams &params);
	static void SetZOrder(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
