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
#include "bagel/hodjnpodj/metagame/gtl/resource.h"
#include "bagel/hodjnpodj/metagame/gtl/gtl.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/metagame/gtl/init.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

CGtlApp *theApp;
CWinApp *priorApp;
HINSTANCE   hDLLInst;
HINSTANCE   hExeInst;
HWND        ghwndParent;

//CMainDFAWindow    *pMainGameWnd = nullptr;   // pointer to the poker's main window
CPalette        *pTestPalette = nullptr;
HCURSOR         hGameCursor;
extern CGtlFrame       *pMainWindow;
LPGAMESTRUCT    pGameInfo = nullptr;
HWND            hThisWind;
CBfcMgr         *lpMetaGameStruct = nullptr;
bool            bJustReturned = false;

/////////////////////////////////////////////////////////////////////////////
// Public C interface

/*****************************************************************
 *
 * RunMeta
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *          This is the API function for the DLL. It is what the calling app
 *          calls to invoke the grand tour
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

HWND FAR PASCAL RunMeta(HWND hParentWnd, CBfcMgr *lpBfcMgr, bool bMetaLoaded) {
	ghwndParent = hParentWnd;
	lpMetaGameStruct = lpBfcMgr;
	priorApp = AfxGetApp();

	theApp = new CGtlApp();
	theApp->addResources("hnpmeta.dll");
	theApp->addFontResource("msserif.fon");
	theApp->InitApplication();
	theApp->InitInstance();
	theApp->setKeybinder(KeybindToKeycode);

	#if RETAIN_META_DLL
	if (bMetaLoaded) {
		CGtlView    *xpGtlFocusView, *xpGtlMouseView;
		CGtlDoc     *xpGtlDoc = nullptr;

		pMainWindow->GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;
		xpGtlDoc->m_xpGtlData->m_xpGtlView->SetTimer(ANIMATION_TIMER_ID, ANIMATION_TIMER_INTERVAL, nullptr);
		if (lpBfcMgr->m_bRestart == false) {
			xpGtlDoc->m_xpGtlData->m_xpXodjChain = nullptr;
			xpGtlDoc->m_xpGtlData->m_xpCurXodj = nullptr;
			xpGtlDoc->m_xpGtlData->m_iMishMoshLoc = 0;
			pMainWindow->m_lpBfcMgr = nullptr;
			pMainWindow->m_lpBfcMgr = lpBfcMgr;
			xpGtlDoc->m_xpGtlData->m_bGameOver = false;
		}
		pMainWindow->ShowWindow(SW_SHOWNORMAL);
		// if restoring a saved game
		//
		bJustReturned = true;
		if (lpBfcMgr->m_bRestoredGame) {

			// Re-init the game using the restored info (i.e. lpBfcMgr)
			//
			xpGtlDoc->m_xpGtlData->m_bInitMetaGame = true;
			xpGtlDoc->m_xpGtlData->InitMetaGame(xpGtlDoc->m_xpGtlData->m_xpGtlView, true);

		} else if (lpBfcMgr->m_iFunctionCode) {

			xpGtlDoc->m_xpGtlData->ReturnFromInterface();
			xpGtlDoc->m_xpGtlData->ProcessMove();
		}
	} else
		SetupWindow(lpBfcMgr);
	#else
	bJustReturned = lpBfcMgr->m_bRestart;
	SetupWindow(lpBfcMgr);
	#endif

	sndPlaySound(nullptr, 0);              // clear all rogue sounds

	theApp->Run();

	delete theApp;
	theApp = nullptr;

	return 0;
}


void SetupWindow(CBfcMgr *) {
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

	SetupCursor();

	xpGtlApp->CreateInstance();

	hDLLInst = (HINSTANCE)MFC::GetWindowWord(pMainWindow->m_hWnd, GWW_HINSTANCE);
	hExeInst = (HINSTANCE)MFC::GetWindowWord(ghwndParent, GWW_HINSTANCE);

}


void SetupCursor() {
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
	HCURSOR hNewCursor = nullptr;

	hNewCursor = xpGtlApp->LoadStandardCursor(IDC_ARROW);
	//if (hNewCursor != nullptr);
	MFC::SetCursor(hNewCursor);
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
