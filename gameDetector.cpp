/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "scumm.h"
#include "sound/mididrv.h"
#include "sound/imuse.h"
#include "gameDetector.h"


#define CHECK_OPTION() if ((current_option != NULL) || (*s != '\0')) goto ShowHelpAndExit
#define HANDLE_OPTION() if ((*s == '\0') && (current_option == NULL)) goto ShowHelpAndExit;  \
                        if ((*s != '\0') && (current_option != NULL)) goto ShowHelpAndExit; \
                        option = (*s == '\0' ? current_option : s);                         \
                        current_option = NULL
#define HANDLE_OPT_OPTION() if ((*s != '\0') && (current_option != NULL)) goto ShowHelpAndExit; \
                            if ((*s == '\0') && (current_option == NULL)) option = NULL;         \
                            else option = (*s == '\0' ? current_option : s);                    \
                            current_option = NULL

static const char USAGE_STRING[] = 
	"ScummVM - Scumm Interpreter\n"
	"Syntax:\n"
	"\tscummvm [-v] [-d[<num>]] [-n] [-b<num>] [-t<num>] [-s<num>] [-p<path>] [-m<num>] [-f] game\n"
	"Flags:\n"
	"\t-v         - show version info and exit\n"
	"\t-c<num>    - use cdrom <num> for cd audio\n"
	"\t-d[<num>]  - enable debug output (debug level [1])\n"
	"\t-n         - no subtitles for speech\n"
	"\t-b<num>    - start in room <num>\n"
	"\t-t<num>    - set music tempo (default- adlib: 0x1F0000, midi: 0x460000)\n"
	"\t-p<path>   - look for game in <path>\n"
	"\t-m<num>    - set music volume to <num> (0-100)\n"
	"\t-s<num>    - set sfx volume to <num> (0-255)\n"
	"\t-e<mode>   - set music engine (see readme.txt for details)\n"
	"\t-r         - emulate roland mt32 instruments\n"
	"\t-f         - fullscreen mode\n"
	"\t-g<mode>   - graphics mode (normal,2x,3x,2xsai,super2xsai,supereagle,advmame2x)\n"
	"\t-a         - specify game is amiga version\n"
#if defined(UNIX) || defined(UNIX_X11)
	"\t-w[<file>] - write to config file [~/.scummvmrc]\n"
#else
	"\t-w[<file>] - write to config file [scummvm.ini]\n"
#endif
	"\t-l<file>   - load config file instead of default\n"
;

void GameDetector::updateconfig()
{
	const char * val;

	if ((val = scummcfg->get("amiga")))
		if (!scumm_stricmp(val, "true"))
			_amiga = true;
		else
			_amiga = false;

	if ((val = scummcfg->get("fullscreen", "scummvm")))
		if (!scumm_stricmp(val, "true"))
			_fullScreen = true;
		else
			_fullScreen = false;

	if ((val = scummcfg->get("path")))
		_gameDataPath = Scumm::Strdup(val);

	if ((val = scummcfg->get("tempo")))
		_gameTempo = strtol(val, NULL, 0);

	if ((val = scummcfg->get("music_volume")))
		_music_volume = atoi(val);

	if ((val = scummcfg->get("sfx_volume")))
		_sfx_volume = atoi(val);

	if ((val = scummcfg->get("mt32emulate")))
		if (!scumm_stricmp(val, "true"))
			_mt32emulate = true;
		else
			_mt32emulate = false;
	
	if ((val = scummcfg->get("nosubtitles")))
		if (!scumm_stricmp(val, "true"))
			_noSubtitles = true;
		else
			_noSubtitles = false;

	if ((val = scummcfg->get("music_driver")))
		if (!parseMusicDriver(val)) {
			printf("Error in the config file: invalid music_driver.\n");
			printf(USAGE_STRING);
			exit(-1);
		}

	if ((val = scummcfg->get("gfx_mode")))
		if ((_gfx_mode = parseGraphicsMode(val)) == -1) {
			printf("Error in the config file: invalid gfx_mode.\n");
			printf(USAGE_STRING);
			exit(-1);
		}

	if ((val = scummcfg->get("cdrom")))
		_cdrom = atoi(val);
}

void GameDetector::parseCommandLine(int argc, char **argv)
{
#if !defined(__APPLE__CW)
	int i;
	char *s;
	char *current_option = NULL;
	char *option = NULL;

	// check for arguments
	if (argc < 2) {
		printf(USAGE_STRING);
		//exit(1);
	}

	scummcfg->set_domain("game-specific");
	/* Parse the arguments */
	for (i = argc - 1; i >= 1; i--) {
		s = argv[i];

		if (s[0] == '-') {
			s++;
			switch (tolower(*s++)) {
			case 'a':
				CHECK_OPTION();
				_amiga = true;
				scummcfg->set("amiga", "true");
				break;
			case 'b':
				HANDLE_OPTION();
				_bootParam = atoi(option);
				break;
			case 'c':
				HANDLE_OPTION();
				_cdrom = atoi(option);
				scummcfg->set("cdrom", _cdrom);
				break;
			case 'd':
				_debugMode = true;
				HANDLE_OPT_OPTION();
				if (option != NULL)
					_debugLevel = atoi(option);
				debug(1,"Debugmode (level %d) on", _debugLevel);
				break;
			case 'e':
				HANDLE_OPTION();
				if (!parseMusicDriver(option))
					goto ShowHelpAndExit;
				scummcfg->set("music_driver", option);
				break;
			case 'f':
				CHECK_OPTION();
				_fullScreen = true;
				scummcfg->set("fullscreen", "true", "scummvm");
				break;
			case 'g':
				HANDLE_OPTION();
				_gfx_mode = parseGraphicsMode(option);
				if (_gfx_mode == -1)
					goto ShowHelpAndExit;
				scummcfg->set("gfx_mode", option, "scummvm");
				break;
			case 'l':
				HANDLE_OPTION();
				{
					Config * newconfig = new Config(option, "scummvm");
					scummcfg->merge_config(newconfig);
					delete newconfig;
					updateconfig();
					break;
				}
				break;
			case 'm':
				HANDLE_OPTION();
				_music_volume = atoi(option);
				scummcfg->set("music_volume", _music_volume, "scummvm");
				break;
			case 'n':
				CHECK_OPTION();
				_noSubtitles = true;
				scummcfg->set("nosubtitles", "true");
				break;
			case 'p':
				HANDLE_OPTION();
				_gameDataPath = option;
				scummcfg->set("path", _gameDataPath);
				break;
			case 'r':
				CHECK_OPTION();
				_mt32emulate = true;
				scummcfg->set("mt32emulate", "true");
				break;
			case 's':
				HANDLE_OPTION();
				_sfx_volume = atoi(option);
				scummcfg->set("sfx_volume", _sfx_volume, "scummvm");
				break;
			case 't':
				HANDLE_OPTION();
				_gameTempo = strtol(option, 0, 0);
				scummcfg->set("tempo", option);
				break;
			case 'v':
				CHECK_OPTION();
				printf("ScummVM " SCUMMVM_VERSION "\nBuilt on " __DATE__ " "
							 __TIME__ "\n");
#ifdef SCUMMVM_PLATFORM_VERSION
				printf("    " SCUMMVM_PLATFORM_VERSION "\n");
#endif
				exit(1);
			case 'w':
				_saveconfig = true;
				HANDLE_OPT_OPTION();
				if (option != NULL)
					scummcfg->change_filename(option);
				break;
			default:
				goto ShowHelpAndExit;
			}
		} else {
			if (i == (argc - 1)) {
				_exe_name = s;
				scummcfg->set_domain(s);
				scummcfg->rename_domain("game-specific");
				scummcfg->rename_domain(s);
				updateconfig();
			} else {
				if (current_option == NULL)
					current_option = s;
				else
					goto ShowHelpAndExit;
			}
		}
	}
	
	if (_saveconfig)
		scummcfg->flush();

	return;

 ShowHelpAndExit:
	printf(USAGE_STRING);
	exit(1);

#else
	_midi_driver = MD_QTMUSIC;
	_exe_name = *argv;
	_gameDataPath = (char *)malloc(strlen(_exe_name) + 3);
	sprintf(_gameDataPath, ":%s:", _exe_name);
#endif

}

int GameDetector::parseGraphicsMode(const char *s) {
	struct GraphicsModes {
		const char *name;
		int id;
	};

	const struct GraphicsModes gfx_modes[] = {
		{"normal",GFX_NORMAL},
		{"2x",GFX_DOUBLESIZE},
		{"3x",GFX_TRIPLESIZE},
		{"2xsai",GFX_2XSAI},
		{"super2xsai",GFX_SUPER2XSAI},
		{"supereagle",GFX_SUPEREAGLE},
		{"advmame2x",GFX_ADVMAME2X}
	};

	const GraphicsModes *gm = gfx_modes;
	int i;
	for(i=0; i!=ARRAYSIZE(gfx_modes); i++,gm++) {
		if (!scumm_stricmp(gm->name, s))
			return gm->id;
	}

	return -1;
}

bool GameDetector::parseMusicDriver(const char *s) {
	struct MusicDrivers {
		const char *name;
		int id;
	};

	const struct MusicDrivers music_drivers[] = {
		{"auto",MD_AUTO},
		{"null",MD_NULL},
		{"windows",MD_WINDOWS},
		{"timidity",MD_TIMIDITY},
		{"seq",MD_SEQ},
		{"qt",MD_QTMUSIC},
		{"core",MD_COREAUDIO},
		{"amidi",MD_AMIDI},
		{"adlib",-1},
	};

	const MusicDrivers *md = music_drivers;
	int i;

	_use_adlib = false;

	for(i=0; i!=ARRAYSIZE(music_drivers); i++,md++) {
		if (!scumm_stricmp(md->name, s)) {
			if (md->id == -1) {
				_use_adlib = true;
			}
			_midi_driver = md->id;
			return true;
		}
	}

	return false;
}


struct VersionSettings {
	const char *filename;
	const char *gamename;
	byte id, major, middle, minor;
	uint32 features;
};

/*
        This is a list of all known SCUMM games. Commented games are not
        supported at this time */

static const VersionSettings version_settings[] = {
	/* Scumm Version 1 */
//      {"maniac",      "Maniac Mansion (C64)",                         GID_MANIAC64, 1, 0, 0,},
//      {"zak",         "Zak McKracken and the Alien Mindbenders (C64)", GID_ZAK64, 1, 0, 0,},

	/* Scumm Version 2 */
//      {"maniac",      "Maniac Mansion", GID_MANIAC, 2, 0, 0,},
//      {"zak",         "Zak McKracken and the Alien Mindbenders",      GID_ZAK,     2, 0, 0,},
//      {"indy3",       "Indiana Jones and the Last Crusade",           GID_INDY3,   2, 0, 0,},

	/* Scumm Version 3 */
	{"indy3", "Indiana Jones and the Last Crusade (256)", GID_INDY3_256, 3, 0,
	 22,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_OLD256 |
	 GF_NO_SCALLING},
	{"zak256", "Zak McKracken and the Alien Mindbenders (256)", GID_ZAK256, 3,
	 0, 0,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_OLD256 | GF_AUDIOTRACKS
	 | GF_NO_SCALLING},
	{"loom", "Loom", GID_LOOM, 3, 5, 40,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_OLD_BUNDLE | GF_16COLOR
	 | GF_NO_SCALLING},

	/* Scumm Version 4 */
	{"monkeyEGA", "Monkey Island 1 (EGA)", GID_MONKEY_EGA, 4, 0, 67, GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR},	// EGA version

	/* Scumm version 5 */
	{"monkeyVGA", "Monkey Island 1 (256 color Floppy version)", GID_MONKEY_VGA,  5, 0, 16,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT},
	{"loomcd", "Loom (256 color CD version)", GID_LOOM256, 5, 1, 42,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT},
	{"monkey", "Monkey Island 1", GID_MONKEY, 5, 2, 2,
	 GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT},
	{"monkey1", "Monkey Island 1 (alt)", GID_MONKEY, 5, 2, 2,
	 GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT},
	{"monkey2", "Monkey Island 2: LeChuck's revenge", GID_MONKEY2, 5, 2, 2,
	 GF_USE_KEY | GF_ADLIB_DEFAULT},
	{"atlantis", "Indiana Jones 4 and the Fate of Atlantis", GID_INDY4, 5, 5, 0,
	 GF_USE_KEY | GF_ADLIB_DEFAULT},
	{"playfate", "Indiana Jones 4 and the Fate of Atlantis (Demo)", GID_INDY4,
	 5, 5, 0, GF_USE_KEY | GF_ADLIB_DEFAULT},

	/* Scumm Version 6 */
	{"tentacle", "Day Of The Tentacle", GID_TENTACLE, 6, 4, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT},
	{"dottdemo", "Day Of The Tentacle (Demo)", GID_TENTACLE, 6, 3, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT},
	{"samnmax", "Sam & Max", GID_SAMNMAX, 6, 4, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER},
	{"snmdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY  | GF_ADLIB_DEFAULT},

	/* Scumm Version 7 */
	{"ft", "Full Throttle", GID_FT, 7, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7},
	{"dig", "The Dig", GID_DIG, 7, 5, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7},

	/* Simon the Sorcerer 1 & 2 (not SCUMM games) */
	{"simon1dos", "Simon the Sorcerer 1 for DOS", GID_SIMON_FIRST+0, 99, 99, 99, 0},
	{"simon1win", "Simon the Sorcerer 1 for Windows", GID_SIMON_FIRST+2, 99, 99, 99, 0},
	{"simon2win", "Simon the Sorcerer 2 for Windows", GID_SIMON_FIRST+3, 99, 99, 99, 0},

	/* Scumm Version 8 */
//    {"curse",       "The Curse of Monkey Island",                   GID_CMI,      8, 1, 0,},
	{NULL, NULL}
};

bool GameDetector::detectGame()
{
	const VersionSettings *gnl = version_settings;

	_gameId = 0;
	_gameText = NULL;
	do {
		if (!scumm_stricmp(_exe_name, gnl->filename)) {
			_gameId = gnl->id;
			_scummVersion = gnl->major;

			_features = gnl->features;
			_gameText = gnl->gamename;
			debug(1, "Detected game '%s', version %d.%d.%d",
						gnl->gamename, gnl->major, gnl->middle, gnl->minor);
			return true;
		}
	} while ((++gnl)->filename);

	debug(1, "Failed game detection");

	return true;
}

char *GameDetector::getGameName()
{
	if (_gameText == NULL) {
		char buf[256];
		sprintf(buf, "Unknown game: \"%s\"", _exe_name);
		return Scumm::Strdup(buf);
	}
	return Scumm::Strdup(_gameText);
}

int GameDetector::detectMain(int argc, char **argv)
{
	_debugMode = 0;								// off by default...

	_noSubtitles = 0;							// use by default - should this depend on soundtrack?

#ifndef _WIN32_WCE
	_gfx_mode = GFX_DOUBLESIZE;
#else
	_gfx_mode = GFX_NORMAL;
#endif
	_sfx_volume = 100;
	_music_volume = 60;

#if defined(USE_NULL_DRIVER)
	_gfx_driver = GD_NULL;
#elif defined(__DC__)
	_gfx_driver = GD_DC;
#elif defined(UNIX_X11)
	_gfx_driver = GD_X;
#elif defined(__MORPHOS__)
	_gfx_driver = GD_MORPHOS;
#elif defined(_WIN32_WCE)
	_gfx_driver = GD_WINCE;
#else
	/* SDL is the default driver for now */
	_gfx_driver = GD_SDL;
#endif

	_gameDataPath = NULL;
	_gameTempo = 0;
	_soundCardType = 3;



	_midi_driver = MD_AUTO;

#ifdef __DC__
	extern int dc_setup(GameDetector &detector);
	dc_setup(*this);
#else
	_saveconfig = false;
	updateconfig();
	parseCommandLine(argc, argv);	
#endif

	if (_exe_name == NULL) {
		//launcherLoop();
		//setWindowName(this);
		warning("No game was specified...");
		return (-1);
	}


	if (!detectGame()) {
		warning("Game detection failed. Using default settings");
		_features = GF_DEFAULT;
		_gameText = "Please choose a game";
	}

	/* Use the adlib sound driver if auto mode is selected,
	 * and the game is one of those that want adlib as
	 * default */
	if (_midi_driver == MD_AUTO && _features&GF_ADLIB_DEFAULT) {
		_use_adlib = true;
	}

	if (!_gameDataPath) {
		warning("No path was provided. Assuming the data files are in the current directory");
		_gameDataPath = Scumm::Strdup("");
	}

	if (_amiga)
		_features = _features | GF_AMIGA;

	return (0);
}

OSystem *GameDetector::createSystem() {
	/* auto is to use SDL */
	switch(_gfx_driver) {
#if defined(UNIX_X11)
	case GD_X:
		return OSystem_X11_create();
#elif defined(__DC__)
	case GD_DC:
		return OSystem_Dreamcast_create();
#elif defined(_WIN32_WCE)
	case GD_WINCE:
		return OSystem_WINCE3_create();
#elif defined(__MORPHOS__)
	case GD_MORPHOS:
		return OSystem_MorphOS_create(_gameId, _gfx_mode, _fullScreen);
#elif defined(USE_NULL_DRIVER)
	case GD_NULL:
		return OSystem_NULL_create();
#else
	case GD_SDL:
		return OSystem_SDL_create(_gfx_mode, _fullScreen);
#endif
	}

	error("Invalid graphics driver");
	return NULL;
}

MidiDriver *GameDetector::createMidi() {
	int drv = _midi_driver;

#if defined (WIN32) && !defined(_WIN32_WCE)
	/* MD_WINDOWS is default MidiDriver on windows targets */
	if (drv == MD_AUTO) drv = MD_WINDOWS;
#elif defined(__APPLE__)
	/* MD_QTMUSIC is default MidiDriver on MacOS targets */
	if (drv == MD_AUTO) drv = MD_QTMUSIC;
#endif

	switch(drv) {
	case MD_AUTO:
	case MD_NULL:		return MidiDriver_NULL_create();
#if defined(WIN32) && !defined(_WIN32_WCE)
	case MD_WINDOWS:	return MidiDriver_WIN_create();
#endif
#if defined(__MORPHOS__)
	case MD_AMIDI:		return MidiDriver_AMIDI_create();
#endif
#if defined(UNIX) && !defined(__BEOS__)
    case MD_SEQ:        return MidiDriver_SEQ_create();
#endif
#if defined(__APPLE__)
	case MD_QTMUSIC:		return MidiDriver_QT_create();
	case MD_COREAUDIO:		return MidiDriver_CORE_create();
#endif
	}

	error("Invalid midi driver selected");
	return NULL;
}
