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

#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/graphics/image.h"
#include "ultima/ultima4/graphics/imageloader.h"
#include "ultima/ultima4/graphics/imageloader_png.h"

namespace Ultima {
namespace Ultima4 {

#ifdef TODO
static void png_read_xu4(png_structp png_ptr, png_bytep data, png_size_t length) {
	png_size_t check;
	U4FILE *file;

	file = (U4FILE *) png_get_io_ptr(png_ptr);
	check = file->read(data, (png_size_t)1, length);

	if (check != length)
		png_error(png_ptr, "Read Error");
}
#endif
/**
 * Loads in the PNG with the libpng library.
 */
Image *PngImageLoader::load(U4FILE *file, int width, int height, int bpp) {
	if (width != -1 || height != -1 || bpp != -1) {
		errorWarning("dimensions set for PNG image, will be ignored");
	}

#ifdef TODO
	char header[8];
	file->read(header, 1, sizeof(header));
	if (png_sig_cmp((png_byte *)header, 0, sizeof(header)) != 0)
		return NULL;

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return NULL;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		return NULL;
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		return NULL;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}

	png_set_read_fn(png_ptr, file, &png_read_xu4);

	png_set_sig_bytes(png_ptr, sizeof(header));

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	png_uint_32 pwidth, pheight;
	int bit_depth, color_type, interlace_type, compression_type, filter_method;
	png_get_IHDR(png_ptr, info_ptr, &pwidth, &pheight, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

	width = pwidth;
	height = pheight;

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		bpp = bit_depth;
	else if (color_type == PNG_COLOR_TYPE_RGB)
		bpp = bit_depth * 3;
	else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		bpp = bit_depth * 4;

	png_byte **row_pointers = png_get_rows(png_ptr, info_ptr);

	unsigned char *raw = new unsigned char[width * height * bpp / 8];

	unsigned char *p = raw;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width * bpp / 8; j++) {
			*p++ = row_pointers[i][j];
		}
	}

	Image *image = Image::create(width, height, bpp == 4 || bpp == 8, Image::HARDWARE);
	if (!image) {
		delete [] raw;
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}

	if (bpp == 4 || bpp == 8) {
		int num_pngpalette;
		png_colorp pngpalette;
		png_get_PLTE(png_ptr, info_ptr, &pngpalette, &num_pngpalette);
		RGBA *palette = new RGBA[num_pngpalette];
		for (int c = 0; c < num_pngpalette; c++) {
			palette[c].r = pngpalette[c].red;
			palette[c].g = pngpalette[c].green;
			palette[c].b = pngpalette[c].blue;
			palette[c].a = IM_OPAQUE;
		}
		image->setPalette(palette, num_pngpalette);
		delete [] palette;
	}

	setFromRawData(image, width, height, bpp, raw);

	delete [] raw;
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	return image;
#else
	return nullptr;
#endif
}

} // End of namespace Ultima4
} // End of namespace Ultima
