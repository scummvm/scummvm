/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/system.h"

#include "cruise/cruise_main.h"

namespace Cruise {

uint8 page00[320 * 200];
uint8 page10[320 * 200];

char screen[320 * 200];
palEntry lpalette[256];
short globalAtariScreen[320 * 200 / 4];

gfxModuleDataStruct gfxModuleData = {
	0,			// field_1
	0,			// use Tandy
	0,			// use EGA
	1,			// use VGA

	page00,			// pPage00
	page10,			// pPage10
};

void gfxModuleData_gfxClearFrameBuffer(uint8 *ptr) {
	memset(ptr, 0, 64000);
}

void gfxModuleData_gfxCopyScreen(char *sourcePtr, char *destPtr) {
	memcpy(destPtr, sourcePtr, 64000);
}

void outputBit(char *buffer, int bitPlaneNumber, uint8 data) {
	*(buffer + (8000 * bitPlaneNumber)) = data;
}

void gfxModuleData_field_60(char *sourcePtr, int width, int height,
    char *destPtr, int x, int y) {
				/*
				 * int loc_1064;
				 * int loc_10AA;
				 * int loc_10AD;
				 * int loc_10C5;
				 * int loc_10DF;
				 * int loc_10EC;
				 * int loc_1147;
				 * int loc_114B;
				 * int loc_117C = 0xF8;
				 * int loc_11DC;
				 * int loc_1277;
				 * int loc_12D9;
				 * int loc_12DD;
				 * 
				 * int loc_11E7;
				 * int loc_127A;
				 * int loc_1203;
				 * int loc_122B;
				 * int loc_117F;
				 * int loc_11EF;
				 * int loc_1217;
				 * int loc_12E1;
				 * 
				 * int tempSwap;
				 * 
				 * int cx;
				 * int bp;
				 * int bpSave;
				 * 
				 * char* diPtr;
				 * char* siPtr;
				 * 
				 * int direction = 1;
				 * int dx = height;
				 * int ax = width;
				 * int es = ax << 1;
				 * int bx = 0;
				 * int di = 199;
				 * int si;
				 * 
				 * ax = y;
				 * si = 0;
				 * 
				 * if(y>199) // out of screen vertically
				 * return;
				 * 
				 * if(y<0) // cropped on the top
				 * {
				 * cx = bx;
				 * bx -= ax;
				 * dx -= bx;
				 * if(dx <= 0)
				 * {
				 * return;
				 * }
				 * ax = es; // es = size of a line ?
				 * ax*=(bx&0xFF); // bx number of lines to skip vertically
				 * si+=ax;
				 * ax = cx;
				 * }
				 * 
				 * bx = ax;
				 * ax += dx;
				 * ax--;
				 * 
				 * if(ax > di)
				 * {
				 * ax -= di;
				 * dx -= ax;
				 * 
				 * if(dx <= 0)
				 * {
				 * return;
				 * }
				 * }
				 * 
				 * ax = dx;
				 * loc_10DF = ax;
				 * ax = bx;
				 * loc_10AD = ax;
				 * 
				 * bx = 0;
				 * di = 319;
				 * 
				 * ax = x;
				 * dx = ax;
				 * cx = ax&0xFF;
				 * cx &= 7;
				 * {
				 * int cl = cx;
				 * int ch = cl;
				 * 
				 * cl-=8;
				 * cl=-cl;
				 * cl&=7;
				 * ax = (ch<<8) | (cl);
				 * }
				 * loc_1064 = ax;
				 * ax = es;
				 * ax <<= 3;
				 * 
				 * tempSwap = dx;
				 * dx = ax;
				 * ax = tempSwap;
				 * 
				 * if(ax > di)
				 * {
				 * return;
				 * }
				 * 
				 * cx = ax;
				 * cx += dx;
				 * cx --;
				 * 
				 * dx >>= 3;
				 * 
				 * dx = dx&0xFF;
				 * 
				 * if(cx<bx)
				 * {
				 * return;
				 * }
				 * 
				 * if(cx>di)
				 * {
				 * cx -= di;
				 * cx >>= 3;
				 * dx = (dx&0xFF00) | (((dx&0xFF) - (cx&0xFF))&0xFF);
				 * dx = ((cx&0xFF)<<8) | (dx&0xFF);
				 * di = 0xF8F9;
				 * }
				 * else
				 * {
				 * di = 0xF8F8;
				 * }
				 * 
				 * if(ax<bx)
				 * {
				 * ax -= bx;
				 * ax = -ax;
				 * ax >>= 3;
				 * 
				 * si += ax;
				 * dx = (dx&0xFF00) | (((dx&0xFF)-(ax&0xFF))&0xFF);
				 * dx = (((dx&0xFF00) + ((ax&0xFF)<<8))&0xFF00) | (dx&0xFF);
				 * ax = bx;
				 * cx = di;
				 * cx = (248<<8)|(cx&0xFF);
				 * di = cx;
				 * }
				 * 
				 * loc_10AA = ax;
				 * ax = (ax&0xFF00) | (((dx&0xFF00)>>8)&0xFF);
				 * ax = ax&0xFF;
				 * loc_10C5 = ax;
				 * ax = (ax&0xFF00) | (dx&0xFF);
				 * 
				 * dx = loc_1064;
				 * 
				 * if(dx)
				 * {
				 * if(di&1)
				 * {
				 * loc_10C5++;
				 * }
				 * 
				 * bx = ax;
				 * ax--;
				 * loc_11DC = ax;
				 * 
				 * if(di&0x100)
				 * {
				 * bx--;
				 * }
				 * 
				 * ax = bx;
				 * ax -= 40;
				 * ax = -ax;
				 * loc_12D9 = ax;
				 * ax = di;
				 * loc_1277 = ax&0xFF;
				 * ax = (ax&0xFF00) | (((ax&0xFF00)>>8)&0xFF);
				 * loc_117C = ax&0xFF;
				 * }
				 * else
				 * {
				 * loc_10EC = ax;
				 * ax -= 40;
				 * ax = -ax;
				 * loc_1147 = ax;
				 * }
				 * 
				 * bx = loc_10AA;
				 * ax = loc_10AD;
				 * bx = ((((((bx&0xFF00)>>8)&0xFF) + (ax&0xFF))<<8)&0xFF00) | (bx&0xFF);
				 * 
				 * bx>>=3;
				 * ax<<=3;
				 * 
				 * bx+=ax;
				 * 
				 * diPtr = destPtr;
				 * diPtr += bx;
				 * 
				 * ax = loc_10C5;
				 * ax<<=2;
				 * loc_114B = ax;
				 * loc_12DD = ax;
				 * ax = si;
				 * ax <<=2;
				 * siPtr = sourcePtr;
				 * 
				 * siPtr+=ax;
				 * 
				 * bp = loc_10DF;
				 * bx = dx;
				 * dx = 974;
				 * 
				 * if(!bx) // no crop ?
				 * {
				 * do // for each line
				 * {
				 * bpSave = bp;
				 * cx = loc_10EC;
				 * 
				 * do // for the line
				 * {
				 * outputBit(diPtr,0,*(siPtr));
				 * outputBit(diPtr,1,*(siPtr+1));
				 * outputBit(diPtr,2,*(siPtr+2));
				 * outputBit(diPtr,3,*(siPtr+3));
				 * 
				 * siPtr+=4;
				 * diPtr++;
				 * }while(--cx);
				 * 
				 * diPtr += loc_1147; // interline
				 * siPtr += loc_114B;
				 * bp = bpSave;
				 * }while(--bp);
				 * }
				 * else // croped
				 * {
				 * ASSERT(0);
				 * loc_1156:
				 * ax = (ax&0xFF00) | bx&0xFF;
				 * loc_11E7 = ax&0xFF;
				 * loc_127A = ax&0xFF;
				 * loc_1203 = ax&0xFF;
				 * loc_122B = ax&0xFF; 
				 * 
				 * ax = (ax&0xFF00) | (((bx&0xFF00)>>8)&0xFF);
				 * loc_117F = ax&0xFF;
				 * loc_11EF = ax&0xFF;
				 * loc_1217 = ax&0xFF;
				 * 
				 * do // main copy loop
				 * {
				 * ax = bp;
				 * loc_12E1 = ax;
				 * 
				 * if(loc_117C == 0xF8)
				 * {
				 * direction = 1;
				 * }
				 * else
				 * {
				 * direction = -1;
				 * }
				 * 
				 * if(direction == -1)
				 * {
				 * goto label_11DC;
				 * }
				 * 
				 * cx = loc_117F;
				 * 
				 * ax = (ax&0xFF00) | (((*siPtr)&0xFF)>>cx)&0xFF;
				 * dx = (((ax&0xFF)<<8)&0xFF00) | (dx&0xFF);
				 * ax = (((ax&0xFF)<<8)&0xFF00) | (ax&0xFF);
				 * 
				 * ax = (ax&0xFF00) | (((*(siPtr+1))&0xFF)>>cx)&0xFF;
				 * dx = (dx&0xFF00) | (ax&0xFF);
				 * ax = ((((((ax&0xFF00)>>8)&0xFF) | (ax&0xFF))<<8)&0xFF00) | (ax&0xFF);
				 * 
				 * ax = (ax&0xFF00) | (((*(siPtr+2))&0xFF)>>cx)&0xFF;
				 * bx = (((ax&0xFF)<<8)&0xFF00) | (bx&0xFF);
				 * ax = ((((((ax&0xFF00)>>8)&0xFF) | (ax&0xFF))<<8)&0xFF00) | (ax&0xFF);
				 * 
				 * ax = (ax&0xFF00) | (((*(siPtr+3))&0xFF)>>cx)&0xFF;
				 * bx = (bx&0xFF00) | (ax&0xFF);
				 * ax = ((((((ax&0xFF00)>>8)&0xFF) | (ax&0xFF))<<8)&0xFF00) | (ax&0xFF);
				 * 
				 * if(ax)
				 * {
				 * bp = dx;
				 * ax = (ax&0xFF00) | (*diPtr)&0xFF;
				 * ax = (ax&0xFF00) | 8;
				 * 
				 * outputBit(diPtr,0,(bp>>8)&0xFF);
				 * outputBit(diPtr,1,(bp&0xFF));
				 * outputBit(diPtr,2,(bx>>8)&0xFF);
				 * outputBit(diPtr,3,(bx&0xFF));
				 * }
				 * 
				 * diPtr++;
				 * 
				 * label_11DC:
				 * 
				 * bp = loc_11DC;
				 * if(bp >0)
				 * {
				 * do
				 * {
				 * cx = loc_11E7;
				 * 
				 * ax = (ax&0xFF00) | (((*siPtr)&0xFF)>>cx)&0xFF;
				 * dx = (((ax&0xFF)<<8)&0xFF00) | (dx&0xFF);
				 * cx = loc_11EF;
				 * ax = (ax&0xFF00) | (((*(siPtr+4))&0xFF)>>cx)&0xFF;
				 * dx = (((dx&0xFF00) | (((ax&0xFF)<<8)&0xFF00))&0xFF00) | (dx&0xFF);
				 * ax = (ax&0xFF00) | (((ax&0xFF) | (((dx&0xFF00)>>8)&0xFF))&0xFF);
				 * ax = ((ax&0xFF)<<8) | (ax&0xFF);
				 * 
				 * ax = (ax&0xFF00) | (((*(siPtr+5))&0xFF)>>cx)&0xFF;
				 * dx = (dx&0xFF00) | (ax&0xFF);
				 * cx = loc_1203;
				 * ax = (ax&0xFF00) | (((*(siPtr+1))&0xFF)>>cx)&0xFF;
				 * dx = (dx&0xFF00) | ((dx&0xFF) | (ax&0xFF));
				 * ax = (ax&0xFF00) | ((ax&0xFF) | dx&0xFF);
				 * ax = (ax&0xFF00) | ((ax&0xFF)<<8) | (ax&0xFF);
				 * 
				 * ax = (ax&0xFF00) | (((*(siPtr+2))&0xFF)>>cx)&0xFF;
				 * bx = (((ax&0xFF)<<8)&0xFF00) | (bx&0xFF);
				 * cx = loc_1217;
				 * ax = (ax&0xFF00) | (((*(siPtr+7))&0xFF)>>cx)&0xFF;
				 * bx = (((bx&0xFF00) | (((ax&0xFF)<<8)&0xFF00))&0xFF00) | (bx&0xFF);
				 * ax = (ax&0xFF00) | ((ax&0xFF) | ((bx&0xFF00)>>8));
				 * ax = (ax&0xFF00) | ((ax&0xFF)<<8) | (ax&0xFF);
				 * 
				 * ax = (ax&0xFF00) | (((*(siPtr+7))&0xFF)>>cx)&0xFF;
				 * bx = (bx&0xFF00) | (ax&0xFF);
				 * cx = loc_122B;
				 * ax = (ax&0xFF00) | (((*(siPtr+3))&0xFF)>>cx)&0xFF;
				 * bx = (bx&0xFF00) | ((bx&0xFF) | (ax&0xFF));
				 * ax = (ax&0xFF00) | ((ax&0xFF) | bx&0xFF);
				 * ax = (ax&0xFF00) | ((ax&0xFF)<<8) | (ax&0xFF);
				 * 
				 * if(ax)
				 * {
				 * cx = dx;
				 * ax = (ax&0xFF00) | (*diPtr)&0xFF;
				 * ax = (ax&0xFF00) | 8;
				 * 
				 * outputBit(diPtr,0,(cx>>8)&0xFF);
				 * outputBit(diPtr,1,(cx&0xFF));
				 * outputBit(diPtr,2,(cx>>8)&0xFF);
				 * outputBit(diPtr,3,(cx&0xFF));
				 * }
				 * 
				 * siPtr += 4;
				 * diPtr++;
				 * }while(--bp);
				 * }
				 * 
				 * if(loc_122B == 0xF8)
				 * {
				 * direction = 1;
				 * }
				 * else
				 * {
				 * direction = -1;
				 * }
				 * 
				 * if(direction == -1)
				 * {
				 * goto label_12D9;
				 * }
				 * 
				 * cx = loc_127A;
				 * 
				 * ax = (ax&0xFF00) | (((*siPtr)&0xFF)>>cx)&0xFF;
				 * dx = (((ax&0xFF)<<8)&0xFF00) | (dx&0xFF);
				 * ax = (((ax&0xFF)<<8)&0xFF00) | (ax&0xFF);
				 * 
				 * ax = (ax&0xFF00) | (((*(siPtr+1))&0xFF)>>cx)&0xFF;
				 * dx = (dx&0xFF00) | (ax&0xFF);
				 * ax = ((((((ax&0xFF00)>>8)&0xFF) | (ax&0xFF))<<8)&0xFF00) | (ax&0xFF);
				 * 
				 * ax = (ax&0xFF00) | (((*(siPtr+2))&0xFF)>>cx)&0xFF;
				 * bx = (((ax&0xFF)<<8)&0xFF00) | (bx&0xFF);
				 * ax = ((((((ax&0xFF00)>>8)&0xFF) | (ax&0xFF))<<8)&0xFF00) | (ax&0xFF);
				 * 
				 * ax = (ax&0xFF00) | (((*(siPtr+3))&0xFF)>>cx)&0xFF;
				 * bx = (bx&0xFF00) | (ax&0xFF);
				 * ax = ((((((ax&0xFF00)>>8)&0xFF) | (ax&0xFF))<<8)&0xFF00) | (ax&0xFF);
				 * 
				 * if(ax)
				 * {
				 * bp = dx;
				 * ax = (ax&0xFF00) | (*diPtr)&0xFF;
				 * ax = (ax&0xFF00) | 8;
				 * 
				 * outputBit(diPtr,0,(bp>>8)&0xFF);
				 * outputBit(diPtr,1,(bp&0xFF));
				 * outputBit(diPtr,2,(bx>>8)&0xFF);
				 * outputBit(diPtr,3,(bx&0xFF));
				 * }
				 * 
				 * siPtr+=4;
				 * 
				 * label_12D9:
				 * diPtr+=loc_12D9;
				 * siPtr+=loc_12DD;
				 * bp = loc_12E1;
				 * 
				 * }while(--bp);
				 * } */

	{
		int cols = 320;
		int rows = 200;
		int row;
		int col;
		int i;
		uint8 *pP;
		short atariScreen[320 * 200 / 4];

		for (i = 0; i < rows * cols / 4; i++) {
			atariScreen[i] = *(int16 *) sourcePtr;
			flipShort(&atariScreen[i]);
			sourcePtr += 2;
		}

		memcpy(globalAtariScreen, atariScreen, sizeof(atariScreen));

		pP = (uint8 *) destPtr;

		for (row = 0; row < rows; ++row) {
			for (col = 0; col < cols; ++col, ++pP) {
				long int c, ind, b, plane;

				ind = 80 * row + ((col >> 4) << 2);
				b = 0x8000 >> (col & 0xf);
				c = 0;

				for (plane = 0; plane < 4; ++plane) {
					if (b & atariScreen[ind + plane]) {
						c |= (1 << plane);
					}
				}

				*pP = (uint8) c;
			}
		}
	}
/*
	{
		int i;
		int j;

		for(i=x;i<height+x;i++)
		{
			for(j=y;j<width*16+y;j++)
			{
				if(i>=0&&i<200&&j>=0&&j<320)
					destPtr[i*320+j] = *(sourcePtr++);
			}
		}
	}*/
}

void gfxModuleData_setPal256(int16 *ptr) {
	int R;
	int G;
	int B;
	int i;

	for (i = 0; i < 256; i++) {
		R = *(ptr++);
		G = *(ptr++);
		B = *(ptr++);

		lpalette[i].R = R;
		lpalette[i].G = G;
		lpalette[i].B = B;
		lpalette[i].A = 255;
	}
}

void gfxModuleData_setPal(uint8 *ptr) {
	int i;
	int R;
	int G;
	int B;

	for (i = 0; i < 16; i++) {
#define convertRatio 36.571428571428571428571428571429
		short int atariColor = *(int16 *) ptr;
		//flipShort(&atariColor);
		ptr += 2;

		R = (int)(convertRatio * ((atariColor & 0x700) >> 8));
		G = (int)(convertRatio * ((atariColor & 0x070) >> 4));
		B = (int)(convertRatio * ((atariColor & 0x007)));

		if (R > 0xFF)
			R = 0xFF;
		if (G > 0xFF)
			G = 0xFF;
		if (B > 0xFF)
			B = 0xFF;

		lpalette[i].R = R;
		lpalette[i].G = G;
		lpalette[i].B = B;
		lpalette[i].A = 255;

	}
}

void gfxModuleData_field_90(void) {
}

void gfxModuleData_gfxWaitVSync(void) {
}

void gfxModuleData_flip(void) {
}

void gfxModuleData_field_64(char *sourceBuffer, int width, int height,
	    char *dest, int x, int y, int color) {
	int i;
	int j;

	x = 0;
	y = 0;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			dest[(y + i) * 320 / 4 + x + j] =
			    sourceBuffer[i * width + j];
		}
	}
}

void gfxModuleData_flipScreen(void) {
	memcpy(globalScreen, gfxModuleData.pPage00, 320 * 200);

	flip();
}

void flip() {
	int i;
	byte paletteRGBA[256 * 4];
	//uint8* outPtr = scaledScreen;
	//uint8* inPtr  = globalScreen;

	for (i = 0; i < 256; i++) {
		paletteRGBA[i * 4 + 0] = lpalette[i].R;
		paletteRGBA[i * 4 + 1] = lpalette[i].G;
		paletteRGBA[i * 4 + 2] = lpalette[i].B;
		paletteRGBA[i * 4 + 3] = 0xFF;
	}
	g_system->setPalette(paletteRGBA, 0, 16);

	g_system->copyRectToScreen(globalScreen, 320, 0, 0, 320, 200);
	g_system->updateScreen();

}

} // End of namespace Cruise
