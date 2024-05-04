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

#include "bagel/baglib/log_msg.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/gui/movie.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pda.h"
#include "bagel/boflib/gfx/text.h"
#include "bagel/boflib/log.h"

namespace Bagel {

CBagLog *CBagLog::_lastFloatPage;

#define LOG_BORDER (42)
#define LOG_Z_BORDER (84)

#define OVERRIDE_SMK             "$SBARDIR\\BAR\\LOG\\OVERRIDE.SMK"
#define OVERRIDE_MOVIE           "OVERRIDE_MOVIE"

CBagLog::CBagLog() : CBagStorageDevBmp() {
	_queuedMsgList = new CBofList<CBagObject *>;
	setCurFltPage(1);
}

CBagLog::~CBagLog() {
	if (_queuedMsgList != nullptr) {
		releaseMsg();       // Delete all master sprite objects
		delete _queuedMsgList;
		_queuedMsgList = nullptr;
	}
}

CBofPoint CBagLog::arrangeFloater(CBofPoint pos, CBagObject *bagObj) {
	CBofPoint   nextPos = pos;

	// Things are so convoluted now, it is entirely  possible that this method
	// will get called on a storage device that is not the current one,
	// so we will not require a backdrop.

	if (getBackground() != nullptr) {
		CBofString sdevName = GetName();
		int borderSize = 0;

		// Get this from script, allows individual log states to set border.
		CBagVar *curVar = VAR_MANAGER->GetVariable("LOGBORDER");
		if (curVar != nullptr) {
			borderSize = curVar->GetNumValue();
		}

		if (borderSize == 0) {
			if (sdevName == "LOG_WLD") {
				borderSize = LOG_BORDER;
			} else if (sdevName == "LOGZ_WLD") {
				borderSize = LOG_Z_BORDER;
			}
		}

		// The log window has instructional text at the top and bottom, so
		// create a floater rect to stay in the middle area
		CBofRect floatRect = getBackground()->getRect();
		floatRect.top += borderSize;
		floatRect.bottom -= (borderSize / 2);

		// calculate what page the whole object belongs on
		int pageNum = ((nextPos.y + bagObj->getRect().height()) / floatRect.height());
		// page numbering is 1-N
		pageNum++;
		SetNumFloatPages(pageNum);

		int totalPages = getCurFltPage();
		// Now position this object int the sdev
		// if it fell on this page, show it
		if (pageNum == totalPages) {
			CBofPoint pagePos = nextPos;
			// Bring the current page into view
			pagePos.y = pagePos.y - ((pageNum - 1) * floatRect.height());
			// Add in the border
			pagePos.y += borderSize;
			bagObj->setPosition(pagePos);
		} else {
			// Set the position to be off the sdev, so it won't show
			bagObj->setPosition(CBofPoint(nextPos.x, getBackground()->height() + 1));
		}

		// Calculate the position for the next floater
		// This will get sent back to the calling func
		nextPos.x += bagObj->getRect().width();

		// Check to see if the whole object can fit in width, if it can't wrap
		if (nextPos.x > (floatRect.width() - bagObj->getRect().width())) {
			nextPos.x = 0;
			nextPos.y += bagObj->getRect().height();
		}
	}

	_lastFloatPage = this;
	return nextPos;
}

void CBagLog::arrangePages() {
	// Don't bother if we don't have a floater worth arranging...
	if (_lastFloatPage == nullptr) {
		return;
	}

	CBagLog *lastFloat = _lastFloatPage;

	// Get the up button and the down button...
	CBagObject *upObj = lastFloat->GetObject("LOGPAGUP");
	CBagObject *downObj = lastFloat->GetObject("LOGPAGDOWN");

	if (upObj == nullptr || downObj == nullptr) {
		return;
	}

	// Get current page number and last page number
	int lastPage = lastFloat->GetNumFloatPages();
	int curPage = lastFloat->getCurFltPage();
	int firstPage = 1;

	if (curPage > firstPage && curPage < lastPage) {
		if (upObj->isAttached() == false) {
			upObj->setActive();
			upObj->attach();
		}
		if (downObj->isAttached() == false) {
			downObj->setActive();
			downObj->attach();
		}
	} else if (curPage == firstPage && curPage == lastPage) {
		if (upObj->isAttached()) {
			upObj->setActive(false);
			upObj->detach();
		}
		if (downObj->isAttached()) {
			downObj->setActive(false);
			downObj->detach();
		}
	} else if (curPage <= firstPage) {
		if (upObj->isAttached()) {
			upObj->setActive(false);
			upObj->detach();
		}
		if (downObj->isAttached() == false) {
			downObj->setActive();
			downObj->attach();
		}
	} else if (curPage >= lastPage) {
		if (upObj->isAttached() == false) {
			upObj->setActive();
			upObj->attach();
		}

		if (downObj->isAttached()) {
			downObj->setActive(false);
			downObj->detach();
		}
	}

	// Reinitialize
	_lastFloatPage = nullptr;
}

int CBagLog::getCurFltPage() {
	int nCurFltPage = 0;

	// Read in their total nuggets from game
	CBagVar *curVar = VAR_MANAGER->GetVariable("CUR_BAR_LOG_PAGE");

	if (curVar) {
		nCurFltPage = curVar->GetNumValue();
	}

	return nCurFltPage;
}

void CBagLog::setCurFltPage(int fltPage) {
	// Read in their total nuggets from game
	CBagVar *curVar = VAR_MANAGER->GetVariable("CUR_BAR_LOG_PAGE");

	if (curVar)
		curVar->SetValue(fltPage);
}

ErrorCode CBagLog::releaseMsg() {
	ErrorCode errCode = ERR_NONE;
	int count = _queuedMsgList->getCount();

	for (int i = 0; i < count; ++i) {
		CBagObject *curObj = _queuedMsgList->removeHead();

		// This is waiting to be played, mark it in memory as such, the fixes
		// get uglier and uglier... since zoomed pda doesn't have a message light,
		// only set this thing as waiting if we are in the regular PDA,
		// otherwise, we get superfluous blinking of the PDA light.
		CBofString  devName = GetName();
		if (devName == "LOG_WLD") {
			curObj->SetMsgWaiting(true);
		}
	}

	_queuedMsgList->removeAll();
	return errCode;
}

CBagObject *CBagLog::onNewUserObject(const CBofString &initStr) {
	CBagTextObject *retLogObj = nullptr;
	CBofRect sdevRect = getRect();
	CBofString  sdevName = GetName();
	int     pointSize = 10;

	if (sdevName == "LOG_WLD")
		pointSize = FONT_8POINT;
	else if (sdevName == "LOGZ_WLD")
		pointSize = FONT_18POINT;

	if (initStr == "MSG") {
		retLogObj = (CBagTextObject *)new CBagLogMsg(sdevRect.width());
		retLogObj->SetInitInfo(initStr);
		retLogObj->setPointSize(pointSize);
		retLogObj->setColor(7);
		retLogObj->SetFloating();
	} else if (initStr == "SUS") {
		retLogObj = (CBagTextObject *)new CBagLogSuspect(sdevRect.width());
		retLogObj->SetInitInfo(initStr);

		// Reduce point size on zoompda suspect list, make it
		// all fit in the zoompda window.
		if (pointSize == FONT_18POINT) {
			pointSize -= 2;
		}
		retLogObj->setPointSize(pointSize);
		retLogObj->setColor(7);
		retLogObj->SetFloating();
	} else if (initStr == "CLU") {
		retLogObj = (CBagTextObject *)new CBagLogClue(initStr, sdevRect.width(), pointSize);
	} else if (initStr == "RES") {
		retLogObj = (CBagTextObject *)new CBagLogResidue(sdevRect.width());
		retLogObj->SetInitInfo(initStr);
		retLogObj->setPointSize(pointSize);
		retLogObj->setColor(7);
		retLogObj->SetFloating();
	}

	return retLogObj;
}

bool CBagLog::removeFromMsgQueue(CBagObject *deletedObj) {
	bool removedFl = false;
	int count = _queuedMsgList->getCount();

	for (int i = 0; i < count; i++) {
		CBagObject *curObj = _queuedMsgList->getNodeItem(i);

		if (curObj == deletedObj) {
			_queuedMsgList->remove(i);
			removedFl = true;
			break;
		}
	}

	return removedFl;
}

ErrorCode CBagLog::activateLocalObject(CBagObject *bagObj) {
	ErrorCode errCode = ERR_NONE;

	if (bagObj == nullptr)
		return errCode;

	if (bagObj->IsMsgWaiting() ||
	        (bagObj->GetType() == USEROBJ && (bagObj->GetInitInfo() != nullptr) && (*bagObj->GetInitInfo() == "MSG"))) {
		_queuedMsgList->addToTail(bagObj);

		// Since zoomed pda doesn't  have a message light, only set this thing
		// as waiting if we are in the  regular PDA, otherwise, we get superfluous
		// blinking of the PDA light.
		CBofString  sdevName = GetName();
		if (sdevName == "LOG_WLD") {
			bagObj->SetMsgWaiting(true);
		}

		CBagStorageDev *pda = SDEV_MANAGER->GetStorageDevice("BPDA_WLD");

		if (pda) {
			CBagButtonObject *msgLight = (CBagButtonObject *)pda->GetObject("MSGLIGHT");

			if (msgLight) {
				if (!msgLight->isAttached()) {
					msgLight->setActive();
					msgLight->attach();
				}

				// Make sure this guy always gets updated regardless of its
				// dirty bit.
				msgLight->SetAlwaysUpdate(true);
				msgLight->setAnimated(true);
			}
		}
	} else {
		errCode = CBagStorageDev::activateLocalObject(bagObj);
	}

	return errCode;
}

ErrorCode CBagLog::playMsgQueue() {
	ErrorCode errCode = ERR_NONE;
	int count = _queuedMsgList->getCount();

	// Walk through the message queue and play all the messages
	// Only play one message per click on the pda message light.
	if (count) {

		CBagStorageDev *bpda = SDEV_MANAGER->GetStorageDevice("BPDA_WLD");

		// If we're in a closeup, then don't play the message!
		CBagStorageDev *sdev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
		bool playMsgFl = true;

		if ((sdev != nullptr) && sdev->IsCIC()) {
			playMsgFl = false;

			char localBuffer[256];
			CBofString smkName(localBuffer, 256);

			smkName = OVERRIDE_SMK;
			MACROREPLACE(smkName);

			CBagMovieObject *pMovie = (CBagMovieObject *)GetObject(OVERRIDE_MOVIE);
			if (pMovie) {
				if (pMovie->isAttached() == false) {
					pMovie->attach();
					pMovie->SetVisible();
				}
				pMovie->runObject();
			}
		}

		// If we're playing a valid message (not the override message) then make sure
		// we remove it from the queue.
		if (playMsgFl) {
			CBagObject *curObj = _queuedMsgList->removeHead();

			if (curObj) {
				CRect  r = getRect();

				errCode = CBagStorageDev::activateLocalObject(curObj);
				CBagMenu *objMenu = curObj->getMenuPtr();
				if (objMenu)
					objMenu->TrackPopupMenu(0, 0, 0, CBofApp::getApp()->getMainWindow(), nullptr, &r);
				curObj->runObject();
				curObj->SetMsgWaiting(false);

				// Mark this guy as played...
				((CBagLogMsg *)curObj)->setMsgPlayed(true);
			}

			// Although this might seem like a superfluous thing to do, but wait!
			// it is not!  the runObject call above can cause the number of objects in the
			// message queue to be decremented.
			count = _queuedMsgList->getCount();

			// Don't stop message light from blinking unless we're down to zero
			// messages in the queue.

			if (bpda) {
				CBagButtonObject *pMsgLight = (CBagButtonObject *)bpda->GetObject("MSGLIGHT");

				if (pMsgLight) {
					if (count) {
						pMsgLight->setAnimated(true);
					} else {
						pMsgLight->setAnimated(false);
					}
				}
			}
		}
	}

	return errCode;
}

CBagLogResidue::CBagLogResidue(int sdevWidth) : CBagTextObject() {
	m_xObjType = USEROBJ;
	_sdevWidth = sdevWidth;
	m_bTitle = true;
}

void CBagLogResidue::setSize(const CBofSize &size) {
	CBagTextObject::setSize(CBofSize(_sdevWidth, size.cy));
}

CBagLogMsg::CBagLogMsg(int sdevWidth) : CBagTextObject() {
	m_xObjType = USEROBJ;
	_sdevWidth = sdevWidth;
	m_bTitle = true;

	// Start all messages off as not played
	setMsgPlayed(false);
}

void CBagLogMsg::setSize(const CBofSize &size) {
	CBagTextObject::setSize(CBofSize(_sdevWidth, size.cy));
}

PARSE_CODES CBagLogMsg::setInfo(CBagIfstream &istr) {
	bool nObjectUpdated = false;
	char localBuffer[256];
	CBofString string1(localBuffer, 256);

	char localBuffer2[256];
	CBofString string2(localBuffer2, 256);

	while (!istr.eof()) {
		istr.eatWhite();
		
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  SENDEE FRANK - Sets the sendee name of the message to FRANK
		//
		case 'S': {
			GetAlphaNumFromStream(istr, string1);

			if (!string1.Find("SENDEE")) {
				istr.eatWhite();
				GetAlphaNumFromStream(istr, string2);

				// Replace any underscores with spaces
				string2.ReplaceChar('_', ' ');

				setMsgSendee(string2);

				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, string1);
			}
			break;
		}

		//
		//   TIME x- Sets the time of the message to xx:xx
		//
		case 'T': {
			GetAlphaNumFromStream(istr, string1);

			if (!string1.Find("TIME")) {
				istr.eatWhite();
				char nextCh = (char)istr.peek();
				int msgTime = 0;
				if (Common::isDigit(nextCh)) {
					GetIntFromStream(istr, msgTime);
				} else {
					GetAlphaNumFromStream(istr, _msgTimeStr);
				}

				setMsgTime(msgTime);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, string1);
			}
			break;
		}

		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

void CBagLogMsg::setProperty(const CBofString &prop, int val) {
	if (!prop.Find("TIME")) {
		setMsgTime(val);
	} else if (!prop.Find("PLAYED")) {
		setMsgPlayed(val);
	}

	CBagObject::setProperty(prop, val);
}

int CBagLogMsg::getProperty(const CBofString &prop) {
	if (!prop.Find("TIME"))
		return getMsgTime();

	// Played requires a 1 or a 0 (don't use true or false).
	if (!prop.Find("PLAYED")) {
		bool playedFl = getMsgPlayed();
		return (playedFl ? 1 : 0);
	}

	return CBagObject::getProperty(prop);
}

ErrorCode CBagLogMsg::update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect, int maskColor) {
	// We could use a variable here, translate it's value if that's the case.
	if (getMsgTime() == 0) {
		CBagVar *pVar = VAR_MANAGER->GetVariable(_msgTimeStr);
		int nMsgTime = pVar->GetNumValue();
		setMsgTime(nMsgTime);
	}

	int msgTime = getMsgTime();
	int hour = msgTime / 100;
	int minutes = msgTime - (hour * 100);

	setFont(FONT_MONO);
	setText(buildString("%-30s%02d:%02d", _msgSendee.GetBuffer(), hour, minutes));

	return CBagTextObject::update(bmp, pt, srcRect, maskColor);
}

ErrorCode CBagLogResidue::update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect, int maskColor) {
	return CBagTextObject::update(bmp, pt, srcRect, maskColor);
}

CBagLogSuspect::CBagLogSuspect(int sdevWidth) : CBagTextObject() {
	m_xObjType = USEROBJ;
	m_nSdevWidth = sdevWidth;

	m_bTitle = true;

	// Need to save state info, set all to false.
	CBagObject::setState(0);
}

PARSE_CODES CBagLogSuspect::setInfo(CBagIfstream &istr) {
	bool objectUpdatedFl = false;
	char localBuffer[256];
	CBofString string1(localBuffer, 256);

	char localBuffer2[256];
	CBofString string2(localBuffer2, 256);

	while (!istr.eof()) {
		istr.eatWhite();

		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  NAME FRANK - Sets the sendee name of the message to FRANK
		//
		case 'N': {
			GetAlphaNumFromStream(istr, string1);

			if (!string1.Find("NAME")) {
				istr.eatWhite();
				GetAlphaNumFromStream(istr, string2);

				// Replace any underscores with spaces
				string2.ReplaceChar('_', ' ');
				setSusName(string2);

				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, string1);
			}
			break;
		}

		case 'S': {
			GetAlphaNumFromStream(istr, string1);

			if (!string1.Find("SPECIES")) {
				istr.eatWhite();
				GetAlphaNumFromStream(istr, string2);

				// Replace any underscores with spaces
				string2.ReplaceChar('_', ' ');

				setSusSpecies(string2);

				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, string1);
			}
			break;
		}

		case 'R': {
			GetAlphaNumFromStream(istr, string1);

			if (!string1.Find("ROOM")) {
				istr.eatWhite();
				GetAlphaNumFromStream(istr, string2);

				// Replace any underscores with spaces
				string2.ReplaceChar('_', ' ');
				setSusRoom(string2);

				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, string1);
			}
			break;
		}

		default: {
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
	}

	return PARSING_DONE;
}
void CBagLogSuspect::setProperty(const CBofString &prop, int val) {
	if (!prop.Find("ROOM")) {
		switch (val) {
		case 1:  // BAP
			setSusRoom("Entry Vestibule");
			break;
		case 2:  // BBP
			setSusRoom("Howdy Saloon");
			break;
		case 4:  // BDP
			setSusRoom("Bar Area");
			break;
		case 5:  // BEP
			setSusRoom("Dance Floor");
			break;
		case 6:  // BFP
			setSusRoom("Dining Area");
			break;
		case 7:  // BGP
			setSusRoom("Gambling Hall");
			break;
		case 10: // BJP
			setSusRoom("Kitchen");
			break;
		case 23:  // BWP
			setSusRoom("Outside Howdy Saloon");
			break;
		default:
			break;
		}
	} else {
		// Hack alert!  If our value is 2, then this means toggle the boolean!!!
		int hackVal = false;
		if (val == 1)
			hackVal = true;
		if (val == 0)
			hackVal = false;
		if (val == 2)
			hackVal = 2;

		if (!prop.Find("CHECKED"))
			setSusChecked(hackVal == 2 ? !getSusChecked() : hackVal);
		else if (!prop.Find("VP"))
			setSusVoicePrinted(hackVal == 2 ? !getSusVoicePrinted() : hackVal);
		else if (!prop.Find("RP"))
			setSusResiduePrinted(hackVal == 2 ? !getSusResiduePrinted() : hackVal);
		else
			CBagObject::setProperty(prop, val);
	}
}


int CBagLogSuspect::getProperty(const CBofString &prop) {
	if (!prop.Find("CHECKED"))
		return getSusChecked();

	if (!prop.Find("VP"))
		return getSusVoicePrinted();

	if (!prop.Find("RP"))
		return getSusResiduePrinted();

	return CBagObject::getProperty(prop);
}


void CBagLogSuspect::setSize(const CBofSize &size) {
	CBagTextObject::setSize(CBofSize(m_nSdevWidth, size.cy));

}


ErrorCode CBagLogSuspect::update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect, int maskColor) {
	char susCheckedFl[256];
	CBofString susCheckedString(susCheckedFl, 256);

	// Remove all the references to the jamming and voice printer state
	char voiceBuffer[256];
	char residueBuffer[256];

	CBofString voicePrintString(voiceBuffer, 256);
	CBofString residuePrintString(residueBuffer, 256);

	if (getSusChecked())
		susCheckedString = "Y";
	else
		susCheckedString = "N";

	if (getSusVoicePrinted())
		voicePrintString = "OK";
	else
		voicePrintString = "?";

	if (getSusResiduePrinted())
		residuePrintString = "Y";
	else
		residuePrintString = "N";

	setFont(FONT_MONO);

	setText(buildString(" %-5.5s %-17.17s %-12.12s %-20.20s %-4.4s %-4.4s",
	                    susCheckedString.GetBuffer(),
	                    _susName.GetBuffer(),
	                    _susSpecies.GetBuffer(),
	                    _susRoom.GetBuffer(),
	                    voicePrintString.GetBuffer(),
	                    residuePrintString.GetBuffer()));

	return CBagTextObject::update(bmp, pt, srcRect, maskColor);
}

// Energy detector objects, this should be pretty straightforward.

CBagEnergyDetectorObject::CBagEnergyDetectorObject() {
	setFont(FONT_MONO);				// Correct for spacing
	setColor(7);					// Make it white
	SetFloating();					// Is definitely floating
	setHighlight();					// Is highlight
	SetTitle();						// As title
	_textInitializedFl = false;     // Not initialized yet
}

CBagEnergyDetectorObject::~CBagEnergyDetectorObject() {
}

PARSE_CODES CBagEnergyDetectorObject::setInfo(CBagIfstream &istr) {
	bool objectUpdatedFl = false;
	char localBuffer[256];
	CBofString string1(localBuffer, 256);

	char localBuffer2[256];
	CBofString string2(localBuffer2, 256);

	while (!istr.eof()) {
		istr.eatWhite();
		
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  ZHAPS - NUMBER OF ZHAPS (ENERGY UNITS)
		//
		case 'Z': {
			GetAlphaNumFromStream(istr, string1);

			if (!string1.Find("ZHAPS")) {
				istr.eatWhite();
				GetAlphaNumFromStream(istr, string2);

				_zhapsStr = string2;

				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, string1);
			}
			break;
		}

		//
		//  CAUSE - REASON FOR ENERGY BURST
		//
		case 'C': {
			GetAlphaNumFromStream(istr, string1);

			if (!string1.Find("CAUSE")) {
				istr.eatWhite();
				GetAlphaNumFromStream(istr, string2);

				_causeStr = string2;

				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, string1);
			}
			break;
		}

		//
		//   TIME x- Sets the time of the message to xx:xx
		//
		case 'T': {
			GetAlphaNumFromStream(istr, string1);

			if (!string1.Find("TIME")) {
				istr.eatWhite();
				(void)istr.peek();

				GetAlphaNumFromStream(istr, _energyTimeStr);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, string1);
			}
			break;
		}

		//
		//  SIZE n - n point size of the txt
		//
		case 'S': {
			char localBuffer3[256];
			localBuffer3[0] = 0;
			CBofString string3(localBuffer3, 256);

			GetAlphaNumFromStream(istr, string3);

			if (!string3.Find("SIZE")) {
				istr.eatWhite();
				int n;
				GetIntFromStream(istr, n);
				setPointSize(n);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, string3);
			}
			break;
		}

		default: {
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
	}

	return PARSING_DONE;
}

ErrorCode CBagEnergyDetectorObject::update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect, int maskColor) {
	// Don't draw until we're attached
	if (isAttached() == false) {
		return ERR_NONE;
	}

	return CBagTextObject::update(bmp, pt, srcRect, maskColor);
}

ErrorCode CBagEnergyDetectorObject::attach() {
	assert(isValidObject(this));

	int nMsgTime;
	char localBuffer[256];
	CBofString causeString(localBuffer, 256);

	char zhapsBuffer[256];
	CBofString zhapsString(zhapsBuffer, 256);

	// We could use a variable here, translate it's value if that's the case.
	CBagVar *curVar = VAR_MANAGER->GetVariable(_energyTimeStr);
	if (curVar) {
		nMsgTime = curVar->GetNumValue();
	} else {
		nMsgTime = atoi(_energyTimeStr.GetBuffer());
	}

	int hour = nMsgTime / 100;
	int minute = nMsgTime - (hour * 100);

	// Get the number of zhaps.
	curVar = VAR_MANAGER->GetVariable(_zhapsStr);
	if (curVar) {
		zhapsString = curVar->GetValue();
	} else {
		zhapsString = _zhapsStr;
	}

	// Get the cause
	curVar = VAR_MANAGER->GetVariable(_causeStr);
	if (curVar) {
		causeString = curVar->GetValue();
	} else {
		causeString = _causeStr;
	}

	// Replace any underscores with spaces
	causeString.ReplaceChar('_', ' ');

	CBofString cStr;
	SetPSText(&cStr);

	setText(buildString("%02d:%02d %6.6s %s  %-35.35s", hour, minute, zhapsString.GetBuffer(), "zhaps", causeString.GetBuffer()));
	RecalcTextRect(false);

	return CBagObject::attach();
}

CBagLogClue::CBagLogClue(const CBofString &initStr, int sdevWidth, int pointSize) : CBagTextObject() {
	m_xObjType = USEROBJ;
	_sdevWidth = sdevWidth;

	m_bTitle = true;

	_stringVar1 = nullptr;
	_stringVar2 = nullptr;
	_stringVar3 = nullptr;
	_stringVar4 = nullptr;

	setFont(FONT_MONO);
	SetInitInfo(initStr);
	setPointSize(pointSize);
	setColor(7);
	SetFloating();
}

ErrorCode CBagLogClue::attach() {
	char szFormatStr[256];
	char szClueStr[256];
	CBofString cFormat(szFormatStr, 256);

	assert(isValidObject(this));

	ErrorCode ec = CBagTextObject::attach();

	// Get what is defined in the script.
	cFormat = getFileName();

	// Replace '$' with '%' (% is an illegal character embedded in a clue string).
	cFormat.ReplaceChar('_', ' ');
	cFormat.ReplaceChar('$', '%');

	// Format the text appropriately.
	Common::sprintf_s(szClueStr, cFormat.GetBuffer(),
	                  (_stringVar1 ? (const char *)_stringVar1->GetValue() : (const char *)""),
	                  (_stringVar2 ? (const char *)_stringVar2->GetValue() : (const char *)""),
	                  (_stringVar3 ? (const char *)_stringVar3->GetValue() : (const char *)""),
	                  (_stringVar4 ? (const char *)_stringVar4->GetValue() : (const char *)""));

	CBofString cStr(szClueStr);
	SetPSText(&cStr);

	return ec;
}

PARSE_CODES CBagLogClue::setInfo(CBagIfstream &istr) {
	bool nObjectUpdated = false;

	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	while (!istr.eof()) {
		istr.eatWhite();
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  STRINGVAR - This will be a variable used to display some information that
		//  is contained in script in a clue statement.
		//
		case 'S': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("STRINGVAR")) {
				istr.eatWhite();
				GetAlphaNumFromStream(istr, sStr);
				CBagVar *pVar = VAR_MANAGER->GetVariable(sStr);
				// The variable must have been found, if it wasn't, then
				// complain violently.

				if (pVar == nullptr) {
					return UNKNOWN_TOKEN;
				}

				if (_stringVar1 == nullptr) {
					_stringVar1 = pVar;
				} else if (_stringVar2 == nullptr) {
					_stringVar2 = pVar;
				} else if (_stringVar3 == nullptr) {
					_stringVar3 = pVar;
				} else if (_stringVar4 == nullptr) {
					_stringVar4 = pVar;
				} else {
					return UNKNOWN_TOKEN;
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}

	}

	return PARSING_DONE;
}

ErrorCode CBagLogClue::update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *srcRect, int maskColor) {
	return CBagTextObject::update(pBmp, pt, srcRect, maskColor);
}

} // namespace Bagel
