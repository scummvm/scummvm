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

#ifndef M4_BURGER_ROOMS_SECTION4_ROOM405_H
#define M4_BURGER_ROOMS_SECTION4_ROOM405_H

#include "m4/burger/rooms/section4/section4_room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room405 : public Section4Room {
private:
	static const char *SAID[][4];
	static const seriesStreamBreak SERIES1[];
	static const seriesPlayBreak PLAY1[];
	static const seriesPlayBreak PLAY2[];
	static const seriesPlayBreak PLAY3[];
	static const seriesPlayBreak PLAY4[];
	static const seriesPlayBreak PLAY5[];
	static const seriesPlayBreak PLAY6[];
	static const seriesPlayBreak PLAY7[];
	machine *_series1 = nullptr;
	machine *_records = nullptr;
	machine *_box = nullptr;
	machine *_lid = nullptr;
	machine *_eu02 = nullptr;
	const char *_digiName = nullptr;
	KernelTriggerType _newMode = (KernelTriggerType)0;
	int _volume = 0;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _newTrigger = 0;

	void conv86();
	void conv89();
	void conv90();
	void conv91();
	void conv92();
	void startConv91();
	void talkToVipe();
	void talkToVera();
	void playDigi();

public:
	Room405() : Section4Room() {}
	~Room405() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
