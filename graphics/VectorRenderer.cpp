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

namespace Graphics {

inline uint32 fp_sqroot( uint32 x );

VectorRenderer *createRenderer() {
	return new VectorRendererAA<uint16,ColorMasks<565> >;
}


void vector_renderer_test( OSystem *_system ) {
	VectorRenderer *vr = createRenderer();

	Surface _screen;
	_screen.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));

	if (!_screen.pixels)
		return;

	_system->clearOverlay();
	_system->grabOverlay((OverlayColor*)_screen.pixels, _screen.w);

	vr->setSurface( &_screen );
	vr->setColor( 255, 0, 0 );
	vr->fillSurface();
	vr->setColor( 255, 255, 0 );

	_system->showOverlay();

	while( true ) { // draw!!

		vr->setColor( 255, 255, 255 );
		vr->fillSurface();
		vr->setColor( 255, 0, 0 );
		vr->drawLine( 25, 25, 125, 300 );
		vr->drawCircle( 250, 250, 100 );

		_system->copyRectToOverlay((OverlayColor*)_screen.getBasePtr(0, 0), _screen.w, 0, 0, _screen.w, _screen.w);
		_system->updateScreen();
		_system->delayMillis(100);
	}

	_system->hideOverlay();
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType,PixelFormat>::
drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = surfacePitch();
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
void VectorRendererAA<PixelType,PixelFormat>::
blendPixelPtr( PixelType *ptr, uint8 alpha ) {
	register int idst = *ptr;
	register int isrc = Base::_color;

	*ptr = (PixelType)(
		(PixelFormat::kRedMask & ((idst & PixelFormat::kRedMask) +
		((int)(((int)(isrc & PixelFormat::kRedMask) -
		(int)(idst & PixelFormat::kRedMask)) * alpha) >>8))) |
		(PixelFormat::kGreenMask & ((idst & PixelFormat::kGreenMask) +
		((int)(((int)(isrc & PixelFormat::kGreenMask) -
		(int)(idst & PixelFormat::kGreenMask)) * alpha) >>8))) |
		(PixelFormat::kBlueMask & ((idst & PixelFormat::kBlueMask) +
		((int)(((int)(isrc & PixelFormat::kBlueMask) -
		(int)(idst & PixelFormat::kBlueMask)) * alpha) >>8))) );
}


template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType,PixelFormat>::
drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy) {

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);
	int pitch = Base::surfacePitch();
	int xdir = (x2 > x1) ? 1 : -1;
	uint16 error_tmp, error_acc, gradient;

	*ptr = (PixelType)Base::_color;

	if ( dx > dy ) {
		gradient = (uint32)(dy<<16)/(uint32)dx;
		error_acc = 0;

		while( --dx ) {
			error_tmp = error_acc;
			error_acc += gradient;

			if ( error_acc <= error_tmp )
				ptr += pitch;

			ptr += xdir;

			blendPixelPtr( ptr, (error_acc >> 8) ^ 0xFF );
			blendPixelPtr( ptr + pitch, (error_acc >> 8) & 0xFF );
		}
	} else {
		gradient = (uint32)(dx<<16)/(uint32)dy;
		error_acc = 0;

		while( --dy ) {
			error_tmp = error_acc;
			error_acc += gradient;

			if ( error_acc <= error_tmp )
				ptr += xdir;

			ptr += pitch;

			blendPixelPtr( ptr, (error_acc >> 8) ^ 0xFF );
			blendPixelPtr( ptr + xdir, (error_acc >> 8) & 0xFF );
		}
	}

	Base::putPixel( x2, y2 );
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType,PixelFormat>::
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
	int pitch = surfacePitch();

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

inline uint32 fp_sqroot( uint32 x ) {
	register uint32 root, remHI, remLO, testDIV, count;

	root = 0;
	remHI = 0;
	remLO = x;
	count = 23;

	do {
		remHI = (remHI<<2) | (remLO>>30);
		remLO <<= 2;
		root <<= 1;
		testDIV = (root<<1) + 1;

		if ( remHI >= testDIV ) {
			remHI -= testDIV;
			root++;
		}
	} while( count-- );

	return root;
}

template<typename PixelType, typename PixelFormat>
void VectorRendererAA<PixelType,PixelFormat>::
drawCircleAlg(int x1, int y1, int r) {

#define __CIRCLE_SIM(x,y,a) { \
	blendPixel( x1 + (x), y1 + (y), a ); \
	blendPixel( x1 + (x), y1 - (y), a ); \
	blendPixel( x1 - (y), y1 + (x), a ); \
	blendPixel( x1 + (y), y1 + (x), a ); \
	blendPixel( x1 - (x), y1 + (y), a ); \
	blendPixel( x1 - (x), y1 - (y), a ); \
	blendPixel( x1 + (y), y1 - (x), a ); \
	blendPixel( x1 - (y), y1 - (x), a ); \
}

	// first quadrant
/*#define __CIRCLE_SIM(x,y,a) { \
	blendPixel( x1 + (x), y1 - (y), a ); \
	blendPixel( x1 + (y), y1 - (x), a ); \
}*/

	int x = r;
	int y = 0;
	uint32 rsq = (r*r)<<16;
	uint32 T = 0, oldT;
	
	__CIRCLE_SIM( x, y, 255 );

	while( x > y++ )
	{
		oldT = T;
		T = fp_sqroot( rsq - ((y*y)<<16) ) ^ 0xFFFF;

		if ( T < oldT )
			x--;

		__CIRCLE_SIM( x,   y, (T>>8) ^ 0xFF );
		__CIRCLE_SIM( x-1, y, (T>>8) & 0xFF );
	}
}

} // end of namespace Graphics
