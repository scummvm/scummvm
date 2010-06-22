/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "engines/engine.h"
#include "engines/metaengine.h"
#include "base/commandLine.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/tokenizer.h"
#include "common/translation.h"

#include "gui/GuiManager.h"
#include "gui/message.h"
#include "gui/error.h"

#include "sound/audiocd.h"
#include "sound/mididrv.h"

#include "backends/keymapper/keymapper.h"

#if defined(_WIN32_WCE)
#include "backends/platform/wince/CELauncherDialog.h"
#elif defined(__DC__)
#include "backends/platform/dc/DCLauncherDialog.h"
#else
#include "gui/launcher.h"
#endif


static bool launcherDialog() {

	// Discard any command line options. Those that affect the graphics
	// mode and the others (like bootparam etc.) should not
	// blindly be passed to the first game launched from the launcher.
	ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();

#if defined(_WIN32_WCE)
	CELauncherDialog dlg;
#elif defined(__DC__)
	DCLauncherDialog dlg;
#else
	GUI::LauncherDialog dlg;
#endif
	return (dlg.runModal() != -1);
}

static const EnginePlugin *detectPlugin() {
	const EnginePlugin *plugin = 0;

	// Make sure the gameid is set in the config manager, and that it is lowercase.
	Common::String gameid(ConfMan.getActiveDomainName());
	assert(!gameid.empty());
	if (ConfMan.hasKey("gameid")) {
		gameid = ConfMan.get("gameid");

		// Set last selected game, that the game will be highlighted
		// on RTL
		ConfMan.set("lastselectedgame", ConfMan.getActiveDomainName(), Common::ConfigManager::kApplicationDomain);
		ConfMan.flushToDisk();
	}

	gameid.toLowercase();
	ConfMan.set("gameid", gameid);

	// Query the plugins and find one that will handle the specified gameid
	printf(_t("User picked target '%s' (gameid '%s')...\n"), ConfMan.getActiveDomainName().c_str(), gameid.c_str());
	printf("%s", _t("  Looking for a plugin supporting this gameid... "));
	GameDescriptor game = EngineMan.findGame(gameid, &plugin);

	if (plugin == 0) {
		printf("%s", _t("failed\n"));
		warning(_t("%s is an invalid gameid. Use the --list-games option to list supported gameid"), gameid.c_str());
		return 0;
	} else {
		printf("%s\n", plugin->getName());
	}

	// FIXME: Do we really need this one?
	printf(_t("  Starting '%s'\n"), game.description().c_str());

	return plugin;
}

// TODO: specify the possible return values here
static Common::Error runGame(const EnginePlugin *plugin, OSystem &system, const Common::String &edebuglevels) {
	// Determine the game data path, for validation and error messages
	Common::FSNode dir(ConfMan.get("path"));
	Common::Error err = Common::kNoError;
	Engine *engine = 0;

	// Verify that the game path refers to an actual directory
	if (!(dir.exists() && dir.isDirectory()))
		err = Common::kInvalidPathError;

	// Create the game engine
	if (err == Common::kNoError)
		err = (*plugin)->createInstance(&system, &engine);

	// Check for errors
	if (!engine || err != Common::kNoError) {

		// TODO: An errorDialog for this and engine related errors is displayed already in the scummvm_main function
		// Is a separate dialog here still required?

		//GUI::displayErrorDialog("ScummVM could not find any game in the specified directory!");
		const char *errMsg = _(Common::errorToString(err));

		warning(_t("%s failed to instantiate engine: %s (target '%s', path '%s')"),
			plugin->getName(),
			errMsg,
			ConfMan.getActiveDomainName().c_str(),
			dir.getPath().c_str()
			);

		// Autoadded is set only when no path was provided and
		// the game is run from command line.
		//
		// Thus, we remove this garbage entry
		//
		// Fixes bug #1544799
		if (ConfMan.hasKey("autoadded")) {
			ConfMan.removeGameDomain(ConfMan.getActiveDomainName().c_str());
		}

		return err;
	}

	// Set the window caption to the game name
	Common::String caption(ConfMan.get("description"));

	if (caption.empty()) {
		caption = EngineMan.findGame(ConfMan.get("gameid")).description();
	}
	if (caption.empty())
		caption = ConfMan.getActiveDomainName();	// Use the domain (=target) name
	if (!caption.empty())	{
		system.setWindowCaption(caption.c_str());
	}

	//
	// Setup various paths in the SearchManager
	//

	// Add the game path to the directory search list
	SearchMan.addDirectory(dir.getPath(), dir, 0, 4);

	// Add extrapath (if any) to the directory search list
	if (ConfMan.hasKey("extrapath")) {
		dir = Common::FSNode(ConfMan.get("extrapath"));
		SearchMan.addDirectory(dir.getPath(), dir);
	}

	// If a second extrapath is specified on the app domain level, add that as well.
	if (ConfMan.hasKey("extrapath", Common::ConfigManager::kApplicationDomain)) {
		dir = Common::FSNode(ConfMan.get("extrapath", Common::ConfigManager::kApplicationDomain));
		SearchMan.addDirectory(dir.getPath(), dir);
	}

	// On creation the engine should have set up all debug levels so we can use
	// the command line arugments here
	Common::StringTokenizer tokenizer(edebuglevels, " ,");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (!DebugMan.enableDebugChannel(token))
			warning(_("Engine does not support debug level '%s'"), token.c_str());
	}

	// Inform backend that the engine is about to be run
	system.engineInit();

	// Run the engine
	Common::Error result = engine->run();

	// Inform backend that the engine finished
	system.engineDone();

	// We clear all debug levels again even though the engine should do it
	DebugMan.clearAllDebugChannels();

	// Free up memory
	delete engine;

	// Reset the file/directory mappings
	SearchMan.clear();

	// Return result (== 0 means no error)
	return result;
}

static void setupGraphics(OSystem &system) {

	system.beginGFXTransaction();
		// Set the user specified graphics mode (if any).
		system.setGraphicsMode(ConfMan.get("gfx_mode").c_str());

		system.initSize(320, 200);

		if (ConfMan.hasKey("aspect_ratio"))
			system.setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio"));
		if (ConfMan.hasKey("fullscreen"))
			system.setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen"));
	system.endGFXTransaction();

	// When starting up launcher for the first time, the user might have specified
	// a --gui-theme option, to allow that option to be working, we need to initialize
	// GUI here.
	// FIXME: Find a nicer way to allow --gui-theme to be working
	GUI::GuiManager::instance();

	// Set initial window caption
	system.setWindowCaption(gScummVMFullVersion);

	// Clear the main screen
	system.fillScreen(0);
}

static void setupKeymapper(OSystem &system) {

#ifdef ENABLE_KEYMAPPER
	using namespace Common;

	Keymapper *mapper = system.getEventManager()->getKeymapper();
	Keymap *globalMap = new Keymap("global");
	Action *act;
	HardwareKeySet *keySet;

	keySet = system.getHardwareKeySet();

	// Query backend for hardware keys and register them
	mapper->registerHardwareKeySet(keySet);

	// Now create the global keymap
	act = new Action(globalMap, "MENU", _("Menu"), kGenericActionType, kSelectKeyType);
	act->addKeyEvent(KeyState(KEYCODE_F5, ASCII_F5, 0));

	act = new Action(globalMap, "SKCT", _("Skip"), kGenericActionType, kActionKeyType);
	act->addKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE, 0));

	act = new Action(globalMap, "PAUS", _("Pause"), kGenericActionType, kStartKeyType);
	act->addKeyEvent(KeyState(KEYCODE_SPACE, ' ', 0));

	act = new Action(globalMap, "SKLI", _("Skip line"), kGenericActionType, kActionKeyType);
	act->addKeyEvent(KeyState(KEYCODE_PERIOD, '.', 0));

	act = new Action(globalMap, "VIRT", _("Display keyboard"), kVirtualKeyboardActionType);
	act->addKeyEvent(KeyState(KEYCODE_F7, ASCII_F7, 0));

	act = new Action(globalMap, "REMP", _("Remap keys"), kKeyRemapActionType);
	act->addKeyEvent(KeyState(KEYCODE_F8, ASCII_F8, 0));

	mapper->addGlobalKeymap(globalMap);

	mapper->pushKeymap("global");
#endif

}

extern "C" int scummvm_main(int argc, const char * const argv[]) {
	Common::String specialDebug;
	Common::String command;

	// Verify that the backend has been initialized (i.e. g_system has been set).
	assert(g_system);
	OSystem &system = *g_system;

	// Register config manager defaults
	Base::registerDefaults();

	// Parse the command line
	Common::StringMap settings;
	command = Base::parseCommandLine(settings, argc, argv);

	// Load the config file (possibly overriden via command line):
	if (settings.contains("config")) {
		ConfMan.loadConfigFile(settings["config"]);
		settings.erase("config");
	} else {
		ConfMan.loadDefaultConfigFile();
	}

	// Update the config file
	ConfMan.set("versioninfo", gScummVMVersion, Common::ConfigManager::kApplicationDomain);

	// Enable translation
	TransMan.setLanguage(ConfMan.get("gui_language").c_str());

	// Load and setup the debuglevel and the debug flags. We do this at the
	// soonest possible moment to ensure debug output starts early on, if
	// requested.
	if (settings.contains("debuglevel")) {
		gDebugLevel = (int)strtol(settings["debuglevel"].c_str(), 0, 10);
		printf("Debuglevel (from command line): %d\n", gDebugLevel);
		settings.erase("debuglevel");	// This option should not be passed to ConfMan.
	} else if (ConfMan.hasKey("debuglevel"))
		gDebugLevel = ConfMan.getInt("debuglevel");

	if (settings.contains("debugflags")) {
		specialDebug = settings["debugflags"];
		settings.erase("debugflags");
	}

	// Load the plugins.
	PluginManager::instance().loadPlugins();

	// If we received an invalid music parameter via command line we check this here.
	// We can't check this before loading the music plugins.
	// On the other hand we cannot load the plugins before we know the file paths (in case of external plugins).
	if (!settings["music-driver"].empty()) {
		if (MidiDriver::getMusicType(MidiDriver::getDeviceHandle(settings["music-driver"])) == MT_NULL) {
				warning("Unrecognized music driver '%s'\nSwitching to default device.", settings["music-driver"].c_str());
				settings["music-driver"] = "auto";
		}
	}

	// Process the remaining command line settings. Must be done after the
	// config file and the plugins have been loaded.
	Common::Error res;

	if ((res = Base::processSettings(command, settings)) != Common::kArgumentNotProcessed)
		return res;

	// Init the backend. Must take place after all config data (including
	// the command line params) was read.
	system.initBackend();

	setupGraphics(system);

	// Init the event manager. As the virtual keyboard is loaded here, it must
	// take place after the backend is initiated and the screen has been setup
	system.getEventManager()->init();

	// Directly after initializing the event manager, we will initialize our
	// event recorder.
	//
	// TODO: This is just to match the current behavior, when we further extend
	// our event recorder, we might do this at another place. Or even change
	// the whole API for that ;-).
	g_eventRec.init();

	// Now as the event manager is created, setup the keymapper
	setupKeymapper(system);

	// Unless a game was specified, show the launcher dialog
	if (0 == ConfMan.getActiveDomain())
		launcherDialog();

	// FIXME: We're now looping the launcher. This, of course, doesn't
	// work as well as it should. In theory everything should be destroyed
	// cleanly, so this is now enabled to encourage people to fix bits :)
	while (0 != ConfMan.getActiveDomain()) {
		// Try to find a plugin which feels responsible for the specified game.
		const EnginePlugin *plugin = detectPlugin();
		if (plugin) {
			// Unload all plugins not needed for this game,
			// to save memory
			PluginManager::instance().unloadPluginsExcept(PLUGIN_TYPE_ENGINE, plugin);

			// Try to run the game
			Common::Error result = runGame(plugin, system, specialDebug);

			// Did an error occur ?
			if (result != Common::kNoError) {
				// Shows an informative error dialog if starting the selected game failed.
				GUI::displayErrorDialog(result, _("Error running game:"));
			}

			// Quit unless an error occurred, or Return to launcher was requested
			#ifndef FORCE_RTL
			if (result == 0 && !g_system->getEventManager()->shouldRTL())
				break;
			#endif
			// Reset RTL flag in case we want to load another engine
			g_system->getEventManager()->resetRTL();
			#ifdef FORCE_RTL
			g_system->getEventManager()->resetQuit();
			#endif

			// Discard any command line options. It's unlikely that the user
			// wanted to apply them to *all* games ever launched.
			ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();

			// Clear the active config domain
			ConfMan.setActiveDomain("");

			// PluginManager::instance().unloadPlugins();
			PluginManager::instance().loadPlugins();
		} else {
			// A dialog would be nicer, but we don't have any
			// screen to draw on yet.
			warning("%s", _("Could not find any engine capable of running the selected game"));
			GUI::displayErrorDialog(_("Could not find any engine capable of running the selected game"));
		}

		// We will destroy the AudioCDManager singleton here to save some memory.
		// This will not make the CD audio stop, one would have to enable this:
		//AudioCD.stop();
		// but the engine is responsible for stopping CD playback anyway and
		// this way we catch engines not doing it properly. For some more
		// information about why AudioCDManager::destroy does not stop the CD
		// playback read the FIXME in sound/audiocd.h
		Audio::AudioCDManager::destroy();

		// reset the graphics to default
		setupGraphics(system);
		launcherDialog();
	}
	PluginManager::instance().unloadPlugins();
	PluginManager::destroy();
	Common::ConfigManager::destroy();
	Common::SearchManager::destroy();
	GUI::GuiManager::destroy();

	return 0;
}
