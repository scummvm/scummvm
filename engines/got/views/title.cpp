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

#include "got/views/title.h"
#include "got/vars.h"

namespace Got {
namespace Views {

void Title::draw() {
	GfxSurface s = getSurface();

	for (int col = 0, xp = 0; col < 10; ++col, xp += 32) {
		for (int yp = 0; yp < 240; yp += 32)
			s.blitFrom(_G(gfx)[26], Common::Point(xp, yp));
	}
}

bool Title::tick() {
	if (isFocused()) {
		// When the title screen is shown, show the main menu
		draw();
		addView("MainMenu");
	}

	return true;
}

} // namespace Views
} // namespace Got
