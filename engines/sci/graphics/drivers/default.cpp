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
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "sci/graphics/drivers/gfxdriver_intern.h"
#include "sci/resource/resource.h"

namespace Sci {

GfxDefaultDriver::GfxDefaultDriver(uint16 screenWidth, uint16 screenHeight, bool isSCI0, bool rgbRendering) : GfxDriver(screenWidth, screenHeight, 0), _cursorUsesScreenPalette(true),  _colorConv(nullptr), _colorConvMod(nullptr),
	_srcPixelSize(1), _requestRGBMode(rgbRendering), _compositeBuffer(nullptr), _currentBitmap(nullptr), _internalPalette(nullptr), _currentPalette(nullptr), _virtualW(screenWidth), _virtualH(screenHeight), _alwaysCreateBmpBuffer(!isSCI0) {
	switch (g_sci->getResMan()->getViewType()) {
	case kViewEga:
		_numColors = 16;	// QFG PC-98 with 8 colors also reports 16 here
		break;
	case kViewAmiga:
		_numColors = 32;
		break;
	case kViewAmiga64:
		_numColors = 64;
		break;
	case kViewVga:
	case kViewVga11:
		_numColors = 256;
		break;
	default:
		break;
	}

	if (_numColors == 0)
		error("GfxDefaultDriver: Unknown view type");
}

GfxDefaultDriver::~GfxDefaultDriver() {
	delete[] _compositeBuffer;
	delete[] _currentBitmap;
	delete[] _internalPalette;
	delete[] _currentPalette;
}

template <typename T> void colorConvert(byte *dst, const byte *src, int pitch, int w, int h, const byte *pal) {
	T *d = reinterpret_cast<T*>(dst);
	const T *p = reinterpret_cast<const T*>(pal);
	const byte *s = src;
	pitch -= w;

	while (h--) {
		for (int i = 0; i < w; ++i)
			*d++ = p[*s++];
		s += pitch;
	}
}

#define applyMod(a, b) MIN<uint>(a * (128 + b) / 128, 255)
template <typename T> void colorConvertMod(byte *dst, const byte *src, int pitch, int w, int h, const byte *srcPal, const byte *internalPal, Graphics::PixelFormat &f, const PaletteMod *mods, const byte *modMapping) {
	T *d = reinterpret_cast<T*>(dst);
	const T *p = reinterpret_cast<const T*>(internalPal);
	const byte *s1 = src;
	const byte *s2 = modMapping;
	pitch -= w;

	while (h--) {
		for (int i = 0; i < w; ++i) {
			byte m = *s2++;
			if (m) {
				const byte *col = &srcPal[*s1++ * 3];
				*d++ = f.RGBToColor(applyMod(col[0], mods[m].r), applyMod(col[1], mods[m].g), applyMod(col[2], mods[m].b));
			} else {
				*d++ = p[*s1++];
			}
		}
		s1 += pitch;
		s2 += pitch;
	}
}
#undef applyMod

void GfxDefaultDriver::initScreen(const Graphics::PixelFormat *srcRGBFormat) {
	Graphics::PixelFormat format8bt(Graphics::PixelFormat::createFormatCLUT8());
	initGraphics(_screenW, _screenH, srcRGBFormat ? srcRGBFormat : (_requestRGBMode ? nullptr : &format8bt));
	_format = g_system->getScreenFormat();

	int srcPixelSize = srcRGBFormat ? _format.bytesPerPixel : 1;
	if (srcPixelSize != _srcPixelSize || _pixelSize != _format.bytesPerPixel) {
		delete[] _compositeBuffer;
		delete[] _currentBitmap;
		delete[] _internalPalette;
		delete[] _currentPalette;
		_compositeBuffer = _currentBitmap = _internalPalette = _currentPalette = nullptr;
	}

	_pixelSize = _format.bytesPerPixel;
	_srcPixelSize = srcPixelSize;

	if (_requestRGBMode && _pixelSize == 1)
		warning("GfxDefaultDriver::initScreen(): RGB rendering not available in this ScummVM build");

	if (_pixelSize != _srcPixelSize) {
		uint32 bufferSize = _screenW * _screenH * _pixelSize;
		_compositeBuffer = new byte[bufferSize]();
		assert(_compositeBuffer);
	}

	// Not needed for SCI0, except for rgb rendering. Unfortunately, SCI_VERSION_01
	// does need it and we can't tell the version from the number of colors there.
	// That's why we have the _alwaysCreateBmpBuffer flag...
	if (_alwaysCreateBmpBuffer || _numColors > 16 || _pixelSize > 1) {
		_currentBitmap = new byte[_virtualW * _virtualH * _srcPixelSize]();
		assert(_currentBitmap);
	}

	if (_numColors > 16 || _pixelSize > 1) {
		// Not needed for SCI0, except for rgb rendering
		_currentPalette = new byte[256 * 3]();
		assert(_currentPalette);
		if (_pixelSize != _srcPixelSize) {
			_internalPalette = new byte[256 * _pixelSize]();
			assert(_internalPalette);
		}
	}

	static const ColorConvProc colorConvProcs[] = {
		&colorConvert<byte>,
		&colorConvert<uint16>,
		&colorConvert<uint32>
	};
	assert((_pixelSize >> 1) < ARRAYSIZE(colorConvProcs));
	_colorConv = colorConvProcs[_pixelSize >> 1];

	static const ColorConvModProc colorConvModProcs[] = {
		&colorConvertMod<byte>,
		&colorConvertMod<uint16>,
		&colorConvertMod<uint32>
	};
	assert((_pixelSize >> 1) < ARRAYSIZE(colorConvModProcs));
	_colorConvMod = colorConvModProcs[_pixelSize >> 1];

	_ready = true;
}

void GfxDefaultDriver::setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	if (_pixelSize > 1) {
		updatePalette(colors, start, num);
		if (update)
			copyRectToScreen(_currentBitmap, 0, 0, _virtualW, 0, 0, _virtualW, _virtualH, palMods, palModMapping);
		if (_cursorUsesScreenPalette)
			CursorMan.replaceCursorPalette(_currentPalette, 0, 256);
	} else {
		g_system->getPaletteManager()->setPalette(colors, start, num);
	}
}

void GfxDefaultDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	assert (h >= 0 && w >= 0);

	src += (srcY * pitch + srcX * _srcPixelSize);
	if (src != _currentBitmap)
		SciGfxDrvInternal::updateBitmapBuffer(_currentBitmap, _screenW * _srcPixelSize, src, pitch, destX * _srcPixelSize, destY, w * _srcPixelSize, h);

	if (_pixelSize != _srcPixelSize) {
		generateOutput(_compositeBuffer, src, pitch, w, h, palMods, palModMapping + destY * pitch + destX);
		src = _compositeBuffer;
		pitch = w * _pixelSize;
	}

	g_system->copyRectToScreen(src, pitch, destX, destY, w, h);
}

void GfxDefaultDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	CursorMan.replaceCursor(cursor, w, h, hotspotX, hotspotY, keycolor);
	if (_pixelSize > 1 && _currentPalette != nullptr)
		CursorMan.replaceCursorPalette(_currentPalette, 0, 256);
}

void GfxDefaultDriver::replaceMacCursor(const Graphics::Cursor*) {
	// This is not needed for any non-Mac version of games at all)
	error("GfxDefaultDriver::replaceMacCursor(Graphics::Cursor*): Not implemented");
}

void GfxDefaultDriver::copyCurrentBitmap(byte *dest, uint32 size) const {
	GFXDRV_ASSERT_READY;
	assert(dest);
	assert(size <= (uint32)(_screenW * _screenH));

	// SCI 0 should not make calls to this method (except when using palette mods), but we have to know if it does...
	if (!_currentBitmap)
		error("GfxDefaultDriver::copyCurrentBitmap(): unexpected call");

	// I have changed the implementation a bit from what the engine did before. For non-rgb rendering
	// it would call OSystem::lockScreen() and then memcpy the data from there (which avoided the need
	// for the extra bitmap buffer). However, OSystem::lockScreen() is meant more as an update method
	// for the screen, the call to OSystem::unlockScreen() will turn the whole screen dirty (to be
	// updated on the next OSysten::updateScreen() call. This is not what we need here, so I rather use
	// the extra bitmap buffer (which is required for rgb rendering anyway).
	memcpy(dest, _currentBitmap, size);
}

void GfxDefaultDriver::copyCurrentPalette(byte *dest, int start, int num) const {
	GFXDRV_ASSERT_READY;

	if (_pixelSize == 1) {
		GfxDriver::copyCurrentPalette(dest, start, num);
		return;
	}

	assert(dest);
	assert(_currentPalette);
	assert(start + num <= 256);
	memcpy(dest + start * 3, _currentPalette + start * 3, num * 3);
}

void GfxDefaultDriver::drawTextFontGlyph(const byte*, int, int, int, int, int, int, const PaletteMod*, const byte*) {
	// This is only needed for scaling drivers with unscaled hires fonts.
	error("GfxDefaultDriver::drawTextFontGlyph(): Not implemented");
}

void GfxDefaultDriver::updatePalette(const byte *colors, uint start, uint num) {
	memcpy(_currentPalette + start * 3, colors, num * 3);
	if (_pixelSize == 4)
		SciGfxDrvInternal::updateRGBPalette<uint32>(_internalPalette, colors, start, num, _format);
	else if (_pixelSize == 2)
		SciGfxDrvInternal::updateRGBPalette<uint16>(_internalPalette, colors, start, num, _format);
	else
		error("GfxDefaultDriver::updatePalette(): Unsupported pixel size %d", _pixelSize);
}

void GfxDefaultDriver::generateOutput(byte *dst, const byte *src, int pitch, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	if (palMods && palModMapping)
		_colorConvMod(dst, src, pitch, w, h, _currentPalette, _internalPalette, _format, palMods, palModMapping);
	else
		_colorConv(dst, src, pitch, w, h, _internalPalette);
}

GfxDriver *GfxDefaultDriver_create(int rgbRendering, ...) {
	va_list args;
	va_start(args, rgbRendering);
	int config = va_arg(args, int);
	int width = va_arg(args, int);
	int height = va_arg(args, int);
	va_end(args);

	return new GfxDefaultDriver(width, height, config == 0, rgbRendering != 0);
}

} // End of namespace Sci
