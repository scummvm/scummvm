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
#include <SDL.h>
#include <SDL_video.h>
#include "bitmap.h"
#include "resource.h"
#include "debug.h"
#include "lua.h"
#include "registry.h"
#include "engine.h"
#include "sound.h"
#include "timer.h"
#include "smush.h"
#include "mixer/mixer.h"

#include "driver_gl.h"

#ifndef _MSC_VER
#include <unistd.h>
#endif

// Hacky global toggles for experimental/debug code
bool ZBUFFER_GLOBAL, SCREENBLOCKS_GLOBAL, SHOWFPS_GLOBAL;

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

static bool parseBoolStr(const char *val) {
	if (val == NULL || val[0] == 0)
		return false;

	switch (val[0]) {
	case 'y': case 'Y':	// yes
	case 't': case 'T':	// true
	case '1':
		return true;
	case 'n': case 'N':	// no
	case 'f': case 'F':	// false
	case '0':
		return false;
	case 'o':
		switch (val[1]) {
		case 'n': case 'N': // on
			return true;
		case 'f': case 'F': // off
			return false;
		}
	}

	error("Unrecognized boolean value %s\n", val);
	return false;
}

int main(int argc, char *argv[]) {
	int i;

	// Parse command line
	ZBUFFER_GLOBAL = parseBoolStr(Registry::instance()->get("zbuffer"));
	SCREENBLOCKS_GLOBAL = parseBoolStr(Registry::instance()->get("screenblocks"));
	SHOWFPS_GLOBAL = parseBoolStr(Registry::instance()->get("fps"));
	for (i=1;i<argc;i++) {
		if (strcmp(argv[i], "-zbuffer") == 0)
			ZBUFFER_GLOBAL = true;
		else if (strcmp(argv[i], "-nozbuffer") == 0)
			ZBUFFER_GLOBAL = false;
		else if (strcmp(argv[i], "-screenblocks") == 0)
			SCREENBLOCKS_GLOBAL = true;
		else if (strcmp(argv[i], "-noscreenblocks") == 0)
			SCREENBLOCKS_GLOBAL = false;
		else if (strcmp(argv[i], "-fps") == 0)
			SHOWFPS_GLOBAL = true;
		else if (strcmp(argv[i], "-nofps") == 0)
			SHOWFPS_GLOBAL = false;
		else {
			printf("Residual CVS Version\n");
			printf("--------------------\n");
			printf("Recognised options:\n");
			printf("\t-[no]zbuffer\t\tEnable/disable ZBuffers (Very slow on older cards)\n");
			printf("\t-[no]screenblocks\t\tEnable/disable Screenblocks (Experimental zbuffer speedup on older cards - BROKEN!!\n");
			printf("\t-[no]fps\t\tEnable/disable fps display in upper right corner\n");
			exit(-1);
		}
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return 1;

	atexit(SDL_Quit);
	atexit(saveRegistry);

	g_mixer = new SoundMixer();
	g_timer = new Timer();
	g_smush = new Smush();

	g_driver = new Driver(640, 480, 24);
	
	Mixer::instance()->start();

	Bitmap *splash_bm = ResourceLoader::instance()->loadBitmap("splash.bm");

	SDL_Event event;
	
// For some reason we don't get the SDL_VIDEOEXPOSE event on OSX, so just don't wait for it.
#ifndef MACOSX
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_VIDEOEXPOSE) {
#else
	SDL_PollEvent(&event);
#endif	
			g_driver->clearScreen();

			splash_bm->draw();

			g_driver->flipBuffer();
#ifndef MACOSX
		}
	}
#endif

	lua_iolibopen();
	lua_strlibopen();
	lua_mathlibopen();

	register_lua();
	bundle_dofile("_system.lua");

	lua_pushnil();		// resumeSave
	lua_pushnil();		// bootParam
//	lua_pushnumber(0);		// bootParam
	lua_call("BOOT");

	Engine::instance()->setMode(ENGINE_MODE_NORMAL);
	Engine::instance()->mainLoop();

	lua_close();

	delete g_smush;
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
