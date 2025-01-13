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

#include "got/views/story.h"
#include "got/gfx/palette.h"
#include "got/metaengine.h"
#include "got/utils/file.h"
#include "got/vars.h"

namespace Got {
namespace Views {

#define MAX_Y 236

bool Story::msgFocus(const FocusMessage &msg) {
	res_read(Common::String::format("STORY%d", _G(area)), _G(tmp_buff));

	res_read("STORYPAL", _G(pbuff));

	for (int i = 0; i < PALETTE_SIZE; ++i)
		_G(pbuff[i]) = ((int)_G(pbuff[i]) * 255 + 31) / 63;
	Gfx::set_palette(_G(pbuff));

	// Create story image and load in it's fragments
	_surface.create(320, 240 * 2);

	for (int i = 0; i < 12; i++) {
		Gfx::Pics pics(Common::String::format("OPENP%d", i + 1));
		pics.load();

		_surface.simpleBlitFrom(pics[0], Common::Point(0, i * 40));
	}

	// Set up the story text
	int i = 0;
	int x = 8, y = 2;
	byte color = 72;
	char s[21];

	const char *p = (const char *)_G(tmp_buff);

	while (i < 46) {
		if (*p == '\n') {
			x = 8;
			y += 10;
			i++;

			if (i == 23) {
				// Move to start of of "second page" of the surface
				y = 240 + 2;
			}
		} else if (*p == '/' && *(p + 4) == '/') {
			p++;
			s[0] = *p++;
			s[1] = *p++;
			s[2] = *p++;
			s[3] = 0;
			color = atoi(s);
		} else if (*p != '\r') {
			_surface.rawPrintChar(*p, x - 1, y - 1, 255);
			_surface.rawPrintChar(*p, x + 1, y + 1, 255);
			_surface.rawPrintChar(*p, x - 1, y + 1, 255);
			_surface.rawPrintChar(*p, x + 1, y - 1, 255);
			_surface.rawPrintChar(*p, x, y - 1, 255);
			_surface.rawPrintChar(*p, x, y + 1, 255);
			_surface.rawPrintChar(*p, x - 1, y, 255);
			_surface.rawPrintChar(*p, x + 1, y, 255);
			_surface.rawPrintChar(*p, x, y, color);
			x += 8;
		}

		p++;
	}

	// Final two glyphs
	Gfx::Pics glyphs("STORYPIC", 262);

	if (_G(area) == 1) {
		_surface.simpleBlitFrom(glyphs[0], Common::Point(146, 64));
		_surface.simpleBlitFrom(glyphs[1], Common::Point(24, 88 + 240));
	} else {
		_surface.simpleBlitFrom(glyphs[0], Common::Point(146, 16));
	}

	// Play the opening music
	music_play("OPENSONG", 1);

	_yp = 0;
	_scrolling = false;

	return true;
}

bool Story::msgUnfocus(const UnfocusMessage &msg) {
	_surface.clear();
	music_pause();

	return true;
}

void Story::draw() {
	GfxSurface s = getSurface();

	// Draw the currently visible part of the surface
	s.simpleBlitFrom(_surface, Common::Rect(0, _yp, 320, _yp + 240),
					 Common::Point(0, 0));
}

bool Story::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE || _yp == MAX_Y)
		done();
	else if (!_scrolling)
		_scrolling = true;
	else
		_yp = MAX_Y;

	return true;
}

bool Story::msgKeypress(const KeypressMessage &msg) {
	if (_yp == MAX_Y)
		done();
	else if (!_scrolling)
		_scrolling = true;
	else
		_yp = 240;

	return true;
}

bool Story::tick() {
	if (_scrolling && _yp < MAX_Y) {
		_yp = MIN(_yp + 4, MAX_Y);
		redraw();
	}

	return true;
}

void Story::done() {
	music_stop();

	fadeOut();
	Gfx::load_palette();
	replaceView("PartTitle");
	fadeIn();
}

} // namespace Views
} // namespace Got
