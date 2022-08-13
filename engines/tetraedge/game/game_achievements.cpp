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

#include "tetraedge/game/game_achievements.h"
#include "tetraedge/te/te_lua_context.h"

namespace Tetraedge {

GameAchievements::GameAchievements() {
}

/*static*/ void GameAchievements::registerAchievements(TeLuaContext &context) {
	context.setGlobal("PS3_Welcome", 0);
	context.setGlobal("PS3_Legacy", -1);
	context.setGlobal("PS3_Graduate", -2);
	context.setGlobal("PS3_Easten", -3);
	context.setGlobal("PS3_Odyssey", -4);
	context.setGlobal("PS3_Code", -5);
	context.setGlobal("PS3_Escape", -6);
	context.setGlobal("PS3_Amerzone", -7);
	context.setGlobal("PS3_Music", -8);
	context.setGlobal("PS3_OnWay", -9);
	context.setGlobal("PS3_Gossip", -10);
	context.setGlobal("PS3_Snoop", -0xb);
	context.setGlobal("PS3_School", -0xc);
	context.setGlobal("XBOX_Welcome", 1);
	context.setGlobal("XBOX_Legacy", 2);
	context.setGlobal("XBOX_Graduate", 3);
	context.setGlobal("XBOX_Easten", 4);
	context.setGlobal("XBOX_Odyssey", 5);
	context.setGlobal("XBOX_Code", 6);
	context.setGlobal("XBOX_Escape", 7);
	context.setGlobal("XBOX_Amerzone", 8);
	context.setGlobal("XBOX_Music", 9);
	context.setGlobal("XBOX_OnWay", 10);
	context.setGlobal("XBOX_Gossip", 0xb);
	context.setGlobal("XBOX_Snoop", 0xc);
	context.setGlobal("XBOX_School", 0xd);
}

} // end namespace Tetraedge
