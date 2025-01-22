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
#include "got/got.h"
#include "got/vars.h"

namespace Got {
namespace Views {

int ctr = 0;

void Opening::draw() {
	GfxSurface s = getSurface();

	if (_shakeX == 0) {
		s.blitFrom(_surface, Common::Rect(0, 0, 320, 400), Common::Rect(0, 0, 320, 240));
	} else {
		s.clear();
		Common::Rect destRect(0, 0, 320, 240);
		destRect.translate(_shakeX, 0);
		s.blitFrom(_surface, Common::Rect(0, 0, 320, 400), destRect);
	}
}

bool Opening::msgFocus(const FocusMessage &msg) {
	_surface.create(320, 400);
	for (int chunkNum = 0; chunkNum < 4; ++chunkNum) {
		const byte *src = _G(gfx[36 + chunkNum])._data;
		byte *dest = (byte *)_surface.getBasePtr(chunkNum, 0);

		for (int i = 0; i < (320 * 400 / 4); ++i, ++src, dest += 4)
			*dest = *src;
	}

	// Fade in the screen
	const Gfx::Palette63 pal = _G(gfx[35]);
	draw();
	fadeIn(pal);

	return true;
}

void Opening::drawTitle() {
	const byte *src = _G(gfx[40])._data;

	for (int pane = 0; pane < 4; ++pane) {
		byte *dest = (byte *)_surface.getBasePtr(pane, 0);

		for (int i = 0; i < (320 * 80 / 4); ++i, ++src, dest += 4)
			*dest = *src;
	}
}

bool Opening::msgUnfocus(const UnfocusMessage &msg) {
	_surface.clear();
	return true;
}

bool Opening::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		fadeOut();
		send("TitleBackground", GameMessage("MAIN_MENU"));
		return true;
	}

	return false;
}

bool Opening::tick() {
	++_frameCtr;

	if (_frameCtr == 20) {
		drawTitle();
		redraw();
		playSound(_G(gfx[104]));
	} else if (_frameCtr < 40) {
		if ((_frameCtr % 4) == 0) {
			_shakeX = g_engine->getRandomNumber(19) - 10;
			redraw();
		}
	} else if (_frameCtr == 41) {
		_shakeX = 0;
		redraw();
	} else if (_frameCtr == 150) {
		fadeOut();
		replaceView("Credits", true);
	}

	return true;
}

} // namespace Views
} // namespace Got
