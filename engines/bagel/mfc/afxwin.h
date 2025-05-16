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
class CBitmap;

/*============================================================================*/
// Window message map handling


enum AfxSig {
	AfxSig_end = 0,     // [marks end of message map]

	AfxSig_b_D_v,               // BOOL (CDC*)
	AfxSig_b_b_v,               // BOOL (BOOL)
	AfxSig_b_u_v,               // BOOL (UINT)
	AfxSig_b_h_v,               // BOOL (HANDLE)
	AfxSig_b_W_uu,              // BOOL (CWnd*, UINT, UINT)
	AfxSig_b_W_COPYDATASTRUCT,              // BOOL (CWnd*, COPYDATASTRUCT*)
	AfxSig_b_v_HELPINFO,        // BOOL (LPHELPINFO);
	AfxSig_CTLCOLOR,            // HBRUSH (CDC*, CWnd*, UINT)
	AfxSig_CTLCOLOR_REFLECT,    // HBRUSH (CDC*, UINT)
	AfxSig_i_u_W_u,             // int (UINT, CWnd*, UINT)  // ?TOITEM
	AfxSig_i_uu_v,              // int (UINT, UINT)
	AfxSig_i_W_uu,              // int (CWnd*, UINT, UINT)
	AfxSig_i_v_s,               // int (LPTSTR)
	AfxSig_l_w_l,               // LRESULT (WPARAM, LPARAM)
	AfxSig_l_uu_M,              // LRESULT (UINT, UINT, CMenu*)
	AfxSig_v_b_h,               // void (BOOL, HANDLE)
	AfxSig_v_h_v,               // void (HANDLE)
	AfxSig_v_h_h,               // void (HANDLE, HANDLE)
	AfxSig_v_v_v,               // void ()
	AfxSig_v_u_v,               // void (UINT)
	AfxSig_v_up_v,              // void (UINT_PTR)
	AfxSig_v_u_u,               // void (UINT, UINT)
	AfxSig_v_uu_v,              // void (UINT, UINT)
	AfxSig_v_v_ii,              // void (int, int)
	AfxSig_v_u_uu,              // void (UINT, UINT, UINT)
	AfxSig_v_u_ii,              // void (UINT, int, int)
	AfxSig_v_u_W,               // void (UINT, CWnd*)
	AfxSig_i_u_v,               // int (UINT)
	AfxSig_u_u_v,               // UINT (UINT)
	AfxSig_b_v_v,               // BOOL ()
	AfxSig_v_w_l,               // void (WPARAM, LPARAM)
	AfxSig_MDIACTIVATE,         // void (BOOL, CWnd*, CWnd*)
	AfxSig_v_D_v,               // void (CDC*)
	AfxSig_v_M_v,               // void (CMenu*)
	AfxSig_v_M_ub,              // void (CMenu*, UINT, BOOL)
	AfxSig_v_W_v,               // void (CWnd*)
	AfxSig_v_v_W,               // void (CWnd*)
	AfxSig_v_W_uu,              // void (CWnd*, UINT, UINT)
	AfxSig_v_W_p,               // void (CWnd*, CPoint)
	AfxSig_v_W_h,               // void (CWnd*, HANDLE)
	AfxSig_C_v_v,               // HCURSOR ()
	AfxSig_ACTIVATE,            // void (UINT, CWnd*, BOOL)
	AfxSig_SCROLL,              // void (UINT, UINT, CWnd*)
	AfxSig_SCROLL_REFLECT,      // void (UINT, UINT)
	AfxSig_v_v_s,               // void (LPTSTR)
	AfxSig_v_u_cs,              // void (UINT, LPCTSTR)
	AfxSig_OWNERDRAW,           // void (int, LPTSTR) force return TRUE
	AfxSig_i_i_s,               // int (int, LPTSTR)
	AfxSig_u_v_p,               // UINT (CPoint)
	AfxSig_u_v_v,               // UINT ()
	AfxSig_v_b_NCCALCSIZEPARAMS,        // void (BOOL, NCCALCSIZE_PARAMS*)
	AfxSig_v_v_WINDOWPOS,           // void (WINDOWPOS*)
	AfxSig_v_uu_M,              // void (UINT, UINT, HMENU)
	AfxSig_v_u_p,               // void (UINT, CPoint)
	AfxSig_SIZING,              // void (UINT, LPRECT)
	AfxSig_MOUSEWHEEL,          // BOOL (UINT, short, CPoint)
	AfxSig_MOUSEHWHEEL,         // void (UINT, short, CPoint)
	AfxSigCmd_v,                // void ()
	AfxSigCmd_b,                // BOOL ()
	AfxSigCmd_RANGE,            // void (UINT)
	AfxSigCmd_EX,               // BOOL (UINT)
	AfxSigNotify_v,             // void (NMHDR*, LRESULT*)
	AfxSigNotify_b,             // BOOL (NMHDR*, LRESULT*)
	AfxSigNotify_RANGE,         // void (UINT, NMHDR*, LRESULT*)
	AfxSigNotify_EX,            // BOOL (UINT, NMHDR*, LRESULT*)
	AfxSigCmdUI,                // void (CCmdUI*)
	AfxSigCmdUI_RANGE,          // void (CCmdUI*, UINT)
	AfxSigCmd_v_pv,             // void (void*)
	AfxSigCmd_b_pv,             // BOOL (void*)
	AfxSig_l,               // LRESULT ()
	AfxSig_l_p,             // LRESULT (CPOINT)
	AfxSig_u_W_u,               // UINT (CWnd*, UINT)
	AfxSig_v_u_M,               // void (UINT, CMenu* )
	AfxSig_u_u_M,               // UINT (UINT, CMenu* )
	AfxSig_u_v_MENUGETOBJECTINFO,       // UINT (MENUGETOBJECTINFO*)
	AfxSig_v_M_u,               // void (CMenu*, UINT)
	AfxSig_v_u_LPMDINEXTMENU,       // void (UINT, LPMDINEXTMENU)
	AfxSig_APPCOMMAND,          // void (CWnd*, UINT, UINT, UINT)
	AfxSig_RAWINPUT,            // void (UINT, HRAWINPUT)
	AfxSig_u_u_u,               // UINT (UINT, UINT)
	AfxSig_MOUSE_XBUTTON,           // void (UINT, UINT, CPoint)
	AfxSig_MOUSE_NCXBUTTON,         // void (short, UINT, CPoint)
	AfxSig_INPUTLANGCHANGE,         // void (UINT, UINT)
	AfxSig_v_u_hkl,                 // void (UINT, HKL)
	AfxSig_INPUTDEVICECHANGE,       // void (unsigned short, HANDLE)
	AfxSig_l_D_u,                   // LRESULT (CDC*, UINT)
	AfxSig_i_v_S,               // int (LPCTSTR)
	AfxSig_v_F_b,               // void (CFont*, BOOL)
	AfxSig_h_v_v,               // HANDLE ()
	AfxSig_h_b_h,               // HANDLE (BOOL, HANDLE)
	AfxSig_b_v_ii,              // BOOL (int, int)
	AfxSig_h_h_h,               // HANDLE (HANDLE, HANDLE)
	AfxSig_MDINext,             // void (CWnd*, BOOL)
	AfxSig_u_u_l,               // UINT (UINT, LPARAM)

	// Old
	AfxSig_bD = AfxSig_b_D_v,      // BOOL (CDC*)
	AfxSig_bb = AfxSig_b_b_v,      // BOOL (BOOL)
	AfxSig_bWww = AfxSig_b_W_uu,    // BOOL (CWnd*, UINT, UINT)
	AfxSig_hDWw = AfxSig_CTLCOLOR,    // HBRUSH (CDC*, CWnd*, UINT)
	AfxSig_hDw = AfxSig_CTLCOLOR_REFLECT,     // HBRUSH (CDC*, UINT)
	AfxSig_iwWw = AfxSig_i_u_W_u,    // int (UINT, CWnd*, UINT)
	AfxSig_iww = AfxSig_i_uu_v,     // int (UINT, UINT)
	AfxSig_iWww = AfxSig_i_W_uu,    // int (CWnd*, UINT, UINT)
	AfxSig_is = AfxSig_i_v_s,      // int (LPTSTR)
	AfxSig_lwl = AfxSig_l_w_l,     // LRESULT (WPARAM, LPARAM)
	AfxSig_lwwM = AfxSig_l_uu_M,    // LRESULT (UINT, UINT, CMenu*)
	AfxSig_vv = AfxSig_v_v_v,      // void (void)

	AfxSig_vw = AfxSig_v_u_v,      // void (UINT)
	AfxSig_vww = AfxSig_v_u_u,     // void (UINT, UINT)
	AfxSig_vww2 = AfxSig_v_uu_v,    // void (UINT, UINT) // both come from wParam
	AfxSig_vvii = AfxSig_v_v_ii,    // void (int, int) // wParam is ignored
	AfxSig_vwww = AfxSig_v_u_uu,    // void (UINT, UINT, UINT)
	AfxSig_vwii = AfxSig_v_u_ii,    // void (UINT, int, int)
	AfxSig_vwl = AfxSig_v_w_l,     // void (UINT, LPARAM)
	AfxSig_vbWW = AfxSig_MDIACTIVATE,    // void (BOOL, CWnd*, CWnd*)
	AfxSig_vD = AfxSig_v_D_v,      // void (CDC*)
	AfxSig_vM = AfxSig_v_M_v,      // void (CMenu*)
	AfxSig_vMwb = AfxSig_v_M_ub,    // void (CMenu*, UINT, BOOL)

	AfxSig_vW = AfxSig_v_W_v,      // void (CWnd*)
	AfxSig_vWww = AfxSig_v_W_uu,    // void (CWnd*, UINT, UINT)
	AfxSig_vWp = AfxSig_v_W_p,     // void (CWnd*, CPoint)
	AfxSig_vWh = AfxSig_v_W_h,     // void (CWnd*, HANDLE)
	AfxSig_vwW = AfxSig_v_u_W,     // void (UINT, CWnd*)
	AfxSig_vwWb = AfxSig_ACTIVATE,    // void (UINT, CWnd*, BOOL)
	AfxSig_vwwW = AfxSig_SCROLL,    // void (UINT, UINT, CWnd*)
	AfxSig_vwwx = AfxSig_SCROLL_REFLECT,    // void (UINT, UINT)
	AfxSig_vs = AfxSig_v_v_s,      // void (LPTSTR)
	AfxSig_vOWNER = AfxSig_OWNERDRAW,  // void (int, LPTSTR), force return TRUE
	AfxSig_iis = AfxSig_i_i_s,     // int (int, LPTSTR)
	AfxSig_wp = AfxSig_u_v_p,      // UINT (CPoint)
	AfxSig_wv = AfxSig_u_v_v,      // UINT (void)
	AfxSig_vPOS = AfxSig_v_v_WINDOWPOS,    // void (WINDOWPOS*)
	AfxSig_vCALC = AfxSig_v_b_NCCALCSIZEPARAMS,   // void (BOOL, NCCALCSIZE_PARAMS*)
	AfxSig_vNMHDRpl = AfxSigNotify_v,    // void (NMHDR*, LRESULT*)
	AfxSig_bNMHDRpl = AfxSigNotify_b,    // BOOL (NMHDR*, LRESULT*)
	AfxSig_vwNMHDRpl = AfxSigNotify_RANGE,   // void (UINT, NMHDR*, LRESULT*)
	AfxSig_bwNMHDRpl = AfxSigNotify_EX,   // BOOL (UINT, NMHDR*, LRESULT*)
	AfxSig_bHELPINFO = AfxSig_b_v_HELPINFO,   // BOOL (HELPINFO*)
	AfxSig_vwSIZING = AfxSig_SIZING,    // void (UINT, LPRECT) -- return TRUE

	// signatures specific to CCmdTarget
	AfxSig_cmdui = AfxSigCmdUI,   // void (CCmdUI*)
	AfxSig_cmduiw = AfxSigCmdUI_RANGE,  // void (CCmdUI*, UINT)
	AfxSig_vpv = AfxSigCmd_v_pv,     // void (void*)
	AfxSig_bpv = AfxSigCmd_b_pv,     // BOOL (void*)

	// Other aliases (based on implementation)
	AfxSig_vwwh = AfxSig_v_uu_M,                // void (UINT, UINT, HMENU)
	AfxSig_vwp = AfxSig_v_u_p,                 // void (UINT, CPoint)
	AfxSig_bw = AfxSig_b_u_v,      // BOOL (UINT)
	AfxSig_bh = AfxSig_b_h_v,      // BOOL (HANDLE)
	AfxSig_iw = AfxSig_i_u_v,      // int (UINT)
	AfxSig_ww = AfxSig_u_u_v,      // UINT (UINT)
	AfxSig_bv = AfxSig_b_v_v,      // BOOL (void)
	AfxSig_hv = AfxSig_C_v_v,      // HANDLE (void)
	AfxSig_vb = AfxSig_vw,      // void (BOOL)
	AfxSig_vbh = AfxSig_v_b_h,    // void (BOOL, HANDLE)
	AfxSig_vbw = AfxSig_vww,    // void (BOOL, UINT)
	AfxSig_vhh = AfxSig_v_h_h,    // void (HANDLE, HANDLE)
	AfxSig_vh = AfxSig_v_h_v,      // void (HANDLE)
	AfxSig_viSS = AfxSig_vwl,   // void (int, STYLESTRUCT*)
	AfxSig_bwl = AfxSig_lwl,
	AfxSig_vwMOVING = AfxSig_vwSIZING,  // void (UINT, LPRECT) -- return TRUE

	AfxSig_vW2 = AfxSig_v_v_W,                 // void (CWnd*) (CWnd* comes from lParam)
	AfxSig_bWCDS = AfxSig_b_W_COPYDATASTRUCT,               // BOOL (CWnd*, COPYDATASTRUCT*)
	AfxSig_bwsp = AfxSig_MOUSEWHEEL,                // BOOL (UINT, short, CPoint)
	AfxSig_vws = AfxSig_v_u_cs,
};


#define CN_COMMAND              0               // void ()
#define CN_UPDATE_COMMAND_UI    ((UINT)(-1))    // void (CCmdUI*)
#define CN_EVENT                ((UINT)(-2))    // OLE event
#define CN_OLECOMMAND           ((UINT)(-3))    // OLE document command
#define CN_OLE_UNREGISTER       ((UINT)(-4))    // OLE unregister
// > 0 are control notifications
// < 0 are for MFC's use

/*
 * PeekMessage() Options
 */
#define PM_NOREMOVE         0x0000
#define PM_REMOVE           0x0001
#define PM_NOYIELD          0x0002

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

#define BEGIN_TEMPLATE_MESSAGE_MAP(theClass, type_name, baseClass)          \
	template < typename type_name >                                         \
	const AFX_MSGMAP *theClass< type_name >::GetMessageMap() const          \
	{ return GetThisMessageMap(); }                                     \
	template < typename type_name >                                         \
	const AFX_MSGMAP *theClass< type_name >::GetThisMessageMap() {      \
		typedef theClass< type_name > ThisClass;                            \
		typedef baseClass TheBaseClass;                                     \
		static const AFX_MSGMAP_ENTRY _messageEntries[] =                   \
		{

#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
		const AFX_MSGMAP *theClass::GetMessageMap() const \
		{ return GetThisMessageMap(); } \
		const AFX_MSGMAP *theClass::GetThisMessageMap() { \
			typedef theClass ThisClass;                        \
			typedef baseClass TheBaseClass;                    \
			static const AFX_MSGMAP_ENTRY _messageEntries[] =  \
			{

#define END_MESSAGE_MAP() \
	{ 0, 0, 0, 0, AfxSig_end, (AFX_PMSG)nullptr } \
	}; \
	static const AFX_MSGMAP messageMap = \
	{ &TheBaseClass::GetThisMessageMap, &_messageEntries[0] }; \
	return &messageMap; \
	}                                 \

#define DECLARE_DYNCREATE(class_name) \
	DECLARE_DYNAMIC(class_name) \
	static CObject *CreateObject();

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
	HGDIOBJ m_hObject;

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
	CBrush();
	CBrush(CBitmap *pBitmap);
	CBrush(COLORREF crColor);
	CBrush(int nIndex, COLORREF crColor);
	~CBrush() override {
	}

	BOOL CreateSolidBrush(COLORREF crColor);
	BOOL CreateBrushIndirect(const LOGBRUSH *lpLogBrush);
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
	int GetObject(int nCount, LPVOID lpObject) const;
	LONG GetBitmapBits(LONG dwCount, LPVOID lpBits) const;
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
	BOOL AnimatePalette(UINT nStartIndex, UINT nNumEntries,
	                    const PALETTEENTRY *lpPaletteColors);
	UINT GetNearestPaletteIndex(COLORREF crColor);
};

class CRgn : public CGdiObject {
public:
	~CRgn() override {
	}

	BOOL CreatePolygonRgn(
	    const POINT *lpPoints,
	    int nCount,
	    int nPolyFillMode
	);
};

class CDC : public CObject {
	DECLARE_DYNAMIC(CDC)
public:
	HDC m_hDC = nullptr;

	static CDC *PASCAL FromHandle(HDC hDC);

public:
	~CDC() override {
	}

	BOOL CreateDC(LPCSTR lpszDriverName, LPCSTR lpszDeviceName,
	              LPCSTR lpszOutput, const void *lpInitData);
	BOOL CreateCompatibleDC(CDC *pDC);
	BOOL DeleteDC();
	void Attach(HDC hDC);
	void Detach();
	int SetStretchBltMode(int nStretchMode);
	int GetDeviceCaps(int nIndex) const;
	int SetROP2(int nDrawMode);

	BOOL BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	            int xSrc, int ySrc, DWORD dwRop);
	BOOL StretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	                int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop);
	void Ellipse(LPCRECT lpRect);
	void Ellipse(int x1, int y1, int x2, int y2);
	void FrameRect(LPCRECT lpRect, CBrush *pBrush);
	void FillRect(LPCRECT lpRect, CBrush *pBrush);
	BOOL FloodFill(int x, int y, COLORREF crColor);
	BOOL FloodFill(int x, int y, COLORREF crColor,
	               UINT nFillType);
	void Rectangle(LPCRECT lpRect);
	void Rectangle(int x1, int y1, int x2, int y2);
	BOOL Pie(
	    int x1, int y1,     // Upper-left corner of bounding rectangle
	    int x2, int y2,     // Lower-right corner of bounding rectangle
	    int x3, int y3,     // Starting point of the arc (on the ellipse)
	    int x4, int y4      // Ending point of the arc (on the ellipse)
	);
	BOOL Pie(
	    LPCRECT lpRect,         // Pointer to bounding rectangle
	    const POINT &ptStart,   // Start point of arc
	    const POINT &ptEnd      // End point of arc
	);
	BOOL FrameRgn(CRgn *pRgn, CBrush *pBrush,
	              int nWidth, int nHeight);

	void MoveTo(int x, int y);
	void LineTo(int x, int y);
	COLORREF GetPixel(int x, int y) const;
	COLORREF GetPixel(const POINT &point) const;

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

	static const AFX_MSGMAP *GetThisMessageMap();
	virtual const AFX_MSGMAP *GetMessageMap() const {
		return GetThisMessageMap();
	}

	void BeginWaitCursor();
	void EndWaitCursor();
	void RestoreWaitCursor();
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
	virtual void DoDataExchange(CDataExchange *dataExch);
	DECLARE_MESSAGE_MAP()

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
	CVIRTUAL BOOL Create(LPCSTR lpszClassName, LPCSTR lpszWindowName,
	                     DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID,
	                     CCreateContext *pContext = nullptr);

	CWnd *GetParent() const;
	HWND GetSafeHwnd() const;
	void ShowWindow(int nCmdShow);
	BOOL EnableWindow(BOOL bEnable = TRUE);
	void UpdateWindow();
	BOOL RedrawWindow(
	    LPCRECT lpRectUpdate = nullptr,
	    CRgn *prgnUpdate = nullptr,
	    UINT flags = RDW_INVALIDATE | RDW_UPDATENOW
	);
	void SetActiveWindow();
	void SetFocus();
	BOOL IsWindowVisible() const;
	void DestroyWindow();
	void Invalidate(BOOL bErase = TRUE);
	int GetWindowText(CString &rString) const;
	int GetWindowText(LPSTR lpszStringBuf, int nMaxCount) const;
	BOOL SetWindowText(LPCSTR lpszString);
	UINT GetState() const;

	CDC *GetDC();
	int ReleaseDC(CDC *pDC);

	BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

	BOOL ValidateRect(LPCRECT lpRect = NULL);
	BOOL InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE);
	void GetWindowRect(LPRECT lpRect) const;
	BOOL GetUpdateRect(LPRECT lpRect, BOOL bErase = FALSE);
	BOOL GetClientRect(LPRECT lpRect) const;
	void MoveWindow(LPCRECT lpRect, BOOL bRepaint = true);
	void MoveWindow(int x, int y, int nWidth, int nHeight,
	                BOOL bRepaint = TRUE);
	HDC BeginPaint(LPPAINTSTRUCT lpPaint);
	BOOL EndPaint(const PAINTSTRUCT *lpPaint);

	CWnd *GetDlgItem(int nID) const;
	CWnd *GetNextDlgGroupItem(CWnd *pWndCtl, BOOL bPrevious = FALSE) const;
	BOOL GotoDlgCtrl(CWnd *pWndCtrl);
	BOOL SubclassDlgItem(UINT nID, CWnd *pParent);
	BOOL SetDlgItemText(int nIDDlgItem, LPCSTR lpString);
	int GetDlgCtrlID() const;
	void CheckDlgButton(int nIDButton, UINT nCheck);
	LRESULT SendDlgItemMessage(int nID, UINT message,
	                           WPARAM wParam = 0, LPARAM lParam = 0) const;

	UINT_PTR SetTimer(UINT_PTR nIDEvent, UINT nElapse,
	                  void (CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) = NULL);
	BOOL KillTimer(UINT_PTR nIDEvent);
};

class CFrameWnd : public CWnd {
	DECLARE_DYNCREATE(CFrameWnd)

protected:
	DECLARE_MESSAGE_MAP()

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

	int DoModal();
	virtual BOOL OnInitDialog();

	DWORD GetDefID();
	void SetDefID(UINT nID);
	void AFXAPI DDX_Control(CDataExchange *pDX, int nIDC, CWnd &rControl);
	void AFXAPI DDX_Radio(CDataExchange *pDX,
	                      int nIDCButton1, int &value);
	void AFXAPI DDX_Text(CDataExchange *pDX,
	                     int nIDC, int &value);
	void AFXAPI DDX_Text(CDataExchange *pDX, int nIDC, CString &value);
	void AFXAPI DDX_Text(CDataExchange *pDX, int nIDC, UINT &value);
	void AFXAPI DDX_Text(CDataExchange *pDX, int nIDC, long &value);
	void AFXAPI DDX_Text(CDataExchange *pDX, int nIDC, double &value);
	void AFXAPI DDX_Check(CDataExchange *pDX,
	                      int nIDC, int &value);
	void AFXAPI DDV_MinMaxInt(CDataExchange *pDX,
	                          int value, int nMin, int nMax);

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
	CVIRTUAL BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle,
	                     const RECT &rect, CWnd *pParentWnd, UINT nID);

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

protected:
	DECLARE_MESSAGE_MAP()

public:
	~CEdit() override {
	}
	CVIRTUAL BOOL Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID);
	void LimitText(int nChars);
	void SetSel(int nStartChar, int nEndChar, BOOL bNoScroll = FALSE);
};

class CScrollBar : public CWnd {
	DECLARE_DYNAMIC(CScrollBar)

protected:
	DECLARE_MESSAGE_MAP()

public:
	~CScrollBar() override { }
	CVIRTUAL BOOL Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID);

	int GetScrollPos() const;
	int SetScrollPos(int nPos, BOOL bRedraw = true);
	void GetScrollRange(LPINT lpMinPos, LPINT lpMaxPos) const;
	void SetScrollRange(int nMinPos, int nMaxPos, BOOL bRedraw);
	void ShowScrollBar(BOOL bShow);
};

class CView : public CWnd {
	DECLARE_DYNAMIC(CView)

protected:
	DECLARE_MESSAGE_MAP()

public:
	~CView() override {
	}
};

class CScrollView : public CView {
	DECLARE_DYNAMIC(CScrollView)

protected:
	DECLARE_MESSAGE_MAP()

public:
	~CScrollView() override {
	}
};

class CWinThread : public CCmdTarget {
	DECLARE_DYNAMIC(CWinThread)
public:
	~CWinThread() override {
	}

	virtual BOOL OnIdle(LONG lCount);
};

class CWinApp : public CWinThread {
	DECLARE_DYNAMIC(CWinApp)
public:
	CWinApp(const char *appName);
	~CWinApp() override {
	}

	virtual BOOL PreTranslateMessage(MSG *pMsg);
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
