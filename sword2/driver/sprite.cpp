/* Copyright (C) 1994-2003 Revolution Software Ltd
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

//=============================================================================
//
//	Filename	:	sprite.c
//	Created		:	23rd September 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the sprite drawing functions.
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		25-Sep-96	PRP		Blits full sprites only.  Attempting to draw
//								any other type of sprite will return an error
//								code.  Also, scroll target position is handled
//								here
//
//	1.1		26-Sep-96	PRP		Added include file render.h for link to scroll
//								position.
//
//	1.2		04-Oct-96	PRP		Added direct path to ddraw.h
//
//	1.3		09-Oct-96	PRP		Made the sprite drawing routines write to either
//								the screen buffer or video memory.
//
//	1.4		23-Oct-96	PRP		Added the ability to draw a transparent sprite,
//								i.e. one with holes in. (with clipping and
//								everything)
//
//	1.5		29-Oct-96	PRP		Sprite drawing greatly improved.  We can now
//								combine flags to define whether the sprite is
//								compressed, transparent or aligned to the 
//								top corner of the monitor, as well as adding
//								a flag for fx.
//
//	1.6		29-Oct-96	PRP		Fixed drawing of sprites with the display align
//								bit set in type.
//
//	1.7		29-Oct-96	PRP		Made it so that when RDSPR_DISPLAYALIGN is
//								combined with RDSPR_TRANS the sprite is drawn
//								in the correct place.
//
//	1.8		31-Oct-96	PRP		Fixed bug in sprite width calculation.
//
//	1.9		01-Nov-96	PRP		Fixed transparent sprite clipping at the bottom
//
//	1.10	01-Nov-96	PRP		Added RDSPR_SPRITECOMPRESSION to the attributes
//								which may be passed into DrawSprite, and 
//								implemented a basic decompression piece of code
//								which is currently very slooowwww.
//
//	1.11	05-Nov-96	PRP		RENDERWIDE and RENDERDEEP now defined in 
//								render.h
//
//	1.12	06-Nov-96	PRP		Fixed clipping for RDSPR_DISPLAYALIGN sprites.
//
//	1.13	08-Nov-96	PRP		Added decompression for types RDSPR_RLE256 and
//								RDSPR_RLE16.  Also changed the format of
//								DrawSprite() so that the details required by
//								it are passed in a structure.
//
//	1.14	12-Nov-96	PRP		Enabled drawing of flipped sprites.  This is
//								only possible for sprites which are compressed
//								and not drawn with DISPLAYALIGN
//
//	1.15	15-Nov-96	PRP		Size of menubar moved to menu.h
//
//	1.16	21-Nov-96	PRP		Implemented scaling for sprites - only certain
//								types mind.
//
//	1.17	25-Nov-96	PRP		Removed the second displayed sprite to compare
//								the anti-aliasing code.
//
//	1.18	25-Nov-96	PRP		Removed the ColourMatch function which only
//								worked on a reduced sub-set of the palette,
//								and replaced it with QuickMatch in palette.c.
//
//	1.19	26-Nov-96	PRP		Implemented scaling of sprites which are
//								larger than their original size.
//
//	1.20	27-Nov-96	PRP		Fixed the problem with displaying sprites which
//								are transparent, uncompressed and display
//								aligned.  (James will be happy)
//
//	1.21	02-Dec-96	PRP		Implemented sprites which blend with the
//								background.
//
//	1.22	18-Dec-96	PRP		Moved origin of sprites which are not transparent
//								and display aligned.
//
//	1.23	09-Jan-97	PRP		Display align clipping fixed for sprites which
//								are completely off the screen.
//
//	1.24	24-Jan-97	PRP		Added an offset to a sprite's x and y position
//								depending upon where the latest parallax layer
//								was with reference to the background position.
//
//	1.25	28-Jan-97	PRP		Updated sprite drawing code to utilise hardware
//								blitting where available.
//
//	1.26	06-Feb-97	PRP		Fixed drawing of sprites after video memory
//								is full up.
//
//	1.27	10-Feb-97	PRP		Changed direct draw error reporting function
//								calls.
//
//	1.28	11-Feb-97	PRP		Implemented blending of sprites - depending
//								upon values passed in by spriteInfo.
//
//	1.29	12-Feb-97	PRP		Fixed the blended sprite rendering position
//								bug.
//
//	1.30	12-Feb-97	PRP		Changed the rendering code for blended sprites
//								in ALLHARDWARE mode.
//
//	1.31	19-Mar-97	PRP		Added CreateSurface, DrawSurface and, you
//								guessed it, DeleteSurface functions.
//								Added temporary profiling code to optimise
//								sprite drawing.
//								Implemented sprite compression, and drawing
//								whilst decompressing code for sprite 
//								optimisation
//
//	1.32	24-Mar-97	PRP		Fixed DrawSurface so that it returns an error
//								code if the surface has been lost.
//
//	1.33	25-Mar-97	PRP		Initialised freeSprite in the create surface
//								function.
//
//	1.34	27-Mar-97	PRP		Further optimisation of different types
//								of sprites.  New sprite compression type
//								RDSPR_RLE256FAST which decompresses and
//								draws at the same time.  This type should
//								be used for static 256 colour transparent
//								sprites.
//
//	1.35	01-Apr-97	PRP		Got rid of the RDSPR_LAYERCOMPRESSION
//								sprite type.
//
//	1.36	01-Apr-97	PRP		Made transparent sprites stippled when
//								hardware acceleration is used.
//
//	1.37	01-Apr-97	PRP		Fixed the bug which was tring to anti-alias
//								sprites which were not over the screen area.
//
//	1.38	07-Apr-97	PRP		Added code which looks at the light mask
//								and renders George accordingly.
//
//	1.38	08-Apr-97	PRP		Added code to decompress the light mask!
//
//	1.38	08-Apr-97	JEL		Added another instance of light mask check.
//
//	1.39	08-Apr-97	JEL		Changed screenWide to locationWide
//
//	1.40	09-Apr-97	PRP		Fixed bug with fast draw not clipping
//								correctly on the left edge.  Now it does :)
//
//	1.41	09-Apr-97	PRP		Added shadow layer effects for sprites which
//								are not scaled, but have the RDSPR_SHADOW
//								flag set.
//
//	1.42	09-Apr-97	JEL		No changes.
//
//	1.43	09-Apr-97	PRP		Fixed bug.  Shadow mask was being referenced
//								from the top of the screen, rather than the
//								top of the background!!!
//
//	1.44	09-Apr-97	JEL		Changed starty to scrolly for driver bug.
//
//	1.45	22-Apr-97	PRP		Added return values for decompression code.
//
//	1.46	24-Apr-97	JEL		Fixed the decompression choice code.
//
//	1.47	28-May-97	CJR		Added more information to the Direct draw
//								errors.
//
//	1.48	13-Jun-97	PRP		Enabled the shadow mask when the arithmetic 
//								stretching and anti-aliasing is switched off.
//
//	1.49	13-Jun-97	PRP		Made use of shadow mask independently switchable.
//
//	1.50	07-Jul-97	PRP		Fixed a bug with the offset of sprites which are
//								not compressed.
//
//	1.51	23-Jul-97	PRP		Checked src colour key blitting flag before creating
//								a surface in video memory
//
//	1.52	31-Jul-97	PRP		Only allowed shadow masks to affect the sprite palette.
//
//	1.53	31-Jul-97	PRP		Fixed a bug with the above change.  Non sprite
//								palette sprites were not being drawn, but are now :)
//
//	1.54	31-Jul-97	PRP		Fixed the black box from around shadow sprites.
//
//	1.55	31-Jul-97	JEL		Fixed the bug of transparent sprites suddenly printing in wrong position!
//
//	1.56	01-Aug-97	PRP		Tried to fix the corruption at the end of stippled sprites.
//
//	1.57	01-Aug-97	PRP		Replaced algorithm to fix corruption at end of stippled sprites.
//
//	1.58	01-Aug-97	PRP		Removed the shitty sprite palette fix (1.52)
//
//	1.60	27-Aug-97	PRP		Changed the drawing of transparent sprites so that
//								the uncompressed sprites display properly.
//
//	1.61	02-Sep-97	CJR		Changed the Decompression code to return an error before causing
//								an access violation.
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//	int32 CreateSurface(_spriteInfo *s, uint32 *surface)
//
//	Creates a sprite surface in video memory (if possible) and returns it's
//	handle in surface.
//
//	---------------------------------------------------------------------------
//
//	int32 DrawSurface(_spriteInfo *s, uint32 surface)
//
//	Draws the sprite surface created earlier.  If the surface has been lost,
//	it is recreated.
//
//	---------------------------------------------------------------------------
//
//	int32 DeleteSurface(uint32 surface)
//
//	Deletes a surface from video memory.
//
//	--------------------------------------------------------------------------
//
//	int32 DrawSprite(_spriteInfo *s)
//
//	Draws a sprite onto the screen.  The _spriteInfo structure holds all of
//	the information needed to draw the sprite - see driver96.h for details
//
//=============================================================================




#include <stdio.h>
#include "stdafx.h"
#include "driver96.h"
#include "d_draw.h"
#include "render.h"
#include "menu.h"
#include "palette.h"


#if PROFILING == 1
extern int32 profileSpriteRender;
extern int32 profileDecompression;
#endif


#define DEBUG_TIMING 0
//long int myTimers[10][2];


char shitColourTable[1024];
static uint8 *lightMask = 0;

















//	--------------------------------------------------------------------------
//
//	int32 MirrorSprite(uint8 *dst, uint8 *src, int16 w, int16 h)
//
//	This function takes the sprite pointed to by src and creates a mirror
//	image of it in dst.
//
//	--------------------------------------------------------------------------
int32 MirrorSprite(uint8 *dst, uint8 *src, int16 w, int16 h)

{

	int16 x, y;

	for (y=0; y<h; y++)
	{
		for (x=0; x<w; x++)
		{
			*dst++ = *(src + w-x-1);
		}
		src += w;
	}
	
	return(RD_OK);

}


//	--------------------------------------------------------------------------
//
//	int32 DecompressRLE256(uint8 *dest, uint8 *source, int32 decompSize)
//
//	This function takes a compressed frame of a sprite (with up to 256 colours)
//	and decompresses it into the area of memory marked by the destination
//	pointer.  The decompSize is used to measure when the decompression process
//	has completed.
//
//	--------------------------------------------------------------------------
int32 DecompressRLE256(uint8 *dest, uint8 *source, int32 decompSize)
{
	// PARAMETERS:
	// source		points to the start of the sprite data for input
	// decompSize	gives size of decompressed data in bytes
	// dest			points to start of destination buffer for decompressed data

	uint8 headerByte;							// block header byte
	uint8 *endDest=dest+decompSize;				// pointer to byte after end of decomp buffer
	int32 rv;

#if PROFILING == 1
	long int startTime, endTime;
	QueryPerformanceCounter(&startTime);
#endif


	while(1)
	{
		//----------------------------
		// FLAT block
		headerByte = *source++;					// read FLAT block header & increment 'scan' to first pixel of block

		if (headerByte)							// if this isn't a zero-length block
		{
			if (dest+headerByte > endDest)
			{
				rv = 1;
				break;
			}

			memset(dest,*source,headerByte);	// set the next 'headerByte' pixels to the next colour at 'source'
			dest+=headerByte;					// increment destination pointer to just after this block
			source++;							// increment source pointer to just after this colour

			if (dest==endDest)					// if we've decompressed all of the data
			{
				rv = 0;					// return "OK"
				break;
			}
		}
		//----------------------------
		// RAW block
		headerByte = *source++;					// read RAW block header & increment 'scan' to first pixel of block

		if (headerByte)							// if this isn't a zero-length block
		{
			if (dest+headerByte > endDest)
			{
				rv = 1;
				break;
			}

			memcpy(dest,source,headerByte);		// copy the next 'headerByte' pixels from source to destination
			dest+=headerByte;					// increment destination pointer to just after this block
			source+=headerByte;					// increment source pointer to just after this block

			if (dest==endDest)					// if we've decompressed all of the data
			{
				rv = 0;					// return "OK"
				break;
			}
		}
		//----------------------------
	}
#if PROFILING == 1
	QueryPerformanceCounter(&endTime);
	profileDecompression += (endTime.LowPart - startTime.LowPart);
#endif

	return(rv);
}



//	--------------------------------------------------------------------------
//
//	void UnwindRaw16(uint8 *dest, uint8 *source, uint8 blockSize, uint8 *colTable)
//
//	This function unwinds a run of colour 16 data into 256 colour palette
//	data.
//	--------------------------------------------------------------------------
void UnwindRaw16(uint8 *dest, uint8 *source, uint8 blockSize, uint8 *colTable)
{
	while(blockSize>1)							// for each pair of pixels
	{
		*dest++ = colTable[(*source) >> 4];		// 1st colour = number in table at position given by upper nibble of source byte
		*dest++ = colTable[(*source) & 0x0f];	// 2nd colour = number in table at position given by lower nibble of source byte
		source++;								// point to next source byte
		blockSize-=2;							// decrement count of how many pixels left to read
	}

	if (blockSize)								// if there's a final odd pixel
		*dest++ = colTable[(*source)>>4];		// colour = number in table at position given by upper nibble of source byte
}



//	--------------------------------------------------------------------------
//
//	int32 DecompressRLE16(uint8 *dest, uint8 *source, int32 decompSize, uint8 *colTable)
//
//	This function takes a compressed frame of a sprite (with up to 16 colours)
//	and decompresses it into the area of memory marked by the destination
//	pointer.  The decompSize is used to measure when the decompression process
//	has completed.  The colour table which maps the 16 encoded colours to the 
//	current palette is passed in to colTable.
//
//	--------------------------------------------------------------------------
int32 DecompressRLE16(uint8 *dest, uint8 *source, int32 decompSize, uint8 *colTable)
{
	// PARAMETERS:
	// source		points to the start of the sprite data for input
	// decompSize	gives size of decompressed data in bytes
	// dest			points to start of destination buffer for decompressed data
	// colTable		points to a 16-byte table of colours used to encode the RAW pixels into 4-bits each

	uint8 headerByte;							// block header byte
	uint8 *endDest=dest+decompSize;				// pointer to byte after end of decomp buffer
	int32 rv;

#if PROFILING == 1
	long int startTime, endTime;
	QueryPerformanceCounter(&startTime);
#endif

	while(1)
	{
		//----------------------------
		// FLAT block
		headerByte = *source++;					// read FLAT block header & increment 'scan' to first pixel of block

		if (headerByte)							// if this isn't a zero-length block
		{
			if (dest+headerByte > endDest)
			{
				rv = 1;
				break;
			}

			memset(dest,*source,headerByte);	// set the next 'headerByte' pixels to the next colour at 'source'
			dest+=headerByte;					// increment destination pointer to just after this block
			source++;							// increment source pointer to just after this colour

			if (dest==endDest)					// if we've decompressed all of the data
			{	
				rv = 0;					// return "OK"
				break;
			}
		}
		//----------------------------
		// RAW block
		headerByte = *source++;					// read RAW block header & increment 'scan' to first pixel of block

		if (headerByte)							// if this isn't a zero-length block
		{
			if (dest+headerByte > endDest)
			{
				rv = 1;
				break;
			}

			UnwindRaw16(dest,source,headerByte,colTable);	// copy the next 'headerByte' pixels from source to destination (NB. 2 pixels per byte)
			dest+=headerByte;					// increment destination pointer to just after this block
			source+=(headerByte+1)/2;			// increment source pointer to just after this block (NB. headerByte gives pixels, so /2 for bytes)

			if (dest>=endDest)					// if we've decompressed all of the data
			{
				rv = 0;					// return "OK"
				break;
			}
		}
		//----------------------------
	}

#if PROFILING == 1
	QueryPerformanceCounter(&endTime);
	profileDecompression += (endTime.LowPart - startTime.LowPart);
#endif

	return(rv);

}



int32 PaulCompression(_spriteInfo *s, uint8 *src, uint8 *dst)

{

	int		j;
	int		bytesToGo;
	uint8	colour;
	uint8	runCount;
	uint8	*runPointer;

	//  Compressed the 256 colour source data into line compressed data.
	for (j=0; j<s->h; j++)
	{
		bytesToGo = s->w;

		while (bytesToGo)
		{
			//  Look for a run of flat colour or zeros first.
			if ((bytesToGo == 1) || (*src == 0) || (*src == *(src+1)))
			{
				colour = *src;
				runCount = 0;
				while ((*src == colour) && (bytesToGo) && (runCount < 255))
				{
					runCount++;
					bytesToGo--;
					src++;
				}
				*dst++ = runCount;
				*dst++ = colour;
			}
			else
			{
				// We do not have a run of flat colour, so set the run length to zero
				*dst++ = 0;
			}

			if (bytesToGo)
			{
				// Look for a run of different colours excluding zero
				runCount = 0;
				colour = 0;
				if (((bytesToGo == 1) && (*src)) || ((*src) && (*(src) != *(src+1))))
				{
					runPointer = dst++;
					while ((bytesToGo) && (runCount < 255) && (*src != colour) && (*src))
					{
						runCount++;
						bytesToGo--;
						colour = *src;
						*dst++ = *src++;
					}
					if (*src == colour)
					{
						runCount--;
						bytesToGo++;
						dst--;
						src--;
					}
					*runPointer = runCount;
				}
				else
				{
					*dst++ = 0;
				}
			}
			else
			{
				// We do not have a run of different colours
				*dst++ = 0;
			}
		}
	}
	return(RD_OK);
}



int32 SoftwareFlipRenderCompressed256(_spriteInfo *s)

{

	return(RDERR_NOTIMPLEMENTED);

}




int32 SoftwareRenderCompressed256(_spriteInfo *s)

{
	warning("stub SoftwareRenderCompressed256");
/*
	int32 rv;
	int32 i;
	int32 clipped = 0;
	int32 clippedLeft, clippedRight;
	int32 clippedCount;
	RECT rSrc;
	RECT rDst;

	if (s->type & RDSPR_DISPLAYALIGN)
		return(RDERR_NOTIMPLEMENTED);

	if (s->type & RDSPR_FLIP)
	{
		rv = SoftwareFlipRenderCompressed256(s);
	}
	else
	{
		// Check to see whether we are dealing with a scaled sprite or not.
		if ((s->scale == 0) || (s->scale == 256))
		{
			//	The sprite is not scaled
			//	Work out the clip rectangles - source and destination
			rDst.left = s->x - scrollx;
			rDst.right = rDst.left + s->w;
			rDst.top = s->y - scrolly;
			rDst.bottom = rDst.top + s->h;

			// Do major clipping
			if ((rDst.left <= 0) && (rDst.right <= 0))
				return(RD_OK);
			if ((rDst.left >= RENDERWIDE) && (rDst.right >= RENDERWIDE))
				return(RD_OK);
			if ((rDst.top <= 0) && (rDst.bottom <= 0))
				return(RD_OK);
			if ((rDst.top >= RENDERDEEP) && (rDst.bottom >= RENDERDEEP))
				return(RD_OK);

			rSrc.top = 0;
			rSrc.bottom = s->h;
			rSrc.left = 0;
			rSrc.right = s->w;
			clippedLeft = 0;
			clippedRight = 0;

			//	Now clip the fuckers
			if (rDst.top < 0)
			{
				rSrc.top -= rDst.top;
				rDst.top = 0;
				clipped = 1;
			}
			if (rDst.bottom > RENDERDEEP)
			{
				rSrc.bottom -= (rDst.bottom - RENDERDEEP);
				rDst.bottom = RENDERDEEP;
				clipped = 1;
			}
			if (rDst.left < 0)
			{
				clippedLeft = -rDst.left;
				rSrc.left += clippedLeft;
				rDst.left = 0;
				clipped = 1;
			}
			if (rDst.right > RENDERWIDE)
			{
				clippedRight = rDst.right - RENDERWIDE;
				rSrc.right -= clippedRight;
				rDst.right = RENDERWIDE;
				clipped = 1;
			}

			if (s->type & RDSPR_BLEND)
			{
				//	THE SPRITE IS BLENDED
				int32	lineBytesToGo = s->w;
//				int32	bytesToGo;
				uint8	*spr = s->data;
				uint8	*dst = myScreenBuffer + rDst.left + rDst.top * RENDERWIDE;
				uint8	runLength = 0;

				//	The sprite is not blended
				
				//	Decompress the source data until we are at the line where
				//	the drawing is to start.

				if (clipped)
				{
					//	Clip off any whole lines from the top of the sprite
					for (i=0; i<rSrc.top; i++)
					{
						lineBytesToGo = s->w;
						while (lineBytesToGo)
						{
							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of the same colour or zeros
								if (*spr)
								{
									memset(dst, *spr, runLength);
								}
//								dst += runLength;
								spr++;
								lineBytesToGo -= runLength;
							}

							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of different colours
								memcpy(dst, spr, runLength);
								spr += runLength;
//								dst += runLength;
								lineBytesToGo -= runLength;
							}
						}
//						dst += (RENDERWIDE - s->w);
					}

					//	Draw until we get to the clipped bottom of the sprite.
					while (i < rSrc.bottom)
					{
						lineBytesToGo = s->w;
						// Do the first part of the line
						if (clippedLeft)
						{
							clippedCount = clippedLeft;
							while (clippedCount > 0)
							{
								runLength = *spr++;
								if (runLength)
								{
									//	This is a run of the same colour or zeros
									//	Check that we aren't going to go over clippedCount
									if (runLength > clippedCount)
									{
										runLength -= clippedCount;
										if (*spr)
											memset(dst, *spr, runLength);
										dst += runLength;
//											memset(dst + clippedCount, *spr, runLength);
//										dst += (runLength + clippedCount);
										spr++;
										lineBytesToGo -= (runLength + clippedCount);
										clippedCount = -runLength;
									}
									else
									{
//	WE DON'T DO THIS BIT BECUASE WE ARE IN THE CLIPPED REGION
//										if (*spr)
//										{
//											memset(dst, *spr, runLength);
//										}

//										dst += runLength;

										spr++;
										clippedCount -= runLength;
										lineBytesToGo -= runLength;
									}
								}

								runLength = *spr++;
								if (runLength)
								{
									//	This is a run of different colours
									//	See if we are already over the clipped count
									if (clippedCount < 0)
									{
										memcpy(dst, spr, runLength);
										dst += runLength;
										spr += runLength;
										lineBytesToGo -= runLength;
									}
									//	See if we are going to go over the clipped count
									else if (runLength > clippedCount)
									{
										runLength -= clippedCount;
//										memcpy(dst + clippedCount , spr + clippedCount, runLength);
//										dst += (runLength + clippedCount);
										memcpy(dst, spr + clippedCount, runLength);
										dst += runLength;
										spr += (runLength + clippedCount);
										lineBytesToGo -= (runLength + clippedCount);
										clippedCount = -runLength;
									}
									else
									{
//	WE DON'T DO THIS BIT BECAUSE WE ARE IN THE CLIPPED REGION
//										memcpy(dst, spr, runLength);
										spr += runLength;
//										dst += runLength;
										lineBytesToGo -= runLength;
										clippedCount -= runLength;
									}
								}
							}
						}

//						lineBytesToGo > clippedRight;
						while (lineBytesToGo > clippedRight)
						{
							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of the same colour or zeros
								//  Ensure we're not going over the clipped region
								if (*spr)
								{
									//	Let's check that the run is not longer than the clipped region
									if (lineBytesToGo - runLength < clippedRight)
									{
										if (lineBytesToGo - clippedRight > 0)
										{
											memset(dst, *spr, lineBytesToGo - clippedRight);
											dst += (lineBytesToGo - clippedRight);
										}
									}
									else
									{
										memset(dst, *spr, runLength);
										dst += runLength;
									}
								}
								spr++;
								lineBytesToGo -= runLength;
							}

							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of different colours
								//	Let's check that the run is not longer than the clipped region
								if (lineBytesToGo - runLength < clippedRight)
								{
									if (lineBytesToGo - clippedRight > 0)
									{
										memcpy(dst, spr, lineBytesToGo - clippedRight);
										dst += (lineBytesToGo - clippedRight);
									}
								}
								else
								{
									memcpy(dst, spr, runLength);
									dst += runLength;
								}
								spr += runLength;
								lineBytesToGo -= runLength;
							}
						}

						//	Go through the compressed data to the end of the line.
						while (lineBytesToGo)
						{
							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of the same colour or zeros
//	DON'T DO THIS COS WE'RE OUTSIDE CLIP REGION
//								if (*spr)
//								{
//									memset(dst, *spr, runLength);
//								}

//								dst += runLength;
								spr++;
								lineBytesToGo -= runLength;
							}

							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of different colours
//	DON'T DO THIS COS WE'RE OUTSIDE CLIP REGION
//								memcpy(dst, spr, runLength);
								spr += runLength;
//								dst += runLength;
								lineBytesToGo -= runLength;
							}
						}

//						dst += (RENDERWIDE - s->w);
						dst += (RENDERWIDE - (rDst.right - rDst.left));
						i++;
					}
				}
				else
				{
					//	Fuck the clipping, let's just draw it for now
					for (i=0; i<s->h; i++)
					{
						lineBytesToGo = s->w;
						while (lineBytesToGo)
						{
							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of the same colour or zeros
								if (*spr)
								{
									memset(dst, *spr, runLength);
								}
								dst += runLength;
								spr++;
								lineBytesToGo -= runLength;
							}

							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of different colours
								memcpy(dst, spr, runLength);
								spr += runLength;
								dst += runLength;
								lineBytesToGo -= runLength;
							}
						}
						dst += (RENDERWIDE - s->w);
					}
				}
			}
			else
			{
				int32	lineBytesToGo = s->w;
//				int32	bytesToGo;
				uint8	*spr = s->data;
				uint8	*dst = myScreenBuffer + rDst.left + rDst.top * RENDERWIDE;
				uint8	runLength = 0;

				//	The sprite is not blended
				
				//	Decompress the source data until we are at the line where
				//	the drawing is to start.

				if (clipped)
				{
					//	Clip off any whole lines from the top of the sprite
					for (i=0; i<rSrc.top; i++)
					{
						lineBytesToGo = s->w;
						while (lineBytesToGo)
						{
							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of the same colour or zeros
								if (*spr)
								{
									memset(dst, *spr, runLength);
								}
//								dst += runLength;
								spr++;
								lineBytesToGo -= runLength;
							}

							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of different colours
								memcpy(dst, spr, runLength);
								spr += runLength;
//								dst += runLength;
								lineBytesToGo -= runLength;
							}
						}
//						dst += (RENDERWIDE - s->w);
					}

					//	Draw until we get to the clipped bottom of the sprite.
					while (i < rSrc.bottom)
					{
						lineBytesToGo = s->w;
						// Do the first part of the line
						if (clippedLeft)
						{
							clippedCount = clippedLeft;
							while (clippedCount > 0)
							{
								runLength = *spr++;
								if (runLength)
								{
									//	This is a run of the same colour or zeros
									//	Check that we aren't going to go over clippedCount
									if (runLength > clippedCount)
									{
										runLength -= clippedCount;
										if (*spr)
											memset(dst, *spr, runLength);
										dst += runLength;
//											memset(dst + clippedCount, *spr, runLength);
//										dst += (runLength + clippedCount);
										spr++;
										lineBytesToGo -= (runLength + clippedCount);
										clippedCount = -runLength;
									}
									else
									{
//	WE DON'T DO THIS BIT BECUASE WE ARE IN THE CLIPPED REGION
//										if (*spr)
//										{
//											memset(dst, *spr, runLength);
//										}

//										dst += runLength;

										spr++;
										clippedCount -= runLength;
										lineBytesToGo -= runLength;
									}
								}

								runLength = *spr++;
								if (runLength)
								{
									//	This is a run of different colours
									//	See if we are already over the clipped count
									if (clippedCount < 0)
									{
										memcpy(dst, spr, runLength);
										dst += runLength;
										spr += runLength;
										lineBytesToGo -= runLength;
									}
									//	See if we are going to go over the clipped count
									else if (runLength > clippedCount)
									{
										runLength -= clippedCount;
//										memcpy(dst + clippedCount , spr + clippedCount, runLength);
//										dst += (runLength + clippedCount);
										memcpy(dst, spr + clippedCount, runLength);
										dst += runLength;
										spr += (runLength + clippedCount);
										lineBytesToGo -= (runLength + clippedCount);
										clippedCount = -runLength;
									}
									else
									{
//	WE DON'T DO THIS BIT BECAUSE WE ARE IN THE CLIPPED REGION
//										memcpy(dst, spr, runLength);
										spr += runLength;
//										dst += runLength;
										lineBytesToGo -= runLength;
										clippedCount -= runLength;
									}
								}
							}
						}

//						lineBytesToGo > clippedRight;
						while (lineBytesToGo > clippedRight)
						{
							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of the same colour or zeros
								//  Ensure we're not going over the clipped region
								//	Let's check that the run is not longer than the clipped region
								if (lineBytesToGo - runLength < clippedRight)
								{
									if (lineBytesToGo - clippedRight > 0)
									{
										if (*spr)
											memset(dst, *spr, lineBytesToGo - clippedRight);
										dst += (lineBytesToGo - clippedRight);
									}
								}
								else
								{
									if (*spr)
										memset(dst, *spr, runLength);
									dst += runLength;
								}
								spr++;
								lineBytesToGo -= runLength;
							}

							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of different colours
								//	Let's check that the run is not longer than the clipped region
								if (lineBytesToGo - runLength < clippedRight)
								{
									if (lineBytesToGo - clippedRight > 0)
									{
										memcpy(dst, spr, lineBytesToGo - clippedRight);
										dst += (lineBytesToGo - clippedRight);
									}
								}
								else
								{
									memcpy(dst, spr, runLength);
									dst += runLength;
								}
								spr += runLength;
								lineBytesToGo -= runLength;
							}
						}

						//	Go through the compressed data to the end of the line.
						while (lineBytesToGo)
						{
							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of the same colour or zeros
//	DON'T DO THIS COS WE'RE OUTSIDE CLIP REGION
//								if (*spr)
//								{
//									memset(dst, *spr, runLength);
//								}

//								dst += runLength;
								spr++;
								lineBytesToGo -= runLength;
							}

							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of different colours
//	DON'T DO THIS COS WE'RE OUTSIDE CLIP REGION
//								memcpy(dst, spr, runLength);
								spr += runLength;
//								dst += runLength;
								lineBytesToGo -= runLength;
							}
						}

//						dst += (RENDERWIDE - s->w);
						dst += (RENDERWIDE - (rDst.right - rDst.left));
						i++;
					}
				}
				else
				{
					//	Fuck the clipping, let's just draw it for now
					for (i=0; i<s->h; i++)
					{
						lineBytesToGo = s->w;
						while (lineBytesToGo)
						{
							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of the same colour or zeros
								if (*spr)
								{
									memset(dst, *spr, runLength);
								}
								dst += runLength;
								spr++;
								lineBytesToGo -= runLength;
							}

							runLength = *spr++;
							if (runLength)
							{
								//	This is a run of different colours
								memcpy(dst, spr, runLength);
								spr += runLength;
								dst += runLength;
								lineBytesToGo -= runLength;
							}
						}
						dst += (RENDERWIDE - s->w);
					}
				}
			}
		}
		else
		{
			//	The sprite is scaled.
			rv = RDERR_NOTIMPLEMENTED;
		}
	}
	return(rv);
*/
	return 0;
}


int32 CreateSurface(_spriteInfo *s, uint32 *surface)

{
	warning("stub CreateSurface");
/*

	uint8				*sprite, *newSprite;
	uint8				*src, *dst;
	int32				freeSprite = 0;
	int32				i;
	HRESULT				hr;
	DDSURFACEDESC		ddsd;
	LPDIRECTDRAWSURFACE dds;


	//	Create a surface for the sprite and then draw it.
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	if (s->scale & 0xff)
	{
		if (dxHalCaps & RDCAPS_BLTSTRETCH)
			ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		else
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	}
	else
	{
		if (dxHalCaps & RDCAPS_SRCBLTCKEY)
			ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		else
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	}

	ddsd.dwWidth = s->w;
	ddsd.dwHeight = s->h;
	hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, &dds, NULL);
	if (hr == DDERR_OUTOFVIDEOMEMORY)
	{
		ddsd.ddsCaps.dwCaps &= (0xffffffff - DDSCAPS_VIDEOMEMORY);
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, &dds, NULL);
	}
	if (hr != DD_OK)
	{
		DirectDrawError("Cannot create sprite surface", hr);
		return(hr);
	}

	if (s->type & RDSPR_TRANS)
		hr = IDirectDrawSurface2_SetColorKey(dds, DDCKEY_SRCBLT, &blackColorKey);

	if (s->type &RDSPR_NOCOMPRESSION)
	{
		sprite = s->data;
	}
	else
	{
		sprite = (uint8 *) malloc(s->w * s->h);
		freeSprite = 1;
		if (sprite == NULL)
		{
			IDirectDrawSurface2_Release(dds);
			return(RDERR_OUTOFMEMORY);
		}

		if (s->type >> 8 == RDSPR_RLE16 >> 8)
		{
			if (DecompressRLE16(sprite, s->data, s->w * s->h, s->colourTable))
				return(RDERR_DECOMPRESSION);
		}
		else
		{
			if (DecompressRLE256(sprite, s->data, s->w * s->h))
				return(RDERR_DECOMPRESSION);
		}

		if (s->type & RDSPR_FLIP)
		{
			newSprite = (uint8 *) malloc(s->w * s->h);
			if (newSprite == NULL)
			{
				free(sprite);
				return(RDERR_OUTOFMEMORY);
			}
			MirrorSprite(newSprite, sprite, s->w, s->h);
			free(sprite);
			sprite = newSprite;
		}
	}

	hr = IDirectDrawSurface2_Lock(dds, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	if (hr != DD_OK)
	{
		IDirectDrawSurface2_Restore(dds);
		hr = IDirectDrawSurface2_Lock(dds, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if (hr != DD_OK)
		{
			DirectDrawError("Cannot lock sprite surface", hr);
			return(hr);
		}
	}

	src = sprite;
	dst = ddsd.lpSurface;
	for (i=0; i<s->h; i++)
	{
		memcpy(dst, src, s->w);
		src += s->w;
		dst += ddsd.lPitch;
	}

	IDirectDrawSurface2_Unlock(dds, ddsd.lpSurface);

	*surface = (uint32) dds;

	if (freeSprite)
		free(sprite);
*/
	return(RD_OK);

}


int32 DrawSurface(_spriteInfo *s, uint32 surface)

{
	warning("stub DrawSurface");
/*

	HRESULT				hr;
	RECT				rd, rs;
	LPDIRECTDRAWSURFACE	dds;
	char myString[256];


	dds = (LPDIRECTDRAWSURFACE) surface;

	// Set startx and starty for the screen buffer		ADDED THIS!
	if (s->type & RDSPR_DISPLAYALIGN)
		rd.top = s->y;
	else
		rd.top = s->y - scrolly;
		
	if (s->type & RDSPR_DISPLAYALIGN)
		rd.left = s->x;
	else
		rd.left = s->x - scrollx;

	rs.left = 0;
	rs.right = s->w;
	rs.top = 0;
	rs.bottom = s->h;
	if (s->scale & 0xff)
	{
		rd.right = rd.left + s->scaledWidth;
		rd.bottom = rd.top + s->scaledHeight;
		// Do clipping
		if (rd.top < 40)
		{
			rs.top = (40 - rd.top) * 256 / s->scale;
			rd.top = 40;
		}
		if (rd.bottom > 440)
		{
			rs.bottom -= ((rd.bottom - 440) * 256 / s->scale);
			rd.bottom = 440;
		}
		if (rd.left < 0)
		{
			rs.left = (0 - rd.left) * 256 / s->scale;
			rd.left = 0;
		}
		if (rd.right > 640)
		{
			rs.right -= ((rd.right - 640) * 256 / s->scale);
			rd.right = 640;
		}
	}
	else
	{
		rd.right = rd.left + s->w;
		rd.bottom = rd.top + s->h;

		// Do clipping
		if (rd.top < 40)
		{
			rs.top = 40 - rd.top;
			rd.top = 40;
		}
		if (rd.bottom > 440)
		{
			rs.bottom -= (rd.bottom - 440);
			rd.bottom = 440;
		}
		if (rd.left < 0)
		{
			rs.left = 0 - rd.left;
			rd.left = 0;
		}
		if (rd.right > 640)
		{
			rs.right -= (rd.right - 640);
			rd.right = 640;
		}
	}

	if (s->type & RDSPR_TRANS)
	{
		hr = IDirectDrawSurface2_Blt(lpBackBuffer, &rd, dds, &rs, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		if (hr)
		{
			if (hr == DDERR_SURFACELOST)
				hr = RDERR_SURFACELOST;
			else if (dxHalCaps & RDCAPS_BLTSTRETCH)
				dxHalCaps -= RDCAPS_BLTSTRETCH;
			else
			{
				sprintf(myString, "DrawSurface failed, sprite: x%d y%d w%d h%d s%d t%d sx%d sy%d", s->x, s->y, s->w, s->h, s->scale, s->type, scrollx, scrolly);
				DirectDrawError(myString, hr);
			}
		}
	}
	else
	{
		hr = IDirectDrawSurface2_Blt(lpBackBuffer, &rd, dds, &rs, DDBLT_WAIT, NULL);
		if (hr)
		{
			if (hr == DDERR_SURFACELOST)
				hr = RDERR_SURFACELOST;
			else
			{
				sprintf(myString, "DrawSurface failed, sprite: x%d y%d w%d h%d s%d t%d sx%d sy%d", s->x, s->y, s->w, s->h, s->scale, s->type, scrollx, scrolly);
				DirectDrawError(myString, hr);
			}
		}
	}

	return(hr);
*/
	return 0;
}


int32 DeleteSurface(uint32 surface)

{
	warning("stub DeleteSurface( %d )", surface);
/*

	LPDIRECTDRAWSURFACE dds;

	dds = (LPDIRECTDRAWSURFACE) surface;
	return(IDirectDrawSurface2_Release(dds));
*/
	return 0;

}

#define SCALE_MAXWIDTH 512
#define SCALE_MAXHEIGHT 512

uint16 xScale[SCALE_MAXWIDTH];
uint16 yScale[SCALE_MAXHEIGHT];

int32 DrawSprite(_spriteInfo *s) {
	uint8 *src, *dst;
	uint8 *sprite, *newSprite;
	uint8 pixel, red, green, blue;
	uint16 scale;
	int16 i, j;
	uint16 srcPitch;
	bool freeSprite = false;
	ScummVM::Rect rd, rs;

	// FIXME: There are still a few minor details that I think the
	// original did:
	//
	// * Anti-aliasing sprites when upscaling them.
	// * The light mask. If a room has one the sprite should be colored
	//   by it, simulating light sources in the room.
	// * We don't implement the various graphics quality settings at all.
	//
	// But it should be good enough for now.

	// -----------------------------------------------------------------
	// Decompression and mirroring
	// -----------------------------------------------------------------

	if (s->type & RDSPR_NOCOMPRESSION)
		sprite = s->data;
	else {
		sprite = (uint8 *) malloc(s->w * s->h);
		freeSprite = true;
		if (!sprite)
			return RDERR_OUTOFMEMORY;
		if (s->type >> 8 == RDSPR_RLE16 >> 8) {
			if (DecompressRLE16(sprite, s->data, s->w * s->h, s->colourTable)) {
				free(sprite);
				return RDERR_DECOMPRESSION;
			}
		} else {
			if (DecompressRLE256(sprite, s->data, s->w * s->h)) {
				free(sprite);
				return RDERR_DECOMPRESSION;
			}
		}
	}

	if (s->type & RDSPR_FLIP) {
		newSprite = (uint8 *) malloc(s->w * s->h);
		if (newSprite == NULL) {
			if (freeSprite)
				free(sprite);
			return RDERR_OUTOFMEMORY;
		}
		MirrorSprite(newSprite, sprite, s->w, s->h);
		if (freeSprite)
			free(sprite);
		sprite = newSprite;
		freeSprite = true;
	}

	// -----------------------------------------------------------------
	// Positioning and clipping.
	// -----------------------------------------------------------------

	if (!(s->type & RDSPR_DISPLAYALIGN)) {
		s->x += parallaxScrollx;
		s->y += parallaxScrolly;
	}

	s->y += 40;

	// A scale factor 0 or 256 means don't scale. Why do they use two
	// different values to mean the same thing? Normalize it here for
	// convenience.

	scale = (s->scale == 0) ? 256 : s->scale;

	rs.top = 0;
	rs.left = 0;

	if (scale != 256) {
		rs.right = s->scaledWidth;
		rs.bottom = s->scaledHeight;
		srcPitch = s->scaledWidth;
	} else {
		rs.right = s->w;
		rs.bottom = s->h;
		srcPitch = s->w;
	}

	rd.top = s->y;
	rd.left = s->x;

	if (!(s->type & RDSPR_DISPLAYALIGN)) {
		rd.top -= scrolly;
		rd.left -= scrollx;
	}

	rd.right = rd.left + rs.right;
	rd.bottom = rd.top + rs.bottom;

	if (rd.top < 40) {
		rs.top = (40 - rd.top) * 256 / scale;
		rd.top = 40;
	}
	if (rd.bottom > 440) {
		rs.bottom -= ((rd.bottom - 440) * 256 / scale);
		rd.bottom = 440;
	}
	if (rd.left < 0) {
		rs.left = (0 - rd.left) * 256 / scale;
		rd.left = 0;
	}
	if (rd.right > 640) {
		rs.right -= ((rd.right - 640) * 256 / scale);
		rd.right = 640;
	}

	// -----------------------------------------------------------------
	// Scaling
	// -----------------------------------------------------------------

	if (scale != 256) {
		if (s->scaledWidth > SCALE_MAXWIDTH || s->scaledHeight > SCALE_MAXHEIGHT) {
			if (freeSprite)
				free(sprite);
			return RDERR_NOTIMPLEMENTED;
		}

		newSprite = (uint8 *) malloc(s->scaledWidth * s->scaledHeight);
		if (newSprite == NULL) {
			if (freeSprite)
				free(sprite);
			return RDERR_OUTOFMEMORY;
		}

		if (scale < 256) {
			SquashImage(newSprite, s->scaledWidth, s->scaledWidth, s->scaledHeight, sprite, s->w, s->w, s->h);
		} else {
			if (s->scale > 512) {
				if (freeSprite)
					free(sprite);
				return RDERR_INVALIDSCALING;
			}
			StretchImage(newSprite, s->scaledWidth, s->scaledWidth, s->scaledHeight, sprite, s->w, s->w, s->h);
		}

		if (freeSprite)
			free(sprite);
		sprite = newSprite;
		freeSprite = true;
	}

	// -----------------------------------------------------------------
	// Drawing
	// -----------------------------------------------------------------

	src = sprite + rs.top * s->w + rs.left;
	dst = (uint8 *) lpBackBuffer->_pixels + lpBackBuffer->_width * rd.top + rd.left;

	if (s->type & RDSPR_BLEND) {
		if (s->blend & 0x01) {
			red = s->blend >> 8;
			for (i = 0; i < rs.bottom - rs.top; i++) {
				for (j = 0; j < rs.right - rs.left; j++) {
					if (src[j]) {
						pixel = dst[j];
						dst[j] = paletteMatch[(((palCopy[src[j]][0] * red + palCopy[pixel][0] * (8 - red)) >> 5) << 12) +
							(((palCopy[src[j]][1] * red + palCopy[pixel][1] * (8 - red)) >> 5) << 6) +
							(((palCopy[src[j]][2] * red + palCopy[pixel][2] * (8 - red)) >> 5))];
					}
				}
				src += srcPitch;
				dst += lpBackBuffer->_width;
			}
		} else if (s->blend & 0x02) {
			red = palCopy[s->blend >> 8][0];
			green = palCopy[s->blend >> 8][0];
			blue = palCopy[s->blend >> 8][0];
			for (i = 0; i < rs.bottom - rs.top; i++) {
				for (j = 0; j < rs.right - rs.left; j++) {
					if (src[j]) {
						pixel = dst[j];
						dst[j] = paletteMatch[((((src[j] * red + (16 - src[j]) * palCopy[pixel][0]) >> 4) >> 2) << 12) +
							((((src[j] * green + (16 - src[j]) * palCopy[pixel][1]) >> 4) >> 2) << 6) +
							(((src[j] * blue + (16 - src[j]) * palCopy[pixel][2]) >> 4) >> 2)];
					}
				}
				src += srcPitch;
				dst += lpBackBuffer->_width;
			}
		} else {
			warning("DrawSprite: Invalid blended sprite");
			if (freeSprite)
				free(sprite);
			return RDERR_UNKNOWNTYPE;
		}
	} else {
		if (s->type & RDSPR_TRANS) {
			for (i = 0; i < rs.bottom - rs.top; i++) {
				for (j = 0; j < rs.right - rs.left; j++) {
					if (src[j])
						dst[j] = src[j];
				}
				src += srcPitch;
				dst += lpBackBuffer->_width;
			}
		} else {
			for (i = 0; i < rs.bottom - rs.top; i++) {
				memcpy(dst, src, rs.right - rs.left);
				src += srcPitch;
				dst += lpBackBuffer->_width;
			}
		}
	}

	if (freeSprite)
		free(sprite);

	lpBackBuffer->upload(&rd);

/*

#if PROFILING == 1
	long int startTime, endTime;

	QueryPerformanceCounter(&startTime);
#endif

	if (!(s->type & RDSPR_DISPLAYALIGN))
	{
		s->x += parallaxScrollx;
		s->y += parallaxScrolly;
	}

	if (renderCaps & RDBLTFX_ALLHARDWARE)
	{
		uint8			*src, *dst;
		uint8			*sprite, *newSprite;
		uint8			pixel, red, green, blue;
		int16			i, j;
		int16			flag = 0;
		int16			freeSprite = 0;
	//	int32			start, total;		// For timing!
		RECT			rd, rs;
		HRESULT			hr;
		DDSURFACEDESC	ddsd;
		LPDIRECTDRAWSURFACE dds;

		s->y += 40;

		//	For now, we draw blended sprites with our own code, as they
		//	are not supported by DX3.
		if ((s->type & RDSPR_BLEND) && (renderCaps & RDBLTFX_FLATALPHA))
		{
			if (s->type & RDSPR_NOCOMPRESSION)
				sprite = s->data;
			else
			{
				sprite = malloc(s->w * s->h);
				if (sprite == NULL)
					return(RDERR_OUTOFMEMORY);
				freeSprite = 1;
				if (s->type >> 8 == RDSPR_RLE16 >> 8)
				{
					if (DecompressRLE16(sprite, s->data, s->w * s->h, s->colourTable))
						return(RDERR_DECOMPRESSION);
				}
				else
				{
					if (DecompressRLE256(sprite, s->data, s->w * s->h))
						return(RDERR_DECOMPRESSION);
				}
			}

			// We want to blend the sprite FROM the RECT rs.
			// We want to blend the sprite TO   the RECT rd.
			rd.left = s->x - scrollx;
			rd.right = rd.left + s->w;
			rd.top = s->y - scrolly;
			rd.bottom = rd.top + s->h;

			rs.top = 0;
			rs.bottom = s->h;
			rs.left = 0;
			rs.right = s->w;

			//Now do the clipping - top
			if (rd.top < 40)
			{
				rs.top = (40 - rd.top);
				rd.top = 40;
			}
			//Clip the bottom
			if (rd.bottom > RENDERDEEP)
			{
				rs.bottom -= (rd.bottom - RENDERDEEP);
				rd.bottom = RENDERDEEP;
			}
			//Clip the left
			if (rd.left < 0)
			{
				rs.left -= rd.left;
				rd.left = 0;
			}
			//Clip the right
			if (rd.right > RENDERWIDE)
			{
				rs.right -= (rd.right - RENDERWIDE);
				rd.right = RENDERWIDE;
			}

	//		start = timeGetTime();
			memset(&ddsd, 0, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
			if (hr != DD_OK)
			{
				hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
			}
			if (hr == DD_OK)
			{
				if (s->blend & 0x01)
				{
					red = s->blend >> 8;
					for (i=0; i<rs.bottom - rs.top; i++)
					{
						src = sprite + (rs.top + i) * s->w + rs.left;
						dst = (uint8 *) ddsd.lpSurface + ddsd.lPitch * (rd.top + i) + rd.left;
		//				newSprite = (uint8 *) ddsdSystem.lpSurface + ddsdSystem.lPitch * i;
						for (j=0; j<rs.right - rs.left; j++)
						{
							if (*src)
							{
								pixel = *dst;
		//						pixel = *newSprite;
								*dst = paletteMatch[(((palCopy[*src][0] * red + palCopy[pixel][0] * (8 - red)) >> 5) << 12) +
													(((palCopy[*src][1] * red + palCopy[pixel][1] * (8 - red)) >> 5) << 6) +
													(((palCopy[*src][2] * red + palCopy[pixel][2] * (8 - red)) >> 5)     )];
							}
							src++;
							dst++;
		//					newSprite++;
						}
					}
				}
				else if (s->blend & 0x02)
				{
					red = palCopy[s->blend >> 8][0];
					green = palCopy[s->blend >> 8][0];
					blue = palCopy[s->blend >> 8][0];
					for (i=0; i<rs.bottom - rs.top; i++)
					{
						src = sprite + (rs.top + i) * s->w + rs.left;
						dst = (uint8 *) ddsd.lpSurface + ddsd.lPitch * (rd.top + i) + rd.left;
		//				newSprite = (uint8 *) ddsdSystem.lpSurface + ddsdSystem.lPitch * i;
						for (j=0; j<rs.right - rs.left; j++)
						{
							if (*src)
							{
								pixel = *dst;
		//						pixel = *newSprite;
								*dst = paletteMatch[((((*src * red + (16 - *src) * palCopy[pixel][0]) >> 4) >> 2) << 12) +
													((((*src * green + (16 - *src) * palCopy[pixel][1]) >> 4) >> 2) << 6) +
													(((*src * blue + (16 - *src) * palCopy[pixel][2]) >> 4) >> 2)];
							}
							src++;
							dst++;
		//					newSprite++;
						}
					}
				}
				else
				{
					IDirectDrawSurface2_Unlock(lpBackBuffer, ddsd.lpSurface);
					if (freeSprite)
						free(sprite);
					DirectDrawError("Invalid blended sprite", 0);
					return(RDERR_UNKNOWNTYPE);
				}
				IDirectDrawSurface2_Unlock(lpBackBuffer, ddsd.lpSurface);
			}
			
	//		IDirectDrawSurface2_Unlock(ddsSystem, ddsdSystem.lpSurface);
	//		IDirectDrawSurface2_Release(ddsSystem);

	//		total = timeGetTime() - start;
	//		PlotDots(10, 14, total);

			if (freeSprite)
				free(sprite);

		}
		else
		{

			//	Create a surface for the sprite and then draw it.
			memset(&ddsd, 0, sizeof(DDSURFACEDESC));
			ddsd.dwSize = sizeof(DDSURFACEDESC);
			ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			if (s->scale & 0xff)
			{
				if (dxHalCaps & RDCAPS_BLTSTRETCH)
					ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
				else
					ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			}
			else
			{
				if (dxHalCaps & RDCAPS_SRCBLTCKEY)
					ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
				else
					ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			}

			ddsd.dwWidth = s->w;
			ddsd.dwHeight = s->h;
			hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, &dds, NULL);
			if (hr == DDERR_OUTOFVIDEOMEMORY)
			{
				ddsd.ddsCaps.dwCaps &= (0xffffffff - DDSCAPS_VIDEOMEMORY);
				ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
				hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, &dds, NULL);
			}
			if (hr != DD_OK)
			{
				DirectDrawError("Cannot create sprite surface", hr);
				return(hr);
			}

			if (s->type & RDSPR_TRANS)
				hr = IDirectDrawSurface2_SetColorKey(dds, DDCKEY_SRCBLT, &blackColorKey);

			if (s->type &RDSPR_NOCOMPRESSION)
			{
				sprite = s->data;
			}
			else
			{
				sprite = (uint8 *) malloc(s->w * s->h);
				freeSprite = 1;
				if (sprite == NULL)
				{
					IDirectDrawSurface2_Release(dds);
					return(RDERR_OUTOFMEMORY);
				}
				if (s->type >> 8 == RDSPR_RLE16 >> 8)
				{
					if (DecompressRLE16(sprite, s->data, s->w * s->h, s->colourTable))
						return(RDERR_DECOMPRESSION);
				}
				else
				{
					if (DecompressRLE256(sprite, s->data, s->w * s->h))
						return(RDERR_DECOMPRESSION);
				}

				if (s->type & RDSPR_FLIP)
				{
					newSprite = (uint8 *) malloc(s->w * s->h);
					if (newSprite == NULL)
					{
						free(sprite);
						return(RDERR_OUTOFMEMORY);
					}
					MirrorSprite(newSprite, sprite, s->w, s->h);
					free(sprite);
					sprite = newSprite;
				}
			}

			hr = IDirectDrawSurface2_Lock(dds, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
			if (hr != DD_OK)
			{
				IDirectDrawSurface2_Restore(dds);
				hr = IDirectDrawSurface2_Lock(dds, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
				if (hr != DD_OK)
				{
					DirectDrawError("Cannot lock sprite surface", hr);
					return(hr);
				}
			}

			flag = 0;
			if (s->type & RDSPR_BLEND)
			{
				for (i=0; i<s->h; i++)
				{
					src = sprite + i * s->w;
					dst = (uint8 *) ddsd.lpSurface + ddsd.lPitch * i;
					flag ^= 1;
					if (flag)
					{
						j=0;
						while (j < s->w)
						{
							*dst = *src;
							if (++j == s->w)
								break;
							*(dst+1) = 0;
							j++;
							dst += 2;
							src += 2;
						}

//						for (j=0; j<(s->w-1)/2; j++)
//						{
//							*dst = *src;
//							*(dst+1) = 0;
//							dst += 2;
//							src += 2;
//						}
					}
					else
					{
//						dst += 1;
//						src += 1;
						j = 0;
						while (j < s->w)
						{
							*dst = 0;
							if (++j == s->w)
								break;
							*(dst + 1) = *(src + 1);
							j++;
							dst += 2;
							src += 2;
						}
//						for (j=0; j<s->w/2; j++)
//						{
//							*dst = 0;
//							*(dst+1) = *(src+1);
//							dst += 2;
//							src += 2;
//						}
					}
				}
			}
			else
			{
				src = sprite;
				dst = (uint8 *) ddsd.lpSurface;
				for (i=0; i<s->h; i++)
				{
					memcpy(dst, src, s->w);
					src += s->w;
					dst += ddsd.lPitch;
				}
			}

			IDirectDrawSurface2_Unlock(dds, ddsd.lpSurface);

			// Set startx and starty for the screen buffer		ADDED THIS!
			if (s->type & RDSPR_DISPLAYALIGN)
				rd.top = s->y;
			else
				rd.top = s->y - scrolly;
				
			if (s->type & RDSPR_DISPLAYALIGN)
				rd.left = s->x;
			else
				rd.left = s->x - scrollx;

			rs.left = 0;
			rs.right = s->w;
			rs.top = 0;
			rs.bottom = s->h;
			if (s->scale & 0xff)
			{
				rd.right = rd.left + s->scaledWidth;
				rd.bottom = rd.top + s->scaledHeight;
				// Do clipping
				if (rd.top < 40)
				{
					rs.top = (40 - rd.top) * 256 / s->scale;
					rd.top = 40;
				}
				if (rd.bottom > 440)
				{
					rs.bottom -= ((rd.bottom - 440) * 256 / s->scale);
					rd.bottom = 440;
				}
				if (rd.left < 0)
				{
					rs.left = (0 - rd.left) * 256 / s->scale;
					rd.left = 0;
				}
				if (rd.right > 640)
				{
					rs.right -= ((rd.right - 640) * 256 / s->scale);
					rd.right = 640;
				}
			}
			else
			{
				rd.right = rd.left + s->w;
				rd.bottom = rd.top + s->h;

				// Do clipping
				if (rd.top < 40)
				{
					rs.top = 40 - rd.top;
					rd.top = 40;
				}
				if (rd.bottom > 440)
				{
					rs.bottom -= (rd.bottom - 440);
					rd.bottom = 440;
				}
				if (rd.left < 0)
				{
					rs.left = 0 - rd.left;
					rd.left = 0;
				}
				if (rd.right > 640)
				{
					rs.right -= (rd.right - 640);
					rd.right = 640;
				}
			}

			if (s->type & RDSPR_TRANS)
			{
				hr = IDirectDrawSurface2_Blt(lpBackBuffer, &rd, dds, &rs, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
				if (hr)
					if (dxHalCaps & RDCAPS_BLTSTRETCH)
						dxHalCaps -= RDCAPS_BLTSTRETCH;
			}
			else
				hr = IDirectDrawSurface2_Blt(lpBackBuffer, &rd, dds, &rs, DDBLT_WAIT, NULL);

			IDirectDrawSurface2_Release(dds);
			if (freeSprite)
				free(sprite);
		}

	}
	else
	{
		uint16			i, j, k;
		int16			starty, endy;
		int16			startx, endx;
		uint8			*src, *dst;
		uint8			*lightMap;
		uint8			*srcExtra[4];
		uint8			*sprite, *newSprite, *newerSprite, *spr, *p;
		uint8			pixel;
		int32			dx, dy, ince, incne, d;
		int16			x, y;
		static int16	randCount = 0;
		int16			count, spriteCount;
		int16			red, green, blue;
		uint8			pt[5];
		RECT			clippedTarget, clippedSprite;
		int32			diff;

//		if (!(s->type & RDSPR_DISPLAYALIGN))
//		{
//			s->x += parallaxScrollx;
//			s->y += parallaxScrolly;
//		}

		if (s->type & RDSPR_TRANS)
		{

			if (s->type & RDSPR_NOCOMPRESSION)
			{

				// We want to blend the sprite FROM the RECT clippedSprite.
				// We want to blend the sprite TO   the RECT clippedTarget.
				clippedSprite.top = 0;
				clippedSprite.bottom = s->h;
				clippedSprite.left = 0;
				clippedSprite.right = s->w;
				if (s->type & RDSPR_DISPLAYALIGN)
					clippedTarget.top = s->y;
				else
					clippedTarget.top = s->y - scrolly;
					
				if (s->type & RDSPR_DISPLAYALIGN)
					clippedTarget.left = s->x;
				else
					clippedTarget.left = s->x - scrollx;

				clippedTarget.right = clippedTarget.left + s->w;
				clippedTarget.bottom = clippedTarget.top + s->h;

				//Now do the clipping - top
				if (clippedTarget.top < 0)
				{
					clippedSprite.top -= clippedTarget.top;
					clippedTarget.top = 0;
				}
				//Clip the bottom
				if (clippedTarget.bottom > RENDERDEEP)
				{
					clippedSprite.bottom -= (clippedTarget.bottom - RENDERDEEP);
					clippedTarget.bottom = RENDERDEEP;
				}
				//Clip the left
				if (clippedTarget.left < 0)
				{
					clippedSprite.left -= clippedTarget.left;
					clippedTarget.left = 0;
				}
				//Clip the right
				if (clippedTarget.right > RENDERWIDE)
				{
					clippedSprite.right -= (clippedTarget.right - RENDERWIDE);
					clippedTarget.right = RENDERWIDE;
				}

				if (s->type & RDSPR_BLEND)
				{
					if (renderCaps & RDBLTFX_FLATALPHA)
					{
						red = s->blend >> 8;
						for (i=0; i<clippedSprite.bottom - clippedSprite.top; i++)
						{
							src = s->data + (clippedSprite.top + i) * s->w + clippedSprite.left;
							dst = myScreenBuffer + RENDERWIDE * (clippedTarget.top + i) + clippedTarget.left;
			//				newSprite = (uint8 *) ddsdSystem.lpSurface + ddsdSystem.lPitch * i;
							for (j=0; j<clippedSprite.right - clippedSprite.left; j++)
							{
								if (*src)
								{
									pixel = *dst;
			//						pixel = *newSprite;
									*dst = paletteMatch[(((palCopy[*src][0] * red + palCopy[pixel][0] * (8 - red)) >> 5) << 12) +
														(((palCopy[*src][1] * red + palCopy[pixel][1] * (8 - red)) >> 5) << 6) +
														(((palCopy[*src][2] * red + palCopy[pixel][2] * (8 - red)) >> 5)     )];
								}
								src++;
								dst++;
			//					newSprite++;
							}
						}
					}
				}
				else
				{
					for (i=0; i<clippedSprite.bottom - clippedSprite.top; i++)
					{
						src = s->data + (clippedSprite.top + i) * s->w + clippedSprite.left;
						dst = myScreenBuffer + RENDERWIDE * (clippedTarget.top + i) + clippedTarget.left;
						for (j=0; j<clippedSprite.right - clippedSprite.left; j++)
						{
							if (*src)
								*dst = *src;
							dst++;
							src++;
						}
					}
				}
			}
			else
			{

				//  Ok, the sprite is compressed, so lets find out what type
				switch (s->type >> 8)
				{

				case (RDSPR_RLE256FAST >> 8):
					SoftwareRenderCompressed256(s);
					break;

				case (RDSPR_RLE256 >> 8):
				case (RDSPR_RLE16>>8):
				
					if (s->type & RDSPR_DISPLAYALIGN)
						return(RDERR_NOTIMPLEMENTED);

					sprite = malloc(s->w * s->h);
					if (sprite == NULL)
						return(RDERR_OUTOFMEMORY);

					if (s->type >> 8 == RDSPR_RLE16 >> 8)
					{
						if (DecompressRLE16(sprite, s->data, s->w * s->h, s->colourTable))
							return(RDERR_DECOMPRESSION);
					}
					else
					{
						if (DecompressRLE256(sprite, s->data, s->w * s->h))
							return(RDERR_DECOMPRESSION);
					}

					if (s->type & RDSPR_FLIP)
					{
						newSprite = (uint8 *) malloc(s->w * s->h);
						if (newSprite == NULL)
						{
							free(sprite);
							return(RDERR_OUTOFMEMORY);
						}
						MirrorSprite(newSprite, sprite, s->w, s->h);
						free(sprite);
						sprite = newSprite;
					}

					if ((s->scale == 0) || (s->scale == 256))
					{
						if ((s->type & RDSPR_EDGEBLEND) && (renderCaps & RDBLTFX_EDGEBLEND))
						{
							// The sprite is to be edge blended.  Let's get a copy of the background
							// into a new sprite buffer, and write the sprite over it with edge blending
							newSprite = malloc(s->w * s->h);
							if (newSprite == NULL)
							{
								free(sprite);
								return(RDERR_OUTOFMEMORY);
							}

							// RIGHT, SORT THIS FUCKING MESS OUT.
							// We want to draw the sprite FROM the RECT clippedSprite.
							// We want to draw the sprite TO   the RECT clippedTarget.
							clippedTarget.left = s->x - scrollx;
							clippedTarget.right = clippedTarget.left + s->w;
							clippedTarget.top = s->y - scrolly;
							clippedTarget.bottom = clippedTarget.top + s->h;

							clippedSprite.top = 0;
							clippedSprite.bottom = s->h;
							clippedSprite.left = 0;
							clippedSprite.right = s->w;

							//Now do the clipping - top
							if (clippedTarget.top < 0)
							{
								clippedSprite.top -= clippedTarget.top;
								clippedTarget.top = 0;
							}
							//Clip the bottom
							if (clippedTarget.bottom > RENDERDEEP)
							{
								clippedSprite.bottom -= (clippedTarget.bottom - RENDERDEEP);
								clippedTarget.bottom = RENDERDEEP;
							}
							//Clip the left
							if (clippedTarget.left < 0)
							{
								clippedSprite.left -= clippedTarget.left;
								clippedTarget.left = 0;
							}
							//Clip the right
							if (clippedTarget.right > RENDERWIDE)
							{
								clippedSprite.right -= (clippedTarget.right - RENDERWIDE);
								clippedTarget.right = RENDERWIDE;
							}

							// We now have our clipped regions, so let's copy the background to the sprite.
							for (i=0; i<clippedSprite.bottom - clippedSprite.top; i++)
							{
								memcpy(newSprite + s->w * i + clippedSprite.left, myScreenBuffer + RENDERWIDE * (clippedTarget.top+i) + clippedTarget.left, clippedSprite.right - clippedSprite.left);
							}


							// Now put the sprite onto the background sprite!?!
							src = sprite + (clippedSprite.top + 1) * s->w + clippedSprite.left + 1;
							srcExtra[0] = src - s->w;
							srcExtra[1] = src - 1;
							srcExtra[2] = src + 1;
							srcExtra[3] = src + s->w;
							dst = newSprite + (clippedSprite.top + 1) * s->w + clippedSprite.left + 1;
							for (j=1; j<clippedSprite.bottom - clippedSprite.top - 1; j++)
							{
								for (i=1; i<clippedSprite.right - clippedSprite.left - 1; i++)
								{
									if (*src)
									{
										uint8 newp = *src;
										count = 1;

										red = palCopy[newp][0];
										green = palCopy[newp][1];
										blue = palCopy[newp][2];
										for (k=0; k<4; k++)
										{
											if (newp = *srcExtra[k])
											{
												red += palCopy[newp][0];
												green += palCopy[newp][1];
												blue += palCopy[newp][2];
												count += 1;
											}
											else
											{
												newp = *(dst - (src - srcExtra[k]));
												red += palCopy[newp][0];
												green += palCopy[newp][1];
												blue += palCopy[newp][2];
											}
										}

										if (count != 5)
											*dst = QuickMatch((uint8) (red / 5), (uint8) (green / 5), (uint8) (blue / 5));
										else
											*dst = *src;
									}
									src++;
									dst++;
									for (k=0; k<4; k++)
										srcExtra[k]++;
								}
								diff = (s->w - (clippedSprite.right - clippedSprite.left)) + 2;
								for (k=0; k<4; k++)
									srcExtra[k] += diff;
								src += diff;
								dst += diff;
							}

							// And copy the sprite back to the screen
							for (i=0; i<clippedSprite.bottom - clippedSprite.top; i++)
							{
								memcpy(myScreenBuffer + RENDERWIDE * (clippedTarget.top+i) + clippedTarget.left, newSprite + s->w * i + clippedSprite.left, clippedSprite.right - clippedSprite.left);
							}

							free(newSprite);
						}
						else if (s->type & RDSPR_BLEND)
						{

							// We want to blend the sprite FROM the RECT clippedSprite.
							// We want to blend the sprite TO   the RECT clippedTarget.
							clippedTarget.left = s->x - scrollx;
							clippedTarget.right = clippedTarget.left + s->w;
							clippedTarget.top = s->y - scrolly;
							clippedTarget.bottom = clippedTarget.top + s->h;

							clippedSprite.top = 0;
							clippedSprite.bottom = s->h;
							clippedSprite.left = 0;
							clippedSprite.right = s->w;

							//Now do the clipping - top
							if (clippedTarget.top < 0)
							{
								clippedSprite.top -= clippedTarget.top;
								clippedTarget.top = 0;
							}
							//Clip the bottom
							if (clippedTarget.bottom > RENDERDEEP)
							{
								clippedSprite.bottom -= (clippedTarget.bottom - RENDERDEEP);
								clippedTarget.bottom = RENDERDEEP;
							}
							//Clip the left
							if (clippedTarget.left < 0)
							{
								clippedSprite.left -= clippedTarget.left;
								clippedTarget.left = 0;
							}
							//Clip the right
							if (clippedTarget.right > RENDERWIDE)
							{
								clippedSprite.right -= (clippedTarget.right - RENDERWIDE);
								clippedTarget.right = RENDERWIDE;
							}

							if (s->blend & 0x01)
							{
								if (renderCaps & RDBLTFX_FLATALPHA)
								{
									red = s->blend >> 8;
									for (i=0; i<clippedSprite.bottom - clippedSprite.top; i++)
									{
										src = sprite + (clippedSprite.top + i) * s->w + clippedSprite.left;
										dst = myScreenBuffer + RENDERWIDE * (clippedTarget.top + i) + clippedTarget.left;
						//				newSprite = (uint8 *) ddsdSystem.lpSurface + ddsdSystem.lPitch * i;
										for (j=0; j<clippedSprite.right - clippedSprite.left; j++)
										{
											if (*src)
											{
												pixel = *dst;
						//						pixel = *newSprite;
												*dst = paletteMatch[(((palCopy[*src][0] * red + palCopy[pixel][0] * (8 - red)) >> 5) << 12) +
																	(((palCopy[*src][1] * red + palCopy[pixel][1] * (8 - red)) >> 5) << 6) +
																	(((palCopy[*src][2] * red + palCopy[pixel][2] * (8 - red)) >> 5)     )];
											}
											src++;
											dst++;
						//					newSprite++;
										}
									}
								}
								else
								{
									for (i=0; i<clippedSprite.bottom - clippedSprite.top; i++)
									{
										src = sprite + (clippedSprite.top + i) * s->w + clippedSprite.left;
										dst = myScreenBuffer + RENDERWIDE * (clippedTarget.top + i) + clippedTarget.left;
						//				newSprite = (uint8 *) ddsdSystem.lpSurface + ddsdSystem.lPitch * i;
										for (j=0; j<clippedSprite.right - clippedSprite.left; j++)
										{
											if (*src)
											{
												*dst = *src;
											}
											src++;
											dst++;
										}
									}
								}
							}
							else if (s->blend & 0x02)
							{
								red = palCopy[s->blend >> 8][0];
								green = palCopy[s->blend >> 8][0];
								blue = palCopy[s->blend >> 8][0];


								if (renderCaps & RDBLTFX_GRADEDALPHA)
								{
									for (i=0; i<clippedSprite.bottom - clippedSprite.top; i++)
									{
										src = sprite + (clippedSprite.top + i) * s->w + clippedSprite.left;
										dst = myScreenBuffer + RENDERWIDE * (clippedTarget.top + i) + clippedTarget.left;
										for (j=0; j<clippedSprite.right - clippedSprite.left; j++)
										{
											if (*src)
											{
												*dst = paletteMatch[((((*src * red + (16 - *src) * palCopy[*(dst)][0]) >> 4) >> 2) << 12) +
																	((((*src * green + (16 - *src) * palCopy[*(dst)][1]) >> 4) >> 2) << 6) +
																	(((*src * blue + (16 - *src) * palCopy[*(dst)][2]) >> 4) >> 2)];

											}
											src++;
											dst++;
										}
									}
								}
								else
								{
									char col = paletteMatch[((36 >> 2) << 12) + ((35 >> 2) << 6) + (34 >> 2)];
									for (i=0; i<clippedSprite.bottom - clippedSprite.top; i++)
									{
										src = sprite + (clippedSprite.top + i) * s->w + clippedSprite.left;
										dst = myScreenBuffer + RENDERWIDE * (clippedTarget.top + i) + clippedTarget.left;
										for (j=0; j<clippedSprite.right - clippedSprite.left; j++)
										{
											if (*src)
											{
												*dst = col;
											}
											src++;
											dst++;
										}
									}
								}
							}
							else
							{
								free(sprite);
								DirectDrawError("Invalid blended sprite params", RDERR_UNKNOWNTYPE);
								return(RDERR_UNKNOWNTYPE);
							}



						}
						else
						{

							starty = s->y - scrolly;
							endy = starty + s->h;
							if (starty < 0)
								starty = 0;
							if (endy > RENDERDEEP)
								endy = RENDERDEEP;

							startx = s->x - scrollx;
							endx = startx + s->w;
							if (startx < 0)
								startx = 0;
							if (endx > RENDERWIDE)
								endx = RENDERWIDE;

							for (i=starty; i<endy; i++)
							{
								dst = myScreenBuffer + i * RENDERWIDE+ startx;
								src = sprite + (i - (s->y - scrolly)) * s->w + (startx - (s->x - scrollx));
								if ((s->type & RDSPR_SHADOW) && (lightMask) && (renderCaps & RDBLTFX_SHADOWBLEND))
								{
									lightMap = lightMask + (i+scrolly) * locationWide + s->x;
									for (j=startx; j<endx; j++)
									{
										if (*src)
										{
											if (*lightMap)
												*dst = QuickMatch((uint8) (((int32) (32 - *lightMap) * palCopy[*src][0]) >> 5), 
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][1]) >> 5),
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][2]) >> 5));
											else
												*dst = *src;
										}
										dst++;
										src++;
										lightMap++;
									}
								}
								else
								{
									for (j=startx; j<endx; j++)
									{
										if (*src)
											*dst = *src;
										dst++;
										src++;
									}
								}
							}
						}
					}
					else if (s->scale < 256) // Draw the sprite with scaling and arithmetic stretching
					{
						if (renderCaps & RDBLTFX_ARITHMETICSTRETCH)
						{

							if ((s->scaledWidth > SCALE_MAXWIDTH) || (s->scaledHeight > SCALE_MAXHEIGHT))
							{
								free(sprite);
								return(RDERR_NOTIMPLEMENTED);
							}

							// Create the buffer to copy the small sprite into
							newSprite = malloc(s->scaledWidth * s->scaledHeight);
							if (newSprite == NULL)
							{
								free(sprite);
								return(RDERR_OUTOFMEMORY);
							}

							// Work out the x-scale
							dx = s->w;
							dy = s->scaledWidth;
							ince = 2 * dy;
							incne = 2 * (dy - dx);
							d = 2 * dy - dx;

							x = 0;
							y = 0;
							xScale[y] = x;
							while (x < s->w)
							{
								if (d <= 0)
								{
									d += ince;
									x += 1;
								}
								else
								{
									d += incne;
									x += 1;
									y += 1;
								}
								xScale[y] = x;
							}

							// Work out the y-scale
							dx = s->h;
							dy = s->scaledHeight;
							ince = 2 * dy;
							incne = 2 * (dy - dx);
							d = 2 * dy - dx;

							x = 0;
							y = 0;
							yScale[y] = x;
							while (x < s->h)
							{
								if (d <= 0)
								{
									d += ince;
									x += 1;
								}
								else
								{
									d += incne;
									x += 1;
									y += 1;
								}
								yScale[y] = x;
							}

							// If the sprite is clipped, then don't try to anti-alias it and arithmetic stretch.
							//	Just go for the quick and dirty squash!
							if ((s->x - scrollx < 0) || (s->x - scrollx + s->scaledWidth > RENDERWIDE) ||
								 (s->y - scrolly < 0) || (s->y - scrolly + s->scaledHeight > RENDERDEEP))
							{
								spr = newSprite;
								for (y=0; y<s->scaledHeight; y++)
								{
									for (x=0; x<s->scaledWidth; x++)
									{
										*spr++ = *(sprite + yScale[y] * s->w + xScale[x]);
									}
								}
							}
							else
							{
								spr = newSprite;
								for (y=0; y<s->scaledHeight; y++)
								{
									for (x=0; x<s->scaledWidth; x++)
									{
										count = 0;
										spriteCount = 0;
										red = 0;
										green = 0;
										blue = 0;
										for (j=yScale[y]; j<yScale[y+1]; j++)
										{
											for (i=xScale[x]; i<xScale[x+1]; i++)
											{
												p = sprite + j * s->w + i;
												if (*p == 0)
												{
													uint8 newp;

													newp = myScreenBuffer[(s->y - scrolly + y) * RENDERWIDE + s->x - scrollx + x];
													red += palCopy[newp][0];
													green += palCopy[newp][1];
													blue += palCopy[newp][2];
												}
												else
												{
													red += palCopy[*p][0];
													green += palCopy[*p][1];
													blue += palCopy[*p][2];
													spriteCount += 1;
												}
												count += 1;
											}
										}
										if (spriteCount == 0)
										{
											*spr++ = 0;
										}
										else if (count == 1)
										{
											*spr++ = *p;
										}
										else
										{
											*spr++ = QuickMatch((uint8) (red / count), (uint8) (green / count), (uint8) (blue / count));
										}
									}
								}
							}

							// Draw the sprite
							starty = s->y - scrolly;
							endy = starty + s->scaledHeight;
							if (starty < 0)
								starty = 0;
							if (endy > RENDERDEEP)
								endy = RENDERDEEP;

							startx = s->x - scrollx;
							endx = startx + s->scaledWidth;
							if (startx < 0)
								startx = 0;
							if (endx > RENDERWIDE)
								endx = RENDERWIDE;

							for (i=starty; i<endy; i++)
							{
								dst = myScreenBuffer + i * RENDERWIDE+ startx;
								src = newSprite + (i - (s->y - scrolly)) * s->scaledWidth + (startx - (s->x - scrollx));

								if ((lightMask) && (renderCaps & RDBLTFX_SHADOWBLEND))
								{
									lightMap = lightMask + (i+scrolly) * locationWide + s->x;
									for (j=startx; j<endx; j++)
									{
										if (*src)
										{
											if (*lightMap)
												*dst = QuickMatch((uint8) (((int32) (32 - *lightMap) * palCopy[*src][0]) >> 5), 
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][1]) >> 5),
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][2]) >> 5));
											else
												*dst = *src;
										}
										dst++;
										src++;
										lightMap++;
									}
								}
								else
								{


									for (j=startx; j<endx; j++)
									{
										if (*src)
											*dst = *src;
										dst++;
										src++;
									}
								}
							}
							
							free(newSprite);

						}
						else		//  Draw scaled, value less than 256, line doubling
						{
							if ((s->scaledWidth > SCALE_MAXWIDTH) || (s->scaledHeight > SCALE_MAXHEIGHT))
							{
								free(sprite);
								return(RDERR_NOTIMPLEMENTED);
							}

							// Create the buffer to copy the small sprite into
							newSprite = malloc(s->scaledWidth * s->scaledHeight);
							if (newSprite == NULL)
							{
								free(sprite);
								return(RDERR_OUTOFMEMORY);
							}

							// Work out the x-scale
							dx = s->w;
							dy = s->scaledWidth;
							ince = 2 * dy;
							incne = 2 * (dy - dx);
							d = 2 * dy - dx;

							x = 0;
							y = 0;
							xScale[y] = x;
							while (x < s->w)
							{
								if (d <= 0)
								{
									d += ince;
									x += 1;
								}
								else
								{
									d += incne;
									x += 1;
									y += 1;
								}
								xScale[y] = x;
							}

							// Work out the y-scale
							dx = s->h;
							dy = s->scaledHeight;
							ince = 2 * dy;
							incne = 2 * (dy - dx);
							d = 2 * dy - dx;

							x = 0;
							y = 0;
							yScale[y] = x;
							while (x < s->h)
							{
								if (d <= 0)
								{
									d += ince;
									x += 1;
								}
								else
								{
									d += incne;
									x += 1;
									y += 1;
								}
								yScale[y] = x;
							}

							// Copy the sprite
		//					InitialiseColourMatch(s->colourTable);
							spr = newSprite;
							for (y=0; y<s->scaledHeight; y++)
							{
								for (x=0; x<s->scaledWidth; x++)
								{
									*spr++ = *(sprite + yScale[y] * s->w + xScale[x]);
								}
							}

							// Draw the sprite
							starty = s->y - scrolly;
							endy = starty + s->scaledHeight;
							if (starty < 0)
								starty = 0;
							if (endy > RENDERDEEP)
								endy = RENDERDEEP;

							startx = s->x - scrollx;
							endx = startx + s->scaledWidth;
							if (startx < 0)
								startx = 0;
							if (endx > RENDERWIDE)
								endx = RENDERWIDE;

							for (i=starty; i<endy; i++)
							{
								dst = myScreenBuffer + i * RENDERWIDE+ startx;
								src = newSprite + (i - (s->y - scrolly)) * s->scaledWidth + (startx - (s->x - scrollx));
								if ((lightMask) && (renderCaps & RDBLTFX_SHADOWBLEND))
								{
									lightMap = lightMask + (i + scrolly) * locationWide + s->x;
									for (j=startx; j<endx; j++)
									{
										if (*src)
										{
											if (*lightMap)
												*dst = QuickMatch((uint8) (((int32) (32 - *lightMap) * palCopy[*src][0]) >> 5), 
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][1]) >> 5),
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][2]) >> 5));
											else
												*dst = *src;
										}
										dst++;
										src++;
										lightMap++;
									}
								}
								else
								{
									for (j=startx; j<endx; j++)
									{
										if (*src)
											*dst = *src;
										dst++;
										src++;
									}
								}
							}
							
							free(newSprite);
						}

					}
					else //if (s->scale > 256) 
					{
						if (s->scale > 512)
						{
							free(sprite);
							return(RDERR_INVALIDSCALING);
						}

						if (renderCaps & RDBLTFX_ARITHMETICSTRETCH)	// Draw the sprite with scaling and anti-aliasing
						{

							// Create the buffer to copy the bigger sprite into
							newSprite = malloc(s->scaledWidth * s->scaledHeight);
							if (newSprite == NULL)
							{
								free(sprite);
								return(RDERR_OUTOFMEMORY);
							}

							// Work out the x-scale
							dy = s->w;
							dx = s->scaledWidth;
							ince = 2 * dy;
							incne = 2 * (dy - dx);
							d = 2 * dy - dx;

							x = 0;
							y = 0;
							xScale[y] = x;
							while (x < s->scaledWidth)
							{
								if (d <= 0)
								{
									d += ince;
									x += 1;
								}
								else
								{
									d += incne;
									x += 1;
									y += 1;
									xScale[y] = x;
								}
							}

							// Work out the y-scale
							dy = s->h;
							dx = s->scaledHeight;
							ince = 2 * dy;
							incne = 2 * (dy - dx);
							d = 2 * dy - dx;

							x = 0;
							y = 0;
							yScale[y] = x;
							while (x < s->scaledHeight)
							{
								if (d <= 0)
								{
									d += ince;
									x += 1;
								}
								else
								{
									d += incne;
									x += 1;
									y += 1;
									yScale[y] = x;
								}
							}

							// Copy the sprite
		//					InitialiseColourMatch(s->colourTable);
							spr = newSprite;
							for (y=0; y<s->h; y++)
							{
								for (j=yScale[y]; j<yScale[y+1]; j++)
								{
									for (x=0; x<s->w; x++)
									{
										for (i=xScale[x]; i<xScale[x+1]; i++)
										{
											*spr++ = *(sprite + y * s->w + x);
										}
									}
								}
							}

							//	Piece of code to anti-alias the sprite.
							newerSprite = malloc(s->scaledWidth * s->scaledHeight);
							if (newerSprite == NULL)
							{
								free(newSprite);
								free(sprite);
								return(RDERR_OUTOFMEMORY);
							}

							//	THIS IS THE ANTI-ALIAS LOOP  -  IF THE SPRITE IS CLIPPED, DON'T ANTI-ALIAS

							if (!((s->x - scrollx < 0) || (s->x - scrollx + s->scaledWidth > RENDERWIDE) ||
								 (s->y - scrolly < 0) || (s->y - scrolly + s->scaledHeight > RENDERDEEP)))
							{
								memcpy(newerSprite, newSprite, s->scaledWidth);
								for (y = 1; y < s->scaledHeight-1; y++)
								{
									src = newSprite + y * s->scaledWidth;
									dst = newerSprite + y * s->scaledWidth;
									*dst++ = *src++;
									for (x=1; x<s->scaledWidth-1; x++)
									{
										p = &myScreenBuffer[(s->y - scrolly + y) * RENDERWIDE + s->x - scrollx + x];
										count = 0;
										if (*src)
										{
											count ++;
											pt[0] = *src;
										}
										else
											pt[0] = *p;
										
										pt[1] = *(src - s->scaledWidth);
										if (pt[1] == 0)
											pt[1] = *(p - RENDERWIDE);
										else
											count++;

										pt[2] = *(src - 1);
										if (pt[2] == 0)
											pt[2] = *(p - 1);
										else
											count++;
										
										pt[3] = *(src + 1);
										if (pt[3] == 0)
											pt[3] = *(p + 1);
										else
											count++;
										
										pt[4] = *(src + s->scaledWidth);
										if (pt[4] == 0)
											pt[4] = *(p + RENDERWIDE);
										else
											count++;

										if (count)
										{
											red = palCopy[pt[0]][0] << 2;
											green = palCopy[pt[0]][1] << 2;
											blue = palCopy[pt[0]][2] << 2;
											for (i=1; i<5; i++)
											{
												red += palCopy[pt[i]][0];
												green += palCopy[pt[i]][1];
												blue += palCopy[pt[i]][2];
											}

											*dst++ = QuickMatch((uint8) (red >> 3), (uint8) (green >> 3), (uint8) (blue >> 3));
										}
										else
										{
											*dst++ = 0;
										}
										src++;
									}
									*dst++ = *src++;

								}
								memcpy(dst, src, s->scaledWidth);
								free(newSprite);
								newSprite = newerSprite;
							}





							// Draw the sprite
							starty = s->y - scrolly;
							endy = starty + s->scaledHeight;
							if (starty < 0)
								starty = 0;
							if (endy > RENDERDEEP)
								endy = RENDERDEEP;

							startx = s->x - scrollx;
							endx = startx + s->scaledWidth;
							if (startx < 0)
								startx = 0;
							if (endx > RENDERWIDE)
								endx = RENDERWIDE;

							for (i=starty; i<endy; i++)
							{
								dst = myScreenBuffer + i * RENDERWIDE+ startx;
								src = newSprite + (i - (s->y - scrolly)) * s->scaledWidth + (startx - (s->x - scrollx));
								if ((lightMask) && (renderCaps & RDBLTFX_SHADOWBLEND))
								{
									lightMap = lightMask + (i + scrolly) * locationWide + s->x;
									for (j=startx; j<endx; j++)
									{
										if (*src)
										{
											if (*lightMap)
												*dst = QuickMatch((uint8) (((int32) (32 - *lightMap) * palCopy[*src][0]) >> 5), 
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][1]) >> 5),
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][2]) >> 5));
											else
												*dst = *src;
										}
										dst++;
										src++;
										lightMap++;
									}
								}
								else
								{
									for (j=startx; j<endx; j++)
									{
										if (*src)
											*dst = *src;
										dst++;
										src++;
									}
								}
							}
							
							free(newSprite);
						}
						else		// Draw the stretched sprite with line doubling.
						{
							// Create the buffer to copy the bigger sprite into
							newSprite = malloc(s->scaledWidth * s->scaledHeight);
							if (newSprite == NULL)
							{
								free(sprite);
								return(RDERR_OUTOFMEMORY);
							}

							// Work out the x-scale
							dy = s->w;
							dx = s->scaledWidth;
							ince = 2 * dy;
							incne = 2 * (dy - dx);
							d = 2 * dy - dx;

							x = 0;
							y = 0;
							xScale[y] = x;
							while (x < s->scaledWidth)
							{
								if (d <= 0)
								{
									d += ince;
									x += 1;
								}
								else
								{
									d += incne;
									x += 1;
									y += 1;
									xScale[y] = x;
								}
							}

							// Work out the y-scale
							dy = s->h;
							dx = s->scaledHeight;
							ince = 2 * dy;
							incne = 2 * (dy - dx);
							d = 2 * dy - dx;

							x = 0;
							y = 0;
							yScale[y] = x;
							while (x < s->scaledHeight)
							{
								if (d <= 0)
								{
									d += ince;
									x += 1;
								}
								else
								{
									d += incne;
									x += 1;
									y += 1;
									yScale[y] = x;
								}
							}

							// Copy the sprite
		//					InitialiseColourMatch(s->colourTable);
							spr = newSprite;
							for (y=0; y<s->h; y++)
							{
								for (j=yScale[y]; j<yScale[y+1]; j++)
								{
									for (x=0; x<s->w; x++)
									{
										for (i=xScale[x]; i<xScale[x+1]; i++)
										{
											*spr++ = *(sprite + y * s->w + x);
										}
									}
								}
							}

							//	Piece of code to anti-alias the sprite.

							// Draw the sprite
							starty = s->y - scrolly;
							endy = starty + s->scaledHeight;
							if (starty < 0)
								starty = 0;
							if (endy > RENDERDEEP)
								endy = RENDERDEEP;

							startx = s->x - scrollx;
							endx = startx + s->scaledWidth;
							if (startx < 0)
								startx = 0;
							if (endx > RENDERWIDE)
								endx = RENDERWIDE;

							for (i=starty; i<endy; i++)
							{
								dst = myScreenBuffer + i * RENDERWIDE+ startx;
								src = newSprite + (i - (s->y - scrolly)) * s->scaledWidth + (startx - (s->x - scrollx));



								if ((lightMask) && (renderCaps & RDBLTFX_SHADOWBLEND))
								{
									lightMap = lightMask + (i + scrolly) * locationWide + s->x;
									for (j=startx; j<endx; j++)
									{
										if (*src)
										{
											if (*lightMap)
												*dst = QuickMatch((uint8) (((int32) (32 - *lightMap) * palCopy[*src][0]) >> 5), 
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][1]) >> 5),
																			 (uint8) (((int32) (32 - *lightMap) * palCopy[*src][2]) >> 5));
											else
												*dst = *src;
										}
										dst++;
										src++;
										lightMap++;
									}
								}
								else
								{
									for (j=startx; j<endx; j++)
									{
										if (*src)
											*dst = *src;
										dst++;
										src++;
									}
								}
							}
							
							free(newSprite);
						}

					}

					free(sprite);
					break;
//				case (RDSPR_LAYERCOMPRESSION>>8):
//					return(RDERR_NOTIMPLEMENTED);
//					break;
				default:
					return(RDERR_UNKNOWNTYPE);
				}
			}
		}
		else //(type & RDSPR_FULL):
		{

			// Set startx and starty for the screen buffer
			if (s->type & RDSPR_DISPLAYALIGN)
			{
				starty = s->y;
				startx = s->x;
			}
			else
			{
				starty = s->y - scrolly;
				startx = s->x - scrollx;
			}

			
			// Set end position of x and y.
			endy = s->h + starty;
			endx = s->w + startx;


			// Do clipping
			if ((starty < RENDERDEEP) && (endy > 0) && (startx < RENDERWIDE) && (endx > 0))
			{
				if (starty < 0)
					starty = 0;

				if (endy > RENDERDEEP)
					endy = RENDERDEEP;

				if (startx < 0)
					startx = 0;

				if (endx > RENDERWIDE)
					endx = RENDERWIDE;


				dst = (uint8 *) myScreenBuffer + starty * RENDERWIDE + s->x;
				if (s->type & RDSPR_DISPLAYALIGN)
					src = (uint8 *) s->data + s->w * (starty - (s->y)) + (startx - s->x);
				else
					src = (uint8 *) s->data + s->w * (starty - (s->y - scrolly)) + (startx - (s->x - scrollx));

				for (i=starty; i<endy; i++)
				{
					memcpy(dst, src, endx - startx);
					src += s->w;
					dst += RENDERWIDE;
				}
			}
		}
	}

#if PROFILING == 1
	QueryPerformanceCounter(&endTime);
	profileSpriteRender += (endTime.LowPart - startTime.LowPart);
#endif
*/
	return(RD_OK);
}



int32 OpenLightMask(_spriteInfo *s)

{

	if (lightMask)
		return(RDERR_NOTCLOSED);

	lightMask = (uint8 *) malloc(s->w * s->h);

	if (lightMask == NULL)
		return(RDERR_OUTOFMEMORY);

	if (DecompressRLE256(lightMask, s->data, s->w * s->h))
		return(RDERR_DECOMPRESSION);

	return(RD_OK);

}

int32 CloseLightMask(void)
{

	if (!lightMask)
		return(RDERR_NOTOPEN);

	free(lightMask);

	lightMask = 0;

	return(RD_OK);

}


















































