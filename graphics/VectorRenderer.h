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

#ifndef VECTOR_RENDERER_H
#define VECTOR_RENDERER_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "graphics/colormasks.h"
#include "common/system.h"

namespace Graphics {

void vector_renderer_test(OSystem *_system);

/**
 * VectorRenderer: The core Vector Renderer Class
 *
 * This virtual class exposes the API with all the vectorial
 * rendering functions that may be used to draw on a given Surface.
 *
 * This class must be instantiated as one of its children, which implement
 * the actual rendering functionality for each Byte Depth / Byte Format
 * combination, and may also contain platform specific code.
 *
 * TODO: Expand documentation.
 *
 * @see VectorRendererSpec
 * @see VectorRendererAA
 */
class VectorRenderer {

public:
	virtual ~VectorRenderer() {}

	/**
	 * Draws a line by considering the special cases for optimization.
	 *
	 * @param x1 Horizontal (X) coordinate for the line start
	 * @param x2 Horizontal (X) coordinate for the line end
	 * @param y1 Vertical (Y) coordinate for the line start
	 * @param y2 Vertical (Y) coordinate for the line end
	 */
	virtual void drawLine(int x1, int y1, int x2, int y2) = 0;

	/**
	 * Draws a circle centered at (x,y) with radius r.
	 *
	 * @param x Horizontal (X) coordinate for the center of the circle
	 * @param y Vertical (Y) coordinate for the center of the circle
	 * @param r Radius of the circle.
	 */
	virtual void drawCircle(int x, int y, int r) = 0;


	virtual void drawSquare(int x, int y, int w, int h, bool fill) = 0;

	/**
	 * Gets the pixel pitch for the current drawing surface.
	 * Note: This is a real pixel-pitch, not a byte-pitch.
	 * That means it can be safely used in pointer arithmetics and
	 * in pixel manipulation.
	 *
	 * @return integer with the active bytes per pixel
	 */
	virtual uint16 surfacePitch() {
		return _activeSurface->pitch / _activeSurface->bytesPerPixel;
	}

	/**
	 * Gets the BYTES (not bits) per Pixel we are working on,
	 * based on the active drawing surface.
	 *
	 * @return integer byte with the active bytes per pixel value
	 */
	virtual uint8 bytesPerPixel() {
		return _activeSurface->bytesPerPixel;
	}

	/**
	 * Set the active painting color for the renderer.
	 * All the drawing from then on will be done with that color, unless
	 * specified otherwise.
	 *
	 * @param r	value of the red color byte
	 * @param g	value of the green color byte
	 * @param b	value of the blue color byte
	 */
	virtual void setColor(uint8 r, uint8 g, uint8 b) = 0;

	/**
	 * Sets the active drawing surface. All drawing from this
	 * point on will be done on that surface.
	 *
	 * @param surface Pointer to a Surface object.
	 */
	virtual void setSurface(Surface *surface) {
		_activeSurface = surface;
	}

	/**
	 * Fills the active surface with the currently active drawing color.
	 */
	virtual void fillSurface() = 0;

	/**
	 * Clears the active surface.
	 */
	virtual void clearSurface() {
		byte *src = (byte *)_activeSurface->pixels;
		memset(src, 0, _activeSurface->w * _activeSurface->h * _activeSurface->bytesPerPixel);
	}

	/**
	 * Draws a single pixel on the surface with the given coordinates and
	 * the currently active drawing color.
	 *
	 * @param x Horizontal coordinate of the pixel.
	 * @param y Vertical coordinate of the pixel.
	 */
	inline virtual void putPixel(int x, int y) = 0;

	/**
	 * Blends a single pixel on the surface with the given coordinates, with
	 * the currently active drawing color and with the given Alpha intensity.
	 *
	 * @param x Horizontal coordinate of the pixel.
	 * @param y Vertical coordinate of the pixel.
	 * @param alpha Alpha intensity of the pixel (0-255)
	 */
	inline virtual void blendPixel(int x, int y, uint8 alpha) = 0;

protected:

	/**
	 * Generic line drawing algorithm. May be implemented by each
	 * inheriting class, i.e. with platform specific code.
	 *
	 * @see VectorRenderer::drawLine()
	 * @param x1 Horizontal (X) coordinate for the line start
	 * @param x2 Horizontal (X) coordinate for the line end
	 * @param y1 Vertical (Y) coordinate for the line start
	 * @param y2 Vertical (Y) coordinate for the line end
	 * @param dx Horizontal (X) increasement.
	 * @param dy Vertical (Y) increasement.
	 */
	virtual void drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy) = 0;

	/**
	 * Specific circle drawing algorithm with symmetry. Must be implemented
	 * on each renderer.
	 *
	 * @param x Horizontal (X) coordinate for the center of the circle
	 * @param y Vertical (Y) coordinate for the center of the circle
	 * @param r Radius of the circle.
	 */
	virtual void drawCircleAlg(int x, int y, int r) = 0;

	Surface *_activeSurface; /** Pointer to the surface currently being drawn */
};



/**
 * VectorRendererSpec: Specialized Vector Renderer Class
 *
 * This templated class implements the basic subset of vector operations for
 * all platforms by allowing the user to provide the actual Pixel Type and
 * pixel information structs.
 *
 * This class takes two template parameters:
 *
 * @param PixelType Defines a type which may hold the color value of a single
 *        pixel, such as "byte" or "uint16" for 8 and 16 BPP respectively.
 *
 * @param PixelFormat Defines the type of the PixelFormat struct which contains all
 *        the actual information of the pixels being used, as declared in "graphics/colormasks.h"
 *                    
 * TODO: Expand documentation.
 *
 * @see VectorRenderer
 */
template<typename PixelType, typename PixelFormat>
class VectorRendererSpec : public VectorRenderer {
	typedef VectorRenderer Base;

public:
	/**
	 * @see VectorRenderer::drawLine()
	 */
	void drawLine(int x1, int y1, int x2, int y2);

	void drawCircle(int x, int y, int r) {
		drawCircleAlg(x, y, r);
	}

	void drawSquare(int x, int y, int w, int h, bool fill);

/**
	 * @see VectorRenderer::setColor()
	 */
	void setColor(uint8 r, uint8 g, uint8 b) {
        this->_color = RGBToColor<PixelFormat>(r, g, b);
	}

	/**
	 * @see VectorRenderer::fillSurface()
	 */
	void fillSurface() {
		PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(0, 0);
		int s = _activeSurface->w * _activeSurface->h;
		Common::set_to(ptr, ptr + s, (PixelType)_color);
	}

	/**
	 * @see VectorRenderer::putPixel()
	 */
	inline void putPixel( int x, int y ) {
		PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
		*ptr = _color;
	}

	/**
	 * On the Specialized Renderer, alpha blending is not supported. 
	 *
	 * @see VectorRenderer::blendPixel()
	 */
	virtual inline void blendPixel(int x, int y, uint8 alpha) {
			putPixel(x, y);
	}

protected:

	/*
	 * "Bresenham's Line Algorithm", as described in Wikipedia.
	 * Based on the current implementation in "graphics/primitives.cpp".
	 *
	 * Generic line drawing algorithm for the aliased renderer. Optimized with no
	 * floating point operations and direct access to pixel buffer, assumes no special cases.
	 *
	 * @see VectorRenderer::drawLineAlg()
	 */
	virtual void drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy);

	/**
	 * @see VectorRenderer::drawCircleAlg()
	 */
	virtual void drawCircleAlg(int x, int y, int r);

	PixelType _color; /** Color currently being used to draw on the renderer */
};

/**
 * VectorRendererAA: Anti-Aliased Vector Renderer Class
 *
 * This templated class inherits all the functionality of the VectorRendererSpec
 * class but uses better looking yet slightly slower AA algorithms for drawing
 * most primivitves. May be used in faster platforms.
 *                    
 * TODO: Expand documentation.
 *
 * @see VectorRenderer
 * @see VectorRendererSpec
 */
template<typename PixelType, typename PixelFormat>
class VectorRendererAA : public VectorRendererSpec<PixelType, PixelFormat> {
	typedef VectorRendererSpec<PixelType, PixelFormat> Base;
protected:
	/**
	 * "Wu's Line Antialiasing Algorithm" as published by Xiaolin Wu, July 1991
	 * Based on the implementation found in Michael Abrash's Graphics Programming Black Book.
	 *
	 * Generic line drawing algorithm for the Antialiased renderer. Optimized with no
	 * floating point operations, assumes no special cases.
	 *
	 * @see VectorRenderer::drawLineAlg()
	 */
	void drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy);

	/**
	 * Perform alpha blending on top of a given pixel, not on a given
	 * coordinate (just so we don't have to recalculate the surface
	 * pointer while we are blending consecutive pixels).
	 *
	 * Everything from blendPixel() applies here.
	 *
	 * @see VectorRenderer::blendPixel()
	 * @param ptr Pointer to the pixel where we must draw
	 * @param alpha Intensity of the pixel (0-255).
	 */
	inline void blendPixelPtr(PixelType *ptr, uint8 alpha);

	/**
	 * @see VectorRenderer::blendPixel()
	 *
	 * The AA renderer does support alpha blending. Special cases are
	 * handled separately.
	 */
	inline void blendPixel(int x, int y, uint8 alpha) {
		if (alpha == 0)
			return;
		else if (alpha < 255)
			blendPixelPtr((PixelType*)Base::_activeSurface->getBasePtr(x, y), alpha);
		else
			Base::putPixel(x, y);
	}

	/**
	 * "Wu's Circle Antialiasing Algorithm" as published by Xiaolin Wu, July 1991
	 * Based on the theoretical concept of the algorithm.
	 *
	 * Implementation of Wu's algorithm for circles using fixed point arithmetics.
	 * Could be quite fast.
	 *
	 * @see VectorRenderer::drawCircleAlg()
	 */
	virtual void drawCircleAlg(int x, int y, int r);
};

} // end of namespace Graphics

#endif
