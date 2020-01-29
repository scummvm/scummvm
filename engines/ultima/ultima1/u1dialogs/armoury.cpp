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

#include "ultima/ultima1/u1dialogs/armoury.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/core/str.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

EMPTY_MESSAGE_MAP(Armoury, BuySellDialog);

Armoury::Armoury(Ultima1Game *game, int armouryNum) : BuySellDialog(game, game->_res->ARMOURY_NAMES[armouryNum]) {
	Maps::Ultima1Map *map = static_cast<Maps::Ultima1Map *>(game->_map);

	_startIndex = 1;
	_endIndex = (map->_moveCounter > 3000) ? 5 : 3;
}

void Armoury::setMode(BuySell mode) {
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

		if (c._armour.hasNothing()) {
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

void Armoury::draw() {
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

void Armoury::drawBuy() {
	Shared::Gfx::VisualSurface s = getSurface();
	const Shared::Character &c = *_game->_party;
	int titleLines = String(_title).split("\r\n").size();
	Common::String line;

	for (uint idx = _startIndex, yp = titleLines + 2; idx <= _endIndex; ++idx, ++yp) {
		const Armour &armour = *static_cast<Armour *>(c._armour[idx]);

		line = Common::String::format("%c) %s", 'a' + idx, armour._name.c_str());
		s.writeString(line, TextPoint(5, yp));
		line = Common::String::format("-%4u", armour.getBuyCost());
		s.writeString(line, TextPoint(22, yp));
	}
}

void Armoury::drawSell() {
	Shared::Gfx::VisualSurface s = getSurface();
	const Shared::Character &c = *_game->_party;
	int lineCount = c._armour.itemsCount();
	int titleLines = String(_title).split("\r\n").size();
	Common::String line;

	if (lineCount == 0) {
		centerText(_game->_res->NO_ARMOUR_TO_SELL, titleLines + 2);
	} else {
		for (uint idx = 1; idx < c._armour.size(); ++idx) {
			const Armour &armour = *static_cast<Armour *>(c._armour[idx]);
			if (!armour.empty()) {
				line = Common::String::format("%c) %s", 'a' + idx, armour._name.c_str());
				s.writeString(line, TextPoint(5, idx + titleLines + 1));
				line = Common::String::format("-%4u", armour.getSellCost());
				s.writeString(line, TextPoint(22, idx + titleLines + 1));
			}
		}
	}
}

bool Armoury::CharacterInputMsg(CCharacterInputMsg &msg) {
	Shared::Character &c = *_game->_party;

	if (_mode == BUY) {
		if (msg._keyState.keycode >= (int)(Common::KEYCODE_a + _startIndex) &&
			msg._keyState.keycode <= (int)(Common::KEYCODE_a + _endIndex)) {
			uint armourNum = msg._keyState.keycode - Common::KEYCODE_a;
			Armour &armour = *static_cast<Armour *>(c._armour[armourNum]);

			if (armour.getBuyCost() <= c._coins) {
				// Display the sold armour in the info area
				addInfoMsg(armour._name);

				// Remove coins for armour and add it to the inventory
				c._coins -= armour.getBuyCost();
				armour.incrQuantity();

				// Show sold and close the dialog
				setMode(SOLD);
				return true;
			}
		}

		nothing();
		return true;
	} else if (_mode == SELL && !c._armour.hasNothing()) {
		if (msg._keyState.keycode >= Common::KEYCODE_b &&
			msg._keyState.keycode < (Common::KEYCODE_a + (int)c._armour.size())) {
			uint armourNum = msg._keyState.keycode - Common::KEYCODE_a;
			Armour &armour = *static_cast<Armour *>(c._armour[armourNum]);

			if (!armour.empty()) {
				// Display the sold armour in the info area
				addInfoMsg(armour._name);

				// Give coins for armour and remove it from the inventory
				c._coins += armour.getSellCost();
				if (armour.decrQuantity() && (int)armourNum == c._equippedArmour)
					c.removeArmour();

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
