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

#include "ags/plugins/ags_joy/ags_joy.h"
#include "ags/shared/core/platform.h"

namespace AGS3 {
namespace Plugins {
namespace AGSJoy {

const char *AGSJoy::AGS_GetPluginName() {
	return "AGSJoy";
}

void AGSJoy::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	// Register functions
	SCRIPT_METHOD(JoystickCount, AGSJoy::Count);
	SCRIPT_METHOD(JoystickName, AGSJoy::Name);
	SCRIPT_METHOD(JoystickRescan, AGSJoy::Rescan);
	SCRIPT_METHOD(Joystick::Open, AGSJoy::Open);
	SCRIPT_METHOD(Joystick::IsOpen, AGSJoy::IsOpen);
	SCRIPT_METHOD(Joystick::Click, AGSJoy::Click);
	SCRIPT_METHOD(Joystick::Close, AGSJoy::Close);
	SCRIPT_METHOD(Joystick::Valid, AGSJoy::Valid);
	SCRIPT_METHOD(Joystick::Unplugged, AGSJoy::Unplugged);
	SCRIPT_METHOD(Joystick::GetName, AGSJoy::GetName);
	SCRIPT_METHOD(Joystick::GetAxis, AGSJoy::GetAxis);
	SCRIPT_METHOD(Joystick::IsButtonDown, AGSJoy::IsButtonDown);
	SCRIPT_METHOD(Joystick::IsJoyBtnDown, AGSJoy::IsJoyBtnDown);
	SCRIPT_METHOD(Joystick::Update, AGSJoy::Update);
	SCRIPT_METHOD(Joystick::DisableEvents, AGSJoy::DisableEvents);
	SCRIPT_METHOD(Joystick::EnableEvents, AGSJoy::EnableEvents);
}

void AGSJoy::Count(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::Name(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::Rescan(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::Open(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::IsOpen(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::Click(ScriptMethodParams &params) {
}

void AGSJoy::Close(ScriptMethodParams &params) {
}

void AGSJoy::Valid(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::Unplugged(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::GetName(ScriptMethodParams &params) {
	params._result = NumberPtr();
}

void AGSJoy::GetAxis(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::IsButtonDown(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::IsJoyBtnDown(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSJoy::Update(ScriptMethodParams &params) {
}

void AGSJoy::DisableEvents(ScriptMethodParams &params) {
}

void AGSJoy::EnableEvents(ScriptMethodParams &params) {
}

} // namespace AGSJoy
} // namespace Plugins
} // namespace AGS3
