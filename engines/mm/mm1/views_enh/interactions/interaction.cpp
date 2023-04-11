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

#include "mm/mm1/views_enh/interactions/interaction.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/shared/utils/strings.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

#define BTN_SIZE 10

Interaction::Interaction(const Common::String &name, int portrait) : PartyView(name) {
	_bounds = Common::Rect(8, 8, 224, 140);

	if (portrait != -1) {
		_frame.load("frame.fac");
		_portrait.load(Common::String::format("face%02d.fac", portrait));
	}
}

void Interaction::addText(const Common::String &str) {
	_lines = splitLines(searchAndReplace(str, "\n", " "));
}

bool Interaction::msgGame(const GameMessage &msg) {
	if (msg._name == "DISPLAY") {
		addView(this);
		return true;
	}

	return PartyView::msgGame(msg);
}

bool Interaction::msgUnfocus(const UnfocusMessage &msg) {
	PartyView::msgUnfocus(msg);
	return true;
}

void Interaction::draw() {
  	PartyView::draw();

	Graphics::ManagedSurface s = getSurface();

	if (!_frame.empty()) {
		_frame.draw(&s, 0, Common::Point(8, 8));
		_portrait.draw(&s, _portraitFrameNum, Common::Point(15, 14));
	}

	setReduced(false);
	if (!_title.empty()) {
		size_t strWidth = getStringWidth(_title);
		writeString(125 - strWidth / 2, 20, _title);
	}

	// Write any text lines
	for (uint i = 0; i < _lines.size(); ++i) {
		writeString(0, (6 + i) * 9 - 5, _lines[i], ALIGN_MIDDLE);
	}

	// Write out any buttons
	if (!_buttons.empty()) {
		_textPos = Common::Point(0, (8 + _lines.size()) * 8);
		setReduced(true);

		// Create a blank button
		Graphics::ManagedSurface btnSmall(BTN_SIZE, BTN_SIZE);
		btnSmall.blitFrom(g_globals->_blankButton, Common::Rect(0, 0, 20, 20),
			Common::Rect(0, 0, BTN_SIZE, BTN_SIZE));

		for (uint i = 0; i < _buttons.size(); ++i, _textPos.x += 10) {
			InteractionButton &btn = _buttons[i];

			int itemWidth = BTN_SIZE + 5 + getStringWidth(_buttons[i]._text);
			if ((_textPos.x + itemWidth) > _innerBounds.width()) {
				_textPos.x = 0;
				_textPos.y += BTN_SIZE + 2;
			}
			Common::Point pt = _textPos;

			// Display button and write character in the middle
			s.blitFrom(btnSmall, Common::Point(pt.x + _bounds.borderSize(),
				pt.y + _bounds.borderSize()));

			writeString(pt.x + (BTN_SIZE / 2) + 1, pt.y,
				Common::String::format("%c", _buttons[i]._c), ALIGN_MIDDLE);

			// Write text to the right of the button
			writeString(pt.x + BTN_SIZE + 5, pt.y, _buttons[i]._text);

			// Set up bounds for the area covered by the button & text
			btn._bounds = Common::Rect(pt.x, pt.y,
				pt.x + BTN_SIZE + 5 + itemWidth, pt.y + BTN_SIZE);
			btn._bounds.translate(_innerBounds.left, _innerBounds.top);
		}
	}
}

bool Interaction::tick() {
	if (_animated && ++_tickCtr >= 10) {
		_tickCtr = 0;
		_portraitFrameNum = g_engine->getRandomNumber(0, 2);
		redraw();
	}

	return PartyView::tick();
}

void Interaction::leave() {
	if (g_events->focusedView() == this)
		close();

	g_maps->turnAround();
	g_events->redraw();
}

bool Interaction::msgKeypress(const KeypressMessage &msg) {
	viewAction();
	return true;
}

bool Interaction::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		leave();

	} else if (msg._action == KEYBIND_SELECT) {
		// ***DEBUG*** - Used for cycling through portraits.
		// To let me pick good portraits from Xeen
		_lines.clear();
		++_portraitNum;
		_portrait.load(Common::String::format("face%02d.fac", _portraitNum));

		Interaction::draw();
		writeNumber(20, 70, _portraitNum);

	} else if (!PartyView::msgAction(msg)) {
		viewAction();
	}

	return true;
}

bool Interaction::msgMouseDown(const MouseDownMessage &msg) {
	if (!PartyView::msgMouseDown(msg)) {
		// Check if a button was pressed
		for (uint i = 0; i < _buttons.size(); ++i) {
			const auto &btn = _buttons[i];
			if (_buttons[i]._bounds.contains(msg._pos)) {
				msgKeypress(KeypressMessage(Common::KeyState(
					(Common::KeyCode)(Common::KEYCODE_a + btn._c - 'A'), btn._c
				)));
				return true;
			}
		}

		// Fall back on treating click as a standard acknowledgement
		viewAction();
	}

	return true;
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
