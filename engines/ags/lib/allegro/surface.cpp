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

#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/ags.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

namespace AGS3 {

BITMAP::BITMAP(Graphics::ManagedSurface *owner) : _owner(owner),
		w(owner->w), h(owner->h), pitch(owner->pitch), format(owner->format),
		clip(false), ct(0), cl(0), cr(owner->w), cb(owner->h) {
	line.resize(h);
	for (uint y = 0; y < h; ++y)
		line[y] = (byte *)_owner->getBasePtr(0, y);
}

int BITMAP::getpixel(int x, int y) const {
	if (x < 0 || y < 0 || x >= w || y >= h)
		return -1;

	const byte *pixel = (const byte *)getBasePtr(x, y);
	if (format.bytesPerPixel == 1)
		return *pixel;
	else if (format.bytesPerPixel == 2)
		return *(const uint16 *)pixel;
	else
		return *(const uint32 *)pixel;
}

void BITMAP::circlefill(int x, int y, int radius, int color) {
	int cx = 0;
	int cy = radius;
	int df = 1 - radius;
	int d_e = 3;
	int d_se = -2 * radius + 5;

	do {
		_owner->hLine(x - cy, y - cx, x + cy, color);

		if (cx)
			_owner->hLine(x - cy, y + cx, x + cy, color);

		if (df < 0) {
			df += d_e;
			d_e += 2;
			d_se += 2;
		} else {
			if (cx != cy) {
				_owner->hLine(x - cx, y - cy, x + cx, color);

				if (cy)
					_owner->hLine(x - cx, y + cy, x + cx, color);
			}

			df += d_se;
			d_e += 2;
			d_se += 4;
			cy--;
		}

		cx++;

	} while (cx <= cy);
}

void BITMAP::floodfill(int x, int y, int color) {
	AGS3::floodfill(this, x, y, color);
}

const int SCALE_THRESHOLD = 0x100;
#define IS_TRANSPARENT(R, G, B) ((R) == 255 && (G) == 0 && (B) == 255)

void BITMAP::draw(const BITMAP *srcBitmap, const Common::Rect &srcRect,
		const Common::Rect &destRect, bool horizFlip, bool vertFlip,
		bool skipTrans, int srcAlpha) {
	assert(format.bytesPerPixel == 2 || format.bytesPerPixel == 4 ||
		(format.bytesPerPixel == 1 && srcBitmap->format.bytesPerPixel == 1));

	const Graphics::ManagedSurface &src = **srcBitmap;
	Graphics::ManagedSurface &dest = *_owner;
	Graphics::Surface destArea = dest.getSubArea(destRect);
	const int scaleX = SCALE_THRESHOLD * srcRect.width() / destRect.width();
	const int scaleY = SCALE_THRESHOLD * srcRect.height() / destRect.height();
	const int xDir = horizFlip ? -1 : 1;

	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest, gDest, bDest, aDest;
	uint32 pal[PALETTE_COUNT];

	if (src.format.bytesPerPixel == 1) {
		for (int i = 0; i < PALETTE_COUNT; ++i)
			pal[i] = format.RGBToColor(_current_palette[i].r,
				_current_palette[i].g, _current_palette[i].b);
		pal[0] = format.RGBToColor(0xff, 0, 0xff);
	}

	for (int destY = destRect.top, yCtr = 0, scaleYCtr = 0; yCtr < destArea.h;
			++destY, ++yCtr, scaleYCtr += scaleY) {
		if (destY < 0 || destY >= h)
			continue;
		byte *destP = (byte *)destArea.getBasePtr(0, yCtr);
		const byte *srcP = (const byte *)src.getBasePtr(
			horizFlip ? srcRect.right - 1 : srcRect.left,
			vertFlip ? srcRect.bottom - 1 - scaleYCtr / SCALE_THRESHOLD :
			srcRect.top + scaleYCtr / SCALE_THRESHOLD);

		// Loop through the pixels of the row
		for (int destX = destRect.left, xCtr = 0, scaleXCtr = 0; xCtr < destArea.w;
				++destX, ++xCtr, scaleXCtr += scaleX) {
			if (destX < 0 || destX >= w)
				continue;

			const byte *srcVal = srcP + xDir * (scaleXCtr / SCALE_THRESHOLD * src.format.bytesPerPixel);
			byte *destVal = (byte *)&destP[xCtr * format.bytesPerPixel];

			switch (src.format.bytesPerPixel) {
			case 1:
				if (format.bytesPerPixel == 1) {
					*destVal = *srcVal;
					continue;
				}
				format.colorToARGB(pal[*srcVal], aSrc, rSrc, gSrc, bSrc);
				break;
			case 2:
				src.format.colorToARGB(*(uint16 *)srcVal, aSrc, rSrc, gSrc, bSrc);
				break;
			case 4:
				src.format.colorToARGB(*(uint32 *)srcVal, aSrc, rSrc, gSrc, bSrc);
				break;
			default:
				error("Unknown format");
			}

			if (srcAlpha != -1)
				aSrc = srcAlpha;
			if (aSrc == 0)
				aSrc = 0xff;

			if (aSrc != 0xff) {
				// Get the pixel at the destination, to check if it's transparent.
				// Transparent pixels can be considered to be 0 alph
				format.colorToARGB(format.bytesPerPixel == 2 ?
					*(uint16 *)destVal : *(uint32 *)destVal, aDest, rDest, gDest, bDest);
				if (IS_TRANSPARENT(rDest, gDest, bDest))
					aDest = 0;
			} else {
				// Source is opaque, so just treat destination as transparent
				aDest = 0;
			}

			if (aDest != 0 && (aSrc != 0xff || srcAlpha != -1)) {
				// Alpha blender
				double sAlpha = (double)aSrc / 255.0;
				double dAlpha = (double)aDest / 255.0;
				dAlpha *= (1.0 - sAlpha);
				rDest = static_cast<uint8>((rSrc * sAlpha + rDest * dAlpha) / (sAlpha + dAlpha));
				gDest = static_cast<uint8>((gSrc * sAlpha + gDest * dAlpha) / (sAlpha + dAlpha));
				bDest = static_cast<uint8>((bSrc * sAlpha + bDest * dAlpha) / (sAlpha + dAlpha));
				aDest = static_cast<uint8>(255. * (sAlpha + dAlpha));
			} else {
				rDest = rSrc;
				gDest = gSrc;
				bDest = bSrc;
				aDest = aSrc;
			}

			if (!(IS_TRANSPARENT(rSrc, gSrc, bSrc) && skipTrans)) {
				uint32 pixel = format.ARGBToColor(aDest, rDest, gDest, bDest);
				if (format.bytesPerPixel == 4)
					*(uint32 *)destVal = pixel;
				else
					*(uint16 *)destVal = pixel;
			}
		}
	}
}

/*-------------------------------------------------------------------*/

/**
 * Dervied screen surface
 */
class Screen : public Graphics::Screen, public BITMAP {
public:
	Screen() : Graphics::Screen(), BITMAP(this) {}
	Screen(int width, int height) : Graphics::Screen(width, height), BITMAP(this) {}
	Screen(int width, int height, const Graphics::PixelFormat &pixelFormat) :
		Graphics::Screen(width, height, pixelFormat), BITMAP(this) {}
	~Screen() override {}
};

/*-------------------------------------------------------------------*/

BITMAP *create_bitmap(int width, int height) {
	return new Surface(width, height);
}

BITMAP *create_bitmap_ex(int color_depth, int width, int height) {
	Graphics::PixelFormat format;

	switch (color_depth) {
	case 8:
		format = Graphics::PixelFormat::createFormatCLUT8();
		break;
	case 16:
		format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
		break;
	case 32:
		format = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
		break;
	default:
		error("Invalid color depth");
	}

	BITMAP *bitmap = new Surface(width, height, format);
	return bitmap;
}

BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height) {
	Graphics::ManagedSurface &surf = **parent;
	return new Surface(surf, Common::Rect(x, y, x + width, y + height));
}

BITMAP *create_video_bitmap(int width, int height) {
	return new Screen(width, height);
}

BITMAP *create_system_bitmap(int width, int height) {
	return create_bitmap(width, height);
}

void destroy_bitmap(BITMAP *bitmap) {
	delete bitmap;
}

} // namespace AGS3
