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

#ifndef HODJNPODJ_LIBS_MAIN_MENU_H
#define HODJNPODJ_LIBS_MAIN_MENU_H

#include "bagel/hodjnpodj/boflib/dialog.h"
#include "bagel/hodjnpodj/libs/types.h"

namespace Bagel {
namespace HodjNPodj {

#define NO_RULES    0x0001
#define NO_NEWGAME  0x0002
#define NO_OPTIONS  0x0004
#define NO_RETURN   0x0008
#define NO_QUIT     0x0010
#define NO_AUDIO    0x0020

#define IDC_OPTIONS_ARROWUP	503
#define IDC_OPTIONS_ARROWDN	504

#define	ID_OPTIONS_CODES	450

// obsolete
//#define IDR_BITMAP_SCROLL   451

#define IDR_OPTION_SCROLL	452
#define IDD_OPTIONS_DIALOG	453

#define IDC_OPTIONS_RETURN	454
#define IDC_OPTIONS_QUIT	455
#define IDC_OPTIONS_RULES	456
#define IDC_OPTIONS_NEWGAME	457
#define IDC_OPTIONS_OPTIONS	458
#define IDC_OPTIONS_AUDIO	459

#define IDB_SCROLBTN        460
#define SCROLLUP            461
#define SCROLLDOWN          462

#define IDD_AUDIOCFG		463

typedef void (*FPFUNC)(CWnd *parentWnd);

class CMainMenu : public CBmpDialog {
public:
	CMainMenu(CWnd *pParent, CPalette *pPalette, UINT, FPFUNC, const char *,
		const char *pWavFileName = nullptr, LPGAMESTRUCT pGameParams = nullptr);
	void ClearDialogImage();

private:


	// Dialog Data
		//{{AFX_DATA(CMainMenu)
// IDD_OPTIONS_DIALOG DIALOG DISCARDABLE  0, 0, 131, 110
//	enum { IDD = IDD_OPTIONS_DIALOG };
	// NOTE: the ClassWizard will add data members here
//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CMainMenu)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnClickedRules();
	afx_msg void OnClickedNewgame();
	afx_msg void OnClickedOptions();
	afx_msg void OnClickedAudio();
	afx_msg void OnClickedReturn();
	afx_msg void OnClickedQuit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	FPFUNC      	m_pOptionsFunction;
	char *m_pRulesFileName;
	char *m_pWavFileName;
	int         	m_nFlags;
	LPGAMESTRUCT	m_pGameParams;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
