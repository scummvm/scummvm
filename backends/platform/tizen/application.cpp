/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include "engines/engine.h"

#include "backends/platform/tizen/form.h"
#include "backends/platform/tizen/system.h"
#include "backends/platform/tizen/application.h"

Application *TizenScummVM::createInstance() {
	logEntered();
	return new TizenScummVM();
}

TizenScummVM::TizenScummVM() : _appForm(NULL) {
	logEntered();
}

TizenScummVM::~TizenScummVM() {
	logEntered();
	if (g_system) {
		TizenSystem *system = (TizenSystem *)g_system;
		system->destroyBackend();
		delete system;
		g_system = NULL;
	}
}

bool TizenScummVM::OnAppInitialized(void) {
	logEntered();
	_appForm->SetOrientation(Tizen::Ui::ORIENTATION_LANDSCAPE);
	return true;
}

bool TizenScummVM::OnAppWillTerminate(void) {
	logEntered();
	return true;
}

bool TizenScummVM::OnAppInitializing(AppRegistry &appRegistry) {
	logEntered();
	_appForm = systemStart(this);
	return (_appForm != NULL);
}

bool TizenScummVM::OnAppTerminating(AppRegistry &appRegistry, bool forcedTermination) {
	logEntered();
	return true;
}

void TizenScummVM::OnUserEventReceivedN(RequestId requestId, IList *args) {
	logEntered();
	MessageBox messageBox;
	int modalResult;
	String *message;

	switch (requestId) {
	case USER_MESSAGE_EXIT:
		// normal program termination
		Terminate();
		break;

	case USER_MESSAGE_EXIT_ERR:
		// assertion failure termination
		if (args) {
			message = (String *)args->GetAt(0);
		}
		if (!message) {
			message = new String("Unknown error");
		}
		messageBox.Construct(L"Oops...", *message, MSGBOX_STYLE_OK);
		messageBox.ShowAndWait(modalResult);
		Terminate();
		break;

	case USER_MESSAGE_EXIT_ERR_CONFIG:
		// the config file was corrupted
		messageBox.Construct(L"Config file corrupted",
				L"Settings have been reverted, please restart.", MSGBOX_STYLE_OK);
		messageBox.ShowAndWait(modalResult);
		Terminate();
		break;
	}
}

void TizenScummVM::OnForeground(void) {
	logEntered();
	pauseGame(false);
}

void TizenScummVM::OnBackground(void) {
	logEntered();
	pauseGame(true);
}

void TizenScummVM::OnBatteryLevelChanged(BatteryLevel batteryLevel) {
	logEntered();
}

void TizenScummVM::OnLowMemory(void) {
	logEntered();
}

void TizenScummVM::OnScreenOn(void) {
	logEntered();
}

void TizenScummVM::OnScreenOff(void) {
	logEntered();
}

void TizenScummVM::OnScreenBrightnessChanged(int brightness) {
	logEntered();
}

void TizenScummVM::pauseGame(bool pause) {
	if (_appForm) {
		if (pause && g_engine && !g_engine->isPaused()) {
			_appForm->pushKey(Common::KEYCODE_SPACE);
		}
		if (g_system) {
			((TizenSystem *)g_system)->setMute(pause);
		}
	}
}
