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

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/mouse.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/inventory.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/object.h"

namespace Hugo {

MouseHandler::MouseHandler(HugoEngine *vm) : _vm(vm) {
}

/**
* Shadow-blit supplied string into dib_a at cx,cy and add to display list
*/
void MouseHandler::cursorText(char *buffer, int16 cx, int16 cy, uif_t fontId, int16 color) {
	debugC(1, kDebugMouse, "cursorText(%s, %d, %d, %d, %d)", buffer, cx, cy, fontId, color);

	_vm->_screen->loadFont(fontId);

	// Find bounding rect for string
	int16 sdx = _vm->_screen->stringLength(buffer);
	int16 sdy = _vm->_screen->fontHeight() + 1;     // + 1 for shadow
	int16 sx, sy;
	if (cx < kXPix / 2) {
		sx = cx + kCursorNameOffX;
		sy = (_vm->getGameStatus().inventoryObjId == -1) ? cy + kCursorNameOffY : cy + kCursorNameOffY - (_vm->_screen->fontHeight() + 1);
	} else {
		sx = cx - sdx - kCursorNameOffX / 2;
		sy = cy + kCursorNameOffY;
	}

	// Display the string and add rect to display list
	_vm->_screen->shadowStr(sx, sy, buffer, _TBRIGHTWHITE);
	_vm->_screen->displayList(kDisplayAdd, sx, sy, sdx, sdy);
}

/**
* Find the exit hotspot containing cx, cy.
* Return hotspot index or -1 if not found.
*/
int16 MouseHandler::findExit(int16 cx, int16 cy) {
	debugC(2, kDebugMouse, "findExit(%d, %d)", cx, cy);

	int i = 0;
	for (hotspot_t *hotspot = _vm->_hotspots; hotspot->screenIndex >= 0; i++, hotspot++) {
		if (hotspot->screenIndex == *_vm->_screen_p) {
			if (cx >= hotspot->x1 && cx <= hotspot->x2 && cy >= hotspot->y1 && cy <= hotspot->y2)
				return i;
		}
	}
	return -1;
}

/**
* Process a mouse right click at coord cx, cy over object objid
*/
void MouseHandler::processRightClick(int16 objId, int16 cx, int16 cy) {
	debugC(1, kDebugMouse, "Process_rclick(%d, %d, %d)", objId, cx, cy);

	status_t &gameStatus = _vm->getGameStatus();

	if (gameStatus.storyModeFl || _vm->_hero->pathType == kPathQuiet) // Make sure user has control
		return;

	bool foundFl = false;                           // TRUE if route found to object
	// Check if this was over iconbar
	if ((gameStatus.inventoryState == kInventoryActive) && (cy < kInvDy + kDibOffY)) { // Clicked over iconbar object
		if (gameStatus.inventoryObjId == -1)
			_vm->_screen->selectInventoryObjId(objId);
		else if (gameStatus.inventoryObjId == objId)
			_vm->_screen->resetInventoryObjId();
		else
			_vm->_object->useObject(objId);         // Use status.objid on object
	} else {                                        // Clicked over viewport object
		object_t *obj = &_vm->_object->_objects[objId];
		int16 x, y;
		switch (obj->viewx) {                       // Where to walk to
		case -1:                                    // Walk to object position
			if (_vm->_object->findObjectSpace(obj, &x, &y))
				foundFl = _vm->_route->startRoute(kRouteGet, objId, x, y);
			if (!foundFl)                           // Can't get there, try to use from here
				_vm->_object->useObject(objId);
			break;
		case 0:                                     // Immediate use
			_vm->_object->useObject(objId);         // Pick up or use object
			break;
		default:                                    // Walk to view point if possible
			if (!_vm->_route->startRoute(kRouteGet, objId, obj->viewx, obj->viewy)) {
				if (_vm->_hero->cycling == kCycleInvisible) // If invisible do
					_vm->_object->useObject(objId); // immediate use
				else
					Utils::Box(kBoxAny, "%s", _vm->_textMouse[kMsNoWayText]);      // Can't get there
			}
			break;
		}
	}
}

/** Process a left mouse click over:
* 1.  An icon - show description
* 2.  An object - walk to and show description
* 3.  An icon scroll arrow - scroll the iconbar
* 4.  Nothing - attempt to walk there
* 5.  Exit - walk to exit hotspot
*/
void MouseHandler::processLeftClick(int16 objId, int16 cx, int16 cy) {
	debugC(1, kDebugMouse, "Process_lclick(%d, %d, %d)", objId, cx, cy);

	int16 i, x, y;
	object_t *obj;

	status_t &gameStatus = _vm->getGameStatus();

	if (gameStatus.storyModeFl || _vm->_hero->pathType == kPathQuiet) // Make sure user has control
		return;

	switch (objId) {
	case -1:                                        // Empty space - attempt to walk there
		_vm->_route->startRoute(kRouteSpace, 0, cx, cy);
		break;
	case kLeftArrow:                                // A scroll arrow - scroll the iconbar
	case kRightArrow:
		// Scroll the iconbar and display results
		_vm->_inventory->processInventory((objId == kLeftArrow) ? kInventoryActionLeft : kInventoryActionRight);
		_vm->_screen->moveImage(_vm->_screen->getIconBuffer(), 0, 0, kXPix, kInvDy, kXPix, _vm->_screen->getFrontBuffer(), 0, kDibOffY, kXPix);
		_vm->_screen->moveImage(_vm->_screen->getIconBuffer(), 0, 0, kXPix, kInvDy, kXPix, _vm->_screen->getBackBuffer(), 0, kDibOffY, kXPix);
		_vm->_screen->displayList(kDisplayAdd, 0, kDibOffY, kXPix, kInvDy);
		break;
	case kExitHotspot:                              // Walk to exit hotspot
		i = findExit(cx, cy);
		x = _vm->_hotspots[i].viewx;
		y = _vm->_hotspots[i].viewy;
		if (x >= 0) {                               // Hotspot refers to an exit
			// Special case of immediate exit
			if (gameStatus.jumpExitFl) {
				// Get rid of iconbar if necessary
				if (gameStatus.inventoryState != kInventoryOff)
					gameStatus.inventoryState = kInventoryUp;
				_vm->_scheduler->insertActionList(_vm->_hotspots[i].actIndex);
			} else {    // Set up route to exit spot
				if (_vm->_hotspots[i].direction == Common::KEYCODE_RIGHT)
					x -= kHeroMaxWidth;
				else if (_vm->_hotspots[i].direction == Common::KEYCODE_LEFT)
					x += kHeroMaxWidth;
				if (!_vm->_route->startRoute(kRouteExit, i, x, y))
					Utils::Box(kBoxAny, "%s", _vm->_textMouse[kMsNoWayText]); // Can't get there
			}

			// Get rid of any attached icon
			_vm->_screen->resetInventoryObjId();
		}
		break;
	default:                                        // Look at an icon or object
		obj = &_vm->_object->_objects[objId];

		// Over iconbar - immediate description
		if ((gameStatus.inventoryState == kInventoryActive) && (cy < kInvDy + kDibOffY)) {
			_vm->_object->lookObject(obj);
		} else {
			bool foundFl = false;                   // TRUE if route found to object
			switch (obj->viewx) {                   // Clicked over viewport object
			case -1:                                // Walk to object position
				if (_vm->_object->findObjectSpace(obj, &x, &y))
					foundFl = _vm->_route->startRoute(kRouteLook, objId, x, y);
				if (!foundFl)                       // Can't get there, immediate description
					_vm->_object->lookObject(obj);
				break;
			case 0:                                 // Immediate description
				_vm->_object->lookObject(obj);
				break;
			default:                                // Walk to view point if possible
				if (!_vm->_route->startRoute(kRouteLook, objId, obj->viewx, obj->viewy)) {
					if (_vm->_hero->cycling == kCycleInvisible) // If invisible do
						_vm->_object->lookObject(obj);          // immediate decription
					else
						Utils::Box(kBoxAny, "%s", _vm->_textMouse[kMsNoWayText]);  // Can't get there
				}
				break;
			}
		}
		break;
	}
}

/**
* Process mouse activity
*/
void MouseHandler::mouseHandler() {
	debugC(2, kDebugMouse, "mouseHandler");

	status_t &gameStatus = _vm->getGameStatus();

	if ((gameStatus.viewState != kViewPlay) && (gameStatus.inventoryState != kInventoryActive))
		return;

	int16 cx = _vm->getMouseX();
	int16 cy = _vm->getMouseY();

	gameStatus.cx = cx;                             // Save cursor coords
	gameStatus.cy = cy;

	// Don't process if outside client area
	if ((cx < 0) || (cx > kXPix) || (cy < kDibOffY) || (cy > kViewSizeY + kDibOffY))
		return;

	int16 objId = -1;                               // Current source object
	// Process cursor over an object or icon
	if (gameStatus.inventoryState == kInventoryActive) { // Check inventory icon bar first
		objId = _vm->_inventory->processInventory(kInventoryActionGet, cx, cy);
	} else {
		if (cy < 5 && cy > 0) {
			_vm->_topMenu->runModal();
		}
	}

	if (!gameStatus.gameOverFl) {
		if (objId == -1)                            // No match, check rest of view
			objId = _vm->_object->findObject(cx, cy);

		if (objId >= 0) {                           // Got a match
			// Display object name next to cursor (unless CURSOR_NOCHAR)
			// Note test for swapped hero name
			char *name = _vm->_arrayNouns[_vm->_object->_objects[(objId == kHeroIndex) ? _vm->_heroImage : objId].nounIndex][kCursorNameIndex];
			if (name[0] != kCursorNochar)
				cursorText(name, cx, cy, U_FONT8, _TBRIGHTWHITE);

			// Process right click over object in view or iconbar
			if (gameStatus.rightButtonFl)
				processRightClick(objId, cx, cy);
		}

		// Process cursor over an exit hotspot
		if (objId == -1) {
			int i = findExit(cx, cy);
			if (i != -1 && _vm->_hotspots[i].viewx >= 0) {
				objId = kExitHotspot;
				cursorText(_vm->_textMouse[kMsExit], cx, cy, U_FONT8, _TBRIGHTWHITE);
			}
		}
	}
	// Left click over icon, object or to move somewhere
	if (gameStatus.leftButtonFl)
		processLeftClick(objId, cx, cy);

	// Clear mouse click states
	gameStatus.leftButtonFl = false;
	gameStatus.rightButtonFl = false;
}

} // End of namespace Hugo
