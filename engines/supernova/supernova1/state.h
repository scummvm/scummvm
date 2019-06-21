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

#ifndef SUPERNOVA_STATE_H
#define SUPERNOVA_STATE_H

#include "common/error.h"
#include "common/events.h"
#include "common/rect.h"
#include "common/keyboard.h"
#include "supernova/supernova1/rooms.h"
#include "supernova/room.h"
#include "supernova/sound.h"
#include "supernova/game-manager.h"

namespace Supernova {

class GameManager;

class GameState {
	public:
		int32  _timeSleep;
		int32  _timeAlarm;
		int32  _eventTime;
		EventFunction _eventCallback;
		int32  _arrivalDaysLeft;
		int32  _shipEnergyDaysLeft;
		int32  _landingModuleEnergyDaysLeft;
		uint16 _greatFlag;
		int16  _timeRobot;
		int16  _money;
		byte   _coins;
		byte   _shoes;
		byte   _origin;
		byte   _destination;
		byte   _language;
		bool   _corridorSearch;
		bool   _alarmOn;
		bool   _terminalStripConnected;
		bool   _terminalStripWire;
		bool   _cableConnected;
		bool   _powerOff;
		bool   _dream;
		bool   _nameSeen[4];
		bool   _playerHidden;
};

class GameManager1: public GameManager {
public:
	GameManager1(SupernovaEngine *vm, Sound *sound);
	virtual ~GameManager1();

	GameState _state;

	virtual void updateEvents();
	virtual void executeRoom();
	virtual bool serialize(Common::WriteStream *out);
	virtual bool deserialize(Common::ReadStream *in, int version);

	virtual void initState();
	virtual void initRooms();
	virtual void destroyRooms();
	virtual bool canSaveGameStateCurrently();
	virtual bool genericInteract(Action verb, Object &obj1, Object &obj2);
	bool isHelmetOff();
	void great(uint number);
	bool airless();
	void shock();
	void turnOff();
	void turnOn();
	virtual void roomBrightness();
	void openLocker(const Room *room, Object *obj, Object *lock, int section);
	void closeLocker(const Room *room, Object *obj, Object *lock, int section);
	virtual void drawMapExits();
	virtual void handleInput();
	virtual void handleTime();
	virtual void loadTime();
	virtual void saveTime();
	void shot(int a, int b);
	virtual void takeMoney(int amount);
	void search(int time);
	void startSearch();
	void guardNoticed();
	void busted(int i);
	void corridorOnEntrance();
	void telomat(int number);
	void novaScroll();
	void supernovaEvent();
	void guardReturnedEvent();
	void walk(int a);
	void guardWalkEvent();
	void taxiEvent();
	void searchStartEvent();
	void guardShot();
	void guard3Shot();
	void alarm();
	void alarmSound();

private:
	int _prevImgId;
};

}

#endif // SUPERNOVA_STATE_H
