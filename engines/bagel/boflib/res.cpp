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

#include "bagel/boflib/res.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

CBofStringTable::CBofStringTable(const CHAR *pszFileName) : CBofFile(nullptr) {
	Assert(pszFileName != nullptr);

	Load(pszFileName);
}

CBofStringTable::~CBofStringTable() {
	Assert(IsValidObject(this));

	Release();
}

ERROR_CODE CBofStringTable::Load(const CHAR *pszFileName) {
	Assert(IsValidObject(this));

	// Deallocate any previous data
	Release();

	// Open this string file
	Open(pszFileName);

	m_lBufSize = GetLength();

	Assert(m_lBufSize > 0);

	// Allocate a buffer to hold entire file
	if ((m_pBuf = (UBYTE *)BofAlloc(m_lBufSize + 1)) != nullptr) {
		BofMemSet(m_pBuf, 0, m_lBufSize + 1);

		// Read in entire file
		Read(m_pBuf, m_lBufSize);

		BuildTable();

	} else {
		ReportError(ERR_MEMORY, "Unable to allocate %ld bytes for String Table", m_lBufSize);
	}

	// Don't need this file open anymore
	Close();

	return m_errCode;
}

VOID CBofStringTable::Release() {
	Assert(IsValidObject(this));

	KillTable();

	if (m_pBuf != nullptr) {
		BofFree(m_pBuf);
		m_pBuf = nullptr;
	}
}

VOID CBofStringTable::KillTable() {
	Assert(IsValidObject(this));

	CResString *pString, *pNextString;

	pString = m_pStringTable;

	while (pString != nullptr) {
		pNextString = (CResString *)pString->GetNext();
		delete pString;

		pString = pNextString;
	}

	m_pStringTable = nullptr;
}

ERROR_CODE CBofStringTable::BuildTable() {
	Assert(IsValidObject(this));

	// Deallocate any previous table
	KillTable();

	Assert(m_pStringTable == nullptr);
	Assert(m_pBuf != nullptr);

	CResString *pString;
	UBYTE *pBuf;
	INT nId;

	MemReplaceChar(m_pBuf, '\r', '\0', m_lBufSize);
	MemReplaceChar(m_pBuf, '\n', '\0', m_lBufSize);
	pBuf = m_pBuf;

	while (pBuf < m_pBuf + m_lBufSize) {
		nId = atoi((const CHAR *)pBuf);
		pBuf = (UBYTE *)strchr((const CHAR *)pBuf, '=');
		pBuf++;

		if ((pString = new CResString(nId, (CHAR *)pBuf)) != nullptr) {
			// Add this string to the table
			if (m_pStringTable == nullptr) {
				m_pStringTable = pString;
			} else {
				m_pStringTable->AddToTail(pString);
			}
		} else {
			ReportError(ERR_MEMORY, "Unable to allocate a CResString");
			break;
		}

		while (*pBuf++ != '\0') {
			if (pBuf > m_pBuf + m_lBufSize)
				break;
		}

		while (*pBuf == '\0') {
			pBuf++;
			if (pBuf > m_pBuf + m_lBufSize)
				break;
		}
	}

	return (m_errCode);
}

const CHAR *CBofStringTable::GetString(INT nId) {
	Assert(IsValidObject(this));

	CResString *pCurString = m_pStringTable;
	const CHAR *pszString = nullptr;

	while (pCurString != nullptr) {
		if (pCurString->m_nId == nId) {
			pszString = (const CHAR *)pCurString->m_pszString;
			break;
		}

		pCurString = (CResString *)pCurString->GetNext();
	}

	if (pCurString == nullptr) {
		LogWarning(BuildString("Resource String %d not found in %s", nId, m_szFileName));
	}

	return pszString;
}

} // namespace Bagel
