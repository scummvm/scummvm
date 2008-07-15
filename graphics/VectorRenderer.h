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
#include "common/system.h"

#include "graphics/surface.h"
#include "graphics/colormasks.h"

#include "gui/ThemeRenderer.h"


namespace Graphics {
class VectorRenderer;
struct DrawStep;

struct TextStep {
	struct { 
		uint8 r, g, b;
		bool set;
	}
	color; /** text color */

	GUI::Theme::TextAlign alignHorizontal;
	GUI::Theme::TextAlignVertical alignVertical;
	char *text;
	const Graphics::Font *font;
};

struct DrawStep {
	struct { 
		uint8 r, g, b;
		bool set;
	}	
	fgColor, /** Foreground color */
	bgColor, /** backgroudn color */
	gradColor1, /** gradient start*/
	gradColor2; /** gradient end */

	bool autoWidth, autoHeight;
	int16 x, y, w, h; /** width, height and position, if not measured automatically.
	 					  negative values mean counting from the opposite direction */
	
	enum VectorAlignment {
		kVectorAlignManual,
		kVectorAlignLeft,
		kVectorAlignRight,
		kVectorAlignBottom,
		kVectorAlignTop,
		kVectorAlignCenter
	} xAlign, yAlign;

	uint8 shadow, stroke, factor, radius; /** Misc options... */

	uint8 fillMode; /** active fill mode */
	uint8 extraData; /** Generic parameter for extra options (orientation/bevel) */

	uint32 scale; /** scale of all the coordinates in FIXED POINT with 16 bits mantissa */

	void (VectorRenderer::*drawingCall)(const Common::Rect &, const DrawStep &); /** Pointer to drawing function */
};

VectorRenderer *createRenderer(int mode);

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
	VectorRenderer() : _shadowOffset(0), _fillMode(kFillDisabled), 
		_activeSurface(NULL), _strokeWidth(1), _gradientFactor(1) {
	
	}

	virtual ~VectorRenderer() {}

	/** Specifies the way in which a shape is filled */
	enum FillMode {
		kFillDisabled = 0,
		kFillForeground = 1,
		kFillBackground = 2,
		kFillGradient = 3
	};

	enum TriangleOrientation {
		kTriangleUp,
		kTriangleDown,
		kTriangleLeft,
		kTriangleRight
	};

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

	/**
	 * Draws a square starting at (x,y) with the given width and height.
	 *
	 * @param x Horizontal (X) coordinate for the center of the square
	 * @param y Vertical (Y) coordinate for the center of the square
	 * @param w Width of the square.
	 * @param h Height of the square
	 */
	virtual void drawSquare(int x, int y, int w, int h) = 0;

	/**
	 * Draws a rounded square starting at (x,y) with the given width and height.
	 * The corners of the square are rounded with the given radius.
	 *
	 * @param x Horizontal (X) coordinate for the center of the square
	 * @param y Vertical (Y) coordinate for the center of the square
	 * @param w Width of the square.
	 * @param h Height of the square
	 * @param r Radius of the corners.
	 */
	virtual void drawRoundedSquare(int x, int y, int r, int w, int h) = 0;

	/**
	 * Draws a triangle starting at (x,y) with the given base and height.
	 * The triangle will always be isosceles, with the given base and height.
	 * The orientation parameter controls the position of the base of the triangle.
	 *
	 * @param x Horizontal (X) coordinate for the top left corner of the triangle
	 * @param y Vertical (Y) coordinate for the top left corner of the triangle
	 * @param base Width of the base of the triangle
	 * @param h Height of the triangle
	 * @param orient Orientation of the triangle.
	 */
	virtual void drawTriangle(int x, int y, int base, int height, TriangleOrientation orient) = 0;

	/**
	 * Draws a beveled square like the ones in the Classic GUI themes.
	 * Beveled squares are always drawn with a transparent background. Draw them on top
	 * of a standard square to fill it.
	 *
	 * @param x Horizontal (X) coordinate for the center of the square
	 * @param y Vertical (Y) coordinate for the center of the square
	 * @param w Width of the square.
	 * @param h Height of the square
	 * @param bevel Amount of bevel. Must be positive.
	 */
	virtual void drawBeveledSquare(int x, int y, int w, int h, int bevel) = 0;

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
	 * Set the active foreground painting color for the renderer.
	 * All the foreground drawing from then on will be done with that color, unless
	 * specified otherwise.
	 *
	 * Foreground drawing means all outlines and basic shapes.
	 *
	 * @param r	value of the red color byte
	 * @param g	value of the green color byte
	 * @param b	value of the blue color byte
	 */
	virtual void setFgColor(uint8 r, uint8 g, uint8 b) = 0;

	/**
	 * Set the active background painting color for the renderer.
	 * All the background drawing from then on will be done with that color, unless
	 * specified otherwise.
	 *
	 * Background drawing means all the shape filling.
	 *
	 * @param r	value of the red color byte
	 * @param g	value of the green color byte
	 * @param b	value of the blue color byte
	 */
	virtual void setBgColor(uint8 r, uint8 g, uint8 b) = 0;

	/**
	 * Set the active gradient color. All shapes drawn using kFillGradient
	 * as their fill mode will use this VERTICAL gradient as their fill color.
	 *
	 * @param r1	value of the red color byte for the start color
	 * @param g1	value of the green color byte for the start color
	 * @param b1	value of the blue color byte for the start color
	 * @param r2	value of the red color byte for the end color
	 * @param g2	value of the green color byte for the end color
	 * @param b2	value of the blue color byte for the end color
	 */
	virtual void setGradientColors(uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2) = 0;

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
	 * Fills the active surface with the specified fg/bg color or the active gradient.
	 * Defaults to using the active Foreground color for filling.
	 *
	 * @param mode Fill mode (bg, fg or gradient) used to fill the surface
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
	 * Sets the active fill mode for all shapes.
	 *
	 * @see VectorRenderer::FillMode
	 * @param mode Specified fill mode.
	 */
	virtual void setFillMode(FillMode mode) {
		_fillMode = mode;
	}

	/**
	 * Sets the stroke width. All shapes drawn with a stroke will
	 * have that width. Pass 0 to disable shape stroking.
	 *
	 * @param width Witdh of the stroke in pixels.
	 */
	virtual void setStrokeWidth(int width) {
		_strokeWidth = width;
	}

	/**
	 * Enables adding shadows to all drawn primitives.
	 * Shadows are drawn automatically under the shapes. The given offset
	 * controls their intensity and size (the higher the offset, the
	 * bigger the shadows).
	 *
	 * @param offset Shadow offset.
	 * @see shadowDisable()
	 */
	virtual void shadowEnable(int offset) {
		if (offset >= 0)
			_shadowOffset = offset;
	}

	/**
	 * Disables adding shadows to all drawn primitives.
	 *
	 * @see shadowEnable()
	 */
	virtual void shadowDisable() {
		_shadowOffset = 0;
	}

	/**
	 * Sets the multiplication factor of the active gradient.
	 *
	 * @see _gradientFactor
	 * @param factor Multiplication factor.
	 */
	virtual void setGradientFactor(int factor) {
		if (factor > 0)
			_gradientFactor = factor;
	}

	void stepGetPositions(const DrawStep &step, const Common::Rect &area, uint16 &in_x, uint16 &in_y, uint16 &in_w, uint16 &in_h) {
		if (!step.autoWidth) {
			in_w = step.w == -1 ? area.height() : step.w;
			
			switch(step.xAlign) {
				case Graphics::DrawStep::kVectorAlignManual:
					if (step.x >= 0) in_x = area.left + step.x;
					else in_x = area.left + area.width() + step.x; // value relative to the opposite corner.
					break;
					
				case Graphics::DrawStep::kVectorAlignCenter:
					in_x = area.left + (area.width() / 2) - (in_w / 2); 
					break;
					
				case Graphics::DrawStep::kVectorAlignLeft:
					in_x = area.left;
					break;
					
				case Graphics::DrawStep::kVectorAlignRight:
					in_x = area.left + area.width() - in_w;
					break;
					
				default:
					error("Vertical alignment in horizontal data.");
			}
		} else {
			in_x = area.left;
			in_w = area.width();
		}
		
		if (!step.autoHeight) {
			in_h = step.h == -1 ? area.width() : step.h;
			
			switch(step.yAlign) {
				case Graphics::DrawStep::kVectorAlignManual:
					if (step.y >= 0) in_y = area.top + step.y;
					else in_y = area.top + area.height() + step.y; // relative
					break;
					
				case Graphics::DrawStep::kVectorAlignCenter:
					in_y = area.top + (area.height() / 2) - (in_h / 2); 
					break;
					
				case Graphics::DrawStep::kVectorAlignTop:
					in_y = area.top;
					break;
					
				case Graphics::DrawStep::kVectorAlignBottom:
					in_y = area.top + area.height() - in_h;
					break;
					
				default:
					error("Horizontal alignment in vertical data.");
			}
		} else {
			in_y = area.top;
			in_h = area.height();
		}

		if (step.scale != (1 << 16) && step.scale != 0) {
			in_x = (in_x * step.scale) >> 16;
			in_y = (in_y * step.scale) >> 16;
			in_w = (in_w * step.scale) >> 16;
			in_h = (in_h * step.scale) >> 16;
		}
	}

	int stepGetRadius(const DrawStep &step, const Common::Rect &area) {
		int radius = 0;

		if (step.radius == 0xFF)
			radius = MIN(area.width(), area.height()) / 2;
		else
			radius = step.radius;

		if (step.scale != (1 << 16) && step.scale != 0)
			radius = (radius * step.scale) >> 16;

		return radius;
	}

	/**
	 * DrawStep callback functions for each drawing feature 
	 */
	void drawCallback_CIRCLE(const Common::Rect &area, const DrawStep &step) {
		uint16 x, y, w, h, radius;

		radius = stepGetRadius(step, area);
		stepGetPositions(step, area, x, y, w, h);

		drawCircle(x + radius, y + radius, radius);
	}

	void drawCallback_SQUARE(const Common::Rect &area, const DrawStep &step) {
		uint16 x, y, w, h;
		stepGetPositions(step, area, x, y, w, h);
		drawSquare(x, y, w, h);
	}

	void drawCallback_LINE(const Common::Rect &area, const DrawStep &step) {
		uint16 x, y, w, h;
		stepGetPositions(step, area, x, y, w, h);
		drawLine(x, y, x + w, y + w);
	}

	void drawCallback_ROUNDSQ(const Common::Rect &area, const DrawStep &step) {
		uint16 x, y, w, h;
		stepGetPositions(step, area, x, y, w, h);
		drawRoundedSquare(x, y, stepGetRadius(step, area), w, h);
	}

	void drawCallback_FILLSURFACE(const Common::Rect &area, const DrawStep &step) {
		fillSurface();
	}

	void drawCallback_TRIANGLE(const Common::Rect &area, const DrawStep &step) {
		uint16 x, y, w, h;
		stepGetPositions(step, area, x, y, w, h);
		drawTriangle(x, y, w, h, (TriangleOrientation)step.extraData);
	}

	void drawCallback_BEVELSQ(const Common::Rect &area, const DrawStep &step) {
		uint16 x, y, w, h;
		stepGetPositions(step, area, x, y, w, h);
		drawBeveledSquare(x, y, w, h, step.extraData);
	}

	void drawCallback_VOID(const Common::Rect &area, const DrawStep &step) {}

	/**
	 * Draws the specified draw step on the screen.
	 * 
	 * @see DrawStep
	 * @param area Zone to paint on
	 * @param step Pointer to a DrawStep struct.
	 */
	virtual void drawStep(const Common::Rect &area, const DrawStep &step);
	virtual void textStep(const Common::String &text, const Common::Rect &area, const TextStep &step);

	/**
	 * Copies the current surface to the system overlay 
	 *
	 * @param sys Pointer to the global System class
	 */
	virtual void copyFrame(OSystem *sys, const Common::Rect &r) = 0;
	virtual void copyWholeFrame(OSystem *sys) = 0;

	/**
	 * Blits a given graphics surface on top of the current drawing surface.
	 *
	 * @param source Surface to blit into the drawing surface.
	 * @param r Position in the active drawing surface to do the blitting.
	 */
	virtual void blitSurface(Graphics::Surface *source, const Common::Rect &r) = 0;
	
	virtual uint32 buildColor(uint8 r, uint8 g, uint8 b) = 0;
	
	virtual void drawString(const Graphics::Font *font, const Common::String &text, const Common::Rect &area, GUI::Theme::TextAlign alignH, GUI::Theme::TextAlignVertical alignV) = 0;

protected:
	Surface *_activeSurface; /** Pointer to the surface currently being drawn */

	FillMode _fillMode; /** Defines in which way (if any) are filled the drawn shapes */
	
	int _shadowOffset; /** offset for drawn shadows */
	int _strokeWidth; /** Width of the stroke of all drawn shapes */

	int _gradientFactor; /** Multiplication factor of the active gradient */
	int _gradientBytes[3]; /** Color bytes of the active gradient, used to speed up calculation */
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

	/**
	 * @see VectorRenderer::drawCircle()
	 */
	void drawCircle(int x, int y, int r);

	/**
	 * @see VectorRenderer::drawSquare()
	 */
	void drawSquare(int x, int y, int w, int h);

	/**
	 * @see VectorRenderer::drawRoundedSquare()
	 */
	void drawRoundedSquare(int x, int y, int r, int w, int h);

	/**
	 * @see VectorRenderer::drawTriangle()
	 */
	void drawTriangle(int x, int y, int base, int height, TriangleOrientation orient);

	void drawBeveledSquare(int x, int y, int w, int h, int bevel) {
		drawBevelSquareAlg(x, y, w, h, bevel, _fgColor, _bgColor);
	}
	
	void drawString(const Graphics::Font *font, const Common::String &text, 
					const Common::Rect &area, GUI::Theme::TextAlign alignH,
					GUI::Theme::TextAlignVertical alignV);

	/**
	 * @see VectorRenderer::setFgColor()
	 */
	void setFgColor(uint8 r, uint8 g, uint8 b) {
		this->_fgColor = RGBToColor<PixelFormat>(r, g, b);
	}

	/**
	 * @see VectorRenderer::setBgColor()
	 */
	void setBgColor(uint8 r, uint8 g, uint8 b) {
		this->_bgColor = RGBToColor<PixelFormat>(r, g, b);
	}

	/**
	 * @see VectorRenderer::setGradientColors()
	 */
	void setGradientColors(uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2) {
		_gradientEnd = RGBToColor<PixelFormat>(r2, g2, b2);
		_gradientStart = RGBToColor<PixelFormat>(r1, g1, b1);

		Base::_gradientBytes[0] = (_gradientEnd & PixelFormat::kRedMask) - (_gradientStart & PixelFormat::kRedMask);
		Base::_gradientBytes[1] = (_gradientEnd & PixelFormat::kGreenMask) - (_gradientStart & PixelFormat::kGreenMask);
		Base::_gradientBytes[2] = (_gradientEnd & PixelFormat::kBlueMask) - (_gradientStart & PixelFormat::kBlueMask);
	}

	/**
	 * @see VectorRenderer::fillSurface()
	 */
	void fillSurface() {
		PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(0, 0);

		int w = _activeSurface->w;
		int h = _activeSurface->h ;
		int pitch = surfacePitch();

		if (Base::_fillMode == kFillBackground)
			colorFill(ptr, ptr + w * h, _bgColor);
		else if (Base::_fillMode == kFillForeground)
			colorFill(ptr, ptr + w * h, _fgColor);
		else if (Base::_fillMode == kFillGradient) {
			int i = h;
			while (i--) {
				colorFill(ptr, ptr + w, calcGradient(h - i, h));
				ptr += pitch;
			}
		}
	}

	/**
	 * @see VectorRenderer::copyFrame()
	 */
	virtual void copyFrame(OSystem *sys, const Common::Rect &r) {
#ifdef OVERLAY_MULTIPLE_DEPTHS
		sys->copyRectToOverlay((const PixelType*)_activeSurface->getBasePtr(r.left, r.top),
			_activeSurface->w, r.left, r.top, r.width(), r.height());
#else
		sys->copyRectToOverlay((const OverlayColor*)_activeSurface->getBasePtr(r.left, r.top),
			_activeSurface->w, r.left, r.top, r.width(), r.height());
#endif
	}

	virtual void copyWholeFrame(OSystem *sys) {
#ifdef OVERLAY_MULTIPLE_DEPTHS
		sys->copyRectToOverlay((const PixelType*)_activeSurface->getBasePtr(0, 0),
			_activeSurface->w, 0, 0, _activeSurface->w, _activeSurface->h);
#else
		sys->copyRectToOverlay((const OverlayColor*)_activeSurface->getBasePtr(0, 0),
			_activeSurface->w, 0, 0, _activeSurface->w, _activeSurface->h);
#endif
	}

	/**
	 * @see VectorRenderer::blitSurface()
	 */
	virtual void blitSurface(Graphics::Surface *source, const Common::Rect &r) {
		PixelType *dst_ptr = (PixelType *)_activeSurface->getBasePtr(r.top, r.left);
		PixelType *src_ptr = (PixelType *)source->getBasePtr(0, 0);

		int dst_pitch = surfacePitch();
		int src_pitch = source->pitch / source->bytesPerPixel;

		int h = r.height(), w = r.width();

		while (h--) {
			colorCopy(src_ptr, dst_ptr, w);
			dst_ptr += dst_pitch;
			src_ptr += src_pitch;
		}
	}
	
	virtual uint32 buildColor(uint8 r, uint8 g, uint8 b) {
		return RGBToColor<PixelFormat>(r, g, b);
	}

protected:

	/**
	 * Draws a single pixel on the surface with the given coordinates and
	 * the given color.
	 *
	 * @param x Horizontal coordinate of the pixel.
	 * @param y Vertical coordinate of the pixel.
	 * @param color Color of the pixel
	 */
	virtual inline void putPixel(int x, int y, PixelType color) {
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
	virtual inline void blendPixel(int x, int y, PixelType color, uint8 alpha) {
		if (alpha == 255)
			putPixel(x, y, color);
		else if (alpha > 0)
			blendPixelPtr((PixelType*)Base::_activeSurface->getBasePtr(x, y), color, alpha);
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
	virtual inline void blendPixelPtr(PixelType *ptr, PixelType color, uint8 alpha)	{
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
	virtual void drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy, PixelType color);
	virtual void drawCircleAlg(int x, int y, int r, PixelType color, FillMode fill_m);
	virtual void drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, FillMode fill_m);
	virtual void drawSquareAlg(int x, int y, int w, int h, PixelType color, FillMode fill_m);
	virtual void drawTriangleVertAlg(int x, int y, int w, int h, bool inverted, PixelType color, FillMode fill_m);
	virtual void drawTriangleFast(int x, int y, int size, bool inverted, PixelType color, FillMode fill_m);
	virtual void drawBevelSquareAlg(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color);

	/**
	 * SHADOW DRAWING ALGORITHMS
	 *
	 * Optimized versions of the primitive drawing algorithms with alpha blending
	 * for shadow drawing.
	 * There functions may be overloaded in inheriting classes to improve performance
	 * in the slowest platforms where pixel alpha blending just doesn't cut it.
	 *
	 * @param blur Intensity/size of the shadow.
	 */
	virtual void drawSquareShadow(int x, int y, int w, int h, int blur);
	virtual void drawRoundedSquareShadow(int x, int y, int r, int w, int h, int blur);

	/**
	 * Calculates the color gradient on a given point.
	 * This function assumes that the gradient start/end colors have been set
	 * beforehand from the API function call.
	 *
	 * @param pos Progress of the gradient.
	 * @param max Maximum amount of the progress.
	 * @return Composite color of the gradient at the given "progress" amount.
	 */
	virtual inline PixelType calcGradient(uint32 pos, uint32 max) {
		PixelType output = 0;
		pos = (MIN(pos * Base::_gradientFactor, max) << 12) / max;
		
		output |= (_gradientStart + ((Base::_gradientBytes[0] * pos) >> 12)) & PixelFormat::kRedMask;
		output |= (_gradientStart + ((Base::_gradientBytes[1] * pos) >> 12)) & PixelFormat::kGreenMask;
		output |= (_gradientStart + ((Base::_gradientBytes[2] * pos) >> 12)) & PixelFormat::kBlueMask;
		output |= ~(PixelFormat::kRedMask | PixelFormat::kGreenMask | PixelFormat::kBlueMask);
	
		return output;
	}

	/**
	 * Fills several pixels in a row with a given color and the specifed alpha blending.
	 *
	 * @see blendPixelPtr
	 * @see blendPixel
	 * @param first Pointer to the first pixel to fill.
	 * @param last Pointer to the last pixel to fill.
	 * @param color Color of the pixel
	 * @param alpha Alpha intensity of the pixel (0-255)
	 */
	virtual inline void blendFill(PixelType *first, PixelType *last, PixelType color, uint8 alpha) {
		while (first != last)
			blendPixelPtr(first++, color, alpha);
	}

	/**
	 * Fills several pixels in a row with a given color.
	 *
	 * This is a replacement function for Common::set_to, using an unrolled
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
	virtual inline void colorFill(PixelType *first, PixelType *last, PixelType color) {
		if (first == last) {
			*first = color;
			return;
		}

		register PixelType *ptr = first;
		register int count = (last - first);
		register int n = (count + 7) >> 3;
		switch (count % 8) {
		case 0: do { 
					*ptr++ = color;
		case 7:		*ptr++ = color;
		case 6:		*ptr++ = color;
		case 5:		*ptr++ = color;
		case 4:		*ptr++ = color;
		case 3:		*ptr++ = color;
		case 2:		*ptr++ = color;
		case 1:		*ptr++ = color;
				} while (--n > 0);
		}
	}

	/**
	 * Copies several pixes in a row from a surface to another one.
	 * Used for surface blitting.
	 * See colorFill() for optimization guidelines.
	 *
	 * @param src Source surface.
	 * @param dst Destination surface.
	 * @param count Amount of pixels to copy over.
	 */
	virtual inline void colorCopy(PixelType *src, PixelType *dst, int count) {
		register int n = (count + 7) >> 3;
		switch (count % 8) {
		case 0: do { 
					*dst++ = *src++;
		case 7:		*dst++ = *src++;
		case 6:		*dst++ = *src++;
		case 5:		*dst++ = *src++;
		case 4:		*dst++ = *src++;
		case 3:		*dst++ = *src++;
		case 2:		*dst++ = *src++;
		case 1:		*dst++ = *src++;
				} while (--n > 0);
		}
	}

	PixelType _fgColor; /** Foreground color currently being used to draw on the renderer */
	PixelType _bgColor; /** Background color currently being used to draw on the renderer */

	PixelType _gradientStart; /** Start color for the fill gradient */
	PixelType _gradientEnd; /** End color for the fill gradient */
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
	virtual void drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy, PixelType color);

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
};

} // end of namespace Graphics

#endif
