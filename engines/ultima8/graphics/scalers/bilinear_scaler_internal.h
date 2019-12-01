/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */

#include "BilinearScaler.h"
#include "Manips.h"


namespace Pentagram {

#define SimpleLerp(a,b,fac) ((b<<8)+((a)-(b))*(fac))
#define SimpleLerp2(a,b,fac) ((b<<16)+((a)-(b))*(fac))

#define CopyLerp(d,a,b,f) { \
		(d)[0] = SimpleLerp2(b[0],a[0],f)>>16;\
		(d)[1] = SimpleLerp2(b[1],a[1],f)>>16;\
		(d)[2] = SimpleLerp2(b[2],a[2],f)>>16;\
		(d)[3] = SimpleLerp2(b[3],a[3],f)>>16;}

#define FilterPixel(a,b,f,g,fx,fy) { \
		*reinterpret_cast<uintX*>(pixel) = Manip::merge( \
		                                   SimpleLerp(SimpleLerp(a[0],f[0],fx),SimpleLerp(b[0],g[0],fx),fy)>>16,\
		                                   SimpleLerp(SimpleLerp(a[1],f[1],fx),SimpleLerp(b[1],g[1],fx),fy)>>16,\
		                                   SimpleLerp(SimpleLerp(a[2],f[2],fx),SimpleLerp(b[2],g[2],fx),fy)>>16,\
		                                   SimpleLerp(SimpleLerp(a[3],f[3],fx),SimpleLerp(b[3],g[3],fx),fy)>>16);}

#define ScalePixel2x(a,b,f,g) { \
		*(reinterpret_cast<uintX*>(pixel)) = Manip::merge(a[0], a[1], a[2], a[3]); \
		*(reinterpret_cast<uintX*>(pixel+sizeof(uintX))) = Manip::merge((a[0]+f[0])>>1, (a[1]+f[1])>>1, (a[2]+f[2])>>1, (a[3]+f[3])>>1); \
		pixel+=pitch; \
		*(reinterpret_cast<uintX*>(pixel)) = Manip::merge((a[0]+b[0])>>1, (a[1]+b[1])>>1, (a[2]+b[2])>>1, (a[3]+b[3])>>1);\
		*(reinterpret_cast<uintX*>(pixel+sizeof(uintX))) = Manip::merge((a[0]+b[0]+f[0]+g[0])>>2, (a[1]+b[1]+f[1]+g[1])>>2, (a[2]+b[2]+f[2]+g[2])>>2, (a[3]+b[3]+f[3]+g[3])>>2);\
		pixel+=pitch; } \

#define X2Xy24xLerps(c0,c1,y)   \
	*(reinterpret_cast<uintX*>(pixel)) = Manip::merge(                      \
	                                     cols[c0][y][0], cols[c0][y][1], cols[c0][y][2], cols[c0][y][3]);    \
	*(reinterpret_cast<uintX*>(pixel+sizeof(uintX))) = Manip::merge(        \
	        (cols[c0][y][0]+cols[c1][y][0])>>1,                                 \
	        (cols[c0][y][1]+cols[c1][y][1])>>1,                                 \
	        (cols[c0][y][2]+cols[c1][y][2])>>1,                                 \
	        (cols[c0][y][3]+cols[c1][y][3])>>1);

#define X2xY24xInnerLoop(c0,c1) {           \
		X2Xy24xLerps(c0,c1,0); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,1); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,2); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,3); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,4); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,5); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,6); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,7); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,8); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,9); pixel+=pitch;    \
		X2Xy24xLerps(c0,c1,10); pixel+=pitch;   \
		X2Xy24xLerps(c0,c1,11); pixel+=pitch;   }

#define X2xY24xDoColsA() {  \
		CopyLerp(cols[0][0],a,b, 0x0000);   \
		CopyLerp(cols[0][1],a,b, 0x6AAA);   \
		CopyLerp(cols[0][2],a,b, 0xD554);   \
		CopyLerp(cols[0][3],b,c, 0x3FFE);   \
		CopyLerp(cols[0][4],b,c, 0xAAA8);   \
		CopyLerp(cols[0][5],c,d, 0x1552);   \
		CopyLerp(cols[0][6],c,d, 0x7FFC);   \
		CopyLerp(cols[0][7],c,d, 0xEAA6);   \
		CopyLerp(cols[0][8],d,e, 0x5550);   \
		CopyLerp(cols[0][9],d,e, 0xBFFA);   \
		CopyLerp(cols[0][10],e,l, 0x2AA4);  \
		CopyLerp(cols[0][11],e,l, 0x954E);  }

#define X2xY24xDoColsB() {  \
		CopyLerp(cols[1][0],f,g, 0x0000);   \
		CopyLerp(cols[1][1],f,g, 0x6AAA);   \
		CopyLerp(cols[1][2],f,g, 0xD554);   \
		CopyLerp(cols[1][3],g,h, 0x3FFE);   \
		CopyLerp(cols[1][4],g,h, 0xAAA8);   \
		CopyLerp(cols[1][5],h,i, 0x1552);   \
		CopyLerp(cols[1][6],h,i, 0x7FFC);   \
		CopyLerp(cols[1][7],h,i, 0xEAA6);   \
		CopyLerp(cols[1][8],i,j, 0x5550);   \
		CopyLerp(cols[1][9],i,j, 0xBFFA);   \
		CopyLerp(cols[1][10],j,k, 0x2AA4);  \
		CopyLerp(cols[1][11],j,k, 0x954E); }

#define X1xY12xCopy(y)  \
	*(reinterpret_cast<uintX*>(pixel)) = Manip::merge(                      \
	                                     cols[y][0], cols[y][1], cols[y][2], cols[y][3]);

#define X1xY12xInnerLoop() {        \
		X1xY12xCopy(0); pixel+=pitch;   \
		X1xY12xCopy(1); pixel+=pitch;   \
		X1xY12xCopy(2); pixel+=pitch;   \
		X1xY12xCopy(3); pixel+=pitch;   \
		X1xY12xCopy(4); pixel+=pitch;   \
		X1xY12xCopy(5); pixel+=pitch;   }

#define X1xY12xDoCols() {   \
		CopyLerp(cols[0],a,b, 0x0000);  \
		CopyLerp(cols[1],a,b, 0xD554);  \
		CopyLerp(cols[2],b,c, 0xAAA8);  \
		CopyLerp(cols[3],c,d, 0x7FFC);  \
		CopyLerp(cols[4],d,e, 0x5550);  \
		CopyLerp(cols[5],e,l, 0x2AA4);  }

#define ArbInnerLoop(a,b,f,g) {     \
		if (pos_y < end_y) do {         \
				pos_x = dst_x;              \
				pixel = blockline_start;    \
				/* Dest Loop X */           \
				if (pos_x < end_x) do {     \
						FilterPixel(a,b,f,g,(end_x-pos_x)>>8,(end_y-pos_y)>>8);\
						pixel+=sizeof(uintX);   \
						pos_x += add_x;         \
					} while (pos_x < end_x);    \
				if (!next_block) next_block = pixel;    \
				blockline_start += pitch;   \
				pos_y += add_y;             \
			} while (pos_y < end_y);        \
		end_y += 1 << 16; }


#define Read5(a,b,c,d,e) {  \
		Manip::split(*(texel+tpitch*0), a[0], a[1], a[2], a[3]);\
		Manip::split(*(texel+tpitch*1), b[0], b[1], b[2], b[3]);\
		Manip::split(*(texel+tpitch*2), c[0], c[1], c[2], c[3]);\
		Manip::split(*(texel+tpitch*3), d[0], d[1], d[2], d[3]);\
		Manip::split(*(texel+tpitch*4), e[0], e[1], e[2], e[3]); }

#define Read5_Clipped(a,b,c,d,e) {  \
		Manip::split(*(texel+tpitch*0), a[0], a[1], a[2], a[3]);\
		Manip::split(*(texel+tpitch*1), b[0], b[1], b[2], b[3]);\
		Manip::split(*(texel+tpitch*2), c[0], c[1], c[2], c[3]);\
		Manip::split(*(texel+tpitch*3), d[0], d[1], d[2], d[3]);\
		e[0]=d[0]; e[1]=d[1], e[2]=d[2], e[3]=d[3]; }

#define Read6(a,b,c,d,e,l) {    \
		Manip::split(*(texel+tpitch*0), a[0], a[1], a[2], a[3]);\
		Manip::split(*(texel+tpitch*1), b[0], b[1], b[2], b[3]);\
		Manip::split(*(texel+tpitch*2), c[0], c[1], c[2], c[3]);\
		Manip::split(*(texel+tpitch*3), d[0], d[1], d[2], d[3]);\
		Manip::split(*(texel+tpitch*4), e[0], e[1], e[2], e[3]);\
		Manip::split(*(texel+tpitch*5), l[0], l[1], l[2], l[3]); }

#define Read6_Clipped(a,b,c,d,e,l) {    \
		Manip::split(*(texel+tpitch*0), a[0], a[1], a[2], a[3]);\
		Manip::split(*(texel+tpitch*1), b[0], b[1], b[2], b[3]);\
		Manip::split(*(texel+tpitch*2), c[0], c[1], c[2], c[3]);\
		Manip::split(*(texel+tpitch*3), d[0], d[1], d[2], d[3]);\
		Manip::split(*(texel+tpitch*4), e[0], e[1], e[2], e[3]);\
		l[0]=e[0]; l[1]=e[1]; l[2]=e[2]; l[3]=e[3]; }

// Very very simple point scaler
	template<class uintX, class Manip, class uintS>
	bool BilinearScalerInternal_2x(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                               uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src);

	template<class uintX, class Manip, class uintS>
	bool BilinearScalerInternal_X2Y24(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                                  uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src);

	template<class uintX, class Manip, class uintS>
	bool BilinearScalerInternal_X1Y12(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                                  uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src);

	template<class uintX, class Manip, class uintS>
	bool BilinearScalerInternal_Arb(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                                uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src);

#ifdef COMPILE_GAMMA_CORRECT_SCALERS
#define InstantiateFunc(func,a,b,c) \
	template bool func<a,b,c> (Texture*,int32,int32,int32,int32,uint8*,int32,int32,int32,bool); \
	template bool func<a,b##_GC,c> (Texture*,int32,int32,int32,int32,uint8*,int32,int32,int32,bool)
#else
#define InstantiateFunc(func,a,b,c) \
	template bool func<a,b,c> (Texture*,int32,int32,int32,int32,uint8*,int32,int32,int32,bool)
#endif

#ifdef COMPILE_ALL_BILINEAR_SCALERS
#define InstantiateBilinearScalerFunc(func) \
	InstantiateFunc(func,uint16,Manip_Nat2Nat_16,uint16); \
	InstantiateFunc(func,uint16,Manip_Sta2Nat_16,uint32); \
	InstantiateFunc(func,uint32,Manip_Nat2Nat_32,uint32); \
	InstantiateFunc(func,uint32,Manip_Sta2Nat_32,uint32); \
	InstantiateFunc(func,uint32,Manip_32_A888,uint32); \
	InstantiateFunc(func,uint32,Manip_32_888A,uint32)
#else
#define InstantiateBilinearScalerFunc(func) \
	InstantiateFunc(func,uint32,Manip_Nat2Nat_32,uint32); \
	InstantiateFunc(func,uint32,Manip_Sta2Nat_32,uint32)
#endif
	};
