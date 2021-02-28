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
#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)

void BITMAP::draw(const BITMAP *srcBitmap, const Common::Rect &srcRect,
		const Common::Rect &dstRect, bool horizFlip, bool vertFlip,
		bool skipTrans, int srcAlpha, int tintRed, int tintGreen,
		int tintBlue) {
	assert(format.bytesPerPixel == 2 || format.bytesPerPixel == 4 ||
		(format.bytesPerPixel == 1 && srcBitmap->format.bytesPerPixel == 1));

	// Get source and dest surface. Note that for the destination we create
	// a temporary sub-surface based on the allowed clipping area
	const Graphics::ManagedSurface &src = **srcBitmap;
	Graphics::ManagedSurface &allDest = *_owner;
	Graphics::ManagedSurface dest(allDest, Common::Rect(cl, ct, cr, cb));

	Common::Rect destRect = dstRect;
	destRect.translate(-cl, -ct);
	Graphics::Surface destArea = dest.getSubArea(destRect);

	// Define scaling and other stuff used by the drawing loops
	const int scaleX = SCALE_THRESHOLD * srcRect.width() / destRect.width();
	const int scaleY = SCALE_THRESHOLD * srcRect.height() / destRect.height();
	const int xDir = horizFlip ? -1 : 1;
	bool isScreenDest = dynamic_cast<Graphics::Screen *>(_owner);
	bool useTint = (tintRed >= 0 && tintGreen >= 0 && tintBlue >= 0);

	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
	uint32 pal[PALETTE_COUNT];

	Graphics::PixelFormat srcFormat = src.format;
	if (srcFormat.bytesPerPixel == 1) {
		for (int i = 0; i < PALETTE_COUNT; ++i)
			pal[i] = format.RGBToColor(
				VGA_COLOR_TRANS(_current_palette[i].r),
				VGA_COLOR_TRANS(_current_palette[i].g),
				VGA_COLOR_TRANS(_current_palette[i].b));
		srcFormat = format;
		// TODO: check if the line below is correct. If it is correct, we can simplify the skipTrans
		// below as we can use IS_TRANSPARENT() can be used for the 1 bytePerPixel case as well
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

			if (src.format.bytesPerPixel == 1 && format.bytesPerPixel == 1) {
				// TODO: Need to skip transparent color if skip_trans is true?
				//if (!skipTrans || *srcVal != 0)
				*destVal = *srcVal;
				continue;
			}
			srcFormat.colorToARGB(getColor(srcVal, src.format.bytesPerPixel, pal), aSrc, rSrc, gSrc, bSrc);

			// FIXME: I had at least one case in Black Cauldron Remake when a screen
			// clear was all the pink transparent color because blit was called,
			// and in Allegro, blit doesn't skip transparent pixels. So for now,
			// I hacked in an extra check to still skip them if blitting to screen
			if (skipTrans || isScreenDest) {
				if (src.format.bytesPerPixel == 1) {
					if (!isScreenDest && *srcVal == 0)
						continue;
				} else if (IS_TRANSPARENT(rSrc, gSrc, bSrc))
					continue;
			}

			if (srcAlpha == -1) {
				// This means we don't use blending.
				aDest = aSrc;
				rDest = rSrc;
				gDest = gSrc;
				bDest = bSrc;
			} else {
				if (useTint) {
					rDest = rSrc;
					gDest = gSrc;
					bDest = bSrc;
					aDest = aSrc;
					rSrc = tintRed;
					gSrc = tintGreen;
					bSrc = tintBlue;
					aSrc = srcAlpha;
				} else {
					// TODO: move this to blendPixel to only do it when needed?
					format.colorToARGB(getColor(destVal, format.bytesPerPixel, nullptr), aDest, rDest, gDest, bDest);
				}
				blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, srcAlpha);
			}

			uint32 pixel = format.ARGBToColor(aDest, rDest, gDest, bDest);
			if (format.bytesPerPixel == 4)
				*(uint32 *)destVal = pixel;
			else
				*(uint16 *)destVal = pixel;
		}
	}
}

void BITMAP::blendPixel(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
	if (IS_TRANSPARENT(rDest, gDest, bDest)) {
		aDest = aSrc;
		rDest = rSrc;
		gDest = gSrc;
		bDest = bSrc;
		return;
	}
	switch(_blender_mode) {
	case kSourceAlphaBlender:
		blendSourceAlpha(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kArgbToArgbBlender:
		blendArgbToArgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kArgbToRgbBlender:
		blendArgbToRgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kRgbToArgbBlender:
		blendRgbToArgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kRgbToRgbBlender:
		blendRgbToRgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kAlphaPreservedBlenderMode:
		blendPreserveAlpha(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kOpaqueBlenderMode:
		blendOpaque(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kAdditiveBlenderMode:
		blendAdditiveAlpha(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kTintBlenderMode:
		blendTintSprite(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha, false);
		break;
	case kTintLightBlenderMode:
		blendTintSprite(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha, true);
		break;
	}
}

void BITMAP::blendTintSprite(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha, bool light) const {
	// Used from draw_lit_sprite after set_blender_mode(kTintBlenderMode or kTintLightBlenderMode)
	// Original blender function: _myblender_color32 and _myblender_color32_light
	float xh, xs, xv;
	float yh, ys, yv;
	int r, g, b;
	rgb_to_hsv(rSrc, gSrc, bSrc, &xh, &xs, &xv);
	rgb_to_hsv(rDest, gDest, bDest, &yh, &ys, &yv);
	if (light) {
		// adjust luminance
		yv -= (1.0 - ((float)alpha / 250.0));
		if (yv < 0.0)
			yv = 0.0;
	}
	hsv_to_rgb(xh, xs, yv, &r, &g, &b);
	rDest = static_cast<uint8>(r & 0xff);
	gDest = static_cast<uint8>(g & 0xff);
	bDest = static_cast<uint8>(b & 0xff);
	// Preserve value in aDest
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
