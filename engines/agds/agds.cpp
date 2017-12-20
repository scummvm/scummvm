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
#include "agds/animation.h"
#include "agds/mjpgPlayer.h"
#include "agds/object.h"
#include "agds/process.h"
#include "agds/region.h"
#include "agds/screen.h"
#include "agds/systemVariable.h"
#include "common/error.h"
#include "common/events.h"
#include "common/ini-file.h"
#include "common/file.h"
#include "common/debug.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/transparent_surface.h"

namespace AGDS {

AGDSEngine::AGDSEngine(OSystem *system, const ADGameDescription *gameDesc) : Engine(system),
		_gameDescription(gameDesc), _pictureCacheId(0), _sharedStorageIndex(-2), _timer(0),
		_mjpgPlayer(), _currentScreen(), _previousScreen(),
		_defaultMouseCursor(),
		_mouse(400, 300), _userEnabled(false), _currentRegion(),
		_random("agds"), _soundManager(this, system->getMixer()),
		_fastMode(false) {
}

AGDSEngine::~AGDSEngine() {
	delete _currentScreen;
	delete _previousScreen;
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

	initSystemVariables();
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

Common::String AGDSEngine::loadText(const Common::String &entryName) {
	return ResourceManager::loadText(_data.getEntry(entryName));
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
	runProcess(object);
	if (_currentScreen)
		_currentScreen->add(object);
}

void AGDSEngine::runProcess(Object *object, uint ip) {
	_processes.push_front(Process(this, object, ip));
	ProcessListType::iterator it = _processes.begin();
	runProcess(it);
}

void AGDSEngine::runObject(const Common::String & name, const Common::String &prototype)
{
	runObject(loadObject(name, prototype));
}

void AGDSEngine::loadScreen(const Common::String & name) {
	debug("loadScreen %s", name.c_str());
	resetCurrentScreen();
	_currentScreenName = name;
	_currentScreen = new Screen(loadObject(name), _mouseMap);
	_mouseMap.clear();
	runObject(name); //is it called once or per screen activation?
}

void AGDSEngine::setCurrentScreen(Screen *screen) {
	if (!screen)
		error("no previous screen");

	resetCurrentScreen();

	_currentScreenName = screen->getName();
	_currentScreen = screen;
	_previousScreen = NULL;
}

void AGDSEngine::resetCurrentScreen()
{
	if (_currentRegion) {
		if (_currentRegion->currentlyIn)
			runObject(_currentRegion->onLeave);
		_currentRegion = NULL;
	}

	delete _currentScreen;
	_currentScreen = NULL;
}


void AGDSEngine::runProcess(ProcessListType::iterator &it) {
	Process & process = *it;
	if (process.parentScreenName() != _currentScreenName) {
		++it;
		return;
	}

	const Common::String &name = process.getName();
	if (process.getStatus() == Process::kStatusDone || process.getStatus() == Process::kStatusError) {
		debug("process %s finished", name.c_str());
		it = _processes.erase(it);
		return;
	}
	process.activate();
	ProcessExitCode code = process.execute();
	bool destroy = false;
	switch(code) {
	case kExitCodeDestroy:
		destroy = true;
		break;
	case kExitCodeLoadScreenObject:
	case kExitCodeRunDialog:
		runObject(process.getExitArg1(), process.getExitArg2());
		break;
	case kExitCodeSetNextScreen:
		loadScreen(process.getExitArg1());
		destroy = true;
		break;
	case kExitCodeSetNextScreenSaveInHistory:
		if (_currentScreen) {
			delete _previousScreen;
			_previousScreen = _currentScreen;
			_currentScreen = NULL;
		}
		loadScreen(process.getExitArg1());
		destroy = true;
		break;
	case kExitCodeLoadPreviousScreenObject:
		setCurrentScreen(_previousScreen);
		break;
	case kExitCodeMouseAreaChange:
		changeMouseArea(process.getExitIntArg1(), process.getExitIntArg2());
		break;
	case kExitCodeLoadInventoryObject:
		_inventory.add(loadObject(process.getExitArg1()));
		break;
	case kExitCodeSuspend:
		break;
	case kExitCodeCreatePatchLoadResources:
		runObject(process.getExitArg1());
		_inventory.clear();
		runObject(process.getExitArg2());
		break;
	default:
		error("unknown process exit code %d", code);
	}
	if (destroy) {
		debug("destroying process %s...", name.c_str());
		it = _processes.erase(it);
	} else
		++it;
}

void AGDSEngine::runProcess() {
	for(ProcessListType::iterator p = _processes.begin(); active() && p != _processes.end(); ) {
		runProcess(p);
	}
}

void AGDSEngine::changeMouseArea(int id, int enabled) {
	if (id < 0)
		return;
	MouseRegion * mouseArea = _mouseMap.find(id);
	if (mouseArea) {
		switch(enabled) {
			case 1:
				mouseArea->enable();
				break;
			case 0:
			case -1:
				if (mouseArea->currentlyIn) {
					runObject(mouseArea->onLeave);
				}
				mouseArea->disable();
				if (enabled == -1) {
					_mouseMap.remove(id);
				}
				break;
		}
	} else
		warning("mouse area %d could not be found", id);
}

Common::Error AGDSEngine::run() {
	if (!load())
		return Common::kNoGameDataFoundError;

	Common::EventManager *eventManager = _system->getEventManager();

	while(!shouldQuit()) {
		uint32 frameStarted = _system->getMillis();

		if (_timer > 0)
			--_timer;

		Common::Event event;
		while(eventManager->pollEvent(event)) {
			if (!_currentScreen)
				continue;

			switch(event.type) {
				case Common::EVENT_KEYDOWN:
					{
						Common::String key;

						switch(event.kbd.keycode) {
							case Common::KEYCODE_SPACE:
								skipFilm();
								key = "space";
								break;
							case Common::KEYCODE_ESCAPE:
								skipFilm();
								key = "escape";
								break;
							case Common::KEYCODE_TAB:
								key = "tab";
								break;
							case Common::KEYCODE_f:
								if (event.kbd.flags & Common::KBD_CTRL) {
									_fastMode = !_fastMode;
									break;
								}
							default:
								if (event.kbd.ascii)
									key = Common::String(static_cast<char>(event.kbd.ascii));
						};
						if (_userEnabled && !key.empty()) {
							Screen::KeyHandler handler = _currentScreen->findKeyHandler(key);
							if (handler.object) {
								debug("found handler for key %s: %s %08x", key.c_str(), handler.object->getName().c_str(), handler.ip + 7);
								runProcess(handler.object, handler.ip);
							}
						}
					}
					break;
				case Common::EVENT_MOUSEMOVE:
					_mouse = event.mouse;
					if (_userEnabled) {
						MouseMap &mouseMap = _currentScreen->mouseMap();
						MouseRegion *region = mouseMap.find(_mouse);
						if (region != _currentRegion) {
							if (_currentRegion) {
								MouseRegion *currentRegion = _currentRegion;
								_currentRegion = NULL;
								currentRegion->currentlyIn = false;
								runObject(currentRegion->onLeave);
							}
							if (region) {
								_currentRegion = region;
								_currentRegion->currentlyIn = true;
								runObject(region->onEnter);
							}
						}
					}
					break;
				case Common::EVENT_LBUTTONDOWN:
					_mouse = event.mouse;
					if (_userEnabled) {
						debug("lclick %d, %d", _mouse.x, _mouse.y);
						Object *object = _currentScreen->find(_mouse);
						if (object) {
							uint ip = object->getClickHandler();
							if (ip) {
								debug("found handler: %s %08x", object->getName().c_str(), ip + 7);
								runProcess(object, ip);
							}
						}
					}
					break;
				default:
					break;
			}
		}

		Animation *mouseCursor = NULL;

		if (_userEnabled && _currentScreen) {
			Object *object = _currentScreen->find(_mouse);
			Animation *cursor = object? object->getMouseCursor(): NULL;
			if (cursor)
				mouseCursor = cursor;
		}

		_soundManager.tick();
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
			_currentScreen->paint(*this, *backbuffer);
		}

		if (!mouseCursor)
			mouseCursor = _defaultMouseCursor;

		if (_userEnabled && mouseCursor) {
			mouseCursor->paint(*this, *backbuffer, _mouse);
		}

		_system->unlockScreen();
		_system->updateScreen();

		if (!_fastMode) {
			static const uint32 kFPS = 25;
			static const uint32 kMaxTick = 1000 / kFPS;

			uint32 dt = _system->getMillis() - frameStarted;
			if (dt < kMaxTick)
				_system->delayMillis(kMaxTick - dt);
		}
	}

	return Common::kNoError;
}

void AGDSEngine::playFilm(const Common::String &video, const Common::String &audio) {
	delete _mjpgPlayer;
	_mjpgPlayer = new MJPGPlayer(_resourceManager.getResource(video));
}

void AGDSEngine::skipFilm() {
	delete _mjpgPlayer;
	_mjpgPlayer = NULL;
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

Animation * AGDSEngine::loadAnimation(const Common::String &name) {
	AnimationsType::iterator i = _animations.find(name);
	if (i != _animations.end())
		return i->_value;

	Common::SeekableReadStream *stream = _resourceManager.getResource(name);
	if (!stream)
		error("could not load animation from %s", name.c_str());
	Animation *animation = new Animation();
	if (!animation->load(stream))
		error("could not load animation from %s", name.c_str());
	_animations[name] = animation;
	return animation;
}

Graphics::TransparentSurface * AGDSEngine::loadPicture(const Common::String &name)
{ return convertToTransparent(_resourceManager.loadPicture(name, _pixelFormat)); }

Graphics::TransparentSurface *AGDSEngine::loadFromCache(int id) const {
	PictureCacheType::const_iterator i = _pictureCache.find(id);
	return (i != _pictureCache.end())? i->_value: NULL;
}

Graphics::TransparentSurface *AGDSEngine::convertToTransparent(const Graphics::Surface *surface) {
	if (!surface)
		return NULL;
	Graphics::TransparentSurface * t = new Graphics::TransparentSurface(*surface, true);
	t->applyColorKey(0xff, 0, 0xff);
	delete surface;
	return t;
}

void AGDSEngine::initSystemVariables() {
	_systemVars["inventory_scr"]	= new StringSystemVariable();
	_systemVars["escape_scr"]		= new StringSystemVariable("none");
	_systemVars["load_scr"]			= new StringSystemVariable();
	_systemVars["load_scr"]			= new StringSystemVariable();

	_systemVars["gfx_bright"]		= new IntegerSystemVariable(50);
	_systemVars["gfx_contrast"]		= new IntegerSystemVariable(50);

	_systemVars["sound_volume"]		= new IntegerSystemVariable(100);
	_systemVars["music_volume"]		= new IntegerSystemVariable(80);
	_systemVars["tell_volume"]		= new IntegerSystemVariable(100);

	_systemVars["text_speed"]		= new IntegerSystemVariable(70);
	_systemVars["tell_mode"]		= new IntegerSystemVariable(3);
	_systemVars["version"]			= new IntegerSystemVariable(1);

	_systemVars["objtext_x"]		= new IntegerSystemVariable(-1);
	_systemVars["objtext_y"]		= new IntegerSystemVariable(-1);
	_systemVars["objtext_mode"]		= new IntegerSystemVariable(-1);
	_systemVars["objtext_font"]		= new IntegerSystemVariable(-1);

	_systemVars["inv_open"]			= new StringSystemVariable();
	_systemVars["inv_close"]		= new StringSystemVariable();
	_systemVars["inv_region"]		= new StringSystemVariable();

	_systemVars["anim_zoom"]		= new IntegerSystemVariable(1);

	_systemVars["screen_curtain"]	= new IntegerSystemVariable(1);
	_systemVars["music_curtain"]	= new IntegerSystemVariable(1);
	_systemVars["sound_curtain"]	= new IntegerSystemVariable(1);

	_systemVars["old_music_volume"]	= new IntegerSystemVariable();
	_systemVars["old_sound_volume"]	= new IntegerSystemVariable();
	_systemVars["old_screen_fade"]	= new IntegerSystemVariable();

	_systemVars["subtitle_x"]		= new IntegerSystemVariable();
	_systemVars["subtitle_y"]		= new IntegerSystemVariable();
	_systemVars["subtitle_type"]	= new IntegerSystemVariable(3);
	_systemVars["subtitles"]		= new IntegerSystemVariable();

	_systemVars["tell_font"]		= new IntegerSystemVariable();
	_systemVars["npc_tell_font"]	= new IntegerSystemVariable();
	_systemVars["edit_font"]		= new IntegerSystemVariable();
	_systemVars["delay_after_tell"]	= new IntegerSystemVariable();

	_systemVars["scroll_factor"]	= new IntegerSystemVariable(30);

	_systemVars["dialog_var"]		= new IntegerSystemVariable();
	_systemVars["subtitle_width"]	= new IntegerSystemVariable(-1);
	_systemVars["flash_mouse"]		= new IntegerSystemVariable();
	_systemVars["scale_char"]		= new IntegerSystemVariable();

	_systemVars["init_resources"]	= new StringSystemVariable();
	_systemVars["done_resources"]	= new StringSystemVariable();
	_systemVars["tell_close_inv"]	= new IntegerSystemVariable(1);
	_systemVars["gamma"]			= new IntegerSystemVariable();
}

SystemVariable *AGDSEngine::getSystemVariable(const Common::String &name) {
	SystemVariablesType::iterator i = _systemVars.find(name);
	if (i != _systemVars.end())
		return i->_value;

	error("no system variable %s", name.c_str());
}



} // End of namespace AGDS
