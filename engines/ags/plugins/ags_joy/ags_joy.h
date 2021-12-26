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

#ifndef AGS_PLUGINS_AGS_JOY_AGS_JOY_H
#define AGS_PLUGINS_AGS_JOY_AGS_JOY_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace AGSJoy {

class AGSJoy : public PluginBase {
	SCRIPT_HASH(AGSJoy)
private:
	void Count(ScriptMethodParams &params);
	void Name(ScriptMethodParams &params);
	void Rescan(ScriptMethodParams &params);
	void Open(ScriptMethodParams &params);
	void IsOpen(ScriptMethodParams &params);
	void Click(ScriptMethodParams &params);
	void Close(ScriptMethodParams &params);
	void Valid(ScriptMethodParams &params);
	void Unplugged(ScriptMethodParams &params);
	void GetName(ScriptMethodParams &params);
	void GetAxis(ScriptMethodParams &params);
	void IsButtonDown(ScriptMethodParams &params);
	void IsJoyBtnDown(ScriptMethodParams &params);
	void Update(ScriptMethodParams &params);
	void DisableEvents(ScriptMethodParams &params);
	void EnableEvents(ScriptMethodParams &params);

public:
	AGSJoy() : PluginBase() {}
	virtual ~AGSJoy() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *lpEngine) override;
};

} // namespace AGSJoy
} // namespace Plugins
} // namespace AGS3

#endif
