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
	if (ConfMan.hasKey("path"))
		Common::File::addDefaultDirectory(ConfMan.get("path"));
	else
		Common::File::addDefaultDirectory(".");

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

	if (ConfMan.hasKey("path"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("path"));
	else
		Common::File::addDefaultDirectory(".");

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

extern "C" int scummvm_main(int argc, char *argv[]) {
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
		return 0;
	} else if (command == "list-games") {
		listGames();
		return 0;
	}
	

	// Process the remaining command line settings
	GameDetector detector;
	detector.processSettings(command, settings);

#if defined(__SYMBIAN32__) || defined(_WIN32_WCE)
	// init keymap support here: we wanna move this somewhere else?
	GUI::Actions::init(detector._gameid);
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
	if (detector._targetName.empty()) {
		running = launcherDialog(detector, system);

		// Discard any command line options. Those that affect the graphics
		// mode etc. already have should have been handled by the backend at
		// this point. And the others (like bootparam etc.) should not
		// blindly be passed to the first game launched from the launcher.
		ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();
	}

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

			// Discard any command line options. It's unlikely that the user
			// wanted to apply them to *all* games ever launched.
			ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();
			
			// TODO: Reset the game detector fully

			// PluginManager::instance().unloadPlugins();
			PluginManager::instance().loadPlugins();
		}

		running = launcherDialog(detector, system);
	}

	// Deinit the timer
	delete Common::g_timer;

	return 0;
}
