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


// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
// Also with clock() in sys/time.h in some macOS SDKs.
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	//On IRIX, sys/stat.h includes sys/time.h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#include "common/scummsys.h"

#if defined(POSIX) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#include "backends/saves/posix/posix-saves.h"
#include "backends/fs/posix/posix-fs.h"
#if defined(MACOSX)
#include "backends/platform/sdl/macosx/macosx_wrapper.h"
#endif

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/textconsole.h"

#include <sys/stat.h>

POSIXSaveFileManager::POSIXSaveFileManager() {
	// Register default savepath.
	Common::Path savePath;

#if defined(MACOSX)
	savePath = getAppSupportPathMacOSX();
	if (!savePath.empty()) {
		savePath.joinInPlace("Savegames");
		ConfMan.registerDefault("savepath", savePath);
	}
#else
	const char *envVar;

	// Previously we placed our default savepath in HOME. If the directory
	// still exists, we will use it for backwards compatibility.
	envVar = getenv("HOME");
	if (envVar && *envVar) {
		savePath = envVar;
		savePath.joinInPlace("/.scummvm");

		struct stat sb;
		if (stat(savePath.toString(Common::Path::kNativeSeparator).c_str(), &sb) != 0 || !S_ISDIR(sb.st_mode)) {
			savePath.clear();
		}
	}

	if (savePath.empty()) {
		Common::String prefix;

		// On POSIX systems we follow the XDG Base Directory Specification for
		// where to store files. The version we based our code upon can be found
		// over here: https://specifications.freedesktop.org/basedir-spec/basedir-spec-0.8.html
		envVar = getenv("XDG_DATA_HOME");
		if (!envVar || !*envVar) {
			envVar = getenv("HOME");
			if (envVar && *envVar) {
				prefix = envVar;
				savePath = Common::Path(".local/share");
			}
		} else {
			prefix = envVar;
		}

		// Our default save path is '$XDG_DATA_HOME/scummvm/saves'
		savePath.joinInPlace("scummvm/saves");

		if (!Posix::assureDirectoryExists(savePath.toString(Common::Path::kNativeSeparator), prefix.c_str())) {
			savePath.clear();
		} else {
			savePath = Common::Path(prefix).join(savePath);
		}
	}

	if (!savePath.empty() && savePath.toString(Common::Path::kNativeSeparator).size() < MAXPATHLEN) {
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
				ConfMan.setPath("savepath", dir, Common::ConfigManager::kTransientDomain);
			}
		}
	}
}

#endif
