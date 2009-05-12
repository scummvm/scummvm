/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"

#include "engine/engine.h"
#include "engine/lua.h"
#include "engine/actor.h"
#include "engine/smush/smush.h"
#include "engine/savegame.h"
#include "engine/registry.h"

#include "engine/imuse/imuse.h"

// Entries in the system.controls table

const ControlDescriptor controls[] = {
	{ "KEY_ESCAPE", Common::KEYCODE_ESCAPE },
	{ "KEY_1", Common::KEYCODE_1 },
	{ "KEY_2", Common::KEYCODE_2 },
	{ "KEY_3", Common::KEYCODE_3 },
	{ "KEY_4", Common::KEYCODE_4 },
	{ "KEY_5", Common::KEYCODE_5 },
	{ "KEY_6", Common::KEYCODE_6 },
	{ "KEY_7", Common::KEYCODE_7 },
	{ "KEY_8", Common::KEYCODE_8 },
	{ "KEY_9", Common::KEYCODE_9 },
	{ "KEY_0", Common::KEYCODE_0 },
	{ "KEY_MINUS", Common::KEYCODE_MINUS },
	{ "KEY_EQUALS", Common::KEYCODE_EQUALS },
	{ "KEY_BACK", Common::KEYCODE_BACKSPACE },
	{ "KEY_TAB", Common::KEYCODE_TAB },
	{ "KEY_Q", Common::KEYCODE_q },
	{ "KEY_W", Common::KEYCODE_w },
	{ "KEY_E", Common::KEYCODE_e },
	{ "KEY_R", Common::KEYCODE_r },
	{ "KEY_T", Common::KEYCODE_t },
	{ "KEY_Y", Common::KEYCODE_y },
	{ "KEY_U", Common::KEYCODE_u },
	{ "KEY_I", Common::KEYCODE_i },
	{ "KEY_O", Common::KEYCODE_o },
	{ "KEY_P", Common::KEYCODE_p },
	{ "KEY_LBRACKET", Common::KEYCODE_LEFTBRACKET },
	{ "KEY_RBRACKET", Common::KEYCODE_RIGHTBRACKET },
	{ "KEY_RETURN", Common::KEYCODE_RETURN },
	{ "KEY_LCONTROL", Common::KEYCODE_LCTRL },
	{ "KEY_A", Common::KEYCODE_a },
	{ "KEY_S", Common::KEYCODE_s },
	{ "KEY_D", Common::KEYCODE_d },
	{ "KEY_F", Common::KEYCODE_f },
	{ "KEY_G", Common::KEYCODE_g },
	{ "KEY_H", Common::KEYCODE_h },
	{ "KEY_J", Common::KEYCODE_j },
	{ "KEY_K", Common::KEYCODE_k },
	{ "KEY_L", Common::KEYCODE_l },
	{ "KEY_SEMICOLON", Common::KEYCODE_SEMICOLON },
	{ "KEY_APOSTROPHE", Common::KEYCODE_QUOTEDBL },
	{ "KEY_GRAVE", Common::KEYCODE_BACKQUOTE },
	{ "KEY_LSHIFT", Common::KEYCODE_LSHIFT },
	{ "KEY_BACKSLASH", Common::KEYCODE_BACKSLASH },
	{ "KEY_Z", Common::KEYCODE_z },
	{ "KEY_X", Common::KEYCODE_x },
	{ "KEY_C", Common::KEYCODE_c },
	{ "KEY_V", Common::KEYCODE_v },
	{ "KEY_B", Common::KEYCODE_b },
	{ "KEY_N", Common::KEYCODE_n },
	{ "KEY_M", Common::KEYCODE_m },
	{ "KEY_COMMA", Common::KEYCODE_COMMA },
	{ "KEY_PERIOD", Common::KEYCODE_PERIOD },
	{ "KEY_SLASH", Common::KEYCODE_SLASH },
	{ "KEY_RSHIFT", Common::KEYCODE_RSHIFT },
	{ "KEY_MULTIPLY", Common::KEYCODE_ASTERISK },
	{ "KEY_LMENU", Common::KEYCODE_LALT },
	{ "KEY_SPACE", Common::KEYCODE_SPACE },
	{ "KEY_CAPITAL", Common::KEYCODE_CAPSLOCK },
	{ "KEY_F1", Common::KEYCODE_F1 },
	{ "KEY_F2", Common::KEYCODE_F2 },
	{ "KEY_F3", Common::KEYCODE_F3 },
	{ "KEY_F4", Common::KEYCODE_F4 },
	{ "KEY_F5", Common::KEYCODE_F5 },
	{ "KEY_F6", Common::KEYCODE_F6 },
	{ "KEY_F7", Common::KEYCODE_F7 },
	{ "KEY_F8", Common::KEYCODE_F8 },
	{ "KEY_F9", Common::KEYCODE_F9 },
	{ "KEY_F10", Common::KEYCODE_F10 },
	{ "KEY_NUMLOCK", Common::KEYCODE_NUMLOCK },
	{ "KEY_SCROLL", Common::KEYCODE_SCROLLOCK },
	{ "KEY_NUMPAD7", Common::KEYCODE_KP7 },
	{ "KEY_NUMPAD8", Common::KEYCODE_KP8 },
	{ "KEY_NUMPAD9", Common::KEYCODE_KP9 },
	{ "KEY_SUBTRACT", Common::KEYCODE_KP_MINUS },
	{ "KEY_NUMPAD4", Common::KEYCODE_KP4 },
	{ "KEY_NUMPAD5", Common::KEYCODE_KP5 },
	{ "KEY_NUMPAD6", Common::KEYCODE_KP6 },
	{ "KEY_ADD", Common::KEYCODE_KP_PLUS },
	{ "KEY_NUMPAD1", Common::KEYCODE_KP1 },
	{ "KEY_NUMPAD2", Common::KEYCODE_KP2 },
	{ "KEY_NUMPAD3", Common::KEYCODE_KP3 },
	{ "KEY_NUMPAD0", Common::KEYCODE_KP0 },
	{ "KEY_DECIMAL", Common::KEYCODE_KP_PERIOD },
	{ "KEY_F11", Common::KEYCODE_F11 },
	{ "KEY_F12", Common::KEYCODE_F12 },
	{ "KEY_F13", Common::KEYCODE_F13 },
	{ "KEY_F14", Common::KEYCODE_F14 },
	{ "KEY_F15", Common::KEYCODE_F15 },
	{ "KEY_NUMPADEQUALS", Common::KEYCODE_KP_EQUALS },
	{ "KEY_AT", Common::KEYCODE_AT },
	{ "KEY_COLON", Common::KEYCODE_COLON },
	{ "KEY_UNDERLINE", Common::KEYCODE_UNDERSCORE },
	{ "KEY_STOP", Common::KEYCODE_BREAK },
	{ "KEY_NUMPADENTER", Common::KEYCODE_KP_ENTER },
	{ "KEY_RCONTROL", Common::KEYCODE_RCTRL },
	{ "KEY_NUMPADCOMMA", Common::KEYCODE_KP_PERIOD },
	{ "KEY_DIVIDE", Common::KEYCODE_KP_DIVIDE },
	{ "KEY_SYSRQ", Common::KEYCODE_SYSREQ },
	{ "KEY_RMENU", Common::KEYCODE_RALT },
	{ "KEY_HOME", Common::KEYCODE_HOME },
	{ "KEY_UP", Common::KEYCODE_UP },
	{ "KEY_PRIOR", Common::KEYCODE_PAGEUP },
	{ "KEY_LEFT", Common::KEYCODE_LEFT },
	{ "KEY_RIGHT", Common::KEYCODE_RIGHT },
	{ "KEY_END", Common::KEYCODE_END },
	{ "KEY_DOWN", Common::KEYCODE_DOWN },
	{ "KEY_NEXT", Common::KEYCODE_PAGEDOWN },
	{ "KEY_INSERT", Common::KEYCODE_INSERT },
	{ "KEY_DELETE", Common::KEYCODE_DELETE },
	{ "KEY_LWIN", Common::KEYCODE_LSUPER },
	{ "KEY_RWIN", Common::KEYCODE_RSUPER },
	{ "KEY_APPS", Common::KEYCODE_MENU },

	{ "KEY_JOY1_B1", KEYCODE_JOY1_B1 },
	{ "KEY_JOY1_B2", KEYCODE_JOY1_B2 },
	{ "KEY_JOY1_B3", KEYCODE_JOY1_B3 },
	{ "KEY_JOY1_B4", KEYCODE_JOY1_B4 },
	{ "KEY_JOY1_B5", KEYCODE_JOY1_B5 },
	{ "KEY_JOY1_B6", KEYCODE_JOY1_B6 },
	{ "KEY_JOY1_B7", KEYCODE_JOY1_B7 },
	{ "KEY_JOY1_B8", KEYCODE_JOY1_B8 },
	{ "KEY_JOY1_B9", KEYCODE_JOY1_B9 },
	{ "KEY_JOY1_B10", KEYCODE_JOY1_B10 },
	{ "KEY_JOY1_HLEFT", KEYCODE_JOY1_HLEFT },
	{ "KEY_JOY1_HUP", KEYCODE_JOY1_HUP },
	{ "KEY_JOY1_HRIGHT", KEYCODE_JOY1_HRIGHT },
	{ "KEY_JOY1_HDOWN", KEYCODE_JOY1_HDOWN },
	{ "KEY_JOY2_B1", KEYCODE_JOY2_B1 },
	{ "KEY_JOY2_B2", KEYCODE_JOY2_B2 },
	{ "KEY_JOY2_B3", KEYCODE_JOY2_B3 },
	{ "KEY_JOY2_B4", KEYCODE_JOY2_B4 },
	{ "KEY_JOY2_B5", KEYCODE_JOY2_B5 },
	{ "KEY_JOY2_B6", KEYCODE_JOY2_B6 },
	{ "KEY_JOY2_B7", KEYCODE_JOY2_B7 },
	{ "KEY_JOY2_B8", KEYCODE_JOY2_B8 },
	{ "KEY_JOY2_B9", KEYCODE_JOY2_B9 },
	{ "KEY_JOY2_B10", KEYCODE_JOY2_B10 },
	{ "KEY_JOY2_HLEFT", KEYCODE_JOY1_HLEFT },
	{ "KEY_JOY2_HUP", KEYCODE_JOY2_HUP },
	{ "KEY_JOY2_HRIGHT", KEYCODE_JOY2_HRIGHT },
	{ "KEY_JOY2_HDOWN", KEYCODE_JOY2_HDOWN },
	{ "KEY_MOUSE_B1", KEYCODE_MOUSE_B1 },
	{ "KEY_MOUSE_B2", KEYCODE_MOUSE_B2 },
	{ "KEY_MOUSE_B3", KEYCODE_MOUSE_B3 },
	{ "KEY_MOUSE_B4", KEYCODE_MOUSE_B4 },
	{ "AXIS_JOY1_X", KEYCODE_AXIS_JOY1_X },
	{ "AXIS_JOY1_Y", KEYCODE_AXIS_JOY1_Y },
	{ "AXIS_JOY1_Z", KEYCODE_AXIS_JOY1_Z },
	{ "AXIS_JOY1_R", KEYCODE_AXIS_JOY1_R },
	{ "AXIS_JOY1_U", KEYCODE_AXIS_JOY1_U },
	{ "AXIS_JOY1_V", KEYCODE_AXIS_JOY1_V },
	{ "AXIS_JOY2_X", KEYCODE_AXIS_JOY2_X },
	{ "AXIS_JOY2_Y", KEYCODE_AXIS_JOY2_Y },
	{ "AXIS_JOY2_Z", KEYCODE_AXIS_JOY2_Z },
	{ "AXIS_JOY2_R", KEYCODE_AXIS_JOY2_R },
	{ "AXIS_JOY2_U", KEYCODE_AXIS_JOY2_U },
	{ "AXIS_JOY2_V", KEYCODE_AXIS_JOY2_V },
	{ "AXIS_MOUSE_X", KEYCODE_AXIS_MOUSE_X },
	{ "AXIS_MOUSE_Y", KEYCODE_AXIS_MOUSE_Y },
	{ "AXIS_MOUSE_Z", KEYCODE_AXIS_MOUSE_Z },

	{ NULL, 0 }
};

// CHAR_KEY tests to see whether a keycode is for
// a "character" handler or a "button" handler
#define CHAR_KEY(k) ((k >= 'a' && k <= 'z') || (k >= 'A' && k <= 'Z') || (k >= '0' && k <= '9') || k == ' ')

Engine *g_engine = NULL;

extern Imuse *g_imuse;
int g_imuseState = -1;
int g_flags = 0;

extern Common::StringList::const_iterator g_filesiter;

// hack for access current upated actor to allow access position of actor to sound costume component
Actor *g_currentUpdatedActor = NULL;

Engine::Engine() :
		_currScene(NULL), _selectedActor(NULL) {

	_controlsEnabled = new bool[KEYCODE_EXTRA_LAST];
	_controlsState = new bool[KEYCODE_EXTRA_LAST];
	for (int i = 0; i < KEYCODE_EXTRA_LAST; i++) {
		_controlsEnabled[i] = false;
		_controlsState[i] = false;
	}
	_speechMode = 3; // VOICE + TEXT
	_textSpeed = 7;
	_mode = _previousMode = ENGINE_MODE_IDLE;
	_flipEnable = true;
	int speed = atol(g_registry->get("engine_speed", "30"));
	if (speed == 0)
		_speedLimitMs = 0;
	else if (speed < 0 || speed > 100)
		_speedLimitMs = 33;
	else
		_speedLimitMs = 1000 / speed;
	char buf[20];
	sprintf(buf, "%d", 1000 / _speedLimitMs);
	g_registry->set("engine_speed", buf);
	_refreshDrawNeeded = true;
	g_filesiter = NULL;
	_savedState = NULL;
	_fps[0] = 0;

	printLineDefaults.x = 0;
	printLineDefaults.y = 100;
	printLineDefaults.width = 0;
	printLineDefaults.height = 0;
	printLineDefaults.font = NULL;
	printLineDefaults.justify = TextObject::LJUSTIFY;

	sayLineDefaults.x = 0;
	sayLineDefaults.y = 100;
	sayLineDefaults.width = 0;
	sayLineDefaults.height = 0;
	sayLineDefaults.font = NULL;
	sayLineDefaults.justify = TextObject::CENTER;

	blastTextDefaults.x = 0;
	blastTextDefaults.y = 200;
	blastTextDefaults.width = 0;
	blastTextDefaults.height = 0;
	blastTextDefaults.fgColor._vals[2] = 80;
	blastTextDefaults.font = NULL;
	blastTextDefaults.justify = TextObject::LJUSTIFY;
}

Engine::~Engine() {
	delete[] _controlsEnabled;
	delete[] _controlsState;

	for (SceneListType::const_iterator i = _scenes.begin(); i != _scenes.end(); i++)
		delete (*i);

	for (ActorListType::const_iterator i = _actors.begin(); i != _actors.end(); i++)
		delete (*i);

	killPrimitiveObjects();
	killTextObjects();
}

void Engine::handleButton(int operation, int key, int /*keyModifier*/, uint16 ascii) {
	lua_Object handler, system_table, userPaintHandler;

	// If we're not supposed to handle the key then don't
	if (!_controlsEnabled[key])
		return;

	if (operation == Common::EVENT_KEYDOWN)
		_controlsState[key] = true;
	else if (operation == Common::EVENT_KEYUP)
		_controlsState[key] = false;

	lua_beginblock();
	system_table = lua_getglobal("system");
	userPaintHandler = getTableValue(system_table, "userPaintHandler");
	if (userPaintHandler != LUA_NOOBJECT && CHAR_KEY(ascii)) {
		handler = getTableFunction(userPaintHandler, "characterHandler");
		// Ignore EVENT_KEYUP so there are not duplicate keystrokes, but
		// don't pass on to the normal buttonHandler since it doesn't
		// recognize character codes
		if (handler != LUA_NOOBJECT && operation == Common::EVENT_KEYDOWN) {
			char keychar[2];

			lua_beginblock();
			lua_pushobject(userPaintHandler);
			keychar[0] = ascii;
			keychar[1] = '\0';
			lua_pushstring(keychar);
			lua_pushnil();
			lua_callfunction(handler);
			lua_endblock();
		}
	} else {
		// Only allow the "Q" safe-exit when in-game, otherwise
		// it interferes with menu operation
		if (ascii == 'q') {
			lua_beginblock();
			lua_Object handler = getEventHandler("exitHandler");
			if (handler != LUA_NOOBJECT)
				lua_callfunction(handler);
			lua_endblock();
		} else {
			handler = getEventHandler("buttonHandler");
			if (handler != LUA_NOOBJECT) {
				lua_pushnumber(key);
				if (operation == Common::EVENT_KEYDOWN)
					lua_pushnumber(1);
				else
					lua_pushnil();
				lua_pushnil();
				lua_callfunction(handler);
			}
		}
	}
	lua_endblock();
}

void Engine::handleDebugLoadResource() {
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
		resource = (void *)g_smush->play(buf, 0, 0);
	else if (strstr(buf, ".wav") || strstr(buf, ".imu")) {
		g_imuse->startSfx(buf);
		resource = (void *)1;
	} else if (strstr(buf, ".mat")) {
		CMap *cmap = g_resourceloader->loadColormap("item.cmp");
		warning("Default colormap applied to resources loaded in this fashion!");
		resource = (void *)g_resourceloader->loadMaterial(buf, *cmap);
	} else {
		warning("Resource type not understood!");
	}
	if (!resource)
		warning("Requested resouce (%s) not found!");
}

void Engine::drawPrimitives() {
	// Draw Primitives
	for (PrimitiveListType::iterator i = _primitiveObjects.begin(); i != _primitiveObjects.end(); i++) {
		(*i)->draw();
	}

	// Draw text
	for (TextListType::iterator i = _textObjects.begin(); i != _textObjects.end(); i++) {
		(*i)->draw();
	}
}

void Engine::luaUpdate() {
	// Update timing information
	unsigned newStart = g_system->getMillis();
	if (newStart < _frameStart) {
		_frameStart = newStart;
		return;
	}
	_frameTime = newStart - _frameStart;
	_frameStart = newStart;

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
}

void Engine::updateDisplayScene() {
	_doFlip = true;

	if (_mode == ENGINE_MODE_SMUSH) {
		if (g_smush->isPlaying()) {
			//_mode = ENGINE_MODE_NORMAL; ???
			_movieTime = g_smush->getMovieTime();
			if (g_smush->isUpdateNeeded()) {
				g_driver->prepareSmushFrame(g_smush->getWidth(), g_smush->getHeight(), g_smush->getDstPtr());
				g_smush->clearUpdateNeeded();
			}
			int frame = g_smush->getFrame();
			if (frame > 0) {
				if (frame != _prevSmushFrame) {
					_prevSmushFrame = g_smush->getFrame();
					g_driver->drawSmushFrame(g_smush->getX(), g_smush->getY());
					if (SHOWFPS_GLOBAL)
						g_driver->drawEmergString(550, 25, _fps, Color(255, 255, 255));
				} else
					_doFlip = false;
			} else
				g_driver->releaseSmushFrame();
		}
		drawPrimitives();
	} else if (_mode == ENGINE_MODE_NORMAL) {
		if (!_currScene)
			return;

		g_driver->clearScreen();

		_prevSmushFrame = 0;

		_currScene->drawBackground();

		// Draw underlying scene components
		// Background objects are drawn underneath everything except the background
		// There are a bunch of these, especially in the tube-switcher room
		_currScene->drawBitmaps(ObjectState::OBJSTATE_BACKGROUND);
		// State objects are drawn on top of other things, such as the flag
		// on Manny's message tube
		_currScene->drawBitmaps(ObjectState::OBJSTATE_STATE);

		// Play SMUSH Animations
		// This should occur on top of all underlying scene objects,
		// a good example is the tube switcher room where some state objects
		// need to render underneath the animation or you can't see what's going on
		// This should not occur on top of everything though or Manny gets covered
		// up when he's next to Glottis's service room
		if (g_smush->isPlaying()) {
			_movieTime = g_smush->getMovieTime();
			if (g_smush->isUpdateNeeded()) {
				g_driver->prepareSmushFrame(g_smush->getWidth(), g_smush->getHeight(), g_smush->getDstPtr());
				g_smush->clearUpdateNeeded();
			}
			if (g_smush->getFrame() > 0)
				g_driver->drawSmushFrame(g_smush->getX(), g_smush->getY());
			else
				g_driver->releaseSmushFrame();
		}

		// Underlay objects are just above the background
		_currScene->drawBitmaps(ObjectState::OBJSTATE_UNDERLAY);

		_currScene->setupCamera();

		g_driver->set3DMode();

		_currScene->setupLights();

		// Update actor costumes & sets
		for (ActorListType::iterator i = _actors.begin(); i != _actors.end(); i++) {
			Actor *a = *i;

			// Update the actor's costumes & chores
			g_currentUpdatedActor = *i;
			// Note that the actor need not be visible to update chores, for example:
			// when Manny has just brought Meche back he is offscreen several times
			// when he needs to perform certain chores
			if (a->inSet(_currScene->name()))
				a->update();
		}
		g_currentUpdatedActor = NULL;

		// Draw actors
		for (ActorListType::iterator i = _actors.begin(); i != _actors.end(); i++) {
			Actor *a = *i;
			if (a->inSet(_currScene->name()) && a->visible())
				a->draw();
			a->undraw(a->inSet(_currScene->name()) && a->visible());
		}
		flagRefreshShadowMask(false);

		// Draw overlying scene components
		// The overlay objects should be drawn on top of everything else,
		// including 3D objects such as Manny and the message tube
		_currScene->drawBitmaps(ObjectState::OBJSTATE_OVERLAY);

		g_driver->storeDisplay();
		drawPrimitives();
	} else if (_mode == ENGINE_MODE_DRAW) {
		if (_refreshDrawNeeded) {
			lua_beginblock();
			lua_Object drawHandler = getEventHandler("userPaintHandler");
			if (drawHandler != LUA_NOOBJECT)
				lua_callfunction(drawHandler);
			lua_endblock();

			g_driver->flipBuffer();
		}
		_refreshDrawNeeded = false;
		return;
	}
}

void Engine::doFlip() {
	if (SHOWFPS_GLOBAL && _doFlip)
		g_driver->drawEmergString(550, 25, _fps, Color(255, 255, 255));

	if (_doFlip && _flipEnable)
		g_driver->flipBuffer();

	if (SHOWFPS_GLOBAL && _doFlip && _mode != ENGINE_MODE_DRAW) {
		_frameCounter++;
		_timeAccum += _frameTime;
		if (_timeAccum > 500) {
			sprintf(_fps, "%7.2f", (double)(_frameCounter * 1000) / (double)_timeAccum );
			_frameCounter = 0;
			_timeAccum = 0;
		}
	}
}

void Engine::mainLoop() {
	_movieTime = 0;
	_frameTime = 0;
	_frameStart = g_system->getMillis();
	_frameCounter = 0;
	_timeAccum = 0;
	_frameTimeCollection = 0;
	_prevSmushFrame = 0;
	_savegameLoadRequest = false;
	_savegameSaveRequest = false;
	_savegameFileName = NULL;
	_refreshShadowMask = false;

	for (;;) {
		uint32 startTime = g_system->getMillis();

		if (_savegameLoadRequest) {
			savegameRestore();
		}
		if (_savegameSaveRequest) {
			savegameSave();
		}

		g_imuse->flushTracks();
		g_imuse->refreshScripts();

		if (_mode == ENGINE_MODE_IDLE) {
			// don't kill CPU
			g_system->delayMillis(10);
			continue;
		}

		// Process events
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			// Handle any button operations
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP)
				handleButton(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
			// Check for "Hard" quit"
			if (event.type == Common::EVENT_QUIT)
				return;
			if (event.type == Common::EVENT_SCREEN_CHANGED)
				_refreshDrawNeeded = true;
			if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.ascii == 'z' && (event.kbd.flags & Common::KBD_CTRL)) {
					handleDebugLoadResource();
				}
			}
		}

		luaUpdate();

		if (_mode != ENGINE_MODE_PAUSE) {
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

void Engine::savegameReadStream(void *data, int32 size) {
	g_engine->_savedState->read(data, size);
}

void Engine::savegameWriteStream(void *data, int32 size) {
	g_engine->_savedState->write(data, size);
}

int32 Engine::savegameReadSint32() {
	return g_engine->_savedState->readLESint32();
}

void Engine::savegameWriteSint32(int32 val) {
	g_engine->_savedState->writeLESint32(val);
}

uint32 Engine::savegameReadUint32() {
	return g_engine->_savedState->readLEUint32();
}

void Engine::savegameWriteUint32(uint32 val) {
	g_engine->_savedState->writeLEUint32(val);
}

void Engine::savegameRestore() {
	printf("Engine::savegameRestore() started.\n");
	_savegameLoadRequest = false;
	char filename[200];
	if (!_savegameFileName) {
		strcpy(filename, "grim.sav");
	} else {
		strcpy(filename, _savegameFileName);
	}
	_savedState = new SaveGame(filename, false);
	if (!_savedState)
		return;
	g_imuse->stopAllSounds();
	g_imuse->resetState();
	g_smush->stop();
	g_imuse->pause(true);
	g_smush->pause(true);

	//  free all resource
	//  lock resources

	//Chore_Restore(_savedState);
	//Resource_Restore(_savedState);
	//Text_Restore(_savedState);
	//Room_Restore(_savedState);
	//Actor_Restore(_savedState);
	//Render_Restore(_savedState);
	//Primitive_Restore(_savedState);
	//Smush_Restore(_savedState);
	g_imuse->restoreState(_savedState);
	_savedState->beginSection('LUAS');
	lua_Restore(savegameReadStream, savegameReadSint32, savegameReadUint32);
	_savedState->endSection();
	//  unlock resources
	delete _savedState;

	//bundle_dofile("patch05.bin");

	g_imuse->pause(false);
	g_smush->pause(false);
	printf("Engine::savegameRestore() finished.\n");
}

void Engine::storeSaveGameImage(SaveGame *savedState) {
	int width = 250, height = 188;
	Bitmap *screenshot;

	printf("Engine::StoreSaveGameImage() started.\n");

	int mode = g_engine->getMode();
	g_engine->setMode(ENGINE_MODE_NORMAL);
	g_engine->updateDisplayScene();
	screenshot = g_driver->getScreenshot(width, height);
	g_engine->setMode(mode);
	savedState->beginSection('SIMG');
	if (screenshot) {
		int size = screenshot->width() * screenshot->height() * sizeof(uint16);
		screenshot->setNumber(0);
		char *data = screenshot->getData();
		savedState->write(data, size);
	} else {
		error("Unable to store screenshot!");
	}
	savedState->endSection();
	delete screenshot;
	printf("Engine::StoreSaveGameImage() finished.\n");
}

void Engine::savegameSave() {
	printf("Engine::savegameSave() started.\n");
	_savegameSaveRequest = false;
	char filename[200];
	if (!_savegameFileName) {
		strcpy(filename, "grim.sav");
	} else {
		strcpy(filename, _savegameFileName);
	}
	_savedState = new SaveGame(filename, true);
	if (!_savedState)
		return;

	storeSaveGameImage(_savedState);

	g_imuse->pause(true);
	g_smush->pause(true);

	savegameCallback();

	//Chore_Save(_savedState);
	//Resource_Save(_savedState);
	//Text_Save(_savedState);
	//Room_Save(_savedState);
	//Actor_Save(_savedState);
	//Render_Save(_savedState);
	//Primitive_Save(_savedState);
	//Smush_Save(_savedState);
	g_imuse->saveState(_savedState);
	_savedState->beginSection('LUAS');
	lua_Save(savegameWriteStream, savegameWriteSint32, savegameWriteUint32);
	_savedState->endSection();

	delete _savedState;

	g_imuse->pause(false);
	g_smush->pause(false);
	printf("Engine::savegameSave() finished.\n");
}

void Engine::savegameCallback() {
	lua_Object funcParam1;
	lua_Object funcParam2;
	bool unk1 = false;
	bool unk2 = false;

	lua_beginblock();
	lua_pushobject(lua_getglobal("system"));
	lua_pushstring("saveGameCallback");
	funcParam2 = lua_gettable();

	if (lua_istable(funcParam2)) {
		lua_pushobject(funcParam2);
		lua_pushstring("saveGameCallback");
		funcParam1 = lua_gettable();
		if (lua_isfunction(funcParam1)) {
			unk1 = true;
			unk2 = true;
		} else {
			assert(false);
		}
	} else if (lua_isfunction(funcParam2)) {
		funcParam1 = funcParam2;
		unk1 = false;
		unk2 = true;
	} else if (!lua_isnil(funcParam2)) {
		assert(false);
	}
	if (unk2) {
		if (unk1) {
			lua_pushobject(funcParam2);
		}
		lua_callfunction(funcParam1);
	}
	lua_endblock();
}

Scene *Engine::findScene(const char *name) {
	// Find scene object
	for (SceneListType::const_iterator i = scenesBegin(); i != scenesEnd(); i++) {
		if(!strcmp((char *) (*i)->name(), (char *) name))
			return *i;
	}
	return NULL;
}

void Engine::setSceneLock(const char *name, bool lockStatus) {
	Scene *scene = findScene(name);

	if (!scene) {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Scene object '%s' not found in list!", name);
		return;
	}
	// Change the locking status
	scene->_locked = lockStatus;
}

void Engine::setScene(const char *name) {
	Scene *scene = findScene(name);
	Scene *lastScene = _currScene;

	// If the scene already exists then use the existing data
	if (scene) {
		setScene(scene);
		return;
	}
	Block *b = g_resourceloader->getFileBlock(name);
	if (!b)
		warning("Could not find scene file %s", name);
	_currScene = new Scene(name, b->data(), b->len());
	registerScene(_currScene);
	_currScene->setSoundParameters(20, 127);
	// should delete the old scene after creating the new one
	if (lastScene && !lastScene->_locked) {
		removeScene(lastScene);
		delete lastScene;
	}
	delete b;
}

void Engine::setScene(Scene *scene) {
	Scene *lastScene = _currScene;

	_currScene = scene;
	_currScene->setSoundParameters(20, 127);
	// should delete the old scene after setting the new one
	if (lastScene && !lastScene->_locked) {
		removeScene(lastScene);
		delete lastScene;
	}
}

void Engine::setTextSpeed(int speed) {
	if (speed < 1)
		_textSpeed = 1;
	if (speed > 10)
		_textSpeed = 10;
	_textSpeed = speed;
}

float Engine::getControlAxis(int num) {
	return 0;
}

bool Engine::getControlState(int num) {
	return _controlsState[num];
}
