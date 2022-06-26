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

bool CharacterInfo::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		close();
		return true;
	}

	return false;
}

void CharacterInfo::draw() {
	ScrollView::draw();
	drawTitle();
	drawIcons();
	drawStats();
}

void CharacterInfo::drawTitle() {
	const Character &c = *g_globals->_currCharacter;
	Common::String msg = Common::String::format(
		"%s : %s %s %s",
		c._name,
		"X",
		"Y",
		"Z"
	);

	writeString(8, 8, msg);
}

void CharacterInfo::drawIcons() {
	Graphics::ManagedSurface s = getSurface();
	for (int i = 0; i < CHAR_ICONS_COUNT; ++i) {
		_viewIcon.draw(&s, ICONS[i]._frame,
			Common::Point(ICONS[i]._x + FRAME_BORDER_SIZE,
				ICONS[i]._y + FRAME_BORDER_SIZE));
	}
}

void CharacterInfo::drawStats() {
	// Draw stat titles
	for (int i = 0; i < 18; ++i) {
		writeString(ICONS[i]._x + 35, ICONS[i]._y + 10,
			ICONS_TEXT[i]);
	}

	// Draw stat values
	const Character &c = *g_globals->_currCharacter;
	const uint CURR[17] = {
		c._might._current, c._intelligence._current,
		c._personality._current, c._endurance._current,
		c._speed._current, c._accuracy._current,
		c._luck._current, c._age._base, c._level._current,
		c._ac._current, c._hp, c._sp._current, 0,
		c._exp, c._gold, c._gems, c._food
	};
	const uint BASE[17] = {
		c._might._base, c._intelligence._base,
		c._personality._base, c._endurance._base,
		c._speed._base, c._accuracy._base,
		c._luck._base, c._age._base, c._level._base,
		c._ac._base, c._hp, c._sp._base, 0,
		c._exp, c._gold, c._gems, c._food
	};

	for (int i = 0; i < 17; ++i) {
		if (i == 12)
			continue;

		Common::Point pt(ICONS[i]._x + 35, ICONS[i]._y + 19);
		if (i < 10)
			pt.x += 8 + (CURR[i] < 10 ? 8 : 0);

		setTextColor(statColor(CURR[i], BASE[i]));
		writeNumber(pt.x, pt.y, CURR[i]);
	}
}

int CharacterInfo::statColor(int amount, int threshold) {
	if (amount < 1)
		return 6;
	else if (amount > threshold)
		return 2;
	else if (amount == threshold)
		return 15;
	else if (amount >= (threshold / 4))
		return 9;
	else
		return 32;
}

} // namespace Views
} // namespace MM1
} // namespace MM
