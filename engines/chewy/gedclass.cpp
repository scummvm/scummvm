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

#include "common/file.h"
#include "chewy/chewy.h"
#include "chewy/globals.h"
#include "chewy/gedclass.h"

namespace Chewy {

void GedClass::load_ged_pool(const char *fname, GedChunkHeader *Gh, int16 ch_nr, byte *speicher) {
	Common::File f;
	if (f.open(fname)) {
		load_ged_pool(&f, Gh, ch_nr, speicher);
	} else {
		error("load_ged_pool error");
	}
}

void GedClass::load_ged_pool(Common::SeekableReadStream *stream, GedChunkHeader *Gh, int16 ch_nr, byte *speicher) {
	if (stream) {
		stream->seek(0, SEEK_SET);
		if (_gedPoolHeader.load(stream)) {
			load_ged_chunk(Gh, stream, ch_nr, speicher);
		}
	} else {
		error("load_ged_pool error");
	}
}

void GedClass::load_ged_chunk(GedChunkHeader *Gh, Common::SeekableReadStream *stream, int16 nr, byte *speicher) {
	if (stream) {
		// Scan for the correct index entry
		int i = 0;
		do {
			Gh->load(stream);
			if (i != nr) {
				// Skip over the entry's data
				stream->seek(Gh->Len, SEEK_CUR);
			}
		} while (++i <= nr);

		if (stream->read(speicher, Gh->Len) != Gh->Len) {
			error("load_ged_chunk error");
		}
	} else {
		error("load_ged_chunk error");
	}
}

int16 GedClass::ged_idx(int16 x, int16 y, int16 x_anz, byte *speicher) {
	int16 result = 0;
	if (_gedUserFunc)
		result = _gedUserFunc(speicher[((y / 8) * x_anz) + (x / 8)]);

	return result;
}

int16 GedClass::ged_idx(int16 g_idx, int16 x_anz, byte *speicher) {
	int16 result = 0;
	if (_gedUserFunc)
		result = _gedUserFunc(speicher[g_idx]);

	return result;
}

} // namespace Chewy
