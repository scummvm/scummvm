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


// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
// Also with clock() in sys/time.h in some Mac OS X SDKs.
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	//On IRIX, sys/stat.h includes sys/time.h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#include "common/scummsys.h"

#if defined(POSIX) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#include "backends/saves/posix/posix-saves.h"
#include "backends/fs/posix/posix-fs.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/textconsole.h"

#include <sys/stat.h>

POSIXSaveFileManager::POSIXSaveFileManager() {
	// Register default savepath.
	Common::String savePath;

#if defined(MACOSX)
	const char *home = getenv("HOME");
	if (home && *home && strlen(home) < MAXPATHLEN) {
		savePath = home;
		savePath += "/Documents/ScummVM Savegames";

		ConfMan.registerDefault("savepath", savePath);
	}

#else
	const char *envVar;

	// Previously we placed our default savepath in HOME. If the directory
	// still exists, we will use it for backwards compatability.
	envVar = getenv("HOME");
	if (envVar && *envVar) {
		savePath = envVar;
		savePath += "/.scummvm";

		struct stat sb;
		if (stat(savePath.c_str(), &sb) != 0 || !S_ISDIR(sb.st_mode)) {
			savePath.clear();
		}
	}

	if (savePath.empty()) {
		Common::String prefix;

		// On POSIX systems we follow the XDG Base Directory Specification for
		// where to store files. The version we based our code upon can be found
		// over here: http://standards.freedesktop.org/basedir-spec/basedir-spec-0.8.html
		envVar = getenv("XDG_DATA_HOME");
		if (!envVar || !*envVar) {
			envVar = getenv("HOME");
			if (envVar && *envVar) {
				prefix = envVar;
				savePath = ".local/share/";
			}
		} else {
			prefix = envVar;
		}

		// Our default save path is '$XDG_DATA_HOME/scummvm/saves'
		savePath += "scummvm/saves";

		if (!Posix::assureDirectoryExists(savePath, prefix.c_str())) {
			savePath.clear();
		} else {
			savePath = prefix + '/' + savePath;
		}
	}

	if (!savePath.empty() && savePath.size() < MAXPATHLEN) {
		ConfMan.registerDefault("savepath", savePath);
	}
#endif

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
