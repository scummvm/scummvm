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

#ifndef VECTOR_RENDERER_SPEC_H
#define VECTOR_RENDERER_SPEC_H

#include "graphics/VectorRenderer.h"

namespace Graphics {

/**
 * @defgroup graphics_vector_renderer_spec Specialized vector renderer
 * @ingroup graphics
 *
 * @brief VectorRendererSpec class.
 *
 * @{
 */

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
 * TODO: Expand documentation.
 *
 * @see VectorRenderer
 */
template<typename PixelType>
class VectorRendererSpec : public VectorRenderer {
	typedef VectorRenderer Base;

public:
	VectorRendererSpec(PixelFormat format);

	void drawLine(int x1, int y1, int x2, int y2) override;
	void drawCircle(int x, int y, int r) override;
	void drawSquare(int x, int y, int w, int h) override;
	void drawRoundedSquare(int x, int y, int r, int w, int h) override;
	void drawTriangle(int x, int y, int base, int height, TriangleOrientation orient) override;
	void drawTab(int x, int y, int r, int w, int h) override;

	void drawBeveledSquare(int x, int y, int w, int h) override {
		bool useClippingVersions = !_clippingArea.contains(Common::Rect(x, y, x + w, y + h));
		if (useClippingVersions) {
			drawBevelSquareAlgClip(x, y, w, h, _bevel, _bevelColor, _fgColor);
		} else {
			drawBevelSquareAlg(x, y, w, h, _bevel, _bevelColor, _fgColor);
		}
	}
	void drawString(const Graphics::Font *font, const Common::U32String &text,
		const Common::Rect &area, Graphics::TextAlign alignH, GUI::ThemeEngine::TextAlignVertical alignV,
		int deltax, bool elipsis, const Common::Rect &textDrawableArea = Common::Rect(0, 0, 0, 0)) override;

	void setFgColor(uint8 r, uint8 g, uint8 b) override { _fgColor = _format.RGBToColor(r, g, b); }
	void setBgColor(uint8 r, uint8 g, uint8 b) override { _bgColor = _format.RGBToColor(r, g, b); }
	void setBevelColor(uint8 r, uint8 g, uint8 b) override { _bevelColor = _format.RGBToColor(r, g, b); }
	void setGradientColors(uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2) override;
	void setClippingRect(const Common::Rect &clippingArea) override { _clippingArea = clippingArea; }

	void copyFrame(OSystem *sys, const Common::Rect &r) override;
	void copyWholeFrame(OSystem *sys) override { copyFrame(sys, Common::Rect(0, 0, _activeSurface->w, _activeSurface->h)); }

	void fillSurface() override;
	void blitSurface(const Graphics::Surface *source, const Common::Rect &r) override;
	void blitSubSurface(const Graphics::Surface *source, const Common::Point &p) override;
	void blitKeyBitmap(const Graphics::Surface *source, const Common::Point &p) override;
	void blitAlphaBitmap(Graphics::TransparentSurface *source, const Common::Rect &r,
			GUI::ThemeEngine::AutoScaleMode autoscale = GUI::ThemeEngine::kAutoScaleNone,
			Graphics::DrawStep::VectorAlignment xAlign = Graphics::DrawStep::kVectorAlignManual,
			Graphics::DrawStep::VectorAlignment yAlign = Graphics::DrawStep::kVectorAlignManual,
			int alpha = 255) override;

	void applyScreenShading(GUI::ThemeEngine::ShadingStyle shadingStyle) override;

protected:

	Common::Rect _clippingArea;

	/**
	 * Draws a single pixel on the surface with the given coordinates and
	 * the given color.
	 *
	 * @param x Horizontal coordinate of the pixel.
	 * @param y Vertical coordinate of the pixel.
	 * @param color Color of the pixel
	 */
	inline void putPixel(int x, int y, PixelType color) {
		PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
		*ptr = color;
	}

	/**
	 * Blends a single pixel on the surface with the given coordinates, color
	 * and Alpha intensity.
	 *
	 * @param x Horizontal coordinate of the pixel.
	 * @param y Vertical coordinate of the pixel.
	 * @param color Color of the pixel
	 * @param alpha Alpha intensity of the pixel (0-255)
	 */
	inline void blendPixel(int x, int y, PixelType color, uint8 alpha) {
		blendPixelPtr((PixelType *)Base::_activeSurface->getBasePtr(x, y), color, alpha);
	}

	/**
	 * Blends a single pixel on the surface in the given pixel pointer, using supplied color
	 * and Alpha intensity.
	 *
	 * This is implemented to prevent blendPixel() to calculate the surface pointer on each call.
	 * Optimized drawing algorithms should call this function when possible.
	 *
	 * @see blendPixel
	 * @param ptr Pointer to the pixel to blend on top of
	 * @param color Color of the pixel
	 * @param alpha Alpha intensity of the pixel (0-255)
	 */
	inline void blendPixelPtr(PixelType *ptr, PixelType color, uint8 alpha);
	inline void blendPixelPtrClip(PixelType *ptr, PixelType color, uint8 alpha, int x, int y);

	/**
	 * Blends a single pixel on the surface in the given pixel pointer, using supplied color
	 * and Alpha intensity.
	 * If the destination pixel has 0 alpha, set the color and alpha channels,
	 * overwriting the destination pixel entirely.
	 * If the destination pixel has non-zero alpha, blend dest with src.
	 *
	 * This is implemented to prevent blendPixel() to calculate the surface pointer on each call.
	 * Optimized drawing algorithms should call this function when possible.
	 *
	 * @see blendPixel
	 * @param ptr Pointer to the pixel to blend on top of
	 * @param color Color of the pixel
	 * @param alpha Alpha intensity of the pixel (0-255)
	 */
	inline void blendPixelDestAlphaPtr(PixelType *ptr, PixelType color, uint8 alpha);


	/**
	 * PRIMITIVE DRAWING ALGORITHMS
	 *
	 * Generic algorithms for drawing all kinds of aliased primitive shapes.
	 * These may be overloaded in inheriting classes to implement platform-specific
	 * optimizations or improve looks.
	 *
	 * @see VectorRendererAA
	 * @see VectorRendererAA::drawLineAlg
	 * @see VectorRendererAA::drawCircleAlg
	 */
	virtual void drawLineAlg(int x1, int y1, int x2, int y2,
	    uint dx, uint dy, PixelType color);

	virtual void drawLineAlgClip(int x1, int y1, int x2, int y2,
		uint dx, uint dy, PixelType color);

	virtual void drawCircleAlg(int x, int y, int r,
	    PixelType color, FillMode fill_m);

	virtual void drawCircleAlgClip(int x, int y, int r,
		PixelType color, FillMode fill_m);

	virtual void drawRoundedSquareAlg(int x1, int y1, int r, int w, int h,
	    PixelType color, FillMode fill_m);

	virtual void drawRoundedSquareAlgClip(int x1, int y1, int r, int w, int h,
		PixelType color, FillMode fill_m);

	virtual void drawBorderRoundedSquareAlg(int x1, int y1, int r, int w, int h,
	    PixelType color, FillMode fill_m, uint8 alpha_t, uint8 alpha_r, uint8 alpha_b, uint8 alpha_l);

	virtual void drawBorderRoundedSquareAlgClip(int x1, int y1, int r, int w, int h,
		PixelType color, FillMode fill_m, uint8 alpha_t, uint8 alpha_r, uint8 alpha_b, uint8 alpha_l);

	virtual void drawInteriorRoundedSquareAlg(int x1, int y1, int r, int w, int h,
	    PixelType color, FillMode fill_m);

	virtual void drawInteriorRoundedSquareAlgClip(int x1, int y1, int r, int w, int h,
		PixelType color, FillMode fill_m);

	virtual void drawSquareAlg(int x, int y, int w, int h,
	    PixelType color, FillMode fill_m);

	virtual void drawSquareAlgClip(int x, int y, int w, int h,
		PixelType color, FillMode fill_m);

	virtual void drawTriangleVertAlg(int x, int y, int w, int h,
	    bool inverted, PixelType color, FillMode fill_m);

	virtual void drawTriangleVertAlgClip(int x, int y, int w, int h,
		bool inverted, PixelType color, FillMode fill_m);

	virtual void drawTriangleFast(int x, int y, int size,
	    bool inverted, PixelType color, FillMode fill_m);

	virtual void drawBevelSquareAlg(int x, int y, int w, int h,
	    int bevel, PixelType top_color, PixelType bottom_color);

	virtual void drawBevelSquareAlgClip(int x, int y, int w, int h,
		int bevel, PixelType top_color, PixelType bottom_color);

	virtual void drawTabAlg(int x, int y, int w, int h, int r,
	    PixelType color, VectorRenderer::FillMode fill_m,
	    int baseLeft = 0, int baseRight = 0);

	virtual void drawTabAlgClip(int x, int y, int w, int h, int r,
		PixelType color, VectorRenderer::FillMode fill_m,
		int baseLeft = 0, int baseRight = 0);

	virtual void drawTabShadow(int x, int y, int w, int h, int r);

	virtual void drawTabShadowClip(int x, int y, int w, int h, int r);

	virtual void drawBevelTabAlg(int x, int y, int w, int h,
	    int bevel, PixelType topColor, PixelType bottomColor,
	    int baseLeft = 0, int baseRight = 0);

	virtual void drawBevelTabAlgClip(int x, int y, int w, int h,
		int bevel, PixelType topColor, PixelType bottomColor,
		int baseLeft = 0, int baseRight = 0);

	/**
	 * SHADOW DRAWING ALGORITHMS
	 *
	 * Optimized versions of the primitive drawing algorithms with alpha blending
	 * for shadow drawing.
	 * There functions may be overloaded in inheriting classes to improve performance
	 * in the slowest platforms where pixel alpha blending just doesn't cut it.
	 *
	 * @param offset Intensity/size of the shadow.
	 */
	virtual void drawSquareShadow(int x, int y, int w, int h, int offset);
	virtual void drawSquareShadowClip(int x, int y, int w, int h, int offset);
	virtual void drawRoundedSquareShadow(int x, int y, int r, int w, int h, int offset);
	virtual void drawRoundedSquareShadowClip(int x, int y, int r, int w, int h, int offset);

	/**
	 * Calculates the color gradient on a given point.
	 * This function assumes that the gradient start/end colors have been set
	 * beforehand from the API function call.
	 *
	 * @param pos Progress of the gradient.
	 * @param max Maximum amount of the progress.
	 * @return Composite color of the gradient at the given "progress" amount.
	 */
	inline PixelType calcGradient(uint32 pos, uint32 max);

	void precalcGradient(int h);
	void gradientFill(PixelType *first, int width, int x, int y);
	void gradientFillClip(PixelType *first, int width, int x, int y, int realX, int realY);

	/**
	 * Fills several pixels in a row with a given color and the specified alpha blending.
	 *
	 * @see blendPixelPtr
	 * @see blendPixel
	 * @param first Pointer to the first pixel to fill.
	 * @param last Pointer to the last pixel to fill.
	 * @param color Color of the pixel
	 * @param alpha Alpha intensity of the pixel (0-255)
	 */
	inline void blendFill(PixelType *first, PixelType *last, PixelType color, uint8 alpha) {
		while (first != last)
			blendPixelPtr(first++, color, alpha);
	}

	inline void blendFillClip(PixelType *first, PixelType *last, PixelType color, uint8 alpha, int realX, int realY) {
		if (_clippingArea.top <= realY && realY < _clippingArea.bottom) {
			while (first != last) {
				if (_clippingArea.left <= realX && realX < _clippingArea.right)
					blendPixelPtr(first++, color, alpha);
				else
					++first;
				++realX;
			}
		}
	}

	void darkenFill(PixelType *first, PixelType *last);
	void darkenFillClip(PixelType *first, PixelType *last, int x, int y);

	const PixelFormat _format;
	const PixelType _redMask, _greenMask, _blueMask, _alphaMask;

	PixelType _fgColor; /**< Foreground color currently being used to draw on the renderer */
	PixelType _bgColor; /**< Background color currently being used to draw on the renderer */

	PixelType _gradientStart; /**< Start color for the fill gradient */
	PixelType _gradientEnd; /**< End color for the fill gradient */

	int _gradientBytes[3]; /**< Color bytes of the active gradient, used to speed up calculation */

	Common::Array<PixelType> _gradCache;
	Common::Array<int> _gradIndexes;

	PixelType _bevelColor;
	PixelType _bitmapAlphaColor;
};


#ifndef DISABLE_FANCY_THEMES
/**
 * VectorRendererAA: Anti-Aliased Vector Renderer Class
 *
 * This templated class inherits all the functionality of the VectorRendererSpec
 * class but uses better looking yet slightly slower AA algorithms for drawing
 * most primitives. May be used in faster platforms.
 *
 * TODO: Expand documentation.
 *
 * @see VectorRenderer
 * @see VectorRendererSpec
 */
template<typename PixelType>
class VectorRendererAA : public VectorRendererSpec<PixelType> {
	typedef VectorRendererSpec<PixelType> Base;
public:
	VectorRendererAA(PixelFormat format) : VectorRendererSpec<PixelType>(format) {
	}

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
	virtual void drawLineAlg(int x1, int y1, int x2, int y2, uint dx, uint dy, PixelType color);

	/**
	 * "Wu's Circle Antialiasing Algorithm" as published by Xiaolin Wu, July 1991
	 * Based on the theoretical concept of the algorithm.
	 *
	 * Implementation of Wu's algorithm for circles using fixed point arithmetics.
	 * Could be quite fast.
	 *
	 * @see VectorRenderer::drawCircleAlg()
	 */
	virtual void drawCircleAlg(int x, int y, int r, PixelType color, VectorRenderer::FillMode fill_m);

	/**
	 * "Wu's Circle Antialiasing Algorithm" as published by Xiaolin Wu, July 1991,
	 * modified with corner displacement to allow drawing of squares with rounded
	 * corners.
	 *
	 * @see VectorRenderer::drawRoundedAlg()
	 */
	virtual void drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m);

	virtual void drawBorderRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m, uint8 alpha_t, uint8 alpha_l, uint8 alpha_r, uint8 alpha_b);

	virtual void drawInteriorRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m);

	virtual void drawRoundedSquareShadow(int x, int y, int r, int w, int h, int offset) {
		Base::drawRoundedSquareShadow(x, y, r, w, h, offset);
	}

	virtual void drawTabAlg(int x, int y, int w, int h, int r,
	    PixelType color, VectorRenderer::FillMode fill_m,
	    int baseLeft = 0, int baseRight = 0);
};
#endif
/** @} */
}
#endif
