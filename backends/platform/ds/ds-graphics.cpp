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

// Shake
static int s_shakeXOffset = 0;
static int s_shakeYOffset = 0;

// 8-bit surface size
static int gameWidth = 320;
static int gameHeight = 200;

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

void setGameSize(int width, int height) {
	gameWidth = width;
	gameHeight = height;
	setTopScreenTarget(width / 2, height / 2);
}

void setShakePos(int shakeXOffset, int shakeYOffset) {
	s_shakeXOffset = shakeXOffset;
	s_shakeYOffset = shakeYOffset;
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
	OSystem_DS::instance()->setSubScreen(subScX, subScY, subScreenWidth, ((subScreenHeight * (256 << 8)) / 192) >> 8);

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

		OSystem_DS::instance()->setMainScreen((scX << 8) + (s_shakeXOffset << 8), (scY << 8) + (s_shakeYOffset << 8), 256, 256);
	} else {
		if (scY > gameHeight - 192 - 1) {
			scY = gameHeight - 192 - 1;
		}

		if (scY < 0) {
			scY = 0;
		}

		OSystem_DS::instance()->setMainScreen(64 + (s_shakeXOffset << 8), (scY << 8) + (s_shakeYOffset << 8), gameWidth, ((gameHeight * (256 << 8)) / 192) >> 8);
	}
}

void initHardware() {
	powerOn(POWER_ALL);

	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06040000);
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

	_overlay.create(256, 192, true, 2, false, 0, false);
}

void OSystem_DS::setMainScreen(int32 x, int32 y, int32 sx, int32 sy) {
	_framebuffer.setScalef(sx, sy);
	_framebuffer.setScrollf(x, y);
}

void OSystem_DS::setSubScreen(int32 x, int32 y, int32 sx, int32 sy) {
#ifdef DISABLE_TEXT_CONSOLE
	_subScreen.setScalef(sx, sy);
	_subScreen.setScrollf(x, y);
#endif
}

bool OSystem_DS::hasFeature(Feature f) {
	return (f == kFeatureCursorPalette) || (f == kFeatureStretchMode);
}

void OSystem_DS::setFeatureState(Feature f, bool enable) {
	if (f == kFeatureCursorPalette) {
		_disableCursorPalette = !enable;
		_cursorDirty = true;
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

bool OSystem_DS::setGraphicsMode(int mode, uint flags) {
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
	bool isRGB = (actualFormat != _pfCLUT8), swScale = ((_graphicsMode == GFX_SWSCALE) && (width == 320));

	// For Lost in Time, the title screen is displayed in 640x400.
	// In order to support this game, the screen mode is set, but
	// all draw calls are ignored until the game switches to 320x200.
	if (_framebuffer.getRequiredVRAM(width, height, isRGB, swScale) > 0x40000) {
		_framebuffer.create(width, height, isRGB);
	} else {
		_framebuffer.reset();
		_framebuffer.create(width, height, isRGB, 3, false, 8, swScale);
	}

#ifdef DISABLE_TEXT_CONSOLE
	if (_framebuffer.getRequiredVRAM(width, height, isRGB, false) > 0x20000) {
		_subScreen.init(&_framebuffer);
	} else {
		_subScreen.reset();
		_subScreen.init(&_framebuffer, 3, true, 0, false);
	}
#endif

	DS::setGameSize(width, height);
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

		_palette[r] = RGB8(red, green, blue);
		colors += 3;
	}

	_paletteDirty = true;
	if (_disableCursorPalette)
		_cursorDirty = true;
}

void OSystem_DS::setCursorPalette(const byte *colors, uint start, uint num) {

	for (unsigned int r = start; r < start + num; r++) {
		int red = *colors;
		int green = *(colors + 1);
		int blue = *(colors + 2);

		_cursorPalette[r] = RGB8(red, green, blue);
		colors += 3;
	}

	_disableCursorPalette = false;
	_cursorDirty = true;
}

void OSystem_DS::grabPalette(unsigned char *colors, uint start, uint num) const {
	for (unsigned int r = start; r < start + num; r++) {
		*colors++ = (_palette[r] & 0x001F) << 3;
		*colors++ = (_palette[r] & 0x03E0) >> 5 << 3;
		*colors++ = (_palette[r] & 0x7C00) >> 10 << 3;
	}
}

void OSystem_DS::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	_framebuffer.copyRectToSurface(buf, pitch, x, y, w, h);
}

void OSystem_DS::updateScreen() {
	swiWaitForVBlank();
	bgUpdate();

	if (_cursorDirty) {
		refreshCursor(_cursorSprite, _cursor, !_disableCursorPalette ? _cursorPalette : _palette);
		_cursorDirty = false;
	}
	oamSet(&oamMain, 0, _cursorPos.x - _cursorHotX, _cursorPos.y - _cursorHotY, 0, 15, SpriteSize_64x64,
	       SpriteColorFormat_Bmp, _cursorSprite, 0, false, !_cursorVisible, false, false, false);
	oamUpdate(&oamMain);

	if (_overlay.isVisible()) {
		_overlay.update();
	} else {
		if (_paletteDirty) {
			dmaCopyHalfWords(3, _palette, BG_PALETTE, 256 * 2);
#ifdef DISABLE_TEXT_CONSOLE
			dmaCopyHalfWords(3, _palette, BG_PALETTE_SUB, 256 * 2);
#endif
			_paletteDirty = false;
		}

		_framebuffer.update();
#ifdef DISABLE_TEXT_CONSOLE
		_subScreen.update();
#endif
	}
}

void OSystem_DS::setShakePos(int shakeXOffset, int shakeYOffset) {
	DS::setShakePos(shakeXOffset, shakeYOffset);
}

void OSystem_DS::showOverlay() {
	_overlay.reset();
	_overlay.show();
}

void OSystem_DS::hideOverlay() {
	_overlay.hide();
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

Common::Point OSystem_DS::transformPoint(int16 x, int16 y) {
	if (_overlay.isVisible())
		return Common::Point(x, y);
	else
		return _framebuffer.realToScaled(x, y);
}

bool OSystem_DS::showMouse(bool visible) {
	const bool last = _cursorVisible;
	_cursorVisible = visible;
	return last;
}

void OSystem_DS::warpMouse(int x, int y) {
	if (_overlay.isVisible())
		_cursorPos = Common::Point(x, y);
	else
		_cursorPos = _framebuffer.scaledToReal(x, y);
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

	_cursorDirty = true;
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
	DS::setTopScreenTarget(rect.left + rect.width() / 2, rect.top + rect.height() / 2);
}

void OSystem_DS::clearFocusRectangle() {

}
