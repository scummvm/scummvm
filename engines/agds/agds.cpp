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
#include "agds/mjpgPlayer.h"
#include "agds/object.h"
#include "agds/process.h"
#include "agds/region.h"
#include "common/error.h"
#include "common/events.h"
#include "common/ini-file.h"
#include "common/file.h"
#include "common/debug.h"
#include "common/system.h"
#include "engines/util.h"

namespace AGDS {

AGDSEngine::AGDSEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _sharedStorageIndex(-2), _timer(0), _mjpgPlayer(NULL) {
}

AGDSEngine::~AGDSEngine() {
}

bool AGDSEngine::initGraphics() {
	//fixme: get mode from config?
	typedef Common::List<Graphics::PixelFormat> FormatsType;
	FormatsType formats = _system->getSupportedFormats();

	FormatsType::iterator fi;
	for(fi = formats.begin(); fi != formats.end(); ++fi) {
		if (fi->bytesPerPixel == 4) {
			debug("found mode %s", fi->toString().c_str());
			::initGraphics(800, 600, &*fi);
			return true;
		}
	}
	error("nope");
	return false;
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

	if (!initGraphics())
		error("no video mode found");

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

Region * AGDSEngine::loadRegion(const Common::String &name) {
	debug("loading region %s", name.c_str());
	Common::SeekableReadStream * stream = _data.getEntry(name);
	if (!stream)
		error("no database entry for %s\n", name.c_str());
	return new Region(name, stream);
}

Common::String AGDSEngine::loadFilename(const Common::String &entryName) {
	Common::SeekableReadStream * stream = _data.getEntry(entryName);
	if (!stream)
		error("no database entry for %s\n", entryName.c_str());

	byte name[32];
	int end = stream->read(name, sizeof(name));
	byte *nameEnd = Common::find(name, name + end, 0);
	unsigned size = nameEnd - name;
	ResourceManager::decrypt(name, size);

	delete stream;
	return Common::String(reinterpret_cast<const char *>(name), size);
}


void AGDSEngine::loadObject(const Common::String & name) {
	debug("loading object %s", name.c_str());
	Common::SeekableReadStream * stream = _data.getEntry(name);
	if (!stream)
		error("no database entry for %s\n", name.c_str());
	ObjectsType::iterator i = _objects.find(name);
	Object *object = i != _objects.end()? i->_value: NULL;
	if (!object)
		_objects.setVal(name, object = new Object(name, stream));
	else
		return;

	delete stream;

	_processes.push_front(Process(this, object));
}

void AGDSEngine::runProcess() {
	for(ProcessListType::iterator p = _processes.begin(); active() && p != _processes.end(); ) {
		Process & process = *p;
		if (process.getStatus() == Process::kStatusDone) {
			p = _processes.erase(p);
			continue;
		}
		process.activate();
		ProcessExitCode code = process.execute();
		switch(code) {
		case kExitCodeLoadScreenObject:
		case kExitCodeDestroyProcessSetNextScreen:
			debug("loading screen object...");
			loadObject(process.getExitValue());
			break;
		case kExitCodeSuspend:
			debug("process suspended");
			break;
		default:
			debug("destroying process...");
			p = _processes.erase(p);
			continue;
		}
		break;
	}
}


Common::Error AGDSEngine::run() {
	if (!load())
		return Common::kNoGameDataFoundError;

	Common::EventManager *eventManager = _system->getEventManager();
	_system->fillScreen(0);

	while(!shouldQuit()) {
		if (_timer > 0)
			--_timer;

		if (!_nextScreen.empty()) {
			debug("loading screen %s", _nextScreen.c_str());
			Common::String nextScreen;
			nextScreen = _nextScreen;
			_nextScreen.clear();
			loadObject(nextScreen);
		}

		while(active() && !_processes.empty())
			runProcess();

		if (_mjpgPlayer) {
			const Graphics::Surface *surface = _mjpgPlayer->decodeFrame();

			if (surface)
				_system->copyRectToScreen(surface->getPixels(), surface->pitch, 0, 0, surface->w, surface->h);

			if (_mjpgPlayer->eos()) {
				delete _mjpgPlayer;
				_mjpgPlayer = NULL;
			}
		}

		Common::Event event;
		while(eventManager->pollEvent(event)) {
		}
		_system->updateScreen();
		_system->delayMillis(20);
	}

	return Common::kNoError;
}

void AGDSEngine::playFilm(const Common::String &video, const Common::String &audio) {
	delete _mjpgPlayer;
	_mjpgPlayer = new MJPGPlayer(_resourceManager.getResource(video));
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
