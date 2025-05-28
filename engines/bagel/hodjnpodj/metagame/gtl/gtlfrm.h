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

#ifndef BAGEL_METAGAME_GTL_GTLFRM_H
#define BAGEL_METAGAME_GTL_GTLFRM_H

#include "bagel/hodjnpodj/metagame/gtl/gtl.h"
#include "bagel/hodjnpodj/metagame/gtl/infdlg.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/hodjnpodj/metagame/bgen/note.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#if GTLMDI
	#define MFC_FRAME CMDIFrameWnd
#else
	#define MFC_FRAME CFrameWnd
#endif

class CGtlFrame : public MFC_FRAME {
	#if GTLMDI
	DECLARE_DYNAMIC(CGtlFrame)
	#else
	DECLARE_DYNCREATE(CGtlFrame)
	#endif

public:
	CGtlFrame();

private:
	void ProcessMiniGameReturn();

public:
	void ShowInventory(CPalette *, int = 1);
	void ShowOptions(CPalette *);
	BOOL ShowClue(CPalette *, CNote *);

	class CGtlView *m_xpcLastFocusView ;        // last view with focus
	class CGtlView *m_xpcLastMouseView ;        // last view touched by mouse
	class CGtlDoc *m_xpDocument ;
	class CBfcMgr *m_lpBfcMgr ;         // interface class
	BOOL m_bExitDll ;           // if turned on, exit meta game DLL
	int         m_nReturnCode;

	virtual ~CGtlFrame();
	#ifdef BAGEL_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	#endif


// Generated message map functions
protected:
	//{{AFX_MSG(CGtlFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	#if GTLDLL
	afx_msg void OnDestroy();
	#endif
	afx_msg void OnUpdateViewStatusBar(CCmdUI *pCmdUI);
	afx_msg void OnViewInfoDlg();
	afx_msg void OnViewCtlDlg();
	afx_msg void OnViewNodeDlg();
	afx_msg void OnViewMenuDlg();
	afx_msg void OnCallNew();
	afx_msg void OnCallOpen();
	afx_msg void OnCallClose();
	afx_msg void OnCallSave();
	afx_msg void OnCallExit();
	afx_msg BOOL OnEraseBkgnd(CDC *);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnSysCommand(UINT, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// gtlfrm.cpp : implementation of the CGtlFrame class
public:
	//- NewFrame -- set pointer to interface manager in frame
	BOOL NewFrame(CBfcMgr *lpBfcMgr PDFT(nullptr)) ;
	BOOL ShowControlDialog(void) ;
	BOOL ShowNodeDialog(void) ;
	BOOL ShowInfoDialog(void) ;
	BOOL ShowMenuDialog(void) ;
	//- GetCurrentDocAndView -- get last focused doc/view
	BOOL GetCurrentDocAndView(CGtlDoc * &xpcGtlDoc,
	                          CGtlView *&xpcGtlFocusView, CGtlView *&xpcGtlMouseView) ;

protected:
	//- PreCreateWindow -- change frame window style
	BOOL PreCreateWindow(CREATESTRUCT &cCs) override;

public:
	//- RecalcLayout -- override CFrameWnd::RecalcLayout
	virtual void RecalcLayout(BOOL bNotify PDFT(TRUE)) ;
};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
