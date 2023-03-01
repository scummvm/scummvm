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

Interaction::Interaction(const Common::String &name, int portrait) : ScrollView(name) {
	_bounds = Common::Rect(8, 8, 224, 140);
	_frame.load("frame.fac");
	_portrait.load(Common::String::format("face%02d.fac", portrait));
}

void Interaction::addText(const Common::String &str) {
	_lines = splitLines(searchAndReplace(str, "\n", " "));
}

bool Interaction::msgGame(const GameMessage &msg) {
	if (msg._name == "DISPLAY") {
		addView(this);
		return true;
	}

	return ScrollView::msgGame(msg);
}

bool Interaction::msgUnfocus(const UnfocusMessage &msg) {
	ScrollView::msgUnfocus(msg);
	return true;
}

void Interaction::draw() {
  	ScrollView::draw();

	Graphics::ManagedSurface s = getSurface();
	_frame.draw(&s, 0, Common::Point(8, 8));
	_portrait.draw(&s, _portraitFrameNum, Common::Point(15, 14));

	for (uint i = 0; i < _lines.size(); ++i) {
		writeLine(6 + i, _lines[i], ALIGN_MIDDLE);
	}
}

bool Interaction::tick() {
	if (_animated && ++_tickCtr >= 10) {
		_tickCtr = 0;
		_portraitFrameNum = g_engine->getRandomNumber(3);
		redraw();
	}

	return true;
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
		_animated = false;
		_lines.clear();
		++_portraitNum;
		_portrait.load(Common::String::format("face%02d.fac", _portraitNum));

		Interaction::draw();
		writeNumber(20, 70, _portraitNum);

	} else {
		viewAction();
	}

	return true;
}

bool Interaction::msgMouseDown(const MouseDownMessage &msg) {
	viewAction();
	return true;
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
