/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
#include "palm.h"

#include "scumm.h"
#include "common/scaler.h"

#include "cd_msa.h"
#include "cd_default.h"

OSystem *OSystem_PALMOS_create() {
	return OSystem_PALMOS::create();
}

OSystem *OSystem_PALMOS::create() {
	return new OSystem_PALMOS();
}

uint32 OSystem_PALMOS::get_msecs() {
	uint32 ticks = TimGetTicks();
	ticks *= (1000/SysTicksPerSecond());
	return ticks;

}

void OSystem_PALMOS::delay_msecs(uint msecs) {
	UInt32 current = get_msecs();
	UInt32 last = current + msecs;

	do {
		timer_handler(current);
		current = get_msecs();
	} while (current < last);
}

OSystem::MutexRef OSystem_PALMOS::createMutex() {return NULL;}
void OSystem_PALMOS::lockMutex(MutexRef mutex) {}
void OSystem_PALMOS::unlockMutex(MutexRef mutex) {}
void OSystem_PALMOS::deleteMutex(MutexRef mutex) {}

void OSystem_PALMOS::battery_handler() {
	UInt16 voltage, warnThreshold, criticalThreshold;
	voltage = SysBatteryInfo(false, &warnThreshold, &criticalThreshold, NULL, NULL, NULL, NULL);

	if (voltage <= warnThreshold) {
		if (!_showBatLow) {
			_showBatLow = true;
			draw1BitGfx(kDrawBatLow, (getWidth() >> 1), -16, true);
		}
	} else {
		if (_showBatLow) {
			_showBatLow = false;
			draw1BitGfx(kDrawBatLow, (getWidth() >> 1), -16, false);
		}
	}

	if (voltage <= criticalThreshold) {
		EventType event;
		event.eType = keyDownEvent;
		event.data.keyDown.chr = vchrAutoOff;
		event.data.keyDown.modifiers = commandKeyMask;
		EvtAddUniqueEventToQueue(&event, 0, true);
	}
}

void OSystem_PALMOS::timer_handler(UInt32 current_msecs) {
	if (_timer.active && (current_msecs >= _timer.nextExpiry)) {
		_timer.duration = _timer.callback(_timer.duration);
		_timer.nextExpiry = current_msecs + _timer.duration;
	}
}

void OSystem_PALMOS::setTimerCallback(TimerProc callback, int timer) {
	if (callback != NULL) {
		_timer.duration = timer;
		_timer.nextExpiry = get_msecs() + timer;
		_timer.callback = callback;
		_timer.active = true;
	} else {
		_timer.active = false;
	}
}

void OSystem_PALMOS::setWindowCaption(const char *caption) {
	Char *loading = "Loading, please wait\0";
	Coord h = FntLineHeight() + 2;
	Coord w, y;

	// quick erase the screen
	WinScreenLock(winLockErase);
	WinScreenUnlock();
	
	WinSetTextColor(255);
	WinSetForeColor(255);

	if (_useHRmode) {
		y = 160 - (h >> 1) - 10;
		HRFntSetFont(gVars->HRrefNum,hrTinyBoldFont);
		w = FntCharsWidth(loading,StrLen(loading));
		w = (320 - w) >> 1;
		HRWinDrawChars(gVars->HRrefNum, loading, StrLen(loading), w, y + h);

		HRFntSetFont(gVars->HRrefNum,hrTinyFont);
		w = FntCharsWidth(caption, StrLen(caption));
		w = (320 - w) >> 1;
		HRWinDrawChars(gVars->HRrefNum, caption, StrLen(caption), w, y);
	} else {
		Err e;
		BitmapTypeV3 *bmp2P;
		BitmapType *bmp1P = BmpCreate(320, (h << 1), 8, NULL, &e);
		WinHandle tmpH = WinCreateBitmapWindow(bmp1P, &e);

		WinSetDrawWindow(tmpH);

		FntSetFont(boldFont);
		y = 80 - (h >> 2) - 5;
		w = FntCharsWidth(loading, StrLen(loading));
		w = (320 - w) >> 1;
		WinDrawChars(loading, StrLen(loading), w, 0 + h);

		FntSetFont(stdFont);
		w = FntCharsWidth(caption, StrLen(caption));
		w = (320 - w) >> 1;
		WinDrawChars(caption, StrLen(caption), w, 0);

		WinSetDrawWindow(WinGetDisplayWindow());
		bmp2P = BmpCreateBitmapV3(bmp1P, kDensityDouble, BmpGetBits(bmp1P), NULL);
		WinDrawBitmap((BitmapPtr)bmp2P, 0, y);

		BmpDelete((BitmapPtr)bmp2P);
		WinDeleteWindow(tmpH,0);
		BmpDelete(bmp1P);
	}
}

bool OSystem_PALMOS::hasFeature(Feature f) {
	return	(f == kFeatureFullscreenMode && OPTIONS_TST(kOptModeWide)) ||
			(f == kFeatureAspectRatioCorrection);
}

void OSystem_PALMOS::setFeatureState(Feature f, bool enable) {
	switch (f) {
		case kFeatureFullscreenMode:
			if (_gfxLoaded)
				if (OPTIONS_TST(kOptModeWide) && _initMode != GFX_WIDE) {
					_fullscreen = enable;
					hotswap_gfx_mode(_mode);
				}
			break;

		case kFeatureAspectRatioCorrection:
			if (_mode == GFX_WIDE && OPTIONS_TST(kOptDeviceZodiac)) {
				_adjustAspectRatio ^= true;
				ClearScreen();
			}
			break;
	}
}

bool OSystem_PALMOS::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureAspectRatioCorrection:
		return _adjustAspectRatio;
	default:
		return false;
	}
}

void OSystem_PALMOS::quit() {
	// There is no exit(.) function under PalmOS, to exit an app
	// we need to send an 'exit' event to the event handler
	// and then the system return to the launcher. But this event
	// is not handled by the main loop and so we need to force exit.
	// In other systems like Windows ScummVM exit immediatly and so this doesn't appear.

	if (_quitCount)
		return;

#ifndef DISABLE_SCUMM
	if (_selfQuit && Scumm::g_scumm)
		Scumm::g_scumm->_quit = true;
#endif

	free(_currentPalette);
	free(_mouseBackupP);
	free(_mouseDataP);
	
	_PnoFree(&_arm[PNO_COPY].pnoDesc, _arm[PNO_COPY].pnoPtr);

	if (_cdPlayer) {
		_cdPlayer->release();
		_cdPlayer = NULL;
	}

	unload_gfx_mode();
	_quitCount++;
	exit(1);
}

OSystem_PALMOS::OSystem_PALMOS() {
	_quitCount = 0;
	_selfQuit = false; // prevent illegal access to g_scumm
	_current_shake_pos = 0;
	_new_shake_pos = 0;

	_paletteDirtyStart = 0;
	_paletteDirtyEnd = 0;
	
	memset(&_sound, 0, sizeof(SoundDataType));
	
	_currentPalette = NULL;
	_modeChanged = false;
	_gfxLoaded = false;

	_lastKeyPressed = kLastKeyNone;
	_lastKeyRepeat = 100;
	_lastKeyModifier = MD_NONE;
	
	_useNumPad = false;
	_showBatLow = false;

	// mouse
	memset(&_mouseOldState,0,sizeof(MousePos));
	memset(&_mouseCurState,0,sizeof(MousePos));
	_mouseDrawn = false;
	_mouseBackupP = NULL;
	_mouseVisible = false;
	_mouseDataP = NULL;

	_currentPalette = (RGBColorType*)calloc(sizeof(RGBColorType), 256);
	_mouseBackupP = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H);
	
	// overlay
	_tmpScreenP = NULL;
	_tmpBackupP = NULL;
	
	// HiRes
	_useHRmode	= (gVars->HRrefNum != sysInvalidRefNum);
	
	// ARM
	memset(_arm, 0, sizeof(_arm));
	_arm[PNO_COPY].pnoPtr = _PnoInit(ARM_OCOPYRECT, &_arm[PNO_COPY].pnoDesc);
	
	// enable cdrom ?
	_cdPlayer = NULL;
	if (gVars->CD.enable) {
		switch (gVars->CD.driver) {
			case 0:	// No CD
				_cdPlayer = new DefaultCDPlayer(this);
				break;
			case 1:	// MSA Library
				_cdPlayer = new MsaCDPlayer(this);
				break;
		}
		
		if (_cdPlayer) {
			if (!_cdPlayer->init()) {
				_cdPlayer->release();
				_cdPlayer = NULL;
			}
		}
	}
	
	// sound
	memset(&_sound,0,sizeof(SoundDataType));

	// init
	_vibrate = gVars->vibrator;
}

void ClearScreen() {
	RGBColorType rgb	= {0, 0,0,0};
	RectangleType r		= {0, 0, gVars->screenFullWidth, gVars->screenFullHeight};
	WinSetForeColor(WinRGBToIndex(&rgb));
	WinDrawRectangle(&r,0);
}
