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
 */

#include "common/util.h"
#include "common/system.h"
#include "common/frac.h"

#include "graphics/surface.h"
#include "graphics/transparent_surface.h"
#include "graphics/nine_patch.h"

#include "gui/ThemeEngine.h"
#include "graphics/VectorRenderer.h"
#include "graphics/VectorRendererSpec.h"

#define VECTOR_RENDERER_FAST_TRIANGLES

/** Fixed point SQUARE ROOT **/
inline frac_t fp_sqroot(uint32 x) {
#if 0
	// Use the FPU to compute the square root and then convert it to fixed
	// point data. On systems with a fast FPU, this can be a lot faster than
	// the integer based code below - on my system roughly 50x! However, on
	// systems without an FPU, the converse might be true.
	// For now, we only use the integer based code.
	return doubleToFrac(sqrt((double)x));
#else
	// The code below wants to use a lot of registers, which is not good on
	// x86 processors. By taking advantage of the fact the the input value is
	// an integer, it might be possible to improve this. Furthermore, we could
	// take advantage of the fact that we call this function several times on
	// decreasing values. By feeding it the sqrt of the previous old x, as well
	// as the old x, it should be possible to compute the correct sqrt with far
	// fewer than 23 iterations.
	uint32 root, remHI, remLO, testDIV, count;

	root = 0;
	remHI = 0;
	remLO = x << 16;
	count = 23;

	do {
		remHI = (remHI << 2) | (remLO >> 30);
		remLO <<= 2;
		root <<= 1;
		testDIV = (root << 1) + 1;

		if (remHI >= testDIV) {
			remHI -= testDIV;
			root++;
		}
	} while (count--);

	return root;
#endif
}

/*
	HELPER MACROS for Bresenham's circle drawing algorithm
	Note the proper spelling on this header.
*/
#define BE_ALGORITHM() do { \
	if (f >= 0) { \
		y--; \
		ddF_y += 2; \
		f += ddF_y; \
		py -= pitch; \
	} \
	px += pitch; \
	ddF_x += 2; \
	f += ddF_x + 1; \
} while(0)

#define BE_DRAWCIRCLE_TOP(ptr1,ptr2,x,y,px,py) do { \
	*(ptr1 + (y) - (px)) = color; \
	*(ptr1 + (x) - (py)) = color; \
	*(ptr2 - (x) - (py)) = color; \
	*(ptr2 - (y) - (px)) = color; \
} while (0)

#define BE_DRAWCIRCLE_BOTTOM(ptr3,ptr4,x,y,px,py) do { \
	*(ptr3 - (y) + (px)) = color; \
	*(ptr3 - (x) + (py)) = color; \
	*(ptr4 + (x) + (py)) = color; \
	*(ptr4 + (y) + (px)) = color; \
} while (0)

#define BE_DRAWCIRCLE(ptr1,ptr2,ptr3,ptr4,x,y,px,py) do { \
	BE_DRAWCIRCLE_TOP(ptr1,ptr2,x,y,px,py); \
	BE_DRAWCIRCLE_BOTTOM(ptr3,ptr4,x,y,px,py); \
} while (0)

#define BE_DRAWCIRCLE_TOP_CLIP(ptr1,ptr2,x,y,px,py,realX1,realY1,realX2,realY2) do { \
	if (IS_IN_CLIP((realX1) + (y), (realY1) - (x))) \
		*(ptr1 + (y) - (px)) = color; \
	if (IS_IN_CLIP((realX1) + (x), (realY1) - (y))) \
		*(ptr1 + (x) - (py)) = color; \
	if (IS_IN_CLIP((realX2) - (x), (realY2) - (y))) \
		*(ptr2 - (x) - (py)) = color; \
	if (IS_IN_CLIP((realX2) - (y), (realY2) - (x))) \
		*(ptr2 - (y) - (px)) = color; \
} while (0)

#define BE_DRAWCIRCLE_BOTTOM_CLIP(ptr3,ptr4,x,y,px,py,realX3,realY3,realX4,realY4) do { \
	if (IS_IN_CLIP((realX3) - (y), (realY3) + (x))) \
		*(ptr3 - (y) + (px)) = color; \
	if (IS_IN_CLIP((realX3) - (x), (realY3) + (y))) \
		*(ptr3 - (x) + (py)) = color; \
	if (IS_IN_CLIP((realX4) + (x), (realY4) + (y))) \
		*(ptr4 + (x) + (py)) = color; \
	if (IS_IN_CLIP((realX4) + (y), (realY4) + (x))) \
		*(ptr4 + (y) + (px)) = color; \
} while (0)

#define BE_DRAWCIRCLE_CLIP(ptr1,ptr2,ptr3,ptr4,x,y,px,py,realX1,realY1,realX2,realY2,realX3,realY3,realX4,realY4) do { \
	BE_DRAWCIRCLE_TOP_CLIP(ptr1,ptr2,x,y,px,py,realX1,realY1,realX2,realY2); \
	BE_DRAWCIRCLE_BOTTOM_CLIP(ptr3,ptr4,x,y,px,py,realX3,realY3,realX4,realY4); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR(ptr1,ptr2,ptr3,ptr4,x,y,px,py) do { \
	*(ptr1 + (y) - (px)) = color1; \
	*(ptr1 + (x) - (py)) = color1; \
	*(ptr2 - (x) - (py)) = color1; \
	*(ptr2 - (y) - (px)) = color1; \
	*(ptr3 - (y) + (px)) = color1; \
	*(ptr3 - (x) + (py)) = color1; \
	*(ptr4 + (x) + (py)) = color2; \
	*(ptr4 + (y) + (px)) = color2; \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_CLIP(ptr1,ptr2,ptr3,ptr4,x,y,px,py,realX1,realY1,realX2,realY2,realX3,realY3,realX4,realY4) do { \
	if (IS_IN_CLIP((realX1) + (y), (realY1) - (x))) \
		*(ptr1 + (y) - (px)) = color1; \
	if (IS_IN_CLIP((realX1) + (x), (realY1) - (y))) \
		*(ptr1 + (x) - (py)) = color1; \
	if (IS_IN_CLIP((realX2) - (x), (realY2) - (y))) \
		*(ptr2 - (x) - (py)) = color1; \
	if (IS_IN_CLIP((realX2) - (y), (realY2) - (x))) \
		*(ptr2 - (y) - (px)) = color1; \
	if (IS_IN_CLIP((realX3) - (y), (realY3) + (x))) \
		*(ptr3 - (y) + (px)) = color1; \
	if (IS_IN_CLIP((realX3) - (x), (realY3) + (y))) \
		*(ptr3 - (x) + (py)) = color1; \
	if (IS_IN_CLIP((realX4) + (x), (realY4) + (y))) \
		*(ptr4 + (x) + (py)) = color2; \
	if (IS_IN_CLIP((realX4) + (y), (realY4) + (x))) \
		*(ptr4 + (y) + (px)) = color2; \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_TR_CW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr + (y) - (px), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_TR_CCW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr + (x) - (py), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_TR_CW_CLIP(ptr,x,y,px,py,a,realX,realY) do { \
	if (IS_IN_CLIP((realX) + (y), (realY) - (x))) \
		this->blendPixelPtr(ptr + (y) - (px), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_TR_CCW_CLIP(ptr,x,y,px,py,a,realX,realY) do { \
	if (IS_IN_CLIP((realX) + (x), (realY) - (y))) \
		this->blendPixelPtr(ptr + (x) - (py), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_TL_CW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr - (x) - (py), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_TL_CCW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr - (y) - (px), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_TL_CW_CLIP(ptr,x,y,px,py,a,realX,realY) do { \
	if (IS_IN_CLIP((realX) - (x), (realY) - (y))) \
		this->blendPixelPtr(ptr - (x) - (py), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_TL_CCW_CLIP(ptr,x,y,px,py,a,realX,realY) do { \
	if (IS_IN_CLIP((realX) - (y), (realY) - (x))) \
		this->blendPixelPtr(ptr - (y) - (px), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_BL_CW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr - (y) + (px), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_BL_CCW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr - (x) + (py), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_BL_CW_CLIP(ptr,x,y,px,py,a,realX,realY) do { \
	if (IS_IN_CLIP((realX) - (y), (realY) + (x))) \
		this->blendPixelPtr(ptr - (y) + (px), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_BL_CCW_CLIP(ptr,x,y,px,py,a,realX,realY) do { \
	if (IS_IN_CLIP((realX) - (x), (realY) + (y))) \
		this->blendPixelPtr(ptr - (x) + (py), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_BR_CW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr + (x) + (py), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_BR_CCW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr + (y) + (px), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_BR_CW_CLIP(ptr,x,y,px,py,a,realX,realY) do { \
	if (IS_IN_CLIP((realX) + (x), (realY) + (y))) \
		this->blendPixelPtr(ptr + (x) + (py), color, a); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR_BR_CCW_CLIP(ptr,x,y,px,py,a,realX,realY) do { \
	if (IS_IN_CLIP((realX) + (y), (realY) + (x))) \
		this->blendPixelPtr(ptr + (y) + (px), color, a); \
} while (0)

#define BE_DRAWCIRCLE_XCOLOR_TOP(ptr1,ptr2,x,y,px,py) do { \
	*(ptr1 + (y) - (px)) = color1; \
	*(ptr1 + (x) - (py)) = color2; \
	*(ptr2 - (x) - (py)) = color2; \
	*(ptr2 - (y) - (px)) = color1; \
} while (0)

#define BE_DRAWCIRCLE_XCOLOR_BOTTOM(ptr3,ptr4,x,y,px,py) do { \
	*(ptr3 - (y) + (px)) = color3; \
	*(ptr3 - (x) + (py)) = color4; \
	*(ptr4 + (x) + (py)) = color4; \
	*(ptr4 + (y) + (px)) = color3; \
} while (0)

#define BE_DRAWCIRCLE_XCOLOR(ptr1,ptr2,ptr3,ptr4,x,y,px,py) do { \
	BE_DRAWCIRCLE_XCOLOR_TOP(ptr1,ptr2,x,y,px,py); \
	BE_DRAWCIRCLE_XCOLOR_BOTTOM(ptr3,ptr4,x,y,px,py); \
} while (0)

#define IS_IN_CLIP(x,y) (_clippingArea.left <= (x) && (x) < _clippingArea.right \
	&& _clippingArea.top <= (y) && (y) < _clippingArea.bottom)

#define BE_DRAWCIRCLE_XCOLOR_TOP_CLIP(ptr1,ptr2,x,y,px,py,realX1,realY1,realX2,realY2) do { \
	if (IS_IN_CLIP((realX1) + (y), (realY1) - (x))) \
		*(ptr1 + (y) - (px)) = color1; \
\
	if (IS_IN_CLIP((realX1) + (x), (realY1) - (y))) \
		*(ptr1 + (x) - (py)) = color2; \
\
	if (IS_IN_CLIP((realX2) - (x), (realY2) - (y))) \
		*(ptr2 - (x) - (py)) = color2; \
\
	if (IS_IN_CLIP((realX2) - (y), (realY2) - (x))) \
		*(ptr2 - (y) - (px)) = color1; \
} while (0)

#define BE_DRAWCIRCLE_XCOLOR_BOTTOM_CLIP(ptr3,ptr4,x,y,px,py,realX3,realY3,realX4,realY4) do { \
	if (IS_IN_CLIP((realX3) - (y), (realY3) + (x))) \
		*(ptr3 - (y) + (px)) = color3; \
\
	if (IS_IN_CLIP((realX3) - (x), (realY3) + (y))) \
		*(ptr3 - (x) + (py)) = color4; \
\
	if (IS_IN_CLIP((realX4) + (x), (realY4) + (y))) \
		*(ptr4 + (x) + (py)) = color4; \
\
	if (IS_IN_CLIP((realX4) + (y), (realY4) + (x))) \
		*(ptr4 + (y) + (px)) = color3; \
} while (0)

#define BE_DRAWCIRCLE_XCOLOR_CLIP(ptr1,ptr2,ptr3,ptr4,x,y,px,py,realX1,realY1,realX2,realY2,realX3,realY3,realX4,realY4) do { \
	BE_DRAWCIRCLE_XCOLOR_TOP_CLIP(ptr1,ptr2,x,y,px,py,realX1,realY1,realX2,realY2); \
	BE_DRAWCIRCLE_XCOLOR_BOTTOM_CLIP(ptr3,ptr4,x,y,px,py,realX3,realY3,realX4,realY4); \
} while (0)


#define BE_RESET() do { \
	f = 1 - r; \
	ddF_x = 0; ddF_y = -2 * r; \
	x = 0; y = r; px = 0; py = pitch * r; \
} while (0)

#define TRIANGLE_MAINX() \
		if (error_term >= 0) { \
			ptr_right += pitch; \
			ptr_left += pitch; \
			error_term += dysub; \
		} else { \
			error_term += ddy; \
		} \
		ptr_right++; \
		ptr_left--;

#define TRIANGLE_MAINY() \
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
#define WU_DRAWCIRCLE_TOP(ptr1,ptr2,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr1 + (y) - (px), color, a); \
	this->blendPixelPtr(ptr1 + (x) - (py), color, a); \
	this->blendPixelPtr(ptr2 - (x) - (py), color, a); \
	this->blendPixelPtr(ptr2 - (y) - (px), color, a); \
} while (0)

#define WU_DRAWCIRCLE_BOTTOM(ptr3,ptr4,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr3 - (y) + (px), color, a); \
	this->blendPixelPtr(ptr3 - (x) + (py), color, a); \
	this->blendPixelPtr(ptr4 + (x) + (py), color, a); \
	this->blendPixelPtr(ptr4 + (y) + (px), color, a); \
} while (0)

#define WU_DRAWCIRCLE(ptr1,ptr2,ptr3,ptr4,x,y,px,py,a) do { \
	WU_DRAWCIRCLE_TOP(ptr1,ptr2,x,y,px,py,a); \
	WU_DRAWCIRCLE_BOTTOM(ptr3,ptr4,x,y,px,py,a); \
} while (0)


// Color depending on y
// Note: this is only for the outer pixels
#define WU_DRAWCIRCLE_XCOLOR_TOP(ptr1,ptr2,x,y,px,py,a,func) do { \
	this->func(ptr1 + (y) - (px), color1, a); \
	this->func(ptr1 + (x) - (py), color2, a); \
	this->func(ptr2 - (x) - (py), color2, a); \
	this->func(ptr2 - (y) - (px), color1, a); \
} while (0)

#define WU_DRAWCIRCLE_XCOLOR_BOTTOM(ptr3,ptr4,x,y,px,py,a,func) do { \
	this->func(ptr3 - (y) + (px), color3, a); \
	this->func(ptr3 - (x) + (py), color4, a); \
	this->func(ptr4 + (x) + (py), color4, a); \
	this->func(ptr4 + (y) + (px), color3, a); \
} while (0)

#define WU_DRAWCIRCLE_XCOLOR(ptr1,ptr2,ptr3,ptr4,x,y,px,py,a,func) do { \
	WU_DRAWCIRCLE_XCOLOR_TOP(ptr1,ptr2,x,y,px,py,a,func); \
	WU_DRAWCIRCLE_XCOLOR_BOTTOM(ptr3,ptr4,x,y,px,py,a,func); \
} while (0)

// Color depending on corner (tl,tr,bl: color1, br: color2)
// Note: this is only for the outer pixels
#define WU_DRAWCIRCLE_BCOLOR(ptr1,ptr2,ptr3,ptr4,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr1 + (y) - (px), color1, a); \
	this->blendPixelPtr(ptr1 + (x) - (py), color1, a); \
	this->blendPixelPtr(ptr2 - (x) - (py), color1, a); \
	this->blendPixelPtr(ptr2 - (y) - (px), color1, a); \
	this->blendPixelPtr(ptr3 - (y) + (px), color1, a); \
	this->blendPixelPtr(ptr3 - (x) + (py), color1, a); \
	this->blendPixelPtr(ptr4 + (x) + (py), color2, a); \
	this->blendPixelPtr(ptr4 + (y) + (px), color2, a); \
} while (0)

#define WU_DRAWCIRCLE_BCOLOR_TR_CW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr + (y) - (px), color, a); \
} while (0)

#define WU_DRAWCIRCLE_BCOLOR_TR_CCW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr + (x) - (py), color, a); \
} while (0)

#define WU_DRAWCIRCLE_BCOLOR_TL_CW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr - (x) - (py), color, a); \
} while (0)

#define WU_DRAWCIRCLE_BCOLOR_TL_CCW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr - (y) - (px), color, a); \
} while (0)

#define WU_DRAWCIRCLE_BCOLOR_BL_CW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr - (y) + (px), color, a); \
} while (0)

#define WU_DRAWCIRCLE_BCOLOR_BL_CCW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr - (x) + (py), color, a); \
} while (0)

#define WU_DRAWCIRCLE_BCOLOR_BR_CW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr + (x) + (py), color, a); \
} while (0)

#define WU_DRAWCIRCLE_BCOLOR_BR_CCW(ptr,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr + (y) + (px), color, a); \
} while (0)

// optimized Wu's algorithm
#define WU_ALGORITHM() do { \
	oldT = T; \
	T = fp_sqroot(rsq - y*y) ^ 0xFFFF; \
	py += pitch; \
	if (T < oldT) { \
		x--; px -= pitch; \
	} \
	a2 = (T >> 8); \
	a1 = ~a2; \
} while (0)


namespace Graphics {

/**
 * Fills several pixels in a row with a given color.
 *
 * This is a replacement function for Common::fill, using an unrolled
 * loop to maximize performance on most architectures.
 * This function may (and should) be overloaded in any child renderers
 * for portable platforms with platform-specific assembly code.
 *
 * This fill operation is extensively used throughout the renderer, so this
 * counts as one of the main bottlenecks. Please replace it with assembly
 * when possible!
 *
 * @param first Pointer to the first pixel to fill.
 * @param last Pointer to the last pixel to fill.
 * @param color Color of the pixel
 */
template<typename PixelType>
void colorFill(PixelType *first, PixelType *last, PixelType color) {
	int count = (last - first);
	if (!count)
		return;
	int n = (count + 7) >> 3;
	switch (count % 8) {
	default:
	case 0:	do {
	       		*first++ = color; // fall through
	case 7:		*first++ = color; // fall through
	case 6:		*first++ = color; // fall through
	case 5:		*first++ = color; // fall through
	case 4:		*first++ = color; // fall through
	case 3:		*first++ = color; // fall through
	case 2:		*first++ = color; // fall through
	case 1:		*first++ = color;
	       	} while (--n > 0);
	}
}

template<typename PixelType>
void colorFillClip(PixelType *first, PixelType *last, PixelType color, int realX, int realY, Common::Rect &clippingArea) {
	if (realY < clippingArea.top || realY >= clippingArea.bottom)
		return;

	int count = (last - first);

	if (realX > clippingArea.right || realX + count < clippingArea.left)
		return;

	if (realX < clippingArea.left) {
		int diff = (clippingArea.left - realX);
		realX += diff;
		first += diff;
		count -= diff;
	}

	if (clippingArea.right <= realX + count) {
		int diff = (realX + count - clippingArea.right);
		count -= diff;
	}

	if (!count)
		return;

	int n = (count + 7) >> 3;
	switch (count % 8) {
	default:
	case 0:	do {
	       		*first++ = color; // fall through
	case 7:		*first++ = color; // fall through
	case 6:		*first++ = color; // fall through
	case 5:		*first++ = color; // fall through
	case 4:		*first++ = color; // fall through
	case 3:		*first++ = color; // fall through
	case 2:		*first++ = color; // fall through
	case 1:		*first++ = color;
	       	} while (--n > 0);
	}
}


VectorRenderer *createRenderer(int mode) {
#ifdef DISABLE_FANCY_THEMES
	assert(mode == GUI::ThemeEngine::kGfxStandard);
#endif

	PixelFormat format = g_system->getOverlayFormat();
	switch (mode) {
	case GUI::ThemeEngine::kGfxStandard:
		if (g_system->getOverlayFormat().bytesPerPixel == 4)
			return new VectorRendererSpec<uint32>(format);
		else if (g_system->getOverlayFormat().bytesPerPixel == 2)
			return new VectorRendererSpec<uint16>(format);
		break;
#ifndef DISABLE_FANCY_THEMES
	case GUI::ThemeEngine::kGfxAntialias:
		if (g_system->getOverlayFormat().bytesPerPixel == 4)
			return new VectorRendererAA<uint32>(format);
		else if (g_system->getOverlayFormat().bytesPerPixel == 2)
			return new VectorRendererAA<uint16>(format);
		break;
#endif
	default:
		break;
	}

	return 0;
}

template<typename PixelType>
VectorRendererSpec<PixelType>::
VectorRendererSpec(PixelFormat format) :
	_format(format),
	_redMask((0xFF >> format.rLoss) << format.rShift),
	_greenMask((0xFF >> format.gLoss) << format.gShift),
	_blueMask((0xFF >> format.bLoss) << format.bShift),
	_alphaMask((0xFF >> format.aLoss) << format.aShift) {

	_bitmapAlphaColor = _format.RGBToColor(255, 0, 255);
	_clippingArea = Common::Rect(0, 0, 32767, 32767);

	_fgColor = _bgColor = _bevelColor = 0;
	_gradientStart = _gradientEnd = 0;
}

/****************************
 * Gradient-related methods *
 ****************************/

template<typename PixelType>
void VectorRendererSpec<PixelType>::
setGradientColors(uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2) {
	_gradientEnd = _format.RGBToColor(r2, g2, b2);
	_gradientStart = _format.RGBToColor(r1, g1, b1);

	if (sizeof(PixelType) == 4) {
		_gradientBytes[0] = ((_gradientEnd & _redMask) >> _format.rShift) - ((_gradientStart & _redMask) >> _format.rShift);
		_gradientBytes[1] = ((_gradientEnd & _greenMask) >> _format.gShift) - ((_gradientStart & _greenMask) >> _format.gShift);
		_gradientBytes[2] = ((_gradientEnd & _blueMask) >> _format.bShift) - ((_gradientStart & _blueMask) >> _format.bShift);
	} else {
		_gradientBytes[0] = (_gradientEnd & _redMask) - (_gradientStart & _redMask);
		_gradientBytes[1] = (_gradientEnd & _greenMask) - (_gradientStart & _greenMask);
		_gradientBytes[2] = (_gradientEnd & _blueMask) - (_gradientStart & _blueMask);
	}
}

template<typename PixelType>
inline PixelType VectorRendererSpec<PixelType>::
calcGradient(uint32 pos, uint32 max) {
	PixelType output = 0;
	pos = (MIN(pos * Base::_gradientFactor, max) << 12) / max;

	if (sizeof(PixelType) == 4) {
		output |= ((_gradientStart & _redMask) + (((_gradientBytes[0] * pos) >> 12) << _format.rShift)) & _redMask;
		output |= ((_gradientStart & _greenMask) + (((_gradientBytes[1] * pos) >> 12) << _format.gShift)) & _greenMask;
		output |= ((_gradientStart & _blueMask) + (((_gradientBytes[2] * pos) >> 12) << _format.bShift)) & _blueMask;
	} else {
		output |= ((_gradientStart & _redMask) + ((_gradientBytes[0] * pos) >> 12)) & _redMask;
		output |= ((_gradientStart & _greenMask) + ((_gradientBytes[1] * pos) >> 12)) & _greenMask;
		output |= ((_gradientStart & _blueMask) + ((_gradientBytes[2] * pos) >> 12)) & _blueMask;
	}
	output |= _alphaMask;

	return output;
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
precalcGradient(int h) {
	PixelType prevcolor = 0, color;

	_gradCache.resize(0);
	_gradIndexes.resize(0);

	for (int i = 0; i < h + 2; i++) {
		color = calcGradient(i, h);
		if (color != prevcolor || i == 0 || i > h - 1) {
			prevcolor = color;
			_gradCache.push_back(color);
			_gradIndexes.push_back(i);
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
gradientFill(PixelType *ptr, int width, int x, int y) {
	bool ox = ((y & 1) == 1);
	int curGrad = 0;

	while (_gradIndexes[curGrad + 1] <= y)
		curGrad++;

	// precalcGradient assures that _gradIndexes entries always differ in
	// their value. This assures stripSize is always different from zero.
	int stripSize = _gradIndexes[curGrad + 1] - _gradIndexes[curGrad];

	int grad = (((y - _gradIndexes[curGrad]) % stripSize) << 2) / stripSize;

	// Dithering:
	//   +--+ +--+ +--+ +--+
	//   |  | |  | | *| | *|
	//   |  | | *| |* | |**|
	//   +--+ +--+ +--+ +--+
	//     0    1    2    3
	if (grad == 0 ||
		_gradCache[curGrad] == _gradCache[curGrad + 1] || // no color change
		stripSize < 2) { // the stip is small
		colorFill<PixelType>(ptr, ptr + width, _gradCache[curGrad]);
	} else if (grad == 3 && ox) {
		colorFill<PixelType>(ptr, ptr + width, _gradCache[curGrad + 1]);
	} else {
		for (int j = x; j < x + width; j++, ptr++) {
			bool oy = ((j & 1) == 1);

			if ((ox && oy) ||
				((grad == 2 || grad == 3) && ox && !oy) ||
				(grad == 3 && oy))
				*ptr = _gradCache[curGrad + 1];
			else
				*ptr = _gradCache[curGrad];
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
gradientFillClip(PixelType *ptr, int width, int x, int y, int realX, int realY) {
	if (realY < _clippingArea.top || realY >= _clippingArea.bottom) return;
	bool ox = ((y & 1) == 1);
	int curGrad = 0;

	while (_gradIndexes[curGrad + 1] <= y)
		curGrad++;

	// precalcGradient assures that _gradIndexes entries always differ in
	// their value. This assures stripSize is always different from zero.
	int stripSize = _gradIndexes[curGrad + 1] - _gradIndexes[curGrad];

	int grad = (((y - _gradIndexes[curGrad]) % stripSize) << 2) / stripSize;

	// Dithering:
	//   +--+ +--+ +--+ +--+
	//   |  | |  | | *| | *|
	//   |  | | *| |* | |**|
	//   +--+ +--+ +--+ +--+
	//     0    1    2    3
	if (grad == 0 ||
		_gradCache[curGrad] == _gradCache[curGrad + 1] || // no color change
		stripSize < 2) { // the stip is small
		colorFillClip<PixelType>(ptr, ptr + width, _gradCache[curGrad], realX, realY, _clippingArea);
	} else if (grad == 3 && ox) {
		colorFillClip<PixelType>(ptr, ptr + width, _gradCache[curGrad + 1], realX, realY, _clippingArea);
	} else {
		for (int j = x; j < x + width; j++, ptr++) {
			if (realX + j - x < _clippingArea.left || realX + j - x >= _clippingArea.right) continue;
			bool oy = ((j & 1) == 1);

			if ((ox && oy) ||
				((grad == 2 || grad == 3) && ox && !oy) ||
				(grad == 3 && oy))
				*ptr = _gradCache[curGrad + 1];
			else
				*ptr = _gradCache[curGrad];
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
fillSurface() {
	Common::Rect drawRect(0, 0, _activeSurface->w, _activeSurface->h);
	drawRect.clip(_clippingArea);

	if (drawRect.isEmpty()) {
		return;
	}

	int h = _activeSurface->h;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(0, drawRect.top);

	if (Base::_fillMode == kFillBackground || Base::_fillMode == kFillForeground) {
		PixelType color = (Base::_fillMode == kFillBackground ? _bgColor : _fgColor);
		PixelType *ptrLeft = (ptr + drawRect.left), *ptrRight = ptr + drawRect.right;
		for (int i = drawRect.top; i < drawRect.bottom; i++) {
			colorFill<PixelType>(ptrLeft, ptrRight, color);

			ptrLeft += pitch;
			ptrRight += pitch;
		}

	} else if (Base::_fillMode == kFillGradient) {
		precalcGradient(h);

		for (int i = drawRect.top; i < drawRect.bottom; i++) {
			gradientFill(ptr + drawRect.left, drawRect.width(), 0, i);

			ptr += pitch;
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
copyFrame(OSystem *sys, const Common::Rect &r) {

	sys->copyRectToOverlay(
		_activeSurface->getBasePtr(r.left, r.top),
		_activeSurface->pitch,
	    r.left, r.top, r.width(), r.height()
	);
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
blitSurface(const Graphics::Surface *source, const Common::Rect &r) {
	assert(source->w == _activeSurface->w && source->h == _activeSurface->h);

	byte *dst_ptr = (byte *)_activeSurface->getBasePtr(r.left, r.top);
	const byte *src_ptr = (const byte *)source->getBasePtr(r.left, r.top);

	const int dst_pitch = _activeSurface->pitch;
	const int src_pitch = source->pitch;

	int h = r.height();
	const int w = r.width() * sizeof(PixelType);

	while (h--) {
		memcpy(dst_ptr, src_ptr, w);
		dst_ptr += dst_pitch;
		src_ptr += src_pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
blitSubSurface(const Graphics::Surface *source, const Common::Point &p) {
	Common::Rect drawRect(p.x, p.y, p.x + source->w, p.y + source->h);
	drawRect.clip(_clippingArea);

	if (drawRect.isEmpty()) {
		return;
	}

	int sourceOffsetX = drawRect.left - p.x;
	int sourceOffsetY = drawRect.top - p.y;

	byte *dst_ptr = (byte *)_activeSurface->getBasePtr(drawRect.left, drawRect.top);
	const byte *src_ptr = (const byte *)source->getBasePtr(sourceOffsetX, sourceOffsetY);

	const int dst_pitch = _activeSurface->pitch;
	const int src_pitch = source->pitch;

	int lines = drawRect.height();
	const int sz = drawRect.width() * sizeof(PixelType);

	while (lines--) {
		memcpy(dst_ptr, src_ptr, sz);
		dst_ptr += dst_pitch;
		src_ptr += src_pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
blitKeyBitmap(const Graphics::Surface *source, const Common::Point &p) {
	Common::Rect drawRect(p.x, p.y, p.x + source->w, p.y + source->h);
	drawRect.clip(_clippingArea);

	if (drawRect.isEmpty()) {
		return;
	}

	int sourceOffsetX = drawRect.left - p.x;
	int sourceOffsetY = drawRect.top - p.y;

	PixelType *dst_ptr = (PixelType *)_activeSurface->getBasePtr(drawRect.left, drawRect.top);
	const PixelType *src_ptr = (const PixelType *)source->getBasePtr(sourceOffsetX, sourceOffsetY);

	int dst_pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int src_pitch = source->pitch / source->format.bytesPerPixel;

	int w, h = drawRect.height();

	while (h--) {
		w = drawRect.width();

		while (w--) {
			if (*src_ptr != _bitmapAlphaColor)
				*dst_ptr = *src_ptr;

			dst_ptr++;
			src_ptr++;
		}

		dst_ptr = dst_ptr - drawRect.width() + dst_pitch;
		src_ptr = src_ptr - drawRect.width() + src_pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
blitAlphaBitmap(Graphics::TransparentSurface *source, const Common::Rect &r, GUI::ThemeEngine::AutoScaleMode autoscale,
			Graphics::DrawStep::VectorAlignment xAlign, Graphics::DrawStep::VectorAlignment yAlign, int alpha) {
	if (autoscale == GUI::ThemeEngine::kAutoScaleStretch) {
		source->blit(*_activeSurface, r.left, r.top, Graphics::FLIP_NONE,
			nullptr, TS_ARGB(alpha, 255, 255, 255),
			  r.width(), r.height());
	} else if (autoscale == GUI::ThemeEngine::kAutoScaleFit) {
		double ratio = (double)r.width() / source->w;
		double ratio2 = (double)r.height() / source->h;

		if (ratio2 < ratio)
			ratio = ratio2;

		int offx = 0, offy = 0;
		if (xAlign == Graphics::DrawStep::kVectorAlignCenter)
			offx = (r.width() - (int)(source->w * ratio)) >> 1;

		if (yAlign == Graphics::DrawStep::kVectorAlignCenter)
			offy = (r.height() - (int)(source->h * ratio)) >> 1;

		source->blit(*_activeSurface, r.left + offx, r.top + offy, Graphics::FLIP_NONE,
			nullptr, TS_ARGB(alpha, 255, 255, 255),
	                  (int)(source->w * ratio), (int)(source->h * ratio));

	} else if (autoscale == GUI::ThemeEngine::kAutoScaleNinePatch) {
		Graphics::NinePatchBitmap nine(source, false);
		nine.blit(*_activeSurface, r.left, r.top, r.width(), r.height());
	} else {
		source->blit(*_activeSurface, r.left, r.top);
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
applyScreenShading(GUI::ThemeEngine::ShadingStyle shadingStyle) {
	int pixels = _activeSurface->w * _activeSurface->h;
	PixelType *ptr = (PixelType *)_activeSurface->getPixels();
	uint8 r, g, b;
	uint lum;

	// Mask to clear the last bit of every color component and all unused bits
	const uint32 colorMask = ~((1 << _format.rShift) | (1 << _format.gShift) | (1 << _format.bShift) // R/G/B components
			| (_format.aLoss == 8 ? 0 : (1 << _format.aShift)) // Alpha component
			| ~(_alphaMask | _redMask | _greenMask | _blueMask)); // All unused bits

	if (shadingStyle == GUI::ThemeEngine::kShadingDim) {

		// TODO: Check how this interacts with kFeatureOverlaySupportsAlpha
		for (int i = 0; i < pixels; ++i) {
			*ptr = ((*ptr & colorMask) >> 1) | _alphaMask;
			++ptr;
		}

	} else if (shadingStyle == GUI::ThemeEngine::kShadingLuminance) {
		while (pixels--) {
			_format.colorToRGB(*ptr, r, g, b);
			lum = (r >> 2) + (g >> 1) + (b >> 3);
			*ptr++ = _format.RGBToColor(lum, lum, lum);
		}
	}
}

template<typename PixelType>
inline void VectorRendererSpec<PixelType>::
blendPixelPtr(PixelType *ptr, PixelType color, uint8 alpha) {
	if (alpha == 0xff) {
		// fully opaque pixel, don't blend
		*ptr = color | _alphaMask;
	} else if (sizeof(PixelType) == 4) {
		const byte sR = (color & _redMask) >> _format.rShift;
		const byte sG = (color & _greenMask) >> _format.gShift;
		const byte sB = (color & _blueMask) >> _format.bShift;

		byte dR = (*ptr & _redMask) >> _format.rShift;
		byte dG = (*ptr & _greenMask) >> _format.gShift;
		byte dB = (*ptr & _blueMask) >> _format.bShift;
		byte dA = (*ptr & _alphaMask) >> _format.aShift;

		dR += ((sR - dR) * alpha) >> 8;
		dG += ((sG - dG) * alpha) >> 8;
		dB += ((sB - dB) * alpha) >> 8;
		dA += ((0xff - dA) * alpha) >> 8;

		*ptr = ((dR << _format.rShift) & _redMask)
		     | ((dG << _format.gShift) & _greenMask)
		     | ((dB << _format.bShift) & _blueMask)
		     | ((dA << _format.aShift) & _alphaMask);
	} else if (sizeof(PixelType) == 2) {
		int idst = *ptr;
		int isrc = color;

		*ptr = (PixelType)(
			(_redMask & ((idst & _redMask) +
			((int)(((int)(isrc & _redMask) -
			(int)(idst & _redMask)) * alpha) >> 8))) |
			(_greenMask & ((idst & _greenMask) +
			((int)(((int)(isrc & _greenMask) -
			(int)(idst & _greenMask)) * alpha) >> 8))) |
			(_blueMask & ((idst & _blueMask) +
			((int)(((int)(isrc & _blueMask) -
			(int)(idst & _blueMask)) * alpha) >> 8))) |
			(_alphaMask & ((idst & _alphaMask) +
			((int)(((int)(_alphaMask) -
			(int)(idst & _alphaMask)) * alpha) >> 8))));
	} else {
		error("Unsupported BPP format: %u", (uint)sizeof(PixelType));
	}
}

template<typename PixelType>
inline void VectorRendererSpec<PixelType>::
blendPixelPtrClip(PixelType *ptr, PixelType color, uint8 alpha, int x, int y) {
	if (IS_IN_CLIP(x, y))
		blendPixelPtr(ptr, color, alpha);
}

template<typename PixelType>
inline void VectorRendererSpec<PixelType>::
blendPixelDestAlphaPtr(PixelType *ptr, PixelType color, uint8 alpha) {
	int idst = *ptr;
	// This function is only used for corner pixels in rounded rectangles, so
	// the performance hit of this if shouldn't be too high.
	// We're also ignoring the cases where dst has intermediate alpha.
	if ((idst & _alphaMask) == 0) {
		// set color and alpha channels
		*ptr = (PixelType)(color & (_redMask | _greenMask | _blueMask)) |
		                  ((alpha >> _format.aLoss) << _format.aShift);
	} else {
		// blend color with background
		blendPixelPtr(ptr, color, alpha);
	}
}

template<typename PixelType>
inline void VectorRendererSpec<PixelType>::
darkenFill(PixelType *ptr, PixelType *end) {
	PixelType mask = (PixelType)((3 << _format.rShift) | (3 << _format.gShift) | (3 << _format.bShift));

	if (!g_system->hasFeature(OSystem::kFeatureOverlaySupportsAlpha)) {
		// !kFeatureOverlaySupportsAlpha (but might have alpha bits)

		while (ptr != end) {
			*ptr = ((*ptr & ~mask) >> 2) | _alphaMask;
			++ptr;
		}
	} else {
		// kFeatureOverlaySupportsAlpha
		// assuming at least 3 alpha bits

		mask |= 3 << _format.aShift;
		PixelType addA = (PixelType)(3 << (_format.aShift + 6 - _format.aLoss));

		while (ptr != end) {
			// Darken the color, and increase the alpha
			// (0% -> 75%, 100% -> 100%)
			*ptr = (PixelType)(((*ptr & ~mask) >> 2) + addA);
			++ptr;
		}
	}
}

template<typename PixelType>
inline void VectorRendererSpec<PixelType>::
darkenFillClip(PixelType *ptr, PixelType *end, int x, int y) {
	PixelType mask = (PixelType)((3 << _format.rShift) | (3 << _format.gShift) | (3 << _format.bShift));

	if (!g_system->hasFeature(OSystem::kFeatureOverlaySupportsAlpha)) {
		// !kFeatureOverlaySupportsAlpha (but might have alpha bits)

		while (ptr != end) {
			if (IS_IN_CLIP(x, y)) *ptr = ((*ptr & ~mask) >> 2) | _alphaMask;
			++ptr;
			++x;
		}
	} else {
		// kFeatureOverlaySupportsAlpha
		// assuming at least 3 alpha bits

		mask |= 3 << _format.aShift;
		PixelType addA = (PixelType)(3 << (_format.aShift + 6 - _format.aLoss));

		while (ptr != end) {
			// Darken the color, and increase the alpha
			// (0% -> 75%, 100% -> 100%)
			if (IS_IN_CLIP(x, y)) *ptr = (PixelType)(((*ptr & ~mask) >> 2) + addA);
			++ptr;
			++x;
		}
	}
}

/********************************************************************
 ********************************************************************
 * Primitive shapes drawing - Public API calls - VectorRendererSpec *
 ********************************************************************
 ********************************************************************/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawString(const Graphics::Font *font, const Common::U32String &text, const Common::Rect &area,
			Graphics::TextAlign alignH, GUI::ThemeEngine::TextAlignVertical alignV, int deltax, bool ellipsis, const Common::Rect &textDrawableArea) {

	int offset = area.top;

	if (font->getFontHeight() < area.height()) {
		switch (alignV) {
		case GUI::ThemeEngine::kTextAlignVCenter:
			offset = area.top + ((area.height() - font->getFontHeight()) >> 1);
			break;
		case GUI::ThemeEngine::kTextAlignVBottom:
			offset = area.bottom - font->getFontHeight();
			break;
		default:
			break;
		}
	}

	Common::Rect drawArea;
	if (textDrawableArea.isEmpty()) {
		// In case no special area to draw to is given we only draw in the
		// area specified by the user.
		drawArea = area;
		// warning("there is no text drawable area. Please set this area for clipping");
	} else {
		// The area we can draw to is the intersection between the allowed
		// drawing area (textDrawableArea) and the area where we try to draw
		// the text (area).
		drawArea = textDrawableArea.findIntersectingRect(area);
	}

	// Better safe than sorry. We intersect with the actual surface boundaries
	// to avoid any ugly clipping in _activeSurface->getSubArea which messes
	// up the calculation of the x and y coordinates where to draw the string.
	drawArea = drawArea.findIntersectingRect(Common::Rect(0, 0, _activeSurface->w, _activeSurface->h));

	if (!drawArea.isEmpty()) {
		Common::Rect textArea(area);
		textArea.right -= deltax;

		Surface textAreaSurface = _activeSurface->getSubArea(drawArea);

		if (deltax >= 0) {
			textArea.left += deltax;
			deltax = 0;
		}

		font->drawString(&textAreaSurface, text, textArea.left - drawArea.left, offset - drawArea.top, textArea.width(), _fgColor, alignH, deltax, ellipsis);
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
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

	uint dx = ABS(x2 - x1);
	uint dy = ABS(y2 - y1);

	// this is a point, not a line. stoopid.
	if (dy == 0 && dx == 0)
		return;

	if (Base::_strokeWidth == 0)
		return;

	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int st = Base::_strokeWidth >> 1;

	bool useClippingVersions = !_clippingArea.contains(x1, y1) || !_clippingArea.contains(x2, y2);

	int ptr_x = x1, ptr_y = y1;

	if (dy == 0) { // horizontal lines
		if (useClippingVersions) {
			colorFillClip<PixelType>(ptr, ptr + dx + 1, (PixelType)_fgColor, x1, y1, _clippingArea);
		} else {
			colorFill<PixelType>(ptr, ptr + dx + 1, (PixelType)_fgColor);
		}

		for (int i = 0, p = pitch; i < st; ++i, p += pitch) {
			if (useClippingVersions) {
				colorFillClip<PixelType>(ptr + p, ptr + dx + 1 + p, (PixelType)_fgColor, x1, y1 + p/pitch, _clippingArea);
				colorFillClip<PixelType>(ptr - p, ptr + dx + 1 - p, (PixelType)_fgColor, x1, y1 - p/pitch, _clippingArea);
			} else {
				colorFill<PixelType>(ptr + p, ptr + dx + 1 + p, (PixelType)_fgColor);
				colorFill<PixelType>(ptr - p, ptr + dx + 1 - p, (PixelType)_fgColor);
			}
		}

	} else if (dx == 0) { // vertical lines
						  // these ones use a static pitch increase.
		while (y1++ <= y2) {
			if (useClippingVersions) {
				colorFillClip<PixelType>(ptr - st, ptr + st, (PixelType)_fgColor, x1 - st, ptr_y, _clippingArea);
			} else {
				colorFill<PixelType>(ptr - st, ptr + st, (PixelType)_fgColor);
			}
			ptr += pitch;
			++ptr_y;
		}

	} else if (dx == dy) { // diagonal lines
						   // these ones also use a fixed pitch increase
		pitch += (x2 > x1) ? 1 : -1;

		while (dy--) {
			if (useClippingVersions) {
				colorFillClip<PixelType>(ptr - st, ptr + st, (PixelType)_fgColor, ptr_x - st, ptr_y, _clippingArea);
			} else {
				colorFill<PixelType>(ptr - st, ptr + st, (PixelType)_fgColor);
			}
			ptr += pitch;
			++ptr_y;
			if (x2 > x1) ++ptr_x; else --ptr_x;
		}

	} else { // generic lines, use the standard algorithm...
		if (useClippingVersions) {
			drawLineAlgClip(x1, y1, x2, y2, dx, dy, (PixelType)_fgColor);
		} else {
			drawLineAlg(x1, y1, x2, y2, dx, dy, (PixelType)_fgColor);
		}
	}
}

/** CIRCLES **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawCircle(int x, int y, int r) {
	if (x + r > Base::_activeSurface->w || y + r > Base::_activeSurface->h ||
		x - r < 0 || y - r < 0 || x == 0 || y == 0 || r <= 0)
		return;

	bool useClippingVersions = !_clippingArea.contains(Common::Rect(x - r, y - r, x + r, y + r));

	if (Base::_fillMode != kFillDisabled && Base::_shadowOffset
		&& x + r + Base::_shadowOffset < Base::_activeSurface->w
		&& y + r + Base::_shadowOffset < Base::_activeSurface->h) {
		if (useClippingVersions)
			drawCircleAlgClip(x + Base::_shadowOffset + 1, y + Base::_shadowOffset + 1, r, 0, kFillForeground);
		else
			drawCircleAlg(x + Base::_shadowOffset + 1, y + Base::_shadowOffset + 1, r, 0, kFillForeground);
	}

	switch (Base::_fillMode) {
	case kFillDisabled:
		if (Base::_strokeWidth) {
			if (useClippingVersions)
				drawCircleAlgClip(x, y, r, _fgColor, kFillDisabled);
			else
				drawCircleAlg(x, y, r, _fgColor, kFillDisabled);
		}
		break;

	case kFillForeground:
		if (useClippingVersions)
			drawCircleAlgClip(x, y, r, _fgColor, kFillForeground);
		else
			drawCircleAlg(x, y, r, _fgColor, kFillForeground);
		break;

	case kFillBackground:
		if (Base::_strokeWidth > 1) {
			if (useClippingVersions) {
				drawCircleAlgClip(x, y, r, _fgColor, kFillForeground);
				drawCircleAlgClip(x, y, r - Base::_strokeWidth, _bgColor, kFillBackground);
			} else {
				drawCircleAlg(x, y, r, _fgColor, kFillForeground);
				drawCircleAlg(x, y, r - Base::_strokeWidth, _bgColor, kFillBackground);
			}
		} else {
			if (useClippingVersions) {
				drawCircleAlgClip(x, y, r, _bgColor, kFillBackground);
				drawCircleAlgClip(x, y, r, _fgColor, kFillDisabled);
			} else {
				drawCircleAlg(x, y, r, _bgColor, kFillBackground);
				drawCircleAlg(x, y, r, _fgColor, kFillDisabled);
			}
		}
		break;

	case kFillGradient:
		break;

	default:
		break;
	}
}

/** SQUARES **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawSquare(int x, int y, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0)
		return;

	bool useClippingVersions = !_clippingArea.contains(Common::Rect(x, y, x + w, y + h));

	if (Base::_fillMode != kFillDisabled && Base::_shadowOffset
		&& x + w + Base::_shadowOffset < Base::_activeSurface->w
		&& y + h + Base::_shadowOffset < Base::_activeSurface->h) {
		if (useClippingVersions)
			drawSquareShadowClip(x, y, w, h, Base::_shadowOffset);
		else
			drawSquareShadow(x, y, w, h, Base::_shadowOffset);
	}

	switch (Base::_fillMode) {
	case kFillDisabled:
		if (Base::_strokeWidth) {
			if (useClippingVersions)
				drawSquareAlgClip(x, y, w, h, _fgColor, kFillDisabled);
			else
				drawSquareAlg(x, y, w, h, _fgColor, kFillDisabled);
		}
		break;

	case kFillForeground:
		if (useClippingVersions)
			drawSquareAlgClip(x, y, w, h, _fgColor, kFillForeground);
		else
			drawSquareAlg(x, y, w, h, _fgColor, kFillForeground);
		break;

	case kFillBackground:
		if (useClippingVersions) {
			drawSquareAlgClip(x, y, w, h, _bgColor, kFillBackground);
			drawSquareAlgClip(x, y, w, h, _fgColor, kFillDisabled);
		} else {
			drawSquareAlg(x, y, w, h, _bgColor, kFillBackground);
			drawSquareAlg(x, y, w, h, _fgColor, kFillDisabled);
		}
		break;

	case kFillGradient:
		VectorRendererSpec::drawSquareAlg(x, y, w, h, 0, kFillGradient);
		if (Base::_strokeWidth) {
			if (useClippingVersions)
				drawSquareAlgClip(x, y, w, h, _fgColor, kFillDisabled);
			else
				drawSquareAlg(x, y, w, h, _fgColor, kFillDisabled);
		}
		break;

	default:
		break;
	}
}

/** ROUNDED SQUARES **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawRoundedSquare(int x, int y, int r, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0 || r <= 0)
		return;

	if ((r * 2) > w || (r * 2) > h)
		r = MIN(w / 2, h / 2);

	if (r <= 0)
		return;

	bool useOriginal = _clippingArea.contains(Common::Rect(x, y, x + w, y + h));

	if (Base::_fillMode != kFillDisabled && Base::_shadowOffset
		&& x + w + Base::_shadowOffset + 1 < Base::_activeSurface->w
		&& y + h + Base::_shadowOffset + 1 < Base::_activeSurface->h
		&& h > (Base::_shadowOffset + 1) * 2) {
		if (useOriginal) {
			drawRoundedSquareShadow(x, y, r, w, h, Base::_shadowOffset);
		} else {
			drawRoundedSquareShadowClip(x, y, r, w, h, Base::_shadowOffset);
		}
	}

	if (useOriginal) {
		drawRoundedSquareAlg(x, y, r, w, h, _fgColor, Base::_fillMode);
	} else {
		drawRoundedSquareAlgClip(x, y, r, w, h, _fgColor, Base::_fillMode);
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTab(int x, int y, int r, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0 || r > w || r > h)
		return;

	bool useClippingVersions = !_clippingArea.contains(Common::Rect(x, y, x + w, y + h));

	if (r == 0 && Base::_bevel > 0) {
		if (useClippingVersions)
			drawBevelTabAlgClip(x, y, w, h, Base::_bevel, _bevelColor, _fgColor, (Base::_dynamicData >> 16), (Base::_dynamicData & 0xFFFF));
		else
			drawBevelTabAlg(x, y, w, h, Base::_bevel, _bevelColor, _fgColor, (Base::_dynamicData >> 16), (Base::_dynamicData & 0xFFFF));
		return;
	}

	if (r == 0) {
		return;
	}

	switch (Base::_fillMode) {
	case kFillDisabled:
		// FIXME: Implement this
		return;

	case kFillGradient:
	case kFillBackground:
		// FIXME: This is broken for the AA renderer.
		// See the rounded rect alg for how to fix it. (The border should
		// be drawn before the interior, both inside drawTabAlg.)
		if (useClippingVersions) {
			drawTabShadowClip(x, y, w - 2, h, r);
			drawTabAlgClip(x, y, w - 2, h, r, _bgColor, Base::_fillMode);
			if (Base::_strokeWidth)
				drawTabAlgClip(x, y, w, h, r, _fgColor, kFillDisabled, (Base::_dynamicData >> 16), (Base::_dynamicData & 0xFFFF));
		} else {
			drawTabShadow(x, y, w - 2, h, r);
			drawTabAlg(x, y, w - 2, h, r, _bgColor, Base::_fillMode);
			if (Base::_strokeWidth)
				drawTabAlg(x, y, w, h, r, _fgColor, kFillDisabled, (Base::_dynamicData >> 16), (Base::_dynamicData & 0xFFFF));
		}
		break;

	case kFillForeground:
		if (useClippingVersions)
			drawTabAlgClip(x, y, w, h, r, _fgColor, Base::_fillMode);
		else
			drawTabAlg(x, y, w, h, r, _fgColor, Base::_fillMode);
		break;

	default:
		break;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
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

	bool useClippingVersions = !_clippingArea.contains(Common::Rect(x, y, x + w, y + h));

	if (w == h) {
		int newW = w;

		switch (orient) {
		case kTriangleUp:
		case kTriangleDown:
			if (useClippingVersions)
				drawTriangleVertAlgClip(x, y, newW, newW, (orient == kTriangleDown), color, Base::_fillMode);
			else
				drawTriangleVertAlg(x, y, newW, newW, (orient == kTriangleDown), color, Base::_fillMode);
			break;

		case kTriangleLeft:
		case kTriangleRight:
		case kTriangleAuto:
		default:
			break;
		}

		if (Base::_strokeWidth > 0)
			if (Base::_fillMode == kFillBackground || Base::_fillMode == kFillGradient) {
				if (useClippingVersions)
					drawTriangleVertAlgClip(x, y, newW, newW, (orient == kTriangleDown), color, Base::_fillMode);
				else
					drawTriangleVertAlg(x, y, newW, newW, (orient == kTriangleDown), color, Base::_fillMode);
			}
	} else {
		int newW = w;
		int newH = h;

		switch (orient) {
		case kTriangleUp:
		case kTriangleDown:
			if (useClippingVersions)
				drawTriangleVertAlgClip(x, y, newW, newH, (orient == kTriangleDown), color, Base::_fillMode);
			else
				drawTriangleVertAlg(x, y, newW, newH, (orient == kTriangleDown), color, Base::_fillMode);
			break;

		case kTriangleLeft:
		case kTriangleRight:
		case kTriangleAuto:
		default:
			break;
		}

		if (Base::_strokeWidth > 0) {
			if (Base::_fillMode == kFillBackground || Base::_fillMode == kFillGradient) {
				if (useClippingVersions)
					drawTriangleVertAlgClip(x, y, newW, newH, (orient == kTriangleDown), _fgColor, kFillDisabled);
				else
					drawTriangleVertAlg(x, y, newW, newH, (orient == kTriangleDown), _fgColor, kFillDisabled);
			}
		}
	}
}


/********************************************************************
 ********************************************************************
 * Aliased Primitive drawing ALGORITHMS - VectorRendererSpec
 ********************************************************************
 ********************************************************************/
/** TAB ALGORITHM - NON AA */
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTabAlg(int x1, int y1, int w, int h, int r, PixelType color, VectorRenderer::FillMode fill_m, int baseLeft, int baseRight) {
	// Don't draw anything for empty rects.
	if (w <= 0 || h <= 0) {
		return;
	}

	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int sw  = 0, sp = 0, hp = 0;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;
	int short_h = h - r;
	int long_h = h;

	if (fill_m == kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			colorFill<PixelType>(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
			colorFill<PixelType>(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color);
			sp += pitch;

			BE_RESET();
			r--;

			while (x++ < y) {
				BE_ALGORITHM();
				BE_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px, py);

				if (Base::_strokeWidth > 1)
					BE_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px - pitch, py);
			}
		}

		ptr_fill += pitch * real_radius;
		while (short_h--) {
			colorFill<PixelType>(ptr_fill, ptr_fill + Base::_strokeWidth, color);
			colorFill<PixelType>(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}

		if (baseLeft) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1 + h + 1);
			while (sw++ < Base::_strokeWidth) {
				colorFill<PixelType>(ptr_fill - baseLeft, ptr_fill, color);
				ptr_fill += pitch;
			}
		}

		if (baseRight) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w, y1 + h + 1);
			while (sw++ < Base::_strokeWidth) {
				colorFill<PixelType>(ptr_fill, ptr_fill + baseRight, color);
				ptr_fill += pitch;
			}
		}
	} else {
		BE_RESET();

		precalcGradient(long_h);

		PixelType color1, color2;
		color1 = color2 = color;

		while (x++ < y) {
			BE_ALGORITHM();

			if (fill_m == kFillGradient) {
				color1 = calcGradient(real_radius - x, long_h);
				color2 = calcGradient(real_radius - y, long_h);

				gradientFill(ptr_tl - x - py, w - 2 * r + 2 * x, x1 + r - x - y, real_radius - y);
				gradientFill(ptr_tl - y - px, w - 2 * r + 2 * y, x1 + r - y - x, real_radius - x);

				BE_DRAWCIRCLE_XCOLOR_TOP(ptr_tr, ptr_tl, x, y, px, py);
			} else {
				colorFill<PixelType>(ptr_tl - x - py, ptr_tr + x - py, color);
				colorFill<PixelType>(ptr_tl - y - px, ptr_tr + y - px, color);

				BE_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px, py);
			}
		}

		ptr_fill += pitch * r;
		while (short_h--) {
			if (fill_m == kFillGradient) {
				gradientFill(ptr_fill, w + 1, x1, real_radius++);
			} else {
				colorFill<PixelType>(ptr_fill, ptr_fill + w + 1, color);
			}
			ptr_fill += pitch;
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTabAlgClip(int x1, int y1, int w, int h, int r, PixelType color, VectorRenderer::FillMode fill_m, int baseLeft, int baseRight) {
	// Don't draw anything for empty rects.
	if (w <= 0 || h <= 0) {
		return;
	}

	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int sw = 0, sp = 0, hp = 0;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);
	int tl_x = x1 + r, tl_y = y1 + r;
	int tr_x = x1 + w - r, tr_y = y1 + r;
	int fill_x = x1, fill_y = y1;

	int real_radius = r;
	int short_h = h - r;
	int long_h = h;

	if (fill_m == kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			colorFillClip<PixelType>(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color, fill_x + r, fill_y + sp/pitch, _clippingArea);
			colorFillClip<PixelType>(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color, fill_x + r, fill_y + hp / pitch - sp / pitch, _clippingArea);
			sp += pitch;

			BE_RESET();
			r--;

			while (x++ < y) {
				BE_ALGORITHM();
				BE_DRAWCIRCLE_TOP_CLIP(ptr_tr, ptr_tl, x, y, px, py, tr_x, tr_y, tl_x, tl_y);

				if (Base::_strokeWidth > 1)
					BE_DRAWCIRCLE_TOP_CLIP(ptr_tr, ptr_tl, x, y, px - pitch, py, tr_x, tr_y, tl_x, tl_y);
			}
		}

		ptr_fill += pitch * real_radius;
		fill_y += real_radius;
		while (short_h--) {
			colorFillClip<PixelType>(ptr_fill, ptr_fill + Base::_strokeWidth, color, fill_x, fill_y, _clippingArea);
			colorFillClip<PixelType>(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color, fill_x + w - Base::_strokeWidth + 1, fill_y, _clippingArea);
			ptr_fill += pitch;
			++fill_y;
		}

		if (baseLeft) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1 + h + 1);
			fill_x = x1;
			fill_y = y1 + h + 1;
			while (sw++ < Base::_strokeWidth) {
				colorFillClip<PixelType>(ptr_fill - baseLeft, ptr_fill, color, fill_x - baseLeft, fill_y, _clippingArea);
				ptr_fill += pitch;
				++fill_y;
			}
		}

		if (baseRight) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w, y1 + h + 1);
			fill_x = x1 + w;
			fill_y = y1 + h + 1;
			while (sw++ < Base::_strokeWidth) {
				colorFillClip<PixelType>(ptr_fill, ptr_fill + baseRight, color, fill_x, fill_y, _clippingArea);
				ptr_fill += pitch;
				++fill_y;
			}
		}
	} else {
		BE_RESET();

		precalcGradient(long_h);

		PixelType color1, color2;
		color1 = color2 = color;

		while (x++ < y) {
			BE_ALGORITHM();

			if (fill_m == kFillGradient) {
				color1 = calcGradient(real_radius - x, long_h);
				color2 = calcGradient(real_radius - y, long_h);

				gradientFillClip(ptr_tl - x - py, w - 2 * r + 2 * x, x1 + r - x - y, real_radius - y, tl_x - x, tl_y - y);
				gradientFillClip(ptr_tl - y - px, w - 2 * r + 2 * y, x1 + r - y - x, real_radius - x, tl_x - y, tl_y - x);

				BE_DRAWCIRCLE_XCOLOR_TOP_CLIP(ptr_tr, ptr_tl, x, y, px, py, tr_x, tr_y, tl_x, tl_y);
			} else {
				colorFillClip<PixelType>(ptr_tl - x - py, ptr_tr + x - py, color, tl_x - x, tl_y - y, _clippingArea);
				colorFillClip<PixelType>(ptr_tl - y - px, ptr_tr + y - px, color, tl_x - y, tl_y - x, _clippingArea);

				BE_DRAWCIRCLE_TOP_CLIP(ptr_tr, ptr_tl, x, y, px, py, tr_x, tr_y, tl_x, tl_y);
			}
		}

		ptr_fill += pitch * r;
		fill_y += r;
		while (short_h--) {
			if (fill_m == kFillGradient) {
				gradientFillClip(ptr_fill, w + 1, x1, real_radius++, fill_x, fill_y);
			} else {
				colorFillClip<PixelType>(ptr_fill, ptr_fill + w + 1, color, fill_x, fill_y, _clippingArea);
			}
			ptr_fill += pitch;
			++fill_y;
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTabShadow(int x1, int y1, int w, int h, int r) {
	int offset = 3;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	// "Harder" shadows when having lower BPP, since we will have artifacts (greenish tint on the modern theme)
	uint8 expFactor = 3;
	uint16 alpha = (_activeSurface->format.bytesPerPixel > 2) ? 4 : 8;

	int xstart = x1;
	int ystart = y1;
	int width = w;
	int height = h + offset + 1;

	for (int i = offset; i >= 0; i--) {
		int f, ddF_x, ddF_y;
		int x, y, px, py;

		PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(xstart + r, ystart + r);
		PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(xstart + width - r, ystart + r);
		PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(xstart, ystart);

		int short_h = height - (2 * r);
		PixelType color = _format.RGBToColor(0, 0, 0);

		BE_RESET();

		// HACK: As we are drawing circles exploting 8-axis symmetry,
		// there are 4 pixels on each circle which are drawn twice.
		// this is ok on filled circles, but when blending on surfaces,
		// we cannot let it blend twice. awful.
		uint32 hb = 0;

		while (x++ < y) {
			BE_ALGORITHM();

			if (((1 << x) & hb) == 0) {
				blendFill(ptr_tl - y - px, ptr_tr + y - px, color, (uint8)alpha);
				hb |= (1 << x);
			}

			if (((1 << y) & hb) == 0) {
				blendFill(ptr_tl - x - py, ptr_tr + x - py, color, (uint8)alpha);
				hb |= (1 << y);
			}
		}

		ptr_fill += pitch * r;
		while (short_h--) {
			blendFill(ptr_fill, ptr_fill + width + 1, color, (uint8)alpha);
			ptr_fill += pitch;
		}

		// Move shadow one pixel upward each iteration
		xstart += 1;
		// Multiply with expfactor
		alpha = (alpha * (expFactor << 8)) >> 9;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTabShadowClip(int x1, int y1, int w, int h, int r) {
	int offset = 3;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	// "Harder" shadows when having lower BPP, since we will have artifacts (greenish tint on the modern theme)
	uint8 expFactor = 3;
	uint16 alpha = (_activeSurface->format.bytesPerPixel > 2) ? 4 : 8;

	int xstart = x1;
	int ystart = y1;
	int width = w;
	int height = h + offset + 1;

	for (int i = offset; i >= 0; i--) {
		int f, ddF_x, ddF_y;
		int x, y, px, py;

		PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(xstart + r, ystart + r);
		PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(xstart + width - r, ystart + r);
		PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(xstart, ystart);

		int tl_x = xstart + r, tl_y = ystart + r;
		int fill_x = xstart, fill_y = ystart;

		int short_h = height - (2 * r);
		PixelType color = _format.RGBToColor(0, 0, 0);

		BE_RESET();

		// HACK: As we are drawing circles exploting 8-axis symmetry,
		// there are 4 pixels on each circle which are drawn twice.
		// this is ok on filled circles, but when blending on surfaces,
		// we cannot let it blend twice. awful.
		uint32 hb = 0;

		while (x++ < y) {
			BE_ALGORITHM();

			if (((1 << x) & hb) == 0) {
				blendFillClip(ptr_tl - y - px, ptr_tr + y - px, color, (uint8)alpha, tl_x - y, tl_y - x);
				hb |= (1 << x);
			}

			if (((1 << y) & hb) == 0) {
				blendFillClip(ptr_tl - x - py, ptr_tr + x - py, color, (uint8)alpha, tl_x - x, tl_y - y);
				hb |= (1 << y);
			}
		}

		ptr_fill += pitch * r;
		fill_y += r;
		while (short_h--) {
			blendFillClip(ptr_fill, ptr_fill + width + 1, color, (uint8)alpha, fill_x, fill_y);
			ptr_fill += pitch;
			++fill_y;
		}

		// Move shadow one pixel upward each iteration
		xstart += 1;
		// Multiply with expfactor
		alpha = (alpha * (expFactor << 8)) >> 9;
	}
}

/** BEVELED TABS FOR CLASSIC THEME **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawBevelTabAlg(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color, int baseLeft, int baseRight) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int i, j;

	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);

	i = bevel;
	while (i--) {
		colorFill<PixelType>(ptr_left, ptr_left + w, top_color);
		ptr_left += pitch;
	}

	if (baseLeft > 0) {
		i = h - bevel;
		ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
		while (i--) {
			colorFill<PixelType>(ptr_left, ptr_left + bevel, top_color);
			ptr_left += pitch;
		}
	}

	i = h - bevel;
	j = bevel - 1;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y);
	while (i--) {
		colorFill<PixelType>(ptr_left + j, ptr_left + bevel, bottom_color);
		if (j > 0) j--;
		ptr_left += pitch;
	}

	i = bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y + h - bevel);
	while (i--) {
		colorFill<PixelType>(ptr_left, ptr_left + baseRight + bevel, bottom_color);

		if (baseLeft)
			colorFill<PixelType>(ptr_left - w - baseLeft + bevel, ptr_left - w + bevel + bevel, top_color);
		ptr_left += pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawBevelTabAlgClip(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color, int baseLeft, int baseRight) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int i, j;

	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
	int ptr_x = x, ptr_y = y;

	i = bevel;
	while (i--) {
		colorFillClip<PixelType>(ptr_left, ptr_left + w, top_color, ptr_x, ptr_y, _clippingArea);
		ptr_left += pitch;
		++ptr_y;
	}

	if (baseLeft > 0) {
		i = h - bevel;
		ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
		ptr_x = x; ptr_y = y;
		while (i--) {
			colorFillClip<PixelType>(ptr_left, ptr_left + bevel, top_color, ptr_x, ptr_y, _clippingArea);
			ptr_left += pitch;
			++ptr_y;
		}
	}

	i = h - bevel;
	j = bevel - 1;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y);
	ptr_x = x + w - bevel; ptr_y = y;
	while (i--) {
		colorFillClip<PixelType>(ptr_left + j, ptr_left + bevel, bottom_color, ptr_x + j, ptr_y, _clippingArea);
		if (j > 0) j--;
		ptr_left += pitch;
		++ptr_y;
	}

	i = bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y + h - bevel);
	ptr_x = x + w - bevel; ptr_y = y + h - bevel;
	while (i--) {
		colorFillClip<PixelType>(ptr_left, ptr_left + baseRight + bevel, bottom_color, ptr_x, ptr_y, _clippingArea);

		if (baseLeft)
			colorFillClip<PixelType>(ptr_left - w - baseLeft + bevel, ptr_left - w + bevel + bevel, top_color, ptr_x - w - baseLeft + bevel, ptr_y, _clippingArea);
		ptr_left += pitch;
		++ptr_y;
	}
}

/** SQUARE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawSquareAlg(int x, int y, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	// Do not draw anything for empty rects.
	if (w <= 0 || h <= 0) {
		return;
	}

	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int max_h = h;

	if (fill_m != kFillDisabled) {
		while (h--) {
			if (fill_m == kFillGradient)
				color = calcGradient(max_h - h, max_h);

			colorFill<PixelType>(ptr, ptr + w, color);
			ptr += pitch;
		}
	} else {
		int sw = Base::_strokeWidth, sp = 0, hp = pitch * (h - 1);

		while (sw--) {
			colorFill<PixelType>(ptr + sp, ptr + w + sp, color);
			colorFill<PixelType>(ptr + hp - sp, ptr + w + hp - sp, color);
			sp += pitch;
		}

		while (h--) {
			colorFill<PixelType>(ptr, ptr + Base::_strokeWidth, color);
			colorFill<PixelType>(ptr + w - Base::_strokeWidth, ptr + w, color);
			ptr += pitch;
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawSquareAlgClip(int x, int y, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	// Do not draw anything for empty rects.
	if (w <= 0 || h <= 0) {
		return;
	}

	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int max_h = h;
	int ptr_y = y;

	if (fill_m != kFillDisabled) {
		while (h--) {
			if (fill_m == kFillGradient)
				color = calcGradient(max_h - h, max_h);

			colorFillClip<PixelType>(ptr, ptr + w, color, x, ptr_y, _clippingArea);
			ptr += pitch;
			++ptr_y;
		}
	} else {
		int sw = Base::_strokeWidth, sp = 0, hp = pitch * (h - 1);

		while (sw--) {
			colorFillClip<PixelType>(ptr + sp, ptr + w + sp, color, x, ptr_y + sp/pitch, _clippingArea);
			colorFillClip<PixelType>(ptr + hp - sp, ptr + w + hp - sp, color, x, ptr_y + h - sp/pitch, _clippingArea);
			sp += pitch;
		}

		while (h--) {
			colorFillClip<PixelType>(ptr, ptr + Base::_strokeWidth, color, x, ptr_y, _clippingArea);
			colorFillClip<PixelType>(ptr + w - Base::_strokeWidth, ptr + w, color, x + w - Base::_strokeWidth, ptr_y, _clippingArea);
			ptr += pitch;
			ptr_y += 1;
		}
	}
}

/** SQUARE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawBevelSquareAlg(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int i, j;
	PixelType *ptr_left;

	// Fill Background
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
	i = h;
	// Optimize rendering in case the background color is black
	if ((_bgColor & ~_alphaMask) == 0) {
		while (i--) {
			darkenFill(ptr_left, ptr_left + w);
			ptr_left += pitch;
		}
	} else {
		while (i--) {
			blendFill(ptr_left, ptr_left + w, _bgColor, 200);
			ptr_left += pitch;
		}
	}

	x = MAX(x - bevel, 0);
	y = MAX(y - bevel, 0);

	w = MIN(x + w + (bevel * 2), (int)_activeSurface->w) - x;
	h = MIN(y + h + (bevel * 2), (int)_activeSurface->h) - y;

	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
	i = bevel;
	while (i--) {
		colorFill<PixelType>(ptr_left, ptr_left + w, top_color);
		ptr_left += pitch;
	}

	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y + bevel);
	i = h - bevel;
	while (i--) {
		colorFill<PixelType>(ptr_left, ptr_left + bevel, top_color);
		ptr_left += pitch;
	}

	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y + h - bevel);
	i = bevel;
	while (i--) {
		colorFill<PixelType>(ptr_left + i, ptr_left + w, bottom_color);
		ptr_left += pitch;
	}

	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y);
	i = h - bevel;
	j = bevel - 1;
	while (i--) {
		colorFill<PixelType>(ptr_left + j, ptr_left + bevel, bottom_color);
		if (j > 0) j--;
		ptr_left += pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawBevelSquareAlgClip(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int i, j;
	PixelType *ptr_left;
	int ptr_x, ptr_y;

	// Fill Background
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
	ptr_x = x; ptr_y = y;
	i = h;
	// Optimize rendering in case the background color is black
	if ((_bgColor & ~_alphaMask) == 0) {
		while (i--) {
			darkenFillClip(ptr_left, ptr_left + w, ptr_x, ptr_y);
			ptr_left += pitch;
			++ptr_y;
		}
	} else {
		while (i-- ) {
			blendFillClip(ptr_left, ptr_left + w, ptr_x, ptr_y, _bgColor, 200);
			ptr_left += pitch;
		}
	}

	x = MAX(x - bevel, 0);
	y = MAX(y - bevel, 0);

	w = MIN(x + w + (bevel * 2), (int)_activeSurface->w) - x;
	h = MIN(y + h + (bevel * 2), (int)_activeSurface->h) - y;

	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
	ptr_x = x; ptr_y = y;
	i = bevel;
	while (i--) {
		colorFillClip<PixelType>(ptr_left, ptr_left + w, top_color, ptr_x, ptr_y, _clippingArea);
		ptr_left += pitch;
		++ptr_y;
	}

	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y + bevel);
	ptr_x = x; ptr_y = y + bevel;
	i = h - bevel;
	while (i--) {
		colorFillClip<PixelType>(ptr_left, ptr_left + bevel, top_color, ptr_x, ptr_y, _clippingArea);
		ptr_left += pitch;
		++ptr_y;
	}

	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y + h - bevel);
	ptr_x = x; ptr_y = y + h - bevel;
	i = bevel;
	while (i--) {
		colorFillClip<PixelType>(ptr_left + i, ptr_left + w, bottom_color, ptr_x + i, ptr_y, _clippingArea);
		ptr_left += pitch;
		++ptr_y;
	}

	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y);
	ptr_x = x + w - bevel; ptr_y = y;
	i = h - bevel;
	j = bevel - 1;
	while (i--) {
		colorFillClip<PixelType>(ptr_left + j, ptr_left + bevel, bottom_color, ptr_x + j, ptr_y, _clippingArea);
		if (j > 0) j--;
		ptr_left += pitch;
		++ptr_y;
	}
}

/** GENERIC LINE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawLineAlg(int x1, int y1, int x2, int y2, uint dx, uint dy, PixelType color) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
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

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawLineAlgClip(int x1, int y1, int x2, int y2, uint dx, uint dy, PixelType color) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int xdir = (x2 > x1) ? 1 : -1;
	int ptr_x = x1, ptr_y = y1;

	if (IS_IN_CLIP(ptr_x, ptr_y)) *ptr = (PixelType)color;

	if (dx > dy) {
		int ddy = dy * 2;
		int dysub = ddy - (dx * 2);
		int error_term = ddy - dx;

		while (dx--) {
			if (error_term >= 0) {
				ptr += pitch;
				++ptr_y;
				error_term += dysub;
			} else {
				error_term += ddy;
			}

			ptr += xdir;
			ptr_x += xdir;
			if (IS_IN_CLIP(ptr_x, ptr_y)) *ptr = (PixelType)color;
		}
	} else {
		int ddx = dx * 2;
		int dxsub = ddx - (dy * 2);
		int error_term = ddx - dy;

		while (dy--) {
			if (error_term >= 0) {
				ptr += xdir;
				ptr_x += xdir;
				error_term += dxsub;
			} else {
				error_term += ddx;
			}

			ptr += pitch;
			++ptr_y;
			if (IS_IN_CLIP(ptr_x, ptr_y)) *ptr = (PixelType)color;
		}
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x2, y2);
	ptr_x = x2; ptr_y = y2;
	if (IS_IN_CLIP(ptr_x, ptr_y)) *ptr = (PixelType)color;
}

/** VERTICAL TRIANGLE DRAWING ALGORITHM **/
/**
	FIXED POINT ARITHMETIC
**/

#define FIXED_POINT 1

#if FIXED_POINT
#define ipart(x) ((x) & ~0xFF)
// This is not really correct since gradient is not percentage, but [0..255]
#define rfpart(x) ((0x100 - ((x) & 0xFF)) * 100 >> 8)
//#define rfpart(x) (0x100 - ((x) & 0xFF))
#else
#define ipart(x) ((int)x)
#define round(x) (ipart(x + 0.5))
#define fpart(x) (x - ipart(x))
#define rfpart(x) (int)((1 - fpart(x)) * 100)
#endif

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTriangleVertAlg(int x1, int y1, int w, int h, bool inverted, PixelType color, VectorRenderer::FillMode fill_m) {
	// Don't draw anything for empty rects. This assures dy is always different
	// from zero.
	if (w <= 0 || h <= 0) {
		return;
	}

	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int gradient_h = 0;
	if (!inverted) {
		pitch = -pitch;
		y1 += h;
	}

	PixelType *ptr_right = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	PixelType *floor = ptr_right - 1;
	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x1 + w, y1);

	int x2 = x1 + w / 2;
	int y2 = y1 + h;

#if FIXED_POINT
	int dx = (x2 - x1) << 8;
	int dy = (y2 - y1) << 8;

	if (abs(dx) > abs(dy)) {
#else
	double dx = (double)x2 - (double)x1;
	double dy = (double)y2 - (double)y1;

	if (fabs(dx) > fabs(dy)) {
#endif
		while (floor++ != ptr_left)
			blendPixelPtr(floor, color, 50);

#if FIXED_POINT
		// In this branch dx is always different from zero. This is because
		// abs(dx) is strictly greater than abs(dy), and abs returns zero
		// as minimal value.
		int gradient = (dy << 8) / dx;
		int intery = (y1 << 8) + gradient;
#else
		double gradient = dy / dx;
		double intery = y1 + gradient;
#endif

		for (int x = x1 + 1; x < x2; x++) {
#if FIXED_POINT
			if (intery + gradient > ipart(intery) + 0x100) {
#else
			if (intery + gradient > ipart(intery) + 1) {
#endif
				ptr_right++;
				ptr_left--;
			}

			ptr_left += pitch;
			ptr_right += pitch;

			intery += gradient;

			switch (fill_m) {
			case kFillDisabled:
				*ptr_left = *ptr_right = color;
				break;
			case kFillForeground:
			case kFillBackground:
				colorFill<PixelType>(ptr_right + 1, ptr_left, color);
				blendPixelPtr(ptr_right, color, rfpart(intery));
				blendPixelPtr(ptr_left, color, rfpart(intery));
				break;
			case kFillGradient:
				colorFill<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, h));
				blendPixelPtr(ptr_right, color, rfpart(intery));
				blendPixelPtr(ptr_left, color, rfpart(intery));
				break;
			default:
				break;
			}
		}

		return;
	}

#if FIXED_POINT
	if (abs(dx) < abs(dy)) {
#else
	if (fabs(dx) < fabs(dy)) {
#endif
		ptr_left--;
		while (floor++ != ptr_left)
			blendPixelPtr(floor, color, 50);

#if FIXED_POINT
		int gradient = (dx << 8) / (dy + 0x100);
		int interx = (x1 << 8) + gradient;
#else
		double gradient = dx / (dy + 1);
		double interx = x1 + gradient;
#endif

		for (int y = y1 + 1; y < y2; y++) {
#if FIXED_POINT
			if (interx + gradient > ipart(interx) + 0x100) {
#else
			if (interx + gradient > ipart(interx) + 1) {
#endif
				ptr_right++;
				ptr_left--;
			}

			ptr_left += pitch;
			ptr_right += pitch;

			interx += gradient;

			switch (fill_m) {
			case kFillDisabled:
				*ptr_left = *ptr_right = color;
				break;
			case kFillForeground:
			case kFillBackground:
				colorFill<PixelType>(ptr_right + 1, ptr_left, color);
				blendPixelPtr(ptr_right, color, rfpart(interx));
				blendPixelPtr(ptr_left, color, rfpart(interx));
				break;
			case kFillGradient:
				colorFill<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, h));
				blendPixelPtr(ptr_right, color, rfpart(interx));
				blendPixelPtr(ptr_left, color, rfpart(interx));
				break;
			default:
				break;
			}
		}

		return;
	}

	ptr_left--;

	while (floor++ != ptr_left)
		blendPixelPtr(floor, color, 50);

#if FIXED_POINT
	int gradient = (dx / dy) << 8;
	int interx = (x1 << 8) + gradient;
#else
	double gradient = dx / dy;
	double interx = x1 + gradient;
#endif

	for (int y = y1 + 1; y < y2; y++) {
		ptr_right++;
		ptr_left--;

		ptr_left += pitch;
		ptr_right += pitch;

		interx += gradient;

		switch (fill_m) {
		case kFillDisabled:
			*ptr_left = *ptr_right = color;
			break;
		case kFillForeground:
		case kFillBackground:
			colorFill<PixelType>(ptr_right + 1, ptr_left, color);
			blendPixelPtr(ptr_right, color, rfpart(interx));
			blendPixelPtr(ptr_left, color, rfpart(interx));
			break;
		case kFillGradient:
			colorFill<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, h));
			blendPixelPtr(ptr_right, color, rfpart(interx));
			blendPixelPtr(ptr_left, color, rfpart(interx));
			break;
		default:
			break;
		}
	}

}

/////////////

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTriangleVertAlgClip(int x1, int y1, int w, int h, bool inverted, PixelType color, VectorRenderer::FillMode fill_m) {
	// Don't draw anything for empty rects. This assures dy is always different
	// from zero.
	if (w <= 0 || h <= 0) {
		return;
	}

	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int gradient_h = 0;
	int y_pitch_sign = 1;
	if (!inverted) {
		pitch = -pitch;
		y1 += h;
		y_pitch_sign = -1;
	}

	PixelType *ptr_right = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	PixelType *floor = ptr_right - 1;
	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x1 + w, y1);

	int x2 = x1 + w / 2;
	int y2 = y1 + h;
	int x_right = x1;
	int y_right = y1;
	int x_left = x1 + w;
	int y_left = y1;
	int x_floor = x_right - 1;
	int y_floor = y_right;

#if FIXED_POINT
	int dx = (x2 - x1) << 8;
	int dy = (y2 - y1) << 8;

	if (abs(dx) > abs(dy)) {
#else
	double dx = (double)x2 - (double)x1;
	double dy = (double)y2 - (double)y1;

	if (fabs(dx) > fabs(dy)) {
#endif
		while (floor++ != ptr_left)
			blendPixelPtrClip(floor, color, 50, ++x_floor, y_floor);

#if FIXED_POINT
		// In this branch dx is always different from zero. This is because
		// abs(dx) is strictly greater than abs(dy), and abs returns zero
		// as minimal value.
		int gradient = (dy << 8) / dx;
		int intery = (y1 << 8) + gradient;
#else
		double gradient = dy / dx;
		double intery = y1 + gradient;
#endif

		for (int x = x1 + 1; x < x2; x++) {
#if FIXED_POINT
			if (intery + gradient > ipart(intery) + 0x100) {
#else
			if (intery + gradient > ipart(intery) + 1) {
#endif
				ptr_right++;
				ptr_left--;
				++x_right;
				--x_left;
			}

			ptr_left += pitch;
			ptr_right += pitch;
			y_right += y_pitch_sign;
			y_left += y_pitch_sign;

			intery += gradient;

			switch (fill_m) {
			case kFillDisabled:
				if (IS_IN_CLIP(x_left, y_left)) *ptr_left = color;
				if (IS_IN_CLIP(x_right, y_right)) *ptr_right = color;
				break;
			case kFillForeground:
			case kFillBackground:
				colorFillClip<PixelType>(ptr_right + 1, ptr_left, color, x_right + 1, y_right, _clippingArea);
				blendPixelPtrClip(ptr_right, color, rfpart(intery), x_right, y_right);
				blendPixelPtrClip(ptr_left, color, rfpart(intery), x_left, y_left);
				break;
			case kFillGradient:
				colorFillClip<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, h), x_right, y_right, _clippingArea);
				blendPixelPtrClip(ptr_right, color, rfpart(intery), x_right, y_right);
				blendPixelPtrClip(ptr_left, color, rfpart(intery), x_left, y_left);
				break;
			default:
				break;
			}
			}

		return;
		}

#if FIXED_POINT
	if (abs(dx) < abs(dy)) {
#else
	if (fabs(dx) < fabs(dy)) {
#endif
		ptr_left--;
		--x_left;
		while (floor++ != ptr_left)
			blendPixelPtrClip(floor, color, 50, ++x_floor, y_floor);

#if FIXED_POINT
		int gradient = (dx << 8) / (dy + 0x100);
		int interx = (x1 << 8) + gradient;
#else
		double gradient = dx / (dy + 1);
		double interx = x1 + gradient;
#endif

		for (int y = y1 + 1; y < y2; y++) {
#if FIXED_POINT
			if (interx + gradient > ipart(interx) + 0x100) {
#else
			if (interx + gradient > ipart(interx) + 1) {
#endif
				ptr_right++;
				ptr_left--;
				++x_right;
				--x_left;
			}

			ptr_left += pitch;
			ptr_right += pitch;
			y_right += y_pitch_sign;
			y_left += y_pitch_sign;

			interx += gradient;

			switch (fill_m) {
			case kFillDisabled:
				if (IS_IN_CLIP(x_left, y_left)) *ptr_left = color;
				if (IS_IN_CLIP(x_right, y_right)) *ptr_right = color;
				break;
			case kFillForeground:
			case kFillBackground:
				colorFillClip<PixelType>(ptr_right + 1, ptr_left, color, x_right + 1, y_right, _clippingArea);
				blendPixelPtrClip(ptr_right, color, rfpart(interx), x_right, y_right);
				blendPixelPtrClip(ptr_left, color, rfpart(interx), x_left, y_left);
				break;
			case kFillGradient:
				colorFillClip<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, h), x_right, y_right, _clippingArea);
				blendPixelPtrClip(ptr_right, color, rfpart(interx), x_right, y_right);
				blendPixelPtrClip(ptr_left, color, rfpart(interx), x_left, y_left);
				break;
			default:
				break;
			}
			}

		return;
		}

	ptr_left--;
	--x_left;
	while (floor++ != ptr_left)
		blendPixelPtrClip(floor, color, 50, ++x_floor, y_floor);

#if FIXED_POINT
	int gradient = (dx / dy) << 8;
	int interx = (x1 << 8) + gradient;
#else
	double gradient = dx / dy;
	double interx = x1 + gradient;
#endif

	for (int y = y1 + 1; y < y2; y++) {
		ptr_right++;
		ptr_left--;
		++x_right;
		--x_left;

		ptr_left += pitch;
		ptr_right += pitch;
		y_right += y_pitch_sign;
		y_left += y_pitch_sign;

		interx += gradient;

		switch (fill_m) {
		case kFillDisabled:
			if (IS_IN_CLIP(x_left, y_left)) *ptr_left = color;
			if (IS_IN_CLIP(x_right, y_right)) *ptr_right = color;
			break;
		case kFillForeground:
		case kFillBackground:
			colorFillClip<PixelType>(ptr_right + 1, ptr_left, color, x_right + 1, y_right, _clippingArea);
			blendPixelPtrClip(ptr_right, color, rfpart(interx), x_right, y_right);
			blendPixelPtrClip(ptr_left, color, rfpart(interx), x_left, y_left);
			break;
		case kFillGradient:
			colorFillClip<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, h), x_right, y_right, _clippingArea);
			blendPixelPtrClip(ptr_right, color, rfpart(interx), x_right, y_right);
			blendPixelPtrClip(ptr_left, color, rfpart(interx), x_left, y_left);
			break;
		default:
			break;
		}
	}
}

/////////////

/** VERTICAL TRIANGLE DRAWING - FAST VERSION FOR SQUARED TRIANGLES */
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTriangleFast(int x1, int y1, int size, bool inverted, PixelType color, VectorRenderer::FillMode fill_m) {
	// Do not draw anything for empty rects.
	if (size <= 0) {
		return;
	}

	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	if (!inverted) {
		pitch = -pitch;
		y1 += size;
	}

	int gradient_h = 0;
	PixelType *ptr_right = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x1 + size, y1);
	int x2 = x1 + size / 2;
	int y2 = y1 + size;
	int deltaX = abs(x2 - x1);
	int deltaY = abs(y2 - y1);
	int signX = x1 < x2 ? 1 : -1;
	int signY = y1 < y2 ? 1 : -1;
	int error = deltaX - deltaY;

	colorFill<PixelType>(ptr_right, ptr_left, color);

	while (1) {
		switch (fill_m) {
		case kFillDisabled:
			*ptr_left = *ptr_right = color;
			break;
		case kFillForeground:
		case kFillBackground:
			colorFill<PixelType>(ptr_right, ptr_left, color);
			break;
		case kFillGradient:
			colorFill<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, size));
			break;
		default:
			break;
		}

		if (x1 == x2 && y1 == y2)
			break;

		int error2 = error * 2;

		if (error2 > -deltaY) {
			error -= deltaY;
			x1 += signX;
			ptr_right += signX;
			ptr_left += -signX;
		}

		if (error2 < deltaX) {
			error += deltaX;
			y1 += signY;
			ptr_right += pitch;
			ptr_left += pitch;
		}
	}
}

/** ROUNDED SQUARE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawBorderRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m, uint8 alpha_t, uint8 alpha_r, uint8 alpha_b, uint8 alpha_l) {
	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int sw = 0, sp = 0, hp = h * pitch;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;
	int short_h = h - (2 * r) + 2;

	PixelType color1 = color;
	PixelType color2 = color;

	while (sw++ < Base::_strokeWidth) {
		blendFill(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color1, alpha_t); // top
		blendFill(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color2, alpha_b); // bottom
		sp += pitch;

		BE_RESET();
		r--;

		int alphaStep_tr = ((alpha_t - alpha_r) / (y + 1));
		int alphaStep_br = ((alpha_r - alpha_b) / (y + 1));
		int alphaStep_bl = ((alpha_b - alpha_l) / (y + 1));
		int alphaStep_tl = ((alpha_l - alpha_t) / (y + 1));

		// Avoid blending the last pixels twice, since we have an alpha
		while (x++ < (y - 2)) {
			BE_ALGORITHM();

			BE_DRAWCIRCLE_BCOLOR_TR_CW(ptr_tr, x, y, px, py, (uint8)(alpha_r + (alphaStep_tr * x)));
			BE_DRAWCIRCLE_BCOLOR_BR_CW(ptr_br, x, y, px, py, (uint8)(alpha_b + (alphaStep_br * x)));
			BE_DRAWCIRCLE_BCOLOR_BL_CW(ptr_bl, x, y, px, py, (uint8)(alpha_l + (alphaStep_bl * x)));
			BE_DRAWCIRCLE_BCOLOR_TL_CW(ptr_tl, x, y, px, py, (uint8)(alpha_t + (alphaStep_tl * x)));

			BE_DRAWCIRCLE_BCOLOR_TR_CCW(ptr_tr, x, y, px, py, (uint8)(alpha_t - (alphaStep_tr * x)));
			BE_DRAWCIRCLE_BCOLOR_BR_CCW(ptr_br, x, y, px, py, (uint8)(alpha_r - (alphaStep_br * x)));
			BE_DRAWCIRCLE_BCOLOR_BL_CCW(ptr_bl, x, y, px, py, (uint8)(alpha_b - (alphaStep_bl * x)));
			BE_DRAWCIRCLE_BCOLOR_TL_CCW(ptr_tl, x, y, px, py, (uint8)(alpha_l - (alphaStep_tl * x)));

			if (Base::_strokeWidth > 1) {
				BE_DRAWCIRCLE_BCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x - 1, y, px, py);
				BE_DRAWCIRCLE_BCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px - pitch, py);
			}
		}
	}

	ptr_fill += pitch * real_radius;
	while (short_h--) {
		blendFill(ptr_fill, ptr_fill + Base::_strokeWidth, color1, alpha_l); // left
		blendFill(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color2, alpha_r); // right
		ptr_fill += pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawBorderRoundedSquareAlgClip(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m, uint8 alpha_t, uint8 alpha_r, uint8 alpha_b, uint8 alpha_l) {
	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int sw = 0, sp = 0, hp = h * pitch;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;
	int short_h = h - (2 * r) + 2;

	PixelType color1 = color;
	PixelType color2 = color;

	while (sw++ < Base::_strokeWidth) {
		blendFillClip(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color1, alpha_t,
			x1 + r, y1 + sp/pitch); // top
		blendFillClip(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color2, alpha_b,
			x1 + r, y1 + (hp - sp)/ pitch); // bottom
		sp += pitch;

		BE_RESET();
		r--;

		int alphaStep_tr = ((alpha_t - alpha_r) / (y + 1));
		int alphaStep_br = ((alpha_r - alpha_b) / (y + 1));
		int alphaStep_bl = ((alpha_b - alpha_l) / (y + 1));
		int alphaStep_tl = ((alpha_l - alpha_t) / (y + 1));

		// Avoid blending the last pixels twice, since we have an alpha
		while (x++ < (y - 2)) {
			BE_ALGORITHM();

			BE_DRAWCIRCLE_BCOLOR_TR_CW_CLIP(ptr_tr, x, y, px, py, (uint8)(alpha_r + (alphaStep_tr * x)), x1 + w - r, y1 + r);
			BE_DRAWCIRCLE_BCOLOR_BR_CW_CLIP(ptr_br, x, y, px, py, (uint8)(alpha_b + (alphaStep_br * x)), x1 + w - r, y1 + h - r);
			BE_DRAWCIRCLE_BCOLOR_BL_CW_CLIP(ptr_bl, x, y, px, py, (uint8)(alpha_l + (alphaStep_bl * x)), x1 + r, y1 + h - r);
			BE_DRAWCIRCLE_BCOLOR_TL_CW_CLIP(ptr_tl, x, y, px, py, (uint8)(alpha_t + (alphaStep_tl * x)), x1 + r, y1 + r);

			BE_DRAWCIRCLE_BCOLOR_TR_CCW_CLIP(ptr_tr, x, y, px, py, (uint8)(alpha_t - (alphaStep_tr * x)), x1 + w - r, y1 + r);
			BE_DRAWCIRCLE_BCOLOR_BR_CCW_CLIP(ptr_br, x, y, px, py, (uint8)(alpha_r - (alphaStep_br * x)), x1 + w - r, y1 + h - r);
			BE_DRAWCIRCLE_BCOLOR_BL_CCW_CLIP(ptr_bl, x, y, px, py, (uint8)(alpha_b - (alphaStep_bl * x)), x1 + r, y1 + h - r);
			BE_DRAWCIRCLE_BCOLOR_TL_CCW_CLIP(ptr_tl, x, y, px, py, (uint8)(alpha_l - (alphaStep_tl * x)), x1 + r, y1 + r);

			if (Base::_strokeWidth > 1) {
				BE_DRAWCIRCLE_BCOLOR_CLIP(ptr_tr, ptr_tl, ptr_bl, ptr_br, x - 1, y, px, py,
					x1 + w - r, y1 + r, x1 + r, y1 + r, x1 + r, y1 + h - r, x1 + w - r, y1 + h - r);
				BE_DRAWCIRCLE_BCOLOR_CLIP(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px - pitch, py,
					x1 + w - r, y1 + r, x1 + r, y1 + r, x1 + r, y1 + h - r, x1 + w - r, y1 + h - r);
			}
		}
	}

	ptr_fill += pitch * real_radius;
	while (short_h--) {
		blendFillClip(ptr_fill, ptr_fill + Base::_strokeWidth, color1, alpha_l,
			x1, y1 + real_radius + h - (2 * r) + 2 - short_h - 1); // left
		blendFillClip(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color2, alpha_r,
			x1 + w - Base::_strokeWidth + 1, y1 + real_radius + h - (2 * r) + 2 - short_h - 1); // right
		ptr_fill += pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawInteriorRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	// Do not draw empty space rounded squares.
	if (w <= 0 || h <= 0) {
		return;
	}

	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;
	int short_h = h - (2 * r) + 2;
	int long_h = h;

	BE_RESET();

	PixelType color1 = color;

	if (fill_m == kFillGradient) {
		PixelType color2, color3, color4;
		precalcGradient(long_h);

		while (x++ < y) {
			BE_ALGORITHM();

			color1 = calcGradient(real_radius - x, long_h);
			color2 = calcGradient(real_radius - y, long_h);
			color3 = calcGradient(long_h - r + x, long_h);
			color4 = calcGradient(long_h - r + y, long_h);

			gradientFill(ptr_tl - x - py, w - 2 * r + 2 * x, x1 + r - x - y, real_radius - y);
			gradientFill(ptr_tl - y - px, w - 2 * r + 2 * y, x1 + r - y - x, real_radius - x);

			gradientFill(ptr_bl - x + py, w - 2 * r + 2 * x, x1 + r - x - y, long_h - r + y);
			gradientFill(ptr_bl - y + px, w - 2 * r + 2 * y, x1 + r - y - x, long_h - r + x);

			BE_DRAWCIRCLE_XCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py);
		}
	} else {
		while (x++ < y) {
			BE_ALGORITHM();

			colorFill<PixelType>(ptr_tl - x - py, ptr_tr + x - py, color1);
			colorFill<PixelType>(ptr_tl - y - px, ptr_tr + y - px, color1);

			colorFill<PixelType>(ptr_bl - x + py, ptr_br + x + py, color1);
			colorFill<PixelType>(ptr_bl - y + px, ptr_br + y + px, color1);

			// do not remove - messes up the drawing at lower resolutions
			BE_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py);
		}
	}

	ptr_fill += pitch * r;
	while (short_h--) {
		if (fill_m == kFillGradient) {
			gradientFill(ptr_fill, w + 1, x1, real_radius++);
		} else {
			colorFill<PixelType>(ptr_fill, ptr_fill + w + 1, color1);
		}
		ptr_fill += pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawInteriorRoundedSquareAlgClip(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	// Do not draw empty space rounded squares.
	if (w <= 0 || h <= 0) {
		return;
	}

	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;
	int short_h = h - (2 * r) + 2;
	int long_h = h;

	BE_RESET();

	PixelType color1 = color;

	if (fill_m == kFillGradient) {
		PixelType color2, color3, color4;
		precalcGradient(long_h);

		while (x++ < y) {
			BE_ALGORITHM();

			color1 = calcGradient(real_radius - x, long_h);
			color2 = calcGradient(real_radius - y, long_h);
			color3 = calcGradient(long_h - r + x, long_h);
			color4 = calcGradient(long_h - r + y, long_h);

			//TL = (x1 + r, y1 + r)
			gradientFillClip(ptr_tl - x - py, w - 2 * r + 2 * x, x1 + r - x - y, real_radius - y,
				x1 + r - x, y1 + r - y);
			gradientFillClip(ptr_tl - y - px, w - 2 * r + 2 * y, x1 + r - y - x, real_radius - x,
				x1 + r - y, y1 + r - x);

			//BL = (x1 + r, y1 + h - r)
			gradientFillClip(ptr_bl - x + py, w - 2 * r + 2 * x, x1 + r - x - y, long_h - r + y,
				x1 + r - x, y1 + h - r + y);
			gradientFillClip(ptr_bl - y + px, w - 2 * r + 2 * y, x1 + r - y - x, long_h - r + x,
				x1 + r - y, y1 + h - r + x);

			BE_DRAWCIRCLE_XCOLOR_CLIP(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py,
				x1 + w - r, y1 + r, x1 + r, y1 + r, x1 + r, y1 + h - r, x1 + w - r, y1 + h - r);
		}
	} else {
		while (x++ < y) {
			BE_ALGORITHM();

			colorFillClip<PixelType>(ptr_tl - x - py, ptr_tr + x - py, color1,
				x1 + r - x, y1 + r - y, _clippingArea);
			colorFillClip<PixelType>(ptr_tl - y - px, ptr_tr + y - px, color1,
				x1 + r - y, y1 + r - x, _clippingArea);

			colorFillClip<PixelType>(ptr_bl - x + py, ptr_br + x + py, color1,
				x1 + r - x, y1 + h - r + y, _clippingArea);
			colorFillClip<PixelType>(ptr_bl - y + px, ptr_br + y + px, color1,
				x1 + r - y, y1 + h - r + x, _clippingArea);

			// do not remove - messes up the drawing at lower resolutions
			BE_DRAWCIRCLE_CLIP(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py,
				x1 + w - r, y1 + r, x1 + r, y1 + r, x1 + r, y1 + h - r, x1 + w - r, y1 + h - r);
		}
	}

	ptr_fill += pitch * r;
	int short_h_orig = short_h;
	while (short_h--) {
		if (fill_m == kFillGradient) {
			gradientFillClip(ptr_fill, w + 1, x1, real_radius++, x1, y1 + r + short_h_orig - short_h -1);
		} else {
			colorFillClip<PixelType>(ptr_fill, ptr_fill + w + 1, color1, x1, y1 + r + short_h_orig - short_h - 1, _clippingArea);
		}
		ptr_fill += pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	const uint8 borderAlpha_t = 0;
	const uint8 borderAlpha_r = 127;
	const uint8 borderAlpha_b = 255;
	const uint8 borderAlpha_l = 63;

	const uint8 bevelAlpha_t = 255;
	const uint8 bevelAlpha_r = 31;
	const uint8 bevelAlpha_b = 0;
	const uint8 bevelAlpha_l = 127;

	// If only border is visible
	if ((!(w <= 0 || h <= 0)) && (fill_m != Base::kFillDisabled)) {
		if (fill_m == Base::kFillBackground)
			drawInteriorRoundedSquareAlg(x1, y1, r, w, h, _bgColor, fill_m);
		else
			drawInteriorRoundedSquareAlg(x1, y1, r, w, h, color, fill_m);
	}

	if (Base::_strokeWidth) {
		if (r != 0 && _bevel > 0) {
			drawBorderRoundedSquareAlg(x1, y1, r, w, h, color, fill_m, borderAlpha_t, borderAlpha_r, borderAlpha_b, borderAlpha_l);
			drawBorderRoundedSquareAlg(x1, y1, r, w, h, _bevelColor, fill_m, bevelAlpha_t, bevelAlpha_r, bevelAlpha_b, bevelAlpha_l);
		} else {
			drawBorderRoundedSquareAlg(x1, y1, r, w, h, color, fill_m, 255, 255, 255, 255);
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawRoundedSquareAlgClip(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	const uint8 borderAlpha_t = 0;
	const uint8 borderAlpha_r = 127;
	const uint8 borderAlpha_b = 255;
	const uint8 borderAlpha_l = 63;

	const uint8 bevelAlpha_t = 255;
	const uint8 bevelAlpha_r = 31;
	const uint8 bevelAlpha_b = 0;
	const uint8 bevelAlpha_l = 127;

	// If only border is visible
	if ((!(w <= 0 || h <= 0)) && (fill_m != Base::kFillDisabled)) {
		if (fill_m == Base::kFillBackground)
			drawInteriorRoundedSquareAlgClip(x1, y1, r, w, h, _bgColor, fill_m);
		else
			drawInteriorRoundedSquareAlgClip(x1, y1, r, w, h, color, fill_m);
	}

	//I expect these to work fine with clipping:
	if (Base::_strokeWidth) {
		if (r != 0 && _bevel > 0) {
			drawBorderRoundedSquareAlgClip(x1, y1, r, w, h, color, fill_m, borderAlpha_t, borderAlpha_r, borderAlpha_b, borderAlpha_l);
			drawBorderRoundedSquareAlgClip(x1, y1, r, w, h, _bevelColor, fill_m, bevelAlpha_t, bevelAlpha_r, bevelAlpha_b, bevelAlpha_l);
		} else {
			drawBorderRoundedSquareAlgClip(x1, y1, r, w, h, color, fill_m, 255, 255, 255, 255);
		}
	}
}

/** CIRCLE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawCircleAlg(int x1, int y1, int r, PixelType color, VectorRenderer::FillMode fill_m) {
	int f, ddF_x, ddF_y;
	int x, y, px, py, sw = 0;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	if (fill_m == kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			BE_RESET();
			r--;

			*(ptr + y) = color;
			*(ptr - y) = color;
			*(ptr + py) = color;
			*(ptr - py) = color;

			while (x++ < y) {
				BE_ALGORITHM();
				BE_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py);

				if (Base::_strokeWidth > 1) {
					BE_DRAWCIRCLE(ptr, ptr, ptr, ptr, x - 1, y, px, py);
					BE_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px - pitch, py);
				}
			}
		}
	} else {
		colorFill<PixelType>(ptr - r, ptr + r, color);
		BE_RESET();

		while (x++ < y) {
			BE_ALGORITHM();
			colorFill<PixelType>(ptr - x + py, ptr + x + py, color);
			colorFill<PixelType>(ptr - x - py, ptr + x - py, color);
			colorFill<PixelType>(ptr - y + px, ptr + y + px, color);
			colorFill<PixelType>(ptr - y - px, ptr + y - px, color);
		}
	}
}


template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawCircleAlgClip(int x1, int y1, int r, PixelType color, VectorRenderer::FillMode fill_m) {
	int f, ddF_x, ddF_y;
	int x, y, px, py, sw = 0;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	if (fill_m == kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			BE_RESET();
			r--;

			if (IS_IN_CLIP(x1 + y, y1)) *(ptr + y) = color;
			if (IS_IN_CLIP(x1 - y, y1)) *(ptr - y) = color;
			if (IS_IN_CLIP(x1, y1 + y)) *(ptr + py) = color;
			if (IS_IN_CLIP(x1, y1 - y)) *(ptr - py) = color;

			while (x++ < y) {
				BE_ALGORITHM();
				BE_DRAWCIRCLE_CLIP(ptr, ptr, ptr, ptr, x, y, px, py, x1, y1, x1, y1, x1, y1, x1, y1);

				if (Base::_strokeWidth > 1) {
					BE_DRAWCIRCLE_CLIP(ptr, ptr, ptr, ptr, x - 1, y, px, py, x1, y1, x1, y1, x1, y1, x1, y1);
					BE_DRAWCIRCLE_CLIP(ptr, ptr, ptr, ptr, x, y, px - pitch, py, x1, y1, x1, y1, x1, y1, x1, y1);
				}
			}
		}
	} else {
		colorFillClip<PixelType>(ptr - r, ptr + r, color, x1 - r, y1 + r, _clippingArea);
		BE_RESET();

		while (x++ < y) {
			BE_ALGORITHM();
			colorFillClip<PixelType>(ptr - x + py, ptr + x + py, color, x1 - x, y1 + y, _clippingArea);
			colorFillClip<PixelType>(ptr - x - py, ptr + x - py, color, x1 - x, y1 - y, _clippingArea);
			colorFillClip<PixelType>(ptr - y + px, ptr + y + px, color, x1 - y, y1 + x, _clippingArea);
			colorFillClip<PixelType>(ptr - y - px, ptr + y - px, color, x1 - y, y1 - x, _clippingArea);
		}
	}
}


/********************************************************************
 ********************************************************************
 * SHADOW drawing algorithms - VectorRendererSpec *******************
 ********************************************************************
 ********************************************************************/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawSquareShadow(int x, int y, int w, int h, int offset) {
	// Do nothing for empty rects or no shadow offset.
	if (w <= 0 || h <= 0 || offset <= 0) {
		return;
	}

	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x + w - 1, y + offset);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int i, j;

	i = h - offset;

	while (i--) {
		j = offset;
		while (j--)
			blendPixelPtr(ptr + j, 0, ((offset - j) << 8) / offset);
		ptr += pitch;
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x + offset, y + h - 1);

	while (i++ < offset) {
		j = w - offset;
		while (j--)
			blendPixelPtr(ptr + j, 0, ((offset - i) << 8) / offset);
		ptr += pitch;
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x + w, y + h);

	i = 0;
	while (i++ < offset) {
		j = offset - 1;
		while (j--)
			blendPixelPtr(ptr + j, 0, (((offset - j) * (offset - i)) << 8) / (offset * offset));
		ptr += pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawSquareShadowClip(int x, int y, int w, int h, int offset) {
	// Do nothing for empty rects or no shadow offset.
	if (w <= 0 || h <= 0 || offset <= 0) {
		return;
	}

	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x + w - 1, y + offset);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int i, j, ptr_x = x+w-1, ptr_y = y+offset;

	i = h - offset;

	while (i--) {
		j = offset;
		while (j--)
			blendPixelPtrClip(ptr + j, 0, ((offset - j) << 8) / offset, ptr_x + j, ptr_y);
		ptr += pitch;
		++ptr_y;
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x + offset, y + h - 1);
	ptr_x = x + offset;
	ptr_y = y + h - 1;

	while (i++ < offset) {
		j = w - offset;
		while (j--)
			blendPixelPtrClip(ptr + j, 0, ((offset - i) << 8) / offset, ptr_x + j, ptr_y);
		ptr += pitch;
		++ptr_y;
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x + w, y + h);
	ptr_x = x + w;
	ptr_y = y + h;

	i = 0;
	while (i++ < offset) {
		j = offset - 1;
		while (j--)
			blendPixelPtrClip(ptr + j, 0, (((offset - j) * (offset - i)) << 8) / (offset * offset), ptr_x + j, ptr_y);
		ptr += pitch;
		++ptr_y;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawRoundedSquareShadow(int x1, int y1, int r, int w, int h, int offset) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	// "Harder" shadows when having lower BPP, since we will have artifacts (greenish tint on the modern theme)
	uint8 expFactor = 3;
	uint16 alpha = (_activeSurface->format.bytesPerPixel > 2) ? 4 : 8;

	// These constants ensure a border of 2px on the left and of each rounded square
	Common::Rect shadowRect(w + offset + 2, h + offset + 1);
	shadowRect.translate((x1 > 2) ? x1 - 2 : x1, y1);

	// The rounded rectangle drawn on top of this shadow is guaranteed
	// to occlude entirely the following rect with a non-transparent color.
	// As an optimization, we don't draw the shadow inside of it.
	Common::Rect occludingRect(x1, y1, x1 + w, y1 + h);
	occludingRect.top    += r;
	occludingRect.bottom -= r;

	// Soft shadows are constructed by drawing increasingly
	// darker and smaller rectangles on top of each other.
	for (int i = offset; i >= 0; i--) {
		int f, ddF_x, ddF_y;
		int x, y, px, py;

		PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(shadowRect.left  + r, shadowRect.top + r);
		PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(shadowRect.right - r, shadowRect.top + r);
		PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(shadowRect.left  + r, shadowRect.bottom - r);
		PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(shadowRect.right - r, shadowRect.bottom - r);

		PixelType color = _format.RGBToColor(0, 0, 0);

		BE_RESET();

		// HACK: As we are drawing circles exploiting 8-axis symmetry,
		// there are 4 pixels on each circle which are drawn twice.
		// this is ok on filled circles, but when blending on surfaces,
		// we cannot let it blend twice. awful.
		uint32 hb = 0;

		// Draw the top and bottom parts of the shadow. Those parts have rounded corners.
		while (x++ < y) {
			BE_ALGORITHM();

			if (((1 << x) & hb) == 0) {
				blendFill(ptr_tl - y - px, ptr_tr + y - px, color, (uint8)alpha);
				blendFill(ptr_bl - y + px, ptr_br + y + px, color, (uint8)alpha);
				hb |= (1 << x);
			}

			if (((1 << y) & hb) == 0) {
				blendFill(ptr_tl - x - py, ptr_tr + x - py, color, (uint8)alpha);
				blendFill(ptr_bl - x + py, ptr_br + x + py, color, (uint8)alpha);
				hb |= (1 << y);
			}
		}

		// Draw the middle part of the shadow. This part is a rectangle with regular corners.
		PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(0, shadowRect.top + r);
		for (int y2 = shadowRect.top + r; y2 < shadowRect.bottom - r + 1; y2++) {

			if (occludingRect.top <= y2 && y2 < occludingRect.bottom) {
				if (shadowRect.left < occludingRect.left) {
					blendFill(ptr_fill + shadowRect.left, ptr_fill + occludingRect.left, color, (uint8)alpha);
				}
				if (occludingRect.right < shadowRect.right + 1) {
					blendFill(ptr_fill + occludingRect.right, ptr_fill + shadowRect.right + 1, color, (uint8)alpha);
				}
			} else {
				blendFill(ptr_fill + shadowRect.left, ptr_fill + shadowRect.right + 1, color, (uint8)alpha);
			}

			ptr_fill += pitch;
		}

		// Make shadow smaller each iteration
		shadowRect.grow(-1);

		if (_shadowFillMode == kShadowExponential)
			// Multiply with expfactor
			alpha = (alpha * (expFactor << 8)) >> 9;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawRoundedSquareShadowClip(int x1, int y1, int r, int w, int h, int offset) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	// "Harder" shadows when having lower BPP, since we will have artifacts (greenish tint on the modern theme)
	uint8 expFactor = 3;
	uint16 alpha = (_activeSurface->format.bytesPerPixel > 2) ? 4 : 8;

	// These constants ensure a border of 2px on the left and of each rounded square
	Common::Rect shadowRect(w + offset + 2, h + offset + 1);
	shadowRect.translate((x1 > 2) ? x1 - 2 : x1, y1);

	// The rounded rectangle drawn on top of this shadow is guaranteed
	// to occlude entirely the following rect with a non-transparent color.
	// As an optimization, we don't draw the shadow inside of it.
	Common::Rect occludingRect(x1, y1, x1 + w, y1 + h);
	occludingRect.top    += r;
	occludingRect.bottom -= r;

	// Soft shadows are constructed by drawing increasingly
	// darker and smaller rectangles on top of each other.
	for (int i = offset; i >= 0; i--) {
		int f, ddF_x, ddF_y;
		int x, y, px, py;

		PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(shadowRect.left  + r, shadowRect.top + r);
		PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(shadowRect.right - r, shadowRect.top + r);
		PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(shadowRect.left  + r, shadowRect.bottom - r);
		PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(shadowRect.right - r, shadowRect.bottom - r);

		PixelType color = _format.RGBToColor(0, 0, 0);

		BE_RESET();

		// HACK: As we are drawing circles exploiting 8-axis symmetry,
		// there are 4 pixels on each circle which are drawn twice.
		// this is ok on filled circles, but when blending on surfaces,
		// we cannot let it blend twice. awful.
		uint32 hb = 0;

		while (x++ < y) {
			BE_ALGORITHM();

			if (((1 << x) & hb) == 0) {
				blendFillClip(ptr_tl - y - px, ptr_tr + y - px, color, (uint8)alpha,
				              shadowRect.left + r - y, shadowRect.top + r - x);
				blendFillClip(ptr_bl - y + px, ptr_br + y + px, color, (uint8)alpha,
				              shadowRect.left + r - y, shadowRect.bottom - r + x);

				hb |= (1 << x);
			}

			if (((1 << y) & hb) == 0) {
				blendFillClip(ptr_tl - x - py, ptr_tr + x - py, color, (uint8)alpha,
				              shadowRect.left + r - x, shadowRect.top + r - y);
				blendFillClip(ptr_bl - x + py, ptr_br + x + py, color, (uint8)alpha,
				              shadowRect.left + r - x, shadowRect.bottom - r + y);

				hb |= (1 << y);
			}
		}

		// Draw the middle part of the shadow. This part is a rectangle with regular corners.
		PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(0, shadowRect.top + r);
		for (int y2 = shadowRect.top + r; y2 < shadowRect.bottom - r + 1; y2++) {

			if (occludingRect.top <= y2 && y2 < occludingRect.bottom) {
				if (shadowRect.left < occludingRect.left) {
					blendFillClip(ptr_fill + shadowRect.left, ptr_fill + occludingRect.left, color, (uint8)alpha,
					              shadowRect.left, y2);
				}
				if (occludingRect.right < shadowRect.right + 1) {
					blendFillClip(ptr_fill + occludingRect.right, ptr_fill + shadowRect.right + 1, color, (uint8)alpha,
					              occludingRect.right, y2);
				}
			} else {
				blendFillClip(ptr_fill + shadowRect.left, ptr_fill + shadowRect.right + 1, color, (uint8)alpha,
				              shadowRect.left, y2);
			}

			ptr_fill += pitch;
		}

		// Make shadow smaller each iteration
		shadowRect.grow(-1);

		if (_shadowFillMode == kShadowExponential)
			// Multiply with expfactor
			alpha = (alpha * (expFactor << 8)) >> 9;
	}
}

/******************************************************************************/



#ifndef DISABLE_FANCY_THEMES

/********************************************************************
 * ANTIALIASED PRIMITIVES drawing algorithms - VectorRendererAA
 ********************************************************************/
/** LINES **/
template<typename PixelType>
void VectorRendererAA<PixelType>::
drawLineAlg(int x1, int y1, int x2, int y2, uint dx, uint dy, PixelType color) {
	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);
	int pitch = Base::_activeSurface->pitch / Base::_activeSurface->format.bytesPerPixel;
	int xdir = (x2 > x1) ? 1 : -1;
	uint16 error_tmp, error_acc, gradient;
	uint8 alpha;

	*ptr = (PixelType)color;

	if (dx > dy) {
		gradient = (dy << 16) / dx;
		error_acc = 0;

		while (--dx) {
			error_tmp = error_acc;
			error_acc += gradient;

			if (error_acc <= error_tmp)
				ptr += pitch;

			ptr += xdir;
			alpha = (error_acc >> 8);

			this->blendPixelPtr(ptr, color, ~alpha);
			this->blendPixelPtr(ptr + pitch, color, alpha);
		}
	} else if (dy != 0) {
		gradient = (dx << 16) / dy;
		error_acc = 0;

		while (--dy) {
			error_tmp = error_acc;
			error_acc += gradient;

			if (error_acc <= error_tmp)
				ptr += xdir;

			ptr += pitch;
			alpha = (error_acc >> 8);

			this->blendPixelPtr(ptr, color, ~alpha);
			this->blendPixelPtr(ptr + xdir, color, alpha);
		}
	}

	Base::putPixel(x2, y2, color);
}

/** TAB ALGORITHM */
template<typename PixelType>
void VectorRendererAA<PixelType>::
drawTabAlg(int x1, int y1, int w, int h, int r, PixelType color, VectorRenderer::FillMode fill_m, int baseLeft, int baseRight) {
	// Don't draw anything for empty rects.
	if (w <= 0 || h <= 0) {
		return;
	}

	int x, y, px, py;
	int pitch = Base::_activeSurface->pitch / Base::_activeSurface->format.bytesPerPixel;
	int sw = 0, sp = 0, hp = 0;

	frac_t T = 0, oldT;
	uint8 a1, a2;
	uint32 rsq = r*r;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;

	if (fill_m == Base::kFillDisabled) {
		color = 0;
		while (sw++ < Base::_strokeWidth) {
			colorFill<PixelType>(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
			colorFill<PixelType>(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color);
			sp += pitch;

			x = r - (sw - 1);
			y = 0;
			T = 0;
			px = pitch * x;
			py = 0;


			while (x > y++) {
				WU_ALGORITHM();

				// sw == 1: outside, sw = _strokeWidth: inside
				if (sw != Base::_strokeWidth)
					a2 = 255;

				// inner arc
				WU_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px, py, a2);

				if (sw == 1) // outer arc
					WU_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px - pitch, py, a1);
			}
		}

		int short_h = h - r;

		ptr_fill += pitch * real_radius;
		while (short_h--) {
			colorFill<PixelType>(ptr_fill, ptr_fill + Base::_strokeWidth, color);
			colorFill<PixelType>(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}

		if (baseLeft) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1 + h + 1);
			while (sw++ < Base::_strokeWidth) {
				colorFill<PixelType>(ptr_fill - baseLeft, ptr_fill, color);
				ptr_fill += pitch;
			}
		}

		if (baseRight) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w, y1 + h + 1);
			while (sw++ < Base::_strokeWidth) {
				colorFill<PixelType>(ptr_fill, ptr_fill + baseRight, color);
				ptr_fill += pitch;
			}
		}
	} else {
		PixelType color1, color2;
		color1 = color2 = color;

		int long_h = h;
		int short_h = h - real_radius;
		x = real_radius;
		y = 0;
		T = 0;
		px = pitch * x;
		py = 0;

		Base::precalcGradient(long_h);

		while (x > y++) {
			WU_ALGORITHM();

			if (fill_m == Base::kFillGradient) {
				color1 = Base::calcGradient(real_radius - x, long_h);
				color2 = Base::calcGradient(real_radius - y, long_h);

				Base::gradientFill(ptr_tl - x - py + 1, w - 2 * r + 2 * x - 1, x1 + r - x - y + 1, real_radius - y);

				// Only fill each horizontal line once (or we destroy
				// the gradient effect at the edges)
				if (T < oldT || y == 1)
					Base::gradientFill(ptr_tl - y - px + 1, w - 2 * r + 2 * y - 1, x1 + r - y - x + 1, real_radius - x);

				WU_DRAWCIRCLE_XCOLOR_TOP(ptr_tr, ptr_tl, x, y, px, py, a1, Base::blendPixelPtr);
			} else {
				colorFill<PixelType>(ptr_tl - x - py + 1, ptr_tr + x - py, color);
				if (T < oldT || y == 1)
					colorFill<PixelType>(ptr_tl - y - px + 1, ptr_tr + y - px, color);

				WU_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px, py, a1);
			}
		}

		ptr_fill += pitch * r;
		while (short_h--) {
			if (fill_m == Base::kFillGradient) {
				Base::gradientFill(ptr_fill, w + 1, x1, real_radius++);
			} else {
				colorFill<PixelType>(ptr_fill, ptr_fill + w + 1, color);
			}
			ptr_fill += pitch;
		}
	}
}


/** ROUNDED SQUARES **/
template<typename PixelType>
void VectorRendererAA<PixelType>::
drawBorderRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m, uint8 alpha_t, uint8 alpha_r, uint8 alpha_b, uint8 alpha_l) {
	int x, y;
	const int pitch = Base::_activeSurface->pitch / Base::_activeSurface->format.bytesPerPixel;
	int px, py;

	uint32 rsq = r*r;
	frac_t T = 0, oldT;
	uint8 a1, a2;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int sw = 0, sp = 0;
	int short_h = h - 2 * r;
	int hp = h * pitch;

	int strokeWidth = Base::_strokeWidth;

	while (sw++ < strokeWidth) {
		this->blendFill(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color, alpha_b); // bottom
		this->blendFill(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color, alpha_t); // top

		sp += pitch;

		x = r - (sw - 1);
		y = 0;
		T = 0;
		px = pitch * x;
		py = 0;

		int alphaStep_tr = ((alpha_t - alpha_r) / (x + 1));
		int alphaStep_br = ((alpha_r - alpha_b) / (x + 1));
		int alphaStep_bl = ((alpha_b - alpha_l) / (x + 1));
		int alphaStep_tl = ((alpha_l - alpha_t) / (x + 1));

		while (x > y++) {
			WU_ALGORITHM();

			// sw == 1: outside, sw = _strokeWidth: inside
			// We always draw the outer edge AAed, but the inner edge
			// only when the inside isn't filled
			if (sw != strokeWidth || fill_m != Base::kFillDisabled)
				a2 = 255;

				// inner arc
				WU_DRAWCIRCLE_BCOLOR_TR_CW(ptr_tr, (x - 1), y, (px - pitch), py, (uint8)((uint32)(((alpha_t - (alphaStep_tr * y)) << 8) * a2) >> 16));
				WU_DRAWCIRCLE_BCOLOR_BR_CW(ptr_br, (x - 1), y, (px - pitch), py, (uint8)((uint32)(((alpha_r - (alphaStep_br * y)) << 8) * a2) >> 16));
				WU_DRAWCIRCLE_BCOLOR_BL_CW(ptr_bl, (x - 1), y, (px - pitch), py, (uint8)((uint32)(((alpha_b - (alphaStep_bl * y)) << 8) * a2) >> 16));
				WU_DRAWCIRCLE_BCOLOR_TL_CW(ptr_tl, (x - 1), y, (px - pitch), py, (uint8)((uint32)(((alpha_l - (alphaStep_tl * y)) << 8) * a2) >> 16));

				WU_DRAWCIRCLE_BCOLOR_TR_CCW(ptr_tr, (x - 1), y, (px - pitch), py, (uint8)((uint32)(((alpha_r + (alphaStep_tr * y)) << 8) * a2) >> 16));
				WU_DRAWCIRCLE_BCOLOR_BR_CCW(ptr_br, (x - 1), y, (px - pitch), py, (uint8)((uint32)(((alpha_b + (alphaStep_br * y)) << 8) * a2) >> 16));
				WU_DRAWCIRCLE_BCOLOR_BL_CCW(ptr_bl, (x - 1), y, (px - pitch), py, (uint8)((uint32)(((alpha_l + (alphaStep_bl * y)) << 8) * a2) >> 16));
				WU_DRAWCIRCLE_BCOLOR_TL_CCW(ptr_tl, (x - 1), y, (px - pitch), py, (uint8)((uint32)(((alpha_t + (alphaStep_tl * y)) << 8) * a2) >> 16));

				// outer arc
				if (sw == 1) {
					WU_DRAWCIRCLE_BCOLOR_TR_CW(ptr_tr, x, y, px, py, (uint8)((uint32)(((alpha_t - (alphaStep_tr * y)) << 8) * a1) >> 16));
					WU_DRAWCIRCLE_BCOLOR_BR_CW(ptr_br, x, y, px, py, (uint8)((uint32)(((alpha_r - (alphaStep_br * y)) << 8) * a1) >> 16));
					WU_DRAWCIRCLE_BCOLOR_BL_CW(ptr_bl, x, y, px, py, (uint8)((uint32)(((alpha_b - (alphaStep_bl * y)) << 8) * a1) >> 16));
					WU_DRAWCIRCLE_BCOLOR_TL_CW(ptr_tl, x, y, px, py, (uint8)((uint32)(((alpha_l - (alphaStep_tl * y)) << 8) * a1) >> 16));

					WU_DRAWCIRCLE_BCOLOR_TR_CCW(ptr_tr, x, y, px, py, (uint8)((uint32)(((alpha_r + (alphaStep_tr * y)) << 8) * a1) >> 16));
					WU_DRAWCIRCLE_BCOLOR_BR_CCW(ptr_br, x, y, px, py, (uint8)((uint32)(((alpha_b + (alphaStep_br * y)) << 8) * a1) >> 16));
					WU_DRAWCIRCLE_BCOLOR_BL_CCW(ptr_bl, x, y, px, py, (uint8)((uint32)(((alpha_l + (alphaStep_bl * y)) << 8) * a1) >> 16));
					WU_DRAWCIRCLE_BCOLOR_TL_CCW(ptr_tl, x, y, px, py, (uint8)((uint32)(((alpha_t + (alphaStep_tl * y)) << 8) * a1) >> 16));
				}
			}

		ptr_fill += pitch * r;

		while (short_h-- >= -2) {
			this->blendFill(ptr_fill, ptr_fill + Base::_strokeWidth, color, alpha_l); // left
			this->blendFill(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color, alpha_r); // right
			ptr_fill += pitch;
		}
	}
}

template<typename PixelType>
void VectorRendererAA<PixelType>::
drawInteriorRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	w -= 2*Base::_strokeWidth;
	h -= 2*Base::_strokeWidth;

	// Do not draw empty space rounded squares.
	if (w <= 0 || h <= 0) {
		return;
	}

	int x, y;
	const int pitch = Base::_activeSurface->pitch / Base::_activeSurface->format.bytesPerPixel;
	int px, py;

	uint32 rsq = r*r;
	frac_t T = 0, oldT;
	uint8 a1, a2;

	r -= Base::_strokeWidth;
	x1 += Base::_strokeWidth;
	y1 += Base::_strokeWidth;
	rsq = r*r;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int short_h = h - 2 * r;
	x = r;
	y = 0;
	T = 0;
	px = pitch * x;
	py = 0;

	if (fill_m == Base::kFillGradient) {

		Base::precalcGradient(h);

		PixelType color1, color2, color3, color4;
		while (x > y++) {
			WU_ALGORITHM();

			color1 = Base::calcGradient(r - x, h);
			color2 = Base::calcGradient(r - y, h);
			color3 = Base::calcGradient(h - r + x, h);
			color4 = Base::calcGradient(h - r + y, h);

			Base::gradientFill(ptr_tl - x - py + 1, w - 2 * r + 2 * x - 1, x1 + r - x - y + 1, r - y);

			// Only fill each horizontal line once (or we destroy
			// the gradient effect at the edges)
			if (T < oldT || y == 1)
				Base::gradientFill(ptr_tl - y - px + 1, w - 2 * r + 2 * y - 1, x1 + r - y - x + 1, r - x);

			Base::gradientFill(ptr_bl - x + py + 1, w - 2 * r + 2 * x - 1, x1 + r - x - y + 1, h - r + y);

			// Only fill each horizontal line once (or we destroy
			// the gradient effect at the edges)
			if (T < oldT || y == 1)
				Base::gradientFill(ptr_bl - y + px + 1, w - 2 * r + 2 * y - 1, x1 + r - y - x + 1, h - r + x);

			// This shape is used for dialog backgrounds.
			// If we're drawing on top of an empty overlay background,
			// and the overlay supports alpha, we have to do AA by
			// setting the dest alpha channel, instead of blending with
			// dest color channels.
			if (!g_system->hasFeature(OSystem::kFeatureOverlaySupportsAlpha))
				WU_DRAWCIRCLE_XCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1, blendPixelPtr);
			else
				WU_DRAWCIRCLE_XCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1, blendPixelDestAlphaPtr);
		}

		ptr_fill += pitch * r;
		while (short_h-- >= 0) {
			Base::gradientFill(ptr_fill, w + 1, x1, r++);
			ptr_fill += pitch;
		}

	} else {

		while (x > 1 + y++) {
			WU_ALGORITHM();

			colorFill<PixelType>(ptr_tl - x - py + 1, ptr_tr + x - py, color);
			if (T < oldT || y == 1)
				colorFill<PixelType>(ptr_tl - y - px + 1, ptr_tr + y - px, color);

			colorFill<PixelType>(ptr_bl - x + py + 1, ptr_br + x + py, color);
			if (T < oldT || y == 1)
				colorFill<PixelType>(ptr_bl - y + px + 1, ptr_br + y + px, color);

			WU_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1);
		}

		ptr_fill += pitch * r;
		while (short_h-- >= 0) {
			colorFill<PixelType>(ptr_fill, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}
	}
}

template<typename PixelType>
void VectorRendererAA<PixelType>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	const uint8 borderAlpha_t = 0;
	const uint8 borderAlpha_r = 127;
	const uint8 borderAlpha_b = 255;
	const uint8 borderAlpha_l = 63;

	const uint8 bevelAlpha_t = 255;
	const uint8 bevelAlpha_r = 31;
	const uint8 bevelAlpha_b = 0;
	const uint8 bevelAlpha_l = 127;

	if (Base::_strokeWidth) {
		if (r != 0 && Base::_bevel > 0) {
			drawBorderRoundedSquareAlg(x1, y1, r, w, h, color, fill_m, borderAlpha_t, borderAlpha_r, borderAlpha_b, borderAlpha_l);
			drawBorderRoundedSquareAlg(x1, y1, r, w, h, Base::_bevelColor, fill_m, bevelAlpha_t, bevelAlpha_r, bevelAlpha_b, bevelAlpha_l);
		} else {
			drawBorderRoundedSquareAlg(x1, y1, r, w, h, color, fill_m, 255, 255, 255, 255);
		}
	}

	// If only border is visible
	if ((!(w <= 0 || h <= 0)) && (fill_m != Base::kFillDisabled)) {
		if (fill_m == Base::kFillBackground)
			drawInteriorRoundedSquareAlg(x1, y1, r, w, h, Base::_bgColor, fill_m);
		else
			drawInteriorRoundedSquareAlg(x1, y1, r, w, h, color, fill_m);
	}
}

/** CIRCLES **/
template<typename PixelType>
void VectorRendererAA<PixelType>::
drawCircleAlg(int x1, int y1, int r, PixelType color, VectorRenderer::FillMode fill_m) {
	int x, y, sw = 0;
	const int pitch = Base::_activeSurface->pitch / Base::_activeSurface->format.bytesPerPixel;
	int px, py;

	uint32 rsq = r*r;
	frac_t T = 0, oldT;
	uint8 a1, a2;

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	if (fill_m == VectorRenderer::kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			x = r - (sw - 1);
			y = 0;
			T = 0;
			px = pitch * x;
			py = 0;

			*(ptr + x) = (PixelType)color;
			*(ptr - x) = (PixelType)color;
			*(ptr + px) = (PixelType)color;
			*(ptr - px) = (PixelType)color;

			while (x > y++) {
				WU_ALGORITHM();

				if (sw != 1 && sw != Base::_strokeWidth)
					a2 = a1 = 255;

				WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, (x - 1), y, (px - pitch), py, a2);
				WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py, a1);
			}
		}
	} else {
		colorFill<PixelType>(ptr - r, ptr + r + 1, color);
		x = r;
		y = 0;
		T = 0;
		px = pitch * x;
		py = 0;

		while (x > y++) {
			WU_ALGORITHM();

			colorFill<PixelType>(ptr - x + py, ptr + x + py, color);
			colorFill<PixelType>(ptr - x - py, ptr + x - py, color);
			colorFill<PixelType>(ptr - y + px, ptr + y + px, color);
			colorFill<PixelType>(ptr - y - px, ptr + y - px, color);

			WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py, a1);
		}
	}
}

#endif

}
