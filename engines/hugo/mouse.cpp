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

// mouse.cpp : Handle all mouse activity

#include "common/system.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/mouse.h"
#include "hugo/global.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/inventory.h"
#include "hugo/route.h"
#include "hugo/util.h"

namespace Hugo {

#define EXIT_HOTSPOT   -4                           // Cursor over Exit hotspot
#define CURSOR_NAME    2                            // Index of name used under cursor
#define CURSOR_NOCHAR  '~'                          // Don't show name of object under cursor
#define SX_OFF         10                           // Cursor offset to name string
#define SY_OFF         -2                           // Cursor offset to name string
#define IX_OFF         8                            // Cursor to icon image (dib coords)
#define IY_OFF         10                           // Cursor to icon image (dib coords)

enum seqTextMouse {
	kMsNoWayText = 0,
	kMsExit      = 1
};

MouseHandler::MouseHandler(HugoEngine &vm) : _vm(vm) {
}

// Shadow-blit supplied string into dib_a at cx,cy and add to display list
void MouseHandler::cursorText(char *buffer, int16 cx, int16 cy, uif_t fontId, int16 color) {

	debugC(1, kDebugMouse, "cursorText(%s, %d, %d, %d, %d)", buffer, cx, cy, fontId, color);

	if (_vm.getPlatform() == Common::kPlatformWindows)
		_vm.screen().loadFont(fontId);

	// Find bounding rect for string
	int16 sdx = _vm.screen().stringLength(buffer);
	int16 sdy = _vm.screen().fontHeight() + 1;                      // + 1 for shadow
	int16 sx  = (cx < XPIX / 2) ? cx + SX_OFF : cx - sdx - SX_OFF / 2;
	int16 sy  = cy + SY_OFF;

	// Display the string and add rect to display list
	_vm.screen().shadowStr(sx, sy, buffer, _TBRIGHTWHITE);
	_vm.screen().displayList(D_ADD, sx, sy, sdx, sdy);
}


// Find the exit hotspot containing cx, cy.
// Return hotspot index or -1 if not found.
int16 MouseHandler::findExit(int16 cx, int16 cy) {
	int i;
	hotspot_t *hotspot;

	debugC(2, kDebugMouse, "findExit(%d, %d)", cx, cy);

	for (i = 0, hotspot = _vm._hotspots; hotspot->screenIndex >= 0; i++, hotspot++)
		if (hotspot->screenIndex == *_vm._screen_p)
			if (cx >= hotspot->x1 && cx <= hotspot->x2 && cy >= hotspot->y1 && cy <= hotspot->y2)
				return(i);
	return(-1);
}

// Process a mouse right click at coord cx, cy over object objid
void MouseHandler::processRightClick(int16 objId, int16 cx, int16 cy) {
	object_t *obj;
	int16     x, y;
	bool      foundFl = false;                      // TRUE if route found to object

	debugC(1, kDebugMouse, "Process_rclick(%d, %d, %d)", objId, cx, cy);

	status_t &gameStatus = _vm.getGameStatus();

	if (gameStatus.storyModeFl || _vm._hero->pathType == QUIET)     // Make sure user has control
		return;

	// Check if this was over iconbar
	if (gameStatus.inventoryState == I_ACTIVE && cy < INV_DY + DIBOFF_Y) { // Clicked over iconbar object
		if (gameStatus.inventoryObjId == -1)
			gameStatus.inventoryObjId = objId;      // Not using so select new object
		else if (gameStatus.inventoryObjId == objId)
			gameStatus.inventoryObjId = -1;     // Same icon - deselect it
		else
			_vm.useObject(objId);               // Use status.objid on object
	} else {                                        // Clicked over viewport object
		obj = &_vm._objects[objId];
		switch (obj->viewx) {                       // Where to walk to
		case -1:                                    // Walk to object position
			if (_vm.findObjectSpace(obj, &x, &y))
				foundFl = _vm.route().startRoute(GO_GET, objId, x, y);
			if (!foundFl)                               // Can't get there, try to use from here
				_vm.useObject(objId);
			break;
		case 0:                                     // Immediate use
			_vm.useObject(objId);                   // Pick up or use object
			break;
		default:                                    // Walk to view point if possible
			if (!_vm.route().startRoute(GO_GET, objId, obj->viewx, obj->viewy)) {
				if (_vm._hero->cycling == INVISIBLE)    // If invisible do
					_vm.useObject(objId);           // immediate use
				else
					Utils::Box(BOX_ANY, _vm._textMouse[kMsNoWayText]);      // Can't get there
			}
			break;
		}
	}
}

// Process a left mouse click over:
// 1.  An icon - show description
// 2.  An object - walk to and show description
// 3.  An icon scroll arrow - scroll the iconbar
// 4.  Nothing - attempt to walk there
// 5.  Exit - walk to exit hotspot
void MouseHandler::processLeftClick(int16 objId, int16 cx, int16 cy) {
	int16 i, x, y;
	object_t *obj;
	bool foundFl = false;                               // TRUE if route found to object

	debugC(1, kDebugMouse, "Process_lclick(%d, %d, %d)", objId, cx, cy);

	status_t &gameStatus = _vm.getGameStatus();

	if (gameStatus.storyModeFl || _vm._hero->pathType == QUIET)     // Make sure user has control
		return;

	switch (objId) {
	case -1:                                        // Empty space - attempt to walk there
		_vm.route().startRoute(GO_SPACE, 0, cx, cy);
		break;
	case LEFT_ARROW:                                // A scroll arrow - scroll the iconbar
	case RIGHT_ARROW:
		// Scroll the iconbar and display results
		_vm.inventory().processInventory(objId == LEFT_ARROW ? INV_LEFT : INV_RIGHT);
		_vm.screen().moveImage(_vm.screen().getIconBuffer(), 0, 0, XPIX, INV_DY, XPIX, _vm.screen().getFrontBuffer(), 0, DIBOFF_Y, XPIX);
		_vm.screen().moveImage(_vm.screen().getIconBuffer(), 0, 0, XPIX, INV_DY, XPIX, _vm.screen().getBackBuffer(), 0, DIBOFF_Y, XPIX);
		_vm.screen().displayList(D_ADD, 0, DIBOFF_Y, XPIX, INV_DY);
		break;
	case EXIT_HOTSPOT:                              // Walk to exit hotspot
		i = findExit(cx, cy);
		x = _vm._hotspots[i].viewx;
		y = _vm._hotspots[i].viewy;
		if (x >= 0) {                               // Hotspot refers to an exit
			// Special case of immediate exit
			if (gameStatus.jumpExitFl) {
				// Get rid of iconbar if necessary
				if (gameStatus.inventoryState != I_OFF)
					gameStatus.inventoryState = I_UP;
				_vm.scheduler().insertActionList(_vm._hotspots[i].actIndex);
			} else {    // Set up route to exit spot
				if (_vm._hotspots[i].direction == Common::KEYCODE_RIGHT)
					x -= HERO_MAX_WIDTH;
				else if (_vm._hotspots[i].direction == Common::KEYCODE_LEFT)
					x += HERO_MAX_WIDTH;
				if (!_vm.route().startRoute(GO_EXIT, i, x, y))
					Utils::Box(BOX_ANY, _vm._textMouse[kMsNoWayText]);      // Can't get there
			}

			// Get rid of any attached icon
			gameStatus.inventoryObjId = -1;
		}
		break;
	default:                                        // Look at an icon or object
		obj = &_vm._objects[objId];

		// Over iconbar - immediate description
		if (gameStatus.inventoryState == I_ACTIVE && cy < INV_DY + DIBOFF_Y)
			_vm.lookObject(obj);
		else {
			switch (obj->viewx) {                   // Clicked over viewport object
			case -1:                                // Walk to object position
				if (_vm.findObjectSpace(obj, &x, &y))
					foundFl = _vm.route().startRoute(GO_LOOK, objId, x, y);
				if (!foundFl)                           // Can't get there, immediate description
					_vm.lookObject(obj);
				break;
			case 0:                                 // Immediate description
				_vm.lookObject(obj);
				break;
			default:                                // Walk to view point if possible
				if (!_vm.route().startRoute(GO_LOOK, objId, obj->viewx, obj->viewy)) {
					if (_vm._hero->cycling == INVISIBLE)    // If invisible do
						_vm.lookObject(obj);            // immediate decription
					else
						Utils::Box(BOX_ANY, _vm._textMouse[kMsNoWayText]);  // Can't get there
				}
				break;
			}
		}
		break;
	}
}

// Process mouse activity
void MouseHandler::mouseHandler() {
	int16 iconId;                                   // Find index of dragged icon
	int   iconx, icony;                             // Icon position (in dib_a)
	int16 ux, uy;                                   // Icon position (in dib_u)
	int16 objId = -1;                               // Current source object
	char *name;                                     // Name of object to display

	debugC(2, kDebugMouse, "mouseHandler");

	int16 cx = _vm.getMouseX();
	int16 cy = _vm.getMouseY();

	status_t &gameStatus = _vm.getGameStatus();

	gameStatus.cx = cx;                                 // Save cursor coords
	gameStatus.cy = cy;

	// Don't process if outside client area
	if (cx < 0 || cx > XPIX || cy < DIBOFF_Y || cy > VIEW_DY + DIBOFF_Y)
		return;

	// Display dragged inventory icon if one currently selected
	if (gameStatus.inventoryObjId != -1) {
		// Find index of icon
		for (iconId = 0; iconId < _vm._maxInvent; iconId++)
			if (gameStatus.inventoryObjId == _vm._invent[iconId])
				break;

		// Compute source coordinates in dib_u
		ux = (iconId + NUM_ARROWS) * INV_DX % XPIX;
		uy = (iconId + NUM_ARROWS) * INV_DX / XPIX * INV_DY;

		// Compute destination coordinates in dib_a
		iconx = cx + IX_OFF;
		icony = cy + IY_OFF;
		iconx = MAX(iconx, 0);                      // Keep within dib_a bounds
		iconx = MIN(iconx, XPIX - INV_DX);
		icony = MAX(icony, 0);
		icony = MIN(icony, YPIX - INV_DY);

		// Copy the icon and add to display list
		_vm.screen().moveImage(_vm.screen().getGUIBuffer(), ux, uy, INV_DX, INV_DY, XPIX, _vm.screen().getFrontBuffer(), iconx, icony, XPIX);
		_vm.screen().displayList(D_ADD, iconx, icony, INV_DX, INV_DY);
	}

	// Process cursor over an object or icon
	if (gameStatus.inventoryState == I_ACTIVE)      // Check inventory icon bar first
		objId = _vm.inventory().processInventory(INV_GET, cx, cy);
	if (objId == -1)                                // No match, check rest of view
		objId = _vm.findObject(cx, cy);
	if (objId >= 0) {                               // Got a match
		// Display object name next to cursor (unless CURSOR_NOCHAR)
		// Note test for swapped hero name
		name = _vm._arrayNouns[_vm._objects[objId == HERO ? _vm._heroImage : objId].nounIndex][CURSOR_NAME];
		if (name[0] != CURSOR_NOCHAR)
			cursorText(name, cx, cy, U_FONT8, _TBRIGHTWHITE);

		// Process right click over object in view or iconbar
		if (gameStatus.rightButtonFl)
			processRightClick(objId, cx, cy);
	}

	// Process cursor over an exit hotspot
	if (objId == -1) {
		int i = findExit(cx, cy);
		if (i != -1 && _vm._hotspots[i].viewx >= 0) {
			objId = EXIT_HOTSPOT;
			cursorText(_vm._textMouse[kMsExit], cx, cy, U_FONT8, _TBRIGHTWHITE);
		}
	}

	// Left click over icon, object or to move somewhere
	if (gameStatus.leftButtonFl)
		processLeftClick(objId, cx, cy);

	// Clear mouse click states
	gameStatus.leftButtonFl = false;
	gameStatus.rightButtonFl = false;
}

} // end of namespace Hugo
