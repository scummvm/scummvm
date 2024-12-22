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

class WindowsGfx16ColorsDriver final : public SCI1_EGADriver {
public:
	// The original does not take into account the extra lines required for the 200->440 vertical scaling. There is a noticeable dithering glitch every 11th line, as the
	// two pixels of the checkerbox pattern appear in the wrong order. I have implemented a fix for this which can be activated with the fixDithering parameter.
	WindowsGfx16ColorsDriver(bool fixDithering, bool rgbRendering);
	~WindowsGfx16ColorsDriver() override {}
	void initScreen(const Graphics::PixelFormat *format) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	Common::Point getRealCoords(Common::Point &pos) const override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformWindows && checkDriver(&_driverFile, 1)); }
private:
	void loadData() override;
	void renderBitmap(byte *dst, const byte *src, int pitch, int y, int w, int h, const byte *patterns, const byte *palette, uint16 &realWidth, uint16 &realHeight) override;
	LineProc _renderLine2;
	const bool _enhancedDithering;
	static const char *_driverFile;
};

WindowsGfx16ColorsDriver::WindowsGfx16ColorsDriver(bool enhancedDithering, bool rgbRendering) : SCI1_EGADriver(rgbRendering), _enhancedDithering(enhancedDithering), _renderLine2(nullptr) {
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

void WindowsGfx16ColorsDriver::initScreen(const Graphics::PixelFormat *format) {
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

void WindowsGfx16ColorsDriver::replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) {
	GFXDRV_ASSERT_READY;
	// The original windows interpreter always renders the cursor as b/w, regardless of which cursor views (the DOS
	// cursors or the new Windows ones) the user selects. This is also regardless of color mode (16 or 256 colors).
	byte col1 = SciGfxDrvInternal::findColorInPalette(0x00000000, _convPalette, _numColors);
	byte col2 = SciGfxDrvInternal::findColorInPalette(0x00FFFFFF, _convPalette, _numColors);
	SciGfxDrvInternal::renderWinMonochromeCursor(_compositeBuffer, cursor, _currentPalette, w, h, hotspotX, hotspotY, col1, col2, keycolor, false);
	CursorMan.replaceCursor(_compositeBuffer, w, h, hotspotX, hotspotY, keycolor);
}

Common::Point WindowsGfx16ColorsDriver::getRealCoords(Common::Point &pos) const {
	return Common::Point(pos.x << 1, (pos.y << 1) + (pos.y + 4) / 5);
}

void WindowsGfx16ColorsDriver::loadData() {
	Common::File file;
	if (!file.open(_driverFile))
		GFXDRV_ERR_OPEN(_driverFile);

	int64 sz = file.size();
	byte *buf = new byte[sz];
	file.read(buf, sz);
	file.close();

	// We can keep the search for the table simple, since there are only two supported executables (KQ6
	// Windows and SQ4 Windows) and both contain the following value only within the pattern table...
	uint32 srch = FROM_LE_32(0xCC4C4404);

	const byte *tblOffs = nullptr;
	for (const byte *pos = buf; pos < buf + sz - 67 && tblOffs == nullptr; ++pos) {
		// We check three times, just to be sure. Checking once would actually suffice.
		if (READ_UINT32(pos) != srch || READ_UINT32(pos + 8) != srch || READ_UINT32(pos + 64) != srch)
			continue;
		tblOffs = pos - 4;
	}

	if (tblOffs == nullptr)
		error("%s(): Failed to load 16 colors match table", __FUNCTION__);

	byte *tbl = new byte[512];
	memcpy(tbl, tblOffs, 512);
	_egaMatchTable = tbl;

	delete[] buf;

	_colAdjust = (_egaMatchTable[482] == 0x79) ? 4 : 0;
	_numColors = 16;
}

void WindowsGfx16ColorsDriver::renderBitmap(byte *dst, const byte *src, int pitch, int y, int w, int h, const byte *patterns, const byte *palette, uint16 &realWidth, uint16 &realHeight) {
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

const char *WindowsGfx16ColorsDriver::_driverFile = "SCIWV.EXE";

SCI_GFXDRV_VALIDATE_IMPL(WindowsGfx16Colors)

GfxDriver *WindowsGfx16ColorsDriver_create(int rgbRendering, ...) {
	return new WindowsGfx16ColorsDriver(true, rgbRendering != 0);
}

} // End of namespace Sci
