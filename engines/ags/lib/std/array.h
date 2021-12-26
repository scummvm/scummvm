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

#ifndef AGS_STD_ARRAY_H
#define AGS_STD_ARRAY_H

#include "common/array.h"

namespace AGS3 {
namespace std {

template<class T>
class array : public Common::Array<T> {
public:
	array() : Common::Array<T>() {
	}
	array(size_t size) : Common::Array<T>() {
		Common::Array<T>::resize(size);
	}
};

} // namespace std
} // namespace AGS3

#endif
