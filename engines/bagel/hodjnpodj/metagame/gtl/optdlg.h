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

#ifndef BAGEL_METAGAME_GTL_OPTDLG_H
#define BAGEL_METAGAME_GTL_OPTDLG_H

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Word Search
//////////////////////////////////////////////////////////////////////////////
class CMetaOptDlg : public CBmpDialog {
private:
	CWnd         *m_pParent;
	CPalette     *m_pPalette;
	CBfcMgr      *m_pBfcMgr;
	CColorButton *m_pRulesGameButton;
	CColorButton *m_pSaveGameButton;
	CColorButton *m_pLeaveGameButton;
	CColorButton *m_pContinueGameButton;
	CColorButton *m_pOptionsGameButton;

	bool         m_bMusic;
	bool         m_bSoundFX;
	bool         m_bScrolling;

public:
	CMetaOptDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr);
	void SetInitialOptions(CBfcMgr *);    // Sets the private members

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

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
