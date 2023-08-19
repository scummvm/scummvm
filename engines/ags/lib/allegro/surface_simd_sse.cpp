#include "ags/lib/allegro/surface_simd_sse.h"
#ifdef AGS_LIB_ALLEGRO_SURFACE_SIMD_SSE_IMPL

#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

namespace AGS3 {

inline uint32 extract32_idx0(__m128i x) {
	return _mm_cvtsi128_si32(x);
}
inline uint32 extract32_idx1(__m128i x) {
	return _mm_cvtsi128_si32(_mm_shuffle_epi32(x, _MM_SHUFFLE(1, 1, 1, 1)));
}
inline uint32 extract32_idx2(__m128i x) {
	return _mm_cvtsi128_si32(_mm_shuffle_epi32(x, _MM_SHUFFLE(2, 2, 2, 2)));
}
inline uint32 extract32_idx3(__m128i x) {
	return _mm_cvtsi128_si32(_mm_shuffle_epi32(x, _MM_SHUFFLE(3, 3, 3, 3)));
}

// This template handles 2bpp and 4bpp, the other specializations handle 1bpp and format conversion blits
template<int DestBytesPerPixel, int SrcBytesPerPixel, int ScaleThreshold>
void BITMAP::drawInner4BppWithConv(DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
    __m128i tint = _mm_sll_epi32(_mm_set1_epi32(args.srcAlpha), _mm_set1_epi32(24));
	tint = _mm_or_si128(tint, _mm_sll_epi32(_mm_set1_epi32(args.tintRed), _mm_set1_epi32(16)));
	tint = _mm_or_si128(tint, _mm_sll_epi32(_mm_set1_epi32(args.tintGreen), _mm_set1_epi32(8)));
	tint = _mm_or_si128(tint, _mm_set1_epi32(args.tintBlue));
	__m128i maskedAlphas = _mm_set1_epi32(args.alphaMask);
	__m128i transColors = _mm_set1_epi32(args.transColor);
    __m128i alphas = _mm_set1_epi32(args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	__m128i addIndexes = _mm_set_epi32(3, 2, 1, 0);
	if (args.horizFlip) addIndexes = _mm_set_epi32(0, 1, 2, 3);

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	__m128i scaleAdds = _mm_set_epi32((uint32)args.scaleX*3, (uint32)args.scaleX*2, (uint32)args.scaleX, 0);

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
		__m128i xCtrWidthSIMD = _mm_set1_epi32(xCtrWidth); // This is the width of the row

		if (ScaleThreshold == 0) {
			// If we are not scaling the image
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart; xCtr < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
				byte *destPtr = &destP[destX * DestBytesPerPixel];
				// Skip pixels that are beyond the row
				__m128i skipMask = _mm_cmpgt_epi32(_mm_add_epi32(_mm_add_epi32(_mm_set1_epi32(xCtr), addIndexes), _mm_set1_epi32(1)), xCtrWidthSIMD);
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
			byte srcBuffer[4*4] = {0};
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX; xCtr < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
				if (yCtr + 1 == yCtrHeight && xCtr + 4 > xCtrWidth) break; // Don't go past the last 4 pixels
				__m128i indexes = _mm_set1_epi32(scaleXCtr);
#if (ScaleThreshold == 0 || ScaleThreshold == 0x100)
				// Calculate in parallel the indexes of the pixels
				if (SrcBytesPerPixel == 4)
					indexes = _mm_slli_epi32(_mm_srli_epi32(_mm_add_epi32(indexes, scaleAdds), 8), 2);
				else
					indexes = _mm_slli_epi32(_mm_srli_epi32(_mm_add_epi32(indexes, scaleAdds), 8), 1);
#else
#error Change code to allow different scale threshold!
#endif
				// Simply memcpy them in. memcpy has no real performance overhead here
				memcpy(&srcBuffer[0*(size_t)SrcBytesPerPixel], srcP + extract32_idx0(indexes), SrcBytesPerPixel);
				memcpy(&srcBuffer[1*(size_t)SrcBytesPerPixel], srcP + extract32_idx1(indexes), SrcBytesPerPixel);
				memcpy(&srcBuffer[2*(size_t)SrcBytesPerPixel], srcP + extract32_idx2(indexes), SrcBytesPerPixel);
				memcpy(&srcBuffer[3*(size_t)SrcBytesPerPixel], srcP + extract32_idx3(indexes), SrcBytesPerPixel);
				scaleXCtr += args.scaleX*4;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actuall source bitmap
				byte *destPtr = &destP[destX * (intptr_t)DestBytesPerPixel];
				__m128i skipMask = _mm_cmpgt_epi32(_mm_add_epi32(_mm_add_epi32(_mm_set1_epi32(xCtr), addIndexes), _mm_set1_epi32(1)), xCtrWidthSIMD);
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
			drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm_setzero_si128());
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
void BITMAP::drawInner2Bpp(DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
	__m128i tint = _mm_set1_epi16(args.src.format.ARGBToColor(args.srcAlpha, args.tintRed, args.tintGreen, args.tintBlue));
	__m128i transColors = _mm_set1_epi16(args.transColor);
	__m128i alphas = _mm_set1_epi16(args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	__m128i addIndexes = _mm_set_epi16(7, 6, 5, 4, 3, 2, 1, 0);

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	if (args.horizFlip) addIndexes = _mm_set_epi16(0, 1, 2, 3, 4, 5, 6, 7);
	__m128i scaleAdds = _mm_set_epi32((uint32)args.scaleX*3, (uint32)args.scaleX*2, (uint32)args.scaleX, 0);
	__m128i scaleAdds2 = _mm_set_epi32((uint32)args.scaleX*7, (uint32)args.scaleX*6, (uint32)args.scaleX*5, (uint32)args.scaleX*4);

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
		__m128i xCtrWidthSIMD = _mm_set1_epi16(xCtrWidth); // This is the width of the row
		if (ScaleThreshold == 0) {
			// If we are not scaling the image
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart; xCtr < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += 16) {
				byte *destPtr = &destP[destX * 2];
				// Skip pixels that are beyond the row
				__m128i skipMask = _mm_cmpgt_epi16(_mm_add_epi16(_mm_add_epi16(_mm_set1_epi16(xCtr), addIndexes), _mm_set1_epi16(1)), xCtrWidthSIMD);
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
				__m128i indexes = _mm_set1_epi32(scaleXCtr), indexes2 = _mm_set1_epi32(scaleXCtr);
#if (ScaleThreshold == 0 || ScaleThreshold == 0x100)
				// Calculate in parallel the indexes of the pixels
				indexes = _mm_slli_epi32(_mm_srli_epi32(_mm_add_epi32(indexes, scaleAdds), 8), 1);
				indexes2 = _mm_slli_epi32(_mm_srli_epi32(_mm_add_epi32(indexes2, scaleAdds2), 8), 1);
#else
#error Change code to allow different scale threshold!
#endif
				// Simply memcpy them in. memcpy has no real performance overhead here
				srcBuffer[0] = *(const uint16 *)(srcP + extract32_idx0(indexes));
				srcBuffer[1] = *(const uint16 *)(srcP + extract32_idx1(indexes));
				srcBuffer[2] = *(const uint16 *)(srcP + extract32_idx2(indexes));
				srcBuffer[3] = *(const uint16 *)(srcP + extract32_idx3(indexes));
				srcBuffer[4] = *(const uint16 *)(srcP + extract32_idx0(indexes2));
				srcBuffer[5] = *(const uint16 *)(srcP + extract32_idx1(indexes2));
				srcBuffer[6] = *(const uint16 *)(srcP + extract32_idx2(indexes2));
				srcBuffer[7] = *(const uint16 *)(srcP + extract32_idx3(indexes2));
				scaleXCtr += args.scaleX*8;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actuall source bitmap
				byte *destPtr = &destP[destX * 2];
				__m128i skipMask = _mm_cmpgt_epi16(_mm_add_epi16(_mm_add_epi16(_mm_set1_epi16(xCtr), addIndexes), _mm_set1_epi16(1)), xCtrWidthSIMD);
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
			drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm_setzero_si128());
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
void BITMAP::drawInner1Bpp(DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	__m128i transColors = _mm_set1_epi16(args.transColor | (args.transColor << 8));

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	__m128i scaleAdds1 = _mm_set_epi32((uint32)args.scaleX*3, (uint32)args.scaleX*2, (uint32)args.scaleX, 0);
	__m128i scaleAdds2 = _mm_set_epi32((uint32)args.scaleX*7, (uint32)args.scaleX*6, (uint32)args.scaleX*5, (uint32)args.scaleX*4);
	__m128i scaleAdds3 = _mm_set_epi32((uint32)args.scaleX*11, (uint32)args.scaleX*10, (uint32)args.scaleX*9, (uint32)args.scaleX*8);
	__m128i scaleAdds4 = _mm_set_epi32((uint32)args.scaleX*15, (uint32)args.scaleX*14, (uint32)args.scaleX*13, (uint32)args.scaleX*12);
	
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
			__m128i destCols = _mm_loadu_si128((const __m128i *)destPtr);
			__m128i srcCols = _mm_loadu_si128((const __m128i *)(srcP + xDir * xCtr));
			if (ScaleThreshold != 0) {
				// If we are scaling, we have to set each pixel individually
				__m128i indexes1 = _mm_set1_epi32(scaleXCtr), indexes2 = _mm_set1_epi32(scaleXCtr);
				__m128i indexes3 = _mm_set1_epi32(scaleXCtr), indexes4 = _mm_set1_epi32(scaleXCtr);
#if (ScaleThreshold == 0 || ScaleThreshold == 0x100)
				// Calculate in parallel the indexes of the pixels
				indexes1 = _mm_srli_epi32(_mm_add_epi32(indexes1, scaleAdds1), 8);
				indexes2 = _mm_srli_epi32(_mm_add_epi32(indexes2, scaleAdds2), 8);
				indexes3 = _mm_srli_epi32(_mm_add_epi32(indexes3, scaleAdds3), 8);
				indexes4 = _mm_srli_epi32(_mm_add_epi32(indexes4, scaleAdds4), 8);
#else
#error Change code to allow different scale threshold!
#endif
				srcCols = _mm_set_epi8(
					srcP[extract32_idx3(indexes4)],
					srcP[extract32_idx2(indexes4)],
					srcP[extract32_idx1(indexes4)],
					srcP[extract32_idx0(indexes4)],
					srcP[extract32_idx3(indexes3)],
					srcP[extract32_idx2(indexes3)],
					srcP[extract32_idx1(indexes3)],
					srcP[extract32_idx0(indexes3)],
					srcP[extract32_idx3(indexes2)],
					srcP[extract32_idx2(indexes2)],
					srcP[extract32_idx1(indexes2)],
					srcP[extract32_idx0(indexes2)],
					srcP[extract32_idx3(indexes1)],
					srcP[extract32_idx2(indexes1)],
					srcP[extract32_idx1(indexes1)],
					srcP[extract32_idx0(indexes1)]);
				scaleXCtr += args.scaleX*16;
			}

			// Mask out transparent pixels
			__m128i mask1 = args.skipTrans ? _mm_cmpeq_epi8(srcCols, transColors) : _mm_setzero_si128();
			__m128i final = _mm_or_si128(_mm_andnot_si128(mask1, srcCols), _mm_and_si128(destCols, mask1));
			if (args.horizFlip) {
				__m128i final_swap16 = _mm_srli_epi16(final, 8);
				final_swap16 = _mm_or_si128(final_swap16, _mm_slli_epi16(_mm_and_si128(final, _mm_set1_epi16(0xff)), 8));
				final_swap16 = _mm_shufflelo_epi16(final_swap16, _MM_SHUFFLE(0, 1, 2, 3));
				final_swap16 = _mm_shufflehi_epi16(final_swap16, _MM_SHUFFLE(0, 1, 2, 3));
				final = _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(final_swap16), _mm_castsi128_pd(final_swap16), _MM_SHUFFLE2(0, 1)));
			}
			_mm_storeu_si128((__m128i *)destPtr, final);
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
