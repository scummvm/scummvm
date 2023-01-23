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


#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "common/scummsys.h"

#if !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#include "backends/saves/kolibrios/kolibrios-saves.h"
#include "backends/fs/kolibrios/kolibrios-fs.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/textconsole.h"

#include <sys/stat.h>

KolibriOSSaveFileManager::KolibriOSSaveFileManager(const Common::Path& writeablePath) {
	// Register default savepath.
	Common::String savePath;

	if (KolibriOS::assureDirectoryExists("saves", writeablePath.toString().c_str())) {
		savePath = writeablePath.join("saves").toString().c_str();
	}

	if (!savePath.empty() && savePath.size() < MAXPATHLEN) {
		ConfMan.registerDefault("savepath", savePath);
	}

	// The user can override the savepath with the SCUMMVM_SAVEPATH
	// environment variable. This is weaker than a --savepath on the
	// command line, but overrides the default savepath.
	//
	// To ensure that the command line option (if given) has precedence,
	// we only set the value in the transient domain if it is not
	// yet present there.
	if (!ConfMan.hasKey("savepath", Common::ConfigManager::kTransientDomain)) {
		const char *dir = getenv("SCUMMVM_SAVEPATH");
		if (dir && *dir && strlen(dir) < MAXPATHLEN) {
			Common::FSNode saveDir(dir);
			if (!saveDir.exists()) {
				warning("Ignoring non-existent SCUMMVM_SAVEPATH '%s'", dir);
			} else if (!saveDir.isWritable()) {
				warning("Ignoring non-writable SCUMMVM_SAVEPATH '%s'", dir);
			} else {
				ConfMan.set("savepath", dir, Common::ConfigManager::kTransientDomain);
			}
		}
	}
}

#endif
