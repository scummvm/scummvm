/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

namespace AGS3 {

BITMAP::BITMAP(Graphics::ManagedSurface *owner) : _owner(owner),
	w(owner->w), h(owner->h), pitch(owner->pitch), format(owner->format),
	clip(true), ct(0), cl(0), cr(owner->w), cb(owner->h) {
	line.resize(h);
	for (int y = 0; y < h; ++y)
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

void BITMAP::makeOpaque() {
	if (format.aBits() == 0)
		return;
	assert(format.bytesPerPixel == 4);
	uint32 alphaMask = format.ARGBToColor(0xff, 0, 0, 0);

	unsigned char *pixels = getPixels();
	for (int y = 0 ; y < h ; ++y, pixels += pitch) {
		uint32 *data = (uint32 *)pixels;
		for (int x = 0 ; x < w ; ++x, ++data)
			(*data) |= alphaMask;
	}
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
#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)
template<int DestBytesPerPixel, int SrcBytesPerPixel, int ScaleThreshold>
void BITMAP::drawInnerGeneric(int yStart, int xStart, uint32_t transColor, uint32_t alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
	const int xDir = horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;

	// Instead of skipping pixels outside our boundary here, we just clip
	// our area instead.
	int xCtrStart = 0, xCtrBppStart = 0, xCtrWidth = dstRect.width();
	if (xStart + xCtrWidth > destArea.w) { // Clip the right
		xCtrWidth = destArea.w - xStart;
	}
	if (xStart < 0) { // Clip the left
		xCtrStart = -xStart;
		xCtrBppStart = xCtrStart * SrcBytesPerPixel;
		xStart = 0;
	}
	int destY = yStart, yCtr = 0, srcYCtr = 0, scaleYCtr = 0, yCtrHeight = dstRect.height();
	if (yStart < 0) { // Clip the top
		yCtr = -yStart;
		destY = 0;
		if (ScaleThreshold != 0) {
			scaleYCtr = yCtr * scaleY;
			srcYCtr = scaleYCtr / ScaleThreshold;
		}
	}
	if (yStart + yCtrHeight > destArea.h) { // Clip the bottom
		yCtrHeight = destArea.h - yStart;
	}

	byte *destP = (byte *)destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)src.getBasePtr(
	                       horizFlip ? srcArea.right - 1 : srcArea.left,
	                       vertFlip ? srcArea.bottom - 1 - yCtr :
	                       srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += scaleY) {
		if (ScaleThreshold != 0) {
			int newSrcYCtr = scaleYCtr / ScaleThreshold;
			if (srcYCtr != newSrcYCtr) {
				int diffSrcYCtr = newSrcYCtr - srcYCtr;
				srcP += src.pitch * diffSrcYCtr;
				srcYCtr = newSrcYCtr;
			}
		}
		// Loop through the pixels of the row
		for (int destX = xStart, xCtr = xCtrStart, xCtrBpp = xCtrBppStart, scaleXCtr = xCtr * scaleX; xCtr < xCtrWidth; ++destX, ++xCtr, xCtrBpp += SrcBytesPerPixel, scaleXCtr += scaleX) {
			const byte *srcVal = srcP + xDir * xCtrBpp;
			if (ScaleThreshold != 0) {
				srcVal = srcP + (scaleXCtr / ScaleThreshold) * SrcBytesPerPixel;
			}
			uint32 srcCol = getColor(srcVal, SrcBytesPerPixel);

			// Check if this is a transparent color we should skip
			if (skipTrans && ((srcCol & alphaMask) == transColor))
				continue;

			byte *destVal = (byte *)&destP[destX * DestBytesPerPixel];

			// When blitting to the same format we can just copy the color
			if (DestBytesPerPixel == 1) {
				*destVal = srcCol;
				continue;
			} else if ((DestBytesPerPixel == SrcBytesPerPixel) && srcAlpha == -1) {
				if (DestBytesPerPixel)
					*(uint32 *)destVal = srcCol;
				else
					*(uint16 *)destVal = srcCol;
				continue;
			}

			// We need the rgb values to do blending and/or convert between formats
			if (SrcBytesPerPixel == 1) {
				const RGB &rgb = palette[srcCol];
				aSrc = 0xff;
				rSrc = rgb.r;
				gSrc = rgb.g;
				bSrc = rgb.b;
			} else {
				if (SrcBytesPerPixel == 4) {
					aSrc = srcCol >> 24;
					rSrc = (srcCol >> 16) & 0xff;
					gSrc = (srcCol >> 8) & 0xff;
					bSrc = srcCol & 0xff;
				} else { // SrcBytesPerPixel == 2
					aSrc = 0xff;
					rSrc = (srcCol >> 11) & 0x1f;
					rSrc = (rSrc << 3) | (rSrc >> 2);
					gSrc = (srcCol >> 5) & 0x3f;
					gSrc = (gSrc << 2) | (gSrc >> 4);
					bSrc = srcCol & 0x1f;
					bSrc = (bSrc << 3) | (bSrc >> 2);
				}
				//src.format.colorToARGB(srcCol, aSrc, rSrc, gSrc, bSrc);
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
				}
				blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, srcAlpha, useTint, destVal);
			}

			uint32 pixel;// = format.ARGBToColor(aDest, rDest, gDest, bDest);
			if (DestBytesPerPixel == 4) {
				pixel = (aDest << 24) | (rDest << 16) | (gDest << 8) | (bDest);
				*(uint32 *)destVal = pixel;
			}
			else {
				pixel = ((rDest >> 3) << 11) | ((gDest >> 2) << 5) | (bDest >> 3);
				*(uint16 *)destVal = pixel;
			}
		}

		destP += destArea.pitch;
		if (ScaleThreshold == 0) srcP += vertFlip ? -src.pitch : src.pitch;
	}
}

void BITMAP::draw(const BITMAP *srcBitmap, const Common::Rect &srcRect,
                  int dstX, int dstY, bool horizFlip, bool vertFlip,
                  bool skipTrans, int srcAlpha, int tintRed, int tintGreen,
                  int tintBlue) {
	assert(format.bytesPerPixel == 2 || format.bytesPerPixel == 4 ||
	       (format.bytesPerPixel == 1 && srcBitmap->format.bytesPerPixel == 1));

	// Allegro disables draw when the clipping rect has negative width/height.
	// Common::Rect instead asserts, which we don't want.
	if (cr <= cl || cb <= ct)
		return;

	// Ensure the src rect is constrained to the source bitmap
	Common::Rect srcArea = srcRect;
	srcArea.clip(Common::Rect(0, 0, srcBitmap->w, srcBitmap->h));
	if (srcArea.isEmpty())
		return;

	// Figure out the dest area that will be updated
	Common::Rect dstRect(dstX, dstY, dstX + srcArea.width(), dstY + srcArea.height());
	Common::Rect destRect = dstRect.findIntersectingRect(
	                            Common::Rect(cl, ct, cr, cb));
	if (destRect.isEmpty())
		// Area is entirely outside the clipping area, so nothing to draw
		return;

	// Get source and dest surface. Note that for the destination we create
	// a temporary sub-surface based on the allowed clipping area
	const Graphics::ManagedSurface &src = **srcBitmap;
	Graphics::ManagedSurface &dest = *_owner;
	Graphics::Surface destArea = dest.getSubArea(destRect);

	// Define scaling and other stuff used by the drawing loops
	bool useTint = (tintRed >= 0 && tintGreen >= 0 && tintBlue >= 0);
	bool sameFormat = (src.format == format);

	PALETTE palette;
	if (src.format.bytesPerPixel == 1 && format.bytesPerPixel != 1) {
		for (int i = 0; i < PAL_SIZE; ++i) {
			palette[i].r = VGA_COLOR_TRANS(_G(current_palette)[i].r);
			palette[i].g = VGA_COLOR_TRANS(_G(current_palette)[i].g);
			palette[i].b = VGA_COLOR_TRANS(_G(current_palette)[i].b);
		}
	}

	uint32 transColor = 0, alphaMask = 0xff;
	if (skipTrans && src.format.bytesPerPixel != 1) {
		transColor = src.format.ARGBToColor(0, 255, 0, 255);
		alphaMask = src.format.ARGBToColor(255, 0, 0, 0);
		alphaMask = ~alphaMask;
	}

	int xStart = (dstRect.left < destRect.left) ? dstRect.left - destRect.left : 0;
	int yStart = (dstRect.top < destRect.top) ? dstRect.top - destRect.top : 0;

#define DRAWINNER(func) func(yStart, xStart, transColor, alphaMask, palette, useTint, sameFormat, src, destArea, horizFlip, vertFlip, skipTrans, srcAlpha, tintRed, tintGreen, tintBlue, dstRect, srcArea, _G(_blender_mode), 0, 0)
	// Calling drawInnerXXXX with a ScaleThreshold of 0 just does normal un-scaled drawing
	if (!_G(_bitmap_simd_optimizations)) {
		if (sameFormat) {
			switch (format.bytesPerPixel) {
			case 1: DRAWINNER((drawInnerGeneric<1, 1, 0>)); return;
			case 2: DRAWINNER((drawInnerGeneric<2, 2, 0>)); return;
			case 4: DRAWINNER((drawInnerGeneric<4, 4, 0>)); return;
			}
		} else if (format.bytesPerPixel == 4 && src.format.bytesPerPixel == 2) { 
			DRAWINNER((drawInnerGeneric<4, 2, 0>));
		} else if (format.bytesPerPixel == 2 && src.format.bytesPerPixel == 4) {
			DRAWINNER((drawInnerGeneric<2, 4, 0>));
		}
	} else {
		if (sameFormat) {
			switch (format.bytesPerPixel) {
			case 1: DRAWINNER(drawInner1Bpp<0>); return;
			case 2: DRAWINNER(drawInner2Bpp<0>); return;
			case 4: DRAWINNER((drawInner4BppWithConv<4, 4, 0>)); return;
			}
		} else if (format.bytesPerPixel == 4 && src.format.bytesPerPixel == 2) { 
			DRAWINNER((drawInner4BppWithConv<4, 2, 0>));
			return;
		} else if (format.bytesPerPixel == 2 && src.format.bytesPerPixel == 4) {
			DRAWINNER((drawInner4BppWithConv<2, 4, 0>));
			return;
		}
	}
	if (format.bytesPerPixel == 4) // src.bytesPerPixel must be 1 here
		DRAWINNER((drawInnerGeneric<4, 1, 0>));
	else
		DRAWINNER((drawInnerGeneric<2, 1, 0>));
#undef DRAWINNER
}

void BITMAP::stretchDraw(const BITMAP *srcBitmap, const Common::Rect &srcRect,
                         const Common::Rect &dstRect, bool skipTrans, int srcAlpha) {
	assert(format.bytesPerPixel == 2 || format.bytesPerPixel == 4 ||
	       (format.bytesPerPixel == 1 && srcBitmap->format.bytesPerPixel == 1));

	// Allegro disables draw when the clipping rect has negative width/height.
	// Common::Rect instead asserts, which we don't want.
	if (cr <= cl || cb <= ct)
		return;

	// Figure out the dest area that will be updated
	Common::Rect destRect = dstRect.findIntersectingRect(
	                            Common::Rect(cl, ct, cr, cb));
	if (destRect.isEmpty())
		// Area is entirely outside the clipping area, so nothing to draw
		return;

	// Get source and dest surface. Note that for the destination we create
	// a temporary sub-surface based on the allowed clipping area
	const Graphics::ManagedSurface &src = **srcBitmap;
	Graphics::ManagedSurface &dest = *_owner;
	Graphics::Surface destArea = dest.getSubArea(destRect);

	// Define scaling and other stuff used by the drawing loops
	const int scaleX = SCALE_THRESHOLD * srcRect.width() / dstRect.width();
	const int scaleY = SCALE_THRESHOLD * srcRect.height() / dstRect.height();
	bool sameFormat = (src.format == format);

	PALETTE palette;
	if (src.format.bytesPerPixel == 1 && format.bytesPerPixel != 1) {
		for (int i = 0; i < PAL_SIZE; ++i) {
			palette[i].r = VGA_COLOR_TRANS(_G(current_palette)[i].r);
			palette[i].g = VGA_COLOR_TRANS(_G(current_palette)[i].g);
			palette[i].b = VGA_COLOR_TRANS(_G(current_palette)[i].b);
		}
	}

	uint32 transColor = 0, alphaMask = 0xff;
	if (skipTrans && src.format.bytesPerPixel != 1) {
		transColor = src.format.ARGBToColor(0, 255, 0, 255);
		alphaMask = src.format.ARGBToColor(255, 0, 0, 0);
		alphaMask = ~alphaMask;
	}

	int xStart = (dstRect.left < destRect.left) ? dstRect.left - destRect.left : 0;
	int yStart = (dstRect.top < destRect.top) ? dstRect.top - destRect.top : 0;

#define DRAWINNER(func) func(yStart, xStart, transColor, alphaMask, palette, 0, sameFormat, src, destArea, false, false, skipTrans, srcAlpha, 0, 0, 0, dstRect, srcRect, _G(_blender_mode), scaleX, scaleY)
	if (!_G(_bitmap_simd_optimizations)) {
		if (sameFormat) {
			switch (format.bytesPerPixel) {
			case 1: DRAWINNER((drawInnerGeneric<1, 1, SCALE_THRESHOLD>)); return;
			case 2: DRAWINNER((drawInnerGeneric<2, 2, SCALE_THRESHOLD>)); return;
			case 4: DRAWINNER((drawInnerGeneric<4, 4, SCALE_THRESHOLD>)); return;
			}
		} else if (format.bytesPerPixel == 4 && src.format.bytesPerPixel == 2) { 
			DRAWINNER((drawInnerGeneric<4, 2, SCALE_THRESHOLD>));
		} else if (format.bytesPerPixel == 2 && src.format.bytesPerPixel == 4) {
			DRAWINNER((drawInnerGeneric<2, 4, SCALE_THRESHOLD>));
		}
	} else {
		if (sameFormat) {
			switch (format.bytesPerPixel) {
			case 1: DRAWINNER(drawInner1Bpp<SCALE_THRESHOLD>); return;
			case 2: DRAWINNER(drawInner2Bpp<SCALE_THRESHOLD>); return;
			case 4: DRAWINNER((drawInner4BppWithConv<4, 4, SCALE_THRESHOLD>)); return;
			}
		} else if (format.bytesPerPixel == 4 && src.format.bytesPerPixel == 2) { 
			DRAWINNER((drawInner4BppWithConv<4, 2, SCALE_THRESHOLD>));
			return;
		} else if (format.bytesPerPixel == 2 && src.format.bytesPerPixel == 4) {
			DRAWINNER((drawInner4BppWithConv<2, 4, SCALE_THRESHOLD>));
			return;
		}
	}
	if (format.bytesPerPixel == 4) // src.bytesPerPixel must be 1 here
		DRAWINNER((drawInnerGeneric<4, 1, SCALE_THRESHOLD>));
	else
		DRAWINNER((drawInnerGeneric<2, 1, SCALE_THRESHOLD>));
#undef DRAWINNER
}
void BITMAP::blendPixel(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha, bool useTint, byte *destVal) const {
	switch (_G(_blender_mode)) {
	case kSourceAlphaBlender:
		if (!useTint) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendSourceAlpha(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kArgbToArgbBlender:
		if (!useTint) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendArgbToArgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kArgbToRgbBlender:
		if (!useTint) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendArgbToRgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kRgbToArgbBlender:
		if (!useTint) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendRgbToArgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kRgbToRgbBlender:
		if (!useTint) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendRgbToRgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kAlphaPreservedBlenderMode:
		if (!useTint) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendPreserveAlpha(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kOpaqueBlenderMode:
		blendOpaque(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kAdditiveBlenderMode:
		if (!useTint) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
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
		// (I think the writer meant value, since they are using hsV)
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
	return create_bitmap_ex(get_color_depth(), width, height);
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
	width = MAX(width, 0);
	height = MAX(height, 0);
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
