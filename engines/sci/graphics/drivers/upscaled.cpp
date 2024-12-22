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


#include "common/system.h"
#include "graphics/cursorman.h"
#include "sci/graphics/drivers/gfxdriver_intern.h"

namespace Sci {

UpscaledGfxDriver::UpscaledGfxDriver(int16 textAlignX, bool scaleCursor, bool rgbRendering) :
	UpscaledGfxDriver(640, 400, textAlignX, scaleCursor, rgbRendering) {
}

UpscaledGfxDriver::UpscaledGfxDriver(uint16 scaledW, uint16 scaledH, int16 textAlignX, bool scaleCursor, bool rgbRendering) :
	GfxDefaultDriver(scaledW, scaledH, false, rgbRendering), _textAlignX(textAlignX), _scaleCursor(scaleCursor), _needCursorBuffer(false),
	_scaledBitmap(nullptr), _renderScaled(nullptr), _renderGlyph(nullptr), _cursorWidth(0), _cursorHeight(0), _hScaleMult(2), _vScaleMult(2), _vScaleDiv(1) {
	_virtualW = 320;
	_virtualH = 200;
}

UpscaledGfxDriver::~UpscaledGfxDriver() {
	delete[] _scaledBitmap;
}

void renderGlyph(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h, int transpCol) {
	dstPitch -= w;
	srcPitch -= w;

	while (h--) {
		for (int i = 0; i < w; ++i) {
			byte in = *src++;
			if (in != transpCol)
				*dst = in;
			++dst;
		}
		src += srcPitch;
		dst += dstPitch;
	}
}

void UpscaledGfxDriver::initScreen(const Graphics::PixelFormat *format) {
	GfxDefaultDriver::initScreen(format);
	_scaledBitmap = new byte[_screenW * _screenH * _srcPixelSize]();

	static const ScaledRenderProc scaledRenderProcs[] = {
		&SciGfxDrvInternal::scale2x<byte>,
		&SciGfxDrvInternal::scale2x<uint16>,
		&SciGfxDrvInternal::scale2x<uint32>
	};
	assert((_srcPixelSize >> 1) < ARRAYSIZE(scaledRenderProcs));
	_renderScaled = scaledRenderProcs[_srcPixelSize >> 1];
	_renderGlyph = &renderGlyph;
}

void UpscaledGfxDriver::setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	if (_pixelSize == 1) {
		GfxDefaultDriver::setPalette(colors, start, num, update, palMods, palModMapping);
		return;
	}
	updatePalette(colors, start, num);
	if (update)
		updateScreen(0, 0, _screenW, _screenH, palMods, palModMapping);
	if (_cursorUsesScreenPalette)
		CursorMan.replaceCursorPalette(_currentPalette, 0, 256);
}

void UpscaledGfxDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	assert (h >= 0 && w >= 0);

	src += (srcY * pitch + srcX * _srcPixelSize);
	if (src != _currentBitmap)
		SciGfxDrvInternal::updateBitmapBuffer(_currentBitmap, _virtualW * _srcPixelSize, src, pitch, destX * _srcPixelSize, destY, w * _srcPixelSize, h);

	int realWidth = 0;
	int realHeight = 0;

	// We need to scale and color convert the bitmap in separate functions, because we want
	// to keep the scaled non-color-modified bitmap for palette updates in rgb rendering mode.
	renderBitmap(src, pitch, destX, destY, w, h, realWidth, realHeight);

	Common::Point p(destX, destY);
	p = getRealCoords(p);

	updateScreen(p.x, p.y, realWidth, realHeight, palMods, palModMapping);
}

void UpscaledGfxDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	if (_scaleCursor) {
		adjustCursorBuffer(w << 1, h << 1);
		SciGfxDrvInternal::scale2x<byte>(_compositeBuffer, reinterpret_cast<const byte*>(cursor), w, w, h);
		CursorMan.replaceCursor(_compositeBuffer, w << 1, h << 1, hotspotX << 1, hotspotY << 1, keycolor);
	} else {
		CursorMan.replaceCursor(cursor, w, h, hotspotX, hotspotY, keycolor);
	}
}

Common::Point UpscaledGfxDriver::getMousePos() const {
	Common::Point res = GfxDriver::getMousePos();
	res.x /= _hScaleMult;
	res.y = res.y * _vScaleDiv / _vScaleMult;
	return res;
}

void UpscaledGfxDriver::setMousePos(const Common::Point &pos) const {
	g_system->warpMouse(pos.x * _hScaleMult, pos.y * _vScaleMult / _vScaleDiv);
}

void UpscaledGfxDriver::setShakePos(int shakeXOffset, int shakeYOffset) const {
	g_system->setShakePos(shakeXOffset * _hScaleMult, shakeYOffset * _vScaleMult / _vScaleDiv);
}

void UpscaledGfxDriver::clearRect(const Common::Rect &r) const {
	Common::Rect r2(r.left * _hScaleMult, r.top * _vScaleMult / _vScaleDiv, r.right * _hScaleMult, r.bottom * _vScaleMult / _vScaleDiv);
	GfxDriver::clearRect(r2);
}

Common::Point UpscaledGfxDriver::getRealCoords(Common::Point &pos) const {
	return Common::Point(pos.x * _hScaleMult, pos.y * _vScaleMult / _vScaleDiv);
}

void UpscaledGfxDriver::drawTextFontGlyph(const byte *src, int pitch, int hiresDestX, int hiresDestY, int hiresW, int hiresH, int transpColor, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	hiresDestX &= ~(_textAlignX - 1);
	byte *scb = _scaledBitmap + hiresDestY * _screenW * _srcPixelSize + hiresDestX * _srcPixelSize;
	_renderGlyph(scb, _screenW, src, pitch, hiresW, hiresH, transpColor);
	updateScreen(hiresDestX, hiresDestY, hiresW, hiresH, palMods, palModMapping);
}

void UpscaledGfxDriver::updateScreen(int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	byte *buff = _compositeBuffer;
	int pitch = w * _pixelSize;
	byte *scb = _scaledBitmap + destY * _screenW * _srcPixelSize + destX * _srcPixelSize;
	if (palMods && palModMapping) {
		_colorConvMod(buff, scb, _screenW, w, h, _currentPalette, _internalPalette, _format, palMods, palModMapping);
	} else if (_pixelSize != _srcPixelSize) {
		_colorConv(buff, scb, _screenW, w, h, _internalPalette);
	} else {
		buff = scb;
		pitch = _screenW *_pixelSize;
	}

	g_system->copyRectToScreen(buff, pitch, destX, destY, w, h);
}

void UpscaledGfxDriver::adjustCursorBuffer(uint16 newWidth, uint16 newHeight) {
	// For configs which need/have the composite buffer for other purposes, we can skip this.
	if (!_compositeBuffer)
		_needCursorBuffer = true;
	else if (!_needCursorBuffer)
		return;

	if (_cursorWidth * _cursorHeight < newWidth * newHeight) {
		delete[] _compositeBuffer;
		_compositeBuffer = new byte[newWidth * newHeight * _srcPixelSize]();
		_cursorWidth = newWidth;
		_cursorHeight = newHeight;
	}
}

void UpscaledGfxDriver::renderBitmap(const byte *src, int pitch, int dx, int dy, int w, int h, int &realWidth, int &realHeight) {
	byte *scb = _scaledBitmap + (dy << 1) * _screenW * _srcPixelSize + (dx << 1) * _srcPixelSize;
	_renderScaled(scb, src, pitch, w, h);
	realWidth = w << 1;
	realHeight = h << 1;
}

GfxDriver *UpscaledGfxDriver_create(int rgbRendering, ...) {
	return new UpscaledGfxDriver(1, true, rgbRendering != 0);
}

} // End of namespace Sci
