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

#define CHECK_OPTION() if ((current_option != NULL) || (*s != '\0')) goto ShowHelpAndExit
#define HANDLE_OPTION() if ((*s == '\0') && (current_option == NULL)) goto ShowHelpAndExit;  \
                        if ((*s != '\0') && (current_option != NULL)) goto ShowHelpAndExit; \
                        option = (*s == '\0' ? current_option : s);                         \
                        current_option = NULL
#define HANDLE_OPT_OPTION() if ((*s != '\0') && (current_option != NULL)) goto ShowHelpAndExit; \
                            if ((*s == '\0') && (current_option == NULL)) option = NULL;         \
                            else option = (*s == '\0' ? current_option : s);                    \
                            current_option = NULL

// DONT FIXME: DO NOT ORDER ALPHABETICALY, THIS IS ORDERED BY IMPORTANCE/CATEGORY! :)
// FIXME: Reconsider and reevaluate our command line options. I propose some changes:
// * add long alternatives for (almost?) all options. There should be some generic way
//   to specify this, in order to avoid code duplication and to simplify the code
// * some more obscure options which currently have a single-letter options assigned
//   really don't need it. E.g. the "-t<NUM>" option probably is rarely used, and anybody
//   who needs it should still be happy with "--tempo=NUM"
//   This will make room in the ranks of single-letter options for other more useful options
//   (for example, "-t" could stand for "List (T)argets")
// * -y, -t, -c, -j all are candidates for this, IMHO: their settings are rather obscure,
//   and anybody who needs them always should put them into the config file anyway
//
#ifdef __PALM_OS__
static const char USAGE_STRING[] = "NoUsageString"; // save more data segment space
#else
static const char USAGE_STRING[] = 
	"ScummVM - Graphical Adventure Game Interpreter\n"
	"Syntax:\n"
	"  scummvm [OPTIONS] [game]\n"
	"Options:\n"
	"  -p<path>       - Look for game in <path>\n"
	"  -x[num]        - Load this savegame (default: 0 - autosave)\n"
	"  -f             - Full-screen mode (-F forces window mode.)\n"
	"  -g<mode>       - Graphics mode (normal,2x,3x,2xsai,super2xsai,supereagle,\n"
	"                   advmame2x,advmame3x,hq2x,hq3x,tv2x,dotmatrix)\n"
	"  -e<mode>       - Set music engine (see README for details)\n"
	"  -q<lang>       - Specify language (en,de,fr,it,pt,es,jp,zh,kr,se,gb,hb)\n"
	"\n"
	"  -c<num>        - Use cdrom <num> for cd audio\n"
	"  -j[num]        - Enable input with joystick (default: 0 - first joystick)\n"
	"  -m<num>        - Set music volume to <num> (0-255)\n"
	"  -o<num>        - Set master volume to <num> (0-255)\n"
	"  -s<num>        - Set sfx volume to <num> (0-255)\n"
#ifndef DISABLE_SCUMM
	"  -t<num>        - Set music tempo (50-200, default 100%%)\n"
#endif
	"\n"
	"  -n             - No subtitles for speech\n"
#ifndef DISABLE_SCUMM
	"  -y             - Set text speed (default: 60)\n"
#endif
	"\n"
	"  -v             - Show version info and exit\n"
	"  -h             - Display this text and exit\n"
	"  -z             - Display list of supported games\n"
	"\n"
	"  -b<num>        - Pass number to the boot script (boot param)\n"
	"  -d[num]        - Enable debug output (debug level [0])\n"
	"  -u             - Dump scripts\n"
	"\n"
	"  --platform=    - Specify version of game (amiga,atari-st,macintosh)\n"
	"  --multi-midi   - Enable combination Adlib and native MIDI\n"
	"  --native-mt32  - True Roland MT-32 (disable GM emulation)\n"
	"  --fullscreen   - Full-screen mode (same as -f)\n"
	"  --aspect-ratio - Enable aspect ratio correction\n"
#ifndef DISABLE_SCUMM
	"  --demo-mode    - Start demo mode of Maniac Mansion (Classic version)\n"
#endif
#ifndef DISABLE_SKY
	"  --floppy-intro - Use floppy version intro for Beneath a Steel Sky CD\n"
#endif
	"\n"
	"The meaning of long options can be inverted by prefixing them with \"no-\",\n"
	"e.g. \"--no-aspect-ratio\".\n"
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

struct LanguageDescription {
	const char *name;
	const char *description;
	Language id;
};

static const struct LanguageDescription languages[] = {
	{"en", "English", EN_USA},
	{"de", "German", DE_DEU},
	{"fr", "French", FR_FRA},
	{"it", "Italian", IT_ITA},
	{"pt", "Portuguese", PT_BRA},
	{"es", "Spanish", ES_ESP},
	{"jp", "Japanese", JA_JPN},
	{"zh", "Chinese (Taiwan)", ZH_TWN},
	{"kr", "Korean", KO_KOR},
	{"gb", "English", EN_GRB},
	{"se", "Swedish", SE_SWE},
	{"hb", "Hebrew", HB_HEB},
	{0, 0, UNK_LANG}
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
	ConfMan.registerDefault("master_volume", kDefaultMasterVolume);
	ConfMan.registerDefault("music_volume", kDefaultMusicVolume);
	ConfMan.registerDefault("sfx_volume", kDefaultSFXVolume);

	ConfMan.registerDefault("multi_midi", false);
	ConfMan.registerDefault("native_mt32", false);
//	ConfMan.registerDefault("music_driver", ???);

	ConfMan.registerDefault("cdrom", 0);

	// Game specifc
	ConfMan.registerDefault("path", "");

	ConfMan.registerDefault("amiga", false);
	ConfMan.registerDefault("platform", kPlatformPC);
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

	_debugMode = false;
	_dumpScripts = false;
	_midi_driver = MD_AUTO;

	_saveconfig = false;

	_game.features = 0;
	_plugin = 0;
}

void GameDetector::list_games() {
	// FIXME / TODO: config rewrite
	// Right now this lists all known built-in targets; and also for each of
	// those it tells the user if the target is "configured".
	// To me this seems like an ill mix of two different functionalities.
	// IMHO we should split this into two seperate commands/options:
	// 1) List all built-in gameids (e.g. monkey, atlantis, ...) similiar to 
	//    what this code does, but without the "Config" column.
	// 2) List all available (configured) targets, including those with custom
	//    names, e.g. "monkey-mac", "skycd-demo", ...
	const PluginList &plugins = PluginManager::instance().getPlugins();
	const TargetSettings *v;

	printf("Game             Full Title                                            \n"
	       "---------------- ------------------------------------------------------\n");

	PluginList::ConstIterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		v = (*iter)->getTargets();
		while (v->targetName && v->description) {
#if 1
			printf("%-17s%-56s\n", v->targetName, v->description);
#else
			const char *config = (g_config->has_domain(v->targetName)) ? "Yes" : "";
			printf("%-17s%-56s%s\n", v->targetName, v->description, config);
#endif
			v++;
		}
	}
}

const TargetSettings *GameDetector::findTarget(const String &targetName, const Plugin **plugin) const {
	// Find the TargetSettings for this target
	const TargetSettings *target;
	const PluginList &plugins = PluginManager::instance().getPlugins();
	
	PluginList::ConstIterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		target = (*iter)->findTarget(targetName.c_str());
		if (target) {
			if (plugin)
				*plugin = *iter;
			return target;
		}
	}
	return 0;
}

void GameDetector::parseCommandLine(int argc, char **argv) {
	int i;
	char *s;
	char *current_option = NULL;
	char *option = NULL;
	char c;
	bool long_option_value;

	// Iterate over all comman line arguments, backwards.
	// FIXME: Looping backwards has a major problem: Consider this example
	// invocation: "scummvm -g 1x". It should work exactly like "scummvm -g1x"
	// but it doesn't! Instead of starting the launcher with the 1x sacler
	// in effect, it will give an error about target 1x being unknown.
	// This can be fixed by forward iterating the args. Of course doing that
	// essentially means we have to rewrite the whole command line parser,
	// but that seems like a good idea anyway.
	for (i = argc - 1; i >= 1; i--) {
		s = argv[i];

		if (s[0] == '-') {
			s++;
			c = *s++;
			switch (tolower(c)) {
			case 'b':
				HANDLE_OPTION();
				ConfMan.set("boot_param", (int)strtol(option, 0, 10));
				break;
			case 'c':
				HANDLE_OPTION();
				ConfMan.set("cdrom", (int)strtol(option, 0, 10));
				break;
			case 'd':
				_debugMode = true;
				HANDLE_OPT_OPTION();
				if (option != NULL)
					ConfMan.set("debuglevel", (int)strtol(option, 0, 10));
				if (ConfMan.getInt("debuglevel")) {
					printf("Debuglevel (from command line): %d\n", ConfMan.getInt("debuglevel"));
				} else {
					printf("Debuglevel (from command line): 0 - Engine only\n");
				}
				break;
			case 'e':
				HANDLE_OPTION();
				// TODO: Instead of just showing the generic help text,
				// maybe print a message like:
				// "'option' is not a supported music driver on this machine.
				//  Available driver: ..."
				if (parseMusicDriver(option) < 0)
					goto ShowHelpAndExit;
				ConfMan.set("music_driver", option);
				break;
			case 'f':
				CHECK_OPTION();
				ConfMan.set("fullscreen", (c == 'f'));
				break;
			case 'g':{
				HANDLE_OPTION();
				int _gfx_mode = parseGraphicsMode(option);
				// TODO: Instead of just showing the generic help text,
				// maybe print a message like:
				// "'option' is not a supported graphic mode on this machine.
				//  Available graphic modes: ..."
				if (_gfx_mode == -1)
					goto ShowHelpAndExit;
				ConfMan.set("gfx_mode", option);
				break;}
			// case 'h': reserved for help
			case 'j':
				HANDLE_OPT_OPTION();
				ConfMan.set("joystick_num", (option != NULL) ? (int)strtol(option, 0, 10) : 0);
				break;
			case 'm':
				HANDLE_OPTION();
				ConfMan.set("music_volume", (int)strtol(option, 0, 10));
				break;
			case 'n':
				CHECK_OPTION();
				ConfMan.set("nosubtitles", (c == 'n'));
				break;
 			case 'o':
 				HANDLE_OPTION();
 				ConfMan.set("master_volume", (int)strtol(option, 0, 10));
 				break;
			case 'p':
				HANDLE_OPTION();
				// TODO: Verify path is valid
				ConfMan.set("path", option);
				break;
			case 'q':
				HANDLE_OPTION();
				if (parseLanguage(option) == UNK_LANG)
					goto ShowHelpAndExit;
				ConfMan.set("language", option);
				break;
			case 's':
				HANDLE_OPTION();
				ConfMan.set("sfx_volume", (int)strtol(option, 0, 10));
				break;
#ifndef DISABLE_SCUMM
			case 't':
				HANDLE_OPTION();
				// Use the special value '0' for the base in (int)strtol. 
				// Doing that makes it possible to enter hex values
				// as "0x1234", but also decimal values ("123").
				ConfMan.set("tempo", (int)strtol(option, 0, 0));
				break;
#endif
			case 'u':
				CHECK_OPTION();
				_dumpScripts = true;
				break;
			case 'v':
				CHECK_OPTION();
				printf("%s\n", gScummVMFullVersion);
				exit(0);
				break;
			case 'x':
				HANDLE_OPT_OPTION();
				ConfMan.set("save_slot", (option != NULL) ? (int)strtol(option, 0, 10) : 0);
				break;
#ifndef DISABLE_SCUMM
			case 'y':
				HANDLE_OPTION();
				ConfMan.set("talkspeed", (int)strtol(option, 0, 10));
				break;
#endif
			case 'z':
				CHECK_OPTION();
				list_games();
				exit(0);
			case '-':
				// Long options. Let the fun begin!
				if (!strncmp(s, "platform=", 9)) {
					s += 9;
					int platform = parsePlatform(s);
					if (platform == kPlatformUnknown)
						goto ShowHelpAndExit;

					ConfMan.set("platform", platform);
					break;
				} 

				if (!strncmp(s, "no-", 3)) {
					long_option_value = false;
					s += 3;
				} else
					long_option_value = true;

				if (!strcmp (s, "multi-midi")) {
					ConfMan.set("multi_midi", long_option_value);
				} else if (!strcmp (s, "native-mt32")) {
					ConfMan.set("native_mt32", long_option_value);
				} else if (!strcmp (s, "aspect-ratio")) {
					ConfMan.set("aspect_ratio", long_option_value);
				} else if (!strcmp (s, "fullscreen")) {
					ConfMan.set("fullscreen", long_option_value);
#ifndef DISABLE_SCUMM
				} else if (!strcmp (s, "demo-mode")) {
					ConfMan.set("demo_mode", long_option_value);
#endif

#ifndef DISABLE_SKY
				} else if (!strcmp (s, "floppy-intro")) {
					ConfMan.set("floppy_intro", long_option_value);
#endif
				} else {
					goto ShowHelpAndExit;
				}
				break;
			default:
				goto ShowHelpAndExit;
			}
		} else {
			// Last argument: this could be a target name.
			// To verify this, check if there is either a game domain (i.e
			// a configured target) matching this argument, or if we can
			// find any target with that name.
			if (i == (argc - 1) && (ConfMan.hasGameDomain(s) || findTarget(s))) {
				setGame(s);
			} else {
				if (current_option == NULL)
					current_option = s;
				else
					goto ShowHelpAndExit;
			}
		}
	}
	
	return;

ShowHelpAndExit:
	printf(USAGE_STRING);
	exit(1);
}

void GameDetector::setGame(const String &name) {
	_gameFileName = name;
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

Language GameDetector::parseLanguage(const String &str) {
	if (str.isEmpty())
		return UNK_LANG;

	const char *s = str.c_str();
	const LanguageDescription *l = languages;
	while (l->name) {
		if (!scumm_stricmp(l->name, s))
			return l->id;
		l++;
	}

	return UNK_LANG;
}

Platform GameDetector::parsePlatform(const String &str) {
	if (str.isEmpty())
		return kPlatformUnknown;

	const char *s = str.c_str();
	if (!scumm_stricmp(s, "pc"))
		return kPlatformPC;
	else if (!scumm_stricmp(s, "amiga") || !scumm_stricmp(s, "1"))
		return kPlatformAmiga;
	else if (!scumm_stricmp(s, "atari-st") || !scumm_stricmp(s, "atari") || !scumm_stricmp(s, "2"))
		return kPlatformAtariST;
	else if (!scumm_stricmp(s, "macintosh") || !scumm_stricmp(s, "mac") || !scumm_stricmp(s, "3"))
		return kPlatformMacintosh;
	else
		return kPlatformUnknown;
}

int GameDetector::parseMusicDriver(const String &str) {
	if (str.isEmpty())
		return -1;

	const char *s = str.c_str();
	const MidiDriverDescription *md = getAvailableMidiDrivers();

	while (md->name) {
		if (!scumm_stricmp(md->name, s)) {
			return md->id;
		}
		md++;
	}

	return -1;
}

bool GameDetector::detectGame() {
	const TargetSettings *target;
	String realGame;

	if (ConfMan.hasKey("gameid"))
		realGame = ConfMan.get("gameid");
	else
		realGame = _gameFileName;
	printf("Looking for %s\n", realGame.c_str());
	
	target = findTarget(realGame, &_plugin);
	
	if (target) {
		_game = *target;
		if (ConfMan.hasKey("basename")) {
			// FIXME: What is this good for?
			// FIXME: This leaks now!
			_game.targetName = strdup(ConfMan.get("basename").c_str());
		}
		printf("Trying to start game '%s'\n", _game.description);
		return true;
	} else {
		printf("Failed game detection\n");
		return false;
	}
}

bool GameDetector::detectMain() {
	if (_gameFileName.isEmpty()) {
		warning("No game was specified...");
		return false;
	}

	if (!detectGame()) {
		warning("%s is an invalid target. Use the -z parameter to list targets", _gameFileName.c_str());
		return false;
	}

	/* Use the adlib sound driver if auto mode is selected,
	 * and the game is one of those that want adlib as
	 * default, OR if the game is an older game that doesn't
	 * support anything else anyway. */
	_midi_driver = parseMusicDriver(ConfMan.get("music_driver"));
	if (_midi_driver == MD_AUTO || _midi_driver < 0) {
		if (_game.midi & MDT_PREFER_NATIVE)
			_midi_driver = getMidiDriverType();
		else
			_midi_driver = MD_TOWNS;
	}
	bool nativeMidiDriver =
		(_midi_driver != MD_NULL && _midi_driver != MD_ADLIB &&
		 _midi_driver != MD_PCSPK && _midi_driver != MD_PCJR &&
		 _midi_driver != MD_TOWNS);
	if (nativeMidiDriver && !(_game.midi & MDT_NATIVE))
		_midi_driver = MD_TOWNS;
	if (_midi_driver == MD_TOWNS && !(_game.midi & MDT_TOWNS))
		_midi_driver = MD_ADLIB;
	if (_midi_driver == MD_ADLIB && !(_game.midi & MDT_ADLIB))
		_midi_driver = MD_PCJR;
	if ((_midi_driver == MD_PCSPK || _midi_driver == MD_PCJR) && !(_game.midi & MDT_PCSPK))
		_midi_driver = MD_NULL;

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
	int _gfx_mode = parseGraphicsMode(ConfMan.get("gfx_mode"));	// FIXME: Get rid of this again!
	
#if defined(USE_NULL_DRIVER)
	return OSystem_NULL_create();
#elif defined(__DC__)
	return OSystem_Dreamcast_create();
#elif defined(X11_BACKEND)
	return OSystem_X11_create();
#elif defined(__MORPHOS__)
	return OSystem_MorphOS_create(_gfx_mode, ConfMan.getBool("fullscreen"));
#elif defined(_WIN32_WCE)
	return OSystem_WINCE3_create();
#elif defined(MACOS_CARBON)
	return OSystem_MAC_create(_gfx_mode, ConfMan.getBool("fullscreen"));
#elif defined(__GP32__)	// ph0x
	return OSystem_GP32_create(GFX_NORMAL, true);
#elif defined(__PALM_OS__) //chrilith
	return OSystem_PALMOS_create(_gfx_mode, ConfMan.getBool("fullscreen"));
#else
	/* SDL is the default driver for now */
	return OSystem_SDL_create(_gfx_mode, ConfMan.getBool("fullscreen"), ConfMan.getBool("aspect_ratio"), ConfMan.getInt("joystick_num"));
#endif
}

Engine *GameDetector::createEngine(OSystem *sys) {
	assert(_plugin);
	return _plugin->createInstance(this, sys);
}

int GameDetector::getMidiDriverType() {

	if (_midi_driver != MD_AUTO) return _midi_driver;

#if defined (WIN32) && !defined(_WIN32_WCE)
		return MD_WINDOWS; // MD_WINDOWS is default MidiDriver on windows targets
#elif defined(MACOSX)
		return MD_COREAUDIO;
#elif defined(__PALM_OS__)	// must be before mac
		return MD_YPA1;
#elif defined(macintosh)
		return MD_QTMUSIC;
#elif defined(__MORPHOS__)
		return MD_ETUDE;
#elif defined (_WIN32_WCE) || defined(UNIX) || defined(X11_BACKEND)
	// Always use MIDI emulation via adlib driver on CE and UNIX device

	// TODO: We should, for the Unix targets, attempt to detect
	// whether a sequencer is available, and use it instead.
	return MD_ADLIB;
#endif
    return MD_NULL;
}

SoundMixer *GameDetector::createMixer() {
	return new SoundMixer();
}

MidiDriver *GameDetector::createMidi() {
	int drv = getMidiDriverType();

	switch(drv) {
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
