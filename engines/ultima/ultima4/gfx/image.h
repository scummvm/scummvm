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

#ifndef ULTIMA4_GFX_IMAGE_H
#define ULTIMA4_GFX_IMAGE_H

#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/gfx/textcolor.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima4 {

typedef Graphics::ManagedSurface *BackendSurface;

#define DARK_GRAY_HALO RGBA(14,15,16,255)

struct RGBA {
	RGBA(int red, int green, int blue, int alpha) : r(red), g(green), b(blue), a(alpha) {}
	RGBA() : r(0), g(0), b(0), a(255) {}
	uint r, g, b, a;

	operator uint32() const {
		return r | (g << 8) | (b << 16) | (0xff << 24);
	}
};
bool operator==(const RGBA &lhs, const RGBA &rhs);

class Image;

struct SubImage : public Common::Rect {
	Common::String _name;
	Common::String _srcImageName;
};

#define IM_OPAQUE (uint) 255
#define IM_TRANSPARENT 0

/**
 * A simple image object that can be drawn and read/written to at the
 * pixel level.
 * @todo
 *  <ul>
 *      <li>drawing methods should be pushed to Drawable subclass</li>
 *  </ul>
 */
class Image {
private:
	Graphics::ManagedSurface *_surface;
	DisposeAfterUse::Flag _disposeAfterUse;
	bool _paletted;
	RGBA _backgroundColor;
	Image();                    /* use create method to construct images */

	// disallow assignments, copy contruction
	Image(const Image &);
	const Image &operator=(const Image &);

	Graphics::ManagedSurface *getSurface(Image *d) const;

	uint getColor(byte r, byte g, byte b, byte a);
public:
	enum Type {
		HARDWARE,
		SOFTWARE
	};

	/**
	 * Creates a new image.  Scale is stored to allow drawing using U4
	 * (320x200) coordinates, regardless of the actual image scale.
	 * Indexed is true for palette based images, or false for RGB images.
	 * Image type determines whether to create a hardware (i.e. video ram)
	 * or software (i.e. normal ram) image.
	 */
	static Image *create(int w, int h, bool paletted, Type type);

	/**
	 * Create a special purpose image the represents the whole screen.
	 */
	static Image *createScreenImage();

	/**
	 * Creates a duplicate of another image
	 */
	static Image *duplicate(Image *image);

	/**
	 * Frees the image.
	 */
	~Image();

	void create(int w, int h, bool paletted);

	/* palette handling */
	/**
	 * Sets the palette
	 */
	void setPalette(const RGBA *colors, unsigned n_colors);

	/**
	 * Copies the palette from another image.
	 */
	void setPaletteFromImage(const Image *src);
	bool getTransparentIndex(uint &index) const;
	void performTransparencyHack(uint colorValue, uint numFrames, uint currentFrameIndex, uint haloWidth, uint haloOpacityIncrementByPixelDistance);
	void setTransparentIndex(uint index);

	/**
	 * Sets the specified font colors
	 */
	bool setFontColor(ColorFG fg, ColorBG bg);

	/**
	 * Sets the specified font colors
	 */
	bool setFontColorFG(ColorFG fg);

	/**
	 * Sets the specified font colors
	 */
	bool setFontColorBG(ColorBG bg);

	/**
	 * Returns the color of the specified palette index
	 */
	RGBA getPaletteColor(int index);       // returns the color of the specified palette index

	/**
	 * Sets the specified palette index to the specified RGB color
	 */
	bool setPaletteIndex(uint index, RGBA color);

	/**
	 * Returns the palette index of the specified RGB color
	 */
	int getPaletteIndex(RGBA color);
	RGBA setColor(uint8 r, uint8 g, uint8 b, uint8 a = IM_OPAQUE);


	/* alpha handling */
	bool isAlphaOn() const;
	void alphaOn();
	void alphaOff();


	/* Will clear the image to the background color, and set the internal backgroundColor variable */
	void initializeToBackgroundColor(RGBA backgroundColor = DARK_GRAY_HALO);
	/* Will make the pixels that match the background color disappear, with a blur halo */
	void makeBackgroundColorTransparent(int haloSize = 0,  int shadowOpacity = 255);


	//void finalizeAlphaSurface(RGBA * key = nullptr);

	/* writing to image */

	/**
	 * Sets the color of a single pixel.
	 */
	void putPixel(int x, int y, int r, int g, int b, int a); //TODO Consider using &

	/**
	 * Sets the palette index of a single pixel.  If the image is in
	 * indexed mode, then the index is simply the palette entry number.
	 * If the image is RGB, it is a packed RGB triplet.
	 */
	void putPixelIndex(int x, int y, uint index);

	/**
	 * Fills a rectangle in the image with a given color.
	 */
	void fillRect(int x, int y, int w, int h, int r, int g, int b, int a = IM_OPAQUE);

	void blitFrom(const Graphics::Surface &src);

	/* reading from image */
	/**
	 * Gets the color of a single pixel.
	 */
	void getPixel(int x, int y, uint &r, uint &g, uint &b, uint &a) const;

	/**
	 * Gets the palette index of a single pixel.  If the image is in
	 * indexed mode, then the index is simply the palette entry number.
	 * If the image is RGB, it is a packed RGB triplet.
	 */
	void getPixelIndex(int x, int y, uint &index) const;

	/* image drawing methods */
	/**
	 * Draws the entire image onto the screen at the given offset.
	 */
	void draw(int x, int y) const {
		drawOn(nullptr, x, y);
	}

	/**
	 * Draws a piece of the image onto the screen at the given offset.
	 * The area of the image to draw is defined by the rectangle rx, ry,
	 * rw, rh.
	 */
	void drawSubRect(int x, int y, int rx, int ry, int rw, int rh) const {
		drawSubRectOn(nullptr, x, y, rx, ry, rw, rh);
	}

	/**
	 * Draws a piece of the image onto the screen at the given offset, inverted.
	 * The area of the image to draw is defined by the rectangle rx, ry,
	 * rw, rh.
	 */
	void drawSubRectInverted(int x, int y, int rx, int ry, int rw, int rh) const {
		drawSubRectInvertedOn(nullptr, x, y, rx, ry, rw, rh);
	}

	/* image drawing methods for drawing onto another image instead of the screen */
	/**
	 * Draws the image onto another image.
	 */
	void drawOn(Image *d, int x, int y) const;

	/**
	 * Draws a piece of the image onto another image.
	 */
	void drawSubRectOn(Image *d, int x, int y, int rx, int ry, int rw, int rh) const;

	/**
	 * Draws a piece of the image onto another image, inverted.
	 */
	void drawSubRectInvertedOn(Image *d, int x, int y, int rx, int ry, int rw, int rh) const;

	int width() const {
		return _surface->w;
	}
	int height() const {
		return _surface->h;
	}
	bool isIndexed() const {
		return _paletted;
	}
	BackendSurface getSurface() {
		return _surface;
	}

	void drawHighlighted();

	/**
	 * Debug method that dumps a given image directly to the screen
	 * and does a screen update. Useful for validating images are correct
	 */
	void dump();
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
