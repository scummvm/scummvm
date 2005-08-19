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
#include "common/stream.h"
#include "kyra/resource.h"
#include "kyra/screen.h"

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
		Screen::decodeFrame4(data + 10, _palette, imageSize);
		delete [] data;
	} else {
		_palette = data;
	}
}

} // end of namespace Kyra

