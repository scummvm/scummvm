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
#include "localize.h"
#include "engine.h"
#include "timer.h"
#include "smush.h"
#include "mixer/mixer.h"
#include "imuse/imuse.h"

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
extern Imuse *g_imuse;

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

void quit();

int main(int argc, char *argv[]) {
	int i;

	// Parse command line
	ZBUFFER_GLOBAL = parseBoolStr(Registry::instance()->get("zbuffer"));
	SCREENBLOCKS_GLOBAL = parseBoolStr(Registry::instance()->get("screenblocks"));
	SHOWFPS_GLOBAL = parseBoolStr(Registry::instance()->get("fps"));
	for (i = 1; i < argc; i++) {
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

	atexit(quit);

	g_engine = new Engine();
	g_resourceloader = new ResourceLoader();
	g_localizer = new Localizer();
	g_mixer = new SoundMixer();
	g_mixer->setVolume(255);
	g_timer = new Timer();
	g_smush = new Smush();
	g_driver = new Driver(640, 480, 24);
	g_imuse = new Imuse(10);

	Bitmap *splash_bm = g_resourceloader->loadBitmap("splash.bm");

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

	g_engine->setMode(ENGINE_MODE_NORMAL);
	g_engine->mainLoop();

	quit();

	return 0;
}

void quit() {
	saveRegistry();

	delete g_smush;
	delete g_imuse;
	delete g_localizer;
	delete g_resourceloader;
	delete g_engine;
	delete g_timer;
	delete g_mixer;
	delete g_driver;

	lua_removelibslists();
	lua_close();

	SDL_Quit();
}

StackLock::StackLock(MutexRef mutex) :
		_mutex(mutex) {
	lockMutex(_mutex);
}

StackLock::~StackLock() {
	unlockMutex(_mutex);
}

MutexRef createMutex() {
	return (MutexRef)SDL_CreateMutex();
}

void lockMutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *)mutex);
}

void unlockMutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *)mutex);
}

void deleteMutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *)mutex);
}
