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

#ifndef DOUBLE_SERIALIZATION_H
#define DOUBLE_SERIALIZATION_H

#include "common/types.h"

/**
 * @defgroup lua_ds Double serialization
 * @ingroup lua
 *
 * @brief Functions for encoding and decoding double values.
 *
 * @{
 */

namespace Util {

struct SerializedDouble {
	uint32 significandOne;
	uint32 signAndSignificandTwo;
	int16 exponent;
};

struct CompactSerializedDouble {
	uint32 signAndSignificandOne;
	uint32 exponentAndSignificandTwo;
};

/**
 * Encodes a double as two uint32 and a one int16
 *
 * Supports denormalized numbers. Does NOT support NaN, or Inf
 *
 * @param value    The value to encode
 * @return         The encoded value
 */
SerializedDouble encodeDouble(double value);
/**
 * Decodes a previously encoded double
 *
 * @param value    The value to decode
 * @return         The decoded value
 */
double decodeDouble(SerializedDouble value);

} // End of namespace Util

/** @} */

#endif
