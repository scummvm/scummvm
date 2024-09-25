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

#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	// sys/stat.h includes sys/time.h

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/translation.h"
#include "backends/platform/sdl/psp2/psp2.h"
#include "backends/saves/default/default-saves.h"
#include "backends/fs/posix-drives/posix-drives-fs-factory.h"
#include "backends/events/psp2sdl/psp2sdl-events.h"
#include "backends/keymapper/hardware-input.h"
#include <sys/stat.h>

#include <psp2/io/stat.h>

#ifdef __PSP2_DEBUG__
#include <psp2shell.h>
#endif

static const Common::HardwareInputTableEntry psp2JoystickButtons[] = {
	{ "JOY_A",              Common::JOYSTICK_BUTTON_A,              _s("Cross")       },
	{ "JOY_B",              Common::JOYSTICK_BUTTON_B,              _s("Circle")      },
	{ "JOY_X",              Common::JOYSTICK_BUTTON_X,              _s("Square")      },
	{ "JOY_Y",              Common::JOYSTICK_BUTTON_Y,              _s("Triangle")    },
	{ "JOY_BACK",           Common::JOYSTICK_BUTTON_BACK,           _s("Select")      },
	{ "JOY_START",          Common::JOYSTICK_BUTTON_START,          _s("Start")       },
	{ "JOY_LEFT_SHOULDER",  Common::JOYSTICK_BUTTON_LEFT_SHOULDER,  _s("L")           },
	{ "JOY_RIGHT_SHOULDER", Common::JOYSTICK_BUTTON_RIGHT_SHOULDER, _s("R")           },
	{ "JOY_UP",             Common::JOYSTICK_BUTTON_DPAD_UP,        _s("D-pad Up")    },
	{ "JOY_DOWN",           Common::JOYSTICK_BUTTON_DPAD_DOWN,      _s("D-pad Down")  },
	{ "JOY_LEFT",           Common::JOYSTICK_BUTTON_DPAD_LEFT,      _s("D-pad Left")  },
	{ "JOY_RIGHT",          Common::JOYSTICK_BUTTON_DPAD_RIGHT,     _s("D-pad Right") },
	{ nullptr,              0,                                      nullptr           }
};

static const Common::AxisTableEntry psp2JoystickAxes[] = {
	{ "JOY_LEFT_STICK_X",  Common::JOYSTICK_AXIS_LEFT_STICK_X,  Common::kAxisTypeFull, _s("Left Stick X")  },
	{ "JOY_LEFT_STICK_Y",  Common::JOYSTICK_AXIS_LEFT_STICK_Y,  Common::kAxisTypeFull, _s("Left Stick Y")  },
	{ "JOY_RIGHT_STICK_X", Common::JOYSTICK_AXIS_RIGHT_STICK_X, Common::kAxisTypeFull, _s("Right Stick X") },
	{ "JOY_RIGHT_STICK_Y", Common::JOYSTICK_AXIS_RIGHT_STICK_Y, Common::kAxisTypeFull, _s("Right Stick Y") },
	{ nullptr,             0,                                   Common::kAxisTypeFull, nullptr             }
};

int access(const char *pathname, int mode) {
	struct stat sb;

	if (stat(pathname, &sb) == -1) {
		return -1;
	}

	return 0;
}

void OSystem_PSP2::init() {

#if __PSP2_DEBUG__
	gDebugLevel = 3;
#endif

	// Initialize File System Factory
	sceIoMkdir("ux0:data", 0755);
	sceIoMkdir("ux0:data/scummvm", 0755);
	sceIoMkdir("ux0:data/scummvm/saves", 0755);

	DrivesPOSIXFilesystemFactory *fsFactory = new DrivesPOSIXFilesystemFactory();
	fsFactory->addDrive("ux0:");
	fsFactory->addDrive("uma0:");

	_fsFactory = fsFactory;

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_PSP2::initBackend() {

	ConfMan.set("joystick_num", 0);
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.registerDefault("gfx_mode", "2x");
	ConfMan.registerDefault("filtering", true);
	ConfMan.registerDefault("kbdmouse_speed", 3);
	ConfMan.registerDefault("joystick_deadzone", 2);
	ConfMan.registerDefault("touchpad_mouse_mode", false);
	ConfMan.registerDefault("frontpanel_touchpad_mode", false);
	ConfMan.registerDefault("gm_device", "null");

	ConfMan.setBool("fullscreen", true);

	if (!ConfMan.hasKey("aspect_ratio")) {
		ConfMan.setBool("aspect_ratio", false);
	}
	if (!ConfMan.hasKey("gfx_mode")) {
		ConfMan.set("gfx_mode", "2x");
	}
	if (!ConfMan.hasKey("filtering")) {
		ConfMan.setBool("filtering", true);
	}
	if (!ConfMan.hasKey("touchpad_mouse_mode")) {
		ConfMan.setBool("touchpad_mouse_mode", false);
	}
	if (!ConfMan.hasKey("frontpanel_touchpad_mode")) {
		ConfMan.setBool("frontpanel_touchpad_mode", false);
	}


	// Create the savefile manager
	if (_savefileManager == 0)
		_savefileManager = new DefaultSaveFileManager("ux0:data/scummvm/saves");

	// Controller mappings for Vita, various names have been used in various SDL versions
	SDL_GameControllerAddMapping("50535669746120436f6e74726f6c6c65,PSVita Controller,y:b0,b:b1,a:b2,x:b3,leftshoulder:b4,rightshoulder:b5,dpdown:b6,dpleft:b7,dpup:b8,dpright:b9,back:b10,start:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,");
	SDL_GameControllerAddMapping("50535669746120636f6e74726f6c6c65,PSVita controller,y:b0,b:b1,a:b2,x:b3,leftshoulder:b4,rightshoulder:b5,dpdown:b6,dpleft:b7,dpup:b8,dpright:b9,back:b10,start:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,");
	SDL_GameControllerAddMapping("50535669746120636f6e74726f6c6c65,PSVita controller 2,y:b0,b:b1,a:b2,x:b3,leftshoulder:b4,rightshoulder:b5,dpdown:b6,dpleft:b7,dpup:b8,dpright:b9,back:b10,start:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,");
	SDL_GameControllerAddMapping("50535669746120636f6e74726f6c6c65,PSVita controller 3,y:b0,b:b1,a:b2,x:b3,leftshoulder:b4,rightshoulder:b5,dpdown:b6,dpleft:b7,dpup:b8,dpright:b9,back:b10,start:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,");
	SDL_GameControllerAddMapping("50535669746120636f6e74726f6c6c65,PSVita controller 4,y:b0,b:b1,a:b2,x:b3,leftshoulder:b4,rightshoulder:b5,dpdown:b6,dpleft:b7,dpup:b8,dpright:b9,back:b10,start:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,");
	SDL_GameControllerAddMapping("505356697461206275696c74696e206a,PSVita builtin joypad,y:b0,b:b1,a:b2,x:b3,leftshoulder:b4,rightshoulder:b5,dpdown:b6,dpleft:b7,dpup:b8,dpright:b9,back:b10,start:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,");

	// Event source
	if (_eventSource == 0)
		_eventSource = new PSP2EventSource();

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

bool OSystem_PSP2::hasFeature(Feature f) {
	if (f == kFeatureFullscreenMode)
		return false;
	return (f == kFeatureKbdMouseSpeed ||
		f == kFeatureJoystickDeadzone ||
		f == kFeatureTouchpadMode ||
		OSystem_SDL::hasFeature(f));
}

void OSystem_PSP2::logMessage(LogMessageType::Type type, const char *message) {
#if __PSP2_DEBUG__
	psp2shell_print(message);
#endif
}

Common::Path OSystem_PSP2::getDefaultConfigFileName() {
	return "ux0:data/scummvm/scummvm.ini";
}

Common::Path OSystem_PSP2::getDefaultLogFileName() {
	return "ux0:data/scummvm/scummvm.log";
}

Common::HardwareInputSet *OSystem_PSP2::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();

	// Users may use USB / bluetooth mice and keyboards
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));
	inputSet->addHardwareInputSet(new KeyboardHardwareInputSet(defaultKeys, defaultModifiers));
	inputSet->addHardwareInputSet(new JoystickHardwareInputSet(psp2JoystickButtons, psp2JoystickAxes));

	return inputSet;
}
