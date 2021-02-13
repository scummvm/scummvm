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

#ifndef ULTIMA8_KERNEL_COREAPP_H
#define ULTIMA8_KERNEL_COREAPP_H

#include "ultima/ultima8/games/game_info.h"
#include "ultima/detection.h"

namespace Ultima {
namespace Ultima8 {

class Console;
class FileSystem;
class ConfigFileManager;
struct GameInfo;


#define GAME_IS_U8 (CoreApp::get_instance()->getGameInfo()->_type == GameInfo::GAME_U8)
#define GAME_IS_REMORSE (CoreApp::get_instance()->getGameInfo()->_type == GameInfo::GAME_REMORSE)
#define GAME_IS_REGRET (CoreApp::get_instance()->getGameInfo()->_type == GameInfo::GAME_REGRET)
#define GAME_IS_CRUSADER (GAME_IS_REMORSE || GAME_IS_REGRET)


class CoreApp {
public:
	CoreApp(const Ultima::UltimaGameDescription *gameDesc);
	virtual ~CoreApp();

	static CoreApp *get_instance() {
		return _application;
	};

	virtual bool runGame() = 0;
	virtual void paint() = 0; // probably shouldn't exist

	//! Startup the application. This will prepare the application for run().
	//! Should call parent class' startup().
	void startup();

	//! Get current GameInfo struct
	const GameInfo *getGameInfo() const {
		return _gameInfo;
	}

protected:
	bool _isRunning;
	GameInfo *_gameInfo;

	// minimal system
	FileSystem *_fileSystem;
	ConfigFileManager *_configFileMan;

	static CoreApp *_application;

private:
	const Ultima::UltimaGameDescription *_gameDesc;

	//! Fill a GameInfo struct for the give game name
	//! \param game The id of the game to check (from pentagram.cfg)
	//! \param gameinfo The GameInfo struct to fill
	//! \return true if detected all the fields, false if detection failed
	bool getGameInfo(const istring &game, GameInfo *gameinfo);

protected:
	//! Setup up a game
	//! \return false if failed
	bool setupGame();

	//! kill current gameinfo
	void killGame();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
