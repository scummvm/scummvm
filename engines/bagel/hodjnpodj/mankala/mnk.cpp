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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/libs/macros.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/mankala/mnk.h"
#include "bagel/hodjnpodj/mankala/resource.h"

#ifdef BAGEL_DEBUG
	#include "bagel/hodjnpodj/mankala/transform.h"
#endif

namespace Bagel {
namespace HodjNPodj {
namespace Mankala {

//#define _MACROS3
#define IDC_PITBUTTON 1
#define PITBUTTON_TIMER 2

LPGAMESTRUCT pGameParams;
extern HWND ghParentWnd;

#ifdef BAGEL_DEBUG
	BOOL ResetPitsDlgProc(HWND, UINT, WPARAM, LPARAM);
	inline void FlushMouseMessages(HWND);

	static CMove *gpcMove,
	*gpcStoreMove;
#endif
///DEFS mnk.h

/////////////////////////////////////////////////////////////////////////////

// theMnkApp:
// Just creating this application object runs the whole application.
//
//CMnkApp NEAR theMnkApp ;

/////////////////////////////////////////////////////////////////////////////

/*****************************************************************
 *
 * CMnkWindow
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Create the window with the appropriate style, size, menu, etc. ;
 * it will be later revealed by CMnkApp::InitInstance().  Then
 * create our splash screen object by opening and loading its DIB.
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

static HCURSOR  hHourGlassCursor;
extern BOOL gbTurnSoundsOff;

//* CMnkWindow::CMnkWindow() -- mankala window constructor function
CMnkWindow::CMnkWindow(void) {
	CDC* pDC = nullptr ;                         // device context for the screen
	CString xpszWndClass ;
	CDibDoc *xpDibDoc = nullptr ;   // pointer to the background art DIB
	CSize   cMySize ;
	HCURSOR hOldCursor;

	memset((char *)&m_cStartData2, 0, (size_t)((char *)&m_cEndData2 - (char *)&m_cStartData2)) ;
	// zero out all my fields

	m_bJustStarted = TRUE;
	m_bStartGame = TRUE ;   // set flag - game just starting
	m_bRulesActive = FALSE; //flag to indicate the rules scroll is unfurled.
	// this flag is set only when rules are invoked via the F1 key.

	m_bPlaySound = pGameParams->bSoundEffectsEnabled;
	pGameParams->lScore = 0L;
	gbTurnSoundsOff = GetPrivateProfileInt("Mankala", "MuteCrab", 0, INI_FILENAME) ; // Crab talks

	hHourGlassCursor = LoadCursor(nullptr, IDC_WAIT);
	SetCursor(hHourGlassCursor);
	hOldCursor = LoadCursor(nullptr, IDC_ARROW);

	if (pGameParams->bMusicEnabled)
		m_pSound = new CSound(this, MIDI_BCKGND, SOUND_MIDI | SOUND_DONT_LOOP_TO_END);

	// initialize default game options
	m_iStartStones = GetPrivateProfileInt("Mankala", "StartStones", 3, INI_FILENAME) ; // 3 stones per pit

	m_bComputer[1] = TRUE ; // player 1 is computer (0 is human)
	m_iTableStones = MAXTABLESTONES ;
	m_bInitData = FALSE ;       // don't init table
	m_iMaxDepth[0] = m_iMaxDepth[1] = 5 ;   // minimax depth
	m_iCapDepth[0] = m_iCapDepth[1] = 3 ;   // capture depth

	// *** debugging defaults
//    m_bDumpPopulate = m_bDumpMoves = m_bDumpTree = TRUE ;
//    m_bInitData = TRUE ;
//    m_iTableStones = 3 ;

	Common::strcpy_s(m_szDataDirectory, DATADIR) ;    // data directory
	// containing the bitmap files

// Define a special window class which traps double-clicks, is byte
// aligned to maximize BITBLT performance, and creates "owned" DCs
// rather than sharing the five system defined DCs which are not
// guaranteed to be available ; this adds a bit to our app size but
// avoids hangs/freezes/lockups.

	xpszWndClass = AfxRegisterWndClass(CS_DBLCLKS |
	                                   CS_BYTEALIGNWINDOW | CS_OWNDC,
	                                   nullptr, nullptr, nullptr) ;


	xpDibDoc = new CDibDoc() ;  // create an object to hold our screen
	if (! xpDibDoc) {
		MFC::SetCursor(hOldCursor);
		MFC::MessageBox(nullptr, "Abnormal MiniGame Termination", "Internal Error", MB_ICONSTOP);
		delete this;
		return;
	}      // ... and verify we got it

	if (!(*xpDibDoc).OpenDocument(".\\ART\\MANKALA.BMP")) {
		MFC::SetCursor(hOldCursor);
		MFC::MessageBox(nullptr, "Cannot Open Background Bitmap. Please Check for file path and/or system resources. Terminating Game", "Open Error", MB_ICONEXCLAMATION | MB_OK) ;
		delete xpDibDoc;
		delete this;
		return;
	}

	// next load in the actual DIB based artwork for screen
	if (!(CMnkWindow::m_xpGamePalette = (*xpDibDoc).DetachPalette())) {
		// grab its palette and save it for later use
		MFC::SetCursor(hOldCursor);
		MFC::MessageBox(nullptr, "Cannot acquire a non nullptr Palette. Game Terminated", "Palette Error", MB_OK | MB_ICONEXCLAMATION);
		delete xpDibDoc;
		delete this;
		return;
	}
	delete xpDibDoc ;   // now discard the splash screen

	if ((pDC = GetDC())) {   // get a device context for our window

		// determine where to place the game window
		// ... so it is centered on the screen
		#ifndef _CODEVIEW
		m_cMainRect.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
		#else
		m_cMainRect.SetRect(300, 300, 300 + GAME_WIDTH, 300 + GAME_HEIGHT);
		#endif

		#ifndef BAGEL_DEBUG
		m_cMainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1 ;
		m_cMainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1 ;
		m_cMainRect.right = m_cMainRect.left + GAME_WIDTH ;
		m_cMainRect.bottom = m_cMainRect.top + GAME_HEIGHT ;
		#endif

		// Create the window as a POPUP so that no borders, title, or menu are
		// present ; this is because the game's background art will fill the
		// entire 640x480 area.

#define WSTYLE WS_POPUP|WS_CLIPCHILDREN

		pDC->SelectPalette(CMnkWindow::m_xpGamePalette, FALSE);
		pDC->RealizePalette();
		ReleaseDC(pDC);
		pDC = nullptr ;        // zero out context pointer
	} else {
		#ifdef BAGEL_DEBUG
		MFC::MessageBox(nullptr, "Cannot acquire device context. Abnormal Program Termination", "Error", MB_ICONSTOP) ;
		#endif
		return;
	}
	if (!Create(xpszWndClass, "Boffo Games -- Mankala", WSTYLE,
	            m_cMainRect, nullptr, 0)) {
		MFC::MessageBox(nullptr, "Cannot open window. Close some other windows to continue", "Error", MB_ICONSTOP) ;
		delete this;
		return ;
	}

	//
	// Put up the splash screen so they have somethin' to look at:
	//
	ShowWindow(SW_SHOWNORMAL);
	PaintBitmapObject(&m_cBmpMain, BMT_MAIN) ;                      // paint main screen bitmap
	m_cBmpScroll.m_bSprite = TRUE ;                                 // scroll is a sprite
	PaintBitmapObject(&m_cBmpScroll, BMT_SCROLL) ;


	if (!pGameParams->bPlayingMetagame) {
		switch (GetPrivateProfileInt("Mankala", "StartLevel", 3, INI_FILENAME)) {       // minimax algorithm is the default ...
		case 0:
			m_eLevel[0] = m_eLevel[1] = LEV_RANDOM;
			break;
		case 1:
			m_eLevel[0] = m_eLevel[1] = LEV_LOWEST;
			break;
		case 2:
			m_eLevel[0] = m_eLevel[1] = LEV_HIGHEST;
			break;
		default:
		case 3:
			m_eLevel[0] = m_eLevel[1] = LEV_EVAL;
			break;
		}
	} else {
		/* ...unless you're playing Meta Game, Levels 2,3,4 are mapped
		      into nSKilllevel=0,1,2 respectively,  and the initial number of
		      stones are 3,4, 5 respectively.
		*/

		switch (pGameParams->nSkillLevel) {
		case  SKILLLEVEL_LOW:
			m_iStartStones = 3;
			m_eLevel[0] = m_eLevel[1] = LEV_LOWEST;
			break;
		case SKILLLEVEL_MEDIUM :
			m_iStartStones = 4;
			m_eLevel[0] = m_eLevel[1] = LEV_HIGHEST;
			break;
		case SKILLLEVEL_HIGH :
			m_iStartStones = 5;
			m_eLevel[0] = m_eLevel[1] = LEV_EVAL;
			break;
		default :
			m_iStartStones = 3;
			m_eLevel[0] = m_eLevel[1] = LEV_EVAL;
			break;
		}
	}

	AllocatePits() ;        // allocate pit objects
	//CMnk::InitData() ;      // initialize data tables

	#ifdef _MACROS
	//always play music in test mode.
	if (m_pSound)
		if (!m_pSound->midiLoopPlaySegment(1004L, 34040L, 1004L, FMT_MILLISEC))
			MessageBox("Unable to Play Background Music", "Internal Error");
	#else
	//otherwise test for flag before playing music.
	if (pGameParams->bMusicEnabled && m_pSound) {
		//#if 0
		if (!(m_pSound->midiLoopPlaySegment(1004L, 34040L, 1004L, FMT_MILLISEC)))
			MFC::MessageBox("Unable to Play Background Music", "Internal Error");
		//#endif
	}
	#endif

//    //srand((unsigned) time(nullptr)) ;  // seed the random number generator

	MFC::SetCursor(hOldCursor);

//    ASSERT(SetTimer(SPRITE_TIMER,SPRITE_INTERVAL,nullptr)) ;
	// set the interval timer for movement
	#ifdef _MACROS
	EM("///////////////////////////////////////////////////////////////////////////////////////////");
	EM("Leaving Constructor");
	#endif

}

//* CMnkWindow::~CMnkWindow -- Mankala window destructor function
CMnkWindow::~CMnkWindow(void) {
}

/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed ; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.  Ensures that the entire client area
 * of the main screen window is repainted, not just the portion in the
 * update region ; see PaintScreen() ;
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

void CMnkWindow::OnPaint() {
	PAINTSTRUCT lpPaint ;

	InvalidateRect(nullptr, FALSE) ;   // invalidate the entire window
	BeginPaint(&lpPaint) ;              // bracket start of window update
	PaintScreen() ;                    // repaint our window's content
	EndPaint(&lpPaint) ;                // bracket end of window update
}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the QUIT and OKAY buttons when they are clicked.
 *
 * This function is called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
 *
 * FORMAL PARAMETERS:
 *
 *  wParam      identifier for the button to be processed
 *  lParam      type of message to be processed
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

// OnCommand
//

BOOL CMnkWindow::OnCommand(WPARAM wParam, LPARAM lParam) {

	if (HIWORD(lParam) == BN_CLICKED)
		// only want to look at button clicks
		switch (wParam) {

		case IDC_SCROLL:
			if (!m_bInMenu) {
				OptionsDialog() ;     //bring up main menu.
				(*this).SetFocus() ; // Reset focus back to the main window
			}                           //end if !m_bInMenu
			break ;                // ... to force a repaint
		default :
			break;
		}

	#ifdef _MACROS
	EM("Leaving OnCommand");
	#endif

	return TRUE ;
}


/*****************************************************************
 *
 * OnChar
 *
 * FUNCTIONAL DESCRIPTION:
 *
....... *  This function is called when keyboard input generates a character.
 *
 *  (Add game-specific processing)
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/

void CMnkWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// TODO: Add your message handler code here and/or call default


	CFrameWnd ::OnChar(nChar, nRepCnt, nFlags) ; // default action

}

void CMnkWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// TODO: Add your message handler code here and/or call default
	#ifdef _MACROS
	EM("Entering OnKeyDown");
	#endif

	int iRVal;

	if (nChar == VK_F1) {
		m_bRulesActive = FALSE;
		CRules cRulesDlg(this, RULES, m_xpGamePalette, pGameParams->bSoundEffectsEnabled ? RULES_NARRATION : nullptr) ;
		CSound::waitWaveSounds();
		if ((iRVal = cRulesDlg.DoModal()) == -1)
			MessageBox("The Mankala Rules Text File Can't Be Opened", "Error Opening File");
		m_bRulesActive = FALSE;
	}

	else if (nChar == VK_F2)
		OptionsDialog() ;
	/*
	else if (nChar == VK_F12){
	    gbTurnSoundsOff=!gbTurnSoundsOff; //F12 is pressed, toggle sounds ON/OFF.
	    WritePrivateProfileString("Mankala","MuteCrab",gbTurnSoundsOff?"1":"0",INI_FILENAME);
	}*/
	else if (nChar == VK_F8) {
		#ifdef BAGEL_DEBUG
		HWND hWnd;
		HINSTANCE hInst;
		int u,
		    i,
		    j ;

		if (hWnd = GetSafeHwnd()) {
			hInst = (HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE);

			if ((gpcMove = new CMove) && (gpcStoreMove = new CMove)) {
				memcpy(gpcMove, &m_cCurrentMove, sizeof(CMove));
				memcpy(gpcStoreMove, &m_cCurrentMove, sizeof(CMove));
				u = MFC::DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RESET_PITS_DLG), hWnd, (DLGPROC)ResetPitsDlgProc, (LPARAM)m_iStartStones);
				if (u) {
					memcpy(&m_cCurrentMove, gpcMove, sizeof(CMove));
					for (i = 0; i < 2; i++) {
						for (j = 0; j < NUMPITS + 2; j++) {
							m_xpcPits[i][j]->m_iNumStones = m_cCurrentMove.m_iNumStones[i][j];
							AdjustPitDisplay(m_xpcPits[i][j], TRUE);
						}
					}
				}//end if u
				if (gpcMove) delete gpcMove;
				if (gpcStoreMove) delete gpcStoreMove;
			}
		}
		#endif  //_debug
	}/*else if(nChar==VK_F9){
        #ifdef BAGEL_DEBUG
        double* v, *J;
        HGLOBAL hglbV=GlobalAlloc(GHND,sizeof(double)* NUMPITS);
        HGLOBAL hglbJ=GlobalAlloc(GHND,sizeof(double)* NUMPITS);

        J=(double*) GlobalLock(hglbJ);
        for(int j=0; j<NUMPITS; J[ j ]= ++j);
        GlobalUnlock(hglbJ);

        v=(double*) GlobalLock(hglbV);
        if(!dft(v, m_cCurrentMove.m_iNumStones[1]+2, NUMPITS)){
            GlobalUnlock(hglbV);
            graph(m_hWnd, hglbV, hglbJ,  NUMPITS);
        }else{
            GlobalUnlock(hglbV);
        }
        #endif
    }  */
	else CFrameWnd ::OnKeyDown(nChar, nRepCnt, nFlags);

	#ifdef _MACROS
	EM("Leaving OnKeyDown");
	#endif
}


/*****************************************************************
 *
 * OnSysChar
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when keyboard input generates a
 *  system character.
 *
 *      (Add game-specific processing)
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/

void CMnkWindow::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// TODO: Add your message handler code here and/or call default

	#ifdef _MACROS
	EM("Entering OnSysChar");
	#endif

	// terminate game on ALT-Q
	if ((nChar == 'q') && (nFlags & 0x2000))
		PostMessage(WM_CLOSE, 0, 0) ; // same as clicking QUIT button
	#ifdef BAGEL_DEBUG
	else if ((nChar == 'd') && (nFlags & 0x2000)) {
		BOOL bStartGame = m_bStartGame ;

		DebugDialog() ;
		if (bStartGame && !m_bStartGame)
			StartGame() ;
	}
	#endif
	else
		CFrameWnd ::OnSysChar(nChar, nRepCnt, nFlags) ; // default action

	#ifdef _MACROS
	EM("Entering OnSysChar");
	#endif

}


void CMnkWindow::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// terminate game on ALT-F4
	if ((nChar == VK_F4) && (nFlags & 0x2000))
		PostMessage(WM_CLOSE, 0, 0) ; // same as clicking QUIT button
	else
		CFrameWnd ::OnSysChar(nChar, nRepCnt, nFlags) ; // default action

}
/*****************************************************************
 *
 * OnXXXXXX
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      These functions are called when ever the corresponding WM_
 *      event message is generated for the mouse.
 *
 *      (Add game-specific processing)
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/

void CMnkWindow::OnMouseMove(UINT nFlags, CPoint point) {
	#ifdef _MACROS
	EM("Entering MouseMove");
	#endif
	MFC::SetCursor(LoadCursor(nullptr, IDC_ARROW));

	static int dxCursor = GetSystemMetrics(SM_CXCURSOR);        //cursor size.
	static int dyCursor = GetSystemMetrics(SM_CYCURSOR);

	int iPlayer = 0, iPit = 0;
	BOOL bFound;
	CDC *pDC;
	CRect crctPitBounds, crctTxt;
	HLOCAL hlocShells;
	NPSTR npszShells;
	CBmpObject *pcBmpObject;

	/*
	    DETERMINE the current location of the mouse visavis the pit locations
	    if not in the main menu (scroll down mode).
	*/
	for (iPlayer = 0, bFound = FALSE; !(m_bRulesActive || m_bInMenu) && iPlayer < 2; iPlayer++) {
		for (iPit = -1; !bFound && iPit < NUMPITS ; bFound || ++iPit) {
			pcBmpObject = &(m_xpcPits[iPlayer][iPit + 2]->m_cBmpObject) ;
			crctPitBounds = CRect(pcBmpObject->m_cPosition, pcBmpObject->m_cSize);
			if (crctPitBounds.PtInRect(point))
				bFound = TRUE ;
		}
		if (bFound) break;
	}//end for

	if (bFound && (m_cCurrentMove.m_iNumStones[iPlayer][iPit + 2])) {

		if ((pDC = GetDC())) {
			if ((hlocShells = MFC::LocalAlloc(GHND, 16))) {
				npszShells = (NPSTR)MFC::LocalLock(hlocShells);
				Common::sprintf_s(npszShells, 16, "%2d shell%c", m_cCurrentMove.m_iNumStones[iPlayer][iPit + 2], (m_cCurrentMove.m_iNumStones[iPlayer][iPit + 2] > 1) ? 's' : 0x0);

				if (m_pText) {
					m_pText->RestoreBackground(pDC);
					delete m_pText;
					m_pText = nullptr;
				}

				/* the following values are arrived thru trial and error */
				/* "Attach" the text  to the cursor */
				crctTxt.SetRect(point.x + dxCursor - 20, point.y + dyCursor - 20, point.x + dxCursor + 40, point.y + dyCursor + 10);
				if ((m_pText = new CText(pDC, m_xpGamePalette, &crctTxt, JUSTIFY_CENTER))) {
					m_pText->DisplayString(pDC, npszShells, 20,  FW_NORMAL, CTEXT_COLOR);
				}

				MFC::LocalUnlock(hlocShells);
				MFC::LocalFree(hlocShells);
			}
			ReleaseDC(pDC);
			pDC = nullptr;
		}
	} else {

		if (m_pText) {
			pDC = GetDC();
			m_pText->RestoreBackground(pDC);
			delete m_pText;
			m_pText = nullptr;
			ReleaseDC(pDC);
			pDC = nullptr;
		}
	}//end if bFound&&...


	CFrameWnd ::OnMouseMove(nFlags, point) ;    // default action
	/*
	#ifdef _MACROS
	    EM("End MouseMove");
	#endif
	*/

}


void CMnkWindow::OnLButtonDown(UINT nFlags, CPoint point) {
	CRect   rectTemp;
	CSprite *pSpriteGlobe, *pSpriteChair;
	CDC     *pDC = nullptr;
	HCURSOR hOldCur;
	int i;
	CSound  *pChairWaveSound = nullptr,
	         *pGlobeWaveSound = nullptr;

	const POINT pointGlobeSprite = {14, 30};       // Top-Left Corner of painting Globe animation.
	const POINT pointChairSprite = {202, 21};     // Top-Left Corner to paint Chair animation.

	const RECT InkWell = {534, 41, 584, 66};
	const RECT NetShells = {324, 20, 384, 58};
	const RECT Pails = {197, 81, 246, 165};
	const RECT FishHook = {118, 34, 152, 98};
	const RECT ArmChair = {110, 116, 174, 209};
	const RECT TrvBrch = {580, 58, 618, 157};
	const RECT Globe = {15, 104, 83, 218};
	const RECT Chair = {266, 64, 317, 183};
	//const RECT Chair={273,103,351,180};
	const RECT Wheel = {427, 75, 500, 146};


	#ifdef _MACROS
	EM("OnLBDn");
	#endif

	/*Delete any cText objects upon Mouse Click */
	if (m_pText) {
		pDC = GetDC();
		m_pText->RestoreBackground(pDC);
		ReleaseDC(pDC);
		pDC = nullptr;
		delete m_pText;
		m_pText = nullptr;
	}

	rectTemp.SetRect(NEWGAME_LOCATION_X, NEWGAME_LOCATION_Y,
	                 NEWGAME_LOCATION_X + NEWGAME_WIDTH,
	                 NEWGAME_LOCATION_Y + NEWGAME_HEIGHT);

	if (rectTemp.PtInRect(point)) {
		if ((pGameParams->bPlayingMetagame && !bPlayedGameOnce) || (!pGameParams->bPlayingMetagame)) {
			bPlayedGameOnce = TRUE;
			StartGame();
		}
	} else {

		if (pGameParams->bSoundEffectsEnabled) {

			rectTemp.SetRect(Globe.left, Globe.top, Globe.right, Globe.bottom);
			if (rectTemp.PtInRect(point)) {
				#ifdef _MACROS3
				EM("Clicked On Globe");
				#endif
				CSound::waitWaveSounds();
				sndPlaySound(nullptr, 0);
				if ((pDC = GetDC())) {
					if ((pSpriteGlobe = new CSprite())) {
						hOldCur = MFC::SetCursor(hHourGlassCursor);

						if (pSpriteGlobe->LoadCels(pDC, GLOBE_SPRITE, 25)) {

							#ifdef _MACROS3
							EM("New pSpriteGlobe Cells loaded");
							#endif
							pSpriteGlobe->SetCel(-1);
							pSpriteGlobe->LinkSprite();

							if (pGameParams->bSoundEffectsEnabled)
								pGlobeWaveSound = new CSound(this, POP,
								                             SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);
							for (i = 0; i < 25; i++) {
								if (pGameParams->bSoundEffectsEnabled && i == 4) pGlobeWaveSound->play();   //begin playing sound at the 4th loop cycle, for sync.
								if (!pSpriteGlobe->PaintSprite(pDC, pointGlobeSprite.x, pointGlobeSprite.y)) {
									MFC::SetCursor(hOldCur);
									MessageBox("Can't Conduct Animation Anymore", "Insufficient Memory");
									break;
								}
								//MFC::Sleep(gSleepTime);
							}
							pSpriteGlobe->EraseSprite(pDC);
							pSpriteGlobe->UnlinkSprite();
							#ifdef _MACROS3
							EM("pGlobeSprite Unloaded");
							#endif
							MFC::SetCursor(hOldCur);
						} else {
							MFC::SetCursor(hOldCur);
							MessageBox("Unable to Play Animation", " Out Of Memory");
						}
						delete pSpriteGlobe;
					}
					ReleaseDC(pDC);
					pDC = nullptr;
				}       // end if pDC
			}       //  if rectTemp.SetRect(Globe.left,....)  over.
			else if ((point.x < Chair.right) && (point.x > Chair.left) && (point.y < Chair.bottom) && (point.y > Chair.top)) {
				CSound::waitWaveSounds();
				sndPlaySound(nullptr, 0);
				if ((pDC = GetDC())) {
					if ((pSpriteChair = new CSprite())) {
						hOldCur = MFC::SetCursor(hHourGlassCursor);
						if (pSpriteChair->LoadCels(pDC, CHAIR_SPRITE, 25)) {
							pSpriteChair->SetCel(-1);
							pSpriteChair->LinkSprite();

							if (pGameParams->bSoundEffectsEnabled) {
								pChairWaveSound = new CSound(this, HONK, SOUND_WAVE | SOUND_ASYNCH | SOUND_AUTODELETE | SOUND_QUEUE);
//			                       pChairWaveSound->play();
							}
							for (i = 0; i < 25; i++) {
								if (pGameParams->bSoundEffectsEnabled && i == 7) pChairWaveSound->play();   //begin playing sound at the 4th loop cycle, for sync.
								if (!pSpriteChair->PaintSprite(pDC, pointChairSprite.x, pointChairSprite.y)) {
									MFC::SetCursor(hOldCur);
									MessageBox("Can't paint anymore animation", "Insufficient Memory");
									break;
								}
								MFC::Sleep(110);    //10);
							}
							pSpriteChair->EraseSprite(pDC);
							pSpriteChair->UnlinkSprite();
						} else {
							MFC::SetCursor(hOldCur);
							MessageBox("Unable To Play Animation", "Insufficient Memory");
						}//end if pSpriteChair->LoadCels...
						MFC::SetCursor(hOldCur);
						delete pSpriteChair;
					}//end if pSpriteChair=new...
					ReleaseDC(pDC);
					pDC = nullptr;
				}   // end if pDC
			} else {                      //  elseif (point.x <Chair.right....) over.
				if (pGameParams->bSoundEffectsEnabled) { // play EasterEggs only if sounds enabled
					rectTemp.SetRect(InkWell.left, InkWell.top, InkWell.right, InkWell.bottom);
					if (rectTemp.PtInRect(point))
						sndPlaySound(INK, SND_ASYNC);

					rectTemp.SetRect(Wheel.left, Wheel.top, Wheel.right, Wheel.bottom);
					if (rectTemp.PtInRect(point))
						sndPlaySound(WHEEL, SND_ASYNC);

					rectTemp.SetRect(NetShells.left, NetShells.top, NetShells.right, NetShells.bottom);
					if (rectTemp.PtInRect(point))
						sndPlaySound(NETSHELL, SND_ASYNC);

					rectTemp.SetRect(Pails.left, Pails.top, Pails.right, Pails.bottom);
					if (rectTemp.PtInRect(point))
						sndPlaySound(PAILS, SND_ASYNC);

					rectTemp.SetRect(FishHook.left, FishHook.top, FishHook.right, FishHook.bottom);
					if (rectTemp.PtInRect(point))
						sndPlaySound(FISHHOOK, SND_ASYNC);

					rectTemp.SetRect(ArmChair.left, ArmChair.top, ArmChair.right, ArmChair.bottom);
					if (rectTemp.PtInRect(point))
						sndPlaySound(ARMCHAIR, SND_ASYNC);

					rectTemp.SetRect(TrvBrch.left, TrvBrch.top, TrvBrch.right, TrvBrch.bottom);
					if (rectTemp.PtInRect(point))
						sndPlaySound(BROCHURE, SND_ASYNC);
				}//end if pGameParams->...
			}// end else .... rectTemp block.
		}//END if   pGameParams->bsoundEffectsEnabled
	} // end else .
	#ifdef _MACROS
	EM("exiting OnLBDn");
	#endif
}


void CMnkWindow::OnLButtonUp(UINT nFlags, CPoint point) {

	CDC *pDC = nullptr;

	#ifdef _MACROS
	EM("OnLBUp");
	#endif


	/*Delete any cText objects upon Mouse Click */
	if (m_pText) {
		if ((pDC = GetDC())) {
			m_pText->RestoreBackground(pDC);
			ReleaseDC(pDC);
			pDC = nullptr;
			delete m_pText;
			m_pText = nullptr;
		}//end if pDC.
	}



//  insert mouse button processing code here
	if (nFlags & (MK_CONTROL | MK_SHIFT | MK_RBUTTON))
		// if control key, shift key, or right button down
		;       // ignore the click
	else {


		#ifdef _DEMO
#define TOGGLE(x)   ((x)=!(x))
		CMove* xpcMove = &m_cCurrentMove;
		int iPitToStartWith;

		xpcMove->m_iPlayer = 0;     //start from the human side.

		UpdateWindow();

		do {
			while (!m_bGameOver && !m_bComputer[xpcMove->m_iPlayer] && !(CMnk *)this->SearchMove(xpcMove, iPitToStartWith)) {
				(CMnk*)this->Move((CPit*)(m_xpcPits[xpcMove->m_iPlayer][iPitToStartWith + 2]));
				SetCrabSign(FALSE);
			}
			while (!m_bGameOver && m_bComputer[xpcMove->m_iPlayer] && !(CMnk *)this->SearchMove(xpcMove, iPitToStartWith)) {
				(CMnk*)this->Move((CPit*)(m_xpcPits[xpcMove->m_iPlayer][iPitToStartWith + 2]));
				SetCrabSign(TRUE);
			}
		} while (!m_bGameOver);
		#endif //_DEMO


		AcceptClick(point) ;    // process the mouse click


	}
	#ifdef _MACROS
	EM("exiting OnLBUp");
	#endif
}


void CMnkWindow::OnLButtonDblClk(UINT nFlags, CPoint point) {
//  insert mouse button processing code here
}


void CMnkWindow::OnMButtonDown(UINT nFlags, CPoint point) {
//  insert mouse button processing code here
}


void CMnkWindow::OnMButtonUp(UINT nFlags, CPoint point) {
//  insert mouse button processing code here
}


void CMnkWindow::OnMButtonDblClk(UINT nFlags, CPoint point) {
//  insert mouse button processing code here
}


void CMnkWindow::OnRButtonDown(UINT nFlags, CPoint point) {
	if (nFlags & MK_CONTROL) {
		gbTurnSoundsOff = !gbTurnSoundsOff; //F12 is pressed, toggle sounds ON/OFF.
		WritePrivateProfileString("Mankala", "MuteCrab", gbTurnSoundsOff ? "1" : "0", INI_FILENAME);
	}

}


void CMnkWindow::OnRButtonUp(UINT nFlags, CPoint point) {

}

void CMnkWindow::OnRButtonDblClk(UINT nFlags, CPoint point) {
//  insert mouse button processing code here

}




/*****************************************************************
 *
 * OnTimer
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Update sprite positions on the screen ; this is cyclic based on
 *  the interval specified when the timer was initiated.
 *
 *  This function is called when ever the interval timer generates
 *  an event message ; i.e. the timer fires.
 *
 * FORMAL PARAMETERS:
 *
 *  nIDEvent    identifies the particular timer that fired
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

void CMnkWindow::OnTimer(UINT_PTR nIDEvent) {
//   CDC    *m_xpcDC ;


	if (nIDEvent == PITBUTTON_TIMER) {
		PostMessage(WM_RBUTTONUP, 0, 0L);
		MFC::KillTimer(this->m_hWnd, nIDEvent);
	} else {
		CFrameWnd::OnTimer(nIDEvent) ;
	}
}


/*****************************************************************
 *
 * OnClose
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This function is called when a Close event is generated.  For
 *  this sample application we need only kill our event timer ;
 *  The ExitInstance will handle releasing resources.
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
 *      n/a
 *
 ****************************************************************/

void CMnkWindow::OnClose() {
	CBrush Brush;
	CDC *pDC = nullptr;
	CRect rctTmp;
	NPSTR npszTmp;
	HLOCAL hlocTmp;
	int level;

	KillTimer(SPRITE_TIMER) ;

	if ((pDC = GetDC())) {              // paint black
		if (Brush.CreateStockObject(BLACK_BRUSH)) {
			rctTmp.SetRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
			pDC->FillRect(&rctTmp, &Brush);
		}
		ReleaseDC(pDC);
		pDC = nullptr;
	}

	hlocTmp = MFC::LocalAlloc(LHND, 16);
	npszTmp = (NPSTR)MFC::LocalLock(hlocTmp);

	Common::sprintf_s(npszTmp, 16, "%d", m_iStartStones);
	WritePrivateProfileString("Mankala", "StartStones", npszTmp, INI_FILENAME);

	switch (m_eLevel[0]) {
	case LEV_RANDOM:
		level = 0;
		break;
	case LEV_LOWEST:
		level = 1;
		break;
	case LEV_HIGHEST:
		level = 2;
		break;
	default:
	case LEV_EVAL:
		level = 3;
		break;
	}
	Common::sprintf_s(npszTmp, 16, "%d", level);
	WritePrivateProfileString("Mankala", "StartLevel", npszTmp, INI_FILENAME);

	MFC::LocalUnlock(hlocTmp);
	MFC::LocalFree(hlocTmp);

	if (m_xpGamePalette != nullptr) {
		m_xpGamePalette->DeleteObject();
		delete m_xpGamePalette;
		m_xpGamePalette = nullptr;
	}

	if (m_pText) {
		delete (m_pText);
		m_pText = nullptr;
	}

	ReleaseResources() ;    // release game specific resources

	CFrameWnd ::OnClose() ;

	MFC::PostMessage(ghParentWnd, WM_PARENTNOTIFY, WM_DESTROY, MAKELPARAM(m_hWnd, 0));
}


/*****************************************************************
 *
 * InitInstance
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This routine is automatically called when the application is
 *  started.
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
 *      BOOL        Success (TRUE) / Failure (FALSE) status
 *
 ****************************************************************/
/*
BOOL CMnkApp::InitInstance()
{
    CMnkWindow  *pMyMain ;

    SetDialogBkColor() ;                                // hook gray dialogs (was default in MFC V1)

    m_pMainWnd = pMyMain = m_xpcMnkWindow = new CMnkWindow() ;
            // make the main window visible
    m_pMainWnd->ShowWindow( m_nCmdShow ) ;
    m_pMainWnd->UpdateWindow() ;

    return(TRUE) ;
}
*/

inline void FlushMouseMessages(HWND hWnd) {
	MSG msg;
	// find and remove all mouse events
	while (PeekMessage(&msg, hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE));
}


LRESULT CMnkWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound* pSnd;

	pSnd = CSound::OnMCIStopped(wParam, lParam);

	#ifdef _MACROS
	EM("OnMCINotify");
	#endif
	return 0;
}

LRESULT CMnkWindow::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound* pSnd;

	pSnd = CSound::OnMMIOStopped(wParam, lParam);

	#ifdef _MACROS
	EM("OnMMIONotify");
	#endif

	return 0;
}

// CMnkWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CMnkWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CMnkWindow )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_CHAR()
	ON_WM_SYSCHAR()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_SYSKEYDOWN()
	ON_WM_KEYDOWN()
	ON_MESSAGE(MM_MCINOTIFY, CMnkWindow::OnMCINotify)
	ON_MESSAGE(MM_WOM_DONE, CMnkWindow::OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#ifdef BAGEL_DEBUG
BOOL ResetPitsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	static long lTotalStones;

	if (!gpcMove)
		MFC::EndDialog(hDlg, 0);

	switch (msg) {
	case WM_INITDIALOG:
		lTotalStones = lParam * NUMPITS * 2;
		return gpcMove->SetBackToOriginal(hDlg);
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			EndDialog(hDlg, 1);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		case IDAPPLY:
			gpcMove->ReConfig(hDlg, lTotalStones);
			break;
			/*
			case IDC_EDIT_HUMAN0:
			case IDC_EDIT_HUMAN1:
			case IDC_EDIT_HUMAN2:
			case IDC_EDIT_HUMAN3:
			case IDC_EDIT_HUMAN4:
			case IDC_EDIT_HUMAN5:
			                        gpcMove->RearrangeConfiguration(hDlg, (int)(wParam-IDC_EDIT_HUMAN0), (long)lTotalStones*NUMPITS*2 , 0);
			                        break;
			case IDC_EDIT_CRAB0:
			case IDC_EDIT_CRAB1:
			case IDC_EDIT_CRAB2:
			case IDC_EDIT_CRAB3:
			case IDC_EDIT_CRAB4:
			case IDC_EDIT_CRAB5:
			                        gpcMove->RearrangeConfiguration(hDlg,(int)( wParam-IDC_EDIT_CRAB0), lTotalStones*NUMPITS*2 , 1);
			*/
			break;
		case IDC_CRAB:
		case IDC_HUMAN:
			MFC::CheckRadioButton(hDlg, IDC_HUMAN, IDC_CRAB, wParam);
			gpcMove->SetPlayer(wParam - IDC_HUMAN);
			break;

		case IDRESET:
			memcpy(gpcMove, gpcStoreMove, sizeof(CMove));
			gpcMove->SetBackToOriginal(hDlg);
			break;

		default:
			break;
		}//end switch(wparam)
	default:
		break;
	}//end switch(msg)
	return FALSE;
}

BOOL CMove::SetBackToOriginal(HWND hwndDlg) {
	short i;
	Common::String tempStr;
	HWND hwndItem;

	SetDlgItemInt(hwndDlg, IDC_HOME_HUMAN, m_iNumStones[0][2 + HOMEINDEX], TRUE);
	SetDlgItemInt(hwndDlg, IDC_HOME_CRAB, m_iNumStones[1][2 + HOMEINDEX], TRUE);

	for (i = 0; i < NUMPITS; i++) {
		tempStr = Common::String::format("%d", m_iNumStones[1][i + 2]);
		hwndItem = MFC::GetDlgItem(hwndDlg, IDC_EDIT_CRAB0 + i);
		if (MFC::IsWindow(hwndItem))
			MFC::SetWindowText(hwndItem, tempStr.c_str());

		tempStr = Common::String::format("%d", m_iNumStones[0][i + 2]);
		hwndItem = MFC::GetDlgItem(hwndDlg, IDC_EDIT_HUMAN0 + i);
		if (MFC::IsWindow(hwndItem))
			MFC::SetWindowText(hwndItem, tempStr.c_str());
	}

	MFC::CheckRadioButton(hwndDlg, IDC_HUMAN, IDC_CRAB, m_iPlayer + IDC_HUMAN);

	return TRUE;
}

void CMove::ReConfig(HWND hwndDialog, long lTotalStartStones) {
	int Sum,
	    iPlayer,
	    iPit;
	HWND hwndItem;
	NPSTR npszItemText;
	HLOCAL hlocItemText;
	Common::String tempStr;

	hlocItemText = (HLOCAL)MFC::LocalAlloc(GHND, 10);
	npszItemText = (NPSTR)MFC::LocalLock(hlocItemText);



	/* Sum up all Stones not including hand or home bins , set NumStones*/
	for (iPlayer = 0, Sum = 0; iPlayer < 2; iPlayer++) {
		for (iPit = 0; iPit < NUMPITS; iPit++) {
			if (MFC::IsWindow(hwndItem = MFC::GetDlgItem(hwndDialog, iPit + (iPlayer ? IDC_EDIT_CRAB0 : IDC_EDIT_HUMAN0)))) {
				if (MFC::GetWindowText(hwndItem, npszItemText, 10))
					gpcMove->m_iNumStones[iPlayer][iPit + 2] = atoi(npszItemText);
			}//end if(MFC::IsWindow(...))

			Sum += gpcMove->m_iNumStones[iPlayer][iPit + 2];

		}
	}
	if ((Sum -= gpcMove->m_iNumStones[1][2 + HOMEINDEX]) < 0) {
		MFC::MessageBox(hwndDialog, "You are exceeding Total Allowable Stones", "", MB_OK);
	} else {
		gpcMove->m_iNumStones[0][2 + HOMEINDEX] = lTotalStartStones - Sum;
		tempStr = Common::String::format("%d",
		                                 gpcMove->m_iNumStones[0][2 + HOMEINDEX]);
		hwndItem = MFC::GetDlgItem(hwndDialog, IDC_HOME_HUMAN);
		MFC::SetWindowText(hwndItem, tempStr.c_str());
	}

	MFC::LocalUnlock(hlocItemText);
	MFC::LocalFree(hlocItemText);
}

void CMove::RearrangeConfiguration(HWND hDlg, int iThePit, long lTotalStartStones, BOOL b) {
	int Sum,
	    iPlayer,
	    iPit;
	HWND hwndItem;
	NPSTR npszItemText;
	HLOCAL hlocItemText;

	hlocItemText = (HLOCAL)MFC::LocalAlloc(GHND, 10);
	npszItemText = (NPSTR)MFC::LocalLock(hlocItemText);

	if (MFC::IsWindow(hwndItem = MFC::GetDlgItem(hDlg, iThePit + b ? IDC_EDIT_CRAB0 : IDC_EDIT_HUMAN0))) {
		if (MFC::GetWindowText(hwndItem, (LPSTR)npszItemText, 10))
			gpcMove->m_iNumStones[1][iThePit + 2] = atoi(npszItemText);

		/* Sum up all Stones not including hand or home bins */
		for (iPlayer = 0, Sum = 0; iPlayer < 2; iPlayer++) {
			for (iPit = 0; iPit < NUMPITS; iPit++) {
				Sum += gpcMove->m_iNumStones[iPlayer][iPit + 2];
			}
		}
		gpcMove->m_iNumStones[0][2 + HOMEINDEX] = lTotalStartStones - Sum - gpcMove->m_iNumStones[1][2 + HOMEINDEX];
	}

	MFC::LocalUnlock(hlocItemText);
	MFC::LocalFree(hlocItemText);
}


inline void CMove::SetPlayer(int iPlayer) {
	m_iPlayer = iPlayer;
}
void ObjSizes(void) {
	EM("CMOVE");
	DMint(sizeof CMove);
	EM("CMnk");
	DMint(sizeof CMnk);
	EM("CMnkWindow");
	DMint(sizeof CMnkWindow);
	EM("CBmpTable");
	DMint(sizeof CBmpTable);
	EM("CBmpObject");
	DMint(sizeof CBmpObject);
	EM("CPit");
	DMint(sizeof CPit);
	EM("CFileHeader");
	DMint(sizeof CFileHeader);
	EM("CMnkApp");
	DMint(sizeof CMnkApp);
	EM("CPitWnd");
	DMint(sizeof CPitWnd);
}
#endif

} // namespace Mankala
} // namespace HodjNPodj
} // namespace Bagel
