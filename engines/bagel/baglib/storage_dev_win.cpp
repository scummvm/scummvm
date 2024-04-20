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
#include "bagel/boflib/log.h"

namespace Bagel {

#define FPS_TEST 1

#if FPS_TEST
bool g_bFPSTest = false;
bool g_bFullTest = false;
double g_fFPSTotal = 0.0;
uint32 g_lFPSCount = 0;
static uint32 gLastFPSUpdate = 0;
#endif

// Globals (hacks)
bool g_bAllowPaint = true;
bool g_bAAOk = true;            // Prevent AttachActiveObjects() after a RUN LNK
bool g_bAllowAAO = true;        // Prevent AttachActiveObjects() after a RUN LNK
extern bool g_bPauseTimer;
extern bool g_bWaitOK;

// Static inits
CBagEventSDev *CBagStorageDevWnd::m_pEvtSDev = nullptr;    // Pointer to the EventSDev
CBofPoint   CBagStorageDev::m_xCursorLocation;
bool        CBagStorageDev::m_bHidePDA = false;
bool        CBagStorageDev::m_bHandledUpEvent = false;

// Dirty object variables
bool        CBagStorageDev::m_bPanPreFiltered = false;
bool        CBagStorageDev::m_bDirtyAllObjects = false;
bool        CBagStorageDev::m_bPreFilter = false;

int         CBagStorageDevManager::nSDevMngrs = 0;

// Local globals
static int gLastBackgroundUpdate = 0;
CBagStorageDevWnd *g_pLastWindow = nullptr;

CBofRect gRepaintRect;
#define kCursWidth 55


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
	m_bFirstPaint = true;

	m_nFloatPages = 0;

	m_xSDevType = SDEV_UNDEF;

	m_pBitmapFilter = nullptr;

	SetCloseOnOpen(false);
	SetExitOnEdge(0);
	SetFilterId(0);
	SetFadeId(0);

	// Default is this thing is not a customized sdev.
	SetCustom(false);

	// Make sure all objects that are prefiltered are dirty
	SetDirtyAllObjects(true);

	// Not sure what the hell is going on here...
#if !BOF_MAC
	SetLActivity(kMouseNONE);
#endif

	SDEVMNGR->RegisterStorageDev(this);
}

CBagStorageDev::~CBagStorageDev() {
	if (!m_bForiegnList) {

		if (m_pObjectList != nullptr) {
			ReleaseObjects();                            // Delete all master sprite objects
			delete m_pObjectList;
			m_pObjectList = nullptr;
		}
		if (m_pExpressionList) {
			while (m_pExpressionList->GetCount()) {
				CBagExpression *pExp = m_pExpressionList->RemoveHead();
				delete pExp;
			}
			delete m_pExpressionList;
			m_pExpressionList = nullptr;
		}
	}  // If the lists belong to this storage device

	SDEVMNGR->UnRegisterStorageDev(this);

	if (CBagStorageDevWnd::m_pEvtSDev == this) {
		CBagStorageDevWnd::m_pEvtSDev = nullptr;
	}
}


void CBagStorageDev::SetPosition(const CBofPoint &pos) {
	CBofRect        OrigRect;

	OrigRect =  GetRect();                  // Get the destination (screen) rect

	m_cDestRect.SetRect(pos.x, pos.y,
	                    pos.x + OrigRect.Width() - 1,
	                    pos.y + OrigRect.Height() - 1);
}


bool CBagStorageDev::Contains(CBagObject *pObj, bool bActive) {
	int nCount;
	if ((nCount = GetObjectCount()) != 0) {
		for (int i = 0; i < nCount; ++i) {
			if (pObj == GetObjectByPos(i)) {
				if (bActive && (pObj->IsActive()))
					return true;
			}
		}
	}
	return false;
}


ErrorCode CBagStorageDev::AddObject(CBagObject *pObj, int /*nPos*/) {
	ErrorCode errCode = ERR_NONE;

	// can't use a null pointer
	Assert(pObj != nullptr);

	m_pObjectList->AddToTail(pObj);

	return errCode;
}


ErrorCode CBagStorageDev::RemoveObject(CBagObject *pRObj) {
	ErrorCode errCode = ERR_NONE;
	int nCount = GetObjectCount();

	if (!m_bForiegnList) {
		for (int i = 0; i < nCount; ++i) {
			if (pRObj == GetObjectByPos(i)) {
				m_pObjectList->Remove(i);
				return errCode;
			}
		}
	}

	return errCode;
}


ErrorCode CBagStorageDev::ActivateLocalObject(CBagObject  *pObj) {
	ErrorCode errCode = ERR_NONE;

	if (pObj != nullptr) {
		pObj->SetLocal();
		if (!pObj->IsActive() && (!pObj->GetExpression() || pObj->GetExpression()->Evaluate(pObj->IsNegative()))) {
			pObj->SetActive();
			pObj->Attach();

			// Preform an update and arrange objects in the storage device
			if (g_bAllowAAO) {
				AttachActiveObjects();
			}
		}
	} else  {
		errCode = ERR_FFIND;
	}

	return errCode;
}


ErrorCode CBagStorageDev::ActivateLocalObject(const CBofString &sName) {
	// can't use a empty string
	Assert(!sName.IsEmpty());

	return ActivateLocalObject(GetObject(sName));
}

ErrorCode CBagStorageDev::DeactivateLocalObject(CBagObject *pObj) {
	ErrorCode  errCode = ERR_NONE;

	if (pObj != nullptr) {
		pObj->SetLocal(false);
		if (pObj->IsActive()) {
			pObj->SetActive(false);
			pObj->Detach();
		}
	} else  {
		errCode = ERR_FFIND;
	}

	return errCode;
}


ErrorCode CBagStorageDev::DeactivateLocalObject(const CBofString &sName) {
	// Can't use a empty string
	Assert(!sName.IsEmpty());

	return DeactivateLocalObject(GetObject(sName));
}


CBofPoint CBagStorageDev::ArrangeFloater(CBofPoint nPos, CBagObject *pObj) {
	CBofPoint NextPos = nPos;

	if (GetBackground() != nullptr) {

		int nPageNum = 0;

		int     nBackWidth = GetBackground()->Width();
		int     nBackHeight = GetBackground()->Height();
		int     nObjWidth = pObj->GetRect().Width();
		int     nObjHeight = pObj->GetRect().Height();

		// Check to see if the whole object can fit in, if it can't wrap
		if (NextPos.x > (nBackWidth - nObjWidth)) {
			NextPos.x = 0;
			NextPos.y += pObj->GetRect().Height();
		}
		pObj->SetPosition(NextPos);

		// Always round this figure up...

		nPageNum = ((NextPos.y + nObjHeight) / nBackHeight);
		if (((NextPos.y + nObjHeight) % nBackHeight) != 0) {
			nPageNum++;
		}

		SetNumFloatPages(nPageNum);

		NextPos.x += pObj->GetRect().Width();
	}

	return NextPos;
}


ErrorCode CBagStorageDev::AttachActiveObjects() {
	ErrorCode errCode = ERR_NONE;
	CBofPoint nArrangePos(0, 0);	// Removed 5,5 padding

	CBagLog::InitArrangePages();

	int nCount = GetObjectCount();
	if (nCount != 0) {
		SetContainsModal(false);

		for (int i = 0; i < nCount; ++i) {
			if (g_engine->shouldQuit())
				return ERR_NONE;

			CBagObject *pObj = GetObjectByPos(i);
			if (pObj != nullptr) {
				if (pObj->IsLocal() && (!pObj->GetExpression() || pObj->GetExpression()->Evaluate(pObj->IsNegative()))) {
					if (!pObj->IsAttached()) {
						pObj->SetActive();
						pObj->Attach();
					}

					// If we have already painted the storage device once
					if (pObj->IsImmediateRun()) {

						if (m_bFirstPaint == false) {

							pObj->RunObject();

							if (pObj->GetType() == LINKOBJ) {
								break;
							}
						}
					}
					if (pObj->IsModal())
						SetContainsModal(true);
					if (pObj->IsFloating()) {
						nArrangePos = ArrangeFloater(nArrangePos, pObj);
					}
				} else if (pObj->IsAttached()) {

					if (pObj->GetType() != SOUNDOBJ || !((CBagSoundObject *)pObj)->IsPlaying()) {
						pObj->SetActive(false);
						pObj->Detach();
					}
				}
			} else
				errCode = ERR_FFIND;
		}
	}

	CBagLog::ArrangePages();

	return errCode;
}

ErrorCode CBagStorageDev::DetachActiveObjects() {
	ErrorCode  errCode = ERR_NONE;
	int nCount = GetObjectCount();

	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = GetObjectByPos(i);
			if (pObj != nullptr) {
				if (pObj->IsAttached()) {
					// If this object is not removed from memory, then
					// make sure it is drawn next time it is activated.
					pObj->SetDirty(true);
					pObj->Detach();
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
				CBagObject *pObj = m_pObjectList->RemoveHead();
				delete pObj;
			}
		}

		m_pObjectList->RemoveAll();
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
		CBofWindow *pWnd1 = CBagel::GetBagApp()->GetMasterWnd();
		if (pWnd1)
			pWnd1->ScreenToClient(&m_xCursorLocation);

		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = GetObjectByPos(i);
			if (pObj->IsAttached()) {
				CBofRect xBmpRect = pObj->GetRect();
				CBofPoint pt = xBmpRect.TopLeft();
				xBmpRect.OffsetRect(-pt.x, -pt.y);

				if (pObj->IsVisible()) {
					if (pBmp) {
						// Only update dirty objects...
						if (pObj->IsDirty() || pObj->IsAlwaysUpdate()) {
							pObj->Update(pBmp, pt, &xBmpRect);
						}
					}
				}

				// if it is visible update it
				if (pObj->GetRect().PtInRect(m_xCursorLocation)) {
					pObj->OnMouseOver(0, m_xCursorLocation, this);
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

void CBagStorageDev::OnMouseMove(uint32 nFlags, CBofPoint *xPoint, void *vpInfo) {
	m_xCursorLocation = *xPoint;

	if (GetLActiveObject() && GetLActivity()) {
		GetLActiveObject()->OnMouseMove(nFlags, *xPoint, vpInfo);
	}
}


ErrorCode CBagStorageDev::OnMouseOver(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void *) {
	return ERR_NONE;
}


void CBagStorageDev::OnLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *vpInfo) {
	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::GetActiveCursor() == 6) {
		return;
	}

	m_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = DevPtToViewPort(*xPoint);

	SetLActivity(kMouseNONE);

	CBagObject *pObj = GetObject(xCursorLocation, true);
	if ((pObj != nullptr) && (pObj->IsActive())) {
		pObj->OnLButtonDown(nFlags, xPoint, vpInfo);
		SetLActivity(kMouseDRAGGING);
	}

	SetLActiveObject(pObj);
}

bool g_bNoMenu = false;


void CBagStorageDev::OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *vpInfo) {
	char szLocalBuff[256];
	CBofString sCurrSDev(szLocalBuff, 256);

	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::GetActiveCursor() == 6) {
		return;
	}

	sCurrSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetName();

	m_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = DevPtToViewPort(*xPoint);

	bool bUseWield = true;
	CBagObject *pObj = GetObject(xCursorLocation, true);
	if (pObj != nullptr) {
		bUseWield = false;

		g_bNoMenu = false;
		if (pObj->IsActive()) {
			pObj->OnLButtonUp(nFlags, xPoint, vpInfo);

			if (g_bNoMenu) {
				g_bNoMenu = false;
				bUseWield = true;
			}
			SetLActiveObject(pObj);
		}
		m_bHandledUpEvent = true;
	}

	if (bUseWield) {

		CBagel *pApp = CBagel::GetBagApp();
		if (pApp != nullptr) {
			CBagPanWindow *pWin = (CBagPanWindow *)pApp->GetMasterWnd();
			if (pWin != nullptr) {
				if (pWin->m_pWieldBmp != nullptr) {
					pObj = pWin->m_pWieldBmp->GetCurrObj();
					if (pObj != nullptr) {

						if (pObj->IsActive()) {
							pObj->OnLButtonUp(nFlags, xPoint, vpInfo);
							SetLActiveObject(pObj);
							m_bHandledUpEvent = true;
						}
					}
				}
			}
		}
	}

	SetLActivity(kMouseNONE);

	if (g_bAAOk && (sCurrSDev == (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetName()))) {
		AttachActiveObjects();
	}

	g_bAAOk = true;
}


ErrorCode CBagStorageDev::LoadFile(const CBofString &sWldName) {
	char szLocalBuff[256];
	CBofString sWldFileName(szLocalBuff, 256);

	sWldFileName = sWldName;

	MACROREPLACE(sWldFileName);

#if BOF_MAC
	// ios::nocreate does not appear to be defined in the mac
	// cw8 iostreams environment, so make sure that the file exists before
	// opening it, otherwise it will create the file.

	if (FileExists(sWldFileName) == false) {
		return false;
	}
#endif

	// Force buffer to be big enough so that the entire script
	// is pre-loaded
	int nLength = FileLength(sWldFileName);
	char *pBuf = (char *)BofAlloc(nLength);
	if (pBuf != nullptr) {
		bof_ifstream fpInput(pBuf, nLength);

		CBofFile cFile;
		cFile.Open(sWldFileName);
		cFile.Read(pBuf, nLength);
		cFile.Close();

		CBagStorageDev::LoadFileFromStream(fpInput, sWldFileName);

		BofFree(pBuf);
	}

	// Add everything to the window
	return ERR_NONE;
}


ErrorCode CBagStorageDev::LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName, bool bAttach) {
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

	fpInput.EatWhite();
	int ch = fpInput.Get();
	if (ch != SDEV_START_DELIM) {
		char szWarningMsg[256];
		CBofString s(szWarningMsg, 256);

		s = "Warning: { expected: at start of storage device: ";
		s += sWldName;
		BofMessageBox(s.GetBuffer(), "Warning");

		fpInput.putback((char)ch);
	}

	fpInput.EatWhite();

	while (/*fpInput &&*/ !fpInput.eof() && !(fpInput.peek() == SDEV_END_DELIM)) {
		bool bOperSet           = false;    // Set if an operator was found
		bool bHoldActivation    = false;    // Set if the object should be held
		bool bRunActivation     = true;     // Set if the object should be run instantly on attach

		// Get Operator SET or HOLD or RUN; on none RUN is default
		GetAlphaNumFromStream(fpInput, sWorkStr);

		if (sWorkStr.IsEmpty()) {
			ParseAlertBox(fpInput, "Error in line No Operator:", __FILE__, __LINE__);

			bOperSet        = true;
		}

		if (!sWorkStr.Find("SET")) {
			bRunActivation  = false;
			bHoldActivation = false;
			bOperSet        = true;

		} else if (!sWorkStr.Find("HOLD")) {
			bRunActivation  = true;
			bHoldActivation = true;
			bOperSet        = true;

		} else if (!sWorkStr.Find("RUN")) {
			bRunActivation  = true;
			bHoldActivation = false;
			bOperSet        = true;
		}

		fpInput.EatWhite();

		if (bOperSet) {
			// If we are not doing the default RUN get next argument
			GetAlphaNumFromStream(fpInput, sWorkStr);
		}

		if (sWorkStr.IsEmpty()) {
			ParseAlertBox(fpInput, "Error in line:", __FILE__, __LINE__);
		}

		fpInput.EatWhite();

		CBagObject *pObj = nullptr;

		if (!sWorkStr.Find("BKG")) {
			SetInfo(fpInput);
			if (bAttach && Attach()) {
				Assert(false);
			}

		} else if (!sWorkStr.Find("DISKID")) {
			fpInput.EatWhite();
			ch = (char)fpInput.Get();
			if (ch == '=') {
				GetAlphaNumFromStream(fpInput, str);
				fpInput.EatWhite();
				m_nDiskID = (uint16)atoi(str);
				if (fpInput.peek() == ';') {
					fpInput.Get();
				}
			}
		} else if (!sWorkStr.Find("HELP")) {
			fpInput.EatWhite();
			ch = (char)fpInput.Get();
			if (ch == '=') {
				GetAlphaNumFromStream(fpInput, str);
				fpInput.EatWhite();

				SetHelpFilename(str);

				if (fpInput.peek() == ';') {
					fpInput.Get();
				}
			}
		} else if (!sWorkStr.Find("ENDIF")) {
			if (bElseExprList.IsEmpty()) {
				ParseAlertBox(fpInput, "Error: ENDIF without IF", __FILE__, __LINE__);
			} else {
				bElseExprList.RemoveHead();
			}
			if (pActiveExpr) {
				pActiveExpr = pActiveExpr->GetPrevExpression();
			} else {
				CBofString str2("Unexpected ENDIF:");
				str2 += sWldName;
				ParseAlertBox(fpInput, str2.GetBuffer(), __FILE__, __LINE__);
			}
		} else if (!sWorkStr.Find("IF")) {
			// Added a bPrevNeg to keep track of nested else-if's
			bool bPrevNeg = false;
			if (bElseExprList.GetHead())
				bPrevNeg = bElseExprList.GetHead()->GetNodeItem();

			bElseExprList.AddToHead((bool) false);

			// Added a bPrevNeg to keep track of nested else-if's
			CBagExpression *pExp = new CBagExpression(pActiveExpr, bPrevNeg);

			Assert(pExp != nullptr);
			pExp->SetInfo(fpInput);
			if (!m_pExpressionList)
				m_pExpressionList = new CBofList<CBagExpression * >;
			Assert(m_pExpressionList != nullptr);
			m_pExpressionList->AddToTail(pExp);
			pActiveExpr = pExp;
		} else if (!sWorkStr.Find("ELSE")) {
			if (bElseExprList.IsEmpty()) {
				ParseAlertBox(fpInput, "Error: ELSE without IF", __FILE__, __LINE__);
			} else {
				bElseExprList.RemoveHead();
				bElseExprList.AddToHead((bool) true);
			}
		} else if (!sWorkStr.Find("BMP")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewSpriteObject(sWorkStr);
		} else if (!sWorkStr.Find("SPR")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewSpriteObject(sWorkStr);

		} else if (!sWorkStr.Find("LNK")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewLinkObject(sWorkStr);

		} else if (!sWorkStr.Find("RPO")) {
			// Allow residue printing objects
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewRPObject(sWorkStr);

		} else if (!sWorkStr.Find("EDO")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewEDObject(sWorkStr);

		} else if (!sWorkStr.Find("DOS")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewDosObject(sWorkStr);
		} else if (!sWorkStr.Find("SND")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewSoundObject(sWorkStr);
		} else if (!sWorkStr.Find("BUT")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewButtonObject(sWorkStr);
		} else if (!sWorkStr.Find("CHR")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewCharacterObject(sWorkStr);
		} else if (!sWorkStr.Find("TNG")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewThingObject(sWorkStr);
		} else if (!sWorkStr.Find("ARE")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewAreaObject(sWorkStr);
		} else if (!sWorkStr.Find("VAR")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewVariableObject(sWorkStr);
		} else if (!sWorkStr.Find("TXT")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewTextObject(sWorkStr);
		} else if (!sWorkStr.Find("MOVIE")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewMovieObject(sWorkStr);
		} else if (!sWorkStr.Find("COMMAND")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewCommandObject(sWorkStr);
		} else if (!sWorkStr.Find("EXPR")) {
			GetStringFromStream(fpInput, sWorkStr, "=", true);
			pObj = OnNewExpressionObject(sWorkStr);
		} else if (!sWorkStr.Find("REM") || !sWorkStr.Find("//")) {
			char s[256];
			fpInput.Get(s, 256);
		} else {
			pObj = OnNewUserObject(sWorkStr);
		}

		if (pObj != nullptr) {
			Assert(CBofObject::IsValidObject(pObj));

			if (!bElseExprList.IsEmpty() && bElseExprList[0]) {
				pObj->SetNegative();
			} else if (pActiveExpr && pActiveExpr->IsNegative()) {
				// If there is an active expression that is negative
				// i.e if NOT( blah blah blah)
				pObj->SetNegative();
			}

			// Determine if the object is an active object in this world
			if (!bHoldActivation) {
				pObj->SetLocal();
				if (!pActiveExpr || pActiveExpr->Evaluate(pObj->IsNegative())) {
					pObj->SetActive();
				}
			}
			if (bRunActivation) {
				pObj->SetImmediateRun();
			}

			pObj->SetInfo(fpInput);
			pObj->SetExpression(pActiveExpr);

			AddObject(pObj);
		}

		fpInput.EatWhite();

	}  // While not eof

	if (fpInput.peek() == SDEV_END_DELIM)
		fpInput.Get();

	fpInput.EatWhite();

	if (pActiveExpr) {
		CBofString str2("Mismatch in IF/ENDIF:");
		str2 += sWldName;
		ParseAlertBox(fpInput, str2.GetBuffer(), __FILE__, __LINE__);

		return ERR_UNKNOWN;
	}

	if (bAttach)
		return AttachActiveObjects();

	// Add everything to the window
	return ERR_NONE;
}


int CBagStorageDev::GetObjectCount() {
	if (m_pObjectList != nullptr) {
		return m_pObjectList->GetCount();

	}

	return 0;
}


CBagObject *CBagStorageDev::GetObjectByPos(int nIndex) {
	Assert(m_pObjectList != nullptr);
	Assert((nIndex >= 0) && (nIndex < m_pObjectList->GetCount()));

	return m_pObjectList->GetNodeItem(nIndex);
}


CBagObject *CBagStorageDev::GetObject(int nRefId, bool bActiveOnly) {
	Assert(m_pObjectList != nullptr);

	int nListLen = m_pObjectList->GetCount();

	for (int i = 0; i < nListLen; ++i) {
		CBagObject *pObj = GetObjectByPos(i);

		if ((pObj->GetRefId() == nRefId) && (!bActiveOnly || (pObj->IsActive() && pObj->IsAttached())))
			return pObj;
	}

	return nullptr;
}


CBagObject *CBagStorageDev::GetObject(const CBofString &sName, bool bActiveOnly) {
	Assert(m_pObjectList != nullptr);

	CBagObject *pObjFound = nullptr;
	CBofListNode<CBagObject *> *pNode = m_pObjectList->GetHead();
	while (pNode != nullptr) {
		CBagObject *pObj = pNode->GetNodeItem();

		if (pObj->GetRefName().Compare(sName) == 0) {
			pObjFound = pObj;
			break;
		}

		pNode = pNode->m_pNext;
	}
	if (bActiveOnly && (pObjFound != nullptr) && !pObjFound->IsActive())
		pObjFound = nullptr;

	return pObjFound;
}


CBagObject *CBagStorageDev::GetObjectByType(const CBofString &sType, bool bActiveOnly) {
	Assert(m_pObjectList != nullptr);

	int nListLen = m_pObjectList->GetCount();

	for (int i = 0; i < nListLen; ++i) {
		CBagObject *pObj = GetObjectByPos(i);

		if (bActiveOnly) {
			if (pObj->IsActive() && !GetStringTypeOfObject(pObj->GetType()).Find(sType))
				return pObj;
		} else if (!GetStringTypeOfObject(pObj->GetType()).Find(sType))
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

			if (pObj->IsInside(xPoint) && (!bActiveOnly || (pObj->IsActive() && pObj->IsAttached())))
				return pObj;
		}
	}
	return nullptr;
}


void CBagStorageDev::HandleError(ErrorCode errCode) {
}


PARSE_CODES CBagStorageDev::SetInfo(bof_ifstream &fpInput) {
	char szStr[256];
	szStr[0] = 0;
	CBofString str(szStr, 256);

	fpInput.EatWhite();

	char ch = (char)fpInput.Get();
	if (ch == '=') {
		GetAlphaNumFromStream(fpInput, str);
		fpInput.EatWhite();

		MACROREPLACE(str);

		m_sBackgroundName = str;

		if (fpInput.peek() == ';') {
			fpInput.Get();
		}
	}

	return PARSING_DONE;
}


ErrorCode CBagStorageDev::Attach() {
	// Assume no error
	ErrorCode errCode = ERR_NONE;

	m_bFirstPaint = true;

	if (!m_sBackgroundName.IsEmpty()) {
		CBofBitmap *pBmp = new CBofBitmap(m_sBackgroundName);

		if ((pBmp != nullptr) && !pBmp->ErrorOccurred()) {
			SetBackground(pBmp);
			errCode = AttachActiveObjects();
		} else {
			errCode = ERR_FOPEN;
		}
	}

	return errCode;
}


ErrorCode CBagStorageDev::Detach() {
	// Must force people to not use a bad App's palette
	CBofApp::GetApp()->SetPalette(nullptr);
	SetBackground(nullptr);

	// Notify the main window that we need to redraw the background filter.
	CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());
	if (pMainWin != nullptr) {
		((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);
	}

	return DetachActiveObjects();
}


ErrorCode CBagStorageDev::Close() {
	return ERR_NONE;
}


CBagObject *CBagStorageDev::OnNewSpriteObject(const CBofString &) {
	return new CBagSpriteObject();
}


CBagObject *CBagStorageDev::OnNewBitmapObject(const CBofString &) {
	return new CBagBmpObject();
}


CBagObject *CBagStorageDev::OnNewLinkObject(const CBofString &) {
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


CBagObject *CBagStorageDev::OnNewButtonObject(const CBofString &) {
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


CBagObject *CBagStorageDev::OnNewUserObject(const CBofString &str) {
	char szLocalBuff[256];
	CBofString s(szLocalBuff, 256);

	s = str;

	BofMessageBox(s.GetBuffer(), "Could not find object type");

	return nullptr;
}

void CBagStorageDev::OnSetFilter(bool (*filterFunction)(const uint16 nFilterid, CBofBitmap *, CBofRect *)) {
#if BOF_MAC && __POWERPC__
	m_pBitmapFilter = NewRoutineDescriptor((ProcPtr) filterFunction,
	                                       uppFilterProcInfo,
	                                       GetCurrentArchitecture());
#else
	m_pBitmapFilter = filterFunction;
#endif
}

FilterFunction CBagStorageDev::GetFilter() {
	return m_pBitmapFilter;
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

#if BOF_MAC || BOF_WINMAC
		// need our transparent color to be white
		pBmp->FillRect(fillRect, RGB(255, 255, 255));
#else
		pBmp->FillRect(fillRect, RGB(0, 0, 0));
#endif
	}

	// Let pda know that we've been prefiltered
	SetPreFiltered(true);

	if (GetDirtyAllObjects()) {
		MakeListDirty(m_pObjectList);
		MakeListDirty(pList);
	} else {
		SetDirtyAllObjects(true);
	}

	return ERR_NONE;
}


void CBagStorageDev::MakeListDirty(CBofList<CBagObject *> *pList) {
	if (pList) {
		int nCount = pList->GetCount();
		if (nCount != 0) {

			for (int i = 0; i < nCount; ++i) {
				CBagObject *pObj = pList->GetNodeItem(i);
				pObj->SetDirty(true);
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
	SetOnUpdate(true);
	SetCloseOnOpen(false);
	m_pWorkBmp = nullptr;

	// Set a default help file for when there is not one specified
	m_sHelpFileName = "$SBARDIR\\GENERAL\\RULES\\DEFAULT.TXT";
	MACROREPLACE(m_sHelpFileName);

	m_xSDevType = SDEV_WND;
}


CBagStorageDevWnd::~CBagStorageDevWnd() {
	Assert(IsValidObject(this));

	KillWorkBmp();
}


ErrorCode CBagStorageDevWnd::Attach() {
	char szLocalBuff[256];
	CBofString s(szLocalBuff, 256);

	s = GetName();

	if (!GetBackgroundName().IsEmpty()) {
		// This should actually be moved to sbarapp, but the load file will then
		// need to be removed from the constructor.
		//CBofApp::GetApp()->SetMainWindow(this);

		// Associate this window with callbacks so that any public member function can
		// be accessed by objects inserted into this class.
		SetAssociateWnd(this);

		CBofBitmap *pBmp  = new CBofBitmap(GetBackgroundName());

		if ((pBmp == nullptr) || (pBmp->Height() <= 0) || (pBmp->Width() <= 0)) {
			ReportError(ERR_FOPEN, "BarComputer Background Opened Failed");
		} else {

			SetBackground(pBmp);

			// Set the bagel crap
			CBofPalette *pPalette = pBmp->GetPalette();
			CBofApp::GetApp()->SetPalette(pPalette);
			CBofSprite::OpenLibrary(pPalette);

			CBofRect r;
			r = pBmp->GetRect();

			if (r.Width() && r.Height()) {
				Create(s.GetBuffer(), &r, CBagel::GetBagApp()->GetMasterWnd());

			} else {
				Create(s.GetBuffer(), nullptr, CBagel::GetBagApp()->GetMasterWnd());
			}

			Show();

			AttachActiveObjects();
		}

	} else {
		ReportError(ERR_UNKNOWN, "No background for this storage device window");
	}

	SetPreFilterPan(true);
	g_pLastWindow = this;

	CBagStorageDev *pSDev = SDEVMNGR->GetStorageDevice("EVT_WLD");

	if (pSDev != nullptr) {
		// Have we allocated one yet ?
		if (m_pEvtSDev == nullptr) {
			m_pEvtSDev = (CBagEventSDev *)pSDev;
			m_pEvtSDev->SetAssociateWnd(this);
			if (!m_pEvtSDev->IsAttached())
				m_pEvtSDev->Attach();

#if BOF_MAC
			SetTimer(EVAL_EXPR, 1000);  // null for now, will not play music.
#else
			SetTimer(EVAL_EXPR, 1000);
#endif
			g_bPauseTimer = false;

		} else {
			// We already allocated one
			// We just need to re-associate the parent window and reset the timer
			m_pEvtSDev->SetAssociateWnd(this);

#if BOF_MAC
			SetTimer(EVAL_EXPR, 1000);  // null for now, will not play music.
#else
			SetTimer(EVAL_EXPR, 1000);
#endif
			g_bPauseTimer = false;
		}
	}

	return m_errCode;
}

void CBagStorageDevWnd::OnTimer(uint32 nEventID) {
	Assert(IsValidObject(this));
	static bool bAlready = false;

	if (!g_bPauseTimer) {
		// Don't allow recursion
		if (!bAlready) {
			bAlready = true;
			// Evaluate the event storage device IF MOVIE NOT PLAYING
			if ((CBofApp::GetApp()->GetMainWindow())->IsEnabled() && nEventID == EVAL_EXPR) {
				if (m_pEvtSDev != nullptr) {
					m_pEvtSDev->EvaluateExpressions();

					// If our turncount was updated, then execute the event world
					// for the turncount dependent storage device.
					if (CBagEventSDev::GetEvalTurnEvents() == true) {
						CBagEventSDev::SetEvalTurnEvents(false);
						CBagTurnEventSDev *pSDev = (CBagTurnEventSDev *) SDEVMNGR->GetStorageDevice("TURN_WLD");
						if (pSDev != nullptr) {
							// If unable to execute event world, try again next time through.
							if (pSDev->EvaluateExpressions() == ERR_UNKNOWN) {
								CBagEventSDev::SetEvalTurnEvents(true);
							}
						}
					}
				}

				g_bWaitOK = true;
			}
			bAlready = false;
		}
	}
}


ErrorCode CBagStorageDevWnd::Detach() {
	DetachActiveObjects();

	CBofApp::GetApp()->SetPalette(nullptr);

	SetBackground(nullptr);
	CBofSprite::CloseLibrary();
	CBagStorageDev::Detach();

	KillTimer(EVAL_EXPR);

	Destroy();

	return m_errCode;
}

ErrorCode CBagStorageDevWnd::Close() {
	CBagel::GetBagApp()->GetMasterWnd()->SetStorageDev(GetPrevSDev(), false);

	return m_errCode;
}


ErrorCode CBagStorageDevWnd::SetBackground(CBofBitmap *pBmp) {
	if (pBmp) {
		SetBackdrop(pBmp);
		SetWorkBmp();
	} else {
		KillBackdrop();
		KillWorkBmp();
	}
	return m_errCode;
}


ErrorCode CBagStorageDevWnd::SetWorkBmp() {
	// Delete any previous work area
	KillWorkBmp();

	CBofBitmap *pBmp = GetBackground();
	if (pBmp != nullptr) {
		m_pWorkBmp = new CBofBitmap(pBmp->Width(), pBmp->Height(), pBmp->GetPalette());
		pBmp->Paint(m_pWorkBmp);
	}

	return m_errCode;
}


ErrorCode CBagStorageDevWnd::KillWorkBmp() {
	if (m_pWorkBmp != nullptr) {
		delete m_pWorkBmp;
		m_pWorkBmp = nullptr;
	}

	return m_errCode;
}

void CBagStorageDevWnd::OnPaint(CBofRect *) {
	Assert(IsValidObject(this));

	PaintScreen();
}

void CBagStorageDevWnd::OnMainLoop() {
	Assert(IsValidObject(this));

	PaintScreen();

	g_pLastWindow = this;
}

ErrorCode CBagStorageDevWnd::PaintScreen(CBofRect *pRect) {
	Assert(IsValidObject(this));

	if (m_pBackdrop != nullptr) {
		OnRender(m_pBackdrop, pRect);

#if FPS_TEST
		if (g_bFPSTest) {
			uint32 lTimerStop = TimerStop();
			if (lTimerStop != 0) {
				g_fFPSTotal += 1000L / lTimerStop;
				g_lFPSCount++;

				uint32 nCurTime = GetTimer();
				if (nCurTime > gLastFPSUpdate + 1000) {
					gLastFPSUpdate = nCurTime;
					CBofRect cRect(540, 2, 639, 22);
					m_pBackdrop->FillRect(&cRect, COLOR_BLACK);         // dirty object system causes overwriting of text
					PaintText(m_pBackdrop, &cRect, BuildString("%2.2f fps", g_fFPSTotal / g_lFPSCount), FONT_14POINT, TEXT_BOLD, RGB(255, 255, 255), JUSTIFY_RIGHT, FORMAT_TOP_RIGHT);
					cRect.SetRect(540, 23, 639, 43);
					m_pBackdrop->FillRect(&cRect, COLOR_BLACK);         // dirty object system causes overwriting of text
					PaintText(m_pBackdrop, &cRect, BuildString("Correction: %d", CBagPanWindow::GetRealCorrection()), FONT_14POINT, TEXT_BOLD, RGB(255, 255, 255), JUSTIFY_RIGHT, FORMAT_TOP_RIGHT);
				}
			}

			TimerStart();
		}
#endif

		if (g_bAllowPaint) {
			m_pBackdrop->Paint(this, pRect, pRect);
		}
	}

	if (m_bFirstPaint) {
		m_bFirstPaint = false;
		AttachActiveObjects();
	}

	return m_errCode;
}


ErrorCode CBagStorageDevWnd::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	if (PreFilterPan()) {
		PreFilter(pBmp, pRect, nullptr);
		SetPreFilterPan(false);

		if (m_pWorkBmp != nullptr) {
			m_pWorkBmp->Paint(pBmp, pRect, pRect);
		}
	}

	PaintStorageDevice(this, pBmp, pRect);


	if (IsFiltered()) {
		uint16 nFilterId = GetFilterId();
#if BOF_MAC && __POWERPC__
		CallUniversalProc(m_pBitmapFilter,
		                  uppFilterProcInfo,
		                  nFilterId, pBmp, pRect);
#else
		(*m_pBitmapFilter)(nFilterId, pBmp, pRect);
#endif
	}

	return m_errCode;
}


ErrorCode CBagStorageDevWnd::RunModal(CBagObject *pObj) {
	Assert(pObj != nullptr);

	if (pObj->IsModal() && pObj->IsActive()) {

		EventLoop eventLoop;
		CBofBitmap *pBmp = GetBackdrop();

		if (pBmp != nullptr) {
			while (!g_engine->shouldQuit() && !pObj->IsModalDone()) {
				// Make sure we redraw each and every frame!
				SetPreFilterPan(true);
				OnRender(pBmp, nullptr);
				if (g_bAllowPaint) {
					pBmp->Paint(this, 0, 0);
				}

				if (eventLoop.frame())
					break;
			}
		}
	}

	return m_errCode;
}



ErrorCode CBagStorageDevWnd::PaintObjects(CBofList<CBagObject *> * /*list*/, CBofBitmap * /*pBmp*/, CBofRect & /*viewRect*/, CBofList<CBofRect> * /*pUpdateArea*/, bool /*tempVar*/) {
	return m_errCode;
}

ErrorCode CBagStorageDevWnd::LoadFile(const CBofString &sFile) {
	char        szWldFile[256];
	szWldFile[0] = 0;
	CBofString sWldFile(szWldFile, 256);        // performance improvement

	if (sFile.IsEmpty())
		sWldFile = "StoreDev.Wld";
	else
		sWldFile = sFile;

	MACROREPLACE(sWldFile);

#if BOF_MAC
	// ios::nocreate does not appear to be defined in the mac
	// cw8 iostreams environment, so make sure that the file exists before
	// opening it, otherwise it will create the file.

	if (FileExists(sWldFile) == false) {
		ReportError(ERR_FFIND, "Could not find file %s", sWldFile.GetBuffer());
	}

#endif
	// Force buffer to be big enough so that the entire script
	// is pre-loaded
	int nLength = FileLength(sWldFile);
	char *pBuf = (char *)BofAlloc(nLength);
	if (pBuf != nullptr) {
		bof_ifstream fpInput(pBuf, nLength);

		CBofFile cFile;
		cFile.Open(sWldFile);
		cFile.Read(pBuf, nLength);
		cFile.Close();

		CBagStorageDev::LoadFileFromStream(fpInput, sWldFile);

		// If the window.IsCreated()
		//
		if (IsCreated())
			InvalidateRect(nullptr);

		BofFree(pBuf);

	} else {
		ReportError(ERR_MEMORY);
	}

	// Add everything to the window

	return m_errCode;
}


void CBagStorageDevWnd::OnClose() {
	CBofWindow::OnClose();
	DestroyWindow();                            // destruct the main window
}

void CBagStorageDevWnd::OnMouseMove(uint32 n, CBofPoint *pPoint, void *) {
	CBagStorageDev::OnMouseMove(n, pPoint, GetAssociateWnd());

	if (CBagCursor::isSystemCursorVisible())
		return;

	CBagMasterWin::SetActiveCursor(0);

	// If a zelda movie is playing then just give 'em the wait cursor
	// as we're not gonna allow them to do squat anyway.
	if (CBagPDA::IsMoviePlaying()) {
		CBagMasterWin::SetActiveCursor(6);
		return;
	}

	if (GetExitOnEdge() && (pPoint->x < GetExitOnEdge()) && (pPoint->y < 360 + 10) && !(GetPrevSDev().IsEmpty())) {
		CBagMasterWin::SetActiveCursor(10);

	} else {
		// Added wield cursors
		bool bWield = false;
		if (CBagWield::GetWieldCursor() >= 0 && !CBagCursor::isSystemCursorVisible()) {
			CBagMasterWin::SetActiveCursor(CBagWield::GetWieldCursor());
			bWield = true;
		}

		// Run thru background object list and find if the cursor is over an object
		CBofList<CBagObject *> *pList = GetObjectList();
		if (pList != nullptr) {
			CBofPoint cCursorLocation = DevPtToViewPort(m_xCursorLocation);

			// Go thru list backwards to find the 1st top-most object
			CBofListNode<CBagObject *> *pNode = pList->GetTail();
			while (pNode != nullptr) {
				CBagObject *pObj = pNode->GetNodeItem();

				// Change cursor as long as it's not a link to a closeup, or
				// link to another Pan, or a text menu, or button.
				//
				if (pObj->IsAttached() && pObj->IsInside(cCursorLocation)) {
					int nCursor = pObj->GetOverCursor();
					if (!bWield || (nCursor == 2 || nCursor == 5 || nCursor == 55 || pObj->GetType() == TEXTOBJ || pObj->GetType() == BUTTONOBJ)) {
						CBagMasterWin::SetActiveCursor(nCursor);
					}
					break;
				}

				pNode = pNode->m_pPrev;
			}
		}
	}
}

void CBagStorageDevWnd::OnLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	// If asynch movie playing in PDA don't react to mouse down (8033)
	//  if it's not a wait cursor, then allow the user to access that hotspot.
	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::GetActiveCursor() == 6) {
		return;
	}

	CBagStorageDev::OnLButtonDown(nFlags, xPoint, GetAssociateWnd());
	CBofWindow::OnLButtonDown(nFlags, xPoint);
}

void CBagStorageDevWnd::OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	// If asynch movie playing in PDA don't react to mouse down (8033)
	// if it's not a wait cursor, then allow the user to access that hotspot.
	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::GetActiveCursor() == 6) {
		return;
	}

	// React to a mouse up, it will probably involve drawing a new window...
	SetPreFilterPan(true);

	if (GetExitOnEdge() && xPoint->x < GetExitOnEdge() && !(GetPrevSDev().IsEmpty())) {
		// Set the initial location as the last full panoramas position
		Close();

	} else {
		CBagStorageDev::OnLButtonUp(nFlags, xPoint, GetAssociateWnd());
		CBofWindow::OnLButtonUp(nFlags, xPoint);
	}
}

void CBagStorageDevWnd::OnKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {
#if FPS_TEST
#if BOF_MAC
	case 'f':
	case 'F':
#else
	case BKEY_F8:
#endif
		SetPreFilterPan(true);
		g_bFPSTest = !g_bFPSTest;

		// Reset FPS info
		g_fFPSTotal = 0.0;
		g_lFPSCount = 0;
		break;
#endif

	default:
		CBofWindow::OnKeyHit(lKey, nRepCount);
		break;
	}
}


CBagStorageDevDlg::CBagStorageDevDlg() : CBofDialog() {
	m_xSDevType = SDEV_DLG;

	// Set a default help file for when there is not one specified
	//
	m_sHelpFileName = "$SBARDIR\\GENERAL\\RULES\\DEFAULT.TXT";
	MACROREPLACE(m_sHelpFileName);
}


ErrorCode CBagStorageDevDlg::Attach() {
	Assert(IsValidObject(this));

	CBagStorageDev::Attach();

	char szLocalBuff[256];
	CBofString s(szLocalBuff, 256);
	s = GetName();

	CBofBitmap *pBmp = GetBackground();
	CBofRect r;
	if (pBmp)
		r = pBmp->GetRect();

	if (r.Width() && r.Height()) {
		Create(s.GetBuffer(), &r, CBagel::GetBagApp()->GetMasterWnd());

	} else {
		Create(s.GetBuffer(), nullptr, CBagel::GetBagApp()->GetMasterWnd());
	}

	SetPreFilterPan(true);

	CBofDialog::DoModal();

	Destroy();

	return m_errCode;
}


ErrorCode CBagStorageDevDlg::Close() {
	ReleaseCapture();

	CBofDialog::Close();

	return m_errCode;
}


ErrorCode CBagStorageDevDlg::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	PaintStorageDevice(this, pBmp, pRect);

	if (IsFiltered()) {
		uint16 nFilterId = GetFilterId();
#if BOF_MAC && __POWERPC__
		CallUniversalProc(m_pBitmapFilter,
		                  uppFilterProcInfo,
		                  nFilterId, pBmp, pRect);
#else
		(*m_pBitmapFilter)(nFilterId, pBmp, pRect);
#endif
	}

	return m_errCode;
}


void CBagStorageDevDlg::OnMainLoop() {
	Assert(IsValidObject(this));

	// The background of a storage device might be in motion, i.e. it
	// might need updates, but since it is a background screen, it probably isn't that
	// important, so we'll update it 4 times / second.

	int nCurTime = GetTimer();
	gLastBackgroundUpdate = nCurTime;
	if (g_pLastWindow) {
		g_pLastWindow->SetPreFilterPan(true);
	}

	PaintScreen();
}


void CBagStorageDevDlg::OnPaint(CBofRect *) {
	Assert(IsValidObject(this));

	PaintScreen();

	ValidateAnscestors();

	CBagPanWindow::FlushInputEvents();
}


ErrorCode CBagStorageDevDlg::PaintScreen(CBofRect *pRect) {
	Assert(IsValidObject(this));

	if (m_pBackdrop != nullptr) {
		CBagStorageDevWnd *pWin = g_pLastWindow;

		if (pWin != nullptr) {
			CBofBitmap *pBmp = pWin->GetBackdrop();

			if (pBmp != nullptr) {
				// Don't redraw the background window unless we have to.
				if (pWin->PreFilterPan()) {
					CBofBitmap *pWorkBmp = pWin->GetWorkBmp();
					if (pWorkBmp != nullptr) {
						pWorkBmp->Paint(pBmp, pRect, pRect);
					}

					pWin->OnRender(pBmp, pRect);
				}

				OnRender(m_pBackdrop, pRect);
				CBofRect wrect(GetWindowRect());
				m_pBackdrop->Paint(pBmp, &wrect, nullptr);

				if (g_bAllowPaint) {
					pBmp->Paint(pWin, pRect, pRect);
				}
			}
		}
	}

	// Set the firstpaint flag and attach objects
	// to allow for immediate run objects to run
	if (m_bFirstPaint) {
		m_bFirstPaint = false;
		AttachActiveObjects();
	}

	return m_errCode;
}


ErrorCode CBagStorageDevDlg::PaintObjects(CBofList<CBagObject *> * /*list*/, CBofBitmap * /*pBmp*/, CBofRect & /*viewRect*/, CBofList<CBofRect> * /*pUpdateArea*/, bool /*tempVar*/) {
	return m_errCode;
}


ErrorCode CBagStorageDevDlg::LoadFile(const CBofString &sFile) {
	char        szWldFile[256];
	szWldFile[0] = 0;
	CBofString sWldFile(szWldFile, 256);

	if (sFile.IsEmpty())
		sWldFile = "StoreDev.Wld";
	else
		sWldFile = sFile;

	MACROREPLACE(sWldFile);

#if BOF_MAC
	// ios::nocreate does not appear to be defined in the mac
	// cw8 iostreams environment, so make sure that the file exists before
	// opening it, otherwise it will create the file.

	if (FileExists(sWldFile) == false) {
		return ERR_FOPEN;
	}
#endif

	// Force buffer to be big enough so that the entire script is pre-loaded
	int nLength = FileLength(sWldFile);
	char *pBuf = (char *)BofAlloc(nLength);
	if (pBuf != nullptr) {
		bof_ifstream fpInput(pBuf, nLength);

		CBofFile cFile;
		cFile.Open(sWldFile);
		cFile.Read(pBuf, nLength);
		cFile.Close();

		CBagStorageDev::LoadFileFromStream(fpInput, sWldFile);

		BofFree(pBuf);

		if (IsCreated())
			InvalidateRect(nullptr);
	}

	// Add everything to the window
	return m_errCode;
}

ErrorCode CBagStorageDevDlg::Create(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID) {
	ErrorCode rc = CBofDialog::Create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
	SetCapture();
	return rc;
}

ErrorCode CBagStorageDevDlg::Create(const char *pszName, CBofRect *pRect, CBofWindow *pParent, uint32 nControlID) {
	ErrorCode rc = CBofDialog::Create(pszName, pRect, pParent, nControlID);
	SetCapture();
	return rc;
}


void CBagStorageDevDlg::OnClose() {
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
				pObj->SetDirty(true);
			}
		}
	}

	CBofDialog::OnClose();

	Destroy();		// Destruct the main window

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

void CBagStorageDevDlg::OnMouseMove(uint32 n, CBofPoint *xPoint, void *) {
	CBagStorageDev::OnMouseMove(n, xPoint, GetAssociateWnd());
}

void CBagStorageDevDlg::OnLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	CBagStorageDev::OnLButtonDown(nFlags, xPoint, GetAssociateWnd());
	CBofDialog::OnLButtonDown(nFlags, xPoint);
}

void CBagStorageDevDlg::OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	if (CBofDialog::GetRect().PtInRect(*xPoint)) {
		CBagStorageDev::OnLButtonUp(nFlags, xPoint, GetAssociateWnd());
		CBofDialog::OnLButtonUp(nFlags, xPoint);
	} else  {
		Close();
	}
}

CBagStorageDevManager::CBagStorageDevManager() {
	Assert(++nSDevMngrs < 2);
}

CBagStorageDevManager::~CBagStorageDevManager() {
	Assert(IsValidObject(this));

	Assert(--nSDevMngrs >= 0);
	ReleaseStorageDevices();
	m_xStorageDeviceList.RemoveAll();
}

ErrorCode CBagStorageDevManager::RegisterStorageDev(CBagStorageDev *pSDev) {
	Assert(IsValidObject(this));

	m_xStorageDeviceList.AddToTail(pSDev);

	return ERR_NONE;
}

ErrorCode CBagStorageDevManager::UnRegisterStorageDev(CBagStorageDev *pSDev) {
	Assert(IsValidObject(this));

	CBofListNode<CBagStorageDev *> *pList = m_xStorageDeviceList.GetHead();
	while (pList != nullptr) {
		if (pSDev == pList->GetNodeItem()) {
			m_xStorageDeviceList.Remove(pList);
			break;
		}

		pList = pList->m_pNext;
	}
	return ERR_NONE;
}

ErrorCode CBagStorageDevManager::ReleaseStorageDevices() {
	Assert(IsValidObject(this));

	while (m_xStorageDeviceList.GetCount()) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[0];

		// The CBagStorageDev destructor will remove it from the list
		delete pSDev;
	}

	return ERR_NONE;
}

CBagStorageDev *CBagStorageDevManager::GetStorageDeviceContaining(CBagObject *pObj) {
	Assert(IsValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.GetCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];
		if (pSDev && pSDev->Contains(pObj))
			return m_xStorageDeviceList[i];
	}
	return nullptr;
}

CBagStorageDev *CBagStorageDevManager::GetStorageDeviceContaining(const CBofString &sName) {
	Assert(IsValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.GetCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];
		if (pSDev && pSDev->GetObject(sName))
			return m_xStorageDeviceList[i];
	}
	return nullptr;
}

CBagStorageDev *CBagStorageDevManager::GetStorageDevice(const CBofString &sName) {
	Assert(IsValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.GetCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];
		if (pSDev && (pSDev->GetName().GetLength() == sName.GetLength()) &&
		        !pSDev->GetName().Find(sName))
			return m_xStorageDeviceList[i];
	}
	return nullptr;
}

bool CBagStorageDevManager::MoveObject(const CBofString &sDstName, const CBofString &sSrcName, const CBofString &sObjName) {
	Assert(IsValidObject(this));

	CBagStorageDev *pDstSDev = SDEVMNGR->GetStorageDevice(sDstName);

	// Find the storage device
	if (pDstSDev == nullptr)
		return false;

	CBagStorageDev *pSrcSDev = SDEVMNGR->GetStorageDevice(sSrcName);
	if (pSrcSDev == nullptr)
		return false;

	// Find the storage device
	if (pDstSDev->ActivateLocalObject(sObjName) != ERR_NONE)
		return false;
	if (pSrcSDev->DeactivateLocalObject(sObjName) != ERR_NONE) {
		pDstSDev->DeactivateLocalObject(sObjName);
		return false;
	}

	return true;
}

bool CBagStorageDevManager::AddObject(const CBofString &sDstName, const CBofString &sObjName) {
	Assert(IsValidObject(this));

	CBagStorageDev *pDstSDev = SDEVMNGR->GetStorageDevice(sDstName);

	// Find the storage device
	if (pDstSDev == nullptr)
		return false;

	// Find the storage device
	if (pDstSDev->ActivateLocalObject(sObjName) != ERR_NONE)
		return false;

	return true;
}


bool CBagStorageDevManager::RemoveObject(const CBofString &sSrcName, const CBofString &sObjName) {
	Assert(IsValidObject(this));

	CBagStorageDev *pSrcSDev = SDEVMNGR->GetStorageDevice(sSrcName);

	// Find the storage device
	if (pSrcSDev == nullptr)
		return false;

	// Find the storage device
	if (pSrcSDev->DeactivateLocalObject(sObjName) != ERR_NONE)
		return false;

	return true;
}


int CBagStorageDevManager::GetObjectValue(const CBofString &sObject, const CBofString &sProperty) {
	Assert(IsValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.GetCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];

		if (pSDev) {
			CBagObject *pObj = pSDev->GetObject(sObject);
			if (pObj != nullptr)
				return pObj->GetProperty(sProperty);
		}
	}

	return 0;
}


// Set object will set a property to a numeric value or set the object to a string value -
// I am too lazy to write two funtions
void CBagStorageDevManager::SetObjectValue(const CBofString &sObject, const CBofString &sProperty, int nValue/*=DO_NOT_USE_THIS_VALUE*/) {
	Assert(IsValidObject(this));

	// Make sure that all objects are set and not just one?
	// Make sure that the first object is changed?
	for (int i = 0; i < m_xStorageDeviceList.GetCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];

		if (pSDev) {
			CBagObject *pObj = pSDev->GetObject(sObject);
			if (pObj != nullptr) {
				pObj->SetProperty(sProperty, nValue);
			}
		}
	}
}


void CBagStorageDevManager::SaveObjList(ST_OBJ *pObjList, int nNumEntries) {
	Assert(IsValidObject(this));
	Assert(pObjList != nullptr);


	int k = 0;
	int n = GetNumStorageDevices();
	for (int i = 0; i < n; i++) {
		CBagStorageDev *pSDev = GetStorageDevice(i);
		if (pSDev != nullptr) {

			int m = pSDev->GetObjectCount();
			for (int j = 0; j < m; j++) {
				CBagObject *pObj = pSDev->GetObjectByPos(j);

				if (!pObj->GetRefName().IsEmpty()) {
					Assert(strlen(pObj->GetRefName()) < MAX_OBJ_NAME);
					strncpy((pObjList + k)->m_szName, pObj->GetRefName(), MAX_OBJ_NAME);

					// We MUST have put something in here
					Assert((pObjList + k)->m_szName[0] != '\0');

					Assert(strlen(pSDev->GetName()) < MAX_SDEV_NAME);
					strncpy((pObjList + k)->m_szSDev, pSDev->GetName(), MAX_SDEV_NAME);

					// Save if this guy is waiting to play
					(pObjList + k)->m_nFlags = (uint16)(pObj->IsMsgWaiting() ? mIsMsgWaiting : 0);
					(pObjList + k)->m_lState = pObj->GetState();
					(pObjList + k)->m_lProperties = pObj->GetProperties();
					(pObjList + k)->m_lType = pObj->GetType();
					(pObjList + k)->m_bUsed = 1;

					k++;
					Assert(k < nNumEntries);

					// This is pretty dangerous, put up an error
					if (k >= nNumEntries) {
						BofMessageBox("SaveObjList encountered too many objects", "Internal Error");
						break;
					}
				}
			}
		}
	}
}


void CBagStorageDevManager::RestoreObjList(ST_OBJ *pObjList, int nNumEntries) {
	Assert(IsValidObject(this));
	Assert(pObjList != nullptr);

	// Restore the state of all objects
	for (int i = 0; i < nNumEntries; i++) {
		if ((pObjList + i)->m_bUsed) {
			CBagStorageDev *pSDev = GetStorageDevice((pObjList + i)->m_szSDev);
			if (pSDev != nullptr) {
				CBagObject *pObj = pSDev->GetObject((pObjList + i)->m_szName);
				if (pObj != nullptr) {
					pObj->SetProperties((pObjList + i)->m_lProperties);
					pObj->SetState((pObjList + i)->m_lState);
					pObj->SetType((BAG_OBJECT_TYPE)(pObjList + i)->m_lType);

					pObj->SetMsgWaiting(((pObjList + i)->m_nFlags & mIsMsgWaiting) == mIsMsgWaiting);
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
		return GetCICStatus();
	}

	return false;
}

} // namespace Bagel
