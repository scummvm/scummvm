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

#include "glk/archetype/array.h"

namespace Glk {
namespace Archetype {

void new_xarray(XArrayType &the_xarray) {
	the_xarray.clear();
}

void dispose_xarray(XArrayType &the_xarray) {
	the_xarray.clear();
}

void append_to_xarray(XArrayType &the_xarray, void *element) {
	the_xarray.push_back(element);
}

bool access_xarray(XArrayType &the_xarray, int index, void *&result, AccessType direction) {
	if (index <= 0)
		error("Invalid index - double check arrays were 1 based in original");

	if (index > (int)the_xarray.size())
		return false;

	switch (direction) {
	case PEEK_ACCESS:
		result = the_xarray[index - 1];
		break;
	case POKE_ACCESS:
		the_xarray[index - 1] = result;
		break;
	default:
		break;
	}

	return true;
}

bool index_xarray(XArrayType &the_xarray, int index, void *&result) {
	return access_xarray(the_xarray, index, result, PEEK_ACCESS);
}

void shrink_xarray(XArrayType &the_xarray) {
	if (!the_xarray.empty())
		the_xarray.resize(the_xarray.size() - 1);
}

} // End of namespace Archetype
} // End of namespace Glk
