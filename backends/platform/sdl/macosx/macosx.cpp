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

#ifdef MACOSX

#include "backends/platform/sdl/macosx/macosx.h"
#include "backends/mixer/bufferingsdl/bufferingsdl-mixer.h"

#include "CoreFoundation/CoreFoundation.h"

#define DEFAULT_CONFIG_FILE "Library/Preferences/ScummVM Preferences"

OSystem_MacOSX::OSystem_MacOSX() {
}

void OSystem_MacOSX::initBackend() {
	// Create the mixer manager
	if (_mixer == 0) {
		_mixerManager = new BufferingSDLMixerManager();

		// Setup and start mixer
		_mixerManager->init();
	}

	// Invoke parent implementation of this method
	OSystem_POSIX::initBackend();
}

Common::String OSystem_MacOSX::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];

	// On UNIX type systems, by default we store the config file inside
	// to the HOME directory of the user.
	const char *home = getenv("HOME");
	if (home != NULL && strlen(home) < MAXPATHLEN)
		snprintf(configFile, MAXPATHLEN, "%s/%s", home, DEFAULT_CONFIG_FILE);
	else
		strcpy(configFile, DEFAULT_CONFIG_FILE);

	return configFile;
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

#endif
