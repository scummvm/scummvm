/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <nds.h>

#include "backends/platform/ds/osystem_ds.h"
#include "backends/platform/ds/blitters.h"

#include "common/translation.h"

namespace DS {

// From console.c in NDSLib

// Defines
#define SCUMM_GAME_HEIGHT 142
#define SCUMM_GAME_WIDTH 227

// Scaled
static int scX;
static int scY;

static int subScX;
static int subScY;
static int subScTargetX;
static int subScTargetY;
static int subScreenWidth = SCUMM_GAME_WIDTH;
static int subScreenHeight = SCUMM_GAME_HEIGHT;
static int subScreenScale = 256;

static bool gameScreenSwap = false;

// Shake
static int s_shakeXOffset = 0;
static int s_shakeYOffset = 0;

// Touch
static int touchScX, touchScY, touchX, touchY;

// 8-bit surface size
static int gameWidth = 320;
static int gameHeight = 200;

void setGameScreenSwap(bool enable) {
	gameScreenSwap = enable;
}

void setTopScreenZoom(int percentage) {
	s32 scale = (percentage << 8) / 100;
	subScreenScale = (256 * 256) / scale;
}

void setTopScreenTarget(int x, int y) {
	subScTargetX = (x - (subScreenWidth >> 1));
	subScTargetY = (y - (subScreenHeight >> 1));

	if (subScTargetX < 0) subScTargetX = 0;
	if (subScTargetX > gameWidth - subScreenWidth) subScTargetX = gameWidth - subScreenWidth;

	if (subScTargetY < 0) subScTargetY = 0;
	if (subScTargetY > gameHeight - subScreenHeight) subScTargetY = gameHeight - subScreenHeight;

	subScTargetX <<=8;
	subScTargetY <<=8;
}

void setGameSize(int width, int height, bool isRGB) {
	gameWidth = width;
	gameHeight = height;

	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06040000);

	if (g_system->getGraphicsMode() == GFX_SWSCALE) {
		REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(8);

		REG_BG3PA = 256;
		REG_BG3PB = 0;
		REG_BG3PC = 0;
		REG_BG3PD = (int) ((200.0f / 192.0f) * 256);

	} else {
		REG_BG3CNT = (isRGB ? BG_BMP16_512x256 :BG_BMP8_512x256) | BG_BMP_BASE(8);

		REG_BG3PA = (int) (((float) (gameWidth) / 256.0f) * 256);
		REG_BG3PB = 0;
		REG_BG3PC = 0;
		REG_BG3PD = (int) ((200.0f / 192.0f) * 256);
	}

#ifdef DISABLE_TEXT_CONSOLE
	REG_BG3CNT_SUB = BG_BMP8_512x256;

	REG_BG3PA_SUB = (int) (subScreenWidth / 256.0f * 256);
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = (int) (subScreenHeight / 192.0f * 256);
#endif
}

void setShakePos(int shakeXOffset, int shakeYOffset) {
	s_shakeXOffset = shakeXOffset;
	s_shakeYOffset = shakeYOffset;
}

Common::Point warpMouse(int penX, int penY, bool isOverlayShown) {
	int storedMouseX, storedMouseY;
	if (!isOverlayShown) {
		storedMouseX = ((penX - touchX) << 8) / touchScX;
		storedMouseY = ((penY - touchY) << 8) / touchScY;
	} else {
		storedMouseX = penX;
		storedMouseY = penY;
	}

	return Common::Point(storedMouseX, storedMouseY);
}

void setMainScreenScroll(int x, int y) {
		REG_BG3X = x;
		REG_BG3Y = y;

		if (!gameScreenSwap) {
			touchX = x >> 8;
			touchY = y >> 8;
		}
}

void setMainScreenScale(int x, int y) {
		if ((g_system->getGraphicsMode() == GFX_SWSCALE) && (x==320)) {
			REG_BG3PA = 256;
			REG_BG3PB = 0;
			REG_BG3PC = 0;
			REG_BG3PD = y;
		} else {
			REG_BG3PA = x;
			REG_BG3PB = 0;
			REG_BG3PC = 0;
			REG_BG3PD = y;
		}

		if (!gameScreenSwap) {
			touchScX = x;
			touchScY = y;
		}
}

void setZoomedScreenScroll(int x, int y, bool shake) {
		if (gameScreenSwap) {
			touchX = x >> 8;
			touchY = y >> 8;
		}

#ifdef DISABLE_TEXT_CONSOLE
		REG_BG3X_SUB = x;
		REG_BG3Y_SUB = y;
#endif
}

void setZoomedScreenScale(int x, int y) {
		if (gameScreenSwap) {
			touchScX = x;
			touchScY = y;
		}

#ifdef DISABLE_TEXT_CONSOLE
		REG_BG3PA_SUB = x;
		REG_BG3PB_SUB = 0;
		REG_BG3PC_SUB = 0;
		REG_BG3PD_SUB = y;
#endif
}

Common::Point transformPoint(uint16 x, uint16 y, bool isOverlayShown) {
	if (!isOverlayShown) {
		x = ((x * touchScX) >> 8) + touchX;
		x = CLIP<uint16>(x, 0, gameWidth  - 1);

		y = ((y * touchScY) >> 8) + touchY;
		y = CLIP<uint16>(y, 0, gameHeight - 1);
	}

	return Common::Point(x, y);
}

void VBlankHandler(void) {
	int xCenter = subScTargetX + ((subScreenWidth >> 1) << 8);
	int yCenter = subScTargetY + ((subScreenHeight >> 1) << 8);

	subScreenWidth = (256 * subScreenScale) >> 8;
	subScreenHeight = (192 * subScreenScale) >> 8;

	if ( ((subScreenWidth) > 256 - 8) && ((subScreenWidth) < 256 + 8) ) {
		subScreenWidth = 256;
		subScreenHeight = 192;
	} else if ( ((subScreenWidth) > 128 - 8) && ((subScreenWidth) < 128 + 8) ) {
		subScreenWidth = 128;
		subScreenHeight = 96;
	} else if (subScreenWidth > 256) {
		subScreenWidth = 320;
		subScreenHeight = 200;
	}

	subScTargetX = xCenter - ((subScreenWidth  >> 1) << 8);
	subScTargetY = yCenter - ((subScreenHeight >> 1) << 8);

	subScTargetX = CLIP(subScTargetX, 0, (gameWidth  - subScreenWidth)  << 8);
	subScTargetY = CLIP(subScTargetY, 0, (gameHeight - subScreenHeight) << 8);

	subScX += (subScTargetX - subScX) >> 2;
	subScY += (subScTargetY - subScY) >> 2;

	if (g_system->getGraphicsMode() == GFX_NOSCALE) {
		if (scX + 256 > gameWidth - 1) {
			scX = gameWidth - 1 - 256;
		}

		if (scX < 0) {
			scX = 0;
		}

		if (scY + 192 > gameHeight - 1) {
			scY = gameHeight - 1 - 192;
		}

		if (scY < 0) {
			scY = 0;
		}

		setZoomedScreenScroll(subScX, subScY, (subScreenWidth != 256) && (subScreenWidth != 128));
		setZoomedScreenScale(subScreenWidth, ((subScreenHeight * (256 << 8)) / 192) >> 8);

		setMainScreenScroll((scX << 8) + (s_shakeXOffset << 8), (scY << 8) + (s_shakeYOffset << 8));
		setMainScreenScale(256, 256);		// 1:1 scale
	} else {
		if (scY > gameHeight - 192 - 1) {
			scY = gameHeight - 192 - 1;
		}

		if (scY < 0) {
			scY = 0;
		}

		setZoomedScreenScroll(subScX, subScY, (subScreenWidth != 256) && (subScreenWidth != 128));
		setZoomedScreenScale(subScreenWidth, ((subScreenHeight * (256 << 8)) / 192) >> 8);

		setMainScreenScroll(64 + (s_shakeXOffset << 8), (scY << 8) + (s_shakeYOffset << 8));
		setMainScreenScale(320, 256);		// 1:1 scale
	}
}

void setTalkPos(int x, int y) {
	setTopScreenTarget(x, y);
}

void initHardware() {
	powerOn(POWER_ALL);

	for (int r = 0; r < 255; r++) {
		BG_PALETTE[r] = 0;
	}

	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankE(VRAM_E_MAIN_SPRITE);

	scX = 0;
	scY = 0;
	subScX = 0;
	subScY = 0;
	subScTargetX = 0;
	subScTargetY = 0;

	lcdMainOnBottom();

	//irqs are nice
	irqSet(IRQ_VBLANK, VBlankHandler);
	irqEnable(IRQ_VBLANK);

#ifndef DISABLE_TEXT_CONSOLE
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankH(VRAM_H_SUB_BG);
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
#else
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
#endif
}

}

void OSystem_DS::initGraphics() {
	DS::initHardware();

	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);
	_cursorSprite = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_Bmp);

	_overlay.create(256, 192, true, 2, false, 0);
}

bool OSystem_DS::hasFeature(Feature f) {
	return (f == kFeatureCursorPalette) || (f == kFeatureStretchMode);
}

void OSystem_DS::setFeatureState(Feature f, bool enable) {
	if (f == kFeatureCursorPalette) {
		_disableCursorPalette = !enable;
		refreshCursor(_cursorSprite, _cursor, !_disableCursorPalette ? _cursorPalette : _palette);
	}
}

bool OSystem_DS::getFeatureState(Feature f) {
	if (f == kFeatureCursorPalette)
		return !_disableCursorPalette;
	return false;
}

static const OSystem::GraphicsMode graphicsModes[] = {
	{ "NONE",  _s("Unscaled"),                                  GFX_NOSCALE },
	{ "HW",    _s("Hardware scale (fast, but low quality)"),    GFX_HWSCALE },
	{ "SW",    _s("Software scale (good quality, but slower)"), GFX_SWSCALE },
	{ nullptr, nullptr,                                         0           }
};

const OSystem::GraphicsMode *OSystem_DS::getSupportedGraphicsModes() const {
	return graphicsModes;
}

int OSystem_DS::getDefaultGraphicsMode() const {
	return GFX_HWSCALE;
}

bool OSystem_DS::setGraphicsMode(int mode) {
	switch (mode) {
	case GFX_NOSCALE:
	case GFX_HWSCALE:
	case GFX_SWSCALE:
		_graphicsMode = mode;
		return true;
	default:
		return false;
	}
}

int OSystem_DS::getGraphicsMode() const {
	return _graphicsMode;
}

static const OSystem::GraphicsMode stretchModes[] = {
	{ "100",   "100%",  100 },
	{ "150",   "150%",  150 },
	{ "200",   "200%",  200 },
	{ nullptr, nullptr, 0   }
};

const OSystem::GraphicsMode *OSystem_DS::getSupportedStretchModes() const {
	return stretchModes;
}

int OSystem_DS::getDefaultStretchMode() const {
	return 100;
}

bool OSystem_DS::setStretchMode(int mode) {
	_stretchMode = mode;
	DS::setTopScreenZoom(mode);
	return true;
}

int OSystem_DS::getStretchMode() const {
	return _stretchMode;
}

Graphics::PixelFormat OSystem_DS::getScreenFormat() const {
	return _framebuffer.format;
}

Common::List<Graphics::PixelFormat> OSystem_DS::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> res;
	res.push_back(_pfABGR1555);
	res.push_back(_pfCLUT8);

	return res;
}

void OSystem_DS::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	Graphics::PixelFormat actualFormat = format ? *format : _pfCLUT8;
	_framebuffer.create(width, height, actualFormat);

	// For Lost in Time, the title screen is displayed in 640x400.
	// In order to support this game, the screen mode is set, but
	// all draw calls are ignored until the game switches to 320x200.
	if ((width == 640) && (height == 400)) {
		_graphicsEnable = false;
	} else {
		_graphicsEnable = true;
		DS::setGameSize(width, height, (actualFormat != _pfCLUT8));
	}
}

int16 OSystem_DS::getHeight() {
	return _framebuffer.h;
}

int16 OSystem_DS::getWidth() {
	return _framebuffer.w;
}

void OSystem_DS::setPalette(const byte *colors, uint start, uint num) {
	for (unsigned int r = start; r < start + num; r++) {
		int red = *colors;
		int green = *(colors + 1);
		int blue = *(colors + 2);

		red >>= 3;
		green >>= 3;
		blue >>= 3;

		{
			u16 paletteValue = red | (green << 5) | (blue << 10);

			if (!_overlay.isVisible()) {
				BG_PALETTE[r] = paletteValue;
#ifdef DISABLE_TEXT_CONSOLE
				BG_PALETTE_SUB[r] = paletteValue;
#endif
			}

			_palette[r] = paletteValue;
		}

		colors += 3;
	}
}

void OSystem_DS::setCursorPalette(const byte *colors, uint start, uint num) {

	for (unsigned int r = start; r < start + num; r++) {
		int red = *colors;
		int green = *(colors + 1);
		int blue = *(colors + 2);

		red >>= 3;
		green >>= 3;
		blue >>= 3;

		u16 paletteValue = red | (green << 5) | (blue << 10);
		_cursorPalette[r] = paletteValue;

		colors += 3;
	}

	_disableCursorPalette = false;
	refreshCursor(_cursorSprite, _cursor, !_disableCursorPalette ? _cursorPalette : _palette);
}

void OSystem_DS::grabPalette(unsigned char *colors, uint start, uint num) const {
	for (unsigned int r = start; r < start + num; r++) {
		*colors++ = (BG_PALETTE[r] & 0x001F) << 3;
		*colors++ = (BG_PALETTE[r] & 0x03E0) >> 5 << 3;
		*colors++ = (BG_PALETTE[r] & 0x7C00) >> 10 << 3;
	}
}

void OSystem_DS::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	_framebuffer.copyRectToSurface(buf, pitch, x, y, w, h);
}

void OSystem_DS::dmaBlit(uint16 *dst, const uint dstPitch, const uint16 *src, const uint srcPitch,
                         const uint w, const uint h, const uint bytesPerPixel) {
	// The DS video RAM doesn't support 8-bit writes because Nintendo wanted
	// to save a few pennies/euro cents on the hardware.

	uint row = w * bytesPerPixel;

	for (uint dy = 0; dy < h; dy += 2) {
		const u16 *src1 = src;
		src += (srcPitch >> 1);
		DC_FlushRange(src1, row << 1);

		const u16 *src2 = src;
		src += (srcPitch >> 1);
		DC_FlushRange(src2, row << 1);

		u16 *dest1 = dst;
		dst += (dstPitch >> 1);
		DC_FlushRange(dest1, row << 1);

		u16 *dest2 = dst;
		dst += (dstPitch >> 1);
		DC_FlushRange(dest2, row << 1);

		dmaCopyHalfWordsAsynch(2, src1, dest1, row);
		dmaCopyHalfWordsAsynch(3, src2, dest2, row);

		while (dmaBusy(2) || dmaBusy(3));
	}
}

void OSystem_DS::updateScreen() {
	oamSet(&oamMain, 0, _cursorPos.x - _cursorHotX, _cursorPos.y - _cursorHotY, 0, 15, SpriteSize_64x64,
	       SpriteColorFormat_Bmp, _cursorSprite, 0, false, !_cursorVisible, false, false, false);
	oamUpdate(&oamMain);

	if (_overlay.isVisible()) {
		_overlay.update();
	} else if (_graphicsEnable) {
		u16 *base = BG_GFX + 0x10000;
		if (_graphicsMode == GFX_SWSCALE) {
			if (_framebuffer.format == _pfCLUT8) {
				Rescale_320x256xPAL8_To_256x256x1555(
					base,
					(const u8 *)_framebuffer.getPixels(),
					256,
					_framebuffer.pitch,
					BG_PALETTE,
					_framebuffer.h );
			} else {
				Rescale_320x256x1555_To_256x256x1555(
					base,
					(const u16 *)_framebuffer.getPixels(),
					256,
					_framebuffer.pitch / 2 );
			}
		} else {
			dmaBlit(base, 512 * _framebuffer.format.bytesPerPixel,
				(const u16 *)_framebuffer.getPixels(), _framebuffer.pitch,
				_framebuffer.w, _framebuffer.h, _framebuffer.format.bytesPerPixel);

#ifdef DISABLE_TEXT_CONSOLE
			if (_framebuffer.format == _pfCLUT8)
				dmaCopy(base, BG_GFX_SUB, 512 * 256);
#endif
		}
	}
}

void OSystem_DS::setShakePos(int shakeXOffset, int shakeYOffset) {
	DS::setShakePos(shakeXOffset, shakeYOffset);
}

void OSystem_DS::showOverlay() {
	_overlay.reset();
	_overlay.show();
	lcdMainOnBottom();
}

void OSystem_DS::hideOverlay() {
	_overlay.hide();

	if (DS::gameScreenSwap) {
		lcdMainOnTop();
	} else {
		lcdMainOnBottom();
	}
}

bool OSystem_DS::isOverlayVisible() const {
	return _overlay.isVisible();
}

void OSystem_DS::clearOverlay() {
	_overlay.clear();
}

void OSystem_DS::grabOverlay(void *buf, int pitch) {
	_overlay.grab((byte *)buf, pitch);
}

void OSystem_DS::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_overlay.copyRectToSurface(buf, pitch, x, y, w, h);
}

int16 OSystem_DS::getOverlayHeight() {
	return _overlay.h;
}

int16 OSystem_DS::getOverlayWidth() {
	return _overlay.w;
}

Graphics::PixelFormat OSystem_DS::getOverlayFormat() const {
	return _overlay.format;
}

Common::Point OSystem_DS::transformPoint(uint16 x, uint16 y) {
	return DS::transformPoint(x, y, _overlay.isVisible());
}

bool OSystem_DS::showMouse(bool visible) {
	const bool last = _cursorVisible;
	_cursorVisible = visible;
	return last;
}

void OSystem_DS::warpMouse(int x, int y) {
	_cursorPos = DS::warpMouse(x, y, _overlay.isVisible());
}

void OSystem_DS::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, u32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	if (!buf || w == 0 || h == 0)
		return;

	Graphics::PixelFormat actualFormat = format ? *format : _pfCLUT8;
	if (_cursor.w != w || _cursor.h != h || _cursor.format != actualFormat)
		_cursor.create(w, h, actualFormat);
	_cursor.copyRectToSurface(buf, w * actualFormat.bytesPerPixel, 0, 0, w, h);
	_cursorHotX = hotspotX;
	_cursorHotY = hotspotY;
	_cursorKey = keycolor;

	if (actualFormat != _pfCLUT8 && actualFormat != _pfABGR1555) {
		uint8 a, r, g, b;
		actualFormat.colorToARGB(_cursorKey, a, r, g, b);
		_cursorKey = _pfABGR1555.ARGBToColor(a, r, g, b);
		_cursor.convertToInPlace(_pfABGR1555);
	}

	refreshCursor(_cursorSprite, _cursor, !_disableCursorPalette ? _cursorPalette : _palette);
}

void OSystem_DS::refreshCursor(u16 *dst, const Graphics::Surface &src, const uint16 *palette) {
	const uint w = MIN<uint>(src.w, 64);
	const uint h = MIN<uint>(src.h, 64);

	dmaFillHalfWords(0, dst, 64 * 64 * 2);

	if (src.format == _pfCLUT8) {
		for (uint y = 0; y < h; y++) {
			const uint8 *row = (const uint8 *)src.getBasePtr(0, y);
			for (uint x = 0; x < w; x++) {
				uint8 color = *row++;

				if (color != _cursorKey)
					dst[y * 64 + x] = palette[color] | 0x8000;
			}
		}
	} else {
		for (uint y = 0; y < h; y++) {
			const uint16 *row = (const uint16 *)src.getBasePtr(0, y);
			for (uint x = 0; x < w; x++) {
				uint16 color = *row++;

				if (color != _cursorKey)
					dst[y * 64 + x] = color;
			}
		}
	}
}

Graphics::Surface *OSystem_DS::lockScreen() {
	return &_framebuffer;
}

void OSystem_DS::unlockScreen() {
	// No need to do anything here.  The screen will be updated in updateScreen().
}

void OSystem_DS::setFocusRectangle(const Common::Rect& rect) {
	DS::setTalkPos(rect.left + rect.width() / 2, rect.top + rect.height() / 2);
}

void OSystem_DS::clearFocusRectangle() {

}
