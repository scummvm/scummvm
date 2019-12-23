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

#include "ultima/ultima6/ultima6.h"
#include "ultima/ultima6/actors/actor.h"
#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/ultima6/conf/configuration.h"
#include "ultima/ultima6/misc/u6_misc.h"
#include "ultima/ultima6/files/nuvie_io_file.h"
#include "ultima/ultima6/screen/screen.h"
#include "ultima/ultima6/script/script.h"
#include "ultima/ultima6/core/game.h"
#include "ultima/ultima6/gui/gui.h"
#include "ultima/ultima6/core/console.h"
#include "ultima/ultima6/sound/sound_manager.h"

namespace Ultima {
namespace Ultima6 {

Ultima6Engine *g_engine;

Ultima6Engine::Ultima6Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Ultima6"),
		_config(nullptr), _screen(nullptr), _script(nullptr), _game(nullptr) {
	g_engine = this;
}

Ultima6Engine::~Ultima6Engine() {
	delete _config;
	delete _screen;
	delete _script;
	delete _game;

	g_engine = nullptr;
}

bool Ultima6Engine::initialize() {
	uint8 gameType;
	bool playENding = false;
	bool showVirtueMsg = false;
	bool resetVideo = false;

	// Get which game to play
	switch (_gameDescription->gameId) {
	case GAME_ULTIMA6:
		gameType = NUVIE_GAME_U6;
		break;
	case GAME_MARTIAN_DREAMS:
		gameType = NUVIE_GAME_MD;
		break;
	case GAME_SAVAGE_EMPIRE:
		gameType = NUVIE_GAME_SE;
		break;
	default:
		error("Unknown game");
		break;
	}

	// Find and load config file
	if (initConfig() == false) {
		DEBUG(0, LEVEL_ERROR, "No config file found!\n");
		return false;
	} else if (resetVideo) {
		set_safe_video_settings();
		_config->write();
	}

	// Setup screen
	_screen = new Screen(_config);

	if (_screen->init() == false) {
		DEBUG(0, LEVEL_ERROR, "Initializing screen!\n");
		return false;
	}

	GUI *gui = new GUI(_config, _screen);

	ConsoleInit(_config, _screen, gui, 320, 200);
	ConsoleAddInfo("\n ScummVM Nuvie: ver 0.5 rev 1927 \n");
	ConsoleAddInfo("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");

	if (showVirtueMsg) {
		ConsoleShow();
		ConsoleAddInfo("");
		ConsoleAddInfo("");
		ConsoleAddInfo("\"The path to victory is marked with the laws of");
		ConsoleAddInfo(" Virtue, not the commands of DOS.\"");
		ConsoleAddInfo("");
		ConsoleAddInfo("     -- Lord British");
		return false;
	}

	ConsoleAddInfo("Config file: \"%s\"", _config->filename().c_str());

	// Setup various game related config variables.
	assignGameConfigValues(gameType);

	// Check for a valid path to the selected game.
	if (checkGameDir(gameType) == false)
		return false;

	if (checkDataDir() == false)
		return false;

	SoundManager *sound_manager = new SoundManager(_mixer);
	sound_manager->nuvieStartup(_config);

	_game = new Game(_config, _screen, gui, gameType, sound_manager);

	_script = new Script(_config, gui, sound_manager, gameType);
	if (_script->init() == false)
		return false;

	if (playENding) {
		_script->play_cutscene("/ending.lua");
		return false;
	}

	if (playIntro() == false) {
		ConsoleDelete();
		return false;
	}

	if (_game->loadGame(_script) == false) {
		delete _game;
		return false;
	}

	ConsoleDelete();

	return true;
}

Common::Error Ultima6Engine::run() {
	if (initialize()) {
		if (_game)
			_game->play();
	}

	return Common::kNoError;
}

const char *Ultima6Engine::getConfigPathWin32() {
	static char configFile[MAXPATHLEN];
	configFile[0] = '\0';

	return configFile;
}

bool Ultima6Engine::initConfig() {
	std::string config_path;

	_config = new Configuration();

	const char *configFilePath = getConfigPathWin32();
	config_path.assign(configFilePath);
	if (loadConfigFile(config_path))
		return true;

	// nuvie.cfg in the working dir
	config_path.assign("nuvie.cfg");

	if (loadConfigFile(config_path))
		return true;

	if (initDefaultConfigWin32())
		return true;

	delete _config;
	_config = NULL;

	return false;
}

void Ultima6Engine::SharedDefaultConfigValues() {
	_config->set("config/loadgame", "ultima6");
	_config->set("config/datadir", "./data");
	_config->set("config/keys", "(default)");

	set_safe_video_settings();
	_config->set("config/video/game_style", "original");
	_config->set("config/video/game_position", "center");

	_config->set("config/audio/enabled", true);
	_config->set("config/audio/enable_music", true);
	_config->set("config/audio/enable_sfx", true);
	_config->set("config/audio/music_volume", 100);
	_config->set("config/audio/sfx_volume", 255);
	_config->set("config/audio/combat_changes_music", true);
	_config->set("config/audio/vehicles_change_music", true);
	_config->set("config/audio/conversations_stop_music", false); // original stopped music - maybe due to memory and disk swapping
	_config->set("config/audio/stop_music_on_group_change", true);

#ifdef HAVE_JOYSTICK_SUPPORT
	_config->set("config/joystick/enable_joystick", false);
	_config->set("config/joystick/repeat_hat", false);
	_config->set("config/joystick/repeat_delay", 50);

	uint8 axisx[] = { 0, 3, 4, 6 };
	uint8 axisy[] = { 1, 2, 5, 7 };
	std::string axes_str = "config/joystick/axes_pair";
	std::string pair_str[] = { axes_str + "1", axes_str + "2", axes_str + "3", axes_str + "4" };
	for (int i = 0; i < 4; i++) {
		_config->set(pair_str[i] + "/x_axis", axisx[i]);
		_config->set(pair_str[i] + "/y_axis", axisy[i]);
		_config->set(pair_str[i] + "/delay", 110);
		_config->set(pair_str[i] + "/x_deadzone", 8000);
		_config->set(pair_str[i] + "/y_deadzone", 8000);
	}
#endif

	_config->set("config/input/enable_doubleclick", true);
	_config->set("config/input/doubleclick_opens_containers", false);
	_config->set("config/input/party_view_targeting", false);
	_config->set("config/input/new_command_bar", false);
	_config->set("config/input/enabled_dragging", true);
	_config->set("config/input/look_on_left_click", true);
	_config->set("config/input/walk_with_left_button", true);
	_config->set("config/input/direction_selects_target", true);
	_config->set("config/input/interface", "normal");

	_config->set("config/general/lighting", "original");
	_config->set("config/general/dither_mode", "none");
	_config->set("config/general/enable_cursors", true);
	_config->set("config/general/show_console", true);
	_config->set("config/general/converse_gump", "default");
	_config->set("config/general/use_text_gumps", false);
	_config->set("config/general/party_formation", "standard");

	_config->set("config/cheats/enabled", false);
	_config->set("config/cheats/enable_hackmove", false);
	_config->set("config/cheats/min_brightness", 0);
	_config->set("config/cheats/party_all_the_time", false);
// game specific settings
	uint8 bg_color[] = { 218, 136, 216 }; // U6, MD, SE
	uint8 border_color[] = { 220, 133, 219 }; // U6, MD, SE
	std::string game_str[] = { "config/ultima6/", "config/martian/", "config/savage/" };
	for (int i = 0; i < 3; i++) {
		_config->set(game_str[i] + "language", "en");
		_config->set(game_str[i] + "music", "native");
		_config->set(game_str[i] + "sfx", "native");
		if (i == 0) // U6
			_config->set(game_str[i] + "enable_speech", "yes");
		_config->set(game_str[i] + "skip_intro", false);
		_config->set(game_str[i] + "show_eggs", false);
		if (i == 0) { // U6
			_config->set(game_str[i] + "show_stealing", false);
			_config->set(game_str[i] + "roof_mode", false);
		}
		_config->set(game_str[i] + "use_new_dolls", false);
		_config->set(game_str[i] + "cb_position", "default");
		_config->set(game_str[i] + "show_orig_style_cb", "default");
		if (i == 0) // U6
			_config->set(game_str[i] + "cb_text_color", 115);
		_config->set(game_str[i] + "map_tile_lighting", i == 1 ? false : true); // MD has canals lit up so disable
		_config->set(game_str[i] + "custom_actor_tiles", "default");
		_config->set(game_str[i] + "converse_solid_bg", false);
		_config->set(game_str[i] + "converse_bg_color", bg_color[i]);
		_config->set(game_str[i] + "converse_width", "default");
		_config->set(game_str[i] + "converse_height", "default");
		if (i == 0) { // U6
			_config->set(game_str[i] + "displayed_wind_dir", "from");
			_config->set(game_str[i] + "free_balloon_movement", false);
		}
		_config->set(game_str[i] + "game_specific_keys", "(default)");
		_config->set(game_str[i] + "newscroll/width", 30);
		_config->set(game_str[i] + "newscroll/height", 19);
		_config->set(game_str[i] + "newscroll/solid_bg", false);
		_config->set(game_str[i] + "newscroll/bg_color", bg_color[i]);
		_config->set(game_str[i] + "newscroll/border_color", border_color[i]);
	}

//	_config->set("config/newgamedata/name", "Avatar");
//	_config->set("config/newgamedata/gender", 0);
//	_config->set("config/newgamedata/portrait", 0);
//	_config->set("config/newgamedata/str", 0xf);
//	_config->set("config/newgamedata/dex", 0xf);
//	_config->set("config/newgamedata/int", 0xf);

}

/* Should be safe default video settings
 */
void Ultima6Engine::set_safe_video_settings() {
	_config->set("config/video/scale_method", "point");

	_config->set("config/video/scale_factor", "1");

//FIXME SDL2    const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
//	if(!vinfo) // couldn't get display mode
//		_config->set("config/video/scale_factor", "1");
//	else
//	{
//		if(vinfo->current_w  >= 640 && vinfo->current_h >= 400)
//			_config->set("config/video/scale_factor", "2");
//		else // portable with small screen
//			_config->set("config/video/scale_factor", "1");
//	}

	_config->set("config/video/fullscreen", "no");
	_config->set("config/video/non_square_pixels", "no");
	_config->set("config/video/screen_width", 320);
	_config->set("config/video/screen_height", 200);
	_config->set("config/video/game_width", 320);
	_config->set("config/video/game_height", 200);
}

bool Ultima6Engine::initDefaultConfigWin32() {
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
	_config->set("config/ultima6/gamedir", "c:\\ultima6");
	_config->set("config/ultima6/townsdir", "c:\\fmtownsU6");
	_config->set("config/ultima6/savedir", "./u6_save");
	_config->set("config/ultima6/sfxdir", "./custom_sfx");
	_config->set("config/ultima6/patch_keys", "./patchkeys.txt");
	_config->set("config/martian/gamedir", "c:\\martian");
	_config->set("config/martian/savedir", "./martian_save");
	_config->set("config/martian/patch_keys", "./patchkeys.txt");
	_config->set("config/savage/gamedir", "c:\\savage");
	_config->set("config/savage/savedir", "./savage_save");
	_config->set("config/savage/patch_keys", "./patchkeys.txt");

	_config->write();

	return true;
}

bool Ultima6Engine::initDefaultConfigMacOSX(const char *home_env) {
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
	_config->set("config/ultima6/gamedir", "/Library/Application Support/Nuvie Support/ultima6");
	_config->set("config/ultima6/townsdir", "/Library/Application Support/Nuvie Support/townsU6");
	_config->set("config/ultima6/savedir", home + "/Library/Application Support/Nuvie/savegames");
	_config->set("config/ultima6/sfxdir", home + "/Library/Application Support/Nuvie/custom_sfx");
	_config->set("config/ultima6/patch_keys", home + "/Library/Preferences/Nuvie Preferences/patchkeys.txt");
	_config->set("config/martian/gamedir", "/Library/Application Support/Nuvie Support/martian");
	_config->set("config/martian/patch_keys", home + "/Library/Preferences/Nuvie Preferences/patchkeys.txt");
	_config->set("config/savage/gamedir", "/Library/Application Support/Nuvie Support/savage");
	_config->set("config/savage/patch_keys", home + "/Library/Preferences/Nuvie Preferences/patchkeys.txt");

	_config->write();

	return true;
}

bool Ultima6Engine::initDefaultConfigUnix(const char *home_env) {
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
	_config->set("config/ultima6/gamedir", "./ultima6");
	_config->set("config/ultima6/townsdir", "./townsU6");
	_config->set("config/ultima6/savedir", home + "/.nuvie/savegames");
	_config->set("config/ultima6/patch_keys", home + "/.nuvie/patchkeys.txt");
	_config->set("config/ultima6/sfxdir", home + "/.nuvie/custom_sfx");
	_config->set("config/martian/gamedir", "./martian");
	_config->set("config/martian/patch_keys", home + "/.nuvie/patchkeys.txt");
	_config->set("config/savage/gamedir", "./savage");
	_config->set("config/savage/patch_keys", home + "/.nuvie/patchkeys.txt");

	_config->write();

	return true;
}

bool Ultima6Engine::loadConfigFile(std::string filename, bool readOnly) {
	DEBUG(0, LEVEL_INFORMATIONAL, "Loading Config from '%s': ", filename.c_str());

	if (Common::File::exists(filename)) {
		if (_config->readConfigFile(filename, "config", readOnly) == true) {
			DEBUG(1, LEVEL_INFORMATIONAL, "Done.\n");
			return true;
		}
	}

	DEBUG(1, LEVEL_INFORMATIONAL, "Failed.\n", filename.c_str());
	DEBUG(0, LEVEL_ERROR, "Failed to load config from '%s'.\n", filename.c_str());
	return false;
}

void Ultima6Engine::assignGameConfigValues(uint8 gameType) {
	std::string game_name, game_id;

	_config->set("config/GameType", gameType);

	switch (gameType) {
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

	_config->set("config/GameName", game_name);
	_config->set("config/GameID", game_id);

	return;
}

bool Ultima6Engine::checkGameDir(uint8 gameType) {
	std::string path;

	config_get_path(_config, "", path);
	ConsoleAddInfo("gamedir: \"%s\"", path.c_str());

	return true;
}

bool Ultima6Engine::checkDataDir() {
	std::string path;
	_config->value("config/datadir", path, "");
	ConsoleAddInfo("datadir: \"%s\"", path.c_str());

	return true;
}

bool Ultima6Engine::playIntro() {
	bool skip_intro;

	string key = config_get_game_key(_config);
	key.append("/skip_intro");
	_config->value(key, skip_intro, false);

	if (skip_intro)
		return true;

	if (_script->play_cutscene("/intro.lua")) {
		bool should_quit = false;
		_config->value("config/quit", should_quit, false);
		if (!should_quit) {
			ConsoleHide();
			return true;
		}
	}

	return false;
}

} // End of namespace Ultima6
} // End of namespace Ultima
