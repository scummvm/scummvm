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

#include "common/events.h"
#include "common/fs.h"
#include "graphics/palette.h"
#include "graphics/screen.h"
#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/wingdi.h"
#include "bagel/mfc/afx.h"
#include "bagel/mfc/afxmsg.h"
#include "bagel/mfc/afxstr.h"
#include "bagel/mfc/atltypes.h"
#include "bagel/mfc/global_functions.h"
#include "bagel/mfc/gfx/clipped_surface.h"
#include "bagel/mfc/gfx/cursor.h"
#include "bagel/mfc/gfx/dialog_template.h"
#include "bagel/mfc/gfx/fonts.h"
#include "bagel/mfc/gfx/text_render.h"
#include "bagel/mfc/libs/array.h"
#include "bagel/mfc/libs/event_loop.h"
#include "bagel/mfc/libs/events.h"
#include "bagel/mfc/libs/list.h"
#include "bagel/mfc/libs/settings.h"

namespace Bagel {
namespace MFC {

class CBitmap;
class CCmdTarget;
class CCmdUI;
class CDC;
class CDocTemplate;
class CDocument;
class CFrameWnd;
class CListBox;
class CScrollBar;
class CView;
class CWinThread;
class CWnd;

// Dummy class definitions that may need replacing later
class CDataExchange {};
struct COPYDATASTRUCT {};
struct HELPINFO {};
struct NCCALCSIZE_PARAMS {};
struct NMHDR {};
struct WINDOWPOS {};

/*============================================================================*/
// CMenu

class CMenu : public CObject {
public:
	// Constructors
	CMenu();

	BOOL CreateMenu();
	BOOL CreatePopupMenu();
	BOOL LoadMenu(LPCTSTR lpszResourceName);
	BOOL LoadMenu(UINT nIDResource);
	BOOL LoadMenuIndirect(const void *lpMenuTemplate);
	BOOL DestroyMenu();

	static CMenu *FromHandle(HMENU hMenu);

	UINT CheckMenuItem(UINT nIDCheckItem, UINT nCheck);
};

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
	AfxSig_vwpb,                // void (HFONT, BOOL)
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


union MessageMapFunctions {
	AFX_PMSG pfn;   // generic member function pointer

	// specific type safe variants for WM_COMMAND and WM_NOTIFY messages
	void (AFX_MSG_CALL CCmdTarget:: *pfn_COMMAND)();
	BOOL(AFX_MSG_CALL CCmdTarget:: *pfn_bCOMMAND)();
	void (AFX_MSG_CALL CCmdTarget:: *pfn_COMMAND_RANGE)(UINT);
	BOOL(AFX_MSG_CALL CCmdTarget:: *pfn_COMMAND_EX)(UINT);

	void (AFX_MSG_CALL CCmdTarget:: *pfn_UPDATE_COMMAND_UI)(CCmdUI *);
	void (AFX_MSG_CALL CCmdTarget:: *pfn_UPDATE_COMMAND_UI_RANGE)(CCmdUI *, UINT);
	void (AFX_MSG_CALL CCmdTarget:: *pfn_OTHER)(void *);
	BOOL(AFX_MSG_CALL CCmdTarget:: *pfn_OTHER_EX)(void *);

	void (AFX_MSG_CALL CCmdTarget:: *pfn_NOTIFY)(NMHDR *, LRESULT *);
	BOOL(AFX_MSG_CALL CCmdTarget:: *pfn_bNOTIFY)(NMHDR *, LRESULT *);
	void (AFX_MSG_CALL CCmdTarget:: *pfn_NOTIFY_RANGE)(UINT, NMHDR *, LRESULT *);
	BOOL(AFX_MSG_CALL CCmdTarget:: *pfn_NOTIFY_EX)(UINT, NMHDR *, LRESULT *);

	// Type safe variant for thread messages

	void (AFX_MSG_CALL CWinThread:: *pfn_THREAD)(WPARAM, LPARAM);

	// Specific type safe variants for WM-style messages
	BOOL(AFX_MSG_CALL CWnd:: *pfn_bD)(CDC *);
	BOOL(AFX_MSG_CALL CWnd:: *pfn_bb)(BOOL);
	BOOL(AFX_MSG_CALL CWnd:: *pfn_bWww)(CWnd *, UINT, UINT);
	BOOL(AFX_MSG_CALL CWnd:: *pfn_bHELPINFO)(HELPINFO *);
	BOOL(AFX_MSG_CALL CWnd:: *pfn_bWCDS)(CWnd *, COPYDATASTRUCT *);
	HBRUSH(AFX_MSG_CALL CWnd:: *pfn_hDWw)(CDC *, CWnd *, UINT);
	HBRUSH(AFX_MSG_CALL CWnd:: *pfn_hDw)(CDC *, UINT);
	int (AFX_MSG_CALL CWnd:: *pfn_iwWw)(UINT, CWnd *, UINT);
	int (AFX_MSG_CALL CWnd:: *pfn_iww)(UINT, UINT);
	int (AFX_MSG_CALL CWnd:: *pfn_iWww)(CWnd *, UINT, UINT);
	int (AFX_MSG_CALL CWnd:: *pfn_is)(LPTSTR);
	LRESULT(AFX_MSG_CALL CWnd:: *pfn_lwl)(WPARAM, LPARAM);
	LRESULT(AFX_MSG_CALL CWnd:: *pfn_lwwM)(UINT, UINT, CMenu *);
	void (AFX_MSG_CALL CWnd:: *pfn_vv)(void);

	void (AFX_MSG_CALL CWnd:: *pfn_vw)(UINT);
	void (AFX_MSG_CALL CWnd:: *pfn_vww)(UINT, UINT);
	void (AFX_MSG_CALL CWnd:: *pfn_vvii)(int, int);
	void (AFX_MSG_CALL CWnd:: *pfn_vwww)(UINT, UINT, UINT);
	void (AFX_MSG_CALL CWnd:: *pfn_vwii)(UINT, int, int);
	void (AFX_MSG_CALL CWnd:: *pfn_vwl)(WPARAM, LPARAM);
	void (AFX_MSG_CALL CWnd:: *pfn_vbWW)(BOOL, CWnd *, CWnd *);
	void (AFX_MSG_CALL CWnd:: *pfn_vD)(CDC *);
	void (AFX_MSG_CALL CWnd:: *pfn_vM)(CMenu *);
	void (AFX_MSG_CALL CWnd:: *pfn_vMwb)(CMenu *, UINT, BOOL);

	void (AFX_MSG_CALL CWnd:: *pfn_vW)(CWnd *);
	void (AFX_MSG_CALL CWnd:: *pfn_vWww)(CWnd *, UINT, UINT);
	void (AFX_MSG_CALL CWnd:: *pfn_vWp)(CWnd *, CPoint);
	void (AFX_MSG_CALL CWnd:: *pfn_vWh)(CWnd *, HANDLE);
	void (AFX_MSG_CALL CWnd:: *pfn_vwW)(UINT, CWnd *);
	void (AFX_MSG_CALL CWnd:: *pfn_vwWb)(UINT, CWnd *, BOOL);
	void (AFX_MSG_CALL CWnd:: *pfn_vwwW)(UINT, UINT, CWnd *);
	void (AFX_MSG_CALL CWnd:: *pfn_vwwx)(UINT, UINT);
	void (AFX_MSG_CALL CWnd:: *pfn_vs)(LPTSTR);
	void (AFX_MSG_CALL CWnd:: *pfn_vOWNER)(int, LPTSTR);      // force return TRUE
	int (AFX_MSG_CALL CWnd:: *pfn_iis)(int, LPTSTR);
	UINT(AFX_MSG_CALL CWnd:: *pfn_wp)(CPoint);
	UINT(AFX_MSG_CALL CWnd:: *pfn_wv)(void);
	void (AFX_MSG_CALL CWnd:: *pfn_vPOS)(WINDOWPOS *);
	void (AFX_MSG_CALL CWnd:: *pfn_vCALC)(BOOL, NCCALCSIZE_PARAMS *);
	void (AFX_MSG_CALL CWnd:: *pfn_vwp)(UINT, CPoint);
	void (AFX_MSG_CALL CWnd:: *pfn_vwwh)(UINT, UINT, HANDLE);
	BOOL(AFX_MSG_CALL CWnd:: *pfn_bwsp)(UINT, short, CPoint);
	void (AFX_MSG_CALL CWnd:: *pfn_vws)(UINT, LPCTSTR);
	void (AFX_MSG_CALL CWnd:: *pfn_vFb)(HFONT, BOOL);
};

/*============================================================================*/

class CArchive {
public:
	bool IsStoring() const {
		return false;
	}
};

/*============================================================================*/

// Creation information structure
// All fields are optional and may be nullptr
struct CCreateContext {
	// for creating new views
	const CRuntimeClass *m_pNewViewClass = nullptr; // runtime class of view to create or nullptr
	CDocument *m_pCurrentDoc = nullptr;

	// for creating MDI children (CMDIChildWnd::LoadFrame)
	CDocTemplate *m_pNewDocTemplate = nullptr;

	// for sharing view/frame state from the original view/frame
	CView *m_pLastView = nullptr;
	CFrameWnd *m_pCurrentFrame = nullptr;
};

/*============================================================================*/
// Implementation of command routing

struct AFX_CMDHANDLERINFO {
	void (CCmdTarget::*pmf)();
	CCmdTarget *pTarget;
};

/*============================================================================*/
// WM_NOTIFY support

struct AFX_NOTIFY {
	LRESULT *pResult;
	NMHDR *pNMHDR;
};

/*============================================================================*/

class CGdiObject : public CObject {
	DECLARE_DYNCREATE(CGdiObject)

private:
	bool _permanent = false;

protected:
	void AfxHookObject();
	void AfxUnhookObject();

public:
	HGDIOBJ m_hObject = nullptr;

	operator HGDIOBJ() const {
		return m_hObject;
	}

public:
	static CGdiObject *FromHandle(HGDIOBJ h);

public:
	~CGdiObject() override;

	BOOL Attach(HGDIOBJ hObject);
	HGDIOBJ Detach();
	BOOL DeleteObject();

	BOOL operator==(const CGdiObject &obj) const {
		return obj.m_hObject == m_hObject;
	}
	BOOL operator!=(const CGdiObject &obj) const {
		return obj.m_hObject != m_hObject;
	}
};

class CPen : public CGdiObject {
public:
	class Impl : public CGdiObjectImpl {
	public:
		int _penStyle;
		int _width;
		COLORREF _color;

	public:
		Impl(int nPenStyle, int nWidth, COLORREF crColor) :
			_penStyle(nPenStyle), _width(nWidth),
			_color(crColor) {
		}
	};

	Impl *&_pen = *(Impl **)&m_hObject;

public:
	CPen() {}
	CPen(int nPenStyle, int nWidth, COLORREF crColor);
	~CPen() override {}

	BOOL CreatePen(int nPenStyle, int nWidth, COLORREF crColor);
};

class CBrush : public CGdiObject {
public:
	struct Impl : public CGdiObjectImpl {
		int _type;
		COLORREF _color = 0;

		Impl();
		Impl(COLORREF crColor);
		Impl(int nIndex, COLORREF crColor);
		Impl(CBitmap *pBitmap);
		~Impl() override {}

		byte getColor() const;
	};

	Impl *&_brush = *(Impl **)&m_hObject;

public:
	static CBrush *FromHandle(HGDIOBJ h) {
		return (CBrush *)CGdiObject::FromHandle(h);
	}

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
	struct Impl : public CGdiObjectImpl {
	public:
		Gfx::Font *_font;
	public:
		Impl(Gfx::Font *font) : _font(font) {}
		~Impl() override {}
		operator Gfx::Font *() const {
			return _font;
		}
	};

	Impl *&_font = *(Impl **)&m_hObject;

public:
	static CFont *FromHandle(HGDIOBJ h) {
		return (CFont *)CGdiObject::FromHandle(h);
	}

public:
	~CFont() override {}

	BOOL CreateFont(int nHeight, int nWidth, int nEscapement,
	    int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline,
	    BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision,
	    BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily,
	    LPCSTR lpszFacename);
	BOOL CreateFontIndirect(const LOGFONT *lpLogFont);
};

class CBitmap : public CGdiObject {
public:
	struct Impl : public CGdiObjectImpl,
		public Gfx::ClippedSurface {
		~Impl() override {}
	};

	Impl *&_bitmap = *(Impl **)&m_hObject;

public:
	static CBitmap *FromHandle(HGDIOBJ h) {
		return (CBitmap *)CGdiObject::FromHandle(h);
	}

public:
	~CBitmap() override {
	}

	BOOL Attach(HBITMAP hObject);
	HBITMAP Detach();
	BOOL CreateCompatibleBitmap(CDC *pDC, int nWidth, int nHeight);
	BOOL CreateBitmap(int nWidth, int nHeight, UINT nPlanes,
	    UINT nBitcount, const void *lpBits);
	int GetObject(int nCount, LPVOID lpObject) const;
	LONG GetBitmapBits(LONG dwCount, LPVOID lpBits) const;
	BOOL GetBitmap(BITMAP *pBitMap) const {
		return GetObject(sizeof(BITMAP), pBitMap);
	}
};

class CPalette : public CGdiObject {
public:
	struct Impl : public CGdiObjectImpl,
			public Graphics::Palette {
		Impl(const LPLOGPALETTE pal);
		~Impl() override {}
	};

	Impl *&_palette = *(Impl **)&m_hObject;

public:
	static CPalette *FromHandle(HGDIOBJ h) {
		return (CPalette *)CGdiObject::FromHandle(h);
	}

public:
	~CPalette() override {
	}

	BOOL CreatePalette(LPLOGPALETTE lpLogPalette);
	int GetObject(int nCount, LPVOID lpObject) const;
	UINT GetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
	                       LPPALETTEENTRY lpPaletteColors) const;
	UINT SetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
	                       LPPALETTEENTRY lpPaletteColors);
	UINT SetPaletteEntries(const Graphics::Palette &pal);
	BOOL AnimatePalette(UINT nStartIndex, UINT nNumEntries,
	                    const PALETTEENTRY *lpPaletteColors);
	UINT GetNearestPaletteIndex(COLORREF crColor);
	bool isEmpty() const {
		return !_palette || _palette->empty();
	}
};

class CRgn : public CGdiObject {
public:
	Common::Array<POINT> _points;
	Common::Rect _rect;
	int _polyFillMode = 0;

public:
	~CRgn() override {
	}

	BOOL CreateRectRgn(int x1, int y1, int x2, int y2);
	BOOL CreateRectRgnIndirect(LPCRECT lpRect);
	BOOL CreatePolygonRgn(const POINT *lpPoints,
	                      int nCount, int nPolyFillMode);
};

class CDC : public CObject {
	DECLARE_DYNCREATE(CDC)
	friend class CWnd;

private:
	bool _permanent = false;
	int _stretchMode = STRETCH_ANDSCANS;

protected:
	void AfxHookObject();
	void AfxUnhookObject();

public:
	class DefaultBitmap : public CBitmap {
	public:
		~DefaultBitmap() override {
		};
	};

	class Impl : public Gfx::TextRender {
	private:
		DefaultBitmap _defaultBitmap;
		Common::Point _linePos;
		COLORREF _bkColor = RGB(255, 255, 255);
		int _bkMode = TRANSPARENT;
		COLORREF _textColor = 0;
		uint _textAlign = TA_LEFT;
		int _drawMode;
		CPoint _viewportOrigin;

		uint getPenColor() const;
		uint getBrushColor() const;

	public:
		HBITMAP _bitmap;
		HPEN _pen;
		HFONT _font;
		HBRUSH _brush;
		HPALETTE _palette = nullptr;
		CPalette *_cPalette = nullptr;

	public:
		Impl();
		Impl(HDC srcDc);

		HGDIOBJ Attach(HGDIOBJ gdiObj);
		Gfx::ClippedSurface *getSurface() const;
		const Graphics::PixelFormat &getFormat() const;
		void setFormat(const Graphics::PixelFormat &format);
		void setScreenRect();
		void setScreenRect(const Common::Rect &r);
		CPoint setViewportOrg(const CPoint &pt);
		CPoint offsetViewportOrg(int xDelta, int yDelta);

		HPALETTE selectPalette(HPALETTE pal);
		CPalette *selectPalette(CPalette *pal);
		UINT realizePalette();
		COLORREF GetNearestColor(COLORREF crColor) const;

		void fillSolidRect(LPCRECT lpRect, COLORREF clr);
		void fillSolidRect(int x, int y, int cx, int cy, COLORREF clr);
		void fillRect(const Common::Rect &r, COLORREF crColor);
		void drawRect(const Common::Rect &r, CBrush *brush);
		void frameRgn(const CRgn *pRgn, CBrush *brush, int nWidth, int nHeight);
		void floodFill(int x, int y, COLORREF crColor);
		void floodFill(int x, int y, COLORREF crColor,
			UINT nFillType);
		void rectangle(LPCRECT lpRect);
		void rectangle(int x1, int y1, int x2, int y2);
		void draw3dRect(const CRect &rect, COLORREF clrTopLeft, COLORREF clrBottomRight);
		void drawFocusRect(const CRect &rect);

		void ellipse(const Common::Rect &r, COLORREF crColor);
		void ellipse(LPCRECT lpRect);
		void ellipse(int x1, int y1, int x2, int y2);

		void bitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
			int xSrc, int ySrc, DWORD dwRop);
		void stretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
			int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop);

		void moveTo(int x, int y);
		void lineTo(int x, int y);
		COLORREF getPixel(int x, int y) const;
		int setROP2(int nDrawMode);

		// Text functions
		COLORREF setBkColor(COLORREF crColor);
		COLORREF getBkColor() const;
		COLORREF getBkPixel() const;
		int setBkMode(int nBkMode);
		COLORREF setTextColor(COLORREF crColor);
		BOOL textOut(int x, int y, LPCSTR lpszString, int nCount,
			int nTabPositions = 0, const LPINT lpnTabStopPositions = nullptr,
			int nTabOrigin = 0, CSize *size = nullptr);
		BOOL textOut(int x, int y, const CString &str,
			int nTabPositions = 0, const LPINT lpnTabStopPositions = nullptr,
			int nTabOrigin = 0, CSize *size = nullptr);
		BOOL extTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
			LPCSTR lpszString, UINT nCount, LPINT lpDxWidths);
		BOOL extTextOut(int x, int y, UINT nOptions, LPCRECT lpRect,
			const CString &str, LPINT lpDxWidths);
		CSize tabbedTextOut(int x, int y, LPCSTR lpszString, int nCount,
			int nTabPositions, const LPINT lpnTabStopPositions, int nTabOrigin);
		CSize tabbedTextOut(int x, int y, const CString &str,
			int nTabPositions, const LPINT lpnTabStopPositions, int nTabOrigin);

		int drawText(LPCSTR lpszString, int nCount, LPRECT lpRect, UINT nFormat,
			int nTabPositions = 0, const LPINT lpnTabStopPositions = nullptr,
			int nTabOrigin = 0, CSize *size = nullptr);
		int drawText(const CString &str, LPRECT lpRect, UINT nFormat,
			int nTabPositions = 0, const LPINT lpnTabStopPositions = nullptr,
			int nTabOrigin = 0, CSize *size = nullptr);

		CSize getTextExtent(LPCSTR lpszString, int nCount) const;
		CSize getTextExtent(const CString &str) const;
		CSize getOutputTextExtent(LPCSTR lpszString, int nCount) const;
		CSize getOutputTextExtent(const CString &str) const;
		CSize getTabbedTextExtent(LPCSTR lpszString, int nCount,
			int nTabPositions, LPINT lpnTabStopPositions) const;
		CSize getTabbedTextExtent(const CString &str,
			int nTabPositions, LPINT lpnTabStopPositions) const;
		CSize getOutputTabbedTextExtent(LPCSTR lpszString, int nCount,
			int nTabPositions, LPINT lpnTabStopPositions) const;
		CSize getOutputTabbedTextExtent(const CString &str,
			int nTabPositions, LPINT lpnTabStopPositions) const;
		BOOL grayString(CBrush *pBrush,
			BOOL(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
			int nCount, int x, int y, int nWidth, int nHeight);
		UINT getTextAlign() const;
		UINT setTextAlign(UINT nFlags);
		BOOL getTextMetrics(LPTEXTMETRIC lpMetrics) const;
	};

public:
	HDC m_hDC = nullptr;

	static CDC *FromHandle(HDC hDC);

public:
	~CDC() override;

	operator HDC() const {
		return m_hDC;
	}
	Impl *impl() const {
		return static_cast<Impl *>(m_hDC);
	}

	BOOL CreateDC(LPCSTR lpszDriverName, LPCSTR lpszDeviceName,
	              LPCSTR lpszOutput, const void *lpInitData);
	BOOL CreateCompatibleDC(CDC *pDC);
	BOOL DeleteDC();
	BOOL Attach(HDC hDC);
	HDC Detach();
	int SetStretchBltMode(int nStretchMode);
	int GetDeviceCaps(int nIndex) const;

	// Mapping Functions
	int GetMapMode() const;
	CPoint GetViewportOrg() const;
	virtual int SetMapMode(int nMapMode);
	// Viewport Origin
	virtual CPoint SetViewportOrg(int x, int y);
	CPoint SetViewportOrg(POINT point);
	virtual CPoint OffsetViewportOrg(int nWidth, int nHeight);

	// Clipping Functions
	virtual int GetClipBox(LPRECT lpRect) const;
	void setClipRect(const Common::Rect &r);
	void resetClipRect();
	virtual BOOL PtVisible(int x, int y) const;
	BOOL PtVisible(POINT point) const;
	virtual BOOL RectVisible(LPCRECT lpRect) const;
	int SelectClipRgn(CRgn *pRgn);
	int ExcludeClipRect(int x1, int y1, int x2, int y2);
	int ExcludeClipRect(LPCRECT lpRect);
	int ExcludeUpdateRgn(CWnd *pWnd);
	int IntersectClipRect(int x1, int y1, int x2, int y2);
	int IntersectClipRect(LPCRECT lpRect);
	int OffsetClipRgn(int x, int y);
	int OffsetClipRgn(SIZE size);
	int SelectClipRgn(CRgn *pRgn, int nMode);

	int SetROP2(int nDrawMode);
	BOOL DPtoLP(LPPOINT lpPoints, int nCount = 1);
	BOOL DPtoLP(RECT *lpRect);
	BOOL LPtoDP(RECT *lpRect);
	BOOL BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	            int xSrc, int ySrc, DWORD dwRop);
	BOOL StretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	                int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop);
	void Ellipse(LPCRECT lpRect);
	void Ellipse(int x1, int y1, int x2, int y2);
	void FrameRect(LPCRECT lpRect, CBrush *pBrush);
	void Draw3dRect(const CRect &rect, COLORREF clrTopLeft, COLORREF clrBottomRight);
	void DrawFocusRect(const CRect &rect);
	void FillRect(LPCRECT lpRect, CBrush *pBrush);
	void FillSolidRect(LPCRECT lpRect, COLORREF crColor);
	BOOL FloodFill(int x, int y, COLORREF crColor);
	BOOL FloodFill(int x, int y, COLORREF crColor,
	               UINT nFillType);
	void Rectangle(LPCRECT lpRect);
	void Rectangle(int x1, int y1, int x2, int y2);
	BOOL DrawEdge(LPRECT lpRect, UINT nEdge, UINT nFlags);

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
	HGDIOBJ SelectObject(HGDIOBJ hGdiObj);

	COLORREF GetNearestColor(COLORREF crColor) const;
	CPalette *SelectPalette(CPalette *pPalette, BOOL bForceBackground);
	UINT RealizePalette();
	void UpdateColors();
	virtual COLORREF SetBkColor(COLORREF crColor);
	COLORREF GetBkColor() const;
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
	int DrawText(LPCSTR lpszString, int nCount,
	             LPRECT lpRect, UINT nFormat);
	int DrawText(const CString &str, LPRECT lpRect, UINT nFormat);


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
	PAINTSTRUCT m_ps;
	HWND m_hWnd = nullptr;

public:
	explicit CPaintDC(CWnd *pWnd);
	~CPaintDC() override;
};

class CCmdTarget : public CObject {
	DECLARE_DYNAMIC(CCmdTarget)

protected:
	const AFX_MSGMAP_ENTRY *LookupMessage(UINT message);
	const AFX_MSGMAP_ENTRY *AfxFindMessageEntry(
		const AFX_MSGMAP_ENTRY *lpEntry,
		UINT nMsg, UINT nCode, UINT nID);
	bool _AfxDispatchCmdMsg(CCmdTarget *pTarget,
		UINT nID, int nCode, AFX_PMSG pfn, void *pExtra,
		UINT nSig, AFX_CMDHANDLERINFO *pHandlerInfo);

public:
	~CCmdTarget() override {
	}

	static const AFX_MSGMAP *GetThisMessageMap();
	virtual const AFX_MSGMAP *GetMessageMap() const {
		return GetThisMessageMap();
	}

	/**
	 * Adds or remove a wait mouse cursor
	 * @param nCode     0 => restore, 1=> begin, -1=> end
	 */
	virtual void DoWaitCursor(int nCode);
	void BeginWaitCursor();
	void EndWaitCursor();
	void RestoreWaitCursor();

	BOOL OnCmdMsg(UINT nID, int nCode, void *pExtra,
		AFX_CMDHANDLERINFO *pHandlerInfo);
};

class CCmdUI {
public:
	bool m_bContinueRouting = false;
	int m_nID = 0;
};

typedef Libs::Array<CView *> ViewArray;

class CDocument : public CCmdTarget {
	DECLARE_DYNAMIC(CDocument)
private:
	CString _title;
	CString _unusedPathName;
	BOOL m_bModified = FALSE;
	ViewArray m_viewList;

public:
	BOOL m_bAutoDelete = TRUE;	// default to auto delete document
	BOOL m_bEmbedded = FALSE;	// default to file-based document
	CDocTemplate *m_pDocTemplate = nullptr;

public:
	~CDocument() override {}

	void UpdateAllViews(CView *pSender, LPARAM lHint = 0,
	                    CObject *pHint = nullptr);
	POSITION GetFirstViewPosition();
	CView *GetNextView(POSITION &rPosition) const;

	const CString &GetTitle() const;
	virtual void SetTitle(LPCSTR lpszTitle);
	const CString &GetPathName() const;
	virtual void SetPathName(LPCSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual void ClearPathName();

	virtual BOOL IsModified();
	virtual void SetModifiedFlag(BOOL bModified = TRUE);
	virtual void ReportSaveLoadException(LPCSTR lpszPathName,
		CException *e, BOOL bSaving, UINT nIDPDefault);
	bool SaveModified();

	void AddView(CView *pView);
	void RemoveView(CView *pView);

	// Delete doc items etc
	virtual void DeleteContents();

	virtual BOOL OnNewDocument() {
		return true;
	}
	virtual BOOL OnOpenDocument(LPCSTR lpszPathName) {
		return true;
	}
	virtual void OnFileSaveAs() {}
	virtual void OnCloseDocument();
	virtual void OnChangedViewList();
	void UpdateFrameCounts();

	DECLARE_MESSAGE_MAP()
};


typedef struct tagCREATESTRUCTA {
	LPVOID      lpCreateParams = nullptr;
	HINSTANCE   hInstance = 0;
	HMENU       hMenu = 0;
	HWND        hwndParent = 0;
	int         cy = 0;
	int         cx = 0;
	int         y = 0;
	int         x = 0;
	LONG        style = 0;
	LPCSTR      lpszName = nullptr;
	LPCSTR      lpszClass = nullptr;
	DWORD       dwExStyle = 0;
} CREATESTRUCT, *LPCREATESTRUCT;


// CWnd::m_nFlags (generic to CWnd)
#define WF_TOOLTIPS         0x0001  // window is enabled for tooltips
#define WF_TEMPHIDE         0x0002  // window is temporarily hidden
#define WF_STAYDISABLED     0x0004  // window should stay disabled
#define WF_MODALLOOP        0x0008  // currently in modal loop
#define WF_CONTINUEMODAL    0x0010  // modal loop should continue running
#define WF_OLECTLCONTAINER  0x0100  // some descendant is an OLE control
#define WF_TRACKINGTOOLTIPS 0x0400  // window is enabled for tracking tooltips

class CWnd : public CCmdTarget {
	DECLARE_DYNCREATE(CWnd)

protected:
	static const MSG *GetCurrentMessage();

	virtual void DoDataExchange(CDataExchange *dataExch) {}
	virtual BOOL PreCreateWindow(CREATESTRUCT &cs) {
		return true;
	}
	void createDialogIndirect(LPCDLGTEMPLATE dlgTemplate);

	DECLARE_MESSAGE_MAP()

protected:
	Libs::Array<CWnd *> _ownedControls;
	CDC *_pDC = nullptr;
	HFONT _hFont;
	HBRUSH _hBrush;
	HPEN _hPen;
	HPALETTE _hPalette;

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) {
		return false;
	}
	virtual void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized) {
	}

	virtual LRESULT DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam) {
		return 0;
	}
	virtual void OnSetFont(CFont *pFont) {
	}

	BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID) {}
	afx_msg LRESULT OnActivateTopLevel(WPARAM, LPARAM) {
		return 0;
	}
	afx_msg void OnCancelMode() {}
	afx_msg void OnChildActivate() {}
	afx_msg void OnClose();
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos) {}
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct) {
		return 0;
	}

	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor) {
		return 0;
	}

	afx_msg void OnDestroy() {}
	afx_msg void OnEnable(BOOL bEnable) {}
	afx_msg void OnEndSession(BOOL bEnding) {}
	afx_msg void OnEnterIdle(UINT nWhy, CWnd *pWho) {}
	afx_msg BOOL OnEraseBkgnd(CDC *pDC) {
		return false;
	}
	afx_msg void OnIconEraseBkgnd(CDC *pDC) {}
	afx_msg void OnKillFocus(CWnd *pNewWnd) {}
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) {}
	afx_msg void OnMove(int x, int y) {}
	afx_msg void OnPaint() {}
	afx_msg void OnSyncPaint() {}
	afx_msg void OnParentNotify(UINT message, LPARAM lParam) {}
	afx_msg UINT OnNotifyFormat(CWnd *pWnd, UINT nCommand) {
		return 0;
	}
	afx_msg HCURSOR OnQueryDragIcon() {
		return 0;
	}
	afx_msg BOOL OnQueryEndSession() {
		return false;
	}
	afx_msg BOOL OnQueryNewPalette() {
		return false;
	}
	afx_msg BOOL OnQueryOpen() {
		return false;
	}
	afx_msg void OnSetFocus(CWnd *pOldWnd) {}
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus) {}
	afx_msg void OnSize(UINT nType, int cx, int cy) {}
	afx_msg void OnTCard(UINT idAction, DWORD dwActionData) {}
	afx_msg void OnSessionChange(UINT nSessionState, UINT nId) {}

	afx_msg void OnChangeUIState(UINT nAction, UINT nUIElement) {}
	afx_msg void OnUpdateUIState(UINT nAction, UINT nUIElement) {}
	afx_msg UINT OnQueryUIState() {
		return 0;
	}

	// Nonclient-Area message handler member functions
	afx_msg BOOL OnNcActivate(BOOL bActive) {
		return false;
	}
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct) {
		return false;
	}
	afx_msg void OnNcDestroy() {}
	afx_msg LRESULT OnNcHitTest(CPoint point) {
		return 0;
	}
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcMButtonDblClk(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcMButtonDown(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcMButtonUp(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcMouseHover(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcMouseLeave() {}
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcPaint() {}
	afx_msg void OnNcRButtonDblClk(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point) {}
	afx_msg void OnNcXButtonDown(short zHitTest, UINT nButton, CPoint point) {}
	afx_msg void OnNcXButtonUp(short zHitTest, UINT nButton, CPoint point) {}
	afx_msg void OnNcXButtonDblClk(short zHitTest, UINT nButton, CPoint point) {}

	// System message handler member functions
	afx_msg void OnDropFiles(HDROP hDropInfo) {}
	afx_msg void OnPaletteIsChanging(CWnd *pRealizeWnd) {}
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam) {}
	afx_msg void OnSysDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {}
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {}
	afx_msg BOOL OnAppCommand(CWnd *pWnd, UINT nCmd, UINT nDevice, UINT nKey) {
		return false;
	}
	afx_msg void OnRawInput(UINT nInputCode, HRAWINPUT hRawInput) {}
	afx_msg void OnCompacting(UINT nCpuTime) {}
	afx_msg void OnDevModeChange(_In_z_ LPTSTR lpDeviceName) {}
	afx_msg void OnFontChange() {}
	afx_msg void OnPaletteChanged(CWnd *pFocusWnd) {}
	afx_msg void OnSpoolerStatus(UINT nStatus, UINT nJobs) {}
	afx_msg void OnSysColorChange() {}
	afx_msg void OnTimeChange() {}
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection) {}
	afx_msg void OnWinIniChange(LPCTSTR lpszSection) {}
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM lEventData) {
		return 0;
	}
	afx_msg void OnUserChanged() {}
	afx_msg void OnInputLangChange(UINT nCharSet, UINT nLocaleId) {}
	afx_msg void OnInputLangChangeRequest(UINT nFlags, UINT nLocaleId) {}
	afx_msg void OnInputDeviceChange(unsigned short nFlags, HANDLE hDevice) {}

	// Input message handler member functions
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
	afx_msg void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
	afx_msg void OnUniChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {}
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {}
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {}
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {}
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2) {}
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point) {}
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point) {}
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point) {}
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point) {}
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point) {}
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point) {}
	afx_msg void OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point) {}
	afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point) {}
	afx_msg void OnXButtonUp(UINT nFlags, UINT nButton, CPoint point) {}
	afx_msg int OnMouseActivate(CWnd *pDesktopWnd, UINT nHitTest, UINT message) {
		return 0;
	}
	afx_msg void OnMouseHover(UINT nFlags, CPoint point) {}
	afx_msg void OnMouseLeave() {}
	afx_msg void OnMouseMove(UINT nFlags, CPoint point) {}
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt) {}
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
		return false;
	}
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point) {}
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point) {}
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point) {}
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent) {}

	// Control message handler member functions
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg UINT OnGetDlgCode() {
		return 0;
	}
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) {}
	afx_msg int OnCharToItem(UINT nChar, CListBox *pListBox, UINT nIndex) {
		return 0;
	}
	afx_msg int OnVKeyToItem(UINT nKey, CListBox *pListBox, UINT nIndex) {
		return 0;
	}
	afx_msg void OnSetFont(HFONT hFont, BOOL bRedraw);

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {}

protected:
	Common::HashMap<int, CWnd *> _children;
	int m_nFlags = 0;
	int _itemState = 0;
	int _controlId = 0;

protected:
	/**
	 * Returns true if the given message should
	 * be sent recursively to all child controls
	 * in addition to the current.
	 */
	static bool isRecursiveMessage(UINT message);

public:
	// For ScummVM the m_hWnd just points to the window itself,
	// so it'll be set up once in the constructor
	CWnd *const m_hWnd = nullptr;

	CWnd *m_pParentWnd = nullptr;
	int m_nClassStyle = 0;
	int m_nStyle = 0;
	int m_nModalResult = -1;
	bool _hasFocus = false;
	CString _windowText;
	Common::Rect _windowRect;
	Common::Rect _updateRect;
	Common::Rect _updatingRect;
	bool _updateErase = false;

public:
	/**
	 * Gets the handle for the wnd from the handle map
	 */
	static CWnd *FromHandlePermanent(HWND hWnd);

	/**
	 * Gets the handle for the wnd. For ScummVM,
	 * all window hWnd point to themselves. We just
	 * use the handle map as a convenient way to
	 * prevent messages being sent to destroyed windows.
	 * As such, internally FromHandle uses the
	 * FromHandlePermnanent function only
	 */
	static CWnd *FromHandle(HWND hWnd);

public:
	CWnd();
	~CWnd() override;

	BOOL Create(LPCSTR lpszClassName, LPCSTR lpszWindowName,
	    DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID,
	    CCreateContext *pContext = nullptr);
	BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		int x, int y, int nWidth, int nHeight,
		HWND hWndParent, HMENU nIDorHMenu, LPVOID lpParam = nullptr);
	BOOL CreateEx(DWORD dwExStyle, LPCSTR lpszClassName,
		LPCSTR lpszWindowName, DWORD dwStyle,
		const RECT &rect, CWnd *pParentWnd, UINT nID,
		LPVOID lpParam = nullptr);

	/**
	 * Gets a list of CWnd pointers for parent controls
	 * up to the window. It only allows a single dialog
	 * parent to be present, working around the issue
	 * when modal dialogs create other modal dialogs.
	 */
	Common::Array<const CWnd *> GetSafeParents(bool includeSelf = true) const;
	Common::Array<CWnd *> GetSafeParents(bool includeSelf = true);

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
	CWnd *GetFocus() const;
	BOOL IsWindowVisible() const {
		return (GetStyle() & WS_VISIBLE) != 0;
	}
	void DestroyWindow();
	void Invalidate(BOOL bErase = TRUE);
	int GetWindowText(CString &rString) const;
	int GetWindowText(LPSTR lpszStringBuf, int nMaxCount) const;
	BOOL SetWindowText(LPCSTR lpszString);
	UINT GetState() const;
	DWORD GetStyle() const {
		return m_nStyle;
	}
	void SetStyle(DWORD nStyle);
	bool IsWindowEnabled() const {
		return (_itemState & ODS_DISABLED) == 0;
	}
	bool IsActiveWindow() const;

	CDC *GetDC();
	int ReleaseDC(CDC *pDC);

	BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
	void SendMessageToDescendants(UINT message,
		WPARAM wParam, LPARAM lParam,
		bool bDeep = true, bool bOnlyPerm = false);
	static void SendMessageToDescendants(HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam, BOOL bDeep, BOOL bOnlyPerm);

	BOOL Validate();
	BOOL ValidateRect(LPCRECT lpRect = nullptr);
	BOOL InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE);
	void GetWindowRect(LPRECT lpRect) const;
	BOOL GetUpdateRect(LPRECT lpRect, BOOL bErase = FALSE);
	BOOL GetClientRect(LPRECT lpRect) const;
	bool PointInClientRect(const POINT &pt) const;
	void ClientToScreen(LPPOINT lpPoint) const;
	void ClientToScreen(LPRECT lpRect) const;
	void ScreenToClient(LPPOINT lpPoint) const;
	void ScreenToClient(LPRECT lpRect) const;

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
	    void (CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) = nullptr);
	BOOL KillTimer(UINT_PTR nIDEvent);

	BOOL GetScrollRange(int nBar,
	                    LPINT lpMinPos, LPINT lpMaxPos) const;
	INT GetScrollPosition() const;
	int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE);

	void SetCapture();
	void ReleaseCapture();

	const Common::HashMap<int, CWnd *> &getChildren() const {
		return _children;
	}
	bool IsWindowDirty() const {
		return !_updateRect.isEmpty();
	}

	HFONT GetFont() const {
		return _hFont;
	}
	void SetFont(CFont *pFont, BOOL bRedraw = true);

	void pause();
};

class CFrameWnd : public CWnd {
	DECLARE_DYNCREATE(CFrameWnd)

private:
	CString m_strTitle;
	UINT m_nIDHelp = 0;
	static const CRect rectDefault;
	CView *m_pViewActive = nullptr;

	void ActivateFrame(int nCmdShow);
	void OnUpdateFrameTitle(BOOL bAddToTitle);
	CWnd *CreateView(CCreateContext *pContext, UINT nID);

protected:
	BOOL PreCreateWindow(CREATESTRUCT &cCs) override {
		return true;
	}
	int OnCreate(LPCREATESTRUCT lpcs);
	int OnCreateHelper(LPCREATESTRUCT lpcs, CCreateContext *pContext);
	BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext *pContext);

	enum RepositionFlags {
		reposDefault = 0, reposQuery = 1, reposExtra = 2, reposNoPosLeftOver = 0x8000
	};
	BOOL RepositionBars(
	    UINT nIDFirst,
	    UINT nIDLast,
	    UINT nIDLeftOver,
	    UINT nFlag = reposDefault,
	    LPRECT lpRectParam = nullptr,
	    LPCRECT lpRectClient = nullptr,
	    BOOL bStretch = TRUE
	);

	DECLARE_MESSAGE_MAP()

public:
	~CFrameWnd() override {
	}

	BOOL Create(LPCSTR lpszClassName,
		LPCSTR lpszWindowName,
		DWORD dwStyle = WS_OVERLAPPEDWINDOW,
		const RECT &rect = rectDefault,
		CWnd *pParentWnd = nullptr,
		LPCSTR lpszMenuName = nullptr,
		DWORD dwExStyle = 0,
		CCreateContext *pContext = nullptr);

	HMENU GetMenu() const;
	void RecalcLayout(BOOL bNotify = TRUE);
	void InitialUpdateFrame(CDocument *pDoc, BOOL bMakeVisible);

	BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
		CWnd *pParentWnd, CCreateContext *pContext);

	CView *GetActiveView() const;
	void SetActiveView(CView *pViewNew, BOOL bNotify = TRUE);
	void OnSetFocus(CWnd *pOldWnd);
	CDocument *GetActiveDocument();
};

class CDialog : public CWnd {
	DECLARE_DYNAMIC(CDialog)
	friend class CDialogTemplate;

private:
	LPCSTR m_lpszTemplateName = nullptr;
	UINT m_nIDHelp = 0;
	LPCDLGTEMPLATE m_lpDialogTemplate = nullptr;
	HGLOBAL m_hDialogTemplate = 0;
	int _defaultId = 0;
	CFont _dialogFont;

	void SetParent(CWnd *wnd);
	BOOL CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd *pParentWnd,
		void *lpDialogInit, HINSTANCE hInst);
	BOOL CreateIndirect(HGLOBAL hDialogTemplate, CWnd *pParentWnd,
		HINSTANCE hInst);
	BOOL CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate,
		CWnd *pParentWnd, HINSTANCE hInst);

protected:
	DECLARE_MESSAGE_MAP()

	virtual void OnOK();
	virtual void OnCancel();
	virtual void PostModal() {
	}

	void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);

public:
	CDialog() {}
	explicit CDialog(LPCSTR lpszTemplateName,
	                 CWnd *pParentWnd = nullptr);
	explicit CDialog(UINT nIDTemplate,
	                 CWnd *pParentWnd = nullptr);
	~CDialog() override {}
	BOOL Create(LPCSTR lpszTemplateName,
	            CWnd *pParentWnd = nullptr);
	BOOL Create(UINT nIDTemplate,
	            CWnd *pParentWnd = nullptr);

	int DoModal();
	virtual void PreInitDialog() {}
	virtual BOOL OnInitDialog() {
		return true;
	}

	DWORD GetDefID() const;
	void SetDefID(UINT nID);
	LRESULT HandleInitDialog(WPARAM, LPARAM);
	LRESULT HandleSetFont(WPARAM, LPARAM);
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

	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);
};

class CStatic : public CWnd {
	DECLARE_DYNAMIC(CStatic)

protected:
	DECLARE_MESSAGE_MAP()

public:
	~CStatic() override {
	}
};

class CButton : public CWnd {
	DECLARE_DYNAMIC(CButton)

private:
	bool _pressed = false;

	void OnPushButtonPaint();
	void OnPushBoxPaint();
	void OnOwnerDrawPaint();

protected:
	DECLARE_MESSAGE_MAP()

	void OnPaint();
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	LRESULT OnBnClicked(WPARAM wParam, LPARAM lParam);
	LRESULT OnBnSetCheck(WPARAM wParam, LPARAM lParam);

public:
	~CButton() override {
	}
	CVIRTUAL BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle,
	                     const RECT &rect, CWnd *pParentWnd, UINT nID);

	int GetCheck() const;
	void SetCheck(int nCheck);
	void SetButtonStyle(UINT nStyle, BOOL bRedraw = TRUE);
	UINT GetButtonStyle() const;
};

class CListBox : public CWnd {
	DECLARE_DYNAMIC(CListBox)

protected:
	DECLARE_MESSAGE_MAP()

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

private:
	size_t _maxLength = 0;
	bool _cursorVisible = false;

protected:
	DECLARE_MESSAGE_MAP()

public:
	~CEdit() override {
	}
	CVIRTUAL BOOL Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID);
	void LimitText(int nChars);
	void SetSel(int nStartChar, int nEndChar, BOOL bNoScroll = FALSE);

	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnKillFocus(CWnd *pNewWnd);
	afx_msg void OnTimer(UINT_PTR nTimerId);
};

class CScrollBar : public CWnd {
	DECLARE_DYNAMIC(CScrollBar)

private:
	int _minValue = 0;
	int _maxValue = 100;
	int _pageSize = 10;
	int _value = 0;

	void drawSquare(CPaintDC &dc, const CRect &r);
	void drawArrow(CPaintDC &dc, const CRect &r, bool leftArrow);
	CRect getThumbRect() const;
	int getIndexFromX(int xp) const;
	void scrollEvent(int action);

protected:
	DECLARE_MESSAGE_MAP()
	void OnPaint();
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMouseMove(UINT, CPoint point);

public:
	~CScrollBar() override { }
	CVIRTUAL BOOL Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID);

	int GetScrollPos() const;
	int SetScrollPos(int nPos, BOOL bRedraw = true);
	void GetScrollRange(LPINT lpMinPos, LPINT lpMaxPos) const;
	void SetScrollRange(int nMinPos, int nMaxPos, BOOL bRedraw);
	void ShowScrollBar(BOOL bShow);
	BOOL SetScrollInfo(LPSCROLLINFO lpScrollInfo, BOOL bRedraw);
};

class CView : public CWnd {
	DECLARE_DYNAMIC(CView)

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT &cCs) override;
	virtual void OnDraw(CDC *pDC) {}
	void OnPaint();

	DECLARE_MESSAGE_MAP()

public:
	CDocument *m_pDocument = nullptr;

public:
	~CView() override {
	}

	int OnCreate(LPCREATESTRUCT lpcs);
	void OnDestroy();
	void PostNcDestroy();

	void OnInitialUpdate();
	void OnUpdate(CView *pSender, LPARAM /*lHint*/, CObject * /*pHint*/);
	virtual void OnActivateView(BOOL, CView *, CView *) {}
	virtual void OnActivateFrame(UINT nState, CFrameWnd *pFrameWnd) {}

	CFrameWnd *GetParentFrame() const {
		return dynamic_cast<CFrameWnd *>(m_pParentWnd);
	}
	CDocument *GetDocument() const {
		return m_pDocument;
	}
};

class CScrollView : public CView {
	DECLARE_DYNAMIC(CScrollView)

private:
	static const SIZE sizeNull;

protected:
	DECLARE_MESSAGE_MAP()

public:
	~CScrollView() override {
	}

	void SetScrollSizes(
	    int nMapMode,
	    SIZE sizeTotal,
	    const SIZE &sizePage = sizeNull,
	    const SIZE &sizeLine = sizeNull
	);
};

/*============================================================================*/
// class CDocTemplate creates documents

class CDocTemplate : public CCmdTarget {
	DECLARE_DYNAMIC(CDocTemplate)
private:
	UINT m_nIDResource = 0;
	const CRuntimeClass *m_pDocClass = nullptr;
	const CRuntimeClass *m_pFrameClass = nullptr;
	const CRuntimeClass *m_pViewClass = nullptr;

protected:
	DECLARE_MESSAGE_MAP()

	CDocTemplate(UINT nIDResource, const CRuntimeClass *pDocClass,
	    const CRuntimeClass *pFrameClass, const CRuntimeClass *pViewClass);

public:
	/**
	 * '\n' separated names
	 * The document names sub-strings are represented as _one_ string:
	 * windowTitle\ndocName\n ... (see DocStringIndex enum)
	 */
	CString m_strDocStrings;

public:
	/**
	 * Open named file
	 * @param lpszPathName	Path name. If null,
	 * then create new file with this type
	 */
	virtual CDocument *OpenDocumentFile(LPCSTR lpszPathName,
		BOOL bMakeVisible = TRUE) = 0;
	virtual CDocument *CreateNewDocument();
	virtual CFrameWnd *CreateNewFrame(CDocument *pDoc, CFrameWnd *pOther);
	virtual void SetDefaultTitle(CDocument *pDocument) = 0;
	virtual void InitialUpdateFrame(CFrameWnd *pFrame, CDocument *pDoc,
		BOOL bMakeVisible = TRUE);

	virtual void AddDocument(CDocument *pDoc);      // must override
	virtual void RemoveDocument(CDocument *pDoc);   // must override

	void LoadTemplate();
};

class CSingleDocTemplate : public CDocTemplate {
	DECLARE_DYNAMIC(CSingleDocTemplate)
private:
	CDocument *m_pOnlyDoc = nullptr;

public:
	DECLARE_MESSAGE_MAP()

public:
	CSingleDocTemplate(
	    UINT nIDResource,
	    const CRuntimeClass *pDocClass,
	    const CRuntimeClass *pFrameClass,
	    const CRuntimeClass *pViewClass
	) : CDocTemplate(nIDResource, pDocClass,
		                 pFrameClass, pViewClass) {
	}
	~CSingleDocTemplate();

	/**
	 * Open named file
	 * @param lpszPathName	Path name. If null,
	 * then create new file with this type
	 */
	CDocument *OpenDocumentFile(LPCSTR lpszPathName,
		BOOL bMakeVisible = TRUE) override;
	void AddDocument(CDocument *pDoc) override;

	void SetDefaultTitle(CDocument *pDocument) override;
};

typedef Libs::List<CDocTemplate *> CTemplateList;

class CDocManager {
private:
	CTemplateList pStaticList;
	bool bStaticInit = false;

protected:
	CTemplateList m_templateList;

public:
	void AddDocTemplate(CDocTemplate *pTemplate);
	void OnFileNew();
	void OnFileOpen();
};

/*============================================================================*/

template<class T>class CHandleMap;

class CWinThread : public CCmdTarget {
	DECLARE_DYNAMIC(CWinThread)
public:
	~CWinThread() override {
	}
};

class CWinApp : public CWinThread, public Libs::EventLoop {
	DECLARE_DYNAMIC(CWinApp)

private:
	CWinApp *_priorWinApp;
	Libs::Resources _resources;
	Libs::Settings _settings;
	Gfx::Cursors _cursors;
	Gfx::Fonts _fonts;
	Graphics::Screen _screen;
	int m_nWaitCursorCount = 0;
	HCURSOR m_hcurWaitCursorRestore = nullptr;
	HCURSOR _currentCursor = nullptr;
	Common::FSNode _currentDirectory;
	CHandleMap<CDC> *m_pmapHDC = nullptr;
	CHandleMap<CGdiObject> *m_pmapHGDIOBJ = nullptr;
	CHandleMap<CWnd> *m_pmapWnd = nullptr;

protected:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

public:
	static CWinApp *_activeApp;
	int m_nCmdShow = SW_SHOWNORMAL;
	const char *m_lpCmdLine = "";
	CWnd *m_pMainWnd = nullptr;
	CPalette _palette;
	CFont _defaultFont;
	CPen _defaultPen;
	CBrush _defaultBrush;
	MSG _currentMessage;
	CDocManager *m_pDocManager = nullptr;

public:
	CWinApp(const char *appName = nullptr);
	~CWinApp() override;

	/**
	 * Main execution for MFC applications
	 */
	int Run();

	virtual BOOL InitApplication();
	virtual BOOL SaveAllModified();

	void SetDialogBkColor();
	HCURSOR LoadStandardCursor(LPCSTR lpszCursorName);
	HCURSOR LoadCursor(LPCSTR lpszResourceName);
	HCURSOR LoadCursor(UINT nIDResource);
	HCURSOR SetCursor(HCURSOR hCursor);
	void BeginWaitCursor();
	void EndWaitCursor();

	/**
	 * Adds or remove a wait mouse cursor
	 * @param nCode     0 => restore, 1=> begin, -1=> end
	 */
	void DoWaitCursor(int nCode) override;

	void AddDocTemplate(CDocTemplate *pTemplate);
	void CloseAllDocuments(BOOL bEndSession);

	UINT GetProfileInt(LPCSTR lpszSection,
	    LPCSTR lpszEntry, int nDefault);
	void WriteProfileInt(LPCSTR lpszSection,
	    LPCSTR lpszEntry, int nValue);
	CString GetProfileString(LPCSTR lpszSection,
		LPCSTR lpszEntry, LPCSTR lpszDefault = nullptr) ;
	BOOL WriteProfileString(LPCSTR lpszSection,
		LPCSTR lpszEntry, LPCSTR lpszValue);


	virtual void OnFileNew();
	virtual void OnFileOpen();
	virtual void OnFilePrintSetup() {}

	CHandleMap<CGdiObject> *afxMapHGDIOBJ(BOOL bCreate = false);
	CHandleMap<CDC> *afxMapHDC(BOOL bCreate = false);
	CHandleMap<CWnd> *afxMapWnd(BOOL bCreate = false);
	void AfxUnlockTempMaps();

	/*== ScummVM added functions ==*/
	void setDirectory(const char *folder);
	void setPalette(const Graphics::Palette &pal);
	byte getColor(COLORREF color) const;
	Graphics::Screen *getScreen() {
		return &_screen;
	}

	// resource functions
	/**
	 * Adds a Windows file containing resources
	 */
	void addResources(const Common::Path &file) {
		_resources.addResources(file);
	}
	void removeResources(const Common::Path &file) {
		_resources.removeResources(file);
	}
	HRSRC findResource(LPCSTR lpName, LPCSTR lpType);
	size_t sizeofResource(HRSRC hResInfo);
	HGLOBAL loadResource(HRSRC hResInfo);
	LPVOID lockResource(HGLOBAL hResData);
	void unlockResource(HGLOBAL hResData);
	BOOL freeResource(HGLOBAL hResData);
	const Libs::Resources &getResources() const {
		return _resources;
	}

	// Fonts
	int addFontResource(LPCSTR fontName) {
		return _fonts.addFontResource(fontName);
	}
	bool removeFontResource(LPCSTR fontName) {
		return _fonts.removeFontResource(fontName);
	}
	HFONT createFont(int nHeight, int nWidth, int nEscapement,
			int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline,
			BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision,
			BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily,
			LPCSTR lpszFacename) {
		return _fonts.createFont(nHeight, nWidth,
			nEscapement, nOrientation, nWeight, bItalic,
			bUnderline, cStrikeOut, nCharSet, nOutPrecision,
			nClipPrecision, nQuality, nPitchAndFamily,
			lpszFacename);
	}
	HFONT getFont(const char *lpszFacename, int nHeight) {
		return _fonts.getFont(lpszFacename, nHeight);
	}
	HFONT getDefaultFont() {
		assert(_defaultFont._font);
		return (HFONT)_defaultFont._font;
	}
	HPEN getDefaultPen() const {
		assert(_defaultPen.m_hObject);
		return (HPEN)_defaultPen.m_hObject;
	}
	HBRUSH getDefaultBrush() {
		assert(_defaultBrush.m_hObject);
		return (HBRUSH)_defaultBrush.m_hObject;
	}
	HPALETTE getSystemPalette() {
		assert(_palette._palette);
		return _palette._palette;
	}

	LPCSTR AfxRegisterWndClass(UINT nClassStyle,
		HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon);
	BOOL GetClassInfo(HINSTANCE hInstance,
		LPCSTR   lpClassName, LPWNDCLASS lpWndClass);
};

extern CWinApp *AfxGetApp();
HINSTANCE AfxGetInstanceHandle();

} // namespace MFC
} // namespace Bagel

#endif
