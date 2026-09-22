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

#include "ultima/ultima2/views/space_stats.h"
#include "ultima/ultima2/logic/space_logic.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

constexpr int COLOR_TEXT = 3; // white, within the game's 4-color CGA palette

SpaceStats::SpaceStats(UIElement *parent) : Shared::UIElement("SpaceStats", parent) {
	setBounds(TextRect(30, 20, 39, 23));
}

void SpaceStats::draw() {
	auto s = getSurface();
	s.clear();

	auto *logic = dynamic_cast<Logic::SpaceLogic *>(_G(logic).get());
	if (!logic)
		return;

	s.setColor(COLOR_TEXT);
	s.writeString(Common::Point(0, 0), "  FUEL=%.2d", _G(savegame)._items[Data::ITEM_TRI_LITHIUM] % 100);
	s.writeString(Common::Point(0, 1), "  XENO=%.2d", logic->coords()[0]);
	s.writeString(Common::Point(0, 2), "  YAKO=%.2d", logic->coords()[1]);
	s.writeString(Common::Point(0, 3), "  ZABO=%.2d", logic->coords()[2]);
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
