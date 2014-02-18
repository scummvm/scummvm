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

#include "common/scummsys.h"

#include "zvision/utility/single_value_container.h"

#include "common/textconsole.h"
#include "common/str.h"


namespace ZVision {

SingleValueContainer::SingleValueContainer(ValueType type) : _objectType(type) { }

SingleValueContainer::SingleValueContainer(bool value) : _objectType(BOOL) {
	_value.boolVal = value;
}

SingleValueContainer::SingleValueContainer(byte value) : _objectType(BYTE) {
	_value.byteVal = value;
}

SingleValueContainer::SingleValueContainer(int16 value) : _objectType(INT16) {
	_value.int16Val = value;
}

SingleValueContainer::SingleValueContainer(uint16 value) : _objectType(UINT16) {
	_value.uint16Val = value;
}

SingleValueContainer::SingleValueContainer(int32 value) : _objectType(INT32) {
	_value.int32Val = value;
}

SingleValueContainer::SingleValueContainer(uint32 value) : _objectType(UINT32) {
	_value.uint32Val = value;
}

SingleValueContainer::SingleValueContainer(float value) : _objectType(FLOAT) {
	_value.floatVal = value;
}

SingleValueContainer::SingleValueContainer(double value) : _objectType(DOUBLE) {
	_value.doubleVal = value;
}

SingleValueContainer::SingleValueContainer(Common::String value) : _objectType(BYTE) {
	_value.stringVal = new char[value.size() + 1];
	memcpy(_value.stringVal, value.c_str(), value.size() + 1);
}

SingleValueContainer::SingleValueContainer(const SingleValueContainer &other) {
	_objectType = other._objectType;

	switch (_objectType) {
	case BOOL:
		_value.boolVal = other._value.boolVal;
		break;
	case BYTE:
		_value.byteVal = other._value.byteVal;
		break;
	case INT16:
		_value.int16Val = other._value.int16Val;
		break;
	case UINT16:
		_value.uint16Val = other._value.uint16Val;
		break;
	case INT32:
		_value.int32Val = other._value.int32Val;
		break;
	case UINT32:
		_value.uint32Val = other._value.uint32Val;
		break;
	case FLOAT:
		_value.floatVal = other._value.floatVal;
		break;
	case DOUBLE:
		_value.doubleVal = other._value.doubleVal;
		break;
	case STRING:
		uint32 length = strlen(other._value.stringVal);
		_value.stringVal = new char[length + 1];
		memcpy(_value.stringVal, other._value.stringVal, length + 1);
		break;
	}
}

SingleValueContainer::~SingleValueContainer() {
	deleteCharPointer();
}

void SingleValueContainer::deleteCharPointer() {
	if (_objectType == STRING)
		delete[] _value.stringVal;
}


SingleValueContainer &SingleValueContainer::operator=(const bool &rhs) {
	if (_objectType == BOOL) {
		_value.boolVal = rhs;
		return *this;
	}

	deleteCharPointer();
	_objectType = BOOL;
	_value.boolVal = rhs;

	return *this;
}

SingleValueContainer &SingleValueContainer::operator=(const byte &rhs) {
	if (_objectType == BYTE) {
		_value.byteVal = rhs;
		return *this;
	}

	deleteCharPointer();
	_objectType = BYTE;
	_value.byteVal = rhs;

	return *this;
}

SingleValueContainer &SingleValueContainer::operator=(const int16 &rhs) {
	if (_objectType == INT16) {
		_value.int16Val = rhs;
		return *this;
	}

	deleteCharPointer();
	_objectType = INT16;
	_value.int16Val = rhs;

	return *this;
}

SingleValueContainer &SingleValueContainer::operator=(const uint16 &rhs) {
	if (_objectType == UINT16) {
		_value.uint16Val = rhs;
		return *this;
	}

	deleteCharPointer();
	_objectType = UINT16;
	_value.uint16Val = rhs;

	return *this;
}

SingleValueContainer &SingleValueContainer::operator=(const int32 &rhs) {
	if (_objectType == INT32) {
		_value.int32Val = rhs;
		return *this;
	}

	deleteCharPointer();
	_objectType = INT32;
	_value.int32Val = rhs;

	return *this;
}

SingleValueContainer &SingleValueContainer::operator=(const uint32 &rhs) {
	if (_objectType == UINT32) {
		_value.uint32Val = rhs;
		return *this;
	}

	deleteCharPointer();
	_objectType = UINT32;
	_value.uint32Val = rhs;

	return *this;
}

SingleValueContainer &SingleValueContainer::operator=(const float &rhs) {
	if (_objectType == FLOAT) {
		_value.floatVal = rhs;
		return *this;
	}

	deleteCharPointer();
	_objectType = FLOAT;
	_value.floatVal = rhs;

	return *this;
}

SingleValueContainer &SingleValueContainer::operator=(const double &rhs) {
	if (_objectType == DOUBLE) {
		_value.doubleVal = rhs;
		return *this;
	}

	deleteCharPointer();
	_objectType = DOUBLE;
	_value.doubleVal = rhs;

	return *this;
}

SingleValueContainer &SingleValueContainer::operator=(const Common::String &rhs) {
	if (_objectType != STRING) {
		_objectType = STRING;
		_value.stringVal = new char[rhs.size() + 1];
		memcpy(_value.stringVal, rhs.c_str(), rhs.size() + 1);

		return *this;
	}

	uint32 length = strlen(_value.stringVal);
	if (length <= rhs.size() + 1) {
		memcpy(_value.stringVal, rhs.c_str(), rhs.size() + 1);
	} else {
		delete[] _value.stringVal;
		_value.stringVal = new char[rhs.size() + 1];
		memcpy(_value.stringVal, rhs.c_str(), rhs.size() + 1);
	}

	return *this;
}

SingleValueContainer &SingleValueContainer::operator=(const SingleValueContainer &rhs) {
	switch (_objectType) {
	case BOOL:
		return operator=(rhs._value.boolVal);
	case BYTE:
		return operator=(rhs._value.byteVal);
	case INT16:
		return operator=(rhs._value.int16Val);
	case UINT16:
		return operator=(rhs._value.uint16Val);
	case INT32:
		return operator=(rhs._value.int32Val);
	case UINT32:
		return operator=(rhs._value.uint32Val);
	case FLOAT:
		return operator=(rhs._value.floatVal);
	case DOUBLE:
		return operator=(rhs._value.doubleVal);
	case STRING:
		uint32 length = strlen(rhs._value.stringVal);

		_value.stringVal = new char[length + 1];
		memcpy(_value.stringVal, rhs._value.stringVal, length + 1);

		return *this;
	}

	return *this;
}


bool SingleValueContainer::getBoolValue(bool *returnValue) const {
	if (_objectType !=  BOOL) {
		warning("'Object' is not storing a bool.");
		return false;
	}

	*returnValue = _value.boolVal;
	return true;
}

bool SingleValueContainer::getByteValue(byte *returnValue) const {
	if (_objectType !=  BYTE)
		warning("'Object' is not storing a byte.");

	*returnValue = _value.byteVal;
	return true;
}

bool SingleValueContainer::getInt16Value(int16 *returnValue) const {
	if (_objectType !=  INT16)
		warning("'Object' is not storing an int16.");

	*returnValue = _value.int16Val;
	return true;
}

bool SingleValueContainer::getUInt16Value(uint16 *returnValue) const {
	if (_objectType !=  UINT16)
		warning("'Object' is not storing a uint16.");

	*returnValue = _value.uint16Val;
	return true;
}

bool SingleValueContainer::getInt32Value(int32 *returnValue) const {
	if (_objectType !=  INT32)
		warning("'Object' is not storing an int32.");

	*returnValue = _value.int32Val;
	return true;
}

bool SingleValueContainer::getUInt32Value(uint32 *returnValue) const {
	if (_objectType !=  UINT32)
		warning("'Object' is not storing a uint32.");

	*returnValue = _value.uint32Val;
	return true;
}

bool SingleValueContainer::getFloatValue(float *returnValue) const {
	if (_objectType !=  FLOAT)
		warning("'Object' is not storing a float.");

	*returnValue = _value.floatVal;
	return true;
}

bool SingleValueContainer::getDoubleValue(double *returnValue) const {
	if (_objectType !=  DOUBLE)
		warning("'Object' is not storing a double.");

	*returnValue = _value.doubleVal;
	return true;
}

bool SingleValueContainer::getStringValue(Common::String *returnValue) const {
	if (_objectType !=  STRING)
		warning("'Object' is not storing a Common::String.");

	*returnValue = _value.stringVal;
	return true;
}

} // End of namespace ZVision
