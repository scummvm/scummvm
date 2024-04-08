
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

#ifndef BAGEL_BAGLIB_PARSE_OBJECT_H
#define BAGEL_BAGLIB_PARSE_OBJECT_H

#include "bagel/baglib/ifstream.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/vector.h"

namespace Bagel {

#define RECT_START_DELIM '['
#define RECT_END_DELIM ']'
#define RECT_VAR_DELIM ','

#define SDEV_START_DELIM '{'
#define SDEV_END_DELIM '}'

#define OBJ_START_DELIM '='
#define OBJ_END_DELIM ';'

// Support Vector Class
class CBagVector : public CVector {
public:
	int nMoveRate;
	int nSprStartIndex;
	int nSprEndIndex;
	int nChangeRate;
};

enum PARSE_CODES { PARSING_DONE = 0, UPDATED_OBJECT, UNKNOWN_TOKEN };

#ifdef BAG_DEBUG
static ofstream g_fParseLog;
#endif

class CBagParseObject {
public:
	enum KEYWORDS {
		UNKNOWN = 0,
		STORAGEDEV,
		START_WLD,
		VARIABLE,
		REMARK,
		DISKID,
		CURSOR,
		SHAREDPAL,
		DISKAUDIO,
		PDASTATE,
		SYSSCREEN,
		WIELDCURSOR
	};

private:
	static int m_nIndentation;
	static bool m_bBinaryData;
	bool m_bAttached;

public:
	CBagParseObject();
	virtual ~CBagParseObject() {}
	static void initialize();

	virtual ERROR_CODE Attach() {
		m_bAttached = true;
		return ERR_NONE;
	}
	virtual ERROR_CODE Detach() {
		m_bAttached = FALSE;
		return ERR_NONE;
	}
	virtual bool IsAttached() {
		return m_bAttached;
	}

	virtual PARSE_CODES SetInfo(bof_ifstream &) {
		return PARSING_DONE;
	}

	int SetIndent(int n) {
		return m_nIndentation = n;
	}
	int GetIndent() const {
		return m_nIndentation;
	}
	int Tab(int n = 2) {
		return m_nIndentation += n;
	}
	int UnTab(int n = 2) {
		return m_nIndentation -= n;
	}

	int SetBinaryData(bool b = true) const {
		return m_bBinaryData = b;
	}
	int IsBinaryData() {
		return m_bBinaryData;
	}

	int GetStringFromStream(bof_ifstream &istr, CBofString &sStr, const char cEndChar, bool bPutBack = FALSE);
	int GetStringFromStream(bof_ifstream &istr, CBofString &sStr, const CBofString &sEndChars, bool bPutBack = FALSE);
	int GetAlphaNumFromStream(bof_ifstream &istr, CBofString &sStr);
	int GetOperStrFromStream(bof_ifstream &istr, CBofString &sStr);
	int GetIntFromStream(bof_ifstream &istr, int &nNum);
	int GetRectFromStream(bof_ifstream &istr, CBofRect &rect);
	int GetVectorFromStream(bof_ifstream &istr, CBagVector &vector);
	int GetKeywordFromStream(bof_ifstream &istr, KEYWORDS &keyword);
	int PutbackStringOnStream(bof_ifstream &istr, const CBofString &sStr);

	int ParseAlertBox(bof_ifstream &istr, const char *sTitle, const char *sFile, int nLine);

	virtual void *GetDataStart() {
		return &m_bAttached;
	}
	virtual void *GetDataEnd() {
		return &m_bAttached + sizeof(bool);
	}
};

} // namespace Bagel

#endif
