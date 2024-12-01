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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef MACOSX

#include "backends/audiocd/macosx/macosx-audiocd.h"
#include "backends/platform/sdl/macosx/appmenu_osx.h"
#include "backends/platform/sdl/macosx/macosx.h"
#include "backends/platform/sdl/macosx/macosx-touchbar.h"
#include "backends/platform/sdl/macosx/macosx-window.h"
#include "backends/updates/macosx/macosx-updates.h"
#include "backends/taskbar/macosx/macosx-taskbar.h"
#include "backends/text-to-speech/macosx/macosx-text-to-speech.h"
#include "backends/dialogs/macosx/macosx-dialogs.h"
#include "backends/platform/sdl/macosx/macosx_wrapper.h"
#include "backends/fs/posix/posix-fs.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/translation.h"

#include <ApplicationServices/ApplicationServices.h>	// for LSOpenFSRef
#include <CoreFoundation/CoreFoundation.h>	// for CF* stuff

// For querying number of MIDI devices
#include <pthread.h>
#include <CoreMIDI/CoreMIDI.h>

void *coreMIDIthread(void *threadarg) {
	(void)MIDIGetNumberOfDestinations();

	pthread_exit(NULL);

	return NULL;
}

OSystem_MacOSX::~OSystem_MacOSX() {
	releaseMenu();

#if defined(USE_OSD)
	macOSTouchbarDestroy();
#endif
}

void OSystem_MacOSX::init() {
	initSDL();
	_window = new SdlWindow_MacOSX();

#if defined(USE_TASKBAR)
	// Initialize taskbar manager
	_taskbarManager = new MacOSXTaskbarManager();
#endif

#if defined(USE_SYSDIALOGS)
	// Initialize dialog manager
	_dialogManager = new MacOSXDialogManager();
#endif

#if defined(USE_OSD)
	macOSTouchbarCreate();
#endif

	// The call to query the number of MIDI devices is ubiquitously slow
	// on the first run. This is apparent when opening Options in GUI,
	// which takes 2-3 secs.
	//
	// Thus, we are launching it now, in a separate thread, so
	// the subsequent calls are instantaneous
	pthread_t thread;
	pthread_create(&thread, NULL, coreMIDIthread, NULL);

	// Invoke parent implementation of this method
	OSystem_POSIX::init();
}

void OSystem_MacOSX::initBackend() {
#ifdef USE_TRANSLATION
	// We need to initialize the translation manager here for the following
	// call to replaceApplicationMenuItems() work correctly
	TransMan.setLanguage(ConfMan.get("gui_language").c_str());
#endif // USE_TRANSLATION

	// Replace the SDL generated menu items with our own translated ones on macOS
	replaceApplicationMenuItems();

#ifdef USE_SPARKLE
	// Initialize updates manager
	_updateManager = new MacOSXUpdateManager();
#endif

#ifdef USE_TTS
	// Initialize Text to Speech manager
	_textToSpeechManager = new MacOSXTextToSpeechManager();
#endif

	// Migrate savepath.
	// It used to be in ~/Documents/ScummVM Savegames/, but was changed to use the application support
	// directory. To migrate old config files we use a flag to indicate if the config file was migrated.
	// This allows detecting old config files. If the flag is not set we:
	// 1. Set the flag
	// 2. If the config file has no custom savepath and has some games, we set the savepath to the old default.
	if (!ConfMan.hasKey("macos_savepath_migrated", Common::ConfigManager::kApplicationDomain)) {
		if (!ConfMan.hasKey("savepath", Common::ConfigManager::kApplicationDomain) && !ConfMan.getGameDomains().empty()) {
			ConfMan.set("savepath", getDocumentsPathMacOSX() + "/ScummVM Savegames", Common::ConfigManager::kApplicationDomain);
		}
		ConfMan.setBool("macos_savepath_migrated", true, Common::ConfigManager::kApplicationDomain);
		ConfMan.flushToDisk();
	}

	// Invoke parent implementation of this method
	OSystem_POSIX::initBackend();
}

#ifdef USE_OPENGL
OSystem_SDL::GraphicsManagerType OSystem_MacOSX::getDefaultGraphicsManager() const {
	return GraphicsManagerOpenGL;
}
#endif

void OSystem_MacOSX::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// Invoke parent implementation of this method
	OSystem_POSIX::addSysArchivesToSearchSet(s, priority);

	// Get URL of the Resource directory of the .app bundle
	Common::Path bundlePath(getResourceAppBundlePathMacOSX(), Common::Path::kNativeSeparator);
	if (!bundlePath.empty()) {
		// Success: search with a depth of 2 so the shaders are found
		s.add("__OSX_BUNDLE__", new Common::FSDirectory(bundlePath, 2), priority);
	}
}

bool OSystem_MacOSX::hasFeature(Feature f) {
	if (f == kFeatureDisplayLogFile || f == kFeatureClipboardSupport || f == kFeatureOpenUrl)
		return true;

#ifdef USE_SYSDIALOGS
	if (f == kFeatureSystemBrowserDialog)
		return true;
#endif

	return OSystem_POSIX::hasFeature(f);
}

bool OSystem_MacOSX::displayLogFile() {
	// Use LaunchServices to open the log file, if possible.

	if (_logFilePath.empty())
		return false;

	Common::String logFilePath(_logFilePath.toString(Common::Path::kNativeSeparator));

	CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8 *)logFilePath.c_str(), logFilePath.size(), false);
	OSStatus err = LSOpenCFURLRef(url, NULL);
	CFRelease(url);

	return err != noErr;
}

bool OSystem_MacOSX::openUrl(const Common::String &url) {
	CFURLRef urlRef = CFURLCreateWithBytes (NULL, (const UInt8*)url.c_str(), url.size(), kCFStringEncodingASCII, NULL);
	OSStatus err = LSOpenCFURLRef(urlRef, NULL);
	CFRelease(urlRef);
	return err == noErr;
}

Common::String OSystem_MacOSX::getSystemLanguage() const {
#if defined(USE_DETECTLANG) && defined(USE_TRANSLATION)
	CFArrayRef availableLocalizations = CFBundleCopyBundleLocalizations(CFBundleGetMainBundle());
	if (availableLocalizations) {
		CFArrayRef preferredLocalizations = CFBundleCopyPreferredLocalizationsFromArray(availableLocalizations);
		CFRelease(availableLocalizations);
		if (preferredLocalizations) {
			CFIndex localizationsSize = CFArrayGetCount(preferredLocalizations);
			// Since we have a list of sorted preferred localization, I would like here to
			// check that they are supported by the TranslationManager and take the first
			// one that is supported. The listed localizations are taken from the Bundle
			// plist file, so they should all be supported, unless the plist file is not
			// synchronized with the translations.dat file. So this is not really a big
			// issue. And because getSystemLanguage() is called from the constructor of
			// TranslationManager (therefore before the instance pointer is set), calling
			// TransMan here results in an infinite loop and creation of a lot of TransMan
			// instances.
			/*
			for (CFIndex i = 0 ; i < localizationsSize ; ++i) {
				CFStringRef language = (CFStringRef)CFArrayGetValueAtIndex(preferredLocalizations, i);
				char buffer[10];
				CFStringGetCString(language, buffer, sizeof(buffer), kCFStringEncodingASCII);
				int32 languageId = TransMan.findMatchingLanguage(buffer);
				if (languageId != -1) {
					CFRelease(preferredLocalizations);
					return TransMan.getLangById(languageId);
				}
			}
			*/
			if (localizationsSize > 0) {
				CFStringRef language = (CFStringRef)CFArrayGetValueAtIndex(preferredLocalizations, 0);
				char buffer[10];
				CFStringGetCString(language, buffer, sizeof(buffer), kCFStringEncodingASCII);
				CFRelease(preferredLocalizations);
				return buffer;
			}
			CFRelease(preferredLocalizations);
		}

	}
	// Fallback to POSIX implementation
	return OSystem_POSIX::getSystemLanguage();
#else // USE_DETECTLANG
	return OSystem_POSIX::getSystemLanguage();
#endif // USE_DETECTLANG
}

Common::Path OSystem_MacOSX::getDefaultConfigFileName() {
	const Common::String baseConfigName = "Library/Preferences/" + getMacBundleName() + " Preferences";

	Common::Path configFile;

	Common::String prefix = getenv("HOME");

	if (!prefix.empty() && (prefix.size() + 1 + baseConfigName.size()) < MAXPATHLEN) {
		configFile = prefix;
		configFile.joinInPlace(baseConfigName);
	} else {
		configFile = baseConfigName;
	}

	return configFile;
}

Common::Path OSystem_MacOSX::getDefaultLogFileName() {
	const char *prefix = getenv("HOME");
	if (prefix == nullptr) {
		return Common::Path();
	}

	if (!Posix::assureDirectoryExists("Library/Logs", prefix)) {
		return Common::Path();
	}

	Common::String appName = getMacBundleName();
	appName.toLowercase();
	return Common::Path(prefix).join(Common::String("Library/Logs/") + appName + ".log");
}

Common::Path OSystem_MacOSX::getDefaultIconsPath() {
	const Common::String defaultIconsPath = getAppSupportPathMacOSX() + "/Icons";

	if (!Posix::assureDirectoryExists(defaultIconsPath)) {
		return Common::Path();
	}

	return Common::Path(defaultIconsPath);
}

Common::Path OSystem_MacOSX::getDefaultDLCsPath() {
	const Common::Path defaultDLCsPath(getAppSupportPathMacOSX() + "/DLCs");

	if (!Posix::assureDirectoryExists(defaultDLCsPath.toString(Common::Path::kNativeSeparator))) {
		return Common::Path();
	}

	return defaultDLCsPath;
}

Common::Path OSystem_MacOSX::getScreenshotsPath() {
	// If the user has configured a screenshots path, use it
	const Common::Path path = OSystem_SDL::getScreenshotsPath();
	if (!path.empty())
		return path;

	Common::Path desktopPath(getDesktopPathMacOSX(), Common::Path::kNativeSeparator);
	return desktopPath;
}

AudioCDManager *OSystem_MacOSX::createAudioCDManager() {
	return createMacOSXAudioCDManager();
}

#endif
