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
#include "sound/mididrv.h"
#include "scumm/imuse.h"
#include "common/engine.h"
#include "common/gameDetector.h"
#include "common/config-file.h"


extern uint16 _debugLevel;


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
	"\t-a         - specify game is amiga version\n"
	"\t-b<num>    - start in room <num>\n"
	"\t-c<num>    - use cdrom <num> for cd audio\n"
	"\t-d[<num>]  - enable debug output (debug level [1])\n"
	"\t-e<mode>   - set music engine (see README for details)\n"
	"\t-f         - fullscreen mode\n"
	"\t-g<mode>   - graphics mode (normal,2x,3x,2xsai,super2xsai,supereagle,advmame2x)\n"
	"\t-l<file>   - load config file instead of default\n"
	"\t-m<num>    - set music volume to <num> (0-255)\n"
	"\t-n         - no subtitles for speech\n"
	"\t-o<num>    - set master volume to <num> (0-255)\n"
	"\t-p<path>   - look for game in <path>\n"
	"\t-s<num>    - set sfx volume to <num> (0-255)\n"
	"\t-t<num>    - set music tempo (default- adlib: 0x1D9000, midi: 0x4A0000)\n"
	"\t-v         - show version info and exit\n"
#if defined(UNIX)
	"\t-w[<file>] - write to config file [~/.scummvmrc]\n"
#else
	"\t-w[<file>] - write to config file [scummvm.ini]\n"
#endif
	"\t-x[<num>]  - save game slot to load (default: autosave)\n"
	"\t-y         - set text speed (default: 60)\n"
	"\t-z         - display list of games\n"
;


GameDetector::GameDetector()
{
	_fullScreen = false;
	_gameId = 0;

	_use_adlib = false;

	_master_volume = kDefaultMasterVolume;
	_music_volume = kDefaultMusicVolume;
	_sfx_volume = kDefaultSFXVolume;
	_amiga = false;

	_talkSpeed = 60;
	_debugMode = 0;
	_noSubtitles = false;
	_bootParam = 0;

	_gameDataPath = 0;
	_gameTempo = 0;
	_midi_driver = MD_AUTO;
	_features = 0;

	_cdrom = 0;
	_save_slot = 0;
	
	_saveconfig = false;
	
#ifndef _WIN32_WCE
	_gfx_mode = GFX_DOUBLESIZE;
#else
	_gfx_mode = GFX_NORMAL;
#endif

#if defined(USE_NULL_DRIVER)
	_gfx_driver = GD_NULL;
#elif defined(__DC__)
	_gfx_driver = GD_DC;
#elif defined(X11_BACKEND)
	_gfx_driver = GD_X;
#elif defined(__MORPHOS__)
	_gfx_driver = GD_MORPHOS;
#elif defined(_WIN32_WCE)
	_gfx_driver = GD_WINCE;
#elif defined(MACOS_CARBON)
	_gfx_driver = GD_MAC;
#elif defined(__GP32__)	// ph0x
	_gfx_driver = GD_GP32;
#else
	/* SDL is the default driver for now */
	_gfx_driver = GD_SDL;
#endif
}

void GameDetector::updateconfig()
{
	const char * val;

	_amiga = g_config->getBool("amiga", _amiga);

	_save_slot = g_config->getInt("save_slot", _save_slot);

	_cdrom = g_config->getInt("cdrom", _cdrom);

	if ((val = g_config->get("music_driver")))
		if (!parseMusicDriver(val)) {
			printf("Error in the config file: invalid music_driver.\n");
			printf(USAGE_STRING);
			exit(-1);
		}

	_fullScreen = g_config->getBool("fullscreen", _fullScreen);

	if ((val = g_config->get("gfx_mode")))
		if ((_gfx_mode = parseGraphicsMode(val)) == -1) {
			printf("Error in the config file: invalid gfx_mode.\n");
			printf(USAGE_STRING);
			exit(-1);
		}

	_master_volume = g_config->getInt("master_volume", _master_volume);

	_music_volume = g_config->getInt("music_volume", _music_volume);

	_noSubtitles = g_config->getBool("nosubtitles", _noSubtitles ? true : false);

	if ((val = g_config->get("path")))
		_gameDataPath = strdup(val);

	_sfx_volume = g_config->getInt("sfx_volume", _sfx_volume);

	// We use strtol for the tempo to allow it to be specified in hex.
	if ((val = g_config->get("tempo")))
		_gameTempo = strtol(val, NULL, 0);

	_talkSpeed = g_config->getInt("talkspeed", _talkSpeed);
}

void GameDetector::list_games()
{
	const VersionSettings *v = version_settings;
	char config[4] = "";

	printf("Game          SCUMM ver Full Title                                     Config\n"
	       "------------- --------- ---------------------------------------------- -------\n");

	while (v->filename && v->gamename) {
		if (g_config->has_domain(v->filename)) {
			strcpy(config, "Yes");
		}
		else {
			strcpy(config, "");
		}

		if (v->major != 99)
			printf("%-14s%d.%d.%d\t%-47s%s\n", v->filename,
		     	  v->major, v->middle, v->minor, v->gamename, config);
		else
			printf("%-14s%-7s\t%-47s%s\n", v->filename, "n/a", 
					v->gamename, config);

		v++;
	}
		
}

void GameDetector::parseCommandLine(int argc, char **argv)
{
	int i;
	char *s;
	char *current_option = NULL;
	char *option = NULL;
	char c;
	_save_slot = -1;

	/* Parse the arguments */
	// FIXME: Add more lemons
	for (i = argc - 1; i >= 1; i--) {
		s = argv[i];

		if (s[0] == '-') {
			s++;
			c = *s++;
			switch (tolower(c)) {
			case 'a':
				CHECK_OPTION();
				_amiga = (c == 'a');
				g_config->setBool("amiga", _amiga);
				break;
			case 'b':
				HANDLE_OPTION();
				_bootParam = atoi(option);
				break;
			case 'c':
				HANDLE_OPTION();
				_cdrom = atoi(option);
				g_config->setInt("cdrom", _cdrom);
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
				g_config->set("music_driver", option);
				break;
			case 'f':
				CHECK_OPTION();
				_fullScreen = (c == 'f');
				g_config->setBool("fullscreen", _fullScreen, "scummvm");
				break;
			case 'g':
				HANDLE_OPTION();
				_gfx_mode = parseGraphicsMode(option);
				if (_gfx_mode == -1)
					goto ShowHelpAndExit;
				g_config->set("gfx_mode", option, "scummvm");
				break;
			case 'l':
				HANDLE_OPTION();
				{
					Config * newconfig = new Config(option, "scummvm");
					g_config->merge_config(*newconfig);
					delete newconfig;
					updateconfig();
					break;
				}
				break;
			case 'm':
				HANDLE_OPTION();
				_music_volume = atoi(option);
				g_config->setInt("music_volume", _music_volume);
				break;
			case 'n':
				CHECK_OPTION();
				_noSubtitles = (c == 'n');
				g_config->setBool("nosubtitles", _noSubtitles ? true : false);
				break;
 			case 'o':
 				HANDLE_OPTION(); 
 				_master_volume = atoi(option); 
 				g_config->setInt("master_volume", _master_volume); 
 				break; 
			case 'p':
				HANDLE_OPTION();
				_gameDataPath = option;
				g_config->set("path", _gameDataPath);
				break;
			case 'r':
				HANDLE_OPTION();
				// Ignore -r for now, to ensure backward compatibility.
				break;
			case 's':
				HANDLE_OPTION();
				_sfx_volume = atoi(option);
				g_config->setInt("sfx_volume", _sfx_volume);
				break;
			case 't':
				HANDLE_OPTION();
				_gameTempo = strtol(option, 0, 0);
				g_config->set("tempo", option);
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
				g_config->set_writing(true);
				HANDLE_OPT_OPTION();
				if (option != NULL)
					g_config->set_filename(option);
				break;
			case 'x':
				_save_slot = 0;
				HANDLE_OPT_OPTION();
				if (option != NULL) {
					_save_slot = atoi(option);
					g_config->setInt("save_slot", _save_slot);
				}
				break;
			case 'y':
				HANDLE_OPTION();
				_talkSpeed = atoi(option);				
				g_config->setInt("talkspeed", _talkSpeed);
				break;
			case 'z':
				CHECK_OPTION();
				list_games();
				exit(1);
			default:
				goto ShowHelpAndExit;
			}
		} else {
			if (i == (argc - 1)) {
				setGame(s);
			} else {
				if (current_option == NULL)
					current_option = s;
				else
					goto ShowHelpAndExit;
			}
		}
	}
	
	if (!_gameFileName.isEmpty())
		g_config->flush();

	return;

 ShowHelpAndExit:
	printf(USAGE_STRING);
	exit(1);
}

void GameDetector::setGame(const String &name)
{
	_gameFileName = name;
	g_config->set_domain(name);
	g_config->rename_domain(name, "game-specific");
	g_config->rename_domain("game-specific", name);
	updateconfig();
}

int GameDetector::parseGraphicsMode(const char *s) {
	struct GraphicsModes {
		const char *name;
		int id;
	};

	const struct GraphicsModes gfx_modes[] = {
		{"normal",GFX_NORMAL},
		{"1x",GFX_NORMAL},
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
		{"seq",MD_SEQ},
		{"qt",MD_QTMUSIC},
		{"core",MD_COREAUDIO},
		{"etude",MD_ETUDE},
		{"midiemu",MD_MIDIEMU},
		{"alsa", MD_ALSA},
		{"adlib", MD_ADLIB},
	};

	const MusicDrivers *md = music_drivers;
	int i;

	_use_adlib = false;

	for(i=0; i!=ARRAYSIZE(music_drivers); i++,md++) {
		if (!scumm_stricmp(md->name, s)) {
			if (md->id == MD_ADLIB) {
				_use_adlib = true;
			}
			_midi_driver = md->id;
			return true;
		}
	}

	return false;
}


/*
        This is a list of all known SCUMM games. Commented games are not
        supported at this time */

const VersionSettings version_settings[] = {
	/* Scumm Version 1 */
//      {"maniac",      "Maniac Mansion (C64)",                         GID_MANIAC64, 1, 0, 0,},
//      {"zak",         "Zak McKracken and the Alien Mindbenders (C64)", GID_ZAK64, 1, 0, 0,},

	/* Scumm Version 2 */
//      {"maniac",      "Maniac Mansion", GID_MANIAC, 2, 0, 0,
//      GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALLING, "MANIACEX.EXE"},
//      {"zak",         "Zak McKracken and the Alien Mindbenders",      GID_ZAK,     2, 0, 0,
//      GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALLING, "ZAKEXE.EXE"},
//      {"indy3",       "Indiana Jones and the Last Crusade",           GID_INDY3,   2, 0, 0,},

	/* Scumm Version 3 */
	{"indy3", "Indiana Jones and the Last Crusade (256)", GID_INDY3_256, 3, 0, 22,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_OLD256 | GF_NO_SCALLING | GF_ADLIB_DEFAULT, "00.LFL"},
	{"zak256", "Zak McKracken and the Alien Mindbenders (256)", GID_ZAK256, 3, 0, 0,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_OLD256 | GF_AUDIOTRACKS | GF_NO_SCALLING, "00.LFL"},
	{"loom", "Loom", GID_LOOM, 3, 5, 40,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_OLD_BUNDLE | GF_16COLOR | GF_NO_SCALLING, "00.LFL"},

	/* Scumm Version 4 */
	{"monkeyEGA", "Monkey Island 1 (EGA)", GID_MONKEY_EGA, 4, 0, 67,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR},	// EGA version

	/* Scumm version 5 */
	{"monkeyVGA", "Monkey Island 1 (256 color Floppy version)", GID_MONKEY_VGA,  5, 0, 16,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_ADLIB_DEFAULT, "000.LFL"},
	{"loomcd", "Loom (256 color CD version)", GID_LOOM256, 5, 1, 42,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT, "000.LFL"},
	{"monkey", "Monkey Island 1", GID_MONKEY, 5, 2, 2,
	 GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT},
	{"monkey1", "Monkey Island 1 (alt)", GID_MONKEY, 5, 2, 2,
	 GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT},
	{"monkey2", "Monkey Island 2: LeChuck's revenge", GID_MONKEY2, 5, 2, 2,
	 GF_USE_KEY | GF_ADLIB_DEFAULT},
	{"atlantis", "Indiana Jones and the Fate of Atlantis", GID_INDY4, 5, 5, 0,
	 GF_USE_KEY | GF_ADLIB_DEFAULT},
	{"playfate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, 5, 0,
	 GF_USE_KEY | GF_ADLIB_DEFAULT},
	{"fate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, 5, 0,
	 GF_USE_KEY | GF_ADLIB_DEFAULT},

	/* Scumm Version 6 */
	{"puttputt", "Putt-Putt Joins The Parade (DOS)", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS},
	{"puttdemo", "Putt-Putt Joins The Parade (Demo)", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS},
	{"moondemo", "Putt-Putt Goes To The Moon (Demo)", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS},
	{"tentacle", "Day Of The Tentacle", GID_TENTACLE, 6, 4, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT},
	{"dottdemo", "Day Of The Tentacle (Demo)", GID_TENTACLE, 6, 3, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT},
	{"samnmax", "Sam & Max", GID_SAMNMAX, 6, 4, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER},
	{"samdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY  | GF_DRAWOBJ_OTHER_ORDER | GF_ADLIB_DEFAULT},
	{"snmdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY  | GF_DRAWOBJ_OTHER_ORDER | GF_ADLIB_DEFAULT},

	{"test", "Test demo game", GID_SAMNMAX, 6, 6, 6, GF_NEW_OPCODES | GF_AFTER_V6},

	/* Scumm Version 7 */
	{"ft", "Full Throttle", GID_FT, 7, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7},
	{"dig", "The Dig", GID_DIG, 7, 5, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7},

	/* Simon the Sorcerer 1 & 2 (not SCUMM games) */
	{"simon1dos", "Simon the Sorcerer 1 for DOS", GID_SIMON_FIRST+0, 99, 99, 99, 0, "GAMEPC"},
	{"simon2dos", "Simon the Sorcerer 2 for DOS", GID_SIMON_FIRST+1, 99, 99, 99, 0, "GAME32"},
	{"simon1talkie", "Simon the Sorcerer 1 Talkie for DOS", GID_SIMON_FIRST+4, 99, 99, 99, 0, "GAMEPC"},
	{"simon2talkie", "Simon the Sorcerer 2 Talkie for DOS", GID_SIMON_FIRST+5, 99, 99, 99, 0, "GSPTR30"},
	{"simon1win", "Simon the Sorcerer 1 Talkie for Windows", GID_SIMON_FIRST+6, 99, 99, 99, 0, "GAMEPC"},	
	{"simon2win", "Simon the Sorcerer 2 Talkie for Windows", GID_SIMON_FIRST+7, 99, 99, 99, 0, "GSPTR30"},
	{"simon1demo", "Simon the Sorcerer 1 for DOS (Demo)", GID_SIMON_FIRST+8, 99, 99, 99, 0, "GDEMO"}, 

	/* Scumm Version 8 */
	{"comi", "The Curse of Monkey Island", GID_CMI, 8, 1, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7 | GF_AFTER_V8},

	{NULL, NULL}
};


bool GameDetector::detectGame()
{
	const VersionSettings *gnl = version_settings;
	char *realGame;
	_gameId = 0;
	_gameText.clear();

	if (!(realGame = (char*)g_config->get("gameid")))
		realGame = (char*)_gameFileName.c_str();
	printf("Looking for %s\n", realGame);

	do {
		if (!scumm_stricmp(realGame, gnl->filename)) {
			_gameId = gnl->id;
			_gameRealName = gnl->filename;
			_features = gnl->features;
			_gameText = gnl->gamename;
			debug(1, "Detected game '%s', version %d.%d.%d",
						gnl->gamename, gnl->major, gnl->middle, gnl->minor);
			return true;
		}
	} while ((++gnl)->filename);

	debug(1, "Failed game detection");

	return false;
}

const ScummVM::String& GameDetector::getGameName()
{
	if (_gameText.isEmpty()) {
		_gameText = "Unknown game: \"";
		_gameText += _gameFileName;
		_gameText += "\"";
	}
	return _gameText;
}

int GameDetector::detectMain()
{
	if (_gameFileName.isEmpty()) {
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
	if (_midi_driver == MD_AUTO && _features & GF_ADLIB_DEFAULT) {
		_midi_driver = MD_ADLIB;
		_use_adlib = true;
	}

	if (!_gameDataPath) {
		warning("No path was provided. Assuming the data files are in the current directory");
		_gameDataPath = "";
	} else if (_gameDataPath[strlen(_gameDataPath)-1] != '/'
#ifdef __MORPHOS__
					&& _gameDataPath[strlen(_gameDataPath)-1] != ':'
#endif
					&& _gameDataPath[strlen(_gameDataPath)-1] != '\\') {
		char slashless[1024];	/* Append slash to path */
		strcpy(slashless, _gameDataPath);
		
		// need to allocate 2 extra bytes, one for the "/" and one for the NULL terminator
		_gameDataPath = (char *)malloc((strlen(slashless) + 2) * sizeof(char));
		sprintf(_gameDataPath, "%s/", slashless);
	}

	if (_amiga)
		_features = _features | GF_AMIGA;

	return (0);
}

OSystem *GameDetector::createSystem() {
	/* auto is to use SDL */
	switch(_gfx_driver) {
#if defined(X11_BACKEND)
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
#elif defined(MACOS_CARBON)
	case GD_MAC:
		return OSystem_MAC_create(_gfx_mode, _fullScreen);
#elif defined(USE_NULL_DRIVER)
	case GD_NULL:
		return OSystem_NULL_create();
#elif defined(__GP32__) //ph0x
	case GD_GP32:
		return OSystem_GP32_create(GFX_NORMAL, true);
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
#elif defined(MACOSX)
	if (drv == MD_AUTO) drv = MD_COREAUDIO;
#elif defined(macintosh)
	if (drv == MD_AUTO) drv = MD_QTMUSIC;
#elif defined(__MORPHOS__)
	if (drv == MD_AUTO) drv = MD_ETUDE;
#elif defined (_WIN32_WCE) || defined(UNIX) || defined(X11_BACKEND)
	/* Always use MIDI emulation via adlib driver on CE and UNIX devices */

	/* FIXME: We should, for the Unix targets, attempt to detect */
	/*        whether a sequencer is available, and use it in */
	/*	  preference */	
/*
	if (drv == MD_AUTO) {
		_use_adlib = true;
		return NULL;
	}
*/
	if (drv == MD_AUTO) drv = MD_ADLIB;
#endif

	switch(drv) {
	case MD_AUTO:
	case MD_NULL:		return MidiDriver_NULL_create();
	case MD_ADLIB:		_use_adlib = true; return MidiDriver_ADLIB_create();
#if defined(WIN32) && !defined(_WIN32_WCE)
	case MD_WINDOWS:	return MidiDriver_WIN_create();
#endif
#if defined(__MORPHOS__)
	case MD_ETUDE:		return MidiDriver_ETUDE_create();
#endif
#if defined(UNIX) && !defined(__BEOS__)
	case MD_SEQ:		return MidiDriver_SEQ_create();
#endif
#if defined(MACOSX) || defined(macintosh)
	case MD_QTMUSIC:	return MidiDriver_QT_create();
#endif
#if defined(MACOSX)
	case MD_COREAUDIO:	return MidiDriver_CORE_create();
#endif
#if defined(UNIX) && defined(USE_ALSA)
	case MD_ALSA:		return MidiDriver_ALSA_create();
#endif
	}

	error("Invalid midi driver selected");
	return NULL;
}
