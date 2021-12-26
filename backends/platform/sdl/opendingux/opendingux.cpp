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

#define FORBIDDEN_SYMBOL_EXCEPTION_system

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/translation.h"

#include "backends/platform/sdl/opendingux/opendingux.h"

#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"
#include "backends/saves/default/default-saves.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper-defaults.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"

#define SCUMM_DIR	"~/.scummvm"
#define CONFIG_FILE	"~/.scummvmrc"
#define SAVE_PATH	"~/.scummvm/saves"
#define LOG_FILE	"~/.scummvm/scummvm.log"
#define JOYSTICK_DIR	"/sys/devices/platform/joystick"

static const Common::KeyTableEntry odKeyboardButtons[] = {
	{ "JOY_A",		Common::KEYCODE_LCTRL,		_s("A")			},
	{ "JOY_B",		Common::KEYCODE_LALT,		_s("B")			},
	{ "JOY_X",		Common::KEYCODE_SPACE,		_s("X")			},
	{ "JOY_Y",		Common::KEYCODE_LSHIFT,		_s("Y")			},
	{ "JOY_BACK",		Common::KEYCODE_ESCAPE,		_s("Select")		},
	{ "JOY_START",		Common::KEYCODE_RETURN,		_s("Start")		},
	{ "JOY_LEFT_SHOULDER",	Common::KEYCODE_TAB,		_s("L")			},
	{ "JOY_RIGHT_SHOULDER", Common::KEYCODE_BACKSPACE,	_s("R")			},
	{ "JOY_UP",		Common::KEYCODE_UP,		_s("D-pad Up")	},
	{ "JOY_DOWN",		Common::KEYCODE_DOWN,		_s("D-pad Down")	},
	{ "JOY_LEFT",		Common::KEYCODE_LEFT,		_s("D-pad Left")	},
	{ "JOY_RIGHT",		Common::KEYCODE_RIGHT,		_s("D-pad Right")	},
	{nullptr,			Common::KEYCODE_INVALID,	nullptr			}
};

static const Common::HardwareInputTableEntry odJoystickButtons[] = {
	{ "JOY_LEFT_TRIGGER",	Common::JOYSTICK_BUTTON_LEFT_STICK,	_s("L3")	 },
	{ nullptr,		0,					nullptr		 }
};

static const Common::AxisTableEntry odJoystickAxes[] = {
	{ "JOY_LEFT_STICK_X",  Common::JOYSTICK_AXIS_LEFT_STICK_X,  Common::kAxisTypeFull, _s("Left Stick X")  },
	{ "JOY_LEFT_STICK_Y",  Common::JOYSTICK_AXIS_LEFT_STICK_Y,  Common::kAxisTypeFull, _s("Left Stick Y")  },
	{ nullptr,	       0,				    Common::kAxisTypeFull, nullptr	       }
};

Common::KeymapperDefaultBindings *OSystem_SDL_Opendingux::getKeymapperDefaultBindings() {
	Common::KeymapperDefaultBindings *keymapperDefaultBindings = new Common::KeymapperDefaultBindings();

	if (!Posix::assureDirectoryExists(JOYSTICK_DIR)) { 
		keymapperDefaultBindings->setDefaultBinding(Common::kGlobalKeymapName, "VMOUSEUP", "JOY_UP");
		keymapperDefaultBindings->setDefaultBinding(Common::kGlobalKeymapName, "VMOUSEDOWN", "JOY_DOWN");
		keymapperDefaultBindings->setDefaultBinding(Common::kGlobalKeymapName, "VMOUSELEFT", "JOY_LEFT");
		keymapperDefaultBindings->setDefaultBinding(Common::kGlobalKeymapName, "VMOUSERIGHT", "JOY_RIGHT");
		keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, "UP", "");
		keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, "DOWN", "");
		keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, "LEFT", "");
		keymapperDefaultBindings->setDefaultBinding(Common::kGuiKeymapName, "RIGHT", "");
		keymapperDefaultBindings->setDefaultBinding("engine-default", "UP", "");
		keymapperDefaultBindings->setDefaultBinding("engine-default", "DOWN", "");
		keymapperDefaultBindings->setDefaultBinding("engine-default", "LEFT", "");
		keymapperDefaultBindings->setDefaultBinding("engine-default", "RIGHT", "");
	}

	return keymapperDefaultBindings;
}

void OSystem_SDL_Opendingux::init() {

	_fsFactory = new POSIXFilesystemFactory();
	if (!Posix::assureDirectoryExists(SCUMM_DIR)) {
		system("mkdir " SCUMM_DIR);
	}

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_SDL_Opendingux::initBackend() {
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);
	ConfMan.registerDefault("themepath", "./themes");
	ConfMan.registerDefault("extrapath", "./engine-data");
	ConfMan.registerDefault("gui_theme", "builtin");
	ConfMan.registerDefault("scale_factor", "1");

	ConfMan.setBool("fullscreen", true);
	ConfMan.setInt("joystick_num", 0);

	if (!ConfMan.hasKey("aspect_ratio")) {
		ConfMan.setBool("aspect_ratio", true);
	}
	if (!ConfMan.hasKey("themepath")) {
		ConfMan.set("themepath", "./themes");
	}
	if (!ConfMan.hasKey("extrapath")) {
		ConfMan.set("extrapath", "./engine-data");
	}
	if (!ConfMan.hasKey("savepath")) {
		ConfMan.set("savepath", SAVE_PATH);
	}
	if (!ConfMan.hasKey("gui_theme")) {
		ConfMan.set("gui_theme", "builtin");
	}
	if (!ConfMan.hasKey("scale_factor")) {
		ConfMan.set("scale_factor", "1");
	}
	if (!ConfMan.hasKey("opl_driver")) {
		ConfMan.set("opl_driver", "db");
	}
#ifdef LEPUS
	if (!ConfMan.hasKey("output_rate")) {
		ConfMan.set("output_rate", "22050");
	}
#elif RS90
	if (!ConfMan.hasKey("output_rate")) {
                ConfMan.set("output_rate", "11025");
        }
#endif
	// Create the savefile manager
	if (_savefileManager == nullptr) {
		_savefileManager = new DefaultSaveFileManager(SAVE_PATH);
	}

	OSystem_SDL::initBackend();
}

Common::String OSystem_SDL_Opendingux::getDefaultConfigFileName() {
	return CONFIG_FILE;

}

Common::String OSystem_SDL_Opendingux::getDefaultLogFileName() {
	return LOG_FILE;
}

bool OSystem_SDL_Opendingux::hasFeature(Feature f) {
	if (f == kFeatureFullscreenMode)
		return false;
	if (f == kFeatureAspectRatioCorrection)
		return false;

	return OSystem_SDL::hasFeature(f);
}

void OSystem_SDL_Opendingux::setFeatureState(Feature f, bool enable) {
	OSystem_SDL::setFeatureState(f, enable);
	}

bool OSystem_SDL_Opendingux::getFeatureState(Feature f) {
	return OSystem_SDL::getFeatureState(f);
}

Common::HardwareInputSet *OSystem_SDL_Opendingux::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();

	// Users may use USB mice - keyboards currently not possible with SDL1 as it conflicts with gpios
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));
	//inputSet->addHardwareInputSet(new KeyboardHardwareInputSet(defaultKeys, defaultModifiers));
	inputSet->addHardwareInputSet(new KeyboardHardwareInputSet(odKeyboardButtons, defaultModifiers));
	inputSet->addHardwareInputSet(new JoystickHardwareInputSet(odJoystickButtons, odJoystickAxes));

	return inputSet;
}

