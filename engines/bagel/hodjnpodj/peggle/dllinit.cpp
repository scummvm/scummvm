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
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "resource.h"
#include "dllinit.h"
#include "game.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
// #include your main header file for your game 
#include "gamedll.h"

HINSTANCE   hDLLInst;
HINSTANCE   hExeInst;

extern CSprite      *pCursorSprite;
extern CSprite      *pShotGlass;
extern CSprite      *pTableSlot;
extern CSprite      *pInvalidSlot;
extern CPalette     *pGamePalette;
extern CBmpButton   *pScrollButton;

CMainWindow     *pcwndPeggle = NULL;
LPGAMESTRUCT    pGameInfo;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

/*****************************************************************
 *
 * RunPeggle
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
HWND FAR PASCAL RunPeggle( HWND hParentWnd, LPGAMESTRUCT lpGameInfo )
{
    pGameInfo = lpGameInfo;
    
//#ifdef _DEBUG
//  (*pGameInfo).bPlayingMetagame = TRUE;                                     
//#endif
                                    
// if the pointer has garbage in it, the clean it out
  if ( pcwndPeggle != NULL ) {
          pcwndPeggle = NULL;
  }
// create a my window and show it
  pcwndPeggle = new CMainWindow( hParentWnd );
  pcwndPeggle->ShowWindow( SW_SHOWNORMAL );
  pcwndPeggle->UpdateWindow();
  pcwndPeggle->SetActiveWindow(); 
// return the handle to this window
  hDLLInst = (HINSTANCE)GetWindowWord( pcwndPeggle->m_hWnd, GWW_HINSTANCE);
  hExeInst = (HINSTANCE)GetWindowWord( hParentWnd, GWW_HINSTANCE);  
  return pcwndPeggle->m_hWnd;
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

CSprite::FlushSpriteChain();

if (pShotGlass != NULL)
    delete pShotGlass;
if (pTableSlot != NULL)
    delete pTableSlot;
if (pInvalidSlot != NULL)
    delete pInvalidSlot;
if (pCursorSprite != NULL)
    delete pCursorSprite;

if (pScrollButton != NULL)
    delete pScrollButton;
    
if (pGamePalette != NULL ) {
    pGamePalette->DeleteObject ;
    delete pGamePalette;
    }

// don't forget to set the Cursor back to normal!

#ifndef SHOW_CURSOR
::ShowCursor(TRUE);
#endif

    return(0);
}


CTracerDLL  NEAR tracerDLL("hnppggl.dll");

/////////////////////////////////////////////////////////////////////////////
