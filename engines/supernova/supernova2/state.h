/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SUPERNOVA2_STATE_H
#define SUPERNOVA2_STATE_H

#include "common/events.h"
#include "common/rect.h"
#include "common/keyboard.h"
#include "supernova/room.h"
#include "supernova/supernova2/rooms.h"
#include "supernova/sound.h"
#include "supernova/game-manager.h"

namespace Supernova {

struct ConstructionEntry {
	int _e;
	int _s;
	int _z;
	int _r;
	int _a;
};

struct GameState2 {
	int16 _money;
	int32 _startTime;
	bool _addressKnown;
	Room *_previousRoom;
	bool _poleMagnet;
	char _admission;
	bool _tipsy;
	bool _dark;
	char _elevatorE;
	char _elevatorNumber;
	bool _toMuseum;
	EventFunction _eventCallback;
	uint32 _eventTime;
	int16 _pyraE;
	char _pyraS;
	char _pyraZ;
	int16 _pyraDirection;
	int16 _puzzleTab[15];
	bool _alarmCracked;
	bool _alarmOn;
	bool _haste;
	byte _pressureCounter;
	bool _sirenOn;
	byte _taxiPossibility;
};

class GameManager2: public GameManager{
public:
	GameManager2(SupernovaEngine *vm, Sound *sound);
	~GameManager2() override;

	GameState2 _state;

	void updateEvents() override;
	void executeRoom() override;
	bool serialize(Common::WriteStream *out) override;
	bool deserialize(Common::ReadStream *in, int version) override;

	byte _dials[6];

	//state
	unsigned char _puzzleField[16];
	bool _mapOn;
	bool _steps;
	bool _cracking;
	bool _alarmBefore;
	RoomId _securityTab[10];
	int _restTime;

	void initState() override;
	void initRooms() override;
	void destroyRooms() override;
	bool canSaveGameStateCurrently() override;
	bool genericInteract(Action verb, Object &obj1, Object &obj2) override;
	void roomBrightness() override {}
	void drawMapExits() override;
	void handleInput() override;
	void handleTime() override;
	void loadTime() override {}
	void saveTime() override {}
	void takeMoney(int amount) override;
	void taxi();
	void leaveTaxi();
	void taxiUnknownDestination();
	void taxiPayment(int price, int destination);
	void playerTakeOut();
	void sober();
	void playCD();
	bool talk(int mod1, int mod2, int rest, MessagePosition pos, int id);
	bool talkRest(int mod1, int mod2, int rest);
	void pyramidEnd();
	void passageConstruction();
	byte wall(int s, int z, int direction, int stepsForward, int stepsRight);
	bool move(Action verb, Object &obj);
	void compass();
	void puzzleConstruction();
	void drawClock();
	void caught();
	void caught2();
	void alarm();
	void crack(int time);
	bool crackDoor(int time);
	void museumDoorInteract(Action verb, Object &obj1, Object &obj2);
	void securityEntrance();
	void pressureAlarmCount();
	void pressureAlarmEntrance();
};

}

#endif // SUPERNOVA2_STATE_H
