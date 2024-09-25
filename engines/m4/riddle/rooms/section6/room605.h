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

#ifndef M4_RIDDLE_ROOMS_SECTION6_ROOM605_H
#define M4_RIDDLE_ROOMS_SECTION6_ROOM605_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room605 : public Room {
private:
	int _trigger1 = 0;
	bool _walkerHidden = false;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _ripHandsHips = 0;
	int _ripHandOnChin = 0;
	int _ripTwoHandTalk = 0;
	int _ripTalk = 0;
	int _ripGetsIrisWithCloth = 0;
	int _ripHandOnIris = 0;
	int _605tt = 0;
	machine *_pupil = nullptr;
	machine *_ripley = nullptr;
	machine *_tt = nullptr;
	machine *_ttShadow = nullptr;
	machine *_shadow = nullptr;

	void conv605a();
	bool takePupilDisk();
	bool sleeveDisk1();
	bool sleeveDisk2();
	bool parserMisc();

public:
	Room605() : Room() {}
	~Room605() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
