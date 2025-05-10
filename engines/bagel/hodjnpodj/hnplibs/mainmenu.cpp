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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/dibapi.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/audiocfg.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
//include <mmsystem.h>

namespace Bagel {
namespace HodjNPodj {

static	CColorButton *pRulesButton = NULL;
static	CColorButton *pNewGameButton = NULL;
static	CColorButton *pOptionsButton = NULL;
static	CColorButton *pAudioButton = NULL;
static	CColorButton *pReturnButton = NULL;
static	CColorButton *pQuitButton = NULL;

CMainMenu::CMainMenu(CWnd *pParent, CPalette *pPalette,
		UINT nFlags, FPFUNC pOptionsFunc, const char *pRulesFileName,
		const char *pWavFileName, LPGAMESTRUCT pGameParams)
        : CBmpDialog(pParent, pPalette, IDD_OPTIONS_DIALOG, ".\\ART\\OSCROLL.BMP") {
    // Can't access null pointers
    assert(pParent != NULL);
    assert(pPalette != NULL);
    assert(pRulesFileName != NULL);

    // Inits
    m_pRulesFileName = pRulesFileName;
    m_pWavFileName = pWavFileName;
    m_nFlags = nFlags;
    m_pGameParams = pGameParams;

    if (!(m_nFlags & NO_OPTIONS)) {
        assert(pOptionsFunc != NULL);
    }
    m_pOptionsFunction = pOptionsFunc;

    //{{AFX_DATA_INIT(CMainMenu)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void CMainMenu::DoDataExchange(CDataExchange* pDX)
{
    CBmpDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMainMenu)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}


void CMainMenu::ClearDialogImage(void)
{
	if (pRulesButton != NULL) {                          // release the button
		delete pRulesButton;
		pRulesButton = NULL;
	}

	if (pNewGameButton != NULL) {                          // release the button
		delete pNewGameButton;
		pNewGameButton = NULL;
	}

	if (pOptionsButton != NULL) {                          // release the button
		delete pOptionsButton;
		pOptionsButton = NULL;
	}

	if (pAudioButton != NULL) {                          // release the button
		delete pAudioButton;
		pAudioButton = NULL;
	}

	if (pReturnButton != NULL) {                          // release the button
		delete pReturnButton;
		pReturnButton = NULL;
	}

	if (pQuitButton != NULL) {                          // release the button
		delete pQuitButton;
		pQuitButton = NULL;
	}

	if (m_pDlgBackground != NULL)
		InvalidateRect(NULL,FALSE);
}

void CMainMenu::OnDestroy()
{
    CBmpDialog::OnDestroy();

	if (pRulesButton != NULL) {                          // release the button
		delete pRulesButton;
		pRulesButton = NULL;
	}

	if (pNewGameButton != NULL) {                          // release the button
		delete pNewGameButton;
		pNewGameButton = NULL;
	}

	if (pOptionsButton != NULL) {                          // release the button
		delete pOptionsButton;
		pOptionsButton = NULL;
	}

	if (pAudioButton != NULL) {                          // release the button
		delete pAudioButton;
		pAudioButton = NULL;
	}

	if (pReturnButton != NULL) {                          // release the button
		delete pReturnButton;
		pReturnButton = NULL;
	}

	if (pQuitButton != NULL) {                          // release the button
		delete pQuitButton;
		pQuitButton = NULL;
	}

}

BOOL CMainMenu::OnInitDialog()
{
    CWnd *pWndTemp;

    CBmpDialog::OnInitDialog();            // do basic dialog initialization

	if ((pRulesButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pRulesButton).SetPalette(m_pPalette);						// set the palette to use
		(*pRulesButton).SetControl(IDC_OPTIONS_RULES,this);				// tie to the dialog control
	}
	
	if ((pNewGameButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pNewGameButton).SetPalette(m_pPalette);						// set the palette to use
		(*pNewGameButton).SetControl(IDC_OPTIONS_NEWGAME,this);				// tie to the dialog control
	}
	
	if ((pOptionsButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pOptionsButton).SetPalette(m_pPalette);						// set the palette to use
		(*pOptionsButton).SetControl(IDC_OPTIONS_OPTIONS,this);				// tie to the dialog control
	}
	
	if ((pAudioButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pAudioButton).SetPalette(m_pPalette);						// set the palette to use
		(*pAudioButton).SetControl(IDC_OPTIONS_AUDIO,this);				// tie to the dialog control
	}
	
	if ((pReturnButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pReturnButton).SetPalette(m_pPalette);						// set the palette to use
		(*pReturnButton).SetControl(IDC_OPTIONS_RETURN,this);				// tie to the dialog control
	}
	
	if ((pQuitButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pQuitButton).SetPalette(m_pPalette);						// set the palette to use
		(*pQuitButton).SetControl(IDC_OPTIONS_QUIT,this);				// tie to the dialog control
	}
    // Disable the Rules button if told to do so
    //
    if (m_nFlags & NO_RULES) {

        pWndTemp = GetDlgItem(IDC_OPTIONS_RULES);
        assert(pWndTemp != NULL);
        if (pWndTemp != NULL)
            pWndTemp->EnableWindow(FALSE);
    }

    // Disable the NewGame button if told to do so
    //
    if (m_nFlags & NO_NEWGAME) {

        pWndTemp = GetDlgItem(IDC_OPTIONS_NEWGAME);
        assert(pWndTemp != NULL);
        if (pWndTemp != NULL)
            pWndTemp->EnableWindow(FALSE);
    }

    // Disable the Options button if told to do so
    //
    if (m_nFlags & NO_OPTIONS) {

        pWndTemp = GetDlgItem(IDC_OPTIONS_OPTIONS);
        assert(pWndTemp != NULL);
        if (pWndTemp != NULL)
            pWndTemp->EnableWindow(FALSE);
    }

    // Disable the Audio button if told to do so
    //
    if (m_nFlags & NO_AUDIO) {

        pWndTemp = GetDlgItem(IDC_OPTIONS_AUDIO);
        assert(pWndTemp != NULL);
        if (pWndTemp != NULL)
            pWndTemp->EnableWindow(FALSE);
    }

    // Disable the Return button if told to do so
    //
    if (m_nFlags & NO_RETURN) {

        pWndTemp = GetDlgItem(IDC_OPTIONS_RETURN);
        assert(pWndTemp != NULL);
        if (pWndTemp != NULL)
            pWndTemp->EnableWindow(FALSE);
    }

    // Disable the Return button if told to do so
    //
    if (m_nFlags & NO_QUIT) {

        pWndTemp = GetDlgItem(IDC_OPTIONS_QUIT);
        assert(pWndTemp != NULL);
        if (pWndTemp != NULL)
            pWndTemp->EnableWindow(FALSE);
    }

    return(TRUE);   // return TRUE  unless focused on a control
}


BOOL CMainMenu::OnEraseBkgnd(CDC *pDC)
{
    return(TRUE);
}


void CMainMenu::OnPaint()
{
    CBmpDialog::OnPaint();
}


void CMainMenu::OnClickedRules()
{
    CWnd *pControl;

    // Load the rules
    CRules  RulesDlg(this, m_pRulesFileName, m_pPalette, m_pWavFileName);

    // display the rules
    RulesDlg.DoModal();

    SetDefID(IDC_OPTIONS_OPTIONS);
    if (m_nFlags & NO_RETURN) {
	    SetDefID(IDC_OPTIONS_NEWGAME);
	    pControl = GetDlgItem(IDC_OPTIONS_NEWGAME);
	}
    else {
	    SetDefID(IDC_OPTIONS_RETURN);
	    pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	}
    GotoDlgCtrl(pControl);
}

void CMainMenu::OnClickedNewgame()
{
    // user has chosen to start a new game
    //
	ClearDialogImage();
    EndDialog(IDC_OPTIONS_NEWGAME);
}

void CMainMenu::OnClickedOptions()
{
    CWnd *pControl;

    SetDefID(IDC_OPTIONS_RULES);
    if (m_nFlags & NO_RETURN) {
	    SetDefID(IDC_OPTIONS_NEWGAME);
	    pControl = GetDlgItem(IDC_OPTIONS_NEWGAME);
	}
    else {
	    SetDefID(IDC_OPTIONS_RETURN);
	    pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	}
    GotoDlgCtrl(pControl);

    // call the user defined sub-options (we are the parent)
    //
    if (m_pOptionsFunction != NULL)
        (m_pOptionsFunction)(this);
}

void CMainMenu::OnClickedAudio()
{
    CWnd *pControl;

    SetDefID(IDC_OPTIONS_RULES);
    if (m_nFlags & NO_RETURN) {
	    SetDefID(IDC_OPTIONS_NEWGAME);
	    pControl = GetDlgItem(IDC_OPTIONS_NEWGAME);
	}
    else {
	    SetDefID(IDC_OPTIONS_RETURN);
	    pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	}
    GotoDlgCtrl(pControl);

	CAudioCfgDlg dlgAudioCfg(this, m_pPalette, IDD_AUDIOCFG);

	if (m_pGameParams != NULL) {
	    m_pGameParams->bMusicEnabled = GetPrivateProfileInt("Meta", "Music", TRUE, "HODJPODJ.INI");
	    m_pGameParams->bSoundEffectsEnabled = GetPrivateProfileInt("Meta", "SoundEffects", TRUE, "HODJPODJ.INI");
	}
}

void CMainMenu::OnOK()
{
    //
    // Don't do anything
    //
}

void CMainMenu::OnCancel()
{
    // user is returning to Mini-game (only if Continue is not disabled)
    //
    if (!(m_nFlags & NO_RETURN)) {
        ClearDialogImage();
        EndDialog(IDC_OPTIONS_RETURN);
    }
}

void CMainMenu::OnClickedReturn()
{
    // user is returning to Mini-game
    //
	ClearDialogImage();
    EndDialog(IDC_OPTIONS_RETURN);
}

void CMainMenu::OnClickedQuit()
{
    // user hit the Quit Button
    //
	ClearDialogImage();
   	EndDialog(IDC_OPTIONS_QUIT);
}

BEGIN_MESSAGE_MAP(CMainMenu, CBmpDialog)
    //{{AFX_MSG_MAP(CMainMenu)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_OPTIONS_RULES, OnClickedRules)
    ON_BN_CLICKED(IDC_OPTIONS_NEWGAME, OnClickedNewgame)
    ON_BN_CLICKED(IDC_OPTIONS_OPTIONS, OnClickedOptions)
    ON_BN_CLICKED(IDC_OPTIONS_AUDIO, OnClickedAudio)
    ON_BN_CLICKED(IDC_OPTIONS_RETURN, OnClickedReturn)
    ON_BN_CLICKED(IDC_OPTIONS_QUIT, OnClickedQuit)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace HodjNPodj
} // namespace Bagel
