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
#include "bagel/mfc/gfx/surface.h"
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

constexpr uint32 DEFAULT_MODAL_RESULT = MKTAG('S', 'C', 'V', 'M');

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

	bool CreateMenu();
	bool CreatePopupMenu();
	bool LoadMenu(const char *lpszResourceName);
	bool LoadMenu(unsigned int nIDResource);
	bool LoadMenuIndirect(const void *lpMenuTemplate);
	bool DestroyMenu();

	static CMenu *FromHandle(HMENU hMenu);

	unsigned int CheckMenuItem(unsigned int nIDCheckItem, unsigned int nCheck);
};

/*============================================================================*/
// Window message map handling


enum AfxSig {
	AfxSig_end = 0,     // [marks end of message map]

	AfxSig_b_D_v,               // bool (CDC*)
	AfxSig_b_b_v,               // bool (bool)
	AfxSig_b_u_v,               // bool (unsigned int)
	AfxSig_b_h_v,               // bool (HANDLE)
	AfxSig_b_W_uu,              // bool (CWnd*, unsigned int, unsigned int)
	AfxSig_b_W_COPYDATASTRUCT,              // bool (CWnd*, COPYDATASTRUCT*)
	AfxSig_b_v_HELPINFO,        // bool (LPHELPINFO);
	AfxSig_CTLCOLOR,            // HBRUSH (CDC*, CWnd*, unsigned int)
	AfxSig_CTLCOLOR_REFLECT,    // HBRUSH (CDC*, unsigned int)
	AfxSig_i_u_W_u,             // int (unsigned int, CWnd*, unsigned int)  // ?TOITEM
	AfxSig_i_uu_v,              // int (unsigned int, unsigned int)
	AfxSig_i_W_uu,              // int (CWnd*, unsigned int, unsigned int)
	AfxSig_i_v_s,               // int (uint16 *)
	AfxSig_l_w_l,               // LRESULT (WPARAM, LPARAM)
	AfxSig_l_uu_M,              // LRESULT (unsigned int, unsigned int, CMenu*)
	AfxSig_v_b_h,               // void (bool, HANDLE)
	AfxSig_v_h_v,               // void (HANDLE)
	AfxSig_v_h_h,               // void (HANDLE, HANDLE)
	AfxSig_v_v_v,               // void ()
	AfxSig_v_u_v,               // void (unsigned int)
	AfxSig_v_up_v,              // void (uintptr)
	AfxSig_v_u_u,               // void (unsigned int, unsigned int)
	AfxSig_v_uu_v,              // void (unsigned int, unsigned int)
	AfxSig_v_v_ii,              // void (int, int)
	AfxSig_v_u_uu,              // void (unsigned int, unsigned int, unsigned int)
	AfxSig_v_u_ii,              // void (unsigned int, int, int)
	AfxSig_v_u_W,               // void (unsigned int, CWnd*)
	AfxSig_i_u_v,               // int (unsigned int)
	AfxSig_u_u_v,               // unsigned int (unsigned int)
	AfxSig_b_v_v,               // bool ()
	AfxSig_v_w_l,               // void (WPARAM, LPARAM)
	AfxSig_MDIACTIVATE,         // void (bool, CWnd*, CWnd*)
	AfxSig_v_D_v,               // void (CDC*)
	AfxSig_v_M_v,               // void (CMenu*)
	AfxSig_v_M_ub,              // void (CMenu*, unsigned int, bool)
	AfxSig_v_W_v,               // void (CWnd*)
	AfxSig_v_v_W,               // void (CWnd*)
	AfxSig_v_W_uu,              // void (CWnd*, unsigned int, unsigned int)
	AfxSig_v_W_p,               // void (CWnd*, CPoint)
	AfxSig_v_W_h,               // void (CWnd*, HANDLE)
	AfxSig_C_v_v,               // HCURSOR ()
	AfxSig_ACTIVATE,            // void (unsigned int, CWnd*, bool)
	AfxSig_SCROLL,              // void (unsigned int, unsigned int, CWnd*)
	AfxSig_SCROLL_REFLECT,      // void (unsigned int, unsigned int)
	AfxSig_v_v_s,               // void (uint16 *)
	AfxSig_v_u_cs,              // void (unsigned int, const char *)
	AfxSig_OWNERDRAW,           // void (int, uint16 *) force return true
	AfxSig_i_i_s,               // int (int, uint16 *)
	AfxSig_u_v_p,               // unsigned int (CPoint)
	AfxSig_u_v_v,               // unsigned int ()
	AfxSig_v_b_NCCALCSIZEPARAMS,        // void (bool, NCCALCSIZE_PARAMS*)
	AfxSig_v_v_WINDOWPOS,           // void (WINDOWPOS*)
	AfxSig_v_uu_M,              // void (unsigned int, unsigned int, HMENU)
	AfxSig_v_u_p,               // void (unsigned int, CPoint)
	AfxSig_SIZING,              // void (unsigned int, LPRECT)
	AfxSig_MOUSEWHEEL,          // bool (unsigned int, short, CPoint)
	AfxSig_MOUSEHWHEEL,         // void (unsigned int, short, CPoint)
	AfxSigCmd_v,                // void ()
	AfxSigCmd_b,                // bool ()
	AfxSigCmd_RANGE,            // void (unsigned int)
	AfxSigCmd_EX,               // bool (unsigned int)
	AfxSigNotify_v,             // void (NMHDR*, LRESULT*)
	AfxSigNotify_b,             // bool (NMHDR*, LRESULT*)
	AfxSigNotify_RANGE,         // void (unsigned int, NMHDR*, LRESULT*)
	AfxSigNotify_EX,            // bool (unsigned int, NMHDR*, LRESULT*)
	AfxSigCmdUI,                // void (CCmdUI*)
	AfxSigCmdUI_RANGE,          // void (CCmdUI*, unsigned int)
	AfxSigCmd_v_pv,             // void (void*)
	AfxSigCmd_b_pv,             // bool (void*)
	AfxSig_l,               // LRESULT ()
	AfxSig_l_p,             // LRESULT (CPOINT)
	AfxSig_u_W_u,               // unsigned int (CWnd*, unsigned int)
	AfxSig_v_u_M,               // void (unsigned int, CMenu* )
	AfxSig_u_u_M,               // unsigned int (unsigned int, CMenu* )
	AfxSig_u_v_MENUGETOBJECTINFO,       // unsigned int (MENUGETOBJECTINFO*)
	AfxSig_v_M_u,               // void (CMenu*, unsigned int)
	AfxSig_v_u_LPMDINEXTMENU,       // void (unsigned int, LPMDINEXTMENU)
	AfxSig_APPCOMMAND,          // void (CWnd*, unsigned int, unsigned int, unsigned int)
	AfxSig_RAWINPUT,            // void (unsigned int, HRAWINPUT)
	AfxSig_u_u_u,               // unsigned int (unsigned int, unsigned int)
	AfxSig_MOUSE_XBUTTON,           // void (unsigned int, unsigned int, CPoint)
	AfxSig_MOUSE_NCXBUTTON,         // void (short, unsigned int, CPoint)
	AfxSig_INPUTLANGCHANGE,         // void (unsigned int, unsigned int)
	AfxSig_v_u_hkl,                 // void (unsigned int, HKL)
	AfxSig_INPUTDEVICECHANGE,       // void (unsigned short, HANDLE)
	AfxSig_l_D_u,                   // LRESULT (CDC*, unsigned int)
	AfxSig_i_v_S,               // int (const char *)
	AfxSig_vwpb,                // void (HFONT, bool)
	AfxSig_h_v_v,               // HANDLE ()
	AfxSig_h_b_h,               // HANDLE (bool, HANDLE)
	AfxSig_b_v_ii,              // bool (int, int)
	AfxSig_h_h_h,               // HANDLE (HANDLE, HANDLE)
	AfxSig_MDINext,             // void (CWnd*, bool)
	AfxSig_u_u_l,               // unsigned int (unsigned int, LPARAM)

	// Old
	AfxSig_bD = AfxSig_b_D_v,      // bool (CDC*)
	AfxSig_bb = AfxSig_b_b_v,      // bool (bool)
	AfxSig_bWww = AfxSig_b_W_uu,    // bool (CWnd*, unsigned int, unsigned int)
	AfxSig_hDWw = AfxSig_CTLCOLOR,    // HBRUSH (CDC*, CWnd*, unsigned int)
	AfxSig_hDw = AfxSig_CTLCOLOR_REFLECT,     // HBRUSH (CDC*, unsigned int)
	AfxSig_iwWw = AfxSig_i_u_W_u,    // int (unsigned int, CWnd*, unsigned int)
	AfxSig_iww = AfxSig_i_uu_v,     // int (unsigned int, unsigned int)
	AfxSig_iWww = AfxSig_i_W_uu,    // int (CWnd*, unsigned int, unsigned int)
	AfxSig_is = AfxSig_i_v_s,      // int (uint16 *)
	AfxSig_lwl = AfxSig_l_w_l,     // LRESULT (WPARAM, LPARAM)
	AfxSig_lwwM = AfxSig_l_uu_M,    // LRESULT (unsigned int, unsigned int, CMenu*)
	AfxSig_vv = AfxSig_v_v_v,      // void ()

	AfxSig_vw = AfxSig_v_u_v,      // void (unsigned int)
	AfxSig_vww = AfxSig_v_u_u,     // void (unsigned int, unsigned int)
	AfxSig_vww2 = AfxSig_v_uu_v,    // void (unsigned int, unsigned int) // both come from wParam
	AfxSig_vvii = AfxSig_v_v_ii,    // void (int, int) // wParam is ignored
	AfxSig_vwww = AfxSig_v_u_uu,    // void (unsigned int, unsigned int, unsigned int)
	AfxSig_vwii = AfxSig_v_u_ii,    // void (unsigned int, int, int)
	AfxSig_vwl = AfxSig_v_w_l,     // void (unsigned int, LPARAM)
	AfxSig_vbWW = AfxSig_MDIACTIVATE,    // void (bool, CWnd*, CWnd*)
	AfxSig_vD = AfxSig_v_D_v,      // void (CDC*)
	AfxSig_vM = AfxSig_v_M_v,      // void (CMenu*)
	AfxSig_vMwb = AfxSig_v_M_ub,    // void (CMenu*, unsigned int, bool)

	AfxSig_vW = AfxSig_v_W_v,      // void (CWnd*)
	AfxSig_vWww = AfxSig_v_W_uu,    // void (CWnd*, unsigned int, unsigned int)
	AfxSig_vWp = AfxSig_v_W_p,     // void (CWnd*, CPoint)
	AfxSig_vWh = AfxSig_v_W_h,     // void (CWnd*, HANDLE)
	AfxSig_vwW = AfxSig_v_u_W,     // void (unsigned int, CWnd*)
	AfxSig_vwWb = AfxSig_ACTIVATE,    // void (unsigned int, CWnd*, bool)
	AfxSig_vwwW = AfxSig_SCROLL,    // void (unsigned int, unsigned int, CWnd*)
	AfxSig_vwwx = AfxSig_SCROLL_REFLECT,    // void (unsigned int, unsigned int)
	AfxSig_vs = AfxSig_v_v_s,      // void (uint16 *)
	AfxSig_vOWNER = AfxSig_OWNERDRAW,  // void (int, uint16 *), force return true
	AfxSig_iis = AfxSig_i_i_s,     // int (int, uint16 *)
	AfxSig_wp = AfxSig_u_v_p,      // unsigned int (CPoint)
	AfxSig_wv = AfxSig_u_v_v,      // unsigned int ()
	AfxSig_vPOS = AfxSig_v_v_WINDOWPOS,    // void (WINDOWPOS*)
	AfxSig_vCALC = AfxSig_v_b_NCCALCSIZEPARAMS,   // void (bool, NCCALCSIZE_PARAMS*)
	AfxSig_vNMHDRpl = AfxSigNotify_v,    // void (NMHDR*, LRESULT*)
	AfxSig_bNMHDRpl = AfxSigNotify_b,    // bool (NMHDR*, LRESULT*)
	AfxSig_vwNMHDRpl = AfxSigNotify_RANGE,   // void (unsigned int, NMHDR*, LRESULT*)
	AfxSig_bwNMHDRpl = AfxSigNotify_EX,   // bool (unsigned int, NMHDR*, LRESULT*)
	AfxSig_bHELPINFO = AfxSig_b_v_HELPINFO,   // bool (HELPINFO*)
	AfxSig_vwSIZING = AfxSig_SIZING,    // void (unsigned int, LPRECT) -- return true

	// signatures specific to CCmdTarget
	AfxSig_cmdui = AfxSigCmdUI,   // void (CCmdUI*)
	AfxSig_cmduiw = AfxSigCmdUI_RANGE,  // void (CCmdUI*, unsigned int)
	AfxSig_vpv = AfxSigCmd_v_pv,     // void (void*)
	AfxSig_bpv = AfxSigCmd_b_pv,     // bool (void*)

	// Other aliases (based on implementation)
	AfxSig_vwwh = AfxSig_v_uu_M,                // void (unsigned int, unsigned int, HMENU)
	AfxSig_vwp = AfxSig_v_u_p,                 // void (unsigned int, CPoint)
	AfxSig_bw = AfxSig_b_u_v,      // bool (unsigned int)
	AfxSig_bh = AfxSig_b_h_v,      // bool (HANDLE)
	AfxSig_iw = AfxSig_i_u_v,      // int (unsigned int)
	AfxSig_ww = AfxSig_u_u_v,      // unsigned int (unsigned int)
	AfxSig_bv = AfxSig_b_v_v,      // bool ()
	AfxSig_hv = AfxSig_C_v_v,      // HANDLE ()
	AfxSig_vb = AfxSig_vw,      // void (bool)
	AfxSig_vbh = AfxSig_v_b_h,    // void (bool, HANDLE)
	AfxSig_vbw = AfxSig_vww,    // void (bool, unsigned int)
	AfxSig_vhh = AfxSig_v_h_h,    // void (HANDLE, HANDLE)
	AfxSig_vh = AfxSig_v_h_v,      // void (HANDLE)
	AfxSig_viSS = AfxSig_vwl,   // void (int, STYLESTRUCT*)
	AfxSig_bwl = AfxSig_lwl,
	AfxSig_vwMOVING = AfxSig_vwSIZING,  // void (unsigned int, LPRECT) -- return true

	AfxSig_vW2 = AfxSig_v_v_W,                 // void (CWnd*) (CWnd* comes from lParam)
	AfxSig_bWCDS = AfxSig_b_W_COPYDATASTRUCT,               // bool (CWnd*, COPYDATASTRUCT*)
	AfxSig_bwsp = AfxSig_MOUSEWHEEL,                // bool (unsigned int, short, CPoint)
	AfxSig_vws = AfxSig_v_u_cs,
};


#define CN_COMMAND              0               // void ()
#define CN_UPDATE_COMMAND_UI    ((unsigned int)(-1))    // void (CCmdUI*)
#define CN_EVENT                ((unsigned int)(-2))    // OLE event
#define CN_OLECOMMAND           ((unsigned int)(-3))    // OLE document command
#define CN_OLE_UNREGISTER       ((unsigned int)(-4))    // OLE unregister
// > 0 are control notifications
// < 0 are for MFC's use

/*
 * PeekMessage() Options
 */
#define PM_NOREMOVE         0x0000
#define PM_REMOVE           0x0001
#define PM_NOYIELD          0x0002

typedef void (CCmdTarget::*AFX_PMSG)();
typedef void (CWnd::*AFX_PMSGW)();

#pragma warning(disable: 4121)
struct AFX_MSGMAP_ENTRY {
	unsigned int nMessage;   // windows message
	unsigned int nCode;      // control code or WM_NOTIFY code
	unsigned int nID;        // control ID (or 0 for windows messages)
	unsigned int nLastID;    // used for entries specifying a range of control id's
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
typedef void (AFX_MSG_CALL CCmdTarget:: *AFX_PMSG)();

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
			{ ThisClass *dummy; (void)dummy; }              \
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
	bool(AFX_MSG_CALL CCmdTarget:: *pfn_bCOMMAND)();
	void (AFX_MSG_CALL CCmdTarget:: *pfn_COMMAND_RANGE)(unsigned int);
	bool(AFX_MSG_CALL CCmdTarget:: *pfn_COMMAND_EX)(unsigned int);

	void (AFX_MSG_CALL CCmdTarget:: *pfn_UPDATE_COMMAND_UI)(CCmdUI *);
	void (AFX_MSG_CALL CCmdTarget:: *pfn_UPDATE_COMMAND_UI_RANGE)(CCmdUI *, unsigned int);
	void (AFX_MSG_CALL CCmdTarget:: *pfn_OTHER)(void *);
	bool(AFX_MSG_CALL CCmdTarget:: *pfn_OTHER_EX)(void *);

	void (AFX_MSG_CALL CCmdTarget:: *pfn_NOTIFY)(NMHDR *, LRESULT *);
	bool(AFX_MSG_CALL CCmdTarget:: *pfn_bNOTIFY)(NMHDR *, LRESULT *);
	void (AFX_MSG_CALL CCmdTarget:: *pfn_NOTIFY_RANGE)(unsigned int, NMHDR *, LRESULT *);
	bool(AFX_MSG_CALL CCmdTarget:: *pfn_NOTIFY_EX)(unsigned int, NMHDR *, LRESULT *);

	// Type safe variant for thread messages

	void (AFX_MSG_CALL CWinThread:: *pfn_THREAD)(WPARAM, LPARAM);

	// Specific type safe variants for WM-style messages
	bool(AFX_MSG_CALL CWnd:: *pfn_bD)(CDC *);
	bool(AFX_MSG_CALL CWnd:: *pfn_bb)(bool);
	bool(AFX_MSG_CALL CWnd:: *pfn_bWww)(CWnd *, unsigned int, unsigned int);
	bool(AFX_MSG_CALL CWnd:: *pfn_bHELPINFO)(HELPINFO *);
	bool(AFX_MSG_CALL CWnd:: *pfn_bWCDS)(CWnd *, COPYDATASTRUCT *);
	HBRUSH(AFX_MSG_CALL CWnd:: *pfn_hDWw)(CDC *, CWnd *, unsigned int);
	HBRUSH(AFX_MSG_CALL CWnd:: *pfn_hDw)(CDC *, unsigned int);
	int (AFX_MSG_CALL CWnd:: *pfn_iwWw)(unsigned int, CWnd *, unsigned int);
	int (AFX_MSG_CALL CWnd:: *pfn_iww)(unsigned int, unsigned int);
	int (AFX_MSG_CALL CWnd:: *pfn_iWww)(CWnd *, unsigned int, unsigned int);
	int (AFX_MSG_CALL CWnd:: *pfn_is)(uint16 *);
	LRESULT(AFX_MSG_CALL CWnd:: *pfn_lwl)(WPARAM, LPARAM);
	LRESULT(AFX_MSG_CALL CWnd:: *pfn_lwwM)(unsigned int, unsigned int, CMenu *);
	void (AFX_MSG_CALL CWnd:: *pfn_vv)();

	void (AFX_MSG_CALL CWnd:: *pfn_vw)(unsigned int);
	void (AFX_MSG_CALL CWnd:: *pfn_vww)(unsigned int, unsigned int);
	void (AFX_MSG_CALL CWnd:: *pfn_vvii)(int, int);
	void (AFX_MSG_CALL CWnd:: *pfn_vwww)(unsigned int, unsigned int, unsigned int);
	void (AFX_MSG_CALL CWnd:: *pfn_vwii)(unsigned int, int, int);
	void (AFX_MSG_CALL CWnd:: *pfn_vwl)(WPARAM, LPARAM);
	void (AFX_MSG_CALL CWnd:: *pfn_vbWW)(bool, CWnd *, CWnd *);
	void (AFX_MSG_CALL CWnd:: *pfn_vD)(CDC *);
	void (AFX_MSG_CALL CWnd:: *pfn_vM)(CMenu *);
	void (AFX_MSG_CALL CWnd:: *pfn_vMwb)(CMenu *, unsigned int, bool);

	void (AFX_MSG_CALL CWnd:: *pfn_vW)(CWnd *);
	void (AFX_MSG_CALL CWnd:: *pfn_vWww)(CWnd *, unsigned int, unsigned int);
	void (AFX_MSG_CALL CWnd:: *pfn_vWp)(CWnd *, CPoint);
	void (AFX_MSG_CALL CWnd:: *pfn_vWh)(CWnd *, HANDLE);
	void (AFX_MSG_CALL CWnd:: *pfn_vwW)(unsigned int, CWnd *);
	void (AFX_MSG_CALL CWnd:: *pfn_vwWb)(unsigned int, CWnd *, bool);
	void (AFX_MSG_CALL CWnd:: *pfn_vwwW)(unsigned int, unsigned int, CWnd *);
	void (AFX_MSG_CALL CWnd:: *pfn_vwwx)(unsigned int, unsigned int);
	void (AFX_MSG_CALL CWnd:: *pfn_vs)(uint16 *);
	void (AFX_MSG_CALL CWnd:: *pfn_vOWNER)(int, uint16 *);      // force return true
	int (AFX_MSG_CALL CWnd:: *pfn_iis)(int, uint16 *);
	unsigned int(AFX_MSG_CALL CWnd:: *pfn_wp)(CPoint);
	unsigned int(AFX_MSG_CALL CWnd:: *pfn_wv)();
	void (AFX_MSG_CALL CWnd:: *pfn_vPOS)(WINDOWPOS *);
	void (AFX_MSG_CALL CWnd:: *pfn_vCALC)(bool, NCCALCSIZE_PARAMS *);
	void (AFX_MSG_CALL CWnd:: *pfn_vwp)(unsigned int, CPoint);
	void (AFX_MSG_CALL CWnd:: *pfn_vwwh)(unsigned int, unsigned int, HANDLE);
	bool(AFX_MSG_CALL CWnd:: *pfn_bwsp)(unsigned int, short, CPoint);
	void (AFX_MSG_CALL CWnd:: *pfn_vws)(unsigned int, const char *);
	void (AFX_MSG_CALL CWnd:: *pfn_vFb)(HFONT, bool);
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

	bool Attach(HGDIOBJ hObject);
	HGDIOBJ Detach();
	bool DeleteObject();

	bool operator==(const CGdiObject &obj) const {
		return obj.m_hObject == m_hObject;
	}
	bool operator!=(const CGdiObject &obj) const {
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

	Impl *pen() const {
		return (Impl *)m_hObject;
	}

public:
	CPen() {}
	CPen(int nPenStyle, int nWidth, COLORREF crColor);
	~CPen() override {}

	bool CreatePen(int nPenStyle, int nWidth, COLORREF crColor);
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

	Impl *brush() const {
		return (Impl *)m_hObject;
	}

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

	bool CreateSolidBrush(COLORREF crColor);
	bool CreateBrushIndirect(const LOGBRUSH *lpLogBrush);
	bool CreateStockObject(int nIndex);
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

	Impl *font() const {
		return (Impl *)m_hObject;
	}

public:
	static CFont *FromHandle(HGDIOBJ h) {
		return (CFont *)CGdiObject::FromHandle(h);
	}

public:
	~CFont() override {}

	bool CreateFont(int nHeight, int nWidth, int nEscapement,
	    int nOrientation, int nWeight, byte bItalic, byte bUnderline,
	    byte cStrikeOut, byte nCharSet, byte nOutPrecision,
	    byte nClipPrecision, byte nQuality, byte nPitchAndFamily,
	    const char *lpszFacename);
	bool CreateFontIndirect(const LOGFONT *lpLogFont);
};

class CBitmap : public CGdiObject {
public:
	struct Impl : public CGdiObjectImpl,
		public Gfx::Surface {
		~Impl() override {}
	};

	Impl *bitmap() const {
		return (Impl *)m_hObject;
	}

public:
	static CBitmap *FromHandle(HGDIOBJ h) {
		return (CBitmap *)CGdiObject::FromHandle(h);
	}

public:
	~CBitmap() override {
	}

	bool Attach(HBITMAP hObject);
	HBITMAP Detach();
	bool CreateCompatibleBitmap(CDC *pDC, int nWidth, int nHeight);
	bool CreateBitmap(int nWidth, int nHeight, unsigned int nPlanes,
	    unsigned int nBitcount, const void *lpBits);
	int GetObject(int nCount, void *lpObject) const;
	long GetBitmapBits(long dwCount, void *lpBits) const;
	bool GetBitmap(BITMAP *pBitMap) const {
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

	Impl *palette() const {
		return (Impl *)m_hObject;
	}

public:
	static CPalette *FromHandle(HGDIOBJ h) {
		return (CPalette *)CGdiObject::FromHandle(h);
	}

public:
	~CPalette() override {
	}

	bool CreatePalette(LPLOGPALETTE lpLogPalette);
	int GetObject(int nCount, void *lpObject) const;
	unsigned int GetEntryCount() const;
	unsigned int GetPaletteEntries(unsigned int nStartIndex, unsigned int nNumEntries,
	                       LPPALETTEENTRY lpPaletteColors) const;
	unsigned int SetPaletteEntries(unsigned int nStartIndex, unsigned int nNumEntries,
	                       LPPALETTEENTRY lpPaletteColors);
	unsigned int SetPaletteEntries(const Graphics::Palette &pal);
	bool AnimatePalette(unsigned int nStartIndex, unsigned int nNumEntries,
	                    const PALETTEENTRY *lpPaletteColors);
	unsigned int GetNearestPaletteIndex(COLORREF crColor);
	bool isEmpty() const {
		return !palette() || palette()->empty();
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

	bool CreateRectRgn(int x1, int y1, int x2, int y2);
	bool CreateRectRgnIndirect(LPCRECT lpRect);
	bool CreatePolygonRgn(const POINT *lpPoints,
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
		CWnd *m_pWnd = nullptr;
		DefaultBitmap _defaultBitmap;
		Common::Point _linePos;
		COLORREF _bkColor = RGB(255, 255, 255);
		int _bkMode = TRANSPARENT;
		COLORREF _textColor = 0;
		uint _textAlign = TA_LEFT;
		int _drawMode;

		uint getPenColor() const;
		uint getBrushColor() const;

	public:
		HBITMAP _bitmap;
		HPEN _pen;
		HFONT _font;
		HBRUSH _brush;
		HPALETTE _palette = nullptr;
		CPalette *_cPalette = nullptr;
		bool m_bForceBackground = false;

	public:
		Impl(CWnd *wndOwner = nullptr);
		Impl(HDC srcDc);

		HGDIOBJ Attach(HGDIOBJ gdiObj);
		Gfx::Surface *getSurface() const;
		const Graphics::PixelFormat &getFormat() const;
		void setFormat(const Graphics::PixelFormat &format);
		void setScreenRect();
		void setScreenRect(const Common::Rect &r);

		HPALETTE selectPalette(HPALETTE pal, bool bForceBackground);
		CPalette *selectPalette(CPalette *pal, bool bForceBackground);
		unsigned int realizePalette();
		COLORREF GetNearestColor(COLORREF crColor) const;

		void fillSolidRect(LPCRECT lpRect, COLORREF clr);
		void fillSolidRect(int x, int y, int cx, int cy, COLORREF clr);
		void fillRect(const Common::Rect &r, COLORREF crColor);
		void drawRect(const Common::Rect &r, CBrush *brush);
		void frameRect(const Common::Rect &r, CBrush *brush);
		void frameRgn(const CRgn *pRgn, CBrush *brush, int nWidth, int nHeight);
		void floodFill(int x, int y, COLORREF crColor);
		void floodFill(int x, int y, COLORREF crColor,
			unsigned int nFillType);
		void rectangle(LPCRECT lpRect);
		void rectangle(int x1, int y1, int x2, int y2);
		void draw3dRect(const CRect &rect, COLORREF clrTopLeft, COLORREF clrBottomRight);
		void drawFocusRect(const CRect &rect);

		void ellipse(const Common::Rect &r, COLORREF crColor);
		void ellipse(LPCRECT lpRect);
		void ellipse(int x1, int y1, int x2, int y2);

		void bitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
			int xSrc, int ySrc, uint32 dwRop);
		void stretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
			int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, uint32 dwRop);

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
		bool textOut(int x, int y, const char *lpszString, int nCount,
			int nTabPositions = 0, const int *lpnTabStopPositions = nullptr,
			int nTabOrigin = 0, CSize *size = nullptr);
		bool textOut(int x, int y, const CString &str,
			int nTabPositions = 0, const int *lpnTabStopPositions = nullptr,
			int nTabOrigin = 0, CSize *size = nullptr);
		bool extTextOut(int x, int y, unsigned int nOptions, LPCRECT lpRect,
			const char *lpszString, unsigned int nCount, int *lpDxWidths);
		bool extTextOut(int x, int y, unsigned int nOptions, LPCRECT lpRect,
			const CString &str, int *lpDxWidths);
		CSize tabbedTextOut(int x, int y, const char *lpszString, int nCount,
			int nTabPositions, const int *lpnTabStopPositions, int nTabOrigin);
		CSize tabbedTextOut(int x, int y, const CString &str,
			int nTabPositions, const int *lpnTabStopPositions, int nTabOrigin);

		int drawText(const char *lpszString, int nCount, LPRECT lpRect, unsigned int nFormat,
			int nTabPositions = 0, const int *lpnTabStopPositions = nullptr,
			int nTabOrigin = 0, CSize *size = nullptr);
		int drawText(const CString &str, LPRECT lpRect, unsigned int nFormat,
			int nTabPositions = 0, const int *lpnTabStopPositions = nullptr,
			int nTabOrigin = 0, CSize *size = nullptr);

		CSize getTextExtent(const char *lpszString, int nCount) const;
		CSize getTextExtent(const CString &str) const;
		CSize getOutputTextExtent(const char *lpszString, int nCount) const;
		CSize getOutputTextExtent(const CString &str) const;
		CSize getTabbedTextExtent(const char *lpszString, int nCount,
			int nTabPositions, int *lpnTabStopPositions) const;
		CSize getTabbedTextExtent(const CString &str,
			int nTabPositions, int *lpnTabStopPositions) const;
		CSize getOutputTabbedTextExtent(const char *lpszString, int nCount,
			int nTabPositions, int *lpnTabStopPositions) const;
		CSize getOutputTabbedTextExtent(const CString &str,
			int nTabPositions, int *lpnTabStopPositions) const;
		bool grayString(CBrush *pBrush,
			bool(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
			int nCount, int x, int y, int nWidth, int nHeight);
		unsigned int getTextAlign() const;
		unsigned int setTextAlign(unsigned int nFlags);
		bool getTextMetrics(LPTEXTMETRIC lpMetrics) const;
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

	bool CreateDC(const char *lpszDriverName, const char *lpszDeviceName,
	              const char *lpszOutput, const void *lpInitData);
	bool CreateCompatibleDC(CDC *pDC);
	bool DeleteDC();
	bool Attach(HDC hDC);
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
	virtual bool PtVisible(int x, int y);
	bool PtVisible(POINT point);
	virtual bool RectVisible(LPCRECT lpRect);
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
	bool DPtoLP(LPPOINT lpPoints, int nCount = 1);
	bool DPtoLP(RECT *lpRect);
	bool LPtoDP(LPPOINT lpPoints, int nCount = 1);
	bool LPtoDP(RECT *lpRect);
	bool BitBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	            int xSrc, int ySrc, uint32 dwRop);
	bool StretchBlt(int x, int y, int nWidth, int nHeight, CDC *pSrcDC,
	                int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, uint32 dwRop);
	void Ellipse(LPCRECT lpRect);
	void Ellipse(int x1, int y1, int x2, int y2);
	void FrameRect(LPCRECT lpRect, CBrush *pBrush);
	void Draw3dRect(const CRect &rect, COLORREF clrTopLeft, COLORREF clrBottomRight);
	void DrawFocusRect(const CRect &rect);
	void FillRect(LPCRECT lpRect, CBrush *pBrush);
	void FillSolidRect(LPCRECT lpRect, COLORREF crColor);
	bool FloodFill(int x, int y, COLORREF crColor);
	bool FloodFill(int x, int y, COLORREF crColor,
	               unsigned int nFillType);
	void Rectangle(LPCRECT lpRect);
	void Rectangle(int x1, int y1, int x2, int y2);
	bool DrawEdge(LPRECT lpRect, unsigned int nEdge, unsigned int nFlags);

	bool Pie(
	    int x1, int y1,     // Upper-left corner of bounding rectangle
	    int x2, int y2,     // Lower-right corner of bounding rectangle
	    int x3, int y3,     // Starting point of the arc (on the ellipse)
	    int x4, int y4      // Ending point of the arc (on the ellipse)
	);
	bool Pie(
	    LPCRECT lpRect,         // Pointer to bounding rectangle
	    const POINT &ptStart,   // Start point of arc
	    const POINT &ptEnd      // End point of arc
	);
	bool FrameRgn(CRgn *pRgn, CBrush *pBrush,
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
	CPalette *SelectPalette(CPalette *pPalette, bool bForceBackground);
	unsigned int RealizePalette();
	void UpdateColors();
	virtual COLORREF SetBkColor(COLORREF crColor);
	COLORREF GetBkColor() const;
	int SetBkMode(int nBkMode);

	// Text Functions
	virtual COLORREF SetTextColor(COLORREF crColor);
	virtual bool TextOut(int x, int y, const char *lpszString, int nCount);
	bool TextOut(int x, int y, const CString &str);
	virtual bool ExtTextOut(int x, int y, unsigned int nOptions, LPCRECT lpRect,
	                        const char *lpszString, unsigned int nCount, int *lpDxWidths);
	bool ExtTextOut(int x, int y, unsigned int nOptions, LPCRECT lpRect,
	                const CString &str, int *lpDxWidths);
	virtual CSize TabbedTextOut(int x, int y, const char *lpszString, int nCount,
	                            int nTabPositions, int *lpnTabStopPositions, int nTabOrigin);
	CSize TabbedTextOut(int x, int y, const CString &str,
	                    int nTabPositions, int *lpnTabStopPositions, int nTabOrigin);
	int DrawText(const char *lpszString, int nCount,
	             LPRECT lpRect, unsigned int nFormat);
	int DrawText(const CString &str, LPRECT lpRect, unsigned int nFormat);


	CSize GetTextExtent(const char *lpszString, int nCount) const;
	CSize GetTextExtent(const CString &str) const;
	CSize GetOutputTextExtent(const char *lpszString, int nCount) const;
	CSize GetOutputTextExtent(const CString &str) const;
	CSize GetTabbedTextExtent(const char *lpszString, int nCount,
	                          int nTabPositions, int *lpnTabStopPositions) const;
	CSize GetTabbedTextExtent(const CString &str,
	                          int nTabPositions, int *lpnTabStopPositions) const;
	CSize GetOutputTabbedTextExtent(const char *lpszString, int nCount,
	                                int nTabPositions, int *lpnTabStopPositions) const;
	CSize GetOutputTabbedTextExtent(const CString &str,
	                                int nTabPositions, int *lpnTabStopPositions) const;
	virtual bool GrayString(CBrush *pBrush,
	                        bool(CALLBACK *lpfnOutput)(HDC, LPARAM, int), LPARAM lpData,
	                        int nCount, int x, int y, int nWidth, int nHeight);
	unsigned int GetTextAlign() const;
	unsigned int SetTextAlign(unsigned int nFlags);
	bool GetTextMetrics(LPTEXTMETRIC lpMetrics) const;
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
	const AFX_MSGMAP_ENTRY *LookupMessage(unsigned int message);
	const AFX_MSGMAP_ENTRY *AfxFindMessageEntry(
		const AFX_MSGMAP_ENTRY *lpEntry,
		unsigned int nMsg, unsigned int nCode, unsigned int nID);
	bool _AfxDispatchCmdMsg(CCmdTarget *pTarget,
		unsigned int nID, int nCode, AFX_PMSG pfn, void *pExtra,
		unsigned int nSig, AFX_CMDHANDLERINFO *pHandlerInfo);

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

	bool OnCmdMsg(unsigned int nID, int nCode, void *pExtra,
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
	bool m_bModified = false;
	ViewArray m_viewList;

public:
	bool m_bAutoDelete = true;	// default to auto delete document
	bool m_bEmbedded = false;	// default to file-based document
	CDocTemplate *m_pDocTemplate = nullptr;

public:
	~CDocument() override {}

	void UpdateAllViews(CView *pSender, LPARAM lHint = 0,
	                    CObject *pHint = nullptr);
	POSITION GetFirstViewPosition();
	CView *GetNextView(POSITION &rPosition) const;

	const CString &GetTitle() const;
	virtual void SetTitle(const char *lpszTitle);
	const CString &GetPathName() const;
	virtual void SetPathName(const char *lpszPathName, bool bAddToMRU = true);
	virtual void ClearPathName();

	virtual bool IsModified();
	virtual void SetModifiedFlag(bool bModified = true);
	virtual void ReportSaveLoadException(const char *lpszPathName,
		CException *e, bool bSaving, unsigned int nIDPDefault);
	bool SaveModified();

	void AddView(CView *pView);
	void RemoveView(CView *pView);

	// Delete doc items etc
	virtual void DeleteContents();

	virtual bool OnNewDocument() {
		return true;
	}
	virtual bool OnOpenDocument(const char *lpszPathName) {
		return true;
	}
	virtual void OnFileSaveAs() {}
	virtual void OnCloseDocument();
	virtual void OnChangedViewList();
	virtual void PreCloseFrame(CFrameWnd *pFrame) {}
	virtual void UpdateFrameCounts() {}

	DECLARE_MESSAGE_MAP()
};


typedef struct tagCREATESTRUCTA {
	void *     lpCreateParams = nullptr;
	HINSTANCE   hInstance = 0;
	HMENU       hMenu = 0;
	HWND        hwndParent = 0;
	int         cy = 0;
	int         cx = 0;
	int         y = 0;
	int         x = 0;
	long        style = 0;
	const char *     lpszName = nullptr;
	const char *     lpszClass = nullptr;
	uint32       dwExStyle = 0;
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
	virtual bool PreCreateWindow(CREATESTRUCT &cs) {
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
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam);
	virtual bool OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) {
		return false;
	}
	virtual void OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized);

	virtual LRESULT DefWindowProc(unsigned int nMsg, WPARAM wParam, LPARAM lParam) {
		return 0;
	}
	virtual void OnSetFont(CFont *pFont) {
	}

	bool OnWndMsg(unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	afx_msg void OnActivateApp(bool bActive, uint32 dwThreadID) {}
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

	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, unsigned int nCtlColor) {
		return 0;
	}

	afx_msg void OnDestroy() {}
	afx_msg void OnEnable(bool bEnable) {}
	afx_msg void OnEndSession(bool bEnding) {}
	afx_msg void OnEnterIdle(unsigned int nWhy, CWnd *pWho) {}
	afx_msg bool OnEraseBkgnd(CDC *pDC) {
		return false;
	}
	afx_msg void OnIconEraseBkgnd(CDC *pDC) {}
	afx_msg void OnKillFocus(CWnd *pNewWnd) {}
	afx_msg void OnMenuSelect(unsigned int nItemID, unsigned int nFlags, HMENU hSysMenu) {}
	afx_msg void OnMove(int x, int y) {}
	afx_msg void OnPaint() {}
	afx_msg void OnSyncPaint() {}
	afx_msg void OnParentNotify(unsigned int message, LPARAM lParam) {}
	afx_msg unsigned int OnNotifyFormat(CWnd *pWnd, unsigned int nCommand) {
		return 0;
	}
	afx_msg HCURSOR OnQueryDragIcon() {
		return 0;
	}
	afx_msg bool OnQueryEndSession() {
		return false;
	}
	afx_msg bool OnQueryNewPalette() {
		return false;
	}
	afx_msg bool OnQueryOpen() {
		return false;
	}
	afx_msg void OnSetFocus(CWnd *pOldWnd) {}
	afx_msg void OnShowWindow(bool bShow, unsigned int nStatus);
	afx_msg void OnSize(unsigned int nType, int cx, int cy) {}
	afx_msg void OnTCard(unsigned int idAction, uint32 dwActionData) {}
	afx_msg void OnSessionChange(unsigned int nSessionState, unsigned int nId) {}

	afx_msg void OnChangeUIState(unsigned int nAction, unsigned int nUIElement) {}
	afx_msg void OnUpdateUIState(unsigned int nAction, unsigned int nUIElement) {}
	afx_msg unsigned int OnQueryUIState() {
		return 0;
	}

	// Nonclient-Area message handler member functions
	afx_msg bool OnNcActivate(bool bActive) {
		return false;
	}
	afx_msg bool OnNcCreate(LPCREATESTRUCT lpCreateStruct) {
		return false;
	}
	afx_msg void OnNcDestroy();
	virtual afx_msg void PostNcDestroy() {}
	afx_msg LRESULT OnNcHitTest(CPoint point) {
		return 0;
	}
	afx_msg void OnNcLButtonDblClk(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcLButtonDown(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcLButtonUp(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcMButtonDblClk(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcMButtonDown(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcMButtonUp(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcMouseHover(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcMouseLeave() {}
	afx_msg void OnNcMouseMove(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcPaint() {}
	afx_msg void OnNcRButtonDblClk(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcRButtonDown(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcRButtonUp(unsigned int nHitTest, CPoint point) {}
	afx_msg void OnNcXButtonDown(short zHitTest, unsigned int nButton, CPoint point) {}
	afx_msg void OnNcXButtonUp(short zHitTest, unsigned int nButton, CPoint point) {}
	afx_msg void OnNcXButtonDblClk(short zHitTest, unsigned int nButton, CPoint point) {}

	// System message handler member functions
	afx_msg void OnDropFiles(HDROP hDropInfo) {}
	afx_msg void OnPaletteIsChanging(CWnd *pRealizeWnd) {}
	afx_msg void OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {}
	afx_msg void OnSysCommand(unsigned int nID, LPARAM lParam) {}
	afx_msg void OnSysDeadChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {}
	afx_msg void OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {}
	afx_msg void OnSysKeyUp(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {}
	afx_msg bool OnAppCommand(CWnd *pWnd, unsigned int nCmd, unsigned int nDevice, unsigned int nKey) {
		return false;
	}
	afx_msg void OnRawInput(unsigned int nInputCode, HRAWINPUT hRawInput) {}
	afx_msg void OnCompacting(unsigned int nCpuTime) {}
	afx_msg void OnDevModeChange(_In_z_ uint16 * lpDeviceName) {}
	afx_msg void OnFontChange() {}
	afx_msg void OnPaletteChanged(CWnd *pFocusWnd) {}
	afx_msg void OnSpoolerStatus(unsigned int nStatus, unsigned int nJobs) {}
	afx_msg void OnSysColorChange() {}
	afx_msg void OnTimeChange() {}
	afx_msg void OnSettingChange(unsigned int uFlags, const char *lpszSection) {}
	afx_msg void OnWinIniChange(const char *lpszSection) {}
	afx_msg unsigned int OnPowerBroadcast(unsigned int nPowerEvent, LPARAM lEventData) {
		return 0;
	}
	afx_msg void OnUserChanged() {}
	afx_msg void OnInputLangChange(unsigned int nCharSet, unsigned int nLocaleId) {}
	afx_msg void OnInputLangChangeRequest(unsigned int nFlags, unsigned int nLocaleId) {}
	afx_msg void OnInputDeviceChange(unsigned short nFlags, HANDLE hDevice) {}

	// Input message handler member functions
	afx_msg void OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {}
	afx_msg void OnDeadChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {}
	afx_msg void OnUniChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {}
	afx_msg void OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar *pScrollBar) {}
	afx_msg void OnVScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar *pScrollBar) {}
	afx_msg void OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {}
	afx_msg void OnKeyUp(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {}
	afx_msg void OnHotKey(unsigned int nHotKeyId, unsigned int nKey1, unsigned int nKey2) {}
	afx_msg void OnLButtonDblClk(unsigned int nFlags, CPoint point) {}
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point) {}
	afx_msg void OnLButtonUp(unsigned int nFlags, CPoint point) {}
	afx_msg void OnMButtonDblClk(unsigned int nFlags, CPoint point) {}
	afx_msg void OnMButtonDown(unsigned int nFlags, CPoint point) {}
	afx_msg void OnMButtonUp(unsigned int nFlags, CPoint point) {}
	afx_msg void OnXButtonDblClk(unsigned int nFlags, unsigned int nButton, CPoint point) {}
	afx_msg void OnXButtonDown(unsigned int nFlags, unsigned int nButton, CPoint point) {}
	afx_msg void OnXButtonUp(unsigned int nFlags, unsigned int nButton, CPoint point) {}
	afx_msg int OnMouseActivate(CWnd *pDesktopWnd, unsigned int nHitTest, unsigned int message) {
		return 0;
	}
	afx_msg void OnMouseHover(unsigned int nFlags, CPoint point) {}
	afx_msg void OnMouseLeave() {}
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point) {}
	afx_msg void OnMouseHWheel(unsigned int nFlags, short zDelta, CPoint pt) {}
	afx_msg bool OnMouseWheel(unsigned int nFlags, short zDelta, CPoint pt) {
		return false;
	}
	afx_msg void OnRButtonDblClk(unsigned int nFlags, CPoint point) {}
	afx_msg void OnRButtonDown(unsigned int nFlags, CPoint point) {}
	afx_msg void OnRButtonUp(unsigned int nFlags, CPoint point) {}
	afx_msg bool OnSetCursor(CWnd *pWnd, unsigned int nHitTest, unsigned int message);
	afx_msg void OnTimer(uintptr nIDEvent) {}

	// Control message handler member functions
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg unsigned int OnGetDlgCode() {
		return 0;
	}
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) {}
	afx_msg int OnCharToItem(unsigned int nChar, CListBox *pListBox, unsigned int nIndex) {
		return 0;
	}
	afx_msg int OnVKeyToItem(unsigned int nKey, CListBox *pListBox, unsigned int nIndex) {
		return 0;
	}
	afx_msg void OnSetFont(HFONT hFont, bool bRedraw);

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {}

	HWND Detach();

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
	static bool isRecursiveMessage(unsigned int message);

public:
	// For ScummVM the m_hWnd just points to the window itself,
	// so it'll be set up once in the constructor
	CWnd *m_hWnd = nullptr;

	CWnd *m_pParentWnd = nullptr;
	int m_nClassStyle = 0;
	int m_nStyle = 0;
	int _modalResult = DEFAULT_MODAL_RESULT;
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

	bool Create(const char *lpszClassName, const char *lpszWindowName,
	    uint32 dwStyle, const RECT &rect, CWnd *pParentWnd, unsigned int nID,
	    CCreateContext *pContext = nullptr);
	bool CreateEx(uint32 dwExStyle, const char *lpszClassName,
		const char *lpszWindowName, uint32 dwStyle,
		int x, int y, int nWidth, int nHeight,
		HWND hWndParent, LPARAM nIDorHMenu, void *lpParam = nullptr);
	bool CreateEx(uint32 dwExStyle, const char *lpszClassName,
		const char *lpszWindowName, uint32 dwStyle,
		const RECT &rect, CWnd *pParentWnd, unsigned int nID,
		void *lpParam = nullptr);

	/**
	 * Gets a list of CWnd pointers for parent controls
	 * up to the window. It only allows a single dialog
	 * parent to be present, working around the issue
	 * when modal dialogs create other modal dialogs.
	 */
	Common::Array<const CWnd *> GetSafeParents(bool includeSelf = true) const;
	Common::Array<CWnd *> GetSafeParents(bool includeSelf = true);

	CWnd *GetParent() const;
	CWnd *GetTopLevelFrame();
	HWND GetSafeHwnd() const;

	void ShowWindow(int nCmdShow);
	bool EnableWindow(bool bEnable = true);
	void UpdateWindow();
	bool RedrawWindow(
	    LPCRECT lpRectUpdate = nullptr,
	    CRgn *prgnUpdate = nullptr,
	    unsigned int flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN
	);
	void SetActiveWindow();
	void SetFocus();
	CWnd *GetFocus() const;
	bool IsWindowVisible() const {
		return (GetStyle() & WS_VISIBLE) != 0;
	}
	void DestroyWindow();
	void Invalidate(bool bErase = true);
	int GetWindowText(CString &rString) const;
	int GetWindowText(char *lpszStringBuf, int nMaxCount) const;
	bool SetWindowText(const char *lpszString);
	unsigned int GetState() const;
	uint32 GetStyle() const {
		return m_nStyle;
	}
	void SetStyle(uint32 nStyle);
	bool IsWindowEnabled() const {
		return (_itemState & ODS_DISABLED) == 0;
	}
	bool IsActiveWindow() const;

	CDC *GetDC();
	int ReleaseDC(CDC *pDC);

	bool PostMessage(unsigned int message, WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT SendMessage(unsigned int message, WPARAM wParam = 0, LPARAM lParam = 0);
	void SendMessageToDescendants(unsigned int message,
		WPARAM wParam = 0, LPARAM lParam = 0,
		bool bDeep = true, bool bOnlyPerm = false);
	static void SendMessageToDescendants(HWND hWnd, unsigned int message,
		WPARAM wParam, LPARAM lParam, bool bDeep, bool bOnlyPerm);

	bool Validate();
	bool ValidateRect(LPCRECT lpRect = nullptr);
	bool InvalidateRect(LPCRECT lpRect, bool bErase = true);
	void GetWindowRect(LPRECT lpRect) const;
	bool GetUpdateRect(LPRECT lpRect, bool bErase = false);
	bool GetClientRect(LPRECT lpRect) const;
	RECT GetClientRect() const;
	bool PointInClientRect(const POINT &pt) const;
	void ClientToScreen(LPPOINT lpPoint) const;
	void ClientToScreen(LPRECT lpRect) const;
	void ScreenToClient(LPPOINT lpPoint) const;
	void ScreenToClient(LPRECT lpRect) const;
	RECT GetWindowRectInParentCoords() const;

	void MoveWindow(LPCRECT lpRect, bool bRepaint = true);
	void MoveWindow(int x, int y, int nWidth, int nHeight,
	                bool bRepaint = true);
	HDC BeginPaint(LPPAINTSTRUCT lpPaint);
	bool EndPaint(const PAINTSTRUCT *lpPaint);

	CWnd *GetDlgItem(int nID) const;
	CWnd *GetNextDlgGroupItem(CWnd *pWndCtl, bool bPrevious = false) const;
	bool GotoDlgCtrl(CWnd *pWndCtrl);
	bool SubclassDlgItem(unsigned int nID, CWnd *pParent);
	bool SetDlgItemText(int nIDDlgItem, const char *lpString);
	int GetDlgCtrlID() const;
	void CheckDlgButton(int nIDButton, unsigned int nCheck);
	LRESULT SendDlgItemMessage(int nID, unsigned int message,
	                           WPARAM wParam = 0, LPARAM lParam = 0) const;

	uintptr SetTimer(uintptr nIDEvent, unsigned int nElapse,
	    void (CALLBACK *lpfnTimer)(HWND, unsigned int, uintptr, uint32) = nullptr);
	bool KillTimer(uintptr nIDEvent);

	virtual bool PreTranslateMessage(MSG *pMsg) {
		return false;
	}
	virtual int GetScrollPos() const {
		return 0;
	}
	int GetScrollPosition() const {
		return GetScrollPos();
	}
	virtual int SetScrollPos(int nPos, bool bRedraw = true) {
		return 0;
	}
	virtual void GetScrollRange(int *lpMinPos, int *lpMaxPos) const {}
	void GetScrollRange(int /*nBar*/, int *lpMinPos, int *lpMaxPos) const {
		GetScrollRange(lpMinPos, lpMaxPos);
	}
	virtual void SetScrollRange(int nMinPos, int nMaxPos, bool bRedraw) {}

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
	void SetFont(CFont *pFont, bool bRedraw = true);

	void pause();
};

class CFrameWnd : public CWnd {
	DECLARE_DYNCREATE(CFrameWnd)

private:
	CString m_strTitle;
	unsigned int m_nIDHelp = 0;
	CView *m_pViewActive = nullptr;

	void ActivateFrame(int nCmdShow);
	void OnUpdateFrameTitle(bool bAddToTitle);
	CWnd *CreateView(CCreateContext *pContext, unsigned int nID);

protected:
	bool PreCreateWindow(CREATESTRUCT &cCs) override {
		return true;
	}
	int OnCreate(LPCREATESTRUCT lpcs);
	int OnCreateHelper(LPCREATESTRUCT lpcs, CCreateContext *pContext);
	bool OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext *pContext);
	void OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) override;

	enum RepositionFlags {
		reposDefault = 0, reposQuery = 1, reposExtra = 2, reposNoPosLeftOver = 0x8000
	};
	bool RepositionBars(
	    unsigned int nIDFirst,
	    unsigned int nIDLast,
	    unsigned int nIDLeftOver,
	    unsigned int nFlag = reposDefault,
	    LPRECT lpRectParam = nullptr,
	    LPCRECT lpRectClient = nullptr,
	    bool bStretch = true
	);

	DECLARE_MESSAGE_MAP()

public:
	~CFrameWnd() override {
	}

	bool Create(const char *lpszClassName,
		const char *lpszWindowName,
		uint32 dwStyle,
		const RECT &rect,
		CWnd *pParentWnd = nullptr,
		const char *lpszMenuName = nullptr,
		uint32 dwExStyle = 0,
		CCreateContext *pContext = nullptr);
	bool Create(const char *lpszClassName, const char *lpszWindowName,
			uint32 dwStyle = WS_OVERLAPPEDWINDOW) {
		CRect rectDefault;
		return Create(lpszClassName, lpszWindowName, dwStyle, rectDefault);
	}

	HMENU GetMenu() const;
	void RecalcLayout(bool bNotify = true);
	void InitialUpdateFrame(CDocument *pDoc, bool bMakeVisible);

	bool LoadFrame(unsigned int nIDResource, uint32 dwDefaultStyle,
		CWnd *pParentWnd, CCreateContext *pContext);

	CView *GetActiveView() const;
	void SetActiveView(CView *pViewNew, bool bNotify = true);
	void OnSetFocus(CWnd *pOldWnd);
	CDocument *GetActiveDocument();
};

class CDialog : public CWnd {
	DECLARE_DYNAMIC(CDialog)
	friend class CDialogTemplate;

private:
	const char *m_lpszTemplateName = nullptr;
	unsigned int m_nIDHelp = 0;
	LPCDLGTEMPLATE m_lpDialogTemplate = nullptr;
	HGLOBAL m_hDialogTemplate = 0;
	int _defaultId = 0;
	CFont _dialogFont;

	void SetParent(CWnd *wnd);
	bool CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd *pParentWnd,
		void *lpDialogInit, HINSTANCE hInst);
	bool CreateIndirect(HGLOBAL hDialogTemplate, CWnd *pParentWnd,
		HINSTANCE hInst);
	bool CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate,
		CWnd *pParentWnd, HINSTANCE hInst);
	CWnd *GetDefaultPushButton() const;

protected:
	DECLARE_MESSAGE_MAP()

	virtual void OnOK();
	virtual void OnCancel();
	virtual void PostModal() {
	}

	void OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	void OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) override;

public:
	CDialog() {}
	explicit CDialog(const char *lpszTemplateName,
	                 CWnd *pParentWnd = nullptr);
	explicit CDialog(unsigned int nIDTemplate,
	                 CWnd *pParentWnd = nullptr);
	~CDialog() override {}
	bool Create(const char *lpszTemplateName,
	            CWnd *pParentWnd = nullptr);
	bool Create(unsigned int nIDTemplate,
	            CWnd *pParentWnd = nullptr);

	int DoModal();
	virtual void PreInitDialog() {}
	virtual bool OnInitDialog() {
		return true;
	}
	bool PreTranslateMessage(MSG *pMsg) override;

	uint32 GetDefID() const;
	void SetDefID(unsigned int nID);
	LRESULT HandleInitDialog(WPARAM, LPARAM);
	LRESULT HandleSetFont(WPARAM, LPARAM);
	void AFXAPI DDX_Control(CDataExchange *pDX, int nIDC, CWnd &rControl);
	void AFXAPI DDX_Radio(CDataExchange *pDX,
	                      int nIDCButton1, int &value);
	void AFXAPI DDX_Text(CDataExchange *pDX,
	                     int nIDC, int &value);
	void AFXAPI DDX_Text(CDataExchange *pDX, int nIDC, CString &value);
	void AFXAPI DDX_Text(CDataExchange *pDX, int nIDC, unsigned int &value);
	void AFXAPI DDX_Text(CDataExchange *pDX, int nIDC, long &value);
	void AFXAPI DDX_Text(CDataExchange *pDX, int nIDC, double &value);
	void AFXAPI DDX_Check(CDataExchange *pDX,
	                      int nIDC, bool value);
	void AFXAPI DDV_MinMaxInt(CDataExchange *pDX,
	                          int value, int nMin, int nMax);

	// termination
	void EndDialog(int nResult);

	bool UpdateData(bool bSaveAndValidate = true);
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
	void OnLButtonDown(unsigned int nFlags, CPoint point);
	void OnLButtonUp(unsigned int nFlags, CPoint point);
	void OnMouseMove(unsigned int nFlags, CPoint point);
	LRESULT OnBnClicked(WPARAM wParam, LPARAM lParam);
	LRESULT OnBnSetCheck(WPARAM wParam, LPARAM lParam);

public:
	~CButton() override {
	}
	CVIRTUAL bool Create(const char *lpszCaption, uint32 dwStyle,
	                     const RECT &rect, CWnd *pParentWnd, unsigned int nID);

	int GetCheck() const;
	void SetCheck(int nCheck);
	void SetButtonStyle(unsigned int nStyle, bool bRedraw = true);
	unsigned int GetButtonStyle() const;
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
	CVIRTUAL bool Create(uint32 dwStyle, const RECT &rect, CWnd *pParentWnd, unsigned int nID);
	void LimitText(int nChars);
	void SetSel(int nStartChar, int nEndChar, bool bNoScroll = false);

	afx_msg void OnPaint();
	afx_msg void OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnKillFocus(CWnd *pNewWnd);
	afx_msg void OnTimer(uintptr nTimerId);
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
	void OnLButtonDown(unsigned int nFlags, CPoint point);
	void OnLButtonUp(unsigned int nFlags, CPoint point);
	void OnMouseMove(unsigned int, CPoint point);

public:
	~CScrollBar() override { }
	CVIRTUAL bool Create(uint32 dwStyle, const RECT &rect, CWnd *pParentWnd, unsigned int nID);

	int GetScrollPos() const override;
	int SetScrollPos(int nPos, bool bRedraw = true) override;
	void GetScrollRange(int *lpMinPos, int *lpMaxPos) const override;
	void SetScrollRange(int nMinPos, int nMaxPos, bool bRedraw) override;
	void ShowScrollBar(bool bShow);
	bool SetScrollInfo(LPSCROLLINFO lpScrollInfo, bool bRedraw);
};

class CView : public CWnd {
	DECLARE_DYNAMIC(CView)

protected:
	virtual bool PreCreateWindow(CREATESTRUCT &cCs) override;
	virtual void OnDraw(CDC *pDC) {}
	void OnPaint();
	void OnNcDestroy();

	DECLARE_MESSAGE_MAP()

public:
	CDocument *m_pDocument = nullptr;

public:
	~CView() override {
	}

	int OnCreate(LPCREATESTRUCT lpcs);
	void OnDestroy();
	void PostNcDestroy() override;

	void OnInitialUpdate();
	virtual void OnUpdate(CView *pSender, LPARAM /*lHint*/, CObject * /*pHint*/);
	virtual void OnActivateView(bool, CView *, CView *) {}
	virtual void OnActivateFrame(unsigned int nState, CFrameWnd *pFrameWnd) {}

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
	unsigned int m_nIDResource = 0;
	const CRuntimeClass *m_pDocClass = nullptr;
	const CRuntimeClass *m_pFrameClass = nullptr;
	const CRuntimeClass *m_pViewClass = nullptr;

protected:
	DECLARE_MESSAGE_MAP()

	CDocTemplate(unsigned int nIDResource, const CRuntimeClass *pDocClass,
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
	virtual CDocument *OpenDocumentFile(const char *lpszPathName,
		bool bMakeVisible = true) = 0;
	virtual CDocument *CreateNewDocument();
	virtual CFrameWnd *CreateNewFrame(CDocument *pDoc, CFrameWnd *pOther);
	virtual void SetDefaultTitle(CDocument *pDocument) = 0;
	virtual void InitialUpdateFrame(CFrameWnd *pFrame, CDocument *pDoc,
		bool bMakeVisible = true);

	virtual void AddDocument(CDocument *pDoc);      // must override
	virtual void RemoveDocument(CDocument *pDoc);   // must override
	virtual void CloseAllDocuments(bool bEndSession);
	virtual POSITION GetFirstDocPosition() const = 0;
	virtual CDocument *GetNextDoc(POSITION &rPos) const = 0;

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
	    unsigned int nIDResource,
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
	CDocument *OpenDocumentFile(const char *lpszPathName,
		bool bMakeVisible = true) override;
	void AddDocument(CDocument *pDoc) override;

	void SetDefaultTitle(CDocument *pDocument) override;

	POSITION GetFirstDocPosition() const override;
	CDocument *GetNextDoc(POSITION &rPos) const override;
};

typedef Libs::List<CDocTemplate *> CTemplateList;

class CDocManager {
private:
	CTemplateList pStaticList;
	bool bStaticInit = false;

protected:
	CTemplateList m_templateList;

public:
	virtual ~CDocManager() {}
	void AddDocTemplate(CDocTemplate *pTemplate);
	void OnFileNew();
	void OnFileOpen();
	virtual void CloseAllDocuments(bool bEndSession);
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
	virtual bool InitInstance();
	virtual int ExitInstance();

public:
	static CWinApp *_activeApp;
	int m_nCmdShow = SW_SHOWNORMAL;
	const char *m_lpCmdLine = "";
	CWnd *m_pMainWnd = nullptr;
	CPalette _systemPalette;
	CPalette _currentPalette;
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

	virtual bool InitApplication();
	virtual bool SaveAllModified();

	void SetDialogBkColor();
	HCURSOR LoadStandardCursor(const char *lpszCursorName);
	HCURSOR LoadCursor(const char *lpszResourceName);
	HCURSOR LoadCursor(unsigned int nIDResource);
	HCURSOR SetCursor(HCURSOR hCursor);
	void BeginWaitCursor();
	void EndWaitCursor();

	/**
	 * Adds or remove a wait mouse cursor
	 * @param nCode     0 => restore, 1=> begin, -1=> end
	 */
	void DoWaitCursor(int nCode) override;

	void AddDocTemplate(CDocTemplate *pTemplate);
	void CloseAllDocuments(bool bEndSession);

	unsigned int GetProfileInt(const char *lpszSection,
	    const char *lpszEntry, int nDefault);
	void WriteProfileInt(const char *lpszSection,
	    const char *lpszEntry, int nValue);
	CString GetProfileString(const char *lpszSection,
		const char *lpszEntry, const char *lpszDefault = nullptr) ;
	bool WriteProfileString(const char *lpszSection,
		const char *lpszEntry, const char *lpszValue);


	virtual void OnFileNew();
	virtual void OnFileOpen();
	virtual void OnFilePrintSetup() {}

	CHandleMap<CGdiObject> *afxMapHGDIOBJ(bool bCreate = false);
	CHandleMap<CDC> *afxMapHDC(bool bCreate = false);
	CHandleMap<CWnd> *afxMapWnd(bool bCreate = false);
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
	HRSRC findResource(const char *lpName, const char *lpType);
	size_t sizeofResource(HRSRC hResInfo);
	HGLOBAL loadResource(HRSRC hResInfo);
	void *lockResource(HGLOBAL hResData);
	void unlockResource(HGLOBAL hResData);
	bool freeResource(HGLOBAL hResData);
	const Libs::Resources &getResources() const {
		return _resources;
	}

	// Fonts
	int addFontResource(const char *fontName) {
		return _fonts.addFontResource(fontName);
	}
	bool removeFontResource(const char *fontName) {
		return _fonts.removeFontResource(fontName);
	}
	HFONT createFont(int nHeight, int nWidth, int nEscapement,
			int nOrientation, int nWeight, byte bItalic, byte bUnderline,
			byte cStrikeOut, byte nCharSet, byte nOutPrecision,
			byte nClipPrecision, byte nQuality, byte nPitchAndFamily,
			const char *lpszFacename) {
		return _fonts.createFont(nHeight, nWidth,
			nEscapement, nOrientation, nWeight, bItalic,
			bUnderline, cStrikeOut, nCharSet, nOutPrecision,
			nClipPrecision, nQuality, nPitchAndFamily,
			lpszFacename);
	}
	HFONT getFont(const char *lpszFacename, int nHeight);
	HFONT getDefaultFont() {
		assert(_defaultFont.m_hObject);
		return (HFONT)_defaultFont.m_hObject;
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
		assert(_systemPalette.m_hObject);
		return (HPALETTE)_systemPalette.m_hObject;
	}

	const char *AfxRegisterWndClass(unsigned int nClassStyle,
		HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon);
	bool GetClassInfo(HINSTANCE hInstance,
		const char *  lpClassName, LPWNDCLASS lpWndClass);
};

extern CWinApp *AfxGetApp();
HINSTANCE AfxGetInstanceHandle();

} // namespace MFC
} // namespace Bagel

#endif
