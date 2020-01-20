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

#include "ultima/ultima1/u1dialogs/magic.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/core/str.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

EMPTY_MESSAGE_MAP(Magic, BuySellDialog);

Magic::Magic(Ultima1Game *game, int magicNum) : BuySellDialog(game, game->_res->MAGIC_NAMES[magicNum]),
		_magicNum(magicNum) {
	Maps::Ultima1Map *map = static_cast<Maps::Ultima1Map *>(game->_map);

	int offset = (magicNum + 1) % 2 + 1;
	int index = (map->_moveCounter % 0x7fff) / 1500;
	if (index > 3 || map->_moveCounter > 3000)
		index = 3;

	_startIndex = offset;
	_endIndex = offset + (index + 1) * 2;
}

void Magic::setMode(BuySell mode) {
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

		_mode = SELL;
		closeShortly();
		setDirty();
		break;

	default:
		BuySellDialog::setMode(mode);
		break;
	}
}

void Magic::draw() {
	BuySellDialog::draw();
	Shared::Gfx::VisualSurface s = getSurface();
	Ultima1Game *game = getGame();

	switch (_mode) {
	case BUY:
		drawBuy();
		break;

	case SELL:
		centerText(game->_res->DONT_BUY_SPELLS, String(_title).split('\n').size() + 2);
		break;

	default:
		break;
	}
}

void Magic::drawBuy() {
	Shared::Gfx::VisualSurface s = getSurface();
	const Shared::Character &c = *_game->_party;
	int titleLines = String(_title).split("\r\n").size();
	Common::String line;

	for (uint idx = _startIndex, yp = titleLines + 2; idx <= _endIndex; ++idx, ++yp) {
		const Spells::Spell &magic = *static_cast<Spells::Spell *>(c._spells[idx]);

		line = Common::String::format("%c) %s", 'a' + idx, magic._name.c_str());
		s.writeString(line, TextPoint(5, yp));
		line = Common::String::format("-%4u", magic.getBuyCost());
		s.writeString(line, TextPoint(22, yp));
	}
}

bool Magic::CharacterInputMsg(CCharacterInputMsg &msg) {
	Shared::Character &c = *_game->_party;

	if (_mode == BUY) {
		if (msg._keyState.keycode >= (int)(Common::KEYCODE_a + _startIndex) &&
			msg._keyState.keycode <= (int)(Common::KEYCODE_a + _endIndex)) {
			uint magicNum = msg._keyState.keycode - Common::KEYCODE_a;
			Spells::Spell &magic = *static_cast<Spells::Spell *>(c._spells[magicNum]);

			if (magic.getBuyCost() <= c._coins) {
				// Display the sold magic in the info area
				addInfoMsg(magic._name);

				// Remove coins for magic and add it to the inventory
				c._coins -= magic.getBuyCost();
				magic.incrQuantity();

				// Show sold and close the dialog
				setMode(SOLD);
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
