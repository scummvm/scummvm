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

#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)
template<int DestBytesPerPixel, int SrcBytesPerPixel, bool Scale>
void BITMAP::drawInnerGeneric(DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;

	// Instead of skipping pixels outside our boundary here, we just clip
	// our area instead.
	int xCtrStart = 0, xCtrBppStart = 0, xCtrWidth = args.dstRect.width();
	if (args.xStart + xCtrWidth > args.destArea.w) { // Clip the right
		xCtrWidth = args.destArea.w - args.xStart;
	}
	if (args.xStart < 0) { // Clip the left
		xCtrStart = -args.xStart;
		xCtrBppStart = xCtrStart * SrcBytesPerPixel;
		args.xStart = 0;
	}
	int destY = args.yStart, yCtr = 0, srcYCtr = 0, scaleYCtr = 0, yCtrHeight = args.dstRect.height();
	if (args.yStart < 0) { // Clip the top
		yCtr = -args.yStart;
		destY = 0;
		if (Scale) {
			scaleYCtr = yCtr * args.scaleY;
			srcYCtr = scaleYCtr / SCALE_THRESHOLD;
		}
	}
	if (args.yStart + yCtrHeight > args.destArea.h) { // Clip the bottom
		yCtrHeight = args.destArea.h - args.yStart;
	}

	byte *destP = (byte *)args.destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)args.src.getBasePtr(
	                       args.horizFlip ? args.srcArea.right - 1 : args.srcArea.left,
	                       args.vertFlip ? args.srcArea.bottom - 1 - yCtr :
	                       args.srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += args.scaleY) {
		if (Scale) {
			int newSrcYCtr = scaleYCtr / SCALE_THRESHOLD;
			if (srcYCtr != newSrcYCtr) {
				int diffSrcYCtr = newSrcYCtr - srcYCtr;
				srcP += args.src.pitch * diffSrcYCtr;
				srcYCtr = newSrcYCtr;
			}
		}
		// Loop through the pixels of the row
		for (int destX = args.xStart, xCtr = xCtrStart, xCtrBpp = xCtrBppStart, scaleXCtr = xCtr * args.scaleX; xCtr < xCtrWidth; ++destX, ++xCtr, xCtrBpp += SrcBytesPerPixel, scaleXCtr += args.scaleX) {
			const byte *srcVal = srcP + xDir * xCtrBpp;
			if (Scale) {
				srcVal = srcP + (scaleXCtr / SCALE_THRESHOLD) * SrcBytesPerPixel;
			}
			uint32 srcCol = getColor(srcVal, SrcBytesPerPixel);

			// Check if this is a transparent color we should skip
			if (args.skipTrans && ((srcCol & args.alphaMask) == args.transColor))
				continue;

			byte *destVal = (byte *)&destP[destX * DestBytesPerPixel];

			// When blitting to the same format we can just copy the color
			if (DestBytesPerPixel == 1) {
				*destVal = srcCol;
				continue;
			} else if ((DestBytesPerPixel == SrcBytesPerPixel) && args.srcAlpha == -1) {
				if (DestBytesPerPixel)
					*(uint32 *)destVal = srcCol;
				else
					*(uint16 *)destVal = srcCol;
				continue;
			}

			// We need the rgb values to do blending and/or convert between formats
			if (SrcBytesPerPixel == 1) {
				const RGB &rgb = args.palette[srcCol];
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

			if (args.srcAlpha == -1) {
				// This means we don't use blending.
				aDest = aSrc;
				rDest = rSrc;
				gDest = gSrc;
				bDest = bSrc;
			} else {
				if (args.useTint) {
					rDest = rSrc;
					gDest = gSrc;
					bDest = bSrc;
					aDest = aSrc;
					rSrc = args.tintRed;
					gSrc = args.tintGreen;
					bSrc = args.tintBlue;
					aSrc = args.srcAlpha;
				}
				blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, args.srcAlpha, args.useTint, destVal);
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

		destP += args.destArea.pitch;
		if (!Scale) srcP += args.vertFlip ? -args.src.pitch : args.src.pitch;
	}
}

BITMAP::DrawInnerArgs::DrawInnerArgs(BITMAP *dstBitmap, const BITMAP *srcBitmap,
	const Common::Rect &srcRect, const Common::Rect &_dstRect, bool _skipTrans,
	int _srcAlpha, bool _horizFlip, bool _vertFlip, int _tintRed,
	int _tintGreen, int _tintBlue, bool _doScale) : skipTrans(_skipTrans),
		srcAlpha(_srcAlpha), horizFlip(_horizFlip), vertFlip(_vertFlip),
		tintRed(_tintRed), tintGreen(_tintGreen), tintBlue(_tintBlue),
		doScale(_doScale), src(**srcBitmap), shouldDraw(false),
		useTint(_tintRed >= 0 && _tintGreen >= 0 && _tintBlue >= 0),
		blenderMode(_G(_blender_mode)), dstRect(_dstRect) {
	// Allegro disables draw when the clipping rect has negative width/height.
	// Common::Rect instead asserts, which we don't want.
	if (dstBitmap->cr <= dstBitmap->cl || dstBitmap->cb <= dstBitmap->ct)
		return;

	// Figure out the dest area that will be updated
	srcArea = srcRect;
	srcArea.clip(Common::Rect(0, 0, srcBitmap->w, srcBitmap->h));
	if (srcArea.isEmpty())
		return;
	
	if (!doScale) {
		// Ensure the src rect is constrained to the source bitmap
		dstRect.setWidth(srcArea.width());
		dstRect.setHeight(srcArea.height());
	}
	Common::Rect destRect = dstRect.findIntersectingRect(
	                            Common::Rect(dstBitmap->cl, dstBitmap->ct, dstBitmap->cr, dstBitmap->cb));
	if (destRect.isEmpty())
		// Area is entirely outside the clipping area, so nothing to draw
		return;

	// Get source and dest surface. Note that for the destination we create
	// a temporary sub-surface based on the allowed clipping area
	Graphics::ManagedSurface &dest = *dstBitmap->_owner;
	destArea = dest.getSubArea(destRect);

	// Define scaling and other stuff used by the drawing loops
	scaleX = SCALE_THRESHOLD * srcRect.width() / dstRect.width();
	scaleY = SCALE_THRESHOLD * srcRect.height() / dstRect.height();
	sameFormat = (src.format == dstBitmap->format);

	if (src.format.bytesPerPixel == 1 && dstBitmap->format.bytesPerPixel != 1) {
		for (int i = 0; i < PAL_SIZE; ++i) {
			palette[i].r = VGA_COLOR_TRANS(_G(current_palette)[i].r);
			palette[i].g = VGA_COLOR_TRANS(_G(current_palette)[i].g);
			palette[i].b = VGA_COLOR_TRANS(_G(current_palette)[i].b);
		}
	}

	transColor = 0, alphaMask = 0xff;
	if (skipTrans && src.format.bytesPerPixel != 1) {
		transColor = src.format.ARGBToColor(0, 255, 0, 255);
		alphaMask = src.format.ARGBToColor(255, 0, 0, 0);
		alphaMask = ~alphaMask;
	}

	xStart = (dstRect.left < destRect.left) ? dstRect.left - destRect.left : 0;
	yStart = (dstRect.top < destRect.top) ? dstRect.top - destRect.top : 0;
	shouldDraw = true;
}

void BITMAP::draw(const BITMAP *srcBitmap, const Common::Rect &srcRect,
                  int dstX, int dstY, bool horizFlip, bool vertFlip,
                  bool skipTrans, int srcAlpha, int tintRed, int tintGreen,
                  int tintBlue) {
	assert(format.bytesPerPixel == 2 || format.bytesPerPixel == 4 ||
	       (format.bytesPerPixel == 1 && srcBitmap->format.bytesPerPixel == 1));

	auto args = DrawInnerArgs(this, srcBitmap, srcRect, Common::Rect(dstX, dstY, dstX+1, dstY+1), skipTrans, srcAlpha, horizFlip, vertFlip, tintRed, tintGreen, tintBlue, false);
	if (!args.shouldDraw) return;
#define DRAWINNER(func) func(args)
	// Calling drawInnerXXXX with a ScaleThreshold of 0 just does normal un-scaled drawing
	if (_G(simd_flags) == AGS3::Globals::SIMD_NONE) {
		if (args.sameFormat) {
			switch (format.bytesPerPixel) {
			case 1: DRAWINNER((drawInnerGeneric<1, 1, false>)); return;
			case 2: DRAWINNER((drawInnerGeneric<2, 2, false>)); return;
			case 4: DRAWINNER((drawInnerGeneric<4, 4, false>)); return;
			}
		} else if (format.bytesPerPixel == 4 && args.src.format.bytesPerPixel == 2) { 
			DRAWINNER((drawInnerGeneric<4, 2, false>));
		} else if (format.bytesPerPixel == 2 && args.src.format.bytesPerPixel == 4) {
			DRAWINNER((drawInnerGeneric<2, 4, false>));
		}
	} else {
		if (args.sameFormat) {
			switch (format.bytesPerPixel) {
			case 1: DRAWINNER(drawInner1Bpp<false>); return;
			case 2: DRAWINNER(drawInner2Bpp<false>); return;
			case 4: DRAWINNER((drawInner4BppWithConv<4, 4, false>)); return;
			}
		} else if (format.bytesPerPixel == 4 && args.src.format.bytesPerPixel == 2) { 
			DRAWINNER((drawInner4BppWithConv<4, 2, false>));
			return;
		} else if (format.bytesPerPixel == 2 && args.src.format.bytesPerPixel == 4) {
			DRAWINNER((drawInner4BppWithConv<2, 4, false>));
			return;
		}
	}
	if (format.bytesPerPixel == 4) // src.bytesPerPixel must be 1 here
		DRAWINNER((drawInnerGeneric<4, 1, false>));
	else
		DRAWINNER((drawInnerGeneric<2, 1, false>));
#undef DRAWINNER
}

void BITMAP::stretchDraw(const BITMAP *srcBitmap, const Common::Rect &srcRect,
                         const Common::Rect &dstRect, bool skipTrans, int srcAlpha) {
	assert(format.bytesPerPixel == 2 || format.bytesPerPixel == 4 ||
	       (format.bytesPerPixel == 1 && srcBitmap->format.bytesPerPixel == 1));
	auto args = DrawInnerArgs(this, srcBitmap, srcRect, dstRect, skipTrans, srcAlpha, false, false, 0, 0, 0, true);
	if (!args.shouldDraw) return;
#define DRAWINNER(func) func(args)
	if (_G(simd_flags) == AGS3::Globals::SIMD_NONE) {
		if (args.sameFormat) {
			switch (format.bytesPerPixel) {
			case 1: DRAWINNER((drawInnerGeneric<1, 1, true>)); return;
			case 2: DRAWINNER((drawInnerGeneric<2, 2, true>)); return;
			case 4: DRAWINNER((drawInnerGeneric<4, 4, true>)); return;
			}
		} else if (format.bytesPerPixel == 4 && args.src.format.bytesPerPixel == 2) { 
			DRAWINNER((drawInnerGeneric<4, 2, true>));
		} else if (format.bytesPerPixel == 2 && args.src.format.bytesPerPixel == 4) {
			DRAWINNER((drawInnerGeneric<2, 4, true>));
		}
	} else {
		if (args.sameFormat) {
			switch (format.bytesPerPixel) {
			case 1: DRAWINNER(drawInner1Bpp<true>); return;
			case 2: DRAWINNER(drawInner2Bpp<true>); return;
			case 4: DRAWINNER((drawInner4BppWithConv<4, 4, true>)); return;
			}
		} else if (format.bytesPerPixel == 4 && args.src.format.bytesPerPixel == 2) { 
			DRAWINNER((drawInner4BppWithConv<4, 2, true>));
			return;
		} else if (format.bytesPerPixel == 2 && args.src.format.bytesPerPixel == 4) {
			DRAWINNER((drawInner4BppWithConv<2, 4, true>));
			return;
		}
	}
	if (format.bytesPerPixel == 4) // src.bytesPerPixel must be 1 here
		DRAWINNER((drawInnerGeneric<4, 1, true>));
	else
		DRAWINNER((drawInnerGeneric<2, 1, true>));
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
