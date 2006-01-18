/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "be_os5.h"
#include "oscalls.h"
#include "palmdefs.h"

#ifndef __TWKEYS_H__
#include <PalmNavigator.h>
#include <HsKeyCommon.h>
#endif

static TimerExType _timerEx;

OSystem_PalmOS5::OSystem_PalmOS5() : OSystem_PalmBase() {
	_sound.active = false;
	_timerEx.timerID = 0;
	_timerEx.timer = &_timer;

#ifdef PALMOS_ARM
	// CHECK : is this ok for OS5 too ?
	if (HALHRTimerTicksPerSecond(&_timerEx.ticks))
		_timerEx.ticks = SysTicksPerSecond();
#endif
}

#ifdef PALMOS_ARM

static SYSTEM_CALLBACK void timer_handler(void *userDataP) {
	CALLBACK_PROLOGUE
	TimerExPtr _timerEx = (TimerExPtr)userDataP;
	TimerPtr _timer = _timerEx->timer;
	_timer->duration = _timer->callback(_timer->duration);
	KALTimerSet(_timerEx->timerID, (_timer->duration * _timerEx->ticks / 1000));
	CALLBACK_EPILOGUE
}

void OSystem_PalmOS5::setTimerCallback(TimerProc callback, int timer) {
	if (_timer.active && _timerEx.timerID)
		KALTimerDelete(_timerEx.timerID);

	if (callback != NULL) {
		Err e;
		CALLBACK_INIT(_timerEx);
		_timer.duration = timer;
		_timer.callback = callback;

		// create the timer
		e = KALTimerCreate(&_timerEx.timerID, appFileCreator, &::timer_handler, &_timerEx);
		if (!e) {
			e = KALTimerSet(_timerEx.timerID, (timer * _timerEx.ticks / 1000));
			if (e) KALTimerDelete(_timerEx.timerID);
		}
		_timer.active = (!e);

	} else {
		_timer.active = false;
	}
	
	if (!_timer.active)
		_timerEx.timerID = 0;
}

#endif

void OSystem_PalmOS5::int_initBackend() {
	if (OPTIONS_TST(kOpt5WayNavigatorV1)) {
		_keyMouse.bitUp		= keyBitPageUp;
		_keyMouse.bitDown	= keyBitPageDown;
		_keyMouse.bitLeft	= keyBitNavLeft;
		_keyMouse.bitRight	= keyBitNavRight;
		_keyMouse.bitButLeft= keyBitNavSelect;
		_keyMouse.hasMore	= true;

	} else if (OPTIONS_TST(kOpt5WayNavigatorV2)) {
		_keyMouse.bitUp		= keyBitRockerUp;
		_keyMouse.bitDown	= keyBitRockerDown;
		_keyMouse.bitLeft	= keyBitRockerLeft;
		_keyMouse.bitRight	= keyBitRockerRight;
		_keyMouse.bitButLeft= keyBitRockerCenter;
		_keyMouse.hasMore	= true;
	}
}

void OSystem_PalmOS5::int_quit() {
#ifdef PALMOS_ARM
	if (_timerEx.timerID)
		KALTimerDelete(_timerEx.timerID);
#endif
	clearSoundCallback();
	unload_gfx_mode();
	exit(0);
}
