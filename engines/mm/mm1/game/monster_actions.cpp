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

#include "mm/mm1/game/monster_actions.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Game {

const MonsterActionsAction MonsterActions::ACTIONS[25] = {
	&MonsterActions::action00,
	&MonsterActions::action01,
	&MonsterActions::action02,
	&MonsterActions::action03,
	&MonsterActions::action04,
	&MonsterActions::action05,
	&MonsterActions::action06,
	&MonsterActions::action07,
	&MonsterActions::action08,
	&MonsterActions::action09,
	&MonsterActions::action10,
	&MonsterActions::action11,
	&MonsterActions::action12,
	&MonsterActions::action13,
	&MonsterActions::action14,
	&MonsterActions::action15,
	&MonsterActions::action16,
	&MonsterActions::action17,
	&MonsterActions::action18,
	&MonsterActions::action19,
	&MonsterActions::action20,
	&MonsterActions::action21,
	&MonsterActions::action22,
	&MonsterActions::action23,
	&MonsterActions::action24
};

#define LINE(ACTION) line = Common::String::format("    %s%s", \
	STRING["monster_actions.and"].c_str(), STRING[ACTION].c_str())

bool MonsterActions::monsterAction(uint index, Common::String &line) {
	line = "";
	return (this->*ACTIONS[index])(line);
}

bool MonsterActions::canPerform(int level) const {
	return getRandomNumber(level) == level;
}

bool MonsterActions::action00(Common::String &line) {
	if (canPerform(3)) {
		g_globals->_currCharacter->_food = 0;
		LINE("monster_actions.takes_food");
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action01(Common::String &line) {
	if (canPerform(20)) {
		setCondition(DISEASED);
		LINE("monster_actions.inflicts_disease");
		return true;
	}

	return true;
}

bool MonsterActions::action02(Common::String &line) {
	if (canPerform(20)) {
		return action07(line);

	} else {
		return false;
	}
}

bool MonsterActions::action03(Common::String &line) {
	if (canPerform(20)) {
		return action11(line);

	} else {
		return false;
	}
}

bool MonsterActions::action04(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2) && c._gems != 0) {
		c._gems /= 2;
		LINE("monster_actions.steals_gems");
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action05(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (c._endurance._current == 0 || --c._endurance._current == 0)
		setCondition(BAD_CONDITION | DEAD);
	_damage += 3;

	LINE("monster_actions.reduces_endurance");
	return true;
}

bool MonsterActions::action06(Common::String &line) {
	if (damageType7()) {
		setCondition(ASLEEP);
		LINE("monster_actions.induces_sleep");
		return true;
	}

	return false;
}

bool MonsterActions::action07(Common::String &line) {
	if (damageType5()) {
		setCondition(3);
		LINE("monster_actions.cures_paralysis");
		return true;
	}

	return false;
}

bool MonsterActions::action08(Common::String &line) {
	if (canPerform(4)) {
		setCondition(DISEASED);
		LINE("monster_actions.inflicts_disease");
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action09(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2) && c._gold != 0) {
		c._gold /= 2;
		LINE("monster_actions.steals_gold");
		return true;

	} else {
		return false;
	}

	return true;
}

bool MonsterActions::action10(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2) && !c._backpack.empty()) {
		c._backpack.removeAt(c._backpack.size() - 1);
		LINE("monster_actions.steals_something");
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action11(Common::String &line) {
	if (damageType6()) {
		setCondition(16);
		LINE("monster_actions.induces_poison");
		return true;
	}

	return false;
}

bool MonsterActions::action12(Common::String &line) {
	if (canPerform(3)) {
		setCondition(BLINDED);
		LINE("monster_actions.causes_blindness");
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action13(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (c._level._current == 0 || --c._level._current == 0)
		setCondition(BAD_CONDITION | DEAD);

	_damage += 10;
	LINE("monster_actions.drains_lifeforce");
	return true;
}

bool MonsterActions::action14(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(3) && isCharAffected()) {
		setCondition(BAD_CONDITION | STONE);
		line = Common::String::format("%s %s",
			c.getDisplayName().c_str(),
			STRING["monster_actions.turned_to_stone"].c_str()
		);
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action15(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (isCharAffected()) {
		int age = c._age._base + 10;
		c._age._base = age;

		if (age > 255) {
			c._age._base = 200;
			setCondition(ERADICATED);
		}

		LINE("monster_actions.causes_aging");
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action16(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (c._level < 3) {
		c._level._current = 0;
		setCondition(BAD_CONDITION | DEAD);

	} else {
		c._level._current -= 2;
	}

	_damage += 20;
	LINE("monster_actions.drains_lifeforce");
	return true;
}

bool MonsterActions::action17(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(3) && isCharAffected()) {
		setCondition(BAD_CONDITION | DEAD);
		line = Common::String::format("%s %s",
			c.getDisplayName().c_str(),
			STRING["monster_actions.is_killed"].c_str()
		);
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action18(Common::String &line) {
	if (canPerform(3) && isCharAffected()) {
		setCondition(UNCONSCIOUS);
		LINE("monster_actions.induces_unconsciousness");
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action19(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (c._might._current < 4) {
		c._might._current = 0;
		setCondition(BAD_CONDITION | DEAD);

	} else {
		c._might._current -= 3;
	}

	LINE("monster_actions.drains_might");
	return true;
}

bool MonsterActions::action20(Common::String &line) {
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

	LINE("monster_actions.drains_abilities");
	return true;
}

bool MonsterActions::action21(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2)) {
		c._backpack.clear();
		LINE("monster_actions.steals_backpack");
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action22(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	if (canPerform(2)) {
		c._gold = 0;
		c._gems = 0;

		LINE("monster_actions.steals_gold_and_gems");
		return true;

	} else {
		return false;
	}
}

bool MonsterActions::action23(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	setCondition(ERADICATED);
	line = Common::String::format("%s %s",
		c.getDisplayName().c_str(),
		STRING["monster_actions.is_eradicated"].c_str()
	);

	return true;
}

bool MonsterActions::action24(Common::String &line) {
	Character &c = *g_globals->_currCharacter;

	c._sp._current = 0;
	LINE("monster_actions.drains_sp");
	return true;
}

} // namespace Game
} // namespace MM1
} // namespace MM
