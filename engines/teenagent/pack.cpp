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
 * $URL$
 * $Id$
 */

#include "teenagent/pack.h"
#include "common/util.h"
#include "common/debug.h"

namespace TeenAgent {

Pack::Pack() : count(0), offsets(0) {}

Pack::~Pack() {
	close();
}


void Pack::close() {
	delete[] offsets;
	offsets = NULL;
	file.close();
}


void Pack::open(const Common::String &filename) {
	file.open(filename);
	count = file.readUint32LE();
	debug(0, "opened %s, found %u entries", filename.c_str(), count);
	offsets = new uint32[count + 1];
	for (uint32 i = 0; i <= count; ++i) {
		offsets[i] = file.readUint32LE();
		//debug(0, "%d: %06x", i, offsets[i]);
	}
/*	for (uint32 i = 0; i < count; ++i) {
		debug(0, "%d: len = %d", i, offsets[i + 1] - offsets[i]);
	}
*/
}

uint32 Pack::get_size(uint32 id) const {
	if (id < 1 || id > count)
		return 0;
	return offsets[id] - offsets[id - 1];
}

uint32 Pack::read(uint32 id, byte *dst, uint32 size) const {
	if (id < 1 || id > count)
		return 0;

	file.seek(offsets[id - 1]);
	uint32 rsize = offsets[id] - offsets[id - 1];
	uint32 r = file.read(dst, MIN(rsize, size));
	//debug(0, "read(%u, %u) = %u", id, size, r);
	return r;
}

Common::SeekableReadStream * Pack::getStream(uint32 id) const {
	if (id < 1 || id > count)
		return 0;
	debug(0, "stream: %04x-%04x", offsets[id - 1], offsets[id]);
	return new Common::SeekableSubReadStream(&file, offsets[id - 1], offsets[id], false);
}

} // End of namespace TeenAgent
