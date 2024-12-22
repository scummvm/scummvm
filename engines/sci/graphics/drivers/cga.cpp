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
#include "graphics/cursorman.h"
#include "sci/graphics/drivers/gfxdriver_intern.h"

namespace Sci {

class SCI0_CGADriver final : public SCI0_DOSPreVGADriver {
public:
	SCI0_CGADriver(bool emulateCGAModeOnEGACard, bool rgbRendering);
	~SCI0_CGADriver() override;
	void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod*, const byte*) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformDOS) && checkDriver(&_driverFile, 1); }
private:
	void setupRenderProc() override;
	uint16 *_cgaPatterns;
	byte _palette[12];
	const bool _disableMode5;
	typedef void (*LineProc)(byte*&, const byte*, int, int, int, const uint16*, const byte*);
	LineProc _renderLine;
	static const char *_driverFile;
};

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
		GFXDRV_ERR_OPEN(_driverFile);

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
		GFXDRV_ERR_VERSION(_driverFile);

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

SCI_GFXDRV_VALIDATE_IMPL(SCI0_CGA)

GfxDriver *SCI0_CGADriver_create(int rgbRendering, ...) {
	return new SCI0_CGADriver(false, rgbRendering != 0);
}

} // End of namespace Sci
