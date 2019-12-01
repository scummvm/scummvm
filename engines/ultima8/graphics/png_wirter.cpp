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

#include "PNGWriter.h"
#include "ultima8/filesys/odata_source.h"
#include "ultima8/graphics/texture.h"

//#include <png.h>

namespace Ultima8 {

static void odatasource_png_write_data(png_structp png_ptr,
                                       png_bytep data, png_size_t length) {
	void *write_io_ptr = png_get_io_ptr(png_ptr);
	ODataSource *ds = reinterpret_cast<ODataSource *>(write_io_ptr);
	ds->write(data, length);
}

static void odatasource_png_flush_data(png_structp png_ptr) {
	// not necessary
}



PNGWriter::PNGWriter(ODataSource *ods) {
	ds = ods;
	png = 0;
}

PNGWriter::~PNGWriter() {

}

bool PNGWriter::init(uint32 width, uint32 height, const std::string &comment) {
	this->width = width;

	png_structp png_ptr = png_create_write_struct
	                      (PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr)
		return false;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	png_set_write_fn(png_ptr, ds, odatasource_png_write_data,
	                 odatasource_png_flush_data);

	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
	             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
	             PNG_FILTER_TYPE_DEFAULT);


	if (!comment.empty()) {
		std::string::size_type len = comment.size();
		char *t = new char[len + 1];
		std::memcpy(t, comment.c_str(), len + 1);

		png_text_struct text[1];
		text[0].compression = -1;
		text[0].key = "Comment";
		text[0].text = t;
		text[0].text_length = comment.size();

		png_set_text(png_ptr, info_ptr, text, 1);

		delete[] t;
	}

	png_write_info(png_ptr, info_ptr);

	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

	png_set_bgr(png_ptr);

	png = reinterpret_cast<void *>(png_ptr);
	info = reinterpret_cast<void *>(info_ptr);

	return true;
}

bool PNGWriter::writeRows(uint32 nrows, Texture *img) {
	png_bytep *row_pointers = 0;
	png_structp png_ptr = reinterpret_cast<png_structp>(png);
	png_infop info_ptr = reinterpret_cast<png_infop>(info);
	if (!png_ptr) return false;

	assert(img->width == static_cast<int32>(width));
	assert(img->height >= static_cast<int32>(nrows));

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		delete[] row_pointers;
		return false;
	}

	row_pointers = new png_bytep[nrows];

	for (unsigned int i = 0; i < nrows; ++i) {
		row_pointers[i] = reinterpret_cast<png_bytep>(&img->buffer[i * width]);
	}

	png_write_rows(png_ptr, row_pointers, nrows);

	png_write_flush(png_ptr);

	delete[] row_pointers;

	return true;
}

bool PNGWriter::finish() {
	png_structp png_ptr = reinterpret_cast<png_structp>(png);
	png_infop info_ptr = reinterpret_cast<png_infop>(info);
	if (!png_ptr) return false;

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return true;
}

} // End of namespace Ultima8
