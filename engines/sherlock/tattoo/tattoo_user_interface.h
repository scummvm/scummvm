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
#include "sherlock/user_interface.h"
#include "sherlock/tattoo/widget_tooltip.h"
#include "sherlock/tattoo/widget_verbs.h"

namespace Sherlock {

namespace Tattoo {

class TattooUserInterface : public UserInterface {
private:
	Common::Rect _menuBounds;
	Common::Rect _oldMenuBounds;
	Common::Rect _invMenuBounds;
	Common::Rect _oldInvMenuBounds;
	Common::Rect _invGraphicBounds;
	Common::Rect _oldInvGraphicBounds;
	Surface *_menuBuffer;
	Surface *_invMenuBuffer;
	Surface *_invGraphic;
	Common::Array<Common::Rect> _grayAreas;
	Object *_bgShape;
	bool _personFound;
	int _lockoutTimer;
	Common::KeyState _keyState;
	SaveMode _fileMode;
	int _exitZone;
	int _scriptZone;
	int _activeObj;
	WidgetTooltip _tooltipWidget;
	WidgetVerbs _verbsWidget;
private:
	/**
	 * Draws designated areas of the screen that are meant to be grayed out using grayscale colors
	 */
	void drawGrayAreas();

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
	 * Handle input if an inventory command (INVENT, LOOK, or USE) has an open window and is active
	 */
	void doInventoryControl();

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
	 * Handles input when a message window is open at the bottom of the screen
	 */
	void doMessageControl();
	
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
	 * Turn off any active object description text
	 */
	void turnTextOff();

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
	 * Handle displaying the quit menu
	 */
	void doQuitMenu();

	/**
	 * Turn on the command menu showing available actions that can be done on a given item
	 */
	void activateVerbMenu(bool objectsOn);

	/**
	 * Display the long description for an object stored in it's _examine field, in a window that
	 * will be shown at the bottom of the screen
	 */
	void lookAtObject();
public:
	Common::Point _currentScroll, _targetScroll;
	int _scrollSize, _scrollSpeed;
	bool _drawMenu;
	int _bgFound, _oldBgFound;
	int _arrowZone, _oldArrowZone;
public:
	TattooUserInterface(SherlockEngine *vm);

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
public:
	virtual ~TattooUserInterface() {}

	/**
	 * Main input handler for the user interface
	 */
	virtual void handleInput();

	/**
	 * Draw the user interface onto the screen's back buffers
	 */	
	virtual void drawInterface(int bufferNum = 3);
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
