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

#ifndef ARCHETYPE_XARRAY
#define ARCHETYPE_XARRAY

#include "common/array.h"

namespace Glk {
namespace Archetype {

typedef Common::Array<void *> XArrayType;

enum AccessType { POKE_ACCESS, PEEK_ACCESS };

/**
 * Initialize an extendible array
 */
extern void new_xarray(XArrayType &the_xarray);

/**
 * Disposes the array
 */
extern void dispose_xarray(XArrayType &the_xarray);

/**
 * Appends a new element to the end of the array
 */
extern void append_to_xarray(XArrayType &the_xarray, void *element);

/**
 * Access a given index in the array to either set or retrieve an entry
 * @param the_xarray			The array to access
 */
extern bool access_xarray(XArrayType &the_xarray, int index, void *&result, AccessType direction);

/**
 * Access a given index in the array
 */
extern bool index_xarray(XArrayType &the_xarray, int index, void *&result);

/**
 * Removes the last element of the array
 */
extern void shrink_xarray(XArrayType &the_xarray);

} // End of namespace Archetype
} // End of namespace Glk

#endif
