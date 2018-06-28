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
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include <sys/stat.h>
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "backends/platform/sdl/switch/switch.h"
#include "backends/events/switchsdl/switchsdl-events.h"
#include "backends/saves/posix/posix-saves.h"
#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"

int access(const char *pathname, int mode) {

	struct stat sb;
	if (stat(pathname, &sb) == -1) {
		return -1;
	}

	return 0;
}

OSystem_SWITCH::OSystem_SWITCH(Common::String baseConfigName)
	: _baseConfigName(baseConfigName) {
}

void OSystem_SWITCH::init() {
	
	// Initialze File System Factory
	_fsFactory = new POSIXFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_SWITCH::initBackend() {

	ConfMan.registerDefault("joystick_num", 0);
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.registerDefault("gfx_mode", "2x");
	ConfMan.registerDefault("output_rate", 48000);
	ConfMan.registerDefault("touchpad_mouse_mode", true);

	if (!ConfMan.hasKey("joystick_num")) {
		ConfMan.setInt("joystick_num", 0);
	}
	if (!ConfMan.hasKey("fullscreen")) {
		ConfMan.setBool("fullscreen", true);
	}
	if (!ConfMan.hasKey("aspect_ratio")) {
		ConfMan.setBool("aspect_ratio", false);
	}
	if (!ConfMan.hasKey("gfx_mode")) {
		ConfMan.set("gfx_mode", "2x");
	}
	if (!ConfMan.hasKey("output_rate")) {
		ConfMan.setInt("output_rate", 48000);
	}
	if (!ConfMan.hasKey("touchpad_mouse_mode")) {
		ConfMan.setBool("touchpad_mouse_mode", true);
	}

	// Create the savefile manager
	if (_savefileManager == 0) {
		_savefileManager = new POSIXSaveFileManager();
	}

	// Event source
	if (_eventSource == 0) {
		_eventSource = new SWITCHEventSource();
	}

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

bool OSystem_SWITCH::hasFeature(Feature f) {
	
	if (f == kFeatureDisplayLogFile)
		return false;
	if (f == kFeatureOpenUrl)
		return false;

	return OSystem_SDL::hasFeature(f);
}

void OSystem_SWITCH::logMessage(LogMessageType::Type type, const char *message) {
	
	printf("%s\n", message);
}

Common::String OSystem_SWITCH::getDefaultConfigFileName() {
	return _baseConfigName;
}

Common::WriteStream *OSystem_SWITCH::createLogFile() {
	Common::FSNode file("scummvm.log");
	return file.createWriteStream();
}

