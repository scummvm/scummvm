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

#include "bagel/hodjnpodj/metagame/grand_tour/gtl.h"
#include "bagel/hodjnpodj/metagame/grand_tour/gtldoc.h"
#include "bagel/hodjnpodj/metagame/grand_tour/gtldat.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

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
	BOOL m_bBsuInit ;
	class CGtlFrame *m_xpFrame ;    // pointer to frame window
	char m_cEndData ;

	class CGtlDoc *GetDocument();
	void UpdateDialogs(void) ;
	void OnSoundNotify(CSound *);

	//- OnUpdate -- called when document changes to update view
	virtual void OnUpdate(CView *xpSender, LPARAM lHint, CObject * xpHint);
	//- OnDraw -- draw current view
	virtual void OnDraw(CDC* xpDc);

private:
	//- OnInitialUpdate --
	void OnInitialUpdate(void);

#ifdef NODEEDIT
	//- CheckSize -- check window size, adjust if necessary
	BOOL CheckSize(void);
#endif

protected:
	//- PreCreateWindow -- change view window style
	BOOL PreCreateWindow(CREATESTRUCT& cCs);

// Implementation
public:
	virtual ~CGtlView();

#ifdef BAGEL_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void FlushInputEvents(void);


// Generated message map functions
protected:
	//{{AFX_MSG(CGtlView)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg long OnMCINotify(WPARAM, LPARAM);
	afx_msg long OnMMIONotify(WPARAM, LPARAM);
	afx_msg void OnTimer(UINT);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef BAGEL_DEBUG  // debug version in gtlview.cpp
inline CGtlDoc *CGtlView::GetDocument() {
	return (CGtlDoc*)m_pDocument;
}
#endif

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
