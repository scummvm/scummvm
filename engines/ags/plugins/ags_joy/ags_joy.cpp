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

#include "ags/plugins/ags_joy/ags_joy.h"
#include "ags/shared/core/platform.h"

namespace AGS3 {
namespace Plugins {
namespace AGSJoy {

IAGSEngine *AGSJoy::_engine;

AGSJoy::AGSJoy() : PluginBase() {
	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSJoy::AGS_GetPluginName() {
	return "AGSJoy";
}

void AGSJoy::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

	// Register functions
	SCRIPT_METHOD_EXT(JoystickCount, Count);
	SCRIPT_METHOD_EXT(JoystickName, Name);
	SCRIPT_METHOD_EXT(JoystickRescan, Rescan);
	SCRIPT_METHOD_EXT(Joystick::Open, Open);
	SCRIPT_METHOD_EXT(Joystick::IsOpen, IsOpen);
	SCRIPT_METHOD_EXT(Joystick::Click, Click);
	SCRIPT_METHOD_EXT(Joystick::Close, Close);
	SCRIPT_METHOD_EXT(Joystick::Valid, Valid);
	SCRIPT_METHOD_EXT(Joystick::Unplugged, Unplugged);
	SCRIPT_METHOD_EXT(Joystick::GetName, GetName);
	SCRIPT_METHOD_EXT(Joystick::GetAxis, GetAxis);
	SCRIPT_METHOD_EXT(Joystick::IsButtonDown, IsButtonDown);
	SCRIPT_METHOD_EXT(Joystick::IsJoyBtnDown, IsJoyBtnDown);
	SCRIPT_METHOD_EXT(Joystick::Update, Update);
	SCRIPT_METHOD_EXT(Joystick::DisableEvents, DisableEvents);
	SCRIPT_METHOD_EXT(Joystick::EnableEvents, EnableEvents);
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
