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

namespace Graphics {

/**
 * VectorRenderer: The core Vector Renderer Class
 *
 * This virtual class which exposes the API with all the vectorial
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
	/**
	 * Draws a line by considering the special cases for optimization.
	 *
	 * @param x1 Horizontal (X) coordinate for the line start
	 * @param x2 Horizontal (X) coordinate for the line end
	 * @param y1 Vertical (Y) coordinate for the line start
	 * @param y2 Vertical (Y) coordinate for the line end
	 */
	virtual void drawLine( int x1, int x2, int y1, int y2 ) = 0;

	/**
	 * Gets the pixel pitch for the current drawing surface.
	 * Note: This is a real pixel-pitch, not a byte-pitch.
	 * That means it can be safely used in pointer arithmetics and
	 * in pixel manipulation.
	 *
	 * @return integer with the active bytes per pixel
	 */
	virtual uint16 surfacePitch() {
		return _activeSurface->pitch/_activeSurface->bytesPerPixel;
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
	virtual void setColor( uint8 r, uint8 g, uint8 b ) = 0;

	/**
	 * Set the active painting color for the renderer, including alpha
	 * intensity. All the drawing from then on will be done with that color, 
	 * unless specified otherwise.
	 *
	 * @param r	value of the red color byte
	 * @param g	value of the green color byte
	 * @param b	value of the blue color byte
	 * @param a value of the alpha byte
	 */
	virtual void setColor( uint8 r, uint8 g, uint8 b, uint8 a ) = 0;

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
	virtual void drawLineAlg( int x1, int x2, int y1, int y2, int dx, int dy ) = 0;

	virtual void drawCircleAlg( int x, int y, int r ) = 0;

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

	/**
	 * @see VectorRenderer::drawLine()
	 */
	void drawLine( int x1, int x2, int y1, int y2 );

	/**
	 * @see VectorRenderer::setColor()
	 */
	void setColor( uint8 r, uint8 g, uint8 b, uint8 a ) {
		_color = ARGBToColor<PixelFormat>(r, g, b, a);
	}

	/**
	 * @see VectorRenderer::setColor()
	 */
	void setColor( uint8 r, uint8 g, uint8 b ) {
		_color = RGBToColor<PixelFormat>(r, g, b);
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
	virtual void drawLineAlg( int x1, int x2, int y1, int y2, int dx, int dy );

	virtual void drawCircleAlg( int x, int y, int r ) {}

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
class VectorRendererAA : public VectorRendererSpec<PixelType,PixelFormat> {

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
	void drawLineAlg( int x1, int x2, int y1, int y2, int dx, int dy );

	/**
	 * Calculates the blending weight (relative luminosity) value for
	 * a given pixel, based on the distance to the ideal line.
	 * Used for blending pixels in the Wu AA algorithm.
	 *
	 * @param line_color Byte value of a color component (R/G/B) of the color used to draw the line.
	 * @param line_color Byte value of a color component (R/G/B) of the color used to draw the BG.
	 * @param weight Weight of the pixel as calculated by Wu's algorithm.
	 * @return The new color value for the given component.
	 */
	inline uint8 antialiasingBlendWeight( uint8 line_color, uint8 bg_color, uint weight ) {
		uint8 value;
		if ( bg_color > line_color ) {
			value = weight / 255 * ( bg_color - line_color ) + bg_color;  
		} else {
			value = weight / 255 * ( line_color - bg_color ) + line_color;
		}
		return value;
	}
};

} // end of namespace Graphics

#endif
