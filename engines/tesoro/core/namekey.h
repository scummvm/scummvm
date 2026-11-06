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

#ifndef TESORO_CORE_NAMEKEY_H
#define TESORO_CORE_NAMEKEY_H

#include "common/scummsys.h"

namespace Tesoro {

/** Packs the first 12 characters of a name into a 64 bit key, 5 bits per character. */
uint64 nameKey64(const char *name);

/** Spells a key back as 12 characters (padded with spaces) and an end mark; out has room for 13. */
void nameFromKey(uint64 key, char *out);

} // End of namespace Tesoro

#endif
