// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
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

#include "engine.hh"
#include "scene.hh"
#include "lua.hh"
#include "colormap.hh"
#include "actor.hh"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_timer.h>

Engine *Engine::instance_ = NULL;

Engine::Engine() :
  currScene_(NULL), selectedActor_(NULL)
{
  for (int i = 0; i < SDLK_EXTRA_LAST; i++)
    controlsEnabled_[i] = false;
}

void Engine::mainLoop() {
  frameTime_ = 0;
  frameStart_ = SDL_GetTicks();

  for (;;) {
    // Process events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN &&
	  controlsEnabled_[event.key.keysym.sym]) {
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
      if (event.type == SDL_KEYUP &&
	  controlsEnabled_[event.key.keysym.sym]) {
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
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q)
	return;
    }

    // Run asynchronous tasks
    lua_runtasks();

    // Update actor costumes
    for (actor_list_type::iterator i = actors_.begin();
	 i != actors_.end(); i++) {
      Actor *a = *i;
      if (a->inSet(currScene_->name()) && a->visible())
	a->update();
    }

    // Draw the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Bitmap::prepareGL();
    if (currScene_ != NULL)
      currScene_->drawBackground();

    glEnable(GL_DEPTH_TEST);
    currScene_->setupCamera();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw actors
    glEnable(GL_TEXTURE_2D);
    for (actor_list_type::iterator i = actors_.begin();
	 i != actors_.end(); i++) {
      Actor *a = *i;
      if (a->inSet(currScene_->name()) && a->visible())
	a->draw();
    }
    glDisable(GL_TEXTURE_2D);

    SDL_GL_SwapBuffers();

    // Update timing information
    unsigned newStart = SDL_GetTicks();
    frameTime_ = newStart - frameStart_;
    frameStart_ = newStart;

    lua_beginblock();
    set_frameTime(frameTime_);
    lua_endblock();
  }
}

void Engine::setScene(const char *name) {
  Block *b = ResourceLoader::instance()->getFileBlock(name);
  if (b == NULL)
    warning("Could not find scene file %s\n", name);
  delete currScene_;
  currScene_ = new Scene(name, b->data(), b->len());
  delete b;
}
