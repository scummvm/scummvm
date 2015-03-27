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
#include "sherlock/resources.h"

namespace Sherlock {

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

class SherlockEngine;

class UserInterface {
private:
	SherlockEngine *_vm;
	int _bgFound;
	int _oldBgFound;
	Common::KeyCode _keycode;
	int _helpStyle;
	int _lookHelp;
	int _help, _oldHelp;
	int _key, _oldKey;
	int _temp, _oldTemp;
	int _invLookFlag;
	ImageFile *_controls;
	int _oldLook;
	bool _keyboardInput;
	int _invMode;
	bool _pause;
	int _cNum;
	int _selector, _oldSelector;
	Common::String _cAnimStr;
	bool _lookScriptFlag;
	Common::Rect _windowBounds;
	Common::String _descStr;
	int _windowStyle;
private:
	void depressButton(int num);

	void restoreButton(int num);

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

	void makeButton(const Common::Rect &bounds, int textX, const Common::String &str);
public:
	MenuMode _menuMode;
	int _menuCounter;
	bool _infoFlag;
	bool _windowOpen;
public:
	UserInterface(SherlockEngine *vm);
	~UserInterface();

	void reset();

	void handleInput();

	void clearInfo();

	void whileMenuCounter();

	void printObjectDesc(const Common::String &str, bool firstTime);
	void printObjectDesc();

	void summonWindow(const Surface &bgSurface);
	void banishWindow(bool flag);
};

} // End of namespace Sherlock

#endif
