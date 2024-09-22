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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/translation.h"

#include "backends/platform/sdl/sailfish/sailfish.h"
#include "backends/platform/sdl/sailfish/sailfish-window.h"

#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"
#include "backends/saves/default/default-saves.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper-defaults.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"

#define ORG_NAME        "org.scummvm"
#define APP_NAME        "scummvm"

void OSystem_SDL_Sailfish::init() {
	setenv("SDL_VIDEO_WAYLAND_WMCLASS", "org.scummvm.scummvm", 1);
	// Initialze File System Factory
	_fsFactory = new POSIXFilesystemFactory();

	_window = new SdlWindow_Sailfish();

	_isAuroraOS = false;
	char *line = nullptr;
	size_t n = 0;
	FILE *os_release = fopen("/etc/os-release", "r");
	if (os_release) {
		while (getline(&line, &n, os_release) > 0) {
			if (strncmp(line, "ID=auroraos", sizeof("ID=auroraos") - 1) == 0) {
				_isAuroraOS = true;
			}
		}
		free(line);
		fclose(os_release);
	}

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

Common::String OSystem_SDL_Sailfish::getAppSuffix() {
	if (_isAuroraOS) {
		return ORG_NAME "/" APP_NAME;
	} else {
		return ORG_NAME "." APP_NAME;
	}
}

void OSystem_SDL_Sailfish::initBackend() {
	if (!ConfMan.hasKey("rotation_mode")) {
		ConfMan.setPath("rotation_mode", "90");
	}

	if (!ConfMan.hasKey("savepath")) {
		ConfMan.setPath("savepath", getDefaultSavePath());
	}

	// Create the savefile manager
	if (_savefileManager == nullptr) {
		_savefileManager = new DefaultSaveFileManager(getDefaultSavePath());
	}

	OSystem_SDL::initBackend();
}

Common::Path OSystem_SDL_Sailfish::getDefaultSavePath() {
	Common::String saveRelPath;

	const char *prefix = getenv("HOME");
	if (prefix == nullptr) {
		return Common::Path();
	}

	saveRelPath = ".local/share/" + getAppSuffix() + "/saves";

	if (!Posix::assureDirectoryExists(saveRelPath, prefix)) {
		return Common::Path();
	}

	Common::Path savePath(prefix);
	savePath.joinInPlace(saveRelPath);

	return savePath;
}

Common::Path OSystem_SDL_Sailfish::getDefaultConfigFileName() {
	Common::String configPath;

	const char *prefix = getenv("HOME");
	if (prefix == nullptr) {
		return Common::Path();
	}

	configPath = ".local/share/" + getAppSuffix();

	if (!Posix::assureDirectoryExists(configPath, prefix)) {
		return Common::Path();
	}

	Common::Path configFile(prefix);
	configFile.joinInPlace(configPath);
	configFile.joinInPlace("scummvm.ini");

	return configFile;
}

Common::Path OSystem_SDL_Sailfish::getDefaultLogFileName() {
	Common::String logFile;

	const char *prefix = getenv("HOME");
	if (prefix == nullptr) {
		return Common::Path();
	}

	logFile = ".cache/" + getAppSuffix() + "/logs";

	if (!Posix::assureDirectoryExists(logFile, prefix)) {
		return Common::Path();
	}

	Common::Path logPath(prefix);
	logPath.joinInPlace(logFile);
	logPath.joinInPlace("scummvm.log");

	return logPath;
}


bool OSystem_SDL_Sailfish::hasFeature(Feature f) {
	switch (f) {
	case kFeatureFullscreenMode:
		return false;
	case kFeatureTouchpadMode:
		return true;
	default:
		return OSystem_SDL::hasFeature(f);
	}
}
