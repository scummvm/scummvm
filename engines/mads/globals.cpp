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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/globals.h"

namespace MADS {

void Globals::reset() {
	for (uint i = 0; i < _data.size(); ++i)
		_data[i] = 0;
}

void Globals::synchronize(Common::Serializer &s) {
	int count = 0;
	int16 v;
	s.syncAsUint16LE(count);

	if (s.isSaving()) {
		for (int idx = 0; idx < count; ++idx) {
			v = _data[idx];
			s.syncAsSint16LE(v);
		}	
	} else {
		_data.clear();
		for (int idx = 0; idx < count; ++idx) {
			s.syncAsSint16LE(v);
			_data.push_back(v);
		}
	}
}


} // End of namespace MADS
