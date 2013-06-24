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

#ifndef ZVISION_OBJECT_H
#define ZVISION_OBJECT_H

#include "common/str.h"

namespace ZVision {

/**
 * A generic single value storage class. It is useful for storing different 
 * value types in a single List, Hashmap, etc.
 *
 * Mimics C#'s Object class
 *
 * Caution: The actual value is stored on the heap, so be careful creating
 * many objects of this class. Also, when re-using 'Object', try to assign
 * a value of the same type as before, as this will prevent an extra memory allocation.
 */
class Object {
public:
	enum ObjectType {
		BOOL,
		BYTE,
		INT16,
		UINT16,
		INT32,
		UINT32,
		FLOAT,
		DOUBLE,
		STRING,
	};

	// Constructors
	Object(ObjectType type);
	Object(bool value);
	Object(byte value);
	Object(int16 value);
	Object(uint16 value);
	Object(int32 value);
	Object(uint32 value);
	Object(float value);
	Object(double value);
	Object(Common::String value);

	// Copy constructor
	Object(const Object& other);

	// Destructor
	~Object();

private:
	ObjectType _objectType;

	union {
		bool *boolVal;
		byte *byteVal;
		int16 *int16Val;
		uint16 *uint16Val;
		int32 *int32Val;
		uint32 *uint32Val;
		float *floatVal;
		double *doubleVal;
		Common::String *stringVal;
	} _value;

public:
	Object &operator=(const bool &rhs);
	Object &operator=(const byte &rhs);
	Object &operator=(const int16 &rhs);
	Object &operator=(const uint16 &rhs);
	Object &operator=(const int32 &rhs);
	Object &operator=(const uint32 &rhs);
	Object &operator=(const float &rhs);
	Object &operator=(const double &rhs);
	Object &operator=(const Common::String &rhs);

	Object& operator=(const Object &rhs);

	bool getBoolValue(bool *returnValue) const;
	bool getByteValue(byte *returnValue) const;
	bool getInt16Value(int16 *returnValue) const;
	bool getUInt16Value(uint16 *returnValue) const;
	bool getInt32Value(int32 *returnValue) const;
	bool getUInt32Value(uint32 *returnValue) const;
	bool getFloatValue(float *returnValue) const;
	bool getDoubleValue(double *returnValue) const;
	bool getStringValue(Common::String *returnValue) const;

private:
	/**
	 * Helper method for destruction and assignment. Calls delete on
	 * the currently used data pointer			
	 */
	void deleteValue();
};

} // End of namespace ZVision

#endif
