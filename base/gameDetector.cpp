/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "backends/intern.h"
#include "base/engine.h"
#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/scaler.h"	// Only for gfx_modes

#include "sound/mididrv.h"
#include "sound/mixer.h"

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

// DONT FIXME: DO NOT ORDER ALPHABETICALY, THIS IS ORDERED BY IMPORTANCE/CATEGORY! :)
#ifdef __PALM_OS__
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
	"  -p, --path=PATH          Path to where the game is installed\n"
	"  -x, --save-slot[=NUM]    Save game slot to load (default: autosave)\n"
	"  -f, --fullscreen         Force full-screen mode\n"
	"  -F, --no-fullscreen      Force windowed mode\n"
	"  -g, --gfx-mode=MODE      Select graphics scaler (normal,2x,3x,2xsai,\n"
	"                           super2xsai,supereagle,advmame2x,advmame3x,hq2x,\n"
	"                           hq3x,tv2x,dotmatrix)\n"
	"  -e, --music-driver=MODE  Select music driver (see README for details)\n"
	"  -q, --language=LANG      Select language (en,de,fr,it,pt,es,jp,zh,kr,se,gb,\n"
	"                           hb,ru)\n"
	"  -m, --music-volume=NUM   Set the music volume, 0-255 (default: 192)\n"
	"  -o, --master-volume=NUM  Set the master volume, 0-255 (default: 192)\n"
	"  -s, --sfx-volume=NUM     Set the sfx volume, 0-255 (default: 192)\n"
	"  -n, --nosubtitles        Disable subtitles (use with games that have voice)\n"
	"  -b, --boot-param=NUM     Pass number to the boot script (boot param)\n"
	"  -d, --debuglevel=NUM     Set debug verbosity level\n"
	"  -u, --dump-scripts       Enable script dumping if a directory called 'dumps'\n"
	"                           exists in the current directory\n"
	"\n"
	"  --cdrom=NUM              CD drive to play CD audio from (default: 0 = first\n"
	"                           drive)\n"
	"  --joytick[=NUM]          Enable input with joystick (default: 0 = first\n"
	"                           joystick)\n"
	"  --platform=WORD          Specify version of game (allowed values: amiga,\n"
	"                           atari, mac, pc)\n"
	"  --multi-midi             Enable combination Adlib and native MIDI\n"
	"  --native-mt32            True Roland MT-32 (disable GM emulation)\n"
	"  --aspect-ratio           Enable aspect ratio correction\n"
	"\n"
#ifndef DISABLE_SKY
	"  --floppy-intro           Use floppy version intro for Beneath a Steel Sky CD\n"
#endif
#ifndef DISABLE_SCUMM
	"  --demo-mode              Start demo mode of Maniac Mansion (Classic version)\n"
	"  --tempo=NUM              Set music tempo (in percent, 50-200) for SCUMM games\n"
	"                           (default: 100)\n"
	"  --talkspeed=NUM          Set talk speed for SCUMM games (default: 60)\n"
#endif
	"\n"
	"The meaning of boolean long options can be inverted by prefixing them with\n"
	"\"no-\", e.g. \"--no-aspect-ratio\".\n"
;
#endif

struct GraphicsMode {
	const char *name;
	const char *description;
	int id;
};

/**
 * List of graphic 'modes' we potentially support. Potentially because not all
 * backends actually support all the filters listed here. At this point only
 * the SDL backend supports all (except for the PalmOS ones of course).
 * @todo Remove this explicit list of graphic modes and rather extend the 
 * OSystem API to allow querying a backend for the modes it supports.
 */
static const struct GraphicsMode gfx_modes[] = {
	{"normal", "Normal (no scaling)", GFX_NORMAL},
	{"1x", "Normal (no scaling)", GFX_NORMAL},
#ifndef __PALM_OS__	// reduce contant data size
	{"2x", "2x", GFX_DOUBLESIZE},
	{"3x", "3x", GFX_TRIPLESIZE},
	{"2xsai", "2xSAI", GFX_2XSAI},
	{"super2xsai", "Super2xSAI", GFX_SUPER2XSAI},
	{"supereagle", "SuperEagle", GFX_SUPEREAGLE},
	{"advmame2x", "AdvMAME2x", GFX_ADVMAME2X},
	{"advmame3x", "AdvMAME3x", GFX_ADVMAME3X},
	{"hq2x", "HQ2x", GFX_HQ2X},
	{"hq3x", "HQ3x", GFX_HQ3X},
	{"tv2x", "TV2x", GFX_TV2X},
	{"dotmatrix", "DotMatrix", GFX_DOTMATRIX},
	{"opengl", "OpenGL", GFX_BILINEAR},
#else
	{"flipping", "Page Flipping", GFX_FLIPPING},
	{"dbuffer", "Double Buffer", GFX_DOUBLEBUFFER},
	{"wide", "Wide (HiRes+ only)", GFX_WIDE},
#endif
	{0, 0, 0}
};

GameDetector::GameDetector() {

	// Graphics
	ConfMan.registerDefault("fullscreen", false);
	ConfMan.registerDefault("aspect_ratio", false);
#ifndef _WIN32_WCE
	ConfMan.registerDefault("gfx_mode", "2x");
#else
	ConfMan.registerDefault("gfx_mode", "normal");
#endif

	// Sound & Music
	ConfMan.registerDefault("master_volume", 192);
	ConfMan.registerDefault("music_volume", 192);
	ConfMan.registerDefault("sfx_volume", 192);

	ConfMan.registerDefault("multi_midi", false);
	ConfMan.registerDefault("native_mt32", false);
//	ConfMan.registerDefault("music_driver", ???);

	ConfMan.registerDefault("cdrom", 0);

	// Game specifc
	ConfMan.registerDefault("path", "");

//	ConfMan.registerDefault("amiga", false);
	ConfMan.registerDefault("platform", Common::kPlatformPC);
	ConfMan.registerDefault("language", "en");
	ConfMan.registerDefault("nosubtitles", false);
	ConfMan.registerDefault("boot_param", 0);
	ConfMan.registerDefault("save_slot", -1);

#ifndef DISABLE_SCUMM
	ConfMan.registerDefault("demo_mode", false);
	ConfMan.registerDefault("talkspeed", 60);
	ConfMan.registerDefault("tempo", 0);
#endif

#ifndef DISABLE_SKY
	ConfMan.registerDefault("floppy_intro", false);
#endif

	// Miscellaneous
	ConfMan.registerDefault("debuglevel", 0);
	ConfMan.registerDefault("joystick_num", -1);
	ConfMan.registerDefault("confirm_exit", false);

	_dumpScripts = false;

	memset(&_game, 0, sizeof(_game));
	_plugin = 0;
}

/** List all supported games, i.e. all games which any loaded plugin supports. */
void listGames() {
	const PluginList &plugins = PluginManager::instance().getPlugins();

	printf("Game ID              Full Title                                            \n"
	       "-------------------- ------------------------------------------------------\n");

	PluginList::ConstIterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		GameList list = (*iter)->getSupportedGames();
		for (GameList::Iterator v = list.begin(); v != list.end(); ++v) {
			printf("%-20s %s\n", v->gameName, v->description);
		}
	}
}

/** List all targets which are configured in the config file. */
void listTargets() {
	using namespace Common;
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();

	printf("Target               Description                                           \n"
	       "-------------------- ------------------------------------------------------\n");

	ConfigManager::DomainMap::ConstIterator iter = domains.begin();
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		String name(iter->_key);
		String description(iter->_value.get("description"));

		if (description.isEmpty()) {
			GameSettings g = GameDetector::findGame(name);
			if (g.description)
				description = g.description;
		}

		printf("%-20s %s\n", name.c_str(), description.c_str());
	}
}

GameSettings GameDetector::findGame(const String &gameName, const Plugin **plugin) {
	// Find the GameSettings for this target
	const PluginList &plugins = PluginManager::instance().getPlugins();
	GameSettings result = {NULL, NULL, MDT_NONE, 0, NULL};
	
	PluginList::ConstIterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		result = (*iter)->findGame(gameName.c_str());
		if (result.gameName) {
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

#define DO_OPTION_OPT(shortCmd, longCmd) \
	if (isLongCmd ? (!memcmp(s, longCmd"=", sizeof(longCmd"=") - 1)) : (shortCmdLower == shortCmd)) { \
		if (isLongCmd) \
			s += sizeof(longCmd"=") - 1; \
		if ((*s != '\0') && (current_option != NULL)) goto ShowHelpAndExit; \
		option = (*s != '\0') ? s : current_option; \
		current_option = NULL;

#define DO_OPTION(shortCmd, longCmd) \
	DO_OPTION_OPT(shortCmd, longCmd) \
	if (option == NULL) goto ShowHelpAndExit;

#define DO_OPTION_BOOL(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s, longCmd) || !strcmp(s, "no-"longCmd)) : (shortCmdLower == shortCmd)) { \
		if (isLongCmd) { \
			cmdValue = !strcmp(s, longCmd); \
			s += cmdValue ? (sizeof(longCmd) - 1) : (sizeof("no-"longCmd) - 1); \
		} \
		if ((*s != '\0') || (current_option != NULL)) goto ShowHelpAndExit;

#define DO_OPTION_CMD(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s, longCmd)) : (shortCmdLower == shortCmd)) { \
		if (isLongCmd) \
			s += sizeof(longCmd) - 1; \
		if ((*s != '\0') || (current_option != NULL)) goto ShowHelpAndExit;


#define DO_LONG_OPTION_OPT(longCmd) 	DO_OPTION_OPT(0, longCmd)
#define DO_LONG_OPTION(longCmd) 		DO_OPTION(0, longCmd)
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
	char *option = NULL;
	char shortCmdLower;
	bool isLongCmd, cmdValue;

	// Iterate over all command line arguments, backwards.
	for (i = argc - 1; i >= 1; i--) {
		s = argv[i];
		
		if (s[0] != '-' || s[1] == '\0') {
			// Last argument: this could be a target name.
			// To verify this, check if there is either a game domain (i.e
			// a configured target) matching this argument, or if we can
			// find any target with that name.
			if (i == (argc - 1) && (ConfMan.hasGameDomain(s) || findGame(s).gameName)) {
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
			cmdValue = (shortCmdLower == s[1]);
			s += 2;
	
			DO_OPTION('b', "boot-param")
				ConfMan.set("boot_param", (int)strtol(option, 0, 10));
			END_OPTION
			
			DO_OPTION_OPT('d', "debuglevel")
				if (option != NULL)
					ConfMan.set("debuglevel", (int)strtol(option, 0, 10));
				if (ConfMan.getInt("debuglevel"))
					printf("Debuglevel (from command line): %d\n", ConfMan.getInt("debuglevel"));
				else
					printf("Debuglevel (from command line): 0 - Game only\n");
			END_OPTION
			
			DO_OPTION('e', "music-driver")
				// TODO: Instead of just showing the generic help text,
				// maybe print a message like:
				// "'option' is not a supported music driver on this machine.
				//  Available driver: ..."
				if (parseMusicDriver(option) < 0)
					goto ShowHelpAndExit;
				ConfMan.set("music_driver", option);
			END_OPTION
			
			DO_OPTION_BOOL('f', "fullscreen")
				ConfMan.set("fullscreen", cmdValue);
			END_OPTION
			
			DO_OPTION('g', "gfx-mode")
				int gfx_mode = parseGraphicsMode(option);
				// TODO: Instead of just showing the generic help text,
				// maybe print a message like:
				// "'option' is not a supported graphic mode on this machine.
				//  Available graphic modes: ..."
				if (gfx_mode == -1)
					goto ShowHelpAndExit;
				ConfMan.set("gfx_mode", option);
			END_OPTION
	
			DO_OPTION_CMD('h', "help")
				printf(USAGE_STRING);
				exit(0);
			END_OPTION
			
			DO_OPTION('m', "music-volume")
				ConfMan.set("music_volume", (int)strtol(option, 0, 10));
			END_OPTION
			
			DO_OPTION_BOOL('n', "nosubtitles")
				ConfMan.set("nosubtitles", cmdValue);
			END_OPTION
			
			DO_OPTION('o', "master-volume")
				ConfMan.set("master_volume", (int)strtol(option, 0, 10));
			END_OPTION
			
			DO_OPTION('p', "path")
				// TODO: Verify whether the path is valid
				ConfMan.set("path", option);
			END_OPTION
			
			DO_OPTION('q', "language")
				if (Common::parseLanguage(option) == Common::UNK_LANG)
					goto ShowHelpAndExit;
				ConfMan.set("language", option);
			END_OPTION
			
			DO_OPTION('s', "sfx-volume")
				ConfMan.set("sfx_volume", (int)strtol(option, 0, 10));
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
				exit(0);
			END_OPTION
			
			DO_OPTION('x', "save-slot")
				ConfMan.set("save_slot", (option != NULL) ? (int)strtol(option, 0, 10) : 0);
			END_OPTION
			
			DO_OPTION_CMD('z', "list-games")
				listGames();
				exit(0);
			END_OPTION
			
			DO_LONG_OPTION("cdrom")
				ConfMan.set("cdrom", (int)strtol(option, 0, 10));
			END_OPTION
	
			DO_LONG_OPTION_OPT("joystick")
				ConfMan.set("joystick_num", (option != NULL) ? (int)strtol(option, 0, 10) : 0);
			END_OPTION
			
			DO_LONG_OPTION("platform")
				int platform = Common::parsePlatform(option);
				if (platform == Common::kPlatformUnknown)
					goto ShowHelpAndExit;
	
				ConfMan.set("platform", platform);
			END_OPTION
	
			DO_LONG_OPTION_BOOL("multi-midi")
				ConfMan.set("multi_midi", cmdValue);
			END_OPTION
	
			DO_LONG_OPTION_BOOL("native-mt32")
				ConfMan.set("native_mt32", cmdValue);
			END_OPTION
	
			DO_LONG_OPTION_BOOL("aspect-ratio")
				ConfMan.set("aspect_ratio", cmdValue);
			END_OPTION
	
#ifndef DISABLE_SCUMM
			DO_LONG_OPTION("tempo")
				// Use the special value '0' for the base in (int)strtol. 
				// Doing that makes it possible to enter hex values
				// as "0x1234", but also decimal values ("123").
				ConfMan.set("tempo", (int)strtol(option, 0, 0));
			END_OPTION
			
			DO_LONG_OPTION("talkspeed")
				ConfMan.set("talkspeed", (int)strtol(option, 0, 10));
			END_OPTION

			DO_LONG_OPTION_BOOL("demo-mode")
				ConfMan.set("demo_mode", cmdValue);
			END_OPTION
#endif
	
#ifndef DISABLE_SKY
			DO_LONG_OPTION_BOOL("floppy-intro")
				ConfMan.set("floppy_intro", cmdValue);
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
}

void GameDetector::setTarget(const String &name) {
	_targetName = name;
	ConfMan.setActiveDomain(name);
}

int GameDetector::parseGraphicsMode(const String &str) {
	if (str.isEmpty())
		return -1;

	const char *s = str.c_str();
	const GraphicsMode *gm = gfx_modes;
	while (gm->name) {
		if (!scumm_stricmp(gm->name, s)) {
			return gm->id;
		}
		gm++;
	}

	return -1;
}

bool GameDetector::detectGame() {
	String realGame;

	if (ConfMan.hasKey("gameid"))
		realGame = ConfMan.get("gameid");
	else
		realGame = _targetName;
	printf("Looking for %s\n", realGame.c_str());
	
	_game = findGame(realGame, &_plugin);
	
	if (_game.gameName) {
		printf("Trying to start game '%s'\n", _game.description);
		return true;
	} else {
		printf("Failed game detection\n");
		return false;
	}
}

int GameDetector::detectMusicDriver(int midiFlags) {
	int musicDriver = parseMusicDriver(ConfMan.get("music_driver"));
	/* Use the adlib sound driver if auto mode is selected,
	 * and the game is one of those that want adlib as
	 * default, OR if the game is an older game that doesn't
	 * support anything else anyway. */
	if (musicDriver == MD_AUTO || musicDriver < 0) {
		if (midiFlags & MDT_PREFER_NATIVE) {
			if (musicDriver == MD_AUTO) {
				#if defined (WIN32) && !defined(_WIN32_WCE)
					musicDriver = MD_WINDOWS; // MD_WINDOWS is default MidiDriver on windows targets
				#elif defined(MACOSX)
					musicDriver = MD_COREAUDIO;
				#elif defined(__PALM_OS__)	// must be before mac
					musicDriver = MD_YPA1;
				#elif defined(__MORPHOS__)
					musicDriver = MD_ETUDE;
				#elif defined (_WIN32_WCE) || defined(UNIX) || defined(X11_BACKEND)
					// Always use MIDI emulation via adlib driver on CE and UNIX device
				
					// TODO: We should, for the Unix targets, attempt to detect
					// whether a sequencer is available, and use it instead.
					musicDriver = MD_ADLIB;
				#else
				    musicDriver = MD_NULL;
				#endif
			} else
				musicDriver = MD_ADLIB;
		} else
			musicDriver = MD_TOWNS;
	}
	bool nativeMidiDriver =
		(musicDriver != MD_NULL && musicDriver != MD_ADLIB &&
		 musicDriver != MD_PCSPK && musicDriver != MD_PCJR &&
		 musicDriver != MD_TOWNS);

	if (nativeMidiDriver && !(midiFlags & MDT_NATIVE))
		musicDriver = MD_TOWNS;
	if (musicDriver == MD_TOWNS && !(midiFlags & MDT_TOWNS))
		musicDriver = MD_ADLIB;
	if (musicDriver == MD_ADLIB && !(midiFlags & MDT_ADLIB))
		musicDriver = MD_PCJR;
	if ((musicDriver == MD_PCSPK || musicDriver == MD_PCJR) && !(midiFlags & MDT_PCSPK))
		musicDriver = MD_NULL;

	return musicDriver;
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
#ifndef __PALM_OS__	// add last slash also in File::fopenNoCase, so this is not needed
	} else if (gameDataPath.lastChar() != '/'
#ifdef __MORPHOS__
					&& gameDataPath.lastChar() != ':'
#endif
					&& gameDataPath.lastChar() != '\\') {
		gameDataPath += '/';
		ConfMan.set("path", gameDataPath);
#endif
	}

	return true;
}

OSystem *GameDetector::createSystem() {
	int gfx_mode = parseGraphicsMode(ConfMan.get("gfx_mode"));	// FIXME: Get rid of this again!
	
#if defined(USE_NULL_DRIVER)
	return OSystem_NULL_create();
#elif defined(__DC__)
	return OSystem_Dreamcast_create();
#elif defined(X11_BACKEND)
	return OSystem_X11_create();
#elif defined(__MORPHOS__)
	return OSystem_MorphOS_create(gfx_mode, ConfMan.getBool("fullscreen"));
#elif defined(_WIN32_WCE)
	return OSystem_WINCE3_create();
#elif defined(MACOS_CARBON)
	return OSystem_MAC_create(gfx_mode, ConfMan.getBool("fullscreen"));
#elif defined(__GP32__)	// ph0x
	return OSystem_GP32_create(GFX_NORMAL, true);
#elif defined(__PALM_OS__) //chrilith
	return OSystem_PALMOS_create(gfx_mode, ConfMan.getBool("fullscreen"));
#else
	/* SDL is the default driver for now */
	return OSystem_SDL_create(gfx_mode, ConfMan.getBool("fullscreen"), ConfMan.getBool("aspect_ratio"), ConfMan.getInt("joystick_num"));
#endif
}

Engine *GameDetector::createEngine(OSystem *sys) {
	assert(_plugin);
	return _plugin->createInstance(this, sys);
}

SoundMixer *GameDetector::createMixer() {
	return new SoundMixer();
}

MidiDriver *GameDetector::createMidi(int midiDriver) {
	switch(midiDriver) {
	case MD_NULL:      return MidiDriver_NULL_create();

	// In the case of Adlib, we won't specify anything.
	// IMuse is designed to set up its own Adlib driver
	// if need be, and we only have to specify a native
	// driver.
	case MD_ADLIB:     return NULL;

	case MD_TOWNS:     return MidiDriver_YM2612_create(g_engine->_mixer);

	// Right now PC Speaker and PCjr are handled
	// outside the MidiDriver architecture, so
	// don't create anything for now.
	case MD_PCSPK:
	case MD_PCJR:      return NULL;
#if defined(__PALM_OS__)
	case MD_YPA1:      return MidiDriver_YamahaPa1_create();
#endif
#if defined(WIN32) && !defined(_WIN32_WCE)
	case MD_WINDOWS:   return MidiDriver_WIN_create();
#endif
#if defined(__MORPHOS__)
	case MD_ETUDE:     return MidiDriver_ETUDE_create();
#endif
#if defined(UNIX) && !defined(__BEOS__) && !defined(MACOSX)
	case MD_SEQ:       return MidiDriver_SEQ_create();
#endif
#if (defined(MACOSX) || defined(macintosh)) && !defined(__PALM_OS__)
	case MD_QTMUSIC:   return MidiDriver_QT_create();
#endif
#if defined(MACOSX)
	case MD_COREAUDIO: return MidiDriver_CORE_create();
#endif
#if defined(UNIX) && defined(USE_ALSA)
	case MD_ALSA:      return MidiDriver_ALSA_create();
#endif
	}

	error("Invalid midi driver selected");
	return NULL;
}
