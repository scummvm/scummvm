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
#include "mm/shared/utils/strings.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define CURSOR_BLINK_FRAMES 6
#define ICONS_COUNT 18
#define REDUCED_TO_EXPANDED(IDX) ((IDX >= 13) ? IDX + 2 : IDX)
#define EXPANDED_TO_REDUCED(IDX) ((IDX >= 13) ? IDX - 2 : IDX)

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


CharacterInfo::CharacterInfo() :
		PartyView("CharacterInfo"), _statInfo("ScrollText") {
	_bounds = Common::Rect(0, 0, 320, 146);
	_statInfo.setReduced(true);

	static const char *FIELDS[CHAR_ICONS_COUNT] = {
		"might", "intelligence", "personality",
		"endurance", "speed",
		"accuracy", "luck", "age", "level", "ac",
		"hp", "sp", "spells",
		"experience", "gold", "gems", "food", "condition"
	};

	for (int i = 0; i < ICONS_COUNT; ++i) {
		ICONS_TEXT[i] = STRING[Common::String::format(
			"enhdialogs.character.stats.%s", FIELDS[i])];
	}
}

bool CharacterInfo::msgFocus(const FocusMessage &msg) {
	_viewIcon.load("view.icn");
	_cursorCell = 0;
	showCursor(true);
	delayFrames(CURSOR_BLINK_FRAMES);

	return PartyView::msgFocus(msg);
}

bool CharacterInfo::msgUnfocus(const UnfocusMessage &msg) {
	_viewIcon.clear();
	return PartyView::msgUnfocus(msg);
}

bool CharacterInfo::msgKeypress(const KeypressMessage &msg) {
	int idx;

	switch (msg.keycode) {
	case Common::KEYCODE_UP:
		showCursor(false);
		if (--_cursorCell < 0)
			_cursorCell = ICONS_COUNT - 1;
		showCursor(true);
		break;
	case Common::KEYCODE_DOWN:
		showCursor(false);
		if (++_cursorCell >= ICONS_COUNT)
			_cursorCell = 0;
		showCursor(true);
		break;
	case Common::KEYCODE_LEFT:
		showCursor(false);
		idx = REDUCED_TO_EXPANDED(_cursorCell) - 5;
		if (idx == 13 || idx == 14)
			idx -= 5;
		if (idx < 0)
			idx += 20;
		_cursorCell = EXPANDED_TO_REDUCED(idx);
		showCursor(true);
		break;
	case Common::KEYCODE_RIGHT:
		showCursor(false);
		idx = REDUCED_TO_EXPANDED(_cursorCell) + 5;
		if (idx == 13 || idx == 14)
			idx += 5;
		if (idx >= 20)
			idx -= 20;
		_cursorCell = EXPANDED_TO_REDUCED(idx);
		showCursor(true);
		break;
	case Common::KEYCODE_i:
		addView("CharacterInventory");
		break;
	case Common::KEYCODE_e:
		addView("Exchange");
		break;
	case Common::KEYCODE_q:
		addView("QuickRef");
		break;
	default:
		break;
	}

	return true;
}

bool CharacterInfo::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		close();
		return true;

	case KEYBIND_SELECT:
		showAttribute(_cursorCell);
		return true;

	default:
		return PartyView::msgAction(msg);
	}
}

bool CharacterInfo::msgMouseUp(const MouseUpMessage &msg) {
	// Check if a stat icon was clicked
	Common::Rect r(25, 22);
	for (int i = 0; i < CHAR_ICONS_COUNT; ++i) {
		r.moveTo(_innerBounds.left + ICONS[i]._x,
			_innerBounds.top + ICONS[i]._y);
		if (r.contains(msg._pos)) {
			switch (i) {
			case 18:
				msgKeypress(Common::KeyState(Common::KEYCODE_i));
				break;
			case 19:
				msgKeypress(Common::KeyState(Common::KEYCODE_q));
				break;
			case 20:
				msgKeypress(Common::KeyState(Common::KEYCODE_e));
				break;
			case 21:
				msgAction(ActionMessage(KEYBIND_ESCAPE));
				break;
			default:
				showAttribute(i);
				break;
			}

			return true;
		}
	}

	return PartyView::msgMouseUp(msg);
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
		"%s : %s %s %s %s",
		camelCase(c._name).c_str(),
		STRING[Common::String::format("stats.sex.%d", (int)c._sex)].c_str(),
		STRING[Common::String::format("stats.alignments.%d", (int)c._alignment)].c_str(),
		STRING[Common::String::format("stats.races.%d", (int)c._race)].c_str(),
		STRING[Common::String::format("stats.classes.%d", (int)c._class)].c_str()
	);

	writeString(0, 0, msg);
}

void CharacterInfo::drawIcons() {
	Graphics::ManagedSurface s = getSurface();
	for (int i = 0; i < CHAR_ICONS_COUNT; ++i) {
		_viewIcon.draw(&s, ICONS[i]._frame,
			Common::Point(ICONS[i]._x + _bounds.borderSize(),
				ICONS[i]._y + _bounds.borderSize()));
	}

	// Text for buttons
	writeString(277, 25, STRING["enhdialogs.character.item"]);
	writeString(273, 57, STRING["enhdialogs.character.quick"]);
	writeString(276, 90, STRING["enhdialogs.character.exchange"]);
	writeString(278, 122, STRING["enhdialogs.misc.exit"]);
}

void CharacterInfo::drawStats() {
	// Draw stat titles
	for (int i = 0; i < 18; ++i) {
		writeString(ICONS[i]._x + 27, ICONS[i]._y + 2, ICONS_TEXT[i]);
	}

	// Draw stat values
	const Character &c = *g_globals->_currCharacter;
	const uint CURR[16] = {
		c._might._current, c._intelligence._current,
		c._personality._current, c._endurance._current,
		c._speed._current, c._accuracy._current,
		c._luck._current, c._age._base, c._level._current,
		c._ac._current, c._hp, c._sp._current, 0,
		c._exp, c._gold, c._gems
	};
	const uint BASE[16] = {
		c._might._base, c._intelligence._base,
		c._personality._base, c._endurance._base,
		c._speed._base, c._accuracy._base,
		c._luck._base, c._age._base, c._level._base,
		c._ac._current, c._hp, c._sp._base, 0,
		c._exp, c._gold, c._gems
	};

	for (int i = 0; i < 17; ++i) {
		if (i == 12)
			continue;

		Common::Point pt(ICONS[i]._x + 27, ICONS[i]._y + 12);
		if (i < 10)
			pt.x += 8 + (CURR[i] < 10 ? 8 : 0);

		setTextColor(c.statColor(CURR[i], BASE[i]));

		if (i == 16) {
			// Food
			Common::String str = Common::String::format("%d %s",
				c._food,
				STRING[c._food == 1 ? "enhdialogs.character.stats.day" :
					"enhdialogs.character.stats.days"].c_str());
			setTextColor(15);
			writeString(pt.x, pt.y, str);
		} else {
			writeNumber(pt.x, pt.y, CURR[i]);
		}
	}

	// Condition string
	Common::String condStr = camelCase(c.getConditionString());
	setTextColor(!c._condition ? 15 : 19);

	uint i = condStr.findFirstOf(',');
	if (i != Common::String::npos) {
		// More than one condition
		condStr = Common::String(condStr.c_str(), i);
		setTextColor(32);
	}

	writeString(196, 120, condStr);
}

void CharacterInfo::showCursor(bool flag) {
	const int CURSOR_X[4] = { 9, 60, 111, 176 };
	const int CURSOR_Y[5] = { 23, 46, 69, 92, 115 };

	if (flag == _cursorVisible)
		return;

	int idx = REDUCED_TO_EXPANDED(_cursorCell);
	_cursorVisible = flag;

	Graphics::ManagedSurface s = getSurface();
	_viewIcon.draw(&s, flag ? 49 : 48,
		Common::Point(CURSOR_X[idx / 5], CURSOR_Y[idx % 5]));
	s.markAllDirty();
}

void CharacterInfo::timeout() {
	showCursor(!_cursorVisible);
	delayFrames(CURSOR_BLINK_FRAMES);
}

void CharacterInfo::showAttribute(int attrNum) {
	// Switch the cursor to the selected attribute
	showCursor(false);
	_cursorCell = attrNum;
	showCursor(true);

	getSurface().markAllDirty();
	g_events->getScreen()->update();

	// Display the info dialog
	const int STAT_POS[2][20] = {
		{
			61, 61, 61, 61, 61, 112, 112, 112, 112, 112,
			177, 177, 177, 177, 177, 34, 34, 34, 34, 34
		}, {
			24, 47, 70, 93, 93, 24, 47, 70, 93, 93,
			24, 47, 70, 93, 93, 24, 47, 70, 93, 93
		}
	};

	int attrib = REDUCED_TO_EXPANDED(attrNum);
	assert(attrib < 20);

	Common::Rect bounds(STAT_POS[0][attrib], STAT_POS[1][attrib],
		STAT_POS[0][attrib] + 143, STAT_POS[1][attrib] + 44);
	_statInfo.setBounds(bounds);
	_statInfo.clear();

	const Character &c = *g_globals->_currCharacter;
	const uint CURR[12] = {
		c._might._current, c._intelligence._current,
		c._personality._current, c._endurance._current,
		c._speed._current, c._accuracy._current,
		c._luck._current, c._age._base, c._level._current,
		c._ac._current, c._hp, c._sp._current
	};
	const uint BASE[12] = {
		c._might._base, c._intelligence._base,
		c._personality._base, c._endurance._base,
		c._speed._base, c._accuracy._base,
		c._luck._base, c._age._base, c._level._base,
		c._ac._current, c._hp, c._sp._base
	};
	const char *TITLES[12] = {
		"might", "intelligence", "personality",
		"endurance", "speed", "accuracy", "luck",
		"age", "level", "ac", "hp", "sp"
	};

	if (attrib < 12) {
		_statInfo.addLine(
			STRING[Common::String::format("enhdialogs.character.long.%s",
				TITLES[attrib]).c_str()],
			ALIGN_MIDDLE);

		int xc = (bounds.width() - FRAME_BORDER_SIZE * 2) / 2;
		_statInfo.addText(STRING["enhdialogs.character.long.current"],
			1, 0, ALIGN_RIGHT, xc - 8);
		_statInfo.addText("/", 1, 0, ALIGN_MIDDLE);
		_statInfo.addText(STRING["enhdialogs.character.long.base"],
			1, 0, ALIGN_LEFT, xc + 8);

		_statInfo.addText(Common::String::format("%u", CURR[attrib]),
			2, 0, ALIGN_RIGHT, xc - 8);
		_statInfo.addText("/", 2, 0, ALIGN_MIDDLE);
		_statInfo.addText(Common::String::format("%u", BASE[attrib]),
			2, 0, ALIGN_LEFT, xc + 8);
	} else if (attrib >= 16 && attrib <= 18) {
		bounds.bottom -= 8;
		_statInfo.setBounds(bounds);

		uint value = c._gold;
		Common::String title = STRING["enhdialogs.character.stats.gold"];
		if (attrib == 17) {
			value = c._gems;
			title = STRING["enhdialogs.character.stats.gems"];
		} else if (attrib == 18) {
			value = c._food;
			title = STRING["enhdialogs.character.stats.food"];
		}

		_statInfo.addLine(title, ALIGN_MIDDLE);
		_statInfo.addLine(Common::String::format("%u %s",
			value, STRING["enhdialogs.character.long.on_hand"].c_str()),
			ALIGN_MIDDLE);
	}

	/*
	case 15:
		// Experience
		stat1 = c.getCurrentExperience();
		stat2 = c.experienceToNextLevel();
		msg = Common::String::format(Res.EXPERIENCE_TEXT,
			Res.STAT_NAMES[attrib], stat1,
			stat2 == 0 ? Res.ELIGIBLE : Common::String::format("%d", stat2).c_str()
		);
		bounds.setHeight(43);
		break;

	case 19:
	{
		// Conditions
		Common::String lines[20];
		const char **_tmpConditions = c._sex == FEMALE ? (const char **)Res.CONDITION_NAMES_F : (const char **)Res.CONDITION_NAMES_M;
		int total = 0;
		for (int condition = CURSED; condition <= ERADICATED; ++condition) {
			if (c._conditions[condition]) {
				if (condition >= UNCONSCIOUS) {
					lines[condition] = Common::String::format("\n\t020%s",
						_tmpConditions[condition]);
				} else {
					lines[condition] = Common::String::format("\n\t020%s\t095-%d",
						_tmpConditions[condition], c._conditions[condition]);
				}

				++total;
			}
		}

		Condition condition = c.worstCondition();
		if (condition == NO_CONDITION) {
			lines[0] = Common::String::format("\n\t020%s", Res.GOOD);
			++total;
		}

		if (party._blessed) {
			lines[16] = Common::String::format(Res.BLESSED, party._blessed);
			++total;
		}
		if (party._powerShield) {
			lines[17] = Common::String::format(Res.POWER_SHIELD, party._powerShield);
			++total;
		}
		if (party._holyBonus) {
			lines[18] = Common::String::format(Res.HOLY_BONUS, party._holyBonus);
			++total;
		}
		if (party._heroism) {
			lines[19] = Common::String::format(Res.HEROISM, party._heroism);
			++total;
		}

		msg = Common::String::format("\x2\x3""c%s\x3l%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\x1",
			Res.CONSUMABLE_NAMES[3], lines[0].c_str(), lines[1].c_str(),
			lines[2].c_str(), lines[3].c_str(), lines[4].c_str(),
			lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
			lines[8].c_str(), lines[9].c_str(), lines[10].c_str(),
			lines[11].c_str(), lines[12].c_str(), lines[13].c_str(),
			lines[14].c_str(), lines[15].c_str(), lines[16].c_str(),
			lines[17].c_str(), lines[18].c_str(), lines[19].c_str()
		);

		bounds.top -= ((total - 1) / 2) * 8;
		bounds.setHeight(total * 9 + 26);
		if (bounds.bottom >= SCREEN_HEIGHT)
			bounds.moveTo(bounds.left, SCREEN_HEIGHT - bounds.height() - 1);
		break;
	}

	default:
		break;
	}
	*/

	_statInfo.addView();
}

} // namespace Views
} // namespace MM1
} // namespace MM
