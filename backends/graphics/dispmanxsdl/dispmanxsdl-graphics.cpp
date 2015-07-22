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

// Needed for Raspberry Pi header inclusion
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(RASPBERRYPI)

#include "backends/graphics/dispmanxsdl/dispmanxsdl-graphics.h"
#include "graphics/scaler/aspect.h"
#include "common/mutex.h"
#include "common/textconsole.h"

#include <bcm_host.h>

struct dispvarsStruct {
	DISPMANX_DISPLAY_HANDLE_T display;
	DISPMANX_UPDATE_HANDLE_T update;
	DISPMANX_ELEMENT_HANDLE_T element;
	VC_IMAGE_TYPE_T pixFormat;
	VC_DISPMANX_ALPHA_T alpha;

	VC_RECT_T bmpRect;
	VC_RECT_T srcRect;
	VC_RECT_T dstRect;
	uint32_t vcImagePtr;
	int screen;
	int pitch;
	unsigned int dispmanxWidth;
	unsigned int dispmanxHeight;
	bool aspectRatioCorrection;
	void *pixmem;

	int numpages;
	dispmanxPage *pages;
	dispmanxPage *currentPage;
	int pageflipPending;

	pthread_cond_t vsyncCondition;
	pthread_mutex_t vsyncCondMutex;
	pthread_mutex_t pendingMutex;

	SDL_Surface *fscreen;
};

struct dispmanxPage {
	DISPMANX_RESOURCE_HANDLE_T resource;
	bool used;
	// Each page has it's own mutex for
	// isolating the access to it's "used" flag.
	pthread_mutex_t pageUsedMutex;

	// This field will allow us to access the
	// main dispvars struct, for the vsync cb.
	struct dispvarsStruct *dispvars;
};

DispmanXSdlGraphicsManager::DispmanXSdlGraphicsManager(SdlEventSource *sdlEventSource)
 : SurfaceSdlGraphicsManager(sdlEventSource) {
	_dispvars = new(dispvarsStruct);
	dispmanXInit();
}

DispmanXSdlGraphicsManager::~DispmanXSdlGraphicsManager() {
	dispmanXVideoQuit();
	delete(_dispvars);
}

void DispmanXSdlGraphicsManager::dispmanXInit() {
	_dispvars->screen = 0;
	_dispvars->vcImagePtr = 0;
	_dispvars->numpages = 3;
	_dispvars->pages = (struct dispmanxPage *)calloc(_dispvars->numpages, sizeof(struct dispmanxPage));
	_dispvars->pageflipPending = 0;
	_dispvars->currentPage = NULL;
	_dispvars->pixFormat = VC_IMAGE_RGB565;

	/* Transparency disabled */
	_dispvars->alpha.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
	_dispvars->alpha.opacity = 255;
	_dispvars->alpha.mask = 0;
	_dispvars->element = 0;

	// Init each page's variables
	for (int i = 0; i < _dispvars->numpages; i++) {
		_dispvars->pages[i].used = false;
		_dispvars->pages[i].dispvars = _dispvars;
		_dispvars->pages[i].resource = 0;
		pthread_mutex_init(&_dispvars->pages[i].pageUsedMutex, NULL);
	}

	// Initialize the other mutex and condition variables
	pthread_cond_init(&_dispvars->vsyncCondition, NULL);
	pthread_mutex_init(&_dispvars->pendingMutex, NULL);
	pthread_mutex_init(&_dispvars->vsyncCondMutex, NULL);

	// Before we call any vc_* function, we need to call this one.
	bcm_host_init();

	_dispvars->display = vc_dispmanx_display_open(_dispvars->screen);
	graphics_get_display_size(_dispvars->display, &_dispvars->dispmanxWidth, &_dispvars->dispmanxHeight);

	// We need this so SDL_SetVideoMode() is called once.
	_dispvars->fscreen = NULL;
}

void DispmanXSdlGraphicsManager::dispmanXSetup(int srcWidth, int srcHeight) {
	unsigned int dstWidth, dstHeight, dstXpos, dstYpos;

	// If we have an element, we have to free it along with it's resources.
	if (_dispvars->element) {
		dispmanXFreeResources();
	}

	// We do this for 2 bytes per pixel which is default on the Rpi.
	_dispvars->pitch  = srcWidth * 2;
	if (_dispvars->aspectRatioCorrection) {
		float aspect = ((float)srcWidth / (float)srcHeight);
		dstWidth  = _dispvars->dispmanxHeight * aspect;
	} else {
		dstWidth  = _dispvars->dispmanxWidth;
	}
	dstHeight = _dispvars->dispmanxHeight;

	// If we obtain a scaled image width that is bigger than the physical screen width,
	// then we keep the physical screen width as our maximun width.
	if (dstWidth > _dispvars->dispmanxWidth) {
		dstWidth = _dispvars->dispmanxWidth;
	}

	dstXpos = (_dispvars->dispmanxWidth - dstWidth) / 2;
	dstYpos = (_dispvars->dispmanxHeight - dstHeight) / 2;

	// Remember we have to transfer the whole bitmap even if we would have
	// interest in a part of it! Blitting is done by the GPU.
	vc_dispmanx_rect_set(&_dispvars->dstRect, dstXpos, dstYpos, dstWidth, dstHeight);
	vc_dispmanx_rect_set(&_dispvars->bmpRect, 0, 0, srcWidth, srcHeight);
	vc_dispmanx_rect_set(&_dispvars->srcRect, 0, 0, srcWidth << 16, srcHeight << 16);

	for (int i = 0; i < _dispvars->numpages; i++) {
		_dispvars->pages[i].resource = vc_dispmanx_resource_create(_dispvars->pixFormat,
			srcWidth, srcHeight, &(_dispvars->vcImagePtr));
	}

	// Add the element. Has to be removed before getting here again.
	_dispvars->update = vc_dispmanx_update_start(0);

	_dispvars->element = vc_dispmanx_element_add(
		_dispvars->update,_dispvars->display, 0,
		&_dispvars->dstRect, 0,
		&_dispvars->srcRect, DISPMANX_PROTECTION_NONE,
		&_dispvars->alpha, 0, (DISPMANX_TRANSFORM_T)0);

	vc_dispmanx_update_submit_sync(_dispvars->update);
}

void dispmanXVSyncCallback (DISPMANX_UPDATE_HANDLE_T u, void *arg) {
	struct dispmanxPage *page = (struct dispmanxPage*)arg;
	struct dispvarsStruct *dispvars = page->dispvars;

	// Marking the page as free must be done before the signaling
	// so when the update function continues (it won't continue until we signal)
	// we can chose this page as free.
	if (dispvars->currentPage) {
		pthread_mutex_lock(&dispvars->currentPage->pageUsedMutex);

		// We mark as free the page that was visible until now.
		page->dispvars->currentPage->used = false;

		pthread_mutex_unlock(&dispvars->currentPage->pageUsedMutex);
	}

	// The page on which we issued the flip that
	// caused this callback becomes the visible one
	dispvars->currentPage = page;

	// These two things must be isolated "atomically" to avoid getting
	// a false positive in the pending_mutex test in update function.
	pthread_mutex_lock(&dispvars->pendingMutex);

	dispvars->pageflipPending--;
	pthread_cond_signal(&dispvars->vsyncCondition);

	pthread_mutex_unlock(&dispvars->pendingMutex);
}

void DispmanXSdlGraphicsManager::dispmanXUpdate() {
	// Wait until last issued flip completes to get a free page. Also,
	// dispmanx doesn't support issuing more than one pageflip.
	pthread_mutex_lock(&_dispvars->pendingMutex);

	if (_dispvars->pageflipPending > 0) {
		pthread_cond_wait(&_dispvars->vsyncCondition, &_dispvars->pendingMutex);
	}

	pthread_mutex_unlock(&_dispvars->pendingMutex);

	struct dispmanxPage *page = dispmanXGetFreePage();

	// Frame blitting
	vc_dispmanx_resource_write_data(page->resource, _dispvars->pixFormat,
		_dispvars->pitch, _dispvars->pixmem, &_dispvars->bmpRect);

	// Issue a page flip at the next vblank interval (will be done at vsync anyway).
	_dispvars->update = vc_dispmanx_update_start(0);

	vc_dispmanx_element_change_source(_dispvars->update, _dispvars->element,
		page->resource);
	vc_dispmanx_update_submit(_dispvars->update, &dispmanXVSyncCallback, page);

	pthread_mutex_lock(&_dispvars->pendingMutex);
	_dispvars->pageflipPending++;
	pthread_mutex_unlock(&_dispvars->pendingMutex);
}

struct dispmanxPage *DispmanXSdlGraphicsManager::dispmanXGetFreePage(void) {
	struct dispmanxPage *page = NULL;

	while (!page)
	{
		// Try to find a free page
		for (int i = 0; i < _dispvars->numpages; ++i) {
			if (!_dispvars->pages[i].used)
			{
				page = (_dispvars->pages) + i;
				break;
			}
		}

		// If no page is free at the moment,
		// wait until a free page is freed by vsync CB.
		if (!page) {
			pthread_mutex_lock(&_dispvars->vsyncCondMutex);
			pthread_cond_wait(&_dispvars->vsyncCondition, &_dispvars->vsyncCondMutex);
			pthread_mutex_unlock(&_dispvars->vsyncCondMutex);
		}
	}

	// We mark the choosen page as used
	pthread_mutex_lock(&page->pageUsedMutex);
	page->used = true;
	pthread_mutex_unlock(&page->pageUsedMutex);

	return page;
}

void DispmanXSdlGraphicsManager::dispmanXFreeResources(void) {
	// What if we run into the vsync cb code after freeing the resources?
	pthread_mutex_lock(&_dispvars->pendingMutex);
	if (_dispvars->pageflipPending > 0)
	{
	     pthread_cond_wait(&_dispvars->vsyncCondition, &_dispvars->pendingMutex);
	}
	pthread_mutex_unlock(&_dispvars->pendingMutex);

	for (int i = 0; i < _dispvars->numpages; i++) {
		vc_dispmanx_resource_delete(_dispvars->pages[i].resource);
		_dispvars->pages[i].resource = 0;
		_dispvars->pages[i].used = false;
	}

	_dispvars->update = vc_dispmanx_update_start(0);
	vc_dispmanx_element_remove(_dispvars->update, _dispvars->element);
	vc_dispmanx_update_submit_sync(_dispvars->update);
	// We use this on the setup function to know if we have to free resources and element.
	_dispvars->element = 0;
}

void DispmanXSdlGraphicsManager::dispmanXVideoQuit() {
	// This also waits for pending flips to complete, that's needed before
	// we destroy the mutexes and condition.
	dispmanXFreeResources();

	// Destroy the mutexes and conditions
	for (int i = 0; i < _dispvars->numpages; i++) {
		pthread_mutex_destroy(&_dispvars->pages[i].pageUsedMutex);
	}
	pthread_mutex_destroy(&_dispvars->pendingMutex);
	pthread_mutex_destroy(&_dispvars->vsyncCondMutex);
	pthread_cond_destroy(&_dispvars->vsyncCondition);

	free(_dispvars->pages);

	// Close display and deinit
	vc_dispmanx_display_close(_dispvars->display);
	bcm_host_deinit();
}

bool DispmanXSdlGraphicsManager::loadGFXMode() {
	_forceFull = true;

	// In dispmanX, we manage aspect ratio correction, so for scummvm it's always disabled.
	_videoMode.aspectRatioCorrection = false;

	_videoMode.overlayWidth = _videoMode.screenWidth;
	_videoMode.overlayHeight = _videoMode.screenHeight;
	_videoMode.hardwareWidth = _videoMode.screenWidth;
	_videoMode.hardwareHeight = _videoMode.screenHeight;

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

	// We set our own default palette to all black.
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	dispmanXSetup(_videoMode.screenWidth, _videoMode.screenHeight);

	_hwscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.hardwareWidth, _videoMode.hardwareHeight, 16,
		0, 0, 0, 0);

	// This is just so SDL 1.x input is initialized. Only once!
	if (_dispvars->fscreen == NULL)
		_dispvars->fscreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 16, SDL_FULLSCREEN);
	if (_hwscreen == NULL) {
		// Don't use error here because we don't have access to the debug console
		warning("Allocating surface for dispmanX rendering _hwscreen failed");
		g_system->quit();
	}

	// We render to dispmanx resources from _hwscreen pixels array
	_dispvars->pixmem = _hwscreen->pixels;

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
		_videoMode.screenWidth, effectiveScreenHeight());

	return true;
}

void DispmanXSdlGraphicsManager::clearOverlay() {
	//assert(_transactionMode == kTransactionNone);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	if (!_overlayVisible)
		return;

	// Clear the overlay by making the game screen "look through" everywhere.
	SDL_Rect src, dst;
	src.x = src.y = 0;
	dst.x = dst.y = 0;
	src.w = dst.w = _videoMode.screenWidth;
	src.h = dst.h = _videoMode.screenHeight;
	if (SDL_BlitSurface(_screen, &src, _hwscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	SDL_LockSurface(_hwscreen);
	SDL_LockSurface(_overlayscreen);
	Normal1x((byte *)(_hwscreen->pixels), _hwscreen->pitch,
	(byte *)_overlayscreen->pixels, _overlayscreen->pitch, _videoMode.screenWidth, _videoMode.screenHeight);

	SDL_UnlockSurface(_hwscreen);
	SDL_UnlockSurface(_overlayscreen);

	_forceFull = true;
}

void DispmanXSdlGraphicsManager::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos ||
		(_mouseNeedsRedraw && _mouseBackup.y <= _currentShakePos)) {
		SDL_Rect blackrect = {0, 0, (Uint16)(_videoMode.screenWidth * _videoMode.scaleFactor), (Uint16)(_newShakePos * _videoMode.scaleFactor)};

		if (_dispvars->aspectRatioCorrection && !_overlayVisible)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

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
		srcSurf = _hwscreen;
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _hwscreen;
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
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
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			dst = *r;

			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

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
			dispmanXUpdate();
		}
	}

	_numDirtyRects = 0;
	_forceFull = false;
	_mouseNeedsRedraw = false;
}

bool DispmanXSdlGraphicsManager::handleScalerHotkeys(Common::KeyCode key) {

	// Ctrl-Alt-a toggles aspect ratio correction
	if (key == 'a') {
		setFeatureState(OSystem::kFeatureAspectRatioCorrection, !_dispvars->aspectRatioCorrection);
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
	// We simply take note on what's the aspect ratio correction activation state.
	_dispvars->aspectRatioCorrection = enable;

	// If we have a videomode setup already, call dispmanXSetup() again so aspect ratio
	// correction activation/deactivation works from the menu.
	if (_oldVideoMode.setup && _dispvars->aspectRatioCorrection == enable) {
		dispmanXSetup(_videoMode.screenWidth, _videoMode.screenHeight);
	}
}

#endif
