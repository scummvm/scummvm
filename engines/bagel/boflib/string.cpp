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

#include "common/str.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

#define MAX_STRING 256

#define BUF_EXTRA 20

CBofString BofEmptyString;

CBofString::CBofString() {
	Init();
}

CBofString::CBofString(int nLength) {
	Init();
	AllocBuffer(nLength);
}

CBofString::CBofString(const CBofString &cString) {
	Init();
	Copy(cString.m_pszData);
}

CBofString::CBofString(const char *pszBuf) {
	Init();
	Copy(pszBuf);
}

CBofString::CBofString(char *pszBuff, int pszBuffLen) {
	Init();

	// Force a max length (To stop uninitialized buffer from killing us)
	*(pszBuff + pszBuffLen - 1) = '\0';

	// Passing in un-initialized data could cause problems depending on what
	// string functions are used after this constructor.
	m_nLength = (uint16)strlen(pszBuff);

	// Use the high byte of the buffer size to determine if we're using stack memory.
	// Make sure that we don't have an obscenly large string

	Assert((pszBuffLen & mUseStackMem) == FALSE);
	SETBUFFERSIZE(pszBuffLen, true);
	m_pszData = pszBuff;
}

CBofString::CBofString(char ch, int nRepeat) {
	Assert(nRepeat > 0);

	Init();

	AllocBuffer(nRepeat);

	BofMemSet(m_pszData, ch, nRepeat);
}

CBofString::~CBofString() {
	Assert(IsValidObject(this));

	Free();
}

void CBofString::Init() {
	Assert(IsValidObject(this));

	SETBUFFERSIZE(0, FALSE);
	m_nLength = 0;
	m_pszData = nullptr;
}

void CBofString::AllocBuffer(int nLen) {
	Assert(IsValidObject(this));
	Assert(nLen >= 0);

	// Thess 3 lines "should" do the same thing
	Assert(nLen < SHRT_MAX); // max size (enough room for 1 extra)
	Assert((uint16)nLen < mUseStackMem);
	Assert((nLen & mUseStackMem) == 0);

	// delete any previous buffer
	Free();

	// Don't do anything about zero length allocations
	if (nLen) {

		if ((m_pszData = (char *)BofAlloc(nLen + 1)) != nullptr) {
			// Set the entire buffer to nullptr
			BofMemSet(m_pszData, '\0', nLen + 1);
		}
	}

	m_nLength = 0;

	// Use the high byte of the bufflen to indicate if we're using
	// stack mem or not.  Make sure our buff size is not too big
	Assert((nLen & mUseStackMem) == 0);
	SETBUFFERSIZE(nLen, FALSE);
}

void CBofString::Free() {
	Assert(IsValidObject(this));
	bool bStackMem = USESSTACKMEM();

	// Free any attached data
	// Only free if it's not stack memory
	if (m_pszData != nullptr && bStackMem == FALSE) {
		BofFree(m_pszData);
		m_pszData = nullptr;
	}
	m_nLength = 0;
	SETBUFFERSIZE(0, bStackMem);
}

void CBofString::SafeDelete(char *pszBuf) {
	if (pszBuf != nullptr) {
		// delete[] pszBuf;
		BofFree(pszBuf);
	}
}

void CBofString::Copy(const char *pszBuf) {
	Assert(IsValidObject(this));

	if (NORMALIZEBUFFERSIZE() != 0)
		*m_pszData = '\0';

	m_nLength = 0;
	if (pszBuf != nullptr) {
		int n;
		n = strlen(pszBuf);

		if (NORMALIZEBUFFERSIZE() <= n) {
			AllocBuffer(n + 1);
		}

		Common::strcpy_s(m_pszData, n + 1, pszBuf);

		if (m_pszData != nullptr) {
			m_nLength = (uint16)strlen(m_pszData);
		}
	}
}

void CBofString::AllocCopy(CBofString &dest, int nCopyLen, int nCopyIndex, int nExtraLen) const {
	Assert(IsValidObject(this));

	// will clone the data attached to this string
	// allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string

	int nNewLen = nCopyLen + nExtraLen;

	if (nNewLen == 0) {
		dest.Free();

	} else {
		dest.AllocBuffer(nNewLen);

		Assert(m_pszData != nullptr);
		BofMemCopy(dest.m_pszData, &m_pszData[nCopyIndex], nCopyLen * sizeof(char));
	}
}

const CBofString &CBofString::operator=(const CBofString &cString) {
	Assert(IsValidObject(this));

	Copy(cString.m_pszData);
	return *this;
}

const CBofString &CBofString::operator=(const char *lpsz) {
	Assert(IsValidObject(this));

	Copy(lpsz);
	return *this;
}

void CBofString::ConcatCopy(int nSrc1Len, const char *lpszSrc1Data, int nSrc2Len, const char *lpszSrc2Data, int nAllocLen) {
	Assert(IsValidObject(this));

	// -- master concatenation routine
	// Concatenate two sources
	// -- assume that 'this' is a new CBofString object

	int nNewLen = nSrc1Len + nSrc2Len;
	AllocBuffer((nAllocLen == 0 ? nNewLen : nAllocLen));
	BofMemCopy(m_pszData, lpszSrc1Data, nSrc1Len * sizeof(char));
	BofMemCopy(&m_pszData[nSrc1Len], lpszSrc2Data, nSrc2Len * sizeof(char));
	// RMS
	m_nLength = (uint16)nNewLen;
}

CBofString operator+(const CBofString &string1, const CBofString &string2) {
	CBofString s;
	s.ConcatCopy(string1.m_nLength, string1.m_pszData, string2.m_nLength, string2.m_pszData);
	return s;
}

CBofString operator+(const CBofString &string, const char *lpsz) {
	CBofString s;
	s.ConcatCopy(string.m_nLength, string.m_pszData, CBofString::SafeStrlen(lpsz), lpsz);
	return s;
}

CBofString operator+(const char *lpsz, const CBofString &string) {
	CBofString s;
	s.ConcatCopy(CBofString::SafeStrlen(lpsz), lpsz, string.m_nLength, string.m_pszData);
	return s;
}

void CBofString::ConcatInPlace(int nSrcLen, const char *lpszSrcData) {
	char szLocalBuff[512];
	Assert(IsValidObject(this));

	//  -- the main routine for += operators

	// If the buffer is too small, or we have a width mis-match, just
	// allocate a new buffer (slow but sure)
	//
	// Make sure we have an underlying buffer.
	if (m_nLength + nSrcLen >= NORMALIZEBUFFERSIZE()) {
		// Don't increment by buf extra, but set the size if we're
		// less than that default threshhold.
		int nAllocAmount = nSrcLen;

		if (nAllocAmount < BUF_EXTRA) {
			nAllocAmount = BUF_EXTRA;
		}

		if (NORMALIZEBUFFERSIZE() == 0) {

			AllocBuffer(m_nLength + nAllocAmount);
			BofMemCopy(m_pszData, lpszSrcData, nSrcLen);
			*(m_pszData + nSrcLen) = '\0';
			m_nLength = (uint16)(m_nLength + (uint16)nSrcLen);

		} else {

			// Handle no buffer in place.
			if (NORMALIZEBUFFERSIZE() == 0) {
				AllocBuffer(m_nLength + nAllocAmount);
			}

			Assert(m_pszData != nullptr);

			// we have to grow the buffer, use the Concat in place routine
			char *lpszOldData;

			if ((m_nLength + nSrcLen + 1) < 512)
				lpszOldData = szLocalBuff;
			else
				lpszOldData = new char[m_nLength + nSrcLen + 1];

			if (lpszOldData != nullptr) {
				BofMemCopy(lpszOldData, m_pszData, (m_nLength /*+ nSrcLen*/ + 1) * sizeof(char));

				ConcatCopy(m_nLength, lpszOldData, nSrcLen, lpszSrcData, m_nLength + nAllocAmount);

				if (lpszOldData != szLocalBuff)
					delete[] lpszOldData;
			}
		}
	} else {

		// Fast concatenation when buffer big enough
		// Optimize for a single byte

		if (nSrcLen == 1) {
			m_pszData[m_nLength] = *lpszSrcData;
		} else {
			BofMemCopy(&m_pszData[m_nLength], lpszSrcData, nSrcLen * sizeof(char));
		}
		m_nLength = (uint16)(m_nLength + (uint16)nSrcLen);
	}
	Assert(m_nLength <= NORMALIZEBUFFERSIZE());
	m_pszData[m_nLength] = '\0';
}

const CBofString &CBofString::operator+=(const char *lpsz) {
	Assert(IsValidObject(this));

	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const CBofString &CBofString::operator+=(char ch) {
	Assert(IsValidObject(this));

	ConcatInPlace(1, &ch);
	return *this;
}

const CBofString &CBofString::operator+=(const CBofString &string) {
	Assert(IsValidObject(this));

	ConcatInPlace(string.m_nLength, string.m_pszData);
	return *this;
}

char *CBofString::GetBuffer() {
	Assert(IsValidObject(this));

	if (m_pszData == nullptr) {
		AllocBuffer(1);
	}

	return m_pszData;
}

void CBofString::ReleaseBuffer(int nNewLength) {
	Assert(IsValidObject(this));
	Assert(nNewLength >= 0);

	if (m_pszData != nullptr) {

		m_nLength = MIN<uint16>(nNewLength, m_nLength);

		m_pszData[m_nLength] = '\0';
	}
}

void CBofString::FreeExtra() {
	Assert(IsValidObject(this));

	Assert(m_nLength <= NORMALIZEBUFFERSIZE());

	if (m_nLength != NORMALIZEBUFFERSIZE()) {

		char *pszOldData;

		if ((pszOldData = new char[m_nLength + 1]) != nullptr) {
			Common::strlcpy(pszOldData, m_pszData, 9999);

			AllocBuffer(m_nLength);
			Common::strlcpy(m_pszData, pszOldData, 9999);
			Assert(m_pszData[m_nLength] == '\0');

			delete[] pszOldData;
		}
	}
	Assert(m_pszData != nullptr);
}

int CBofString::Find(char ch) const {
	Assert(IsValidObject(this));

	// find first single character
	char *lpsz = nullptr;

	if (m_pszData != nullptr)
		lpsz = strchr(m_pszData, ch);

	// return -1 if not found and index otherwise
	return (lpsz == nullptr) ? -1 : (int)(lpsz - m_pszData);
}

int CBofString::FindOneOf(const char *lpszCharSet) const {
	Assert(IsValidObject(this));

	char *lpsz = nullptr;
	if (m_pszData != nullptr)
		lpsz = strpbrk(m_pszData, lpszCharSet);

	return (lpsz == nullptr) ? -1 : (int)(lpsz - m_pszData);
}

int CBofString::FindNumOccurrences(const char *pszSub) {
	Assert(IsValidObject(this));
	Assert(pszSub != nullptr);
	Assert(*pszSub != '\0');

	char *pszCur;
	int nHits;

	nHits = 0;
	if (m_pszData != nullptr) {

		pszCur = m_pszData;
		while (pszCur != nullptr) {

			if ((pszCur = strstr(pszCur, pszSub)) != nullptr) {
				nHits++;
				pszCur++;
			}
		}
	}

	return nHits;
}

const CBofString &CBofString::operator=(char ch) {
	Assert(IsValidObject(this));

	char szBuf[4];

	Common::sprintf_s(szBuf, "%c", ch);
	Copy(szBuf);

	return *this;
}

CBofString operator+(const CBofString &string1, char ch) {
	CBofString s;
	s.ConcatCopy(string1.m_nLength, string1.m_pszData, 1, &ch);
	return s;
}

CBofString operator+(char ch, const CBofString &string) {
	CBofString s;
	s.ConcatCopy(1, &ch, string.m_nLength, string.m_pszData);
	return s;
}

CBofString CBofString::Mid(int nFirst) const {
	Assert(IsValidObject(this));

	return Mid(nFirst, m_nLength - nFirst);
}

CBofString CBofString::Mid(int nFirst, int nCount) const {
	Assert(IsValidObject(this));

	Assert(nFirst >= 0);
	Assert(nCount >= 0);

	// out-of-bounds requests return sensible things
	if (nFirst + nCount > m_nLength)
		nCount = m_nLength - nFirst;
	if (nFirst > m_nLength)
		nCount = 0;

	CBofString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

void CBofString::Mid(int nFirst, CBofString *mStr) const {
	Assert(IsValidObject(this));
	Mid(nFirst, m_nLength - nFirst, mStr);
}

void CBofString::Mid(int nFirst, int nCount, CBofString *mStr) const {
	Assert(IsValidObject(this));
	Assert(mStr != nullptr);

	Assert(nFirst >= 0);
	Assert(nCount >= 0);

	// out-of-bounds requests return sensible things
	if (nFirst + nCount > m_nLength)
		nCount = m_nLength - nFirst;
	if (nFirst > m_nLength)
		nCount = 0;

	*mStr = *this;
	memcpy(mStr->m_pszData, &m_pszData[nFirst], nCount);
	mStr->m_nLength = (uint16)nCount;
}

CBofString CBofString::Right(int nCount) const {
	Assert(IsValidObject(this));

	Assert(nCount >= 0);

	if (nCount > m_nLength)
		nCount = m_nLength;

	CBofString dest;
	AllocCopy(dest, nCount, m_nLength - nCount, 0);
	return dest;
}

void CBofString::Right(int nCount, CBofString *rStr) const {
	Assert(IsValidObject(this));

	Assert(nCount >= 0);

	if (nCount > m_nLength)
		nCount = m_nLength;

	*rStr = *this;
	memcpy(rStr->m_pszData, &m_pszData[m_nLength - nCount], nCount);
	rStr->m_nLength = (uint16)nCount;
}

CBofString CBofString::Left(int nCount) const {
	Assert(IsValidObject(this));

	Assert(nCount >= 0);

	if (nCount > m_nLength)
		nCount = m_nLength;

	CBofString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

void CBofString::Left(int nCount, CBofString *lStr) const {
	Assert(IsValidObject(this));

	Assert(nCount >= 0);

	if (nCount > m_nLength)
		nCount = m_nLength;

	*lStr = *this;
	lStr->m_nLength = (uint16)nCount;
}

void CBofString::DeleteLastChar() {
	if (!IsEmpty()) {
		*(m_pszData + m_nLength - 1) = '\0';
		--m_nLength;
	}
}

// strspn equivalent
CBofString CBofString::SpanIncluding(const char *lpszCharSet) const {
	Assert(IsValidObject(this));

	int n = 0;

	if (m_pszData != nullptr)
		n = strspn(m_pszData, lpszCharSet);

	return Left(n);
}

// strcspn equivalent
CBofString CBofString::SpanExcluding(const char *lpszCharSet) const {
	Assert(IsValidObject(this));

	int n = 0;

	if (m_pszData != nullptr)
		n = strcspn(m_pszData, lpszCharSet);

	return Left(n);
}

int CBofString::ReverseFind(char ch) const {
	Assert(IsValidObject(this));

	// find last single character
	char *lpsz = nullptr;

	if (m_pszData != nullptr)
		lpsz = strrchr(m_pszData, ch);

	// return -1 if not found, distance from beginning otherwise
	return (lpsz == nullptr) ? -1 : (int)(lpsz - m_pszData);
}

// find a sub-string (like strstr)
int CBofString::Find(const char *lpszSub) const {
#if BOF_DEBUG
	if (!IsValidObject(this)) {
		LogError(BuildString("CBofString::Find(%s)", lpszSub));
	}
#endif
	Assert(IsValidObject(this));

	// find first matching substring
	char *lpsz = nullptr;

	if (m_pszData != nullptr)
		lpsz = strstr(m_pszData, lpszSub);

	// return -1 for not found, distance from beginning otherwise
	return (lpsz == nullptr) ? -1 : (int)(lpsz - m_pszData);
}

// CBofString formatting

#define FORCE_ANSI 0x10000
#define FORCE_UNICODE 0x20000

// formatting (using wsprintf style formatting)
void CBofString::Format(const char *lpszFormat, ...) {
	Assert(IsValidObject(this));

	char szBuf[MAX_STRING];
	va_list argptr;

	Assert(lpszFormat != nullptr);

	//
	// don't parse the variable input if there aren't any
	//
	if (lpszFormat != nullptr) {
		//
		// parse the variable arguement list
		//
		va_start(argptr, lpszFormat);
		Common::vsprintf_s(szBuf, lpszFormat, argptr);
		va_end(argptr);

		/* make sure we didn't blow the stack */
		Assert(strlen(szBuf) < MAX_STRING);

		AllocBuffer(strlen(szBuf));
		Common::strcpy_s(m_pszData, MAX_STRING, szBuf);
	}
}

int CBofString::SafeStrlen(const char *psz) {
	return (psz == nullptr) ? 0 : strlen(psz);
}

// CBofString support (windows specific)
//
int CBofString::Compare(const char *psz) const {
	Assert(IsValidObject(this));
	Assert(psz != nullptr);

	int n = -1;

	if (m_pszData != nullptr)
		n = strcmp(m_pszData, psz);

	return n;
}

int CBofString::CompareNoCase(const char *psz) const {
	Assert(IsValidObject(this));

	int n = -1;

	if (m_pszData != nullptr)
		n = scumm_stricmp(m_pszData, psz);

	return n;
}

int CBofString::Collate(const char *psz) const {
	Assert(IsValidObject(this));

	return strcoll(m_pszData, psz);
}

char CBofString::GetAt(int nIndex) {
	Assert(IsValidObject(this));

	Assert(nIndex >= 0);
	Assert(nIndex < m_nLength);

	return m_pszData[nIndex];
}

char CBofString::operator[](int nIndex) {
	Assert(IsValidObject(this));

	return GetAt(nIndex);
}

void CBofString::SetAt(int nIndex, char ch) {
	Assert(IsValidObject(this));

	Assert(nIndex >= 0);
	Assert(nIndex < m_nLength);
	Assert(ch != 0);

	if (m_pszData != nullptr) {
		m_pszData[nIndex] = ch;
	}
}

void CBofString::ReplaceCharAt(int nIndex, char chNew) {
	if (m_pszData != nullptr && nIndex < m_nLength) {
		m_pszData[nIndex] = chNew;
	}
}

void CBofString::ReplaceChar(char chOld, char chNew) {
	Assert(IsValidObject(this));

	// would never find the terminator
	Assert(chOld != '\0');

	if (m_pszData != nullptr) {
		char *p;
		int i;

		// walk thru the string and replace the specified character
		//
		p = m_pszData;
		for (i = 0; i < m_nLength; i++) {

			if (*p == chOld) {
				*p = chNew;

				// If we just inserted the terminator, then the length of
				// this string has been changed, and we don't have to search
				// any more.
				//
				if (chNew == '\0') {
					m_nLength = (uint16)i;
					break;
				}
			}

			p++;
		}
	}
}

void CBofString::ReplaceStr(const char *pszOld, const char *pszNew) {
	Assert(IsValidObject(this));
	Assert(pszOld != nullptr);
	Assert(pszNew != nullptr);

	if (m_pszData != nullptr) {
		char *p, *pszSearch;
		int nDiff, nNewLen, nOldLen, nNeedLength;

		nOldLen = strlen(pszOld);
		nNewLen = strlen(pszNew);

		// 1st pass to determine how much more storage space we might need
		//
		if (nNewLen > nOldLen) {

			nDiff = nNewLen - nOldLen;
			nNeedLength = m_nLength + 1;
			p = m_pszData;
			while ((pszSearch = strstr(p, pszOld)) != nullptr) {
				p = pszSearch + nOldLen;

				nNeedLength += nDiff;
			}

			// If we need more storage space for the buffer, then get some
			//
			if (nNeedLength > NORMALIZEBUFFERSIZE()) {

				GrowTo(nNeedLength);
			}
		}

		// Loop on the search and replace
		// Make sure we loop on this, not just once as we can have several occurrences
		// of the token that we are searching for.

		p = m_pszData;
		while ((pszSearch = strstr(p, pszOld)) != nullptr) {
			StrReplaceStr(p, pszOld, pszNew);
			p = pszSearch + nNewLen;
		}

		// Get new length
		m_nLength = (uint16)strlen(m_pszData);
	}
}

void CBofString::GrowTo(int nNewSize) {
	Assert(IsValidObject(this));

	char *p;

	// if there is nothing in the buffer to save, then just allocate what
	// is needed
	//
	if (m_nLength == 0) {

		AllocBuffer(nNewSize);

		// otherwise, we must keep track of whats in the buffer
		//
	} else {

		// Create a temp buffer to save string
		//
		if ((p = (char *)BofAlloc(m_nLength + 2)) != nullptr) {

			// Save copy of string
			Common::strcpy_s(p, MAX_STRING, m_pszData);

			// Make the new buffer
			AllocBuffer(nNewSize);

			// Copy saved string back
			strncpy(m_pszData, p, nNewSize - 1);

			// Get it's new length
			m_nLength = (uint16)strlen(m_pszData);

			// Don't need temp buffer anymore
			BofFree(p);
		}
	}
}

int CBofString::Hash() const {
	int returnValue = 0;
	int i;

	// Needs to be case in-sensative
	//
	for (i = 0; i < m_nLength; i++) {
		returnValue = returnValue + (char)toupper(m_pszData[i]);
	}

	return returnValue;
}

void CBofString::MakeUpper() {
	Common::String s(m_pszData);
	s.toUppercase();

	strncpy(m_pszData, s.c_str(), m_nLength);
}

void CBofString::MakeLower() {
	Common::String s(m_pszData);
	s.toLowercase();

	strncpy(m_pszData, s.c_str(), m_nLength);
}

} // namespace Bagel
