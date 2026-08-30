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

#include "ultima/ultima1/views/game.h"
#include "ultima/ultima1/views/overworld_map.h"
#include "ultima/ultima1/views/location_map.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/tiles.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

// Colors used
constexpr int COLOR_BORDER = 1;  // blue - thick outer band
constexpr int COLOR_EDGE = 15;   // white - thin inner highlight line

Game::Game() : Shared::Gfx::View("Game"), _commands(this), _stats(this) {
	setBounds(TextRect(0, 0, 39, 24));
	_commands.setBounds(TextRect(0, 20, 29, 24));
	_stats.setBounds(TextRect(31, 21, 39, 24));
}

void Game::draw() {
	auto s = getSurface();
	Shared::UIElement::draw();

	// Vertical divider between the command/message log (left) and the stats panel (right)
	s.fillRect(Common::Rect(241, 160, 247, 200), COLOR_BORDER);
	s.drawLine(240, 160, 240, 199, COLOR_EDGE);
	s.drawLine(247, 160, 247, 199, COLOR_EDGE);
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
