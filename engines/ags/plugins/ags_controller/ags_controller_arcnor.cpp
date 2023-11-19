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

#include "ags/plugins/ags_controller/ags_controller.h"
#include "ags/plugins/ags_controller/ags_controller_arcnor.h"

#include "ags/events.h"

namespace AGS3 {
namespace Plugins {
namespace AGSController {


void AGSControllerArcnor::AGS_EngineStartup(IAGSEngine *engine) {
	AGSController::AGS_EngineStartup(engine);

	SCRIPT_METHOD(AGSControllerArcnor::Controller_Open^1, AGSControllerArcnor::Controller_Open);
	SCRIPT_METHOD(AGSControllerArcnor::Controller_GetAxis^1, AGSControllerArcnor::Controller_GetAxis);
	SCRIPT_METHOD(AGSControllerArcnor::Controller_IsButtonDown^1, AGSControllerArcnor::Controller_IsButtonDown);
	SCRIPT_METHOD(AGSControllerArcnor::Controller_Rumble^3, AGSControllerArcnor::Controller_Rumble);
	SCRIPT_METHOD(AGSControllerArcnor::Controller_IsButtonDownOnce^1, AGSControllerArcnor::Controller_IsButtonDownOnce);
	SCRIPT_METHOD(AGSControllerArcnor::Controller_IsButtonUpOnce^1, AGSControllerArcnor::Controller_IsButtonUpOnce);
	SCRIPT_METHOD(AGSControllerArcnor::Controller_PressAnyButton, AGSControllerArcnor::Controller_PressAnyKey);
	SCRIPT_METHOD(AGSControllerArcnor::Controller_GetPlayerIndex^0, AGSControllerArcnor::Controller_GetPlayerIndex);
	SCRIPT_METHOD(AGSControllerArcnor::Controller_SetPlayerIndex^1, AGSControllerArcnor::Controller_SetPlayerIndex);
	SCRIPT_METHOD(RunVirtualKeyboard, AGSControllerArcnor::RunVirtualKeyboard);
}

void AGSControllerArcnor::Controller_IsButtonUpOnce(ScriptMethodParams &params) {
	PARAMS1(int, button);
	if (button < 0 || button > 31)
		params._result = false;
	else
		params._result = !(::AGS::g_events->getJoystickButtonOnce(button));
}

void AGSControllerArcnor::Controller_GetPlayerIndex(ScriptMethodParams &params) {
	//  return -1 as "not available"
	debug(0, "AGSController: GetPlayerIndex not implemented");
	params._result = -1;
}

void AGSControllerArcnor::Controller_SetPlayerIndex(ScriptMethodParams &params) {
	//	PARAMS1(int, id);
	//	not implemented
	debug(0, "AGSController: SetPlayerIndex not implemented");
}

void AGSControllerArcnor::RunVirtualKeyboard(ScriptMethodParams &params) {
	//	PARAMS6(int, keyboard_mode, const char *, initialtext, const char *, headertext, const char *, guidetext, const char *, oktext, int, maxchars);
	debug(0, "AGSController: Virtual Keyboard not implemented");
	params._result = _engine->CreateScriptString("");
}

} // namespace AGSController
} // namespace Plugins
} // namespace AGS3
