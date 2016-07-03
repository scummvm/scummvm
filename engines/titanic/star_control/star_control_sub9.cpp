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

#include "titanic/star_control/star_control_sub9.h"
#include "titanic/titanic.h"

namespace Titanic {

#define ARRAY_COUNT 80

void CStarControlSub9::initialize() {
	// Get a reference to the starfield points resource
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource("STARFIELD/POINTS2");

	_data.resize(ARRAY_COUNT);
	for (int rootCtr = 0; rootCtr < ARRAY_COUNT; ++rootCtr) {
		// Get the number of sub-entries for this entry
		int count = stream->readUint32LE();

		// Read in the sub-entries
		RootEntry &rootEntry = _data[rootCtr];
		rootEntry.resize(count);
		for (int idx = 0; idx < count; ++idx) {
			int v1 = stream->readSint32LE();
			int v2 = stream->readSint32LE();
			int v3 = stream->readSint32LE();
			int v4 = stream->readSint32LE();

			// TODO: Processing here
		}
	}
}

} // End of namespace Titanic
