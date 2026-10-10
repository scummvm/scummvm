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
#include "ultima/ultima3/logic/chest_logic.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/views/interactions/cast_spell.h"
#include "ultima/ultima3/views/interactions/enter_shrine.h"
#include "ultima/ultima3/views/interactions/get_chest.h"
#include "ultima/ultima3/views/interactions/hand_equipment.h"
#include "ultima/ultima3/views/interactions/join_gold.h"
#include "ultima/ultima3/views/interactions/other_command.h"
#include "ultima/ultima3/views/interactions/peer_gem.h"
#include "ultima/ultima3/views/interactions/yell.h"
#include "ultima/ultima3/views/interactions/negate_time.h"
#include "ultima/ultima3/views/interactions/look.h"
#include "ultima/ultima3/views/interactions/steal_chest.h"
#include "ultima/ultima3/views/interactions/transact.h"
#include "ultima/ultima3/views/interactions/unlock_door.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int WHIRLPOOL_FRAMES = 4;
constexpr int BREATH_FRAMES = 2;
constexpr int BREATH_RANGE = 3;
constexpr byte TILE_BREATH = 0x3D;
constexpr byte CELL_SHRINE = 0xF8;
constexpr int VIEWPORT_CELLS = VIEWPORT_TILES * VIEWPORT_TILES;
constexpr int VIEWPORT_CENTER = VIEWPORT_CELLS / 2;
constexpr byte TILE_HIDDEN = 0x24;
constexpr byte TILE_GRASS = 1;
constexpr byte TILE_FOREST = 3;
constexpr byte TILE_MOUNTAINS = 4;
constexpr byte TILE_WALL = 0x23;
constexpr byte TILE_FLOOR = 8;

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

	// Coming up out of a dungeon completes the turn it was left on
	if (msg._priorView && msg._priorView->getName() == "DungeonMap")
		_fightOver = true;

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
	const bool wraps = save._location == Data::LOCATION_SOSARIA || save._location == Data::LOCATION_AMBROSIA;

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

	if (!_breaths.empty()) {
		const Common::Point &p = _breaths[0]._tiles[_breathStep];
		_G(shapes).drawTile(s, 8 + p.x * Gfx::SHAPE_SIZE, 8 + p.y * Gfx::SHAPE_SIZE, TILE_BREATH);
	}

	s.addDirtyRect(Common::Rect(8, 8, 8 + VIEWPORT_TILES * Gfx::SHAPE_SIZE, 8 + VIEWPORT_TILES * Gfx::SHAPE_SIZE));
}

void LocationMap::startFight(int creature) {
	_G(combat).begin(creature);
	_fightOver = true;
	addView("CombatMap");
}

void LocationMap::endTurn() {
	// A fight begun by the party's own command carries on the turn when it's over
	if (_fightStarted) {
		_fightStarted = false;
		return;
	}

	_logic.incrementMoveCounter();

	if (_logic.isAtExit())
		_logic.exitToWorld();

	_logic.processPartyTurnEffects(_G(savegame)._location == Data::LOCATION_SOSARIA);

	// Moon gates take the party to where the moons lead, and whirlpools to Ambrosia
	bool throughGate = _logic.isOnMoonGate();
	if (throughGate) {
		_logic.teleportThroughMoonGate();
		_logic.updateMoons();
	} else {
		_logic.updateMoons();
		if (_logic.isOnMoonGate()) {
			_logic.teleportThroughMoonGate();
			throughGate = true;
		}
	}

	if (!throughGate && _logic.isOnWhirlpool())
		_logic.teleportToAmbrosia();

	// The castle of Exodus breaks any holding of time, and lashes out
	const bool exodus = _logic.isInExodusCastle();
	if (exodus)
		_G(holdTime) = 0;

	// A creature reaching the party starts a fight
	int creature = _creatures.update(_moved);
	_moved = false;

	_pendingCreature = creature;
	startBreaths(_creatures.takeBreaths(), exodus);
	if (_breaths.empty())
		finishTurn(creature);
}

void LocationMap::finishTurn(int creature) {
	if (creature >= 0) {
		startFight(creature);
		return;
	}

	if (!checkPartyWipedOut())
		startPrompt();
}

void LocationMap::startBreaths(const Common::Array<CreatureLogic::Breath> &breaths, bool exodusBolt) {
	byte tiles[VIEWPORT_CELLS];
	buildViewport(tiles);

	const int centre = VIEWPORT_TILES / 2;
	_breathStep = 0;
	_breathFrames = 0;

	if (exodusBolt) {
		// A bolt lands somewhere in view, only doing anything on the party or the floor
		BreathPath bolt;
		int x = g_events->getRandomNumber(VIEWPORT_TILES - 1);
		int y = g_events->getRandomNumber(VIEWPORT_TILES - 1);
		bolt._hits = x == centre && y == centre;

		if (bolt._hits || tiles[y * VIEWPORT_TILES + x] == TILE_FLOOR) {
			bolt._tiles.push_back(Common::Point(x, y));
			_breaths.push_back(bolt);

			if (!bolt._hits)
				g_engine->playSoundEffect(0xF7);
		}
	}

	for (uint i = 0; i < breaths.size(); ++i) {
		BreathPath path;
		path._hits = false;
		int x = breaths[i]._x, y = breaths[i]._y;

		// The blast goes a few tiles in, stopping short of anything in the way
		for (int step = 0; step < BREATH_RANGE; ++step) {
			y += breaths[i]._dy;
			x += breaths[i]._dx;
			if (x < 0 || y < 0 || x >= VIEWPORT_TILES || y >= VIEWPORT_TILES)
				break;

			byte tile = tiles[y * VIEWPORT_TILES + x];
			if (tile == TILE_MOUNTAINS || tile == TILE_WALL || tile == TILE_HIDDEN)
				break;

			path._tiles.push_back(Common::Point(x, y));
			if (x == centre && y == centre) {
				path._hits = true;
				break;
			}
		}

		g_engine->playSoundEffect(0xFB);
		if (!path._tiles.empty())
			_breaths.push_back(path);
	}

	if (!_breaths.empty())
		checkBreathHit();
}

void LocationMap::checkBreathHit() {
	if (_breaths[0]._hits && _breathStep == (int)_breaths[0]._tiles.size() - 1)
		ChestLogic().damageAll(_G(savegame)._dungeonLevel);
}

void LocationMap::processFrame() {
	if (!_breaths.empty()) {
		if (++_breathFrames < BREATH_FRAMES)
			return;
		_breathFrames = 0;

		if (++_breathStep == (int)_breaths[0]._tiles.size()) {
			_breaths.remove_at(0);
			_breathStep = 0;

			if (_breaths.empty()) {
				finishTurn(_pendingCreature);
				return;
			}
		}

		checkBreathHit();
		return;
	}

	if (_fightOver) {
		_fightOver = false;
		endTurn();
		return;
	}

	// While waiting for a command, the whirlpool of the sea drifts along
	if (isWaiting() && !hasInteraction() && !_gameOver && ++_whirlpoolFrames >= WHIRLPOOL_FRAMES) {
		_whirlpoolFrames = 0;

		if (_logic.updateWhirlpool()) {
			_logic.teleportToAmbrosia();
			startPrompt();
		}
	}
}

void LocationMap::attackDirection(Direction dir) {
	const Data::Savegame &save = _G(savegame);
	int x = save._posX, y = save._posY;

	switch (dir) {
	case DIR_NORTH: --y; break;
	case DIR_SOUTH: ++y; break;
	case DIR_EAST: ++x; break;
	default: --x; break;
	}

	int creature = _creatures.creatureAt(x & (Data::MAP_SIZE - 1), y & (Data::MAP_SIZE - 1));
	if (creature >= 0) {
		_fightStarted = true;
		startFight(creature);
	} else {
		_G(messages).print("Not Here!\n");
		g_engine->playSoundEffect(0xFF);
	}
}

namespace {

// Asks which way the party attacks on the map
class MapAttack : public Interactions::Interaction {
private:
	LocationMap *_map;
	Interactions::DirectionChooser _chooser;

public:
	MapAttack(LocationMap *map) : _map(map) {}

	bool keypress(const KeypressMessage &msg) override {
		Interactions::DirectionChooser::Result result = _chooser.handleKey(msg);
		if (result == Interactions::DirectionChooser::PENDING)
			return false;

		if (result == Interactions::DirectionChooser::CHOSEN)
			_map->attackDirection(_chooser.direction());

		return true;
	}
};

} // End of anonymous namespace

void LocationMap::doAttack() {
	_G(messages).print("Attack-");
	startInteraction(new MapAttack(this));
}

void LocationMap::doMove(Direction dir, const char *label) {
	_G(messages).print(label);
	_moved = true;

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

	// The shrines of Ambrosia
	const Data::Savegame &save = _G(savegame);
	if (save._location == Data::LOCATION_AMBROSIA && _G(map).cell(save._posX, save._posY) == CELL_SHRINE) {
		_G(messages).print("shrine!\nWho enters? ");
		startInteraction(new Interactions::EnterShrine());
		return;
	}

	const char *text = _logic.enter();
	if (text) {
		_G(messages).print(text);
		_G(messages).print("Please wait...\n");

		// Dungeons are explored in a view of their own, and don't count as a turn
		if (_G(savegame)._location == Data::LOCATION_DUNGEON)
			replaceView("DungeonMap");
		else
			endTurn();
	} else {
		commandFailed("<-What?\n");
	}
}

void LocationMap::doQuitSave() {
	const Data::Savegame &save = _G(savegame);
	_G(messages).print("Quit & Save\n");

	if (save._location != Data::LOCATION_SOSARIA) {
		_G(messages).print("Only on surface!\n");
		g_engine->playSoundEffect(0xFF);
		endTurn();
		return;
	}

	_G(messages).print(Common::String::format("%02X%02X%02X%02X moves\n", save._moveCount[3],
		save._moveCount[2], save._moveCount[1], save._moveCount[0]).c_str());
	_G(messages).print("Please wait...\n");

	// The game carries on afterwards, so this is just a save made from within it
	redraw();
	g_engine->saveGameDialog();
	endTurn();
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
	if (!_breaths.empty())
		return true;

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
	case 'A':
		doAttack();
		return true;
	case 'G':
		_G(messages).print("Get Chest!\nPlr to search-");
		startInteraction(new Interactions::GetChest());
		return true;
	case 'S':
		_G(messages).print("Steal Chest!\nPlayer? ");
		startInteraction(new Interactions::StealChest());
		return true;
	case 'P':
		_G(messages).print("Peer at gem!\nWhose gem? ");
		startInteraction(new Interactions::PeerGem());
		return true;
	case 'H':
		_G(messages).print("Hand Equipment!\nFrom Player: ");
		startInteraction(new Interactions::HandEquipment());
		return true;
	case 'J':
		_G(messages).print("Join gold to:");
		startInteraction(new Interactions::JoinGold());
		return true;
	case 'Y':
		_G(messages).print("Yell, whom? ");
		startInteraction(new Interactions::Yell());
		return true;
	case 'O':
		_G(messages).print("Other command!\nWhose action? ");
		startInteraction(new Interactions::OtherCommand());
		return true;
	case 'Q':
		doQuitSave();
		return true;
	case 'I':
		_G(messages).print("Ignite a torch\n");
		commandFailed("Not Here!\n", 0xFF);
		return true;
	case 'C':
		_G(messages).print("Cast by whom-");
		startInteraction(new Interactions::CastSpell());
		return true;
	case 'N':
		_G(messages).print("Negate Time!\nWhose Powd? ");
		startInteraction(new Interactions::NegateTime());
		return true;
	case 'T':
		_G(messages).print("Who will\nTransact? ");
		startInteraction(new Interactions::Transact());
		return true;
	case 'U':
		_G(messages).print("Unlock-");
		startInteraction(new Interactions::UnlockDoor());
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
