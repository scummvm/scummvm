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
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/riddles/riddles.h"

namespace Bagel {
namespace HodjNPodj {
namespace Riddles {

HINSTANCE   hDLLInst;
HINSTANCE hExeInst;
extern LPGAMESTRUCT pGameParams;

// global the pointer to the your game's main window
HWND ghParentWnd;

HWND RunRiddles(HWND hParentWnd, LPGAMESTRUCT lpGameInfo) {
    CRiddlesWindow *pMain;

    pGameParams = lpGameInfo;

    // invoke your game here by creating a pGame for your main window
    // look at the InitInstance for your game for this

    ghParentWnd = hParentWnd;

    if ((pMain = new CRiddlesWindow) != NULL) {

        pMain->ShowWindow(SW_SHOWNORMAL);

        pMain->UpdateWindow();

        pMain->SetActiveWindow();

        if (pGameParams->bPlayingMetagame)
            pMain->PlayGame();
    }

    // these must be set in this function
    hDLLInst = (HINSTANCE)::GetWindowWord( pMain->m_hWnd, GWW_HINSTANCE);
    hExeInst = (HINSTANCE)::GetWindowWord( hParentWnd, GWW_HINSTANCE);

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

CTracerDLL  NEAR tracerDLL("hnpridl.dll");

} // namespace Riddles
} // namespace HodjNPodj
} // namespace Bagel
