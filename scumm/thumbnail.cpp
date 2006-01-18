/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed file the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/scaler.h"
#include "scumm/scumm.h"

namespace Scumm {

#define THMB_VERSION 1

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct ThumbnailHeader {
	uint32 type;
	uint32 size;
	byte version;
	uint16 width, height;
	byte bpp;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif


inline void colorToRGB(uint16 color, uint8 &r, uint8 &g, uint8 &b) {
	r = (((color >> 11) & 0x1F) << 3);
	g = (((color >> 5) & 0x3F) << 2);
	b = ((color&0x1F) << 3);
}

Graphics::Surface *ScummEngine::loadThumbnail(Common::InSaveFile *file) {
	ThumbnailHeader header;
	file->read(&header.type, 4);
	// We also accept the bad 'BMHT' header here, for the sake of compatibility
	// with some older savegames which were written incorrectly due to a bug in
	// ScummVM which wrote the thumb header type incorrectly on LE systems.
	if (header.type != MKID('THMB') && header.type != MKID('BMHT'))
		return 0;

	header.size = file->readUint32BE();
	header.version = file->readByte();

	if (header.version > THMB_VERSION) {
		file->skip(header.size - 9);
		warning("Loading a newer thumbnail version");
		return 0;
	}

	header.width = file->readUint16BE();
	header.height = file->readUint16BE();
	header.bpp = file->readByte();

	// TODO: support other bpp values than 2
	if (header.bpp != 2) {
		file->skip(header.size - 14);
		return 0;
	}

	Graphics::Surface *thumb = new Graphics::Surface();
	thumb->create(header.width, header.height, sizeof(uint16));

	uint16* pixels = (uint16 *)thumb->pixels;

	for (int y = 0; y < thumb->h; ++y) {
		for (int x = 0; x < thumb->w; ++x) {
			uint8 r, g, b;
			colorToRGB(file->readUint16BE(), r, g, b);

			// converting to current OSystem Color
			*pixels++ = _system->RGBToColor(r, g, b);
		}
	}

	return thumb;
}

void ScummEngine::saveThumbnail(Common::OutSaveFile *file) {
	Graphics::Surface thumb;

#ifndef PALMOS_68K
	if (!createThumbnailFromScreen(&thumb))
#endif
		thumb.create(kThumbnailWidth, kThumbnailHeight2, sizeof(uint16));

	ThumbnailHeader header;
	header.type = MKID('THMB');
#ifdef PALMOS_ARM
	// sizeof(header) is hardcoded here, because the compiler add padding to
	// have a 4byte aligned struct and there is no easy way to pack it.
	header.size = 14 + thumb.w*thumb.h*thumb.bytesPerPixel;
#else
	header.size = sizeof(header) + thumb.w*thumb.h*thumb.bytesPerPixel;
#endif
	header.version = THMB_VERSION;
	header.width = thumb.w;
	header.height = thumb.h;
	header.bpp = thumb.bytesPerPixel;

	file->write(&header.type, 4);
	file->writeUint32BE(header.size);
	file->writeByte(header.version);
	file->writeUint16BE(header.width);
	file->writeUint16BE(header.height);
	file->writeByte(header.bpp);

	// TODO: for later this shouldn't be casted to uint16...
	uint16* pixels = (uint16 *)thumb.pixels;
	for (uint16 p = 0; p < thumb.w*thumb.h; ++p, ++pixels)
		file->writeUint16BE(*pixels);

	thumb.free();
}

} // end of namespace Scumm
