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
#include "graphics/surface.h"
#include "graphics/VectorRenderer.h"
#include "graphics/colormasks.h"
#include "common/system.h"
#include "common/events.h"

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

namespace Graphics {

VectorRenderer *createRenderer() {
	return new VectorRendererAA<uint16, ColorMasks<565> >;
}


void vector_renderer_test(OSystem *_system) {
	Common::EventManager *eventMan = _system->getEventManager();

	VectorRenderer *vr = createRenderer();

	Surface _screen;
	_screen.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));

	if (!_screen.pixels)
		return;

	_system->clearOverlay();
	_system->grabOverlay((OverlayColor*)_screen.pixels, _screen.w);

	vr->setSurface(&_screen);
	vr->clearSurface();

	_system->showOverlay();

	while (true) { // draw!!
		vr->setFgColor(255, 0, 206);
		vr->fillSurface();

		vr->setFgColor(255, 247, 222);
		vr->setBgColor(25, 25, 175);
		vr->setGradientColors(206, 121, 99, 173, 40, 8);
		vr->setFillMode(VectorRenderer::kGradientFill);
		vr->setStrokeWidth(1);
		vr->shadowEnable(1, 1);

		vr->drawLine(25, 25, 125, 300);
		vr->drawCircle(250, 250, 100);
		vr->drawSquare(150, 25, 100, 75);
		vr->drawRoundedSquare(275, 25, 16, 128, 128);

		_system->copyRectToOverlay((OverlayColor*)_screen.getBasePtr(0, 0), _screen.w, 0, 0, _screen.w, _screen.w);
		_system->updateScreen();

		Common::Event event;
		_system->delayMillis(100);
		if (eventMan->pollEvent(event) && event.type == Common::EVENT_QUIT) {
			break;
		}
	}

	_system->hideOverlay();
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawCircle(int x, int y, int r) {
	if (Base::_fillMode != kNoFill && Base::_shadows) {
		drawCircleAlg(x + Base::_shadowXOffset + 1, y + Base::_shadowYOffset + 1, r, 0, kForegroundFill);
	}

	switch(Base::_fillMode) {
	case kNoFill:
		drawCircleAlg(x, y, r, _fgColor, kNoFill);
		break;

	case kForegroundFill:
		drawCircleAlg(x, y, r, _fgColor, kForegroundFill);
		break;

	case kBackgroundFill:
		drawCircleAlg(x, y, r, _fgColor, kForegroundFill);
		drawCircleAlg(x, y, r - Base::_strokeWidth, _bgColor, kBackgroundFill);
		break;

	case kGradientFill:
		break;
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawSquare(int x, int y, int w, int h) {
	if (Base::_fillMode != kNoFill && Base::_shadows) {
		drawSquareShadow(x, y, w, h);
	}

	switch(Base::_fillMode) {
	case kNoFill:
		drawSquareAlg(x, y, w, h, _fgColor, kNoFill);
		break;

	case kForegroundFill:
		drawSquareAlg(x, y, w, h, _fgColor, kForegroundFill);
		break;

	case kBackgroundFill:
		drawSquareAlg(x, y, w, h, _bgColor, kBackgroundFill);
		drawSquareAlg(x, y, w, h, _fgColor, kNoFill);
		break;

	case kGradientFill:
		VectorRendererSpec::drawSquareAlg(x, y, w, h, 0, kGradientFill);
		drawSquareAlg(x, y, w, h, _fgColor, kNoFill);
		break;
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawRoundedSquare(int x, int y, int r, int w, int h) {
	if (Base::_fillMode != kNoFill && Base::_shadows) {
		drawRoundedSquareShadow(x, y, r, w, h);
	}

	switch(Base::_fillMode) {
	case kNoFill:
		drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kNoFill);
		break;

	case kForegroundFill:
		drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kForegroundFill);
		break;

	case kBackgroundFill:
		VectorRendererSpec::drawRoundedSquareAlg(x, y, r, w, h, _bgColor, kBackgroundFill);
		drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kNoFill);
		break;

	case kGradientFill:
		if (Base::_strokeWidth > 1) {
			drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kForegroundFill);
			VectorRendererSpec::drawRoundedSquareAlg(x + Base::_strokeWidth/2, y + Base::_strokeWidth/2, 
				r - Base::_strokeWidth/2, w - Base::_strokeWidth, h - Base::_strokeWidth, 0, kGradientFill);
		} else {
			VectorRendererSpec::drawRoundedSquareAlg(x, y, r, w, h, 0, kGradientFill);
			drawRoundedSquareAlg(x, y, r, w, h, _fgColor, kNoFill);
		}
		break;
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawSquareAlg(int x, int y, int w, int h, PixelType color, FillMode fill_m) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
	int pitch = Base::surfacePitch();
	int max_h = h;
	
	if (fill_m != kNoFill) {
		while (h--) {
			if (fill_m == kGradientFill)
				color = calcGradient(max_h - h, max_h);

			Common::set_to(ptr, ptr + w, color);
			ptr += pitch;
		}
	} else {
		int sw = Base::_strokeWidth, sp = 0, hp = pitch * (h - 1);

		while (sw--) {
			Common::set_to(ptr + sp, ptr + w + sp, color);
			Common::set_to(ptr + hp - sp, ptr + w + hp - sp, color);
			sp += pitch;
		}

		while (h--) {
			Common::set_to(ptr, ptr + Base::_strokeWidth, color);
			Common::set_to(ptr + w - Base::_strokeWidth, ptr + w, color);
			ptr += pitch;
		}
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawSquareShadow(int x, int y, int w, int h) {
	int blur = 8;
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


template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
blendPixelPtr(PixelType *ptr, PixelType color, uint8 alpha) {
	if (alpha == 255) {
		*ptr = color;
		return;
	}

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

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawLine(int x1, int y1, int x2, int y2) {
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

	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = Base::surfacePitch();

	if (dy == 0) { // horizontal lines
		// these can be filled really fast with a single memset.
		// TODO: Platform specific ASM in set_to, would make this thing fly 
		Common::set_to(ptr, ptr + dx + 1, (PixelType)_fgColor);

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

#define __BE_RESET() { \
	f = 1 - r; \
	ddF_x = 0; ddF_y = -2 * r; \
	x = 0; y = r; px = 0; py = pitch * r; \
}


template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawCircleAlg(int x1, int y1, int r, PixelType color, FillMode fill_m) {
	int f, ddF_x, ddF_y;
	int x, y, px, py, sw = 0;
	int pitch = Base::surfacePitch();
	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	if (fill_m == kNoFill) {
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
		Common::set_to(ptr - r, ptr + r, color);
		__BE_RESET();

		while (x++ < y) {
			__BE_ALGORITHM();
			Common::set_to(ptr - x + py, ptr + x + py, color);
			Common::set_to(ptr - x - py, ptr + x - py, color);
			Common::set_to(ptr - y + px, ptr + y + px, color);
			Common::set_to(ptr - y - px, ptr + y - px, color);
		}
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawRoundedSquareShadow(int x1, int y1, int r, int w, int h) {

}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, FillMode fill_m) {
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

	if (fill_m == kNoFill) {
		while (sw++ < Base::_strokeWidth) {
			Common::set_to(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
			Common::set_to(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color);
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
			Common::set_to(ptr_fill, ptr_fill + Base::_strokeWidth, color);
			Common::set_to(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}
	} else {
		__BE_RESET();

		if (fill_m == kGradientFill) {
			while (x++ < y) {
				__BE_ALGORITHM();
				Common::set_to(ptr_tl - x - py, ptr_tr + x - py, calcGradient(real_radius - y, long_h));
				Common::set_to(ptr_tl - y - px, ptr_tr + y - px, calcGradient(real_radius - x, long_h));

				Common::set_to(ptr_bl - x + py, ptr_br + x + py, calcGradient(long_h - r + y, long_h));
				Common::set_to(ptr_bl - y + px, ptr_br + y + px, calcGradient(long_h - r + x, long_h));
			}
		} else {
			while (x++ < y) {
				__BE_ALGORITHM();			

				Common::set_to(ptr_tl - x - py, ptr_tr + x - py, color);
				Common::set_to(ptr_tl - y - px, ptr_tr + y - px, color);

				Common::set_to(ptr_bl - x + py, ptr_br + x + py, color);
				Common::set_to(ptr_bl - y + px, ptr_br + y + px, color);

				__BE_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py);
			}
		}

		ptr_fill += pitch * r;
		while (short_h--) {
			if (fill_m == kGradientFill)
				color = calcGradient(real_radius++, long_h);
			Common::set_to(ptr_fill, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}
	}
}

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

template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, FillMode fill_m) {
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

	if (fill_m == kNoFill) {
		while (sw++ < Base::_strokeWidth) {
			Common::set_to(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
			Common::set_to(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color);
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
			Common::set_to(ptr_fill, ptr_fill + Base::_strokeWidth, color);
			Common::set_to(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color);
			ptr_fill += p;
		}
	} else {
		x = r; y = 0; T = 0;
		px = p * x; py = 0;
		
		while (x > y++) {
			__WU_ALGORITHM();

			Common::set_to(ptr_tl - x - py, ptr_tr + x - py, color);
			Common::set_to(ptr_tl - y - px, ptr_tr + y - px, color);

			Common::set_to(ptr_bl - x + py, ptr_br + x + py, color);
			Common::set_to(ptr_bl - y + px, ptr_br + y + px, color);

			__WU_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1);
		}

		ptr_fill += p * r;
		while (short_h-- >= 0) {
			Common::set_to(ptr_fill, ptr_fill + w + 1, color);
			ptr_fill += p;
		}
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
drawCircleAlg(int x1, int y1, int r, PixelType color, FillMode fill_m) {
	int x, y, sw = 0;
	int p = Base::surfacePitch(), px, py;

	uint32 rsq = (r * r) << 16;
	uint32 T = 0, oldT;
	uint8 a1, a2;

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	if (fill_m == kNoFill) {
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
		Common::set_to(ptr - r, ptr + r + 1, color);
		x = r; y = 0; T = 0;
		px = p * x; py = 0;

		while (x > y++) {
			__WU_ALGORITHM();

			Common::set_to(ptr - x + py, ptr + x + py, color);
			Common::set_to(ptr - x - py, ptr + x - py, color);
			Common::set_to(ptr - y + px, ptr + y + px, color);
			Common::set_to(ptr - y - px, ptr + y - px, color);
				
			__WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py, a1);
		}				
	}
}

} // end of namespace Graphics