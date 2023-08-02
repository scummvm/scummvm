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

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE // atari-graphics.h's unordered_set

#include "backends/graphics/atari/atari-graphics.h"

#include <mint/cookie.h>
#include <mint/falcon.h>
#include <mint/osbind.h>
#include <mint/sysvars.h>

#include "backends/graphics/atari/atari-graphics-superblitter.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "common/config-manager.h"
#include "common/str.h"
#include "common/textconsole.h"	// for warning() & error()
#include "common/translation.h"
#include "engines/engine.h"
#include "graphics/blit.h"
#include "gui/ThemeEngine.h"

#define SCREEN_ACTIVE

#define MAX_HZ_SHAKE 16 // Falcon only
#define MAX_V_SHAKE  16

bool g_unalignedPitch = false;

static const Graphics::PixelFormat PIXELFORMAT_CLUT8 = Graphics::PixelFormat::createFormatCLUT8();
static const Graphics::PixelFormat PIXELFORMAT_RGB332 = Graphics::PixelFormat(1, 3, 3, 2, 0, 5, 2, 0, 0);
static const Graphics::PixelFormat PIXELFORMAT_RGB121 = Graphics::PixelFormat(1, 1, 2, 1, 0, 3, 1, 0, 0);

static bool s_tt;
static int s_shakeXOffset;
static int s_shakeYOffset;

static Graphics::Surface *s_screenSurf;
static void VblHandler() {
	if (s_screenSurf) {
#ifdef SCREEN_ACTIVE
		const int bitsPerPixel = (s_screenSurf->format == PIXELFORMAT_RGB121 ? 4 : 8);
		uintptr p = (uintptr)s_screenSurf->getBasePtr(0, MAX_V_SHAKE + s_shakeYOffset);

		if (!s_tt) {
			s_shakeXOffset = -s_shakeXOffset;

			if (s_shakeXOffset >= 0) {
				p += MAX_HZ_SHAKE;
				*((volatile char *)0xFFFF8265) = s_shakeXOffset;
			} else {
				*((volatile char *)0xFFFF8265) = MAX_HZ_SHAKE + s_shakeXOffset;
			}

			// subtract 4 or 8 words if scrolling
			*((volatile short *)0xFFFF820E) = s_shakeXOffset == 0
			   ? (2 * MAX_HZ_SHAKE * bitsPerPixel / 8) / 2
			   : (2 * MAX_HZ_SHAKE * bitsPerPixel / 8) / 2 - bitsPerPixel;
		}


		union { byte c[4]; uintptr p; } sptr;
		sptr.p = p;

		*((volatile byte *)0xFFFF8201) = sptr.c[1];
		*((volatile byte *)0xFFFF8203) = sptr.c[2];
		*((volatile byte *)0xFFFF820D) = sptr.c[3];
#endif
		s_screenSurf = nullptr;
	}
}

static uint32 InstallVblHandler() {
	uint32 installed = 0;
	*vblsem = 0;  // lock vbl

	for (int i = 0; i < *nvbls; ++i) {
		if (!(*_vblqueue)[i]) {
			(*_vblqueue)[i] = VblHandler;
			installed = 1;
			break;
		}
	}

	*vblsem = 1;  // unlock vbl
	return installed;
}

static uint32 UninstallVblHandler() {
	uint32 uninstalled = 0;
	*vblsem = 0;  // lock vbl

	for (int i = 0; i < *nvbls; ++i) {
		if ((*_vblqueue)[i] == VblHandler) {
			(*_vblqueue)[i] = NULL;
			uninstalled = 1;
			break;
		}
	}

	*vblsem = 1;  // unlock vbl
	return uninstalled;
}

static int  s_oldRez = -1;
static int  s_oldMode = -1;
static void *s_oldPhysbase = nullptr;

void AtariGraphicsShutdown() {
	Supexec(UninstallVblHandler);

	if (s_oldRez != -1) {
		Setscreen(SCR_NOCHANGE, s_oldPhysbase, s_oldRez);
	} else if (s_oldMode != -1) {
		// prevent setting video base address just on the VDB line
		Vsync();
		VsetMode(s_oldMode);
		VsetScreen(SCR_NOCHANGE, s_oldPhysbase, SCR_NOCHANGE, SCR_NOCHANGE);
	}
}

AtariGraphicsManager::AtariGraphicsManager() {
	debug("AtariGraphicsManager()");

	enum {
		VDO_NO_ATARI_HW = 0xffff,
		VDO_ST = 0,
		VDO_STE,
		VDO_TT,
		VDO_FALCON,
		VDO_MILAN
	};

	long vdo = VDO_NO_ATARI_HW<<16;
	Getcookie(C__VDO, &vdo);
	vdo >>= 16;

	_tt = (vdo == VDO_TT);
	s_tt = _tt;

	if (!_tt)
		_vgaMonitor = VgetMonitor() == MON_VGA;

	// no BDF scaling please
	ConfMan.registerDefault("gui_disable_fixed_font_scaling", true);

	// make the standard GUI renderer default (!DISABLE_FANCY_THEMES implies anti-aliased rendering in ThemeEngine.cpp)
	// (and without DISABLE_FANCY_THEMES we can't use 640x480 themes)
	const char *standardThemeEngineName = GUI::ThemeEngine::findModeConfigName(GUI::ThemeEngine::kGfxStandard);
	if (!ConfMan.hasKey("gui_renderer"))
		ConfMan.set("gui_renderer", standardThemeEngineName);

	// make the built-in theme default to avoid long loading times
	if (!ConfMan.hasKey("gui_theme"))
		ConfMan.set("gui_theme", "builtin");

#ifndef DISABLE_FANCY_THEMES
	// make "themes" the default theme path
	if (!ConfMan.hasKey("themepath"))
		ConfMan.set("themepath", "themes");
#endif

	ConfMan.flushToDisk();

	// Generate RGB332/RGB121 palette for the overlay
	const Graphics::PixelFormat &format = getOverlayFormat();
	const int paletteSize = getOverlayPaletteSize();
	for (int i = 0; i < paletteSize; i++) {
		if (_tt) {
			// Bits 15-12    Bits 11-8     Bits 7-4      Bits 3-0
			// Reserved      Red           Green         Blue
			_overlayPalette.tt[i] =  ((i >> format.rShift) & format.rMax()) << (8 + (format.rLoss - 4));
			_overlayPalette.tt[i] |= ((i >> format.gShift) & format.gMax()) << (4 + (format.gLoss - 4));
			_overlayPalette.tt[i] |= ((i >> format.bShift) & format.bMax()) << (0 + (format.bLoss - 4));
		} else {
			_overlayPalette.falcon[i].red    = ((i >> format.rShift) & format.rMax()) << format.rLoss;
			_overlayPalette.falcon[i].green |= ((i >> format.gShift) & format.gMax()) << format.gLoss;
			_overlayPalette.falcon[i].blue  |= ((i >> format.bShift) & format.bMax()) << format.bLoss;
		}
	}

	// although we store/restore video hardware in OSystem_Atari,
	// make sure that internal OS structures are updated correctly, too
	if (_tt) {
		s_oldRez = Getrez();
	} else {
		s_oldMode = VsetMode(VM_INQUIRE);
	}
	s_oldPhysbase = Physbase();

	if (!Supexec(InstallVblHandler)) {
		error("VBL handler was not installed");
	}

	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

AtariGraphicsManager::~AtariGraphicsManager() {
	debug("~AtariGraphicsManager()");

	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	AtariGraphicsShutdown();
}

bool AtariGraphicsManager::hasFeature(OSystem::Feature f) const {
	switch (f) {
	case OSystem::Feature::kFeatureAspectRatioCorrection:
		//debug("hasFeature(kFeatureAspectRatioCorrection): %d", !_vgaMonitor);
		return !_tt && !_vgaMonitor;
	case OSystem::Feature::kFeatureCursorPalette:
		// FIXME: pretend to have cursor palette at all times, this function
		// can get (and it is) called any time, before and after showOverlay()
		// (overlay cursor uses the cross if kFeatureCursorPalette returns false
		// here too soon)
		//debug("hasFeature(kFeatureCursorPalette): %d", isOverlayVisible());
		//return isOverlayVisible();
		return true;
	default:
		return false;
	}
}

void AtariGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::Feature::kFeatureAspectRatioCorrection:
		//debug("setFeatureState(kFeatureAspectRatioCorrection): %d", enable);
		_oldAspectRatioCorrection = _aspectRatioCorrection;
		_aspectRatioCorrection = enable;
		break;
	default:
		[[fallthrough]];
	}
}

bool AtariGraphicsManager::getFeatureState(OSystem::Feature f) const {
	switch (f) {
	case OSystem::Feature::kFeatureAspectRatioCorrection:
		//debug("getFeatureState(kFeatureAspectRatioCorrection): %d", _aspectRatioCorrection);
		return _aspectRatioCorrection;
	case OSystem::Feature::kFeatureCursorPalette:
		//debug("getFeatureState(kFeatureCursorPalette): %d", isOverlayVisible());
		//return isOverlayVisible();
		return true;
	default:
		return false;
	}
}

bool AtariGraphicsManager::setGraphicsMode(int mode, uint flags) {
	debug("setGraphicsMode: %d, %d", mode, flags);

	GraphicsMode graphicsMode = (GraphicsMode)mode;

	if (graphicsMode >= GraphicsMode::DirectRendering && graphicsMode <= GraphicsMode::TripleBuffering) {
		_pendingState.mode = graphicsMode;
		return true;
	}

	return false;
}

void AtariGraphicsManager::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	debug("initSize: %d, %d, %d", width, height, format ? format->bytesPerPixel : 1);

	_pendingState.width = width;
	_pendingState.height = height;
	_pendingState.format = format ? *format : PIXELFORMAT_CLUT8;
}

void AtariGraphicsManager::beginGFXTransaction() {
	debug("beginGFXTransaction");
}

OSystem::TransactionError AtariGraphicsManager::endGFXTransaction() {
	debug("endGFXTransaction");

	int error = OSystem::TransactionError::kTransactionSuccess;

	if (_pendingState.format != PIXELFORMAT_CLUT8)
		error |= OSystem::TransactionError::kTransactionFormatNotSupported;

	if (_pendingState.width > getMaximumScreenWidth() || _pendingState.height > getMaximumScreenHeight())
		error |= OSystem::TransactionError::kTransactionSizeChangeFailed;

	if (_pendingState.width % 16 != 0 && !hasSuperVidel()) {
		warning("Requested width not divisible by 16, please report");
		error |= OSystem::TransactionError::kTransactionSizeChangeFailed;
	}

	if (error != OSystem::TransactionError::kTransactionSuccess) {
		warning("endGFXTransaction failed: %02x", (int)error);
		// all our errors are fatal but engine.cpp takes only this one seriously
		error |= OSystem::TransactionError::kTransactionSizeChangeFailed;
		return static_cast<OSystem::TransactionError>(error);
	}

	_chunkySurface.init(_pendingState.width, _pendingState.height, _pendingState.width,
		_chunkySurface.getPixels(), _pendingState.format);

	_screen[FRONT_BUFFER]->reset(_pendingState.width, _pendingState.height, 8);
	_screen[BACK_BUFFER1]->reset(_pendingState.width, _pendingState.height, 8);
	_screen[BACK_BUFFER2]->reset(_pendingState.width, _pendingState.height, 8);
	_workScreen = _screen[_pendingState.mode <= GraphicsMode::SingleBuffering ? FRONT_BUFFER : BACK_BUFFER1];

	s_screenSurf = nullptr;
	s_shakeXOffset = 0;
	s_shakeYOffset = 0;

	// in case of resolution change from GUI
	if (_oldWorkScreen)
		_oldWorkScreen = _workScreen;

	_palette.clear();
	_pendingScreenChange = kPendingScreenChangeMode | kPendingScreenChangeScreen | kPendingScreenChangePalette;

	static bool firstRun = true;
	if (firstRun) {
		_cursor.setPosition(getOverlayWidth() / 2, getOverlayHeight() / 2);
		_cursor.swap();
		firstRun = false;
	}

	warpMouse(_pendingState.width / 2, _pendingState.height / 2);

	_currentState = _pendingState;

	return OSystem::kTransactionSuccess;
}

void AtariGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	//debug("setPalette: %d, %d", start, num);

	if (_tt) {
		uint16 *pal = &_palette.tt[start];
		for (uint i = 0; i < num; ++i) {
			// Bits 15-12    Bits 11-8     Bits 7-4      Bits 3-0
			// Reserved      Red           Green         Blue
			pal[i]  = ((colors[i * 3 + 0] >> 4) & 0x0f) << 8;
			pal[i] |= ((colors[i * 3 + 1] >> 4) & 0x0f) << 4;
			pal[i] |= ((colors[i * 3 + 2] >> 4) & 0x0f);
		}
	} else {
		_RGB *pal = &_palette.falcon[start];
		for (uint i = 0; i < num; ++i) {
			pal[i].red   = colors[i * 3 + 0];
			pal[i].green = colors[i * 3 + 1];
			pal[i].blue  = colors[i * 3 + 2];
		}
	}

	_pendingScreenChange |= kPendingScreenChangePalette;
}

void AtariGraphicsManager::grabPalette(byte *colors, uint start, uint num) const {
	//debug("grabPalette: %d, %d", start, num);

	if (_tt) {
		const uint16 *pal = &_palette.tt[start];
		for (uint i = 0; i < num; ++i) {
			// Bits 15-12    Bits 11-8     Bits 7-4      Bits 3-0
			// Reserved      Red           Green         Blue
			*colors++ = ((pal[i] >> 8) & 0x0f) << 4;
			*colors++ = ((pal[i] >> 4) & 0x0f) << 4;
			*colors++ = ((pal[i]     ) & 0x0f) << 4;
		}
	} else {
		const _RGB *pal = &_palette.falcon[start];
		for (uint i = 0; i < num; ++i) {
			*colors++ = pal[i].red;
			*colors++ = pal[i].green;
			*colors++ = pal[i].blue;
		}
	}
}

void AtariGraphicsManager::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	//debug("copyRectToScreen: %d, %d, %d(%d), %d", x, y, w, pitch, h);

	copyRectToScreenInternal(buf, pitch, x, y, w, h,
		PIXELFORMAT_CLUT8,
		_currentState.mode == GraphicsMode::DirectRendering,
		_currentState.mode == GraphicsMode::TripleBuffering);
}

// this is not really locking anything but it's an useful function
// to return current rendering surface :)
Graphics::Surface *AtariGraphicsManager::lockScreen() {
	//debug("lockScreen");

	if (isOverlayVisible() && !isOverlayDirectRendering())
		return &_overlaySurface;
	else if ((isOverlayVisible() && isOverlayDirectRendering()) || _currentState.mode == GraphicsMode::DirectRendering)
		return _workScreen->offsettedSurf;
	else
		return &_chunkySurface;
}

void AtariGraphicsManager::unlockScreen() {
	//debug("unlockScreen: %d x %d", _workScreen->surf.w, _workScreen->surf.h);

	const Graphics::Surface &dstSurface = *lockScreen();

	const bool directRendering = (dstSurface.getPixels() != _chunkySurface.getPixels());
	const Common::Rect rect = alignRect(0, 0, dstSurface.w, dstSurface.h);
	_workScreen->addDirtyRect(dstSurface, rect, directRendering);

	if (_currentState.mode == GraphicsMode::TripleBuffering) {
		_screen[BACK_BUFFER2]->addDirtyRect(dstSurface, rect, directRendering);
		_screen[FRONT_BUFFER]->addDirtyRect(dstSurface, rect, directRendering);
	}

	// doc says:
	// Unlock the screen framebuffer, and mark it as dirty, i.e. during the
	// next updateScreen() call, the whole screen will be updated.
	//
	// ... so no updateScreen() from here (otherwise Eco Quest's intro is crawling!)
}

void AtariGraphicsManager::fillScreen(uint32 col) {
	debug("fillScreen: %d", col);

	Graphics::Surface *screen = lockScreen();
	screen->fillRect(Common::Rect(screen->w, screen->h), col);
	unlockScreen();
}

void AtariGraphicsManager::fillScreen(const Common::Rect &r, uint32 col) {
	Graphics::Surface *screen = lockScreen();
	if (screen)
		screen->fillRect(r, col);
	unlockScreen();
}

void AtariGraphicsManager::updateScreen() {
	//debug("updateScreen");

	// avoid falling into the debugger (screen may not not initialized yet)
	Common::setErrorHandler(nullptr);

	if (_checkUnalignedPitch) {
		const Common::ConfigManager::Domain *activeDomain = ConfMan.getActiveDomain();
		if (activeDomain) {
			// FIXME: Some engines are too bound to linear surfaces that it is very
			// hard to repair them. So instead of polluting the engine with
			// Surface::init() & delete[] Surface::getPixels() just use this hack.
			const Common::String engineId = activeDomain->getValOrDefault("engineid");
			const Common::String gameId = activeDomain->getValOrDefault("gameid");
			if (engineId == "hypno"
				|| engineId == "mohawk"
				|| engineId == "parallaction"
				|| engineId == "private"
				|| (engineId == "sci"
					&& (gameId == "phantasmagoria" || gameId == "shivers"))
				|| engineId == "sherlock"
				|| engineId == "teenagent"
				|| engineId == "tsage") {
				g_unalignedPitch = true;
			}
		}

		_checkUnalignedPitch = false;
	}

	// updates outOfScreen OR srcRect/dstRect (only if visible/needed)
	_cursor.update(*lockScreen(), _workScreen->cursorPositionChanged || _workScreen->cursorSurfaceChanged);

	bool screenUpdated = false;

	if (isOverlayVisible()) {
		assert(_workScreen == _screen[OVERLAY_BUFFER]);
		if (isOverlayDirectRendering())
			screenUpdated = updateScreenInternal<true>(Graphics::Surface());
		else
			screenUpdated = updateScreenInternal<false>(_overlaySurface);
	} else {
		switch (_currentState.mode) {
		case GraphicsMode::DirectRendering:
			assert(_workScreen == _screen[FRONT_BUFFER]);
			screenUpdated = updateScreenInternal<true>(Graphics::Surface());
			break;
		case GraphicsMode::SingleBuffering:
			assert(_workScreen == _screen[FRONT_BUFFER]);
			screenUpdated = updateScreenInternal<false>(_chunkySurface);
			break;
		case GraphicsMode::TripleBuffering:
			assert(_workScreen == _screen[BACK_BUFFER1]);
			screenUpdated = updateScreenInternal<false>(_chunkySurface);
			break;
		}
	}

	_workScreen->clearDirtyRects();

#ifdef SCREEN_ACTIVE
	// first change video mode so we can modify video regs later
	if (_pendingScreenChange & kPendingScreenChangeMode) {
		if (_workScreen->rez != -1) {
			// unfortunately this reinitializes VDI, too
			Setscreen(SCR_NOCHANGE, SCR_NOCHANGE, _workScreen->rez);
		} else if (_workScreen->mode != -1) {
			VsetMode(_workScreen->mode);
		}
	}

	if (_pendingScreenChange & kPendingScreenChangeScreen) {
		// calling (V)SetScreen without Vsync() is dangerous (at least on Falcon)
		s_screenSurf = isOverlayVisible() ? &_screen[OVERLAY_BUFFER]->surf : &_screen[FRONT_BUFFER]->surf;
	} else if (screenUpdated && !isOverlayVisible() && _currentState.mode == GraphicsMode::TripleBuffering) {
		// Triple buffer:
		// - alternate BACK_BUFFER1 and BACK_BUFFER2
		// - check if FRONT_BUFFER has been displayed for at least one frame
		// - display the most recent buffer (BACK_BUFFER2 in our case)
		// - alternate BACK_BUFFER2 and FRONT_BUFFER (only if BACK_BUFFER2
		//   has been updated)

		set_sysvar_to_short(vblsem, 0);  // lock vbl

		static long old_vbclock = get_sysvar(_vbclock);
		long curr_vbclock = get_sysvar(_vbclock);

		if (old_vbclock != curr_vbclock) {
			// at least one vbl has passed since setting new video base
			// guard BACK_BUFFER2 from overwriting while presented
			Screen *tmp = _screen[BACK_BUFFER2];
			_screen[BACK_BUFFER2] = _screen[FRONT_BUFFER];
			_screen[FRONT_BUFFER] = tmp;

			old_vbclock = curr_vbclock;
		}

		// swap back buffers
		Screen *tmp = _screen[BACK_BUFFER1];
		_screen[BACK_BUFFER1] = _screen[BACK_BUFFER2];
		_screen[BACK_BUFFER2] = tmp;

		// queue BACK_BUFFER2 with the most recent frame content
		s_screenSurf = &_screen[BACK_BUFFER2]->surf;

		set_sysvar_to_short(vblsem, 1);  // unlock vbl

		_workScreen = _screen[BACK_BUFFER1];
		// BACK_BUFFER2: now contains finished frame
		// FRONT_BUFFER is displayed and still contains previously finished frame
	}

	if (_pendingScreenChange & kPendingScreenChangePalette) {
		if (_tt)
			EsetPalette(0, isOverlayVisible() ? getOverlayPaletteSize() : 256, _workScreen->palette->tt);
		else
			VsetRGB(0, isOverlayVisible() ? getOverlayPaletteSize() : 256, _workScreen->palette->falcon);
	}

	_pendingScreenChange = kPendingScreenChangeNone;

	if (_oldAspectRatioCorrection != _aspectRatioCorrection) {
		if (!isOverlayVisible()) {
			if (!_vgaMonitor) {
				short mode = VsetMode(VM_INQUIRE);
				if (_aspectRatioCorrection) {
					// 60 Hz
					mode &= ~PAL;
					mode |= NTSC;
				} else {
					// 50 Hz
					mode &= ~NTSC;
					mode |= PAL;
				}
				VsetMode(mode);
			} else if (hasSuperVidel()) {
				// TODO: reduce to 200 scan lines?
			} else if (!_tt) {
				// TODO: increase vertical frequency?
			} else {
				// TODO: some tricks with TT's 480 lines?
			}

			_oldAspectRatioCorrection = _aspectRatioCorrection;
		} else {
			// ignore new value in overlay
			_aspectRatioCorrection = _oldAspectRatioCorrection;
		}
	}
#endif
	//debug("end of updateScreen");
}

void AtariGraphicsManager::setShakePos(int shakeXOffset, int shakeYOffset) {
	//debug("setShakePos: %d, %d", shakeXOffset, shakeYOffset);

	if (_tt) {
		// as TT can't horizontally shake anything, do it at least vertically
		s_shakeYOffset = (shakeYOffset == 0 && shakeXOffset != 0) ? shakeXOffset : shakeYOffset;
	} else {
		s_shakeXOffset = shakeXOffset;
		s_shakeYOffset = shakeYOffset;
	}

	_pendingScreenChange |= kPendingScreenChangeScreen;
	updateScreen();
}

void AtariGraphicsManager::showOverlay(bool inGUI) {
	debug("showOverlay");

	if (_overlayVisible)
		return;

	if (_currentState.mode == GraphicsMode::DirectRendering) {
		// make sure that _oldCursorRect is used to restore the original game graphics
		// (but only if resolution hasn't changed, see endGFXTransaction())
		bool wasVisible = showMouse(false);

		// revert back but don't update screen
		_cursor.visible = wasVisible;
	}

	_cursor.swap();
	_oldWorkScreen = _workScreen;
	_workScreen = _screen[OVERLAY_BUFFER];

	// do not cache dirtyRects and oldCursorRect
	const int bitsPerPixel = getBitsPerPixel(getOverlayFormat());
	_workScreen->reset(getOverlayWidth(), getOverlayHeight(), bitsPerPixel);

	_pendingScreenChange = kPendingScreenChangeMode | kPendingScreenChangeScreen | kPendingScreenChangePalette;

	_overlayVisible = true;

	updateScreen();
}

void AtariGraphicsManager::hideOverlay() {
	debug("hideOverlay");

	if (!_overlayVisible)
		return;

	_workScreen = _oldWorkScreen;
	_oldWorkScreen = nullptr;
	_cursor.swap();

	// FIXME: perhaps there's a better way but this will do for now
	_checkUnalignedPitch = true;

	_pendingScreenChange = kPendingScreenChangeMode | kPendingScreenChangeScreen | kPendingScreenChangePalette;

	_overlayVisible = false;

	updateScreen();
}

Graphics::PixelFormat AtariGraphicsManager::getOverlayFormat() const {
#ifndef DISABLE_FANCY_THEMES
	return _tt ? PIXELFORMAT_RGB121 : PIXELFORMAT_RGB332;
#else
	return PIXELFORMAT_RGB121;
#endif
}

void AtariGraphicsManager::clearOverlay() {
	if (isOverlayDirectRendering())
		return;

	debug("clearOverlay");

	if (!_overlayVisible)
		return;

	const Graphics::Surface &sourceSurface =
		_currentState.mode == GraphicsMode::DirectRendering ? *_screen[FRONT_BUFFER]->offsettedSurf : _chunkySurface;

	const bool upscale = _overlaySurface.w / sourceSurface.w >= 2 && _overlaySurface.h / sourceSurface.h >= 2;

	const int w = upscale ? sourceSurface.w * 2 : sourceSurface.w;
	const int h = upscale ? sourceSurface.h * 2 : sourceSurface.h;

	const int hzOffset = (_overlaySurface.w - w) / 2;
	const int vOffset  = (_overlaySurface.h - h) / 2;

	const int srcPadding = sourceSurface.pitch - sourceSurface.w;
	const int dstPadding = hzOffset * 2 + (upscale ? _overlaySurface.pitch : 0);

	// Transpose from game palette to RGB332/RGB121 (overlay palette)
	const byte *src = (const byte*)sourceSurface.getPixels();
	byte *dst = (byte *)_overlaySurface.getBasePtr(hzOffset, vOffset);

	// for TT: 8/4/0 + (xLoss - 4) + xShift
	static const int rShift = (_tt ? (8 - 4) : 0)
		+ _overlaySurface.format.rLoss - _overlaySurface.format.rShift;
	static const int gShift = (_tt ? (4 - 4) : 0)
		+ _overlaySurface.format.gLoss - _overlaySurface.format.gShift;
	static const int bShift = (_tt ? (0 - 4) : 0)
		+ _overlaySurface.format.bLoss - _overlaySurface.format.bShift;

	static const int rMask = _overlaySurface.format.rMax() << _overlaySurface.format.rShift;
	static const int gMask = _overlaySurface.format.gMax() << _overlaySurface.format.gShift;
	static const int bMask = _overlaySurface.format.bMax() << _overlaySurface.format.bShift;

	for (int y = 0; y < sourceSurface.h; y++) {
		for (int x = 0; x < sourceSurface.w; x++) {
			byte pixel;

			if (_tt) {
				// Bits 15-12    Bits 11-8     Bits 7-4      Bits 3-0
				// Reserved      Red           Green         Blue
				const uint16 &col = _palette.tt[*src++];
				pixel = ((col >> rShift) & rMask)
					  | ((col >> gShift) & gMask)
					  | ((col >> bShift) & bMask);
			} else {
				const _RGB &col = _palette.falcon[*src++];
				pixel = ((col.red   >> rShift) & rMask)
					  | ((col.green >> gShift) & gMask)
					  | ((col.blue  >> bShift) & bMask);
			}

			if (upscale) {
				*(dst + _overlaySurface.pitch) = pixel;
				*dst++ = pixel;
				*(dst + _overlaySurface.pitch) = pixel;
			}
			*dst++ = pixel;
		}

		src += srcPadding;
		dst += dstPadding;
	}

	// top rect
	memset(_overlaySurface.getBasePtr(0, 0), 0, vOffset * _overlaySurface.pitch);
	// bottom rect
	memset(_overlaySurface.getBasePtr(0, _overlaySurface.h - vOffset), 0, vOffset * _overlaySurface.pitch);
	// left rect
	_overlaySurface.fillRect(Common::Rect(0, vOffset, hzOffset, _overlaySurface.h - vOffset), 0);
	// right rect
	_overlaySurface.fillRect(Common::Rect(_overlaySurface.w - hzOffset, vOffset, _overlaySurface.w, _overlaySurface.h - vOffset), 0);

	_screen[OVERLAY_BUFFER]->addDirtyRect(_overlaySurface, Common::Rect(_overlaySurface.w, _overlaySurface.h), false);
}

void AtariGraphicsManager::grabOverlay(Graphics::Surface &surface) const {
	debug("grabOverlay: %d(%d), %d", surface.w, surface.pitch, surface.h);

	if (isOverlayDirectRendering()) {
		memset(surface.getPixels(), 0, surface.h * surface.pitch);
	} else {
		assert(surface.w >= _overlaySurface.w);
		assert(surface.h >= _overlaySurface.h);
		assert(surface.format.bytesPerPixel == _overlaySurface.format.bytesPerPixel);

		const byte *src = (const byte *)_overlaySurface.getPixels();
		byte *dst = (byte *)surface.getPixels();
		Graphics::copyBlit(dst, src, surface.pitch,
			_overlaySurface.pitch, _overlaySurface.w, _overlaySurface.h, _overlaySurface.format.bytesPerPixel);
	}
}

void AtariGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	//debug("copyRectToOverlay: %d, %d, %d(%d), %d", x, y, w, pitch, h);

	copyRectToScreenInternal(buf, pitch, x, y, w, h,
		getOverlayFormat(),
		isOverlayDirectRendering(),
		false);
}

bool AtariGraphicsManager::showMouse(bool visible) {
	//debug("showMouse: %d", visible);

	if (_cursor.visible == visible) {
		return visible;
	}

	bool last = _cursor.visible;
	_cursor.visible = visible;

	cursorVisibilityChanged();
	// don't rely on engines to call it (if they don't it confuses the cursor restore logic)
	updateScreen();

	return last;
}

void AtariGraphicsManager::warpMouse(int x, int y) {
	//debug("warpMouse: %d, %d", x, y);

	_cursor.setPosition(x, y);
	cursorPositionChanged();
}

void AtariGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor,
										  bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	//debug("setMouseCursor: %d, %d, %d, %d, %d, %d", w, h, hotspotX, hotspotY, keycolor, format ? format->bytesPerPixel : 1);

	if (mask)
		warning("AtariGraphicsManager::setMouseCursor: Masks are not supported");

	if (format)
		assert(*format == PIXELFORMAT_CLUT8);

	_cursor.setSurface(buf, (int)w, (int)h, hotspotX, hotspotY, keycolor);
	cursorSurfaceChanged();
}

void AtariGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	debug("setCursorPalette: %d, %d", start, num);

	memcpy(&_cursor.palette[start * 3], colors, num * 3);
	cursorSurfaceChanged();
}

void AtariGraphicsManager::updateMousePosition(int deltaX, int deltaY) {
	_cursor.updatePosition(deltaX, deltaY, *lockScreen());
	cursorPositionChanged();
}

bool AtariGraphicsManager::notifyEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_START) {
		return false;
	}

	switch ((CustomEventAction) event.customType) {
	case kActionToggleAspectRatioCorrection:
		_aspectRatioCorrection = !_aspectRatioCorrection;
		return true;
	}

	return false;
}

Common::Keymap *AtariGraphicsManager::getKeymap() const {
	Common::Keymap *keymap = new Common::Keymap(Common::Keymap::kKeymapTypeGlobal, "atari-graphics", _("Graphics"));
	Common::Action *act;

	if (hasFeature(OSystem::kFeatureAspectRatioCorrection)) {
		act = new Common::Action("ASPT", _("Toggle aspect ratio correction"));
		act->addDefaultInputMapping("C+A+a");
		act->setCustomBackendActionEvent(kActionToggleAspectRatioCorrection);
		keymap->addAction(act);
	}

	return keymap;
}

void AtariGraphicsManager::allocateSurfaces() {
	for (int i : { FRONT_BUFFER, BACK_BUFFER1, BACK_BUFFER2 }) {
		_screen[i] = new Screen(this, getMaximumScreenWidth(), getMaximumScreenHeight(), PIXELFORMAT_CLUT8, &_palette);
	}

	_screen[OVERLAY_BUFFER] = new Screen(this, getOverlayWidth(), getOverlayHeight(), getOverlayFormat(), &_overlayPalette);

	_chunkySurface.create(getMaximumScreenWidth(), getMaximumScreenHeight(), PIXELFORMAT_CLUT8);
	_overlaySurface.create(getOverlayWidth(), getOverlayHeight(), getOverlayFormat());
}

void AtariGraphicsManager::freeSurfaces() {
	for (int i : { FRONT_BUFFER, BACK_BUFFER1, BACK_BUFFER2, OVERLAY_BUFFER }) {
		delete _screen[i];
		_screen[i] = nullptr;
	}
	_workScreen = nullptr;

	_chunkySurface.free();
	_overlaySurface.free();
}

template <bool directRendering>	// hopefully compiler optimizes all the branching out
bool AtariGraphicsManager::updateScreenInternal(const Graphics::Surface &srcSurface) {
	//debug("updateScreenInternal");

	const DirtyRects &dirtyRects  = _workScreen->dirtyRects;
	Graphics::Surface *dstSurface = _workScreen->offsettedSurf;
	bool &cursorPositionChanged   = _workScreen->cursorPositionChanged;
	bool &cursorSurfaceChanged    = _workScreen->cursorSurfaceChanged;
	bool &cursorVisibilityChanged = _workScreen->cursorVisibilityChanged;
	Common::Rect &oldCursorRect   = _workScreen->oldCursorRect;
	const bool &fullRedraw        = _workScreen->fullRedraw;

	const int dstBitsPerPixel     = getBitsPerPixel(dstSurface->format);

	bool updated = false;

	const bool cursorDrawEnabled = !_cursor.outOfScreen && _cursor.visible;
	bool drawCursor = cursorDrawEnabled
		&& (cursorPositionChanged || cursorSurfaceChanged || cursorVisibilityChanged || fullRedraw);

	assert(!fullRedraw || oldCursorRect.isEmpty());

	bool restoreCursor = !oldCursorRect.isEmpty()
		&& (cursorPositionChanged || cursorSurfaceChanged || (cursorVisibilityChanged && !_cursor.visible));

	lockSuperBlitter();

	for (auto it = dirtyRects.begin(); it != dirtyRects.end(); ++it) {
		if (cursorDrawEnabled && !drawCursor)
			drawCursor = it->intersects(_cursor.dstRect);

		if (!directRendering) {
			copyRectToSurface(*dstSurface, dstBitsPerPixel, srcSurface, it->left, it->top, *it);
			updated = true;
		}
	}

	if (restoreCursor) {
		//debug("Restore cursor: %d %d %d %d", oldCursorRect.left, oldCursorRect.top, oldCursorRect.width(), oldCursorRect.height());

		// always restore aligned oldCursorRect
		oldCursorRect = alignRect(oldCursorRect);

		if (!directRendering) {
			copyRectToSurface(
				*dstSurface, dstBitsPerPixel, srcSurface,
				oldCursorRect.left, oldCursorRect.top,
				oldCursorRect);
		} else {
			_workScreen->restoreBackground(oldCursorRect);
		}

		oldCursorRect = Common::Rect();

		updated = true;
	}

	unlockSuperBlitter();

	if (drawCursor) {
		//debug("Redraw cursor: %d %d %d %d", _cursor.dstRect.left, _cursor.dstRect.top, _cursor.dstRect.width(), _cursor.dstRect.height());

		if (cursorSurfaceChanged || _cursor.isClipped()) {
			if (dstSurface->format.isCLUT8())
				_cursor.convertTo<true>(dstSurface->format);
			else
				_cursor.convertTo<false>(dstSurface->format);
			{
				// copy in-place (will do nothing on regular Surface::copyRectToSurface)
				Graphics::Surface surf;
				surf.init(
					_cursor.surface.w,
					_cursor.surface.h,
					_cursor.surface.pitch * dstBitsPerPixel / 8,	// 4bpp is not byte per pixel anymore
					_cursor.surface.getPixels(),
					_cursor.surface.format);
				copyRectToSurface(
					surf, dstBitsPerPixel, _cursor.surface,
					0, 0,
					Common::Rect(_cursor.surface.w, _cursor.surface.h));
			}
		}

		if (directRendering)
			_workScreen->storeBackground(alignRect(_cursor.dstRect));

		// don't use _cursor.srcRect for width as this must be aligned first
		// (_cursor.surface.w is recalculated thanks to _cursor.isClipped())
		drawMaskedSprite(
			*dstSurface, dstBitsPerPixel, _cursor.surface, _cursor.surfaceMask,
			_cursor.dstRect.left, _cursor.dstRect.top,
			Common::Rect(0, _cursor.srcRect.top, _cursor.surface.w, _cursor.srcRect.bottom));

		cursorPositionChanged = cursorSurfaceChanged = false;
		oldCursorRect = _cursor.dstRect;

		updated = true;
	}

	cursorVisibilityChanged = false;

	return updated;
}

void AtariGraphicsManager::copyRectToScreenInternal(const void *buf, int pitch, int x, int y, int w, int h,
													const Graphics::PixelFormat &format, bool directRendering, bool tripleBuffer) {
	Graphics::Surface &dstSurface = *lockScreen();

	const Common::Rect rect = alignRect(x, y, w, h);
	_workScreen->addDirtyRect(dstSurface, rect, directRendering);

	if (tripleBuffer) {
		_screen[BACK_BUFFER2]->addDirtyRect(dstSurface, rect, directRendering);
		_screen[FRONT_BUFFER]->addDirtyRect(dstSurface, rect, directRendering);
	}

	if (directRendering) {
		// TODO: mask the unaligned parts and copy the rest
		Graphics::Surface srcSurface;
		byte *srcBuf = (byte *)const_cast<void *>(buf);
		srcBuf -= (x - rect.left);	// HACK: this assumes pointer to a complete buffer
		srcSurface.init(rect.width(), rect.height(), pitch, srcBuf, format);

		copyRectToSurface(
			dstSurface, getBitsPerPixel(format), srcSurface,
			rect.left, rect.top,
			Common::Rect(rect.width(), rect.height()));
	} else {
		dstSurface.copyRectToSurface(buf, pitch, x, y, w, h);
	}
}

int AtariGraphicsManager::getBitsPerPixel(const Graphics::PixelFormat &format) const {
	return format == PIXELFORMAT_RGB121 ? 4 : 8;
}

bool AtariGraphicsManager::isOverlayDirectRendering() const {
	// overlay is direct rendered if in the launcher or if game is directly rendered
	// (on SuperVidel we always want to use shading/transparency but its direct rendering is fine and supported)
	return !hasSuperVidel()
#ifndef DISABLE_FANCY_THEMES
		   && (ConfMan.getActiveDomain() == nullptr || _currentState.mode == GraphicsMode::DirectRendering)
#endif
		;
}

AtariGraphicsManager::Screen::Screen(AtariGraphicsManager *manager, int width, int height, const Graphics::PixelFormat &format, const Palette *palette_)
	: _manager(manager) {
	const AtariMemAlloc &allocFunc = _manager->getStRamAllocFunc();

	palette = palette_;

	width += (_manager->_tt ? 0 : 2 * MAX_HZ_SHAKE);
	height += 2 * MAX_V_SHAKE;

	const int bitsPerPixel = _manager->getBitsPerPixel(format);

	surf.init(width, height, width * bitsPerPixel / 8, nullptr, format);

	void *pixelsUnaligned = allocFunc(sizeof(uintptr) + (surf.h * surf.pitch) + ALIGN - 1);
	if (!pixelsUnaligned) {
		error("Failed to allocate memory in ST RAM");
	}

	surf.setPixels((void *)(((uintptr)pixelsUnaligned + sizeof(uintptr) + ALIGN - 1) & (-ALIGN)));

	// store the unaligned pointer for later release
	*((uintptr *)surf.getPixels() - 1) = (uintptr)pixelsUnaligned;

	memset(surf.getPixels(), 0, surf.h * surf.pitch);

	_offsettedSurf.init(surf.w, surf.h, surf.pitch, surf.getBasePtr(_manager->_tt ? 0 : MAX_HZ_SHAKE, MAX_V_SHAKE), surf.format);
}

AtariGraphicsManager::Screen::~Screen() {
	const AtariMemFree &freeFunc = _manager->getStRamFreeFunc();

	freeFunc((void *)*((uintptr *)surf.getPixels() - 1));
}

void AtariGraphicsManager::Screen::reset(int width, int height, int bitsPerPixel) {
	cursorPositionChanged = true;
	cursorSurfaceChanged = true;
	cursorVisibilityChanged = false;
	clearDirtyRects();
	oldCursorRect = Common::Rect();
	rez = -1;
	mode = -1;

	// erase old screen
	_offsettedSurf.fillRect(Common::Rect(_offsettedSurf.w, _offsettedSurf.h), 0);

	if (_manager->_tt) {
		if (width <= 320 && height <= 240) {
			surf.w = 320;
			surf.h = 240 + 2 * MAX_V_SHAKE;
			surf.pitch = 2 * surf.w * bitsPerPixel / 8;
			rez = kRezValueTTLow;
		} else {
			surf.w = 640;
			surf.h = 480 + 2 * MAX_V_SHAKE;
			surf.pitch = surf.w * bitsPerPixel / 8;
			rez = kRezValueTTMid;
		}
	} else {
		mode = VsetMode(VM_INQUIRE) & PAL;

		if (_manager->_vgaMonitor) {
			mode |= VGA | (bitsPerPixel == 4 ? BPS4 : (hasSuperVidel() ? BPS8C : BPS8));

			if (width <= 320 && height <= 240) {
				surf.w = 320;
				surf.h = 240;
				mode |= VERTFLAG | COL40;
			} else {
				surf.w = 640;
				surf.h = 480;
				mode |= COL80;
			}
		} else {
			mode |= TV | (bitsPerPixel == 4 ? BPS4 : BPS8);

			if (width <= 320 && height <= 200) {
				surf.w = 320;
				surf.h = 200;
				mode |= COL40;
			} else if (width <= 320*1.2 && height <= 200*1.2) {
				surf.w = 320*1.2;
				surf.h = 200*1.2;
				mode |= OVERSCAN | COL40;
			} else if (width <= 640 && height <= 400) {
				surf.w = 640;
				surf.h = 400;
				mode |= VERTFLAG | COL80;
			} else {
				surf.w = 640*1.2;
				surf.h = 400*1.2;
				mode |= VERTFLAG | OVERSCAN | COL80;
			}
		}

		surf.w += 2 * MAX_HZ_SHAKE;
		surf.h += 2 * MAX_V_SHAKE;
		surf.pitch = surf.w * bitsPerPixel / 8;
	}

	_offsettedSurf.init(
		width, height, surf.pitch,
		surf.getBasePtr((surf.w - width) / 2, (surf.h - height) / 2),
		surf.format);
}

void AtariGraphicsManager::Screen::addDirtyRect(const Graphics::Surface &srcSurface, const Common::Rect &rect, bool directRendering) {
	if (fullRedraw)
		return;

	if ((rect.width() == srcSurface.w && rect.height() == srcSurface.h)
		|| dirtyRects.size() == 128) {	// 320x200 can hold at most 250 16x16 rectangles
		//debug("addDirtyRect[%d]: purge %d x %d", (int)dirtyRects.size(), srcSurface.w, srcSurface.h);

		dirtyRects.clear();
		dirtyRects.emplace(srcSurface.w, srcSurface.h);

		oldCursorRect = Common::Rect();

		fullRedraw = true;
		return;
	}

	dirtyRects.insert(rect);

	if (!oldCursorRect.isEmpty()) {
		const Common::Rect alignedOldCursorRect = _manager->alignRect(oldCursorRect);

		// we have to check *aligned* oldCursorRect because it is background which gets copied,
		// i.e. it has to be up to date even outside the cursor rectangle.
		// do it now to avoid complex checking in updateScreenInternal()
		if (rect.contains(alignedOldCursorRect)) {
			oldCursorRect = Common::Rect();
		} else if (rect.intersects(alignedOldCursorRect)) {
			if (!directRendering) {
				_manager->copyRectToSurface(
					*offsettedSurf, _manager->getBitsPerPixel(offsettedSurf->format), srcSurface,
					alignedOldCursorRect.left, alignedOldCursorRect.top,
					alignedOldCursorRect);
			} else {
				restoreBackground(alignedOldCursorRect);
			}

			oldCursorRect = Common::Rect();
		}
	}
}

void AtariGraphicsManager::Screen::storeBackground(const Common::Rect &rect) {
	const int bitsPerPixel = _manager->getBitsPerPixel(offsettedSurf->format);

	if (_cursorBackgroundSurf.w != rect.width()
		|| _cursorBackgroundSurf.h != rect.height()
		|| _cursorBackgroundSurf.format != offsettedSurf->format) {
		_cursorBackgroundSurf.create(rect.width(), rect.height(), offsettedSurf->format);
		_cursorBackgroundSurf.pitch = _cursorBackgroundSurf.pitch * bitsPerPixel / 8;
	}

	Graphics::copyBlit(
		(byte *)_cursorBackgroundSurf.getPixels(),
		(const byte *)offsettedSurf->getPixels() + rect.top * offsettedSurf->pitch + rect.left * bitsPerPixel / 8,
		_cursorBackgroundSurf.pitch, offsettedSurf->pitch,
		rect.width() * bitsPerPixel / 8, rect.height(),	// fake 4bpp by 8bpp's width/2
		offsettedSurf->format.bytesPerPixel);
}

void AtariGraphicsManager::Screen::restoreBackground(const Common::Rect &rect) {
	const int bitsPerPixel = _manager->getBitsPerPixel(offsettedSurf->format);

	Graphics::copyBlit(
		(byte *)offsettedSurf->getPixels() + rect.top * offsettedSurf->pitch + rect.left * bitsPerPixel / 8,
		(const byte *)_cursorBackgroundSurf.getPixels(),
		offsettedSurf->pitch, _cursorBackgroundSurf.pitch,
		rect.width() * bitsPerPixel / 8, rect.height(),	// fake 4bpp by 8bpp's width/2
		offsettedSurf->format.bytesPerPixel);
}


void AtariGraphicsManager::Cursor::update(const Graphics::Surface &screen, bool isModified) {
	if (!_buf) {
		outOfScreen = true;
		return;
	}

	if (!visible || !isModified)
		return;

	srcRect = Common::Rect(_width, _height);

	dstRect = Common::Rect(
		_x - _hotspotX,	// left
		_y - _hotspotY,	// top
		_x - _hotspotX + _width,	// right
		_y - _hotspotY + _height);	// bottom

	outOfScreen = !screen.clip(srcRect, dstRect);

	assert(srcRect.width() == dstRect.width());
	assert(srcRect.height() == dstRect.height());
}

void AtariGraphicsManager::Cursor::updatePosition(int deltaX, int deltaY, const Graphics::Surface &screen) {
	_x += deltaX;
	_y += deltaY;

	if (_x < 0)
		_x = 0;
	else if (_x >= screen.w)
		_x = screen.w - 1;

	if (_y < 0)
		_y = 0;
	else if (_y >= screen.h)
		_y = screen.h - 1;
}

void AtariGraphicsManager::Cursor::setSurface(const void *buf, int w, int h, int hotspotX, int hotspotY, uint32 keycolor) {
	if (w == 0 || h == 0 || buf == nullptr) {
		_buf = nullptr;
		return;
	}

	_buf = (const byte *)buf;
	_width = w;
	_height = h;
	_hotspotX = hotspotX;
	_hotspotY = hotspotY;
	_keycolor = keycolor;
}

template <bool isClut8>	// hopefully compiler optimizes all the branching out
void AtariGraphicsManager::Cursor::convertTo(const Graphics::PixelFormat &format) {
	const int cursorWidth = (srcRect.width() + 15) & (-16);
	const int cursorHeight = _height;

	if (surface.w != cursorWidth || surface.h != cursorHeight || surface.format != format) {
		if (!isClut8 && surface.format != format) {
			_rShift = format.rLoss - format.rShift;
			_gShift = format.gLoss - format.gShift;
			_bShift = format.bLoss - format.bShift;

			_rMask = format.rMax() << format.rShift;
			_gMask = format.gMax() << format.gShift;
			_bMask = format.bMax() << format.bShift;
		}

		surface.create(cursorWidth, cursorHeight, format);

		const bool old_unalignedPitch = g_unalignedPitch;
		g_unalignedPitch = true;
		surfaceMask.create(surface.w / 8, surface.h, format);	// 1 bpl
		g_unalignedPitch = old_unalignedPitch;
	}

	const int srcRectWidth = srcRect.width();

	const byte *src = _buf + srcRect.left;
	byte *dst = (byte *)surface.getPixels();
	uint16 *dstMask = (uint16 *)surfaceMask.getPixels();
	const int srcPadding = _width - srcRectWidth;
	const int dstPadding = surface.w - srcRectWidth;

	for (int j = 0; j < cursorHeight; ++j) {
		for (int i = 0; i < srcRectWidth; ++i) {
			const uint32 color = *src++;
			const uint16 bit = 1 << (15 - (i % 16));

			if (color != _keycolor) {
				if (!isClut8) {
					// Convert CLUT8 to RGB332/RGB121 palette
					*dst++ = ((palette[color*3 + 0] >> _rShift) & _rMask)
						   | ((palette[color*3 + 1] >> _gShift) & _gMask)
						   | ((palette[color*3 + 2] >> _bShift) & _bMask);
				} else {
					*dst++ = color;
				}

				// clear bit
				*dstMask &= ~bit;
			} else {
				*dst++ = 0x00;

				// set bit
				*dstMask |= bit;
			}

			if (bit == 0x0001)
				dstMask++;
		}

		src += srcPadding;

		if (dstPadding) {
			memset(dst, 0x00, dstPadding);
			dst += dstPadding;

			*dstMask |= ((1 << dstPadding) - 1);
			dstMask++;
		}
	}
}
