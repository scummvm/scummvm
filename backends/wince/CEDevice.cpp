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

#include "wince-sdl.h"

bool CEDevice::_hasGAPIMapping = false;
struct GXKeyList CEDevice::_portrait_keys = {0};

#define KEY_CALENDAR 0xc1
#define KEY_CONTACTS 0xc2
#define KEY_INBOX 0xc3
#define KEY_TASK 0xc4

bool CEDevice::hasPocketPCResolution() {
	return ((GetSystemMetrics(SM_CXSCREEN) < 320 && GetSystemMetrics(SM_CXSCREEN) >= 240) ||
			(GetSystemMetrics(SM_CXSCREEN) >= 320 && GetSystemMetrics(SM_CYSCREEN) < 480));
}

bool CEDevice::hasDesktopResolution() {
	return (GetSystemMetrics(SM_CXSCREEN) >= 320 && GetSystemMetrics(SM_CYSCREEN) >= 240);
}

bool CEDevice::hasWideResolution() {
	return ((GetSystemMetrics(SM_CXSCREEN) >= 640 && GetSystemMetrics(SM_CYSCREEN) >= 480)
		   ||(GetSystemMetrics(SM_CYSCREEN) >= 640 && GetSystemMetrics(SM_CXSCREEN) >= 480));
}

bool CEDevice::hasSmartphoneResolution() {
	return (GetSystemMetrics(SM_CXSCREEN) < 240);
}

bool CEDevice::enableHardwareKeyMapping() {
	HINSTANCE GAPI_handle;
	tGXVoidFunction GAPIOpenInput;
	tGXGetDefaultKeys GAPIGetDefaultKeys;

	_hasGAPIMapping = false;
	GAPI_handle = LoadLibrary(TEXT("gx.dll"));
	if (!GAPI_handle)
		return false;
	GAPIOpenInput = (tGXVoidFunction)GetProcAddress(GAPI_handle, TEXT("?GXOpenInput@@YAHXZ"));
	if (!GAPIOpenInput)
		return false;
	GAPIGetDefaultKeys = (tGXGetDefaultKeys)GetProcAddress(GAPI_handle, TEXT("?GXGetDefaultKeys@@YA?AUGXKeyList@@H@Z"));
	if (!GAPIGetDefaultKeys)
		return false;
	GAPIOpenInput();
	_portrait_keys = GAPIGetDefaultKeys(GX_NORMALKEYS);
	_hasGAPIMapping = true;
	FreeLibrary(GAPI_handle);
	return true;
}

bool CEDevice::disableHardwareKeyMapping() {
	HINSTANCE GAPI_handle;
	tGXVoidFunction GAPICloseInput;

	GAPI_handle = LoadLibrary(TEXT("gx.dll"));
	if (!GAPI_handle)
		return false;
	GAPICloseInput = (tGXVoidFunction)GetProcAddress(GAPI_handle, TEXT("?GXCloseInput@@YAHXZ"));
	if (!GAPICloseInput)
		return false;
	GAPICloseInput();
	FreeLibrary(GAPI_handle);
	return true;
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

	sprintf(key_name, "Key %.4x", keyCode);
	return key_name;	
}
