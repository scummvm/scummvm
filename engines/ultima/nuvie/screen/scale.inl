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

#ifndef SCALE_INL_INCLUDED
#define SCALE_INL_INCLUDED

namespace Ultima {
namespace Nuvie {

/*
 *	Manipulate 16-bit 555 format.
 */
class ManipRGB555
{
public:
	inline static uint16 rgb(unsigned int r, unsigned int g, unsigned int b)
	{
		return ((r>>3)<<10)|((g>>3)<<5)|(b>>3);
	}
	inline static void split_col(uint16 pix, unsigned int& r, unsigned int& g, unsigned int& b)
	{
		r = ((pix&0x7c00)>>10)<<3;
		g = ((pix&0x03e0)>>5)<<3;
		b =  (pix&0x001f)<<3;
	}
};

/*
 *	Manipulate 16-bit 565 format.
 */
class ManipRGB565
{
public:
	inline static uint16 rgb(unsigned int r, unsigned int g, unsigned int b)
	{
		return ((r>>3)<<11)|((g>>2)<<5)|(b>>3);
	}
	inline static void split_col(uint16 pix, unsigned int& r, unsigned int& g, unsigned int& b)
	{
		r = ((pix&0xf800)>>11)<<3;
		g = ((pix&0x07e0)>>5)<<2;
		b =  (pix&0x001f)<<3;
	}
};

/*
 *	Manipulate 32-bit 888 format.
 */
class ManipRGB888
{
public:
	inline static uint32 rgb(unsigned int r, unsigned int g, unsigned int b)
	{
		return (r<<16)|(g<<8)|(b);
	}
	inline static void split_col(uint32 pix, unsigned int& r, unsigned int& g, unsigned int& b)
	{
		r = (pix&0xFF0000)>>16;
		g = (pix&0x00FF00)>>8;
		b = (pix&0x0000FF);
	}
};

/*
 *	Manipulate from 16-bit to 16-bit pixels or 32-bit to 32-bit.
 */
class ManipRGBGeneric
{
public:
	inline static uint32 rgb(unsigned int r, unsigned int g, unsigned int b)
	{
		return ((r>>RenderSurface::Rloss)<<RenderSurface::Rshift) |
		       ((g>>RenderSurface::Gloss)<<RenderSurface::Gshift) |
		       ((b>>RenderSurface::Bloss)<<RenderSurface::Bshift);
	}
	inline static void split_col(uint32 pix, unsigned int& r, unsigned int& g, unsigned int& b)
	{
		r = ((pix&RenderSurface::Rmask)>>RenderSurface::Rshift)<<RenderSurface::Rloss;
		g = ((pix&RenderSurface::Gmask)>>RenderSurface::Gshift)<<RenderSurface::Gloss;
		b = ((pix&RenderSurface::Bmask)>>RenderSurface::Bshift)<<RenderSurface::Bloss;
	}
};


template <class Pixel_type, class Manip_pixels> class Scalers {
public:


/**
 ** 2xSaI scaling filter source code adapted for Exult
 ** August 29 2000, originally written in May 1999
 ** by Derek Liauw Kie Fa (DerekL666@yahoo.com/D.A.K.L.LiauwKieFa@student.tudelft.nl)
 ** This source is made available under the terms of the GNU GPL
 ** I'd appreciate it I am given credit in the program or documentation
 **/

static inline Pixel_type Interpolate_2xSaI (Pixel_type colorA, Pixel_type colorB)
{
	unsigned int r0, r1, g0, g1, b0, b1;
	Manip_pixels::split_col(colorA, r0, g0, b0);
	Manip_pixels::split_col(colorB, r1, g1, b1);
	int r = (r0 + r1)>>1;
	int g = (g0 + g1)>>1;
	int b = (b0 + b1)>>1;
	return Manip_pixels::rgb(r, g, b);
}

static inline Pixel_type OInterpolate_2xSaI (Pixel_type colorA, Pixel_type colorB, Pixel_type colorC)
{
	unsigned int r0, r1, g0, g1, b0, b1;
	unsigned int r2, g2, b2;
	Manip_pixels::split_col(colorA, r0, g0, b0);
	Manip_pixels::split_col(colorB, r1, g1, b1);
	Manip_pixels::split_col(colorC, r2, g2, b2);
	unsigned int r = ((r0<<2) + (r0<<1) + r1 + r2)>>3;
	unsigned int g = ((g0<<2) + (g0<<1) + g1 + g2)>>3;
	unsigned int b = ((b0<<2) + (b0<<1) + b1 + b2)>>3;
	return Manip_pixels::rgb(r, g, b);
}

static inline Pixel_type QInterpolate_2xSaI (Pixel_type colorA, Pixel_type colorB, Pixel_type colorC, Pixel_type colorD)
{
	unsigned int r0, r1, g0, g1, b0, b1;
	unsigned int r2, r3, g2, g3, b2, b3;
	Manip_pixels::split_col(colorA, r0, g0, b0);
	Manip_pixels::split_col(colorB, r1, g1, b1);
	Manip_pixels::split_col(colorC, r2, g2, b2);
	Manip_pixels::split_col(colorD, r3, g3, b3);
	unsigned int r = (r0 + r1 + r2 + r3)>>2;
	unsigned int g = (g0 + g1 + g2 + g3)>>2;
	unsigned int b = (b0 + b1 + b2 + b3)>>2;
	return Manip_pixels::rgb(r, g, b);
}

static inline int GetResult1(Pixel_type A, Pixel_type B, Pixel_type C, Pixel_type D)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r+=1;
	if (y <= 1) r-=1;
	return r;
}

static inline int GetResult2(Pixel_type A, Pixel_type B, Pixel_type C, Pixel_type D)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r-=1;
	if (y <= 1) r+=1;
	return r;
}

//
// 2xSaI Scaler
//
static void Scale_2xSaI
(
	Pixel_type *source,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dest,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int scale_factor			// Scale Factor
)
{
	Pixel_type *srcPtr = source + (srcx + srcy*sline_pixels);
	Pixel_type *dstPtr = dest + (2*srcy*dline_pixels + 2*srcx);

	if (srcx + srcw >= (int)sline_pixels)
	{
		srcw = sline_pixels - srcx;
	}
					// Init offset to prev. line, next 2.
	int prev1_yoff = srcy ? sline_pixels : 0;
	int next1_yoff = sline_pixels, next2_yoff = 2*sline_pixels;
					// Figure threshholds for counters.
	int ybeforelast = sheight - 2 - srcy;
	int xbeforelast = sline_pixels - 2 - srcx;
	for (int y = 0; y < srch; y++, prev1_yoff = sline_pixels)
	{
		if (y >= ybeforelast)	// Last/next-to-last row?
		{
			if (y == ybeforelast)
				next2_yoff = sline_pixels;
			else		// Very last line?
				next2_yoff = next1_yoff = 0;
		}

		Pixel_type *bP = srcPtr;
		Pixel_type *dP = dstPtr;
		int prev1_xoff = srcx ? 1 : 0;
		int next1_xoff = 1, next2_xoff = 2;

		for (int x = 0; x < srcw; x++)
		{
			Pixel_type colorA, colorB;
			Pixel_type colorC, colorD,
				colorE, colorF, colorG, colorH,
				colorI, colorJ, colorK, colorL,
				colorM, colorN, colorO; // , colorP;
			Pixel_type product, product1, product2;

				// Last/next-to-last row?
			if (x >= xbeforelast)
			{
				if (x == xbeforelast)
					next2_xoff = 1;
				else
					next2_xoff = next1_xoff = 0;
			}

			//---------------------------------------
			// Map of the pixels:                    I|E F|J
			//                                       G|A B|K
			//                                       H|C D|L
			//                                       M|N O|P
			colorI = *(bP- prev1_yoff - prev1_xoff);
			colorE = *(bP- prev1_yoff);
			colorF = *(bP- prev1_yoff + next1_xoff);
			colorJ = *(bP- prev1_yoff + next2_xoff);

			colorG = *(bP - prev1_xoff);
			colorA = *(bP);
			colorB = *(bP + next1_xoff);
			colorK = *(bP + next2_xoff);

			colorH = *(bP + next1_yoff - prev1_xoff);
			colorC = *(bP + next1_yoff);
			colorD = *(bP + next1_yoff + next1_xoff);
			colorL = *(bP + next1_yoff + next2_xoff);

			colorM = *(bP + next2_yoff - prev1_xoff);
			colorN = *(bP + next2_yoff);
			colorO = *(bP + next2_yoff + next1_xoff);
			//colorP = *(bP + next2_yoff + next2_xoff);

			if ((colorA == colorD) && (colorB != colorC))
			{
			   if ( ((colorA == colorE) && (colorB == colorL)) ||
					((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ)) )
			   {
				  product = colorA;
			   }
			   else
			   {
				  //product = INTERPOLATE(colorA, colorB);
				  product = Interpolate_2xSaI(colorA, colorB);
			   }

			   if (((colorA == colorG) && (colorC == colorO)) ||
				   ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM)) )
			   {
				  product1 = colorA;
			   }
			   else
			   {
				  //product1 = INTERPOLATE(colorA, colorC);
				  product1 = Interpolate_2xSaI(colorA, colorC);
			   }
			   product2 = colorA;
			}
			else
			if ((colorB == colorC) && (colorA != colorD))
			{
			   if (((colorB == colorF) && (colorA == colorH)) ||
				   ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)) )
			   {
				  product = colorB;
			   }
			   else
			   {
				  //product = INTERPOLATE(colorA, colorB);
				  product = Interpolate_2xSaI(colorA, colorB);
			   }

			   if (((colorC == colorH) && (colorA == colorF)) ||
				   ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)) )
			   {
				  product1 = colorC;
			   }
			   else
			   {
				  //product1 = INTERPOLATE(colorA, colorC);
				  product1 = Interpolate_2xSaI(colorA, colorC);
			   }
			   product2 = colorB;
			}
			else
			if ((colorA == colorD) && (colorB == colorC))
			{
			   if (colorA == colorB)
			   {
				  product = colorA;
				  product1 = colorA;
				  product2 = colorA;
			   }
			   else
			   {
			   	  int r = 0;
				  //product1 = INTERPOLATE(colorA, colorC);
				  product1 = Interpolate_2xSaI(colorA, colorC);
				  //product = INTERPOLATE(colorA, colorB);
				  product = Interpolate_2xSaI(colorA, colorB);

				  r += GetResult1 (colorA, colorB, colorG, colorE);
				  r += GetResult2 (colorB, colorA, colorK, colorF);
				  r += GetResult2 (colorB, colorA, colorH, colorN);
				  r += GetResult1 (colorA, colorB, colorL, colorO);

				  if (r > 0)
					  product2 = colorA;
				  else
				  if (r < 0)
					  product2 = colorB;
				  else
				  {
					  //product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
					  product2 = QInterpolate_2xSaI(colorA, colorB, colorC, colorD);
				  }
			   }
			}
			else
			{
			   //product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
			   product2 = QInterpolate_2xSaI(colorA, colorB, colorC, colorD);

			   if ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))
			   {
				  product = colorA;
			   }
			   else
			   if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))
			   {
				  product = colorB;
			   }
			   else
			   {
				  //product = INTERPOLATE(colorA, colorB);
				  product = Interpolate_2xSaI(colorA, colorB);
			   }

			   if ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM))
			   {
				  product1 = colorA;
			   }
			   else
			   if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))
			   {
				  product1 = colorC;
			   }
			   else
			   {
				  //product1 = INTERPOLATE(colorA, colorC);
				  product1 = Interpolate_2xSaI(colorA, colorC);
			   }
			}


			//product = colorA | (product << 16);
			//product1 = product1 | (product2 << 16);
			*dP = colorA;
			*(dP+1) = product;
			*(dP+dline_pixels) = product1;
			*(dP+dline_pixels+1) = product2;

			bP += 1;
			dP += 2;
		}//end of for ( finish= width etc..)

		srcPtr += sline_pixels;
		dstPtr += 2*dline_pixels;
	};
}

//
// Super2xSaI Scaler
//
static void Scale_Super2xSaI
(
	Pixel_type *source,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dest,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int scale_factor			// Scale Factor
)
{

	Pixel_type *srcPtr = source + (srcx + srcy*sline_pixels);
	Pixel_type *dstPtr = dest + (2*srcy*dline_pixels + 2*srcx);

	if (srcx + srcw >= (int)sline_pixels)
	{
		srcw = sline_pixels - srcx;
	}

    int ybeforelast1 = sheight - 1 - srcy;
    int ybeforelast2 = sheight - 2 - srcy;
    int xbeforelast1 = sline_pixels - 1 - srcx;
    int xbeforelast2 = sline_pixels - 2 - srcx;

    for (int y = 0; y < srch; y++)
	{
		Pixel_type *bP = srcPtr;
		Pixel_type *dP = dstPtr;

		for (int x = 0; x < srcw; x++)
		{
           Pixel_type color4, color5, color6;
           Pixel_type color1, color2, color3;
           Pixel_type colorA0, colorA1, colorA2, colorA3,
						colorB0, colorB1, colorB2, colorB3,
						colorS1, colorS2;
           Pixel_type product1a, product1b,
					 product2a, product2b;

			//---------------------------------------  B0 B1 B2 B3
			//                                         4  5  6  S2
			//                                         1  2  3  S1
			//                                         A0 A1 A2 A3
			//--------------------------------------
			int add1, add2;
			int sub1;
			int nextl1, nextl2;
			int prevl1;

			if (x == 0)
				sub1 = 0;
			else
				sub1 = 1;

			if (x >= xbeforelast2)
				add2 = 0;
			else add2 = 1;

			if (x >= xbeforelast1)
				add1 = 0;
			else add1 = 1;

			if (y == 0)
				prevl1 = 0;
			else
				prevl1 = sline_pixels;

			if (y >= ybeforelast2)
				nextl2 = 0;
			else nextl2 = sline_pixels;

			if (y >= ybeforelast1)
				nextl1 = 0;
			else nextl1 = sline_pixels;


            colorB0 = *(bP- prevl1 - sub1);
            colorB1 = *(bP- prevl1);
            colorB2 = *(bP- prevl1 + add1);
            colorB3 = *(bP- prevl1 + add1 + add2);

            color4 = *(bP - sub1);
            color5 = *(bP);
            color6 = *(bP + add1);
            colorS2 = *(bP + add1 + add2);

            color1 = *(bP + nextl1 - sub1);
            color2 = *(bP + nextl1);
            color3 = *(bP + nextl1 + add1);
            colorS1 = *(bP + nextl1 + add1 + add2);

            colorA0 = *(bP + nextl1 + nextl2 - sub1);
            colorA1 = *(bP + nextl1 + nextl2);
            colorA2 = *(bP + nextl1 + nextl2 + add1);
            colorA3 = *(bP + nextl1 + nextl2 + add1 + add2);

			if (color2 == color6 && color5 != color3)
			{
			   product2b = product1b = color2;
			}
			else
			if (color5 == color3 && color2 != color6)
			{
			   product2b = product1b = color5;
			}
			else
			if (color5 == color3 && color2 == color6)
			{
			   	int r = 0;

               	//r += GetResult (color6, color5, color1, colorA1);
               	//r += GetResult (color6, color5, color4, colorB1);
               	//r += GetResult (color6, color5, colorA2, colorS1);
               	//r += GetResult (color6, color5, colorB2, colorS2);
			   	r += GetResult1 (color5, color6, color4, colorB1);
			   	r += GetResult2 (color6, color5, colorA2, colorS1);
			   	r += GetResult2 (color6, color5, color1, colorA1);
 			   	r += GetResult1 (color5, color6, colorB2, colorS2);

			   	if (r > 0)
				{
				 	product2b = product1b = color6;
				}
			   	else
			   	if (r < 0)
				{
					product2b = product1b = color5;
				}
			   	else
			   	{
				  	//product2b = product1b = INTERPOLATE (color5, color6);
				  	product1b = product2b = Interpolate_2xSaI(color5, color6);
			   	}

			}
			else
			{

			   if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
				  	//product2b = Q_INTERPOLATE (color3, color3, color3, color2);
				  	product2b = QInterpolate_2xSaI(color3, color3, color3, color2);
			   else
			   if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
				  	//product2b = Q_INTERPOLATE (color2, color2, color2, color3);
				   	product2b = QInterpolate_2xSaI(color3, color2, color2, color2);
			   else
				  	//product2b = INTERPOLATE (color2, color3);
				  	product2b = Interpolate_2xSaI(color2, color3);


			   if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
				  	//product1b = Q_INTERPOLATE (color6, color6, color6, color5);
				   	product1b = QInterpolate_2xSaI(color5, color6, color6, color6);
			   else
			   if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
				  	//product1b = Q_INTERPOLATE (color6, color5, color5, color5);
				   	product1b = QInterpolate_2xSaI(color6, color5, color5, color5);
			   else
				  	//product1b = INTERPOLATE (color5, color6);
				  	product1b = Interpolate_2xSaI(color5, color6);

			}

			if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
			   	//product2a = INTERPOLATE (color2, color5);
			  	product2a = Interpolate_2xSaI(color5, color2);
			else
			if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
			   	//product2a = INTERPOLATE(color2, color5);
			  	product2a = Interpolate_2xSaI(color5, color2);
			else
			   	product2a = color2;


			if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
			   	//product1a = INTERPOLATE (color2, color5);
			  	product1a = Interpolate_2xSaI(color5, color2);
			else
			if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
			   	//product1a = INTERPOLATE(color2, color5);
			  	product1a = Interpolate_2xSaI(color5, color2);
			else
			   	product1a = color5;


			*dP = product1a;
			*(dP+1) = product1b;
			*(dP+dline_pixels) = product2a;
			*(dP+dline_pixels+1) = product2b;

			bP += 1;
			dP += 2;

		}
		srcPtr += sline_pixels;
		dstPtr += 2*dline_pixels;
	};
}

//
// SuperEagle Scaler
//
static void Scale_SuperEagle
(
	Pixel_type *source,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dest,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int scale_factor			// Scale Factor
)
{

	// Need to ensure that the update is alligned to 4 pixels - Colourless
	// The idea was to prevent artifacts from appearing, but it doesn't seem
	// to help
	/*
	{
		int sx = ((srcx-4)/4)*4;
		int ex = ((srcx+srcw+7)/4)*4;
		int sy = ((srcy-4)/4)*4;
		int ey = ((srcy+srch+7)/4)*4;

		if (sx < 0) sx = 0;
		if (sy < 0) sy = 0;
		if (ex > sline_pixels) ex = sline_pixels;
		if (ey > sheight) ey = sheight;

		srcx = sx;
		srcy = sy;
		srcw = ex - sx;
		srch = ey - sy;
	}
	*/

	Pixel_type *srcPtr = source + (srcx + srcy*sline_pixels);
	Pixel_type *dstPtr = dest + (2*srcy*dline_pixels + 2*srcx);

	if (srcx + srcw >= (int)sline_pixels)
	{
		srcw = sline_pixels - srcx;
	}

    int ybeforelast1 = sheight - 1 - srcy;
    int ybeforelast2 = sheight - 2 - srcy;
    int xbeforelast1 = sline_pixels - 1 - srcx;
    int xbeforelast2 = sline_pixels - 2 - srcx;

    for (int y = 0; y < srch; y++)
	{
		Pixel_type *bP = srcPtr;
		Pixel_type *dP = dstPtr;

		for (int x = 0; x < srcw; x++)
		{
           Pixel_type color4, color5, color6;
           Pixel_type color1, color2, color3;
		   Pixel_type colorA1, colorA2; // , colorA0, colorA3,
		   Pixel_type colorB1, colorB2; // , colorB0, colorB3,
		   Pixel_type colorS1, colorS2;
           Pixel_type product1a, product1b,
				product2a, product2b;

			//---------------------------------------  B0 B1 B2 B3
			//                                         4  5  6  S2
			//                                         1  2  3  S1
			//                                         A0 A1 A2 A3
			//--------------------------------------
			int add1, add2;
			int sub1;
			int nextl1, nextl2;
			int prevl1;

			if (x == 0)
				sub1 = 0;
			else
				sub1 = 1;

			if (x >= xbeforelast2)
				add2 = 0;
			else add2 = 1;

			if (x >= xbeforelast1)
				add1 = 0;
			else add1 = 1;

			if (y == 0)
				prevl1 = 0;
			else
				prevl1 = sline_pixels;

			if (y >= ybeforelast2)
				nextl2 = 0;
			else nextl2 = sline_pixels;

			if (y >= ybeforelast1)
				nextl1 = 0;
			else nextl1 = sline_pixels;


            //colorB0 = *(bP- prevl1 - sub1);
            colorB1 = *(bP- prevl1);
            colorB2 = *(bP- prevl1 + add1);
            //colorB3 = *(bP- prevl1 + add1 + add2);

            color4 = *(bP - sub1);
            color5 = *(bP);
            color6 = *(bP + add1);
            colorS2 = *(bP + add1 + add2);

            color1 = *(bP + nextl1 - sub1);
            color2 = *(bP + nextl1);
            color3 = *(bP + nextl1 + add1);
            colorS1 = *(bP + nextl1 + add1 + add2);

            //colorA0 = *(bP + nextl1 + nextl2 - sub1);
            colorA1 = *(bP + nextl1 + nextl2);
            colorA2 = *(bP + nextl1 + nextl2 + add1);
            //colorA3 = *(bP + nextl1 + nextl2 + add1 + add2);


			if (color2 == color6 && color5 != color3)
			{
			   product1b = product2a = color2;
			   product1b = product2a;


			   if ((color1 == color2) || (color6 == colorB2))
			   {
				   //product1a = INTERPOLATE (color2, color5);
				   //product1a = INTERPOLATE (color2, product1a);
				   product1a = QInterpolate_2xSaI(color2, color2, color2, color5);

			   }
			   else
			   {
				   //product1a = INTERPOLATE (color5, color6);
				   product1a = Interpolate_2xSaI(color6, color5);
			   }

			   if ((color6 == colorS2) || (color2 == colorA1))
               {
                   //product2b = INTERPOLATE (color2, color3);
                   //product2b = INTERPOLATE (color2, product2b);
				   product2b = QInterpolate_2xSaI(color2, color2, color2, color3);

               }
               else
               {
                   //product2b = INTERPOLATE (color2, color3);
				   product2b = Interpolate_2xSaI(color2, color3);
               }
            }
            else
            if (color5 == color3 && color2 != color6)
            {
               product2b = product1a = color5;
			   product2b = product1a;


               if ((colorB1 == color5) ||  (color3 == colorS1))
               {
                   //product1b = INTERPOLATE (color5, color6);
				   //product1b = INTERPOLATE (color5, product1b);
				   product1b = QInterpolate_2xSaI(color5, color5, color5, color6);
               }
               else
               {
                  //product1b = INTERPOLATE (color5, color6);
				  product1b = Interpolate_2xSaI(color5, color6);
               }

			   if ((color3 == colorA2) || (color4 == color5))
               {
                   //product2a = INTERPOLATE (color5, color2);
                   //product2a = INTERPOLATE (color5, product2a);
				   product2a = QInterpolate_2xSaI(color2, color5, color5, color5);
               }
               else
               {
                  //product2a = INTERPOLATE (color2, color3);
				  product2a = Interpolate_2xSaI(color3, color2);
               }

            }
            else
            if (color5 == color3 && color2 == color6)
            {
               int r = 0;

               //r += GetResult (color6, color5, color1, colorA1);
               //r += GetResult (color6, color5, color4, colorB1);
               //r += GetResult (color6, color5, colorA2, colorS1);
               //r += GetResult (color6, color5, colorB2, colorS2);
			   r += GetResult1 (color5, color6, color4, colorB1);
			   r += GetResult2 (color6, color5, colorA2, colorS1);
			   r += GetResult2 (color6, color5, color1, colorA1);
 			   r += GetResult1 (color5, color6, colorB2, colorS2);

               if (r > 0)
               {
                  product1b = product2a = color2;
                  //product1a = product2b = INTERPOLATE (color5, color6);
				  product1a = product2b = Interpolate_2xSaI(color5, color6);
               }
               else
               if (r < 0)
               {
                  product2b = product1a = color5;
                  //product1b = product2a = INTERPOLATE (color5, color6);
				  product1b = product2a = Interpolate_2xSaI(color5, color6);
               }
               else
               {
                  product2b = product1a = color5;
                  product1b = product2a = color2;
               }
            }
            else
            {
                  //product2b = product1a = INTERPOLATE (color2, color6);
                  //product2b = Q_INTERPOLATE (color3, color3, color3, product2b);
                  //product1a = Q_INTERPOLATE (color5, color5, color5, product1a);
				  product2b = OInterpolate_2xSaI(color3, color2, color6);
				  product1a = OInterpolate_2xSaI(color5, color6, color2);

                  //product2a = product1b = INTERPOLATE (color5, color3);
                  //product2a = Q_INTERPOLATE (color2, color2, color2, product2a);
                  //product1b = Q_INTERPOLATE (color6, color6, color6, product1b);
				  product2a = OInterpolate_2xSaI(color2, color5, color3);
				  product1b = OInterpolate_2xSaI(color6, color5, color3);
			}

			*dP = product1a;
			*(dP+1) = product1b;
			*(dP+dline_pixels) = product2a;
			*(dP+dline_pixels+1) = product2b;

			bP += 1;
			dP += 2;

		}
		srcPtr += sline_pixels;
		dstPtr += 2*dline_pixels;
	};
}



/**
 ** End of 2xSaI code
 **/


//
// Scale2X algorithm by Andrea Mazzoleni.
//
/* This file is part of the Scale2x project.
 *
 * Copyright (C) 2001-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
static void Scale_Scale2x
(
	Pixel_type *src,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dest,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int scale_factor			// Scale Factor
)
{
	dest += srcy*2*dline_pixels + srcx*2;
	Pixel_type *dest0 = dest, *dest1 = dest + dline_pixels;
					// ->current row.
	Pixel_type *src1 = src + srcy*sline_pixels + srcx;
	Pixel_type *src0 = src1 - sline_pixels;	// ->prev. row.
	Pixel_type *src2 = src1 + sline_pixels;	// ->next row.
	Pixel_type * limit_y = src1 + srch*sline_pixels;
	Pixel_type * limit_x = src1 + srcw;
					// Very end of source surface:
	Pixel_type * end_src = src + sheight*sline_pixels;

	if (src0 < src)
		src0 = src1;		// Don't go before row 0.
	if (srcx + srcw == sline_pixels)	// Going to right edge?
		limit_x--;		// Stop 1 pixel before it.
	while (src1 < limit_y)
		{
		if (src2 > end_src)
			src2 = src1;	// On last row.
		if (srcx == 0)		// First pixel.
			{
			dest0[0] = dest1[0] = src1[0];
			if (src1[1] == src0[0] && src2[0] != src0[0])
				dest0[1] = src0[0];
			else
				dest0[1] = src1[0];
			if (src1[1] == src2[0] && src0[0] != src2[0])
				dest1[1] = src2[0];
			else
				dest1[1] = src1[0];
			++src0; ++src1; ++src2;
			dest0 += 2; dest1 += 2;
			}
					// Middle pixels.
		while (src1 < limit_x)
			{
			if (src1[-1] == src0[0] && src2[0] != src0[0] &&
			    src1[1] != src0[0])
				dest0[0] = src0[0];
			else
				dest0[0] = src1[0];
			if (src1[1] == src0[0] && src2[0] != src0[0] &&
			    src1[-1] != src0[0])
				dest0[1] = src0[0];
			else
				dest0[1] = src1[0];
			if (src1[-1] == src2[0] && src0[0] != src2[0] &&
			    src1[1] != src2[0])
				dest1[0] = src2[0];
			else
				dest1[0] = src1[0];
			if (src1[1] == src2[0] && src0[0] != src2[0] &&
			    src1[-1] != src2[0])
				dest1[1] = src2[0];
			else
				dest1[1] = src1[0];
			++src0; ++src1; ++src2;
			dest0 += 2; dest1 += 2;
			}
		if (srcx + srcw == sline_pixels)
			{		// End pixel in row.
			if (src1[-1] == src0[0] && src2[0] != src0[0])
				dest0[0] = src0[0];
			else
				dest0[0] = src1[0];
			if (src1[-1] == src2[0] && src0[0] != src2[0])
				dest1[0] = src2[0];
			else
				dest1[0] = src1[0];
			dest0[1] = src1[0];
			dest1[1] = src1[0];
			++src0; ++src1; ++src2;
			dest0 += 2; dest1 += 2;
			}
		src0 += sline_pixels - srcw;
		src1 += sline_pixels - srcw;
		src2 += sline_pixels - srcw;
		dest1 += dline_pixels - 2*srcw;
		if (src0 == src1)	// End of first row?
			src0 -= sline_pixels;
		limit_x += sline_pixels;
		dest0 = dest1;
		dest1 += dline_pixels;
		}
}



typedef unsigned int COMPONENT;


// fill `row' with the the disassembled color components from the original
// pixel values in `from'; if we run out of source pixels, just keep copying
// the last one we got
static inline void fill_rgb_row
	(
	 Pixel_type *from,
	 int src_width,	// number of pixels to read from 'from'
	 COMPONENT *row,
	 int width		// number of pixels to write into 'row'
	 )
{
	COMPONENT *copy_start = row + src_width*3;
	COMPONENT *all_stop = row + width*3;
	while (row < copy_start)
		{
		COMPONENT& r = *row++;
		COMPONENT& g = *row++;
		COMPONENT& b = *row++;
		Manip_pixels::split_col(*from++, r, g, b);
		}
	// any remaining elements to be written to 'row' are a replica of the
	// preceding pixel
	COMPONENT *p = row-3;
	while (row < all_stop) {
		// we're guaranteed three elements per pixel; could unroll the loop
		// further, especially with a Duff's Device, but the gains would be
		// probably limited (judging by profiler output)
		*row++ = *p++;
		*row++ = *p++;
		*row++ = *p++;
	}
}

//
// Bilinear Scaler
//
static void Scale_Bilinear
(
	Pixel_type *source,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dest,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int scale_factor			// Scale Factor
)
{
	Pixel_type *from = source + srcy*sline_pixels + srcx;
	Pixel_type *to = dest + 2*srcy*dline_pixels + 2*srcx;
	Pixel_type *to_odd = to + dline_pixels;

	// the following are static because we don't want to be freeing and
	// reallocating space on each call, as malloc()s are usually very
	// expensive; we do allow it to grow though
	static int buff_size = 0;
	static COMPONENT *rgb_row_cur  = 0;
	static COMPONENT *rgb_row_next = 0;
	if (buff_size < sline_pixels+1) {
		delete [] rgb_row_cur;
		delete [] rgb_row_next;
		buff_size = sline_pixels+1;
		rgb_row_cur  = new COMPONENT[buff_size*3];
		rgb_row_next = new COMPONENT[buff_size*3];
	}

	int from_width = sline_pixels - srcx;
	if (srcw+1 < from_width)
		from_width = srcw+1;

	fill_rgb_row(from, from_width, rgb_row_cur, srcw+1);

	for (int y=0; y < srch; y++)
		{
		Pixel_type *from_orig = from;
		Pixel_type *to_orig = to;

		if (y+1 < sheight)
			fill_rgb_row(from+sline_pixels, from_width, rgb_row_next,
						 srcw+1);
		else
			fill_rgb_row(from, from_width, rgb_row_next, srcw+1);

		// every pixel in the src region, is extended to 4 pixels in the
		// destination, arranged in a square 'quad'; if the current src
		// pixel is 'a', then in what follows 'b' is the src pixel to the
		// right, 'c' is the src pixel below, and 'd' is the src pixel to
		// the right and down
		COMPONENT *cur_row  = rgb_row_cur;
		COMPONENT *next_row = rgb_row_next;
		COMPONENT *ar = cur_row++;
		COMPONENT *ag = cur_row++;
		COMPONENT *ab = cur_row++;
		COMPONENT *cr = next_row++;
		COMPONENT *cg = next_row++;
		COMPONENT *cb = next_row++;
		for (int x=0; x < srcw; x++)
			{
			COMPONENT *br = cur_row++;
			COMPONENT *bg = cur_row++;
			COMPONENT *bb = cur_row++;
			COMPONENT *dr = next_row++;
			COMPONENT *dg = next_row++;
			COMPONENT *db = next_row++;

			// upper left pixel in quad: just copy it in
			*to++ = Manip_pixels::rgb(*ar, *ag, *ab);

			// upper right
			*to++ = Manip_pixels::rgb((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);

			// lower left
			*to_odd++ = Manip_pixels::rgb((*ar+*cr)>>1, (*ag+*cg)>>1, (*ab+*cb)>>1);

			// lower right
			*to_odd++ = Manip_pixels::rgb((*ar+*br+*cr+*dr)>>2,
					  		      (*ag+*bg+*cg+*dg)>>2,
							      (*ab+*bb+*cb+*db)>>2);

			// 'b' becomes 'a', 'd' becomes 'c'
			ar = br;
			ag = bg;
			ab = bb;
			cr = dr;
			cg = dg;
			cb = db;
			}

		// the "next" rgb row becomes the current; the old current rgb row is
		// recycled and serves as the new "next" row
		COMPONENT *temp;
		temp = rgb_row_cur;
		rgb_row_cur = rgb_row_next;
		rgb_row_next = temp;

		// update the pointers for start of next pair of lines
		from = from_orig + sline_pixels;
		to = to_orig + 2*dline_pixels;
		to_odd = to + dline_pixels;
		}
}

//
// BilinearInterlaced Scaler
//
static void Scale_BilinearInterlaced
(
	Pixel_type *source,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dest,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int scale_factor			// Scale Factor
)
{
	Pixel_type *from = source + srcy*sline_pixels + srcx;
	Pixel_type *to = dest + 2*srcy*dline_pixels + 2*srcx;

	// the following are static because we don't want to be freeing and
	// reallocating space on each call, as malloc()s are usually very
	// expensive; we do allow it to grow though
	static int buff_size = 0;
	static COMPONENT *rgb_row_cur  = 0;
	if (buff_size < sline_pixels+1) {
		delete [] rgb_row_cur;
		buff_size = sline_pixels+1;
		rgb_row_cur  = new COMPONENT[buff_size*3];
	}

	int from_width = sline_pixels - srcx;
	if (srcw+1 < from_width)
		from_width = srcw+1;

	for (int y=0; y < srch; y++)
	{
		Pixel_type *from_orig = from;
		Pixel_type *to_orig = to;

		fill_rgb_row(from, from_width, rgb_row_cur, srcw+1);

		// every pixel in the src region, is extended to 4 pixels in the
		// destination, arranged in a square 'quad'; if the current src
		// pixel is 'a', then in what follows 'b' is the src pixel to the
		// right, 'c' is the src pixel below, and 'd' is the src pixel to
		// the right and down
		COMPONENT *cur_row  = rgb_row_cur;
		COMPONENT *ar = cur_row++;
		COMPONENT *ag = cur_row++;
		COMPONENT *ab = cur_row++;
		for (int x=0; x < srcw; x++)
		{
			COMPONENT *br = cur_row++;
			COMPONENT *bg = cur_row++;
			COMPONENT *bb = cur_row++;

			// upper left pixel in quad: just copy it in
			*to++ = Manip_pixels::rgb(*ar, *ag, *ab);

			// upper right
			*to++ = Manip_pixels::rgb((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);

			// 'b' becomes 'a', 'd' becomes 'c'
			ar = br;
			ag = bg;
			ab = bb;
		}

		// update the pointers for start of next pair of lines
		from = from_orig + sline_pixels;
		to = to_orig + 2*dline_pixels;
	}
}

//
// BilinearHalfInterlaced Scaler
//
static void Scale_BilinearHalfInterlaced
(
	Pixel_type *source,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dest,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int scale_factor			// Scale Factor
)
{
	Pixel_type *from = source + srcy*sline_pixels + srcx;
	Pixel_type *to = dest + 2*srcy*dline_pixels + 2*srcx;
	Pixel_type *to_odd = to + dline_pixels;

	// the following are static because we don't want to be freeing and
	// reallocating space on each call, as malloc()s are usually very
	// expensive; we do allow it to grow though
	static int buff_size = 0;
	static COMPONENT *rgb_row_cur  = 0;
	static COMPONENT *rgb_row_next = 0;
	if (buff_size < sline_pixels+1) {
		delete [] rgb_row_cur;
		delete [] rgb_row_next;
		buff_size = sline_pixels+1;
		rgb_row_cur  = new COMPONENT[buff_size*3];
		rgb_row_next = new COMPONENT[buff_size*3];
	}

	int from_width = sline_pixels - srcx;
	if (srcw+1 < from_width)
		from_width = srcw+1;

	fill_rgb_row(from, from_width, rgb_row_cur, srcw+1);

	for (int y=0; y < srch; y++)
		{
		Pixel_type *from_orig = from;
		Pixel_type *to_orig = to;

		if (y+1 < sheight)
			fill_rgb_row(from+sline_pixels, from_width, rgb_row_next,
						 srcw+1);
		else
			fill_rgb_row(from, from_width, rgb_row_next, srcw+1);

		// every pixel in the src region, is extended to 4 pixels in the
		// destination, arranged in a square 'quad'; if the current src
		// pixel is 'a', then in what follows 'b' is the src pixel to the
		// right, 'c' is the src pixel below, and 'd' is the src pixel to
		// the right and down
		COMPONENT *cur_row  = rgb_row_cur;
		COMPONENT *next_row = rgb_row_next;
		COMPONENT *ar = cur_row++;
		COMPONENT *ag = cur_row++;
		COMPONENT *ab = cur_row++;
		COMPONENT *cr = next_row++;
		COMPONENT *cg = next_row++;
		COMPONENT *cb = next_row++;
		for (int x=0; x < srcw; x++)
			{
			COMPONENT *br = cur_row++;
			COMPONENT *bg = cur_row++;
			COMPONENT *bb = cur_row++;
			COMPONENT *dr = next_row++;
			COMPONENT *dg = next_row++;
			COMPONENT *db = next_row++;

			// upper left pixel in quad: just copy it in
			*to++ = Manip_pixels::rgb(*ar, *ag, *ab);

			// upper right
			*to++ = Manip_pixels::rgb((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);

			// lower left
			*to_odd++ = Manip_pixels::rgb((*ar+*cr)>>2, (*ag+*cg)>>2, (*ab+*cb)>>2);

			// lower right
			*to_odd++ = Manip_pixels::rgb((*ar+*br+*cr+*dr)>>3,
					  		      (*ag+*bg+*cg+*dg)>>3,
							      (*ab+*bb+*cb+*db)>>3);

			// 'b' becomes 'a', 'd' becomes 'c'
			ar = br;
			ag = bg;
			ab = bb;
			cr = dr;
			cg = dg;
			cb = db;
			}

		// the "next" rgb row becomes the current; the old current rgb row is
		// recycled and serves as the new "next" row
		COMPONENT *temp;
		temp = rgb_row_cur;
		rgb_row_cur = rgb_row_next;
		rgb_row_next = temp;

		// update the pointers for start of next pair of lines
		from = from_orig + sline_pixels;
		to = to_orig + 2*dline_pixels;
		to_odd = to + dline_pixels;
		}
}

//
// BilinearQuarterInterlaced Scaler
//
static void Scale_BilinearQuarterInterlaced
(
	Pixel_type *source,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dest,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int scale_factor			// Scale Factor
)
{
	Pixel_type *from = source + srcy*sline_pixels + srcx;
	Pixel_type *to = dest + 2*srcy*dline_pixels + 2*srcx;
	Pixel_type *to_odd = to + dline_pixels;

	// the following are static because we don't want to be freeing and
	// reallocating space on each call, as malloc()s are usually very
	// expensive; we do allow it to grow though
	static int buff_size = 0;
	static COMPONENT *rgb_row_cur  = 0;
	static COMPONENT *rgb_row_next = 0;
	if (buff_size < sline_pixels+1) {
		delete [] rgb_row_cur;
		delete [] rgb_row_next;
		buff_size = sline_pixels+1;
		rgb_row_cur  = new COMPONENT[buff_size*3];
		rgb_row_next = new COMPONENT[buff_size*3];
	}

	int from_width = sline_pixels - srcx;
	if (srcw+1 < from_width)
		from_width = srcw+1;

	fill_rgb_row(from, from_width, rgb_row_cur, srcw+1);

	for (int y=0; y < srch; y++)
		{
		Pixel_type *from_orig = from;
		Pixel_type *to_orig = to;

		if (y+1 < sheight)
			fill_rgb_row(from+sline_pixels, from_width, rgb_row_next,
						 srcw+1);
		else
			fill_rgb_row(from, from_width, rgb_row_next, srcw+1);

		// every pixel in the src region, is extended to 4 pixels in the
		// destination, arranged in a square 'quad'; if the current src
		// pixel is 'a', then in what follows 'b' is the src pixel to the
		// right, 'c' is the src pixel below, and 'd' is the src pixel to
		// the right and down
		COMPONENT *cur_row  = rgb_row_cur;
		COMPONENT *next_row = rgb_row_next;
		COMPONENT *ar = cur_row++;
		COMPONENT *ag = cur_row++;
		COMPONENT *ab = cur_row++;
		COMPONENT *cr = next_row++;
		COMPONENT *cg = next_row++;
		COMPONENT *cb = next_row++;
		for (int x=0; x < srcw; x++)
			{
			COMPONENT *br = cur_row++;
			COMPONENT *bg = cur_row++;
			COMPONENT *bb = cur_row++;
			COMPONENT *dr = next_row++;
			COMPONENT *dg = next_row++;
			COMPONENT *db = next_row++;

			// upper left pixel in quad: just copy it in
			*to++ = Manip_pixels::rgb(*ar, *ag, *ab);

			// upper right
			*to++ = Manip_pixels::rgb((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);

			// lower left
			*to_odd++ = Manip_pixels::rgb(
				((*ar+*cr)+((*ar+*cr)<<1))>>3,
				((*ag+*cg)+((*ag+*cg)<<1))>>3,
				((*ab+*cb)+((*ab+*cb)<<1))>>3
			);

			// lower right
			*to_odd++ = Manip_pixels::rgb(
				((*ar+*br+*cr+*dr)+((*ar+*br+*cr+*dr)<<1))>>4,
				((*ag+*bg+*cg+*dg)+((*ag+*bg+*cg+*dg)<<1))>>4,
				((*ab+*bb+*cb+*db)+((*ab+*bb+*cb+*db)<<1))>>4
			);

			// 'b' becomes 'a', 'd' becomes 'c'
			ar = br;
			ag = bg;
			ab = bb;
			cr = dr;
			cg = dg;
			cb = db;
			}

		// the "next" rgb row becomes the current; the old current rgb row is
		// recycled and serves as the new "next" row
		COMPONENT *temp;
		temp = rgb_row_cur;
		rgb_row_cur = rgb_row_next;
		rgb_row_next = temp;

		// update the pointers for start of next pair of lines
		from = from_orig + sline_pixels;
		to = to_orig + 2*dline_pixels;
		to_odd = to + dline_pixels;
		}
}

//
// BilinearPlus Scaler
//
static void Scale_BilinearPlus
(
	Pixel_type *source,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dest,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int scale_factor			// Scale Factor
)
{
	Pixel_type *from = source + srcy*sline_pixels + srcx;
	Pixel_type *to = dest + 2*srcy*dline_pixels + 2*srcx;
	Pixel_type *to_odd = to + dline_pixels;

	// the following are static because we don't want to be freeing and
	// reallocating space on each call, as malloc()s are usually very
	// expensive; we do allow it to grow though
	static int buff_size = 0;
	static COMPONENT *rgb_row_cur  = 0;
	static COMPONENT *rgb_row_next = 0;
	if (buff_size < sline_pixels+1) {
		delete [] rgb_row_cur;
		delete [] rgb_row_next;
		buff_size = sline_pixels+1;
		rgb_row_cur  = new COMPONENT[buff_size*3];
		rgb_row_next = new COMPONENT[buff_size*3];
	}

	int from_width = sline_pixels - srcx;
	if (srcw+1 < from_width)
		from_width = srcw+1;

	fill_rgb_row(from, from_width, rgb_row_cur, srcw+1);

	for (int y=0; y < srch; y++)
		{
		Pixel_type *from_orig = from;
		Pixel_type *to_orig = to;

		if (y+1 < sheight)
			fill_rgb_row(from+sline_pixels, from_width, rgb_row_next,
						 srcw+1);
		else
			fill_rgb_row(from, from_width, rgb_row_next, srcw+1);

		// every pixel in the src region, is extended to 4 pixels in the
		// destination, arranged in a square 'quad'; if the current src
		// pixel is 'a', then in what follows 'b' is the src pixel to the
		// right, 'c' is the src pixel below, and 'd' is the src pixel to
		// the right and down
		COMPONENT *cur_row  = rgb_row_cur;
		COMPONENT *next_row = rgb_row_next;
		COMPONENT *ar = cur_row++;
		COMPONENT *ag = cur_row++;
		COMPONENT *ab = cur_row++;
		COMPONENT *cr = next_row++;
		COMPONENT *cg = next_row++;
		COMPONENT *cb = next_row++;
		for (int x=0; x < srcw; x++)
			{
			COMPONENT *br = cur_row++;
			COMPONENT *bg = cur_row++;
			COMPONENT *bb = cur_row++;
			COMPONENT *dr = next_row++;
			COMPONENT *dg = next_row++;
			COMPONENT *db = next_row++;

			// upper left pixel in quad: just copy it in
			//*to++ = Manip_pixels::rgb(*ar, *ag, *ab);
#ifdef USE_ORIGINAL_BILINEAR_PLUS
			*to++ = Manip_pixels::rgb(
			(((*ar)<<2) +((*ar)) + (*cr+*br+*br) )>> 3,
			(((*ag)<<2) +((*ag)) + (*cg+*bg+*bg) )>> 3,
			(((*ab)<<2) +((*ab)) + (*cb+*bb+*bb) )>> 3);
#else
			*to++ = Manip_pixels::rgb(
			(((*ar)<<3) +((*ar)<<1) + (*cr+*br+*br+*cr) )>> 4,
			(((*ag)<<3) +((*ag)<<1) + (*cg+*bg+*bg+*cg) )>> 4,
			(((*ab)<<3) +((*ab)<<1) + (*cb+*bb+*bb+*cb) )>> 4);
#endif

			// upper right
			*to++ = Manip_pixels::rgb((*ar+*br)>>1, (*ag+*bg)>>1, (*ab+*bb)>>1);

			// lower left
			*to_odd++ = Manip_pixels::rgb((*ar+*cr)>>1, (*ag+*cg)>>1, (*ab+*cb)>>1);

			// lower right
			*to_odd++ = Manip_pixels::rgb((*ar+*br+*cr+*dr)>>2,
					  		      (*ag+*bg+*cg+*dg)>>2,
							      (*ab+*bb+*cb+*db)>>2);

			// 'b' becomes 'a', 'd' becomes 'c'
			ar = br;
			ag = bg;
			ab = bb;
			cr = dr;
			cg = dg;
			cb = db;
			}

		// the "next" rgb row becomes the current; the old current rgb row is
		// recycled and serves as the new "next" row
		COMPONENT *temp;
		temp = rgb_row_cur;
		rgb_row_cur = rgb_row_next;
		rgb_row_next = temp;

		// update the pointers for start of next pair of lines
		from = from_orig + sline_pixels;
		to = to_orig + 2*dline_pixels;
		to_odd = to + dline_pixels;
		}
}

//
// Point Sampling Scaler
//
static void Scale_point
(
	Pixel_type *src,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dst,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int factor					// Scale Factor
)
{
	static Pixel_type *dest;
	static const Pixel_type *source;
	static const Pixel_type *limit_y;
	static const Pixel_type *limit_x;
	static int pitch_src;
	static int add_dst;

	source = src + srcy*sline_pixels + srcx;
	dest = dst + srcy*factor*dline_pixels + srcx*factor;

	limit_y = source + srch*sline_pixels;
	limit_x = source + srcw;

	pitch_src = sline_pixels;
	add_dst = dline_pixels - srcw*factor;

	// Slightly Optimzed 16 bit 2x
	if (factor == 2 && sizeof(Pixel_type) == 2) {
		static Pixel_type *dest2;
		uint32 data;
		static int add_src;
		add_src = pitch_src - srcw;
		while (source < limit_y)
		{
			dest2 = dest;
			dest += dline_pixels;

			while (source < limit_x)
			{
				data = *source++;
				data |= data << 16;
				*(uint32*)dest2 = data;
				dest2+=2;
				*(uint32*)dest = data;
				dest+=2;
			}
			dest += add_dst;
			limit_x += sline_pixels;
			source += add_src;
		}
	}
	// Slightly Optimzed 32 bit 2x
	else if (factor == 2) {
		Pixel_type data;
		static Pixel_type *dest2;
		static int add_src;
		add_src = pitch_src - srcw;
		while (source < limit_y)
		{
			dest2 = dest;
			dest += dline_pixels;

			while (source < limit_x)
			{
				data = *source++;
				*dest2++ = data;
				*dest2++ = data;
				*dest++ = data;
				*dest++ = data;
			}
			dest += add_dst;
			limit_x += sline_pixels;
			source += add_src;
		}
	}
	else
	{
		Pixel_type data;
		static unsigned int src_sub;
		static unsigned int scale_factor;
		static unsigned int dline_pixels_scaled;
		static const Pixel_type * limit_y2;
		static const Pixel_type * limit_x2;

		src_sub = srcw;
		scale_factor = factor;
		dline_pixels_scaled = dline_pixels*scale_factor;
		limit_y2 = dest;

		while (source < limit_y)
		{
			limit_y2 += dline_pixels_scaled;
			while (dest < limit_y2)
			{
				limit_x2 = dest;
				while (source < limit_x)
				{
					data = *source++;
					limit_x2 += scale_factor;
					while (dest < limit_x2) *dest++ = data;
				}
				dest += add_dst;
				source -= src_sub;
			}
			limit_x += pitch_src;
			source += pitch_src;
		}
	}
}

//
// Interlaced Scaler
//
static void Scale_interlaced
(
	Pixel_type *src,			// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int sline_pixels,		// Pixels (words)/line for source.
	const int sheight,			// Source height.
	Pixel_type *dst,			// ->dest pixels.
	const int dline_pixels,		// Pixels (words)/line for dest.
	int factor					// Scale Factor
)
{
	static Pixel_type *dest;
	static const Pixel_type *source;
	static const Pixel_type *limit_y;
	static const Pixel_type *limit_x;
	static int pitch_src;
	static int add_dst;

	source = src + srcy*sline_pixels + srcx;
	dest = dst + srcy*factor*dline_pixels + srcx*factor;

	limit_y = source + srch*sline_pixels;
	limit_x = source + srcw;

	pitch_src = sline_pixels;
	add_dst = dline_pixels - srcw*factor;

	// Slightly Optimzed 16 bit 2x
	if (factor == 2 && sizeof(Pixel_type) == 2) {
		uint32 data;
		static int add_src;
		add_src = pitch_src - srcw;
		add_dst += dline_pixels;
		while (source < limit_y)
		{
			while (source < limit_x)
			{
				data = *source++;
				*(uint32*)dest = data | (data << 16);
				dest+=2;
			}
			dest += add_dst;
			limit_x += sline_pixels;
			source += add_src;
		}
	}
	// Slightly Optimzed 32 bit 2x
	else if (factor == 2) {
		Pixel_type data;
		static int add_src;
		add_src = pitch_src - srcw;
		add_dst += dline_pixels;
		while (source < limit_y)
		{
			while (source < limit_x)
			{
				data = *source++;
				*dest++ = data;
				*dest++ = data;
			}
			dest += add_dst;
			limit_x += sline_pixels;
			source += add_src;
		}
	}
	else
	{
		Pixel_type data;
		static unsigned int src_sub;
		static unsigned int scale_factor;
		static unsigned int dline_pixels_scaled;
		static unsigned int	skipped;
		static const Pixel_type * limit_y2;
		static const Pixel_type * limit_x2;

		src_sub = srcw;
		scale_factor = factor;
		dline_pixels_scaled = dline_pixels*scale_factor;
		limit_y2 = dest;
		skipped = (srcy*scale_factor)%2;

		while (source < limit_y)
		{
			limit_y2 += dline_pixels_scaled;
			while (dest < limit_y2)
			{
				limit_x2 = dest;
				if (!skipped) {
					while (source < limit_x)
					{
						data = *source++;
						limit_x2 += scale_factor;
						while (dest < limit_x2) *dest++ = data;
					}
					dest += add_dst;
					source -= src_sub;
				}
				else {
					dest += dline_pixels;
				}

				skipped = 1-skipped;
			}
			limit_x += pitch_src;
			source += pitch_src;
		}
	}
}

};	// End of class Scalers


#if 0

// Colourless's Experimental BlurFilter
// Note this doesn't actually scale, it just blurs
template <class Pixel_type, class Manip_pixels>
void BlurFilter
	(
	Pixel_type *source,		// ->source pixels.
	int srcx, int srcy,			// Start of rectangle within src.
	int srcw, int srch,			// Dims. of rectangle.
	const int swidth,			// Source width.
	const int sheight,			// Source height.
	int spitch,					// Pixels/line for source.
	Pixel_type *dest,			// ->dest pixels.
	int dpitch					// Pixels/line for dest.
	)
{
/*
	What we do is combine 9 source pixels into 1 destination pixel. It's just a
	simple blur using these weights, then scaled by 16. In a sense this has the
	same effect as super sampling with bilinear filtering

	Normal Pixel
    +--+--+--+
	| 1| 2| 1|
    +--+--+--+
	| 2| 4| 2|
    +--+--+--+
	| 1| 2| 1|
    +--+--+--+

	Left/Right Edge Pixel
    +--+--+
	| 1| 1|
    +--+--+
	| 2| 2|
    +--+--+
	| 1| 1|
    +--+--+

	Top/Bottom Edge Pixel
    +--+--+--+
	| 1| 2| 1|
    +--+--+--+
	| 1| 2| 1|
    +--+--+--+

	Corner Pixel
    +--+--+
	| 1| 1|
    +--+--+
	| 1| 1|
    +--+--+
*/

#define add_source(src,ar,ag,ab) split_col(src,r,g,b); ar+=r; ag+=g; ab+=b;
#define add_shift(src,ar,ag,ab,shift) split_col(src,r,g,b); ar+=r<<shift; ag+=g<<shift; ab+=b<<shift;

	// Number to add to source at end of line
	uint32 source_add = spitch - srcw;
	uint32 dest_add = dpitch - srcw;

	// Set true if first and last column handling is required
	const bool first_col = srcx == 0;
	const bool last_col = srcx+srcw == swidth;

	// Pointers to end of source, and start of last line
	const Pixel_type *last = source + (srcy+sheight-1)*spitch;
	const Pixel_type *end = source + (srcy+srch-1)*spitch+ srcx+srcw;

	// Pointer to the end of the line
	const Pixel_type *end_of_line = source + srcy*spitch + srcx+srcw;
	if (last_col) end_of_line--;

	// Read buffers
	COMPONENT r;
	COMPONENT g;
	COMPONENT b;

	// Accumulators
	COMPONENT ar;
	COMPONENT ag;
	COMPONENT ab;

	// Point buffers to first pixel
	const Pixel_type *sourceP = source + (srcy-1)*spitch + srcx;	// Previous line
	const Pixel_type *sourceN = source + (srcy+1)*spitch + srcx;	// Next line
	source += srcy*spitch + srcx;
	dest += srcy*dpitch + srcx;

	// Special handling for first line
	if (srcy == 0) {

		// Special Handling for first column
		if (first_col) {
			Manip_pixels::split_col(*source,      ar, ag, ab);
			Manip_pixels::add_source  (*++source,    ar, ag, ab);
			Manip_pixels::add_source  (*sourceN,     ar, ag, ab);
			Manip_pixels::add_source  (*++sourceN,   ar, ag, ab);
			*dest++ = Manip_pixels::rgb(ar>>2, ag>>2, ab>>2);
		}

		// Handle all normal pixels
		while (source < end_of_line) {
			Manip_pixels::split_col(*(source-1),  ar, ag, ab);
			Manip_pixels::add_shift   (*source,      ar, ag, ab, 1);
			Manip_pixels::add_source  (*++source,    ar, ag, ab);
			Manip_pixels::add_source  (*(sourceN-1), ar, ag, ab);
			Manip_pixels::add_shift   (*sourceN,     ar, ag, ab, 1);
			Manip_pixels::add_source  (*++sourceN,   ar, ag, ab);
			*dest++ = Manip_pixels::rgb(ar>>3, ag>>3, ab>>3);
		}

		// Special Handling for last column
		if (last_col) {
			Manip_pixels::split_col(*(source-1),  ar, ag, ab);
			Manip_pixels::add_source  (*source,      ar, ag, ab);
			++source;
			Manip_pixels::add_source  (*sourceN,     ar, ag, ab);
			Manip_pixels::add_source  (*(sourceN-1), ar, ag, ab);
			++sourceN;
			*dest++ = Manip_pixels::rgb(ar>>2, ag>>2, ab>>2);
		}


		// Increment buffer pointers to the next line
		dest += dest_add;
		end_of_line += spitch;
		sourceP += spitch;
		source += source_add;
		sourceN += source_add;
	}


	// Do all normal lines
	while (source < last && source < end) {

		// Special Handling for first column
		if (first_col) {
			Manip_pixels::split_col(*sourceP,     ar, ag, ab);
			Manip_pixels::add_source  (*++sourceP,   ar, ag, ab);
			Manip_pixels::add_shift   (*source,      ar, ag, ab, 1);
			Manip_pixels::add_shift   (*++source,    ar, ag, ab, 1);
			Manip_pixels::add_source  (*sourceN,     ar, ag, ab);
			Manip_pixels::add_source  (*++sourceN,   ar, ag, ab);
			*dest++ = Manip_pixels::rgb(ar>>3, ag>>3, ab>>3);
		}

		// Handle all normal pixels
		while (source < end_of_line) {
			Manip_pixels::split_col(*(sourceP-1), ar, ag, ab);
			Manip_pixels::add_shift   (*sourceP,     ar, ag, ab, 1);
			Manip_pixels::add_source  (*++sourceP,   ar, ag, ab);
			Manip_pixels::add_shift   (*(source-1),  ar, ag, ab, 1);
			Manip_pixels::add_shift   (*source,      ar, ag, ab, 2);
			Manip_pixels::add_shift   (*++source,    ar, ag, ab, 1);
			Manip_pixels::add_source  (*(sourceN-1), ar, ag, ab);
			Manip_pixels::add_shift   (*sourceN,     ar, ag, ab, 1);
			Manip_pixels::add_source  (*++sourceN,   ar, ag, ab);
			*dest++ = Manip_pixels::rgb(ar>>4, ag>>4, ab>>4);
		}

		// Special Handling for last column
		if (last_col) {
			Manip_pixels::split_col(*(sourceP-1), ar, ag, ab);
			Manip_pixels::add_source  (*sourceP,     ar, ag, ab);
			++sourceP;
			Manip_pixels::add_shift   (*(source-1),  ar, ag, ab, 1);
			Manip_pixels::add_shift   (*source,      ar, ag, ab, 1);
			++source;
			Manip_pixels::add_source  (*(sourceN-1), ar, ag, ab);
			Manip_pixels::add_source  (*sourceN,     ar, ag, ab);
			++sourceN;
			*dest++ = Manip_pixels::rgb(ar>>3, ag>>3, ab>>3);
		}

		// Increment buffer pointers to the next line
		dest += dest_add;
		end_of_line += spitch;
		sourceP += source_add;
		source += source_add;
		sourceN += source_add;
	}


	// Special handling for last line
	if (srcy+srch == sheight) {

		// Special Handling for first column
		if (first_col) {
			Manip_pixels::split_col(*source,    ar, ag, ab);
			Manip_pixels::add_source  (*++source,  ar, ag, ab);
			Manip_pixels::add_source  (*sourceP,   ar, ag, ab);
			Manip_pixels::add_source  (*++sourceP, ar, ag, ab);
			*dest++ = Manip_pixels::rgb(ar>>2, ag>>2, ab>>2);
		}

		// Handle all normal pixels
		while (source < end_of_line) {
			Manip_pixels::split_col(*(source-1),  ar, ag, ab);
			Manip_pixels::add_shift   (*source,      ar, ag, ab, 1);
			Manip_pixels::add_source  (*++source,    ar, ag, ab);
			Manip_pixels::add_source  (*(sourceP-1), ar, ag, ab);
			Manip_pixels::add_shift   (*sourceP,     ar, ag, ab, 1);
			Manip_pixels::add_source  (*++sourceP,   ar, ag, ab);
			*dest++ = Manip_pixels::rgb(ar>>3, ag>>3, ab>>3);
		}


		// Special Handling for last column
		if (last_col) {
			Manip_pixels::split_col(*(source-1),  ar, ag, ab);
			Manip_pixels::add_source  (*source,      ar, ag, ab);
			++source;
			Manip_pixels::add_source  (*sourceP,     ar, ag, ab);
			Manip_pixels::add_source  (*(sourceP-1), ar, ag, ab);
			++sourceP;
			*dest++ = Manip_pixels::rgb(ar>>2, ag>>2, ab>>2);
		}
	}
}

// Rotator
template <class Pixel_type, class Manip_pixels>
void Rotator
	(
	Pixel_type *source,			// ->source pixels.
	const int srcx, const int srcy,	// Start of rectangle within src.
	const int srcw, const int srch,	// Dims. of rectangle.
	const int swidth,				// Source width.
	const int sheight,				// Source height.
	const int spitch,				// Pixels/line for source.
	Pixel_type *dest,				// ->dest pixels.
	const int dincx,				// Amount to increment for each x pixel (can be negetive)
	const int dincy					// Pixels/line for dest. (amount to increment for each y pixel)
	)
{

	// Number to add to source at end of line
	const uint32 source_add = spitch - srcw;
	const sint32 dest_add = dincy - srcw*dincx;

	// Pointers to end of source, and start of last line
	const Pixel_type *end = source + (srcy+srch)*spitch;

	// Pointer to the end of the line
	const Pixel_type *end_of_line = source + srcy*spitch + srcx+srcw;

	// Point buffers to first pixel
	source += srcy*spitch + srcx;
	dest += srcy*dincy + srcx*dincx;

	int y = srcy;
	// Do all normal lines
	while (source < end) {

		// Handle all normal pixels
		while (source < end_of_line) {
			dest = *source++;
			dest += dincx;
		}

		// Increment buffer pointers to the next line
		source += source_add;
		dest += dest_add;
		end_of_line += spitch;
	}
}

#endif

} // End of namespace Nuvie
} // End of namespace Ultima

#endif // SCALE_INL_INCLUDED
