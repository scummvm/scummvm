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

#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	// sys/stat.h includes sys/time.h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "backends/platform/sdl/ps3/ps3.h"
#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#include "backends/saves/default/default-saves.h"
#include "backends/fs/ps3/ps3-fs-factory.h"
#include "backends/events/ps3sdl/ps3sdl-events.h"
#include "backends/keymapper/hardware-input.h"

#include <dirent.h>
#include <sys/stat.h>

static const Common::HardwareInputTableEntry playstationJoystickButtons[] = {
    { "JOY_A",              Common::JOYSTICK_BUTTON_A,              _s("Cross")       },
    { "JOY_B",              Common::JOYSTICK_BUTTON_B,              _s("Circle")      },
    { "JOY_X",              Common::JOYSTICK_BUTTON_X,              _s("Square")      },
    { "JOY_Y",              Common::JOYSTICK_BUTTON_Y,              _s("Triangle")    },
    { "JOY_BACK",           Common::JOYSTICK_BUTTON_BACK,           _s("Select")      },
    { "JOY_GUIDE",          Common::JOYSTICK_BUTTON_GUIDE,          _s("PS")          },
    { "JOY_START",          Common::JOYSTICK_BUTTON_START,          _s("Start")       },
    { "JOY_LEFT_STICK",     Common::JOYSTICK_BUTTON_LEFT_STICK,     _s("L3")          },
    { "JOY_RIGHT_STICK",    Common::JOYSTICK_BUTTON_RIGHT_STICK,    _s("R3")          },
    { "JOY_LEFT_SHOULDER",  Common::JOYSTICK_BUTTON_LEFT_SHOULDER,  _s("L1")          },
    { "JOY_RIGHT_SHOULDER", Common::JOYSTICK_BUTTON_RIGHT_SHOULDER, _s("R1")          },
    { "JOY_UP",             Common::JOYSTICK_BUTTON_DPAD_UP,        _s("D-pad Up")    },
    { "JOY_DOWN",           Common::JOYSTICK_BUTTON_DPAD_DOWN,      _s("D-pad Down")  },
    { "JOY_LEFT",           Common::JOYSTICK_BUTTON_DPAD_LEFT,      _s("D-pad Left")  },
    { "JOY_RIGHT",          Common::JOYSTICK_BUTTON_DPAD_RIGHT,     _s("D-pad Right") },
    { nullptr,              0,                                      nullptr           }
};

static const Common::AxisTableEntry playstationJoystickAxes[] = {
    { "JOY_LEFT_TRIGGER",  Common::JOYSTICK_AXIS_LEFT_TRIGGER,  Common::kAxisTypeHalf, _s("L2")            },
    { "JOY_RIGHT_TRIGGER", Common::JOYSTICK_AXIS_RIGHT_TRIGGER, Common::kAxisTypeHalf, _s("R2")            },
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

void OSystem_PS3::init() {
	// Initialze File System Factory
	_fsFactory = new PS3FilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_PS3::initBackend() {
	ConfMan.set("joystick_num", 0);
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);

	// Create the savefile manager
	if (_savefileManager == 0)
		_savefileManager = new DefaultSaveFileManager(PREFIX "/saves");

	// Event source
	if (_eventSource == 0)
		_eventSource = new PS3SdlEventSource();

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

Common::String OSystem_PS3::getDefaultConfigFileName() {
	return PREFIX "/scummvm.ini";
}

Common::String OSystem_PS3::getDefaultLogFileName() {
	return PREFIX "/scummvm.log";
}

Common::HardwareInputSet *OSystem_PS3::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();

	// Users may use USB / bluetooth mice and keyboards
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));
	inputSet->addHardwareInputSet(new KeyboardHardwareInputSet(defaultKeys, defaultModifiers));
	inputSet->addHardwareInputSet(new JoystickHardwareInputSet(playstationJoystickButtons, playstationJoystickAxes));

	return inputSet;
}
