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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/sdl/graphics.cpp $
 * $Id: graphics.cpp 44495 2009-09-30 16:16:53Z fingolfin $
 *
 */

#include "backends/platform/samsungtv/sdl.h"
#include "common/mutex.h"
#include "common/util.h"
#include "common/list.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/scaler.h"
#include "graphics/surface.h"

#if defined(SAMSUNGTV)

// Table of relative scalers magnitudes
// [definedScale - 1][scaleFactor - 1]
static ScalerProc *scalersMagn[3][3] = {
#ifndef DISABLE_SCALERS
	{ Normal1x, AdvMame2x, AdvMame3x },
	{ Normal1x, Normal1x, Normal1o5x },
	{ Normal1x, Normal1x, Normal1x }
#else // remove dependencies on other scalers
	{ Normal1x, Normal1x, Normal1x },
	{ Normal1x, Normal1x, Normal1x },
	{ Normal1x, Normal1x, Normal1x }
#endif
};

#ifndef DISABLE_SCALERS
static int cursorStretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY);
#endif

Common::List<Graphics::PixelFormat> OSystem_SDL_SamsungTV::getSupportedFormats() {
	static Common::List<Graphics::PixelFormat>list;
	list.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	list.push_back(Graphics::PixelFormat::createFormatCLUT8());
	return list;
}

void OSystem_SDL_SamsungTV::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	assert(_transactionMode == kTransactionActive);

	//avoid redundant format changes
	Graphics::PixelFormat newFormat;
	if (!format)
		newFormat = Graphics::PixelFormat::createFormatCLUT8();
	else
		newFormat = *format;

	assert(newFormat.bytesPerPixel > 0);

	if (newFormat != _videoMode.format)
	{
		_videoMode.format = newFormat;
		_transactionDetails.formatChanged = true;
		_screenFormat = newFormat;
	}

	// Avoid redundant res changes
	if ((int)w == _videoMode.screenWidth && (int)h == _videoMode.screenHeight)
		return;

	_videoMode.screenWidth = w;
	_videoMode.screenHeight = h;

	_cksumNum = (w * h / (8 * 8));

	_transactionDetails.sizeChanged = true;

	free(_dirtyChecksums);
	_dirtyChecksums = (uint32 *)calloc(_cksumNum * 2, sizeof(uint32));
}


static void fixupResolutionForAspectRatio(AspectRatio desiredAspectRatio, int &width, int &height) {
	assert(&width != &height);

	// FIXME: TV SDL return empty list. However TV accept any resolution
	// Consider list fixed list or make calculation. For now it's disabled.
//	if (desiredAspectRatio.isAuto())
		return;
	
	int kw = desiredAspectRatio.kw();
	int kh = desiredAspectRatio.kh();

	const int w = width;
	const int h = height;

	SDL_Rect const* const*availableModes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_SWSURFACE); //TODO : Maybe specify a pixel format
	assert(availableModes);

	const SDL_Rect *bestMode = NULL;
	uint bestMetric = (uint)-1; // Metric is wasted space
	while (const SDL_Rect *mode = *availableModes++) {
		if (mode->w < w)
			continue;
		if (mode->h < h)
			continue;
		if (mode->h * kw != mode->w * kh)
			continue;
		//printf("%d %d\n", mode->w, mode->h);

		uint metric = mode->w * mode->h - w * h;
		if (metric > bestMetric)
			continue;

		bestMetric = metric;
		bestMode = mode;
	}

	if (!bestMode) {
		warning("Unable to enforce the desired aspect ratio!");
		return;
	}
	//printf("%d %d\n", bestMode->w, bestMode->h);
	width = bestMode->w;
	height = bestMode->h;
}

bool OSystem_SDL_SamsungTV::loadGFXMode() {
	assert(_inited);
	_forceFull = true;

	_videoMode.overlayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.overlayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

	if (_videoMode.screenHeight != 200 && _videoMode.screenHeight != 400)
		_videoMode.aspectRatioCorrection = false;

	if (_videoMode.aspectRatioCorrection)
		_videoMode.overlayHeight = real2Aspect(_videoMode.overlayHeight);

	_videoMode.hardwareWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.hardwareHeight = effectiveScreenHeight();

	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight, 
						_screenFormat.bytesPerPixel << 3, 
						((1 << _screenFormat.rBits()) - 1) << _screenFormat.rShift ,
						((1 << _screenFormat.gBits()) - 1) << _screenFormat.gShift ,
						((1 << _screenFormat.bBits()) - 1) << _screenFormat.bShift ,
						((1 << _screenFormat.aBits()) - 1) << _screenFormat.aShift );
	if (_screen == NULL)
		error("allocating _screen failed");

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	if (_videoMode.fullscreen) {
		fixupResolutionForAspectRatio(_videoMode.desiredAspectRatio, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	}

	_hwscreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 32,
		_videoMode.fullscreen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
	);

	if (_hwscreen == NULL) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwscreen is hosed.

		if (!_oldVideoMode.setup) {
			warning("SDL_SetVideoMode says we can't switch to that mode (%s)", SDL_GetError());
			quit();
		} else {
			return false;
		}
	}

	_prehwscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.hardwareWidth, _videoMode.hardwareHeight,
						16,
						0x0000f800,
						0x000007e0,
						0x0000001f,
						0x00000000);
	if (_prehwscreen == NULL)
		error("allocating _prehwscreen failed");

	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Need some extra bytes around when using 2xSaI
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth + 3, _videoMode.screenHeight + 3,
						16,
						0x0000f800,
						0x000007e0,
						0x0000001f,
						0x00000000);

	if (_tmpscreen == NULL)
		error("allocating _tmpscreen failed");

	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth, _videoMode.overlayHeight,
						16,
						0x0000f800,
						0x000007e0,
						0x0000001f,
						0x00000000);

	if (_overlayscreen == NULL)
		error("allocating _overlayscreen failed");

	_overlayFormat.bytesPerPixel = _overlayscreen->format->BytesPerPixel;

	_overlayFormat.rLoss = _overlayscreen->format->Rloss;
	_overlayFormat.gLoss = _overlayscreen->format->Gloss;
	_overlayFormat.bLoss = _overlayscreen->format->Bloss;
	_overlayFormat.aLoss = _overlayscreen->format->Aloss;

	_overlayFormat.rShift = _overlayscreen->format->Rshift;
	_overlayFormat.gShift = _overlayscreen->format->Gshift;
	_overlayFormat.bShift = _overlayscreen->format->Bshift;
	_overlayFormat.aShift = _overlayscreen->format->Ashift;

	_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth + 3, _videoMode.overlayHeight + 3,
						16,
						0x0000f800,
						0x000007e0,
						0x0000001f,
						0x00000000);

	if (_tmpscreen2 == NULL)
		error("allocating _tmpscreen2 failed");

#ifdef USE_OSD
	_osdSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_hwscreen->w,
						_hwscreen->h,
						32,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);
	if (_osdSurface == NULL)
		error("allocating _osdSurface failed");
	SDL_SetColorKey(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kOSDColorKey);
#endif

	// keyboard cursor control, some other better place for it?
	_km.x_max = _videoMode.screenWidth * _videoMode.scaleFactor - 1;
	_km.y_max = effectiveScreenHeight() - 1;
	_km.delay_time = 25;
	_km.last_time = 0;

	InitScalers(565);

	return true;
}

void OSystem_SDL_SamsungTV::unloadGFXMode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL;
	}

	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen);
		_hwscreen = NULL;
	}

	if (_prehwscreen) {
		SDL_FreeSurface(_prehwscreen);
		_prehwscreen = NULL;
	}

	if (_tmpscreen) {
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}

	if (_tmpscreen2) {
		SDL_FreeSurface(_tmpscreen2);
		_tmpscreen2 = NULL;
	}

	if (_overlayscreen) {
		SDL_FreeSurface(_overlayscreen);
		_overlayscreen = NULL;
	}

#ifdef USE_OSD
	if (_osdSurface) {
		SDL_FreeSurface(_osdSurface);
		_osdSurface = NULL;
	}
#endif
	DestroyScalers();
}

bool OSystem_SDL_SamsungTV::hotswapGFXMode() {
	if (!_screen)
		return false;

	// Keep around the old _screen & _overlayscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	_screen = NULL;
	SDL_Surface *old_overlayscreen = _overlayscreen;
	_overlayscreen = NULL;

	// Release the HW screen surface
	SDL_FreeSurface(_hwscreen); _hwscreen = NULL;

	SDL_FreeSurface(_prehwscreen); _prehwscreen = NULL;

	SDL_FreeSurface(_tmpscreen); _tmpscreen = NULL;
	SDL_FreeSurface(_tmpscreen2); _tmpscreen2 = NULL;

#ifdef USE_OSD
	// Release the OSD surface
	SDL_FreeSurface(_osdSurface); _osdSurface = NULL;
#endif

	// Setup the new GFX mode
	if (!loadGFXMode()) {
		unloadGFXMode();

		_screen = old_screen;
		_overlayscreen = old_overlayscreen;

		return false;
	}

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, NULL, _screen, NULL);
	SDL_BlitSurface(old_overlayscreen, NULL, _overlayscreen, NULL);

	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	SDL_FreeSurface(old_overlayscreen);

	// Update cursor to new scale
	blitCursor();

	// Blit everything to the screen
	internUpdateScreen();

	return true;
}

void OSystem_SDL_SamsungTV::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;
	ScalerProc *scalerProc;
	int scale1;

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _videoMode.screenWidth * _videoMode.scaleFactor, _newShakePos * _videoMode.scaleFactor};

		if (_videoMode.aspectRatioCorrection && !_overlayVisible)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_prehwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_screen && _paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart,
			_paletteDirtyStart,
			_paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceFull = true;
	}

#ifdef USE_OSD
	// OSD visible (i.e. non-transparent)?
	if (_osdAlpha != SDL_ALPHA_TRANSPARENT) {
		// Updated alpha value
		const int diff = SDL_GetTicks() - _osdFadeStartTime;
		if (diff > 0) {
			if (diff >= kOSDFadeOutDuration) {
				// Back to full transparency
				_osdAlpha = SDL_ALPHA_TRANSPARENT;
			} else {
				// Do a linear fade out...
				const int startAlpha = SDL_ALPHA_TRANSPARENT + kOSDInitialAlpha * (SDL_ALPHA_OPAQUE - SDL_ALPHA_TRANSPARENT) / 100;
				_osdAlpha = startAlpha + diff * (SDL_ALPHA_TRANSPARENT - startAlpha) / kOSDFadeOutDuration;
			}
			SDL_SetAlpha(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, _osdAlpha);
			_forceFull = true;
		}
	}
#endif

	if (!_overlayVisible) {
		origSurf = _screen;
		srcSurf = _tmpscreen;
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
		scalerProc = _scalerProc;
		scale1 = _videoMode.scaleFactor;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
		scalerProc = Normal1x;

		scale1 = 1;
	}

	// Add the area covered by the mouse cursor to the list of dirty rects if
	// we have to redraw the mouse.
	if (_mouseNeedsRedraw)
		undrawMouse();

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = width;
		_dirtyRectList[0].h = height;
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0 || _mouseNeedsRedraw) {
		SDL_Rect *r;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			dst = *r;
			dst.x++;	// Shift rect by one since 2xSai needs to access the data around
			dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.

			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		SDL_LockSurface(_prehwscreen);

		srcPitch = srcSurf->pitch;
		dstPitch = _prehwscreen->pitch;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			register int dst_y = r->y + _currentShakePos;
			register int dst_h = 0;
			register int orig_dst_y = 0;
			register int rx1 = r->x * scale1;

			if (dst_y < height) {
				dst_h = r->h;
				if (dst_h > height - dst_y)
					dst_h = height - dst_y;

				orig_dst_y = dst_y;
				dst_y = dst_y * scale1;

				if (_videoMode.aspectRatioCorrection && !_overlayVisible)
					dst_y = real2Aspect(dst_y);

				assert(scalerProc != NULL);
				scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
						   (byte *)_prehwscreen->pixels + rx1 * 2 + dst_y * dstPitch, dstPitch, r->w, dst_h);
			}

			r->x = rx1;
			r->y = dst_y;
			r->w = r->w * scale1;
			r->h = dst_h * scale1;

#ifndef DISABLE_SCALERS
			if (_videoMode.aspectRatioCorrection && orig_dst_y < height && !_overlayVisible)
				r->h = stretch200To240((uint8 *)_prehwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y * scale1);
#endif
		}
		SDL_UnlockSurface(srcSurf);
		SDL_UnlockSurface(_prehwscreen);

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].h = effectiveScreenHeight();
		}

		drawMouse();

#ifdef USE_OSD
		if (_osdAlpha != SDL_ALPHA_TRANSPARENT) {
			SDL_BlitSurface(_osdSurface, 0, _prehwscreen, 0);
		}
#endif
		// Finally, blit all our changes to the screen
		// tv sdl port can not take advantage of dirty rects
		//SDL_UpdateRects(_prehwscreen, _numDirtyRects, _dirtyRectList);
	}

	SDL_BlitSurface(_prehwscreen, 0, _hwscreen, 0);
	SDL_UpdateRect(_hwscreen, 0, 0, _hwscreen->w, _hwscreen->h);

	_numDirtyRects = 0;
	_forceFull = false;
	_mouseNeedsRedraw = false;
}

void OSystem_SDL_SamsungTV::setFullscreenMode(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _oldVideoMode.fullscreen == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.fullscreen = enable;
		_transactionDetails.needHotswap = true;
	}
}

void OSystem_SDL_SamsungTV::warpMouse(int x, int y) {
	int y1 = y;

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		y1 = real2Aspect(y);

	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		if (!_overlayVisible)
			generateMouseMoveEvent(x * _videoMode.scaleFactor, y1 * _videoMode.scaleFactor);
		else
			generateMouseMoveEvent(x, y1);

		setMousePos(x, y);
	}
}

void OSystem_SDL_SamsungTV::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
	if (!format)
		_cursorFormat = Graphics::PixelFormat::createFormatCLUT8();
	else if (format->bytesPerPixel <= _screenFormat.bytesPerPixel)
		_cursorFormat = *format;
	keycolor &= (1 << (_cursorFormat.bytesPerPixel << 3)) - 1;

	if (w == 0 || h == 0)
		return;

	_mouseCurState.hotX = hotspot_x;
	_mouseCurState.hotY = hotspot_y;

	_mouseKeyColor = keycolor;

	_cursorTargetScale = cursorTargetScale;

	if (_mouseCurState.w != (int)w || _mouseCurState.h != (int)h) {
		_mouseCurState.w = w;
		_mouseCurState.h = h;

		if (_mouseOrigSurface)
			SDL_FreeSurface(_mouseOrigSurface);

		// Allocate bigger surface because AdvMame2x adds black pixel at [0,0]
		_mouseOrigSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_mouseCurState.w + 2,
						_mouseCurState.h + 2,
						16,
						0x0000f800,
						0x000007e0,
						0x0000001f,
						0x00000000);

		if (_mouseOrigSurface == NULL)
			error("allocating _mouseOrigSurface failed");
		SDL_SetColorKey(_mouseOrigSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kMouseColorKey);
	}

	free(_mouseData);
	_mouseData = (byte *)malloc(w * h * _cursorFormat.bytesPerPixel);
	memcpy(_mouseData, buf, w * h * _cursorFormat.bytesPerPixel);

	blitCursor();
}

void OSystem_SDL_SamsungTV::blitCursor() {
	byte *dstPtr;
	const byte *srcPtr = _mouseData;
	uint32 color;
	int w, h, i, j;

	if (!_mouseOrigSurface || !_mouseData)
		return;

	_mouseNeedsRedraw = true;

	w = _mouseCurState.w;
	h = _mouseCurState.h;

	SDL_LockSurface(_mouseOrigSurface);

	// Make whole surface transparent
	for (i = 0; i < h + 2; i++) {
		dstPtr = (byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * i;
		for (j = 0; j < w + 2; j++) {
			*(uint16 *)dstPtr = kMouseColorKey;
			dstPtr += 2;
		}
	}

	// Draw from [1,1] since AdvMame2x adds artefact at 0,0
	dstPtr = (byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch + 2;

	SDL_Color *palette;

	if (_cursorPaletteDisabled)
		palette = _currentPalette;
	else
		palette = _cursorPalette;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			if (_cursorFormat.bytesPerPixel > 1) {
				if (_cursorFormat.bytesPerPixel == 2)
					color = *(uint16 *)srcPtr;
				else
					color = *(uint32 *)srcPtr;
				if (color != _mouseKeyColor) {	// transparent, don't draw
					uint8 r, g, b;
					_cursorFormat.colorToRGB(color, r, g, b);
					*(uint16 *)dstPtr = SDL_MapRGB(_mouseOrigSurface->format,
						r, g, b);
				}
				dstPtr += 2;
				srcPtr += _cursorFormat.bytesPerPixel;
			} else {
				color = *srcPtr;
				if (color != _mouseKeyColor) {	// transparent, don't draw
					*(uint16 *)dstPtr = SDL_MapRGB(_mouseOrigSurface->format,
						palette[color].r, palette[color].g, palette[color].b);
				}
				dstPtr += 2;
				srcPtr++;
			}
		}
		dstPtr += _mouseOrigSurface->pitch - w * 2;
	}

	int rW, rH;

	if (_cursorTargetScale >= _videoMode.scaleFactor) {
		// The cursor target scale is greater or equal to the scale at
		// which the rest of the screen is drawn. We do not downscale
		// the cursor image, we draw it at its original size. It will
		// appear too large on screen.

		rW = w;
		rH = h;
		_mouseCurState.rHotX = _mouseCurState.hotX;
		_mouseCurState.rHotY = _mouseCurState.hotY;

		// The virtual dimensions may be larger than the original.

		_mouseCurState.vW = w * _cursorTargetScale / _videoMode.scaleFactor;
		_mouseCurState.vH = h * _cursorTargetScale / _videoMode.scaleFactor;
		_mouseCurState.vHotX = _mouseCurState.hotX * _cursorTargetScale /
			_videoMode.scaleFactor;
		_mouseCurState.vHotY = _mouseCurState.hotY * _cursorTargetScale /
			_videoMode.scaleFactor;
	} else {
		// The cursor target scale is smaller than the scale at which
		// the rest of the screen is drawn. We scale up the cursor
		// image to make it appear correct.

		rW = w * _videoMode.scaleFactor / _cursorTargetScale;
		rH = h * _videoMode.scaleFactor / _cursorTargetScale;
		_mouseCurState.rHotX = _mouseCurState.hotX * _videoMode.scaleFactor /
			_cursorTargetScale;
		_mouseCurState.rHotY = _mouseCurState.hotY * _videoMode.scaleFactor /
			_cursorTargetScale;

		// The virtual dimensions will be the same as the original.

		_mouseCurState.vW = w;
		_mouseCurState.vH = h;
		_mouseCurState.vHotX = _mouseCurState.hotX;
		_mouseCurState.vHotY = _mouseCurState.hotY;
	}

#ifndef DISABLE_SCALERS
	int rH1 = rH; // store original to pass to aspect-correction function later
#endif

	if (_videoMode.aspectRatioCorrection && _cursorTargetScale == 1) {
		rH = real2Aspect(rH - 1) + 1;
		_mouseCurState.rHotY = real2Aspect(_mouseCurState.rHotY);
	}

	if (_mouseCurState.rW != rW || _mouseCurState.rH != rH) {
		_mouseCurState.rW = rW;
		_mouseCurState.rH = rH;

		if (_mouseSurface)
			SDL_FreeSurface(_mouseSurface);

		_mouseSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_mouseCurState.rW,
						_mouseCurState.rH,
						16,
						0x0000f800,
						0x000007e0,
						0x0000001f,
						0x00000000);

		if (_mouseSurface == NULL)
			error("allocating _mouseSurface failed");

		SDL_SetColorKey(_mouseSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kMouseColorKey);
	}

	SDL_LockSurface(_mouseSurface);

	ScalerProc *scalerProc;

	// If possible, use the same scaler for the cursor as for the rest of
	// the game. This only works well with the non-blurring scalers so we
	// actually only use the 1x, 1.5x, 2x and AdvMame scalers.

	if (_cursorTargetScale == 1 && (_videoMode.mode == GFX_DOUBLESIZE || _videoMode.mode == GFX_TRIPLESIZE))
		scalerProc = _scalerProc;
	else
		scalerProc = scalersMagn[_cursorTargetScale - 1][_videoMode.scaleFactor - 1];

	scalerProc((byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch + 2,
		_mouseOrigSurface->pitch, (byte *)_mouseSurface->pixels, _mouseSurface->pitch,
		_mouseCurState.w, _mouseCurState.h);

#ifndef DISABLE_SCALERS
	if (_videoMode.aspectRatioCorrection && _cursorTargetScale == 1)
		cursorStretch200To240((uint8 *)_mouseSurface->pixels, _mouseSurface->pitch, rW, rH1, 0, 0, 0);
#endif

	SDL_UnlockSurface(_mouseSurface);
	SDL_UnlockSurface(_mouseOrigSurface);
}

#ifndef DISABLE_SCALERS
// Basically it is kVeryFastAndUglyAspectMode of stretch200To240 from
// common/scale/aspect.cpp
static int cursorStretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY) {
	int maxDstY = real2Aspect(origSrcY + height - 1);
	int y;
	const uint8 *startSrcPtr = buf + srcX * 2 + (srcY - origSrcY) * pitch;
	uint8 *dstPtr = buf + srcX * 2 + maxDstY * pitch;

	for (y = maxDstY; y >= srcY; y--) {
		const uint8 *srcPtr = startSrcPtr + aspect2Real(y) * pitch;

		if (srcPtr == dstPtr)
			break;
		memcpy(dstPtr, srcPtr, width * 2);
		dstPtr -= pitch;
	}

	return 1 + maxDstY - srcY;
}
#endif

void OSystem_SDL_SamsungTV::drawMouse() {
	if (!_mouseVisible || !_mouseSurface) {
		_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;
		return;
	}

	SDL_Rect dst;
	int scale;
	int width, height;
	int hotX, hotY;

	dst.x = _mouseCurState.x;
	dst.y = _mouseCurState.y;

	if (!_overlayVisible) {
		scale = _videoMode.scaleFactor;
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
		dst.w = _mouseCurState.vW;
		dst.h = _mouseCurState.vH;
		hotX = _mouseCurState.vHotX;
		hotY = _mouseCurState.vHotY;
	} else {
		scale = 1;
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
		dst.w = _mouseCurState.rW;
		dst.h = _mouseCurState.rH;
		hotX = _mouseCurState.rHotX;
		hotY = _mouseCurState.rHotY;
	}

	// The mouse is undrawn using virtual coordinates, i.e. they may be
	// scaled and aspect-ratio corrected.

	_mouseBackup.x = dst.x - hotX;
	_mouseBackup.y = dst.y - hotY;
	_mouseBackup.w = dst.w;
	_mouseBackup.h = dst.h;

	// We draw the pre-scaled cursor image, so now we need to adjust for
	// scaling, shake position and aspect ratio correction manually.

	if (!_overlayVisible) {
		dst.y += _currentShakePos;
	}

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		dst.y = real2Aspect(dst.y);

	dst.x = scale * dst.x - _mouseCurState.rHotX;
	dst.y = scale * dst.y - _mouseCurState.rHotY;
	dst.w = _mouseCurState.rW;
	dst.h = _mouseCurState.rH;

	// Note that SDL_BlitSurface() and addDirtyRect() will both perform any
	// clipping necessary

	if (SDL_BlitSurface(_mouseSurface, NULL, _prehwscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	// The screen will be updated using real surface coordinates, i.e.
	// they will not be scaled or aspect-ratio corrected.

	addDirtyRect(dst.x, dst.y, dst.w, dst.h, true);
}

#endif
