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
 * Files JSON.h and JSONValue.h part of the SimpleJSON Library - https://github.com/MJPA/SimpleJSON
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

#ifndef COMMON_JSON_H
#define COMMON_JSON_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/str.h"

// Win32 incompatibilities
#if (defined(WIN32) && !defined(__GNUC__))
static inline bool isnan(double x) {
	return x != x;
}

static inline bool isinf(double x) {
	return !isnan(x) && isnan(x - x);
}
#endif

// Simple function to check a string 's' has at least 'n' characters
static inline bool simplejson_wcsnlen(const char *s, size_t n) {
	if (s == nullptr)
		return false;

	const char *save = s;
	while (n-- > 0) {
		if (*(save++) == 0) return false;
	}

	return true;
}

namespace Common {

// Custom types
class JSONValue;
typedef Array<JSONValue*> JSONArray;
typedef HashMap<String, JSONValue*> JSONObject;

class JSON;

enum JSONType { JSONType_Null, JSONType_String, JSONType_Bool, JSONType_Number, JSONType_IntegerNumber, JSONType_Array, JSONType_Object };

class JSONValue {
	friend class JSON;

public:
	JSONValue(/*NULL*/);
	JSONValue(const char *charValue);
	JSONValue(const String &stringValue);
	JSONValue(bool boolValue);
	JSONValue(double numberValue);
	JSONValue(long long int numberValue);
	JSONValue(const JSONArray &arrayValue);
	JSONValue(const JSONObject &objectValue);
	JSONValue(const JSONValue &source);
	~JSONValue();

	bool isNull() const;
	bool isString() const;
	bool isBool() const;
	bool isNumber() const;
	bool isIntegerNumber() const;
	bool isArray() const;
	bool isObject() const;

	const String &asString() const;
	bool asBool() const;
	double asNumber() const;
	long long int asIntegerNumber() const;
	const JSONArray &asArray() const;
	const JSONObject &asObject() const;

	size_t countChildren() const;
	bool hasChild(size_t index) const;
	JSONValue *child(size_t index);
	bool hasChild(const char *name) const;
	JSONValue *child(const char *name);
	Array<String> objectKeys() const;

	String stringify(bool const prettyprint = false) const;
protected:
	static JSONValue *parse(const char **data);

private:
	static String stringifyString(const String &str);
	static uint32 decodeUtf8Char(String::const_iterator &begin, const String::const_iterator &end);
	static uint8 decodeUtf8Byte(uint8 state, uint32 &codepoint, uint8 byte);
	String stringifyImpl(size_t const indentDepth) const;
	static String indent(size_t depth);

	JSONType _type;

	union {
		bool _boolValue;
		double _numberValue;
		long long int _integerValue;
		String *_stringValue;
		JSONArray *_arrayValue;
		JSONObject *_objectValue;
	};

};

class JSON {
	friend class JSONValue;

public:
	static JSONValue *parse(const char *data);
	static String stringify(const JSONValue *value);
protected:
	static bool skipWhitespace(const char **data);
	static bool extractString(const char **data, String &str);
	static uint32 parseUnicode(const char **data);
	static double parseInt(const char **data);
	static double parseDecimal(const char **data);
private:
	JSON();
};

} // End of namespace Common

#endif
