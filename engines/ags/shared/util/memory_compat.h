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

#ifndef AGS_SHARED_UTIL_MEMORY_COMPAT_H
#define AGS_SHARED_UTIL_MEMORY_COMPAT_H

#include "ags/lib/std/memory.h"

namespace AGS3 {

// C++14 features
#if __cplusplus < 201402L && !((defined(_MSC_VER) && _MSC_VER >= 1900))
namespace std {
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
} // std
#endif

} // namespace AGS3

#endif
