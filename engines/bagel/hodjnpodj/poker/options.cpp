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

#include "stdafx.h"
#include "resource.h"

#include "options.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"

#include "dibdoc.h"

#include "poker.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"

#define		RULESSOUND		".\\SOUND\\POKE.WAV"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern	CPalette	*pOptionsPalette;

static	CRect		OptionsRect;

static	CFont		*pFont = NULL;
static	int 		tabstop = 20;



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
CWnd		*pControl;
CRules	dlgRules( m_pParentWnd, RULESFILE, pOptionsPalette, RULESSOUND );

	(void) dlgRules.DoModal();      // invoke the help dialog box
	pControl = this->GetDlgItem( IDC_OPTIONS_RETURN );
	GotoDlgCtrl( pControl );
	return;
}


void COptions::OnClickedNewgame()
{
	// TODO: Add your control notification handler code here
	RefreshBackground();
	CDialog::EndDialog(IDC_OPTIONS_NEWGAME);
  return; 
}

void COptions::OnClickedOptions()
{
	RefreshBackground();
	CDialog::EndDialog(IDC_OPTIONS_OPTIONS);
  return; 
}


void COptions::OnClickedReturn()
{
	// TODO: Add your control notification handler code here
	RefreshBackground();
	CDialog::EndDialog(IDC_OPTIONS_RETURN);
  return; 
}

void COptions::OnClickedQuit()
{
	// TODO: Add your control notification handler code here
	RefreshBackground();
	CDialog::EndDialog(IDC_OPTIONS_QUIT);
  return; 
}


//////////////////////////////////////////////////////////////////
void COptions::OnCancel()
{
	RefreshBackground();
	EndDialog( 0 );
	return;
}
