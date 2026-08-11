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

#include "ultima/ultima0/views/title.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

Title::Title() : View("Title") {
}

bool Title::msgFocus(const FocusMessage &msg) {
	_highlightedOption = 0;
	updateSelections();
	MetaEngine::setKeybindingMode(KBMODE_MENUS);


	Common::String priorView = msg._priorView->getName();
	if (priorView == "Startup" || priorView == "Dead") {
		g_engine->playMidi("intro.mid");
	}

	return true;
}

bool Title::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return true;
}

void Title::updateSelections() {
	const int selected = getColor(255, 0, 128);
	const int white = getColor(255, 255, 255);

	for (int i = 0; i < 4; ++i) {
		auto &opt = _options[i];
		opt._color = opt._index == _highlightedOption ? selected : white;
		opt.redraw();
	}
}

void Title::draw() {
	auto s = getSurface();
	s.clear();

	View::draw();

	s.writeString(Common::Point(20, 8), "Ultima 0 - Akalabeth!", Graphics::kTextAlignCenter);
}

bool Title::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_UP:
		_highlightedOption = _highlightedOption ? _highlightedOption - 1 : 3;
		updateSelections();
		break;
	case KEYBIND_DOWN:
		_highlightedOption = (_highlightedOption + 1) % 4;
		updateSelections();
		break;
	case KEYBIND_SELECT:
		selectOption();
		break;
	default:
		break;
	}

	return true;
}

bool Title::msgGame(const GameMessage &msg) {
	if (msg._name == "SELECTION") {
		_highlightedOption = msg._value;
		updateSelections();
		return true;
	}

	return false;
}

bool Title::msgMouseDown(const MouseDownMessage &msg) {
	selectOption();
	return true;
}

void Title::selectOption() {
	if (_highlightedOption == 1 || _highlightedOption == 3)
		g_engine->stopMidi();

	if (_highlightedOption == 3) {
		if (g_engine->savegamesExist()) {
			g_engine->loadGameDialog();
		} else {
			// Otherwise to go the Create Character view
			replaceView("CreateCharacter");
		}
	} else {
		const char *VIEW_NAMES[4] = { "Intro", "CreateCharacter", "Acknowledgements" };
		replaceView(VIEW_NAMES[_highlightedOption]);
	}
}

/*-------------------------------------------------------------------*/

Title::TitleOption::TitleOption(Title *parent, int index, const Common::String &text, int row) :
		UIElement("TitleOption", parent), _index(index), _text(text) {
	int xs = 20 - text.size() / 2;
	setBounds(Gfx::TextRect(xs, row, xs + text.size(), row));
}

void Title::TitleOption::draw() {
	auto s = getSurface();
	s.setColor(_color);
	s.writeString(_text);
}

bool Title::TitleOption::msgMouseEnter(const MouseEnterMessage &msg) {
	_parent->send(GameMessage("SELECTION", _index));
	return true;
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
