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

#include <retro_inline.h>

#include "graphics/colormasks.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "backends/platform/libretro/include/libretro-graphics.h"

#include "gui/message.h"

#ifdef USE_OPENGL
#include "backends/graphics/opengl/opengl-graphics.h"
#include "backends/graphics/opengl/framebuffer.h"
#include "graphics/opengl/debug.h"
#endif

static INLINE void blit_uint8_uint16_fast(Graphics::Surface &aOut, const Graphics::Surface &aIn, const LibretroPalette &aColors) {
	for (int i = 0; i < aIn.h; i++) {
		if (i >= aOut.h)
			continue;

		uint8 *const in = (uint8 *)aIn.getPixels() + (i * aIn.w);
		uint16 *const out = (uint16 *)aOut.getPixels() + (i * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if (j >= aOut.w)
				continue;

			uint8 r, g, b;

			const uint8 val = in[j];
			// if(val != 0xFFFFFFFF)
			{
				if (aIn.format.bytesPerPixel == 1) {
					unsigned char *col = aColors.getColor(val);
					r = *col++;
					g = *col++;
					b = *col++;
				} else
					aIn.format.colorToRGB(in[j], r, g, b);

				out[j] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static INLINE void blit_uint32_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, const LibretroPalette &aColors) {
	for (int i = 0; i < aIn.h; i++) {
		if (i >= aOut.h)
			continue;

		uint32 *const in = (uint32 *)aIn.getPixels() + (i * aIn.w);
		uint16 *const out = (uint16 *)aOut.getPixels() + (i * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if (j >= aOut.w)
				continue;

			uint8 r, g, b;

			// const uint32 val = in[j];
			// if(val != 0xFFFFFFFF)
			{
				aIn.format.colorToRGB(in[j], r, g, b);
				out[j] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static INLINE void blit_uint16_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, const LibretroPalette &aColors) {
	for (int i = 0; i < aIn.h; i++) {
		if (i >= aOut.h)
			continue;

		uint16 *const in = (uint16 *)aIn.getPixels() + (i * aIn.w);
		uint16 *const out = (uint16 *)aOut.getPixels() + (i * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if (j >= aOut.w)
				continue;

			uint8 r, g, b;

			// const uint16 val = in[j];
			// if(val != 0xFFFFFFFF)
			{
				aIn.format.colorToRGB(in[j], r, g, b);
				out[j] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static void blit_uint8_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, int aX, int aY, const LibretroPalette &aColors, uint32 aKeyColor) {
	for (int i = 0; i < aIn.h; i++) {
		if ((i + aY) < 0 || (i + aY) >= aOut.h)
			continue;

		uint8 *const in = (uint8 *)aIn.getPixels() + (i * aIn.w);
		uint16 *const out = (uint16 *)aOut.getPixels() + ((i + aY) * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if ((j + aX) < 0 || (j + aX) >= aOut.w)
				continue;

			uint8 r, g, b;

			const uint8 val = in[j];
			if (val != aKeyColor) {
				unsigned char *col = aColors.getColor(val);
				r = *col++;
				g = *col++;
				b = *col++;
				out[j + aX] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static void blit_uint16_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, int aX, int aY, const LibretroPalette &aColors, uint32 aKeyColor) {
	for (int i = 0; i < aIn.h; i++) {
		if ((i + aY) < 0 || (i + aY) >= aOut.h)
			continue;

		uint16 *const in = (uint16 *)aIn.getPixels() + (i * aIn.w);
		uint16 *const out = (uint16 *)aOut.getPixels() + ((i + aY) * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if ((j + aX) < 0 || (j + aX) >= aOut.w)
				continue;

			uint8 r, g, b;

			const uint16 val = in[j];
			if (val != aKeyColor) {
				aIn.format.colorToRGB(in[j], r, g, b);
				out[j + aX] = aOut.format.RGBToColor(r, g, b);
			}
		}
	}
}

static void blit_uint32_uint16(Graphics::Surface &aOut, const Graphics::Surface &aIn, int aX, int aY, const LibretroPalette &aColors, uint32 aKeyColor) {
	for (int i = 0; i < aIn.h; i++) {
		if ((i + aY) < 0 || (i + aY) >= aOut.h)
			continue;

		uint32 *const in = (uint32 *)aIn.getPixels() + (i * aIn.w);
		uint16 *const out = (uint16 *)aOut.getPixels() + ((i + aY) * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if ((j + aX) < 0 || (j + aX) >= aOut.w)
				continue;

			uint8 in_a, in_r, in_g, in_b;
			uint8 out_r, out_g, out_b;
			uint32 blend_r, blend_g, blend_b;

			const uint32 val = in[j];
			if (val != aKeyColor) {
				aIn.format.colorToARGB(in[j], in_a, in_r, in_g, in_b);

				if (in_a) {
					aOut.format.colorToRGB(out[j + aX], out_r, out_g, out_b);

					blend_r = ((in_r * in_a) + (out_r * (255 - in_a))) / 255;
					blend_g = ((in_g * in_a) + (out_g * (255 - in_a))) / 255;
					blend_b = ((in_b * in_a) + (out_b * (255 - in_a))) / 255;

					out[j + aX] = aOut.format.RGBToColor(blend_r, blend_g, blend_b);
				}
			}
		}
	}
}

static INLINE void copyRectToSurface(uint8 *pixels, int out_pitch, const uint8 *src, int pitch, int x, int y, int w, int h, int out_bpp) {
	uint8 *dst = pixels + y * out_pitch + x * out_bpp;

	do {
		memcpy(dst, src, w * out_bpp);
		src += pitch;
		dst += out_pitch;
	} while (--h);
}

LibretroPalette::LibretroPalette() : _prevColorsSource(NULL) {
	memset(_colors, 0, sizeof(_colors));
}

void LibretroPalette::set(const byte *colors, uint start, uint num) {
	/* TODO: This check is a workaround to handle a SEGFAULT in iOS due to the call from SmushPlayer::play in scumm engine,
	caused by the corruption of start argument (and consequently colors ptr). Root cause to be investigated. */
	if (start > 255) {
		start = 0;
		colors = _prevColorsSource;
	} else
		_prevColorsSource = colors;

	if (num > 256)
		num = 256;

	if (colors)
		memcpy(_colors + start * 3, colors, num * 3);
	else
		LIBRETRO_G_SYSTEM->logMessage(LogMessageType::kError, "LibretroPalette colors ptr is NULL\n");
}

void LibretroPalette::get(byte *colors, uint start, uint num) const {
	memcpy(colors, _colors + start * 3, num * 3);
}

unsigned char *LibretroPalette::getColor(uint aIndex) const {
	return (unsigned char *)&_colors[aIndex * 3];
}

LibretroGraphics::LibretroGraphics() : _mousePaletteEnabled(false), _mouseVisible(false), _mouseKeyColor(0), _mouseDontScale(false), _screenUpdatePending(false) {
	_overlay.create(RES_W_OVERLAY, RES_H_OVERLAY, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
}

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
	_gameScreen.create(width, height, format ? *format : Graphics::PixelFormat::createFormatCLUT8());
	LIBRETRO_G_SYSTEM->refreshRetroSettings();
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
	const uint8 *src = (const uint8 *)buf;
	uint8 *pix = (uint8 *)_gameScreen.getPixels();
	copyRectToSurface(pix, _gameScreen.pitch, src, pitch, x, y, w, h, _gameScreen.format.bytesPerPixel);
}

void LibretroGraphics::updateScreen() {
	_screenUpdatePending = true;
	if (! retro_setting_get_timing_inaccuracies_enabled() && !_overlayInGUI)
		dynamic_cast<LibretroTimerManager *>(LIBRETRO_G_SYSTEM->getTimerManager())->checkThread(THREAD_SWITCH_UPDATE);
}

void LibretroGraphics::realUpdateScreen(void) {
	const Graphics::Surface &srcSurface = (_overlayInGUI) ? _overlay : _gameScreen;
	if (srcSurface.w && srcSurface.h) {
		switch (srcSurface.format.bytesPerPixel) {
		case 1:
		case 3:
			blit_uint8_uint16_fast(_screen, srcSurface, _gamePalette);
			break;
		case 2:
			blit_uint16_uint16(_screen, srcSurface, _gamePalette);
			break;
		case 4:
			blit_uint32_uint16(_screen, srcSurface, _gamePalette);
			break;
		}
	}

	// Draw Mouse
	if (_mouseVisible && _mouseImage.w && _mouseImage.h) {
		const int x = LIBRETRO_G_SYSTEM->_mouseX - _mouseHotspotX;
		const int y = LIBRETRO_G_SYSTEM->_mouseY - _mouseHotspotY;

		switch (_mouseImage.format.bytesPerPixel) {
		case 1:
		case 3:
			blit_uint8_uint16(_screen, _mouseImage, x, y, _mousePaletteEnabled ? _mousePalette : _gamePalette, _mouseKeyColor);
			break;
		case 2:
			blit_uint16_uint16(_screen, _mouseImage, x, y, _mousePaletteEnabled ? _mousePalette : _gamePalette, _mouseKeyColor);
			break;
		case 4:
			blit_uint32_uint16(_screen, _mouseImage, x, y, _mousePaletteEnabled ? _mousePalette : _gamePalette, _mouseKeyColor);
			break;
		}
	}
	_screenUpdatePending = false;
}

void LibretroGraphics::showOverlay(bool inGUI) {
	_overlayVisible = true;
	_overlayInGUI = inGUI;
}

void LibretroGraphics::hideOverlay() {
	_overlayVisible = false;
	_overlayInGUI = false;
}

void LibretroGraphics::clearOverlay() {
	_overlay.fillRect(Common::Rect(_overlay.w, _overlay.h), 0);
}

void LibretroGraphics::grabOverlay(Graphics::Surface &surface) const {
	const unsigned char *src = (unsigned char *)_overlay.getPixels();
	unsigned char *dst = (byte *)surface.getPixels();
	;
	unsigned i = RES_H_OVERLAY;

	do {
		memcpy(dst, src, RES_W_OVERLAY << 1);
		dst += surface.pitch;
		src += RES_W_OVERLAY << 1;
	} while (--i);
}

void LibretroGraphics::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	const uint8 *src = (const uint8 *)buf;
	uint8 *pix = (uint8 *)_overlay.getPixels();
	copyRectToSurface(pix, _overlay.pitch, src, pitch, x, y, w, h, _overlay.format.bytesPerPixel);
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
}

void LibretroGraphics::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	const Graphics::PixelFormat mformat = format ? *format : Graphics::PixelFormat::createFormatCLUT8();

	if (_mouseImage.w != w || _mouseImage.h != h || _mouseImage.format != mformat) {
		_mouseImage.create(w, h, mformat);
	}

	memcpy(_mouseImage.getPixels(), buf, h * _mouseImage.pitch);

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

const Graphics::Surface *LibretroGraphics::getScreen() {
	const Graphics::Surface &srcSurface = (_overlayInGUI) ? _overlay : _gameScreen;

	if (srcSurface.w != _screen.w || srcSurface.h != _screen.h)
		_screen.create(srcSurface.w, srcSurface.h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	return &_screen;
}

void LibretroGraphics::setPalette(const byte *colors, uint start, uint num) {
	_gamePalette.set(colors, start, num);
}

void LibretroGraphics::grabPalette(byte *colors, uint start, uint num) const {
	_gamePalette.get(colors, start, num);
}

bool LibretroGraphics::hasFeature(OSystem::Feature f) const {
	return (f == OSystem::kFeatureCursorPalette) || (f == OSystem::kFeatureCursorAlpha);
}

void LibretroGraphics::setFeatureState(OSystem::Feature f, bool enable) {
	if (f == OSystem::kFeatureCursorPalette)
		_mousePaletteEnabled = enable;
}

void LibretroGraphics::displayMessageOnOSD(const Common::U32String &msg) {
	// Display the message for 3 seconds
	GUI::TimedMessageDialog dialog(msg, 3000);
	dialog.runModal();
}

bool LibretroGraphics::getFeatureState(OSystem::Feature f) const {
	return (f == OSystem::kFeatureCursorPalette) ? _mousePaletteEnabled : false;
}

#ifdef USE_OPENGL
LibretroOpenGLGraphics::LibretroOpenGLGraphics(OpenGL::ContextType contextType) {
	resetContext(contextType);
}

void LibretroOpenGLGraphics::refreshScreen(){
	dynamic_cast<LibretroTimerManager *>(LIBRETRO_G_SYSTEM->getTimerManager())->checkThread(THREAD_SWITCH_UPDATE);
}

void LibretroOpenGLGraphics::setMousePosition(int x, int y){
	OpenGL::OpenGLGraphicsManager::setMousePosition(x,y);
}

void LibretroOpenGLGraphics::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	/* Workaround to fix a cursor glich (e.g. GUI with Classic theme) occurring when any overlay is activated from retroarch (e.g. keyboard overlay).
	   Currently no feedback is available from frontend to detect if overlays are toggled to delete _cursor only if needed.
	   @TODO: root cause to be investigated. */
	delete _cursor;
	_cursor = nullptr;
	OpenGL::OpenGLGraphicsManager::setMouseCursor(buf, w, h, hotspotX, hotspotY, keycolor, dontScale, format, mask);
}


Common::Point LibretroOpenGLGraphics::convertWindowToVirtual(int x, int y) const {
	return OpenGL::OpenGLGraphicsManager::convertWindowToVirtual(x, y);
}

void LibretroHWFramebuffer::activateInternal(){
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, retro_get_hw_fb()));
}

void LibretroOpenGLGraphics::resetContext(OpenGL::ContextType contextType) {
	const Graphics::PixelFormat rgba8888 =
#ifdef SCUMM_LITTLE_ENDIAN
									   Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#else
									   Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#endif
	notifyContextDestroy();
	notifyContextCreate(contextType, new LibretroHWFramebuffer(), rgba8888, rgba8888);
	handleResize(RES_W_OVERLAY, RES_H_OVERLAY);
}

#endif
