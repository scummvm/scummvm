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

#include "bagel/baglib/variable_object.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/var.h"

namespace Bagel {

CBagVariableObject::CBagVariableObject() : CBagObject() {
	_xObjType = VAR_OBJ;
	_nPointSize = 16;
	_nFGColor = CTEXT_COLOR;
	setVisible(true);
	setTimeless(true);
}

CBagVariableObject::~CBagVariableObject() {
	detach();
}

ErrorCode CBagVariableObject::attach() {
	CBagVar *xVar = g_VarManager->getVariable(getFileName());

	if (xVar && !getRefName().isEmpty())
		xVar->setValue(getRefName());

	return CBagObject::attach();
}

ErrorCode CBagVariableObject::detach() {
	return CBagObject::detach();
}

CBofRect CBagVariableObject::getRect() {
	CBofPoint p = getPosition();
	CBofSize s = getSize();
	CBofRect r = CBofRect(p, s);
	return r;
}

//
//   Takes in info and then removes the relative information and returns the info
//   without the relevant info.
ParseCodes CBagVariableObject::setInfo(CBagIfstream &istr) {
	int nChanged;
	bool nObjectUpdated = false;
	char ch;

	while (!istr.eof()) {
		nChanged = 0;

		istr.eatWhite();

		switch (ch = (char)istr.peek()) {
		//
		// SIZE n - n point size of the txt
		//
		case 'S': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);		// jwl 08.28.96 performance improvement

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("SIZE")) {
				istr.eatWhite();
				getIntFromStream(istr, _nPointSize);
				nObjectUpdated = true;
				nChanged++;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		// COLOR n - n color index
		//
		#define		CTEXT_YELLOW		RGB(255,255,0)
		#define     CTEXT_WHITE			RGB(255,255,255)
		case 'C': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);		// jwl 08.28.96 performance improvement

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("COLOR")) {
				int nColor;
				istr.eatWhite();
				getIntFromStream(istr, nColor);
				switch (nColor) {
				case 0:	_nFGColor = RGB(0, 0, 0); break;							// black
				case 1:	_nFGColor = RGB(255, 0, 0); break;
				case 2:	_nFGColor = CTEXT_YELLOW; break;						// yellow
				case 3:	_nFGColor = RGB(0, 255, 0); break;
				case 4:	_nFGColor = RGB(0, 255, 255); break;
				case 5:	_nFGColor = RGB(0, 0, 255); break;
				case 6:	_nFGColor = RGB(255, 0, 255); break;
				case 7:	_nFGColor = CTEXT_WHITE; break;						// white
				default:
					break;
				}
				nObjectUpdated = true;
				nChanged++;
			} else {
				putbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		// No match return from funtion
		//
		default:
			ParseCodes rc;
			if ((rc = CBagObject::setInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else if (!nChanged) {
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
					return UNKNOWN_TOKEN;
			}
			break;
		}
	}

	return PARSING_DONE;
}

ErrorCode CBagVariableObject::update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int) {
	ErrorCode rc = ERR_NONE;
	CBagVar *xVar = g_VarManager->getVariable(getFileName());

	if (isAttached() && xVar && !(xVar->getValue().isEmpty())) {
		// FIXME: Offset for the last accessed time and # times counter in
		// entryway computer terminal. Otherwise, there's no space between
		// them and the preceding text
		Common::String name = xVar->getName().getBuffer();
		if (name.hasSuffix("_LAST") || name.hasSuffix("_TIMES"))
			pt.x += 10;
		// FIXME: Slight spacing out for Fleebix frequency display
		if (name == "NDJAM_INNERDIAL_DISPLAY")
			pt.x -= 5;
		else if (name == "NDJAM_OUTERDIAL_DISPLAY")
			pt.x += 5;

		CBofRect r(pt, pSrcRect->size());
		rc = paintText(pBmp, &r, xVar->getValue(), mapFontPointSize(_nPointSize), TEXT_NORMAL, _nFGColor);

		// Don't need to redraw!
		setDirty(false);
	}
	return rc;
}

ErrorCode CBagVariableObject::update(CBofWindow *pWnd, CBofPoint pt, CBofRect *pSrcRect, int) {
	ErrorCode rc = ERR_NONE;
	CBagVar *xVar = g_VarManager->getVariable(getFileName());

	if (isAttached() && xVar && !(xVar->getValue().isEmpty())) {
		CBofRect r(pt, pSrcRect->size());

		rc = paintText(pWnd, &r, xVar->getValue(), mapFontPointSize(_nPointSize), TEXT_NORMAL, _nFGColor);

		// Don't need to redraw!
		setDirty(false);
	}

	return rc;
}

} // namespace Bagel
