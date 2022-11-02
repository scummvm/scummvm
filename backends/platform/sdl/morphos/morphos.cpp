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

void OSystem_MorphOS::init() {
	// Initialze File System Factory
	_fsFactory = new MorphOSFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();

#if defined(USE_SYSDIALOGS)
	_dialogManager = new MorphosDialogManager();
#endif
}

bool OSystem_MorphOS::hasFeature(Feature f) {
	if (f == kFeatureOpenUrl)
		return true;

#if defined(USE_SYSDIALOGS)
	if (f == kFeatureSystemBrowserDialog)
		return true;
#endif

	return OSystem_SDL::hasFeature(f);
}

void OSystem_MorphOS::logMessage(LogMessageType::Type type, const char * message) {
#ifdef DEBUG_BUILD
	printf("%s\n", message);
#endif
}
#endif
