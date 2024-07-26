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
#include "bagel/boflib/string_functions.h"

namespace Bagel {

#define MAX_STRING 256

#define BUF_EXTRA 20

CBofString::CBofString() {
	init();
}

CBofString::CBofString(int nLength) {
	init();
	allocBuffer(nLength);
}

CBofString::CBofString(const CBofString &cString) {
	init();
	copy(cString._pszData);
}

CBofString::CBofString(const char *pszBuf) {
	init();
	copy(pszBuf);
}

CBofString::CBofString(char *pszBuff, int pszBuffLen) {
	init();

	// Force a max length (To stop uninitialized buffer from killing us)
	*(pszBuff + pszBuffLen - 1) = '\0';

	// Passing in un-initialized data could cause problems depending on what
	// string functions are used after this constructor.
	_nLength = (uint16)strlen(pszBuff);

	// Use the high byte of the buffer size to determine if we're using stack memory.
	// Make sure that we don't have an obscenely large string

	assert((pszBuffLen & mUseStackMem) == false);
	SETBUFFERSIZE(pszBuffLen, true);
	_pszData = pszBuff;
}

CBofString::CBofString(char ch, int nRepeat) {
	assert(nRepeat > 0);

	init();
	allocBuffer(nRepeat);
	memset(_pszData, ch, nRepeat);
}

CBofString::~CBofString() {
	assert(isValidObject(this));

	free();
}

void CBofString::init() {
	assert(isValidObject(this));

	SETBUFFERSIZE(0, false);
	_nLength = 0;
	_pszData = nullptr;
}

void CBofString::allocBuffer(int nLen) {
	assert(isValidObject(this));
	assert(nLen >= 0);

	// These 3 lines "should" do the same thing
	assert(nLen < SHRT_MAX); // max size (enough room for 1 extra)
	assert((uint16)nLen < mUseStackMem);
	assert((nLen & mUseStackMem) == 0);

	// Delete any previous buffer
	free();

	// Don't do anything about zero length allocations
	if (nLen > 0) {
		// Allocate a buffer filled with 0s
		_pszData = (char *)bofCleanAlloc(nLen + 1);
	}

	_nLength = 0;

	// Use the high byte of the bufflen to indicate if we're using
	// stack mem or not.  Make sure our buff size is not too big
	assert((nLen & mUseStackMem) == 0);
	SETBUFFERSIZE(nLen, false);
}

void CBofString::free() {
	assert(isValidObject(this));
	bool bStackMem = USESSTACKMEM();

	// Free any attached data
	// Only free if it's not stack memory
	if (_pszData != nullptr && bStackMem == false) {
		bofFree(_pszData);
		_pszData = nullptr;
	}
	_nLength = 0;
	SETBUFFERSIZE(0, bStackMem);
}

void CBofString::copy(const char *pszBuf) {
	assert(isValidObject(this));

	if (NORMALIZEBUFFERSIZE() != 0)
		*_pszData = '\0';

	_nLength = 0;
	if (pszBuf != nullptr) {
		int n = strlen(pszBuf);

		if (NORMALIZEBUFFERSIZE() <= n) {
			allocBuffer(n + 1);
		}

		Common::strcpy_s(_pszData, n + 1, pszBuf);
		_nLength = (uint16)strlen(_pszData);
	}
}

void CBofString::allocCopy(CBofString &dest, int nCopyLen, int nCopyIndex, int nExtraLen) const {
	assert(isValidObject(this));

	// Will clone the data attached to this string allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string

	int nNewLen = nCopyLen + nExtraLen;

	if (nNewLen == 0) {
		dest.free();

	} else {
		dest.allocBuffer(nNewLen);

		assert(_pszData != nullptr);
		memcpy(dest._pszData, &_pszData[nCopyIndex], nCopyLen * sizeof(char));
	}
}

const CBofString &CBofString::operator=(const CBofString &cString) {
	assert(isValidObject(this));

	copy(cString._pszData);
	return *this;
}

const CBofString &CBofString::operator=(const char *lpsz) {
	assert(isValidObject(this));

	copy(lpsz);
	return *this;
}

void CBofString::concatCopy(int nSrc1Len, const char *lpszSrc1Data, int nSrc2Len, const char *lpszSrc2Data, int nAllocLen) {
	assert(isValidObject(this));

	// -- Master concatenation routine
	// concatenate two sources
	// -- Assume that 'this' is a new CBofString object

	int nNewLen = nSrc1Len + nSrc2Len;
	allocBuffer((nAllocLen == 0 ? nNewLen : nAllocLen));
	memcpy(_pszData, lpszSrc1Data, nSrc1Len * sizeof(char));
	memcpy(&_pszData[nSrc1Len], lpszSrc2Data, nSrc2Len * sizeof(char));
	// RMS
	_nLength = (uint16)nNewLen;
}

CBofString operator+(const CBofString &string1, const CBofString &string2) {
	CBofString s;
	s.concatCopy(string1._nLength, string1._pszData, string2._nLength, string2._pszData);
	return s;
}

CBofString operator+(const CBofString &string, const char *lpsz) {
	CBofString s;
	s.concatCopy(string._nLength, string._pszData, CBofString::safeStrlen(lpsz), lpsz);
	return s;
}

CBofString operator+(const char *lpsz, const CBofString &string) {
	CBofString s;
	s.concatCopy(CBofString::safeStrlen(lpsz), lpsz, string._nLength, string._pszData);
	return s;
}

void CBofString::concatInPlace(int nSrcLen, const char *lpszSrcData) {
	char szLocalBuff[512];
	assert(isValidObject(this));

	//  -- The main routine for += operators

	// If the buffer is too small, or we have a width mis-match, just
	// allocate a new buffer (slow but sure)
	//
	// Make sure we have an underlying buffer.
	if (_nLength + nSrcLen >= NORMALIZEBUFFERSIZE()) {
		// Don't increment by buf extra, but set the size if we're
		// less than that default threshold.
		int nAllocAmount = nSrcLen;

		if (nAllocAmount < BUF_EXTRA) {
			nAllocAmount = BUF_EXTRA;
		}

		if (NORMALIZEBUFFERSIZE() == 0) {
			allocBuffer(_nLength + nAllocAmount);
			memcpy(_pszData, lpszSrcData, nSrcLen);
			*(_pszData + nSrcLen) = '\0';
			_nLength = (uint16)(_nLength + (uint16)nSrcLen);

		} else {
			// Handle no buffer in place.
			if (NORMALIZEBUFFERSIZE() == 0) {
				allocBuffer(_nLength + nAllocAmount);
			}

			assert(_pszData != nullptr);

			// We have to grow the buffer, use the concat in place routine
			char *lpszOldData;

			if ((_nLength + nSrcLen + 1) < 512)
				lpszOldData = szLocalBuff;
			else
				lpszOldData = new char[_nLength + nSrcLen + 1];

			memcpy(lpszOldData, _pszData, (_nLength + 1) * sizeof(char));
			concatCopy(_nLength, lpszOldData, nSrcLen, lpszSrcData, _nLength + nAllocAmount);

			if (lpszOldData != szLocalBuff)
				delete[] lpszOldData;
		}
	} else {

		// Fast concatenation when buffer big enough
		// Optimize for a single byte

		if (nSrcLen == 1) {
			_pszData[_nLength] = *lpszSrcData;
		} else {
			memcpy(&_pszData[_nLength], lpszSrcData, nSrcLen * sizeof(char));
		}
		_nLength = (uint16)(_nLength + (uint16)nSrcLen);
	}

	assert(_nLength <= NORMALIZEBUFFERSIZE());
	_pszData[_nLength] = '\0';
}

const CBofString &CBofString::operator+=(const char *lpsz) {
	assert(isValidObject(this));

	concatInPlace(safeStrlen(lpsz), lpsz);
	return *this;
}

const CBofString &CBofString::operator+=(char ch) {
	assert(isValidObject(this));

	concatInPlace(1, &ch);
	return *this;
}

const CBofString &CBofString::operator+=(const CBofString &string) {
	assert(isValidObject(this));

	concatInPlace(string._nLength, string._pszData);
	return *this;
}

char *CBofString::getBuffer() {
	assert(isValidObject(this));

	if (_pszData == nullptr) {
		allocBuffer(1);
	}

	return _pszData;
}

int CBofString::findNumOccurrences(const char *pszSub) {
	assert(isValidObject(this));
	assert(pszSub != nullptr);
	assert(*pszSub != '\0');

	int nHits = 0;
	if (_pszData != nullptr) {
		char *pszCur = _pszData;
		while (pszCur != nullptr) {
			pszCur = strstr(pszCur, pszSub);
			if (pszCur != nullptr) {
				nHits++;
				pszCur++;
			}
		}
	}

	return nHits;
}

const CBofString &CBofString::operator=(char ch) {
	assert(isValidObject(this));

	char szBuf[4];

	Common::sprintf_s(szBuf, "%c", ch);
	copy(szBuf);

	return *this;
}

CBofString operator+(const CBofString &string1, char ch) {
	CBofString s;
	s.concatCopy(string1._nLength, string1._pszData, 1, &ch);
	return s;
}

CBofString operator+(char ch, const CBofString &string) {
	CBofString s;
	s.concatCopy(1, &ch, string._nLength, string._pszData);
	return s;
}

CBofString CBofString::mid(int nFirst) const {
	assert(isValidObject(this));

	return mid(nFirst, _nLength - nFirst);
}

CBofString CBofString::mid(int nFirst, int nCount) const {
	assert(isValidObject(this));

	assert(nFirst >= 0);
	assert(nCount >= 0);

	// Out-of-bounds requests return sensible things
	if (nFirst + nCount > _nLength)
		nCount = _nLength - nFirst;
	if (nFirst > _nLength)
		nCount = 0;

	CBofString dest;
	allocCopy(dest, nCount, nFirst, 0);
	return dest;
}

CBofString CBofString::right(int nCount) const {
	assert(isValidObject(this));

	assert(nCount >= 0);

	if (nCount > _nLength)
		nCount = _nLength;

	CBofString dest;
	allocCopy(dest, nCount, _nLength - nCount, 0);
	return dest;
}

CBofString CBofString::left(int nCount) const {
	assert(isValidObject(this));

	assert(nCount >= 0);

	if (nCount > _nLength)
		nCount = _nLength;

	CBofString dest;
	allocCopy(dest, nCount, 0, 0);
	return dest;
}

void CBofString::deleteLastChar() {
	if (!isEmpty()) {
		*(_pszData + _nLength - 1) = '\0';
		--_nLength;
	}
}

// Find a sub-string (like strstr)
int CBofString::find(const char *lpszSub) const {
	assert(isValidObject(this));

	// Find first matching substring
	char *lpsz = nullptr;

	if (_pszData != nullptr)
		lpsz = strstr(_pszData, lpszSub);

	// Return -1 for not found, distance from beginning otherwise
	return (lpsz == nullptr) ? -1 : (int)(lpsz - _pszData);
}

// CBofString formatting

#define FORCE_ANSI 0x10000
#define FORCE_UNICODE 0x20000

int CBofString::safeStrlen(const char *psz) {
	return (psz == nullptr) ? 0 : strlen(psz);
}

// CBofString support (windows specific)
//
int CBofString::compare(const char *psz) const {
	assert(isValidObject(this));
	assert(psz != nullptr);

	int n = -1;

	if (_pszData != nullptr)
		n = strcmp(_pszData, psz);

	return n;
}

int CBofString::compareNoCase(const char *psz) const {
	assert(isValidObject(this));

	int n = -1;

	if (_pszData != nullptr)
		n = scumm_stricmp(_pszData, psz);

	return n;
}

char CBofString::getAt(int nIndex) {
	assert(isValidObject(this));

	assert(nIndex >= 0);
	assert(nIndex < _nLength);

	return _pszData[nIndex];
}

char CBofString::operator[](int nIndex) {
	assert(isValidObject(this));

	return getAt(nIndex);
}

void CBofString::replaceCharAt(int nIndex, char chNew) {
	if (_pszData != nullptr && nIndex < _nLength) {
		_pszData[nIndex] = chNew;
	}
}

void CBofString::replaceChar(char chOld, char chNew) {
	assert(isValidObject(this));

	// Would never find the terminator
	assert(chOld != '\0');

	if (_pszData != nullptr) {
		// Walk through the string and replace the specified character
		char *p = _pszData;
		for (int i = 0; i < _nLength; i++) {
			if (*p == chOld) {
				*p = chNew;

				// If we just inserted the terminator, then the length of
				// this string has been changed, and we don't have to search
				// any more.
				//
				if (chNew == '\0') {
					_nLength = (uint16)i;
					break;
				}
			}

			p++;
		}
	}
}

void CBofString::replaceStr(const char *pszOld, const char *pszNew) {
	assert(isValidObject(this));
	assert(pszOld != nullptr);
	assert(pszNew != nullptr);

	if (_pszData != nullptr) {
		char *p, *pszSearch;

		int nOldLen = strlen(pszOld);
		int nNewLen = strlen(pszNew);

		// 1st pass to determine how much more storage space we might need
		if (nNewLen > nOldLen) {
			int nDiff = nNewLen - nOldLen;
			int nNeedLength = _nLength + 1;
			p = _pszData;
			pszSearch = strstr(p, pszOld);
			while (pszSearch != nullptr) {
				p = pszSearch + nOldLen;

				nNeedLength += nDiff;
				pszSearch = strstr(p, pszOld);
			}

			// If we need more storage space for the buffer, then get some
			if (nNeedLength > NORMALIZEBUFFERSIZE()) {
				growTo(nNeedLength);
			}
		}

		// Loop on the search and replace
		// Make sure we loop on this, not just once as we can have several occurrences
		// of the token that we are searching for.

		p = _pszData;
		pszSearch = strstr(p, pszOld);
		while (pszSearch != nullptr) {
			strreplaceStr(p, pszOld, pszNew);
			p = pszSearch + nNewLen;
			pszSearch = strstr(p, pszOld);
		}

		// Get new length
		_nLength = (uint16)strlen(_pszData);
	}
}

void CBofString::growTo(int nNewSize) {
	assert(isValidObject(this));

	// If there is nothing in the buffer to save, then just allocate what
	// is needed
	if (_nLength == 0) {
		allocBuffer(nNewSize);

	} else {
		// Otherwise, we must keep track of whats in the buffer
		// Create a temp buffer to save string
		char *p = (char *)bofAlloc(_nLength + 2);

		// Save copy of string
		Common::strcpy_s(p, MAX_STRING, _pszData);

		// Make the new buffer
		allocBuffer(nNewSize);

		// Copy saved string back
		strncpy(_pszData, p, nNewSize - 1);

		// Get it's new length
		_nLength = (uint16)strlen(_pszData);

		// Don't need temp buffer anymore
		bofFree(p);
	}
}

int CBofString::hash() const {
	int returnValue = 0;

	// Needs to be case in-sensitive
	for (int i = 0; i < _nLength; i++) {
		returnValue = returnValue + (char)toupper(_pszData[i]);
	}

	return returnValue;
}

void CBofString::makeUpper() {
	Common::String s(_pszData);
	s.toUppercase();

	strncpy(_pszData, s.c_str(), _nLength);
}

} // namespace Bagel
