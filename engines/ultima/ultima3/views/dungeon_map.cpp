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

#include "ultima/ultima3/views/dungeon_map.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/views/interactions/cast_spell.h"
#include "ultima/ultima3/views/interactions/dungeon_special.h"
#include "ultima/ultima3/views/interactions/get_chest.h"
#include "ultima/ultima3/views/interactions/hand_equipment.h"
#include "ultima/ultima3/views/interactions/join_gold.h"
#include "ultima/ultima3/views/interactions/other_command.h"
#include "ultima/ultima3/views/interactions/peer_gem.h"
#include "ultima/ultima3/views/interactions/yell.h"
#include "ultima/ultima3/views/interactions/ignite_torch.h"
#include "ultima/ultima3/views/interactions/negate_time.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int VIEW_LEFT = 8;
constexpr int VIEW_TOP = 8;
constexpr int VIEW_SIZE = 176;
constexpr int FIRST_SPECIAL = 1;
constexpr int LAST_SPECIAL = 6;

// What is shown for each way of facing at the bottom of the frame
static const char *const FACING_NAMES[4] = { "North", "-East", "South", "-West" };

bool DungeonMap::msgFocus(const FocusMessage &msg) {
	const Common::String prior = msg._priorView ? msg._priorView->getName() : "";
	_viewValid = false;

	if (prior == "MainMenu" || _G(resumeGame)) {
		startGame();
		_G(shapes).load();
		_G(resumeGame) = false;
		beginCommand();
	} else if (prior == "LocationMap") {
		beginCommand();
	} else if (prior == "CombatMap") {
		_turnPending = true;
	}

	return Game::msgFocus(msg);
}

void DungeonMap::beginCommand() {
	if (checkPartyWipedOut())
		return;

	if (_G(savegame)._lightTurns == 0)
		_G(messages).print("It's dark!\n");

	startPrompt();
}

void DungeonMap::processFrame() {
	if (_turnPending) {
		_turnPending = false;
		endTurn();
	}
}

void DungeonMap::drawViewport(GfxSurface &s) {
	const Data::Savegame &save = _G(savegame);

	if (save._lightTurns > 0) {
		// The view is only redrawn once something has changed it, or the party
		// has been moved by something other than a command
		uint32 key = (save._dungeonLevel << 24) | (save._posX << 16) | (save._posY << 8) | save._facing;
		if (!_viewValid || key != _viewKey) {
			_view.draw(_G(dungeon), save._dungeonLevel, save._posX, save._posY, save._facing);
			_viewValid = true;
			_viewKey = key;
		}

		for (int y = VIEW_TOP; y < VIEW_TOP + VIEW_SIZE; ++y) {
			for (int x = VIEW_LEFT; x < VIEW_LEFT + VIEW_SIZE; ++x) {
				if (_view.isLit(x, y))
					s.setPixel(x, y, 3);
			}
		}
	}

	s.addDirtyRect(Common::Rect(VIEW_LEFT, VIEW_TOP, VIEW_LEFT + VIEW_SIZE, VIEW_TOP + VIEW_SIZE));
}

void DungeonMap::drawLabels(GfxSurface &s) {
	const Data::Savegame &save = _G(savegame);

	// The level is written over the middle of the moons' label
	drawMoons(s);
	s.writeString(Common::Point(9, 0), Common::String::format("LVL:0%d", save._dungeonLevel + 1));
	s.writeString(Common::Point(6, 23), Common::String::format("%cHead-%s%c", 0x10,
		FACING_NAMES[save._facing & 3], 0x11));
}

void DungeonMap::idleTimeout() {
	_G(messages).print("Pass\n");
	endTurn();
}

void DungeonMap::endTurn() {
	Data::Savegame &save = _G(savegame);

	// The squares that make something happen don't take up a turn
	if (_special) {
		_special = false;
		beginCommand();
		return;
	}

	// Climbing out, by ladder or spell, goes on with the turn above ground
	if (save._location != Data::LOCATION_DUNGEON) {
		replaceView("LocationMap");
		return;
	}

	_logic.incrementMoveCounter();
	_logic.processPartyTurnEffects(false);
	if (save._lightTurns > 0)
		--save._lightTurns;
	_viewValid = false;

	byte tile = _logic.tile();
	if (tile == 0) {
		int monsters = _logic.rollEncounter();

		if (monsters >= 0) {
			_G(combat).beginDungeon(monsters);
			_turnPending = true;
			addView("CombatMap");
			return;
		}
	} else if (tile == Data::DTILE_SIGN) {
		// Writing on the wall for this level, which then fades
		const char *sign = _G(dungeon).sign(save._dungeonLevel);
		_G(dungeon).setTile(save._dungeonLevel, save._posX, save._posY, 0);
		_G(messages).print("Misty writing:\n");
		if (sign)
			_G(messages).print(sign);
		_G(messages).print("\n");
	} else if (tile >= FIRST_SPECIAL && tile <= LAST_SPECIAL) {
		_special = true;
		startInteraction(new Interactions::DungeonSpecial(tile));
		return;
	}

	beginCommand();
}

void DungeonMap::doMove(bool forward) {
	_G(messages).print(forward ? "Advance\n" : "Retreat\n");

	if (!(forward ? _logic.moveForward() : _logic.moveBackward())) {
		_G(messages).print("Invalid Move!\n");
		g_engine->playSoundEffect(0xFF);
	}

	endTurn();
}

void DungeonMap::doTurn(bool right) {
	_G(messages).print(right ? "Turn right\n" : "Turn left\n");

	if (!_logic.turn(right)) {
		_G(messages).print("Invalid Move!\n");
		g_engine->playSoundEffect(0xFF);
	}

	endTurn();
}

void DungeonMap::doClimb() {
	_G(messages).print("Klimb\n");

	switch (_logic.climb()) {
	case DungeonLogic::LADDER_NONE:
		_G(messages).print("Invalid Move!\n");
		g_engine->playSoundEffect(0xFF);
		break;
	default:
		break;
	}

	endTurn();
}

void DungeonMap::doDescend() {
	_G(messages).print("Descend\n");

	if (!_logic.descend()) {
		_G(messages).print("Invalid Move!\n");
		g_engine->playSoundEffect(0xFF);
	}

	endTurn();
}

bool DungeonMap::handleCommand(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		doMove(true);
		return true;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		doMove(false);
		return true;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		doTurn(true);
		return true;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		doTurn(false);
		return true;
	case Common::KEYCODE_SPACE:
		_G(messages).print("Pass\n");
		endTurn();
		return true;
	default:
		break;
	}

	switch (commandKey(msg)) {
	case 'K':
		doClimb();
		return true;
	case 'D':
		doDescend();
		return true;
	case 'C':
		_G(messages).print("Cast by whom-");
		startInteraction(new Interactions::CastSpell());
		return true;
	case 'G':
		_G(messages).print("Get Chest!\nPlr to search-");
		startInteraction(new Interactions::GetChest());
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
	case 'I':
		_G(messages).print("Ignite a torch\nWhose torch: ");
		startInteraction(new Interactions::IgniteTorch());
		return true;
	case 'N':
		_G(messages).print("Negate Time!\nWhose Powd? ");
		startInteraction(new Interactions::NegateTime());
		return true;

	// Commands that only make sense above ground
	case 'B': case 'A': case 'E': case 'F': case 'L': case 'Q': case 'T':
	case 'U': case 'X': case 'S':
		commandFailed("Not a DNG cmd!\n", 0xFF);
		return true;

	default:
		break;
	}

	if (!Game::handleCommand(msg))
		commandFailed("<-What?\n");
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
