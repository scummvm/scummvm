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

#include "be_zodiac.h"

void OSystem_PalmZodiac::int_initBackend() {
	_sound.active = false;


	_gfxH = NULL;
	_overlayP = NULL;
	_palmScreenP = NULL;
	_tmpScreenP = NULL;
	_stretched = false;
	_tmpScreenP = NULL;

	_keyMouse.bitUp		= keyBitRockerUp;
	_keyMouse.bitDown	= keyBitRockerDown;
	_keyMouse.bitLeft	= keyBitRockerLeft;
	_keyMouse.bitRight	= keyBitRockerRight;
	_keyMouse.bitButLeft= keyBitRockerCenter;
}
/*
uint32 OSystem_PalmZodiac::getMillis() {
	return TwGetMicroSeconds();
}

void OSystem_PalmZodiac::delayMillis(uint msecs) {
	Int32 delay = (SysTicksPerSecond() * msecs) / 1000;

	if (delay > 0)
		SysTaskDelay(delay);
}
*/

void OSystem_PalmZodiac::calc_rect(Boolean fullscreen) {
	Int32 w, h;
	
	if (fullscreen) {
		w = (_ratio.adjustAspect == kRatioWidth) ? _ratio.width : gVars->screenFullWidth;
		h = (_ratio.adjustAspect == kRatioHeight) ? _ratio.height : gVars->screenFullHeight;

		_screenOffset.x = (_ratio.adjustAspect == kRatioWidth) ? (gVars->screenFullWidth - _ratio.width) / 2 : 0;
		_screenOffset.y = (_ratio.adjustAspect == kRatioHeight) ? (gVars->screenFullHeight - _ratio.height) / 2 : 0;

	} else {
		w = gVars->screenWidth;
		h = gVars->screenHeight;

		_screenOffset.x = 0;
		_screenOffset.y = 0;		
	}

	_dstRect.x = _screenOffset.x;
	_dstRect.y = _screenOffset.y;
	_dstRect.w = w;
	_dstRect.h = h;
	
	_screenDest.w = _dstRect.w;
	_screenDest.h = _dstRect.h;
}

void OSystem_PalmZodiac::setFeatureState(Feature f, bool enable) {
	switch (f) {
/*		case kFeatureFullscreenMode:
			if (_gfxLoaded)
				if (OPTIONS_TST(kOptModeWide) && _initMode != GFX_WIDE) {
					_fullscreen = enable;
					hotswap_gfx_mode(_mode);
				}
			break;
*/
		case kFeatureAspectRatioCorrection:
			if (_mode == GFX_WIDE) {
				_ratio.adjustAspect = (_ratio.adjustAspect + 1) % 3;
				calc_rect(true);
				TwGfxSetClip(_palmScreenP, &_dstRect);
				clearScreen();
			}
			break;
	}
}
