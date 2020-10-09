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

#include "engines/stark/resources/scroll.h"

#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/services/stateprovider.h"

namespace Stark {
namespace Resources {

Scroll::~Scroll() {
}

Scroll::Scroll(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_coordinate(0),
		_field_30(0),
		_field_34(0),
		_bookmarkIndex(0),
		_active(false) {
	_type = TYPE;
}

void Scroll::applyToLocationImmediate() {
	Location *location = findParent<Location>();
	location->scrollToCoordinateImmediate(_coordinate);
}

void Scroll::start() {
	_active = true;

	Location *location = findParent<Location>();
	location->setHasActiveScroll();
}

void Scroll::stop() {
	_active = false;
}

bool Scroll::isActive() {
	return _active;
}

void Scroll::onGameLoop() {
	Object::onGameLoop();

	if (_active) {
		Location *location = findParent<Location>();
		bool complete = location->scrollToCoordinateSmooth(_coordinate);
		if (complete) {
			_active = false;
			location->stopAllScrolls();
		}
	}
}

void Scroll::readData(Formats::XRCReadStream *stream) {
	_coordinate = stream->readUint32LE();
	_field_30 = stream->readUint32LE();
	_field_34 = stream->readUint32LE();
	_bookmarkIndex = stream->readUint32LE();
}

void Scroll::saveLoadCurrent(ResourceSerializer *serializer) {
	serializer->syncAsUint32LE(_active);

	if (serializer->isLoading() && _active) {
		start();
	}
}

void Scroll::printData() {
	debug("coordinate: %d", _coordinate);
	debug("field_30: %d", _field_30);
	debug("field_34: %d", _field_34);
	debug("bookmarkIndex: %d", _bookmarkIndex);
}

} // End of namespace Resources
} // End of namespace Stark
