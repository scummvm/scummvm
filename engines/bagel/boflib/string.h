
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

#include "bagel/boflib/object.h"

namespace Bagel {

// The buffer size and string len members are shorts, and use the high byte
// of the buffer size to tell us if it is stack memory being used.
#define mUseStackMem 0x8000
#define NORMALIZEBUFFERSIZE() (m_nBufferSize & ~mUseStackMem)
#define SETBUFFERSIZE(size, usestackmem) (m_nBufferSize = (USHORT)(size + (usestackmem ? mUseStackMem : 0)))
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
	 * @param pszBuf		Source string
	 */
	CBofString(const CHAR *pszBuf);

	/**
	 * Construct a string based on a source string
	 */
	CBofString(const CBofString &stringSrc);

	/**
	 * Construct a string with a single character repeated a number of times
	 * @param ch		Character to use
	 * @param nRepeat	Number of repeats
	 */
	CBofString(CHAR ch, INT nRepeat = 1);

	/**
	 * Constructs a string, and sets initial buffer size
	 * @param nLength		String length
	 */
	CBofString(INT nLength);

	/**
	 * Construct a string based on a subset of a passed null terminated string
	 * @brief	This modifies the passed source string to null terminate the substring
	 * @param pszBuff		Source string
	 * @param pszBuffLen	Substring length
	 */
	CBofString(CHAR *pszBuff, INT pszBuffLen);

	/**
	 * Destructor
	 */
	virtual ~CBofString();

	// Attributes & Operations

	INT GetBufferSize() const { return NORMALIZEBUFFERSIZE(); }
	INT GetLength() const { return m_nLength; }
	BOOL IsEmpty() const { return m_nLength == 0; }

	/**
	 * De-Allocates internal buffer for current CBofString
	 */
	VOID Free();

	VOID GrowTo(INT nNewSize); // Resize the buffer

	CHAR GetAt(INT nIndex);      // 0 based
	CHAR operator[](INT nIndex); // same as GetAt
	VOID SetAt(INT nIndex, CHAR ch);

	operator const CHAR *() const { return (const CHAR *)m_pszData; }

	// Hashing support.
	//
	INT Hash() const;

	// Overloaded assignment
	/**
	 * Copies specified string into current CBofString
	 * @param pszBuf	Buffer to copy
	 */
	VOID Copy(const CHAR *pszSourceBuf);

	const CBofString &operator=(const CBofString &cStringSrc);
	const CBofString &operator=(CHAR ch);
	const CBofString &operator=(const CHAR *psz);

	// String concatenation
	const CBofString &operator+=(const CBofString &cString);
	const CBofString &operator+=(CHAR ch);
	const CBofString &operator+=(const CHAR *psz);

	friend CBofString operator+(const CBofString &string1, const CBofString &string2);
	friend CBofString operator+(const CBofString &string, CHAR ch);
	friend CBofString operator+(CHAR ch, const CBofString &string);
	friend CBofString operator+(const CBofString &string, const CHAR *lpsz);
	friend CBofString operator+(const CHAR *lpsz, const CBofString &string);

	// String comparison
	INT Compare(const CHAR *lpsz) const;       // straight character
	INT CompareNoCase(const CHAR *lpsz) const; // ignore case
	INT Collate(const CHAR *lpsz) const;       // NLS aware

	// Simple sub-string extraction
	//
	CBofString Mid(INT nFirst, INT nCount) const;
	CBofString Mid(INT nFirst) const;
	CBofString Left(INT nCount) const;
	CBofString Right(INT nCount) const;

	VOID Mid(INT nFirst, INT nCount, CBofString *) const;
	VOID Mid(INT nFirst, CBofString *) const;
	VOID Left(INT nCount, CBofString *) const;
	VOID Right(INT nCount, CBofString *) const;

	CBofString SpanIncluding(const CHAR *lpszCharSet) const;
	CBofString SpanExcluding(const CHAR *lpszCharSet) const;

	// Upper/lower/reverse conversion
	VOID MakeUpper();
	VOID MakeLower();

	// Searching (return starting index, or -1 if not found)
	// look for a single character match
	INT Find(CHAR ch) const; // like "C" strchr
	INT ReverseFind(CHAR ch) const;
	INT FindOneOf(const CHAR *lpszCharSet) const;

	// Look for a specific sub-string
	INT Find(const CHAR *lpszSub) const; // like "C" strstr
	INT FindNumOccurrences(const CHAR *pszSub);

	// Search and replace routines
	VOID ReplaceCharAt(INT, CHAR); // jwl 09.03.96
	VOID ReplaceChar(CHAR chOld, CHAR chNew);
	VOID ReplaceStr(const CHAR *pszOld, const CHAR *pszNew);

	// Simple formatting
	VOID Format(const CHAR *lpszFormat, ...);

	// Access to string implementation buffer as "C" character array
	CHAR *GetBuffer();
	VOID ReleaseBuffer(INT nNewLength = 0);
	VOID FreeExtra();

protected:
	// implementation helpers

	/**
	 * Initialize current CBofString members
	 */
	VOID Init();

	/**
	 * Allocates specified string into specified destination
	 */
	VOID AllocCopy(CBofString &dest, INT nCopyLen, INT nCopyIndex, INT nExtraLen) const;

	/**
	 * Allocates internal buffer for current CBofString
	 * @param nLen		Initial buffer size
	 */
	VOID AllocBuffer(INT nLen);

	VOID ConcatCopy(INT nSrc1Len, const CHAR *lpszSrc1Data, INT nSrc2Len, const CHAR *lpszSrc2Data, INT nAllocLen = 0);
	VOID ConcatInPlace(INT nSrcLen, const CHAR *lpszSrcData);
	static VOID SafeDelete(CHAR *lpsz);
	static INT SafeStrlen(const CHAR *lpsz);

	// Lengths/sizes in characters
	//  (note: an extra character is always allocated)
	//
	CHAR *m_pszData;      // actual string (zero terminated)
	USHORT m_nLength;     // does not include terminating 0
	USHORT m_nBufferSize; // does not include terminating 0
};

// Inline Comparison operators
//
INLINE BOOL operator==(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) == 0;
}

INLINE BOOL operator==(const CBofString &s1, const CHAR *s2) {
	return s1.Compare(s2) == 0;
}

INLINE BOOL operator==(const CHAR *s1, const CBofString &s2) {
	return s2.Compare(s1) == 0;
}

INLINE BOOL operator!=(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) != 0;
}

INLINE BOOL operator!=(const CBofString &s1, const CHAR *s2) {
	return s1.Compare(s2) != 0;
}

INLINE BOOL operator!=(const CHAR *s1, const CBofString &s2) {
	return s2.Compare(s1) != 0;
}

INLINE BOOL operator<(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) < 0;
}

INLINE BOOL operator<(const CBofString &s1, const CHAR *s2) {
	return s1.Compare(s2) < 0;
}

INLINE BOOL operator<(const CHAR *s1, const CBofString &s2) {
	return s2.Compare(s1) > 0;
}

INLINE BOOL operator>(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) > 0;
}

INLINE BOOL operator>(const CBofString &s1, const CHAR *s2) {
	return s1.Compare(s2) > 0;
}

INLINE BOOL operator>(const CHAR *s1, const CBofString &s2) {
	return s2.Compare(s1) < 0;
}

INLINE BOOL operator<=(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) <= 0;
}

INLINE BOOL operator<=(const CBofString &s1, const CHAR *s2) {
	return s1.Compare(s2) <= 0;
}

INLINE BOOL operator<=(const CHAR *s1, const CBofString &s2) {
	return s2.Compare(s1) >= 0;
}

INLINE BOOL operator>=(const CBofString &s1, const CBofString &s2) {
	return s1.Compare(s2) >= 0;
}

INLINE BOOL operator>=(const CBofString &s1, const CHAR *s2) {
	return s1.Compare(s2) >= 0;
}

INLINE BOOL operator>=(const CHAR *s1, const CBofString &s2) {
	return s2.Compare(s1) <= 0;
}

extern CBofString BofEmptyString;

} // namespace Bagel

#endif
