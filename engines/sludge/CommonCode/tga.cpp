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
#include <stdio.h>

#include "tga.h"

//FILE * debugFile = fopen ("TGAdebug.txt", "wt");

unsigned short int makeColour (unsigned char r, unsigned char g, unsigned char b) {
	unsigned short int reply = (unsigned short int) (r >> 3);
	reply <<= 6;
	reply += (unsigned short int) (g >> 2);
	reply <<= 5;
	reply += (unsigned short int) (b >> 3);
	return reply & 65503;
}

int get2bytesReverse (FILE * fp) {
	int a = fgetc (fp);
	return a + fgetc (fp) * 256;
}

int countDown = 0;

bool dither24bitImages = 0;

char ditherArray[4][4] = {{4,12,6,14},{10,0,8,2},{7,15,5,13},{9,3,11,1}};

void grabRGBA (FILE * fp, int bpc, unsigned char & r, unsigned char & g, unsigned char & b, unsigned char & a, palCol thePalette[])
{
	int grabbed1, grabbed2;
	switch (bpc) {
		case 8:
			grabbed1 = fgetc (fp);
			r = thePalette[grabbed1].r;
			g = thePalette[grabbed1].g;
			b = thePalette[grabbed1].b;
			if (r == 255 && g == 0 && b == 255) {
				r = g = b = a = 0;
			} else a = 255;
			break;
			
		case 16:
			grabbed1 = fgetc (fp);
			grabbed2 = fgetc (fp);
			if (grabbed2*256+grabbed1 == 31775) {
				r=g=b=a=0;
				break;
			}

			r = ((grabbed2 & 127) << 1),
			g = ((grabbed1 & 224) >> 2) + (grabbed2 << 6);
			b = ((grabbed1 & 31) << 3);
			if (r == 255 && g == 0 && b == 255) {
				r = g = b = a = 0;
			} else a = 255;
			break;
			
		case 24:
			b = fgetc (fp);
			g = fgetc (fp);
			r = fgetc (fp);
			if (r == 255 && g == 0 && b == 255) {
				r = g = b = a = 0;
			} else a = 255;
			break;
			
		case 32:
			b = fgetc (fp);
			g = fgetc (fp);
			r = fgetc (fp);
			a = fgetc (fp);
			break;		
	}
}


void grabRGB (FILE * fp, int bpc, unsigned char & r, unsigned char & g, unsigned char & b, palCol thePalette[])
{
	int a;
	int grabbed1, grabbed2;
	switch (bpc) {
		case 8:
		grabbed1 = fgetc (fp);
		r = thePalette[grabbed1].r;
		g = thePalette[grabbed1].g;
		b = thePalette[grabbed1].b;
		break;
	
		case 16:
		grabbed1 = fgetc (fp);
		grabbed2 = fgetc (fp);
		r = ((grabbed2 & 127) << 1),
		g = ((grabbed1 & 224) >> 2) + (grabbed2 << 6);
		b = ((grabbed1 & 31) << 3);
		break;
				
		case 24:
		b = fgetc (fp);
		g = fgetc (fp);
		r = fgetc (fp);
		break;
		
		case 32:
		b = fgetc (fp);
		g = fgetc (fp);
		r = fgetc (fp);
		a = fgetc (fp);
			if (a < 100) {
				r = 255;
				g = 0;
				b = 255;
			}
		break;		
	}
}

void grabRGBACompressed (FILE * fp, int bpc, unsigned char & r2, unsigned char & g2, unsigned char & b2, unsigned char & a2, palCol thePalette[]) {
	static unsigned char r, g, b, a;
	static bool oneCol;
	unsigned short col;
	
	// Do we have to start a new packet?
	if (countDown == 0) {
		
		// Read the packet description thingy
		col = fgetc (fp);
		
		// Is it raw data?
		if (col >= 128) {
			oneCol = true;
			countDown = col - 127;
			grabRGBA (fp, bpc, r, g, b, a, thePalette);
			//			fprintf (debugFile, "  %d raw colours...\n", countDown);
		} else {
			oneCol = false;
			countDown = col + 1;
			//			fprintf (debugFile, "  %d pixels the same colour...\n", countDown);
		}
	}
	
	countDown --;
	
	if (! oneCol) {
		grabRGBA (fp, bpc, r2, g2, b2, a2, thePalette);
	} else {
		r2 = r;
		g2 = g;
		b2 = b;
		a2 = a;
	}
}


void grabRGBCompressed (FILE * fp, int bpc, unsigned char & r2, unsigned char & g2, unsigned char & b2, palCol thePalette[]) {
	static unsigned char r, g, b;
	static bool oneCol;
	unsigned short col;
	
	// Do we have to start a new packet?
	if (countDown == 0) {
		
		// Read the packet description thingy
		col = fgetc (fp);
		
		// Is it raw data?
		if (col >= 128) {
			oneCol = true;
			countDown = col - 127;
			grabRGB (fp, bpc, r, g, b, thePalette);
			//			fprintf (debugFile, "  %d raw colours...\n", countDown);
		} else {
			oneCol = false;
			countDown = col + 1;
			//			fprintf (debugFile, "  %d pixels the same colour...\n", countDown);
		}
	}
	
	countDown --;
	
	if (! oneCol) {
		grabRGB (fp, bpc, r2, g2, b2, thePalette);
	} else {
		r2 = r;
		g2 = g;
		b2 = b;
	}
}

void addDither (unsigned char & col, const unsigned char add)
{
	int tot = col;
	tot += add;
	col = (tot > 255) ? 255 : tot;
}

unsigned short readAColour (FILE * fp, int bpc, palCol thePalette[], int x, int y) {
	unsigned char r,g,b;
	grabRGB (fp, bpc, r, g, b, thePalette);

	if (dither24bitImages)
	{
		addDither (r, ditherArray[x&3][y&3]);
		addDither (g, ditherArray[x&3][y&3] / 2);
		addDither (b, ditherArray[x&3][y&3]);
	}

	return makeColour (r, g, b);
}

unsigned short readCompressedColour (FILE * fp, int bpc, palCol thePalette[], int x, int y) {
	unsigned char r,g,b;
	grabRGBCompressed (fp, bpc, r, g, b, thePalette);
	
	if (dither24bitImages)
	{
		addDither (r, ditherArray[x&3][y&3]);
		addDither (g, ditherArray[x&3][y&3] / 2);
		addDither (b, ditherArray[x&3][y&3]);
	}
	
	return makeColour (r, g, b);
}

const char * readTGAHeader (TGAHeader & h, FILE * fp, palCol thePalette[]) {

	h.IDBlockSize = fgetc (fp);
	h.gotMap = fgetc (fp);
	unsigned char imageType = fgetc (fp);
	h.firstPalColour = get2bytesReverse (fp);
	h.numPalColours = get2bytesReverse (fp);
	h.bitsPerPalColour = fgetc (fp);
	h.xOrigin = get2bytesReverse (fp);
	h.yOrigin = get2bytesReverse (fp);
	h.width = get2bytesReverse (fp);
	h.height = get2bytesReverse (fp);
	h.pixelDepth = fgetc (fp);
	h.imageDescriptor = fgetc (fp);
	countDown = 0;
	// Who cares about the ID block?
	fseek (fp, h.IDBlockSize, 1);

	switch (imageType) {
		case 1:
		case 2:
		h.compressed = false;
		break;
		
		case 9:
		case 10:
		h.compressed = true;
		break;
		
		default:
		return "Unsupported internal image format... are you sure this is a valid TGA image file?";
	}

	if (h.pixelDepth != 8 && h.pixelDepth != 16 && h.pixelDepth != 24 && h.pixelDepth != 32) {
		return "Colour depth is not 8, 16, 24 or 32 bits... are you sure this is a valid TGA image file?";
	}

	if (h.gotMap) {
		int c;
		for (c = 0; c < h.numPalColours; c ++) {
			grabRGB (fp, h.bitsPerPalColour, thePalette[c].r, thePalette[c].g, thePalette[c].b, thePalette);
		}
	}

	return NULL;
}

void setDither (int dither)
{
	dither24bitImages = dither;
}

bool getDither ()
{
	return dither24bitImages;
}
