/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <switch.h>

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "backends/platform/sdl/switch/switch.h"
#include "backends/events/switchsdl/switchsdl-events.h"
#include "backends/saves/posix/posix-saves.h"
#include "backends/fs/posix-drives/posix-drives-fs-factory.h"
#include "backends/fs/posix-drives/posix-drives-fs.h"
#include "backends/keymapper/hardware-input.h"

static const Common::HardwareInputTableEntry switchJoystickButtons[] = {
	{ "JOY_A",              Common::JOYSTICK_BUTTON_A,              _s("B")           },
	{ "JOY_B",              Common::JOYSTICK_BUTTON_B,              _s("A")           },
	{ "JOY_X",              Common::JOYSTICK_BUTTON_X,              _s("Y")           },
	{ "JOY_Y",              Common::JOYSTICK_BUTTON_Y,              _s("X")           },
	{ "JOY_BACK",           Common::JOYSTICK_BUTTON_BACK,           _s("Minus")       },
	{ "JOY_START",          Common::JOYSTICK_BUTTON_START,          _s("Plus")        },
	{ "JOY_GUIDE",          Common::JOYSTICK_BUTTON_START,          _s("Plus")        },
	{ "JOY_LEFT_STICK",     Common::JOYSTICK_BUTTON_LEFT_STICK,     _s("L3")          },
	{ "JOY_RIGHT_STICK",    Common::JOYSTICK_BUTTON_RIGHT_STICK,    _s("R3")          },
	{ "JOY_LEFT_SHOULDER",  Common::JOYSTICK_BUTTON_LEFT_SHOULDER,  _s("L")           },
	{ "JOY_RIGHT_SHOULDER", Common::JOYSTICK_BUTTON_RIGHT_SHOULDER, _s("R")           },
	{ "JOY_UP",             Common::JOYSTICK_BUTTON_DPAD_UP,        _s("D-pad Up")    },
	{ "JOY_DOWN",           Common::JOYSTICK_BUTTON_DPAD_DOWN,      _s("D-pad Down")  },
	{ "JOY_LEFT",           Common::JOYSTICK_BUTTON_DPAD_LEFT,      _s("D-pad Left")  },
	{ "JOY_RIGHT",          Common::JOYSTICK_BUTTON_DPAD_RIGHT,     _s("D-pad Right") },
	{ nullptr,              0,                                      nullptr           }
};

static const Common::AxisTableEntry switchJoystickAxes[] = {
	{ "JOY_LEFT_TRIGGER",  Common::JOYSTICK_AXIS_LEFT_TRIGGER,  Common::kAxisTypeHalf, _s("ZL")            },
	{ "JOY_RIGHT_TRIGGER", Common::JOYSTICK_AXIS_RIGHT_TRIGGER, Common::kAxisTypeHalf, _s("ZR")            },
	{ "JOY_LEFT_STICK_X",  Common::JOYSTICK_AXIS_LEFT_STICK_X,  Common::kAxisTypeFull, _s("Left Stick X")  },
	{ "JOY_LEFT_STICK_Y",  Common::JOYSTICK_AXIS_LEFT_STICK_Y,  Common::kAxisTypeFull, _s("Left Stick Y")  },
	{ "JOY_RIGHT_STICK_X", Common::JOYSTICK_AXIS_RIGHT_STICK_X, Common::kAxisTypeFull, _s("Right Stick X") },
	{ "JOY_RIGHT_STICK_Y", Common::JOYSTICK_AXIS_RIGHT_STICK_Y, Common::kAxisTypeFull, _s("Right Stick Y") },
	{ nullptr,             0,                                   Common::kAxisTypeFull, nullptr             }
};

void OSystem_Switch::init() {

	DrivesPOSIXFilesystemFactory *fsFactory = new DrivesPOSIXFilesystemFactory();
	fsFactory->addDrive("sdmc:");
	fsFactory->configureBuffering(DrivePOSIXFilesystemNode::kBufferingModeScummVM, 2048);

	_fsFactory = fsFactory;

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_Switch::initBackend() {

	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.registerDefault("gfx_mode", "2x");
	ConfMan.registerDefault("filtering", true);
	ConfMan.registerDefault("output_rate", 48000);
	ConfMan.registerDefault("touchpad_mouse_mode", false);
	ConfMan.registerDefault("gm_device", "null");

	ConfMan.setBool("fullscreen", true);
	ConfMan.setInt("joystick_num", 0);

	if (!ConfMan.hasKey("aspect_ratio")) {
		ConfMan.setBool("aspect_ratio", false);
	}
	if (!ConfMan.hasKey("gfx_mode")) {
		ConfMan.set("gfx_mode", "2x");
	}
	if (!ConfMan.hasKey("filtering")) {
		ConfMan.setBool("filtering", true);
	}
	if (!ConfMan.hasKey("output_rate")) {
		ConfMan.setInt("output_rate", 48000);
	}
	if (!ConfMan.hasKey("touchpad_mouse_mode")) {
		ConfMan.setBool("touchpad_mouse_mode", false);
	}

	// Create the savefile manager
	if (_savefileManager == 0) {
		_savefileManager = new DefaultSaveFileManager("./saves");
	}

	// Event source
	if (_eventSource == 0) {
		_eventSource = new SwitchEventSource();
	}

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

bool OSystem_Switch::hasFeature(Feature f) {
	if (f == kFeatureFullscreenMode)
		return false;
	return (f == kFeatureTouchpadMode ||
		OSystem_SDL::hasFeature(f));
}

void OSystem_Switch::logMessage(LogMessageType::Type type, const char *message) {
	printf("%s\n", message);
}

Common::Path OSystem_Switch::getDefaultLogFileName() {
	return "scummvm.log";
}

Common::HardwareInputSet *OSystem_Switch::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();

	// Users may use USB / bluetooth mice and keyboards
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));
	inputSet->addHardwareInputSet(new KeyboardHardwareInputSet(defaultKeys, defaultModifiers));
	inputSet->addHardwareInputSet(new JoystickHardwareInputSet(switchJoystickButtons, switchJoystickAxes));

	return inputSet;
}

Common::String OSystem_Switch::getSystemLanguage() const {
	u64 lang;
	SetLanguage langcode;

	setInitialize();
	setGetSystemLanguage(&lang);
	setMakeLanguage(lang, &langcode);

	switch (langcode) {
		case SetLanguage_JA:   return "ja_JP";
		case SetLanguage_ENUS: return "en_US";
		case SetLanguage_FR:   return "fr_FR";
		case SetLanguage_FRCA: return "fr_FR";
		case SetLanguage_DE:   return "de_DE";
		case SetLanguage_IT:   return "it_IT";
		case SetLanguage_ES:   return "es_ES";
		case SetLanguage_ZHCN: return "zh_CN";
		case SetLanguage_KO:   return "ko_KR";
		case SetLanguage_NL:   return "nl_NL";
		case SetLanguage_PT:   return "pt_PT";
		case SetLanguage_RU:   return "ru_RU";
		case SetLanguage_ZHTW: return "zh_HK";
		default:               return "en_US";
	}
}
