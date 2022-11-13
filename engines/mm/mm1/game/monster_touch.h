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

#ifndef MM1_GAME_MONSTER_TOUCH_H
#define MM1_GAME_MONSTER_TOUCH_H

#include "mm/mm1/game/spells_monsters.h"

namespace MM {
namespace MM1 {
namespace Game {

class MonsterTouch;
typedef bool (MonsterTouch::*MonsterTouchAction)(Common::String &line);

class MonsterTouch : public SpellsMonsters {
private:
	static const MonsterTouchAction ACTIONS[25];

	bool canPerform(int level) const;
private:
	bool action00(Common::String &line);
	bool action01(Common::String &line);
	bool action02(Common::String &line);
	bool action03(Common::String &line);
	bool action04(Common::String &line);
	bool action05(Common::String &line);
	bool action06(Common::String &line);
	bool action07(Common::String &line);
	bool action08(Common::String &line);
	bool action09(Common::String &line);
	bool action10(Common::String &line);
	bool action11(Common::String &line);
	bool action12(Common::String &line);
	bool action13(Common::String &line);
	bool action14(Common::String &line);
	bool action15(Common::String &line);
	bool action16(Common::String &line);
	bool action17(Common::String &line);
	bool action18(Common::String &line);
	bool action19(Common::String &line);
	bool action20(Common::String &line);
	bool action21(Common::String &line);
	bool action22(Common::String &line);
	bool action23(Common::String &line);
	bool action24(Common::String &line);

public:
	virtual ~MonsterTouch() {}

	bool monsterTouch(uint index, Common::String &line);
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
