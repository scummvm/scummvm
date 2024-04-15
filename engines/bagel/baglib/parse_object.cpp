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

#include "bagel/baglib/parse_object.h"
#include "bagel/boflib/app.h"

namespace Bagel {

int CBagParseObject::m_nIndentation;

void CBagParseObject::initialize() {
	m_nIndentation = 0;
}

CBagParseObject::CBagParseObject() {
	m_bAttached = false;
#ifdef BAG_DEBUG
	if (!g_fParseLog.is_open()) {
		g_fParseLog.open("BAGELLOG.TXT", ios::out);
	}
#endif
}

int CBagParseObject::GetIntFromStream(bof_ifstream &istr, int &nNum) {
	char ch = 0;
	char szLocalStr[256];
	int i = 0;

	while (Common::isDigit(ch = (char)istr.peek())) {
		ch = (char)istr.Get();
		szLocalStr[i++] = ch;
		Assert(i < 256);
	}

	szLocalStr[i] = 0;
	nNum = atoi(szLocalStr);

	return ch;
}

int CBagParseObject::GetStringFromStream(bof_ifstream &istr, CBofString &sStr, const CBofString &sEndChars, bool bPutBack) {
	bool bDone = false;
	char ch = 0;

	sStr = "";

	do {
		if (istr.eof())
			bDone = true;
		else
			ch = (char)istr.Get();
		for (int i = 0; !bDone && i < sEndChars.GetLength(); ++i) {
			if (sEndChars[i] == ch)
				bDone = true;
		}
		if (!bDone)
			sStr += ch;
	} while (!bDone);

	if (bPutBack)
		istr.putback(ch);

	return ch;
}

int CBagParseObject::GetStringFromStream(bof_ifstream &istr, CBofString &sStr, const char cEndChar, bool bPutBack) {
	return GetStringFromStream(istr, sStr, CBofString(cEndChar), bPutBack);
}

int CBagParseObject::GetRectFromStream(bof_ifstream &istr, CBofRect &rect) {
	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString str(szLocalStr, 256);

	// Tbe first char must be a [
	char ch = (char)istr.Get();
	if (ch != '[')
		return -1;

	// Get the left-coord
	GetStringFromStream(istr, str, ",");
	rect.left = atoi(str);

	// Get the top-coord
	GetStringFromStream(istr, str, "],", true);
	rect.top = atoi(str);

	// If ',' then get the size coords
	ch = (char)istr.Get();
	if (ch == ',') {
		// Get the right-coord (optional)
		GetStringFromStream(istr, str, ",");
		rect.right = atoi(str);

		// Get the bottom-coord (optional)
		GetStringFromStream(istr, str, "]");
		rect.bottom = atoi(str);
	} else {
		rect.right = rect.left - 1;
		rect.bottom = rect.top - 1;
	}

	return 0;
}

int CBagParseObject::GetVectorFromStream(bof_ifstream &istr, CBagVector &vector) {
	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString str(szLocalStr, 256);

	// Tbe first char must be a (
	char ch = (char)istr.Get();
	if (ch != '(')
		return -1;

	// Get the x-coord
	GetStringFromStream(istr, str, ",");
	vector.x = atoi(str);

	// Get the y-coord
	GetStringFromStream(istr, str, ",):@", true);
	vector.y = atoi(str);

	// Vector rate
	vector.nMoveRate = 1;
	ch = (char)istr.peek();
	if (ch == ',') {
		ch = (char)istr.Get();
		GetStringFromStream(istr, str, "):@", true);
		vector.nMoveRate = atoi(str);
	}

	// Start-Stop index
	ch = (char)istr.peek();
	if (ch == '~') {
		ch = (char)istr.Get();
		CBofRect r;
		GetRectFromStream(istr, r);
		vector.nSprStartIndex = r.left;
		vector.nSprEndIndex = r.top;
	}

	// Start-Stop index
	ch = (char)istr.peek();
	if (ch == '@') {
		ch = (char)istr.Get();
		GetStringFromStream(istr, str, ")");
		vector.nChangeRate = atoi(str);
	}

	return 0;
}

int CBagParseObject::GetAlphaNumFromStream(bof_ifstream &istr, CBofString &sStr) {
	bool bDone = false;
	char ch = 0;

	sStr = "";

	istr.EatWhite();

	do {
		if (istr.eof())
			bDone = true;
		else
			ch = (char)istr.Get();

		// if ((lastch == '/') && (ch == '/')) { // put comment back
		//  istr.putback(lastch);
		//  bDone = true;
		// } else
		if (Common::isAlnum(ch) || ch == '_' || ch == '-' || ch == '\\' || ch == '/' || ch == '.' || ch == ':' || ch == '$' || ch == 39 || ch == '~') {
#if BOF_MAC
			if (ch == '\\')
				ch = ':';
#endif
			sStr += ch;
		} else {
			bDone = true;
		}

		// lastch = ch;
	} while (!bDone);

	istr.putback(ch);

	return ch;
}

int CBagParseObject::GetOperStrFromStream(bof_ifstream &istr, CBofString &sStr) {
	bool bDone = false;
	char ch = 0;

	sStr = "";

	istr.EatWhite();

	do {
		if (istr.eof())
			bDone = true;
		else
			ch = (char)istr.Get();
		if (Common::isPunct(ch))
			sStr += ch;
		else
			bDone = true;
	} while (!bDone);

	istr.putback(ch);

	return ch;
}

int CBagParseObject::GetKeywordFromStream(bof_ifstream &istr, CBagParseObject::KEYWORDS &keyword) {
	keyword = CBagParseObject::UNKNOWN;
	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString sStr(szLocalStr, 256);

	int rc = GetAlphaNumFromStream(istr, sStr);

	if (!sStr.Find("SDEV")) {
		keyword = CBagParseObject::STORAGEDEV;

	} else if (!sStr.Find("START_WLD")) {
		keyword = CBagParseObject::START_WLD;

	} else if (!sStr.Find("VAR")) {
		keyword = CBagParseObject::VARIABLE;

	} else if (!sStr.Find("DISKAUDIO")) {
		// Specify the audio that get's played when user is asked to insert another CD.
		keyword = CBagParseObject::DISKAUDIO;

	} else if (!sStr.Find("SYSSCREEN")) {
		keyword = CBagParseObject::SYSSCREEN;

	} else if (!sStr.Find("DISKID")) {
		keyword = CBagParseObject::DISKID;

	} else if (!sStr.Find("SHAREDPAL")) {
		keyword = CBagParseObject::SHAREDPAL; // Allow shared palette at front of script

	} else if (!sStr.Find("PDASTATE")) {
		keyword = CBagParseObject::PDASTATE; // allow shared palette at front of script

	} else if (!sStr.Find("CURSOR")) {
		keyword = CBagParseObject::CURSOR;

	} else if (!sStr.Find("WIELDCURSOR")) {
		keyword = CBagParseObject::WIELDCURSOR;

	} else if (!sStr.Find("REM") || !sStr.Find("//")) {
		keyword = CBagParseObject::REMARK;
#ifdef BOF_DEBUG
	} else {
		keyword = CBagParseObject::UNKNOWN;
#endif
	}

	return rc;
}

int CBagParseObject::PutbackStringOnStream(bof_ifstream &istr, const CBofString &sStr) {
	int nLen = sStr.GetLength();

	while (nLen > 0) {
		--nLen;
		istr.putback(sStr[nLen]);
	}

	return 0;
}

int CBagParseObject::ParseAlertBox(bof_ifstream &istr, const char *sTitle, const char *sFile, int nLine) {
	char str[256];

	CBofString s = sFile;

	Common::String tmp = Common::String::format("(%d)@%d", nLine, istr.GetLineNumber());

	s += tmp.c_str();

	s += ":";
	s += istr.GetLineString();

	istr.getline(str, 255);
	s += str;
	istr.getline(str, 255);
	s += str;
	istr.getline(str, 255);
	s += str;

	BofMessageBox(s, sTitle);

	return 0;
}

} // namespace Bagel
