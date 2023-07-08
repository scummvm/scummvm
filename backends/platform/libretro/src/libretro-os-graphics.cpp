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

//#include "common/system.h"
#include "graphics/colormasks.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "backends/platform/libretro/include/libretro-defs.h"

static INLINE void blit_uint8_uint16_fast(Graphics::Surface &aOut, const Graphics::Surface &aIn, const LibretroPalette &aColors) {
	for (int i = 0; i < aIn.h; i++) {
		if (i >= aOut.h)
			continue;

		uint8_t *const in = (uint8_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + (i * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if (j >= aOut.w)
				continue;

			uint8 r, g, b;

			const uint8_t val = in[j];
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

		uint32_t *const in = (uint32_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + (i * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if (j >= aOut.w)
				continue;

			uint8 r, g, b;

			// const uint32_t val = in[j];
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

		uint16_t *const in = (uint16_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + (i * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if (j >= aOut.w)
				continue;

			uint8 r, g, b;

			// const uint16_t val = in[j];
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

		uint8_t *const in = (uint8_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + ((i + aY) * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if ((j + aX) < 0 || (j + aX) >= aOut.w)
				continue;

			uint8 r, g, b;

			const uint8_t val = in[j];
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

		uint16_t *const in = (uint16_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + ((i + aY) * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if ((j + aX) < 0 || (j + aX) >= aOut.w)
				continue;

			uint8 r, g, b;

			const uint16_t val = in[j];
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

		uint32_t *const in = (uint32_t *)aIn.getPixels() + (i * aIn.w);
		uint16_t *const out = (uint16_t *)aOut.getPixels() + ((i + aY) * aOut.w);

		for (int j = 0; j < aIn.w; j++) {
			if ((j + aX) < 0 || (j + aX) >= aOut.w)
				continue;

			uint8 in_a, in_r, in_g, in_b;
			uint8 out_r, out_g, out_b;
			uint32_t blend_r, blend_g, blend_b;

			const uint32_t val = in[j];
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

static INLINE void copyRectToSurface(uint8_t *pixels, int out_pitch, const uint8_t *src, int pitch, int x, int y, int w, int h, int out_bpp) {
	uint8_t *dst = pixels + y * out_pitch + x * out_bpp;

	do {
		memcpy(dst, src, w * out_bpp);
		src += pitch;
		dst += out_pitch;
	} while (--h);
}

LibretroPalette::LibretroPalette() {
	memset(_colors, 0, sizeof(_colors));
}

void LibretroPalette::set(const byte *colors, uint start, uint num) {
	memcpy(_colors + start * 3, colors, num * 3);
}

void LibretroPalette::get(byte *colors, uint start, uint num) const {
	memcpy(colors, _colors + start * 3, num * 3);
}

unsigned char * LibretroPalette::getColor(uint aIndex) const {
	return (unsigned char *)&_colors[aIndex * 3];
}

Common::List<Graphics::PixelFormat> OSystem_libretro::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> result;

#ifdef SCUMM_LITTLE_ENDIAN
	/* RGBA8888 */
	result.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
#else
	/* ABGR8888 */
	result.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
#endif
#ifdef FRONTEND_SUPPORTS_RGB565
	/* RGB565 - overlay */
	result.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
#endif
	/* RGB555 - fmtowns */
	result.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15));

	/* Palette - most games */
	result.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return result;
}

const OSystem_libretro::GraphicsMode *OSystem_libretro::getSupportedGraphicsModes() const {
	static const OSystem::GraphicsMode s_noGraphicsModes[] = {{0, 0, 0}};
	return s_noGraphicsModes;
}



void OSystem_libretro::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	_gameScreen.create(width, height, format ? *format : Graphics::PixelFormat::createFormatCLUT8());
}

int16 OSystem_libretro::getHeight() {
	return _gameScreen.h;
}

int16 OSystem_libretro::getWidth() {
	return _gameScreen.w;
}

Graphics::PixelFormat OSystem_libretro::getScreenFormat() const {
	return _gameScreen.format;
}

void OSystem_libretro::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	const uint8_t *src = (const uint8_t *)buf;
	uint8_t *pix = (uint8_t *)_gameScreen.getPixels();
	copyRectToSurface(pix, _gameScreen.pitch, src, pitch, x, y, w, h, _gameScreen.format.bytesPerPixel);
}

void OSystem_libretro::updateScreen() {
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
		const int x = _mouseX - _mouseHotspotX;
		const int y = _mouseY - _mouseHotspotY;

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

	if (! timing_inaccuracies_is_enabled()) {
		_threadSwitchCaller = THREAD_SWITCH_UPDATE;
		((LibretroTimerManager *)_timerManager)->checkThread();
	}
}

void OSystem_libretro::showOverlay(bool inGUI) {
	_overlayVisible = true;
	_overlayInGUI = inGUI;
}

void OSystem_libretro::hideOverlay() {
	_overlayVisible = false;
	_overlayInGUI = false;
}

void OSystem_libretro::clearOverlay() {
	_overlay.fillRect(Common::Rect(_overlay.w, _overlay.h), 0);
}

void OSystem_libretro::grabOverlay(Graphics::Surface &surface) {
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

void OSystem_libretro::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	const uint8_t *src = (const uint8_t *)buf;
	uint8_t *pix = (uint8_t *)_overlay.getPixels();
	copyRectToSurface(pix, _overlay.pitch, src, pitch, x, y, w, h, _overlay.format.bytesPerPixel);
}

int16 OSystem_libretro::getOverlayHeight() {
	return _overlay.h;
}

int16 OSystem_libretro::getOverlayWidth() {
	return _overlay.w;
}

Graphics::PixelFormat OSystem_libretro::getOverlayFormat() const {
	return _overlay.format;
}

bool OSystem_libretro::showMouse(bool visible) {
	const bool wasVisible = _mouseVisible;
	_mouseVisible = visible;
	return wasVisible;
}

void OSystem_libretro::warpMouse(int x, int y) {
	_mouseX = x;
	_mouseY = y;
}

void OSystem_libretro::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
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

void OSystem_libretro::setCursorPalette(const byte *colors, uint start, uint num) {
	_mousePalette.set(colors, start, num);
	_mousePaletteEnabled = true;
}

const Graphics::Surface &OSystem_libretro::getScreen() {
	const Graphics::Surface &srcSurface = (_overlayInGUI) ? _overlay : _gameScreen;

	if (srcSurface.w != _screen.w || srcSurface.h != _screen.h) {
#ifdef FRONTEND_SUPPORTS_RGB565
		_screen.create(srcSurface.w, srcSurface.h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
#else
		_screen.create(srcSurface.w, srcSurface.h, Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15));
#endif
	}

	return _screen;
}

void OSystem_libretro::setPalette(const byte *colors, uint start, uint num) {
	_gamePalette.set(colors, start, num);
}

void OSystem_libretro::grabPalette(byte *colors, uint start, uint num) const {
	_gamePalette.get(colors, start, num);
}
