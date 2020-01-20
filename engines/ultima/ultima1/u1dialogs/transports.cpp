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

#include "ultima/ultima1/u1dialogs/transports.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/maps/map_overworld.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/core/str.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

EMPTY_MESSAGE_MAP(Transports, BuySellDialog);

Transports::Transports(Ultima1Game *game, int transportsNum) : BuySellDialog(game, game->_res->WEAPONRY_NAMES[transportsNum]),
		_transportsNum(transportsNum) {
}

void Transports::loadOverworldFreeTiles() {
	Maps::MapOverworld *map = static_cast<Maps::Ultima1Map *>(_game->_map)->getOverworldMap();
	Point delta;

	Maps::U1MapTile mapTile;
	_water = _woods = _grass = 0;

	// Iterate through the surrounding tiles relative to the player
	for (delta.y = -1; delta.y <= 1; ++delta.y) {
		for (delta.x = -1; delta.x <= 1; ++delta.x) {
			if (delta.x != 0 || delta.y != 0) {
				map->getTileAt(map->getPosition() + delta, &mapTile);

				if (mapTile.isOriginalWater())
					++_water;
				else if (mapTile.isOriginalGrass())
					++_grass;
				else if (mapTile.isOriginalWoods())
					++_woods;
			}
		}
	}
}

void Transports::setMode(BuySell mode) {
	Shared::Character &c = *_game->_party;
	_mode = mode;
	setDirty();

	switch (mode) {
	case BUY: {
		addInfoMsg(Common::String::format("%s%s", _game->_res->ACTION_NAMES[19], _game->_res->BUY), false, true);

		getKeypress();
		break;
	}

	case SELL:
		addInfoMsg(Common::String::format("%s%s%s", _game->_res->ACTION_NAMES[19], _game->_res->SELL, _game->_res->NOTHING), false, true);
		closeShortly();
		break;

	default:
		BuySellDialog::setMode(mode);
		break;
	}
}

void Transports::draw() {
	BuySellDialog::draw();

	switch (_mode) {
	case BUY:
		drawBuy();
		break;

	case SELL:
		drawSell();
		break;

	default:
		break;
	}
}

void Transports::drawBuy() {
/*
	Gfx::VisualSurface s = getSurface();
	const Shared::Character &c = *_game->_party;
	int titleLines = String(_title).split("\r\n").size();
	Common::String line;

	for (uint idx = _startIndex, yp = titleLines + 2; idx <= _endIndex; idx += 2, ++yp) {
		const Weapon &weapon = *static_cast<Weapon *>(c._weapons[idx]);

		line = Common::String::format("%c) %s", 'a' + idx, weapon._longName.c_str());
		s.writeString(line, TextPoint(5, yp));
		line = Common::String::format("-%4u", weapon.getBuyCost());
		s.writeString(line, TextPoint(22, yp));
	}
	*/
}

void Transports::drawSell() {
	int titleLines = String(_title).split("\r\n").size();
	centerText(String(_game->_res->TRANSPORTS_TEXT[0]).split("\r\n"), titleLines + 2);
}

bool Transports::CharacterInputMsg(CCharacterInputMsg &msg) {
	Shared::Character &c = *_game->_party;
	if (_mode == BUY) {
/*
		if (msg._keyState.keycode >= (int)(Common::KEYCODE_a + _startIndex) &&
			msg._keyState.keycode <= (int)(Common::KEYCODE_a + _endIndex) &&
			(int)(msg._keyState.keycode - Common::KEYCODE_a - _startIndex) % 2 == 0) {
			uint weaponNum = msg._keyState.keycode - Common::KEYCODE_a;
			Weapon &weapon = *static_cast<Weapon *>(c._weapons[weaponNum]);

			if (weapon.getBuyCost() <= c._coins) {
				// Display the sold weapon in the info area
				addInfoMsg(weapon._longName);

				// Remove coins for weapon and add it to the inventory
				c._coins -= weapon.getBuyCost();
				weapon.incrQuantity();

				// Show sold and close the dialog
				setMode(SOLD);
				return true;
			}
		}
		*/
		nothing();
		return true;
	} else {
		return BuySellDialog::CharacterInputMsg(msg);
	}
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
