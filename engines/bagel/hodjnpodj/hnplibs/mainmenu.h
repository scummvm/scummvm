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

#ifndef HODJNPODJ_HNPLIBS_MAINMENU_H
#define HODJNPODJ_HNPLIBS_MAINMENU_H

#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/afxext.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/menures.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"

namespace Bagel {
namespace HodjNPodj {

typedef void (CALLBACK *FPFUNC)(CWnd *);

#define NO_RULES    0x0001
#define NO_NEWGAME  0x0002
#define NO_OPTIONS  0x0004
#define NO_RETURN   0x0008
#define NO_QUIT     0x0010
#define NO_AUDIO    0x0020

class CMainMenu : public CBmpDialog {
public:
	CMainMenu(CWnd *, CPalette *, unsigned int, FPFUNC,
	          const char *, const char *pWavFileName = nullptr,
	          LPGAMESTRUCT pGameParams = nullptr);
	void ClearDialogImage();

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CMainMenu)
	virtual bool OnInitDialog() override;
	virtual void OnOK() override;
	virtual void OnCancel() override;
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnClickedRules() ;
	afx_msg void OnClickedNewgame() ;
	afx_msg void OnClickedOptions() ;
	afx_msg void OnClickedAudio() ;
	afx_msg void OnClickedReturn();
	afx_msg void OnClickedQuit();
	afx_msg void OnClickedHype();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	FPFUNC _optionsFunction = nullptr;
	const char *_rulesFilename = nullptr;
	const char *_wavFilename = nullptr;
	uint _flags = 0;
	LPGAMESTRUCT _gameParams = nullptr;

private:
	CColorButton *_hypeButton = nullptr;
	CColorButton *_rulesButton = nullptr;
	CColorButton *_newGameButton = nullptr;
	CColorButton *_optionsButton = nullptr;
	CColorButton *_audioButton = nullptr;
	CColorButton *_returnButton = nullptr;
	CColorButton *_quitButton = nullptr;
	CFont _hypeFont;

	//{{AFX_DATA(CMainMenu)
	enum {
		IDD = IDD_OPTIONS_DIALOG
	};
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void clearButtons();
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
