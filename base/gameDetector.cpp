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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

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

// DONT FIXME: DO NOT ORDER ALPHABETICALLY, THIS IS ORDERED BY IMPORTANCE/CATEGORY! :)
#if defined(PALMOS_MODE) || defined(__SYMBIAN32__)
static const char USAGE_STRING[] = "NoUsageString"; // save more data segment space
#else
static const char USAGE_STRING[] =
	"ScummVM - Graphical Adventure Game Interpreter\n"
	"Usage: scummvm [OPTIONS]... [GAME]\n"
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
	"  -u, --dump-scripts       Enable script dumping if a directory called 'dumps'\n"
	"                           exists in the current directory\n"
	"\n"
	"  --cdrom=NUM              CD drive to play CD audio from (default: 0 = first\n"
	"                           drive)\n"
	"  --joystick[=NUM]         Enable input with joystick (default: 0 = first\n"
	"                           joystick)\n"
	"  --platform=WORD          Specify version of game (allowed values: acorn,\n"
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


GameDetector::GameDetector() {

	// Graphics
	ConfMan.registerDefault("fullscreen", false);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.registerDefault("gfx_mode", "normal");
	ConfMan.registerDefault("render_mode", "default");

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
	ConfMan.registerDefault("save_slot", -1);

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
	if (home != NULL && strlen(home) < MAXPATHLEN) {
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

	_dumpScripts = false;

#if defined(__SYMBIAN32__)
	_force1xOverlay = true;
#else
	_force1xOverlay = false;
#endif

	memset(&_game, 0, sizeof(_game));
	_plugin = 0;
}

/** List all supported game IDs, i.e. all games which any loaded plugin supports. */
void listGames() {
	const PluginList &plugins = PluginManager::instance().getPlugins();

	printf("Game ID              Full Title                                            \n"
	       "-------------------- ------------------------------------------------------\n");

	PluginList::const_iterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		GameList list = (*iter)->getSupportedGames();
		for (GameList::iterator v = list.begin(); v != list.end(); ++v) {
			printf("%-20s %s\n", v->gameid, v->description);
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

		if (description.isEmpty()) {
			// FIXME: At this point, we should check for a "gameid" override
			// to find the proper desc. In fact, the platform probably should
			// be take into consideration, too.
			String gameid(name);
			GameSettings g = GameDetector::findGame(gameid);
			if (g.description)
				description = g.description;
		}

		printf("%-20s %s\n", name.c_str(), description.c_str());
	}
}

GameSettings GameDetector::findGame(const String &gameName, const Plugin **plugin) {
	// Find the GameSettings for this target
	const PluginList &plugins = PluginManager::instance().getPlugins();
	GameSettings result = {NULL, NULL, 0};

	PluginList::const_iterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		result = (*iter)->findGame(gameName.c_str());
		if (result.gameid) {
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
#define DO_OPTION_OPT(shortCmd, longCmd) \
	if (isLongCmd ? (!memcmp(s, longCmd"=", sizeof(longCmd"=") - 1)) : (shortCmdLower == shortCmd)) { \
		if (isLongCmd) \
			s += sizeof(longCmd"=") - 1; \
		if ((*s != '\0') && (current_option != NULL)) goto ShowHelpAndExit; \
		char *option = (*s != '\0') ? s : current_option; \
		current_option = NULL;

// Use this for options which have a required (string) value
#define DO_OPTION(shortCmd, longCmd) \
	DO_OPTION_OPT(shortCmd, longCmd) \
	if (option == NULL) goto ShowHelpAndExit;

// Use this for options which have a required integer value
#define DO_OPTION_INT(shortCmd, longCmd) \
	DO_OPTION_OPT(shortCmd, longCmd) \
	if (option == NULL) goto ShowHelpAndExit; \
	char *endptr = 0; \
	int intValue; intValue = (int)strtol(option, &endptr, 10); \
	if (endptr == NULL || *endptr != 0) goto ShowHelpAndExit;

// Use this for boolean options; this distinguishes between "-x" and "-X",
// resp. between "--some-option" and "--no-some-option".
#define DO_OPTION_BOOL(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s, longCmd) || !strcmp(s, "no-"longCmd)) : (shortCmdLower == shortCmd)) { \
		if (isLongCmd) { \
			boolValue = !strcmp(s, longCmd); \
			s += boolValue ? (sizeof(longCmd) - 1) : (sizeof("no-"longCmd) - 1); \
		} \
		if ((*s != '\0') || (current_option != NULL)) goto ShowHelpAndExit;

// Use this for options which never have a value, i.e. for 'commands', like "--help".
#define DO_OPTION_CMD(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s, longCmd)) : (shortCmdLower == shortCmd)) { \
		if (isLongCmd) \
			s += sizeof(longCmd) - 1; \
		if ((*s != '\0') || (current_option != NULL)) goto ShowHelpAndExit;


#define DO_LONG_OPTION_OPT(longCmd) 	DO_OPTION_OPT(0, longCmd)
#define DO_LONG_OPTION(longCmd) 		DO_OPTION(0, longCmd)
#define DO_LONG_OPTION_INT(longCmd) 	DO_OPTION_INT(0, longCmd)
#define DO_LONG_OPTION_BOOL(longCmd) 	DO_OPTION_BOOL(0, longCmd)
#define DO_LONG_OPTION_CMD(longCmd) 	DO_OPTION_CMD(0, longCmd)

// End an option handler
#define END_OPTION \
		continue; \
	}


void GameDetector::parseCommandLine(int argc, char **argv) {
	int i;
	char *s;
	char *current_option = NULL;
	char shortCmdLower;
	bool isLongCmd, boolValue;

	// We store all command line settings in a string map, instead of
	// immediately putting it into the config manager. We do that to
	// make a potential future change to the config manager easier: In
	// particular, right now there is only one transient config domain
	// domain, in the future there might be two (one for the app, one
	// for the active game). Since we only know after all params have
	// been parsed whether a game is going to be started or whether we
	// run the launcher, we need to delay putting things into the config
	// manager until after parsing is complete.
	Common::StringMap settings;


	// The user can override the savepath with the SCUMMVM_SAVEPATH
	// environment variable. This is weaker than a --savepath on the
	// command line, but overrides the default savepath, hence it is
	// handled here, just before the command line gets parsed.
#if !defined(MACOS_CARBON) && !defined(_WIN32_WCE) && !defined(PALMOS_MODE)
	const char *dir = getenv("SCUMMVM_SAVEPATH");
	if (dir && *dir && strlen(dir) < 1024) {
		// TODO: Verify whether the path is valid
		settings["savepath"] = dir;
	}
#endif

	// Iterate over all command line arguments, backwards.
	for (i = argc - 1; i >= 1; i--) {
		s = argv[i];

		if (s[0] != '-' || s[1] == '\0') {
			// Last argument: this could be a target name.
			// To verify this, check if there is either a game domain (i.e.
			// a configured target) matching this argument, or if we can
			// find any target with that name.
			if (i == (argc - 1) && (ConfMan.hasGameDomain(s) || findGame(s).gameid)) {
				setTarget(s);
			} else {
				if (current_option == NULL)
					current_option = s;
				else
					goto ShowHelpAndExit;
			}
		} else {

			shortCmdLower = tolower(s[1]);
			isLongCmd = (s[0] == '-' && s[1] == '-');
			boolValue = (shortCmdLower == s[1]);
			s += 2;

			DO_OPTION('c', "config")
				// Dummy
			END_OPTION

			DO_OPTION_INT('b', "boot-param")
				settings["boot_param"] = option;
			END_OPTION

			DO_OPTION_OPT('d', "debuglevel")
				gDebugLevel = option ? (int)strtol(option, 0, 10) : 0;
				printf("Debuglevel (from command line): %d\n", gDebugLevel);
			END_OPTION

			DO_OPTION('e', "music-driver")
				// TODO: Instead of just showing the generic help text,
				// maybe print a message like:
				// "'option' is not a supported music driver on this machine.
				//  Available driver: ..."
				if (MidiDriver::parseMusicDriver(option) < 0)
					goto ShowHelpAndExit;
				settings["music_driver"] = option;
			END_OPTION

			DO_LONG_OPTION_INT("output-rate")
				settings["output_rate"] = option;
			END_OPTION

			DO_OPTION_BOOL('f', "fullscreen")
				settings["fullscreen"] = boolValue ? "true" : "false";
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
				// TODO: Instead of just showing the generic help text,
				// maybe print a message like:
				// "'option' is not a supported graphic mode on this machine.
				//  Available graphic modes: ..."
				if (!isValid)
					goto ShowHelpAndExit;
				settings["gfx_mode"] = option;
			END_OPTION

			DO_OPTION_CMD('h', "help")
				printf(USAGE_STRING);
				exit(0);
			END_OPTION

			DO_OPTION_INT('m', "music-volume")
				settings["music_volume"] = option;
			END_OPTION

			DO_OPTION_BOOL('n', "subtitles")
				settings["subtitles"] =  boolValue ? "true" : "false";
			END_OPTION

			DO_OPTION('p', "path")
				// TODO: Verify whether the path is valid
				settings["path"] = option;
			END_OPTION

			DO_OPTION('q', "language")
				if (Common::parseLanguage(option) == Common::UNK_LANG)
					goto ShowHelpAndExit;
				settings["language"] = option;
			END_OPTION

			DO_OPTION_INT('s', "sfx-volume")
				settings["sfx_volume"] = option;
			END_OPTION

			DO_OPTION_INT('r', "speech-volume")
				settings["speech_volume"] = option;
			END_OPTION

			DO_LONG_OPTION_INT("midi-gain")
				settings["midi_gain"] = option;
			END_OPTION

			DO_OPTION_CMD('t', "list-targets")
				listTargets();
				exit(0);
			END_OPTION

			DO_OPTION_BOOL('u', "dump-scripts")
				_dumpScripts = true;
			END_OPTION

			DO_OPTION_CMD('v', "version")
				printf("%s\n", gScummVMFullVersion);
				printf("Features compiled in: %s\n", gScummVMFeatures);
				exit(0);
			END_OPTION

			DO_OPTION_OPT('x', "save-slot")
				settings["save_slot"] = (option != NULL) ? option : "0";
			END_OPTION

			DO_OPTION_CMD('z', "list-games")
				listGames();
				exit(0);
			END_OPTION

			DO_LONG_OPTION_INT("cdrom")
				settings["cdrom"] = option;
			END_OPTION

			DO_LONG_OPTION_OPT("joystick")
				settings["joystick_num"] = (option != NULL) ? option : "0";
			END_OPTION

			DO_LONG_OPTION("platform")
				int platform = Common::parsePlatform(option);
				if (platform == Common::kPlatformUnknown)
					goto ShowHelpAndExit;

				settings["platform"] = option;
			END_OPTION

			DO_LONG_OPTION("soundfont")
				// TODO: Verify whether the path is valid
				settings["soundfont"] = option;
			END_OPTION

			DO_LONG_OPTION_BOOL("disable-sdl-parachute")
				settings["disable_sdl_parachute"] = boolValue ? "true" : "false";
			END_OPTION

			DO_LONG_OPTION_BOOL("multi-midi")
				settings["multi_midi"] = boolValue ? "true" : "false";
			END_OPTION

			DO_LONG_OPTION_BOOL("native-mt32")
				settings["native_mt32"] = boolValue ? "true" : "false";
			END_OPTION

			DO_LONG_OPTION_BOOL("enable-gs")
				settings["enable_gs"] = boolValue ? "true" : "false";
			END_OPTION

			DO_LONG_OPTION_BOOL("aspect-ratio")
				settings["aspect_ratio"] = boolValue ? "true" : "false";
			END_OPTION

			DO_LONG_OPTION("render-mode")
				int renderMode = Common::parseRenderMode(option);
				if (renderMode == Common::kRenderDefault)
					goto ShowHelpAndExit;

				settings["render_mode"] = option;
			END_OPTION

			DO_LONG_OPTION_BOOL("force-1x-overlay")
				_force1xOverlay = true;
			END_OPTION

			DO_LONG_OPTION("savepath")
				// TODO: Verify whether the path is valid
				settings["savepath"] = option;
			END_OPTION

			DO_LONG_OPTION_INT("talkspeed")
				settings["talkspeed"] = option;
			END_OPTION

			DO_LONG_OPTION_BOOL("copy-protection")
				settings["copy_protection"] = boolValue ? "true" : "false";
			END_OPTION

			DO_LONG_OPTION("gui-theme")
				settings["gui_theme"] = option;
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
				settings["demo_mode"] = boolValue ? "true" : "false";
			END_OPTION
#endif

#if !defined(DISABLE_SKY) || !defined(DISABLE_QUEEN)
			DO_LONG_OPTION_BOOL("alt-intro")
				settings["alt_intro"] = boolValue ? "true" : "false";
			END_OPTION
#endif

			// If we get till here, the option is unhandled and hence unknown.
			goto ShowHelpAndExit;
		}
	}

	if (current_option) {
ShowHelpAndExit:
		printf(USAGE_STRING);
		exit(1);
	}

	// Finally, store the command line settings into the config manager.
	for (Common::StringMap::const_iterator x = settings.begin(); x != settings.end(); ++x)
		ConfMan.set(x->_key, x->_value, Common::ConfigManager::kTransientDomain);

}

void GameDetector::setTarget(const String &target) {
	_targetName = target;
	ConfMan.setActiveDomain(target);
}

bool GameDetector::detectGame() {
	String realGame;

	if (ConfMan.hasKey("gameid"))
		realGame = ConfMan.get("gameid");
	else
		realGame = _targetName;

	printf("Looking for %s\n", realGame.c_str());
	_game = findGame(realGame, &_plugin);

	if (_game.gameid) {
		printf("Trying to start game '%s'\n", _game.description);
		return true;
	} else {
		printf("Failed game detection\n");
		return false;
	}
}

bool GameDetector::detectMain() {
	if (_targetName.isEmpty()) {
		warning("No game was specified...");
		return false;
	}

	if (!detectGame()) {
		warning("%s is an invalid target. Use the --list-targets option to list targets", _targetName.c_str());
		return false;
	}

	String gameDataPath(ConfMan.get("path"));
	if (gameDataPath.isEmpty()) {
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
