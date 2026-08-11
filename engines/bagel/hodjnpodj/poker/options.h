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

#ifndef HODJNPODJ_POKER_OPTIONS_H
#define HODJNPODJ_POKER_OPTIONS_H

#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/poker/poker.h"
#include "bagel/hodjnpodj/poker/optres.h"

namespace Bagel {
namespace HodjNPodj {
namespace Poker {

class COptions : public CDialog {
private:

	int m_iDlgId;

// Construction
public:
//	COptions(CWnd* pParent = nullptr, CPalette *pPalette = nullptr);   // standard constructor
	COptions(CWnd* pParent, CPalette *pPalette, int iDlgId) ;
// #include "options.inc"       // put in your own include file

private:


// Dialog Data
	//{{AFX_DATA(COptions)
	enum { IDD = IDD_OPTIONS_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	void RefreshBackground();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void OnCancel();
	// Generated message map functions
	//{{AFX_MSG(COptions)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual bool OnInitDialog();
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	virtual void OnOK();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(bool bShow, unsigned int nStatus);
	afx_msg void OnSize(unsigned int nType, int cx, int cy);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnClickedRules() ;
	afx_msg void OnClickedNewgame() ;
	afx_msg void OnClickedOptions() ;
	afx_msg void OnClickedReturn();
	afx_msg void OnClickedQuit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CFont   *m_pFont;
	CBitmap *m_pDlgBackground;
};

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel

#endif
