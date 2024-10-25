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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM806_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM806_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room806 : public Room {
public:
	Room806() : Room() {}
	~Room806() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	int32 _unkSeries1 = 0;

	int32 _unkVar1 = 0;
	int32 _unkVar2 = 0;
	int32 _unkVar3 = 0;
	int32 _unkVar4 = 0;
	int32 _unkVar5 = 0;

	int32 _806mc01Series = 0;
	int32 _806mc02Series = 0;
	int32 _806rp01Series = 0;
	int32 _806rp02Series = 0;
	int32 _806rp03Series = 0;
	int32 _meiTalkPos3Series = 0;
	int32 _meiTrekHandsOnHipsPos3Series = 0;
	int32 _meiTrekHeadTurnPos3Series = 0;
	int32 _ripHighReacherPos2Series = 0;
	int32 _ripKneelAndExamineWallSeries = 0;
	int32 _ripLookUpPos3Series = 0;
	int32 _ripMedHighReacherPos2Series = 0;
	int32 _ripPos3LookAroundSeries = 0;
	int32 _ripTrekHandTalkPos3Series = 0;
	int32 _ripTrekHeadTurnPos3Series = 0;
	int32 _ripTrekLookDownPos3Series = 0;
	int32 _ripTrekTalkerPos3Series = 0;

	machine *_806CrankMach = nullptr;
	machine *_806ChartMach = nullptr;
	machine *_806mc01Mach = nullptr;
	machine *_806rp01Mach = nullptr;
	machine *_candlemanShadow3Mach = nullptr;
	machine *_mcTrekMach = nullptr;
	machine *_safariShadow3Mach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
