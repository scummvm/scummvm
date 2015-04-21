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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SHERLOCK_UI_H
#define SHERLOCK_UI_H

#include "common/scummsys.h"
#include "common/events.h"
#include "sherlock/graphics.h"
#include "sherlock/objects.h"
#include "sherlock/resources.h"

namespace Sherlock {

#define CONTROLS_Y  138
#define CONTROLS_Y1 151

enum MenuMode {
	STD_MODE		=  0,
	LOOK_MODE		=  1,
	MOVE_MODE		=  2,
	TALK_MODE		=  3,
	PICKUP_MODE		=  4,
	OPEN_MODE		=  5,
	CLOSE_MODE		=  6,
	INV_MODE		=  7,
	USE_MODE		=  8,
	GIVE_MODE		=  9,
	JOURNAL_MODE	= 10,
	FILES_MODE		= 11,
	SETUP_MODE		= 12
};

extern const char COMMANDS[13];
extern const int MENU_POINTS[12][4];

extern const int INVENTORY_POINTS[8][3];
extern const char INVENTORY_COMMANDS[9];
extern const char *const PRESS_KEY_FOR_MORE;
extern const char *const PRESS_KEY_TO_CONTINUE;

class SherlockEngine;
class Inventory;
class Talk;
class UserInterface;

class Settings {
private:
	SherlockEngine *_vm;
public:
	Settings(SherlockEngine *vm) : _vm(vm) {}

	void drawInteface(bool flag);

	int drawButtons(const Common::Point &pt, int key);
};

class UserInterface {
	friend class Inventory;
	friend class Settings;
	friend class Talk;
private:
	SherlockEngine *_vm;
	ImageFile *_controlPanel;
	ImageFile *_controls;
	int _bgFound;
	int _oldBgFound;
	Common::KeyCode _keycode;
	int _helpStyle;
	int _lookHelp;
	int _help, _oldHelp;
	int _key, _oldKey;
	int _temp, _oldTemp;
	int _oldLook;
	bool _keyboardInput;
	bool _pause;
	int _cNum;
	int _selector, _oldSelector;
	Common::String _cAnimStr;
	bool _lookScriptFlag;
	Common::Rect _windowBounds;
	Common::String _descStr;
	int _windowStyle;
	int _find;
	int _oldUse;
private:
	void depressButton(int num);

	void pushButton(int num);

	void toggleButton(int num);

	void examine();

	void lookScreen(const Common::Point &pt);

	void lookInv();

	void doEnvControl();
	void doInvControl();
	void doLookControl();
	void doMainControl();
	void doMiscControl(int allowed);
	void doPickControl();
	void doTalkControl();
	void journalControl();

	void environment();
	void doControls();

	void checkUseAction(const UseType *use, const Common::String &invName, const char *const messages[],
		int objNum, int giveMode);
	void checkAction(ActionType &action, const char *const messages[], int objNum);
public:
	MenuMode _menuMode;
	int _menuCounter;
	bool _infoFlag;
	bool _windowOpen;
	bool _endKeyActive;
	int _invLookFlag;
	int _temp1;
public:
	UserInterface(SherlockEngine *vm);
	~UserInterface();

	void reset();

	void drawInterface(int bufferNum = 3);

	void handleInput();

	void clearInfo();
	void clearWindow();

	void whileMenuCounter();

	void printObjectDesc(const Common::String &str, bool firstTime);
	void printObjectDesc();

	void summonWindow(const Surface &bgSurface, bool slideUp = true);
	void summonWindow(bool slideUp = true, int height = CONTROLS_Y);
	void banishWindow(bool slideUp = true);

	void restoreButton(int num);
};

} // End of namespace Sherlock

#endif
