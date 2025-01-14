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

#include "got/views/credits.h"
#include "got/vars.h"

namespace Got {
namespace Views {

#define CREDITS_COUNT 9
#define FADE_FRAMES 15
#define DISPLAY_TIME 15
#define CREDIT_TIME (FADE_FRAMES * 2 + DISPLAY_TIME)

bool Credits::msgFocus(const FocusMessage &msg) {
	_delayCtr = 0;
	_frameCtr = 0;

	draw();
	Gfx::Palette63 pal = _G(gfx[41]);
	fadeIn(pal);

	return true;
}

void Credits::draw() {
	GfxSurface s = getSurface();

	// Draw scroll background
	const Graphics::ManagedSurface &bg = _G(gfx[42]);
	s.clear(*(const byte *)bg.getPixels());
	s.simpleBlitFrom(bg, Common::Point(0, 24));

	int creditNum = _frameCtr / CREDIT_TIME;
	int subNum = _frameCtr % CREDIT_TIME;

	if (subNum >= (FADE_FRAMES + DISPLAY_TIME)) {
		subNum = (FADE_FRAMES - 1) - (subNum - (FADE_FRAMES + DISPLAY_TIME));
	} else if (subNum >= FADE_FRAMES) {
		subNum = FADE_FRAMES - 1;
	}

	if (creditNum < CREDITS_COUNT) {
		int gfxNum1 = 43 + creditNum;
		int gfxNum2 = 67 + creditNum;
		int gfxNum3 = 52 + subNum;
		int gfxNum4 = 76 + subNum;

		drawCredit(s, gfxNum1, gfxNum3, 16, 40 + 24);
		drawCredit(s, gfxNum2, gfxNum4, 16, 40 + 24);
	}

	s.markAllDirty();
}

void Credits::drawCredit(GfxSurface &s, int gfxNum1, int gfxNum2, int x, int y) {
	const Gfx::GraphicChunk &data = _G(gfx[gfxNum1]);
	const Gfx::GraphicChunk &lookup = _G(gfx[gfxNum2]);
	const byte *lines = data._data;
	const byte *lineData = data._data + 2 * data._height;

	assert(x >= 0 && (x + data._width) <= 320);
	assert(y >= 0 && (y + data._height) <= 200);

	for (int yCtr = 0; yCtr < data._height; ++yCtr) {
		byte *dest = (byte *)s.getBasePtr(x, y + yCtr);
		uint16 lineParts = READ_LE_UINT16(lines);
		lines += 2;

		if (lineParts == 0)
			// Nothing on line, move to next
			continue;

		for (; lineParts > 0; --lineParts) {
			byte count = *lineData++;

			if (count & 0x80) {
				// Shade a range of pixels using lookup table
				count &= 0x7f;
				for (int i = 0; i < count; ++i, ++dest)
					*dest = lookup._data[*dest];
			} else {
				dest += count;
			}
		}
	}
}

bool Credits::tick() {
	if (++_delayCtr >= 3) {
		_delayCtr = 0;

		if (_frameCtr == (CREDIT_TIME * CREDITS_COUNT) + 10) {
			replaceView("HighScores", true, true);
		} else {
			++_frameCtr;
			redraw();
		}
	}

	return true;
}

bool Credits::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		fadeOut();
		send("TitleBackground", GameMessage("MAIN_MENU"));
		return true;
	}

	return false;
}

} // namespace Views
} // namespace Got
