/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2007 The ScummVM project
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

#include "common/stdafx.h"
#include "CEDevice.h"

#include <SDL.h>

#include "wince-sdl.h"

static void (WINAPI* _SHIdleTimerReset)(void) = NULL;
static HANDLE (WINAPI* _SetPowerRequirement)(PVOID,int,ULONG,PVOID,ULONG) = NULL;
static DWORD (WINAPI* _ReleasePowerRequirement)(HANDLE) = NULL;
static HANDLE _hPowerManagement = NULL;
static DWORD _lastTime = 0;
static DWORD REG_bat = 0, REG_ac = 0, REG_disp = 0, bat_timeout = 0;
#ifdef __GNUC__
extern "C" void WINAPI SystemIdleTimerReset(void);
#define SPI_GETPLATFORMTYPE 		257
#define SPI_SETBATTERYIDLETIMEOUT	251
#define SPI_GETBATTERYIDLETIMEOUT	252
#endif

#define TIMER_TRIGGER 9000

DWORD CEDevice::reg_access(TCHAR *key, TCHAR *val, DWORD data) {
	HKEY regkey;
	DWORD tmpval, cbdata;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, 0, &regkey) != ERROR_SUCCESS)
		return data;

	cbdata = sizeof(DWORD);
	if (RegQueryValueEx(regkey, val, NULL, NULL, (LPBYTE) &tmpval, &cbdata) != ERROR_SUCCESS)
	{
		RegCloseKey(regkey);
		return data;
	}

	cbdata = sizeof(DWORD);
	if (RegSetValueEx(regkey, val, 0, REG_DWORD, (LPBYTE) &data, cbdata) != ERROR_SUCCESS)
	{
		RegCloseKey(regkey);
		return data;
	}

	RegCloseKey(regkey);
	return tmpval;
}

void CEDevice::backlight_xchg() {
	HANDLE h;

	REG_bat = reg_access(TEXT("ControlPanel\\BackLight"), TEXT("BatteryTimeout"), REG_bat);
	REG_ac = reg_access(TEXT("ControlPanel\\BackLight"), TEXT("ACTimeout"), REG_ac);
	REG_disp = reg_access(TEXT("ControlPanel\\Power"), TEXT("Display"), REG_disp);

	h = CreateEvent(NULL, FALSE, FALSE, TEXT("BackLightChangeEvent"));
	if (h)
	{
		SetEvent(h);
		CloseHandle(h);
	}
}

void CEDevice::init() {
	// 2003+ power management code borrowed from MoDaCo & Betaplayer. Thanks !
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
		_hPowerManagement = _SetPowerRequirement((PVOID) TEXT("BKL1:"), 0, 1, (PVOID) NULL, 0);
	_lastTime = GetTickCount();

	// older devices
	REG_bat = REG_ac = REG_disp = 2 * 60 * 60 * 1000;	// 2hrs should do it
	backlight_xchg();
	SystemParametersInfo(SPI_GETBATTERYIDLETIMEOUT, 0, (void *) &bat_timeout, 0);
	SystemParametersInfo(SPI_SETBATTERYIDLETIMEOUT, 60 * 60 * 2, NULL, SPIF_SENDCHANGE);	
}

void CEDevice::end() {
	if (_ReleasePowerRequirement && _hPowerManagement)
		_ReleasePowerRequirement(_hPowerManagement);

	backlight_xchg();
	SystemParametersInfo(SPI_SETBATTERYIDLETIMEOUT, bat_timeout, NULL, SPIF_SENDCHANGE);	
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

bool CEDevice::hasSquareQVGAResolution() {
	return (OSystem_WINCE3::getScreenWidth() == 240 && OSystem_WINCE3::getScreenHeight() == 240);
}

bool CEDevice::hasPocketPCResolution() {
	if (OSystem_WINCE3::isOzone() && hasWideResolution())
		return true;
	return (OSystem_WINCE3::getScreenWidth() <= 320 && OSystem_WINCE3::getScreenWidth() >= 240);
}

bool CEDevice::hasDesktopResolution() {
	if (OSystem_WINCE3::isOzone() && hasWideResolution())
		return true;
	return (OSystem_WINCE3::getScreenWidth() > 320);
}

bool CEDevice::hasWideResolution() {
	return (OSystem_WINCE3::getScreenWidth() >= 640 || OSystem_WINCE3::getScreenHeight() >= 640);
}

bool CEDevice::hasSmartphoneResolution() {
	return (OSystem_WINCE3::getScreenWidth() < 240);
}

bool CEDevice::isSmartphone() {
	TCHAR platformType[100];
	BOOL result = SystemParametersInfo(SPI_GETPLATFORMTYPE, sizeof(platformType), platformType, 0);
	if (!result && GetLastError() == ERROR_ACCESS_DENIED)
		return true;
	return (_wcsnicmp(platformType, TEXT("SmartPhone"), 10) == 0);
}

Common::String CEDevice::getKeyName(unsigned int keyCode) {
	switch (keyCode) {
		case SDLK_F1:
			return "Softkey A";
		case SDLK_F2:
			return "Softkey B";
		case SDLK_F3:
			return "Talk";
		case SDLK_F4:
			return "End";
		case SDLK_APP1:
			return "Application 1";
		case SDLK_APP2:
			return "Application 2";
		case SDLK_APP3:
			return "Application 3";
		case SDLK_APP4:
			return "Application 4";
		case SDLK_APP5:
			return "Application 5";
		case SDLK_APP6:
			return "Application 6";
		case SDLK_LSUPER:
			return "Home";
		case SDLK_ESCAPE:
			return "Back";
		case SDLK_UP:
			return "Up";
		case SDLK_DOWN:
			return "Down";
		case SDLK_LEFT:
			return "Left";
		case SDLK_RIGHT:
			return "Right";
		case SDLK_RETURN:
			return "Action";
		case SDLK_F10:
			return "Record";
		case SDLK_F6:
			return "Volume Up";
		case SDLK_F7:
			return "Volume Down";
		case SDLK_F17:
			return "Flip";
		case SDLK_F18:
			return "Power";
		case SDLK_F16:
			return "Speaker";
		case SDLK_F8:
			return "Star";
		case SDLK_F9:
			return "Pound";
		case SDLK_F11:
			return "Symbol";
		case SDLK_F19:
			return "Red Key";
		case 0:
			return "None";
		default:
			return SDL_GetKeyName((SDLKey)keyCode);
	}
}
