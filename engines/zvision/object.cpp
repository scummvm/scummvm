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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/textconsole.h"

#include "zvision/object.h"

namespace ZVision {

Object::Object(ObjectType type) {
	_objectType = type;

	switch (type) {
	case BOOL:
		_value.boolVal = new bool;
		break;
	case BYTE:
		_value.byteVal = new byte;
		break;
	case INT16:
		_value.int16Val = new int16;
		break;
	case UINT16:
		_value.uint16Val = new uint16;
		break;
	case INT32:
		_value.int32Val = new int32;
		break;
	case UINT32:
		_value.uint32Val = new uint32;
		break;
	case FLOAT:
		_value.floatVal = new float;
		break;
	case DOUBLE:
		_value.doubleVal = new double;
		break;
	case STRING:
		_value.stringVal = new Common::String;
		break;
	}
}

Object::Object(bool value) : _objectType(BOOL) {
	_value.boolVal = new bool(value);
}

Object::Object(byte value) : _objectType(BYTE) {
	_value.byteVal = new byte(value);
}

Object::Object(int16 value) : _objectType(INT16) {
	_value.int16Val = new int16(value);
}

Object::Object(uint16 value) : _objectType(UINT16) {
	_value.uint16Val = new uint16(value);
}

Object::Object(int32 value) : _objectType(INT32) {
	_value.int32Val = new int32(value);
}

Object::Object(uint32 value) : _objectType(UINT32) {
	_value.uint32Val = new uint32(value);
}

Object::Object(float value) : _objectType(FLOAT) {
	_value.floatVal = new float(value);
}

Object::Object(double value) : _objectType(DOUBLE) {
	_value.doubleVal = new double(value);
}

Object::Object(Common::String value) : _objectType(BYTE) {
	_value.stringVal = new Common::String(value);
}

Object::Object(const Object &other) {
	_objectType = other._objectType;

	switch (_objectType) {
	case BOOL:
		_value.boolVal = new bool(*other._value.boolVal);
		break;
	case BYTE:
		_value.byteVal = new byte(*other._value.byteVal);
		break;
	case INT16:
		_value.int16Val = new int16(*other._value.int16Val);
		break;
	case UINT16:
		_value.uint16Val = new uint16(*other._value.uint16Val);
		break;
	case INT32:
		_value.int32Val = new int32(*other._value.int32Val);
		break;
	case UINT32:
		_value.uint32Val = new uint32(*other._value.uint32Val);
		break;
	case FLOAT:
		_value.floatVal = new float(*other._value.floatVal);
		break;
	case DOUBLE:
		_value.doubleVal = new double(*other._value.doubleVal);
		break;
	case STRING:
		_value.stringVal = new Common::String(*other._value.stringVal);
		break;
	}
}

Object::~Object() {
	deleteValue();
}

void Object::deleteValue() {
	// Call delete on the correct part of the union.
	// Even though they all point to the same memory and will all be cast 
	// to a void *, this can still cause undefined behavior.
	switch (_objectType) {
	case BOOL:
		delete _value.boolVal;
		break;
	case BYTE:
		delete _value.byteVal;
		break;
	case INT16:
		delete _value.int16Val;
		break;
	case UINT16:
		delete _value.uint16Val;
		break;
	case INT32:
		delete _value.int32Val;
		break;
	case UINT32:
		delete _value.uint32Val;
		break;
	case FLOAT:
		delete _value.floatVal;
		break;
	case DOUBLE:
		delete _value.doubleVal;
		break;
	case STRING:
		delete _value.stringVal;
		break;
	}
}


Object &Object::operator=(const bool &rhs) {
	if (_objectType == BOOL)
		*_value.boolVal = rhs;
	else {
		deleteValue();
		_objectType = BOOL;
		_value.boolVal = new bool(rhs);
	}

	return *this;
}

Object &Object::operator=(const byte &rhs) {
	if (_objectType == BYTE)
		*_value.byteVal = rhs;
	else {
		deleteValue();
		_objectType = BYTE;
		_value.byteVal = new byte(rhs);
	}

	return *this;
}

Object &Object::operator=(const int16 &rhs) {
	if (_objectType == INT16)
		*_value.int16Val = rhs;
	else {
		deleteValue();
		_objectType = INT16;
		_value.int16Val = new int16(rhs);
	}

	return *this;
}

Object &Object::operator=(const uint16 &rhs) {
	if (_objectType == UINT16)
		*_value.uint16Val = rhs;
	else {
		deleteValue();
		_objectType = UINT16;
		_value.uint16Val = new uint16(rhs);
	}

	return *this;
}

Object &Object::operator=(const int32 &rhs) {
	if (_objectType == INT32)
		*_value.int32Val = rhs;
	else {
		deleteValue();
		_objectType = INT32;
		_value.int32Val = new int32(rhs);
	}

	return *this;
}

Object &Object::operator=(const uint32 &rhs) {
	if (_objectType == UINT32)
		*_value.uint32Val = rhs;
	else {
		deleteValue();
		_objectType = UINT32;
		_value.uint32Val = new uint32(rhs);
	}

	return *this;
}

Object &Object::operator=(const float &rhs) {
	if (_objectType == FLOAT)
		*_value.floatVal = rhs;
	else {
		deleteValue();
		_objectType = FLOAT;
		_value.floatVal = new float(rhs);
	}

	return *this;
}

Object &Object::operator=(const double &rhs) {
	if (_objectType == DOUBLE)
		*_value.doubleVal = rhs;
	else {
		deleteValue();
		_objectType = DOUBLE;
		_value.doubleVal = new double(rhs);
	}

	return *this;
}

Object &Object::operator=(const Common::String &rhs) {
	if (_objectType == STRING)
		*_value.stringVal = rhs;
	else {
		deleteValue();
		_objectType = STRING;
		_value.stringVal = new Common::String(rhs);
	}

	return *this;
}

Object &Object::operator=(const Object &rhs) {
	switch (_objectType) {
	case BOOL:
		return operator=(*rhs._value.boolVal);
	case BYTE:
		return operator=(*rhs._value.byteVal);
	case INT16:
		return operator=(*rhs._value.int16Val);
	case UINT16:
		return operator=(*rhs._value.uint16Val);
	case INT32:
		return operator=(*rhs._value.int32Val);
	case UINT32:
		return operator=(*rhs._value.uint32Val);
	case FLOAT:
		return operator=(*rhs._value.floatVal);
	case DOUBLE:
		return operator=(*rhs._value.doubleVal);
	case STRING:
		return operator=(*rhs._value.stringVal);
	}

	return *this;
}


bool Object::getBoolValue(bool *returnValue) const {
	if (_objectType !=  BOOL) {
		warning("'Object' not of type bool.");
		return false;
	}

	*returnValue = *_value.boolVal;
	return true;
}

bool Object::getByteValue(byte *returnValue) const {
	if (_objectType !=  BYTE)
		warning("'Object' not of type byte.");

	*returnValue = *_value.byteVal;
	return true;
}

bool Object::getInt16Value(int16 *returnValue) const {
	if (_objectType !=  INT16)
		warning("'Object' not of type int16.");

	*returnValue = *_value.int16Val;
	return true;
}

bool Object::getUInt16Value(uint16 *returnValue) const {
	if (_objectType !=  UINT16)
		warning("'Object' not of type uint16.");

	*returnValue = *_value.uint16Val;
	return true;
}

bool Object::getInt32Value(int32 *returnValue) const {
	if (_objectType !=  INT32)
		warning("'Object' not of type int32.");

	*returnValue = *_value.int32Val;
	return true;
}

bool Object::getUInt32Value(uint32 *returnValue) const {
	if (_objectType !=  UINT32)
		warning("'Object' not of type uint32.");

	*returnValue = *_value.uint32Val;
	return true;
}

bool Object::getFloatValue(float *returnValue) const {
	if (_objectType !=  FLOAT)
		warning("'Object' not of type float.");

	*returnValue = *_value.floatVal;
	return true;
}

bool Object::getDoubleValue(double *returnValue) const {
	if (_objectType !=  DOUBLE)
		warning("'Object' not of type double.");

	*returnValue = *_value.doubleVal;
	return true;
}

bool Object::getStringValue(Common::String *returnValue) const {
	if (_objectType !=  STRING)
		warning("'Object' not of type Common::String.");

	*returnValue = *_value.stringVal;
	return true;
}

} // End of namespace ZVision
