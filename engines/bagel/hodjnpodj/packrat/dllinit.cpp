/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * dllinit.cpp
 *
 * HISTORY
 *
 *  1.0 5/13/94 GTB     
 *
 * MODULE DESCRIPTION:
 *
 *
 * LOCALS:
 *
 *
 * GLOBALS:
 *
 *
 * RELEVANT DOCUMENTATION:
 *
 *      n/a
 *
 * FILES USED:
 *
 ****************************************************************/

#include "bagel/afxwin.h"
#include "resource.h"
#include "packrat.h" 

//#ifndef _DEBUG
//#error This source file must be compiled with _DEBUG defined
//#endif

HINSTANCE   hDLLInst;
HINSTANCE   hExeInst;

CMainPackRatWindow  *pcwndPackRat = NULL;   // pointer to the poker's main window 
CPalette            *pTestPalette = NULL;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

/*****************************************************************
 *
 * RunPoker
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          This is the API function for the DLL. It is what the calling app
 *          calls to invoke poker 
 *   
 * FORMAL PARAMETERS:
 *
 *      hParentWnd, lpGameInfo
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
extern "C" 
HWND FAR PASCAL RunPackRat( HWND hParentWnd, LPGAMESTRUCT lpGameInfo )
{

// if the pointer has garbage in it, the clean it out
    if ( pcwndPackRat != NULL ) {
        pcwndPackRat = NULL;
    }
    
// create a my poker window and show it
    pcwndPackRat = new CMainPackRatWindow( hParentWnd, lpGameInfo );
//  pcwndPackRat->ShowWindow( SW_SHOWNORMAL );
//  pcwndPackRat->UpdateWindow();
    pcwndPackRat->SetActiveWindow(); 
// return the handle to this window
    hDLLInst = (HINSTANCE)GetWindowWord( pcwndPackRat->m_hWnd, GWW_HINSTANCE);
    hExeInst = (HINSTANCE)GetWindowWord( hParentWnd, GWW_HINSTANCE);  
    if (lpGameInfo->bPlayingMetagame == FALSE)
        MFC::PostMessage( pcwndPackRat->m_hWnd, WM_COMMAND, IDC_OPTION, BN_CLICKED );
    return pcwndPackRat->m_hWnd;
}
/////////////////////////////////////////////////////////////////////////////
// DLL initialization
// this was take straight from the MSVC MFC Sample DLLTRACE

class CTracerDLL : public CWinApp
{
public:
    virtual BOOL InitInstance(); // Initialization
    virtual int ExitInstance();  // Termination (WEP-like code)

    // nothing special for the constructor
    CTracerDLL(const char* pszAppName)
        : CWinApp(pszAppName)
        { }
};

/*****************************************************************
 *
 * InitInstance
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This routine is automatically called when the application is
 *      started. 
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
 *      BOOL            Success (TRUE) / Failure (FALSE) status
 *
 ****************************************************************/
BOOL CTracerDLL::InitInstance()
{
    return TRUE;
}

/*****************************************************************
 *
 * ExitInstance
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This routine is automatically called when the application is
 *      being terminated. 
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
 *      int                     Success (0) / Failure status
 *
 ****************************************************************/
int CTracerDLL::ExitInstance()
{
    return(0);
}


CTracerDLL  NEAR tracerDLL("hnppkrt.dll");

/////////////////////////////////////////////////////////////////////////////
