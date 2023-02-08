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

#include "mm/mm1/views/title.h"
#include "mm/mm1/gfx/gfx.h"
#include "mm/mm1/gfx/screen_decoder.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace Views {

#define FADE_SEGMENTS 20
#define FADE_SEGMENT_X (SCREEN_W / 2 / FADE_SEGMENTS)
#define FADE_SEGMENT_Y (SCREEN_H / 2 / FADE_SEGMENTS)

Title::Title() : UIElement("Title", g_engine) {
}

bool Title::msgFocus(const FocusMessage &msg) {
	Gfx::ScreenDecoder decoder;
	decoder._indexes[0] = 0;
	decoder._indexes[1] = 2;
	decoder._indexes[2] = 4;
	decoder._indexes[3] = 15;

	for (int i = 0; i < SCREENS_COUNT; ++i) {
		if (i == 2) {
			decoder._indexes[1] = 3;
			decoder._indexes[2] = 5;
		}

		if (decoder.loadFile(
			Common::String::format("screen%d", i))) {
			_screens[i].copyFrom(decoder.getSurface());
		} else {
			error("Could not load title screen");
		}
	}

	_screenNum = -1;
	_fadeIndex = 0;

	return true;
}

bool Title::msgUnfocus(const UnfocusMessage & msg) {
	for (int i = 0; i < SCREENS_COUNT; ++i)
		_screens[i].clear();

	return true;
}

void Title::draw() {
	Graphics::ManagedSurface surf = getSurface();

	if (_screenNum == -1) {
		// Initially, display the entire first screen
		surf.blitFrom(_screens[0]);

		// Start up fading in the second one
		_screenNum = 1;
		_fadeIndex = 0;
		delaySeconds(1);

	} else if (_screenNum < 2 && _fadeIndex == 0) {
		// Brief pause before starting next screen scroll in
		delaySeconds(1);

	} else if (_screenNum < 2) {
		// Gradually displaying more of the next screen
		int deltaX = _fadeIndex * FADE_SEGMENT_X;
		int deltaY = _fadeIndex * FADE_SEGMENT_Y;

		const Graphics::ManagedSurface &src = _screens[_screenNum];
		const Common::Rect top(0, 0, SCREEN_W, deltaY);
		const Common::Rect left(0, 0, deltaX, SCREEN_H);
		const Common::Rect right(SCREEN_W - deltaX, 0, SCREEN_W, SCREEN_H);
		const Common::Rect bottom(0, SCREEN_H - deltaY, SCREEN_W, SCREEN_H);

		surf.blitFrom(src, top, top);
		surf.blitFrom(src, left, left);
		surf.blitFrom(src, right, right);
		surf.blitFrom(src, bottom, bottom);

		delayFrames(2);

	} else {
		// Scene screens
		const Graphics::ManagedSurface &src = _screens[_screenNum];
		surf.blitFrom(src);

		delaySeconds(5);
	}
}

void Title::timeout() {
	if (_screenNum < 2) {
		if (_fadeIndex++ == FADE_SEGMENTS) {
			_screenNum = (_screenNum == 0) ? 1 : 0;
			_fadeIndex = 0;
		}
	} else {
		if (++_screenNum >= SCREENS_COUNT) {
			// Go back to alternating first two screens
			_screenNum = -1;
			_fadeIndex = 0;
		}
	}

	redraw();
}

bool Title::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_SPACE) {
		// Start showing game screens slideshow
		cancelDelay();
		_screenNum = 2;
		_fadeIndex = 0;
		redraw();
	}

	return true;
}

bool Title::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		g_events->replaceView("AreYouReady");
		return true;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
