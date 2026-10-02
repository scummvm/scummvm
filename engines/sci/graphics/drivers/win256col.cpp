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


#include "common/array.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "sci/graphics/drivers/gfxdriver_intern.h"

namespace Sci {

// In sharp mode the screen is 3200x2200 instead of 640x440, which is the smallest size where both the 320x200 lowres
// graphics (10x11 pixel blocks) and the 640x440 hires graphics (5x5 pixel blocks) get scaled by integer factors.
class WindowsGfx256ColorsDriver final : public UpscaledGfxDriver {
public:
	WindowsGfx256ColorsDriver(bool coloredDosStyleCursors, bool smallWindow, bool sharpScaling, bool rgbRendering);
	~WindowsGfx256ColorsDriver() override {}
	bool initScreen(const Graphics::PixelFormat *format) override;
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
	Common::Point getScreenCoords(Common::Point pos) const override;
	uint32 _flags;
	const byte *_colorMap;
	const bool _smallWindow;
	const bool _dosStyleCursors;
	uint16 _vScaleMult2;
	// Size of a 640x440 hires pixel on the screen (5 in sharp mode, 1 otherwise)
	const uint16 _hiresScale;
	Common::Array<byte> _sharpCursor;
};

WindowsGfx256ColorsDriver::WindowsGfx256ColorsDriver(bool coloredDosStyleCursors, bool smallWindow, bool sharpScaling, bool rgbRendering) :
	UpscaledGfxDriver(smallWindow ? 320 : (sharpScaling ? 3200 : 640), smallWindow ? 240 : (sharpScaling ? 2200 : 440), 1, coloredDosStyleCursors && !smallWindow, rgbRendering), _dosStyleCursors(coloredDosStyleCursors), _smallWindow(smallWindow),
		_renderLine(nullptr), _renderLine2(nullptr), _flags(0), _colorMap(nullptr), _vScaleMult2(smallWindow ? 1 : 2), _hiresScale((sharpScaling && !smallWindow) ? 5 : 1) {
	_virtualW = 320;
	_virtualH = 200;
	if (smallWindow)
		_hScaleMult = 1;
	_vScaleMult = _smallWindow ? 6 : 11;
	_vScaleDiv = 5;
	if (_hiresScale > 1) {
		_hScaleMult = 10;
		_vScaleDiv = 1;
	}
}

static void renderScaledBlocks(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h, int hScale, int vScale, const byte *colorMap) {
	const int rowBytes = w * hScale;
	while (h--) {
		byte *d = dst;
		for (int i = 0; i < w; ++i) {
			memset(d, colorMap ? colorMap[src[i]] : src[i], hScale);
			d += hScale;
		}
		for (int i = 1; i < vScale; ++i)
			memcpy(dst + i * dstPitch, dst, rowBytes);
		src += srcPitch;
		dst += dstPitch * vScale;
	}
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

bool WindowsGfx256ColorsDriver::initScreen(const Graphics::PixelFormat *format) {
	if (!UpscaledGfxDriver::initScreen(format))
		return false;

	_renderLine = _smallWindow ? &smallWindowRenderLine : &largeWindowRenderLine;
	_renderLine2 = _smallWindow ? &renderLineDummy : &hiresRenderLine;

	return true;
}

void WindowsGfx256ColorsDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	assert (h >= 0 && w >= 0);

	if (!(_flags & (kHiResMode | kMovieMode))) {
		UpscaledGfxDriver::copyRectToScreen(src, srcX, srcY, pitch, destX, destY, w, h, palMods, palModMapping);
		return;
	}

	if (_hiresScale > 1) {
		// Movies are shown at 2x the lowres size, other hires content at 1x (in 640x440 hires space).
		const bool movie = _flags & kMovieMode;
		const int scale = movie ? _hiresScale * 2 : _hiresScale;
		if (movie) {
			destX = (_screenW >> 1) - (w & ~1) * scale / 2;
			destY = (_screenH >> 1) - (h & ~1) * scale / 2;
		} else {
			destX *= scale;
			destY *= scale;
		}
		src += (srcY * pitch + srcX);
		renderScaledBlocks(_scaledBitmap + destY * _screenW + destX, _screenW, src, pitch, w, h, scale, scale, movie ? nullptr : _colorMap);
		updateScreen(destX, destY, w * scale, h * scale, palMods, palModMapping);
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
	if (_hiresScale > 1) {
		// Build the cursor in 640x440 hires space like the normal mode does, then scale it up to the screen.
		const byte *hiresCursor = reinterpret_cast<const byte*>(cursor);
		int scale = _hiresScale;
		if (_dosStyleCursors) {
			scale *= 2;
		} else {
			adjustCursorBuffer(w << 1, h << 1);
			if (_pixelSize == 1)
				copyCurrentPalette(_currentPalette, 0, _numColors);
			byte col1 = SciGfxDrvInternal::findColorInPalette(0x00000000, _currentPalette, _numColors);
			byte col2 = SciGfxDrvInternal::findColorInPalette(0x00FFFFFF, _currentPalette, _numColors);
			SciGfxDrvInternal::renderWinMonochromeCursor(_compositeBuffer, cursor, _currentPalette, w, h, hotspotX, hotspotY, col1, col2, keycolor, false);
			hiresCursor = _compositeBuffer;
		}
		_sharpCursor.resize(w * h * scale * scale);
		renderScaledBlocks(_sharpCursor.data(), w * scale, hiresCursor, w, w, h, scale, scale, nullptr);
		CursorMan.replaceCursor(_sharpCursor.data(), w * scale, h * scale, hotspotX * scale, hotspotY * scale, keycolor);
		return;
	}
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
	return Common::Point(pos.x * (_smallWindow ? 1 : 2), pos.y * _vScaleMult2 + (pos.y + 4) / 5);
}

Common::Point WindowsGfx256ColorsDriver::getScreenCoords(Common::Point pos) const {
	if (_hiresScale > 1)
		return Common::Point(pos.x * _hScaleMult, pos.y * _vScaleMult);
	return getRealCoords(pos);
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
	if (_hiresScale > 1) {
		renderScaledBlocks(_scaledBitmap + dy * _vScaleMult * _screenW + dx * _hScaleMult, _screenW, src, pitch, w, h, _hScaleMult, _vScaleMult, nullptr);
		realWidth = w * _hScaleMult;
		realHeight = h * _vScaleMult;
		return;
	}

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

	bool sharpScaling = ConfMan.hasKey("enable_sharp_hires_scaling") && ConfMan.getBool("enable_sharp_hires_scaling");
	return new WindowsGfx256ColorsDriver(!winCursors, config == 0, sharpScaling, rgbRendering != 0);
}

} // End of namespace Sci
