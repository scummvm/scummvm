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
#include "common/system.h"
#include "engines/util.h"
#include "graphics/transparent_surface.h"

namespace AGDS {

AGDSEngine::AGDSEngine(OSystem *system, const ADGameDescription *gameDesc) : Engine(system),
                                                                             _gameDescription(gameDesc), _pictureCacheId(1), _sharedStorageIndex(-2),
                                                                             _mjpgPlayer(), _currentScreen(), _previousScreen(),
                                                                             _defaultMouseCursor(),
                                                                             _mouse(400, 300), _userEnabled(false), _currentRegion(),
                                                                             _random("agds"),
                                                                             _inventoryRegion(),
                                                                             _soundManager(this, system->getMixer()),
                                                                             _fastMode(false),
                                                                             _dialogScriptPos(0) {
}

AGDSEngine::~AGDSEngine() {
	delete _currentScreen;
	delete _previousScreen;
	for (PictureCacheType::iterator i = _pictureCache.begin(); i != _pictureCache.end(); ++i) {
		i->_value->free();
		delete i->_value;
	}
}

bool AGDSEngine::initGraphics(int w, int h) {
	//fixme: get mode from config?
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

	if (!initGraphics(w, h))
		error("no video mode found");

	Common::INIFile::SectionKeyList values = config.getKeys("core");
	for (Common::INIFile::SectionKeyList::iterator i = values.begin(); i != values.end(); ++i) {
		if (i->key == "path")
			if (!_resourceManager.addPath(i->value))
				return false;
	}
	if (!_data.open("data.adb"))
		return false;

	initSystemVariables();
	loadScreen("main");

	Common::File file;
	file.open("patch.adb");
	Database patch;
	patch.open("patch.adb");

	loadPatches(&file, patch);

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
	return ResourceManager::loadText(_data.getEntry(entryName));
}

ObjectPtr AGDSEngine::loadObject(const Common::String &name, const Common::String &prototype) {
	debug("loadObject %s %s", name.c_str(), prototype.c_str());
	Common::String clone = prototype.empty() ? name : prototype;
	Common::SeekableReadStream *stream = _data.getEntry(clone);
	if (!stream)
		error("no database entry for %s\n", clone.c_str());

	ObjectPtr object(new Object(name, stream));
	delete stream;
	return object;
}

void AGDSEngine::runObject(ObjectPtr object) {
	if (_currentScreen)
		_currentScreen->add(object);
	else
		warning("object %s has been loaded, but was not added to any screen", object->getName().c_str());
	runProcess(object);
}

void AGDSEngine::runProcess(ObjectPtr object, uint ip, Process *caller) {
	object->activate(true);
	_processes.push_front(Process(this, object, ip, caller));
	ProcessListType::iterator it = _processes.begin();
	runProcess(it);
}

void AGDSEngine::runObject(const Common::String &name, const Common::String &prototype) {
	ObjectPtr object = getCurrentScreenObject(name);
	if (!object)
		object = loadObject(name, prototype);
	runObject(object);
}

void AGDSEngine::loadScreen(const Common::String &name) {
	debug("loadScreen %s", name.c_str());
	resetCurrentScreen();
	_soundManager.stopAll();
	_currentScreenName = name;
	_currentScreen = new Screen(loadObject(name), _mouseMap);
	_mouseMap.clear();
	runObject(_currentScreen->getObject()); //is it called once or per screen activation?

	PatchesType::const_iterator it = _patches.find(name);
	if (it == _patches.end())
		return;

	PatchPtr patch = it->_value;
	const Common::Array<Patch::Object> &objects = patch->objects;
	debug("found patch with %u objects", objects.size());
	for(uint i = 0; i < objects.size(); ++i) {
		const Patch::Object &object = objects[i];
		debug("patch object %s %d", object.name.c_str(), object.flag);
		if (object.flag <= 0)
			_currentScreen->remove(object.name);
		else if (!_currentScreen->find(object.name)) {
			runObject(object.name);
		}
	}
	loadDefaultMouseCursor(patch->defaultMouseCursor);
}

void AGDSEngine::setCurrentScreen(Screen *screen) {
	if (!screen)
		error("no previous screen");

	resetCurrentScreen();

	_currentScreenName = screen->getName();
	_currentScreen = screen;
	_previousScreen = NULL;
	_previousScreenName.clear();
}

void AGDSEngine::resetCurrentScreen() {
	if (_currentRegion) {
		if (_currentRegion->currentlyIn)
			runObject(_currentRegion->onLeave);
		_currentRegion = NULL;
	}

	if (_currentScreen != _previousScreen) //we didnt come from back command, fixme: refactor it
		delete _currentScreen;
	_currentScreen = NULL;
	_currentScreenName.clear();
}

void AGDSEngine::runProcess(ProcessListType::iterator &it) {
	Process &process = *it;
	if (process.parentScreenName() != _currentScreenName) {
		if (process.active())
			process.activate(false);
		it = _processes.erase(it);
		return;
	}

	if (!process.active()) {
		++it;
		return;
	}

	const Common::String &name = process.getName();
	if (process.getStatus() == Process::kStatusDone || process.getStatus() == Process::kStatusError) {
		debug("process %s finished", name.c_str());
		process.activate(false);
		it = _processes.erase(it);
		return;
	}
	process.activate(true);
	ProcessExitCode code = process.execute();
	bool destroy = false;
	switch (code) {
	case kExitCodeDestroy:
		destroy = true;
		break;
	case kExitCodeLoadScreenObjectAs:
	case kExitCodeLoadScreenObject:
		runObject(process.getExitArg1(), process.getExitArg2());
		break;
	case kExitCodeRunDialog:
		_dialogProcessName = process.getExitArg1();
		break;
	case kExitCodeSetNextScreen:
		loadScreen(process.getExitArg1());
		destroy = true;
		break;
	case kExitCodeSetNextScreenSaveInHistory:
		if (_currentScreen) {
			delete _previousScreen;
			_previousScreen = _currentScreen;
			_previousScreenName = _currentScreenName;
		}
		loadScreen(process.getExitArg1());
		destroy = true;
		break;
	case kExitCodeLoadPreviousScreenObject:
		if (_previousScreen)
			setCurrentScreen(_previousScreen);
		else if (!_previousScreenName.empty()) {
			loadScreen(_previousScreenName);
			_previousScreenName.clear();
		}
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
		{
			debug("exitProcessCreatePatch");

			SystemVariable *doneVar = getSystemVariable("done_resources");
			Common::String done = doneVar->getString();
			runObject(done);

			_patches.clear();
			_inventory.clear();
			_globals.clear();

			SystemVariable *initVar = getSystemVariable("init_resources");
			Common::String init = initVar->getString();
			runObject(init);
		}
		break;
	case kExitCodeLoadSaveGame:
		loadGameState(process.getExitIntArg1());
		break;
	default:
		error("unknown process exit code %d", code);
	}
	if (destroy) {
		debug("destroying process %s...", name.c_str());
		process.activate(false);
		it = _processes.erase(it);
	} else
		++it;
}

void AGDSEngine::tick() {
	if (tickDialog())
		return;
	tickInventory();
	for (ProcessListType::iterator p = _processes.begin(); active() && p != _processes.end();) {
		runProcess(p);
	}
}

Animation *AGDSEngine::loadMouseCursor(const Common::String &name) {
	Animation *animation = loadAnimation(name);
	animation->loop(true);
	animation->phaseVar(Common::String());
	animation->play();
	return animation;
}

void AGDSEngine::changeMouseArea(int id, int enabled) {
	if (id < 0)
		return;

	MouseRegion *mouseArea = _mouseMap.find(id);
	if (mouseArea) {
		switch (enabled) {
		case 1:
			debug("enabling mouse area %d", id);
			mouseArea->enable();
			break;
		case 0:
			debug("disabling mouse area %d", id);
			if (mouseArea->currentlyIn) {
				runObject(mouseArea->onLeave);
			}
			mouseArea->disable();
			break;
		case -1:
			debug("removing mouse area %d", id);
			_mouseMap.remove(id);
			break;
		}
	} else
		warning("mouse area %d could not be found", id);
}

Common::Error AGDSEngine::run() {
	if (!load())
		return Common::kNoGameDataFoundError;

	int loadSlot = ConfMan.getInt("save_slot");
	if (loadSlot >= 0)
		loadGameState(loadSlot);

	Common::EventManager *eventManager = _system->getEventManager();

	while (!shouldQuit()) {
		uint32 frameStarted = _system->getMillis();

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
			} break;
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
					_inventory.enable(_inventoryRegion ? !mouseMap.disabled() && _inventoryRegion->pointIn(_mouse) : false);
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
				_mouse = event.mouse;
				if (_userEnabled) {
					bool lclick = event.type == Common::EVENT_LBUTTONDOWN;
					debug("%s %d, %d", lclick ? "lclick" : "rclick", _mouse.x, _mouse.y);
					ObjectPtr object = _currentScreen->find(_mouse);
					if (object) {
						uint ip = lclick ? object->getClickHandler() : object->getExamineHandler();
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
			ObjectPtr object = _currentScreen->find(_mouse);
			Animation *cursor = object ? object->getMouseCursor() : NULL;
			if (cursor)
				mouseCursor = cursor;
		}

		_soundManager.tick();
		if (active())
			tick();

		Graphics::Surface *backbuffer = _system->lockScreen();
		backbuffer->fillRect(backbuffer->getRect(), 0);

		if (_mjpgPlayer) {
			const Graphics::Surface *surface = _mjpgPlayer->decodeFrame();

			if (surface) {
				Graphics::Surface *converted = surface->convertTo(_pixelFormat);
				Common::Point dst((backbuffer->w - converted->w) / 2, (backbuffer->h - converted->h) / 2);
				Common::Rect srcRect(converted->getRect());
				if (Common::Rect::getBlitRect(dst, srcRect, backbuffer->getRect()))
					backbuffer->copyRectToSurface(*converted, dst.x, dst.y, srcRect);
				converted->free();
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

		for (CharactersType::iterator i = _characters.begin(); i != _characters.end(); ++i)
			i->_value->paint(*this, *backbuffer);

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

const Common::String &AGDSEngine::getSharedStorage(int id) const {
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

Animation *AGDSEngine::loadAnimation(const Common::String &name) {
	debug("loadAnimation %s", name.c_str());
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

Animation *AGDSEngine::findAnimationByPhaseVar(const Common::String &phaseVar) {
	for (AnimationsType::iterator i = _animations.begin(); i != _animations.end(); ++i) {
		Animation *animation = i->_value;
		if (animation->phaseVar() == phaseVar)
			return animation;
	}
	return NULL;
}

Character *AGDSEngine::loadCharacter(const Common::String &id, const Common::String &filename, const Common::String &object) {
	debug("loadCharacter %s %s %s", id.c_str(), filename.c_str(), object.c_str());
	CharactersType::iterator i = _characters.find(id);
	if (i != _characters.end())
		return i->_value;

	Character *character = new Character(id, object);
	character->load(_resourceManager.getResource(filename));
	_characters[id] = character;
	return character;
}

Character *AGDSEngine::getCharacter(const Common::String &name) const {
	CharactersType::const_iterator i = _characters.find(name);
	return i != _characters.end() ? i->_value : NULL;
}

Graphics::TransparentSurface *AGDSEngine::loadPicture(const Common::String &name) { return convertToTransparent(_resourceManager.loadPicture(name, _pixelFormat)); }

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
	t->applyColorKey(0xff, 0, 0xff);
	surface->free();
	delete surface;
	return t;
}

void AGDSEngine::addSystemVar(const Common::String &name, SystemVariable *var) {
	_systemVarList.push_back(name);
	_systemVars[name] = var;
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

void AGDSEngine::runDialog(const Common::String &dialogScript, const Common::String &defs) {
	parseDialogDefs(defs);
	_dialogScript = dialogScript;
	_dialogScriptPos = 0;
	getSystemVariable("dialog_var")->setInteger(-1);
}

void AGDSEngine::parseDialogDefs(const Common::String &defs) {
	Common::String name, value;
	bool readName = true;
	for (uint32 p = 0, size = defs.size(); p < size; ++p) {
		char ch = defs[p];
		if (ch == ' ') {
			continue;
		} else if (ch == '\n' || ch == '\r') {
			//debug("dialog definition: '%s' = '%s'", name.c_str(), value.c_str());
			if (!name.empty() && !value.empty()) {
				_dialogDefs[name] = atoi(value.c_str());
			}
			readName = true;
			name.clear();
			value.clear();
			continue;
		} else if (ch == '=') {
			if (readName) {
				readName = false;
			} else {
				warning("equal sign in value, skipping");
			}
		} else {
			if (readName)
				name += ch;
			else
				value += ch;
		}
	}
}

void AGDSEngine::tickInventory() {
	if (!_inventory.enabled() && _inventory.visible()) {
		debug("closing inventory...");
		Common::String inv_close = getSystemVariable("inv_close")->getString();
		if (!inv_close.empty())
			runObject(inv_close);
		_inventory.visible(false);
	} else if (_inventory.enabled() && !_inventory.visible()) {
		debug("opening inventory...");
		Common::String inv_open = getSystemVariable("inv_open")->getString();
		if (!inv_open.empty())
			runObject(inv_open);
		_inventory.visible(true);
	}

	const Common::String &inv_region_name = getSystemVariable("inv_region")->getString();
	if (inv_region_name.empty())
		return;

	_inventoryRegion = loadRegion(inv_region_name);
}

bool AGDSEngine::tickDialog() {
	if (_dialogProcessName.empty())
		return false;

	int dialog_var = getSystemVariable("dialog_var")->getInteger();
	if (dialog_var > 0) {
		getSystemVariable("dialog_var")->setInteger(-3);
		return false;
	} else if (dialog_var < 0) {
		getSystemVariable("dialog_var")->setInteger(0);
		return true;
	}

	uint n = _dialogScript.size();
	if (_dialogScriptPos >= n)
		return false;

	Common::String line;
	while (_dialogScriptPos < n && _dialogScript[_dialogScriptPos] != '\n' && _dialogScript[_dialogScriptPos] != '\r') {
		line += _dialogScript[_dialogScriptPos++];
	}
	++_dialogScriptPos;

	if (line.empty())
		return true;

	if (line[0] == '@') {
		if (line[1] == '@') //comment
			return true;

		line.erase(0, 1);

		if (line.hasPrefix("sound")) {
			debug("sound: %s", line.c_str());
		} else {
			DialogDefsType::const_iterator it = _dialogDefs.find(line);
			if (it != _dialogDefs.end()) {
				int value = it->_value;
				debug("dialog value %d (0x%04x)", value, value);
				getSystemVariable("dialog_var")->setInteger(value);
			} else
				warning("invalid dialog directive: %s", line.c_str());
		}
	} else if (line[0] == ' ') {
		debug("text: %s", line.c_str() + 1);
	}
	if (_dialogScriptPos >= n && !_dialogProcessName.empty()) {
		Common::String process = _dialogProcessName;
		_dialogProcessName.clear();

		debug("end of dialog, running %s", process.c_str());
		runObject(process);
		getSystemVariable("dialog_var")->setInteger(-2);
		return false;
	}
	return true;
}

bool AGDSEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsSubtitleOptions:
	case kSupportsReturnToLauncher:
	case kSupportsLoadingDuringRuntime:
	case kSupportsSavingDuringRuntime:
	case kSupportsChangingOptionsDuringRuntime:
		return true;
	default:
		return Engine::hasFeature(f);
	}
}

void AGDSEngine::loadPatches(Common::SeekableReadStream *file, Database & db) {
	debug("loading patches");
	_patches.clear();
	Common::Array<Common::String> entries = db.getEntries();
	for (uint i = 0; i < entries.size(); ++i) {
		const Common::String & name = entries[i];
		if (name[0] == '_')
			continue;
		debug("loading patch for %s", name.c_str());
		Common::ScopedPtr<Common::SeekableReadStream> patchStream(db.getEntry(file, name));
		PatchPtr patch(new Patch());
		patch->load(patchStream.get());
		_patches[name] = patch;
	}
	debug("done loading patches");
}


Common::Error AGDSEngine::loadGameStream(Common::SeekableReadStream *file) {
	Database db;
	if (!db.open("savefile", file))
		return Common::kReadingFailed;

	{
		// Compiled version (should be 2)
		Common::ScopedPtr<Common::SeekableReadStream> agds_ver(db.getEntry(file, "__agds_ver"));
		int version = agds_ver->readUint32LE();
		debug("version: %d", version);
		if (version != 2) {
			warning("wrong engine version (%d)", version);
			return Common::kReadingFailed;
		}
	}

	{
		// Current character
		Common::ScopedPtr<Common::SeekableReadStream> agds_c(db.getEntry(file, "__agds_c"));
		Common::String object = readString(agds_c.get());
		Common::String name = readString(agds_c.get());
		Common::String id = readString(agds_c.get());
		Common::String filename = loadText(name);
		debug("savegame character %s %s -> %s %s", object.c_str(), name.c_str(), filename.c_str(), id.c_str());
		loadCharacter(id, filename, object);
		int x = agds_c->readUint16LE();
		int y = agds_c->readUint16LE();
		debug("character at %d, %d", x, y);
		int n = 3;
		while(n--) {
			int v = agds_c->readUint16LE();
			debug("savegame character leftover: %d", v);
		}
	}

	Common::String screenName;
	{
		// Screenshot and screen name
		Common::ScopedPtr<Common::SeekableReadStream> agds_s(db.getEntry(file, "__agds_s"));
		screenName = readString(agds_s.get());
	}

	{
		// Global vars
		_globals.clear();
		Common::ScopedPtr<Common::SeekableReadStream> agds_v(db.getEntry(file, "__agds_v"));
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
		// Audio samples
		Common::ScopedPtr<Common::SeekableReadStream> agds_a(db.getEntry(file, "__agds_a"));
		Common::String sample = loadText(readString(agds_a.get()));
		Common::String phaseVar = readString(agds_a.get());
		uint unk0 = agds_a->readUint32LE();
		uint unk1 = agds_a->readUint32LE();
		debug("saved audio state: sample: %s, var: %s %u %u", sample.c_str(), phaseVar.c_str(), unk0, unk1);
		playSound(sample, phaseVar);
	}

	{
		// System vars
		Common::ScopedPtr<Common::SeekableReadStream> agds_d(db.getEntry(file, "__agds_d"));
		for(uint i = 0, n = _systemVarList.size(); i < n; ++i) {
			Common::String & name = _systemVarList[i];
			_systemVars[name]->read(agds_d.get());
		}
	}

	SystemVariable *initVar = getSystemVariable("init_resources");
	runObject(initVar->getString());

	loadPatches(file, db);
	loadScreen(screenName);

	{
		// Inventory
		_inventory.clear();
		Common::ScopedPtr<Common::SeekableReadStream> agds_i(db.getEntry(file, "__agds_i"));
		int n = 34;
		while(n--) {
			Common::String name = readString(agds_i.get());
			int unk = agds_i->readUint32LE();
			int present = agds_i->readUint32LE();
			if (!name.empty() && present) {
				debug("inventory: %s %d %d", name.c_str(), unk, present);
				ObjectPtr object = loadObject(name);
				runObject(name);
				_inventory.add(object);
			}
		}
	}

	return Common::kNoError;
}

Common::Error AGDSEngine::saveGameStream(Common::WriteStream *file, bool isAutosave) { return Common::Error(Common::kNoError); }

} // End of namespace AGDS
