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
#include "backends/graphics/atari/videl-resolutions.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "common/config-manager.h"
#include "common/str.h"
#include "common/textconsole.h"	// for warning() & error()
#include "common/translation.h"
#include "graphics/blit.h"
#include "gui/ThemeEngine.h"

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

	// make "data" the default theme path (defining DATA_PATH as "data"
	// is forbidden, it must be an absolute path, see bug #14174)
	ConfMan.registerDefault("themepath", "data");
	if (!ConfMan.hasKey("themepath"))
		ConfMan.set("themepath", "data");

	ConfMan.flushToDisk();

	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

AtariGraphicsManager::~AtariGraphicsManager() {
	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	for (int i = 0; i < SCREENS; ++i) {
		Mfree(_screen[i]);
		_screen[i] = _screenAligned[i] = nullptr;
	}

	Mfree(_overlayScreen);
	_overlayScreen = nullptr;
}

bool AtariGraphicsManager::hasFeature(OSystem::Feature f) const {
	switch (f) {
	case OSystem::Feature::kFeatureAspectRatioCorrection:
		debug("hasFeature(kFeatureAspectRatioCorrection): %d", !_vgaMonitor);
		return !_vgaMonitor;
	case OSystem::Feature::kFeatureCursorPalette:
		debug("hasFeature(kFeatureCursorPalette): %d", isOverlayVisible());
		return isOverlayVisible();
	case OSystem::Feature::kFeatureVSync:
		debug("hasFeature(kFeatureVSync): %d", _vsync);
		return true;
	default:
		return false;
	}
}

void AtariGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::Feature::kFeatureAspectRatioCorrection:
		debug("setFeatureState(kFeatureAspectRatioCorrection): %d", enable);
		_oldAspectRatioCorrection = _aspectRatioCorrection;
		_aspectRatioCorrection = enable;
		break;
	case OSystem::Feature::kFeatureVSync:
		debug("setFeatureState(kFeatureVSync): %d", enable);
		_vsync = enable;
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
		return isOverlayVisible();
	case OSystem::Feature::kFeatureVSync:
		//debug("getFeatureState(kFeatureVSync): %d", _vsync);
		return _vsync;
	default:
		return false;
	}
}

bool AtariGraphicsManager::setGraphicsMode(int mode, uint flags) {
	debug("setGraphicsMode: %d, %d", mode, flags);

	if (mode >= 0 && mode <= 3) {
		_pendingState.mode = (GraphicsMode)mode;
		return true;
	}

	return false;
}

void AtariGraphicsManager::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	debug("initSize: %d, %d, %d (vsync: %d, mode: %d)", width, height, format ? format->bytesPerPixel : 1, _vsync, (int)_pendingState.mode);

	_pendingState.width = width;
	_pendingState.height = height;
	_pendingState.format = format ? *format : PIXELFORMAT8;
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

	if (_pendingState.format != PIXELFORMAT8)
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
	_screenSurface.init(_pendingState.width, _pendingState.height, _pendingState.width,
		_screenAligned[(int)_pendingState.mode <= 1 ? FRONT_BUFFER : BACK_BUFFER1], _screenSurface.format);

	// some games do not initialize their viewport entirely
	if (_pendingState.mode != GraphicsMode::DirectRendering) {
		memset(_chunkySurface.getPixels(), 0, _chunkySurface.pitch * _chunkySurface.h);

		if (_pendingState.mode == GraphicsMode::SingleBuffering)
			handleModifiedRect(_chunkySurface, Common::Rect(_chunkySurface.w, _chunkySurface.h), _modifiedChunkyRects);
		else
			_screenModified = true;
	} else {
		memset(_screenSurface.getPixels(), 0, _screenSurface.pitch * _screenSurface.h);
	}

	memset(_palette, 0, sizeof(_palette));
	_pendingScreenChange = kPendingScreenChangeScreen | kPendingScreenChangePalette;

	static bool firstRun = true;
	if (firstRun) {
		_cursor.setPosition(getOverlayWidth() / 2, getOverlayHeight() / 2, true);
		_cursor.swap();
		firstRun = false;
	}

	// reinitialize old cursor position, there's no use for it anymore and it's dangerous
	//  to let it set to possibly bigger values then the upcoming resolution
	_oldCursorRect = Common::Rect();

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
	//debug("copyRectToScreen: %d, %d, %d, %d, %d", pitch, x, y, w, h);

	if (_currentState.mode != GraphicsMode::DirectRendering) {
		_chunkySurface.copyRectToSurface(buf, pitch, x, y, w, h);

		if (_currentState.mode == GraphicsMode::SingleBuffering)
			handleModifiedRect(_chunkySurface, Common::Rect(x, y, x + w, y + h), _modifiedChunkyRects);
		else
			_screenModified = true;
	} else {
		// TODO: c2p with 16pix align
		_screenSurface.copyRectToSurface(buf, pitch, x, y, w, h);

		_modifiedScreenRect = Common::Rect(x, y, x + w, y + h);

		bool vsync = _vsync;
		_vsync = false;
		updateScreen();
		_vsync = vsync;
	}
}

Graphics::Surface *AtariGraphicsManager::lockScreen() {
	//debug("lockScreen");

	return _currentState.mode != GraphicsMode::DirectRendering ? &_chunkySurface : &_screenSurface;
}

void AtariGraphicsManager::unlockScreen() {
	if (_currentState.mode != GraphicsMode::DirectRendering) {
		if (_currentState.mode == GraphicsMode::SingleBuffering)
			handleModifiedRect(_chunkySurface, Common::Rect(_chunkySurface.w, _chunkySurface.h), _modifiedChunkyRects);
		else
			_screenModified = true;
	} else {
		_modifiedScreenRect = Common::Rect(_screenSurface.w, _screenSurface.h);

		bool vsync = _vsync;
		_vsync = false;
		updateScreen();
		_vsync = vsync;
	}
}

void AtariGraphicsManager::fillScreen(uint32 col) {
	debug("fillScreen: %d", col);

	if (_currentState.mode != GraphicsMode::DirectRendering) {
		const Common::Rect rect = Common::Rect(_chunkySurface.w, _chunkySurface.h);
		_chunkySurface.fillRect(rect, col);

		if (_currentState.mode == GraphicsMode::SingleBuffering)
			handleModifiedRect(_chunkySurface, rect, _modifiedChunkyRects);
		else
			_screenModified = true;
	} else {
		const Common::Rect rect = Common::Rect(_screenSurface.w, _screenSurface.h);
		_screenSurface.fillRect(rect, col);
	}
}

void AtariGraphicsManager::updateScreen() {
	//debug("updateScreen");

	// updates outOfScreen OR srcRect/dstRect (only if visible/needed)
	_cursor.update(isOverlayVisible() ? _screenOverlaySurface : _screenSurface);

	bool screenUpdated = false;

	if (isOverlayVisible()) {
		screenUpdated = updateOverlay();
	} else {
		switch(_currentState.mode) {
		case GraphicsMode::DirectRendering:
			screenUpdated = updateDirectBuffer();
			break;
		case GraphicsMode::SingleBuffering:
			screenUpdated = updateSingleBuffer();
			break;
		case GraphicsMode::DoubleBuffering:
		case GraphicsMode::TripleBuffering:
			screenUpdated = updateDoubleAndTripleBuffer();
			break;
		}
	}

	//if (_cursor.outOfScreen)
	//	warning("mouse out of screen");

	bool vsync = _vsync;

	if (_screenModified) {
		sync();

		if (_currentState.mode == GraphicsMode::DoubleBuffering) {
			byte *tmp = _screenAligned[FRONT_BUFFER];
			_screenAligned[FRONT_BUFFER] = _screenAligned[BACK_BUFFER1];
			_screenAligned[BACK_BUFFER1] = tmp;

			// always wait for vbl
			vsync = true;
		} else if (_currentState.mode == GraphicsMode::TripleBuffering) {
			if (vsync) {
				// render into BACK_BUFFER1 and/or BACK_BUFFER2 and set the most recent one
				_screenAligned[FRONT_BUFFER] = _screenAligned[BACK_BUFFER1];

				byte *tmp = _screenAligned[BACK_BUFFER1];
				_screenAligned[BACK_BUFFER1] = _screenAligned[BACK_BUFFER2];
				_screenAligned[BACK_BUFFER2] = tmp;
			} else {
				// render into BACK_BUFFER1 and/or BACK_BUFFER2 and/or FRONT_BUFFER
				byte *tmp = _screenAligned[FRONT_BUFFER];
				_screenAligned[FRONT_BUFFER] = _screenAligned[BACK_BUFFER1];
				_screenAligned[BACK_BUFFER1] = _screenAligned[BACK_BUFFER2];
				_screenAligned[BACK_BUFFER2] = tmp;
			}

			// never wait for vbl (use it only as a flag for the modes above)
			vsync = false;
		}

#ifdef SCREEN_ACTIVE
		asm_screen_set_vram(_screenAligned[FRONT_BUFFER]);
#endif
		_screenSurface.setPixels(_screenAligned[BACK_BUFFER1]);
		_screenModified = false;
	}

	// everything below this line is done in VBL so don't wait if nothing has been updated!
	vsync &= screenUpdated;

#ifdef SCREEN_ACTIVE
	bool resolutionChanged = false;

	if (_pendingScreenChange & kPendingScreenChangeOverlay) {
		if (_vgaMonitor) {
			if (getOverlayWidth() == 640 && getOverlayHeight() == 480)
				asm_screen_set_scp_res(scp_640x480x16_vga);
			else
				asm_screen_set_scp_res(scp_320x240x16_vga);
		} else {
			//asm_screen_set_scp_res(scp_320x240x16_rgb);
			asm_screen_set_scp_res(scp_640x480x16_rgb);
		}

		asm_screen_set_vram(_screenOverlaySurface.getPixels());
		resolutionChanged = true;
	}

	if (_pendingScreenChange & kPendingScreenChangeScreen) {
		setVidelResolution();
		asm_screen_set_vram(_screenAligned[FRONT_BUFFER]);
		resolutionChanged = true;
	}

	if ((_pendingScreenChange & kPendingScreenChangePalette) && !isOverlayVisible()) {
		static uint falconPalette[256];

		for (uint i = 0; i < 256; ++i) {
			// RRRRRRRR GGGGGGGG BBBBBBBB -> RRRRRRrr GGGGGGgg 00000000 BBBBBBbb
			falconPalette[i] = (_palette[i * 3 + 0] << 24) | (_palette[i * 3 + 1] << 16) | _palette[i * 3 + 2];
		}
#ifdef SCREEN_ACTIVE
		asm_screen_set_falcon_palette(falconPalette);
#endif
	}

	_pendingScreenChange = kPendingScreenChangeNone;

	if (_oldAspectRatioCorrection != _aspectRatioCorrection) {
		if (!isOverlayVisible() && !resolutionChanged) {
			setVidelResolution();
		}
		_oldAspectRatioCorrection = _aspectRatioCorrection;
	}

	if (vsync)
		waitForVbl();
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

	_pendingScreenChange &= ~kPendingScreenChangeScreen;
	_pendingScreenChange |= kPendingScreenChangeOverlay;

	_cursor.swap();
	if (_currentState.mode == GraphicsMode::DirectRendering) {
		// make sure that _oldCursorRect is used to restore the original game graphics
		// (but only if resolution hasn't changed, see endGFXTransaction())
		bool wasVisible = showMouse(false);
		updateDirectBuffer();
		showMouse(wasVisible);
	}

	_overlayVisible = true;
}

void AtariGraphicsManager::hideOverlay() {
	debug("hideOverlay");

	if (!_overlayVisible)
		return;

	_pendingScreenChange &= ~kPendingScreenChangeOverlay;
	_pendingScreenChange |= kPendingScreenChangeScreen;

	_cursor.swap();
	// don't fool game cursor logic (especially direct rendering)
	// (the overlay doesn't need any restoration upon re-entering)
	_oldCursorRect = Common::Rect();

	_overlayVisible = false;
}

void AtariGraphicsManager::clearOverlay() {
	debug("clearOverlay");

	if (!_overlayVisible)
		return;

	const Graphics::Surface &sourceSurface = _currentState.mode == GraphicsMode::DirectRendering ? _screenSurface : _chunkySurface;

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

	ScaleMode scaleMode;
	if (w == _overlaySurface.w && h == _overlaySurface.h) {
		scaleMode = ScaleMode::NONE;
	} else if (w / _overlaySurface.w == 2 && h / _overlaySurface.h == 2) {
		scaleMode = ScaleMode::DOWNSCALE;
		vOffset /= 2;
	} else if (_overlaySurface.w / w == 2 && _overlaySurface.h / h == 2) {
		scaleMode = ScaleMode::UPSCALE;
		vOffset *= 2;
	} else {
		warning("Unknown overlay (%d, %d) / screen (%d, %d) ratio: ",
			_overlaySurface.w, _overlaySurface.h, w, h);
		return;
	}

	memset(_overlaySurface.getBasePtr(0, 0), 0, _overlaySurface.pitch * vOffset);
	copySurface8ToSurface16(
		sourceSurface,
		_palette,
		_overlaySurface,
		0, vOffset,
		Common::Rect(sourceSurface.w, sourceSurface.h),
		scaleMode);
	memset(_overlaySurface.getBasePtr(0, _overlaySurface.h - vOffset), 0, _overlaySurface.pitch * vOffset);

	handleModifiedRect(_overlaySurface, Common::Rect(_overlaySurface.w, _overlaySurface.h), _modifiedOverlayRects);
}

void AtariGraphicsManager::grabOverlay(Graphics::Surface &surface) const {
	debug("grabOverlay: %d, %d, %d", surface.pitch, surface.w, surface.h);

	memcpy(surface.getPixels(), _overlaySurface.getPixels(), surface.pitch * surface.h);
}

void AtariGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	//debug("copyRectToOverlay: %d, %d, %d, %d, %d", pitch, x, y, w, h);

	_overlaySurface.copyRectToSurface(buf, pitch, x, y, w, h);

	handleModifiedRect(_overlaySurface, Common::Rect(x, y, x + w, y + h), _modifiedOverlayRects);
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

	_cursor.setPosition(x, y, true);
}

void AtariGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor,
										  bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	//debug("setMouseCursor: %d, %d, %d, %d, %d, %d", w, h, hotspotX, hotspotY, keycolor, format ? format->bytesPerPixel : 1);

	const Graphics::PixelFormat cursorFormat = format ? *format : PIXELFORMAT8;
	_cursor.setSurface(buf, (int)w, (int)h, hotspotX, hotspotY, keycolor, cursorFormat);
}

void AtariGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	debug("setCursorPalette: %d, %d", start, num);

	memcpy(&_cursor.palette[start * 3], colors, num * 3);
	_cursor.surfaceChanged = true;
}

void AtariGraphicsManager::updateMousePosition(int deltaX, int deltaY) {
	_cursor.updatePosition(deltaX, deltaY, isOverlayVisible() ? _screenOverlaySurface : _screenSurface);
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

bool AtariGraphicsManager::allocateAtariSurface(
		byte *&buf, Graphics::Surface &surface, int width, int height,
		const Graphics::PixelFormat &format, int mode, uintptr mask) {
	buf = (mode == MX_STRAM)
		? (byte*)Mxalloc(width * height * format.bytesPerPixel + 15, mode)
		: (byte*)allocFast(width * height * format.bytesPerPixel + 15);

	if (!buf)
		return false;

	byte *bufAligned = (byte*)((((uintptr)buf + 15) | mask) & 0xfffffff0);
	memset(bufAligned, 0, width * height * format.bytesPerPixel);

	surface.init(width, height, width * format.bytesPerPixel, bufAligned, format);
	return true;
}

void AtariGraphicsManager::setVidelResolution() const {
	if (_vgaMonitor) {
		// TODO: aspect ratio correction
		// TODO: supervidel 320x240...
		if (_screenSurface.w == 320) {
			if (_screenSurface.h == 200)
				asm_screen_set_scp_res(scp_320x200x8_vga);
			else
				asm_screen_set_scp_res(scp_320x240x8_vga);
		} else {
			if (_screenSurface.h == 400)
				asm_screen_set_scp_res(scp_640x400x8_vga);
			else
				asm_screen_set_scp_res(scp_640x480x8_vga);
		}
	} else {
		if (_screenSurface.w == 320) {
			if (_screenSurface.h == 240)
				asm_screen_set_scp_res(scp_320x240x8_rgb);
			else if (_screenSurface.h == 200 && _aspectRatioCorrection)
				asm_screen_set_scp_res(scp_320x200x8_rgb60);
			else
				asm_screen_set_scp_res(scp_320x200x8_rgb);
		} else {
			if (_screenSurface.h == 480)
				asm_screen_set_scp_res(scp_640x480x8_rgb);
			else if (_screenSurface.h == 400 && _aspectRatioCorrection)
				asm_screen_set_scp_res(scp_640x400x8_rgb60);
			else
				asm_screen_set_scp_res(scp_640x400x8_rgb);
		}
	}
}

void AtariGraphicsManager::waitForVbl() const {
	extern volatile uint32 vbl_counter;
	uint32 counter = vbl_counter;

	while (counter == vbl_counter);
}

bool AtariGraphicsManager::updateOverlay() {
	bool updated = false;
	bool drawCursor = _cursor.isModified();

	while (!_modifiedOverlayRects.empty()) {
		const Common::Rect &rect = _modifiedOverlayRects.back();

		if (!drawCursor && !_cursor.outOfScreen && _cursor.visible)
			drawCursor = rect.intersects(_cursor.dstRect);

		_screenOverlaySurface.copyRectToSurface(_overlaySurface, rect.left, rect.top, rect);

		_modifiedOverlayRects.pop_back();

		updated = true;
	}

	if (_cursor.outOfScreen)
		return updated;

	if ((_cursor.positionChanged || !_cursor.visible) && !_oldCursorRect.isEmpty()) {
		_screenOverlaySurface.copyRectToSurface(_overlaySurface, _oldCursorRect.left, _oldCursorRect.top, _oldCursorRect);
		_oldCursorRect = Common::Rect();

		updated = true;
	}

	if (drawCursor && _cursor.visible) {
		//debug("Redraw cursor (overlay): %d %d %d %d", _cursor.dstRect.left, _cursor.dstRect.top, _cursor.dstRect.width(), _cursor.dstRect.height());

		copySurface8ToSurface16WithKey(
			_cursor.surface,
			_cursor.palette,
			_screenOverlaySurface,
			_cursor.dstRect.left, _cursor.dstRect.top,
			_cursor.srcRect,
			_cursor.keycolor);

		_cursor.positionChanged = _cursor.surfaceChanged = false;
		_oldCursorRect = _cursor.dstRect;

		updated = true;
	}

	return updated;
}

bool AtariGraphicsManager::updateDirectBuffer() {
	bool updated = false;

	if (_cursor.outOfScreen)
		return updated;

	bool drawCursor = _cursor.isModified();

	if (!drawCursor && _cursor.visible && !_modifiedScreenRect.isEmpty())
		drawCursor = _modifiedScreenRect.intersects(_cursor.dstRect);

	static Graphics::Surface cachedCursorSurface;

	if (!_oldCursorRect.isEmpty() && !_modifiedScreenRect.isEmpty()) {
		const Common::Rect intersectingRect = _modifiedScreenRect.findIntersectingRect(_oldCursorRect);
		if (!intersectingRect.isEmpty()) {
			// update cached surface
			const Graphics::Surface intersectingScreenSurface = _screenSurface.getSubArea(intersectingRect);
			cachedCursorSurface.copyRectToSurface(
				intersectingScreenSurface,
				intersectingRect.left - _oldCursorRect.left,
				intersectingRect.top - _oldCursorRect.top,
				Common::Rect(intersectingScreenSurface.w, intersectingScreenSurface.h));
		}
	}

	_modifiedScreenRect = Common::Rect();

	if ((_cursor.positionChanged || !_cursor.visible) && !_oldCursorRect.isEmpty()) {
		_screenSurface.copyRectToSurface(
			cachedCursorSurface,
			_oldCursorRect.left, _oldCursorRect.top,
			Common::Rect(_oldCursorRect.width(), _oldCursorRect.height()));

		_oldCursorRect = Common::Rect();

		updated = true;
	}

	if (drawCursor && _cursor.visible) {
		//debug("Redraw cursor (direct): %d %d %d %d", _cursor.dstRect.left, _cursor.dstRect.top, _cursor.dstRect.width(), _cursor.dstRect.height());

		if (cachedCursorSurface.w != _cursor.dstRect.width() || cachedCursorSurface.h != _cursor.dstRect.height()) {
			cachedCursorSurface.create(_cursor.dstRect.width(), _cursor.dstRect.height(), _cursor.surface.format);
		}

		// background has been restored, so it's safe to read _screenSurface
		if (_oldCursorRect.isEmpty())
			cachedCursorSurface.copyRectToSurface(_screenSurface, 0, 0, _cursor.dstRect);

		_screenSurface.copyRectToSurfaceWithKey(
			_cursor.surface,
			_cursor.dstRect.left, _cursor.dstRect.top,
			_cursor.srcRect,
			_cursor.keycolor);

		_cursor.positionChanged = _cursor.surfaceChanged = false;
		_oldCursorRect = _cursor.dstRect;

		updated = true;
	}

	return updated;
}

bool AtariGraphicsManager::updateSingleBuffer() {
	bool updated = false;
	bool drawCursor = _cursor.isModified();

	while (!_modifiedChunkyRects.empty()) {
		const Common::Rect &rect = _modifiedChunkyRects.back();

		if (!drawCursor && !_cursor.outOfScreen && _cursor.visible)
			drawCursor = rect.intersects(_cursor.dstRect);

		if (rect.width() == _screenSurface.w && rect.height() == _screenSurface.h)
			copySurfaceToSurface(_chunkySurface, _screenSurface);
		else
			copyRectToSurface(_chunkySurface, rect.left, rect.top, _screenSurface, rect);

		_modifiedChunkyRects.pop_back();

		updated = true;
	}

	if (_cursor.outOfScreen)
		return updated;

	if ((_cursor.positionChanged || !_cursor.visible) && !_oldCursorRect.isEmpty()) {
		alignRect(_chunkySurface, _oldCursorRect);
		copyRectToSurface(_chunkySurface, _oldCursorRect.left, _oldCursorRect.top,
			_screenSurface, _oldCursorRect);

		_oldCursorRect = Common::Rect();

		updated = true;
	}

	if (drawCursor && _cursor.visible) {
		//debug("Redraw cursor (single): %d %d %d %d", _cursor.dstRect.left, _cursor.dstRect.top, _cursor.dstRect.width(), _cursor.dstRect.height());
		copyRectToSurfaceWithKey(_cursor.surface, _cursor.dstRect.left, _cursor.dstRect.top,
			_screenSurface, _cursor.srcRect, _cursor.keycolor);

		_cursor.positionChanged = _cursor.surfaceChanged = false;
		_oldCursorRect = _cursor.dstRect;

		updated = true;
	}

	return updated;
}

bool AtariGraphicsManager::updateDoubleAndTripleBuffer() {
	bool updated = false;
	bool drawCursor = _cursor.isModified();

	if (_screenModified) {
		drawCursor = true;

		copySurfaceToSurface(_chunkySurface, _screenSurface);

		// updated in screen swapping
		//_screenModified = false;
		updated = true;
	}

	if (_cursor.outOfScreen)
		return updated;

	// render directly to the screen to be swapped (so we don't have to refresh full screen when only cursor moves)
	Graphics::Surface frontBufferScreenSurface;
	frontBufferScreenSurface.init(_screenSurface.w, _screenSurface.h, _screenSurface.pitch,
		_screenAligned[_screenModified ? BACK_BUFFER1 : FRONT_BUFFER], _screenSurface.format);

	if ((_cursor.positionChanged || !_cursor.visible) && !_oldCursorRect.isEmpty() && !_screenModified) {
		alignRect(_chunkySurface, _oldCursorRect);
		copyRectToSurface(_chunkySurface, _oldCursorRect.left, _oldCursorRect.top,
			frontBufferScreenSurface, _oldCursorRect);

		_oldCursorRect = Common::Rect();

		updated = true;
	}

	if (drawCursor && _cursor.visible) {
		//debug("Redraw cursor (double/triple): %d %d %d %d", _cursor.dstRect.left, _cursor.dstRect.top, _cursor.dstRect.width(), _cursor.dstRect.height());

		copyRectToSurfaceWithKey(_cursor.surface, _cursor.dstRect.left, _cursor.dstRect.top,
			frontBufferScreenSurface, _cursor.srcRect, _cursor.keycolor);

		_cursor.positionChanged = _cursor.surfaceChanged = false;
		_oldCursorRect = _cursor.dstRect;

		updated = true;
	}

	return updated;
}

void AtariGraphicsManager::copySurface8ToSurface16(
		const Graphics::Surface &srcSurface, const byte *srcPalette,
		Graphics::Surface &dstSurface, int destX, int destY,
		const Common::Rect subRect, ScaleMode scaleMode) const {
	assert(srcSurface.format.bytesPerPixel == 1);
	assert(dstSurface.format.bytesPerPixel == 2);

	// faster (no memory (re-)allocation) version of Surface::convertTo()
	const int w = subRect.width();
	const int h = subRect.height();

	const int srcScale = scaleMode == ScaleMode::DOWNSCALE ? 2 : 1;
	const byte *srcRow = (const byte*)srcSurface.getBasePtr(subRect.left * srcScale, subRect.top * srcScale);
	uint16 *dstRow = (uint16*)dstSurface.getBasePtr(destX, destY);	// already upscaled if needed

	static uint32 srcPaletteMap[256];
	Graphics::convertPaletteToMap(srcPaletteMap, srcPalette, 256, dstSurface.format);

	// optimized paths for each case...
	if (scaleMode == ScaleMode::NONE) {
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				*dstRow++ = srcPaletteMap[*srcRow++];
			}

			srcRow += srcSurface.w - w;
			dstRow += dstSurface.w - w;
		}
	} else if (scaleMode == ScaleMode::DOWNSCALE) {
		for (int y = 0; y < h / 2; y++) {
			for (int x = 0; x < w / 2; x++) {
				*dstRow++ = srcPaletteMap[*srcRow];
				srcRow += 2;
			}

			srcRow += srcSurface.w - w + srcSurface.w;
			dstRow += dstSurface.w - w / 2;
		}
	} else if (scaleMode == ScaleMode::UPSCALE) {
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				const uint16 pixel = srcPaletteMap[*srcRow++];

				*(dstRow + dstSurface.w) = pixel;
				*dstRow++ = pixel;
				*(dstRow + dstSurface.w) = pixel;
				*dstRow++ = pixel;
			}

			srcRow += srcSurface.w - w;
			dstRow += dstSurface.w - w * 2 + dstSurface.w;
		}
	}
}

void AtariGraphicsManager::copySurface8ToSurface16WithKey(
		const Graphics::Surface &srcSurface, const byte *srcPalette,
		Graphics::Surface &dstSurface, int destX, int destY,
		const Common::Rect subRect, uint32 key) const {
	assert(srcSurface.format.bytesPerPixel == 1);
	assert(dstSurface.format.bytesPerPixel == 2);

	// faster (no memory (re-)allocation) version of Surface::convertTo()
	const int w = subRect.width();
	const int h = subRect.height();

	const byte *srcRow = (const byte *)srcSurface.getBasePtr(subRect.left, subRect.top);
	uint16 *dstRow = (uint16 *)dstSurface.getBasePtr(destX, destY);

	static uint32 srcPaletteMap[256];
	Graphics::convertPaletteToMap(srcPaletteMap, srcPalette, 256, dstSurface.format);

	const uint16 keyColor = srcPaletteMap[key];

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			const uint16 pixel = srcPaletteMap[*srcRow++];

			if (pixel != keyColor) {
				*dstRow++ = pixel;
			} else {
				dstRow++;
			}
		}

		srcRow += srcSurface.w - w;
		dstRow += dstSurface.w - w;
	}
}

void AtariGraphicsManager::handleModifiedRect(
		const Graphics::Surface &surface,
		Common::Rect rect, Common::Array<Common::Rect> &rects) const {
	if (_currentState.mode == GraphicsMode::SingleBuffering)
		alignRect(surface, rect);

	if (rect.width() == surface.w && rect.height() == surface.h) {
		//debug("handleModifiedRect: purge");

		rects.clear();
		rects.push_back(rect);
		return;
	}

	for (const Common::Rect &r : rects) {
		if (r.contains(rect))
			return;
	}

	rects.push_back(rect);
}

void AtariGraphicsManager::Cursor::update(const Graphics::Surface &screen) {
	if (!surface.getPixels()) {
		outOfScreen = true;
		return;
	}

	if (!visible || (!surfaceChanged && !positionChanged))
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
	if (!visible)
		return;

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

	positionChanged = true;
}

void AtariGraphicsManager::Cursor::setSurface(const void *buf, int w, int h, int _hotspotX, int _hotspotY, uint32 _keycolor, const Graphics::PixelFormat &format) {
	if (w == 0 || h == 0 || buf == nullptr) {
		if (surface.getPixels())
			surface.free();
		return;
	}

	if (surface.w != w || surface.h != h || surface.format != format)
		surface.create(w, h, format);

	surface.copyRectToSurface(buf, w * format.bytesPerPixel, 0, 0, w, h);

	hotspotX = _hotspotX;
	hotspotY = _hotspotY;
	keycolor = _keycolor;

	surfaceChanged = true;
}
