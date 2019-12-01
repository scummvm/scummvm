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
#include "ultima8/graphics/textureTarga.h"
#include "ultima8/filesys/idata_source.h"

namespace Ultima8 {

struct TGA {
	uint32      Height;                                     // Height of Image
	uint32      Width;                                      // Width ofImage
	uint32      Bpp;                                        // Bits Per Pixel
	uint32      bytesPerPixel;
	uint32      Flags;
	bool        Flipped;
	bool        Compressed;

	void Read(IDataSource *ds) {
		Width   = ds->read2();
		Height  = ds->read2();
		Bpp = ds->read1();
		Flags = ds->read1();

		Flipped = (Flags & (1 << 5)) ? false : true;
		bytesPerPixel   = Bpp / 8;
	}
};

static const uint8 uTGAcompare[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Uncompressed TGA Header
static const uint8 cTGAcompare[12] = {0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Compressed TGA Header

bool TextureTarga::Read(IDataSource *ds) {
	// TGA File Header
	uint8 header[12];

	// TGA image data
	TGA tga;

	// Seek to start
	ds->seek(0);

	// Attempt to read 12 byte header from file
	ds->read(static_cast<uint8 *>(header), 12);

	// Is it uncompressed?
	//if(memcmp(uTGAcompare+1, header+1, 11) == 0) tga.Compressed = false;
	//// Maybe it's compressed?
	//else if(memcmp(cTGAcompare+1, header+1, 11) == 0) tga.Compressed = true;
	/// Not a recognized Targa
	//else return false;

	// Colour Map? We don't support it
	if (header[1] != 0) return false;

	// Image type. Only True Colour image data
	if (header[2] == 2) tga.Compressed = false;
	else if (header[2] == 10) tga.Compressed = true;
	else return false;

	// Don't care about the other header data

	// Read TGA header
	tga.Read(ds);

	ds->skip(header[0]);

	// Set Width and Height
	width   = tga.Width;
	height  = tga.Height;
	format = TEX_FMT_STANDARD;

	// Make sure all information is valid
	if (width <= 0 || height <= 0 || (tga.Bpp != 24 && tga.Bpp != 32))
		return false;

	// Allocate temporary buffer
	uint8 *temp_buffer  = new uint8[width * height * tga.bytesPerPixel];

	// Couldn't allocate
	if (!temp_buffer) return false;

	// Read it
	if (tga.Compressed) {
		// Is compressed
		uint8 *image = temp_buffer;
		uint8 *end = temp_buffer + tga.bytesPerPixel * width * height;

		while (image < end) {
			uint32 chunkheader = ds->read1();

			// RAW (copy chunkheader+1 pixels)
			if (chunkheader < 128) {
				chunkheader = (chunkheader + 1) * tga.bytesPerPixel;

				// Read chunkheader pixels
				ds->read(static_cast<uint8 *>(image), chunkheader);

				image += chunkheader;
			}
			// SINGLE COLOUR (duplicate colour for chunkheader-127 pixels)
			else {
				chunkheader = (chunkheader - 127) * tga.bytesPerPixel;

				// Read the pixel
				static uint8 colorbuffer[4];
				ds->read(static_cast<uint8 *>(colorbuffer), tga.bytesPerPixel);

				uint8 *end_run = image + chunkheader;

				if (tga.bytesPerPixel == 4) {
					while (image < end_run) {
						*image++ = colorbuffer[0];
						*image++ = colorbuffer[1];
						*image++ = colorbuffer[2];
						*image++ = colorbuffer[3];
					}
				} else {
					while (image < end_run) {
						*image++ = colorbuffer[0];
						*image++ = colorbuffer[1];
						*image++ = colorbuffer[2];
					}
				}
			}
		}
	} else {
		// Is uncompressed
		ds->read(static_cast<uint8 *>(temp_buffer), width * height * tga.bytesPerPixel);
	}

	// Create Actual Buffer
	buffer  = new uint32[width * height];

	// Couldn't allocate
	if (!buffer) {
		delete [] temp_buffer;
		return false;
	}

	// Convert TGA Format into Actual Format
	ConvertFormat(temp_buffer, tga);

	// Calc texture log2's
	CalcLOG2s();

	// Free temp buffer and return
	delete [] temp_buffer;
	return true;
}

//
// Convert the loaded buffer into something a little more useful
//
void TextureTarga::ConvertFormat(uint8 *src, TGA &tga) {
	// End iterations
	uint32 *dst = buffer;
	uint32 *end = buffer + width * height;
	uint32 line_size = width;

	uint32 src_line_inc = 0;

	if (tga.Flipped) {
		src_line_inc = width * tga.bytesPerPixel * 2;
		src = src + (height - 1) * width * tga.bytesPerPixel;
	}

	while (dst < end) {

		uint32 *line_end = dst + line_size;

		if (tga.Bpp == 32) while (dst < line_end) {

				// extract RGB components, and pack them into an int
				*dst++ = (src[0] << TEX32_B_SHIFT)
				         | (src[1] << TEX32_G_SHIFT)
				         | (src[2] << TEX32_R_SHIFT)
				         | (src[3] << TEX32_A_SHIFT);

				src += 4;
			}
		else if (tga.Bpp == 24) while (dst < line_end) {

				// extract RGB components, and pack them into an int
				*dst++ = (src[0] << TEX32_B_SHIFT)
				         | (src[1] << TEX32_G_SHIFT)
				         | (src[2] << TEX32_R_SHIFT)
				         | (255 << TEX32_A_SHIFT);

				src += 3;
			}

		src -= src_line_inc;
	}
}

} // End of namespace Ultima8
