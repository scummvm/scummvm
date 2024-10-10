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

#include "common/events.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "sci/graphics/gfxdrivers.h"
#include "sci/resource/resource.h"
#include "sci/sci.h"

namespace Sci {

#define GFXDRV_ASSERT_READY \
	if (!_ready) \
		error("%s: initScreen() must be called before using this method", __FUNCTION__)

Common::Point GfxDriver::getMousePos() const {
	return g_system->getEventManager()->getMousePos();
}

void GfxDriver::setMousePos(const Common::Point &pos) const {
	g_system->warpMouse(pos.x, pos.y);
}

void GfxDriver::setShakePos(int shakeXOffset, int shakeYOffset) const {
	g_system->setShakePos(shakeXOffset, shakeYOffset);
}

void GfxDriver::clearRect(const Common::Rect &r) const {
	GFXDRV_ASSERT_READY;
	g_system->fillScreen(r, 0);
}

void GfxDriver::copyCurrentPalette(byte *dest, int start, int num) const {
	assert(dest);
	assert(start + num <= 256);
	g_system->getPaletteManager()->grabPalette(dest, start, num);
}

bool GfxDriver::checkDriver(const char *const *driverNames, int listSize) {
	Common::String missing;
	while (listSize-- && *driverNames) {
		if (Common::File::exists(*driverNames))
			return true;
		if (!missing.empty())
			missing += " or ";
		missing += "'" + Common::String(*driverNames) + "'";
		++driverNames;
	}
	warning("Driver file %s not found. Starting game in default mode", missing.c_str());
	return false;
}

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

void updateBitmapBuffer(byte *dst, int dstPitch, const byte *src, int srcPitch, int x, int y, int w, int h) {
	if (!dst)
		return;

	if (w == srcPitch && w == dstPitch) {
		memcpy(dst + y * w, src, w * h);
	} else {
		const byte *s = src;
		byte *d = dst + y * dstPitch + x;
		for (int i = 0; i < h; ++i) {
			memcpy(d, s, w);
			s += srcPitch;
			d += dstPitch;
		}
	}
}

void GfxDefaultDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	assert (h >= 0 && w >= 0);

	src += (srcY * pitch + srcX * _srcPixelSize);
	if (src != _currentBitmap)
		updateBitmapBuffer(_currentBitmap, _screenW * _srcPixelSize, src, pitch, destX * _srcPixelSize, destY, w * _srcPixelSize, h);

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

template <typename T> void updateRGBPalette(byte *dest, const byte *src, uint start, uint num, Graphics::PixelFormat &f) {
	T *dst = &reinterpret_cast<T*>(dest)[start];
	for (uint i = 0; i < num; ++i) {
		*dst++ = f.RGBToColor(src[0], src[1], src[2]);
		src += 3;
	}
}

void GfxDefaultDriver::updatePalette(const byte *colors, uint start, uint num) {
	memcpy(_currentPalette + start * 3, colors, num * 3);
	if (_pixelSize == 4)
		updateRGBPalette<uint32>(_internalPalette, colors, start, num, _format);
	else if (_pixelSize == 2)
		updateRGBPalette<uint16>(_internalPalette, colors, start, num, _format);
	else
		error("GfxDefaultDriver::updatePalette(): Unsupported pixel size %d", _pixelSize);
}

void GfxDefaultDriver::generateOutput(byte *dst, const byte *src, int pitch, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	if (palMods && palModMapping)
		_colorConvMod(dst, src, pitch, w, h, _currentPalette, _internalPalette, _format, palMods, palModMapping);
	else
		_colorConv(dst, src, pitch, w, h, _internalPalette);
}

SCI0_DOSPreVGADriver::SCI0_DOSPreVGADriver(int numColors, int screenW, int screenH, bool rgbRendering) :
	GfxDriver(screenW, screenH, numColors), _requestRGBMode(rgbRendering), _colors(nullptr), _compositeBuffer(nullptr), _internalPalette(nullptr) {
}

SCI0_DOSPreVGADriver::~SCI0_DOSPreVGADriver() {
	delete[] _compositeBuffer;
	delete[] _internalPalette;
}

void SCI0_DOSPreVGADriver::assignPalette(const byte *colors) {
	_colors = colors;
}

void SCI0_DOSPreVGADriver::initScreen(const Graphics::PixelFormat*) {
	Graphics::PixelFormat format(Graphics::PixelFormat::createFormatCLUT8());
	initGraphics(_screenW, _screenH, _requestRGBMode ? nullptr : &format);
	format = g_system->getScreenFormat();
	_pixelSize = format.bytesPerPixel;

	if (_requestRGBMode && _pixelSize == 1)
		warning("SCI0_DOSPreVGADriver::initScreen(): RGB rendering not available in this ScummVM build");

	delete[] _compositeBuffer;
	delete[] _internalPalette;
	_internalPalette = nullptr;
	_compositeBuffer = nullptr;

	assert(_colors);
	if (_pixelSize == 1) {
		g_system->getPaletteManager()->setPalette(_colors, 0, _numColors);
	} else {
		byte *rgbpal = new byte[_numColors * _pixelSize]();
		assert(rgbpal);

		if (_pixelSize == 2)
			updateRGBPalette<uint16>(rgbpal, _colors, 0, _numColors, format);
		else if (_pixelSize == 4)
			updateRGBPalette<uint32>(rgbpal, _colors, 0, _numColors, format);
		else
			error("SCI0_DOSPreVGADriver::initScreen(): Unsupported screen format");
		_internalPalette = rgbpal;
		CursorMan.replaceCursorPalette(_colors, 0, _numColors);
	}

	_compositeBuffer = new byte[_screenW * _screenH * _pixelSize]();
	assert(_compositeBuffer);

	setupRenderProc();

	_ready = true;
}

void SCI0_DOSPreVGADriver::replaceMacCursor(const Graphics::Cursor*) {
	// This is not needed for SCI0 (and not for any PC version of games at all)
	error("SCI0_DOSPreVGADriver::replaceMacCursor(Graphics::Cursor*): Not implemented");
}

void SCI0_DOSPreVGADriver::copyCurrentBitmap(byte*, uint32) const {
	// This is not needed for SCI0
	error("SCI0_DOSPreVGADriver::copyCurrentBitmap(): Not implemented");
}

void SCI0_DOSPreVGADriver::copyCurrentPalette(byte *dest, int start, int num) const {
	GFXDRV_ASSERT_READY;

	if (_pixelSize == 1) {
		GfxDriver::copyCurrentPalette(dest, start, num);
		return;
	}

	assert(dest);
	memcpy(dest + start * 3, _colors + start * 3, MIN<int>(num, _numColors) * 3);
}

void SCI0_DOSPreVGADriver::drawTextFontGlyph(const byte*, int, int, int, int, int, int, const PaletteMod*, const byte*) {
	// This is only needed for scaling drivers with unscaled hires fonts.
	error("SCI0_DOSPreVGADriver::drawTextFontGlyph(): Not implemented");
}

SCI0_CGADriver::SCI0_CGADriver(bool emulateCGAModeOnEGACard, bool rgbRendering) : SCI0_DOSPreVGADriver(4, 320, 200, rgbRendering), _cgaPatterns(nullptr), _disableMode5(emulateCGAModeOnEGACard), _renderLine(nullptr) {
	static const byte cgaColors[48] = {
		/*
		// Canonical CGA palette
		0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0x00, 0xAA, 0xAA,
		0xAA, 0x00, 0x00, 0xAA, 0x00, 0xAA, 0xAA, 0x55, 0x00, 0xAA, 0xAA, 0xAA,
		0x55, 0x55, 0x55, 0x55, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x55, 0xFF, 0xFF,
		0xFF, 0x55, 0x55, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF
		*/
		// Improved palette model taken from https://int10h.org/blog/2022/06/ibm-5153-color-true-cga-palette/
		0x00, 0x00, 0x00, 0x00, 0x00, 0xC4, 0x00, 0xC4, 0x00, 0x00, 0xC4, 0xC4,
		0xC4, 0x00, 0x00, 0xC4, 0x00, 0xC4, 0xC4, 0x7E, 0x00, 0xC4, 0xC4, 0xC4,
		0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0xDC, 0x4E, 0xDC, 0x4E, 0x4E, 0xF3, 0xF3,
		0xDC, 0x4E, 0x4E, 0xF3, 0x4E, 0xF3, 0xF3, 0xF3, 0x4E, 0xFF, 0xFF, 0xFF
	};

	static const byte modeColorMap[3][4] = {
		{ 0, 2, 4, 6 },
		{ 0, 3, 5, 7 },
		{ 0, 3, 4, 7 }
	};

	Common::File drv;
	if (!drv.open(_driverFile))
		error("Failed to open '%s'", _driverFile);

	byte palIndex = 1;
	byte palIntensity = 1;
	byte mode = 4;

	byte colMap[4];
	memset(colMap, 0, sizeof(colMap));

	uint16 eprcOffs = 0;

	uint32 cmd = drv.readUint32LE();
	if ((cmd & 0xFF) == 0xE9)
		eprcOffs = ((cmd >> 8) & 0xFFFF) + 3;

	if (!eprcOffs || drv.readUint32LE() != 0x87654321 || !drv.skip(1) || !drv.seek(drv.readByte(), SEEK_CUR) || !drv.seek(drv.readByte(), SEEK_CUR))
		error("Driver file '%s' unknown version", _driverFile);

	drv.skip(drv.readByte() == 0x90 ? 2 : 1);

	uint16 op1st = drv.readUint16LE();
	int op1len = drv.readUint16LE() - op1st;

	// sanity check
	assert(op1len > 0 && op1len < 0x100);

	drv.seek(op1st, SEEK_SET);
	byte *buf = new byte[op1len]();
	drv.read(buf, op1len);

	// Try figuring out the correct settings...
	for (int i = 0; i < op1len - 7; ++i) {
		uint32 cfg = READ_BE_UINT32(buf + i);
		cmd = READ_BE_UINT32(buf + 4 + i);
		if ((cmd >> 16) == 0xCD10 && (cfg & 0xff00ff) == 0xB80000) {
			mode = (cfg >> 8) & 0xff;
		} else if (cmd == 0xB40BCD10) {
			if (cfg >> 8 == 0x00B701B3) {
				palIndex = cfg & 1;
				palIntensity = (cfg >> 4) & 1;
			} else if (cfg >> 8 == 0x00B700B3) {
				colMap[0] = (cfg & 0x0f) + ((cfg & 0x10) >> 1);
			}
		}
	}

	delete[] buf;

	assert(palIndex <= 1);
	assert(palIntensity <= 1);

	for (int i = 1; i < 4; ++i)
		colMap[i] = modeColorMap[(!_disableMode5 && mode == 5) ? 2 : palIndex][i] + (palIntensity << 3);

	memset (_palette, 0, sizeof(_palette));
	for (int i = 0; i < 4; ++i) {
		for (int ii = 0; ii < 3; ++ii)
			_palette[i * 3 + ii] = cgaColors[colMap[i] * 3 + ii];
	}

	assignPalette(_palette);

	_cgaPatterns = new uint16[256]();
	// The pattern map is always located right before the driver entry point proc.
	drv.seek(eprcOffs - 512, SEEK_SET);
	for (int i = 0; i < 256; ++i)
		_cgaPatterns[i] = drv.readUint16LE();

	drv.close();
}

SCI0_CGADriver::~SCI0_CGADriver() {
	delete[] _cgaPatterns;
}

void SCI0_CGADriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod*, const byte*) {
	GFXDRV_ASSERT_READY;

	byte diff = srcX & 1;
	srcX &= ~1;
	destX &= ~1;
	w = (w + diff + 1) & ~1;

	src += (srcY * pitch + srcX);

	byte *dst = _compositeBuffer;
	int ty = destY;

	for (int i = 0; i < h; ++i) {
		_renderLine(dst, src, w, srcX & 3, ++ty, _cgaPatterns, _internalPalette);
		src += pitch;
	}

	g_system->copyRectToScreen(_compositeBuffer, w * _pixelSize, destX, destY, w, h);
}

void SCI0_CGADriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	// Instead of implementing the original cursor rendering code, we rely on the 8 bit cursor
	// that has already been generated by the engine. We simply convert the colors as needed...
	assert(keycolor == 1);
	const byte *s = reinterpret_cast<const byte*>(cursor);
	byte *d = _compositeBuffer;
	for (uint i = w * h; i; --i)
		*d++ = *s++ & 3;

	CursorMan.replaceCursor(_compositeBuffer, w, h, hotspotX, hotspotY, keycolor);
}


template <typename T> void cgaRenderLine(byte *&dst, const byte *src, int w, int tx, int ty, const uint16 *patterns, const byte *pal) {
	T *d = reinterpret_cast<T*>(dst);
	const T *p = reinterpret_cast<const T*>(pal);
	w >>= 1;

	for (int i = 0; i < w; ++i) {
		uint16 pattern = patterns[((src[0] & 0x0f) << 4) | (src[1] & 0x0f)];
		src += 2;
		byte sh = (ty & 3) << 1;
		byte lo = ((pattern & 0xff) >> sh) | ((pattern & 0xff) << (8 - sh));
		byte hi = (pattern >> (8 + sh)) | ((pattern >> 8) << (8 - sh));
		if (sizeof(T) == 1) {
			*d++ = (lo >> (6 - (tx << 1))) & 3;
			*d++ = (hi >> (4 - (tx << 1))) & 3;
		} else {
			*d++ = p[(lo >> (6 - (tx << 1))) & 3];
			*d++ = p[(hi >> (4 - (tx << 1))) & 3];
		}
		tx ^= 2;
	}

	dst = reinterpret_cast<byte*>(d);
}

void SCI0_CGADriver::setupRenderProc() {
	static const LineProc lineProcs[] = {
		&cgaRenderLine<byte>,
		&cgaRenderLine<uint16>,
		&cgaRenderLine<uint32>
	};

	assert((_pixelSize >> 1) < ARRAYSIZE(lineProcs));
	_renderLine = lineProcs[_pixelSize >> 1];
}

const char *SCI0_CGADriver::_driverFile = "CGA320C.DRV";

const byte *monochrInit(const char *drvFile, bool &earlyVersion) {
	Common::File drv;
	if (!drv.open(drvFile))
		return nullptr;

	uint16 eprcOffs = 0;

	uint32 cmd = drv.readUint32LE();
	if ((cmd & 0xFF) == 0xE9)
		eprcOffs = ((cmd >> 8) & 0xFFFF) + 3;

	if (!eprcOffs || drv.readUint32LE() != 0x87654321 || !drv.skip(1) || !drv.seek(drv.readByte(), SEEK_CUR) || !drv.seek(drv.readByte(), SEEK_CUR))
		error("Driver file '%s' unknown version", drv.getName());

	// This is a safe assumption, as the early version pattern map is 4 times the size of the later one.
	earlyVersion = (eprcOffs > 0x500);
	uint16 size = earlyVersion ? 512 : 128;

	byte *result = new byte[size];
	// For CGA, the pattern map is always located before the entry point dispatcher proc.
	drv.seek(eprcOffs - size, SEEK_SET);
	drv.read(result, size);

	// For Hercules there are some extra vars in between, all with initial values
	// of zero. The last entry of the pattern map is definitely never zero...
	int xtraOffs = 0;
	while (result[size - 1 - xtraOffs] == 0)
		++xtraOffs;
	if (xtraOffs != 0) {
		drv.seek(eprcOffs - size - xtraOffs, SEEK_SET);
		drv.read(result, size);
	}

	drv.close();

	if (earlyVersion) {
		uint16 *r = reinterpret_cast<uint16*>(result);
		for (int i = 0; i < 256; ++i)
			r[i] = FROM_LE_16(r[i]);
	}

	return result;
}

SCI0_CGABWDriver::SCI0_CGABWDriver(uint32 monochromeColor, bool rgbRendering) : SCI0_DOSPreVGADriver(2, 640, 400, rgbRendering), _monochromePatterns(nullptr), _earlyVersion(false), _renderLine(nullptr) {
	_monochromePalette[0] = _monochromePalette[1] = _monochromePalette[2] = 0;
	_monochromePalette[3] = (monochromeColor >> 16) & 0xff;
	_monochromePalette[4] = (monochromeColor >> 8) & 0xff;
	_monochromePalette[5] = monochromeColor & 0xff;
	assignPalette(_monochromePalette);

	if (!(_monochromePatterns = monochrInit(_driverFiles[0], _earlyVersion)) && !(_monochromePatterns = monochrInit(_driverFiles[1], _earlyVersion)))
		error("Failed to open '%s' or '%s'", _driverFiles[0], _driverFiles[1]);
}

SCI0_CGABWDriver::~SCI0_CGABWDriver() {
	delete[] _monochromePatterns;
}

void SCI0_CGABWDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod*, const byte*) {
	GFXDRV_ASSERT_READY;

	byte *dst = _compositeBuffer;
	int ty = destY & 7;

	if (_earlyVersion) {
		++ty;
		byte diff = srcX & 1;
		srcX &= ~1;
		destX &= ~1;
		w = (w + diff + 1) & ~1;
	}

	src += (srcY * pitch + srcX);

	for (int i = 0; i < h; ++i) {
		_renderLine(dst, src, w, srcX & 3, ty, _monochromePatterns, _internalPalette);
		ty = (ty + 1) & 7;
		src += pitch;
	}

	g_system->copyRectToScreen(_compositeBuffer, (w << 1) * _pixelSize, destX << 1, destY << 1, w << 1, h << 1);
}

void SCI0_CGABWDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	// Instead of implementing the original cursor rendering code, we rely on the 8 bit cursor that
	// has already been generated by the engine. We simply convert the colors as needed and scale the cursor...
	assert(keycolor == 1);
	keycolor = 0x0f;
	w <<= 1;
	const byte *s = reinterpret_cast<const byte*>(cursor);
	byte *d0 = _compositeBuffer;
	byte *d1 = _compositeBuffer + w;

	for (uint i = 0; i < h; ++i) {
		for (uint ii = 0; ii < w; ++ii) {
			*d0++ = *d1++ = *s ? (*s ^ 0x0e) : 0;
			if (ii & 1)
				++s;
		}
		d0 += w;
		d1 += w;
	}

	CursorMan.replaceCursor(_compositeBuffer, w, h << 1, hotspotX << 1, hotspotY << 1, keycolor);
}

Common::Point SCI0_CGABWDriver::getMousePos() const {
	Common::Point res = GfxDriver::getMousePos();
	res.x >>= 1;
	res.y >>= 1;
	return res;
}

void SCI0_CGABWDriver::setMousePos(const Common::Point &pos) const {
	g_system->warpMouse(pos.x << 1, pos.y << 1);
}

void SCI0_CGABWDriver::setShakePos(int shakeXOffset, int shakeYOffset) const {
	g_system->setShakePos(shakeXOffset << 1, shakeYOffset << 1);
}

void SCI0_CGABWDriver::clearRect(const Common::Rect &r) const {
	Common::Rect r2(r.left << 1, r.top << 1, r.right << 1, r.bottom << 1);
	GfxDriver::clearRect(r2);
}

Common::Point SCI0_CGABWDriver::getRealCoords(Common::Point &pos) const {
	return Common::Point(pos.x << 1, pos.y << 1);
}

template <typename T> void cgabwRenderLine_v1(byte *&dst, const byte *src, int w, int tx, int ty, const byte *patterns, const byte *pal) {
	const T *p = reinterpret_cast<const T*>(pal);
	const uint16 *patterns16 = reinterpret_cast<const uint16*>(patterns);
	T *d1 = reinterpret_cast<T*>(dst);
	T *d2 = d1 + (w << 1);
	w >>= 1;

	for (int i = 0; i < w; ++i) {
		uint16 pt = patterns16[((src[0] & 0x0f) << 4) | (src[1] & 0x0f)];
		src += 2;
		byte sh = (ty & 3) << 1;
		byte lo = ((pt & 0xff) >> sh) | ((pt & 0xff) << (8 - sh));
		byte hi = (pt >> (8 + sh)) | ((pt >> 8) << (8 - sh));
		if (sizeof(T) == 1) {
			*d1++ = *d2++ = ((lo >> (6 - (tx << 1))) >> 1) & 1;
			*d1++ = *d2++ = (lo >> (6 - (tx << 1))) & 1;
			*d1++ = *d2++ = ((hi >> (4 - (tx << 1))) >> 1) & 1;
			*d1++ = *d2++ = (hi >> (4 - (tx << 1))) & 1;
		} else {
			*d1++ = *d2++ = p[((lo >> (6 - (tx << 1))) >> 1) & 1];
			*d1++ = *d2++ = p[(lo >> (6 - (tx << 1))) & 1];
			*d1++ = *d2++ = p[((hi >> (4 - (tx << 1))) >> 1) & 1];
			*d1++ = *d2++ = p[(hi >> (4 - (tx << 1))) & 1];
		}
		tx ^= 2;
	}

	dst = reinterpret_cast<byte*>(d2);
}

template <typename T> void cgabwRenderLine_v2(byte *&dst, const byte *src, int w, int tx, int ty, const byte *patterns, const byte *pal) {
	const T *p = reinterpret_cast<const T*>(pal);
	T *d1 = reinterpret_cast<T*>(dst);
	T *d2 = d1 + (w << 1);

	for (int i = 0; i < w; ++i) {
		byte pt = patterns[((*src++ & 0x0f) << 3) + ty] >> (6 - (tx << 1));
		if (sizeof(T) == 1) {
			*d1++ = *d2++ = (pt >> 1) & 1;
			*d1++ = *d2++ = pt & 1;
		} else {
			*d1++ = *d2++ = p[(pt >> 1) & 1];
			*d1++ = *d2++ = p[pt & 1];
		}
		tx = (tx + 1) & 3;
	}

	dst = reinterpret_cast<byte*>(d2);
}

void SCI0_CGABWDriver::setupRenderProc() {
	static const LineProc lineProcs[] = {
		&cgabwRenderLine_v1<byte>,
		&cgabwRenderLine_v1<uint16>,
		&cgabwRenderLine_v1<uint32>,
		&cgabwRenderLine_v2<byte>,
		&cgabwRenderLine_v2<uint16>,
		&cgabwRenderLine_v2<uint32>
	};

	int t = _pixelSize >> 1;
	if (!_earlyVersion)
		t += 3;

	assert(t < ARRAYSIZE(lineProcs));
	_renderLine = lineProcs[t];
}

const char *SCI0_CGABWDriver::_driverFiles[2] = { "CGA320BW.DRV", "CGA320M.DRV" };

SCI0_HerculesDriver::SCI0_HerculesDriver(uint32 monochromeColor, bool rgbRendering, bool cropImage) : SCI0_DOSPreVGADriver(2, cropImage ? 640 : 720, cropImage ? 300 : 350, rgbRendering),
	_centerX(cropImage ? 0 : 40), _centerY(cropImage ? 0 : 25), _monochromePatterns(nullptr), _renderLine(nullptr) {
	_monochromePalette[0] = _monochromePalette[1] = _monochromePalette[2] = 0;
	_monochromePalette[3] = (monochromeColor >> 16) & 0xff;
	_monochromePalette[4] = (monochromeColor >> 8) & 0xff;
	_monochromePalette[5] = monochromeColor & 0xff;
	assignPalette(_monochromePalette);
	bool unused = false;

	if (!(_monochromePatterns = monochrInit(_driverFile, unused)))
		error("Failed to open '%s'", _driverFile);
}

SCI0_HerculesDriver::~SCI0_HerculesDriver() {
	delete[] _monochromePatterns;
}

void SCI0_HerculesDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod*, const byte*) {
	GFXDRV_ASSERT_READY;

	byte *dst = _compositeBuffer;
	byte sw = destY & 1;
	src += (srcY * pitch + srcX);
	destY = (destY & ~1) * 3 / 2 + (destY & 1);
	int ty = destY & 7;
	int rh = 0;

	for (int i = 0; i < h; ++i) {
		const byte *src2 = src;
		_renderLine(dst, src2, w, srcX & 3, ty, _monochromePatterns, _internalPalette);
		ty = (ty + 1) & 7;
		++rh;

		if (sw & 1)
			sw ^= 2;

		if (sw != 3) {
			src += pitch;
			sw ^= 1;
		} else {
			--i;
		}
	}

	g_system->copyRectToScreen(_compositeBuffer, (w << 1) * _pixelSize, (destX << 1) + _centerX, destY + _centerY, w << 1, rh);
}

void SCI0_HerculesDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	// Instead of implementing the original cursor rendering code, we rely on the 8 bit cursor that
	// has already been generated by the engine. We simply convert the colors as needed and scale the cursor...
	assert(keycolor == 1);
	keycolor = 0x0f;
	int alt = 0;
	const byte *s = reinterpret_cast<const byte *>(cursor);
	byte *d = _compositeBuffer;

	for (uint i = 0; i < h; ++i) {
		for (uint ii = 0; ii < (w << 1); ++ii) {
			*d++ = *s ? (*s ^ 0x0e) : 0;
			if (ii & 1)
				++s;
		}
		if (i & 1) {
			alt ^= 1;
			if (alt) {
				s -= w;
				--i;
			}
		}
	}

	CursorMan.replaceCursor(_compositeBuffer, w << 1, (h & ~1) * 3 / 2 + (h & 1), hotspotX << 1, (hotspotY & ~1) * 3 / 2 + (hotspotY & 1), keycolor);
}

Common::Point SCI0_HerculesDriver::getMousePos() const {
	Common::Point res = GfxDriver::getMousePos();
	res.x = CLIP<int>(res.x - _centerX, 0, 639) >> 1;
	res.y = (CLIP<int>(res.y - _centerY, 0, 299) * 2 + 1) / 3;
	return res;
}

void SCI0_HerculesDriver::setMousePos(const Common::Point &pos) const {
	g_system->warpMouse((pos.x << 1) + _centerX, (pos.y & ~1) * 3 / 2 + (pos.y & 1) + _centerY);
}

void SCI0_HerculesDriver::setShakePos(int shakeXOffset, int shakeYOffset) const {
	g_system->setShakePos(shakeXOffset << 1, (shakeYOffset & ~1) * 3 / 2 + (shakeYOffset & 1));
}

void SCI0_HerculesDriver::clearRect(const Common::Rect &r) const {
	Common::Rect r2((r.left << 1) + _centerX, (r.top & ~1) * 3 / 2 + (r.top & 1) + _centerY, (r.right << 1) + 40, (r.bottom & ~1) * 3 / 2 + (r.bottom & 1) + _centerY);
	GfxDriver::clearRect(r2);
}

Common::Point SCI0_HerculesDriver::getRealCoords(Common::Point &pos) const {
	return Common::Point((pos.x << 1) + _centerX, (pos.y & ~1) * 3 / 2 + (pos.y & 1) + _centerY);
}

template <typename T> void herculesRenderLine(byte *&dst, const byte *src, int w, int tx, int ty, const byte *patterns, const byte *pal) {
	T *d = reinterpret_cast<T*>(dst);
	const T *p = reinterpret_cast<const T*>(pal);

	for (int i = 0; i < w; ++i) {
		byte pt = patterns[((*src++ & 0x0f) << 3) + ty] >> (6 - (tx << 1));
		if (sizeof(T) == 1) {
			*d++ = (pt >> 1) & 1;
			*d++ = pt & 1;
		} else {
			*d++ = p[(pt >> 1) & 1];
			*d++ = p[pt & 1];
		}
		tx = (tx + 1) & 3;
	}

	dst = reinterpret_cast<byte*>(d);
}

void SCI0_HerculesDriver::setupRenderProc() {
	static const LineProc lineProcs[] = {
		&herculesRenderLine<byte>,
		&herculesRenderLine<uint16>,
		&herculesRenderLine<uint32>
	};

	assert((_pixelSize >> 1) < ARRAYSIZE(lineProcs));
	_renderLine = lineProcs[_pixelSize >> 1];
}

const char *SCI0_HerculesDriver::_driverFile = "HERCMONO.DRV";


SCI1_VGAGreyScaleDriver::SCI1_VGAGreyScaleDriver(bool rgbRendering) : GfxDefaultDriver(320, 200, false, rgbRendering), _greyScalePalette(nullptr) {
	_greyScalePalette = new byte[_numColors * 3]();
}

SCI1_VGAGreyScaleDriver::~SCI1_VGAGreyScaleDriver() {
	delete[] _greyScalePalette;
}

void SCI1_VGAGreyScaleDriver::setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY; 
	byte *d = _greyScalePalette;
	for (uint i = 0; i < num; ++i) {
		// In the driver files I inspected there were never any other color distributions than this.
		// So I guess it is safe to hardcode that instead of loading it from the driver file.
		d[0] = d[1] = d[2] = (colors[0] * 77 + colors[1] * 150 + colors[2] * 28) >> 8;
		colors += 3;
		d += 3;
	}

	GfxDefaultDriver::setPalette(_greyScalePalette, start, num, update, palMods, palModMapping);
}

const char *SCI1_VGAGreyScaleDriver::_driverFile = "VGA320BW.DRV";

SCI1_EGADriver::SCI1_EGADriver(bool rgbRendering) : GfxDriver(320, 200, 256), _requestRGBMode(rgbRendering), _egaColorPatterns(nullptr), _egaMatchTable(nullptr),
	_currentBitmap(nullptr), _compositeBuffer(nullptr), _currentPalette(nullptr), _internalPalette(nullptr), _colAdjust(0), _renderLine(nullptr), _vScaleMult(2), _vScaleDiv(1) {
	static const byte egaColors[48] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0x00, 0xAA, 0xAA,
		0xAA, 0x00, 0x00, 0xAA, 0x00, 0xAA, 0xAA, 0x55, 0x00, 0xAA, 0xAA, 0xAA,
		0x55, 0x55, 0x55, 0x55, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x55, 0xFF, 0xFF,
		0xFF, 0x55, 0x55, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF
	};
	_convPalette = egaColors;
}

SCI1_EGADriver::~SCI1_EGADriver() {
	delete[] _egaMatchTable;
	delete[] _egaColorPatterns;
	delete[] _compositeBuffer;
	delete[] _currentBitmap;
	delete[] _currentPalette;
	delete[] _internalPalette;
}

template <typename T> void ega640RenderLine(byte *&dst, const byte *src, int w, const byte *patterns, const byte *pal, bool) {
	const T *p = reinterpret_cast<const T*>(pal);
	T *d1 = reinterpret_cast<T*>(dst);
	T *d2 = d1 + (w << 1);

	for (int i = 0; i < w; ++i) {
		byte pt = patterns[*src++];
		if (sizeof(T) == 1) {
			*d1++ = *d2++ = pt >> 4;
			*d1++ = *d2++ = pt & 0x0f;
		} else {
			*d1++ = *d2++ = p[pt >> 4];
			*d1++ = *d2++ = p[pt & 0x0f];
		}
	}
	dst = reinterpret_cast<byte*>(d2);
}

void SCI1_EGADriver::initScreen(const Graphics::PixelFormat*) {
	if (!_ready)
		loadData();

	Graphics::PixelFormat format(Graphics::PixelFormat::createFormatCLUT8());
	initGraphics(_screenW << 1, _screenH * _vScaleMult / _vScaleDiv, _requestRGBMode ? nullptr : &format);
	format = g_system->getScreenFormat();
	_pixelSize = format.bytesPerPixel;

	if (_requestRGBMode && _pixelSize == 1)
		warning("SCI1_EGADriver::initScreen(): RGB rendering not available in this ScummVM build");

	delete[] _egaColorPatterns;
	delete[] _compositeBuffer;
	delete[] _currentBitmap;
	delete[] _currentPalette;
	delete[] _internalPalette;
	_internalPalette = nullptr;
	_egaColorPatterns = _compositeBuffer = _currentBitmap = _currentPalette = nullptr;

	if (_pixelSize == 1) {
		g_system->getPaletteManager()->setPalette(_convPalette, 0, 16);
	} else {
		byte *rgbpal = new byte[_numColors * _pixelSize]();
		assert(rgbpal);

		if (_pixelSize == 2)
			updateRGBPalette<uint16>(rgbpal, _convPalette, 0, 16, format);
		else if (_pixelSize == 4)
			updateRGBPalette<uint32>(rgbpal, _convPalette, 0, 16, format);
		else
			error("SCI1_EGADriver::initScreen(): Unsupported screen format");
		_internalPalette = rgbpal;
		CursorMan.replaceCursorPalette(_convPalette, 0, 16);
	}

	_compositeBuffer = new byte[(_screenW << 1) * (_screenH * _vScaleMult / _vScaleDiv) * _pixelSize]();
	assert(_compositeBuffer);
	_currentBitmap = new byte[_screenW * _screenH]();
	assert(_currentBitmap);
	_currentPalette = new byte[256 * 3]();
	assert(_currentPalette);
	_egaColorPatterns = new byte[256]();
	assert(_egaColorPatterns);

	static const LineProc lineProcs[] = {
		&ega640RenderLine<byte>,
		&ega640RenderLine<uint16>,
		&ega640RenderLine<uint32>
	};

	assert((_pixelSize >> 1) < ARRAYSIZE(lineProcs));
	_renderLine = lineProcs[_pixelSize >> 1];

	_ready = true;
}

void SCI1_EGADriver::setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod*, const byte*) {
	GFXDRV_ASSERT_READY;
	memcpy(_currentPalette + start * 3, colors, num * 3);
	byte *d = &_egaColorPatterns[start];
	for (uint i = 0; i < num; ++i) {
		*d++ = _egaMatchTable[((MIN<byte>((colors[0] >> 2) + _colAdjust, 63) & 0x38) << 3) | (MIN<byte>((colors[1] >> 2) + _colAdjust, 63) & 0x38) | (MIN<byte>((colors[2] >> 2) + _colAdjust, 63) >> 3)];
		colors += 3;
	}
	if (update)
		copyRectToScreen(_currentBitmap, 0, 0, _screenW, 0, 0, _screenW, _screenH, nullptr, nullptr);
}

void SCI1_EGADriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod*, const byte*) {
	GFXDRV_ASSERT_READY;
	assert (h >= 0 && w >= 0);

	src += (srcY * pitch + srcX);

	if (src != _currentBitmap)
		updateBitmapBuffer(_currentBitmap, _screenW, src, pitch, destX, destY, w, h);

	uint16 realWidth, realHeight;
	renderBitmap(_compositeBuffer, src, pitch, destY, w, h, _egaColorPatterns, _internalPalette, realWidth, realHeight);

	Common::Point pos(destX, destY);
	pos = getRealCoords(pos);

	g_system->copyRectToScreen(_compositeBuffer, realWidth * _pixelSize, pos.x, pos.y, realWidth, realHeight);
}

void SCI1_EGADriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	const byte *s = reinterpret_cast<const byte*>(cursor);
	int dstPitch = (w << 1);
	byte *d1 = _compositeBuffer;
	byte *d2 = _compositeBuffer + dstPitch;
	uint32 newKeyColor = 0xFF;

	for (uint i = 0; i < h; ++i) {
		for (uint ii = 0; ii < w; ++ii) {
			byte col = *s++;
			if (col == keycolor) {
				*d1++ = *d2++ = newKeyColor;
				*d1++ = *d2++ = newKeyColor;
			} else {
				byte pt = _egaColorPatterns[col];
				*d1++ = *d2++ = pt >> 4;
				*d1++ = *d2++ = pt & 0x0f;
			}
		}
		d1 += dstPitch;
		d2 += dstPitch;
	}

	CursorMan.replaceCursor(_compositeBuffer, w << 1, h << 1, hotspotX << 1, hotspotY << 1, newKeyColor);
}

void SCI1_EGADriver::copyCurrentBitmap(byte *dest, uint32 size) const {
	GFXDRV_ASSERT_READY;
	assert(dest);
	assert(size <= (uint32)(_screenW * _screenH));
	memcpy(dest, _currentBitmap, size);
}

void SCI1_EGADriver::copyCurrentPalette(byte *dest, int start, int num) const {
	GFXDRV_ASSERT_READY;
	assert(dest);
	assert(start + num <= 256);
	memcpy(dest + start * 3, _currentPalette + start * 3, num * 3);
}

void SCI1_EGADriver::drawTextFontGlyph(const byte*, int, int, int, int, int, int, const PaletteMod*, const byte*) {
	// This is only needed for scaling drivers with unscaled hires fonts.
	error("SCI1_EGADriver::drawTextFontGlyph(): Not implemented");
}

Common::Point SCI1_EGADriver::getMousePos() const {
	Common::Point res = GfxDriver::getMousePos();
	res.x >>= 1;
	res.y = res.y * _vScaleDiv / _vScaleMult;
	return res;
}

void SCI1_EGADriver::setMousePos(const Common::Point &pos) const {
	g_system->warpMouse(pos.x << 1, pos.y * _vScaleMult / _vScaleDiv);
}

void SCI1_EGADriver::setShakePos(int shakeXOffset, int shakeYOffset) const {
	g_system->setShakePos(shakeXOffset << 1, shakeYOffset * _vScaleMult / _vScaleDiv);
}

void SCI1_EGADriver::clearRect(const Common::Rect &r) const {
	Common::Rect r2(r.left << 1, r.top * _vScaleMult / _vScaleDiv, r.right << 1, r.bottom * _vScaleMult / _vScaleDiv);
	GfxDriver::clearRect(r2);
}

Common::Point SCI1_EGADriver::getRealCoords(Common::Point &pos) const {
	return Common::Point(pos.x << 1, pos.y * _vScaleMult / _vScaleDiv);
}

void SCI1_EGADriver::loadData() {
	Common::File drv;
	if (!drv.open(_driverFile))
		error("SCI1_EGADriver: Failed to open '%s'", _driverFile);

	uint16 eprcOffs = 0;

	uint32 cmd = drv.readUint32LE();
	if ((cmd & 0xFF) == 0xE9)
		eprcOffs = ((cmd >> 8) & 0xFFFF) + 3;

	if (!eprcOffs || drv.readUint32LE() != 0x87654321 || !drv.skip(1) || !drv.seek(drv.readByte(), SEEK_CUR) || !drv.seek(drv.readByte(), SEEK_CUR) || drv.readUint32LE() != 0xFEDCBA98 || !drv.skip(4))
		error("SCI1_EGADriver: Driver file '%s' unknown version", _driverFile);

	uint32 pos = (drv.pos() + 1) & ~1;

	drv.seek(pos);
	drv.seek(drv.readUint16LE());
	uint32 colResponse = drv.readUint32LE();
	_numColors = (colResponse >> 8) & 0xffff;
	if (_numColors < 16 || _numColors > 256 || (colResponse & 0xff0000ff) != 0xC30000B8)
		error("SCI1_EGADriver: Failed to retrieve color info from '%s'", _driverFile);
	
	drv.seek(pos + 20);
	drv.seek(drv.readUint16LE());
	byte *buff = new byte[128];
	drv.read(buff, 128);

	uint16 tableOffs = 0;
	for (int i = 0; i < 120 && !tableOffs; ++i) {
		uint32 c = READ_BE_UINT32(buff + i);
		if (c == 0x8BD82E8A) {
			if (buff[i + 4] == 0x87)
				tableOffs = READ_LE_UINT16(buff + i + 5);
		} else if (c == 0xD0E8D0E8) {
			for (int ii = 4; ii < 14; ++ii) {
				if (READ_BE_UINT16(buff + i + ii) == 0x83C0) {
					c = READ_BE_UINT32(buff + i + ii + 2);
					if ((c & 0xFFFFFF) == 0x83F83F)
						_colAdjust = c >> 24;
				}
			}
		}
	}
	delete[] buff;

	if (!tableOffs)
		error("SCI1_EGADriver: Failed to load color data from '%s'", _driverFile);

	drv.seek(tableOffs);
	byte *table = new byte[512]();
	drv.read(table, 512);
	_egaMatchTable = table;

	if (drv.readUint16LE() != 152 || drv.readUint16LE() != 160)
		error("SCI1_EGADriver: Driver file '%s' unknown version", _driverFile);

	drv.close();
}

void SCI1_EGADriver::renderBitmap(byte *dst, const byte *src, int pitch, int, int w, int h, const byte *patterns, const byte *palette, uint16 &realWidth, uint16 &realHeight) {
	for (int i = 0; i < h; ++i) {
		_renderLine(dst, src, w, patterns, palette, 0);
		src += pitch;
	}
	realWidth = w << 1;
	realHeight = h << 1;
}

const char *SCI1_EGADriver::_driverFile = "EGA640.DRV";

template <typename T> void scale2x(byte *dst, const byte *src, int pitch, int w, int h) {
	const T *s = reinterpret_cast<const T*>(src);
	int dstPitch = pitch << 1;
	T *d1 = reinterpret_cast<T*>(dst);
	T *d2 = d1 + dstPitch;
	pitch -= w;
	dstPitch += (pitch << 1);

	while (h--) {
		for (int i = 0; i < w; ++i) {
			d1[0] = d1[1] = d2[0] = d2[1] = *s++;
			d1 += 2;
			d2 += 2;
		}
		s += pitch;
		d1 += dstPitch;
		d2 += dstPitch;
	}
}

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
		&scale2x<byte>,
		&scale2x<uint16>,
		&scale2x<uint32>
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
		updateBitmapBuffer(_currentBitmap, _virtualW * _srcPixelSize, src, pitch, destX * _srcPixelSize, destY, w * _srcPixelSize, h);

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
		scale2x<byte>(_compositeBuffer, reinterpret_cast<const byte*>(cursor), w, w, h);
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

KQ6WinGfxDriver::KQ6WinGfxDriver(bool dosStyleCursors, bool smallWindow,bool rgbRendering) :
	UpscaledGfxDriver(smallWindow ? 320 : 640, smallWindow ? 240 : 440, 1, dosStyleCursors && !smallWindow, rgbRendering), _dosStyleCursors(dosStyleCursors), _smallWindow(smallWindow),
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

void KQ6WinGfxDriver::initScreen(const Graphics::PixelFormat *format) {
	UpscaledGfxDriver::initScreen(format);
	_renderLine = _smallWindow ? &smallWindowRenderLine : &largeWindowRenderLine;
	_renderLine2 = _smallWindow ? &renderLineDummy : &hiresRenderLine;
}

void KQ6WinGfxDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
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

byte findColorInPalette(uint32 rgbTriplet, const byte *palette, int numColors) {
	byte color[3];
	for (int i = 2; i >= 0; --i) {
		color[i] = rgbTriplet & 0xFF;
		rgbTriplet >>= 8;
	}
	int min = 65025;
	byte match = 0;
	for (int i = 0; i < numColors && min; ++i) {
		const byte *rgb = &palette[i * 3];
		int t = (color[0] - rgb[0]) * (color[0] - rgb[0]) + (color[1] - rgb[1]) * (color[1] - rgb[1]) + (color[2] - rgb[2]) * (color[2] - rgb[2]);
		if (t < min) {
			min = t;
			match = i;
		}
	}
	return match;
}

void renderWinMonochromeCursor(byte *dst, const void *src, const byte *palette, uint &w, uint &h, int &hotX, int &hotY, byte blackColor, byte whiteColor, uint32 &keycolor) {
	const byte *s = reinterpret_cast<const byte*>(src);
	uint16 min = 65025;
	uint16 max = 0;

	byte newKeyColor = 0;
	while (newKeyColor == blackColor || newKeyColor == whiteColor)
		++newKeyColor;

	for (uint i = 0; i < w * h; ++i) {
		byte col = *s++;
		if (col == keycolor)
			continue;
		const byte *rgb = &palette[col * 3];
		uint16 t = rgb[0] * 28 + rgb[1] * 150 + rgb[2] * 28;
		if (t > max)
			max = t;
		if (t < min)
			min = t;
	}

#if 0
	// The original interpreter will accidentally let the value overflow like this,
	// making most cursors completely white. I have fixed it.
	uint16 med = (uint16)(min + max) >> 1;
#else
	uint16 med = (min + max) >> 1;
#endif
	uint16 lim1 = max - (max - min) / 3;
	uint16 lim2 = min + max - lim1;
	s = reinterpret_cast<const byte*>(src);

	if (w < 17 && h < 17) {
		// Small cursors (like the insignia ring in KQ6) get scaled and dithered.
		byte *dst2 = dst + (w << 1);
		for (uint i = 0; i < h; ++i) {
			for (uint ii = 0; ii < w; ++ii) {
				byte col = *s++;
				if (col == keycolor) {
					*dst++ = *dst2++ = newKeyColor;
					*dst++ = *dst2++ = newKeyColor;
					continue;
				}
				const byte *rgb = &palette[col * 3];
				uint16 t = rgb[0] * 28 + rgb[1] * 150 + rgb[2] * 28;

				dst[0] = dst2[1] = t > lim2 ? whiteColor : blackColor;
				dst2[0] = dst[1] = t > lim1 ? whiteColor : blackColor;
				dst += 2;
				dst2 += 2;
			};
			dst	+= (w << 1);
			dst2 += (w << 1);
		}
		w <<= 1;
		h <<= 1;
		hotX <<= 1;
		hotY <<= 1;
	} else {
		for (uint i = 0; i < w * h; ++i) {
			byte col = *s++;
			if (col == keycolor) {
				*dst++ = newKeyColor;
				continue;
			}
			const byte *rgb = &palette[col * 3];
			uint16 t = rgb[0] * 28 + rgb[1] * 150 + rgb[2] * 28;
			*dst++ = t > med ? whiteColor : blackColor;
		}
	}
	keycolor = newKeyColor;
}

void KQ6WinGfxDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	if (_dosStyleCursors) {
		// The original windows interpreter always renders the cursor as b/w, regardless of which cursor views (the DOS
		// cursors or the new Windows ones) the user selects. This is also regardless of color mode (16 or 256 colors).
		// But I think it is on purpose that we have the cursors look like in the DOS version as a default.
		UpscaledGfxDriver::replaceCursor(cursor, w, h, hotspotX, hotspotY, keycolor);
		return;
	}
	adjustCursorBuffer(w << 1, h << 1);

	if (_pixelSize == 1)
		copyCurrentPalette(_currentPalette, 0, _numColors);

	byte col1 = findColorInPalette(0x00000000, _currentPalette, _numColors);
	byte col2 = findColorInPalette(0x00FFFFFF, _currentPalette, _numColors);
	renderWinMonochromeCursor(_compositeBuffer, cursor, _currentPalette, w, h, hotspotX, hotspotY, col1, col2, keycolor);
	CursorMan.replaceCursor(_compositeBuffer, w, h, hotspotX, hotspotY, keycolor);
}

Common::Point KQ6WinGfxDriver::getRealCoords(Common::Point &pos) const {
	return Common::Point(pos.x * _hScaleMult, pos.y * _vScaleMult2 + (pos.y + 4) / 5);
}

void KQ6WinGfxDriver::setFlags(uint32 flags) {
	flags ^= (_flags & flags);
	if (!flags)
		return;

	if (flags & kMovieMode)
		_renderLine2 = _smallWindow ? &smallWindowRenderLineMovie : &largeWindowRenderLineMovie;

	_flags |= flags;
}

void KQ6WinGfxDriver::clearFlags(uint32 flags) {
	flags &= _flags;
	if (!flags)
		return;

	if (flags & kMovieMode)
		_renderLine2 = _smallWindow ? &renderLineDummy : &hiresRenderLine;

	_flags &= ~flags;
}

void KQ6WinGfxDriver::renderBitmap(const byte *src, int pitch, int dx, int dy, int w, int h, int &realWidth, int &realHeight) {
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

KQ6WinGfx16ColorsDriver::KQ6WinGfx16ColorsDriver(bool altCursor, bool enhancedDithering, bool rgbRendering) : SCI1_EGADriver(rgbRendering), _enhancedDithering(enhancedDithering), _altCursor(altCursor), _renderLine2(nullptr) {
	static const byte win16Colors[48] = {
		0x00, 0x00, 0x00, 0xA8, 0x00, 0x57, 0x00, 0xA8, 0x57, 0xA8, 0xA8, 0x57,
		0x00, 0x00, 0xA8, 0xA8, 0x57, 0xA8, 0x57, 0xA8, 0xA8, 0x87, 0x88, 0x8F,
		0xC0, 0xC7, 0xC8, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00,
		0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

	_convPalette = win16Colors;
	_vScaleMult = 11;
	_vScaleDiv = 5;
}

KQ6WinGfx16ColorsDriver::~KQ6WinGfx16ColorsDriver() {
	_egaMatchTable = nullptr; // prevent invalid deletion in SCI1_EGADriver::~SCI1_EGADriver()
}

template <typename T, bool extScale> void win16ColRenderLine(byte *&dst, const byte *src, int w, const byte *patterns, const byte *pal, bool swap) {
	const T *p = reinterpret_cast<const T*>(pal);
	T *d1 = reinterpret_cast<T*>(dst);
	T *d2 = d1 + (w << 1);
	T *d3 = d2 + (w << 1);
	T *&d3r = swap ? d2 : d1;

	if (swap)
		SWAP(d1, d2);

	for (int i = 0; i < w; ++i) {
		byte pt = patterns[*src++];
		if (sizeof(T) == 1) {
			*d1++ = d2[1] = pt & 0x0F;
			*d1++ = *d2++ = pt >> 4;
		} else {
			*d1++ = d2[1] = p[pt & 0x0F];
			*d1++ = *d2++ = p[pt >> 4];
		}
		d2++;

		if (extScale) {
			*d3++ = *(d3r - 2);
			*d3++ = *(d3r - 1);
		}
	}

	dst = reinterpret_cast<byte*>(extScale ? d3 : (swap ? d1 : d2));
}

void KQ6WinGfx16ColorsDriver::initScreen(const Graphics::PixelFormat *format) {
	SCI1_EGADriver::initScreen(format);

	static const LineProc lineProcs[] = {
		&win16ColRenderLine<byte, false>,
		&win16ColRenderLine<byte, true>,
		&win16ColRenderLine<uint16, false>,
		&win16ColRenderLine<uint16, true>,
		&win16ColRenderLine<uint32, false>,
		&win16ColRenderLine<uint32, true>
	};

	assert((_pixelSize | 1) < ARRAYSIZE(lineProcs));
	_renderLine = lineProcs[_pixelSize & ~1];
	_renderLine2 = lineProcs[_pixelSize | 1];
}

void KQ6WinGfx16ColorsDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	// The original windows interpreter always renders the cursor as b/w, regardless of which cursor views (the DOS
	// cursors or the new Windows ones) the user selects. This is also regardless of color mode (16 or 256 colors).
	byte col1 = findColorInPalette(0x00000000, _convPalette, _numColors);
	byte col2 = findColorInPalette(0x00FFFFFF, _convPalette, _numColors);
	renderWinMonochromeCursor(_compositeBuffer, cursor, _currentPalette, w, h, hotspotX, hotspotY, col1, col2, keycolor);
	CursorMan.replaceCursor(_compositeBuffer, w, h, hotspotX, hotspotY, keycolor);
}

Common::Point KQ6WinGfx16ColorsDriver::getRealCoords(Common::Point &pos) const {
	return Common::Point(pos.x << 1, (pos.y << 1) + (pos.y + 4) / 5);
}

void KQ6WinGfx16ColorsDriver::loadData() {
	_egaMatchTable = _win16ColorsDitherPatterns;
	_numColors = 16;
}

void KQ6WinGfx16ColorsDriver::renderBitmap(byte *dst, const byte *src, int pitch, int y, int w, int h, const byte *patterns, const byte *palette, uint16 &realWidth, uint16 &realHeight) {
	const byte *dst0 = dst;
	byte mod = (y + 4) % 5;
	byte swap = _enhancedDithering ? ((y + 4) / 5) & 1 : 0;
	for (int i = 0; i < h; ++i) {
		if (++mod == 5) {
			_renderLine2(dst, src, w, patterns, palette, swap);
			if (_enhancedDithering)
				swap ^= 1;
			mod = 0;
		} else {
			_renderLine(dst, src, w, patterns, palette, swap);
		}
		src += pitch;
	}
	realWidth = w << 1;
	realHeight = (dst - dst0) / (realWidth * _pixelSize);
}

const byte KQ6WinGfx16ColorsDriver::_win16ColorsDitherPatterns[512] = {
	0x00, 0x00, 0x00, 0x04, 0x04, 0x44, 0x4c, 0xcc, 0x00, 0x00, 0x00, 0x04, 0x04, 0x44, 0x4c, 0xcc,
	0x00, 0x00, 0x00, 0x04, 0x04, 0x44, 0x4c, 0xcc, 0x02, 0x02, 0x02, 0x06, 0x06, 0x46, 0x6c, 0x6c,
	0x02, 0x02, 0x02, 0x06, 0x06, 0x2c, 0x6c, 0xce, 0x22, 0x22, 0x22, 0x26, 0x26, 0x66, 0x4e, 0xce,
	0x2a, 0x2a, 0x2a, 0x6a, 0x6a, 0x2e, 0x6e, 0x6e, 0xaa, 0xaa, 0xaa, 0xaa, 0xae, 0xae, 0xee, 0xee,
	0x00, 0x00, 0x00, 0x04, 0x04, 0x44, 0x4c, 0xcc, 0x00, 0x00, 0x00, 0x04, 0x04, 0x44, 0x4c, 0xcc,
	0x00, 0x00, 0x00, 0x04, 0x04, 0x44, 0x4c, 0xcc, 0x02, 0x02, 0x02, 0x06, 0x06, 0x46, 0x6c, 0x6c,
	0x02, 0x02, 0x02, 0x06, 0x06, 0x2c, 0x6c, 0xce, 0x22, 0x22, 0x22, 0x26, 0x26, 0x66, 0x4e, 0xce,
	0x2a, 0x2a, 0x2a, 0x6a, 0x6a, 0x2e, 0x6e, 0x6e, 0xaa, 0xaa, 0xaa, 0xaa, 0xae, 0xae, 0xae, 0xee,
	0x00, 0x00, 0x00, 0x04, 0x04, 0x44, 0x4c, 0xcc, 0x00, 0x00, 0x00, 0x04, 0x04, 0x44, 0x4c, 0xcc,
	0x00, 0x00, 0x00, 0x05, 0x04, 0x44, 0x4c, 0xcc, 0x02, 0x02, 0x02, 0x06, 0x06, 0x46, 0x6c, 0x6c,
	0x02, 0x02, 0x02, 0x06, 0x06, 0x2c, 0x6c, 0xce, 0x22, 0x22, 0x22, 0x26, 0x26, 0x66, 0x4e, 0xce,
	0x2a, 0x2a, 0x2a, 0x6a, 0x6a, 0x2e, 0x6e, 0x6e, 0xaa, 0xaa, 0xaa, 0xaa, 0xae, 0xae, 0xee, 0xee,
	0x01, 0x01, 0x01, 0x05, 0x05, 0x45, 0x5c, 0x5c, 0x01, 0x01, 0x01, 0x05, 0x05, 0x45, 0x5c, 0x5c,
	0x01, 0x01, 0x01, 0x05, 0x05, 0x1c, 0x5c, 0x5c, 0x03, 0x03, 0x03, 0x07, 0x07, 0x47, 0x7c, 0x7c,
	0x03, 0x03, 0x03, 0x07, 0x08, 0x3c, 0x7c, 0x8c, 0x23, 0x23, 0x23, 0x27, 0x27, 0x67, 0x5e, 0x5e,
	0x3a, 0x3a, 0x3a, 0x7a, 0x7a, 0x3e, 0x7e, 0x7e, 0xaa, 0xaa, 0xaa, 0x8a, 0x8a, 0xae, 0x8e, 0xee,
	0x01, 0x01, 0x01, 0x05, 0x05, 0x45, 0x5c, 0xcd, 0x01, 0x01, 0x01, 0x49, 0x05, 0x1c, 0x5c, 0xcd,
	0x11, 0x01, 0x01, 0x49, 0x05, 0x1c, 0x4d, 0xcd, 0x03, 0x03, 0x03, 0x07, 0x07, 0x47, 0x7c, 0x7c,
	0x13, 0x03, 0x03, 0x08, 0x08, 0x48, 0x48, 0x8c, 0x9a, 0x23, 0x1a, 0x4b, 0x28, 0x68, 0x4f, 0xcf,
	0x3a, 0x3a, 0x3a, 0x6b, 0x7a, 0x3e, 0x7e, 0x8e, 0xab, 0xab, 0xab, 0xab, 0xaf, 0xaf, 0xaf, 0xef,
	0x11, 0x11, 0x11, 0x15, 0x15, 0x55, 0x4d, 0xcd, 0x11, 0x11, 0x11, 0x15, 0x15, 0x55, 0x4d, 0xcd,
	0x11, 0x11, 0x11, 0x49, 0x49, 0x9c, 0x4d, 0xcd, 0x29, 0x29, 0x13, 0x17, 0x17, 0x2d, 0x6d, 0x6d,
	0x13, 0x13, 0x13, 0x18, 0x18, 0x58, 0x58, 0x8c, 0x33, 0x33, 0x33, 0x37, 0x37, 0x77, 0x4f, 0xcf,
	0x2b, 0x2b, 0x2b, 0x6b, 0x6b, 0x2f, 0x6f, 0x6f, 0xab, 0xab, 0xab, 0xab, 0xaf, 0xaf, 0xef, 0xef,
	0x19, 0x19, 0x19, 0x59, 0x59, 0x1d, 0x5d, 0x5d, 0x19, 0x19, 0x19, 0x59, 0x59, 0x1d, 0x5d, 0x5d,
	0x19, 0x19, 0x19, 0x59, 0x59, 0x1d, 0x5d, 0xdd, 0x39, 0x39, 0x39, 0x79, 0x79, 0x3d, 0x7d, 0x7d,
	0x39, 0x39, 0x39, 0x89, 0x89, 0x3d, 0x8d, 0x8d, 0x1b, 0x1b, 0x1b, 0x5b, 0x5b, 0x1f, 0x5f, 0x5f,
	0x3b, 0x3b, 0x3b, 0x7b, 0x7b, 0x3f, 0x88, 0x88, 0xbb, 0xab, 0xab, 0x8b, 0x8b, 0xaf, 0x8f, 0x8f,
	0x99, 0x99, 0x99, 0x99, 0x9d, 0x9d, 0xdd, 0xdd, 0x99, 0x99, 0x99, 0x99, 0x9d, 0x9d, 0xdd, 0xdd,
	0x99, 0x99, 0x99, 0x99, 0x9d, 0x9d, 0x7d, 0xdd, 0x39, 0x39, 0x39, 0x79, 0x89, 0x3d, 0x7d, 0x5d,
	0x9b, 0x9b, 0x89, 0x89, 0x89, 0x89, 0x8d, 0x8d, 0x9b, 0x9b, 0x9b, 0x9b, 0x9f, 0x9f, 0xdf, 0xdf,
	0x3b, 0x3b, 0x3b, 0x8b, 0x8b, 0x3f, 0x8f, 0x8f, 0xbb, 0xbb, 0xbb, 0xbb, 0xbf, 0xbf, 0xff, 0xff
};

PC98Gfx16ColorsDriver::PC98Gfx16ColorsDriver(int textAlignX, bool cursorScaleWidth, bool cursorScaleHeight, SjisFontStyle sjisFontStyle, bool rgbRendering, bool needsUnditheringPalette) :
	UpscaledGfxDriver(textAlignX, cursorScaleWidth && cursorScaleHeight, rgbRendering), _textModePalette(nullptr), _fontStyle(sjisFontStyle),
		_cursorScaleHeightOnly(!cursorScaleWidth && cursorScaleHeight), _convPalette(nullptr) {
	// Palette taken from driver file (identical for all versions of the
	// driver I have seen so far, also same for SCI0 and SCI1)
	static const byte pc98colorsV16[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x07, 0x07,
		0x07, 0x00, 0x00, 0x07, 0x00, 0x07, 0x05, 0x07, 0x00, 0x09, 0x09, 0x09,
		0x06, 0x06, 0x06, 0x00, 0x00, 0x0f, 0x07, 0x0f, 0x06, 0x00, 0x0f, 0x0f,
		0x0f, 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x0f, 0x0f, 0x00, 0x0f, 0x0f, 0x0f
	};

	byte *col = new byte[768]();
	const byte *s = pc98colorsV16;

	for (uint i = 0; i < sizeof(pc98colorsV16) / 3; ++i) {
		int a = ((i & 6) == 4 || (i & 6) == 2 ? i ^ 6 : i) * 3;
		col[a + 0] = (s[1] * 0x11);
		col[a + 1] = (s[0] * 0x11);
		col[a + 2] = (s[2] * 0x11);
		s += 3;
	}

	if (_fontStyle == kFontStyleTextMode) {
		byte *d = &col[48];
		for (uint8 i = 0; i < 8; ++i) {
			*d++ = (i & 4) ? 0xff : 0;
			*d++ = (i & 2) ? 0xff : 0;
			*d++ = (i & 1) ? 0xff : 0;
		}
	}

	if (needsUnditheringPalette) {
		// We store the text mode color separately, since we need the slots for the undithering.
		if (_fontStyle == kFontStyleTextMode) {
			byte *tpal = new byte[24]();
			memcpy(tpal, &col[48], 24);
			_textModePalette = tpal;
		}
		// For the undithered mode, we generate the missing colors using the same formula as for EGA.
		byte *d = &col[48];
		for (int i = 16; i < 256; i++) {
			const byte *s1 = &col[(i & 0x0f) * 3];
			const byte *s2 = &col[(i >> 4) * 3];
			for (int ii = 0; ii < 3; ++ii)
				*d++ = (byte)(0.5 + (pow(0.5 * ((pow(*s1++ / 255.0, 2.2 / 1.0) * 255.0) + (pow(*s2++ / 255.0, 2.2 / 1.0) * 255.0)) / 255.0, 1.0 / 2.2) * 255.0));
		}
	}

	_convPalette = col;
}

PC98Gfx16ColorsDriver::~PC98Gfx16ColorsDriver() {
	delete[] _convPalette;
	delete[] _textModePalette;
}

void renderPC98GlyphFat(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h, int transpCol) {
	dstPitch -= w;
	srcPitch -= w;

	while (h--) {
		for (int i = 0; i < w - 1; ++i) {
			uint8 a = *src++;
			uint8 b = *src;
			if (a != transpCol)
				*dst = a;
			else if (b != transpCol)
				*dst = b;
			++dst;
		}
		byte l = *src++;
		if (l != transpCol)
			*dst = l;
		++dst;
		src += srcPitch;
		dst += dstPitch;
	}
}

void renderPC98GlyphSpecial(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h, int transpCol) {
	assert(h == 16); // This is really not suitable for anything but the special SCI1 PC98 glyph drawing
	dstPitch -= w;
	srcPitch -= w;

	while (h--) {
		if (h > 10 || h < 5) {
			for (int i = 0; i < w - 1; ++i) {
				uint8 a = *src++;
				uint8 b = *src;
				if (a != transpCol)
					*dst = a;
				else if (b != transpCol)
					*dst = b;
				++dst;
			}
			byte l = *src++;
			if (l != transpCol)
				*dst = l;
			++dst;
		} else {
			for (int i = 0; i < w; ++i) {
				byte in = *src++;
				if (in != transpCol)
					*dst = in;
				++dst;
			}
		}
		src += srcPitch;
		dst += dstPitch;
	}
}

void PC98Gfx16ColorsDriver::initScreen(const Graphics::PixelFormat *format) {
	UpscaledGfxDriver::initScreen(format);

	assert(_convPalette);
	GfxDefaultDriver::setPalette(_convPalette, 0, 256, true, nullptr, nullptr);

	if (_fontStyle == kFontStyleTextMode)
		_renderGlyph = &renderPC98GlyphFat;

	if (_fontStyle != kFontStyleSpecialSCI1)
		return;

	_renderGlyph = &renderPC98GlyphSpecial;
}

void PC98Gfx16ColorsDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	if (!_cursorScaleHeightOnly) {
		UpscaledGfxDriver::replaceCursor(cursor, w, h, hotspotX, hotspotY, keycolor);
		return;
	}

	// Special case for PQ2 which scales the cursor height (but not the width)
	adjustCursorBuffer(w, h << 1);
	const byte *s = reinterpret_cast<const byte*>(cursor);
	byte *d = _compositeBuffer;

	for (uint i = 0; i < h; ++i) {
		memcpy(d, s, w);
		d += w;
		memcpy(d, s, w);
		d += w;
		s += w;
	}

	CursorMan.replaceCursor(_compositeBuffer, w, h << 1, hotspotX, hotspotY << 1, keycolor);
}

byte PC98Gfx16ColorsDriver::remapTextColor(byte color) const {
	// Always black for QFG and SCI1. For QFG, this is on purpose. The code just copies the inverted glyph data
	// into all 4 vmem planes. For SCI1 the driver opcode actually has a pen color argument, but it isn't put
	// to any use. Not sure if it is intended.
	if (_fontStyle != kFontStyleTextMode)
		return 0;

	color &= 7;
	// This seems to be a bug in the original PQ2 interpreter, which I replicate, so that we get the same colors.
	// What they were trying to do is just getting the rgb bits in the right order (switch red and green). But
	// instead, before checking and setting the bits, they also copy the full color byte to the target color. So,
	// the extra bits come just on top. The result: All green and red colors are turned into yellow, all magenta
	// and cyan colors are turned into white.
	if (color & 2)
		color |= 4;
	if (color & 4)
		color |= 2;
	// This is the blue color that PQ2 uses basically for all Japanese text...
	if (color == 0)
		color = 1;

	byte textCol = color;
	color += 0x10;

	if (_textModePalette) {
		// If we have used up the whole space of the CLUT8 for the undithering, we try
		// to relocate the color which will work for all text mode colors with the default
		// palette that is used by the PC-98 ports...
		for (int i = 0; i < 256; ++i) {
			if (_convPalette[i * 3] != _textModePalette[textCol * 3] || _convPalette[i * 3 + 1] != _textModePalette[textCol * 3 + 1] || _convPalette[i * 3 + 2] != _textModePalette[textCol * 3 + 2])
				continue;
			color = i;
			break;
		}
		if (color >= 16)
			color = 0;
	}
	return color;
}

SCI0_PC98Gfx8ColorsDriver::SCI0_PC98Gfx8ColorsDriver(bool cursorScaleHeight, bool useTextModeForSJISChars, bool rgbRendering) :
	UpscaledGfxDriver(8, false, rgbRendering), _cursorScaleHeightOnly(cursorScaleHeight), _useTextMode(useTextModeForSJISChars), _convPalette(nullptr) {
	byte *col = new byte[8 * 3]();
	_convPalette = col;

	for (uint8 i = 0; i < 8; ++i) {
		*col++ = (i & 4) ? 0xff : 0;
		*col++ = (i & 2) ? 0xff : 0;
		*col++ = (i & 1) ? 0xff : 0;
	}
}

SCI0_PC98Gfx8ColorsDriver::~SCI0_PC98Gfx8ColorsDriver() {
	delete[] _convPalette;
}

void pc98SimpleDither(byte *dst, const byte *src, int pitch, int w, int h) {
	int dstPitch = pitch << 1;
	byte *d1 = dst;
	byte *d2 = d1 + dstPitch;
	pitch -= w;
	dstPitch += (pitch << 1);

	while (h--) {
		for (int i = 0; i < w; ++i) {
			uint8 v = *src++;
			d1[0] = d2[0] = (v & 7);
			d1[1] = d2[1] = (v & 8) ? (v & 7) : 0;
			d1 += 2;
			d2 += 2;
		}
		src += pitch;
		d1 += dstPitch;
		d2 += dstPitch;
	}
}

void SCI0_PC98Gfx8ColorsDriver::initScreen(const Graphics::PixelFormat *format) {
	UpscaledGfxDriver::initScreen(format);
	_renderScaled = &pc98SimpleDither;
	if (_useTextMode)
		_renderGlyph = &renderPC98GlyphFat;
	assert(_convPalette);
	GfxDefaultDriver::setPalette(_convPalette, 0, 8, true, nullptr, nullptr);
}

void SCI0_PC98Gfx8ColorsDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	adjustCursorBuffer(w, _cursorScaleHeightOnly ? h << 1 : h);
	const byte *s = reinterpret_cast<const byte*>(cursor);
	byte *d = _compositeBuffer;
	uint32 newKeyColor = 0xFF;

	for (uint i = 0; i < h; ++i) {
		for (uint ii = 0; ii < w; ++ii) {
			byte col = *s++;
			*d++ = (col == keycolor) ? newKeyColor : (col & 7);
		}
	}

	// Special case for PQ2 which 2x scales the cursor height
	if (_cursorScaleHeightOnly) {
		s = _compositeBuffer + h * w - w;
		d = _compositeBuffer + h * w * 2 - w;

		for (uint i = 0; i < h; ++i) {
			memcpy(d, s, w);
			d -= w;
			memcpy(d, s, w);
			d -= w;
			s -= w;
		}
		h <<= 1;
		hotspotX <<= 1;
	}

	CursorMan.replaceCursor(_compositeBuffer, w, h, hotspotX, hotspotY, newKeyColor);
}

byte SCI0_PC98Gfx8ColorsDriver::remapTextColor(byte color) const {
	// Always black. For QFG, this is on purpose. The code just copies the inverted glyph data into all 4 vmem planes.
	if (!_useTextMode)
		return 0;

	color &= 7;
	// This seems to be a bug in the original PQ2 interpreter, which I replicate, so that we get the same colors.
	// What they were trying to do is just getting the rgb bits in the right order (switch red and green). But
	// instead, before checking and setting the bits, they also copy the full color byte to the target color. So,
	// the extra bits come just on top. The result: All green and red colors are turned into yellow, all magenta
	// and cyan colors are turned into white.
	if (color & 2)
		color |= 4;
	if (color & 4)
		color |= 2;
	// This is the blue color that PQ2 uses basically for all Japanese text...
	if (color == 0)
		color = 1;

	return color;
}

const char *SCI0_PC98Gfx8ColorsDriver::_driverFiles[2] = { "9801V8M.DRV", "9801VID.DRV" };

SCI1_PC98Gfx8ColorsDriver::SCI1_PC98Gfx8ColorsDriver(bool rgbRendering) : UpscaledGfxDriver(1, true, rgbRendering), _ditheringTable(nullptr), _convPalette(nullptr) {
	Common::File drv;
	if (!drv.open(_driverFile))
		error("SCI1_PC98Gfx8ColorsDriver: Failed to open '%s'", _driverFile);

	uint16 eprcOffs = 0;

	uint32 cmd = drv.readUint32LE();
	if ((cmd & 0xFF) == 0xE9)
		eprcOffs = ((cmd >> 8) & 0xFFFF) + 3;

	if (!eprcOffs || drv.readUint32LE() != 0x87654321 || !drv.skip(1) || !drv.seek(drv.readByte(), SEEK_CUR) || !drv.seek(drv.readByte(), SEEK_CUR))
		error("SCI1_PC98Gfx8ColorsDriver: Driver file '%s' unknown version", _driverFile);

	uint32 pos = (drv.pos() + 1) & ~1;

	drv.seek(pos + 2);
	drv.seek(drv.readUint16LE());
	byte *buff = new byte[190];
	drv.read(buff, 190);

	uint16 tableOffs = 0;
	int step = 0;
	for (int i = 0; i < 182 && !tableOffs; ++i) {
		uint32 c = READ_BE_UINT32(buff + i);
		if (step == 0 && ((c & 0xFFF0FFF0) != 0xD1E0D1E0 || (READ_BE_UINT32(buff + i + 4) ^ c)))
			continue;

		if (step == 0) {
			step = 1;
			i += 7;
			continue;
		}

		if (c >> 20 != 0x81C || ((READ_BE_UINT32(buff + i + 4) >> 20) ^ (c >> 20)))
			continue;

		if ((c & 0xFFFF) == READ_BE_UINT16(buff + i + 6))
			tableOffs = FROM_BE_16(c);
	}
	delete[] buff;

	if (!tableOffs)
		error("SCI1_PC98Gfx8ColorsDriver: Failed to load dithering data from '%s'", _driverFile);

	drv.seek(tableOffs);
	byte *dmx = new byte[96]();
	drv.read(dmx, 96);

	if (drv.readUint16LE() != 0xA800 || drv.readUint16LE() != 0xB000)
		error("SCI1_PC98Gfx8ColorsDriver: Driver file '%s' unknown version", _driverFile);

	drv.close();

	byte *dt = new byte[1536]();
	_ditheringTable = dt;	

	for (uint16 i = 0; i < 256; ++i) {
		for (int ii = 0; ii < 6; ++ii)
			*dt++ = (dmx[(i >> 4) * 6 + ii] & 0xCC) | (dmx[(i & 0x0f) * 6 + ii] & 0x33);
	}

	delete[] dmx;

	_textAlignX = 1;

	byte *col = new byte[8 * 3]();
	_convPalette = col;

	for (uint8 i = 0; i < 8; ++i) {
		*col++ = (i & 2) ? 0xff : 0;
		*col++ = (i & 1) ? 0xff : 0;
		*col++ = (i & 4) ? 0xff : 0;
	}
}

SCI1_PC98Gfx8ColorsDriver::~SCI1_PC98Gfx8ColorsDriver() {
	delete[] _ditheringTable;
	delete[] _convPalette;
}

void renderPlanarMatrix(byte *dst, const byte *src, int pitch, int w, int h, const byte *tbl) {
	int dstPitch = pitch << 1;
	byte *d1 = dst;
	byte *d2 = d1 + dstPitch;
	pitch -= w;
	dstPitch += (pitch << 1);

	while (h--) {
		byte sh = 0;
		for (int i = 0; i < (w >> 1); ++i) {
			const byte *c = &tbl[(src[0] << 4 | src[1]) * 6];
			for (int ii = sh; ii < sh + 4; ++ii) {
				*d1++ = (((c[0] >> (7 - ii)) & 1) << 2) | (((c[1] >> (7 - ii)) & 1) << 1) | ((c[2] >> (7 - ii)) & 1);
				*d2++ = (((c[3] >> (7 - ii)) & 1) << 2) | (((c[4] >> (7 - ii)) & 1) << 1) | ((c[5] >> (7 - ii)) & 1);
			}
			src += 2;
			sh ^= 4;
		}

		src += pitch;
		d1 += dstPitch;
		d2 += dstPitch;
	}
}

void SCI1_PC98Gfx8ColorsDriver::initScreen(const Graphics::PixelFormat *format) {
	UpscaledGfxDriver::initScreen(format);

	_renderGlyph = &renderPC98GlyphFat;

	assert(_convPalette);
	GfxDefaultDriver::setPalette(_convPalette, 0, 8, true, nullptr, nullptr);
}

void SCI1_PC98Gfx8ColorsDriver::copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	assert (h >= 0 && w >= 0);

	byte diff = srcX & 7;
	srcX &= ~7;
	destX &= ~7;
	w = (w + diff + 7) & ~7;

	src += (srcY * pitch + srcX * _srcPixelSize);
	if (src != _currentBitmap)
		updateBitmapBuffer(_currentBitmap, _virtualW * _srcPixelSize, src, pitch, destX * _srcPixelSize, destY, w * _srcPixelSize, h);

	byte *scb = _scaledBitmap + (destY << 1) * _screenW * _srcPixelSize + (destX << 1) * _srcPixelSize;
	renderPlanarMatrix(scb, src, pitch, w, h, _ditheringTable);

	updateScreen(destX << 1, destY << 1,  w << 1, h << 1, palMods, palModMapping);
}

void SCI1_PC98Gfx8ColorsDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	adjustCursorBuffer(w << 1, h << 1);
	const byte *s = reinterpret_cast<const byte*>(cursor);
	byte *d1 = _compositeBuffer;
	uint32 newKeyColor = 0xFF;

	int dstPitch = (w << 1);
	byte *d2 = _compositeBuffer + dstPitch;	

	for (uint i = 0; i < h; ++i) {
		for (uint ii = 0; ii < w; ++ii) {
			byte col = *s++;
			if (col == keycolor) {
				*d1++ = *d2++ = newKeyColor;
				*d1++ = *d2++ = newKeyColor;
			} else {
				*d1++ = *d2++ = (col & 7);
				*d1++ = *d2++ = (col & 8) ? (col & 7) : 0;
			}
		}
		d1 += dstPitch;
		d2 += dstPitch;
	}

	CursorMan.replaceCursor(_compositeBuffer, w << 1, h << 1, hotspotX << 1, hotspotY << 1, newKeyColor);
}

byte SCI1_PC98Gfx8ColorsDriver::remapTextColor(byte) const {
	// Always black. The driver opcode actually has a pen color argument, but it isn't put to any use. Not sure if it is intended.
	return 0;
}
const char *SCI1_PC98Gfx8ColorsDriver::_driverFile = "9801V8.DRV";

#undef GFXDRV_ASSERT_READY
#undef GFXDRV_ASEERT_ALIGNED

} // End of namespace Sci
