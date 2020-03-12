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

#ifndef ULTIMA4_IMAGE_H
#define ULTIMA4_IMAGE_H

#include "ultima/ultima4/types.h"
#include "ultima/ultima4/u4file.h"
#include "ultima/ultima4/textcolor.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima4 {

typedef Graphics::ManagedSurface *BackendSurface;

#define DARK_GRAY_HALO RGBA(14,15,16,255)

struct RGBA {
	RGBA(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {}
	RGBA() : r(0), g(0), b(0), a(255){}
    unsigned int r, g, b, a;

	operator uint32() const {
		return r | (g << 8) | (b << 16) | (0xff << 24);
	}
};
bool operator==(const RGBA &lhs, const RGBA &rhs);

class Image;

struct SubImage {
    Common::String _name;
    Common::String _srcImageName;
    int x, y, width, height;
};

#define IM_OPAQUE (unsigned int) 255
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
public:
    enum Type {
        HARDWARE,
        SOFTWARE
    };

    static Image *create(int w, int h, bool indexed, Type type);
    static Image *createScreenImage();
    static Image *duplicate(Image *image);
    ~Image();

    /* palette handling */
    void setPalette(const RGBA *colors, unsigned n_colors);
    void setPaletteFromImage(const Image *src);
    bool getTransparentIndex(unsigned int &index) const;
    void performTransparencyHack(unsigned int colorValue, unsigned int numFrames, unsigned int currentFrameIndex, unsigned int haloWidth, unsigned int haloOpacityIncrementByPixelDistance);
    void setTransparentIndex(unsigned int index);
//    void invokeTransparencyHack(ImageInfo * info);

    bool setFontColor(ColorFG fg, ColorBG bg);
    bool setFontColorFG(ColorFG fg);
    bool setFontColorBG(ColorBG bg);

    RGBA getPaletteColor(int index);       // returns the color of the specified palette index
    bool setPaletteIndex(unsigned int index, RGBA color);  // sets the specified palette index to the specified RGB color
    int getPaletteIndex(RGBA color);              // returns the palette index of the specified RGB color
    RGBA setColor(uint8 r, uint8 g, uint8 b, uint8 a = IM_OPAQUE);


    /* alpha handling */
    bool isAlphaOn() const;
    void alphaOn();
    void alphaOff();


    /* Will clear the image to the background color, and set the internal backgroundColor variable */
    void initializeToBackgroundColor(RGBA backgroundColor = DARK_GRAY_HALO);
    /* Will make the pixels that match the background color disappear, with a blur halo */
    void makeBackgroundColorTransparent(int haloSize = 0,  int shadowOpacity = 255);


    //void finalizeAlphaSurface(RGBA * key = NULL);

    /* writing to image */

    /**
     * Sets the color of a single pixel.
     */
    void putPixel(int x, int y, int r, int g, int b, int a); //TODO Consider using &


    void putPixelIndex(int x, int y, unsigned int index);


    void fillRect(int x, int y, int w, int h, int r, int g, int b, int a=IM_OPAQUE);

    /* reading from image */
    void getPixel(int x, int y, unsigned int &r, unsigned int &g, unsigned int &b, unsigned int &a) const;
    void getPixelIndex(int x, int y, unsigned int &index) const;

    /* image drawing methods */
    /**
     * Draws the entire image onto the screen at the given offset.
     */
    void draw(int x, int y) const {
        drawOn(NULL, x, y);
    }

    /**
     * Draws a piece of the image onto the screen at the given offset.
     * The area of the image to draw is defined by the rectangle rx, ry,
     * rw, rh.
     */
    void drawSubRect(int x, int y, int rx, int ry, int rw, int rh) const {
        drawSubRectOn(NULL, x, y, rx, ry, rw, rh);
    }

    /**
     * Draws a piece of the image onto the screen at the given offset, inverted.
     * The area of the image to draw is defined by the rectangle rx, ry,
     * rw, rh.
     */
    void drawSubRectInverted(int x, int y, int rx, int ry, int rw, int rh) const {
        drawSubRectInvertedOn(NULL, x, y, rx, ry, rw, rh);
    }

    /* image drawing methods for drawing onto another image instead of the screen */
    void drawOn(Image *d, int x, int y) const;
    void drawSubRectOn(Image *d, int x, int y, int rx, int ry, int rw, int rh) const;
    void drawSubRectInvertedOn(Image *d, int x, int y, int rx, int ry, int rw, int rh) const;

    int width() const { return w; }
    int height() const { return h; }
    bool isIndexed() const { return _indexed; }
    BackendSurface getSurface() { return _surface; }
    void save(const Common::String &filename);
    void drawHighlighted();


private:
    unsigned int w, h;
    bool _indexed;
    RGBA _backgroundColor;
    Image();                    /* use create method to construct images */

    // disallow assignments, copy contruction
    Image(const Image&);
    const Image &operator=(const Image&);

    BackendSurface _surface;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
