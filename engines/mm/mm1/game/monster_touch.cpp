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

#include "mm/mm1/game/monster_touch.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Game {

const MonsterTouchAction MonsterTouch::ACTIONS[25] = {
	&MonsterTouch::action00,
	&MonsterTouch::action01,
	&MonsterTouch::action02,
	&MonsterTouch::action03,
	&MonsterTouch::action04,
	&MonsterTouch::action05,
	&MonsterTouch::action06,
	&MonsterTouch::action07,
	&MonsterTouch::action08,
	&MonsterTouch::action09,
	&MonsterTouch::action10,
	&MonsterTouch::action11,
	&MonsterTouch::action12,
	&MonsterTouch::action13,
	&MonsterTouch::action14,
	&MonsterTouch::action15,
	&MonsterTouch::action16,
	&MonsterTouch::action17,
	&MonsterTouch::action18,
	&MonsterTouch::action19,
	&MonsterTouch::action20,
	&MonsterTouch::action21,
	&MonsterTouch::action22,
	&MonsterTouch::action23,
	&MonsterTouch::action24
};

static Common::String formatMonsterAction(const Common::String &action) {
	return Common::String::format("    and %s!", action.c_str());
}

#define LINE(ACTION) line = formatMonsterAction(ACTION)

bool MonsterTouch::monsterTouch(uint index, Common::String &line) {
	line = "";

	// Touch effect IDs are 1-based
	if (index == 0 || index > ARRAYSIZE(ACTIONS))
		return false;

	return (this->*ACTIONS[index - 1])(line);
}

bool MonsterTouch::canPerform(int level) const {
	return getRandomNumber(level) == level;
}

bool MonsterTouch::action00(Common::String &line) {
	if (canPerform(3)) {
		g_globals->_currCharacter->_food = 0;
		LINE("takes food");
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action01(Common::String &line) {
	if (canPerform(20)) {
		setCondition(DISEASED);
		LINE("inflicts disease");
		return true;
	}

	return true;
}

bool MonsterTouch::action02(Common::String &line) {
	if (canPerform(20)) {
		return action07(line);

	} else {
		return false;
	}
}

bool MonsterTouch::action03(Common::String &line) {
	if (canPerform(20)) {
		return action11(line);

	} else {
		return false;
	}
}

bool MonsterTouch::action04(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2) && c._gems != 0) {
		c._gems /= 2;
		LINE("steals some gems");
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action05(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (c._endurance._current == 0 || --c._endurance._current == 0)
		setCondition(BAD_CONDITION | DEAD);
	_damage += 3;

	LINE("reduces endurance");
	return true;
}

bool MonsterTouch::action06(Common::String &line) {
	if (damageType7()) {
		setCondition(ASLEEP);
		LINE("induces sleep");
		return true;
	}

	return false;
}

bool MonsterTouch::action07(Common::String &line) {
	if (damageType5()) {
		setCondition(3);
		LINE("causes paralysis");
		return true;
	}

	return false;
}

bool MonsterTouch::action08(Common::String &line) {
	if (canPerform(4)) {
		setCondition(DISEASED);
		LINE("inflicts disease");
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action09(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2) && c._gold != 0) {
		c._gold /= 2;
		LINE("steals some gold");
		return true;

	} else {
		return false;
	}

	return true;
}

bool MonsterTouch::action10(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2) && !c._backpack.empty()) {
		c._backpack.removeAt(c._backpack.size() - 1);
		LINE("steals something");
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action11(Common::String &line) {
	if (damageType6()) {
		setCondition(16);
		LINE("induces poison");
		return true;
	}

	return false;
}

bool MonsterTouch::action12(Common::String &line) {
	if (canPerform(3)) {
		setCondition(BLINDED);
		LINE("causes blindness");
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action13(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (c._level._current == 0 || --c._level._current == 0)
		setCondition(BAD_CONDITION | DEAD);

	_damage += 10;
	LINE("drains lifeforce");
	return true;
}

bool MonsterTouch::action14(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(3) && isCharAffected()) {
		setCondition(BAD_CONDITION | STONE);
		line = Common::String::format("%s is turned to stone",
			c._name);
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action15(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (isCharAffected()) {
		int age = c._age + 10;
		c._age = age;

		if (age > 255) {
			c._age = 200;
			setCondition(ERADICATED);
		}

		LINE("causes rapid aging");
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action16(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (c._level < 3) {
		c._level._current = 0;
		setCondition(BAD_CONDITION | DEAD);

	} else {
		c._level._current -= 2;
	}

	_damage += 20;
	LINE("drains lifeforce");
	return true;
}

bool MonsterTouch::action17(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(3) && isCharAffected()) {
		setCondition(BAD_CONDITION | DEAD);
		line = Common::String::format("%s is killed",
			c._name);
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action18(Common::String &line) {
	if (canPerform(3) && isCharAffected()) {
		setCondition(UNCONSCIOUS);
		LINE("induces unconsciousness");
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action19(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (c._might._current < 4) {
		c._might._current = 0;
		setCondition(BAD_CONDITION | DEAD);

	} else {
		c._might._current -= 3;
	}

	LINE("drains might");
	return true;
}

bool MonsterTouch::action20(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	for (int i = 0; i < 7; ++i) {
		AttributePair &attr = c.getAttribute(i);

		if (attr._current < 3) {
			attr._current = 0;
			setCondition(BAD_CONDITION | DEAD);

		} else {
			attr._current -= 2;
		}
	}

	LINE("drains ability");
	return true;
}

bool MonsterTouch::action21(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2)) {
		c._backpack.clear();
		line = formatMonsterAction("steals backpack");
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action22(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2)) {
		c._gold = 0;
		c._gems = 0;

		LINE("steals gold and gems");
		return true;

	} else {
		return false;
	}
}

bool MonsterTouch::action23(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	setCondition(ERADICATED);
	line = Common::String::format("%s is eradicated",
		c._name);

	return true;
}

bool MonsterTouch::action24(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	c._sp._current = 0;
	LINE("drains spell points");
	return true;
}

} // namespace Game
} // namespace MM1
} // namespace MM
