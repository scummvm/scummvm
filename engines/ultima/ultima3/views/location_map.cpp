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

#include "ultima/ultima3/views/location_map.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/views/interactions/look.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int VIEWPORT_CELLS = VIEWPORT_TILES * VIEWPORT_TILES;
constexpr int VIEWPORT_CENTER = VIEWPORT_CELLS / 2;
constexpr byte TILE_HIDDEN = 0x24;
constexpr byte TILE_GRASS = 1;
constexpr byte TILE_FOREST = 3;
constexpr byte TILE_MOUNTAINS = 4;
constexpr byte TILE_WALL = 0x23;

// Tiles that block the view of whatever lies behind them
static bool blocksView(byte tile) {
	return tile == TILE_FOREST || tile == TILE_MOUNTAINS || tile == TILE_WALL || tile == TILE_HIDDEN;
}

// The direction a position at a given offset from the centre should step to
// get a line closer to the middle of the viewport
static int stepToCentre(int pos) {
	const int centre = VIEWPORT_TILES / 2;
	return pos < centre ? 1 : (pos > centre ? -1 : 0);
}

bool LocationMap::msgFocus(const FocusMessage &msg) {
	// The game starts when arriving from the main menu, or continues on from a load
	bool fromMenu = msg._priorView && msg._priorView->getName() == "MainMenu";
	if (fromMenu || _G(resumeGame)) {
		startGame();
		_G(shapes).load();

		if (!_G(savegame)._mapLoaded) {
			_G(map).load("SOSARIA.ULT");
			_G(savegame)._mapLoaded = true;
		}

		_G(resumeGame) = false;
		startPrompt();
	}

	return Game::msgFocus(msg);
}

void LocationMap::buildViewport(byte *tiles) const {
	const Data::Savegame &save = _G(savegame);
	const int half = VIEWPORT_TILES / 2;

	// The world wraps around, whereas beyond the edge of a town is open grass
	const bool wraps = save._location == Data::LOCATION_SOSARIA;

	for (int row = 0; row < VIEWPORT_TILES; ++row) {
		for (int col = 0; col < VIEWPORT_TILES; ++col) {
			int x = save._posX - half + col, y = save._posY - half + row;
			bool outside = x < 0 || x >= Data::MAP_SIZE || y < 0 || y >= Data::MAP_SIZE;

			tiles[row * VIEWPORT_TILES + col] = (outside && !wraps) ? TILE_GRASS : _G(map).tile(x, y);
		}
	}

	tiles[VIEWPORT_CENTER] = save._transport;

	// Working from the far corner in, hide each tile if the straight run of
	// tiles between it and the party is blocked. Tiles hidden earlier hide
	// in turn whatever is behind them
	for (int idx = VIEWPORT_CELLS - 1; idx >= 0; --idx) {
		int col = idx % VIEWPORT_TILES, row = idx / VIEWPORT_TILES;
		int pos = idx;

		for (;;) {
			pos += stepToCentre(col) + stepToCentre(row) * VIEWPORT_TILES;
			if (pos == VIEWPORT_CENTER)
				break;

			if (blocksView(tiles[pos])) {
				tiles[idx] = TILE_HIDDEN;
				break;
			}

			col += stepToCentre(col);
			row += stepToCentre(row);
		}
	}
}

void LocationMap::drawViewport(GfxSurface &s) {
	byte tiles[VIEWPORT_CELLS];
	buildViewport(tiles);

	for (int row = 0; row < VIEWPORT_TILES; ++row) {
		for (int col = 0; col < VIEWPORT_TILES; ++col)
			_G(shapes).drawTile(s, 8 + col * Gfx::SHAPE_SIZE, 8 + row * Gfx::SHAPE_SIZE,
				tiles[row * VIEWPORT_TILES + col]);
	}

	s.addDirtyRect(Common::Rect(8, 8, 8 + VIEWPORT_TILES * Gfx::SHAPE_SIZE, 8 + VIEWPORT_TILES * Gfx::SHAPE_SIZE));
}

void LocationMap::endTurn() {
	_logic.incrementMoveCounter();

	if (_logic.isAtExit()) {
		_G(messages).print("Exit to Sosaria!\nPlease wait...\n");
		_logic.exitToWorld();
	}

	_logic.processPartyTurnEffects(_G(savegame)._location == Data::LOCATION_SOSARIA);

	if (!checkPartyWipedOut())
		startPrompt();
}

void LocationMap::doMove(Direction dir, const char *label) {
	_G(messages).print(label);

	if (!_logic.move(dir)) {
		_G(messages).print("Invalid Move!\n");
		g_engine->playSoundEffect(0xFF);
	}

	endTurn();
}

void LocationMap::doPass() {
	_G(messages).print("Pass\n");
	endTurn();
}

void LocationMap::doInvalid() {
	_G(messages).print("<-What?\n");
	g_engine->playSoundEffect(0xFE);
	endTurn();
}

void LocationMap::idleTimeout() {
	doPass();
}

void LocationMap::doBoard() {
	const char *text = _logic.board();
	if (text) {
		_G(messages).print(text);
		endTurn();
	} else {
		_G(messages).print("Board");
		commandFailed("<-What?\n");
	}
}

void LocationMap::doEnter() {
	_G(messages).print("Enter ");

	const char *text = _logic.enter();
	if (text) {
		_G(messages).print(text);
		_G(messages).print("Please wait...\n");
		endTurn();
	} else {
		commandFailed("<-What?\n");
	}
}

void LocationMap::doExitVehicle() {
	_G(messages).print("X-it ");

	switch (_logic.exitVehicle()) {
	case LocationLogic::EXIT_DONE:
		_G(messages).print("Craft\n");
		endTurn();
		break;
	case LocationLogic::EXIT_NOT_HERE:
		commandFailed("Not Here!\n", 0xFF);
		break;
	default:
		commandFailed("<-What?\n");
		break;
	}
}

bool LocationMap::handleCommand(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		doMove(DIR_NORTH, "North\n");
		return true;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		doMove(DIR_SOUTH, "South\n");
		return true;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		doMove(DIR_EAST, "East\n");
		return true;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		doMove(DIR_WEST, "West\n");
		return true;
	case Common::KEYCODE_SPACE:
		doPass();
		return true;
	default:
		break;
	}

	switch (commandKey(msg)) {
	case 'B':
		doBoard();
		return true;
	case 'X':
		doExitVehicle();
		return true;
	case 'E':
		doEnter();
		return true;
	case 'L':
		_G(messages).print("Look-");
		startInteraction(new Interactions::Look());
		return true;
	default:
		break;
	}

	if (!Game::handleCommand(msg))
		doInvalid();
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
