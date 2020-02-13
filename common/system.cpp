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

#define FORBIDDEN_SYMBOL_EXCEPTION_exit

#include "common/system.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/taskbar.h"
#include "common/updates.h"
#include "common/dialogs.h"
#include "common/textconsole.h"
#include "common/text-to-speech.h"

#include "backends/audiocd/default/default-audiocd.h"
#include "backends/fs/fs-factory.h"
#include "backends/timer/default/default-timer.h"

OSystem *g_system = nullptr;

OSystem::OSystem() {
	_audiocdManager = nullptr;
	_eventManager = nullptr;
	_timerManager = nullptr;
	_savefileManager = nullptr;
#if defined(USE_TASKBAR)
	_taskbarManager = nullptr;
#endif
#if defined(USE_UPDATES)
	_updateManager = nullptr;
#endif
#if defined(USE_TTS)
	_textToSpeechManager = nullptr;
#endif
#if defined(USE_SYSDIALOGS)
	_dialogManager = nullptr;
#endif
	_fsFactory = nullptr;
	_backendInitialized = false;
}

OSystem::~OSystem() {
	delete _audiocdManager;
	_audiocdManager = nullptr;

	delete _eventManager;
	_eventManager = nullptr;

	delete _timerManager;
	_timerManager = nullptr;

#if defined(USE_TASKBAR)
	delete _taskbarManager;
	_taskbarManager = nullptr;
#endif

#if defined(USE_UPDATES)
	delete _updateManager;
	_updateManager = nullptr;
#endif

#if defined(USE_TTS)
	delete _textToSpeechManager;
	_textToSpeechManager = 0;
#endif

#if defined(USE_SYSDIALOGS)
	delete _dialogManager;
	_dialogManager = nullptr;
#endif

	delete _savefileManager;
	_savefileManager = nullptr;

	delete _fsFactory;
	_fsFactory = nullptr;
}

void OSystem::initBackend() {
	// Verify all managers has been set
	if (!_audiocdManager)
		error("Backend failed to instantiate audio CD manager");
	if (!_eventManager)
		error("Backend failed to instantiate event manager");
	if (!getTimerManager())
		error("Backend failed to instantiate timer manager");

	// TODO: We currently don't check _savefileManager, because at least
	// on the Nintendo DS, it is possible that none is set. That should
	// probably be treated as "saving is not possible". Or else the NDS
	// port needs to be changed to always set a _savefileManager
// 	if (!_savefileManager)
// 		error("Backend failed to instantiate savefile manager");

	// TODO: We currently don't check _fsFactory because not all ports
	// set it.
// 	if (!_fsFactory)
// 		error("Backend failed to instantiate fs factory");

	_backendInitialized = true;
}

void OSystem::destroy() {
	_backendInitialized = false;
	Common::String::releaseMemoryPoolMutex();
	delete this;
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

bool OSystem::setStretchMode(const char *name) {
	if (!name)
		return false;

	// Special case for the 'default' filter
	if (!scumm_stricmp(name, "default")) {
		return setStretchMode(getDefaultStretchMode());
	}

	const GraphicsMode *sm = getSupportedStretchModes();

	while (sm->name) {
		if (!scumm_stricmp(sm->name, name)) {
			return setStretchMode(sm->id);
		}
		sm++;
	}

	return false;
}

void OSystem::fatalError() {
	quit();
	exit(1);
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
	return nullptr;
#else
	Common::FSNode file(getDefaultConfigFileName());
	return file.createWriteStream();
#endif
}

Common::String OSystem::getDefaultConfigFileName() {
	return "scummvm.ini";
}

Common::String OSystem::getSystemLanguage() const {
	return "en_US";
}

bool OSystem::isConnectionLimited() {
	warning("OSystem::isConnectionLimited(): not limited by default");
	return false;
}

Common::TimerManager *OSystem::getTimerManager() {
	return _timerManager;
}

Common::SaveFileManager *OSystem::getSavefileManager() {
	return _savefileManager;
}
