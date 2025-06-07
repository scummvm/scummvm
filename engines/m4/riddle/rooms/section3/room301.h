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

#ifndef M4_RIDDLE_ROOMS_SECTION3_ROOM301_H
#define M4_RIDDLE_ROOMS_SECTION3_ROOM301_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room301 : public Room {
private:
	int32 _travelDest = 0;
	const char *_soundName = nullptr;
	const char *_nextSound = nullptr;
	KernelTriggerType _val6_triggerMode = KT_DAEMON;
	KernelTriggerType _georgeTriggerMode = KT_DAEMON;
	bool _showWalkerFlag = false;	// Value never seems to be set to true
	bool _msgRipleyFlag = false;
	bool _convResumeFlag = false;

	int _agentStander = 0;
	int _agentCheckingList = 0;
	int _agentSlidesPaper = 0;
	int _agentTakesMoney = 0;
	int _agentTakesTelegram = 0;
	int _agentTalk = 0;
	int _agentSalutes = 0;
	int _georgeMode = 0;
	int _georgeRandomActionCounter = 0;
	int _georgeShould = 0;
	int _georgeTrigger = 0;
	int _marshalMatt = 0;
	int _ripleyMode = 0;
	int _ripleyShould = 0;
	int _ripleyTrigger = -1;
	int _ripTrekArms = 0;
	int _ripTrekHandTalk3 = 0;
	int _ripTrekTalker3 = 0;
	int _ripTrekTravel = 0;

	int _val12 = 0;
	int _marshallShould = 0;
	int _marshallMode = 0;

	machine *_georgeMach = nullptr;
	machine *_marshallMach = nullptr;
	machine *_ripleyMach = nullptr;
	machine *_shadowMach = nullptr;

	const char *_digiSound1 = nullptr;
	const char *_digiSound2 = nullptr;
	const char *_digiSound3 = nullptr;
	const char *_digiSound4 = nullptr;

	void conv301a();

public:
	Room301() : Room() {}
	~Room301() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
