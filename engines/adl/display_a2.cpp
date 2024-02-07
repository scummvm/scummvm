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

// Based on AppleWin's code for NTSC emulation and its RGB Monitor palette
// Copyright (C) 2010-2011, William S Simms
// Copyright (C) 2014-2016, Michael Pohoreski, Tom Charlesworth
// Licensed under GPLv2+

#include "common/stream.h"
#include "common/rect.h"
#include "common/system.h"
#include "common/str.h"
#include "common/config-manager.h"
#include "common/math.h"
#include "common/memstream.h"

#include "graphics/surface.h"
#include "graphics/thumbnail.h"

#include "engines/util.h"

#include "adl/display_a2.h"
#include "adl/adl.h"

namespace Adl {

#define NTSC_REMOVE_BLACK_GHOSTING
// #define NTSC_REMOVE_WHITE_RINGING

// Uppercase-only Apple II font (manually created).
const byte Display_A2::_font[64][8] = {
	{ 0x00, 0x1c, 0x22, 0x2a, 0x3a, 0x1a, 0x02, 0x3c }, { 0x00, 0x08, 0x14, 0x22, 0x22, 0x3e, 0x22, 0x22 }, // @A
	{ 0x00, 0x1e, 0x22, 0x22, 0x1e, 0x22, 0x22, 0x1e }, { 0x00, 0x1c, 0x22, 0x02, 0x02, 0x02, 0x22, 0x1c }, // BC
	{ 0x00, 0x1e, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1e }, { 0x00, 0x3e, 0x02, 0x02, 0x1e, 0x02, 0x02, 0x3e }, // DE
	{ 0x00, 0x3e, 0x02, 0x02, 0x1e, 0x02, 0x02, 0x02 }, { 0x00, 0x3c, 0x02, 0x02, 0x02, 0x32, 0x22, 0x3c }, // FG
	{ 0x00, 0x22, 0x22, 0x22, 0x3e, 0x22, 0x22, 0x22 }, { 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1c }, // HI
	{ 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x22, 0x1c }, { 0x00, 0x22, 0x12, 0x0a, 0x06, 0x0a, 0x12, 0x22 }, // JK
	{ 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x3e }, { 0x00, 0x22, 0x36, 0x2a, 0x2a, 0x22, 0x22, 0x22 }, // LM
	{ 0x00, 0x22, 0x22, 0x26, 0x2a, 0x32, 0x22, 0x22 }, { 0x00, 0x1c, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1c }, // NO
	{ 0x00, 0x1e, 0x22, 0x22, 0x1e, 0x02, 0x02, 0x02 }, { 0x00, 0x1c, 0x22, 0x22, 0x22, 0x2a, 0x12, 0x2c }, // PQ
	{ 0x00, 0x1e, 0x22, 0x22, 0x1e, 0x0a, 0x12, 0x22 }, { 0x00, 0x1c, 0x22, 0x02, 0x1c, 0x20, 0x22, 0x1c }, // RS
	{ 0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 }, { 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1c }, // TU
	{ 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x14, 0x08 }, { 0x00, 0x22, 0x22, 0x22, 0x2a, 0x2a, 0x36, 0x22 }, // VW
	{ 0x00, 0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22 }, { 0x00, 0x22, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08 }, // XY
	{ 0x00, 0x3e, 0x20, 0x10, 0x08, 0x04, 0x02, 0x3e }, { 0x00, 0x3e, 0x06, 0x06, 0x06, 0x06, 0x06, 0x3e }, // Z[
	{ 0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00 }, { 0x00, 0x3e, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3e }, // \]
	{ 0x00, 0x00, 0x00, 0x08, 0x14, 0x22, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e }, // ^_
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08 }, //  !
	{ 0x00, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x14, 0x14, 0x3e, 0x14, 0x3e, 0x14, 0x14 }, // "#
	{ 0x00, 0x08, 0x3c, 0x0a, 0x1c, 0x28, 0x1e, 0x08 }, { 0x00, 0x06, 0x26, 0x10, 0x08, 0x04, 0x32, 0x30 }, // $%
	{ 0x00, 0x04, 0x0a, 0x0a, 0x04, 0x2a, 0x12, 0x2c }, { 0x00, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00 }, // &'
	{ 0x00, 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 }, { 0x00, 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08 }, // ()
	{ 0x00, 0x08, 0x2a, 0x1c, 0x08, 0x1c, 0x2a, 0x08 }, { 0x00, 0x00, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x00 }, // *+
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x04 }, { 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00 }, // ,-
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 }, { 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00 }, // ./
	{ 0x00, 0x1c, 0x22, 0x32, 0x2a, 0x26, 0x22, 0x1c }, { 0x00, 0x08, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x1c }, // 01
	{ 0x00, 0x1c, 0x22, 0x20, 0x18, 0x04, 0x02, 0x3e }, { 0x00, 0x3e, 0x20, 0x10, 0x18, 0x20, 0x22, 0x1c }, // 23
	{ 0x00, 0x10, 0x18, 0x14, 0x12, 0x3e, 0x10, 0x10 }, { 0x00, 0x3e, 0x02, 0x1e, 0x20, 0x20, 0x22, 0x1c }, // 45
	{ 0x00, 0x38, 0x04, 0x02, 0x1e, 0x22, 0x22, 0x1c }, { 0x00, 0x3e, 0x20, 0x10, 0x08, 0x04, 0x04, 0x04 }, // 67
	{ 0x00, 0x1c, 0x22, 0x22, 0x1c, 0x22, 0x22, 0x1c }, { 0x00, 0x1c, 0x22, 0x22, 0x3c, 0x20, 0x10, 0x0e }, // 89
	{ 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x08, 0x04 }, // :;
	{ 0x00, 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10 }, { 0x00, 0x00, 0x00, 0x3e, 0x00, 0x3e, 0x00, 0x00 }, // <=
	{ 0x00, 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04 }, { 0x00, 0x1c, 0x22, 0x10, 0x08, 0x08, 0x00, 0x08 }  // >?
};

struct LineDoubleBright {
	static uint8 blend(uint8 c1, uint8 c2) {
		return c1;
	}
};

struct LineDoubleDim {
	static uint8 blend(uint8 c1, uint8 c2) {
		return (c1 >> 1) + (c1 >> 2);
	}
};

struct BlendBright {
	static uint8 blend(uint8 c1, uint8 c2) {
		return (c1 + c2) >> 1;
	}
};

struct BlendDim {
	static uint8 blend(uint8 c1, uint8 c2) {
		// AppleWin does c1 >>= 2; return (c1 < c2 ? c2 - c1 : 0);
		// I think the following looks a lot better:
		return ((c1 + c2) >> 2) + ((c1 + c2) >> 3);
	}
};

static const uint kColorPhases = 4;
// All PixelWriters have been adjusted to have 3 pixels of "pre-render" that
// will be cut off when blitting to the screen
static const uint kPreRender = 3;

template<typename ColorType, typename T>
class PixelWriter {
public:
	PixelWriter() : _ptr(nullptr), _format(g_system->getScreenFormat()), _phase(0), _window(0) { }

	void setupWrite(ColorType *dest) {
		_ptr = dest;
		_phase = 3;
		_window = 0;
	}

	void writePixels(uint bits) {
		for (uint b = 0; b < 14; ++b) {
			_window <<= 1;
			_window |= bits & 1;
			bits >>= 1;
			*_ptr++ = static_cast<T *>(this)->getColor();
			_phase = (_phase + 1) & 3;
		}
	}

protected:
	ColorType *_ptr;
	Graphics::PixelFormat _format;
	uint _phase;
	uint _window;
};

template<typename ColorType>
class PixelWriterColor : public PixelWriter<ColorType, PixelWriterColor<ColorType> > {
public:
	static const uint kColors = 16;
	typedef LineDoubleBright BlendRegular;
	typedef LineDoubleDim BlendScanlines;

	PixelWriterColor() {
		const byte palette[kColors][3] = {
			{ 0x00, 0x00, 0x00 }, { 0x9d, 0x09, 0x66 }, { 0x2a, 0x2a, 0xe5 }, { 0xc7, 0x34, 0xff },
			{ 0x00, 0x80, 0x00 }, { 0x80, 0x80, 0x80 }, { 0x0d, 0xa1, 0xff }, { 0xaa, 0xaa, 0xff },
			{ 0x55, 0x55, 0x00 }, { 0xf2, 0x5e, 0x00 }, { 0xc0, 0xc0, 0xc0 }, { 0xff, 0x89, 0xe5 },
			{ 0x38, 0xcb, 0x00 }, { 0xd5, 0xd5, 0x1a }, { 0x62, 0xf6, 0x99 }, { 0xff, 0xff, 0xff }
		};

		for (uint pattern = 0; pattern < kColors; ++pattern) {
			uint color = ((pattern & 1) << 3) | ((pattern & 2) << 1) | ((pattern & 4) >> 1) | ((pattern & 8) >> 3);

			for (uint phase = 0; phase < kColorPhases; ++phase) {
				_colors[phase][pattern] = this->_format.RGBToColor(palette[color][0], palette[color][1], palette[color][2]);
				color = ((color & 8) >> 3) | ((color << 1) & 0x0f);
			}
		}
	}

	// >> 2 to synchronize rendering output with NTSC
	ColorType getColor() { return _colors[this->_phase][(this->_window >> 2) & (kColors - 1)]; }

private:
	ColorType _colors[kColorPhases][kColors];
};

template<typename ColorType, uint8 R, uint8 G, uint8 B>
class PixelWriterMono : public PixelWriter<ColorType, PixelWriterMono<ColorType, R, G, B> > {
public:
	static const uint kColors = 2;

	typedef LineDoubleBright BlendRegular;
	typedef LineDoubleDim BlendScanlines;

	PixelWriterMono() {
		_colors[0] = this->_format.RGBToColor(0, 0, 0);
		_colors[1] = this->_format.RGBToColor(R, G, B);
	}

	ColorType getColor() { return _colors[(this->_window >> 3) & (kColors - 1)]; }

private:
	ColorType _colors[kColors];
};

static double filterChroma(double z) {
	static double x[3] = {0, 0, 0};
	static double y[3] = {0, 0, 0};

	x[0] = x[1];
	x[1] = x[2];
	x[2] = z / 7.438011255;

	y[0] = y[1];
	y[1] = y[2];
	y[2] = -x[0] + x[2] + (-0.7318893645 * y[0]) + (1.2336442711 * y[1]);

	return y[2];
}

static double filterLuma(double z) {
	static double x[3] = {0, 0, 0};
	static double y[3] = {0, 0, 0};

	x[0] = x[1];
	x[1] = x[2];
	x[2] = z / 13.71331570;

	y[0] = y[1];
	y[1] = y[2];
	y[2] = x[0] + x[2] + (2.f * x[1]) + (-0.3961075449 * y[0]) + (1.1044202472 * y[1]);

	return y[2];
}

static double filterSignal(double z) {
	static double x[3] = {0, 0, 0};
	static double y[3] = {0, 0, 0};

	x[0] = x[1];
	x[1] = x[2];
	x[2] = z / 7.614490548;

	y[0] = y[1];
	y[1] = y[2];
	y[2] = x[0] + x[2] + (2.0 * x[1]) + (-0.2718798058 * y[0]) + (0.7465656072 * y[1]);

	return y[2];
}

template<typename ColorType>
class PixelWriterColorNTSC : public PixelWriter<ColorType, PixelWriterColorNTSC<ColorType> > {
public:
	static const uint kColors = 4096;

	typedef BlendBright BlendRegular;
	typedef BlendDim BlendScanlines;

	PixelWriterColorNTSC() {
		for (uint phase = 0; phase < kColorPhases; ++phase) {
			double phi = Common::deg2rad(phase * 90.0 + 45.0);
			for (uint s = 0; s < kColors; ++s) {
				uint t = s;
				double y;
				double i = 0.0;
				double q = 0.0;

				for (uint n = 0; n < 12; ++n) {
					double z = (double)(0 != (t & 0x800));
					t = t << 1;

					for (uint k = 0; k < 2; k++ ) {
						const double zz = filterSignal(z);
						double c = filterChroma(zz);
						y = filterLuma(zz - c);

						c = c * 2.0;
						i = i + (c * cos(phi) - i) / 8.0;
						q = q + (c * sin(phi) - q) / 8.0;

						phi += Common::deg2rad(45.0);
					}
				}

				// YIQ to RGB
				const double r64 = y + (0.956 * i) + (0.621 * q);
				const double g64 = y + (-0.272 * i) + (-0.647 * q);
				const double b64 = y + (-1.105 * i) + (1.702 * q);

				uint8 r = CLIP<double>(r64, 0.0, 1.0) * 255;
				uint8 g = CLIP<double>(g64, 0.0, 1.0) * 255;
				uint8 b = CLIP<double>(b64, 0.0, 1.0) * 255;

	#ifdef NTSC_REMOVE_WHITE_RINGING
				if ((s & 0xf) == 15) {
					// white
					r = 255;
					g = 255;
					b = 255;
				}
	#endif

	#ifdef NTSC_REMOVE_BLACK_GHOSTING
				if ((s & 0xf) == 0) {
					// Black
					r = 0;
					g = 0;
					b = 0;
				}
	#endif

				_colors[phase][s] = this->_format.RGBToColor(r, g, b);
			}
		}
	}

	ColorType getColor() { return _colors[this->_phase][(this->_window >> 1) & (kColors - 1)]; }

private:
	ColorType _colors[kColorPhases][kColors];
};

template<typename ColorType>
class PixelWriterMonoNTSC : public PixelWriter<ColorType, PixelWriterMonoNTSC<ColorType> > {
public:
	static const uint kColors = 4096;

	typedef BlendBright BlendRegular;
	typedef BlendDim BlendScanlines;

	PixelWriterMonoNTSC() {
		for (uint s = 0; s < kColors; ++s) {
			uint t = s;
			double y;

			for (uint n = 0; n < 12; ++n) {
				double z = (double)(0 != (t & 0x800));
				t = t << 1;

				for (uint k = 0; k < 2; k++ ) {
					const double zz = filterSignal(z);
					double c = filterChroma(zz);
					y = filterLuma(zz - c);
				}
			}

			const uint8 brightness = CLIP<double>(y, 0.0, 1.0) * 255;
			_colors[s] = this->_format.RGBToColor(brightness, brightness, brightness);
		}
	}

	ColorType getColor() { return _colors[(this->_window >> 1) & (kColors - 1)]; }

private:
	ColorType _colors[kColors];
};

template<typename ColorType, typename GfxWriter, typename TextWriter>
class DisplayImpl_A2 : public Display_A2 {
public:
	DisplayImpl_A2();
	~DisplayImpl_A2() override;

	void renderText() override;
	void renderGraphics() override;

private:
	enum {
		kRenderBufWidth = (kGfxPitch + 1) * 14, // one extra chunk to account for pre-render
		kRenderBufHeight = (kGfxHeight * 2) + 1 // one extra line to simplify scanline mixing
	};

	template<typename BlendFunc>
	void blendScanlines(uint yStart, uint yEnd);

	template<typename Reader, typename Writer>
	void render(Writer &writer);

	ColorType *_renderBuf;
	uint16 _doublePixelMasks[128];

	GfxWriter _writerColor;
	TextWriter _writerMono;
};

template<typename ColorType, typename GfxWriter, typename TextWriter>
DisplayImpl_A2<ColorType, GfxWriter, TextWriter>::DisplayImpl_A2() : _doublePixelMasks() {
	_renderBuf = new ColorType[kRenderBufHeight * kRenderBufWidth]();

	for (uint8 val = 0; val < ARRAYSIZE(_doublePixelMasks); ++val)
		for (uint8 mask = 0; mask < 7; mask++)
			if (val & (1 << mask))
				_doublePixelMasks[val] |= 3 << (mask * 2);
}

template<typename ColorType, typename GfxWriter, typename TextWriter>
DisplayImpl_A2<ColorType, GfxWriter, TextWriter>::~DisplayImpl_A2() {
	delete[] _renderBuf;
}

template<typename ColorType, typename GfxWriter, typename TextWriter>
template<typename Reader, typename Writer>
void DisplayImpl_A2<ColorType, GfxWriter, TextWriter>::render(Writer &writer) {
	uint startY = Reader::getStartY(this);
	const uint endY = Reader::getEndY(this);

	ColorType *ptr = _renderBuf + startY * kRenderBufWidth * 2;

	for (uint y = startY; y < endY; ++y) {
		uint16 lastBit = 0;

		writer.setupWrite(ptr);

		for (uint x = 0; x < kGfxPitch; ++x) {
			const uint8 m = Reader::getBits(this, y, x);

			uint16 bits = _doublePixelMasks[m & 0x7F];

			if (m & 0x80)
				bits = (bits << 1) | lastBit;

			lastBit = (bits >> 13) & 1;

			writer.writePixels(bits);
		}

		// Because of the pre-render, we need to feed
		// in some more bits to get the full picture
		writer.writePixels(0);

		// The odd lines will be filled in later, so skip a line
		ptr += 2 * kRenderBufWidth;
	}

	if (_enableScanlines)
		blendScanlines<typename Writer::BlendScanlines>(startY, endY);
	else
		blendScanlines<typename Writer::BlendRegular>(startY, endY);

	// For the NTSC modes we need to redo the scanline that blends with our first line
	if (GfxWriter::kColors == 4096 && startY > 0) {
		--startY;
		if (_enableScanlines)
			blendScanlines<typename GfxWriter::BlendScanlines>(startY, startY + 1);
		else
			blendScanlines<typename GfxWriter::BlendRegular>(startY, startY + 1);
	}

	g_system->copyRectToScreen(_renderBuf + startY * 2 * kRenderBufWidth + kPreRender, kRenderBufWidth * sizeof(ColorType), 0, startY * 2, kGfxWidth * 2, (endY - startY) * 2);
	g_system->updateScreen();
}

template<typename ColorType, typename GfxWriter, typename TextWriter>
template<typename BlendType>
void DisplayImpl_A2<ColorType, GfxWriter, TextWriter>::blendScanlines(uint yStart, uint yEnd) {
	const Graphics::PixelFormat rgbFormat = g_system->getScreenFormat();

	// Note: this reads line yEnd * 2 of _renderBuf!
	for (uint y = yStart; y < yEnd; ++y) {
		ColorType *buf = &_renderBuf[y * 2 * kRenderBufWidth];
		for (uint x = 0; x < kRenderBufWidth; ++x) {
			const ColorType color1 = buf[x];
			const ColorType color2 = buf[2 * kRenderBufWidth + x];

			uint8 r1, g1, b1, r2, g2, b2;

			rgbFormat.colorToRGB(color1, r1, g1, b1);
			rgbFormat.colorToRGB(color2, r2, g2, b2);

			const uint8 r3 = BlendType::blend(r1, r2);
			const uint8 g3 = BlendType::blend(g1, g2);
			const uint8 b3 = BlendType::blend(b1, b2);

			buf[kRenderBufWidth + x] = rgbFormat.RGBToColor(r3, g3, b3);
		}
	}
}

template<typename ColorType, typename GfxWriter, typename TextWriter>
void DisplayImpl_A2<ColorType, GfxWriter, TextWriter>::renderText() {
	if (_mode == kModeGraphics)
		return;

	_blink = (g_system->getMillis() / 270) & 1;

	if (_mode == kModeMixed && _enableColor && !_enableMonoText)
		render<TextReader>(_writerColor);
	else
		render<TextReader>(_writerMono);
}

template<typename ColorType, typename GfxWriter, typename TextWriter>
void DisplayImpl_A2<ColorType, GfxWriter, TextWriter>::renderGraphics() {
	if (_mode == kModeText)
		return;

	render<GfxReader>(_writerColor);
}

Display_A2::Display_A2() :
		_frameBuf(nullptr),
		_showCursor(false),
		_enableColor(false),
		_enableScanlines(false),
		_enableMonoText(false),
		_blink(false) { }

Display_A2::~Display_A2() {
	delete[] _frameBuf;
}

void Display_A2::init() {
	createTextBuffer(Display_A2::kTextWidth, Display_A2::kTextHeight);

	_frameBuf = new byte[Display_A2::kGfxSize]();

	_enableColor = ConfMan.getBool("color");
	_enableScanlines = ConfMan.getBool("scanlines");
	_enableMonoText = ConfMan.getBool("monotext");
}

void Display_A2::loadFrameBuffer(Common::ReadStream &stream, byte *dst) const {
	for (uint j = 0; j < 8; ++j) {
		for (uint i = 0; i < 8; ++i) {
			stream.read(dst, Display_A2::kGfxPitch);
			dst += Display_A2::kGfxPitch * 64;
			stream.read(dst, Display_A2::kGfxPitch);
			dst += Display_A2::kGfxPitch * 64;
			stream.read(dst, Display_A2::kGfxPitch);
			stream.readUint32LE();
			stream.readUint32LE();
			dst -= Display_A2::kGfxPitch * 120;
		}
		dst -= Display_A2::kGfxPitch * 63;
	}

	if (stream.eos() || stream.err())
		error("Failed to read frame buffer");
}

void Display_A2::loadFrameBuffer(Common::ReadStream &stream) {
	loadFrameBuffer(stream, _frameBuf);
}

void Display_A2::putPixel(const Common::Point &p, byte color) {
	const byte offset = p.x / 7;
	byte mask = 0x80 | (1 << (p.x % 7));

	// Since white and black are in both palettes, we leave
	// the palette bit alone
	if ((color & 0x7f) == 0x7f || (color & 0x7f) == 0)
		mask &= 0x7f;

	// Adjust colors starting with bits '01' or '10' for
	// odd offsets
	if (offset & 1) {
		byte c = color << 1;
		if (c >= 0x40 && c < 0xc0)
			color ^= 0x7f;
	}

	writeFrameBuffer(p, color, mask);
}

void Display_A2::setPixelByte(const Common::Point &p, byte color) {
	assert(p.x >= 0 && p.x < Display_A2::kGfxWidth && p.y >= 0 && p.y < Display_A2::kGfxHeight);

	_frameBuf[p.y * Display_A2::kGfxPitch + p.x / 7] = color;
}

void Display_A2::setPixelBit(const Common::Point &p, byte color) {
	writeFrameBuffer(p, color, 1 << (p.x % 7));
}

void Display_A2::setPixelPalette(const Common::Point &p, byte color) {
	writeFrameBuffer(p, color, 0x80);
}

byte Display_A2::getPixelByte(const Common::Point &p) const {
	assert(p.x >= 0 && p.x < Display_A2::kGfxWidth && p.y >= 0 && p.y < Display_A2::kGfxHeight);

	return _frameBuf[p.y * Display_A2::kGfxPitch + p.x / 7];
}

bool Display_A2::getPixelBit(const Common::Point &p) const {
	assert(p.x >= 0 && p.x < Display_A2::kGfxWidth && p.y >= 0 && p.y < Display_A2::kGfxHeight);

	const byte *b = _frameBuf + p.y * Display_A2::kGfxPitch + p.x / 7;
	return *b & (1 << (p.x % 7));
}

void Display_A2::clear(byte color) {
	byte val = 0;

	const byte c = color << 1;
	if (c >= 0x40 && c < 0xc0)
		val = 0x7f;

	for (uint i = 0; i < Display_A2::kGfxSize; ++i) {
		_frameBuf[i] = color;
		color ^= val;
	}
}

// FIXME: This does not currently update the surfaces
void Display_A2::printChar(char c) {
	if (c == Display_A2::asciiToNative('\r'))
		_cursorPos = (_cursorPos / Display_A2::kTextWidth + 1) * Display_A2::kTextWidth;
	else if (c == Display_A2::asciiToNative('\a')) {
		renderText();
		static_cast<AdlEngine *>(g_engine)->bell();
	} else if ((byte)c < 0x80 || (byte)c >= 0xa0) {
		setCharAtCursor(c);
		++_cursorPos;
	}

	if (_cursorPos == Display_A2::kTextWidth * Display_A2::kTextHeight)
		scrollUp();
}

void Display_A2::showCursor(bool enable) {
	_showCursor = enable;
}

void Display_A2::writeFrameBuffer(const Common::Point &p, byte color, byte mask) {
	assert(p.x >= 0 && p.x < Display_A2::kGfxWidth && p.y >= 0 && p.y < Display_A2::kGfxHeight);

	byte *b = _frameBuf + p.y * Display_A2::kGfxPitch + p.x / 7;
	color ^= *b;
	color &= mask;
	*b ^= color;
}

template<typename ColorType>
static Display_A2 *Display_A2_create_helper() {
	const bool ntsc = ConfMan.getBool("ntsc");
	const bool color = ConfMan.getBool("color");
	const bool monotext = ConfMan.getBool("monotext");

	typedef PixelWriterMono<ColorType, 0xff, 0xff, 0xff> PixelWriterMonoWhite;
	typedef PixelWriterMono<ColorType, 0x00, 0xc0, 0x00> PixelWriterMonoGreen;

	if (ntsc) {
		if (color) {
			if (monotext)
				return new DisplayImpl_A2<ColorType, PixelWriterColorNTSC<ColorType>, PixelWriterMonoWhite>;
			else
				return new DisplayImpl_A2<ColorType, PixelWriterColorNTSC<ColorType>, PixelWriterMonoNTSC<ColorType> >;
		} else {
			if (monotext)
				return new DisplayImpl_A2<ColorType, PixelWriterMonoNTSC<ColorType>, PixelWriterMonoWhite>;
			else
				return new DisplayImpl_A2<ColorType, PixelWriterMonoNTSC<ColorType>, PixelWriterMonoNTSC<ColorType> >;
		}
	}

	if (color)
		return new DisplayImpl_A2<ColorType, PixelWriterColor<ColorType>, PixelWriterMonoWhite>;
	else
		return new DisplayImpl_A2<ColorType, PixelWriterMonoGreen, PixelWriterMonoGreen>;
}

Display_A2 *Display_A2_create() {
	initGraphics(Display_A2::kGfxWidth * 2, Display_A2::kGfxHeight * 2, nullptr);
	debugN(1, "Initialized graphics with format: %s", g_system->getScreenFormat().toString().c_str());

	const uint bpp = g_system->getScreenFormat().bytesPerPixel;

	if (bpp == 4)
		return Display_A2_create_helper<uint32>();
	else if (bpp == 2)
		return Display_A2_create_helper<uint16>();
	else
		return nullptr;
}

} // End of namespace Adl
