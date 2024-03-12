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

#include "common/system.h"
#include "graphics/palette.h"
#include "xyzzy/view1.h"

namespace Xyzzy {

bool View1::msgFocus(const FocusMessage &msg) {
	Common::fill(&_pal[0], &_pal[256 * 3], 0);
	_offset = 128;
	return true;
}

bool View1::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	close();
	return true;
}

void View1::draw() {
	// Draw a bunch of squares on screen
	Graphics::ManagedSurface s = getSurface();

	for (int i = 0; i < 100; ++i)
		s.frameRect(Common::Rect(i, i, 320 - i, 200 - i), i);
}

bool View1::tick() {
	// Cycle the palette
	++_offset;
	for (int i = 0; i < 256; ++i)
		_pal[i * 3 + 1] = (i + _offset) % 256;
	g_system->getPaletteManager()->setPalette(_pal, 0, 256);

	// Below is redundant since we're only cycling the palette, but it demonstrates
	// how to trigger the view to do further draws after the first time, since views
	// don't automatically keep redrawing unless you tell it to
	if ((_offset % 256) == 0)
		redraw();

	return true;
}

} // namespace Xyzzy
