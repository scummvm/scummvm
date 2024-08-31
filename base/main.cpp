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

#ifdef ANDROID_BACKEND
#include "backends/fs/android/android-fs-factory.h"
#endif

#include "gui/dump-all-dialogs.h"

static bool launcherDialog() {

	// Discard any command line options. Those that affect the graphics
	// mode and the others (like bootparam etc.) should not
	// blindly be passed to the first game launched from the launcher.
	ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();

	// If the backend does not allow quitting, loop on the launcher until a game is started
	bool noQuit = g_system->hasFeature(OSystem::kFeatureNoQuit);
	bool status = true;
	do {
#if defined(__DC__)
		DCLauncherDialog dlg;
#else
		GUI::LauncherChooser dlg;
		dlg.selectLauncher();
#endif
		status = (dlg.runModal() != -1);
	} while (noQuit && nullptr == ConfMan.getActiveDomain());
	return status;
}

static Common::Error identifyGame(const Common::String &debugLevels, const Plugin **detectionPlugin, DetectedGame &game, const void **descriptor) {
	assert(detectionPlugin);

	// Figure out the engine ID and game ID
	Common::String engineId = ConfMan.get("engineid");
	Common::String gameId = ConfMan.get("gameid");

	// Print text saying what's going on
	printf("User picked target '%s' (engine ID '%s', game ID '%s')...\n", ConfMan.getActiveDomainName().c_str(), engineId.c_str(), gameId.c_str());

	// At this point the engine ID and game ID must be known
	if (engineId.empty()) {
		warning("The engine ID is not set for target '%s'", ConfMan.getActiveDomainName().c_str());
		return Common::kUnknownError;
	}

	if (gameId.empty()) {
		warning("The game ID is not set for target '%s'", ConfMan.getActiveDomainName().c_str());
		return Common::kUnknownError;
	}

	*detectionPlugin = EngineMan.findDetectionPlugin(engineId);
	if (!*detectionPlugin) {
		warning("'%s' is an invalid engine ID. Use the --list-engines command to list supported engine IDs", engineId.c_str());
		return Common::kMetaEnginePluginNotFound;
	}
	// Query the plugin for the game descriptor
	MetaEngineDetection &metaEngine = (*detectionPlugin)->get<MetaEngineDetection>();

	// before doing anything, we register the debug channels of the (Meta)Engine(Detection)
	DebugMan.addAllDebugChannels(metaEngine.getDebugChannels());
	// Setup now the debug channels
	Common::StringTokenizer tokenizer(debugLevels, " ,");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token.equalsIgnoreCase("all"))
			DebugMan.enableAllDebugChannels();
		else if (!DebugMan.enableDebugChannel(token))
			warning("Engine does not support debug level '%s'", token.c_str());
	}

	Common::Error result = metaEngine.identifyGame(game, descriptor);
	if (result.getCode() != Common::kNoError) {
		warning("Couldn't identify game '%s' for the engine '%s'.", gameId.c_str(), engineId.c_str());
	}
	return result;
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
static Common::Error runGame(const Plugin *enginePlugin, OSystem &system, const DetectedGame &game, const void *meDescriptor) {
	assert(enginePlugin);

	// Determine the game data path, for validation and error messages
	Common::FSNode dir(ConfMan.getPath("path"));
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

	// Create the game's MetaEngine.
	MetaEngine &metaEngine = enginePlugin->get<MetaEngine>();
	if (err.getCode() == Common::kNoError) {
		// Set default values for all of the custom engine options
		// Apparently some engines query them in their constructor, thus we
		// need to set this up before instance creation.
		metaEngine.registerDefaultSettings(target);
		err = metaEngine.createInstance(&system, &engine, game, meDescriptor);
	}

	// Check for errors
	if (!engine || err.getCode() != Common::kNoError) {

		// Print a warning; note that scummvm_main will also
		// display an error dialog, so we don't have to do this here.
		warning("%s failed to instantiate engine: %s (target '%s', path '%s')",
			game.engineId.c_str(),
			err.getDesc().c_str(),
			target.c_str(),
			dir.getPath().toString(Common::Path::kNativeSeparator).c_str()
			);

		// If a temporary target failed to launch, remove it from the configuration manager
		// so it not visible in the launcher.
		// Temporary targets are created when starting games from the command line using the game id.
		if (ConfMan.hasKey("id_came_from_command_line")) {
			ConfMan.removeGameDomain(target.c_str());
		}

		return err;
	}

	// Set up the metaengine
	engine->setMetaEngine(&metaEngine);

	// Set the window caption to the game name
	Common::String caption(ConfMan.get("description"));

	if (caption.empty())
		caption = game.description;
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
		dir = Common::FSNode(ConfMan.getPath("extrapath"));
		SearchMan.addDirectory(dir);
	}

	// If a second extrapath is specified on the app domain level, add that as well.
	// However, since the default hasKey() and get() check the app domain level,
	// verify that it's not already there before adding it. The search manager will
	// check for that too, so this check is mostly to avoid a warning message.
	if (ConfMan.hasKey("extrapath", Common::ConfigManager::kApplicationDomain)) {
		Common::Path extraPath = ConfMan.getPath("extrapath", Common::ConfigManager::kApplicationDomain);
		dir = Common::FSNode(extraPath);
		if (!SearchMan.hasArchive(dir.getPath().toString())) {
			SearchMan.addDirectory(dir);
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
	metaEngine.deleteInstance(engine, game, meDescriptor);

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
		system.setShader(ConfMan.getPath("shader"));

#if defined(OPENDINGUX) || defined(MIYOO) || defined(MIYOOMINI) || defined(ATARI)
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
	command = Base::parseCommandLine(settings, argc, argv);

	// Check for backend start settings
	Common::String executable;
	if (argc && argv && argv[0]) {
		const char *s = strrchr(argv[0], '/');
		if (!s)
			s = strrchr(argv[0], '\\');
		executable = s ? (s + 1) : argv[0];
	}
	Common::StringArray additionalArgs;
	system.updateStartSettings(executable, command, settings, additionalArgs);

	if (!additionalArgs.empty()) {
		// Parse those additional command line arguments.
		additionalArgs.insert_at(0, executable);
		uint argumentsSize = additionalArgs.size();
		char **arguments = (char **)malloc(argumentsSize * sizeof(char *));
		for (uint i = 0; i < argumentsSize; i++) {
			arguments[i] = (char *)malloc(additionalArgs[i].size() + 1);
			Common::strlcpy(arguments[i], additionalArgs[i].c_str(), additionalArgs[i].size() + 1);
		}

		Common::StringMap additionalSettings;
		Common::String additionalCommand = Base::parseCommandLine(additionalSettings, argumentsSize, arguments);

		for (uint i = 0; i < argumentsSize; i++)
			free(arguments[i]);
		free(arguments);

		// Merge additional settings and command with command line. Command line has priority.
		if (command.empty())
			command = additionalCommand;
		for (Common::StringMap::const_iterator x = additionalSettings.begin(); x != additionalSettings.end(); ++x) {
			if (!settings.contains(x->_key))
				settings[x->_key] = x->_value;
		}
	}

	// Load the config file (possibly overridden via command line):
	Common::Path initConfigFilename;
	if (settings.contains("initial-cfg"))
		initConfigFilename = Common::Path(settings["initial-cfg"], Common::Path::kNativeSeparator);

	bool configLoadStatus;
	if (settings.contains("config")) {
		configLoadStatus = ConfMan.loadConfigFile(
			Common::Path(settings["config"], Common::Path::kNativeSeparator), initConfigFilename);
	} else {
		configLoadStatus = ConfMan.loadDefaultConfigFile(initConfigFilename);
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

	if (!configLoadStatus) {
		GUI::MessageDialog alert(_("Bad config file format. overwrite?"), _("Yes"), _("Cancel"));
		if (alert.runModal() != GUI::kMessageOK)
   			return 0;
	}
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

#ifdef ANDROID_BACKEND
	// This early popup message for Android, informing the users about important
	// changes to file access, needs to be *after* language for the GUI has been selected.
	// Hence, we instantiate GUI Manager here, to take care of this.
	GUI::GuiManager::instance();
	if (AndroidFilesystemFactory::instance().hasSAF()
		&& !ConfMan.hasKey("android_saf_dialog_shown")) {

		bool cancelled = false;

		if (!ConfMan.getGameDomains().empty()) {
			GUI::MessageDialog alert(_(
				// I18N: <Add a new folder> must match the translation done in backends/fs/android/android-saf-fs.h
				"In this new version of ScummVM for Android, significant changes were made to "
				"the file access system to allow support for modern versions of the Android "
				"Operating System.\n"
				"If you find that your existing added games or custom paths no longer work, "
				"please edit those paths:"
				"\n"
				"  1. From the Launcher, go to **Game Options > Paths**."
				" Select **Game Path** or **Extra Path**, as appropriate. \n"
				"  2. Inside the ScummVM file browser, select **Go Up** until you reach the root folder "
				"which has the **<Add a new folder>** option.\n"
				"  3. Double-tap **<Add a new folder>**. In your device's file browser, navigate to the folder "
				"containing all your game folders. For example, **SD Card > ScummVMgames** \n"
				"  4. Select **Use this folder**. \n"
				"  5. Select **Allow** to give ScummVM permission to access the folder. \n"
				"  6. In the ScummVM file browser, double-tap to browse through your added folder. "
				"Select the folder containing the game's files, then tap **Choose**. \n"
				"\n"
				"Repeat steps 1 and 6 for each game."
				), _("Ok"),
				// I18N: A button caption to dismiss amessage and read it later
				_("Read Later"), Graphics::kTextAlignLeft);

			if (alert.runModal() != GUI::kMessageOK)
				cancelled = true;
		} else {
			GUI::MessageDialog alert(_(
				// I18N: <Add a new folder> must match the translation done in backends/fs/android/android-saf-fs.h
				"In this new version of ScummVM for Android, significant changes were made to "
				"the file access system to allow support for modern versions of the Android "
				"Operating System.\n"
				"To add a game:\n"
				"\n"
				"  1. Select **Add Game...** from the launcher. \n"
				"  2. Inside the ScummVM file browser, select **Go Up** until you reach the root folder "
				"which has the **<Add a new folder>** option.\n"
				"  3. Double-tap **<Add a new folder>**. In your device's file browser, navigate to the folder "
				"containing all your game folders. For example, **SD Card > ScummVMgames** \n"
				"  4. Select **Use this folder**. \n"
				"  5. Select **Allow** to give ScummVM permission to access the folder. \n"
				"  6. In the ScummVM file browser, double-tap to browse through your added folder. "
				"Select the sub-folder containing the game's files, then tap **Choose**."
				"\n"
				"Repeat steps 1 and 6 for each game."
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

#if 0
	GUI::dumpAllDialogs();
#endif

// Print out CPU extension info
// Separate block to keep the stack clean
	{
		Common::String extensionSupportString[3] = { "not supported", "disabled", "enabled" };

		byte sse2Support = 0;
		byte avx2Support = 0;
		byte neonSupport = 0;

#ifdef SCUMMVM_SSE2
		++sse2Support;
		if (g_system->hasFeature(OSystem::kFeatureCpuSSE2))
			++sse2Support;
#endif
#ifdef SCUMMVM_AVX2
		++avx2Support;
		if (g_system->hasFeature(OSystem::kFeatureCpuAVX2))
			++avx2Support;
#endif
#ifdef SCUMMVM_NEON
		++neonSupport;
		if (g_system->hasFeature(OSystem::kFeatureCpuNEON))
			++neonSupport;
#endif

		debug(0, "CPU extensions:");
		debug(0, "SSE2(%s) AVX2(%s) NEON(%s)",
			extensionSupportString[sse2Support].c_str(),
			extensionSupportString[avx2Support].c_str(),
			extensionSupportString[neonSupport].c_str());
	}

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
		const Plugin *enginePlugin = nullptr;
		const Plugin *plugin = nullptr;
		DetectedGame game;
		const void *meDescriptor = nullptr;
		Common::Error result = identifyGame(specialDebug, &plugin, game, &meDescriptor);

		if (result.getCode() == Common::kNoError) {
			Common::String engineId = plugin->getName();
#if defined(UNCACHED_PLUGINS) && defined(DYNAMIC_MODULES) && !defined(DETECTION_STATIC)
			// Unload all MetaEnginesDetection if we're using uncached plugins to save extra memory.
			PluginManager::instance().unloadDetectionPlugin();
#endif

			// Then, get the relevant Engine plugin from MetaEngine.
			enginePlugin = PluginMan.findEnginePlugin(engineId);
			if (enginePlugin == nullptr) {
				result = Common::kEnginePluginNotFound;
			}
		}

		if (result.getCode() == Common::kNoError) {
			// Unload all plugins not needed for this game, to save memory
			// Right now, we have a MetaEngine plugin, and we want to unload all except Engine.

			// Pass in the pointer to enginePlugin, with the matching type, so our function behaves as-is.
			PluginManager::instance().unloadPluginsExcept(PLUGIN_TYPE_ENGINE, enginePlugin);

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

				DebugMan.removeAllDebugChannels();
				break;
			}
#endif
			Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
			if (ttsMan != nullptr) {
				ttsMan->pushState();
			}
			// Try to run the game
			result = runGame(enginePlugin, system, game, meDescriptor);
			if (ttsMan != nullptr) {
				ttsMan->popState();
			}

			DebugMan.removeAllDebugChannels();

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
			DebugMan.removeAllDebugChannels();

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
