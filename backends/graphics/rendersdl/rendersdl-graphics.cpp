/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"

#if defined(SDL_BACKEND)
#include "backends/graphics/rendersdl/rendersdl-graphics.h"
#include "backends/events/sdl/sdl-events.h"
#include "common/config-manager.h"
#include "common/mutex.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/util.h"
#include "common/file.h"
#include "common/frac.h"
#ifdef USE_RGB_COLOR
#include "common/list.h"
#endif
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/scaler.h"
#include "graphics/surface.h"
#include "gui/debugger.h"
#include "gui/EventRecorder.h"
#ifdef USE_PNG
#include "image/png.h"
#else
#include "image/bmp.h"
#endif
#include "common/text-to-speech.h"

#ifdef USE_OSD
#if defined(MACOSX)
#include "backends/platform/sdl/macosx/macosx-touchbar.h"
#endif
#endif

static void destroySurface(SDL_Surface *surface) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_DestroySurface(surface);
#else
	SDL_FreeSurface(surface);
#endif
}

#if SDL_VERSION_ATLEAST(3, 0, 0)
static bool blitSurface(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect) {
	return SDL_BlitSurface(src, srcrect, dst, dstrect);
}
#elif SDL_VERSION_ATLEAST(2, 0, 0)
static bool blitSurface(SDL_Surface * src, const SDL_Rect * srcrect, SDL_Surface * dst, SDL_Rect * dstrect) {
	return SDL_BlitSurface(src, srcrect, dst, dstrect) != -1;
}
#else
static bool blitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	return SDL_BlitSurface(src, srcrect, dst, dstrect) != -1;
}
#endif


static bool lockSurface(SDL_Surface *surface) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	return SDL_LockSurface(surface);
#else
	return SDL_LockSurface(surface) != -1;
#endif
}

static SDL_Surface *createSurface(int width, int height, SDL_Surface *surface) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	const SDL_PixelFormatDetails *pixelFormatDetails = SDL_GetPixelFormatDetails(surface->format);
	if (pixelFormatDetails == nullptr)
		error("getting pixel format details failed");
	return SDL_CreateSurface(width, height,
					SDL_GetPixelFormatForMasks(
						pixelFormatDetails->bits_per_pixel,
						pixelFormatDetails->Rmask,
						pixelFormatDetails->Gmask,
						pixelFormatDetails->Bmask,
						pixelFormatDetails->Amask));
#else
	return SDL_CreateRGBSurface(SDL_SWSURFACE,
					width,
					height,
					surface->format->BitsPerPixel,
					surface->format->Rmask,
					surface->format->Gmask,
					surface->format->Bmask,
					surface->format->Amask);
#endif
}

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"rendersdl", _s("SDL Renderer"), GFX_RENDERSDL},
	{nullptr, nullptr, 0}
};

const OSystem::GraphicsMode s_supportedStretchModes[] = {
	{"center", _s("Center"), STRETCH_CENTER},
	{"pixel-perfect", _s("Pixel-perfect scaling"), STRETCH_INTEGRAL},
	{"fit", _s("Fit to window"), STRETCH_FIT},
	{"stretch", _s("Stretch to window"), STRETCH_STRETCH},
	{"fit_force_aspect", _s("Fit to window (4:3)"), STRETCH_FIT_FORCE_ASPECT},
	{nullptr, nullptr, 0}
};

RenderSdlGraphicsManager::RenderSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window)
	:
	SdlGraphicsManager(sdlEventSource, window),
#ifdef USE_OSD
	_osdMessageTexture(nullptr), _osdMessageAlpha(SDL_ALPHA_TRANSPARENT), _osdMessageFadeStartTime(0),
	_osdIconTexture(nullptr),
#endif
	_renderer(nullptr), _screenTexture(nullptr),
	_screen(nullptr), _tmpscreen(nullptr),
	_screenFormat(Graphics::PixelFormat::createFormatCLUT8()),
	_cursorFormat(Graphics::PixelFormat::createFormatCLUT8()),
	_useOldSrc(false),
	_overlayTexture(nullptr), _overlaySurface(nullptr),
	_screenChangeCount(0),
	_mouseTexture(nullptr), _mouseScaler(nullptr), _mouseSurface(nullptr),
	_mouseOrigSurface(nullptr), _cursorDontScale(false), _cursorPaletteDisabled(true),
	_currentShakeXOffset(0), _currentShakeYOffset(0),
	_paletteDirtyStart(0), _paletteDirtyEnd(0),
	_screenIsLocked(false),
	_displayDisabled(false),
	_transactionMode(kTransactionNone),
	_scalerPlugins(ScalerMan.getPlugins()), _scalerPlugin(nullptr), _scaler(nullptr),
	_mouseKeyColor(0), _disableMouseKeyColor(false) {

	// allocate palette storage
	_currentPalette = (SDL_Color *)calloc(256, sizeof(SDL_Color));
	_cursorPalette = (SDL_Color *)calloc(256, sizeof(SDL_Color));

	_videoMode.aspectRatioCorrection = ConfMan.getBool("aspect_ratio");

	_scaler = nullptr;
	_maxExtraPixels = ScalerMan.getMaxExtraPixels();

	_videoMode.fullscreen = ConfMan.getBool("fullscreen");
	_videoMode.filtering = ConfMan.getBool("filtering");
	_videoMode.stretchMode = STRETCH_FIT;
	_videoMode.vsync = ConfMan.getBool("vsync");

	_videoMode.scalerIndex = getDefaultScaler();
	_videoMode.scaleFactor = getDefaultScaleFactor();
}

RenderSdlGraphicsManager::~RenderSdlGraphicsManager() {
	unloadGFXMode();
	delete _scaler;
	delete _mouseScaler;
	if (_mouseOrigSurface) {
		destroySurface(_mouseOrigSurface);
		if (_mouseOrigSurface == _mouseSurface) {
			_mouseSurface = nullptr;
		}
	}
	if (_mouseSurface) {
		destroySurface(_mouseSurface);
	}
	free(_currentPalette);
	free(_cursorPalette);
}

bool RenderSdlGraphicsManager::hasFeature(OSystem::Feature f) const {
	if (f == OSystem::kFeatureOverlaySupportsAlpha)
		return _overlayFormat.aBits() > 3;

	return
		(f == OSystem::kFeatureFullscreenMode) ||
#ifdef USE_SCALERS
		(f == OSystem::kFeatureScalers) ||
#endif
		(f == OSystem::kFeatureAspectRatioCorrection) ||
		(f == OSystem::kFeatureFilteringMode) ||
		(f == OSystem::kFeatureStretchMode) ||
		(f == OSystem::kFeatureRotationMode) ||
		(f == OSystem::kFeatureVSync) ||
		(f == OSystem::kFeatureCursorPalette) ||
		(f == OSystem::kFeatureCursorAlpha) ||
		(f == OSystem::kFeatureIconifyWindow) ||
		(f == OSystem::kFeatureCursorMask);
}

void RenderSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		setFullscreenMode(enable);
		break;
	case OSystem::kFeatureAspectRatioCorrection:
		setAspectRatioCorrection(enable);
		break;
	case OSystem::kFeatureFilteringMode:
		setFilteringMode(enable);
		break;
	case OSystem::kFeatureVSync:
		setVSync(enable);
		break;
	case OSystem::kFeatureCursorPalette:
		_cursorPaletteDisabled = !enable;
		blitCursor();
		break;
	case OSystem::kFeatureIconifyWindow:
		if (enable)
			_window->iconifyWindow();
		break;
	default:
		break;
	}
}

bool RenderSdlGraphicsManager::getFeatureState(OSystem::Feature f) const {
	// We need to allow this to be called from within a transaction, since we
	// currently use it to retrieve the graphics state, when switching from
	// SDL->OpenGL mode for example.
	//assert(_transactionMode == kTransactionNone);

	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return _videoMode.fullscreen;
	case OSystem::kFeatureAspectRatioCorrection:
		return _videoMode.aspectRatioCorrection;
	case OSystem::kFeatureVSync:
		return _videoMode.vsync;
	case OSystem::kFeatureFilteringMode:
		return _videoMode.filtering;
	case OSystem::kFeatureCursorPalette:
		return !_cursorPaletteDisabled;
	default:
		return false;
	}
}

const OSystem::GraphicsMode *RenderSdlGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int RenderSdlGraphicsManager::getDefaultGraphicsMode() const {
	return GFX_RENDERSDL;
}

bool RenderSdlGraphicsManager::setGraphicsMode(int mode, uint flags) {
	Common::StackLock lock(_graphicsMutex);

	assert(_transactionMode == kTransactionActive);

	if (_oldVideoMode.setup && _oldVideoMode.mode == mode)
		return true;

	// Check this is a valid mode
	const OSystem::GraphicsMode *sm = getSupportedGraphicsModes();
	bool found = false;
	while (sm->name) {
		if (sm->id == mode) {
			found = true;
			break;
		}
		sm++;
	}
	if (!found) {
		warning("unknown mode %d", mode);
		return false;
	}

	_transactionDetails.needUpdatescreen = true;

	_videoMode.mode = mode;

	return true;
}

int RenderSdlGraphicsManager::getGraphicsMode() const {
	return _videoMode.mode;
}

void RenderSdlGraphicsManager::beginGFXTransaction() {
	assert(_transactionMode == kTransactionNone);

	_transactionMode = kTransactionActive;

	_transactionDetails.sizeChanged = false;

	_transactionDetails.needHotswap = false;
	_transactionDetails.needUpdatescreen = false;

	_transactionDetails.needDisplayResize = false;
	_transactionDetails.needTextureUpdate = false;
#ifdef USE_RGB_COLOR
	_transactionDetails.formatChanged = false;
#endif

	_oldVideoMode = _videoMode;
}

OSystem::TransactionError RenderSdlGraphicsManager::endGFXTransaction() {
	int errors = OSystem::kTransactionSuccess;

	assert(_transactionMode != kTransactionNone);

	if (_transactionMode == kTransactionRollback) {
		if (_videoMode.mode != _oldVideoMode.mode) {
			errors |= OSystem::kTransactionModeSwitchFailed;

			_videoMode.mode = _oldVideoMode.mode;
		}

		if (_videoMode.fullscreen != _oldVideoMode.fullscreen) {
			errors |= OSystem::kTransactionFullscreenFailed;

			_videoMode.fullscreen = _oldVideoMode.fullscreen;
		}

		if (_videoMode.aspectRatioCorrection != _oldVideoMode.aspectRatioCorrection) {
			errors |= OSystem::kTransactionAspectRatioFailed;

			_videoMode.aspectRatioCorrection = _oldVideoMode.aspectRatioCorrection;
		}

		if (_videoMode.scalerIndex != _oldVideoMode.scalerIndex) {
			errors |= OSystem::kTransactionModeSwitchFailed;

			_videoMode.scalerIndex = _oldVideoMode.scalerIndex;
			_videoMode.scaleFactor = _oldVideoMode.scaleFactor;
		}

		if (_videoMode.stretchMode != _oldVideoMode.stretchMode) {
			errors |= OSystem::kTransactionStretchModeSwitchFailed;

			_videoMode.stretchMode = _oldVideoMode.stretchMode;
		}

		if (_videoMode.vsync != _oldVideoMode.vsync) {
			errors |= OSystem::kTransactionVSyncFailed;

			_videoMode.vsync = _oldVideoMode.vsync;
		}

		if (_videoMode.filtering != _oldVideoMode.filtering) {
			errors |= OSystem::kTransactionFilteringFailed;

			_videoMode.filtering = _oldVideoMode.filtering;
		}
#ifdef USE_RGB_COLOR
		if (_videoMode.format != _oldVideoMode.format) {
			errors |= OSystem::kTransactionFormatNotSupported;

			_videoMode.format = _oldVideoMode.format;
			_screenFormat = _videoMode.format;
		}
#endif

		if (_videoMode.screenWidth != _oldVideoMode.screenWidth || _videoMode.screenHeight != _oldVideoMode.screenHeight) {
			errors |= OSystem::kTransactionSizeChangeFailed;

			_videoMode.screenWidth = _oldVideoMode.screenWidth;
			_videoMode.screenHeight = _oldVideoMode.screenHeight;
		}

		// Our new video mode would now be exactly the same as the
		// old one. Since we still can not assume SDL_SetVideoMode
		// to be working fine, we need to invalidate the old video
		// mode, so loadGFXMode would error out properly.
		_oldVideoMode.setup = false;
	}

#ifdef USE_RGB_COLOR
	if (_transactionDetails.sizeChanged || _transactionDetails.formatChanged) {
#else
	if (_transactionDetails.sizeChanged) {
#endif
		unloadGFXMode();
		if (!loadGFXMode()) {
			if (_oldVideoMode.setup) {
				_transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			setGraphicsModeIntern();
			clearOverlay();

			_videoMode.setup = true;
			// OSystem_SDL::pollEvent used to update the screen change count,
			// but actually it gives problems when a video mode was changed
			// but OSystem_SDL::pollEvent was not called. This for example
			// caused a crash under certain circumstances when returning to launcher.
			// To fix this issue we update the screen change count right here.
			_screenChangeCount++;
		}
	} else if (_transactionDetails.needHotswap) {
		setGraphicsModeIntern();
		if (!hotswapGFXMode()) {
			if (_oldVideoMode.setup) {
				_transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			}
		} else {
			_videoMode.setup = true;
			// OSystem_SDL::pollEvent used to update the screen change count,
			// but actually it gives problems when a video mode was changed
			// but OSystem_SDL::pollEvent was not called. This for example
			// caused a crash under certain circumstances when returning to launcher.
			// To fix this issue we update the screen change count right here.
			_screenChangeCount++;

			if (_transactionDetails.needDisplayResize) {
				recalculateDisplayAreas();
				recalculateCursorScaling();
			}

			if (_transactionDetails.needUpdatescreen)
				internUpdateScreen();
		}
	} else if (_transactionDetails.needTextureUpdate) {
		setGraphicsModeIntern();
		recreateScreenTexture();
		if (_transactionDetails.needDisplayResize) {
			recalculateDisplayAreas();
			recalculateCursorScaling();
		}
		internUpdateScreen();
	} else if (_transactionDetails.needUpdatescreen) {
		setGraphicsModeIntern();
		if (_transactionDetails.needDisplayResize) {
			recalculateDisplayAreas();
			recalculateCursorScaling();
		}
		internUpdateScreen();
	}

	_transactionMode = kTransactionNone;
	return (OSystem::TransactionError)errors;
}

#if SDL_VERSION_ATLEAST(3, 0, 0)
Graphics::PixelFormat RenderSdlGraphicsManager::convertSDLPixelFormat(SDL_PixelFormat format) const {
	const SDL_PixelFormatDetails *in = SDL_GetPixelFormatDetails(format);
	assert(in);
	if (in->bytes_per_pixel == 1 && (
		    (in->Rmask == 0xff && in->Gmask == 0xff && in->Bmask == 0xff) ||
		    (in->Rmask == 0 && in->Gmask == 0 && in->Bmask == 0)
		    ))
		return Graphics::PixelFormat::createFormatCLUT8();
	Graphics::PixelFormat out(in->bytes_per_pixel,
		in->Rbits, in->Gbits,
		in->Bbits, in->Abits,
		in->Rshift, in->Gshift,
		in->Bshift, in->Ashift);

	// Workaround to SDL not providing an accurate Aloss value on some platforms.
	if (in->Amask == 0)
		out.aLoss = 8;

	return out;
}
#else
Graphics::PixelFormat RenderSdlGraphicsManager::convertSDLPixelFormat(SDL_PixelFormat *in) const {
	if (in->BytesPerPixel == 1 && (
		    (in->Rmask == 0xff && in->Gmask == 0xff && in->Bmask == 0xff) ||
		    (in->Rmask == 0 && in->Gmask == 0 && in->Bmask == 0)
		    ))
		return Graphics::PixelFormat::createFormatCLUT8();
	Graphics::PixelFormat out(in->BytesPerPixel,
		8 - in->Rloss, 8 - in->Gloss,
		8 - in->Bloss, 8 - in->Aloss,
		in->Rshift, in->Gshift,
		in->Bshift, in->Ashift);

	// Workaround to SDL not providing an accurate Aloss value on some platforms.
	if (in->Amask == 0)
		out.aLoss = 8;

	return out;
}
#endif

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> RenderSdlGraphicsManager::getSupportedFormats() const {
	assert(!_supportedFormats.empty());
	return _supportedFormats;
}

void RenderSdlGraphicsManager::detectSupportedFormats() {
	_supportedFormats.clear();

	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();

	if (_hwScreen) {
		// Get our currently set hardware format
		Graphics::PixelFormat hwFormat = convertSDLPixelFormat(_hwScreen->format);

		// This is the first supported format to prevent pixel format conversion
		// on blitting. This gives us a lot more performance on low perf hardware.
		_supportedFormats.push_back(hwFormat);

		format = hwFormat;
	}

	// Some tables with standard formats that we always list
	// as "supported". If frontend code tries to use one of
	// these, we will perform the necessary format
	// conversion in the background. Of course this incurs a
	// performance hit, but on desktop ports this should not
	// matter. We still push the currently active format to
	// the front, so if frontend code just uses the first
	// available format, it will get one that is "cheap" to
	// use.
	const Graphics::PixelFormat RGBList[] = {
		// RGBA8888, ARGB8888, RGB888
		Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0),
		Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24),
		Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0),
		// RGB565, XRGB1555, RGB555, RGBA4444, ARGB4444
		Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),
		Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15),
		Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0),
		Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0),
		Graphics::PixelFormat(2, 4, 4, 4, 4, 8, 4, 0, 12)
	};
	const Graphics::PixelFormat BGRList[] = {
		// ABGR8888, BGRA8888, BGR888
		Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24),
		Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0),
		Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0),
		// BGR565, XBGR1555, BGR555, ABGR4444, BGRA4444
		Graphics::PixelFormat(2, 5, 6, 5, 0, 0, 5, 11, 0),
		Graphics::PixelFormat(2, 5, 5, 5, 1, 0, 5, 10, 15),
		Graphics::PixelFormat(2, 5, 5, 5, 0, 0, 5, 10, 0),
		Graphics::PixelFormat(2, 4, 4, 4, 4, 0, 4, 8, 12),
		Graphics::PixelFormat(2, 4, 4, 4, 4, 4, 8, 12, 0)
	};

	// TODO: prioritize matching alpha masks
	int i;

	// Push some RGB formats
	for (i = 0; i < ARRAYSIZE(RGBList); i++) {
		if (_hwScreen && (RGBList[i].bytesPerPixel > format.bytesPerPixel))
			continue;
		if (RGBList[i] != format)
			_supportedFormats.push_back(RGBList[i]);
	}

	// Push some BGR formats
	for (i = 0; i < ARRAYSIZE(BGRList); i++) {
		if (_hwScreen && (BGRList[i].bytesPerPixel > format.bytesPerPixel))
			continue;
		if (BGRList[i] != format)
			_supportedFormats.push_back(BGRList[i]);
	}

	// Finally, we always supposed 8 bit palette graphics
	_supportedFormats.push_back(Graphics::PixelFormat::createFormatCLUT8());
}
#endif

uint RenderSdlGraphicsManager::getDefaultScaler() const {
	return ScalerMan.findScalerPluginIndex("normal");
}

uint RenderSdlGraphicsManager::getDefaultScaleFactor() const {
#if defined (USE_SCALERS)
	return 2;
#else
	return 1;
#endif
}

bool RenderSdlGraphicsManager::setScaler(uint mode, int factor) {
	Common::StackLock lock(_graphicsMutex);

	assert(_transactionMode == kTransactionActive);

	if (_oldVideoMode.setup && _oldVideoMode.scalerIndex == mode && _oldVideoMode.scaleFactor == factor)
		return true;

	int newFactor;
	if (factor == -1)
		newFactor = getDefaultScaleFactor();
	else if (_scalerPlugins[mode]->get<ScalerPluginObject>().hasFactor(factor))
		newFactor = factor;
	else if (_scalerPlugins[mode]->get<ScalerPluginObject>().hasFactor(_oldVideoMode.scaleFactor))
		newFactor = _oldVideoMode.scaleFactor;
	else
		newFactor = _scalerPlugins[mode]->get<ScalerPluginObject>().getDefaultFactor();

	if (_oldVideoMode.setup && _oldVideoMode.scaleFactor != newFactor)
		_transactionDetails.needHotswap = true;

	_transactionDetails.needUpdatescreen = true;

	_videoMode.scalerIndex = mode;
	_videoMode.scaleFactor = newFactor;

	return true;
}

void RenderSdlGraphicsManager::setGraphicsModeIntern() {
	Common::StackLock lock(_graphicsMutex);

	if (!_screen || !_hwScreen)
		return;


	// If the scalerIndex has changed, change scaler plugins
	if (&_scalerPlugins[_videoMode.scalerIndex]->get<ScalerPluginObject>() != _scalerPlugin
#ifdef USE_RGB_COLOR
		|| _transactionDetails.formatChanged
#endif
		) {
		Graphics::PixelFormat format = convertSDLPixelFormat(_hwScreen->format);
		delete _scaler;

		_scalerPlugin = &_scalerPlugins[_videoMode.scalerIndex]->get<ScalerPluginObject>();
		_scaler = _scalerPlugin->createInstance(format);

		if (_mouseScaler != nullptr) {
			delete _mouseScaler;
			_mouseScaler = _scalerPlugin->createInstance(_cursorFormat);
		}
	}

	_scaler->setFactor(_videoMode.scaleFactor);
	_extraPixels = _scalerPlugin->extraPixels();
	_useOldSrc = _scalerPlugin->useOldSource();
	if (_useOldSrc) {
		_scaler->enableSource(true);
		_scaler->setSource((byte *)_tmpscreen->pixels, _tmpscreen->pitch,
									_videoMode.screenWidth, _videoMode.screenHeight, _maxExtraPixels);
	}

	// Blit everything to the screen
	_forceRedraw = true;

	// Even if the old and new scale factors are the same, we may have a
	// different scaler for the cursor now.
	blitCursor();
}

uint RenderSdlGraphicsManager::getScaler() const {
	assert(_transactionMode == kTransactionNone);
	return _videoMode.scalerIndex;
}

uint RenderSdlGraphicsManager::getScaleFactor() const {
	assert(_transactionMode == kTransactionNone);
	return _videoMode.scaleFactor;
}

const OSystem::GraphicsMode *RenderSdlGraphicsManager::getSupportedStretchModes() const {
	return s_supportedStretchModes;
}

int RenderSdlGraphicsManager::getDefaultStretchMode() const {
	return STRETCH_FIT;
}

bool RenderSdlGraphicsManager::setStretchMode(int mode) {
	Common::StackLock lock(_graphicsMutex);

	assert(_transactionMode == kTransactionActive);

	if (_oldVideoMode.setup && _oldVideoMode.stretchMode == mode)
		return true;

	// Check this is a valid mode
	const OSystem::GraphicsMode *sm = s_supportedStretchModes;
	bool found = false;
	while (sm->name) {
		if (sm->id == mode) {
			found = true;
			break;
		}
		sm++;
	}
	if (!found) {
		warning("unknown stretch mode %d", mode);
		return false;
	}

	_transactionDetails.needUpdatescreen = true;
	_transactionDetails.needDisplayResize = true;

	_videoMode.stretchMode = mode;

	return true;
}

int RenderSdlGraphicsManager::getStretchMode() const {
	return _videoMode.stretchMode;
}

void RenderSdlGraphicsManager::getDefaultResolution(uint &w, uint &h) {
	w = 320;
	h = 200;
}

void RenderSdlGraphicsManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	assert(_transactionMode == kTransactionActive);

	_gameScreenShakeXOffset = 0;
	_gameScreenShakeYOffset = 0;

	if (w == 0) {
		getDefaultResolution(w, h);
	}

#ifdef USE_RGB_COLOR
	//avoid redundant format changes
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

	if ((int)w != _videoMode.screenWidth || (int)h != _videoMode.screenHeight) {
		const bool useDefault = defaultGraphicsModeConfig();
		int scaleFactor = ConfMan.getInt("scale_factor");
		if (scaleFactor == -1)
			scaleFactor = getDefaultScaleFactor();
		int mode = _videoMode.scalerIndex;
		if (useDefault && w > 320) {
			// The default scaler is assumed to be for low
			// resolution games. For high resolution games, pick
			// the largest scale factor that gives at most the
			// same window width.
			scaleFactor = MAX<uint>(1, (scaleFactor * 320) / w);

			// Check that the current scaler is available at the
			// new scale factor. If not, the normal scaler is
			// assumed to be available at any reasonable factor,
			// and is - of course -the only one that has a 1x mode.
			const Common::Array<uint> &factors = _scalerPlugins[mode]->get<ScalerPluginObject>().getFactors();
			if (Common::find(factors.begin(), factors.end(), (uint)scaleFactor) == factors.end()) {
				mode = ScalerMan.findScalerPluginIndex("normal");
			}
		}
		setScaler(mode, scaleFactor);
	}

	_videoMode.screenWidth = w;
	_videoMode.screenHeight = h;

	_transactionDetails.sizeChanged = true;
}

bool RenderSdlGraphicsManager::gameNeedsAspectRatioCorrection() const {
	if (_videoMode.aspectRatioCorrection) {
		const uint width = getWidth();
		const uint height = getHeight();

		// In case we enable aspect ratio correction we force a 4/3 ratio.
		// But just for 320x200 and 640x400 games, since other games do not need
		// this.
		return (width == 320 && height == 200) || (width == 640 && height == 400);
	}

	return false;
}

void RenderSdlGraphicsManager::setupHardwareSize() {
	_videoMode.hardwareWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.hardwareHeight = _videoMode.screenHeight * _videoMode.scaleFactor;
}

bool RenderSdlGraphicsManager::loadGFXMode() {
	_forceRedraw = true;

	setupHardwareSize();

	//
	// Create the surface that contains the game data
	//

	const Graphics::PixelFormat &format = _screenFormat;
	const Uint32 rMask = ((0xFF >> format.rLoss) << format.rShift);
	const Uint32 gMask = ((0xFF >> format.gLoss) << format.gShift);
	const Uint32 bMask = ((0xFF >> format.bLoss) << format.bShift);
	const Uint32 aMask = ((0xFF >> format.aLoss) << format.aShift);
#if SDL_VERSION_ATLEAST(3, 0, 0)
	_screen = SDL_CreateSurface(_videoMode.screenWidth, _videoMode.screenHeight,
						SDL_GetPixelFormatForMasks(_screenFormat.bytesPerPixel * 8, rMask, gMask, bMask, aMask));
#else
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight,
						_screenFormat.bytesPerPixel * 8, rMask, gMask, bMask, aMask);
#endif
	if (_screen == nullptr)
		error("allocating _screen failed");

#ifdef USE_RGB_COLOR
	// Avoid having the blend mode set even if we supplied an alpha-channel in the format.
	SDL_SetSurfaceBlendMode(_screen, SDL_BLENDMODE_NONE);
#endif

	// SDL 1.2 palettes default to all black,
	// SDL 1.3 palettes default to all white,
	// Thus set our own default palette to all black.
	if (_screen->format->palette)
		SDL_SetPaletteColors(_screen->format->palette, _currentPalette, 0, 256);

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

#ifdef ENABLE_EVENTRECORDER
	_displayDisabled = ConfMan.getBool("disable_display");

	if (_displayDisabled) {
		_hwScreen = g_eventRec.getSurface(_videoMode.hardwareWidth, _videoMode.hardwareHeight);
	} else
#endif
	{

		uint32 createWindowFlags = SDL_WINDOW_RESIZABLE;
		uint32 rendererFlags = 0;

		if (_videoMode.fullscreen)
			createWindowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		if (!createOrUpdateWindow(_videoMode.hardwareWidth, _videoMode.hardwareHeight, createWindowFlags)) {
			error("Failed to create window: %s", SDL_GetError());
		}

#if defined(MACOSX) && SDL_VERSION_ATLEAST(2, 0, 10)
		// WORKAROUND: Bug #11430: "macOS: blurry content on Retina displays"
		// Since SDL 2.0.10, Metal takes priority over OpenGL rendering on macOS,
		// but this causes blurriness issues on Retina displays. Just switch
		// back to OpenGL for now.
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif

		if (_videoMode.vsync) {
			rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
		}

		_renderer = SDL_CreateRenderer(_window->getSDLWindow(), -1, rendererFlags);
		if (!_renderer) {
			if (_videoMode.vsync) {
				// VSYNC might not be available, so retry without VSYNC
				warning("SDL_SetVideoMode: SDL_CreateRenderer() failed with VSYNC option, retrying without it...");
				_videoMode.vsync = false;
				rendererFlags &= ~SDL_RENDERER_PRESENTVSYNC;
				_renderer = SDL_CreateRenderer(_window->getSDLWindow(), -1, rendererFlags);
			}
			if (!_renderer) {
				error("Failed to create renderer: %s", SDL_GetError());
			}
		}

		getWindowSizeFromSdl(&_windowWidth, &_windowHeight);
		handleResize(_windowWidth, _windowHeight);

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, _videoMode.filtering ? "linear" : "nearest");

		Uint32 screenFormat = SDL_PIXELFORMAT_RGB565;

		_screenTexture = SDL_CreateTexture(_renderer, screenFormat, SDL_TEXTUREACCESS_STREAMING, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
		if (!_screenTexture) {
			error("Failed to create texture: %s", SDL_GetError());
		}

		_hwScreen = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, _videoMode.hardwareWidth, _videoMode.hardwareHeight, SDL_BITSPERPIXEL(screenFormat), screenFormat);
	}


#ifdef USE_RGB_COLOR
	detectSupportedFormats();
#endif

	if (_hwScreen == nullptr) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwScreen is hosed.

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
	_tmpscreen = createSurface(_videoMode.screenWidth + _maxExtraPixels * 2, _videoMode.screenHeight + _maxExtraPixels * 2, _hwScreen);
	if (_tmpscreen == nullptr)
		error("allocating _tmpscreen failed");

	if (_useOldSrc) {
		// Create surface containing previous frame's data to pass to scaler
		_scaler->setSource((byte *)_tmpscreen->pixels, _tmpscreen->pitch,
									_videoMode.screenWidth, _videoMode.screenHeight, _maxExtraPixels);
	}

#if defined(USE_IMGUI) && defined(USE_IMGUI_SDLRENDERER2)
	// Setup Dear ImGui
	initImGui(_renderer, nullptr);
#endif

	return true;
}

void RenderSdlGraphicsManager::unloadGFXMode() {
#if defined(USE_IMGUI) && defined(USE_IMGUI_SDLRENDERER2)
	destroyImGui();
#endif

	if (_screen) {
		destroySurface(_screen);
		_screen = nullptr;
	}

#ifdef USE_OSD
	if (_osdMessageTexture) {
		SDL_DestroyTexture(_osdMessageTexture);
		_osdMessageTexture = nullptr;
	}

	if (_osdIconTexture) {
		SDL_DestroyTexture(_osdIconTexture);
		_osdIconTexture = nullptr;
	}
#endif

	if (_mouseTexture) {
		SDL_DestroyTexture(_mouseTexture);
		_mouseTexture = nullptr;
	}

	if (_overlayTexture) {
		SDL_DestroyTexture(_overlayTexture);
		_overlayTexture = nullptr;
	}

	if (_overlaySurface) {
		SDL_FreeSurface(_overlaySurface);
		_overlaySurface = nullptr;
	}

	if (_screenTexture) {
		SDL_DestroyTexture(_screenTexture);
		_screenTexture = nullptr;
	}

	if (_hwScreen) {
		destroySurface(_hwScreen);
		_hwScreen = nullptr;
	}

	if (_tmpscreen) {
		destroySurface(_tmpscreen);
		_tmpscreen = nullptr;
	}

	if (_renderer) {
		SDL_DestroyRenderer(_renderer);
		_renderer = nullptr;
	}
}

bool RenderSdlGraphicsManager::hotswapGFXMode() {
	if (!_screen)
		return false;

	// Keep around the old _screen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	_screen = nullptr;

	// Release the HW screen surface
	if (_hwScreen) {
		destroySurface(_hwScreen);
		_hwScreen = nullptr;
	}
	if (_tmpscreen) {
		destroySurface(_tmpscreen);
		_tmpscreen = nullptr;
	}

	// Setup the new GFX mode
	if (!loadGFXMode()) {
		unloadGFXMode();

		_screen = old_screen;

		return false;
	}

	// reset palette
	if (_screen->format->palette)
		SDL_SetPaletteColors(_screen->format->palette, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, nullptr, _screen, nullptr);

	// Free the old surfaces
	destroySurface(old_screen);

	// Update cursor to new scale
	blitCursor();

	// Blit everything to the screen
	internUpdateScreen();

	return true;
}

void RenderSdlGraphicsManager::updateScreen() {
	assert(_transactionMode == kTransactionNone);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	internUpdateScreen();
}

void RenderSdlGraphicsManager::updateScreen(SDL_Rect *dirtyRectList, int actualDirtyRects) {
	SDL_UpdateTexture(_screenTexture, nullptr, _hwScreen->pixels, _hwScreen->pitch);
}

void RenderSdlGraphicsManager::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;
	int scale1;

	// If there's an active debugger, update it
	GUI::Debugger *debugger = g_engine ? g_engine->getDebugger() : nullptr;
	if (debugger)
		debugger->onFrame();

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_screen && _paletteDirtyEnd != 0) {
		if (_screen->format->palette)
			SDL_SetPaletteColors(_screen->format->palette,
				_currentPalette + _paletteDirtyStart,
				_paletteDirtyStart,
				_paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceRedraw = true;
	}

	int oldScaleFactor;

	origSurf = _screen;
	srcSurf = _tmpscreen;
	width = _videoMode.screenWidth;
	height = _videoMode.screenHeight;
	oldScaleFactor = scale1 = _videoMode.scaleFactor;

#ifdef USE_OSD
	updateOSD();
#endif

#if defined(USE_IMGUI) && (defined(USE_IMGUI_SDLRENDERER2) || defined(USE_IMGUI_SDLRENDERER3))
	if (_imGuiCallbacks.render) {
		_forceRedraw = true;
	}
#endif

	// Force a full redraw if requested.
	// If _useOldSrc, the scaler will do its own partial updates.
	if (_forceRedraw) {
		_numDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = width;
		_dirtyRectList[0].h = height;
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0) {
		SDL_Rect *r;
		SDL_Rect dst;
		uint32 bpp, srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			dst = *r;
			dst.x += _maxExtraPixels;	// Shift rect since some scalers need to access the data around
			dst.y += _maxExtraPixels;	// any pixel to scale it, and we want to avoid mem access crashes.

			if (!blitSurface(origSurf, r, srcSurf, &dst))
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		SDL_LockSurface(_hwScreen);

#if SDL_VERSION_ATLEAST(3, 0, 0)
		const SDL_PixelFormatDetails *pixelFormatDetails = SDL_GetPixelFormatDetails(_hwScreen->format);
		if (!pixelFormatDetails)
			error("SDL_GetPixelFormatDetails failed: %s", SDL_GetError());
		bpp = pixelFormatDetails->bytes_per_pixel;
#else
		bpp = _hwScreen->format->BytesPerPixel;
#endif
		srcPitch = srcSurf->pitch;
		dstPitch = _hwScreen->pitch;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			int src_x = r->x;
			int src_y = r->y;
			int dst_x = r->x;
			int dst_y = r->y;
			int dst_w = r->w;
			int dst_h = r->h;

			dst_x += _currentShakeXOffset;
			if (dst_x < 0) {
				src_x -= dst_x;
				dst_w += dst_x;
				dst_x = 0;
			}

			dst_y += _currentShakeYOffset;
			if (dst_y < 0) {
				src_y -= dst_y;
				dst_h += dst_y;
				dst_y = 0;
			}

			if (dst_x < width && dst_y < height && dst_w > 0 && dst_h > 0) {
				if (dst_w > width - dst_x)
					dst_w = width - dst_x;
				if (dst_w > width - src_x)
					dst_w = width - src_x;

				if (dst_h > height - dst_y)
					dst_h = height - dst_y;
				if (dst_h > height - src_y)
					dst_h = height - src_y;

				dst_x *= scale1;
				dst_y *= scale1;

				_scaler->scale((byte *)srcSurf->pixels + (src_x + _maxExtraPixels) * bpp + (src_y + _maxExtraPixels) * srcPitch, srcPitch,
						(byte *)_hwScreen->pixels + dst_x * bpp + dst_y * dstPitch, dstPitch, dst_w, dst_h, src_x, src_y);

				r->x = dst_x;
				r->y = dst_y;
				r->w = dst_w * scale1;
				r->h = dst_h * scale1;
			}
		}
		SDL_UnlockSurface(srcSurf);
		SDL_UnlockSurface(_hwScreen);

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceRedraw) {
			_dirtyRectList[0].x = 0;
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].w = _videoMode.hardwareWidth;
			_dirtyRectList[0].h = _videoMode.hardwareHeight;
		}

		// Finally, blit all our changes to the screen
		if (!_displayDisabled) {
			updateScreen(_dirtyRectList, _numDirtyRects);
		}
	}

	SDL_RenderClear(_renderer);
	drawScreen();
	drawOverlay();
	drawMouse();
#ifdef USE_OSD
	drawOSD();
#endif

	// Set up the old scale factor
	if (_scaler)
		_scaler->setFactor(oldScaleFactor);

	_numDirtyRects = 0;
	_forceRedraw = false;
	_cursorNeedsRedraw = false;

#if defined(USE_IMGUI) && (defined(USE_IMGUI_SDLRENDERER2) || defined(USE_IMGUI_SDLRENDERER3))
	renderImGui();
#endif

	SDL_RenderPresent(_renderer);
}

bool RenderSdlGraphicsManager::saveScreenshot(const Common::Path &filename) const {
	assert(_hwScreen != nullptr);

	Common::StackLock lock(_graphicsMutex);

	Common::DumpFile out;
	if (!out.open(filename)) {
		return false;
	}

	Graphics::Surface data;
	data.create(_windowWidth, _windowHeight, Graphics::PixelFormat::createFormatRGB24());

	if (SDL_RenderReadPixels(_renderer, NULL, SDL_PIXELFORMAT_RGB24, data.getPixels(), data.pitch) < 0) {
		warning("Could not create screenshot: %s", SDL_GetError());
		data.free();
		return false;
	}

#ifdef USE_PNG
	const bool success = Image::writePNG(out, data);
#else
	const bool success = Image::writeBMP(out, data);
#endif

	data.free();

	return success;
}

void RenderSdlGraphicsManager::setFullscreenMode(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (!g_system->hasFeature(OSystem::kFeatureFullscreenMode))
		return;

	if (_oldVideoMode.setup && _oldVideoMode.fullscreen == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.fullscreen = enable;
		_transactionDetails.needHotswap = true;
	}
}

void RenderSdlGraphicsManager::setVSync(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (!g_system->hasFeature(OSystem::kFeatureVSync))
		return;

	if (_oldVideoMode.setup && _oldVideoMode.vsync == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.vsync = enable;
		_transactionDetails.needHotswap = true;
	}
}

void RenderSdlGraphicsManager::setAspectRatioCorrection(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _oldVideoMode.aspectRatioCorrection == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.aspectRatioCorrection = enable;
		_transactionDetails.needUpdatescreen = true;
		_transactionDetails.needDisplayResize = true;
	}
}

void RenderSdlGraphicsManager::setFilteringMode(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _oldVideoMode.filtering == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.filtering = enable;
		_transactionDetails.needTextureUpdate = true;
	}
}

void RenderSdlGraphicsManager::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	assert(_transactionMode == kTransactionNone);
	assert(buf);

	if (_screen == nullptr) {
		warning("RenderSdlGraphicsManager::copyRectToScreen: _screen == NULL");
		return;
	}

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	assert(x >= 0 && x < _videoMode.screenWidth);
	assert(y >= 0 && y < _videoMode.screenHeight);
	assert(h > 0 && y + h <= _videoMode.screenHeight);
	assert(w > 0 && x + w <= _videoMode.screenWidth);

	addDirtyRect(x, y, w, h);

	// Try to lock the screen surface
	if (!lockSurface(_screen))
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_screen->pixels + y * _screen->pitch + x * _screenFormat.bytesPerPixel;
	if (_videoMode.screenWidth == w && pitch == _screen->pitch) {
		memcpy(dst, buf, h*pitch);
	} else {
		const byte *src = (const byte *)buf;
		do {
			memcpy(dst, src, w * _screenFormat.bytesPerPixel);
			src += pitch;
			dst += _screen->pitch;
		} while (--h);
	}

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);
}

Graphics::Surface *RenderSdlGraphicsManager::lockScreen() {
	assert(_transactionMode == kTransactionNone);

	// Lock the graphics mutex
	_graphicsMutex.lock();

	// paranoia check
	assert(!_screenIsLocked);
	_screenIsLocked = true;

	// Try to lock the screen surface
	if (!lockSurface(_screen))
		error("SDL_LockSurface failed: %s", SDL_GetError());

	_framebuffer.init(_screen->w, _screen->h, _screen->pitch, _screen->pixels, _screenFormat);

	return &_framebuffer;
}

void RenderSdlGraphicsManager::unlockScreen() {
	assert(_transactionMode == kTransactionNone);

	// paranoia check
	assert(_screenIsLocked);
	_screenIsLocked = false;

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);

	// Trigger a full screen update
	_forceRedraw = true;

	// Finally unlock the graphics mutex
	_graphicsMutex.unlock();
}

void RenderSdlGraphicsManager::fillScreen(uint32 col) {
	Graphics::Surface *screen = lockScreen();
	if (screen)
		screen->fillRect(Common::Rect(screen->w, screen->h), col);
	unlockScreen();
}

void RenderSdlGraphicsManager::fillScreen(const Common::Rect &r, uint32 col) {
	Graphics::Surface *screen = lockScreen();
	if (screen)
		screen->fillRect(r, col);
	unlockScreen();
}

void RenderSdlGraphicsManager::addDirtyRect(int x, int y, int w, int h) {
	if (_forceRedraw)
		return;

	if (_numDirtyRects == NUM_DIRTY_RECT) {
		_forceRedraw = true;
		return;
	}

	int width = _videoMode.screenWidth;
	int height = _videoMode.screenHeight;

	// Extend the dirty region for scalers
	// that "smear" the screen, e.g. 2xSAI
	// Aspect ratio correction requires this to be at least one
	int adjust = MAX(_extraPixels, (uint)1);
	x -= adjust;
	y -= adjust;
	w += adjust * 2;
	h += adjust * 2;

	// clip
	if (x < 0) {
		w += x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		y = 0;
	}

	if (w > width - x) {
		w = width - x;
	}

	if (h > height - y) {
		h = height - y;
	}

	if (w == width && h == height) {
		_forceRedraw = true;
		return;
	}

	if (w > 0 && h > 0) {
		SDL_Rect *r = &_dirtyRectList[_numDirtyRects++];

		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
}

int16 RenderSdlGraphicsManager::getHeight() const {
	return _videoMode.screenHeight;
}

int16 RenderSdlGraphicsManager::getWidth() const {
	return _videoMode.screenWidth;
}

void RenderSdlGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	assert(colors);
	assert(_screenFormat.bytesPerPixel == 1);

	// Setting the palette before _screen is created is allowed - for now -
	// since we don't actually set the palette until the screen is updated.
	// But it could indicate a programming error, so let's warn about it.

	if (!_screen)
		warning("RenderSdlGraphicsManager::setPalette: _screen == NULL");

	const byte *b = colors;
	uint i;
	SDL_Color *base = _currentPalette + start;
	for (i = 0; i < num; i++, b += 3) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
		base[i].a = 255;
	}

	if (start < _paletteDirtyStart)
		_paletteDirtyStart = start;

	if (start + num > _paletteDirtyEnd)
		_paletteDirtyEnd = start + num;

	// Some games blink cursors with palette
	if (_cursorPaletteDisabled)
		blitCursor();
}

void RenderSdlGraphicsManager::grabPalette(byte *colors, uint start, uint num) const {
	assert(colors);
	assert(_screenFormat.bytesPerPixel == 1);

	const SDL_Color *base = _currentPalette + start;

	for (uint i = 0; i < num; ++i) {
		colors[i * 3] = base[i].r;
		colors[i * 3 + 1] = base[i].g;
		colors[i * 3 + 2] = base[i].b;
	}
}

void RenderSdlGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	assert(colors);
	const byte *b = colors;
	uint i;
	SDL_Color *base = _cursorPalette + start;
	for (i = 0; i < num; i++, b += 3) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
		base[i].a = 255;
	}

	_cursorPaletteDisabled = false;
	blitCursor();
}

#pragma mark -
#pragma mark --- Overlays ---
#pragma mark -

void RenderSdlGraphicsManager::clearOverlay() {
	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	if (!_overlayVisible)
		return;

	if (SDL_FillRect(_overlaySurface, NULL, 0) != 0)
		error("SDL_FillRect failed: %s", SDL_GetError());

	_overlayDirty = true;
}

void RenderSdlGraphicsManager::grabOverlay(Graphics::Surface &surface) const {
	assert(_transactionMode == kTransactionNone);

	if (_overlaySurface == nullptr)
		return;

	if (!lockSurface(_overlaySurface))
		error("SDL_LockSurface failed: %s", SDL_GetError());

	assert(surface.w >= _overlaySurface->w);
	assert(surface.h >= _overlaySurface->h);
	assert(surface.format.bytesPerPixel == _overlayFormat.bytesPerPixel);

	byte *src = (byte *)_overlaySurface->pixels;
	byte *dst = (byte *)surface.getPixels();
	Graphics::copyBlit(dst, src, surface.pitch, _overlaySurface->pitch,
		_overlaySurface->w, _overlaySurface->h, _overlayFormat.bytesPerPixel);

	SDL_UnlockSurface(_overlaySurface);
}

void RenderSdlGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	assert(_transactionMode == kTransactionNone);

	if (_overlaySurface == nullptr)
		return;

	const byte *src = (const byte *)buf;
#if SDL_VERSION_ATLEAST(3, 0, 0)
	const SDL_PixelFormatDetails *pixelFormatDetails = SDL_GetPixelFormatDetails(_overlaySurface->format);
	if (!pixelFormatDetails)
		error("SDL_GetPixelFormatDetails failed: %s", SDL_GetError());
	uint bpp = pixelFormatDetails->bytes_per_pixel;
#else
	uint bpp = _overlaySurface->format->BytesPerPixel;
#endif

	// Clip the coordinates
	if (x < 0) {
		w += x;
		src -= x * bpp;
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
		y = 0;
	}

	if (w > _overlaySurface->w - x) {
		w = _overlaySurface->w - x;
	}

	if (h > _overlaySurface->h - y) {
		h = _overlaySurface->h - y;
	}

	if (w <= 0 || h <= 0)
		return;

	// Mark the overlay as dirty
	_overlayDirty = true;

	if (!lockSurface(_overlaySurface))
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_overlaySurface->pixels + y * _overlaySurface->pitch + x * bpp;
	Graphics::copyBlit(dst, src, _overlaySurface->pitch, pitch, w, h, bpp);

	SDL_UnlockSurface(_overlaySurface);
}

void RenderSdlGraphicsManager::drawOverlay() {
	if (!_overlayTexture || !_overlaySurface || !_overlayVisible)
		return;

	if (_overlayDirty) {
		SDL_UpdateTexture(_overlayTexture, NULL, _overlaySurface->pixels, _overlaySurface->pitch);
		_overlayDirty = false;
	}

	SDL_Rect viewport;

	Common::Rect &drawRect = _overlayDrawRect;

	/* Destination rectangle represents the texture before rotation */
	if (_rotationMode == Common::kRotation90 || _rotationMode == Common::kRotation270) {
		viewport.w = drawRect.height();
		viewport.h = drawRect.width();
		int delta = (viewport.w - viewport.h) / 2;
		viewport.x = drawRect.left - delta;
		viewport.y = drawRect.top + delta;
	} else {
		viewport.w = drawRect.width();
		viewport.h = drawRect.height();
		viewport.x = drawRect.left;
		viewport.y = drawRect.top;
	}

	if (_rotationMode != 0) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_FRect fViewport;
		SDL_RectToFRect(&viewport, &fViewport);
		SDL_RenderTextureRotated(_renderer, _overlayTexture, nullptr, &fViewport, _rotationMode, nullptr, SDL_FLIP_NONE);
#else
		SDL_RenderCopyEx(_renderer, _overlayTexture, nullptr, &viewport, _rotationMode, nullptr, SDL_FLIP_NONE);
#endif
	} else {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_FRect fViewport;
		SDL_RectToFRect(&viewport, &fViewport);
		SDL_RenderTexture(_renderer, _overlayTexture, nullptr, &fViewport);
#else
		SDL_RenderCopy(_renderer, _overlayTexture, nullptr, &viewport);
#endif
	}
}

void RenderSdlGraphicsManager::recreateOverlay(const int width, const int height) {

	SDL_RendererInfo info;
	if (SDL_GetRendererInfo(_renderer, &info) < 0)
		error("SDL_GetRendererInfo failed: %s", SDL_GetError());

	uint overlayWidth = width;
	uint overlayHeight = height;
	Uint32 overlayFormat = SDL_PIXELFORMAT_UNKNOWN;

	Common::RotationMode rotation = _rotationMode;
	if (rotation == Common::kRotation90 || rotation == Common::kRotation270) {
		overlayWidth = height;
		overlayHeight = width;
	}

	// WORKAROUND: We can only support surfaces up to the maximum supported
	// texture size. Thus, in case we encounter a physical size bigger than
	// this maximum texture size we will simply use an overlay as big as
	// possible and then scale it to the physical display size. This sounds
	// bad but actually all recent chips should support full HD resolution
	// anyway. Thus, it should not be a real issue for modern hardware.
	if (   overlayWidth  > (uint)info.max_texture_width
	    || overlayHeight > (uint)info.max_texture_height) {
		const frac_t outputAspect = intToFrac(_windowWidth) / _windowHeight;

		if (outputAspect > (frac_t)FRAC_ONE) {
			overlayWidth  = info.max_texture_width;
			overlayHeight = intToFrac(overlayWidth) / outputAspect;
		} else {
			overlayHeight = info.max_texture_height;
			overlayWidth  = fracToInt(overlayHeight * outputAspect);
		}
	}

	// HACK: We limit the minimal overlay size to 256x200, which is the
	// minimum of the dimensions of the two resolutions 256x240 (NES) and
	// 320x200 (many DOS games use this). This hopefully assure that our
	// GUI has working layouts.
	overlayWidth = MAX<uint>(overlayWidth, 256);
	overlayHeight = MAX<uint>(overlayHeight, 200);

	// Pick the best pixel format for the overlay based on what the
	// renderer supports - prefer ones with alpha and higher bit depths.
	for (Uint32 i = 0; i < info.num_texture_formats; i++) {
		if (!SDL_ISPIXELFORMAT_PACKED(info.texture_formats[i]))
			continue;

		if (SDL_BITSPERPIXEL(info.texture_formats[i]) > SDL_BITSPERPIXEL(overlayFormat) &&
		   !SDL_ISPIXELFORMAT_ALPHA(overlayFormat)) {
			overlayFormat = info.texture_formats[i];
			continue;
		}

		if (SDL_BITSPERPIXEL(info.texture_formats[i]) > SDL_BITSPERPIXEL(overlayFormat) &&
		    SDL_ISPIXELFORMAT_ALPHA(info.texture_formats[i])) {
			overlayFormat = info.texture_formats[i];
			continue;
		}
	}
	if (overlayFormat == SDL_PIXELFORMAT_UNKNOWN)
		overlayFormat = SDL_PIXELFORMAT_RGBA32;

	// Recreate the overlay surface and texture with the new information.

	if (_overlaySurface) {
		SDL_FreeSurface(_overlaySurface);
	}
	_overlaySurface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, overlayWidth, overlayHeight,
						SDL_BITSPERPIXEL(overlayFormat), overlayFormat);

	if (_overlaySurface == nullptr)
		error("allocating _overlaySurface failed");

	_overlayFormat = convertSDLPixelFormat(_overlaySurface->format);

	if (_overlayTexture) {
		SDL_DestroyTexture(_overlayTexture);
	}
	_overlayTexture = SDL_CreateTexture(_renderer, _overlaySurface->format->format, SDL_TEXTUREACCESS_STREAMING,
						_overlaySurface->w, _overlaySurface->h);

	if (_overlayTexture == nullptr)
		error("allocating _overlayTexture failed");

	SDL_SetTextureBlendMode(_overlayTexture, SDL_BLENDMODE_BLEND);
}

#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

void RenderSdlGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keyColor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask, bool disableKeyColor) {

	if (mask && (!format || format->bytesPerPixel == 1)) {
		// 8-bit masked cursor, RenderSdl has no alpha mask support so we must convert this to color key
		const byte *bufBytes = static_cast<const byte *>(buf);

		uint numPixelUsingColor[256];
		for (uint i = 0; i < 256; i++)
			numPixelUsingColor[i] = 0;

		uint numPixels = w * h;

		for (uint i = 0; i < numPixels; i++) {
			if (mask[i] == kCursorMaskOpaque)
				numPixelUsingColor[bufBytes[i]]++;
		}

		uint bestColorNumPixels = 0xffffffffu;
		uint bestKey = 0;
		for (uint i = 0; i < 256; i++) {
			if (numPixelUsingColor[i] < bestColorNumPixels) {
				bestColorNumPixels = numPixelUsingColor[i];
				bestKey = i;
				if (bestColorNumPixels == 0)
					break;
			}
		}

		if (bestColorNumPixels != 0)
			warning("RenderSdlGraphicsManager::setMouseCursor: A mask was specified for an 8-bit cursor but the cursor couldn't be converted to color key");

		Common::Array<byte> maskedImage;
		maskedImage.resize(w * h);
		for (uint i = 0; i < numPixels; i++) {
			if (mask[i] == kCursorMaskOpaque)
				maskedImage[i] = bufBytes[i];
			else
				maskedImage[i] = static_cast<byte>(bestKey);
		}

		setMouseCursor(&maskedImage[0], w, h, hotspotX, hotspotY, bestKey, dontScale, format, nullptr, disableKeyColor);
		return;
	}

#ifdef USE_RGB_COLOR
	if (mask && format && format->bytesPerPixel > 1) {
		const uint numPixels = w * h;
		const uint inBPP = format->bytesPerPixel;

		Graphics::PixelFormat formatWithAlpha = Graphics::createPixelFormat<8888>();

		// Use the existing format if it already has alpha
		if (format->aBits() > 0)
			formatWithAlpha = *format;

		const uint outBPP = formatWithAlpha.bytesPerPixel;

		Common::Array<byte> maskedImage;
		maskedImage.resize(numPixels * outBPP);

		uint32 inColor = 0;
		byte *inColorPtr = reinterpret_cast<byte *>(&inColor);
#ifdef SCUMM_BIG_ENDIAN
		inColorPtr += 4 - inBPP;
#endif

		uint32 outColor = 0;
		byte *outColorPtr = reinterpret_cast<byte *>(&outColor);
#ifdef SCUMM_BIG_ENDIAN
		outColorPtr += 4 - inBPP;
#endif

		for (uint i = 0; i < numPixels; i++) {
			if (mask[i] != kCursorMaskOpaque)
				outColor = 0;
			else {
				memcpy(inColorPtr, static_cast<const byte *>(buf) + i * inBPP, inBPP);

				uint8 r = 0;
				uint8 g = 0;
				uint8 b = 0;
				uint8 a = 0;
				format->colorToARGB(inColor, a, r, g, b);
				if (a == 0)
					outColor = 0;
				else
					outColor = formatWithAlpha.ARGBToColor(a, r, g, b);
			}
			memcpy(&maskedImage[i * outBPP], outColorPtr, outBPP);
		}

		// Disable the key color because SDL_SetColorKey ignores the alpha channel, which would make 0xFF000000 transparent
		setMouseCursor(&maskedImage[0], w, h, hotspotX, hotspotY, 0, dontScale, &formatWithAlpha, nullptr, true);
		return;
	}
#endif

	bool formatChanged = false;

	if (format) {
		if (format->bytesPerPixel != _cursorFormat.bytesPerPixel) {
			formatChanged = true;
		}
		_cursorFormat = *format;
	} else {
		if (_cursorFormat.bytesPerPixel != 1) {
			formatChanged = true;
		}
		_cursorFormat = Graphics::PixelFormat::createFormatCLUT8();
	}

	if (_cursorFormat.bytesPerPixel < 4) {
		assert(keyColor < 1U << (_cursorFormat.bytesPerPixel * 8));
	}

	_mouseCurState.hotX = hotspotX;
	_mouseCurState.hotY = hotspotY;

	bool keycolorChanged = false;

	if (_mouseKeyColor != keyColor || _disableMouseKeyColor != disableKeyColor) {
		_mouseKeyColor = keyColor;
		_disableMouseKeyColor = disableKeyColor;
		keycolorChanged = true;
	}

	_cursorDontScale = dontScale;

	if (_mouseCurState.w != (int)w || _mouseCurState.h != (int)h || formatChanged || !_mouseOrigSurface) {
		_mouseCurState.w = w;
		_mouseCurState.h = h;

		if (_mouseOrigSurface) {
			destroySurface(_mouseOrigSurface);

			if (_mouseSurface == _mouseOrigSurface) {
				_mouseSurface = nullptr;
			}

			_mouseOrigSurface = nullptr;
		}

		if (formatChanged && _mouseSurface) {
			destroySurface(_mouseSurface);
			_mouseSurface = nullptr;
		}

		if (formatChanged && _mouseTexture) {
			SDL_DestroyTexture(_mouseTexture);
			_mouseTexture = nullptr;
		}

		if (!w || !h) {
			return;
		}

		assert(!_mouseOrigSurface);

		// Allocate bigger surface because scalers will read past the boudaries.
#if SDL_VERSION_ATLEAST(3, 0, 0)
		_mouseOrigSurface = SDL_CreateSurface(
						_mouseCurState.w + _maxExtraPixels * 2,
						_mouseCurState.h + _maxExtraPixels * 2,
						SDL_GetPixelFormatForMasks(
							_cursorFormat.bytesPerPixel * 8,
							((0xFF >> _cursorFormat.rLoss) << _cursorFormat.rShift),
							((0xFF >> _cursorFormat.gLoss) << _cursorFormat.gShift),
							((0xFF >> _cursorFormat.bLoss) << _cursorFormat.bShift),
							((0xFF >> _cursorFormat.aLoss) << _cursorFormat.aShift)));
#else
		_mouseOrigSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
						_mouseCurState.w + _maxExtraPixels * 2,
						_mouseCurState.h + _maxExtraPixels * 2,
						_cursorFormat.bytesPerPixel * 8,
						((0xFF >> _cursorFormat.rLoss) << _cursorFormat.rShift),
						((0xFF >> _cursorFormat.gLoss) << _cursorFormat.gShift),
						((0xFF >> _cursorFormat.bLoss) << _cursorFormat.bShift),
						((0xFF >> _cursorFormat.aLoss) << _cursorFormat.aShift));
#endif

		if (_mouseOrigSurface == nullptr) {
			error("Allocating _mouseOrigSurface failed");
		}

		if (_mouseOrigSurface->format->Aloss < 8)
			SDL_SetSurfaceBlendMode(_mouseOrigSurface, SDL_BLENDMODE_BLEND);
		else
			SDL_SetSurfaceRLE(_mouseOrigSurface, SDL_TRUE);

#ifdef USE_SCALERS
		if (_mouseScaler != nullptr) {
			delete _mouseScaler;
			_mouseScaler = nullptr;
		}
		if (_scalerPlugin) {
			_mouseScaler = _scalerPlugin->createInstance(_cursorFormat);
		}
#endif

		// Force setup of border
		keycolorChanged = true;
	}

	if (keycolorChanged) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		uint32 flags = _disableMouseKeyColor ? SDL_FALSE : SDL_TRUE;
		SDL_SetSurfaceColorKey(_mouseOrigSurface, flags, _mouseKeyColor);
#else
		uint32 flags = _disableMouseKeyColor ? SDL_FALSE : SDL_TRUE;
		SDL_SetColorKey(_mouseOrigSurface, flags, _mouseKeyColor);
#endif
	}

	SDL_LockSurface(_mouseOrigSurface);

	if (keycolorChanged && _mouseScaler && _maxExtraPixels > 0 && _cursorFormat.aBits() == 0) {
		// We have extra pixels and no alpha channel, we must use color key on the borders
		Graphics::Surface cursorSurface;
		Common::Rect border(0, 0, _mouseCurState.w + _maxExtraPixels * 2, _mouseCurState.h + _maxExtraPixels * 2);
		cursorSurface.init(border.width(), border.height(), border.width() * _cursorFormat.bytesPerPixel,
				_mouseOrigSurface->pixels, _cursorFormat);
		for(unsigned int i = 0; i < _maxExtraPixels; i++) {
			cursorSurface.frameRect(border, _mouseKeyColor);
			border.grow(-1);
		}
	}

	// Draw from [_maxExtraPixels,_maxExtraPixels] since scalers will read past boudaries
#if SDL_VERSION_ATLEAST(3, 0, 0)
	Graphics::copyBlit((byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * _cursorFormat.bytesPerPixel,
	                   (const byte *)buf, _mouseOrigSurface->pitch, w * _cursorFormat.bytesPerPixel, w, h, _cursorFormat.bytesPerPixel);
#else
	Graphics::copyBlit((byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * _mouseOrigSurface->format->BytesPerPixel,
	                   (const byte *)buf, _mouseOrigSurface->pitch, w * _cursorFormat.bytesPerPixel, w, h, _cursorFormat.bytesPerPixel);
#endif
	SDL_UnlockSurface(_mouseOrigSurface);

	blitCursor();
}

void RenderSdlGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keyColor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	setMouseCursor(buf, w, h, hotspotX, hotspotY, keyColor, dontScale, format, mask, false);
}

void RenderSdlGraphicsManager::blitCursor() {
	const int w = _mouseCurState.w;
	const int h = _mouseCurState.h;

	if (!w || !h || !_mouseOrigSurface) {
		return;
	}

	if (!_mouseOrigSurface) {
		return;
	}

	int cursorScale;
	if (_cursorDontScale) {
		// Don't scale the cursor at all if the user requests this behavior.
		cursorScale = 1;
	} else {
		// Scale the cursor with the game screen scale factor.
		cursorScale = _videoMode.scaleFactor;
	}

	// Adapt the real hotspot according to the scale factor.
	int rW = w * cursorScale;
	int rH = h * cursorScale;
	_mouseCurState.rHotX = _mouseCurState.hotX * cursorScale;
	_mouseCurState.rHotY = _mouseCurState.hotY * cursorScale;

	// The virtual dimensions will be the same as the original.

	_mouseCurState.vW = w;
	_mouseCurState.vH = h;
	_mouseCurState.vHotX = _mouseCurState.hotX;
	_mouseCurState.vHotY = _mouseCurState.hotY;

	bool sizeChanged = false;
	if (_mouseCurState.rW != rW || _mouseCurState.rH != rH) {
		_mouseCurState.rW = rW;
		_mouseCurState.rH = rH;
		sizeChanged = true;
	}

	if (sizeChanged || !_mouseSurface) {
		if (_mouseTexture) {
			SDL_DestroyTexture(_mouseTexture);
			_mouseTexture = nullptr;
		}

		if (_mouseSurface)
			destroySurface(_mouseSurface);

#if SDL_VERSION_ATLEAST(3, 0, 0)
		const SDL_PixelFormatDetails *pixelFormatDetails = SDL_GetPixelFormatDetails(_mouseOrigSurface->format);
		if (pixelFormatDetails == nullptr)
			error("getting pixel format details failed");
		_mouseSurface = SDL_CreateSurface(
			_mouseCurState.rW,
			_mouseCurState.rH,
			SDL_GetPixelFormatForMasks(
				pixelFormatDetails->bits_per_pixel,
				pixelFormatDetails->Rmask,
				pixelFormatDetails->Gmask,
				pixelFormatDetails->Bmask,
				pixelFormatDetails->Amask));
#else
		_mouseSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
						_mouseCurState.rW,
						_mouseCurState.rH,
						_mouseOrigSurface->format->BitsPerPixel,
						_mouseOrigSurface->format->Rmask,
						_mouseOrigSurface->format->Gmask,
						_mouseOrigSurface->format->Bmask,
						_mouseOrigSurface->format->Amask);
#endif

		if (_mouseSurface == nullptr)
			error("Allocating _mouseSurface failed");

		if (_mouseSurface->format->Aloss < 8)
			SDL_SetSurfaceBlendMode(_mouseSurface, SDL_BLENDMODE_BLEND);
		else
			SDL_SetSurfaceRLE(_mouseSurface, SDL_TRUE);
	}

	if (_mouseSurface->format->palette)
		SDL_SetPaletteColors(_mouseSurface->format->palette,
			_cursorPaletteDisabled ? _currentPalette : _cursorPalette, 0, 256);
#if SDL_VERSION_ATLEAST(3, 0, 0)
	uint32 flags = _disableMouseKeyColor ? SDL_FALSE : SDL_TRUE;
	SDL_SetSurfaceColorKey(_mouseSurface, flags, _mouseKeyColor);
#else
	uint32 flags = _disableMouseKeyColor ? SDL_FALSE : SDL_TRUE;
	SDL_SetColorKey(_mouseSurface, flags, _mouseKeyColor);
#endif

	SDL_LockSurface(_mouseOrigSurface);
	SDL_LockSurface(_mouseSurface);

	// If possible, use the same scaler for the cursor as for the rest of
	// the game. This only works well with the non-blurring scalers so we
	// otherwise use the Normal scaler
	if (!_cursorDontScale) {
#ifdef USE_SCALERS
		// HACK: AdvMame4x requires a height of at least 4 pixels, so we
		// fall back on the Normal scaler when a smaller cursor is supplied.
		if (_mouseScaler && _scalerPlugin->canDrawCursor() && (uint)_mouseCurState.h >= _extraPixels) {
			_mouseScaler->setFactor(_videoMode.scaleFactor);
#if SDL_VERSION_ATLEAST(3, 0, 0)
			const SDL_PixelFormatDetails *pixelFormatDetails = SDL_GetPixelFormatDetails(_mouseOrigSurface->format);
			if (pixelFormatDetails == nullptr)
				error("getting pixel format details failed");
			_mouseScaler->scale(
					(byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * pixelFormatDetails->bytes_per_pixel,
					_mouseOrigSurface->pitch, (byte *)_mouseSurface->pixels, _mouseSurface->pitch,
					_mouseCurState.w, _mouseCurState.h, 0, 0);
#else
			_mouseScaler->scale(
					(byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * _mouseOrigSurface->format->BytesPerPixel,
					_mouseOrigSurface->pitch, (byte *)_mouseSurface->pixels, _mouseSurface->pitch,
					_mouseCurState.w, _mouseCurState.h, 0, 0);
#endif
		} else
#endif
		{
#if SDL_VERSION_ATLEAST(3, 0, 0)
			const SDL_PixelFormatDetails *pixelFormatDetails = SDL_GetPixelFormatDetails(_mouseOrigSurface->format);
			if (pixelFormatDetails == nullptr)
				error("getting pixel format details failed");
			Graphics::scaleBlit((byte *)_mouseSurface->pixels, (const byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * pixelFormatDetails->bytes_per_pixel,
			                    _mouseSurface->pitch, _mouseOrigSurface->pitch,
				                _mouseCurState.w * _videoMode.scaleFactor, _mouseCurState.h * _videoMode.scaleFactor,
			                    _mouseCurState.w, _mouseCurState.h, convertSDLPixelFormat(_mouseSurface->format));
#else
			Graphics::scaleBlit((byte *)_mouseSurface->pixels, (const byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * _mouseOrigSurface->format->BytesPerPixel,
			                    _mouseSurface->pitch, _mouseOrigSurface->pitch,
				                _mouseCurState.w * _videoMode.scaleFactor, _mouseCurState.h * _videoMode.scaleFactor,
			                    _mouseCurState.w, _mouseCurState.h, convertSDLPixelFormat(_mouseSurface->format));
#endif

		}
	} else {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		const SDL_PixelFormatDetails *srcPixelFormatDetails = SDL_GetPixelFormatDetails(_mouseOrigSurface->format);
		if (srcPixelFormatDetails == nullptr)
			error("getting pixel format details failed");
		const SDL_PixelFormatDetails *dstPixelFormatDetails = SDL_GetPixelFormatDetails(_mouseSurface->format);
		if (dstPixelFormatDetails == nullptr)
			error("getting pixel format details failed");
		Graphics::copyBlit((byte *)_mouseSurface->pixels, (const byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * srcPixelFormatDetails->bytes_per_pixel,
		                   _mouseSurface->pitch, _mouseOrigSurface->pitch,
		                   _mouseCurState.w, _mouseCurState.h, dstPixelFormatDetails->bytes_per_pixel);
#else
		Graphics::copyBlit((byte *)_mouseSurface->pixels, (const byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * _mouseOrigSurface->format->BytesPerPixel,
		                   _mouseSurface->pitch, _mouseOrigSurface->pitch,
		                   _mouseCurState.w, _mouseCurState.h, _mouseSurface->format->BytesPerPixel);
#endif
	}

	SDL_UnlockSurface(_mouseSurface);
	SDL_UnlockSurface(_mouseOrigSurface);

	if (!_mouseTexture) {
		// TODO: Pick a better pixel format?
		_mouseTexture = SDL_CreateTexture(_renderer,
						SDL_PIXELFORMAT_RGBA32,
						SDL_TEXTUREACCESS_STREAMING,
						_mouseSurface->w,
						_mouseSurface->h);

		if (_mouseTexture == nullptr)
			error("Creating _mouseTexture failed");

		SDL_SetTextureBlendMode(_mouseTexture, SDL_BLENDMODE_BLEND);
	}

	SDL_Surface *tmp;
	if (SDL_LockTextureToSurface(_mouseTexture, nullptr, &tmp) >= 0) {
		SDL_FillRect(tmp, nullptr, 0);
		SDL_BlitSurface(_mouseSurface, nullptr, tmp, nullptr);
		SDL_UnlockTexture(_mouseTexture);
	}

	recalculateCursorScaling();
}

void RenderSdlGraphicsManager::drawMouse() {
	if (!_cursorVisible || !_mouseTexture || !_mouseCurState.w || !_mouseCurState.h) {
		return;
	}

	SDL_Rect dst;

	// The offsets must be applied, since the call to convertWindowToVirtual()
	// counteracts the move of the view port.

	dst.x = _cursorX - _mouseCurState.vHotX + _gameScreenShakeXOffset;
	dst.y = _cursorY - _mouseCurState.vHotY + _gameScreenShakeYOffset;
	dst.w = _mouseCurState.vW;
	dst.h = _mouseCurState.vH;

	// We draw the pre-scaled cursor image, so now we need to adjust for
	// scaling and shake position manually.

	switch (_rotationMode) {
	case Common::kRotationNormal:
	case Common::kRotation180:
		break;
	case Common::kRotation90:
	case Common::kRotation270: {
//		SWAP(dst.w, dst.h);
		break;
	}
	}

	// TODO: Handle clipping?

	assert(_mouseTexture);
	if (_rotationMode != 0) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		const SDL_FPoint center = { 0, 0 };
		SDL_FRect dstF;
		SDL_RectToFRect(&dst, &dstF);
		if (!SDL_RenderTextureRotated(_renderer, _mouseTexture, nullptr, &dstF, _rotationMode, &center, SDL_FLIP_NONE))
			error("SDL_RenderTextureRotated failed: %s", SDL_GetError());
#else
		const SDL_Point center = { 0, 0 };
		if (SDL_RenderCopyEx(_renderer, _mouseTexture, nullptr, &dst, _rotationMode, &center, SDL_FLIP_NONE) != 0)
			error("SDL_RenderCopyEx failed: %s", SDL_GetError());
#endif
	} else {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_FRect dstF;
		SDL_RectToFRect(&dst, &dstF);
		if (!SDL_RenderTexture(_renderer, _mouseTexture, nullptr, &dstF))
			error("SDL_RenderTexture failed: %s", SDL_GetError());
#else
		if (SDL_RenderCopy(_renderer, _mouseTexture, nullptr, &dst) != 0)
			error("SDL_RenderCopy failed: %s", SDL_GetError());
#endif
	}
}

void RenderSdlGraphicsManager::recalculateCursorScaling() {
	if (!_mouseTexture) {
		return;
	}

#if SDL_VERSION_ATLEAST(3, 0, 0)
	float cursorWidth, cursorHeight;
	SDL_GetTextureSize(_mouseTexture, &cursorWidth, &cursorHeight);
#else
	int cursorWidth, cursorHeight;
	SDL_QueryTexture(_mouseTexture, nullptr, nullptr, &cursorWidth, &cursorHeight);
#endif

	// By default we use the unscaled versions.
	_mouseCurState.vHotX = _mouseCurState.rHotX;
	_mouseCurState.vHotY = _mouseCurState.rHotY;
	_mouseCurState.vW    = cursorWidth;
	_mouseCurState.vH    = cursorHeight;

	if (_rotationMode == Common::kRotation90 || _rotationMode == Common::kRotation270) {
		SWAP(cursorWidth, cursorHeight);
	}

	// In case scaling is actually enabled we will scale the cursor according
	// to the game screen.
	if (!_cursorDontScale) {
		const frac_t screenScaleFactorX = intToFrac(_gameDrawRect.width()) / _windowWidth;
		const frac_t screenScaleFactorY = intToFrac(_gameDrawRect.height()) / _windowHeight;

		_mouseCurState.vHotX = fracToInt(_mouseCurState.rHotX * screenScaleFactorX);
		_mouseCurState.vW    = fracToInt(cursorWidth          * screenScaleFactorX);

		_mouseCurState.vHotY = fracToInt(_mouseCurState.rHotY * screenScaleFactorY);
		_mouseCurState.vH    = fracToInt(cursorHeight         * screenScaleFactorY);
	}
}

#pragma mark -
#pragma mark --- On Screen Display ---
#pragma mark -

#ifdef USE_OSD
void RenderSdlGraphicsManager::displayMessageOnOSD(const Common::U32String &msg) {
	assert(_transactionMode == kTransactionNone);
	assert(!msg.empty());
	Common::U32String textToSay = msg;

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	removeOSDMessage();

	// The font we are going to use:
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);

	// Split the message into separate lines.
	Common::Array<Common::U32String> lines;
	Common::U32String::const_iterator strLineItrBegin = msg.begin();

	for (const auto &itr : msg) {
		if (itr == '\n') {
			lines.push_back(Common::U32String(strLineItrBegin, itr));
			strLineItrBegin = &itr + 1;
		}
	}
	if (strLineItrBegin != msg.end())
		lines.push_back(Common::U32String(strLineItrBegin, msg.end()));

	// Determine a rect which would contain the message string (clipped to the
	// screen dimensions).
	const int vOffset = 6;
	const int lineSpacing = 1;
	const int lineHeight = font->getFontHeight() + 2 * lineSpacing;
	int width = 0;
	int height = lineHeight * lines.size() + 2 * vOffset;
	uint i;
	for (i = 0; i < lines.size(); i++) {
		width = MAX(width, font->getStringWidth(lines[i]) + 14);
	}

	// Clip the rect
	if (width > _hwScreen->w)
		width = _hwScreen->w;
	if (height > _hwScreen->h)
		height = _hwScreen->h;

#if SDL_VERSION_ATLEAST(3, 0, 0)
	const SDL_PixelFormatDetails *pixelFormatDetails = SDL_GetPixelFormatDetails(_hwScreen->format);
	if (pixelFormatDetails == nullptr)
		error("getting pixel format details failed");
	SDL_Surface *osdMessageSurface = SDL_CreateSurface(
		width, height,
		SDL_GetPixelFormatForMasks(pixelFormatDetails->bits_per_pixel, pixelFormatDetails->Rmask, pixelFormatDetails->Gmask, pixelFormatDetails->Bmask, pixelFormatDetails->Amask));
#else
	SDL_Surface *osdMessageSurface = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		width, height, _hwScreen->format->BitsPerPixel, _hwScreen->format->Rmask, _hwScreen->format->Gmask, _hwScreen->format->Bmask, _hwScreen->format->Amask
	);
#endif

	// Lock the surface
	if (!lockSurface(osdMessageSurface))
		error("displayMessageOnOSD: SDL_LockSurface failed: %s", SDL_GetError());

	// Draw a dark gray rect
	// TODO: Rounded corners ? Border?
#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_FillSurfaceRect(osdMessageSurface, nullptr, SDL_MapSurfaceRGB(osdMessageSurface, 64, 64, 64));
#else
	SDL_FillRect(osdMessageSurface, nullptr, SDL_MapRGB(osdMessageSurface->format, 64, 64, 64));
#endif

	Graphics::Surface dst;
	dst.init(osdMessageSurface->w, osdMessageSurface->h, osdMessageSurface->pitch, osdMessageSurface->pixels,
		convertSDLPixelFormat(osdMessageSurface->format));

	// Render the message, centered, and in white
	for (i = 0; i < lines.size(); i++) {
		font->drawString(&dst, lines[i],
			0, 0 + i * lineHeight + vOffset + lineSpacing, width,
#if SDL_VERSION_ATLEAST(3, 0, 0)
			SDL_MapSurfaceRGB(osdMessageSurface, 255, 255, 255),
#else
			SDL_MapRGB(osdMessageSurface->format, 255, 255, 255),
#endif
			Graphics::kTextAlignCenter, 0, true);
	}

	// Finished drawing, so unlock the OSD message surface
	SDL_UnlockSurface(osdMessageSurface);

	// Create a new texture from the surface
	_osdMessageTexture = SDL_CreateTextureFromSurface(_renderer, osdMessageSurface);

	// We don't need the surface any more
	SDL_FreeSurface(osdMessageSurface);

	// Init the OSD display parameters, and the fade out
	_osdMessageAlpha = SDL_ALPHA_TRANSPARENT + kOSDInitialAlpha * (SDL_ALPHA_OPAQUE - SDL_ALPHA_TRANSPARENT) / 100;
	_osdMessageFadeStartTime = SDL_GetTicks() + kOSDFadeOutDelay;
	// Enable alpha blending
	SDL_SetTextureBlendMode(_osdMessageTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(_osdMessageTexture, _osdMessageAlpha);

#if defined(MACOSX)
	macOSTouchbarUpdate(msg.encode().c_str());
#endif
	// Ensure a full redraw takes place next time the screen is updated
	_forceRedraw = true;
	if (ConfMan.hasKey("tts_enabled", "scummvm") &&
			ConfMan.getBool("tts_enabled", "scummvm")) {
		Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
		if (ttsMan)
			ttsMan->say(textToSay);
	}
}

SDL_Rect RenderSdlGraphicsManager::getOSDMessageRect() const {
	int w, h;
	SDL_QueryTexture(_osdMessageTexture, nullptr, nullptr, &w, &h);

	SDL_Rect rect;
	rect.x = (_windowWidth - w) / 2;
	rect.y = (_windowHeight - h) / 2;
	rect.w = w;
	rect.h = h;
	return rect;
}

void RenderSdlGraphicsManager::displayActivityIconOnOSD(const Graphics::Surface *icon) {
	assert(_transactionMode == kTransactionNone);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	if (_osdIconTexture) {
		SDL_DestroyTexture(_osdIconTexture);
		_osdIconTexture = nullptr;
	}

	if (icon) {
		const Graphics::PixelFormat &iconFormat = icon->format;

#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_Surface *osdIconSurface = SDL_CreateSurfaceFrom(
				icon->w, icon->h,
				SDL_GetPixelFormatForMasks(
					iconFormat.bytesPerPixel * 8,
					((0xFF >> iconFormat.rLoss) << iconFormat.rShift),
					((0xFF >> iconFormat.gLoss) << iconFormat.gShift),
					((0xFF >> iconFormat.bLoss) << iconFormat.bShift),
					((0xFF >> iconFormat.aLoss) << iconFormat.aShift)),
				const_cast<void *>(icon->getPixels()),
				icon->pitch
		);
#else
		SDL_Surface *osdIconSurface = SDL_CreateRGBSurfaceFrom(const_cast<void *>(icon->getPixels()),
				icon->w, icon->h, iconFormat.bytesPerPixel * 8, icon->pitch,
				((0xFF >> iconFormat.rLoss) << iconFormat.rShift),
				((0xFF >> iconFormat.gLoss) << iconFormat.gShift),
				((0xFF >> iconFormat.bLoss) << iconFormat.bShift),
				((0xFF >> iconFormat.aLoss) << iconFormat.aShift)
		);
#endif

		// Create a new texture from the surface
		_osdIconTexture = SDL_CreateTextureFromSurface(_renderer, osdIconSurface);

		// We don't need the surface any more
		SDL_FreeSurface(osdIconSurface);

		// Enable alpha blending
		SDL_SetTextureBlendMode(_osdIconTexture, SDL_BLENDMODE_BLEND);
	}
}

SDL_Rect RenderSdlGraphicsManager::getOSDIconRect() const {
	int w, h;
	SDL_QueryTexture(_osdIconTexture, nullptr, nullptr, &w, &h);

	SDL_Rect dstRect;
	dstRect.x = _windowWidth - w - 10;
	dstRect.y = 10;
	dstRect.w = w;
	dstRect.h = h;
	return dstRect;
}

void RenderSdlGraphicsManager::removeOSDMessage() {
	// Remove the previous message
	if (_osdMessageTexture) {
		SDL_DestroyTexture(_osdMessageTexture);
		_forceRedraw = true;
	}

	_osdMessageTexture = nullptr;
	_osdMessageAlpha = SDL_ALPHA_TRANSPARENT;

#if defined(MACOSX)
	macOSTouchbarUpdate(nullptr);
#endif
}

void RenderSdlGraphicsManager::updateOSD() {
	// OSD message visible (i.e. non-transparent)?
	if (_osdMessageAlpha != SDL_ALPHA_TRANSPARENT) {
		// Updated alpha value
		const int diff = SDL_GetTicks() - _osdMessageFadeStartTime;
		if (diff > 0) {
			if (diff >= kOSDFadeOutDuration) {
				// Back to full transparency
				_osdMessageAlpha = SDL_ALPHA_TRANSPARENT;
			} else {
				// Do a linear fade out...
				const int startAlpha = SDL_ALPHA_TRANSPARENT + kOSDInitialAlpha * (SDL_ALPHA_OPAQUE - SDL_ALPHA_TRANSPARENT) / 100;
				_osdMessageAlpha = startAlpha + diff * (SDL_ALPHA_TRANSPARENT - startAlpha) / kOSDFadeOutDuration;
			}
			SDL_SetTextureAlphaMod(_osdMessageTexture, _osdMessageAlpha);
		}

		if (_osdMessageAlpha == SDL_ALPHA_TRANSPARENT) {
			removeOSDMessage();
		}
	}
}

void RenderSdlGraphicsManager::drawOSD() {
	if (_osdMessageTexture) {
		SDL_Rect dstRect = getOSDMessageRect();
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_FRect dstRectF;
		SDL_RectToFRect(&dstRect, &dstRectF);
		SDL_RenderTexture(_renderer, _osdMessageTexture, nullptr, &dstRectF);
#else
		SDL_RenderCopy(_renderer, _osdMessageTexture, nullptr, &dstRect);
#endif
	}

	if (_osdIconTexture) {
		SDL_Rect dstRect = getOSDIconRect();
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_FRect dstRectF;
		SDL_RectToFRect(&dstRect, &dstRectF);
		SDL_RenderTexture(_renderer, _osdIconTexture, nullptr, &dstRectF);
#else
		SDL_RenderCopy(_renderer, _osdIconTexture, nullptr, &dstRect);
#endif
	}
}

#endif

void RenderSdlGraphicsManager::drawScreen() {
	SDL_Rect viewport;

	Common::Rect &drawRect = _gameDrawRect;

	/* Destination rectangle represents the texture before rotation */
	if (_rotationMode == Common::kRotation90 || _rotationMode == Common::kRotation270) {
		viewport.w = drawRect.height();
		viewport.h = drawRect.width();
		int delta = (viewport.w - viewport.h) / 2;
		viewport.x = drawRect.left - delta;
		viewport.y = drawRect.top + delta;
	} else {
		viewport.w = drawRect.width();
		viewport.h = drawRect.height();
		viewport.x = drawRect.left;
		viewport.y = drawRect.top;
	}

	int rotangle = (int)_rotationMode;

	if (rotangle != 0) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_FRect fViewport;
		SDL_RectToFRect(&viewport, &fViewport);
		SDL_RenderTextureRotated(_renderer, _screenTexture, nullptr, &fViewport, rotangle, nullptr, SDL_FLIP_NONE);
#else
		SDL_RenderCopyEx(_renderer, _screenTexture, nullptr, &viewport, rotangle, nullptr, SDL_FLIP_NONE);
#endif
	} else {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_FRect fViewport;
		SDL_RectToFRect(&viewport, &fViewport);
		SDL_RenderTexture(_renderer, _screenTexture, nullptr, &fViewport);
#else
		SDL_RenderCopy(_renderer, _screenTexture, nullptr, &viewport);
#endif
	}
}

void RenderSdlGraphicsManager::handleResizeImpl(const int width, const int height) {
	SdlGraphicsManager::handleResizeImpl(width, height);

	recreateOverlay(width, height);
	recalculateDisplayAreas();
	recalculateCursorScaling();

	// Something changed, so update the screen change ID.
	_screenChangeCount++;
}

void RenderSdlGraphicsManager::handleScalerHotkeys(uint mode, int factor) {
	bool sizeChanged = _videoMode.scaleFactor != factor;

	beginGFXTransaction();
		setScaler(mode, factor);
	endGFXTransaction();
#ifdef USE_OSD
	const char *newScalerName = _scalerPlugin->getPrettyName();
	if (newScalerName) {
		const Common::U32String message = Common::U32String::format(
			"%S %s%d\n%d x %d -> %d x %d",
			_("Active graphics filter:").c_str(),
			newScalerName,
			_scaler->getFactor(),
			_videoMode.screenWidth, _videoMode.screenHeight,
			_hwScreen->w, _hwScreen->h);
		displayMessageOnOSD(message);
	}
#endif

	if (sizeChanged) {
		// Forcibly resizing the window here since a user switching scaler
		// size will not normally cause the window to update
		_window->createOrUpdateWindow(_hwScreen->w, _hwScreen->h, _lastFlags);
	}

	internUpdateScreen();
}

bool RenderSdlGraphicsManager::notifyEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_START) {
		return SdlGraphicsManager::notifyEvent(event);
	}

	switch ((CustomEventAction) event.customType) {
	case kActionToggleAspectRatioCorrection: {
		beginGFXTransaction();
			setFeatureState(OSystem::kFeatureAspectRatioCorrection, !_videoMode.aspectRatioCorrection);
		endGFXTransaction();
#ifdef USE_OSD
		if (getFeatureState(OSystem::kFeatureAspectRatioCorrection))
			displayMessageOnOSD(_("Enabled aspect ratio correction"));
		else
			displayMessageOnOSD(_("Disabled aspect ratio correction"));
#endif
		_forceRedraw = true;
		internUpdateScreen();
		return true;
	}

	case kActionToggleFilteredScaling:
		beginGFXTransaction();
			setFeatureState(OSystem::kFeatureFilteringMode, !_videoMode.filtering);
		endGFXTransaction();
#ifdef USE_OSD
		if (getFeatureState(OSystem::kFeatureFilteringMode)) {
			displayMessageOnOSD(_("Filtering enabled"));
		} else {
			displayMessageOnOSD(_("Filtering disabled"));
		}
#endif
		_forceRedraw = true;
		internUpdateScreen();
		return true;

	case kActionCycleStretchMode: {
		int index = 0;
		const OSystem::GraphicsMode *sm = s_supportedStretchModes;
		while (sm->name) {
			if (sm->id == _videoMode.stretchMode)
				break;
			sm++;
			index++;
		}
		index++;
		if (!s_supportedStretchModes[index].name)
			index = 0;

		beginGFXTransaction();
			setStretchMode(s_supportedStretchModes[index].id);
		endGFXTransaction();

#ifdef USE_OSD
		Common::U32String message = Common::U32String::format("%S: %S",
		                                                      _("Stretch mode").c_str(),
		                                                      _(s_supportedStretchModes[index].description).c_str()
		                            );
		displayMessageOnOSD(message);
#endif
		_forceRedraw = true;
		internUpdateScreen();
		return true;
	}

	case kActionIncreaseScaleFactor:
		handleScalerHotkeys(_videoMode.scalerIndex, _scaler->increaseFactor());
		return true;

	case kActionDecreaseScaleFactor:
		handleScalerHotkeys(_videoMode.scalerIndex, _scaler->decreaseFactor());
		return true;

	case kActionNextScaleFilter: {
		uint scalerIndex =  _videoMode.scalerIndex + 1;
		if (scalerIndex >= _scalerPlugins.size()) {
			scalerIndex = 0;
		}

		handleScalerHotkeys(scalerIndex, _scaler->getFactor());
		return true;
	}

	case kActionPreviousScaleFilter: {
		uint scalerIndex =  _videoMode.scalerIndex;
		if (scalerIndex == 0) {
			scalerIndex = _scalerPlugins.size();
		}
		scalerIndex--;

		handleScalerHotkeys(scalerIndex, _scaler->getFactor());
		return true;
	}

	default:
		return SdlGraphicsManager::notifyEvent(event);
	}
}

void RenderSdlGraphicsManager::notifyVideoExpose() {
	_forceRedraw = true;
}

void RenderSdlGraphicsManager::notifyResize(const int width, const int height) {
	handleResize(width, height);
}

void RenderSdlGraphicsManager::recreateScreenTexture() {
	if (!_renderer)
		return;

#if !SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, _videoMode.filtering ? "linear" : "nearest");
#endif

	SDL_Texture *oldTexture = _screenTexture;
	_screenTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	if (_screenTexture) {
		SDL_DestroyTexture(oldTexture);
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_SetTextureScaleMode(_screenTexture, _videoMode.filtering ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST);
#endif
	}
	else
		_screenTexture = oldTexture;
}

#if defined(USE_IMGUI) && (defined(USE_IMGUI_SDLRENDERER2) || defined(USE_IMGUI_SDLRENDERER3))
void *RenderSdlGraphicsManager::getImGuiTexture(const Graphics::Surface &image, const byte *palette, int palCount) {

	// Upload pixels into texture
	SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, image.w, image.h);
	if (texture == nullptr) {
		error("getImGuiTexture: errror creating tetxure: %s", SDL_GetError());
		return nullptr;
	}

	Graphics::Surface *s = image.convertTo(Graphics::PixelFormat::createFormatRGBA32(), palette, palCount);
	SDL_UpdateTexture(texture, nullptr, s->getPixels(), s->pitch);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
#ifdef USE_IMGUI_SDLRENDERER3
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
#elif defined(USE_IMGUI_SDLRENDERER2)
	SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
#endif

	s->free();
	delete s;

	return (void *)texture;
}

void RenderSdlGraphicsManager::freeImGuiTexture(void *texture) {
	SDL_DestroyTexture((SDL_Texture *) texture);
}
#endif // defined(USE_IMGUI) && (defined(USE_IMGUI_SDLRENDERER2) || defined(USE_IMGUI_SDLRENDERER3))

#endif
