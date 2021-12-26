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

#include "ags/ags.h"
#include "ags/plugins/ags_shell/ags_shell.h"

namespace AGS3 {
namespace Plugins {
namespace AGSShell {

const char *AGSShell::AGS_GetPluginName() {
	return "AGS shell plugin";
}

void AGSShell::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	// Make sure it's got the version with the features we need
	if (_engine->version < 3)
		_engine->AbortGame("Plugin needs engine version 3 or newer.");

	SCRIPT_METHOD(ShellExecute, AGSShell::ShellExecute);
}

void AGSShell::ShellExecute(ScriptMethodParams &params) {
	PARAMS3(const char *, operation, const char *, filename, const char *, parameters);
	warning("Unsupported ShellExecute(%s, %s, %s) command", operation, filename, parameters);

	params._result = 0;
}

} // namespace AGSShell
} // namespace Plugins
} // namespace AGS3
