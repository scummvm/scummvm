
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
#define NORMALIZEBUFFERSIZE() (m_nBufferSize & ~mUseStackMem)
#define SETBUFFERSIZE(size, usestackmem) (m_nBufferSize = (uint16)(size + (usestackmem ? mUseStackMem : 0)))
#define USESSTACKMEM() (m_nBufferSize & mUseStackMem)

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

	int GetBufferSize() const {
		return NORMALIZEBUFFERSIZE();
	}
	int GetLength() const {
		return m_nLength;
	}
	bool IsEmpty() const {
		return m_nLength == 0;
	}

	/**
	 * De-Allocates internal buffer for current CBofString
	 */
	void Free();

	void GrowTo(int nNewSize); // Resize the buffer

	char GetAt(int nIndex);      // 0 based
	char operator[](int nIndex); // same as GetAt
	void SetAt(int nIndex, char ch);

	operator const char *() const {
		return (const char *)m_pszData;
	}
	const char *GetBuffer() const {
		return m_pszData;
	}

	// Hashing support.
	//
	int Hash() const;

	// Overloaded assignment
	/**
	 * Copies specified string into current CBofString
	 * @param pszSourceBuf    Buffer to copy
	 */
	void Copy(const char *pszSourceBuf);

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
	int Compare(const char *lpsz) const;       // straight character
	int CompareNoCase(const char *lpsz) const; // ignore case
	int Collate(const char *lpsz) const;       // NLS aware

	// Simple sub-string extraction
	//
	CBofString Mid(int nFirst, int nCount) const;
	CBofString Mid(int nFirst) const;
	CBofString Left(int nCount) const;
	CBofString Right(int nCount) const;

	void Mid(int nFirst, int nCount, CBofString *) const;
	void Mid(int nFirst, CBofString *) const;
	void Left(int nCount, CBofString *) const;
	void Right(int nCount, CBofString *) const;

	void DeleteLastChar();

	CBofString SpanIncluding(const char *lpszCharSet) const;
	CBofString SpanExcluding(const char *lpszCharSet) const;

	// Upper/lower/reverse conversion
	void MakeUpper();
	void MakeLower();

	// Searching (return starting index, or -1 if not found)
	// look for a single character match
	int Find(char ch) const; // like "C" strchr
	int ReverseFind(char ch) const;
	int FindOneOf(const char *lpszCharSet) const;

	// Look for a specific sub-string
	int Find(const char *lpszSub) const; // like "C" strstr
	int FindNumOccurrences(const char *pszSub);

	// Search and replace routines
	void ReplaceCharAt(int, char);
	void ReplaceChar(char chOld, char chNew);
	void ReplaceStr(const char *pszOld, const char *pszNew);

	// Simple formatting
	void Format(const char *lpszFormat, ...);

	// Access to string implementation buffer as "C" character array
	char *GetBuffer();
	void ReleaseBuffer(int nNewLength = 0);
	void FreeExtra();

protected:
	// implementation helpers

	/**
	 * initialize current CBofString members
	 */
	void Init();

	/**
	 * Allocates specified string into specified destination
	 */
	void AllocCopy(CBofString &dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;

	/**
	 * Allocates internal buffer for current CBofString
	 * @param nLen      Initial buffer size
	 */
	void AllocBuffer(int nLen);

	void ConcatCopy(int nSrc1Len, const char *lpszSrc1Data, int nSrc2Len, const char *lpszSrc2Data, int nAllocLen = 0);
	void ConcatInPlace(int nSrcLen, const char *lpszSrcData);
	static void SafeDelete(char *lpsz);
	static int SafeStrlen(const char *lpsz);

	// Lengths/sizes in characters
	//  (note: an extra character is always allocated)
	//
	char *m_pszData;      // actual string (zero terminated)
	uint16 m_nLength;     // does not include terminating 0
	uint16 m_nBufferSize; // does not include terminating 0
};

// Inline Comparison operators
//
inline bool operator==(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) == 0;
}

inline bool operator==(const CBofString &s1, const char *s2) {
	return s1.Compare(s2) == 0;
}

inline bool operator==(const char *s1, const CBofString &s2) {
	return s2.Compare(s1) == 0;
}

inline bool operator!=(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) != 0;
}

inline bool operator!=(const CBofString &s1, const char *s2) {
	return s1.Compare(s2) != 0;
}

inline bool operator!=(const char *s1, const CBofString &s2) {
	return s2.Compare(s1) != 0;
}

inline bool operator<(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) < 0;
}

inline bool operator<(const CBofString &s1, const char *s2) {
	return s1.Compare(s2) < 0;
}

inline bool operator<(const char *s1, const CBofString &s2) {
	return s2.Compare(s1) > 0;
}

inline bool operator>(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) > 0;
}

inline bool operator>(const CBofString &s1, const char *s2) {
	return s1.Compare(s2) > 0;
}

inline bool operator>(const char *s1, const CBofString &s2) {
	return s2.Compare(s1) < 0;
}

inline bool operator<=(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) <= 0;
}

inline bool operator<=(const CBofString &s1, const char *s2) {
	return s1.Compare(s2) <= 0;
}

inline bool operator<=(const char *s1, const CBofString &s2) {
	return s2.Compare(s1) >= 0;
}

inline bool operator>=(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) >= 0;
}

inline bool operator>=(const CBofString &s1, const char *s2) {
	return s1.Compare(s2) >= 0;
}

inline bool operator>=(const char *s1, const CBofString &s2) {
	return s2.Compare(s1) <= 0;
}

extern CBofString BofEmptyString;

} // namespace Bagel

#endif
