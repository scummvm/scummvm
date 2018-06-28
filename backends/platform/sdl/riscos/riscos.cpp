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

#include "common/scummsys.h"

#ifdef RISCOS

#include "backends/platform/sdl/riscos/riscos.h"
#include "backends/saves/default/default-saves.h"
#include "backends/fs/riscos/riscos-fs-factory.h"
#include "backends/fs/riscos/riscos-fs.h"

#include <kernel.h>
#include <swis.h>

#ifndef URI_Dispatch
#define URI_Dispatch 0x4e381
#endif

void OSystem_RISCOS::init() {
	// Initialze File System Factory
	_fsFactory = new RISCOSFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_RISCOS::initBackend() {
	// Create the savefile manager
	if (_savefileManager == 0) {
		Common::String savePath = "/<Choices$Write>/ScummVM/Saves";
		if (Riscos::assureDirectoryExists(savePath))
			_savefileManager = new DefaultSaveFileManager(savePath);
	}

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

bool OSystem_RISCOS::hasFeature(Feature f) {
	if (f == kFeatureOpenUrl)
		return true;

	return OSystem_SDL::hasFeature(f);
}

bool OSystem_RISCOS::openUrl(const Common::String &url) {
	int flags;
	if (_swix(URI_Dispatch, _INR(0,2)|_OUT(0), 0, url.c_str(), 0, &flags) != NULL) {
		warning("openUrl() (RISCOS) failed to open URL");
		return false;
	}
	if ((flags & 1) == 1) {
		warning("openUrl() (RISCOS) failed to open URL");
		return false;
	}
	return true;
}

Common::String OSystem_RISCOS::getDefaultConfigFileName() {
	return "/<Choices$Write>/ScummVM/scummvmrc";
}

Common::WriteStream *OSystem_RISCOS::createLogFile() {
	// Start out by resetting _logFilePath, so that in case
	// of a failure, we know that no log file is open.
	_logFilePath.clear();

	Common::String logFile = "/<Choices$Write>/ScummVM/Logs";

	if (!Riscos::assureDirectoryExists(logFile)) {
		return 0;
	}

	logFile += "/scummvm";

	Common::FSNode file(logFile);
	Common::WriteStream *stream = file.createWriteStream();
	if (stream)
		_logFilePath = logFile;
	return stream;
}

#endif

