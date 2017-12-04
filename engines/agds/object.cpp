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
 */

#include "agds/object.h"
#include "common/debug.h"

namespace AGDS {

Object::Object(Common::SeekableReadStream * stream) {
	stream->skip(2);
	uint16 dataSize = stream->readUint16LE();
	if (dataSize != 0)
		error("implement me: object with data (%u)", dataSize);
	uint16 codeSize = stream->readUint16LE();
	uint8 flags = stream->readByte();
	if (flags != 1)
		error("implement me: no flags handling yet");

	debug("object code size %u", codeSize);
	_code.resize(codeSize);
	stream->read(_code.data(), _code.size());
	delete stream;
}

}
