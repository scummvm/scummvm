/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "common/scaler/intern.h"

#define PIXEL00_0   *(q) = w[5];
#define PIXEL00_10  *(q) = interpolate16_2<bitFormat,3,1>(w[5], w[1]);
#define PIXEL00_11  *(q) = interpolate16_2<bitFormat,3,1>(w[5], w[4]);
#define PIXEL00_12  *(q) = interpolate16_2<bitFormat,3,1>(w[5], w[2]);
#define PIXEL00_20  *(q) = interpolate16_3<bitFormat,2,1,1>(w[5], w[4], w[2]);
#define PIXEL00_30  *(q) = interpolate16_3<bitFormat,6,1,1>(w[5], w[4], w[2]);
#define PIXEL00_40  *(q) = interpolate16_3<bitFormat,14,1,1>(w[5], w[4], w[2]);
#define PIXEL00_50  *(q) = interpolate16_3<bitFormat,5,2,1>(w[5], w[2], w[4]);
#define PIXEL00_51  *(q) = interpolate16_3<bitFormat,5,2,1>(w[5], w[4], w[2]);
#define PIXEL00_60  *(q) = interpolate16_3<bitFormat,2,3,3>(w[5], w[4], w[2]);

#define PIXEL01_0   *(q+1) = w[5];
#define PIXEL01_10  *(q+1) = interpolate16_2<bitFormat,3,1>(w[5], w[3]);
#define PIXEL01_11  *(q+1) = interpolate16_2<bitFormat,3,1>(w[5], w[2]);
#define PIXEL01_12  *(q+1) = interpolate16_2<bitFormat,3,1>(w[5], w[6]);
#define PIXEL01_20  *(q+1) = interpolate16_3<bitFormat,2,1,1>(w[5], w[2], w[6]);
#define PIXEL01_30  *(q+1) = interpolate16_3<bitFormat,6,1,1>(w[5], w[2], w[6]);
#define PIXEL01_40  *(q+1) = interpolate16_3<bitFormat,14,1,1>(w[5], w[2], w[6]);
#define PIXEL01_50  *(q+1) = interpolate16_3<bitFormat,5,2,1>(w[5], w[6], w[2]);
#define PIXEL01_51  *(q+1) = interpolate16_3<bitFormat,5,2,1>(w[5], w[2], w[6]);
#define PIXEL01_60  *(q+1) = interpolate16_3<bitFormat,2,3,3>(w[5], w[2], w[6]);

#define PIXEL10_0   *(q+nextlineDst) = w[5];
#define PIXEL10_10  *(q+nextlineDst) = interpolate16_2<bitFormat,3,1>(w[5], w[7]);
#define PIXEL10_11  *(q+nextlineDst) = interpolate16_2<bitFormat,3,1>(w[5], w[8]);
#define PIXEL10_12  *(q+nextlineDst) = interpolate16_2<bitFormat,3,1>(w[5], w[4]);
#define PIXEL10_20  *(q+nextlineDst) = interpolate16_3<bitFormat,2,1,1>(w[5], w[8], w[4]);
#define PIXEL10_30  *(q+nextlineDst) = interpolate16_3<bitFormat,6,1,1>(w[5], w[8], w[4]);
#define PIXEL10_40  *(q+nextlineDst) = interpolate16_3<bitFormat,14,1,1>(w[5], w[8], w[4]);
#define PIXEL10_50  *(q+nextlineDst) = interpolate16_3<bitFormat,5,2,1>(w[5], w[4], w[8]);
#define PIXEL10_51  *(q+nextlineDst) = interpolate16_3<bitFormat,5,2,1>(w[5], w[8], w[4]);
#define PIXEL10_60  *(q+nextlineDst) = interpolate16_3<bitFormat,2,3,3>(w[5], w[8], w[4]);

#define PIXEL11_0   *(q+1+nextlineDst) = w[5];
#define PIXEL11_10  *(q+1+nextlineDst) = interpolate16_2<bitFormat,3,1>(w[5], w[9]);
#define PIXEL11_11  *(q+1+nextlineDst) = interpolate16_2<bitFormat,3,1>(w[5], w[6]);
#define PIXEL11_12  *(q+1+nextlineDst) = interpolate16_2<bitFormat,3,1>(w[5], w[8]);
#define PIXEL11_20  *(q+1+nextlineDst) = interpolate16_3<bitFormat,2,1,1>(w[5], w[6], w[8]);
#define PIXEL11_30  *(q+1+nextlineDst) = interpolate16_3<bitFormat,6,1,1>(w[5], w[6], w[8]);
#define PIXEL11_40  *(q+1+nextlineDst) = interpolate16_3<bitFormat,14,1,1>(w[5], w[6], w[8]);
#define PIXEL11_50  *(q+1+nextlineDst) = interpolate16_3<bitFormat,5,2,1>(w[5], w[8], w[6]);
#define PIXEL11_51  *(q+1+nextlineDst) = interpolate16_3<bitFormat,5,2,1>(w[5], w[6], w[8]);
#define PIXEL11_60  *(q+1+nextlineDst) = interpolate16_3<bitFormat,2,3,3>(w[5], w[6], w[8]);

/**
 * The HQ2x high quality 2x graphics filter.
 * Original author Maxim Stepin (see http://www.hiend3d.com/hq2x.html).
 * Adapted for ScummVM to 16 bit output and optimized by Max Horn.
 */
template<int bitFormat>
void HQ2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	int  w[10];
	int  yuv[10];
  
	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;
	
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w1 | w2 | w3 |
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w4 | w5 | w6 |
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w7 | w8 | w9 |
	//	 +----+----+----+

	while (height--) {
		w[2] = *(p - 1 - nextlineSrc);  yuv[2] = RGBtoYUV[w[2]];
		w[5] = *(p - 1);                yuv[5] = RGBtoYUV[w[5]];
		w[8] = *(p - 1 + nextlineSrc);  yuv[8] = RGBtoYUV[w[8]];

		w[3] = *(p - nextlineSrc);      yuv[3] = RGBtoYUV[w[3]];
		w[6] = *(p);                    yuv[6] = RGBtoYUV[w[6]];
		w[9] = *(p + nextlineSrc);      yuv[9] = RGBtoYUV[w[9]];

		int tmpWidth = width;
		while (tmpWidth--) {
			p++;

			w[1] = w[2];                yuv[1] = yuv[2];
			w[4] = w[5];                yuv[4] = yuv[5];
			w[7] = w[8];                yuv[7] = yuv[8];

			w[2] = w[3];                yuv[2] = yuv[3];
			w[5] = w[6];                yuv[5] = yuv[6];
			w[8] = w[9];                yuv[8] = yuv[9];

			w[3] = *(p - nextlineSrc);	yuv[3] = RGBtoYUV[w[3]];
			w[6] = *(p);				yuv[6] = RGBtoYUV[w[6]];
			w[9] = *(p + nextlineSrc);	yuv[9] = RGBtoYUV[w[9]];

			int pattern = 0;
			int flag = 1;

			for (int k = 1; k <= 9; k++) {
				if (k == 5) continue;

				if (w[k] != w[5]) {
					if (diffYUV(yuv[5], yuv[k]))
						pattern |= flag;
				}
				flag <<= 1;
			}

			switch (pattern) {
			case 0:
			case 1:
			case 4:
			case 32:
			case 128:
			case 5:
			case 132:
			case 160:
			case 33:
			case 129:
			case 36:
			case 133:
			case 164:
			case 161:
			case 37:
			case 165:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_20
				PIXEL11_20
				break;
			case 2:
			case 34:
			case 130:
			case 162:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_20
				PIXEL11_20
				break;
			case 16:
			case 17:
			case 48:
			case 49:
				PIXEL00_20
				PIXEL01_10
				PIXEL10_20
				PIXEL11_10
				break;
			case 64:
			case 65:
			case 68:
			case 69:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_10
				PIXEL11_10
				break;
			case 8:
			case 12:
			case 136:
			case 140:
				PIXEL00_10
				PIXEL01_20
				PIXEL10_10
				PIXEL11_20
				break;
			case 3:
			case 35:
			case 131:
			case 163:
				PIXEL00_11
				PIXEL01_10
				PIXEL10_20
				PIXEL11_20
				break;
			case 6:
			case 38:
			case 134:
			case 166:
				PIXEL00_10
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
			case 20:
			case 21:
			case 52:
			case 53:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_10
				break;
			case 144:
			case 145:
			case 176:
			case 177:
				PIXEL00_20
				PIXEL01_10
				PIXEL10_20
				PIXEL11_12
				break;
			case 192:
			case 193:
			case 196:
			case 197:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_10
				PIXEL11_11
				break;
			case 96:
			case 97:
			case 100:
			case 101:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_10
				break;
			case 40:
			case 44:
			case 168:
			case 172:
				PIXEL00_10
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
			case 9:
			case 13:
			case 137:
			case 141:
				PIXEL00_12
				PIXEL01_20
				PIXEL10_10
				PIXEL11_20
				break;
			case 18:
			case 50:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_20
				}
				PIXEL10_20
				PIXEL11_10
				break;
			case 80:
			case 81:
				PIXEL00_20
				PIXEL01_10
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_20
				}
				break;
			case 72:
			case 76:
				PIXEL00_10
				PIXEL01_20
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 10:
			case 138:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				PIXEL11_20
				break;
			case 66:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_10
				PIXEL11_10
				break;
			case 24:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_10
				PIXEL11_10
				break;
			case 7:
			case 39:
			case 135:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
			case 148:
			case 149:
			case 180:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_12
				break;
			case 224:
			case 228:
			case 225:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_11
				break;
			case 41:
			case 169:
			case 45:
				PIXEL00_12
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
			case 22:
			case 54:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_20
				PIXEL11_10
				break;
			case 208:
			case 209:
				PIXEL00_20
				PIXEL01_10
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 104:
			case 108:
				PIXEL00_10
				PIXEL01_20
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 11:
			case 139:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				PIXEL11_20
				break;
			case 19:
			case 51:
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL00_11
					PIXEL01_10
				} else {
					PIXEL00_50
					PIXEL01_60
				}
				PIXEL10_20
				PIXEL11_10
				break;
			case 146:
			case 178:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
					PIXEL11_12
				} else {
					PIXEL01_60
					PIXEL11_51
				}
				PIXEL10_20
				break;
			case 84:
			case 85:
				PIXEL00_20
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL01_11
					PIXEL11_10
				} else {
					PIXEL01_50
					PIXEL11_60
				}
				PIXEL10_10
				break;
			case 112:
			case 113:
				PIXEL00_20
				PIXEL01_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL10_12
					PIXEL11_10
				} else {
					PIXEL10_51
					PIXEL11_60
				}
				break;
			case 200:
			case 204:
				PIXEL00_10
				PIXEL01_20
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
					PIXEL11_11
				} else {
					PIXEL10_60
					PIXEL11_50
				}
				break;
			case 73:
			case 77:
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL00_12
					PIXEL10_10
				} else {
					PIXEL00_51
					PIXEL10_60
				}
				PIXEL01_20
				PIXEL11_10
				break;
			case 42:
			case 170:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
					PIXEL10_11
				} else {
					PIXEL00_60
					PIXEL10_50
				}
				PIXEL01_10
				PIXEL11_20
				break;
			case 14:
			case 142:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
					PIXEL01_12
				} else {
					PIXEL00_60
					PIXEL01_51
				}
				PIXEL10_10
				PIXEL11_20
				break;
			case 67:
				PIXEL00_11
				PIXEL01_10
				PIXEL10_10
				PIXEL11_10
				break;
			case 70:
				PIXEL00_10
				PIXEL01_12
				PIXEL10_10
				PIXEL11_10
				break;
			case 28:
				PIXEL00_10
				PIXEL01_11
				PIXEL10_10
				PIXEL11_10
				break;
			case 152:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_10
				PIXEL11_12
				break;
			case 194:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_10
				PIXEL11_11
				break;
			case 98:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_12
				PIXEL11_10
				break;
			case 56:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_11
				PIXEL11_10
				break;
			case 25:
				PIXEL00_12
				PIXEL01_10
				PIXEL10_10
				PIXEL11_10
				break;
			case 26:
			case 31:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				PIXEL11_10
				break;
			case 82:
			case 214:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 88:
			case 248:
				PIXEL00_10
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 74:
			case 107:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 27:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				PIXEL11_10
				break;
			case 86:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				PIXEL11_10
				break;
			case 216:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 106:
				PIXEL00_10
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 30:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				PIXEL11_10
				break;
			case 210:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 120:
				PIXEL00_10
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 75:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				PIXEL11_10
				break;
			case 29:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_10
				PIXEL11_10
				break;
			case 198:
				PIXEL00_10
				PIXEL01_12
				PIXEL10_10
				PIXEL11_11
				break;
			case 184:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_11
				PIXEL11_12
				break;
			case 99:
				PIXEL00_11
				PIXEL01_10
				PIXEL10_12
				PIXEL11_10
				break;
			case 57:
				PIXEL00_12
				PIXEL01_10
				PIXEL10_11
				PIXEL11_10
				break;
			case 71:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_10
				PIXEL11_10
				break;
			case 156:
				PIXEL00_10
				PIXEL01_11
				PIXEL10_10
				PIXEL11_12
				break;
			case 226:
				PIXEL00_10
				PIXEL01_10
				PIXEL10_12
				PIXEL11_11
				break;
			case 60:
				PIXEL00_10
				PIXEL01_11
				PIXEL10_11
				PIXEL11_10
				break;
			case 195:
				PIXEL00_11
				PIXEL01_10
				PIXEL10_10
				PIXEL11_11
				break;
			case 102:
				PIXEL00_10
				PIXEL01_12
				PIXEL10_12
				PIXEL11_10
				break;
			case 153:
				PIXEL00_12
				PIXEL01_10
				PIXEL10_10
				PIXEL11_12
				break;
			case 58:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				PIXEL10_11
				PIXEL11_10
				break;
			case 83:
				PIXEL00_11
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 92:
				PIXEL00_10
				PIXEL01_11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 202:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				PIXEL11_11
				break;
			case 78:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				PIXEL01_12
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				PIXEL11_10
				break;
			case 154:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				PIXEL10_10
				PIXEL11_12
				break;
			case 114:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				PIXEL10_12
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 89:
				PIXEL00_12
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 90:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 55:
			case 23:
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL00_11
					PIXEL01_0
				} else {
					PIXEL00_50
					PIXEL01_60
				}
				PIXEL10_20
				PIXEL11_10
				break;
			case 182:
			case 150:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
					PIXEL11_12
				} else {
					PIXEL01_60
					PIXEL11_51
				}
				PIXEL10_20
				break;
			case 213:
			case 212:
				PIXEL00_20
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL01_11
					PIXEL11_0
				} else {
					PIXEL01_50
					PIXEL11_60
				}
				PIXEL10_10
				break;
			case 241:
			case 240:
				PIXEL00_20
				PIXEL01_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL10_12
					PIXEL11_0
				} else {
					PIXEL10_51
					PIXEL11_60
				}
				break;
			case 236:
			case 232:
				PIXEL00_10
				PIXEL01_20
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
					PIXEL11_11
				} else {
					PIXEL10_60
					PIXEL11_50
				}
				break;
			case 109:
			case 105:
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL00_12
					PIXEL10_0
				} else {
					PIXEL00_51
					PIXEL10_60
				}
				PIXEL01_20
				PIXEL11_10
				break;
			case 171:
			case 43:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
					PIXEL10_11
				} else {
					PIXEL00_60
					PIXEL10_50
				}
				PIXEL01_10
				PIXEL11_20
				break;
			case 143:
			case 15:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
					PIXEL01_12
				} else {
					PIXEL00_60
					PIXEL01_51
				}
				PIXEL10_10
				PIXEL11_20
				break;
			case 124:
				PIXEL00_10
				PIXEL01_11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 203:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				PIXEL11_11
				break;
			case 62:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_11
				PIXEL11_10
				break;
			case 211:
				PIXEL00_11
				PIXEL01_10
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 118:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_12
				PIXEL11_10
				break;
			case 217:
				PIXEL00_12
				PIXEL01_10
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 110:
				PIXEL00_10
				PIXEL01_12
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 155:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				PIXEL11_12
				break;
			case 188:
				PIXEL00_10
				PIXEL01_11
				PIXEL10_11
				PIXEL11_12
				break;
			case 185:
				PIXEL00_12
				PIXEL01_10
				PIXEL10_11
				PIXEL11_12
				break;
			case 61:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_11
				PIXEL11_10
				break;
			case 157:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_10
				PIXEL11_12
				break;
			case 103:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_12
				PIXEL11_10
				break;
			case 227:
				PIXEL00_11
				PIXEL01_10
				PIXEL10_12
				PIXEL11_11
				break;
			case 230:
				PIXEL00_10
				PIXEL01_12
				PIXEL10_12
				PIXEL11_11
				break;
			case 199:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_10
				PIXEL11_11
				break;
			case 220:
				PIXEL00_10
				PIXEL01_11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 158:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				PIXEL11_12
				break;
			case 234:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_11
				break;
			case 242:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				PIXEL10_12
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 59:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				PIXEL10_11
				PIXEL11_10
				break;
			case 121:
				PIXEL00_12
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 87:
				PIXEL00_11
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 79:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_12
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				PIXEL11_10
				break;
			case 122:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 94:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 218:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 91:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 229:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_11
				break;
			case 167:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
			case 173:
				PIXEL00_12
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
			case 181:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_12
				break;
			case 186:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				PIXEL10_11
				PIXEL11_12
				break;
			case 115:
				PIXEL00_11
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				PIXEL10_12
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 93:
				PIXEL00_12
				PIXEL01_11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 206:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				PIXEL01_12
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				PIXEL11_11
				break;
			case 205:
			case 201:
				PIXEL00_12
				PIXEL01_20
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_10
				} else {
					PIXEL10_30
				}
				PIXEL11_11
				break;
			case 174:
			case 46:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_10
				} else {
					PIXEL00_30
				}
				PIXEL01_12
				PIXEL10_11
				PIXEL11_20
				break;
			case 179:
			case 147:
				PIXEL00_11
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_10
				} else {
					PIXEL01_30
				}
				PIXEL10_20
				PIXEL11_12
				break;
			case 117:
			case 116:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_12
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_10
				} else {
					PIXEL11_30
				}
				break;
			case 189:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_11
				PIXEL11_12
				break;
			case 231:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_12
				PIXEL11_11
				break;
			case 126:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 219:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 125:
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL00_12
					PIXEL10_0
				} else {
					PIXEL00_51
					PIXEL10_60
				}
				PIXEL01_11
				PIXEL11_10
				break;
			case 221:
				PIXEL00_12
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL01_11
					PIXEL11_0
				} else {
					PIXEL01_50
					PIXEL11_60
				}
				PIXEL10_10
				break;
			case 207:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
					PIXEL01_12
				} else {
					PIXEL00_60
					PIXEL01_51
				}
				PIXEL10_10
				PIXEL11_11
				break;
			case 238:
				PIXEL00_10
				PIXEL01_12
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
					PIXEL11_11
				} else {
					PIXEL10_60
					PIXEL11_50
				}
				break;
			case 190:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
					PIXEL11_12
				} else {
					PIXEL01_60
					PIXEL11_51
				}
				PIXEL10_11
				break;
			case 187:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
					PIXEL10_11
				} else {
					PIXEL00_60
					PIXEL10_50
				}
				PIXEL01_10
				PIXEL11_12
				break;
			case 243:
				PIXEL00_11
				PIXEL01_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL10_12
					PIXEL11_0
				} else {
					PIXEL10_51
					PIXEL11_60
				}
				break;
			case 119:
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL00_11
					PIXEL01_0
				} else {
					PIXEL00_50
					PIXEL01_60
				}
				PIXEL10_12
				PIXEL11_10
				break;
			case 237:
			case 233:
				PIXEL00_12
				PIXEL01_20
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_40
				}
				PIXEL11_11
				break;
			case 175:
			case 47:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_40
				}
				PIXEL01_12
				PIXEL10_11
				PIXEL11_20
				break;
			case 183:
			case 151:
				PIXEL00_11
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_40
				}
				PIXEL10_20
				PIXEL11_12
				break;
			case 245:
			case 244:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_12
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_40
				}
				break;
			case 250:
				PIXEL00_10
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 123:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 95:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				PIXEL11_10
				break;
			case 222:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 252:
				PIXEL00_10
				PIXEL01_11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_40
				}
				break;
			case 249:
				PIXEL00_12
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_40
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 235:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_40
				}
				PIXEL11_11
				break;
			case 111:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_40
				}
				PIXEL01_12
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 63:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_40
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_11
				PIXEL11_10
				break;
			case 159:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_40
				}
				PIXEL10_10
				PIXEL11_12
				break;
			case 215:
				PIXEL00_11
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_40
				}
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 246:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_12
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_40
				}
				break;
			case 254:
				PIXEL00_10
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_40
				}
				break;
			case 253:
				PIXEL00_12
				PIXEL01_11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_40
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_40
				}
				break;
			case 251:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_40
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 239:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_40
				}
				PIXEL01_12
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_40
				}
				PIXEL11_11
				break;
			case 127:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_40
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 191:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_40
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_40
				}
				PIXEL10_11
				PIXEL11_12
				break;
			case 223:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_40
				}
				PIXEL10_10
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 247:
				PIXEL00_11
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_40
				}
				PIXEL10_12
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_40
				}
				break;
			case 255:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_0
				} else {
					PIXEL00_40
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_0
				} else {
					PIXEL01_40
				}
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_0
				} else {
					PIXEL10_40
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL11_0
				} else {
					PIXEL11_40
				}
				break;
			}
			q += 2;
		}
		p += nextlineSrc - width;
		q += (nextlineDst - width) * 2;
	}
}

MAKE_WRAPPER(HQ2x)
