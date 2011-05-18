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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	//On IRIX, sys/stat.h includes sys/time.h

#include "common/scummsys.h"

#ifdef UNIX

#include "backends/platform/sdl/posix/posix.h"
#include "backends/saves/posix/posix-saves.h"
#include "backends/fs/posix/posix-fs-factory.h"

#include <errno.h>
#include <sys/stat.h>


OSystem_POSIX::OSystem_POSIX(Common::String baseConfigName)
	:
	_baseConfigName(baseConfigName) {
}

void OSystem_POSIX::init() {
	// Initialze File System Factory
	_fsFactory = new POSIXFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_POSIX::initBackend() {
	// Create the savefile manager
	if (_savefileManager == 0)
		_savefileManager = new POSIXSaveFileManager();

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

Common::String OSystem_POSIX::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];

	// On UNIX type systems, by default we store the config file inside
	// to the HOME directory of the user.
	const char *home = getenv("HOME");
	if (home != NULL && strlen(home) < MAXPATHLEN)
		snprintf(configFile, MAXPATHLEN, "%s/%s", home, _baseConfigName.c_str());
	else
		strcpy(configFile, _baseConfigName.c_str());

	return configFile;
}

Common::WriteStream *OSystem_POSIX::createLogFile() {
	const char *home = getenv("HOME");
	if (home == NULL)
		return 0;

	Common::String logFile(home);
#ifdef MACOSX
	logFile += "/Library";
#else
	logFile += "/.residual";
#endif

	struct stat sb;

	// Check whether the dir exists
	if (stat(logFile.c_str(), &sb) == -1) {
		// The dir does not exist, or stat failed for some other reason.
		if (errno != ENOENT)
			return 0;

		// If the problem was that the path pointed to nothing, try
		// to create the dir.
		if (mkdir(logFile.c_str(), 0755) != 0)
			return 0;
	} else if (!S_ISDIR(sb.st_mode)) {
		// Path is no directory. Oops
		return 0;
	}

#ifdef MACOSX
	logFile += "/Logs";
#else
	logFile += "/logs";
#endif

	// Check whether the dir exists
	if (stat(logFile.c_str(), &sb) == -1) {
		// The dir does not exist, or stat failed for some other reason.
		if (errno != ENOENT)
			return 0;

		// If the problem was that the path pointed to nothing, try
		// to create the dir.
		if (mkdir(logFile.c_str(), 0755) != 0)
			return 0;
	} else if (!S_ISDIR(sb.st_mode)) {
		// Path is no directory. Oops
		return 0;
	}

	logFile += "/residual.log";

	Common::FSNode file(logFile);
	return file.createWriteStream();
}

#endif
