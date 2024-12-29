/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
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

#include "graphics/colormasks.h"
#include "graphics/palette.h"
#include "graphics/managed_surface.h"

#include "gui/message.h"
#include "gui/gui-manager.h"

#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "backends/platform/libretro/include/libretro-graphics-surface.h"

LibretroGraphics::LibretroGraphics() : _cursorPaletteEnabled(false),
	_cursorKeyColor(0),
	_cursorDontScale(false),
	_screenUpdatePending(false),
	_gamePalette(256),
	_cursorPalette(256),
	_screenChangeID(1 << (sizeof(int) * 8 - 2)) {}

LibretroGraphics::~LibretroGraphics() {
	_gameScreen.free();
	_overlay.free();
	_cursor.free();
	_screen.free();
}

Common::List<Graphics::PixelFormat> LibretroGraphics::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> result;

#ifdef SCUMM_LITTLE_ENDIAN
	/* RGBA8888 */
	result.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#else
	/* ABGR8888 */
	result.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#endif
	/* RGB565 - overlay */
	result.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	/* RGB555 - fmtowns */
	result.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15));

	/* Palette - most games */
	result.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return result;
}

const OSystem::GraphicsMode *LibretroGraphics::getSupportedGraphicsModes() const {
	static const OSystem::GraphicsMode s_noGraphicsModes[] = {{0, 0, 0}};
	return s_noGraphicsModes;
}

void LibretroGraphics::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	Graphics::PixelFormat actFormat = format ? *format : Graphics::PixelFormat::createFormatCLUT8();
	bool force_gui_redraw = false;
	/* Override for ScummVM Launcher */
	if (nullptr == ConfMan.getActiveDomain()) {
		/* 0 w/h is used to notify libretro gui res settings is changed */
		force_gui_redraw = (width == 0);
		width = retro_setting_get_gui_res_w();
		height = retro_setting_get_gui_res_h();
	}
	/* no need to update now libretro gui res settings changes if not in ScummVM launcher */
	if (! width)
		return;

	if (_gameScreen.w != width || _gameScreen.h != height || _gameScreen.format != actFormat)
		_gameScreen.create(width, height, actFormat);

	if (_overlay.w != width || _overlay.h != height)
		_overlay.create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	if (getWindowWidth() != width || getWindowHeight() != height)
		_screen.create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	handleResize(width, height);
	recalculateDisplayAreas();
	retro_set_size(width, height);
	LIBRETRO_G_SYSTEM->refreshRetroSettings();
	++_screenChangeID;

	if (force_gui_redraw)
		g_gui.checkScreenChange();
}

int16 LibretroGraphics::getHeight() const {
	return _gameScreen.h;
}

int16 LibretroGraphics::getWidth() const {
	return _gameScreen.w;
}

Graphics::PixelFormat LibretroGraphics::getScreenFormat() const {
	return _gameScreen.format;
}

void LibretroGraphics::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	_gameScreen.copyRectToSurface(buf, pitch, x, y, w, h);
}

void LibretroGraphics::updateScreen() {
	_screenUpdatePending = true;
	dynamic_cast<LibretroTimerManager *>(LIBRETRO_G_SYSTEM->getTimerManager())->checkThread(THREAD_SWITCH_UPDATE);
}

void LibretroGraphics::realUpdateScreen(void) {
	const Graphics::Surface &srcSurface = _overlayVisible ? _overlay : _gameScreen;

	if (srcSurface.w && srcSurface.h)
		_screen.blitFrom(srcSurface, Common::Rect(srcSurface.w, srcSurface.h), Common::Rect(_screen.w, _screen.h), &_gamePalette);

	if (_cursorVisible && _cursor.w && _cursor.h) {
		Common::Point topLeft(_cursorX - _cursorHotspotXScaled, _cursorY - _cursorHotspotYScaled);
		_screen.transBlitFrom(_cursor, Common::Rect(_cursor.w, _cursor.h), Common::Rect(topLeft, topLeft + Common::Point(_cursorWidthScaled, _cursorHeightScaled)),  _cursorKeyColor, false, 0xff, nullptr, false,  _cursorPaletteEnabled ? &_cursorPalette : &_gamePalette);
	}
	_screenUpdatePending = false;
}

void LibretroGraphics::clearOverlay() {
	_overlay.fillRect(Common::Rect(_overlay.w, _overlay.h), 0);
}

void LibretroGraphics::grabOverlay(Graphics::Surface &surface) const {
	surface.copyFrom(_overlay);
}

void LibretroGraphics::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_overlay.copyRectToSurface(buf, pitch, x, y, w, h);
}

int16 LibretroGraphics::getOverlayHeight() const {
	return _overlay.h;
}

int16 LibretroGraphics::getOverlayWidth() const {
	return _overlay.w;
}

Graphics::PixelFormat LibretroGraphics::getOverlayFormat() const {
	return _overlay.format;
}

void LibretroGraphics::warpMouse(int x, int y) {
	LIBRETRO_G_SYSTEM->_mouseX = x;
	LIBRETRO_G_SYSTEM->_mouseY = y;
	WindowedGraphicsManager::warpMouse(x, y);
}

void LibretroGraphics::overrideCursorScaling() {
	const frac_t screenScaleFactor = (_cursorDontScale || ! _overlayVisible) ? intToFrac(1) : intToFrac(getWindowHeight()) / 200; /* hard coded as base resolution 320x200 is hard coded upstream */

	_cursorHotspotXScaled = fracToInt(_cursorHotspotX * screenScaleFactor);
	_cursorWidthScaled    = fracToDouble(_cursor.w * screenScaleFactor);

	_cursorHotspotYScaled = fracToInt(_cursorHotspotY * screenScaleFactor);
	_cursorHeightScaled   = fracToDouble(_cursor.h * screenScaleFactor);
}

void LibretroGraphics::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	if (!buf || !w || !h)
		return;

	const Graphics::PixelFormat mformat = format ? *format : Graphics::PixelFormat::createFormatCLUT8();

	if (_cursor.w != w || _cursor.h != h || _cursor.format != mformat)
		_cursor.create(w, h, mformat);

	_cursor.copyRectToSurface(buf, _cursor.pitch, 0, 0, w, h);

	_cursorHotspotX = hotspotX;
	_cursorHotspotY = hotspotY;
	_cursorKeyColor = keycolor;
	_cursorDontScale = dontScale;

	overrideCursorScaling();
}


OSystem::TransactionError LibretroGraphics::endGFXTransaction() {
	overrideCursorScaling();
	return OSystem::TransactionError::kTransactionSuccess;
}

void LibretroGraphics::handleResizeImpl(const int width, const int height) {
	overrideCursorScaling();
}

void LibretroGraphics::setCursorPalette(const byte *colors, uint start, uint num) {
	_cursorPalette.set(colors, start, num);
	_cursorPaletteEnabled = true;
}

bool LibretroGraphics::isOverlayInGUI(void) {
	return _overlayInGUI;
}

const Graphics::ManagedSurface *LibretroGraphics::getScreen() {
	return &_screen;
}

void LibretroGraphics::setPalette(const byte *colors, uint start, uint num) {
	_gamePalette.set(colors, start, num);
}

void LibretroGraphics::grabPalette(byte *colors, uint start, uint num) const {
	_gamePalette.grab(colors, start, num);
}

bool LibretroGraphics::hasFeature(OSystem::Feature f) const {
	return (f == OSystem::kFeatureCursorPalette) ||
#ifdef SCUMMVM_NEON
	       (f == OSystem::kFeatureCpuNEON) ||
#endif
	       (f == OSystem::kFeatureCursorAlpha);
}

void LibretroGraphics::setFeatureState(OSystem::Feature f, bool enable) {
	if (f == OSystem::kFeatureCursorPalette)
		_cursorPaletteEnabled = enable;
}

bool LibretroGraphics::getFeatureState(OSystem::Feature f) const {
	return (f == OSystem::kFeatureCursorPalette) ? _cursorPaletteEnabled : false;
}

void LibretroGraphics::setMousePosition(int x, int y) {
	WindowedGraphicsManager::setMousePosition(x, y);
}

void LibretroGraphics::displayMessageOnOSD(const Common::U32String &msg) {
	// Display the message for 3 seconds
	GUI::TimedMessageDialog dialog(msg, 3000);
	dialog.runModal();
}

int LibretroGraphics::getScreenChangeID() const {
	return _screenChangeID;
}
