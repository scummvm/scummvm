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
	uint16 greatFlag;
	int16  timeRobot;
	int16  money;
	byte   coins;
	byte   shoes;
	byte   nameSeen;
	byte   destination;
	byte   benOverlay;
	byte   language;
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
	void clear();
	Object *get(int index) const;
	Object *get(ObjectID id) const;
	int getSize() const { return _numObjects; }

private:
	Object *_inventory[kMaxCarry];
	int _numObjects;
};

class GuiElement : public Common::Rect {
public:
	GuiElement();

	void setSize(int x1, int y1, int x2, int y2);
	void setText(const char *text);
	void setTextPosition(int x, int y);
	void setColor(int bgColor, int textColor, int bgColorHighlighted, int textColorHightlighted);
	void setHighlight(bool isHighlighted);

	Common::Point _textPosition;
	char _text[128];
	int _bgColor;
	int _textColor;
	int _bgColorNormal;
	int _bgColorHighlighted;
	int _textColorNormal;
	int _textColorHighlighted;
	bool _isHighlighted;
};

class GameManager {
public:
	GameManager(SupernovaEngine *vm);
	~GameManager();

	void processInput(Common::KeyState &state);
	void processInput();
	void executeRoom();

	SupernovaEngine *_vm;
	Common::KeyState _key;
	Common::EventType _mouseClickType;
	bool _mouseClicked;
	bool _keyPressed;
	int _mouseX;
	int _mouseY;
	int _mouseField;
	Room *_currentRoom;
	Room *_rooms[kRoomsNum];
	Inventory _inventory;
	GameState _state;
	int _status;
	bool _processInput;
	bool _guiEnabled;
	bool _animationEnabled;
	Action _inputVerb;
	Object *_currentInputObject;
	Object *_inputObject[2];
	bool _waitEvent;
	bool _newRoom;
	bool _newOverlay;
	int _timer1;
	int _animationTimer;
	int _inventoryScroll;
	int _exitList[25];
	GuiElement _guiCommandButton[10];
	GuiElement _guiInventory[8];
	// 0 PC Speaker | 1 SoundBlaster | 2 No Sound
	int _soundDevice;

	void takeObject(Object &obj);

	void initState();
	void initRooms();
	void destroyRooms();
	void initGui();
	bool genericInteract(Action verb, Object &obj1, Object &obj2);
	bool isHelmetOff();
	void great(uint number);
	bool airless();
	void shock();
	Common::EventType getMouseInput();
	uint16 getKeyInput(bool blockForPrintChar = false);
	void getInput();
	void mouseInput3();
	void mouseWait(int delay);
	void wait2(int ticks);
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
	void edit(char *text, int x, int y, uint length);
	int invertSection(int section);
	void drawMapExits();
	void drawStatus();
	void drawCommandBox();
	void inventory_arrow(int num, bool brightness);
	void drawInventory();
	void drawImage(int section);
	void changeRoom(RoomID id);
	void resetInputState();
	void handleInput();
	void handleTime();
	void setAnimationTimer(int ticks);
	void death(const char *message);
	int  dialog(int num, byte *rowLength[6], const char **text[6], int number);
	void sentence(int number, bool brightness);
	void removeSentence(int sentence, int number);
	void addSentence(int sentence, int number);
	void say(const char *text);
	void reply(const char *text, int aus1, int aus2);
	void shipStart();
	void shot(int a, int b);
	void takeMoney(int amount);
	void search(int time);
	void startSearch();
	void guardNoticed();
	void guardReturned();
	void busted(int i);
	void corridorOnEntrance();
	void event(int time);
	void telomat(int number);
	void taxi();
	void outro();
};

}

#endif // STATE_H
