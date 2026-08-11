/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGSCONTROLLER_AGSCONTROLLER_H
#define AGS_PLUGINS_AGSCONTROLLER_AGSCONTROLLER_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace AGSController {

class ConReader : public IAGSManagedObjectReader {
public:
	virtual void Unserialize(int key, const char *serializedData, int dataSize);
};

class AGSController : public PluginBase {
	SCRIPT_HASH(AGSController)
protected:
	void Controller_Update();

	void ControllerCount(ScriptMethodParams &params);
	void Controller_Open(ScriptMethodParams &params);
	void Controller_Plugged(ScriptMethodParams &params);
	void Controller_GetAxis(ScriptMethodParams &params);
	void Controller_GetPOV(ScriptMethodParams &params);
	void Controller_IsButtonDown(ScriptMethodParams &params);
	void Controller_Close(ScriptMethodParams &params);
	void Controller_GetName(ScriptMethodParams &params);
	void Controller_Rumble(ScriptMethodParams &params);
	void Controller_IsButtonDownOnce(ScriptMethodParams &params);
	void Controller_PressAnyKey(ScriptMethodParams &params);
	void Controller_BatteryStatus(ScriptMethodParams &params);
	void ClickMouse(ScriptMethodParams &params);
public:
	AGSController() : PluginBase() {}
	virtual ~AGSController();

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;
	int64 AGS_EngineOnEvent(int event, NumberPtr data) override;
};

} // namespace AGSController
} // namespace Plugins
} // namespace AGS3

#endif
