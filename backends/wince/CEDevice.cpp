/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "CEDevice.h"

#include <SDL.h>

#include "wince-sdl.h"

#define KEY_CALENDAR 0xc1
#define KEY_CONTACTS 0xc2
#define KEY_INBOX 0xc3
#define KEY_TASK 0xc4

//#ifdef WIN32_PLATFORM_WFSP
const char* SMARTPHONE_KEYS_NAME[] = {
	"1", "2", "3","4", "5", "6", "7", "8", "9", "*", "0", "#",
	"Home", "Back", "Up", "Down", "Left", "Right", "Action", "Hang up", "Call",
	"Soft 1", "Soft 2", "Power", "Volume Up" ,"Volume Down", "Record", "None",
	0
};

// Old mapping from the previous (non SDL) version. To be forgotten.
/*
const int SMARTPHONE_KEYS_MAPPING[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', VK_F8, '0', VK_F9,
        VK_LWIN, VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RETURN, VK_F4, VK_F3,
        VK_F1, VK_F2, VK_F18, VK_F6, VK_F7, VK_F10, 0xff, 0
};
*/

// FIXME : Home and Record are not mapped
const int SMARTPHONE_KEYS_MAPPING[] = {
		'1', '2', '3', '4', '5', '6', '7', '8', '9', VK_F9, '0', VK_F10,
		0xFF, VK_ESCAPE, 0x113, 0x114, 0x111, 0x112, VK_RETURN, 0x11D, 0x11C,
		0x11A, 0x11B, 0x11D, 0x11F, 0x120, 0xFF, 0
};

static void (WINAPI* _SHIdleTimerReset)(void) = NULL;
static HANDLE (WINAPI* _SetPowerRequirement)(PVOID,int,ULONG,PVOID,ULONG) = NULL;
static DWORD (WINAPI* _ReleasePowerRequirement)(HANDLE) = NULL;
static HANDLE _hPowerManagement = NULL;
static DWORD _lastTime = 0;

#define TIMER_TRIGGER 9000

//#endif

// Power management code borrowed from MoDaCo & Betaplayer. Thanks !
void CEDevice::init() {
	HINSTANCE dll = LoadLibrary(TEXT("aygshell.dll"));
	if (dll) {
		*(FARPROC*)&_SHIdleTimerReset = GetProcAddress(dll, MAKEINTRESOURCE(2006));
	}
	dll = LoadLibrary(TEXT("coredll.dll"));
	if (dll) {
		*(FARPROC*)&_SetPowerRequirement = GetProcAddress(dll, TEXT("SetPowerRequirement"));
		*(FARPROC*)&_ReleasePowerRequirement = GetProcAddress(dll, TEXT("ReleasePowerRequirement"));

	}
	if (_SetPowerRequirement)
		_hPowerManagement = _SetPowerRequirement(TEXT("BKL1:"), 0, 1, NULL, 0);
	_lastTime = GetTickCount();
}

void CEDevice::end() {
	if (_ReleasePowerRequirement && _hPowerManagement) {
		_ReleasePowerRequirement(_hPowerManagement);
	}
}

void CEDevice::wakeUp() {
	DWORD currentTime = GetTickCount();
	if (currentTime > _lastTime + TIMER_TRIGGER) {
		_lastTime = currentTime;
		SystemIdleTimerReset();
		if (_SHIdleTimerReset)
			_SHIdleTimerReset();
	}
}

bool CEDevice::hasPocketPCResolution() {
#ifdef SIMU_SMARTPHONE
	return false;
#else
	if (OSystem_WINCE3::isOzone() && hasWideResolution())
		return true;
	return (OSystem_WINCE3::getScreenWidth() < 320 && OSystem_WINCE3::getScreenWidth() >= 240);
#endif
}

bool CEDevice::hasDesktopResolution() {
#ifdef SIMU_SMARTPHONE
	return false;
#else
	if (OSystem_WINCE3::isOzone() && hasWideResolution())
		return true;
	return (OSystem_WINCE3::getScreenWidth() >= 320);
#endif
}

bool CEDevice::hasWideResolution() {
#ifdef SIMU_SMARTPHONE
	return false;
#else
	return (OSystem_WINCE3::getScreenWidth() >= 640 || OSystem_WINCE3::getScreenHeight() >= 640);
#endif
}

bool CEDevice::hasSmartphoneResolution() {
#ifdef SIMU_SMARTPHONE
	return true;
#else
	return (OSystem_WINCE3::getScreenWidth() < 240);
#endif
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

//#ifdef WIN32_PLATFORM_WFSP
	if (hasSmartphoneResolution()) {
		int i = 0;
		while (SMARTPHONE_KEYS_MAPPING[i]) {
			if (keyCode == SMARTPHONE_KEYS_MAPPING[i])
				return SMARTPHONE_KEYS_NAME[i];
			i++;
		}
	}
//#endif

	sprintf(key_name, "Key %.4x", keyCode);
	return key_name;
}

