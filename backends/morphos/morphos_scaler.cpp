/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM Team
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "engine.h"

#include <exec/types.h>
#include <cybergraphics/cybergraphics.h>

#include <proto/cybergraphics.h>

#include "morphos.h"
#include "morphos_scaler.h"

#define GET_RESULT(A, B, C, D) ((A != C || A != D) - (B != C || B != D))
#define INTERPOLATE(A, B) (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask))
#define Q_INTERPOLATE(A, B, C, D) ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2) + ((((A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask)) >> 2) & qlowpixelMask)
#define SWAP_WORD(word) word = ((word & 0xff) << 8) | (word >> 8)
#define SWAP_LONG(lng) lng = ((lng & 0xff) << 24) | ((lng & 0xff00) << 8) | ((lng & 0xff0000) >> 8) | (lng >> 24)

MorphOSScaler::GfxScaler MorphOSScaler::ScummScalers[11]
	=	{ { "none", 	   "normal",	  ST_NONE },
		  { "Point", 		"2x",			  ST_POINT },
		  { "AdvMame2x",  "advmame2x",  ST_ADVMAME2X },
		  { "SuperEagle", "supereagle", ST_SUPEREAGLE },
		  { "Super2xSaI", "super2xsai", ST_SUPER2XSAI },
		  { NULL, NULL, ST_INVALID },
		  { NULL, NULL, ST_INVALID },
		  { NULL, NULL, ST_INVALID },
		  { NULL, NULL, ST_INVALID },
		  { NULL, NULL, ST_INVALID },
		  // This is the end marker ... do not assign a scaler to it!
		  { NULL, NULL, ST_INVALID }
		};

MorphOSScaler::MorphOSScaler(APTR buffer, int width, int height, ULONG *col_table, UWORD *col_table16, BitMap *bmap)
{
	dest = NULL;
	handle = NULL;

	Buffer = buffer;
	BufferWidth = width;
	BufferHeight = height;

	ScummColors = col_table;
	ScummColors16 = col_table16;

	/* Initialize scaling stuff */
	int minr, ming, minb;
	ULONG depth = GetCyberMapAttr(bmap, CYBRMATTR_DEPTH);

	if (depth > 16)
	{
		minr = 1 << 16;
		ming = 1 << 8;
		minb = 1;
	}
	else
	{
		minr = 1 << ((depth == 15) ? 10 : 11);
		ming = 1 << 5;
		minb = 1;
	}

	int pixfmt = GetCyberMapAttr(bmap, CYBRMATTR_PIXFMT);

	ScummPCMode = false;
	if (pixfmt == PIXFMT_RGB15PC || pixfmt == PIXFMT_BGR15PC ||
		pixfmt == PIXFMT_RGB16PC || pixfmt == PIXFMT_BGR16PC ||
		pixfmt == PIXFMT_BGRA32)
		ScummPCMode = true;
	
	colorMask = (MakeColor(pixfmt, 255, 0, 0) - minr) | (MakeColor(pixfmt, 0, 255, 0) - ming) | (MakeColor(pixfmt, 0, 0, 255) - minb);
	lowPixelMask = minr | ming | minb;
	qcolorMask = (MakeColor(pixfmt, 255, 0, 0) - 3*minr) | (MakeColor(pixfmt, 0, 255, 0) - 3*ming) | (MakeColor(pixfmt, 0, 0, 255) - 3*minb);
	qlowpixelMask = (minr * 3) | (ming * 3) | (minb * 3);
	redblueMask = MakeColor(pixfmt, 255, 0, 255);
	greenMask = MakeColor(pixfmt, 0, 255, 0);

	PixelsPerMask = (depth <= 16) ? 2 : 1;

	if (PixelsPerMask == 2)
	{
		colorMask |= (colorMask << 16);
		qcolorMask |= (qcolorMask << 16);
		lowPixelMask |= (lowPixelMask << 16);
		qlowpixelMask |= (qlowpixelMask << 16);
	}
}

MorphOSScaler::~MorphOSScaler()
{
	Finish();
}

MorphOSScaler *MorphOSScaler::Create(SCALERTYPE scaler_type, APTR buffer, int width, int height, ULONG *col_table, UWORD *col_table16, BitMap *bmap)
{
	MorphOSScaler *new_scaler = NULL;

	switch (scaler_type)
	{
		case ST_POINT:
			new_scaler = new PointScaler(buffer, width, height, col_table, col_table16, bmap);
			break;

		case ST_ADVMAME2X:
			new_scaler = new AdvMame2xScaler(buffer, width, height, col_table, col_table16, bmap);
			break;

		case ST_SUPEREAGLE:
			new_scaler = new SuperEagleScaler(buffer, width, height, col_table, col_table16, bmap);
			break;

		case ST_SUPER2XSAI:
			new_scaler = new Super2xSaIScaler(buffer, width, height, col_table, col_table16, bmap);
			break;

		default:
			warning("Invalid scaler requested - falling back to Super2xSaI");
			new_scaler = new Super2xSaIScaler(buffer, width, height, col_table, col_table16, bmap);
			break;
	}

	return new_scaler;
}

bool MorphOSScaler::Prepare(BitMap *render_bmap)
{
	handle = LockBitMapTags(render_bmap, LBMI_BYTESPERPIX, &dest_bpp,
										 LBMI_BYTESPERROW, &dest_pitch,
										 LBMI_BASEADDRESS, &dest,
										 LBMI_PIXFMT, &dest_pixfmt,
										 TAG_DONE);

	return handle != NULL;
}

void MorphOSScaler::Finish()
{
	if (handle)
	{
		UnLockBitMap(handle);
		handle = NULL;
	}
}

uint32 MorphOSScaler::MakeColor(int pixfmt, int r, int g, int b)
{
	uint32 col = 0;

	switch (pixfmt)
	{
		case PIXFMT_RGB15:
		case PIXFMT_RGB15PC:
			col = (((r*31)/255) << 10) | (((g*31)/255) << 5) | ((b*31)/255);
			break;

		case PIXFMT_BGR15:
		case PIXFMT_BGR15PC:
			col = (((b*31)/255) << 10) | (((g*31)/255) << 5) | ((r*31)/255);
			break;

		case PIXFMT_RGB16:
		case PIXFMT_RGB16PC:
			col = (((r*31)/255) << 11) | (((g*63)/255) << 5) | ((b*31)/255);
			break;

		case PIXFMT_BGR16:
		case PIXFMT_BGR16PC:
			col = (((b*31)/255) << 11) | (((g*63)/255) << 5) | ((r*31)/255);
			break;

		case PIXFMT_ARGB32:
		case PIXFMT_BGRA32:
			col = (r << 16) | (g << 8) | b;
			break;

		case PIXFMT_RGBA32:
			col = (r << 24) | (g << 16) | (b << 8);
			break;

		case PIXFMT_RGB24:
		case PIXFMT_BGR24:
			error("The scaling engines do not support 24 bit modes at the moment");
			break;

		default:
			error("Unsupported pixel format: %d. Please contact author at tomjoad@muenster.de", pixfmt);
	}

	return col;
}

void Super2xSaIScaler::Scale(uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height)
{
	unsigned int x, y;
	unsigned long color[16];
	byte *src;

	if (!handle)
		return;

	src = ((byte *)Buffer)+src_y*BufferWidth+src_x;

	/* Point to the first 3 lines. */
	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + BufferWidth;
	src_line[3] = src + BufferWidth * 2;

	dst_line[0] = dest+dest_y*2*dest_pitch+dest_x*2*dest_bpp;
	dst_line[1] = dst_line[0]+dest_pitch;

	if (PixelsPerMask == 2)
	{
		byte *sbp;
		sbp = src_line[0];
		color[0] = ScummColors16[*sbp];       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = color[0];   color[5] = color[0];   color[6] = ScummColors16[*(sbp+1)];  color[7] = ScummColors16[*(sbp+2)];
		sbp = src_line[2];
		color[8] = ScummColors16[*sbp];     color[9] = color[8];     color[10] = ScummColors16[*(sbp+1)]; color[11] = ScummColors16[*(sbp+2)];
		sbp = src_line[3];
		color[12] = ScummColors16[*sbp];    color[13] = color[12];   color[14] = ScummColors16[*(sbp+1)]; color[15] = ScummColors16[*(sbp+2)];
	}
	else
	{
		byte *lbp;
		lbp = src_line[0];
		color[0] = ScummColors[*lbp];       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = color[0];   color[5] = color[0];   color[6] = ScummColors[*(lbp+1)];  color[7] = ScummColors[*(lbp+2)];
		lbp = src_line[2];
		color[8] = ScummColors[*lbp];     color[9] = color[8];     color[10] = ScummColors[*(lbp+1)]; color[11] = ScummColors[*(lbp+2)];
		lbp = src_line[3];
		color[12] = ScummColors[*lbp];    color[13] = color[12];   color[14] = ScummColors[*(lbp+1)]; color[15] = ScummColors[*(lbp+2)];
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			unsigned long product1a, product1b, product2a, product2b;

//---------------------------------------  B0 B1 B2 B3    0  1  2  3
//                                         4  5* 6  S2 -> 4  5* 6  7
//                                         1  2  3  S1    8  9 10 11
//                                         A0 A1 A2 A3   12 13 14 15
//--------------------------------------
			if (color[9] == color[6] && color[5] != color[10])
			{
				product2b = color[9];
				product1b = product2b;
			}
			else if (color[5] == color[10] && color[9] != color[6])
			{
				product2b = color[5];
				product1b = product2b;
			}
			else if (color[5] == color[10] && color[9] == color[6])
			{
				int r = 0;

				r += GET_RESULT(color[6], color[5], color[8], color[13]);
				r += GET_RESULT(color[6], color[5], color[4], color[1]);
				r += GET_RESULT(color[6], color[5], color[14], color[11]);
				r += GET_RESULT(color[6], color[5], color[2], color[7]);

				if (r > 0)
					product1b = color[6];
				else if (r < 0)
					product1b = color[5];
				else
					product1b = INTERPOLATE(color[5], color[6]);

				product2b = product1b;

			}
			else
			{
				if (color[6] == color[10] && color[10] == color[13] && color[9] != color[14] && color[10] != color[12])
					product2b = Q_INTERPOLATE(color[10], color[10], color[10], color[9]);
				else if (color[5] == color[9] && color[9] == color[14] && color[13] != color[10] && color[9] != color[15])
					product2b = Q_INTERPOLATE(color[9], color[9], color[9], color[10]);
				else
					product2b = INTERPOLATE(color[9], color[10]);

				if (color[6] == color[10] && color[6] == color[1] && color[5] != color[2] && color[6] != color[0])
					product1b = Q_INTERPOLATE(color[6], color[6], color[6], color[5]);
				else if (color[5] == color[9] && color[5] == color[2] && color[1] != color[6] && color[5] != color[3])
					product1b = Q_INTERPOLATE(color[6], color[5], color[5], color[5]);
				else
					product1b = INTERPOLATE(color[5], color[6]);
			}

			if (color[5] == color[10] && color[9] != color[6] && color[4] == color[5] && color[5] != color[14])
				product2a = INTERPOLATE(color[9], color[5]);
			else if (color[5] == color[8] && color[6] == color[5] && color[4] != color[9] && color[5] != color[12])
				product2a = INTERPOLATE(color[9], color[5]);
			else
				product2a = color[9];

			if (color[9] == color[6] && color[5] != color[10] && color[8] == color[9] && color[9] != color[2])
				product1a = INTERPOLATE(color[9], color[5]);
			else if (color[4] == color[9] && color[10] == color[9] && color[8] != color[5] && color[9] != color[0])
				product1a = INTERPOLATE(color[9], color[5]);
			else
				product1a = color[5];

			if (PixelsPerMask == 2)
			{
				if (ScummPCMode)
				{
					SWAP_WORD(product1a);
					SWAP_WORD(product1b);
					SWAP_WORD(product2a);
					SWAP_WORD(product2b);
				}
				*((unsigned long *) (&dst_line[0][x * 4])) = (product1a << 16) | product1b;
				*((unsigned long *) (&dst_line[1][x * 4])) = (product2a << 16) | product2b;
			}
			else
			{
				if (ScummPCMode)
				{
					SWAP_LONG(product1a);
					SWAP_LONG(product1b);
					SWAP_LONG(product2a);
					SWAP_LONG(product2b);
				}
				*((unsigned long *) (&dst_line[0][x * 8])) = product1a;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = product1b;
				*((unsigned long *) (&dst_line[1][x * 8])) = product2a;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = product2b;
			}

			/* Move color matrix forward */
			color[0] = color[1]; color[4] = color[5]; color[8] = color[9];   color[12] = color[13];
			color[1] = color[2]; color[5] = color[6]; color[9] = color[10];  color[13] = color[14];
			color[2] = color[3]; color[6] = color[7]; color[10] = color[11]; color[14] = color[15];

			if (src_x+x < BufferWidth-3)
			{
				x += 3;
				if (PixelsPerMask == 2)
				{
					color[3] = ScummColors16[*(src_line[0] + x) ];
					color[7] = ScummColors16[*(src_line[1] + x) ];
					color[11] = ScummColors16[*(src_line[2] + x) ];
					color[15] = ScummColors16[*(src_line[3] + x) ];
				}
				else
				{
					color[3] = ScummColors[*(src_line[0] + x)];
					color[7] = ScummColors[*(src_line[1] + x)];
					color[11] = ScummColors[*(src_line[2] + x)];
					color[15] = ScummColors[*(src_line[3] + x)];
				}
				x -= 3;
			}
		}

		/* We're done with one line, so we shift the source lines up */
		src_line[0] = src_line[1];
		src_line[1] = src_line[2];
		src_line[2] = src_line[3];

		/* Read next line */
		if (src_y + y + 3 >= BufferHeight)
			src_line[3] = src_line[2];
		else
			src_line[3] = src_line[2] + BufferWidth;

		/* Then shift the color matrix up */
		if (PixelsPerMask == 2)
		{
			byte *sbp;
			sbp = src_line[0];
			color[0] = ScummColors16[*sbp];     color[1] = color[0];    color[2] = ScummColors16[ *(sbp + 1) ];  color[3] = ScummColors16[*(sbp + 2)];
			sbp = src_line[1];
			color[4] = ScummColors16[*sbp];     color[5] = color[4];    color[6] = ScummColors16[ *(sbp + 1) ];  color[7] = ScummColors16[*(sbp + 2)];
			sbp = src_line[2];
			color[8] = ScummColors16[*sbp];     color[9] = color[8];    color[10] = ScummColors16[ *(sbp + 1) ]; color[11] = ScummColors16[*(sbp + 2)];
			sbp = src_line[3];
			color[12] = ScummColors16[*sbp];    color[13] = color[12];  color[14] = ScummColors16[ *(sbp + 1) ]; color[15] = ScummColors16[*(sbp + 2)];

			if (src_x + x > 0)
			{
				color[0] = ScummColors16[src_line[0][-1]];
				color[4] = ScummColors16[src_line[1][-1]];
				color[8] = ScummColors16[src_line[2][-1]];
				color[12] = ScummColors16[src_line[3][-1]];
			}
		}
		else
		{
			byte *lbp;
			lbp = src_line[0];
			color[0] = ScummColors[*lbp];     color[1] = color[0];    color[2] = ScummColors[ *(lbp + 1) ];  color[3] = ScummColors[*(lbp+2)];
			lbp = src_line[1];
			color[4] = ScummColors[*lbp];     color[5] = color[4];    color[6] = ScummColors[ *(lbp + 1) ];  color[7] = ScummColors[*(lbp+2)];
			lbp = src_line[2];
			color[8] = ScummColors[*lbp];     color[9] = color[8];    color[10] = ScummColors[ *(lbp + 1) ]; color[11] = ScummColors[*(lbp+2)];
			lbp = src_line[3];
			color[12] = ScummColors[*lbp];    color[13] = color[12];  color[14] = ScummColors[ *(lbp + 1) ]; color[15] = ScummColors[*(lbp+2)];
		}

		if (src_y + y < BufferHeight - 1)
		{
			dst_line[0] = dst_line[1]+dest_pitch;
			dst_line[1] = dst_line[0]+dest_pitch;
		}
	}
}

void SuperEagleScaler::Scale(uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height)
{
	unsigned int x, y;
	unsigned long color[12];
	byte *src;

	if (!handle)
		return;

	src = (byte *)Buffer+src_y*BufferWidth+src_x;

	/* Point to the first 3 lines. */
	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + BufferWidth;
	src_line[3] = src + BufferWidth * 2;

	dst_line[0] = dest+dest_y*2*dest_pitch+dest_x*2*dest_bpp;
	dst_line[1] = dst_line[0]+dest_pitch;

	x = 0, y = 0;

	if (PixelsPerMask == 2)
	{
		byte *sbp;
		sbp = src_line[0];
		color[0] = ScummColors16[*sbp];       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = ScummColors16[*(sbp+1)]; color[5] = ScummColors16[*(sbp+2)];
		sbp = src_line[2];
		color[6] = ScummColors16[*sbp];     color[7] = color[6];     color[8] = ScummColors16[*(sbp+1)]; color[9] = ScummColors16[*(sbp+2)];
		sbp = src_line[3];
		color[10] = ScummColors16[*sbp];    color[11] = ScummColors16[*(sbp+1)];
	}
	else
	{
		byte *lbp;
		lbp = src_line[0];
		color[0] = ScummColors[*lbp];       color[1] = color[0];   color[2] = color[0];    color[3] = color[0];
		color[4] = ScummColors[*(lbp+1)]; color[5] = ScummColors[*(lbp+2)];
		lbp = src_line[2];
		color[6] = ScummColors[*lbp];     color[7] = color[6];     color[8] = ScummColors[*(lbp+1)]; color[9] = ScummColors[*(lbp+2)];
		lbp = src_line[3];
		color[10] = ScummColors[*lbp];    color[11] = ScummColors[*(lbp+1)];
	}

	for (y = 0; y < height; y++)
	{
		/* Todo: x = width - 2, x = width - 1 */

		for (x = 0; x < width; x++)
		{
			unsigned long product1a, product1b, product2a, product2b;

//---------------------------------------     B1 B2           0  1
//                                         4  5  6  S2 ->  2  3  4  5
//                                         1  2  3  S1     6  7  8  9
//                                            A1 A2          10 11

			if (color[7] == color[4] && color[3] != color[8])
			{
				product1b = product2a = color[7];

				if ((color[6] == color[7]) || (color[4] == color[1]))
					product1a = INTERPOLATE(color[7], INTERPOLATE(color[7], color[3]));
				else
					product1a = INTERPOLATE(color[3], color[4]);

				if ((color[4] == color[5]) || (color[7] == color[10]))
					product2b = INTERPOLATE(color[7], INTERPOLATE(color[7], color[8]));
				else
					product2b = INTERPOLATE(color[7], color[8]);
			}
			else if (color[3] == color[8] && color[7] != color[4])
			{
				product2b = product1a = color[3];

				if ((color[0] == color[3]) || (color[5] == color[9]))
					product1b = INTERPOLATE(color[3], INTERPOLATE(color[3], color[4]));
				else
					product1b = INTERPOLATE(color[3], color[1]);

				if ((color[8] == color[11]) || (color[2] == color[3]))
					product2a = INTERPOLATE(color[3], INTERPOLATE(color[3], color[2]));
				else
					product2a = INTERPOLATE(color[7], color[8]);

			}
			else if (color[3] == color[8] && color[7] == color[4])
			{
				register int r = 0;

				r += GET_RESULT(color[4], color[3], color[6], color[10]);
				r += GET_RESULT(color[4], color[3], color[2], color[0]);
				r += GET_RESULT(color[4], color[3], color[11], color[9]);
				r += GET_RESULT(color[4], color[3], color[1], color[5]);

				if (r > 0)
				{
					product1b = product2a = color[7];
					product1a = product2b = INTERPOLATE(color[3], color[4]);
				}
				else if (r < 0)
				{
					product2b = product1a = color[3];
					product1b = product2a = INTERPOLATE(color[3], color[4]);
				}
				else
				{
					product2b = product1a = color[3];
					product1b = product2a = color[7];
				}
			}
			else
			{
				product2b = product1a = INTERPOLATE(color[7], color[4]);
				product2b = Q_INTERPOLATE(color[8], color[8], color[8], product2b);
				product1a = Q_INTERPOLATE(color[3], color[3], color[3], product1a);

				product2a = product1b = INTERPOLATE(color[3], color[8]);
				product2a = Q_INTERPOLATE(color[7], color[7], color[7], product2a);
				product1b = Q_INTERPOLATE(color[4], color[4], color[4], product1b);
			}

			if (PixelsPerMask == 2)
			{
				if (ScummPCMode)
				{
					SWAP_WORD(product1a);
					SWAP_WORD(product1b);
					SWAP_WORD(product2a);
					SWAP_WORD(product2b);
				}
				*((unsigned long *) (&dst_line[0][x * 4])) = (product1a << 16) | product1b;
				*((unsigned long *) (&dst_line[1][x * 4])) = (product2a << 16) | product2b;
			}
			else
			{
				if (ScummPCMode)
				{
					SWAP_LONG(product1a);
					SWAP_LONG(product1b);
					SWAP_LONG(product2a);
					SWAP_LONG(product2b);
				}
				*((unsigned long *) (&dst_line[0][x * 8])) = product1a;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = product1b;
				*((unsigned long *) (&dst_line[1][x * 8])) = product2a;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = product2b;
			}

			/* Move color matrix forward */
			color[0] = color[1];
			color[2] = color[3]; color[3] = color[4]; color[4] = color[5];
			color[6] = color[7]; color[7] = color[8]; color[8] = color[9];
			color[10] = color[11];

			if (src_x+x < BufferWidth - 2)
			{
				x += 2;
				if (PixelsPerMask == 2)
				{
					color[1] = ScummColors16[ *(src_line[0] + x) ];
					if (src_x+x < BufferWidth-1)
					{
						color[5] = ScummColors16[*(src_line[1]+x+1)];
						color[9] = ScummColors16[*(src_line[2]+x+1)];
					}
					color[11] = ScummColors16[*(src_line[3]+x)];
				}
				else
				{
					color[1] = ScummColors[*(src_line[0]+x)];
					if (src_x+x < BufferWidth-1)
					{
						color[5] = ScummColors[*(src_line[1]+x+1)];
						color[9] = ScummColors[ *(src_line[2]+x+1)];
					}
					color[11] = ScummColors[*(src_line[3]+x)];
				}
				x -= 2;
			}
		}

		/* We're done with one line, so we shift the source lines up */
		src_line[0] = src_line[1];
		src_line[1] = src_line[2];
		src_line[2] = src_line[3];

		/* Read next line */
		if (src_y+y+3 >= BufferHeight)
			src_line[3] = src_line[2];
		else
			src_line[3] = src_line[2] + BufferWidth;

		/* Then shift the color matrix up */
		if (PixelsPerMask == 2)
		{
			byte *sbp;
			sbp = src_line[0];
			color[0] = ScummColors16[*sbp];     color[1] = ScummColors16[*(sbp+1)];
			sbp = src_line[1];
			color[2] = ScummColors16[*sbp];     color[3] = color[2];    color[4] = ScummColors16[*(sbp+1)];  color[5] = ScummColors16[*(sbp+2)];
			sbp = src_line[2];
			color[6] = ScummColors16[*sbp];     color[7] = color[6];    color[8] = ScummColors16[*(sbp+1)];  color[9] = ScummColors16[*(sbp+2)];
			sbp = src_line[3];
			color[10] = ScummColors16[*sbp];    color[11] = ScummColors16[*(sbp+1)];
		}
		else
		{
			byte *lbp;
			lbp = src_line[0];
			color[0] = ScummColors[*lbp];     color[1] = ScummColors[*(lbp+1)];
			lbp = src_line[1];
			color[2] = ScummColors[*lbp];     color[3] = color[2];    color[4] = ScummColors[*(lbp+1)];  color[5] = ScummColors[*(lbp+2)];
			lbp = src_line[2];
			color[6] = ScummColors[*lbp];     color[7] = color[6];    color[8] = ScummColors[*(lbp+1)];  color[9] = ScummColors[*(lbp+2)];
			lbp = src_line[3];
			color[10] = ScummColors[*lbp];    color[11] = ScummColors[*(lbp+1)];
		}


		if (src_y + y < BufferHeight - 1)
		{
			dst_line[0] = dst_line[1]+dest_pitch;
			dst_line[1] = dst_line[0]+dest_pitch;
		}
	}
}

void AdvMame2xScaler::Scale(uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height)
{
	if (!handle)
		return;

	byte *src = (byte *)Buffer+src_y*BufferWidth+src_x;

	src_line[0] = src;
	src_line[1] = src;
	src_line[2] = src + BufferWidth;

	dst_line[0] = dest+dest_y*2*dest_pitch+dest_x*2*dest_bpp;
	dst_line[1] = dst_line[0]+dest_pitch;

	for (uint32 y = 0; y < height; y++)
	{
		for (uint32 x = 0; x < width; x++)
		{
			uint32 B, D, E, F, H;

			if (PixelsPerMask == 2)
			{
				// short A = *(src + i - nextlineSrc - 1);
				B = ScummColors16[src_line[0][x]];
				// short C = *(src + i - nextlineSrc + 1);
				D = ScummColors16[src_line[1][x-1]];
				E = ScummColors16[src_line[1][x]];
				F = ScummColors16[src_line[1][x+1]];
				// short G = *(src + i + nextlineSrc - 1);
				H = ScummColors16[src_line[2][x]];
				// short I = *(src + i + nextlineSrc + 1);
			}
			else
			{
				// short A = *(src + i - nextlineSrc - 1);
				B = ScummColors[src_line[0][x]];
				// short C = *(src + i - nextlineSrc + 1);
				D = ScummColors[src_line[1][x-1]];
				E = ScummColors[src_line[1][x]];
				F = ScummColors[src_line[1][x+1]];
				// short G = *(src + i + nextlineSrc - 1);
				H = ScummColors[src_line[2][x]];
				// short I = *(src + i + nextlineSrc + 1);
			}


			if (PixelsPerMask == 2)
			{
				if (ScummPCMode)
				{
					SWAP_WORD(B);
					SWAP_WORD(D);
					SWAP_WORD(E);
					SWAP_WORD(F);
					SWAP_WORD(H);
				}
				*((unsigned long *) (&dst_line[0][x * 4])) = ((D == B && B != F && D != H ? D : E) << 16) | (B == F && B != D && F != H ? F : E);
				*((unsigned long *) (&dst_line[1][x * 4])) = ((D == H && D != B && H != F ? D : E) << 16) | (H == F && D != H && B != F ? F : E);
			}
			else
			{
				if (ScummPCMode)
				{
					SWAP_LONG(B);
					SWAP_LONG(D);
					SWAP_LONG(E);
					SWAP_LONG(F);
					SWAP_LONG(H);
				}
				*((unsigned long *) (&dst_line[0][x * 8])) = D == B && B != F && D != H ? D : E;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = B == F && B != D && F != H ? F : E;
				*((unsigned long *) (&dst_line[1][x * 8])) = D == H && D != B && H != F ? D : E;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = H == F && D != H && B != F ? F : E;
			}
		}

		src_line[0] = src_line[1];
		src_line[1] = src_line[2];
		if (src_y+y+2 >= BufferHeight)
			src_line[2] = src_line[1];
		else
			src_line[2] = src_line[1] + BufferWidth;

		if (src_y+y < BufferHeight-1)
		{
			dst_line[0] = dst_line[1]+dest_pitch;
			dst_line[1] = dst_line[0]+dest_pitch;
		}
	}
}

void PointScaler::Scale(uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height)
{
	byte *src;
	uint32 color;
	uint32 r, g, b;
	uint32 x, y;

	if (!handle)
		return;

	src = (byte *)Buffer+src_y*BufferWidth+src_x;

	dst_line[0] = dest+dest_y*2*dest_pitch+dest_x*2*dest_bpp;
	dst_line[1] = dst_line[0]+dest_pitch;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			r = (ScummColors[*(src+x)] >> 16) & 0xff;
			g = (ScummColors[*(src+x)] >> 8) & 0xff;
			b = ScummColors[*(src+x)] & 0xff;

			color = MakeColor(dest_pixfmt, r, g, b);
			if (PixelsPerMask == 2)
			{
				if (ScummPCMode)
					SWAP_WORD(color);

				*((unsigned long *) (&dst_line[0][x * 4])) = (color << 16) | color;
				*((unsigned long *) (&dst_line[1][x * 4])) = (color << 16) | color;
			}
			else
			{
				if (ScummPCMode)
					SWAP_LONG(color);

				*((unsigned long *) (&dst_line[0][x * 8])) = color;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = color;
				*((unsigned long *) (&dst_line[1][x * 8])) = color;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = color;
			}
		}

		src += BufferWidth;

		if (src_y+y < BufferHeight-1)
		{
			dst_line[0] = dst_line[1]+dest_pitch;
			dst_line[1] = dst_line[0]+dest_pitch;
		}
	}
}

SCALERTYPE MorphOSScaler::FindByName(const char *ScalerName)
{
	int scaler = 0;

	while (ScummScalers[scaler].gs_Name)
	{
		if (!stricmp(ScalerName, ScummScalers[scaler].gs_Name))
			return ScummScalers[scaler].gs_Type;
		scaler++;
	}

	if (ScummScalers[scaler].gs_Name == NULL)
	{
		puts("Invalid scaler name. Please use one of the following:");
		for (scaler = 0; ScummScalers[scaler].gs_Name != NULL; scaler++)
			printf("  %s\n", ScummScalers[scaler].gs_Name);
	}

	return ST_INVALID;
}

SCALERTYPE MorphOSScaler::FindByIndex(int index)
{
	if (index >= 0 && index < 10 && ScummScalers[index].gs_Name)
		return ScummScalers[index].gs_Type;

	return ST_INVALID;
}

const char *MorphOSScaler::GetParamName(SCALERTYPE type)
{
	int scaler = 0;

	while (ScummScalers[scaler].gs_Name)
	{
		if (ScummScalers[scaler].gs_Type == type)
			return ScummScalers[scaler].gs_ParamName;
		scaler++;
	}

	return NULL;
}

