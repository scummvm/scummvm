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
#include "common/list.h"
#include "sherlock/saveload.h"
#include "sherlock/screen.h"
#include "sherlock/user_interface.h"
#include "sherlock/tattoo/widget_credits.h"
#include "sherlock/tattoo/widget_files.h"
#include "sherlock/tattoo/widget_inventory.h"
#include "sherlock/tattoo/widget_options.h"
#include "sherlock/tattoo/widget_quit.h"
#include "sherlock/tattoo/widget_text.h"
#include "sherlock/tattoo/widget_tooltip.h"
#include "sherlock/tattoo/widget_verbs.h"

namespace Sherlock {

namespace Tattoo {

// Button width/height
#define BUTTON_SIZE 15
// How long to play the intro before it can be skipped
#define STARTUP_KEYS_DISABLED_DELAY 200

class WidgetBase;

enum ScrollHighlight { SH_NONE = 0, SH_SCROLL_UP = 1, SH_PAGE_UP = 2, SH_THUMBNAIL = 3, SH_PAGE_DOWN = 4, SH_SCROLL_DOWN = 5 };

class WidgetList : public Common::List <WidgetBase *> {
public:
	bool contains(const WidgetBase *item) const;
};

class TattooUserInterface : public UserInterface {
	friend class WidgetBase;
private:
	int _scriptZone;
	int _cAnimFramePause;
	WidgetInventory _inventoryWidget;
	WidgetMessage _messageWidget;
	WidgetQuit _quitWidget;
	WidgetList _fixedWidgets;
	WidgetList _widgets;
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
	 * Free any active menu
	 */
	void freeMenu();
public:
	Common::Point _targetScroll;
	int _scrollSize, _scrollSpeed;
	bool _drawMenu;
	int _arrowZone, _oldArrowZone;
	Object *_bgShape;
	bool _personFound;
	int _activeObj;
	Common::KeyState _keyState;
	Common::Point _lookPos;
	ScrollHighlight _scrollHighlight;
	Common::SeekableReadStream *_mask, *_mask1;
	Common::Point _maskOffset;
	int _maskCounter;
	int _lockoutTimer;
	ImageFile *_interfaceImages;
	WidgetCredits _creditsWidget;
	WidgetOptions _optionsWidget;
	WidgetText _textWidget;
	WidgetSceneTooltip _tooltipWidget;
	WidgetVerbs _verbsWidget;
	WidgetList _postRenderWidgets;
public:
	TattooUserInterface(SherlockEngine *vm);
	~TattooUserInterface() override;

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
	 * Handle the display of the quit menu
	 */
	void doQuitMenu();

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

	/**
	 * Draws overlays onto the scene. Basically, the smoke effects some scenes have
	 */
	void drawMaskArea(bool mode);

	/**
	 * Takes the data passed in the image and apply it to the surface at the given position.
	 * The src mask data is encoded with a different color for each item. To highlight one,
	 the runs that do not match the highlight number will be darkened
	 */
	void maskArea(Common::SeekableReadStream &mask, const Common::Point &pt);

	/**
	 * Translate a given area of the back buffer to greyscale shading
	 */
	void makeBGArea(const Common::Rect &r);

	/**
	 * Draws all the dialog rectangles for any items that need them
	 */
	void drawDialogRect(Surface &s, const Common::Rect &r, bool raised);

	/**
	 * If the mouse cursor is point at the cursor, then display the name of the object on the screen.
	 * If there is no object being pointed it, clear any previously displayed name
	 */
	void displayObjectNames();

	/**
	 * Show the load game dialog, and allow the user to load a game
	 */
	void loadGame();

	/**
	 * Show the save game dialog, and allow the user to save the game
	 */
	void saveGame();

	/**
	 * Add a widget to the current scene to be executed if there are no active widgets in the
	 * main _widgets list
	 */
	void addFixedWidget(WidgetBase *widget);
public:
	/**
	 * Resets the user interface
	 */
	void reset() override;

	/**
	 * Main input handler for the user interface
	 */
	void handleInput() override;

	/**
	 * Draw the user interface onto the screen's back buffers
	 */
	void drawInterface(int bufferNum = 3) override;

	/**
	 * Clear any active text window
	 */
	void clearWindow() override;

	/**
	 * Banish any active window
	 * @remarks		Tattoo doesn't use sliding windows, but the parameter is in the base
	 * UserInterface class as a convenience for Scalpel UI code
	 */
	void banishWindow(bool slideUp = true) override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
