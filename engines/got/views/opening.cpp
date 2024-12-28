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

#include "got/views/opening.h"
#include "got/vars.h"

namespace Got {
namespace Views {

int ctr = 0;

void Opening::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_surface, Common::Rect(0, 0, 320, 400), Common::Rect(0, 0, 320, 240));
}

bool Opening::msgFocus(const FocusMessage &msg) {
	const byte *src;
	byte *dest;
	int i;

	Gfx::Palette63 pal = _G(gfx)[35];
	Gfx::xsetpal(pal);

	_surface.create(320, 400);
	for (int chunkNum = 0; chunkNum < 4; ++chunkNum) {
		src = _G(gfx)[36 + chunkNum]._data;
		dest = (byte *)_surface.getBasePtr(chunkNum, 0);

		for (i = 0; i < (320 * 400 / 4); ++i, ++src, dest += 4)
			*dest = *src;
	}

	return true;
}

bool Opening::msgUnfocus(const UnfocusMessage &msg) {
	_surface.clear();
	return true;
}

bool Opening::tick() {
	redraw();
	return true;
}

} // namespace Views
} // namespace Got
