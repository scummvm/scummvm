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

#include "ultima/ultima2/views/interactions/food_shop.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

constexpr int FOOD_PRICE_INDEX = 3;
constexpr int MAX_FOOD = 9999;

FoodShop::FoodShop() : Interaction("FoodShop") {
}

bool FoodShop::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	_price = _G(savegame).computeItemPrice(FOOD_PRICE_INDEX);
	writeString("THE FOOD HERE COSTS %.4d\nPER 100, WANT ONE? ", _price);

	return Interaction::msgFocus(msg);
}

bool FoodShop::msgKeypress(const KeypressMessage &msg) {
	close();
	if (toupper(msg.ascii) != 'Y') {
		writeString("NO\n");
	} else {
		writeString("YES,\nNO ONIONS AND TO GO PLEASE!\n");
		if (_G(logic)->trySpendGold(_price)) {
			Data::Savegame &sg = _G(savegame);
			sg._food = MIN(sg._food + 100, MAX_FOOD);
			writeString("THANK YOU, COME AGAIN!\n");
		}
	}

	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
