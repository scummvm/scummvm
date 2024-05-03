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

#include "bagel/baglib/link_object.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"

namespace Bagel {

extern bool g_noMenuFl;

CBagLinkObject::CBagLinkObject() {
	m_xObjType = LINKOBJ;
	_linkType = LINK;
	_destLocation = CBofPoint(0, 0);
	_srcLocation = CBofPoint(-1, -1);
	_fade = 0;
	SetVisible(false);
	CBagObject::SetOverCursor(5);
}

CBagLinkObject::~CBagLinkObject() {
	CBagObject::detach();
}

CBofRect CBagLinkObject::getRect() {
	CBofPoint p = getPosition();
	CBofSize  s = getSize();
	CBofRect r = CBofRect(p, s);
	return r;
}

PARSE_CODES CBagLinkObject::setInfo(CBagIfstream &istr) {
	bool objectUpdatedFl = false;

	PARSE_CODES parsingCode = PARSING_DONE;
	bool doneFl = false;

	while (!doneFl && !istr.eof()) {
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  @[x,y]  - destination of a flythru.  start point in next world
		//
		case '@': {
			CBofRect r;
			istr.getCh();
			getRectFromStream(istr, r);
			setDstLoc(r.TopLeft());
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
			setSrcLoc(r.TopLeft());
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
			GetAlphaNumFromStream(istr, curString);

			if (!curString.Find("AS")) {
				istr.eatWhite();
				GetAlphaNumFromStream(istr, curString);
				if (!curString.Find("CLOSEUP")) {
					_linkType = CLOSEUP;
					SetOverCursor(2);
					objectUpdatedFl = true;
				} else if (!curString.Find("LINK")) {
					_linkType = LINK;
					objectUpdatedFl = true;
				} else {
					PutbackStringOnStream(istr, curString);
					PutbackStringOnStream(istr, "AS ");
				}
			} else {
				PutbackStringOnStream(istr, curString);
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
			GetAlphaNumFromStream(istr, curString);

			if (!curString.Find("FADE")) {
				istr.eatWhite();
				GetIntFromStream(istr, _fade);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, curString);
			}
			break;
		}

		//
		//  No match return from function
		//
		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);

			if (rc == PARSING_DONE) {
				parsingCode = PARSING_DONE;
				doneFl = true;

			} else if (rc == UPDATED_OBJECT) {
				objectUpdatedFl = true;

			} else { // rc==UNKNOWN_TOKEN
				if (objectUpdatedFl)
					parsingCode = UPDATED_OBJECT;
				else
					parsingCode = UNKNOWN_TOKEN;

				doneFl = true;
			}
			break;
		}
		}
	}

	return parsingCode;
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
	if (getFileName().Find("$LASTWORLD") != -1) {
		curStr = getFileName();

		CBagVar *var = VAR_MANAGER->GetVariable("$LASTWORLD");
		if (var != nullptr) {
			curStr.ReplaceStr("$LASTWORLD", var->GetValue());
		}
	}

	CBagMasterWin::setActiveCursor(6);

	CBagStorageDevWnd *curSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();

	// Set the link position for the storage device we are about to jump to
	CBagStorageDev *destWin = SDEV_MANAGER->GetStorageDevice(curStr);
	if (destWin) {
		destWin->setloadFilePos(getDestLoc());
	}

	CBagMasterWin *pMasterWin = CBagel::getBagApp()->getMasterWnd();
	if (pMasterWin) {
		pMasterWin->setCurrfadeIn(_fade);
		pMasterWin->setStorageDev(curStr);
	}

	CBagStorageDevWnd *otherSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
	if (!curSDev->IsCloseup() && !otherSDev->IsCloseup()) {
		VAR_MANAGER->IncrementTimers();
	}

	return CBagObject::runObject();
}

} // namespace Bagel
