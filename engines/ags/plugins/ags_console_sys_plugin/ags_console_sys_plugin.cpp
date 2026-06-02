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

#include "ags/plugins/ags_console_sys_plugin/ags_console_sys_plugin.h"
// #include "ags/shared/core/platform.h"
#include "common/config-manager.h"
#include "common/language.h"

namespace AGS3 {
namespace Plugins {
namespace AGSConsoleSysPlugin {

const char *AGSConsoleSysPlugin::AGS_GetPluginName() {
	return "AGS Console System Plugin";
}

void AGSConsoleSysPlugin::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(AGSConsoleSysPlugin::GetSystemInputDeviceType^0, AGSConsoleSysPlugin::GetSystemInputDeviceType);
	SCRIPT_METHOD(AGSConsoleSysPlugin::GetSystemLanguage^0, AGSConsoleSysPlugin::GetSystemLanguage);
	SCRIPT_METHOD(AGSConsoleSysPlugin::GetSystemPlatform^0, AGSConsoleSysPlugin::GetSystemPlatform);
	SCRIPT_METHOD(AGSConsoleSysPlugin::GetTextureFilteringType^0, AGSConsoleSysPlugin::GetTextureFilteringType);
	SCRIPT_METHOD(AGSConsoleSysPlugin::IsSaveHandlerBusy^0, AGSConsoleSysPlugin::IsSaveHandlerBusy);
	SCRIPT_METHOD(AGSConsoleSysPlugin::ResetControllerColor^0, AGSConsoleSysPlugin::ResetControllerColor);
	SCRIPT_METHOD(AGSConsoleSysPlugin::SetBorder^1, AGSConsoleSysPlugin::SetBorder);
	SCRIPT_METHOD(AGSConsoleSysPlugin::SetControllerColor^3, AGSConsoleSysPlugin::SetControllerColor);
	SCRIPT_METHOD(AGSConsoleSysPlugin::SetPixelPerfect^1, AGSConsoleSysPlugin::SetPixelPerfect);
	SCRIPT_METHOD(AGSConsoleSysPlugin::SetTextureFilteringType^1, AGSConsoleSysPlugin::SetTextureFilteringType);

	SCRIPT_METHOD(Game_RAG::EnableCheats^0, AGSConsoleSysPlugin::EnableCheats);
}

void AGSConsoleSysPlugin::GetSystemInputDeviceType(ScriptMethodParams &params) {
	warning("AGSConsoleSysPlugin::GetSystemInputDeviceType STUB: returning false");
	params._result = 0;
}

void AGSConsoleSysPlugin::GetSystemLanguage(ScriptMethodParams &params) {
	// PARAMS0();
	// Returns the current system language as an integer
	// 0   = German
	// 1/2 = English (unknown which is UK/US)
	// 3   = Spanish
	// 4   = ?
	// 5/6 = French (unknown which is France/Canada)
	// 7   = Italian
	if (!ConfMan.get("language").empty()) {
		// Map the language defined in the command-line "language" option to its description
		Common::Language lang = Common::parseLanguage(ConfMan.get("language"));

		switch (lang) {
		case Common::DE_DEU:
			params._result = 0;
			break;
		case Common::EN_ANY:
			params._result = 1;
			break;
		case Common::ES_ESP:
			params._result = 3;
			break;
		case Common::FR_FRA:
			params._result = 5;
			break;
		case Common::IT_ITA:
			params._result = 7;
			break;
		default:
			params._result = 1; // default to English
			break;
		}
	} else
		params._result = 1; // default to English
}

void AGSConsoleSysPlugin::GetSystemPlatform(ScriptMethodParams &params) {
	warning("AGSConsoleSysPlugin::GetSystemPlatform STUB: returning 0");
	params._result = 0;
}

void AGSConsoleSysPlugin::GetTextureFilteringType(ScriptMethodParams &params) {
	warning("AGSConsoleSysPlugin::GetTextureFilteringType STUB: returning false");
	params._result = false;
}

void AGSConsoleSysPlugin::IsSaveHandlerBusy(ScriptMethodParams &params) {
	warning("AGSConsoleSysPlugin::IsSaveHandlerBusy STUB: returning false");
	params._result = false;
}

void AGSConsoleSysPlugin::ResetControllerColor(ScriptMethodParams &params) {
	warning("AGSConsoleSysPlugin::ResetControllerColor STUB: returning false");
	params._result = false;
}

void AGSConsoleSysPlugin::SetBorder(ScriptMethodParams &params) {
	// PARAMS1(char *, filename);
	// Loads a BMP and uses it as a border. Result is ignored.
	// If "none.bmp", border is removed.
	params._result = 0;
}

void AGSConsoleSysPlugin::SetControllerColor(ScriptMethodParams &params) {
	// PARAMS3(?);
	warning("AGSConsoleSysPlugin::SetControllerColor STUB: returning false");
	params._result = false;
}

void AGSConsoleSysPlugin::SetPixelPerfect(ScriptMethodParams &params) {
	// PARAMS1(?);
	warning("AGSConsoleSysPlugin::SetPixelPerfect STUB: returning false");
	params._result = false;
}

void AGSConsoleSysPlugin::SetTextureFilteringType(ScriptMethodParams &params) {
	// PARAMS1(?);
	warning("AGSConsoleSysPlugin::SetTextureFilteringType STUB: returning false");
	params._result = false;
}

void AGSConsoleSysPlugin::EnableCheats(ScriptMethodParams &params) {
	params._result = false;
}


} // namespace AGSConsoleSysPlugin
} // namespace Plugins
} // namespace AGS3
