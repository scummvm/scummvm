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

namespace Graphics {

inline uint32 fp_sqroot(uint32 x);

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
		vr->setFgColor(255, 255, 255);
		vr->fillSurface();

		vr->setFgColor(255, 0, 0 );
		vr->setBgColor(25, 25, 175 );
		vr->setFillMode( VectorRenderer::kBackgroundFill );
		vr->shadowEnable( 5, 5 );

		vr->drawLine(25, 25, 125, 300);
		vr->drawCircle(250, 250, 100);
		vr->drawSquare(150, 25, 100, 75);
		vr->drawRoundedSquare(275, 25, 8, 100, 75);

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
drawSquareAlg(int x, int y, int w, int h, PixelType color, bool fill) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
	int pitch = Base::surfacePitch();
	
	if (fill) {
		while (h--) {
			Common::set_to(ptr, ptr + w, color);
			ptr += pitch;
		}
	} else {
		Common::set_to(ptr, ptr + w, color);
		Common::set_to(ptr + pitch * h, ptr + w + pitch * h, color);

		while (h--) {
			*ptr = color;
			*(ptr + w) = color;
			ptr += pitch;
		}
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
		testDIV = (root << 1 ) + 1;

		if (remHI >= testDIV) {
			remHI -= testDIV;
			root++;
		}
	} while (count--);

	return root;
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType, PixelFormat>::
drawCircleAlg(int x1, int y1, int r, PixelType color, bool fill) {
	int f = 1 - r;
	int ddF_x = 0, ddF_y = -2 * r;
	int x = 0, y = r, px, py;
	int pitch = Base::surfacePitch();
	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	px = 0;
	py = pitch * y;

	*(ptr + y) = color;
	*(ptr - y) = color;
	*(ptr + py) = color;
	*(ptr - py) = color;

	if (fill) Common::set_to(ptr - r, ptr + r, color);

	while (x++ < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
			py -= pitch;
		}

		px += pitch;
		ddF_x += 2;
		f += ddF_x + 1;

		if (fill) {
			Common::set_to(ptr - x + py, ptr + x + py, color);
			Common::set_to(ptr - x - py, ptr + x - py, color);
			Common::set_to(ptr - y + px, ptr + y + px, color);
			Common::set_to(ptr - y - px, ptr + y - px, color);
		}

		*(ptr + x + py) = color;
		*(ptr + y - px) = color;
		*(ptr - x - py) = color; 
		*(ptr - y - px) = color;
		*(ptr - y + px) = color;
		*(ptr - x + py) = color;
		*(ptr + y + px) = color;
		*(ptr + x - py) = color;
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, bool fill) {
	int x = r;
	int y = 0;
	int p = Base::surfacePitch(), px, py;
	uint32 rsq = (r * r) << 16;
	uint32 T = 0, oldT;
	uint8 a1, a2;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	Common::set_to(ptr_fill + r, ptr_fill + w - r + 1, color);
	Common::set_to(ptr_fill + r + h*p, ptr_fill + w - r + 1 + h*p, color);

	h -= 2*r;
	ptr_fill += p*r;

	if (!fill) {
		while (h-- >= 0) {
			*(ptr_fill) = (PixelType)color;
			*(ptr_fill + w) = (PixelType)color;
			ptr_fill += p;
		}
	} else {
		while (h-- >= 0) {
			Common::set_to(ptr_fill, ptr_fill + w + 1, color);
			ptr_fill += p;
		}
	}

	px = p * x;
	py = 0;

	while (x > y++)
	{
		oldT = T;
		T = fp_sqroot(rsq - ((y * y) << 16)) ^ 0xFFFF;

		py += p;

		if (T < oldT) {
			x--;
			px -= p;
		}

		a2 = (T >> 8);
		a1 = ~a2;

		if (fill) {
			Common::set_to(ptr_tl - x - py, ptr_tr + x - py, color);
			Common::set_to(ptr_tl - y - px, ptr_tr + y - px, color);

			Common::set_to(ptr_bl - x + py, ptr_br + x + py, color);
			Common::set_to(ptr_bl - y + px, ptr_br + y + px, color);
		} else {
			blendPixelPtr(ptr_tr + y - (px-p), color, a2);
			blendPixelPtr(ptr_tr + x - 1 - py, color, a2);

			blendPixelPtr(ptr_tl - x + 1 - py, color, a2);
			blendPixelPtr(ptr_tl - y - (px-p), color, a2);

			blendPixelPtr(ptr_bl - y + (px-p), color, a2);
			blendPixelPtr(ptr_bl - x + 1 + py, color, a2);

			blendPixelPtr(ptr_br + x - 1 + py, color, a2);
			blendPixelPtr(ptr_br + y + (px-p), color, a2);
		} 

		blendPixelPtr(ptr_tr + y - px, color, a1);
		blendPixelPtr(ptr_tr + x - py, color, a1);

		blendPixelPtr(ptr_tl - x - py, color, a1);
		blendPixelPtr(ptr_tl - y - px, color, a1);

		blendPixelPtr(ptr_bl - y + px, color, a1);
		blendPixelPtr(ptr_bl - x + py, color, a1);

		blendPixelPtr(ptr_br + x + py, color, a1);
		blendPixelPtr(ptr_br + y + px, color, a1);
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
drawCircleAlg(int x1, int y1, int r, PixelType color, bool fill) {
	int x = r;
	int y = 0;
	int p = Base::surfacePitch(), px = 0, py = 0;
	uint32 rsq = (r * r) << 16;
	uint32 T = 0, oldT;
	uint8 a1, a2;

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);
	px = p * x;
	py = p * y;

	*(ptr + x) = (PixelType)color;
	*(ptr - x) = (PixelType)color;
	*(ptr + px) = (PixelType)color;
	*(ptr - px) = (PixelType)color;
	
	if (fill) Common::set_to(ptr - x, ptr + x, color);

	while (x > y++)
	{
		oldT = T;
		T = fp_sqroot(rsq - ((y * y) << 16)) ^ 0xFFFF;

		py += p;

		if (T < oldT) {
			x--;
			px -= p;
		}

		a2 = (T >> 8);
		a1 = ~a2;

		if (fill) {
			Common::set_to(ptr - x + py, ptr + x + py, color);
			Common::set_to(ptr - x - py, ptr + x - py, color);
			Common::set_to(ptr - y + px, ptr + y + px, color);
			Common::set_to(ptr - y - px, ptr + y - px, color);
		} else {
			blendPixelPtr(ptr + x - 1 + py, color, a2);
			blendPixelPtr(ptr + y - (px-p), color, a2);
			blendPixelPtr(ptr - x + 1 - py, color, a2);
			blendPixelPtr(ptr - y - (px-p), color, a2);
			blendPixelPtr(ptr - y + (px-p), color, a2);
			blendPixelPtr(ptr - x + 1 + py, color, a2);
			blendPixelPtr(ptr + y + (px-p), color, a2);
			blendPixelPtr(ptr + x - 1 - py, color, a2);
		}

		blendPixelPtr(ptr + x + py, color, a1);
		blendPixelPtr(ptr + y - px, color, a1);
		blendPixelPtr(ptr - x - py, color, a1); 
		blendPixelPtr(ptr - y - px, color, a1);
		blendPixelPtr(ptr - y + px, color, a1);
		blendPixelPtr(ptr - x + py, color, a1);
		blendPixelPtr(ptr + y + px, color, a1);
		blendPixelPtr(ptr + x - py, color, a1);
	}
}

} // end of namespace Graphics
