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
#include "common/file.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "sci/graphics/drivers/gfxdriver_intern.h"
#include "sci/sci.h"

namespace Sci {

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
			SciGfxDrvInternal::updateRGBPalette<uint16>(rgbpal, _colors, 0, _numColors, format);
		else if (_pixelSize == 4)
			SciGfxDrvInternal::updateRGBPalette<uint32>(rgbpal, _colors, 0, _numColors, format);
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

} // End of namespace Sci

namespace SciGfxDrvInternal {

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

template <typename T> void updateRGBPalette(byte *dest, const byte *src, uint start, uint num, Graphics::PixelFormat &f) {
	T *dst = &reinterpret_cast<T*>(dest)[start];
	for (uint i = 0; i < num; ++i) {
		*dst++ = f.RGBToColor(src[0], src[1], src[2]);
		src += 3;
	}
}


template void updateRGBPalette<byte>(byte *dest, const byte *src, uint start, uint num, Graphics::PixelFormat &f);
template void updateRGBPalette<uint16>(byte *dest, const byte *src, uint start, uint num, Graphics::PixelFormat &f);
template void updateRGBPalette<uint32>(byte *dest, const byte *src, uint start, uint num, Graphics::PixelFormat &f);

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

template void scale2x<byte>(byte *dst, const byte *src, int pitch, int w, int h);
template void scale2x<uint16>(byte *dst, const byte *src, int pitch, int w, int h);
template void scale2x<uint32>(byte *dst, const byte *src, int pitch, int w, int h);

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

void renderWinMonochromeCursor(byte *dst, const void *src, const byte *palette, uint &w, uint &h, int &hotX, int &hotY, byte blackColor, byte whiteColor, uint32 &keycolor, bool noScale) {
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

	if (w < 17 && h < 17 && !noScale) {
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

const byte *monochrInit(const char *drvFile, bool &earlyVersion) {
	Common::File drv;
	if (!drv.open(drvFile))
		return nullptr;

	uint16 eprcOffs = 0;

	uint32 cmd = drv.readUint32LE();
	if ((cmd & 0xFF) == 0xE9)
		eprcOffs = ((cmd >> 8) & 0xFFFF) + 3;

	if (!eprcOffs || drv.readUint32LE() != 0x87654321 || !drv.skip(1) || !drv.seek(drv.readByte(), SEEK_CUR) || !drv.seek(drv.readByte(), SEEK_CUR))
		GFXDRV_ERR_VERSION(drv.getName());

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

} // End of namespace SciGfxDrvInternal
