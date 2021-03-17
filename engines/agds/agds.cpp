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
#include "agds/character.h"
#include "agds/console.h"
#include "agds/database.h"
#include "agds/font.h"
#include "agds/mjpgPlayer.h"
#include "agds/object.h"
#include "agds/patch.h"
#include "agds/process.h"
#include "agds/region.h"
#include "agds/screen.h"
#include "agds/systemVariable.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/ini-file.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/transparent_surface.h"

namespace AGDS {

AGDSEngine::AGDSEngine(OSystem *system, const ADGameDescription *gameDesc) : Engine(system),
                                                                             _gameDescription(gameDesc), _pictureCacheId(1), _sharedStorageIndex(-2),
																			 _shadowIntensity(0),
																			 _processes(MaxProcesses),
                                                                             _mjpgPlayer(), _filmStarted(0),
																			 _currentScreen(), _loadingScreen(false),
																			 _currentCharacter(),
                                                                             _defaultMouseCursor(),
																			 _nextScreenType(ScreenLoadingType::Normal),
                                                                             _mouse(400, 300),
																			 _userEnabled(true), _systemUserEnabled(true),
																			 _currentRegion(),
                                                                             _random("agds"),
                                                                             _inventoryRegion(),
                                                                             _soundManager(this, system->getMixer()),
																			 _inventory(this),
																			 _dialog(this),
																			 _tellTextTimer(0),
																			 _syncSoundId(-1),
																			 _ambientSoundId(-1),
                                                                             _fastMode(true),
																			 _hintMode(false) {
}

AGDSEngine::~AGDSEngine() {
	delete _jokes;
	delete _currentCharacter;
	delete _currentScreen;
	for (PictureCacheType::iterator i = _pictureCache.begin(); i != _pictureCache.end(); ++i) {
		i->_value->free();
		delete i->_value;
	}
}

bool AGDSEngine::initGraphics(int w, int h) {
	typedef Common::List<Graphics::PixelFormat> FormatsType;
	FormatsType formats = _system->getSupportedFormats();

	for (FormatsType::iterator fi = formats.begin(); fi != formats.end(); ++fi) {
		const Graphics::PixelFormat &format = *fi;
		if (fi->bytesPerPixel == 4 && format == Graphics::TransparentSurface::getSupportedPixelFormat()) {
			debug("found mode %s", format.toString().c_str());
			_pixelFormat = format;
			::initGraphics(w, h, &_pixelFormat);
			return true;
		}
	}
	error("no supported video format found");
	return false;
}

void AGDSEngine::Color::FromString(const Common::String &rgb) {
	uint cr, cg, cb;
	if (sscanf(rgb.c_str(), "%u,%u,%u", &cr, &cg, &cb) == 3) {
		r = cr;
		g = cg;
		b = cb;
	}
}

uint32 AGDSEngine::Color::map(const Graphics::PixelFormat &format) const {
	return format.RGBToColor(r, g, b);
}
Common::String AGDSEngine::Color::ToString() const {
	return Common::String::format("#%02x%02x%02x", r, g, b);
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

	int w = 800, h = 600, d = 32;
	Common::String videoMode;
	if (config.getKey("videomode", "core", videoMode) && sscanf(videoMode.c_str(), "%dx%dx%d", &w, &h, &d) == 3) {
		debug("config videomode = %dx%d", w, h);
	}

	{
		Common::String transparency, shadowMin, shadowMax;
		if (config.getKey("transparency", "core", transparency)) {
			_colorKey.FromString(transparency);
		}
		debug("transparent color: %s", _colorKey.ToString().c_str());

		if (config.getKey("shadow_min", "core", shadowMin)) {
			_minShadowColor.FromString(shadowMin);
		}
		debug("shadow color min: %s", _minShadowColor.ToString().c_str());

		if (config.getKey("shadow_max", "core", shadowMax)) {
			_maxShadowColor.FromString(shadowMax);
		}
		debug("shadow color max: %s", _maxShadowColor.ToString().c_str());
	}

	if (!initGraphics(w, h)) {
		warning("no video mode found");
		return false;
	}

	Common::INIFile::SectionKeyList values = config.getKeys("core");
	for (Common::INIFile::SectionKeyList::iterator i = values.begin(); i != values.end(); ++i) {
		if (i->key == "path")
			if (!_resourceManager.addPath(i->value))
				return false;
	}

	if (!_data.open("data.adb"))
		return false;

	initSystemVariables();
	setNextScreenName("main", ScreenLoadingType::Normal);

	{
		Common::File file;
		file.open("patch.adb");
		Database patch;
		patch.open("patch.adb");

		loadPatches(&file, patch);
	}
	{
		Common::File * file = new Common::File();
		file->open("jokes.chr");
		_jokes = new Character(this, "jokes");
		_jokes->load(file);
	}

	return true;
}

RegionPtr AGDSEngine::loadRegion(const Common::String &name) {
	debug("loading region %s", name.c_str());
	Common::SeekableReadStream *stream = _data.getEntry(name);
	if (!stream)
		error("no database entry for %s\n", name.c_str());

	RegionPtr region(new Region(name, stream));
	delete stream;

	return region;
}

Common::String AGDSEngine::loadText(const Common::String &entryName) {
	if (entryName.empty())
		return Common::String();
	return ResourceManager::loadText(_data.getEntry(entryName));
}

ObjectPtr AGDSEngine::loadObject(const Common::String &name, const Common::String &prototype, bool allowInitialise) {
	debug("loadObject %s %s, allow init: %d", name.c_str(), prototype.c_str(), allowInitialise);
	Common::String clone = prototype.empty() ? name : prototype;
	Common::SeekableReadStream *stream = _data.getEntry(clone);
	if (!stream)
		error("no database entry for %s\n", clone.c_str());

	ObjectPtr object(new Object(name, stream));
	object->allowInitialise(allowInitialise);
	if (!prototype.empty()) {
		object->persistent(false);
	}
	delete stream;
	return object;
}

void AGDSEngine::runObject(const ObjectPtr &object) {
	if (_currentScreen) {
		if (_currentScreen->add(object)) {
			runProcess(object);

			auto it = _objectPatches.find(object->getName());
			if (it != _objectPatches.end()) {
				auto& patch = *it->_value;
				if (!patch.region.empty()) {
					RegionPtr region = loadRegion(patch.region);
					debug("runObject: patch region: %s", region->toString().c_str());
					object->region(region);
				}
				if (!patch.text.empty()) {
					auto text = loadText(patch.text);
					debug("runObject: patch title: %s -> %s", patch.text.c_str(), text.c_str());
					object->title(text);
				}
				object->z(patch.z);
			}
		} else
			debug("object %s is in scene, skip run", object->getName().c_str());
	} else
		warning("object %s has been loaded, but was not added to any screen", object->getName().c_str());
}

void AGDSEngine::runProcess(const ObjectPtr &object, uint ip) {
	debug("starting process %s:%04x", object->getName().c_str(), ip);
	for(uint i = 0; i < _processes.size(); ++i) {
		auto &process = _processes[i];
		if (!process) {
			process = ProcessPtr(new Process(this, object, ip));
			process->run();
			return;
		}
	}
	error("process table exhausted");
}

ObjectPtr AGDSEngine::getCurrentScreenObject(const Common::String &name) {
	return _currentScreen? _currentScreen->find(name): ObjectPtr();
}


ObjectPtr AGDSEngine::runObject(const Common::String &name, const Common::String &prototype, bool allowInitialise) {
	debug("runObject %s %s", name.c_str(), prototype.c_str());
	ObjectPtr object = getCurrentScreenObject(name);
	if (!object) {
		object = loadObject(name, prototype, allowInitialise);
	}
	runObject(object);
	return object;
}

void AGDSEngine::reAddInventory() {
	if (!_currentScreen)
		return;

	for(auto & object : _inventory.entries()) {
		if (object)
			runObject(object);
	}
}

PatchPtr AGDSEngine::getPatch(const Common::String &screenName) const {
	auto it = _patches.find(screenName);
	return it != _patches.end()? it->_value: PatchPtr();
}

PatchPtr AGDSEngine::createPatch(const Common::String &screenName) {
	auto & patch = _patches[screenName];
	if (!patch)
		patch = PatchPtr(new Patch());
	return patch;
}

ObjectPatchPtr AGDSEngine::getObjectPatch(const Common::String &objectName) const {
	auto it = _objectPatches.find(objectName);
	return it != _objectPatches.end()? it->_value: ObjectPatchPtr();
}

ObjectPatchPtr AGDSEngine::createObjectPatch(const Common::String &objectName) {
	auto & patch = _objectPatches[objectName];
	if (!patch)
		patch = ObjectPatchPtr(new ObjectPatch());
	return patch;
}

void AGDSEngine::saveScreenPatch() {
	if (!_currentScreen || _currentScreenName.empty())
		return;

	PatchPtr &patch = _patches[_currentScreenName];
	if (!patch)
		patch = PatchPtr(new Patch());
	_currentScreen->save(patch);
	patch->characterPresent = _currentCharacter != nullptr;
	if (_currentCharacter) {
		patch->characterPosition = _currentCharacter->position();
		patch->characterDirection = _currentCharacter->direction();
	}
	patch->defaultMouseCursor = _defaultMouseCursorName;
}


void AGDSEngine::loadScreen(const Common::String &name, ScreenLoadingType loadingType, bool savePatch) {
	_loadingScreen = true;
	debug("loadScreen %s [type: %d, save patch: %d, previous: %s]", name.c_str(), static_cast<int>(loadingType), savePatch, _currentScreenName.c_str());
	if (savePatch)
		saveScreenPatch();
	returnCurrentInventoryObject();
	_inventory.visible(false);
	_mouseMap.hideAll(this);

	auto previousScreenName = _currentScreenName;
	resetCurrentScreen();
	for(uint i = 0; i < _processes.size(); ++i) {
		_processes[i].reset();
	}
	_animations.clear();

	auto patch = getPatch(name);
	bool doPatch = patch && loadingType != ScreenLoadingType::SaveOrLoad;

	_currentScreenName = name;
	auto screenObject = loadObject(name);
	_currentScreen = new Screen(this, screenObject, loadingType, previousScreenName);
	if (doPatch)
		screenObject->allowInitialise(false);

	runProcess(screenObject);

	if (doPatch) {
		_currentScreen->load(patch);
		if (_currentCharacter && patch->characterPresent) {
			_currentCharacter->position(patch->characterPosition);
			_currentCharacter->direction(patch->characterDirection);
		}
		if (!patch->defaultMouseCursor.empty())
			loadDefaultMouseCursor(patch->defaultMouseCursor);
	}
	reAddInventory();
	_loadingScreen = false;
}

void AGDSEngine::resetCurrentScreen() {
	if (_currentRegion) {
		_currentRegion->hide(this);
		_currentRegion = NULL;
	}

	delete _currentScreen;
	_currentScreen = NULL;
	_currentScreenName.clear();
}


void AGDSEngine::runProcesses() {
	for (uint i = 0; i < _processes.size(); ++i) {
		ProcessPtr process = _processes[i];
		if (!process)
			continue;

		if (process->active()) {
			process->run();
		}
		if (process->finished()) {
			debug("deleting process %s", process->getName().c_str());
			_processes[i].reset();
			//FIXME: when the last process exits, remove object from scene
		} else {
			//debug("suspended process %s", process->getName().c_str());
		}
	}
}

Console *AGDSEngine::getConsole() {
	return static_cast<Console *>(getDebugger());
}


void AGDSEngine::newGame() {
	SystemVariable *doneVar = getSystemVariable("done_resources");
	Common::String done = doneVar->getString();
	runObject(done);

	_patches.clear();
	_inventory.clear();
	_globals.clear();
	Console *console = getConsole();
	if (console)
		console->clearVars();

	SystemVariable *initVar = getSystemVariable("init_resources");
	Common::String init = initVar->getString();
	runObject(init);
}

void AGDSEngine::tick() {
	loadNextScreen();
	if (_dialog.tick()) {
		runProcesses();
		return;
	}
	tickInventory();
	runProcesses();
}

Animation *AGDSEngine::loadMouseCursor(const Common::String &name) {
	Animation *animation = loadAnimation(name);
	animation->loop(true);
	animation->phaseVar(Common::String());
	return animation;
}

void AGDSEngine::changeMouseArea(int id, int enabled) {
	if (id < 0) {
		warning("invalid mouse area %d", id);
		return;
	}

	MouseRegion *mouseArea = _mouseMap.find(id);
	if (mouseArea) {
		switch (enabled) {
		case 1:
			debug("enabling mouse area %d", id);
			mouseArea->enable();
			break;
		case 0:
			debug("disabling mouse area %d", id);
			if (_currentRegion) {
				_currentRegion->hide(this);
				_currentRegion = NULL;
			}
			mouseArea->disable(this);
			break;
		case -1:
			debug("removing mouse area %d", id);
			if (_currentRegion) {
				_currentRegion->hide(this);
				_currentRegion = NULL;
			}
			_mouseMap.remove(this, id);
			break;
		default:
			warning("invalid value for changeMouseArea: %d", enabled);
		}
	} else
		warning("mouse area %d could not be found", id);
}

Common::Error AGDSEngine::run() {
	if (!load())
		return Common::kNoGameDataFoundError;

	setDebugger(new Console(this));

	int loadSlot = ConfMan.getInt("save_slot");
	debug("save_slot = %d", loadSlot);
	if (loadSlot >= 0)
		loadGameState(loadSlot);

	Common::EventManager *eventManager = _system->getEventManager();

	uint32 frameStarted = _system->getMillis();
	while (!shouldQuit()) {
		Common::Event event;
		while (eventManager->pollEvent(event)) {
			if (!_currentScreen)
				continue;

			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				Common::String key;

				switch (event.kbd.keycode) {
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
					}
					break;
				case Common::KEYCODE_LCTRL:
					_hintMode = true;
					break;
				default:
					if (event.kbd.ascii)
						key = Common::String(static_cast<char>(event.kbd.ascii));
				};
				if (userEnabled() && !key.empty()) {
					Screen::KeyHandler handler = _currentScreen->findKeyHandler(key);
					if (handler.object) {
						debug("found handler for key %s: %s %08x", key.c_str(), handler.object->getName().c_str(), handler.ip + 7);
						runProcess(handler.object, handler.ip);
					}
				}
			} break;

			case Common::EVENT_KEYUP: {
				switch (event.kbd.keycode) {
				case Common::KEYCODE_LCTRL:
					_hintMode = false;
					break;
				default:
					break;
				}
			} break;

			case Common::EVENT_MOUSEMOVE:
				_mouse = event.mouse;
				if (userEnabled()) {
					MouseRegion *region = _mouseMap.find(_mouse);
					if (region != _currentRegion) {
						if (_currentRegion) {
							MouseRegion *currentRegion = _currentRegion;
							_currentRegion = NULL;
							currentRegion->hide(this);
						}

						if (region) {
							_currentRegion = region;
							region->show(this);
						}
					}
					_inventory.visible(_inventoryRegion ? !_mouseMap.disabled() && _inventoryRegion->pointIn(_mouse) : false);
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
				_mouse = event.mouse;
				if (userEnabled()) {
					bool lclick = event.type == Common::EVENT_LBUTTONDOWN;
					debug("%s %d, %d", lclick ? "lclick" : "rclick", _mouse.x, _mouse.y);
					if (!lclick && _currentInventoryObject && !_currentInventoryObject->useOnHandler()) {
						returnCurrentInventoryObject();
						break;
					}

					auto objects = _currentScreen->find(_mouse);
					if (objects.empty() && !_currentInventoryObject) { //allow inventory to be selected
						auto object = _inventory.find(_mouse);
						if (object)
							objects.push_back(object);
					}

					for(auto & object : objects) {
						debug("found object %s", object->getName().c_str());
						uint ip;
						if (lclick) {
							if (_currentInventoryObject) {
								ip = object->getUseHandler(_currentInventoryObject->getName());
								if (!ip) {
									ip = _currentInventoryObject->useOnHandler();
									if (ip)
										object = _currentInventoryObject;
								}
								if (ip)
									debug("found use handler for current inventory object %s", _currentInventoryObject->getName().c_str());
							} else {
								ip = object->getClickHandler();
								if (ip)
									debug("found click handler");
							}
						} else {
							if (_currentInventoryObject) {
								ip = _currentInventoryObject->throwHandler();
								if (ip)
									object = _currentInventoryObject;
							} else {
								ip = object->getExamineHandler();
							}
							if (ip)
								debug("found examine handler");
						}

						if (ip) {
							debug("found handler: %s %08x", object->getName().c_str(), ip + 7);
							runProcess(object, ip);
							break;
						} else {
							debug("no handler found");
							auto scroll = _currentScreen->scrollPosition();
							scroll.x += _mouse.x - g_system->getWidth() / 2;
							_currentScreen->scrollTo(scroll);
						}
					}
				}
				break;
			default:
				break;
			}
		}

		_soundManager.tick();
		if (active())
			tick();

		Graphics::Surface *backbuffer = _system->lockScreen();
		backbuffer->fillRect(backbuffer->getRect(), 0);

		Animation *mouseCursor = NULL;

		if (userEnabled() && _currentScreen) {
			auto objects = _currentScreen->find(_mouse);
			if (objects.empty()) {
				auto object = _inventory.find(_mouse);
				if (object)
					objects.push_back(object);
			}


			Animation *cursor = nullptr;
			for(auto & object : objects) {
				cursor = object->getMouseCursor();
				if (cursor)
					break;
			}

			if (cursor)
				mouseCursor = cursor;

			for(auto & object : objects) {
				if (!object->title().empty()) {
					auto & title = object->title();
					auto font = getFont(getSystemVariable("objtext_font")->getInteger());
					int w = font->getStringWidth(title);
					int x = getSystemVariable("objtext_x")->getInteger() - w / 2;
					int y = getSystemVariable("objtext_y")->getInteger();
					font->drawString(backbuffer, title, x, y, backbuffer->w - x, 0);
				}
			}
		}

		if (_mjpgPlayer) {
			_mjpgPlayer->paint(*this, *backbuffer);

			if (_mjpgPlayer->eos()) {
				skipFilm();
			}
		} else if (_currentScreen) {
			_currentScreen->tick();
			_currentScreen->paint(*backbuffer);
		}

		if (!mouseCursor)
			mouseCursor = _defaultMouseCursor;

		if (userEnabled()) {
			if (_currentInventoryObject) {
				auto picture = _currentInventoryObject->getPicture();
				Common::Rect srcRect = picture->getRect();
				Common::Point dst = _mouse;
				dst.x -= srcRect.width() / 2;
				dst.y -= srcRect.height() / 2;
				uint32 color = (_currentInventoryObject->alpha() << 24) | 0xffffff; //fixme: _picture->format.ARGBToColor(_alpha, 255, 255, 255); is not working
				if (Common::Rect::getBlitRect(dst, srcRect, backbuffer->getRect())) {
					picture->blit(*backbuffer, dst.x, dst.y, Graphics::FLIP_NONE, &srcRect, color);
				}
			} else if (mouseCursor) {
				mouseCursor->tick();
				mouseCursor->paint(*backbuffer, _mouse);
			}
		}

		if (_textLayout.valid()) {
			if (_syncSoundId >= 0) {
				if (!_soundManager.playing(_syncSoundId)) {
					_textLayout.reset(*this);
					_syncSoundId = -1;
					_tellTextTimer = 0;
				}
			} else if (_tellTextTimer > 0) {
				--_tellTextTimer;
			} else {
				_tellTextTimer = 0;
				_textLayout.reset(*this);
			}
		}

		if (_textLayout.valid()) {
			_textLayout.paint(*this, *backbuffer);
		}

		_system->unlockScreen();
		_system->updateScreen();

		if (!_fastMode) {
			uint32 ts = _system->getMillis();
			if (_mjpgPlayer) {
				uint32 elapsed = ts - _filmStarted;
				uint32 expected = _mjpgPlayer->getNextFrameTimestamp();
				if (expected > elapsed)
					_system->delayMillis(expected - elapsed);
			} else {
				static const uint32 kFPS = 25;
				static const uint32 kMaxTick = 1000 / kFPS;

				uint32 dt = ts - frameStarted;
				if (dt < kMaxTick)
					_system->delayMillis(kMaxTick - dt);
			}
			frameStarted = ts;
		}
	}

	return Common::kNoError;
}

void AGDSEngine::playFilm(Process &process, const Common::String &video, const Common::String &audio, const Common::String &subtitles) {
	delete _mjpgPlayer;
	_mjpgPlayer = nullptr;
	if (_fastMode) {
		debug("fast mode, skipping film");
		process.activate();
		return;
	}

	_filmProcess = process.getName();
	_mjpgPlayer = new MJPGPlayer(_resourceManager.getResource(video), subtitles);
	_soundManager.stopAll();
	_filmStarted = _system->getMillis();
	_syncSoundId = _soundManager.play(process.getName(), Common::String(), audio, Common::String());
}

void AGDSEngine::skipFilm() {
	debug("skip");
	delete _mjpgPlayer;
	_mjpgPlayer = NULL;
	if (_syncSoundId >= 0) {
		debug("skip: stopping sound %d", _syncSoundId);
		_mixer->stopID(_syncSoundId);
		_syncSoundId = -1;
	}
	_tellTextTimer = 0;
	_textLayout.reset(*this);
	reactivate(_filmProcess);
}

int AGDSEngine::appendToSharedStorage(const Common::String &value) {
	int index = _sharedStorageIndex;
	_sharedStorage[-2 - (_sharedStorageIndex--)] = value;
	if (_sharedStorageIndex <= -12)
		_sharedStorageIndex = -2;
	return index;
}

const Common::String &AGDSEngine::getSharedStorage(int id) const {
	int index = -2 - id;
	if (index < 0 || index >= 10)
		error("shared storage id is out of range");
	return _sharedStorage[index];
}

void AGDSEngine::setGlobal(const Common::String &name, int value) {
	debug("setting global %s -> %d", name.c_str(), value);
	bool create = !_globals.contains(name);
	_globals.setVal(name, value);
	if (create) {
		Console *console = getConsole();
		if (console)
			console->registerVar(name, &_globals[name]);
	}
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

Animation *AGDSEngine::loadAnimation(const Common::String &name) {
	debug("loadAnimation %s", name.c_str());
	AnimationsType::iterator i = _animations.find(name);
	if (i != _animations.end())
		return i->_value;

	Common::SeekableReadStream *stream = _resourceManager.getResource(name);
	if (!stream)
		error("could not load animation from %s", name.c_str());
	Animation *animation = new Animation(this);
	if (!animation->load(stream, name))
		error("could not load animation from %s", name.c_str());

	_animations[name] = animation;
	return animation;
}

Animation *AGDSEngine::findAnimationByPhaseVar(const Common::String &phaseVar) {
	return _currentScreen? _currentScreen->findAnimationByPhaseVar(phaseVar): nullptr;
}

void AGDSEngine::loadCharacter(const Common::String &id, const Common::String &filename, const Common::String &object) {
	debug("loadCharacter %s %s %s", id.c_str(), filename.c_str(), object.c_str());

	delete _currentCharacter;

	_currentCharacterName = id;
	_currentCharacterFilename = filename;
	_currentCharacterObject = object;

	_currentCharacter = new Character(this, id);
	_currentCharacter->load(_resourceManager.getResource(loadText(filename)));
	_currentCharacter->associate(object);
}

Graphics::TransparentSurface *AGDSEngine::loadPicture(const Common::String &name) {
	return convertToTransparent(_resourceManager.loadPicture(name, _pixelFormat));
}

int AGDSEngine::loadFromCache(const Common::String &name) const {
	PictureCacheLookup::const_iterator i = _pictureCacheLookup.find(name);
	return i != _pictureCacheLookup.end() ? i->_value : -1;
}

int AGDSEngine::saveToCache(const Common::String &name, Graphics::TransparentSurface *surface) {
	if (!surface)
		return -1;
	int id = _pictureCacheId++;
	_pictureCacheLookup[name] = id;
	_pictureCache[id] = surface;
	return id;
}

Graphics::TransparentSurface *AGDSEngine::loadFromCache(int id) const {
	PictureCacheType::const_iterator i = _pictureCache.find(id);
	return (i != _pictureCache.end()) ? i->_value : NULL;
}

void AGDSEngine::loadFont(int id, const Common::String &name, int gw, int gh) {
	debug("loadFont %d %s %d %d", id, name.c_str(), gw, gh);
	Graphics::TransparentSurface *surface = loadPicture(name);
	Font *&font = _fonts[id];
	delete font;
	font = new Font(surface, gw, gh);
}

Font *AGDSEngine::getFont(int id) const {
	FontsType::const_iterator i = _fonts.find(id);
	if (i == _fonts.end())
		error("no font with id %d", id);
	return i->_value;
}

Graphics::Surface *AGDSEngine::createSurface(int w, int h) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(w, h, _pixelFormat);
	return surface;
}

Graphics::TransparentSurface *AGDSEngine::convertToTransparent(Graphics::Surface *surface) {
	if (!surface)
		return NULL;
	Graphics::TransparentSurface *t = new Graphics::TransparentSurface(*surface, true);
	assert(t->format.bytesPerPixel == 4);
	uint32* pixels = static_cast<uint32 *>(t->getPixels());
	uint8 shadowAlpha = 255 * _shadowIntensity / 100;
	uint delta = t->pitch - t->w * t->format.bytesPerPixel;
	for (uint16 i = 0; i < t->h; ++i, pixels = reinterpret_cast<uint32*>((reinterpret_cast<uint8*>(pixels) + delta))) {
		for (uint16 j = 0; j < t->w; ++j, ++pixels) {
			uint32 pix = *pixels;
			uint8 r, g, b, a;
			t->format.colorToARGB(pix, a, r, g, b);
			if (r == _colorKey.r && g == _colorKey.g && b == _colorKey.b) {
				r = g = b = a = 0;
			} else if (
				r >= _minShadowColor.r && r <= _maxShadowColor.r &&
				g >= _minShadowColor.g && g <= _maxShadowColor.g &&
				b >= _minShadowColor.b && b <= _maxShadowColor.b
			) {
				r = g = b = 0;
				a = shadowAlpha;
			} else
				continue;

			*pixels = t->format.ARGBToColor(a, r, g, b);
		}
	}
	surface->free();
	delete surface;
	return t;
}

void AGDSEngine::addSystemVar(const Common::String &name, SystemVariable *var) {
	_systemVarList.push_back(name);
	_systemVars[name] = var;
}

void AGDSEngine::tell(Process &process, const Common::String &regionName, Common::String text, Common::String sound, bool npc) {
	if (getSystemVariable("tell_close_inv")->getInteger())
		_inventory.visible(false);

	int font_id = getSystemVariable(npc? "npc_tell_font": "tell_font")->getInteger();
	Common::Point pos;

	if (regionName.empty()) {
		pos.x = getSystemVariable("subtitle_x")->getInteger();
		pos.y = getSystemVariable("subtitle_y")->getInteger();
	} else {
		RegionPtr region = loadRegion(regionName);
		pos = region->center;
	}

	if (text.empty())
		text = _dialog.getNextDialogLine();
	if (sound.empty())
		sound = _dialog.getNextDialogSound();

	_tellTextTimer = _dialog.textDelay(text);
	_textLayout.layout(*this, process, text, pos, font_id, npc);
	if (!sound.empty()) {
		playSoundSync(sound);
	} else {
		_syncSoundId = -1;
	}
}


void AGDSEngine::initSystemVariables() {
	addSystemVar("inventory_scr", new StringSystemVariable());
	addSystemVar("escape_scr", new StringSystemVariable("none"));
	addSystemVar("load_scr", new StringSystemVariable());
	addSystemVar("save_scr", new StringSystemVariable());
	addSystemVar("gfx_bright", new IntegerSystemVariable(50));
	addSystemVar("gfx_contrast", new IntegerSystemVariable(50));
	addSystemVar("sound_volume", new IntegerSystemVariable(100));
	addSystemVar("music_volume", new IntegerSystemVariable(80));
	addSystemVar("tell_volume", new IntegerSystemVariable(100));
	addSystemVar("text_speed", new IntegerSystemVariable(70));
	addSystemVar("tell_mode", new IntegerSystemVariable(3));
	addSystemVar("version", new IntegerSystemVariable(1));
	addSystemVar("objtext_x", new IntegerSystemVariable(-1));
	addSystemVar("objtext_y", new IntegerSystemVariable(-1));
	addSystemVar("objtext_mode", new IntegerSystemVariable(-1));
	addSystemVar("objtext_font", new IntegerSystemVariable(-1));
	addSystemVar("inv_open", new StringSystemVariable());
	addSystemVar("inv_close", new StringSystemVariable());
	addSystemVar("inv_region", new StringSystemVariable());
	addSystemVar("anim_zoom", new IntegerSystemVariable(1));
	addSystemVar("screen_curtain", new IntegerSystemVariable(1));
	addSystemVar("music_curtain", new IntegerSystemVariable(1));
	addSystemVar("sound_curtain", new IntegerSystemVariable(1));
	addSystemVar("old_music_volume", new IntegerSystemVariable());
	addSystemVar("old_sound_volume", new IntegerSystemVariable());
	addSystemVar("old_screen_fade", new IntegerSystemVariable());
	addSystemVar("subtitle_x", new IntegerSystemVariable());
	addSystemVar("subtitle_y", new IntegerSystemVariable());
	addSystemVar("subtitle_type", new IntegerSystemVariable(3));
	addSystemVar("subtitles", new IntegerSystemVariable());
	addSystemVar("tell_font", new IntegerSystemVariable());
	addSystemVar("npc_tell_font", new IntegerSystemVariable());
	addSystemVar("edit_font", new IntegerSystemVariable());
	addSystemVar("delay_after_tell", new IntegerSystemVariable());
	addSystemVar("scroll_factor", new IntegerSystemVariable(30));
	addSystemVar("dialog_var", new IntegerSystemVariable());
	addSystemVar("subtitle_width", new IntegerSystemVariable(-1));
	addSystemVar("flash_mouse", new IntegerSystemVariable());
	addSystemVar("scale_char", new IntegerSystemVariable());
	addSystemVar("init_resources", new StringSystemVariable());
	addSystemVar("done_resources", new StringSystemVariable());
	addSystemVar("tell_close_inv", new IntegerSystemVariable(1));
	addSystemVar("gamma", new IntegerSystemVariable());
}

SystemVariable *AGDSEngine::getSystemVariable(const Common::String &name) {
	SystemVariablesType::iterator i = _systemVars.find(name);
	if (i != _systemVars.end())
		return i->_value;

	error("no system variable %s", name.c_str());
}

void AGDSEngine::tickInventory() {
	const Common::String &inv_region_name = getSystemVariable("inv_region")->getString();
	if (!inv_region_name.empty()) {
		if (!_inventoryRegion || _inventoryRegionName != inv_region_name) {
			_inventoryRegionName = inv_region_name;
			_inventoryRegion = loadRegion(inv_region_name);
		}
	} else {
		_inventoryRegionName.clear();
		_inventoryRegion.reset();
	}

}

bool AGDSEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsSubtitleOptions:
	case kSupportsReturnToLauncher:
	case kSupportsChangingOptionsDuringRuntime:
		return true;
	default:
		return false;
	}
}

void AGDSEngine::loadPatches(Common::SeekableReadStream *file, Database & db) {
	debug("loading patches");
	_patches.clear();
	_objectPatches.clear();
	Common::Array<Common::String> entries = db.getEntries();
	for (uint i = 0; i < entries.size(); ++i) {
		const Common::String & name = entries[i];
		if (name[0] == '_')
			continue;
		debug("loading patch for %s", name.c_str());
		Common::ScopedPtr<Common::SeekableReadStream> patchStream(db.getEntry(file, name));
		if (patchStream->size() != ObjectPatch::Size) {
			PatchPtr patch(new Patch());
			patch->load(patchStream.get());
			_patches[name] = patch;
		} else {
			ObjectPatchPtr patch(new ObjectPatch());
			patch->load(patchStream.get());
			_objectPatches[name] = patch;
		}
	}
	debug("done loading patches");
}


Common::Error AGDSEngine::loadGameState(int slot) {
	//saveAutosaveIfEnabled();

	auto fileName = getSaveStateName(slot);
	Common::InSaveFile *saveFile = _saveFileMan->openForLoading(fileName);

	if (!saveFile)
		return Common::kReadingFailed;

	Database db;
	if (!db.open(fileName, saveFile))
		return Common::kReadingFailed;

	_soundManager.stopAll();

	{
		// Compiled version (should be 2)
		Common::ScopedPtr<Common::SeekableReadStream> agds_ver(db.getEntry(saveFile, "__agds_ver"));
		int version = agds_ver->readUint32LE();
		debug("version: %d", version);
		if (version != 2) {
			warning("wrong engine version (%d)", version);
			return Common::kReadingFailed;
		}
	}

	SystemVariable *doneVar = getSystemVariable("done_resources");
	Common::String done = doneVar->getString();
	if (!done.empty())
		runObject(done);

	{
		// Current character
		Common::ScopedPtr<Common::SeekableReadStream> agds_c(db.getEntry(saveFile, "__agds_c"));
		Common::String object = readString(agds_c.get());
		Common::String filename = readString(agds_c.get());
		Common::String id = readString(agds_c.get());
		debug("savegame character %s %s -> %s %s", object.c_str(), filename.c_str(), filename.c_str(), id.c_str());
		loadCharacter(id, filename, object);
		auto character = getCharacter(id);
		if (character) {
			character->loadState(agds_c.get());
		} else
			warning("no character");
	}

	Common::String screenName;
	{
		// Palette and screen name
		Common::ScopedPtr<Common::SeekableReadStream> agds_s(db.getEntry(saveFile, "__agds_s"));
		screenName = readString(agds_s.get());
	}

	{
		// Global vars
		_globals.clear();
		Common::ScopedPtr<Common::SeekableReadStream> agds_v(db.getEntry(saveFile, "__agds_v"));
		uint32 n = agds_v->readUint32LE();
		debug("reading %u vars...", n);
		while(n--) {
			Common::String name = readString(agds_v.get());
			int value = agds_v->readSint32LE();
			debug("setting var %s to %d", name.c_str(), value);
			setGlobal(name, value);
		}
	}

	{
		// System vars
		Common::ScopedPtr<Common::SeekableReadStream> agds_d(db.getEntry(saveFile, "__agds_d"));
		for(uint i = 0, n = _systemVarList.size(); i < n; ++i) {
			Common::String & name = _systemVarList[i];
			_systemVars[name]->read(agds_d.get());
		}
	}
	_mouseMap.clear();

	SystemVariable *initVar = getSystemVariable("init_resources");
	runObject(initVar->getString());

	loadPatches(saveFile, db);
	loadScreen(screenName, ScreenLoadingType::Normal, false);

	{
		// Saved ambient sound
		Common::ScopedPtr<Common::SeekableReadStream> agds_a(db.getEntry(saveFile, "__agds_a"));
		auto resource = readString(agds_a.get());
		auto filename = loadText(resource);
		auto phaseVar = readString(agds_a.get());
		uint volume = agds_a->readUint32LE();
		uint type = agds_a->readUint32LE();
		debug("saved audio state: sample: '%s:%s', var: '%s' %u %u", resource.c_str(), filename.c_str(), phaseVar.c_str(), volume, type);
		debug("phase var for sample -> %d", getGlobal(phaseVar));
		_ambientSoundId = playSound(Common::String(), resource, filename, phaseVar); //fixme: double check
		debug("ambient sound id = %d", _ambientSoundId);
	}
	{
		Common::ScopedPtr<Common::SeekableReadStream> agds_i(db.getEntry(saveFile, "__agds_i"));
		_inventory.load(agds_i.get());
	}

	delete saveFile;
	return Common::kNoError;
}

void AGDSEngine::loadNextScreen() {
	while (!_nextScreenName.empty()) {
		Common::String nextScreenName = _nextScreenName;
		debug("loadNextScreen %s", nextScreenName.c_str());
		auto nextScreenType = _nextScreenType;
		_nextScreenName.clear();
		_nextScreenType = ScreenLoadingType::Normal;
		loadScreen(nextScreenName, nextScreenType);
	}
}

Common::Error AGDSEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	debug("saveGameState %d %s autosave: %d", slot, desc.c_str(), isAutosave);
	auto fileName = getSaveStateName(slot);
	Common::OutSaveFile *saveFile = getSaveFileManager()->openForSaving(fileName, false);

	if (!saveFile)
		return Common::kWritingFailed;

	while(_currentScreen && _currentScreen->loadingType() == ScreenLoadingType::SaveOrLoad) {
		returnToPreviousScreen();
		loadNextScreen();
	}

	Common::HashMap<Common::String, Common::Array<uint8>> entries;

	{
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		stream.writeUint32LE(2);
		entries["__agds_ver"].assign(stream.getData(), stream.getData() + stream.size());
	}

	{
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		writeString(&stream, _currentCharacterObject);
		writeString(&stream, _currentCharacterFilename);
		writeString(&stream, _currentCharacterName);
		auto character = getCharacter(_currentCharacterName);
		if (character) {
			character->saveState(&stream);
		} else
			warning("no character to save");

		auto size = stream.size();
		if (size < 106) {
			Common::Array<unsigned char> filler(106 - size);
			stream.write(filler.data(), filler.size());
		}
		entries["__agds_c"].assign(stream.getData(), stream.getData() + stream.size());
	}

	{
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		writeString(&stream, _currentScreenName);
		debug("saving screen name: %s", _currentScreenName.c_str());
		char palette[0x300];
		memset(palette, 0xaa, sizeof(palette));
		stream.write(palette, sizeof(palette));
		entries["__agds_s"].assign(stream.getData(), stream.getData() + stream.size());
	}

	{
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		for(uint i = 0, n = _systemVarList.size(); i < n; ++i) {
			Common::String & name = _systemVarList[i];
			_systemVars[name]->write(&stream);
		}
		entries["__agds_d"].assign(stream.getData(), stream.getData() + stream.size());
	}

	{
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		auto n = _globals.size();
		stream.writeUint32LE(n);
		debug("saving %u vars...", n);
		for(auto & global : _globals) {
			writeString(&stream, global._key);
			stream.writeUint32LE(global._value);
		}
		entries["__agds_v"].assign(stream.getData(), stream.getData() + stream.size());
	}

	{
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		_inventory.save(&stream);
		entries["__agds_i"].assign(stream.getData(), stream.getData() + stream.size());
	}

	{
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		debug("ambient sound id: %d", _ambientSoundId);
		auto sound = _soundManager.find(_ambientSoundId);
		if (sound) {
			writeString(&stream, sound->resource);
			writeString(&stream, sound->phaseVar);
		} else {
			writeString(&stream, Common::String());
			writeString(&stream, Common::String());
		}
		stream.writeUint32LE(70); //volume
		stream.writeUint32LE(30); //type

		entries["__agds_a"].assign(stream.getData(), stream.getData() + stream.size());
	}

	for(auto & objectPatch : _objectPatches) {
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		objectPatch._value->save(&stream);
		entries[objectPatch._key].assign(stream.getData(), stream.getData() + stream.size());
	}

	for(auto & patch : _patches) {
		Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::YES);
		patch._value->save(&stream);
		entries[patch._key].assign(stream.getData(), stream.getData() + stream.size());
	}

	Database::write(saveFile, entries);

	delete saveFile;
	return Common::kNoError;
}


void AGDSEngine::reactivate(const Common::String &name, bool runNow) {
	if (name.empty())
		return;

	for(uint i = 0; i < _processes.size(); ++i) {
		ProcessPtr &process = _processes[i];
		if (process && process->getName() == name) {
			debug("reactivate %s", name.c_str());
			process->activate();
			if (runNow)
				process->run();
		}
	}
}

void AGDSEngine::stopProcess(const Common::String & name) {
	for(uint i = 0; i < _processes.size(); ++i) {
		ProcessPtr &process = _processes[i];
		if (process && process->getName() == name) {
			debug("stopping %s...", name.c_str());
			process->done();
		}
	}
}

void AGDSEngine::currentInventoryObject(const ObjectPtr & object) {
	_currentInventoryObject = object;
}

void AGDSEngine::resetCurrentInventoryObject() {
	_currentInventoryObject.reset();
}

void AGDSEngine::returnCurrentInventoryObject() {
	auto object = _currentInventoryObject;
	if (!object)
		return;

	_currentInventoryObject.reset();

	_inventory.add(object);
	runObject(object);
}

void AGDSEngine::setNextScreenName(const Common::String &nextScreenName, ScreenLoadingType type) {
	debug("setNextScreenName %s:%d", nextScreenName.c_str(), static_cast<int>(type));
	_nextScreenName = nextScreenName;
	_nextScreenType = type;
}

void AGDSEngine::returnToPreviousScreen() {
	auto previousScreenName = _currentScreen? _currentScreen->getPreviousScreenName(): Common::String();
	debug("returnToPreviousScreen from %s, previous screen: %s", _currentScreenName.c_str(), previousScreenName.c_str());
	if (!previousScreenName.empty()) {
		_nextScreenName = previousScreenName;
		_nextScreenType = ScreenLoadingType::Previous;
	}
}

} // End of namespace AGDS
