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

#ifdef PALMOS_68K
#	include <BmpGlue.h>
#endif

/*
 * Graphics modes
 *
 */

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Normal", GFX_NORMAL},
	{"wide", "Wide", GFX_WIDE},
	{0, 0, 0}
};

int OSystem_PalmBase::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

const OSystem::GraphicsMode *OSystem_PalmBase::getSupportedGraphicsModes() const {
	return int_getSupportedGraphicsModes();
}

const OSystem::GraphicsMode *OSystem_PalmBase::int_getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int OSystem_PalmBase::getGraphicsMode() const {
	return _mode;
}

bool OSystem_PalmBase::setGraphicsMode(int mode) {
	switch(mode) {
	case GFX_NORMAL:
	case GFX_WIDE:
		_setMode = mode;
		break;

	default:
		warning("unknown gfx mode %d", mode);
		_setMode = GFX_NORMAL;
		return false;
	}

	return true;
}

/*
 * Init
 *
 */

void OSystem_PalmBase::initSize(uint w, uint h) {
	_screenWidth	= w;
	_screenHeight	= h;

	_screenDest.w = _screenWidth;
	_screenDest.h = _screenHeight;

	unload_gfx_mode();
	_mode = _setMode;
	load_gfx_mode();

	warpMouse(200, 150);
	int_initSize(w, h);
}

/*
 * Palette
 *
 */
void OSystem_PalmBase::setPalette(const byte *colors, uint start, uint num) {
	RGBColorType *base = _currentPalette + start;

	for(uint i = 0; i < num; i++) {
		base[i].r = colors[0];
		base[i].g = colors[1];
		base[i].b = colors[2];
		
		extras_palette(i + start, colors[0], colors[1], colors[2]);
		colors += 4;
	}

	if (start < _paletteDirtyStart)
		_paletteDirtyStart = start;

	if (start + num > _paletteDirtyEnd)
		_paletteDirtyEnd = start + num;
}

void OSystem_PalmBase::grabPalette(byte *colors, uint start, uint num) {
	const RGBColorType *base = _currentPalette + start;

	for (uint i = 0; i < num; ++i) {
		colors[i * 4] = base[i].r;
		colors[i * 4 + 1] = base[i].g;
		colors[i * 4 + 2] = base[i].b;
		colors[i * 4 + 3] = 0xFF;
	}
}

/*
 * Screen
 *
 */

void OSystem_PalmBase::setShakePos(int shakeOffset) {
	_new_shake_pos = shakeOffset;
	int_setShakePos(shakeOffset);
}

void OSystem_PalmBase::updateScreen() {
	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly. 
	if (_paletteDirtyEnd != 0 && _setPalette) {
		WinSetDrawWindow(WinGetDisplayWindow());
		WinPalette(winPaletteSet, _paletteDirtyStart, _paletteDirtyEnd - _paletteDirtyStart, _currentPalette + _paletteDirtyStart);
		_paletteDirtyEnd = 0;
		//_redawOSD = true;

#ifdef PALMOS_68Ks
		UInt8 oldCol;
		oldCol = gVars->indicator.on;
		gVars->indicator.on = RGBToColor(0,255,0);

		if (oldCol != gVars->indicator.on)
			_redrawOSD = true;

/*		 {
			// redraw if needed
			if (_lastKeyModifier)
				draw1BitGfx((kDrawKeyState + _lastKeyModifier - 1), 2, getHeight() + 2, true);

			if(_useNumPad)
				draw1BitGfx(kDrawNumPad, (getWidth() >> 1) - 32, getHeight() + 2, true);

			if (_showBatLow)
				draw1BitGfx(kDrawBatLow, (getWidth() >> 1), -16, true);		
		}*/	
#endif
	}
	if (_redawOSD) {
		_redawOSD = false;
		draw_osd(kDrawBatLow, _screenDest.w - 18, -16, _showBatLow, 2);
		draw_osd(kDrawFight, _screenDest.w - 34, _screenDest.h + 2, (_useNumPad && !_overlayVisible), 1);
	}
	int_updateScreen();

}

void OSystem_PalmBase::clearScreen() {
	WinSetDrawWindow(WinGetDisplayWindow());
	WinSetBackColor(RGBToColor(0,0,0));
	WinEraseWindow();
}

void OSystem_PalmBase::draw_osd(UInt16 id, Int32 x, Int32 y, Boolean show, UInt8 color) {
//return;

#ifdef PALMOS_68K
	MemHandle hTemp = DmGetResource(bitmapRsc, id);
#else
	MemHandle hTemp = DmGetResource('abmp', id + 100);
#endif

	if (hTemp) {
		/*static const UInt32 pal[3] = {
			(TwGfxComponentsToPackedRGB(0,255,0)),
			(TwGfxComponentsToPackedRGB(255,255,0)),
			(TwGfxComponentsToPackedRGB(255,0,0))
		};*/

		BitmapType *bmTemp;
		bmTemp	= (BitmapType *)MemHandleLock(hTemp);

		Coord w, h;
#ifdef PALMOS_68K
		BmpGlueGetDimensions(bmTemp, &w, &h, 0);
#else
		BmpGetDimensions(bmTemp, &w, &h, 0);
#endif
		PointType dst = { _screenOffset.x + x, _screenOffset.y + y };
		RectangleType r = { dst.x, dst.y, w, h };

		RectangleType c;

		UInt16 old = WinSetCoordinateSystem(kCoordinatesNative);
		WinGetClip(&c);
		WinResetClip();
		if (show) {
			WinSetDrawWindow(_screenH);
			WinSetBackColor(0);
			WinSetForeColor(120);	// pal[color]
			WinFillRectangle(&r, 0);
			WinSetDrawMode(winOverlay);
			WinDrawBitmap(bmTemp, 0, 0);
			WinSetDrawMode(winPaint);

		} else {
			WinSetBackColor(0);
			WinFillRectangle(&r, 0);
		}
		WinSetClip(&c);
		WinSetCoordinateSystem(old);

		MemPtrUnlock(bmTemp);
		DmReleaseResource(hTemp);
	}

/*	MemHandle hTemp = DmGetResource(bitmapRsc, id);
	
	if (hTemp) {
		BitmapType *bmTemp;
		UInt32 *bmData;
		UInt8 ih, iw, ib;
		Coord w, h;
		Int16 blocks, next;

		UInt8 *scr = _screenP + x + _screenPitch * y;
		bmTemp	= (BitmapType *)MemHandleLock(hTemp);
		bmData	= (UInt32 *)BmpGetBits(bmTemp);

#ifdef PALMOS_68K
		BmpGlueGetDimensions(bmTemp, &w, &h, 0);
#else
		BmpGetDimensions(bmTemp, &w, &h, 0);
#endif

		blocks = w >> 5;
		next = w - (blocks << 5);

		if (next)
			blocks++;

		for (ih = 0; ih < h; ih++) {			// line
			for (ib = 0; ib < blocks; ib++) {	// 32pix block
				next = w - (ib << 5);
				next = MIN(next, (Coord)32);
		
				for (iw = 0; iw < next; iw++) {	// row
					*scr++ = ((*bmData & (1 << (31 - iw))) && show) ?
						gVars->indicator.on :
						gVars->indicator.off;
				}

				bmData++;
			}
			scr += _screenPitch - w;
		}

		MemHandleUnlock(hTemp);
		DmReleaseResource(hTemp);
	}*/
}
