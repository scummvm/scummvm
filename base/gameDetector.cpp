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

#include "common/stdafx.h"

#include "base/engine.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/config-manager.h"
#include "common/system.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#ifdef UNIX
#include <errno.h>
#include <sys/stat.h>
#ifdef MACOSX
#define DEFAULT_SAVE_PATH "Documents/ScummVM Savegames"
#else
#define DEFAULT_SAVE_PATH ".scummvm"
#endif
#elif defined(__SYMBIAN32__)
#include <errno.h>
#include <sys/stat.h>
#define DEFAULT_SAVE_PATH "Savegames"
#endif

static const char USAGE_STRING[] =
	"%s: %s\n"
	"Usage: %s [OPTIONS]... [GAME]\n"
	"\n"
	"Try '%s --help' for more options.\n"
;

// DONT FIXME: DO NOT ORDER ALPHABETICALLY, THIS IS ORDERED BY IMPORTANCE/CATEGORY! :)
#if defined(PALMOS_MODE) || defined(__SYMBIAN32__) || defined(__GP32__)
static const char HELP_STRING[] = "NoUsageString"; // save more data segment space
#else
static const char HELP_STRING[] =
	"ScummVM - Graphical Adventure Game Interpreter\n"
	"Usage: %s [OPTIONS]... [GAME]\n"
	"  -v, --version            Display ScummVM version information and exit\n"
	"  -h, --help               Display a brief help text and exit\n"
	"  -z, --list-games         Display list of supported games and exit\n"
	"  -t, --list-targets       Display list of configured targets and exit\n"
	"\n"
	"  -c, --config=CONFIG      Use alternate configuration file\n"
	"  -p, --path=PATH          Path to where the game is installed\n"
	"  -x, --save-slot[=NUM]    Save game slot to load (default: autosave)\n"
	"  -f, --fullscreen         Force full-screen mode\n"
	"  -F, --no-fullscreen      Force windowed mode\n"
	"  -g, --gfx-mode=MODE      Select graphics scaler (normal,2x,3x,2xsai,\n"
	"                           super2xsai,supereagle,advmame2x,advmame3x,hq2x,\n"
	"                           hq3x,tv2x,dotmatrix)\n"
	"  -e, --music-driver=MODE  Select music driver (see README for details)\n"
	"  -q, --language=LANG      Select language (en,de,fr,it,pt,es,jp,zh,kr,se,gb,\n"
	"                           hb,ru,cz)\n"
	"  -m, --music-volume=NUM   Set the music volume, 0-255 (default: 192)\n"
	"  -s, --sfx-volume=NUM     Set the sfx volume, 0-255 (default: 192)\n"
	"  -r, --speech-volume=NUM  Set the speech volume, 0-255 (default: 192)\n"
	"  --midi-gain=NUM          Set the gain for MIDI playback, 0-1000 (default:\n"
        "                           100) (only supported by some MIDI drivers)\n"
	"  -n, --subtitles          Enable subtitles (use with games that have voice)\n"
	"  -b, --boot-param=NUM     Pass number to the boot script (boot param)\n"
	"  -d, --debuglevel=NUM     Set debug verbosity level\n"
	"  --debugflags=FLAGS       Enables engine specific debug flags\n"
	"                           (separated by commas)\n"
	"  -u, --dump-scripts       Enable script dumping if a directory called 'dumps'\n"
	"                           exists in the current directory\n"
	"\n"
	"  --cdrom=NUM              CD drive to play CD audio from (default: 0 = first\n"
	"                           drive)\n"
	"  --joystick[=NUM]         Enable input with joystick (default: 0 = first\n"
	"                           joystick)\n"
	"  --platform=WORD          Specify platform of game (allowed values: 3do, acorn,\n"
	"                           amiga, atari, c64, fmtowns, nes, mac, pc, segacd,\n"
	"                           windows)\n"
	"  --savepath=PATH          Path to where savegames are stored\n"
	"  --soundfont=FILE         Select the SoundFont for MIDI playback (only\n"
	"                           supported by some MIDI drivers)\n"
	"  --multi-midi             Enable combination Adlib and native MIDI\n"
	"  --native-mt32            True Roland MT-32 (disable GM emulation)\n"
	"  --enable-gs              Enable Roland GS mode for MIDI playback\n"
	"  --output-rate=RATE       Select output sample rate in Hz (e.g. 22050)\n"
	"  --aspect-ratio           Enable aspect ratio correction\n"
	"  --render-mode=MODE       Enable additional render modes (cga, ega, hercGreen,\n"
	"                           hercAmber, amiga)\n"
	"  --force-1x-overlay       Make inner GUI 320x200\n"
	"\n"
#if !defined(DISABLE_SKY) || !defined(DISABLE_QUEEN)
	"  --alt-intro              Use alternative intro for CD versions of Beneath a\n"
	"                           Steel Sky and Flight of the Amazon Queen\n"
#endif
	"  --copy-protection        Enable copy protection in SCUMM games, when\n"
	"                           ScummVM disables it by default.\n"
	"  --talkspeed=NUM          Set talk speed for games (default: 60)\n"
#ifndef DISABLE_SCUMM
	"  --demo-mode              Start demo mode of Maniac Mansion\n"
	"  --tempo=NUM              Set music tempo (in percent, 50-200) for SCUMM games\n"
	"                           (default: 100)\n"
#endif
	"\n"
	"The meaning of boolean long options can be inverted by prefixing them with\n"
	"\"no-\", e.g. \"--no-aspect-ratio\".\n"
;
#endif

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
static Common::String s_appName("scummvm");
#else
static const char *s_appName = "scummvm";
#endif

static void usage(const char *s, ...) GCC_PRINTF(1, 2);

static void usage(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	printf(USAGE_STRING, s_appName.c_str(), buf, s_appName.c_str(), s_appName.c_str());
#endif
	exit(1);
}

GameDetector::GameDetector() {

	// Graphics
	ConfMan.registerDefault("fullscreen", false);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.registerDefault("gfx_mode", "normal");
	ConfMan.registerDefault("render_mode", "default");
#if defined(__SYMBIAN32__)
	ConfMan.registerDefault("force_1x_overlay", true);
#else
	ConfMan.registerDefault("force_1x_overlay", false);
#endif


	// Sound & Music
	ConfMan.registerDefault("music_volume", 192);
	ConfMan.registerDefault("sfx_volume", 192);
	ConfMan.registerDefault("speech_volume", 192);

	ConfMan.registerDefault("multi_midi", false);
	ConfMan.registerDefault("native_mt32", false);
	ConfMan.registerDefault("enable_gs", false);
	ConfMan.registerDefault("midi_gain", 100);
//	ConfMan.registerDefault("music_driver", ???);

	ConfMan.registerDefault("cdrom", 0);

	// Game specific
	ConfMan.registerDefault("path", "");
	ConfMan.registerDefault("savepath", "");

//	ConfMan.registerDefault("amiga", false);
	ConfMan.registerDefault("platform", Common::kPlatformPC);
	ConfMan.registerDefault("language", "en");
	ConfMan.registerDefault("speech_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("subtitles", false);
	ConfMan.registerDefault("boot_param", 0);
	ConfMan.registerDefault("dump_scripts", false);
	ConfMan.registerDefault("save_slot", -1);
	ConfMan.registerDefault("autosave_period", 5 * 60);	// By default, trigger autosave every 5 minutes

#if !defined(DISABLE_SCUMM) || !defined(DISABLE_SWORD2)
	ConfMan.registerDefault("object_labels", true);
#endif

	ConfMan.registerDefault("copy_protection", false);
	ConfMan.registerDefault("talkspeed", 60);

#ifndef DISABLE_SCUMM
	ConfMan.registerDefault("demo_mode", false);
	ConfMan.registerDefault("tempo", 0);
#endif

#if !defined(DISABLE_SKY) || !defined(DISABLE_QUEEN)
	ConfMan.registerDefault("alt_intro", false);
#endif

	// Miscellaneous
	ConfMan.registerDefault("joystick_num", -1);
	ConfMan.registerDefault("confirm_exit", false);
	ConfMan.registerDefault("disable_sdl_parachute", false);
#ifdef USE_ALSA
	ConfMan.registerDefault("alsa_port", "65:0");
#endif

#ifdef DEFAULT_SAVE_PATH
	char savePath[MAXPATHLEN];
#ifdef UNIX
	struct stat sb;
	const char *home = getenv("HOME");
	if (home && *home && strlen(home) < MAXPATHLEN) {
		snprintf(savePath, MAXPATHLEN, "%s/%s", home, DEFAULT_SAVE_PATH);
		if (stat(savePath, &sb) == -1) {
			/* create the dir if it does not exist */
			if (errno == ENOENT) {
				if (mkdir(savePath, 0755) != 0) {
					perror("mkdir");
					exit(1);
				}
			}
		}
		/* check that the dir is there */
		if (stat(savePath, &sb) == 0) {
			ConfMan.registerDefault("savepath", savePath);
		}
	}
#elif defined(__SYMBIAN32__)
	strcpy(savePath, Symbian::GetExecutablePath());
	strcat(savePath, DEFAULT_SAVE_PATH);
	struct stat sb;
	if (stat(savePath, &sb) == -1)
		if (errno == ENOENT)// create the dir if it does not exist
			if (mkdir(savePath, 0755) != 0)
				error("mkdir for '%s' failed!", savePath);
	ConfMan.registerDefault("savepath", savePath); // this should be enough...
#endif
#endif // #ifdef DEFAULT_SAVE_PATH

	_plugin = 0;
}

GameDescriptor GameDetector::findGame(const String &gameName, const Plugin **plugin) {
	// Find the GameDescriptor for this target
	const PluginList &plugins = PluginManager::instance().getPlugins();
	GameDescriptor result;

	PluginList::const_iterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		result = (*iter)->findGame(gameName.c_str());
		if (result.gameid.size() > 0) {
			if (plugin)
				*plugin = *iter;
			break;
		}
	}
	return result;
}

//
// Various macros used by the command line parser.
//

// Use this for options which have an *optional* value
#define DO_OPTION_OPT(shortCmd, longCmd, defaultVal) \
	if (isLongCmd ? (!memcmp(s+2, longCmd"=", sizeof(longCmd"=") - 1)) : (tolower(s[1]) == shortCmd)) { \
		s += 2; \
		if (isLongCmd) \
			s += sizeof(longCmd"=") - 1; \
		const char *option = s; \
		if (*s == '\0') { option = s2; i++; } \
		if (!option) option = defaultVal; \
		if (option) settings[longCmd] = option;

// Use this for options which have a required (string) value
#define DO_OPTION(shortCmd, longCmd) \
	DO_OPTION_OPT(shortCmd, longCmd, 0) \
	if (!option) usage("Option '%s' requires an argument", argv[i]);

// Use this for options which have a required integer value
#define DO_OPTION_INT(shortCmd, longCmd) \
	DO_OPTION_OPT(shortCmd, longCmd, 0) \
	if (!option) usage("Option '%s' requires an argument", argv[i]); \
	char *endptr = 0; \
	int intValue; intValue = (int)strtol(option, &endptr, 10); \
	if (endptr == NULL || *endptr != 0) usage("--%s: Invalid number '%s'", longCmd, option);

// Use this for boolean options; this distinguishes between "-x" and "-X",
// resp. between "--some-option" and "--no-some-option".
#define DO_OPTION_BOOL(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s+2, longCmd) || !strcmp(s+2, "no-"longCmd)) : (tolower(s[1]) == shortCmd)) { \
		bool boolValue = islower(s[1]); \
		s += 2; \
		if (isLongCmd) { \
			boolValue = !strcmp(s, longCmd); \
			s += boolValue ? (sizeof(longCmd) - 1) : (sizeof("no-"longCmd) - 1); \
		} \
		if (*s != '\0') goto unknownOption; \
		const char *option = boolValue ? "true" : "false"; \
		settings[longCmd] = option;

// Use this for options which never have a value, i.e. for 'commands', like "--help".
#define DO_OPTION_CMD(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s+2, longCmd)) : (tolower(s[1]) == shortCmd)) { \
		s += 2; \
		if (isLongCmd) \
			s += sizeof(longCmd) - 1; \
		if (*s != '\0') goto unknownOption;


#define DO_LONG_OPTION_OPT(longCmd, d) 	DO_OPTION_OPT(0, longCmd, d)
#define DO_LONG_OPTION(longCmd) 		DO_OPTION(0, longCmd)
#define DO_LONG_OPTION_INT(longCmd) 	DO_OPTION_INT(0, longCmd)
#define DO_LONG_OPTION_BOOL(longCmd) 	DO_OPTION_BOOL(0, longCmd)
#define DO_LONG_OPTION_CMD(longCmd) 	DO_OPTION_CMD(0, longCmd)

// End an option handler
#define END_OPTION \
		continue; \
	}


Common::String GameDetector::parseCommandLine(Common::StringMap &settings, int argc, char **argv) {
	const char *s, *s2;
	
	// argv[0] contains the name of the executable.
	if (argv && argv[0]) {
		s = strchr(argv[0], '/');
		s_appName = s ? s : argv[0];
	}

	// We store all command line settings into a string map.

	// Iterate over all command line arguments and parse them into our string map.
	for (int i = 1; i < argc; ++i) {
		s = argv[i];
		s2 = (i < argc-1) ? argv[i+1] : 0;

		if (s[0] != '-') {
			// The argument doesn't start with a dash, so it's not an option.
			// Hence it must be the target name. We currently enforce that
			// this always comes last.
			if (i != argc - 1)
				usage("Stray argument '%s'", s);

			// We defer checking whether this is a valid target to a later point.
			return s;
		} else {

			bool isLongCmd = (s[0] == '-' && s[1] == '-');

			DO_OPTION_CMD('h', "help")
				printf(HELP_STRING, argv[0]);
				exit(0);
			END_OPTION

			DO_OPTION_CMD('v', "version")
				printf("%s\n", gScummVMFullVersion);
				printf("Features compiled in: %s\n", gScummVMFeatures);
				exit(0);
			END_OPTION

			DO_OPTION_CMD('t', "list-targets")
				return "list-targets";
			END_OPTION

			DO_OPTION_CMD('z', "list-games")
				return "list-games";
			END_OPTION



			DO_OPTION('c', "config")
			END_OPTION

			DO_OPTION_INT('b', "boot-param")
			END_OPTION

			DO_OPTION_OPT('d', "debuglevel", "0")
			END_OPTION

			DO_LONG_OPTION("debugflags")
			END_OPTION

			DO_OPTION('e', "music-driver")
				if (MidiDriver::parseMusicDriver(option) < 0)
					usage("Unrecognized music driver '%s'", option);
			END_OPTION

			DO_LONG_OPTION_INT("output-rate")
			END_OPTION

			DO_OPTION_BOOL('f', "fullscreen")
			END_OPTION

			DO_OPTION('g', "gfx-mode")
				// Check whether 'option' specifies a valid graphics mode.
				bool isValid = false;
				if (!scumm_stricmp(option, "normal") || !scumm_stricmp(option, "default"))
					isValid = true;
				if (!isValid) {
					const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
					while (gm->name && !isValid) {
						isValid = !scumm_stricmp(gm->name, option);
						gm++;
					}
				}
				if (!isValid)
					usage("Unrecognized graphics mode '%s'", option);
			END_OPTION

			DO_OPTION_INT('m', "music-volume")
			END_OPTION

			DO_OPTION_BOOL('n', "subtitles")
			END_OPTION

			DO_OPTION('p', "path")
				// TODO: Verify whether the path is valid
			END_OPTION

			DO_OPTION('q', "language")
				if (Common::parseLanguage(option) == Common::UNK_LANG)
					usage("Unrecognized language '%s'", option);
			END_OPTION

			DO_OPTION_INT('s', "sfx-volume")
			END_OPTION

			DO_OPTION_INT('r', "speech-volume")
			END_OPTION

			DO_LONG_OPTION_INT("midi-gain")
			END_OPTION

			DO_OPTION_BOOL('u', "dump-scripts")
			END_OPTION

			DO_OPTION_OPT('x', "save-slot", "0")
			END_OPTION

			DO_LONG_OPTION_INT("cdrom")
			END_OPTION

			DO_LONG_OPTION_OPT("joystick", "0")
			END_OPTION

			DO_LONG_OPTION("platform")
				int platform = Common::parsePlatform(option);
				if (platform == Common::kPlatformUnknown)
					usage("Unrecognized platform '%s'", option);
			END_OPTION

			DO_LONG_OPTION("soundfont")
				// TODO: Verify whether the path is valid
			END_OPTION

			DO_LONG_OPTION_BOOL("disable-sdl-parachute")
			END_OPTION

			DO_LONG_OPTION_BOOL("multi-midi")
			END_OPTION

			DO_LONG_OPTION_BOOL("native-mt32")
			END_OPTION

			DO_LONG_OPTION_BOOL("enable-gs")
			END_OPTION

			DO_LONG_OPTION_BOOL("aspect-ratio")
			END_OPTION

			DO_LONG_OPTION("render-mode")
				int renderMode = Common::parseRenderMode(option);
				if (renderMode == Common::kRenderDefault)
					usage("Unrecognized render mode '%s'", option);
			END_OPTION

			DO_LONG_OPTION_BOOL("force-1x-overlay")
			END_OPTION

			DO_LONG_OPTION("savepath")
				// TODO: Verify whether the path is valid
			END_OPTION

			DO_LONG_OPTION_INT("talkspeed")
			END_OPTION

			DO_LONG_OPTION_BOOL("copy-protection")
			END_OPTION

			DO_LONG_OPTION("gui-theme")
			END_OPTION

			DO_LONG_OPTION("target-md5")
			END_OPTION

#ifndef DISABLE_SCUMM
			DO_LONG_OPTION("tempo")
				// Use the special value '0' for the base in (int)strtol.
				// Doing that makes it possible to enter hex values
				// as "0x1234", but also decimal values ("123").
				int value = (int)strtol(option, 0, 0);
				char buf[20];
				snprintf(buf, sizeof(buf), "%d", value);
				settings["tempo"] = buf;
			END_OPTION

			DO_LONG_OPTION_BOOL("demo-mode")
			END_OPTION
#endif

#if !defined(DISABLE_SKY) || !defined(DISABLE_QUEEN)
			DO_LONG_OPTION_BOOL("alt-intro")
			END_OPTION
#endif

unknownOption:
			// If we get till here, the option is unhandled and hence unknown.
			usage("Unrecognized option '%s'", argv[i]);
		}
	}


	return Common::String::emptyString;
}


void GameDetector::processSettings(Common::String &target, Common::StringMap &settings) {

	// If a target was specified, check whether there is either a game
	// domain (i.e. a target) matching this argument, or alternatively
	// whether there is a gameid matching that name.
	if (!target.empty()) {
		if (ConfMan.hasGameDomain(target) || findGame(target).gameid.size() > 0) {
			setTarget(target);
		} else {
			usage("Unrecognized game target '%s'", target.c_str());
		}
	}
	

	// The user can override the savepath with the SCUMMVM_SAVEPATH
	// environment variable. This is weaker than a --savepath on the
	// command line, but overrides the default savepath, hence it is
	// handled here, just before the command line gets parsed.
#if !defined(MACOS_CARBON) && !defined(_WIN32_WCE) && !defined(PALMOS_MODE) && !defined(__GP32__)
	if (!settings.contains("savepath")) {
		const char *dir = getenv("SCUMMVM_SAVEPATH");
		if (dir && *dir && strlen(dir) < MAXPATHLEN) {
			// TODO: Verify whether the path is valid
			settings["savepath"] = dir;
		}
	}
#endif


	// Finally, store the command line settings into the config manager.
	for (Common::StringMap::const_iterator x = settings.begin(); x != settings.end(); ++x) {
		String key(x->_key);
		String value(x->_value);

		// Replace any "-" in the key by "_" (e.g. change "save-slot" to "save_slot").
		for (String::iterator c = key.begin(); c != key.end(); ++c)
			if (*c == '-')
				*c = '_';
		
		// Store it into ConfMan.
		ConfMan.set(key, value, Common::ConfigManager::kTransientDomain);
	}
}


void GameDetector::setTarget(const String &target) {
	_targetName = target;
	ConfMan.setActiveDomain(target);

	if (ConfMan.hasKey("gameid"))
		_gameid = ConfMan.get("gameid");
	else
		_gameid = _targetName;
	
	// TODO: In the future, simply insert the gameid into the transient domain.
	// That way, all code (including backends) can reliably access it.
	//ConfMan.set("gameid", _gameid, Common::ConfigManager::kTransientDomain);
}

bool GameDetector::detectMain() {
	if (_targetName.empty()) {
		warning("No game was specified...");
		return false;
	}

	printf("Looking for %s\n", _gameid.c_str());
	GameDescriptor game = findGame(_gameid, &_plugin);

	if (game.gameid.size() == 0) {
		printf("Failed game detection\n");
		warning("%s is an invalid target. Use the --list-targets option to list targets", _targetName.c_str());
		return false;
	}

	printf("Trying to start game '%s'\n", game.description.c_str());

	String gameDataPath(ConfMan.get("path"));
	if (gameDataPath.empty()) {
		warning("No path was provided. Assuming the data files are in the current directory");
		gameDataPath = "./";
	} else if (gameDataPath.lastChar() != '/'
#if defined(__MORPHOS__) || defined(__amigaos4__)
					&& gameDataPath.lastChar() != ':'
#endif
					&& gameDataPath.lastChar() != '\\') {
		gameDataPath += '/';
		ConfMan.set("path", gameDataPath, Common::ConfigManager::kTransientDomain);
	}

	return true;
}

Engine *GameDetector::createEngine(OSystem *sys) {
	assert(_plugin);
	return _plugin->createInstance(this, sys);
}

Audio::Mixer *GameDetector::createMixer() {
	return new Audio::Mixer();
}
