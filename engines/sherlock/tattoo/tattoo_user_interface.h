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

#ifndef SHERLOCK_TATTOO_UI_H
#define SHERLOCK_TATTOO_UI_H

#include "common/scummsys.h"
#include "sherlock/saveload.h"
#include "sherlock/screen.h"
#include "sherlock/user_interface.h"
#include "sherlock/tattoo/widget_inventory.h"
#include "sherlock/tattoo/widget_text.h"
#include "sherlock/tattoo/widget_tooltip.h"
#include "sherlock/tattoo/widget_verbs.h"

namespace Sherlock {

namespace Tattoo {

#define BUTTON_SIZE 15				// Button width/height

class WidgetBase;

class TattooUserInterface : public UserInterface {
	friend class WidgetBase;
private:
	int _lockoutTimer;
	SaveMode _fileMode;
	int _exitZone;
	int _scriptZone;
	int _cAnimFramePause;
	WidgetInventory _inventoryWidget;
	WidgetSceneTooltip _tooltipWidget;
	WidgetVerbs _verbsWidget;
	WidgetMessage _messageWidget;
	WidgetBase *_widget;
	byte _lookupTable[PALETTE_COUNT];
	byte _lookupTable1[PALETTE_COUNT];
private:
	/**
	 * Handle any input when we're in standard mode (with no windows open)
	 */
	void doStandardControl();

	/**
	 * Handle input when in look mode
	 */
	void doLookControl();

	/**
	 * Handle input when the File window is open
	 */
	void doFileControl();

	/**
	 * Handle input while the verb menu is open
	 */
	void doVerbControl();

	/**
	 * Handles input when in talk mode. It highlights the buttons and response statements,
	 * and handles any actions for clicking on the buttons or statements.
	 */
	void doTalkControl();
	
	/**
	 * Handles input when the player is in the Lab Table scene
	 */
	void doLabControl();

	/**
	 * If the mouse cursor is point at the cursor, then display the name of the object on the screen.
	 * If there is no object being pointed it, clear any previously displayed name
	 */
	void displayObjectNames();

	/**
	 * Set up to display the Files menu
	 */
	void initFileMenu();

	/**
	 * Handle displaying the quit menu
	 */
	void doQuitMenu();

	/**
	 * Free any active menu
	 */
	void freeMenu();
public:
	Common::Point _currentScroll, _targetScroll;
	int _scrollSize, _scrollSpeed;
	bool _drawMenu;
	int _arrowZone, _oldArrowZone;
	Object *_bgShape;
	bool _personFound;
	int _activeObj;
	Common::KeyState _keyState;
	Common::Point _lookPos;
	int _scrollHighlight;
	ImageFile *_mask, *_mask1;
	Common::Point _maskOffset;
	int _maskCounter;
	ImageFile *_interfaceImages;
	WidgetText _textWidget;
	Common::String _action;
public:
	TattooUserInterface(SherlockEngine *vm);
	virtual ~TattooUserInterface();

	/**
	 * Handles restoring any areas of the back buffer that were/are covered by UI elements
	 */
	void doBgAnimRestoreUI();

	/**
	 * Checks to see if the screen needs to be scrolled. If so, scrolls it towards the target position
	 */
	void doScroll();

	/**
	 * Initializes scroll variables
	 */
	void initScrollVars();

	/**
	 * Display the long description for an object in a window
	 */
	void lookAtObject();

	/**
	 * Display the passed long description for an object. If the flag firstTime is set,
	 * the window will be opened to accomodate the text. Otherwise, the remaining text
	 * will be printed in an already open window
	 */
	void printObjectDesc(const Common::String &str, bool firstTime);

	/**
	 * Handles displaying the journal
	 */
	void doJournal();

	/**
	 * Put the game in inventory mode by opening the inventory dialog
	 */
	void doInventory(int mode);

	/**
	 * Handle the display of the options/setup menu
	 */
	void doControls();

	/**
	 * Pick up the selected object
	 */
	void pickUpObject(int objNum);

	/**
	 * This will display a text message in a dialog at the bottom of the screen
	 */
	void putMessage(const char *formatStr, ...) GCC_PRINTF(2, 3);

	/**
	 * Makes a greyscale translation table for each palette entry in the table
	 */
	void setupBGArea(const byte cMap[PALETTE_SIZE]);

	/**
	 * Erase any background as needed before drawing frame
	 */
	void doBgAnimEraseBackground();

	void drawMaskArea(bool mode);

	/**
	 * Translate a given area of the back buffer to greyscale shading
	 */
	void makeBGArea(const Common::Rect &r);

	/**
	 * Draws all the dialog rectangles for any items that need them
	 */
	void drawDialogRect(Surface &s, const Common::Rect &r, bool raised);
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
	 * Clear any active text window
	 */
	virtual void clearWindow();

	/**
	 * Banish any active window
	 */
	virtual void banishWindow();
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
