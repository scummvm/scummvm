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
#include "hugo/object.h"

namespace Hugo {

static const int kMaxDisp = (kXPix / kInvDx);       // Max icons displayable

InventoryHandler::InventoryHandler(HugoEngine *vm) : _vm(vm) {
}

/**
* Construct the inventory scrollbar in dib_i
* imageTotNumb is total number of inventory icons
* displayNumb is number requested for display
* scrollFl is TRUE if scroll arrows required
* firstObjId is index of first (scrolled) inventory object to display
*/
void InventoryHandler::constructInventory(int16 imageTotNumb, int displayNumb, bool scrollFl, int16 firstObjId) {
	debugC(1, kDebugInventory, "constructInventory(%d, %d, %d, %d)", imageTotNumb, displayNumb, (scrollFl) ? 0 : 1, firstObjId);

	// Clear out icon buffer
	memset(_vm->_screen->getIconBuffer(), 0, sizeof(_vm->_screen->getIconBuffer()));

	// If needed, copy arrows - reduce number of icons displayable
	if (scrollFl) { // Display at first and last icon positions
		_vm->_screen->moveImage(_vm->_screen->getGUIBuffer(), 0, 0, kInvDx, kInvDy, kXPix, _vm->_screen->getIconBuffer(), 0, 0, kXPix);
		_vm->_screen->moveImage(_vm->_screen->getGUIBuffer(), kInvDx, 0, kInvDx, kInvDy, kXPix, _vm->_screen->getIconBuffer(), kInvDx *(kMaxDisp - 1), 0, kXPix);
		displayNumb = MIN(displayNumb, kMaxDisp - kArrowNumb);
	} else  // No, override first index - we can show 'em all!
		firstObjId = 0;

	// Copy inventory icons to remaining positions
	int16 displayed = 0;
	int16 carried = 0;
	for (int16 i = 0; (i < imageTotNumb) && (displayed < displayNumb); i++) {
		if (_vm->_object->isCarried(_vm->_invent[i])) {
			// Check still room to display and past first scroll index
			if (displayed < displayNumb && carried >= firstObjId) {
				// Compute source coordinates in dib_u
				int16 ux = (i + kArrowNumb) * kInvDx % kXPix;
				int16 uy = (i + kArrowNumb) * kInvDx / kXPix * kInvDy;

				// Compute dest coordinates in dib_i
				int16 ix = ((scrollFl) ? displayed + 1 : displayed) * kInvDx;
				displayed++;                        // Count number displayed

				// Copy the icon
				_vm->_screen->moveImage(_vm->_screen->getGUIBuffer(), ux, uy, kInvDx, kInvDy, kXPix, _vm->_screen->getIconBuffer(), ix, 0, kXPix);
			}
			carried++;                              // Count number carried
		}
	}
}

/**
* Process required action for inventory
* Returns objId under cursor (or -1) for INV_GET
*/
int16 InventoryHandler::processInventory(invact_t action, ...) {
	debugC(1, kDebugInventory, "processInventory(invact_t action, ...)");

	static int16 firstIconId = 0;                   // Index of first icon to display

	int16 imageNumb;                                // Total number of inventory items
	int displayNumb;                                // Total number displayed/carried
	// Compute total number and number displayed, i.e. number carried
	for (imageNumb = 0, displayNumb = 0; imageNumb < _vm->_maxInvent && _vm->_invent[imageNumb] != -1; imageNumb++) {
		if (_vm->_object->isCarried(_vm->_invent[imageNumb]))
			displayNumb++;
	}

	// Will we need the scroll arrows?
	bool scrollFl = displayNumb > kMaxDisp;
	va_list marker;                                 // Args used for D_ADD operation
	int16 cursorx, cursory;                         // Current cursor position
	int16 objId = -1;                               // Return objid under cursor

	switch (action) {
	case kInventoryActionInit:                      // Initialize inventory display
		constructInventory(imageNumb, displayNumb, scrollFl, firstIconId);
		break;
	case kInventoryActionLeft:                      // Scroll left by one icon
		firstIconId = MAX(0, firstIconId - 1);
		constructInventory(imageNumb, displayNumb, scrollFl, firstIconId);
		break;
	case kInventoryActionRight:                     // Scroll right by one icon
		firstIconId = MIN(displayNumb, firstIconId + 1);
		constructInventory(imageNumb, displayNumb, scrollFl, firstIconId);
		break;
	case kInventoryActionGet:                       // Return object id under cursor
		// Get cursor position from variable argument list
		va_start(marker, action);                   // Initialize variable arguments
		cursorx = va_arg(marker, int);              // Cursor x
		cursory = va_arg(marker, int);              // Cursor y
		va_end(marker);                             // Reset variable arguments

		cursory -= kDibOffY;                        // Icon bar is at true zero
		if (cursory > 0 && cursory < kInvDy) {      // Within icon bar?
			int16 i = cursorx / kInvDx;             // Compute icon index
			if (scrollFl) {                         // Scroll buttons displayed
				if (i == 0) {                       // Left scroll button
					objId = kLeftArrow;
				} else {
					if (i == kMaxDisp - 1)          // Right scroll button
						objId = kRightArrow;
					else                            // Adjust for scroll
						i += firstIconId - 1;       // i is icon index
				}
			}

			// If not an arrow, find object id - limit to valid range
			if (objId == -1 && i < displayNumb) {
				// Find objid by counting # carried objects == i+1
				int16 j;
				for (j = 0, i++; i > 0 && j < _vm->_object->_numObj; j++) {
					if (_vm->_object->isCarried(j)) {
						if (--i == 0)
							objId = j;
					}
				}
			}
		}
		break;
	}
	return objId;                                   // For the INV_GET action
}

/**
* Process inventory state machine
*/
void InventoryHandler::runInventory() {
	status_t &gameStatus = _vm->getGameStatus();

	debugC(1, kDebugInventory, "runInventory");

	switch (gameStatus.inventoryState) {
	case kInventoryOff:                             // Icon bar off screen
		break;
	case kInventoryUp:                              // Icon bar moving up
		gameStatus.inventoryHeight -= kStepDy;      // Move the icon bar up
		if (gameStatus.inventoryHeight <= 0)        // Limit travel
			gameStatus.inventoryHeight = 0;

		// Move visible portion to _frontBuffer, restore uncovered portion, display results
		_vm->_screen->moveImage(_vm->_screen->getIconBuffer(), 0, 0, kXPix, gameStatus.inventoryHeight, kXPix, _vm->_screen->getFrontBuffer(), 0, kDibOffY, kXPix);
		_vm->_screen->moveImage(_vm->_screen->getBackBufferBackup(), 0, gameStatus.inventoryHeight + kDibOffY, kXPix, kStepDy, kXPix, _vm->_screen->getFrontBuffer(), 0, gameStatus.inventoryHeight + kDibOffY, kXPix);
		_vm->_screen->displayRect(0, kDibOffY, kXPix, gameStatus.inventoryHeight + kStepDy);

		if (gameStatus.inventoryHeight == 0) {      // Finished moving up?
			// Yes, restore dibs and exit back to game state machine
			_vm->_screen->moveImage(_vm->_screen->getBackBufferBackup(), 0, 0, kXPix, kYPix, kXPix, _vm->_screen->getBackBuffer(), 0, 0, kXPix);
			_vm->_screen->moveImage(_vm->_screen->getBackBuffer(), 0, 0, kXPix, kYPix, kXPix, _vm->_screen->getFrontBuffer(), 0, 0, kXPix);
			_vm->_object->updateImages();           // Add objects back into display list for restore
			gameStatus.inventoryState = kInventoryOff;
			gameStatus.viewState = kViewPlay;
		}
		break;
	case kInventoryDown:                            // Icon bar moving down
		// If this is the first step, initialize dib_i
		// and get any icon/text out of _frontBuffer
		if (gameStatus.inventoryHeight == 0) {
			processInventory(kInventoryActionInit); // Initialize dib_i
			_vm->_screen->displayList(kDisplayRestore); // Restore _frontBuffer
			_vm->_object->updateImages();           // Rebuild _frontBuffer without icons/text
			_vm->_screen->displayList(kDisplayDisplay); // Blit display list to screen
		}

		gameStatus.inventoryHeight += kStepDy;      // Move the icon bar down
		if (gameStatus.inventoryHeight > kInvDy)    // Limit travel
			gameStatus.inventoryHeight = kInvDy;

		// Move visible portion to _frontBuffer, display results
		_vm->_screen->moveImage(_vm->_screen->getIconBuffer(), 0, 0, kXPix, gameStatus.inventoryHeight, kXPix, _vm->_screen->getFrontBuffer(), 0, kDibOffY, kXPix);
		_vm->_screen->displayRect(0, kDibOffY, kXPix, gameStatus.inventoryHeight);

		if (gameStatus.inventoryHeight == kInvDy) { // Finished moving down?
			// Yes, prepare view dibs for special inventory display since
			// we can't refresh objects while icon bar overlayed...
			// 1. Save backing store _backBuffer in temporary dib_c
			// 2. Make snapshot of _frontBuffer the new _backBuffer backing store
			// 3. Reset the display list
			_vm->_screen->moveImage(_vm->_screen->getBackBuffer(), 0, 0, kXPix, kYPix, kXPix, _vm->_screen->getBackBufferBackup(), 0, 0, kXPix);
			_vm->_screen->moveImage(_vm->_screen->getFrontBuffer(), 0, 0, kXPix, kYPix, kXPix, _vm->_screen->getBackBuffer(), 0, 0, kXPix);
			_vm->_screen->displayList(kDisplayInit);
			gameStatus.inventoryState = kInventoryActive;
		}
		break;
	case kInventoryActive:                          // Inventory active
		_vm->_parser->charHandler();                // Still allow commands
		_vm->_screen->displayList(kDisplayRestore); // Restore previous background
		_vm->_screen->displayList(kDisplayDisplay); // Blit the display list to screen
		break;
	}
}

} // End of namespace Hugo
