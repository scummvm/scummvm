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

#ifndef BAGEL_METAGAME_FRAME_DIALOGS_H
#define BAGEL_METAGAME_FRAME_DIALOGS_H

#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/metagame/frame/hodjpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

#define BUTTONSBMP      ".\\ART\\NEWBTNS.BMP"

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Word Search
//////////////////////////////////////////////////////////////////////////////
class CMainGameDlg : public CBmpDialog {
private:

public:
	CMainGameDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr);

private:
	bool OnInitDialog() override;
	void ClearDialogImage();

protected:
	//{{AFX_MSG(COptions)
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual bool OnCommand(WPARAM, LPARAM) override;
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// data
	#ifndef BMP_BUTTONS
	CColorButton    *m_pPlayMetaButton;
	CColorButton    *m_pPlayMiniButton;
	CColorButton    *m_pRestoreButton;
	CColorButton    *m_pGrandTourButton;
	CColorButton    *m_pRestartMovieButton;
	CColorButton    *m_pQuitButton;
	#else
	CBmpButton  *m_pPlayMetaButton;
	CBmpButton  *m_pPlayMiniButton;
	CBmpButton  *m_pRestoreButton;
	CBmpButton  *m_pGrandTourButton;
	CBmpButton  *m_pRestartMovieButton;
	CBmpButton  *m_pQuitButton;
	CBitmap     *m_pButtonsBmp;
	CPalette    *m_pButtonPalette;
	#endif
};

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Word Search
//////////////////////////////////////////////////////////////////////////////
class CMetaSetupDlg : public CBmpDialog {
private:
	CBfcMgr     *m_lpMetaGameStruct;
	int         m_nHodjSkillLevel;
	int         m_nPodjSkillLevel;
	int         m_nGameTime;
	bool        m_bPodjIsComputer;

public:
	CMetaSetupDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr);
	void SetInitialOptions(CBfcMgr *);  // Sets the private members

private:
	bool OnInitDialog() override;
	void ClearDialogImage();

protected:
	//{{AFX_MSG(COptions)
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual bool OnCommand(WPARAM, LPARAM) override;
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
