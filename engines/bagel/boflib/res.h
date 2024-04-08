
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

#ifndef BAGEL_BOFLIB_RES_H
#define BAGEL_BOFLIB_RES_H

#include "bagel/boflib/boffo.h"
#include "bagel/boflib/file.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/object.h"

namespace Bagel {

class CResString : public CLList, public CBofObject {
public:
	CResString(INT nId, CHAR *pszString) {
		m_nId = nId;
		m_pszString = pszString;
	}

	INT m_nId;
	CHAR *m_pszString;
};

class CBofStringTable : public CBofFile {
private:
	CResString *m_pStringTable = nullptr;
	byte *m_pBuf = nullptr;
	uint32 m_lBufSize = 0;

protected:
	/**
	 * Loads string table from specified res file
	 * @param pszFileName   Name of file containing resources
	 * @return              Error return code
	 */
	ERROR_CODE Load(const CHAR *pszFileName);

	/**
	 * De-allocates the current Resource String Table
	 */
	void Release();

	/**
	 * Allocates the current Resource String Table
	 * @return      Error return code
	 */
	ERROR_CODE BuildTable();

	/**
	 * De-allocates the current Resource String Table
	 */
	void KillTable();

public:
	/**
	 * Constructor for Boffo Resource String Table
	 * @param pszFileName   Name of file containing resources
	**/
	CBofStringTable(const CHAR *pszFileName);

	/**
	 * Destructor
	 */
	virtual ~CBofStringTable();

	/**
	 * Retrieves the specified resource string
	 * @param nId       Res ID for string to be retrieved
	 */
	const CHAR *GetString(INT nId);
};

} // namespace Bagel

#endif
