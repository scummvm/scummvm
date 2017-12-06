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

#include "agds/agds.h"
#include "agds/object.h"
#include "agds/process.h"
#include "common/error.h"
#include "common/ini-file.h"
#include "common/file.h"
#include "common/debug.h"

namespace AGDS {

AGDSEngine::AGDSEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _sharedStorageIndex(-2) {
}

AGDSEngine::~AGDSEngine() {
}

bool AGDSEngine::load() {
	Common::INIFile config;
	Common::File configFile;
	if (!configFile.open("agds.cfg"))
		return false;

	configFile.readLine(); //skip first line
	config.setDefaultSectionName("core");
	if (!config.loadFromStream(configFile))
		return false;

	Common::INIFile::SectionKeyList values = config.getKeys("core");
	for(Common::INIFile::SectionKeyList::iterator i = values.begin(); i != values.end(); ++i) {
		if (i->key == "path")
			if (!_resourceManager.addPath(i->value))
				return false;
	}
	if (!_data.open("data.adb"))
		return false;

	_patch.open("patch.adb"); //it's ok
	_nextScreen = "main";

	return true;
}

ProcessExitCode AGDSEngine::loadObject(const Common::String & name) {
	debug("loading object %s", name.c_str());
	Common::SeekableReadStream * stream = _data.getEntry(name);
	if (!stream)
		error("no database entry for %s\n", name.c_str());
	ObjectsType::iterator i = _objects.find(name);
	Object *object = i != _objects.end()? i->_value: NULL;
	if (!object)
		_objects.setVal(name, object = new Object(name, stream));

	_processes.push_back(Process(this, object));

	ProcessExitCode code = kExitCodeDestroy;
	while(!_processes.empty()) {
		Process & process = _processes.back();
		process.activate();
		code = process.execute();
		switch(code) {
		case kExitCodeLoadScreenObject:
		case kExitCodeDestroyProcessSetNextScreen:
			debug("loading screen object...");
			code = loadObject(process.getExitValue());
			break;
		default:
			debug("destroying process...");
			_processes.pop_back();
		}
	}
	return code;
}

Common::Error AGDSEngine::run() {
	if (!load())
		return Common::kNoGameDataFoundError;
	if (!_nextScreen.empty()) {
		debug("loading screen %s", _nextScreen.c_str());
		Common::String nextScreen;
		nextScreen = _nextScreen;
		_nextScreen.clear();
		loadObject(nextScreen);
	}

	return Common::kNoError;
}

int AGDSEngine::appendToSharedStorage(const Common::String &value) {
	int index = _sharedStorageIndex;
	_sharedStorage[-2 - (_sharedStorageIndex--)] = value;
	if (_sharedStorageIndex <= -12)
		_sharedStorageIndex = -2;
	return index;
}

const Common::String & AGDSEngine::getSharedStorage(int id) const {
	int index = -2 - id;
	if (index < 0 || index >= 10)
		error("shared storage id is out of range");
	return _sharedStorage[index];
}

int AGDSEngine::getGlobal(const Common::String &name) const {
	GlobalsType::const_iterator i = _globals.find(name);
	if (i != _globals.end())
		return i->_value;
	else {
		debug("global %s was not declared, returning 0", name.c_str());
		return 0;
	}
}



} // End of namespace AGDS
