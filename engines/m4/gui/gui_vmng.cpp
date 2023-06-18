/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * THE GUI:
 * gui_vmng.cpp controls the windowing system. The gui is comprised of layered independent
 * windows.  The view manager controls which pieces of each window are visible, and which
 * window receives events.  The contents of of the windows, whether they be dialog boxes,
 * buffers, or some new addition has no bearing on the performance of the view manager.
 * Therefore, each window is created with a layer, an event handler, and a redraw function.
 * When the view manager determines that an area of a window needs to be redrawn, it
 * simply calls that window's redraw function.  It is up to the redraw function to ensure
 * that the rectangle is properly redrawn.  If an event occurs, the view manager will
 * determine which window should handle the event.
 *
 * To recap then, it manages the visual display of each window's current position
 * and relative layer, and when either a keyboard event, or mouse event is registered, which
 * window's evtHandler will be given the event to process. In addition to requesting a
 * window to redraw a portion of itself, or handle an event which has occurred, vmng.cpp
 * also displays the mouse in its current location.  Through the use of an off screen bitmap
 * which is an exact duplicate of what is visible on the monitor, the view manager creates
 * a flicker-free graphical display of the mouse and all visible windows.
 *
 * NOTE: FOR MANY OF THE FOLLOWING PROCEDURES, A "void *scrnContent" IS LISTED AMONG THE
 * PARAMETERS.  THIS PARAMETER REFERS TO THE STRUCTURE FOR WHICH THE WINDOW WAS CREATED, BE
 * IT A (Buffer*), (Dialog*), (TextScrn*), OR WHATEVER.  SINCE THE VIEW MANAGER ONLY
 * REQUESTS WINDOW REFRESHES AND PASSES EVENTS, THE CONTENTS OF THE WINDOW ARE UNKNOWN,
 * AND THEREFORE, ALL ARE STORED AS (void*). FROM NOW ON, THIS WILL BE KNOWN AS THE "WINDOW
 * IDENTIFIER".
 *
 * NOTE: THE TERM "WINDOW" AND THE TERM "SCREEN" ARE COMPLETELY INTERCHANGEABLE DURING
 * THE DOCUMENTATION OF ANY GUI SOURCE CODE.
 *
 * NOTE: ANY PROCEDURE IN THIS FILE WHICH, WHEN EXECUTED, RESULTS IN A VISUAL CHANGE TO 
 *          THE MONITOR (SUCH AS vmng_screen_show(), or MoveScreen())
 *			WILL ALSO RESTORE THE MONITOR'S IMAGE, TAKING CARE OF A VIDEO REFRESH REQUIREMENTS.
 */

#include "m4/gui/gui_vmng.h"
#include "m4/gui/gui_dialog.h"
#include "m4/mem/memman.h"
#include "m4/mem/mem.h"
#include "m4/globals.h"

namespace M4 {

bool vmng_init() {
	if (_G(vmng_Initted))
		return false;
	_G(vmng_Initted) = true;

	_G(frontScreen) = nullptr;
	_G(backScreen) = nullptr;
	_G(inactiveScreens) = nullptr;

	if (!mem_register_stash_type(&_G(memtypeSCRN), sizeof(ScreenContext), 32, "+SCRN")) {
		return false;
	}
	if (!mem_register_stash_type(&_G(memtypeMATTE), sizeof(matte), 32, "+guiMATTE")) {
		return false;
	}
	if (!mem_register_stash_type(&_G(memtypeRECT), sizeof(RectList), 256, "+guiRecList")) {
		return false;
	}

	return true;
}

void vmng_shutdown() {
	ScreenContext *myScreen;
	Hotkey *myHotkeys, *tempHotkey;

	if (!_G(vmng_Initted))
		return;
	_G(vmng_Initted) = false;

	// First, destroy all active windows
	myScreen = _G(frontScreen);
	while (myScreen) {
		_G(frontScreen) = _G(frontScreen)->behind;
		if (myScreen->scrnType == SCRN_DLG) {
			vmng_Dialog_Destroy((Dialog *)myScreen->scrnContent);
		} else if (myScreen->scrnType == SCRN_TEXT) {
			vmng_TextScrn_Destroy((TextScrn *)myScreen->scrnContent);
		}

		myHotkeys = myScreen->scrnHotkeys;
		tempHotkey = myHotkeys;
		while (tempHotkey) {
			myHotkeys = myHotkeys->next;
			mem_free(tempHotkey);
			tempHotkey = myHotkeys;
		}

		mem_free_to_stash((void *)myScreen, _G(memtypeSCRN));
		myScreen = _G(frontScreen);
	}

	// Duplicate the above loop exactly for the list of inactive windows
	myScreen = _G(inactiveScreens);
	while (myScreen) {
		_G(inactiveScreens) = _G(inactiveScreens)->behind;
		if (myScreen->scrnType == SCRN_DLG) {
			vmng_Dialog_Destroy((Dialog *)myScreen->scrnContent);
		} else if (myScreen->scrnType == SCRN_TEXT) {
			vmng_TextScrn_Destroy((TextScrn *)myScreen->scrnContent);
		}
		myHotkeys = myScreen->scrnHotkeys;
		tempHotkey = myHotkeys;
		while (tempHotkey) {
			myHotkeys = myHotkeys->next;
			mem_free(tempHotkey);
			//mem_free_to_stash((void*)tempHotkey, memtypeHOTKEY);
			tempHotkey = myHotkeys;
		}

		mem_free_to_stash((void *)myScreen, _G(memtypeSCRN));
		myScreen = _G(inactiveScreens);
	}
}

ScreenContext *vmng_screen_create(int32 x1, int32 y1, int32 x2, int32 y2, int32 scrnType, uint32 scrnFlags,
	void *scrnContent, RefreshFunc redraw, EventHandler evtHandler) {
	ScreenContext *myScreen;

	if (!_G(vmng_Initted))
		return nullptr;

	if ((myScreen = (ScreenContext *)mem_get_from_stash(_G(memtypeSCRN), "+SCRN")) == nullptr)
		return nullptr;

	myScreen->x1 = x1;
	myScreen->y1 = y1;
	myScreen->x2 = x2;
	myScreen->y2 = y2;
	myScreen->scrnType = scrnType;
	myScreen->scrnFlags = scrnFlags;
	myScreen->scrnContent = scrnContent;
	myScreen->redraw = redraw;
	myScreen->evtHandler = evtHandler;
	myScreen->scrnHotkeys = nullptr;

	if (_G(inactiveScreens))
		_G(inactiveScreens)->infront = myScreen;

	myScreen->behind = _G(inactiveScreens);
	myScreen->infront = nullptr;
	_G(inactiveScreens) = myScreen;
	return myScreen;
}

} // End of namespace M4
