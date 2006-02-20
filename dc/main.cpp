// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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
#include "debug.h"
#include "bitmap.h"
#include "resource.h"
#include "lua.h"
#include "registry.h"
#include "localize.h"
#include "engine.h"
#include "timer.h"
#include "smush.h"
#include "driver_ronin.h"

#include "mixer/mixer.h"

#include "imuse/imuse.h"

#define Timer RoninTimer

#include <ronin/ronin.h>
#include <ronin/report.h>

#undef Timer

// Hacky global toggles for experimental/debug code
bool ZBUFFER_GLOBAL, SHOWFPS_GLOBAL, TINYGL_GLOBAL;
enDebugLevels debugLevel = DEBUG_NONE;

static bool g_lua_initialized = false;
Driver *g_driver = NULL;

void quit();

int main() {
	int i;

#ifndef NOSERIAL
	serial_init(57600);
	usleep(2000000);
	printf("Serial OK\r\n");
#endif

	atexit(quit);

	g_registry = new Registry();
	g_registry->set("DataDir", "/GRIMDATA");
	g_registry->set("good_times", "TRUE");

	g_driver = new DriverRonin();
	g_engine = new Engine();
	g_resourceloader = new ResourceLoader();
	g_localizer = new Localizer();
	g_mixer = new SoundMixer();
	g_mixer->setVolume(255);
	g_timer = new Timer();
	g_smush = new Smush();
	g_imuse = new Imuse(20);

	Bitmap *splash_bm = NULL;
	splash_bm = g_resourceloader->loadBitmap("splash.bm");

	g_driver->clearScreen();
	splash_bm->draw();
	g_driver->flipBuffer();

	lua_iolibopen();
	lua_strlibopen();
	lua_mathlibopen();

	register_lua();
	g_lua_initialized = true;

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
	if (g_lua_initialized) {
		lua_removelibslists();
		lua_close();
		g_lua_initialized = false;
	}
	if (g_registry) {
		g_registry->save();
		delete g_registry;
		g_registry = NULL;
	}
	if (g_smush) {
		delete g_smush;
		g_smush = NULL;
	}
	if (g_imuse) {
		delete g_imuse;
		g_imuse = NULL;
	}
	if (g_localizer) {
		delete g_localizer;
		g_localizer = NULL;
	}
	if (g_resourceloader) {
		delete g_resourceloader;
		g_resourceloader = NULL;
	}
	if (g_engine) {
		delete g_engine;
		g_engine = NULL;
	}
	if (g_timer) {
		delete g_timer;
		g_timer = NULL;
	}
	if (g_mixer) {
		delete g_mixer;
		g_mixer = NULL;
	}
	if (g_driver) {
		delete g_driver;
		g_driver = NULL;
	}

	report("Exiting to IP slave...\n");
	((void (*)())0x8cf00000)();
}

int system(const char *command)
{
  return -1;
}

int remove(const char *pathname)
{
  return -1;
}

int unlink(const char *pathname)
{
  return -1;
}

int rename(const char *oldpath, const char *newpath)
{
  return -1;
}

char *tmpnam(char *s)
{
  return NULL;
}

clock_t clock(void)
{
  return 0;
}

