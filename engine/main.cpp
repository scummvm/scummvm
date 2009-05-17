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

#include "common/fs.h"
#include "common/events.h"

#include "engine/lua.h"
#include "engine/registry.h"
#include "engine/localize.h"
#include "engine/grim.h"
#include "engine/version.h"
#include "engine/cmd_line.h"
#include "engine/smush/smush.h"
#include "engine/imuse/imuse.h"
#include "engine/lua/lualib.h"

#ifdef USE_OPENGL
#include "engine/gfx_opengl.h"
#endif
#include "engine/gfx_tinygl.h"

// Hacky global toggles for experimental/debug code
bool SHOWFPS_GLOBAL, TINYGL_GLOBAL;
enDebugLevels gDebugLevel = DEBUG_NONE;

static bool g_lua_initialized = false;
GfxBase *g_driver = NULL;

void quit();

extern "C" int residual_main(int argc, char *argv[]) {
	Common::String specialDebug;
	Common::String command;
	bool opengl = false;

	printf("%s\n", gResidualFullVersion);
	printf("------------------------------------------------\n");

	// Verify that the backend has been initialized (i.e. g_system has been set).
	assert(g_system);
	OSystem &system = *g_system;

	registerDefaults();

	// Parse the command line
	Common::StringMap settings;
	command = parseCommandLine(settings, argc, argv);

	// Load the config file (possibly overriden via command line):
	if (settings.contains("config")) {
		ConfMan.loadConfigFile(settings["config"]);
		settings.erase("config");
	} else {
		ConfMan.loadDefaultConfigFile();
	}

	// Update the config file
	ConfMan.set("versioninfo", gResidualVersion, Common::ConfigManager::kApplicationDomain);

	// Load and setup the debuglevel and the debug flags. We do this at the
	// soonest possible moment to ensure debug output starts early on, if
	// requested.
	if (settings.contains("debuglevel")) {
		gDebugLevel = (enDebugLevels)strtol(settings["debuglevel"].c_str(), 0, 10);
		printf("Debuglevel (from command line): %d\n", gDebugLevel);
		settings.erase("debuglevel");	// This option should not be passed to ConfMan.
	} else if (ConfMan.hasKey("debuglevel"))
		gDebugLevel = (enDebugLevels)ConfMan.getInt("debuglevel");

	if (settings.contains("debugflags")) {
		specialDebug = settings["debugflags"];
		settings.erase("debugflags");
	}

	if (!processSettings(command, settings))
		return 0;

	// Init the backend. Must take place after all config data (including
	// the command line params) was read.
	system.initBackend();

	system.getEventManager()->init();

	g_registry = new Registry();

	SHOWFPS_GLOBAL = (tolower(g_registry->get("show_fps", "FALSE")[0]) == 't');
	TINYGL_GLOBAL = (tolower(g_registry->get("soft_renderer", "FALSE")[0]) == 't');
	bool fullscreen = (tolower(g_registry->get("fullscreen", "FALSE")[0]) == 't');

	if (!TINYGL_GLOBAL) {
		if (g_system->hasFeature(OSystem::kFeatureOpenGL))
			opengl = true;
		else
			error("gfx backend doesn't support hardware rendering");
	}

	if (TINYGL_GLOBAL)
		g_driver = new GfxTinyGL();
#ifdef USE_OPENGL
	else
		g_driver = new GfxOpenGL();
#else
	else
		error("gfx backend doesn't support hardware rendering");
#endif

	g_driver->setupScreen(640, 480, fullscreen, opengl);

	atexit(quit);

	Common::FSNode dir(g_registry->get("GrimDataDir", "."));
	SearchMan.addDirectory(dir.getPath(), dir, 0, 1);

	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, Audio::Mixer::kMaxMixerVolume);
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, Audio::Mixer::kMaxMixerVolume);
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxMixerVolume);
	g_grim = new GrimEngine();
	g_resourceloader = new ResourceLoader();
	g_localizer = new Localizer();
	g_smush = new Smush();
	g_imuse = new Imuse(20);

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


	quit();

	return 0;
}

void quit() {
	if (!g_driver)
		return;

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
	delete g_grim;
	g_grim = NULL;
	delete g_resourceloader;
	g_resourceloader = NULL;
	delete g_driver;
	g_driver = NULL;

	Common::ConfigManager::destroy();
	Common::SearchManager::destroy();
}
