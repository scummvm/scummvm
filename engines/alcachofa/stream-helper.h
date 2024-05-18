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

#ifndef STREAM_HELPER_H
#define STREAM_HELPER_H

#include "common/stream.h"
#include "common/serializer.h"
#include "common/rect.h"

namespace Alcachofa {

bool readBool(Common::ReadStream &stream);
Common::Point readPoint(Common::ReadStream &stream);
Common::String readVarString(Common::ReadStream &stream);
void skipVarString(Common::SeekableReadStream &stream);

void syncPoint(Common::Serializer &serializer, Common::Point &point);

template<typename T>
inline void syncArray(Common::Serializer &serializer, Common::Array<T> &array, void (*serializeFunction)(Common::Serializer &, T &)) {
	auto size = array.size();
	serializer.syncAsUint32LE(size);
	array.resize(size);
	serializer.syncArray(array.data(), size, serializeFunction);
}

template<typename T>
inline void syncEnum(Common::Serializer &serializer, T &enumValue) {
	// syncAs does not have a cast for saving
	int32 intValue = static_cast<int32>(enumValue);
	serializer.syncAsSint32LE(intValue);
	enumValue = static_cast<T>(intValue);
}

}

#endif // STREAM_HELPER_H
