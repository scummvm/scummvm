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

#include "common/scummsys.h"

#ifdef __MORPHOS__
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/tasks.h>
#include <dos/dosextens.h>

#include "backends/platform/sdl/morphos/morphos.h"
#include "backends/fs/morphos/morphos-fs-factory.h"
#include "backends/dialogs/morphos/morphos-dialogs.h"

static bool cleanupDone = false;

static Common::Path getProcessDirectoryPath(const char *name, bool directory) {
	char path[1024];

	struct Process *process = (struct Process *)FindTask(NULL);

	if (!process || !process->pr_HomeDir) {
		Common::String fallback = Common::String::format("PROGDIR:%s", name);
		if (directory && !fallback.hasSuffix("/"))
			fallback += "/";
		return Common::Path(fallback);
	}

	if (!NameFromLock(process->pr_HomeDir, path, sizeof(path)))
		return Common::Path(Common::String::format("PROGDIR:%s", name));

	if (!AddPart(path, name, sizeof(path)))
		return Common::Path(Common::String::format("PROGDIR:%s", name));

	Common::String result(path);
	if (directory && !result.hasSuffix("/") && !result.hasSuffix(":"))
		result += "/";

	return Common::Path(result);
}

static void cleanup() {
	if (!cleanupDone && g_system)
		g_system->destroy();
}

OSystem_MorphOS::~OSystem_MorphOS() {
	cleanupDone = true;
}

void OSystem_MorphOS::init() {
	// Register cleanup function to avoid unfreed signals
	if (atexit(cleanup))
		warning("Failed to register cleanup function via atexit()");

	// Initialze File System Factory
	_fsFactory = new MorphOSFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();

#if defined(USE_SYSDIALOGS)
	_dialogManager = new MorphosDialogManager();
#endif
}

bool OSystem_MorphOS::hasFeature(Feature f) {

#if defined(USE_SYSDIALOGS)
	if (f == kFeatureSystemBrowserDialog)
		return true;
#endif

	return OSystem_SDL::hasFeature(f);
}

static Common::Path getProcessDirectoryFile(const char *file) {
	return getProcessDirectoryPath(file, false);
}

Common::Path OSystem_MorphOS::getDefaultConfigFileName() {
	return getProcessDirectoryFile("scummvm.ini");
}

static void assureDrawerExists(const Common::Path &path) {
	Common::String nativePath = path.toString(Common::Path::kNativeSeparator);

	if (nativePath.size() > 0 && nativePath.lastChar() == '/')
		nativePath.deleteLastChar();

	BPTR lock = Lock(nativePath.c_str(), ACCESS_READ);
	if (lock) {
		UnLock(lock);
		return;
	}

	BPTR created = CreateDir(nativePath.c_str());
	if (created)
		UnLock(created);
}

void OSystem_MorphOS::initBackend() {
	const Common::Path savePath = getProcessDirectoryPath("saves", true);
	const Common::Path extraPath = getProcessDirectoryPath("extras", true);
	const Common::Path themePath = getProcessDirectoryPath("themes", true);
	const Common::Path iconPath = getProcessDirectoryPath("icons", true);

	assureDrawerExists(savePath);

	ConfMan.registerDefault("audio_buffer_size", "2048");
	ConfMan.registerDefault("savepath", savePath);
	ConfMan.registerDefault("extrapath", extraPath);
	ConfMan.registerDefault("themepath", themePath);
	ConfMan.registerDefault("iconspath", iconPath);

	if (!ConfMan.hasKey("audio_buffer_size"))
		ConfMan.set("audio_buffer_size", "2048");

	if (!ConfMan.hasKey("savepath"))
		ConfMan.setPath("savepath", savePath);

	if (!ConfMan.hasKey("extrapath"))
		ConfMan.setPath("extrapath", extraPath);

	if (!ConfMan.hasKey("themepath"))
		ConfMan.setPath("themepath", themePath);

	if (!ConfMan.hasKey("iconspath"))
		ConfMan.setPath("iconspath", iconPath);

	OSystem_SDL::initBackend();
}

void OSystem_MorphOS::logMessage(LogMessageType::Type type, const char * message) {
#ifdef DEBUG_BUILD
	OSystem_SDL::logMessage(type, message);
#endif
}
#endif
