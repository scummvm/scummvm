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

#include "be_base.h"
#include "common/config-file.h"
#include "common/config-manager.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/fs/palmos/palmos-fs-factory.h"
#include "sound/mixer_intern.h"

#define DEFAULT_SAVE_PATH "/PALM/Programs/ScummVM/Saved/"


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

	_wasKey = false;
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
	_mouseBackupP = NULL;
	_mouseVisible = false;
	_mouseDrawn = false;
	MemSet(&_keyExtra, sizeof(_keyExtra), 0);
	MemSet(&_mouseCurState, sizeof(_mouseCurState), 0);
	MemSet(&_mouseOldState, sizeof(_mouseOldState), 0);
	MemSet(&_timer, sizeof(TimerType), 0);
	MemSet(&_sound, sizeof(SoundType), 0);

	_keyExtraRepeat = 0;
	_keyExtraPressed = 0;
	_keyExtraDelay = (gVars->arrowKeys) ? computeMsecs(125) : computeMsecs(25);
}

static int timer_handler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

void OSystem_PalmBase::initBackend() {
	if (gVars->autoSave != -1)
		ConfMan.setInt("autosave_period", gVars->autoSave);

	_keyExtra.bitUp		= keyBitPageUp;
	_keyExtra.bitDown	= keyBitPageDown;
	_keyExtra.bitLeft	= keyBitHard2;
	_keyExtra.bitRight	= keyBitHard3;

	int_initBackend();
	_keyExtraMask = (_keyExtra.bitUp | _keyExtra.bitDown | _keyExtra.bitLeft | _keyExtra.bitRight | _keyExtra.bitActionA | _keyExtra.bitActionB);

	// Create the savefile manager, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_saveMgr == 0) {
		_saveMgr = new DefaultSaveFileManager(DEFAULT_SAVE_PATH);
	}

	// Create and hook up the mixer, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_mixerMgr == 0) {
		_mixerMgr = new Audio::MixerImpl(this);
		setSoundCallback(0, _mixerMgr);
//		setSoundCallback(Audio::Mixer::mixCallback, _mixerMgr);
	}

	// Create and hook up the timer manager, if none exists yet (we check for
	// this to allow subclasses to provide their own).
	if (_timerMgr == 0) {
		_timerMgr = new DefaultTimerManager();
		setTimerCallback(::timer_handler, 10);
	}

	OSystem::initBackend();
}

void OSystem_PalmBase::getTimeAndDate(TimeDate &td) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
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

	if (_mouseDataP) {
		MemPtrFree(_mouseBackupP);
		MemPtrFree(_mouseDataP);
	}

	delete _saveMgr;
	delete _timerMgr;
	delete _mixerMgr;

	exit(0);
}

Common::SaveFileManager *OSystem_PalmBase::getSavefileManager() {
	return _saveMgr;
}

Audio::Mixer *OSystem_PalmBase::getMixer() {
	return _mixerMgr;
}

Common::TimerManager *OSystem_PalmBase::getTimerManager() {
	return _timerMgr;
}

FilesystemFactory *OSystem_PalmBase::getFilesystemFactory() {
	return &PalmOSFilesystemFactory::instance();
}


#define PALMOS_CONFIG_FILE "/PALM/Programs/ScummVM/scummvm.ini"

Common::SeekableReadStream *OSystem_PalmBase::createConfigReadStream() {
	Common::FSNode file(PALMOS_CONFIG_FILE);
	return file.createReadStream();
}

Common::WriteStream *OSystem_PalmBase::createConfigWriteStream() {
	Common::FSNode file(PALMOS_CONFIG_FILE);
	return file.createWriteStream();
}
