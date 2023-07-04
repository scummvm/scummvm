#ifdef __aarch64__

#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

#include "ags/lib/allegro/surface_simd_neon.h"

namespace AGS3 {

// This template handles 2bpp and 4bpp, the other specializations handle 1bpp and format conversion blits
template<int DestBytesPerPixel, int SrcBytesPerPixel, int ScaleThreshold>
void BITMAP::drawInner4BppWithConv(int yStart, int xStart, uint32_t transColor, uint32_t alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
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
				memcpy(&srcBuffer[0*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 0), SrcBytesPerPixel);
				memcpy(&srcBuffer[1*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 1), SrcBytesPerPixel);
				memcpy(&srcBuffer[2*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 2), SrcBytesPerPixel);
				memcpy(&srcBuffer[3*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 3), SrcBytesPerPixel);
				scaleXCtr += scaleX*4;
				byte *destPtr = &destP[destX * (uintptr_t)DestBytesPerPixel];
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
			}/* else {
				format.colorToARGB(getColor(destVal, DestBytesPerPixel), aDest, rDest, gDest, bDest);
			}*/
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
void BITMAP::drawInner2Bpp(int yStart, int xStart, uint32_t transColor, uint32_t alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
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
			}/* else {
				format.colorToARGB((uint32)(*(uint16 *)destVal), aDest, rDest, gDest, bDest);
			}*/
			blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, srcAlpha, useTint, destVal);
			srcCol = format.ARGBToColor(aDest, rDest, gDest, bDest);
		} else {
			srcCol = format.ARGBToColor(aSrc, rSrc, gSrc, bSrc);
		}
		*(uint16 *)destVal = srcCol;
	}
}

template<int ScaleThreshold>
void BITMAP::drawInner1Bpp(int yStart, int xStart, uint32_t transColor, uint32_t alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
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


template void BITMAP::drawInner4BppWithConv<4, 4, 0>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<4, 4, 0x100>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<4, 2, 0>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<4, 2, 0x100>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<2, 4, 0>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<2, 4, 0x100>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner2Bpp<0>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner2Bpp<0x100>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner1Bpp<0>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner1Bpp<0x100>(int, int, uint32_t, uint32_t, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);

} // namespace AGS3

#endif
