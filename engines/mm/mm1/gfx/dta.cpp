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
#include "mm/mm1/gfx/dta.h"
#include "mm/mm1/gfx/screen_decoder.h"

namespace MM {
namespace MM1 {
namespace Gfx {

Common::SeekableReadStream *DTA::load(uint entryIndex) {
	Common::File f;

	if (!f.open(_fname))
		error("Could not open - %s", _fname.c_str());

	uint indexSize = f.readUint16LE();
	assert(entryIndex < (indexSize / 4));
	f.seek(entryIndex * 4, SEEK_CUR);
	size_t entryOffset = f.readUint32LE();
	size_t nextOffset = (entryIndex == (indexSize / 4 - 1)) ?
		f.size() : f.readUint32LE();

	f.seek(2 + indexSize + entryOffset);
	return f.readStream(nextOffset - entryOffset);
}

} // namespace Gfx
} // namespace MM1
} // namespace MM
