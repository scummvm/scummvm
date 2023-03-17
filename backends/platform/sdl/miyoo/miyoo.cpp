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

#ifdef MIYOOMINI
#include "backends/graphics/miyoo/miyoomini-graphics.h"
#endif
#include "backends/platform/sdl/miyoo/miyoo.h"

#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"
#include "backends/saves/default/default-saves.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper-defaults.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"

#ifdef MIYOOMINI
#define SCUMM_DIR	"/mnt/SDCARD/.scummvm"
#define CONFIG_FILE	"/mnt/SDCARD/.scummvmrc"
#define SAVE_PATH	"/mnt/SDCARD/.scummvm/saves"
#define LOG_FILE	"/mnt/SDCARD/.scummvm/scummvm.log"
#else
#define SCUMM_DIR	"/mnt/.scummvm"
#define CONFIG_FILE	"/mnt/.scummvmrc"
#define SAVE_PATH	"/mnt/.scummvm/saves"
#define LOG_FILE	"/mnt/.scummvm/scummvm.log"
#endif
#define JOYSTICK_DIR	"/sys/devices/platform/joystick"

static const Common::KeyTableEntry odKeyboardButtons[] = {
#ifdef MIYOOMINI
	{ "JOY_A",		Common::KEYCODE_SPACE,		_s("A")			}, // I18N: Hardware key
	{ "JOY_B",		Common::KEYCODE_LCTRL,		_s("B")			}, // I18N: Hardware key
	{ "JOY_X",		Common::KEYCODE_LSHIFT,		_s("X")			}, // I18N: Hardware key
	{ "JOY_Y",		Common::KEYCODE_LALT,		_s("Y")			}, // I18N: Hardware key
	{ "JOY_BACK",		Common::KEYCODE_RCTRL,		_s("Select")		}, // I18N: Hardware key
	{ "JOY_START",		Common::KEYCODE_RETURN,		_s("Start")		}, // I18N: Hardware key
	{ "JOY_LEFT_SHOULDER",	Common::KEYCODE_e,		_s("L")			}, // I18N: Hardware key
	{ "JOY_RIGHT_SHOULDER", Common::KEYCODE_t,		_s("R")			}, // I18N: Hardware key
	{ "JOY_UP",		Common::KEYCODE_UP,		_s("D-pad Up")	},
	{ "JOY_DOWN",		Common::KEYCODE_DOWN,		_s("D-pad Down")	},
	{ "JOY_LEFT",		Common::KEYCODE_LEFT,		_s("D-pad Left")	},
	{ "JOY_RIGHT",		Common::KEYCODE_RIGHT,		_s("D-pad Right")	},
	{ "JOY_LEFT_STICK",     Common::KEYCODE_TAB,		_s("L2")		}, // I18N: Hardware key
	{ "JOY_RIGHT_STICK",    Common::KEYCODE_BACKSPACE,	_s("R2")		}, // I18N: Hardware key
	{ "JOY_GUIDE",		Common::KEYCODE_ESCAPE,		_s("Menu")	 	}, // I18N: Hardware key
#else
	{ "JOY_A",		Common::KEYCODE_LALT,		_s("A")			}, // I18N: Hardware key
	{ "JOY_B",		Common::KEYCODE_LCTRL,		_s("B")			}, // I18N: Hardware key
	{ "JOY_X",		Common::KEYCODE_LSHIFT,		_s("X")			}, // I18N: Hardware key
	{ "JOY_Y",		Common::KEYCODE_SPACE,		_s("Y")			}, // I18N: Hardware key
	{ "JOY_BACK",		Common::KEYCODE_ESCAPE,		_s("Select")		}, // I18N: Hardware key
	{ "JOY_START",		Common::KEYCODE_RETURN,		_s("Start")		}, // I18N: Hardware key
	{ "JOY_LEFT_SHOULDER",	Common::KEYCODE_TAB,		_s("L")			}, // I18N: Hardware key
	{ "JOY_RIGHT_SHOULDER", Common::KEYCODE_BACKSPACE,	_s("R")			}, // I18N: Hardware key
	{ "JOY_UP",		Common::KEYCODE_UP,		_s("D-pad Up")	},
	{ "JOY_DOWN",		Common::KEYCODE_DOWN,		_s("D-pad Down")	},
	{ "JOY_LEFT",		Common::KEYCODE_LEFT,		_s("D-pad Left")	},
	{ "JOY_RIGHT",		Common::KEYCODE_RIGHT,		_s("D-pad Right")	},
	{ "JOY_LEFT_STICK",     Common::KEYCODE_PAGEUP,		_s("L2")		}, // I18N: Hardware key
	{ "JOY_RIGHT_STICK",    Common::KEYCODE_PAGEDOWN,	_s("R2")		}, // I18N: Hardware key
	{ "JOY_LEFT_TRIGGER",	Common::KEYCODE_RALT,		_s("L3")	 	}, // I18N: Hardware key
	{ "JOY_RIGHT_TRIGGER",	Common::KEYCODE_RSHIFT,		_s("R3")	 	}, // I18N: Hardware key
	{ "JOY_GUIDE",		Common::KEYCODE_RCTRL,		_s("Menu")	 	}, // I18N: Hardware key
#endif
	{nullptr,			Common::KEYCODE_INVALID,	nullptr			}
};

Common::KeymapperDefaultBindings *OSystem_SDL_Miyoo::getKeymapperDefaultBindings() {
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

void OSystem_SDL_Miyoo::init() {

	_fsFactory = new POSIXFilesystemFactory();
	if (!Posix::assureDirectoryExists(SCUMM_DIR)) {
		system("mkdir " SCUMM_DIR);
	}

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_SDL_Miyoo::initBackend() {
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
	if (!ConfMan.hasKey("kbdmouse_speed")) {
		ConfMan.setInt("kbdmouse_speed", 2);
	}
	// Create the savefile manager
	if (_savefileManager == nullptr) {
		_savefileManager = new DefaultSaveFileManager(SAVE_PATH);
	}

#ifdef MIYOOMINI
	if (!_eventSource)
		_eventSource = new SdlEventSource();
	if (!_graphicsManager)
		_graphicsManager = new MiyooMiniGraphicsManager(_eventSource, _window);
#endif

	OSystem_SDL::initBackend();
}

Common::String OSystem_SDL_Miyoo::getDefaultConfigFileName() {
	return CONFIG_FILE;

}

Common::String OSystem_SDL_Miyoo::getDefaultLogFileName() {
	return LOG_FILE;
}

bool OSystem_SDL_Miyoo::hasFeature(Feature f) {
	switch (f) {
	case kFeatureFullscreenMode:
	case kFeatureAspectRatioCorrection:
		return false;
	case kFeatureKbdMouseSpeed:
		return true;
	default:
		return OSystem_SDL::hasFeature(f);
	}
}

void OSystem_SDL_Miyoo::setFeatureState(Feature f, bool enable) {
	OSystem_SDL::setFeatureState(f, enable);
}

bool OSystem_SDL_Miyoo::getFeatureState(Feature f) {
	return OSystem_SDL::getFeatureState(f);
}

Common::HardwareInputSet *OSystem_SDL_Miyoo::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();

	// Users may use USB mice - keyboards currently not possible with SDL1 as it conflicts with gpios
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));
	inputSet->addHardwareInputSet(new KeyboardHardwareInputSet(odKeyboardButtons, defaultModifiers));

	return inputSet;
}
