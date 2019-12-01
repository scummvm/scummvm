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

#include "ultima8/misc/pent_include.h"
#include "ultima8/graphics/textureBitmap.h"
#include "ultima8/filesys/idata_source.h"

namespace Ultima8 {

// graphics defines
#define BITMAP_ID            0x4D42 // universal id for a bitmap

//
// Bitmap Header
//
struct BMPHeader {
	uint16      bfType;
	uint32      bfSize;
	uint16      bfReserved1;
	uint16      bfReserved2;
	uint32      bfOffBits;

	void Read(IDataSource *ds) {
		bfType = ds->read2();
		bfSize = ds->read4();
		bfReserved1 = ds->read2();
		bfReserved2 = ds->read2();
		bfOffBits = ds->read4();
	}
};

//
// Bitmap Info Header
//
struct BMPInfoHeader {
	uint32      biSize;
	int32       biWidth;
	int32       biHeight;
	uint16      biPlanes;
	uint16      biBitCount;
	uint32      biCompression;
	uint32      biSizeImage;
	int32       biXPelsPerMeter;
	int32       biYPelsPerMeter;
	uint32      biClrUsed;
	uint32      biClrImportant;

	void Read(IDataSource *ds) {
		biSize = ds->read4();
		biWidth = ds->read4();
		biHeight = ds->read4();
		biPlanes = ds->read2();
		biBitCount = ds->read2();
		biCompression = ds->read4();
		biSizeImage = ds->read4();
		biXPelsPerMeter = ds->read4();
		biYPelsPerMeter = ds->read4();
		biClrUsed = ds->read4();
		biClrImportant = ds->read4();
	}
};


//
// Read from a Data Source
//
bool TextureBitmap::Read(IDataSource *ds) {
	// Seek to start
	ds->seek(0);

	// looping index
	int index;

	// used to convert images to 32 bit
	uint8 *temp_buffer = NULL;

	// this contains the bitmapfile header
	BMPHeader       bitmapfileheader;

	// this is all the info including the palette
	BMPInfoHeader   bitmapinfoheader;

	// now load the bitmap file header
	bitmapfileheader.Read(ds);

	// test if this is a bitmap file, if not return false
	if (bitmapfileheader.bfType != BITMAP_ID) return false;

	// now we know this is a bitmap, so read in all the sections
	// first the bitmap infoheader
	bitmapinfoheader.Read(ds);

	// now load the color palette if there is one
	uint8 palette[768];
	if (bitmapinfoheader.biBitCount == 8) {
		for (index = 0; index < 256; index++) {
			palette[index * 3] = ds->read1();
			palette[index * 3 + 1] = ds->read1();
			palette[index * 3 + 2] = ds->read1();
			ds->skip(1);

		}

	}

	// finally the image data itself
	ds->seek(ds->getSize() - bitmapinfoheader.biSizeImage);

	// allocate temporary buffer
	if (0 == (temp_buffer = new uint8 [bitmapinfoheader.biSizeImage]))
		return false;

	// allocate final 32 bit storage buffer
	if (0 == (buffer = new uint32[bitmapinfoheader.biWidth * bitmapinfoheader.biHeight])) {
		// release working buffer
		delete [] temp_buffer;

		// return error
		return false;
	} // end if

	// now read it in
	ds->read(static_cast<uint8 *>(temp_buffer), bitmapinfoheader.biSizeImage);

	// 8 Bit Palette
	if (bitmapinfoheader.biBitCount == 8) {
		for (index = 0; index < bitmapinfoheader.biWidth * bitmapinfoheader.biHeight; index++) {
			// extract RGB components (in BGR order), note the scaling
			uint8 palindex = temp_buffer[index];

			int row = index / bitmapinfoheader.biWidth;
			int write = index % bitmapinfoheader.biWidth;
			write += (bitmapinfoheader.biHeight - 1 - row) * bitmapinfoheader.biWidth;

			buffer[write] = (palette[palindex * 3 + TEX32_B_SHIFT])
			                | (palette[palindex * 3 + 1] << TEX32_G_SHIFT)
			                | (palette[palindex * 3 + 2] << TEX32_R_SHIFT)
			                | (255 << TEX32_A_SHIFT);

		} // end for index
	}
	// 16 Bit High Colour
	else if (bitmapinfoheader.biBitCount == 16) {
		// Colour shifting values
#define UNPACK_BMP16(pix,r,g,b) { r = static_cast<uint8>((((pix)&31)>>10)<<5); g = static_cast<uint8>((((pix)&31)>>5)<<5); b = static_cast<uint8>(((pix)&31)<<5); }

		for (index = 0; index < bitmapinfoheader.biWidth * bitmapinfoheader.biHeight; index++) {
			// extract RGB components, and pack them into an int
			uint8 red, green, blue ;
			uint16 color = temp_buffer[index * 2 + 0] | (temp_buffer[index * 2 + 1] << 8);
			UNPACK_BMP16(color, red, green, blue);

			int row = index / bitmapinfoheader.biWidth;
			int write = index % bitmapinfoheader.biWidth;
			write += (bitmapinfoheader.biHeight - 1 - row) * bitmapinfoheader.biWidth;


			buffer[write] = (blue << TEX32_B_SHIFT)
			                | (green << TEX32_G_SHIFT)
			                | (red << TEX32_R_SHIFT)
			                | (255 << TEX32_A_SHIFT);
		}
	}
	// 24 Bit True Colour
	else if (bitmapinfoheader.biBitCount == 24) {
		for (index = 0; index < bitmapinfoheader.biWidth * bitmapinfoheader.biHeight; index++) {
			int row = index / bitmapinfoheader.biWidth;
			int write = index % bitmapinfoheader.biWidth;
			write += (bitmapinfoheader.biHeight - 1 - row) * bitmapinfoheader.biWidth;

			// extract RGB components, and pack them into an int
			buffer[write] = (temp_buffer[index * 3 + 0] << TEX32_B_SHIFT)
			                | (temp_buffer[index * 3 + 1] << TEX32_G_SHIFT)
			                | (temp_buffer[index * 3 + 2] << TEX32_R_SHIFT)
			                | (255 << TEX32_A_SHIFT);

		}
	}

	// Make sure the width and height are correctly copied
	width = bitmapinfoheader.biWidth;
	height = bitmapinfoheader.biHeight;
	format = TEX_FMT_STANDARD;

	// Calc texture log2's
	CalcLOG2s();

	// return success
	return true;

}

} // End of namespace Ultima8
