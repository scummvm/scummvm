/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * gamedll.cpp
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
#include <afxwin.h>
#include <gamedll.h>
#include "fuge.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

HWND FAR PASCAL _export RunFuge(HWND, LPGAMESTRUCT);

#ifdef __cplusplus
}
#endif

HINSTANCE   hDLLInst;
HINSTANCE   hExeInst;
extern LPGAMESTRUCT pGameParams;

// global the pointer to the your game's main window
HWND ghParentWnd;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

/*****************************************************************
 *
 * RunFuge
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
//extern "C"
HWND FAR PASCAL _export RunFuge(HWND hParentWnd, LPGAMESTRUCT lpGameInfo)
{
    CFugeWindow *pMain;

    pGameParams = lpGameInfo;

    // invoke your game here by creating a pGame for your main window
    // look at the InitInstance for your game for this

    ghParentWnd = hParentWnd;

    if ((pMain = new CFugeWindow) != NULL) {

        pMain->ShowWindow(SW_SHOWNORMAL);

        pMain->UpdateWindow();

        pMain->SetActiveWindow();

        // these are used by Rules
        hDLLInst = (HINSTANCE)::GetWindowWord(pMain->m_hWnd, GWW_HINSTANCE);
        hExeInst = (HINSTANCE)::GetWindowWord(hParentWnd, GWW_HINSTANCE);

        if (pGameParams->bPlayingMetagame)
            pMain->PlayGame();
    }

    return pMain->m_hWnd;   // return the m_hWnd of your main game window
}

/////////////////////////////////////////////////////////////////////////////
// DLL initialization
// this was taken straight from the MSVC MFC Sample DLLTRACE

class CFugeDLL : public CWinApp
{
public:
    virtual BOOL InitInstance(); // Initialization
    virtual int ExitInstance();  // Termination (WEP-like code)

    // nothing special for the constructor
    CFugeDLL(const char* pszAppName)
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
BOOL CFugeDLL::InitInstance()
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
int CFugeDLL::ExitInstance()
{
    return(0);
}

CFugeDLL  NEAR cFugeDLL("hnpfuge.dll");
