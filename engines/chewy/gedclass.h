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

#ifndef CHEWY_GEDCLASS_H
#define CHEWY_GEDCLASS_H

#include "chewy/ngstypes.h"

namespace Chewy {

typedef int16(*GedUserFunc)(int16 idx_nr);

class GedClass {
public:
	GedClass(GedUserFunc func) : _gedUserFunc(func) {
	}

	void load_ged_pool(const char *fname, GedChunkHeader *Gh, int16 ch_nr, byte *speicher);

	void load_ged_pool(Common::SeekableReadStream *stream, GedChunkHeader *Gh, int16 ch_nr, byte *speicher);
	int16 ged_idx(int16 x, int16 y, int16 x_anz, byte *speicher);
	int16 ged_idx(int16 g_idx, int16 x_anz, byte *speicher);

private:
	void load_ged_chunk(GedChunkHeader *Gh, Common::SeekableReadStream *stream, int16 nr, byte *speicher);

	GedPoolHeader _gedPoolHeader;
	GedUserFunc _gedUserFunc;
};

} // namespace Chewy

#endif
