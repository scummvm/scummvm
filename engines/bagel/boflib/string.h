
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef BAGEL_BOFLIB_STRING_H
#define BAGEL_BOFLIB_STRING_H

#include "common/system.h"
#include "bagel/boflib/object.h"

namespace Bagel {

// The buffer size and string len members are shorts, and use the high byte
// of the buffer size to tell us if it is stack memory being used.
#define mUseStackMem 0x8000
#define NORMALIZEBUFFERSIZE() (_nBufferSize & ~mUseStackMem)
#define SETBUFFERSIZE(size, usestackmem) (_nBufferSize = (uint16)((size) + ((usestackmem) ? mUseStackMem : 0)))
#define USESSTACKMEM() (_nBufferSize & mUseStackMem)

class CBofString : public CBofObject {
public:
	// Constructors
	/**
	 * Default constructor
	 */
	CBofString();

	/**
	 * Construct a string from a null terminated string
	 * @param pszBuf        Source string
	 */
	CBofString(const char *pszBuf);

	/**
	 * Construct a string based on a source string
	 */
	CBofString(const CBofString &stringSrc);

	/**
	 * Construct a string with a single character repeated a number of times
	 * @param ch        Character to use
	 * @param nRepeat   Number of repeats
	 */
	CBofString(char ch, int nRepeat = 1);

	/**
	 * Constructs a string, and sets initial buffer size
	 * @param nLength       String length
	 */
	CBofString(int nLength);

	/**
	 * Construct a string based on a subset of a passed null terminated string
	 * @brief   This modifies the passed source string to null terminate the substring
	 * @param pszBuff       Source string
	 * @param pszBuffLen    Substring length
	 */
	CBofString(char *pszBuff, int pszBuffLen);

	/**
	 * Destructor
	 */
	virtual ~CBofString();

	// Attributes & Operations

	int getBufferSize() const {
		return NORMALIZEBUFFERSIZE();
	}
	int getLength() const {
		return _nLength;
	}
	bool isEmpty() const {
		return _nLength == 0;
	}

	/**
	 * De-Allocates internal buffer for current CBofString
	 */
	void free();

	void growTo(int nNewSize); // Resize the buffer

	char getAt(int nIndex);      // 0 based
	char operator[](int nIndex); // same as getAt

	operator const char *() const {
		return (const char *)_pszData;
	}
	const char *getBuffer() const {
		return _pszData;
	}

	// Hashing support.
	//
	int hash() const;

	// Overloaded assignment
	/**
	 * Copies specified string into current CBofString
	 * @param pszSourceBuf    Buffer to copy
	 */
	void copy(const char *pszSourceBuf);

	const CBofString &operator=(const CBofString &cStringSrc);
	const CBofString &operator=(char ch);
	const CBofString &operator=(const char *psz);

	// String concatenation
	const CBofString &operator+=(const CBofString &cString);
	const CBofString &operator+=(char ch);
	const CBofString &operator+=(const char *psz);

	friend CBofString operator+(const CBofString &string1, const CBofString &string2);
	friend CBofString operator+(const CBofString &string, char ch);
	friend CBofString operator+(char ch, const CBofString &string);
	friend CBofString operator+(const CBofString &string, const char *lpsz);
	friend CBofString operator+(const char *lpsz, const CBofString &string);

	// String comparison
	int compare(const char *lpsz) const;       // straight character
	int compareNoCase(const char *lpsz) const; // ignore case

	// Simple sub-string extraction
	//
	CBofString mid(int nFirst, int nCount) const;
	CBofString mid(int nFirst) const;
	CBofString left(int nCount) const;
	CBofString right(int nCount) const;

	void deleteLastChar();

	// Upper/lower/reverse conversion
	void makeUpper();

	// Searching (return starting index, or -1 if not found)
	// Look for a specific sub-string
	int find(const char *lpszSub) const; // like "C" strstr
	int findNumOccurrences(const char *pszSub);

	// Search and replace routines
	void replaceCharAt(int, char);
	void replaceChar(char chOld, char chNew);
	void replaceStr(const char *pszOld, const char *pszNew);

	// Access to string implementation buffer as "C" character array
	char *getBuffer();

protected:
	// implementation helpers

	/**
	 * initialize current CBofString members
	 */
	void init();

	/**
	 * Allocates specified string into specified destination
	 */
	void allocCopy(CBofString &dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;

	/**
	 * Allocates internal buffer for current CBofString
	 * @param nLen      Initial buffer size
	 */
	void allocBuffer(int nLen);

	void concatCopy(int nSrc1Len, const char *lpszSrc1Data, int nSrc2Len, const char *lpszSrc2Data, int nAllocLen = 0);
	void concatInPlace(int nSrcLen, const char *lpszSrcData);
	static int safeStrlen(const char *lpsz);

	// Lengths/sizes in characters
	//  (note: an extra character is always allocated)
	//
	char *_pszData;      // actual string (zero terminated)
	uint16 _nLength;     // does not include terminating 0
	uint16 _nBufferSize; // does not include terminating 0
};

// Inline Comparison operators
//
inline bool operator==(const CBofString &s1, const CBofString &s2) {
	return s1.compare(s2) == 0;
}

inline bool operator==(const CBofString &s1, const char *s2) {
	return s1.compare(s2) == 0;
}

inline bool operator==(const char *s1, const CBofString &s2) {
	return s2.compare(s1) == 0;
}

inline bool operator!=(const CBofString &s1, const CBofString &s2) {
	return s1.compare(s2) != 0;
}

inline bool operator!=(const CBofString &s1, const char *s2) {
	return s1.compare(s2) != 0;
}

inline bool operator!=(const char *s1, const CBofString &s2) {
	return s2.compare(s1) != 0;
}

inline bool operator<(const CBofString &s1, const CBofString &s2) {
	return s1.compare(s2) < 0;
}

inline bool operator<(const CBofString &s1, const char *s2) {
	return s1.compare(s2) < 0;
}

inline bool operator<(const char *s1, const CBofString &s2) {
	return s2.compare(s1) > 0;
}

inline bool operator>(const CBofString &s1, const CBofString &s2) {
	return s1.compare(s2) > 0;
}

inline bool operator>(const CBofString &s1, const char *s2) {
	return s1.compare(s2) > 0;
}

inline bool operator>(const char *s1, const CBofString &s2) {
	return s2.compare(s1) < 0;
}

inline bool operator<=(const CBofString &s1, const CBofString &s2) {
	return s1.compare(s2) <= 0;
}

inline bool operator<=(const CBofString &s1, const char *s2) {
	return s1.compare(s2) <= 0;
}

inline bool operator<=(const char *s1, const CBofString &s2) {
	return s2.compare(s1) >= 0;
}

inline bool operator>=(const CBofString &s1, const CBofString &s2) {
	return s1.compare(s2) >= 0;
}

inline bool operator>=(const CBofString &s1, const char *s2) {
	return s1.compare(s2) >= 0;
}

inline bool operator>=(const char *s1, const CBofString &s2) {
	return s2.compare(s1) <= 0;
}

} // namespace Bagel

#endif
