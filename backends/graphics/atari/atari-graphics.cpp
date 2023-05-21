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

#include "backends/graphics/atari/atari-graphics.h"

#include <mint/cookie.h>
#include <mint/falcon.h>
#include <mint/osbind.h>

#include "backends/graphics/atari/atari-graphics-asm.h"
#include "backends/graphics/atari/atari-graphics-superblitter.h"
#include "backends/graphics/atari/videl-resolutions.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "common/config-manager.h"
#include "common/str.h"
#include "common/textconsole.h"	// for warning() & error()
#include "common/translation.h"
#include "graphics/blit.h"
#include "gui/ThemeEngine.h"

bool g_unalignedPitch = false;

#define SCREEN_ACTIVE

AtariGraphicsManager::AtariGraphicsManager() {
	_vgaMonitor = VgetMonitor() == MON_VGA;

	// make the standard GUI renderer default (!DISABLE_FANCY_THEMES implies anti-aliased rendering in ThemeEngine.cpp)
	// (and without DISABLE_FANCY_THEMES we can't use 640x480 themes)
	const char *standardThemeEngineName = GUI::ThemeEngine::findModeConfigName(GUI::ThemeEngine::kGfxStandard);
	ConfMan.registerDefault("gui_renderer", standardThemeEngineName);
	if (!ConfMan.hasKey("gui_renderer"))
		ConfMan.set("gui_renderer", standardThemeEngineName);

	// make the built-in theme default to avoid long loading times
	ConfMan.registerDefault("gui_theme", "builtin");
	if (!ConfMan.hasKey("gui_theme"))
		ConfMan.set("gui_theme", "builtin");

	// make "themes" the default theme path
	ConfMan.registerDefault("themepath", "themes");
	if (!ConfMan.hasKey("themepath"))
		ConfMan.set("themepath", "themes");

	ConfMan.flushToDisk();

	// Generate RGB332 palette for the overlay
	for (uint i = 0; i < 256; i++) {
		_overlayPalette[i*3 + 0] = ((i >> 5) & 7) << 5;
		_overlayPalette[i*3 + 1] = ((i >> 2) & 7) << 5;
		_overlayPalette[i*3 + 2] = (i & 3) << 6;
	}

	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

AtariGraphicsManager::~AtariGraphicsManager() {
	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
}

bool AtariGraphicsManager::hasFeature(OSystem::Feature f) const {
	switch (f) {
	case OSystem::Feature::kFeatureAspectRatioCorrection:
		//debug("hasFeature(kFeatureAspectRatioCorrection): %d", !_vgaMonitor);
		return !_vgaMonitor;
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

	// always initialize (clear screen, mouse init, ...)
	//if (_pendingState == _currentState)
	//	return static_cast<OSystem::TransactionError>(error);

	if (_pendingState.format != PIXELFORMAT_CLUT8)
		error |= OSystem::TransactionError::kTransactionFormatNotSupported;

	// TODO: Several engines support unusual resolutions like 256x240 (NES Maniac Mansion),
	// 512x342 (MacVenture, WAGE) or 544x332 (Myst)
	if ((_pendingState.width != 320 || (_pendingState.height != 200 && _pendingState.height != 240))
			&& (_pendingState.width != 640 || (_pendingState.height != 400 && _pendingState.height != 480)))
		error |= OSystem::TransactionError::kTransactionSizeChangeFailed;

	if (error != OSystem::TransactionError::kTransactionSuccess) {
		// all our errors are fatal but engine.cpp takes only this one seriously
		error |= OSystem::TransactionError::kTransactionSizeChangeFailed;
		return static_cast<OSystem::TransactionError>(error);
	}

	_chunkySurface.init(_pendingState.width, _pendingState.height, _pendingState.width,
		_chunkySurface.getPixels(), _pendingState.format);

	_screen[FRONT_BUFFER]->reset(_pendingState.width, _pendingState.height);
	_screen[BACK_BUFFER1]->reset(_pendingState.width, _pendingState.height);
	_screen[BACK_BUFFER2]->reset(_pendingState.width, _pendingState.height);

	_workScreen = _screen[_pendingState.mode <= GraphicsMode::SingleBuffering ? FRONT_BUFFER : BACK_BUFFER1];

	// in case of resolution change from GUI
	if (_oldWorkScreen)
		_oldWorkScreen = _workScreen;

	memset(_palette, 0, sizeof(_palette));
	_pendingScreenChange = kPendingScreenChangeScreen | kPendingScreenChangePalette;

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

	memcpy(&_palette[start * 3], colors, num * 3);
	_pendingScreenChange |= kPendingScreenChangePalette;
}

void AtariGraphicsManager::grabPalette(byte *colors, uint start, uint num) const {
	//debug("grabPalette: %d, %d", start, num);

	memcpy(colors, &_palette[start * 3], num * 3);
}

void AtariGraphicsManager::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	//debug("copyRectToScreen: %d, %d, %d(%d), %d", x, y, w, pitch, h);

	if (_currentState.mode != GraphicsMode::DirectRendering) {
		_chunkySurface.copyRectToSurface(buf, pitch, x, y, w, h);
		_workScreen->addDirtyRect(Common::Rect(x, y, x + w, y + h));
	} else {
		// TODO: c2p with 16pix align
		_workScreen->surf.copyRectToSurface(buf, pitch, x, y, w, h);
		_workScreen->addDirtyRect(Common::Rect(x, y, x + w, y + h));

		updateScreen();
	}
}

Graphics::Surface *AtariGraphicsManager::lockScreen() {
	//debug("lockScreen");

	return _currentState.mode != GraphicsMode::DirectRendering ? &_chunkySurface : &_workScreen->surf;
}

void AtariGraphicsManager::unlockScreen() {
	//debug("unlockScreen: %d x %d", _workScreen->surf.w, _workScreen->surf.h);

	_workScreen->addDirtyRect(Common::Rect(_workScreen->surf.w, _workScreen->surf.h));

	if (_currentState.mode == GraphicsMode::DirectRendering)
		updateScreen();
}

void AtariGraphicsManager::fillScreen(uint32 col) {
	debug("fillScreen: %d", col);

	Graphics::Surface *screen = lockScreen();
	screen->fillRect(Common::Rect(screen->w, screen->h), col);
	unlockScreen();
}

void AtariGraphicsManager::updateScreen() {
	//debug("updateScreen");

	if (_checkUnalignedPitch) {
		const Common::ConfigManager::Domain *activeDomain = ConfMan.getActiveDomain();
		if (activeDomain) {
			// FIXME: Some engines are too bound to linear surfaces that it is very
			// hard to repair them. So instead of polluting the engine with
			// Surface::init() & delete[] Surface::getPixels() just use this hack.
			Common::String engineId = activeDomain->getValOrDefault("engineid");
			if (engineId == "parallaction"
				|| engineId == "mohawk"
				|| engineId == "sherlock"
				|| engineId == "tsage") {
				g_unalignedPitch = true;
			}
		}

		_checkUnalignedPitch = false;
	}

	// updates outOfScreen OR srcRect/dstRect (only if visible/needed)
	_cursor.update(_workScreen->surf, _workScreen->cursorPositionChanged || _workScreen->cursorSurfaceChanged);

	bool screenUpdated;

	lockSuperBlitter();

	assert(_currentState.mode >= GraphicsMode::DirectRendering && _currentState.mode <= GraphicsMode::TripleBuffering);

	if (isOverlayVisible()) {
		screenUpdated = updateBuffered(_overlaySurface, _workScreen->dirtyRects);
		assert(_workScreen == _screen[OVERLAY_BUFFER]);

		_workScreen->clearDirtyRects();
		unlockSuperBlitter();
	} else if (_currentState.mode == GraphicsMode::DirectRendering) {
		screenUpdated = updateDirect();
		assert(_workScreen == _screen[FRONT_BUFFER]);

		_workScreen->clearDirtyRects();
		unlockSuperBlitter();
	} else if (_currentState.mode == GraphicsMode::SingleBuffering) {
		screenUpdated = updateBuffered(_chunkySurface, _workScreen->dirtyRects);
		assert(_workScreen == _screen[FRONT_BUFFER]);

		_workScreen->clearDirtyRects();
		unlockSuperBlitter();
	} else {
		assert(_workScreen == _screen[BACK_BUFFER1]);

		if (_workScreen->fullRedrawPending) {
			// scheduled fullscreen redraw in this frame...
			screenUpdated = updateBuffered(_chunkySurface, _workScreen->dirtyRects);
		} else if (_screen[BACK_BUFFER2]->fullRedrawPending) {
			// scheduled fullscreen redraw in previous frame...
			screenUpdated = updateBuffered(_chunkySurface, _screen[BACK_BUFFER2]->dirtyRects);
		} else {
			screenUpdated = updateBuffered(_chunkySurface, _workScreen->dirtyRects);
			// apply dirty rects from previous frame
			if (!_screen[BACK_BUFFER2]->dirtyRects.empty())
				screenUpdated |= updateBuffered(_chunkySurface, _screen[BACK_BUFFER2]->dirtyRects);
		}

		// clear the least recent dirty rects
		_screen[BACK_BUFFER2]->clearDirtyRects();

		// render into BACK_BUFFER1 and/or BACK_BUFFER2 and set the most recent one
		if (screenUpdated) {
			_screen[FRONT_BUFFER] = _screen[BACK_BUFFER1];

			Screen *tmp = _screen[BACK_BUFFER1];
			_screen[BACK_BUFFER1] = _screen[BACK_BUFFER2];
			_screen[BACK_BUFFER2] = tmp;
		}

		// finish blitting before setting new screen address
		unlockSuperBlitter();

#ifdef SCREEN_ACTIVE
		asm_screen_set_vram(_screen[FRONT_BUFFER]->surf.getPixels());
#endif
		_workScreen = _screen[BACK_BUFFER1];
	}

#ifdef SCREEN_ACTIVE
	bool resolutionChanged = false;

	if (_pendingScreenChange & kPendingScreenChangeOverlay) {
		if (_vgaMonitor)
			asm_screen_set_scp_res(scp_640x480x8_vga);
		else
			asm_screen_set_scp_res(scp_640x480x8_rgb);

		asm_screen_set_vram(_screen[OVERLAY_BUFFER]->surf.getPixels());
		asm_screen_set_falcon_palette(_overlayPalette);
		resolutionChanged = true;
	}

	if (_pendingScreenChange & kPendingScreenChangeScreen) {
		setVidelResolution();
		asm_screen_set_vram(_screen[FRONT_BUFFER]->surf.getPixels());
		resolutionChanged = true;
	}

	if (_pendingScreenChange & kPendingScreenChangePalette) {
		asm_screen_set_falcon_palette(_palette);
	}

	_pendingScreenChange = kPendingScreenChangeNone;

	if (_oldAspectRatioCorrection != _aspectRatioCorrection) {
		if (!isOverlayVisible() && !resolutionChanged) {
			setVidelResolution();
		}
		_oldAspectRatioCorrection = _aspectRatioCorrection;
	}
#endif
	//debug("end of updateScreen");
}

void AtariGraphicsManager::setShakePos(int shakeXOffset, int shakeYOffset) {
	debug("setShakePos: %d, %d", shakeXOffset, shakeYOffset);
}

void AtariGraphicsManager::showOverlay(bool inGUI) {
	debug("showOverlay");

	if (_overlayVisible)
		return;

	_pendingScreenChange &= ~(kPendingScreenChangeScreen | kPendingScreenChangePalette);
	_pendingScreenChange |= kPendingScreenChangeOverlay;

	if (_currentState.mode == GraphicsMode::DirectRendering) {
		// make sure that _oldCursorRect is used to restore the original game graphics
		// (but only if resolution hasn't changed, see endGFXTransaction())
		bool wasVisible = showMouse(false);
		updateDirect();
		showMouse(wasVisible);
	}

	_cursor.swap();
	_oldWorkScreen = _workScreen;
	_workScreen = _screen[OVERLAY_BUFFER];

	_overlayVisible = true;
}

void AtariGraphicsManager::hideOverlay() {
	debug("hideOverlay");

	if (!_overlayVisible)
		return;

	_pendingScreenChange &= ~kPendingScreenChangeOverlay;
	_pendingScreenChange |= (kPendingScreenChangeScreen | kPendingScreenChangePalette);

	// do not cache dirtyRects and oldCursorRect
	_workScreen->reset(getOverlayWidth(), getOverlayHeight());

	_workScreen = _oldWorkScreen;
	_oldWorkScreen = nullptr;
	_cursor.swap();

	// FIXME: perhaps there's a better way but this will do for now
	_checkUnalignedPitch = true;

	_overlayVisible = false;
}

void AtariGraphicsManager::clearOverlay() {
	debug("clearOverlay");

	if (!_overlayVisible)
		return;

	const Graphics::Surface &sourceSurface =
		_currentState.mode == GraphicsMode::DirectRendering ? _screen[FRONT_BUFFER]->surf : _chunkySurface;

	int w = sourceSurface.w;
	int h = sourceSurface.h;
	int vOffset = 0;

	if (h == 200) {
		h = 240;
		vOffset = (240 - 200) / 2;
	} else if (h == 400) {
		h = 480;
		vOffset = (480 - 400) / 2;
	}

	bool upscale = false;

	if (_overlaySurface.w / w == 2 && _overlaySurface.h / h == 2) {
		upscale = true;
		vOffset *= 2;
	} else if (_overlaySurface.w / w != 1 && _overlaySurface.h / h != 1) {
		warning("Unknown overlay (%d, %d) / screen (%d, %d) ratio",
			_overlaySurface.w, _overlaySurface.h, w, h);
		return;
	}

	memset(_overlaySurface.getBasePtr(0, 0), 0, _overlaySurface.pitch * vOffset);

	// Transpose from game palette to RGB332 (overlay palette)
	const byte *src = (const byte*)sourceSurface.getPixels();
	byte *dst = (byte*)_overlaySurface.getBasePtr(0, vOffset);

	for (int y = 0; y < sourceSurface.h; y++) {
		for (int x = 0; x < sourceSurface.w; x++) {
			const byte col = *src++;
			const byte pixel = (_palette[3*col + 0] & 0xe0)
							| ((_palette[3*col + 1] >> 3) & 0x1c)
							| ((_palette[3*col + 2] >> 6) & 0x03);

			if (upscale) {
				*(dst + _overlaySurface.pitch) = pixel;
				*dst++ = pixel;
				*(dst + _overlaySurface.pitch) = pixel;
				*dst++ = pixel;
			} else {
				*dst++ = pixel;
			}
		}

		if (upscale)
			dst += _overlaySurface.pitch;
	}

	memset(_overlaySurface.getBasePtr(0, _overlaySurface.h - vOffset), 0, _overlaySurface.pitch * vOffset);

	_screen[OVERLAY_BUFFER]->addDirtyRect(Common::Rect(_screen[OVERLAY_BUFFER]->surf.w, _screen[OVERLAY_BUFFER]->surf.h));
}

void AtariGraphicsManager::grabOverlay(Graphics::Surface &surface) const {
	debug("grabOverlay: %d(%d), %d", surface.w, surface.pitch, surface.h);

	assert(surface.w >= _overlaySurface.w);
	assert(surface.h >= _overlaySurface.h);
	assert(surface.format.bytesPerPixel == _overlaySurface.format.bytesPerPixel);

	const byte *src = (const byte *)_overlaySurface.getPixels();
	byte *dst = (byte *)surface.getPixels();
	Graphics::copyBlit(dst, src, surface.pitch,
		_overlaySurface.pitch, _overlaySurface.w, _overlaySurface.h, _overlaySurface.format.bytesPerPixel);
}

void AtariGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	//debug("copyRectToOverlay: %d, %d, %d(%d), %d", x, y, w, pitch, h);

	_overlaySurface.copyRectToSurface(buf, pitch, x, y, w, h);
	_screen[OVERLAY_BUFFER]->addDirtyRect(Common::Rect(x, y, x + w, y + h));
}

bool AtariGraphicsManager::showMouse(bool visible) {
	//debug("showMouse: %d", visible);

	if (_cursor.visible == visible) {
		return visible;
	}

	bool last = _cursor.visible;
	_cursor.visible = visible;
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

	const Graphics::PixelFormat cursorFormat = format ? *format : PIXELFORMAT_CLUT8;
	_cursor.setSurface(buf, (int)w, (int)h, hotspotX, hotspotY, keycolor, cursorFormat);
	cursorSurfaceChanged();
}

void AtariGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	debug("setCursorPalette: %d, %d", start, num);

	memcpy(&_cursor.palette[start * 3], colors, num * 3);
	cursorSurfaceChanged();
}

void AtariGraphicsManager::updateMousePosition(int deltaX, int deltaY) {
	_cursor.updatePosition(deltaX, deltaY, _workScreen->surf);
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
		_screen[i] = new Screen(this, SCREEN_WIDTH, SCREEN_HEIGHT, PIXELFORMAT_CLUT8);
	}
	_screen[OVERLAY_BUFFER] = new Screen(this, getOverlayWidth(), getOverlayHeight(), getOverlayFormat());

	_chunkySurface.create(SCREEN_WIDTH, SCREEN_HEIGHT, PIXELFORMAT_CLUT8);
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

void AtariGraphicsManager::setVidelResolution() const {
	if (_vgaMonitor) {
		// TODO: aspect ratio correction
		// TODO: supervidel 320x240...
		if (_workScreen->surf.w == 320) {
			if (_workScreen->surf.h == 200)
				asm_screen_set_scp_res(scp_320x200x8_vga);
			else
				asm_screen_set_scp_res(scp_320x240x8_vga);
		} else {
			if (_workScreen->surf.h == 400)
				asm_screen_set_scp_res(scp_640x400x8_vga);
			else
				asm_screen_set_scp_res(scp_640x480x8_vga);
		}
	} else {
		if (_workScreen->surf.w == 320) {
			if (_workScreen->surf.h == 240)
				asm_screen_set_scp_res(scp_320x240x8_rgb);
			else if (_workScreen->surf.h == 200 && _aspectRatioCorrection)
				asm_screen_set_scp_res(scp_320x200x8_rgb60);
			else
				asm_screen_set_scp_res(scp_320x200x8_rgb);
		} else {
			if (_workScreen->surf.h == 480)
				asm_screen_set_scp_res(scp_640x480x8_rgb);
			else if (_workScreen->surf.h == 400 && _aspectRatioCorrection)
				asm_screen_set_scp_res(scp_640x400x8_rgb60);
			else
				asm_screen_set_scp_res(scp_640x400x8_rgb);
		}
	}
}

bool AtariGraphicsManager::updateDirect() {
	const Common::Rect &dirtyScreenRect = _workScreen->dirtyRects.empty() ? Common::Rect() : _workScreen->dirtyRects.front();
	bool &cursorPositionChanged = _workScreen->cursorPositionChanged;
	bool &cursorSurfaceChanged  = _workScreen->cursorSurfaceChanged;
	Common::Rect &oldCursorRect = _workScreen->oldCursorRect;

	bool updated = false;

	if (_cursor.outOfScreen)
		return updated;

	bool drawCursor = cursorPositionChanged || cursorSurfaceChanged;

	if (!drawCursor && _cursor.visible && !dirtyScreenRect.isEmpty())
		drawCursor = dirtyScreenRect.intersects(_cursor.dstRect);

	static Graphics::Surface cachedCursorSurface;

	if (!oldCursorRect.isEmpty() && !dirtyScreenRect.isEmpty()) {
		const Common::Rect intersectingRect = dirtyScreenRect.findIntersectingRect(oldCursorRect);
		if (!intersectingRect.isEmpty()) {
			// update cached surface
			const Graphics::Surface intersectingScreenSurface = _workScreen->surf.getSubArea(intersectingRect);
			cachedCursorSurface.copyRectToSurface(
				intersectingScreenSurface,
				intersectingRect.left - oldCursorRect.left,
				intersectingRect.top - oldCursorRect.top,
				Common::Rect(intersectingScreenSurface.w, intersectingScreenSurface.h));
		}
	}

	if ((cursorPositionChanged || !_cursor.visible) && !oldCursorRect.isEmpty()) {
		_workScreen->surf.copyRectToSurface(
			cachedCursorSurface,
			oldCursorRect.left, oldCursorRect.top,
			Common::Rect(oldCursorRect.width(), oldCursorRect.height()));

		oldCursorRect = Common::Rect();

		updated = true;
	}

	if (drawCursor && _cursor.visible) {
		//debug("Redraw cursor (direct): %d %d %d %d", _cursor.dstRect.left, _cursor.dstRect.top, _cursor.dstRect.width(), _cursor.dstRect.height());

		if (cachedCursorSurface.w != _cursor.dstRect.width() || cachedCursorSurface.h != _cursor.dstRect.height()) {
			cachedCursorSurface.create(_cursor.dstRect.width(), _cursor.dstRect.height(), _cursor.surface.format);
		}

		// background has been restored, so it's safe to read _workScreen
		if (oldCursorRect.isEmpty())
			cachedCursorSurface.copyRectToSurface(_workScreen->surf, 0, 0, _cursor.dstRect);

		_workScreen->surf.copyRectToSurfaceWithKey(
			_cursor.surface,
			_cursor.dstRect.left, _cursor.dstRect.top,
			_cursor.srcRect,
			_cursor.keycolor);

		cursorPositionChanged = cursorSurfaceChanged = false;
		oldCursorRect = _cursor.dstRect;

		updated = true;
	}

	return updated;
}

bool AtariGraphicsManager::updateBuffered(const Graphics::Surface &srcSurface, const DirtyRects &dirtyRects) {
	//debug("updateBuffered: %d", dirtyRects.size());

	// workscreen related setting; these are used even if called repeatedly for triple buffering
	Graphics::Surface &dstSurface = _workScreen->surf;
	bool &cursorPositionChanged   = _workScreen->cursorPositionChanged;
	bool &cursorSurfaceChanged    = _workScreen->cursorSurfaceChanged;
	Common::Rect &oldCursorRect   = _workScreen->oldCursorRect;

	bool updated = false;
	bool drawCursor = cursorPositionChanged || cursorSurfaceChanged;;

	for (auto it = dirtyRects.begin(); it != dirtyRects.end(); ++it) {
		if (!drawCursor && !_cursor.outOfScreen && _cursor.visible)
			drawCursor = it->intersects(_cursor.dstRect);

		copyRectToSurface(dstSurface, srcSurface, it->left, it->top, *it);

		updated = true;
	}

	if (_cursor.outOfScreen)
		return updated;

	if ((cursorPositionChanged || !_cursor.visible) && !oldCursorRect.isEmpty()) {
		alignRect(dstSurface, oldCursorRect);
		copyRectToSurface(
			dstSurface, srcSurface,
			oldCursorRect.left, oldCursorRect.top,
			oldCursorRect);

		oldCursorRect = Common::Rect();

		updated = true;
	}

	if (drawCursor && _cursor.visible) {
		//debug("Redraw cursor: %d %d %d %d", _cursor.dstRect.left, _cursor.dstRect.top, _cursor.dstRect.width(), _cursor.dstRect.height());
		copyRectToSurfaceWithKey(
			dstSurface, srcSurface, _cursor.surface,
			_cursor.dstRect.left, _cursor.dstRect.top,
			_cursor.srcRect, _cursor.keycolor, _cursor.palette);

		cursorPositionChanged = cursorSurfaceChanged = false;
		oldCursorRect = _cursor.dstRect;

		updated = true;
	}

	return updated;
}

AtariGraphicsManager::Screen::Screen(AtariGraphicsManager *manager, int width, int height, const Graphics::PixelFormat &format)
	: _manager(manager) {
	const AtariMemAlloc &allocFunc = _manager->getStRamAllocFunc();

	surf.init(width, height, (width * format.bytesPerPixel + ALIGN - 1) & (-ALIGN), nullptr, format);

	void *pixelsUnaligned = allocFunc(sizeof(uintptr) + (surf.h * surf.pitch) + ALIGN - 1);
	if (!pixelsUnaligned) {
		error("Failed to allocate memory in ST RAM");
	}

	surf.setPixels((void *)(((uintptr)pixelsUnaligned + sizeof(uintptr) + ALIGN - 1) & (-ALIGN)));

	// store the unaligned pointer for later release
	*((uintptr *)surf.getPixels() - 1) = (uintptr)pixelsUnaligned;

	memset(surf.getPixels(), 0, surf.h * surf.pitch);
}

AtariGraphicsManager::Screen::~Screen() {
	const AtariMemFree &freeFunc = _manager->getStRamFreeFunc();

	freeFunc((void *)*((uintptr *)surf.getPixels() - 1));
}

void AtariGraphicsManager::Screen::addDirtyRect(Common::Rect rect) {
	if (_fullRedraw)
		return;

	_manager->alignRect(surf, rect);

	// TODO: this assumes that screen resolution == chunky buffer resolution
	if ((rect.width() == surf.w && rect.height() == surf.h)
		|| dirtyRects.size() == dirtyRects.capacity()) {
		//debug("addDirtyRect[%d]: purge %d x %d", (int)dirtyRects.size(), surf.w, surf.h);

		dirtyRects.clear();
		dirtyRects.push_back(Common::Rect(surf.w, surf.h));

		_fullRedraw = true;
		return;
	}

	dirtyRects.push_back(rect);
}

void AtariGraphicsManager::Cursor::update(const Graphics::Surface &screen, bool isModified) {
	if (!surface.getPixels()) {
		outOfScreen = true;
		return;
	}

	if (!visible || !isModified)
		return;

	srcRect = Common::Rect(surface.w, surface.h);

	dstRect = Common::Rect(
		x - hotspotX,	// left
		y - hotspotY,	// top
		x - hotspotX + surface.w,	// right
		y - hotspotY + surface.h);	// bottom

	outOfScreen = !screen.clip(srcRect, dstRect);
}

void AtariGraphicsManager::Cursor::updatePosition(int deltaX, int deltaY, const Graphics::Surface &screen) {
	x += deltaX;
	y += deltaY;

	if (x < 0)
		x = 0;
	else if (x >= screen.w)
		x = screen.w - 1;

	if (y < 0)
		y = 0;
	else if (y >= screen.h)
		y = screen.h - 1;
}

void AtariGraphicsManager::Cursor::setSurface(const void *buf, int w, int h, int hotspotX_, int hotspotY_, uint32 keycolor_,
											  const Graphics::PixelFormat &format) {
	if (w == 0 || h == 0 || buf == nullptr) {
		if (surface.getPixels())
			surface.free();
		return;
	}

	if (surface.w != w || surface.h != h || surface.format != format)
		surface.create(w, h, format);

	surface.copyRectToSurface(buf, w * surface.format.bytesPerPixel, 0, 0, w, h);

	hotspotX = hotspotX_;
	hotspotY = hotspotY_;
	keycolor = keycolor_;
}
