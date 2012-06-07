/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "graphics/scaler/hq.h"
#include "graphics/scaler.h"

// RGB-to-YUV lookup table
extern "C" {

#ifdef USE_NASM
// NOTE: if your compiler uses different mangled names, add another
//       condition here

#if !defined(_WIN32) && !defined(MACOSX) && !defined(__OS2__)
#define RGBtoYUV _RGBtoYUV
#define hqx_highbits _hqx_highbits
#define hqx_lowbits _hqx_lowbits
#define hqx_low2bits _hqx_low2bits
#define hqx_low3bits _hqx_low3bits
#define hqx_greenMask _hqx_greenMask
#define hqx_redBlueMask _hqx_redBlueMask
#define hqx_green_redBlue_Mask _hqx_green_redBlue_Mask
#endif

uint32 hqx_highbits = 0xF7DEF7DE;
uint32 hqx_lowbits = 0x0821;
uint32 hqx_low2bits = 0x0C63;
uint32 hqx_low3bits = 0x1CE7;
uint32 hqx_greenMask = 0;
uint32 hqx_redBlueMask = 0;
uint32 hqx_green_redBlue_Mask = 0;
#endif

/**
 * 16bit RGB to YUV conversion table. This table is setup by InitLUT().
 * Used by the hq scaler family.
 *
 * FIXME/TODO: The RGBtoYUV table sucks up 256 KB. This is bad.
 * In addition we never free it...
 *
 * Note: a memory lookup table is *not* necessarily faster than computing
 * these things on the fly, because of its size. The table together with
 * the code, plus the input/output GFX data, may not fit in the cache on some
 * systems, so main memory has to be accessed, which is about the worst thing
 * that can happen to code which tries to be fast...
 *
 * So we should think about ways to get this smaller / removed. Maybe we can
 * use the same technique employed by our MPEG code to reduce the size of the
 * lookup table at the cost of some additional computations?
 *
 * Of course, the above is largely a conjecture, and the actual speed
 * differences are likely to vary a lot between different architectures and
 * CPUs.
 */
uint32 *RGBtoYUV = 0;
}

void InitLUT(Graphics::PixelFormat format) {
	uint8 r, g, b;
	int Y, u, v;

	assert(format.bytesPerPixel == 2);

	// Allocate the YUV/LUT buffers on the fly if needed.
	if (RGBtoYUV == 0)
		RGBtoYUV = (uint32 *)malloc(65536 * sizeof(uint32));

	if (!RGBtoYUV)
		error("[InitLUT] Cannot allocate memory for YUV/LUT buffers");

	for (int color = 0; color < 65536; ++color) {
		format.colorToRGB(color, r, g, b);
		Y = (r + g + b) >> 2;
		u = 128 + ((r - b) >> 2);
		v = 128 + ((-r + 2 * g - b) >> 3);
		RGBtoYUV[color] = (Y << 16) | (u << 8) | v;
	}

#ifdef USE_NASM
	hqx_lowbits  = (1 << format.rShift) | (1 << format.gShift) | (1 << format.bShift),
	hqx_low2bits = (3 << format.rShift) | (3 << format.gShift) | (3 << format.bShift),
	hqx_low3bits = (7 << format.rShift) | (7 << format.gShift) | (7 << format.bShift),

	hqx_highbits = format.RGBToColor(255,255,255) ^ hqx_lowbits;

	// FIXME: The following code only does the right thing
	// if the color order is RGB or BGR, i.e., green is in the middle.
	hqx_greenMask = format.RGBToColor(0,255,0);
	hqx_redBlueMask = format.RGBToColor(255,0,255);

	hqx_green_redBlue_Mask = (hqx_greenMask << 16) | hqx_redBlueMask;
#endif
}

HQPlugin::HQPlugin() {
	_factor = 2;
	_factors.push_back(2);
	_factors.push_back(3);
}

void HQPlugin::initialize(Graphics::PixelFormat format) {
	InitLUT(format);
	_format = format;
}

void HQPlugin::deinitialize() {
	free(RGBtoYUV);
	RGBtoYUV = 0;
}

void HQPlugin::scale(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	if (!_doScale) {
		scale1x(srcPtr, srcPitch, dstPtr, dstPitch, width, height, _format.bytesPerPixel);
		return;
	}
	switch (_factor) {
	case 2:
		HQ2x(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		break;
	case 3:
		HQ3x(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		break;
	}
}

uint HQPlugin::increaseFactor() {
	if (_factor < 3)
		++_factor;
	return _factor;
}

uint HQPlugin::decreaseFactor() {
	if (_factor > 2)
		--_factor;
	return _factor;
}

const char *HQPlugin::getName() const {
	return "hq";
}

const char *HQPlugin::getPrettyName() const {
	return "HQ";
}

REGISTER_PLUGIN_STATIC(HQ, PLUGIN_TYPE_SCALER, HQPlugin);
