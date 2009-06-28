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

#include "be_os5.h"
#include <HsKeyCommon.h>

OSystem_PalmOS5::OSystem_PalmOS5() : OSystem_PalmBase() {
	_sound.active = false;
	_setPalette = false;

	_workScreenH = NULL;
	_overlayH = NULL;
	_isSwitchable = false;
	_wasRotated = false;
	_cursorPaletteDisabled = true;

	MemSet(&_soundEx, sizeof(SoundExType), 0);
	_soundEx.sound = &_sound;
}

void OSystem_PalmOS5::calc_scale() {
	for (int y = 0; y < _screenDest.h; y++) {
		int ys = y * _screenHeight / _screenDest.h;
		_scaleTableY[y] = ys * _screenWidth;
	}

	for (int x = 0; x < _screenDest.w; x++) {
		int xs = x * _screenWidth / _screenDest.w;
		_scaleTableX[x] = xs;
	}
}

void OSystem_PalmOS5::calc_rect(Boolean fullscreen) {
	Int32 w, h;

	if (fullscreen) {
		w = (_ratio.adjustAspect == kRatioWidth) ? _ratio.width : gVars->screenFullWidth;
		h = (_ratio.adjustAspect == kRatioHeight) ? _ratio.height : gVars->screenFullHeight;

		_screenOffset.x = (_ratio.adjustAspect == kRatioWidth) ? (gVars->screenFullWidth - _ratio.width) / 2 : 0;
		_screenOffset.y = (_ratio.adjustAspect == kRatioHeight) ? (gVars->screenFullHeight - _ratio.height) / 2 : 0;

	} else {
		w = gVars->screenWidth;
		h = gVars->screenHeight * _screenHeight / _screenWidth;

		_screenOffset.x = 0;
		_screenOffset.y = (gVars->screenHeight - h) / 2;
	}

	_screenDest.w = w;
	_screenDest.h = h;
}

void OSystem_PalmOS5::int_initBackend() {
	_keyExtra.bitUp		= keyBitRockerUp|keyBitPageUp;
	_keyExtra.bitDown	= keyBitRockerDown|keyBitPageDown;
	_keyExtra.bitLeft	= keyBitRockerLeft;
	_keyExtra.bitRight	= keyBitRockerRight;
	_keyExtra.bitActionA	= keyBitHard3;
	_keyExtra.bitActionB	= keyBitHard4;
}

bool OSystem_PalmOS5::hasFeature(Feature f) {
	switch (f) {
		case kFeatureCursorHasPalette:
			return true;
	}

	return false;
}

void OSystem_PalmOS5::setFeatureState(Feature f, bool enable) {
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
				//calc_rect(true);
				hotswap_gfx_mode(_mode);
//				TwGfxSetClip(_palmScreenP, &_dstRect);
				clear_screen();
			}
			break;
	}
}

void OSystem_PalmOS5::setWindowCaption(const char *caption) {
FIXME/TODO: Convert caption from ISO Latin 1 to "right" encoding ?
If the output encoding is unclear or conversion impossible,
then one could just skip over any chars > 0x7F and display the rest

	Err e;
	Char buf[64];
	Coord w, y, h = FntLineHeight() + 2;
	const Char *loading = "Loading, please wait\0";

	// allocate bitmap
	BitmapTypeV3 *bmp2P;
	BitmapType *bmp1P = BmpCreate(320, (h * 3), 8, NULL, &e);
	WinHandle tmpH = WinCreateBitmapWindow(bmp1P, &e);

	WinSetDrawWindow(tmpH);
	WinSetBackColor(0);
	WinSetTextColor(255);
	WinEraseWindow();

	// loading message
	FntSetFont(boldFont);
	w = FntCharsWidth(loading, StrLen(loading));
	w = (320 - w) / 2;
	WinDrawChars(loading, StrLen(loading), w, 0 + h);

	// caption
	FntSetFont(stdFont);
	w = FntCharsWidth(caption, StrLen(caption));
	w = (320 - w) / 2;
	WinDrawChars(caption, StrLen(caption), w, 0);

	// memory size
	StrPrintF(buf, "memory : %ld KB", gVars->startupMemory);
	w = FntCharsWidth(buf, StrLen(buf));
	w = (320 - w) / 2;
	WinDrawChars(buf, StrLen(buf), w, h * 2);

	// set the bitmap as v3
	bmp2P = BmpCreateBitmapV3(bmp1P, kDensityDouble, BmpGetBits(bmp1P), NULL);
	y = (80 - (h / 4) - 5);

	// draw it
	WinSetDrawWindow(WinGetDisplayWindow());
	WinEraseWindow();
	WinDrawBitmap((BitmapPtr)bmp2P, 0, y);

	// free
	WinDeleteWindow(tmpH, 0);
	BmpDelete((BitmapPtr)bmp2P);
	BmpDelete(bmp1P);
}
