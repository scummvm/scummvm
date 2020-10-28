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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/misc/args.h"
#include "ultima/ultima8/games/game_info.h"
#include "ultima/shared/std/misc.h"

namespace Ultima {
namespace Ultima8 {

using Std::string;

CoreApp *CoreApp::_application = nullptr;

CoreApp::CoreApp(const Ultima::UltimaGameDescription *gameDesc)
		: _gameDesc(gameDesc), _isRunning(false), _gameInfo(nullptr), _fileSystem(nullptr),
		_configFileMan(nullptr), _settingMan(nullptr) {
	_application = this;
}

CoreApp::~CoreApp() {
	for (GameMap::iterator i = _games.begin(); i != _games.end(); ++i)
		delete i->_value;

	_games.clear();

	FORGET_OBJECT(_fileSystem);
	FORGET_OBJECT(_settingMan);
	FORGET_OBJECT(_configFileMan);
	FORGET_OBJECT(_gameInfo);

	_application = nullptr;
}

void CoreApp::startup() {
	sysInit();
	loadConfig(); // load config files
}

void CoreApp::sysInit() {
	_gameInfo = nullptr;

	_fileSystem = new FileSystem;

	_configFileMan = new ConfigFileManager();
	_settingMan = new SettingManager();
	_settingMan->setDomainName(SettingManager::DOM_GLOBAL, "pentagram");
	_settingMan->setCurrentDomain(SettingManager::DOM_GLOBAL);
}

// load configuration files
void CoreApp::loadConfig() {
	pout << "Loading configuration files:" << Std::endl;

	bool dataconf, homeconf;

	// system-wide config, read-only
	dataconf = _settingMan->readConfigFile("@data/pentagram.ini", true);

	// user config
	homeconf = _settingMan->readConfigFile("@home/pentagram.ini");

	if (!homeconf && !dataconf) {
		pout << "No configuration files found." << Std::endl;
	} else {

		if (dataconf)
			pout << "@data/pentagram.ini" << Std::endl;
		if (homeconf)
			pout << "@home/pentagram.ini" << Std::endl;
	}

	//  load pentagram specific data path
	Std::string data;
	if (_settingMan->get("data", data, SettingManager::DOM_GLOBAL)) {
		pout << "Setting custom data path: " << data << Std::endl;
		bool ok = _fileSystem->AddVirtualPath("@data", data);
		if (!ok) {
			perr << "Error opening data directory." << Std::endl;
		}
	}
}

void CoreApp::setupGameList() {
	Std::vector<istring> gamelist;
	gamelist = _settingMan->listGames();
	debugN(MM_INFO, "Scanning config file for games:\n");
	Std::vector<istring>::iterator iter;
	istring gamename;

	for (iter = gamelist.begin(); iter != gamelist.end(); ++iter) {
		istring game = *iter;
		GameInfo *info = new GameInfo;
		bool detected = getGameInfo(game, info);

		// output detected game info
		debugN(MM_INFO, "%s: ", game.c_str());
		if (detected) {
			// add game to games map
			_games[game] = info;

			Std::string details = info->getPrintDetails();
			debugN(MM_INFO, "%s", details.c_str());
		} else {
			debugN(MM_INFO, "unknown, skipping");
		}
		debugN(MM_INFO, "\n");
	}
}

GameInfo *CoreApp::getDefaultGame() {
	istring gamename;

	Std::string defaultgame;
	bool defaultset = _settingMan->get("defaultgame", defaultgame,
	                                  SettingManager::DOM_GLOBAL);
	if (defaultset) {
		// default game specified in config file
		gamename = defaultgame;
	} else {
		gamename = _gameDesc->desc.gameId;
	}

	GameInfo *info = getGameInfo(gamename);

	if (!info) {
		perr << "Game \"" << gamename << "\" not found." << Std::endl;
	}

	// We've got a default game name, doesn't mean it will work though
	return info;
}

bool CoreApp::setupGame(GameInfo *info) {
	if (!info) return false;
	assert(info->_name != "");

	_gameInfo = info;

	pout << "Selected game: " << info->_name << Std::endl;
	pout << info->getPrintDetails() << Std::endl;

	setupGamePaths(info);

	return info->_name != "pentagram";
}

void CoreApp::killGame() {
	// Save the settings!
	pout << "Saving settings" << Std::endl;
	_settingMan->write();

	_fileSystem->RemoveVirtualPath("@game");
	_fileSystem->RemoveVirtualPath("@work");
	_fileSystem->RemoveVirtualPath("@save");

	_configFileMan->clearRoot("bindings");
	_configFileMan->clearRoot("language");
	_configFileMan->clearRoot("weapons");
	_configFileMan->clearRoot("armour");
	_configFileMan->clearRoot("monsters");
	_configFileMan->clearRoot("game");
	_settingMan->setCurrentDomain(SettingManager::DOM_GLOBAL);

	_gameInfo = nullptr;
}


bool CoreApp::getGameInfo(const istring &game, GameInfo *ginfo) {
	// first try getting the information from the config file
	// if that fails, try to autodetect it

	ginfo->_name = game;
	ginfo->_type = GameInfo::GAME_UNKNOWN;
	ginfo->version = 0;
	ginfo->_language = GameInfo::GAMELANG_UNKNOWN;

	istring gamekey = "settings/";
	gamekey += game;

	if (game == "pentagram") {
		ginfo->_type = GameInfo::GAME_PENTAGRAM_MENU;
		ginfo->_language = GameInfo::GAMELANG_ENGLISH;

	} else {
		assert(game == "ultima8" || game == "remorse" || game == "regret");

		if (game == "ultima8")
			ginfo->_type = GameInfo::GAME_U8;
		else if (game == "remorse")
			ginfo->_type = GameInfo::GAME_REMORSE;
		else if (game == "regret")
			ginfo->_type = GameInfo::GAME_REGRET;

		switch (_gameDesc->desc.language) {
		case Common::EN_ANY:
			ginfo->_language = GameInfo::GAMELANG_ENGLISH;
			break;
		case Common::FR_FRA:
			ginfo->_language = GameInfo::GAMELANG_FRENCH;
			break;
		case Common::DE_DEU:
			ginfo->_language = GameInfo::GAMELANG_GERMAN;
			break;
		case Common::ES_ESP:
			ginfo->_language = GameInfo::GAMELANG_SPANISH;
			break;
		case Common::JA_JPN:
			ginfo->_language = GameInfo::GAMELANG_JAPANESE;
			break;
		default:
			error("Unknown language");
			break;
		}
	}

	return ginfo->_type != GameInfo::GAME_UNKNOWN;
}

void CoreApp::setupGamePaths(GameInfo *ginfo) {
	if (!ginfo || ginfo->_name == "pentagram") {
		_settingMan->setCurrentDomain(SettingManager::DOM_GLOBAL);
		return;
	}

	istring game = ginfo->_name;

	_settingMan->setDomainName(SettingManager::DOM_GAME, game);
	_settingMan->setCurrentDomain(SettingManager::DOM_GAME);

	// load main game data path
	Std::string gpath;
	_settingMan->get("path", gpath, SettingManager::DOM_GAME);
	_fileSystem->AddVirtualPath("@game", gpath);

	// load work path. Default is @home/game-work
	// where 'game' in the above is the specified 'game' loaded
	Std::string work;
	if (!_settingMan->get("work", work, SettingManager::DOM_GAME))
		work = "@home/" + game + "-work";

	// load savegame path. Default is @home/game-save
	Std::string save;
	if (!_settingMan->get("save", save, SettingManager::DOM_GAME))
		save = "@home/" + game + "-save";

	// force creation if it doesn't exist
	_fileSystem->AddVirtualPath("@save", save, true);
	debugN(MM_INFO, "Savegame directory: %s\n", save.c_str());
}

void CoreApp::ParseArgs(const int argc, const char *const *const argv) {
	_parameters.process(argc, argv);
}

GameInfo *CoreApp::getGameInfo(const istring &game) const {
	GameMap::const_iterator i;
	i = _games.find(game);

	if (i != _games.end())
		return i->_value;
	else
		return nullptr;
}

} // End of namespace Ultima8
} // End of namespace Ultima
