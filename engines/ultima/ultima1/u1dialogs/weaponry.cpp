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

#include "ultima/ultima1/u1dialogs/weaponry.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/core/str.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

EMPTY_MESSAGE_MAP(Weaponry, BuySellDialog);

Weaponry::Weaponry(Ultima1Game *game, int weaponryNum) : BuySellDialog(game, game->_res->WEAPONRY_NAMES[weaponryNum]) {
	//, _weaponryNum(weaponryNum) {
	Maps::Ultima1Map *map = static_cast<Maps::Ultima1Map *>(game->_map);

	int offset = (weaponryNum + 1) % 2 + 1;
	int index = (map->_moveCounter % 0x7fff) / 1500;
	if (index > 3 || map->_moveCounter > 3000)
		index = 3;

	_startIndex = offset;
	_endIndex = offset + (index + 1) * 2;
}

void Weaponry::setMode(BuySell mode) {
	Shared::Character &c = *_game->_party;

	switch (mode) {
	case BUY: {
		addInfoMsg(Common::String::format("%s%s", _game->_res->ACTION_NAMES[19], _game->_res->BUY), false, true);

		_mode = BUY;
		setDirty();
		getKeypress();
		break;
	}

	case SELL:
		addInfoMsg(Common::String::format("%s%s", _game->_res->ACTION_NAMES[19], _game->_res->SELL), false, true);

		if (c._weapons.hasNothing()) {
			addInfoMsg(_game->_res->NOTHING);
			closeShortly();
		} else {
			getKeypress();
		}

		_mode = SELL;
		setDirty();
		break;

	default:
		BuySellDialog::setMode(mode);
		break;
	}
}

void Weaponry::draw() {
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

void Weaponry::drawBuy() {
	Shared::Gfx::VisualSurface s = getSurface();
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
}

void Weaponry::drawSell() {
	Shared::Gfx::VisualSurface s = getSurface();
	const Shared::Character &c = *_game->_party;
	int lineCount = c._weapons.itemsCount();
	int titleLines = String(_title).split("\r\n").size();
	Common::String line;

	if (lineCount == 0) {
		centerText(_game->_res->NO_WEAPONRY_TO_SELL, titleLines + 2);
	} else {
		for (uint idx = 1; idx < c._weapons.size(); ++idx) {
			const Weapon &weapon = *static_cast<Weapon *>(c._weapons[idx]);
			if (!weapon.empty()) {
				line = Common::String::format("%c) %s", 'a' + idx, weapon._longName.c_str());
				s.writeString(line, TextPoint(5, idx + titleLines + 1));
				line = Common::String::format("-%4u", weapon.getSellCost());
				s.writeString(line, TextPoint(22, idx + titleLines + 1));
			}
		}
	}
}

bool Weaponry::CharacterInputMsg(CCharacterInputMsg &msg) {
	Shared::Character &c = *_game->_party;

	if (_mode == BUY) {
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

		nothing();
		return true;
	} else if (_mode == SELL && !c._weapons.hasNothing()) {
		if (msg._keyState.keycode >= Common::KEYCODE_b &&
			msg._keyState.keycode < (Common::KEYCODE_a + (int)c._weapons.size())) {
			uint weaponNum = msg._keyState.keycode - Common::KEYCODE_a;
			Weapon &weapon = *static_cast<Weapon *>(c._weapons[weaponNum]);

			if (!weapon.empty()) {
				// Display the sold weapon in the info area
				addInfoMsg(weapon._longName);

				// Give coins for weapon and remove it from the inventory
				c._coins += weapon.getSellCost();
				if (weapon.decrQuantity() && (int)weaponNum == c._equippedWeapon)
					c.removeWeapon();

				// Close the dialog
				setMode(DONE);
				return true;
			}
		}

		nothing();
		return true;
	} else {
		return BuySellDialog::CharacterInputMsg(msg);
	}
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
