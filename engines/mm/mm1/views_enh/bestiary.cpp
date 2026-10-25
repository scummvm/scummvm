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

#include "mm/mm1/views_enh/bestiary.h"
#include "mm/mm1/views_enh/bestiary_stats.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/metaengine.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

constexpr int BESTIARY_PAGE_SIZE = 12;

Bestiary::Bestiary() : ScrollView("Bestiary") {
	setBounds(Common::Rect(0, 0, 320, 200));
	addButton(&g_globals->_escSprites, Common::Point(134, 170), 0, KEYBIND_ESCAPE, true);
	addButton(Common::Rect(8, 168, 76, 183), KEYBIND_BESTIARY_PAGEUP);
	addButton(Common::Rect(244, 168, 304, 183), KEYBIND_BESTIARY_PAGEDOWN);
}

void Bestiary::buildOrder() {
	if (!_monsterIds.empty())
		return;

	// Insertion sort of the display IDs by monster name
	for (int id = 1; id <= MONSTERS_COUNT; ++id) {
		_monsterIds.push_back(id);

		int pos = id - 1;
		while (pos > 0 && (*g_globals->_monsters)[id]._name.compareToIgnoreCase(
				(*g_globals->_monsters)[_monsterIds[pos - 1]]._name) < 0) {
			_monsterIds[pos] = _monsterIds[pos - 1];
			--pos;
		}

		_monsterIds[pos] = id;
	}
}

bool Bestiary::msgFocus(const FocusMessage &msg) {
	buildOrder();
	ScrollView::msgFocus(msg);
	MetaEngine::setKeybindingMode(KBMODE_BESTIARY);
	return true;
}

void Bestiary::draw() {
	ScrollView::draw();
	setTextColor(0);
	setReduced(false);
	writeLine(0, "Bestiary", ALIGN_MIDDLE);

	setReduced(true);
	writeString(0, 17, Common::String::format("Discovered: %u/%u",
		g_globals->_bestiary.count(), (uint)MONSTERS_COUNT), ALIGN_MIDDLE);

	for (int row = 0; row < BESTIARY_PAGE_SIZE; ++row) {
		const int index = _topIndex + row;
		if (index >= MONSTERS_COUNT)
			break;

		const int id = _monsterIds[index];
		if (index == _selectedIndex) {
			setTextColor(15);
			writeString(28, 33 + row * 9, ">");
		}

		setTextColor(g_globals->_bestiary.hasSeen(id) ? 37 : 1);
		writeString(44, 33 + row * 9, Common::String::format("%03d  %s",
			index + 1, (*g_globals->_monsters)[id]._name.c_str()));
	}

	setTextColor(0);
	writeString(0, 145, Common::String::format("%d-%d of %d",
		_topIndex + 1, _topIndex + BESTIARY_PAGE_SIZE, MONSTERS_COUNT), ALIGN_MIDDLE);
	writeString(0, 155, "Up/Down: select  Enter: details", ALIGN_MIDDLE);

	setTextColor(_topIndex > 0 ? 37 : 1);
	writeString(8, 172, "Previous");
	setTextColor(_topIndex < MONSTERS_COUNT - BESTIARY_PAGE_SIZE ? 37 : 1);
	writeString(262, 172, "Next");

	setTextColor(0);
	writeString(152, 172, "Go back");
}

void Bestiary::select(int index) {
	_selectedIndex = CLIP(index, 0, MONSTERS_COUNT - 1);

	if (_selectedIndex < _topIndex)
		_topIndex = _selectedIndex;
	else if (_selectedIndex >= _topIndex + BESTIARY_PAGE_SIZE)
		_topIndex = _selectedIndex - BESTIARY_PAGE_SIZE + 1;

	_topIndex = CLIP(_topIndex, 0, MONSTERS_COUNT - BESTIARY_PAGE_SIZE);
	redraw();
}

void Bestiary::scroll(int delta) {
	const int oldTop = _topIndex;
	_topIndex = CLIP(_topIndex + delta, 0, MONSTERS_COUNT - BESTIARY_PAGE_SIZE);
	_selectedIndex = CLIP(_selectedIndex + _topIndex - oldTop,
		_topIndex, _topIndex + BESTIARY_PAGE_SIZE - 1);
	redraw();
}

void Bestiary::openImage() {
	g_events->send("BestiaryImage", GameMessage("MONSTER", _monsterIds[_selectedIndex]));
	addView("BestiaryImage");
}

bool Bestiary::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
	case KEYBIND_BESTIARY:
		close();
		break;
	case KEYBIND_SELECT:
		openImage();
		break;
	case KEYBIND_BESTIARY_UP:
		select(_selectedIndex - 1);
		break;
	case KEYBIND_BESTIARY_DOWN:
		select(_selectedIndex + 1);
		break;
	case KEYBIND_BESTIARY_PAGEUP:
		scroll(-BESTIARY_PAGE_SIZE);
		break;
	case KEYBIND_BESTIARY_PAGEDOWN:
		scroll(BESTIARY_PAGE_SIZE);
		break;
	case KEYBIND_BESTIARY_HOME:
		select(0);
		break;
	case KEYBIND_BESTIARY_END:
		select(MONSTERS_COUNT - 1);
		break;
	default:
		return false;
	}

	return true;
}

bool Bestiary::msgKeypress(const KeypressMessage &msg) {
	// Some keyboard paths report the navigation-labelled numpad keys as
	// KP#. Numeric keypad input is preserved while Num Lock is on
	if (!(msg.flags & Common::KBD_NUM)) {
		switch (msg.keycode) {
		case Common::KEYCODE_KP9:
			scroll(-BESTIARY_PAGE_SIZE);
			return true;
		case Common::KEYCODE_KP3:
			scroll(BESTIARY_PAGE_SIZE);
			return true;
		case Common::KEYCODE_KP8:
			select(_selectedIndex - 1);
			return true;
		case Common::KEYCODE_KP2:
			select(_selectedIndex + 1);
			return true;
		case Common::KEYCODE_KP7:
			select(0);
			return true;
		case Common::KEYCODE_KP1:
			select(MONSTERS_COUNT - 1);
			return true;
		default:
			break;
		}
	}

	switch (msg.keycode) {
	case Common::KEYCODE_UP:
		select(_selectedIndex - 1);
		break;
	case Common::KEYCODE_DOWN:
		select(_selectedIndex + 1);
		break;
	case Common::KEYCODE_PAGEUP:
		scroll(-BESTIARY_PAGE_SIZE);
		break;
	case Common::KEYCODE_PAGEDOWN:
		scroll(BESTIARY_PAGE_SIZE);
		break;
	case Common::KEYCODE_HOME:
		select(0);
		break;
	case Common::KEYCODE_END:
		select(MONSTERS_COUNT - 1);
		break;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		openImage();
		break;
	case Common::KEYCODE_n:
	case Common::KEYCODE_ESCAPE:
		close();
		break;
	default:
		break;
	}

	return true;
}

bool Bestiary::msgMouseUp(const MouseUpMessage &msg) {
	if (ScrollView::msgMouseUp(msg))
		return true;

	const int y = msg._pos.y - _innerBounds.top;
	if (msg._button == MouseMessage::MB_LEFT && _innerBounds.contains(msg._pos) &&
			y >= 33 && y < 33 + BESTIARY_PAGE_SIZE * 9) {
		select(_topIndex + (y - 33) / 9);
		return true;
	}

	return false;
}

BestiaryImage::BestiaryImage() : ScrollView("BestiaryImage") {
	setBounds(Common::Rect(0, 0, 320, 200));
	addButton(&g_globals->_escSprites, Common::Point(134, 170), 0, KEYBIND_ESCAPE, true);
	addButton(Common::Rect(148, 168, 220, 183), KEYBIND_ESCAPE);
	addButton(Common::Rect(8, 168, 76, 183), KEYBIND_BESTIARY_PAGEUP);
	addButton(Common::Rect(240, 168, 304, 183), KEYBIND_BESTIARY_PAGEDOWN);
}

bool BestiaryImage::msgGame(const GameMessage &msg) {
	if (msg._name != "MONSTER" || msg._value < 1 || msg._value > MONSTERS_COUNT)
		return false;

	_monsterId = msg._value;
	_statsPage = 0;

	const Monster &monster = (*g_globals->_monsters)[_monsterId];
	_image = g_globals->_monsters->getMonsterImage(monster._imgNum);
	return true;
}

bool BestiaryImage::msgFocus(const FocusMessage &msg) {
	ScrollView::msgFocus(msg);
	MetaEngine::setKeybindingMode(KBMODE_BESTIARY);
	return true;
}

void BestiaryImage::changePage(int delta) {
	_statsPage = g_globals->_bestiary.hasSeen(_monsterId) ?
		CLIP(_statsPage + delta, 0, 1) : 0;
	redraw();
}

void BestiaryImage::draw() {
	ScrollView::draw();
	setReduced(false);
	setTextColor(0);

	const Monster &monster = (*g_globals->_monsters)[_monsterId];
	writeLine(0, monster._name, ALIGN_MIDDLE);

	setReduced(true);
	const bool seen = g_globals->_bestiary.hasSeen(_monsterId);
	setTextColor(seen ? 37 : 1);
	writeString(0, 16, !seen ? "Undiscovered" :
		(_statsPage == 0 ? "Discovered - Stats & specials (1/2)" :
			"Discovered - Defenses & specials (2/2)"), ALIGN_MIDDLE);

	// Keep the original 104x96 sprite centered between two 94-pixel columns
	getSurface().blitFrom(_image, Common::Point((_bounds.width() - _image.w) / 2, 48));

	if (!seen) {
		writeString(0, 151, "Stats unlock after an encounter.", ALIGN_MIDDLE);
	} else {
		setTextColor(0);

		for (int col = 0; col < 2; ++col) {
			const int x = col ? 210 : 0;
			int y = 32;
			const Common::Array<Common::String> lines =
				bestiaryColumn(monster, seen, col != 0, _statsPage);

			for (uint i = 0; i < lines.size(); ++i) {
				const Common::StringArray wrapped = splitLines(lines[i], 94);
				for (uint j = 0; j < wrapped.size(); ++j) {
					writeString(x, y, wrapped[j]);
					y += 9;
				}
			}
		}

		writeString(104, 142, "HP: base value");
		writeString(104, 151, "MR: rating");
	}

	setTextColor(seen && _statsPage > 0 ? 37 : 1);
	writeString(8, 172, "Stats");
	setTextColor(seen && _statsPage == 0 ? 37 : 1);
	writeString(244, 172, "Defenses");
	setTextColor(0);
	writeString(152, 172, "Back to list");
}

bool BestiaryImage::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
	case KEYBIND_SELECT:
	case KEYBIND_BESTIARY:
		close();
		break;
	case KEYBIND_BESTIARY_PAGEUP:
	case KEYBIND_BESTIARY_HOME:
		changePage(-1);
		break;
	case KEYBIND_BESTIARY_PAGEDOWN:
	case KEYBIND_BESTIARY_END:
		changePage(1);
		break;
	default:
		return false;
	}

	return true;
}

bool BestiaryImage::msgKeypress(const KeypressMessage &msg) {
	if (!(msg.flags & Common::KBD_NUM)) {
		if (msg.keycode == Common::KEYCODE_KP9) {
			changePage(-1);
			return true;
		}

		if (msg.keycode == Common::KEYCODE_KP3) {
			changePage(1);
			return true;
		}
	}

	switch (msg.keycode) {
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_PAGEUP:
	case Common::KEYCODE_HOME:
		changePage(-1);
		break;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_PAGEDOWN:
	case Common::KEYCODE_END:
		changePage(1);
		break;
	case Common::KEYCODE_ESCAPE:
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
	case Common::KEYCODE_n:
		close();
		break;
	default:
		break;
	}

	return true;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
