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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/audiocfg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/menures.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/metagame/gtl/resource.h"
#include "bagel/hodjnpodj/metagame/gtl/optdlg.h"
#include "bagel/hodjnpodj/metagame/saves/savegame.h"
#include "bagel/hodjnpodj/metagame/bgen/c2btndlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

extern HWND     ghwndParent;

/*****************************************************************
 *
 * CMetaOptDlg
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Constructor sends the input to the CBmpDialog constructor and
 *      the intializes the private members
 *
 * FORMAL PARAMETERS:
 *
 *      Those needed to contruct a CBmpDialog dialog: pParent,pPalette, nID
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      private member m_nCurrenLEVEL
 *  globals     rectDisplayAmount and pSeLEVELPalette
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
CMetaOptDlg::CMetaOptDlg(CWnd *pParent, CPalette *pPalette) :  CBmpDialog(pParent, pPalette, IDD_OPTIONDLG, ".\\ART\\MLSCROLL.BMP") {
	m_pParent = pParent;
	m_pPalette = pPalette;
	m_pBfcMgr = nullptr;
	m_pSaveGameButton = nullptr;
	m_pOptionsGameButton = nullptr;
	m_pLeaveGameButton = nullptr;
	m_pContinueGameButton = nullptr;
	m_pRulesGameButton = nullptr;
}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the "Set" and "Cancel" buttons
 *
 * This function is called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
 *
 * FORMAL PARAMETERS:
 *
 *      wParam          identifier for the button to be processed
 *      lParam          type of message to be processed
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
bool CMetaOptDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
	// What ever button is clicked, end the dialog and send the ID of the button
	// clicked as the return from the dialog

	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case IDC_SAVE_GAME:
			Saves::SaveGame((CWnd *)this, m_pPalette, nullptr);
			ClearDialogImage();
			EndDialog(0);
			return true;

		case IDC_LEAVE_GAME:
			if (m_pBfcMgr->m_bChanged) {
				C2ButtonDialog dlg2Button(this, m_pPalette, "&Yes", "&No", "Would you like to", "save this game", "before leaving?");
				if (dlg2Button.DoModal() == CBUTTON1)
					Saves::SaveGame((CWnd *)this, m_pPalette, nullptr);
			}
			ClearDialogImage();
			EndDialog(1);
			return true;

		case IDC_OPTIONS: {
			CAudioCfgDlg dlgAudioCfg(this, m_pPalette, IDD_AUDIOCFG);
			m_bMusic = GetPrivateProfileInt("Meta", "Music", true, "HODJPODJ.INI");
			m_bSoundFX = GetPrivateProfileInt("Meta", "SoundEffects", true, "HODJPODJ.INI");
		}
		return true;

		case IDC_RULES: {
			CRules RulesDlg(this, "metarule.txt", m_pPalette, nullptr);
			RulesDlg.DoModal();
			return true;
		}

		case IDC_CONTINUE:
			m_pBfcMgr->m_stGameStruct.bMusicEnabled = m_bMusic;
			m_pBfcMgr->m_stGameStruct.bSoundEffectsEnabled = m_bSoundFX;
			m_pBfcMgr->m_bScrolling = m_bScrolling;
			ClearDialogImage();
			EndDialog(0);
			return true;
		}
	}
	return CBmpDialog::OnCommand(wParam, lParam);
}


void CMetaOptDlg::OnOK() {
	if (m_pSaveGameButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_SAVE_GAME, (LPARAM)BN_CLICKED);
		return;
	}
	if (m_pLeaveGameButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_LEAVE_GAME, (LPARAM)BN_CLICKED);
		return;
	}
	if (m_pContinueGameButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_CONTINUE, (LPARAM)BN_CLICKED);
		return;
	}
	if (m_pOptionsGameButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_OPTIONS, (LPARAM)BN_CLICKED);
		return;
	}
	if (m_pRulesGameButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_RULES, (LPARAM)BN_CLICKED);
		return;
	}
}

void CMetaOptDlg::OnCancel() {
	ClearDialogImage();
	EndDialog(0);
}

void CMetaOptDlg::ClearDialogImage() {
	if (m_pSaveGameButton != nullptr) {
		delete m_pSaveGameButton;
		m_pSaveGameButton = nullptr;
	}
	if (m_pLeaveGameButton != nullptr) {
		delete m_pLeaveGameButton;
		m_pLeaveGameButton = nullptr;
	}
	if (m_pContinueGameButton != nullptr) {
		delete m_pContinueGameButton;
		m_pContinueGameButton = nullptr;
	}
	if (m_pOptionsGameButton != nullptr) {
		delete m_pOptionsGameButton;
		m_pOptionsGameButton = nullptr;
	}
	if (m_pRulesGameButton != nullptr) {
		delete m_pRulesGameButton;
		m_pRulesGameButton = nullptr;
	}

	ValidateRect(nullptr);
}
/*****************************************************************
 *
 * SetInitialOptions
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This sets the privates to the inputted values
 *
 * FORMAL PARAMETERS:
 *
 *      lCurrenLEVEL = the current amount the user has
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      m_nCurrenLEVEL = (int)min( AMOUNTMAX, lCurrenLEVEL)
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *
 ****************************************************************/
void CMetaOptDlg::SetInitialOptions(CBfcMgr *pMetaGameStruct) {
	m_pBfcMgr = pMetaGameStruct;
	m_bMusic = m_pBfcMgr->m_stGameStruct.bMusicEnabled;
	m_bSoundFX = m_pBfcMgr->m_stGameStruct.bSoundEffectsEnabled;
	m_bScrolling = m_pBfcMgr->m_bScrolling;
}


bool CMetaOptDlg::OnInitDialog() {
	bool    bSuccess;

	CBmpDialog::OnInitDialog();

	m_pSaveGameButton = new CColorButton();
	ASSERT(m_pSaveGameButton != nullptr);
	m_pSaveGameButton->SetPalette(m_pPalette);
	bSuccess = m_pSaveGameButton->SetControl(IDC_SAVE_GAME, this);
	ASSERT(bSuccess);

	m_pLeaveGameButton = new CColorButton();
	ASSERT(m_pLeaveGameButton != nullptr);
	m_pLeaveGameButton->SetPalette(m_pPalette);
	bSuccess = m_pLeaveGameButton->SetControl(IDC_LEAVE_GAME, this);
	ASSERT(bSuccess);

	m_pContinueGameButton = new CColorButton();
	ASSERT(m_pContinueGameButton != nullptr);
	m_pContinueGameButton->SetPalette(m_pPalette);
	bSuccess = m_pContinueGameButton->SetControl(IDC_CONTINUE, this);
	ASSERT(bSuccess);

	m_pOptionsGameButton = new CColorButton();
	ASSERT(m_pOptionsGameButton != nullptr);
	m_pOptionsGameButton->SetPalette(m_pPalette);
	bSuccess = m_pOptionsGameButton->SetControl(IDC_OPTIONS, this);
	ASSERT(bSuccess);

	m_pRulesGameButton = new CColorButton();
	ASSERT(m_pRulesGameButton != nullptr);
	m_pRulesGameButton->SetPalette(m_pPalette);
	bSuccess = m_pRulesGameButton->SetControl(IDC_RULES, this);
	ASSERT(bSuccess);
//    m_pRulesGameButton->EnableWindow(false);

	return true;
}

/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.
 *
 * This uses the CBmpDialog Paint as its base, and displays the current
 * amount chosen from the scrollbar
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CMetaOptDlg::OnPaint() {
	PAINTSTRUCT lpPaint;

	CBmpDialog::OnPaint();

	Invalidate(false);
	BeginPaint(&lpPaint);

	EndPaint(&lpPaint);
}

void CMetaOptDlg::OnDestroy() {
	if (m_pSaveGameButton != nullptr) {
		delete m_pSaveGameButton;
		m_pSaveGameButton = nullptr;
	}
	if (m_pLeaveGameButton != nullptr) {
		delete m_pLeaveGameButton;
		m_pLeaveGameButton = nullptr;
	}
	if (m_pContinueGameButton != nullptr) {
		delete m_pContinueGameButton;
		m_pContinueGameButton = nullptr;
	}
	if (m_pOptionsGameButton != nullptr) {
		delete m_pOptionsGameButton;
		m_pOptionsGameButton = nullptr;
	}
	if (m_pRulesGameButton != nullptr) {
		delete m_pRulesGameButton;
		m_pRulesGameButton = nullptr;
	}

	CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CMetaOptDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
