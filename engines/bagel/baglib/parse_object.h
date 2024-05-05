
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
	int _moveRate;
	int _sprStartIndex;
	int _sprEndIndex;
	int _changeRate;
};

enum PARSE_CODES { PARSING_DONE = 0, UPDATED_OBJECT, UNKNOWN_TOKEN };

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
	static int _nIndentation;
	static bool _bBinaryData;
	bool _bAttached;

public:
	CBagParseObject();
	virtual ~CBagParseObject() {}
	static void initialize();

	virtual ErrorCode attach() {
		_bAttached = true;
		return ERR_NONE;
	}
	virtual ErrorCode detach() {
		_bAttached = false;
		return ERR_NONE;
	}
	virtual bool isAttached() {
		return _bAttached;
	}

	virtual PARSE_CODES setInfo(CBagIfstream &) {
		return PARSING_DONE;
	}

	int setIndent(int n) {
		return _nIndentation = n;
	}
	int getIndent() const {
		return _nIndentation;
	}
	int tab(int n = 2) {
		return _nIndentation += n;
	}
	int unTab(int n = 2) {
		return _nIndentation -= n;
	}

	int setBinaryData(bool b = true) const {
		return _bBinaryData = b;
	}
	int isBinaryData() {
		return _bBinaryData;
	}

	int getStringFromStream(CBagIfstream &istr, CBofString &sStr, const char cEndChar, bool bPutBack = false);
	int getStringFromStream(CBagIfstream &istr, CBofString &sStr, const CBofString &sEndChars, bool bPutBack = false);
	int getAlphaNumFromStream(CBagIfstream &istr, CBofString &sStr);
	int getOperStrFromStream(CBagIfstream &istr, CBofString &sStr);
	int getIntFromStream(CBagIfstream &istr, int &nNum);
	int getRectFromStream(CBagIfstream &istr, CBofRect &rect);
	int getVectorFromStream(CBagIfstream &istr, CBagVector &vector);
	int getKeywordFromStream(CBagIfstream &istr, KEYWORDS &keyword);
	int putbackStringOnStream(CBagIfstream &istr, const CBofString &sStr);

	int parseAlertBox(CBagIfstream &istr, const char *sTitle, const char *sFile, int nLine);

	virtual void *getDataStart() {
		return &_bAttached;
	}
	virtual void *getDataEnd() {
		return &_bAttached + sizeof(bool);
	}
};

} // namespace Bagel

#endif
