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

#ifndef M4_RIDDLE_ROOMS_SECTION6_ROOM610_H
#define M4_RIDDLE_ROOMS_SECTION6_ROOM610_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room610 : public Room {
private:
	int _ripPeerAroundHut = 0;
	int _rp04 = 0;
	int _k00 = 0;
	int _k01 = 0;
	int _sgSlapsTt1 = 0;
	int _sgSlapsTt2 = 0;
	int _sgPullsGun = 0;
	machine *_ripAction = nullptr;
	machine *_k = nullptr;
	machine *_sgTt = nullptr;
	machine *_pu01 = nullptr;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;
	bool _flag1 = false;
	bool _flag2 = false;

	static void triggerMachineByHashCallback610(frac16 myMessage, machine *sender);
	void setup();
	void talkKuangShenGuo();
	void talkSamantha();
	bool useHorn();
	void useWindow();
	void daemonPreprocess();
	void daemonPostprocess();

public:
	Room610() : Room() {}
	~Room610() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
