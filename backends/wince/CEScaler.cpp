/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "CEScaler.h"

void PocketPCPortrait(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *work;
	int i;

	while (height--) {
		i = 0;
		work = dstPtr;

		for (int i=0; i<width; i+=4) {
			// Work with 4 pixels		
			uint16 color1 = *(((const uint16 *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)srcPtr) + (i + 2));
			uint16 color4 = *(((const uint16 *)srcPtr) + (i + 3));
		
			*(((uint16 *)work) + 0) = interpolate16_2<565, 3, 1>(color1, color2);
			*(((uint16 *)work) + 1) = interpolate16_2<565, 1, 1>(color2, color3);
			*(((uint16 *)work) + 2) = interpolate16_2<565, 1, 3>(color3, color4);
		
			work += 3 * sizeof(uint16);
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

void PocketPCHalf(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *work;
	int i;
	uint16 srcPitch16 = (uint16)(srcPitch / sizeof(uint16));

	while ((height-=2) >= 0) {
		i = 0;
		work = dstPtr;

		for (int i=0; i<width; i+=2) {
			// Another lame filter attempt :) 
			uint16 color1 = *(((const uint16 *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)srcPtr) + (i + srcPitch16));
			uint16 color4 = *(((const uint16 *)srcPtr) + (i + srcPitch16 + 1));
			*(((uint16 *)work) + 0) = interpolate16_4<565, 1, 1, 1, 1>(color1, color2, color3, color4);
			
			work += sizeof(uint16);
		}
		srcPtr += 2 * srcPitch; 
		dstPtr += dstPitch;
	}
}

