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

namespace Bagel {

#define FPS_TEST 1

#if FPS_TEST
BOOL g_bFPSTest = FALSE;
BOOL g_bFullTest = FALSE;
DOUBLE g_fFPSTotal = 0.0;
ULONG g_lFPSCount = 0;
static DWORD gLastFPSUpdate = 0;
#endif

// Globals (hacks)
BOOL g_bAllowPaint = TRUE;
BOOL g_bAAOk = TRUE;            // Prevent AttachActiveObjects() after a RUN LNK
BOOL g_bAllowAAO = TRUE;        // Prevent AttachActiveObjects() after a RUN LNK
extern BOOL g_bPauseTimer;
extern BOOL g_bWaitOK;

// static inits
CBagEventSDev *CBagStorageDevWnd::m_pEvtSDev = NULL;    // Pointer to the EventSDev
CBofPoint   CBagStorageDev::m_xCursorLocation;
BOOL        CBagStorageDev::m_bHidePDA = FALSE;
BOOL        CBagStorageDev::m_bHandledUpEvent = FALSE;

// dirty object variables
BOOL        CBagStorageDev::m_bPanPreFiltered = FALSE;
BOOL        CBagStorageDev::m_bDirtyAllObjects = FALSE;
BOOL        CBagStorageDev::m_bPreFilter = FALSE;

// jwl 10.11.96 to handle drawing of cursor backdrop
CBofBitmap *CBagStorageDev::m_pUnderCursorBmp = NULL;
BOOL        CBagStorageDev::m_bDrawCursorBackdrop = TRUE;

// Local prototypes
VOID       GetCurrentCursPos(CBagCursor *, INT *, INT *);
ERROR_CODE PaintCursor(CBofBitmap *pBmp);

// Local globals
static INT gLastBackgroundUpdate = 0;
CBagStorageDevWnd *g_pLastWindow = NULL;

CBofRect gRepaintRect;
#define kCursWidth 55


CBagStorageDev::CBagStorageDev() {
	m_pLActiveObject = NULL;        // The last object selected on mouse down
	//m_pRActiveObject = NULL;      // The last object selected on mouse down

	m_pAssociateWnd = NULL;         // The associate window for attaching sounds

	m_bForiegnList = FALSE;
	m_pObjectList = new CBofList<CBagObject *>;
	m_pExpressionList = NULL;
	m_nDiskID = 1;
	m_bCloseup = FALSE;
	m_bCIC = FALSE;

	// run object stuff - mdm 7/26/96
	m_bFirstPaint = TRUE;

	m_nFloatPages = 0;

	m_xSDevType = SDEV_UNDEF;

	m_pBitmapFilter = NULL;

	SetCloseOnOpen(FALSE);
	SetExitOnEdge(0);
	SetFilterId(0);
	SetFadeId(0);

	// jwl 1.13.97 default is this thing is not a customized sdev.
	SetCustom(FALSE);

	// jwl 10.21.96 make sure all objects that are prefiltered are dirty
	SetDirtyAllObjects(TRUE);

	// jwl 07.23.96 not sure what the hell is going on here...
#if !BOF_MAC
	SetLActivity(kMouseNONE);
#endif

	SDEVMNGR->RegisterStorageDev(this);
}

CBagStorageDev::~CBagStorageDev() {
	if (!m_bForiegnList) {

		if (m_pObjectList != NULL) {
			ReleaseObjects();                            // delete all master sprite objects
			delete m_pObjectList;
			m_pObjectList = NULL;
		}
		if (m_pExpressionList) {
			CBagExpression *pExp;
			while (m_pExpressionList->GetCount()) {
				pExp = m_pExpressionList->RemoveHead();
				delete pExp;
			}
			delete m_pExpressionList;
			m_pExpressionList = NULL;
		}
	}  // if the lists belong to this storage device

	SDEVMNGR->UnRegisterStorageDev(this);

	if (CBagStorageDevWnd::m_pEvtSDev == this) {
		CBagStorageDevWnd::m_pEvtSDev = NULL;
	}
}


VOID CBagStorageDev::SetPosition(const CBofPoint &pos) {
	CBofRect        OrigRect;

	OrigRect =  GetRect();                  // Get the destination (screen) rect

	m_cDestRect.SetRect(pos.x, pos.y,
	                    pos.x + OrigRect.Width() - 1,
	                    pos.y + OrigRect.Height() - 1);
}


BOOL CBagStorageDev::Contains(CBagObject *pObj, BOOL bActive) {
	int nCount;
	if ((nCount = GetObjectCount()) != 0) {
		for (int i = 0; i < nCount; ++i) {
			if (pObj == GetObjectByPos(i)) {
				if (bActive && (pObj->IsActive()))
					return TRUE;
			}
		}
	}
	return FALSE;
}


ERROR_CODE CBagStorageDev::AddObject(CBagObject *pObj, int /*nPos*/) {
	ERROR_CODE errCode = ERR_NONE;

	// can't use a null pointer
	Assert(pObj != NULL);

	m_pObjectList->AddToTail(pObj);

	return (errCode);
}


ERROR_CODE CBagStorageDev::RemoveObject(CBagObject *pRObj) {
	ERROR_CODE errCode = ERR_NONE;
	int nCount = GetObjectCount();

	if (!m_bForiegnList) {
		for (int i = 0; i < nCount; ++i) {
			if (pRObj == GetObjectByPos(i)) {
				m_pObjectList->Remove(i);
				return (errCode);
			}
		}
	}

	return (errCode);
}


ERROR_CODE CBagStorageDev::ActivateLocalObject(CBagObject  *pObj) {
	ERROR_CODE errCode = ERR_NONE;

	if (pObj != NULL) {
		pObj->SetLocal();
		if (!pObj->IsActive() && (!pObj->GetExpression() || pObj->GetExpression()->Evaluate(pObj->IsNegative()))) {
			pObj->SetActive();
			pObj->Attach();

			// Preform an update and arrange objects in the storage device
			//
			if (g_bAllowAAO) {
				AttachActiveObjects();
			}
		}
	} else  {
		errCode = ERR_FFIND;
	}

	return (errCode);
}


ERROR_CODE CBagStorageDev::ActivateLocalObject(const CBofString &sName) {
	// can't use a empty string
	Assert(!sName.IsEmpty());

	return ActivateLocalObject(GetObject(sName));
}

ERROR_CODE CBagStorageDev::DeactivateLocalObject(CBagObject *pObj) {
	ERROR_CODE  errCode = ERR_NONE;

	if (pObj != NULL) {
		pObj->SetLocal(FALSE);
		if (pObj->IsActive()) {
			pObj->SetActive(FALSE);
			pObj->Detach();
		}
	} else  {
		errCode = ERR_FFIND;
	}

	return (errCode);
}


ERROR_CODE CBagStorageDev::DeactivateLocalObject(const CBofString &sName) {
	// can't use a empty string
	Assert(!sName.IsEmpty());

	return DeactivateLocalObject(GetObject(sName));
}


CBofPoint CBagStorageDev::ArrangeFloater(CBofPoint nPos, CBagObject *pObj) {
	CBofPoint NextPos = nPos;

	if (GetBackground() != NULL) {

		INT nPageNum = 0;

		INT     nBackWidth = GetBackground()->Width();
		INT     nBackHeight = GetBackground()->Height();
		INT     nObjWidth = pObj->GetRect().Width();
		INT     nObjHeight = pObj->GetRect().Height();

		// Check to see if the whole object can fit in, if it can't wrap
		if (NextPos.x > (nBackWidth - nObjWidth)) {
			NextPos.x = 0;
			NextPos.y += pObj->GetRect().Height();
		}
		pObj->SetPosition(NextPos);

		// jwl 11.07.96 always round this figure up...

		nPageNum = ((NextPos.y + nObjHeight) / nBackHeight);
		if (((NextPos.y + nObjHeight) % nBackHeight) != 0) {
			nPageNum++;
		}

		SetNumFloatPages(nPageNum);

		NextPos.x += pObj->GetRect().Width();
	}

	return NextPos;
}


ERROR_CODE CBagStorageDev::AttachActiveObjects() {
	ERROR_CODE      errCode = ERR_NONE;
	CBagObject     *pObj;
	CBofPoint       nArrangePos(0, 0);          // removed 5,5 padding
	int             nCount;
	//CBagExpression*   pPrevExpr = NULL;       // To speed up the evaluation process the previous expression should
	//BOOL          pPrevEval = TRUE;           // be used to determine if the obj should be attached (code to be written)
	//CBagLog           *pLastFloater = NULL;

	CBagLog::InitArrangePages();

	if ((nCount = GetObjectCount()) != 0) {
		SetContainsModal(FALSE);

		for (int i = 0; i < nCount; ++i) {

			if ((pObj = GetObjectByPos(i)) != NULL) {
				if (pObj->IsLocal() && (!pObj->GetExpression() || pObj->GetExpression()->Evaluate(pObj->IsNegative()))) {
					if (!pObj->IsAttached()) {
						pObj->SetActive();
						pObj->Attach();
					}

					// if we have already painted the storage device once
					if (pObj->IsImmediateRun()) {

						if (m_bFirstPaint == FALSE) {

							pObj->RunObject();  //  pObj->Detach();

							if (pObj->GetType() == LINKOBJ) {
								break;
							}
						}
					}
					if (pObj->IsModal())
						SetContainsModal(TRUE);
					if (pObj->IsFloating()) {
						nArrangePos = ArrangeFloater(nArrangePos, pObj);
					}
				} else if (pObj->IsAttached()) {

					if (pObj->GetType() != SOUNDOBJ || !((CBagSoundObject *)pObj)->IsPlaying()) {
						pObj->SetActive(FALSE);
						pObj->Detach();
					}
				}
			} else
				errCode = ERR_FFIND;
		}
	}

	CBagLog::ArrangePages();

	return (errCode);
}

ERROR_CODE CBagStorageDev::DetachActiveObjects() {
	ERROR_CODE  errCode = ERR_NONE;
	CBagObject *pObj;
	int nCount;

	if ((nCount = GetObjectCount()) != 0) {

		for (int i = 0; i < nCount; ++i) {

			if ((pObj = GetObjectByPos(i)) != NULL) {
				if (pObj->IsAttached()) {
					// jwl 10.11.96 if this object is not removed from memory, then
					// make sure it is drawn next time it is activated.
					pObj->SetDirty(TRUE);
					pObj->Detach();
				}
			} else
				errCode = ERR_FFIND;
		}
	}
	return (errCode);
}

ERROR_CODE CBagStorageDev::LoadObjects() {
	ERROR_CODE errCode = ERR_NONE;
	return (errCode);
}

ERROR_CODE CBagStorageDev::ReleaseObjects() {
	ERROR_CODE errCode = ERR_NONE;
	CBagObject *pObj;
	int nCount = GetObjectCount();

	if (!m_bForiegnList) {
		if (nCount) {
			for (int i = 0; i < nCount; ++i) {
				pObj = m_pObjectList->RemoveHead();
				delete pObj;
			}
		}

		m_pObjectList->RemoveAll();
	}
	return (errCode);
}


VOID CBagStorageDev::SetObjectList(CBofList<CBagObject *> *pOList, CBofList<CBagExpression *> *pEList) {
	//if (m_pObjectList != NULL) {
	delete m_pObjectList;
	//}
	m_bForiegnList    = TRUE;
	m_pObjectList     = pOList;
	m_pExpressionList = pEList;
}


ERROR_CODE CBagStorageDev::PaintStorageDevice(CBofWindow * /*pWnd*/, CBofBitmap *pBmp, CBofRect * /*pRect*/) {
	BOOL        bMouseOverObj = FALSE;
	int         nCount        = GetObjectCount();

	if (nCount) {

		//m_xCursorLocation = GetMousePos();

		CBofWindow *pWnd1 = CBagel::GetBagApp()->GetMasterWnd();
		if (pWnd1)
			pWnd1->ScreenToClient(&m_xCursorLocation);

		CBagObject *pObj;
		for (int i = 0; i < nCount; ++i) {
			pObj =  GetObjectByPos(i);
			if (pObj->IsAttached()) {
				CBofRect xBmpRect = pObj->GetRect();
				CBofPoint pt = xBmpRect.TopLeft();
				xBmpRect.OffsetRect(-pt.x, -pt.y);

				if (pObj->IsVisible()) {
					if (pBmp) {
						// jwl 10.11.96 only update dirty objects...
						if (pObj->IsDirty() || pObj->IsAlwaysUpdate()) {
							pObj->Update(pBmp, pt, &xBmpRect);
						}
						//if (pObj->IsHighlight())
						//    pBmp->DrawRect(&(pObj->GetRect()), RGB(0,0,0)/*RGB(255,255,255)*/);
					}
				}  // if it is visible update it

				if (pObj->GetRect().PtInRect(m_xCursorLocation)) {
					pObj->OnMouseOver(0, m_xCursorLocation, this);
					bMouseOverObj = TRUE;
				}  // if on screen
			}  // If the object is attached

		}  // While a valid object
	}   // if there is any valid objects

	if (!bMouseOverObj)
		NoObjectsUnderMouse();

	return (ERR_NONE);
}

ERROR_CODE CBagStorageDev::OnLActiveObject(UINT /*nFlags*/, CBofPoint * /*xPoint*/, void * /*vpInfo*/) {
	return (ERR_NONE);
}


ERROR_CODE CBagStorageDev::NoObjectsUnderMouse() {
	//::SetCursor(::LoadCursor(NULL,IDC_ARROW));

	return (ERR_NONE);
}

void CBagStorageDev::OnMouseMove(UINT nFlags, CBofPoint *xPoint, void *vpInfo) {
	m_xCursorLocation = *xPoint;

	if (GetLActiveObject() && GetLActivity()) {
		GetLActiveObject()->OnMouseMove(nFlags, *xPoint, vpInfo);
	}

	/*if (GetRActiveObject() && GetRActivity()) {
	    GetRActiveObject()->OnMouseMove(nFlags, *xPoint, vpInfo);
	}*/

	return;
}


ERROR_CODE CBagStorageDev::OnMouseOver(UINT /*nFlags*/, CBofPoint * /*xPoint*/, void *) {
	return (ERR_NONE);
}


void CBagStorageDev::OnLButtonDown(UINT nFlags, CBofPoint *xPoint, void *vpInfo) {
	CBagObject *pObj;

	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::GetActiveCursor() == 6) {
		return;
	}

	m_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = DevPtToViewPort(*xPoint);

	SetLActivity(kMouseNONE);

	if ((pObj = GetObject(xCursorLocation, TRUE)) != NULL) {
		if (pObj->IsActive()) {
			pObj->OnLButtonDown(nFlags, xPoint, vpInfo);
			SetLActivity(kMouseDRAGGING);
		}
	}

	SetLActiveObject(pObj);
}

BOOL g_bNoMenu = FALSE;


void CBagStorageDev::OnLButtonUp(UINT nFlags, CBofPoint *xPoint, VOID *vpInfo) {
	CBagObject *pObj;
	CHAR       szLocalBuff[256];
	CBofString sCurrSDev(szLocalBuff, 256);

	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::GetActiveCursor() == 6) {
		return;
	}

	BOOL bUseWield;
	sCurrSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetName();

	m_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = DevPtToViewPort(*xPoint);

	bUseWield = TRUE;
	if (((pObj = GetObject(xCursorLocation, TRUE)) != NULL) /*&& (pObj == GetLActiveObject())*/) {
		bUseWield = FALSE;

		g_bNoMenu = FALSE;
		if (pObj->IsActive()) {
			pObj->OnLButtonUp(nFlags, xPoint, vpInfo);

			if (g_bNoMenu) {
				g_bNoMenu = FALSE;
				bUseWield = TRUE;
			}
			SetLActiveObject(pObj);
		}
		m_bHandledUpEvent = TRUE;
	}

	if (bUseWield) {

		CBagel *pApp;
		CBagPanWindow *pWin;

		if ((pApp = CBagel::GetBagApp()) != NULL) {

			if ((pWin = (CBagPanWindow *)pApp->GetMasterWnd()) != NULL) {
				if (pWin->m_pWieldBmp != NULL) {

					if ((pObj = pWin->m_pWieldBmp->GetCurrObj()) != NULL) {

						if (pObj->IsActive()) {
							//pObj->RunObject();
							pObj->OnLButtonUp(nFlags, xPoint, vpInfo);
							SetLActiveObject(pObj);
							m_bHandledUpEvent = TRUE;
						}
					}
				}
			}
		}
	}

	SetLActivity(kMouseNONE);
	//SetLActiveObject(NULL);

	if (g_bAAOk && (sCurrSDev == (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetName()))) {
		AttachActiveObjects();
	}
	g_bAAOk = TRUE;
}


ERROR_CODE CBagStorageDev::LoadFile(const CBofString &sWldName) {
	CHAR szLocalBuff[256];
	CBofString sWldFileName(szLocalBuff, 256);

	sWldFileName = sWldName;

	MACROREPLACE(sWldFileName);

#if BOF_MAC
	// jwl 07.23.96 ios::nocreate does not appear to be defined in the mac
	// cw8 iostreams environment, so make sure that the file exists before
	// opening it, otherwise it will create the file.

	if (FileExists(sWldFileName) == FALSE) {
		return FALSE;
	}
#endif

	CHAR *pBuf;
	INT nLength;

	// Force buffer to be big enough so that the entire script
	// is pre-loaded
	//
	nLength = FileLength(sWldFileName);
	if ((pBuf = (CHAR *)BofAlloc(nLength)) != NULL) {
		bof_ifstream fpInput(pBuf, nLength);

		CBofFile cFile;
		cFile.Open(sWldFileName);
		cFile.Read(pBuf, nLength);
		cFile.Close();

		CBagStorageDev::LoadFileFromStream(fpInput, sWldFileName);

		BofFree(pBuf);
	}

	// Add everything to the window
	return (ERR_NONE);
}


ERROR_CODE CBagStorageDev::LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName, BOOL bAttach) {
	CHAR                szWorkStr[256];
	CHAR                szStr[256];
	szWorkStr[0] = 0;
	szStr[0] = 0;
	CBofString          sWorkStr(szWorkStr, 256);
	CBofString          str(szStr, 256);
	CBagObject          *pObj;
	int                 ch;
	CBagExpression      *pActiveExpr = NULL;
	CBofList<BOOL>      bElseExprList;

	ReleaseObjects();

	SetName(sWldName);

	fpInput.EatWhite();
	if ((ch = fpInput.Get()) != SDEV_START_DELIM) {
		CHAR szWarningMsg[256];
		CBofString s(szWarningMsg, 256);

		s = "Warning: { expected: at start of storage device: ";
		s += sWldName;
		BofMessageBox(s.GetBuffer(), "Warning");
		//ParseAlertBox(CBofApp::GetApp()->GetMainWindow(), istr, "Error in expression:",__FILE__,__LINE__);
		fpInput.putback((CHAR)ch);
		//return ERR_UNKNOWN;
	}

	fpInput.EatWhite();

//LOGINFO("Opening SDEV:" << sWldName)

	while (/*fpInput &&*/ !fpInput.eof() && !(fpInput.peek() == SDEV_END_DELIM)) {
		BOOL bOperSet           = FALSE;    // Set if an operator was found
		BOOL bHoldActivation    = FALSE;    // Set if the object should be held
		BOOL bRunActivation     = TRUE;     // Set if the object should be run instantly on attach

		// Get Operator SET or HOLD or RUN; on none RUN is default
		GetAlphaNumFromStream(fpInput, sWorkStr);

		if (sWorkStr.IsEmpty()) {
			ParseAlertBox(fpInput, "Error in line No Operator:", __FILE__, __LINE__);

			bOperSet        = TRUE;

//LOGINFO("Error before line No Operator:" << s)
		}

		if (!sWorkStr.Find("SET")) {
			bRunActivation  = FALSE;
			bHoldActivation = FALSE;
			bOperSet        = TRUE;

//LOGINFO("SET ");

		} else if (!sWorkStr.Find("HOLD")) {
			bRunActivation  = TRUE;
			bHoldActivation = TRUE;
			bOperSet        = TRUE;

//LOGINFO("HOLD ");

		} else if (!sWorkStr.Find("RUN")) {
			bRunActivation  = TRUE;
			bHoldActivation = FALSE;
			bOperSet        = TRUE;
//LOGINFO("RUN ");
		}

		fpInput.EatWhite();

		if (bOperSet) {     // if we are not doing the default RUN get next argument
			GetAlphaNumFromStream(fpInput, sWorkStr);
		}
		if (sWorkStr.IsEmpty()) {
			//char s[256];
			//fpInput.getline(s,255);
			//int n = strlen(s);
			//fpInput.getline(&s[n],255-n);
			//BofMessageBox(s, "Error before line ");
			ParseAlertBox(fpInput, "Error in line:", __FILE__, __LINE__);

//LOGINFO("Error before line:" << s << endl;
		}

		fpInput.EatWhite();

		pObj = NULL;

		if (!sWorkStr.Find("BKG")) {
			SetInfo(fpInput);
			if (bAttach && Attach()) {
				Assert(FALSE);
			}
			//LOGINFO("  BKG:"); << m_sBackgroundName << endl;

		} else if (!sWorkStr.Find("DISKID")) {
			fpInput.EatWhite();
			if ((ch = (CHAR)fpInput.Get()) == '=') {
				GetAlphaNumFromStream(fpInput, str);
				fpInput.EatWhite();
				m_nDiskID = (USHORT)atoi(str);
				if (fpInput.peek() == ';') {
					fpInput.Get();
				}
			}
		} else if (!sWorkStr.Find("HELP")) {
			fpInput.EatWhite();
			if ((ch = (char)fpInput.Get()) == '=') {
				GetAlphaNumFromStream(fpInput, str);
				fpInput.EatWhite();

				SetHelpFilename(str);

				if (fpInput.peek() == ';') {
					fpInput.Get();
				}
			}
			//LOGINFO("  HELP:"); << m_sBackgroundName << endl;
		} else if (!sWorkStr.Find("ENDIF")) {
			if (bElseExprList.IsEmpty()) {
				ParseAlertBox(fpInput, "Error: ENDIF without IF", __FILE__, __LINE__);
			} else {
				bElseExprList.RemoveHead();
			}
			if (pActiveExpr) {
				pActiveExpr = pActiveExpr->GetPrevExpression();
//LOGINFO("ENDIF");
			} else {
				CBofString str2("Unexpected ENDIF:");
				str2 += sWldName;
				ParseAlertBox(fpInput, str2.GetBuffer(), __FILE__, __LINE__);
				//BofMessageBox(str.GetBuffer(), "Continuing");
//LOGINFO(str);
			}
		} else if (!sWorkStr.Find("IF")) {
			// MDM added a bPrevNeg to keep track of nested else-if's
			BOOL bPrevNeg = FALSE;
			if (bElseExprList.GetHead())
				bPrevNeg = bElseExprList.GetHead()->GetNodeItem();

			bElseExprList.AddToHead((BOOL) FALSE);

			// MDM added a bPrevNeg to keep track of nested else-if's
			CBagExpression *pExp = new CBagExpression(pActiveExpr, bPrevNeg);
			// CBagExpression *pExp = new CBagExpression(pActiveExpr);

			Assert(pExp != NULL);
			pExp->SetInfo(fpInput);
			if (!m_pExpressionList)
				m_pExpressionList = new CBofList<CBagExpression * >;
			Assert(m_pExpressionList != NULL);
			m_pExpressionList->AddToTail(pExp);
			pActiveExpr = pExp;
//LOGINFO("IF ()");
		} else if (!sWorkStr.Find("ELSE")) {
			if (bElseExprList.IsEmpty()) {
				//LogError("Er
				ParseAlertBox(fpInput, "Error: ELSE without IF", __FILE__, __LINE__);
			} else {
				bElseExprList.RemoveHead();
				bElseExprList.AddToHead((BOOL) TRUE);
			}
//LOGINFO("ELSE");
		} else if (!sWorkStr.Find("BMP")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewSpriteObject(sWorkStr);
//LOGINFO("  BMP:");
		} else if (!sWorkStr.Find("SPR")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewSpriteObject(sWorkStr);
//LOGINFO("  SPR:");

		} else if (!sWorkStr.Find("LNK")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewLinkObject(sWorkStr);

			// jwl 11.07.96 allow residue printing objects
		} else if (!sWorkStr.Find("RPO")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewRPObject(sWorkStr);

		} else if (!sWorkStr.Find("EDO")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewEDObject(sWorkStr);

		} else if (!sWorkStr.Find("DOS")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewDosObject(sWorkStr);

//LOGINFO("  LNK:");
		} else if (!sWorkStr.Find("SND")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewSoundObject(sWorkStr);
//LOGINFO("  SND:");
		} else if (!sWorkStr.Find("BUT")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewButtonObject(sWorkStr);
//LOGINFO("  BUT:");
		} else if (!sWorkStr.Find("CHR")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewCharacterObject(sWorkStr);
//LOGINFO("  CHR:");
		} else if (!sWorkStr.Find("TNG")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewThingObject(sWorkStr);
//LOGINFO("  THG:");
		} else if (!sWorkStr.Find("ARE")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewAreaObject(sWorkStr);
//LOGINFO("  ARE:");
		} else if (!sWorkStr.Find("VAR")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewVariableObject(sWorkStr);
//LOGINFO("  VAR:");
		} else if (!sWorkStr.Find("TXT")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewTextObject(sWorkStr);
//LOGINFO("  TXT:");
		} else if (!sWorkStr.Find("MOVIE")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewMovieObject(sWorkStr);
//LOGINFO("  MOVIE:");
		} else if (!sWorkStr.Find("COMMAND")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewCommandObject(sWorkStr);
//LOGINFO("  COMMAND:");
		} else if (!sWorkStr.Find("EXPR")) {
			GetStringFromStream(fpInput, sWorkStr, "=", TRUE);
			pObj = OnNewExpressionObject(sWorkStr);
//LOGINFO("  EXPR:");
		} else if (!sWorkStr.Find("REM") || !sWorkStr.Find("//")) {
			char s[256];
			fpInput.Get(s, 256);
//LOGINFO("  //" <<s);
		} else {
			pObj = OnNewUserObject(sWorkStr);
//LOGINFO("  NEW Object " << sWorkStr << ";");
		}

		if (pObj != NULL) {
			Assert(CBofObject::IsValidObject(pObj));

			if (!bElseExprList.IsEmpty() && bElseExprList[0]) {
				pObj->SetNegative();
			} else if (pActiveExpr && pActiveExpr->IsNegative()) {
				// if there is an active expression that is negative
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

//LOGINFO(pObj->GetFileName() << " inserted.");
#if defined(BOF_DEBUG) && defined(RMS_MESSAGES)
			CBofString s = pObj->GetFileName();
			LogInfo(BuildString("  Adding Object %s at %d [ %d / %d ]", s.GetBuffer(), pObj, GetFreePhysMem(), GetFreeMem()));
#endif
			AddObject(pObj);
		}

		fpInput.EatWhite();

	}  // While not eof

//LOGINFO("Closing SDEV:" << sWldName

	if (fpInput.peek() == SDEV_END_DELIM)
		fpInput.Get();

	fpInput.EatWhite();

	if (pActiveExpr) {
		CBofString str2("Mismatch in IF/ENDIF:");
		str2 += sWldName;
		ParseAlertBox(fpInput, str2.GetBuffer(), __FILE__, __LINE__);
		//BofMessageBox(str.GetBuffer(), "Could not attach object");
//LOGINFO("Mismatch in IF/ENDIF of " << sWldName << endl;
		return ERR_UNKNOWN;
	}

	if (bAttach)
		return AttachActiveObjects();

	// Add everything to the window
	return ERR_NONE;
}


INT CBagStorageDev::GetObjectCount() {
	if (m_pObjectList != NULL) {
		return (m_pObjectList->GetCount());

	} else {
		return (0);
	}
}


CBagObject *CBagStorageDev::GetObjectByPos(INT nIndex) {
	Assert(m_pObjectList != NULL);
	Assert((nIndex >= 0) && (nIndex < m_pObjectList->GetCount()));

	return (m_pObjectList->GetNodeItem(nIndex));
}


CBagObject *CBagStorageDev::GetObject(int nRefId, BOOL bActiveOnly) {
	Assert(m_pObjectList != NULL);

	CBagObject *pObj;
	int         nListLen = m_pObjectList->GetCount();

	for (int i = 0; i < nListLen; ++i) {
		pObj =  GetObjectByPos(i);

		if ((pObj->GetRefId() == nRefId) && (!bActiveOnly || (pObj->IsActive() && pObj->IsAttached())))
			return pObj;
	}

	return NULL;
}


CBagObject *CBagStorageDev::GetObject(const CBofString &sName, BOOL bActiveOnly) {
	Assert(m_pObjectList != NULL);

#if 1
	CBagObject *pObj, *pObjFound;
	CBofListNode<CBagObject *> *pNode;

	pObjFound = NULL;
	pNode = m_pObjectList->GetHead();
	while (pNode != NULL) {
		pObj = pNode->GetNodeItem();

		if (pObj->GetRefName().Compare(sName) == 0) {
			pObjFound = pObj;
			break;
		}

		pNode = pNode->m_pNext;
	}
	if (bActiveOnly && (pObjFound != NULL) && !pObjFound->IsActive())
		pObjFound = NULL;

	return (pObjFound);

#else
	CBagObject *pObj;
	int         nListLen = m_pObjectList->GetCount();

	for (int i = 0; i < nListLen; ++i) {
		pObj =  GetObjectByPos(i);
		if ((!bActiveOnly || pObj->IsActive())                     &&
		        (pObj->GetRefName().GetLength() == sName.GetLength())    &&
		        (!pObj->GetRefName().Find(sName)))
			return pObj;
	}
	return NULL;
#endif
}


CBagObject *CBagStorageDev::GetObjectByType(const CBofString &sType, BOOL bActiveOnly) {
	Assert(m_pObjectList != NULL);

	CBagObject *pObj;
	INT         nListLen = m_pObjectList->GetCount();

	for (INT i = 0; i < nListLen; ++i) {
		pObj =  GetObjectByPos(i);

		if (bActiveOnly) {
			if (pObj->IsActive() && !GetStringTypeOfObject(pObj->GetType()).Find(sType))
				return pObj;
		} else {
			if (!GetStringTypeOfObject(pObj->GetType()).Find(sType))
				return pObj;
		}
	}
	return NULL;
}


CBagObject *CBagStorageDev::GetObject(const CBofPoint &xPoint, BOOL bActiveOnly) {
	CBagObject *pObj;
	INT nCount;

	// Resolve in reverse order since the last painted is on top
	if ((nCount = GetObjectCount()) != 0) {
		for (int i = nCount - 1; i >= 0; --i) {
			pObj = GetObjectByPos(i);

			if (pObj->IsInside(xPoint) && (!bActiveOnly || (pObj->IsActive() && pObj->IsAttached())))
				return pObj;
		}
	}
	return NULL;
}


VOID CBagStorageDev::HandleError(ERROR_CODE errCode) {
	//
	// Exit this application on fatal errors
	//
	if (errCode != ERR_NONE) {

#if 0 //RMS_PORT
		// Display Error Message to the user
		BofMessageBox(errList[errCode], "Fatal Error!");

		// Force this application to terminate
		Close();

		// Don't allow a repaint (remove all WM_PAINT messages)
		ValidateRect(NULL);
#endif // RMS_PORT
	}
}



PARSE_CODES CBagStorageDev::SetInfo(bof_ifstream &fpInput) {
	CHAR                szStr[256];
	szStr[0] = 0;
	CBofString          str(szStr, 256);                // jwl 08.28.96 perf improvement
	char ch;

	fpInput.EatWhite();

	if ((ch = (char)fpInput.Get()) == '=') {
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


ERROR_CODE CBagStorageDev::Attach() {
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

	m_bFirstPaint = TRUE;

	if (!m_sBackgroundName.IsEmpty()) {

		CBofBitmap *pBmp;

		if (((pBmp = new CBofBitmap(m_sBackgroundName)) != NULL) && !pBmp->ErrorOccurred()) {

			SetBackground(pBmp);
			errCode = AttachActiveObjects();

		} else {
			errCode = ERR_FOPEN;
		}
	}

	return (errCode);
}


ERROR_CODE CBagStorageDev::Detach() {
	//if (GetBackground())
	//  delete GetBackground();

	// BCW 08/30/96 08:41 pm
	// Must force people to not use a bad App's palette
	//
	CBofApp::GetApp()->SetPalette(NULL);
	SetBackground(NULL);

	// jwl 10.11.96 Notify the main window that we need to redraw the background filter.
	CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());
	if (pMainWin != NULL) {
		((CBagPanWindow *)pMainWin)->SetPreFilterPan(TRUE);
	}

	return DetachActiveObjects();
}


ERROR_CODE CBagStorageDev::Close() {
	return (ERR_NONE);
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
	CHAR szLocalBuff[256];
	CBofString s(szLocalBuff, 256);

	s = str;

	BofMessageBox(s.GetBuffer(), "Could not find object type");

	return NULL;
}

VOID CBagStorageDev::OnSetFilter(BOOL (*filterFunction)(const USHORT nFilterid, CBofBitmap *, CBofRect *)) {
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


ERROR_CODE CBagStorageDev::PreFilter(CBofBitmap *pBmp, CBofRect *pRect, CBofList<CBagObject *> *pList) {
	if (pBmp != NULL) {

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

	// let pda know that we've been prefiltered
	SetPreFiltered(TRUE);

	if (GetDirtyAllObjects()) {
		MakeListDirty(m_pObjectList);
		MakeListDirty(pList);
	} else {
		SetDirtyAllObjects(TRUE);
	}

	return (ERR_NONE);
}


VOID CBagStorageDev::MakeListDirty(CBofList<CBagObject *> *pList) {

	if (pList) {
		INT nCount = pList->GetCount();
		if (nCount != 0) {
			CBagObject *pObj;

			for (INT i = 0; i < nCount; ++i) {
				pObj = pList->GetNodeItem(i);
				pObj->SetDirty(TRUE);
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
	SetOnUpdate(TRUE);
	SetCloseOnOpen(FALSE);
	m_pWorkBmp = NULL;

	// Set a default help file for when there is not one specified
	//
#if BOF_MAC
	m_sHelpFileName = "$SBARDIR:GENERAL:RULES:DEFAULT.TXT";
#else
	m_sHelpFileName = "$SBARDIR\\GENERAL\\RULES\\DEFAULT.TXT";
#endif
	MACROREPLACE(m_sHelpFileName);

	m_xSDevType = SDEV_WND;
}


CBagStorageDevWnd::~CBagStorageDevWnd() {
	Assert(IsValidObject(this));

	KillWorkBmp();

	//ReleaseObjects();                          // delete all master sprite objects
}


ERROR_CODE CBagStorageDevWnd::Attach() {
	CBofPalette *pPalette;
	CHAR szLocalBuff[256];
	CBofString s(szLocalBuff, 256);

	s = GetName();

	if (!GetBackgroundName().IsEmpty()) {

		// This should actually be moved to sbarapp, but the load file will then
		// need to be removed from the constructor.
		//CBofApp::GetApp()->SetMainWindow(this);

		// Associtate this window with callbacks so that any public member function can
		// be accessed by objects inserted into this class.
		SetAssociateWnd(this);

		CBofBitmap *pBmp  = new CBofBitmap(GetBackgroundName());

		if ((pBmp == NULL) || (pBmp->Height() <= 0) || (pBmp->Width() <= 0)) {
			ReportError(ERR_FOPEN, "BarComputer Background Opened Failed");
		} else {

			SetBackground(pBmp);

			// Set the bagel crap
			pPalette = pBmp->GetPalette();
			CBofApp::GetApp()->SetPalette(pPalette);
			// CBagel::GetBagApp()->GetMasterWnd()->SelectPalette(pPalette);
			CBofRect  ScreenRect =  CBofApp::GetApp()->GetMainWindow()->GetRect();
			//ReSize(&ScreenRect);
			CBofSprite::OpenLibrary(pPalette);

			CBofRect r;
			if (pBmp)
				r = pBmp->GetRect();

			if (r.Width() && r.Height()) {
				Create(s.GetBuffer(), &r, CBagel::GetBagApp()->GetMasterWnd());
				//CBofApp::GetApp()->GetMainWindow();
			} else {
				Create(s.GetBuffer(), NULL, CBagel::GetBagApp()->GetMasterWnd());
			}

			// BCW - 12/11/96 09:51 pm - Fix for bug #7638
			Show();

			AttachActiveObjects();
		}

	} else {
		ReportError(ERR_UNKNOWN, "No background for this storage device window");
	}

	SetPreFilterPan(TRUE);
	g_pLastWindow = this;

	CBagStorageDev *pSDev;

	if ((pSDev = SDEVMNGR->GetStorageDevice("EVT_WLD")) != NULL) {

		// Have we allocated one yet ?
		//
		if (m_pEvtSDev == NULL) {
			m_pEvtSDev = (CBagEventSDev *)pSDev;
			m_pEvtSDev->SetAssociateWnd(this);
			if (!m_pEvtSDev->IsAttached())
				m_pEvtSDev->Attach();

#if BOF_MAC
			SetTimer(EVAL_EXPR, 1000);  // null for now, will not play music.
#else
			SetTimer(EVAL_EXPR, 1000);
#endif
			g_bPauseTimer = FALSE;

		} else {
			// We already allocated one
			// we just need to re-associate the parent
			// window and reset the timer
			m_pEvtSDev->SetAssociateWnd(this);

#if BOF_MAC
			SetTimer(EVAL_EXPR, 1000);  // null for now, will not play music.
#else
			SetTimer(EVAL_EXPR, 1000);
#endif
			g_bPauseTimer = FALSE;
		}
	}

	return (m_errCode);
}

VOID CBagStorageDevWnd::OnTimer(UINT nEventID) {
	Assert(IsValidObject(this));
	static BOOL bAlready = FALSE;

	if (!g_bPauseTimer) {

		// Don't allow recursion
		//
		if (!bAlready) {
			bAlready = TRUE;

			// mdm 7/17 - evaluate the event storage device IF MOVIE NOT PLAYING
			if ((CBofApp::GetApp()->GetMainWindow())->IsEnabled() && nEventID == EVAL_EXPR) {

#if 0
				KillTimer(nEventID);

				TimerStart();
				if (m_pEvtSDev != NULL) {
					for (INT i = 0; i < 1000; i++) {
						m_pEvtSDev->EvaluateExpressions();
					}
				}
				BofMessageBox(BuildString("Time: %ld", TimerStop()), "Eval * 1000");
#else
				if (m_pEvtSDev != NULL) {
					m_pEvtSDev->EvaluateExpressions();

					// jwl 12.27.96 If our turncount was updated, then execute the event world
					// for the turncount dependent storage device.
					if (CBagEventSDev::GetEvalTurnEvents() == TRUE) {
						CBagEventSDev::SetEvalTurnEvents(FALSE);
						CBagTurnEventSDev *pSDev = (CBagTurnEventSDev *) SDEVMNGR->GetStorageDevice("TURN_WLD");
						if (pSDev != NULL) {
							// If unable to execute event world, try again next time through.
							if (pSDev->EvaluateExpressions() == ERR_UNKNOWN) {
								CBagEventSDev::SetEvalTurnEvents(TRUE);
							}
						}
					}
				}
#endif
				g_bWaitOK = TRUE;
			}
			bAlready = FALSE;
		}
	}
}


ERROR_CODE CBagStorageDevWnd::Detach() {
	//SetCloseOnOpen(FALSE);
	DetachActiveObjects();

	CBofApp::GetApp()->SetPalette(NULL);

	SetBackground(NULL);
	CBofSprite::CloseLibrary();
	CBagStorageDev::Detach();

	KillTimer(EVAL_EXPR);

	Destroy();

	return (m_errCode);
}

ERROR_CODE CBagStorageDevWnd::Close() {
	CBagel::GetBagApp()->GetMasterWnd()->SetStorageDev(GetPrevSDev(), FALSE);

	return (m_errCode);
}


ERROR_CODE CBagStorageDevWnd::SetBackground(CBofBitmap *pBmp) {
	if (pBmp) {
		SetBackdrop(pBmp);
		SetWorkBmp();
	} else {
		KillBackdrop();
		KillWorkBmp();
	}
	return (m_errCode);
}


ERROR_CODE CBagStorageDevWnd::SetWorkBmp() {
	// delete any previous work area
	KillWorkBmp();

	CBofBitmap *pBmp = GetBackground();
	if (pBmp != NULL) {
		m_pWorkBmp = new CBofBitmap(pBmp->Width(), pBmp->Height(), pBmp->GetPalette());
		pBmp->Paint(m_pWorkBmp);
	}

	return (m_errCode);
}


ERROR_CODE CBagStorageDevWnd::KillWorkBmp() {
	if (m_pWorkBmp != NULL) {
		delete m_pWorkBmp;
		m_pWorkBmp = NULL;
	}

	return (m_errCode);
}


VOID CBagStorageDevWnd::OnPaint(CBofRect *) {
	Assert(IsValidObject(this));

	PaintScreen();
}



VOID CBagStorageDevWnd::OnMainLoop(VOID) {
	Assert(IsValidObject(this));

	PaintScreen();

#if 0
	if (g_bFullTest) {
		DOUBLE fFPS;
		g_bFullTest = FALSE;

		TimerStart();
		for (INT i = 0; i < 1000; i++) {
			PaintScreen();
		}
		fFPS = (DOUBLE)1000000 / TimerStop();
		BofMessageBox(BuildString("PaintScreen: %f FPS", fFPS), "FPS Test");
	}
#endif

	g_pLastWindow = this;
}


ERROR_CODE PaintCursor(CBofBitmap *pBmp) {
	Assert(pBmp != NULL);

	CBagCursor *pCursor;
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

	pCursor = CBagCursor::GetCurrent();

	if (pBmp != NULL && pCursor != NULL) {
		CBofBitmap *pCursorBmp;

		pCursorBmp = pCursor->GetImage();

#if OPTIMIZELOADTIME
		if (pCursorBmp == NULL && pCursor->IsWieldCursor()) {
			pCursor->Load();
			pCursorBmp = pCursor->GetImage();
		}
#endif

		if (pCursorBmp != NULL) {

			INT x, y;

			GetCurrentCursPos(pCursor, &x, &y);

			// jwl 10.11.96 save what was there so we can later paint it back to the screen
			gRepaintRect.SetRect(x, y, x + pCursorBmp->Width() - 1, y + pCursorBmp->Height() - 1);
			CBofRect cDstRect(0, 0, pCursorBmp->Width() - 1, pCursorBmp->Height() - 1);

//			CBagPanWindow *pMainWin = (CBagPanWindow *)(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow());
			pBmp->Paint(CBagStorageDev::m_pUnderCursorBmp, &cDstRect, &gRepaintRect, NOT_TRANSPARENT);

			errCode = pCursorBmp->Paint(pBmp, x, y, NULL, DEFAULT_CHROMA_COLOR);
		}
	}

	return (errCode);
}

ERROR_CODE CBagStorageDevWnd::PaintScreen(CBofRect *pRect, BOOL bPaintCursor) {
	Assert(IsValidObject(this));

	if (m_pBackdrop != NULL) {
		if (m_pUnderCursorBmp == NULL) {
			m_pUnderCursorBmp = new CBofBitmap(kCursWidth, kCursWidth, NULL);
			SetDrawCursorBackdrop(TRUE);
		} else {
			if (DrawCursorBackdrop()) {
				if (!gRepaintRect.IsRectEmpty()) {
					CBofRect cSrcRect(0, 0, gRepaintRect.Width() - 1, gRepaintRect.Height() - 1);
					m_pUnderCursorBmp->Paint(m_pBackdrop, &gRepaintRect, &cSrcRect, NOT_TRANSPARENT);
				}
			} else {
				SetDrawCursorBackdrop(TRUE);
			}
		}

		// Make sure the background is empty
		//m_pBackdrop->FillRect(NULL, COLOR_BLACK)

		OnRender(m_pBackdrop, pRect);

#if FPS_TEST
		if (g_bFPSTest) {
			ULONG lTimerStop;
			if ((lTimerStop = TimerStop()) != 0) {
				g_fFPSTotal += 1000L / lTimerStop;
				g_lFPSCount++;

				DWORD nCurTime = GetTimer();
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

		if (bPaintCursor) {
			// Paint the cursor as a top level object
			//
			PaintCursor(m_pBackdrop);
		}

		if (g_bAllowPaint) {
			m_pBackdrop->Paint(this, pRect, pRect);
		}
	}

	if (m_bFirstPaint) {
		m_bFirstPaint = FALSE;
		AttachActiveObjects();
	}

	return (m_errCode);
}


ERROR_CODE CBagStorageDevWnd::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != NULL);

	if (PreFilterPan()) {
		PreFilter(pBmp, pRect, NULL);
		SetPreFilterPan(FALSE);

		if (m_pWorkBmp != NULL) {
			m_pWorkBmp->Paint(pBmp, pRect, pRect);
		}
	}

	PaintStorageDevice(this, pBmp, pRect);


	if (IsFiltered()) {
		USHORT nFilterId = GetFilterId();
#if BOF_MAC && __POWERPC__
		CallUniversalProc(m_pBitmapFilter,
		                  uppFilterProcInfo,
		                  nFilterId, pBmp, pRect);
#else
		(*m_pBitmapFilter)(nFilterId, pBmp, pRect);
#endif
	}

	return (m_errCode);
}


ERROR_CODE CBagStorageDevWnd::RunModal(CBagObject *pObj) {
	Assert(pObj != NULL);

	if (pObj->IsModal() && pObj->IsActive()) {

		CBofBitmap *pBmp;
		if ((pBmp = GetBackdrop()) != NULL) {
#if BOF_MAC
			// switch to the grafport of the window that we're going
			// to be drawing into.
			STBofPort stSavePort(GetMacWindow());
#endif
			while (!pObj->IsModalDone()) {
				// jwl 11.14.96 make sure we redraw each and every frame!
				SetPreFilterPan(TRUE);
				OnRender(pBmp, NULL);
				if (g_bAllowPaint) {
					pBmp->Paint(this, 0, 0);
				}
			}
		}
	}

	return (m_errCode);
}



ERROR_CODE CBagStorageDevWnd::PaintObjects(CBofList<CBagObject *> * /*list*/, CBofBitmap * /*pBmp*/, CBofRect & /*viewRect*/, CBofList<CBofRect> * /*pUpdateArea*/, BOOL /*tempVar*/) {
	return (m_errCode);
}

ERROR_CODE CBagStorageDevWnd::LoadFile(const CBofString &sFile) {
	CHAR        szWldFile[256];
	szWldFile[0] = 0;
	CBofString sWldFile(szWldFile, 256);        // jwl 08.28.96 performance improvement

	if (sFile.IsEmpty())
		sWldFile = "StoreDev.Wld";
	else
		sWldFile = sFile;

	MACROREPLACE(sWldFile);

#if BOF_MAC
	// ios::nocreate does not appear to be defined in the mac
	// cw8 iostreams environment, so make sure that the file exists before
	// opening it, otherwise it will create the file.

	if (FileExists(sWldFile) == FALSE) {
		ReportError(ERR_FFIND, "Could not find file %s", sWldFile.GetBuffer());
	}

#endif
	CHAR *pBuf;
	INT nLength;

	// Force buffer to be big enough so that the entire script
	// is pre-loaded
	//
	nLength = FileLength(sWldFile);
	if ((pBuf = (CHAR *)BofAlloc(nLength)) != NULL) {
		bof_ifstream fpInput(pBuf, nLength);

		CBofFile cFile;
		cFile.Open(sWldFile);
		cFile.Read(pBuf, nLength);
		cFile.Close();

		CBagStorageDev::LoadFileFromStream(fpInput, sWldFile);

		// If the window.IsCreated()
		//
		if (IsCreated())
			InvalidateRect(NULL);

		BofFree(pBuf);

	} else {
		ReportError(ERR_MEMORY);
	}

	// Add everything to the window

	return (m_errCode);
}


VOID CBagStorageDevWnd::OnClose() {
	CBofWindow::OnClose();
	DestroyWindow();                            // destruct the main window
}

void CBagStorageDevWnd::OnMouseMove(UINT n, CBofPoint *pPoint, void *) {
	CBagStorageDev::OnMouseMove(n, pPoint, GetAssociateWnd());

	CBagMasterWin::SetActiveCursor(0);

	// if a zelda movie is playing then just give 'em the wait cursor
	// as we're not gonna allow them to do squat anyway.
	//
	// Brian, note I did not do "== TRUE", you should be very proud of me.
	if (CBagPDA::IsMoviePlaying()) {
		CBagMasterWin::SetActiveCursor(6);
		return;
	}

	// This should be on update cursor virtual func
	if (GetExitOnEdge() && (pPoint->x < GetExitOnEdge()) && (pPoint->y < 360 + 10) && !(GetPrevSDev().IsEmpty())) {
		CBagMasterWin::SetActiveCursor(10);

	} else {

		// BCW 08/19/96 01:04 pm Added wield cursors
		BOOL bWield;

		bWield = FALSE;
		if (CBagWield::GetWieldCursor() >= 0) {
			CBagMasterWin::SetActiveCursor(CBagWield::GetWieldCursor());
			bWield = TRUE;
		}

		CBofList<CBagObject *> *pList;

		// Run thru background object list and find if the cursor is over an object
		//
		if ((pList = GetObjectList()) != NULL) {

			CBofListNode<CBagObject *> *pNode;
			CBagObject *pObj;
			CBofPoint cCursorLocation;
			INT nCursor;

			cCursorLocation = DevPtToViewPort(m_xCursorLocation);

			// Go thru list backwards to find the 1st top-most object
			//
			pNode = pList->GetTail();
			while (pNode != NULL) {
				pObj = pNode->GetNodeItem();

				// Switch to that cursor if it's a LINK or CHAR
				//
#if 1
				// Change cursor as long as it's not a link to a closeup, or
				// link to another Pan, or a text menu, or button.
				//
				if (pObj->IsAttached() && pObj->IsInside(cCursorLocation)) {
					nCursor = pObj->GetOverCursor();
					if (!bWield || (nCursor == 2 || nCursor == 5 || nCursor == 55 || pObj->GetType() == TEXTOBJ || pObj->GetType() == BUTTONOBJ)) {
						CBagMasterWin::SetActiveCursor(nCursor);
					}
					break;
				}
#else
				if (pObj->IsAttached() && pObj->IsInside(cCursorLocation)) {
					if (!bWield || (pObj->GetType() == LINKOBJ || pObj->GetType() == CHAROBJ || pObj->GetType() == TEXTOBJ || pObj->GetType() == BUTTONOBJ)) {
						CBagMasterWin::SetActiveCursor(pObj->GetOverCursor());
					}
					break;
				}
#endif
				pNode = pNode->m_pPrev;
			}
		}
	}
}


void CBagStorageDevWnd::OnLButtonDown(UINT nFlags, CBofPoint *xPoint, void *) {
	// if asynch movie playing in PDA don't react to mouse down
	// (8033) if it's not a wait cursor, then allow the user to access
	// that hotspot.
	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::GetActiveCursor() == 6) {
		return;
	}

	CBagStorageDev::OnLButtonDown(nFlags, xPoint, GetAssociateWnd());
	CBofWindow::OnLButtonDown(nFlags, xPoint);
}


VOID CBagStorageDevWnd::OnLButtonUp(UINT nFlags, CBofPoint *xPoint, void *) {
	// if asynch movie playing in PDA don't react to mouse down
	// (8033) if it's not a wait cursor, then allow the user to access
	// that hotspot.
	if (CBagPDA::IsMoviePlaying() && CBagMasterWin::GetActiveCursor() == 6) {
		return;
	}

	// react to a mouse up, it will probably involve drawing a new
	// window...
	SetPreFilterPan(TRUE);
	SetDrawCursorBackdrop(FALSE);  // will get crap left on the screen during chats if removed

	if (GetExitOnEdge() && xPoint->x < GetExitOnEdge() && !(GetPrevSDev().IsEmpty())) {
		// Set the initial location as the last full panoramas position
		//m_xInitLoc = m_xLastLeavingLoc;
		Close();

	} else {

		CBagStorageDev::OnLButtonUp(nFlags, xPoint, GetAssociateWnd());
		CBofWindow::OnLButtonUp(nFlags, xPoint);
	}
}


VOID CBagStorageDevWnd::OnKeyHit(ULONG lKey, ULONG nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {

	// Gamma Correction
	//
	case BKEY_F11: {

#if BOF_WINDOWS && 0
		static DOUBLE g_fGammaPow = 1.2;
		PALETTEENTRY stEntry;
		HPALETTE hPal;
		INT i;

		g_fGammaPow *= 0.9;
		if (g_fGammaPow < 0.5) {
			g_fGammaPow = 2.0;
		}

		hPal = NULL;
		if (m_pBackdrop != NULL && m_pBackdrop->GetPalette() != NULL) {
			if ((hPal = m_pBackdrop->GetPalette()->GetPalette()) != NULL) {

				for (i = 0; i < 265; i++) {
					::GetPaletteEntries(hPal, i, 1, &stEntry);

					stEntry.peRed = (BYTE)(powl(((DOUBLE)stEntry.peRed / 256.0), g_fGammaPow) * 256);
					stEntry.peGreen = (BYTE)(powl(((DOUBLE)stEntry.peGreen / 256.0), g_fGammaPow) * 256);
					stEntry.peBlue = (BYTE)(powl(((DOUBLE)stEntry.peBlue / 256.0), g_fGammaPow) * 256);
					::SetPaletteEntries(hPal, i, 1, &stEntry);
				}
			}
		}

#endif
	}
	break;

#if FPS_TEST

#if BOF_MAC
	case 'f':
	case 'F':
#else
	case BKEY_F8:
#endif
		SetPreFilterPan(TRUE);
		g_bFPSTest = !g_bFPSTest;

		// Reset FPS info
		g_fFPSTotal = 0.0;
		g_lFPSCount = 0;
		break;

#if 0
	case 'g':
	case 'G':
	case BKEY_F9:
		g_bFullTest = TRUE;
		g_bFPSTest = FALSE;
		break;
#endif

#endif

	default:
		CBofWindow::OnKeyHit(lKey, nRepCount);
		break;
	}
}



//
// CBagStorageDevDlg -
//  CBagStorageDevDlg is a window that contains a slide bitmap object.  It has specialize
//  functions for handling slide bitmaps and slide objects.
//



CBagStorageDevDlg::CBagStorageDevDlg() : CBofDialog() {
	m_xSDevType = SDEV_DLG;

	// Set a default help file for when there is not one specified
	//
#if BOF_MAC
	m_sHelpFileName = "$SBARDIR:GENERAL:RULES:DEFAULT.TXT";
#else
	m_sHelpFileName = "$SBARDIR\\GENERAL\\RULES\\DEFAULT.TXT";
#endif
	MACROREPLACE(m_sHelpFileName);
}


ERROR_CODE CBagStorageDevDlg::Attach() {
	Assert(IsValidObject(this));

	CBagStorageDev::Attach();

	CHAR szLocalBuff[256];
	CBofString s(szLocalBuff, 256);
	s = GetName();

	CBofBitmap *pBmp = GetBackground();
	CBofRect r;
	if (pBmp)
		r = pBmp->GetRect();

	if (r.Width() && r.Height()) {
		Create(s.GetBuffer(), &r, CBagel::GetBagApp()->GetMasterWnd());
		//CBofApp::GetApp()->GetMainWindow();
	} else {
		Create(s.GetBuffer(), NULL, CBagel::GetBagApp()->GetMasterWnd());
	}

	SetPreFilterPan(TRUE);

	CBofDialog::DoModal();

	Destroy();

	return (m_errCode);
}


ERROR_CODE CBagStorageDevDlg::Close() {
	ReleaseCapture();

	CBofDialog::Close();

	return (m_errCode);
}


ERROR_CODE CBagStorageDevDlg::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != NULL);

	PaintStorageDevice(this, pBmp, pRect);

	if (IsFiltered()) {
		USHORT nFilterId = GetFilterId();
#if BOF_MAC && __POWERPC__
		CallUniversalProc(m_pBitmapFilter,
		                  uppFilterProcInfo,
		                  nFilterId, pBmp, pRect);
#else
		(*m_pBitmapFilter)(nFilterId, pBmp, pRect);
#endif
	}

	return (m_errCode);
}


VOID CBagStorageDevDlg::OnMainLoop() {
	Assert(IsValidObject(this));

	// the background of a storage device might be in motion, i.e. it
	// might need updates, but since it is a background screen, it probably isn't that
	// important, so we'll update it 4 times / second.

	INT nCurTime = GetTimer();
	if (nCurTime > (gLastBackgroundUpdate + 250)) {
		gLastBackgroundUpdate = nCurTime;
		if (g_pLastWindow) {
			g_pLastWindow->SetPreFilterPan(TRUE);
		}
	}

	PaintScreen();

#if 0 //FPS_TEST
	if (g_bFPSTest) {
		ULONG lTimerStop;
		if ((lTimerStop = TimerStop()) != 0) {
			g_fFPSTotal += 1000L / lTimerStop;
			g_lFPSCount++;

			DWORD nCurTime = GetTimer();
			if (nCurTime > gLastFPSUpdate + 1000) {
				gLastFPSUpdate = nCurTime;
				CBofRect cRect(540, 2, 639, 22);
				m_pBackdrop->FillRect(&cRect, COLOR_BLACK);         // dirty object system causes overwriting of text
				PaintText(m_pBackdrop, &cRect, BuildString("%2.2f fps", g_fFPSTotal / g_lFPSCount), FONT_14POINT, TEXT_BOLD, RGB(255, 255, 255), JUSTIFY_RIGHT, FORMAT_TOP_RIGHT);
				cRect.SetRect(540, 23, 639, 43);
				PaintText(m_pBackdrop, &cRect, BuildString("Correction: %d", CBagPanWindow::GetRealCorrection()), FONT_14POINT, TEXT_BOLD, RGB(255, 255, 255), JUSTIFY_RIGHT, FORMAT_TOP_RIGHT);
			}
		}

		TimerStart();
	}
#endif
}


VOID CBagStorageDevDlg::OnPaint(CBofRect *) {
	Assert(IsValidObject(this));

	PaintScreen();

	ValidateAnscestors();

	CBagPanWindow::FlushInputEvents();
}


ERROR_CODE CBagStorageDevDlg::PaintScreen(CBofRect *pRect, BOOL bPaintCursor) {
	Assert(IsValidObject(this));

	if (m_pBackdrop != NULL) {

		CBofBitmap *pBmp, *pWorkBmp;
		CBagStorageDevWnd *pWin;

		if ((pWin = g_pLastWindow) != NULL) {

			if ((pBmp = pWin->GetBackdrop()) != NULL) {
				//pBmp->FillRect(NULL, COLOR_BLACK);

				// Paint what was under the cursor last time through
				// this loop.
				if (m_pUnderCursorBmp == NULL) {
					m_pUnderCursorBmp = new CBofBitmap(kCursWidth, kCursWidth, NULL);
					SetDrawCursorBackdrop(TRUE);
				} else {
					if (DrawCursorBackdrop()) {
						CBofRect cSrcRect(0, 0, gRepaintRect.Width() - 1, gRepaintRect.Height() - 1);
						m_pUnderCursorBmp->Paint(pBmp, &gRepaintRect, &cSrcRect, NOT_TRANSPARENT);
					} else {
						SetDrawCursorBackdrop(TRUE);
					}
				}

				// don't redraw the background window unless we have to.
				if (pWin->PreFilterPan()) {
					if ((pWorkBmp = pWin->GetWorkBmp()) != NULL) {
						pWorkBmp->Paint(pBmp, pRect, pRect);
					}

					pWin->OnRender(pBmp, pRect);
				}

				OnRender(m_pBackdrop, pRect);
				CBofRect wrect(GetWindowRect());
				m_pBackdrop->Paint(pBmp, &wrect, NULL);

				if (bPaintCursor) {
					PaintCursor(pBmp);
				}

				if (g_bAllowPaint) {
					pBmp->Paint(pWin, pRect, pRect);
				}
			}
		}
	}

	// Set the firstpaint flag and attach objects
	// to allow for immediate run objects to run
	// mdm 7.30.96
	if (m_bFirstPaint) {
		m_bFirstPaint = FALSE;
		AttachActiveObjects();
	}

	return (m_errCode);
}


ERROR_CODE CBagStorageDevDlg::PaintObjects(CBofList<CBagObject *> * /*list*/, CBofBitmap * /*pBmp*/, CBofRect & /*viewRect*/, CBofList<CBofRect> * /*pUpdateArea*/, BOOL /*tempVar*/) {
	return (m_errCode);
}


ERROR_CODE CBagStorageDevDlg::LoadFile(const CBofString &sFile) {

	CHAR        szWldFile[256];
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

	if (FileExists(sWldFile) == FALSE) {
		return ERR_FOPEN;
	}
#endif

	CHAR *pBuf;
	INT nLength;

	// Force buffer to be big enough so that the entire script
	// is pre-loaded
	//
	nLength = FileLength(sWldFile);
	if ((pBuf = (CHAR *)BofAlloc(nLength)) != NULL) {
		bof_ifstream fpInput(pBuf, nLength);

		CBofFile cFile;
		cFile.Open(sWldFile);
		cFile.Read(pBuf, nLength);
		cFile.Close();

		CBagStorageDev::LoadFileFromStream(fpInput, sWldFile);

		BofFree(pBuf);

		// If the window.IsCreated()
		//
		if (IsCreated())
			InvalidateRect(NULL);
	}

	// Add everything to the window
	return (m_errCode);
}



ERROR_CODE
CBagStorageDevDlg::Create(const CHAR *pszName, INT x, INT y, INT nWidth, INT nHeight, CBofWindow *pParent, UINT nControlID) {
	ERROR_CODE rc = CBofDialog::Create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
	SetCapture();
	return rc;
}

ERROR_CODE CBagStorageDevDlg::Create(const CHAR *pszName, CBofRect *pRect, CBofWindow *pParent, UINT nControlID) {
	ERROR_CODE rc = CBofDialog::Create(pszName, pRect, pParent, nControlID);
	SetCapture();
	return rc;
}


VOID CBagStorageDevDlg::OnClose(VOID) {
	// MOVED TO CLOSE
	//ReleaseCapture();

	// KillBackground();
	//
	if (m_pDlgBackground != NULL) {
		delete m_pDlgBackground;
		m_pDlgBackground = NULL;
	}

	// since our sdevdlg doesn't have a detach active objects
	// method, we will have to manually take all the existing objects and
	// tell them to redraw themselves should they ever be called on in life
	// to do as such.
	INT nCount = GetObjectCount();
	CBagObject *pObj;
	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			if ((pObj = GetObjectByPos(i)) != NULL) {
				pObj->SetDirty(TRUE);
			}
		}
	}

	CBofDialog::OnClose();

	Destroy();                            // destruct the main window

	// our dlog may have dirtied our backdrop, make sure it is
	// redrawn.
	if (g_pLastWindow != NULL) {
		g_pLastWindow->SetPreFilterPan(TRUE);
		g_pLastWindow->PaintScreen(NULL);

		// this is fairly shameful, but for some reason, some
		// updates don't work in the above paintscreen and must be updated the
		// next time through.  Don't know why, would love to find out, but
		// running out of time.
		g_pLastWindow->SetPreFilterPan(TRUE);
	}
}


void CBagStorageDevDlg::OnMouseMove(UINT n, CBofPoint *xPoint, void *) {
	CBagStorageDev::OnMouseMove(n, xPoint, GetAssociateWnd());
}



void CBagStorageDevDlg::OnLButtonDown(UINT nFlags, CBofPoint *xPoint, void *) {
	CBagStorageDev::OnLButtonDown(nFlags, xPoint, GetAssociateWnd());
	CBofDialog::OnLButtonDown(nFlags, xPoint);
}


void CBagStorageDevDlg::OnLButtonUp(UINT nFlags, CBofPoint *xPoint, void *) {
	if (CBofDialog::GetRect().PtInRect(*xPoint)) {
		CBagStorageDev::OnLButtonUp(nFlags, xPoint, GetAssociateWnd());
		CBofDialog::OnLButtonUp(nFlags, xPoint);
	} else  {
		Close();
	}
}


int CBagStorageDevManager::nSDevMngrs = 0;

CBagStorageDevManager::CBagStorageDevManager() {
	Assert(++nSDevMngrs < 2);
}


CBagStorageDevManager::~CBagStorageDevManager() {
	Assert(IsValidObject(this));

	Assert(--nSDevMngrs >= 0);
	ReleaseStorageDevices();
	m_xStorageDeviceList.RemoveAll();
}

ERROR_CODE CBagStorageDevManager::RegisterStorageDev(CBagStorageDev *pSDev) {
	Assert(IsValidObject(this));

	m_xStorageDeviceList.AddToTail(pSDev);

	return (ERR_NONE);
}


ERROR_CODE CBagStorageDevManager::UnRegisterStorageDev(CBagStorageDev *pSDev) {
	Assert(IsValidObject(this));

#if 1
	CBofListNode<CBagStorageDev *> *pList;

	pList = m_xStorageDeviceList.GetHead();
	while (pList != NULL) {
		if (pSDev == pList->GetNodeItem()) {
			m_xStorageDeviceList.Remove(pList);
			break;
		}

		pList = pList->m_pNext;
	}
	return (ERR_NONE);

#else
	int i, n;

	//Assert(nSDevMngrs==1);

	for (i = 0, n = -1; i < m_xStorageDeviceList.GetCount(); ++i) {
		if (pSDev == m_xStorageDeviceList[i])
			n = i;
	}
	if (n != -1) {
		m_xStorageDeviceList.Remove(n);
		return ERR_NONE;
	}
	return ERR_UNKNOWN;
#endif
}


ERROR_CODE CBagStorageDevManager::ReleaseStorageDevices() {
	Assert(IsValidObject(this));

	CBagStorageDev *pSDev;

	while (m_xStorageDeviceList.GetCount()) {
		pSDev = m_xStorageDeviceList[0];

		if (pSDev != NULL) {

			// The CBagStorageDev destructor will remove it from the list
			delete pSDev;
		}
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
	return NULL;
}


CBagStorageDev *CBagStorageDevManager::GetStorageDeviceContaining(const CBofString &sName) {
	Assert(IsValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.GetCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];
		if (pSDev && pSDev->GetObject(sName))
			return m_xStorageDeviceList[i];
	}
	return NULL;
}


CBagStorageDev *CBagStorageDevManager::GetStorageDevice(const CBofString &sName) {
	Assert(IsValidObject(this));

	for (int i = 0; i < m_xStorageDeviceList.GetCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];
		if (pSDev && (pSDev->GetName().GetLength() == sName.GetLength()) &&
		        !pSDev->GetName().Find(sName))
			return m_xStorageDeviceList[i];
	}
	return NULL;
}


BOOL CBagStorageDevManager::MoveObject(const CBofString &sDstName, const CBofString &sSrcName, const CBofString &sObjName) {
	Assert(IsValidObject(this));

	CBagStorageDev *pSrcSDev;
	CBagStorageDev *pDstSDev;

	// Find the storage device
	if ((pDstSDev = SDEVMNGR->GetStorageDevice(sDstName)) == NULL)
		return FALSE;
	if ((pSrcSDev = SDEVMNGR->GetStorageDevice(sSrcName)) == NULL)
		return FALSE;

	// Find the storage device
	if (pDstSDev->ActivateLocalObject(sObjName) != ERR_NONE)
		return FALSE;
	if (pSrcSDev->DeactivateLocalObject(sObjName) != ERR_NONE) {
		pDstSDev->DeactivateLocalObject(sObjName);
		return FALSE;
	}

	return TRUE;
}


BOOL CBagStorageDevManager::AddObject(const CBofString &sDstName, const CBofString &sObjName) {
	Assert(IsValidObject(this));

	CBagStorageDev *pDstSDev;

	// Find the storage device
	if ((pDstSDev = SDEVMNGR->GetStorageDevice(sDstName)) == NULL)
		return FALSE;

	// Find the storage device
	if (pDstSDev->ActivateLocalObject(sObjName) != ERR_NONE)
		return FALSE;

	return TRUE;
}


BOOL CBagStorageDevManager::RemoveObject(const CBofString &sSrcName, const CBofString &sObjName) {
	Assert(IsValidObject(this));

	CBagStorageDev *pSrcSDev;

	// Find the storage device
	if ((pSrcSDev = SDEVMNGR->GetStorageDevice(sSrcName)) == NULL)
		return FALSE;

	// Find the storage device
	if (pSrcSDev->DeactivateLocalObject(sObjName) != ERR_NONE)
		return FALSE;

	return TRUE;
}


INT CBagStorageDevManager::GetObjectValue(const CBofString &sObject, const CBofString &sProperty) {
	Assert(IsValidObject(this));

	CBagObject *pObj;
	for (int i = 0; i < m_xStorageDeviceList.GetCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];

		if (pSDev && ((pObj = pSDev->GetObject(sObject)) != NULL)) {
			return pObj->GetProperty(sProperty);
		}
	}
	return 0;
}


//#define DO_NOT_USE_THIS_VALUE -8675309
// Set object will set a property to a numberic value or set the object to a string value - I am too
// lazy to write two funtions
VOID CBagStorageDevManager::SetObjectValue(const CBofString &sObject, const CBofString &sProperty, INT nValue/*=DO_NOT_USE_THIS_VALUE*/) {
	Assert(IsValidObject(this));

	// Make sure that all objects are set and not just one?
	// Make sure that the first object is changed?
	CBagObject *pObj;
	for (int i = 0; i < m_xStorageDeviceList.GetCount(); ++i) {
		CBagStorageDev *pSDev = m_xStorageDeviceList[i];

		if (pSDev && ((pObj = pSDev->GetObject(sObject)) != NULL)) {
			//if (nValue == DO_NOT_USE_THIS_VALUE)
			pObj->SetProperty(sProperty, nValue);
		}
	}
}


VOID CBagStorageDevManager::SaveObjList(ST_OBJ *pObjList, INT nNumEntries) {
	Assert(IsValidObject(this));
	Assert(pObjList != NULL);

	CBagStorageDev *pSDev;
	CBagObject *pObj;
	INT i, j, k, m, n;

	k = 0;
	n = GetNumStorageDevices();
	for (i = 0; i < n; i++) {

		if ((pSDev = GetStorageDevice(i)) != NULL) {

			m = pSDev->GetObjectCount();
			for (j = 0; j < m; j++) {
				pObj = pSDev->GetObjectByPos(j);

				if (!pObj->GetRefName().IsEmpty()) {
					Assert(strlen(pObj->GetRefName()) < MAX_OBJ_NAME);
					strncpy((pObjList + k)->m_szName, pObj->GetRefName(), MAX_OBJ_NAME);

					// We MUST have put something in here
					Assert((pObjList + k)->m_szName[0] != '\0');

					Assert(strlen(pSDev->GetName()) < MAX_SDEV_NAME);
					strncpy((pObjList + k)->m_szSDev, pSDev->GetName(), MAX_SDEV_NAME);

					// jwl 11.01.96 save if this guy is waiting to play
					(pObjList + k)->m_nFlags = (USHORT)(pObj->IsMsgWaiting() ? mIsMsgWaiting : 0);
					(pObjList + k)->m_lState = pObj->GetState();
					(pObjList + k)->m_lProperties = pObj->GetProperties();
					(pObjList + k)->m_lType = pObj->GetType();
					(pObjList + k)->m_bUsed = 1;

					k++;
					Assert(k < nNumEntries);

					// jwl 11.01.96 This is pretty dangerous, put up an error
					if (k >= nNumEntries) {
						BofMessageBox("SaveObjList encountered too many objects", "Internal Error");
						break;
					}
				}
			}
		}
	}
}


VOID CBagStorageDevManager::RestoreObjList(ST_OBJ *pObjList, INT nNumEntries) {
	Assert(IsValidObject(this));
	Assert(pObjList != NULL);
	CBagStorageDev *pSDev;
	CBagObject *pObj;
	INT i;

	// Restore the state of all objects
	//
	for (i = 0; i < nNumEntries; i++) {

		if ((pObjList + i)->m_bUsed) {

			if ((pSDev = GetStorageDevice((pObjList + i)->m_szSDev)) != NULL) {

				if ((pObj = pSDev->GetObject((pObjList + i)->m_szName)) != NULL) {
					pObj->SetProperties((pObjList + i)->m_lProperties);
					pObj->SetState((pObjList + i)->m_lState);
					pObj->SetType((BAG_OBJECT_TYPE)(pObjList + i)->m_lType);

					pObj->SetMsgWaiting(((pObjList + i)->m_nFlags & mIsMsgWaiting) == mIsMsgWaiting);

					// Moved to BAGMAST.CPP so that no sprites are loaded
					// until after a CBofSprite::OpenLibrary()
					//
					/*if (pObj->IsMsgWaiting ()) {
					    pSDev->ActivateLocalObject (pObj);
					}*/
				}
			}
		}
	}
}

VOID GetCurrentCursPos(CBagCursor *pCursor, INT *px, INT *py) {

	warning("STUB: GetCurrentCursPos()");

#if 0
#if BOF_MAC
	Point stPoint;
	::GetMouse(&stPoint);
	::LocalToGlobal(&stPoint);

	*px = stPoint.h - pCursor->GetX();
	*py = stPoint.v - pCursor->GetY();
#else
	POINT stPoint;
	GetCursorPos(&stPoint);

	*px = stPoint.x - pCursor->GetX();
	*py = stPoint.y - pCursor->GetY();
#endif

#endif

	if (*px < 0)
		*px = 0;

	if (*py < 0)
		*py = 0;
}

// slightly more complicated then before... if we're in a
// CIC, then return true, however, if we're in the zoompda, then return
// the previous SDEV's cic value.
BOOL CBagStorageDev::IsCIC(VOID) {
	if (m_bCIC != FALSE) {
		return TRUE;
	}

	CHAR szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	sStr = "BPDAZ_WLD";
	if (m_sName == sStr) {
		return GetCICStatus();
	}

	return FALSE;
}

} // namespace Bagel
