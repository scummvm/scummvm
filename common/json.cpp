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

/*
 * Files JSON.cpp and JSONValue.cpp part of the SimpleJSON Library - https://github.com/MJPA/SimpleJSON
 *
 * Copyright (C) 2010 Mike Anchor
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "common/json.h"

#ifdef __MINGW32__
#define wcsncasecmp wcsnicmp
#endif

// Macros to free an array/object
#define FREE_ARRAY(x) { JSONArray::iterator iter; for (iter = x.begin(); iter != x.end(); iter++) { delete *iter; } }
#define FREE_OBJECT(x) { JSONObject::iterator iter; for (iter = x.begin(); iter != x.end(); iter++) { delete (*iter)._value; } }

namespace Common {

/**
* Blocks off the public constructor
*
* @access private
*
*/
JSON::JSON() {}

/**
* Parses a complete JSON encoded string (UNICODE input version)
*
* @access public
*
* @param char* data The JSON text
*
* @return JSONValue* Returns a JSON Value representing the root, or NULL on error
*/
JSONValue *JSON::parse(const char *data) {
	// Skip any preceding whitespace, end of data = no JSON = fail
	if (!skipWhitespace(&data))
		return nullptr;

	// We need the start of a value here now...
	JSONValue *value = JSONValue::parse(&data);
	if (value == nullptr)
		return nullptr;

	// Can be white space now and should be at the end of the string then...
	if (skipWhitespace(&data)) {
		delete value;
		return nullptr;
	}

	// We're now at the end of the string
	return value;
}

/**
* Turns the passed in JSONValue into a JSON encode string
*
* @access public
*
* @param JSONValue* value The root value
*
* @return String Returns a JSON encoded string representation of the given value
*/
String JSON::stringify(const JSONValue *value) {
	if (value != nullptr)
		return value->stringify();
	else
		return "";
}

/**
* Skips over any whitespace characters (space, tab, \r or \n) defined by the JSON spec
*
* @access protected
*
* @param char** data Pointer to a char* that contains the JSON text
*
* @return bool Returns true if there is more data, or false if the end of the text was reached
*/
bool JSON::skipWhitespace(const char **data) {
	while (**data != 0 && (**data == ' ' || **data == '\t' || **data == '\r' || **data == '\n'))
		(*data)++;

	return **data != 0;
}

/**
* Extracts a JSON String as defined by the spec - "<some chars>"
* Any escaped characters are swapped out for their unescaped values
*
* @access protected
*
* @param char** data Pointer to a char* that contains the JSON text
* @param String& str Reference to a String to receive the extracted string
*
* @return bool Returns true on success, false on failure
*/
bool JSON::extractString(const char **data, String &str) {
	str = "";

	while (**data != 0) {
		// Save the char so we can change it if need be
		char next_char = **data;
		uint32 next_uchar = 0;

		// Escaping something?
		if (next_char == '\\') {
			// Move over the escape char
			(*data)++;

			// Deal with the escaped char
			switch (**data) {
			case '"': next_char = '"';
				break;
			case '\\': next_char = '\\';
				break;
			case '/': next_char = '/';
				break;
			case 'b': next_char = '\b';
				break;
			case 'f': next_char = '\f';
				break;
			case 'n': next_char = '\n';
				break;
			case 'r': next_char = '\r';
				break;
			case 't': next_char = '\t';
				break;
			case 'u': {
				next_char = 0;
				next_uchar = parseUnicode(data);
				// If the codepoint is a high surrogate, we should have a low surrogate now
				if (next_uchar >= 0xD800 && next_uchar <= 0xDBFF) {
					(*data)++;
					if (**data != '\\')
						return false;
					(*data)++;
					uint32 low_surrogate = parseUnicode(data);
					if (low_surrogate < 0xDC00 || low_surrogate > 0xDFFF)
						return false;
					//next_uchar = 0x10000 + (next_uchar - 0xD800) * 0x400 + (low_surrogate - 0xDC00);
					next_uchar = (next_uchar << 10) + low_surrogate - 0x35FDC00u;
				} else if (next_uchar >= 0xDC00 && next_uchar <= 0xDFFF)
					return false; // low surrogate, which should only follow a high surrogate
				// Check this is a valid code point
				if (next_uchar > 0x10FFFF)
					return false;
				break;
			}

				// By the spec, only the above cases are allowed
			default:
				return false;
			}
		}

		// End of the string?
		else if (next_char == '"') {
			(*data)++;
			//str.reserve(); // Remove unused capacity //TODO
			return true;
		}

		// Disallowed char?
		else if (next_char < ' ' && next_char != '\t') {
			// SPEC Violation: Allow tabs due to real world cases
			return false;
		}

		// Add the next char
		if (next_char != 0)
			str += next_char;
		else {
			if (next_uchar < 0x80)
				// 1-byte character (ASCII)
				str += (char)next_uchar;
			else if (next_uchar <= 0x7FF) {
				// 2-byte characters: 110xxxxx 10xxxxxx
				str += (char)(0xC0 | (next_uchar >> 6));
				str += (char)(0x80 | (next_uchar & 0x3F));
			} else if (next_uchar <= 0xFFFF) {
				// 3-byte characters: 1110xxxx 10xxxxxx 10xxxxxx
				str += (char)(0xE0 | (next_uchar >> 12));
				str += (char)(0x80 | ((next_uchar >> 6) & 0x3F));
				str += (char)(0x80 | (next_uchar & 0x3F));
			} else {
				// 4-byte characters: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
				str += (char)(0xF0 | (next_uchar >> 18));
				str += (char)(0x80 | ((next_uchar >> 12) & 0x3F));
				str += (char)(0x80 | ((next_uchar >> 6) & 0x3F));
				str += (char)(0x80 | (next_uchar & 0x3F));
			}
		}

		// Move on
		(*data)++;
	}

	// If we're here, the string ended incorrectly
	return false;
}

/**
* Parses some text as though it is a unicode hexadecimal sequence.
* It assumes that the data is currently pointing on the 'u' part of '\uXXXX`.
*
* @access protected
*
* @param char** data Pointer to a char* that contains the JSON text
* @param String& str Reference to a String to receive the extracted string
*
* @return uint32 Returns the unicode code point value or 0xFFFFFFFF in case of error.
*/
uint32 JSON::parseUnicode(const char **data) {
	if (**data != 'u')
		return 0xFFFFFFFF;
	// We need 5 chars (4 hex + the 'u') or its not valid
	if (!simplejson_wcsnlen(*data, 5))
		return 0xFFFFFFFF;

	// Deal with the chars
	uint32 codepoint = 0;
	for (int i = 0; i < 4; i++) {
		// Do it first to move off the 'u' and leave us on the
		// final hex digit as we move on by one later on
		(*data)++;

		codepoint <<= 4;

		// Parse the hex digit
		if (**data >= '0' && **data <= '9')
			codepoint |= (**data - '0');
		else if (**data >= 'A' && **data <= 'F')
			codepoint |= (10 + (**data - 'A'));
		else if (**data >= 'a' && **data <= 'f')
			codepoint |= (10 + (**data - 'a'));
		else {
			// Invalid hex digit
			return 0xFFFFFFFF;
		}
	}
	return codepoint;
}

/**
* Parses some text as though it is an integer
*
* @access protected
*
* @param char** data Pointer to a char* that contains the JSON text
*
* @return double Returns the double value of the number found
*/
double JSON::parseInt(const char **data) {
	double integer = 0;
	while (**data != 0 && **data >= '0' && **data <= '9')
		integer = integer * 10 + (*(*data)++ - '0');

	return integer;
}

/**
* Parses some text as though it is a decimal
*
* @access protected
*
* @param char** data Pointer to a char* that contains the JSON text
*
* @return double Returns the double value of the decimal found
*/
double JSON::parseDecimal(const char **data) {
	double decimal = 0.0;
	double factor = 0.1;
	while (**data != 0 && **data >= '0' && **data <= '9') {
		int digit = (*(*data)++ - '0');
		decimal = decimal + digit * factor;
		factor *= 0.1;
	}
	return decimal;
}

/**
* Parses a JSON encoded value to a JSONValue object
*
* @access protected
*
* @param char** data Pointer to a char* that contains the data
*
* @return JSONValue* Returns a pointer to a JSONValue object on success, NULL on error
*/
JSONValue *JSONValue::parse(const char **data) {
	// Is it a string?
	if (**data == '"') {
		String str;
		if (!JSON::extractString(&(++(*data)), str))
			return nullptr;
		else
			return new JSONValue(str);
	}

	// Is it a boolean?
	else if ((simplejson_wcsnlen(*data, 4) && scumm_strnicmp(*data, "true", 4) == 0) || (simplejson_wcsnlen(*data, 5) && scumm_strnicmp(*data, "false", 5) == 0)) {
		bool value = scumm_strnicmp(*data, "true", 4) == 0;
		(*data) += value ? 4 : 5;
		return new JSONValue(value);
	}

	// Is it a null?
	else if (simplejson_wcsnlen(*data, 4) && scumm_strnicmp(*data, "null", 4) == 0) {
		(*data) += 4;
		return new JSONValue();
	}

	// Is it a number?
	else if (**data == '-' || (**data >= '0' && **data <= '9')) {
		// Negative?
		bool neg = **data == '-';
		if (neg) (*data)++;

		long long int integer = 0;
		double number = 0.0;
		bool onlyInteger = true;

		// Parse the whole part of the number - only if it wasn't 0
		if (**data == '0')
			(*data)++;
		else if (**data >= '1' && **data <= '9')
			number = integer = JSON::parseInt(data);
		else
			return nullptr;

		// Could be a decimal now...
		if (**data == '.') {
			(*data)++;

			// Not get any digits?
			if (!(**data >= '0' && **data <= '9'))
				return nullptr;

			// Find the decimal and sort the decimal place out
			// Use ParseDecimal as ParseInt won't work with decimals less than 0.1
			// thanks to Javier Abadia for the report & fix
			double decimal = JSON::parseDecimal(data);

			// Save the number
			number += decimal;
			onlyInteger = false;
		}

		// Could be an exponent now...
		if (**data == 'E' || **data == 'e') {
			(*data)++;

			// Check signage of expo
			bool neg_expo = false;
			if (**data == '-' || **data == '+') {
				neg_expo = **data == '-';
				(*data)++;
			}

			// Not get any digits?
			if (!(**data >= '0' && **data <= '9'))
				return nullptr;

			// Sort the expo out
			double expo = JSON::parseInt(data);
			for (double i = 0.0; i < expo; i++)
				number = neg_expo ? (number / 10.0) : (number * 10.0);
			onlyInteger = false;
		}

		// Was it neg?
		if (neg) number *= -1;

		if (onlyInteger)
			return new JSONValue(neg ? -integer : integer);

		return new JSONValue(number);
	}

	// An object?
	else if (**data == '{') {
		JSONObject object;

		(*data)++;

		while (**data != 0) {
			// Whitespace at the start?
			if (!JSON::skipWhitespace(data)) {
				FREE_OBJECT(object);
				return nullptr;
			}

			// Special case - empty object
			if (object.size() == 0 && **data == '}') {
				(*data)++;
				return new JSONValue(object);
			}

			// We want a string now...
			String name;
			if (!JSON::extractString(&(++(*data)), name)) {
				FREE_OBJECT(object);
				return nullptr;
			}

			// More whitespace?
			if (!JSON::skipWhitespace(data)) {
				FREE_OBJECT(object);
				return nullptr;
			}

			// Need a : now
			if (*((*data)++) != ':') {
				FREE_OBJECT(object);
				return nullptr;
			}

			// More whitespace?
			if (!JSON::skipWhitespace(data)) {
				FREE_OBJECT(object);
				return nullptr;
			}

			// The value is here
			JSONValue *value = parse(data);
			if (value == nullptr) {
				FREE_OBJECT(object);
				return nullptr;
			}

			// Add the name:value
			if (object.find(name) != object.end())
				delete object[name];
			object[name] = value;

			// More whitespace?
			if (!JSON::skipWhitespace(data)) {
				FREE_OBJECT(object);
				return nullptr;
			}

			// End of object?
			if (**data == '}') {
				(*data)++;
				return new JSONValue(object);
			}

			// Want a , now
			if (**data != ',') {
				FREE_OBJECT(object);
				return nullptr;
			}

			(*data)++;
		}

		// Only here if we ran out of data
		FREE_OBJECT(object);
		return nullptr;
	}

	// An array?
	else if (**data == '[') {
		JSONArray array;

		(*data)++;

		while (**data != 0) {
			// Whitespace at the start?
			if (!JSON::skipWhitespace(data)) {
				FREE_ARRAY(array);
				return nullptr;
			}

			// Special case - empty array
			if (array.size() == 0 && **data == ']') {
				(*data)++;
				return new JSONValue(array);
			}

			// Get the value
			JSONValue *value = parse(data);
			if (value == nullptr) {
				FREE_ARRAY(array);
				return nullptr;
			}

			// Add the value
			array.push_back(value);

			// More whitespace?
			if (!JSON::skipWhitespace(data)) {
				FREE_ARRAY(array);
				return nullptr;
			}

			// End of array?
			if (**data == ']') {
				(*data)++;
				return new JSONValue(array);
			}

			// Want a , now
			if (**data != ',') {
				FREE_ARRAY(array);
				return nullptr;
			}

			(*data)++;
		}

		// Only here if we ran out of data
		FREE_ARRAY(array);
		return nullptr;
	}

	// Ran out of possibilites, it's bad!
	else {
		return nullptr;
	}
}

/**
* Basic constructor for creating a JSON Value of type NULL
*
* @access public
*/
JSONValue::JSONValue(/*NULL*/) {
	_type = JSONType_Null;
}

/**
* Basic constructor for creating a JSON Value of type String
*
* @access public
*
* @param char* m_char_value The string to use as the value
*/
JSONValue::JSONValue(const char *charValue) {
	_type = JSONType_String;
	_stringValue = new String(String(charValue));
}

/**
* Basic constructor for creating a JSON Value of type String
*
* @access public
*
* @param String m_string_value The string to use as the value
*/
JSONValue::JSONValue(const String &stringValue) {
	_type = JSONType_String;
	_stringValue = new String(stringValue);
}

/**
* Basic constructor for creating a JSON Value of type Bool
*
* @access public
*
* @param bool m_bool_value The bool to use as the value
*/
JSONValue::JSONValue(bool boolValue) {
	_type = JSONType_Bool;
	_boolValue = boolValue;
}

/**
* Basic constructor for creating a JSON Value of type Number
*
* @access public
*
* @param double m_number_value The number to use as the value
*/
JSONValue::JSONValue(double numberValue) {
	_type = JSONType_Number;
	_numberValue = numberValue;
}

/**
* Basic constructor for creating a JSON Value of type Number (Integer)
*
* @access public
*
* @param int numberValue The number to use as the value
*/
JSONValue::JSONValue(long long int numberValue) {
	_type = JSONType_IntegerNumber;
	_integerValue = numberValue;
}

/**
* Basic constructor for creating a JSON Value of type Array
*
* @access public
*
* @param JSONArray m_array_value The JSONArray to use as the value
*/
JSONValue::JSONValue(const JSONArray &arrayValue) {
	_type = JSONType_Array;
	_arrayValue = new JSONArray(arrayValue);
}

/**
* Basic constructor for creating a JSON Value of type Object
*
* @access public
*
* @param JSONObject m_object_value The JSONObject to use as the value
*/
JSONValue::JSONValue(const JSONObject &objectValue) {
	_type = JSONType_Object;
	_objectValue = new JSONObject(objectValue);
}

/**
* Copy constructor to perform a deep copy of array / object values
*
* @access public
*
* @param JSONValue m_source The source JSONValue that is being copied
*/
JSONValue::JSONValue(const JSONValue &source) {
	_type = source._type;

	switch (_type) {
	case JSONType_String:
		_stringValue = new String(*source._stringValue);
		break;

	case JSONType_Bool:
		_boolValue = source._boolValue;
		break;

	case JSONType_Number:
		_numberValue = source._numberValue;
		break;

	case JSONType_IntegerNumber:
		_integerValue = source._integerValue;
		break;

	case JSONType_Array: {
		JSONArray source_array = *source._arrayValue;
		JSONArray::iterator iter;
		_arrayValue = new JSONArray();
		for (iter = source_array.begin(); iter != source_array.end(); iter++)
			_arrayValue->push_back(new JSONValue(**iter));
		break;
	}

	case JSONType_Object: {
		JSONObject source_object = *source._objectValue;
		_objectValue = new JSONObject();
		JSONObject::iterator iter;
		for (iter = source_object.begin(); iter != source_object.end(); iter++) {
			String name = (*iter)._key;
			(*_objectValue)[name] = new JSONValue(*((*iter)._value));
		}
		break;
	}

	default:
		// fallthrough intended
	case JSONType_Null:
		// Nothing to do.
		break;
	}
}

/**
* The destructor for the JSON Value object
* Handles deleting the objects in the array or the object value
*
* @access public
*/
JSONValue::~JSONValue() {
	if (_type == JSONType_Array) {
		JSONArray::iterator iter;
		for (iter = _arrayValue->begin(); iter != _arrayValue->end(); iter++)
			delete *iter;
		delete _arrayValue;
	} else if (_type == JSONType_Object) {
		JSONObject::iterator iter;
		for (iter = _objectValue->begin(); iter != _objectValue->end(); iter++) {
			delete (*iter)._value;
		}
		delete _objectValue;
	} else if (_type == JSONType_String) {
		delete _stringValue;
	}
}

/**
* Checks if the value is a NULL
*
* @access public
*
* @return bool Returns true if it is a NULL value, false otherwise
*/
bool JSONValue::isNull() const {
	return _type == JSONType_Null;
}

/**
* Checks if the value is a String
*
* @access public
*
* @return bool Returns true if it is a String value, false otherwise
*/
bool JSONValue::isString() const {
	return _type == JSONType_String;
}

/**
* Checks if the value is a Bool
*
* @access public
*
* @return bool Returns true if it is a Bool value, false otherwise
*/
bool JSONValue::isBool() const {
	return _type == JSONType_Bool;
}

/**
* Checks if the value is a Number
*
* @access public
*
* @return bool Returns true if it is a Number value, false otherwise
*/
bool JSONValue::isNumber() const {
	return _type == JSONType_Number;
}

/**
* Checks if the value is an Integer
*
* @access public
*
* @return bool Returns true if it is an Integer value, false otherwise
*/
bool JSONValue::isIntegerNumber() const {
	return _type == JSONType_IntegerNumber;
}

/**
* Checks if the value is an Array
*
* @access public
*
* @return bool Returns true if it is an Array value, false otherwise
*/
bool JSONValue::isArray() const {
	return _type == JSONType_Array;
}

/**
* Checks if the value is an Object
*
* @access public
*
* @return bool Returns true if it is an Object value, false otherwise
*/
bool JSONValue::isObject() const {
	return _type == JSONType_Object;
}

/**
* Retrieves the String value of this JSONValue
* Use isString() before using this method.
*
* @access public
*
* @return String Returns the string value
*/
const String &JSONValue::asString() const {
	return (*_stringValue);
}

/**
* Retrieves the Bool value of this JSONValue
* Use isBool() before using this method.
*
* @access public
*
* @return bool Returns the bool value
*/
bool JSONValue::asBool() const {
	return _boolValue;
}

/**
* Retrieves the Number value of this JSONValue
* Use isNumber() before using this method.
*
* @access public
*
* @return double Returns the number value
*/
double JSONValue::asNumber() const {
	return _numberValue;
}

/**
* Retrieves the Integer value of this JSONValue
* Use isIntegerNumber() before using this method.
*
* @access public
*
* @return int Returns the number value
*/
long long int JSONValue::asIntegerNumber() const {
	return _integerValue;
}

/**
* Retrieves the Array value of this JSONValue
* Use isArray() before using this method.
*
* @access public
*
* @return JSONArray Returns the array value
*/
const JSONArray &JSONValue::asArray() const {
	return (*_arrayValue);
}

/**
* Retrieves the Object value of this JSONValue
* Use isObject() before using this method.
*
* @access public
*
* @return JSONObject Returns the object value
*/
const JSONObject &JSONValue::asObject() const {
	return (*_objectValue);
}

/**
* Retrieves the number of children of this JSONValue.
* This number will be 0 or the actual number of children
* if isArray() or isObject().
*
* @access public
*
* @return The number of children.
*/
std::size_t JSONValue::countChildren() const {
	switch (_type) {
	case JSONType_Array:
		return _arrayValue->size();
	case JSONType_Object:
		return _objectValue->size();
	default:
		return 0;
	}
}

/**
* Checks if this JSONValue has a child at the given index.
* Use isArray() before using this method.
*
* @access public
*
* @return bool Returns true if the array has a value at the given index.
*/
bool JSONValue::hasChild(std::size_t index) const {
	if (_type == JSONType_Array) {
		return index < _arrayValue->size();
	} else {
		return false;
	}
}

/**
* Retrieves the child of this JSONValue at the given index.
* Use isArray() before using this method.
*
* @access public
*
* @return JSONValue* Returns JSONValue at the given index or NULL
*                    if it doesn't exist.
*/
JSONValue *JSONValue::child(std::size_t index) {
	if (index < _arrayValue->size()) {
		return (*_arrayValue)[index];
	} else {
		return nullptr;
	}
}

/**
* Checks if this JSONValue has a child at the given key.
* Use isObject() before using this method.
*
* @access public
*
* @return bool Returns true if the object has a value at the given key.
*/
bool JSONValue::hasChild(const char *name) const {
	if (_type == JSONType_Object) {
		return _objectValue->find(name) != _objectValue->end();
	} else {
		return false;
	}
}

/**
* Retrieves the child of this JSONValue at the given key.
* Use isObject() before using this method.
*
* @access public
*
* @return JSONValue* Returns JSONValue for the given key in the object
*                    or NULL if it doesn't exist.
*/
JSONValue *JSONValue::child(const char *name) {
	JSONObject::const_iterator it = _objectValue->find(name);
	if (it != _objectValue->end()) {
		return it->_value;
	} else {
		return nullptr;
	}
}

/**
* Retrieves the keys of the JSON Object or an empty vector
* if this value is not an object.
*
* @access public
*
* @return std::vector<String> A vector containing the keys.
*/
Array<String> JSONValue::objectKeys() const {
	Array<String> keys;

	if (_type == JSONType_Object) {
		JSONObject::const_iterator iter = _objectValue->begin();
		while (iter != _objectValue->end()) {
			keys.push_back(iter->_key);

			iter++;
		}
	}

	return keys;
}

/**
* Creates a JSON encoded string for the value with all necessary characters escaped
*
* @access public
*
* @param bool prettyprint Enable prettyprint
*
* @return String Returns the JSON string
*/
String JSONValue::stringify(bool const prettyprint) const {
	size_t const indentDepth = prettyprint ? 1 : 0;
	return stringifyImpl(indentDepth);
}


/**
* Creates a JSON encoded string for the value with all necessary characters escaped
*
* @access private
*
* @param size_t indentDepth The prettyprint indentation depth (0 : no prettyprint)
*
* @return String Returns the JSON string
*/
String JSONValue::stringifyImpl(size_t const indentDepth) const {
	String ret_string;
	size_t const indentDepth1 = indentDepth ? indentDepth + 1 : 0;
	String const indentStr = indent(indentDepth);
	String const indentStr1 = indent(indentDepth1);

	switch (_type) {
	default:
		// fallthrough intended
	case JSONType_Null:
		ret_string = "null";
		break;

	case JSONType_String:
		ret_string = stringifyString(*_stringValue);
		break;

	case JSONType_Bool:
		ret_string = _boolValue ? "true" : "false";
		break;

	case JSONType_Number: {
		if (isinf(_numberValue) || isnan(_numberValue))
			ret_string = "null";
		else {
			ret_string = String::format("%g", _numberValue);
		}
		break;
	}

	case JSONType_IntegerNumber: {
		ret_string = String::format("%lld", _integerValue);
		break;
	}

	case JSONType_Array: {
		ret_string = indentDepth ? "[\n" + indentStr1 : "[";
		JSONArray::const_iterator iter = _arrayValue->begin();
		while (iter != _arrayValue->end()) {
			ret_string += (*iter)->stringifyImpl(indentDepth1);

			// Not at the end - add a separator
			if (++iter != _arrayValue->end())
				ret_string += ",";
		}
		ret_string += indentDepth ? "\n" + indentStr + "]" : "]";
		break;
	}

	case JSONType_Object: {
		ret_string = indentDepth ? "{\n" + indentStr1 : "{";
		JSONObject::const_iterator iter = _objectValue->begin();
		while (iter != _objectValue->end()) {
			ret_string += stringifyString((*iter)._key);
			ret_string += ":";
			ret_string += (*iter)._value->stringifyImpl(indentDepth1);

			// Not at the end - add a separator
			if (++iter != _objectValue->end())
				ret_string += ",";
		}
		ret_string += indentDepth ? "\n" + indentStr + "}" : "}";
		break;
	}
	}

	return ret_string;
}

/**
* Creates a JSON encoded string with all required fields escaped
* Works from http://www.ecma-internationl.org/publications/files/ECMA-ST/ECMA-262.pdf
* Section 15.12.3.
*
* @access private
*
* @param String str The string that needs to have the characters escaped
*
* @return String Returns the JSON string
*/
String JSONValue::stringifyString(const String &str) {
	String str_out = "\"";

	String::const_iterator iter = str.begin();
	while (iter != str.end()) {
		uint32 uchr = decodeUtf8Char(iter, str.end());
		if (uchr == 0xFFFFFFFF)
			break; // error - truncate the result

		if (uchr == '"' || uchr == '\\' || uchr == '/') {
			str_out += '\\';
			str_out += (char)uchr;
		} else if (uchr == '\b') {
			str_out += "\\b";
		} else if (uchr == '\f') {
			str_out += "\\f";
		} else if (uchr == '\n') {
			str_out += "\\n";
		} else if (uchr == '\r') {
			str_out += "\\r";
		} else if (uchr == '\t') {
			str_out += "\\t";
		} else if (uchr >= ' ' && uchr <= 126 ) {
			str_out += (char)uchr;
		} else {
			if (uchr <= 0xFFFF)
				str_out += String::format("\\u%04x", uchr);
			else
				str_out += String::format("\\u%04x\\u%04x", 0xD7C0 + (uchr >> 10), 0xDC00 + (uchr & 0x3FF));
		}

		iter++;
	}

	str_out += "\"";
	return str_out;
}

/**
* Decode the next utf-8 character in the String pointed to by begin.
*
* @param String::const_iterator &iter Iterator pointing to the start of the character to decode.
*
* @param const String::const_iterator &end Iterator pointing past the end of the string being decoded.
*
* @return The codepoint value for the next utf-8 character starting at the current iterator position,
* or 0xFFFFFFFF in case of error.
*/
uint32 JSONValue::decodeUtf8Char(String::const_iterator &iter, const String::const_iterator &end) {
	uint8 state = 0;
	uint32 codepoint = 0;
	int nbRead = 0;
	do {
		uint8 byte = uint8(*iter);
		state = decodeUtf8Byte(state, codepoint, byte);
		++nbRead;
		if (state == 0)
			return codepoint;
	} while (state != 1 && ++iter != end);
	if (state == 1) {
		// We failed to read this as a UTF-8 character. The string might be encoded differently, which
		// would be invalid (since the json standard indicate the string has to be in utf-8) but rather
		// that return 0FFFFFFFF and truncate, try to recover from it by rewinding and returning the
		// raw byte.
		while (--nbRead > 0) { --iter; }
		uint8 byte = uint8(*iter);
		warning("Invalid UTF-8 character 0x%x in JSON string.", byte);
		return byte;
	}
	return 0xFFFFFFFF;
}

/**
* Decode one byte from a UTF-8 string.
*
* The function must initially (for the first byte) be called with a state of 0, and then
* with the state from the previous byte until it returns 0 (success) or 1 (failure).
*
* Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
* See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
*
* @access private
*
* @param uint8 state The state from the previous byte, or 0 when decoding the first byte.
*
* @param uint32 &codepoint The codepoint value. Unless the returned state is 0, the codepoint is
* a partial reasult and the function needs to be called again with the next byte.
*
* @param uint8 byte The byte to decode.
*
* @return The state of the utf8 decoder: 0 if a character has been decoded, 1 in case of
* error, and any other value for decoding in progress.
*/
uint8 JSONValue::decodeUtf8Byte(uint8 state, uint32 &codepoint, uint8 byte) {
	static const uint8 utf8d[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 00..1F
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20..3F
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 40..5F
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 60..7F
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, // 80..9F
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // A0..BF
		8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // C0..DF
		0xA, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 0x3, // E0..EF
		0xB, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // F0..FF
		0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4, 0x6, 0x1, 0x1, 0x1, 0x1, // s0..s0
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, // s1..s2
		1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, // s3..s4
		1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, // s5..s6
		1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 // s7..s8
	};

	const uint8 type = utf8d[byte];
	codepoint = state != 0 ?
		(codepoint << 6) | (byte & 0x3f) :
		(0xFF >> type) & byte;
	return utf8d[256 + state * 16 + type];
}

/**
* Creates the indentation string for the depth given
*
* @access private
*
* @param size_t indent The prettyprint indentation depth (0 : no indentation)
*
* @return String Returns the string
*/
String JSONValue::indent(size_t depth) {
	const size_t indent_step = 2;
	depth ? --depth : 0;
	String indentStr;
	for (size_t i = 0; i < depth * indent_step; ++i) indentStr += ' ';
	return indentStr;
}

} // End of namespace Common
