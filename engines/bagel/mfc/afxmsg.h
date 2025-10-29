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

#ifndef BAGEL_MFC_AFXMSG_H
#define BAGEL_MFC_AFXMSG_H

#include "bagel/mfc/minwindef.h"

namespace Bagel {
namespace MFC {

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
/*
 * Window Messages
 */

#define WM_NULL                         0x0000
#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_MOVE                         0x0003
#define WM_SIZE                         0x0005

#define WM_ACTIVATE                     0x0006
/*
 * WM_ACTIVATE state values
 */
#define     WA_INACTIVE     0
#define     WA_ACTIVE       1
#define     WA_CLICKACTIVE  2

#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_ENABLE                       0x000A
#define WM_SETREDRAW                    0x000B
#define WM_SETTEXT                      0x000C
#define WM_GETTEXT                      0x000D
#define WM_GETTEXTLENGTH                0x000E
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#ifndef _WIN32_WCE
	#define WM_QUERYENDSESSION              0x0011
	#define WM_QUERYOPEN                    0x0013
	#define WM_ENDSESSION                   0x0016
#endif
#define WM_QUIT                         0x0012
#define WM_ERASEBKGND                   0x0014
#define WM_SYSCOLORCHANGE               0x0015
#define WM_SHOWWINDOW                   0x0018
#define WM_CTLCOLOR                     0x0019
#define WM_WININICHANGE                 0x001A
#define WM_SETTINGCHANGE                WM_WININICHANGE
#define WM_DEVMODECHANGE                0x001B
#define WM_ACTIVATEAPP                  0x001C
#define WM_FONTCHANGE                   0x001D
#define WM_TIMECHANGE                   0x001E
#define WM_CANCELMODE                   0x001F
#define WM_SETCURSOR                    0x0020
#define WM_MOUSEACTIVATE                0x0021
#define WM_CHILDACTIVATE                0x0022
#define WM_QUEUESYNC                    0x0023
#define WM_GETMINMAXINFO                0x0024
#define WM_PAINTICON                    0x0026
#define WM_ICONERASEBKGND               0x0027
#define WM_NEXTDLGCTL                   0x0028
#define WM_SPOOLERSTATUS                0x002A
#define WM_DRAWITEM                     0x002B
#define WM_MEASUREITEM                  0x002C
#define WM_DELETEITEM                   0x002D
#define WM_VKEYTOITEM                   0x002E
#define WM_CHARTOITEM                   0x002F
#define WM_SETFONT                      0x0030
#define WM_GETFONT                      0x0031
#define WM_SETHOTKEY                    0x0032
#define WM_GETHOTKEY                    0x0033
#define WM_QUERYDRAGICON                0x0037
#define WM_COMPAREITEM                  0x0039
#define WM_GETOBJECT                    0x003D
#define WM_COMPACTING                   0x0041
#define WM_COMMNOTIFY                   0x0044  /* no longer suported */
#define WM_WINDOWPOSCHANGING            0x0046
#define WM_WINDOWPOSCHANGED             0x0047
#define WM_POWER                        0x0048
#define WM_COPYDATA                     0x004A
#define WM_CANCELJOURNAL                0x004B
#define WM_NOTIFY                       0x004E
#define WM_INPUTLANGCHANGEREQUEST       0x0050
#define WM_INPUTLANGCHANGE              0x0051
#define WM_TCARD                        0x0052
#define WM_HELP                         0x0053
#define WM_USERCHANGED                  0x0054
#define WM_NOTIFYFORMAT                 0x0055

#define NFR_ANSI                             1
#define NFR_UNICODE                          2
#define NF_QUERY                             3
#define NF_REQUERY                           4

#define WM_CONTEXTMENU                  0x007B
#define WM_STYLECHANGING                0x007C
#define WM_STYLECHANGED                 0x007D
#define WM_DISPLAYCHANGE                0x007E
#define WM_GETICON                      0x007F
#define WM_SETICON                      0x0080
#define WM_NCCREATE                     0x0081
#define WM_NCDESTROY                    0x0082
#define WM_NCCALCSIZE                   0x0083
#define WM_NCHITTEST                    0x0084
#define WM_NCPAINT                      0x0085
#define WM_NCACTIVATE                   0x0086
#define WM_GETDLGCODE                   0x0087
#ifndef _WIN32_WCE
	#define WM_SYNCPAINT                    0x0088
#endif
#define WM_NCMOUSEMOVE                  0x00A0
#define WM_NCLBUTTONDOWN                0x00A1
#define WM_NCLBUTTONUP                  0x00A2
#define WM_NCLBUTTONDBLCLK              0x00A3
#define WM_NCRBUTTONDOWN                0x00A4
#define WM_NCRBUTTONUP                  0x00A5
#define WM_NCRBUTTONDBLCLK              0x00A6
#define WM_NCMBUTTONDOWN                0x00A7
#define WM_NCMBUTTONUP                  0x00A8
#define WM_NCMBUTTONDBLCLK              0x00A9
#define WM_KEYFIRST                     0x0100
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_DEADCHAR                     0x0103
#define WM_SYSKEYDOWN                   0x0104
#define WM_SYSKEYUP                     0x0105
#define WM_SYSCHAR                      0x0106
#define WM_SYSDEADCHAR                  0x0107
#define WM_INITDIALOG                   0x0110
#define WM_COMMAND                      0x0111
#define WM_SYSCOMMAND                   0x0112
#define WM_TIMER                        0x0113
#define WM_HSCROLL                      0x0114
#define WM_VSCROLL                      0x0115
#define WM_INITMENU                     0x0116
#define WM_INITMENUPOPUP                0x0117
#define WM_CTLCOLORMSGBOX               0x0132
#define WM_CTLCOLOREDIT                 0x0133
#define WM_CTLCOLORLISTBOX              0x0134
#define WM_CTLCOLORBTN                  0x0135
#define WM_CTLCOLORDLG                  0x0136
#define WM_CTLCOLORSCROLLBAR            0x0137
#define WM_CTLCOLORSTATIC               0x0138
#define MN_GETHMENU                     0x01E1
#define WM_MOUSEFIRST                   0x0200
#define WM_MOUSEMOVE                    0x0200
#define WM_LBUTTONDOWN                  0x0201
#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_MBUTTONDOWN                  0x0207
#define WM_MBUTTONUP                    0x0208
#define WM_MBUTTONDBLCLK                0x0209
#define WM_MOUSEWHEEL                   0x020A
#define WM_XBUTTONDOWN                  0x020B
#define WM_XBUTTONUP                    0x020C
#define WM_XBUTTONDBLCLK                0x020D
#define WM_MOUSEHWHEEL                  0x020E
#define WM_MOUSELAST                    0x020E
#define WM_UNICHAR                      0x0109
#define WM_KEYLAST                      0x0109
#define WM_PARENTNOTIFY                 0x0210
#define WM_ENTERMENULOOP                0x0211
#define WM_EXITMENULOOP                 0x0212
#define WM_MOUSELEAVE					0x02A3
#define UNICODE_NOCHAR                  0xFFFF

#define WM_QUERYNEWPALETTE				0x030f
#define WM_PALETTEISCHANGING			0x0310
#define WM_PALETTECHANGED				0x0311
#define WM_INITIALUPDATE                0x0364  // (params unused) - sent to children

#define MM_JOY1MOVE						0x3A0
#define MM_JOY2MOVE						0x3A1
#define MM_JOY1ZMOVE					0x3A2
#define MM_JOY2ZMOVE					0x3A3
#define MM_JOY1BUTTONDOWN				0x3B5
#define MM_JOY2BUTTONDOWN				0x3B6
#define MM_JOY1BUTTONUP					0x3B7
#define MM_JOY2BUTTONUP					0x3B8

#define MM_MCINOTIFY                    0x3B9           /* MCI */
#define MM_WOM_OPEN                     0x3BB           /* waveform output */
#define MM_WOM_CLOSE                    0x3BC
#define MM_WOM_DONE                     0x3BD

#define ON_COMMAND(id, memberFxn) \
	{ WM_COMMAND, CN_COMMAND, (uint16)id, (uint16)id, AfxSigCmd_v, \
		static_cast<AFX_PMSG> (&memberFxn) },
// ON_COMMAND(id, OnBar) is the same as
//   ON_CONTROL(0, id, OnBar) or ON_BN_CLICKED(0, id, OnBar)

#define ON_COMMAND_RANGE(id, idLast, memberFxn) \
	{ WM_COMMAND, CN_COMMAND, (uint16)id, (uint16)idLast, AfxSigCmd_RANGE, \
		(AFX_PMSG) \
		(static_cast< void (AFX_MSG_CALL CCmdTarget::*)(unsigned int) > \
		 (memberFxn)) },
// ON_COMMAND_RANGE(id, idLast, OnBar) is the same as
//   ON_CONTROL_RANGE(0, id, idLast, OnBar)

#define ON_COMMAND_EX(id, memberFxn) \
	{ WM_COMMAND, CN_COMMAND, (uint16)id, (uint16)id, AfxSigCmd_EX, \
		(AFX_PMSG) \
		(static_cast< bool (AFX_MSG_CALL CCmdTarget::*)(unsigned int) > \
		 (memberFxn)) },

#define ON_COMMAND_EX_RANGE(id, idLast, memberFxn) \
	{ WM_COMMAND, CN_COMMAND, (uint16)id, (uint16)idLast, AfxSigCmd_EX, \
		(AFX_PMSG) \
		(static_cast< bool (AFX_MSG_CALL CCmdTarget::*)(unsigned int) > \
		 (memberFxn)) },

// update ui's are listed as WM_COMMAND's so they get routed like commands
#define ON_UPDATE_COMMAND_UI(id, memberFxn) \
	{ WM_COMMAND, CN_UPDATE_COMMAND_UI, (uint16)id, (uint16)id, AfxSigCmdUI, \
		(AFX_PMSG) \
		(static_cast< void (AFX_MSG_CALL CCmdTarget::*)(CCmdUI*) > \
		 (memberFxn)) },

#define ON_UPDATE_COMMAND_UI_RANGE(id, idLast, memberFxn) \
	{ WM_COMMAND, CN_UPDATE_COMMAND_UI, (uint16)id, (uint16)idLast, AfxSigCmdUI, \
		(AFX_PMSG) \
		(static_cast< void (AFX_MSG_CALL CCmdTarget::*)(CCmdUI*) > \
		 (memberFxn)) },

#define ON_NOTIFY(wNotifyCode, id, memberFxn) \
	{ WM_NOTIFY, (uint16)(int)wNotifyCode, (uint16)id, (uint16)id, AfxSigNotify_v, \
		(AFX_PMSG) \
		(static_cast< void (AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*) > \
		 (memberFxn)) },

#define ON_NOTIFY_RANGE(wNotifyCode, id, idLast, memberFxn) \
	{ WM_NOTIFY, (uint16)(int)wNotifyCode, (uint16)id, (uint16)idLast, AfxSigNotify_RANGE, \
		(AFX_PMSG) \
		(static_cast< void (AFX_MSG_CALL CCmdTarget::*)(unsigned int, NMHDR*, LRESULT*) > \
		 (memberFxn)) },

#define ON_NOTIFY_EX(wNotifyCode, id, memberFxn) \
	{ WM_NOTIFY, (uint16)(int)wNotifyCode, (uint16)id, (uint16)id, AfxSigNotify_EX, \
		(AFX_PMSG) \
		(static_cast< bool (AFX_MSG_CALL CCmdTarget::*)(unsigned int, NMHDR*, LRESULT*) > \
		 (memberFxn)) },

#define ON_NOTIFY_EX_RANGE(wNotifyCode, id, idLast, memberFxn) \
	{ WM_NOTIFY, (uint16)(int)wNotifyCode, (uint16)id, (uint16)idLast, AfxSigNotify_EX, \
		(AFX_PMSG) \
		(static_cast< bool (AFX_MSG_CALL CCmdTarget::*)(unsigned int, NMHDR*, LRESULT*) > \
		 (memberFxn)) },

// for general controls
#define ON_CONTROL(wNotifyCode, id, memberFxn) \
	{ WM_COMMAND, (uint16)wNotifyCode, (uint16)id, (uint16)id, AfxSigCmd_v, \
		(static_cast< AFX_PMSG > (&memberFxn)) },

#define ON_CONTROL_RANGE(wNotifyCode, id, idLast, memberFxn) \
	{ WM_COMMAND, (uint16)wNotifyCode, (uint16)id, (uint16)idLast, AfxSigCmd_RANGE, \
		(AFX_PMSG) \
		(static_cast< void (AFX_MSG_CALL CCmdTarget::*)(unsigned int) > (memberFxn)) },

#define WM_REFLECT_BASE 0xBC00

// for control notification reflection
#define ON_CONTROL_REFLECT(wNotifyCode, memberFxn) \
	{ WM_COMMAND+WM_REFLECT_BASE, (uint16)wNotifyCode, 0, 0, AfxSigCmd_v, \
		(static_cast<AFX_PMSG> (memberFxn)) },

#define ON_CONTROL_REFLECT_EX(wNotifyCode, memberFxn) \
	{ WM_COMMAND+WM_REFLECT_BASE, (uint16)wNotifyCode, 0, 0, AfxSigCmd_b, \
		(AFX_PMSG) \
		(static_cast<bool (AFX_MSG_CALL CCmdTarget::*)()> (memberFxn)) },

#define ON_NOTIFY_REFLECT(wNotifyCode, memberFxn) \
	{ WM_NOTIFY+WM_REFLECT_BASE, (uint16)(int)wNotifyCode, 0, 0, AfxSigNotify_v, \
		(AFX_PMSG) \
		(static_cast<void (AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*) > \
		 (memberFxn)) },

#define ON_NOTIFY_REFLECT_EX(wNotifyCode, memberFxn) \
	{ WM_NOTIFY+WM_REFLECT_BASE, (uint16)(int)wNotifyCode, 0, 0, AfxSigNotify_b, \
		(AFX_PMSG) \
		(static_cast<bool (AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*) > \
		 (memberFxn)) },

#define ON_UPDATE_COMMAND_UI_REFLECT(memberFxn) \
	{ WM_COMMAND+WM_REFLECT_BASE, (uint16)CN_UPDATE_COMMAND_UI, 0, 0, AfxSigCmdUI, \
		(AFX_PMSG) \
		(static_cast<void (AFX_MSG_CALL CCmdTarget::*)(CCmdUI*) > \
		 (memberFxn)) },

/////////////////////////////////////////////////////////////////////////////
// Message map tables for Windows messages

#define ON_WM_CREATE() \
	{ WM_CREATE, 0, 0, 0, AfxSig_is, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(LPCREATESTRUCT) > ( &ThisClass::OnCreate)) },

#define ON_WM_COPYDATA() \
	{ WM_COPYDATA, 0, 0, 0, AfxSig_bWCDS, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)(CWnd*, COPYDATASTRUCT*) > ( &ThisClass::OnCopyData)) },

#define ON_WM_DESTROY() \
	{ WM_DESTROY, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnDestroy)) },

#define ON_WM_MOVE() \
	{ WM_MOVE, 0, 0, 0, AfxSig_vvii, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(int, int) > ( &ThisClass::OnMove)) },

#define ON_WM_SIZE() \
	{ WM_SIZE, 0, 0, 0, AfxSig_vwii, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, int, int) > ( &ThisClass::OnSize)) },

#define ON_WM_ACTIVATE() \
	{ WM_ACTIVATE, 0, 0, 0, AfxSig_vwWb, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CWnd*, bool) > ( &ThisClass::OnActivate)) },

#define ON_WM_SETFOCUS() \
	{ WM_SETFOCUS, 0, 0, 0, AfxSig_vW, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*) > ( &ThisClass::OnSetFocus)) },

#define ON_WM_KILLFOCUS() \
	{ WM_KILLFOCUS, 0, 0, 0, AfxSig_vW, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*) > ( &ThisClass::OnKillFocus)) },

#define ON_WM_ENABLE() \
	{ WM_ENABLE, 0, 0, 0, AfxSig_vb, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool) > ( &ThisClass::OnEnable)) },

#define ON_WM_HELPINFO() \
	{ WM_HELP, 0, 0, 0, AfxSig_bHELPINFO, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)(HELPINFO*) > ( &ThisClass::OnHelpInfo)) },

#define ON_WM_PAINT() \
	{ WM_PAINT, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnPaint)) },

#define ON_WM_SYNCPAINT() \
	{ WM_SYNCPAINT, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnSyncPaint)) },

#define ON_WM_CLOSE() \
	{ WM_CLOSE, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnClose)) },

#define ON_WM_QUERYENDSESSION() \
	{ WM_QUERYENDSESSION, 0, 0, 0, AfxSig_bv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnQueryEndSession)) },

#define ON_WM_QUERYOPEN() \
	{ WM_QUERYOPEN, 0, 0, 0, AfxSig_bv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnQueryOpen)) },

#define ON_WM_ERASEBKGND() \
	{ WM_ERASEBKGND, 0, 0, 0, AfxSig_bD, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)(CDC*) > ( &ThisClass::OnEraseBkgnd)) },

#define ON_WM_PRINTCLIENT() \
	{ WM_PRINTCLIENT, 0, 0, 0, AfxSig_l_D_u, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)(CDC*, unsigned int) > ( &ThisClass::OnPrintClient)) },

#define ON_WM_SYSCOLORCHANGE() \
	{ WM_SYSCOLORCHANGE, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnSysColorChange)) },

#define ON_WM_ENDSESSION() \
	{ WM_ENDSESSION, 0, 0, 0, AfxSig_vb, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool) > ( &ThisClass::OnEndSession)) },

#define ON_WM_SETTEXT() \
	{ WM_SETTEXT, 0, 0, 0, AfxSig_i_v_S, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(const char *) > ( &ThisClass::OnSetText)) },

#define ON_WM_GETTEXT() \
	{ WM_GETTEXT, 0, 0, 0, AfxSig_i_i_s, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(int, uint16 *) > ( &ThisClass::OnGetText)) },

#define ON_WM_GETTEXTLENGTH() \
	{ WM_GETTEXTLENGTH, 0, 0, 0, AfxSig_u_v_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< unsigned int (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnGetTextLength)) },

#define ON_WM_SETFONT() \
	{ WM_SETFONT, 0, 0, 0, AfxSig_vwpb, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(HFONT, bool) > ( &ThisClass::OnSetFont)) },

#define ON_WM_GETFONT() \
	{ WM_GETFONT, 0, 0, 0, AfxSig_h_v_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< HFONT (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnGetFont)) },

#define ON_WM_SETICON() \
	{ WM_SETICON, 0, 0, 0, AfxSig_h_b_h, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< HICON (AFX_MSG_CALL CWnd::*)(bool, HICON) > ( &ThisClass::OnSetIcon)) },

#define ON_WM_MDISETMENU() \
	{ WM_MDISETMENU, 0, 0, 0, AfxSig_h_h_h, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< HMENU (AFX_MSG_CALL CWnd::*)(HMENU, HMENU) > ( &ThisClass::OnMDISetMenu)) },

#define  ON_WM_MDIREFRESHMENU() \
	{ WM_MDIREFRESHMENU, 0, 0, 0, AfxSig_h_v_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< HMENU (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnMDIRefreshMenu)) },

#define  ON_WM_MDIDESTROY() \
	{ WM_MDIDESTROY, 0, 0, 0, AfxSig_v_W_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*) > ( &ThisClass::OnMDIDestroy)) },

#define  ON_WM_MDINEXT() \
	{ WM_MDINEXT, 0, 0, 0, AfxSig_MDINext, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*, bool) > ( &ThisClass::OnMDINext)) },

#define  ON_WM_CUT() \
	{ WM_CUT, 0, 0, 0, AfxSig_v_v_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnCut)) },

#define  ON_WM_COPY() \
	{ WM_COPY, 0, 0, 0, AfxSig_v_v_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnCopy)) },

#define  ON_WM_PASTE() \
	{ WM_PASTE, 0, 0, 0, AfxSig_v_v_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnPaste)) },

#define  ON_WM_CLEAR() \
	{ WM_CLEAR, 0, 0, 0, AfxSig_v_v_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnClear)) },

#define  ON_WM_DISPLAYCHANGE() \
	{ WM_DISPLAYCHANGE, 0, 0, 0, AfxSig_v_u_ii, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, int, int) > ( &ThisClass::OnDisplayChange)) },

#define  ON_WM_DDE_INITIATE() \
	{ WM_DDE_INITIATE, 0, 0, 0, AfxSig_v_W_uu, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*, unsigned int, unsigned int) > ( &ThisClass::OnDDEInitiate)) },

#define  ON_WM_DDE_EXECUTE() \
	{ WM_DDE_EXECUTE, 0, 0, 0, AfxSig_v_W_h, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*, HANDLE) > ( &ThisClass::OnDDEExecute)) },

#define  ON_WM_DDE_TERMINATE() \
	{ WM_DDE_TERMINATE, 0, 0, 0, AfxSig_v_W_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*) > ( &ThisClass::OnDDETerminate)) },

#define ON_WM_WTSSESSION_CHANGE() \
	{ WM_WTSSESSION_CHANGE, 0, 0, 0, AfxSig_vww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::OnSessionChange)) },

#define ON_WM_SHOWWINDOW() \
	{ WM_SHOWWINDOW, 0, 0, 0, AfxSig_vbw, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool, unsigned int) > ( &ThisClass::OnShowWindow)) },

#define ON_WM_CTLCOLOR() \
	{ WM_CTLCOLOR, 0, 0, 0, AfxSig_hDWw, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< HBRUSH (AFX_MSG_CALL CWnd::*)(CDC*, CWnd*, unsigned int)>  ( &ThisClass::OnCtlColor)) },

#define ON_WM_CTLCOLOR_REFLECT() \
	{ WM_CTLCOLOR+WM_REFLECT_BASE, 0, 0, 0, AfxSig_hDw, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< HBRUSH (AFX_MSG_CALL CWnd::*)(CDC*, unsigned int) > ( &ThisClass::CtlColor)) },

#define ON_WM_SETTINGCHANGE() \
	{ WM_SETTINGCHANGE, 0, 0, 0, AfxSig_vws, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, const char *) > ( &ThisClass::OnSettingChange)) },

#define ON_WM_WININICHANGE() \
	{ WM_WININICHANGE, 0, 0, 0, AfxSig_vs, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(const char *) > ( &ThisClass::OnWinIniChange)) },

#define ON_WM_DEVMODECHANGE() \
	{ WM_DEVMODECHANGE, 0, 0, 0, AfxSig_vs, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(uint16 *) > ( &ThisClass::OnDevModeChange)) },

#define ON_WM_ACTIVATEAPP() \
	{ WM_ACTIVATEAPP, 0, 0, 0, AfxSig_vww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool, HTASK) > ( &ThisClass::OnActivateApp)) },

#define ON_WM_FONTCHANGE() \
	{ WM_FONTCHANGE, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnFontChange)) },

#define ON_WM_TIMECHANGE() \
	{ WM_TIMECHANGE, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnTimeChange)) },

#define ON_WM_CANCELMODE() \
	{ WM_CANCELMODE, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnCancelMode)) },

#define ON_WM_SETCURSOR() \
	{ WM_SETCURSOR, 0, 0, 0, AfxSig_bWww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)(CWnd*, unsigned int, unsigned int) > ( &ThisClass::OnSetCursor)) },

#define ON_WM_MOUSEACTIVATE() \
	{ WM_MOUSEACTIVATE, 0, 0, 0, AfxSig_iWww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(CWnd*, unsigned int, unsigned int) > ( &ThisClass::OnMouseActivate)) },

#define ON_WM_CHILDACTIVATE() \
	{ WM_CHILDACTIVATE, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnChildActivate)) },

#define ON_WM_GETMINMAXINFO() \
	{ WM_GETMINMAXINFO, 0, 0, 0, AfxSig_vs, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(MINMAXINFO*) > ( &ThisClass::OnGetMinMaxInfo)) },

#define ON_WM_ICONERASEBKGND() \
	{ WM_ICONERASEBKGND, 0, 0, 0, AfxSig_vD, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CDC*) > ( &ThisClass::OnIconEraseBkgnd)) },

#define ON_WM_SPOOLERSTATUS() \
	{ WM_SPOOLERSTATUS, 0, 0, 0, AfxSig_vww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::OnSpoolerStatus)) },

#define ON_WM_DRAWITEM() \
	{ WM_DRAWITEM, 0, 0, 0, AfxSig_vOWNER, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(int, LPDRAWITEMSTRUCT) > ( &ThisClass::OnDrawItem)) },

#define ON_WM_DRAWITEM_REFLECT() \
	{ WM_DRAWITEM+WM_REFLECT_BASE, 0, 0, 0, AfxSig_vs, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(LPDRAWITEMSTRUCT) > ( &ThisClass::DrawItem)) },

#define ON_WM_MEASUREITEM() \
	{ WM_MEASUREITEM, 0, 0, 0, AfxSig_vOWNER, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(int, LPMEASUREITEMSTRUCT) > ( &ThisClass::OnMeasureItem)) },

#define ON_WM_MEASUREITEM_REFLECT() \
	{ WM_MEASUREITEM+WM_REFLECT_BASE, 0, 0, 0, AfxSig_vs, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(LPMEASUREITEMSTRUCT) > ( &ThisClass::MeasureItem)) },

#define ON_WM_DELETEITEM() \
	{ WM_DELETEITEM, 0, 0, 0, AfxSig_vOWNER, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(int, LPDELETEITEMSTRUCT) > ( &ThisClass::OnDeleteItem)) },

#define ON_WM_DELETEITEM_REFLECT() \
	{ WM_DELETEITEM+WM_REFLECT_BASE, 0, 0, 0, AfxSig_vs, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(LPDELETEITEMSTRUCT) > ( &ThisClass::DeleteItem)) },

#define ON_WM_CHARTOITEM() \
	{ WM_CHARTOITEM, 0, 0, 0, AfxSig_iwWw, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(unsigned int, CListBox*, unsigned int) > ( &ThisClass::OnCharToItem)) },

#define ON_WM_CHARTOITEM_REFLECT() \
	{ WM_CHARTOITEM+WM_REFLECT_BASE, 0, 0, 0, AfxSig_iww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::CharToItem)) },

#define ON_WM_VKEYTOITEM() \
	{ WM_VKEYTOITEM, 0, 0, 0, AfxSig_iwWw, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(unsigned int, CListBox*, unsigned int) > ( &ThisClass::OnVKeyToItem)) },

#define ON_WM_VKEYTOITEM_REFLECT() \
	{ WM_VKEYTOITEM+WM_REFLECT_BASE, 0, 0, 0, AfxSig_iww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::VKeyToItem)) },

#define ON_WM_QUERYDRAGICON() \
	{ WM_QUERYDRAGICON, 0, 0, 0, AfxSig_hv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< HCURSOR (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnQueryDragIcon)) },

#define ON_WM_COMPAREITEM() \
	{ WM_COMPAREITEM, 0, 0, 0, AfxSig_iis, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(int, LPCOMPAREITEMSTRUCT) > ( &ThisClass::OnCompareItem)) },

#define ON_WM_COMPAREITEM_REFLECT() \
	{ WM_COMPAREITEM+WM_REFLECT_BASE, 0, 0, 0, AfxSig_is, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< int (AFX_MSG_CALL CWnd::*)(LPCOMPAREITEMSTRUCT) > ( &ThisClass::CompareItem)) },

#define ON_WM_COMPACTING() \
	{ WM_COMPACTING, 0, 0, 0, AfxSig_vw, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int) > ( &ThisClass::OnCompacting)) },

#define ON_WM_NCCREATE() \
	{ WM_NCCREATE, 0, 0, 0, AfxSig_is, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)(LPCREATESTRUCT) > ( &ThisClass::OnNcCreate)) },

#define ON_WM_NCDESTROY() \
	{ WM_NCDESTROY, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnNcDestroy)) },

#define ON_WM_NCCALCSIZE() \
	{ WM_NCCALCSIZE, 0, 0, 0, AfxSig_vCALC, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool, NCCALCSIZE_PARAMS*) > ( &ThisClass::OnNcCalcSize)) },

#define ON_WM_NCHITTEST() \
	{ WM_NCHITTEST, 0, 0, 0, AfxSig_l_p, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)(CPoint) > (&ThisClass::OnNcHitTest)) },

#define ON_WM_NCPAINT() \
	{ WM_NCPAINT, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnNcPaint)) },

#define ON_WM_NCACTIVATE() \
	{ WM_NCACTIVATE, 0, 0, 0, AfxSig_bb, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)(bool) > ( &ThisClass::OnNcActivate)) },

#define ON_WM_GETDLGCODE() \
	{ WM_GETDLGCODE, 0, 0, 0, AfxSig_wv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< unsigned int (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnGetDlgCode)) },

#define ON_WM_NCMOUSEMOVE() \
	{ WM_NCMOUSEMOVE, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcMouseMove)) },

#define ON_WM_NCMOUSEHOVER() \
	{ WM_NCMOUSEHOVER, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcMouseHover)) },

#define ON_WM_NCMOUSELEAVE() \
	{ WM_NCMOUSELEAVE, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnNcMouseLeave)) },

#define ON_WM_NCLBUTTONDOWN() \
	{ WM_NCLBUTTONDOWN, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcLButtonDown)) },

#define ON_WM_NCLBUTTONUP() \
	{ WM_NCLBUTTONUP, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcLButtonUp)) },

#define ON_WM_NCLBUTTONDBLCLK() \
	{ WM_NCLBUTTONDBLCLK, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcLButtonDblClk)) },

#define ON_WM_NCRBUTTONDOWN() \
	{ WM_NCRBUTTONDOWN, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcRButtonDown)) },

#define ON_WM_NCRBUTTONUP() \
	{ WM_NCRBUTTONUP, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcRButtonUp)) },

#define ON_WM_NCRBUTTONDBLCLK() \
	{ WM_NCRBUTTONDBLCLK, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcRButtonDblClk)) },

#define ON_WM_NCMBUTTONDOWN() \
	{ WM_NCMBUTTONDOWN, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcMButtonDown)) },

#define ON_WM_NCMBUTTONUP() \
	{ WM_NCMBUTTONUP, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcMButtonUp)) },

#define ON_WM_NCMBUTTONDBLCLK() \
	{ WM_NCMBUTTONDBLCLK, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnNcMButtonDblClk)) },

#define ON_WM_NCXBUTTONDOWN() \
	{ WM_NCXBUTTONDOWN, 0, 0, 0, AfxSig_MOUSE_NCXBUTTON, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(short, unsigned int, CPoint) > ( &ThisClass::OnNcXButtonDown)) },

#define ON_WM_NCXBUTTONUP() \
	{ WM_NCXBUTTONUP, 0, 0, 0, AfxSig_MOUSE_NCXBUTTON, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(short, unsigned int, CPoint) > ( &ThisClass::OnNcXButtonUp)) },

#define ON_WM_NCXBUTTONDBLCLK() \
	{ WM_NCXBUTTONDBLCLK, 0, 0, 0, AfxSig_MOUSE_NCXBUTTON, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(short, unsigned int, CPoint) > ( &ThisClass::OnNcXButtonDblClk)) },

#define ON_WM_KEYDOWN() \
	{ WM_KEYDOWN, 0, 0, 0, AfxSig_vwww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnKeyDown)) },

#define ON_WM_KEYUP() \
	{ WM_KEYUP, 0, 0, 0, AfxSig_vwww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnKeyUp)) },

#define ON_WM_HOTKEY() \
	{ WM_HOTKEY, 0, 0, 0, AfxSig_v_u_uu, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnHotKey)) },

#define ON_WM_CHAR() \
	{ WM_CHAR, 0, 0, 0, AfxSig_vwww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnChar)) },

#define ON_WM_UNICHAR() \
	{ WM_UNICHAR, 0, 0, 0, AfxSig_vwww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnUniChar)) },

#define ON_WM_DEADCHAR() \
	{ WM_DEADCHAR, 0, 0, 0, AfxSig_vwww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnDeadChar)) },

#define ON_WM_SYSKEYDOWN() \
	{ WM_SYSKEYDOWN, 0, 0, 0, AfxSig_vwww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnSysKeyDown)) },

#define ON_WM_SYSKEYUP() \
	{ WM_SYSKEYUP, 0, 0, 0, AfxSig_vwww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnSysKeyUp)) },

#define ON_WM_SYSCHAR() \
	{ WM_SYSCHAR, 0, 0, 0, AfxSig_vwww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast<void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnSysChar)) },

#define ON_WM_SYSDEADCHAR() \
	{ WM_SYSDEADCHAR, 0, 0, 0, AfxSig_vwww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnSysDeadChar)) },

#define ON_WM_SYSCOMMAND() \
	{ WM_SYSCOMMAND, 0, 0, 0, AfxSig_vwl, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, LPARAM) > ( &ThisClass::OnSysCommand)) },

#define ON_WM_INPUTLANGCHANGE() \
	{ WM_INPUTLANGCHANGE, 0, 0, 0, AfxSig_INPUTLANGCHANGE, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::OnInputLangChange)) },

#define ON_WM_INPUTLANGCHANGEREQUEST() \
	{ WM_INPUTLANGCHANGEREQUEST, 0, 0, 0, AfxSig_vww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::OnInputLangChangeRequest)) },

#define ON_WM_APPCOMMAND() \
	{ WM_APPCOMMAND, 0, 0, 0, AfxSig_APPCOMMAND, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*, unsigned int, unsigned int, unsigned int) > ( &ThisClass::OnAppCommand)) },

#define ON_WM_INPUT() \
	{ WM_INPUT, 0, 0, 0, AfxSig_RAWINPUT, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, HRAWINPUT) > ( &ThisClass::OnRawInput)) },

#define ON_WM_INPUT_DEVICE_CHANGE() \
	{ WM_INPUT_DEVICE_CHANGE, 0, 0, 0, AfxSig_INPUTDEVICECHANGE, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned short, HANDLE) > ( &ThisClass::OnInputDeviceChange)) },

#define ON_WM_TCARD() \
	{ WM_TCARD, 0, 0, 0, AfxSig_vwl, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, uint32) > ( &ThisClass::OnTCard)) },

#define ON_WM_TIMER() \
	{ WM_TIMER, 0, 0, 0, AfxSig_vw, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(uintptr) > ( &ThisClass::OnTimer)) },

#define ON_WM_HSCROLL() \
	{ WM_HSCROLL, 0, 0, 0, AfxSig_vwwW, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, CScrollBar*) > ( &ThisClass::OnHScroll)) },

#define ON_WM_HSCROLL_REFLECT() \
	{ WM_HSCROLL+WM_REFLECT_BASE, 0, 0, 0, AfxSig_vwwx, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::HScroll)) },

#define ON_WM_VSCROLL() \
	{ WM_VSCROLL, 0, 0, 0, AfxSig_vwwW, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, CScrollBar*) > ( &ThisClass::OnVScroll)) },

#define ON_WM_VSCROLL_REFLECT() \
	{ WM_VSCROLL+WM_REFLECT_BASE, 0, 0, 0, AfxSig_vwwx, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::VScroll)) },

#define ON_WM_INITMENU() \
	{ WM_INITMENU, 0, 0, 0, AfxSig_vM, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CMenu*) > ( &ThisClass::OnInitMenu)) },

#define ON_WM_INITMENUPOPUP() \
	{ WM_INITMENUPOPUP, 0, 0, 0, AfxSig_vMwb, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CMenu*, unsigned int, bool) > ( &ThisClass::OnInitMenuPopup)) },

#define ON_WM_MENUSELECT() \
	{ WM_MENUSELECT, 0, 0, 0, AfxSig_vwwh, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, HMENU) > ( &ThisClass::OnMenuSelect)) },

#define ON_WM_MENUCHAR() \
	{ WM_MENUCHAR, 0, 0, 0, AfxSig_lwwM, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, CMenu*) > ( &ThisClass::OnMenuChar)) },

#define ON_WM_MENURBUTTONUP() \
	{ WM_MENURBUTTONUP, 0, 0, 0, AfxSig_v_u_M, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CMenu*) > ( &ThisClass::OnMenuRButtonUp)) },

#define ON_WM_MENUDRAG() \
	{ WM_MENUDRAG, 0, 0, 0, AfxSig_u_u_M, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< unsigned int (AFX_MSG_CALL CWnd::*)(unsigned int, CMenu*) > ( &ThisClass::OnMenuDrag)) },

#define ON_WM_MENUGETOBJECT() \
	{ WM_MENUGETOBJECT, 0, 0, 0, AfxSig_u_v_MENUGETOBJECTINFO, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< unsigned int (AFX_MSG_CALL CWnd::*)(MENUGETOBJECTINFO*) > ( &ThisClass::OnMenuGetObject)) },

#define ON_WM_UNINITMENUPOPUP() \
	{ WM_UNINITMENUPOPUP, 0, 0, 0, AfxSig_v_M_u, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CMenu*, unsigned int) > ( &ThisClass::OnUnInitMenuPopup)) },

#define ON_WM_NEXTMENU() \
	{ WM_NEXTMENU, 0, 0, 0, AfxSig_v_u_LPMDINEXTMENU, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, LPMDINEXTMENU) > ( &ThisClass::OnNextMenu)) },

#define ON_WM_ENTERIDLE() \
	{ WM_ENTERIDLE, 0, 0, 0, AfxSig_vwW, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CWnd*) > ( &ThisClass::OnEnterIdle)) },

#define ON_WM_MOUSEMOVE() \
	{ WM_MOUSEMOVE, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnMouseMove)) },

#define ON_WM_MOUSEHOVER() \
	{ WM_MOUSEHOVER, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnMouseHover)) },

#define ON_WM_MOUSELEAVE() \
	{ WM_MOUSELEAVE, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnMouseLeave)) },

#define ON_WM_MOUSEWHEEL() \
	{ WM_MOUSEWHEEL, 0, 0, 0, AfxSig_bwsp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)(unsigned int, short, CPoint) > ( &ThisClass::OnMouseWheel)) },

#define ON_WM_MOUSEHWHEEL() \
	{ WM_MOUSEHWHEEL, 0, 0, 0, AfxSig_MOUSEHWHEEL, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, short, CPoint) > ( &ThisClass::OnMouseHWheel)) },

#define ON_WM_LBUTTONDOWN() \
	{ WM_LBUTTONDOWN, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnLButtonDown)) },

#define ON_WM_LBUTTONUP() \
	{ WM_LBUTTONUP, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnLButtonUp)) },

#define ON_WM_LBUTTONDBLCLK() \
	{ WM_LBUTTONDBLCLK, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnLButtonDblClk)) },

#define ON_WM_RBUTTONDOWN() \
	{ WM_RBUTTONDOWN, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnRButtonDown)) },

#define ON_WM_RBUTTONUP() \
	{ WM_RBUTTONUP, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnRButtonUp)) },

#define ON_WM_RBUTTONDBLCLK() \
	{ WM_RBUTTONDBLCLK, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnRButtonDblClk)) },

#define ON_WM_MBUTTONDOWN() \
	{ WM_MBUTTONDOWN, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnMButtonDown)) },

#define ON_WM_MBUTTONUP() \
	{ WM_MBUTTONUP, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnMButtonUp)) },

#define ON_WM_MBUTTONDBLCLK() \
	{ WM_MBUTTONDBLCLK, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, CPoint) > ( &ThisClass::OnMButtonDblClk)) },

#define ON_WM_XBUTTONDOWN() \
	{ WM_XBUTTONDOWN, 0, 0, 0, AfxSig_MOUSE_XBUTTON, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, CPoint) > ( &ThisClass::OnXButtonDown)) },

#define ON_WM_XBUTTONUP() \
	{ WM_XBUTTONUP, 0, 0, 0, AfxSig_MOUSE_XBUTTON, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, CPoint) > ( &ThisClass::OnXButtonUp)) },

#define ON_WM_XBUTTONDBLCLK() \
	{ WM_XBUTTONDBLCLK, 0, 0, 0, AfxSig_MOUSE_XBUTTON, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int, CPoint) > ( &ThisClass::OnXButtonDblClk)) },

#define ON_WM_PARENTNOTIFY() \
	{ WM_PARENTNOTIFY, 0, 0, 0, AfxSig_vwl, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast<void (AFX_MSG_CALL CWnd::*)(unsigned int, LPARAM) > ( &ThisClass::OnParentNotify)) },

#define ON_WM_PARENTNOTIFY_REFLECT() \
	{ WM_PARENTNOTIFY+WM_REFLECT_BASE, 0, 0, 0, AfxSig_vwl, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, LPARAM) > ( &ThisClass::ParentNotify)) },

#define ON_WM_NOTIFYFORMAT() \
	{ WM_NOTIFYFORMAT, 0, 0, 0, AfxSig_u_W_u, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< unsigned int (AFX_MSG_CALL CWnd::*)(CWnd*, unsigned int) > ( &ThisClass::OnNotifyFormat)) },

#define ON_WM_MDIACTIVATE() \
	{ WM_MDIACTIVATE, 0, 0, 0, AfxSig_vbWW, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool, CWnd*, CWnd*) > ( &ThisClass::OnMDIActivate)) },

#define ON_WM_RENDERFORMAT() \
	{ WM_RENDERFORMAT, 0, 0, 0, AfxSig_vw, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int) > ( &ThisClass::OnRenderFormat)) },

#define ON_WM_RENDERALLFORMATS() \
	{ WM_RENDERALLFORMATS, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnRenderAllFormats)) },

#define ON_WM_DESTROYCLIPBOARD() \
	{ WM_DESTROYCLIPBOARD, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnDestroyClipboard)) },

#define ON_WM_DRAWCLIPBOARD() \
	{ WM_DRAWCLIPBOARD, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnDrawClipboard)) },

#define ON_WM_PAINTCLIPBOARD() \
	{ WM_PAINTCLIPBOARD, 0, 0, 0, AfxSig_vWh, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*, HGLOBAL) > ( &ThisClass::OnPaintClipboard)) },

#define ON_WM_VSCROLLCLIPBOARD() \
	{ WM_VSCROLLCLIPBOARD, 0, 0, 0, AfxSig_vWww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*, unsigned int, unsigned int) > ( &ThisClass::OnVScrollClipboard)) },

#define ON_WM_CONTEXTMENU() \
	{ WM_CONTEXTMENU, 0, 0, 0, AfxSig_vWp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*, CPoint) > ( &ThisClass::OnContextMenu)) },

#define ON_WM_SIZECLIPBOARD() \
	{ WM_SIZECLIPBOARD, 0, 0, 0, AfxSig_vWh, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*, HGLOBAL)> ( &ThisClass::OnSizeClipboard)) },

#define ON_WM_ASKCBFORMATNAME() \
	{ WM_ASKCBFORMATNAME, 0, 0, 0, AfxSig_vwl, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, uint16 *) > ( &ThisClass::OnAskCbFormatName)) },

#define ON_WM_CHANGECBCHAIN() \
	{ WM_CHANGECBCHAIN, 0, 0, 0, AfxSig_vhh, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(HWND, HWND) > ( &ThisClass::OnChangeCbChain)) },

#define ON_WM_HSCROLLCLIPBOARD() \
	{ WM_HSCROLLCLIPBOARD, 0, 0, 0, AfxSig_vWww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*, unsigned int, unsigned int) > ( &ThisClass::OnHScrollClipboard)) },

#define ON_WM_CLIPBOARDUPDATE() \
	{ WM_CLIPBOARDUPDATE, 0, 0, 0, AfxSig_v_v_v, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnClipboardUpdate)) },

#define ON_WM_QUERYNEWPALETTE() \
	{ WM_QUERYNEWPALETTE, 0, 0, 0, AfxSig_bv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnQueryNewPalette)) },

#define ON_WM_PALETTECHANGED() \
	{ WM_PALETTECHANGED, 0, 0, 0, AfxSig_vW, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*) > ( &ThisClass::OnPaletteChanged)) },

#define ON_WM_PALETTEISCHANGING() \
	{ WM_PALETTEISCHANGING, 0, 0, 0, AfxSig_vW, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*) > ( &ThisClass::OnPaletteIsChanging)) },

#define ON_WM_DROPFILES() \
	{ WM_DROPFILES, 0, 0, 0, AfxSig_vh, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(HDROP) > ( &ThisClass::OnDropFiles)) },

#define ON_WM_WINDOWPOSCHANGING() \
	{ WM_WINDOWPOSCHANGING, 0, 0, 0, AfxSig_vPOS, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(WINDOWPOS*) > ( &ThisClass::OnWindowPosChanging)) },

#define ON_WM_WINDOWPOSCHANGED() \
	{ WM_WINDOWPOSCHANGED, 0, 0, 0, AfxSig_vPOS, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(WINDOWPOS*) > ( &ThisClass::OnWindowPosChanged)) },

#define ON_WM_EXITMENULOOP() \
	{ WM_EXITMENULOOP, 0, 0, 0, AfxSig_vb, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool) > ( &ThisClass::OnExitMenuLoop)) },

#define ON_WM_ENTERMENULOOP() \
	{ WM_ENTERMENULOOP, 0, 0, 0, AfxSig_vb, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool) > ( &ThisClass::OnEnterMenuLoop)) },

#define ON_WM_STYLECHANGED() \
	{ WM_STYLECHANGED, 0, 0, 0, AfxSig_viSS, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(int, LPSTYLESTRUCT) > ( &ThisClass::OnStyleChanged)) },

#define ON_WM_STYLECHANGING() \
	{ WM_STYLECHANGING, 0, 0, 0, AfxSig_viSS, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(int, LPSTYLESTRUCT) > ( &ThisClass::OnStyleChanging)) },

#define ON_WM_SIZING() \
	{ WM_SIZING, 0, 0, 0, AfxSig_vwSIZING, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, LPRECT) > ( &ThisClass::OnSizing)) },

#define ON_WM_MOVING() \
	{ WM_MOVING, 0, 0, 0, AfxSig_vwMOVING, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, LPRECT) > ( &ThisClass::OnMoving)) },

#define ON_WM_ENTERSIZEMOVE() \
	{ WM_ENTERSIZEMOVE, 0, 0, 0, AfxSig_v_v_v, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnEnterSizeMove)) },

#define ON_WM_EXITSIZEMOVE() \
	{ WM_EXITSIZEMOVE, 0, 0, 0, AfxSig_v_v_v, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnExitSizeMove)) },

#define ON_WM_CAPTURECHANGED() \
	{ WM_CAPTURECHANGED, 0, 0, 0, AfxSig_vW2, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(CWnd*) > ( &ThisClass::OnCaptureChanged)) },

#define ON_WM_DEVICECHANGE() \
	{ WM_DEVICECHANGE, 0, 0, 0, AfxSig_bwl, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)(unsigned int, uintptr) > ( &ThisClass::OnDeviceChange)) },

#define ON_WM_POWERBROADCAST() \
	{ WM_POWERBROADCAST, 0, 0, 0, AfxSig_u_u_l, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< unsigned int (AFX_MSG_CALL CWnd::*)(unsigned int, LPARAM) > ( &ThisClass::OnPowerBroadcast)) },

#define ON_WM_USERCHANGED() \
	{ WM_USERCHANGED, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnUserChanged)) },

#define ON_WM_CHANGEUISTATE() \
	{ WM_CHANGEUISTATE, 0, 0, 0, AfxSig_vww2, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::OnChangeUIState)) },

#define ON_WM_UPDATEUISTATE() \
	{ WM_UPDATEUISTATE, 0, 0, 0, AfxSig_vww2, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(unsigned int, unsigned int) > ( &ThisClass::OnUpdateUIState)) },

#define ON_WM_QUERYUISTATE() \
	{ WM_QUERYUISTATE, 0, 0, 0, AfxSig_wv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< unsigned int (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnQueryUIState)) },

#define ON_WM_THEMECHANGED() \
	{ WM_THEMECHANGED, 0, 0, 0, AfxSig_l, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnThemeChanged)) },

#define ON_WM_DWMCOMPOSITIONCHANGED() \
	{ WM_DWMCOMPOSITIONCHANGED, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnCompositionChanged)) },

#define ON_WM_DWMNCRENDERINGCHANGED() \
	{ WM_DWMNCRENDERINGCHANGED, 0, 0, 0, AfxSig_vb, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool) > ( &ThisClass::OnNcRenderingChanged)) },

#define ON_WM_DWMCOLORIZATIONCOLORCHANGED() \
	{ WM_DWMCOLORIZATIONCOLORCHANGED, 0, 0, 0, AfxSig_vww, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(uint32, bool) > ( &ThisClass::OnColorizationColorChanged)) },

#define ON_WM_DWMWINDOWMAXIMIZEDCHANGE() \
	{ WM_DWMWINDOWMAXIMIZEDCHANGE, 0, 0, 0, AfxSig_vb, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(bool) > ( &ThisClass::OnWindowMaximizedChange)) },

#define ON_WM_DWMSENDICONICTHUMBNAIL() \
	{ WM_DWMSENDICONICTHUMBNAIL, 0, 0, 0, AfxSig_b_v_ii, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)(int, int) > ( &ThisClass::OnSendIconicThumbnail)) },

#define ON_WM_DWMSENDICONICLIVEPREVIEWBITMAP() \
	{ WM_DWMSENDICONICLIVEPREVIEWBITMAP, 0, 0, 0, AfxSig_b_v_v, \
		(AFX_PMSG) (AFX_PMSGW) \
		(static_cast< bool (AFX_MSG_CALL CWnd::*)() > ( &ThisClass::OnSendIconicLivePreviewBitmap)) },

/////////////////////////////////////////////////////////////////////////////
// Message map tables for Control Notification messages

// Static control notification codes
#define ON_STN_CLICKED(id, memberFxn) \
	ON_CONTROL(STN_CLICKED, id, memberFxn)
#define ON_STN_DBLCLK(id, memberFxn) \
	ON_CONTROL(STN_DBLCLK, id, memberFxn)
#define ON_STN_ENABLE(id, memberFxn) \
	ON_CONTROL(STN_ENABLE, id, memberFxn)
#define ON_STN_DISABLE(id, memberFxn) \
	ON_CONTROL(STN_DISABLE, id, memberFxn)


// Edit Control Notification Codes
#define ON_EN_SETFOCUS(id, memberFxn) \
	ON_CONTROL(EN_SETFOCUS, id, memberFxn)
#define ON_EN_KILLFOCUS(id, memberFxn) \
	ON_CONTROL(EN_KILLFOCUS, id, memberFxn)
#define ON_EN_CHANGE(id, memberFxn) \
	ON_CONTROL(EN_CHANGE, id, memberFxn)
#define ON_EN_UPDATE(id, memberFxn) \
	ON_CONTROL(EN_UPDATE, id, memberFxn)
#define ON_EN_ERRSPACE(id, memberFxn) \
	ON_CONTROL(EN_ERRSPACE, id, memberFxn)
#define ON_EN_MAXTEXT(id, memberFxn) \
	ON_CONTROL(EN_MAXTEXT, id, memberFxn)
#define ON_EN_HSCROLL(id, memberFxn) \
	ON_CONTROL(EN_HSCROLL, id, memberFxn)
#define ON_EN_VSCROLL(id, memberFxn) \
	ON_CONTROL(EN_VSCROLL, id, memberFxn)

#define ON_EN_ALIGN_LTR_EC(id, memberFxn) \
	ON_CONTROL(EN_ALIGN_LTR_EC, id, memberFxn)
#define ON_EN_ALIGN_RTL_EC(id, memberFxn) \
	ON_CONTROL(EN_ALIGN_RTL_EC, id, memberFxn)

// Richedit Control Notification Codes
#define ON_EN_IMECHANGE(id, memberFxn) \
	ON_CONTROL(EN_IMECHANGE, id, memberFxn)
#define ON_EN_ALIGNLTR(id, memberFxn) \
	ON_CONTROL(EN_ALIGNLTR, id, memberFxn)
#define ON_EN_ALIGNRTL(id, memberFxn) \
	ON_CONTROL(EN_ALIGNRTL, id, memberFxn)

// Animation Control Notification Codes
#define ON_ACN_START(id, memberFxn) \
	ON_CONTROL(ACN_START, id, memberFxn)
#define ON_ACN_STOP(id, memberFxn) \
	ON_CONTROL(ACN_STOP, id, memberFxn)

// User Button Notification Codes
#define ON_BN_CLICKED(id, memberFxn) \
	ON_CONTROL(BN_CLICKED, id, memberFxn)
#define ON_BN_DOUBLECLICKED(id, memberFxn) \
	ON_CONTROL(BN_DOUBLECLICKED, id, memberFxn)
#define ON_BN_SETFOCUS(id, memberFxn) \
	ON_CONTROL(BN_SETFOCUS, id, memberFxn)
#define ON_BN_KILLFOCUS(id, memberFxn) \
	ON_CONTROL(BN_KILLFOCUS, id, memberFxn)

// old BS_USERBUTTON button notifications - obsolete in Win31
#define ON_BN_PAINT(id, memberFxn) \
	ON_CONTROL(BN_PAINT, id, memberFxn)
#define ON_BN_HILITE(id, memberFxn) \
	ON_CONTROL(BN_HILITE, id, memberFxn)
#define ON_BN_UNHILITE(id, memberFxn) \
	ON_CONTROL(BN_UNHILITE, id, memberFxn)
#define ON_BN_DISABLE(id, memberFxn) \
	ON_CONTROL(BN_DISABLE, id, memberFxn)

// Listbox Notification Codes
#define ON_LBN_ERRSPACE(id, memberFxn) \
	ON_CONTROL(LBN_ERRSPACE, id, memberFxn)
#define ON_LBN_SELCHANGE(id, memberFxn) \
	ON_CONTROL(LBN_SELCHANGE, id, memberFxn)
#define ON_LBN_DBLCLK(id, memberFxn) \
	ON_CONTROL(LBN_DBLCLK, id, memberFxn)
#define ON_LBN_SELCANCEL(id, memberFxn) \
	ON_CONTROL(LBN_SELCANCEL, id, memberFxn)
#define ON_LBN_SETFOCUS(id, memberFxn) \
	ON_CONTROL(LBN_SETFOCUS, id, memberFxn)
#define ON_LBN_KILLFOCUS(id, memberFxn) \
	ON_CONTROL(LBN_KILLFOCUS, id, memberFxn)

// Check Listbox Notification codes
#define CLBN_CHKCHANGE (40)
#define ON_CLBN_CHKCHANGE(id, memberFxn) \
	ON_CONTROL(CLBN_CHKCHANGE, id, memberFxn)

// Combo Box Notification Codes
#define ON_CBN_ERRSPACE(id, memberFxn) \
	ON_CONTROL(CBN_ERRSPACE, id, memberFxn)
#define ON_CBN_SELCHANGE(id, memberFxn) \
	ON_CONTROL(CBN_SELCHANGE, id, memberFxn)
#define ON_CBN_DBLCLK(id, memberFxn) \
	ON_CONTROL(CBN_DBLCLK, id, memberFxn)
#define ON_CBN_SETFOCUS(id, memberFxn) \
	ON_CONTROL(CBN_SETFOCUS, id, memberFxn)
#define ON_CBN_KILLFOCUS(id, memberFxn) \
	ON_CONTROL(CBN_KILLFOCUS, id, memberFxn)
#define ON_CBN_EDITCHANGE(id, memberFxn) \
	ON_CONTROL(CBN_EDITCHANGE, id, memberFxn)
#define ON_CBN_EDITUPDATE(id, memberFxn) \
	ON_CONTROL(CBN_EDITUPDATE, id, memberFxn)
#define ON_CBN_DROPDOWN(id, memberFxn) \
	ON_CONTROL(CBN_DROPDOWN, id, memberFxn)
#define ON_CBN_CLOSEUP(id, memberFxn)  \
	ON_CONTROL(CBN_CLOSEUP, id, memberFxn)
#define ON_CBN_SELENDOK(id, memberFxn)  \
	ON_CONTROL(CBN_SELENDOK, id, memberFxn)
#define ON_CBN_SELENDCANCEL(id, memberFxn)  \
	ON_CONTROL(CBN_SELENDCANCEL, id, memberFxn)

/////////////////////////////////////////////////////////////////////////////
// User extensions for message map entries

// for Windows messages
#define ON_MESSAGE(message, memberFxn) \
	{ message, 0, 0, 0, AfxSig_lwl, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)(WPARAM, LPARAM) > \
		 (&memberFxn)) },

// for Registered Windows messages
#define ON_REGISTERED_MESSAGE(nMessageVariable, memberFxn) \
	{ 0xC000, 0, 0, 0, (uintptr)(unsigned int*)(&nMessageVariable), \
		/*implied 'AfxSig_lwl'*/ \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)(WPARAM, LPARAM) > \
		 (memberFxn)) },

// for Thread messages
#define ON_THREAD_MESSAGE(message, memberFxn) \
	{ message, 0, 0, 0, AfxSig_vwl, \
		(AFX_PMSG)(AFX_PMSGT) \
		(static_cast< void (AFX_MSG_CALL CWinThread::*)(WPARAM, LPARAM) > \
		 (memberFxn)) },

// for Registered Windows messages
#define ON_REGISTERED_THREAD_MESSAGE(nMessageVariable, memberFxn) \
	{ 0xC000, 0, 0, 0, (uintptr)(unsigned int*)(&nMessageVariable), \
		/*implied 'AfxSig_vwl'*/ \
		(AFX_PMSG)(AFX_PMSGT) \
		(static_cast< void (AFX_MSG_CALL CWinThread::*)(WPARAM, LPARAM) > \
		 (memberFxn)) },

} // namespace MFC
} // namespace Bagel

#endif
