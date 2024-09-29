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
#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
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
#include "graphics/scaler/aspect.h"
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

// SDL surface flags which got removed in SDL2.
#if SDL_VERSION_ATLEAST(2, 0, 0)
#define SDL_SRCCOLORKEY 0
#define SDL_SRCALPHA    0
#define SDL_FULLSCREEN  0x40000000
#endif

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"surfacesdl", _s("SDL Surface"), GFX_SURFACESDL},
	{nullptr, nullptr, 0}
};

#if SDL_VERSION_ATLEAST(2, 0, 0)
const OSystem::GraphicsMode s_supportedStretchModes[] = {
	{"center", _s("Center"), STRETCH_CENTER},
	{"pixel-perfect", _s("Pixel-perfect scaling"), STRETCH_INTEGRAL},
	{"fit", _s("Fit to window"), STRETCH_FIT},
	{"stretch", _s("Stretch to window"), STRETCH_STRETCH},
	{"fit_force_aspect", _s("Fit to window (4:3)"), STRETCH_FIT_FORCE_ASPECT},
	{nullptr, nullptr, 0}
};
#endif

SurfaceSdlGraphicsManager::AspectRatio::AspectRatio(int w, int h) {
	// TODO : Validation and so on...
	// Currently, we just ensure the program don't instantiate non-supported aspect ratios
	_kw = w;
	_kh = h;
}

#if defined(USE_ASPECT)
SurfaceSdlGraphicsManager::AspectRatio SurfaceSdlGraphicsManager::getDesiredAspectRatio() {
	const size_t AR_COUNT = 4;
	const char *desiredAspectRatioAsStrings[AR_COUNT] = {	"auto",				"4/3",				"16/9",				"16/10" };
	const AspectRatio desiredAspectRatios[AR_COUNT] = {		AspectRatio(0, 0),	AspectRatio(4,3),	AspectRatio(16,9),	AspectRatio(16,10) };

	//TODO : We could parse an arbitrary string, if we code enough proper validation
	Common::String desiredAspectRatio = ConfMan.get("desired_screen_aspect_ratio");

	for (size_t i = 0; i < AR_COUNT; i++) {
		assert(desiredAspectRatioAsStrings[i] != nullptr);

		if (!scumm_stricmp(desiredAspectRatio.c_str(), desiredAspectRatioAsStrings[i])) {
			return desiredAspectRatios[i];
		}
	}
	// TODO : Report a warning
	return AspectRatio(0, 0);
}
#endif

SurfaceSdlGraphicsManager::SurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window)
	:
	SdlGraphicsManager(sdlEventSource, window),
#ifdef USE_OSD
	_osdMessageSurface(nullptr), _osdMessageAlpha(SDL_ALPHA_TRANSPARENT), _osdMessageFadeStartTime(0),
	_osdIconSurface(nullptr),
#endif
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_renderer(nullptr), _screenTexture(nullptr),
#endif
#if defined(WIN32) && !SDL_VERSION_ATLEAST(2, 0, 0)
	_originalBitsPerPixel(0),
#endif
	_screen(nullptr), _tmpscreen(nullptr),
	_screenFormat(Graphics::PixelFormat::createFormatCLUT8()),
	_cursorFormat(Graphics::PixelFormat::createFormatCLUT8()),
	_useOldSrc(false), _isHwPalette(false),
	_overlayscreen(nullptr), _tmpscreen2(nullptr),
	_screenChangeCount(0),
	_mouseSurface(nullptr), _mouseScaler(nullptr),
	_mouseOrigSurface(nullptr), _cursorDontScale(false), _cursorPaletteDisabled(true),
	_currentShakeXOffset(0), _currentShakeYOffset(0),
	_paletteDirtyStart(0), _paletteDirtyEnd(0),
	_screenIsLocked(false),
	_displayDisabled(false),
#ifdef USE_SDL_DEBUG_FOCUSRECT
	_enableFocusRectDebugCode(false), _enableFocusRect(false), _focusRect(),
#endif
	_transactionMode(kTransactionNone),
	_scalerPlugins(ScalerMan.getPlugins()), _scalerPlugin(nullptr), _scaler(nullptr),
	_needRestoreAfterOverlay(false), _isInOverlayPalette(false), _isDoubleBuf(false), _prevForceRedraw(false), _numPrevDirtyRects(0),
	_prevCursorNeedsRedraw(false),
	_mouseKeyColor(0), _disableMouseKeyColor(false) {

	// allocate palette storage
	_currentPalette = (SDL_Color *)calloc(256, sizeof(SDL_Color));
	_overlayPalette = (SDL_Color *)calloc(256, sizeof(SDL_Color));
	_cursorPalette = (SDL_Color *)calloc(256, sizeof(SDL_Color));

	// Generate RGB332 palette for overlay
	for (uint i = 0; i < 256; i++) {
		_overlayPalette[i].r = ((i >> 5) & 7) << 5;
		_overlayPalette[i].g = ((i >> 2) & 7) << 5;
		_overlayPalette[i].b = (i & 3) << 6;
#if SDL_VERSION_ATLEAST(2, 0, 0)
		_overlayPalette[i].a = 0xff;
#endif
	}

	_mouseLastRect.x = _mouseLastRect.y = _mouseLastRect.w = _mouseLastRect.h = 0;
	_mouseNextRect.x = _mouseNextRect.y = _mouseNextRect.w = _mouseNextRect.h = 0;

#ifdef USE_SDL_DEBUG_FOCUSRECT
	if (ConfMan.hasKey("use_sdl_debug_focusrect"))
		_enableFocusRectDebugCode = ConfMan.getBool("use_sdl_debug_focusrect");
#endif

#if defined(USE_ASPECT)
	_videoMode.aspectRatioCorrection = ConfMan.getBool("aspect_ratio");
	_videoMode.desiredAspectRatio = getDesiredAspectRatio();
#else // for small screen platforms
	_videoMode.aspectRatioCorrection = false;
#endif

	_scaler = nullptr;
	_maxExtraPixels = ScalerMan.getMaxExtraPixels();

	_videoMode.fullscreen = ConfMan.getBool("fullscreen");
	_videoMode.filtering = ConfMan.getBool("filtering");
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_videoMode.stretchMode = STRETCH_FIT;
#endif
	_videoMode.vsync = ConfMan.getBool("vsync");

	_videoMode.scalerIndex = getDefaultScaler();
	_videoMode.scaleFactor = getDefaultScaleFactor();
}

SurfaceSdlGraphicsManager::~SurfaceSdlGraphicsManager() {
	unloadGFXMode();
	delete _scaler;
	delete _mouseScaler;
	if (_mouseOrigSurface) {
		SDL_FreeSurface(_mouseOrigSurface);
		if (_mouseOrigSurface == _mouseSurface) {
			_mouseSurface = nullptr;
		}
	}
	if (_mouseSurface) {
		SDL_FreeSurface(_mouseSurface);
	}
	free(_currentPalette);
	free(_overlayPalette);
	free(_cursorPalette);
}

bool SurfaceSdlGraphicsManager::hasFeature(OSystem::Feature f) const {
	return
		(f == OSystem::kFeatureFullscreenMode) ||
#ifdef USE_SCALERS
		(f == OSystem::kFeatureScalers) ||
#endif
#ifdef USE_ASPECT
		(f == OSystem::kFeatureAspectRatioCorrection) ||
#endif
		(f == OSystem::kFeatureFilteringMode) ||
#if SDL_VERSION_ATLEAST(2, 0, 0)
		(f == OSystem::kFeatureFullscreenToggleKeepsContext) ||
		(f == OSystem::kFeatureStretchMode) ||
		(f == OSystem::kFeatureRotationMode) ||
		(f == OSystem::kFeatureVSync) ||
#endif
		(f == OSystem::kFeatureCursorPalette) ||
		(f == OSystem::kFeatureCursorAlpha && !_isHwPalette) ||
		(f == OSystem::kFeatureIconifyWindow) ||
		(f == OSystem::kFeatureCursorMask);
}

void SurfaceSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		setFullscreenMode(enable);
		break;
#ifdef USE_ASPECT
	case OSystem::kFeatureAspectRatioCorrection:
		setAspectRatioCorrection(enable);
		break;
#endif
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
	case OSystem::kFeatureRotationMode:
		notifyResize(getWindowWidth(), getWindowHeight());
		break;
	default:
		break;
	}
}

bool SurfaceSdlGraphicsManager::getFeatureState(OSystem::Feature f) const {
	// We need to allow this to be called from within a transaction, since we
	// currently use it to retrieve the graphics state, when switching from
	// SDL->OpenGL mode for example.
	//assert(_transactionMode == kTransactionNone);

	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return _videoMode.fullscreen;
#ifdef USE_ASPECT
	case OSystem::kFeatureAspectRatioCorrection:
		return _videoMode.aspectRatioCorrection;
#endif
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

const OSystem::GraphicsMode *SurfaceSdlGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int SurfaceSdlGraphicsManager::getDefaultGraphicsMode() const {
	return GFX_SURFACESDL;
}

bool SurfaceSdlGraphicsManager::setGraphicsMode(int mode, uint flags) {
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

int SurfaceSdlGraphicsManager::getGraphicsMode() const {
	return _videoMode.mode;
}

void SurfaceSdlGraphicsManager::beginGFXTransaction() {
	assert(_transactionMode == kTransactionNone);

	_transactionMode = kTransactionActive;

	_transactionDetails.sizeChanged = false;

	_transactionDetails.needHotswap = false;
	_transactionDetails.needUpdatescreen = false;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	_transactionDetails.needDisplayResize = false;
	_transactionDetails.needTextureUpdate = false;
#endif
#ifdef USE_RGB_COLOR
	_transactionDetails.formatChanged = false;
#endif

	_oldVideoMode = _videoMode;
}

OSystem::TransactionError SurfaceSdlGraphicsManager::endGFXTransaction() {
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

#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (_videoMode.stretchMode != _oldVideoMode.stretchMode) {
			errors |= OSystem::kTransactionStretchModeSwitchFailed;

			_videoMode.stretchMode = _oldVideoMode.stretchMode;
		}
#endif
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
			_videoMode.overlayWidth = _oldVideoMode.overlayWidth;
			_videoMode.overlayHeight = _oldVideoMode.overlayHeight;
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

#if SDL_VERSION_ATLEAST(2, 0, 0)
			if (_transactionDetails.needDisplayResize)
				recalculateDisplayAreas();
#endif
			if (_transactionDetails.needUpdatescreen)
				internUpdateScreen();
		}
#if SDL_VERSION_ATLEAST(2, 0, 0)
	} else if (_transactionDetails.needTextureUpdate) {
		setGraphicsModeIntern();
		recreateScreenTexture();
		if (_transactionDetails.needDisplayResize)
			recalculateDisplayAreas();
		internUpdateScreen();
#endif
	} else if (_transactionDetails.needUpdatescreen) {
		setGraphicsModeIntern();
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (_transactionDetails.needDisplayResize)
			recalculateDisplayAreas();
#endif
		internUpdateScreen();
	}

	_transactionMode = kTransactionNone;
	return (OSystem::TransactionError)errors;
}

Graphics::PixelFormat SurfaceSdlGraphicsManager::convertSDLPixelFormat(SDL_PixelFormat *in) const {
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

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> SurfaceSdlGraphicsManager::getSupportedFormats() const {
	assert(!_supportedFormats.empty());
	return _supportedFormats;
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
static void maskToBitCount(Uint32 mask, uint8 &numBits, uint8 &shift) {
	numBits = 0;
	shift = 32;
	for (int i = 0; i < 32; ++i) {
		if (mask & 1) {
			if (i < shift) {
				shift = i;
			}
			++numBits;
		}

		mask >>= 1;
	}
}
#endif

void SurfaceSdlGraphicsManager::detectSupportedFormats() {
	_supportedFormats.clear();

	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();

#if SDL_VERSION_ATLEAST(2, 0, 0)
	{
		SDL_DisplayMode defaultMode;
		if (SDL_GetDesktopDisplayMode(_window->getDisplayIndex(), &defaultMode) != 0) {
			error("Could not get default system display mode");
		}

		int bpp;
		Uint32 rMask, gMask, bMask, aMask;
		if (SDL_PixelFormatEnumToMasks(defaultMode.format, &bpp, &rMask, &gMask, &bMask, &aMask) != SDL_TRUE) {
			error("Could not convert system pixel format %s to masks", SDL_GetPixelFormatName(defaultMode.format));
		}

		const uint8 bytesPerPixel = SDL_BYTESPERPIXEL(defaultMode.format);
		uint8 rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift;
		maskToBitCount(rMask, rBits, rShift);
		maskToBitCount(gMask, gBits, gShift);
		maskToBitCount(bMask, bBits, bShift);
		maskToBitCount(aMask, aBits, aShift);

		format = Graphics::PixelFormat(bytesPerPixel, rBits, gBits, bBits, aBits, rShift, gShift, bShift, aShift);

		_supportedFormats.push_back(format);
	}
#endif

	if (_hwScreen) {
		// Get our currently set hardware format
		Graphics::PixelFormat hwFormat = convertSDLPixelFormat(_hwScreen->format);

		_supportedFormats.push_back(hwFormat);

#if !SDL_VERSION_ATLEAST(2, 0, 0)
		format = hwFormat;
#endif
	}

	if (!_isHwPalette) {
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
	}

	// Finally, we always supposed 8 bit palette graphics
	_supportedFormats.push_back(Graphics::PixelFormat::createFormatCLUT8());
}
#endif

uint SurfaceSdlGraphicsManager::getDefaultScaler() const {
	return ScalerMan.findScalerPluginIndex("normal");
}

uint SurfaceSdlGraphicsManager::getDefaultScaleFactor() const {
#if defined (USE_SCALERS)
	return 2;
#else
	return 1;
#endif
}

bool SurfaceSdlGraphicsManager::setScaler(uint mode, int factor) {
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

void SurfaceSdlGraphicsManager::setGraphicsModeIntern() {
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

uint SurfaceSdlGraphicsManager::getScaler() const {
	assert(_transactionMode == kTransactionNone);
	return _videoMode.scalerIndex;
}

uint SurfaceSdlGraphicsManager::getScaleFactor() const {
	assert(_transactionMode == kTransactionNone);
	return _videoMode.scaleFactor;
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
const OSystem::GraphicsMode *SurfaceSdlGraphicsManager::getSupportedStretchModes() const {
	return s_supportedStretchModes;
}

int SurfaceSdlGraphicsManager::getDefaultStretchMode() const {
	return STRETCH_FIT;
}

bool SurfaceSdlGraphicsManager::setStretchMode(int mode) {
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

int SurfaceSdlGraphicsManager::getStretchMode() const {
	return _videoMode.stretchMode;
}
#endif

void SurfaceSdlGraphicsManager::getDefaultResolution(uint &w, uint &h) {
	w = 320;
	h = 200;
}

void SurfaceSdlGraphicsManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
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

#if !SDL_VERSION_ATLEAST(2, 0, 0)
	// Avoid redundant res changes, only in SDL1. In SDL2, redundancies may not
	// actually be redundant if ScummVM is switching between game engines and
	// the screen dimensions are being reinitialized, since window resizing is
	// supposed to reset when this happens
	if ((int)w == _videoMode.screenWidth && (int)h == _videoMode.screenHeight)
		return;
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

void SurfaceSdlGraphicsManager::fixupResolutionForAspectRatio(AspectRatio desiredAspectRatio, int &width, int &height) const {
	assert(&width != &height);

	if (desiredAspectRatio.isAuto())
		return;

	int kw = desiredAspectRatio.kw();
	int kh = desiredAspectRatio.kh();

	const int w = width;
	const int h = height;

	int bestW = 0, bestH = 0;
	uint bestMetric = (uint)-1; // Metric is wasted space

#if SDL_VERSION_ATLEAST(2, 0, 0)
	const int display = _window->getDisplayIndex();
	const int numModes = SDL_GetNumDisplayModes(display);
	SDL_DisplayMode modeData, *mode = &modeData;
	for (int i = 0; i < numModes; ++i) {
		if (SDL_GetDisplayMode(display, i, &modeData)) {
			continue;
		}
#else
	SDL_Rect const* const*availableModes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_SWSURFACE); //TODO : Maybe specify a pixel format
	assert(availableModes);

	while (const SDL_Rect *mode = *availableModes++) {
#endif
		if (mode->w < w)
			continue;
		if (mode->h < h)
			continue;
		if (mode->h * kw != mode->w * kh)
			continue;

		uint metric = mode->w * mode->h - w * h;
		if (metric > bestMetric)
			continue;

		bestMetric = metric;
		bestW = mode->w;
		bestH = mode->h;

	// Make editors a bit more happy by having the same amount of closing as
	// opening curley braces.
#if SDL_VERSION_ATLEAST(2, 0, 0)
	}
#else
	}
#endif

	if (!bestW || !bestH) {
		warning("Unable to enforce the desired aspect ratio");
		return;
	}
	width = bestW;
	height = bestH;
}

void SurfaceSdlGraphicsManager::setupHardwareSize() {
	_videoMode.overlayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.overlayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

	if (_videoMode.screenHeight != 200 && _videoMode.screenHeight != 400)
		_videoMode.aspectRatioCorrection = false;

	_videoMode.hardwareWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.hardwareHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

	if (_videoMode.aspectRatioCorrection) {
		_videoMode.overlayHeight = real2Aspect(_videoMode.overlayHeight);
		_videoMode.hardwareHeight = real2Aspect(_videoMode.hardwareHeight);
	}
}

void SurfaceSdlGraphicsManager::initGraphicsSurface() {
	Uint32 flags = SDL_SWSURFACE;
	if (_videoMode.fullscreen)
		flags |= SDL_FULLSCREEN;

	_hwScreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 16, flags);
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_isDoubleBuf = false;
	_isHwPalette = false;
#else
	_isDoubleBuf = flags & SDL_DOUBLEBUF;
	_isHwPalette = flags & SDL_HWPALETTE;
#endif
}

bool SurfaceSdlGraphicsManager::loadGFXMode() {
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
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight,
						_screenFormat.bytesPerPixel * 8, rMask, gMask, bMask, aMask);
	if (_screen == nullptr)
		error("allocating _screen failed");

#ifdef USE_RGB_COLOR
	// Avoid having SDL_SRCALPHA set even if we supplied an alpha-channel in the format.
	SDL_SetAlpha(_screen, 0, 255);
#endif

	// SDL 1.2 palettes default to all black,
	// SDL 1.3 palettes default to all white,
	// Thus set our own default palette to all black.
	// SDL_SetColors does nothing for non indexed surfaces.
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	if (_videoMode.fullscreen) {
		fixupResolutionForAspectRatio(_videoMode.desiredAspectRatio, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	}


#ifdef ENABLE_EVENTRECORDER
	_displayDisabled = ConfMan.getBool("disable_display");

	if (_displayDisabled) {
		_hwScreen = g_eventRec.getSurface(_videoMode.hardwareWidth, _videoMode.hardwareHeight);
	} else
#endif
	{
#if defined(WIN32) && !SDL_VERSION_ATLEAST(2, 0, 0)
		// Save the original bpp to be able to restore the video mode on
		// unload. See _originalBitsPerPixel documentation.
		if (_originalBitsPerPixel == 0) {
			const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
			_originalBitsPerPixel = videoInfo->vfmt->BitsPerPixel;
		}
#endif

		initGraphicsSurface();
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

#if !SDL_VERSION_ATLEAST(2, 0, 0)
	handleResize(_videoMode.hardwareWidth, _videoMode.hardwareHeight);
#endif

	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Need some extra bytes around when using 2xSaI
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth + _maxExtraPixels * 2,
						_videoMode.screenHeight + _maxExtraPixels * 2,
						_hwScreen->format->BitsPerPixel,
						_hwScreen->format->Rmask,
						_hwScreen->format->Gmask,
						_hwScreen->format->Bmask,
						_hwScreen->format->Amask);

	if (_tmpscreen == nullptr)
		error("allocating _tmpscreen failed");

	if (_useOldSrc) {
		// Create surface containing previous frame's data to pass to scaler
		_scaler->setSource((byte *)_tmpscreen->pixels, _tmpscreen->pitch,
									_videoMode.screenWidth, _videoMode.screenHeight, _maxExtraPixels);
	}

	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth, _videoMode.overlayHeight,
						_hwScreen->format->BitsPerPixel,
						_hwScreen->format->Rmask,
						_hwScreen->format->Gmask,
						_hwScreen->format->Bmask,
						_hwScreen->format->Amask);

	if (_overlayscreen == nullptr)
		error("allocating _overlayscreen failed");

	_overlayFormat = convertSDLPixelFormat(_overlayscreen->format);
	// Overlay uses RGB332 palette
	if (_overlayFormat.bytesPerPixel == 1 && _overlayFormat.rBits() == 0)
		_overlayFormat = Graphics::PixelFormat(1, 3, 3, 2, 0, 5, 2, 0, 0);

	_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth + _maxExtraPixels * 2,
						_videoMode.overlayHeight + _maxExtraPixels * 2,
						_hwScreen->format->BitsPerPixel,
						_hwScreen->format->Rmask,
						_hwScreen->format->Gmask,
						_hwScreen->format->Bmask,
						_hwScreen->format->Amask);

	if (_tmpscreen2 == nullptr)
		error("allocating _tmpscreen2 failed");

	if (_isHwPalette) {
		SDL_SetColors(_tmpscreen2, _overlayPalette, 0, 256);
		SDL_SetColors(_overlayscreen, _overlayPalette, 0, 256);
	}

	return true;
}

void SurfaceSdlGraphicsManager::unloadGFXMode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = nullptr;
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	deinitializeRenderer();
#endif

	if (_hwScreen) {
		SDL_FreeSurface(_hwScreen);
		_hwScreen = nullptr;
	}

	if (_tmpscreen) {
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = nullptr;
	}

	if (_tmpscreen2) {
		SDL_FreeSurface(_tmpscreen2);
		_tmpscreen2 = nullptr;
	}

	if (_overlayscreen) {
		SDL_FreeSurface(_overlayscreen);
		_overlayscreen = nullptr;
	}

#ifdef USE_OSD
	if (_osdMessageSurface) {
		SDL_FreeSurface(_osdMessageSurface);
		_osdMessageSurface = nullptr;
	}

	if (_osdIconSurface) {
		SDL_FreeSurface(_osdIconSurface);
		_osdIconSurface = nullptr;
	}
#endif

#if defined(WIN32) && !SDL_VERSION_ATLEAST(2, 0, 0)
	// Reset video mode to original.
	// This will ensure that any new graphic manager will use the initial BPP
	// when listing available modes. See _originalBitsPerPixel documentation.
	if (_originalBitsPerPixel != 0)
		SDL_SetVideoMode(_videoMode.screenWidth, _videoMode.screenHeight, _originalBitsPerPixel, _videoMode.fullscreen ? (SDL_FULLSCREEN | SDL_SWSURFACE) : SDL_SWSURFACE);
#endif
}

bool SurfaceSdlGraphicsManager::hotswapGFXMode() {
	if (!_screen)
		return false;

	// Keep around the old _screen & _overlayscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	_screen = nullptr;
	SDL_Surface *old_overlayscreen = _overlayscreen;
	_overlayscreen = nullptr;

	// Release the HW screen surface
	if (_hwScreen) {
		SDL_FreeSurface(_hwScreen);
		_hwScreen = nullptr;
	}
	if (_tmpscreen) {
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = nullptr;
	}
	if (_tmpscreen2) {
		SDL_FreeSurface(_tmpscreen2);
		_tmpscreen2 = nullptr;
	}

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
	SDL_BlitSurface(old_screen, nullptr, _screen, nullptr);
	SDL_BlitSurface(old_overlayscreen, nullptr, _overlayscreen, nullptr);

	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	SDL_FreeSurface(old_overlayscreen);

	// Update cursor to new scale
	blitCursor();

	// Blit everything to the screen
	internUpdateScreen();

	return true;
}

void SurfaceSdlGraphicsManager::updateScreen() {
	assert(_transactionMode == kTransactionNone);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	internUpdateScreen();
}

void SurfaceSdlGraphicsManager::updateScreen(SDL_Rect *dirtyRectList, int actualDirtyRects) {
	SDL_UpdateRects(_hwScreen, actualDirtyRects, dirtyRectList);
}

void SurfaceSdlGraphicsManager::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;
	int scale1;

	// If there's an active debugger, update it
	GUI::Debugger *debugger = g_engine ? g_engine->getDebugger() : nullptr;
	if (debugger)
		debugger->onFrame();

	bool curCursorNeedsRedraw = _cursorNeedsRedraw;
	if (_prevCursorNeedsRedraw && _isDoubleBuf) {
		_cursorNeedsRedraw = true;
	}
	_prevCursorNeedsRedraw = curCursorNeedsRedraw;

#if !SDL_VERSION_ATLEAST(2, 0, 0)
	// If the shake position changed, fill the dirty area with blackness
	// When building with SDL2, the shake offset is added to the active rect instead,
	// so this isn't needed there.
	if (_currentShakeXOffset != _gameScreenShakeXOffset ||
		(_cursorNeedsRedraw && _mouseLastRect.x <= _currentShakeXOffset)) {
		SDL_Rect blackrect = {0, 0, (Uint16)(_videoMode.screenWidth * _videoMode.scaleFactor), (Uint16)(_videoMode.screenHeight * _videoMode.scaleFactor)};

		if (_gameScreenShakeXOffset >= 0)
			blackrect.w = (Uint16)(_gameScreenShakeXOffset * _videoMode.scaleFactor);
		else {
			blackrect.w = ((-_gameScreenShakeXOffset) * _videoMode.scaleFactor);
			blackrect.x = ((_videoMode.screenWidth + _gameScreenShakeXOffset) * _videoMode.scaleFactor);
		}

		if (_videoMode.aspectRatioCorrection && !_overlayInGUI) {
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;
		}

		SDL_FillRect(_hwScreen, &blackrect, 0);

		_currentShakeXOffset = _gameScreenShakeXOffset;

		_forceRedraw = true;
	}
	if (_currentShakeYOffset != _gameScreenShakeYOffset ||
		(_cursorNeedsRedraw && _mouseLastRect.y <= _currentShakeYOffset)) {
		SDL_Rect blackrect = {0, 0, (Uint16)(_videoMode.screenWidth * _videoMode.scaleFactor), (Uint16)(_videoMode.screenHeight * _videoMode.scaleFactor)};

		if (_gameScreenShakeYOffset >= 0)
			blackrect.h = (Uint16)(_gameScreenShakeYOffset * _videoMode.scaleFactor);
		else {
			blackrect.h = ((-_gameScreenShakeYOffset) * _videoMode.scaleFactor);
			blackrect.y = ((_videoMode.screenHeight + _gameScreenShakeYOffset) * _videoMode.scaleFactor);
		}

		if (_videoMode.aspectRatioCorrection && !_overlayInGUI) {
			blackrect.y = real2Aspect(blackrect.y);
			blackrect.h = real2Aspect(blackrect.h + blackrect.y - 1) - blackrect.y + 1;
		}

		SDL_FillRect(_hwScreen, &blackrect, 0);

		_currentShakeYOffset = _gameScreenShakeYOffset;

		_forceRedraw = true;
	}
#endif

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_screen && _paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart,
			_paletteDirtyStart,
			_paletteDirtyEnd - _paletteDirtyStart);
		if (_isHwPalette)
			SDL_SetColors(_tmpscreen, _currentPalette + _paletteDirtyStart,
				      _paletteDirtyStart,
				      _paletteDirtyEnd - _paletteDirtyStart);
		if (_isHwPalette && !_isInOverlayPalette)
			SDL_SetColors(_hwScreen, _currentPalette + _paletteDirtyStart,
				       _paletteDirtyStart,
				       _paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceRedraw = true;
	}

	int oldScaleFactor;

	if (!_overlayVisible) {
		if (_needRestoreAfterOverlay) {
			// This is needed for the Edge scaler which seems to be the only scaler to use the "_useOldSrc" feature.
			// Otherwise the screen will not be properly restored after removing the overlay. We need to trigger a
			// regeneration of SourceScaler::_bufferedOutput. The call to _scaler->setFactor() down below could
			// do that in theory, but it won't unless the factor actually changes (which it doesn't). Now, the code
			// in SourceScaler::setSource() looks a bit fishy, e. g. the *src argument isn't even used. But otherwise
			// it does what we want here at least...
			_scaler->setSource(nullptr, _tmpscreen->pitch, _videoMode.screenWidth, _videoMode.screenHeight, _maxExtraPixels);
		}

		origSurf = _screen;
		srcSurf = _tmpscreen;
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
		oldScaleFactor = scale1 = _videoMode.scaleFactor;
		_needRestoreAfterOverlay = false;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
		scale1 = 1;
		oldScaleFactor = _scaler->setFactor(1);
		_needRestoreAfterOverlay = _useOldSrc;
	}

	// Add the area covered by the mouse cursor to the list of dirty rects if
	// we have to redraw the mouse, or if the cursor is alpha-blended since
	// alpha-blended cursors will happily blend into themselves if the surface
	// under the cursor is not reset first
	if (_cursorNeedsRedraw || _cursorFormat.aBits() > 1)
		undrawMouse();

#ifdef USE_OSD
	updateOSD();
#endif

	if (_isHwPalette && _isInOverlayPalette != _overlayVisible) {
		SDL_SetColors(_hwScreen, _overlayVisible ? _overlayPalette : _currentPalette, 0, 256);
		_forceRedraw = true;
		_isInOverlayPalette = _overlayVisible;
	}

	// In case of double buferring partially good version may be on another page,
	// so we need to fully redraw
	if (_isDoubleBuf && _numDirtyRects)
		_forceRedraw = true;

#if defined(USE_IMGUI) && defined(USE_IMGUI_SDLRENDERER2)
	if (_imGuiCallbacks.render) {
		_forceRedraw = true;
	}
#endif

	bool doRedraw = _forceRedraw || (_prevForceRedraw && _isDoubleBuf);
	int actualDirtyRects = _numDirtyRects;
	if (_isDoubleBuf && _numPrevDirtyRects > 0) {
		memcpy(_dirtyRectList + _numDirtyRects, _prevDirtyRectList, _numPrevDirtyRects * sizeof(_dirtyRectList[0]));
		actualDirtyRects += _numPrevDirtyRects;
	}

	// Force a full redraw if requested.
	// If _useOldSrc, the scaler will do its own partial updates.
	if (doRedraw) {
		actualDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = width;
		_dirtyRectList[0].h = height;
	}

	_prevForceRedraw = _forceRedraw;
	if (!_prevForceRedraw && _numDirtyRects && _isDoubleBuf) {
		memcpy(_prevDirtyRectList, _dirtyRectList, _numDirtyRects * sizeof(_dirtyRectList[0]));
		_numPrevDirtyRects = _numDirtyRects;
	}

	// Only draw anything if necessary
	bool doPresent = false;
	if (actualDirtyRects > 0 || _cursorNeedsRedraw) {
		SDL_Rect *r;
		SDL_Rect dst;
		uint32 bpp, srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + actualDirtyRects;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			dst = *r;
			dst.x += _maxExtraPixels;	// Shift rect since some scalers need to access the data around
			dst.y += _maxExtraPixels;	// any pixel to scale it, and we want to avoid mem access crashes.

			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		SDL_LockSurface(_hwScreen);

		bpp = _hwScreen->format->BytesPerPixel;
		srcPitch = srcSurf->pitch;
		dstPitch = _hwScreen->pitch;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			int src_x = r->x;
			int src_y = r->y;
			int dst_x = r->x;
			int dst_y = r->y;
			int dst_w = r->w;
			int dst_h = r->h;
#ifdef USE_ASPECT
			int orig_dst_y = 0;
#endif
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

#ifdef USE_ASPECT
				orig_dst_y = dst_y;
#endif
				dst_x *= scale1;
				dst_y *= scale1;

				if (_videoMode.aspectRatioCorrection && !_overlayInGUI)
					dst_y = real2Aspect(dst_y);

				_scaler->scale((byte *)srcSurf->pixels + (src_x + _maxExtraPixels) * bpp + (src_y + _maxExtraPixels) * srcPitch, srcPitch,
						(byte *)_hwScreen->pixels + dst_x * bpp + dst_y * dstPitch, dstPitch, dst_w, dst_h, src_x, src_y);

				r->x = dst_x;
				r->y = dst_y;
				r->w = dst_w * scale1;
				r->h = dst_h * scale1;

#ifdef USE_ASPECT
				if (_videoMode.aspectRatioCorrection && orig_dst_y < height && !_overlayInGUI)
					r->h = stretch200To240((uint8 *) _hwScreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y * scale1, _videoMode.filtering, 	convertSDLPixelFormat(_hwScreen->format));
#endif
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

		drawMouse();

#ifdef USE_OSD
		drawOSD();
#endif

#ifdef USE_SDL_DEBUG_FOCUSRECT
		// We draw the focus rectangle on top of everything, to assure it's easily visible.
		// Of course when the overlay is visible we do not show it, since it is only for game
		// specific focus.
		if (_enableFocusRect && !_overlayInGUI) {
			int x = _focusRect.left + _currentShakeXOffset;
			int y = _focusRect.top + _currentShakeYOffset;

			if (x < width && y < height) {
				int w = _focusRect.width();
				if (w > width - x)
					w = width - x;

				int h = _focusRect.height();
				if (h > height - y)
					h = height - y;

				x *= scale1;
				y *= scale1;
				w *= scale1;
				h *= scale1;

				if (_videoMode.aspectRatioCorrection && !_overlayInGUI)
					y = real2Aspect(y);

				if (h > 0 && w > 0) {
					SDL_LockSurface(_hwScreen);

					// Use white as color for now.
					Uint32 rectColor = SDL_MapRGB(_hwScreen->format, 0xFF, 0xFF, 0xFF);

					// First draw the top and bottom lines
					// then draw the left and right lines
					if (_hwScreen->format->BytesPerPixel == 2) {
						uint16 *top = (uint16 *)((byte *)_hwScreen->pixels + y * _hwScreen->pitch + x * 2);
						uint16 *bottom = (uint16 *)((byte *)_hwScreen->pixels + (y + h) * _hwScreen->pitch + x * 2);
						byte *left = ((byte *)_hwScreen->pixels + y * _hwScreen->pitch + x * 2);
						byte *right = ((byte *)_hwScreen->pixels + y * _hwScreen->pitch + (x + w - 1) * 2);

						while (w--) {
							*top++ = rectColor;
							*bottom++ = rectColor;
						}

						while (h--) {
							*(uint16 *)left = rectColor;
							*(uint16 *)right = rectColor;

							left += _hwScreen->pitch;
							right += _hwScreen->pitch;
						}
					} else if (_hwScreen->format->BytesPerPixel == 4) {
						uint32 *top = (uint32 *)((byte *)_hwScreen->pixels + y * _hwScreen->pitch + x * 4);
						uint32 *bottom = (uint32 *)((byte *)_hwScreen->pixels + (y + h) * _hwScreen->pitch + x * 4);
						byte *left = ((byte *)_hwScreen->pixels + y * _hwScreen->pitch + x * 4);
						byte *right = ((byte *)_hwScreen->pixels + y * _hwScreen->pitch + (x + w - 1) * 4);

						while (w--) {
							*top++ = rectColor;
							*bottom++ = rectColor;
						}

						while (h--) {
							*(uint32 *)left = rectColor;
							*(uint32 *)right = rectColor;

							left += _hwScreen->pitch;
							right += _hwScreen->pitch;
						}
					}

					SDL_UnlockSurface(_hwScreen);
				}
			}
		}
#endif

		// Finally, blit all our changes to the screen
		if (!_displayDisabled) {
			updateScreen(_dirtyRectList, actualDirtyRects);
			doPresent = true;
		}
	}

	// Set up the old scale factor
	if (_scaler)
		_scaler->setFactor(oldScaleFactor);

	_numDirtyRects = 0;
	_forceRedraw = false;
	_cursorNeedsRedraw = false;

#if SDL_VERSION_ATLEAST(2, 0, 0)

#if defined(USE_IMGUI) && defined(USE_IMGUI_SDLRENDERER2)
	renderImGui();
#endif

	if (doPresent) {
		SDL_RenderPresent(_renderer);
	}
#else
	if (_isDoubleBuf)
		SDL_Flip(_hwScreen);
#endif
}

bool SurfaceSdlGraphicsManager::saveScreenshot(const Common::Path &filename) const {
	assert(_hwScreen != nullptr);

	Common::StackLock lock(_graphicsMutex);

	Common::DumpFile out;
	if (!out.open(filename)) {
		return false;
	}

	int result = SDL_LockSurface(_hwScreen);
	if (result < 0) {
		warning("Could not lock RGB surface");
		return false;
	}

	Graphics::PixelFormat format = convertSDLPixelFormat(_hwScreen->format);
	Graphics::Surface data;
	data.init(_hwScreen->w, _hwScreen->h, _hwScreen->pitch, _hwScreen->pixels, format);

	bool success;

	SDL_Palette *sdlPalette = _hwScreen->format->palette;
	if (sdlPalette) {
		byte palette[256 * 3];
		for (int i = 0; i < sdlPalette->ncolors; i++) {
			palette[(i * 3) + 0] = sdlPalette->colors[i].r;
			palette[(i * 3) + 1] = sdlPalette->colors[i].g;
			palette[(i * 3) + 2] = sdlPalette->colors[i].b;
		}

#ifdef USE_PNG
		success = Image::writePNG(out, data, palette);
#else
		success = Image::writeBMP(out, data, palette);
#endif
	} else {
#ifdef USE_PNG
		success = Image::writePNG(out, data);
#else
		success = Image::writeBMP(out, data);
#endif
	}

	SDL_UnlockSurface(_hwScreen);

	return success;
}

void SurfaceSdlGraphicsManager::setFullscreenMode(bool enable) {
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

void SurfaceSdlGraphicsManager::setVSync(bool enable) {
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

void SurfaceSdlGraphicsManager::setAspectRatioCorrection(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _oldVideoMode.aspectRatioCorrection == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.aspectRatioCorrection = enable;
		_transactionDetails.needHotswap = true;
	}
}

void SurfaceSdlGraphicsManager::setFilteringMode(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _oldVideoMode.filtering == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.filtering = enable;
#if SDL_VERSION_ATLEAST(2, 0, 0)
		_transactionDetails.needTextureUpdate = true;
#endif
	}
}

void SurfaceSdlGraphicsManager::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	assert(_transactionMode == kTransactionNone);
	assert(buf);

	if (_screen == nullptr) {
		warning("SurfaceSdlGraphicsManager::copyRectToScreen: _screen == NULL");
		return;
	}

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	assert(x >= 0 && x < _videoMode.screenWidth);
	assert(y >= 0 && y < _videoMode.screenHeight);
	assert(h > 0 && y + h <= _videoMode.screenHeight);
	assert(w > 0 && x + w <= _videoMode.screenWidth);

	addDirtyRect(x, y, w, h, false);

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
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

Graphics::Surface *SurfaceSdlGraphicsManager::lockScreen() {
	assert(_transactionMode == kTransactionNone);

	// Lock the graphics mutex
	_graphicsMutex.lock();

	// paranoia check
	assert(!_screenIsLocked);
	_screenIsLocked = true;

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	_framebuffer.init(_screen->w, _screen->h, _screen->pitch, _screen->pixels, _screenFormat);

	return &_framebuffer;
}

void SurfaceSdlGraphicsManager::unlockScreen() {
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

void SurfaceSdlGraphicsManager::fillScreen(uint32 col) {
	Graphics::Surface *screen = lockScreen();
	if (screen)
		screen->fillRect(Common::Rect(screen->w, screen->h), col);
	unlockScreen();
}

void SurfaceSdlGraphicsManager::fillScreen(const Common::Rect &r, uint32 col) {
	Graphics::Surface *screen = lockScreen();
	if (screen)
		screen->fillRect(r, col);
	unlockScreen();
}

void SurfaceSdlGraphicsManager::addDirtyRect(int x, int y, int w, int h, bool inOverlay, bool realCoordinates) {
	if (_forceRedraw)
		return;

	if (_numDirtyRects == NUM_DIRTY_RECT) {
		_forceRedraw = true;
		return;
	}

	int height, width;

	if (!inOverlay && !realCoordinates) {
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
	} else {
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
	}

	// Extend the dirty region for scalers
	// that "smear" the screen, e.g. 2xSAI
	if (!realCoordinates) {
		// Aspect ratio correction requires this to be at least one
		int adjust = MAX(_extraPixels, (uint)1);
		x -= adjust;
		y -= adjust;
		w += adjust * 2;
		h += adjust * 2;
	}

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

#ifdef USE_ASPECT
	if (_videoMode.aspectRatioCorrection && !_overlayInGUI && !realCoordinates)
		makeRectStretchable(x, y, w, h, _videoMode.filtering);
#endif

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

int16 SurfaceSdlGraphicsManager::getHeight() const {
	return _videoMode.screenHeight;
}

int16 SurfaceSdlGraphicsManager::getWidth() const {
	return _videoMode.screenWidth;
}

void SurfaceSdlGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	assert(colors);
	assert(_screenFormat.bytesPerPixel == 1);

	// Setting the palette before _screen is created is allowed - for now -
	// since we don't actually set the palette until the screen is updated.
	// But it could indicate a programming error, so let's warn about it.

	if (!_screen)
		warning("SurfaceSdlGraphicsManager::setPalette: _screen == NULL");

	const byte *b = colors;
	uint i;
	SDL_Color *base = _currentPalette + start;
	for (i = 0; i < num; i++, b += 3) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
#if SDL_VERSION_ATLEAST(2, 0, 0)
		base[i].a = 255;
#endif
	}

	if (start < _paletteDirtyStart)
		_paletteDirtyStart = start;

	if (start + num > _paletteDirtyEnd)
		_paletteDirtyEnd = start + num;

	// Some games blink cursors with palette
	if (_cursorPaletteDisabled)
		blitCursor();
}

void SurfaceSdlGraphicsManager::grabPalette(byte *colors, uint start, uint num) const {
	assert(colors);
	assert(_screenFormat.bytesPerPixel == 1);

	const SDL_Color *base = _currentPalette + start;

	for (uint i = 0; i < num; ++i) {
		colors[i * 3] = base[i].r;
		colors[i * 3 + 1] = base[i].g;
		colors[i * 3 + 2] = base[i].b;
	}
}

void SurfaceSdlGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	assert(colors);
	const byte *b = colors;
	uint i;
	SDL_Color *base = _cursorPalette + start;
	for (i = 0; i < num; i++, b += 3) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
#if SDL_VERSION_ATLEAST(2, 0, 0)
		base[i].a = 255;
#endif
	}

	_cursorPaletteDisabled = false;
	blitCursor();
}

void SurfaceSdlGraphicsManager::setFocusRectangle(const Common::Rect &rect) {
#ifdef USE_SDL_DEBUG_FOCUSRECT
	// Only enable focus rectangle debug code, when the user wants it
	if (!_enableFocusRectDebugCode)
		return;

	_enableFocusRect = true;
	_focusRect = rect;

	if (rect.left < 0 || rect.top < 0 || rect.right > _videoMode.screenWidth || rect.bottom > _videoMode.screenHeight)
		warning("SurfaceSdlGraphicsManager::setFocusRectangle: Got a rect which does not fit inside the screen bounds: %d,%d,%d,%d", rect.left, rect.top, rect.right, rect.bottom);

	// It's gross but we actually sometimes get rects, which are not inside the screen bounds,
	// thus we need to clip the rect here...
	_focusRect.clip(_videoMode.screenWidth, _videoMode.screenHeight);

	// We just fake this as a dirty rect for now, to easily force a screen update whenever
	// the rect changes.
	addDirtyRect(_focusRect.left, _focusRect.top, _focusRect.width(), _focusRect.height(), _overlayVisible);
#endif
}

void SurfaceSdlGraphicsManager::clearFocusRectangle() {
#ifdef USE_SDL_DEBUG_FOCUSRECT
	// Only enable focus rectangle debug code, when the user wants it
	if (!_enableFocusRectDebugCode)
		return;

	_enableFocusRect = false;

	// We just fake this as a dirty rect for now, to easily force a screen update whenever
	// the rect changes.
	addDirtyRect(_focusRect.left, _focusRect.top, _focusRect.width(), _focusRect.height(), _overlayVisible);
#endif
}

#pragma mark -
#pragma mark --- Overlays ---
#pragma mark -

void SurfaceSdlGraphicsManager::clearOverlay() {
	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	if (!_overlayVisible)
		return;

	// Clear the overlay by making the game screen "look through" everywhere.
	SDL_Rect src, dst;
	src.x = src.y = 0;
	dst.x = dst.y = _maxExtraPixels;
	src.w = dst.w = _videoMode.screenWidth;
	src.h = dst.h = _videoMode.screenHeight;
	if (SDL_BlitSurface(_screen, &src, _tmpscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	SDL_LockSurface(_tmpscreen);
	SDL_LockSurface(_overlayscreen);

	// Transpose from game palette to RGB332 (overlay palette)
	if (_isHwPalette) {
		byte *p = (byte *)(_tmpscreen->pixels) + _maxExtraPixels * _tmpscreen->pitch + _maxExtraPixels * _tmpscreen->format->BytesPerPixel;
		int pitchSkip = _tmpscreen->pitch - _videoMode.screenWidth;
		for (int y = 0; y < _videoMode.screenHeight; y++) {
			for (int x = 0; x < _videoMode.screenWidth; x++, p++) {
				const SDL_Color &col = _currentPalette[*p];
				*p = (col.r & 0xe0) | ((col.g >> 3) & 0x1c) | ((col.b >> 6) & 0x03);
			}
			p += pitchSkip;
		}
	}

	_scaler->scale((byte *)(_tmpscreen->pixels) + _maxExtraPixels * _tmpscreen->pitch + _maxExtraPixels * _tmpscreen->format->BytesPerPixel, _tmpscreen->pitch,
	(byte *)_overlayscreen->pixels, _overlayscreen->pitch, _videoMode.screenWidth, _videoMode.screenHeight, 0, 0);

#ifdef USE_ASPECT
	if (_videoMode.aspectRatioCorrection)
		stretch200To240((uint8 *)_overlayscreen->pixels, _overlayscreen->pitch,
						_videoMode.overlayWidth, _videoMode.screenHeight * _videoMode.scaleFactor, 0, 0, 0,
						_videoMode.filtering, _overlayFormat);
#endif
	SDL_UnlockSurface(_tmpscreen);
	SDL_UnlockSurface(_overlayscreen);

	_forceRedraw = true;
}

void SurfaceSdlGraphicsManager::grabOverlay(Graphics::Surface &surface) const {
	assert(_transactionMode == kTransactionNone);

	if (_overlayscreen == nullptr)
		return;

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	assert(surface.w >= _videoMode.overlayWidth);
	assert(surface.h >= _videoMode.overlayHeight);
	assert(surface.format.bytesPerPixel == _overlayFormat.bytesPerPixel);

	byte *src = (byte *)_overlayscreen->pixels;
	byte *dst = (byte *)surface.getPixels();
	Graphics::copyBlit(dst, src, surface.pitch, _overlayscreen->pitch,
		_videoMode.overlayWidth, _videoMode.overlayHeight, _overlayFormat.bytesPerPixel);

	SDL_UnlockSurface(_overlayscreen);
}

void SurfaceSdlGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	assert(_transactionMode == kTransactionNone);

	if (_overlayscreen == nullptr)
		return;

	const byte *src = (const byte *)buf;
	uint bpp = _overlayscreen->format->BytesPerPixel;

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

	if (w > _videoMode.overlayWidth - x) {
		w = _videoMode.overlayWidth - x;
	}

	if (h > _videoMode.overlayHeight - y) {
		h = _videoMode.overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	// Mark the modified region as dirty
	addDirtyRect(x, y, w, h, true);

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_overlayscreen->pixels + y * _overlayscreen->pitch + x * bpp;
	do {
		memcpy(dst, src, w * bpp);
		dst += _overlayscreen->pitch;
		src += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}


#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

void SurfaceSdlGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keyColor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask, bool disableKeyColor) {

	if (mask && (!format || format->bytesPerPixel == 1)) {
		// 8-bit masked cursor, SurfaceSdl has no alpha mask support so we must convert this to color key
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
			warning("SurfaceSdlGraphicsManager::setMouseCursor: A mask was specified for an 8-bit cursor but the cursor couldn't be converted to color key");

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
	if (mask && format && format->bytesPerPixel > 1 && !_isHwPalette) {
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
		assert(format->bytesPerPixel == 1 || !_isHwPalette);

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
			SDL_FreeSurface(_mouseOrigSurface);

			if (_mouseSurface == _mouseOrigSurface) {
				_mouseSurface = nullptr;
			}

			_mouseOrigSurface = nullptr;
		}

		if (formatChanged && _mouseSurface) {
			SDL_FreeSurface(_mouseSurface);
			_mouseSurface = nullptr;
		}

		if (!w || !h) {
			return;
		}

		assert(!_mouseOrigSurface);

		// Allocate bigger surface because scalers will read past the boudaries.
		_mouseOrigSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
						_mouseCurState.w + _maxExtraPixels * 2,
						_mouseCurState.h + _maxExtraPixels * 2,
						_cursorFormat.bytesPerPixel * 8,
						((0xFF >> _cursorFormat.rLoss) << _cursorFormat.rShift),
						((0xFF >> _cursorFormat.gLoss) << _cursorFormat.gShift),
						((0xFF >> _cursorFormat.bLoss) << _cursorFormat.bShift),
						((0xFF >> _cursorFormat.aLoss) << _cursorFormat.aShift));

		if (_mouseOrigSurface == nullptr) {
			error("Allocating _mouseOrigSurface failed");
		}

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
		uint32 flags = _disableMouseKeyColor ? 0 : SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA;
		SDL_SetColorKey(_mouseOrigSurface, flags, _mouseKeyColor);
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
	Graphics::copyBlit((byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * _mouseOrigSurface->format->BytesPerPixel,
	                   (const byte *)buf, _mouseOrigSurface->pitch, w * _cursorFormat.bytesPerPixel, w, h, _cursorFormat.bytesPerPixel);

	SDL_UnlockSurface(_mouseOrigSurface);

	blitCursor();
}

void SurfaceSdlGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keyColor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	setMouseCursor(buf, w, h, hotspotX, hotspotY, keyColor, dontScale, format, mask, false);
}

void SurfaceSdlGraphicsManager::blitCursor() {
	const int w = _mouseCurState.w;
	const int h = _mouseCurState.h;

	if (!w || !h || !_mouseOrigSurface) {
		return;
	}

	_cursorNeedsRedraw = true;

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

#ifdef USE_ASPECT
	// store original to pass to aspect-correction function later
	const int rH1 = rH;
#endif

	if (!_cursorDontScale && _videoMode.aspectRatioCorrection) {
		rH = real2Aspect(rH - 1) + 1;
		_mouseCurState.rHotY = real2Aspect(_mouseCurState.rHotY);
	}

	bool sizeChanged = false;
	if (_mouseCurState.rW != rW || _mouseCurState.rH != rH) {
		_mouseCurState.rW = rW;
		_mouseCurState.rH = rH;
		sizeChanged = true;
	}

	if (sizeChanged || !_mouseSurface) {
		if (_mouseSurface)
			SDL_FreeSurface(_mouseSurface);

		_mouseSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
						_mouseCurState.rW,
						_mouseCurState.rH,
						_mouseOrigSurface->format->BitsPerPixel,
						_mouseOrigSurface->format->Rmask,
						_mouseOrigSurface->format->Gmask,
						_mouseOrigSurface->format->Bmask,
						_mouseOrigSurface->format->Amask);

		if (_mouseSurface == nullptr)
			error("Allocating _mouseSurface failed");
	}

	SDL_SetColors(_mouseSurface, _cursorPaletteDisabled ? _currentPalette : _cursorPalette, 0, 256);
	uint32 flags = _disableMouseKeyColor ? 0 : SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA;
	SDL_SetColorKey(_mouseSurface, flags, _mouseKeyColor);

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
			_mouseScaler->scale(
					(byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * _mouseOrigSurface->format->BytesPerPixel,
					_mouseOrigSurface->pitch, (byte *)_mouseSurface->pixels, _mouseSurface->pitch,
					_mouseCurState.w, _mouseCurState.h, 0, 0);
		} else
#endif
		{
			Graphics::scaleBlit((byte *)_mouseSurface->pixels, (const byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * _mouseOrigSurface->format->BytesPerPixel,
			                    _mouseSurface->pitch, _mouseOrigSurface->pitch,
				                _mouseCurState.w * _videoMode.scaleFactor, _mouseCurState.h * _videoMode.scaleFactor,
			                    _mouseCurState.w, _mouseCurState.h, convertSDLPixelFormat(_mouseSurface->format));

		}
	} else {
		Graphics::copyBlit((byte *)_mouseSurface->pixels, (const byte *)_mouseOrigSurface->pixels + _mouseOrigSurface->pitch * _maxExtraPixels + _maxExtraPixels * _mouseOrigSurface->format->BytesPerPixel,
		                   _mouseSurface->pitch, _mouseOrigSurface->pitch,
		                   _mouseCurState.w, _mouseCurState.h, _mouseSurface->format->BytesPerPixel);
	}

#ifdef USE_ASPECT
	if (!_cursorDontScale && _videoMode.aspectRatioCorrection)
		stretch200To240Nearest((uint8 *)_mouseSurface->pixels, _mouseSurface->pitch, rW, rH1, 0, 0, 0, convertSDLPixelFormat(_mouseSurface->format));
#endif

	SDL_UnlockSurface(_mouseSurface);
	SDL_UnlockSurface(_mouseOrigSurface);
}

void SurfaceSdlGraphicsManager::undrawMouse() {
	_mouseLastRect = _mouseNextRect;

	const Common::Point virtualCursor = convertWindowToVirtual(_cursorX, _cursorY);

	// The offsets must be applied, since the call to convertWindowToVirtual()
	// counteracts the move of the view port.

	_mouseNextRect.x = virtualCursor.x + _gameScreenShakeXOffset;
	_mouseNextRect.y = virtualCursor.y + _gameScreenShakeYOffset;

	if (!_overlayInGUI) {
		_mouseNextRect.w = _mouseCurState.vW;
		_mouseNextRect.h = _mouseCurState.vH;
		_mouseNextRect.x -= _mouseCurState.vHotX;
		_mouseNextRect.y -= _mouseCurState.vHotY;
	} else {
		_mouseNextRect.w = _mouseCurState.rW;
		_mouseNextRect.h = _mouseCurState.rH;
		_mouseNextRect.x -= _mouseCurState.rHotX;
		_mouseNextRect.y -= _mouseCurState.rHotY;
	}

	if (!_cursorVisible || !_mouseSurface) {
		_mouseNextRect.x = _mouseNextRect.y = _mouseNextRect.w = _mouseNextRect.h = 0;
	}

	// Add the area covered by the mouse cursor to the list of dirty rects if
	// we have to redraw the mouse, or if the cursor is alpha-blended since
	// alpha-blended cursors will happily blend into themselves if the surface
	// under the cursor is not reset first
	//
	// The mouse is undrawn using virtual coordinates, i.e. they may be
	// scaled and aspect-ratio corrected.

	if (_mouseLastRect.w != 0 && _mouseLastRect.h != 0)
		addDirtyRect(_mouseLastRect.x, _mouseLastRect.y, _mouseLastRect.w, _mouseLastRect.h, _overlayInGUI);

	if (_mouseNextRect.w != 0 && _mouseNextRect.h != 0)
		addDirtyRect(_mouseNextRect.x, _mouseNextRect.y, _mouseNextRect.w, _mouseNextRect.h, _overlayInGUI);
}

void SurfaceSdlGraphicsManager::drawMouse() {
	if (!_cursorVisible || !_mouseSurface || !_mouseCurState.w || !_mouseCurState.h) {
		return;
	}

	SDL_Rect dst;

	// We draw the pre-scaled cursor image, so now we need to adjust for
	// scaling, shake position and aspect ratio correction manually.

	dst.x = _mouseNextRect.x;
	dst.y = _mouseNextRect.y;
	dst.w = _mouseNextRect.w;
	dst.h = _mouseNextRect.h;

	if (_videoMode.aspectRatioCorrection && !_overlayInGUI)
		dst.y = real2Aspect(dst.y);

	if (!_overlayInGUI) {
		dst.x *= _videoMode.scaleFactor;
		dst.y *= _videoMode.scaleFactor;
	}
	dst.w = _mouseCurState.rW;
	dst.h = _mouseCurState.rH;

	// Note that SDL_BlitSurface() and addDirtyRect() will both perform any
	// clipping necessary

	if (SDL_BlitSurface(_mouseSurface, nullptr, _hwScreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());
}

#pragma mark -
#pragma mark --- On Screen Display ---
#pragma mark -

#ifdef USE_OSD
void SurfaceSdlGraphicsManager::displayMessageOnOSD(const Common::U32String &msg) {
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

	for (Common::U32String::const_iterator itr = msg.begin(); itr != msg.end(); itr++) {
		if (*itr == '\n') {
			lines.push_back(Common::U32String(strLineItrBegin, itr));
			strLineItrBegin = itr + 1;
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

	_osdMessageSurface = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		width, height, _hwScreen->format->BitsPerPixel, _hwScreen->format->Rmask, _hwScreen->format->Gmask, _hwScreen->format->Bmask, _hwScreen->format->Amask
	);

	// Lock the surface
	if (SDL_LockSurface(_osdMessageSurface))
		error("displayMessageOnOSD: SDL_LockSurface failed: %s", SDL_GetError());

	// Draw a dark gray rect
	// TODO: Rounded corners ? Border?
	SDL_FillRect(_osdMessageSurface, nullptr, SDL_MapRGB(_osdMessageSurface->format, 64, 64, 64));

	Graphics::Surface dst;
	dst.init(_osdMessageSurface->w, _osdMessageSurface->h, _osdMessageSurface->pitch, _osdMessageSurface->pixels,
		convertSDLPixelFormat(_osdMessageSurface->format));

	// Render the message, centered, and in white
	for (i = 0; i < lines.size(); i++) {
		font->drawString(&dst, lines[i],
			0, 0 + i * lineHeight + vOffset + lineSpacing, width,
			SDL_MapRGB(_osdMessageSurface->format, 255, 255, 255),
			Graphics::kTextAlignCenter, 0, true);
	}

	// Finished drawing, so unlock the OSD message surface
	SDL_UnlockSurface(_osdMessageSurface);

	// Init the OSD display parameters, and the fade out
	_osdMessageAlpha = SDL_ALPHA_TRANSPARENT + kOSDInitialAlpha * (SDL_ALPHA_OPAQUE - SDL_ALPHA_TRANSPARENT) / 100;
	_osdMessageFadeStartTime = SDL_GetTicks() + kOSDFadeOutDelay;
	// Enable alpha blending
	SDL_SetAlpha(_osdMessageSurface, SDL_RLEACCEL | SDL_SRCALPHA, _osdMessageAlpha);

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

SDL_Rect SurfaceSdlGraphicsManager::getOSDMessageRect() const {
	SDL_Rect rect;
	rect.x = (_hwScreen->w - _osdMessageSurface->w) / 2;
	rect.y = (_hwScreen->h - _osdMessageSurface->h) / 2;
	rect.w = _osdMessageSurface->w;
	rect.h = _osdMessageSurface->h;
	return rect;
}

void SurfaceSdlGraphicsManager::displayActivityIconOnOSD(const Graphics::Surface *icon) {
	assert(_transactionMode == kTransactionNone);

	Common::StackLock lock(_graphicsMutex);	// Lock the mutex until this function ends

	if (_osdIconSurface && !icon) {
		// Force a redraw to clear the icon on the next update
		_forceRedraw = true;
	}

	if (_osdIconSurface) {
		SDL_FreeSurface(_osdIconSurface);
		_osdIconSurface = nullptr;
	}

	if (icon) {
		const Graphics::PixelFormat &iconFormat = icon->format;

		_osdIconSurface = SDL_CreateRGBSurface(
				SDL_SWSURFACE,
				icon->w, icon->h, iconFormat.bytesPerPixel * 8,
				((0xFF >> iconFormat.rLoss) << iconFormat.rShift),
				((0xFF >> iconFormat.gLoss) << iconFormat.gShift),
				((0xFF >> iconFormat.bLoss) << iconFormat.bShift),
				((0xFF >> iconFormat.aLoss) << iconFormat.aShift)
		);

		// Lock the surface
		if (SDL_LockSurface(_osdIconSurface))
			error("displayActivityIconOnOSD: SDL_LockSurface failed: %s", SDL_GetError());

		byte *dst = (byte *) _osdIconSurface->pixels;
		const byte *src = (const byte *) icon->getPixels();
		for (int y = 0; y < icon->h; y++) {
			memcpy(dst, src, icon->w * iconFormat.bytesPerPixel);
			src += icon->pitch;
			dst += _osdIconSurface->pitch;
		}

		// Finished drawing, so unlock the OSD icon surface
		SDL_UnlockSurface(_osdIconSurface);
	}
}

SDL_Rect SurfaceSdlGraphicsManager::getOSDIconRect() const {
	SDL_Rect dstRect;
	dstRect.x = _hwScreen->w - _osdIconSurface->w - 10;
	dstRect.y = 10;
	dstRect.w = _osdIconSurface->w;
	dstRect.h = _osdIconSurface->h;
	return dstRect;
}

void SurfaceSdlGraphicsManager::removeOSDMessage() {
	// Remove the previous message
	if (_osdMessageSurface) {
		SDL_FreeSurface(_osdMessageSurface);
		_forceRedraw = true;
	}

	_osdMessageSurface = nullptr;
	_osdMessageAlpha = SDL_ALPHA_TRANSPARENT;

#if defined(MACOSX)
	macOSTouchbarUpdate(nullptr);
#endif
}

void SurfaceSdlGraphicsManager::updateOSD() {
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
			SDL_SetAlpha(_osdMessageSurface, SDL_RLEACCEL | SDL_SRCALPHA, _osdMessageAlpha);
		}

		if (_osdMessageAlpha == SDL_ALPHA_TRANSPARENT) {
			removeOSDMessage();
		}
	}

	if (_osdIconSurface || _osdMessageSurface) {
		// Redraw the area below the icon and message for the transparent blit to give correct results.
		_forceRedraw = true;
	}
}

void SurfaceSdlGraphicsManager::drawOSD() {
	if (_osdMessageSurface) {
		SDL_Rect dstRect = getOSDMessageRect();
		SDL_BlitSurface(_osdMessageSurface, nullptr, _hwScreen, &dstRect);
	}

	if (_osdIconSurface) {
		SDL_Rect dstRect = getOSDIconRect();
		SDL_BlitSurface(_osdIconSurface, nullptr, _hwScreen, &dstRect);
	}
}

#endif

void SurfaceSdlGraphicsManager::handleResizeImpl(const int width, const int height) {
	SdlGraphicsManager::handleResizeImpl(width, height);
	recalculateDisplayAreas();
}

void SurfaceSdlGraphicsManager::handleScalerHotkeys(uint mode, int factor) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	bool sizeChanged = _videoMode.scaleFactor != factor;
#endif

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

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (sizeChanged) {
		// Forcibly resizing the window here since a user switching scaler
		// size will not normally cause the window to update
		_window->createOrUpdateWindow(_hwScreen->w, _hwScreen->h, _lastFlags);
	}
#endif

	internUpdateScreen();
}

bool SurfaceSdlGraphicsManager::notifyEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_START) {
		return SdlGraphicsManager::notifyEvent(event);
	}

	switch ((CustomEventAction) event.customType) {
	case kActionToggleAspectRatioCorrection: {
		beginGFXTransaction();
			setFeatureState(OSystem::kFeatureAspectRatioCorrection, !_videoMode.aspectRatioCorrection);
		endGFXTransaction();
#ifdef USE_OSD
		Common::U32String message;
		if (_videoMode.aspectRatioCorrection)
			message = Common::U32String::format("%S\n%d x %d -> %d x %d",
			                                    _("Enabled aspect ratio correction").c_str(),
			                                    _videoMode.screenWidth, _videoMode.screenHeight,
			                                    _hwScreen->w, _hwScreen->h
			          );
		else
			message = Common::U32String::format("%S\n%d x %d -> %d x %d",
			                                    _("Disabled aspect ratio correction").c_str(),
			                                    _videoMode.screenWidth, _videoMode.screenHeight,
			                                    _hwScreen->w, _hwScreen->h
			          );
		displayMessageOnOSD(message);
#endif
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

#if SDL_VERSION_ATLEAST(2, 0, 0)
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
#endif

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

void SurfaceSdlGraphicsManager::notifyVideoExpose() {
	_forceRedraw = true;
}

void SurfaceSdlGraphicsManager::notifyResize(const int width, const int height) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	handleResize(width, height);
#endif
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
void SurfaceSdlGraphicsManager::deinitializeRenderer() {
#if defined(USE_IMGUI) && defined(USE_IMGUI_SDLRENDERER2)
	destroyImGui();
#endif

	if (_screenTexture)
		SDL_DestroyTexture(_screenTexture);
	_screenTexture = nullptr;

	if (_renderer)
		SDL_DestroyRenderer(_renderer);
	_renderer = nullptr;
}

void SurfaceSdlGraphicsManager::recreateScreenTexture() {
	if (!_renderer)
		return;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, _videoMode.filtering ? "linear" : "nearest");

	SDL_Texture *oldTexture = _screenTexture;
	_screenTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	if (_screenTexture)
		SDL_DestroyTexture(oldTexture);
	else
		_screenTexture = oldTexture;
}

SDL_Surface *SurfaceSdlGraphicsManager::SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {
	deinitializeRenderer();

	uint32 createWindowFlags = SDL_WINDOW_RESIZABLE;
	uint32 rendererFlags = 0;
	if ((flags & SDL_FULLSCREEN) != 0) {
		createWindowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	if (!createOrUpdateWindow(width, height, createWindowFlags)) {
		return nullptr;
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
			deinitializeRenderer();
			return nullptr;
		}
	}

	getWindowSizeFromSdl(&_windowWidth, &_windowHeight);
	handleResize(_windowWidth, _windowHeight);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, _videoMode.filtering ? "linear" : "nearest");

	Uint32 format = SDL_PIXELFORMAT_RGB565;

	_screenTexture = SDL_CreateTexture(_renderer, format, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!_screenTexture) {
		deinitializeRenderer();
		return nullptr;
	}

	SDL_Surface *screen = SDL_CreateRGBSurfaceWithFormat(0, width, height, SDL_BITSPERPIXEL(format), format);
	if (!screen) {
		deinitializeRenderer();
		return nullptr;
	}

#if defined(USE_IMGUI) && defined(USE_IMGUI_SDLRENDERER2)
	// Setup Dear ImGui
	initImGui(_renderer, nullptr);
#endif

	return screen;
}

void SurfaceSdlGraphicsManager::SDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects) {
	SDL_UpdateTexture(_screenTexture, nullptr, screen->pixels, screen->pitch);

	SDL_Rect viewport;

	Common::Rect &drawRect = (_overlayVisible) ? _overlayDrawRect : _gameDrawRect;
	viewport.x = drawRect.left;
	viewport.y = drawRect.top;

	int rotation = getRotationMode();
	int rotangle = 0;
	if (rotation == Common::kRotation90 || rotation == Common::kRotation270) {
		int delta = (drawRect.width() - drawRect.height()) / 2;
		viewport.x = drawRect.top - delta;
		viewport.y = drawRect.left + delta;
	}
	rotangle = rotation;

	viewport.w = drawRect.width();
	viewport.h = drawRect.height();

	SDL_RenderClear(_renderer);

	if (rotangle != 0)
		SDL_RenderCopyEx(_renderer, _screenTexture, nullptr, &viewport, rotangle, nullptr, SDL_FLIP_NONE);
	else
		SDL_RenderCopy(_renderer, _screenTexture, nullptr, &viewport);
}

int SurfaceSdlGraphicsManager::SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors) {
	if (surface->format->palette) {
		return !SDL_SetPaletteColors(surface->format->palette, colors, firstcolor, ncolors) ? 1 : 0;
	} else {
		return 0;
	}
}

int SurfaceSdlGraphicsManager::SDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha) {
	if (SDL_SetSurfaceAlphaMod(surface, alpha)) {
		return -1;
	}

	if (alpha == 255 || !flag) {
		if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE)) {
			return -1;
		}
	} else {
		if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND)) {
			return -1;
		}
	}

	return 0;
}

int SurfaceSdlGraphicsManager::SDL_SetColorKey(SDL_Surface *surface, Uint32 flag, Uint32 key) {
	return ::SDL_SetColorKey(surface, flag ? SDL_TRUE : SDL_FALSE, key) ? -1 : 0;
}

#if defined(USE_IMGUI) && defined(USE_IMGUI_SDLRENDERER2)
void *SurfaceSdlGraphicsManager::getImGuiTexture(const Graphics::Surface &image, const byte *palette, int palCount) {

	// Upload pixels into texture
	SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, image.w, image.h);
	if (texture == nullptr) {
		error("getImGuiTexture: errror creating tetxure: %s", SDL_GetError());
		return nullptr;
	}

	Graphics::Surface *s = image.convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), palette, palCount);
	SDL_UpdateTexture(texture, nullptr, s->getPixels(), s->pitch);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);

	s->free();
	delete s;

	return (void *)texture;
}

void SurfaceSdlGraphicsManager::freeImGuiTexture(void *texture) {
	SDL_DestroyTexture((SDL_Texture *) texture);
}
#endif // defined(USE_IMGUI) && defined(USE_IMGUI_SDLRENDERER2)

#endif // SDL_VERSION_ATLEAST(2, 0, 0)

#endif
