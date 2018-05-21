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

#ifndef PINK_UTILS_H
#define PINK_UTILS_H

#include "pink/archive.h"

namespace Pink {

template <typename T>
class Array : public Common::Array<T>, public Object {
public:
	virtual void deserialize(Archive &archive) {
		uint size = archive.readCount();
		this->resize(size);
		for (uint i = 0; i < size; ++i) {
			this->data()[i] = reinterpret_cast<T>(archive.readObject()); // dynamic_cast needs to know complete type
		}
	}
};

class StringArray : public Common::StringArray {
public:
	inline void deserialize(Archive &archive) {
		uint32 size = archive.readCount();
		this->resize(size);
		for (uint i = 0; i < size; ++i) {
			this->data()[i] = archive.readString();
		}
	}
};

} // End of namespace Pink

#endif