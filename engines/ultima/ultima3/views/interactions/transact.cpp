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

#include "ultima/ultima3/views/interactions/transact.h"
#include "ultima/ultima3/views/interactions/shops.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr byte FIRST_COUNTER_CELL = 0x94;
constexpr byte LAST_COUNTER_CELL = 0xE4;
constexpr byte CELL_MERCHANT = 0x40;
constexpr byte CELL_LORD_BRITISH = 0x4C;
constexpr byte FLAGS_HOSTILE = 0xC0;
constexpr byte MARK_OF_KINGS = 0x80;

// The levels that can be reached without the Mark of Kings, and the most there are
constexpr int LEVEL_NEEDING_MARK = 5;
constexpr int LEVEL_LIMIT = 0x25;

Transact::~Transact() {
	_G(effects)._highlight = -1;
}

void Transact::lordBritish() {
	Data::RosterEntry &e = _G(savegame).partyMember(_players.slot());
	byte level = e._maxHitPoints >> 8;

	_G(messages).print("\nWelcome my child\n");

	if (level > (e._experience >> 8)) {
		_G(messages).print("Experience more!\n\n");
	} else if (level >= LEVEL_LIMIT) {
		_G(messages).print("No more!\n");
	} else if (level >= LEVEL_NEEDING_MARK && !(e._marksAndCards & MARK_OF_KINGS)) {
		_G(messages).print("Seek ye, the\nMark of Kings!\n");
	} else {
		e._maxHitPoints = (e._maxHitPoints & 0xFF) | (Data::toBcd(Data::fromBcd(level) + 1) << 8);
		_G(messages).print("Thou art greater\n\n");
		_G(effects).flashViewport();
		g_engine->playSoundEffect(0xFD);
	}
}

void Transact::talkTo(int creature) {
	const Data::Creatures &c = _G(map)._creatures;

	if (c._tile[creature] == CELL_LORD_BRITISH) {
		lordBritish();
		return;
	}

	byte flags = c._flags[creature];
	if (flags == FLAGS_HOSTILE) {
		_G(messages).print("\nEat Death Scum!\n\n");
	} else if ((flags & 0x3F) == 0) {
		_G(messages).print("\nGood day!\n\n");
	} else {
		const char *text = _G(map).text((flags & 0x3F) - 1);
		if (text)
			_G(messages).print(text);
	}
}

bool Transact::startShop(int dx, int dy) {
	const Data::Savegame &save = _G(savegame);

	byte counter = _G(map).cell(save._posX + dx, save._posY + dy);
	if (counter < FIRST_COUNTER_CELL || counter > LAST_COUNTER_CELL)
		return false;
	if (_G(map).cell(save._posX + dx * 2, save._posY + dy * 2) != CELL_MERCHANT)
		return false;

	const int payer = _players.slot();
	_G(effects)._highlight = payer;

	// Horses are bought facing west, and the rest according to where the shop is
	if (dx < 0) {
		_shop.reset(new Stable(payer));
		return true;
	}

	switch (save._posY & 7) {
	case 0:
		_shop.reset(new Tavern(payer));
		break;
	case 1:
		_shop.reset(new Grocer(payer));
		break;
	case 2:
		_shop.reset(new Temple(payer));
		break;
	case 3:
		_shop.reset(new EquipmentShop(payer, true));
		break;
	case 4:
		_shop.reset(new EquipmentShop(payer, false));
		break;
	case 5:
		_shop.reset(new Guild(payer));
		break;
	case 6:
		_shop.reset(new Oracle(payer));
		break;
	default:
		_shop.reset(new Stable(payer));
		break;
	}

	return true;
}

bool Transact::keypress(const KeypressMessage &msg) {
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

		_G(messages).print("Direct? ");
		_stage = CHOOSE_DIRECTION;
		return false;
	}

	case CHOOSE_DIRECTION: {
		DirectionChooser::Result result = _directions.handleKey(msg);
		if (result == DirectionChooser::PENDING)
			return false;
		if (result == DirectionChooser::CANCELLED)
			return true;

		int dx = 0, dy = 0;
		switch (_directions.direction()) {
		case DIR_NORTH: dy = -1; break;
		case DIR_SOUTH: dy = 1; break;
		case DIR_EAST: dx = 1; break;
		default: dx = -1; break;
		}

		// Someone standing there is spoken to
		const Data::Creatures &c = _G(map)._creatures;
		int x = (save._posX + dx) & (Data::MAP_SIZE - 1), y = (save._posY + dy) & (Data::MAP_SIZE - 1);
		for (int i = Data::CREATURE_COUNT - 1; i >= 0; --i) {
			if (c._tile[i] && c._x[i] == x && c._y[i] == y) {
				talkTo(i);
				return true;
			}
		}

		if (startShop(dx, dy)) {
			_stage = IN_SHOP;
			return false;
		}

		_G(messages).print("Not Here!\n");
		g_engine->playSoundEffect(0xFF);
		return true;
	}

	default:
		if (_shop->keypress(msg)) {
			_shop.reset();
			return true;
		}

		return false;
	}
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
