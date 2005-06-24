/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "kyra/resource.h"

#include "common/stream.h"
#include "kyra/codecs.h"

namespace Kyra {

Palette::Palette(uint8* data, uint32 size) {
	if (!data) {
		error("resource created without data");
	}
	
	if (size != 768) {
		Common::MemoryReadStream datastream(data, size);
		
		datastream.readSint32LE();
		int imageSize = datastream.readSint16LE();
		
		if (imageSize != 768) {
			error("decompresed palette is not 768 byte long!");
		}
		
		// lets uncompress this palette :)
		_palette = new uint8[imageSize];
		assert(_palette);
		
		// made decompression
		if (Compression::decode80(data + 10, _palette) != 768) {
			error("decode80 decompressesize != 768 bytes");
		}
		
		delete [] data;
		data = _palette;
	}
		
	// hmm.. common/system.h Docu is wrong or SDL Backend has a bug :)
	// a palette should have this order:
	// R1-G1-B1-A1-R2-G2-B2-A2-...
	// so we need 4 bytes per color
	_palette = new uint8[256 * 4];
		
	uint8* currentpossrc = &data[0];
	uint8* currentposdst = &_palette[0];
		
	// creates the original pallette (only first 6 bits are used)
	for (uint32 i = 0; i < 256; i++) {
		currentposdst[0] = currentpossrc[0] << 2;
		currentposdst[1] = currentpossrc[1] << 2;
		currentposdst[2] = currentpossrc[2] << 2;
		currentpossrc += 3;
		currentposdst += 4;
	}
		
	delete [] data;
}

} // end of namespace Kyra

