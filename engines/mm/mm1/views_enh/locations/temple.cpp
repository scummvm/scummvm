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

#include "mm/mm1/views_enh/locations/temple.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/shared/utils/strings.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

static const int BTN_SIZE = 10;
static const int OPTION_X = 0;
static const int OPTION_Y = 45;
static const int OPTION_SPACING = 11;
static const int OPTION_TEXT_X = OPTION_X + BTN_SIZE;

static Common::Rect getOptionButtonRect(int row) {
	const int y = OPTION_Y + OPTION_SPACING * row;
	return Common::Rect(OPTION_X, y, OPTION_X + BTN_SIZE, y + BTN_SIZE);
}

static Common::String getOptionText(const Common::String &text) {
	const char *s = text.c_str();

	// Option strings highlight their hotkey as "\x01NNx".
	if (*s == '\x01' && s[1] && s[2] && s[3])
		return Common::String(s + 4);
	if (*s)
		return Common::String(s + 1);

	return text;
}

Temple::Temple() : Location("Temple", LOC_TEMPLE) {
	addButton(getOptionButtonRect(0), Common::KEYCODE_h);
	addButton(getOptionButtonRect(1), Common::KEYCODE_u);
	addButton(getOptionButtonRect(2), Common::KEYCODE_r);
	addButton(getOptionButtonRect(3), Common::KEYCODE_d);
	addButton(&_escSprite, Common::Point(24, 100), 0, KEYBIND_ESCAPE);
}

bool Temple::msgFocus(const FocusMessage &msg) {
	Location::msgFocus(msg);
	updateCosts();

	return true;
}

void Temple::draw() {
	Location::draw();

	setReduced(false);
	writeLine(0, STRING["enhdialogs.temple.title"], ALIGN_MIDDLE);
	writeLine(1, STRING["enhdialogs.location.options_for"], ALIGN_MIDDLE);
	writeString(0, 23, camelCase(g_globals->_currCharacter->_name), ALIGN_MIDDLE);

	setReduced(true);
	writeOption(0, 'H', STRING["enhdialogs.temple.heal"],
		_healCost ? Common::String::format("%d", _healCost) : "----");
	writeOption(1, 'U', STRING["enhdialogs.temple.uncurse"],
		_uncurseCost ? Common::String::format("%d", _uncurseCost) : "----");
	writeOption(2, 'R', STRING["enhdialogs.temple.realign"],
		_alignmentCost ? Common::String::format("%d", _alignmentCost) : "----");
	writeOption(3, 'D', STRING["enhdialogs.temple.donate"],
		Common::String::format("%d", _donateCost));

	setReduced(false);
	writeLine(10, STRING["enhdialogs.location.gold"]);
	writeLine(10, Common::String::format("%d",
		g_globals->_currCharacter->_gold), ALIGN_RIGHT);

	setReduced(true);
	writeString(27, 122, STRING["enhdialogs.location.esc"]);
}

void Temple::writeOption(int row, char c, const Common::String &text,
		const Common::String &cost) {
	const int y = OPTION_Y + OPTION_SPACING * row;
	const int textY = y + (BTN_SIZE - 8) / 2 + 1;

	Graphics::ManagedSurface btnSmall(BTN_SIZE, BTN_SIZE);
	btnSmall.blitFrom(g_globals->_blankButton, Common::Rect(0, 0, 20, 20),
		Common::Rect(0, 0, BTN_SIZE, BTN_SIZE));

	Graphics::ManagedSurface s = getSurface();
	s.blitFrom(btnSmall, Common::Point(OPTION_X + _bounds.borderSize(),
		y + _bounds.borderSize()));

	writeString(OPTION_X + (BTN_SIZE / 2) + 1, textY,
		Common::String::format("%c", c), ALIGN_MIDDLE);
	writeString(OPTION_TEXT_X, textY, getOptionText(text));
	writeString(0, textY, cost, ALIGN_RIGHT);
}

bool Temple::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_h:
		restoreHealth();
		break;
	case Common::KEYCODE_u:
		uncurseItems();
		break;
	case Common::KEYCODE_r:
		restoreAlignment();
		break;
	case Common::KEYCODE_d:
		donate();
		break;
	case Common::KEYCODE_g:
		g_globals->_currCharacter->gatherGold();
		redraw();
		break;
	default:
		return Location::msgKeypress(msg);
	}

	return true;
}

bool Temple::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		leave();
		return true;
	default:
		return Location::msgAction(msg);
	}
}

bool Temple::msgGame(const GameMessage &msg) {
	Location::msgGame(msg);
	if (msg._name == "UPDATE")
		updateCosts();
	return true;
}

void Temple::updateCosts() {
	Maps::Map &map = *g_maps->_currentMap;
	int i;
	_isEradicated = false;

	int townNum = map[Maps::MAP_ID];
	if (townNum < 1 || townNum >= 6)
		townNum = 1;
	--townNum;

	Character &c = *g_globals->_currCharacter;

	_healCost = 0;
	if (c._condition == ERADICATED) {
		_healCost = HEAL_COST1[townNum];
		_isEradicated = true;
	} else if (c._condition & BAD_CONDITION) {
		_healCost = HEAL_COST2[townNum];
	} else if (c._condition || c._hp < c._hpMax) {
		_healCost = HEAL_COST3[townNum];
	}

	_uncurseCost = UNCURSE_COST[townNum];
	for (i = 0; i < INVENTORY_COUNT; ++i) {
		if (c._equipped[i]) {
			if (g_globals->_items.getItem(c._equipped[i])->_constBonus_id == EQUIP_CURSED)
				break;
		}
	}
	if (i == INVENTORY_COUNT)
		_uncurseCost = 0;

	_alignmentCost = ALIGNMENT_COST[townNum];
	if (c._alignment == c._alignmentInitial)
		_alignmentCost = 0;

	_donateCost = DONATE_COST[townNum];
}

void Temple::restoreHealth() {
	if (subtractGold(_healCost)) {
		Character &c = *g_globals->_currCharacter;
		c._condition = FINE;
		c._hpCurrent = c._hp;

		if (_isEradicated) {
			c._age += 10;
			--c._endurance;
		}

		redraw();
	}
}

void Temple::uncurseItems() {
	if (subtractGold(_uncurseCost)) {
		Character &c = *g_globals->_currCharacter;
		for (int i = 0; i < INVENTORY_COUNT; ++i) {
			if (c._equipped[i]) {
				g_globals->_items.getItem(c._equipped[i]);
				if (g_globals->_currItem._constBonus_id == EQUIP_CURSED) {
					c._equipped.removeAt(i);
					--i;
				}
			}
		}

		redraw();
	}
}

void Temple::restoreAlignment() {
	if (subtractGold(_alignmentCost)) {
		Character &c = *g_globals->_currCharacter;
		c._alignment = c._alignmentInitial;
		c._alignmentCtr = ALIGNMENT_VALS[c._alignment];

		redraw();
	}
}

void Temple::donate() {
	if (subtractGold(_donateCost)) {
		Maps::Map &map = *g_maps->_currentMap;
		Character &c = *g_globals->_currCharacter;

		int townNum = map[Maps::MAP_ID];
		if (townNum < 1 || townNum >= 6)
			townNum = 1;
		--townNum;

		c._worthiness |= DONATE_VALS[townNum];
		draw();

		if (g_engine->getRandomNumber(15) == 10) {
			for (int i = 0; i < 13; ++i)
				g_globals->_activeSpells._arr[i] = 75;

			Sound::sound(SOUND_3);
			displayMessage(STRING["dialogs.temple.protected"]);
		} else {
			displayMessage(STRING["dialogs.temple.thankyou"]);
		}
	}
}

} // namespace Location
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
