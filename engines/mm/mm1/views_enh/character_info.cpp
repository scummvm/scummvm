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
#include "mm/mm1/data/text.h"
#include "mm/mm1/views_enh/scroll_popup.h"
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

bool CharacterInfo::AttributeView::msgFocus(const FocusMessage &msg) {
	ScrollPopup::msgFocus(msg);
	g_events->send("GameParty", GameMessage("CHAR_HIGHLIGHT", (int)true));
	return true;
}

CharacterInfo::CharacterInfo() : PartyView("CharacterInfo") {
	_bounds = Common::Rect(0, 0, 320, 146);
	_statInfo.setReduced(true);

	static const char *const FIELDS[CHAR_ICONS_COUNT] = {
		"Mgt", "Int", "Per",
		"End", "Spd",
		"Acy", "Luc", "Age", "Lvl", "AC",
		"H.P.", "S.P.", "Spells",
		"Experience", "Gold", "Gems", "Food", "Condition"
	};

	for (int i = 0; i < ICONS_COUNT; ++i) {
		ICONS_TEXT[i] = FIELDS[i];
	}
}

bool CharacterInfo::msgFocus(const FocusMessage &msg) {
	_viewIcon.load("view.icn");

	// Don't reset selection after having viewed an attribute
	if (dynamic_cast<ScrollPopup *>(msg._priorView) == nullptr)
		_cursorCell = 0;

	showCursor(true);
	delayFrames(CURSOR_BLINK_FRAMES);
	return PartyView::msgFocus(msg);
}

bool CharacterInfo::msgUnfocus(const UnfocusMessage &msg) {
	_viewIcon.clear();
	return PartyView::msgUnfocus(msg);
}

bool CharacterInfo::msgGame(const GameMessage &msg) {
	if (msg._name == "USE") {
		g_events->send("CharacterInventory", GameMessage("USE"));
		return true;
	}

	return false;
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
		replaceView("QuickRef");
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
	if (msg._button == MouseMessage::MB_RIGHT)
		return true;

	if (msg._button != MouseMessage::MB_LEFT)
		return PartyView::msgMouseUp(msg);

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
	const char *sex = (c._sex == MALE) ? "M" : (c._sex == FEMALE ? "F" : "O");
	Common::String suffix = Common::String::format(
		" : %s %s %s %s",
		sex,
		ALIGNMENT_NAMES[(int)c._alignment - 1],
		RACE_NAMES[(int)c._race - 1],
		CLASS_NAMES[(int)c._class - 1]
	);
	Common::String name = truncateString(camelCase(c._name),
		269 - (int)getStringWidth(suffix));
	Common::String msg = Common::String::format(
		"%s%s",
		name.c_str(),
		suffix.c_str()
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
	writeString(277, 25, "\x01""37Item");
	writeString(273, 57, "\x01""37Quick");
	writeString(276, 90, "\x01""37Exch");
	writeString(278, 122, "Exit");
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
		c._luck._current, c._age, c._level._current,
		c._ac._current, c._hpCurrent, c._sp._current, 0,
		c._exp, c._gold, c._gems
	};
	const uint BASE[16] = {
		c._might._base, c._intelligence._base,
		c._personality._base, c._endurance._base,
		c._speed._base, c._accuracy._base,
		c._luck._base, c._age, c._level._base,
		c._ac._current, c._hp, c._sp._base, 0,
		c._exp, c._gold, c._gems
	};

	for (int i = 0; i < 17; ++i) {
		if (i == 12)
			continue;

		Common::Point pt(ICONS[i]._x + 27, ICONS[i]._y + 12);
		if (i < 10)
			pt.x += 8 + (CURR[i] < 10 ? 8 : 0);


		if (i == 16) {
			// Food
			Common::String str = Common::String::format("%d %s",
				c._food,
				c._food == 1 ? "Day" : "Days");
			setTextColor(15);
			writeString(pt.x, pt.y, str);
		} else {
			setTextColor(c.statColor(CURR[i], BASE[i]));
			writeNumber(pt.x, pt.y, CURR[i]);

			// LEVEL-UP-READY: Experience is derived from current XP/level.
			// Rotate the Xeen right-arrow glyph upward immediately after XP.
			// Its color is exactly the color used for SP == 0.
			if (i == 13 && c.canLevelUp()) {
				// LEVEL-UP-ARROW-ROTATED-V3: rotate only this glyph, not the font.
				XeenFont &font = _fontReduced ? g_globals->_fontReduced : g_globals->_fontNormal;
				XeenFont::setColors(c.statColor(0, c._sp._base));
				Graphics::ManagedSurface glyph, mask;
				const int gw = font.getMaxCharWidth() + 2;
				const int gh = font.getFontHeight() + 2;
				glyph.create(gw, gh, Graphics::PixelFormat::createFormatCLUT8());
				mask.create(gw, gh, Graphics::PixelFormat::createFormatCLUT8());
				// Two backgrounds distinguish untouched pixels from ALL palette colors.
				glyph.clear(0);
				mask.clear(255);
				font.drawChar(&glyph, '^', 1, 1, 9);
				font.drawChar(&mask, '^', 1, 1, 9);
				int left = gw, top = gh, right = -1, bottom = -1;
				for (int y = 0; y < gh; ++y) {
					for (int x = 0; x < gw; ++x) {
						if (*(const byte *)glyph.getBasePtr(x, y) == *(const byte *)mask.getBasePtr(x, y)) {
							if (x < left) left = x;
							if (x > right) right = x;
							if (y < top) top = y;
							if (y > bottom) bottom = y;
						}
					}
				}
				if (right >= left && bottom >= top) {
					Graphics::ManagedSurface s = getSurface();
					const int destX = _bounds.borderSize() + _textPos.x + 2;
					const int destY = _bounds.borderSize() + _textPos.y +
						(font.getFontHeight() - (right - left + 1)) / 2;
					for (int y = top; y <= bottom; ++y) {
						for (int x = left; x <= right; ++x) {
							const byte pixel = *(const byte *)glyph.getBasePtr(x, y);
							if (pixel != *(const byte *)mask.getBasePtr(x, y)) continue;
							// Counterclockwise: rightmost source column becomes top row.
							const int dx = destX + y - top;
							const int dy = destY + right - x;
							if (dx >= 0 && dx < s.w && dy >= 0 && dy < s.h)
								s.fillRect(Common::Rect(dx, dy, dx + 1, dy + 1), pixel);
						}
					}
				}
				XeenFont::setColors(_colorsNum);
			}
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

void CharacterInfo::charSwitched(Character *priorChar) {
	PartyView::charSwitched(priorChar);

	_cursorCell = 0;
	redraw();
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

	if (attrib == 12) {
		// Show active party spells
		g_events->addView("Protect");
		return;
	}

	Common::Rect bounds(STAT_POS[0][attrib], STAT_POS[1][attrib],
		STAT_POS[0][attrib] + 143, STAT_POS[1][attrib] + 44);
	_statInfo.setBounds(bounds);
	_statInfo.clear();

	const Character &c = *g_globals->_currCharacter;
	const uint CURR[12] = {
		c._might._current, c._intelligence._current,
		c._personality._current, c._endurance._current,
		c._speed._current, c._accuracy._current,
		c._luck._current, c._age, c._level._current,
		c._ac._current, c._hpCurrent, c._sp._current
	};
	const uint BASE[12] = {
		c._might._base, c._intelligence._base,
		c._personality._base, c._endurance._base,
		c._speed._base, c._accuracy._base,
		c._luck._base, c._age, c._level._base,
		c._ac._current, c._hp, c._sp._base
	};
	const char *const TITLES[12] = {
		"Might", "Intelligence", "Personality", "Endurance",
		"Speed", "Accuracy", "Luck", "Age",
		"Level", "Armor Class", "Hit Points", "Spell Points"
	};

	if (attrib < 12) {
		_statInfo.addLine(
			TITLES[attrib],
			ALIGN_MIDDLE);

		int xc = (bounds.width() - FRAME_BORDER_SIZE * 2) / 2;
		_statInfo.addText("Current",
			1, 0, ALIGN_RIGHT, xc - 8);
		_statInfo.addText("/", 1, 0, ALIGN_MIDDLE);
		_statInfo.addText("Base",
			1, 0, ALIGN_LEFT, xc + 8);

		_statInfo.addText(Common::String::format("%u", CURR[attrib]),
			2, 0, ALIGN_RIGHT, xc - 8);
		_statInfo.addText("/", 2, 0, ALIGN_MIDDLE);
		_statInfo.addText(Common::String::format("%u", BASE[attrib]),
			2, 0, ALIGN_LEFT, xc + 8);

	} else if (attrib == 15) {
		// Experience
		_statInfo.addLine("Experience", ALIGN_MIDDLE);
		_statInfo.addLine(Common::String::format("%u", c._exp), ALIGN_MIDDLE);

	} else if (attrib >= 16 && attrib <= 18) {
		bounds.bottom -= 8;
		_statInfo.setBounds(bounds);

		uint value = c._gold;
		Common::String title = "Gold";
		if (attrib == 17) {
			value = c._gems;
			title = "Gems";
		} else if (attrib == 18) {
			value = c._food;
			title = "Food";
		}

		_statInfo.addLine(title, ALIGN_MIDDLE);
		_statInfo.addLine(Common::String::format("%u On Hand",
			value),
			ALIGN_MIDDLE);
	} else {
		// Condition
		Common::String conditionStr = c.getConditionString();

		_statInfo.addLine("Condition", ALIGN_MIDDLE);
		_statInfo.addLine(conditionStr, ALIGN_MIDDLE);
	}

	_statInfo.addView();
}

} // namespace Views
} // namespace MM1
} // namespace MM
