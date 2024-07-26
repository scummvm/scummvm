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
CBagStorageDevWnd *g_lastWindow = nullptr;
extern bool g_pauseTimerFl;
extern bool g_waitOKFl;

// Statics
CBagEventSDev *CBagStorageDevWnd::_pEvtSDev = nullptr;    // Pointer to the EventSDev
CBofPoint *CBagStorageDev::_xCursorLocation;
CBofRect *CBagStorageDev::gRepaintRect;
bool CBagStorageDev::_hidePdaFl = false;
bool CBagStorageDev::_bHandledUpEvent = false;

// Dirty object variables
bool CBagStorageDev::_bPanPreFiltered = false;
bool CBagStorageDev::_bDirtyAllObjects = false;
bool CBagStorageDev::_bPreFilter = false;

int CBagStorageDevManager::nSDevMngrs = 0;

// Local globals
static int gLastBackgroundUpdate = 0;

#define kCursWidth 55

void CBagStorageDev::initialize() {
	g_allowPaintFl = true;
	g_bAAOk = true;
	g_allowattachActiveObjectsFl = true;
	g_lastWindow = nullptr;

	_xCursorLocation = new CBofPoint();
	gRepaintRect = new CBofRect();
	_hidePdaFl = false;
	_bHandledUpEvent = false;

	_bPanPreFiltered = false;
	_bDirtyAllObjects = false;
	_bPreFilter = false;

	gLastBackgroundUpdate = 0;
}

void CBagStorageDev::shutdown() {
	delete _xCursorLocation;
	delete gRepaintRect;
}

CBagStorageDev::CBagStorageDev() {
	_pLActiveObject = nullptr;        // The last object selected on mouse down

	_pAssociateWnd = nullptr;         // The associate window for attaching sounds

	_bForeignList = false;
	_pObjectList = new CBofList<CBagObject *>;
	_pExpressionList = nullptr;
	_nDiskID = 1;
	_bCloseup = false;
	_bCIC = false;

	// run object stuff
	_bFirstPaint = true;
	_nFloatPages = 0;
	_xSDevType = SDEV_UNDEF;
	_pBitmapFilter = nullptr;

	setCloseOnOpen(false);
	setExitOnEdge(0);
	setFilterId(0);
	setFadeId(0);

	// Default is this thing is not a customized sdev.
	setCustom(false);

	// Make sure all objects that are prefiltered are dirty
	setDirtyAllObjects(true);

	// Not sure what the hell is going on here...
	setLActivity(kMouseNONE);

	g_SDevManager->registerStorageDev(this);
}

CBagStorageDev::~CBagStorageDev() {
	if (!_bForeignList) {

		if (_pObjectList != nullptr) {
			// Delete all master sprite objects
			CBagStorageDev::releaseObjects();
			delete _pObjectList;
			_pObjectList = nullptr;
		}
		if (_pExpressionList) {
			while (_pExpressionList->getCount()) {
				CBagExpression *pExp = _pExpressionList->removeHead();
				delete pExp;
			}
			delete _pExpressionList;
			_pExpressionList = nullptr;
		}
	}  // If the lists belong to this storage device

	g_SDevManager->unregisterStorageDev(this);

	if (CBagStorageDevWnd::_pEvtSDev == this) {
		CBagStorageDevWnd::_pEvtSDev = nullptr;
	}
}


void CBagStorageDev::setPosition(const CBofPoint &pos) {
	CBofRect OrigRect = getRect();                  // Get the destination (screen) rect

	_cDestRect.setRect(pos.x, pos.y,
	                    pos.x + OrigRect.width() - 1,
	                    pos.y + OrigRect.height() - 1);
}


bool CBagStorageDev::contains(CBagObject *pObj, bool bActive) {
	int nCount = getObjectCount();
	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			if (pObj == getObjectByPos(i)) {
				if (bActive && (pObj->isActive()))
					return true;
			}
		}
	}
	return false;
}


ErrorCode CBagStorageDev::addObject(CBagObject *pObj, int /*nPos*/) {
	ErrorCode errorCode = ERR_NONE;

	// can't use a null pointer
	assert(pObj != nullptr);

	_pObjectList->addToTail(pObj);

	return errorCode;
}


ErrorCode CBagStorageDev::removeObject(CBagObject *pRObj) {
	ErrorCode errorCode = ERR_NONE;

	if (!_bForeignList) {
		int nCount = getObjectCount();
		for (int i = 0; i < nCount; ++i) {
			if (pRObj == getObjectByPos(i)) {
				_pObjectList->remove(i);
				return errorCode;
			}
		}
	}

	return errorCode;
}


ErrorCode CBagStorageDev::activateLocalObject(CBagObject  *pObj) {
	ErrorCode errCode = ERR_NONE;

	if (pObj != nullptr) {
		pObj->setLocal();
		if (!pObj->isActive() && (!pObj->getExpression() || pObj->getExpression()->evaluate(pObj->isNegative()))) {
			pObj->setActive();
			pObj->attach();

			// Perform an update and arrange objects in the storage device
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

	return activateLocalObject(getObject(sName));
}

ErrorCode CBagStorageDev::deactivateLocalObject(CBagObject *pObj) {
	ErrorCode errorCode = ERR_NONE;

	if (pObj != nullptr) {
		pObj->setLocal(false);
		if (pObj->isActive()) {
			pObj->setActive(false);
			pObj->detach();
		}
	} else  {
		errorCode = ERR_FFIND;
	}

	return errorCode;
}


ErrorCode CBagStorageDev::deactivateLocalObject(const CBofString &sName) {
	// Can't use a empty string
	assert(!sName.isEmpty());

	return deactivateLocalObject(getObject(sName));
}


CBofPoint CBagStorageDev::arrangeFloater(CBofPoint &nPos, CBagObject *pObj) {
	CBofPoint NextPos = nPos;

	if (getBackground() != nullptr) {
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

		int nPageNum = ((NextPos.y + nObjHeight) / nBackHeight);
		if (((NextPos.y + nObjHeight) % nBackHeight) != 0) {
			nPageNum++;
		}

		setNumFloatPages(nPageNum);

		NextPos.x += pObj->getRect().width();
	}

	return NextPos;
}


ErrorCode CBagStorageDev::attachActiveObjects() {
	ErrorCode errorCode = ERR_NONE;
	CBofPoint nArrangePos(0, 0);	// Removed 5,5 padding

	CBagLog::initArrangePages();

	int nCount = getObjectCount();
	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			if (g_engine->shouldQuit())
				return ERR_NONE;

			CBagObject *pObj = getObjectByPos(i);
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

							if (pObj->getType() == LINK_OBJ) {
								break;
							}
						}
					}

					if (pObj->isFloating()) {
						nArrangePos = arrangeFloater(nArrangePos, pObj);
					}
				} else if (pObj->isAttached()) {

					if (pObj->getType() != SOUND_OBJ || !((CBagSoundObject *)pObj)->isPlaying()) {
						pObj->setActive(false);
						pObj->detach();
					}
				}
			} else
				errorCode = ERR_FFIND;
		}
	}

	CBagLog::arrangePages();

	return errorCode;
}

ErrorCode CBagStorageDev::detachActiveObjects() {
	ErrorCode errorCode = ERR_NONE;
	int nCount = getObjectCount();

	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = getObjectByPos(i);
			if (pObj != nullptr) {
				if (pObj->isAttached()) {
					// If this object is not removed from memory, then
					// make sure it is drawn next time it is activated.
					pObj->setDirty(true);
					pObj->detach();
				}
			} else
				errorCode = ERR_FFIND;
		}
	}
	return errorCode;
}

ErrorCode CBagStorageDev::loadObjects() {
	ErrorCode errorCode = ERR_NONE;
	return errorCode;
}

ErrorCode CBagStorageDev::releaseObjects() {
	ErrorCode errorCode = ERR_NONE;
	int nCount = getObjectCount();

	if (!_bForeignList) {
		if (nCount) {
			for (int i = 0; i < nCount; ++i) {
				CBagObject *pObj = _pObjectList->removeHead();
				delete pObj;
			}
		}

		_pObjectList->removeAll();
	}
	return errorCode;
}


void CBagStorageDev::setObjectList(CBofList<CBagObject *> *pOList, CBofList<CBagExpression *> *pEList) {
	delete _pObjectList;

	_bForeignList    = true;
	_pObjectList     = pOList;
	_pExpressionList = pEList;
}


ErrorCode CBagStorageDev::paintStorageDevice(CBofWindow * /*pWnd*/, CBofBitmap *pBmp, CBofRect * /*pRect*/) {
	bool        bMouseOverObj = false;
	int         nCount        = getObjectCount();

	if (nCount) {
		CBofWindow *pWnd1 = CBagel::getBagApp()->getMasterWnd();
		if (pWnd1)
			pWnd1->screenToClient(&*_xCursorLocation);

		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = getObjectByPos(i);
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
		noObjectsUnderMouse();

	return ERR_NONE;
}

ErrorCode CBagStorageDev::onLActiveObject(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * /*vpInfo*/) {
	return ERR_NONE;
}


ErrorCode CBagStorageDev::noObjectsUnderMouse() {
	return ERR_NONE;
}

void CBagStorageDev::onMouseMove(uint32 nFlags, CBofPoint *xPoint, void *vpInfo) {
	*_xCursorLocation = *xPoint;

	if (getLActiveObject() && getLActivity()) {
		getLActiveObject()->onMouseMove(nFlags, *xPoint, vpInfo);
	}
}


ErrorCode CBagStorageDev::onMouseOver(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void *) {
	return ERR_NONE;
}


void CBagStorageDev::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *vpInfo) {
	if (CBagPDA::isMoviePlaying() && CBagMasterWin::getActiveCursor() == 6) {
		return;
	}

	*_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = devPtToViewPort(*xPoint);

	setLActivity(kMouseNONE);

	CBagObject *pObj = getObject(xCursorLocation, true);
	if ((pObj != nullptr) && (pObj->isActive())) {
		pObj->onLButtonDown(nFlags, xPoint, vpInfo);
		setLActivity(kMouseDRAGGING);
	}

	setLActiveObject(pObj);
}

// TODO: This is a global, move it to the main class
bool g_noMenuFl = false;


void CBagStorageDev::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *vpInfo) {
	char szLocalBuff[256];
	CBofString sCurrSDev(szLocalBuff, 256);

	if (CBagPDA::isMoviePlaying() && CBagMasterWin::getActiveCursor() == 6) {
		return;
	}

	sCurrSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->getName();

	*_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = devPtToViewPort(*xPoint);

	bool bUseWield = true;
	CBagObject *pObj = getObject(xCursorLocation, true);
	if (pObj != nullptr) {
		bUseWield = false;

		g_noMenuFl = false;
		if (pObj->isActive()) {
			pObj->onLButtonUp(nFlags, xPoint, vpInfo);

			if (g_noMenuFl) {
				g_noMenuFl = false;
				bUseWield = true;
			}
			setLActiveObject(pObj);
		}
		_bHandledUpEvent = true;
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
						setLActiveObject(pObj);
						_bHandledUpEvent = true;
					}
				}
			}
		}
	}

	setLActivity(kMouseNONE);

	if (g_bAAOk && (sCurrSDev == (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->getName()))) {
		attachActiveObjects();
	}

	g_bAAOk = true;
}


ErrorCode CBagStorageDev::loadFile(const CBofString &sWldName) {
	char szLocalBuff[256];
	CBofString sWldFileName(szLocalBuff, 256);

	sWldFileName = sWldName;

	fixPathName(sWldFileName);

	// Force buffer to be big enough so that the entire script
	// is pre-loaded
	int nLength = fileLength(sWldFileName);
	if (nLength <= 0)
		error("Unable to open or read %s", sWldFileName.getBuffer());

	char *pBuf = (char *)bofAlloc(nLength);
	CBagIfstream fpInput(pBuf, nLength);

	CBofFile cFile;
	cFile.open(sWldFileName);
	cFile.read(pBuf, nLength);
	cFile.close();

	CBagStorageDev::loadFileFromStream(fpInput, sWldFileName);

	bofFree(pBuf);

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

	releaseObjects();

	setName(sWldName);

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
		getAlphaNumFromStream(fpInput, sWorkStr);

		if (sWorkStr.isEmpty()) {
			parseAlertBox(fpInput, "Error in line No Operator:", __FILE__, __LINE__);

			bOperSet        = true;
		}

		if (!sWorkStr.find("SET")) {
			bRunActivation  = false;
			bOperSet        = true;
		} else if (!sWorkStr.find("HOLD")) {
			bHoldActivation = true;
			bOperSet        = true;
		} else if (!sWorkStr.find("RUN")) {
			bOperSet        = true;
		}

		fpInput.eatWhite();

		if (bOperSet) {
			// If we are not doing the default RUN get next argument
			getAlphaNumFromStream(fpInput, sWorkStr);
		}

		if (sWorkStr.isEmpty()) {
			parseAlertBox(fpInput, "Error in line:", __FILE__, __LINE__);
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
				getAlphaNumFromStream(fpInput, str);
				fpInput.eatWhite();
				_nDiskID = (uint16)atoi(str);
				if (fpInput.peek() == ';') {
					fpInput.getCh();
				}
			}
		} else if (!sWorkStr.find("HELP")) {
			fpInput.eatWhite();
			ch = (char)fpInput.getCh();
			if (ch == '=') {
				getAlphaNumFromStream(fpInput, str);
				fpInput.eatWhite();

				setHelpFilename(str);

				if (fpInput.peek() == ';') {
					fpInput.getCh();
				}
			}
		} else if (!sWorkStr.find("ENDIF")) {
			if (bElseExprList.isEmpty()) {
				parseAlertBox(fpInput, "Error: ENDIF without IF", __FILE__, __LINE__);
			} else {
				bElseExprList.removeHead();
			}
			if (pActiveExpr) {
				pActiveExpr = pActiveExpr->getPrevExpression();
			} else {
				CBofString str2("Unexpected ENDIF:");
				str2 += sWldName;
				parseAlertBox(fpInput, str2.getBuffer(), __FILE__, __LINE__);
			}
		} else if (!sWorkStr.find("IF")) {
			// Added a bPrevNeg to keep track of nested else-if's
			bool bPrevNeg = false;
			if (bElseExprList.getHead())
				bPrevNeg = bElseExprList.getHead()->getNodeItem();

			bElseExprList.addToHead((bool) false);

			// Added a bPrevNeg to keep track of nested else-if's
			CBagExpression *pExp = new CBagExpression(pActiveExpr, bPrevNeg);

			pExp->setInfo(fpInput);
			if (!_pExpressionList)
				_pExpressionList = new CBofList<CBagExpression * >;

			_pExpressionList->addToTail(pExp);
			pActiveExpr = pExp;
		} else if (!sWorkStr.find("ELSE")) {
			if (bElseExprList.isEmpty()) {
				parseAlertBox(fpInput, "Error: ELSE without IF", __FILE__, __LINE__);
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
			pObj = onNewRPObject(sWorkStr);

		} else if (!sWorkStr.find("EDO")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewEDObject(sWorkStr);

		} else if (!sWorkStr.find("DOS")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewDosObject(sWorkStr);
		} else if (!sWorkStr.find("SND")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewSoundObject(sWorkStr);
		} else if (!sWorkStr.find("BUT")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewButtonObject(sWorkStr);
		} else if (!sWorkStr.find("CHR")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewCharacterObject(sWorkStr);
		} else if (!sWorkStr.find("TNG")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewThingObject(sWorkStr);
		} else if (!sWorkStr.find("ARE")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewAreaObject(sWorkStr);
		} else if (!sWorkStr.find("VAR")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewVariableObject(sWorkStr);
		} else if (!sWorkStr.find("TXT")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewTextObject(sWorkStr);
		} else if (!sWorkStr.find("MOVIE")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewMovieObject(sWorkStr);
		} else if (!sWorkStr.find("COMMAND")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewCommandObject(sWorkStr);
		} else if (!sWorkStr.find("EXPR")) {
			getStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = onNewExpressionObject(sWorkStr);
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

			addObject(pObj);
		}

		fpInput.eatWhite();

	}  // While not eof

	if (fpInput.peek() == SDEV_END_DELIM)
		fpInput.getCh();

	fpInput.eatWhite();

	if (pActiveExpr) {
		CBofString str2("Mismatch in IF/ENDIF:");
		str2 += sWldName;
		parseAlertBox(fpInput, str2.getBuffer(), __FILE__, __LINE__);

		return ERR_UNKNOWN;
	}

	if (bAttach)
		return attachActiveObjects();

	// Add everything to the window
	return ERR_NONE;
}


int CBagStorageDev::getObjectCount() {
	if (_pObjectList != nullptr) {
		return _pObjectList->getCount();

	}

	return 0;
}


CBagObject *CBagStorageDev::getObjectByPos(int nIndex) {
	assert(_pObjectList != nullptr);
	assert((nIndex >= 0) && (nIndex < _pObjectList->getCount()));

	return _pObjectList->getNodeItem(nIndex);
}


CBagObject *CBagStorageDev::getObject(int nRefId, bool bActiveOnly) {
	assert(_pObjectList != nullptr);

	int nListLen = _pObjectList->getCount();

	for (int i = 0; i < nListLen; ++i) {
		CBagObject *pObj = getObjectByPos(i);

		if ((pObj->getRefId() == nRefId) && (!bActiveOnly || (pObj->isActive() && pObj->isAttached())))
			return pObj;
	}

	return nullptr;
}


CBagObject *CBagStorageDev::getObject(const CBofString &sName, bool bActiveOnly) {
	assert(_pObjectList != nullptr);

	CBagObject *pObjFound = nullptr;
	CBofListNode<CBagObject *> *pNode = _pObjectList->getHead();
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


CBagObject *CBagStorageDev::getObjectByType(const CBofString &sType, bool bActiveOnly) {
	assert(_pObjectList != nullptr);

	int nListLen = _pObjectList->getCount();

	for (int i = 0; i < nListLen; ++i) {
		CBagObject *pObj = getObjectByPos(i);

		if (bActiveOnly) {
			if (pObj->isActive() && !getStringTypeOfObject(pObj->getType()).find(sType))
				return pObj;
		} else if (!getStringTypeOfObject(pObj->getType()).find(sType))
			return pObj;
	}

	return nullptr;
}


CBagObject *CBagStorageDev::getObject(const CBofPoint &xPoint, bool bActiveOnly) {
	int nCount = getObjectCount();

	// Resolve in reverse order since the last painted is on top
	if (nCount != 0) {
		for (int i = nCount - 1; i >= 0; --i) {
			CBagObject *pObj = getObjectByPos(i);

			if (pObj->isInside(xPoint) && (!bActiveOnly || (pObj->isActive() && pObj->isAttached())))
				return pObj;
		}
	}
	return nullptr;
}


ParseCodes CBagStorageDev::setInfo(CBagIfstream &fpInput) {
	char szStr[256];
	szStr[0] = 0;
	CBofString str(szStr, 256);

	fpInput.eatWhite();

	char ch = (char)fpInput.getCh();
	if (ch == '=') {
		getAlphaNumFromStream(fpInput, str);
		fpInput.eatWhite();

		fixPathName(str);

		_sBackgroundName = str;

		if (fpInput.peek() == ';') {
			fpInput.getCh();
		}
	}

	return PARSING_DONE;
}


ErrorCode CBagStorageDev::attach() {
	// Assume no error
	ErrorCode errorCode = ERR_NONE;

	_bFirstPaint = true;

	if (!_sBackgroundName.isEmpty()) {
		CBofBitmap *pBmp = new CBofBitmap(_sBackgroundName);

		if (!pBmp->errorOccurred()) {
			setBackground(pBmp);
			errorCode = attachActiveObjects();
		} else {
			errorCode = ERR_FOPEN;
		}
	}

	return errorCode;
}


ErrorCode CBagStorageDev::detach() {
	// Must force people to not use a bad App's palette
	CBofApp::getApp()->setPalette(nullptr);
	setBackground(nullptr);

	// Notify the main window that we need to redraw the background filter.
	CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());
	if (pMainWin != nullptr) {
		((CBagPanWindow *)pMainWin)->setPreFilterPan(true);
	}

	return detachActiveObjects();
}


ErrorCode CBagStorageDev::close() {
	return ERR_NONE;
}


CBagObject *CBagStorageDev::onNewSpriteObject(const CBofString &) {
	return new CBagSpriteObject();
}


CBagObject *CBagStorageDev::onNewBitmapObject(const CBofString &) {
	return new CBagBmpObject();
}


CBagObject *CBagStorageDev::onNewLinkObject(const CBofString &) {
	return new CBagLinkObject();
}

CBagObject *CBagStorageDev::onNewRPObject(const CBofString &) {
	return new CBagRPObject();
}

CBagObject *CBagStorageDev::onNewEDObject(const CBofString &) {
	return new CBagEnergyDetectorObject();
}

CBagObject *CBagStorageDev::onNewDosObject(const CBofString &) {
	return new CBagDossierObject();
}

CBagObject *CBagStorageDev::onNewTextObject(const CBofString &) {
	return new CBagTextObject();
}

CBagObject *CBagStorageDev::onNewSoundObject(const CBofString &) {
	return new CBagSoundObject();
}


CBagObject *CBagStorageDev::onNewButtonObject(const CBofString &) {
	return new CBagButtonObject();
}


CBagObject *CBagStorageDev::onNewCharacterObject(const CBofString &) {
	return new CBagCharacterObject();
}

CBagObject *CBagStorageDev::onNewThingObject(const CBofString &) {
	return new CBagThingObject();
}

CBagObject *CBagStorageDev::onNewMovieObject(const CBofString &) {
	return new CBagMovieObject();
}

CBagObject *CBagStorageDev::onNewCommandObject(const CBofString &) {
	return new CBagCommandObject();
}


CBagObject *CBagStorageDev::onNewAreaObject(const CBofString &) {
	return new CBagAreaObject();
}


CBagObject *CBagStorageDev::onNewExpressionObject(const CBofString &) {
	return new CBagExpressionObject();
}


CBagObject *CBagStorageDev::onNewVariableObject(const CBofString &) {
	return new CBagVariableObject();
}


CBagObject *CBagStorageDev::onNewUserObject(const CBofString &sInit) {
	char szLocalBuff[256];
	CBofString s(szLocalBuff, 256);

	s = sInit;
	bofMessageBox(s.getBuffer(), "Could not find object type");

	return nullptr;
}

void CBagStorageDev::onSetFilter(bool (*filterFunction)(uint16 nFilterId, CBofBitmap *, CBofRect *)) {
	_pBitmapFilter = filterFunction;
}

FilterFunction CBagStorageDev::getFilter() {
	return _pBitmapFilter;
}


ErrorCode CBagStorageDev::preFilter(CBofBitmap *pBmp, CBofRect *pRect, CBofList<CBagObject *> *pList) {
	if (pBmp != nullptr) {

		// If we are not dirtying all the objects, then only fill up the viewport.
		CBofRect viewPortRect(80, 10, 559, 369);
		CBofRect *fillRect;
		if (getDirtyAllObjects()) {
			fillRect = pRect;
		} else {
			fillRect = &viewPortRect;
		}

		pBmp->fillRect(fillRect, RGB(0, 0, 0));
	}

	// Let pda know that we've been prefiltered
	setPreFiltered(true);

	if (getDirtyAllObjects()) {
		makeListDirty(_pObjectList);
		makeListDirty(pList);
	} else {
		setDirtyAllObjects(true);
	}

	return ERR_NONE;
}


void CBagStorageDev::makeListDirty(CBofList<CBagObject *> *pList) {
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
	_pWorkBmp = nullptr;

	// Set a default help file for when there is not one specified
	_sHelpFileName = "$SBARDIR\\GENERAL\\RULES\\DEFAULT.TXT";
	fixPathName(_sHelpFileName);

	_xSDevType = SDEV_WND;
}


CBagStorageDevWnd::~CBagStorageDevWnd() {
	assert(isValidObject(this));

	CBagStorageDevWnd::killWorkBmp();
}


ErrorCode CBagStorageDevWnd::attach() {
	char szLocalBuff[256];
	CBofString s(szLocalBuff, 256);

	s = getName();

	if (!getBackgroundName().isEmpty()) {
		// This should actually be moved to sbarapp, but the load file will then
		// need to be removed from the constructor.
		//CBofApp::getApp()->setMainWindow(this);

		// Associate this window with callbacks so that any public member function can
		// be accessed by objects inserted into this class.
		setAssociateWnd(this);

		CBofBitmap *pBmp  = new CBofBitmap(getBackgroundName());

		if ((pBmp->height() <= 0) || (pBmp->width() <= 0)) {
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

	setPreFilterPan(true);
	g_lastWindow = this;

	CBagStorageDev *pSDev = g_SDevManager->getStorageDevice("EVT_WLD");

	if (pSDev != nullptr) {
		// Have we allocated one yet ?
		if (_pEvtSDev == nullptr) {
			_pEvtSDev = (CBagEventSDev *)pSDev;
			_pEvtSDev->setAssociateWnd(this);
			if (!_pEvtSDev->isAttached())
				_pEvtSDev->attach();

			setTimer(EVAL_EXPR, 1000);
			g_pauseTimerFl = false;

		} else {
			// We already allocated one
			// We just need to re-associate the parent window and reset the timer
			_pEvtSDev->setAssociateWnd(this);

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
						CBagTurnEventSDev *pSDev = (CBagTurnEventSDev *) g_SDevManager->getStorageDevice("TURN_WLD");
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
	detachActiveObjects();

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
		setWorkBmp();
	} else {
		killBackdrop();
		killWorkBmp();
	}
	return _errCode;
}


ErrorCode CBagStorageDevWnd::setWorkBmp() {
	// Delete any previous work area
	killWorkBmp();

	CBofBitmap *pBmp = getBackground();
	if (pBmp != nullptr) {
		_pWorkBmp = new CBofBitmap(pBmp->width(), pBmp->height(), pBmp->getPalette());
		pBmp->paint(_pWorkBmp);
	}

	return _errCode;
}


ErrorCode CBagStorageDevWnd::killWorkBmp() {
	delete _pWorkBmp;
	_pWorkBmp = nullptr;

	return _errCode;
}

void CBagStorageDevWnd::onPaint(CBofRect *) {
	assert(isValidObject(this));

	paintScreen();
}

void CBagStorageDevWnd::onMainLoop() {
	assert(isValidObject(this));

	paintScreen();

	g_lastWindow = this;
}

ErrorCode CBagStorageDevWnd::paintScreen(CBofRect *pRect) {
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

	if (preFilterPan()) {
		preFilter(pBmp, pRect, nullptr);
		setPreFilterPan(false);

		if (_pWorkBmp != nullptr) {
			_pWorkBmp->paint(pBmp, pRect, pRect);
		}
	}

	paintStorageDevice(this, pBmp, pRect);


	if (isFiltered()) {
		uint16 nFilterId = getFilterId();
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
				setPreFilterPan(true);
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

	fixPathName(sWldFile);

	// Force buffer to be big enough so that the entire script
	// is pre-loaded
	int nLength = fileLength(sWldFile);
	if (nLength <= 0)
		reportError(ERR_FOPEN, "Unable to open file %s", sWldFile.getBuffer());
	else {
		char *pBuf = (char *)bofAlloc(nLength);
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
	}

	// Add everything to the window

	return _errCode;
}


void CBagStorageDevWnd::onClose() {
	CBofWindow::onClose();
	// destruct the main window
	destroyWindow();
}

void CBagStorageDevWnd::onMouseMove(uint32 n, CBofPoint *pPoint, void *) {
	CBagStorageDev::onMouseMove(n, pPoint, getAssociateWnd());

	if (CBagCursor::isSystemCursorVisible())
		return;

	CBagMasterWin::setActiveCursor(0);

	// If a zelda movie is playing then just give 'em the wait cursor
	// as we're not gonna allow them to do squat anyway.
	if (CBagPDA::isMoviePlaying()) {
		CBagMasterWin::setActiveCursor(6);
		return;
	}

	if (getExitOnEdge() && (pPoint->x < getExitOnEdge()) && (pPoint->y < 360 + 10) && !(getPrevSDev().isEmpty())) {
		CBagMasterWin::setActiveCursor(10);

	} else {
		// Added wield cursors
		bool bWield = false;
		if (CBagWield::getWieldCursor() >= 0 && !CBagCursor::isSystemCursorVisible()) {
			CBagMasterWin::setActiveCursor(CBagWield::getWieldCursor());
			bWield = true;
		}

		// Run through background object list and find if the cursor is over an object
		CBofList<CBagObject *> *pList = getObjectList();
		if (pList != nullptr) {
			CBofPoint cCursorLocation = devPtToViewPort(*_xCursorLocation);

			// Go through list backwards to find the 1st top-most object
			CBofListNode<CBagObject *> *pNode = pList->getTail();
			while (pNode != nullptr) {
				CBagObject *pObj = pNode->getNodeItem();

				// Change cursor as long as it's not a link to a closeup, or
				// link to another Pan, or a text menu, or button.
				//
				if (pObj->isAttached() && pObj->isInside(cCursorLocation)) {
					int nCursor = pObj->getOverCursor();
					if (!bWield || (nCursor == 2 || nCursor == 5 || nCursor == 55 || pObj->getType() == TEXT_OBJ || pObj->getType() == BUTTON_OBJ)) {
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
	if (CBagPDA::isMoviePlaying() && CBagMasterWin::getActiveCursor() == 6) {
		return;
	}

	CBagStorageDev::onLButtonDown(nFlags, xPoint, getAssociateWnd());
	CBofWindow::onLButtonDown(nFlags, xPoint);
}

void CBagStorageDevWnd::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	// If asynch movie playing in PDA don't react to mouse down (8033)
	// if it's not a wait cursor, then allow the user to access that hotspot.
	if (CBagPDA::isMoviePlaying() && CBagMasterWin::getActiveCursor() == 6) {
		return;
	}

	// React to a mouse up, it will probably involve drawing a new window...
	setPreFilterPan(true);

	if (getExitOnEdge() && xPoint->x < getExitOnEdge() && !(getPrevSDev().isEmpty())) {
		// Set the initial location as the last full panoramas position
		close();

	} else {
		CBagStorageDev::onLButtonUp(nFlags, xPoint, getAssociateWnd());
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
	_sHelpFileName = "$SBARDIR\\GENERAL\\RULES\\DEFAULT.TXT";
	fixPathName(_sHelpFileName);
}


ErrorCode CBagStorageDevDlg::attach() {
	assert(isValidObject(this));

	CBagStorageDev::attach();

	char szLocalBuff[256];
	CBofString s(szLocalBuff, 256);
	s = getName();

	CBofBitmap *pBmp = getBackground();
	CBofRect r;
	if (pBmp)
		r = pBmp->getRect();

	if (r.width() && r.height()) {
		create(s.getBuffer(), &r, CBagel::getBagApp()->getMasterWnd());

	} else {
		create(s.getBuffer(), nullptr, CBagel::getBagApp()->getMasterWnd());
	}

	setPreFilterPan(true);

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

	paintStorageDevice(this, pBmp, pRect);

	if (isFiltered()) {
		uint16 nFilterId = getFilterId();
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
	if (g_lastWindow) {
		g_lastWindow->setPreFilterPan(true);
	}

	paintScreen();
}


void CBagStorageDevDlg::onPaint(CBofRect *) {
	assert(isValidObject(this));

	paintScreen();

	validateAnscestors();

	CBagPanWindow::flushInputEvents();
}


ErrorCode CBagStorageDevDlg::paintScreen(CBofRect *pRect) {
	assert(isValidObject(this));

	if (_pBackdrop != nullptr) {
		CBagStorageDevWnd *pWin = g_lastWindow;

		if (pWin != nullptr) {
			CBofBitmap *pBmp = pWin->getBackdrop();

			if (pBmp != nullptr) {
				// Don't redraw the background window unless we have to.
				if (pWin->preFilterPan()) {
					CBofBitmap *pWorkBmp = pWin->getWorkBmp();
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

	fixPathName(sWldFile);

	// Force buffer to be big enough so that the entire script is pre-loaded
	int nLength = fileLength(sWldFile);
	if (nLength <= 0)
		reportError(ERR_FOPEN, "Unable to open file %s", sWldFile.getBuffer());
	else {
		char *pBuf = (char *)bofAlloc(nLength);
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

ErrorCode CBagStorageDevDlg::create(const char *pszName, CBofRect *pRect, CBofWindow *pParent, uint32 nControlID) {
	ErrorCode errorCode = CBofDialog::create(pszName, pRect, pParent, nControlID);
	setCapture();
	return errorCode;
}


void CBagStorageDevDlg::onClose() {
	delete _pDlgBackground;
	_pDlgBackground = nullptr;

	// Since our sdevdlg doesn't have a detach active objects
	// method, we will have to manually take all the existing objects and
	// tell them to redraw themselves should they ever be called on in life
	// to do as such.
	int nCount = getObjectCount();
	for (int i = 0; i < nCount; ++i) {
		CBagObject *pObj = getObjectByPos(i);
		if (pObj != nullptr) {
			pObj->setDirty(true);
		}
	}

	CBofDialog::onClose();

	destroy();		// Destruct the main window

	// Our dlog may have dirtied our backdrop, make sure it is redrawn.
	if (g_lastWindow != nullptr) {
		g_lastWindow->setPreFilterPan(true);
		g_lastWindow->paintScreen(nullptr);

		// This is fairly shameful, but for some reason, some
		// updates don't work in the above paintscreen and must be updated the
		// next time through.  Don't know why, would love to find out, but
		// running out of time.
		g_lastWindow->setPreFilterPan(true);
	}
}

void CBagStorageDevDlg::onMouseMove(uint32 n, CBofPoint *xPoint, void *) {
	CBagStorageDev::onMouseMove(n, xPoint, getAssociateWnd());
}

void CBagStorageDevDlg::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	CBagStorageDev::onLButtonDown(nFlags, xPoint, getAssociateWnd());
	CBofDialog::onLButtonDown(nFlags, xPoint);
}

void CBagStorageDevDlg::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	if (CBofDialog::getRect().ptInRect(*xPoint)) {
		CBagStorageDev::onLButtonUp(nFlags, xPoint, getAssociateWnd());
		CBofDialog::onLButtonUp(nFlags, xPoint);
	} else  {
		close();
	}
}

CBagStorageDevManager::CBagStorageDevManager() {
	++nSDevMngrs;
	assert(nSDevMngrs < 2);
}

CBagStorageDevManager::~CBagStorageDevManager() {
	assert(isValidObject(this));
	--nSDevMngrs;
	assert(nSDevMngrs >= 0);
	releaseStorageDevices();
	_xStorageDeviceList.removeAll();
}

ErrorCode CBagStorageDevManager::registerStorageDev(CBagStorageDev *pSDev) {
	assert(isValidObject(this));

	_xStorageDeviceList.addToTail(pSDev);

	return ERR_NONE;
}

ErrorCode CBagStorageDevManager::unregisterStorageDev(CBagStorageDev *pSDev) {
	assert(isValidObject(this));

	CBofListNode<CBagStorageDev *> *pList = _xStorageDeviceList.getHead();
	while (pList != nullptr) {
		if (pSDev == pList->getNodeItem()) {
			_xStorageDeviceList.remove(pList);
			break;
		}

		pList = pList->_pNext;
	}
	return ERR_NONE;
}

ErrorCode CBagStorageDevManager::releaseStorageDevices() {
	assert(isValidObject(this));

	while (_xStorageDeviceList.getCount()) {
		CBagStorageDev *pSDev = _xStorageDeviceList[0];

		// The CBagStorageDev destructor will remove it from the list
		delete pSDev;
	}

	return ERR_NONE;
}

CBagStorageDev *CBagStorageDevManager::getStorageDeviceContaining(CBagObject *pObj) {
	assert(isValidObject(this));

	for (int i = 0; i < _xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = _xStorageDeviceList[i];
		if (pSDev && pSDev->contains(pObj))
			return _xStorageDeviceList[i];
	}
	return nullptr;
}

CBagStorageDev *CBagStorageDevManager::getStorageDeviceContaining(const CBofString &sName) {
	assert(isValidObject(this));

	for (int i = 0; i < _xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = _xStorageDeviceList[i];
		if (pSDev && pSDev->getObject(sName))
			return _xStorageDeviceList[i];
	}
	return nullptr;
}

CBagStorageDev *CBagStorageDevManager::getStorageDevice(const CBofString &sName) {
	assert(isValidObject(this));

	for (int i = 0; i < _xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = _xStorageDeviceList[i];
		if (pSDev && (pSDev->getName().getLength() == sName.getLength()) &&
		        !pSDev->getName().find(sName))
			return _xStorageDeviceList[i];
	}
	return nullptr;
}

bool CBagStorageDevManager::moveObject(const CBofString &sDstName, const CBofString &sSrcName, const CBofString &sObjName) {
	assert(isValidObject(this));

	CBagStorageDev *pDstSDev = g_SDevManager->getStorageDevice(sDstName);

	// Find the storage device
	if (pDstSDev == nullptr)
		return false;

	CBagStorageDev *pSrcSDev = g_SDevManager->getStorageDevice(sSrcName);
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

bool CBagStorageDevManager::addObject(const CBofString &sDstName, const CBofString &sObjName) {
	assert(isValidObject(this));

	CBagStorageDev *pDstSDev = g_SDevManager->getStorageDevice(sDstName);

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

	CBagStorageDev *pSrcSDev = g_SDevManager->getStorageDevice(sSrcName);

	// Find the storage device
	if (pSrcSDev == nullptr)
		return false;

	// Find the storage device
	if (pSrcSDev->deactivateLocalObject(sObjName) != ERR_NONE)
		return false;

	return true;
}


int CBagStorageDevManager::getObjectValue(const CBofString &sObject, const CBofString &sProperty) {
	assert(isValidObject(this));

	for (int i = 0; i < _xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = _xStorageDeviceList[i];

		if (pSDev) {
			CBagObject *pObj = pSDev->getObject(sObject);
			if (pObj != nullptr)
				return pObj->getProperty(sProperty);
		}
	}

	return 0;
}


// Set object will set a property to a numeric value or set the object to a string value -
// I am too lazy to write two funtions
void CBagStorageDevManager::setObjectValue(const CBofString &sObject, const CBofString &sProperty, int nValue/*=DO_NOT_USE_THIS_VALUE*/) {
	assert(isValidObject(this));

	// Make sure that all objects are set and not just one?
	// Make sure that the first object is changed?
	for (int i = 0; i < _xStorageDeviceList.getCount(); ++i) {
		CBagStorageDev *pSDev = _xStorageDeviceList[i];

		if (pSDev) {
			CBagObject *pObj = pSDev->getObject(sObject);
			if (pObj != nullptr) {
				pObj->setProperty(sProperty, nValue);
			}
		}
	}
}


void CBagStorageDevManager::saveObjList(StObj *pObjList, int nNumEntries) {
	assert(isValidObject(this));
	assert(pObjList != nullptr);


	int k = 0;
	int n = getNumStorageDevices();
	for (int i = 0; i < n; i++) {
		CBagStorageDev *pSDev = getStorageDevice(i);
		if (pSDev != nullptr) {

			int m = pSDev->getObjectCount();
			for (int j = 0; j < m; j++) {
				CBagObject *pObj = pSDev->getObjectByPos(j);

				if (!pObj->getRefName().isEmpty()) {
					assert(strlen(pObj->getRefName()) < MAX_OBJ_NAME);
					Common::strlcpy((pObjList + k)->_szName, pObj->getRefName(), MAX_OBJ_NAME);

					// We MUST have put something in here
					assert((pObjList + k)->_szName[0] != '\0');

					assert(strlen(pSDev->getName()) < MAX_SDEV_NAME);
					Common::strlcpy((pObjList + k)->_szSDev, pSDev->getName(), MAX_SDEV_NAME);

					// Save if this guy is waiting to play
					(pObjList + k)->_nFlags = (uint16)(pObj->isMsgWaiting() ? mIsMsgWaiting : 0);
					(pObjList + k)->_lState = pObj->getState();
					(pObjList + k)->_lProperties = pObj->getProperties();
					(pObjList + k)->_lType = pObj->getType();
					(pObjList + k)->_bUsed = 1;

					k++;
					assert(k < nNumEntries);

					// This is pretty dangerous, put up an error
					if (k >= nNumEntries) {
						bofMessageBox("saveObjList encountered too many objects", "Internal Error");
						break;
					}
				}
			}
		}
	}
}


void CBagStorageDevManager::restoreObjList(StObj *pObjList, int nNumEntries) {
	assert(isValidObject(this));
	assert(pObjList != nullptr);

	// Restore the state of all objects
	for (int i = 0; i < nNumEntries; i++) {
		if ((pObjList + i)->_bUsed) {
			CBagStorageDev *pSDev = getStorageDevice((pObjList + i)->_szSDev);
			if (pSDev != nullptr) {
				CBagObject *pObj = pSDev->getObject((pObjList + i)->_szName);
				if (pObj != nullptr) {
					pObj->setProperties((pObjList + i)->_lProperties);
					pObj->setState((pObjList + i)->_lState);
					pObj->setType((BagObjectType)(pObjList + i)->_lType);

					pObj->setMsgWaiting(((pObjList + i)->_nFlags & mIsMsgWaiting) == mIsMsgWaiting);
				}
			}
		}
	}
}

bool CBagStorageDev::isCIC() {
	// Slightly more complicated then before... if we're in a CIC, then return true,
	// however, if we're in the zoompda, then return the previous SDEV's cic value.
	if (_bCIC != false) {
		return true;
	}

	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	sStr = "BPDAZ_WLD";
	if (_sName == sStr) {
		return getCICStatus();
	}

	return false;
}

} // namespace Bagel
