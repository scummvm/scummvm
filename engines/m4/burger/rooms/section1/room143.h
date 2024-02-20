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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM143_H
#define M4_BURGER_ROOMS_SECTION1_ROOM143_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room143 : public Room {
private:
	static const char *SAID[][4];
	static const seriesPlayBreak PLAY1[];
	static const seriesPlayBreak PLAY2[];
	static const seriesPlayBreak PLAY3[];
	static const seriesPlayBreak PLAY4[];
	static const seriesPlayBreak PLAY5[];
	static const seriesPlayBreak PLAY6[];
	static const seriesPlayBreak PLAY7[];
	static const seriesPlayBreak PLAY8[];
	static const seriesPlayBreak PLAY9[];
	static const seriesPlayBreak PLAY10[];
	static const seriesPlayBreak PLAY11[];
	machine *_cheese = nullptr;
	machine *_mouseTrap = nullptr;
	machine *_plate = nullptr;
	machine *_eu02 = nullptr;
	machine *_wi03 = nullptr;
	machine *_wi03S = nullptr;
	machine *_ve03 = nullptr, *_ve03S = nullptr;
	Series _cat;
	Series _emptyPlates;
	noWalkRect *_walk1 = nullptr;
	const char *_digiName = nullptr;
	bool _flag1 = false;
	int _frame = 0;
	int _veraShould = 0;
	int _veraMode = 0;
	int _val3 = 0;
	int _catShould = 0;
	int _burlMode = 0;
	int _burlShould = 0;
	KernelTriggerType _digiMode = (KernelTriggerType)0;
	int _digiTrigger = 0;

	void conv35();
	void conv30();
	void talkToVera();
	void talkToBurl();
	void loadCheese();
	void showEmptyPlates();
	void playDigi1();
	void playDigi2();

public:
	Room143() : Room() {}
	~Room143() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
