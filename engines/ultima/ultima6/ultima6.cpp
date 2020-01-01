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
#include "ultima/ultima6/core/events.h"
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
#include "ultima/ultima6/core/map_window.h"
#include "ultima/ultima6/save/save_game.h"
#include "ultima/ultima6/core/msg_scroll.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/unzip.h"

namespace Ultima {
namespace Ultima6 {

#define DATA_FILENAME "ultima.dat"
#define DATA_VERSION_MAJOR 1
#define DATA_VERSION_MINOR 0

Ultima6Engine *g_engine;

Ultima6Engine::Ultima6Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Ultima::Shared::UltimaEngine(syst, gameDesc),  _config(nullptr), _savegame(nullptr),
		_screen(nullptr), _script(nullptr), _game(nullptr) {
	g_engine = this;
}

Ultima6Engine::~Ultima6Engine() {
	delete _config;
	delete _events;
	delete _savegame;
	delete _screen;
	delete _script;
	delete _game;

	g_engine = nullptr;
}

bool Ultima6Engine::isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) {
	folder = "ultima6";
	majorVersion = 1;
	minorVersion = 0;
	return true;
}


bool Ultima6Engine::initialize() {
	uint8 gameType;
	bool playEnding = false;
	bool showVirtueMsg = false;

	if (!Ultima::Shared::UltimaEngine::initialize())
		return false;

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
	initConfig();

	// Setup events
	Events *events = new Ultima::Ultima6::Events(_config);
	_events = events;

	// Setup savegame handler
	_savegame = new SaveGame(_config);

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

	// Setup various game related config variables.
	assignGameConfigValues(gameType);

	// Check for a valid path to the selected game.
	if (checkGameDir(gameType) == false)
		return false;

	if (checkDataDir() == false)
		return false;

	SoundManager *sound_manager = new SoundManager(_mixer);
	sound_manager->nuvieStartup(_config);

	_game = new Game(_config, events, _screen, gui, gameType, sound_manager);

	_script = new Script(_config, gui, sound_manager, gameType);
	if (_script->init() == false)
		return false;

	if (playEnding) {
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

void Ultima6Engine::initConfig() {
	_config = new Configuration();

	// nuvie.cfg in the game folder can supercede any ScummVM settings
	if (Common::File::exists("nuvie.cfg"))
		(void)_config->readConfigFile("nuvie.cfg", "config");

	_config->setScummVMDefaultsIfNeeded(_gameDescription->gameId);
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

bool Ultima6Engine::canLoadGameStateCurrently() {
	if (_events == nullptr)
		return false;

	// Note that to mimic what Nuvie originally did, any attempt to try and open
	// the save dialog will result in active gumps being closed
	Events *events = static_cast<Events *>(_events);
	MapWindow *mapWindow = _game->get_map_window();
	events->close_gumps();

	switch (events->get_mode()) {
	case EQUIP_MODE:
		events->cancelAction();
		return false;
	case MOVE_MODE:
		mapWindow->set_looking(false);
		mapWindow->set_walking(false);
		return true;
	default:
		// Saving/loading only available in standard move mode in-game
		return false;
	}
}

bool Ultima6Engine::canSaveGameStateCurrently() {
	if (!canLoadGameStateCurrently())
		return false;

	// Further checks against saving
	Events *events = static_cast<Events *>(_events);
	MsgScroll *scroll = _game->get_scroll();

	if (_game->is_armageddon()) {
		scroll->message("Can't save. You killed everyone!\n\n");
		return false;
	} else if (events->using_control_cheat()) {
		scroll->message(" Can't save while using control cheat\n\n");
		return false;
	}

	return true;
}

Common::Error Ultima6Engine::loadGameState(int slot) {
	// TODO
	return Common::kNoError;
}

Common::Error Ultima6Engine::saveGameState(int slot, const Common::String &desc) {
	// TODO
	return Common::kNoError;
}

bool Ultima6Engine::journeyOnwards() {
	bool newsave = false;

	_config->value("config/newgame", newsave, false);

	if (newsave) {
		return _savegame->load_new();
	}

	return loadLatestSave();
}

bool Ultima6Engine::loadLatestSave() {
	if (ConfMan.hasKey("latest_save")) {
		int saveSlot = ConfMan.getInt("latest_save");
		return loadGameState(saveSlot).getCode() == Common::kNoError;
	}

	return _savegame->load_new();
}

bool Ultima6Engine::quickSave(int saveSlot, bool isLoad) {
	if (saveSlot < 0 || saveSlot > 99)
		return false;

	std::string text;
	MsgScroll *scroll = _game->get_scroll();

	if (isLoad) {
		if (!canLoadGameStateCurrently())
			return false;

		text = _("loading quick save %d");
	} else {
		if (!canSaveGameStateCurrently())
			return false;

		text = _("saving quick save %d");
	}

	text = std::string::format(text.c_str(), saveSlot);
	scroll->display_string(text);

	if (isLoad) {
		if (loadGameState(saveSlot).getCode() == Common::kNoError) {
			return true;
		} else {
			scroll->message("\nfailed!\n\n");
			return false;
		}
	} else {
		Common::String saveDesc = Common::String::format(_("Quicksave %03d"), saveSlot);
		return saveGameState(saveSlot, saveDesc).getCode() == Common::kNoError;
	}
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
