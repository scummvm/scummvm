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

#include "bladerunner/set.h"

#include "bladerunner/bladerunner.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/stream.h"

namespace BladeRunner {

#define kSet0 0x53657430

Set::Set(BladeRunnerEngine *vm) : _vm(vm) {
	_objectCount = 0;
	_walkboxCount = 0;
	_objects = new Object[85];
	_walkboxes = new Walkbox[95];
}

Set::~Set() {
	delete[] _objects;
	delete[] _walkboxes;
}

bool Set::open(const Common::String &name) {
	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->getResourceStream(name));

	uint32 sig = s->readUint32BE();
	if (sig != kSet0)
		return false;

	s->skip(4); // TODO: LITE length

	_objectCount = s->readUint32LE();
	assert(_objectCount <= 85);

	for (uint32 i = 0; i != _objectCount; ++i) {
		s->read(_objects[i]._name, 20);

		float x0, y0, z0, x1, y1, z1;
		x0 = s->readFloatLE();
		y0 = s->readFloatLE();
		z0 = s->readFloatLE();
		x1 = s->readFloatLE();
		y1 = s->readFloatLE();
		z1 = s->readFloatLE();

		_objects[i]._bbox = BoundingBox(x0, y0, z0, x1, y1, z1);

		_objects[i]._isObstacle = s->readByte();
		_objects[i]._isClickable = s->readByte();
		_objects[i]._isHotMouse = 0;
		_objects[i]._isCombatTarget = 0;
		s->skip(4);

		// debug("OBJECT: %s", _objects[i]._name);
	}

	_walkboxCount = s->readUint32LE();
	assert(_walkboxCount <= 95);

	for (uint32 i = 0; i != _walkboxCount; ++i) {
		float x, y, z;

		s->read(_walkboxes[i]._name, 20);
		y = s->readFloatLE();
		_walkboxes[i]._vertexCount = s->readUint32LE();

		assert(_walkboxes[i]._vertexCount <= 8);

		for (uint32 j = 0; j != _walkboxes[i]._vertexCount; ++j) {
			x = s->readFloatLE();
			z = s->readFloatLE();

			_walkboxes[i]._vertices[j] = Vector3(x, y, z);
		}

		// debug("WALKBOX: %s", _walkboxes[i]._name);
	}

	// TODO: Read LITE

	return true;
}

} // End of namespace BladeRunner
