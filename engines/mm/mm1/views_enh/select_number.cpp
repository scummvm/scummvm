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

#include "mm/mm1/views_enh/select_number.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

void SelectNumber::openNumbers(int maxNum) {
	_subView.open(maxNum, [](int choiceNum) {
		g_events->send(KeypressMessage(Common::KeyState(
			(Common::KeyCode)(Common::KEYCODE_0 + choiceNum),
			'0' + choiceNum
		)));
	});
}

bool SelectNumber::msgMouseDown(const MouseDownMessage &msg) {
	if (_subView.msgMouseDown(msg))
		return true;

	return false;
}

/*------------------------------------------------------------------------*/

SelectNumberSubview::SelectNumberSubview() : ScrollView("SelectNumberSubview") {
	_bounds = Common::Rect(234, 144, 320, 200);

	// Get the Escape glyph we'll use as a base
	Shared::Xeen::SpriteResource escSprite;
	escSprite.load("esc.icn");
	_button.create(20, 20);
	_button.clear(255);
	_button.setTransparentColor(255);
	escSprite.draw(&_button, 0, Common::Point(0, 0));
	_button.fillRect(Common::Rect(2, 2, 18, 18), 0x9f);
}

void SelectNumberSubview::open(int maxNum, SelectNumberProc callback) {
	assert(maxNum <= 6);
	_maxNumber = maxNum;
	_callback = callback;

	draw();
}

void SelectNumberSubview::close() {
	_maxNumber = 0;
	g_events->redraw();
}

void SelectNumberSubview::draw() {
	if (_maxNumber == 0)
		return;

	ScrollView::draw();
	Graphics::ManagedSurface s = getSurface();

	for (int i = 0; i < _maxNumber; ++i) {
		
		s.blitFrom(_button, Common::Point((i % 3) * 22 + 8,
			(i / 3) * 22 + 4));
		writeString((i % 3) * 22 + 10, (i / 3) * 22 + 2,
			Common::String::format("%d", i + 1), ALIGN_MIDDLE);
	}
}

bool SelectNumberSubview::msgMouseDown(const MouseDownMessage &msg) {
	if (_maxNumber == 0)
		return false;

	Common::Rect r(0, 0, 20, 20);

	for (int i = 0; i < _maxNumber; ++i) {
		r.moveTo(Common::Point((i % 3) * 22 + _innerBounds.left,
			(i / 3) * 22 + _innerBounds.top));

		if (r.contains(msg._pos)) {
			close();
			_callback(i + 1);
			return true;
		}
	}

	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
