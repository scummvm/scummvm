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

#include "engines/stark/resources/knowledge.h"

#include "engines/stark/formats/xrc.h"

#include "engines/stark/services/stateprovider.h"

namespace Stark {
namespace Resources {

Knowledge::~Knowledge() {
}

Knowledge::Knowledge(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_booleanValue(false),
		_integerValue(0) {
	_type = TYPE;
}

void Knowledge::setBooleanValue(bool value) {
	_booleanValue = value;
}

bool Knowledge::getBooleanValue() {
	return _booleanValue;
}

void Knowledge::setIntegerValue(int32 value) {
	_integerValue = value;
}

int32 Knowledge::getIntegerValue() {
	return _integerValue;
}

void Knowledge::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsSint32LE(_integerValue);
	serializer->syncAsSint32LE(_booleanValue);
	serializer->syncAsResourceReference(_referenceValue);
}

void Knowledge::readData(Formats::XRCReadStream *stream) {
	Object::readData(stream);

	switch (_subType) {
	case kBoolean:
	case kBooleanWithChild:
		_booleanValue = stream->readBool();
		break;
	case kInteger:
	case kInteger2:
		_integerValue = stream->readSint32LE();
		break;
	case kReference:
		_referenceValue = stream->readResourceReference();
		break;
	default:
		error("Unknown knowledge subtype %d", _subType);
	}
}

void Knowledge::printData() {
	switch (_subType) {
	case kBoolean:
	case kBooleanWithChild:
		debug("value: %d", _booleanValue);
		break;
	case kInteger:
	case kInteger2:
		debug("value: %d", _integerValue);
		break;
	case kReference:
		debug("value: %s", _referenceValue.describe().c_str());
		break;
	default:
		error("Unknown knowledge subtype %d", _subType);
	}
}

} // End of namespace Resources
} // End of namespace Stark
