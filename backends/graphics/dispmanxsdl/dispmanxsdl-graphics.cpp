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

//Needed for Raspberry Pi header incussion
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(RASPBERRYPI)

#include "backends/graphics/dispmanxsdl/dispmanxsdl-graphics.h"
#include "graphics/scaler/aspect.h"
#include "common/mutex.h"
#include "common/textconsole.h"

#include <bcm_host.h>

#define NUMPAGES 2

struct dispvarsStruct { 
	DISPMANX_DISPLAY_HANDLE_T display;
	DISPMANX_MODEINFO_T amode;
	DISPMANX_UPDATE_HANDLE_T update;
	DISPMANX_ELEMENT_HANDLE_T element;
	VC_IMAGE_TYPE_T pixFormat;
	VC_DISPMANX_ALPHA_T *alpha;    
	VC_RECT_T srcRect;
	VC_RECT_T dstRect; 
	VC_RECT_T bmpRect;
	uint vcImagePtr;
	uint screen;
	uint pitch;
	bool aspectRatioCorrection;
	void *pixmem;
	
	struct dispmanxPage *pages;
	struct dispmanxPage *nextPage;
	bool pageflipPending;

	pthread_cond_t vsyncCondition;	
	pthread_mutex_t pendingMutex;
};

struct dispmanxPage {
   	DISPMANX_RESOURCE_HANDLE_T resource;
};

DispmanXSdlGraphicsManager::DispmanXSdlGraphicsManager(SdlEventSource *sdlEventSource)
 : SurfaceSdlGraphicsManager(sdlEventSource) {
	_dispvars = new dispvarsStruct;
	DispmanXInit();
}

DispmanXSdlGraphicsManager::~DispmanXSdlGraphicsManager() {
	DispmanXVideoQuit();
	delete _dispvars;
}

void DispmanXSdlGraphicsManager::DispmanXInit() {
	_dispvars->screen = 0;
	_dispvars->vcImagePtr = 0;
	_dispvars->pages = (struct dispmanxPage *)calloc(NUMPAGES, sizeof(struct dispmanxPage));
	_dispvars->pageflipPending = 0;	
	_dispvars->nextPage = &_dispvars->pages[0];

	// Initialize mutex and condition variable objects
	pthread_mutex_init(&_dispvars->pendingMutex, NULL);
	pthread_cond_init(&_dispvars->vsyncCondition, NULL);
	
	// Before we call any vc_* function, we need to call this one.
	bcm_host_init();
	
	_dispvars->display = vc_dispmanx_display_open(_dispvars->screen);
}

void DispmanXSdlGraphicsManager::DispmanXSetup(int width, int height, int bpp) {
	DispmanXFreeResources();
	vc_dispmanx_display_get_info(_dispvars->display, &_dispvars->amode);

	_dispvars->pitch = width * (bpp / 8);
	_dispvars->pixFormat = VC_IMAGE_RGB565;	       

	// Transparency disabled
	VC_DISPMANX_ALPHA_T layerAlpha;    
	layerAlpha.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
	layerAlpha.opacity = 255;
	layerAlpha.mask = 0;
	_dispvars->alpha = &layerAlpha;	

	if (_dispvars->aspectRatioCorrection) {
		float orig_ratio = ((float)width / (float)height); 
		int dst_width = _dispvars->amode.height * orig_ratio;	
		
		// If we obtain an scaled image width that is bigger than the physical screen width,
		// then we keep the physical screen width as our maximun width.	
		if (dst_width > _dispvars->amode.width) 
			dst_width = _dispvars->amode.width;
		int dst_ypos  = (_dispvars->amode.width - dst_width) / 2; 
		vc_dispmanx_rect_set(&(_dispvars->dstRect), dst_ypos, 0, 
	   		dst_width, _dispvars->amode.height);
	} else {
		vc_dispmanx_rect_set(&(_dispvars->dstRect), 0, 0, 
	   		_dispvars->amode.width, _dispvars->amode.height);
	}

	// We configure the rects now	
	vc_dispmanx_rect_set(&(_dispvars->bmpRect), 0, 0, width, height);	
	vc_dispmanx_rect_set(&(_dispvars->srcRect), 0, 0, width << 16, height << 16);	

	for (int i = 0; i < NUMPAGES; i++)
		_dispvars->pages[i].resource = vc_dispmanx_resource_create(_dispvars->pixFormat, width, height, 
			&(_dispvars->vcImagePtr));
	
	// Add element
	_dispvars->update = vc_dispmanx_update_start(0);
	
	_dispvars->element = vc_dispmanx_element_add(_dispvars->update, _dispvars->display, 0,
		&(_dispvars->dstRect), _dispvars->pages[0].resource, &(_dispvars->srcRect), 
		DISPMANX_PROTECTION_NONE, _dispvars->alpha, 0, (DISPMANX_TRANSFORM_T)0);
	
	vc_dispmanx_update_submit_sync(_dispvars->update);		
}

void DispmanXVSyncCallback (DISPMANX_UPDATE_HANDLE_T u, void *arg) {
	struct dispvarsStruct *_dispvars = (struct dispvarsStruct*)arg;

	// Changing the page to write must be done before the signaling
	// so we have the right page in nextPage when update_main continues
	if (_dispvars->nextPage == &_dispvars->pages[0])	
	   _dispvars->nextPage = &_dispvars->pages[1];
	else 
	   _dispvars->nextPage = &_dispvars->pages[0];

	// These two things must be isolated "atomically" to avoid getting 
        // a false positive in the pending_mutex test in update_main.
	pthread_mutex_lock(&_dispvars->pendingMutex);
	
	pthread_cond_signal(&_dispvars->vsyncCondition);
	_dispvars->pageflipPending = false;	
	
	pthread_mutex_unlock(&_dispvars->pendingMutex);
		
}

void DispmanXSdlGraphicsManager::DispmanXUpdate() {	
	pthread_mutex_lock(&_dispvars->pendingMutex);

	if (_dispvars->pageflipPending) {
		pthread_cond_wait(&_dispvars->vsyncCondition, &_dispvars->pendingMutex);
	}
	
	pthread_mutex_unlock(&_dispvars->pendingMutex);

	// Frame blitting
	vc_dispmanx_resource_write_data(_dispvars->nextPage->resource, _dispvars->pixFormat,
		_dispvars->pitch, _dispvars->pixmem, &(_dispvars->bmpRect));
	
	// Issue a page flip at the next vblank interval (will be done at vsync anyway).
	_dispvars->update = vc_dispmanx_update_start(0);

	vc_dispmanx_element_change_source(_dispvars->update, _dispvars->element,
		_dispvars->nextPage->resource);
	vc_dispmanx_update_submit(_dispvars->update, &DispmanXVSyncCallback, _dispvars);
	
	pthread_mutex_lock(&_dispvars->pendingMutex);
	_dispvars->pageflipPending = true;	
	pthread_mutex_unlock(&_dispvars->pendingMutex);
}

void DispmanXSdlGraphicsManager::DispmanXFreeResources(void) {
	_dispvars->update = vc_dispmanx_update_start(0);
		
    	for (int i = 0; i < NUMPAGES; i++)
		vc_dispmanx_resource_delete(_dispvars->pages[i].resource);
	
	vc_dispmanx_element_remove(_dispvars->update, _dispvars->element);
	
	vc_dispmanx_update_submit_sync(_dispvars->update);		
}

void DispmanXSdlGraphicsManager::DispmanXVideoQuit() {
	DispmanXFreeResources();
	// Close display and deinit 
	vc_dispmanx_display_close(_dispvars->display);
	bcm_host_deinit();

	// Destroy mutexes and conditions	
	pthread_mutex_destroy(&_dispvars->pendingMutex);
	pthread_cond_destroy(&_dispvars->vsyncCondition);		

	free(_dispvars->pages);
}

bool DispmanXSdlGraphicsManager::loadGFXMode() {
	_forceFull = true;
	
	// In DispmanX, we manage aspect ratio correction, so for scummvm it's always disabled.
	_videoMode.aspectRatioCorrection = false;
	
	_videoMode.overlayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.overlayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;
	_videoMode.hardwareWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.hardwareHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

	//
	// Create the surface that contains the 8 bit game data
	//

	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight,
		_screenFormat.bytesPerPixel << 3,
		((1 << _screenFormat.rBits()) - 1) << _screenFormat.rShift ,
		((1 << _screenFormat.gBits()) - 1) << _screenFormat.gShift ,
		((1 << _screenFormat.bBits()) - 1) << _screenFormat.bShift ,
		((1 << _screenFormat.aBits()) - 1) << _screenFormat.aShift );
		if (_screen == NULL)
			error("allocating _screen failed");
	// Avoid having SDL_SRCALPHA set even if we supplied an alpha-channel in the format.
	SDL_SetAlpha(_screen, 0, 255);
	
	// SDL 1.2 palettes default to all black,
	// SDL 1.3 palettes default to all white,
	// Thus set our own default palette to all black.
	// SDL_SetColors does nothing for non indexed surfaces.
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	// We call DispmanXSetup() before SDL_SetVideoMode() because we use _hwscreen == null
	// to know inside DispmanXSetup() if we've been there before and need to free resources.
	DispmanXSetup(_videoMode.screenWidth, _videoMode.screenHeight, 16);	
	// _hwscreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 16, SDL_FULLSCREEN);
	_hwscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.hardwareWidth, _videoMode.hardwareHeight, 16,
		0, 0, 0, 0);
	
	_dispvars->pixmem = _hwscreen->pixels;

	// We draw on a RAM surface, but we make this call just to get SDL input initialized. 
	// Even if we don't use the returned SDL_Surface *, we still need to use the right dimensions
	// for mouse pointer adjustment to work correctly.
	SDL_SetVideoMode(_videoMode.screenWidth, _videoMode.screenHeight, 16, SDL_FULLSCREEN);

	detectSupportedFormats();

	if (_hwscreen == NULL) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwscreen is hosed.

		if (!_oldVideoMode.setup) {
			warning("SDL_SetVideoMode says we can't switch to that mode (%s)", SDL_GetError());
			g_system->quit();
		} else {
			return false;
		}
	}

	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Need some extra bytes around when using 2xSaI
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth + 3, _videoMode.screenHeight + 3,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("allocating _tmpscreen failed");

	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth, _videoMode.overlayHeight,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

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
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen2 == NULL)
		error("allocating _tmpscreen2 failed");

#ifdef USE_OSD
	_osdSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_hwscreen->w,
						_hwscreen->h,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);
	if (_osdSurface == NULL)
		error("allocating _osdSurface failed");
	SDL_SetColorKey(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kOSDColorKey);
#endif

	_eventSource->resetKeyboadEmulation(
		_videoMode.screenWidth * _videoMode.scaleFactor - 1,
		effectiveScreenHeight() - 1);

	// Distinguish 555 and 565 mode
	if (_hwscreen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);

	return true;
}

void DispmanXSdlGraphicsManager::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;
	ScalerProc *scalerProc;
	int scale1;

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos ||
		(_mouseNeedsRedraw && _mouseBackup.y <= _currentShakePos)) {
		SDL_Rect blackrect = {0, 0, (Uint16)(_videoMode.screenWidth * _videoMode.scaleFactor), (Uint16)(_newShakePos * _videoMode.scaleFactor)};

		SDL_FillRect(_hwscreen, &blackrect, 0);

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
		SDL_LockSurface(_hwscreen);

		srcPitch = srcSurf->pitch;
		dstPitch = _hwscreen->pitch;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			register int dst_y = r->y + _currentShakePos;
			register int dst_h = 0;
			register int rx1 = r->x * scale1;

			if (dst_y < height) {
				dst_h = r->h;
				if (dst_h > height - dst_y)
					dst_h = height - dst_y;

				dst_y = dst_y * scale1;

				assert(scalerProc != NULL);
				scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
					(byte *)_hwscreen->pixels + rx1 * 2 + dst_y * dstPitch, dstPitch, r->w, dst_h);
			}

			r->x = rx1;
			r->y = dst_y;
			r->w = r->w * scale1;
			r->h = dst_h * scale1;

		}
		SDL_UnlockSurface(srcSurf);
		SDL_UnlockSurface(_hwscreen);

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].h = effectiveScreenHeight();
		}

		drawMouse();

#ifdef USE_OSD
		if (_osdAlpha != SDL_ALPHA_TRANSPARENT) {
			SDL_BlitSurface(_osdSurface, 0, _hwscreen, 0);
		}
#endif
		// Finally, blit all our changes to the screen
		if (!_displayDisabled) {
			SDL_UpdateRects(_hwscreen, _numDirtyRects, _dirtyRectList);
			DispmanXUpdate();
		}
	}

	_numDirtyRects = 0;
	_forceFull = false;
	_mouseNeedsRedraw = false;
}

bool DispmanXSdlGraphicsManager::handleScalerHotkeys(Common::KeyCode key) {

	// Ctrl-Alt-a toggles aspect ratio correction
	if (key == 'a') {
		beginGFXTransaction();
			setFeatureState(OSystem::kFeatureAspectRatioCorrection, !_dispvars->aspectRatioCorrection);
		endGFXTransaction();
#ifdef USE_OSD
		char buffer[128];
		if (_dispvars->aspectRatioCorrection)
			sprintf(buffer, "%s", ("Enabled aspect ratio correction"));
		else
			sprintf(buffer, "%s", ("Disabled aspect ratio correction"));
		displayMessageOnOSD(buffer);
#endif
		internUpdateScreen();
		return true;
	}

	return true;
}

void DispmanXSdlGraphicsManager::setFullscreenMode(bool enable) {
	_videoMode.fullscreen = enable;
}

void DispmanXSdlGraphicsManager::setAspectRatioCorrection(bool enable) {
	Common::StackLock lock(_graphicsMutex);
	// Ratio correction is managed externally by dispmanx, so we disable it at the SDL level but take note,
	// so it's effectively taken into account at the dispmanx level in DispmanXSetup().
	if (_oldVideoMode.setup && _dispvars->aspectRatioCorrection == enable)
		return;
	
	if (_transactionMode == kTransactionActive) {
		_dispvars->aspectRatioCorrection = enable;
		_transactionDetails.needHotswap = false;
		DispmanXSetup(_videoMode.screenWidth, _videoMode.screenHeight, 16);	
	}
}

#endif
