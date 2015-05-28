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

#ifndef SHERLOCK_UI_H
#define SHERLOCK_UI_H

#include "common/scummsys.h"
#include "common/events.h"
#include "sherlock/surface.h"
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

class UserInterface {
protected:
	SherlockEngine *_vm;

	UserInterface(SherlockEngine *vm);
public:
	MenuMode _menuMode;
	int _menuCounter;
	bool _infoFlag;
	bool _windowOpen;
	bool _endKeyActive;
	int _invLookFlag;
	bool _slideWindows;
	bool _helpStyle;
	Common::Rect _windowBounds;
	bool _lookScriptFlag;

	// TODO: Not so sure these should be in the base class. May want to refactor them to SherlockEngine, or refactor
	// various Scalpel dialogs to keep their own private state of key/selections
	char _key, _oldKey;
	int _selector, _oldSelector;
	int _temp, _oldTemp;
	int _temp1;
	int _lookHelp;
public:
	static UserInterface *init(SherlockEngine *vm);
	virtual ~UserInterface() {}

	/**
	 * Resets the user interface
	 */
	virtual void reset() {}

	/**
	 * Draw the user interface onto the screen's back buffers
	 */	
	virtual void drawInterface(int bufferNum = 3) {}

	/**
	 * Main input handler for the user interface
	 */
	virtual void handleInput() {}

	/**
	 * Displays a passed window by gradually scrolling it vertically on-screen
	 */
	virtual void summonWindow(const Surface &bgSurface, bool slideUp = true) {}

	/**
	 * Slide the window stored in the back buffer onto the screen
	 */
	virtual void summonWindow(bool slideUp = true, int height = CONTROLS_Y) {}

	/**
	 * Close a currently open window
	 * @param flag	0 = slide old window down, 1 = slide prior UI back up
	 */
	virtual void banishWindow(bool slideUp = true) {}

	/**
	 * Clears the info line of the screen
	 */
	virtual void clearInfo() {}

	/**
	 * Clear any active text window
	 */
	virtual void clearWindow() {}

	/**
	 * Print the previously selected object's decription
	 */
	virtual void printObjectDesc() {}
};

namespace Scalpel {

class ScalpelUserInterface: public UserInterface {
	friend class Inventory;
	friend class Settings;
	friend class Talk;
private:
	ImageFile *_controlPanel;
	ImageFile *_controls;
	char _keyPress;
	int _lookHelp;
	int _bgFound, _oldBgFound;
	int _help, _oldHelp;
	char _key, _oldKey;
	int _temp, _oldTemp;
	int _oldLook;
	bool _keyboardInput;
	bool _pause;
	int _cNum;
	Common::String _cAnimStr;
	Common::String _descStr;
	int _find;
	int _oldUse;
private:
	/**
	 * Draws the image for a user interface button in the down/pressed state.
	 */
	void depressButton(int num);

	/**
	 * If he mouse button is pressed, then calls depressButton to draw the button
	 * as pressed; if not, it will show it as released with a call to "restoreButton".
	 */
	void pushButton(int num);

	/**
	 * By the time this method has been called, the graphics for the button change
	 * have already been drawn. This simply takes care of switching the mode around
	 * accordingly
	 */
	void toggleButton(int num);

	/**
	 * Creates a text window and uses it to display the in-depth description
	 * of the highlighted object
	 */
	void examine();

	/**
	 * Print the name of an object in the scene
	 */
	void lookScreen(const Common::Point &pt);

	/**
	 * Gets the item in the inventory the mouse is on and display's it's description
	 */
	void lookInv();

	/**
	 * Handles input when the file list window is being displayed
	 */
	void doEnvControl();
	
	/**
	 * Handle input whilst the inventory is active
	 */
	void doInvControl();
	
	/**
	 * Handles waiting whilst an object's description window is open.
	 */
	void doLookControl();
	
	/**
	 * Handles input until one of the user interface buttons/commands is selected
	 */
	void doMainControl();
	
	/**
	 * Handles the input for the MOVE, OPEN, and CLOSE commands
	 */
	void doMiscControl(int allowed);
	
	/**
	 * Handles input for picking up items
	 */
	void doPickControl();
	
	/**
	 * Handles input when in talk mode. It highlights the buttons and available statements,
	 * and handles allowing the user to click on them
	 */
	void doTalkControl();
	
	/**
	 * Handles events when the Journal is active.
	 * @remarks		Whilst this would in theory be better in the Journal class, since it displays in
	 *		the user interface, it uses so many internal UI fields, that it sort of made some sense
	 *		to put it in the UserInterface class.
	 */
	void journalControl();

	/**
	 * Checks to see whether a USE action is valid on the given object
	 */
	void checkUseAction(const UseType *use, const Common::String &invName, const char *const messages[],
		int objNum, bool giveMode);
	
	/**
	 * Called for OPEN, CLOSE, and MOVE actions are being done
	 */
	void checkAction(ActionType &action, const char *const messages[], int objNum);

	/**
	 * Print the previously selected object's decription
	 */
	void printObjectDesc(const Common::String &str, bool firstTime);
public:
	ScalpelUserInterface(SherlockEngine *vm);
	virtual ~ScalpelUserInterface();

	/**
	 * Handles counting down whilst checking for input, then clears the info line.
	 */
	void whileMenuCounter();

	/**
	 * Draws the image for the given user interface button in the up
	 * (not selected) position
	 */
	void restoreButton(int num);
public:
	/**
	 * Resets the user interface
	 */
	virtual void reset();

	/**
	 * Main input handler for the user interface
	 */
	virtual void handleInput();

	/**
	 * Draw the user interface onto the screen's back buffers
	 */	
	virtual void drawInterface(int bufferNum = 3);

	/**
	 * Displays a passed window by gradually scrolling it vertically on-screen
	 */
	virtual void summonWindow(const Surface &bgSurface, bool slideUp = true);

	/**
	 * Slide the window stored in the back buffer onto the screen
	 */
	virtual void summonWindow(bool slideUp = true, int height = CONTROLS_Y);

	/**
	 * Close a currently open window
	 * @param flag	0 = slide old window down, 1 = slide prior UI back up
	 */
	virtual void banishWindow(bool slideUp = true);

	/**
	 * Clears the info line of the screen
	 */
	virtual void clearInfo();

	/**
	 * Clear any active text window
	 */
	virtual void clearWindow();

	/**
	 * Print the previously selected object's decription
	 */	
	virtual void printObjectDesc();
};

} // End of namespace Scalpel

namespace Tattoo {

class TattooUserInterface : public UserInterface {
private:
	Common::Rect _menuBounds;
	Common::Rect _oldMenuBounds;
	Common::Rect _invMenuBounds;
	Common::Rect _oldInvMenuBounds;
	Common::Rect _oldTagBounds;
	Common::Rect _oldInvGraphicBounds;
	Surface *_menuBuffer;
	Surface *_invMenuBuffer;
public:
	TattooUserInterface(SherlockEngine *vm);

	void doBgAnimRestoreUI();
public:
	virtual ~TattooUserInterface() {}

	/**
	 * Main input handler for the user interface
	 */
	virtual void handleInput();
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
