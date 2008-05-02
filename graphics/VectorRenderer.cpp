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

VectorRenderer *createRenderer() {
	return new VectorRendererSpec<uint16,ColorMasks<565>>;
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
	vr->setColor( 255, 255, 255 );

	_system->showOverlay();

	while( true ) { // draw!!
		vr->drawLine( 25, 100, 25, 150 );
		_system->copyRectToOverlay((OverlayColor*)_screen.getBasePtr(0, 0), _screen.w, 0, 0, _screen.w, _screen.w);
		_system->updateScreen();
		_system->delayMillis(100);
	}

	_system->hideOverlay();
}


template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType,PixelFormat>::
drawLineAlg(int x1, int x2, int y1, int y2, int dx, int dy) {
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
drawLineAlg(int x1, int x2, int y1, int y2, int dx, int dy) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = surfacePitch();
	int xdir = (x2 > x1) ? 1 : -1;

	int error_line, error_tmp, weight;
	int error_total = 0;
	uint8 line_r, line_g, line_b;
	uint8 bg_r, bg_g, bg_b;

	colorToRGB<PixelFormat>(_color, line_r, line_g, line_b);

	uint line_lum = (line_r >> 2) + (line_g >> 1) + (line_b >> 3);
	uint bg_lum;

	// first pixel, should be perfectly accurate so no fading out
	*ptr = (PixelType)_color; 

#define __WULINE_PUTPIXEL( pixel_ptr ) { \
		colorToRGB<PixelFormat>((PixelType)*(pixel_ptr), bg_r, bg_g, bg_b); \
		bg_lum = (bg_r >> 2) + (bg_g >> 1) + (bg_b >> 3); \
		weight = (line_lum < bg_lum) ? error_total >> 8 : (error_total >> 8)^0xFF; \
		*(pixel_ptr) = RGBToColor<PixelFormat>( \
			antialiasingBlendWeight(line_r, bg_r, weight), \
			antialiasingBlendWeight(line_g, bg_g, weight), \
			antialiasingBlendWeight(line_b, bg_b, weight)); \
	}

	// draw from top to bottom while fading out.
	// optimized for mostly vertical lines
	if (dy > dx) {
		error_line = (dx << 16) / dy;
		while (--dy) {
			error_tmp = error_total;
			error_total += error_line;

			if (error_total <= error_tmp)
				ptr += xdir; // move right or left

			ptr += pitch; // move down			

			__WULINE_PUTPIXEL(ptr);
			__WULINE_PUTPIXEL(ptr + xdir);
		}
	} else { // optimized for mostly horizontal lines
		error_line = (dy << 16) / dx;
		while (--dx) {
			error_tmp = error_total;
			error_total += error_line;

			if (error_total <= error_tmp)
				ptr += pitch; // move down

			ptr += xdir; // move left or right	

			__WULINE_PUTPIXEL(ptr);
			__WULINE_PUTPIXEL(ptr + pitch);
		}	
	} // end of line direction cases

	// last pixel, also perfectly accurate.
	ptr = (PixelType *)_activeSurface->getBasePtr(x2, y2);
	*ptr = (PixelType)_color;
}

template<typename PixelType, typename PixelFormat>
void VectorRendererSpec<PixelType,PixelFormat>::
drawLine(int x1, int x2, int y1, int y2) {
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
		drawLineAlg(x1, x2, y1, y2, dx, dy);
	}
}

} // end of namespace Graphics
