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

#ifndef AGS_LIB_ALLEGRO_SURFACE_H
#define AGS_LIB_ALLEGRO_SURFACE_H

#include "graphics/managed_surface.h"
#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/color.h"
#include "common/array.h"

#if defined(__aarch64__)
// M1/M2 SIMD intrensics
#include "arm_neon.h"
#endif
//#define WYATTOPT

namespace AGS3 {

class BITMAP {
private:
	Graphics::ManagedSurface *_owner;
	public:
	int16 &w, &h;
	int32 &pitch;
	Graphics::PixelFormat &format;
	bool clip;
	int ct, cb, cl, cr;
	Common::Array<byte *> line;
public:
	BITMAP(Graphics::ManagedSurface *owner);
	virtual ~BITMAP() {
	}

	Graphics::ManagedSurface &operator*() const {
		return *_owner;
	}
	Graphics::ManagedSurface &getSurface() {
		return *_owner;
	}
	const Graphics::ManagedSurface &getSurface() const {
		return *_owner;
	}

	unsigned char *getPixels() const {
		return (unsigned char *)_owner->getPixels();
	}

	unsigned char *getBasePtr(uint16 x, uint16 y) const {
		return (unsigned char *)_owner->getBasePtr(x, y);
	}

	uint getTransparentColor() const {
		// See allegro bitmap_mask_color
		// For paletted sprites this is 0.
		// For other color depths this is bright pink (RGB 255, 0, 255) with alpha set to 0.
		if (format.bytesPerPixel == 1)
			return 0;
		return format.ARGBToColor(0, 255, 0, 255);
	}

	inline const Common::Point getOffsetFromOwner() const {
		return _owner->getOffsetFromOwner();
	}

	int getpixel(int x, int y) const;

	void clear() {
		_owner->clear();
	}

	void makeOpaque();

	/**
	 * Draws a solid filled in circle
	 */
	void circlefill(int x, int y, int radius, int color);

	/**
	 * Fills an enclosed area starting at a given point
	 */
	void floodfill(int x, int y, int color);

	/**
	 * Draw a horizontal line
	 */
	void hLine(int x, int y, int x2, uint32 color) {
		_owner->hLine(x, y, x2, color);
	}

	/**
	 * Draw a vertical line.
	 */
	void vLine(int x, int y, int y2, uint32 color) {
		_owner->vLine(x, y, y2, color);
	}

	/**
	 * Draws the passed surface onto this one
	 */
	void draw(const BITMAP *srcBitmap, const Common::Rect &srcRect,
			  int dstX, int dstY, bool horizFlip, bool vertFlip,
			  bool skipTrans, int srcAlpha, int tintRed = -1, int tintGreen = -1,
			  int tintBlue = -1);

	/**
	 * Stretches and draws the passed surface onto this one
	 */
	void stretchDraw(const BITMAP *srcBitmap, const Common::Rect &srcRect,
					 const Common::Rect &destRect, bool skipTrans, int srcAlpha);

	inline bool isSubBitmap() const {
		return _owner->disposeAfterUse() == DisposeAfterUse::NO;
	}

	private:
	// True color blender functions
	// In Allegro all the blender functions are of the form
	// unsigned int blender_func(unsigned long x, unsigned long y, unsigned long n)
	// when x is the sprite color, y the destination color, and n an alpha value

	void blendPixel(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha, bool useTint, byte *destVal) const;
	uint32x4_t blendPixelSIMD(uint32x4_t srcCols, uint32x4_t destCols, uint32x4_t alphas) const;
	uint16x8_t blendPixelSIMD2Bpp(uint16x8_t srcCols, uint16x8_t destCols, uint16x8_t alphas) const;
#ifndef WYATTOPT
	void blendPixel(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const;
#endif

	inline void rgbBlend(uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
		// Note: the original's handling varies slightly for R & B vs G.
		// We need to exactly replicate it to ensure Lamplight City's
		// calendar puzzle works correctly
		if (alpha)
			alpha++;

		uint32 x = ((uint32)rSrc << 16) | ((uint32)gSrc << 8) | (uint32)bSrc;
		uint32 y = ((uint32)rDest << 16) | ((uint32)gDest << 8) | (uint32)bDest;

		uint32 res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * alpha / 256 + y;
		y &= 0xFF00;
		x &= 0xFF00;
		uint32 g = (x - y) * alpha / 256 + y;

		rDest = (res >> 16) & 0xff;
		gDest = (g >> 8) & 0xff;
		bDest = res & 0xff;
	}

	inline uint16x8_t rgbBlendSIMD2Bpp(uint16x8_t srcCols, uint16x8_t destCols, uint16x8_t alphas) const {
		alphas = vaddq_u16(alphas, vandq_u16(vceqq_u16(alphas, vmovq_n_u16(0)), vmovq_n_u16(1)));
		uint16x8_t srcComps[] = {
			vandq_u16(srcCols, vmovq_n_u16(0x1f)),
			vandq_u16(vshrq_n_u16(srcCols, 5), vmovq_n_u16(0x3f)),
			vshrq_n_u16(srcCols, 11),
		}, destComps[] = {
			vandq_u16(destCols, vmovq_n_u16(0x1f)),
			vandq_u16(vshrq_n_u16(destCols, 5), vmovq_n_u16(0x3f)),
			vshrq_n_u16(destCols, 11),
		};
		uint16x8_t diffs[] = {
			vsubq_u16(srcComps[0], destComps[0]), // B
			vsubq_u16(srcComps[1], destComps[1]), // G
			vsubq_u16(srcComps[2], destComps[2]), // R
		};
		alphas = vshrq_n_u16(alphas, 2);
		diffs[1] = vshrq_n_u16(vmulq_u16(diffs[1], alphas), 6);
		alphas = vshrq_n_u16(alphas, 1);
		diffs[0] = vshrq_n_u16(vmulq_u16(diffs[0], alphas), 5);
		diffs[2] = vshrq_n_u16(vmulq_u16(diffs[2], alphas), 5);
		diffs[0] = vorrq_u16(diffs[0], vshlq_n_u16(diffs[1], 5));
		diffs[0] = vorrq_u16(diffs[0], vshlq_n_u16(diffs[2], 11));
		return vaddq_u16(diffs[0], destCols);
	}

	inline uint32x4_t rgbBlendSIMD(uint32x4_t srcCols, uint32x4_t destCols, uint32x4_t alphas, bool preserveAlpha) const {
		alphas = vaddq_u32(alphas, vandq_u32(vceqq_u32(alphas, vmovq_n_u32(0)), vmovq_n_u32(1)));
		uint32x4_t alpha = vandq_u32(destCols, vmovq_n_u32(0xff000000));
		uint32x4_t srcColsCopy = srcCols;
		srcColsCopy = vandq_u32(srcColsCopy, vmovq_n_u32(0xff00ff));
		uint32x4_t destColsCopy = destCols;
		destColsCopy = vandq_u32(destColsCopy, vmovq_n_u32(0xff00ff));
		srcColsCopy = vsubq_u32(srcColsCopy, destColsCopy);
		srcColsCopy = vmulq_u32(srcColsCopy, alphas);
		srcColsCopy = vshrq_n_u32(srcColsCopy, 8);
		srcColsCopy = vaddq_u32(srcColsCopy, destCols);

		srcCols = vandq_u32(srcCols, vmovq_n_u32(0xff00));
		destCols = vandq_u32(destCols, vmovq_n_u32(0xff00));
		srcCols = vsubq_u32(srcCols, destCols);
		srcCols = vmulq_u32(srcCols, alphas);
		srcCols = vshrq_n_u32(srcCols, 8);
		srcCols = vaddq_u32(srcCols, destCols);
		srcColsCopy = vandq_u32(srcColsCopy, vmovq_n_u32(0xff00ff));
		srcCols = vandq_u32(srcCols, vmovq_n_u32(0xff00));
		srcCols = vorrq_u32(srcCols, srcColsCopy);
		if (preserveAlpha) {
			srcCols = vandq_u32(srcCols, vmovq_n_u32(0x00ffffff));
			srcCols = vorrq_u32(srcCols, alpha);
		}
		return srcCols;
	}

	inline void argbBlend(uint32 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest) const {
		// Original logic has uint32 src and dst colors as ARGB8888
		// ++src_alpha;
		// uint32 dst_alpha = geta32(dst);
		// if (dst_alpha)
		//     ++dst_alpha;
		// uint32 dst_g = (dst & 0x00FF00) * dst_alpha / 256;
		// dst = (dst & 0xFF00FF) * dst_alpha / 256;
		// dst_g = (((src & 0x00FF00) - (dst_g & 0x00FF00)) * src_alpha / 256 + dst_g) & 0x00FF00;
		// dst = (((src & 0xFF00FF) - (dst & 0xFF00FF)) * src_alpha / 256 + dst) & 0xFF00FF;
		// dst_alpha = 256 - (256 - src_alpha) * (256 - dst_alpha) / 256;
		// src_alpha = /* 256 * 256 == */ 0x10000 / dst_alpha;
		// dst_g = (dst_g * src_alpha / 256) & 0x00FF00;
		// dst = (dst * src_alpha / 256) & 0xFF00FF;
		// return dst | dst_g | (--dst_alpha << 24);
		double sAlpha = (double)(aSrc & 0xff) / 255.0;
		double dAlpha = (double)aDest / 255.0;
		dAlpha *= (1.0 - sAlpha);
		rDest = static_cast<uint8>((rSrc * sAlpha + rDest * dAlpha) / (sAlpha + dAlpha));
		gDest = static_cast<uint8>((gSrc * sAlpha + gDest * dAlpha) / (sAlpha + dAlpha));
		bDest = static_cast<uint8>((bSrc * sAlpha + bDest * dAlpha) / (sAlpha + dAlpha));
		aDest = static_cast<uint8>(255. * (sAlpha + dAlpha));
	}

	inline uint32x4_t argbBlendSIMD(uint32x4_t srcCols, uint32x4_t destCols) const {
		float16x4_t sAlphas = vcvt_f16_f32(vcvtq_f32_u32(vshrq_n_u32(srcCols, 24)));
		sAlphas = vmul_n_f16(sAlphas, 1.0 / 255.0);
		float16x8_t sAlphas1 = vcombine_f16(vmov_n_f16(vduph_lane_f16(sAlphas, 0)), vmov_n_f16(vduph_lane_f16(sAlphas, 1)));
		float16x8_t sAlphas2 = vcombine_f16(vmov_n_f16(vduph_lane_f16(sAlphas, 2)), vmov_n_f16(vduph_lane_f16(sAlphas, 3)));
		float16x4_t dAlphas = vcvt_f16_f32(vcvtq_f32_u32(vshrq_n_u32(destCols, 24)));
		dAlphas = vmul_n_f16(dAlphas, 1.0 / 255.0);
		dAlphas = vmul_f16(dAlphas, vsub_f16(vmov_n_f16(1.0), sAlphas));
		float16x8_t dAlphas1 = vcombine_f16(vmov_n_f16(vduph_lane_f16(dAlphas, 0)), vmov_n_f16(vduph_lane_f16(dAlphas, 1)));
		float16x8_t dAlphas2 = vcombine_f16(vmov_n_f16(vduph_lane_f16(dAlphas, 2)), vmov_n_f16(vduph_lane_f16(dAlphas, 3)));
		float16x8_t srcRgb1 = vcvtq_f16_u16(vmovl_u8(vreinterpret_u8_u32(vget_low_u32(srcCols))));
		float16x8_t destRgb1 = vcvtq_f16_u16(vmovl_u8(vreinterpret_u8_u32(vget_low_u32(destCols))));
		float16x8_t srcRgb2 = vcvtq_f16_u16(vmovl_u8(vreinterpret_u8_u32(vget_high_u32(srcCols))));
		float16x8_t destRgb2 = vcvtq_f16_u16(vmovl_u8(vreinterpret_u8_u32(vget_high_u32(destCols))));
		srcRgb1 = vmulq_f16(srcRgb1, sAlphas1);
		destRgb1 = vmulq_f16(destRgb1, dAlphas1);
		srcRgb1 = vaddq_f16(srcRgb1, destRgb1);
		float16x8_t alphasRec = vrecpeq_f16(vaddq_f16(sAlphas1, dAlphas1));
		srcRgb1 = vmulq_f16(srcRgb1, alphasRec);
		srcRgb2 = vmulq_f16(srcRgb2, sAlphas2);
		destRgb2 = vmulq_f16(destRgb2, dAlphas2);
		srcRgb2 = vaddq_f16(srcRgb2, destRgb2);
		alphasRec = vrecpeq_f16(vaddq_f16(sAlphas2, dAlphas2));
		srcRgb2 = vmulq_f16(srcRgb2, alphasRec);
		uint16x4_t alphas = vcvta_u16_f16(vmul_n_f16(vadd_f16(sAlphas, dAlphas), 255.0));
		srcRgb1 = vcopyq_lane_u16(srcRgb1, 3, alphas, 0);
		srcRgb1 = vcopyq_lane_u16(srcRgb1, 7, alphas, 1);
		srcRgb2 = vcopyq_lane_u16(srcRgb2, 3, alphas, 2);
		srcRgb2 = vcopyq_lane_u16(srcRgb2, 7, alphas, 3);
		return vcombine_u32(vreinterpret_u32_u8(vmovn_u16(vcvtq_u16_f16(srcRgb1))), vreinterpret_u32_u8(vmovn_u16(vcvtq_u16_f16(srcRgb2))));
	}

	// kRgbToRgbBlender
	inline void blendRgbToRgb(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
		// Default mode for set_trans_blender
		rgbBlend(rSrc, gSrc, bSrc, rDest, gDest, bDest, alpha);
		// Original doesn't set alpha (so it is 0), but the function is not meant to be used
		// on bitmap with transparency. Should we set alpha to 0xff?
		aDest = 0;
	}

	// kAlphaPreservedBlenderMode
	inline void blendPreserveAlpha(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
		// Original blender function: _myblender_alpha_trans24
		// Like blendRgbToRgb, but result as the same alpha as destColor
		rgbBlend(rSrc, gSrc, bSrc, rDest, gDest, bDest, alpha);
		// Preserve value in aDest
	}

	// kArgbToArgbBlender
	inline void blendArgbToArgb(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
		// Original blender functions: _argb2argb_blender
		if (alpha == 0)
			alpha = aSrc;
		else
			alpha = aSrc * ((alpha & 0xff) + 1) / 256;
		if (alpha != 0)
			argbBlend(alpha, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest);
	}

	// kRgbToArgbBlender
	inline void blendRgbToArgb(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
		// Original blender function: _rgb2argb_blenders
		if (alpha == 0 || alpha == 0xff) {
			aDest = 0xff;
			rDest = rSrc;
			gDest = gSrc;
			bDest = bSrc;
		} else
			argbBlend(alpha, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest);
	}

	// kArgbToRgbBlender
	inline void blendArgbToRgb(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
		// Original blender function: _argb2rgb_blender
		if (alpha == 0)
			alpha = aSrc;
		else
			alpha = aSrc * ((alpha & 0xff) + 1) / 256;
		rgbBlend(rSrc, gSrc, bSrc, rDest, gDest, bDest, alpha);
		// Original doesn't set alpha (so it is 0), but the function is not meant to be used
		// on bitmap with transparency. Should we set alpha to 0xff?
		aDest = 0;
	}

	// kOpaqueBlenderMode
	inline void blendOpaque(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
		// Original blender function: _opaque_alpha_blender
		aDest = 0xff;
		rDest = rSrc;
		gDest = gSrc;
		bDest = bSrc;
	}

	// kSourceAlphaBlender
	inline void blendSourceAlpha(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
		// Used after set_alpha_blender
		// Uses alpha from source. Result is fully opaque
		rgbBlend(rSrc, gSrc, bSrc, rDest, gDest, bDest, aSrc);
		// Original doesn't set alpha (so it is 0), but the function is not meant to be used
		// on bitmap with transparency. Should we set alpha to 0xff?
		aDest = 0;
	}

	// kAdditiveBlenderMode
	inline void blendAdditiveAlpha(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha) const {
		// Original blender function: _additive_alpha_copysrc_blender
		rDest = rSrc;
		gDest = gSrc;
		bDest = bSrc;
		uint32 a = (uint32)aSrc + (uint32)aDest;
		if (a > 0xff)
			aDest = 0xff;
		else
			aDest = static_cast<uint8>(a);
	}

	// kTintBlenderMode and kTintLightBlenderMode
	void blendTintSprite(uint8 aSrc, uint8 rSrc, uint8 gSrc, uint8 bSrc, uint8 &aDest, uint8 &rDest, uint8 &gDest, uint8 &bDest, uint32 alpha, bool light) const;

	// kTintBlenderMode and kTintLightBlenderMode for SIMD
	uint32x4_t blendTintSpriteSIMD(uint32x4_t srcCols, uint32x4_t destCols, uint32x4_t alphas, bool light) const;

	inline uint32x4_t simd2BppTo4Bpp(uint16x4_t pixels) const {
		uint32x4_t x = vmovl_u16(pixels);
		uint32x4_t c = vshrq_n_u32(vandq_u32(x, vmovq_n_u32(0xf800)), 11);
		uint32x4_t r = vshlq_n_u32(vorrq_u32(vshlq_n_u32(c, 3), vshrq_n_u32(c, 2)), 16);
		c = vshrq_n_u32(vandq_u32(x, vmovq_n_u32(0x07e0)), 5);
		uint32x4_t g = vshlq_n_u32(vorrq_u32(vshlq_n_u32(c, 2), vshrq_n_u32(c, 4)), 8);
		c = vandq_u32(x, vmovq_n_u32(0x001f));
		uint32x4_t b = vorrq_u32(vshlq_n_u32(c, 3), vshrq_n_u32(c, 2));
		return vorrq_u32(vorrq_u32(vorrq_u32(r, g), b), vmovq_n_u32(0xff000000));
	}

	inline uint16x4_t simd4BppTo2Bpp(uint32x4_t pixels) const {
		uint32x4_t x = vshrq_n_u32(vandq_u32(pixels, vmovq_n_u32(0x000000ff)), 3);
		x = vorrq_u32(x, vshlq_n_u32(vshrq_n_u32(vandq_u32(pixels, vmovq_n_u32(0x0000ff00)), 8+3), 5));
		x = vorrq_u32(x, vshlq_n_u32(vshrq_n_u32(vandq_u32(pixels, vmovq_n_u32(0x00ff0000)), 16+3), 11));
		return vmovn_u32(x);
	}

	template<int DestBytesPerPixel, int SrcBytesPerPixel>
	inline void drawPixelSIMD(byte *destPtr, const byte *srcP2, uint32x4_t tint, uint32x4_t alphas, uint32x4_t maskedAlphas, uint32x4_t transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, uint32x4_t skipMask) {
		uint32x4_t srcCols, destCol;
		if (SrcBytesPerPixel == 4) {
			destCol = vld1q_u32((uint32 *)destPtr);
			srcCols = vld1q_u32((const uint32 *)(srcP2 + xDir * xCtrBpp));
		} else {
			destCol = simd2BppTo4Bpp(vld1_u16((uint16 *)destPtr));
			srcCols = simd2BppTo4Bpp(vld1_u16((const uint16 *)(srcP2 + xDir * xCtrBpp)));
		}
		uint32x4_t anded = vandq_u32(srcCols, maskedAlphas);
		if (srcAlpha != -1) {
			// take into account for useTint
			if (useTint) {
				srcCols = blendPixelSIMD(tint, srcCols, alphas);
			} else {
				srcCols = blendPixelSIMD(srcCols, destCol, alphas);
			}
		}
		uint32x4_t mask1 = skipTrans ? vceqq_u32(anded, transColors) : vmovq_n_u32(0);
		mask1 = vorrq_u32(mask1, skipMask);
		uint32x4_t destCols2 = vandq_u32(destCol, mask1);
		uint32x4_t srcCols2 = vandq_u32(srcCols, vmvnq_u32(mask1));
		uint32x4_t final = vorrq_u32(destCols2, srcCols2);
		if (horizFlip) {
			final = vrev64q_u32(final);
			final = vcombine_u32(vget_high_u32(final), vget_low_u32(final));
		}
		if (DestBytesPerPixel == 4) {
			vst1q_u32((uint32 *)destPtr, final);
		} else {
			vst1_u16((uint16 *)destPtr, simd4BppTo2Bpp(final));
		}
	}

	inline void drawPixelSIMD2Bpp(byte *destPtr, const byte *srcP2, uint16x8_t tint, uint16x8_t alphas, uint16x8_t transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, uint16x8_t skipMask) {
		uint16x8_t destCol = vld1q_u16((uint16 *)destPtr);
		uint16x8_t srcCols = vld1q_u16((const uint16 *)(srcP2 + xDir * xCtrBpp));
		if (srcAlpha != -1) {
			// take into account for useTint
			if (useTint) {
				srcCols = blendPixelSIMD2Bpp(tint, srcCols, alphas);
			} else {
				srcCols = blendPixelSIMD2Bpp(srcCols, destCol, alphas);
			}
		}
		uint16x8_t mask1 = skipTrans ? vceqq_u16(srcCols, transColors) : vmovq_n_u16(0);
		mask1 = vorrq_u16(mask1, skipMask);
		uint16x8_t destCols2 = vandq_u16(destCol, mask1);
		uint16x8_t srcCols2 = vandq_u16(srcCols, vmvnq_u16(mask1));
		uint16x8_t final = vorrq_u16(destCols2, srcCols2);
		if (horizFlip) {
			final = vrev64q_u16(final);
			final = vcombine_u16(vget_high_u16(final), vget_low_u16(final));
		}
		vst1q_u16((uint16 *)destPtr, final);
	}

	// This template handles 2bpp and 4bpp, the other specializations handle 1bpp and format conversion blits
	template<int DestBytesPerPixel, int SrcBytesPerPixel, int ScaleThreshold>
	void drawInner4BppWithConv(int yStart, int xStart, uint32_t transColor, uint32_t alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
		const int xDir = horizFlip ? -1 : 1;
		byte rSrc, gSrc, bSrc, aSrc;
		byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
		uint32x4_t tint = vshlq_n_u32(vdupq_n_u32(srcAlpha), 24);
		tint = vorrq_u32(tint, vshlq_n_u32(vdupq_n_u32(tintRed), 16));
		tint = vorrq_u32(tint, vshlq_n_u32(vdupq_n_u32(tintGreen), 8));
		tint = vorrq_u32(tint, vdupq_n_u32(tintBlue));
		uint32x4_t maskedAlphas = vld1q_dup_u32(&alphaMask);
		uint32x4_t transColors = vld1q_dup_u32(&transColor);
		uint32x4_t alphas = vld1q_dup_u32(&srcAlpha);
		uint32x4_t addIndexes = {0, 1, 2, 3};
		if (horizFlip) addIndexes = {3, 2, 1, 0};
		uint32x4_t scaleAdds = {0, (uint32)scaleX, (uint32)scaleX*2, (uint32)scaleX*3};
		
		int xCtrStart = 0, xCtrBppStart = 0, xCtrWidth = dstRect.width();
		if (xStart + xCtrWidth > destArea.w) {
			xCtrWidth = destArea.w - xStart;
		}
		if (xStart < 0) {
			xCtrStart = -xStart;
			xCtrBppStart = xCtrStart * SrcBytesPerPixel;
			xStart = 0;
		}
		int destY = yStart, srcYCtr = 0, yCtr = 0, scaleYCtr = 0, yCtrHeight = (xCtrWidth % 4 == 0) ? dstRect.height() : (dstRect.height() - 1);
		if (ScaleThreshold != 0) yCtrHeight = dstRect.height();
		if (yStart < 0) {
			yCtr = -yStart;
			destY = 0;
			if (ScaleThreshold != 0) {
				scaleYCtr = yCtr * scaleY;
				srcYCtr = scaleYCtr / ScaleThreshold;
			}
		}
		if (yStart + yCtrHeight > destArea.h) {
			yCtrHeight = destArea.h - yStart;
		}
		
		byte *destP = (byte *)destArea.getBasePtr(0, destY);
		const byte *srcP = (const byte *)src.getBasePtr(
		                       horizFlip ? srcArea.right - 4 : srcArea.left,
		                       vertFlip ? srcArea.bottom - 1 - yCtr : srcArea.top + yCtr);
		for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += scaleY) {
			uint32x4_t xCtrWidthSIMD = vdupq_n_u32(xCtrWidth);

			if (ScaleThreshold == 0) {
				for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = xStart; xCtr < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
					byte *destPtr = &destP[destX * DestBytesPerPixel];
					uint32x4_t skipMask = vcgeq_u32(vaddq_u32(vdupq_n_u32(xCtr), addIndexes), xCtrWidthSIMD);
					drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, srcAlpha, skipTrans, horizFlip, useTint, skipMask);
				}
				destP += destArea.pitch;
				srcP += vertFlip ? -src.pitch : src.pitch;
			} else {
				int newSrcYCtr = scaleYCtr / ScaleThreshold;
				if (srcYCtr != newSrcYCtr) {
					int diffSrcYCtr = newSrcYCtr - srcYCtr;
					srcP += src.pitch * diffSrcYCtr;
					srcYCtr = newSrcYCtr;
				}
				byte srcBuffer[4*4];
				for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = xStart, scaleXCtr = xCtrStart * scaleX; xCtr < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
					if (yCtr + 1 == yCtrHeight && xCtr + 4 > xCtrWidth) break;
					uint32x4_t indexes = vdupq_n_u32(scaleXCtr);
#if (ScaleThreshold == 0 || ScaleThreshold == 0x100)
					indexes = vmulq_n_u32(vshrq_n_u32(vaddq_u32(indexes, scaleAdds), 8), SrcBytesPerPixel);
#else
#error Change code to allow different scale threshold!
#endif
					memcpy(&srcBuffer[0*SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 0), SrcBytesPerPixel);
					memcpy(&srcBuffer[1*SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 1), SrcBytesPerPixel);
					memcpy(&srcBuffer[2*SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 2), SrcBytesPerPixel);
					memcpy(&srcBuffer[3*SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 3), SrcBytesPerPixel);
					scaleXCtr += scaleX*4;
					byte *destPtr = &destP[destX * DestBytesPerPixel];
					uint32x4_t skipMask = vcgeq_u32(vaddq_u32(vdupq_n_u32(xCtr), addIndexes), xCtrWidthSIMD);
					drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, (const byte *)srcBuffer, tint, alphas, maskedAlphas, transColors, 1, 0, srcAlpha, skipTrans, horizFlip, useTint, skipMask);
				}
				if (yCtr + 1 != yCtrHeight) destP += destArea.pitch;
			}
		}

		// Get the last x values of the last row
		int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = xStart;
		if (xCtrWidth % 4 == 0) return;
		if (ScaleThreshold == 0) {
			for (; xCtr + 4 < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
				byte *destPtr = &destP[destX * DestBytesPerPixel];
				drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, srcAlpha, skipTrans, horizFlip, useTint, vmovq_n_u32(0));
			}
			if (horizFlip) srcP += SrcBytesPerPixel * 3;
		} else {
			xCtr = xCtrWidth - xCtrWidth % 4;
			xCtrBpp = xCtr * SrcBytesPerPixel;
			destX = xStart+xCtr;
		}
		for (; xCtr < xCtrWidth; ++destX, ++xCtr, xCtrBpp += SrcBytesPerPixel) {
			const byte *srcColPtr = (const byte *)(srcP + xDir * xCtrBpp);
			if (ScaleThreshold != 0) {
				srcColPtr = (const byte *)(srcP + (xCtr * scaleX) / ScaleThreshold * SrcBytesPerPixel);
			}
			byte *destVal = (byte *)&destP[destX * DestBytesPerPixel];
			uint32 srcCol = getColor(srcColPtr, SrcBytesPerPixel);
			
			// Check if this is a transparent color we should skip
			if (skipTrans && ((srcCol & alphaMask) == transColor))
				continue;

			src.format.colorToARGB(srcCol, aSrc, rSrc, gSrc, bSrc);
			if (srcAlpha != -1) {
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
					format.colorToARGB(getColor(destVal, DestBytesPerPixel), aDest, rDest, gDest, bDest);
				}
				blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, srcAlpha, useTint, destVal);
				srcCol = format.ARGBToColor(aDest, rDest, gDest, bDest);
			} else {
				srcCol = format.ARGBToColor(aSrc, rSrc, gSrc, bSrc);
			}
			if (DestBytesPerPixel == 4)
				*(uint32 *)destVal = srcCol;
			else
				*(uint16 *)destVal = srcCol;
		}
	}

	template<int ScaleThreshold>
	void drawInner2Bpp(int yStart, int xStart, uint32_t transColor, uint32_t alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
		const int xDir = horizFlip ? -1 : 1;
		byte rSrc, gSrc, bSrc, aSrc;
		byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
		uint16x8_t tint = vdupq_n_u16(src.format.ARGBToColor(srcAlpha, tintRed, tintGreen, tintBlue));
		uint16x8_t transColors = vdupq_n_u16(transColor);
		uint16x8_t alphas = vdupq_n_u16(srcAlpha);
		uint16x8_t addIndexes = {0, 1, 2, 3, 4, 5, 6, 7};
		if (horizFlip) addIndexes = {7, 6, 5, 4, 3, 2, 1, 0};
		uint32x4_t scaleAdds = {0, (uint32)scaleX, (uint32)scaleX*2, (uint32)scaleX*3};
		uint32x4_t scaleAdds2 = {(uint32)scaleX*4, (uint32)scaleX*5, (uint32)scaleX*6, (uint32)scaleX*7};
		
		int xCtrStart = 0, xCtrBppStart = 0, xCtrWidth = dstRect.width();
		if (xStart + xCtrWidth > destArea.w) {
			xCtrWidth = destArea.w - xStart;
		}
		if (xStart < 0) {
			xCtrStart = -xStart;
			xCtrBppStart = xCtrStart * 2;
			xStart = 0;
		}
		int destY = yStart, yCtr = 0, srcYCtr = 0, scaleYCtr = 0, yCtrHeight = (xCtrWidth % 8 == 0) ? dstRect.height() : (dstRect.height() - 1);
		if (ScaleThreshold != 0) yCtrHeight = dstRect.height();
		if (yStart < 0) {
			yCtr = -yStart;
			destY = 0;
			if (ScaleThreshold != 0) {
				scaleYCtr = yCtr * scaleY;
				srcYCtr = scaleYCtr / ScaleThreshold;
			}
		}
		if (yStart + yCtrHeight > destArea.h) {
			yCtrHeight = destArea.h - yStart;
		}
		
		byte *destP = (byte *)destArea.getBasePtr(0, destY);
		const byte *srcP = (const byte *)src.getBasePtr(
		                       horizFlip ? srcArea.right - 8 : srcArea.left,
		                       vertFlip ? srcArea.bottom - 1 - yCtr : srcArea.top + yCtr);
		for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += scaleY) {
			uint16x8_t xCtrWidthSIMD = vmovq_n_u16(xCtrWidth);
			if (ScaleThreshold == 0) {
				for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = xStart; xCtr < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += 16) {
					byte *destPtr = &destP[destX * 2];
					uint16x8_t skipMask = vcgeq_u16(vaddq_u16(vdupq_n_u16(xCtr), addIndexes), xCtrWidthSIMD);
					drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, srcAlpha, skipTrans, horizFlip, useTint, skipMask);
				}
				destP += destArea.pitch;
				srcP += vertFlip ? -src.pitch : src.pitch;
			} else {
				int newSrcYCtr = scaleYCtr / ScaleThreshold;
				if (srcYCtr != newSrcYCtr) {
					int diffSrcYCtr = newSrcYCtr - srcYCtr;
					srcP += src.pitch * diffSrcYCtr;
					srcYCtr = newSrcYCtr;
				}
				uint16 srcBuffer[8];
				for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = xStart, scaleXCtr = xCtrStart * scaleX; xCtr < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += 16) {
					if (yCtr + 1 == yCtrHeight && xCtr + 8 > xCtrWidth) break;
					uint32x4_t indexes = vdupq_n_u32(scaleXCtr), indexes2 = vdupq_n_u32(scaleXCtr);
#if (ScaleThreshold == 0 || ScaleThreshold == 0x100)
					indexes = vmulq_n_u32(vshrq_n_u32(vaddq_u32(indexes, scaleAdds), 8), 2);
					indexes2 = vmulq_n_u32(vshrq_n_u32(vaddq_u32(indexes2, scaleAdds2), 8), 2);
#else
#error Change code to allow different scale threshold!
#endif
					srcBuffer[0] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 0));
					srcBuffer[1] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 1));
					srcBuffer[2] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 2));
					srcBuffer[3] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 3));
					srcBuffer[4] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 0));
					srcBuffer[5] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 1));
					srcBuffer[6] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 2));
					srcBuffer[7] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 3));
					scaleXCtr += scaleX*8;
					byte *destPtr = &destP[destX * 2];
					uint16x8_t skipMask = vcgeq_u16(vaddq_u16(vdupq_n_u16(xCtr), addIndexes), xCtrWidthSIMD);
					drawPixelSIMD2Bpp(destPtr, (const byte *)srcBuffer, tint, alphas, transColors, 1, 0, srcAlpha, skipTrans, horizFlip, useTint, skipMask);
				}
				if (yCtr + 1 != yCtrHeight) destP += destArea.pitch;
			}
		}

		// Get the last x values of the last row
		if (xCtrWidth % 8 == 0) return;
		int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = xStart;
		if (ScaleThreshold == 0) {
			for (; xCtr + 8 < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += 16) {
				byte *destPtr = &destP[destX * 2];
				drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, srcAlpha, skipTrans, horizFlip, useTint, vmovq_n_u16(0));
			}
			if (horizFlip) srcP += 2*3;
		} else {
			xCtr = xCtrWidth - xCtrWidth % 8;
			xCtrBpp = xCtr * 2;
			destX = xStart+xCtr;
		}
		for (; xCtr < xCtrWidth; ++destX, ++xCtr, xCtrBpp += 2) {
			const byte *srcColPtr = (const byte *)(srcP + xDir * xCtrBpp);
			if (ScaleThreshold != 0) {
				srcColPtr = (const byte *)(srcP + (xCtr * scaleX) / ScaleThreshold * 2);
			}
			byte *destVal = (byte *)&destP[destX * 2];
			uint32 srcCol = (uint32)(*(const uint16 *)srcColPtr);
			
			// Check if this is a transparent color we should skip
			if (skipTrans && srcCol == transColor)
				continue;

			src.format.colorToARGB(srcCol, aSrc, rSrc, gSrc, bSrc);
			if (srcAlpha != -1) {
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
					format.colorToARGB((uint32)(*(uint16 *)destVal), aDest, rDest, gDest, bDest);
				}
				blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, srcAlpha, useTint, destVal);
				srcCol = format.ARGBToColor(aDest, rDest, gDest, bDest);
			} else {
				srcCol = format.ARGBToColor(aSrc, rSrc, gSrc, bSrc);
			}
			*(uint16 *)destVal = srcCol;
		}
	}

	// Call drawInner with BytesPerPixel=0 if both formats aren't the same.
	template<int ScaleThreshold>
	void drawInnerGeneric(int yStart, int xStart, uint32_t transColor, uint32_t alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
		const int xDir = horizFlip ? -1 : 1;
		byte rSrc, gSrc, bSrc, aSrc;
		byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
		
		int xCtrStart = 0, xCtrBppStart = 0, xCtrWidth = dstRect.width();
		if (xStart + xCtrWidth > destArea.w) {
			xCtrWidth = destArea.w - xStart;
		}
		if (xStart < 0) {
			xCtrStart = -xStart;
			xCtrBppStart = xCtrStart * src.format.bytesPerPixel;
			xStart = 0;
		}
		int destY = yStart, yCtr = 0, srcYCtr = 0, scaleYCtr = 0, yCtrHeight = dstRect.height();
		if (yStart < 0) {
			yCtr = -yStart;
			destY = 0;
			if (ScaleThreshold != 0) {
				scaleYCtr = yCtr * scaleY;
				srcYCtr = scaleYCtr / ScaleThreshold;
			}
		}
		if (yStart + yCtrHeight > destArea.h) {
			yCtrHeight = destArea.h - yStart;
		}

		byte *destP = (byte *)destArea.getBasePtr(0, destY);
		const byte *srcP = (const byte *)src.getBasePtr(
		                       horizFlip ? srcArea.right - 1 : srcArea.left,
		                       vertFlip ? srcArea.bottom - 1 - yCtr :
		                       srcArea.top + yCtr);
		for (; yCtr < dstRect.height(); ++destY, ++yCtr, scaleYCtr += scaleY) {
			if (ScaleThreshold != 0) {
				int newSrcYCtr = scaleYCtr / ScaleThreshold;
				if (srcYCtr != newSrcYCtr) {
					int diffSrcYCtr = newSrcYCtr - srcYCtr;
					srcP += src.pitch * diffSrcYCtr;
					srcYCtr = newSrcYCtr;
				}
			}
			// Loop through the pixels of the row
			for (int destX = xStart, xCtr = xCtrStart, xCtrBpp = xCtrBppStart, scaleXCtr = xCtr * scaleX; xCtr < xCtrWidth; ++destX, ++xCtr, xCtrBpp += src.format.bytesPerPixel, scaleXCtr += scaleX) {
				const byte *srcVal = srcP + xDir * xCtrBpp;
				if (ScaleThreshold != 0) {
					srcVal = srcP + (scaleXCtr / ScaleThreshold) * src.format.bytesPerPixel;
				}
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
				} else {
					src.format.colorToARGB(srcCol, aSrc, rSrc, gSrc, bSrc);
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

				uint32 pixel = format.ARGBToColor(aDest, rDest, gDest, bDest);
				if (format.bytesPerPixel == 4)
					*(uint32 *)destVal = pixel;
				else
					*(uint16 *)destVal = pixel;
			}

			destP += destArea.pitch;
			if (ScaleThreshold == 0) srcP += vertFlip ? -src.pitch : src.pitch;
		}
	}
	
	template<int ScaleThreshold>
	void drawInner1Bpp(int yStart, int xStart, uint32_t transColor, uint32_t alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
		const int xDir = horizFlip ? -1 : 1;
		uint8x16_t transColors = vld1q_dup_u8(&transColor);
		uint32x4_t scaleAdds1 = {0, (uint32)scaleX, (uint32)scaleX*2, (uint32)scaleX*3};
		uint32x4_t scaleAdds2 = {(uint32)scaleX*4, (uint32)scaleX*5, (uint32)scaleX*6, (uint32)scaleX*7};
		uint32x4_t scaleAdds3 = {(uint32)scaleX*8, (uint32)scaleX*9, (uint32)scaleX*10, (uint32)scaleX*11};
		uint32x4_t scaleAdds4 = {(uint32)scaleX*12, (uint32)scaleX*13, (uint32)scaleX*14, (uint32)scaleX*15};
		
		int xCtrStart = 0, xCtrWidth = dstRect.width();
		if (xStart + xCtrWidth > destArea.w) {
			xCtrWidth = destArea.w - xStart;
		}
		if (xStart < 0) {
			xCtrStart = -xStart;
			xStart = 0;
		}
		int destY = yStart, yCtr = 0, srcYCtr = 0, scaleYCtr = 0, yCtrHeight = dstRect.height();
		if (ScaleThreshold != 0) yCtrHeight = dstRect.height();
		if (yStart < 0) {
			yCtr = -yStart;
			destY = 0;
			if (ScaleThreshold != 0) {
				scaleYCtr = yCtr * scaleY;
				srcYCtr = scaleYCtr / ScaleThreshold;
			}
		}
		if (yStart + yCtrHeight > destArea.h) {
			yCtrHeight = destArea.h - yStart;
		}
		
		byte *destP = (byte *)destArea.getBasePtr(0, destY);
		const byte *srcP = (const byte *)src.getBasePtr(
		                       horizFlip ? srcArea.right - 16 : srcArea.left,
		                       vertFlip ? srcArea.bottom - 1 - yCtr : srcArea.top + yCtr);
		for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += scaleY) {
			if (ScaleThreshold != 0) {
				int newSrcYCtr = scaleYCtr / ScaleThreshold;
				if (srcYCtr != newSrcYCtr) {
					int diffSrcYCtr = newSrcYCtr - srcYCtr;
					srcP += src.pitch * diffSrcYCtr;
					srcYCtr = newSrcYCtr;
				}
			}
			int xCtr = xCtrStart, destX = xStart, scaleXCtr = xCtrStart * scaleX;
			for (; xCtr + 16 < xCtrWidth; destX += 16, xCtr += 16) {
				byte *destPtr = &destP[destX];
				uint8x16_t destCols = vld1q_u8(destPtr);
				uint8x16_t srcCols = vld1q_u8(srcP + xDir * xCtr);
				if (ScaleThreshold != 0) {
					uint32x4_t indexes1 = vdupq_n_u32(scaleXCtr), indexes2 = vdupq_n_u32(scaleXCtr);
					uint32x4_t indexes3 = vdupq_n_u32(scaleXCtr), indexes4 = vdupq_n_u32(scaleXCtr);
#if (ScaleThreshold == 0 || ScaleThreshold == 0x100)
					indexes1 = vshrq_n_u32(vaddq_u32(indexes1, scaleAdds1), 8);
					indexes2 = vshrq_n_u32(vaddq_u32(indexes2, scaleAdds2), 8);
					indexes3 = vshrq_n_u32(vaddq_u32(indexes3, scaleAdds3), 8);
					indexes4 = vshrq_n_u32(vaddq_u32(indexes4, scaleAdds4), 8);
#else
#error Change code to allow different scale threshold!
#endif
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes1, 0)], srcCols, 0);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes1, 1)], srcCols, 1);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes1, 2)], srcCols, 2);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes1, 3)], srcCols, 3);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes2, 0)], srcCols, 4);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes2, 1)], srcCols, 5);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes2, 2)], srcCols, 6);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes2, 3)], srcCols, 7);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes3, 0)], srcCols, 8);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes3, 1)], srcCols, 9);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes3, 2)], srcCols, 10);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes3, 3)], srcCols, 11);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes4, 0)], srcCols, 12);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes4, 1)], srcCols, 13);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes4, 2)], srcCols, 14);
					srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes4, 3)], srcCols, 15);
					scaleXCtr += scaleX*16;
				}
				uint8x16_t mask1 = skipTrans ? vceqq_u8(srcCols, transColors) : vmovq_n_u8(0);
				uint8x16_t final = vorrq_u8(vandq_u8(srcCols, vmvnq_u8(mask1)), vandq_u8(destCols, mask1));
				if (horizFlip) {
					final = vrev64q_u8(final);
					final = vcombine_u8(vget_high_u8(final), vget_low_u8(final));
				}
				vst1q_u8(destPtr, final);
			}
			// Get the last x values
			if (horizFlip) srcP += 15;
			for (; xCtr < xCtrWidth; ++destX, ++xCtr, scaleXCtr += scaleX) {
				const byte *srcCol = (const byte *)(srcP + xDir * xCtr);
				if (ScaleThreshold != 0) {
					srcCol = (const byte *)(srcP + scaleXCtr / ScaleThreshold);
				}
				// Check if this is a transparent color we should skip
				if (skipTrans && *srcCol == transColor)
					continue;

				byte *destVal = (byte *)&destP[destX];
				*destVal = *srcCol;
			}
			if (horizFlip) srcP -= 15;
			destP += destArea.pitch;
			if (ScaleThreshold == 0) srcP += vertFlip ? -src.pitch : src.pitch;
		}
	}

	inline uint32 getColor(const byte *data, byte bpp) const {
		switch (bpp) {
		case 1:
			return *data;
		case 2:
			return *(const uint16 *)data;
		case 4:
			return *(const uint32 *)data;
		default:
			error("Unsupported format in BITMAP::getColor");
		}
	}
};

/**
 * Derived surface class
 */
class Surface : public Graphics::ManagedSurface, public BITMAP {
public:
	Surface(int width, int height, const Graphics::PixelFormat &pixelFormat) :
		Graphics::ManagedSurface(width, height, pixelFormat), BITMAP(this) {
		// Allegro uses 255, 0, 255 RGB as the transparent color
		if (pixelFormat.bytesPerPixel == 2 || pixelFormat.bytesPerPixel == 4)
			setTransparentColor(pixelFormat.RGBToColor(255, 0, 255));
	}
	Surface(Graphics::ManagedSurface &surf, const Common::Rect &bounds) :
		Graphics::ManagedSurface(surf, bounds), BITMAP(this) {
		// Allegro uses 255, 0, 255 RGB as the transparent color
		if (surf.format.bytesPerPixel == 2 || surf.format.bytesPerPixel == 4)
			setTransparentColor(surf.format.RGBToColor(255, 0, 255));
	}
	~Surface() override {
	}
};

BITMAP *create_bitmap(int width, int height);
BITMAP *create_bitmap_ex(int color_depth, int width, int height);
BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height);
BITMAP *create_video_bitmap(int width, int height);
BITMAP *create_system_bitmap(int width, int height);
void destroy_bitmap(BITMAP *bitmap);

} // namespace AGS3

#endif
