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

#ifndef COMMON_COMPRESSION_UNP64_H
#define COMMON_COMPRESSION_UNP64_H

#include "common/scummsys.h"

namespace Common {
namespace Unp64 {

/**
 * Unpack a C64 PRG, preserving its two-byte load address.
 * destinationBuffer must hold 65536 bytes. Returns zero on failure.
 */
int unp64(const byte *compressed, uint32 length, byte *destinationBuffer, uint32 *finalLength, const char *settings);

} // End of namespace Unp64
} // End of namespace Common

#endif
