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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef MACOSX

#include "backends/audiocd/macosx/macosx-audiocd.h"
#include "backends/platform/sdl/macosx/appmenu_osx.h"
#include "backends/platform/sdl/macosx/macosx.h"
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

#include "ApplicationServices/ApplicationServices.h"	// for LSOpenFSRef
#include "CoreFoundation/CoreFoundation.h"	// for CF* stuff

OSystem_MacOSX::OSystem_MacOSX()
	:
	OSystem_POSIX("Library/Preferences/ScummVM Preferences") {
}

OSystem_MacOSX::~OSystem_MacOSX() {
	releaseMenu();
}

void OSystem_MacOSX::init() {
	// Use an iconless window on OS X, as we use a nicer external icon there.
	_window = new SdlIconlessWindow();

#if defined(USE_TASKBAR)
	// Initialize taskbar manager
	_taskbarManager = new MacOSXTaskbarManager();
#endif

#if defined(USE_SYSDIALOGS)
	// Initialize dialog manager
	_dialogManager = new MacOSXDialogManager();
#endif

	// Invoke parent implementation of this method
	OSystem_POSIX::init();
}

void OSystem_MacOSX::initBackend() {
#ifdef USE_TRANSLATION
	// We need to initialize the translataion manager here for the following
	// call to replaceApplicationMenuItems() work correctly
	TransMan.setLanguage(ConfMan.get("gui_language").c_str());
#endif // USE_TRANSLATION

	// Replace the SDL generated menu items with our own translated ones on Mac OS X
	replaceApplicationMenuItems();

#ifdef USE_SPARKLE
	// Initialize updates manager
	_updateManager = new MacOSXUpdateManager();
#endif

#ifdef USE_TTS
	// Initialize Text to Speech manager
	_textToSpeechManager = new MacOSXTextToSpeechManager();
#endif

	// Invoke parent implementation of this method
	OSystem_POSIX::initBackend();
}

void OSystem_MacOSX::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// Invoke parent implementation of this method
	OSystem_POSIX::addSysArchivesToSearchSet(s, priority);

	// Get URL of the Resource directory of the .app bundle
	CFURLRef fileUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	if (fileUrl) {
		// Try to convert the URL to an absolute path
		UInt8 buf[MAXPATHLEN];
		if (CFURLGetFileSystemRepresentation(fileUrl, true, buf, sizeof(buf))) {
			// Success: Add it to the search path
			Common::String bundlePath((const char *)buf);
			s.add("__OSX_BUNDLE__", new Common::FSDirectory(bundlePath), priority);
		}
		CFRelease(fileUrl);
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

    CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8 *)_logFilePath.c_str(), _logFilePath.size(), false);
    OSStatus err = LSOpenCFURLRef(url, NULL);
    CFRelease(url);

	return err != noErr;
}

bool OSystem_MacOSX::hasTextInClipboard() {
	return hasTextInClipboardMacOSX();
}

Common::String OSystem_MacOSX::getTextFromClipboard() {
	return getTextFromClipboardMacOSX();
}

bool OSystem_MacOSX::setTextInClipboard(const Common::String &text) {
	return setTextInClipboardMacOSX(text);
}

bool OSystem_MacOSX::openUrl(const Common::String &url) {
	CFURLRef urlRef = CFURLCreateWithBytes (NULL, (UInt8*)url.c_str(), url.size(), kCFStringEncodingASCII, NULL);
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
	// Falback to POSIX implementation
	return OSystem_POSIX::getSystemLanguage();
#else // USE_DETECTLANG
	return OSystem_POSIX::getSystemLanguage();
#endif // USE_DETECTLANG
}

Common::String OSystem_MacOSX::getDefaultLogFileName() {
	const char *prefix = getenv("HOME");
	if (prefix == nullptr) {
		return Common::String();
	}

	if (!Posix::assureDirectoryExists("Library/Logs", prefix)) {
		return Common::String();
	}

	return Common::String(prefix) + "/Library/Logs/scummvm.log";
}

Common::String OSystem_MacOSX::getScreenshotsPath() {
	Common::String path = ConfMan.get("screenshotpath");
	if (path.empty())
		path = getDesktopPathMacOSX();
	if (!path.empty() && !path.hasSuffix("/"))
		path += "/";
	return path;
}

AudioCDManager *OSystem_MacOSX::createAudioCDManager() {
	return createMacOSXAudioCDManager();
}

#endif
