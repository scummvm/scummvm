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
	explicit Object(ObjectType type);
	explicit Object(bool value);
	explicit Object(byte value);
	explicit Object(int16 value);
	explicit Object(uint16 value);
	explicit Object(int32 value);
	explicit Object(uint32 value);
	explicit Object(float value);
	explicit Object(double value);
	explicit Object(Common::String value);

	// Copy constructor
	explicit Object(const Object& other);

	// Destructor
	~Object();

private:
	ObjectType _objectType;

	union {
		bool boolVal;
		byte byteVal;
		int16 int16Val;
		uint16 uint16Val;
		int32 int32Val;
		uint32 uint32Val;
		float floatVal;
		double doubleVal;
		char *stringVal;
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

	/**
	 * Retrieve a bool from the container. If the container is not storing a 
	 * bool, this will return false and display a warning().
	 *
	 * @param returnValue	Pointer to where you want the value stored
	 * @return				Value indicating whether the value assignment was successful
	 */
	bool getBoolValue(bool *returnValue) const;
	/**
	 * Retrieve a byte from the container. If the container is not storing a 
	 * byte, this will return false and display a warning().
	 *
	 * @param returnValue	Pointer to where you want the value stored
	 * @return				Value indicating whether the value assignment was successful
	 */
	bool getByteValue(byte *returnValue) const;
	/**
	 * Retrieve an int16 from the container. If the container is not storing an 
	 * int16, this will return false and display a warning().
	 *
	 * @param returnValue	Pointer to where you want the value stored
	 * @return				Value indicating whether the value assignment was successful
	 */
	bool getInt16Value(int16 *returnValue) const;
	/**
	 * Retrieve a uint16 from the container. If the container is not storing a 
	 * uint16, this will return false and display a warning().
	 *
	 * @param returnValue	Pointer to where you want the value stored
	 * @return				Value indicating whether the value assignment was successful
	 */
	bool getUInt16Value(uint16 *returnValue) const;
	/**
	 * Retrieve an int32 from the container. If the container is not storing an 
	 * int32, this will return false and display a warning().
	 *
	 * @param returnValue	Pointer to where you want the value stored
	 * @return				Value indicating whether the value assignment was successful
	 */
	bool getInt32Value(int32 *returnValue) const;
	/**
	 * Retrieve a uint32 from the container. If the container is not storing a 
	 * uint32, this will return false and display a warning().
	 *
	 * @param returnValue	Pointer to where you want the value stored
	 * @return				Value indicating whether the value assignment was successful
	 */
	bool getUInt32Value(uint32 *returnValue) const;
	/**
	 * Retrieve a float from the container. If the container is not storing a 
	 * float, this will return false and display a warning().
	 *
	 * @param returnValue	Pointer to where you want the value stored
	 * @return				Value indicating whether the value assignment was successful
	 */
	bool getFloatValue(float *returnValue) const;
	/**
	 * Retrieve a double from the container. If the container is not storing a 
	 * double, this will return false and display a warning().
	 *
	 * @param returnValue	Pointer to where you want the value stored
	 * @return				Value indicating whether the value assignment was successful
	 */
	bool getDoubleValue(double *returnValue) const;
	/**
	 * Retrieve a String from the container. If the container is not storing a 
	 * string, this will return false and display a warning().
	 * 
	 * Caution: Strings are internally stored as char[]. getStringValue uses
	 * Common::String::operator=(char *) to do the assigment, which uses both
	 * strlen() AND memmove().
	 *
	 * @param returnValue	Pointer to where you want the value stored
	 * @return				Value indicating whether the value assignment was successful
	 */
	bool getStringValue(Common::String *returnValue) const;

private:
	/**
	 * Helper method for destruction and assignment. It checks to see
	 * if the char pointer is being used, and if so calls delete on it
	 */
	void deleteCharPointer();
};

} // End of namespace ZVision

#endif
