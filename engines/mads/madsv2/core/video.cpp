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

#include "mads/madsv2/core/video.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

void video_init(int mode, int set_mode) {
	// No implementation in ScummVM
}

void video_update() {
	auto &screen = *g_engine->getScreen();
	screen.update();
}

void video_update(Buffer *from, int from_x, int from_y,
		int unto_x, int unto_y,
		int size_x, int size_y) {
	auto &screen = *g_engine->getScreen();
	Graphics::Surface subArea = screen.getSubArea(Common::Rect(unto_x, unto_y,
		unto_x + size_x, unto_y + size_y));

	for (int yp = 0; yp < size_y; ++yp) {
		const byte *src = from->data + (from_y + yp) * from->x + from_x;
		byte *dest = (byte *)subArea.getBasePtr(0, yp);
		Common::copy(src, src + size_x, dest);
	}
}

} // namespace MADSV2
} // namespace MADS
