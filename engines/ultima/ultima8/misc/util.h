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

#ifndef ULTIMA8_MISC_UTIL_H
#define ULTIMA8_MISC_UTIL_H

#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/misc.h"
#include "ultima/ultima8/misc/istring.h"

namespace Ultima {
namespace Ultima8 {

template<class T> void StringToArgv(const T &args, Std::vector<T> &argv);
template<class T> void ArgvToString(const Std::vector<T> &argv, T &args);


template<class T> void TrimSpaces(T &str);

template<class T> void TabsToSpaces(T &str, unsigned int nspaces);

template<class T> void SplitString(const T &args, char sep, Std::vector<T> &argv);

template<class T> void SplitStringKV(const T &args, char sep,
                                     Std::vector<Std::pair<T, T> > &argv);

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
