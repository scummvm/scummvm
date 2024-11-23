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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#include "common/scummsys.h"

#ifdef __MORPHOS__

#include "backends/platform/sdl/morphos/morphos.h"
#include "backends/fs/morphos/morphos-fs-factory.h"
#include "backends/dialogs/morphos/morphos-dialogs.h"

static bool cleanupDone = false;

static void cleanup() {
	if (!cleanupDone)
		g_system->destroy();
}

OSystem_MorphOS::~OSystem_MorphOS() {
	cleanupDone = true;
}

void OSystem_MorphOS::init() {
	// Register cleanup function to avoid unfreed signals
	if (atexit(cleanup))
		warning("Failed to register cleanup function via atexit()");

	// Initialze File System Factory
	_fsFactory = new MorphOSFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();

#if defined(USE_SYSDIALOGS)
	_dialogManager = new MorphosDialogManager();
#endif
}

bool OSystem_MorphOS::hasFeature(Feature f) {

#if defined(USE_SYSDIALOGS)
	if (f == kFeatureSystemBrowserDialog)
		return true;
#endif

	return OSystem_SDL::hasFeature(f);
}

void OSystem_MorphOS::initBackend() {

	// First time user defaults
	ConfMan.registerDefault("audio_buffer_size", "2048");
	ConfMan.registerDefault("extrapath", Common::Path("PROGDIR:extras/"));
	ConfMan.registerDefault("savepath", Common::Path("PROGDIR:saves/"));
	ConfMan.registerDefault("themepath", Common::Path("PROGDIR:themes/"));
	// First time .ini defaults
	if (!ConfMan.hasKey("audio_buffer_size")) {
		ConfMan.set("audio_buffer_size", "2048");
	}
	if (!ConfMan.hasKey("extrapath")) {
		ConfMan.setPath("extrapath", "PROGDIR:extras/");
	}
	if (!ConfMan.hasKey("savepath")) {
		ConfMan.setPath("savepath", "PROGDIR:saves/");
	}
	if (!ConfMan.hasKey("themepath")) {
		ConfMan.setPath("themepath", "PROGDIR:themes/");
	}
	OSystem_SDL::initBackend();
}

void OSystem_MorphOS::logMessage(LogMessageType::Type type, const char * message) {
#ifdef DEBUG_BUILD
	printf("%s\n", message);
#endif
}
#endif
