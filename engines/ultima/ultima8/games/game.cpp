/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/games/u8_game.h"
#include "ultima/ultima8/games/cru_game.h"
#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/gumps/main_menu_process.h"

namespace Ultima {
namespace Ultima8 {

Game *Game::_game = nullptr;

Game::Game() {
	_game = this;
}

Game::~Game() {
	assert(_game == this);
	_game = nullptr;
}


// static
Game *Game::createGame(const GameInfo *info) {
	switch (info->_type) {
	case GameInfo::GAME_U8:
		return new U8Game();
	case GameInfo::GAME_REMORSE:
	case GameInfo::GAME_REGRET:
		return new CruGame();
	default:
		error("createGame: invalid game tyoe");
	}

	return nullptr;
}

uint32 Game::I_playEndgame(const uint8 *args, unsigned int /*argsize*/) {
	ConfMan.setBool("endgame", true);
	ConfMan.setBool("quotes", true);
	ConfMan.flushToDisk();

	PaletteManager *palman = PaletteManager::get_instance();
	palman->untransformPalette(PaletteManager::Pal_Game);

	Process *menuproc = new MainMenuProcess();
	Kernel::get_instance()->addProcess(menuproc);

	ProcId moviepid = Game::get_instance()->playEndgameMovie(false);
	Process *movieproc = Kernel::get_instance()->getProcess(moviepid);
	if (movieproc) {
		menuproc->waitFor(movieproc);
	}

	return 0;
}

uint32 Game::I_playCredits(const uint8 */*args*/, unsigned int /*argsize*/) {
	Game::get_instance()->playCredits();
	return 0;
}

uint32 Game::I_playDemoScreen(const uint8 */*args*/, unsigned int /*argsize*/) {
	Game::get_instance()->playDemoScreen();
	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
