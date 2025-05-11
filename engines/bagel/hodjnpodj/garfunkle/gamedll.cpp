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
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "garfunk.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

HWND FAR PASCAL RunGarf( HWND, LPGAMESTRUCT);

#ifdef __cplusplus
}
#endif

HINSTANCE   hDLLInst;
HINSTANCE hExeInst;

LPGAMESTRUCT pGameInfo;

// global the pointer to the your game's main window
HWND ghParentWnd;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

/*****************************************************************
 *
 * RunBeac
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
HWND FAR PASCAL RunGarf( HWND hParentWnd, LPGAMESTRUCT lpGameInfo )
{
    CMainWindow *pMain;

#ifdef _DEBUG
//  lpGameInfo->bPlayingMetagame = TRUE;
//  lpGameInfo->nSkillLevel = SKILLLEVEL_LOW;
#endif  

    pGameInfo = lpGameInfo;

    // invoke your game here by creating a pGame for your main window
    // look at the InitInstance for your game for this

    ghParentWnd = hParentWnd;

    if ((pMain = new CMainWindow) != NULL) {

//        pMain->ShowWindow(SW_SHOWNORMAL);

//        pMain->UpdateWindow();

        pMain->SetActiveWindow();
    }

    // these must be set in this function
    hDLLInst = (HINSTANCE)GetWindowWord( pMain->m_hWnd, GWW_HINSTANCE);
    hExeInst = (HINSTANCE)GetWindowWord( hParentWnd, GWW_HINSTANCE);

    return pMain->m_hWnd;   // return the m_hWnd of your main game window
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
 *      started. Use this InitInstance instead of your own 
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
    // gray dialogs
    SetDialogBkColor();

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


CTracerDLL  NEAR tracerDLL("hnpgarf.dll");

/////////////////////////////////////////////////////////////////////////////
