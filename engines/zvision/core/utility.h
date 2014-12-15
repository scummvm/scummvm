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

#ifndef ZVISION_UTILITY_H
#define ZVISION_UTILITY_H

#include "common/array.h"

namespace Common {
class String;
}

namespace ZVision {

class ZVision;

/**
 * Removes any line comments using '#' as a sequence start.
 * Then removes any trailing and leading 'whitespace' using String::trim()
 * Note: String::trim uses isspace() to determine what is whitespace and what is not.
 *
 * @param string    The string to modify. It is modified in place
 */
void trimCommentsAndWhiteSpace(Common::String *string);

} // End of namespace ZVision

#endif
