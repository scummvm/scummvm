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

#include "ultima/ultima1/data/game.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

void Game::startGame() {
	_G(map).load(_G(player)._mapNum);

	// TODO: dungeons aren't implemented yet, so always start on the overworld
	g_engine->replaceView("Overworld");

	promptForCommand();
}

void Game::message(const Common::String &name, const char *param) {
	GameMessage msg(name);
	if (param)
		msg._stringValue = param;
	g_engine->focusedView()->send(msg);
}

void Game::promptForCommand() {
	g_engine->focusedView()->send(GameMessage("COMMAND"));
}

void Game::doCommand(int action) {
	switch (action) {
	case KEYBIND_PASS:
		message("INFO", "PASS");
		break;

	case KEYBIND_UP:


	default:
		message("INFO", "-ILLEGAL COMMAND!");
		break;
	}

	endOfTurn();
}

void Game::endOfTurn() {
	auto &player = g_engine->_player;

	subtractFoodCtr(10);
	if (!player.isDead()) {
		g_engine->focusedView()->redraw();
		promptForCommand();
	}
}

void Game::subtractFoodCtr(int amount) {
	auto &player = g_engine->_player;
	if (player.isDead())
		return;		// Already dead

	player._food = MAX((int)player._food - amount, 0);
	if (player._food == 0)
		dead();
}

void Game::subtractHp(int amount) {
	auto &player = g_engine->_player;
	player._hits = MAX((int)player._hits - amount, 0);

	if (player._hits == 0)
		dead();
}

void Game::dead() {
	auto &player = g_engine->_player;
	player._hits = 0;
	player._food = 0;
	player._experience = 0;
	player._coins = 0;

	message("DEAD");
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
