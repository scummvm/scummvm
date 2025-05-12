// options.cpp -- implementation of COptions class for options dialog
// Written by John J. Xenakis for Boffo Games Inc., 1994


/*****************************************************************
 * Copyright (c) 1994 by Ledge Multimedia, All Rights Reserved.
 *
 *
 * options.cpp
 *
 * HISTORY
 *
 *	  1.0	03/08/94	EDS		first release
 *
 * MODULE DESCRIPTION:
 *
 *     tbs
 *
 * CONSTRUCTORS:
 *
 *	COptions				create a options object and initialize elements
 *
 * DESTRUCTORS:
 *
 *	~COptions				destroy a options object and release its resources
 *
 * PUBLIC:
 *
 *
 * PUBLIC GLOBAL:
 *
 *
 * PROTECTED:
 *
 *      n/a
 *
 * PRIVATE:
 *
 *
 * MEMBERS:
 *
 *      refer to options.h
 *   
 * RELEVANT DOCUMENTATION:
 *
 *      n/a
 *   
 ****************************************************************/

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "resource.h"
#include "options.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "optndlg.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "game.h"
#include "gamedll.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "audiocfg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern	LPGAMESTRUCT 	pGameInfo;

static	CRect		OptionsRect,
					OkayRect;

static	CFont		*pFont = NULL;
static	int 		tabstop = 20;

extern CPalette *pOptionsPalette;


void COptions::OnLButtonDown(UINT nFlags, CPoint point)
{
	
	CDialog::OnLButtonDown(nFlags, point);
}


void COptions::OnMouseMove(UINT nFlags, CPoint point)
{
	
	CDialog::OnMouseMove(nFlags, point);
}

void COptions::OnClickedRules()
{
CWnd	*pControl = NULL;

	CRules	RulesDlg(this,RULESSPEC,pOptionsPalette, ((*pGameInfo).bSoundEffectsEnabled ? NARRATIVESPEC : NULL));
	(void) RulesDlg.DoModal();      				// invoke the help dialog box
	SetDefID(IDC_OPTIONS_OPTIONS);
	SetDefID(IDC_OPTIONS_RETURN);
	pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	GotoDlgCtrl(pControl);
}  


void COptions::OnClickedNewgame()
{
	ClearDialogImage();
	EndDialog( IDC_NEWGAME );
	
}


void COptions::OnClickedOptions()
{
CWnd	*pControl = NULL;

	COptnDlg OptionsDlg( this, pOptionsPalette );	// Call Specific Game
	
	if ( OptionsDlg.DoModal() == IDOK )	{	// save values set in dialog box
		ClearDialogImage();
		EndDialog( IDC_RESTART ) ;}
	else {
		SetDefID(IDC_OPTIONS_RULES);
		SetDefID(IDC_OPTIONS_RETURN);
		pControl = GetDlgItem(IDC_OPTIONS_RETURN);
		GotoDlgCtrl(pControl);
	}
}


void COptions::OnClickedAudio()
{
CWnd	*pControl = NULL;

	CAudioCfgDlg dlgAudioCfg(this, pOptionsPalette, IDD_AUDIOCFG);

	if (pGameInfo != NULL) {
	    pGameInfo->bMusicEnabled = GetPrivateProfileInt("Meta", "Music", TRUE, "HODJPODJ.INI");
	    pGameInfo->bSoundEffectsEnabled = GetPrivateProfileInt("Meta", "SoundEffects", TRUE, "HODJPODJ.INI");
	}
	
	SetDefID(IDC_OPTIONS_RULES);
	SetDefID(IDC_OPTIONS_RETURN);
	pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	GotoDlgCtrl(pControl);
}


void COptions::OnClickedReturn()
{
	ClearDialogImage();
	EndDialog( IDC_RETURN ) ;		// successful completion
}

void COptions::OnClickedQuit()
{
	ClearDialogImage();
	EndDialog( IDC_QUIT ) ;			// unsuccessful completion
}
