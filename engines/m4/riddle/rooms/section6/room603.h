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

#ifndef M4_RIDDLE_ROOMS_SECTION6_ROOM603_H
#define M4_RIDDLE_ROOMS_SECTION6_ROOM603_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room603 : public Room {
private:
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val9 = 0;
	int _val10 = 0;
	int _ripleyMode = 0;
	int _ripleyShould = 0;
	int _ttMode = 0;
	int _ttShould = 0;
	int _trigger1 = 0;
	machine *_shed = nullptr;
	machine *_shenWalker = nullptr;
	machine *_kuangWalker = nullptr;
	machine *_note = nullptr;
	machine *_pole = nullptr;
	machine *_shirt = nullptr;
	machine *_sleeve = nullptr;
	machine *_door = nullptr;
	machine *_tt = nullptr;
	machine *_ttShadow = nullptr;
	machine *_pu01 = nullptr;
	machine *_ripley = nullptr;
	machine *_shadow = nullptr;
	machine *_treesGoneHome = nullptr;
	machine *_explosion = nullptr;
	int _rp01 = 0;
	int _rp02 = 0;
	int _ttDigShirtOff = 0;
	int _ttDigShirtOn = 0;
	int _tt03 = 0;
	int _ttD01 = 0;
	int _rp04 = 0;
	int _ripHandsBehindBack = 0;
	int _ripTalk = 0;
	int _ripHandsHips = 0;
	int _ripHandChin = 0;
	int _ripTwoHandTalk = 0;
	int _ripAction = 0;

	void conv603a();
	void conv603b();
	void unloadSeries();
	bool takeSleeve();
	bool takeNote();
	bool takePole();
	void enter();
	void playRandomDigi(int max);

public:
	Room603() : Room() {}
	~Room603() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
	void syncGame(Common::Serializer &s) override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
