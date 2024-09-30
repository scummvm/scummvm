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

#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "backends/platform/libretro/include/libretro-graphics-surface.h"

LibretroGraphics::LibretroGraphics() : _mousePaletteEnabled(false),
	_mouseVisible(false),
	_mouseKeyColor(0),
	_mouseDontScale(false),
	_screenUpdatePending(false),
	_gamePalette(256),
	_mousePalette(256),
	_screenChangeID(1 << (sizeof(int) * 8 - 2)){}

LibretroGraphics::~LibretroGraphics() {
	_gameScreen.free();
	_overlay.free();
	_mouseImage.free();
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

	/* Override for ScummVM Launcher */
	if (nullptr == ConfMan.getActiveDomain()){
		width = RES_W_OVERLAY;
		height = RES_H_OVERLAY;
	}

	if (_gameScreen.w != width || _gameScreen.h != height || _gameScreen.format != actFormat)
		_gameScreen.create(width, height, actFormat);

	if (_overlay.w != width || _overlay.h != height)
		_overlay.create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	if (getWindowWidth() != width || getWindowHeight() != height) {
		_screen.create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
		handleResize(width, height);
		recalculateDisplayAreas();
		retro_set_size(width, height);
		LIBRETRO_G_SYSTEM->refreshRetroSettings();
		++_screenChangeID;
	}
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
	if (! retro_setting_get_timing_inaccuracies_enabled() && !_overlayInGUI)
		dynamic_cast<LibretroTimerManager *>(LIBRETRO_G_SYSTEM->getTimerManager())->checkThread(THREAD_SWITCH_UPDATE);
}

void LibretroGraphics::realUpdateScreen(void) {
	const Graphics::Surface &srcSurface = (_overlayInGUI) ? _overlay : _gameScreen;

	if (srcSurface.w && srcSurface.h)
		_screen.blitFrom(srcSurface, Common::Rect(srcSurface.w,srcSurface.h),Common::Rect(_screen.w,_screen.h),&_gamePalette);

	if (_mouseVisible && _mouseImage.w && _mouseImage.h)
		_screen.transBlitFrom(_mouseImage, Common::Point(_cursorX - _mouseHotspotX, _cursorY - _mouseHotspotY), _mouseKeyColor, false, 0, 0xff, _mousePaletteEnabled ? &_mousePalette : &_gamePalette);

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

bool LibretroGraphics::showMouse(bool visible) {
	const bool wasVisible = _mouseVisible;
	_mouseVisible = visible;
	return wasVisible;
}

void LibretroGraphics::warpMouse(int x, int y) {
	LIBRETRO_G_SYSTEM->_mouseX = x;
	LIBRETRO_G_SYSTEM->_mouseY = y;
	WindowedGraphicsManager::warpMouse(x, y);
}

void LibretroGraphics::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	if (!buf || !w || !h)
		return;

	const Graphics::PixelFormat mformat = format ? *format : Graphics::PixelFormat::createFormatCLUT8();

	if (_mouseImage.w != w || _mouseImage.h != h || _mouseImage.format != mformat)
		_mouseImage.create(w, h, mformat);

	_mouseImage.copyRectToSurface(buf, _mouseImage.pitch, 0, 0, w, h);

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;
	_mouseKeyColor = keycolor;
	_mouseDontScale = dontScale;
}

void LibretroGraphics::setCursorPalette(const byte *colors, uint start, uint num) {
	_mousePalette.set(colors, start, num);
	_mousePaletteEnabled = true;
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
	return (f == OSystem::kFeatureCursorPalette) || (f == OSystem::kFeatureCursorAlpha);
}

void LibretroGraphics::setFeatureState(OSystem::Feature f, bool enable) {
	if (f == OSystem::kFeatureCursorPalette)
		_mousePaletteEnabled = enable;
}

bool LibretroGraphics::getFeatureState(OSystem::Feature f) const {
	return (f == OSystem::kFeatureCursorPalette) ? _mousePaletteEnabled : false;
}

void LibretroGraphics::setMousePosition(int x, int y){
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
