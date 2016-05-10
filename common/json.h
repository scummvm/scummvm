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
 * Files JSON.h and JSONValue.h part of the SimpleJSON Library - http://mjpa.in/json
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

#include "array.h"
#include "hashmap.h"
#include "hash-str.h"
#include "str.h"

// Win32 incompatibilities
#if defined(WIN32) && !defined(__GNUC__)
#define wcsncasecmp _strnicmp

static inline bool isnan(double x) {
	return x != x;
}

static inline bool isinf(double x) {
	return !isnan(x) && isnan(x - x);
}
#endif

// Linux compile fix - from quaker66
#ifdef __GNUC__
	#include <cstring>
	#include <cstdlib>
#endif

// Mac compile fixes - from quaker66, Lion fix by dabrahams
// Tkachov: I probably broke those
#if defined(__APPLE__) && __DARWIN_C_LEVEL < 200809L || (defined(WIN32) && defined(__GNUC__)) || defined(ANDROID)
	#include <wctype.h>
	#include <wchar.h>
	
	static inline int wcsncasecmp(const char *s1, const char *s2, size_t n)
	{
		int lc1  = 0;
		int lc2  = 0;

		while (n--)
		{
			lc1 = towlower (*s1);
			lc2 = towlower (*s2);

			if (lc1 != lc2)
				return (lc1 - lc2);

			if (!lc1)
				return 0;

			++s1;
			++s2;
		}

		return 0;
	}
#endif

// Simple function to check a string 's' has at least 'n' characters
// Tkachov: that's not wchar_t anymore, though it should work for C-strings too
static inline bool simplejson_wcsnlen(const char* s, size_t n) {
	if (s == 0)
		return false;

	const char* save = s;
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

//JSONValue.h:

class JSON;

enum JSONType { JSONType_Null, JSONType_String, JSONType_Bool, JSONType_Number, JSONType_Array, JSONType_Object };

class JSONValue {
	friend class JSON;

public:
	JSONValue(/*NULL*/);
	JSONValue(const char* m_char_value);
	JSONValue(const String& m_string_value);
	JSONValue(bool m_bool_value);
	JSONValue(double m_number_value);
	JSONValue(const JSONArray& m_array_value);
	JSONValue(const JSONObject& m_object_value);
	JSONValue(const JSONValue& m_source);
	~JSONValue();

	bool IsNull() const;
	bool IsString() const;
	bool IsBool() const;
	bool IsNumber() const;
	bool IsArray() const;
	bool IsObject() const;

	const String& AsString() const;
	bool AsBool() const;
	double AsNumber() const;
	const JSONArray& AsArray() const;
	const JSONObject& AsObject() const;

	std::size_t CountChildren() const;
	bool HasChild(std::size_t index) const;
	JSONValue* Child(std::size_t index);
	bool HasChild(const char* name) const;
	JSONValue* Child(const char* name);
	Array<String> ObjectKeys() const;

	String Stringify(bool const prettyprint = false) const;
protected:
	static JSONValue* Parse(const char** data);

private:
	static String StringifyString(const String& str);
	String StringifyImpl(size_t const indentDepth) const;
	static String Indent(size_t depth);

	JSONType type;

	union {
		bool bool_value;
		double number_value;
		String* string_value;
		JSONArray* array_value;
		JSONObject* object_value;
	};

};

//EOF JSONValue.h

class JSON {
	friend class JSONValue;

public:
	static JSONValue* Parse(const char* data);
	static String Stringify(const JSONValue* value);
protected:
	static bool SkipWhitespace(const char** data);
	static bool ExtractString(const char** data, String& str);
	static double ParseInt(const char** data);
	static double ParseDecimal(const char** data);
private:
	JSON();
};

}  // End of namespace Common

#endif
