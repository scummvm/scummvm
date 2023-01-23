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

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"

#include "backends/audiocd/default/default-audiocd.h"
#include "backends/platform/sdl/kolibrios/kolibrios.h"
#include "backends/saves/kolibrios/kolibrios-saves.h"
#include "backends/fs/kolibrios/kolibrios-fs-factory.h"
#include "backends/fs/kolibrios/kolibrios-fs.h"

#include "common/textconsole.h"

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

OSystem_KolibriOS::OSystem_KolibriOS(const char *exeName) : _exeName(exeName) {
}

void OSystem_KolibriOS::init() {
	_exePath = Common::Path(_exeName).getParent();
	if (KolibriOS::assureDirectoryExists("scummvm-home", _exePath.toString().c_str())) {
		debug("Using <exec>/scummvm-home");
		_writablePath = _exePath.join("scummvm-home");
	} else {
		KolibriOS::assureDirectoryExists("scummvm", "/tmp0/1");
		_writablePath = "/tmp0/1/scummvm";
		debug("Using /tmp0/1");
	}

	// Initialze File System Factory
	_fsFactory = new KolibriOSFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_KolibriOS::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	Common::FSNode dataNode(_exePath);
	s.add("exePath", new Common::FSDirectory(dataNode, 4), priority);
}

void OSystem_KolibriOS::initBackend() {
	Common::String defaultThemePath = _exePath.join("themes").toString();
	Common::String defaultEngineData = _exePath.join("engine-data").toString();
	ConfMan.registerDefault("themepath", defaultThemePath);
	ConfMan.registerDefault("extrapath", defaultEngineData);

	if (!ConfMan.hasKey("themepath")) {
		ConfMan.set("themepath", defaultThemePath);
	}
	if (!ConfMan.hasKey("extrapath")) {
		ConfMan.set("extrapath", defaultEngineData);
	}

	// Create the savefile manager
	if (_savefileManager == 0)
		_savefileManager = new KolibriOSSaveFileManager(_writablePath);

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

Common::String OSystem_KolibriOS::getDefaultConfigFileName() {
	return _writablePath.join("scummvm.ini").toString();
}

Common::String OSystem_KolibriOS::getDefaultIconsPath() {
	return _exePath.join("icons").toString();
}

Common::String OSystem_KolibriOS::getScreenshotsPath() {
	// If the user has configured a screenshots path, use it
	const Common::String path = OSystem_SDL::getScreenshotsPath();
	if (!path.empty()) {
		return path;
	}

	static const char *SCREENSHOTS_DIR_NAME = "ScummVM Screenshots";
	if (!KolibriOS::assureDirectoryExists(SCREENSHOTS_DIR_NAME, _writablePath.toString().c_str())) {
		return "";
	}

	return _writablePath.join(SCREENSHOTS_DIR_NAME).toString();
}

Common::String OSystem_KolibriOS::getDefaultLogFileName() {
	return _writablePath.join("scummvm.log").toString();
}

AudioCDManager *OSystem_KolibriOS::createAudioCDManager() {
	return new DefaultAudioCDManager();
}
