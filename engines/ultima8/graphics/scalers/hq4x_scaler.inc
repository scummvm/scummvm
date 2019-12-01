//hq4x filter demo program
//----------------------------------------------------------
//Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later
//version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifdef USE_HQ4X_SCALER

#include "hq4xScaler.h"
#include "Manips.h"
#include "ultima8/graphics/texture.h"

namespace Pentagram {

static bool InitedLUT = false;
static uint32 RGBtoYUV[65536];
static uint32 YUV1;
static uint32 YUV2;
static const uint32 Ymask = 0x00FF0000;
static const uint32 Umask = 0x0000FF00;
static const uint32 Vmask = 0x000000FF;
static const uint32 trY   = 0x00300000;
static const uint32 trU   = 0x00000700;
static const uint32 trV   = 0x00000006;
static const uint32 tableMaskR = 0;
static const uint32 tableMaskG = 0;
static const uint32 tableMaskB = 0;

template<class uintX, class Manip, class uintS=uintX> class hq4xScalerInternal {

	static inline void Interp1(uint8 * pc, uintS c1, uintS c2)
	{
		//	*((int*)pc) = (c1*3+c2) >> 2;
		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]*3+rgba2[i])>>2;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);
	}

	static inline void Interp2(uint8 * pc, uintS c1, uintS c2, uintS c3)
	{
		//  *((int*)pc) = (c1*2+c2+c3) >> 2;
		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 rgba3[4];
		uint8 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);
		Manip::split(c3,rgba3[0], rgba3[1], rgba3[2], rgba3[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]*2+rgba2[i]+rgba3[i])>>2;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);

	}

	static inline void Interp3(uint8 * pc, uintS c1, uintS c2)
	{

		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]*7+rgba2[i])>>3;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);
	}
	
		static inline void Interp4(uint8 * pc, uintS c1, uintS c2, uintS c3)
	{
		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 rgba3[4];
		uint8 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);
		Manip::split(c3,rgba3[0], rgba3[1], rgba3[2], rgba3[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]*2+rgba2[i]+rgba3[i]*7)>>4;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);

	}
	
	static inline void Interp5(uint8 * pc, uintS c1, uintS c2)
	{
		//  *((int*)pc) = (c1+c2) >> 1;
		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]+rgba2[i])>>1;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);

	}

	static inline void Interp6(uint8 * pc, uintS c1, uintS c2, uintS c3)
	{
		//*((int*)pc) = (c1*5+c2*2+c3)/8;

		//*((int*)pc) = ((((c1 & 0x00FF00)*5 + (c2 & 0x00FF00)*2 + (c3 & 0x00FF00) ) & 0x0007F800) +
		//               (((c1 & 0xFF00FF)*5 + (c2 & 0xFF00FF)*2 + (c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 rgba3[4];
		uint8 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);
		Manip::split(c3,rgba3[0], rgba3[1], rgba3[2], rgba3[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]*5+rgba2[i]*2+rgba3[i])>>3;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);
	}

	static inline void Interp7(uint8 * pc, uintS c1, uintS c2, uintS c3)
	{
		//*((int*)pc) = (c1*6+c2+c3)/8;

		//*((int*)pc) = ((((c1 & 0x00FF00)*6 + (c2 & 0x00FF00) + (c3 & 0x00FF00) ) & 0x0007F800) +
		//               (((c1 & 0xFF00FF)*6 + (c2 & 0xFF00FF) + (c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 rgba3[4];
		uint8 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);
		Manip::split(c3,rgba3[0], rgba3[1], rgba3[2], rgba3[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]*6+rgba2[i]+rgba3[i])>>3;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);

	}
	
	static inline void Interp8(uint8 * pc, uintS c1, uintS c2)
	{
		///*((int*)pc) = (c1*5+c2*3)/8;

  		//*((int*)pc) = ((((c1 & 0x00FF00)*5 + (c2 & 0x00FF00)*3 ) & 0x0007F800) +
        //         (((c1 & 0xFF00FF)*5 + (c2 & 0xFF00FF)*3 ) & 0x07F807F8)) >> 3;
		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]*5+rgba2[i]*3)>>3;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);

	}

	static inline void Interp9(uint8 * pc, uintS c1, uintS c2, uintS c3)
	{
		//*((int*)pc) = (c1*2+(c2+c3)*3)/8;

		//*((int*)pc) = ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + (c3 & 0x00FF00))*3 ) & 0x0007F800) +
		//               (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + (c3 & 0xFF00FF))*3 ) & 0x07F807F8)) >> 3;
		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 rgba3[4];
		uint8 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);
		Manip::split(c3,rgba3[0], rgba3[1], rgba3[2], rgba3[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]*2+(rgba2[i]+rgba3[i])*3)>>3;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);
	}

	static inline void Interp10(uint8 * pc, uintS c1, uintS c2, uintS c3)
	{
		//*((int*)pc) = (c1*14+c2+c3)/16;

		//*((int*)pc) = ((((c1 & 0x00FF00)*14 + (c2 & 0x00FF00) + (c3 & 0x00FF00) ) & 0x000FF000) +
		//               (((c1 & 0xFF00FF)*14 + (c2 & 0xFF00FF) + (c3 & 0xFF00FF) ) & 0x0FF00FF0)) >> 4;
		uint8 rgba1[4];
		uint8 rgba2[4];
		uint8 rgba3[4];
		uint32 res[4];

		Manip::split(c1,rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
		Manip::split(c2,rgba2[0], rgba2[1], rgba2[2], rgba2[3]);
		Manip::split(c3,rgba3[0], rgba3[1], rgba3[2], rgba3[3]);

		for (int i=0; i<4; i++) res[i]=(rgba1[i]*14+rgba2[i]+rgba3[i])>>4;

		*reinterpret_cast<uintX*>(pc) = Manip::merge(res[0],res[1],res[2],res[3]);
	}


#define PIXEL00_0     *(reinterpret_cast<uintX*>(pOut)) = Manip::copy(c32[5]);
#define PIXEL00_11    Interp1(pOut, c32[5], c32[4]);
#define PIXEL00_12    Interp1(pOut, c32[5], c32[2]);
#define PIXEL00_20    Interp2(pOut, c32[5], c32[2], c32[4]);
#define PIXEL00_50    Interp5(pOut, c32[2], c32[4]);
#define PIXEL00_80    Interp8(pOut, c32[5], c32[1]);
#define PIXEL00_81    Interp8(pOut, c32[5], c32[4]);
#define PIXEL00_82    Interp8(pOut, c32[5], c32[2]);

#define PIXEL01_0     *(reinterpret_cast<uintX*>(pOut+sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL01_10    Interp1(pOut+sizeof(uintX), c32[5], c32[1]);
#define PIXEL01_12    Interp1(pOut+sizeof(uintX), c32[5], c32[2]);
#define PIXEL01_14    Interp1(pOut+sizeof(uintX), c32[2], c32[5]);
#define PIXEL01_21    Interp2(pOut+sizeof(uintX), c32[2], c32[5], c32[4]);
#define PIXEL01_31    Interp3(pOut+sizeof(uintX), c32[5], c32[4]);
#define PIXEL01_50    Interp5(pOut+sizeof(uintX), c32[2], c32[5]);
#define PIXEL01_60    Interp6(pOut+sizeof(uintX), c32[5], c32[2], c32[4]);
#define PIXEL01_61    Interp6(pOut+sizeof(uintX), c32[5], c32[2], c32[1]);
#define PIXEL01_82    Interp8(pOut+sizeof(uintX), c32[5], c32[2]);
#define PIXEL01_83    Interp8(pOut+sizeof(uintX), c32[2], c32[4]);

#define PIXEL02_0     *(reinterpret_cast<uintX*>(pOut+2*sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL02_10    Interp1(pOut+2*sizeof(uintX), c32[5], c32[3]);
#define PIXEL02_11    Interp1(pOut+2*sizeof(uintX), c32[5], c32[2]);
#define PIXEL02_13    Interp1(pOut+2*sizeof(uintX), c32[2], c32[5]);
#define PIXEL02_21    Interp2(pOut+2*sizeof(uintX), c32[2], c32[5], c32[6]);
#define PIXEL02_32    Interp3(pOut+2*sizeof(uintX), c32[5], c32[6]);
#define PIXEL02_50    Interp5(pOut+2*sizeof(uintX), c32[2], c32[5]);
#define PIXEL02_60    Interp6(pOut+2*sizeof(uintX), c32[5], c32[2], c32[6]);
#define PIXEL02_61    Interp6(pOut+2*sizeof(uintX), c32[5], c32[2], c32[3]);
#define PIXEL02_81    Interp8(pOut+2*sizeof(uintX), c32[5], c32[2]);
#define PIXEL02_83    Interp8(pOut+2*sizeof(uintX), c32[2], c32[6]);

#define PIXEL03_0     *(reinterpret_cast<uintX*>(pOut+3*sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL03_11    Interp1(pOut+3*sizeof(uintX), c32[5], c32[2]);
#define PIXEL03_12    Interp1(pOut+3*sizeof(uintX), c32[5], c32[6]);
#define PIXEL03_20    Interp2(pOut+3*sizeof(uintX), c32[5], c32[2], c32[6]);
#define PIXEL03_50    Interp5(pOut+3*sizeof(uintX), c32[2], c32[6]);
#define PIXEL03_80    Interp8(pOut+3*sizeof(uintX), c32[5], c32[3]);
#define PIXEL03_81    Interp8(pOut+3*sizeof(uintX), c32[5], c32[2]);
#define PIXEL03_82    Interp8(pOut+3*sizeof(uintX), c32[5], c32[6]);

#define PIXEL10_0     *(reinterpret_cast<uintX*>(pOut+BpL)) = Manip::copy(c32[5]);
#define PIXEL10_10    Interp1(pOut+BpL, c32[5], c32[1]);
#define PIXEL10_11    Interp1(pOut+BpL, c32[5], c32[4]);
#define PIXEL10_13    Interp1(pOut+BpL, c32[4], c32[5]);
#define PIXEL10_21    Interp2(pOut+BpL, c32[4], c32[5], c32[2]);
#define PIXEL10_32    Interp3(pOut+BpL, c32[5], c32[2]);
#define PIXEL10_50    Interp5(pOut+BpL, c32[4], c32[5]);
#define PIXEL10_60    Interp6(pOut+BpL, c32[5], c32[4], c32[2]);
#define PIXEL10_61    Interp6(pOut+BpL, c32[5], c32[4], c32[1]);
#define PIXEL10_81    Interp8(pOut+BpL, c32[5], c32[4]);
#define PIXEL10_83    Interp8(pOut+BpL, c32[4], c32[2]);

#define PIXEL11_0     *(reinterpret_cast<uintX*>(pOut+BpL+sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL11_30    Interp3(pOut+BpL+sizeof(uintX), c32[5], c32[1]);
#define PIXEL11_31    Interp3(pOut+BpL+sizeof(uintX), c32[5], c32[4]);
#define PIXEL11_32    Interp3(pOut+BpL+sizeof(uintX), c32[5], c32[2]);
#define PIXEL11_70    Interp7(pOut+BpL+sizeof(uintX), c32[5], c32[4], c32[2]);

#define PIXEL12_0     *(reinterpret_cast<uintX*>(pOut+BpL+2*sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL12_30    Interp3(pOut+BpL+2*sizeof(uintX), c32[5], c32[3]);
#define PIXEL12_31    Interp3(pOut+BpL+2*sizeof(uintX), c32[5], c32[2]);
#define PIXEL12_32    Interp3(pOut+BpL+2*sizeof(uintX), c32[5], c32[6]);
#define PIXEL12_70    Interp7(pOut+BpL+2*sizeof(uintX), c32[5], c32[6], c32[2]);

#define PIXEL13_0     *(reinterpret_cast<uintX*>(pOut+BpL+3*sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL13_10    Interp1(pOut+BpL+3*sizeof(uintX), c32[5], c32[3]);
#define PIXEL13_12    Interp1(pOut+BpL+3*sizeof(uintX), c32[5], c32[6]);
#define PIXEL13_14    Interp1(pOut+BpL+3*sizeof(uintX), c32[6], c32[5]);
#define PIXEL13_21    Interp2(pOut+BpL+3*sizeof(uintX), c32[6], c32[5], c32[2]);
#define PIXEL13_31    Interp3(pOut+BpL+3*sizeof(uintX), c32[5], c32[2]);
#define PIXEL13_50    Interp5(pOut+BpL+3*sizeof(uintX), c32[6], c32[5]);
#define PIXEL13_60    Interp6(pOut+BpL+3*sizeof(uintX), c32[5], c32[6], c32[2]);
#define PIXEL13_61    Interp6(pOut+BpL+3*sizeof(uintX), c32[5], c32[6], c32[3]);
#define PIXEL13_82    Interp8(pOut+BpL+3*sizeof(uintX), c32[5], c32[6]);
#define PIXEL13_83    Interp8(pOut+BpL+3*sizeof(uintX), c32[6], c32[2]);

#define PIXEL20_0     *(reinterpret_cast<uintX*>(pOut+2*BpL)) = Manip::copy(c32[5]);
#define PIXEL20_10    Interp1(pOut+2*BpL, c32[5], c32[7]);
#define PIXEL20_12    Interp1(pOut+2*BpL, c32[5], c32[4]);
#define PIXEL20_14    Interp1(pOut+2*BpL, c32[4], c32[5]);
#define PIXEL20_21    Interp2(pOut+2*BpL, c32[4], c32[5], c32[8]);
#define PIXEL20_31    Interp3(pOut+2*BpL, c32[5], c32[8]);
#define PIXEL20_50    Interp5(pOut+2*BpL, c32[4], c32[5]);
#define PIXEL20_60    Interp6(pOut+2*BpL, c32[5], c32[4], c32[8]);
#define PIXEL20_61    Interp6(pOut+2*BpL, c32[5], c32[4], c32[7]);
#define PIXEL20_82    Interp8(pOut+2*BpL, c32[5], c32[4]);
#define PIXEL20_83    Interp8(pOut+2*BpL, c32[4], c32[8]);

#define PIXEL21_0     *(reinterpret_cast<uintX*>(pOut+2*BpL+sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL21_30    Interp3(pOut+2*BpL+sizeof(uintX), c32[5], c32[7]);
#define PIXEL21_31    Interp3(pOut+2*BpL+sizeof(uintX), c32[5], c32[8]);
#define PIXEL21_32    Interp3(pOut+2*BpL+sizeof(uintX), c32[5], c32[4]);
#define PIXEL21_70    Interp7(pOut+2*BpL+sizeof(uintX), c32[5], c32[4], c32[8]);

#define PIXEL22_0     *(reinterpret_cast<uintX*>(pOut+2*BpL+2*sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL22_30    Interp3(pOut+2*BpL+2*sizeof(uintX), c32[5], c32[9]);
#define PIXEL22_31    Interp3(pOut+2*BpL+2*sizeof(uintX), c32[5], c32[6]);
#define PIXEL22_32    Interp3(pOut+2*BpL+2*sizeof(uintX), c32[5], c32[8]);
#define PIXEL22_70    Interp7(pOut+2*BpL+2*sizeof(uintX), c32[5], c32[6], c32[8]);

#define PIXEL23_0     *(reinterpret_cast<uintX*>(pOut+2*BpL+3*sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL23_10    Interp1(pOut+2*BpL+3*sizeof(uintX), c32[5], c32[9]);
#define PIXEL23_11    Interp1(pOut+2*BpL+3*sizeof(uintX), c32[5], c32[6]);
#define PIXEL23_13    Interp1(pOut+2*BpL+3*sizeof(uintX), c32[6], c32[5]);
#define PIXEL23_21    Interp2(pOut+2*BpL+3*sizeof(uintX), c32[6], c32[5], c32[8]);
#define PIXEL23_32    Interp3(pOut+2*BpL+3*sizeof(uintX), c32[5], c32[8]);
#define PIXEL23_50    Interp5(pOut+2*BpL+3*sizeof(uintX), c32[6], c32[5]);
#define PIXEL23_60    Interp6(pOut+2*BpL+3*sizeof(uintX), c32[5], c32[6], c32[8]);
#define PIXEL23_61    Interp6(pOut+2*BpL+3*sizeof(uintX), c32[5], c32[6], c32[9]);
#define PIXEL23_81    Interp8(pOut+2*BpL+3*sizeof(uintX), c32[5], c32[6]);
#define PIXEL23_83    Interp8(pOut+2*BpL+3*sizeof(uintX), c32[6], c32[8]);

#define PIXEL30_0     *(reinterpret_cast<uintX*>(pOut+3*BpL)) = Manip::copy(c32[5]);
#define PIXEL30_11    Interp1(pOut+3*BpL, c32[5], c32[8]);
#define PIXEL30_12    Interp1(pOut+3*BpL, c32[5], c32[4]);
#define PIXEL30_20    Interp2(pOut+3*BpL, c32[5], c32[8], c32[4]);
#define PIXEL30_50    Interp5(pOut+3*BpL, c32[8], c32[4]);
#define PIXEL30_80    Interp8(pOut+3*BpL, c32[5], c32[7]);
#define PIXEL30_81    Interp8(pOut+3*BpL, c32[5], c32[8]);
#define PIXEL30_82    Interp8(pOut+3*BpL, c32[5], c32[4]);

#define PIXEL31_0     *(reinterpret_cast<uintX*>(pOut+3*BpL+sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL31_10    Interp1(pOut+3*BpL+sizeof(uintX), c32[5], c32[7]);
#define PIXEL31_11    Interp1(pOut+3*BpL+sizeof(uintX), c32[5], c32[8]);
#define PIXEL31_13    Interp1(pOut+3*BpL+sizeof(uintX), c32[8], c32[5]);
#define PIXEL31_21    Interp2(pOut+3*BpL+sizeof(uintX), c32[8], c32[5], c32[4]);
#define PIXEL31_32    Interp3(pOut+3*BpL+sizeof(uintX), c32[5], c32[4]);
#define PIXEL31_50    Interp5(pOut+3*BpL+sizeof(uintX), c32[8], c32[5]);
#define PIXEL31_60    Interp6(pOut+3*BpL+sizeof(uintX), c32[5], c32[8], c32[4]);
#define PIXEL31_61    Interp6(pOut+3*BpL+sizeof(uintX), c32[5], c32[8], c32[7]);
#define PIXEL31_81    Interp8(pOut+3*BpL+sizeof(uintX), c32[5], c32[8]);
#define PIXEL31_83    Interp8(pOut+3*BpL+sizeof(uintX), c32[8], c32[4]);

#define PIXEL32_0     *(reinterpret_cast<uintX*>(pOut+3*BpL+2*sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL32_10    Interp1(pOut+3*BpL+2*sizeof(uintX), c32[5], c32[9]);
#define PIXEL32_12    Interp1(pOut+3*BpL+2*sizeof(uintX), c32[5], c32[8]);
#define PIXEL32_14    Interp1(pOut+3*BpL+2*sizeof(uintX), c32[8], c32[5]);
#define PIXEL32_21    Interp2(pOut+3*BpL+2*sizeof(uintX), c32[8], c32[5], c32[6]);
#define PIXEL32_31    Interp3(pOut+3*BpL+2*sizeof(uintX), c32[5], c32[6]);
#define PIXEL32_50    Interp5(pOut+3*BpL+2*sizeof(uintX), c32[8], c32[5]);
#define PIXEL32_60    Interp6(pOut+3*BpL+2*sizeof(uintX), c32[5], c32[8], c32[6]);
#define PIXEL32_61    Interp6(pOut+3*BpL+2*sizeof(uintX), c32[5], c32[8], c32[9]);
#define PIXEL32_82    Interp8(pOut+3*BpL+2*sizeof(uintX), c32[5], c32[8]);
#define PIXEL32_83    Interp8(pOut+3*BpL+2*sizeof(uintX), c32[8], c32[6]);

#define PIXEL33_0     *(reinterpret_cast<uintX*>(pOut+3*BpL+3*sizeof(uintX))) = Manip::copy(c32[5]);
#define PIXEL33_11    Interp1(pOut+3*BpL+3*sizeof(uintX), c32[5], c32[6]);
#define PIXEL33_12    Interp1(pOut+3*BpL+3*sizeof(uintX), c32[5], c32[8]);
#define PIXEL33_20    Interp2(pOut+3*BpL+3*sizeof(uintX), c32[5], c32[8], c32[6]);
#define PIXEL33_50    Interp5(pOut+3*BpL+3*sizeof(uintX), c32[8], c32[6]);
#define PIXEL33_80    Interp8(pOut+3*BpL+3*sizeof(uintX), c32[5], c32[9]);
#define PIXEL33_81    Interp8(pOut+3*BpL+3*sizeof(uintX), c32[5], c32[6]);
#define PIXEL33_82    Interp8(pOut+3*BpL+3*sizeof(uintX), c32[5], c32[8]);

	static inline bool Diff(unsigned int w1, unsigned int w2)
	{
		YUV1 = RGBtoYUV[w1];
		YUV2 = RGBtoYUV[w2];
		return ( ( (unsigned int)abs(int32(YUV1 & Ymask) - int32(YUV2 & Ymask)) > trY ) ||
			( (unsigned int)abs(int32(YUV1 & Umask) - int32(YUV2 & Umask)) > trU ) ||
			( (unsigned int)abs(int32(YUV1 & Vmask) - int32(YUV2 & Vmask)) > trV ) );
	}

public:

	static void InitLUTs(void)
	{
		if (InitedLUT) return;
		InitedLUT = true;

		int i, j, k, r, g, b, Y, u, v;

		for (i=0; i<32; i++)
			for (j=0; j<64; j++)
				for (k=0; k<32; k++)
				{
					r = i << 3;
					g = j << 2;
					b = k << 3;
					Y = (r + g + b) >> 2;
					u = 128 + ((r - b) >> 2);
					v = 128 + ((-r + 2*g -b)>>3);
					RGBtoYUV[ (i << 11) + (j << 5) + k ] = (Y<<16) + (u<<8) + v;
				}
	}

	static bool hq4x_32(Texture *tex, int32 sx, int32 sy, int32 Xres, int32 Yres, 
		uint8* pOut, int32 dw, int32 dh, int32 BpL, bool clamp_src)
	{
		if (Xres*4!=dw || Yres*4!=dh) return false;

		InitLUTs();

		int		i, j, k;
		int		prevline, nextline;
		uint32	w16[10];
		uintS	c32[10];

		// Source buffer pointers
		int tpitch = tex->width*sizeof(uintS);
		uint8 *pIn = reinterpret_cast<uint8*>(tex->buffer) + sy*tpitch + sx*sizeof(uintS);
		int tex_diff = tpitch - Xres*sizeof(uintX);

		int pix_diff = BpL*4-Xres*4*sizeof(uintX);

		bool clipX = true;
		bool clipY_Begin = true;
		bool clipY_End = true;

		if (!clamp_src && sy!=0) clipY_Begin = false;
		if (!clamp_src && (Yres+sy)<tex->height) clipY_End = false;

		//   +----+----+----+
		//   |    |    |    |
		//   | w1 | w2 | w3 |
		//   +----+----+----+
		//   |    |    |    |
		//   | w4 | w5 | w6 |
		//   +----+----+----+
		//   |    |    |    |
		//   | w7 | w8 | w9 |
		//   +----+----+----+

		for (j=0; j<Yres; j++)
		{
			if (j>0 || !clipY_Begin)   prevline = -tpitch; else prevline = 0;
			if (j<Yres-1 || !clipY_End)	nextline =  tpitch; else nextline = 0;

			// Read first 2 columns of pixels 
			c32[2] = c32[3] = *reinterpret_cast<uintS*>(pIn + prevline);
			c32[5] = c32[6] = *reinterpret_cast<uintS*>(pIn);
			c32[8] = c32[9] = *reinterpret_cast<uintS*>(pIn + nextline);

			w16[2] = w16[3] = Manip::to16bit(c32[2]);
			w16[5] = w16[6] = Manip::to16bit(c32[5]);
			w16[8] = w16[9] = Manip::to16bit(c32[6]);

			for (i=0; i<Xres; i++)
			{
				// First col = Prev Second column
				c32[1] = c32[2];
				c32[4] = c32[5];
				c32[7] = c32[8];
				w16[1] = w16[2];
				w16[4] = w16[5];
				w16[7] = w16[8];

				// Second col = Prev Third column
				c32[2] = c32[3];
				c32[5] = c32[6];
				c32[8] = c32[9];
				w16[2] = w16[3];
				w16[5] = w16[6];
				w16[8] = w16[9];

				// Read Next Pixel
				if (i<Xres-1 || !clipX)
				{
					c32[3] = *reinterpret_cast<uintS*>(pIn + sizeof(uintS) + prevline);
					c32[6] = *reinterpret_cast<uintS*>(pIn + sizeof(uintS));
					c32[9] = *reinterpret_cast<uintS*>(pIn + sizeof(uintS) + nextline);
					w16[3] = Manip::to16bit(c32[3]);
					w16[6] = Manip::to16bit(c32[6]);
					w16[9] = Manip::to16bit(c32[9]);
				}

				int pattern = 0;
				int flag = 1;

				YUV1 = RGBtoYUV[w16[5]];

				for (k=1; k<=9; k++)
				{
					if (k==5) continue;

					if ( w16[k] != w16[5] )
					{
						YUV2 = RGBtoYUV[w16[k]];
						if ( ( (unsigned int)abs(int32(YUV1 & Ymask) - int32(YUV2 & Ymask)) > trY ) ||
							( (unsigned int)abs(int32(YUV1 & Umask) - int32(YUV2 & Umask)) > trU ) ||
							( (unsigned int)abs(int32(YUV1 & Vmask) - int32(YUV2 & Vmask)) > trV ) )
							pattern |= flag;
					}
					flag <<= 1;
				}

				switch (pattern)
				{
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
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 2:
        case 34:
        case 130:
        case 162:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 16:
        case 17:
        case 48:
        case 49:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 64:
        case 65:
        case 68:
        case 69:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 8:
        case 12:
        case 136:
        case 140:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 3:
        case 35:
        case 131:
        case 163:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 6:
        case 38:
        case 134:
        case 166:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 20:
        case 21:
        case 52:
        case 53:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 144:
        case 145:
        case 176:
        case 177:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 192:
        case 193:
        case 196:
        case 197:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 96:
        case 97:
        case 100:
        case 101:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 40:
        case 44:
        case 168:
        case 172:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 9:
        case 13:
        case 137:
        case 141:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 18:
        case 50:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL12_0
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 80:
        case 81:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 72:
        case 76:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_50
            PIXEL21_0
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 10:
        case 138:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
            PIXEL11_0
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 66:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 24:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 7:
        case 39:
        case 135:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 148:
        case 149:
        case 180:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 224:
        case 228:
        case 225:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 41:
        case 169:
        case 45:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 22:
        case 54:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 208:
        case 209:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 104:
        case 108:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 11:
        case 139:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 19:
        case 51:
        {
          if (Diff(w16[2], w16[6]))
          {
            PIXEL00_81
            PIXEL01_31
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL00_12
            PIXEL01_14
            PIXEL02_83
            PIXEL03_50
            PIXEL12_70
            PIXEL13_21
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 146:
        case 178:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
            PIXEL23_32
            PIXEL33_82
          }
          else
          {
            PIXEL02_21
            PIXEL03_50
            PIXEL12_70
            PIXEL13_83
            PIXEL23_13
            PIXEL33_11
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          break;
        }
        case 84:
        case 85:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          if (Diff(w16[6], w16[8]))
          {
            PIXEL03_81
            PIXEL13_31
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL03_12
            PIXEL13_14
            PIXEL22_70
            PIXEL23_83
            PIXEL32_21
            PIXEL33_50
          }
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL20_61
          PIXEL21_30
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 112:
        case 113:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_82
          PIXEL21_32
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL30_82
            PIXEL31_32
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_70
            PIXEL23_21
            PIXEL30_11
            PIXEL31_13
            PIXEL32_83
            PIXEL33_50
          }
          break;
        }
        case 200:
        case 204:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
            PIXEL32_31
            PIXEL33_81
          }
          else
          {
            PIXEL20_21
            PIXEL21_70
            PIXEL30_50
            PIXEL31_83
            PIXEL32_14
            PIXEL33_12
          }
          PIXEL22_31
          PIXEL23_81
          break;
        }
        case 73:
        case 77:
        {
          if (Diff(w16[8], w16[4]))
          {
            PIXEL00_82
            PIXEL10_32
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL00_11
            PIXEL10_13
            PIXEL20_83
            PIXEL21_70
            PIXEL30_50
            PIXEL31_21
          }
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 42:
        case 170:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
            PIXEL20_31
            PIXEL30_81
          }
          else
          {
            PIXEL00_50
            PIXEL01_21
            PIXEL10_83
            PIXEL11_70
            PIXEL20_14
            PIXEL30_12
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 14:
        case 142:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL02_32
            PIXEL03_82
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_50
            PIXEL01_83
            PIXEL02_13
            PIXEL03_11
            PIXEL10_21
            PIXEL11_70
          }
          PIXEL12_32
          PIXEL13_82
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 67:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 70:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 28:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 152:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 194:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 98:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 56:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 25:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 26:
        case 31:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 82:
        case 214:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 88:
        case 248:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 74:
        case 107:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 27:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 86:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 216:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 106:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 30:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 210:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 120:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 75:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 29:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 198:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 184:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 99:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 57:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 71:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 156:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 226:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 60:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 195:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 102:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 153:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 58:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 83:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_61
          PIXEL21_30
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 92:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 202:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 78:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL12_32
          PIXEL13_82
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 154:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 114:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_82
          PIXEL21_32
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 89:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 90:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 55:
        case 23:
        {
          if (Diff(w16[2], w16[6]))
          {
            PIXEL00_81
            PIXEL01_31
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
          }
          else
          {
            PIXEL00_12
            PIXEL01_14
            PIXEL02_83
            PIXEL03_50
            PIXEL12_70
            PIXEL13_21
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 182:
        case 150:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
            PIXEL23_32
            PIXEL33_82
          }
          else
          {
            PIXEL02_21
            PIXEL03_50
            PIXEL12_70
            PIXEL13_83
            PIXEL23_13
            PIXEL33_11
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          break;
        }
        case 213:
        case 212:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          if (Diff(w16[6], w16[8]))
          {
            PIXEL03_81
            PIXEL13_31
            PIXEL22_0
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL03_12
            PIXEL13_14
            PIXEL22_70
            PIXEL23_83
            PIXEL32_21
            PIXEL33_50
          }
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL20_61
          PIXEL21_30
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 241:
        case 240:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_82
          PIXEL21_32
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_0
            PIXEL23_0
            PIXEL30_82
            PIXEL31_32
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL22_70
            PIXEL23_21
            PIXEL30_11
            PIXEL31_13
            PIXEL32_83
            PIXEL33_50
          }
          break;
        }
        case 236:
        case 232:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
            PIXEL32_31
            PIXEL33_81
          }
          else
          {
            PIXEL20_21
            PIXEL21_70
            PIXEL30_50
            PIXEL31_83
            PIXEL32_14
            PIXEL33_12
          }
          PIXEL22_31
          PIXEL23_81
          break;
        }
        case 109:
        case 105:
        {
          if (Diff(w16[8], w16[4]))
          {
            PIXEL00_82
            PIXEL10_32
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL00_11
            PIXEL10_13
            PIXEL20_83
            PIXEL21_70
            PIXEL30_50
            PIXEL31_21
          }
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 171:
        case 43:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
            PIXEL11_0
            PIXEL20_31
            PIXEL30_81
          }
          else
          {
            PIXEL00_50
            PIXEL01_21
            PIXEL10_83
            PIXEL11_70
            PIXEL20_14
            PIXEL30_12
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 143:
        case 15:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL02_32
            PIXEL03_82
            PIXEL10_0
            PIXEL11_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_83
            PIXEL02_13
            PIXEL03_11
            PIXEL10_21
            PIXEL11_70
          }
          PIXEL12_32
          PIXEL13_82
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 124:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 203:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 62:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 211:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 118:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_10
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 217:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 110:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_10
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 155:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 188:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 185:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 61:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 157:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 103:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 227:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 230:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 199:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 220:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 158:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 234:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 242:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 59:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL11_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 121:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 87:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL20_61
          PIXEL21_30
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 79:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 122:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 94:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL12_0
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 218:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 91:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL11_0
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 229:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 167:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 173:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 181:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 186:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 115:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_82
          PIXEL21_32
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 93:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 206:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL12_32
          PIXEL13_82
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 205:
        case 201:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 174:
        case 46:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL12_32
          PIXEL13_82
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 179:
        case 147:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 117:
        case 116:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_82
          PIXEL21_32
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 189:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 231:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 126:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 219:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 125:
        {
          if (Diff(w16[8], w16[4]))
          {
            PIXEL00_82
            PIXEL10_32
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL00_11
            PIXEL10_13
            PIXEL20_83
            PIXEL21_70
            PIXEL30_50
            PIXEL31_21
          }
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 221:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          if (Diff(w16[6], w16[8]))
          {
            PIXEL03_81
            PIXEL13_31
            PIXEL22_0
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL03_12
            PIXEL13_14
            PIXEL22_70
            PIXEL23_83
            PIXEL32_21
            PIXEL33_50
          }
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL20_10
          PIXEL21_30
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 207:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL02_32
            PIXEL03_82
            PIXEL10_0
            PIXEL11_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_83
            PIXEL02_13
            PIXEL03_11
            PIXEL10_21
            PIXEL11_70
          }
          PIXEL12_32
          PIXEL13_82
          PIXEL20_10
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 238:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_10
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
            PIXEL32_31
            PIXEL33_81
          }
          else
          {
            PIXEL20_21
            PIXEL21_70
            PIXEL30_50
            PIXEL31_83
            PIXEL32_14
            PIXEL33_12
          }
          PIXEL22_31
          PIXEL23_81
          break;
        }
        case 190:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
            PIXEL23_32
            PIXEL33_82
          }
          else
          {
            PIXEL02_21
            PIXEL03_50
            PIXEL12_70
            PIXEL13_83
            PIXEL23_13
            PIXEL33_11
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          break;
        }
        case 187:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
            PIXEL11_0
            PIXEL20_31
            PIXEL30_81
          }
          else
          {
            PIXEL00_50
            PIXEL01_21
            PIXEL10_83
            PIXEL11_70
            PIXEL20_14
            PIXEL30_12
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_10
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 243:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_10
          PIXEL20_82
          PIXEL21_32
          if (Diff(w16[6], w16[8]))
          {
            PIXEL22_0
            PIXEL23_0
            PIXEL30_82
            PIXEL31_32
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL22_70
            PIXEL23_21
            PIXEL30_11
            PIXEL31_13
            PIXEL32_83
            PIXEL33_50
          }
          break;
        }
        case 119:
        {
          if (Diff(w16[2], w16[6]))
          {
            PIXEL00_81
            PIXEL01_31
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
          }
          else
          {
            PIXEL00_12
            PIXEL01_14
            PIXEL02_83
            PIXEL03_50
            PIXEL12_70
            PIXEL13_21
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_10
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 237:
        case 233:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_0
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          if (Diff(w16[8], w16[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 175:
        case 47:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_32
          PIXEL03_82
          PIXEL10_0
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 183:
        case 151:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_0
          if (Diff(w16[2], w16[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL13_0
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 245:
        case 244:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          PIXEL23_0
          PIXEL30_82
          PIXEL31_32
          PIXEL32_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 250:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 123:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 95:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 222:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 252:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          PIXEL32_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 249:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          if (Diff(w16[8], w16[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          break;
        }
        case 235:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_0
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          if (Diff(w16[8], w16[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 111:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_32
          PIXEL03_82
          PIXEL10_0
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 63:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 159:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_0
          if (Diff(w16[2], w16[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 215:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_0
          if (Diff(w16[2], w16[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL13_0
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 246:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          PIXEL23_0
          PIXEL30_82
          PIXEL31_32
          PIXEL32_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 254:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          PIXEL32_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 253:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          if (Diff(w16[8], w16[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 251:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          if (Diff(w16[8], w16[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          break;
        }
        case 239:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_32
          PIXEL03_82
          PIXEL10_0
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          PIXEL20_0
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          if (Diff(w16[8], w16[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 127:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          if (Diff(w16[2], w16[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          if (Diff(w16[8], w16[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 191:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_0
          if (Diff(w16[2], w16[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 223:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_0
          if (Diff(w16[2], w16[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 247:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_0
          if (Diff(w16[2], w16[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL13_0
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          PIXEL23_0
          PIXEL30_82
          PIXEL31_32
          PIXEL32_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 255:
        {
          if (Diff(w16[4], w16[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_0
          if (Diff(w16[2], w16[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          if (Diff(w16[8], w16[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_0
          if (Diff(w16[6], w16[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
				}
				pIn+=sizeof(uintS);
				pOut+=sizeof(uintX)*4;
			}
			pIn+=tex_diff;
			pOut+=pix_diff;
		}

		return true;
	}

};	 // class

template<class uintX, class Manip, class uintS> Scaler::ScalerFunc hq4xScaler::GetScaler()
{
	return hq4xScalerInternal<uintX, Manip, uintS>::hq4x_32;
}

};	// namespace Pentagram

#endif
