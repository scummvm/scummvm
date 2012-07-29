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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */
 
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_registry.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/wintermute.h"
#include "common/system.h"

namespace WinterMute {

BaseEngine *BaseEngine::_instance = 0;
	/*
class BaseEngine {
	static BaseEngine *_instance;
	BaseEngine();
	BaseFileManager *_fileManager;
	BaseRegistry *_registry;
public:
	~BaseEngine();
	BaseEngine *getInstance();
	BaseFileManager *getFileMan() { return _fileManager; }
	BaseRegistry *getRegistry() { return _registry; }
};*/
BaseEngine::BaseEngine() {

}

void BaseEngine::init() {
	_registry = new BaseRegistry();
	// File-manager depends on registry.
	_fileManager = new BaseFileManager();
}

BaseEngine::~BaseEngine() {
	delete _fileManager;
	delete _registry;
}

BaseEngine *BaseEngine::getInstance() {
	if (!_instance)
		assert(0);
	return _instance;
}

void BaseEngine::createInstance(const Common::String &gameid) {
	if (_instance) {
		delete _instance;
		_instance = NULL;
	}
	_instance = new BaseEngine();
	_instance->_gameId = gameid;
	_instance->init();
}

void BaseEngine::destroyInstance() {
	delete _instance;
	_instance = NULL;
}

void BaseEngine::LOG(bool res, const char *fmt, ...) {
	uint32 secs = g_system->getMillis() / 1000;
	uint32 hours = secs / 3600;
	secs = secs % 3600;
	uint32 mins = secs / 60;
	secs = secs % 60;
	
	char buff[512];
	va_list va;
	
	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	if (_instance && _instance->_gameRef) {
		_instance->_gameRef->LOG("%s", buff);
	} else {
		debugCN(kWinterMuteDebugLog, "%02d:%02d:%02d: %s\n", hours, mins, secs, buff);		
	}
}

} // end of namespace WinterMute