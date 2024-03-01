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

#ifndef BAGEL_BOFLIB_GFX_DISPLAY_WINDOW_H
#define BAGEL_BOFLIB_GFX_DISPLAY_WINDOW_H

#include "bagel/boflib/gui/bof_display_object.h"
#include "bagel/boflib/bof_list.h"
#include "bagel/boflib/queue.h"
#include "bagel/boflib/stack.h"

namespace Bagel {

#define DISPWIN_SIZE_DEF 100

#define BWM_CLOSE 1
#define BWM_USER  2

class CBofDisplayWindow: public CBofDisplayObject {

public:
	CBofDisplayWindow();
	CBofDisplayWindow(const CHAR *pszName, INT x = 0, INT y = 0, INT nWidth = DISPWIN_SIZE_DEF, INT nHeight = DISPWIN_SIZE_DEF, CBofDisplayWindow *pParent = NULL, INT nID = 0);
	virtual ~CBofDisplayWindow();

	ERROR_CODE  Create(const CHAR *pszName, INT x = 0, INT y = 0, INT nWidth = DISPWIN_SIZE_DEF, INT nHeight = DISPWIN_SIZE_DEF, CBofDisplayWindow *pParent = NULL, UINT nControlID = 0);
	ERROR_CODE  Create(const CHAR *pszName, CBofRect *pRect = NULL, CBofDisplayWindow *pParent = NULL, UINT nControlID = 0);

	virtual VOID Destroy(VOID)      {
		Erase();
	}
	ERROR_CODE  Close(VOID)         {
		return (PostMessage(BWM_CLOSE, 0, 0));
	}

	ERROR_CODE  Show(VOID)          {
		return (Paint(m_cPosition));
	}
	ERROR_CODE  Hide(VOID)          {
		return (Erase());
	}

	ERROR_CODE  Center(BOOL bRepaint);
	ERROR_CODE  Move(const INT x, const INT y, BOOL bRepaint = FALSE);
	ERROR_CODE  ReSize(CBofSize *pSize, BOOL bRepaint = FALSE);

	ERROR_CODE  PostMessage(ULONG lMessage, ULONG lParam1, ULONG lParam2);
	ERROR_CODE  PostUserMessage(ULONG lMessage, ULONG lExtraInfo);
	static VOID HandleMessages(VOID);
	ERROR_CODE  FlushMessages(VOID);

	ERROR_CODE  SetTimer(UINT nID, UINT nInterval, BOFCALLBACK pCallBack = NULL);
	ERROR_CODE  KillTimer(UINT nTimerID);
	ERROR_CODE  KillMyTimers(VOID);

	CBofDisplayObject *GetParent(VOID)      {
		return (m_pParent);
	}

	static CBofDisplayWindow *GetActiveWindow(VOID) {
		return (m_pActiveWindow);
	}

	VOID        SetActive(VOID)             {
		m_pActiveWindow = this;
		OnActivate();
	}
	BOOL        IsActiveWindow(VOID)        {
		return (m_pActiveWindow == this);
	}

	virtual VOID Enable(VOID)               {
		m_bDisabled = FALSE;
	}
	virtual VOID Disable(VOID)              {
		m_bDisabled = TRUE;
	}

	ERROR_CODE  SetBackdrop(CBofBitmap *pBitmap, BOOL bRefresh = FALSE);
	ERROR_CODE  SetBackdrop(const CHAR *pszBmpFile, BOOL bRefresh = FALSE);

	CBofBitmap *GetBackdrop(VOID)           {
		return (m_pImage);
	}

	BOOL        HasBackdrop(VOID)           {
		return (m_pImage != NULL);
	}

	ERROR_CODE  KillBackdrop(VOID)          {
		return (ReleaseImage());
	}

	VOID        SetControlID(UINT nID)      {
		m_nID = nID;
	}
	UINT        GetControlID(VOID)          {
		return (m_nID);
	}

	ERROR_CODE  SetCapture(VOID);
	ERROR_CODE  ReleaseCapture(VOID);
	BOOL        HasCapture(VOID)            {
		return (m_pCaptureWindow == this);
	}

	static ERROR_CODE InvalidateRect(CBofRect *pRect) {
		return (AddToDirtyRect(pRect));
	}
	ERROR_CODE  UpdateWindow(VOID)          {
		return (UpdateDirtyRect());
	}

	static VOID HandleMouseMove(UINT nFlags, const CBofPoint &cPoint);
	static VOID HandleLButtonDown(UINT nFlags, const CBofPoint &cPoint);
	static VOID HandleLButtonUp(UINT nFlags, const CBofPoint &cPoint);
	static VOID HandleLButtonDblClk(UINT nFlags, const CBofPoint &cPoint);
	static VOID HandleRButtonDown(UINT nFlags, const CBofPoint &cPoint);
	static VOID HandleRButtonUp(UINT nFlags, const CBofPoint &cPoint);
	static VOID HandleRButtonDblClk(UINT nFlags, const CBofPoint &cPoint);

	static VOID HandleKeyHit(ULONG lKeyCode, ULONG lRepCount);
	static CBofDisplayWindow *FindWindowFromPoint(const CBofPoint &cPoint);

	// Overide-able member functions
	//
	virtual VOID OnBofButton(CBofObject *pButton, INT nExtraInfo);
	virtual VOID OnBofScrollBar(CBofObject *pButton, INT nNewPos);
	virtual VOID OnBofListBox(CBofObject *pListBox, INT nItemIndex);
	virtual VOID OnMainLoop(VOID);

	virtual VOID OnSoundNotify(ULONG lParam1, ULONG lParam2);
	virtual VOID OnMovieNotify(ULONG lParam1, ULONG lParam2);

	virtual VOID OnMCINotify(ULONG lParam1, ULONG lParam2);

	virtual VOID OnTimer(UINT nTimerId);

	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);

	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnLButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonDblClk(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnRButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnRButtonDblClk(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnReSize(CBofSize *pSize);
	virtual VOID OnClose(VOID);

	virtual VOID OnUserMessage(ULONG nMessage, ULONG lParam);

	virtual VOID OnActivate(VOID);

protected:

	static CBofList<CBofDisplayWindow *> m_cWindowList;
	static CBofDisplayWindow *m_pActiveWindow;
	static CBofDisplayWindow *m_pCaptureWindow;
	static CQueue m_cMessageQueue;
	static CStack m_cCaptureStack;

	CHAR        m_szTitle[MAX_TITLE];       // Title of window
	INT         m_nID;
	BOOL        m_bHasModal;
	BOOL        m_bDisabled;
};

} // namespace Bagel

#endif
