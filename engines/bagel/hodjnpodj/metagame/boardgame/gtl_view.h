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

#ifndef HODJNPODJ_METAGAME_BGEN_GTL_VIEW_H
#define HODJNPODJ_METAGAME_BGEN_GTL_VIEW_H

#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/metagame/bgen/bs_util.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {


class CGtlView : public View {
	friend class CGtlData;

	// Attributes
public:
	CBsuSet m_cViewBsuSet;				// boffo scroll bar set for windows

	Common::Point m_cCurrentPosition;	// current mouse position
	bool m_bBsuInit = false;
#ifdef TODO
	CGtlFrame *m_xpFrame = false;	// pointer to frame window
#endif

private:
	//- OnInitialUpdate -- 
	void OnInitialUpdate(void);

protected:
	//- PreCreateWindow -- change view window style
	//bool PreCreateWindow(CREATESTRUCT &cCs);

public:
	CGtlView() : View("GtlView") {}
	virtual ~CGtlView();

	void FlushInputEvents(void);

	class CGtlDoc *GetDocument();
	void UpdateDialogs(void);
	void OnSoundNotify(CBofSound *);

	//- OnUpdate -- called when document changes to update view
	virtual void OnUpdate(CView *xpSender, int lHint, CObject *xpHint);
	//- OnDraw -- draw current view
	virtual void OnDraw(CDC *xpDc);

	// Generated message map functions
#ifdef TODO
	//{{AFX_MSG(CGtlView)
	void OnKeyDown(uint nChar, uint nRepCnt, uint nFlags);
	void OnSysKeyDown(uint nChar, uint nRepCnt, uint nFlags);
	void OnKeyUp(uint nChar, uint nRepCnt, uint nFlags);
	void OnLButtonDblClk(uint nFlags, CPoint point);
	void OnLButtonDown(uint nFlags, CPoint point);
	void OnLButtonUp(uint nFlags, CPoint point);
	void OnMouseMove(uint nFlags, CPoint point);
	void OnRButtonDblClk(uint nFlags, CPoint point);
	void OnRButtonDown(uint nFlags, CPoint point);
	void OnRButtonUp(uint nFlags, CPoint point);
	void OnChar(uint nChar, uint nRepCnt, uint nFlags);
	void OnSize(uint nType, int cx, int cy);
	bool OnEraseBkgnd(CDC *pDC);
	void OnSetFocus(CWnd *pOldWnd);
	void OnHScroll(uint nSBCode, uint nPos, CScrollBar *pScrollBar);
	void OnVScroll(uint nSBCode, uint nPos, CScrollBar *pScrollBar);
	void OnDestroy();
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	long OnMCINotify(WPARAM, LPARAM);
	long OnMMIONotify(WPARAM, LPARAM);
	void OnTimer(uint);
	bool OnSetCursor(CWnd *pWnd, uint nHitTest, uint message);
#endif
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
