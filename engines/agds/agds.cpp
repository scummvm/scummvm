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
#include "agds/screen.h"
#include "common/error.h"
#include "common/events.h"
#include "common/ini-file.h"
#include "common/file.h"
#include "common/debug.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/transparent_surface.h"

namespace AGDS {

AGDSEngine::AGDSEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _sharedStorageIndex(-2), _timer(0),
		_mjpgPlayer(NULL), _currentScreen(NULL), _mouseCursor(NULL),
		_mouse(400, 300), _userEnabled(false), _currentRegion(NULL) {
}

AGDSEngine::~AGDSEngine() {
}

bool AGDSEngine::initGraphics() {
	//fixme: get mode from config?
	typedef Common::List<Graphics::PixelFormat> FormatsType;
	FormatsType formats = _system->getSupportedFormats();

	for(FormatsType::iterator fi = formats.begin(); fi != formats.end(); ++fi) {
		const Graphics::PixelFormat & format = *fi;
		if (fi->bytesPerPixel == 4 && format == Graphics::TransparentSurface::getSupportedPixelFormat()) {
			debug("found mode %s", format.toString().c_str());
			_pixelFormat = format;
			::initGraphics(800, 600, &_pixelFormat);
			return true;
		}
	}
	error("no supported video format found");
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
	loadScreen("main");

	return true;
}

Region * AGDSEngine::loadRegion(const Common::String &name) {
	RegionsType::iterator i = _regions.find(name);
	if (i != _regions.end())
		return i->_value;

	debug("loading region %s", name.c_str());
	Common::SeekableReadStream * stream = _data.getEntry(name);
	if (!stream)
		error("no database entry for %s\n", name.c_str());

	Region *region = new Region(name, stream);
	delete stream;

	_regions[name] = region;
	return region;
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


Object *AGDSEngine::loadObject(const Common::String & name, const Common::String &prototype) {
	ObjectsType::iterator i = _objects.find(name);
	Object *object = i != _objects.end()? i->_value: NULL;
	if (!object) {
		Common::String clone = prototype.empty()? name: prototype;
		Common::SeekableReadStream * stream = _data.getEntry(clone);
		if (!stream)
			error("no database entry for %s\n", clone.c_str());

		object = new Object(name, stream);
		_objects.setVal(name, object);
		delete stream;
	}
	return object;
}

void AGDSEngine::runObject(Object *object) {
	_processes.push_front(Process(this, object));
	if (_currentScreen)
		_currentScreen->add(object);
}

void AGDSEngine::loadScreen(const Common::String & name) {
	debug("loadScreen %s", name.c_str());
	ScreensType::iterator i = _screens.find(name);
	if (i == _screens.end())
	{
		Object *object = loadObject(name);
		Screen *screen = new Screen(object);
		_currentScreen = screen;
		_screens[name] = screen;
	}
	runObject(name);
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
			runObject(process.getExitArg1(), process.getExitArg2());
			break;
		case kExitCodeDestroyProcessSetNextScreen:
			loadScreen(process.getExitArg1());
			break;
		case kExitCodeSuspend:
			debug("process suspended");
			return;
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

		Common::Event event;
		while(eventManager->pollEvent(event)) {
			switch(event.type) {
				case Common::EVENT_MOUSEMOVE:
					_mouse = event.mouse;
					if (_userEnabled && _currentScreen) {
						const MouseRegion *region = _mouseMap.find(_mouse);
						if ((region? region->region: NULL) != _currentRegion) {
							if (_currentRegion) {
								_currentRegion = NULL;
								runObject(_onLeaveObject);
							}
							if (region) {
								_onLeaveObject = region->onLeave;
								_currentRegion = region->region;
								runObject(region->onEnter);
							}
						}
					}
					break;
				case Common::EVENT_LBUTTONDOWN:
					_mouse = event.mouse;
					if (_userEnabled && _currentScreen) {
						debug("lclick %d, %d", _mouse.x, _mouse.y);
						Object *object = _currentScreen->find(_mouse);
						if (object) {
							uint ip = object->getClickHandler();
							if (ip) {
								debug("found handler: %s %08x", object->getName().c_str(), ip + 7);
								_processes.push_front(Process(this, object, ip));
							}
						}
					}
					break;
				default:
					break;
			}
		}

		if (active())
			runProcess();

		Graphics::Surface *backbuffer = _system->lockScreen();
		backbuffer->fillRect(backbuffer->getRect(), 0);

		if (_mjpgPlayer) {
			const Graphics::Surface *surface = _mjpgPlayer->decodeFrame();

			if (surface) {
				Graphics::Surface * converted = surface->convertTo(_pixelFormat);
				Common::Point dst((backbuffer->w - converted->w) / 2, (backbuffer->h - converted->h) / 2);
				Common::Rect srcRect(converted->getRect());
				if (Common::Rect::getBlitRect(dst, srcRect, backbuffer->getRect()))
					backbuffer->copyRectToSurface(*converted, dst.x, dst.y, srcRect);
				delete converted;
			}

			if (_mjpgPlayer->eos()) {
				delete _mjpgPlayer;
				_mjpgPlayer = NULL;
			}
		} else if (_currentScreen) {
			_currentScreen->paint(*backbuffer);
		}

		if (_userEnabled && _mouseCursor) {
			const Graphics::Surface * frame = _mouseCursor->decodeNextFrame();
			if (!frame) {
				_mouseCursor->rewind();
				frame = _mouseCursor->decodeNextFrame();
			}
			Graphics::TransparentSurface * c = convertToTransparent(frame->convertTo(_pixelFormat, _mouseCursor->getPalette()));
			Common::Point dst = _mouse;
			Common::Rect srcRect = c->getRect();
			if (Common::Rect::getBlitRect(dst, srcRect, backbuffer->getRect()))
				c->blit(*backbuffer, dst.x, dst.y, Graphics::FLIP_NONE, &srcRect);
			delete c;
		}

		_system->unlockScreen();
		_system->updateScreen();
		_system->delayMillis(40);
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

void AGDSEngine::loadCursor(const Common::String &name, unsigned index) {
	Video::FlicDecoder * cursor = new Video::FlicDecoder;
	if (cursor->loadStream(_resourceManager.getResource(name))) {
		delete _mouseCursor;
		_mouseCursor = cursor;
	} else
		delete cursor;
}

const Graphics::Surface * AGDSEngine::loadPicture(const Common::String &name)
{ return convertToTransparent(_resourceManager.loadPicture(name, _pixelFormat)); }

Graphics::TransparentSurface *AGDSEngine::convertToTransparent(const Graphics::Surface *surface) {
	if (!surface)
		return NULL;
	Graphics::TransparentSurface * t = new Graphics::TransparentSurface(*surface, true);
	t->applyColorKey(0xff, 0, 0xff, true);
	delete surface;
	return t;
}

} // End of namespace AGDS
