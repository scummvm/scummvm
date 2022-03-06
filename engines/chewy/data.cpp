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
#include "chewy/chewy.h"
#include "chewy/defines.h"
#include "chewy/globals.h"

namespace Chewy {

Data::Data() {
}

Data::~Data() {
}

uint16 Data::select_pool_item(Common::Stream *stream, uint16 nr) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);

	if (rs) {
		rs->seek(0, SEEK_SET);
		NewPhead ph;
		if (!ph.load(rs))
			error("select_pool_item error");

		if (!strncmp(ph._id, "NGS", 3)) {
			if (nr >= ph._poolNr)
				nr = ph._poolNr - 1;

			rs->seek(-(int)((ph._poolNr - nr) * sizeof(uint32)), SEEK_END);
			uint32 tmp1 = rs->readUint32LE();
			rs->seek(tmp1, SEEK_SET);
		}
	}

	return nr;
}

uint32 Data::load_tmf(Common::Stream *handle, TmfHeader *song) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(handle);
	uint32 size = 0;

	if (rs) {
		rs->seek(-ChunkHead::SIZE(), SEEK_CUR);
		ChunkHead ch;
		if (!ch.load(rs))
			error("load_tmf error");

		if (ch.type != TMFDATA)
			error("load_tmf error");

		assert(ch.size > (uint32)TmfHeader::SIZE());

		if (!song->load(rs))
			error("load_tmf error");

		size = ch.size + sizeof(TmfHeader);
		byte *speicher = (byte *)song + sizeof(TmfHeader);
		speicher += ((uint32)song->_patternNr) * 1024l;
		for (int16 i = 0; i < 31; ++i) {
			if (song->instrument[i].laenge) {
				song->ipos[i] = speicher;
				speicher += song->instrument[i].laenge;
			}
		}
	}

	return size;
}

uint32 Data::get_poolsize(const char *fname, int16 chunk_start, int16 chunk_anz) {
	uint32 size = 0;

	Common::File f;
	if (!f.open(fname))
		error("get_poolsize error");

	NewPhead Nph;
	if (!Nph.load(&f))
		error("get_poolsize error");

	if (!strncmp(Nph._id, "NGS", 3)) {
		select_pool_item(&f, chunk_start);
		f.seek(-ChunkHead::SIZE(), SEEK_CUR);

		for (int16 i = chunk_start; (i < Nph._poolNr) && i < (chunk_start + chunk_anz); i++) {
			ChunkHead ch;
			if (!ch.load(&f))
				error("get_poolsize error");

			if (ch.size > size)
				size = ch.size;

			f.seek(ch.size, SEEK_CUR);
		}
	}

	f.close();

	return size;
}

} // namespace Chewy
