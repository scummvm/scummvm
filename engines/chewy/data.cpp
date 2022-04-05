/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/system.h"
#include "chewy/data.h"
#include "chewy/ngsdefs.h"
#include "chewy/globals.h"

namespace Chewy {

void Data::selectPoolItem(Common::File *stream, uint16 nr) {
	if (stream) {
		stream->seek(0, SEEK_SET);
		NewPhead ph;
		if (!ph.load(stream))
			error("selectPoolItem error");

		if (!strncmp(ph._id, "NGS", 3)) {
			if (nr >= ph._poolNr)
				nr = ph._poolNr - 1;

			stream->seek(-(int)((ph._poolNr - nr) * sizeof(uint32)), SEEK_END);
			uint32 tmp1 = stream->readUint32LE();
			stream->seek(tmp1, SEEK_SET);
		}
	}
}

uint32 Data::getPoolSize(const char *filename, int16 chunkStart, int16 chunkNr) {
	uint32 size = 0;

	Common::File f;
	if (!f.open(filename))
		error("getPoolSize error");

	NewPhead Nph;
	if (!Nph.load(&f))
		error("getPoolSize error");

	if (!strncmp(Nph._id, "NGS", 3)) {
		selectPoolItem(&f, chunkStart);
		f.seek(-ChunkHead::SIZE(), SEEK_CUR);

		for (int16 i = chunkStart; (i < Nph._poolNr) && i < (chunkStart + chunkNr); i++) {
			ChunkHead ch;
			if (!ch.load(&f))
				error("getPoolSize error");

			if (ch.size > size)
				size = ch.size;

			f.seek(ch.size, SEEK_CUR);
		}
	}

	f.close();

	return size;
}

} // namespace Chewy
