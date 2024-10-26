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

#ifndef M4_RIDDLE_ROOMS_SECTION2_ROOM201_H
#define M4_RIDDLE_ROOMS_SECTION2_ROOM201_H

#include "m4/riddle/rooms/section2/section2_room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room201 : public Section2Room {
private:
	bool _flag1 = false;
	bool _flag2 = false;
	bool _flag3 = false;
	int32 _travelDest = 0;
	int _val1 = 0;
	int32 _givenYuan = 0;
	const char *_itemDigi = nullptr;
	const char *_itemDigi2 = nullptr;
	const char *_itemDigi3 = nullptr;
	const char *_itemDigi4 = nullptr;
	const char *_itemDigi5 = nullptr;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val8 = 0;
	int _val9 = 0;
	int _conv1 = 0;
	int _conv2 = 0;
	int _nod = 0;
	int _guyWriting = 0;
	int _guyParcel = 0;
	int _guyPassForm = 0;
	int _guyBow = 0;
	int _guySeries1 = 0;
	int _series1 = -1;
	int _series2 = -2;
	int _series3 = -1;
	int _series4 = -1;
	int _series5 = -1;
	int _series6 = -2;
	int _series7 = -1;
	int _series8 = -1;
	machine *_mei0 = nullptr;
	machine *_mei1 = nullptr;
	machine *_mei2 = nullptr;
	machine *_ripley = nullptr;
	machine *_doc = nullptr;
	machine *_shadow3 = nullptr;
	machine *_agent = nullptr;
	int _meiHandHip = 0;
	int _meiTalker = 0;
	int _meiWalk = 0;
	int _meiHeadTurn = 0;
	int _meiTalkLoop = 0;
	int _ripHandChin = -1;
	int _ripTalk = -1;
	int _num1 = 0;
	int _num2 = 0;
	int _num3 = 0;
	int _trigger1 = -1;
	int _trigger2 = -1;
	int _trigger3 = -1;
	int _trigger4 = -1;
	int _trigger5 = -1;
	int _trigger6 = -1;
	int _trigger7 = -1;
	int _trigger8 = -1;
	int _trigger9 = -1;
	int _trigger10 = -1;
	int _trigger11 = -1;
	int _trigger12 = -1;
	int _items[12];
	int32 _itemFlags[12];
	int _totalItems = 0;
	int32 _hasKeyItems = 0;

	int _ctr1 = 0;
	int _ctr2 = 0;

	void setupMei();
	void conv201a();
	void animateRipley();
	void checkFlags();
	bool buildKeyItemsArray();

public:
	Room201();
	~Room201() override {}

	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
