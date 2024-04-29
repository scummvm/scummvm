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

#include "bagel/baglib/dossier_object.h"
#include "bagel/baglib/rp_object.h"
#include "bagel/baglib/master_win.h"

namespace Bagel {

CBagDossierObject *CBagDossierObject::_curDossier;

void CBagDossierObject::initialize() {
	_curDossier = nullptr;
}

CBagDossierObject::CBagDossierObject() {
	m_xObjType = DOSSIEROBJ;

	_dossierSetFl = false;
	_indexRect.SetRect(0, 0, 0, 0);

	// Keep track of the original text rect.
	_dossierRect.SetRect(0, 0, 0, 0);
	_dosRectInitFl = false;

	setNotActive(false);

	// Ptr to parent rp obj nullptr for now.
	_residuePrintedObject = nullptr;

	// Start with index line displayed
	_showIndexFl = true;
}

CBagDossierObject::~CBagDossierObject() {
}

PARSE_CODES CBagDossierObject::setInfo(bof_ifstream &istr) {
	bool objectUpdatedFl = false;
	char localBuffer[256];
	CBofString sStr(localBuffer, 256);

	while (!istr.eof()) {
		istr.EatWhite();
		char ch = (char)istr.peek();
		switch (ch) {

		//  SIZE n - n point size of the txt
		//
		//  -or-
		//
		// SUSPECTVAR is a variable that will be in the LOGPDA list of
		// objects and will be tightly tied to residue printing.  If a res print
		// yields positive results on a character then set the rp field of the
		// associated suspect object to true.
		case 'S':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SIZE")) {
				istr.EatWhite();
				int size;
				GetIntFromStream(istr, size);
				setPointSize((byte)size);
				objectUpdatedFl = true;
			} else if (!sStr.Find("SUSPECTVAR")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				m_sSuspectVar = sStr;
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

		//
		//  NOTACTIVE KEYWORD MEANS DON'T DO ANYTHING ON MOUSE DOWN!!!
		//
		case 'N':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("NOTACTIVE")) {
				setNotActive(true);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

		//
		//  FONT MONO or DEFAULT
		//
		case 'F':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("FONT")) {
				istr.EatWhite();
				int fontId;
				GetIntFromStream(istr, fontId);
				fontId = MapFont(fontId);
				setFont(fontId);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

		//
		//  INDEX line, required.  This is the line that will show up in the
		//  log entry.
		//
		case 'I':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("INDEX")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);

				// replace underlines with spaces.

				sStr.ReplaceChar('_', ' ');
				_indexLine = sStr;

				objectUpdatedFl = true;

				// If the next character is a '[' then we got another rect coming
				// along.  This rect is for the index string.

				istr.EatWhite();
				if ((char)istr.peek() == '[') {
					CBofRect r;
					GetRectFromStream(istr, r);
					_indexRect = r;
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

		//
		//  AS [CAPTION]  - how to run the link
		case 'A':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				if (!sStr.Find("CAPTION")) {
					m_bCaption = true;
					objectUpdatedFl = true;

				} else if (!sStr.Find("TITLE")) {
					m_bTitle = true;
					objectUpdatedFl = true;

				} else {
					PutbackStringOnStream(istr, sStr);
					PutbackStringOnStream(istr, "AS ");
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

			//
			//  COLOR n - n color index
			//
#define CTEXT_YELLOW RGB(255, 255, 0)
#define CTEXT_WHITE RGB(255, 255, 255)
		case 'C':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("COLOR")) {
				int nColor;
				istr.EatWhite();
				GetIntFromStream(istr, nColor);
				SetColor(nColor);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		//
		//  no match return from function
		//
		default:
			PARSE_CODES rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				objectUpdatedFl = true;
			} else { // rc==UNKNOWN_TOKEN
				if (objectUpdatedFl)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
	}

	return PARSING_DONE;
}

// Implement attach and detach just so we can set our own attributes

ErrorCode CBagDossierObject::attach() {
	ErrorCode ec = CBagTextObject::attach();

	// Keep track of the original text rectangle (for the dossier).
	if (_dosRectInitFl == false) {
		_dossierRect = CBagTextObject::getRect();
		_dosRectInitFl = true;
	}

	SetVisible(false); // Don't display until needed.
	SetActive(false);  // Not active until we need it.
	return ec;
}

ErrorCode CBagDossierObject::detach() {
	ErrorCode ec = CBagTextObject::detach();

	SetVisible(false); // Make this invisible, don't want it redrawn.
	return ec;
}

// Called to splash one of these guys to the screen
ErrorCode CBagDossierObject::update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect, int n) {
	if (_showIndexFl) {
		if (_dossierSetFl == false) {
			// Set the text to be the index line.
			SetPSText(&_indexLine);
			_dossierSetFl = true;
		}
	} else if (_dossierSetFl == false) {
		// Set the text to be nullptr, this forces the bagtx code to
		// paint the text file.
		SetPSText(nullptr);

		CBagTextObject::attach();

		_dossierSetFl = true;
	}

	return CBagTextObject::update(bmp, pt, srcRect, n);
}

CBofRect CBagDossierObject::getRect() {
	CBofRect rect;
	CBofPoint pos = GetPosition();

	if (_showIndexFl) {
		rect = _indexRect;
		rect.OffsetRect(pos);
	} else {
		rect = _dossierRect;
	}

	return rect;
}

void CBagDossierObject::onLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * /*pInfo*/) {
	// If this is an inactive object (such as "plus residue too old" or "negative")
	// then ignore the mousedown.

	if (getNotActive()) {
		return;
	}

	showDossierText();
}

// utility routine for activating dossier objects
void CBagDossierObject::activateDossierObject(CBagLog *logWld) {
	if (logWld) {
		logWld->ActivateLocalObject(this);
	}

	if (CBagRPObject::GetLogState() == RP_READ_DOSSIER) {
		SetFloating(false); // This is not floating
	} else {
		SetFloating(); // This is not floating
	}

	SetVisible(); // can see this thing now...
	SetActive();  // make sure it counts for something

	// Starting state will ALWAYS be index first
	_dossierSetFl = false;
	_showIndexFl = true;
}

void CBagDossierObject::deactivateDossierObject(CBagLog *logWld) {
	if (CBagRPObject::GetLogState() == RP_READ_DOSSIER) {
		SetFloating(false); // This is not floating
	} else {
		SetFloating(); // This is not floating
	}

	SetVisible(false); // Cover it up
	SetActive(false);  // Make sure it counts for something
	SetLocal(false);   // Not local anymore
}

// Called from script to deactivate a dossier view screen.
void CBagDossierObject::deactivateCurDossier() {
	// If we have a current dossier, then turn it off and activate the
	// previous rp object.
	if (_curDossier != nullptr) {
		CBagLog *logWld = nullptr;
		CBagRPObject *residuePrintedObj = ((CBagRPObject *)(_curDossier->_residuePrintedObject));

		if (residuePrintedObj) {
			if (residuePrintedObj->Zoomed()) {
				logWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
			} else {
				logWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
			}
		}
		_curDossier->deactivateDossierObject(logWld);

		// When we hit the update code, we're gonna need to reevaluate this
		// guys current view status.
		_curDossier->_showIndexFl = true;
		_curDossier->_dossierSetFl = false;

		((CBagRPObject *)(_curDossier->_residuePrintedObject))->ActivateRPObject();

		_curDossier = nullptr;
	}
}

void CBagDossierObject::showDossierText() {
	// Make sure our script knows where we're going with this.
	CBagRPObject::SetLogState(RP_READ_DOSSIER);

	// We got a mouse down on one of our dossier's, so now we have to deactivate
	// everything that is in the residue print object that this dossier is
	// contained in.
	bool zoomedFl = false;
	if (_residuePrintedObject) {
		zoomedFl = ((CBagRPObject *)_residuePrintedObject)->Zoomed();
		((CBagRPObject *)_residuePrintedObject)->DeactivateRPObject();
	}

	// Get the appropriate storage device

	CBagLog *logWld;
	if (zoomedFl) {
		logWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
	} else {
		logWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	}

	// Now reactivate this object.
	activateDossierObject(logWld);

	// Mark this one as our current dossier
	_curDossier = this;

	// Make sure the text file is displayed, not the index line.  See the
	// update code to see why this makes this happen.
	_dossierSetFl = false;
	_showIndexFl = false;

	// Let the residue printing object know which one we're displaying.

	CBagRPObject *pRPObj = (CBagRPObject *)_residuePrintedObject;
	pRPObj->SetActiveDossier(this);
}

} // namespace Bagel
