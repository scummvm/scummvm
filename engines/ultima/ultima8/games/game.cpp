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
#include "ultima/ultima8/games/game.h"
#include "ultima/ultima8/games/u8_game.h"
#include "ultima/ultima8/games/remorse_game.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/graphics/palette_manager.h"
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
		return new RemorseGame();
	default:
		CANT_HAPPEN_MSG("createGame: invalid _game");
	}

	return nullptr;
}

uint32 Game::I_playEndgame(const uint8 *args, unsigned int /*argsize*/) {
	SettingManager *settingman = SettingManager::get_instance();
	settingman->set("endgame", true);
	settingman->write();

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

uint32 Game::I_playCredits(const uint8 *args, unsigned int /*argsize*/) {
	perr << "TODO: Implement I_playCredits";
	// TODO: need a process to wait for here.
	// Should fade out, credits, fade in.
	// Double-check in the disasm that this should just play credits and
	// not also endgame movie.
	// Game::get_instance()->playCredits();

	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
