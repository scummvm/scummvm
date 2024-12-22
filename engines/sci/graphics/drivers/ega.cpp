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


#include "common/file.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "sci/graphics/drivers/gfxdriver_intern.h"

namespace Sci {

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
			SciGfxDrvInternal::updateRGBPalette<uint16>(rgbpal, _convPalette, 0, 16, format);
		else if (_pixelSize == 4)
			SciGfxDrvInternal::updateRGBPalette<uint32>(rgbpal, _convPalette, 0, 16, format);
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
		SciGfxDrvInternal::updateBitmapBuffer(_currentBitmap, _screenW, src, pitch, destX, destY, w, h);

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
		GFXDRV_ERR_OPEN(_driverFile);

	uint16 eprcOffs = 0;

	uint32 cmd = drv.readUint32LE();
	if ((cmd & 0xFF) == 0xE9)
		eprcOffs = ((cmd >> 8) & 0xFFFF) + 3;

	if (!eprcOffs || drv.readUint32LE() != 0x87654321 || !drv.skip(1) || !drv.seek(drv.readByte(), SEEK_CUR) || !drv.seek(drv.readByte(), SEEK_CUR) || drv.readUint32LE() != 0xFEDCBA98 || !drv.skip(4))
		GFXDRV_ERR_VERSION(_driverFile);

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
		GFXDRV_ERR_VERSION(_driverFile);

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

SCI_GFXDRV_VALIDATE_IMPL(SCI1_EGA)

GfxDriver *SCI1_EGADriver_create(int rgbRendering, ...) {
	return new SCI1_EGADriver(rgbRendering != 0);
}

} // End of namespace Sci
