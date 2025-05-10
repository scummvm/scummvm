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

#ifndef BAGEL_MFC_AFXWIN_H
#define BAGEL_MFC_AFXWIN_H

#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/wingdi.h"
#include "bagel/mfc/afx.h"
#include "bagel/mfc/afxstr.h"
#include "bagel/mfc/atltypes.h"

namespace Bagel {
namespace MFC {

class CCmdTarget;
class CWnd;
class CDocument;
class CDocTemplate;
class CView;
class CFrameWnd;
class CDC;
class CScrollBar;
class CListBox;

/*============================================================================*/
// Window message map handling

typedef void (CCmdTarget::*AFX_PMSG)();
typedef void (CWnd::*AFX_PMSGW)(void);

#pragma warning(disable: 4121)
struct AFX_MSGMAP_ENTRY {
	UINT nMessage;   // windows message
	UINT nCode;      // control code or WM_NOTIFY code
	UINT nID;        // control ID (or 0 for windows messages)
	UINT nLastID;    // used for entries specifying a range of control id's
	int nSig;   // signature type (action) or pointer to message #
	AFX_PMSG pfn;    // routine to call (or special value)
};
#pragma warning(default: 4121)

struct AFX_MSGMAP {
	const AFX_MSGMAP *(*pfnGetBaseMap)();
	const AFX_MSGMAP_ENTRY *lpEntries;
};

#define afx_msg

#ifndef AFX_MSG_CALL
#define AFX_MSG_CALL
#endif
typedef void (AFX_MSG_CALL CCmdTarget:: *AFX_PMSG)(void);

#define DECLARE_MESSAGE_MAP() \
protected: \
	static const AFX_MSGMAP *GetThisMessageMap(); \
	const AFX_MSGMAP *GetMessageMap() const override; \

#define BEGIN_TEMPLATE_MESSAGE_MAP(theClass, type_name, baseClass)			\
	template < typename type_name >											\
	const AFX_MSGMAP *theClass< type_name >::GetMessageMap() const			\
		{ return GetThisMessageMap(); }										\
	template < typename type_name >											\
	const AFX_MSGMAP *theClass< type_name >::GetThisMessageMap() {		\
		typedef theClass< type_name > ThisClass;							\
		typedef baseClass TheBaseClass;										\
		static const AFX_MSGMAP_ENTRY _messageEntries[] =					\
		{

#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	const AFX_MSGMAP *theClass::GetMessageMap() const \
		{ return GetThisMessageMap(); } \
	const AFX_MSGMAP *theClass::GetThisMessageMap() { \
		typedef theClass ThisClass;						   \
		typedef baseClass TheBaseClass;					   \
		static const AFX_MSGMAP_ENTRY _messageEntries[] =  \
		{

#define END_MESSAGE_MAP() \
		{ 0, 0, 0, 0, AfxSig_end, (AFX_PMSG)nullptr } \
	}; \
		static const AFX_MSGMAP messageMap = \
		{ &TheBaseClass::GetThisMessageMap, &_messageEntries[0] }; \
		return &messageMap; \
	}								  \

#define DECLARE_DYNCREATE(class_name) \
	DECLARE_DYNAMIC(class_name) \
	static CObject *CreateObject();

/*
 * Message structure
 */
typedef struct tagMSG {
	HWND        hwnd;
	UINT        message;
	WPARAM      wParam;
	LPARAM      lParam;
	DWORD       time;
	POINT       pt;
} MSG, *PMSG, NEAR *NPMSG, FAR *LPMSG;

class CDataExchange {
};

/*============================================================================*/

struct CCreateContext   // Creation information structure
	// All fields are optional and may be NULL
{
	// for creating new views
	CRuntimeClass *m_pNewViewClass; // runtime class of view to create or NULL
	CDocument *m_pCurrentDoc;

	// for creating MDI children (CMDIChildWnd::LoadFrame)
	CDocTemplate *m_pNewDocTemplate;

	// for sharing view/frame state from the original view/frame
	CView *m_pLastView;
	CFrameWnd *m_pCurrentFrame;

	// Implementation
	CCreateContext();
};

/*============================================================================*/

class CGdiObject : public CObject {
public:
	~CGdiObject() override {
	}

	BOOL DeleteObject();
};

class CPen : public CGdiObject {
public:
	~CPen() override {
	}

	BOOL CreatePen(int nPenStyle, int nWidth, COLORREF crColor);
};

class CBrush : public CGdiObject {
public:
	~CBrush() override {
	}

	BOOL CreateSolidBrush(COLORREF crColor);
	BOOL CreateStockObject(int nIndex);
};

class CFont : public CGdiObject {
public:
	~CFont() override {
	}

	BOOL CreateFont(int nHeight, int nWidth, int nEscapement,
		int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline,
		BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision,
		BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily,
		LPCSTR lpszFacename);
};

class CBitmap : public CGdiObject {
public:
	~CBitmap() override {
	}

	BOOL Attach(HGDIOBJ hObject);
	HGDIOBJ Detach();
	BOOL CreateCompatibleBitmap(CDC *pDC, int nWidth, int nHeight);
	BOOL CreateBitmap(int nWidth, int nHeight, UINT nPlanes,
		UINT nBitcount, const void *lpBits);
};

class CPalette : public CGdiObject {
public:
	HGDIOBJ m_hObject = nullptr;

public:
	~CPalette() override {
	}

	BOOL CreatePalette(LPLOGPALETTE lpLogPalette);
	int GetObject(int nCount, LPVOID lpObject) const;
	UINT GetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
		LPPALETTEENTRY lpPaletteColors) const;
	UINT SetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
		LPPALETTEENTRY lpPaletteColors);
};

class CRgn : public CGdiObject {
public:
	~CRgn() override {
	}
};

class CDC : public CObject {
public:
	HDC m_hDC = nullptr;

public:
	~CDC() override {
	}

	BOOL CreateDC(LPCSTR lpszDriverName, LPCSTR lpszDeviceName,
		LPCSTR lpszOutput, const void *lpInitData);
	BOOL CreateCompatibleDC(CDC *pDC);
	BOOL DeleteDC();
	void Attach(HDC hDC);
	void Detach();

	BOOL BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, DWORD dwRop);
	BOOL StretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
		int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop);
	void Ellipse(LPCRECT lpRect);
	void Ellipse(int x1, int y1, int x2, int y2);
	void FrameRect(LPCRECT lpRect, CBrush *pBrush);
	void Rectangle(LPCRECT lpRect);
	void Rectangle(int x1, int y1, int x2, int y2);
	void MoveTo(int x, int y);
	void LineTo(int x, int y);

	virtual CGdiObject *SelectStockObject(int nIndex);
	CPen *SelectObject(CPen *pPen);
	CBrush *SelectObject(CBrush *pBrush);
	virtual CFont *SelectObject(CFont *pFont);
	CBitmap *SelectObject(CBitmap *pBitmap);
	int SelectObject(CRgn *pRgn);       // special return for regions
	CGdiObject *SelectObject(CGdiObject *pObject);

	COLORREF GetNearestColor(COLORREF crColor) const;
	CPalette *SelectPalette(CPalette *pPalette, BOOL bForceBackground);
	UINT RealizePalette();
	void UpdateColors();
	virtual COLORREF SetBkColor(COLORREF crColor);
	int SetBkMode(int nBkMode);

	// Text Functions
	virtual COLORREF SetTextColor(COLORREF crColor);
	virtual BOOL TextOut(int x, int y, LPCSTR lpszString, int nCount);
	BOOL TextOut(int x, int y, const CString &str);
	virtual BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
		LPCSTR lpszString, UINT nCount, LPINT lpDxWidths);
	BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
		const CString &str, LPINT lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);
	CSize TabbedTextOut(int x, int y, const CString &str,
		int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin);

	CSize GetTextExtent(LPCSTR lpszString, int nCount) const;
	CSize GetTextExtent(const CString &str) const;
	CSize GetOutputTextExtent(LPCSTR lpszString, int nCount) const;
	CSize GetOutputTextExtent(const CString &str) const;
	CSize GetTabbedTextExtent(LPCSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	CSize GetTabbedTextExtent(const CString &str,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	CSize GetOutputTabbedTextExtent(LPCSTR lpszString, int nCount,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	CSize GetOutputTabbedTextExtent(const CString &str,
		int nTabPositions, LPINT lpnTabStopPositions) const;
	virtual BOOL GrayString(CBrush *pBrush,
		BOOL(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
		int nCount, int x, int y, int nWidth, int nHeight);
	UINT GetTextAlign() const;
	UINT SetTextAlign(UINT nFlags);
	BOOL GetTextMetrics(LPTEXTMETRIC lpMetrics) const;
};

/*============================================================================*/
// CDC Helpers

class CPaintDC : public CDC {
	DECLARE_DYNAMIC(CPaintDC)

protected:
	HWND m_hWnd = nullptr;

public:
	explicit CPaintDC(CWnd *pWnd);
};

class CCmdTarget : public CObject {
	DECLARE_DYNAMIC(CCmdTarget)
public:
	~CCmdTarget() override {
	}

	virtual const AFX_MSGMAP *GetMessageMap() const;
};

class CDocument : public CCmdTarget {
	DECLARE_DYNAMIC(CDocument)
private:
	CString _title;
	bool _isModified = false;
	CString _unusedPathName;

public:
	~CDocument() override {
	}

	const CString &GetTitle() const;
	virtual void SetTitle(LPCSTR lpszTitle);
	const CString &GetPathName() const;
	virtual void SetPathName(LPCSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual void ClearPathName();

	virtual BOOL IsModified();
	virtual void SetModifiedFlag(BOOL bModified = TRUE);
	virtual void ReportSaveLoadException(LPCSTR lpszPathName,
		CException *e, BOOL bSaving, UINT nIDPDefault);

	// delete doc items etc
	virtual void DeleteContents();

	DECLARE_MESSAGE_MAP()
};


typedef struct tagCREATESTRUCTA {
	LPVOID      lpCreateParams;
	HINSTANCE   hInstance;
	HMENU       hMenu;
	HWND        hwndParent;
	int         cy;
	int         cx;
	int         y;
	int         x;
	LONG        style;
	LPCSTR      lpszName;
	LPCSTR      lpszClass;
	DWORD       dwExStyle;
} CREATESTRUCT, *LPCREATESTRUCT;

class CWnd : public CCmdTarget {
	DECLARE_DYNCREATE(CWnd)
protected:
	static const MSG *GetCurrentMessage();
	virtual void DoDataExchange(CDataExchange *) {}

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg LRESULT OnActivateTopLevel(WPARAM, LPARAM);
	afx_msg void OnCancelMode();
	afx_msg void OnChildActivate();
	afx_msg void OnClose();
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

	afx_msg void OnDestroy();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd *pWho);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnIconEraseBkgnd(CDC *pDC);
	afx_msg void OnKillFocus(CWnd *pNewWnd);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnPaint();
	afx_msg void OnSyncPaint();
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg UINT OnNotifyFormat(CWnd *pWnd, UINT nCommand);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnQueryEndSession();
	afx_msg BOOL OnQueryNewPalette();
	afx_msg BOOL OnQueryOpen();
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTCard(UINT idAction, DWORD dwActionData);
	afx_msg void OnSessionChange(UINT nSessionState, UINT nId);

	afx_msg void OnChangeUIState(UINT nAction, UINT nUIElement);
	afx_msg void OnUpdateUIState(UINT nAction, UINT nUIElement);
	afx_msg UINT OnQueryUIState();

	// Nonclient-Area message handler member functions
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcDestroy();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseHover(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseLeave();
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnNcRButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcXButtonDown(short zHitTest, UINT nButton, CPoint point);
	afx_msg void OnNcXButtonUp(short zHitTest, UINT nButton, CPoint point);
	afx_msg void OnNcXButtonDblClk(short zHitTest, UINT nButton, CPoint point);

	// System message handler member functions
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnPaletteIsChanging(CWnd *pRealizeWnd);
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSysDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnAppCommand(CWnd *pWnd, UINT nCmd, UINT nDevice, UINT nKey);
	afx_msg void OnRawInput(UINT nInputCode, HRAWINPUT hRawInput);
	afx_msg void OnCompacting(UINT nCpuTime);
	afx_msg void OnDevModeChange(_In_z_ LPTSTR lpDeviceName);
	afx_msg void OnFontChange();
	afx_msg void OnPaletteChanged(CWnd *pFocusWnd);
	afx_msg void OnSpoolerStatus(UINT nStatus, UINT nJobs);
	afx_msg void OnSysColorChange();
	afx_msg void OnTimeChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnWinIniChange(LPCTSTR lpszSection);
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM lEventData);
	afx_msg void OnUserChanged();
	afx_msg void OnInputLangChange(UINT nCharSet, UINT nLocaleId);
	afx_msg void OnInputLangChangeRequest(UINT nFlags, UINT nLocaleId);
	afx_msg void OnInputDeviceChange(unsigned short nFlags, HANDLE hDevice);

	// Input message handler member functions
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUniChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point);
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);
	afx_msg void OnXButtonUp(UINT nFlags, UINT nButton, CPoint point);
	afx_msg int OnMouseActivate(CWnd *pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	// Control message handler member functions
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg int OnCharToItem(UINT nChar, CListBox *pListBox, UINT nIndex);
	afx_msg int OnVKeyToItem(UINT nKey, CListBox *pListBox, UINT nIndex);

public:
	HWND m_hWnd = (HWND)0;
	CWnd *m_pParentWnd = nullptr;

	static CWnd *FromHandlePermanent(HWND hWnd);

public:
	~CWnd() override {
	}

	CWnd *GetParent() const;
	void ShowWindow(int nCmdShow);
	BOOL EnableWindow(BOOL bEnable = TRUE);
	void UpdateWindow();
	void SetActiveWindow();
	BOOL IsWindowVisible() const;
	int GetWindowText(CString &rString) const;
	int GetWindowText(LPSTR lpszStringBuf, int nMaxCount) const;

	CDC *GetDC();
	int ReleaseDC(CDC *pDC);

	BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

	BOOL ValidateRect(LPCRECT lpRect = NULL);
	BOOL InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE);
	void GetWindowRect(LPRECT lpRect) const;
	BOOL GetUpdateRect(LPRECT lpRect, BOOL bErase = FALSE);
	void MoveWindow(LPCRECT lpRect, BOOL bRepaint = true);
	void MoveWindow(int x, int y, int nWidth, int nHeight,
		BOOL bRepaint = TRUE);

	virtual BOOL Create(LPCSTR lpszClassName, LPCSTR lpszWindowName,
		DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID,
		CCreateContext *pContext = nullptr);
	CWnd *GetDlgItem(int nID) const;
	CWnd *GetNextDlgGroupItem(CWnd *pWndCtl, BOOL bPrevious = FALSE) const;
	BOOL GotoDlgCtrl(CWnd *pWndCtrl);
	BOOL SubclassDlgItem(UINT nID, CWnd *pParent);
};

class CFrameWnd : public CWnd {
	DECLARE_DYNCREATE(CFrameWnd)
public:
	~CFrameWnd() override {
	}
};

class CDialog : public CWnd {
	DECLARE_DYNAMIC(CDialog)
protected:
	DECLARE_MESSAGE_MAP()

	virtual void OnOK();
	virtual void OnCancel();

public:
	CDialog();
	explicit CDialog(LPCSTR lpszTemplateName,
		CWnd *pParentWnd = NULL);
	explicit CDialog(UINT nIDTemplate,
		CWnd *pParentWnd = NULL);

	~CDialog() override {
	}

	void DoModal();
	virtual BOOL OnInitDialog();

	DWORD GetDefID();
	void SetDefID(UINT nID);

	// termination
	void EndDialog(int nResult);
};

class CStatic : public CWnd {
	DECLARE_DYNAMIC(CStatic)
public:
	~CStatic() override {
	}
};

class CButton : public CWnd {
	DECLARE_DYNAMIC(CButton)

protected:
	DECLARE_MESSAGE_MAP()

public:
	~CButton() override {
	}

	int GetCheck() const;
	void SetCheck(int nCheck);
	void SetButtonStyle(UINT nStyle, BOOL bRedraw = TRUE);
};

class CListBox : public CWnd {
	DECLARE_DYNAMIC(CListBox)
public:
	~CListBox() override {
	}
};

class CCheckListBox : public CListBox {
	DECLARE_DYNAMIC(CCheckListBox)
public:
	~CCheckListBox() override {
	}
};

class CEdit : public CWnd {
	DECLARE_DYNAMIC(CEdit)
public:
	~CEdit() override {
	}
};

class CScrollBar : public CWnd {
	DECLARE_DYNAMIC(CScrollBar)
public:
	~CScrollBar() override { }
	virtual BOOL Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID);

	int GetScrollPos() const;
	int SetScrollPos(int nPos, BOOL bRedraw = true);
	void GetScrollRange(LPINT lpMinPos, LPINT lpMaxPos) const;
	void SetScrollRange(int nMinPos, int nMaxPos, BOOL bRedraw);
	void ShowScrollBar(BOOL bShow);
};

class CView : public CWnd {
	DECLARE_DYNAMIC(CView)
public:
	~CView() override {
	}
};

class CScrollView : public CView {
	DECLARE_DYNAMIC(CScrollView)
public:
	~CScrollView() override {
	}
};

class CWinThread : public CCmdTarget {
	DECLARE_DYNAMIC(CWinThread)
public:
	~CWinThread() override {
	}
};

class CWinApp : public CWinThread {
	DECLARE_DYNAMIC(CWinApp)
public:
	CWinApp(const char *appName);
	~CWinApp() override {
	}

	void SetDialogBkColor();
	HCURSOR LoadStandardCursor(LPCSTR lpszCursorName) const;
	HCURSOR LoadCursor(LPCSTR lpszResourceName) const;
	HCURSOR LoadCursor(UINT nIDResource) const;
	void BeginWaitCursor();
	void EndWaitCursor();
};

extern CWinApp *AfxGetApp();
HINSTANCE AfxGetInstanceHandle();

} // namespace MFC
} // namespace Bagel

#endif
