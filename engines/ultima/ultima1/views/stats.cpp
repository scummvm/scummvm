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

#include "ultima/ultima1/views/stats.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

// Colors used
constexpr int COLOR_TEXT = 11;      // light cyan - normal values
constexpr int COLOR_HIGHLIGHT = 12; // light red - low hits/food warning

void Stats::draw() {
	Data::Player &p = g_engine->_player;
	auto s = getSurface();
	s.clear();

	// Hits and food are clamped and shown in a highlight color once low
	if (p._hits < 0)
		p._hits = 0;
	else if (p._hits > 9999)
		p._hits = 9999;
	if (p._food < 0)
		p._food = 0;
	else if (p._food > 9999)
		p._food = 9999;
	if (p._experience > 9999)
		p._experience = 9999;
	if (p._coins > 9999)
		p._coins = 9999;

	s.setColor(COLOR_TEXT);
	s.writeString(Common::Point(0, 0), "Hits:");
	s.setColor(p._hits < 100 ? COLOR_HIGHLIGHT : COLOR_TEXT);
	s.writeString(Common::Point(5, 0), Common::String::format("%4d", p._hits));

	s.setColor(COLOR_TEXT);
	s.writeString(Common::Point(0, 1), "Food:");
	s.setColor(p._food < 100 ? COLOR_HIGHLIGHT : COLOR_TEXT);
	s.writeString(Common::Point(5, 1), Common::String::format("%4d", p._food));

	s.setColor(COLOR_TEXT);
	s.writeString(Common::Point(0, 2), "Exp.:");
	s.writeString(Common::Point(5, 2), Common::String::format("%4d", p._experience));

	s.writeString(Common::Point(0, 3), "Coin:");
	s.writeString(Common::Point(5, 3), Common::String::format("%4d", p._coins));
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
