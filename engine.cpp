// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "engine.h"
#include "scene.h"
#include "lua.h"
#include "colormap.h"
#include "actor.h"
#include "textobject.h"
#include "smush.h"
#include "driver.h"

#include "imuse/imuse.h"

#include <SDL.h>
#include <SDL_timer.h>
#include <assert.h>

Engine *g_engine = NULL;

extern Imuse *g_imuse;
int g_imuseState = -1;

#ifdef _WIN32

WIN32_FIND_DATAA g_find_file_data;
HANDLE g_searchFile;
bool g_firstFind;

#else

char g_find_file_data[100];
DIR *g_searchFile;

#endif

// hack for access current upated actor to allow access position of actor to sound costume component
Actor *g_currentUpdatedActor = NULL;

Engine::Engine() :
		_currScene(NULL), _selectedActor(NULL) {
	for (int i = 0; i < SDLK_EXTRA_LAST; i++)
		_controlsEnabled[i] = false;
	_speechMode = 3; // VOICE + TEXT
	_textSpeed = 6;
	_mode = _previousMode = ENGINE_MODE_IDLE;
	_flipEnable = true;
	_refreshDrawNeeded = true;
	g_searchFile = NULL;

	textObjectDefaults.x = 0;
	textObjectDefaults.y = 200;
	textObjectDefaults.width = 10;
	textObjectDefaults.height = 0;
	textObjectDefaults.fgColor._vals[0] = 255;
	textObjectDefaults.fgColor._vals[1] = 255;
	textObjectDefaults.fgColor._vals[2] = 255;
	textObjectDefaults.font = NULL;
	textObjectDefaults.justify = 2;

	sayLineDefaults.x = 0;
	sayLineDefaults.y = 100;
	sayLineDefaults.width = 0;
	sayLineDefaults.height = 0;
	sayLineDefaults.fgColor._vals[0] = 255;
	sayLineDefaults.fgColor._vals[1] = 255;
	sayLineDefaults.fgColor._vals[2] = 255;
	sayLineDefaults.font = NULL;
	sayLineDefaults.justify = 1;

	printLineDefaults.x = 0;
	printLineDefaults.y = 100;
	printLineDefaults.width = 0;
	printLineDefaults.height = 0;
	printLineDefaults.fgColor._vals[0] = 255;
	printLineDefaults.fgColor._vals[1] = 255;
	printLineDefaults.fgColor._vals[2] = 255;
	printLineDefaults.font = NULL;
	printLineDefaults.justify = 2;
}

void Engine::handleButton(int operation, int key) {
	// If we're not supposed to handle the key then don't
	if (!_controlsEnabled[key])
		return;

	lua_beginblock();
	lua_Object handler = getEventHandler("buttonHandler");
	if (handler != LUA_NOOBJECT) {
		lua_pushnumber(key);
		if (operation == SDL_KEYDOWN)
			lua_pushnumber(1);
		else
			lua_pushnil();
		lua_pushnil();
		lua_callfunction(handler);
	}
	lua_endblock();
}

void Engine::handleDebugLoadResource() {
	void *resource;
	int c, i = 0;
	char buf[512];

	// Tool for debugging the loading of a particular resource without
	// having to actually make it all the way to it in the game
	fprintf(stderr, "Enter resource to load (extension specifies type): ");
	while (i < 512 && (c = fgetc(stdin)) != EOF && c != '\n')
		buf[i++] = c;

	buf[i] = '\0';
	if (strncmp(buf, "exp:", 4) == 0)
		// Export a resource in order to view it directly
		resource = (void *)g_resourceloader->exportResource(&buf[4]);
	else if (strstr(buf, ".key"))
		resource = (void *)g_resourceloader->loadKeyframe(buf);
	else if (strstr(buf, ".zbm") || strstr(buf, ".bm"))
		resource = (void *)g_resourceloader->loadBitmap(buf);
	else if (strstr(buf, ".cmp"))
		resource = (void *)g_resourceloader->loadColormap(buf);
	else if (strstr(buf, ".cos"))
		resource = (void *)g_resourceloader->loadCostume(buf, NULL);
	else if (strstr(buf, ".lip"))
		resource = (void *)g_resourceloader->loadLipSynch(buf);
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
	if (resource == NULL)
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

	if (_mode == ENGINE_MODE_DRAW) {
		g_engine->killPrimitiveObjects();
		g_engine->killTextObjects();

		// Cleanup references to deleted text objects
		for (Engine::ActorListType::const_iterator i = g_engine->actorsBegin(); i != g_engine->actorsEnd(); i++)
			(*i)->lineCleanup();
	}
}

void Engine::luaUpdate() {
	// Update timing information
	unsigned newStart = SDL_GetTicks();
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
	static char fps[8] = "";
	bool doFlip = true;

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
						g_driver->drawEmergString(550, 25, fps, Color(255, 255, 255));
				} else
					doFlip = false;
			}
		}
	} else if (_mode == ENGINE_MODE_NORMAL) {
		if (_currScene == NULL)
			return;

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

		_prevSmushFrame = 0;

		g_driver->clearScreen();

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
		}

		// Underlay objects are just above the background
		_currScene->drawBitmaps(ObjectState::OBJSTATE_UNDERLAY);

		_currScene->setupCamera();

		g_driver->set3DMode();

		_currScene->setupLights();

		// Draw actors
		for (ActorListType::iterator i = _actors.begin(); i != _actors.end(); i++) {
			Actor *a = *i;
			if (a->inSet(_currScene->name()) && a->visible())
				a->draw();
			a->undraw(a->inSet(_currScene->name()) && a->visible());
		}

		// Draw overlying scene components
		// The overlay objects should be drawn on top of everything else,
		// including 3D objects such as Manny and the message tube
		_currScene->drawBitmaps(ObjectState::OBJSTATE_OVERLAY);

		g_driver->storeDisplay();
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

	drawPrimitives();

	if (SHOWFPS_GLOBAL)
		g_driver->drawEmergString(550, 25, fps, Color(255, 255, 255));

	if (doFlip && _flipEnable)
		g_driver->flipBuffer();

	// don't kill CPU
	SDL_Delay(1);

	if (SHOWFPS_GLOBAL && doFlip) {
		_frameCounter++;
		_timeAccum += _frameTime;
		if (_timeAccum > 1000) {
			sprintf(fps, "%7.2f", (double)(_frameCounter * 1000) / (double)_timeAccum );
			_frameCounter = 0;
			_timeAccum = 0;
		}
	}
}

void Engine::mainLoop() {
	_movieTime = 0;
	_frameTime = 0;
	_frameStart = SDL_GetTicks();
	_frameCounter = 0;
	_timeAccum = 0;
	_frameTimeCollection = 0;
	_prevSmushFrame = 0;
	_savegameLoadRequest = false;
	_savegameSaveRequest = false;
	_savegameFileName = NULL;

	for (;;) {
		if (_savegameLoadRequest) {
			savegameRestore();
		}
		if (_savegameSaveRequest) {
			savegameSave();
		}

		g_imuse->flushTracks();
		g_imuse->refreshScripts();

		if (_mode == ENGINE_MODE_IDLE)
			continue;

		// Process events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Handle any button operations
			if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
				handleButton(event.type, event.key.keysym.sym);
			// Check for "Hard" quit"
			if (event.type == SDL_QUIT)
				return;
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_z
						&& (event.key.keysym.mod & KMOD_CTRL)) {
					handleDebugLoadResource();
				}
				if ((event.key.keysym.sym == SDLK_RETURN ||
						event.key.keysym.sym == SDLK_KP_ENTER) &&
						(event.key.keysym.mod & KMOD_ALT)) {
					g_driver->toggleFullscreenMode();
				}
				if (event.key.keysym.sym == SDLK_q) {
					lua_beginblock();
					lua_Object handler = getEventHandler("exitHandler");
					if (handler != LUA_NOOBJECT)
						lua_callfunction(handler);
					lua_endblock();
				}
			}
		}

		luaUpdate();

		if (_mode != ENGINE_MODE_PAUSE) {
			updateDisplayScene();
		}

		if (g_imuseState != -1) {
			g_imuse->setMusicState(g_imuseState);
			g_imuseState = -1;
		}
	}
}

void Engine::savegameGzread(void *data, int size) {
	gzread(g_engine->_savegameFileHandle, data, size);
}

void Engine::savegameGzwrite(void *data, int size) {
	gzwrite(g_engine->_savegameFileHandle, data, size);
}

void Engine::savegameRestore() {
	printf("Engine::savegameRestore() started.\n");
	_savegameLoadRequest = false;
	char filename[200];
	if (_savegameFileName == NULL) {
		strcpy(filename, "grim.sav");
	} else {
		strcpy(filename, _savegameFileName);
	}
	_savegameFileHandle = gzopen(filename, "rb");
	if (_savegameFileHandle == NULL) {
		warning("savegameRestore() Error opening savegame file");
		return;
	}

	g_imuse->stopAllSounds();
	g_imuse->resetState();
	g_smush->stop();
	g_imuse->pause(true);
	g_smush->pause(true);

	//  free all resource
	//  lock resources

	uint32 tag;
	uint32 version;
	savegameGzread(&tag, 4);
	assert(tag == 'RSAV');
	savegameGzread(&version, 4);
	assert(version == 1);

	//Chore_Restore(savegameGzread);
	//Resource_Restore(savegameGzread);
	//Text_Restore(savegameGzread);
	//Room_Restore(savegameGzread);
	//Actor_Restore(savegameGzread);
	//Render_Restore(savegameGzread);
	//Primitive_Restore(savegameGzread);
	//Smush_Restore(savegameGzread);
	g_imuse->restoreState(savegameGzread);
	lua_Restore(savegameGzread);
	//  unlock resources
	gzclose(_savegameFileHandle);

	//bundle_dofile("patch05.bin");

	g_imuse->pause(false);
	g_smush->pause(false);
	printf("Engine::savegameRestore() finished.\n");
}

void Engine::savegameSave() {
	printf("Engine::savegameSave() started.\n");
	_savegameSaveRequest = false;
	char filename[200];
	if (_savegameFileName == NULL) {
		strcpy(filename, "grim.sav");
	} else {
		strcpy(filename, _savegameFileName);
	}
	_savegameFileHandle = gzopen(filename, "wb");
	if (_savegameFileHandle == NULL) {
		warning("savegameSave() Error creating savegame file");
		return;
	}

	g_imuse->pause(true);
	g_smush->pause(true);

	uint32 tag = 'RSAV';
	uint32 version = 1;
	savegameGzwrite(&tag, 4);
	savegameGzwrite(&version, 4);

	savegameCallback();

	//Chore_Save(savegameGzwrite);
	//Resource_Save(savegameGzwrite);
	//Text_Save(savegameGzwrite);
	//Room_Save(savegameGzwrite);
	//Actor_Save(savegameGzwrite);
	//Render_Save(savegameGzwrite);
	//Primitive_Save(savegameGzwrite);
	//Smush_Save(savegameGzwrite);
	g_imuse->saveState(savegameGzwrite);
	lua_Save(savegameGzwrite);

	gzclose(_savegameFileHandle);

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
	
	if (scene == NULL) {
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
	if (scene != NULL) {
		setScene(scene);
		return;
	}
	Block *b = g_resourceloader->getFileBlock(name);
	if (b == NULL)
		warning("Could not find scene file %s\n", name);
	_currScene = new Scene(name, b->data(), b->len());
	registerScene(_currScene);
	_currScene->setSoundParameters(20, 127);
	// should delete the old scene after creating the new one
	if (lastScene != NULL && !lastScene->_locked) {
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
	if (lastScene != NULL && !lastScene->_locked) {
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
