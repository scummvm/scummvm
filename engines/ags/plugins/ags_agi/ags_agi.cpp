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

#include "ags/plugins/ags_agi/ags_agi.h"

namespace AGS3 {
namespace Plugins {
namespace AGSAgi {

const char *AGSAgi::AGS_GetPluginName() {
	return "AGS AGI Plugin stub (ags_agi.dll)";
}

void AGSAgi::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(SetAGIScalingMode, AGSAgi::SetAGIScalingMode);
	SCRIPT_METHOD(GetAGIScalingMode, AGSAgi::GetAGIScalingMode);
	SCRIPT_METHOD(UseAGIScaling, AGSAgi::UseAGIScaling);
}

void AGSAgi::SetAGIScalingMode(ScriptMethodParams &params) {
	PARAMS1(int, mode);
	// TODO rest of the code

	_scaling_mode = mode;
}

void AGSAgi::GetAGIScalingMode(ScriptMethodParams &params) {
	params._result = _scaling_mode;
}

void AGSAgi::UseAGIScaling(ScriptMethodParams &params) {
	PARAMS1(int, active);
	// TODO rest of the code

	_enabled = active;
}

} // namespace AGSAgi
} // namespace Plugins
} // namespace AGS3
