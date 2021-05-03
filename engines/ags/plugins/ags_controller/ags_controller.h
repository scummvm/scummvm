/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGSCONTROLLER_AGSCONTROLLER_H
#define AGS_PLUGINS_AGSCONTROLLER_AGSCONTROLLER_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace AGSController {

class AGSController : public PluginBase {
private:
	static IAGSEngine *_engine;

private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);
	static void AGS_EngineShutdown();
	static int64 AGS_EngineOnEvent(int event, NumberPtr data);

private:
	static void Controller_Update();

	static void ControllerCount(ScriptMethodParams &params);
	static void Controller_Open(ScriptMethodParams &params);
	static void Controller_Plugged(ScriptMethodParams &params);
	static void Controller_GetAxis(ScriptMethodParams &params);
	static void Controller_GetPOV(ScriptMethodParams &params);
	static void Controller_IsButtonDown(ScriptMethodParams &params);
	static void Controller_Close(ScriptMethodParams &params);
	static void Controller_GetName(ScriptMethodParams &params);
	static void Controller_Rumble(ScriptMethodParams &params);
	static void Controller_IsButtonDownOnce(ScriptMethodParams &params);
	static void Controller_PressAnyKey(ScriptMethodParams &params);
	static void Controller_BatteryStatus(ScriptMethodParams &params);
	static void ClickMouse(ScriptMethodParams &params);
public:
	AGSController();
};

} // namespace AGSController
} // namespace Plugins
} // namespace AGS3

#endif
