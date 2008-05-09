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
	vr->setColor(255, 0, 0);
	vr->fillSurface();
	vr->setColor(255, 255, 0);

	_system->showOverlay();

	while (true) { // draw!!
		vr->setColor(255, 255, 255);
		vr->fillSurface();
		vr->setColor(255, 0, 0 );
		vr->drawLine(25, 25, 125, 300);
		vr->drawCircle(250, 250, 100);
//		vr->drawSquare(150, 25, 100, 100, true);
		vr->drawRoundedSquare(150, 25, 8, 100, 75);
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
drawSquare(int x, int y, int w, int h, bool fill) {
	if ( fill ) {
		PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
		int pitch = Base::surfacePitch();

		while (h--) {
			Common::set_to(ptr, ptr + w, (PixelType)_color);
			ptr += pitch;
		}
	} else {
		drawLine(x, y, x + w, y);
		drawLine(x + w, y, x + w, y + w);
		drawLine(x, y + w, x + w, y + w);
		drawLine(x, y, x, y + w);
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType,PixelFormat>::
drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = Base::surfacePitch();
	int xdir = (x2 > x1) ? 1 : -1;

	*ptr = (PixelType)_color;

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
			*ptr = (PixelType)_color;
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
			*ptr = (PixelType)_color;
		}
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x2, y2);
	*ptr = (PixelType)_color;
}


template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
blendPixelPtr(PixelType *ptr, uint8 alpha) {
	register int idst = *ptr;
	register int isrc = Base::_color;

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
drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy) {

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);
	int pitch = Base::surfacePitch();
	int xdir = (x2 > x1) ? 1 : -1;
	uint16 error_tmp, error_acc, gradient;
	uint8 alpha;

	*ptr = (PixelType)Base::_color;

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

			blendPixelPtr(ptr, ~alpha);
			blendPixelPtr(ptr + pitch, alpha);
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

			blendPixelPtr(ptr, ~alpha);
			blendPixelPtr(ptr + xdir, alpha);
		}
	}

	Base::putPixel(x2, y2);
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
		Common::set_to(ptr, ptr + dx + 1, (PixelType)_color);

	} else if (dx == 0) { // vertical lines
		// these ones use a static pitch increase.
		while (y1++ <= y2) {
			*ptr = (PixelType)_color;
			ptr += pitch;
		}

	} else if (ABS(dx) == ABS(dy)) { // diagonal lines
		// these ones also use a fixed pitch increase
		pitch += (x2 > x1) ? 1 : -1;

		while (dy--) {
			*ptr = (PixelType)_color;
			ptr += pitch;
		}

	} else { // generic lines, use the standard algorithm...
		drawLineAlg(x1, y1, x2, y2, dx, dy);
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
drawCircleAlg(int x1, int y1, int r) {
	int f = 1 - r;
	int ddF_x = 0, ddF_y = -2 * r;
	int x = 0, y = r, px, py;
	int pitch = Base::surfacePitch();
	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);
	bool fill = true;

	px = 0;
	py = pitch * y;

	*(ptr + y) = _color;
	*(ptr - y) = _color;
	*(ptr + py) = _color;
	*(ptr - py) = _color;

	if (fill) Common::set_to(ptr - r, ptr + r, _color);

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
			Common::set_to(ptr - x + py, ptr + x + py, _color);
			Common::set_to(ptr - x - py, ptr + x - py, _color);
			Common::set_to(ptr - y + px, ptr + y + px, _color);
			Common::set_to(ptr - y - px, ptr + y - px, _color);
		}

		*(ptr + x + py) = _color;
		*(ptr + y - px) = _color;
		*(ptr - x - py) = _color; 
		*(ptr - y - px) = _color;
		*(ptr - y + px) = _color;
		*(ptr - x + py) = _color;
		*(ptr + y + px) = _color;
		*(ptr + x - py) = _color;
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h) {
	int x = r;
	int y = 0;
	int p = Base::surfacePitch(), px, py;
	uint32 rsq = (r * r) << 16;
	uint32 T = 0, oldT;
	uint8 a1, a2;
	bool fill = true;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	Common::set_to(ptr_fill + r, ptr_fill + w - r + 1, Base::_color);
	Common::set_to(ptr_fill + r + h*p, ptr_fill + w - r + 1 + h*p, Base::_color);

	h -= 2*r;
	ptr_fill += p*r;

	if (!fill) {
		while (h-- >= 0) {
			*(ptr_fill) = (PixelType)Base::_color;
			*(ptr_fill + w) = (PixelType)Base::_color;
			ptr_fill += p;
		}
	} else {
		while (h-- >= 0) {
			Common::set_to(ptr_fill, ptr_fill + w + 1, Base::_color);
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
			Common::set_to(ptr_tl - x - py, ptr_tr + x - py, Base::_color);
			Common::set_to(ptr_tl - y - px, ptr_tr + y - px, Base::_color);

			Common::set_to(ptr_bl - x + py, ptr_br + x + py, Base::_color);
			Common::set_to(ptr_bl - y + px, ptr_br + y + px, Base::_color);
		} else {
			blendPixelPtr(ptr_tr + y - (px-p), a2);
			blendPixelPtr(ptr_tr + x - 1 - py, a2);

			blendPixelPtr(ptr_tl - x + 1 - py, a2);
			blendPixelPtr(ptr_tl - y - (px-p), a2);

			blendPixelPtr(ptr_bl - y + (px-p), a2);
			blendPixelPtr(ptr_bl - x + 1 + py, a2);

			blendPixelPtr(ptr_br + x - 1 + py, a2);
			blendPixelPtr(ptr_br + y + (px-p), a2);
		} 

		blendPixelPtr(ptr_tr + y - px, a1);
		blendPixelPtr(ptr_tr + x - py, a1);

		blendPixelPtr(ptr_tl - x - py, a1);
		blendPixelPtr(ptr_tl - y - px, a1);

		blendPixelPtr(ptr_bl - y + px, a1);
		blendPixelPtr(ptr_bl - x + py, a1);

		blendPixelPtr(ptr_br + x + py, a1);
		blendPixelPtr(ptr_br + y + px, a1);
	}
}

template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType, PixelFormat>::
drawCircleAlg(int x1, int y1, int r) {
	int x = r;
	int y = 0;
	int p = Base::surfacePitch(), px = 0, py = 0;
	uint32 rsq = (r * r) << 16;
	uint32 T = 0, oldT;
	uint8 a1, a2;
	bool fill = false;

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);
	px = p * x;
	py = p * y;

	*(ptr + x) = (PixelType)Base::_color;
	*(ptr - x) = (PixelType)Base::_color;
	*(ptr + px) = (PixelType)Base::_color;
	*(ptr - px) = (PixelType)Base::_color;
	
	if (fill) Common::set_to(ptr - x, ptr + x, Base::_color);

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
			Common::set_to(ptr - x + py, ptr + x + py, Base::_color);
			Common::set_to(ptr - x - py, ptr + x - py, Base::_color);
			Common::set_to(ptr - y + px, ptr + y + px, Base::_color);
			Common::set_to(ptr - y - px, ptr + y - px, Base::_color);
		} else {
			blendPixelPtr(ptr + x - 1 + py, a2);
			blendPixelPtr(ptr + y - (px-p), a2);
			blendPixelPtr(ptr - x + 1 - py, a2);
			blendPixelPtr(ptr - y - (px-p), a2);
			blendPixelPtr(ptr - y + (px-p), a2);
			blendPixelPtr(ptr - x + 1 + py, a2);
			blendPixelPtr(ptr + y + (px-p), a2);
			blendPixelPtr(ptr + x - 1 - py, a2);
		}

		blendPixelPtr(ptr + x + py, a1);
		blendPixelPtr(ptr + y - px, a1);
		blendPixelPtr(ptr - x - py, a1); 
		blendPixelPtr(ptr - y - px, a1);
		blendPixelPtr(ptr - y + px, a1);
		blendPixelPtr(ptr - x + py, a1);
		blendPixelPtr(ptr + y + px, a1);
		blendPixelPtr(ptr + x - py, a1);
	}
}

} // end of namespace Graphics
