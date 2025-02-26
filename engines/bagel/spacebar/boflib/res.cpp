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

#include "bagel/spacebar/boflib/res.h"
#include "bagel/spacebar/boflib/debug.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/string_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {
namespace SpaceBar {

CBofStringTable::CBofStringTable(const char *pszFileName) : CBofFile(nullptr) {
	assert(pszFileName != nullptr);

	load(pszFileName);
}

CBofStringTable::~CBofStringTable() {
	assert(isValidObject(this));

	release();
}

ErrorCode CBofStringTable::load(const char *pszFileName) {
	assert(isValidObject(this));

	// Deallocate any previous data
	release();

	// Open this string file
	open(pszFileName);

	_lBufSize = getLength();

	assert(_lBufSize > 0);

	// Allocate a buffer to hold entire file and fill it with 0s
	_pBuf = (byte *)bofCleanAlloc(_lBufSize + 1);

	// Read in entire file
	read(_pBuf, _lBufSize);
	buildTable();

	// Don't need this file open anymore
	close();

	return _errCode;
}

void CBofStringTable::release() {
	assert(isValidObject(this));

	killTable();

	if (_pBuf != nullptr) {
		bofFree(_pBuf);
		_pBuf = nullptr;
	}
}

void CBofStringTable::killTable() {
	assert(isValidObject(this));

	CResString *pString = _pStringTable;
	while (pString != nullptr) {
		CResString *pNextString = (CResString *)pString->getNext();
		delete pString;

		pString = pNextString;
	}

	_pStringTable = nullptr;
}

ErrorCode CBofStringTable::buildTable() {
	assert(isValidObject(this));

	// Deallocate any previous table
	killTable();

	assert(_pStringTable == nullptr);
	assert(_pBuf != nullptr);

	memreplaceChar(_pBuf, '\r', '\0', _lBufSize);
	memreplaceChar(_pBuf, '\n', '\0', _lBufSize);
	const byte *pBuf = _pBuf;

	while (pBuf < _pBuf + _lBufSize) {
		int nId = atoi((const char *)pBuf);
		pBuf = (const byte *)strchr((const char *)pBuf, '=');
		if (pBuf == nullptr) {
			reportError(ERR_NONE, "Parsing error in buildTable()");
			break;
		}
		pBuf++;

		CResString *pString = new CResString(nId, (const char *)pBuf);

		// Add this string to the table
		if (_pStringTable == nullptr) {
			_pStringTable = pString;
		} else {
			_pStringTable->addToTail(pString);
		}

		while (*pBuf++ != '\0') {
			if (pBuf > _pBuf + _lBufSize)
				break;
		}

		while (*pBuf == '\0') {
			pBuf++;
			if (pBuf > _pBuf + _lBufSize)
				break;
		}
	}

	return _errCode;
}

const char *CBofStringTable::getString(int nId) {
	assert(isValidObject(this));

	CResString *pCurString = _pStringTable;
	const char *pszString = nullptr;

	while (pCurString != nullptr) {
		if (pCurString->_nId == nId) {
			pszString = (const char *)pCurString->_pszString;
			break;
		}

		pCurString = (CResString *)pCurString->getNext();
	}

	if (pCurString == nullptr) {
		logWarning(buildString("Resource String %d not found in %s", nId, _szFileName));
	}

	return pszString;
}

} // namespace SpaceBar
} // namespace Bagel
