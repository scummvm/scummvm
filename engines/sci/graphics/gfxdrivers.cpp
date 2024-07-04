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

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "sci/graphics/gfxdrivers.h"
#include "sci/resource/resource.h"
#include "sci/sci.h"
#include "sci/version.h"

namespace Sci {

Common::Point GfxDriver::getMousePos() const {
	return g_system->getEventManager()->getMousePos();
}

void GfxDriver::clearRect(const Common::Rect &r) const {
	g_system->fillScreen(r, 0);
}

GfxDefaultDriver::GfxDefaultDriver(uint16 screenWidth, uint16 screenHeight) : GfxDriver(screenWidth, screenHeight, 0, 1) {
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

void GfxDefaultDriver::setPalette(const byte *colors, uint start, uint num) {
	g_system->getPaletteManager()->setPalette(colors, start, num);
}

void GfxDefaultDriver::copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) {
	g_system->copyRectToScreen(src, pitch, x, y, w, h);
}

void GfxDefaultDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	CursorMan.replaceCursor(cursor, w, h, hotspotX, hotspotY, keycolor);
}

SCI0_DOSPreVGADriver::SCI0_DOSPreVGADriver(int numColors, int screenW, int screenH, int horizontalAlignment) : GfxDriver(screenW, screenH, numColors, horizontalAlignment), _palNeedUpdate(true), _colors(nullptr), _compositeBuffer(nullptr) {
	_compositeBuffer = new byte[screenW * screenH]();
}

SCI0_DOSPreVGADriver::~SCI0_DOSPreVGADriver() {
	delete[] _compositeBuffer;
}

bool SCI0_DOSPreVGADriver::checkDriver(const char *const *driverNames, int listSize) {
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

void SCI0_DOSPreVGADriver::assignPalette(const byte *colors) {
	_colors = colors;
}

void SCI0_DOSPreVGADriver::setPalette(const byte*, uint, uint) {
	if (!_palNeedUpdate || !_colors)
		return;
	_palNeedUpdate = false;;
	g_system->getPaletteManager()->setPalette(_colors, 0, _numColors);
}

SCI0_CGADriver::SCI0_CGADriver(bool emulateCGAModeOnEGACard) : SCI0_DOSPreVGADriver(4, 320, 200, 1), _cgaPatterns(nullptr), _disableMode5(emulateCGAModeOnEGACard) {
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

void SCI0_CGADriver::copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) {
	// We can't really properly fix the boundaries here, we have to do that before calling this function.
	assert(!(w & _hAlign));
	assert(!(x & _hAlign));

	byte *dst = _compositeBuffer;
	pitch -= w;
	int ty = y;

	for (int i = 0; i < h; ++i) {
		int tx = x & 3;
		++ty;
		for (int ii = 0; ii < (w >> 1); ++ii) {
			uint16 pattern = _cgaPatterns[((src[0] & 0x0f) << 4) | (src[1] & 0x0f)];
			src += 2;
			byte sh = (ty & 3) << 1;
			byte lo = ((pattern & 0xff) >> sh) | ((pattern & 0xff) << (8 - sh));
			byte hi = (pattern >> (8 + sh)) | ((pattern >> 8) << (8 - sh));
			*dst++ = (lo >> (6 - (tx << 1))) & 3;
			*dst++ = (hi >> (4 - (tx << 1))) & 3;
			tx ^= 2;
		}
		src += pitch;
	}

	g_system->copyRectToScreen(_compositeBuffer, w, x, y, w, h);
}

void SCI0_CGADriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	// Instead of implementing the original cursor rendering code, we rely on the 8 bit cursor that
	// has already been generated by the engine. Of course, that code could be moved to the EGA
	// driver class and implemented again for each mode, but I don't see the benefit. Instead,
	// we simply convert the colors as needed...
	assert(keycolor == 1);
	const byte *s = reinterpret_cast<const byte*>(cursor);
	byte *d = _compositeBuffer;
	for (uint i = w * h; i; --i)
		*d++ = *s++ & 3;

	CursorMan.replaceCursor(_compositeBuffer, w, h, hotspotX, hotspotY, keycolor);
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

SCI0_CGABWDriver::SCI0_CGABWDriver(uint32 monochromeColor) : SCI0_DOSPreVGADriver(2, 640, 400, 1), _monochromePatterns(nullptr), _earlyVersion(false) {
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

void SCI0_CGABWDriver::copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) {
	// We can't really properly fix the boundaries here, we have to do that before calling this function.
	assert(!(w & _hAlign));
	assert(!(x & _hAlign));

	byte *dst1 = _compositeBuffer;
	byte *dst2 = _compositeBuffer + (w << 1);
	int ty = y & 7;
	pitch -= w;

	for (int i = 0; i < h; ++i) {
		int tx = x & 3;
		if (_earlyVersion) {
			++ty;
			for (int ii = 0; ii < (w >> 1); ++ii) {
				uint16 p16 = reinterpret_cast<const uint16*>(_monochromePatterns)[((src[0] & 0x0f) << 4) | (src[1] & 0x0f)];
				src += 2;
				byte sh = (ty & 3) << 1;
				byte lo = ((p16 & 0xff) >> sh) | ((p16 & 0xff) << (8 - sh));
				byte hi = (p16 >> (8 + sh)) | ((p16 >> 8) << (8 - sh));
				*dst1++ = *dst2++ = ((lo >> (6 - (tx << 1))) >> 1) & 1;
				*dst1++ = *dst2++ = (lo >> (6 - (tx << 1))) & 1;
				*dst1++ = *dst2++ = ((hi >> (4 - (tx << 1))) >> 1) & 1;
				*dst1++ = *dst2++ = (hi >> (4 - (tx << 1))) & 1;
				tx ^= 2;
			}
		} else {
			for (int ii = 0; ii < w; ++ii) {
				byte p = _monochromePatterns[((*src++ & 0x0f) << 3) + ty] >> (6 - (tx << 1));
				*dst1++ = *dst2++ = (p >> 1) & 1;
				*dst1++ = *dst2++ = p & 1;
				tx = (tx + 1) & 3;
			}
			ty = (ty + 1) & 7;
		}
		src += pitch;
		dst1 += (w << 1);
		dst2 += (w << 1);
	}

	g_system->copyRectToScreen(_compositeBuffer, w << 1, x << 1, y << 1, w << 1, h << 1);
}

void SCI0_CGABWDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	// Instead of implementing the original cursor rendering code, we rely on the 8 bit cursor that
	// has already been generated by the engine. Of course, that code could be moved to the EGA
	// driver class and implemented again for each mode, but I don't see the benefit. Instead,
	// we simply convert the colors as needed and scale the cursor...
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

const char *SCI0_CGABWDriver::_driverFiles[2] = { "CGA320BW.DRV", "CGA320M.DRV" };

SCI0_HerculesDriver::SCI0_HerculesDriver(uint32 monochromeColor, bool cropImage) : SCI0_DOSPreVGADriver(2, cropImage ? 640 : 720, cropImage ? 300 : 350, 0),
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

void SCI0_HerculesDriver::copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) {
	// We can't really properly fix the boundaries here, we have to do that before calling this function.
	assert(!(w & _hAlign));
	assert(!(x & _hAlign));

	byte *dst = _compositeBuffer;
	byte sw = y & 1;
	y = (y & ~1) * 3 / 2 + (y & 1);
	int ty = y & 7;
	int rh = 0;

	for (int i = 0; i < h; ++i) {
		const byte *src2 = src;
		int tx = x & 3;
		for (int ii = 0; ii < w; ++ii) {
			byte p = _monochromePatterns[((*src2++ & 0x0f) << 3) + ty] >> (6 - (tx << 1));
			*dst++ = (p >> 1) & 1;
			*dst++ = p & 1;
			tx = (tx + 1) & 3;
		}

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

	g_system->copyRectToScreen(_compositeBuffer, w << 1, (x << 1) + _centerX, y + _centerY, w << 1, rh);
}

void SCI0_HerculesDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	// Instead of implementing the original cursor rendering code, we rely on the 8 bit cursor that
	// has already been generated by the engine. Of course, that code could be moved to the EGA
	// driver class and implemented again for each mode, but I don't see the benefit. Instead,
	// we simply convert the colors as needed and scale the cursor...
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

const char *SCI0_HerculesDriver::_driverFile = "HERCMONO.DRV";

} // End of namespace Sci
