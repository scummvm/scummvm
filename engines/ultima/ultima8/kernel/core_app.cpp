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
#include "ultima/ultima8/games/game_detector.h"
#include "ultima/shared/std/misc.h"

namespace Ultima {
namespace Ultima8 {

using Std::string;

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(CoreApp)

CoreApp *CoreApp::application = 0;

CoreApp::CoreApp(const Ultima::UltimaGameDescription *gameDesc)
		: _gameDesc(gameDesc), isRunning(false), gameinfo(0), filesystem(0),
		configfileman(0), settingman(0), oHelp(false), oQuiet(false), oVQuiet(false) {
	application = this;
	_console = new Console();
}

CoreApp::~CoreApp() {
	for (GameMap::iterator i = games.begin(); i != games.end(); ++i)
		delete i->_value;

	games.clear();

	FORGET_OBJECT(filesystem);
	FORGET_OBJECT(settingman);
	FORGET_OBJECT(configfileman);
	FORGET_OBJECT(gameinfo);
	FORGET_OBJECT(_console);

	application = 0;
}

void CoreApp::startup() {
	DeclareArgs(); // Note: this is virtual

	ParseArgs(0, 0);

	// if we're spitting out help, we probably want to avoid having the
	// other cruft dumped too...
	if (oHelp) {
		oQuiet = oVQuiet = true;
	}
	if (oQuiet)
		con->setMsgMask(static_cast<MsgMask>(MM_ALL & ~MM_INFO &
		                                    ~MM_MINOR_WARN));
	if (oVQuiet)
		con->setMsgMask(static_cast<MsgMask>(MM_ALL & ~MM_INFO & ~MM_MINOR_WARN
		                                    & ~MM_MAJOR_WARN & ~MM_MINOR_ERR));

	if (oHelp) {
		helpMe(); // Note: this is virtual
		error("Startup failed");
	}


	sysInit();

	loadConfig(); // load config files
}

void CoreApp::DeclareArgs() {
	parameters.declare("--game",    &oGamename, "");
	parameters.declare("-h",        &oHelp,     true);
	parameters.declare("--help",    &oHelp,     true);
	parameters.declare("-q",        &oQuiet,    true);
	parameters.declare("-qq",       &oVQuiet,   true);
}

void CoreApp::sysInit() {
	gameinfo = 0;

	filesystem = new FileSystem;

	configfileman = new ConfigFileManager();
	settingman = new SettingManager();
	settingman->setDomainName(SettingManager::DOM_GLOBAL, "pentagram");
	settingman->setCurrentDomain(SettingManager::DOM_GLOBAL);
}

// load configuration files
void CoreApp::loadConfig() {
	pout << "Loading configuration files:" << Std::endl;

	bool dataconf, homeconf;

	// system-wide config, read-only
	dataconf = settingman->readConfigFile("@data/pentagram.ini", true);

	// user config
	homeconf = settingman->readConfigFile("@home/pentagram.ini");

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
	if (settingman->get("data", data, SettingManager::DOM_GLOBAL)) {
		pout << "Setting custom data path: " << data << Std::endl;
		bool ok = filesystem->AddVirtualPath("@data", data);
		if (!ok) {
			perr << "Error opening data directory." << Std::endl;
		}
	}
}

void CoreApp::setupGameList() {
	Std::vector<Pentagram::istring> gamelist;
	gamelist = settingman->listGames();
	con->Print(MM_INFO, "Scanning config file for games:\n");
	Std::vector<Pentagram::istring>::iterator iter;
	Pentagram::istring gamename;

	for (iter = gamelist.begin(); iter != gamelist.end(); ++iter) {
		Pentagram::istring game = *iter;
		GameInfo *info = new GameInfo;
		bool detected = getGameInfo(game, info);

		// output detected game info
		con->Printf(MM_INFO, "%s: ", game.c_str());
		if (detected) {
			// add game to games map
			games[game] = info;

			Std::string details = info->getPrintDetails();
			con->Print(MM_INFO, details.c_str());
		} else {
			con->Print(MM_INFO, "unknown, skipping");
		}
		con->Print(MM_INFO, "\n");
	}
}

GameInfo *CoreApp::getDefaultGame() {
	Pentagram::istring gamename;

	Std::string defaultgame;
	bool defaultset = settingman->get("defaultgame", defaultgame,
	                                  SettingManager::DOM_GLOBAL);

	if (oGamename != "") {
		// game specified on commandline
		gamename = oGamename;

	} else if (defaultset) {
		// default game specified in config file
		gamename = defaultgame;

	} else if (games.size() == 2) {// TODO - Do this in a better method
		// only one game in config file, so pick that
		for (GameMap::iterator i = games.begin(); i != games.end(); ++i) {
			if (i->_value->name != "pentagram")
				gamename = i->_value->name;
		}

	} else if (games.size() == 1) {
		gamename = games.begin()->_value->name;

	} else {
		perr << "Multiple games found in configuration, but no default "
		     << "game is selected." << Std::endl
		     << "Either start Pentagram with the \"--game <gamename>\","
		     << Std::endl
		     << "or set pentagram/defaultgame in pentagram.ini"
		     << Std::endl;  // FIXME - report more useful error message
		return 0;
	}

	pout << "Default game: " << gamename << Std::endl;

	GameInfo *info = getGameInfo(gamename);

	if (!info) {
		perr << "Game \"" << gamename << "\" not found." << Std::endl;
	}

	// We've got a default game name, doesn't mean it will work though
	return info;
}

bool CoreApp::setupGame(GameInfo *info) {
	if (!info) return false;
	assert(info->name != "");

	gameinfo = info;

	pout << "Selected game: " << info->name << Std::endl;
	pout << info->getPrintDetails() << Std::endl;

	setupGamePaths(info);

	return info->name != "pentagram";
}

void CoreApp::killGame() {
	// Save the settings!
	pout << "Saving settings" << Std::endl;
	settingman->write();

	filesystem->RemoveVirtualPath("@game");
	filesystem->RemoveVirtualPath("@work");
	filesystem->RemoveVirtualPath("@save");

	configfileman->clearRoot("bindings");
	configfileman->clearRoot("language");
	configfileman->clearRoot("weapons");
	configfileman->clearRoot("armour");
	configfileman->clearRoot("monsters");
	configfileman->clearRoot("game");
	settingman->setCurrentDomain(SettingManager::DOM_GLOBAL);

	gameinfo = 0;
}


bool CoreApp::getGameInfo(Pentagram::istring &game, GameInfo *ginfo) {
	// first try getting the information from the config file
	// if that fails, try to autodetect it

	ginfo->name = game;
	ginfo->type = GameInfo::GAME_UNKNOWN;
	ginfo->version = 0;
	ginfo->language = GameInfo::GAMELANG_UNKNOWN;

	Pentagram::istring gamekey = "settings/";
	gamekey += game;

	if (game == "pentagram") {
		ginfo->type = GameInfo::GAME_PENTAGRAM_MENU;
		ginfo->language = GameInfo::GAMELANG_ENGLISH;

	} else {
		assert(game == "ultima8");

		ginfo->type = GameInfo::GAME_U8;
		
		switch (_gameDesc->desc.language) {
		case Common::EN_ANY:
			ginfo->language = GameInfo::GAMELANG_ENGLISH;
			break;
		case Common::FR_FRA:
			ginfo->language = GameInfo::GAMELANG_FRENCH;
			break;
		case Common::DE_DEU:
			ginfo->language = GameInfo::GAMELANG_GERMAN;
			break;
		case Common::ES_ESP:
			ginfo->language = GameInfo::GAMELANG_SPANISH;
			break;
		case Common::JA_JPN:
			ginfo->language = GameInfo::GAMELANG_JAPANESE;
			break;
		default:
			error("Unknown language");
			break;
		}
	}

	return ginfo->type != GameInfo::GAME_UNKNOWN;
}

void CoreApp::setupGamePaths(GameInfo *ginfo) {
	if (!ginfo || ginfo->name == "pentagram") {
		settingman->setCurrentDomain(SettingManager::DOM_GLOBAL);
		return;
	}

	Pentagram::istring game = ginfo->name;

	settingman->setDomainName(SettingManager::DOM_GAME, game);
	settingman->setCurrentDomain(SettingManager::DOM_GAME);

	// load main game data path
	Std::string gpath;
	settingman->get("path", gpath, SettingManager::DOM_GAME);
	filesystem->AddVirtualPath("@game", gpath);

	// load work path. Default is @home/game-work
	// where 'game' in the above is the specified 'game' loaded
	Std::string work;
	if (!settingman->get("work", work, SettingManager::DOM_GAME))
		work = "@home/" + game + "-work";

#if 0
	// force creation if it doesn't exist

	// TODO: I don't like these being created here.
	//       I'd prefer them to be created when needed. (-wjp)

	filesystem->AddVirtualPath("@work", work, true);
	con->Printf(MM_INFO, "U8 Workdir: %s\n", work.c_str()); //!!FIXME (u8)

	// make sure we've got a minimal sane filesystem under there...
	filesystem->MkDir("@work/usecode");
	filesystem->MkDir("@work/usecode/obj");
	filesystem->MkDir("@work/usecode/src");
	filesystem->MkDir("@work/usecode/asm");
#endif

	// load savegame path. Default is @home/game-save
	Std::string save;
	if (!settingman->get("save", save, SettingManager::DOM_GAME))
		save = "@home/" + game + "-save";

	// force creation if it doesn't exist
	filesystem->AddVirtualPath("@save", save, true);
	con->Printf(MM_INFO, "Savegame directory: %s\n", save.c_str());
}

void CoreApp::ParseArgs(const int argc_, const char *const *const argv_) {
	parameters.process(argc_, argv_);
}

void CoreApp::helpMe() {
	con->Print("\t-h\t\t- quick help menu (this)\n");
	con->Print("\t-q\t\t- silence general logging messages\n");
	con->Print("\t-qq\t\t- silence general logging messages and\n\t\t\t  non-critical warnings/errors\n");
	con->Print("\t--game {name}\t- select a game\n");
}

GameInfo *CoreApp::getGameInfo(Pentagram::istring game) const {
	GameMap::const_iterator i;
	i = games.find(game);

	if (i != games.end())
		return i->_value;
	else
		return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
