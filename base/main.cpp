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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*! \mainpage %ScummVM Source Reference
 *
 * These pages contains a cross referenced documentation for the %ScummVM source code,
 * generated with Doxygen (http://www.doxygen.org) directly from the source.
 * Currently not much is actually properly documented, but at least you can get an overview
 * of almost all the classes, methods and variables, and how they interact.
 */

// FIXME: Avoid using printf
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "engines/engine.h"
#include "engines/metaengine.h"
#include "base/commandLine.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h" /* for debug manager */
#include "common/events.h"
#include "gui/EventRecorder.h"
#include "common/fs.h"
#ifdef ENABLE_EVENTRECORDER
#include "common/recorderfile.h"
#endif
#include "common/system.h"
#include "common/textconsole.h"
#include "common/tokenizer.h"
#include "common/translation.h"
#include "common/text-to-speech.h"
#include "common/osd_message_queue.h"

#include "gui/gui-manager.h"
#include "gui/error.h"

#include "audio/mididrv.h"
#include "audio/musicplugin.h"  /* for music manager */

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/yuv_to_rgb.h"
#ifdef USE_FREETYPE2
#include "graphics/fonts/ttf.h"
#endif

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"

#ifdef USE_CLOUD
#ifdef USE_LIBCURL
#include "backends/cloud/cloudmanager.h"
#include "backends/networking/curl/connectionmanager.h"
#endif
#ifdef USE_SDL_NET
#include "backends/networking/sdl_net/localwebserver.h"
#endif
#endif

#if defined(__DC__)
#include "backends/platform/dc/DCLauncherDialog.h"
#else
#include "gui/launcher.h"
#endif

#ifdef USE_UPDATES
#include "gui/updates-dialog.h"
#endif

static bool launcherDialog() {

	// Discard any command line options. Those that affect the graphics
	// mode and the others (like bootparam etc.) should not
	// blindly be passed to the first game launched from the launcher.
	ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();

#if defined(__DC__)
	DCLauncherDialog dlg;
#else
	GUI::LauncherDialog dlg;
#endif
	return (dlg.runModal() != -1);
}

static const Plugin *detectPlugin() {
	// Figure out the engine ID and game ID
	Common::String engineId = ConfMan.get("engineid");
	Common::String gameId = ConfMan.get("gameid");

	// Print text saying what's going on
	printf("User picked target '%s' (engine ID '%s', game ID '%s')...\n", ConfMan.getActiveDomainName().c_str(), engineId.c_str(), gameId.c_str());

	// At this point the engine ID and game ID must be known
	if (engineId.empty()) {
		warning("The engine ID is not set for target '%s'", ConfMan.getActiveDomainName().c_str());
		return 0;
	}

	if (gameId.empty()) {
		warning("The game ID is not set for target '%s'", ConfMan.getActiveDomainName().c_str());
		return 0;
	}

	const Plugin *plugin = EngineMan.findPlugin(engineId);
	if (!plugin) {
		warning("'%s' is an invalid engine ID. Use the --list-engines command to list supported engine IDs", engineId.c_str());
		return 0;
	}

	// Query the plugin for the game descriptor
	printf("   Looking for a plugin supporting this target... %s\n", plugin->getName());
	PlainGameDescriptor game = plugin->get<MetaEngineDetection>().findGame(gameId.c_str());
	if (!game.gameId) {
		warning("'%s' is an invalid game ID for the engine '%s'. Use the --list-games option to list supported game IDs", gameId.c_str(), engineId.c_str());
		return 0;
	}

	return plugin;
}

void saveLastLaunchedTarget(const Common::String &target) {
	if (ConfMan.hasGameDomain(target)) {
		// Set the last selected game, so the game will be highlighted next time the user
		// returns to the launcher.
		ConfMan.set("lastselectedgame", target, Common::ConfigManager::kApplicationDomain);
		ConfMan.flushToDisk();
	}
}

// TODO: specify the possible return values here
static Common::Error runGame(const Plugin *plugin, OSystem &system, const Common::String &edebuglevels) {
	assert(plugin);

	// Determine the game data path, for validation and error messages
	Common::FSNode dir(ConfMan.get("path"));
	Common::String target = ConfMan.getActiveDomainName();
	Common::Error err = Common::kNoError;
	Engine *engine = 0;

#if defined(SDL_BACKEND) && defined(USE_OPENGL) && defined(USE_RGB_COLOR)
	// HACK: We set up the requested graphics mode setting here to allow the
	// backend to switch from Surface SDL to OpenGL if necessary. This is
	// needed because otherwise the g_system->getSupportedFormats might return
	// bad values.
	g_system->beginGFXTransaction();
		g_system->setGraphicsMode(ConfMan.get("gfx_mode").c_str());
	if (g_system->endGFXTransaction() != OSystem::kTransactionSuccess) {
		warning("Switching graphics mode to '%s' failed", ConfMan.get("gfx_mode").c_str());
		return Common::kUnknownError;
	}
#endif

	// Verify that the game path refers to an actual directory
	if (!dir.exists()) {
		err = Common::kPathDoesNotExist;
	} else if (!dir.isDirectory()) {
		err = Common::kPathNotDirectory;
	}

	// Create the game's MetaEngine.
	const MetaEngineDetection &metaEngine = plugin->get<MetaEngineDetection>();
	if (err.getCode() == Common::kNoError) {
		// Set default values for all of the custom engine options
		// Apparently some engines query them in their constructor, thus we
		// need to set this up before instance creation.
		metaEngine.registerDefaultSettings(target);
	}

	// Right now we have a MetaEngine plugin. We must find the matching engine plugin to
	// call createInstance and other connecting functions.
	Plugin *enginePluginToLaunchGame = PluginMan.getEngineFromMetaEngine(plugin);

	if (!enginePluginToLaunchGame) {
		err = Common::kEnginePluginNotFound;
		return err;
	}

	// Create the game's MetaEngineConnect.
	const MetaEngine &metaEngineConnect = enginePluginToLaunchGame->get<MetaEngine>();
	err = metaEngineConnect.createInstance(&system, &engine);

	// Check for errors
	if (!engine || err.getCode() != Common::kNoError) {

		// Print a warning; note that scummvm_main will also
		// display an error dialog, so we don't have to do this here.
		warning("%s failed to instantiate engine: %s (target '%s', path '%s')",
			plugin->getName(),
			err.getDesc().c_str(),
			target.c_str(),
			dir.getPath().c_str()
			);

		// If a temporary target failed to launch, remove it from the configuration manager
		// so it not visible in the launcher.
		// Temporary targets are created when starting games from the command line using the game id.
		if (ConfMan.hasKey("id_came_from_command_line")) {
			ConfMan.removeGameDomain(target.c_str());
		}

		return err;
	}

	// Set the window caption to the game name
	Common::String caption(ConfMan.get("description"));

	if (caption.empty()) {
		PlainGameDescriptor game = metaEngine.findGame(ConfMan.get("gameid").c_str());
		if (game.description) {
			caption = game.description;
		}
	}
	if (caption.empty())
		caption = target;
	if (!caption.empty())	{
		system.setWindowCaption(caption.decode());
	}

	//
	// Setup various paths in the SearchManager
	//

	// Add the game path to the directory search list
	engine->initializePath(dir);

	// Add extrapath (if any) to the directory search list
	if (ConfMan.hasKey("extrapath")) {
		dir = Common::FSNode(ConfMan.get("extrapath"));
		SearchMan.addDirectory(dir.getPath(), dir);
	}

	// If a second extrapath is specified on the app domain level, add that as well.
	// However, since the default hasKey() and get() check the app domain level,
	// verify that it's not already there before adding it. The search manager will
	// check for that too, so this check is mostly to avoid a warning message.
	if (ConfMan.hasKey("extrapath", Common::ConfigManager::kApplicationDomain)) {
		Common::String extraPath = ConfMan.get("extrapath", Common::ConfigManager::kApplicationDomain);
		if (!SearchMan.hasArchive(extraPath)) {
			dir = Common::FSNode(extraPath);
			SearchMan.addDirectory(dir.getPath(), dir);
		}
	}

	// On creation the engine should have set up all debug levels so we can use
	// the command line arguments here
	Common::StringTokenizer tokenizer(edebuglevels, " ,");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token.equalsIgnoreCase("all"))
			DebugMan.enableAllDebugChannels();
		else if (!DebugMan.enableDebugChannel(token))
			warning("Engine does not support debug level '%s'", token.c_str());
	}

#ifdef USE_TRANSLATION
	Common::String previousLanguage = TransMan.getCurrentLanguage();
	if (ConfMan.hasKey("gui_use_game_language")
	    && ConfMan.getBool("gui_use_game_language")
	    && ConfMan.hasKey("language")) {
		TransMan.setLanguage(ConfMan.get("language"));
#ifdef USE_TTS
		Common::TextToSpeechManager *ttsMan;
		if ((ttsMan = g_system->getTextToSpeechManager()) != nullptr) {
			ttsMan->setLanguage(ConfMan.get("language"));
		}
#endif // USE_TTS
	}
#endif // USE_TRANSLATION

	// Initialize any game-specific keymaps
	Common::KeymapArray gameKeymaps = metaEngineConnect.initKeymaps(target.c_str());
	Common::Keymapper *keymapper = system.getEventManager()->getKeymapper();
	for (uint i = 0; i < gameKeymaps.size(); i++) {
		keymapper->addGameKeymap(gameKeymaps[i]);
	}

	system.applyBackendSettings();

	// Inform backend that the engine is about to be run
	system.engineInit();

	// Run the engine
	Common::Error result = engine->run();

	// Inform backend that the engine finished
	system.engineDone();

	// Clean up any game-specific keymaps
	keymapper->cleanupGameKeymaps();

	// Free up memory
	delete engine;

	// We clear all debug levels again even though the engine should do it
	DebugMan.clearAllDebugChannels();

	// Reset the file/directory mappings
	SearchMan.clear();

#ifdef USE_TRANSLATION
	TransMan.setLanguage(previousLanguage);
#ifdef USE_TTS
		Common::TextToSpeechManager *ttsMan;
		if ((ttsMan = g_system->getTextToSpeechManager()) != nullptr) {
			ttsMan->setLanguage(ConfMan.get("language"));
		}
#endif // USE_TTS
#endif // USE_TRANSLATION

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
		if (ConfMan.hasKey("filtering"))
			system.setFeatureState(OSystem::kFeatureFilteringMode, ConfMan.getBool("filtering"));
		if (ConfMan.hasKey("stretch_mode"))
			system.setStretchMode(ConfMan.get("stretch_mode").c_str());
		if (ConfMan.hasKey("shader"))
			system.setShader(ConfMan.get("shader").c_str());
	system.endGFXTransaction();

	system.applyBackendSettings();

	// When starting up launcher for the first time, the user might have specified
	// a --gui-theme option, to allow that option to be working, we need to initialize
	// GUI here.
	// FIXME: Find a nicer way to allow --gui-theme to be working
	GUI::GuiManager::instance();

	// Set initial window caption
	system.setWindowCaption(Common::U32String(gScummVMFullVersion));

	// Clear the main screen
	system.fillScreen(0);
}

static void setupKeymapper(OSystem &system) {
	using namespace Common;

	Keymapper *mapper = system.getEventManager()->getKeymapper();
	mapper->clear();

	// Query the backend for hardware keys and default bindings and register them
	HardwareInputSet *inputSet = system.getHardwareInputSet();
	KeymapperDefaultBindings *backendDefaultBindings = system.getKeymapperDefaultBindings();

	mapper->registerHardwareInputSet(inputSet, backendDefaultBindings);

	Keymap *primaryGlobalKeymap = system.getEventManager()->getGlobalKeymap();
	if (primaryGlobalKeymap) {
		mapper->addGlobalKeymap(primaryGlobalKeymap);
	}

	// Get the platform-specific global keymap (if it exists)
	KeymapArray platformKeymaps = system.getGlobalKeymaps();
	for (uint i = 0; i < platformKeymaps.size(); i++) {
		mapper->addGlobalKeymap(platformKeymaps[i]);
	}
}

extern "C" int scummvm_main(int argc, const char * const argv[]) {
	Common::String specialDebug;
	Common::String command;

	// Verify that the backend has been initialized (i.e. g_system has been set).
	assert(g_system);
	OSystem &system = *g_system;

	// Register config manager defaults
	Base::registerDefaults();
	system.registerDefaultSettings(Common::ConfigManager::kApplicationDomain);

	// Parse the command line
	Common::StringMap settings;
	command = Base::parseCommandLine(settings, argc, argv);

	// Load the config file (possibly overridden via command line):
	if (settings.contains("config")) {
		ConfMan.loadConfigFile(settings["config"]);
		settings.erase("config");
	} else {
		ConfMan.loadDefaultConfigFile();
	}

	// Update the config file
	ConfMan.set("versioninfo", gScummVMVersion, Common::ConfigManager::kApplicationDomain);

	// Load and setup the debuglevel and the debug flags. We do this at the
	// soonest possible moment to ensure debug output starts early on, if
	// requested.
	if (settings.contains("debuglevel")) {
		gDebugLevel = (int)strtol(settings["debuglevel"].c_str(), 0, 10);
		printf("Debuglevel (from command line): %d\n", gDebugLevel);
		settings.erase("debuglevel"); // This option should not be passed to ConfMan.
	} else if (ConfMan.hasKey("debuglevel"))
		gDebugLevel = ConfMan.getInt("debuglevel");

	if (settings.contains("debugflags")) {
		specialDebug = settings["debugflags"];
		settings.erase("debugflags");
	} else if (ConfMan.hasKey("debugflags"))
		specialDebug = ConfMan.get("debugflags");

	if (settings.contains("debug-channels-only"))
		gDebugChannelsOnly = true;


	ConfMan.registerDefault("always_run_fallback_detection_extern", true);
	PluginManager::instance().init();
 	PluginManager::instance().loadAllPlugins(); // load plugins for cached plugin manager
	PluginManager::instance().loadDetectionPlugin(); // load detection plugin for uncached plugin manager

	// If we received an invalid music parameter via command line we check this here.
	// We can't check this before loading the music plugins.
	// On the other hand we cannot load the plugins before we know the file paths (in case of external plugins).
	if (settings.contains("music-driver")) {
		if (MidiDriver::getMusicType(MidiDriver::getDeviceHandle(settings["music-driver"])) == MT_INVALID) {
			warning("Unrecognized music driver '%s'. Switching to default device", settings["music-driver"].c_str());
			settings["music-driver"] = "auto";
		}
	}

	// Process the remaining command line settings. Must be done after the
	// config file and the plugins have been loaded.
	Common::Error res;

	// TODO: deal with settings that require plugins to be loaded
	if (Base::processSettings(command, settings, res)) {
		if (res.getCode() != Common::kNoError)
			warning("%s", res.getDesc().c_str());
		return res.getCode();
	}

	if (settings.contains("dump-midi")) {
		// Store this command line setting in ConfMan, since all transient settings are destroyed
		ConfMan.registerDefault("dump_midi", true);
	}


	// Init the backend. Must take place after all config data (including
	// the command line params) was read.
	system.initBackend();

	// If we received an invalid graphics mode parameter via command line
	// we check this here. We can't do it until after the backend is inited,
	// or there won't be a graphics manager to ask for the supported modes.

	if (settings.contains("gfx-mode")) {
		const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
		Common::String option = settings["gfx-mode"];
		bool isValid = false;

		while (gm->name && !isValid) {
			isValid = !scumm_stricmp(gm->name, option.c_str());
			gm++;
		}
		if (!isValid) {
			warning("Unrecognized graphics mode '%s'. Switching to default mode", option.c_str());
			settings["gfx-mode"] = "default";
		}
	}
	if (settings.contains("disable-display")) {
		ConfMan.setInt("disable-display", 1, Common::ConfigManager::kTransientDomain);
	}
	setupGraphics(system);

	// Init the different managers that are used by the engines.
	// Do it here to prevent fragmentation later
	system.getAudioCDManager();
	MusicManager::instance();
	Common::DebugManager::instance();

	// Init the event manager. As the virtual keyboard is loaded here, it must
	// take place after the backend is initiated and the screen has been setup
	system.getEventManager()->init();

#ifdef ENABLE_EVENTRECORDER
	// Directly after initializing the event manager, we will initialize our
	// event recorder.
	//
	// TODO: This is just to match the current behavior, when we further extend
	// our event recorder, we might do this at another place. Or even change
	// the whole API for that ;-).
	g_eventRec.RegisterEventSource();
#endif

	Common::OSDMessageQueue::instance().registerEventSource();

	// Now as the event manager is created, setup the keymapper
	setupKeymapper(system);

#ifdef USE_UPDATES
	if (!ConfMan.hasKey("updates_check") && g_system->getUpdateManager()) {
		GUI::UpdatesDialog dlg;
		dlg.runModal();
	}
#endif

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	CloudMan.init();
	CloudMan.syncSaves();
#endif

	// Unless a game was specified, show the launcher dialog
	if (0 == ConfMan.getActiveDomain())
		launcherDialog();

	// FIXME: We're now looping the launcher. This, of course, doesn't
	// work as well as it should. In theory everything should be destroyed
	// cleanly, so this is now enabled to encourage people to fix bits :)
	while (0 != ConfMan.getActiveDomain()) {
		saveLastLaunchedTarget(ConfMan.getActiveDomainName());

		EngineMan.upgradeTargetIfNecessary(ConfMan.getActiveDomainName());

		// Try to find a MetaEnginePlugin which feels responsible for the specified game.
		const Plugin *plugin = detectPlugin();
		if (plugin) {
			// Unload all plugins not needed for this game, to save memory

			// Right now, we have a MetaEngine plugin, and we want to unload all except Engine.
			// First, get the relevant Engine plugin from MetaEngine.
			const Plugin *enginePlugin = PluginMan.getEngineFromMetaEngine(plugin);

			// Then, pass in the pointer to enginePlugin, with the matching type, so our function behaves as-is.
			PluginManager::instance().unloadPluginsExcept(PLUGIN_TYPE_ENGINE, enginePlugin);

#if defined(UNCACHED_PLUGINS) && defined(DYNAMIC_MODULES)
			// Unload all MetaEngines not needed for the current engine, if we're using uncached plugins
			// to save extra memory.
			PluginManager::instance().unloadPluginsExcept(PLUGIN_TYPE_ENGINE_DETECTION, plugin);
#endif

#ifdef ENABLE_EVENTRECORDER
			Common::String recordMode = ConfMan.get("record_mode");
			Common::String recordFileName = ConfMan.get("record_file_name");

			if (recordMode == "record") {
				g_eventRec.init(g_eventRec.generateRecordFileName(ConfMan.getActiveDomainName()), GUI::EventRecorder::kRecorderRecord);
			} else if (recordMode == "playback") {
				g_eventRec.init(recordFileName, GUI::EventRecorder::kRecorderPlayback);
			} else if ((recordMode == "info") && (!recordFileName.empty())) {
				Common::PlaybackFile record;
				record.openRead(recordFileName);
				debug("info:author=%s name=%s description=%s", record.getHeader().author.c_str(), record.getHeader().name.c_str(), record.getHeader().description.c_str());
				break;
			}
#endif
#ifdef USE_TTS
			Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
			if (ttsMan != nullptr) {
				ttsMan->pushState();
			}
#endif
			// Try to run the game
			Common::Error result = runGame(plugin, system, specialDebug);

#ifdef USE_TTS
			if (ttsMan != nullptr) {
				ttsMan->popState();
			}
#endif

#ifdef ENABLE_EVENTRECORDER
			// Flush Event recorder file. The recorder does not get reinitialized for next game
			// which is intentional. Only single game per session is allowed.
			g_eventRec.deinit();
#endif

#if defined(UNCACHED_PLUGINS) && defined(DYNAMIC_MODULES)
			// do our best to prevent fragmentation by unloading as soon as we can
			PluginManager::instance().unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false);
			PluginManager::instance().unloadDetectionPlugin();
			// reallocate the config manager to get rid of any fragmentation
			ConfMan.defragment();
			// The keymapper keeps pointers to the configuration domains. It needs to be reinitialized.
			setupKeymapper(system);
#endif

			// Did an error occur ?
			if (result.getCode() != Common::kNoError && result.getCode() != Common::kUserCanceled) {
				// Shows an informative error dialog if starting the selected game failed.
				GUI::displayErrorDialog(result, _("Error running game:"));
			}

			// Quit unless an error occurred, or Return to launcher was requested
			if (result.getCode() == Common::kNoError && !g_system->getEventManager()->shouldReturnToLauncher() &&
			    !g_system->hasFeature(OSystem::kFeatureNoQuit) && !ConfMan.getBool("gui_return_to_launcher_at_exit"))
				break;

			// Reset the return to launcher and quit flags in case we want to load another engine
			g_system->getEventManager()->resetReturnToLauncher();
			g_system->getEventManager()->resetQuit();

#ifdef ENABLE_EVENTRECORDER
			if (g_eventRec.checkForContinueGame()) {
				continue;
			}
#endif

			// At this point, we usually return to the launcher. However, the
			// game may have requested that one or more other games be "chained"
			// to the current one, with optional save slots to start the games
			// at. At the time of writing, this is used for the Maniac Mansion
			// easter egg in Day of the Tentacle.

			Common::String chainedGame;
			int saveSlot = -1;

			ChainedGamesMan.pop(chainedGame, saveSlot);

			// Discard any command line options. It's unlikely that the user
			// wanted to apply them to *all* games ever launched.
			ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();

			if (!chainedGame.empty()) {
				if (saveSlot != -1) {
					ConfMan.setInt("save_slot", saveSlot, Common::ConfigManager::kTransientDomain);
				}
				// Start the chained game
				ConfMan.setActiveDomain(chainedGame);
			} else {
				// Clear the active config domain
				ConfMan.setActiveDomain("");
			}

			PluginManager::instance().loadAllPluginsOfType(PLUGIN_TYPE_ENGINE); // only for cached manager
			PluginManager::instance().loadDetectionPlugin(); // only for uncached manager
		} else {
			GUI::displayErrorDialog(_("Could not find any engine capable of running the selected game"));

			// Clear the active domain
			ConfMan.setActiveDomain("");
		}

		// reset the graphics to default
		setupGraphics(system);
		if (0 == ConfMan.getActiveDomain()) {
			launcherDialog();
		}
	}
#ifdef USE_CLOUD
#ifdef USE_SDL_NET
	Networking::LocalWebserver::destroy();
#endif
#ifdef USE_LIBCURL
	Networking::ConnectionManager::destroy();
	//I think it's important to destroy it after ConnectionManager
	Cloud::CloudManager::destroy();
#endif
#endif
	PluginManager::instance().unloadDetectionPlugin();
	PluginManager::instance().unloadAllPlugins();
	PluginManager::destroy();
	GUI::GuiManager::destroy();
	Common::ConfigManager::destroy();
	Common::DebugManager::destroy();
	Common::OSDMessageQueue::destroy();
#ifdef ENABLE_EVENTRECORDER
	GUI::EventRecorder::destroy();
#endif
	Common::SearchManager::destroy();
#ifdef USE_TRANSLATION
	Common::MainTranslationManager::destroy();
#endif
	MusicManager::destroy();
	Graphics::CursorManager::destroy();
	Graphics::FontManager::destroy();
#ifdef USE_FREETYPE2
	Graphics::shutdownTTF();
#endif
	EngineManager::destroy();
	Graphics::YUVToRGBManager::destroy();

	return 0;
}
