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

#include "vibrate.h"
#include "shared.h"

#ifndef DISABLE_TAPWAVE
// Tapwave code will come here
#endif

void OSystem_PALMOS::set_shake_pos(int shake_pos) {
	_new_shake_pos = shake_pos;
	
	if (shake_pos == 0 && _vibrate) {
		Boolean active = false;
		HwrVibrateAttributes(1, kHwrVibrateActive, &active);
	}
}

#ifndef DISABLE_TAPWAVE
// Tapwave code will come here
#endif

void OSystem_PALMOS::updateScreen__wide_portrait() {
	UInt8 *dst = _screenP + _screenOffset.y;
	UInt8 *src1 = _offScreenP + WIDE_PITCH - 1;

#ifndef DISABLE_ARM
	if (OPTIONS_TST(kOptDeviceARM)) {
		OSysWideType userData = { dst, src1 };
		_PnoCall(&_arm[PNO_WIDE].pnoDesc, &userData);
		
#ifdef DEBUG_ARM
	} else if (OPTIONS_TST(kOptDeviceProcX86)) {
		DataOSysWideType userData = { kOSysWidePortrait, dst, src1 };
		UInt32 result = PceNativeCall((NativeFuncType*)"ARMlet.dll\0ARMlet_Main", &userData);		
#endif

	} else
#endif	
	{
		Coord x, y;
		UInt8 *src2 = src1;

		for (x = 0; x < WIDE_HALF_WIDTH; x++) {
			for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
				*dst++ = *src1;
				src1 += WIDE_PITCH;
				*dst++ = *src1;
				*dst++ = *src1;
				src1 += WIDE_PITCH;
			}
			src1 = --src2;
			dst += 20; // we draw 200pix scaled to 1.5 = 300, screen width=320, so next is 20

			for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
				*dst++ = *src1;
				src1 += WIDE_PITCH;
				*dst++ = *src1;
				*dst++ = *src1;
				src1 += WIDE_PITCH;
			}
			src1 = --src2;
			dst += 20;

			MemMove(dst, dst - WIDE_PITCH, 300);	// 300 = 200 x 1.5
			dst += WIDE_PITCH;
		}
	}

	WinScreenUnlock();
	_screenP = WinScreenLock(winLockCopy) + _screenOffset.addr;
}

void OSystem_PALMOS::updateScreen__wide_landscape() {
	UInt8 *dst = _screenP;
	UInt8 *src = _offScreenP;

#ifndef DISABLE_ARM
	if (OPTIONS_TST(kOptDeviceARM)) {
		OSysWideType userData = { dst, src };
		_PnoCall(&_arm[PNO_WIDE].pnoDesc, &userData);

#ifdef DEBUG_ARM
	} else if (OPTIONS_TST(kOptDeviceProcX86)) {
		DataOSysWideType userData = { kOSysWideLandscape, dst, src };
		UInt32 result = PceNativeCall((NativeFuncType*)"ARMlet.dll\0ARMlet_Main", &userData);		
#endif
	} else
#endif	
	{
		Coord x, y;

		for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
			// draw 2 lines
			for (x = 0; x < WIDE_FULL_WIDTH; x++) {
				*dst++ = *src++;
				*dst++ = *src;
				*dst++ = *src++;
			}
			// copy the second to the next line
			MemMove(dst, dst - 480, 480);
			dst += 480;
		}
	}

	WinScreenUnlock();
	_screenP = WinScreenLock(winLockCopy) + _screenOffset.addr;
}

void OSystem_PALMOS::updateScreen__flipping() {
	RectangleType r, dummy;
	Boolean shaked = false;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		RctSetRectangle(&r, _screenOffset.x, _screenOffset.y - _new_shake_pos, _screenWidth, _screenHeight + (_new_shake_pos << 2));

		if (_useHRmode)
			HRWinScrollRectangle(gVars->HRrefNum, &r, winDown, _new_shake_pos, &dummy);
		else
			WinScrollRectangle(&r, winDown, _new_shake_pos, &dummy);

		if (_vibrate) {
			Boolean active = (_new_shake_pos >= 3);
			HwrVibrateAttributes(1, kHwrVibrateActive, &active);
		}

		_current_shake_pos = _new_shake_pos;
		shaked = true;
	}

	// update screen
	WinScreenUnlock();
	_offScreenP = WinScreenLock(winLockCopy) + _screenOffset.addr;
	_screenP = _offScreenP;

	if (shaked) {
		if (_useHRmode)
			HRWinScrollRectangle(gVars->HRrefNum, &r, winUp, _new_shake_pos, &dummy);
		else
			WinScrollRectangle(&r, winUp, _new_shake_pos, &dummy);
	}

}

void OSystem_PALMOS::updateScreen__buffered() {
	RectangleType r;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		if (_new_shake_pos != 0) {
			RctSetRectangle(&r, _screenOffset.x, _screenOffset.y + _screenHeight, _screenWidth, _new_shake_pos);

			if (OPTIONS_TST(kOptModeHiDensity))
				WinCopyRectangle(_screenH, _screenH, &r, _screenOffset.x, _screenOffset.y, winPaint);
			else
				HRWinCopyRectangle(gVars->HRrefNum, _screenH, _screenH, &r, _screenOffset.x, _screenOffset.y, winPaint);
		}

		if (_vibrate) {
			Boolean active = (_new_shake_pos >= 3);
			HwrVibrateAttributes(1, kHwrVibrateActive, &active);
		}

		_current_shake_pos = _new_shake_pos;
	}

	// update screen
	RctSetRectangle(&r, 0, 0, _screenWidth, _screenHeight - _current_shake_pos);

	if (OPTIONS_TST(kOptModeHiDensity))
		WinCopyRectangle(_offScreenH, _screenH, &r, _screenOffset.x, _screenOffset.y + _current_shake_pos, winPaint);
	else
		HRWinCopyRectangle(gVars->HRrefNum, _offScreenH, _screenH, &r, _screenOffset.x, _screenOffset.y + _current_shake_pos, winPaint);
}

void OSystem_PALMOS::updateScreen__direct() {
	if (_current_shake_pos != _new_shake_pos) {
		if (_vibrate) {
			Boolean active = (_new_shake_pos >= 3);
			HwrVibrateAttributes(1, kHwrVibrateActive, &active);
		}	
		_current_shake_pos = _new_shake_pos;
	}
}