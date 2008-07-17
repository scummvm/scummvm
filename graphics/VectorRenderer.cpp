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

#include "gui/ThemeRenderer.h"
#include "graphics/VectorRenderer.h"

#define VECTOR_RENDERER_FAST_TRIANGLES

namespace Graphics {

VectorRenderer *createRenderer(int mode) {
	switch (mode) {
	case GUI::ThemeRenderer::kGfxStandard16bit:
		return new VectorRendererSpec<uint16, ColorMasks<565> >;

	case GUI::ThemeRenderer::kGfxAntialias16bit:
		return new VectorRendererAA<uint16, ColorMasks<565> >;

	default:
		return 0;
	}
}

/********************************************************************
 * DRAWSTEP handling functions
 ********************************************************************/
void VectorRenderer::drawStep(const Common::Rect &area, const DrawStep &step, uint32 extra) {

	if (step.bgColor.set)
		setBgColor(step.bgColor.r, step.bgColor.g, step.bgColor.b);

	if (step.fgColor.set)
		setFgColor(step.fgColor.r, step.fgColor.g, step.fgColor.b);

	if (step.gradColor1.set && step.gradColor2.set)
		setGradientColors(step.gradColor1.r, step.gradColor1.g, step.gradColor1.b, 
						  step.gradColor2.r, step.gradColor2.g, step.gradColor2.b);

	shadowEnable(step.shadow);
	setGradientFactor(step.factor);
	setStrokeWidth(step.stroke);
	setFillMode((FillMode)step.fillMode);
	
	_dynamicData = extra;

	(this->*(step.drawingCall))(area, step);
}

void VectorRenderer::textStep(const Common::String &text, const Common::Rect &area, const TextStep &step) {
	if (step.color.set)
		setFgColor(step.color.r, step.color.g, step.color.b);
		
	drawString(step.font, text.c_str(), area, step.alignHorizontal, step.alignVertical);
}

/********************************************************************
 * MISCELANEOUS functions
 ********************************************************************/
/** Fixed point SQUARE ROOT **/
inline uint32 fp_sqroot(uint32 x) {
	register uint32 root, remHI, remLO, testDIV, count;

	root = 0;
	remHI = 0;
	remLO = x;
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
}

/** HELPER MACROS for BESENHALM's circle drawing algorithm **/
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

#define __WU_ALGORITHM() { \
	oldT = T; \
	T = fp_sqroot(rsq - ((y * y) << 16)) ^ 0xFFFF; \
	py += p; \
	if (T < oldT) { \
		x--; px -= p; \
	} \
	a2 = (T >> 8); \
	a1 = ~a2; \
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

/********************************************************************
 * Primitive shapes drawing - Public API calls - VectorRendererSpec
 ********************************************************************/
template <typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawString(const Graphics::Font *font, const Common::String &text, const Common::Rect &area, 
			GUI::Theme::TextAlign alignH, GUI::Theme::TextAlignVertical alignV) {

	int offset = 0;
	
	switch (alignV) {
		case GUI::Theme::kTextAlignVCenter:
			offset = area.top + (area.height() - font->getFontHeight()) / 2;
			break;
		case GUI::Theme::kTextAlignVBottom:
			offset = area.bottom - font->getFontHeight();
			break;
		case GUI::Theme::kTextAlignVTop:
			offset = area.top;
			break;
	}
	
	font->drawString(_activeSurface, text, area.left, offset, area.width(), _fgColor, (Graphics::TextAlignment)alignH, 0, false);
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

	if (dy == 0) { // horizontal lines
		// these can be filled really fast with a single memset.
		colorFill(ptr, ptr + dx + 1, (PixelType)_fgColor);

	} else if (dx == 0) { // vertical lines
		// these ones use a static pitch increase.
		while (y1++ <= y2) {
			*ptr = (PixelType)_fgColor;
			ptr += pitch;
		}

	} else if (ABS(dx) == ABS(dy)) { // diagonal lines
		// these ones also use a fixed pitch increase
		pitch += (x2 > x1) ? 1 : -1;

		while (dy--) {
			*ptr = (PixelType)_fgColor;
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
		w <= 0 || h <= 0 || x < 0 || y < 0 || (r << 1) > w || (r << 1) > h)
		return;

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
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawTab(int x, int y, int r, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0 || (r << 1) > w || (r << 1) > h)
		return;
	
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
	// Awesome hack: the AA messes up the last pixel triangles if their width is even
	// ...fix the width instead of fixing the AA :p
	if (w % 2 == 0) {
		w++; h++;
	}
	
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

	switch(orient) {
		case kTriangleUp:
		case kTriangleDown:
#ifdef VECTOR_RENDERER_FAST_TRIANGLES
			if (w == h)
				drawTriangleFast(x, y, w, (orient == kTriangleDown), color, Base::_fillMode);
			else 
#endif
				drawTriangleVertAlg(x, y, w, h, (orient == kTriangleDown), color, Base::_fillMode);
			break;

		case kTriangleLeft:
		case kTriangleRight:
			break;
	}

	if (Base::_strokeWidth > 0)
		if (Base::_fillMode == kFillBackground || Base::_fillMode == kFillGradient) {
#ifdef VECTOR_RENDERER_FAST_TRIANGLES
			if (w == h)
				drawTriangleFast(x, y, w, (orient == kTriangleDown), _fgColor, kFillDisabled);
			else
#endif
				drawTriangleVertAlg(x, y, w, h, (orient == kTriangleDown), _fgColor, kFillDisabled);
		}
}

/********************************************************************
 * Aliased Primitive drawing ALGORITHMS - VectorRendererSpec
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
	PixelType color1, color2;
	
	if (fill_m == kFillForeground || fill_m == kFillBackground)
		color1 = color2 = color;
		
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
drawBevelSquareAlg(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color) {
	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
	int pitch = Base::surfacePitch();
	int i, j;
	
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
 * SHADOW drawing algorithms - VectorRendererSpec
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
	bool *hb = new bool[r];

	int i = 0;
	while (i < r)
		hb[i++] = false;

	while (x++ < y) {
		__BE_ALGORITHM();

		if (!hb[x]) {
			blendFill(ptr_tr - px - r, ptr_tr + y - px, 0, alpha);
			blendFill(ptr_bl - y + px, ptr_br + y + px, 0, alpha);
			hb[x] = true;
		}

		if (!hb[y]) {
			blendFill(ptr_tr - r - py, ptr_tr + x - py, 0, alpha);
			blendFill(ptr_bl - x + py, ptr_br + x + py, 0, alpha);
			hb[y] = true;
		}
	}

	delete[] hb;

	while (short_h--) {
		blendFill(ptr_fill - r, ptr_fill + blur, 0, alpha);
		ptr_fill += pitch;
	}
}


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
		
		while (x > y++) {
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

} // end of namespace Graphics
