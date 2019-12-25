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
#include "common/translation.h"
#include "common/unzip.h"

namespace Ultima {
namespace Ultima6 {

#define DATA_FILENAME "ultima.dat"
#define DATA_VERSION_MAJOR 1
#define DATA_VERSION_MINOR 0

Ultima6Engine *g_engine;

Ultima6Engine::Ultima6Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Ultima6"),
		_config(nullptr), _dataArchive(nullptr), _events(nullptr), _screen(nullptr),
		_script(nullptr), _game(nullptr) {
	g_engine = this;
}

Ultima6Engine::~Ultima6Engine() {
	delete _config;
	delete _events;
	delete _screen;
	delete _script;
	delete _game;

	g_engine = nullptr;
}

bool Ultima6Engine::initialize() {
	uint8 gameType;
	bool playENding = false;
	bool showVirtueMsg = false;

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

	// Hook up to the data archive
	if (!loadData())
		return false;

	// Find and load config file
	initConfig();

	// Setup events and screen
	_events = new Events();
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


bool Ultima6Engine::loadData() {
	Common::File f;

	if (!Common::File::exists(DATA_FILENAME) ||
		(_dataArchive = Common::makeZipArchive(DATA_FILENAME)) == 0 ||
		!f.open("ultima8/version.txt", *_dataArchive)) {
		delete _dataArchive;
		GUIError(Common::String::format(_("Could not locate engine data %s"), DATA_FILENAME));
		return false;
	}

	// Validate the version
	char buffer[5];
	f.read(buffer, 4);
	buffer[4] = '\0';

	int major = 0, minor = 0;
	if (buffer[1] == '.') {
		major = buffer[0] - '0';
		minor = atoi(&buffer[2]);
	}

	if (major != DATA_VERSION_MAJOR || minor != DATA_VERSION_MINOR) {
		delete _dataArchive;
		GUIError(Common::String::format(_("Out of date engine data. Expected %d.%d, but got version %d.%d"),
			DATA_VERSION_MAJOR, DATA_VERSION_MINOR, major, minor));
		return false;
	}

	SearchMan.add("data", _dataArchive);
	return true;
}

void Ultima6Engine::GUIError(const Common::String& msg) {
	GUIErrorMessage(msg);
}

} // End of namespace Ultima6
} // End of namespace Ultima
