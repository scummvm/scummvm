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

#include "ags/plugins/ags_agi/ags_agi.h"

namespace AGS3 {
namespace Plugins {
namespace AGSAgi {

int screen_mode = 0;

IAGSEngine *AGSAgi::_engine;
int AGSAgi::_enabled;
int AGSAgi::_scaling_mode;

AGSAgi::AGSAgi() : PluginBase() {
	_engine = nullptr;
	_enabled = 0;
	_scaling_mode = 0;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSAgi::AGS_GetPluginName() {
	return "AGS AGI Plugin stub (ags_agi.dll)";
}

void AGSAgi::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;
	_enabled = 0;
	_scaling_mode = 0;

	SCRIPT_METHOD(SetAGIScalingMode);
	SCRIPT_METHOD(GetAGIScalingMode);
	SCRIPT_METHOD(UseAGIScaling);
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
