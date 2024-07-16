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
	warning("Driver file %s not found. Starting game in EGA mode", missing.c_str());
	return false;
}

GfxDefaultDriver::GfxDefaultDriver(uint16 screenWidth, uint16 screenHeight, bool rgbRendering) : GfxDriver(screenWidth, screenHeight, 0),
	_srcPixelSize(1), _requestRGBMode(rgbRendering), _compositeBuffer(nullptr), _currentBitmap(nullptr), _internalPalette(nullptr), _currentPalette(nullptr) {
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

	if (_numColors > 16 || _pixelSize > 1) {
		// Not needed for SCI0, except for rgb rendering
		_currentBitmap = new byte[_screenW * _screenH * _srcPixelSize]();
		assert(_currentBitmap);
		_currentPalette = new byte[256 * 3]();
		assert(_currentPalette);
		if (_pixelSize != _srcPixelSize) {
			_internalPalette = new byte[256 * _pixelSize]();
			assert(_internalPalette);
		}
	}

	_ready = true;
}

void GfxDefaultDriver::setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	if (_pixelSize > 1) {
		updatePalette(colors, start, num);
		if (update)
			copyRectToScreen(_currentBitmap, 0, 0, _screenW, 0, 0, _screenW, _screenH, palMods, palModMapping);
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
}

void GfxDefaultDriver::copyCurrentBitmap(byte *dest, uint32 size) const {
	GFXDRV_ASSERT_READY;
	assert(dest);
	assert(size <= (uint32)(_screenW * _screenH));

	// SCI 0 should not make calls to this method (except when using palette mods), but we have to know if it does...
	if (!_currentBitmap)
		error("GfxDefaultDriver::copyDataFromCurrentBitmap(): unexpected call");

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

template <typename T> void render(byte *dst, const byte *src, int pitch, int w, int h, const byte *pal) {
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
template <typename T> void renderMod(byte *dst, const byte *src, int pitch, int w, int h, const byte *srcPal, const byte *internalPal, Graphics::PixelFormat &f, const PaletteMod *mods, const byte *modMapping) {
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

void GfxDefaultDriver::generateOutput(byte *dst, const byte *src, int pitch, int w, int h, const PaletteMod *palMods, const byte *palModMapping) {
	if (palMods && palModMapping) {
		if (_pixelSize == 2)
			renderMod<uint16>(dst, src, pitch, w, h, _currentPalette, _internalPalette, _format, palMods, palModMapping);
		else if (_pixelSize == 4)
			renderMod<uint32>(dst, src, pitch, w, h, _currentPalette, _internalPalette, _format, palMods, palModMapping);
		else
			error("GfxDefaultDriver::generateOutput(): Unsupported pixel size %d", _pixelSize);
	} else {
		if (_pixelSize == 2)
			render<uint16>(dst, src, pitch, w, h, _internalPalette);
		else if (_pixelSize == 4)
			render<uint32>(dst, src, pitch, w, h, _internalPalette);
		else
			error("GfxDefaultDriver::generateOutput(): Unsupported pixel size %d", _pixelSize);
	}
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

SCI0_CGADriver::SCI0_CGADriver(bool emulateCGAModeOnEGACard, bool rgbRendering) : SCI0_DOSPreVGADriver(4, 320, 200, rgbRendering), _cgaPatterns(nullptr), _disableMode5(emulateCGAModeOnEGACard) {
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

	srcX &= ~1;
	destX &= ~1;
	w = (w + 1) & ~1;

	src += (srcY * pitch + srcX);

	byte *dst = _compositeBuffer;
	int ty = destY;

	for (int i = 0; i < h; ++i) {
		_renderLine(dst, src, w, destX & 3, ++ty, _cgaPatterns, _internalPalette);
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

SCI0_CGABWDriver::SCI0_CGABWDriver(uint32 monochromeColor, bool rgbRendering) : SCI0_DOSPreVGADriver(2, 640, 400, rgbRendering), _monochromePatterns(nullptr), _earlyVersion(false) {
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
		srcX &= ~1;
		destX &= ~1;
		w = (w + 1) & ~1;
	}

	src += (srcY * pitch + srcX);

	for (int i = 0; i < h; ++i) {
		_renderLine(dst, src, w, destX & 3, ty, _monochromePatterns, _internalPalette);
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

void SCI0_CGABWDriver::clearRect(const Common::Rect &r) const {
	Common::Rect r2(r.left << 1, r.top << 1, r.right << 1, r.bottom << 1);
	GfxDriver::clearRect(r2);
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
	_centerX(cropImage ? 0 : 40), _centerY(cropImage ? 0 : 25), _monochromePatterns(nullptr) {
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
		_renderLine(dst, src2, w, destX & 3, ty, _monochromePatterns, _internalPalette);
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

void SCI0_HerculesDriver::clearRect(const Common::Rect &r) const {
	Common::Rect r2((r.left << 1) + _centerX, (r.top & ~1) * 3 / 2 + (r.top & 1) + _centerY, (r.right << 1) + 40, (r.bottom & ~1) * 3 / 2 + (r.bottom & 1) + 25);
	GfxDriver::clearRect(r2);
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


SCI1_VGAGreyScaleDriver::SCI1_VGAGreyScaleDriver(bool rgbRendering) : GfxDefaultDriver(320, 200, rgbRendering), _greyScalePalette(nullptr) {
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
	_currentBitmap(nullptr), _compositeBuffer(nullptr), _currentPalette(nullptr), _internalPalette(nullptr), _colAdjust(0) {
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

SCI1_EGADriver::~SCI1_EGADriver() {
	delete[] _egaMatchTable;
	delete[] _egaColorPatterns;
	delete[] _compositeBuffer;
	delete[] _currentBitmap;
	delete[] _currentPalette;
	delete[] _internalPalette;
}

template <typename T> void ega640RenderLine(byte *&dst, const byte *src, int w, const byte *patterns, const byte *pal) {
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
	Graphics::PixelFormat format(Graphics::PixelFormat::createFormatCLUT8());
	initGraphics(_screenW << 1, _screenH << 1, _requestRGBMode ? nullptr : &format);
	format = g_system->getScreenFormat();
	_pixelSize = format.bytesPerPixel;

	if (_requestRGBMode && _pixelSize == 1)
		warning("SCI1_EGADriver::initScreen(): RGB rendering not available in this ScummVM build");

	static const byte egaColors[48] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0x00, 0xAA, 0xAA,
		0xAA, 0x00, 0x00, 0xAA, 0x00, 0xAA, 0xAA, 0x55, 0x00, 0xAA, 0xAA, 0xAA,
		0x55, 0x55, 0x55, 0x55, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x55, 0xFF, 0xFF,
		0xFF, 0x55, 0x55, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF
	};

	delete[] _egaColorPatterns;
	delete[] _compositeBuffer;
	delete[] _currentBitmap;
	delete[] _currentPalette;
	delete[] _internalPalette;
	_internalPalette = nullptr;
	_egaColorPatterns = _compositeBuffer = _currentBitmap = _currentPalette = nullptr;

	if (_pixelSize == 1) {
		g_system->getPaletteManager()->setPalette(egaColors, 0, ARRAYSIZE(egaColors) / 3);
	} else {
		byte *rgbpal = new byte[_numColors * _pixelSize]();
		assert(rgbpal);

		if (_pixelSize == 2)
			updateRGBPalette<uint16>(rgbpal, egaColors, 0, ARRAYSIZE(egaColors) / 3, format);
		else if (_pixelSize == 4)
			updateRGBPalette<uint32>(rgbpal, egaColors, 0, ARRAYSIZE(egaColors) / 3, format);
		else
			error("SCI1_EGADriver::initScreen(): Unsupported screen format");
		_internalPalette = rgbpal;
		CursorMan.replaceCursorPalette(egaColors, 0, ARRAYSIZE(egaColors) / 3);
	}

	_compositeBuffer = new byte[(_screenW << 1) * (_screenH << 1) * _pixelSize]();
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

	src += (srcY * pitch + srcX);

	if (src != _currentBitmap)
		updateBitmapBuffer(_currentBitmap, _screenW, src, pitch, destX, destY, w, h);

	byte *dst = _compositeBuffer;
	for (int i = 0; i < h; ++i) {
		_renderLine(dst, src, w, _egaColorPatterns, _internalPalette);
		src += pitch;
	}

	g_system->copyRectToScreen(_compositeBuffer, (w << 1) * _pixelSize, destX << 1, destY << 1, w << 1, h << 1);
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

	keycolor = newKeyColor;

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

Common::Point SCI1_EGADriver::getMousePos() const {
	Common::Point res = GfxDriver::getMousePos();
	res.x >>= 1;
	res.y >>= 1;
	return res;
}

void SCI1_EGADriver::clearRect(const Common::Rect &r) const {
	Common::Rect r2(r.left << 1, r.top << 1, r.right << 1, r.bottom << 1);
	GfxDriver::clearRect(r2);
}

const char *SCI1_EGADriver::_driverFile = "EGA640.DRV";

#undef GFXDRV_ASSERT_READY
#undef GFXDRV_ASEERT_ALIGNED

} // End of namespace Sci
