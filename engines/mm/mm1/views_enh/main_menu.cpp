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

#include "mm/mm1/views_enh/main_menu.h"
#include "mm/mm1/gfx/screen_decoder.h"
#include "mm/mm1/globals.h"

#define CIRCLE_X 110
#define CIRCLE_Y 30
#define CIRCLE_RADIUS 50

namespace MM {
namespace MM1 {
namespace ViewsEnh {

bool MainMenu::msgFocus(const FocusMessage &msg) {
	ScrollView::msgFocus(msg);
	loadCircles();
	return true;
}

bool MainMenu::msgUnfocus(const UnfocusMessage &msg) {
	ScrollView::msgUnfocus(msg);
	_circles[0].clear();
	_circles[1].clear();
	return true;
}

void MainMenu::loadCircles() {
	const Common::Rect r(0, 0, (CIRCLE_RADIUS + 1) * 2, (CIRCLE_RADIUS + 1) * 2);
	Gfx::ScreenDecoder decoder;
	decoder._indexes[0] = 0;
	decoder._indexes[1] = 2;
	decoder._indexes[2] = 4;
	decoder._indexes[3] = 15;

	for (int i = 0; i < 2; ++i) {
		if (decoder.loadFile(Common::String::format("screen%d", i))) {
			const Graphics::Surface *src = decoder.getSurface();
			Graphics::ManagedSurface *dest = &_circles[i];

			dest->create((CIRCLE_RADIUS + 1) * 2, (CIRCLE_RADIUS + 1) * 2);
			dest->fillRect(r, 255);
			dest->setTransparentColor(255);
			copyCircle(src, dest);
		}
	}
}

void MainMenu::copyCircle(const Graphics::Surface *src,
		Graphics::ManagedSurface *dest) {
	int radius = CIRCLE_RADIUS;
	int x = 0;
	int y = radius;
	int p = 1 - radius;

	/* Plot first set of points */
	drawCircleLine(src, dest, x, y);

	while (x < y) {
		x++;
		if (p < 0)
			p += 2 * x + 1;
		else {
			y--;
			p += 2 * (x - y) + 1;
		}
		drawCircleLine(src, dest, x + 1, y + 1);
	}
}

void MainMenu::drawCircleLine(const Graphics::Surface *src,
		Graphics::ManagedSurface *dest, int x, int y) {
	const byte *src1, *src2;
	byte *dest1;

	src1 = (const byte *)src->getBasePtr(CIRCLE_X + CIRCLE_RADIUS - x, CIRCLE_Y + CIRCLE_RADIUS + y);
	src2 = (const byte *)src->getBasePtr(CIRCLE_X + CIRCLE_RADIUS + x, CIRCLE_Y + CIRCLE_RADIUS + y);
	dest1 = (byte *)dest->getBasePtr(CIRCLE_RADIUS - x, CIRCLE_RADIUS + y);

const byte *dest2 = dest1 + (src2 - src1);
assert(dest2 < (const byte *)dest->getBasePtr(0, dest->h));
	Common::copy(src1, src2, dest1);

	src1 = (const byte *)src->getBasePtr(CIRCLE_X + CIRCLE_RADIUS - x, CIRCLE_Y + CIRCLE_RADIUS - y);
	src2 = (const byte *)src->getBasePtr(CIRCLE_X + CIRCLE_RADIUS + x, CIRCLE_Y + CIRCLE_RADIUS - y);
	dest1 = (byte *)dest->getBasePtr(CIRCLE_RADIUS - x, CIRCLE_RADIUS - y);
	Common::copy(src1, src2, dest1);

	src1 = (const byte *)src->getBasePtr(CIRCLE_X + CIRCLE_RADIUS - y, CIRCLE_Y + CIRCLE_RADIUS - x);
	src2 = (const byte *)src->getBasePtr(CIRCLE_X + CIRCLE_RADIUS + y, CIRCLE_Y + CIRCLE_RADIUS - x);
	dest1 = (byte *)dest->getBasePtr(CIRCLE_RADIUS - y, CIRCLE_RADIUS - x);
	Common::copy(src1, src2, dest1);

	src1 = (const byte *)src->getBasePtr(CIRCLE_X + CIRCLE_RADIUS - y, CIRCLE_Y + CIRCLE_RADIUS + x);
	src2 = (const byte *)src->getBasePtr(CIRCLE_X + CIRCLE_RADIUS + y, CIRCLE_Y + CIRCLE_RADIUS + x);
	dest1 = (byte *)dest->getBasePtr(CIRCLE_RADIUS - y, CIRCLE_RADIUS + x);
	Common::copy(src1, src2, dest1);
}

void MainMenu::draw() {
	Graphics::ManagedSurface s = getSurface();
	s.clear(0);
	ScrollView::draw();

	s.blitFrom(_circles[0], Common::Point(160 - CIRCLE_RADIUS * 2 - 10, 10));
	s.blitFrom(_circles[1], Common::Point(160 + 10, 10));

	writeString(0, 100, STRING["dialogs.main_menu.title3"], ALIGN_MIDDLE);
	writeString(0, 110, STRING["dialogs.main_menu.title4"], ALIGN_MIDDLE);

	Common::String line1 = STRING["dialogs.main_menu.option1"];
	Common::String line2 = STRING["dialogs.main_menu.option2"];
	Common::String line3 = STRING["dialogs.main_menu.option3"];

	writeString(80, 120, Common::String(line1.c_str(), line1.c_str() + 3));
	writeString(80, 130, Common::String(line2.c_str(), line2.c_str() + 3));
	writeString(80, 140, Common::String(line3.c_str(), line3.c_str() + 3));
	writeString(110, 120, Common::String(line1.c_str() + line1.findLastOf('.') + 1));
	writeString(110, 130, Common::String(line2.c_str() + line2.findLastOf('.') + 1));
	writeString(110, 140, Common::String(line3.c_str() + line3.findLastOf('.') + 1));

	writeString(0, 165, STRING["dialogs.main_menu.copyright1"], ALIGN_MIDDLE);
	writeString(0, 175, STRING["dialogs.main_menu.scummvm"], ALIGN_MIDDLE);
}

bool MainMenu::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_c:
		addView("CreateCharacters");
		break;

	case Common::KEYCODE_v:
		addView("Characters");
		break;

	case Common::KEYCODE_1:
	case Common::KEYCODE_2:
	case Common::KEYCODE_3:
	case Common::KEYCODE_4:
	case Common::KEYCODE_5:
		g_globals->_startingTown = (Maps::TownId)(
			msg.keycode - Common::KEYCODE_0);
		replaceView("Inn");
		break;

	default:
		break;
	}

	return true;

}

bool msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT) {
	}

	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
