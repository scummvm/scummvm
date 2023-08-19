#include "ags/lib/allegro/surface_simd_neon.h"

#ifdef AGS_LIB_ALLEGRO_SURFACE_SIMD_NEON_IMPL

#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

namespace AGS3 {

// This template handles 2bpp and 4bpp, the other specializations handle 1bpp and format conversion blits
template<int DestBytesPerPixel, int SrcBytesPerPixel, int ScaleThreshold>
void BITMAP::drawInner4BppWithConv(BITMAP::DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
	uint32x4_t tint = vshlq_n_u32(vdupq_n_u32(args.srcAlpha), 24);
	tint = vorrq_u32(tint, vshlq_n_u32(vdupq_n_u32(args.tintRed), 16));
	tint = vorrq_u32(tint, vshlq_n_u32(vdupq_n_u32(args.tintGreen), 8));
	tint = vorrq_u32(tint, vdupq_n_u32(args.tintBlue));
	uint32x4_t maskedAlphas = vld1q_dup_u32(&args.alphaMask);
	uint32x4_t transColors = vld1q_dup_u32(&args.transColor);
	uint32x4_t alphas = vld1q_dup_u32(&args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	uint32x4_t addIndexes = {0, 1, 2, 3};
	if (args.horizFlip) addIndexes = {3, 2, 1, 0};

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	uint32x4_t scaleAdds = {0, (uint32)args.scaleX, (uint32)args.scaleX*2, (uint32)args.scaleX*3};

	// Clip the bounds ahead of time (so we don't waste time checking if we are in bounds when
	// we are in the inner loop)
	int xCtrStart = 0, xCtrBppStart = 0, xCtrWidth = args.dstRect.width();
	if (args.xStart + xCtrWidth > args.destArea.w) {
		xCtrWidth = args.destArea.w - args.xStart;
	}
	if (args.xStart < 0) {
		xCtrStart = -args.xStart;
		xCtrBppStart = xCtrStart * SrcBytesPerPixel;
		args.xStart = 0;
	}
	int destY = args.yStart, srcYCtr = 0, yCtr = 0, scaleYCtr = 0, yCtrHeight = (xCtrWidth % 4 == 0) ? args.dstRect.height() : (args.dstRect.height() - 1);
	if (ScaleThreshold != 0) yCtrHeight = args.dstRect.height();
	if (args.yStart < 0) {
		yCtr = -args.yStart;
		destY = 0;
		if (ScaleThreshold != 0) {
			scaleYCtr = yCtr * args.scaleY;
			srcYCtr = scaleYCtr / ScaleThreshold;
		}
	}
	if (args.yStart + yCtrHeight > args.destArea.h) {
		yCtrHeight = args.destArea.h - args.yStart;
	}
	
	byte *destP = (byte *)args.destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)args.src.getBasePtr(
	                       args.horizFlip ? args.srcArea.right - 4 : args.srcArea.left,
	                       args.vertFlip ? args.srcArea.bottom - 1 - yCtr : args.srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += args.scaleY) {
		uint32x4_t xCtrWidthSIMD = vdupq_n_u32(xCtrWidth); // This is the width of the row

		if (ScaleThreshold == 0) {
			// If we are not scaling the image
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart; xCtr < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
				byte *destPtr = &destP[destX * DestBytesPerPixel];
				// Skip pixels that are beyond the row
				uint32x4_t skipMask = vcgeq_u32(vaddq_u32(vdupq_n_u32(xCtr), addIndexes), xCtrWidthSIMD);
				drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, skipMask);
			}
			// Goto next row in source and destination image
			destP += args.destArea.pitch;
			srcP += args.vertFlip ? -args.src.pitch : args.src.pitch;
		} else {
			// Here we are scaling the image
			int newSrcYCtr = scaleYCtr / ScaleThreshold;
			// Since the source yctr might not update every row of the destination, we have
			// to see if we are on a new row...
			if (srcYCtr != newSrcYCtr) {
				int diffSrcYCtr = newSrcYCtr - srcYCtr; // Have we moved yet
				srcP += args.src.pitch * diffSrcYCtr;
				srcYCtr = newSrcYCtr;
			}

			// Now also since we might skip a pixel or 2 or duplicate one to reach the desired
			// scaling size, we create a small dummy buffer that we copy the pixels into and then
			// call the drawPixelsSIMD function
			byte srcBuffer[4*4];
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX; xCtr < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
				if (yCtr + 1 == yCtrHeight && xCtr + 4 > xCtrWidth) break; // Don't go past the last 4 pixels
				uint32x4_t indexes = vdupq_n_u32(scaleXCtr);
#if (ScaleThreshold == 0 || ScaleThreshold == 0x100)
				// Calculate in parallel the indexes of the pixels
				indexes = vmulq_n_u32(vshrq_n_u32(vaddq_u32(indexes, scaleAdds), 8), SrcBytesPerPixel);
#else
#error Change code to allow different scale threshold!
#endif
				// Simply memcpy them in. memcpy has no real performance overhead here
				memcpy(&srcBuffer[0*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 0), SrcBytesPerPixel);
				memcpy(&srcBuffer[1*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 1), SrcBytesPerPixel);
				memcpy(&srcBuffer[2*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 2), SrcBytesPerPixel);
				memcpy(&srcBuffer[3*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 3), SrcBytesPerPixel);
				scaleXCtr += args.scaleX*4;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actuall source bitmap
				byte *destPtr = &destP[destX * (uintptr_t)DestBytesPerPixel];
				uint32x4_t skipMask = vcgeq_u32(vaddq_u32(vdupq_n_u32(xCtr), addIndexes), xCtrWidthSIMD);
				drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, (const byte *)srcBuffer, tint, alphas, maskedAlphas, transColors, 1, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, skipMask);
			}
			// We calculate every row here except the last (because then we need to
			// check for if we fall off the edge of the row)
			// The only exception here is scaling drawing this is because:
			// 1) if statements are costly, and the less we do the faster this loop is
			// 2) with this, the only branch in the normal drawing loop is the width check
			// 3) the scaling code will actually draw the until the last 4 pixels of the image
			//    and do the extra if checks because the scaling code is already much slower
			//    than the normal drawing loop, and the less duplicate code helps here.
			if (yCtr + 1 != yCtrHeight) destP += args.destArea.pitch;
		}
	}

	// Get the last x values of the last row
	int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
	// We have a picture that is a multiple of 4, so no extra pixels to draw
	if (xCtrWidth % 4 == 0) return;
	// Drawing the last few not scaled pixels here.
	// Same as the loop above but now we check if we are going to overflow,
	// and thus we don't need to mask out pixels that go over the row.
	if (ScaleThreshold == 0) {
		for (; xCtr + 4 < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
			byte *destPtr = &destP[(ptrdiff_t)destX * DestBytesPerPixel];
			drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, vmovq_n_u32(0));
		}
		// Because we move in 4 pixel units, and horizFlip moves in 1, we have to move
		// 1 pixel past the last pixel we did not blit, meaning going forward 3 pixels.
		if (args.horizFlip) srcP += SrcBytesPerPixel * 3;
	} else {
		// So if we are scaling, set up the xCtr to what it was before (AKA the last 4 or so pixels of the image)
		xCtr = xCtrWidth - xCtrWidth % 4;
		xCtrBpp = xCtr * SrcBytesPerPixel;
		destX = args.xStart+xCtr;
	}

	// For the last 4 pixels, we just do them in serial, nothing special
	for (; xCtr < xCtrWidth; ++destX, ++xCtr, xCtrBpp += SrcBytesPerPixel) {
		const byte *srcColPtr = (const byte *)(srcP + xDir * xCtrBpp);
		if (ScaleThreshold != 0) {
			srcColPtr = (const byte *)(srcP + (xCtr * args.scaleX) / ScaleThreshold * SrcBytesPerPixel);
		}
		byte *destVal = (byte *)&destP[destX * DestBytesPerPixel];
		uint32 srcCol = getColor(srcColPtr, SrcBytesPerPixel);
		
		// Check if this is a transparent color we should skip
		if (args.skipTrans && ((srcCol & args.alphaMask) == args.transColor))
			continue;

		args.src.format.colorToARGB(srcCol, aSrc, rSrc, gSrc, bSrc);
		if (args.srcAlpha != -1) {
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
void BITMAP::drawInner2Bpp(BITMAP::DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
	uint16x8_t tint = vdupq_n_u16(args.src.format.ARGBToColor(args.srcAlpha, args.tintRed, args.tintGreen, args.tintBlue));
	uint16x8_t transColors = vdupq_n_u16(args.transColor);
	uint16x8_t alphas = vdupq_n_u16(args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	uint16x8_t addIndexes = {0, 1, 2, 3, 4, 5, 6, 7};

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	if (args.horizFlip) addIndexes = {7, 6, 5, 4, 3, 2, 1, 0};
	uint32x4_t scaleAdds = {0, (uint32)args.scaleX, (uint32)args.scaleX*2, (uint32)args.scaleX*3};
	uint32x4_t scaleAdds2 = {(uint32)args.scaleX*4, (uint32)args.scaleX*5, (uint32)args.scaleX*6, (uint32)args.scaleX*7};

	// Clip the bounds ahead of time (so we don't waste time checking if we are in bounds when
	// we are in the inner loop)
	int xCtrStart = 0, xCtrBppStart = 0, xCtrWidth = args.dstRect.width();
	if (args.xStart + xCtrWidth > args.destArea.w) {
		xCtrWidth = args.destArea.w - args.xStart;
	}
	if (args.xStart < 0) {
		xCtrStart = -args.xStart;
		xCtrBppStart = xCtrStart * 2;
		args.xStart = 0;
	}
	int destY = args.yStart, yCtr = 0, srcYCtr = 0, scaleYCtr = 0, yCtrHeight = (xCtrWidth % 8 == 0) ? args.dstRect.height() : (args.dstRect.height() - 1);
	if (ScaleThreshold != 0) yCtrHeight = args.dstRect.height();
	if (args.yStart < 0) {
		yCtr = -args.yStart;
		destY = 0;
		if (ScaleThreshold != 0) {
			scaleYCtr = yCtr * args.scaleY;
			srcYCtr = scaleYCtr / ScaleThreshold;
		}
	}
	if (args.yStart + yCtrHeight > args.destArea.h) {
		yCtrHeight = args.destArea.h - args.yStart;
	}
	
	byte *destP = (byte *)args.destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)args.src.getBasePtr(
	                       args.horizFlip ? args.srcArea.right - 8 : args.srcArea.left,
	                       args.vertFlip ? args.srcArea.bottom - 1 - yCtr : args.srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += args.scaleY) {
		uint16x8_t xCtrWidthSIMD = vmovq_n_u16(xCtrWidth); // This is the width of the row
		if (ScaleThreshold == 0) {
			// If we are not scaling the image
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart; xCtr < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += 16) {
				byte *destPtr = &destP[destX * 2];
				// Skip pixels that are beyond the row
				uint16x8_t skipMask = vcgeq_u16(vaddq_u16(vdupq_n_u16(xCtr), addIndexes), xCtrWidthSIMD);
				drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, skipMask);
			}
			// Goto next row in source and destination image
			destP += args.destArea.pitch;
			srcP += args.vertFlip ? -args.src.pitch : args.src.pitch;
		} else {
			// Here we are scaling the image
			int newSrcYCtr = scaleYCtr / ScaleThreshold;
			// Since the source yctr might not update every row of the destination, we have
			// to see if we are on a new row...
			if (srcYCtr != newSrcYCtr) {
				int diffSrcYCtr = newSrcYCtr - srcYCtr;
				srcP += args.src.pitch * diffSrcYCtr;
				srcYCtr = newSrcYCtr;
			}

			// Now also since we might skip a pixel or 2 or duplicate one to reach the desired
			// scaling size, we create a small dummy buffer that we copy the pixels into and then
			// call the drawPixelsSIMD function
			uint16 srcBuffer[8];
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX; xCtr < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += 16) {
				if (yCtr + 1 == yCtrHeight && xCtr + 8 > xCtrWidth) break;
				uint32x4_t indexes = vdupq_n_u32(scaleXCtr), indexes2 = vdupq_n_u32(scaleXCtr);
#if (ScaleThreshold == 0 || ScaleThreshold == 0x100)
				// Calculate in parallel the indexes of the pixels
				indexes = vmulq_n_u32(vshrq_n_u32(vaddq_u32(indexes, scaleAdds), 8), 2);
				indexes2 = vmulq_n_u32(vshrq_n_u32(vaddq_u32(indexes2, scaleAdds2), 8), 2);
#else
#error Change code to allow different scale threshold!
#endif
				// Simply memcpy them in. memcpy has no real performance overhead here
				srcBuffer[0] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 0));
				srcBuffer[1] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 1));
				srcBuffer[2] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 2));
				srcBuffer[3] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 3));
				srcBuffer[4] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 0));
				srcBuffer[5] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 1));
				srcBuffer[6] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 2));
				srcBuffer[7] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 3));
				scaleXCtr += args.scaleX*8;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actuall source bitmap
				byte *destPtr = &destP[destX * 2];
				uint16x8_t skipMask = vcgeq_u16(vaddq_u16(vdupq_n_u16(xCtr), addIndexes), xCtrWidthSIMD);
				drawPixelSIMD2Bpp(destPtr, (const byte *)srcBuffer, tint, alphas, transColors, 1, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, skipMask);
			}
			// We calculate every row here except the last (because then we need to
			// check for if we fall off the edge of the row)
			// The only exception here is scaling drawing this is because:
			// 1) if statements are costly, and the less we do the faster this loop is
			// 2) with this, the only branch in the normal drawing loop is the width check
			// 3) the scaling code will actually draw the until the last 4 pixels of the image
			//    and do the extra if checks because the scaling code is already much slower
			//    than the normal drawing loop, and the less duplicate code helps here.
			if (yCtr + 1 != yCtrHeight) destP += args.destArea.pitch;
		}
	}

	// We have a picture that is a multiple of 8, so no extra pixels to draw
	if (xCtrWidth % 8 == 0) return;
	// Get the last x values of the last row
	int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
	// Drawing the last few not scaled pixels here.
	// Same as the loop above but now we check if we are going to overflow,
	// and thus we don't need to mask out pixels that go over the row.
	if (ScaleThreshold == 0) {
		for (; xCtr + 8 < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += 16) {
			byte *destPtr = &destP[destX * 2];
			drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, vmovq_n_u16(0));
		}
		// Because we move in 8 pixel units, and horizFlip moves in 1, we have to move
		// 1 pixel past the last pixel we did not blit, meaning going forward 7 pixels.
		if (args.horizFlip) srcP += 2 * 7;
	} else {
		// So if we are scaling, set up the xCtr to what it was before (AKA the last 8 or so pixels of the image)
		xCtr = xCtrWidth - xCtrWidth % 8;
		xCtrBpp = xCtr * 2;
		destX = args.xStart+xCtr;
	}

	// For the last 4 pixels, we just do them in serial, nothing special
	for (; xCtr < xCtrWidth; ++destX, ++xCtr, xCtrBpp += 2) {
		const byte *srcColPtr = (const byte *)(srcP + xDir * xCtrBpp);
		if (ScaleThreshold != 0) {
			srcColPtr = (const byte *)(srcP + (xCtr * args.scaleX) / ScaleThreshold * 2);
		}
		byte *destVal = (byte *)&destP[destX * 2];
		uint32 srcCol = (uint32)(*(const uint16 *)srcColPtr);
		
		// Check if this is a transparent color we should skip
		if (args.skipTrans && srcCol == args.transColor)
			continue;

		args.src.format.colorToARGB(srcCol, aSrc, rSrc, gSrc, bSrc);
		if (args.srcAlpha != -1) {
			if (args.useTint) {
				rDest = rSrc;
				gDest = gSrc;
				bDest = bSrc;
				aDest = aSrc;
				rSrc = args.tintRed;
				gSrc = args.tintGreen;
				bSrc = args.tintBlue;
				aSrc = args.srcAlpha;
			}/* else {
				format.colorToARGB((uint32)(*(uint16 *)destVal), aDest, rDest, gDest, bDest);
			}*/
			blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, args.srcAlpha, args.useTint, destVal);
			srcCol = format.ARGBToColor(aDest, rDest, gDest, bDest);
		} else {
			srcCol = format.ARGBToColor(aSrc, rSrc, gSrc, bSrc);
		}
		*(uint16 *)destVal = srcCol;
	}
}

template<int ScaleThreshold>
void BITMAP::drawInner1Bpp(BITMAP::DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	uint8x16_t transColors = vld1q_dup_u8(&args.transColor);

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	uint32x4_t scaleAdds1 = {0, (uint32)args.scaleX, (uint32)args.scaleX*2, (uint32)args.scaleX*3};
	uint32x4_t scaleAdds2 = {(uint32)args.scaleX*4, (uint32)args.scaleX*5, (uint32)args.scaleX*6, (uint32)args.scaleX*7};
	uint32x4_t scaleAdds3 = {(uint32)args.scaleX*8, (uint32)args.scaleX*9, (uint32)args.scaleX*10, (uint32)args.scaleX*11};
	uint32x4_t scaleAdds4 = {(uint32)args.scaleX*12, (uint32)args.scaleX*13, (uint32)args.scaleX*14, (uint32)args.scaleX*15};
	
	// Clip the bounds ahead of time (so we don't waste time checking if we are in bounds when
	// we are in the inner loop)
	int xCtrStart = 0, xCtrWidth = args.dstRect.width();
	if (args.xStart + xCtrWidth > args.destArea.w) {
		xCtrWidth = args.destArea.w - args.xStart;
	}
	if (args.xStart < 0) {
		xCtrStart = -args.xStart;
		args.xStart = 0;
	}
	int destY = args.yStart, yCtr = 0, srcYCtr = 0, scaleYCtr = 0, yCtrHeight = args.dstRect.height();
	if (ScaleThreshold != 0) yCtrHeight = args.dstRect.height();
	if (args.yStart < 0) {
		yCtr = -args.yStart;
		destY = 0;
		if (ScaleThreshold != 0) {
			scaleYCtr = yCtr * args.scaleY;
			srcYCtr = scaleYCtr / ScaleThreshold;
		}
	}
	if (args.yStart + yCtrHeight > args.destArea.h) {
		yCtrHeight = args.destArea.h - args.yStart;
	}
	
	byte *destP = (byte *)args.destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)args.src.getBasePtr(
	                       args.horizFlip ? args.srcArea.right - 16 : args.srcArea.left,
	                       args.vertFlip ? args.srcArea.bottom - 1 - yCtr : args.srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += args.scaleY) {
		if (ScaleThreshold != 0) {
			// So here we update the srcYCtr differently due to this being for
			// scaling
			int newSrcYCtr = scaleYCtr / ScaleThreshold;
			if (srcYCtr != newSrcYCtr) {
				// Since the source yctr might not update every row of the destination, we have
				// to see if we are on a new row...
				int diffSrcYCtr = newSrcYCtr - srcYCtr;
				srcP += args.src.pitch * diffSrcYCtr;
				srcYCtr = newSrcYCtr;
			}
		}
		int xCtr = xCtrStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX;
		for (; xCtr + 16 < xCtrWidth; destX += 16, xCtr += 16) {
			byte *destPtr = &destP[destX];

			// Here we dont use the drawPixelSIMD function because 1bpp bitmaps in allegro
			// can't have any blending applied to them
			uint8x16_t destCols = vld1q_u8(destPtr);
			uint8x16_t srcCols = vld1q_u8(srcP + xDir * xCtr);
			if (ScaleThreshold != 0) {
				// If we are scaling, we have to set each pixel individually
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
				scaleXCtr += args.scaleX*16;
			}

			// Mask out transparent pixels
			uint8x16_t mask1 = args.skipTrans ? vceqq_u8(srcCols, transColors) : vmovq_n_u8(0);
			uint8x16_t final = vorrq_u8(vandq_u8(srcCols, vmvnq_u8(mask1)), vandq_u8(destCols, mask1));
			if (args.horizFlip) {
				final = vrev64q_u8(final);
				final = vcombine_u8(vget_high_u8(final), vget_low_u8(final));
			}
			vst1q_u8(destPtr, final);
		}
		// Get the last x values

		// Because we move in 16 pixel units, and horizFlip moves in 1, we have to move
		// 1 pixel past the last pixel we did not blit, meaning going forward 15 pixels.
		if (args.horizFlip) srcP += 15;
		for (; xCtr < xCtrWidth; ++destX, ++xCtr, scaleXCtr += args.scaleX) {
			const byte *srcCol = (const byte *)(srcP + xDir * xCtr);
			if (ScaleThreshold != 0) {
				srcCol = (const byte *)(srcP + scaleXCtr / ScaleThreshold);
			}
			// Check if this is a transparent color we should skip
			if (args.skipTrans && *srcCol == args.transColor)
				continue;

			byte *destVal = (byte *)&destP[destX];
			*destVal = *srcCol;
		}
		if (args.horizFlip) srcP -= 15; // Undo what we did up there
		destP += args.destArea.pitch; // Goto next row
		// Only advance the src row by 1 every time like this if we don't scale
		if (ScaleThreshold == 0) srcP += args.vertFlip ? -args.src.pitch : args.src.pitch;
	}
}


template void BITMAP::drawInner4BppWithConv<4, 4, 0>(DrawInnerArgs &args);
template void BITMAP::drawInner4BppWithConv<4, 4, 0x100>(DrawInnerArgs &args);
template void BITMAP::drawInner4BppWithConv<4, 2, 0>(DrawInnerArgs &args);
template void BITMAP::drawInner4BppWithConv<4, 2, 0x100>(DrawInnerArgs &args);
template void BITMAP::drawInner4BppWithConv<2, 4, 0>(DrawInnerArgs &args);
template void BITMAP::drawInner4BppWithConv<2, 4, 0x100>(DrawInnerArgs &args);
template void BITMAP::drawInner2Bpp<0>(DrawInnerArgs &args);
template void BITMAP::drawInner2Bpp<0x100>(DrawInnerArgs &args);
template void BITMAP::drawInner1Bpp<0>(DrawInnerArgs &args);
template void BITMAP::drawInner1Bpp<0x100>(DrawInnerArgs &args);

} // namespace AGS3

#endif
