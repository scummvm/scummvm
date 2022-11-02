/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ICB_PX_STRING_H
#define ICB_PX_STRING_H

#include "engines/icb/common/px_rcutypes.h"

namespace ICB {

class pxString {
protected:
	char *s; // The actual string

public:
	pxString();                 // Empty creator
	pxString(const char *);     // Copy constructor
	pxString(const pxString &); // Copy constructor
	~pxString();                // Destructor

	operator const char *() const {
		return (s); // Return a pointer to the string
	}
	const char *operator=(const char *);     // Assign a value
	void operator=(const pxString &); // Assign a value
	const char *operator+=(const char *);    // Add a string
	char &operator[](uint32 n) {
		return (s[n]); // Get a character (no reason not to allow it to change)
	}
	char &operator[](int32 n) {
		return (s[n]); // Get a character (no reason not to allow it to change)
	}
	bool IsNull() const {
		return ((bool)(s == NULL)); // Check for a null value
	}
	bool IsEmpty() const;   // Check for an empty string
	uint32 GetLen() const;    // Returns the length of the string.
	void ToUpper();     // Make contents of string uppercase
	void ToLower();     // Make contents of string lowercase
	void ConvertPath(); // Converts a path to native format
	const char *c_str() { return s; }
	const pxString &Format(const char *, ...); // Use variable arguments to set the string

	const pxString operator+(const char *) const;

	inline pxString Substr(uint32 nStart, uint32 nLen) const;       // Return a part of this string
	void Substr(pxString &rsStr, uint32 nStart, uint32 nLen) const; // A faster substring.

	void SetString(const char *data, uint32 len); // Get the first len characters from another string

	uint32 StrChr(char cToFind, uint32 nStartPos = 0) const; // Find position of a character in a string [PS 17/08/98]

	// char * comparisons
	bool operator==(const char *string) const; // Do a character by character comparison
	bool operator!=(const char *string) const; // Do a character by character uncomparison
};

inline pxString::pxString(const pxString &str) {
	if (str.s) {
		// There is some data to copy
		uint32 len = strlen((char *)str.s) + 1;
		s = new char[len];
		memcpy((unsigned char *)s, (unsigned char *)str.s, len);
	} else
		// No data for this string
		s = NULL;
}

inline pxString::~pxString() {
	// Destructor
	if (s)
		delete[] s;
}

const char *pxVString(const char *format, ...);

class pxFlexiCharBuffer {
	char *m_buffer; // The buffer itself
	uint32 m_bufLen;  // The buffer length

public:
	// explicit
	pxFlexiCharBuffer(uint32 len = 40);
	~pxFlexiCharBuffer();

	char &operator[](uint32); // Allow array access
	void CheckSize(uint32);   // Make sure we have enough room
	char *GetBuffer() {
		return (m_buffer); // Make it a little more difficult to pass the pointer
	}

	// Pointer access was originally const char *, but for use as a buffer for reading in from
	// files this needs to be changeable, and needs a void * operator
	operator char *() {
		return (m_buffer); // Treat as a char *
	}
	operator void *() {
		return (m_buffer); // Treat as a void *
	}

	void StrCpy(uint32 offset, const char *text);            // Copy a string to the buffer
	void StrnCpy(uint32 offset, const char *text, uint32 len); // Copy a number of characters to the buffer

	// Prevent copy or assignment
private:
	pxFlexiCharBuffer(const pxFlexiCharBuffer &) {}
	void operator=(const pxFlexiCharBuffer &) {}
};

class pxFixedCharBuffer {
	char *m_data;

public:
	pxFixedCharBuffer(uint32 len);
	~pxFixedCharBuffer() { delete[] m_data; }

	operator void *() { return (m_data); }
	operator char *() { return (m_data); }

	// Prevent copy or assignment
private:
	pxFixedCharBuffer(const pxFixedCharBuffer &) {}
	void operator=(const pxFixedCharBuffer &) {}
};

inline pxString::pxString() {
	// Empty creator
	s = NULL;
}

inline pxString::pxString(const char *str) {
	// Copy constructor
	if (str) {
		uint32 len = strlen(str) + 1;
		s = new char[len];
		memcpy(s, str, len);
	} else
		s = NULL;
}

inline bool pxString::IsEmpty() const {
	// Check for an empty string
	if ((s == NULL) || (*s == 0))
		return (true);

	return (false);
}

inline uint32 pxString::GetLen() const {
	if ((s == NULL))
		return (0);

	return (strlen(s));
}

inline pxString pxString::Substr(uint32 nStart, uint32 nNum) const {
	pxString rsRetVal;

	Substr(rsRetVal, nStart, nNum);

	return rsRetVal;
}

inline bool pxString::operator!=(const char *string) const {
	// Do a character by character uncomparison
	// Simply return the opposit of the == function
	return ((bool)!((*this) == string));
}

} // End of namespace ICB

#endif // #ifndef _PX_pxString_H
