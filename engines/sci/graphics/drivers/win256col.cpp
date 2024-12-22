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

class WindowsGfx256ColorsDriver final : public UpscaledGfxDriver {
public:
	WindowsGfx256ColorsDriver(bool coloredDosStyleCursors, bool smallWindow, bool rgbRendering);
	~WindowsGfx256ColorsDriver() override {}
	void initScreen(const Graphics::PixelFormat *format) override;
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	Common::Point getRealCoords(Common::Point &pos) const override;
	void setColorMap(const byte *colorMap) override { _colorMap = colorMap; }
	void setFlags(uint32 flags) override;
	void clearFlags(uint32 flags) override;
	bool supportsHiResGraphics() const override { return !_smallWindow; }
	bool driverBasedTextRendering() const override { return false; }
protected:
	typedef void (*LineProc)(byte*&, const byte*, int, int, int);
	LineProc _renderLine;
private:
	typedef void (*LineProcSpec)(byte*&, const byte*, int, int, const byte*);
	LineProcSpec _renderLine2;
	void renderBitmap(const byte *src, int pitch, int dx, int dy, int w, int h, int &realWidth, int &realHeight) override;
	uint32 _flags;
	const byte *_colorMap;
	const bool _smallWindow;
	const bool _dosStyleCursors;
	uint16 _vScaleMult2;
};

WindowsGfx256ColorsDriver::WindowsGfx256ColorsDriver(bool coloredDosStyleCursors, bool smallWindow,bool rgbRendering) :
	UpscaledGfxDriver(smallWindow ? 320 : 640, smallWindow ? 240 : 440, 1, coloredDosStyleCursors && !smallWindow, rgbRendering), _dosStyleCursors(coloredDosStyleCursors), _smallWindow(smallWindow),
		_renderLine(nullptr), _renderLine2(nullptr), _flags(0), _colorMap(nullptr), _vScaleMult2(smallWindow ? 1 : 2) {
	_virtualW = 320;
	_virtualH = 200;
	if (smallWindow)
		_hScaleMult = 1;
	_vScaleMult = _smallWindow ? 6 : 11;
	_vScaleDiv = 5;
}

void largeWindowRenderLine(byte *&dst, const byte *src, int pitch, int w, int ty) {
	int dstPitch = pitch;
	int dstPitch2 = pitch - (w << 1);
	byte *d1 = dst;
	byte *d2 = d1 + dstPitch;

	if (ty == 5) {
		byte *d3 = d2 + dstPitch;
		for (int i = 0; i < w; ++i) {
			d1[0] = d1[1] = d2[0] = d2[1] = d3[0] = d3[1] = *src++;
			d1 += 2;
			d2 += 2;
			d3 += 2;
		}
		dst = d3 + dstPitch2;
	} else {
		for (int i = 0; i < w; ++i) {
			d1[0] = d1[1] = d2[0] = d2[1] = *src++;
			d1 += 2;
			d2 += 2;
		}
		dst = d2 + dstPitch2;
	}
}

void largeWindowRenderLineMovie(byte *&dst, const byte *src, int pitch, int w, const byte*) {
	int dstPitch = pitch;
	int dstPitch2 = pitch - (w << 1);
	byte *d1 = dst;
	byte *d2 = d1 + dstPitch;

	for (int i = 0; i < w; ++i) {
		d1[0] = d1[1] = d2[0] = d2[1] = *src++;
		d1 += 2;
		d2 += 2;
	}
	dst = d2 + dstPitch2;
}

void smallWindowRenderLine(byte *&dst, const byte *src, int pitch, int w, int ty) {
	int dstPitch = pitch;
	int dstPitch2 = pitch - w;
	byte *d1 = dst;

	if (ty == 5) {
		byte *d2 = d1 + dstPitch;
		for (int i = 0; i < w; ++i)
			*d1++ = *d2++ = *src++;
		dst = d2 + dstPitch2;
	} else {
		for (int i = 0; i < w; ++i)
			*d1++ = *src++;
		dst = d1 + dstPitch2;
	}
}

void smallWindowRenderLineMovie(byte *&dst, const byte *src, int pitch, int w, const byte*) {
	int dstPitch = pitch - w;
	byte *d1 = dst;

	for (int i = 0; i < w; ++i)
		*d1++ = *src++;
	dst = d1 + dstPitch;
}

void hiresRenderLine(byte *&dst, const byte *src, int pitch, int w, const byte *colorMap) {
	if (!colorMap) {
		memcpy(dst, src, w);
	} else {
		byte *d = dst;
		for (int i = 0; i < w; ++i)
			*d++ = colorMap[*src++];
	}
	dst += pitch;
}

void renderLineDummy(byte *&, const byte* , int, int, const byte*) {
}

void WindowsGfx256ColorsDriver::initScreen(const Graphics::PixelFormat *format) {
	UpscaledGfxDriver::initScreen(format);
	_renderLine = _smallWindow ? &smallWindowRenderLine : &largeWindowRenderLine;
	_renderLine2 = _smallWindow ? &renderLineDummy : &hiresRenderLine;
}

void WindowsGfx256ColorsDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	assert (h >= 0 && w >= 0);

	if (!(_flags & (kHiResMode | kMovieMode))) {
		UpscaledGfxDriver::copyRectToScreen(src, srcX, srcY, pitch, destX, destY, w, h, palMods, palModMapping);
		return;
	}

	if (_flags & kMovieMode) {
		destX = (_screenW >> 1) - (w & ~1) * _hScaleMult / 2;
		destY = (_screenH >> 1) - (h & ~1) * _vScaleMult2 / 2;
	}

	src += (srcY * pitch + srcX * _srcPixelSize);
	byte *dst = _scaledBitmap + destY * _screenW * _srcPixelSize + destX * _srcPixelSize;

	for (int i = 0; i < h; ++i) {
		_renderLine2(dst, src, _screenW, w, _colorMap);
		src += pitch;
	}

	if (_flags & kMovieMode) {
		w *= _hScaleMult;
		h *= _vScaleMult2;
	}

	updateScreen(destX, destY, w, h, palMods, palModMapping);
}

void WindowsGfx256ColorsDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	if (_dosStyleCursors) {
		// The original windows interpreter always renders the cursor as b/w, regardless of which cursor views (the DOS
		// cursors or the new Windows ones) the user selects. This is also regardless of color mode (16 or 256 colors).
		// It is a technical limitation on Windows 95 with VGA hardware that mouse cursors have to be b/w.
		// Instead, we use the colored DOS style cursors as a default, since there was consensus to do that.
		UpscaledGfxDriver::replaceCursor(cursor, w, h, hotspotX, hotspotY, keycolor);
		return;
	}
	adjustCursorBuffer(w << 1, h << 1);

	if (_pixelSize == 1)
		copyCurrentPalette(_currentPalette, 0, _numColors);

	byte col1 = SciGfxDrvInternal::findColorInPalette(0x00000000, _currentPalette, _numColors);
	byte col2 = SciGfxDrvInternal::findColorInPalette(0x00FFFFFF, _currentPalette, _numColors);
	SciGfxDrvInternal::renderWinMonochromeCursor(_compositeBuffer, cursor, _currentPalette, w, h, hotspotX, hotspotY, col1, col2, keycolor, _smallWindow);
	CursorMan.replaceCursor(_compositeBuffer, w, h, hotspotX, hotspotY, keycolor);
}

Common::Point WindowsGfx256ColorsDriver::getRealCoords(Common::Point &pos) const {
	return Common::Point(pos.x * _hScaleMult, pos.y * _vScaleMult2 + (pos.y + 4) / 5);
}

void WindowsGfx256ColorsDriver::setFlags(uint32 flags) {
	flags ^= (_flags & flags);
	if (!flags)
		return;

	if (flags & kMovieMode)
		_renderLine2 = _smallWindow ? &smallWindowRenderLineMovie : &largeWindowRenderLineMovie;

	_flags |= flags;
}

void WindowsGfx256ColorsDriver::clearFlags(uint32 flags) {
	flags &= _flags;
	if (!flags)
		return;

	if (flags & kMovieMode)
		_renderLine2 = _smallWindow ? &renderLineDummy : &hiresRenderLine;

	_flags &= ~flags;
}

void WindowsGfx256ColorsDriver::renderBitmap(const byte *src, int pitch, int dx, int dy, int w, int h, int &realWidth, int &realHeight) {
	assert(_renderLine);

	byte *dst = _scaledBitmap + (dy * _vScaleMult2 + (dy + 4) / 5) * _screenW * _srcPixelSize + dx *_hScaleMult * _srcPixelSize;
	const byte *dstart = dst;
	dy = (dy + 4) % 5;

	while (h--) {
		_renderLine(dst, src, _screenW, w, ++dy);
		dy %= 5;
		src += pitch;
	}

	realWidth = w * _hScaleMult;
	realHeight = (dst - dstart) / _screenW;
}

GfxDriver *WindowsGfx256ColorsDriver_create(int rgbRendering, ...) {
	va_list args;
	va_start(args, rgbRendering);
	int config = va_arg(args, int);
	va_arg(args, int);
	va_arg(args, int);
	va_arg(args, int);
	bool winCursors = (va_arg(args, int) != 0);
	va_end(args);

	return new WindowsGfx256ColorsDriver(!winCursors, config == 0, rgbRendering != 0);
}

} // End of namespace Sci
