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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "backends/platform/sdl/switch/switch.h"
#include "backends/events/switchsdl/switchsdl-events.h"
#include "backends/saves/posix/posix-saves.h"
#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"

OSystem_Switch::OSystem_Switch(Common::String baseConfigName)
	: _baseConfigName(baseConfigName) {
}

void OSystem_Switch::init() {
	
	// Initialze File System Factory
	_fsFactory = new POSIXFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_Switch::initBackend() {

	ConfMan.registerDefault("joystick_num", 0);
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.registerDefault("gfx_mode", "2x");
	ConfMan.registerDefault("filtering", true);
	ConfMan.registerDefault("output_rate", 48000);
	ConfMan.registerDefault("touchpad_mouse_mode", false);

	ConfMan.setBool("fullscreen", true);

	if (!ConfMan.hasKey("joystick_num")) {
		ConfMan.setInt("joystick_num", 0);
	}
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

void OSystem_Switch::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureTouchpadMode:
		ConfMan.setBool("touchpad_mouse_mode", enable);
		break;
	case kFeatureFullscreenMode:
		break;
	default:
		OSystem_SDL::setFeatureState(f, enable);
		break;
	}
}

bool OSystem_Switch::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureTouchpadMode:
		return ConfMan.getBool("touchpad_mouse_mode");
		break;
	case kFeatureFullscreenMode:
		return true;
		break;
	default:
		return OSystem_SDL::getFeatureState(f);
		break;
	}
}

void OSystem_Switch::logMessage(LogMessageType::Type type, const char *message) {
	printf("%s\n", message);
}

Common::String OSystem_Switch::getDefaultConfigFileName() {
	return _baseConfigName;
}

Common::String OSystem_Switch::getDefaultLogFileName() {
	return "scummvm.log";
}
