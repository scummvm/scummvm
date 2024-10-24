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
#include "ags/events.h"
#include "common/config-manager.h"
#include "common/system.h"

namespace AGS3 {
namespace Plugins {
namespace AGSController {

struct Controller : public IAGSScriptManagedObject {
public:

	int Dispose(const char *address, bool force) override {
		return true;
	}

	const char *GetType() override {
		return "Controller";
	};

	int Serialize(const char *address, char *buffer, int bufsize) override {
		return 0;
	}
};

Controller *ctrlInterface = nullptr;
ConReader *ctrlReader = nullptr;

Controller *getController() {
	if (!ctrlInterface)
		ctrlInterface = new Controller;

	return ctrlInterface;
}

ConReader *getConReader() {
	if (!ctrlReader)
		ctrlReader = new ConReader;

	return ctrlReader;
}

AGSController::~AGSController() {
	delete ctrlInterface;
	delete ctrlReader;
}

const char *AGSController::AGS_GetPluginName() {
	return "AGSController";
}

void AGSController::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(ControllerCount, AGSController::ControllerCount);
	SCRIPT_METHOD(Controller::Open, AGSController::Controller_Open);
	SCRIPT_METHOD(Controller::Close, AGSController::Controller_Close);
	SCRIPT_METHOD(Controller::Plugged, AGSController::Controller_Plugged);
	SCRIPT_METHOD(Controller::GetAxis, AGSController::Controller_GetAxis);
	SCRIPT_METHOD(Controller::GetPOV, AGSController::Controller_GetPOV);
	SCRIPT_METHOD(Controller::IsButtonDown, AGSController::Controller_IsButtonDown);
	SCRIPT_METHOD(Controller::GetName^0, AGSController::Controller_GetName);
	SCRIPT_METHOD(Controller::Rumble, AGSController::Controller_Rumble);
	SCRIPT_METHOD(Controller::IsButtonDownOnce, AGSController::Controller_IsButtonDownOnce);
	SCRIPT_METHOD(Controller::PressAnyKey, AGSController::Controller_PressAnyKey);
	SCRIPT_METHOD(Controller::BatteryStatus, AGSController::Controller_BatteryStatus);
	SCRIPT_METHOD(ClickMouse, AGSController::ClickMouse);

	_engine->RequestEventHook(AGSE_PREGUIDRAW);
	_engine->AddManagedObjectReader("Controller", getConReader());
}

int64 AGSController::AGS_EngineOnEvent(int event, NumberPtr data) {
	if (event == AGSE_PREGUIDRAW) {
		Controller_Update();
	}

	return 0;
}

void AGSController::Controller_Update() {
	//	::AGS::g_events->pollEvents();
}

void ConReader::Unserialize(int key, const char *serializedData, int dataSize) {
	//	no implementation
}

void AGSController::ControllerCount(ScriptMethodParams &params) {

	// WORKAROUND: The current implementation doesn't work at all, in UIHY at least
	// Just disable the gamepad for now, also to prevent accessing the controller options
	// that crash everything
	if ((ConfMan.get("gameid") == "untilihaveyou")) {
		debug(0, "AGSController: Returning ControllerCount=0 to force-disable controller!");
		params._result = 0;
		return;
	}
	int joystickNum = ConfMan.getInt("joystick_num");
	params._result = (joystickNum == -1) ? 0 : 1;
}

void AGSController::Controller_Open(ScriptMethodParams &params) {
	Controller *newCtrl = new Controller();
	_engine->RegisterManagedObject(newCtrl, getController());
	params._result = newCtrl;
}

void AGSController::Controller_Close(ScriptMethodParams &params) {
	// No implementation needed
}

void AGSController::Controller_Plugged(ScriptMethodParams &params) {
	int joystickNum = ConfMan.getInt("joystick_num");
	params._result = joystickNum != -1;
}

void AGSController::Controller_GetAxis(ScriptMethodParams &params) {
	PARAMS1(int, axis);
	if (axis < 0 || axis > 31)
		params._result = false;
	else
		params._result = ::AGS::g_events->getJoystickAxis(axis);
}

void AGSController::Controller_GetPOV(ScriptMethodParams &params) {
	// Not supported
	debug(0, "AGSController: POV is not implemented");
	params._result = 0;
}

void AGSController::Controller_IsButtonDown(ScriptMethodParams &params) {
	PARAMS1(int, button);
	if (button < 0 || button > 31)
		params._result = false;
	else
		params._result = ::AGS::g_events->getJoystickButton(button);
}

void AGSController::Controller_GetName(ScriptMethodParams &params) {
	int joystickNum = ConfMan.getInt("joystick_num");
	params._result = (joystickNum != -1) ? _engine->CreateScriptString("Joystick")
										 : _engine->CreateScriptString("");
}

void AGSController::Controller_Rumble(ScriptMethodParams &params) {
	// Not supported
	debug(0, "AGSController: Rumble is not supported");
}

void AGSController::Controller_IsButtonDownOnce(ScriptMethodParams &params) {
	PARAMS1(int, button);
	if (button < 0 || button > 31)
		params._result = false;
	else
		params._result = ::AGS::g_events->getJoystickButtonOnce(button);
}

void AGSController::Controller_PressAnyKey(ScriptMethodParams &params) {
	params._result = -1;

	for (int index = 0; index < 32; ++index) {
		if (::AGS::g_events->getJoystickButton(index)) {
			params._result = index;
			break;
		}
	}
}

void AGSController::Controller_BatteryStatus(ScriptMethodParams &params) {
	// Not supported, so return -1 for "UNKNOWN"
	debug(0, "AGSController: Battery status is not supported");
	params._result = -1;
}

void AGSController::ClickMouse(ScriptMethodParams &params) {
	PARAMS1(int, button);

	// WORKAROUND: This method is used to fake mouseclicks in the game menus (for keyboard
	// and gamepad control) but the implementation using eventmanager doesn't seem to work
	// Just use PluginSimulateMouseClick, which matches AGSController original implementation
	if ((ConfMan.get("gameid") == "untilihaveyou")) {
		PluginSimulateMouseClick(button);
		return;
	}

	assert(button < 3);
	Common::EventType DOWN[3] = {
		Common::EVENT_LBUTTONDOWN, Common::EVENT_RBUTTONDOWN, Common::EVENT_MBUTTONDOWN
	};
	Common::EventType UP[3] = {
		Common::EVENT_LBUTTONUP, Common::EVENT_RBUTTONUP, Common::EVENT_MBUTTONUP
	};

	Common::Point mousePos = ::AGS::g_events->getMousePos();
	Common::Event down, up;
	down.type = DOWN[button];
	down.mouse.x = mousePos.x;
	down.mouse.y = mousePos.y;
	g_system->getEventManager()->pushEvent(down);

	up.type = UP[button];
	up.mouse.x = mousePos.x;
	up.mouse.y = mousePos.y;
	g_system->getEventManager()->pushEvent(up);
}

} // namespace AGSController
} // namespace Plugins
} // namespace AGS3
