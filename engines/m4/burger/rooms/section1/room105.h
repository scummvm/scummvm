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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM105_H
#define M4_BURGER_ROOMS_SECTION1_ROOM105_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room105 : public Room {
private:
	int _val1 = 0;
	int _elgusShould = 0;
	int _elgusMode = 0;
	KernelTriggerType _savedMode = (KernelTriggerType)0;
	int _savedTrigger = 0;
	//int _val7 = 0;
	int _val8 = 0;
	bool _flag1 = false;
	bool _flag2 = false;
	bool _flag3 = false;
	bool _flag4 = false;
	bool _flag5 = false;
	const char *_digi1 = nullptr;
	int _series1[12];
	int _series2[4];
	int _series3[12];
	int _series4[5];
	machine *_series5 = nullptr;
	machine *_series6 = nullptr;
	machine *_series7 = nullptr;
	machine *_series8 = nullptr;
	machine *_series9 = nullptr;
	machine *_series10 = nullptr;
	int _series11 = 0;
	int _series12 = 0;

	void conv10();
	void conv11();
	void conv12();
	void conv13();
	void startConv13();
	void mayorsDoor();
	void loadAssets1();
	void loadAssets2();
	void loadAssets3();
	void loadAssets4();
	void unloadAssets1();
	void unloadAssets2();
	void unloadAssets3();
	void unloadAssets4();
	void townRecords();
	void complaintsDept();
	void mayorsOffice();

public:
	Room105() : Room() {}
	~Room105() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
