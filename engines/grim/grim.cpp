/* Residual - A 3D game interpreter
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
#include "common/config-manager.h"

#include "engines/engine.h"

#include "engines/grim/grim.h"
#include "engines/grim/lua.h"
#include "engines/grim/actor.h"
#include "engines/grim/smush/smush.h"
#include "engines/grim/savegame.h"
#include "engines/grim/registry.h"
#include "engines/grim/resource.h"
#include "engines/grim/localize.h"
#include "engines/grim/gfx_tinygl.h"
#include "engines/grim/gfx_opengl.h"

#include "engines/grim/lua/lualib.h"

#include "engines/grim/imuse/imuse.h"

namespace Grim {

static bool g_lua_initialized = false;

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

GrimEngine *g_grim = NULL;
GfxBase *g_driver = NULL;
int g_imuseState = -1;
int g_flags = 0;

extern Common::StringList::const_iterator g_filesiter;

// hack for access current upated actor to allow access position of actor to sound costume component
Actor *g_currentUpdatedActor = NULL;


GrimEngine::GrimEngine(OSystem *syst, const GrimGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _currScene(NULL), _selectedActor(NULL) {
	g_grim = this;

	g_registry = new Registry();
	g_resourceloader = NULL;
	g_localizer = NULL;
	g_smush = NULL;
	g_imuse = NULL;

	_showFps = (tolower(g_registry->get("show_fps", "FALSE")[0]) == 't');
	_softRenderer = (tolower(g_registry->get("soft_renderer", "FALSE")[0]) == 't');

	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_currScene = NULL;
	_selectedActor = NULL;
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
	if (speed <= 0 || speed > 100)
		_speedLimitMs = 30;
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

GrimEngine::~GrimEngine() {
	delete[] _controlsEnabled;
	delete[] _controlsState;

	for (SceneListType::const_iterator i = _scenes.begin(); i != _scenes.end(); i++)
		delete (*i);

	for (ActorListType::const_iterator i = _actors.begin(); i != _actors.end(); i++)
		delete (*i);

	killPrimitiveObjects();
	killTextObjects();

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
	delete g_smush;
	g_smush = NULL;
	delete g_imuse;
	g_imuse = NULL;
	delete g_localizer;
	g_localizer = NULL;
	delete g_resourceloader;
	g_resourceloader = NULL;
	delete g_driver;
	g_driver = NULL;
}

Common::Error GrimEngine::run() {
	g_resourceloader = new ResourceLoader();
	g_localizer = new Localizer();
	g_smush = new Smush();
	g_imuse = new Imuse(20);

	bool fullscreen = (tolower(g_registry->get("fullscreen", "FALSE")[0]) == 't');

	if (!_softRenderer && !g_system->hasFeature(OSystem::kFeatureOpenGL))
		error("gfx backend doesn't support hardware rendering");

	if (_softRenderer)
		g_driver = new GfxTinyGL();
#ifdef USE_OPENGL
	else
		g_driver = new GfxOpenGL();
#else
	else
		error("gfx backend doesn't support hardware rendering");
#endif

	g_driver->setupScreen(640, 480, fullscreen);

	Bitmap *splash_bm = NULL;
	if (!(g_flags & GF_DEMO))
		splash_bm = g_resourceloader->loadBitmap("splash.bm");
	if (splash_bm)
		splash_bm->ref();

	g_driver->clearScreen();

	if (!(g_flags & GF_DEMO))
		splash_bm->draw();

	g_driver->flipBuffer();

	if (splash_bm)
		splash_bm->deref();

	lua_iolibopen();
	lua_strlibopen();
	lua_mathlibopen();

	register_lua();
	g_lua_initialized = true;

	bundle_dofile("_system.lua");

	lua_pushnil();		// resumeSave
	lua_pushnil();		// bootParam - not used in scripts
//	lua_pushnumber(0);		// bootParam
	lua_call("BOOT");

	g_grim->setMode(ENGINE_MODE_NORMAL);
	g_grim->mainLoop();

	return Common::kNoError;
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
	bool buttonFuncIsTable, joyFuncIsTable;

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
		joyFuncIsTable = true;
	} else if (lua_isfunction(joyHandler)) {
		joyFunc = joyHandler;
		joyFuncIsTable = false;
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
		resource = (void *)g_smush->play(buf, false, 0, 0);
	else if (strstr(buf, ".wav") || strstr(buf, ".imu")) {
		g_imuse->startSfx(buf);
		resource = (void *)1;
	} else if (strstr(buf, ".mat")) {
		CMap *cmap = g_resourceloader->loadColormap("item.cmp");
		warning("Default colormap applied to resources loaded in this fashion");
		resource = (void *)g_resourceloader->loadMaterial(buf, *cmap);
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
	// Draw Primitives
	for (PrimitiveListType::iterator i = _primitiveObjects.begin(); i != _primitiveObjects.end(); i++) {
		(*i)->draw();
	}

	// Draw text
	for (TextListType::iterator i = _textObjects.begin(); i != _textObjects.end(); i++) {
		(*i)->draw();
	}
}

void GrimEngine::luaUpdate() {
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

void GrimEngine::updateDisplayScene() {
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
					if (_showFps)
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

		cameraPostChangeHandle(_currScene->setup());

		g_driver->clearScreen();

		_prevSmushFrame = 0;

		_currScene->drawBackground();

		// Draw underlying scene components
		// Background objects are drawn underneath everything except the background
		// There are a bunch of these, especially in the tube-switcher room
		_currScene->drawBitmaps(ObjectState::OBJSTATE_BACKGROUND);

		// Underlay objects are just above the background
		_currScene->drawBitmaps(ObjectState::OBJSTATE_UNDERLAY);

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
			handleUserPaint();
			g_driver->flipBuffer();
		}
		_refreshDrawNeeded = false;
		return;
	}
}

void GrimEngine::doFlip() {
	if (_showFps && _doFlip)
		g_driver->drawEmergString(550, 25, _fps, Color(255, 255, 255));

	if (_doFlip && _flipEnable)
		g_driver->flipBuffer();

	if (_showFps && _doFlip && _mode != ENGINE_MODE_DRAW) {
		_frameCounter++;
		_timeAccum += _frameTime;
		if (_timeAccum > 500) {
			sprintf(_fps, "%7.2f", (double)(_frameCounter * 1000) / (double)_timeAccum );
			_frameCounter = 0;
			_timeAccum = 0;
		}
	}
}

void GrimEngine::mainLoop() {
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
			// Handle any buttons, keys and joystick operations
			if (event.type == Common::EVENT_KEYDOWN) {
				if (_mode != ENGINE_MODE_DRAW && (event.kbd.ascii == 'q')) {
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

void GrimEngine::savegameReadStream(void *data, int32 size) {
	g_grim->_savedState->read(data, size);
}

void GrimEngine::savegameWriteStream(void *data, int32 size) {
	g_grim->_savedState->write(data, size);
}

int32 GrimEngine::savegameReadSint32() {
	return g_grim->_savedState->readLESint32();
}

void GrimEngine::savegameWriteSint32(int32 val) {
	g_grim->_savedState->writeLESint32(val);
}

uint32 GrimEngine::savegameReadUint32() {
	return g_grim->_savedState->readLEUint32();
}

void GrimEngine::savegameWriteUint32(uint32 val) {
	g_grim->_savedState->writeLEUint32(val);
}

void GrimEngine::savegameRestore() {
	printf("GrimEngine::savegameRestore() started.\n");
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
	printf("GrimEngine::savegameRestore() finished.\n");
}

void GrimEngine::storeSaveGameImage(SaveGame *savedState) {
	int width = 250, height = 188;
	Bitmap *screenshot;

	printf("GrimEngine::StoreSaveGameImage() started.\n");

	int mode = g_grim->getMode();
	g_grim->setMode(ENGINE_MODE_NORMAL);
	g_grim->updateDisplayScene();
	screenshot = g_driver->getScreenshot(width, height);
	g_grim->setMode(mode);
	savedState->beginSection('SIMG');
	if (screenshot) {
		int size = screenshot->width() * screenshot->height() * sizeof(uint16);
		screenshot->setNumber(0);
		char *data = screenshot->getData();
		savedState->write(data, size);
	} else {
		error("Unable to store screenshot");
	}
	savedState->endSection();
	delete screenshot;
	printf("GrimEngine::StoreSaveGameImage() finished.\n");
}

void GrimEngine::savegameSave() {
	printf("GrimEngine::savegameSave() started.\n");
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

	saveFonts(_savedState);
	saveActors(_savedState);

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
	printf("GrimEngine::savegameSave() finished.\n");
}

void GrimEngine::saveActors(SaveGame *savedState) {
	savedState->beginSection('ACTR');

	savedState->writeLESint32(_actors.size());
	for (ActorListType::iterator i = _actors.begin(); i != _actors.end(); i++) {
		Actor *a = *i;
		PointerId ptr = makeIdFromPointer(this);
		savedState->writeLEUint32(ptr.low);
		savedState->writeLEUint32(ptr.hi);
		a->saveState(savedState);
	}

	savedState->endSection();
}

void GrimEngine::saveFonts(SaveGame *savedState) {
	savedState->beginSection('FONT');

	savedState->writeLESint32(_fonts.size());
	for (Common::List<Font *>::iterator i = _fonts.begin(); i != _fonts.end(); i++) {
		Font *f = *i;
		PointerId ptr = makeIdFromPointer(this);
		savedState->writeLEUint32(ptr.low);
		savedState->writeLEUint32(ptr.hi);
		Common::String filename = f->getFilename();
		savedState->writeLESint32(filename.size());
		savedState->write(filename.c_str(), filename.size());
	}

	savedState->endSection();
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

Scene *GrimEngine::findScene(const char *name) {
	// Find scene object
	for (SceneListType::const_iterator i = scenesBegin(); i != scenesEnd(); i++) {
		if (!strcmp((char *) (*i)->name(), (char *) name))
			return *i;
	}
	return NULL;
}

void GrimEngine::setSceneLock(const char *name, bool lockStatus) {
	Scene *scene = findScene(name);

	if (!scene) {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Scene object '%s' not found in list", name);
		return;
	}
	// Change the locking status
	scene->_locked = lockStatus;
}

void GrimEngine::setScene(const char *name) {
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

void GrimEngine::setScene(Scene *scene) {
	Scene *lastScene = _currScene;

	_currScene = scene;
	_currScene->setSoundParameters(20, 127);
	// should delete the old scene after setting the new one
	if (lastScene && !lastScene->_locked) {
		removeScene(lastScene);
		delete lastScene;
	}
}

void GrimEngine::makeCurrentSetup(int num) {
	int prevSetup = g_grim->currScene()->setup();
	if (prevSetup != num) {
		currScene()->setSetup(num);
		currScene()->setSoundParameters(20, 127);
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

} // end of namespace Grim
