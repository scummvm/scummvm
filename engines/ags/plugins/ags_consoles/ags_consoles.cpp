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

#include "ags/plugins/ags_consoles/ags_consoles.h"

namespace AGS3 {
namespace Plugins {
namespace AGSConsoles {

const char *AGSConsoles::AGS_GetPluginName() {
	return "AGS Consoles";
}

void AGSConsoles::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(IsPS4, AGSConsoles::IsPS4);
	SCRIPT_METHOD(IsPS5, AGSConsoles::IsPS5);
	SCRIPT_METHOD(IsSwitch, AGSConsoles::IsSwitch);
	SCRIPT_METHOD(IsPSVita, AGSConsoles::IsPSVita);
	SCRIPT_METHOD(IsXboxOne, AGSConsoles::IsXboxOne);
	SCRIPT_METHOD(IsSeriesX, AGSConsoles::IsSeriesX);
	SCRIPT_METHOD(SendStat, AGSConsoles::SendStat);
	SCRIPT_METHOD(SetAchievement, AGSConsoles::SetAchievement);
	SCRIPT_METHOD(RequestAccountPicker, AGSConsoles::RequestAccountPicker);
	SCRIPT_METHOD(LogMessage, AGSConsoles::LogMessage);
	SCRIPT_METHOD(GetGamertag, AGSConsoles::GetGamertag);
	SCRIPT_METHOD(StartStory, AGSConsoles::StartStory);
	SCRIPT_METHOD(ShowKeyboard, AGSConsoles::ShowKeyboard);
	SCRIPT_METHOD(ShouldOpenActivity, AGSConsoles::ShouldOpenActivity);
}

void AGSConsoles::IsPS4(ScriptMethodParams &params) {
	warning("AGSConsoles::IsPS4 STUB: returning false");
	params._result = false;
}

void AGSConsoles::IsPS5(ScriptMethodParams &params) {
	warning("AGSConsoles::IsPS5 STUB: returning false");
	params._result = false;
}

void AGSConsoles::IsSwitch(ScriptMethodParams &params) {
	warning("AGSConsoles::IsSwitch STUB: returning false");
	params._result = false;
}

void AGSConsoles::IsPSVita(ScriptMethodParams &params) {
	warning("AGSConsoles::IsPSVita STUB: returning false");
	params._result = false;
}

void AGSConsoles::IsXboxOne(ScriptMethodParams &params) {
	warning("AGSConsoles::IsXboxOne STUB: returning false");
	params._result = false;
}

void AGSConsoles::IsSeriesX(ScriptMethodParams &params) {
	warning("AGSConsoles::IsSeriesX STUB: returning false");
	params._result = false;
}

void AGSConsoles::SendStat(ScriptMethodParams &params) {
	warning("AGSConsoles::SendStat STUB");
}

void AGSConsoles::SetAchievement(ScriptMethodParams &params) {
	warning("AGSConsoles::SetAchievement STUB");
}

void AGSConsoles::RequestAccountPicker(ScriptMethodParams &params) {
	warning("AGSConsoles::RequestAccountPicker STUB");
}

void AGSConsoles::LogMessage(ScriptMethodParams &params) {
	PARAMS1(char *, msg);

	char buf[1024];
	snprintf(buf, sizeof(buf), "AGSConsoles: %s", msg);
	_engine->PrintDebugConsole(buf);
}

void AGSConsoles::GetGamertag(ScriptMethodParams &params) {
	warning("AGSConsoles::GetGamertag STUB: using \"ScummVM\"");
	params._result = _engine->CreateScriptString("ScummVM");
}

void AGSConsoles::StartStory(ScriptMethodParams &params) {
	warning("AGSConsoles::StartStory STUB");
}

void AGSConsoles::ShowKeyboard(ScriptMethodParams &params) {
	warning("AGSConsoles::ShowKeyboard STUB");
}

void AGSConsoles::ShouldOpenActivity(ScriptMethodParams &params) {
	warning("AGSConsoles::ShouldOpenActivity STUB");
}

} // namespace AGSConsoles
} // namespace Plugins
} // namespace AGS3
