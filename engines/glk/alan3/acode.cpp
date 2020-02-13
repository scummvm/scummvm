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

#include "glk/alan3/acode.h"

namespace Glk {
namespace Alan3 {

void AttributeEntry::synchronize(Common::Serializer &s) {
	// We have to do some annoying temporary copy of the fields to get around gcc
	// errors about getting references to fields of packed structures
	Aint c = code;
	Aptr v = value;
	Aaddr i = id;
	s.syncAsSint32LE(c);
	s.syncAsSint32LE(v);
	s.syncAsSint32LE(i);

	if (s.isLoading()) {
		code = c;
		value = v;
		id = i;
	}
}

} // End of namespace Alan3
} // End of namespace Glk
