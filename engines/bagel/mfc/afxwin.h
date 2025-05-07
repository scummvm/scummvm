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

namespace Bagel {
namespace MFC {

/*============================================================================*/
// Window message map handling

class CCmdTarget;
typedef void (CCmdTarget::*AFX_PMSG)();

struct AFX_MSGMAP_ENTRY {
	AFX_PMSG pfn;    // routine to call (or special value)
	UINT nMessage;   // windows message
	UINT nCode;      // control code or WM_NOTIFY code
	UINT nID;        // control ID (or 0 for windows messages)
	UINT nLastID;    // used for entries specifying a range of control id's
	AFX_PMSG nSig;   // signature type (action) or pointer to message #
};

struct AFX_MSGMAP {
	const AFX_MSGMAP *(*pfnGetBaseMap)();
	const AFX_MSGMAP_ENTRY *lpEntries;
};

#define afx_msg

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

enum AfxSig {
	AfxSig_end = 0		// [marks end of message map]
};

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
};

class CBrush : public CGdiObject {
public:
	~CBrush() override {
	}
};

class CFont : public CGdiObject {
public:
	~CFont() override {
	}
};

class CBitmap : public CGdiObject {
public:
	~CBitmap() override {
	}

	BOOL Attach(HGDIOBJ hObject);
	HGDIOBJ Detach();
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

class CWnd : public CCmdTarget {
	DECLARE_DYNCREATE(CWnd)
protected:
	static const MSG *GetCurrentMessage();

public:
	HWND m_hWnd;

public:
	~CWnd() override {
	}

	void ShowWindow(int nCmdShow);
	void UpdateWindow();
	void SetActiveWindow();
};

class CFrameWnd : public CWnd {
	DECLARE_DYNCREATE(CFrameWnd)
public:
	~CFrameWnd() override {
	}
};

class CDialog : public CWnd {
public:
	~CDialog() override {
	}
};

class CStatic : public CWnd {
public:
	~CStatic() override {
	}
};

class CButton : public CWnd {
public:
	~CButton() override {
	}
};

class CListBox : public CWnd {
public:
	~CListBox() override {
	}
};

class CCheckListBox : public CListBox {
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
public:
	~CScrollBar() override { }
};

class CView : public CWnd {
public:
	~CView() override {
	}
};

class CScrollView : public CView {
public:
	~CScrollView() override {
	}
};

class CWinThread : public CCmdTarget {
public:
	~CWinThread() override {
	}
};

class CWinApp : public CWinThread {
public:
	CWinApp(const char *appName);
	~CWinApp() override {
	}

	void SetDialogBkColor();
};

} // namespace MFC
} // namespace Bagel

#endif
