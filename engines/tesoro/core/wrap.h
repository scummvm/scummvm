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

#ifndef TESORO_CORE_WRAP_H
#define TESORO_CORE_WRAP_H

#include "common/scummsys.h"

namespace Tesoro {

/**
 * 32-bit arithmetic that wraps like the original's.
 * The original multiplies and adds in 32 bits and lets the result wrap
 */
inline int32 wrapMul(int32 a, int32 b) { return (int32)((uint32)a * (uint32)b); }
inline int32 wrapAdd(int32 a, int32 b) { return (int32)((uint32)a + (uint32)b); }
inline int32 wrapSub(int32 a, int32 b) { return (int32)((uint32)a - (uint32)b); }

} // End of namespace Tesoro

#endif
