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

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_string.h"

#include "common/textconsole.h"

namespace ICB {

#define SLEN_CHECK (slen < 0)

const char *pxString::operator=(const char *str) {
	// Assign a value

	// If we are assigning the string to ourself, then no assignment is necessary
	if (str == s)
		return (s);

	if (s)
		delete[] s;
	if (str) {
		// We are not assigning a null string
		uint32 len = strlen(const_cast<char *>(str)) + 1;
		s = new char[len];
		memcpy((unsigned char *)s, (unsigned char *)const_cast<char *>(str), len);
	} else
		s = nullptr; // Just the null string
	return (s);
}

void pxString::operator=(const pxString &str) {
	// Assign a value
	if (s)
		delete[] s;
	if (str.s) {
		// We are not assigning a null string
		uint32 len = strlen((char *)(str.s)) + 1;
		s = new char[len];
		memcpy((unsigned char *)s, (unsigned char *)str.s, len);
	} else
		s = nullptr; // Null string
}

const char *pxString::operator+=(const char *adder) {
	// Add a string

	if (adder) {
		uint32 slen = s ? strlen(s) : 0;             // Get original string length
		uint32 adderlen = strlen(const_cast<char *>(adder));     // Get additional string length
		char *buf = new char[slen + adderlen + 1]; // Create a buffer
		if (s)
			memcpy((unsigned char *)buf, (unsigned char *)s, slen); // Move the original string in

		memcpy((unsigned char *)buf + slen, (unsigned char *)const_cast<char *>(adder), adderlen + 1); // And put the additional string in

		// Tidy up
		delete[] s;
		s = buf;
	}

	return (s);
}

const pxString pxString::operator+(const char *adder) const {
	// Produce a string addition without affecting this object

	pxString temp(s);
	temp += adder;
	return (temp);
}

bool pxString::operator==(const char *string) const {
	// Do a character by character comparison
	if (s == nullptr)
		return ((bool)(string == nullptr));
	if (string == nullptr)
		return (false);
	return ((bool)(strcmp(s, const_cast<char *>(string)) == 0));
}

void pxString::SetString(const char *data, uint32 len) {
	// Get the first len characters from another string

	// Lose any string we currently hold
	if (s)
		delete[] s;

	// If the data is NULL then we become NULL
	if (data) {
		// Copy in the data
		s = new char[len + 1];
		memcpy((unsigned char *)s, (unsigned char *)const_cast<char *>(data), len);
		// And null terminate it
		s[len] = 0;
	} else
		s = nullptr;
}

void pxString::Substr(pxString &rsStr, uint32 nStart, uint32 nNum) const {
	char *pNewString;
	uint32 slen = strlen(s); // ds: No need to calculate this several times

	// Do some range checking.
	if (nStart > (slen - 1)) {
		rsStr = "";
		return;
	}

	// If the requested substring goes past the end of the existing string, simply clip it
	// after the last character in the existing one.
	if (nStart + nNum > slen)
		nNum -= (nStart + nNum) - slen;

	// Create a buffer the correct size to hold the new substring
	pNewString = new char[nNum + 1];

	// Copy in the substring.
	memcpy((unsigned char *)pNewString, (unsigned char *)&s[nStart], nNum);

	// Put the terminator on.
	pNewString[nNum] = '\0';

	// Make the new pxString from the buffer.
	rsStr = pNewString;

	// ds: Deallocate the temporary buffer
	delete[] pNewString;
}

uint32 pxString::StrChr(char cToFind, uint32 nStartPos) const {
	char *pcPositionOfFirst;
	uint32 nStringLength = strlen(s);

	// Check if the start position is outside the string.
	if (nStartPos >= nStringLength)
		return nStringLength;

	// I use strchr, which relies on the pxString being implemented as a character array,
	// but it is OK to do it in here, 'cos if the internal representation changed, presumably
	// every function in this class would need looking at anyway.
	pcPositionOfFirst = strchr(s + nStartPos, static_cast<uint>(cToFind));

	if (pcPositionOfFirst) {
		// Character was found.  Work out its index from the address.
		return (pcPositionOfFirst - s);
	} else {
		// I use the length of the string as a rogue value to indicate the character wasn't found.
		return nStringLength;
	}
}

void pxString::ToUpper() {
	if (s) {
		char *sp = s;
		while (*sp) {
			*sp = (char)toupper(*sp);
			sp++;
		}
	}
}

void pxString::ToLower() {
	if (s) {
		char *sp = s;
		while (*sp) {
			*sp = (char)tolower(*sp);
			sp++;
		}
	}
}

void pxString::ConvertPath() {
	if (s) {
		char *sp = s;
		while (*sp) {
			*sp = (*sp == '\\' ? '/' : *sp);
			sp++;
		}
		// trim '/'
		if (*s == '/') {
			uint32 len = strlen((char *)s);
			sp = new char[len];
			memcpy((unsigned char *)sp, (unsigned char *)(s + 1), len);
			delete[] s;
			s = sp;
		}
	}
}

const pxString &pxString::Format(const char *format, ...) {
	if (s)
		delete[] s;
	s = nullptr;

	// Check for a null parameter
	if (format == nullptr)
		return (*this);

	// The data could be any size. Rather than incrementally allocating memory until
	// it fits a large buffer multiple is used that should cover 99.9% of all strings
	// but will still cope with unfeasably large ones
	uint32 startBufferSize = 1024;

	// Allocate a start buffer
	s = new char[startBufferSize + 2];
	if (s == nullptr)
		return (*this);

	// Process the variable arguments
	va_list arglist;
	int32 slen;

	// Keep doubling the size of the buffer until it fits
	while (va_start(arglist, format), slen = vsnprintf(s, startBufferSize, const_cast<char *>(format), arglist), SLEN_CHECK) {
		delete[] s;

		startBufferSize += startBufferSize;
		s = new char[startBufferSize + 2];

		// According to VC5 release mode this code is unreachable
		// I can't see why, so I shall turn the warning off for this bit of code

		// If the sllocation failed return an empty string
		if (s == nullptr)
			return (*this);
	}

	// Tidy up and finish
	va_end(arglist);

	// At this point the buffer in s is much larger than it needs to be
	// In the interest of saving space, it will now be reduced
	assert(slen == (int32)strlen(s));
	char *tempBuffer = new char[slen + 1];

	// If this allocation fails leave the string as it is
	if (tempBuffer) {
		memcpy((unsigned char *)tempBuffer, (unsigned char *)s, slen + 1);
		delete[] s;
		s = tempBuffer;
	}

	return (*this);
}

// build a temporary string
const char *pxVString(const char *format, ...) {
#define PXV_BUF_SIZE 1024

	static char buf[PXV_BUF_SIZE];
	va_list va;

	va_start(va, format);
	vsnprintf(buf, PXV_BUF_SIZE, format, va);
	va_end(va);

#undef PXV_BUF_SIZE

	return buf;
}

pxFixedCharBuffer::pxFixedCharBuffer(uint32 len) {
	// Construct the object with the appropriate amount of data
	m_data = new char[len];

	// Check for an error
	if (m_data == nullptr) {
		error("pxFixedCharBuffer memory allocation error");
	}
}

pxFlexiCharBuffer::pxFlexiCharBuffer(uint32 initLen) {
	m_bufLen = initLen;

	m_buffer = new char[initLen]; // The buffer itself
}

pxFlexiCharBuffer::~pxFlexiCharBuffer() { delete[] m_buffer; }

char &pxFlexiCharBuffer::operator[](uint32 offset) {
	CheckSize(offset);
	return (m_buffer[offset]);
}

void pxFlexiCharBuffer::CheckSize(uint32 size) {
	// Make sure we have enough room

	if (size >= m_bufLen) {
		uint32 newLen = size + 1;
		char *newb = new char[newLen];
		assert(newb);
		memcpy((unsigned char *)newb, (unsigned char *)m_buffer, m_bufLen);
		delete[] m_buffer;
		m_buffer = newb;
		m_bufLen = newLen;
	}
}

void pxFlexiCharBuffer::StrCpy(uint32 offset, const char *string) {
	// Add a string
	uint32 slen = strlen(const_cast<char *>(string));
	CheckSize(offset + slen);
	memcpy((unsigned char *)(m_buffer + offset), (unsigned char *)const_cast<char *>(string), slen);
}

void pxFlexiCharBuffer::StrnCpy(uint32 offset, const char *string, uint32 len) {
	// Copy a number of characters to the buffer
	CheckSize(offset + len);
	memcpy((unsigned char *)(m_buffer + offset), (unsigned char *)const_cast<char *>(string), len);
}

} // End of namespace ICB
