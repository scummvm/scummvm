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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/files/nuvie_bmp_file.h"

namespace Ultima {
namespace Nuvie {

#define NUVIEBMPFILE_MAGIC 0x4d42 // 'BM'

NuvieBmpFile::NuvieBmpFile() {
	data = NULL;
	prev_width = 0;
	prev_height = 0;
	prev_bits = 0;
	bmp_line_width = 0;
	memset(&header, 0, sizeof(header));
	memset(&infoHeader, 0, sizeof(infoHeader));
}

NuvieBmpFile::~NuvieBmpFile() {
	if (data != NULL)
		free(data);
}

bool NuvieBmpFile::initNewBlankImage(sint32 width, sint32 height, const unsigned char *pal) {
	infoHeader.size = 40;
	infoHeader.width = width;
	infoHeader.height = height;
	infoHeader.planes = 1;
	infoHeader.bits = 8;
	infoHeader.compression = 0;
	infoHeader.imagesize = 0;
	infoHeader.xresolution = 0; //FIXME
	infoHeader.yresolution = 0; //FIXME
	infoHeader.ncolours = 256;
	infoHeader.importantcolours = 256;

	bmp_line_width = infoHeader.width;
	if (bmp_line_width % 4 != 0) {
		bmp_line_width += (4 - (bmp_line_width % 4));
	}

	header.type = NUVIEBMPFILE_MAGIC;
	header.reserved1 = 0;
	header.reserved2 = 0;
	header.offset = NUVIEBMP_HEADER_SIZE + NUVIEBMP_INFOHEADER_SIZE + 256 * 4;
	header.size = header.offset + bmp_line_width * infoHeader.height;

	memcpy(&palette, pal, sizeof(palette));

	data = (unsigned char *)malloc(infoHeader.width * infoHeader.height);
	if (!data) {
		return handleError("Allocating pixel data");
	}

	memset(data, 0, infoHeader.width * infoHeader.height);

	return true;
}

bool NuvieBmpFile::load(Std::string filename) {
	NuvieIOFileRead file;

	if (filename.length() == 0)
		return handleError("zero byte file");

	if (!file.open(filename)) {
		return handleError("opening file");
	}

	if (file.get_size() < 0x36) { //invalid header.
		return handleError("filesize < 0x36");
	}

	header.type = file.read2();
	header.size = file.read4();
	header.reserved1 = file.read2();
	header.reserved2 = file.read2();
	header.offset = file.read4();

	infoHeader.size = file.read4();
	infoHeader.width = file.read4();
	infoHeader.height = file.read4();
	infoHeader.planes = file.read2();
	infoHeader.bits = file.read2();
	infoHeader.compression = file.read4();
	infoHeader.imagesize = file.read4();
	infoHeader.xresolution = file.read4();
	infoHeader.yresolution = file.read4();
	infoHeader.ncolours = file.read4();
	infoHeader.importantcolours = file.read4();

	if (header.type != NUVIEBMPFILE_MAGIC) { //invalid magic.
		return handleError("invalid BMP magic.");
	}

	if (infoHeader.bits != 8 && infoHeader.bits != 24) {
		return handleError("only 256 colour bitmaps supported.");
	}

	if (infoHeader.compression != 0) { // && infoHeader.compression != 2)
		return handleError("only uncompressed BMP files are supported");
		//return handleError("only raw and bi_rle8 compression formats are supported.");

		//FIXME need to handle rle compression.
	}

	if (infoHeader.bits == 8) {
		for (uint32 i = 0; i < infoHeader.ncolours; i++) {
			uint8 b = file.read1();
			uint8 g = file.read1();
			uint8 r = file.read1();
			file.read1(); // 0
			palette[i] = (uint32)r | (uint32)(g << 8) | (uint32)(b << 16);
		}
	}

	file.seekStart();
	file.seek(header.offset);

	uint16 bytes_per_pixel = infoHeader.bits / 8;
	bmp_line_width = infoHeader.width * bytes_per_pixel;
	if (bmp_line_width % 4 != 0) {
		bmp_line_width += (4 - (bmp_line_width % 4));
	}

	if (data == NULL || infoHeader.width != prev_width || infoHeader.height != prev_height || prev_bits != infoHeader.bits) {
		if (data) {
			free(data);
		}
		data = (unsigned char *)malloc(infoHeader.width * infoHeader.height * bytes_per_pixel);
		prev_width = infoHeader.width;
		prev_height = infoHeader.height;
		prev_bits = infoHeader.bits;
		if (data == NULL) {
			return handleError("allocating memory for image");
		}
	}

	uint32 end = header.offset + bmp_line_width * infoHeader.height;

	uint32 data_width = infoHeader.width * bytes_per_pixel;

	for (sint32 i = 0; i < infoHeader.height; i++) {
		file.seek(end - bmp_line_width - (bmp_line_width * i));
		file.readToBuf(&data[i * data_width], data_width);
	}

	return true;
}

bool NuvieBmpFile::save(Std::string filename) {
	NuvieIOFileWrite file;

	if (!file.open(filename)) {
		return handleError("Opening " + filename + ".");
	}

	file.write2(header.type);
	file.write4(header.size);
	file.write2(header.reserved1);
	file.write2(header.reserved2);
	file.write4(header.offset);

	file.write4(infoHeader.size);
	file.write4(infoHeader.width);
	file.write4(infoHeader.height);
	file.write2(infoHeader.planes);
	file.write2(infoHeader.bits);
	file.write4(infoHeader.compression);
	file.write4(infoHeader.imagesize);
	file.write4(infoHeader.xresolution);
	file.write4(infoHeader.yresolution);
	file.write4(infoHeader.ncolours);
	file.write4(infoHeader.importantcolours);

	if (infoHeader.bits == 8) {
		for (uint32 i = 0; i < infoHeader.ncolours; i++) {
			file.write1((palette[i] >> 16) & 0xff); //B
			file.write1((palette[i] >> 8) & 0xff);  //G
			file.write1(palette[i] & 0xff);         //R
			file.write1((palette[i] >> 24) & 0xff); //A
		}
		write8BitData(&file);
	} else {
		//FIXME write out 24bit data here.
	}

	file.close();
	return true;
}

void NuvieBmpFile::write8BitData(NuvieIOFileWrite *file) {
	uint32 i;
	for (i = infoHeader.height; i > 0; i--) {
		file->writeBuf(&data[(i - 1)*infoHeader.width], infoHeader.width);
		if ((sint32)bmp_line_width > infoHeader.width) {
			//write out padding bytes.
			for (uint8 j = 0; j < bmp_line_width - infoHeader.width; j++) {
				file->write1(0);
			}
		}
	}
}

bool NuvieBmpFile::handleError(Std::string error) {
	if (data) {
		free(data);
		data = NULL;
	}

	DEBUG(0, LEVEL_ERROR, error.c_str());

	return false;
}

Tile *NuvieBmpFile::getTile() {
	if (infoHeader.width != 16 || infoHeader.height != 16 || infoHeader.bits != 8) {
		return NULL;
	}

	Tile *t = (Tile *)malloc(sizeof(Tile));
	if (t == NULL) {
		return NULL;
	}
	memset(t, 0, sizeof(Tile));
	memcpy(t->data, data, 256);

	return t;
}

unsigned char *NuvieBmpFile::getRawIndexedData() {
	if (infoHeader.bits != 8) {
		return NULL;
	}

	return data;
}

unsigned char *NuvieBmpFile::getRawIndexedDataCopy() {
	if (data == NULL || infoHeader.bits != 8) {
		return NULL;
	}

	unsigned char *copy = (unsigned char *)malloc(infoHeader.width * infoHeader.height);
	if (copy == NULL) {
		return NULL;
	}
	memcpy(copy, data, infoHeader.width * infoHeader.height);
	return copy;
}

Graphics::ManagedSurface *NuvieBmpFile::getSdlSurface32(Std::string filename) {
	load(filename);
	return getSdlSurface32();
}

Graphics::ManagedSurface *NuvieBmpFile::getSdlSurface32() {
	if (data == NULL) {
		return NULL;
	}

	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface(
		infoHeader.width, infoHeader.height, 
		Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)
	);

	unsigned char *src_buf = data;
	Graphics::Surface s = surface->getSubArea(Common::Rect(0, 0, surface->w, surface->h));
	uint32 *pixels = (uint32 *)s.getPixels();

	if (infoHeader.bits == 8) {
		for (sint32 i = 0; i < infoHeader.height; i++) {
			for (sint32 j = 0; j < infoHeader.width; j++) {
				pixels[j] = palette[src_buf[j]];
			}
			src_buf += infoHeader.width;
			pixels += infoHeader.width;
		}
	} else { //bits == 24
		for (sint32 i = 0; i < infoHeader.height; i++) {
			for (sint32 j = 0; j < infoHeader.width; j++) {
				pixels[j] = (uint32)src_buf[j * 3 + 2] | (uint32)(src_buf[j * 3 + 1] << 8) | (uint32)(src_buf[j * 3 + 0] << 16);
			}
			src_buf += infoHeader.width * 3;
			pixels += infoHeader.width;
		}
	}
	return surface;
}

} // End of namespace Nuvie
} // End of namespace Ultima
