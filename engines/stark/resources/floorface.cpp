/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/floorface.h"
#include "engines/stark/xrcreader.h"
#include "engines/stark/debug.h"

namespace Stark {

FloorFace::FloorFace(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		Resource(parent, subType, index, name),
		_unk1(0),
		_unk2(0) {
	_type = TYPE;

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_indices[i] = 0;
	}
}

FloorFace::~FloorFace() {
}

void FloorFace::readData(XRCReadStream *stream) {
	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_indices[i] = stream->readSint16LE();
	}

	_unk1 = stream->readFloat();

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		stream->readSint16LE(); // Skipped in the original
	}

	_unk2 = stream->readFloat();
}

void FloorFace::printData() {
	debug("indices: %d %d %d, unk1 %f, unk2 %f", _indices[0], _indices[1], _indices[2], _unk1, _unk2);
}

} // End of namespace Stark
