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

#ifndef HODJNPODJ_PEGGLE_OPTIONS_H
#define HODJNPODJ_PEGGLE_OPTIONS_H

#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/peggle/optres.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

class COptions : public CDialog {
// Construction
public:
	COptions(CWnd* pParent, CPalette *pPalette, int iDlgId);

	void ClearDialogImage();
	void RefreshBackground();

	int     m_iDlgId ;  // dialog id
	CWnd    *m_pDlgParentWnd;
	CBitmap *m_pDlgBackground;

private:
	CRect OptionsRect, OkayRect;

	// Dialog Data
	//{{AFX_DATA(COptions)
	enum { IDD = IDD_OPTIONS_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange *pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptions)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual bool OnInitDialog() override;
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	virtual void OnOK() override;
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(bool bShow, unsigned int nStatus);
	afx_msg void OnSize(unsigned int nType, int cx, int cy);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnClickedRules() ;
	afx_msg void OnClickedNewgame() ;
	afx_msg void OnClickedOptions() ;
	afx_msg void OnClickedAudio() ;
	afx_msg void OnClickedReturn();
	afx_msg void OnClickedQuit();
	afx_msg void OnDestroy();
//	afx_msg void OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel

#endif
