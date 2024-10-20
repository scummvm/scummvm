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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM805_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM805_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room805 : public Room {
public:
	Room805() : Room() {}
	~Room805() override {}

	void preload() override;
	void init() override;
	void parser() override;
	void daemon() override;

private:
	int32 _lastRnd = 1;
	int32 _meiHandsBehindBack = 0;
	int32 _meiSpookedPos2 = 0;
	int32 _meiTalkPos3 = 0;
	int32 _meiTrekTalkerSeries = 0;
	int32 _ripHeadTurnPos3 = 0;
	int32 _ripLookDownPos3 = 0;
	int32 _ripLooksUpSeries = 0;
	int32 _ripLowReachPos2 = 0;
	int32 _ripMedHiReachPos2 = 0;
	int32 _ripMedHiReachPos3 = 0;
	int32 _ripPos3LookAround = 0;
	int32 _ripSiftsDirtSeries = 0;
	int32 _ripTalkerPos3 = 0;

	int32 _unkSeries1 = 0;
	int32 _unkSeries5 = 0;
	int32 _unkSeries6 = 0;
	int32 _unkSeries7 = 0;
	int32 _unkSeries8 = 0;

	bool _unkFlag1 = false;

	machine *_chariotRestMach = nullptr;
	machine *_fallenBeamOnFloorMach = nullptr;
	machine *_farSoldiersShieldMach = nullptr;
	machine *_jadeDoorsOpenMach = nullptr;
	machine *_mcMach = nullptr;
	machine *_nearSoldiersShieldMach = nullptr;
	machine *_ripChariotInMach = nullptr;
	machine *_ripSiftsDirtMach = nullptr;

	void initSub2();
	void initHotspots();
	void daemonSub1();
	void daemonSub2(const char *seriesName1, const char *seriesName2);
	void daemonSub3(const char *seriesName1, const char *seriesName2);
	void daemonSub4(const char *seriesName1);
	void parserSub1(const char *name, uint channel, int32 vol, int32 trigger, int32 room_num);
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
