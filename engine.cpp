// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
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
#include <SDL.h>
#include <SDL_timer.h>
#include <assert.h>
#include "screen.h"
#include "smush.h"

#include "driver_gl.h"

Engine *Engine::_instance = NULL;

Engine::Engine() :
		_currScene(NULL), _selectedActor(NULL) {
	for (int i = 0; i < SDLK_EXTRA_LAST; i++)
		_controlsEnabled[i] = false;
	_speechMode = 2;
}

void Engine::mainLoop() {
	_movieTime = 0;
	_frameTime = 0;
	_frameStart = SDL_GetTicks();
	unsigned int frameCounter = 0;
	unsigned int timeAccum = 0;
	unsigned int frameTimeCollection = 0;
	char fps[8] = "";
	_savegameLoadRequest = false;
	_savegameSaveRequest = false;
	_savegameFileName = NULL;

	for (;;) {
		// Process events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && _controlsEnabled[event.key.keysym.sym]) {
				lua_beginblock();
				lua_Object handler = getEventHandler("buttonHandler");
				if (handler != LUA_NOOBJECT) {
					lua_pushnumber(event.key.keysym.sym);
					lua_pushnumber(1);
					lua_pushnil();
					lua_callfunction(handler);
				}
				lua_endblock();
			}
			if (event.type == SDL_KEYUP && _controlsEnabled[event.key.keysym.sym]) {
				// temporary hack for save/load request until game menu will work
				if (event.key.keysym.sym == SDLK_F5) {
					_savegameLoadRequest = true;
					continue;
				} else if (event.key.keysym.sym == SDLK_F8) {
					_savegameSaveRequest = true;
					continue;
				}

				lua_beginblock();
				lua_Object handler = getEventHandler("buttonHandler");
				if (handler != LUA_NOOBJECT) {
					lua_pushnumber(event.key.keysym.sym);
					lua_pushnil();
					lua_pushnil();
					lua_callfunction(handler);
				}
				lua_endblock();
			}
			if (event.type == SDL_QUIT) {
				lua_beginblock();
				lua_Object handler = getEventHandler("exitHandler");
				if (handler != LUA_NOOBJECT)
					lua_callfunction(handler);
				lua_endblock();
			}
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_q)
					return;
			}
		}

		if (_savegameLoadRequest) {
			savegameRestore();
		}
		if (_savegameSaveRequest) {
			savegameSave();
		}

		if (_mode == ENGINE_MODE_SMUSH) {
			if (g_smush->isPlaying()) {
				_movieTime = g_smush->getMovieTime();
				if (g_smush->isUpdateNeeded()) {
					g_driver->prepareSmushFrame(g_smush->getWidth(), g_smush->getHeight(), g_smush->getDstPtr());
					g_smush->clearUpdateNeeded();
				}
				if (g_smush->getFrame() > 0) {
					g_driver->drawSmushFrame(g_smush->getX(), g_smush->getY());
					if (SHOWFPS_GLOBAL)
						g_driver->drawEmergString(550, 25, fps, Color(255, 255, 255));
				}
			}
		} else if (_mode == ENGINE_MODE_NORMAL) {
			if (SCREENBLOCKS_GLOBAL)
				screenBlocksReset();

			if (_currScene != NULL) {
				// Update actor costumes
				for (actor_list_type::iterator i = _actors.begin(); i != _actors.end(); i++) {
					Actor *a = *i;
					if (_currScene != NULL && a->inSet(_currScene->name()) && a->visible())
						a->update();
				}
			}

			g_driver->clearScreen();

			if (SCREENBLOCKS_GLOBAL)
				screenBlocksBlitDirtyBlocks();

			if (_currScene != NULL) {
				_currScene->drawBackground();
			}

			if (g_smush->isPlaying()) {
				_movieTime = g_smush->getMovieTime();
				if (g_smush->isUpdateNeeded()) {
					g_driver->prepareSmushFrame(g_smush->getWidth(), g_smush->getHeight(), g_smush->getDstPtr());
					g_smush->clearUpdateNeeded();
				}
				if (g_smush->getFrame() > 0)
					g_driver->drawSmushFrame(g_smush->getX(), g_smush->getY());
			}

			if (_currScene != NULL) {
				_currScene->drawBitmaps(ObjectState::OBJSTATE_UNDERLAY);
				_currScene->drawBitmaps(ObjectState::OBJSTATE_STATE);
				_currScene->drawBitmaps(ObjectState::OBJSTATE_OVERLAY);
			}

			if (SHOWFPS_GLOBAL)
				g_driver->drawEmergString(550, 25, fps, Color(255, 255, 255));

			g_driver->set3DMode();

			if (_currScene != NULL) {
				_currScene->setupCamera();

				// Draw actors
				for (actor_list_type::iterator i = _actors.begin(); i != _actors.end(); i++) {
					Actor *a = *i;
					if (_currScene != NULL && a->inSet(_currScene->name()) && a->visible())
						a->draw();
				}
				//screenBlocksDrawDebug();
			}

			// Draw text
			for (text_list_type::iterator i = _textObjects.begin(); i != _textObjects.end(); i++) {
				(*i)->draw();
			}

		}

		g_driver->flipBuffer();

		// don't kill CPU
		SDL_Delay(1);

		// Update timing information
		unsigned newStart = SDL_GetTicks();
		_frameTime = newStart - _frameStart;
		_frameStart = newStart;

		frameTimeCollection += _frameTime;
		if (frameTimeCollection > 10000) {
			frameTimeCollection = 0;
			lua_collectgarbage(0);
		}

		lua_beginblock();
		set_frameTime(_frameTime);
		lua_endblock();

		lua_beginblock();
		set_movieTime(_movieTime);
		lua_endblock();

		if (SHOWFPS_GLOBAL) {
			frameCounter++;
			timeAccum += _frameTime;
			if (timeAccum > 1000) {
				sprintf(fps, "%7.2f", (double)(frameCounter * 1000) / (double)timeAccum );
				frameCounter = 0;
				timeAccum = 0;
			}
		}

		// Run asynchronous tasks
		lua_runtasks();
	}
}

void Engine::savegameGzread(void *data, int32 size) {
	gzread(Engine::instance()->_savegameFileHandle, data, size);
}

void Engine::savegameGzwrite(void *data, int32 size) {
	gzwrite(Engine::instance()->_savegameFileHandle, data, size);
}

void Engine::savegameRestore() {
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

	//imuseStopAllSounds();
	g_smush->stop();
	//  free all resource
	//  lock resources
	//Chore_Restore(savegameGzread);
	//Resource_Restore(savegameGzread);
	//Text_Restore(savegameGzread);
	//Room_Restore(savegameGzread);
	//Actor_Restore(savegameGzread);
	//Render_Restore(savegameGzread);
	//Primitive_Restore(savegameGzread);
	//Smush_Restore(savegameGzread);
	//lua_Restore(savegameGzread);
	//  unlock resources
	gzclose(_savegameFileHandle);

	//do_dofile("patch05.bin");
}

void Engine::savegameCallback(void (*func)(void *, int32)) {
	lua_Object funcParam1;
	lua_Object funcParam2;
	bool unk1 = false;
	bool unk2 = false;
	//_savegameUnkFunc = func;

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

void Engine::savegameSave() {
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

	savegameCallback(savegameGzwrite);
	//Chore_Save(savegameGzwrite);
	//Resource_Save(savegameGzwrite);
	//Text_Save(savegameGzwrite);
	//Room_Save(savegameGzwrite);
	//Actor_Save(savegameGzwrite);
	//Render_Save(savegameGzwrite);
	//Primitive_Save(savegameGzwrite);
	//Smush_Save(savegameGzwrite);
	//lua_Save(savegameGzwrite);

	gzclose(_savegameFileHandle);
}

void Engine::setScene(const char *name) {
	Block *b = ResourceLoader::instance()->getFileBlock(name);
	if (b == NULL)
		warning("Could not find scene file %s\n", name);
	delete _currScene;
	_currScene = new Scene(name, b->data(), b->len());
	delete b;
}
