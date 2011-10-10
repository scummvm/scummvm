/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_fgetc
#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_getwd
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_stdin

#if defined(WIN32)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one... - this is needed before including util.h
#undef ARRAYSIZE
#endif

#include "common/archive.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/config-manager.h"

#include "gui/error.h"
#include "gui/gui-manager.h"

#include "engines/engine.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/lua.h"
#include "engines/grim/actor.h"
#include "engines/grim/movie/movie.h"
#include "engines/grim/savegame.h"
#include "engines/grim/registry.h"
#include "engines/grim/resource.h"
#include "engines/grim/localize.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/lab.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/font.h"
#include "engines/grim/primitives.h"
#include "engines/grim/objectstate.h"
#include "engines/grim/set.h"

#include "engines/grim/lua/lualib.h"

#include "engines/grim/imuse/imuse.h"

#include "lua/lobject.h"
#include "lua/lstate.h"

namespace Grim {

static bool g_lua_initialized = false;

// CHAR_KEY tests to see whether a keycode is for
// a "character" handler or a "button" handler
#define CHAR_KEY(k) ((k >= 'a' && k <= 'z') || (k >= 'A' && k <= 'Z') || (k >= '0' && k <= '9') || k == ' ')

GrimEngine *g_grim = NULL;
GfxBase *g_driver = NULL;
int g_imuseState = -1;

GrimEngine::GrimEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language) :
		Engine(syst), _currSet(NULL), _selectedActor(NULL) {
	g_grim = this;

	_gameType = gameType;
	_gameFlags = gameFlags;
	_gamePlatform = platform;
	_gameLanguage = language;

	g_registry = new Registry();
	g_resourceloader = NULL;
	g_localizer = NULL;
	g_movie = NULL;
	g_imuse = NULL;

	_showFps = (tolower(g_registry->get("show_fps", "false")[0]) == 't');

#ifdef USE_OPENGL
	_softRenderer = (tolower(g_registry->get("soft_renderer", "false")[0]) == 't');
#else
	_softRenderer = true;
#endif

	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_currSet = NULL;
	_selectedActor = NULL;
	_talkingActor = NULL;
	_controlsEnabled = new bool[KEYCODE_EXTRA_LAST];
	_controlsState = new bool[KEYCODE_EXTRA_LAST];
	for (int i = 0; i < KEYCODE_EXTRA_LAST; i++) {
		_controlsEnabled[i] = false;
		_controlsState[i] = false;
	}
	_speechMode = TextAndVoice;
	_textSpeed = 7;
	_mode = _previousMode = NormalMode;
	_flipEnable = true;
	int speed = atol(g_registry->get("engine_speed", "30"));
	if (speed <= 0 || speed > 100)
		_speedLimitMs = 30;
	else
		_speedLimitMs = 1000 / speed;
	char buf[20];
	sprintf(buf, "%d", 1000 / _speedLimitMs);
	g_registry->set("engine_speed", buf);
	_refreshDrawNeeded = true;
	_listFilesIter = NULL;
	_savedState = NULL;
	_fps[0] = 0;
	_iris = new Iris();

	PoolColor *c = new PoolColor(0, 0, 0);
	new PoolColor(255, 255, 255); // Default color for actors. Id == 2

	_printLineDefaults.setX(0);
	_printLineDefaults.setY(100);
	_printLineDefaults.setWidth(0);
	_printLineDefaults.setHeight(0);
	_printLineDefaults.setFGColor(c);
	_printLineDefaults.setFont(NULL);
	_printLineDefaults.setJustify(TextObject::LJUSTIFY);
	_printLineDefaults.setDisabled(false);

	_sayLineDefaults.setX(0);
	_sayLineDefaults.setY(100);
	_sayLineDefaults.setWidth(0);
	_sayLineDefaults.setHeight(0);
	_sayLineDefaults.setFGColor(c);
	_sayLineDefaults.setFont(NULL);
	_sayLineDefaults.setJustify(TextObject::CENTER);
	_sayLineDefaults.setDisabled(false);

	_blastTextDefaults.setX(0);
	_blastTextDefaults.setY(200);
	_blastTextDefaults.setWidth(0);
	_blastTextDefaults.setHeight(0);
	_blastTextDefaults.setFGColor(c);
	_blastTextDefaults.setFont(NULL);
	_blastTextDefaults.setJustify(TextObject::LJUSTIFY);
	_blastTextDefaults.setDisabled(false);

	// Add 'movies' subdirectory for the demo
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "movies");

	Debug::registerDebugChannels();
}

GrimEngine::~GrimEngine() {
	delete[] _controlsEnabled;
	delete[] _controlsState;

	Set::getPool()->deleteObjects();
	Actor::getPool()->deleteObjects();
	PrimitiveObject::getPool()->deleteObjects();
	TextObject::getPool()->deleteObjects();
	Bitmap::getPool()->deleteObjects();
	Font::getPool()->deleteObjects();
	ObjectState::getPool()->deleteObjects();
	PoolColor::getPool()->deleteObjects();

	if (g_lua_initialized) {
		lua_removelibslists();
		lua_close();
		lua_iolibclose();
		g_lua_initialized = false;
	}
	if (g_registry) {
		g_registry->save();
		delete g_registry;
		g_registry = NULL;
	}
	delete g_movie;
	g_movie = NULL;
	delete g_imuse;
	g_imuse = NULL;
	delete g_localizer;
	g_localizer = NULL;
	delete g_resourceloader;
	g_resourceloader = NULL;
	delete g_driver;
	g_driver = NULL;
	delete _iris;
}

Common::Error GrimEngine::run() {
	g_resourceloader = new ResourceLoader();
	g_localizer = new Localizer();
	bool demo = getGameFlags() & ADGF_DEMO;
	if (getGameType() == GType_GRIM)
		g_movie = CreateSmushPlayer(demo);
	else if (getGameType() == GType_MONKEY4) {
		if (_gamePlatform == Common::kPlatformPS2)
			g_movie = CreateMpegPlayer();
		else
			g_movie = CreateBinkPlayer(demo);
	}
	g_imuse = new Imuse(20, demo);

	bool fullscreen = (tolower(g_registry->get("fullscreen", "false")[0]) == 't');

	if (!_softRenderer && !g_system->hasFeature(OSystem::kFeatureOpenGL)){
		warning("gfx backend doesn't support hardware rendering");
		_softRenderer=true;
	}

	if (_softRenderer)
		g_driver = CreateGfxTinyGL();
#ifdef USE_OPENGL
	else
		g_driver = CreateGfxOpenGL();
#endif

	g_driver->setupScreen(640, 480, fullscreen);

	// refresh the theme engine so that we can show the gui overlay without it crashing.
	GUI::GuiManager::instance().theme()->refresh();

	Bitmap *splash_bm = NULL;
	if (!(_gameFlags & ADGF_DEMO) && getGameType() == GType_GRIM)
		splash_bm = g_resourceloader->loadBitmap("splash.bm");
	else if ((_gameFlags & ADGF_DEMO) && getGameType() == GType_MONKEY4)
		splash_bm = g_resourceloader->loadBitmap("splash.til");

	g_driver->clearScreen();

	if (!(_gameFlags & ADGF_DEMO) && getGameType() == GType_GRIM)
		splash_bm->draw();
	else if ((_gameFlags & ADGF_DEMO) && getGameType() == GType_MONKEY4)
		splash_bm->draw();

	g_driver->flipBuffer();

	lua_iolibopen();
	lua_strlibopen();
	lua_mathlibopen();

	if (getGameType() == GType_GRIM) {
		registerGrimOpcodes();

		// FIXME/HACK: see PutActorInSet
		const char *func = "function reset_doorman() doorman_in_hot_box = FALSE end";
		lua_pushstring(func);
		lua_call("dostring");
	} else
		registerMonkeyOpcodes();

	registerLua();
	g_lua_initialized = true;

	bundle_dofile("_system.lua");

	lua_pushnil();		// resumeSave
	lua_pushnil();		// bootParam - not used in scripts
	lua_call("BOOT");

	_savegameLoadRequest = false;
	_savegameSaveRequest = false;


	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		int slot = ConfMan.getInt("save_slot");
		assert(slot <= 99);
		assert(slot >= 0);
		char saveName[16];
		sprintf(saveName, "grim%02d.gsv", slot);
		_savegameLoadRequest = true;
		_savegameFileName = saveName;
	}

	g_grim->setMode(NormalMode);
	if (splash_bm)
		delete splash_bm;
	g_grim->mainLoop();

	return Common::kNoError;
}

int GrimEngine::bundle_dofile(const char *filename) {
	Block *b = g_resourceloader->getFileBlock(filename);
	if (!b) {
		delete b;
		// Don't print warnings on Scripts\foo.lua,
		// d:\grimFandango\Scripts\foo.lua
		if (!strstr(filename, "Scripts\\"))
			Debug::warning(Debug::Engine, "Cannot find script %s", filename);

		return 2;
	}

	int result = lua_dobuffer(const_cast<char *>(b->getData()), b->getLen(), const_cast<char *>(filename));
	delete b;
	return result;
}

int GrimEngine::single_dofile(const char *filename) {
	Common::File *f = new Common::File();

	if (!f->open(filename)) {
		delete f;
		Debug::warning(Debug::Engine, "Cannot find script %s", filename);

		return 2;
	}

	int32 size = f->size();
	char *data = new char[size];
	f->read(data, size);

	int result = lua_dobuffer(data, size, const_cast<char *>(filename));
	delete f;
	delete[] data;

	return result;
}

extern int refSystemTable;

void GrimEngine::handlePause() {
	lua_Object func;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("pauseHandler");
	lua_Object handler = lua_gettable();
	if (lua_istable(handler)) {
		lua_pushobject(handler);
		lua_pushstring("pauseHandler");
		func = lua_gettable();
		if (!lua_isfunction(func))
			error("handlePause: handler not a function");
		lua_pushobject(handler);
	} else if (lua_isfunction(handler)) {
		func = handler;
	} else if (!lua_isnil(handler)) {
		error("handlePause: invalid handler");
		return;
	} else {
		lua_endblock();
		return;
	}

	lua_callfunction(func);

	lua_endblock();
}

void GrimEngine::handleExit() {
	lua_Object func;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("exitHandler");
	lua_Object handler = lua_gettable();
	if (lua_istable(handler)) {
		lua_pushobject(handler);
		lua_pushstring("exitHandler");
		func = lua_gettable();
		if (!lua_isfunction(func))
			error("handleExit: handler not a function");
		lua_pushobject(handler);
	} else if (lua_isfunction(handler)) {
		func = handler;
	} else if (!lua_isnil(handler)) {
		error("handleExit: invalid handler");
		return;
	} else {
		lua_endblock();
		return;
	}

	lua_callfunction(func);

	lua_endblock();
}

void GrimEngine::handleUserPaint() {
	lua_Object func;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("userPaintHandler");
	lua_Object handler = lua_gettable();
	if (lua_istable(handler)) {
		lua_pushobject(handler);
		lua_pushstring("userPaintHandler");
		func = lua_gettable();
		if (!lua_isfunction(func))
			error("handleUserPaint: handler not a function");
		lua_pushobject(handler);
	} else if (lua_isfunction(handler)) {
		func = handler;
	} else if (!lua_isnil(handler)) {
		error("handleUserPaint: invalid handler");
		return;
	} else {
		lua_endblock();
		return;
	}

	lua_callfunction(func);

	lua_endblock();
}

void GrimEngine::handleChars(int operation, int key, int /*keyModifier*/, uint16 ascii) {
	lua_Object func;
	char keychar[2];

	if (!CHAR_KEY(ascii))
		return;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("characterHandler");
	lua_Object handler = lua_gettable();
	if (lua_istable(handler)) {
		lua_pushobject(handler);
		lua_pushstring("characterHandler");
		func = lua_gettable();
		if (!lua_isfunction(func))
			error("handleChars: handler not a function");
		lua_pushobject(handler);
	} else if (lua_isfunction(handler)) {
		func = handler;
	} else if (!lua_isnil(handler)) {
		error("handleChars: invalid handler");
		return;
	} else {
		lua_endblock();
		return;
	}

	keychar[0] = ascii;
	keychar[1] = 0;
	lua_pushstring(keychar);
	lua_callfunction(func);

	lua_endblock();
}

void GrimEngine::handleControls(int operation, int key, int /*keyModifier*/, uint16 ascii) {
	lua_Object buttonFunc, joyFunc;
	bool buttonFuncIsTable;

	// If we're not supposed to handle the key then don't
	if (!_controlsEnabled[key])
		return;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("buttonHandler");
	lua_Object buttonHandler = lua_gettable();
	if (lua_istable(buttonHandler)) {
		lua_pushobject(buttonHandler);
		lua_pushstring("buttonHandler");
		buttonFunc = lua_gettable();
		if (!lua_isfunction(buttonFunc)) {
			error("handleControls: button handler not a function");
			return;
		}
		buttonFuncIsTable = true;
	} else if (lua_isfunction(buttonHandler)) {
		buttonFunc = buttonHandler;
		buttonFuncIsTable = false;
	} else {
		error("handleControls: invalid keys handler");
		return;
	}

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("axisHandler");
	lua_Object joyHandler = lua_gettable();
	if (lua_istable(joyHandler)) {
		lua_pushobject(joyHandler);
		lua_pushstring("axisHandler");
		joyFunc = lua_gettable();
		if (!lua_isfunction(joyFunc)) {
			error("handleControls: joystick handler not a function");
			return;
		}
	} else if (lua_isfunction(joyHandler)) {
		joyFunc = joyHandler;
	} else {
		error("handleControls: invalid joystick handler");
		return;
	}
	if (buttonFuncIsTable)
		lua_pushobject(buttonHandler);
	lua_pushnumber(key);
	if (operation == Common::EVENT_KEYDOWN) {
		lua_pushnumber(1);
		lua_pushnumber(1);
	} else {
		lua_pushnil();
		lua_pushnumber(0);
	}
	lua_pushnumber(0);
	lua_callfunction(buttonFunc);

	if (operation == Common::EVENT_KEYDOWN)
		_controlsState[key] = true;
	else if (operation == Common::EVENT_KEYUP)
		_controlsState[key] = false;

	lua_endblock();
}

void GrimEngine::handleDebugLoadResource() {
	void *resource = NULL;
	int c, i = 0;
	char buf[512];

	// Tool for debugging the loading of a particular resource without
	// having to actually make it all the way to it in the game
	fprintf(stderr, "Enter resource to load (extension specifies type): ");
	while (i < 512 && (c = fgetc(stdin)) != EOF && c != '\n')
		buf[i++] = c;

	buf[i] = '\0';
	if (strstr(buf, ".key"))
		resource = (void *)g_resourceloader->loadKeyframe(buf);
	else if (strstr(buf, ".zbm") || strstr(buf, ".bm"))
		resource = (void *)g_resourceloader->loadBitmap(buf);
	else if (strstr(buf, ".cmp"))
		resource = (void *)g_resourceloader->loadColormap(buf);
	else if (strstr(buf, ".cos"))
		resource = (void *)g_resourceloader->loadCostume(buf, NULL);
	else if (strstr(buf, ".lip"))
		resource = (void *)g_resourceloader->loadLipSync(buf);
	else if (strstr(buf, ".snm"))
		resource = (void *)g_movie->play(buf, false, 0, 0);
	else if (strstr(buf, ".wav") || strstr(buf, ".imu")) {
		g_imuse->startSfx(buf);
		resource = (void *)1;
	} else if (strstr(buf, ".mat")) {
		CMap *cmap = g_resourceloader->loadColormap("item.cmp");
		warning("Default colormap applied to resources loaded in this fashion");
		resource = (void *)g_resourceloader->loadMaterial(buf, cmap);
	} else {
		warning("Resource type not understood");
	}
	if (!resource)
		warning("Requested resouce (%s) not found", buf);
}

static void cameraChangeHandle(int prev, int next) {
	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("camChangeHandler");
	lua_Object func = lua_gettable();

	if (lua_isfunction(func)) {
		lua_pushnumber(prev);
		lua_pushnumber(next);
		lua_callfunction(func);
	}

	lua_endblock();
}

static void cameraPostChangeHandle(int num) {
	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("postCamChangeHandler");
	lua_Object func = lua_gettable();

	if (lua_isfunction(func)) {
		lua_pushnumber(num);
		lua_callfunction(func);
	}

	lua_endblock();
}

void GrimEngine::drawPrimitives() {
	_iris->draw();

	// Draw text
	for (TextObject::Pool::Iterator i = TextObject::getPool()->getBegin();
		 i != TextObject::getPool()->getEnd(); ++i) {
		i->_value->draw();
	}
}

void GrimEngine::playIrisAnimation(Iris::Direction dir, int x, int y, int time) {
	_iris->play(dir, x, y, time);
}

void GrimEngine::luaUpdate() {
	if (_savegameLoadRequest || _savegameSaveRequest)
		return;

	// Update timing information
	unsigned newStart = g_system->getMillis();
	if (newStart < _frameStart) {
		_frameStart = newStart;
		return;
	}
	_frameTime = newStart - _frameStart;
	_frameStart = newStart;

	if (_mode == PauseMode || _shortFrame) {
		_frameTime = 0;
	}

	_frameTimeCollection += _frameTime;
	if (_frameTimeCollection > 10000) {
		_frameTimeCollection = 0;
		lua_collectgarbage(0);
	}

	lua_beginblock();
	setFrameTime(_frameTime);
	lua_endblock();

	lua_beginblock();
	setMovieTime(_movieTime);
	lua_endblock();

	// Run asynchronous tasks
	lua_runtasks();

	if (_currSet && (_mode == NormalMode || _mode == SmushMode)) {
		// Update the actors. Do it here so that we are sure to react asap to any change
		// in the actors state caused by lua.
		for (Actor::Pool::Iterator i = Actor::getPool()->getBegin(); i != Actor::getPool()->getEnd(); ++i) {
			Actor *a = i->_value;

			// Note that the actor need not be visible to update chores, for example:
			// when Manny has just brought Meche back he is offscreen several times
			// when he needs to perform certain chores
			if (a->isInSet(_currSet->getName()))
				a->update(_frameTime);
		}

		_iris->update(_frameTime);
	}

	for (TextObject::Pool::Iterator i = TextObject::getPool()->getBegin();
		 i != TextObject::getPool()->getEnd(); i++) {
		i->_value->update();
	}
}

void GrimEngine::updateDisplayScene() {
	_doFlip = true;

	if (_mode == SmushMode) {
		if (g_movie->isPlaying()) {
			_movieTime = g_movie->getMovieTime();
			if (g_movie->isUpdateNeeded()) {
				g_driver->prepareMovieFrame(g_movie->getDstSurface());
				g_movie->clearUpdateNeeded();
			}
			int frame = g_movie->getFrame();
			if (frame >= 0) {
				if (frame != _prevSmushFrame) {
					_prevSmushFrame = g_movie->getFrame();
					g_driver->drawMovieFrame(g_movie->getX(), g_movie->getY());
					if (_showFps)
						g_driver->drawEmergString(550, 25, _fps, Color(255, 255, 255));
				} else
					_doFlip = false;
			} else
				g_driver->releaseMovieFrame();
		}
		// Draw Primitives
		for (PrimitiveObject::Pool::Iterator i = PrimitiveObject::getPool()->getBegin();
			i != PrimitiveObject::getPool()->getEnd(); ++i) {
			i->_value->draw();
		}
		drawPrimitives();
	} else if (_mode == NormalMode) {
		if (!_currSet)
			return;

		cameraPostChangeHandle(_currSet->getSetup());

		g_driver->clearScreen();

		_prevSmushFrame = 0;
		_movieTime = 0;

		_currSet->drawBackground();

		// Draw underlying scene components
		// Background objects are drawn underneath everything except the background
		// There are a bunch of these, especially in the tube-switcher room
		_currSet->drawBitmaps(ObjectState::OBJSTATE_BACKGROUND);

		// Underlay objects are just above the background
		_currSet->drawBitmaps(ObjectState::OBJSTATE_UNDERLAY);

		// State objects are drawn on top of other things, such as the flag
		// on Manny's message tube
		_currSet->drawBitmaps(ObjectState::OBJSTATE_STATE);

		// Play SMUSH Animations
		// This should occur on top of all underlying scene objects,
		// a good example is the tube switcher room where some state objects
		// need to render underneath the animation or you can't see what's going on
		// This should not occur on top of everything though or Manny gets covered
		// up when he's next to Glottis's service room
		if (g_movie->isPlaying()) {
			_movieTime = g_movie->getMovieTime();
			if (g_movie->isUpdateNeeded()) {
				g_driver->prepareMovieFrame(g_movie->getDstSurface());
				g_movie->clearUpdateNeeded();
			}
			if (g_movie->getFrame() >= 0)
				g_driver->drawMovieFrame(g_movie->getX(), g_movie->getY());
			else
				g_driver->releaseMovieFrame();
		}

		// Draw Primitives
		for (PrimitiveObject::Pool::Iterator i = PrimitiveObject::getPool()->getBegin();
			 i != PrimitiveObject::getPool()->getEnd(); ++i) {
			i->_value->draw();
		}

		_currSet->setupCamera();

		g_driver->set3DMode();

		_currSet->setupLights();

		// Draw actors
		for (Actor::Pool::Iterator i = Actor::getPool()->getBegin(); i != Actor::getPool()->getEnd(); ++i) {
			Actor *a = i->_value;
			if (a->isInSet(_currSet->getName()) && a->isVisible())
				a->draw();
			a->undraw(a->isInSet(_currSet->getName()) && a->isVisible());
		}
		flagRefreshShadowMask(false);

		// Draw overlying scene components
		// The overlay objects should be drawn on top of everything else,
		// including 3D objects such as Manny and the message tube
		_currSet->drawBitmaps(ObjectState::OBJSTATE_OVERLAY);

		drawPrimitives();
	} else if (_mode == DrawMode) {
		_doFlip = false;
		_prevSmushFrame = 0;
		_movieTime = 0;
	}
}

void GrimEngine::doFlip() {
	if (_showFps && _doFlip)
		g_driver->drawEmergString(550, 25, _fps, Color(255, 255, 255));

	if (_doFlip && _flipEnable)
		g_driver->flipBuffer();

	if (_showFps && _doFlip && _mode != DrawMode) {
		_frameCounter++;
		unsigned int currentTime = g_system->getMillis();
		unsigned int delta = currentTime - _lastFrameTime;
		if (delta > 500) {
			sprintf(_fps, "%7.2f", (double)(_frameCounter * 1000) / (double)delta );
			_frameCounter = 0;
			_lastFrameTime = currentTime;
		}
	}
}

void GrimEngine::mainLoop() {
	_movieTime = 0;
	_frameTime = 0;
	_frameStart = g_system->getMillis();
	_frameCounter = 0;
	_lastFrameTime = 0;
	_frameTimeCollection = 0;
	_prevSmushFrame = 0;
	_refreshShadowMask = false;
	_shortFrame = false;
	bool resetShortFrame = false;

	for (;;) {
		uint32 startTime = g_system->getMillis();
		if (_shortFrame) {
			if (resetShortFrame) {
				_shortFrame = false;
			}
			resetShortFrame = !resetShortFrame;
		}

		if (_savegameLoadRequest) {
			savegameRestore();
		}
		if (_savegameSaveRequest) {
			savegameSave();
		}

		g_imuse->flushTracks();
		g_imuse->refreshScripts();

		// Process events
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			// Handle any buttons, keys and joystick operations
			if (event.type == Common::EVENT_KEYDOWN) {
				if (_mode != DrawMode && _mode != SmushMode && (event.kbd.ascii == 'q')) {
					handleExit();
					break;
				} else {
					handleChars(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
				}
			}
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP) {
				handleControls(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
			}
			// Check for "Hard" quit"
			if (event.type == Common::EVENT_QUIT)
				return;
			if (event.type == Common::EVENT_SCREEN_CHANGED)
				_refreshDrawNeeded = true;

			// Allow lua to react to the event.
			// Without this lua_update switching the entries in the menu is slow because
			// if the button is not kept pressed the KEYUP will arrive just after the KEYDOWN
			// and it will break the lua scripts that checks for the state of the button
			// with GetControlState()
			luaUpdate();
		}

		luaUpdate();

		if (_mode != PauseMode) {
			updateDisplayScene();
			doFlip();
		}

		if (g_imuseState != -1) {
			g_imuse->setMusicState(g_imuseState);
			g_imuseState = -1;
		}

		uint32 endTime = g_system->getMillis();
		if (startTime > endTime)
			continue;
		uint32 diffTime = endTime - startTime;
		if (_speedLimitMs == 0)
			continue;
		if (diffTime < _speedLimitMs) {
			uint32 delayTime = _speedLimitMs - diffTime;
			g_system->delayMillis(delayTime);
		}
	}
}

void GrimEngine::saveGame(const Common::String &file) {
	_savegameFileName = file;
	_savegameSaveRequest = true;
}

void GrimEngine::loadGame(const Common::String &file) {
	_savegameFileName = file;
	_savegameLoadRequest = true;
}

void GrimEngine::savegameRestore() {
	debug("GrimEngine::savegameRestore() started.");
	_savegameLoadRequest = false;
	Common::String filename;
	if (_savegameFileName.size() == 0) {
		filename = "grim.sav";
	} else {
		filename = _savegameFileName;
	}
	_savedState = SaveGame::openForLoading(filename);
	if (!_savedState || _savedState->saveVersion() != SaveGame::SAVEGAME_VERSION)
		return;
	g_imuse->stopAllSounds();
	g_imuse->resetState();
	g_movie->stop();
	g_imuse->pause(true);
	g_movie->pause(true);

	_selectedActor = NULL;
	_talkingActor = NULL;
	delete _currSet;
	_currSet = NULL;

	PoolColor::getPool()->restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "Colors restored succesfully.");

	Bitmap::getPool()->restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "Bitmaps restored succesfully.");

	Font::getPool()->restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "Fonts restored succesfully.");

	ObjectState::getPool()->restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "ObjectStates restored succesfully.");

	Set::getPool()->restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "Sets restored succesfully.");

	TextObject::getPool()->restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "TextObjects restored succesfully.");

	PrimitiveObject::getPool()->restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "PrimitiveObjects restored succesfully.");

	Actor::getPool()->restoreObjects(_savedState);
	Debug::debug(Debug::Engine, "Actors restored succesfully.");

	restoreGRIM();
	Debug::debug(Debug::Engine, "Engine restored succesfully.");

	g_driver->restoreState(_savedState);
	Debug::debug(Debug::Engine, "Renderer restored succesfully.");

	g_imuse->restoreState(_savedState);
	Debug::debug(Debug::Engine, "iMuse restored succesfully.");

	g_movie->restoreState(_savedState);
	Debug::debug(Debug::Engine, "Movie restored succesfully.");

	_iris->restoreState(_savedState);
	Debug::debug(Debug::Engine, "Iris restored succesfully.");

	lua_Restore(_savedState);
	Debug::debug(Debug::Engine, "Lua restored succesfully.");

	delete _savedState;

	// Apply the patch, only if it wasn't applied already.
	if (lua_isnil(lua_getglobal("  service_release.lua"))) {
		if (bundle_dofile("patch05.bin") == 2)
			single_dofile("patch05.bin");
	}
	const char *devMode = g_registry->get("good_times", "");
	lua_beginblock();
	// Set the developerMode, since the save contains the value of
	// the installation it was made with.
	lua_pushobject(lua_getglobal("developerMode"));
	if (devMode[0] == 0)
		lua_pushnil();
	else
		lua_pushstring(devMode);
	lua_setglobal("developerMode");
	lua_endblock();

	g_imuse->pause(false);
	g_movie->pause(false);
	debug("GrimEngine::savegameRestore() finished.");

	_shortFrame = true;
	clearEventQueue();
}

void GrimEngine::restoreGRIM() {
	_savedState->beginSection('GRIM');

	_mode = (EngineMode)_savedState->readLEUint32();
	_previousMode = (EngineMode)_savedState->readLEUint32();

	// Actor stuff
	int32 id = _savedState->readLEUint32();
	if (id != 0) {
		_selectedActor = Actor::getPool()->getObject(id);
	}
	_talkingActor = Actor::getPool()->getObject(_savedState->readLEUint32());

	//TextObject stuff
	_sayLineDefaults.setDisabled(_savedState->readLESint32());
	_sayLineDefaults.setFGColor(PoolColor::getPool()->getObject(_savedState->readLEUint32()));
	_sayLineDefaults.setFont(Font::getPool()->getObject(_savedState->readLEUint32()));
	_sayLineDefaults.setHeight(_savedState->readLESint32());
	_sayLineDefaults.setJustify(_savedState->readLESint32());
	_sayLineDefaults.setWidth(_savedState->readLESint32());
	_sayLineDefaults.setX(_savedState->readLESint32());
	_sayLineDefaults.setY(_savedState->readLESint32());
	_sayLineDefaults.setDuration(_savedState->readLESint32());

	// Set stuff
	_currSet = Set::getPool()->getObject(_savedState->readLEUint32());

	_savedState->endSection();
}

void GrimEngine::storeSaveGameImage(SaveGame *state) {
	int width = 250, height = 188;
	Bitmap *screenshot;

	debug("GrimEngine::StoreSaveGameImage() started.");

	EngineMode mode = g_grim->getMode();
	g_grim->setMode(_previousMode);
	g_grim->updateDisplayScene();
	g_driver->storeDisplay();
	screenshot = g_driver->getScreenshot(width, height);
	g_grim->setMode(mode);
	state->beginSection('SIMG');
	if (screenshot) {
		int size = screenshot->getWidth() * screenshot->getHeight();
		screenshot->setActiveImage(0);
		uint16 *data = (uint16 *)screenshot->getData();
		for (int l = 0; l < size; l++) {
			state->writeLEUint16(data[l]);
		}
	} else {
		error("Unable to store screenshot");
	}
	state->endSection();
	delete screenshot;
	debug("GrimEngine::StoreSaveGameImage() finished.");
}

void GrimEngine::savegameSave() {
	debug("GrimEngine::savegameSave() started.");
	_savegameSaveRequest = false;
	char filename[200];
	if (_savegameFileName.size() == 0) {
		strcpy(filename, "grim.sav");
	} else {
		strcpy(filename, _savegameFileName.c_str());
	}
	_savedState = SaveGame::openForSaving(filename);
	if (!_savedState) {
		//TODO: Translate this!
		GUI::displayErrorDialog("Error: the game could not be saved.");
		return;
	}

	storeSaveGameImage(_savedState);

	g_imuse->pause(true);
	g_movie->pause(true);

	savegameCallback();

	PoolColor::getPool()->saveObjects(_savedState);
	Debug::debug(Debug::Engine, "Colors saved succesfully.");

	Bitmap::getPool()->saveObjects(_savedState);
	Debug::debug(Debug::Engine, "Bitmaps saved succesfully.");

	Font::getPool()->saveObjects(_savedState);
	Debug::debug(Debug::Engine, "Fonts saved succesfully.");

	ObjectState::getPool()->saveObjects(_savedState);
	Debug::debug(Debug::Engine, "ObjectStates saved succesfully.");

	Set::getPool()->saveObjects(_savedState);
	Debug::debug(Debug::Engine, "Sets saved succesfully.");

	TextObject::getPool()->saveObjects(_savedState);
	Debug::debug(Debug::Engine, "TextObjects saved succesfully.");

	PrimitiveObject::getPool()->saveObjects(_savedState);
	Debug::debug(Debug::Engine, "PrimitiveObjects saved succesfully.");

	Actor::getPool()->saveObjects(_savedState);
	Debug::debug(Debug::Engine, "Actors saved succesfully.");

	saveGRIM();
	Debug::debug(Debug::Engine, "Engine saved succesfully.");

	g_driver->saveState(_savedState);
	Debug::debug(Debug::Engine, "Renderer saved succesfully.");

	g_imuse->saveState(_savedState);
	Debug::debug(Debug::Engine, "iMuse saved succesfully.");

	g_movie->saveState(_savedState);
	Debug::debug(Debug::Engine, "Movie saved succesfully.");

	_iris->saveState(_savedState);
	Debug::debug(Debug::Engine, "Iris saved succesfully.");

	lua_Save(_savedState);

	delete _savedState;

	g_imuse->pause(false);
	g_movie->pause(false);
	debug("GrimEngine::savegameSave() finished.");

	_shortFrame = true;
	clearEventQueue();
}

void GrimEngine::saveGRIM() {
	_savedState->beginSection('GRIM');

	_savedState->writeLEUint32((uint32)_mode);
	_savedState->writeLEUint32((uint32)_previousMode);

	//Actor stuff
	if (_selectedActor) {
		_savedState->writeLEUint32(_selectedActor->getId());
	} else {
		_savedState->writeLEUint32(0);
	}
	if (_talkingActor) {
		_savedState->writeLEUint32(_talkingActor->getId());
	} else {
		_savedState->writeLEUint32(0);
	}

	//TextObject stuff
	_savedState->writeLESint32(_sayLineDefaults.getDisabled());
	_savedState->writeLEUint32(_sayLineDefaults.getFGColor()->getId());
	_savedState->writeLEUint32(_sayLineDefaults.getFont()->getId());
	_savedState->writeLESint32(_sayLineDefaults.getHeight());
	_savedState->writeLESint32(_sayLineDefaults.getJustify());
	_savedState->writeLESint32(_sayLineDefaults.getWidth());
	_savedState->writeLESint32(_sayLineDefaults.getX());
	_savedState->writeLESint32(_sayLineDefaults.getY());
	_savedState->writeLESint32(_sayLineDefaults.getDuration());

	//Set stuff
	_savedState->writeLEUint32(_currSet->getId());

	_savedState->endSection();
}

void GrimEngine::savegameCallback() {
	lua_Object funcParam1;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("saveGameCallback");
	lua_Object funcParam2 = lua_gettable();

	if (lua_istable(funcParam2)) {
		lua_pushobject(funcParam2);
		lua_pushstring("saveGameCallback");
		funcParam1 = lua_gettable();
		if (lua_isfunction(funcParam1)) {
			lua_pushobject(funcParam2);
			lua_callfunction(funcParam1);
		} else {
			error("GrimEngine::savegameCallback: handler is not a function");
		}
	} else if (lua_isfunction(funcParam2)) {
		funcParam1 = funcParam2;
		lua_callfunction(funcParam1);
	} else if (!lua_isnil(funcParam2)) {
		error("GrimEngine::savegameCallback: invalid handler");
	}
	lua_endblock();
}

Set *GrimEngine::findSet(const Common::String &name) {
	// Find scene object
	for (Set::Pool::Iterator i = Set::getPool()->getBegin(); i != Set::getPool()->getEnd(); ++i) {
		if (i->_value->getName() == name)
			return i->_value;
	}
	return NULL;
}

void GrimEngine::setSetLock(const char *name, bool lockStatus) {
	Set *scene = findSet(name);

	if (!scene) {
		Debug::warning(Debug::Engine, "Set object '%s' not found in list", name);
		return;
	}
	// Change the locking status
	scene->_locked = lockStatus;
}

Set *GrimEngine::loadSet(const Common::String &name) {
	Set *s = findSet(name);

	if (!s) {
		Common::String filename(name);
		// EMI-scripts refer to their .setb files as .set
		if (g_grim->getGameType() == GType_MONKEY4) {
			filename += "b";
		}
		Block *b = g_resourceloader->getFileBlock(filename);
		if (!b)
			warning("Could not find scene file %s", name.c_str());
		s = new Set(name, b->getData(), b->getLen());
		delete b;
	}

	return s;
}

void GrimEngine::setSet(const char *name) {
	setSet(loadSet(name));
}

void GrimEngine::setSet(Set *scene) {
	if (scene == _currSet)
		return;

	// Stop the actors. This fixes bug #289 (https://github.com/residual/residual/issues/289)
	// and it makes sense too, since when changing set the directions
	// and coords change too.
	for (Actor::Pool::Iterator i = Actor::getPool()->getBegin(); i != Actor::getPool()->getEnd(); ++i) {
		Actor *a = i->_value;
		a->stopWalking();
	}

	Set *lastSet = _currSet;
	_currSet = scene;
	_currSet->setSoundParameters(20, 127);
	_currSet->setLightsDirty();
	// should delete the old scene after setting the new one
	if (lastSet && !lastSet->_locked) {
		delete lastSet;
	}
	_shortFrame = true;
}

void GrimEngine::makeCurrentSetup(int num) {
	int prevSetup = g_grim->getCurrSet()->getSetup();
	if (prevSetup != num) {
		getCurrSet()->setSetup(num);
		getCurrSet()->setSoundParameters(20, 127);
		cameraChangeHandle(prevSetup, num);
		// here should be set sound position
	}
}

void GrimEngine::setTextSpeed(int speed) {
	if (speed < 1)
		_textSpeed = 1;
	if (speed > 10)
		_textSpeed = 10;
	_textSpeed = speed;
}

float GrimEngine::getControlAxis(int num) {
	return 0;
}

bool GrimEngine::getControlState(int num) {
	return _controlsState[num];
}

float GrimEngine::getPerSecond(float rate) const {
	return rate * _frameTime / 1000;
}

Actor *GrimEngine::getTalkingActor() const {
	return _talkingActor;
}

void GrimEngine::setTalkingActor(Actor *a) {
	_talkingActor = a;
}

const Common::String &GrimEngine::getSetName() const {
	return _currSet->getName();
}

void GrimEngine::clearEventQueue() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
	}

	for (int i = 0; i < KEYCODE_EXTRA_LAST; ++i) {
		_controlsState[i] = false;
	}
}

} // end of namespace Grim
