
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

#include "bagel/spacebar/baglib/ifstream.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/string.h"

namespace Bagel {
namespace SpaceBar {

#define RECT_START_DELIM '['
#define RECT_END_DELIM ']'
#define RECT_VAR_DELIM ','

#define SDEV_START_DELIM '{'
#define SDEV_END_DELIM '}'

#define OBJ_START_DELIM '='
#define OBJ_END_DELIM ';'

enum ParseCodes {
	PARSING_DONE = 0, UPDATED_OBJECT, UNKNOWN_TOKEN
};

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
	bool _bAttached;

public:
	CBagParseObject();
	virtual ~CBagParseObject() {
	}
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

	virtual ParseCodes setInfo(CBagIfstream &) {
		return PARSING_DONE;
	}

	int getStringFromStream(CBagIfstream &istr, CBofString &sStr, const CBofString &sEndChars, bool bPutBack = false);
	int getAlphaNumFromStream(CBagIfstream &istr, CBofString &sStr);
	int getOperStrFromStream(CBagIfstream &istr, CBofString &sStr);
	int getIntFromStream(CBagIfstream &istr, int &nNum);
	int getRectFromStream(CBagIfstream &istr, CBofRect &rect);
	int getKeywordFromStream(CBagIfstream &istr, KEYWORDS &keyword);
	int putbackStringOnStream(CBagIfstream &istr, const CBofString &sStr);

	int parseAlertBox(CBagIfstream &istr, const char *sTitle, const char *sFile, int nLine);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
