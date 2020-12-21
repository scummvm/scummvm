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

#include "twine/parser/sprite.h"
#include "common/stream.h"
#include "twine/shared.h"

namespace TwinE {

bool SpriteBoundingBoxData::loadFromStream(Common::SeekableReadStream &stream) {
	const int32 size = stream.size();
	const int32 amount = size / 16;
	for (int32 i = 0; i < amount; ++i) {
		stream.skip(4);
		BoundingBox bbox;
		bbox.mins.x = stream.readSint16LE();
		bbox.maxs.x = stream.readSint16LE();
		bbox.mins.y = stream.readSint16LE();
		bbox.maxs.y = stream.readSint16LE();
		bbox.mins.z = stream.readSint16LE();
		bbox.maxs.z = stream.readSint16LE();
		_boundingBoxes.push_back(bbox);
	}
	return !stream.err();
}

bool SpriteBoundingBoxData::loadFromBuffer(const uint8 *buf, uint32 size) {
	if (size == 0) {
		return false;
	}
	Common::MemoryReadStream stream(buf, size);
	return loadFromStream(stream);
}

} // End of namespace TwinE
