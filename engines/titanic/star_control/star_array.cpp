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

#include "titanic/star_control/star_array.h"
#include "titanic/star_control/star_control_sub12.h"
#include "titanic/titanic.h"

namespace Titanic {

#define ARRAY_COUNT 876

CStarArray::CStarArray() {
}

void CStarArray::initialize() {
	// Get a reference to the starfield points resource
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource("STARFIELD/POINTS");
	assert(stream && stream->size() == (12 * ARRAY_COUNT));

	_data.resize(ARRAY_COUNT);
	for (int idx = 0; idx < ARRAY_COUNT; ++idx) {
		// Get the next set of values
		int v1 = stream->readUint32LE();
		int v2 = stream->readUint32LE();
		stream->readUint32LE();

		// Pre-process them
		// TODO
	}
}

void CStarArray::draw(CSurfaceArea *surface, CStarControlSub12 *img) {
	// TODO
}

} // End of namespace Titanic
