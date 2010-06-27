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
 * $URL$
 * $Id$
 *
 */

#ifdef UNIX

#include "backends/platform/sdl/posix/posix.h"
#include "backends/saves/posix/posix-saves.h"
#include "backends/fs/posix/posix-fs-factory.h"

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
		snprintf(configFile, MAXPATHLEN, "%s/%s", home, _baseConfigName);
	else
		strcpy(configFile, _baseConfigName);

	return configFile;
}

#endif
