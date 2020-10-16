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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#if (defined(GPH_DEVICE) || defined(DINGUX)) && defined(USE_SCALERS)

#include "backends/graphics/downscalesdl/downscalesdl-graphics.h"
#include "backends/events/sdl/sdl-events.h"
#include "graphics/scaler/downscaler.h"
#include "graphics/scaler/aspect.h"
#include "common/mutex.h"
#include "common/textconsole.h"

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Standard", GFX_NORMAL},
	{0, 0, 0}
};

DownscaleSdlGraphicsManager::DownscaleSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window)
	: SurfaceSdlGraphicsManager(sdlEventSource, window) {
}

const OSystem::GraphicsMode *DownscaleSdlGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int DownscaleSdlGraphicsManager::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

int DownscaleSdlGraphicsManager::getGraphicsModeScale(int mode) const {
	int scale;
	switch (mode) {
	case GFX_NORMAL:
	case GFX_HALF:
		scale = 1;
		break;
	default:
		scale = -1;
	}

	return scale;
}

ScalerProc *DownscaleSdlGraphicsManager::getGraphicsScalerProc(int mode) const {
	ScalerProc *newScalerProc = 0;
	switch (_videoMode.mode) {
	case GFX_NORMAL:
		newScalerProc = Normal1x;
		break;
	case GFX_HALF:
		newScalerProc = DownscaleAllByHalf;
		break;
	}

	return newScalerProc;
}

void DownscaleSdlGraphicsManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	assert(_transactionMode == kTransactionActive);

	_gameScreenShakeXOffset = 0;
	_gameScreenShakeYOffset = 0;

#ifdef USE_RGB_COLOR
	// Avoid redundant format changes
	Graphics::PixelFormat newFormat;
	if (!format)
		newFormat = Graphics::PixelFormat::createFormatCLUT8();
	else
		newFormat = *format;

	assert(newFormat.bytesPerPixel > 0);

	if (newFormat != _videoMode.format) {
		_videoMode.format = newFormat;
		_transactionDetails.formatChanged = true;
		_screenFormat = newFormat;
	}
#endif


	// Avoid redundant res changes
	if ((int)w == _videoMode.screenWidth && (int)h == _videoMode.screenHeight)
		return;

	_videoMode.screenWidth = w;
	_videoMode.screenHeight = h;

	if (w > 320 || h > 240) {
		setGraphicsMode(GFX_HALF);
		setGraphicsModeIntern();
		_window->grabMouse(!getWindow()->mouseIsGrabbed());
	}

	_transactionDetails.sizeChanged = true;
}

void DownscaleSdlGraphicsManager::drawMouse() {
	if (!_cursorVisible || !_mouseSurface || !_mouseCurState.w || !_mouseCurState.h) {
		_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;
		return;
	}

	SDL_Rect dst;
	int scale;
	int hotX, hotY;

	const Common::Point virtualCursor = convertWindowToVirtual(_cursorX, _cursorY);

	if (_videoMode.mode == GFX_HALF && !_overlayVisible) {
		dst.x = virtualCursor.x / 2;
		dst.y = virtualCursor.y / 2;
	} else {
		dst.x = virtualCursor.x;
		dst.y = virtualCursor.y;
	}

	if (!_overlayVisible) {
		scale = _videoMode.scaleFactor;
		dst.w = _mouseCurState.vW;
		dst.h = _mouseCurState.vH;
		hotX = _mouseCurState.vHotX;
		hotY = _mouseCurState.vHotY;
	} else {
		scale = 1;
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

	dst.x += _currentShakeXOffset;
	dst.y += _currentShakeYOffset;

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		dst.y = real2Aspect(dst.y);

	dst.x = scale * dst.x - _mouseCurState.rHotX;
	dst.y = scale * dst.y - _mouseCurState.rHotY;
	dst.w = _mouseCurState.rW;
	dst.h = _mouseCurState.rH;

	// Note that SDL_BlitSurface() and addDirtyRect() will both perform any
	// clipping necessary

	if (SDL_BlitSurface(_mouseSurface, nullptr, _hwScreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	// The screen will be updated using real surface coordinates, i.e.
	// they will not be scaled or aspect-ratio corrected.
	addDirtyRect(dst.x, dst.y, dst.w, dst.h, true);
}

void DownscaleSdlGraphicsManager::undrawMouse() {
	const int x = _mouseBackup.x;
	const int y = _mouseBackup.y;

	// When we switch bigger overlay off mouse jumps. Argh!
	// This is intended to prevent undrawing offscreen mouse
	if (!_overlayVisible && (x >= _videoMode.screenWidth || y >= _videoMode.screenHeight))
		return;

	if (_mouseBackup.w != 0 && _mouseBackup.h != 0) {
		if (_videoMode.mode == GFX_HALF && !_overlayVisible) {
			addDirtyRect(x * 2, y * 2, _mouseBackup.w * 2, _mouseBackup.h * 2);
		} else {
			addDirtyRect(x, y, _mouseBackup.w, _mouseBackup.h);
		}
	}
}

void DownscaleSdlGraphicsManager::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;
	ScalerProc *scalerProc;
	int scale1;

#if defined(DEBUG)
	assert(_hwScreen != NULL);
	assert(_hwScreen->map->sw_data != NULL);
#endif

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakeXOffset != _gameScreenShakeXOffset ||
		(_cursorNeedsRedraw && _mouseBackup.x <= _currentShakeXOffset)) {
		SDL_Rect blackrect = {0, 0, (Uint16)(_gameScreenShakeXOffset * _videoMode.scaleFactor), (Uint16)(_videoMode.screenHeight * _videoMode.scaleFactor)};

		if (_videoMode.aspectRatioCorrection && !_overlayVisible)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwScreen, &blackrect, 0);

		_currentShakeXOffset = _gameScreenShakeXOffset;

		_forceRedraw = true;
	}
	if (_currentShakeYOffset != _gameScreenShakeYOffset ||
		(_cursorNeedsRedraw && _mouseBackup.y <= _currentShakeYOffset)) {
		SDL_Rect blackrect = {0, 0, (Uint16)(_videoMode.screenWidth * _videoMode.scaleFactor), (Uint16)(_gameScreenShakeYOffset * _videoMode.scaleFactor)};

		if (_videoMode.aspectRatioCorrection && !_overlayVisible)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwScreen, &blackrect, 0);

		_currentShakeYOffset = _gameScreenShakeYOffset;

		_forceRedraw = true;
	}

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_screen && _paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart,
		              _paletteDirtyStart,
		              _paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceRedraw = true;
	}

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
	if (_cursorNeedsRedraw)
		undrawMouse();

#ifdef USE_OSD
	updateOSD();
#endif

	// Force a full redraw if requested
	if (_forceRedraw) {
		_numDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = width;
		_dirtyRectList[0].h = height;

#ifdef GPH_DEVICE
		// HACK: Make sure the full hardware screen is wiped clean.
		SDL_FillRect(_hwScreen, NULL, 0);
#endif
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0 || _cursorNeedsRedraw) {
		SDL_Rect *r;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			dst = *r;
			dst.x++;    // Shift rect by one since 2xSai needs to access the data around
			dst.y++;    // any pixel to scale it, and we want to avoid mem access crashes.

			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		SDL_LockSurface(_hwScreen);

		srcPitch = srcSurf->pitch;
		dstPitch = _hwScreen->pitch;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			int dst_y = r->y + _currentShakeYOffset;
			int dst_h = 0;
			int dst_w = 0;
			int orig_dst_y = 0;
			int dst_x = r->x + _currentShakeXOffset;
			int src_y;
			int src_x;

			if (dst_x < width && dst_y < height) {
				dst_w = r->w;
				if (dst_w > width - dst_x)
					dst_w = width - dst_x;

				dst_h = r->h;
				if (dst_h > height - dst_y)
					dst_h = height - dst_y;

				orig_dst_y = dst_y;
				src_x = dst_x;
				src_y = dst_y;

				if (_videoMode.aspectRatioCorrection && !_overlayVisible)
					dst_y = real2Aspect(dst_y);

				assert(scalerProc != NULL);

				if (_videoMode.mode == GFX_HALF && scalerProc == DownscaleAllByHalf) {
					if (dst_x % 2 == 1) {
						dst_x--;
						dst_w++;
					}
					if (dst_y % 2 == 1) {
						dst_y--;
						dst_h++;
					}
					src_x = dst_x;
					src_y = dst_y;
					dst_x = dst_x / 2;
					dst_y = dst_y / 2;

					scalerProc((byte *)srcSurf->pixels + (src_x * 2 + 2) + (src_y + 1) * srcPitch, srcPitch,
					           (byte *)_hwScreen->pixels + dst_x * 2 + dst_y * dstPitch, dstPitch, dst_w, dst_h);
				} else {
					scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
					           (byte *)_hwScreen->pixels + dst_x * 2 + dst_y * dstPitch, dstPitch, dst_w, dst_h);
				}
			}

			if (_videoMode.mode == GFX_HALF && scalerProc == DownscaleAllByHalf) {
				r->w = dst_w / 2;
				r->h = dst_h / 2;
			} else {
				r->w = dst_w;
				r->h = dst_h;
			}

			r->x = dst_x;
			r->y = dst_y;

#ifdef USE_SCALERS
			if (_videoMode.aspectRatioCorrection && orig_dst_y < height && !_overlayVisible)
				r->h = stretch200To240((uint8 *) _hwScreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y * scale1, _videoMode.filtering);
#endif
		}
		SDL_UnlockSurface(srcSurf);
		SDL_UnlockSurface(_hwScreen);

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceRedraw) {
			_dirtyRectList[0].x = 0;
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].w = (_videoMode.mode == GFX_HALF) ? _videoMode.hardwareWidth / 2 : _videoMode.hardwareWidth;
			_dirtyRectList[0].h = (_videoMode.mode == GFX_HALF) ? _videoMode.hardwareHeight / 2 : _videoMode.hardwareHeight;
		}

		drawMouse();

#ifdef USE_OSD
		drawOSD();
#endif

		// Finally, blit all our changes to the screen
		SDL_UpdateRects(_hwScreen, _numDirtyRects, _dirtyRectList);
	}

	_numDirtyRects = 0;
	_forceRedraw = false;
	_cursorNeedsRedraw = false;
}

void DownscaleSdlGraphicsManager::showOverlay() {
	if (_videoMode.mode == GFX_HALF) {
		_cursorX /= 2;
		_cursorY /= 2;
	}
	SurfaceSdlGraphicsManager::showOverlay();
}

void DownscaleSdlGraphicsManager::hideOverlay() {
	if (_videoMode.mode == GFX_HALF) {
		_cursorX *= 2;
		_cursorY *= 2;
	}
	SurfaceSdlGraphicsManager::hideOverlay();
}

void DownscaleSdlGraphicsManager::setupHardwareSize() {
	debug("Game ScreenMode = %d*%d", _videoMode.screenWidth, _videoMode.screenHeight);

	// Forcefully disable aspect ratio correction for games
	// which starts with a native 240px height resolution.
	// This fixes games with weird resolutions, like MM Nes (256x240)
	if (_videoMode.screenHeight == 240) {
		_videoMode.aspectRatioCorrection = false;
	}

	if (_videoMode.screenWidth > 320 || _videoMode.screenHeight > 240) {
		_videoMode.aspectRatioCorrection = false;
		setGraphicsMode(GFX_HALF);
		debug("GraphicsMode set to HALF");
	} else {
		setGraphicsMode(GFX_NORMAL);
		debug("GraphicsMode set to NORMAL");
	}

	if ((_videoMode.mode == GFX_HALF) && !_overlayVisible) {
		_videoMode.overlayWidth = _videoMode.screenWidth / 2;
		_videoMode.overlayHeight = _videoMode.screenHeight / 2;

		_videoMode.hardwareWidth = _videoMode.screenWidth / 2;
		_videoMode.hardwareHeight = _videoMode.screenHeight / 2;
	} else {
		SurfaceSdlGraphicsManager::setupHardwareSize();
	}
}

void DownscaleSdlGraphicsManager::warpMouse(int x, int y) {
	if (_cursorX != x || _cursorY != y) {
		if (_videoMode.mode == GFX_HALF && !_overlayVisible) {
			x /= 2;
			y /= 2;
		}
	}
	SurfaceSdlGraphicsManager::warpMouse(x, y);
}

void DownscaleSdlGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	if (!_overlayVisible) {
		if (_videoMode.mode == GFX_HALF) {
			point.x *= 2;
			point.y *= 2;
		}
		point.x /= _videoMode.scaleFactor;
		point.y /= _videoMode.scaleFactor;
		if (_videoMode.aspectRatioCorrection)
			point.y = aspect2Real(point.y);
	}
}

#endif
