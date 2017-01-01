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

#include "titanic/star_control/star_points2.h"
#include "titanic/titanic.h"

namespace Titanic {

#define ARRAY_COUNT 80
const double FACTOR = 3.1415927 * 0.0055555557;

bool CStarPoints2::initialize() {
	// Get a reference to the starfield points resource
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource("STARFIELD/POINTS2");

	_data.resize(ARRAY_COUNT);
	for (int rootCtr = 0; rootCtr < ARRAY_COUNT; ++rootCtr) {
		// Get the number of sub-entries for this entry
		int count = stream->readUint32LE();
		double v1, v2;

		// Read in the sub-entries
		RootEntry &rootEntry = _data[rootCtr];
		rootEntry.resize(count * 2);
		for (int idx = 0; idx < count * 2; ++idx) {
			DataEntry &entry = rootEntry[idx];
			v1 = stream->readSint32LE();
			v2 = stream->readSint32LE();
			v1 *= 0.015 * FACTOR;
			v2 *= 0.0099999998 * FACTOR;
			entry._v1 = cos(v1) * 3000000.0 * cos(v2);
			entry._v2 = sin(v1) * 3000000.0 * cos(v2);
			entry._v3 = sin(v2) * 3000000.0;
		}
	}

	return true;
}

} // End of namespace Titanic
