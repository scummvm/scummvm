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
#include "ultima/ultima2/gfx/text.h"
#include "ultima/ultima2/ultima2.h"
#include "graphics/views/rect.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

Stats::Stats(UIElement *parent) : Graphics::Views::UIElement("Stats", parent) {
	setBounds(Graphics::Views::TextRect(30, 20, 39, 23));
}

void Stats::draw() {
	Data::Savegame &sg = _G(savegame);
	auto s = getSurface();
	s.clear();

	// A value that's run out is highlighted
	const byte normal = Gfx::textColor(Gfx::TEXT_NORMAL);
	const byte highlight = Gfx::textColor(Gfx::TEXT_HIGHLIGHT);

	s.setColor(normal);
	s.writeString(Common::Point(0, 0), "H.P.=");
	s.setColor(sg._hp == 0 ? highlight : normal);
	s.writeString(Common::Point(5, 0), "%4d", sg._hp);

	s.setColor(normal);
	s.writeString(Common::Point(0, 1), "FOOD=");
	s.setColor(sg._food == 0 ? highlight : normal);
	s.writeString(Common::Point(5, 1), "%4d", sg._food);

	s.setColor(normal);
	s.writeString(Common::Point(0, 2), "EXP.=");
	s.writeString(Common::Point(5, 2), "%4d", sg._experience);

	s.writeString(Common::Point(0, 3), "GOLD=");
	s.writeString(Common::Point(5, 3), "%4d", sg._gold);
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
