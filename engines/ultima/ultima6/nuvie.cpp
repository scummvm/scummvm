/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "Actor.h"
#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/ultima6/conf/configuration.h"
#include "U6misc.h"
#include "NuvieIOFile.h"
#include "Screen.h"
#include "Script.h"
#include "Game.h"
#include "GameSelect.h"
#include "GUI.h"
#include "Console.h"
#include "SoundManager.h"

#include "nuvie.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace Ultima {

Nuvie::Nuvie() {
	config = NULL;
	screen = NULL;
	script = NULL;
	game = NULL;
}

Nuvie::~Nuvie() {
	if (config != NULL)
		delete config;

	if (screen != NULL)
		delete screen;

	if (script != NULL)
		delete script;

	if (game != NULL)
		delete game;
}


bool Nuvie::init(int argc, char **argv) {
	GameSelect *game_select;
	uint8 game_type;
	bool play_ending = false;
	bool show_virtue_msg = false;
	bool reset_video = false;

	if (argc > 1) {
		if (strcmp(argv[1], "--reset-video") == 0) {
			reset_video = true;
			game_type = NUVIE_GAME_NONE;
		} else
			game_type = get_game_type(argv[1]);
	} else
		game_type = NUVIE_GAME_NONE;

	if (argc > 2) {
		if (strcmp(argv[2], "--end") == 0) {
			if (argc > 6 && strcmp(argv[3], "5") == 0) {
				play_ending = true;
			} else {
				show_virtue_msg = true;
			}
		} else if (strcmp(argv[2], "--reset-video") == 0)
			reset_video = true;
	}
//find and load config file
	if (initConfig() == false) {
		DEBUG(0, LEVEL_ERROR, "No config file found!\n");
		return false;
	} else if (reset_video) {
		set_safe_video_settings();
		config->write();
	}
//load SDL screen and scaler if selected.
	screen = new Screen(config);

	if (screen->init() == false) {
		DEBUG(0, LEVEL_ERROR, "Initializing screen!\n");
		return false;
	}

#if !SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_WM_SetCaption("Nuvie", "Nuvie");
#endif

	GUI *gui = new GUI(config, screen);



	ConsoleInit(config, screen, gui, 320, 200);
	ConsoleAddInfo("\n Nuvie: ver 0.5 rev 1927 \n");
	ConsoleAddInfo("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");

	if (show_virtue_msg) {
		ConsoleShow();
		ConsoleAddInfo("");
		ConsoleAddInfo("");
		ConsoleAddInfo("\"The path to victory is marked with the laws of");
		ConsoleAddInfo(" Virtue, not the commands of DOS.\"");
		ConsoleAddInfo("");
		ConsoleAddInfo("     -- Lord British");
		return false;
	}

	ConsoleAddInfo("Config file: \"%s\"", config->filename().c_str());
	game_select = new GameSelect(config);

// select game from graphical menu if required
	game_type = game_select->load(screen, game_type);
	delete game_select;
	if (game_type == NUVIE_GAME_NONE)
		return false;


//setup various game related config variables.
	assignGameConfigValues(game_type);

//check for a valid path to the selected game.
	if (checkGameDir(game_type) == false)
		return false;

	if (checkDataDir() == false)
		return false;

	SoundManager *sound_manager = new SoundManager();
	sound_manager->nuvieStartup(config);

	game = new Game(config, screen, gui, game_type, sound_manager);

	script = new Script(config, gui, sound_manager, game_type);
	if (script->init() == false)
		return false;

	if (play_ending) {
		script->play_cutscene("/ending.lua");
		return false;
	}

	if (playIntro() == false) {
		ConsoleDelete();
		return false;
	}

	if (game->loadGame(script) == false) {
		delete game;
		return false;
	}

	ConsoleDelete();

	return true;
}

bool Nuvie::play() {

	if (game)
		game->play();

	return true;
}

const char *Nuvie::getConfigPathWin32() {
	static char configFile[MAXPATHLEN];
	configFile[0] = '\0';
#ifdef WIN32

	OSVERSIONINFO win32OsVersion;
	ZeroMemory(&win32OsVersion, sizeof(OSVERSIONINFO));
	win32OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&win32OsVersion);
	// Check for non-9X version of Windows.
	if (win32OsVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
		// Use the Application Data directory of the user profile.
		if (win32OsVersion.dwMajorVersion >= 5) {
			if (!GetEnvironmentVariable("APPDATA", configFile, sizeof(configFile)))
				DEBUG(0, LEVEL_ERROR, "Unable to access application data directory\n");

		} else {
			if (!GetEnvironmentVariable("USERPROFILE", configFile, sizeof(configFile)))
				DEBUG(0, LEVEL_ERROR, "Unable to access user profile directory\n");

			strcat(configFile, "\\Application Data");

			// If the directory already exists (as it should in most cases),
			// we don't want to fail, but we need to stop on other errors (such as ERROR_PATH_NOT_FOUND)
			if (!CreateDirectory(configFile, NULL)) {
				if (GetLastError() != ERROR_ALREADY_EXISTS)
					DEBUG(0, LEVEL_ERROR, "Cannot create Application data folder\n");
			}
		}

		strcat(configFile, "\\Nuvie");
		if (!CreateDirectory(configFile, NULL)) {
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				DEBUG(0, LEVEL_ERROR, "Cannot create Nuvie application data folder\n");
		}

		strcat(configFile, "\\" "nuvie.cfg");
	}
#endif
	return configFile;
}

bool Nuvie::initConfig() {
	std::string config_path;


	config = new Configuration();

#ifdef WIN32
	const char *configFilePath = getConfigPathWin32();
	config_path.assign(configFilePath);
	if (loadConfigFile(config_path))
		return true;
#endif

#ifndef WIN32
// ~/.nuvierc

	char *home_env = getenv("HOME");
	if (home_env != NULL) {
		config_path.assign(home_env);
		// config_path.append(U6PATH_DELIMITER);
		config_path.append("/.nuvierc");

		if (loadConfigFile(config_path))
			return true;

#ifdef MACOSX
		config_path.assign(home_env);
		config_path.append("/Library/Preferences/Nuvie Preferences");

		if (loadConfigFile(config_path))
			return true;
#endif

	}
#endif

// nuvie.cfg in the working dir

	config_path.assign("nuvie.cfg");

	if (loadConfigFile(config_path))
		return true;

#ifndef WIN32
// standard share locations

	config_path.assign("/usr/local/share/nuvie/nuvie.cfg");

	if (loadConfigFile(config_path))
		return true;

	config_path.assign("/usr/share/nuvie/nuvie.cfg");

	if (loadConfigFile(config_path))
		return true;
#endif

#ifdef MACOSX
	if (home_env != NULL) {
		if (initDefaultConfigMacOSX(home_env))
			return true;
	}
#endif

#ifdef WIN32
	if (initDefaultConfigWin32())
		return true;
#else //Unix
	if (home_env != NULL && initDefaultConfigUnix(home_env))
		return true;
#endif

	delete config;
	config = NULL;

	return false;
}

void Nuvie::SharedDefaultConfigValues() {
	config->set("config/loadgame", "ultima6");
	config->set("config/datadir", "./data");
	config->set("config/keys", "(default)");

	set_safe_video_settings();
	config->set("config/video/game_style", "original");
	config->set("config/video/game_position", "center");

	config->set("config/audio/enabled", true);
	config->set("config/audio/enable_music", true);
	config->set("config/audio/enable_sfx", true);
	config->set("config/audio/music_volume", 100);
	config->set("config/audio/sfx_volume", 255);
	config->set("config/audio/combat_changes_music", true);
	config->set("config/audio/vehicles_change_music", true);
	config->set("config/audio/conversations_stop_music", false); // original stopped music - maybe due to memory and disk swapping
	config->set("config/audio/stop_music_on_group_change", true);

#ifdef HAVE_JOYSTICK_SUPPORT
	config->set("config/joystick/enable_joystick", false);
	config->set("config/joystick/repeat_hat", false);
	config->set("config/joystick/repeat_delay", 50);

	uint8 axisx[] = { 0, 3, 4, 6 };
	uint8 axisy[] = { 1, 2, 5, 7 };
	std::string axes_str = "config/joystick/axes_pair";
	std::string pair_str[] = { axes_str + "1", axes_str + "2", axes_str + "3", axes_str + "4" };
	for (int i = 0; i < 4; i++) {
		config->set(pair_str[i] + "/x_axis", axisx[i]);
		config->set(pair_str[i] + "/y_axis", axisy[i]);
		config->set(pair_str[i] + "/delay", 110);
		config->set(pair_str[i] + "/x_deadzone", 8000);
		config->set(pair_str[i] + "/y_deadzone", 8000);
	}
#endif

	config->set("config/input/enable_doubleclick", true);
	config->set("config/input/doubleclick_opens_containers", false);
	config->set("config/input/party_view_targeting", false);
	config->set("config/input/new_command_bar", false);
	config->set("config/input/enabled_dragging", true);
	config->set("config/input/look_on_left_click", true);
	config->set("config/input/walk_with_left_button", true);
	config->set("config/input/direction_selects_target", true);
	config->set("config/input/interface", "normal");

	config->set("config/general/lighting", "original");
	config->set("config/general/dither_mode", "none");
	config->set("config/general/enable_cursors", true);
	config->set("config/general/show_console", true);
	config->set("config/general/converse_gump", "default");
	config->set("config/general/use_text_gumps", false);
	config->set("config/general/party_formation", "standard");

	config->set("config/cheats/enabled", false);
	config->set("config/cheats/enable_hackmove", false);
	config->set("config/cheats/min_brightness", 0);
	config->set("config/cheats/party_all_the_time", false);
// game specific settings
	uint8 bg_color[] = { 218, 136, 216 }; // U6, MD, SE
	uint8 border_color[] = { 220, 133, 219 }; // U6, MD, SE
	std::string game_str[] = { "config/ultima6/", "config/martian/", "config/savage/" };
	for (int i = 0; i < 3; i++) {
		config->set(game_str[i] + "language", "en");
		config->set(game_str[i] + "music", "native");
		config->set(game_str[i] + "sfx", "native");
		if (i == 0) // U6
			config->set(game_str[i] + "enable_speech", "yes");
		config->set(game_str[i] + "skip_intro", false);
		config->set(game_str[i] + "show_eggs", false);
		if (i == 0) { // U6
			config->set(game_str[i] + "show_stealing", false);
			config->set(game_str[i] + "roof_mode", false);
		}
		config->set(game_str[i] + "use_new_dolls", false);
		config->set(game_str[i] + "cb_position", "default");
		config->set(game_str[i] + "show_orig_style_cb", "default");
		if (i == 0) // U6
			config->set(game_str[i] + "cb_text_color", 115);
		config->set(game_str[i] + "map_tile_lighting", i == 1 ? false : true); // MD has canals lit up so disable
		config->set(game_str[i] + "custom_actor_tiles", "default");
		config->set(game_str[i] + "converse_solid_bg", false);
		config->set(game_str[i] + "converse_bg_color", bg_color[i]);
		config->set(game_str[i] + "converse_width", "default");
		config->set(game_str[i] + "converse_height", "default");
		if (i == 0) { // U6
			config->set(game_str[i] + "displayed_wind_dir", "from");
			config->set(game_str[i] + "free_balloon_movement", false);
		}
		config->set(game_str[i] + "game_specific_keys", "(default)");
		config->set(game_str[i] + "newscroll/width", 30);
		config->set(game_str[i] + "newscroll/height", 19);
		config->set(game_str[i] + "newscroll/solid_bg", false);
		config->set(game_str[i] + "newscroll/bg_color", bg_color[i]);
		config->set(game_str[i] + "newscroll/border_color", border_color[i]);
	}

//	config->set("config/newgamedata/name", "Avatar");
//	config->set("config/newgamedata/gender", 0);
//	config->set("config/newgamedata/portrait", 0);
//	config->set("config/newgamedata/str", 0xf);
//	config->set("config/newgamedata/dex", 0xf);
//	config->set("config/newgamedata/int", 0xf);

}

/* Should be safe default video settings
 */
void Nuvie::set_safe_video_settings() {
	config->set("config/video/scale_method", "point");

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		DEBUG(0, LEVEL_ERROR, "Couldn't initialize SDL_VIDEO!\n");
		exit(EXIT_FAILURE);
	}
	config->set("config/video/scale_factor", "1");

//FIXME SDL2    const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
//	if(!vinfo) // couldn't get display mode
//		config->set("config/video/scale_factor", "1");
//	else
//	{
//		if(vinfo->current_w  >= 640 && vinfo->current_h >= 400)
//			config->set("config/video/scale_factor", "2");
//		else // portable with small screen
//			config->set("config/video/scale_factor", "1");
//	}
	SDL_Quit();

	config->set("config/video/fullscreen", "no");
	config->set("config/video/non_square_pixels", "no");
	config->set("config/video/screen_width", 320);
	config->set("config/video/screen_height", 200);
	config->set("config/video/game_width", 320);
	config->set("config/video/game_height", 200);
}

bool Nuvie::initDefaultConfigWin32() {
	const unsigned char cfg_stub[] = "<config></config>";
	std::string config_path("./nuvie.cfg");

	NuvieIOFileWrite cfgFile;

	if (cfgFile.open(config_path) == false)
		return false;

	cfgFile.writeBuf(cfg_stub, sizeof(cfg_stub));
	cfgFile.close();

	if (loadConfigFile(config_path, NUVIE_CONF_READWRITE) == false)
		return false;

	SharedDefaultConfigValues();
	config->set("config/ultima6/gamedir", "c:\\ultima6");
	config->set("config/ultima6/townsdir", "c:\\fmtownsU6");
	config->set("config/ultima6/savedir", "./u6_save");
	config->set("config/ultima6/sfxdir", "./custom_sfx");
	config->set("config/ultima6/patch_keys", "./patchkeys.txt");
	config->set("config/martian/gamedir", "c:\\martian");
	config->set("config/martian/savedir", "./martian_save");
	config->set("config/martian/patch_keys", "./patchkeys.txt");
	config->set("config/savage/gamedir", "c:\\savage");
	config->set("config/savage/savedir", "./savage_save");
	config->set("config/savage/patch_keys", "./patchkeys.txt");

	config->write();

	return true;
}

bool Nuvie::initDefaultConfigMacOSX(const char *home_env) {
	const unsigned char cfg_stub[] = "<config></config>";
	std::string config_path;
	std::string home(home_env);
	config_path = home;
	config_path.append("/Library/Preferences/Nuvie Preferences");

	NuvieIOFileWrite cfgFile;

	if (cfgFile.open(config_path) == false)
		return false;

	cfgFile.writeBuf(cfg_stub, sizeof(cfg_stub));
	cfgFile.close();

	if (loadConfigFile(config_path, NUVIE_CONF_READWRITE) == false)
		return false;

	SharedDefaultConfigValues();
	config->set("config/ultima6/gamedir", "/Library/Application Support/Nuvie Support/ultima6");
	config->set("config/ultima6/townsdir", "/Library/Application Support/Nuvie Support/townsU6");
	config->set("config/ultima6/savedir", home + "/Library/Application Support/Nuvie/savegames");
	config->set("config/ultima6/sfxdir", home + "/Library/Application Support/Nuvie/custom_sfx");
	config->set("config/ultima6/patch_keys", home + "/Library/Preferences/Nuvie Preferences/patchkeys.txt");
	config->set("config/martian/gamedir", "/Library/Application Support/Nuvie Support/martian");
	config->set("config/martian/patch_keys", home + "/Library/Preferences/Nuvie Preferences/patchkeys.txt");
	config->set("config/savage/gamedir", "/Library/Application Support/Nuvie Support/savage");
	config->set("config/savage/patch_keys", home + "/Library/Preferences/Nuvie Preferences/patchkeys.txt");

	config->write();

	return true;
}

bool Nuvie::initDefaultConfigUnix(const char *home_env) {
	const unsigned char cfg_stub[] = "<config></config>";
	std::string config_path;
	std::string home(home_env);
	config_path = home;
	config_path.append("/.nuvierc");

	NuvieIOFileWrite cfgFile;

	if (cfgFile.open(config_path) == false)
		return false;

	cfgFile.writeBuf(cfg_stub, sizeof(cfg_stub));
	cfgFile.close();

	if (loadConfigFile(config_path, NUVIE_CONF_READWRITE) == false)
		return false;

	SharedDefaultConfigValues();
	config->set("config/ultima6/gamedir", "./ultima6");
	config->set("config/ultima6/townsdir", "./townsU6");
	config->set("config/ultima6/savedir", home + "/.nuvie/savegames");
	config->set("config/ultima6/patch_keys", home + "/.nuvie/patchkeys.txt");
	config->set("config/ultima6/sfxdir", home + "/.nuvie/custom_sfx");
	config->set("config/martian/gamedir", "./martian");
	config->set("config/martian/patch_keys", home + "/.nuvie/patchkeys.txt");
	config->set("config/savage/gamedir", "./savage");
	config->set("config/savage/patch_keys", home + "/.nuvie/patchkeys.txt");

	config->write();

	return true;
}

bool Nuvie::loadConfigFile(std::string filename, bool readOnly) {
	struct stat sb;
	DEBUG(0, LEVEL_INFORMATIONAL, "Loading Config from '%s': ", filename.c_str());

	if (stat(filename.c_str(), &sb) == 0) {
		if (config->readConfigFile(filename, "config", readOnly) == true) {
			DEBUG(1, LEVEL_INFORMATIONAL, "Done.\n");
			return true;
		}
	}

	DEBUG(1, LEVEL_INFORMATIONAL, "Failed.\n", filename.c_str());
	DEBUG(0, LEVEL_ERROR, "Failed to load config from '%s'.\n", filename.c_str());
	return false;
}

void Nuvie::assignGameConfigValues(uint8 game_type) {
	std::string game_name, game_id;

	config->set("config/GameType", game_type);

	switch (game_type) {
	case NUVIE_GAME_U6 :
		game_name.assign("ultima6");
		game_id.assign("u6");
		break;
	case NUVIE_GAME_MD :
		game_name.assign("martian");
		game_id.assign("md");
		break;
	case NUVIE_GAME_SE :
		game_name.assign("savage");
		game_id.assign("se");
		break;
	}

	config->set("config/GameName", game_name);
	config->set("config/GameID", game_id);

	return;
}

bool Nuvie::checkGameDir(uint8 game_type) {
	std::string path;

	config_get_path(config, "", path);
	ConsoleAddInfo("gamedir: \"%s\"", path.c_str());

#ifndef WIN32
	struct stat sb;

	if (stat(path.c_str(), &sb) == 0 && sb.st_mode & S_IFDIR) {
		return true;
	}

	ConsoleAddError("Cannot open gamedir!");
	ConsoleAddError("\"" + path + "\"");

	return false;
#endif

	return true;
}

bool Nuvie::checkDataDir() {
	std::string path;
	config->value("config/datadir", path, "");
	ConsoleAddInfo("datadir: \"%s\"", path.c_str());

#ifndef WIN32
	struct stat sb;

	if (stat(path.c_str(), &sb) == 0 && sb.st_mode & S_IFDIR) {
		return true;
	}

	ConsoleAddError("Cannot open datadir!");
	ConsoleAddError("\"" + path + "\"");

	return false;
#endif

	return true;
}

bool Nuvie::playIntro() {
	bool skip_intro;

	string key = config_get_game_key(config);
	key.append("/skip_intro");
	config->value(key, skip_intro, false);

	if (skip_intro)
		return true;

	if (script->play_cutscene("/intro.lua")) {
		bool should_quit = false;
		config->value("config/quit", should_quit, false);
		if (!should_quit) {
			ConsoleHide();
			return true;
		}
	}

	return false;
}

} // End of namespace Ultima8
