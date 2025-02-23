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

#include "bagel/spacebar/baglib/parse_object.h"
#include "bagel/spacebar/boflib/app.h"

namespace Bagel {
namespace SpaceBar {

void CBagParseObject::initialize() {
}

CBagParseObject::CBagParseObject() {
	_bAttached = false;
}

int CBagParseObject::getIntFromStream(CBagIfstream &istr, int &nNum) {
	char ch;
	char szLocalStr[256];
	int i = 0;

	while (Common::isDigit(ch = (char)istr.peek())) {
		ch = (char)istr.getCh();
		szLocalStr[i++] = ch;
		assert(i < 256);
	}

	szLocalStr[i] = 0;
	nNum = atoi(szLocalStr);

	return ch;
}

int CBagParseObject::getStringFromStream(CBagIfstream &istr, CBofString &sStr, const CBofString &sEndChars, bool bPutBack) {
	bool bDone = false;
	char ch = 0;

	sStr = "";

	do {
		if (istr.eof())
			bDone = true;
		else
			ch = (char)istr.getCh();
		for (int i = 0; !bDone && i < sEndChars.getLength(); ++i) {
			if (sEndChars[i] == ch)
				bDone = true;
		}
		if (!bDone)
			sStr += ch;
	} while (!bDone);

	if (bPutBack)
		istr.putBack();

	return ch;
}

int CBagParseObject::getRectFromStream(CBagIfstream &istr, CBofRect &rect) {
	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString str(szLocalStr, 256);

	// The first char must be a [
	char ch = (char)istr.getCh();
	if (ch != '[')
		return -1;

	// Get the left-coord
	getStringFromStream(istr, str, ",");
	rect.left = atoi(str);

	// Get the top-coord
	getStringFromStream(istr, str, "],", true);
	rect.top = atoi(str);

	// If ',' then get the size coords
	ch = (char)istr.getCh();
	if (ch == ',') {
		// Get the right-coord (optional)
		getStringFromStream(istr, str, ",");
		rect.right = atoi(str);

		// Get the bottom-coord (optional)
		getStringFromStream(istr, str, "]");
		rect.bottom = atoi(str);
	} else {
		rect.right = rect.left - 1;
		rect.bottom = rect.top - 1;
	}

	return 0;
}

int CBagParseObject::getAlphaNumFromStream(CBagIfstream &istr, CBofString &sStr) {
	bool bDone = false;
	char ch = 0;

	sStr = "";

	istr.eatWhite();

	do {
		if (istr.eof())
			bDone = true;
		else
			ch = (char)istr.getCh();

		if (Common::isAlnum(ch) || ch == '_' || ch == '-' || ch == '\\' || ch == '/' || ch == '.' || ch == ':' || ch == '$' || ch == 39 || ch == '~') {
			sStr += ch;
		} else {
			bDone = true;
		}

	} while (!bDone);

	istr.putBack();

	return ch;
}

int CBagParseObject::getOperStrFromStream(CBagIfstream &istr, CBofString &sStr) {
	bool bDone = false;
	char ch = 0;

	sStr = "";

	istr.eatWhite();

	do {
		if (istr.eof())
			bDone = true;
		else
			ch = (char)istr.getCh();
		if (Common::isPunct(ch))
			sStr += ch;
		else
			bDone = true;
	} while (!bDone);

	istr.putBack();

	return ch;
}

int CBagParseObject::getKeywordFromStream(CBagIfstream &istr, CBagParseObject::KEYWORDS &keyword) {
	keyword = CBagParseObject::UNKNOWN;
	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString sStr(szLocalStr, 256);

	int rc = getAlphaNumFromStream(istr, sStr);

	if (!sStr.find("SDEV")) {
		keyword = CBagParseObject::STORAGEDEV;

	} else if (!sStr.find("START_WLD")) {
		keyword = CBagParseObject::START_WLD;

	} else if (!sStr.find("VAR")) {
		keyword = CBagParseObject::VARIABLE;

	} else if (!sStr.find("DISKAUDIO")) {
		// Specify the audio that get's played when user is asked to insert another CD.
		keyword = CBagParseObject::DISKAUDIO;

	} else if (!sStr.find("SYSSCREEN")) {
		keyword = CBagParseObject::SYSSCREEN;

	} else if (!sStr.find("DISKID")) {
		keyword = CBagParseObject::DISKID;

	} else if (!sStr.find("SHAREDPAL")) {
		keyword = CBagParseObject::SHAREDPAL; // Allow shared palette at front of script

	} else if (!sStr.find("PDASTATE")) {
		keyword = CBagParseObject::PDASTATE; // allow shared palette at front of script

	} else if (!sStr.find("CURSOR")) {
		keyword = CBagParseObject::CURSOR;

	} else if (!sStr.find("WIELDCURSOR")) {
		keyword = CBagParseObject::WIELDCURSOR;

	} else if (!sStr.find("REM") || !sStr.find("//")) {
		keyword = CBagParseObject::REMARK;
	}

	return rc;
}

int CBagParseObject::putbackStringOnStream(CBagIfstream &istr, const CBofString &sStr) {
	int nLen = sStr.getLength();

	while (nLen > 0) {
		--nLen;
		istr.putBack();
	}

	return 0;
}

int CBagParseObject::parseAlertBox(CBagIfstream &istr, const char *sTitle, const char *sFile, int nLine) {
	char str[256];

	CBofString s = sFile;

	Common::String tmp = Common::String::format("(%d)@%d", nLine, istr.getLineNumber());

	s += tmp.c_str();

	s += ":";
	s += istr.getLineString();

	istr.getLine(str, 255);
	s += str;
	istr.getLine(str, 255);
	s += str;
	istr.getLine(str, 255);
	s += str;

	bofMessageBox(s, sTitle);

	return 0;
}

} // namespace SpaceBar
} // namespace Bagel
