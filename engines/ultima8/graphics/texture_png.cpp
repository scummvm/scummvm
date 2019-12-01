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
#include "ultima8/graphics/texturePNG.h"

#include "ultima8/filesys/idata_source.h"

// CHECKME: how should we include png.h ? (And how should we check for it?)
//#include <png.h>

namespace Ultima8 {

static void idatasource_png_read_data(png_structp png_ptr,
                                      png_bytep data, png_size_t length) {
	void *read_io_ptr = png_get_io_ptr(png_ptr);
	IDataSource *ds = reinterpret_cast<IDataSource *>(read_io_ptr);
	ds->read(data, length);
}

bool TexturePNG::Read(IDataSource *ds) {
	png_bytep *row_pointers = 0;

	png_byte header[8];
	if (ds->read(header, 8) < 8) return false;
	if (png_sig_cmp(header, 0, 8)) {
		return false;
	}
	png_structp png_ptr = png_create_read_struct(
	                          PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return false;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return false;
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		delete[] row_pointers;
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return false;
	}

	png_set_read_fn(png_ptr, ds, idatasource_png_read_data);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	png_uint_32 width, height;
	int bit_depth, color_type;
	int interlace_type, compression_type, filter_method;
	png_get_IHDR(png_ptr, info_ptr, &width, &height,
	             &bit_depth, &color_type,
	             &interlace_type, &compression_type, &filter_method);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY ||
	        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	if (color_type == PNG_COLOR_TYPE_RGB)
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

	png_read_update_info(png_ptr, info_ptr);

	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	if (rowbytes != width * 4) {
		perr << "TexturePNG::Read(): failed to convert PNG to 32 bit RGBA"
		     << std::endl;
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return false;
	}

	buffer = new uint32[width * height];
	if (!buffer) {
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return false;
	}

	row_pointers = new png_bytep[height];
	for (unsigned int i = 0; i < height; ++i) {
		row_pointers[i] = reinterpret_cast<png_bytep>(&buffer[i * width]);
	}

	png_read_image(png_ptr, row_pointers);

	delete[] row_pointers;
	row_pointers = 0;

	// the end_info struct isn't used, but passing it anyway for now
	png_read_end(png_ptr, end_info);

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	this->width = width;
	this->height = height;
	this->format = TEX_FMT_STANDARD;
	this->wlog2 = -1;
	this->hlog2 = -1;

	// Repack RGBA
	for (unsigned int i = 0; i < height * width; ++i) {
		png_bytep p = reinterpret_cast<png_bytep>(&buffer[i]);
		buffer[i] = (p[0] << TEX32_R_SHIFT)
		            | (p[1] << TEX32_G_SHIFT)
		            | (p[2] << TEX32_B_SHIFT)
		            | (p[3] << TEX32_A_SHIFT);
	}

	return true;
}

} // End of namespace Ultima8
