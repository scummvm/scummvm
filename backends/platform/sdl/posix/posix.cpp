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

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	//On IRIX, sys/stat.h includes sys/time.h
#define FORBIDDEN_SYMBOL_EXCEPTION_system
#define FORBIDDEN_SYMBOL_EXCEPTION_random
#define FORBIDDEN_SYMBOL_EXCEPTION_srandom

#include "common/scummsys.h"

#ifdef POSIX

#include "backends/platform/sdl/posix/posix.h"
#include "backends/saves/posix/posix-saves.h"
#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"
#include "backends/taskbar/unity/unity-taskbar.h"

#ifdef USE_LINUXCD
#include "backends/audiocd/linux/linux-audiocd.h"
#endif

#include "common/textconsole.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef HAS_POSIX_SPAWN
#include <spawn.h>
#endif

#if defined(USE_SPEECH_DISPATCHER) && defined(USE_TTS)
#include "backends/text-to-speech/linux/linux-text-to-speech.h"
#endif
extern char **environ;

OSystem_POSIX::OSystem_POSIX(Common::String baseConfigName)
	:
	_baseConfigName(baseConfigName) {
}

void OSystem_POSIX::init() {
	// Initialze File System Factory
	_fsFactory = new POSIXFilesystemFactory();

#if defined(USE_TASKBAR) && defined(USE_UNITY)
	// Initialize taskbar manager
	_taskbarManager = new UnityTaskbarManager();
#endif

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_POSIX::initBackend() {
	// Create the savefile manager
	if (_savefileManager == 0)
		_savefileManager = new POSIXSaveFileManager();

#if defined(USE_SPEECH_DISPATCHER) && defined(USE_TTS)
	// Initialize Text to Speech manager
	_textToSpeechManager = new SpeechDispatcherManager();
#endif

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();

#if defined(USE_TASKBAR) && defined(USE_UNITY)
	// Register the taskbar manager as an event source (this is necessary for the glib event loop to be run)
	_eventManager->getEventDispatcher()->registerSource((UnityTaskbarManager *)_taskbarManager, false);
#endif
}

bool OSystem_POSIX::hasFeature(Feature f) {
	if (f == kFeatureDisplayLogFile)
		return true;
#ifdef HAS_POSIX_SPAWN
	if (f == kFeatureOpenUrl)
		return true;
#endif
	return OSystem_SDL::hasFeature(f);
}

Common::String OSystem_POSIX::getDefaultConfigFileName() {
	Common::String configFile;

	Common::String prefix;
#ifdef MACOSX
	prefix = getenv("HOME");
#elif !defined(SAMSUNGTV)
	const char *envVar;
	// Our old configuration file path for POSIX systems was ~/.scummvmrc.
	// If that file exists, we still use it.
	envVar = getenv("HOME");
	if (envVar && *envVar) {
		configFile = envVar;
		configFile += '/';
		configFile += ".scummvmrc";

		if (configFile.size() < MAXPATHLEN) {
			struct stat sb;
			if (stat(configFile.c_str(), &sb) == 0) {
				return configFile;
			}
		}
	}

	// On POSIX systems we follow the XDG Base Directory Specification for
	// where to store files. The version we based our code upon can be found
	// over here: http://standards.freedesktop.org/basedir-spec/basedir-spec-0.8.html
	envVar = getenv("XDG_CONFIG_HOME");
	if (!envVar || !*envVar) {
		envVar = getenv("HOME");
		if (!envVar) {
			return 0;
		}

		if (Posix::assureDirectoryExists(".config", envVar)) {
			prefix = envVar;
			prefix += "/.config";
		}
	} else {
		prefix = envVar;
	}

	if (!prefix.empty() && Posix::assureDirectoryExists("scummvm", prefix.c_str())) {
		prefix += "/scummvm";
	}
#endif

	if (!prefix.empty() && (prefix.size() + 1 + _baseConfigName.size()) < MAXPATHLEN) {
		configFile = prefix;
		configFile += '/';
		configFile += _baseConfigName;
	} else {
		configFile = _baseConfigName;
	}

	return configFile;
}

Common::String OSystem_POSIX::getXdgUserDir(const char *name) {
	// The xdg-user-dirs configuration path is stored in the XDG config
	// home directory. We start by retrieving this value.
	Common::String configHome = getenv("XDG_CONFIG_HOME");
	if (configHome.empty()) {
		const char *home = getenv("HOME");
		if (!home) {
			return "";
		}

		configHome = Common::String::format("%s/.config", home);
	}

	// Find the requested directory line in the xdg-user-dirs configuration file
	//   Example line value: XDG_PICTURES_DIR="$HOME/Pictures"
	Common::FSNode userDirsFile(configHome + "/user-dirs.dirs");
	if (!userDirsFile.exists() || !userDirsFile.isReadable() || userDirsFile.isDirectory()) {
		return "";
	}

	Common::SeekableReadStream *userDirsStream = userDirsFile.createReadStream();
	if (!userDirsStream) {
		return "";
	}

	Common::String dirLinePrefix = Common::String::format("XDG_%s_DIR=", name);

	Common::String directoryValue;
	while (!userDirsStream->eos() && !userDirsStream->err()) {
		Common::String userDirsLine = userDirsStream->readLine();
		userDirsLine.trim();

		if (userDirsLine.hasPrefix(dirLinePrefix)) {
			directoryValue = Common::String(userDirsLine.c_str() + dirLinePrefix.size());
			break;
		}
	}

	delete userDirsStream;

	// Extract the path from the value
	//   Example value: "$HOME/Pictures"
	if (directoryValue.empty() || directoryValue[0] != '"') {
		return "";
	}

	if (directoryValue[directoryValue.size() - 1] != '"') {
		return "";
	}

	// According to the spec the value is shell-escaped, and would need to be
	// unescaped to be used, but neither the GTK+ nor the Qt implementation seem to
	// properly perform that step, it's probably fine if we don't do it either.
	Common::String directoryPath(directoryValue.c_str() + 1, directoryValue.size() - 2);

	if (directoryPath.hasPrefix("$HOME/")) {
		const char *home = getenv("HOME");
		directoryPath = Common::String::format("%s%s", home, directoryPath.c_str() + 5);
	}

	// At this point, the path must be absolute
	if (directoryPath.empty() || directoryPath[0] != '/') {
		return "";
	}

	return directoryPath;
}

Common::String OSystem_POSIX::getScreenshotsPath() {
	// If the user has configured a screenshots path, use it
	const Common::String path = OSystem_SDL::getScreenshotsPath();
	if (!path.empty()) {
		return path;
	}

	// Otherwise, the default screenshots path is the "ScummVM Screenshots"
	// directory in the XDG "Pictures" user directory, as defined in the
	// xdg-user-dirs spec: https://www.freedesktop.org/wiki/Software/xdg-user-dirs/
	Common::String picturesPath = getXdgUserDir("PICTURES");
	if (picturesPath.empty()) {
		return "";
	}

	if (!picturesPath.hasSuffix("/")) {
		picturesPath += "/";
	}

	static const char *SCREENSHOTS_DIR_NAME = "ScummVM Screenshots";
	if (!Posix::assureDirectoryExists(SCREENSHOTS_DIR_NAME, picturesPath.c_str())) {
		return "";
	}

	return picturesPath + SCREENSHOTS_DIR_NAME + "/";
}

void OSystem_POSIX::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
#ifdef DATA_PATH
	const char *snap = getenv("SNAP");
	if (snap) {
		Common::String dataPath = Common::String(snap) + DATA_PATH;
		Common::FSNode dataNode(dataPath);
		if (dataNode.exists() && dataNode.isDirectory()) {
			// This is the same priority which is used for the data path (below),
			// but we insert this one first, so it will be searched first.
			s.add(dataPath, new Common::FSDirectory(dataNode, 4), priority);
		}
	}
#endif

	// For now, we always add the data path, just in case SNAP doesn't make sense.
	OSystem_SDL::addSysArchivesToSearchSet(s, priority);
}

Common::String OSystem_POSIX::getDefaultLogFileName() {
	Common::String logFile;

	// On POSIX systems we follow the XDG Base Directory Specification for
	// where to store files. The version we based our code upon can be found
	// over here: http://standards.freedesktop.org/basedir-spec/basedir-spec-0.8.html
	const char *prefix = getenv("XDG_CACHE_HOME");
	if (prefix == nullptr || !*prefix) {
		prefix = getenv("HOME");
		if (prefix == nullptr) {
			return Common::String();
		}

		logFile = ".cache/";
	}

	logFile += "scummvm/logs";

	if (!Posix::assureDirectoryExists(logFile, prefix)) {
		return Common::String();
	}

	return Common::String::format("%s/%s/scummvm.log", prefix, logFile.c_str());
}

bool OSystem_POSIX::displayLogFile() {
	if (_logFilePath.empty())
		return false;

	// FIXME: This may not work perfectly when in fullscreen mode.
	// On my system it drops from fullscreen without ScummVM noticing,
	// so the next Alt-Enter does nothing, going from windowed to windowed.
	// (wjp, 20110604)

	pid_t pid = fork();
	if (pid < 0) {
		// failed to fork
		return false;
	} else if (pid == 0) {

		// Try xdg-open first
		execlp("xdg-open", "xdg-open", _logFilePath.c_str(), (char *)0);

		// If we're here, that clearly failed.

		// TODO: We may also want to try detecting the case where
		// xdg-open is successfully executed but returns an error code.

		// Try xterm+less next

		execlp("xterm", "xterm", "-e", "less", _logFilePath.c_str(), (char *)0);

		// TODO: If less does not exist we could fall back to 'more'.
		// However, we'll have to use 'xterm -hold' for that to prevent the
		// terminal from closing immediately (for short log files) or
		// unexpectedly.

		exit(127);
	}

	int status;
	// Wait for viewer to close.
	// (But note that xdg-open may have spawned a viewer in the background.)

	// FIXME: We probably want the viewer to always open in the background.
	// This may require installing a SIGCHLD handler.
	pid = waitpid(pid, &status, 0);

	if (pid < 0) {
		// Probably nothing sensible to do in this error situation
		return false;
	}

	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

bool OSystem_POSIX::openUrl(const Common::String &url) {
#ifdef HAS_POSIX_SPAWN
	// inspired by Qt's "qdesktopservices_x11.cpp"

	// try "standards"
	if (launchBrowser("xdg-open", url))
		return true;
	if (launchBrowser(getenv("DEFAULT_BROWSER"), url))
		return true;
	if (launchBrowser(getenv("BROWSER"), url))
		return true;

	// try desktop environment specific tools
	if (launchBrowser("gnome-open", url)) // gnome
		return true;
	if (launchBrowser("kfmclient", url)) // kde
		return true;
	if (launchBrowser("exo-open", url)) // xfce
		return true;

	// try browser names
	if (launchBrowser("firefox", url))
		return true;
	if (launchBrowser("mozilla", url))
		return true;
	if (launchBrowser("netscape", url))
		return true;
	if (launchBrowser("opera", url))
		return true;
	if (launchBrowser("chromium-browser", url))
		return true;
	if (launchBrowser("google-chrome", url))
		return true;

	warning("openUrl() (POSIX) failed to open URL");
	return false;
#else
	return false;
#endif
}

bool OSystem_POSIX::launchBrowser(const Common::String &client, const Common::String &url) {
#ifdef HAS_POSIX_SPAWN
	pid_t pid;
	const char *argv[] = {
		client.c_str(),
		url.c_str(),
		NULL,
		NULL
	};
	if (client == "kfmclient") {
		argv[2] = argv[1];
		argv[1] = "openURL";
	}
	if (posix_spawnp(&pid, client.c_str(), NULL, NULL, const_cast<char **>(argv), environ) != 0) {
		return false;
	}
	return (waitpid(pid, NULL, WNOHANG) != -1);
#else
	return false;
#endif
}

AudioCDManager *OSystem_POSIX::createAudioCDManager() {
#ifdef USE_LINUXCD
	return createLinuxAudioCDManager();
#else
	return OSystem_SDL::createAudioCDManager();
#endif
}

#endif
