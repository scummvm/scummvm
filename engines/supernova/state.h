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

#ifndef STATE_H
#define STATE_H

#include "common/rect.h"
#include "supernova/rooms.h"

namespace Supernova {

struct GameState {
	int32  time;
	int32  timeSleep;
	int32  timeStarting;
	int32  timeAlarm;
	int32  timeAlarmSystem;
	int32  eventTime;
	int32  shipEnergy;
	int32  landingModuleEnergy;
	uint16 greatF;
	int16  timeRobot;
	int16  money;
	char   coins;
	char   shoes;
	char   nameSeen;
	char   destination;
	char   benOverlay;
	char   language;
	bool   corridorSearch;
	bool   alarmOn;
	bool   terminalStripConnected;
	bool   terminalStripWire;
	bool   cableConnected;
	bool   powerOff;
	bool   cockpitSeen;
	bool   airlockSeen;
	bool   holdSeen;
	bool   dream;
};

class Inventory {
public:
	Inventory();

	void add(Object &obj);
	void remove(Object &obj);
	Object *get(uint index) const;
	Object *get(ObjectID id) const;
	uint getSize() const { return _numObjects; }

private:
	Object *_inventory[kMaxCarry];
	uint _numObjects;
};

class GameManager {
public:
	GameManager(SupernovaEngine *vm);

	void processInput(Common::KeyState &state);
	void processInput(Common::EventType eventType, int x, int y);
	void executeRoom();

	SupernovaEngine *_vm;
	uint16 _key;
	Common::EventType _mouseClickType;
	int _mouseX;
	int _mouseY;
	int _mouseField;
	Room *_currentRoom;
	Room *_rooms[kRoomsNum];
	Inventory _inventory;
	GameState _state;
	int _status;
	Action _inputVerb;
	bool _inputVerb2;
	Object _nullObjectInstance;
	Object *_inputObject[2];
	int _objectNumber;
	bool _waitEvent;
	bool _newRoom;
	bool _newOverlay;
	int _timer1;
	int _timer2;
	int _inventoryScroll;
	int _exitList[25];
	Common::Rect _guiCommandDimensions[10];
	// 0 PC Speaker | 1 SoundBlaster | 2 No Sound
	int _soundDevice;

	void takeObject(Object &obj);

	void initGui();
	bool genericInteract(Action verb, Object &obj1, Object &obj2);
	bool isHelmetOff();
	void great(uint number);
	bool airless();
	void shock();
	void mouseInput();
	void mouseInput2();
	void mouseInput3();
	void mouseWait(int delay);
	void wait2(int delay);
	void turnOff();
	void turnOn();
	void screenShake();
	void loadTime();
	void saveTime();
	bool saveGame(int number);
	void errorTemp();
	void roomBrightness();
	void palette();
	void showMenu();
	void animationOff();
	void animationOn();
	void loadOverlayStart();
	void openLocker(const Room *room, Object *obj, Object *lock, int section);
	void closeLocker(const Room *room, Object *obj, Object *lock, int section);
	void edit(char *text, int x, int y, int length);
	int invertSection(int section);
	void drawMapExits();
	void drawStatus();
	void drawCommandBox(int cmd, bool brightness);
	void drawCommandBox();
	void inventory_arrow(int num, bool brightness);
	void inventory_object(int num, bool brightness);
	void drawInventory();
	void changeRoom(RoomID id);
};

}

#endif // STATE_H
