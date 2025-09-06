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

#ifndef BAGEL_METAGAME_GTL_GTLVIEW_H
#define BAGEL_METAGAME_GTL_GTLVIEW_H

#include "bagel/hodjnpodj/metagame/gtl/gtl.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldat.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define SCROLLVIEW 0    /* 0=CView, 1=CScrollView */
#define VK_H    0x0048
#define VK_I    0x0049
#define VK_M    0x004D
#define VK_P    0x0050

#if SCROLLVIEW
	#define MFC_VIEW CScrollView
#else
	#define MFC_VIEW CView
#endif

class CGtlView : public MFC_VIEW {
	friend class CGtlData ;
protected: // create from serialization only
	CGtlView();
	DECLARE_DYNCREATE(CGtlView)

public:
	CBsuSet m_cViewBsuSet ; // boffo scroll bar set for windows

	char m_cStartData ;
	CPoint m_cCurrentPosition ; // current mouse position
	bool m_bBsuInit ;
	class CGtlFrame *m_xpFrame ;    // pointer to frame window
	char m_cEndData ;

	class CGtlDoc *GetDocument();
	void UpdateDialogs() ;
	void OnSoundNotify(CSound *);

	//- OnUpdate -- called when document changes to update view
	void OnUpdate(CView *xpSender, LPARAM lHint, CObject *xpHint) override;
	//- OnDraw -- draw current view
	void OnDraw(CDC* xpDc) override;

private:
	//- OnInitialUpdate --
	void OnInitialUpdate();

protected:
	//- PreCreateWindow -- change view window style
	bool PreCreateWindow(CREATESTRUCT& cCs) override;

// Implementation
public:
	virtual ~CGtlView();

	void FlushInputEvents();


// Generated message map functions
protected:
	//{{AFX_MSG(CGtlView)
	afx_msg void OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnKeyUp(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnLButtonDblClk(unsigned int nFlags, CPoint point);
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnLButtonUp(unsigned int nFlags, CPoint point);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(unsigned int nFlags, CPoint point);
	afx_msg void OnRButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnRButtonUp(unsigned int nFlags, CPoint point);
	afx_msg void OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSize(unsigned int nType, int cx, int cy);
	afx_msg bool OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	afx_msg void OnTimer(uintptr);
	afx_msg bool OnSetCursor(CWnd *pWnd, unsigned int nHitTest, unsigned int message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

inline CGtlDoc *CGtlView::GetDocument() {
	return (CGtlDoc*)m_pDocument;
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
