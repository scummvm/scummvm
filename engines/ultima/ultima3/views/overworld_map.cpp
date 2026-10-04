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

#include "ultima/ultima3/views/overworld_map.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int VIEWPORT_CELLS = VIEWPORT_TILES * VIEWPORT_TILES;
constexpr int VIEWPORT_CENTER = VIEWPORT_CELLS / 2;
constexpr byte TILE_HIDDEN = 0x24;
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

bool OverworldMap::msgFocus(const FocusMessage &msg) {
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

void OverworldMap::buildViewport(byte *tiles) const {
	const Data::Savegame &save = _G(savegame);
	const int half = VIEWPORT_TILES / 2;

	for (int row = 0; row < VIEWPORT_TILES; ++row) {
		for (int col = 0; col < VIEWPORT_TILES; ++col)
			tiles[row * VIEWPORT_TILES + col] = _G(map).tile(save._posX - half + col, save._posY - half + row);
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

void OverworldMap::drawViewport(GfxSurface &s) {
	byte tiles[VIEWPORT_CELLS];
	buildViewport(tiles);

	for (int row = 0; row < VIEWPORT_TILES; ++row) {
		for (int col = 0; col < VIEWPORT_TILES; ++col)
			_G(shapes).drawTile(s, 8 + col * Gfx::SHAPE_SIZE, 8 + row * Gfx::SHAPE_SIZE,
				tiles[row * VIEWPORT_TILES + col]);
	}

	s.addDirtyRect(Common::Rect(8, 8, 8 + VIEWPORT_TILES * Gfx::SHAPE_SIZE, 8 + VIEWPORT_TILES * Gfx::SHAPE_SIZE));
}

void OverworldMap::endTurn() {
	_logic.incrementMoveCounter();
	_logic.processPartyTurnEffects(true);

	if (!checkPartyWipedOut())
		startPrompt();
}

void OverworldMap::doMove(Direction dir, const char *label) {
	_G(messages).print(label);

	if (!_logic.move(dir)) {
		_G(messages).print("Invalid Move!\n");
		g_engine->playSoundEffect(0xFF);
	}

	endTurn();
}

void OverworldMap::doPass() {
	_G(messages).print("Pass\n");
	endTurn();
}

void OverworldMap::doInvalid() {
	_G(messages).print("<-What?\n");
	g_engine->playSoundEffect(0xFE);
	endTurn();
}

void OverworldMap::idleTimeout() {
	doPass();
}

bool OverworldMap::msgKeypress(const KeypressMessage &msg) {
	if (isModifierKey(msg.keycode))
		return true;
	if (Game::msgKeypress(msg))
		return true;

	if (_gameOver) {
		replaceView("Title", true);
		return true;
	}

	switch (msg.keycode) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		doMove(DIR_NORTH, "North\n");
		break;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		doMove(DIR_SOUTH, "South\n");
		break;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		doMove(DIR_EAST, "East\n");
		break;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		doMove(DIR_WEST, "West\n");
		break;
	case Common::KEYCODE_SPACE:
		doPass();
		break;
	default:
		doInvalid();
		break;
	}

	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
