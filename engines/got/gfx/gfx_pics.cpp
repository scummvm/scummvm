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

#include "common/textconsole.h"
#include "common/file.h"
#include "got/gfx/gfx_pics.h"

namespace Got {
namespace Gfx {

void convertPaneDataToSurface(const byte *src, Graphics::ManagedSurface &surf) {
	// It's split into 4 panes per 4 pixels, so we need
	// to juggle the pixels into their correct order
	for (int plane = 0; plane < 4; ++plane) {
		for (int y = 0; y < surf.h; ++y) {
			byte *dest = (byte *)surf.getBasePtr(plane, y);

			for (int x = 0; x < (surf.w / 4); ++x, dest += 4)
				*dest = *src++;
		}
	}
}

void BgPics::setArea(int area) {
	if (area != _area) {
		_area = area;
		load();
	}
}

void BgPics::load() {
	Common::String fname = Common::String::format("BPICS%d", _area);
	Common::File f;
	if (!f.open(Common::Path(fname)))
		error("Could not open - %s", fname.c_str());

	// Process the sprites - they're each 262 bytes
	clear();
	resize(f.size() / 262);

	byte buff[16 * 16];
	for (uint idx = 0; idx < size(); ++idx) {
		Graphics::ManagedSurface &s = (*this)[idx];

		s.create(16, 16);
		s.setTransparentColor(15);

		f.skip(6);
		f.read(buff, 16 * 16);
		convertPaneDataToSurface(buff, s);
	}
}

void Pics::load() {

}

} // namespace Gfx
} // namespace Got
