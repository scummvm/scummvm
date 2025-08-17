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

#include "ags/plugins/ags_utils/ags_utils.h"
#include "common/system.h"

namespace AGS3 {
namespace Plugins {
namespace AGSUtils {

const char *AGSUtils::AGS_GetPluginName() {
	return "AGS Utils";
}

void AGSUtils::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(DebugPrint, AGSUtils::DebugPrint);
	SCRIPT_METHOD(GetTime, AGSUtils::GetTime);
}

void AGSUtils::DebugPrint(ScriptMethodParams &params) {
	PARAMS1(char *, message);

	_engine->PrintDebugConsole(message);
}

void AGSUtils::GetTime(ScriptMethodParams &params) {

	params._result = g_system->getMillis();
}

} // namespace AGSUtils
} // namespace Plugins
} // namespace AGS3
