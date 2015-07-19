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

#include "engines/stark/resources/lipsync.h"

#include "engines/stark/formats/xrc.h"

namespace Stark {
namespace Resources {

LipSync::~LipSync() {
}

LipSync::LipSync(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Object(parent, subType, index, name) {
	_type = TYPE;
}

void LipSync::readData(Formats::XRCReadStream *stream) {
	uint32 shapeCount = stream->readUint32LE();
	for (uint i = 0; i < shapeCount; i++) {
		uint32 shape = stream->readUint32LE();
		_shapes.push_back(shape);

		// The original does not use that data
		stream->skip(4);
	}

	// The original does not use that data
	uint32 unkCount = stream->readUint32LE();
	stream->skip(unkCount);
}

void LipSync::printData() {
	Object::printData();

	Common::String phrase;
	for (uint i = 0; i < _shapes.size(); i++) {
		phrase += _shapes[i];
	}

	debug("shapes: %s", phrase.c_str());
}

} // End of namespace Resources
} // End of namespace Stark
