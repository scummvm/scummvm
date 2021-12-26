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

#include "teenagent/segment.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace TeenAgent {

void Segment::read(Common::ReadStream *stream, uint32 s) {
	_size = s;
	_data = new byte[_size];
	if (stream->read(_data, _size) != _size)
		error("Segment::read: corrupted data");
}

Segment::~Segment() {
	delete[] _data;
}

} // End of namespace TeenAgent
