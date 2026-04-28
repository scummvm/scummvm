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

#include "mm/mm1/views_enh/town_select.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

static const int BTN_SIZE = 10;
static const int BTN_X = 14;
static const int TEXT_X = BTN_X + BTN_SIZE + 5;

TownSelect::TownSelect() : ScrollView("TownSelect") {
	_bounds = Common::Rect(87, 49, 233, 155);
	_bounds.setBorderSize(10);

	for (int i = 1; i <= 5; ++i) {
		int row = i - 1;
		int y = 22 + row * 12;
		Common::String text = STRING[Common::String::format("stats.towns.%d", i)];
		addButton(Common::Rect(BTN_X, y, TEXT_X + (int)getStringWidth(text),
				y + BTN_SIZE),
			(Common::KeyCode)(Common::KEYCODE_0 + i));
	}
}

void TownSelect::draw() {
	ScrollView::draw();
	Graphics::ManagedSurface s = getSurface();
	setReduced(false);
	writeString(0, 0, STRING["spells.which_town"], ALIGN_MIDDLE);

	Graphics::ManagedSurface btnSmall(BTN_SIZE, BTN_SIZE);
	btnSmall.blitFrom(g_globals->_blankButton, Common::Rect(0, 0, 20, 20),
		Common::Rect(0, 0, BTN_SIZE, BTN_SIZE));

	for (int i = 1; i <= 5; ++i) {
		int y = 22 + (i - 1) * 12;
		s.blitFrom(btnSmall, Common::Point(BTN_X + _bounds.borderSize() + 1,
			y + _bounds.borderSize() - 1));

		writeString(BTN_X + (BTN_SIZE / 2) + 1, y,
			Common::String::format("%d", i), ALIGN_MIDDLE);

		byte oldColor = setTextColor(i == _selectedButton + 1 ? 15 : 0);
		writeString(TEXT_X, y, STRING[Common::String::format("stats.towns.%d", i)]);
		setTextColor(oldColor);
	}
}

bool TownSelect::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode >= Common::KEYCODE_1 && msg.keycode <= Common::KEYCODE_5) {
		g_globals->_startingTown = (Maps::TownId)(msg.keycode - Common::KEYCODE_0);
		close();
		send("MainMenu", GameMessage("START_TOWN", (int)g_globals->_startingTown));
		return true;
	}

	return ScrollView::msgKeypress(msg);
}

bool TownSelect::msgMouseMove(const MouseMoveMessage &msg) {
	int selectedButton = getButtonAt(msg._pos);
	if (selectedButton != _selectedButton) {
		_selectedButton = selectedButton;
		redraw();
	}

	return true;
}

bool TownSelect::msgMouseDown(const MouseDownMessage &msg) {
	if (msg._button == MouseMessage::MB_RIGHT || !getBounds().contains(msg._pos)) {
		msgAction(ActionMessage(KEYBIND_ESCAPE));
		return true;
	}

	return ScrollView::msgMouseDown(msg);
}

bool TownSelect::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return ScrollView::msgAction(msg);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
