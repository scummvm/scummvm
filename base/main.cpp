/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*! \mainpage %ScummVM Source Reference
 *
 * These pages contains a cross referenced documentation for the %ScummVM source code,
 * generated with Doxygen (http://www.doxygen.org) directly from the source.
 * Currently not much is actually properly documented, but at least you can get an overview
 * of almost all the classes, methods and variables, and how they interact.
 */

#include "common/stdafx.h"
#include "base/engine.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "base/internal_version.h"
#include "base/version.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/timer.h"
#include "gui/newgui.h"
#include "gui/message.h"
#include "sound/mididrv.h"

#if defined(_WIN32_WCE)
#include "backends/wince/CELauncherDialog.h"
#elif defined(__DC__)
#include "backends/dc/DCLauncherDialog.h"
#else
#include "gui/launcher.h"
#endif

#ifdef PALMOS_68K
#include "args.h"
#endif

#ifdef __SYMBIAN32__
#include "gui/Actions.h"
#endif

/*
 * Version string and build date string. These can be used by anything that
 * wants to display this information to the user (e.g. about dialog).
 *
 * Note: it would be very nice if we could instead of (or in addition to) the
 * build date present a date which corresponds to the date our source files
 * were last changed. To understand the difference, imagine that a user
 * makes a checkout of CVS on January 1, then after a week compiles it
 * (e.g. after doing a 'make clean'). The build date then will say January 8
 * even though the files were last changed on January 1.
 *
 * Another problem is that __DATE__/__TIME__ depend on the local time zone.
 *
 * It's clear that such a "last changed" date would be much more useful to us
 * for feedback purposes. After all, when somebody files a bug report, we
 * don't care about the build date, we want to know which date their checkout
 * was made. This is even more important now since anon CVS lags a few
 * days behind developer CVS.
 *
 * So, how could we implement this? At least on unix systems, a special script
 * could do it. Basically, that script would run over all .cpp/.h files and
 * parse the CVS 'Header' keyword we have in our file headers.
 * That line contains a date/time in GMT. Now, the script just has to collect
 * all these times and find the latest. This time then would be inserted into
 * a header file or so (common/date.h ?) which engine.cpp then could
 * include and put into a global variable analog to gScummVMBuildDate.
 *
 * Drawback: scanning all source/header files will be rather slow. Also, this
 * only works on systems which can run powerful enough scripts (so I guess
 * Visual C++ would be out of the game here? don't know VC enough to be sure).
 *
 * Another approach would be to somehow get CVS to update a global file
 * (e.g. LAST_CHANGED) whenever any checkins are made. That would be
 * faster and work w/o much "logic" on the client side, in particular no
 * scripts have to be run. The problem with this is that I am not even
 * sure it's actually possible! Modifying files during commit time is trivial
 * to setup, but I have no idea if/how one can also change files which are not
 * currently being commit'ed.
 */
const char *gScummVMVersion = SCUMMVM_VERSION;
const char *gScummVMBuildDate = __DATE__ " " __TIME__;
const char *gScummVMVersionDate = SCUMMVM_VERSION " (" __DATE__ " " __TIME__ ")";
const char *gScummVMFullVersion = "ScummVM " SCUMMVM_VERSION " (" __DATE__ " " __TIME__ ")";
const char *gScummVMFeatures = ""
#ifdef USE_TREMOR
	"Tremor "
#else
#ifdef USE_VORBIS
	"Vorbis "
#endif
#endif
	
#ifdef USE_FLAC
	"FLAC "
#endif

#ifdef USE_MAD
	"MP3 "
#endif

#ifdef USE_ALSA
	"ALSA "
#endif

#ifdef USE_ZLIB
	"zLib "
#endif

#ifdef USE_MPEG2
	"MPEG2 "
#endif

#ifdef USE_FLUIDSYNTH
	"FluidSynth "
#endif

#ifdef __SYMBIAN32__
// we want a list of compiled in engines visible in the program,
// because we also release special builds with only one engine
#include "backends/symbian/src/main_features.inl"
#endif
	;

/** List all supported game IDs, i.e. all games which any loaded plugin supports. */
void listGames() {
	const PluginList &plugins = PluginManager::instance().getPlugins();

	printf("Game ID              Full Title                                            \n"
	       "-------------------- ------------------------------------------------------\n");

	PluginList::const_iterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		GameList list = (*iter)->getSupportedGames();
		for (GameList::iterator v = list.begin(); v != list.end(); ++v) {
			printf("%-20s %s\n", v->gameid.c_str(), v->description.c_str());
		}
	}
}

/** List all targets which are configured in the config file. */
void listTargets() {
	using namespace Common;
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();

	printf("Target               Description                                           \n"
	       "-------------------- ------------------------------------------------------\n");

	ConfigManager::DomainMap::const_iterator iter = domains.begin();
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		String name(iter->_key);
		String description(iter->_value.get("description"));

		if (description.empty()) {
			// FIXME: At this point, we should check for a "gameid" override
			// to find the proper desc. In fact, the platform probably should
			// be taken into account, too.
			String gameid(name);
			GameDescriptor g = GameDetector::findGame(gameid);
			if (g.description.size() > 0)
				description = g.description;
		}

		printf("%-20s %s\n", name.c_str(), description.c_str());
	}
}

static void setupDummyPalette(OSystem &system) {
	// FIXME - mouse cursors are currently always set via 8 bit data.
	// Thus for now we need to setup a dummy palette. On the long run, we might
	// want to add a setMouseCursor_overlay() method to OSystem, which would serve
	// two purposes:
	// 1) allow for 16 bit mouse cursors in overlay mode
	// 2) no need to backup & restore the mouse cursor before/after the overlay is shown
	const byte dummy_palette[] = {
		0, 0, 0, 0,
		0, 0, 171, 0,
		0, 171, 0, 0,
		0, 171, 171, 0,
		171, 0, 0, 0,
		171, 0, 171, 0,
		171, 87, 0, 0,
		171, 171, 171, 0,
		87, 87, 87, 0,
		87, 87, 255, 0,
		87, 255, 87, 0,
		87, 255, 255, 0,
		255, 87, 87, 0,
		255, 87, 255, 0,
		255, 255, 87, 0,
		255, 255, 255, 0,
	};

	system.setPalette(dummy_palette, 0, 16);
}

static bool launcherDialog(GameDetector &detector, OSystem &system) {

	system.beginGFXTransaction();
		// Set the user specified graphics mode (if any).
		system.setGraphicsMode(ConfMan.get("gfx_mode").c_str());

		// Make GUI 640 x 400
		system.initSize(320, 200, (detector._force1xOverlay ? 1 : 2));
	system.endGFXTransaction();


	// Clear the main screen
	system.clearScreen();

	// Setup a dummy palette, for the mouse cursor
	setupDummyPalette(system);

#if defined(_WIN32_WCE)
	CELauncherDialog dlg(detector);
#elif defined(__DC__)
	DCLauncherDialog dlg(detector);
#else
	GUI::LauncherDialog dlg(detector);
#endif
	return (dlg.runModal() != -1);
}

static int runGame(GameDetector &detector, OSystem &system, const Common::String &edebuglevels) {
	// We add it here, so MD5-based detection will be able to
	// read mixed case files
	Common::File::addDefaultDirectory(ConfMan.get("path"));

	// Create the game engine
	Engine *engine = detector.createEngine(&system);
	if (!engine) {
		// TODO: Show an error dialog or so?
		//GUI::MessageDialog alert("ScummVM could not find any game in the specified directory!");
		//alert.runModal();
		warning("Failed to instantiate engine for target %s", detector._targetName.c_str());
		return 0;
	}

	// Now the engine should've set up all debug levels so we can use the command line arugments here
	Common::enableSpecialDebugLevelList(edebuglevels);

	// Set the window caption to the game name
	Common::String caption(ConfMan.get("description", detector._targetName));

	Common::String desc = GameDetector::findGame(detector._gameid).description;
	if (caption.empty() && !desc.empty())
		caption = desc;
	if (caption.empty())
		caption = detector._targetName;
	if (!caption.empty())	{
		system.setWindowCaption(caption.c_str());
	}

	Common::File::addDefaultDirectoryRecursive(ConfMan.get("path"));

	// Add extrapath (if any) to the directory search list
	if (ConfMan.hasKey("extrapath"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath"));

	if (ConfMan.hasKey("extrapath", Common::ConfigManager::kApplicationDomain))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath", Common::ConfigManager::kApplicationDomain));

	// As a last resort add current directory
	Common::File::addDefaultDirectory(".");

	int result;

	// Init the engine (this might change the screen parameters
	result = engine->init(detector);

	// Run the game engine if the initialization was successful.
	if (result == 0) {
		result = engine->go();
	}

	// We clear all debug levels again even though the engine should do it
	Common::clearAllSpecialDebugLevels();

	// Free up memory
	delete engine;

	// Stop all sound processing now (this prevents some race conditions later on)
	system.clearSoundCallback();

	// Reset the file/directory mappings
	Common::File::resetDefaultDirectories();

	return result;
}

#ifdef _WIN32_WCE
extern "C" int scummvm_main(GameDetector &detector, int argc, char *argv[]) {
#else
extern "C" int scummvm_main(int argc, char *argv[]) {
#endif
	Common::String specialDebug;
	Common::String command;
	bool running = true;

	// Verify that the backend has been initialized (i.e. g_system has been set).
	assert(g_system);
	OSystem &system = *g_system;

	// Parse the command line
	Common::StringMap settings;
	command = GameDetector::parseCommandLine(settings, argc, argv);

	// Load the config file (possibly overriden via command line):
	if (settings.contains("config")) {
		ConfMan.loadConfigFile(settings["config"]);
		settings.erase("config");
	} else {
		ConfMan.loadDefaultConfigFile();
	}

	// Update the config file
	ConfMan.set("versioninfo", gScummVMVersion, Common::ConfigManager::kApplicationDomain);

	if (settings.contains("debuglevel")) {
		gDebugLevel = (int)strtol(settings["debuglevel"].c_str(), 0, 10);
		printf("Debuglevel (from command line): %d\n", gDebugLevel);
		settings.erase("debuglevel");	// This option should not be passed to ConfMan.
	} else if (ConfMan.hasKey("debuglevel"))
		gDebugLevel = ConfMan.getInt("debuglevel");

	// Look for special debug flags
	if (settings.contains("debugflags")) {
		specialDebug = settings["debugflags"];
		settings.erase("debugflags");
	}

	// Load the plugins
	PluginManager::instance().loadPlugins();
	
	
	// Handle commands passed via the command line (like --list-targets and
	// --list-games). This must be done after the config file and the plugins
	// have been loaded.
	// FIXME: The way are are doing this is rather arbitrary at this time.
	// E.g. --version and --help are very similar, but are still handled
	// inside parseCommandLine. This should be unified.
	if (command == "list-targets") {
		listTargets();
		exit(0);
	} else if (command == "list-games") {
		listGames();
		exit(0);
	}
	

	// Process the remaining command line settings
#ifndef _WIN32_WCE
	GameDetector detector;
#endif
	detector.processSettings(command, settings);

#ifdef __SYMBIAN32__
	// init keymap support here: we wanna move this somewhere else?
	GUI::Actions::init(detector);
#endif

#ifdef PALMOS_68K
	ArgsFree(argv);
#endif

	// Init the backend. Must take place after all config data (including
	// the command line params) was read.
	system.initBackend();

	// Create the timer services
	Common::g_timer = new Common::Timer(&system);

	// Set initial window caption
	system.setWindowCaption(gScummVMFullVersion);

	// Setup a dummy palette, for the mouse cursor, in case an error
	// dialog has to be shown. See bug #1097467.
	setupDummyPalette(system);

	// Unless a game was specified, show the launcher dialog
	if (detector._targetName.empty())
		running = launcherDialog(detector, system);

	// FIXME: We're now looping the launcher. This, of course, doesn't
	// work as well as it should. In theory everything should be destroyed
	// cleanly, so this is now enabled to encourage people to fix bits :)
	while (running) {
		// Verify the given game name is a valid supported game
		if (detector.detectMain()) {
			// Unload all plugins not needed for this game,
			// to save memory
			PluginManager::instance().unloadPluginsExcept(detector._plugin);

			int result = runGame(detector, system, specialDebug);
			if (result == 0)
				break;

			// There are some command-line options that it's
			// unlikely that we want to preserve now that we're
			// going to start a different game.
			ConfMan.removeKey("boot_param", ConfMan.kTransientDomain);
			ConfMan.removeKey("save_slot", ConfMan.kTransientDomain);

			// PluginManager::instance().unloadPlugins();
			PluginManager::instance().loadPlugins();
		}

		running = launcherDialog(detector, system);
	}

	// Deinit the timer
	delete Common::g_timer;

	return 0;
}
