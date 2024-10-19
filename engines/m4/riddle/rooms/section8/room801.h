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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM801_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM801_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room801 : public Room {
public:
	Room801() : Room() {}
	~Room801() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	void room801_conv801a();

	int32 _cellarDoorClosedSeries = 0;
	int32 _cellarDoorOpenSeries = 0;
	int32 _farmerDisplacementSeries = 0;
	int32 _farmerEarthquakeGestureSeries = 0;
	int32 _farmerGiveTakeBowSeries = 0;
	int32 _farmerLooksToZeroSeries = 0;
	int32 _farmerTalkLoopSeries = 0;
	int32 _meiLookToZeroSeries = 0;
	int32 _meiPrattleLoopSeries = 0;
	int32 _mtcsh1Series = 0;
	int32 _planeShadowSeries = 0;
	int32 _ripEntersRootCellarSeries = 0;
	int32 _ripKneelPotterySeries = 0;
	int32 _ripLooksLRDownSeries = 0;
	int32 _ripLooksToZeroSeries = 0;
	int32 _ripReturnsFromRootCellarSeries = 0;
	int32 _ripTrekHandOnChinSeries = 0;
	int32 _ripTrekHandsBehBackSeries = 0;
	int32 _ripTrekHeadTurnSeries = 0;
	int32 _ripTrekNodPos4Series = 0;
	int32 _rptmr15Series = 0;
	int32 _shovelSeries = 0;

	int32 _counter = 0;
	int32 _unkVal1 = 0;
	int32 _unkVal2 = 0;
	int32 _unkVal3 = 0;
	int32 _unkVal4 = 0;
	int32 _unkVal5 = 0;
	int32 _unkVal6 = 0;
	int32 _unkVal7 = 0;

	bool _unkFlag1 = false;
	bool _unkFlag2 = false;
	bool _unkFlag3 = false;

	int32 _roomStates_field2A = 0;
	int32 _roomStates_field4C2 = 0;
	int32 _roomStates_field502 = 0;
	int32 _roomStates_field7E = 0;
	int32 _roomStates_ripTalking = 0;

	machine *_cellarDoorClosedMach = nullptr;
	machine *_cellarDoorOpenMach = nullptr;
	machine *_farmerDisplacementMach = nullptr;
	machine *_farmerShadowMach = nullptr;
	machine *_mcTrekMach = nullptr;
	machine *_planeShadowMach = nullptr;
	machine *_ripAnimationMach = nullptr;
	machine *_safariShadow5Mach = nullptr;
	machine *_safariShadow5Mach2 = nullptr;
	machine *_shovelMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
