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



static const char USAGE_STRING[] = 
	"ScummVM - Scumm Interpreter\n"
	"Syntax:\n"
	"\tscummvm [-v] [-d[<num>]] [-n] [-b<num>] [-t<num>] [-s<num>] [-p<path>] [-m<num>] [-f] game\n"
	"Flags:\n"
	"\tv       - show version info and exit\n"
	"\tc<num>  - use cdrom <num> for cd audio\n"
	"\td[<num>]- enable debug output (level <num>)\n"
	"\tn       - no subtitles for speech\n"
	"\tb<num>  - start in room <num>\n"
	"\tt<num>  - set music tempo. Suggested: 1F0000\n"
	"\tp<path> - look for game in <path>\n"
	"\tm<num>  - set music volume to <num> (0-100)\n"
	"\ts<num>  - set sfx volume to <num> (0-255)\n"
	"\te<mode> - set music engine. see readme.txt for details\n"
	"\tr       - emulate roland mt32 instruments\n"
	"\tf       - fullscreen mode\n"
	"\tg<mode> - graphics mode. normal,2x,3x,2xsai,super2xsai,supereagle.advmame2x\n"
	"\ta       - specify game is amiga version\n"
;

void GameDetector::parseCommandLine(int argc, char **argv)
{
#if !defined(__APPLE__CW)
	int i;
	char *s;

	// check for arguments
	if (argc < 2) {
		printf(USAGE_STRING);
		//exit(1);
	}

	/* Parse the arguments */
	for (i = 1; i < argc; i++) {
		s = argv[i];

		if (s && s[0] == '-') {
			s++;
			while (*s) {
				switch (tolower(*s)) {
				case 'a':
					_amiga = true;
					break;
				case 'b':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_bootParam = atoi(s + 1);
					goto NextArg;
				case 'f':
					_fullScreen = true;
					break;
				case 'd':
					_debugMode = true;
					if (*(s+1) != '\0')
						_debugLevel = atoi(s+1);
					debug(1,"Debugmode (level %d) on", _debugLevel);
					goto NextArg;
				case 'n':
					_noSubtitles = true;
					break;
				case 'v':
					printf("ScummVM " SCUMMVM_VERSION "\nBuilt on " __DATE__ " "
								 __TIME__ "\n");
#ifdef SCUMMVM_PLATFORM_VERSION
					printf("    " SCUMMVM_PLATFORM_VERSION "\n");
#endif
					exit(1);
				case 'p':
					if (*(s + 1) == '\0') {
						if (i+1 < argc && argv[i+1] && argv[i+1][0] != '-' && argv[i+1][0] != '\0') {
							i++;
							_gameDataPath = argv[i];
						} else
							goto ShowHelpAndExit;
					} else
						_gameDataPath = s + 1;
 					goto NextArg; 				
				case 't':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_gameTempo = atoi(s + 1);
					goto NextArg;
				case 'm':{
						if (*(s + 1) == '\0')
							goto ShowHelpAndExit;
						_music_volume = atoi(s + 1);
						goto NextArg;
					}
				case 's':{
						if (*(s + 1) == '\0')
							goto ShowHelpAndExit;
						_sfx_volume = atoi(s + 1);
						goto NextArg;
					}
				case 'r':{
						_mt32emulate = true;
						break;
					}
				case 'e':
					if (!parseMusicDriver(s+1))
						goto ShowHelpAndExit;
					goto NextArg;
				case 'g': {
						int gfx_mode = parseGraphicsMode(s+1);
						if (gfx_mode == -1)
							goto ShowHelpAndExit;
						_gfx_mode = gfx_mode;
					}
					goto NextArg;
				case 'c':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_cdrom = atoi(s + 1);
					goto NextArg;

				default:
				ShowHelpAndExit:;
					printf(USAGE_STRING);
					exit(1);
				}
				s++;
			}
		NextArg:;
		} else {
			if (_exe_name)
				goto ShowHelpAndExit;
			_exe_name = s;
		}
	}

#else
	_midi_driver = 4; /* FIXME: don't use numerics */
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
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER | GF_ADLIB_DEFAULT},
	{"snmdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY},

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
		return strdup(buf);
	}
	return strdup(_gameText);
}

int GameDetector::detectMain(int argc, char **argv)
{
	_debugMode = 0;								// off by default...

	_noSubtitles = 0;							// use by default - should this depend on soundtrack?        

	_gfx_mode = GFX_DOUBLESIZE;
	_sfx_volume = 100;

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
		_gameDataPath = strdup("");
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
