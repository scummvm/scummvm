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

#include "mm/mm1/views_enh/locations/market.h"
#include "mm/mm1/globals.h"
#include "mm/utils/strings.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

Market::Market() : Location("Market") {
	addButton(&g_globals->_confirmIcons,
		Common::Point(_innerBounds.width() / 2 - 24,
			_innerBounds.height() - 32),
		0, Common::KEYCODE_y);
	addButton(&g_globals->_confirmIcons,
		Common::Point(_innerBounds.width() / 2 + 4,
			_innerBounds.height() - 32),
		2, Common::KEYCODE_n);
}

bool Market::msgFocus(const FocusMessage &msg) {
	Maps::Map &map = *g_maps->_currentMap;
	_foodCost = FOOD_COST[map[0] - 1];

	clear();
	addText(STRING["enhdialogs.location.store"], 0, ALIGN_MIDDLE);
	addText(STRING["enhdialogs.location.options"], 1, ALIGN_MIDDLE);
	addText(STRING["enhdialogs.market.buy_food"], 6, ALIGN_MIDDLE);
	addText(Common::String::format("%d %s",
		_foodCost, STRING["dialogs.market.gp"].c_str()),
		7, ALIGN_MIDDLE);
	addText(STRING["enhdialogs.location.gold"], 10, ALIGN_LEFT);

	uint gold = g_globals->_party.getPartyGold();
	addText(Common::String::format("%d", gold), 10, ALIGN_RIGHT);
	return true;
}

bool Market::msgKeypress(const KeypressMessage &msg) {
	if (isDelayActive()) {
		// Any keypress after purchase made closes
		leave();
		return true;
	} else {
		switch (msg.keycode) {
		case Common::KEYCODE_y:
			buyFood();
			return true;
		case Common::KEYCODE_n:
		case Common::KEYCODE_ESCAPE:
			leave();
			return true;
		default:
			break;
		}
	}

	return false;
}

void Market::buyFood() {
	int numPurchases = 0;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		if (buyFood(&g_globals->_party[i]))
			++numPurchases;
	}

	clearSurface();
	writeString(10, 2, numPurchases ?
		STRING["dialogs.market.thankyou"] :
		STRING["dialogs.market.no_gold"]
	);

	delaySeconds(3);
}

bool Market::buyFood(Character *c) {
	if (c->_food == MAX_FOOD)
		return true;

	int tempGold = (int)c->_gold - _foodCost;
	if (tempGold >= 0) {
		// Reduce character's gold
		c->_gold = tempGold;
	} else {
		// Fall back on any one in the party with gold
		uint i;
		for (i = 0; i < g_globals->_party.size(); ++i) {
			if (g_globals->_party[i]._gold >= _foodCost) {
				g_globals->_party[i]._gold -= _foodCost;
				break;
			}
		}
		if (i == g_globals->_party.size())
			return false;
	}

	// Food purchased
	c->_food = MAX_FOOD;
	return true;
}

} // namespace Location
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
