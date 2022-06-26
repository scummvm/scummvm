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

#include "mm/mm1/views_enh/character_info.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

const CharacterInfo::IconPos CharacterInfo::ICONS[CHAR_ICONS_COUNT] = {
	{ 0, 2, 16 },
	{ 2, 2, 39 },
	{ 4, 2, 62 },
	{ 6, 2, 85 },
	{ 8, 2, 108 },

	{ 10, 53, 16 },
	{ 12, 53, 39 },
	{ 14, 53, 62 },
	{ 16, 53, 85 },
	{ 18, 53, 108 },

	{ 20, 104, 16 },
	{ 22, 104, 39 },
	{ 24, 104, 62 },

	{ 30, 169, 16 },
	{ 32, 169, 39 },
	{ 34, 169, 62 },
	{ 36, 169, 85 },
	{ 38, 169, 108 },
	{ 40, 277, 3 },
	{ 42, 277, 35 },
	{ 44, 277, 67 },
	{ 46, 277, 99 }
};


CharacterInfo::CharacterInfo() : ScrollView("CharacterInfo") {
	_bounds = Common::Rect(0, 0, 320, 146);
	static const char *FIELDS[CHAR_ICONS_COUNT] = {
		"might", "intelligence", "personality",
		"endurance", "speed",
		"accuracy", "luck", "age", "level", "ac",
		"hp", "sp", "spells",
		"experience", "gold", "gems", "food", "condition"
	};

	for (int i = 0; i < 18; ++i) {
		ICONS_TEXT[i] = STRING[Common::String::format(
			"enhdialogs.character.%s", FIELDS[i])].c_str();
	}
}

bool CharacterInfo::msgFocus(const FocusMessage &msg) {
	_viewIcon.load("view.icn");
	return ScrollView::msgFocus(msg);
}

bool CharacterInfo::msgUnfocus(const UnfocusMessage &msg) {
	_viewIcon.clear();
	return ScrollView::msgUnfocus(msg);
}

void CharacterInfo::draw() {
	ScrollView::draw();
	drawIcons();
}

void CharacterInfo::drawIcons() {
	Graphics::ManagedSurface s = getSurface();
	for (int i = 0; i < CHAR_ICONS_COUNT; ++i) {
		_viewIcon.draw(&s, ICONS[i]._frame,
			Common::Point(ICONS[i]._x + FRAME_BORDER_SIZE,
				ICONS[i]._y + FRAME_BORDER_SIZE));
	}

	// Write text
	setTextColor(0);
	_textPos = Common::Point(8, 8);
	writeString(5, 4, ICONS_TEXT[0]);
}

} // namespace Views
} // namespace MM1
} // namespace MM
