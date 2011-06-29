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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef MACOSX

#include "backends/platform/sdl/macosx/macosx.h"
#include "backends/mixer/doublebuffersdl/doublebuffersdl-mixer.h"

#include "common/archive.h"
#include "common/fs.h"

#include "ApplicationServices/ApplicationServices.h"	// for LSOpenFSRef
#include "CoreFoundation/CoreFoundation.h"	// for CF* stuff
#include "CoreServices/CoreServices.h"	// for FSPathMakeRef

OSystem_MacOSX::OSystem_MacOSX()
	:
	OSystem_POSIX("Library/Preferences/ScummVM Preferences") {
}

void OSystem_MacOSX::initBackend() {
	// Create the mixer manager
	if (_mixer == 0) {
		_mixerManager = new DoubleBufferSDLMixerManager();

		// Setup and start mixer
		_mixerManager->init();
	}

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

void OSystem_MacOSX::setupIcon() {
	// Don't set icon on OS X, as we use a nicer external icon there.
}

bool OSystem_MacOSX::hasFeature(Feature f) {
	if (f == kFeatureDisplayLogFile)
		return true;
	return OSystem_POSIX::hasFeature(f);
}

bool OSystem_MacOSX::displayLogFile() {
	// Use LaunchServices to open the log file, if possible.

	if (_logFilePath.empty())
		return false;

	FSRef ref;
	OSStatus err;

	err = FSPathMakeRef((const UInt8 *)_logFilePath.c_str(), &ref, NULL);
	if (err == noErr) {
		err = LSOpenFSRef(&ref, NULL);
	}

	return err != noErr;
}

#endif
