/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
 * Copyright (C) 2002-2005 Chris Apers - PalmOS Backend
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

#include "be_base.h"
#include "common/config-file.h"
#include "common/config-manager.h"

void OSystem_PalmBase::initBackend() {
	_overlayVisible = false;

	_current_shake_pos = 0;
	_new_shake_pos = 0;

	_paletteDirtyStart = 0;
	_paletteDirtyEnd = 0;

	_gfxLoaded = false;
	_modeChanged = false;
	_setMode = GFX_NORMAL;
	_mode = _setMode;
	_redawOSD = false;
	
	_offScreenH = NULL;
	_screenH = NULL;
	_offScreenP = NULL;
	_screenP = NULL;
	_screenPitch = gVars->screenPitch;
	
	_lastEvent = nilEvent;
	_lastKey = kKeyNone;
	_lastKeyModifier = kModifierNone;
	_lastKeyRepeat = 100;
	_useNumPad = false;
	_showBatLow = false;
	_batCheckTicks = SysTicksPerSecond() * 15;
	_batCheckLast = TimGetTicks();
	
	_mouseDataH = NULL;
	_mouseDataP = NULL;
	_mouseVisible = false;
	_mouseDrawn = false;
	MemSet(&_keyMouse, sizeof(_keyMouse), 0);
	MemSet(&_mouseCurState, sizeof(_mouseCurState), 0);
	MemSet(&_mouseOldState, sizeof(_mouseOldState), 0);
	
	int_initBackend();
	_keyMouseMask = (_keyMouse.bitUp | _keyMouse.bitDown | _keyMouse.bitLeft | _keyMouse.bitRight | _keyMouse.bitButLeft);
}

uint32 OSystem_PalmBase::getMillis() {
	return TimGetTicks() * 1000 / SysTicksPerSecond();
}

void OSystem_PalmBase::delayMillis(uint msecs) {
	Int32 delay = (SysTicksPerSecond() * msecs) / 1000;

	if (delay > 0)
		SysTaskDelay(delay);
}

void OSystem_PalmBase::setTimerCallback(TimerProc callback, int timer) {
	if (callback != NULL) {
		_timer.duration = timer;
		_timer.nextExpiry = getMillis() + timer;
		_timer.callback = callback;
		_timer.active = true;
	} else {
		_timer.active = false;
	}
}
