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

#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/surface.h"
#include "ags/globals.h"

namespace AGS3 {

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
			srcYCtr = scaleYCtr / BITMAP::SCALE_THRESHOLD;
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
			int newSrcYCtr = scaleYCtr / BITMAP::SCALE_THRESHOLD;
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
				srcVal = srcP + (scaleXCtr / BITMAP::SCALE_THRESHOLD) * SrcBytesPerPixel;
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

template<bool Scale>
void BITMAP::drawGeneric(DrawInnerArgs &args) {
	if (args.sameFormat) {
		switch (format.bytesPerPixel) {
		case 1: drawInnerGeneric<1, 1, Scale>(args); break;
		case 2: drawInnerGeneric<2, 2, Scale>(args); break;
		case 4: drawInnerGeneric<4, 4, Scale>(args); break;
		}
	} else if (format.bytesPerPixel == 4 && args.src.format.bytesPerPixel == 2) { 
		drawInnerGeneric<4, 2, Scale>(args);
	} else if (format.bytesPerPixel == 2 && args.src.format.bytesPerPixel == 4) {
		drawInnerGeneric<2, 4, Scale>(args);
	}
}

template void BITMAP::drawGeneric<false>(DrawInnerArgs &);
template void BITMAP::drawGeneric<true>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<4, 4, false>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<4, 4, true>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<4, 2, false>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<4, 2, true>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<2, 4, false>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<2, 4, true>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<4, 1, false>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<4, 1, true>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<2, 1, false>(DrawInnerArgs &);
template void BITMAP::drawInnerGeneric<2, 1, true>(DrawInnerArgs &);

} // end of namespace AGS3
