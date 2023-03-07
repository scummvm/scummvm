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

#include "mm/mm1/views_enh/characters.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Characters::Characters() : ScrollView("Characters") {
	_bounds.setBorderSize(10);
	_escSprite.load("esc.icn");
	addButton(&_escSprite, Common::Point(120, 172), 0, KEYBIND_ESCAPE, true);
}

void Characters::draw() {
	ScrollView::draw();
	Graphics::ManagedSurface s = getSurface();
	Roster &roster = g_globals->_roster;
	_charIndexes.clear();

	// Write title
	setReduced(false);
	writeString(0, 0, STRING["dialogs.view_characters.title"], ALIGN_MIDDLE);

	if (g_globals->_roster.empty()) {
		writeString(0, 40, STRING["dialogs.misc.no_characters"], ALIGN_MIDDLE);

	} else {
		// Write out the roster
		setReduced(true);

		for (uint charNum = 0; charNum < ROSTER_COUNT; ++charNum) {
			if (!roster._towns[charNum])
				continue;
			const Character &c = roster[charNum];
			_charIndexes.push_back(charNum);
			int idx = _charIndexes.size() - 1;

			// Build up character portrait and/or frame
			Graphics::ManagedSurface portrait;
			portrait.create(30, 30);
			c._faceSprites.draw(&portrait, 0, Common::Point(0, 0));

			Common::Point pt(_innerBounds.left + _innerBounds.width() / 3
				* (idx % 3), 20 + 20 * (idx / 3));
			s.blitFrom(portrait, Common::Rect(0, 0, 30, 30),
				Common::Rect(pt.x + 2, pt.y + 2, pt.x + 17, pt.y + 17));

			writeString(pt.x - _innerBounds.left + 22, pt.y - _innerBounds.top + 5, c._name);
		}

		setReduced(false);
		writeString(0, 152, STRING["enhdialogs.characters.left_click"], ALIGN_MIDDLE);
	}

	writeString(135, 174, STRING["enhdialogs.misc.go_back"]);
}

bool Characters::msgMouseDown(const MouseDownMessage &msg) {
	// Cycle through portraits
	for (uint idx = 0; idx < _charIndexes.size(); ++idx) {
		Common::Point pt(_innerBounds.left + _innerBounds.width() / 3
			* (idx % 3), 20 + 20 * (idx / 3));

		if (Common::Rect(pt.x, pt.y, pt.x + 19, pt.y + 19).contains(msg._pos)) {
			g_globals->_currCharacter = &g_globals->_roster[_charIndexes[idx]];
			_characterView.addView();
			return true;
		}
	}

	return ScrollView::msgMouseDown(msg);
}

bool Characters::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode >= Common::KEYCODE_a &&
			msg.keycode < (Common::KeyCode)(Common::KEYCODE_a + _charIndexes.size())) {
		g_globals->_currCharacter = &g_globals->_roster[_charIndexes[msg.keycode - Common::KEYCODE_a]];
		_characterView.addView();
		return true;
	}

	return false;
}

bool Characters::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		replaceView("MainMenu");
		return true;
	default:
		return ScrollView::msgAction(msg);
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
