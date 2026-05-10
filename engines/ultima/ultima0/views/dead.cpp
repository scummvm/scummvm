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

#include "ultima/ultima0/views/dead.h"
#include "ultima/ultima0/gfx/font.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

Dead::Dead() : View("Dead") {
	setBounds(Common::Rect(0, DEFAULT_SCY - 4 * Gfx::GLYPH_HEIGHT, DEFAULT_SCX, DEFAULT_SCY));
}

void Dead::draw() {
	const auto &player = g_engine->_player;
	auto s = getSurface();
	s.clear();

	const char *name = player._name;
	if (*name == '\0')
		name = "the peasant";

	s.writeString(Common::Point(20, 0), "We mourn the passing of", Graphics::kTextAlignCenter);
	s.writeString(Common::Point(20, 1), Common::String::format("%s and his Computer", name), Graphics::kTextAlignCenter);
	s.writeString(Common::Point(20, 2), "To invoke a miracle of resurrection", Graphics::kTextAlignCenter);
	s.writeString(Common::Point(20, 3), "Press a Key", Graphics::kTextAlignCenter);
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
