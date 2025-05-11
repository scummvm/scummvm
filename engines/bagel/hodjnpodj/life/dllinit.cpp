/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * dllinit.cpp
 *
 * HISTORY
 *
 *      1.0 5/13/94 GTB
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

#include "game.h"

//#ifndef _DEBUG
//#error This source file must be compiled with _DEBUG defined
//#endif

HINSTANCE       hDLLInst;
HINSTANCE       hExeInst;

CMainWindow     *gMainWnd = NULL;   // pointer to the poker's main window
CPalette                *pTestPalette = NULL;
HCURSOR                 hGameCursor;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

/*****************************************************************
 *
 * RunLife
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *                      This is the API function for the DLL. It is what the calling app
 *                      calls to invoke poker
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
HWND FAR PASCAL RunLife( HWND hParentWnd, LPGAMESTRUCT lpGameInfo )
{

#ifdef _DEBUG
//  lpGameInfo->bPlayingMetagame = TRUE;
#endif //_DEBUG

        // create a my poker window and show it

        gMainWnd = new CMainWindow( hParentWnd, lpGameInfo );
        gMainWnd->ShowWindow( SW_SHOWNORMAL );

        gMainWnd->SplashScreen();   // Force immediate display to minimize repaint delay
        gMainWnd->DisplayStats();       // Initializes the stats

        gMainWnd->UpdateWindow();
        gMainWnd->SetActiveWindow();
// return the handle to this window
        hDLLInst = (HINSTANCE)GetWindowWord( gMainWnd->m_hWnd, GWW_HINSTANCE);
        hExeInst = (HINSTANCE)GetWindowWord( hParentWnd, GWW_HINSTANCE);
//        ::PostMessage( pMainGameWnd->m_hWnd, WM_COMMAND, IDC_COMMAND, BN_CLICKED );
        return gMainWnd->m_hWnd;
}
/////////////////////////////////////////////////////////////////////////////
// DLL initialization
// this was take straight from the MSVC MFC Sample DLLTRACE

class CDFADll : public CWinApp
{
public:
        virtual BOOL InitInstance(); // Initialization
        virtual int ExitInstance();  // Termination (WEP-like code)

        // nothing special for the constructor
        CDFADll(const char* pszAppName)
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
BOOL CDFADll::InitInstance()
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
int CDFADll::ExitInstance()
{
    return(0);
}

CDFADll  NEAR tracerDLL("hnplife.dll");
