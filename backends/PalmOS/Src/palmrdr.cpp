/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
#include "palm.h"

#include "rumble.h"
#include "shared.h"

#ifndef DISABLE_TAPWAVE
#include "tapwave.h"
#include "i_zodiac.h"
#endif

#include "init_arm.h"
#include "arm/native.h"
#include "arm/macros.h"

static void rumblePack(Boolean active) {
	if (!gVars->vibrator)
		return;

	RumbleRun(active);
}


void OSystem_PALMOS::setShakePos(int shake_pos) {
	_new_shake_pos = shake_pos;

	if (shake_pos == 0)
		rumblePack(false);
}

#ifndef DISABLE_TAPWAVE
void OSystem_PALMOS::updateScreen_wideZodiac() {
	Err e;
	TwGfxRectType destRect, srcRect;
	TwGfxPointType destPoint = {0,0};
	Int32 h;

	if (_screenHeight == 200)
		h = (_adjustAspectRatio ? 300 : 320);	// 320x200
	else
		h = (_adjustAspectRatio ? 360 : 320);	// 640x480, 320x240

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = _screenWidth;
	srcRect.h = _screenHeight;

	destRect.w = 480;
	destRect.h = h;
	destRect.x = 0;
	destRect.y = ((gVars->screenFullHeight - destRect.h) / 2);

	_screenOffset.y = destRect.y;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		RectangleType r;

		WinSetBackColor(0);
		// clear top
		RctSetRectangle(&r, 0, _screenOffset.y, 480, _new_shake_pos);
		WinFillRectangle(&r, 0);
		// clear bottom
		RctSetRectangle(&r, 0, _screenOffset.y + h, 480, _current_shake_pos);
		WinFillRectangle(&r, 0);

		destRect.y += _new_shake_pos;
		_screenOffset.y += _new_shake_pos;

		rumblePack(_new_shake_pos >= 3);
		_current_shake_pos = _new_shake_pos;
	}

	// update screen
	WinSetDrawWindow(WinGetDisplayWindow());
	e = TwGfxDrawPalmBitmap((TwGfxSurfaceType *)_twSrc, &destPoint, (BitmapPtr)_twBmpV3);
	e = TwGfxStretchBlt2((TwGfxSurfaceType *)_twDst, &destRect, (TwGfxSurfaceType *)_twSrc, &srcRect, twGfxStretchFast| (gVars->filter ? twGfxStretchSmooth : 0));
}
#endif

void OSystem_PALMOS::updateScreen_widePortrait() {
	UInt8 *dst = _screenP + _screenOffset.y;
	UInt8 *src = _offScreenP + WIDE_PITCH - 1;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		RectangleType r;

		WinSetBackColor(0);
		// clear top
		RctSetRectangle(&r, _screenOffset.y, 0, _new_shake_pos, 480);
		WinFillRectangle(&r, 0);
		// clear bottom
		RctSetRectangle(&r, _screenOffset.y + 300, 0, _current_shake_pos, 480);
		WinFillRectangle(&r, 0);

		dst += _new_shake_pos;

		rumblePack(_new_shake_pos >= 3);
		_current_shake_pos = _new_shake_pos;
	}

	// update screen
	ARM_START(WideType)
		ARM_INIT(COMMON_WPORTRAIT)
		ARM_ADDM(dst)
		ARM_ADDM(src)
		ARM_CALL(ARM_COMMON, PNO_DATA())
	ARM_CONTINUE()
	{
		Coord x, y;
		UInt8 *src2 = src;

		for (x = 0; x < WIDE_HALF_WIDTH; x++) {
			for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
				*dst++ = *src;
				src += WIDE_PITCH;
				*dst++ = *src;
				*dst++ = *src;
				src += WIDE_PITCH;
			}
			src = --src2;
			dst += 20; // we draw 200pix scaled to 1.5 = 300, screen width=320, so next is 20

			for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
				*dst++ = *src;
				src += WIDE_PITCH;
				*dst++ = *src;
				*dst++ = *src;
				src += WIDE_PITCH;
			}
			src = --src2;
			dst += 20;

			MemMove(dst, dst - WIDE_PITCH, 300);	// 300 = 200 x 1.5
			dst += WIDE_PITCH;
		}
	}

	WinScreenUnlock();
	_screenP = WinScreenLock(winLockCopy) + _screenOffset.addr;
}

void OSystem_PALMOS::updateScreen_wideLandscape() {
	UInt8 *dst = _screenP;
	UInt8 *src = _offScreenP;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		RectangleType r;

		WinSetBackColor(0);
		// clear top
		RctSetRectangle(&r, 0, _screenOffset.y, 480, _new_shake_pos);
		WinFillRectangle(&r, 0);
		// clear bottom
		RctSetRectangle(&r, 0, _screenOffset.y + 300, 480, _current_shake_pos);
		WinFillRectangle(&r, 0);

		dst += _new_shake_pos * 480;

		rumblePack(_new_shake_pos >= 3);
		_current_shake_pos = _new_shake_pos;
	}

	// update screen
	ARM_START(WideType)
		ARM_INIT(COMMON_WLANDSCAPE)
		ARM_ADDM(dst)
		ARM_ADDM(src)
		ARM_CALL(ARM_COMMON, PNO_DATA())
	ARM_CONTINUE()
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

void OSystem_PALMOS::updateScreen_flipping() {
	RectangleType r, dummy;
	Boolean shaked = false;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		RctSetRectangle(&r, _screenOffset.x, _screenOffset.y - _new_shake_pos, _screenWidth, _screenHeight + (_new_shake_pos << 2));

		if (_useHRmode)
			HRWinScrollRectangle(gVars->HRrefNum, &r, winDown, _new_shake_pos, &dummy);
		else
			WinScrollRectangle(&r, winDown, _new_shake_pos, &dummy);

		rumblePack(_new_shake_pos >= 3);
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

void OSystem_PALMOS::updateScreen_buffered() {
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

		rumblePack(_new_shake_pos >= 3);
		_current_shake_pos = _new_shake_pos;
	}

	// update screen
	RctSetRectangle(&r, 0, 0, _screenWidth, _screenHeight - _current_shake_pos);

	if (OPTIONS_TST(kOptModeHiDensity))
		WinCopyRectangle(_offScreenH, _screenH, &r, _screenOffset.x, _screenOffset.y + _current_shake_pos, winPaint);
	else
		HRWinCopyRectangle(gVars->HRrefNum, _offScreenH, _screenH, &r, _screenOffset.x, _screenOffset.y + _current_shake_pos, winPaint);
}

void OSystem_PALMOS::updateScreen_direct() {
	if (_current_shake_pos != _new_shake_pos) {
		rumblePack(_new_shake_pos >= 3);
		_current_shake_pos = _new_shake_pos;
	}
}
