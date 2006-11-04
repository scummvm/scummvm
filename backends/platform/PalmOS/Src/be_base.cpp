/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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

#include "be_base.h"
#include "be_save.h"
#include "common/config-file.h"
#include "common/config-manager.h"
#include "backends/timer/default/default-timer.h"
#include "sound/mixer.h"

OSystem_PalmBase::OSystem_PalmBase() {
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
	_setPalette = true;
	
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
	
	_saveMgr = 0;
	_timerMgr = 0;
	_mixerMgr = 0;
	
	_mouseDataP = NULL;
	_mouseVisible = false;
	_mouseDrawn = false;
	MemSet(&_keyMouse, sizeof(_keyMouse), 0);
	MemSet(&_mouseCurState, sizeof(_mouseCurState), 0);
	MemSet(&_mouseOldState, sizeof(_mouseOldState), 0);
	MemSet(&_timer, sizeof(TimerType), 0);
	MemSet(&_sound, sizeof(SoundType), 0);
	
	_keyMouseRepeat = 0;
	_keyMouseDelay = (gVars->arrowKeys) ? computeMsecs(125) : computeMsecs(25);
}

static int timer_handler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

void OSystem_PalmBase::initBackend() {
	if (gVars->autoSave != -1)
		ConfMan.setInt("autosave_period", gVars->autoSave);

	_keyMouse.bitUp		= keyBitPageUp;
	_keyMouse.bitDown	= keyBitPageDown;
	_keyMouse.bitLeft	= keyBitHard1;
	_keyMouse.bitRight	= keyBitHard2;
	_keyMouse.bitButLeft= keyBitHard3;
	_keyMouse.hasMore	= false;

	int_initBackend();
	_keyMouseMask = (_keyMouse.bitUp | _keyMouse.bitDown | _keyMouse.bitLeft | _keyMouse.bitRight | _keyMouse.bitButLeft);

	// Create the savefile manager, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_saveMgr == 0) {
		_saveMgr = new PalmSaveFileManager();
	}

	// Create and hook up the mixer, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_mixerMgr == 0) {
		_mixerMgr = new Audio::Mixer();
		setSoundCallback(Audio::Mixer::mixCallback, _mixerMgr);
	}

	// Create and hook up the timer manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_timerMgr == 0) {
		_timerMgr = new DefaultTimerManager();
		setTimerCallback(::timer_handler, 10);
	}

	OSystem::initBackend();
}

uint32 OSystem_PalmBase::getMillis() {
	return TimGetTicks() * 1000 / SysTicksPerSecond();
}

void OSystem_PalmBase::delayMillis(uint msecs) {
	Int32 delay = computeMsecs(msecs);

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

void OSystem_PalmBase::quit() {
	int_quit();
	clearSoundCallback();
	unload_gfx_mode();
	
	delete _saveMgr;
	delete _timerMgr;
	delete _mixerMgr;
	
	exit(0);
}

Common::SaveFileManager *OSystem_PalmBase::getSavefileManager() {
	return _saveMgr;
}

Audio::Mixer * OSystem_PalmBase::getMixer() {
	return _mixerMgr;
}

Common::TimerManager * OSystem_PalmBase::getTimerManager() {
	return _timerMgr;
}
