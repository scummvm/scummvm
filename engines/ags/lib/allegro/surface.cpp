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

#define DRAWINNER(destBPP, srcBPP) drawInner<destBPP, srcBPP>(yStart, xStart, transColor, alphaMask, palette, useTint, sameFormat, src, destArea, horizFlip, vertFlip, skipTrans, srcAlpha, tintRed, tintGreen, tintBlue, dstRect, srcArea, _G(_blender_mode))
	if (sameFormat) {
		switch (format.bytesPerPixel) {
		case 1: DRAWINNER(1, 1); break;
		case 2: DRAWINNER(2, 2); break;
		case 4: DRAWINNER(4, 4); break;
		}
	} else if (format.bytesPerPixel == 4 && src.format.bytesPerPixel == 2) { 
		DRAWINNER(4, 2);
	} else if (format.bytesPerPixel == 2 && src.format.bytesPerPixel == 4) {
		DRAWINNER(2, 4);
	} else { // Older more generic implementation (doesn't use SIMD)
		DRAWINNER(0, 0);
	}
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

	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;

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

	for (int destY = yStart, yCtr = 0, scaleYCtr = 0; yCtr < dstRect.height();
	        ++destY, ++yCtr, scaleYCtr += scaleY) {
		if (destY < 0 || destY >= destArea.h)
			continue;
		byte *destP = (byte *)destArea.getBasePtr(0, destY);
		const byte *srcP = (const byte *)src.getBasePtr(
		                       srcRect.left, srcRect.top + scaleYCtr / SCALE_THRESHOLD);

		// Loop through the pixels of the row
		for (int destX = xStart, xCtr = 0, scaleXCtr = 0; xCtr < dstRect.width();
		        ++destX, ++xCtr, scaleXCtr += scaleX) {
			if (destX < 0 || destX >= destArea.w)
				continue;

			const byte *srcVal = srcP + scaleXCtr / SCALE_THRESHOLD * src.format.bytesPerPixel;
			uint32 srcCol = getColor(srcVal, src.format.bytesPerPixel);

			// Check if this is a transparent color we should skip
			if (skipTrans && ((srcCol & alphaMask) == transColor))
				continue;

			byte *destVal = (byte *)&destP[destX * format.bytesPerPixel];

			// When blitting to the same format we can just copy the color
			if (format.bytesPerPixel == 1) {
				*destVal = srcCol;
				continue;
			} else if (sameFormat && srcAlpha == -1) {
				if (format.bytesPerPixel == 4)
					*(uint32 *)destVal = srcCol;
				else
					*(uint16 *)destVal = srcCol;
				continue;
			}

			// We need the rgb values to do blending and/or convert between formats
			if (src.format.bytesPerPixel == 1) {
				const RGB &rgb = palette[srcCol];
				aSrc = 0xff;
				rSrc = rgb.r;
				gSrc = rgb.g;
				bSrc = rgb.b;
			} else
				src.format.colorToARGB(srcCol, aSrc, rSrc, gSrc, bSrc);

			if (srcAlpha == -1) {
				// This means we don't use blending.
				aDest = aSrc;
				rDest = rSrc;
				gDest = gSrc;
				bDest = bSrc;
			} else {
				// TODO: move this to blendPixel to only do it when needed?
				// format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
				blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, srcAlpha, false, destVal);
			}

			uint32 pixel = format.ARGBToColor(aDest, rDest, gDest, bDest);
			if (format.bytesPerPixel == 4)
				*(uint32 *)destVal = pixel;
			else
				*(uint16 *)destVal = pixel;
		}
	}
}

void BITMAP::blendPixel(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha, bool useTint, byte *destVal) const {
	switch (_G(_blender_mode)) {
	case kSourceAlphaBlender:
		if (!useTint && alpha != 255) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendSourceAlpha(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kArgbToArgbBlender:
		if (!useTint && alpha != 255) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendArgbToArgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kArgbToRgbBlender:
		if (!useTint && alpha != 255) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendArgbToRgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kRgbToArgbBlender:
		if (!useTint && alpha != 255) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendRgbToArgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kRgbToRgbBlender:
		if (!useTint && alpha != 255) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendRgbToRgb(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kAlphaPreservedBlenderMode:
		if (!useTint && alpha != 255) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
		blendPreserveAlpha(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kOpaqueBlenderMode:
		blendOpaque(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, alpha);
		break;
	case kAdditiveBlenderMode:
		if (!useTint && alpha != 255) format.colorToARGB(getColor(destVal, format.bytesPerPixel), aDest, rDest, gDest, bDest);
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

uint32x4_t BITMAP::blendPixelSIMD(uint32x4_t srcCols, uint32x4_t destCols, uint32x4_t alphas) const {
	uint32x4_t srcAlphas, difAlphas, mask, ch1, ch2;
	auto setupArgbAlphas = [&]() {
		srcAlphas = vshrq_n_u32(srcCols, 24);
		difAlphas = vaddq_u32(vandq_u32(alphas, vmovq_n_u32(0xff)), vmovq_n_u32(1));
		difAlphas = vshrq_n_u32(vmulq_u32(srcAlphas, difAlphas), 8);
		difAlphas = vshlq_n_u32(difAlphas, 24);
		srcAlphas = vshlq_n_u32(srcAlphas, 24);
		mask = vceqq_u32(alphas, vmovq_n_u32(0));
		srcAlphas = vandq_u32(srcAlphas, mask);
		difAlphas = vandq_u32(srcAlphas, vmvnq_u32(mask));
		srcCols = vorrq_u32(srcCols, vorrq_u32(srcAlphas, difAlphas));
	};
	switch (_G(_blender_mode)) {
	case kSourceAlphaBlender:
		alphas = vshrq_n_u32(srcCols, 24);
		return rgbBlendSIMD(srcCols, destCols, alphas, false);
	case kArgbToArgbBlender:
		setupArgbAlphas();
		return argbBlendSIMD(srcCols, destCols);
	case kArgbToRgbBlender:
		setupArgbAlphas();
		srcCols = argbBlendSIMD(srcCols, destCols);
		return vandq_u32(srcCols, vmovq_n_u32(0x00ffffff));
	case kRgbToArgbBlender:
		ch2 = vandq_u32(srcCols, vmovq_n_u32(0x00ffffff));
		ch2 = vorrq_u32(ch2, vshlq_n_u32(alphas, 24));
		ch2 = argbBlendSIMD(ch2, destCols);
		ch1 = vorrq_u32(srcCols, vmovq_n_u32(0xff000000));
		mask = vorrq_u32(vceqq_u32(alphas, vmovq_n_u32(0)), vceqq_u32(alphas, vmovq_n_u32(0xff)));
		ch1 = vandq_u32(ch1, mask);
		ch2 = vandq_u32(ch2, vmvnq_u32(mask));
		return vorrq_u32(ch1, ch2);
	case kRgbToRgbBlender:
		return rgbBlendSIMD(srcCols, destCols, alphas, false);
	case kAlphaPreservedBlenderMode:
		return rgbBlendSIMD(srcCols, destCols, alphas, true);
	case kOpaqueBlenderMode:
		return vorrq_u32(srcCols, vmovq_n_u32(0xff000000));
	case kAdditiveBlenderMode:
		srcAlphas = vaddq_u32(vshrq_n_u32(srcCols, 24), vshrq_n_u32(destCols, 24));
		srcAlphas = vminq_u32(srcAlphas, vmovq_n_u32(0xff));
		srcCols = vandq_u32(srcCols, vmovq_n_u32(0x00ffffff));
		return vorrq_u32(srcCols, srcAlphas);
	case kTintBlenderMode:
		return blendTintSpriteSIMD(srcCols, destCols, alphas, false);
	case kTintLightBlenderMode:
		return blendTintSpriteSIMD(srcCols, destCols, alphas, true);
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

uint32x4_t BITMAP::blendTintSpriteSIMD(uint32x4_t srcCols, uint32x4_t destCols, uint32x4_t alphas, bool light) const {
	// This function is NOT 1 to 1 with the original... It just approximates it
	// It gets the value of the dest color
	// Then it gets the h and s of the srcCols

	// srcCols[0] = A | R | G | B
	// srcCols[1] = A | R | G | B
	// srcCols[2] = A | R | G | B
	// srcCols[3] = A | R | G | B
	//  ->
	// dda = { A[0], A[1], A[2], A[3] }
	// ddr = { R[0], R[1], R[2], R[3] }
	// ddg = { G[0], G[1], G[2], G[3] }
	// ddb = { B[0], B[1], B[2], B[3] }
	
	float32x4_t ddr, ddg, ddb;
	ddr = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(vshrq_n_u32(destCols, 16), vmovq_n_u32(0xff))), 1.0 / 255.0);
	ddg = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(vshrq_n_u32(destCols, 8), vmovq_n_u32(0xff))), 1.0 / 255.0);
	ddb = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(destCols, vmovq_n_u32(0xff))), 1.0 / 255.0);
	float32x4_t ssr, ssg, ssb;
	ssr = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(vshrq_n_u32(srcCols, 16), vmovq_n_u32(0xff))), 1.0 / 255.0);
	ssg = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(vshrq_n_u32(srcCols, 8), vmovq_n_u32(0xff))), 1.0 / 255.0);
	ssb = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(srcCols, vmovq_n_u32(0xff))), 1.0 / 255.0);
	float32x4_t dmaxes = vmaxq_f32(ddr, vmaxq_f32(ddg, ddb));
	float32x4_t smaxes = vmaxq_f32(ssr, vmaxq_f32(ssg, ssb));
	//float32x4_t dmins = vminq_f32(ddr, vminq_f32(ddg, ddb));
	float32x4_t smins = vminq_f32(ssr, vminq_f32(ssg, ssb));
	//float32x4_t ddelta = vsubq_f32(dmaxes, dmins);
	float32x4_t sdelta = vsubq_f32(smaxes, smins);

	float32x4_t quotient, product, hr, hg, hb, hue, sat;
	hr = vdivq_f32(vsubq_f32(ssg, ssb), sdelta);
	quotient = vdivq_f32(hr, vmovq_n_f32(6.0));
	product = vmulq_n_f32(quotient, 6.0);
	hr = vmulq_n_f32(vsubq_f32(hr, product), 60.0);
	hg = vaddq_f32(vdivq_f32(vsubq_f32(ssb, ssr), sdelta), vmovq_n_f32(2.0));
	hb = vaddq_f32(vdivq_f32(vsubq_f32(ssr, ssg), sdelta), vmovq_n_f32(4.0));
	float32x4_t hrfactors = vcvtnq_u32_f32(vandq_u32(vceqq_u32(vreinterpretq_u32_f32(ssr), vreinterpretq_u32_f32(smaxes)), vmovq_n_u32(1)));
	float32x4_t hgfactors = vcvtnq_u32_f32(vandq_u32(vceqq_u32(vreinterpretq_u32_f32(ssg), vreinterpretq_u32_f32(smaxes)), vmovq_n_u32(1)));
	float32x4_t hbfactors = vcvtnq_u32_f32(vandq_u32(vceqq_u32(vreinterpretq_u32_f32(ssb), vreinterpretq_u32_f32(smaxes)), vmovq_n_u32(1)));
	hue = vmulq_f32(hr, hrfactors);
	hue = vaddq_f32(hue, vmulq_f32(hg, hgfactors));
	hue = vaddq_f32(hue, vmulq_f32(hb, hbfactors));
	float32x4_t satfactors = vcvtnq_u32_f32(vandq_u32(vceqq_u32(vreinterpretq_u32_f32(smaxes), vmovq_n_f32(0.0)), vmovq_n_u32(1)));
	sat = vmulq_f32(satfactors, vdivq_f32(sdelta, smaxes));

	// Mess with the light
	float32x4_t val = dmaxes;
	if (light) {
		val = vsubq_f32(val, vsubq_f32(vmovq_n_f32(1.0), vmulq_n_f32(vcvtq_f32_u32(alphas), 1.0 / 250.0)));
		val = vmaxq_f32(val, vmovq_n_f32(0.0));
	}
		
	// then it stiches them back together
	float32x4_t hp = vmulq_n_f32(hue, 1.0 / 60.0);
	uint32x4_t hpi = vcvtq_u32_f32(hp);
	val = vmulq_n_f32(val, 255.0);
	uint32x4_t x = vcvtq_u32_f32(vmulq_f32(val, sat));
	uint32x4_t y = vcvtq_u32_f32(vmulq_f32(x, vsubq_f32(hue, vrndq_f32(hue))));
	val = vaddq_f32(val, vmovq_n_f32(0.5));
	uint32x4_t z = vcvtq_u32_f32(vsubq_f32(val, x));
	uint32x4_t v = vcvtq_u32_f32(val);
	
	uint32x4_t c0 = vorrq_u32(z, vorrq_u32(vshlq_n_u32(v, 16), vshlq_n_u32(vaddq_u32(z, y), 8)));
	uint32x4_t m0 = vceqq_u32(hpi, vmovq_n_u32(0));
	uint32x4_t c1 = vorrq_u32(z, vorrq_u32(vshlq_n_u32(v, 8), vshlq_n_u32(vsubq_u32(v, y), 16)));
	uint32x4_t m1 = vceqq_u32(hpi, vmovq_n_u32(1));
	uint32x4_t c2 = vorrq_u32(vshlq_n_u32(z, 16), vorrq_u32(vshlq_n_u32(v, 8), vaddq_u32(z, y)));
	uint32x4_t m2 = vceqq_u32(hpi, vmovq_n_u32(2));
	uint32x4_t c3 = vorrq_u32(v, vorrq_u32(vshlq_n_u32(z, 16), vshlq_n_u32(vsubq_u32(v, y), 8)));
	uint32x4_t m3 = vceqq_u32(hpi, vmovq_n_u32(3));
	uint32x4_t c4 = vorrq_u32(v, vorrq_u32(vshlq_n_u32(z, 8), vshlq_n_u32(vaddq_u32(z, y), 16)));
	uint32x4_t m4 = vceqq_u32(hpi, vmovq_n_u32(4));
	uint32x4_t c5 = vorrq_u32(vshlq_n_u32(v, 16), vorrq_u32(vshlq_n_u32(z, 8), vsubq_u32(v, y)));
	uint32x4_t m5 = vceqq_u32(hpi, vmovq_n_u32(5));

	uint32x4_t final = vandq_u32(c0, m0);
	final = vorrq_u32(final, vandq_u32(c1, m1));
	final = vorrq_u32(final, vandq_u32(c2, m2));
	final = vorrq_u32(final, vandq_u32(c3, m3));
	final = vorrq_u32(final, vandq_u32(c4, m4));
	final = vorrq_u32(final, vandq_u32(c5, m5));
	final = vorrq_u32(final, vandq_u32(destCols, vmovq_n_u32(0xff000000)));
	return final;
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
