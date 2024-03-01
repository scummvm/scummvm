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

#include "bagel/boflib/gui/display_window.h"
#include "bagel/boflib/bof_timer.h"
#include "bagel/boflib/bof_app.h"

namespace Bagel {

class CBofMessageEx: public CBofObject {

public:
	CBofDisplayWindow *m_pWindow;   // destination window for message
	ULONG       m_lMessage;         // message to send (usually WM_USER)
	ULONG       m_lParam1;          // user defined info
	ULONG       m_lParam2;          // more user defined info
};

CBofList<CBofDisplayWindow *> CBofDisplayWindow::m_cWindowList;
CBofDisplayWindow *CBofDisplayWindow::m_pActiveWindow;

CBofDisplayWindow *CBofDisplayWindow::m_pCaptureWindow = NULL;
CQueue CBofDisplayWindow::m_cMessageQueue;
CStack CBofDisplayWindow::m_cCaptureStack;

#if BOF_MAC
VOID *BofDisplayWinTimerCallBack(INT nID, VOID *pParam2);
#endif

CBofDisplayWindow::CBofDisplayWindow() {
	// Add this new window to the list of all windows
	//
	m_cWindowList.AddToTail(this);

	// Inits
	m_nID = 0;
	m_lType = DISPTYPE_WINDOW;
	m_bHasModal = FALSE;
	m_bDisabled = FALSE;
}


CBofDisplayWindow::CBofDisplayWindow(const CHAR *pszName, INT x, INT y, INT nWidth, INT nHeight, CBofDisplayWindow *pParent, INT nID) {
	Assert(pszName != NULL);
	Assert(strlen(pszName) < MAX_TITLE);

	// Inits
	m_nID = 0;
	m_lType = DISPTYPE_WINDOW;
	m_bHasModal = FALSE;
	m_bDisabled = FALSE;

	// Add this new window to the list of all windows
	//
	m_cWindowList.AddToTail(this);

	Create(pszName, x, y, nWidth, nHeight, pParent, nID);
}


CBofDisplayWindow::~CBofDisplayWindow() {
	Assert(IsValidObject(this));

	FlushMessages();

	INT i;

	// Remove this window from the window list
	//
	for (i = 0; i < m_cWindowList.GetCount(); i++) {

		if (m_cWindowList[i] == this) {
			m_cWindowList.Remove(i);
			break;
		}
	}
}


ERROR_CODE CBofDisplayWindow::Create(const CHAR *pszName, INT x, INT y, INT nWidth, INT nHeight, CBofDisplayWindow *pParent, UINT nID) {
	Assert(IsValidObject(this));
	Assert(pszName != NULL);
	Assert(strlen(pszName) < MAX_TITLE);

	// Remember my ID
	m_nID = nID;

	// Determine parent/child relationship
	//
	if (pParent != NULL) {
		pParent->LinkChild(this);
		m_pParent = pParent;
	} else {

		Assert(m_pMainDisplay != NULL);

		m_pMainDisplay->LinkChild(this);
		m_pParent = m_pMainDisplay;
	}

	// Set position and size of this window
	//
	m_cSize.cx = nWidth;
	m_cSize.cy = nHeight;
	SetPosition(x, y);

	// keep track of the name of this window
	Common::strlcpy(m_szTitle, pszName, MAX_TITLE);

	// Window is not yet legally positioned
	m_bPositioned = FALSE;

	// this is now the active window with focus
	SetActive();

	return (m_errCode);
}


ERROR_CODE CBofDisplayWindow::Create(const CHAR *pszName, CBofRect *pRect, CBofDisplayWindow *pParent, UINT nID) {
	Assert(IsValidObject(this));
	INT x, y, dx, dy;

	x = y = 0;
	dx = dy = DISPWIN_SIZE_DEF;
	if (pRect != NULL) {
		x = pRect->left;
		y = pRect->top;
		dx = pRect->Width();
		dy = pRect->Height();
	}
	return (Create(pszName, x, y, dx, dy, pParent, nID));
}


ERROR_CODE CBofDisplayWindow::Center(BOOL bRepaint) {
	Assert(IsValidObject(this));

	CBofDisplayObject *pParent;
	INT x, y;

	if ((pParent = m_pParent) != NULL) {

		x = (pParent->Width() - Width()) / 2;
		y = (pParent->Height() - Height()) / 2;

	} else {
		x = (CBofApp::GetApp()->ScreenWidth() - Width()) / 2;
		y = (CBofApp::GetApp()->ScreenHeight() - Height()) / 2;
	}

	Move(x, y, bRepaint);

	return (m_errCode);
}


ERROR_CODE CBofDisplayWindow::Move(const INT x, const INT y, BOOL bRepaint) {
	Assert(IsValidObject(this));

	BatchPaint(x, y);

	if (bRepaint) {
		UpdateDirtyRect();
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayWindow::ReSize(CBofSize *pNewSize, BOOL bRepaint) {
	Assert(IsValidObject(this));
	Assert(pNewSize != NULL);

	OnReSize(pNewSize);

	if (pNewSize->cx != m_cSize.cx || pNewSize->cy != m_cSize.cy) {

		BatchErase();

		SetSize(*pNewSize);
		SetPosition(m_cPosition);

		BatchPaint(m_cPosition);

		if (bRepaint) {
			UpdateDirtyRect();
		}
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayWindow::PostMessage(ULONG lMessage, ULONG lParam1, ULONG lParam2) {
	Assert(IsValidObject(this));

	CBofMessageEx *pMessage;

	// Create a user defined message.
	// NOTE: This message will be deleted by()
	//
	if ((pMessage = new CBofMessageEx) != NULL) {

		pMessage->m_pWindow = this;
		pMessage->m_lMessage = lMessage;
		pMessage->m_lParam1 = lParam1;
		pMessage->m_lParam2 = lParam2;

		// Insert this message into the boffo message queue
		m_cMessageQueue.AddItem(pMessage);
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayWindow::PostUserMessage(ULONG nMessage, ULONG lExtraInfo) {
	Assert(IsValidObject(this));

	return (PostMessage(BWM_USER, nMessage, lExtraInfo));
}


VOID CBofDisplayWindow::HandleMessages(VOID) {
	CBofMessageEx *pMessage;

	if ((pMessage = (CBofMessageEx *)m_cMessageQueue.RemoveItem()) != NULL) {

		switch (pMessage->m_lMessage) {

		case BWM_CLOSE:

			// If this fails, then the message arrived after the
			// window was destructed.
			Assert(CBofObject::IsValidObject(pMessage->m_pWindow));

			pMessage->m_pWindow->OnClose();
			break;

		case BWM_USER:

			// If this fails, then the message arrived after the
			// window was destructed.
			Assert(CBofObject::IsValidObject(pMessage->m_pWindow));

			pMessage->m_pWindow->OnUserMessage(pMessage->m_lParam1, pMessage->m_lParam2);

		default:
			LogWarning(BuildString("Unknown message: %ld", pMessage->m_lMessage));
			break;
		}

		delete pMessage;
	}
}


ERROR_CODE CBofDisplayWindow::FlushMessages(VOID) {
	Assert(IsValidObject(this));

	CQueue cTempQ;
	CBofMessageEx *pMessage;

	// Remove all of my messages from the message queue
	//
	while ((pMessage = (CBofMessageEx *)m_cMessageQueue.RemoveItem()) != NULL) {

		// If this is one of my messages, then remove it
		//
		if (pMessage->m_pWindow == this) {
			delete pMessage;

			// Otherwise, add it to a temporary Queue, so we can restore the
			// original message queue.
			//
		} else {
			cTempQ.AddItem(pMessage);
		}
	}

	// Restore the original message queue (Minus the messages we removed)
	//
	while ((pMessage = (CBofMessageEx *)cTempQ.RemoveItem()) != NULL) {

		m_cMessageQueue.AddItem(pMessage);
	}

	return (m_errCode);
}


VOID *BofDisplayWinTimerCallBack(INT nID, VOID *pParam2) {
	CBofDisplayWindow *pWindow;

	pWindow = (CBofDisplayWindow *)pParam2;
	Assert(CBofObject::IsValidObject(pWindow));

	pWindow->OnTimer(nID);

	return (NULL);
}


ERROR_CODE CBofDisplayWindow::SetTimer(UINT nID, UINT nInterval, BOFCALLBACK pCallBack) {
	Assert(IsValidObject(this));

	CBofTimer *pTimer;

	if (pCallBack == NULL) {
		pCallBack = BofDisplayWinTimerCallBack;
	}

	if ((pTimer = new CBofTimer(nID, nInterval, this, pCallBack)) != NULL) {

		pTimer->Start();

	} else {
		ReportError(ERR_MEMORY);
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayWindow::KillTimer(UINT nTimerID) {
	Assert(IsValidObject(this));
	INT i;

	CBofTimer *pTimer;

	// Find and remove specified timer from the list of timers
	//
	//pTimer = CBofTimer::m_pTimerList;
	/*for (i = 0; i < CBofTimer::m_cTimerList.GetCount(); i++) {
	    pTimer = CBofTimer::m_cTimerList[i];

	    if (pTimer->m_nID == nTimerID && (CBofDisplayWindow *)pTimer->m_lUserInfo == this) {
	        delete pTimer;
	        break;
	    }
	}*/

	return (m_errCode);
}


ERROR_CODE CBofDisplayWindow::KillMyTimers(VOID) {
	Assert(IsValidObject(this));

	CBofTimer *pTimer;
	INT i;

	i = 0;
	/*while (i < CBofTimer::m_cTimerList.GetCount()) {

	    pTimer = CBofTimer::m_cTimerList[i];

	    if ((CBofDisplayWindow *)pTimer->m_lUserInfo == this) {
	        delete pTimer;
	        i = 0;
	        continue;
	    }

	    // next timer
	    i++;
	}*/

	return (m_errCode);
}

ERROR_CODE CBofDisplayWindow::SetBackdrop(CBofBitmap *pBitmap, BOOL bRefresh) {
	Assert(IsValidObject(this));

	Assert(pBitmap != NULL);

	ReleaseImage();

	m_pImage = pBitmap;

	if (bRefresh) {
		BatchErase();
		BatchPaint(m_cPosition);
		UpdateDirtyRect();
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayWindow::SetBackdrop(const CHAR *pszBmpFile, BOOL bRefresh) {
	Assert(IsValidObject(this));

	CBofBitmap *pBitmap;
	pBitmap = LoadBitmap(pszBmpFile);

	return (SetBackdrop(pBitmap, bRefresh));
}


ERROR_CODE CBofDisplayWindow::SetCapture(VOID) {
	Assert(IsValidObject(this));

	if (m_pCaptureWindow != this) {

		m_cCaptureStack.Push(m_pCaptureWindow);

		m_pCaptureWindow = this;
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayWindow::ReleaseCapture(VOID) {
	Assert(IsValidObject(this));

	if (m_pCaptureWindow == this) {
		m_pCaptureWindow = (CBofDisplayWindow *)m_cCaptureStack.Pop();
	}

	return (m_errCode);
}


VOID CBofDisplayWindow::OnBofButton(CBofObject *, INT)     {}
VOID CBofDisplayWindow::OnBofScrollBar(CBofObject *, INT)  {}
VOID CBofDisplayWindow::OnBofListBox(CBofObject *, INT)    {}
VOID CBofDisplayWindow::OnMainLoop(VOID)                    {}
VOID CBofDisplayWindow::OnSoundNotify(ULONG, ULONG)       {}
VOID CBofDisplayWindow::OnMovieNotify(ULONG, ULONG)       {}
VOID CBofDisplayWindow::OnMCINotify(ULONG, ULONG)         {}
VOID CBofDisplayWindow::OnTimer(UINT)                      {}

VOID CBofDisplayWindow::OnMouseMove(UINT, CBofPoint *)     {}
VOID CBofDisplayWindow::OnLButtonDown(UINT, CBofPoint *)   {}
VOID CBofDisplayWindow::OnLButtonUp(UINT, CBofPoint *)     {}
VOID CBofDisplayWindow::OnLButtonDblClk(UINT, CBofPoint *) {}

VOID CBofDisplayWindow::OnRButtonDown(UINT, CBofPoint *)   {}
VOID CBofDisplayWindow::OnRButtonUp(UINT, CBofPoint *)     {}
VOID CBofDisplayWindow::OnRButtonDblClk(UINT, CBofPoint *) {}

VOID CBofDisplayWindow::OnKeyHit(ULONG, ULONG)            {}

VOID CBofDisplayWindow::OnReSize(CBofSize *)                {}
VOID CBofDisplayWindow::OnClose(VOID)                       {}

VOID CBofDisplayWindow::OnUserMessage(ULONG, ULONG)       {}

VOID CBofDisplayWindow::OnActivate(VOID)                    {}


VOID CBofDisplayWindow::HandleKeyHit(ULONG lKeyCode, ULONG lRepCount) {
	if (m_pActiveWindow != NULL) {
		m_pActiveWindow->OnKeyHit(lKeyCode, lRepCount);
	}
}


VOID CBofDisplayWindow::HandleMouseMove(UINT nFlags, const CBofPoint &cPoint) {
	CBofDisplayWindow *pWindow;

	if ((pWindow = FindWindowFromPoint(cPoint)) != NULL) {
		CBofPoint cLocalPoint;
		cLocalPoint = pWindow->GlobalToLocal(cPoint);

		pWindow->OnMouseMove(nFlags, &cLocalPoint);
	}
}


VOID CBofDisplayWindow::HandleLButtonDown(UINT nFlags, const CBofPoint &cPoint) {
	CBofDisplayWindow *pWindow;

	if ((pWindow = FindWindowFromPoint(cPoint)) != NULL) {
		CBofPoint cLocalPoint;
		cLocalPoint = pWindow->GlobalToLocal(cPoint);

		pWindow->OnLButtonDown(nFlags, &cLocalPoint);
	}
}

VOID CBofDisplayWindow::HandleLButtonUp(UINT nFlags, const CBofPoint &cPoint) {
	CBofDisplayWindow *pWindow;

	if ((pWindow = FindWindowFromPoint(cPoint)) != NULL) {
		CBofPoint cLocalPoint;
		cLocalPoint = pWindow->GlobalToLocal(cPoint);

		pWindow->OnLButtonUp(nFlags, &cLocalPoint);
	}
}
VOID CBofDisplayWindow::HandleLButtonDblClk(UINT nFlags, const CBofPoint &cPoint) {
	CBofDisplayWindow *pWindow;

	if ((pWindow = FindWindowFromPoint(cPoint)) != NULL) {
		CBofPoint cLocalPoint;
		cLocalPoint = pWindow->GlobalToLocal(cPoint);

		pWindow->OnLButtonDblClk(nFlags, &cLocalPoint);
	}
}
VOID CBofDisplayWindow::HandleRButtonDown(UINT nFlags, const CBofPoint &cPoint) {
	CBofDisplayWindow *pWindow;

	if ((pWindow = FindWindowFromPoint(cPoint)) != NULL) {
		CBofPoint cLocalPoint;
		cLocalPoint = pWindow->GlobalToLocal(cPoint);

		pWindow->OnRButtonDown(nFlags, &cLocalPoint);
	}
}
VOID CBofDisplayWindow::HandleRButtonUp(UINT nFlags, const CBofPoint &cPoint) {
	CBofDisplayWindow *pWindow;

	if ((pWindow = FindWindowFromPoint(cPoint)) != NULL) {
		CBofPoint cLocalPoint;
		cLocalPoint = pWindow->GlobalToLocal(cPoint);

		pWindow->OnRButtonUp(nFlags, &cLocalPoint);
	}
}
VOID CBofDisplayWindow::HandleRButtonDblClk(UINT nFlags, const CBofPoint &cPoint) {
	CBofDisplayWindow *pWindow;

	if ((pWindow = FindWindowFromPoint(cPoint)) != NULL) {
		CBofPoint cLocalPoint;
		cLocalPoint = pWindow->GlobalToLocal(cPoint);

		pWindow->OnRButtonDblClk(nFlags, &cLocalPoint);
	}
}


CBofDisplayWindow *CBofDisplayWindow::FindWindowFromPoint(const CBofPoint &cPoint) {
	CBofDisplayWindow *pWindow;

	// Find the top-most window that is under the specified point
	//
	pWindow = m_pCaptureWindow;
	if (m_pCaptureWindow == NULL) {

		pWindow = (CBofDisplayWindow *)m_pMainDisplay->GetChildFromPoint(cPoint);
	}

	return (pWindow);
}

} // namespace Bagel
