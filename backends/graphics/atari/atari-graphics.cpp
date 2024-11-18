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

#include "atari-graphics.h"

#include <mint/cookie.h>
#include <mint/falcon.h>
#include <mint/osbind.h>
#include <mint/sysvars.h>

#include "backends/platform/atari/atari-debug.h"
#include "backends/platform/atari/dlmalloc.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "common/translation.h"
#include "engines/engine.h"
#include "graphics/blit.h"
#include "gui/ThemeEngine.h"

#include "atari-graphics-superblitter.h"

#define SCREEN_ACTIVE

bool g_unalignedPitch = false;
mspace g_mspace = nullptr;

static const Graphics::PixelFormat PIXELFORMAT_CLUT8 = Graphics::PixelFormat::createFormatCLUT8();
static const Graphics::PixelFormat PIXELFORMAT_RGB332 = Graphics::PixelFormat(1, 3, 3, 2, 0, 5, 2, 0, 0);
static const Graphics::PixelFormat PIXELFORMAT_RGB121 = Graphics::PixelFormat(1, 1, 2, 1, 0, 3, 1, 0, 0);

static void shrinkVidelVisibleArea() {
	// Active VGA screen area consists of 960 half-lines, i.e. 480 raster lines.
	// In case of 320x240, the number is still 480 but data is fetched
	// only for 240 lines so it doesn't make a difference to us.
#ifdef SCREEN_ACTIVE
	if (hasSuperVidel()) {
		const int vOffset = ((480 - 400) / 2) * 2;	// *2 because of half-lines

		// VDB = VBE = VDB + paddding/2
		*((volatile uint16*)0xFFFF82A8) = *((volatile uint16*)0xFFFF82A6) = *((volatile uint16*)0xFFFF82A8) + vOffset;
		// VDE = VBB = VDE - padding/2
		*((volatile uint16*)0xFFFF82AA) = *((volatile uint16*)0xFFFF82A4) = *((volatile uint16*)0xFFFF82AA) - vOffset;
	} else {
		// 31500/60.1 = 524 raster lines
		// vft = 524 * 2 + 1 = 1049 half-lines
		// 480 visible lines = 960 half-lines
		// 1049 - 960 = 89 half-lines reserved for borders
		// we want 400 visible lines = 800 half-lines
		// vft = 800 + 89 = 889 half-lines in total ~ 70.1 Hz vertical frequency
		int16 vft = *((volatile int16*)0xFFFF82A2);
		int16 vss = *((volatile int16*)0xFFFF82AC);	// vss = vft - vss_sync
		vss -= vft;	// -vss_sync
		*((volatile int16*)0xFFFF82A2) = 889;
		*((volatile int16*)0xFFFF82AC) = 889 + vss;
	}
#endif
}

static bool s_tt;
static int s_shakeXOffset;
static int s_shakeYOffset;
static int s_aspectRatioCorrectionYOffset;
static bool s_shrinkVidelVisibleArea;
static bool s_setScreenOffsets;
static Graphics::Surface *s_screenSurf;

static void VblHandler() {
	// for easier querying
	static Graphics::Surface *surf;

	if (s_screenSurf)
		surf = s_screenSurf;

	if (s_screenSurf || s_setScreenOffsets) {
#ifdef SCREEN_ACTIVE
		uintptr p = (unsigned long)surf->getBasePtr(0, MAX_V_SHAKE + s_shakeYOffset + s_aspectRatioCorrectionYOffset);

		if (!s_tt) {
			const int bitsPerPixel = (surf->format == PIXELFORMAT_RGB121 ? 4 : 8);

			int shakeXOffset = -s_shakeXOffset;

			if (shakeXOffset >= 0) {
				p += MAX_HZ_SHAKE;
				*((volatile char *)0xFFFF8265) = shakeXOffset;
			} else {
				*((volatile char *)0xFFFF8265) = MAX_HZ_SHAKE + shakeXOffset;
			}

			// subtract 4 or 8 words if scrolling
			*((volatile short *)0xFFFF820E) = shakeXOffset == 0
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
		s_setScreenOffsets = false;
	}

	if (s_shrinkVidelVisibleArea) {
		if (!s_tt)
			shrinkVidelVisibleArea();
		s_shrinkVidelVisibleArea = false;
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
static Palette s_oldPalette;

void AtariGraphicsShutdown() {
	Supexec(UninstallVblHandler);

	if (s_oldRez != -1) {
		Setscreen(SCR_NOCHANGE, s_oldPhysbase, s_oldRez);

		EsetPalette(0, s_oldPalette.entries, s_oldPalette.tt);
	} else if (s_oldMode != -1) {
		static _RGB black[256];
		VsetRGB(0, 256, black);

		VsetScreen(SCR_NOCHANGE, s_oldPhysbase, SCR_NOCHANGE, SCR_NOCHANGE);

		if (hasSuperVidel()) {
			// SuperVidel XBIOS does not restore those (unlike TOS/EmuTOS)
			long ssp = Super(SUP_SET);
			//*((volatile char *)0xFFFF8265) = 0;
			*((volatile short *)0xFFFF820E) = 0;
			Super(ssp);

			VsetMode(SVEXT | SVEXT_BASERES(0) | COL80 | BPS8C);	// resync to proper 640x480
		}
		VsetMode(s_oldMode);

		VsetRGB(0, s_oldPalette.entries, s_oldPalette.falcon);
	}
}

AtariGraphicsManager::AtariGraphicsManager()
	: _pendingScreenChanges(this) {
	atari_debug("AtariGraphicsManager()");

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
		ConfMan.setPath("themepath", "themes");
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

	if (_tt) {
		s_oldRez = Getrez();
		// EgetPalette / EsetPalette doesn't care about current resolution's number of colors
		s_oldPalette.entries = 256;
		EgetPalette(0, 256, s_oldPalette.tt);
	} else {
		s_oldMode = VsetMode(VM_INQUIRE);
		switch (s_oldMode & NUMCOLS) {
		case BPS1:
			s_oldPalette.entries = 2;
			break;
		case BPS2:
			s_oldPalette.entries = 4;
			break;
		case BPS4:
			s_oldPalette.entries = 16;
			break;
		case BPS8:
		case BPS8C:
			s_oldPalette.entries = 256;
			break;
		default:
			s_oldPalette.entries = 0;
		}
		VgetRGB(0, s_oldPalette.entries, s_oldPalette.falcon);
	}
	s_oldPhysbase = Physbase();

	if (!Supexec(InstallVblHandler)) {
		error("VBL handler was not installed");
	}

	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

AtariGraphicsManager::~AtariGraphicsManager() {
	atari_debug("~AtariGraphicsManager()");

	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	AtariGraphicsShutdown();
}

bool AtariGraphicsManager::hasFeature(OSystem::Feature f) const {
	switch (f) {
	case OSystem::Feature::kFeatureAspectRatioCorrection:
		//atari_debug("hasFeature(kFeatureAspectRatioCorrection): %d", !_tt);
		return !_tt;
	case OSystem::Feature::kFeatureCursorPalette:
		// FIXME: pretend to have cursor palette at all times, this function
		// can get (and it is) called any time, before and after showOverlay()
		// (overlay cursor uses the cross if kFeatureCursorPalette returns false
		// here too soon)
		//atari_debug("hasFeature(kFeatureCursorPalette): %d", isOverlayVisible());
		//return isOverlayVisible();
		return true;
	default:
		return false;
	}

	// TODO: kFeatureDisplayLogFile?, kFeatureClipboardSupport, kFeatureSystemBrowserDialog
}

void AtariGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	if (!hasFeature(f))
		return;

	// flags must be set to _pendingScreenChanges here

	switch (f) {
	case OSystem::Feature::kFeatureAspectRatioCorrection:
		//atari_debug("setFeatureState(kFeatureAspectRatioCorrection): %d", enable);
		if (_aspectRatioCorrection != enable) {
			_aspectRatioCorrection = enable;

			if (!_overlayVisible) {
				_pendingScreenChanges.queueAspectRatioCorrection();

				if (!_pendingState.inTransaction)
					updateScreen();
			}
		}
		break;
	default:
		break;
	}
}

bool AtariGraphicsManager::getFeatureState(OSystem::Feature f) const {
	switch (f) {
	case OSystem::Feature::kFeatureAspectRatioCorrection:
		//atari_debug("getFeatureState(kFeatureAspectRatioCorrection): %d", _aspectRatioCorrection);
		return _aspectRatioCorrection;
	case OSystem::Feature::kFeatureCursorPalette:
		//atari_debug("getFeatureState(kFeatureCursorPalette): %d", isOverlayVisible());
		//return isOverlayVisible();
		return true;
	default:
		return false;
	}
}

bool AtariGraphicsManager::setGraphicsMode(int mode, uint flags) {
	atari_debug("setGraphicsMode: %d, %d", mode, flags);

	_pendingState.mode = (GraphicsMode)mode;

	if (!_pendingState.inTransaction)
		return endGFXTransaction() == OSystem::kTransactionSuccess;

	// this doesn't seem to be checked anywhere
	return true;
}

void AtariGraphicsManager::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	atari_debug("initSize: %d, %d, %d", width, height, format ? format->bytesPerPixel : 1);

	_pendingState.width  = width;
	_pendingState.height = height;
	_pendingState.format = format ? *format : PIXELFORMAT_CLUT8;

	if (!_pendingState.inTransaction)
		endGFXTransaction();
}

void AtariGraphicsManager::beginGFXTransaction() {
	atari_debug("beginGFXTransaction");

	_pendingState = GraphicsState();
	_pendingState.inTransaction = true;
	_pendingScreenChanges.clearTransaction();
}

OSystem::TransactionError AtariGraphicsManager::endGFXTransaction() {
	atari_debug("endGFXTransaction");

	_pendingState.inTransaction = false;

	int error = OSystem::TransactionError::kTransactionSuccess;
	bool hasPendingGraphicsMode = false;	// no need to have a global flag

	if (_pendingState.mode != GraphicsMode::Unknown) {
		if (_pendingState.mode < GraphicsMode::DirectRendering || _pendingState.mode > GraphicsMode::TripleBuffering) {
			error |= OSystem::TransactionError::kTransactionModeSwitchFailed;
		} else if (_currentState.mode != _pendingState.mode) {
			hasPendingGraphicsMode = true;
		}
	}

	if (_pendingState.width > 0 && _pendingState.height > 0) {
		if (_pendingState.width > getMaximumScreenWidth() || _pendingState.height > getMaximumScreenHeight()) {
			error |= OSystem::TransactionError::kTransactionSizeChangeFailed;
		} else if (_pendingState.width % 16 != 0 && !hasSuperVidel()) {
			atari_warning("Requested width not divisible by 16, please report");
			error |= OSystem::TransactionError::kTransactionSizeChangeFailed;
		} else if (_overlayVisible || _currentState.width != _pendingState.width || _currentState.height != _pendingState.height) {
			_pendingScreenChanges.queueVideoMode();
		}
	}

	if (_pendingState.format.bytesPerPixel != 0
		&& _pendingState.format != PIXELFORMAT_CLUT8)
		error |= OSystem::TransactionError::kTransactionFormatNotSupported;

	if (error != OSystem::TransactionError::kTransactionSuccess) {
		atari_warning("endGFXTransaction failed: %02x", (int)error);
		_pendingScreenChanges.clearTransaction();
		return static_cast<OSystem::TransactionError>(error);
	}

	if (hasPendingGraphicsMode)
		_currentState.mode = _pendingState.mode;

	if (_pendingScreenChanges.videoMode()) {
		_currentState.width  = _pendingState.width;
		_currentState.height = _pendingState.height;
		_currentState.format = _pendingState.format;

		if (_overlayVisible) {
			_checkUnalignedPitch = true;
			_ignoreHideOverlay = false;
			_overlayVisible = false;
			// if being in the overlay, reset everything (same as hideOverlay() does)
			// s_screenSurf will be set below
			_pendingScreenChanges.queueAll();
		}
	} else if (_overlayVisible) {
		// don't exit overlay unless there is real video mode to be set
		_ignoreHideOverlay = true;
		_pendingState = GraphicsState();
		_pendingScreenChanges.clearTransaction();
		return OSystem::kTransactionSuccess;
	}

	if (_pendingScreenChanges.videoMode() || hasPendingGraphicsMode) {
		_chunkySurface.init(_currentState.width, _currentState.height, _currentState.width,
			_chunkySurface.getPixels(), _currentState.format);

		_screen[FRONT_BUFFER]->reset(_currentState.width, _currentState.height, 8, true);
		_screen[BACK_BUFFER1]->reset(_currentState.width, _currentState.height, 8, true);
		_screen[BACK_BUFFER2]->reset(_currentState.width, _currentState.height, 8, true);
		_workScreen = _screen[_currentState.mode <= GraphicsMode::SingleBuffering ? FRONT_BUFFER : BACK_BUFFER1];
		_pendingScreenChanges.setScreenSurface(&_screen[FRONT_BUFFER]->surf);

		_palette.clear();
		_pendingScreenChanges.queuePalette();
	}

	_pendingState = GraphicsState();

	// apply new screen changes
	updateScreen();

	return OSystem::kTransactionSuccess;
}

void AtariGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	//atari_debug("setPalette: %d, %d", start, num);

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

	_pendingScreenChanges.queuePalette();
}

void AtariGraphicsManager::grabPalette(byte *colors, uint start, uint num) const {
	//atari_debug("grabPalette: %d, %d", start, num);

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
	//atari_debug("copyRectToScreen: %d, %d, %d(%d), %d", x, y, w, pitch, h);

	copyRectToScreenInternal(buf, pitch, x, y, w, h,
		PIXELFORMAT_CLUT8,
		_currentState.mode == GraphicsMode::DirectRendering,
		_currentState.mode == GraphicsMode::TripleBuffering);
}

// this is not really locking anything but it's an useful function
// to return current rendering surface :)
Graphics::Surface *AtariGraphicsManager::lockScreen() {
	//atari_debug("lockScreen");

	if (isOverlayVisible() && !isOverlayDirectRendering())
		return &_overlaySurface;
	else if ((isOverlayVisible() && isOverlayDirectRendering()) || _currentState.mode == GraphicsMode::DirectRendering)
		return _workScreen->offsettedSurf;
	else
		return &_chunkySurface;
}

void AtariGraphicsManager::unlockScreen() {
	//atari_debug("unlockScreen: %d x %d", _workScreen->surf.w, _workScreen->surf.h);

	const Graphics::Surface &dstSurface = *lockScreen();

	const bool directRendering = (dstSurface.getPixels() != _chunkySurface.getPixels());
	const Common::Rect rect = alignRect(0, 0, dstSurface.w, dstSurface.h);
	_workScreen->addDirtyRect(dstSurface, rect, directRendering);

	if (_currentState.mode == GraphicsMode::TripleBuffering) {
		_screen[BACK_BUFFER2]->addDirtyRect(dstSurface, rect, directRendering);
		_screen[FRONT_BUFFER]->addDirtyRect(dstSurface, rect, directRendering);
	}
}

void AtariGraphicsManager::fillScreen(uint32 col) {
	//atari_debug("fillScreen: %d", col);

	Graphics::Surface *screen = lockScreen();

	screen->fillRect(Common::Rect(screen->w, screen->h), col);

	unlockScreen();
}

void AtariGraphicsManager::fillScreen(const Common::Rect &r, uint32 col) {
	//atari_debug("fillScreen: %dx%d %d", r.width(), r.height(), col);

	Graphics::Surface *screen = lockScreen();

	if (r.width() == 1 && r.height() == 1) {
		// handle special case for e.g. Eco Quest's intro
		byte *ptr = (byte *)screen->getBasePtr(r.left, r.top);
		*ptr = col;
	} else {
		screen->fillRect(r, col);
	}

	unlockScreen();
}

void AtariGraphicsManager::updateScreen() {
	//atari_debug("updateScreen");

	// avoid falling into the atari_debugger (screen may not not initialized yet)
	Common::setErrorHandler(nullptr);

	if (_checkUnalignedPitch) {
		const Common::ConfigManager::Domain *activeDomain = ConfMan.getActiveDomain();
		if (activeDomain) {
			// FIXME: Some engines are too bound to linear surfaces that it is very
			// hard to repair them. So instead of polluting the engine with
			// Surface::init() & delete[] Surface::getPixels() just use this hack.
			const Common::String engineId = activeDomain->getValOrDefault("engineid");
			const Common::String gameId = activeDomain->getValOrDefault("gameid");

			atari_debug("checking %s/%s", engineId.c_str(), gameId.c_str());

			if (engineId == "composer"
				|| engineId == "hypno"
				|| engineId == "mohawk"
				|| engineId == "parallaction"
				|| engineId == "private"
				|| (engineId == "sci"
					&& (gameId == "phantasmagoria" || gameId == "shivers"))
				|| engineId == "sherlock"
				|| engineId == "teenagent"
				|| engineId == "tsage") {
				g_unalignedPitch = true;
			} else {
				g_unalignedPitch = false;
			}
		}

		_checkUnalignedPitch = false;
	}

	_workScreen->cursor.update();

	bool screenUpdated = false;

	if (isOverlayVisible()) {
		assert(_workScreen == _screen[OVERLAY_BUFFER]);
		if (isOverlayDirectRendering())
			screenUpdated = updateScreenInternal(Graphics::Surface());
		else
			screenUpdated = updateScreenInternal(_overlaySurface);
	} else {
		switch (_currentState.mode) {
		case GraphicsMode::DirectRendering:
			assert(_workScreen == _screen[FRONT_BUFFER]);
			screenUpdated = updateScreenInternal(Graphics::Surface());
			break;
		case GraphicsMode::SingleBuffering:
			assert(_workScreen == _screen[FRONT_BUFFER]);
			screenUpdated = updateScreenInternal(_chunkySurface);
			break;
		case GraphicsMode::TripleBuffering:
			assert(_workScreen == _screen[BACK_BUFFER1]);
			screenUpdated = updateScreenInternal(_chunkySurface);
			break;
		default:
			atari_warning("Unknown graphics mode %d", (int)_currentState.mode);
		}
	}

	_workScreen->clearDirtyRects();

	if (_overlayPending) {
		atari_debug("Forcing overlay pending state");
		// must be done here because first updateScreen() is not called from showOverlay()
		_pendingScreenChanges.queueAll();
		_overlayPending = false;
	}

#ifdef SCREEN_ACTIVE
	// this assume that the screen surface is not going to be used yet
	_pendingScreenChanges.applyBeforeVblLock();
#endif

	set_sysvar_to_short(vblsem, 0);  // lock vbl

	if (screenUpdated
		&& !isOverlayVisible()
		&& _currentState.mode == GraphicsMode::TripleBuffering) {
		// Triple buffer:
		// - alternate BACK_BUFFER1 and BACK_BUFFER2
		// - present BACK_BUFFER1 (as BACK_BUFFER2)
		// - check if BACK_BUFFER2 has been displayed, if so, switch
		//   BACK_BUFFER2 and FRONT_BUFFER and make previous BACK_BUFFER2 work screen

		if (s_screenSurf == nullptr) {
			// BACK_BUFFER2 has been set; guard it from overwriting while presented
			Screen *tmp = _screen[BACK_BUFFER2];
			_screen[BACK_BUFFER2] = _screen[FRONT_BUFFER];
			_screen[FRONT_BUFFER] = tmp;
		}

		// swap back buffers
		Screen *tmp = _screen[BACK_BUFFER1];
		_screen[BACK_BUFFER1] = _screen[BACK_BUFFER2];
		_screen[BACK_BUFFER2] = tmp;

		// queue BACK_BUFFER2 with the most recent frame content
		_pendingScreenChanges.setScreenSurface(&_screen[BACK_BUFFER2]->surf);
		_workScreen = _screen[BACK_BUFFER1];
	}

#ifdef SCREEN_ACTIVE
	_pendingScreenChanges.applyAfterVblLock();
#endif

	if (_pendingScreenChanges.screenSurface()) {
		s_screenSurf = _pendingScreenChanges.screenSurface();
		_pendingScreenChanges.setScreenSurface(nullptr);
	}

	if (_pendingScreenChanges.aspectRatioCorrectionYOffset().second)
		s_aspectRatioCorrectionYOffset = _pendingScreenChanges.aspectRatioCorrectionYOffset().first;
	if (_pendingScreenChanges.screenOffsets().second)
		s_setScreenOffsets = _pendingScreenChanges.screenOffsets().first;
	if (_pendingScreenChanges.shrinkVidelVisibleArea().second)
		s_shrinkVidelVisibleArea = _pendingScreenChanges.shrinkVidelVisibleArea().first;

	set_sysvar_to_short(vblsem, 1);  // unlock vbl

	//atari_debug("end of updateScreen");
}

void AtariGraphicsManager::setShakePos(int shakeXOffset, int shakeYOffset) {
	//atari_debug("setShakePos: %d, %d", shakeXOffset, shakeYOffset);

	if (_tt) {
		// as TT can't horizontally shake anything, do it at least vertically
		s_shakeYOffset = (shakeYOffset == 0 && shakeXOffset != 0) ? shakeXOffset : shakeYOffset;
	} else {
		s_shakeXOffset = shakeXOffset;
		s_shakeYOffset = shakeYOffset;
	}

	_pendingScreenChanges.queueShakeScreen();
}

void AtariGraphicsManager::showOverlay(bool inGUI) {
	atari_debug("showOverlay (visible: %d)", _overlayVisible);

	if (_overlayVisible)
		return;

	if (_currentState.mode == GraphicsMode::DirectRendering) {
		_workScreen->cursor.restoreBackground(Graphics::Surface(), true);
	}

	_oldWorkScreen = _workScreen;
	_workScreen = _screen[OVERLAY_BUFFER];
	_pendingScreenChanges.setScreenSurface(&_workScreen->surf);

	// do not cache dirtyRects and oldCursorRect
	_workScreen->reset(getOverlayWidth(), getOverlayHeight(), getBitsPerPixel(getOverlayFormat()), false);

	_overlayVisible = true;

	if (!_pendingScreenChanges.empty()) {
		warning("showOverlay: _pendingScreenChanges is %02x", _pendingScreenChanges.get());
	}
	//_pendingScreenChanges.queueAll();	// must be called in updateScreen() ...
	_overlayPending = true;
	updateScreen();
}

void AtariGraphicsManager::hideOverlay() {
	atari_debug("hideOverlay (ignore: %d, visible: %d)", _ignoreHideOverlay, _overlayVisible);

	if (!_overlayVisible)
		return;

	if (_ignoreHideOverlay) {
		// faster than _workScreen->reset()
		_workScreen->clearDirtyRects();
		_workScreen->cursor.reset();
		return;
	}

	// BACK_BUFFER2 is intentional: regardless of the state before calling showOverlay(),
	// this always contains the next desired frame buffer to show
	_pendingScreenChanges.setScreenSurface(&_screen[_currentState.mode == GraphicsMode::TripleBuffering ? BACK_BUFFER2 : FRONT_BUFFER]->surf);
	_workScreen = _oldWorkScreen;
	_oldWorkScreen = nullptr;

	_overlayVisible = false;

	if (!_pendingScreenChanges.empty()) {
		warning("hideOverlay: _pendingScreenChanges is %02x", _pendingScreenChanges.get());
	}
	_pendingScreenChanges.queueAll();
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

	atari_debug("clearOverlay");

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
	atari_debug("grabOverlay: %d(%d), %d", surface.w, surface.pitch, surface.h);

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
	//atari_debug("copyRectToOverlay: %d, %d, %d(%d), %d", x, y, w, pitch, h);

	copyRectToScreenInternal(buf, pitch, x, y, w, h,
		getOverlayFormat(),
		isOverlayDirectRendering(),
		false);
}

bool AtariGraphicsManager::showMouse(bool visible) {
	//atari_debug("showMouse: %d", visible);

	bool last = _workScreen->cursor.setVisible(visible);

	if (!isOverlayVisible() && _currentState.mode == GraphicsMode::TripleBuffering) {
		_screen[BACK_BUFFER2]->cursor.setVisible(visible);
		_screen[FRONT_BUFFER]->cursor.setVisible(visible);
	}

	// don't rely on engines to call it (if they don't it confuses the cursor restore logic)
	updateScreen();

	return last;
}

void AtariGraphicsManager::warpMouse(int x, int y) {
	//atari_debug("warpMouse: %d, %d", x, y);

	_workScreen->cursor.setPosition(x, y);

	if (!isOverlayVisible() && _currentState.mode == GraphicsMode::TripleBuffering) {
		_screen[BACK_BUFFER2]->cursor.setPosition(x, y);
		_screen[FRONT_BUFFER]->cursor.setPosition(x, y);
	}
}

void AtariGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor,
										  bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	//atari_debug("setMouseCursor: %d, %d, %d, %d, %d, %d", w, h, hotspotX, hotspotY, keycolor, format ? format->bytesPerPixel : 1);

	if (mask)
		atari_warning("AtariGraphicsManager::setMouseCursor: Masks are not supported");

	if (format)
		assert(*format == PIXELFORMAT_CLUT8);

	_workScreen->cursor.setSurface(buf, (int)w, (int)h, hotspotX, hotspotY, keycolor);

	if (!isOverlayVisible() && _currentState.mode == GraphicsMode::TripleBuffering) {
		_screen[BACK_BUFFER2]->cursor.setSurface(buf, (int)w, (int)h, hotspotX, hotspotY, keycolor);
		_screen[FRONT_BUFFER]->cursor.setSurface(buf, (int)w, (int)h, hotspotX, hotspotY, keycolor);
	}
}

void AtariGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	atari_debug("setCursorPalette: %d, %d", start, num);

	if (isOverlayVisible()) {
		// cursor palette is supported only in the overlay
		_workScreen->cursor.setPalette(colors, start, num);
	}
}

void AtariGraphicsManager::updateMousePosition(int deltaX, int deltaY) {
	_workScreen->cursor.updatePosition(deltaX, deltaY);

	if (!isOverlayVisible() && _currentState.mode == GraphicsMode::TripleBuffering) {
		_screen[BACK_BUFFER2]->cursor.updatePosition(deltaX, deltaY);
		_screen[FRONT_BUFFER]->cursor.updatePosition(deltaX, deltaY);
	}
}

bool AtariGraphicsManager::notifyEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_RETURN_TO_LAUNCHER:
	case Common::EVENT_QUIT:
		if (isOverlayVisible()) {
			// clear work screen: this is needed if *next* game shows an error upon startup
			Graphics::Surface &surf = _currentState.mode == GraphicsMode::DirectRendering
				? *_screen[FRONT_BUFFER]->offsettedSurf
				: _chunkySurface;
			surf.fillRect(Common::Rect(surf.w, surf.h), 0);

			_ignoreHideOverlay = true;
			return false;
		}
		break;

	case Common::EVENT_CUSTOM_BACKEND_ACTION_START:
		switch ((CustomEventAction) event.customType) {
		case kActionToggleAspectRatioCorrection:
			if (hasFeature(OSystem::Feature::kFeatureAspectRatioCorrection)) {
				_aspectRatioCorrection = !_aspectRatioCorrection;

				if (!_overlayVisible) {
					_pendingScreenChanges.queueAspectRatioCorrection();

					updateScreen();
				}
				return true;
			}
			break;
		}
		break;

	default:
		return false;
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

	// overlay is the default screen upon start
	_workScreen = _screen[OVERLAY_BUFFER] = new Screen(this, getOverlayWidth(), getOverlayHeight(), getOverlayFormat(), &_overlayPalette);
	_workScreen->reset(getOverlayWidth(), getOverlayHeight(), getBitsPerPixel(getOverlayFormat()), true);
	_pendingScreenChanges.setScreenSurface(&_workScreen->surf);

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

bool AtariGraphicsManager::updateScreenInternal(const Graphics::Surface &srcSurface) {
	//atari_debug("updateScreenInternal");

	const Screen::DirtyRects &dirtyRects = _workScreen->dirtyRects;
	Graphics::Surface *dstSurface        = _workScreen->offsettedSurf;
	Cursor &cursor                       = _workScreen->cursor;

	const bool directRendering           = srcSurface.getPixels() == nullptr;
	const int dstBitsPerPixel            = getBitsPerPixel(dstSurface->format);

	bool updated = false;

	const bool cursorDrawEnabled = cursor.isVisible();
	bool forceCursorDraw = cursorDrawEnabled && (_workScreen->fullRedraw || cursor.isChanged());

	lockSuperBlitter();

	for (auto it = dirtyRects.begin(); it != dirtyRects.end(); ++it) {
		if (cursorDrawEnabled && !forceCursorDraw)
			forceCursorDraw = cursor.intersects(*it);

		if (!directRendering) {
			copyRectToSurface(*dstSurface, dstBitsPerPixel, srcSurface, it->left, it->top, *it);
			updated |= true;
		}
	}

	updated |= cursor.restoreBackground(srcSurface, false);

	unlockSuperBlitter();

	updated |= cursor.draw(directRendering, forceCursorDraw);

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
