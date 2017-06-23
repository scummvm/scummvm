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
#include "common/debug-channels.h"
#include "backends/platform/sdl/psp2/psp2.h"
#include "backends/graphics/psp2sdl/psp2sdl-graphics.h"
#include "backends/saves/default/default-saves.h"

#include "backends/fs/psp2/psp2-fs-factory.h"
#include "backends/events/psp2sdl/psp2sdl-events.h"
#include "backends/fs/psp2/psp2-dirent.h"
#include <sys/stat.h>

#ifdef __PSP2_DEBUG__
#include <psp2shell.h>
#endif

int access(const char *pathname, int mode) {
	struct stat sb;

	if (stat(pathname, &sb) == -1) {
		return -1;
	}

	return 0;
}

OSystem_PSP2::OSystem_PSP2(Common::String baseConfigName)
	: _baseConfigName(baseConfigName) {
}

void OSystem_PSP2::init() {
	
#if __PSP2_DEBUG__
	gDebugLevel = 3;
#endif
	
	// Initialze File System Factory
	sceIoMkdir("ux0:data", 0755);
	sceIoMkdir("ux0:data/scummvm", 0755);
	sceIoMkdir("ux0:data/scummvm/saves", 0755);
	_fsFactory = new PSP2FilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_PSP2::initBackend() {
	
	ConfMan.set("joystick_num", 0);
	ConfMan.set("vkeybdpath", PREFIX "/data");
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.registerDefault("gfx_mode", "2x");
	ConfMan.registerDefault("filtering", true);
	ConfMan.registerDefault("kbdmouse_speed", 3);
	ConfMan.registerDefault("joystick_deadzone", 2);
	ConfMan.registerDefault("shader", 0);

	if (!ConfMan.hasKey("fullscreen")) {
		ConfMan.setBool("fullscreen", true);
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
	if (!ConfMan.hasKey("kbdmouse_speed")) {
		ConfMan.setInt("kbdmouse_speed", 3);
	}
	if (!ConfMan.hasKey("joystick_deadzone")) {
		ConfMan.setInt("joystick_deadzone", 2);
	}
	if (!ConfMan.hasKey("shader")) {
		ConfMan.setInt("shader", 0);
	}

	// Create the savefile manager
	if (_savefileManager == 0)
		_savefileManager = new DefaultSaveFileManager("ux0:data/scummvm/saves");

	// Event source
	if (_eventSource == 0)
		_eventSource = new PSP2EventSource();

	// Graphics Manager
	if (_graphicsManager == 0)
		_graphicsManager = new PSP2SdlGraphicsManager(_eventSource, _window);

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

bool OSystem_PSP2::hasFeature(Feature f) {
	return (f == kFeatureKbdMouseSpeed ||
		f == kFeatureJoystickDeadzone ||
		f == kFeatureShader ||
		OSystem_SDL::hasFeature(f));
}

void OSystem_PSP2::logMessage(LogMessageType::Type type, const char *message) {
#if __PSP2_DEBUG__
	psp2shell_print(message);
#endif
}

Common::String OSystem_PSP2::getDefaultConfigFileName() {
	return "ux0:data/scummvm/" + _baseConfigName;
}

Common::WriteStream *OSystem_PSP2::createLogFile() {
	Common::FSNode file("ux0:data/scummvm/scummvm.log");
	return file.createWriteStream();
}
