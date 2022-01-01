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
 */

#include "graphics/scaler/hq.h"
#include "graphics/scaler.h"
#include "graphics/scaler/intern.h"

// RGB-to-YUV lookup table

#ifdef USE_NASM
// NOTE: if your compiler uses different mangled names, add another
//       condition here
struct hqx_parameters {
	uint32 *RGBtoYUV;
	uint32 highbits;
	uint32 lowbits;
	uint32 low2bits;
	uint32 low3bits;
	uint32 greenMask;
	uint32 redBlueMask;
	uint32 green_redBlue_Mask;
};

// Assembly versions of HQ2x and HQ3x

extern "C" {

#if !defined(_WIN32) && !defined(MACOSX) && !defined(__OS2__)
#define hq2x_16 _hq2x_16
#define hq3x_16 _hq3x_16
#endif

void hq2x_16(const byte *, byte *, uint32, uint32, uint32, uint32, struct hqx_parameters *);
void hq3x_16(const byte *, byte *, uint32, uint32, uint32, uint32, struct hqx_parameters *);

}
#endif

/**
 * 16bit RGB to YUV conversion table. This table is setup by InitLUT().
 * Used by the hq scaler family.
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
void HQScaler::initLUT(Graphics::PixelFormat format) {
	uint8 r, g, b;
	int Y, u, v;

	assert(format.bytesPerPixel == 2);

	// Allocate the YUV/LUT buffers on the fly if needed.
	if (!_RGBtoYUV) {
		_RGBtoYUV = new uint32[65536];
	}
	for (int color = 0; color < 65536; ++color) {
		format.colorToRGB(color, r, g, b);
		Y = (r + g + b) >> 2;
		u = 128 + ((r - b) >> 2);
		v = 128 + ((-r + 2 * g - b) >> 3);
		_RGBtoYUV[color] = (Y << 16) | (u << 8) | v;
	}

#ifdef USE_NASM
	if (!_hqx_params) {
		_hqx_params = new hqx_parameters;
	}
	_hqx_params->lowbits  = (1 << format.rShift) | (1 << format.gShift) | (1 << format.bShift),
	_hqx_params->low2bits = (3 << format.rShift) | (3 << format.gShift) | (3 << format.bShift),
	_hqx_params->low3bits = (7 << format.rShift) | (7 << format.gShift) | (7 << format.bShift),

	_hqx_params->highbits = format.RGBToColor(255,255,255) ^ _hqx_params->lowbits;

	// FIXME: The following code only does the right thing
	// if the color order is RGB or BGR, i.e., green is in the middle.
	_hqx_params->greenMask = format.RGBToColor(0,255,0);
	_hqx_params->redBlueMask = format.RGBToColor(255,0,255);

	_hqx_params->green_redBlue_Mask = (_hqx_params->greenMask << 16) | _hqx_params->redBlueMask;

	_hqx_params->RGBtoYUV = _RGBtoYUV;
#endif
}

#define interpolate_1_1(a,b)         (ColorMask::kBytesPerPixel == 2 ? interpolate16_1_1<ColorMask>(a,b) : interpolate32_1_1<ColorMask>(a,b))
#define interpolate_3_1(a,b)         (ColorMask::kBytesPerPixel == 2 ? interpolate16_3_1<ColorMask>(a,b) : interpolate32_3_1<ColorMask>(a,b))
#define interpolate_7_1(a,b)         (ColorMask::kBytesPerPixel == 2 ? interpolate16_7_1<ColorMask>(a,b) : interpolate32_7_1<ColorMask>(a,b))
#define interpolate_2_1_1(a,b,c)     (ColorMask::kBytesPerPixel == 2 ? interpolate16_2_1_1<ColorMask>(a,b,c) : interpolate32_2_1_1<ColorMask>(a,b,c))
#define interpolate_2_3_3(a,b,c)     (ColorMask::kBytesPerPixel == 2 ? interpolate16_2_3_3<ColorMask>(a,b,c) : interpolate32_2_3_3<ColorMask>(a,b,c))
#define interpolate_2_7_7(a,b,c)     (ColorMask::kBytesPerPixel == 2 ? interpolate16_2_7_7<ColorMask>(a,b,c) : interpolate32_2_7_7<ColorMask>(a,b,c))
#define interpolate_5_2_1(a,b,c)     (ColorMask::kBytesPerPixel == 2 ? interpolate16_5_2_1<ColorMask>(a,b,c) : interpolate32_5_2_1<ColorMask>(a,b,c))
#define interpolate_6_1_1(a,b,c)     (ColorMask::kBytesPerPixel == 2 ? interpolate16_6_1_1<ColorMask>(a,b,c) : interpolate32_6_1_1<ColorMask>(a,b,c))
#define interpolate_14_1_1(a,b,c)     (ColorMask::kBytesPerPixel == 2 ? interpolate16_14_1_1<ColorMask>(a,b,c) : interpolate32_14_1_1<ColorMask>(a,b,c))
#define interpolate_1_1_1_1(a,b,c,d) (ColorMask::kBytesPerPixel == 2 ? interpolate16_1_1_1_1<ColorMask>(a,b,c,d) : interpolate32_1_1_1_1<ColorMask>(a,b,c,d))

#define PIXEL00_0	*(q) = w5;
#define PIXEL00_10	*(q) = interpolate_3_1(w5, w1);
#define PIXEL00_11	*(q) = interpolate_3_1(w5, w4);
#define PIXEL00_12	*(q) = interpolate_3_1(w5, w2);
#define PIXEL00_20	*(q) = interpolate_2_1_1(w5, w4, w2);
#define PIXEL00_21	*(q) = interpolate_2_1_1(w5, w1, w2);
#define PIXEL00_22	*(q) = interpolate_2_1_1(w5, w1, w4);
#define PIXEL00_60	*(q) = interpolate_5_2_1(w5, w2, w4);
#define PIXEL00_61	*(q) = interpolate_5_2_1(w5, w4, w2);
#define PIXEL00_70	*(q) = interpolate_6_1_1(w5, w4, w2);
#define PIXEL00_90	*(q) = interpolate_2_3_3(w5, w4, w2);
#define PIXEL00_100	*(q) = interpolate_14_1_1(w5, w4, w2);

#define PIXEL01_0	*(q+1) = w5;
#define PIXEL01_10	*(q+1) = interpolate_3_1(w5, w3);
#define PIXEL01_11	*(q+1) = interpolate_3_1(w5, w2);
#define PIXEL01_12	*(q+1) = interpolate_3_1(w5, w6);
#define PIXEL01_20	*(q+1) = interpolate_2_1_1(w5, w2, w6);
#define PIXEL01_21	*(q+1) = interpolate_2_1_1(w5, w3, w6);
#define PIXEL01_22	*(q+1) = interpolate_2_1_1(w5, w3, w2);
#define PIXEL01_60	*(q+1) = interpolate_5_2_1(w5, w6, w2);
#define PIXEL01_61	*(q+1) = interpolate_5_2_1(w5, w2, w6);
#define PIXEL01_70	*(q+1) = interpolate_6_1_1(w5, w2, w6);
#define PIXEL01_90	*(q+1) = interpolate_2_3_3(w5, w2, w6);
#define PIXEL01_100	*(q+1) = interpolate_14_1_1(w5, w2, w6);

#define PIXEL10_0	*(q+nextlineDst) = w5;
#define PIXEL10_10	*(q+nextlineDst) = interpolate_3_1(w5, w7);
#define PIXEL10_11	*(q+nextlineDst) = interpolate_3_1(w5, w8);
#define PIXEL10_12	*(q+nextlineDst) = interpolate_3_1(w5, w4);
#define PIXEL10_20	*(q+nextlineDst) = interpolate_2_1_1(w5, w8, w4);
#define PIXEL10_21	*(q+nextlineDst) = interpolate_2_1_1(w5, w7, w4);
#define PIXEL10_22	*(q+nextlineDst) = interpolate_2_1_1(w5, w7, w8);
#define PIXEL10_60	*(q+nextlineDst) = interpolate_5_2_1(w5, w4, w8);
#define PIXEL10_61	*(q+nextlineDst) = interpolate_5_2_1(w5, w8, w4);
#define PIXEL10_70	*(q+nextlineDst) = interpolate_6_1_1(w5, w8, w4);
#define PIXEL10_90	*(q+nextlineDst) = interpolate_2_3_3(w5, w8, w4);
#define PIXEL10_100	*(q+nextlineDst) = interpolate_14_1_1(w5, w8, w4);

#define PIXEL11_0	*(q+1+nextlineDst) = w5;
#define PIXEL11_10	*(q+1+nextlineDst) = interpolate_3_1(w5, w9);
#define PIXEL11_11	*(q+1+nextlineDst) = interpolate_3_1(w5, w6);
#define PIXEL11_12	*(q+1+nextlineDst) = interpolate_3_1(w5, w8);
#define PIXEL11_20	*(q+1+nextlineDst) = interpolate_2_1_1(w5, w6, w8);
#define PIXEL11_21	*(q+1+nextlineDst) = interpolate_2_1_1(w5, w9, w8);
#define PIXEL11_22	*(q+1+nextlineDst) = interpolate_2_1_1(w5, w9, w6);
#define PIXEL11_60	*(q+1+nextlineDst) = interpolate_5_2_1(w5, w8, w6);
#define PIXEL11_61	*(q+1+nextlineDst) = interpolate_5_2_1(w5, w6, w8);
#define PIXEL11_70	*(q+1+nextlineDst) = interpolate_6_1_1(w5, w6, w8);
#define PIXEL11_90	*(q+1+nextlineDst) = interpolate_2_3_3(w5, w6, w8);
#define PIXEL11_100	*(q+1+nextlineDst) = interpolate_14_1_1(w5, w6, w8);

#define YUV(x)	(sizeof(Pixel) == 2 ? RGBtoYUV[w ## x] : ConvertYUV<ColorMask>(w ## x, RGBtoYUV))

/**
 * Convert 32 bit RGB values to Yuv
 */
template<typename ColorMask>
static inline uint32 ConvertYUV(uint32 x, const uint32 *RGBtoYUV) {
	int r, g, b;

	r = (ColorMask::kRedMask & (ColorMask::kRedMask << (8 - Graphics::ColorMasks<565>::kRedBits)) & x)
		>> (ColorMask::kRedShift + (8 - Graphics::ColorMasks<565>::kRedBits) - Graphics::ColorMasks<565>::kRedShift);
	g = (ColorMask::kGreenMask & (ColorMask::kGreenMask << (8 - Graphics::ColorMasks<565>::kGreenBits)) & x)
		>> (ColorMask::kGreenShift + (8 - Graphics::ColorMasks<565>::kGreenBits) - Graphics::ColorMasks<565>::kGreenShift);
	b = (ColorMask::kBlueMask & (ColorMask::kBlueMask << (8 - Graphics::ColorMasks<565>::kBlueBits)) & x)
		>> (ColorMask::kBlueShift + (8 - Graphics::ColorMasks<565>::kBlueBits) - Graphics::ColorMasks<565>::kBlueShift);

	// lookup in 16 bit table
	return RGBtoYUV[r | g | b];
}

/*
 * The HQ2x high quality 2x graphics filter.
 * Original author Maxim Stepin (see http://www.hiend3d.com/hq2x.html).
 * Adapted for ScummVM to 16 bit output and optimized by Max Horn.
 */
template<typename ColorMask>
static void HQ2x_implementation(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, const uint32 *RGBtoYUV) {
	typedef typename ColorMask::PixelType Pixel;

	int w1, w2, w3, w4, w5, w6, w7, w8, w9;

	const uint32 nextlineSrc = srcPitch / sizeof(Pixel);
	const Pixel *p = (const Pixel *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(Pixel);
	Pixel *q = (Pixel *)dstPtr;

	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w1 | w2 | w3 |
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w4 | w5 | w6 |
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w7 | w8 | w9 |
	//	 +----+----+----+

	while (height--) {
		w1 = *(p - 1 - nextlineSrc);
		w4 = *(p - 1);
		w7 = *(p - 1 + nextlineSrc);

		w2 = *(p - nextlineSrc);
		w5 = *(p);
		w8 = *(p + nextlineSrc);

		int tmpWidth = width;
		while (tmpWidth--) {
			p++;

			w3 = *(p - nextlineSrc);
			w6 = *(p);
			w9 = *(p + nextlineSrc);

			int pattern = 0;
			const int yuv5 = YUV(5);
			if (w5 != w1 && diffYUV(yuv5, YUV(1))) pattern |= 0x0001;
			if (w5 != w2 && diffYUV(yuv5, YUV(2))) pattern |= 0x0002;
			if (w5 != w3 && diffYUV(yuv5, YUV(3))) pattern |= 0x0004;
			if (w5 != w4 && diffYUV(yuv5, YUV(4))) pattern |= 0x0008;
			if (w5 != w6 && diffYUV(yuv5, YUV(6))) pattern |= 0x0010;
			if (w5 != w7 && diffYUV(yuv5, YUV(7))) pattern |= 0x0020;
			if (w5 != w8 && diffYUV(yuv5, YUV(8))) pattern |= 0x0040;
			if (w5 != w9 && diffYUV(yuv5, YUV(9))) pattern |= 0x0080;

			switch (pattern) {
			case 0:
			case 1:
			case 4:
			case 32:
			case 128:
			case 5:
			case 132:
			case 160:
			case 33:
			case 129:
			case 36:
			case 133:
			case 164:
			case 161:
			case 37:
			case 165:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_20
				PIXEL11_20
				break;
			case 2:
			case 34:
			case 130:
			case 162:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_20
				PIXEL11_20
				break;
			case 16:
			case 17:
			case 48:
			case 49:
				PIXEL00_20
				PIXEL01_22
				PIXEL10_20
				PIXEL11_21
				break;
			case 64:
			case 65:
			case 68:
			case 69:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_21
				PIXEL11_22
				break;
			case 8:
			case 12:
			case 136:
			case 140:
				PIXEL00_21
				PIXEL01_20
				PIXEL10_22
				PIXEL11_20
				break;
			case 3:
			case 35:
			case 131:
			case 163:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_20
				PIXEL11_20
				break;
			case 6:
			case 38:
			case 134:
			case 166:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
			case 20:
			case 21:
			case 52:
			case 53:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_21
				break;
			case 144:
			case 145:
			case 176:
			case 177:
				PIXEL00_20
				PIXEL01_22
				PIXEL10_20
				PIXEL11_12
				break;
			case 192:
			case 193:
			case 196:
			case 197:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_21
				PIXEL11_11
				break;
			case 96:
			case 97:
			case 100:
			case 101:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_22
				break;
			case 40:
			case 44:
			case 168:
			case 172:
				PIXEL00_21
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
			case 9:
			case 13:
			case 137:
			case 141:
				PIXEL00_12
				PIXEL01_20
				PIXEL10_22
				PIXEL11_20
				break;
			case 18:
			case 50:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_20
				}
				PIXEL10_20
				PIXEL11_21
				break;
			case 80:
			case 81:
				PIXEL00_20
				PIXEL01_22
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_20
				}
				break;
			case 72:
			case 76:
				PIXEL00_21
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 10:
			case 138:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_22
				PIXEL11_20
				break;
			case 66:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_21
				PIXEL11_22
				break;
			case 24:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_22
				PIXEL11_21
				break;
			case 7:
			case 39:
			case 135:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
			case 148:
			case 149:
			case 180:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_12
				break;
			case 224:
			case 228:
			case 225:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_11
				break;
			case 41:
			case 169:
			case 45:
				PIXEL00_12
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
			case 22:
			case 54:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_20
				PIXEL11_21
				break;
			case 208:
			case 209:
				PIXEL00_20
				PIXEL01_22
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 104:
			case 108:
				PIXEL00_21
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 11:
			case 139:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_22
				PIXEL11_20
				break;
			case 19:
			case 51:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL00_11
					PIXEL01_10
				} else {
					PIXEL00_60
					PIXEL01_90
				}
				PIXEL10_20
				PIXEL11_21
				break;
			case 146:
			case 178:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
					PIXEL11_12
				} else {
					PIXEL01_90
					PIXEL11_61
				}
				PIXEL10_20
				break;
			case 84:
			case 85:
				PIXEL00_20
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL01_11
					PIXEL11_10
				} else {
					PIXEL01_60
					PIXEL11_90
				}
				PIXEL10_21
				break;
			case 112:
			case 113:
				PIXEL00_20
				PIXEL01_22
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL10_12
					PIXEL11_10
				} else {
					PIXEL10_61
					PIXEL11_90
				}
				break;
			case 200:
			case 204:
				PIXEL00_21
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
					PIXEL11_11
				} else {
					PIXEL10_90
					PIXEL11_60
				}
				break;
			case 73:
			case 77:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL00_12
					PIXEL10_10
				} else {
					PIXEL00_61
					PIXEL10_90
				}
				PIXEL01_20
				PIXEL11_22
				break;
			case 42:
			case 170:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
					PIXEL10_11
				} else {
					PIXEL00_90
					PIXEL10_60
				}
				PIXEL01_21
				PIXEL11_20
				break;
			case 14:
			case 142:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
					PIXEL01_12
				} else {
					PIXEL00_90
					PIXEL01_61
				}
				PIXEL10_22
				PIXEL11_20
				break;
			case 67:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_21
				PIXEL11_22
				break;
			case 70:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_21
				PIXEL11_22
				break;
			case 28:
				PIXEL00_21
				PIXEL01_11
				PIXEL10_22
				PIXEL11_21
				break;
			case 152:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_22
				PIXEL11_12
				break;
			case 194:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_21
				PIXEL11_11
				break;
			case 98:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_12
				PIXEL11_22
				break;
			case 56:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_11
				PIXEL11_21
				break;
			case 25:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_22
				PIXEL11_21
				break;
			case 26:
			case 31:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_22
				PIXEL11_21
				break;
			case 82:
			case 214:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 88:
			case 248:
				PIXEL00_21
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 74:
			case 107:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 27:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_22
				PIXEL11_21
				break;
			case 86:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_21
				PIXEL11_10
				break;
			case 216:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 106:
				PIXEL00_10
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 30:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_22
				PIXEL11_21
				break;
			case 210:
				PIXEL00_22
				PIXEL01_10
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 120:
				PIXEL00_21
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 75:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_10
				PIXEL11_22
				break;
			case 29:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_22
				PIXEL11_21
				break;
			case 198:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_21
				PIXEL11_11
				break;
			case 184:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_11
				PIXEL11_12
				break;
			case 99:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_12
				PIXEL11_22
				break;
			case 57:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_11
				PIXEL11_21
				break;
			case 71:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_21
				PIXEL11_22
				break;
			case 156:
				PIXEL00_21
				PIXEL01_11
				PIXEL10_22
				PIXEL11_12
				break;
			case 226:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_12
				PIXEL11_11
				break;
			case 60:
				PIXEL00_21
				PIXEL01_11
				PIXEL10_11
				PIXEL11_21
				break;
			case 195:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_21
				PIXEL11_11
				break;
			case 102:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_12
				PIXEL11_22
				break;
			case 153:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_22
				PIXEL11_12
				break;
			case 58:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_11
				PIXEL11_21
				break;
			case 83:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 92:
				PIXEL00_21
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 202:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_11
				break;
			case 78:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_22
				break;
			case 154:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_22
				PIXEL11_12
				break;
			case 114:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 89:
				PIXEL00_12
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 90:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 55:
			case 23:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL00_11
					PIXEL01_0
				} else {
					PIXEL00_60
					PIXEL01_90
				}
				PIXEL10_20
				PIXEL11_21
				break;
			case 182:
			case 150:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
					PIXEL11_12
				} else {
					PIXEL01_90
					PIXEL11_61
				}
				PIXEL10_20
				break;
			case 213:
			case 212:
				PIXEL00_20
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL01_11
					PIXEL11_0
				} else {
					PIXEL01_60
					PIXEL11_90
				}
				PIXEL10_21
				break;
			case 241:
			case 240:
				PIXEL00_20
				PIXEL01_22
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL10_12
					PIXEL11_0
				} else {
					PIXEL10_61
					PIXEL11_90
				}
				break;
			case 236:
			case 232:
				PIXEL00_21
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
					PIXEL11_11
				} else {
					PIXEL10_90
					PIXEL11_60
				}
				break;
			case 109:
			case 105:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL00_12
					PIXEL10_0
				} else {
					PIXEL00_61
					PIXEL10_90
				}
				PIXEL01_20
				PIXEL11_22
				break;
			case 171:
			case 43:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
					PIXEL10_11
				} else {
					PIXEL00_90
					PIXEL10_60
				}
				PIXEL01_21
				PIXEL11_20
				break;
			case 143:
			case 15:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
					PIXEL01_12
				} else {
					PIXEL00_90
					PIXEL01_61
				}
				PIXEL10_22
				PIXEL11_20
				break;
			case 124:
				PIXEL00_21
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 203:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_10
				PIXEL11_11
				break;
			case 62:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_11
				PIXEL11_21
				break;
			case 211:
				PIXEL00_11
				PIXEL01_10
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 118:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_12
				PIXEL11_10
				break;
			case 217:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 110:
				PIXEL00_10
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 155:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_22
				PIXEL11_12
				break;
			case 188:
				PIXEL00_21
				PIXEL01_11
				PIXEL10_11
				PIXEL11_12
				break;
			case 185:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_11
				PIXEL11_12
				break;
			case 61:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_11
				PIXEL11_21
				break;
			case 157:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_22
				PIXEL11_12
				break;
			case 103:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_12
				PIXEL11_22
				break;
			case 227:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_12
				PIXEL11_11
				break;
			case 230:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_12
				PIXEL11_11
				break;
			case 199:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_21
				PIXEL11_11
				break;
			case 220:
				PIXEL00_21
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 158:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_22
				PIXEL11_12
				break;
			case 234:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_11
				break;
			case 242:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 59:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_11
				PIXEL11_21
				break;
			case 121:
				PIXEL00_12
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 87:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 79:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_22
				break;
			case 122:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 94:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 218:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 91:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 229:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_11
				break;
			case 167:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
			case 173:
				PIXEL00_12
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
			case 181:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_12
				break;
			case 186:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_11
				PIXEL11_12
				break;
			case 115:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 93:
				PIXEL00_12
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 206:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_11
				break;
			case 205:
			case 201:
				PIXEL00_12
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_11
				break;
			case 174:
			case 46:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_12
				PIXEL10_11
				PIXEL11_20
				break;
			case 179:
			case 147:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_20
				PIXEL11_12
				break;
			case 117:
			case 116:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 189:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_11
				PIXEL11_12
				break;
			case 231:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_12
				PIXEL11_11
				break;
			case 126:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 219:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 125:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL00_12
					PIXEL10_0
				} else {
					PIXEL00_61
					PIXEL10_90
				}
				PIXEL01_11
				PIXEL11_10
				break;
			case 221:
				PIXEL00_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL01_11
					PIXEL11_0
				} else {
					PIXEL01_60
					PIXEL11_90
				}
				PIXEL10_10
				break;
			case 207:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
					PIXEL01_12
				} else {
					PIXEL00_90
					PIXEL01_61
				}
				PIXEL10_10
				PIXEL11_11
				break;
			case 238:
				PIXEL00_10
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
					PIXEL11_11
				} else {
					PIXEL10_90
					PIXEL11_60
				}
				break;
			case 190:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
					PIXEL11_12
				} else {
					PIXEL01_90
					PIXEL11_61
				}
				PIXEL10_11
				break;
			case 187:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
					PIXEL10_11
				} else {
					PIXEL00_90
					PIXEL10_60
				}
				PIXEL01_10
				PIXEL11_12
				break;
			case 243:
				PIXEL00_11
				PIXEL01_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL10_12
					PIXEL11_0
				} else {
					PIXEL10_61
					PIXEL11_90
				}
				break;
			case 119:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL00_11
					PIXEL01_0
				} else {
					PIXEL00_60
					PIXEL01_90
				}
				PIXEL10_12
				PIXEL11_10
				break;
			case 237:
			case 233:
				PIXEL00_12
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				PIXEL11_11
				break;
			case 175:
			case 47:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				PIXEL01_12
				PIXEL10_11
				PIXEL11_20
				break;
			case 183:
			case 151:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_20
				PIXEL11_12
				break;
			case 245:
			case 244:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 250:
				PIXEL00_10
				PIXEL01_10
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 123:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 95:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				PIXEL11_10
				break;
			case 222:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 252:
				PIXEL00_21
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 249:
				PIXEL00_12
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 235:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				PIXEL11_11
				break;
			case 111:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 63:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_11
				PIXEL11_21
				break;
			case 159:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_22
				PIXEL11_12
				break;
			case 215:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 246:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 254:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 253:
				PIXEL00_12
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 251:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 239:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				PIXEL11_11
				break;
			case 127:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 191:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_11
				PIXEL11_12
				break;
			case 223:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 247:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 255:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			}

			w1 = w2;
			w4 = w5;
			w7 = w8;

			w2 = w3;
			w5 = w6;
			w8 = w9;

			q += 2;
		}
		p += nextlineSrc - width;
		q += (nextlineDst - width) * 2;
	}
}

#define PIXEL00_1M  *(q) = interpolate_3_1(w5, w1);
#define PIXEL00_1U  *(q) = interpolate_3_1(w5, w2);
#define PIXEL00_1L  *(q) = interpolate_3_1(w5, w4);
#define PIXEL00_2   *(q) = interpolate_2_1_1(w5, w4, w2);
#define PIXEL00_4   *(q) = interpolate_2_7_7(w5, w4, w2);
#define PIXEL00_5   *(q) = interpolate_1_1(w4, w2);
#define PIXEL00_C   *(q) = w5;

#define PIXEL01_1   *(q+1) = interpolate_3_1(w5, w2);
#define PIXEL01_3   *(q+1) = interpolate_7_1(w5, w2);
#define PIXEL01_6   *(q+1) = interpolate_3_1(w2, w5);
#define PIXEL01_C   *(q+1) = w5;

#define PIXEL02_1M  *(q+2) = interpolate_3_1(w5, w3);
#define PIXEL02_1U  *(q+2) = interpolate_3_1(w5, w2);
#define PIXEL02_1R  *(q+2) = interpolate_3_1(w5, w6);
#define PIXEL02_2   *(q+2) = interpolate_2_1_1(w5, w2, w6);
#define PIXEL02_4   *(q+2) = interpolate_2_7_7(w5, w2, w6);
#define PIXEL02_5   *(q+2) = interpolate_1_1(w2, w6);
#define PIXEL02_C   *(q+2) = w5;

#define PIXEL10_1   *(q+nextlineDst) = interpolate_3_1(w5, w4);
#define PIXEL10_3   *(q+nextlineDst) = interpolate_7_1(w5, w4);
#define PIXEL10_6   *(q+nextlineDst) = interpolate_3_1(w4, w5);
#define PIXEL10_C   *(q+nextlineDst) = w5;

#define PIXEL11     *(q+1+nextlineDst) = w5;

#define PIXEL12_1   *(q+2+nextlineDst) = interpolate_3_1(w5, w6);
#define PIXEL12_3   *(q+2+nextlineDst) = interpolate_7_1(w5, w6);
#define PIXEL12_6   *(q+2+nextlineDst) = interpolate_3_1(w6, w5);
#define PIXEL12_C   *(q+2+nextlineDst) = w5;

#define PIXEL20_1M  *(q+nextlineDst2) = interpolate_3_1(w5, w7);
#define PIXEL20_1D  *(q+nextlineDst2) = interpolate_3_1(w5, w8);
#define PIXEL20_1L  *(q+nextlineDst2) = interpolate_3_1(w5, w4);
#define PIXEL20_2   *(q+nextlineDst2) = interpolate_2_1_1(w5, w8, w4);
#define PIXEL20_4   *(q+nextlineDst2) = interpolate_2_7_7(w5, w8, w4);
#define PIXEL20_5   *(q+nextlineDst2) = interpolate_1_1(w8, w4);
#define PIXEL20_C   *(q+nextlineDst2) = w5;

#define PIXEL21_1   *(q+1+nextlineDst2) = interpolate_3_1(w5, w8);
#define PIXEL21_3   *(q+1+nextlineDst2) = interpolate_7_1(w5, w8);
#define PIXEL21_6   *(q+1+nextlineDst2) = interpolate_3_1(w8, w5);
#define PIXEL21_C   *(q+1+nextlineDst2) = w5;

#define PIXEL22_1M  *(q+2+nextlineDst2) = interpolate_3_1(w5, w9);
#define PIXEL22_1D  *(q+2+nextlineDst2) = interpolate_3_1(w5, w8);
#define PIXEL22_1R  *(q+2+nextlineDst2) = interpolate_3_1(w5, w6);
#define PIXEL22_2   *(q+2+nextlineDst2) = interpolate_2_1_1(w5, w6, w8);
#define PIXEL22_4   *(q+2+nextlineDst2) = interpolate_2_7_7(w5, w6, w8);
#define PIXEL22_5   *(q+2+nextlineDst2) = interpolate_1_1(w6, w8);
#define PIXEL22_C   *(q+2+nextlineDst2) = w5;

/*
 * The HQ3x high quality 3x graphics filter.
 * Original author Maxim Stepin (see http://www.hiend3d.com/hq3x.html).
 * Adapted for ScummVM to 16 bit output and optimized by Max Horn.
 */
template<typename ColorMask>
static void HQ3x_implementation(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, const uint32 *RGBtoYUV) {
	typedef typename ColorMask::PixelType Pixel;

	int  w1, w2, w3, w4, w5, w6, w7, w8, w9;

	const uint32 nextlineSrc = srcPitch / sizeof(Pixel);
	const Pixel *p = (const Pixel *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(Pixel);
	const uint32 nextlineDst2 = 2 * nextlineDst;
	Pixel *q = (Pixel *)dstPtr;

	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w1 | w2 | w3 |
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w4 | w5 | w6 |
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w7 | w8 | w9 |
	//	 +----+----+----+

	while (height--) {
		w1 = *(p - 1 - nextlineSrc);
		w4 = *(p - 1);
		w7 = *(p - 1 + nextlineSrc);

		w2 = *(p - nextlineSrc);
		w5 = *(p);
		w8 = *(p + nextlineSrc);

		int tmpWidth = width;
		while (tmpWidth--) {
			p++;

			w3 = *(p - nextlineSrc);
			w6 = *(p);
			w9 = *(p + nextlineSrc);

			int pattern = 0;
			const int yuv5 = YUV(5);
			if (w5 != w1 && diffYUV(yuv5, YUV(1))) pattern |= 0x0001;
			if (w5 != w2 && diffYUV(yuv5, YUV(2))) pattern |= 0x0002;
			if (w5 != w3 && diffYUV(yuv5, YUV(3))) pattern |= 0x0004;
			if (w5 != w4 && diffYUV(yuv5, YUV(4))) pattern |= 0x0008;
			if (w5 != w6 && diffYUV(yuv5, YUV(6))) pattern |= 0x0010;
			if (w5 != w7 && diffYUV(yuv5, YUV(7))) pattern |= 0x0020;
			if (w5 != w8 && diffYUV(yuv5, YUV(8))) pattern |= 0x0040;
			if (w5 != w9 && diffYUV(yuv5, YUV(9))) pattern |= 0x0080;

			switch (pattern) {
			case 0:
			case 1:
			case 4:
			case 32:
			case 128:
			case 5:
			case 132:
			case 160:
			case 33:
			case 129:
			case 36:
			case 133:
			case 164:
			case 161:
			case 37:
			case 165:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 2:
			case 34:
			case 130:
			case 162:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 16:
			case 17:
			case 48:
			case 49:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 64:
			case 65:
			case 68:
			case 69:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 8:
			case 12:
			case 136:
			case 140:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 3:
			case 35:
			case 131:
			case 163:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 6:
			case 38:
			case 134:
			case 166:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 20:
			case 21:
			case 52:
			case 53:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 144:
			case 145:
			case 176:
			case 177:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 192:
			case 193:
			case 196:
			case 197:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 96:
			case 97:
			case 100:
			case 101:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 40:
			case 44:
			case 168:
			case 172:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 9:
			case 13:
			case 137:
			case 141:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 18:
			case 50:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_1M
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 80:
			case 81:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_1M
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 72:
			case 76:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_1M
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 10:
			case 138:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 66:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 24:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 7:
			case 39:
			case 135:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 148:
			case 149:
			case 180:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 224:
			case 228:
			case 225:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 41:
			case 169:
			case 45:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 22:
			case 54:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 208:
			case 209:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 104:
			case 108:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 11:
			case 139:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 19:
			case 51:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1M
					PIXEL12_C
				} else {
					PIXEL00_2
					PIXEL01_6
					PIXEL02_5
					PIXEL12_1
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 146:
			case 178:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_1M
					PIXEL12_C
					PIXEL22_1D
				} else {
					PIXEL01_1
					PIXEL02_5
					PIXEL12_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				break;
			case 84:
			case 85:
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL02_1U
					PIXEL12_C
					PIXEL21_C
					PIXEL22_1M
				} else {
					PIXEL02_2
					PIXEL12_6
					PIXEL21_1
					PIXEL22_5
				}
				PIXEL00_2
				PIXEL01_1
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				break;
			case 112:
			case 113:
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1M
				} else {
					PIXEL12_1
					PIXEL20_2
					PIXEL21_6
					PIXEL22_5
				}
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				break;
			case 200:
			case 204:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1R
				} else {
					PIXEL10_1
					PIXEL20_5
					PIXEL21_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				break;
			case 73:
			case 77:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL00_1U
					PIXEL10_C
					PIXEL20_1M
					PIXEL21_C
				} else {
					PIXEL00_2
					PIXEL10_6
					PIXEL20_5
					PIXEL21_1
				}
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				PIXEL22_1M
				break;
			case 42:
			case 170:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
					PIXEL01_C
					PIXEL10_C
					PIXEL20_1D
				} else {
					PIXEL00_5
					PIXEL01_1
					PIXEL10_6
					PIXEL20_2
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL21_1
				PIXEL22_2
				break;
			case 14:
			case 142:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
				} else {
					PIXEL00_5
					PIXEL01_6
					PIXEL02_2
					PIXEL10_1
				}
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 67:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 70:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 28:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 152:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 194:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 98:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 56:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 25:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 26:
			case 31:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL10_3
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL11
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 82:
			case 214:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
				} else {
					PIXEL01_3
					PIXEL02_4
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 88:
			case 248:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
				} else {
					PIXEL10_3
					PIXEL20_4
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL22_4
				}
				break;
			case 74:
			case 107:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
				} else {
					PIXEL00_4
					PIXEL01_3
				}
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 27:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 86:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 216:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 106:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 30:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 210:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 120:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 75:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 29:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 198:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 184:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 99:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 57:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 71:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 156:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 226:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 60:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 195:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 102:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 153:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 58:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 83:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 92:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 202:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 78:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1M
				break;
			case 154:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 114:
				PIXEL00_1M
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 89:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 90:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 55:
			case 23:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL00_2
					PIXEL01_6
					PIXEL02_5
					PIXEL12_1
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 182:
			case 150:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
					PIXEL22_1D
				} else {
					PIXEL01_1
					PIXEL02_5
					PIXEL12_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				break;
			case 213:
			case 212:
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL02_1U
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL02_2
					PIXEL12_6
					PIXEL21_1
					PIXEL22_5
				}
				PIXEL00_2
				PIXEL01_1
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				break;
			case 241:
			case 240:
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_1
					PIXEL20_2
					PIXEL21_6
					PIXEL22_5
				}
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				break;
			case 236:
			case 232:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
					PIXEL22_1R
				} else {
					PIXEL10_1
					PIXEL20_5
					PIXEL21_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				break;
			case 109:
			case 105:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL00_1U
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL00_2
					PIXEL10_6
					PIXEL20_5
					PIXEL21_1
				}
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				PIXEL22_1M
				break;
			case 171:
			case 43:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
					PIXEL20_1D
				} else {
					PIXEL00_5
					PIXEL01_1
					PIXEL10_6
					PIXEL20_2
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL21_1
				PIXEL22_2
				break;
			case 143:
			case 15:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
				} else {
					PIXEL00_5
					PIXEL01_6
					PIXEL02_2
					PIXEL10_1
				}
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 124:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 203:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 62:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 211:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 118:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 217:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 110:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 155:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 188:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 185:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 61:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 157:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 103:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 227:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 230:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 199:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 220:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 158:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 234:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1R
				break;
			case 242:
				PIXEL00_1M
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1L
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 59:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 121:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 87:
				PIXEL00_1L
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 79:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1R
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1M
				break;
			case 122:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 94:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 218:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 91:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 229:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 167:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 173:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 181:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 186:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 115:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 93:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 206:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 205:
			case 201:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 174:
			case 46:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 179:
			case 147:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 117:
			case 116:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 189:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 231:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 126:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 219:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 125:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL00_1U
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL00_2
					PIXEL10_6
					PIXEL20_5
					PIXEL21_1
				}
				PIXEL01_1
				PIXEL02_1U
				PIXEL11
				PIXEL12_C
				PIXEL22_1M
				break;
			case 221:
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL02_1U
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL02_2
					PIXEL12_6
					PIXEL21_1
					PIXEL22_5
				}
				PIXEL00_1U
				PIXEL01_1
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				break;
			case 207:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
				} else {
					PIXEL00_5
					PIXEL01_6
					PIXEL02_2
					PIXEL10_1
				}
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 238:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
					PIXEL22_1R
				} else {
					PIXEL10_1
					PIXEL20_5
					PIXEL21_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL11
				PIXEL12_1
				break;
			case 190:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
					PIXEL22_1D
				} else {
					PIXEL01_1
					PIXEL02_5
					PIXEL12_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL10_C
				PIXEL11
				PIXEL20_1D
				PIXEL21_1
				break;
			case 187:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
					PIXEL20_1D
				} else {
					PIXEL00_5
					PIXEL01_1
					PIXEL10_6
					PIXEL20_2
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				PIXEL21_1
				PIXEL22_1D
				break;
			case 243:
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_1
					PIXEL20_2
					PIXEL21_6
					PIXEL22_5
				}
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				break;
			case 119:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL00_2
					PIXEL01_6
					PIXEL02_5
					PIXEL12_1
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 237:
			case 233:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 175:
			case 47:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 183:
			case 151:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 245:
			case 244:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 250:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
				} else {
					PIXEL10_3
					PIXEL20_4
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL22_4
				}
				break;
			case 123:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
				} else {
					PIXEL00_4
					PIXEL01_3
				}
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 95:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL10_3
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL11
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 222:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
				} else {
					PIXEL01_3
					PIXEL02_4
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 252:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
				} else {
					PIXEL10_3
					PIXEL20_4
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 249:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL22_4
				}
				break;
			case 235:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
				} else {
					PIXEL00_4
					PIXEL01_3
				}
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 111:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 63:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 159:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL10_3
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 215:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 246:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
				} else {
					PIXEL01_3
					PIXEL02_4
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 254:
				PIXEL00_1M
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
				} else {
					PIXEL01_3
					PIXEL02_4
				}
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
				} else {
					PIXEL10_3
					PIXEL20_4
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_2
				}
				break;
			case 253:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 251:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
				} else {
					PIXEL00_4
					PIXEL01_3
				}
				PIXEL02_1M
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_2
					PIXEL21_3
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL12_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL22_4
				}
				break;
			case 239:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 127:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_2
					PIXEL01_3
					PIXEL10_3
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 191:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 223:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL10_3
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_2
					PIXEL12_3
				}
				PIXEL11
				PIXEL20_1M
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 247:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 255:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			}

			w1 = w2;
			w4 = w5;
			w7 = w8;

			w2 = w3;
			w5 = w6;
			w8 = w9;

			q += 3;
		}
		p += nextlineSrc - width;
		q += (nextlineDst - width) * 3;
	}
}

HQScaler::HQScaler(const Graphics::PixelFormat &format) : Scaler(format),
#ifdef USE_NASM
	_hqx_params(nullptr),
#endif
	_RGBtoYUV(nullptr) {
	_factor = 2;

	if (format.bytesPerPixel == 2) {
		initLUT(format);
	} else {
		// Pass a 16 bit 565 format to InitLut
		Graphics::PixelFormat format16(2,
		                               5, 6, 5, 0,
		                               11, 5, 0, 0);
		initLUT(format16);
	}
}

HQScaler::~HQScaler() {
	delete[] _RGBtoYUV;
	_RGBtoYUV = nullptr;

#ifdef USE_NASM
	delete _hqx_params;
	_hqx_params = nullptr;
#endif
}

#ifdef USE_NASM
void HQScaler::HQ2x16(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	hq2x_16(srcPtr, dstPtr, width, height, srcPitch, dstPitch, _hqx_params);
}

void HQScaler::HQ3x16(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	hq3x_16(srcPtr, dstPtr, width, height, srcPitch, dstPitch, _hqx_params);
}
#else
void HQScaler::HQ2x16(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	if (_format.gLoss == 2)
		HQ2x_implementation<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr,
				dstPitch, width, height, _RGBtoYUV);
	else
		HQ2x_implementation<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr,
				dstPitch, width, height, _RGBtoYUV);
}

void HQScaler::HQ3x16(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	if (_format.gLoss == 2)
		HQ3x_implementation<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr,
				dstPitch, width, height, _RGBtoYUV);
	else
		HQ3x_implementation<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr,
				dstPitch, width, height, _RGBtoYUV);
}
#endif

void HQScaler::scaleIntern(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	if (_format.bytesPerPixel == 2) {
		switch (_factor) {
		case 2:
			HQ2x16(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		case 3:
			HQ3x16(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
			break;
		}
	} else {
		switch (_factor) {
		case 2:
			if (_format.aLoss == 0)
				HQ2x_implementation<Graphics::ColorMasks<8888> >(srcPtr, srcPitch, dstPtr,
						dstPitch, width, height, _RGBtoYUV);
			else
				HQ2x_implementation<Graphics::ColorMasks<888> >(srcPtr, srcPitch, dstPtr,
						dstPitch, width, height, _RGBtoYUV);
			break;
		case 3:
			if (_format.aLoss == 0)
				HQ3x_implementation<Graphics::ColorMasks<8888> >(srcPtr, srcPitch, dstPtr,
						dstPitch, width, height, _RGBtoYUV);
			else
				HQ3x_implementation<Graphics::ColorMasks<888> >(srcPtr, srcPitch, dstPtr,
						dstPitch, width, height, _RGBtoYUV);
			break;
		}
	}
}

uint HQScaler::increaseFactor() {
	if (_factor < 3)
		setFactor(_factor + 1);
	return _factor;
}

uint HQScaler::decreaseFactor() {
	if (_factor > 2)
		setFactor(_factor - 1);
	return _factor;
}


class HQPlugin final : public ScalerPluginObject {
public:
	HQPlugin();

	Scaler *createInstance(const Graphics::PixelFormat &format) const override;

	bool canDrawCursor() const override { return false; }
	uint extraPixels() const override { return 1; }
	const char *getName() const override;
	const char *getPrettyName() const override;
};

HQPlugin::HQPlugin() {
	_factors.push_back(2);
	_factors.push_back(3);
}

Scaler *HQPlugin::createInstance(const Graphics::PixelFormat &format) const {
	return new HQScaler(format);
}

const char *HQPlugin::getName() const {
	return "hq";
}

const char *HQPlugin::getPrettyName() const {
	return "HQ";
}

REGISTER_PLUGIN_STATIC(HQ, PLUGIN_TYPE_SCALER, HQPlugin);
