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

#include "engines/stark/resources/path.h"

#include "engines/stark/formats/xrc.h"

namespace Stark {
namespace Resources {

Object *Path::construct(Object *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kPath2D:
		return new Path2D(parent, subType, index, name);
	case kPath3D:
		return new Path3D(parent, subType, index, name);
	default:
		error("Unknown path subtype %d", subType);
	}
}

Path::~Path() {
}

Path::Path(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Object(parent, subType, index, name) {
	_type = TYPE;
}

void Path::readData(Formats::XRCReadStream *stream) {
	_field_30 = stream->readUint32LE();
}

void Path::printData() {
	debug("field_30: %d", _field_30);
}

Path2D::Path2D(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Path(parent, subType, index, name) {
}

void Path2D::readData(Formats::XRCReadStream *stream) {
	Path::readData(stream);

	uint32 stepCount = stream->readUint32LE();
	for (uint i = 0; i < stepCount; i++) {
		Step step;
		step.weight = stream->readFloat();
		step.position = stream->readPoint();

		_steps.push_back(step);
	}

	stream->readUint32LE(); // Unused in the original
}

void Path2D::printData() {
	Path::printData();

	for (uint i = 0; i < _steps.size(); i++) {
		debug("step[%d]: (x %d, y %d), weight: %f", i,
		      _steps[i].position.x, _steps[i].position.y, _steps[i].weight);
	}
}

Path2D::~Path2D() {
}

Path3D::Path3D(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Path(parent, subType, index, name),
		_sortKey(0) {
}

void Path3D::readData(Formats::XRCReadStream *stream) {
	Path::readData(stream);

	uint32 stepCount = stream->readUint32LE();
	for (uint i = 0; i < stepCount; i++) {
		Step step;
		step.weight = stream->readFloat();
		step.position = stream->readVector3();

		_steps.push_back(step);
	}

	_sortKey = stream->readFloat();
}

void Path3D::printData() {
	Path::printData();

	for (uint i = 0; i < _steps.size(); i++) {
		debug("step[%d]: (x %f, y %f, z %f), weight: %f", i,
		      _steps[i].position.x(), _steps[i].position.y(), _steps[i].position.z(), _steps[i].weight);
	}

	debug("sortKey: %f", _sortKey);
}

Path3D::~Path3D() {
}

} // End of namespace Resources
} // End of namespace Stark
