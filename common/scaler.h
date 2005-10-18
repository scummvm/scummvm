/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * $Header$
 */

#ifndef COMMON_SCALER_H
#define COMMON_SCALER_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "graphics/surface.h"

extern void InitScalers(uint32 BitFormat);

typedef void ScalerProc(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height);

#define DECLARE_SCALER(x)	\
	extern void x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, \
					uint32 dstPitch, int width, int height)

DECLARE_SCALER(_2xSaI);
DECLARE_SCALER(Super2xSaI);
DECLARE_SCALER(SuperEagle);
DECLARE_SCALER(AdvMame2x);
DECLARE_SCALER(AdvMame3x);
DECLARE_SCALER(Normal1x);
DECLARE_SCALER(Normal2x);
DECLARE_SCALER(Normal3x);
DECLARE_SCALER(Normal1o5x);
DECLARE_SCALER(TV2x);
DECLARE_SCALER(DotMatrix);

#ifndef DISABLE_HQ_SCALERS
DECLARE_SCALER(HQ2x);
DECLARE_SCALER(HQ3x);
#endif

FORCEINLINE int real2Aspect(int y) {
	return y + (y + 1) / 5;
}

FORCEINLINE int aspect2Real(int y) {
	return (y * 5 + 4) / 6;
}

extern void makeRectStretchable(int &x, int &y, int &w, int &h);

extern int stretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY);

// creates a 160x100 thumbnail for 320x200 games
// and 160x120 thumbnail for 320x240 and 640x480 games
// only 565 mode
enum {
	kThumbnailWidth = 160,
	kThumbnailHeight1 = 100,
	kThumbnailHeight2 = 120
};
extern void createThumbnail(const uint8* src, uint32 srcPitch, uint8* dstPtr, uint32 dstPitch, int width, int height);

/**
 * creates a thumbnail from the current screen (without overlay)
 * @param surf	a surface (will always have 16 bpp after this for now)
 * @return		false if a error occured
 */
extern bool createThumbnailFromScreen(Graphics::Surface* surf);

#endif
