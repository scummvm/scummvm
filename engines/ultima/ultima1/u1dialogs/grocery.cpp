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

#include "ultima/ultima1/u1dialogs/grocery.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/str.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(Grocery, BuySellDialog)
	ON_MESSAGE(TextInputMsg)
END_MESSAGE_MAP()

Grocery::Grocery(Ultima1Game *game, int groceryNum) : BuySellDialog(game, game->_res->GROCERY_NAMES[groceryNum - 1]) {
	Shared::Character &c = *game->_party;
	_costPerPack = 5 - c._intelligence / 20;
}

void Grocery::setMode(BuySell mode) {
	switch (mode) {
	case BUY:
		addInfoMsg(Common::String::format("%s%s", _game->_res->ACTION_NAMES[19], _game->_res->BUY), false, true);
		_mode = BUY;
		setDirty();

		getInput(true, 3);
		break;

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

void Grocery::draw() {
	BuySellDialog::draw();
	Shared::Gfx::VisualSurface s = getSurface();
	Ultima1Game *game = getGame();

	switch (_mode) {
	case BUY:
		centerText(Common::String::format(game->_res->GROCERY_PACKS1, _costPerPack), 4);
		centerText(game->_res->GROCERY_PACKS2, 5);
		centerText(game->_res->GROCERY_PACKS3, 6);
		break;

	case SELL:
		centerText(game->_res->GROCERY_SELL, String(_title).split('\n').size() + 2);
		break;

	default:
		break;
	}
}

bool Grocery::TextInputMsg(CTextInputMsg &msg) {
	assert(_mode == BUY);
	Shared::Character &c = *_game->_party;
	uint amount = atoi(msg._text.c_str());
	uint cost = amount * _costPerPack;

	if (msg._escaped || !amount) {
		nothing();
	} else if (cost > c._coins) {
		cantAfford();
	} else {
		addInfoMsg(msg._text);

		c._coins -= cost;
		c._food += amount * 10;
		addInfoMsg(Common::String::format(_game->_res->GROCERY_PACKS_FOOD, amount));
		_game->endOfTurn();
		hide();
	}

	return true;
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
