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
 *
 * $URL$
 * $Id$
 *
 */
 
#include "common/util.h"
#include "common/system.h"
#include "common/events.h"

#include "graphics/surface.h"
#include "graphics/colormasks.h"

#include "gui/ThemeEngine.h"
#include "graphics/VectorRenderer.h"
#include "graphics/VectorRendererSpec.h"

#define VECTOR_RENDERER_FAST_TRIANGLES

const uint16 inv_sqrt_tbl[] = {
	0x0000, 0x0100, 0x016A, 0x01BB, 0x0200, 0x023C, 0x0273, 0x02A5,
	0x02D4, 0x0300, 0x0329, 0x0351, 0x0376, 0x039B, 0x03BD, 0x03DF,
	0x0400, 0x041F, 0x043E, 0x045B, 0x0478, 0x0495, 0x04B0, 0x04CB,
	0x04E6, 0x0500, 0x0519, 0x0532, 0x054A, 0x0562, 0x057A, 0x0591,
	0x05A8, 0x05BE, 0x05D4, 0x05EA, 0x0600, 0x0615, 0x062A, 0x063E,
	0x0653, 0x0667, 0x067B, 0x068E, 0x06A2, 0x06B5, 0x06C8, 0x06DB,
	0x06ED, 0x0700, 0x0712, 0x0724, 0x0736, 0x0747, 0x0759, 0x076A,
	0x077B, 0x078C, 0x079D, 0x07AE, 0x07BE, 0x07CF, 0x07DF, 0x07EF,
	0x0800, 0x080F, 0x081F, 0x082F, 0x083F, 0x084E, 0x085D, 0x086D,
	0x087C, 0x088B, 0x089A, 0x08A9, 0x08B7, 0x08C6, 0x08D4, 0x08E3,
	0x08F1, 0x0900, 0x090E, 0x091C, 0x092A, 0x0938, 0x0946, 0x0953,
	0x0961, 0x096F, 0x097C, 0x098A, 0x0997, 0x09A4, 0x09B2, 0x09BF,
	0x09CC, 0x09D9, 0x09E6, 0x09F3, 0x0A00, 0x0A0C, 0x0A19, 0x0A26,
	0x0A32, 0x0A3F, 0x0A4B, 0x0A58, 0x0A64, 0x0A70, 0x0A7C, 0x0A89,
	0x0A95, 0x0AA1, 0x0AAD, 0x0AB9, 0x0AC5, 0x0AD1, 0x0ADC, 0x0AE8,
	0x0AF4, 0x0B00, 0x0B0B, 0x0B17, 0x0B22, 0x0B2E, 0x0B39, 0x0B44,
	0x0B50, 0x0B5B, 0x0B66, 0x0B72, 0x0B7D, 0x0B88, 0x0B93, 0x0B9E,
	0x0BA9, 0x0BB4, 0x0BBF, 0x0BCA, 0x0BD5, 0x0BDF, 0x0BEA, 0x0BF5,
	0x0C00, 0x0C0A, 0x0C15, 0x0C1F, 0x0C2A, 0x0C34, 0x0C3F, 0x0C49,
	0x0C54, 0x0C5E, 0x0C68, 0x0C73, 0x0C7D, 0x0C87, 0x0C91, 0x0C9C,
	0x0CA6, 0x0CB0, 0x0CBA, 0x0CC4, 0x0CCE, 0x0CD8, 0x0CE2, 0x0CEC,
	0x0CF6, 0x0D00, 0x0D09, 0x0D13, 0x0D1D, 0x0D27, 0x0D30, 0x0D3A,
	0x0D44, 0x0D4D, 0x0D57, 0x0D61, 0x0D6A, 0x0D74, 0x0D7D, 0x0D87,
	0x0D90, 0x0D99, 0x0DA3, 0x0DAC, 0x0DB6, 0x0DBF, 0x0DC8, 0x0DD1,
	0x0DDB, 0x0DE4, 0x0DED, 0x0DF6, 0x0E00, 0x0E09, 0x0E12, 0x0E1B,
	0x0E24, 0x0E2D, 0x0E36, 0x0E3F, 0x0E48, 0x0E51, 0x0E5A, 0x0E63,
	0x0E6C, 0x0E74, 0x0E7D, 0x0E86, 0x0E8F, 0x0E98, 0x0EA0, 0x0EA9,
	0x0EB2, 0x0EBB, 0x0EC3, 0x0ECC, 0x0ED5, 0x0EDD, 0x0EE6, 0x0EEE,
	0x0EF7, 0x0F00, 0x0F08, 0x0F11, 0x0F19, 0x0F21, 0x0F2A, 0x0F32,
	0x0F3B, 0x0F43, 0x0F4C, 0x0F54, 0x0F5C, 0x0F65, 0x0F6D, 0x0F75,
	0x0F7D, 0x0F86, 0x0F8E, 0x0F96, 0x0F9E, 0x0FA7, 0x0FAF, 0x0FB7,
	0x0FBF, 0x0FC7, 0x0FCF, 0x0FD7, 0x0FDF, 0x0FE7, 0x0FEF, 0x0FF7,
	0x1000
};

inline uint32 fp_sqroot(uint32 x) {
    int bit;

//#if defined(ARM9)
//	__asm__ ("clz  %0, %1\nrsb  %0, %0, #31\n" : "=r"(bit) : "r" (x));
#if defined(__i386__)
	__asm__("bsrl %1, %0" : "=r" (bit) : "r" (x));
#else
	unsigned int mask = 0x40000000;
	bit = 30;
	while (bit >= 0) {
	    if (x & mask)
			break;

	    mask = (mask >> 1 | mask >> 2);
	    bit -= 2;
	}
#endif
	
	bit -= 6 + (bit & 1);
	return inv_sqrt_tbl[x >> bit] << (bit >> 1);
}

inline uint32 circleSqrt(int x) {
	return (x > 255 ? fp_sqroot(x) : inv_sqrt_tbl[x]) ^ 0xFF;
}


/*
	HELPER MACROS for Bresenham's circle drawing algorithm 
	Note the proper spelling on this header.
*/
#define __BE_ALGORITHM() { \
	if (f >= 0) { \
		y--; \
		ddF_y += 2; \
		f += ddF_y; \
		py -= pitch; \
	} \
	px += pitch; \
	ddF_x += 2; \
	f += ddF_x + 1; \
}

#define __BE_DRAWCIRCLE(ptr1,ptr2,ptr3,ptr4,x,y,px,py) { \
	*(ptr1 + (y) - (px)) = color; \
	*(ptr1 + (x) - (py)) = color; \
	*(ptr2 - (x) - (py)) = color; \
	*(ptr2 - (y) - (px)) = color; \
	*(ptr3 - (y) + (px)) = color; \
	*(ptr3 - (x) + (py)) = color; \
	*(ptr4 + (x) + (py)) = color; \
	*(ptr4 + (y) + (px)) = color; \
}

#define __BE_DRAWCIRCLE_XCOLOR(ptr1,ptr2,ptr3,ptr4,x,y,px,py) { \
	*(ptr1 + (y) - (px)) = color1; \
	*(ptr1 + (x) - (py)) = color2; \
	*(ptr2 - (x) - (py)) = color2; \
	*(ptr2 - (y) - (px)) = color1; \
	*(ptr3 - (y) + (px)) = color3; \
	*(ptr3 - (x) + (py)) = color4; \
	*(ptr4 + (x) + (py)) = color4; \
	*(ptr4 + (y) + (px)) = color3; \
}

#define __BE_RESET() { \
	f = 1 - r; \
	ddF_x = 0; ddF_y = -2 * r; \
	x = 0; y = r; px = 0; py = pitch * r; \
}

#define __TRIANGLE_MAINX() \
		if (error_term >= 0) { \
			ptr_right += pitch; \
			ptr_left += pitch; \
			error_term += dysub; \
		} else { \
			error_term += ddy; \
		} \
		ptr_right++; \
		ptr_left--;

#define __TRIANGLE_MAINY() \
		if (error_term >= 0) { \
			ptr_right++; \
			ptr_left--; \
			error_term += dxsub; \
		} else { \
			error_term += ddx; \
		} \
		ptr_right += pitch; \
		ptr_left += pitch;
		
/** HELPER MACROS for WU's circle drawing algorithm **/
#define __WU_DRAWCIRCLE(ptr1,ptr2,ptr3,ptr4,x,y,px,py,a) { \
	blendPixelPtr(ptr1 + (y) - (px), color, a); \
	blendPixelPtr(ptr1 + (x) - (py), color, a); \
	blendPixelPtr(ptr2 - (x) - (py), color, a); \
	blendPixelPtr(ptr2 - (y) - (px), color, a); \
	blendPixelPtr(ptr3 - (y) + (px), color, a); \
	blendPixelPtr(ptr3 - (x) + (py), color, a); \
	blendPixelPtr(ptr4 + (x) + (py), color, a); \
	blendPixelPtr(ptr4 + (y) + (px), color, a); \
}

// optimized Wu's algorithm
#define __WU_ALGORITHM() {\
	py += p; \
	oldT = T; \
	T = circleSqrt(rsq - (y * y)); \
	a2 = T; \
	a1 = ~T; \
	if (T < oldT) { x--; px -= p; } \
}



namespace Graphics {
    
VectorRenderer *createRenderer(int mode) {
	switch (mode) {
	case GUI::ThemeEngine::kGfxStandard16bit:
		return new VectorRendererSpec<uint16, ColorMasks<565> >;

	case GUI::ThemeEngine::kGfxAntialias16bit:
		return new VectorRendererAA<uint16, ColorMasks<565> >;

	default:
		return 0;
	}
}
     
template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
areaConvolution(const Common::Rect &area, const int filter[3][3], int filterDiv, int offset) {
	PixelType *ptr = 0;
	int newR, newG, newB;
	uint8 r, g, b;
	int yVal;
	
	for (int y = area.top; y < area.bottom; ++y) {
		for (int x = area.left; x < area.right; ++x) {
			newR = newG = newB = 0;

			for (int j = 0; j < 3; ++j) {
				yVal = MIN(MAX(y - 1 + j, 0), area.bottom - 1);
				
				for (int i = 0; i < 3; ++i) {
					ptr = (PixelType *)Base::_activeSurface->getBasePtr(MIN(MAX(x - 1 + j, 0), area.right - 1), yVal);
					colorToRGB<PixelFormat>((uint32)*ptr, r, g, b);
					
					newR += r * filter[j][i];
					newG += g * filter[j][i];
					newB += b * filter[j][i];
				}
			}
			
			newR = (newR / filterDiv) + offset;
			newG = (newG / filterDiv) + offset;
			newB = (newB / filterDiv) + offset;
			
			ptr = (PixelType *)Base::_activeSurface->getBasePtr(x, y);
			*ptr = RGBToColor<PixelFormat>(CLIP(newR, 0, 255), CLIP(newG, 0, 255), CLIP(newB, 0, 255));
		}		
	}
}

template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
setGradientColors(uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2) {
	_gradientEnd = RGBToColor<PixelFormat>(r2, g2, b2);
	_gradientStart = RGBToColor<PixelFormat>(r1, g1, b1);

	Base::_gradientBytes[0] = (_gradientEnd & PixelFormat::kRedMask) - (_gradientStart & PixelFormat::kRedMask);
	Base::_gradientBytes[1] = (_gradientEnd & PixelFormat::kGreenMask) - (_gradientStart & PixelFormat::kGreenMask);
	Base::_gradientBytes[2] = (_gradientEnd & PixelFormat::kBlueMask) - (_gradientStart & PixelFormat::kBlueMask);
}

template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
fillSurface() {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(0, 0);

	int w = _activeSurface->w;
	int h = _activeSurface->h ;
	int pitch = surfacePitch();

	if (Base::_fillMode == kFillBackground)
		colorFill(ptr, ptr + w * h, _bgColor);
	else if (Base::_fillMode == kFillForeground)
		colorFill(ptr, ptr + w * h, _fgColor);
	else if (Base::_fillMode == kFillGradient) {
		int i = h;
		while (i--) {
			colorFill(ptr, ptr + w, calcGradient(h - i, h));
			ptr += pitch;
		}
	}
}

template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
copyFrame(OSystem *sys, const Common::Rect &r) {

	sys->copyRectToOverlay(

#ifdef OVERLAY_MULTIPLE_DEPTHS
    (const PixelType*)
#else
	(const OverlayColor*)
#endif

	    _activeSurface->getBasePtr(r.left, r.top), _activeSurface->w, 
	    r.left, r.top, r.width(), r.height()
	);
}

template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
blitSurface(const Graphics::Surface *source, const Common::Rect &r) {
	assert(source->w == _activeSurface->w && source->h == _activeSurface->h);
	
	PixelType *dst_ptr = (PixelType *)_activeSurface->getBasePtr(r.left, r.top);
	PixelType *src_ptr = (PixelType *)source->getBasePtr(r.left, r.top);

	int dst_pitch = surfacePitch();
	int src_pitch = source->pitch / source->bytesPerPixel;

	int h = r.height(), w = r.width();

	while (h--) {
		memcpy(dst_ptr, src_ptr, w * sizeof(PixelType));
		dst_ptr += dst_pitch;
		src_ptr += src_pitch;
	}
}

template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
blitSubSurface(const Graphics::Surface *source, const Common::Rect &r) {
	PixelType *dst_ptr = (PixelType *)_activeSurface->getBasePtr(r.left, r.top);
	PixelType *src_ptr = (PixelType *)source->getBasePtr(0, 0);
	
	int dst_pitch = surfacePitch();
	int src_pitch = source->pitch / source->bytesPerPixel;
	
	int h = r.height(), w = r.width();
	
	while (h--) {
		memcpy(dst_ptr, src_ptr, w * sizeof(PixelType));
		dst_ptr += dst_pitch;
		src_ptr += src_pitch;
	}
}

template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
blitAlphaBitmap(const Graphics::Surface *source, const Common::Rect &r) {
	int16 x = r.left;
	int16 y = r.top;
	
	if (r.width() > source->w)
		x = x + (r.width() >> 1) - (source->w >> 1);
		
	if (r.height() > source->h)
		y = y + (r.height() >> 1) - (source->h >> 1);
		
	PixelType *dst_ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
	PixelType *src_ptr = (PixelType *)source->getBasePtr(0, 0);
	
	int dst_pitch = surfacePitch();
	int src_pitch = source->pitch / source->bytesPerPixel;
	
	int w, h = source->h;
	
	while (h--) {
		w = source->w;
		
		while (w--) {
			if (*src_ptr != _bitmapAlphaColor)
				*dst_ptr = *src_ptr;
				
			dst_ptr++;
			src_ptr++;
		}
		
		dst_ptr = dst_ptr - source->w + dst_pitch;
		src_ptr = src_ptr - source->w + src_pitch;
	}
}

template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
applyScreenShading(GUI::Theme::ShadingStyle shadingStyle) {
	int pixels = _activeSurface->w * _activeSurface->h;
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(0, 0);
	uint8 r, g, b;
	uint lum;

	const uint32 shiftMask = (uint32)~(
		(1 << PixelFormat::kGreenShift) | 
		(1 << PixelFormat::kRedShift) | 
		(1 << PixelFormat::kBlueShift)) >> 1;
	
	if (shadingStyle == GUI::Theme::kShadingDim) {

		int n = (pixels + 7) >> 3;
		switch (pixels % 8) {
		case 0: do { 
					*ptr++ = (*ptr >> 1) & shiftMask;
		case 7:		*ptr++ = (*ptr >> 1) & shiftMask;
		case 6:		*ptr++ = (*ptr >> 1) & shiftMask;
		case 5:		*ptr++ = (*ptr >> 1) & shiftMask;
		case 4:		*ptr++ = (*ptr >> 1) & shiftMask;
		case 3:		*ptr++ = (*ptr >> 1) & shiftMask;
		case 2:		*ptr++ = (*ptr >> 1) & shiftMask;
		case 1:		*ptr++ = (*ptr >> 1) & shiftMask;
				} while (--n > 0);
		}

	} else if (shadingStyle == GUI::Theme::kShadingLuminance) {
		while (pixels--) {
			colorToRGB<PixelFormat>(*ptr, r, g, b);
			lum = (r >> 2) + (g >> 1) + (b >> 3);
			*ptr++ = RGBToColor<PixelFormat>(lum, lum, lum);
		}
	}
}

template <typename PixelType, typename PixelFormat>
inline void VectorRendererSpec<PixelType, PixelFormat>::
blendPixelPtr(PixelType *ptr, PixelType color, uint8 alpha)	{
	register int idst = *ptr;
	register int isrc = color;

	*ptr = (PixelType)(
		(PixelFormat::kRedMask & ((idst & PixelFormat::kRedMask) +
		((int)(((int)(isrc & PixelFormat::kRedMask) -
		(int)(idst & PixelFormat::kRedMask)) * alpha) >> 8))) |
		(PixelFormat::kGreenMask & ((idst & PixelFormat::kGreenMask) +
		((int)(((int)(isrc & PixelFormat::kGreenMask) -
		(int)(idst & PixelFormat::kGreenMask)) * alpha) >> 8))) |
		(PixelFormat::kBlueMask & ((idst & PixelFormat::kBlueMask) +
		((int)(((int)(isrc & PixelFormat::kBlueMask) -
		(int)(idst & PixelFormat::kBlueMask)) * alpha) >> 8))) );
}

template <typename PixelType, typename PixelFormat>
inline PixelType VectorRendererSpec<PixelType, PixelFormat>::
calcGradient(uint32 pos, uint32 max) {
	PixelType output = 0;
	pos = (MIN(pos * Base::_gradientFactor, max) << 12) / max;
	
	output |= (_gradientStart + ((Base::_gradientBytes[0] * pos) >> 12)) & PixelFormat::kRedMask;
	output |= (_gradientStart + ((Base::_gradientBytes[1] * pos) >> 12)) & PixelFormat::kGreenMask;
	output |= (_gradientStart + ((Base::_gradientBytes[2] * pos) >> 12)) & PixelFormat::kBlueMask;
	output |= ~(PixelFormat::kRedMask | PixelFormat::kGreenMask | PixelFormat::kBlueMask);

	return output;
}

template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
colorFill(PixelType *first, PixelType *last, PixelType color) {
	register int count = (last - first);
	register int n = (count + 7) >> 3;
	switch (count % 8) {
	case 0: do { 
				*first++ = color;
	case 7:		*first++ = color;
	case 6:		*first++ = color;
	case 5:		*first++ = color;
	case 4:		*first++ = color;
	case 3:		*first++ = color;
	case 2:		*first++ = color;
	case 1:		*first++ = color;
			} while (--n > 0);
	}
}

/********************************************************************
 ********************************************************************
 * Primitive shapes drawing - Public API calls - VectorRendererSpec *
 ********************************************************************
 ********************************************************************/
template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawString(const Graphics::Font *font, const Common::String &text, const Common::Rect &area, 
			GUI::Theme::TextAlign alignH, GUI::Theme::TextAlignVertical alignV, int deltax, bool ellipsis) {

	int offset = area.top;
	
	if (font->getFontHeight() < area.height()) {
		switch (alignV) {
			case GUI::Theme::kTextAlignVCenter:
				offset = area.top + ((area.height() - font->getFontHeight()) >> 1);
				break;
			case GUI::Theme::kTextAlignVBottom:
				offset = area.bottom - font->getFontHeight();
				break;
			default:
				break;
		}
	}
	
	font->drawString(_activeSurface, text, area.left, offset, area.width(), _fgColor, (Graphics::TextAlignment)alignH, deltax, ellipsis);
}

/** LINES **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawLine(int x1, int y1, int x2, int y2) {
	x1 = CLIP(x1, 0, (int)Base::_activeSurface->w);
	x2 = CLIP(x2, 0, (int)Base::_activeSurface->w);
	y1 = CLIP(y1, 0, (int)Base::_activeSurface->h);
	y2 = CLIP(y2, 0, (int)Base::_activeSurface->h);

	// we draw from top to bottom
	if (y2 < y1) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	int dx = ABS(x2 - x1);
	int dy = ABS(y2 - y1);

	// this is a point, not a line. stoopid.
	if (dy == 0 && dx == 0) 
		return;

	if (Base::_strokeWidth == 0)
		return;

	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = Base::surfacePitch();
	int st = Base::_strokeWidth >> 1;

	if (dy == 0) { // horizontal lines
		// these can be filled really fast with a single memset.
		colorFill(ptr, ptr + dx + 1, (PixelType)_fgColor);
		
		for (int i = 0, p = pitch; i < st; ++i, p += pitch) {
			colorFill(ptr + p, ptr + dx + 1 + p, (PixelType)_fgColor);
			colorFill(ptr - p, ptr + dx + 1 - p, (PixelType)_fgColor);
		}

	} else if (dx == 0) { // vertical lines
		// these ones use a static pitch increase.
		while (y1++ <= y2) {
			colorFill(ptr - st, ptr + st, (PixelType)_fgColor);
			ptr += pitch;
		}

	} else if (ABS(dx) == ABS(dy)) { // diagonal lines
		// these ones also use a fixed pitch increase
		pitch += (x2 > x1) ? 1 : -1;

		while (dy--) {
			colorFill(ptr - st, ptr + st, (PixelType)_fgColor);
			ptr += pitch;
		}

	} else { // generic lines, use the standard algorithm...
		drawLineAlg(x1, y1, x2, y2, dx, dy, (PixelType)_fgColor);
	}
}

/** CIRCLES **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawCircle(int x, int y, int r) {
	if (x + r > Base::_activeSurface->w || y + r > Base::_activeSurface->h ||
		x - r < 0 || y - r < 0 || x == 0 || y == 0 || r <= 0)
		return;

	if (Base::_fillMode != kFillDisabled && Base::_shadowOffset 
		&& x + r + Base::_shadowOffset < Base::_activeSurface->w
		&& y + r + Base::_shadowOffset < Base::_activeSurface->h) {
		drawCircleAlg(x + Base::_shadowOffset + 1, y + Base::_shadowOffset + 1, r, 0, kFillForeground);
	}

	switch (Base::_fillMode) {
	case kFillDisabled:
		if (Base::_strokeWidth)
			drawCircleAlg(x, y, r, _fgColor, kFillDisabled);
		break;

	case kFillForeground:
		drawCircleAlg(x, y, r, _fgColor, kFillForeground);
		break;

	case kFillBackground:
		if (Base::_strokeWidth > 1) {
			drawCircleAlg(x, y, r, _fgColor, kFillForeground);
			drawCircleAlg(x, y, r - Base::_strokeWidth, _bgColor, kFillBackground);
		} else {
			drawCircleAlg(x, y, r, _bgColor, kFillBackground);
			drawCircleAlg(x, y, r, _fgColor, kFillDisabled);
		}
		break;

	case kFillGradient:
		break;
	}
}

/** SQUARES **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawSquare(int x, int y, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0)
		return;

	if (Base::_fillMode != kFillDisabled && Base::_shadowOffset
		&& x + w + Base::_shadowOffset < Base::_activeSurface->w
		&& y + h + Base::_shadowOffset < Base::_activeSurface->h) {
		drawSquareShadow(x, y, w, h, Base::_shadowOffset);
	}

	switch (Base::_fillMode) {
	case kFillDisabled:
		if (Base::_strokeWidth)
			drawSquareAlg(x, y, w, h, _fgColor, kFillDisabled);
		break;

	case kFillForeground:
		drawSquareAlg(x, y, w, h, _fgColor, kFillForeground);
		break;

	case kFillBackground:
		drawSquareAlg(x, y, w, h, _bgColor, kFillBackground);
		drawSquareAlg(x, y, w, h, _fgColor, kFillDisabled);
		break;

	case kFillGradient:
		VectorRendererSpec::drawSquareAlg(x, y, w, h, 0, kFillGradient);
		if (Base::_strokeWidth)
			drawSquareAlg(x, y, w, h, _fgColor, kFillDisabled);
		break;
	}
}

/** ROUNDED SQUARES **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawRoundedSquare(int x, int y, int r, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0 || r <= 0)
		return;
		
	if ((r << 1) > w || (r << 1) > h)
		r = MIN(w >> 1, h >> 1);

	if (Base::_fillMode != kFillDisabled && Base::_shadowOffset
		&& x + w + Base::_shadowOffset < Base::_activeSurface->w
		&& y + h + Base::_shadowOffset < Base::_activeSurface->h) {
		drawRoundedSquareShadow(x, y, r, w, h, Base::_shadowOffset);
	}

	switch (Base::_fillMode) {
	case kFillDisabled:
		if (Base::_strokeWidth)
			drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kFillDisabled);
		break;

	case kFillForeground:
		drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kFillForeground);
		break;

	case kFillBackground:
		VectorRendererSpec::drawRoundedSquareAlg(x, y, r, w, h, _bgColor, kFillBackground);
		drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kFillDisabled);
		break;

	case kFillGradient:
		if (Base::_strokeWidth > 1) {
			drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kFillForeground);
			VectorRendererSpec::drawRoundedSquareAlg(x + Base::_strokeWidth/2, y + Base::_strokeWidth/2, 
				r - Base::_strokeWidth/2, w - Base::_strokeWidth, h - Base::_strokeWidth, 0, kFillGradient);
		} else {
			VectorRendererSpec::drawRoundedSquareAlg(x, y, r, w, h, 0, kFillGradient);
			if (Base::_strokeWidth)
				drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kFillDisabled);
		}
		break;
	}
	
	if (Base::_bevel)
		drawRoundedSquareFakeBevel(x, y, r, w, h, Base::_bevel);
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawTab(int x, int y, int r, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0 || r > w || r > h)
		return;
		
	if (r == 0 && Base::_bevel > 0) {
		drawBevelTabAlg(x, y, w, h, Base::_bevel, _bevelColor, _fgColor, (Base::_dynamicData >> 16), (Base::_dynamicData & 0xFFFF));
		return;
	}
	
	if (r == 0) return;
	
	switch (Base::_fillMode) {
		case kFillDisabled:
			return;
		
		case kFillGradient:
		case kFillBackground:
			drawTabAlg(x, y, w, h, r, (Base::_fillMode == kFillBackground) ? _bgColor : _fgColor, Base::_fillMode);
			if (Base::_strokeWidth)
				drawTabAlg(x, y, w, h, r, _fgColor, kFillDisabled, (Base::_dynamicData >> 16), (Base::_dynamicData & 0xFFFF));
			break;
			
		case kFillForeground:
			drawTabAlg(x, y, w, h, r, (Base::_fillMode == kFillBackground) ? _bgColor : _fgColor, Base::_fillMode);
			break;
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawTriangle(int x, int y, int w, int h, TriangleOrientation orient) {
	
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h)
		return;

	PixelType color = 0;

	if (Base::_strokeWidth <= 1) {
		if (Base::_fillMode == kFillForeground)
			color = _fgColor;
		else if (Base::_fillMode == kFillBackground)
			color = _bgColor;
	} else {
		if (Base::_fillMode == kFillDisabled)
			return;
		color = _fgColor;
	}
	
	if (Base::_dynamicData != 0)
		orient = (TriangleOrientation)Base::_dynamicData;
		
	int newW = w / 2;
	if (newW % 2) newW++;

	switch(orient) {
		case kTriangleUp:
		case kTriangleDown:
			drawTriangleFast(x + (newW / 2), y + (h / 2) - (newW / 2), newW, (orient == kTriangleDown), color, Base::_fillMode);
			break;

		case kTriangleLeft:
		case kTriangleRight:
		case kTriangleAuto:
			break;
	}

	if (Base::_strokeWidth > 0)
		if (Base::_fillMode == kFillBackground || Base::_fillMode == kFillGradient) {
			drawTriangleFast(x + (newW / 2), y + (h / 2) - (newW / 2), newW, (orient == kTriangleDown), _fgColor, kFillDisabled);
		}
}





/********************************************************************
 ********************************************************************
 * Aliased Primitive drawing ALGORITHMS - VectorRendererSpec
 ********************************************************************
 ********************************************************************/
/** TAB ALGORITHM - NON AA */
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawTabAlg(int x1, int y1, int w, int h, int r, PixelType color, VectorRenderer::FillMode fill_m, int baseLeft, int baseRight) {
	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = Base::surfacePitch();
	int sw  = 0, sp = 0, hp = 0;
	
	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;
	int short_h = h - r + 2;
	int long_h = h;

	if (fill_m == kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			colorFill(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
			colorFill(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color);
			sp += pitch;

			__BE_RESET();
			r--;
			
			while (x++ < y) {
				__BE_ALGORITHM();
				*(ptr_tr + (y) - (px)) = color;
				*(ptr_tr + (x) - (py)) = color;
				*(ptr_tl - (x) - (py)) = color;
				*(ptr_tl - (y) - (px)) = color;

				if (Base::_strokeWidth > 1) {
					*(ptr_tr + (y) - (px - pitch)) = color;
					*(ptr_tr + (x) - (py)) = color;
					*(ptr_tl - (x) - (py)) = color;
					*(ptr_tl - (y) - (px - pitch)) = color;
				}
			} 
		}

		ptr_fill += pitch * real_radius;
		while (short_h--) {
			colorFill(ptr_fill, ptr_fill + Base::_strokeWidth - 1, color);
			colorFill(ptr_fill + w - Base::_strokeWidth + 2, ptr_fill + w, color);
			ptr_fill += pitch;
		}
		
		if (baseLeft) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1 + h);
			while (sw++ < Base::_strokeWidth) {
				colorFill(ptr_fill - baseLeft, ptr_fill, color);
				ptr_fill += pitch;
			}
		}
		
		if (baseRight) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w, y1 + h);
			while (sw++ < Base::_strokeWidth) {
				colorFill(ptr_fill, ptr_fill + baseRight, color);
				ptr_fill += pitch;
			}
		}
	} else {
		__BE_RESET();

		PixelType color1, color2;
		color1 = color2 = color;
		
		while (x++ < y) {
			__BE_ALGORITHM();
	
			if (fill_m == kFillGradient) {
				color1 = calcGradient(real_radius - x, long_h);
				color2 = calcGradient(real_radius - y, long_h);
			}
	
			colorFill(ptr_tl - x - py, ptr_tr + x - py, color2);
			colorFill(ptr_tl - y - px, ptr_tr + y - px, color1);
	
			*(ptr_tr + (y) - (px)) = color1;
			*(ptr_tr + (x) - (py)) = color2;
			*(ptr_tl - (x) - (py)) = color2;
			*(ptr_tl - (y) - (px)) = color1;
		}
	
		ptr_fill += pitch * r;
		while (short_h--) {
			if (fill_m == kFillGradient)
				color = calcGradient(real_radius++, long_h);
			colorFill(ptr_fill, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}
	}
}


/** BEVELED TABS FOR CLASSIC THEME **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawBevelTabAlg(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color, int baseLeft, int baseRight) {
	int pitch = Base::surfacePitch();
	int i, j;
	
	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
	
	i = bevel;
	while (i--) {
		colorFill(ptr_left, ptr_left + w, top_color);
		ptr_left += pitch;
	}

	if (baseLeft > 0) {
		i = h - bevel;
		ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
		while (i--) {
			colorFill(ptr_left, ptr_left + bevel, top_color);
			ptr_left += pitch;
		}
	}

	i = h - bevel;
	j = bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y);
	while (i--) {
		colorFill(ptr_left + j, ptr_left + bevel, bottom_color);
		if (j > 0) j--;
		ptr_left += pitch;
	}
	
	i = bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y + h - bevel);
	while (i--) {
		colorFill(ptr_left, ptr_left + baseRight + bevel, bottom_color);
		
		if (baseLeft)
			colorFill(ptr_left - w - baseLeft + bevel, ptr_left - w + bevel + bevel, top_color);
		ptr_left += pitch;
	}
}

/** SQUARE ALGORITHM **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawSquareAlg(int x, int y, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
	int pitch = Base::surfacePitch();
	int max_h = h;
	
	if (fill_m != kFillDisabled) {
		while (h--) {
			if (fill_m == kFillGradient)
				color = calcGradient(max_h - h, max_h);

			colorFill(ptr, ptr + w, color);
			ptr += pitch;
		}
	} else {
		int sw = Base::_strokeWidth, sp = 0, hp = pitch * (h - 1);

		while (sw--) {
			colorFill(ptr + sp, ptr + w + sp, color);
			colorFill(ptr + hp - sp, ptr + w + hp - sp, color);
			sp += pitch;
		}

		while (h--) {
			colorFill(ptr, ptr + Base::_strokeWidth, color);
			colorFill(ptr + w - Base::_strokeWidth, ptr + w, color);
			ptr += pitch;
		}
	}
}

/** SQUARE ALGORITHM **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawBevelSquareAlg(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color, bool fill) {
	int pitch = Base::surfacePitch();
	
	int height = h;
	PixelType *ptr_fill = (PixelType *)_activeSurface->getBasePtr(x, y);
	
	if (fill) {
		while (height--) {
			blendFill(ptr_fill, ptr_fill + w, _bgColor, 200);
			ptr_fill += pitch;
		}
	}
	
	int i, j;
	x = MAX(x - bevel, 0);
	y = MAX(y - bevel, 0);
	h += bevel << 1;
	w += bevel << 1;
	
	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
	
	i = bevel;
	while (i--) {
		colorFill(ptr_left, ptr_left + w, top_color);
		ptr_left += pitch;
	}

	i = h - bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y + bevel);
	while (i--) {
		colorFill(ptr_left, ptr_left + bevel, top_color);
		ptr_left += pitch;
	}

	i = bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y + h - bevel);
	while (i--) {
		colorFill(ptr_left + i, ptr_left + w, bottom_color);
		ptr_left += pitch;
	}

	i = h - bevel;
	j = bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y);
	while (i--) {
		colorFill(ptr_left + j, ptr_left + bevel, bottom_color);
		if (j > 0) j--;
		ptr_left += pitch;
	}
}

/** GENERIC LINE ALGORITHM **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType,PixelFormat>::
drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy, PixelType color) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = Base::surfacePitch();
	int xdir = (x2 > x1) ? 1 : -1;

	*ptr = (PixelType)color;

	if (dx > dy) {
		int ddy = dy * 2;
		int dysub = ddy - (dx * 2);
		int error_term = ddy - dx;

		while (dx--) {
			if (error_term >= 0) {
				ptr += pitch;
				error_term += dysub;
			} else {
				error_term += ddy;
			}

			ptr += xdir;
			*ptr = (PixelType)color;
		}
	} else {
		int ddx = dx * 2;
		int dxsub = ddx - (dy * 2);
		int error_term = ddx - dy;

		while (dy--) {
			if (error_term >= 0) {
				ptr += xdir;
				error_term += dxsub;
			} else {
				error_term += ddx;
			}

			ptr += pitch;
			*ptr = (PixelType)color;
		}
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x2, y2);
	*ptr = (PixelType)color;
}

/** VERTICAL TRIANGLE DRAWING ALGORITHM **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType,PixelFormat>::
drawTriangleVertAlg(int x1, int y1, int w, int h, bool inverted, PixelType color, VectorRenderer::FillMode fill_m) {
	int dx = w >> 1, dy = h, gradient_h = 0;
	int pitch = Base::surfacePitch();
	PixelType *ptr_right = 0, *ptr_left = 0;

	if (inverted) {
		ptr_right = (PixelType *)_activeSurface->getBasePtr(x1, y1);
		ptr_left = (PixelType *)_activeSurface->getBasePtr(x1 + w, y1);
	} else {
		ptr_right = ptr_left = (PixelType *)_activeSurface->getBasePtr(x1 + dx, y1);
	}

	if (dx > dy) {
		int ddy = dy * 2;
		int dysub = ddy - (dx * 2);
		int error_term = ddy - dx;

		switch(fill_m) {
		case kFillDisabled:
			while (dx--) {
				__TRIANGLE_MAINX();
				*ptr_right = color;
				*ptr_left = color;
			}
			colorFill(ptr_left, ptr_right, color);
			break;

		case kFillForeground:
		case kFillBackground:
			while (dx--) {
				__TRIANGLE_MAINX();
				if (inverted) colorFill(ptr_right, ptr_left, color);
				else colorFill(ptr_left, ptr_right, color);
			}
			break;

		case kFillGradient:
			while (dx--) {
				__TRIANGLE_MAINX();
				if (inverted) colorFill(ptr_right, ptr_left, calcGradient(gradient_h++, h));
				else colorFill(ptr_left, ptr_right, calcGradient(gradient_h++, h));
			}
			break;
		}
	} else {
		int ddx = dx * 2;
		int dxsub = ddx - (dy * 2);
		int error_term = ddx - dy;

		switch(fill_m) {
		case kFillDisabled:
			while (dy--) {
				__TRIANGLE_MAINY();
				*ptr_right = color;
				*ptr_left = color;
			}
			colorFill(ptr_left, ptr_right, color);
			break;

		case kFillForeground:
		case kFillBackground:
			while (dy--) {
				__TRIANGLE_MAINY();
				if (inverted) colorFill(ptr_right, ptr_left, color);
				else colorFill(ptr_left, ptr_right, color);
			}
			break;
		case kFillGradient:
			while (dy--) {
				__TRIANGLE_MAINY();
				if (inverted) colorFill(ptr_right, ptr_left, calcGradient(gradient_h++, h));
				else colorFill(ptr_left, ptr_right, calcGradient(gradient_h++, h));
			}
			break;
		}
	}
}


/** VERTICAL TRIANGLE DRAWING - FAST VERSION FOR SQUARED TRIANGLES */
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType,PixelFormat>::
drawTriangleFast(int x1, int y1, int size, bool inverted, PixelType color, VectorRenderer::FillMode fill_m) {
	int pitch = Base::surfacePitch();
	int hstep = 0, dy = size;
	bool grad = (fill_m == kFillGradient);
	
	PixelType *ptr_right = 0, *ptr_left = 0;
	
	if (inverted) {
		ptr_left = (PixelType *)_activeSurface->getBasePtr(x1, y1);
		ptr_right = (PixelType *)_activeSurface->getBasePtr(x1 + size, y1);
	} else {
		ptr_left = (PixelType *)_activeSurface->getBasePtr(x1, y1 + size);
		ptr_right = (PixelType *)_activeSurface->getBasePtr(x1 + size, y1 + size);
		pitch = -pitch;
	}
	
	if (fill_m == kFillDisabled) {
		while (ptr_left < ptr_right) {
			*ptr_left = color;
			*ptr_right = color;
			ptr_left += pitch;
			ptr_right += pitch;
			if (hstep++ % 2) {
				ptr_left++;
				ptr_right--;
			}
		}
	} else {
		while (ptr_left < ptr_right) {
			colorFill(ptr_left, ptr_right, grad ? calcGradient(dy--, size) : color);
			ptr_left += pitch;
			ptr_right += pitch;
			if (hstep++ % 2) {
				ptr_left++;
				ptr_right--;
			}	
		}
	}
}

/** ROUNDED SQUARE ALGORITHM **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = Base::surfacePitch();
	int sw = 0, sp = 0, hp = h * pitch;
	
	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;
	int short_h = h - (2 * r) + 2;
	int long_h = h;

	if (fill_m == kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			colorFill(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
			colorFill(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color);
			sp += pitch;

			__BE_RESET();
			r--;
			
			while (x++ < y) {
				__BE_ALGORITHM();
				__BE_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py);

				if (Base::_strokeWidth > 1) {
					__BE_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x - 1, y, px, py);
					__BE_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px - pitch, py);
				}
			} 
		}

		ptr_fill += pitch * real_radius;
		while (short_h--) {
			colorFill(ptr_fill, ptr_fill + Base::_strokeWidth, color);
			colorFill(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}
	} else {
		__BE_RESET();
		PixelType color1, color2, color3, color4;
		
		if (fill_m == kFillGradient) {
			while (x++ < y) {
				__BE_ALGORITHM();
				
				color1 = calcGradient(real_radius - x, long_h);
				color2 = calcGradient(real_radius - y, long_h);
				color3 = calcGradient(long_h - r + x, long_h);
				color4 = calcGradient(long_h - r + y, long_h);
				
				colorFill(ptr_tl - x - py, ptr_tr + x - py, color2);
				colorFill(ptr_tl - y - px, ptr_tr + y - px, color1);

				colorFill(ptr_bl - x + py, ptr_br + x + py, color4);
				colorFill(ptr_bl - y + px, ptr_br + y + px, color3);
				
				__BE_DRAWCIRCLE_XCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py);
			}
		} else {
			while (x++ < y) {
				__BE_ALGORITHM();			

				colorFill(ptr_tl - x - py, ptr_tr + x - py, color);
				colorFill(ptr_tl - y - px, ptr_tr + y - px, color);

				colorFill(ptr_bl - x + py, ptr_br + x + py, color);
				colorFill(ptr_bl - y + px, ptr_br + y + px, color);

				// do not remove - messes up the drawing at lower resolutions
				__BE_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py);
			}
		}

		ptr_fill += pitch * r;
		while (short_h--) {
			if (fill_m == kFillGradient)
				color = calcGradient(real_radius++, long_h);
			colorFill(ptr_fill, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}
	}
}

/** CIRCLE ALGORITHM **/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawCircleAlg(int x1, int y1, int r, PixelType color, VectorRenderer::FillMode fill_m) {
	int f, ddF_x, ddF_y;
	int x, y, px, py, sw = 0;
	int pitch = Base::surfacePitch();
	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	if (fill_m == kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			__BE_RESET();
			r--;

			*(ptr + y) = color;
			*(ptr - y) = color;
			*(ptr + py) = color;
			*(ptr - py) = color;

			while (x++ < y) {
				__BE_ALGORITHM();
				__BE_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py);

				if (Base::_strokeWidth > 1) {
					__BE_DRAWCIRCLE(ptr, ptr, ptr, ptr, x - 1, y, px, py);
					__BE_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px - pitch, py);
				}
			}
		}
	} else {
		colorFill(ptr - r, ptr + r, color);
		__BE_RESET();

		while (x++ < y) {
			__BE_ALGORITHM();
			colorFill(ptr - x + py, ptr + x + py, color);
			colorFill(ptr - x - py, ptr + x - py, color);
			colorFill(ptr - y + px, ptr + y + px, color);
			colorFill(ptr - y - px, ptr + y - px, color);
		}
	}
}





/********************************************************************
 ********************************************************************
 * SHADOW drawing algorithms - VectorRendererSpec *******************
 ********************************************************************
 ********************************************************************/
template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawSquareShadow(int x, int y, int w, int h, int blur) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x + w - 1, y + blur);
	int pitch = Base::surfacePitch();
	int i, j;

	i = h - blur;

	while (i--) {
		j = blur;
		while (j--)
			blendPixelPtr(ptr + j, 0, ((blur - j) << 8) / blur);
		ptr += pitch;
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x + blur, y + h - 1);

	while (i++ < blur) {
		j = w - blur;
		while (j--)
			blendPixelPtr(ptr + j, 0, ((blur - i) << 8) / blur);
		ptr += pitch;
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x + w, y + h);

	i = 0;
	while (i++ < blur) {
		j = blur - 1;
		while (j--)
			blendPixelPtr(ptr + j, 0, (((blur - j) * (blur - i)) << 8) / (blur * blur));
		ptr += pitch;
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawRoundedSquareShadow(int x1, int y1, int r, int w, int h, int blur) {
	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = Base::surfacePitch();
	int alpha = 102;

	x1 += blur;
	y1 += blur;

	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - blur, y1 + r);

	int short_h = h - (2 * r) + 1;

	__BE_RESET();

	// HACK: As we are drawing circles exploting 8-axis symmetry,
	// there are 4 pixels on each circle which are drawn twice.
	// this is ok on filled circles, but when blending on surfaces,
	// we cannot let it blend twice. awful.
	uint32 hb = 0;

	while (x++ < y) {
		__BE_ALGORITHM();

		if (((1 << x) & hb) == 0) {
			blendFill(ptr_tr - px - r, ptr_tr + y - px, 0, alpha);
			blendFill(ptr_bl - y + px, ptr_br + y + px, 0, alpha);
			hb |= (1 << x);
		}

		if (((1 << y) & hb) == 0) {
			blendFill(ptr_tr - r - py, ptr_tr + x - py, 0, alpha);
			blendFill(ptr_bl - x + py, ptr_br + x + py, 0, alpha);
			hb |= (1 << y);
		}
	}

	while (short_h--) {
		blendFill(ptr_fill - r, ptr_fill + blur, 0, alpha);
		ptr_fill += pitch;
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawRoundedSquareFakeBevel(int x1, int y1, int r, int w, int h, int amount) {
	int x, y;
	int p = Base::surfacePitch(), px, py;
	int sw = 0, sp = 0;

	uint32 rsq = (r * r) << 16;
	uint32 T = 0, oldT;
	uint8 a1, a2;
	
	PixelType color = _bevelColor; //RGBToColor<PixelFormat>(63, 60, 17);

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int short_h = h - 2 * r;
	
	while (sw++ < amount) {
		colorFill(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
		sp += p;

		x = r - (sw - 1); y = 0; T = 0;
		px = p * x; py = 0;
		
		while (x > y++) {
			__WU_ALGORITHM();

			blendPixelPtr(ptr_tr + (y) - (px - p), color, a2);
			blendPixelPtr(ptr_tr + (x - 1) - (py), color, a2);
			blendPixelPtr(ptr_tl - (x - 1) - (py), color, a2);
			blendPixelPtr(ptr_tl - (y) - (px - p), color, a2);
			blendPixelPtr(ptr_bl - (y) + (px - p), color, a2);
			blendPixelPtr(ptr_bl - (x - 1) + (py), color, a2);
			
			blendPixelPtr(ptr_tr + (y) - (px), color, a1);
			blendPixelPtr(ptr_tr + (x) - (py), color, a1);
			blendPixelPtr(ptr_tl - (x) - (py), color, a1);
			blendPixelPtr(ptr_tl - (y) - (px), color, a1);
			blendPixelPtr(ptr_bl - (y) + (px), color, a1);
			blendPixelPtr(ptr_bl - (x) + (py), color, a1);
		} 
	}

	ptr_fill += p * r;
	while (short_h-- >= 0) {
		colorFill(ptr_fill, ptr_fill + amount, color);
		ptr_fill += p;
	}
}







/******************************************************************************/









/********************************************************************
 * ANTIALIASED PRIMITIVES drawing algorithms - VectorRendererAA
 ********************************************************************/
/** LINES **/
template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy, PixelType color) {

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);
	int pitch = Base::surfacePitch();
	int xdir = (x2 > x1) ? 1 : -1;
	uint16 error_tmp, error_acc, gradient;
	uint8 alpha;

	*ptr = (PixelType)color;

	if (dx > dy) {
		gradient = (uint32)(dy << 16) / (uint32)dx;
		error_acc = 0;

		while (--dx) {
			error_tmp = error_acc;
			error_acc += gradient;

			if (error_acc <= error_tmp)
				ptr += pitch;

			ptr += xdir;
			alpha = (error_acc >> 8);

			blendPixelPtr(ptr, color, ~alpha);
			blendPixelPtr(ptr + pitch, color, alpha);
		}
	} else {
		gradient = (uint32)(dx << 16) / (uint32)dy;
		error_acc = 0;

		while (--dy) {
			error_tmp = error_acc;
			error_acc += gradient;

			if (error_acc <= error_tmp)
				ptr += xdir;

			ptr += pitch;
			alpha = (error_acc >> 8);

			blendPixelPtr(ptr, color, ~alpha);
			blendPixelPtr(ptr + xdir, color, alpha);
		}
	}

	Base::putPixel(x2, y2, color);
}

/** ROUNDED SQUARES **/
template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	int x, y;
	int p = Base::surfacePitch(), px, py;
	int sw = 0, sp = 0, hp = h * p;

	uint32 rsq = (r * r) << 16;
	uint32 T = 0, oldT;
	uint8 a1, a2;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int short_h = h - 2 * r;

	if (fill_m == VectorRenderer::kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			colorFill(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
			colorFill(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color);
			sp += p;

			x = r - (sw - 1); y = 0; T = 0;
			px = p * x; py = 0;
			
			while (x > y++) {
				__WU_ALGORITHM();

				if (sw != 1 && sw != Base::_strokeWidth)
					a2 = a1 = 255;

				__WU_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, (x - 1), y, (px - p), py, a2);
				__WU_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1);
			} 
		}

		ptr_fill += p * r;
		while (short_h-- >= 0) {
			colorFill(ptr_fill, ptr_fill + Base::_strokeWidth, color);
			colorFill(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color);
			ptr_fill += p;
		} 
	} else {
		x = r; y = 0; T = 0;
		px = p * x; py = 0;
		
		while (x > 1 + y++) {
			__WU_ALGORITHM();

			colorFill(ptr_tl - x - py, ptr_tr + x - py, color);
			colorFill(ptr_tl - y - px, ptr_tr + y - px, color);

			colorFill(ptr_bl - x + py, ptr_br + x + py, color);
			colorFill(ptr_bl - y + px, ptr_br + y + px, color);

			__WU_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1);
		}

		ptr_fill += p * r;
		while (short_h-- >= 0) {
			colorFill(ptr_fill, ptr_fill + w + 1, color);
			ptr_fill += p;
		}
	}
}

/** CIRCLES **/
template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
drawCircleAlg(int x1, int y1, int r, PixelType color, VectorRenderer::FillMode fill_m) {
	int x, y, sw = 0;
	int p = Base::surfacePitch(), px, py;

	uint32 rsq = (r * r) << 16;
	uint32 T = 0, oldT;
	uint8 a1, a2;

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	if (fill_m == VectorRenderer::kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			x = r - (sw - 1); y = 0; T = 0;
			px = p * x; py = 0;

			*(ptr + x) = (PixelType)color;
			*(ptr - x) = (PixelType)color;
			*(ptr + px) = (PixelType)color;
			*(ptr - px) = (PixelType)color;

			while (x > y++) {
				__WU_ALGORITHM();

				if (sw != 1 && sw != Base::_strokeWidth)
					a2 = a1 = 255;

				__WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, (x - 1), y, (px - p), py, a2);
				__WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py, a1);
			}
		}
	} else {
		colorFill(ptr - r, ptr + r + 1, color);
		x = r; y = 0; T = 0;
		px = p * x; py = 0;

		while (x > y++) {
			__WU_ALGORITHM();

			colorFill(ptr - x + py, ptr + x + py, color);
			colorFill(ptr - x - py, ptr + x - py, color);
			colorFill(ptr - y + px, ptr + y + px, color);
			colorFill(ptr - y - px, ptr + y - px, color);
				
			__WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py, a1);
		}				
	}
}

     
}
