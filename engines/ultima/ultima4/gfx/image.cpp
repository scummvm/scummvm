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

#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/misc.h"
#include "common/system.h"
#include "common/list.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

namespace Ultima {
namespace Ultima4 {

Image::Image() : _surface(nullptr), _disposeAfterUse(DisposeAfterUse::NO),
		_paletted(false) {
}

Image *Image::create(int w, int h, bool paletted, Image::Type type) {
	Image *im = new Image();
	im->create(w, h, paletted);

	return im;
}

void Image::create(int w, int h, bool paletted) {
	_paletted = paletted;
	_surface = new Graphics::ManagedSurface(w, h, paletted ?
		Graphics::PixelFormat::createFormatCLUT8() :
		Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
	_disposeAfterUse = DisposeAfterUse::YES;
}

void Image::blitFrom(const Graphics::Surface &src) {
	_surface->blitFrom(src);
}

Image *Image::createScreenImage() {
	Image *screen = new Image();
	screen->_surface = g_screen;
	screen->_disposeAfterUse = DisposeAfterUse::NO;
	screen->_paletted = false;

	return screen;
}

Image *Image::duplicate(Image *image) {
	bool alphaOn = image->isAlphaOn();
	Image *im = create(image->width(), image->height(), false, HARDWARE);

//    if (image->isIndexed())
//        im->setPaletteFromImage(image);

	/* Turn alpha off before blitting to non-screen surfaces */
	if (alphaOn)
		image->alphaOff();

	image->drawOn(im, 0, 0);

	if (alphaOn)
		image->alphaOn();

	im->_backgroundColor = image->_backgroundColor;

	return im;
}

Image::~Image() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _surface;
}

void Image::setPalette(const RGBA *colors, unsigned n_colors) {
	assertMsg(_paletted, "imageSetPalette called on non-paletted image");

	byte *pal = new byte[n_colors * 3];
	byte *palP = pal;
	for (unsigned i = 0; i < n_colors; i++, palP += 3) {
		palP[0] = colors[i].r;
		palP[1] = colors[i].g;
		palP[2] = colors[i].b;
	}

	_surface->setPalette(pal, 0, n_colors);
	delete[] pal;
}

void Image::setPaletteFromImage(const Image *src) {
	assertMsg(_paletted && src->_paletted, "imageSetPaletteFromImage called on non-indexed image");

	const uint32 *srcPal = src->_surface->getPalette();
	_surface->setPalette(srcPal, 0, PALETTE_COUNT);
}

RGBA Image::getPaletteColor(int index) {
	RGBA color = RGBA(0, 0, 0, 0);

	if (_paletted) {
		uint32 pal = _surface->getPalette()[index];
		color.r = (pal & 0xff);
		color.g = (pal >> 8) & 0xff;
		color.b = (pal >> 16) & 0xff;
		color.a = IM_OPAQUE;
	}

	return color;
}

int Image::getPaletteIndex(RGBA color) {
	if (!_paletted)
		return -1;

	const uint32 *pal = _surface->getPalette();
	uint32 color32 = color;

	for (int i = 0; i < PALETTE_COUNT; ++i, ++pal) {
		if (*pal == color32)
			return i;
	}

	// return the proper palette index for the specified color
	return -1;
}

RGBA Image::setColor(uint8 r, uint8 g, uint8 b, uint8 a) {
	RGBA color = RGBA(r, g, b, a);
	return color;
}

bool Image::setFontColor(ColorFG fg, ColorBG bg) {
	if (!setFontColorFG(fg)) return false;
	if (!setFontColorBG(bg)) return false;
	return true;
}

bool Image::setFontColorFG(ColorFG fg) {
	switch (fg) {
	case FG_GREY:
		if (!setPaletteIndex(TEXT_FG_PRIMARY_INDEX,   setColor(153, 153, 153))) return false;
		if (!setPaletteIndex(TEXT_FG_SECONDARY_INDEX, setColor(102, 102, 102))) return false;
		if (!setPaletteIndex(TEXT_FG_SHADOW_INDEX,    setColor(51, 51, 51))) return false;
		break;
	case FG_BLUE:
		if (!setPaletteIndex(TEXT_FG_PRIMARY_INDEX,   setColor(102, 102, 255))) return false;
		if (!setPaletteIndex(TEXT_FG_SECONDARY_INDEX, setColor(51, 51, 204))) return false;
		if (!setPaletteIndex(TEXT_FG_SHADOW_INDEX,    setColor(51, 51, 51))) return false;
		break;
	case FG_PURPLE:
		if (!setPaletteIndex(TEXT_FG_PRIMARY_INDEX,   setColor(255, 102, 255))) return false;
		if (!setPaletteIndex(TEXT_FG_SECONDARY_INDEX, setColor(204, 51, 204))) return false;
		if (!setPaletteIndex(TEXT_FG_SHADOW_INDEX,    setColor(51, 51, 51))) return false;
		break;
	case FG_GREEN:
		if (!setPaletteIndex(TEXT_FG_PRIMARY_INDEX,   setColor(102, 255, 102))) return false;
		if (!setPaletteIndex(TEXT_FG_SECONDARY_INDEX, setColor(0, 153, 0))) return false;
		if (!setPaletteIndex(TEXT_FG_SHADOW_INDEX,    setColor(51, 51, 51))) return false;
		break;
	case FG_RED:
		if (!setPaletteIndex(TEXT_FG_PRIMARY_INDEX,   setColor(255, 102, 102))) return false;
		if (!setPaletteIndex(TEXT_FG_SECONDARY_INDEX, setColor(204, 51, 51))) return false;
		if (!setPaletteIndex(TEXT_FG_SHADOW_INDEX,    setColor(51, 51, 51))) return false;
		break;
	case FG_YELLOW:
		if (!setPaletteIndex(TEXT_FG_PRIMARY_INDEX,   setColor(255, 255, 51))) return false;
		if (!setPaletteIndex(TEXT_FG_SECONDARY_INDEX, setColor(204, 153, 51))) return false;
		if (!setPaletteIndex(TEXT_FG_SHADOW_INDEX,    setColor(51, 51, 51))) return false;
		break;
	default:
		if (!setPaletteIndex(TEXT_FG_PRIMARY_INDEX,   setColor(255, 255, 255))) return false;
		if (!setPaletteIndex(TEXT_FG_SECONDARY_INDEX, setColor(204, 204, 204))) return false;
		if (!setPaletteIndex(TEXT_FG_SHADOW_INDEX,    setColor(68, 68, 68))) return false;
	}
	return true;
}

bool Image::setFontColorBG(ColorBG bg) {
	switch (bg) {
	case BG_BRIGHT:
		if (!setPaletteIndex(TEXT_BG_INDEX, setColor(0, 0, 102)))
			return false;
		break;
	default:
		if (!setPaletteIndex(TEXT_BG_INDEX, setColor(0, 0, 0)))
			return false;
	}
	return true;
}

bool Image::setPaletteIndex(uint index, RGBA color) {
	if (!_paletted)
		return false;

	uint32 color32 = color;
	_surface->setPalette(&color32, index, 1);

	// success
	return true;
}

bool Image::getTransparentIndex(uint &index) const {
	if (!_paletted || !_surface->hasTransparentColor())
		return false;

	index = _surface->getTransparentColor();
	return true;
}

void Image::initializeToBackgroundColor(RGBA backgroundColor) {
	if (_paletted)
		error("initializeToBackgroundColor: Not supported");

	_backgroundColor = backgroundColor;
	fillRect(0, 0, _surface->w, _surface->h, backgroundColor.r,
		backgroundColor.g, backgroundColor.b, backgroundColor.a);
}

bool Image::isAlphaOn() const {
	return !_paletted;
}

void Image::alphaOn() {
}

void Image::alphaOff() {
}

void Image::putPixel(int x, int y, int r, int g, int b, int a) {
	uint32 color = getColor(r, g, b, a);
	putPixelIndex(x, y, color);
}

uint Image::getColor(byte r, byte g, byte b, byte a) {
	uint color;

	if (_surface->format.bytesPerPixel == 1) {
		const uint32 *pal = _surface->getPalette();
		for (color = 0; color <= 0xfe; ++color, ++pal) {
			byte rv = *pal & 0xff;
			byte gv = (*pal >> 8) & 0xff;
			byte bv = (*pal >> 16) & 0xff;
			if (r == rv && g == gv && b == bv)
				break;
		}

		return color;
	} else {
		return _surface->format.ARGBToColor(a, r, g, b);
	}
}

void Image::makeBackgroundColorTransparent(int haloSize, int shadowOpacity) {
	uint32 bgColor = _surface->format.ARGBToColor(
	                     static_cast<byte>(_backgroundColor.a),
	                     static_cast<byte>(_backgroundColor.r),
	                     static_cast<byte>(_backgroundColor.g),
	                     static_cast<byte>(_backgroundColor.b)
	                 );

	performTransparencyHack(bgColor, 1, 0, haloSize, shadowOpacity);
}

void Image::performTransparencyHack(uint colorValue, uint numFrames,
                                    uint currentFrameIndex, uint haloWidth,
                                    uint haloOpacityIncrementByPixelDistance) {
	Common::List<Std::pair<uint, uint> > opaqueXYs;
	uint x, y;
	byte t_r, t_g, t_b;

	_surface->format.colorToRGB(colorValue, t_r, t_g, t_b);

	uint frameHeight = _surface->h / numFrames;
	//Min'd so that they never go out of range (>=h)
	uint top = MIN(_surface->h, (uint16)(currentFrameIndex * frameHeight));
	uint bottom = MIN(_surface->h, (uint16)(top + frameHeight));

	for (y = top; y < bottom; y++) {

		for (x = 0; x < _surface->w; x++) {
			uint r, g, b, a;
			getPixel(x, y, r, g, b, a);
			if (r == t_r &&
			        g == t_g &&
			        b == t_b) {
				putPixel(x, y, r, g, b, IM_TRANSPARENT);
			} else {
				putPixel(x, y, r, g, b, a);
				if (haloWidth)
					opaqueXYs.push_back(Std::pair<uint, uint>(x, y));
			}
		}
	}
	int ox, oy;
	for (Common::List<Std::pair<uint, uint> >::iterator xy = opaqueXYs.begin();
	        xy != opaqueXYs.end();
	        ++xy) {
		ox = xy->first;
		oy = xy->second;
		int span = int(haloWidth);
		uint x_start = MAX(0, ox - span);
		uint x_finish = MIN(int(_surface->w), ox + span + 1);
		for (x = x_start; x < x_finish; ++x) {
			uint y_start = MAX(int(top), oy - span);
			uint y_finish = MIN(int(bottom), oy + span + 1);
			for (y = y_start; y < y_finish; ++y) {

				int divisor = 1 + span * 2 - abs(int(ox - x)) - abs(int(oy - y));

				uint r, g, b, a;
				getPixel(x, y, r, g, b, a);
				if (a != IM_OPAQUE) {
					putPixel(x, y, r, g, b, MIN(IM_OPAQUE, a + haloOpacityIncrementByPixelDistance / divisor));
				}
			}
		}
	}


}

void Image::setTransparentIndex(uint index) {
	if (_paletted)
		_surface->setTransparentColor(index);
}

void Image::putPixelIndex(int x, int y, uint index) {
	int bpp;
	byte *p;

	bpp = _surface->format.bytesPerPixel;
	p = (byte *)_surface->getBasePtr(x, y);

	switch (bpp) {
	case 1:
		*p = index;
		break;

	case 2:
		*((uint16 *)p) = index;
		break;

	case 4:
		*reinterpret_cast<uint32 *>(p) = index;
		break;

	default:
		error("Unsupported format");
	}
}

void Image::fillRect(int x, int y, int w, int h, int r, int g, int b, int a) {
	uint32 color = getColor(r, g, b, a);
	_surface->fillRect(Common::Rect(x, y, x + w, y + h), color);
}

void Image::getPixel(int x, int y, uint &r, uint &g, uint &b, uint &a) const {
	uint index;
	byte r1, g1, b1, a1;

	getPixelIndex(x, y, index);

	if (_surface->format.bytesPerPixel == 1) {
		uint32 col = _surface->getPalette()[index];
		r = col & 0xff;
		g = (col >> 8) & 0xff;
		b = (col >> 16) & 0xff;
		a = (col >> 24) & 0xff;
	} else {
		_surface->format.colorToARGB(index, a1, r1, g1, b1);
		r = r1;
		g = g1;
		b = b1;
		a = a1;
	}
}

void Image::getPixelIndex(int x, int y, uint &index) const {
	int bpp = _surface->format.bytesPerPixel;

	byte *p = (byte *)_surface->getBasePtr(x, y);

	switch (bpp) {
	case 1:
		index = *p;
		break;

	case 2:
		index = *reinterpret_cast<uint16 *>(p);
		break;

	case 4:
		index = *reinterpret_cast<uint32 *>(p);
		break;

	default:
		error("Unsupported format");
	}
}

Graphics::ManagedSurface *Image::getSurface(Image *d) const {
	if (d)
		return d->_surface;

	return g_screen;
}

void Image::drawOn(Image *d, int x, int y) const {
	Graphics::ManagedSurface *destSurface = getSurface(d);
	destSurface->blitFrom(*_surface, Common::Point(x, y));
}

void Image::drawSubRectOn(Image *d, int x, int y, int rx, int ry, int rw, int rh) const {
	Graphics::ManagedSurface *destSurface = getSurface(d);

	Common::Rect srcRect(rx, ry, MIN(rx + rw, (int)_surface->w), MIN(ry + rh, (int)_surface->h));
	Common::Point destPos(x, y);

	// Handle when the source rect is off the surface
	if (srcRect.left < 0) {
		destPos.x += -srcRect.left;
		srcRect.left = 0;
	}

	if (srcRect.top < 0) {
		destPos.y += -srcRect.top;
		srcRect.top = 0;
	}

	if (srcRect.isValidRect())
		// Blit the surface
		destSurface->blitFrom(*_surface, srcRect, destPos);
}

void Image::drawSubRectInvertedOn(Image *d, int x, int y, int rx, int ry, int rw, int rh) const {
	Graphics::ManagedSurface *destSurface = getSurface(d);
	int i;
	Common::Rect src;
	Common::Point destPos;

	for (i = 0; i < rh; i++) {
		src.left = rx;
		src.top = ry + i;
		src.right = rx + rw;
		src.bottom = ry + i + 1;

		destPos.x = x;
		destPos.y = y + rh - i - 1;

		destSurface->blitFrom(*_surface, src, destPos);
	}
}

void Image::dump() {
	g_screen->blitFrom(*_surface, Common::Point(0, 0));
	g_screen->update();
}

void Image::drawHighlighted() {
	RGBA c;
	for (unsigned i = 0; i < _surface->h; i++) {
		for (unsigned j = 0; j < _surface->w; j++) {
			getPixel(j, i, c.r, c.g, c.b, c.a);
			putPixel(j, i, 0xff - c.r, 0xff - c.g, 0xff - c.b, c.a);
		}
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
