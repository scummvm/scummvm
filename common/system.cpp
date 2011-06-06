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

#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fputs
#define FORBIDDEN_SYMBOL_EXCEPTION_fflush
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr

#include "common/system.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/str.h"
#include "common/textconsole.h"

#include "backends/audiocd/default/default-audiocd.h"
#include "backends/timer/default/default-timer.h"

OSystem *g_system = 0;

OSystem::OSystem() {
	_audiocdManager = 0;
	_eventManager = 0;
	_timerManager = 0;
	_savefileManager = 0;
	_fsFactory = 0;
}

OSystem::~OSystem() {
	delete _audiocdManager;
	_audiocdManager = 0;

	delete _eventManager;
	_eventManager = 0;

	delete _timerManager;
	_timerManager = 0;

	delete _savefileManager;
	_savefileManager = 0;

	delete _fsFactory;
	_fsFactory = 0;
}

void OSystem::initBackend() {
	// Init audio CD manager
#ifndef DISABLE_DEFAULT_AUDIOCD_MANAGER
	if (!_audiocdManager)
		_audiocdManager = new DefaultAudioCDManager();
#endif

	// Verify all managers has been set
	if (!_audiocdManager)
		error("Backend failed to instantiate audio CD manager");
	if (!_eventManager)
		error("Backend failed to instantiate event manager");
	if (!_timerManager)
		error("Backend failed to instantiate timer manager");
// 	if (!_savefileManager)
// 		error("Backend failed to instantiate savefile manager");
// 	if (!_fsFactory)
// 		error("Backend failed to instantiate fs factory");
}

bool OSystem::setGraphicsMode(const char *name) {
	if (!name)
		return false;

	// Special case for the 'default' filter
	if (!scumm_stricmp(name, "normal") || !scumm_stricmp(name, "default")) {
		return setGraphicsMode(getDefaultGraphicsMode());
	}

	const GraphicsMode *gm = getSupportedGraphicsModes();

	while (gm->name) {
		if (!scumm_stricmp(gm->name, name)) {
			return setGraphicsMode(gm->id);
		}
		gm++;
	}

	return false;
}

void OSystem::fatalError() {
	quit();
	exit(1);
}

Common::TimerManager *OSystem::getTimerManager() {
	return _timerManager;
}

Common::SaveFileManager *OSystem::getSavefileManager() {
	assert(_savefileManager);
	return _savefileManager;
}

FilesystemFactory *OSystem::getFilesystemFactory() {
	assert(_fsFactory);
	return _fsFactory;
}

Common::SeekableReadStream *OSystem::createConfigReadStream() {
	Common::FSNode file(getDefaultConfigFileName());
	return file.createReadStream();
}

Common::WriteStream *OSystem::createConfigWriteStream() {
#ifdef __DC__
	return 0;
#else
	Common::FSNode file(getDefaultConfigFileName());
	return file.createWriteStream();
#endif
}

Common::String OSystem::getDefaultConfigFileName() {
	return "scummvm.ini";
}

void OSystem::logMessage(LogMessageType::Type type, const char *message) {
#if !defined(__PLAYSTATION2__) && !defined(__DS__)
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);
#endif
}

Common::String OSystem::getSystemLanguage() const {
	return "en_US";
}
