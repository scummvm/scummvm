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

#include "indexed_frame.h"

namespace Scooby {

void IndexedFrame::present() {
	byte pal[64 *3];
	for (int i = 0; i < 64; i++) {
		pal[i*3] = expandLevel(_palette[i].redLevel);
		pal[i*3+1] = expandLevel(_palette[i].greenLevel);
		pal[i*3+2] = expandLevel(_palette[i].blueLevel);
	}
	_screen.setPalette(pal, 0, 64);

	_screen.copyRectToSurface(_pixels.data(), Width, 0, 0, Width, Height);
	_screen.makeAllDirty();
	_screen.update();
}

uint8 IndexedFrame::expandLevel(uint8 level) {
	return static_cast<uint8>((level << 5) | (level << 2) | (level >> 1));
}

} // namespace Scooby