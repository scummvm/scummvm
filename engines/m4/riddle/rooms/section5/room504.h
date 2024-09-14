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

#ifndef M4_RIDDLE_ROOMS_SECTION5_ROOM504_H
#define M4_RIDDLE_ROOMS_SECTION5_ROOM504_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room504 : public Room {
private:
	machine *_ripley = nullptr;
	machine *_waterfall = nullptr;
	int _upSteps = 0;
	int _downSteps = 0;
	int _volume = 0;
	int _volume2 = 0;
	int _convState1 = 0;
	int _convState2 = 0;
	int _convState3 = 0;
	int _ripSketching = 0;
	int _ripLowReach = 0;
	int _ripMedReach = 0;
	int _ripWipe = 0;
	int _ripStepUpLeft = 0;
	int _ripThrowFromLeft = 0;
	int _ripHiReach2Handed = 0;
	int _ripLeansBack = 0;
	int _ripStepUpRight = 0;
	int _ripThrowFromRight = 0;
	int _ripKneels = 0;
	int _ripShrunkenHead = 0;
	int _ripGivesMoneyEmerald = 0;
	int _ripGetsUp = 0;
	int _ripYells = 0;
	int _mzDigs = 0;
	int _mzStandsTalks = 0;
	int _mzTakesEmerald = 0;
	int _mzMenancesClimbs = 0;
	int _mzTakesMoney = 0;
	int _mzGivesHead = 0;
	int _vineTie = 0;
	int _vineUnrolling = 0;
	int _rightVineTie = 0;
	int _leftVineTie = 0;
	int _ropeLSlurpsUp = 0;
	int _ropeRSlurpsUp = 0;

	machine *_vines1 = nullptr;
	machine *_vines2 = nullptr;
	machine *_vines3 = nullptr;
	machine *_vines4 = nullptr;
	machine *_rope = nullptr;
	machine *_ladder = nullptr;
	machine *_toy = nullptr;
	machine *_shovel = nullptr;
	machine *_driftwood = nullptr;
	machine *_pole = nullptr;
	machine *_mzMachine = nullptr;
	machine *_menendez = nullptr;
	machine *_vineMachine1 = nullptr;
	machine *_vineMachine2 = nullptr;
	machine *_ripStep = nullptr;
	machine *_vine = nullptr;
	bool _flag1 = false;
	bool _flag2 = false;
	bool _flag3 = false;
	bool _flag4 = false;
	
	int _trigger1 = 0;
	int _trigger2 = 0;
	int _trigger3 = 0;
	int _trigger4 = 0;
	int _val1 = 0;
	int _val2 = 0;
	int32 _val3 = 0;

	void setVines();
	void freeVines();
	void setVinesRope();
	void setMiscItems();
	void disableVinesRope();
	void deleteHotspot(const char *hotspotName);
	void addRope();
	void addGreenVine();
	void addBrownVine();
	void addHotspot(int x1, int y1, int x2, int y2,
		const char *verb, const char *vocab);
	void addShovel();
	bool checkVinesDistance();
	bool parser1();
	void conv504a();
	bool lookVines();

public:
	Room504() : Room() {}
	~Room504() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
