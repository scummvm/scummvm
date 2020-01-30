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

#ifndef NUVIE_FILES_NUVIE_BMP_FILE_H
#define NUVIE_FILES_NUVIE_BMP_FILE_H

#include "ultima/shared/std/string.h"

#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/core/tile_manager.h"

namespace Ultima {
namespace Nuvie {

class NuvieBmpFile {
private:
	unsigned char *data;
	uint32 palette[256];
	sint32 prev_width;
	sint32 prev_height;
	uint16 prev_bits;
	uint32 bmp_line_width;

	struct {
		uint16 type;                    /* Magic identifier            */
		uint32 size;                    /* File size in bytes          */
		uint16 reserved1, reserved2;
		uint32 offset;                  /* Offset to image data, bytes */
	} header;

#define NUVIEBMP_HEADER_SIZE 14

	struct {
		uint32 size;                    /* Header size in bytes      */
		sint32 width, height;           /* Width and height of image */
		uint16 planes;                  /* Number of colour planes   */
		uint16 bits;                    /* Bits per pixel            */
		uint32 compression;             /* Compression type          */
		uint32 imagesize;               /* Image size in bytes       */
		sint32 xresolution, yresolution; /* Pixels per meter          */
		uint32 ncolours;                /* Number of colours         */
		uint32 importantcolours;        /* Important colours         */
	} infoHeader;

#define NUVIEBMP_INFOHEADER_SIZE 40

public:

	NuvieBmpFile();
	~NuvieBmpFile();

	bool initNewBlankImage(sint32 width, sint32 height, const unsigned char *palette);

	bool load(Std::string filename);
	bool save(Std::string filename);

	uint16 getWidth() {
		return (uint16)infoHeader.width;
	}
	uint16 getHeight() {
		return (uint16)infoHeader.height;
	}


	Tile *getTile();
	unsigned char *getRawIndexedData();
	unsigned char *getRawIndexedDataCopy();
	Graphics::ManagedSurface *getSdlSurface32();
	Graphics::ManagedSurface *getSdlSurface32(Std::string filename);

private:
	bool handleError(Std::string error);
	void write8BitData(NuvieIOFileWrite *file);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
