/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "common/file.h"
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
#include "gui/message.h"

#include "audio/mididrv.h"
#include "audio/musicplugin.h"  /* for music manager */

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/yuv_to_rgb.h"
#ifdef USE_FREETYPE2
#include "graphics/fonts/ttf.h"
#endif
#include "graphics/scalerplugin.h"

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

#ifdef __ANDROID__
#include "backends/fs/android/android-fs-factory.h"
#endif

static bool launcherDialog() {

	// Discard any command line options. Those that affect the graphics
	// mode and the others (like bootparam etc.) should not
	// blindly be passed to the first game launched from the launcher.
	ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();

#if defined(__DC__)
	DCLauncherDialog dlg;
#else
	GUI::LauncherChooser dlg;
	dlg.selectLauncher();
#endif

#ifdef __ANDROID__
	// This early popup message for Android, informing the users about important
	// changes to file access, needs to be after language for the GUI has been selected
	if (AndroidFilesystemFactory::instance().hasSAF()
		&& !ConfMan.hasKey("android_saf_dialog_shown")) {

		bool cancelled = false;

		if (!ConfMan.getGameDomains().empty()) {
			GUI::MessageDialog alert(_(
				// I18N: <Add a new folder> must match the translation done in backends/fs/android/android-saf-fs.h
				"In this new version of ScummVM Android, significant changes were made to "
				"the file access system to allow support for modern versions of the Android "
				"Operating System.\n"
				"If you find that your existing added games or custom paths no longer work, "
				"please edit those paths and this time use the SAF system to browse to the "
				"desired locations.\n"
				"To do that:\n"
				"\n"
				"  1. For each game whose data is not found, go to the \"Paths\" tab in "
				"the \"Game Options\" and change the \"Game path\"\n"
				"  2. Inside the ScummVM file browser, use \"Go Up\" until you reach "
				"the \"root\" folder where you will see the \"<Add a new folder>\" option.\n"
				"  3. Choose that, then browse and select the \"parent\" folder for your "
				"games subfolders, e.g. \"SD Card > myGames\". Click on \"Use this folder\".\n"
				"  4. Then, a new folder \"myGames\" will appear on the \"root\" folder "
				"of the ScummVM browser.\n"
				"  5. Browse through this folder to your game data.\n"
				"\n"
				"Steps 2 and 3 need to be done only once for all of your games."
				), _("Ok"),
				// I18N: A button caption to dismiss amessage and read it later
				_("Read Later"), Graphics::kTextAlignLeft);

			if (alert.runModal() != GUI::kMessageOK)
				cancelled = true;
		} else {
			GUI::MessageDialog alert(_(
				// I18N: <Add a new folder> must match the translation done in backends/fs/android/android-saf-fs.h
				"In this new version of ScummVM Android, significant changes were made to "
				"the file access system to allow support for modern versions of the Android "
				"Operating System.\n"
				"Thus, you need to set up SAF in order to be able to add the games.\n"
				"\n"
				"  1. Inside the ScummVM file browser, use \"Go Up\" until you reach "
				"the \"root\" folder where you will see the \"<Add a new folder>\" option.\n"
				"  2. Choose that, then browse and select the \"parent\" folder for your "
				"games subfolders, e.g. \"SD Card > myGames\". Click on \"Use this folder\".\n"
				"  3. Then, a new folder \"myGames\" will appear on the \"root\" folder "
				"of the ScummVM browser.\n"
				"  4. Browse through this folder to your game data."
				), _("Ok"),
				// I18N: A button caption to dismiss a message and read it later
				_("Read Later"), Graphics::kTextAlignLeft);

			if (alert.runModal() != GUI::kMessageOK)
				cancelled = true;
		}

		if (!cancelled)
			ConfMan.setBool("android_saf_dialog_shown", true);
	}
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
		return nullptr;
	}

	if (gameId.empty()) {
		warning("The game ID is not set for target '%s'", ConfMan.getActiveDomainName().c_str());
		return nullptr;
	}

	const Plugin *plugin = EngineMan.findPlugin(engineId);
	if (!plugin) {
		warning("'%s' is an invalid engine ID. Use the --list-engines command to list supported engine IDs", engineId.c_str());
		return nullptr;
	}

	// Query the plugin for the game descriptor
	const MetaEngineDetection &metaEngine = plugin->get<MetaEngineDetection>();
	printf("   Looking for a plugin supporting this target... %s\n", metaEngine.getEngineName());
	DebugMan.addAllDebugChannels(metaEngine.getDebugChannels());
	PlainGameDescriptor game = metaEngine.findGame(gameId.c_str());
	if (!game.gameId) {
		warning("'%s' is an invalid game ID for the engine '%s'. Use the --list-games option to list supported game IDs", gameId.c_str(), engineId.c_str());
		return nullptr;
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
static Common::Error runGame(const Plugin *plugin, const Plugin *enginePlugin, OSystem &system, const Common::String &debugLevels) {
	assert(plugin);
	assert(enginePlugin);

	// Determine the game data path, for validation and error messages
	Common::FSNode dir(ConfMan.get("path"));
	Common::String target = ConfMan.getActiveDomainName();
	Common::Error err = Common::kNoError;
	Engine *engine = nullptr;

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

	// Create the game's MetaEngineDetection.
	const MetaEngineDetection &metaEngineDetection = plugin->get<MetaEngineDetection>();

	// before we instantiate the engine, we register debug channels for it
	DebugMan.addAllDebugChannels(metaEngineDetection.getDebugChannels());

	// On creation the engine should have set up all debug levels so we can use
	// the command line arguments here
	Common::StringTokenizer tokenizer(debugLevels, " ,");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token.equalsIgnoreCase("all"))
			DebugMan.enableAllDebugChannels();
		else if (!DebugMan.enableDebugChannel(token))
			warning("Engine does not support debug level '%s'", token.c_str());
	}

	// Create the game's MetaEngine.
	MetaEngine &metaEngine = enginePlugin->get<MetaEngine>();
	if (err.getCode() == Common::kNoError) {
		// Set default values for all of the custom engine options
		// Apparently some engines query them in their constructor, thus we
		// need to set this up before instance creation.
		metaEngine.registerDefaultSettings(target);
	}

	err = metaEngine.createInstance(&system, &engine);

	// Check for errors
	if (!engine || err.getCode() != Common::kNoError) {

		// Print a warning; note that scummvm_main will also
		// display an error dialog, so we don't have to do this here.
		warning("%s failed to instantiate engine: %s (target '%s', path '%s')",
			metaEngineDetection.getEngineName(),
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

		DebugMan.removeAllDebugChannels();
		return err;
	}

	// Set up the metaengine
	engine->setMetaEngine(&metaEngine);

	// Set the window caption to the game name
	Common::String caption(ConfMan.get("description"));

	if (caption.empty()) {
		// here, we don't need to set the debug channels because it has been set earlier
		PlainGameDescriptor game = metaEngineDetection.findGame(ConfMan.get("gameid").c_str());
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

#ifdef USE_TRANSLATION
	Common::String previousLanguage = TransMan.getCurrentLanguage();
	if (ConfMan.hasKey("gui_use_game_language")
	    && ConfMan.getBool("gui_use_game_language")
	    && ConfMan.hasKey("language")) {
		TransMan.setLanguage(ConfMan.get("language"));
		Common::TextToSpeechManager *ttsMan;
		if ((ttsMan = g_system->getTextToSpeechManager()) != nullptr) {
			ttsMan->setLanguage(ConfMan.get("language"));
		}
	}
#endif // USE_TRANSLATION

	// Initialize any game-specific keymaps
	Common::KeymapArray gameKeymaps = metaEngine.initKeymaps(target.c_str());
	Common::Keymapper *keymapper = system.getEventManager()->getKeymapper();
	for (uint i = 0; i < gameKeymaps.size(); i++) {
		keymapper->addGameKeymap(gameKeymaps[i]);
	}

	system.applyBackendSettings();

	// Inform backend that the engine is about to be run
	system.engineInit();

	// Purge queued input events that may remain from the GUI (such as key-up)
	system.getEventManager()->purgeKeyboardEvents();
	system.getEventManager()->purgeMouseEvents();

	// Run the engine
	Common::Error result = engine->run();

	// Make sure we do not return to the launcher if this is not possible.
	if (!engine->hasFeature(Engine::kSupportsReturnToLauncher))
		ConfMan.setBool("gui_return_to_launcher_at_exit", false, Common::ConfigManager::kTransientDomain);

	// Inform backend that the engine finished
	system.engineDone();

	// Clean up any game-specific keymaps
	keymapper->cleanupGameKeymaps();

	// Free up memory
	delete engine;

	DebugMan.removeAllDebugChannels();

	// Reset the file/directory mappings
	SearchMan.clear();

#ifdef USE_TRANSLATION
	TransMan.setLanguage(previousLanguage);
	Common::TextToSpeechManager *ttsMan;
	if ((ttsMan = g_system->getTextToSpeechManager()) != nullptr) {
		ttsMan->setLanguage(ConfMan.get("language"));
	}
#endif // USE_TRANSLATION

	// Return result (== 0 means no error)
	return result;
}

static void setupGraphics(OSystem &system) {

	system.beginGFXTransaction();
		// Set the user specified graphics mode (if any).
		system.setGraphicsMode(ConfMan.get("gfx_mode").c_str());
		system.setStretchMode(ConfMan.get("stretch_mode").c_str());
		system.setScaler(ConfMan.get("scaler").c_str(), ConfMan.getInt("scale_factor"));
		system.setShader(ConfMan.get("shader"));

#if defined(OPENDINGUX) || defined(MIYOO) || defined(MIYOOMINI)
		// 0, 0 means "autodetect" but currently only SDL supports
		// it and really useful only on Opendingux. When more platforms
		// support it we will switch to it.
		system.initSize(0, 0);
#else
		system.initSize(320, 200);
#endif

		// Parse graphics configuration, implicit fallback to defaults set with RegisterDefaults()
		system.setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio"));
		system.setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen"));
		system.setFeatureState(OSystem::kFeatureFilteringMode, ConfMan.getBool("filtering"));
		system.setFeatureState(OSystem::kFeatureVSync, ConfMan.getBool("vsync"));
	system.endGFXTransaction();

	system.applyBackendSettings();

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
	Common::String autoCommand;
	bool autodetect = false;

	// Check for the autorun name
	if (argc && argv && argv[0]) {
		const char *s = strrchr(argv[0], '/');

		if (!s)
			s = strrchr(argv[0], '\\');

		const char *appName =s ? (s + 1) : argv[0];

		if (!scumm_strnicmp(appName, "scummvm-auto", strlen("scummvm-auto"))) {
			warning("Will run in autodetection mode");
			autodetect = true;
		}
	}

	Common::StringArray autorunArgs;

	// Check for the autorun file
	if (Common::File::exists("scummvm-autorun")) {
		// Okay, the file exists. We open it and if it is empty, then run in the autorun mode
		// If the file is not empty, we read command line arguments from it, one per line
		warning("Autorun file is detected");

		Common::File autorun;
		Common::String line;
		Common::String res;

		autorunArgs.push_back(argc && argv ? argv[0] : "");

		if (autorun.open("scummvm-autorun")) {
			while (!autorun.eos()) {
				line = autorun.readLine();

				if (!line.empty() && line[0] != '#') {
					autorunArgs.push_back(line);

					res += Common::String::format("\"%s\" ", line.c_str());
				}
			}
		}

		if (!res.empty())
			warning("Autorun command: %s", res.c_str());
		else
			warning("Empty autorun file");

		autorun.close();

		autodetect = true;
	}

	if (autodetect) {
		if (autorunArgs.size() > 1) {
			uint argumentsSize = autorunArgs.size();
			char **arguments = (char **)malloc(argumentsSize * sizeof(char *));

			for (uint i = 0; i < argumentsSize; i++) {
				arguments[i] = (char *)malloc(autorunArgs[i].size() + 1);
				Common::strlcpy(arguments[i], autorunArgs[i].c_str(), autorunArgs[i].size() + 1);
			}

			autoCommand = Base::parseCommandLine(settings, argumentsSize, arguments);

			for (uint i = 0; i < argumentsSize; i++)
				free(arguments[i]);

			free(arguments);
		} else {
			// Simulate autodetection
			const char * const arguments[] = { "scummvm-auto", "-p", ".", "--auto-detect" };

			warning("Running autodetection");

			autoCommand = Base::parseCommandLine(settings, ARRAYSIZE(arguments), arguments);
		}
	}

	command = Base::parseCommandLine(settings, argc, argv);

	// We allow overriding the automatic command
	if (command.empty())
		command = autoCommand;

	// Load the config file (possibly overridden via command line):
	Common::String initConfigFilename;
	if (settings.contains("initial-cfg"))
		initConfigFilename = settings["initial-cfg"];

	if (settings.contains("config")) {
		ConfMan.loadConfigFile(settings["config"], initConfigFilename);
	} else {
		ConfMan.loadDefaultConfigFile(initConfigFilename);
	}

	// Update the config file
	ConfMan.set("versioninfo", gScummVMVersion, Common::ConfigManager::kApplicationDomain);

	// Load and setup the debuglevel and the debug flags. We do this at the
	// soonest possible moment to ensure debug output starts early on, if
	// requested.
	if (settings.contains("debuglevel")) {
		gDebugLevel = (int)strtol(settings["debuglevel"].c_str(), nullptr, 10);
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


	// Now we want to enable global flags if any
	Common::StringTokenizer tokenizer(specialDebug, " ,");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token.equalsIgnoreCase("all"))
			DebugMan.enableAllDebugChannels();
		else
			DebugMan.enableDebugChannel(token);
	}

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

		PluginManager::instance().unloadDetectionPlugin();
		PluginManager::instance().unloadAllPlugins();
		PluginManager::destroy();

		return res.getCode();
	}

	if (settings.contains("dump-midi")) {
		// Store this command line setting in ConfMan, since all transient settings are destroyed
		ConfMan.registerDefault("dump_midi", true);
	}

#ifdef USE_OPENGL
	if (settings.contains("last_window_width")) {
		ConfMan.setInt("last_window_width", atoi(settings["last_window_width"].c_str()));
		ConfMan.setInt("last_window_height", atoi(settings["last_window_height"].c_str()));
	}
#endif

	// If we received an old style graphics mode parameter via command line
	// override it to default at this stage, so that the backend init won't
	// pass it onto updateOldSettings(). If it happened to be a valid new
	// graphics mode, we'll put it back after initBackend().
	Common::String gfxModeSetting;
	if (settings.contains("gfx-mode")) {
		gfxModeSetting = settings["gfx-mode"];
		if (ScalerMan.isOldGraphicsSetting(gfxModeSetting)) {
			settings["gfx-mode"] = "default";
			ConfMan.set("gfx_mode", settings["gfx-mode"], Common::ConfigManager::kSessionDomain);
		}
	}

	// Init the backend. Must take place after all config data (including
	// the command line params) was read.
	system.initBackend();

	// If we received an invalid graphics mode parameter via command line
	// we check this here. We can't do it until after the backend is inited,
	// or there won't be a graphics manager to ask for the supported modes.

	if (!gfxModeSetting.empty()) {
		const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
		bool isValid = false;

		while (gm->name && !isValid) {
			isValid = !scumm_stricmp(gm->name, gfxModeSetting.c_str());
			gm++;
		}
		if (!isValid) {
			// We will actually already have switched to default, but couldn't be sure that it was right until now.
			warning("Unrecognized graphics mode '%s'. Switching to default mode", gfxModeSetting.c_str());
		} else {
			settings["gfx-mode"] = gfxModeSetting;
			system.beginGFXTransaction();
			system.setGraphicsMode(gfxModeSetting.c_str());
			system.endGFXTransaction();
		}
		ConfMan.set("gfx_mode", gfxModeSetting, Common::ConfigManager::kSessionDomain);
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

#ifdef __ANDROID__
	if (AndroidFilesystemFactory::instance().hasSAF()
		&& !ConfMan.hasKey("android_saf_dialog_shown")) {

		bool cancelled = false;

		if (!ConfMan.getGameDomains().empty()) {
			GUI::MessageDialog alert(_(
				// I18N: <Add a new folder> must match the translation done in backends/fs/android/android-saf-fs.h
				"In this new version of ScummVM Android, significant changes were made to "
				"the file access system to allow support for modern versions of the Android "
				"Operating System.\n"
				"If you find that your existing added games or custom paths no longer work, "
				"please edit those paths and this time use the SAF system to browse to the "
				"desired locations.\n"
				"To do that:\n"
				"\n"
				"  1. For each game whose data is not found, go to the \"Paths\" tab in "
				"the \"Game Options\" and change the \"Game path\"\n"
				"  2. Inside the ScummVM file browser, use \"Go Up\" until you reach "
				"the \"root\" folder where you will see the \"<Add a new folder>\" option.\n"
				"  3. Choose that, then browse and select the \"parent\" folder for your "
				"games subfolders, e.g. \"SD Card > myGames\". Click on \"Use this folder\".\n"
				"  4. Then, a new folder \"myGames\" will appear on the \"root\" folder "
				"of the ScummVM browser.\n"
				"  5. Browse through this folder to your game data.\n"
				"\n"
				"Steps 2 and 3 need to be done only once for all of your games."
				), _("Ok"),
				// I18N: A button caption to dismiss amessage and read it later
				_("Read Later"), Graphics::kTextAlignLeft);

			if (alert.runModal() != GUI::kMessageOK)
				cancelled = true;
		} else {
			GUI::MessageDialog alert(_(
				// I18N: <Add a new folder> must match the translation done in backends/fs/android/android-saf-fs.h
				"In this new version of ScummVM Android, significant changes were made to "
				"the file access system to allow support for modern versions of the Android "
				"Operating System.\n"
				"Thus, you need to set up SAF in order to be able to add the games.\n"
				"\n"
				"  1. Inside the ScummVM file browser, use \"Go Up\" until you reach "
				"the \"root\" folder where you will see the \"<Add a new folder>\" option.\n"
				"  2. Choose that, then browse and select the \"parent\" folder for your "
				"games subfolders, e.g. \"SD Card > myGames\". Click on \"Use this folder\".\n"
				"  3. Then, a new folder \"myGames\" will appear on the \"root\" folder "
				"of the ScummVM browser.\n"
				"  4. Browse through this folder to your game data."
				), _("Ok"),
				// I18N: A button caption to dismiss a message and read it later
				_("Read Later"), Graphics::kTextAlignLeft);

			if (alert.runModal() != GUI::kMessageOK)
				cancelled = true;
		}

		if (!cancelled)
			ConfMan.setBool("android_saf_dialog_shown", true);
	}
#endif

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	CloudMan.init();
	CloudMan.syncSaves();
#endif

	// Unless a game was specified, show the launcher dialog
	if (nullptr == ConfMan.getActiveDomain())
		launcherDialog();

	// FIXME: We're now looping the launcher. This, of course, doesn't
	// work as well as it should. In theory everything should be destroyed
	// cleanly, so this is now enabled to encourage people to fix bits :)
	while (nullptr != ConfMan.getActiveDomain()) {
		saveLastLaunchedTarget(ConfMan.getActiveDomainName());

		EngineMan.upgradeTargetIfNecessary(ConfMan.getActiveDomainName());

		// Try to find a MetaEnginePlugin which feels responsible for the specified game.
		const Plugin *plugin = detectPlugin();

		// Then, get the relevant Engine plugin from MetaEngine.
		const Plugin *enginePlugin = nullptr;
		if (plugin)
			enginePlugin = PluginMan.getEngineFromMetaEngine(plugin);

		if (enginePlugin) {
			// Unload all plugins not needed for this game, to save memory
			// Right now, we have a MetaEngine plugin, and we want to unload all except Engine.

			// Pass in the pointer to enginePlugin, with the matching type, so our function behaves as-is.
			PluginManager::instance().unloadPluginsExcept(PLUGIN_TYPE_ENGINE, enginePlugin);

#if defined(UNCACHED_PLUGINS) && defined(DYNAMIC_MODULES) && !defined(DETECTION_STATIC)
			// Unload all MetaEngines not needed for the current engine, if we're using uncached plugins
			// to save extra memory.
			PluginManager::instance().unloadPluginsExcept(PLUGIN_TYPE_ENGINE_DETECTION, plugin);
#endif

#ifdef ENABLE_EVENTRECORDER
			Common::String recordMode = ConfMan.get("record_mode");
			Common::String recordFileName = ConfMan.get("record_file_name");

			if (recordMode == "record") {
				Common::String targetFileName = ConfMan.hasKey("record_file_name") ? recordFileName : g_eventRec.generateRecordFileName(ConfMan.getActiveDomainName());
				g_eventRec.init(targetFileName, GUI::EventRecorder::kRecorderRecord);
			} else if (recordMode == "update") {
				g_eventRec.init(recordFileName, GUI::EventRecorder::kRecorderUpdate);
			} else if (recordMode == "playback") {
				g_eventRec.init(recordFileName, GUI::EventRecorder::kRecorderPlayback);
			} else if ((recordMode == "info") && (!recordFileName.empty())) {
				Common::PlaybackFile record;
				record.openRead(recordFileName);
				debug("info:author=%s name=%s description=%s", record.getHeader().author.c_str(), record.getHeader().name.c_str(), record.getHeader().description.c_str());
				break;
			}
#endif
			Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
			if (ttsMan != nullptr) {
				ttsMan->pushState();
			}
			// Try to run the game
			Common::Error result = runGame(plugin, enginePlugin, system, specialDebug);
			if (ttsMan != nullptr) {
				ttsMan->popState();
			}

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
		if (nullptr == ConfMan.getActiveDomain()) {
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
