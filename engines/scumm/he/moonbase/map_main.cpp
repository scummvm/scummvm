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

#include "common/compression/deflate.h"

#include "common/memstream.h"
#include "common/bufferedstream.h"

#include "scumm/he/intern_he.h"

#include "scumm/he/moonbase/map_data.h"
#include "scumm/he/moonbase/map_main.h"

namespace Scumm {

Map::Map(ScummEngine_v100he *vm) : _vm(vm), _rnd("moonbase") {
}

Map::~Map() {
}

Common::SeekableReadStream *Map::substituteFile(const byte *fileName) {
	if (!strcmp((const char *)fileName, "map\\moon001.thm")) {
		// Load thumbnail data
		Common::MemoryReadStream *thumbnail = new Common::MemoryReadStream(Template_thm, ARRAYSIZE(Template_thm));
		Common::SeekableReadStream *stream = Common::wrapCompressedReadStream(thumbnail);
		return stream;
	}

	return nullptr;
}

} // End of namespace Scumm
