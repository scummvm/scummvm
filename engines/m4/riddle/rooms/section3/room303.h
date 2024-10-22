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

#ifndef M4_RIDDLE_ROOMS_SECTION3_ROOM303_H
#define M4_RIDDLE_ROOMS_SECTION3_ROOM303_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room303 : public Room {
private:
	bool _newExhibitsFlag = false;
	bool _lonelyFlag = 0;
	const char *_digiName1 = nullptr;
	int _destTrigger = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;
	//int _val9 = 0;
	int _val10 = 0;
	int _val11 = 0;
	KernelTriggerType _val12 = KT_DAEMON;
	bool _fengFlag = false;
	int _fengShould = 0;
	int _fengMode = 0;
	int _val16 = 0;
	int _val17 = 0;
	int _val18 = 0;
	KernelTriggerType _triggerMode1 = KT_DAEMON;
	KernelTriggerType _triggerMode2 = KT_DAEMON;
	machine *_door = nullptr;
	int _mei1 = 0;
	int _mei2 = 0;
	int _mei3 = 0;
	machine *_mei = nullptr;
	int _feng1 = 0;
	int _feng2 = 0;
	int _feng3 = 0;
	int _feng4 = 0;
	machine *_shadow4 = nullptr;
	machine *_shadow5 = nullptr;
	machine *_fengLi = nullptr;
	machine *_machine2 = nullptr;
	machine *_machine3 = nullptr;
	machine *_ripPonders = nullptr;
	int _gestTalk4 = 0;
	int _ripBends = 0;
	int _chinTalk4 = 0;
	int _med1 = 0;
	machine *_ripsh1 = nullptr;
	machine *_ripsh2 = nullptr;
	int _suit1 = 0;
	int _suit2 = 0;
	int _meiLips = 0;
	int _ripGesture = 0;
	machine *_priestTalk = nullptr;
	int _suitTalk1 = 0;

	int _ctr1 = 0;
	bool _btnFlag = false;

	static void escapePressed(void *, void *);

	void setupMei();
	void loadFengLi();
	void setFengActive(bool flag);
	void setShadow4(bool active);
	void setShadow5(bool active);
	void setShadow5Alt(bool active);
	void playSeries(bool cow = true);
	void conv303a();
	void conv303b();
	static void priestTalkCallback(frac16 myMessage, machine *sender);
	void priestTalk(bool flag, int trigger);
	int getSize(const Common::String &assetName, int roomNum = -1);
	void playSound(const Common::String &assetName, int trigger1, int trigger2);

public:
	Room303() : Room() {}
	~Room303() override {}

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
