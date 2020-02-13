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
#include "backends/events/riscossdl/riscossdl-events.h"
#include "backends/fs/riscos/riscos-fs-factory.h"
#include "backends/fs/riscos/riscos-fs.h"

#include <kernel.h>
#include <swis.h>

#ifndef URI_Dispatch
#define URI_Dispatch 0x4e381
#endif

#ifndef Report_Text0
#define Report_Text0 0x54c80
#endif

void OSystem_RISCOS::init() {
	// Initialze File System Factory
	_fsFactory = new RISCOSFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_RISCOS::initBackend() {
	ConfMan.registerDefault("enable_reporter", false);

	// Create the events manager
	if (_eventSource == 0)
		_eventSource = new RISCOSSdlEventSource();

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

void OSystem_RISCOS::logMessage(LogMessageType::Type type, const char *message) {
	OSystem_SDL::logMessage(type, message);

	// Log messages using !Reporter, available from http://www.avisoft.force9.co.uk/Reporter.htm
	if (!(ConfMan.hasKey("enable_reporter") && ConfMan.getBool("enable_reporter")))
		return;

	char colour;
	switch (type) {
	case LogMessageType::kError:
		colour = 'r';
		break;
	case LogMessageType::kWarning:
		colour = 'o';
		break;
	case LogMessageType::kInfo:
		colour = 'l';
		break;
	case LogMessageType::kDebug:
	default:
		colour = 'f';
		break;
	}

	Common::String report = Common::String::format("\\%c %s", colour, message);
	_swix(Report_Text0, _IN(0), report.c_str());
}

Common::String OSystem_RISCOS::getDefaultConfigFileName() {
	return "/<Choices$Write>/ScummVM/scummvmrc";
}

Common::String OSystem_RISCOS::getDefaultLogFileName() {
	Common::String logFile = "/<Choices$Write>/ScummVM/Logs";

	if (!Riscos::assureDirectoryExists(logFile)) {
		return Common::String();
	}

	return logFile + "/scummvm";
}

#endif

