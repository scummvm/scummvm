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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM102_H
#define M4_BURGER_ROOMS_SECTION1_ROOM102_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room102 : public Room {
private:
	int _val1 = 0;
	int _val3 = -1;
	int32 _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val8 = 0;
	int _val9 = 0;
	uint32 _val10 = 99999;
	int _harryMode = 0;
	int _harryShould = 0;
	int _wilburChairShould = 0;
	int _wilburChairMode = 0;
	int _val16 = 0;
	int _trigger = -1;
	KernelTriggerType _triggerMode = KT_DAEMON;
	machine *_series1 = nullptr;
	machine *_series1s = nullptr;
	machine *_series3 = nullptr;
	Series _wilburChair;
	machine *_series6 = nullptr;
	machine *_series7 = nullptr;
	machine *_laz1 = nullptr;
	machine *_laz2 = nullptr;
	machine *_stream1 = nullptr;
	int _index1 = 0, _index2 = 0;
	int _index3 = 0, _index4 = 0;
	int _index5 = 0;
	const char *_play1 = nullptr;
	bool _flag1 = false;

	void setup(int val1 = 0, int val2 = 1);
	void setupWax();
	void setupLaz();
	const char *getDigi1(int num) const;
	const char *getDigi2(int num) const;
	void freeWilburChair();
	void queuePlay(const char *filename, int trigger = 29, KernelTriggerType triggerMode = KT_DAEMON);
	void freshen();
	void conv04();
	void conv05();
	void conv06();
	void flagAction(const char *name1, const char *name2);

public:
	Room102() : Room() {}
	~Room102() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
