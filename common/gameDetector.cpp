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
#include "common/config-file.h"
#include "common/engine.h"
#include "common/gameDetector.h"
#include "common/scaler.h"	// Only for gfx_modes
#include "sound/mididrv.h"

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
#ifdef __PALM_OS__
static const char USAGE_STRING[] = "NoUsageString"; // save more data segment space
#else
static const char USAGE_STRING[] = 
	"ScummVM - Graphical Adventure Game Interpreter\n"
	"Syntax:\n"
	"\tscummvm [OPTIONS] [game]\n"
	"Options:\n"
	"\t-p<path>       - Look for game in <path>\n"
	"\t-x[<num>]      - Load this savegame (default: 0 - autosave)\n"
	"\t-f             - Full-screen mode (-F forces window mode.)\n"
	"\t-g<mode>       - Graphics mode (normal,2x,3x,2xsai,super2xsai,\n"
	"\t                 supereagle,advmame2x, advmame3x,tv2x,dotmatrix)\n"
	"\t-e<mode>       - Set music engine (see README for details)\n"
	"\t-q<lang>       - Specify language (en,de,fr,it,pt,es,jp,zh,kr,se,\n"
	"\t                 gb,hb)\n"
	"\n"
	"\t-c<num>        - Use cdrom <num> for cd audio\n"
	"\t-m<num>        - Set music volume to <num> (0-255)\n"
	"\t-o<num>        - Set master volume to <num> (0-255)\n"
	"\t-s<num>        - Set sfx volume to <num> (0-255)\n"
	"\t-t<num>        - Set music tempo (50-200, default 100%%)\n"
	"\n"
	"\t-n             - No subtitles for speech\n"
	"\t-y             - Set text speed (default: 60)\n"
	"\n"
	"\t-l<file>       - Load config file instead of default\n"
#if defined(UNIX)
	"\t-w[<file>]     - Write to config file [~/.scummvmrc]\n"
#else
	"\t-w[<file>]     - Write to config file [scummvm.ini]\n"
#endif
	"\t-v             - Show version info and exit\n"
	"\t-h             - Display this text and exit\n"
	"\t-z             - Display list of games\n"
	"\n"
	"\t-b<num>        - Pass number to the boot script (boot param)\n"
	"\t-d[<num>]      - Enable debug output (debug level [1])\n"
	"\t-u             - Dump scripts\n"
	"\n"
	"\t--platform=    - Specify version of game (amiga,atari-st,macintosh)\n"
	"\t--multi-midi   - Enable combination Adlib and native MIDI\n"
	"\t--native-mt32  - True Roland MT-32 (disable GM emulation)\n"
	"\t--fullscreen   - Full-screen mode (same as -f)\n"
	"\t--aspect-ratio - Enable aspect ratio correction\n"
#ifndef DISABLE_SCUMM
	"\t--demo-mode    - Start demo mode of Maniac Mansion (Classic version)\n"
#endif
#ifndef DISABLE_SKY
	"\t--floppy-intro - Use floppy version intro for Beneath a Steel Sky CD\n"
#endif
	"\n"
	"The meaning of long options can be inverted by prefixing them with \"no-\",\n"
	"e.g. \"--no-aspect-ratio\".\n"
;
#endif
// This contains a pointer to a list of all supported games.
const VersionSettings *version_settings = NULL;

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

static const struct Language languages[] = {
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
	{0, 0, 0}
};

static const struct MusicDriver music_drivers[] = {
	{"auto", "Default", MD_AUTO},
	{"null", "No music", MD_NULL},
#ifndef __PALM_OS__	// reduce contant data size
	{"windows", "Windows MIDI", MD_WINDOWS},
	{"seq", "SEQ", MD_SEQ},
	{"qt", "QuickTime", MD_QTMUSIC},
	{"core", "CoreAudio", MD_COREAUDIO},
	{"etude", "Etude", MD_ETUDE},
	{"alsa", "ALSA", MD_ALSA},
	{"adlib", "Adlib", MD_ADLIB},
	{"pcspk", "PC Speaker", MD_PCSPK},
	{"pcjr", "IBM PCjr", MD_PCJR},
#else
	{"ypa1", "Yamaha Pa1", MD_YPA1},
#endif
	{0, 0, 0}
};

static int countVersions(const VersionSettings *v) {
	int count;
	for (count = 0; v->filename; v++, count++)
		;
	return count;
}

GameDetector::GameDetector() {
	_fullScreen = false;
	_aspectRatio = false;

	_master_volume = kDefaultMasterVolume;
	_music_volume = kDefaultMusicVolume;
	_sfx_volume = kDefaultSFXVolume;
	_amiga = false;
	_platform = 0;
	_language = 0;

#ifndef DISABLE_SCUMM
	_demo_mode = false;
#endif

#ifndef DISABLE_SKY
	_floppyIntro = false;
#endif

	_talkSpeed = 60;
	_debugMode = 0;
	_debugLevel = 0;
	_dumpScripts = 0;
	_noSubtitles = false;
	_bootParam = 0;

	_gameDataPath = 0;
	_gameTempo = 0;
	_midi_driver = MD_AUTO;
	_game.id = 0;
	_game.features = 0;

	_multi_midi = false;
	_native_mt32 = false;

	_cdrom = 0;
	_save_slot = 0;
	
	_saveconfig = false;
	_confirmExit = false;
	
#ifndef _WIN32_WCE
	_gfx_mode = GFX_DOUBLESIZE;
#else
	_gfx_mode = GFX_NORMAL;
#endif
	_default_gfx_mode = true;

	if (version_settings == NULL) {
		int totalCount = 0;
		
		// Gather & combine the target lists from the modules

#ifndef DISABLE_SCUMM
		const VersionSettings *scummVersions = Engine_SCUMM_targetList();
		int scummCount = countVersions(scummVersions);
		totalCount += scummCount;
#endif

#ifndef DISABLE_SIMON
		const VersionSettings *simonVersions = Engine_SIMON_targetList();
		int simonCount = countVersions(simonVersions);
		totalCount += simonCount;
#endif

#ifndef DISABLE_SKY
		const VersionSettings *skyVersions = Engine_SKY_targetList();
		int skyCount = countVersions(skyVersions);
		totalCount += skyCount;
#endif

#ifndef DISABLE_SWORD2
		const VersionSettings *sword2Versions = Engine_SWORD2_targetList();
		int sword2Count = countVersions(sword2Versions);
		totalCount += sword2Count;
#endif
		
		VersionSettings *v = (VersionSettings *)calloc(totalCount + 1, sizeof(VersionSettings));
		version_settings = v;

#ifndef DISABLE_SCUMM
		memcpy(v, scummVersions, scummCount * sizeof(VersionSettings));
		v += scummCount;
#endif

#ifndef DISABLE_SIMON
		memcpy(v, simonVersions, simonCount * sizeof(VersionSettings));
		v += simonCount;
#endif

#ifndef DISABLE_SKY
		memcpy(v, skyVersions, skyCount * sizeof(VersionSettings));
		v += skyCount;
#endif

#ifndef DISABLE_SWORD2
		memcpy(v, sword2Versions, sword2Count * sizeof(VersionSettings));
		v += sword2Count;
#endif

	}
}

#ifdef __PALM_OS__
GameDetector::~GameDetector() {
	// This is a previously allocated chunck (line 224)
	// so we need to free it to prevent memory leak
	VersionSettings *v = (VersionSettings *)version_settings;
	free(v);
}
#endif

void GameDetector::updateconfig() {
	const char *val;

	_amiga = g_config->getBool("amiga", _amiga);

	_platform = g_config->getInt("platform", _platform);

	_save_slot = g_config->getInt("save_slot", _save_slot);

	_cdrom = g_config->getInt("cdrom", _cdrom);

	if ((val = g_config->get("music_driver")))
		if (!parseMusicDriver(val)) {
			printf("Error in the config file: invalid music_driver.\n");
			printf(USAGE_STRING);
			exit(-1);
		}

	_fullScreen = g_config->getBool("fullscreen", _fullScreen);
	_aspectRatio = g_config->getBool("aspect_ratio", _aspectRatio);

	if ((val = g_config->get("gfx_mode")))
		if ((_gfx_mode = parseGraphicsMode(val)) == -1) {
			printf("Error in the config file: invalid gfx_mode.\n");
			printf(USAGE_STRING);
			exit(-1);
		}

#ifndef DISABLE_SKY
	_floppyIntro = g_config->getBool("floppy_intro", _floppyIntro);
#endif

#ifndef DISABLE_SCUMM
	_demo_mode = g_config->getBool("demo_mode", _demo_mode);
#endif

	if ((val = g_config->get("language")))
		if ((_language = parseLanguage(val)) == -1) {
			printf("Error in the config file: invalid language.\n");
			printf(USAGE_STRING);
			exit(-1);
		}

	_master_volume = g_config->getInt("master_volume", _master_volume);

	_music_volume = g_config->getInt("music_volume", _music_volume);

	_noSubtitles = g_config->getBool("nosubtitles", _noSubtitles ? true : false);

	if ((val = g_config->get("path")))
		_gameDataPath = strdup(val);

	_sfx_volume = g_config->getInt("sfx_volume", _sfx_volume);

	_debugLevel = g_config->getInt("debuglevel", _debugLevel);
	if (_debugLevel > 0)
		_debugMode = true;

	// We use strtol for the tempo to allow it to be specified in hex.
	if ((val = g_config->get("tempo")))
		_gameTempo = strtol(val, NULL, 0);

	_talkSpeed = g_config->getInt("talkspeed", _talkSpeed);

	_confirmExit = g_config->getBool("confirm_exit", _confirmExit ? true : false);

	_multi_midi = g_config->getBool ("multi_midi", _multi_midi);
	_native_mt32 = g_config->getBool ("native_mt32", _native_mt32);
}

void GameDetector::list_games() {
	const VersionSettings *v = version_settings;
	const char *config;

	printf("Game             Full Title                                             Config\n"
	       "---------------- ------------------------------------------------------ -------\n");

	while (v->filename && v->gamename) {
		config = (g_config->has_domain(v->filename)) ? "Yes" : "";
		printf("%-17s%-56s%s\n", v->filename, v->gamename, config);
		v++;
	}
		
}

void GameDetector::parseCommandLine(int argc, char **argv) {
	int i;
	char *s;
	char *current_option = NULL;
	char *option = NULL;
	char c;
	bool long_option_value;
	_save_slot = -1;

	// Parse the arguments
	// into a transient "_COMMAND_LINE" config comain.
	g_config->set_domain ("_COMMAND_LINE");
	for (i = argc - 1; i >= 1; i--) {
		s = argv[i];

		if (s[0] == '-') {
			s++;
			c = *s++;
			switch (tolower(c)) {
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
				debug(1,"Debuglevel (from command line): %d", _debugLevel);
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
				g_config->setBool("fullscreen", _fullScreen);
				g_config->setBool("fullscreen", _fullScreen, "scummvm");
				break;
			case 'g':
				HANDLE_OPTION();
				_gfx_mode = parseGraphicsMode(option);
				if (_gfx_mode == -1)
					goto ShowHelpAndExit;
				g_config->set("gfx_mode", option);
				g_config->set("gfx_mode", option, "scummvm");
				break;
			// case 'h': reserved for help
			// case 'j': reserved for joystick select
			case 'l':
				HANDLE_OPTION();
				{
					Config *newconfig = new Config(option, "scummvm");
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
			case 'q':
				HANDLE_OPTION();
				_language = parseLanguage(option);
				if (_language == -1)
					goto ShowHelpAndExit;
				g_config->set("language", option);
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
			case 'u':
				CHECK_OPTION();
				_dumpScripts = true;
				break;
			case 'v':
				CHECK_OPTION();
				printf("%s\n", gScummVMFullVersion);
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
			case '-':
				// Long options. Let the fun begin!
				if (!strncmp(s, "platform=", 9)) {
					s += 9;
					if (!strcmp (s, "amiga"))
						_platform = 1;
					else if (!strcmp (s, "atari-st"))
						_platform = 2;
					else if (!strcmp (s, "macintosh"))
						_platform = 3;
					else
						goto ShowHelpAndExit;

					g_config->setInt ("platform", _platform);
					break;
				} 

				if (!strncmp(s, "no-", 3)) {
					long_option_value = false;
					s += 3;
				} else
					long_option_value = true;

				if (!strcmp (s, "multi-midi")) {
					_multi_midi = long_option_value;
					g_config->setBool ("multi_midi", _multi_midi);
				} else if (!strcmp (s, "native-mt32")) {
					_native_mt32 = long_option_value;
					g_config->setBool ("native_mt32", _native_mt32);
				} else if (!strcmp (s, "aspect-ratio")) {
					_aspectRatio = long_option_value;
					g_config->setBool ("aspect_ratio", _aspectRatio);
				} else if (!strcmp (s, "fullscreen")) {
					_fullScreen = long_option_value;
					g_config->setBool("fullscreen", _fullScreen);
					g_config->setBool("fullscreen", _fullScreen, "scummvm");
#ifndef DISABLE_SCUMM
				} else if (!strcmp (s, "demo-mode")) {
					_demo_mode = long_option_value;
					g_config->setBool ("demo_mode", _demo_mode);
#endif

#ifndef DISABLE_SKY
				} else if (!strcmp (s, "floppy-intro")) {
					_floppyIntro = long_option_value;
					g_config->setBool ("floppy_intro", _floppyIntro);
#endif
				} else {
					goto ShowHelpAndExit;
				}
				break;
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

void GameDetector::setGame(const String &name) {
	_gameFileName = name;
	g_config->set_domain(name);
	g_config->rename_domain(name, "game-specific");
	g_config->rename_domain("game-specific", name);
	updateconfig();

	// The command line and launcher options
	// override config file global and game-specific options.
	g_config->set_domain("_COMMAND_LINE");
	updateconfig();
	g_config->set_domain("_USER_OVERRIDES");
	updateconfig();
	g_config->delete_domain("_COMMAND_LINE");
	g_config->delete_domain("_USER_OVERRIDES");
	g_config->set_domain(name);
	debug(1, "Debuglevel (from config): %d", _debugLevel);
}

int GameDetector::parseGraphicsMode(const char *s) {
	const GraphicsMode *gm = gfx_modes;
	while(gm->name) {
		if (!scumm_stricmp(gm->name, s)) {
			_default_gfx_mode = false;
			return gm->id;
		}
		gm++;
	}

	return -1;
}

int GameDetector::parseLanguage(const char *s) {
	const Language *l = languages;
	while(l->name) {
		if (!scumm_stricmp(l->name, s))
			return l->id;
		l++;
	}

	return -1;
}

bool GameDetector::isMusicDriverAvailable(int drv) {
	switch(drv) {
	case MD_AUTO:
	case MD_NULL: return true;
#ifndef __PALM_OS__	// don't show it on palmos
	case MD_ADLIB:
	case MD_PCSPK:
	case MD_PCJR:  return true;
#else
	case MD_YPA1: return true;
#endif
#if defined(WIN32) && !defined(_WIN32_WCE)
	case MD_WINDOWS: return true;
#endif
#if defined(__MORPHOS__)
	case MD_ETUDE: return true;
#endif
#if defined(UNIX) && !defined(__BEOS__) && !defined(MACOSX)
	case MD_SEQ: return true;
#endif
#if defined(MACOSX) || defined(macintosh)
	case MD_QTMUSIC: return true;
#endif
#if defined(MACOSX)
	case MD_COREAUDIO: return true;
#endif
#if defined(UNIX) && defined(USE_ALSA)
	case MD_ALSA: return true;
#endif
	}
	return false;
}

const MusicDriver *GameDetector::getMusicDrivers() {
	return music_drivers;
}


bool GameDetector::parseMusicDriver(const char *s) {
	const MusicDriver *md = music_drivers;

	while (md->name) {
		if (!scumm_stricmp(md->name, s)) {
			_midi_driver = md->id;
			return true;
		}
		md++;
	}

	return false;
}

bool GameDetector::detectGame() {
	const VersionSettings *gnl = version_settings;
	const char *realGame, *basename;
	_game.id = 0;
	_gameText.clear();

	realGame = g_config->get("gameid");
	if (!realGame)
		realGame = _gameFileName.c_str();
	printf("Looking for %s\n", realGame);

	do {
		if (!scumm_stricmp(realGame, gnl->filename)) {
			_game = *gnl;
			if ((basename = g_config->get("basename")))	{
				// FIXME: What is this good for?
				_game.filename = basename;
			}
			_gameText = gnl->gamename;
			printf("Trying to start game '%s'\n",gnl->gamename);
			return true;
		}
	} while ((++gnl)->filename);

	printf("Failed game detection\n");

	return false;
}

const ScummVM::String& GameDetector::getGameName() {
	if (_gameText.isEmpty()) {
		_gameText = "Unknown game: \"";
		_gameText += _gameFileName;
		_gameText += "\"";
	}
	return _gameText;
}

int GameDetector::detectMain() {
	if (_gameFileName.isEmpty()) {
		warning("No game was specified...");
		return (-1);
	}

	if (!detectGame()) {
		warning("Game detection failed. Using default settings");
		_gameText = "Please choose a game";
	}

	/* Use the adlib sound driver if auto mode is selected,
	 * and the game is one of those that want adlib as
	 * default, OR if the game is an older game that doesn't
	 * support anything else anyway. */
	if (_midi_driver == MD_AUTO) {
		if (_game.midi & MDT_PREFER_NATIVE)
			_midi_driver = getMidiDriverType();
		else
			_midi_driver = MD_ADLIB;
	}
	bool nativeMidiDriver =
		(_midi_driver != MD_NULL && _midi_driver != MD_ADLIB &&
		 _midi_driver != MD_PCSPK && _midi_driver != MD_PCJR);
	if (nativeMidiDriver && !(_game.midi & MDT_NATIVE))
		_midi_driver = MD_ADLIB;
	if (_midi_driver == MD_ADLIB && !(_game.midi & MDT_ADLIB))
		_midi_driver = MD_PCJR;
	if ((_midi_driver == MD_PCSPK || _midi_driver == MD_PCJR) && !(_game.midi & MDT_PCSPK))
		_midi_driver = MD_NULL;

	if (!_gameDataPath) {
		warning("No path was provided. Assuming the data files are in the current directory");
		_gameDataPath = strdup("");
#ifndef __PALM_OS__	// add last slash also in File::fopenNoCase, so this is not needed
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
#endif
	}

	return (0);
}

OSystem *GameDetector::createSystem() {
#if defined(USE_NULL_DRIVER)
	return OSystem_NULL_create();
#elif defined(__DC__)
	return OSystem_Dreamcast_create();
#elif defined(X11_BACKEND)
	return OSystem_X11_create();
#elif defined(__MORPHOS__)
	return OSystem_MorphOS_create(_game.id, _gfx_mode, _fullScreen);
#elif defined(_WIN32_WCE)
	return OSystem_WINCE3_create();
#elif defined(MACOS_CARBON)
	return OSystem_MAC_create(_gfx_mode, _fullScreen);
#elif defined(__GP32__)	// ph0x
	return OSystem_GP32_create(GFX_NORMAL, true);
#elif defined(__PALM_OS__) //chrilith
	return OSystem_PALMOS_create(_gfx_mode, _fullScreen);
#else
	/* SDL is the default driver for now */
	return OSystem_SDL_create(_gfx_mode, _fullScreen, _aspectRatio);
#endif
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

MidiDriver *GameDetector::createMidi() {
	int drv = getMidiDriverType();

	switch(drv) {
	case MD_NULL:      return MidiDriver_NULL_create();
	// In the case of Adlib, we won't specify anything.
	// IMuse is designed to set up its own Adlib driver
	// if need be, and we only have to specify a native
	// driver.
	case MD_ADLIB:     return NULL;

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
