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

#ifndef SUPERNOVA_GAME_MANAGER_H
#define SUPERNOVA_GAME_MANAGER_H

#include "common/events.h"
#include "common/rect.h"
#include "common/keyboard.h"
#include "common/error.h"
#include "supernova/room.h"
#include "supernova/sound.h"

namespace Supernova {

const int32 kMaxTimerValue = 0x7FFFFFFF;

enum EventFunction { kNoFn, kSupernovaFn, kGuardReturnedFn, kGuardWalkFn, kTaxiFn, kSearchStartFn, kSoberFn, kPyramidEndFn, kCaughtFn};


class Inventory {
public:
	Inventory(Object *nullObject, int &inventoryScroll)
		: _numObjects(0)
		, _nullObject(nullObject)
	    , _inventoryScroll(inventoryScroll) {
		for (int i = 0; i < kMaxCarry; ++i)
			_inventory[i] = nullptr;
	}

	void add(Object &obj);
	void remove(Object &obj);
	void clear();
	Object *get(int index) const;
	Object *get(ObjectId id) const;
	int getSize() const { return _numObjects; }

private:
	Object *_inventory[kMaxCarry];
	Object *_nullObject;
	int &_inventoryScroll;
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

	const char *getText() const {
		return _text;
	}
	int getBackgroundColor() const {
		return _bgColor;
	}
	int getTextColor() const {
		return _textColor;
	}
	const Common::Point &getTextPos() const {
		return _textPosition;
	}
	bool isHighlighted() const {
		return _isHighlighted;
	}

private:
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
	GameManager(SupernovaEngine *vm, Sound *sound);
	virtual ~GameManager();

	virtual void updateEvents();
	void processInput(Common::KeyState &state);
	void processInput();
	virtual void executeRoom();
	virtual bool serialize(Common::WriteStream *out);
	virtual bool deserialize(Common::ReadStream *in, int version);

	static int guiCommands[];
	static int guiStatusCommands[];
	SupernovaEngine *_vm;
	Sound *_sound;
	Common::KeyState _key;
	Common::EventType _mouseClickType;
	bool _mouseClicked;
	bool _keyPressed;
	int _mouseX;
	int _mouseY;
	int _mouseField;
	Room *_currentRoom;
	Room *_lastRoom;
	bool _newRoom;
	Room **_rooms;
	Inventory _inventory;
	bool _processInput;
	bool _guiEnabled;
	bool _animationEnabled;
	byte _roomBrightness;
	Action _inputVerb;
	Object _nullObject;
	Object *_currentInputObject;
	Object *_inputObject[2];
	int32 _oldTime;
	uint _timePaused;
	bool _timerPaused;
	int32 _messageDuration;
	int32 _animationTimer;
	int _inventoryScroll;
	int _exitList[25];
	GuiElement _guiCommandButton[10];
	GuiElement _guiInventory[8];
	GuiElement _guiInventoryArrow[2];
	// Dialog
	int _currentSentence;
	int _sentenceNumber[6];
	int _texts[6];
	byte _rows[6];
	byte _rowsStart[6];
	int32 _time;
	bool _dead;

	void takeObject(Object &obj);
	void setObjectNull(Object *&obj);
	bool isNullObject(Object *obj);

	virtual void initState();
	virtual void initRooms();
	virtual void destroyRooms();
	void initGui();
	virtual bool canSaveGameStateCurrently();
	virtual bool genericInteract(Action verb, Object &obj1, Object &obj2);
	void getInput(bool onlyKeys = false);
	void wait(int ticks, bool checkInput = false);
	bool waitOnInput(int ticks, Common::KeyCode &keycode);
	void screenShake();
	virtual void roomBrightness();
	void showMenu();
	void animationOff();
	void animationOn();
	void edit(Common::String &input, int x, int y, uint length);
	int invertSection(int section);
	virtual void drawMapExits();
	void drawStatus();
	void drawCommandBox();
	void drawInventory();
	void drawGUI();
	void changeRoom(RoomId id);
	void resetInputState();
	virtual void handleInput();
	virtual void handleTime();
	void pauseTimer(bool pause);
	virtual void loadTime();
	virtual void saveTime();
	void setAnimationTimer(int ticks);
	void dead(int messageId);
	int  dialog(int num, byte rowLength[6], int text[6], int number);
	void sentence(int number, bool brightness);
	void say(int textId);
	void say(const char *text);
	void reply(int textId, int aus1, int aus2);
	void reply(const char *text, int aus1, int aus2);
	void mousePosDialog(int x, int y);
	virtual void takeMoney(int amount);
};

}

#endif // SUPERNOVA_GAME_MANAGER_H
