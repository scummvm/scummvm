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


} // End of namespace M4
