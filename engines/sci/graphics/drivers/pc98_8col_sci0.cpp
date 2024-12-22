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

class SCI0_PC98Gfx8ColorsDriver final : public UpscaledGfxDriver {
public:
	SCI0_PC98Gfx8ColorsDriver(bool cursorScaleHeight, bool useTextModeForSJISChars, bool rgbRendering);
	~SCI0_PC98Gfx8ColorsDriver() override;
	void initScreen(const Graphics::PixelFormat *format) override;
	void setPalette(const byte*, uint, uint, bool, const PaletteMod*, const byte*) override {}
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	byte remapTextColor(byte color) const override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformPC98) && checkDriver(_driverFiles, 2); }
private:
	const byte *_convPalette;
	const bool _cursorScaleHeightOnly;
	const bool _useTextMode;
	static const char *_driverFiles[2];
};

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
		_renderGlyph = &SciGfxDrvInternal::renderPC98GlyphFat;
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

SCI_GFXDRV_VALIDATE_IMPL(SCI0_PC98Gfx8Colors)

GfxDriver *SCI0_PC98Gfx8ColorsDriver_create(int rgbRendering, ...) {
	va_list args;
	va_start(args, rgbRendering);
	bool conf = (va_arg(args, int) != 0);
	va_end(args);

	return new SCI0_PC98Gfx8ColorsDriver(conf, conf, rgbRendering != 0);
}

} // End of namespace Sci
