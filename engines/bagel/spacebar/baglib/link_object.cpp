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

#include "bagel/spacebar/baglib/link_object.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/pan_window.h"

namespace Bagel {
namespace SpaceBar {

extern bool g_noMenuFl;

CBagLinkObject::CBagLinkObject() {
	_xObjType = LINK_OBJ;
	_linkType = LINK;
	_destLocation = CBofPoint(0, 0);
	_srcLocation = CBofPoint(-1, -1);
	_fade = 0;

	setVisible(false);
	CBagObject::setOverCursor(5);
}

CBagLinkObject::~CBagLinkObject() {
	CBagObject::detach();
}

CBofRect CBagLinkObject::getRect() {
	const CBofPoint p = getPosition();
	const CBofSize  s = getSize();
	CBofRect r = CBofRect(p, s);
	return r;
}

ParseCodes CBagLinkObject::setInfo(CBagIfstream &istr) {
	bool objectUpdatedFl = false;

	ParseCodes returnCode = PARSING_DONE;
	bool doneFl = false;

	while (!doneFl && !istr.eof()) {
		const char ch = (char)istr.peek();
		switch (ch) {
		//
		//  @[x,y]  - destination of a flythru.  start point in next world
		//
		case '@': {
			CBofRect r;
			istr.getCh();
			getRectFromStream(istr, r);
			setDstLoc(r.topLeft());
			objectUpdatedFl = true;
			break;
		}

		//
		//  #[x,y]  - start location of a flythru.  start point in this world
		//
		case '#': {
			CBofRect r;
			istr.getCh();
			getRectFromStream(istr, r);
			setSrcLoc(r.topLeft());
			objectUpdatedFl = true;
			break;
		}

		//
		//  AS [LINK|CLOSEUP]  - how to run the link
		//
		case 'A': {
			char localBuffer[256];
			localBuffer[0] = 0;
			CBofString curString(localBuffer, 256);
			getAlphaNumFromStream(istr, curString);

			if (!curString.find("AS")) {
				istr.eatWhite();
				getAlphaNumFromStream(istr, curString);
				if (!curString.find("CLOSEUP")) {
					_linkType = CLOSEUP;
					setOverCursor(2);
					objectUpdatedFl = true;
				} else if (!curString.find("LINK")) {
					_linkType = LINK;
					objectUpdatedFl = true;
				} else {
					putbackStringOnStream(istr, curString);
					putbackStringOnStream(istr, "AS ");
				}
			} else {
				putbackStringOnStream(istr, curString);
			}
			break;
		}

		//
		//  FADE n
		//
		case 'F': {
			char localBuffer[256];
			localBuffer[0] = 0;
			CBofString curString(localBuffer, 256);
			getAlphaNumFromStream(istr, curString);

			if (!curString.find("FADE")) {
				istr.eatWhite();
				getIntFromStream(istr, _fade);
				objectUpdatedFl = true;
			} else {
				putbackStringOnStream(istr, curString);
			}
			break;
		}

		//
		//  No match return from function
		//
		default: {
			const ParseCodes parseCode = CBagObject::setInfo(istr);

			if (parseCode == PARSING_DONE) {
				returnCode = PARSING_DONE;
				doneFl = true;

			} else if (parseCode == UPDATED_OBJECT) {
				objectUpdatedFl = true;

			} else { // rc==UNKNOWN_TOKEN
				if (objectUpdatedFl)
					returnCode = UPDATED_OBJECT;
				else
					returnCode = UNKNOWN_TOKEN;

				doneFl = true;
			}
			break;
		}
		}
	}

	return returnCode;
}


bool CBagLinkObject::runObject() {
	// Reset Wield
	g_noMenuFl = false;

	char buffer[256];
	buffer[0] = '\0';
	CBofString curStr(buffer, 256);
	curStr = getFileName();

	// If this is a special link (using the last known sdev stack),
	// then find it's value, and use that instead.
	if (getFileName().find("$LASTWORLD") != -1) {
		curStr = getFileName();

		CBagVar *var = g_VarManager->getVariable("$LASTWORLD");
		if (var != nullptr) {
			curStr.replaceStr("$LASTWORLD", var->getValue());
		}
	}

	CBagMasterWin::setActiveCursor(6);

	CBagStorageDevWnd *curSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();

	// Set the link position for the storage device we are about to jump to
	CBagStorageDev *destWin = g_SDevManager->getStorageDevice(curStr);
	if (destWin) {
		destWin->setloadFilePos(getDestLoc());
	}

	CBagMasterWin *pMasterWin = CBagel::getBagApp()->getMasterWnd();
	if (pMasterWin) {
		pMasterWin->setCurrfadeIn(_fade);
		pMasterWin->setStorageDev(curStr);
	}

	CBagStorageDevWnd *otherSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
	if (!curSDev->isCloseup() && !otherSDev->isCloseup()) {
		g_VarManager->incrementTimers();
	}

	return CBagObject::runObject();
}

} // namespace SpaceBar
} // namespace Bagel
