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

class PC98Gfx16ColorsDriver final : public UpscaledGfxDriver {
public:
	enum SjisFontStyle {
		kFontStyleNone,
		kFontStyleTextMode,
		kFontStyleSpecialSCI1
	};

	PC98Gfx16ColorsDriver(int textAlignX, bool cursorScaleWidth, bool cursorScaleHeight, SjisFontStyle sjisFontStyle, bool rgbRendering, bool needsUnditheringPalette);
	~PC98Gfx16ColorsDriver() override;
	void initScreen(const Graphics::PixelFormat *format) override;
	void setPalette(const byte*, uint, uint, bool, const PaletteMod*, const byte*) override {}
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	byte remapTextColor(byte color) const override;
private:
	const byte *_convPalette;
	const byte *_textModePalette;
	const bool _cursorScaleHeightOnly;
	SjisFontStyle _fontStyle;
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
		_renderGlyph = &SciGfxDrvInternal::renderPC98GlyphFat;

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

GfxDriver *PC98Gfx16ColorsDriver_create(int rgbRendering, ...) {
	va_list args;
	va_start(args, rgbRendering);
	int config = va_arg(args, int);
	va_arg(args, int);
	va_arg(args, int);
	bool undither = (va_arg(args, int) != 0);
	va_end(args);

	GfxDriver *result = nullptr;
	if (config == 0)
		result = new PC98Gfx16ColorsDriver(8, false, false, PC98Gfx16ColorsDriver::kFontStyleNone, rgbRendering != 0, true);
	else if (config == 1)
		result = new PC98Gfx16ColorsDriver(1, true, true, PC98Gfx16ColorsDriver::kFontStyleSpecialSCI1, rgbRendering != 0, true);
	else if (config == 2) {
		// PQ2 is a bit special, probably the oldest of the PC-98 ports. Unlike all the others, it uses text mode print,
		// so the text color is a system color outside the normal 16 colors palette. The original does not even have a
		// 16 colors mode driver. Only the 8 colors mode, where the colors are identical for text and graphics mode.
		// But we do want to provide the 16 colors mode, since it is not a big deal (i.e., it does not require data
		// from a driver file and the fat print is also already there for the 8 colors mode). So we just make the
		// necessary adjustments.
		result = new PC98Gfx16ColorsDriver(8, false, true, PC98Gfx16ColorsDriver::kFontStyleTextMode, rgbRendering != 0, undither);
	}
	return result;
}

} // End of namespace Sci
