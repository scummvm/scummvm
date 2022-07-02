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

void Market::draw() {
	Location::draw();

}

} // namespace Location
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
