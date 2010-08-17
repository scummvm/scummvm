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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/mouse.h"
#include "hugo/inventory.h"
#include "hugo/parser.h"

namespace Hugo {

#define MAX_DISP       (XPIX / INV_DX)              // Max icons displayable

InventoryHandler::InventoryHandler(HugoEngine &vm) : _vm(vm) {
}

// Construct the inventory scrollbar in dib_i
// imageTotNumb is total number of inventory icons
// displayNumb is number requested for display
// scrollFl is TRUE if scroll arrows required
// firstObjId is index of first (scrolled) inventory object to display
void InventoryHandler::constructInventory(int16 imageTotNumb, int displayNumb, bool scrollFl, int16 firstObjId) {
	int16 ux, uy, ix;                           // Coordinates of icons

	debugC(1, kDebugInventory, "constructInventory(%d, %d, %d, %d)", imageTotNumb, displayNumb, (scrollFl) ? 0 : 1, firstObjId);

	// Clear out icon buffer
	memset(_vm.screen().getIconBuffer(), 0, sizeof(_vm.screen().getIconBuffer()));

	// If needed, copy arrows - reduce number of icons displayable
	if (scrollFl) { // Display at first and last icon positions
		_vm.screen().moveImage(_vm.screen().getGUIBuffer(), 0, 0, INV_DX, INV_DY, XPIX, _vm.screen().getIconBuffer(), 0, 0, XPIX);
		_vm.screen().moveImage(_vm.screen().getGUIBuffer(), INV_DX, 0, INV_DX, INV_DY, XPIX, _vm.screen().getIconBuffer(), INV_DX *(MAX_DISP - 1), 0, XPIX);
		displayNumb = MIN(displayNumb, MAX_DISP - NUM_ARROWS);
	} else  // No, override first index - we can show 'em all!
		firstObjId = 0;

	// Copy inventory icons to remaining positions
	int16 displayed = 0;
	int16 carried = 0;
	for (int16 i = 0; i < imageTotNumb; i++) {
		if (_vm._objects[_vm._invent[i]].carriedFl) {
			// Check still room to display and past first scroll index
			if (displayed < displayNumb && carried >= firstObjId) {
				// Compute source coordinates in dib_u
				ux = (i + NUM_ARROWS) * INV_DX % XPIX;
				uy = (i + NUM_ARROWS) * INV_DX / XPIX * INV_DY;

				// Compute dest coordinates in dib_i
				ix = ((scrollFl) ? displayed + 1 : displayed) * INV_DX;
				displayed++;        // Count number displayed

				// Copy the icon
				_vm.screen().moveImage(_vm.screen().getGUIBuffer(), ux, uy, INV_DX, INV_DY, XPIX, _vm.screen().getIconBuffer(), ix, 0, XPIX);
			}
			carried++;                              // Count number carried
		}
	}
}

// Process required action for inventory
// Returns objId under cursor (or -1) for INV_GET
int16 InventoryHandler::processInventory(invact_t action, ...) {
	static int16 firstIconId = 0;                   // Index of first icon to display
	int16 i, j;
	int16 objId = -1;                               // Return objid under cursor
	int16 imageNumb;                                // Total number of inventory items
	int displayNumb;                                // Total number displayed/carried
	int16 cursorx, cursory;                         // Current cursor position
	bool scrollFl;                                  // TRUE if scroll arrows needed
	va_list marker;                                 // Args used for D_ADD operation

	debugC(1, kDebugInventory, "processInventory(invact_t action, ...)");

	// Compute total number and number displayed, i.e. number carried
	for (imageNumb = 0, displayNumb = 0; imageNumb < _vm._maxInvent && _vm._invent[imageNumb] != -1; imageNumb++)
		if (_vm._objects[_vm._invent[imageNumb]].carriedFl)
			displayNumb++;

	// Will we need the scroll arrows?
	scrollFl = displayNumb > MAX_DISP;

	switch (action) {
	case INV_INIT:                                  // Initialize inventory display
		constructInventory(imageNumb, displayNumb, scrollFl, firstIconId);
		break;
	case INV_LEFT:                                  // Scroll left by one icon
		firstIconId = MAX(0, firstIconId - 1);
		constructInventory(imageNumb, displayNumb, scrollFl, firstIconId);
		break;
	case INV_RIGHT:                                 // Scroll right by one icon
		firstIconId = MIN(displayNumb, firstIconId + 1);
		constructInventory(imageNumb, displayNumb, scrollFl, firstIconId);
		break;
	case INV_GET:                                   // Return object id under cursor
		// Get cursor position from variable argument list
		va_start(marker, action);                   // Initialize variable arguments
		cursorx = va_arg(marker, int);              // Cursor x
		cursory = va_arg(marker, int);              // Cursor y
		va_end(marker);                             // Reset variable arguments

		cursory -= DIBOFF_Y;                        // Icon bar is at true zero
		if (cursory > 0 && cursory < INV_DY) {      // Within icon bar?
			i = cursorx / INV_DX;                   // Compute icon index
			if (scrollFl) {                          // Scroll buttons displayed
				if (i == 0)                         // Left scroll button
					objId = LEFT_ARROW;
				else {
					if (i == MAX_DISP - 1)          // Right scroll button
						objId = RIGHT_ARROW;
					else                            // Adjust for scroll
						i += firstIconId - 1;       // i is icon index
				}
			}

			// If not an arrow, find object id - limit to valid range
			if (objId == -1 && i < displayNumb)
				// Find objid by counting # carried objects == i+1
				for (j = 0, i++; i > 0 && j < _vm._numObj; j++)
					if (_vm._objects[j].carriedFl)
						if (--i == 0)
							objId = j;
		}
		break;
	}
	return objId;               // For the INV_GET action
}

void InventoryHandler::runInventory() {
	status_t &gameStatus = _vm.getGameStatus();

	debugC(1, kDebugInventory, "runInventory");

// Process inventory state machine
	switch (gameStatus.inventoryState) {
	case I_OFF:                                     // Icon bar off screen
		break;
	case I_UP:                                      // Icon bar moving up
		gameStatus.inventoryHeight -= STEP_DY;      // Move the icon bar up
		if (gameStatus.inventoryHeight <= 0)        // Limit travel
			gameStatus.inventoryHeight = 0;

		// Move visible portion to _frontBuffer, restore uncovered portion, display results
		_vm.screen().moveImage(_vm.screen().getIconBuffer(), 0, 0, XPIX, gameStatus.inventoryHeight, XPIX, _vm.screen().getFrontBuffer(), 0, DIBOFF_Y, XPIX);
		_vm.screen().moveImage(_vm.screen().getBackBufferBackup(), 0, gameStatus.inventoryHeight + DIBOFF_Y, XPIX, STEP_DY, XPIX, _vm.screen().getFrontBuffer(), 0, gameStatus.inventoryHeight + DIBOFF_Y, XPIX);
		_vm.screen().displayRect(0, DIBOFF_Y, XPIX, gameStatus.inventoryHeight + STEP_DY);

		if (gameStatus.inventoryHeight == 0) {      // Finished moving up?
			// Yes, restore dibs and exit back to game state machine
			_vm.screen().moveImage(_vm.screen().getBackBufferBackup(), 0, 0, XPIX, YPIX, XPIX, _vm.screen().getBackBuffer(), 0, 0, XPIX);
			_vm.screen().moveImage(_vm.screen().getBackBuffer(), 0, 0, XPIX, YPIX, XPIX, _vm.screen().getFrontBuffer(), 0, 0, XPIX);
			_vm.updateImages();                     // Add objects back into display list for restore
			gameStatus.inventoryState = I_OFF;
			gameStatus.viewState = V_PLAY;
		}
		break;
	case I_DOWN:                                    // Icon bar moving down
		// If this is the first step, initialize dib_i
		// and get any icon/text out of _frontBuffer
		if (gameStatus.inventoryHeight == 0) {
			processInventory(INV_INIT);             // Initialize dib_i
			_vm.screen().displayList(D_RESTORE);    // Restore _frontBuffer
			_vm.updateImages();                     // Rebuild _frontBuffer without icons/text
			_vm.screen().displayList(D_DISPLAY);    // Blit display list to screen
		}

		gameStatus.inventoryHeight += STEP_DY;      // Move the icon bar down
		if (gameStatus.inventoryHeight >= INV_DY)   // Limit travel
			gameStatus.inventoryHeight = INV_DY;

		// Move visible portion to _frontBuffer, display results
		_vm.screen().moveImage(_vm.screen().getIconBuffer(), 0, 0, XPIX, gameStatus.inventoryHeight, XPIX, _vm.screen().getFrontBuffer(), 0, DIBOFF_Y, XPIX);
		_vm.screen().displayRect(0, DIBOFF_Y, XPIX, gameStatus.inventoryHeight);

		if (gameStatus.inventoryHeight == INV_DY) { // Finished moving down?
			// Yes, prepare view dibs for special inventory display since
			// we can't refresh objects while icon bar overlayed...
			// 1. Save backing store _backBuffer in temporary dib_c
			// 2. Make snapshot of _frontBuffer the new _backBuffer backing store
			// 3. Reset the display list
			_vm.screen().moveImage(_vm.screen().getBackBuffer(), 0, 0, XPIX, YPIX, XPIX, _vm.screen().getBackBufferBackup(), 0, 0, XPIX);
			_vm.screen().moveImage(_vm.screen().getFrontBuffer(), 0, 0, XPIX, YPIX, XPIX, _vm.screen().getBackBuffer(), 0, 0, XPIX);
			_vm.screen().displayList(D_INIT);
			gameStatus.inventoryState = I_ACTIVE;
		}
		break;
	case I_ACTIVE:                                  // Inventory active
		_vm.parser().charHandler();                 // Still allow commands
		_vm.screen().displayList(D_RESTORE);        // Restore previous background
		_vm.mouse().mouseHandler();                 // Mouse activity - adds to display list
		_vm.screen().displayList(D_DISPLAY);        // Blit the display list to screen
		break;
	}
}

} // End of namespace Hugo
