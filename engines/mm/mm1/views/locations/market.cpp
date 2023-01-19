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

#include "mm/mm1/views/locations/market.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

#define TOTAL_FOOD 40
#define DISPLAY_TIMEOUT (3 * FRAME_RATE)

static const byte FOOD_COST[4] = { 5, 10, 20, 200 };

Market::Market() : Location("Market") {
	_bounds = getLineBounds(21, 24);
}

bool Market::msgFocus(const FocusMessage &msg) {
	send("View", ValueMessage(LOC_MARKET));

	Maps::Map &map = *g_maps->_currentMap;
	_foodCost = FOOD_COST[map[0] - 1];

	return true;
}

bool Market::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_ESCAPE:
	case Common::KEYCODE_n:
		leave();
		break;
	case Common::KEYCODE_y:
		if (!_displayCtr)
			buyFood();
		break;
	default:
		break;
	}

	return true;
}

void Market::draw() {
	clearSurface();

	writeString(0, 0, STRING["dialogs.market.special"]);
	writeNumber(_foodCost);
	writeString(STRING["dialogs.market.gp"]);
	writeString(10, 2, STRING["dialogs.market.will_you_pay"]);
}

bool Market::tick() {
	if (_displayCtr && --_displayCtr == 0)
		leave();
	return false;
}

void Market::buyFood() {
	int foodPurchased = 0;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		if (buyFood(&g_globals->_party[i]))
			++foodPurchased;
	}

	clearSurface();
	writeString(10, 2, foodPurchased ?
		STRING["dialogs.market.thankyou"] :
		STRING["dialogs.market.no_gold"]
	);

	_displayCtr = DISPLAY_TIMEOUT;
}

bool Market::buyFood(Character *c) {
	int tempGold = (int)c->_gold - _foodCost;
	if (tempGold < 0)
		return false;

	c->_gold = tempGold;
	c->_food = TOTAL_FOOD;

	return true;
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
