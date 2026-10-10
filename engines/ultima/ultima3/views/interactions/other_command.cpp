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

#include "ultima/ultima3/views/interactions/other_command.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr int COMMAND_LENGTH = 10;
constexpr byte SHRINE_TILE = 0x3E;
constexpr byte GUARD_TILE = 0x48;
constexpr int BRIBE_COST = 100;

// The places on the world map where something can be dug up
constexpr byte WEAPON_SPOT_X = 0x21, WEAPON_SPOT_Y = 0x03;
constexpr byte ARMOUR_SPOT_X = 0x13, ARMOUR_SPOT_Y = 0x2C;

// The place in Yew where praying gives a hint
constexpr byte PRAYER_TOWN_X = 0x22;
constexpr byte PRAYER_X = 0x30, PRAYER_Y = 0x30;

static void noEffect() {
	_G(messages).print("No effect!\n");
}

void OtherCommand::dig() {
	Data::Savegame &save = _G(savegame);
	Data::RosterEntry &e = save.partyMember(_players.slot());

	if (save._location != Data::LOCATION_SOSARIA) {
		noEffect();
		return;
	}

	// Digging in the right place turns up the exotic weapon or armour
	if (save._posX == WEAPON_SPOT_X && save._posY == WEAPON_SPOT_Y) {
		byte &owned = e._weaponOwned[Data::WEAPON_COUNT - 2];
		owned = Data::toBcd(MIN(Data::fromBcd(owned) + 1, 99));
		save._allWeapons = save._plusTwoWeapons = true;
	} else if (save._posX == ARMOUR_SPOT_X && save._posY == ARMOUR_SPOT_Y) {
		byte &owned = e._armourOwned[Data::ARMOUR_COUNT - 2];
		owned = Data::toBcd(MIN(Data::fromBcd(owned) + 1, 99));
		save._allArmour = save._plusTwoArmour = true;
	} else {
		noEffect();
		return;
	}

	_G(messages).print("Exotics!\n");
}

void OtherCommand::search() {
	const Data::Savegame &save = _G(savegame);
	Data::RosterEntry &e = _G(savegame).partyMember(_players.slot());

	// A card is found on one of the four shrines
	if (_G(map).tile(save._posX, save._posY) != SHRINE_TILE) {
		noEffect();
		return;
	}

	e._marksAndCards |= 1 << (save._posX & 3);
	_G(messages).print("A card, with\nstrange marks!\n");
}

void OtherCommand::pray() {
	const Data::Savegame &save = _G(savegame);

	if (save._location == Data::LOCATION_TOWN && save._worldX == PRAYER_TOWN_X &&
			save._posX == PRAYER_X && save._posY == PRAYER_Y)
		_G(messages).print("Yell 'Evocare'\n\n");
	else
		noEffect();
}

bool OtherCommand::bribe(Direction dir) {
	Data::Savegame &save = _G(savegame);
	Data::RosterEntry &e = save.partyMember(_players.slot());
	Data::Creatures &c = _G(map)._creatures;

	int x = save._posX + (dir == DIR_EAST ? 1 : (dir == DIR_WEST ? -1 : 0));
	int y = save._posY + (dir == DIR_SOUTH ? 1 : (dir == DIR_NORTH ? -1 : 0));
	x &= Data::MAP_SIZE - 1;
	y &= Data::MAP_SIZE - 1;

	// A guard can be paid off, to go away
	for (int i = 0; i < Data::CREATURE_COUNT; ++i) {
		if (!c._tile[i] || c._x[i] != x || c._y[i] != y || c._tile[i] != GUARD_TILE)
			continue;

		int gold = Data::fromBcdWord(e._gold);
		if (gold < BRIBE_COST) {
			_G(messages).print("Not enough gold!\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		e._gold = Data::toBcdWord(gold - BRIBE_COST);
		_G(map).setCell(x, y, c._floor[i]);
		c._tile[i] = 0;
		return true;
	}

	noEffect();
	return true;
}

bool OtherCommand::keypress(const KeypressMessage &msg) {
	Data::Savegame &save = _G(savegame);

	switch (_stage) {
	case CHOOSE_PLAYER: {
		PlayerChooser::Result result = _players.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;
		if (result == PlayerChooser::CANCELLED)
			return true;

		if (!save.partyMember(_players.slot()).isAlive()) {
			_G(messages).print("Incapacitated!\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		_G(messages).print("Cmd: ");
		_text.setup(COMMAND_LENGTH);
		_stage = TYPE_COMMAND;
		return false;
	}

	case TYPE_COMMAND: {
		if (!_text.handleKey(msg))
			return false;

		_G(messages).print("\n");
		Common::String word = _text.word();

		if (word == "DIG") {
			dig();
		} else if (word == "SEARCH") {
			search();
		} else if (word == "PRAY") {
			pray();
		} else if (word == "BRIBE") {
			_G(messages).print("Direct? ");
			_stage = DIRECTION;
			return false;
		} else {
			noEffect();
		}

		return true;
	}

	default: {
		DirectionChooser::Result result = _directions.handleKey(msg);
		if (result == DirectionChooser::PENDING)
			return false;
		if (result == DirectionChooser::CANCELLED)
			return true;

		return bribe(_directions.direction());
	}
	}
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
