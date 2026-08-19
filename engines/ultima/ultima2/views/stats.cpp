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

#include "ultima/ultima2/views/stats.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

void Stats::draw() {
	const auto &p = g_engine->_player;
	auto s = getSurface();
	s.clear();

	s.writeString("H.P.=");
	if (p._hp < 100) s.reverseColor();
	s.writeString(Common::String::format("%.4d", p._hp));
	if (p._hp < 100) s.reverseColor();

	s.writeString(Common::Point(0, 1), "FOOD=");
	if (p._food < 100) s.reverseColor();
	s.writeString(Common::String::format("%.4d", p._food));
	if (p._food < 100) s.reverseColor();

	s.writeString(Common::Point(0, 2), Common::String::format("EXP.=%.4d", p._experience));
	s.writeString(Common::Point(0, 3), Common::String::format("GOLD=%.4d", p._gold));
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
