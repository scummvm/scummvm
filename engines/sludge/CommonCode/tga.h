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

struct TGAHeader {
	unsigned char IDBlockSize;
	unsigned char gotMap;
	bool compressed;
	unsigned short int firstPalColour;
	unsigned short int numPalColours;
	unsigned char bitsPerPalColour;
	unsigned short xOrigin;
	unsigned short yOrigin;
	unsigned short width;
	unsigned short height;
	unsigned char pixelDepth;
	unsigned char imageDescriptor;
};

struct palCol {
	unsigned char r, g, b;
};

void grabRGBCompressed(FILE *fp, int bpc, unsigned char &r2, unsigned char &g2, unsigned char &b2, palCol thePalette[]);
void grabRGB(FILE *fp, int bpc, unsigned char &r, unsigned char &g, unsigned char &b, palCol thePalette[]);
void grabRGBACompressed(FILE *fp, int bpc, unsigned char &r2, unsigned char &g2, unsigned char &b2, unsigned char &a2, palCol thePalette[]);
void grabRGBA(FILE *fp, int bpc, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a, palCol thePalette[]);

unsigned short int makeColour(unsigned char r, unsigned char g, unsigned char b);
unsigned short readAColour(FILE *fp, int bpc, palCol thePalette[], int x, int y);
unsigned short readCompressedColour(FILE *fp, int bpc, palCol thePalette[], int x, int y);
const char *readTGAHeader(TGAHeader &h, FILE *fp, palCol thePalette[]);
void setDither(int dither);
bool getDither();

inline unsigned short redValue(unsigned short c) {
	return (c >> 11) << 3;
}
inline unsigned short greenValue(unsigned short c) {
	return ((c >> 5) & 63) << 2;
}
inline unsigned short blueValue(unsigned short c) {
	return (c & 31) << 3;
}
inline int brightness(unsigned short c) {
	return ((int) redValue(c)) + ((int) greenValue(c)) + ((int) blueValue(c) >> 1);
}
