/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "CEDevice.h"

#include <SDL.h>

#define KEY_CALENDAR 0xc1
#define KEY_CONTACTS 0xc2
#define KEY_INBOX 0xc3
#define KEY_TASK 0xc4

#ifdef WIN32_PLATFORM_WFSP
const char* SMARTPHONE_KEYS_NAME[] = {
	"1", "2", "3","4", "5", "6", "7", "8", "9", "*", "0", "#",
	"Home", "Back", "Up", "Down", "Left", "Right", "Action", "Hang up", "Call",
	"Soft 1", "Soft 2", "Power", "Volume Up" ,"Volume Down", "Record", "None",
	0
};

const int SMARTPHONE_KEYS_MAPPING[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', VK_F8, '0', VK_F9,
        VK_LWIN, VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RETURN, VK_F4, VK_F3,
        VK_F1, VK_F2, VK_F18, VK_F6, VK_F7, VK_F10, 0xff, 0
};
#endif


bool CEDevice::hasPocketPCResolution() {
	return (GetSystemMetrics(SM_CXSCREEN) < 320 && GetSystemMetrics(SM_CXSCREEN) >= 240);
}

bool CEDevice::hasDesktopResolution() {
	return (GetSystemMetrics(SM_CXSCREEN) >= 320);
}

bool CEDevice::hasWideResolution() {
	return (GetSystemMetrics(SM_CXSCREEN) >= 640 || GetSystemMetrics(SM_CYSCREEN) >= 640);
}

bool CEDevice::hasSmartphoneResolution() {
	return (GetSystemMetrics(SM_CXSCREEN) < 240);
}

Common::String CEDevice::getKeyName(unsigned int keyCode) {
	char key_name[10];

	if (!keyCode)
		return "No key";

	if (keyCode == KEY_CALENDAR)
		return "Button Calendar";
	if (keyCode == KEY_CONTACTS)
		return "Button Contacts";
	if (keyCode == KEY_INBOX)
		return "Button Inbox";
	if (keyCode == KEY_TASK)
		return "Button Tasks";
	if (keyCode == SDLK_F1) 
		return "F1 (hard 1)";
	if (keyCode == SDLK_F2) 
		return "F2 (hard 2)";
	if (keyCode == SDLK_F3) 
		return "F3 (hard 3)";
	if (keyCode == SDLK_F4) 
		return "F4 (hard 4)";

#ifdef WIN32_PLATFORM_WFSP
	if (hasSmartphoneResolution()) {
		int i = 0;
		while (SMARTPHONE_KEYS_MAPPING[i]) {
			if (keyCode == SMARTPHONE_KEYS_MAPPING[i])
				return SMARTPHONE_KEYS_NAME[i];
			i++;
		}
	}
#endif

	sprintf(key_name, "Key %.4x", keyCode);
	return key_name;	
}
