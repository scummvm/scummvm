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

#include "bagel/baglib/storage_dev_win.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/gfx/sprite.h"
#include "bagel/boflib/event_loop.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/wield.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/log_msg.h"
#include "bagel/baglib/event_sdev.h"

// Objects up the yang
#include "bagel/baglib/area_object.h"
#include "bagel/baglib/bmp_object.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/character_object.h"
#include "bagel/baglib/command_object.h"
#include "bagel/baglib/expression_object.h"
#include "bagel/baglib/link_object.h"
#include "bagel/baglib/movie_object.h"
#include "bagel/baglib/sound_object.h"
#include "bagel/baglib/sprite_object.h"
#include "bagel/baglib/text_object.h"
#include "bagel/baglib/variable_object.h"
#include "bagel/baglib/thing_object.h"
#include "bagel/baglib/rp_object.h"
#include "bagel/baglib/dossier_object.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/bagel.h"
#include "bagel/boflib/file_functions.h"

namespace Bagel {

// Globals (hacks)
bool g_allowPaintFl = true;
bool g_bAAOk = true;            // Prevent attachActiveObjects() after a RUN LNK
bool g_allowattachActiveObjectsFl = true;        // Prevent attachActiveObjects() after a RUN LNK
CBagStorageDevWnd *g_pLastWindow = nullptr;
extern bool g_pauseTimerFl;
extern bool g_waitOKFl;

// Statics
CBagEventSDev *CBagStorageDevWnd::_pEvtSDev = nullptr;    // Pointer to the EventSDev
CBofPoint *CBagStorageDev::_xCursorLocation;
CBofRect *CBagStorageDev::gRepaintRect;
bool CBagStorageDev::m_bHidePDA = false;
bool CBagStorageDev::m_bHandledUpEvent = false;

// Dirty object variables
bool CBagStorageDev::m_bPanPreFiltered = false;
bool CBagStorageDev::m_bDirtyAllObjects = false;
bool CBagStorageDev::m_bPreFilter = false;

int CBagStorageDevManager::nSDevMngrs = 0;

// Local globals
static int gLastBackgroundUpdate = 0;

#define kCursWidth 55

void CBagStorageDev::initialize() {
	g_allowPaintFl = true;
	g_bAAOk = true;
	g_allowattachActiveObjectsFl = true;
	g_pLastWindow = nullptr;

	_xCursorLocation = new CBofPoint();
	gRepaintRect = new CBofRect();
	m_bHidePDA = false;
	m_bHandledUpEvent = false;

	m_bPanPreFiltered = false;
	m_bDirtyAllObjects = false;
	m_bPreFilter = false;

	gLastBackgroundUpdate = 0;
}

void CBagStorageDev::shutdown() {
	delete _xCursorLocation;
	delete gRepaintRect;
}

CBagStorageDev::CBagStorageDev() {
	m_pLActiveObject = nullptr;        // The last object selected on mouse down

	m_pAssociateWnd = nullptr;         // The associate window for attaching sounds

	m_bForiegnList = false;
	m_pObjectList = new CBofList<CBagObject *>;
	m_pExpressionList = nullptr;
	m_nDiskID = 1;
	m_bCloseup = false;
	m_bCIC = false;

	// run object stuff
	_bFirstPaint = true;

	m_nFloatPages = 0;

	_xSDevType = SDEV_UNDEF;

	_pBitmapFilter = nullptr;

	setCloseOnOpen(false);
	SetExitOnEdge(0);
	SetFilterId(0);
	SetFadeId(0);

	// Default is this thing is not a customized sdev.
	SetCustom(false);

	// Make sure all objects that are prefiltered are dirty
	setDirtyAllObjects(true);

	// Not sure what the hell is going on here...
	SetLActivity(kMouseNONE);

	SDEV_MANAGER->RegisterStorageDev(this);
}

CBagStorageDev::~CBagStorageDev() {
	if (!m_bForiegnList) {

		if (m_pObjectList != nullptr) {
			ReleaseObjects();                            // Delete all master sprite objects
			delete m_pObjectList;
			m_pObjectList = nullptr;
		}
		if (m_pExpressionList) {
			while (m_pExpressionList->getCount()) {
				CBagExpression *pExp = m_pExpressionList->removeHead();
				delete pExp;
			}
			delete m_pExpressionList;
			m_pExpressionList = nullptr;
		}
	}  // If the lists belong to this storage device

	SDEV_MANAGER->UnRegisterStorageDev(this);

	if (CBagStorageDevWnd::_pEvtSDev == this) {
		CBagStorageDevWnd::_pEvtSDev = nullptr;
	}
}


void CBagStorageDev::setPosition(const CBofPoint &pos) {
	CBofRect OrigRect = getRect();                  // Get the destination (screen) rect

	m_cDestRect.setRect(pos.x, pos.y,
	                    pos.x + OrigRect.width() - 1,
	                    pos.y + OrigRect.height() - 1);
}


bool CBagStorageDev::Contains(CBagObject *pObj, bool bActive) {
	int nCount = GetObjectCount();
	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			if (pObj == GetObjectByPos(i)) {
				if (bActive && (pObj->isActive()))
					return true;
			}
		}
	}
	return false;
}


ErrorCode CBagStorageDev::AddObject(CBagObject *pObj, int /*nPos*/) {
	ErrorCode errCode = ERR_NONE;

	// can't use a null pointer
	assert(pObj != nullptr);

	m_pObjectList->addToTail(pObj);

	return errCode;
}


ErrorCode CBagStorageDev::removeObject(CBagObject *pRObj) {
	ErrorCode errCode = ERR_NONE;

	if (!m_bForiegnList) {
		int nCount = GetObjectCount();
		for (int i = 0; i < nCount; ++i) {
			if (pRObj == GetObjectByPos(i)) {
				m_pObjectList->remove(i);
				return errCode;
			}
		}
	}

	return errCode;
}


ErrorCode CBagStorageDev::activateLocalObject(CBagObject  *pObj) {
	ErrorCode errCode = ERR_NONE;

	if (pObj != nullptr) {
		pObj->setLocal();
		if (!pObj->isActive() && (!pObj->getExpression() || pObj->getExpression()->evaluate(pObj->isNegative()))) {
			pObj->setActive();
			pObj->attach();

			// Preform an update and arrange objects in the storage device
			if (g_allowattachActiveObjectsFl) {
				attachActiveObjects();
			}
		}
	} else  {
		errCode = ERR_FFIND;
	}

	return errCode;
}


ErrorCode CBagStorageDev::activateLocalObject(const CBofString &sName) {
	// can't use a empty string
	assert(!sName.isEmpty());

	return activateLocalObject(GetObject(sName));
}

ErrorCode CBagStorageDev::deactivateLocalObject(CBagObject *pObj) {
	ErrorCode  errCode = ERR_NONE;

	if (pObj != nullptr) {
		pObj->setLocal(false);
		if (pObj->isActive()) {
			pObj->setActive(false);
			pObj->detach();
		}
	} else  {
		errCode = ERR_FFIND;
	}

	return errCode;
}


ErrorCode CBagStorageDev::deactivateLocalObject(const CBofString &sName) {
	// Can't use a empty string
	assert(!sName.isEmpty());

	return deactivateLocalObject(GetObject(sName));
}


CBofPoint CBagStorageDev::arrangeFloater(CBofPoint nPos, CBagObject *pObj) {
	CBofPoint NextPos = nPos;

	if (getBackground() != nullptr) {

		int nPageNum = 0;

		int     nBackWidth = getBackground()->width();
		int     nBackHeight = getBackground()->height();
		int     nObjWidth = pObj->getRect().width();
		int     nObjHeight = pObj->getRect().height();

		// Check to see if the whole object can fit in, if it can't wrap
		if (NextPos.x > (nBackWidth - nObjWidth)) {
			NextPos.x = 0;
			NextPos.y += pObj->getRect().height();
		}
		pObj->setPosition(NextPos);

		// Always round this figure up...

		nPageNum = ((NextPos.y + nObjHeight) / nBackHeight);
		if (((NextPos.y + nObjHeight) % nBackHeight) != 0) {
			nPageNum++;
		}

		SetNumFloatPages(nPageNum);

		NextPos.x += pObj->getRect().width();
	}

	return NextPos;
}


ErrorCode CBagStorageDev::attachActiveObjects() {
	ErrorCode errCode = ERR_NONE;
	CBofPoint nArrangePos(0, 0);	// Removed 5,5 padding

	CBagLog::initArrangePages();

	int nCount = GetObjectCount();
	if (nCount != 0) {
		SetContainsModal(false);

		for (int i = 0; i < nCount; ++i) {
			if (g_engine->shouldQuit())
				return ERR_NONE;

			CBagObject *pObj = GetObjectByPos(i);
			if (pObj != nullptr) {
				if (pObj->isLocal() && (!pObj->getExpression() || pObj->getExpression()->evaluate(pObj->isNegative()))) {
					if (!pObj->isAttached()) {
						pObj->setActive();
						pObj->attach();
					}

					// If we have already painted the storage device once
					if (pObj->isImmediateRun()) {

						if (_bFirstPaint == false) {

							pObj->runObject();

							if (pObj->getType() == LINKOBJ) {
								break;
							}
						}
					}
					if (pObj->isModal())
						SetContainsModal(true);
					if (pObj->isFloating()) {
						nArrangePos = arrangeFloater(nArrangePos, pObj);
					}
				} else if (pObj->isAttached()) {

					if (pObj->getType() != SOUNDOBJ || !((CBagSoundObject *)pObj)->isPlaying()) {
						pObj->setActive(false);
						pObj->detach();
					}
				}
			} else
				errCode = ERR_FFIND;
		}
	}

	CBagLog::arrangePages();

	return errCode;
}

ErrorCode CBagStorageDev::DetachActiveObjects() {
	ErrorCode  errCode = ERR_NONE;
	int nCount = GetObjectCount();

	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = GetObjectByPos(i);
			if (pObj != nullptr) {
				if (pObj->isAttached()) {
					// If this object is not removed from memory, then
					// make sure it is drawn next time it is activated.
					pObj->setDirty(true);
					pObj->detach();
				}
			} else
				errCode = ERR_FFIND;
		}
	}
	return errCode;
}

ErrorCode CBagStorageDev::LoadObjects() {
	ErrorCode errCode = ERR_NONE;
	return errCode;
}

ErrorCode CBagStorageDev::ReleaseObjects() {
	ErrorCode errCode = ERR_NONE;
	int nCount = GetObjectCount();

	if (!m_bForiegnList) {
		if (nCount) {
			for (int i = 0; i < nCount; ++i) {
				CBagObject *pObj = m_pObjectList->removeHead();
				delete pObj;
			}
		}

		m_pObjectList->removeAll();
	}
	return errCode;
}


void CBagStorageDev::SetObjectList(CBofList<CBagObject *> *pOList, CBofList<CBagExpression *> *pEList) {
	delete m_pObjectList;

	m_bForiegnList    = true;
	m_pObjectList     = pOList;
	m_pExpressionList = pEList;
}


ErrorCode CBagStorageDev::PaintStorageDevice(CBofWindow * /*pWnd*/, CBofBitmap *pBmp, CBofRect * /*pRect*/) {
	bool        bMouseOverObj = false;
	int         nCount        = GetObjectCount();

	if (nCount) {
		CBofWindow *pWnd1 = CBagel::getBagApp()->getMasterWnd();
		if (pWnd1)
			pWnd1->screenToClient(&*_xCursorLocation);

		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = GetObjectByPos(i);
			if (pObj->isAttached()) {
				CBofRect xBmpRect = pObj->getRect();
				CBofPoint pt = xBmpRect.topLeft();
				xBmpRect.offsetRect(-pt.x, -pt.y);

				if (pObj->isVisible()) {
					if (pBmp) {
						// Only update dirty objects...
						if (pObj->isDirty() || pObj->isAlwaysUpdate()) {
							pObj->update(pBmp, pt, &xBmpRect);
						}
					}
				}

				// if it is visible update it
				if (pObj->getRect().ptInRect(*_xCursorLocation)) {
					pObj->onMouseOver(0, *_xCursorLocation, this);
					bMouseOverObj = true;
				}  // if on screen
			}  // If the object is attached

		}  // While a valid object
	}   // if there is any valid objects

	if (!bMouseOverObj)
		NoObjectsUnderMouse();

	return ERR_NONE;
}

ErrorCode CBagStorageDev::OnLActiveObject(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * /*vpInfo*/) {
	return ERR_NONE;
}


ErrorCode CBagStorageDev::NoObjectsUnderMouse() {
	return ERR_NONE;
}

void CBagStorageDev::onMouseMove(uint32 nFlags, CBofPoint *xPoint, void *vpInfo) {
	*_xCursorLocation = *xPoint;

	if (GetLActiveObject() && GetLActivity()) {
		GetLActiveObject()->onMouseMove(nFlags, *xPoint, vpInfo);
	}
}


ErrorCode CBagStorageDev::onMouseOver(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void *) {
	return ERR_NONE;
}


void CBagStorageDev::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *vpInfo) {
	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::getActiveCursor() == 6) {
		return;
	}

	*_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = devPtToViewPort(*xPoint);

	SetLActivity(kMouseNONE);

	CBagObject *pObj = GetObject(xCursorLocation, true);
	if ((pObj != nullptr) && (pObj->isActive())) {
		pObj->onLButtonDown(nFlags, xPoint, vpInfo);
		SetLActivity(kMouseDRAGGING);
	}

	SetLActiveObject(pObj);
}

// TODO: This is a global, move it to the main class
bool g_noMenuFl = false;


void CBagStorageDev::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *vpInfo) {
	char szLocalBuff[256];
	CBofString sCurrSDev(szLocalBuff, 256);

	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::getActiveCursor() == 6) {
		return;
	}

	sCurrSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->GetName();

	*_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = devPtToViewPort(*xPoint);

	bool bUseWield = true;
	CBagObject *pObj = GetObject(xCursorLocation, true);
	if (pObj != nullptr) {
		bUseWield = false;

		g_noMenuFl = false;
		if (pObj->isActive()) {
			pObj->onLButtonUp(nFlags, xPoint, vpInfo);

			if (g_noMenuFl) {
				g_noMenuFl = false;
				bUseWield = true;
			}
			SetLActiveObject(pObj);
		}
		m_bHandledUpEvent = true;
	}

	if (bUseWield) {

		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagPanWindow *pWin = (CBagPanWindow *)pApp->getMasterWnd();
			if (pWin != nullptr) {
				if (pWin->_pWieldBmp != nullptr) {
					pObj = pWin->_pWieldBmp->getCurrObj();
					if ((pObj != nullptr) && pObj->isActive()) {
						pObj->onLButtonUp(nFlags, xPoint, vpInfo);
						SetLActiveObject(pObj);
						m_bHandledUpEvent = true;
					}
				}
			}
		}
	}

	SetLActivity(kMouseNONE);

	if (g_bAAOk && (sCurrSDev == (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->GetName()))) {
		attachActiveObjects();
	}

	g_bAAOk = true;
}


ErrorCode CBagStorageDev::loadFile(const CBofString &sWldName) {
	char szLocalBuff[256];
	CBofString sWldFileName(szLocalBuff, 256);

	sWldFileName = sWldName;

	MACROREPLACE(sWldFileName);

	// Force buffer to be big enough so that the entire script
	// is pre-loaded
	int nLength = fileLength(sWldFileName);
	char *pBuf = (char *)bofAlloc(nLength);
	if (pBuf != nullptr) {
		CBagIfstream fpInput(pBuf, nLength);

		CBofFile cFile;
		cFile.open(sWldFileName);
		cFile.read(pBuf, nLength);
		cFile.close();

		CBagStorageDev::loadFileFromStream(fpInput, sWldFileName);

		bofFree(pBuf);
	}

	// Add everything to the window
	return ERR_NONE;
}


ErrorCode CBagStorageDev::loadFileFromStream(CBagIfstream &fpInput, const CBofString &sWldName, bool bAttach) {
	char                szWorkStr[256];
	char                szStr[256];
	szWorkStr[0] = 0;
	szStr[0] = 0;
	CBofString          sWorkStr(szWorkStr, 256);
	CBofString          str(szStr, 256);
	CBagExpression      *pActiveExpr = nullptr;
	CBofList<bool>      bElseExprList;

	ReleaseObjects();

	SetName(sWldName);

	fpInput.eatWhite();
	int ch = fpInput.getCh();
	if (ch != SDEV_START_DELIM) {
		char szWarningMsg[256];
		CBofString s(szWarningMsg, 256);

		s = "Warning: { expected: at start of storage device: ";
		s += sWldName;
		bofMessageBox(s.getBuffer(), "Warning");

		fpInput.putBack();
	}

	fpInput.eatWhite();

	while (/*fpInput &&*/ !fpInput.eof() && !(fpInput.peek() == SDEV_END_DELIM)) {
		bool bOperSet           = false;    // Set if an operator was found
		bool bHoldActivation    = false;    // Set if the object should be held
		bool bRunActivation     = true;     // Set if the object should be run instantly on attach

		// Get Operator SET or HOLD or RUN; on none RUN is default
		GetAlphaNumFromStream(fpInput, sWorkStr);

		if (sWorkStr.isEmpty()) {
			ParseAlertBox(fpInput, "Error in line No Operator:", __FILE__, __LINE__);

			bOperSet        = true;
		}

		if (!sWorkStr.find("SET")) {
			bRunActivation  = false;
			bHoldActivation = false;
			bOperSet        = true;
		} else if (!sWorkStr.find("HOLD")) {
			bRunActivation  = true;
			bHoldActivation = true;
			bOperSet        = true;
		} else if (!sWorkStr.find("RUN")) {
			bRunActivation  = true;
			bHoldActivation = false;
			bOperSet        = true;
		}

		fpInput.eatWhite();

		if (bOperSet) {
			// If we are not doing the default RUN get next argument
			GetAlphaNumFromStream(fpInput, sWorkStr);
		}

		if (sWorkStr.isEmpty()) {
			ParseAlertBox(fpInput, "Error in line:", __FILE__, __LINE__);
		}

		fpInput.eatWhite();

		CBagObject *pObj = nullptr;

		if (!sWorkStr.find("BKG")) {
			setInfo(fpInput);
			if (bAttach && attach()) {
				assert(false);
			}

		} else if (!sWorkStr.find("DISKID")) {
			fpInput.eatWhite();
			ch = (char)fpInput.getCh();
			if (ch == '=') {
				GetAlphaNumFromStream(fpInput, str);
				fpInput.eatWhite();
				m_nDiskID = (uint16)atoi(str);
				if (fpInput.peek() == ';') {
					fpInput.getCh();
				}
			}
		} else if (!sWorkStr.find("HELP")) {
			fpInput.eatWhite();
			ch = (char)fpInput.getCh();
			if (ch == '=') {
				GetAlphaNumFromStream(fpInput, str);
				fpInput.eatWhite();

				SetHelpFilename(str);

				if (fpInput.peek() == ';') {
					fpInput.getCh();
				}
			}
		} else if (!sWorkStr.find("ENDIF")) {
			if (bElseExprList.isEmpty()) {
				ParseAlertBox(fpInput, "Error: ENDIF without IF", __FILE__, __LINE__);
			} else {
				bElseExprList.removeHead();
			}
			if (pActiveExpr) {
				pActiveExpr = pActiveExpr->getPrevExpression();
			} else {
				CBofString str2("Unexpected ENDIF:");
				str2 += sWldName;
				ParseAlertBox(fpInput, str2.getBuffer(), __FILE__, __LINE__);
			}
		} else if (!sWorkStr.find("IF")) {
			// Added a bPrevNeg to keep track of nested else-if's
			bool bPrevNeg = false;
			if (bElseExprList.getHead())
				bPrevNeg = bElseExprList.getHead()->getNodeItem();

			bElseExprList.addToHead((bool) false);

			// Added a bPrevNeg to keep track of nested else-if's
			CBagExpression *pExp = new CBagExpression(pActiveExpr, bPrevNeg);

			assert(pExp != nullptr);
			pExp->setInfo(fpInput);
			if (!m_pExpressionList)
				m_pExpressionList = new CBofList<CBagExpression * >;
			assert(m_pExpressionList != nullptr);
			m_pExpressionList->addToTail(pExp);
			pActiveExpr = pExp;
		} else if (!sWorkStr.find("ELSE")) {
			if (bElseExprList.isEmpty()) {
				ParseAlertBox(fpInput, "Error: ELSE without IF", __FILE__, __LINE__);
			} else {
				bElseExprList.removeHead();
				bElseExprList.addToHead((bool) true);
			}
		} else if (!sWorkStr.find("BMP")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewSpriteObject(sWorkStr);
		} else if (!sWorkStr.find("SPR")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewSpriteObject(sWorkStr);
		} else if (!sWorkStr.find("LNK")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewLinkObject(sWorkStr);

		} else if (!sWorkStr.find("RPO")) {
			// Allow residue printing objects
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewRPObject(sWorkStr);

		} else if (!sWorkStr.find("EDO")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewEDObject(sWorkStr);

		} else if (!sWorkStr.find("DOS")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewDosObject(sWorkStr);
		} else if (!sWorkStr.find("SND")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewSoundObject(sWorkStr);
		} else if (!sWorkStr.find("BUT")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewButtonObject(sWorkStr);
		} else if (!sWorkStr.find("CHR")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewCharacterObject(sWorkStr);
		} else if (!sWorkStr.find("TNG")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewThingObject(sWorkStr);
		} else if (!sWorkStr.find("ARE")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewAreaObject(sWorkStr);
		} else if (!sWorkStr.find("VAR")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewVariableObject(sWorkStr);
		} else if (!sWorkStr.find("TXT")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewTextObject(sWorkStr);
		} else if (!sWorkStr.find("MOVIE")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewMovieObject(sWorkStr);
		} else if (!sWorkStr.find("COMMAND")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewCommandObject(sWorkStr);
		} else if (!sWorkStr.find("EXPR")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewExpressionObject(sWorkStr);
		} else if (!sWorkStr.find("REM") || !sWorkStr.find("//")) {
			char s[256];
			fpInput.getCh(s, 256);
		} else {
			pObj = onNewUserObject(sWorkStr);
		}

		if (pObj != nullptr) {
			assert(CBofObject::isValidObject(pObj));

			if (!bElseExprList.isEmpty() && bElseExprList[0]) {
				pObj->setNegative();
			} else if (pActiveExpr && pActiveExpr->isNegative()) {
				// If there is an active expression that is negative
				// i.e if NOT( blah blah blah)
				pObj->setNegative();
			}

			// Determine if the object is an active object in this world
			if (!bHoldActivation) {
				pObj->setLocal();
				if (!pActiveExpr || pActiveExpr->evaluate(pObj->isNegative())) {
					pObj->setActive();
				}
			}
			if (bRunActivation) {
				pObj->setImmediateRun();
			}

			pObj->setInfo(fpInput);
			pObj->setExpression(pActiveExpr);

			AddObject(pObj);
		}

		fpInput.eatWhite();

	}  // While not eof

	if (fpInput.peek() == SDEV_END_DELIM)
		fpInput.getCh();

	fpInput.eatWhite();

	if (pActiveExpr) {
		CBofString str2("Mismatch in IF/ENDIF:");
		str2 += sWldName;
		ParseAlertBox(fpInput, str2.getBuffer(), __FILE__, __LINE__);

		return ERR_UNKNOWN;
	}

	if (bAttach)
		return attachActiveObjects();

	// Add everything to the window
	return ERR_NONE;
}


int CBagStorageDev::GetObjectCount() {
	if (m_pObjectList != nullptr) {
		return m_pObjectList->getCount();

	}

	return 0;
}


CBagObject *CBagStorageDev::GetObjectByPos(int nIndex) {
	assert(m_pObjectList != nullptr);
	assert((nIndex >= 0) && (nIndex < m_pObjectList->getCount()));

	return m_pObjectList->getNodeItem(nIndex);
}


CBagObject *CBagStorageDev::GetObject(int nRefId, bool bActiveOnly) {
	assert(m_pObjectList != nullptr);

	int nListLen = m_pObjectList->getCount();

	for (int i = 0; i < nListLen; ++i) {
		CBagObject *pObj = GetObjectByPos(i);

		if ((pObj->getRefId() == nRefId) && (!bActiveOnly || (pObj->isActive() && pObj->isAttached())))
			return pObj;
	}

	return nullptr;
}


CBagObject *CBagStorageDev::GetObject(const CBofString &sName, bool bActiveOnly) {
	assert(m_pObjectList != nullptr);

	CBagObject *pObjFound = nullptr;
	CBofListNode<CBagObject *> *pNode = m_pObjectList->getHead();
	while (pNode != nullptr) {
		CBagObject *pObj = pNode->getNodeItem();

		if (pObj->getRefName().compare(sName) == 0) {
			pObjFound = pObj;
			break;
		}

		pNode = pNode->_pNext;
	}
	if (bActiveOnly && (pObjFound != nullptr) && !pObjFound->isActive())
		pObjFound = nullptr;

	return pObjFound;
}


CBagObject *CBagStorageDev::GetObjectByType(const CBofString &sType, bool bActiveOnly) {
	assert(m_pObjectList != nullptr);

	int nListLen = m_pObjectList->getCount();

	for (int i = 0; i < nListLen; ++i) {
		CBagObject *pObj = GetObjectByPos(i);

		if (bActiveOnly) {
			if (pObj->isActive() && !getStringTypeOfObject(pObj->getType()).find(sType))
				return pObj;
		} else if (!getStringTypeOfObject(pObj->getType()).find(sType))
			return pObj;
	}

	return nullptr;
}


CBagObject *CBagStorageDev::GetObject(const CBofPoint &xPoint, bool bActiveOnly) {
	int nCount = GetObjectCount();

	// Resolve in reverse order since the last painted is on top
	if (nCount != 0) {
		for (int i = nCount - 1; i >= 0; --i) {
			CBagObject *pObj = GetObjectByPos(i);

			if (pObj->isInside(xPoint) && (!bActiveOnly || (pObj->isActive() && pObj->isAttached())))
				return pObj;
		}
	}
	return nullptr;
}


void CBagStorageDev::handleError(ErrorCode errCode) {
}


PARSE_CODES CBagStorageDev::setInfo(CBagIfstream &fpInput) {
	char szStr[256];
	szStr[0] = 0;
	CBofString str(szStr, 256);

	fpInput.eatWhite();

	char ch = (char)fpInput.getCh();
	if (ch == '=') {
		GetAlphaNumFromStream(fpInput, str);
		fpInput.eatWhite();

		MACROREPLACE(str);

		m_sBackgroundName = str;

		if (fpInput.peek() == ';') {
			fpInput.getCh();
		}
	}

	return PARSING_DONE;
}


ErrorCode CBagStorageDev::attach() {
	// Assume no error
	ErrorCode errCode = ERR_NONE;

	_bFirstPaint = true;

	if (!m_sBackgroundName.isEmpty()) {
		CBofBitmap *pBmp = new CBofBitmap(m_sBackgroundName);

		if ((pBmp != nullptr) && !pBmp->errorOccurred()) {
			setBackground(pBmp);
			errCode = attachActiveObjects();
		} else {
			errCode = ERR_FOPEN;
		}
	}

	return errCode;
}


ErrorCode CBagStorageDev::detach() {
	// Must force people to not use a bad App's palette
	CBofApp::getApp()->setPalette(nullptr);
	setBackground(nullptr);

	// Notify the main window that we need to redraw the background filter.
	CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());
	if (pMainWin != nullptr) {
		((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);
	}

	return DetachActiveObjects();
}


ErrorCode CBagStorageDev::close() {
	return ERR_NONE;
}


CBagObject *CBagStorageDev::onNewSpriteObject(const CBofString &) {
	return new CBagSpriteObject();
}


CBagObject *CBagStorageDev::OnNewBitmapObject(const CBofString &) {
	return new CBagBmpObject();
}


CBagObject *CBagStorageDev::onNewLinkObject(const CBofString &) {
	return new CBagLinkObject();
}

CBagObject *CBagStorageDev::OnNewRPObject(const CBofString &) {
	return new CBagRPObject();
}

CBagObject *CBagStorageDev::OnNewEDObject(const CBofString &) {
	return new CBagEnergyDetectorObject();
}

CBagObject *CBagStorageDev::OnNewDosObject(const CBofString &) {
	return new CBagDossierObject();
}

CBagObject *CBagStorageDev::OnNewTextObject(const CBofString &) {
	return new CBagTextObject();
}

CBagObject *CBagStorageDev::OnNewSoundObject(const CBofString &) {
	return new CBagSoundObject();
}


CBagObject *CBagStorageDev::onNewButtonObject(const CBofString &) {
	return new CBagButtonObject();
}


CBagObject *CBagStorageDev::OnNewCharacterObject(const CBofString &) {
	return new CBagCharacterObject();
}

CBagObject *CBagStorageDev::OnNewThingObject(const CBofString &) {
	return new CBagThingObject();
}

CBagObject *CBagStorageDev::OnNewMovieObject(const CBofString &) {
	return new CBagMovieObject();
}

CBagObject *CBagStorageDev::OnNewCommandObject(const CBofString &) {
	return new CBagCommandObject();
}


CBagObject *CBagStorageDev::OnNewAreaObject(const CBofString &) {
	return new CBagAreaObject();
}


CBagObject *CBagStorageDev::OnNewExpressionObject(const CBofString &) {
	return new CBagExpressionObject();
}


CBagObject *CBagStorageDev::OnNewVariableObject(const CBofString &) {
	return new CBagVariableObject();
}


CBagObject *CBagStorageDev::onNewUserObject(const CBofString &str) {
	char szLocalBuff[256];
	CBofString s(szLocalBuff, 256);

	s = str;

	bofMessageBox(s.getBuffer(), "Could not find object type");

	return nullptr;
}

void CBagStorageDev::OnSetFilter(bool (*filterFunction)(const uint16 nFilterid, CBofBitmap *, CBofRect *)) {
	_pBitmapFilter = filterFunction;
}

FilterFunction CBagStorageDev::GetFilter() {
	return _pBitmapFilter;
}


ErrorCode CBagStorageDev::PreFilter(CBofBitmap *pBmp, CBofRect *pRect, CBofList<CBagObject *> *pList) {
	if (pBmp != nullptr) {

		// If we are not dirtying all the objects, then only fill up the viewport.
		CBofRect viewPortRect(80, 10, 559, 369);
		CBofRect *fillRect;
		if (GetDirtyAllObjects()) {
			fillRect = pRect;
		} else {
			fillRect = &viewPortRect;
		}

		pBmp->fillRect(fillRect, RGB(0, 0, 0));
	}

	// Let pda know that we've been prefiltered
	SetPreFiltered(true);

	if (GetDirtyAllObjects()) {
		MakeListDirty(m_pObjectList);
		MakeListDirty(pList);
	} else {
		setDirtyAllObjects(true);
	}

	return ERR_NONE;
}


void CBagStorageDev::MakeListDirty(CBofList<CBagObject *> *pList) {
	if (pList) {
		int nCount = pList->getCount();
		if (nCount != 0) {

			for (int i = 0; i < nCount; ++i) {
				CBagObject *pObj = pList->getNodeItem(i);
				pObj->setDirty(true);
			}
		}
	}
}

/*****************************************************************************
*
*  CBagStorageDevWnd -
*
*
*****************************************************************************/

CBagStorageDevWnd::CBagStorageDevWnd() : CBofWindow() {
	setOnUpdate(true);
	setCloseOnOpen(false);
	m_pWorkBmp = nullptr;

	// Set a default help file for when there is not one specified
	m_sHelpFileName = "$SBARDIR\\GENERAL\\RULES\\DEFAULT.TXT";
	MACROREPLACE(m_sHelpFileName);

	_xSDevType = SDEV_WND;
}


CBagStorageDevWnd::~CBagStorageDevWnd() {
	assert(isValidObject(this));

	KillWorkBmp();
}


ErrorCode CBagStorageDevWnd::attach() {
	char szLocalBuff[256];
	CBofString s(szLocalBuff, 256);

	s = GetName();

	if (!GetBackgroundName().isEmpty()) {
		// This should actually be moved to sbarapp, but the load file will then
		// need to be removed from the constructor.
		//CBofApp::getApp()->setMainWindow(this);

		// Associate this window with callbacks so that any public member function can
		// be accessed by objects inserted into this class.
		SetAssociateWnd(this);

		CBofBitmap *pBmp  = new CBofBitmap(GetBackgroundName());

		if ((pBmp == nullptr) || (pBmp->height() <= 0) || (pBmp->width() <= 0)) {
			reportError(ERR_FOPEN, "BarComputer Background Opened Failed");
		} else {

			setBackground(pBmp);

			// Set the bagel crap
			CBofPalette *pPalette = pBmp->getPalette();
			CBofApp::getApp()->setPalette(pPalette);
			CBofSprite::openLibrary(pPalette);

			CBofRect r = pBmp->getRect();

			if (r.width() && r.height()) {
				create(s.getBuffer(), &r, CBagel::getBagApp()->getMasterWnd());

			} else {
				create(s.getBuffer(), nullptr, CBagel::getBagApp()->getMasterWnd());
			}

			show();

			attachActiveObjects();
		}

	} else {
		reportError(ERR_UNKNOWN, "No background for this storage device window");
	}

	SetPreFilterPan(true);
	g_pLastWindow = this;

	CBagStorageDev *pSDev = SDEV_MANAGER->GetStorageDevice("EVT_WLD");

	if (pSDev != nullptr) {
		// Have we allocated one yet ?
		if (_pEvtSDev == nullptr) {
			_pEvtSDev = (CBagEventSDev *)pSDev;
			_pEvtSDev->SetAssociateWnd(this);
			if (!_pEvtSDev->isAttached())
				_pEvtSDev->attach();

			setTimer(EVAL_EXPR, 1000);
			g_pauseTimerFl = false;

		} else {
			// We already allocated one
			// We just need to re-associate the parent window and reset the timer
			_pEvtSDev->SetAssociateWnd(this);

			setTimer(EVAL_EXPR, 1000);
			g_pauseTimerFl = false;
		}
	}

	return _errCode;
}

void CBagStorageDevWnd::onTimer(uint32 nEventID) {
	assert(isValidObject(this));
	static bool bAlready = false;

	if (!g_pauseTimerFl) {
		// Don't allow recursion
		if (!bAlready) {
			bAlready = true;
			// Evaluate the event storage device IF MOVIE NOT PLAYING
			if ((CBofApp::getApp()->getMainWindow())->isEnabled() && nEventID == EVAL_EXPR) {
				if (_pEvtSDev != nullptr) {
					_pEvtSDev->evaluateExpressions();

					// If our turncount was updated, then execute the event world
					// for the turncount dependent storage device.
					if (CBagEventSDev::getEvalTurnEvents() == true) {
						CBagEventSDev::setEvalTurnEvents(false);
						CBagTurnEventSDev *pSDev = (CBagTurnEventSDev *) SDEV_MANAGER->GetStorageDevice("TURN_WLD");
						if (pSDev != nullptr) {
							// If unable to execute event world, try again next time through.
							if (pSDev->evaluateExpressions() == ERR_UNKNOWN) {
								CBagEventSDev::setEvalTurnEvents(true);
							}
						}
					}
				}

				g_waitOKFl = true;
			}
			bAlready = false;
		}
	}
}


ErrorCode CBagStorageDevWnd::detach() {
	DetachActiveObjects();

	CBofApp::getApp()->setPalette(nullptr);

	setBackground(nullptr);
	CBofSprite::closeLibrary();
	CBagStorageDev::detach();

	killTimer(EVAL_EXPR);

	destroy();

	return _errCode;
}

ErrorCode CBagStorageDevWnd::close() {
	CBagel::getBagApp()->getMasterWnd()->setStorageDev(getPrevSDev(), false);

	return _errCode;
}


ErrorCode CBagStorageDevWnd::setBackground(CBofBitmap *pBmp) {
	if (pBmp) {
		setBackdrop(pBmp);
		SetWorkBmp();
	} else {
		killBackdrop();
		KillWorkBmp();
	}
	return _errCode;
}


ErrorCode CBagStorageDevWnd::SetWorkBmp() {
	// Delete any previous work area
	KillWorkBmp();

	CBofBitmap *pBmp = getBackground();
	if (pBmp != nullptr) {
		m_pWorkBmp = new CBofBitmap(pBmp->width(), pBmp->height(), pBmp->getPalette());
		pBmp->paint(m_pWorkBmp);
	}

	return _errCode;
}


ErrorCode CBagStorageDevWnd::KillWorkBmp() {
	if (m_pWorkBmp != nullptr) {
		delete m_pWorkBmp;
		m_pWorkBmp = nullptr;
	}

	return _errCode;
}

void CBagStorageDevWnd::onPaint(CBofRect *) {
	assert(isValidObject(this));

	PaintScreen();
}

void CBagStorageDevWnd::onMainLoop() {
	assert(isValidObject(this));

	PaintScreen();

	g_pLastWindow = this;
}

ErrorCode CBagStorageDevWnd::PaintScreen(CBofRect *pRect) {
	assert(isValidObject(this));

	if (_pBackdrop != nullptr) {
		onRender(_pBackdrop, pRect);

		if (g_allowPaintFl) {
			_pBackdrop->paint(this, pRect, pRect);
		}
	}

	if (_bFirstPaint) {
		_bFirstPaint = false;
		attachActiveObjects();
	}

	return _errCode;
}


ErrorCode CBagStorageDevWnd::onRender(CBofBitmap *pBmp, CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);

	if (PreFilterPan()) {
		PreFilter(pBmp, pRect, nullptr);
		SetPreFilterPan(false);

		if (m_pWorkBmp != nullptr) {
			m_pWorkBmp->paint(pBmp, pRect, pRect);
		}
	}

	PaintStorageDevice(this, pBmp, pRect);


	if (IsFiltered()) {
		uint16 nFilterId = GetFilterId();
		(*_pBitmapFilter)(nFilterId, pBmp, pRect);
	}

	return _errCode;
}


ErrorCode CBagStorageDevWnd::runModal(CBagObject *pObj) {
	assert(pObj != nullptr);

	if (pObj->isModal() && pObj->isActive()) {

		EventLoop eventLoop;
		CBofBitmap *pBmp = getBackdrop();

		if (pBmp != nullptr) {
			while (!g_engine->shouldQuit() && !pObj->isModalDone()) {
				// Make sure we redraw each and every frame!
				SetPreFilterPan(true);
				onRender(pBmp, nullptr);
				if (g_allowPaintFl) {
					pBmp->paint(this, 0, 0);
				}

				if (eventLoop.frame())
					break;
			}
		}
	}

	return _errCode;
}



ErrorCode CBagStorageDevWnd::paintObjects(CBofList<CBagObject *> * /*list*/, CBofBitmap * /*pBmp*/, CBofRect & /*viewRect*/, CBofList<CBofRect> * /*pUpdateArea*/, bool /*tempVar*/) {
	return _errCode;
}

ErrorCode CBagStorageDevWnd::loadFile(const CBofString &sFile) {
	char        szWldFile[256];
	szWldFile[0] = 0;
	CBofString sWldFile(szWldFile, 256);        // performance improvement

	if (sFile.isEmpty())
		sWldFile = "StoreDev.Wld";
	else
		sWldFile = sFile;

	MACROREPLACE(sWldFile);

	// Force buffer to be big enough so that the entire script
	// is pre-loaded
	int nLength = fileLength(sWldFile);
	char *pBuf = (char *)bofAlloc(nLength);
	if (pBuf != nullptr) {
		CBagIfstream fpInput(pBuf, nLength);

		CBofFile cFile;
		cFile.open(sWldFile);
		cFile.read(pBuf, nLength);
		cFile.close();

		CBagStorageDev::loadFileFromStream(fpInput, sWldFile);

		// If the window.isCreated()
		//
		if (isCreated())
			invalidateRect(nullptr);

		bofFree(pBuf);

	} else {
		reportError(ERR_MEMORY);
	}

	// Add everything to the window

	return _errCode;
}


void CBagStorageDevWnd::onClose() {
	CBofWindow::onClose();
	destroyWindow();                            // destruct the main window
}

void CBagStorageDevWnd::onMouseMove(uint32 n, CBofPoint *pPoint, void *) {
	CBagStorageDev::onMouseMove(n, pPoint, GetAssociateWnd());

	if (CBagCursor::isSystemCursorVisible())
		return;

	CBagMasterWin::setActiveCursor(0);

	// If a zelda movie is playing then just give 'em the wait cursor
	// as we're not gonna allow them to do squat anyway.
	if (CBagPDA::IsMoviePlaying()) {
		CBagMasterWin::setActiveCursor(6);
		return;
	}

	if (GetExitOnEdge() && (pPoint->x < GetExitOnEdge()) && (pPoint->y < 360 + 10) && !(getPrevSDev().isEmpty())) {
		CBagMasterWin::setActiveCursor(10);

	} else {
		// Added wield cursors
		bool bWield = false;
		if (CBagWield::GetWieldCursor() >= 0 && !CBagCursor::isSystemCursorVisible()) {
			CBagMasterWin::setActiveCursor(CBagWield::GetWieldCursor());
			bWield = true;
		}

		// Run thru background object list and find if the cursor is over an object
		CBofList<CBagObject *> *pList = GetObjectList();
		if (pList != nullptr) {
			CBofPoint cCursorLocation = devPtToViewPort(*_xCursorLocation);

			// Go thru list backwards to find the 1st top-most object
			CBofListNode<CBagObject *> *pNode = pList->getTail();
			while (pNode != nullptr) {
				CBagObject *pObj = pNode->getNodeItem();

				// Change cursor as long as it's not a link to a closeup, or
				// link to another Pan, or a text menu, or button.
				//
				if (pObj->isAttached() && pObj->isInside(cCursorLocation)) {
					int nCursor = pObj->getOverCursor();
					if (!bWield || (nCursor == 2 || nCursor == 5 || nCursor == 55 || pObj->getType() == TEXTOBJ || pObj->getType() == BUTTONOBJ)) {
						CBagMasterWin::setActiveCursor(nCursor);
					}
					break;
				}

				pNode = pNode->_pPrev;
			}
		}
	}
}

void CBagStorageDevWnd::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	// If asynch movie playing in PDA don't react to mouse down (8033)
	//  if it's not a wait cursor, then allow the user to access that hotspot.
	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::getActiveCursor() == 6) {
		return;
	}

	CBagStorageDev::onLButtonDown(nFlags, xPoint, GetAssociateWnd());
	CBofWindow::onLButtonDown(nFlags, xPoint);
}

void CBagStorageDevWnd::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	// If asynch movie playing in PDA don't react to mouse down (8033)
	// if it's not a wait cursor, then allow the user to access that hotspot.
	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::getActiveCursor() == 6) {
		return;
	}

	// React to a mouse up, it will probably involve drawing a new window...
	SetPreFilterPan(true);

	if (GetExitOnEdge() && xPoint->x < GetExitOnEdge() && !(getPrevSDev().isEmpty())) {
		// Set the initial location as the last full panoramas position
		close();

	} else {
		CBagStorageDev::onLButtonUp(nFlags, xPoint, GetAssociateWnd());
		CBofWindow::onLButtonUp(nFlags, xPoint);
	}
}

void CBagStorageDevWnd::onKeyHit(uint32 lKey, uint32 nRepCount) {
	CBofWindow::onKeyHit(lKey, nRepCount);
}


CBagStorageDevDlg::CBagStorageDevDlg() : CBofDialog() {
	_xSDevType = SDEV_DLG;

	// Set a default help file for when there is not one specified
	//
	m_sHelpFileName = "$SBARDIR\\GENERAL\\RULES\\DEFAULT.TXT";
	MACROREPLACE(m_sHelpFileName);
}


ErrorCode CBagStorageDevDlg::attach() {
	assert(isValidObject(this));

	CBagStorageDev::attach();

	char szLocalBuff[256];
	CBofString s(szLocalBuff, 256);
	s = GetName();

	CBofBitmap *pBmp = getBackground();
	CBofRect r;
	if (pBmp)
		r = pBmp->getRect();

	if (r.width() && r.height()) {
		create(s.getBuffer(), &r, CBagel::getBagApp()->getMasterWnd());

	} else {
		create(s.getBuffer(), nullptr, CBagel::getBagApp()->getMasterWnd());
	}

	SetPreFilterPan(true);

	CBofDialog::doModal();

	destroy();

	return _errCode;
}


ErrorCode CBagStorageDevDlg::close() {
	releaseCapture();

	CBofDialog::close();

	return _errCode;
}


ErrorCode CBagStorageDevDlg::onRender(CBofBitmap *pBmp, CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);

	PaintStorageDevice(this, pBmp, pRect);

	if (IsFiltered()) {
		uint16 nFilterId = GetFilterId();
		(*_pBitmapFilter)(nFilterId, pBmp, pRect);
	}

	return _errCode;
}


void CBagStorageDevDlg::onMainLoop() {
	assert(isValidObject(this));

	// The background of a storage device might be in motion, i.e. it
	// might need updates, but since it is a background screen, it probably isn't that
	// important, so we'll update it 4 times / second.

	int nCurTime = getTimer();
	gLastBackgroundUpdate = nCurTime;
	if (g_pLastWindow) {
		g_pLastWindow->SetPreFilterPan(true);
	}

	PaintScreen();
}


void CBagStorageDevDlg::onPaint(CBofRect *) {
	assert(isValidObject(this));

	PaintScreen();

	validateAnscestors();

	CBagPanWindow::flushInputEvents();
}


ErrorCode CBagStorageDevDlg::PaintScreen(CBofRect *pRect) {
	assert(isValidObject(this));

	if (_pBackdrop != nullptr) {
		CBagStorageDevWnd *pWin = g_pLastWindow;

		if (pWin != nullptr) {
			CBofBitmap *pBmp = pWin->getBackdrop();

			if (pBmp != nullptr) {
				// Don't redraw the background window unless we have to.
				if (pWin->PreFilterPan()) {
					CBofBitmap *pWorkBmp = pWin->GetWorkBmp();
					if (pWorkBmp != nullptr) {
						pWorkBmp->paint(pBmp, pRect, pRect);
					}

					pWin->onRender(pBmp, pRect);
				}

				onRender(_pBackdrop, pRect);
				CBofRect wrect(getWindowRect());
				_pBackdrop->paint(pBmp, &wrect, nullptr);

				if (g_allowPaintFl) {
					pBmp->paint(pWin, pRect, pRect);
				}
			}
		}
	}

	// Set the firstpaint flag and attach objects
	// to allow for immediate run objects to run
	if (_bFirstPaint) {
		_bFirstPaint = false;
		attachActiveObjects();
	}

	return _errCode;
}


ErrorCode CBagStorageDevDlg::paintObjects(CBofList<CBagObject *> * /*list*/, CBofBitmap * /*pBmp*/, CBofRect & /*viewRect*/, CBofList<CBofRect> * /*pUpdateArea*/, bool /*tempVar*/) {
	return _errCode;
}


ErrorCode CBagStorageDevDlg::loadFile(const CBofString &sFile) {
	char        szWldFile[256];
	szWldFile[0] = 0;
	CBofString sWldFile(szWldFile, 256);

	if (sFile.isEmpty())
		sWldFile = "StoreDev.Wld";
	else
		sWldFile = sFile;

	MACROREPLACE(sWldFile);

	// Force buffer to be big enough so that the entire script is pre-loaded
	int nLength = fileLength(sWldFile);
	char *pBuf = (char *)bofAlloc(nLength);
	if (pBuf != nullptr) {
		CBagIfstream fpInput(pBuf, nLength);

		CBofFile cFile;
		cFile.open(sWldFile);
		cFile.read(pBuf, nLength);
		cFile.close();

		CBagStorageDev::loadFileFromStream(fpInput, sWldFile);

		bofFree(pBuf);

		if (isCreated())
			invalidateRect(nullptr);
	}

	// Add everything to the window
	return _errCode;
}

ErrorCode CBagStorageDevDlg::create(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID) {
	ErrorCode rc = CBofDialog::create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
	setCapture();
	return rc;
}

ErrorCode CBagStorageDevDlg::create(const char *pszName, CBofRect *pRect, CBofWindow *pParent, uint32 nControlID) {
	ErrorCode rc = CBofDialog::create(pszName, pRect, pParent, nControlID);
	setCapture();
	return rc;
}


void CBagStorageDevDlg::onClose() {
	if (_pDlgBackground != nullptr) {
		delete _pDlgBackground;
		_pDlgBackground = nullptr;
	}

	// Since our sdevdlg doesn't have a detach active objects
	// method, we will have to manually take all the existing objects and
	// tell them to redraw themselves should they ever be called on in life
	// to do as such.
	int nCount = GetObjectCount();
	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = GetObjectByPos(i);
			if (pObj != nullptr) {
				pObj->setDirty(true);
			}
		}
	}

	CBofDialog::onClose();

	destroy();		// Destruct the main window

	// Our dlog may have dirtied our backdrop, make sure it is redrawn.
	if (g_pLastWindow != nullptr) {
		g_pLastWindow->SetPreFilterPan(true);
		g_pLastWindow->PaintScreen(nullptr);

		// This is fairly shameful, but for some reason, some
		// updates don't work in the above paintscreen and must be updated the
		// next time through.  Don't know why, would love to find out, but
		// running out of time.
		g_pLastWindow->SetPreFilterPan(true);
	}
}

void CBagStorageDevDlg::onMouseMove(uint32 n, CBofPoint *xPoint, void *) {
	CBagStorageDev::onMouseMove(n, xPoint, GetAssociateWnd());
}

void CBagStorageDevDlg::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	CBagStorageDev::onLButtonDown(nFlags, xPoint, GetAssociateWnd());
	CBofDialog::onLButtonDown(nFlags, xPoint);
}

void CBagStorageDevDlg::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	if (CBofDialog::getRect().ptInRect(*xPoint)) {
		CBagStorageDev::onLButtonUp(nFlags, xPoint, GetAssociateWnd());
		CBofDialog::onLButtonUp(nFlags, xPoint);
	} else  {
		close();
	}
}

CBagStorageDevManager::CBagStorageDevManager() {
	assert(++nSDevMngrs < 2);
}

CBagStorageDevManager::~CBagStorageDevManager() {
	assert(isValidObject(this));

	assert(--nSDevMngrs >= 0);
	ReleaseStorageDevices();
	m_xStorageDeviceList.removeAll();
}

ErrorCode CBagStorageDevManager::RegisterStorageDev(CBagStorageDev *pSDev) {
	assert(isValidObject(this));

	m_xStorageDeviceList.addToTail(pSDev);

	return ERR_NONE;
}

ErrorCode CBagStorageDevManager::UnRegisterStorageDev(CBagStorageDev *pSDev) {
	assert(isValidObject(this));

	CBofListNode<CBagStorageDev *> *pList = m_xStorageDeviceList.getHead();
	while (pList != nullptr) {
		if (pSDev == pList->getNodeItem()) {
			m_xStorageDeviceList.remove(pList);
			break;
		}

		pList = pList->_pNext;
	}
	return ERR_NONE;
}

ErrorCode CBagStorageDevManager::ReleaseStorageDevices() {
	assert(isValidObject(this));

	while (m_xStorageDeviceList.getCount()) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[0];

		// The CBagStorageDev destructor will remove it from the list
		delete pSDev;
	}

	return ERR_NONE;
}

CBagStorageDev *CBagStorageDevManager::GetStorageDeviceContaining(CBagObject *pObj) {
	assert(isValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];
		if (pSDev && pSDev->Contains(pObj))
			return m_xStorageDeviceList[i];
	}
	return nullptr;
}

CBagStorageDev *CBagStorageDevManager::GetStorageDeviceContaining(const CBofString &sName) {
	assert(isValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];
		if (pSDev && pSDev->GetObject(sName))
			return m_xStorageDeviceList[i];
	}
	return nullptr;
}

CBagStorageDev *CBagStorageDevManager::GetStorageDevice(const CBofString &sName) {
	assert(isValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];
		if (pSDev && (pSDev->GetName().getLength() == sName.getLength()) &&
		        !pSDev->GetName().find(sName))
			return m_xStorageDeviceList[i];
	}
	return nullptr;
}

bool CBagStorageDevManager::MoveObject(const CBofString &sDstName, const CBofString &sSrcName, const CBofString &sObjName) {
	assert(isValidObject(this));

	CBagStorageDev *pDstSDev = SDEV_MANAGER->GetStorageDevice(sDstName);

	// Find the storage device
	if (pDstSDev == nullptr)
		return false;

	CBagStorageDev *pSrcSDev = SDEV_MANAGER->GetStorageDevice(sSrcName);
	if (pSrcSDev == nullptr)
		return false;

	// Find the storage device
	if (pDstSDev->activateLocalObject(sObjName) != ERR_NONE)
		return false;
	if (pSrcSDev->deactivateLocalObject(sObjName) != ERR_NONE) {
		pDstSDev->deactivateLocalObject(sObjName);
		return false;
	}

	return true;
}

bool CBagStorageDevManager::AddObject(const CBofString &sDstName, const CBofString &sObjName) {
	assert(isValidObject(this));

	CBagStorageDev *pDstSDev = SDEV_MANAGER->GetStorageDevice(sDstName);

	// Find the storage device
	if (pDstSDev == nullptr)
		return false;

	// Find the storage device
	if (pDstSDev->activateLocalObject(sObjName) != ERR_NONE)
		return false;

	return true;
}


bool CBagStorageDevManager::removeObject(const CBofString &sSrcName, const CBofString &sObjName) {
	assert(isValidObject(this));

	CBagStorageDev *pSrcSDev = SDEV_MANAGER->GetStorageDevice(sSrcName);

	// Find the storage device
	if (pSrcSDev == nullptr)
		return false;

	// Find the storage device
	if (pSrcSDev->deactivateLocalObject(sObjName) != ERR_NONE)
		return false;

	return true;
}


int CBagStorageDevManager::GetObjectValue(const CBofString &sObject, const CBofString &sProperty) {
	assert(isValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];

		if (pSDev) {
			CBagObject *pObj = pSDev->GetObject(sObject);
			if (pObj != nullptr)
				return pObj->getProperty(sProperty);
		}
	}

	return 0;
}


// Set object will set a property to a numeric value or set the object to a string value -
// I am too lazy to write two funtions
void CBagStorageDevManager::SetObjectValue(const CBofString &sObject, const CBofString &sProperty, int nValue/*=DO_NOT_USE_THIS_VALUE*/) {
	assert(isValidObject(this));

	// Make sure that all objects are set and not just one?
	// Make sure that the first object is changed?
	for (int i = 0; i < m_xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];

		if (pSDev) {
			CBagObject *pObj = pSDev->GetObject(sObject);
			if (pObj != nullptr) {
				pObj->setProperty(sProperty, nValue);
			}
		}
	}
}


void CBagStorageDevManager::SaveObjList(ST_OBJ *pObjList, int nNumEntries) {
	assert(isValidObject(this));
	assert(pObjList != nullptr);


	int k = 0;
	int n = GetNumStorageDevices();
	for (int i = 0; i < n; i++) {
		CBagStorageDev *pSDev = GetStorageDevice(i);
		if (pSDev != nullptr) {

			int m = pSDev->GetObjectCount();
			for (int j = 0; j < m; j++) {
				CBagObject *pObj = pSDev->GetObjectByPos(j);

				if (!pObj->getRefName().isEmpty()) {
					assert(strlen(pObj->getRefName()) < MAX_OBJ_NAME);
					strncpy((pObjList + k)->m_szName, pObj->getRefName(), MAX_OBJ_NAME);

					// We MUST have put something in here
					assert((pObjList + k)->m_szName[0] != '\0');

					assert(strlen(pSDev->GetName()) < MAX_SDEV_NAME);
					strncpy((pObjList + k)->m_szSDev, pSDev->GetName(), MAX_SDEV_NAME);

					// Save if this guy is waiting to play
					(pObjList + k)->m_nFlags = (uint16)(pObj->isMsgWaiting() ? mIsMsgWaiting : 0);
					(pObjList + k)->m_lState = pObj->getState();
					(pObjList + k)->m_lProperties = pObj->getProperties();
					(pObjList + k)->m_lType = pObj->getType();
					(pObjList + k)->m_bUsed = 1;

					k++;
					assert(k < nNumEntries);

					// This is pretty dangerous, put up an error
					if (k >= nNumEntries) {
						bofMessageBox("SaveObjList encountered too many objects", "Internal Error");
						break;
					}
				}
			}
		}
	}
}


void CBagStorageDevManager::RestoreObjList(ST_OBJ *pObjList, int nNumEntries) {
	assert(isValidObject(this));
	assert(pObjList != nullptr);

	// Restore the state of all objects
	for (int i = 0; i < nNumEntries; i++) {
		if ((pObjList + i)->m_bUsed) {
			CBagStorageDev *pSDev = GetStorageDevice((pObjList + i)->m_szSDev);
			if (pSDev != nullptr) {
				CBagObject *pObj = pSDev->GetObject((pObjList + i)->m_szName);
				if (pObj != nullptr) {
					pObj->setProperties((pObjList + i)->m_lProperties);
					pObj->setState((pObjList + i)->m_lState);
					pObj->setType((BagObjectType)(pObjList + i)->m_lType);

					pObj->setMsgWaiting(((pObjList + i)->m_nFlags & mIsMsgWaiting) == mIsMsgWaiting);
				}
			}
		}
	}
}

bool CBagStorageDev::IsCIC() {
	// Slightly more complicated then before... if we're in a CIC, then return true,
	// however, if we're in the zoompda, then return the previous SDEV's cic value.
	if (m_bCIC != false) {
		return true;
	}

	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	sStr = "BPDAZ_WLD";
	if (m_sName == sStr) {
		return getCICStatus();
	}

	return false;
}

} // namespace Bagel
