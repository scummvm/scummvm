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

#include "stdafx.h"
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_opengl.h>
#include "bitmap.h"
#include "resource.h"
#include "debug.h"
#include "lua.h"
#include "registry.h"
#include "engine.h"
#include "sound.h"
#include "timer.h"
#include "mixer/mixer.h"
#ifndef _MSC_VER
#include <unistd.h>
#endif

// Hacky includes for temporary font rendering
#ifndef WIN32
 #include <GL/glx.h>
 #include <X11/Xlib.h>
#else
 #include <SDL_syswm.h>
 #include <windows.h>
#endif

// Hacky global toggles for experimental/debug code
int ZBUFFER_GLOBAL, SCREENBLOCKS_GLOBAL;

static void saveRegistry() {
  Registry::instance()->save();
}

#ifdef __MINGW32__
	int PASCAL WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,  LPSTR /*lpCmdLine*/, int /*iShowCmd*/) {
	return main(0, NULL);
}
#endif

extern SoundMixer *g_mixer;
extern Timer *g_timer;

int main(int argc, char *argv[]) {
  char 	GLDriver[1024];
  int i;

  // Parse command line
  ZBUFFER_GLOBAL = 0;
  SCREENBLOCKS_GLOBAL = 0;
  for (i=1;i<argc;i++) {
	if (strcmp(argv[i], "-zbuffer") == 0)
		ZBUFFER_GLOBAL = 1;
	else if (strcmp(argv[i], "-screenblocks") ==0)
		SCREENBLOCKS_GLOBAL = 1;
	else {
		printf("Residual CVS Version\n");
		printf("--------------------\n");
		printf("Recognised options:\n");
		printf("\t-zbuffer\t\tEnable ZBuffers (Very slow on older cards)\n");
		printf("\t-screenblocks\t\tEnable Screenblocks (Experimental zbuffer speedup on older cards - BROKEN!!\n");
		exit(-1);
	}
  }

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    return 1;
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if (SDL_SetVideoMode(640, 480, 24, SDL_OPENGL) == 0)
    error("Could not initialize video");

  atexit(SDL_Quit);
  atexit(saveRegistry);
  sprintf(GLDriver, "Residual: %s/%s", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
  SDL_WM_SetCaption(GLDriver, "Residual");
  
  Bitmap *splash_bm = ResourceLoader::instance()->loadBitmap("splash.bm");

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_VIDEOEXPOSE) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      Bitmap::prepareGL();
      splash_bm->draw();
      SDL_GL_SwapBuffers();
    }
  }

  g_mixer = new SoundMixer();
  g_timer = new Timer();
  Mixer::instance()->start();

  lua_open();

  lua_beginblock();
  lua_iolibopen();
  lua_strlibopen();
  lua_mathlibopen();
  lua_endblock();

  lua_beginblock();
  register_lua();
  lua_endblock();

  lua_beginblock();
  bundle_dofile("_system.lua");
  lua_endblock();

  lua_beginblock();
  lua_pushnil();		// resumeSave
  lua_pushnumber(0);		// bootParam
  lua_call("BOOT");
  lua_endblock();

  // FIXME: Hacky temporary font renderer code
  Engine::instance()->font = glGenLists(256);
  #ifdef WIN32
  {
	HDC   hDC;
	HFONT font;
	SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
	SDL_GetWMInfo(&wmi);

	hDC = GetDC(wmi.window);
	font = CreateFont(0, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 
			  OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 0	,
			  FF_DONTCARE|DEFAULT_PITCH, "Courier New");
	SelectObject(hDC, font);
	wglUseFontBitmaps(hDC, 0, 256, Engine::instance()->font);
  }
  #else
  {
        Display *dpy = XOpenDisplay(NULL);
        XFontStruct *XFont = XLoadQueryFont(dpy, "-misc-fixed-medium-r-*-*-20-*-*-*-*-*-*-*" );
        glXUseXFont(XFont->fid, 0, 256, Engine::instance()->font);
	XFreeFont(dpy, XFont);
	XCloseDisplay(dpy);
  }
  #endif
  Engine::instance()->mainLoop();

  delete g_timer;
  delete g_mixer;

  return 0;
}

StackLock::StackLock(MutexRef mutex)
	: _mutex(mutex) {
	lock_mutex(_mutex);
}

StackLock::~StackLock() {
	unlock_mutex(_mutex);
}

MutexRef create_mutex() {
	return (MutexRef) SDL_CreateMutex();
}

void lock_mutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *) mutex);
}

void unlock_mutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *) mutex);
}

void delete_mutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *) mutex);
}
