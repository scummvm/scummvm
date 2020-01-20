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
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

EMPTY_MESSAGE_MAP(Weaponry, BuySellDialog);

Weaponry::Weaponry(Ultima1Game *game, int weaponryNum) : BuySellDialog(game, game->_res->WEAPONRY_NAMES[weaponryNum]) {
	Shared::Character &c = *game->_party;
	_weaponsPresent.resize(c._weapons.size());

}

void Weaponry::setMode(BuySell mode) {
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
		_closeCounter = 1;
		setDirty();
		break;

	default:
		BuySellDialog::setMode(mode);
		break;
	}
}

void Weaponry::draw() {
	BuySellDialog::draw();
//	Ultima1Game *game = getGame();

	switch (_mode) {
	case BUY:
//		centerText(game->_res->Weaponry_PACKS3, 6);
		break;

	case SELL:
//		centerText(game->_res->Weaponry_SELL, 4);
		break;

	default:
		break;
	}
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
