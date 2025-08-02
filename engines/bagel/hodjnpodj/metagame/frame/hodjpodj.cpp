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

#include "common/config-manager.h"
#include "bagel/afxwin.h"
#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"
#include "bagel/hodjnpodj/metagame/frame/hodjpodj.h"
#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/metagame/frame/app.h"
#include "bagel/hodjnpodj/metagame/frame/dialogs.h"
#include "bagel/hodjnpodj/metagame/frame/movytmpl.h"
#include "bagel/hodjnpodj/metagame/saves/restgame.h"
#include "bagel/hodjnpodj/metagame/saves/savegame.h"
#include "bagel/hodjnpodj/metagame/grand_tour/dllinit.h"
#include "bagel/hodjnpodj/metagame/grand_tour/gtstruct.h"
#include "bagel/hodjnpodj/metagame/gtl/dllinit.h"
#include "bagel/hodjnpodj/metagame/bgen/c1btndlg.h"
#include "bagel/hodjnpodj/metagame/zoom/dllinit.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

#define SAVEDLL         0

#define CONTROL_PHYSICAL_MEMORY     TRUE


VOID InitBFCInfo(CBfcMgr *);
VOID FreeBFCInfo(CBfcMgr *);

// Movie Identifiers
#define MOVIE_ID_NONE   0
#define MOVIE_ID_LOGO   1
#define MOVIE_ID_TITLE  2
#define MOVIE_ID_INTRO  3
#define MOVIE_ID_ENDING 4

#define LOGO_MIDI       ".\\SOUND\\MAINTITL.MID"

#define MOVIE_PATH      "..\\VIDEO"
#define LOGO_MOVIE      "..\\VIDEO\\LOGO.AVI"
#define TITLE_MOVIE     "..\\VIDEO\\TITLE.AVI"
#define STARTUP_MOVIE   "..\\VIDEO\\INTRO.AVI"
#define HODJ_WIN_MOVIE  "..\\VIDEO\\HODJ.AVI"
#define PODJ_WIN_MOVIE  "..\\VIDEO\\PODJ.AVI"

#define CREDITSFILE     ".\\CREDITS.TXT"

#define PATHSPECSIZE    256
#define CDROMHOME       ":\\HODJPODJ\\META"
#define TESTFILESPEC    "HODJPODJ.TST"

using GrandTour::LPGRANDTRSTRUCT;

typedef HWND (FAR PASCAL *FPZOOMFUNCT)(HWND, BOOL);
typedef HWND (FAR PASCAL *FPGTFUNCT)(HWND, LPGRANDTRSTRUCT);
typedef HWND (FAR PASCAL *FPMETAFUNCT)(HWND, CBfcMgr *, BOOL);
typedef DWORD (FAR PASCAL * FPGETFREEMEMINFO)(void);

static FPGETFREEMEMINFO lpfnGetFreeMemInfo;
// Flags when a .dll in the original is loaded

BOOL        bMetaLoaded = FALSE;

BOOL            bAnimationsEnabled = TRUE;
BOOL            bSoundEffectsEnabled = TRUE;
BOOL            bMusicEnabled = TRUE;
BOOL            bScrollingEnabled = TRUE;
BOOL            bSlowCPU = FALSE;
BOOL            bLowMemory = FALSE;

DWORD           dwFreeSpaceMargin;
DWORD           dwFreePhysicalMargin;

int                     nMidiVolume;
int                     nWaveVolume;

const CHAR *gpszSaveDLL = "HNPSAVE.DLL";
const CHAR *gpszSaveGameFile = "HODJPODJ.SAV";

static  BOOL        bScreenSaver;

static  CPalette    *pGamePalette = nullptr;
static  RECT        MainRect;                           // screen area spanned by the game window

int                                     nInstallCode;

BOOL                    bHomeWriteLocked = FALSE;
BOOL                    bPathsDiffer = FALSE;
static  char        chProfilePath[PATHSPECSIZE];
char                    chHomePath[PATHSPECSIZE];
char                    chMiniPath[PATHSPECSIZE];
char                    chCDPath[PATHSPECSIZE];
static  int         nHomeDrive = -1;
static  int         nMiniDrive = -1;
static  int         nCDDrive = -1;

static  BOOL        bActiveWindow = FALSE;

static  BOOL        bReturnToZoom = FALSE;
static  BOOL        bReturnToMeta = FALSE;
static  BOOL        bReturnToGrandTour = FALSE;

CBfcMgr             *lpMetaGame;
const char          *pszTest = "Corruption Test";

LPGAMESTRUCT        lpGameStruct = nullptr;
LPGRANDTRSTRUCT     lpGrandTour = nullptr;

CMovieWindow    *pMovie = nullptr;

int     nChallengePhase = 0;
int     nChallengeGame = 0;
long    lChallengeScore = 0;

CSound  *pBackgroundMidi = nullptr;

#define BELLCURVE   100
UBYTE     anBellCurve[BELLCURVE] = {
	1,
	2, 2,
	3, 3, 3,
	4, 4, 4, 4,
	5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8,
	9, 9, 9, 9, 9, 9, 9, 9, 9,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	11, 11, 11, 11, 11, 11, 11, 11, 11,
	12, 12, 12, 12, 12, 12, 12, 12,
	13, 13, 13, 13, 13, 13, 13,
	14, 14, 14, 14, 14, 14,
	15, 15, 15, 15, 15,
	16, 16, 16, 16,
	17, 17, 17,
	18, 18,
	19
};


#define MAX_CREDITS  12

#define CR_NEXT   0x0001
#define CR_ESCAPE 0x0002

struct CREDITS {
	const char *m_pszCelFile;
	UINT m_nDuration;
};

static const CREDITS stCredits[MAX_CREDITS] = {
	{ ".\\ART\\CREDIT1.BMP", 7000},
	{ ".\\ART\\CREDIT2.BMP", 15000},
	{ ".\\ART\\CREDIT3.BMP", 12000},
	{ ".\\ART\\CREDIT4.BMP", 5000},
	{ ".\\ART\\CREDIT5.BMP", 15000},
	{ ".\\ART\\CREDIT6.BMP", 13000},
	{ ".\\ART\\CREDIT7.BMP", 14000},
	{ ".\\ART\\CREDIT8.BMP", 15000},
	{ ".\\ART\\CREDIT9.BMP", 5000},
	{ ".\\ART\\CREDIT10.BMP", 7000},
	{ ".\\ART\\CREDIT11.BMP", 7000},
	{ ".\\ART\\CREDIT12.BMP", 8000}
};

// local prototypes
void LoadFloatLib();

/////////////////////////////////////////////////////////////////////////////


// CHodjPodjWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP(CHodjPodjWindow, CFrameWnd)
	//{{AFX_MSG_MAP( CHodjPodjWindow )
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_PARENTNOTIFY()
	ON_WM_ACTIVATE()
	ON_WM_KEYDOWN()
	ON_MESSAGE(MM_MCINOTIFY, CHodjPodjWindow::OnMCINotify)
	//    ON_MESSAGE( MM_MCINOTIFY, CHodjPodjWindow::OnMMNotify)
		//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*****************************************************************
 *
 * CHodjPodjWindow
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Create the window with the appropriate style, size, menu, etc.;
 * it will be later revealed by CTheApp::InitInstance().  Then
 * create our splash screen object by opening and loading its DIB.
 *
 * FORMAL PARAMETERS:
 *
 *  lUserAmount = initial amount of money that user starts with
 *                              defaults to zero
 *  nRounds         = the number of rounds to play, if 0 then not playing rounds
 *                          = defaults to zero
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
CHodjPodjWindow::CHodjPodjWindow() {
	BOOL    bSuccess;
	CDC     *pDC = nullptr;                        // device context for the screen
	CString WndClass;
	CSize   mySize;
	BOOL    bTestDibDoc;
	CRect   rQuitRect;
	CBitmap *pBitmap;

	// hack to load the WIN87EM DLL
	LoadFloatLib();

	// Inits
	m_pCurrentBmp = nullptr;
	m_nFlags = 0;
	m_bInCredits = FALSE;

	WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC, nullptr, nullptr, nullptr);

	MainRect.left = 0;
	MainRect.top = 0;
	MainRect.right = GAME_WIDTH;
	MainRect.bottom = GAME_HEIGHT;
	bTestDibDoc = Create(WndClass, "Boffo Games - Hodj 'n' Podj", WS_POPUP, MainRect, nullptr, 0);
	ASSERT(bTestDibDoc);

	BeginWaitCursor();
	ShowWindow(SW_SHOWNORMAL);
	BlackScreen();
	EndWaitCursor();

	pDC = GetDC();                                  // get a device context for our window

	bSuccess = CheckConfig(pDC);                    // verify we are runnable
	if (!bSuccess) {                                // ... and abort if not
		ReleaseDC(pDC);
		PostMessage(WM_CLOSE);
		return;
	}

	pBitmap = FetchBitmap(pDC, &pGamePalette, MAINSCROLL);
	delete pBitmap;
	ReleaseDC(pDC);

	bReturnToZoom = FALSE;
	bReturnToMeta = FALSE;
	bReturnToGrandTour = FALSE;

	hExeInst = AfxGetInstanceHandle();

	GetProfilePath();
	GetHomePath();
	GetCDPath();

	if (chHomePath[0] != chCDPath[0])
		bPathsDiffer = TRUE;

	GetProfileSettings();

	UpdateDLLRouting();

	lpMetaGame = &g_engine->_bfcMgr;

	// Check for skipping intro
	if (ConfMan.getBool("skip_intro")) {
		// Don't play the into movie
		PostMessage(WM_COMMAND, IDC_MAINDLG);
	} else {
		// Play the intro movie
		PostMessage(WM_COMMAND, IDC_PLAY_LOGO_MOVIE);
	}
}


void CHodjPodjWindow::GetProfileSettings(void) {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();
	nInstallCode = pMyApp->GetProfileInt("Meta", "InstallCode", INSTALL_NONE);
	bAnimationsEnabled = pMyApp->GetProfileInt("Meta", "Animations", TRUE);
	bScrollingEnabled = pMyApp->GetProfileInt("Meta", "MapScrolling", FALSE);
	bSoundEffectsEnabled = pMyApp->GetProfileInt("Meta", "SoundEffects", TRUE);
	bMusicEnabled = pMyApp->GetProfileInt("Meta", "Music", TRUE);
	nMidiVolume = pMyApp->GetProfileInt("Meta", "MidiVolume", (VOLUME_INDEX_MAX * 3) >> 2);
	nWaveVolume = pMyApp->GetProfileInt("Meta", "WaveVolume", (VOLUME_INDEX_MAX * 3) >> 2);

	bSlowCPU = (GetWinFlags() & WF_CPU386) ? TRUE : FALSE;

	CSound::setVolume(nMidiVolume, nWaveVolume);
}


void CHodjPodjWindow::SaveProfileSettings(void) {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();
	pMyApp->WriteProfileInt("Meta", "Animations", bAnimationsEnabled);
	pMyApp->WriteProfileInt("Meta", "MapScrolling", bScrollingEnabled);
//      pMyApp->WriteProfileInt("Meta","SoundEffects",bSoundEffectsEnabled);
//      pMyApp->WriteProfileInt("Meta","Music",bMusicEnabled);
}


void LoadFloatLib(void) {
	double fHack = 3.1415;

	fHack = fHack / 1.4;
}


BOOL CHodjPodjWindow::CheckConfig(CDC *pDC) {
	int nDevCaps;

	bLowMemory = CheckLowMemory();

	nDevCaps = (*pDC).GetDeviceCaps(BITSPIXEL);
	if (nDevCaps < 8) {
		error("Please set your display to 256 colors before playing this game");
	} else {
		nDevCaps = (*pDC).GetDeviceCaps(RASTERCAPS);
		if (!(nDevCaps & RC_PALETTE))
			error("It is recommended that your display be set to 256 colors before playing");
	}

	return true;
}


BOOL CHodjPodjWindow::CheckLowMemory(void) {
	BOOL                    bMemoryProblem;
	DWORD                   dwInfo;
	WORD                    wFreePages;
	long                    lFreeBytes;
	DWORD                   dwFreeSpace;

	bMemoryProblem = FALSE;

	dwFreeSpace = GetFreeSpace(0);

	dwFreeSpaceMargin = dwFreeSpace >> 2;
	if (dwFreeSpaceMargin < 850000L)
		dwFreeSpaceMargin = 850000L;

	dwFreePhysicalMargin = dwFreeSpaceMargin;

	if (dwFreeSpace < 3145728L)
		bMemoryProblem = TRUE;
	#if CONTROL_PHYSICAL_MEMORY
	else {
		lpfnGetFreeMemInfo = (FPGETFREEMEMINFO)GetProcAddress(GetModuleHandle("KERNEL"), "GETFREEMEMINFO");
		if (lpfnGetFreeMemInfo != nullptr) {
			dwInfo = lpfnGetFreeMemInfo();
			if (dwInfo != -1L) {
				wFreePages = HIWORD(dwInfo);
				lFreeBytes = (long) wFreePages * 4096L;
				if (lFreeBytes < 1400000L) {
					bMemoryProblem = TRUE;
					dwFreePhysicalMargin = 200000L;
				} else
					dwFreePhysicalMargin = lFreeBytes / 10;
			}
		}
	}
	#endif

	return (bMemoryProblem);
}


/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.  Ensures that the entire client area
 * of the main screen window is repainted, not just the portion in the
 * update region; see SplashScreen();
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

void CHodjPodjWindow::OnPaint() {
	CDC *pDC;

	if (dllLoaded) {
		PAINTSTRUCT lpPaint;

		Invalidate(FALSE);
		BeginPaint(&lpPaint);
		EndPaint(&lpPaint);

	} else {

		if (m_pCurrentBmp != nullptr) {
			if ((pDC = GetDC()) != nullptr) {
				PaintBitmap(pDC, nullptr, m_pCurrentBmp, 0, 0);
				ReleaseDC(pDC);
			}

		} else {
			CPaintDC dc(this);
			CBrush  Brush(RGB(0, 0, 0));

			dc.FillRect(&MainRect, &Brush);
		}
	}
}

/*****************************************************************
 *
 * SetupNewMeta
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

BOOL CHodjPodjWindow::SetupNewMeta() {
	CMetaSetupDlg   cMetaSetupDlg((CWnd *)this, pGamePalette);
	int             nMetaSetupReturn = 0;

	InitBFCInfo(lpMetaGame);

	cMetaSetupDlg.SetInitialOptions(lpMetaGame);
	nMetaSetupReturn = cMetaSetupDlg.DoModal();

	if (nMetaSetupReturn == 0)
		return (FALSE);
	else
		return (TRUE);
}

void CHodjPodjWindow::PlayMovie(const int nMovieId, const char *pszMovie, BOOL bScroll) {
	POINT   ptMovie;

	BlackScreen();

	pMovie.nMovieId = nMovieId;

	ptMovie.x = (MainRect.right / 2) - (MOVIE_WIDTH / 2);
	ptMovie.y = (MainRect.bottom / 2) - (MOVIE_HEIGHT / 2);

	PositionAtCDPath();

	if (pMovie.BlowWindow((CWnd *)this, bScroll, pszMovie, ptMovie.x, ptMovie.y) == TRUE) {
		if (nMovieId != MOVIE_ID_TITLE)
			ShowCursor(FALSE);
	} else {
		ShowCursor(TRUE);

		PositionAtHomePath();
		PostMessage(WM_COMMAND, IDC_MAINDLG);
	}
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
BOOL CHodjPodjWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	// this must be before the CMainGameDlg constructor
	PositionAtHomePath();

	BOOL            bSuccess;
	CMainGameDlg    cMainDlg((CWnd *)this, pGamePalette);
	int             nMainDlgReturn = 0;
	CDC            *pDC = nullptr;
	CWnd           *pWnd = nullptr;
	int                         nMovieId;

	switch (wParam) {
	case MOVIE_OVER:
		PositionAtHomePath();

		nMovieId = pMovie.nMovieId;

		pWnd = (CWnd *)lParam;
		if (pWnd != nullptr) {
			(*pWnd).SendMessage(WM_CLOSE, 0, 0L);
			delete pWnd;
		}

		BlackScreen();

		if (nMovieId == MOVIE_ID_LOGO) {
			ShowCursor(TRUE);
			StartBackgroundMidi();
			ShowCursor(FALSE);
			PostMessage(WM_COMMAND, IDC_PLAY_TITLE_MOVIE);

		} else if (nMovieId == MOVIE_ID_ENDING) {
			ShowCursor(TRUE);
			ShowCredits();
			BlackScreen();
			PostMessage(WM_COMMAND, IDC_MAINDLG);

		} else {
			ShowCursor(TRUE);
			PostMessage(WM_COMMAND, IDC_MAINDLG);
		}
		break;

	case IDC_MAINDLG:
		StartBackgroundMidi();
		PositionAtHomePath();
		nMainDlgReturn = cMainDlg.DoModal();

		switch (nMainDlgReturn) {

		case IDC_PLAY_META:
			if (SetupNewMeta())
				PostMessage(WM_COMMAND, IDC_META);
			else
				PostMessage(WM_COMMAND, IDC_MAINDLG);

			return (TRUE);

		case IDC_PLAY_MINI:
			PostMessage(WM_COMMAND, IDC_ZOOM);
			return (TRUE);

		case IDC_RESTORE_GAME:
			StopBackgroundMidi();

			if (Restore())
				PostMessage(WM_COMMAND, IDC_META);
			else
				PostMessage(WM_COMMAND, IDC_MAINDLG);

			return (TRUE);

		case IDC_GRAND_TOUR:
			PostMessage(WM_COMMAND, IDC_GRANDTOUR);
			break;

		case IDC_RESTART_MOVIE:
			StopBackgroundMidi();
			PlayMovie(MOVIE_ID_INTRO, STARTUP_MOVIE, TRUE);
			break;

		case IDC_QUIT_GAME:
			StopBackgroundMidi();
			PostMessage(WM_CLOSE);
			return (TRUE);

		default:
			PostMessage(WM_COMMAND, IDC_MAINDLG);
			break;
		}
		break;

	case IDC_PLAY_LOGO_MOVIE:
		PlayMovie(MOVIE_ID_LOGO, LOGO_MOVIE, FALSE);
		break;

	case IDC_PLAY_TITLE_MOVIE:
		PlayMovie(MOVIE_ID_TITLE, TITLE_MOVIE, FALSE);
		break;

	case IDC_PLAY_HODJ_MOVIE:
		StopBackgroundMidi();
		PlayMovie(MOVIE_ID_ENDING, HODJ_WIN_MOVIE, TRUE);
		break;

	case IDC_PLAY_PODJ_MOVIE:
		StopBackgroundMidi();
		PlayMovie(MOVIE_ID_ENDING, PODJ_WIN_MOVIE, TRUE);
		break;

	case IDC_ZOOM:
		BlackScreen();

		bSuccess = LoadZoomDLL();
		if (!bSuccess)
			PostMessage(WM_COMMAND, IDC_MAINDLG);
		break;

	case IDC_GRANDTOUR:
		BlackScreen();

		if (lpGrandTour != nullptr) {
			if (lpGameStruct != nullptr)
				lpGameStruct = nullptr;
			delete lpGrandTour;
			lpGrandTour = nullptr;
		}
		bSuccess = LoadGrandTourDLL();
		if (!bSuccess)
			PostMessage(WM_COMMAND, IDC_MAINDLG);
		break;

	case IDC_META:
		StopBackgroundMidi();
		BlackScreen();

		bSuccess = LoadMetaDLL();
		if (!bSuccess)
			PostMessage(WM_COMMAND, IDC_MAINDLG);
		break;
	}

	if (pDC != nullptr)
		ReleaseDC(pDC);

	return (TRUE);
}


void CHodjPodjWindow::OnMouseMove(UINT nFlags, CPoint point) {
	HCURSOR hNewCursor = nullptr;
	CWinApp *pMyApp;

	if (!bActiveWindow)
		return;

	pMyApp = AfxGetApp();

	hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);

	//if (hNewCursor != nullptr);
	SetCursor(hNewCursor);

	CWnd::OnMouseMove(nFlags, point);
}


void CHodjPodjWindow::OnLButtonDown(UINT nFlags, CPoint point) {
	if (m_bInCredits) {
		m_nFlags = CR_NEXT;
	} else {
		CFrameWnd::OnLButtonDown(nFlags, point);
	}
}

void CHodjPodjWindow::OnRButtonDown(UINT nFlags, CPoint point) {
	if (m_bInCredits) {
		m_nFlags = CR_NEXT;
	} else {
		CFrameWnd::OnRButtonDown(nFlags, point);
	}
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

BOOL CHodjPodjWindow::OnEraseBkgnd(CDC *) {
	return TRUE;
}


void CHodjPodjWindow::BlackScreen(void) {
	CDC         *pDC;
	CBrush      Brush(RGB(0, 0, 0));
	CPalette    *pPalOld = nullptr;

	ValidateRect(nullptr);
	pDC = GetDC();

	pDC->FillRect(&MainRect, &Brush);

	if (pGamePalette != nullptr) {                                  // map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pGamePalette, FALSE);
		(void)(*pDC).RealizePalette();
	}

	pDC->FillRect(&MainRect, &Brush);

	if (pPalOld != nullptr)                                 // relinquish the resources we built
		(void)(*pDC).SelectPalette(pPalOld, FALSE);

	ReleaseDC(pDC);
}


void CHodjPodjWindow::OnActivate(UINT nState, CWnd *, BOOL) {
	switch (nState) {

	case WA_INACTIVE:
		bActiveWindow = FALSE;
		break;

	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		#ifdef TODO
		if ((pMovie != nullptr) && (pMovie->pDum->m_hWnd != nullptr)) {
			SetFocus(pMovie->pDum->m_hWnd);
			break;
		} else {
			if ((pMovie != nullptr) && (pMovie->hWndDum != nullptr)) {
				SetFocus(pMovie->hWndDum);
				break;
			}
		}

		if (hwndGame != nullptr)
			::SetActiveWindow(hwndGame);
		else {
			bActiveWindow = TRUE;
			bUpdateNeeded = GetUpdateRect(nullptr, FALSE);
			if (bUpdateNeeded)
				InvalidateRect(nullptr, FALSE);
		}
		#endif
		break;
	}
}


void CHodjPodjWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if (nChar == VK_ESCAPE) {
		m_nFlags = CR_ESCAPE;
	} else {
		m_nFlags = CR_NEXT;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


LRESULT CHodjPodjWindow::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	(void) CSound::OnMCIStopped(wParam, lParam);
	return (0L);
}


/*****************************************************************
 *
 * LoadNewDLL
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

void    CHodjPodjWindow::LoadNewDLL(LPARAM lParam) {
	BOOL    bSuccess = FALSE;
	int     nWhichDLL;
	BOOL    bLoadedDLL;
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	nWhichDLL = (int)lParam;

	BlackScreen();

	FreeCurrentDLL();

	nWhichDLL -= MG_GAME_BASE;

	if ((nWhichDLL > MG_GAME_COUNT) || (nWhichDLL < 0)) {

		if (bReturnToZoom) {
			StartBackgroundMidi();
			bSuccess = LoadZoomDLL();
		}

		if (bReturnToMeta) {
			StopBackgroundMidi();
			lpMetaGame->m_bRestart = TRUE;
			bSuccess = LoadMetaDLL();
		}

		if (bReturnToGrandTour) {
			StartBackgroundMidi();
			bSuccess = LoadGrandTourDLL();
		}

		return;
	}

	bLoadedDLL = FALSE;

	if (lParam == MG_GAME_CHALLENGE) {
		StartBackgroundMidi();
		bSuccess = LoadZoomDLL();
		bReturnToZoom = FALSE;
		if (bSuccess == FALSE) {
			lpMetaGame->m_bRestart = TRUE;
			bSuccess = LoadMetaDLL();
		} else
			StartBackgroundMidi();
		return;
	}

	StopBackgroundMidi();

	if (CMgStatic::cGameTable[ nWhichDLL ]._initFn != nullptr) {
		FPDLLFUNCT  lpfnGame;
		Common::strcpy_s(chMiniPath, CMgStatic::cGameTable[ nWhichDLL ]._path);

		if (bReturnToZoom) {
			if (lpGameStruct != nullptr) {
				delete lpGameStruct;
				lpGameStruct = nullptr;
			}
			lpGameStruct = new GAMESTRUCT;
			lpGameStruct->lCrowns = 1000;
			lpGameStruct->lScore = 0;
			lpGameStruct->nSkillLevel = SKILLLEVEL_MEDIUM;
			bSoundEffectsEnabled = pMyApp->GetProfileInt("Meta", "SoundEffects", TRUE);
			bMusicEnabled = pMyApp->GetProfileInt("Meta", "Music", TRUE);
			lpGameStruct->bSoundEffectsEnabled = bSoundEffectsEnabled;
			lpGameStruct->bMusicEnabled = bMusicEnabled;
			lpGameStruct->bPlayingMetagame = FALSE;
			lpGameStruct->bPlayingHodj = TRUE;
		}

		if (CMgStatic::cGameTable[nWhichDLL]._initFn) {
			dllLoaded = true;
			lpfnGame = CMgStatic::cGameTable[nWhichDLL]._initFn;

			if ((lpfnGame != nullptr) && PositionAtMiniPath(nWhichDLL)) {
				if (bReturnToZoom) {
					g_wndGame = CWnd::FromHandle(lpfnGame(m_hWnd, lpGameStruct));
				} else {
					if (bReturnToGrandTour) {
						g_wndGame = CWnd::FromHandle(lpfnGame(m_hWnd, &lpGrandTour->stMiniGame));
					} else {
						g_wndGame = CWnd::FromHandle(lpfnGame(m_hWnd, &lpMetaGame->m_stGameStruct));
					}
				}
				bLoadedDLL = TRUE;
			}
		}

		if (bLoadedDLL == FALSE) {
			CString     cTmp;

			(void) PositionAtHomePath();

			cTmp = "Unable to find or load the ";
			cTmp += CMgStatic::cGameTable[ nWhichDLL ].m_lpszGameName;
			cTmp += " game!!!";
			MessageBox(cTmp, "Game Launch Error");
			MessageBox("Verify proper game installation and try again.");

			if (bReturnToZoom)
				bSuccess = LoadZoomDLL();

			if (bReturnToMeta) {
				lpMetaGame->m_bRestart = TRUE;
				bSuccess = LoadMetaDLL();
			}
		}
	}
}


void CHodjPodjWindow::FreeCurrentDLL(void) {
	delete g_wndGame;
	g_wndGame = nullptr;
	dllLoaded = false;
}


BOOL CHodjPodjWindow::LoadMetaDLL(void) {
	FreeCurrentDLL();
	Metagame::Gtl::RunMeta(m_hWnd, lpMetaGame, FALSE);

	bReturnToMeta = TRUE;
	return TRUE;
}

BOOL CHodjPodjWindow::LoadZoomDLL(void) {
	dllLoaded = true;
	g_wndGame = CWnd::FromHandle(Metagame::Zoom::RunZoomMap(m_hWnd,
		nChallengePhase == 0));

	bReturnToZoom = TRUE;
	return TRUE;
}


BOOL CHodjPodjWindow::LoadGrandTourDLL(void) {
	if (lpGrandTour == nullptr) {
		int i;

		lpGrandTour = new GrandTour::GRANDTRSTRUCT();
		lpGrandTour->bMidGrandTour = FALSE;
		lpGrandTour->nHodjSkillLevel = SKILLLEVEL_LOW;
		lpGrandTour->nPodjSkillLevel = NOPLAY;
		lpGrandTour->nGameSelection = GAME_ALPHA;
		lpGrandTour->nCurrGameCode = 0;
		lpGrandTour->nHodjScore = 0;
		lpGrandTour->nPodjScore = 0;
		lpGrandTour->nHodjLastGame = 0;
		lpGrandTour->nPodjLastGame = 0;
		lpGrandTour->nHodjLastScore = 0;
		lpGrandTour->nPodjLastScore = 0;
		lpGrandTour->bPlayMusic = bMusicEnabled;
		lpGrandTour->bPlayFX = bSoundEffectsEnabled;
		lpGrandTour->bPlayingHodj = TRUE;
		for (i = 0; i < 18; i++) {
			lpGrandTour->abHGamePlayed[i] = FALSE;
			lpGrandTour->abPGamePlayed[i] = FALSE;
		}
		lpGrandTour->stMiniGame.lCrowns = 0;
		lpGrandTour->stMiniGame.lScore = 0;
		lpGrandTour->stMiniGame.nSkillLevel = SKILLLEVEL_LOW;
		lpGrandTour->stMiniGame.bSoundEffectsEnabled = bSoundEffectsEnabled;
		lpGrandTour->stMiniGame.bMusicEnabled = bMusicEnabled;
		lpGrandTour->stMiniGame.bPlayingMetagame = TRUE;
		lpGrandTour->stMiniGame.bPlayingHodj = TRUE;
	}

	dllLoaded = true;
	g_wndGame = CWnd::FromHandle(Metagame::GrandTour::RunGrandTour(m_hWnd, lpGrandTour));
	bReturnToGrandTour = TRUE;
	return TRUE;
}


void CHodjPodjWindow::UpdateDLLRouting() {
	// No DLLs under ScummVM
}


LPARAM CHodjPodjWindow::UpdateChallengePhase(LPARAM lParam) {
	LPARAM  nGameID = 0;

	nGameID = lParam;

	BlackScreen();

	switch (nChallengePhase) {
	case 1:   // goto the zoom map and determine the game
		if (lpMetaGame->m_stGameStruct.bPlayingHodj) {
			C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", "Podj chooses", "the game.");
			cMsgBox.DoModal();
		} else {
			C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", "Hodj chooses", "the game.");
			cMsgBox.DoModal();
		}

		nChallengePhase++;

		if ((lpMetaGame->m_stGameStruct.bPlayingHodj) && (lpMetaGame->m_cPodj.m_bComputer)) {
			int nGameIndex;
			do {
				nGameIndex = brand() % (MG_GAME_COUNT - 1);
			} while (CMgStatic::cGameTable[nGameIndex]._initFn == nullptr);

			nGameID = (LPARAM)(nGameIndex + MG_GAME_BASE);

			{
				C1ButtonDialog cMsgBox((CWnd *)this, pGamePalette, "&OK", "Podj chooses :", " ", CMgStatic::cGameTable[nGameIndex].m_lpszGameName);
				cMsgBox.DoModal();
			}
			goto PHASE2;
		}
		break;

	case 2:   // send the challenger off to play the mini game and store the Game Code
PHASE2:
		// set up mini-game structure
		lpMetaGame->m_stGameStruct.lCrowns = 50;
		if (lpMetaGame->m_stGameStruct.bPlayingHodj) {
			C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", " ", "Hodj goes first.");
			cMsgBox.DoModal();
		} else {
			C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", " ", "Podj goes first.");
			cMsgBox.DoModal();
		}

		nChallengeGame  = (int)nGameID;
		nChallengePhase++;

		if ((lpMetaGame->m_stGameStruct.bPlayingHodj == FALSE) && (lpMetaGame->m_cPodj.m_bComputer)) {
			SetComputerScore();
			goto PHASE3;
		}
		break;

	case 3:   // check to see the challenger's score > 0, if so send the challengee, if not return to the Meta
PHASE3:
		if ((lChallengeScore = DetermineChallengeScore()) > 0) {
			if (lpMetaGame->m_stGameStruct.bPlayingHodj) {
				C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", " ", "Podj's turn.");
				cMsgBox.DoModal();
			} else {
				C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", " ", "Hodj's turn.");
				cMsgBox.DoModal();
			}

			if (lpMetaGame->m_stGameStruct.bPlayingHodj) {
				lpMetaGame->m_stGameStruct.bPlayingHodj = FALSE;
				lpMetaGame->m_stGameStruct.nSkillLevel = lpMetaGame->m_cPodj.m_iSkillLevel;
				lpMetaGame->m_stGameStruct.lCrowns = 50;
			} else {
				lpMetaGame->m_stGameStruct.bPlayingHodj = TRUE;
				lpMetaGame->m_stGameStruct.nSkillLevel = lpMetaGame->m_cHodj.m_iSkillLevel;
				lpMetaGame->m_stGameStruct.lCrowns = 50;
			}
			lpMetaGame->m_stGameStruct.lScore = 0;

			nGameID  =  nChallengeGame;
		} else {
			lChallengeScore = 0l;
			if (lpMetaGame->m_stGameStruct.bPlayingHodj) {
				C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Hodj Challenges", "Podj keeps", "Mish and Mosh.");
				cMsgBox.DoModal();
			} else {
				C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Podj Challenges", "Hodj keeps", "Mish and Mosh.");
				cMsgBox.DoModal();
			}

			lpMetaGame->m_cHodj.m_iFurlongs = 0;
			lpMetaGame->m_cPodj.m_iFurlongs = 0;
			nGameID = (long)lpMetaGame;
			nChallengePhase = 10;
		}
		nChallengePhase++;

		if ((lpMetaGame->m_stGameStruct.bPlayingHodj == FALSE) && (lpMetaGame->m_cPodj.m_bComputer) && (nChallengePhase < 10)) {
			SetComputerScore();
			goto PHASE4;
		}
		break;

	case 4:   // compare the player's score, if challenger won, give him mish and mosh, and another turn
PHASE4:
		if (lChallengeScore > DetermineChallengeScore()) {
			// give Mish and Mosh to challengee and give the turn back to the challengee
			if (lpMetaGame->m_cHodj.m_bHaveMishMosh) {
				CItem   *pItem = nullptr;
				{
					C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", "Podj has won Mish,", "Mosh and a turn.");
					cMsgBox.DoModal();
				}
				pItem = lpMetaGame->m_cHodj.m_pInventory->FindItem(MG_OBJ_MISH);
				lpMetaGame->m_cHodj.m_pInventory->RemoveItem(pItem);
				lpMetaGame->m_cPodj.m_pInventory->AddItem(pItem);
				pItem = nullptr;
				pItem = lpMetaGame->m_cHodj.m_pInventory->FindItem(MG_OBJ_MOSH);
				lpMetaGame->m_cHodj.m_pInventory->RemoveItem(pItem);
				lpMetaGame->m_cPodj.m_pInventory->AddItem(pItem);
				pItem = nullptr;
				lpMetaGame->m_cHodj.m_bHaveMishMosh = FALSE;
				lpMetaGame->m_cPodj.m_bHaveMishMosh = TRUE;
				lpMetaGame->m_cHodj.m_bMoving = TRUE;
				lpMetaGame->m_cPodj.m_bMoving = FALSE;
			} else {
				CItem   *pItem = nullptr;
				{
					C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", "Hodj has won Mish,", "Mosh and a turn.");
					cMsgBox.DoModal();
				}
				pItem = lpMetaGame->m_cPodj.m_pInventory->FindItem(MG_OBJ_MISH);
				lpMetaGame->m_cPodj.m_pInventory->RemoveItem(pItem);
				lpMetaGame->m_cHodj.m_pInventory->AddItem(pItem);
				pItem = nullptr;
				pItem = lpMetaGame->m_cPodj.m_pInventory->FindItem(MG_OBJ_MOSH);
				lpMetaGame->m_cPodj.m_pInventory->RemoveItem(pItem);
				lpMetaGame->m_cHodj.m_pInventory->AddItem(pItem);
				pItem = nullptr;
				lpMetaGame->m_cPodj.m_bHaveMishMosh = FALSE;
				lpMetaGame->m_cHodj.m_bHaveMishMosh = TRUE;
				lpMetaGame->m_cPodj.m_bMoving = TRUE;
				lpMetaGame->m_cHodj.m_bMoving = FALSE;
			}
		} else {
			if (lpMetaGame->m_stGameStruct.bPlayingHodj) {
				C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", "Hodj keeps", "Mish and Mosh.");
				cMsgBox.DoModal();
			} else {
				C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", "Podj keeps", "Mish and Mosh.");
				cMsgBox.DoModal();
			}
		}
		lpMetaGame->m_cHodj.m_iFurlongs = 0;
		lpMetaGame->m_cPodj.m_iFurlongs = 0;

		nChallengePhase++;
		nGameID = (long)lpMetaGame;
		break;

	default:
		nChallengeGame  =  0;
		nChallengePhase = 0;
		break;
	}
	return (nGameID);
}

void CHodjPodjWindow::SetComputerScore() {
	int anScrambledCurve[BELLCURVE];
	int nBottomScore = 0;
	int nTopScore = 0;
	int nBellScore = 0;
	int i, j;
	BOOL    bTemp;

	for (i = 0; i < BELLCURVE; i++)
		anScrambledCurve[i] = 0;

	for (i = 0; i < BELLCURVE; i++) {
		bTemp = FALSE;
		do {
			j = brand() % BELLCURVE;
			if (anScrambledCurve[j] == 0) {
				anScrambledCurve[j] = (int)anBellCurve[i];
				bTemp = TRUE;
			}
		} while (bTemp == FALSE);
	}

	switch (nChallengeGame) {

	case  MG_GAME_ARCHEROIDS: // 1 or 0
	case  MG_GAME_ARTPARTS: // 1 or 0
	case  MG_GAME_BATTLEFISH: // 1 or 0
	case  MG_GAME_MANKALA: // 1 or 0
	case  MG_GAME_MAZEODOOM: // 1 or 0
	case  MG_GAME_RIDDLES: // 1 or 0
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			lpMetaGame->m_stGameStruct.lScore = ProbableTrue(80);
			break;
		case SKILLLEVEL_MEDIUM :
			lpMetaGame->m_stGameStruct.lScore = ProbableTrue(60);
			break;
		case SKILLLEVEL_HIGH :
			lpMetaGame->m_stGameStruct.lScore = ProbableTrue(40);
			break;
		}
		break;

	case  MG_GAME_BEACON:
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			nBottomScore = 8;
			nTopScore = 40;
			break;
		case SKILLLEVEL_MEDIUM :
			nBottomScore = 6;
			nTopScore = 30;
			break;
		case SKILLLEVEL_HIGH :
			nBottomScore = 4;
			nTopScore = 20;
			break;
		}
		goto CALCSCORE;
		break;

	case  MG_GAME_THGESNGGME : // %
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			nBottomScore = 10;
			nTopScore = 60;
			break;
		case SKILLLEVEL_MEDIUM :
			nBottomScore = 10;
			nTopScore = 50;
			break;
		case SKILLLEVEL_HIGH :
			nBottomScore = 10;
			nTopScore = 40;
			break;
		}
		goto CALCSCORE;
		break;

	case  MG_GAME_LIFE : // number
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			nBottomScore = 24;
			nTopScore = 44;
			break;
		case SKILLLEVEL_MEDIUM :
			nBottomScore = 20;
			nTopScore = 40;
			break;
		case SKILLLEVEL_HIGH :
			nBottomScore = 16;
			nTopScore = 36;
			break;
		}
		goto CALCSCORE;
		break;

	case  MG_GAME_GARFUNKEL : // number
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			nBottomScore = 6;
			nTopScore = 18;
			break;
		case SKILLLEVEL_MEDIUM :
			nBottomScore = 5;
			nTopScore = 16;
			break;
		case SKILLLEVEL_HIGH :
			nBottomScore = 4;
			nTopScore = 14;
			break;
		}
		goto CALCSCORE;
		break;

	case  MG_GAME_WORDSEARCH : // number
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			nBottomScore = 6;
			nTopScore = 12;
			break;
		case SKILLLEVEL_MEDIUM :
			nBottomScore = 5;
			nTopScore = 11;
			break;
		case SKILLLEVEL_HIGH :
			nBottomScore = 4;
			nTopScore = 10;
			break;
		}
		goto CALCSCORE;
		break;

	case  MG_GAME_BARBERSHOP : // number of cards discarded
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			nBottomScore = 56;
			nTopScore = 62;
			break;
		case SKILLLEVEL_MEDIUM :
			nBottomScore = 52;
			nTopScore = 62;
			break;
		case SKILLLEVEL_HIGH :
			nBottomScore = 48;
			nTopScore = 62;
			break;
		}
		goto CALCSCORE;
		break;

	case  MG_GAME_NOVACANCY : // number
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			nBottomScore = 0;
			nTopScore = 10;
			break;
		case SKILLLEVEL_MEDIUM :
			nBottomScore = 0;
			nTopScore = 12;
			break;
		case SKILLLEVEL_HIGH :
			nBottomScore = 0;
			nTopScore = 14;
			break;
		}
		goto CALCSCORE;
		break;

	case  MG_GAME_DAMFURRY : // number
		nBottomScore = 30;
		nTopScore = 45;
		goto CALCSCORE;
		break;

	case  MG_GAME_PACRAT : // number
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			nBottomScore = 300;
			nTopScore = 3000;
			break;
		case SKILLLEVEL_MEDIUM :
			nBottomScore = 600;
			nTopScore = 6000;
			break;
		case SKILLLEVEL_HIGH :
			nBottomScore = 900;
			nTopScore = 9000;
			break;
		}
		goto CALCSCORE;
		break;

	case  MG_GAME_VIDEOPOKER : // crowns
		nBottomScore = 0;
		nTopScore = 150;
		goto CALCSCORE;
		break;

	case  MG_GAME_FUGE : // number
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			if (ProbableTrue(20)) {
				lpMetaGame->m_stGameStruct.lScore = 21;
				goto ENDCASE;
			}
			nBottomScore = 1;
			nTopScore = 15;
			break;
		case SKILLLEVEL_MEDIUM :
			if (ProbableTrue(15)) {
				lpMetaGame->m_stGameStruct.lScore = 37;
				goto ENDCASE;
			}
			nBottomScore = 1;
			nTopScore = 31;
			break;
		case SKILLLEVEL_HIGH :
			if (ProbableTrue(10)) {
				lpMetaGame->m_stGameStruct.lScore = 53;
				goto ENDCASE;
			}
			nBottomScore = 1;
			nTopScore = 47;
			break;
		}
		break;

	case  MG_GAME_CRYPTOGRAMS : // number
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			if (ProbableTrue(30)) {
				lpMetaGame->m_stGameStruct.lScore = 100;
				goto ENDCASE;
			}
			nBottomScore = 24;
			nTopScore = 48;
			break;
		case SKILLLEVEL_MEDIUM :
			if (ProbableTrue(25)) {
				lpMetaGame->m_stGameStruct.lScore = 100;
				goto ENDCASE;
			}
			nBottomScore = 20;
			nTopScore = 48;
			break;
		case SKILLLEVEL_HIGH :
			if (ProbableTrue(20)) {
				lpMetaGame->m_stGameStruct.lScore = 100;
				goto ENDCASE;
			}
			nBottomScore = 16;
			nTopScore = 48;
			break;
		}

CALCSCORE:
		nBellScore = (int)anBellCurve[brand() % BELLCURVE];
		lpMetaGame->m_stGameStruct.lScore = (((((nTopScore - nBottomScore) * 10) / 19) * nBellScore) / 10) + nBottomScore;
ENDCASE:
		break;

	case  MG_GAME_PEGGLEBOZ : // number
		i = brand() % 100;
		lpMetaGame->m_stGameStruct.lScore = 1;

		if (i < 5)
			lpMetaGame->m_stGameStruct.lScore = 25;

		if ((i >= 5) && (i < 13))
			lpMetaGame->m_stGameStruct.lScore = 10;

		if ((i >= 13) && (i < 25))
			lpMetaGame->m_stGameStruct.lScore = 5;

		if ((i >= 25) && (i < 40))
			lpMetaGame->m_stGameStruct.lScore = 4;

		break;

	}
}


// determines the challenge score on a 0-100 scale
long CHodjPodjWindow::DetermineChallengeScore() {
	char    cTemp[32];
	long    lReturn = 0L;
	long    lTemp = 0L;

	switch (nChallengeGame) {

	case  MG_GAME_ARCHEROIDS : // 1 or 0
	case  MG_GAME_ARTPARTS : // 1 or 0
	case  MG_GAME_BATTLEFISH : // 1 or 0
	case  MG_GAME_MANKALA : // 1 or 0
	case  MG_GAME_MAZEODOOM : // 1 or 0
	case  MG_GAME_RIDDLES : // 1 or 0
		lReturn = lpMetaGame->m_stGameStruct.lScore * 100;
		break;

	case  MG_GAME_BEACON : //  %
		lReturn = min(100, (lpMetaGame->m_stGameStruct.lScore * 2));
		break;

	case  MG_GAME_LIFE : // number
		lReturn = min(100, ((lpMetaGame->m_stGameStruct.lScore * 25) / 10));
		break;

	case  MG_GAME_THGESNGGME : // %
		lReturn = min(100, ((lpMetaGame->m_stGameStruct.lScore * 15) / 10));
		break;

	case  MG_GAME_CRYPTOGRAMS : // number
		lReturn = lpMetaGame->m_stGameStruct.lScore;
		break;

	case  MG_GAME_PEGGLEBOZ : // number
	case  MG_GAME_GARFUNKEL : // number
	case  MG_GAME_WORDSEARCH : // number
		lReturn = (lpMetaGame->m_stGameStruct.lScore * 100) / 25;
		break;

	case  MG_GAME_BARBERSHOP : // number of cards discarded
		lReturn = (lpMetaGame->m_stGameStruct.lScore * 100) / 62;
		break;

	case  MG_GAME_NOVACANCY : // number
		lReturn = 100 - ((lpMetaGame->m_stGameStruct.lScore * 100) / 45);
		break;

	case  MG_GAME_DAMFURRY : // number
		lReturn = (lpMetaGame->m_stGameStruct.lScore * 100) / 60;
		break;

	case  MG_GAME_FUGE : // number
		lReturn = (lpMetaGame->m_stGameStruct.lScore * 100) / 53;
		break;

	case  MG_GAME_PACRAT : // number
		lTemp = lpMetaGame->m_stGameStruct.lScore * 100;
		switch (lpMetaGame->m_stGameStruct.nSkillLevel) {
		case SKILLLEVEL_LOW :
			lReturn = lTemp / 2373;
			break;
		case SKILLLEVEL_MEDIUM :
			lReturn = lTemp / 14280;
			break;
		case SKILLLEVEL_HIGH :
			lReturn = lTemp / 28584;
			break;
		}
		break;

	case  MG_GAME_VIDEOPOKER : // number
		lReturn = lpMetaGame->m_stGameStruct.lCrowns;
		break;

	default:
		lReturn = 0;
		break;
	}

	if (nChallengeGame == MG_GAME_VIDEOPOKER) {
		Common::sprintf_s(cTemp, "%li", lReturn);
	} else {
		Common::sprintf_s(cTemp, "%li / 100", lReturn);
	}

	if (lpMetaGame->m_stGameStruct.bPlayingHodj) {
		C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Hodj's Score :", " ", cTemp);
		cMsgBox.DoModal();
	} else {
		C1ButtonDialog  cMsgBox((CWnd *)this, pGamePalette, "&OK", "Podj's Score :", " ", cTemp);
		cMsgBox.DoModal();
	}

	return (lReturn);
}


void CHodjPodjWindow::OnParentNotify(UINT msg, LPARAM lParam) {
	BOOL        bMainDlg = TRUE;
	LPARAM      nGameReturn;

	switch (msg) {
	case WM_DESTROY:

		if (bReturnToMeta && (lpMetaGame != nullptr)) {
			bSoundEffectsEnabled = (*lpMetaGame).m_stGameStruct.bSoundEffectsEnabled;
			bMusicEnabled = (*lpMetaGame).m_stGameStruct.bMusicEnabled;
			bScrollingEnabled = (*lpMetaGame).m_bScrolling;
		} else if (bReturnToGrandTour && (lpGrandTour != nullptr)) {
			bSoundEffectsEnabled = (*lpGrandTour).stMiniGame.bSoundEffectsEnabled;
			bMusicEnabled = (*lpGrandTour).stMiniGame.bMusicEnabled;
		}

		nGameReturn = lParam;

		if (dllLoaded) {
			(void) PositionAtHomePath();

			if (nGameReturn < 0) {
				bReturnToZoom = FALSE;
				bReturnToMeta = FALSE;
				bReturnToGrandTour = FALSE;
				nChallengePhase = 0;

			} else if (nGameReturn == MG_DLLX_HODJ_WINS) {

				bReturnToZoom = FALSE;
				bReturnToMeta = FALSE;
				bReturnToGrandTour = FALSE;
				bMainDlg = FALSE;
				nGameReturn = -1;
				nChallengePhase = 0;
				PostMessage(WM_COMMAND, IDC_PLAY_HODJ_MOVIE);

			} else if (nGameReturn == MG_DLLX_PODJ_WINS) {

				bReturnToZoom = FALSE;
				bReturnToMeta = FALSE;
				bReturnToGrandTour = FALSE;
				bMainDlg = FALSE;
				nGameReturn = -1;
				nChallengePhase = 0;
				PostMessage(WM_COMMAND, IDC_PLAY_PODJ_MOVIE);

			} else {
				if (nGameReturn == MG_GAME_CHALLENGE)
					nChallengePhase = 1;
				nGameReturn = UpdateChallengePhase(nGameReturn);
			}

			LoadNewDLL(nGameReturn);
			if (!dllLoaded) {
				UpdateWindow();
				if (bMainDlg)
					PostMessage(WM_COMMAND, IDC_MAINDLG);
			}

			return;
		}

		UpdateWindow();
		break;
	}

	CWnd::OnParentNotify(msg, lParam);
}


void CHodjPodjWindow::GetProfilePath() {
	chProfilePath[0] = '\0';
}


void CHodjPodjWindow::GetHomePath() {
}


BOOL CHodjPodjWindow::PositionAtHomePath() {
	AfxGetApp()->setDirectory("meta");
	return true;
}


BOOL CHodjPodjWindow::GetCDPath() {
	bPathsDiffer = false;
	return TRUE;
}


BOOL CHodjPodjWindow::PositionAtCDPath() {
	return TRUE;
}


BOOL CHodjPodjWindow::PositionAtMiniPath(int nWhichDLL) {
	const auto &game = CMgStatic::cGameTable[nWhichDLL];

	auto *app = AfxGetApp();
	app->setDirectory(game._path);
	app->addResources(game._dllName);

	return TRUE;
}


BOOL CHodjPodjWindow::FindCDROM() {
	return false;
}


BOOL CHodjPodjWindow::DriveWriteLocked(void) {
	return FALSE;
}


void CHodjPodjWindow::StartBackgroundMidi(void) {
	CWinApp *pMyApp;

	pMyApp = AfxGetApp();

	bMusicEnabled = pMyApp->GetProfileInt("Meta", "Music", TRUE);

	if (bMusicEnabled && (pBackgroundMidi == nullptr)) {
		PositionAtHomePath();
		pBackgroundMidi = new CSound(this, LOGO_MIDI, SOUND_MIDI | SOUND_LOOP /* | SOUND_DONT_LOOP_TO_END */);
		(*pBackgroundMidi).play();
	}
}


void CHodjPodjWindow::StopBackgroundMidi(void) {
	if (pBackgroundMidi != nullptr) {
		(*pBackgroundMidi).stop();
		delete pBackgroundMidi;
		pBackgroundMidi = nullptr;
	}
}


void CHodjPodjWindow::OnDestroy() {
	CFrameWnd::OnDestroy();
}


void CHodjPodjWindow::OnClose() {
#if 0
	if (dllLoaded) {
		if (hwndGame != nullptr)
			MFC::SetActiveWindow(hwndGame);
		return;
	}
#endif
	ReleaseResources();

	SaveProfileSettings();

	CFrameWnd::OnClose();
}


void CHodjPodjWindow::ReleaseResources(void) {
	CSound::clearSounds();

	if (bMetaLoaded) {
		if (hMetaInst > HINSTANCE_ERROR) {
			FreeLibrary(hMetaInst);
			hMetaInst = nullptr;
		}
	}

	if (lpGrandTour != nullptr) {
		delete lpGrandTour;
		lpGrandTour = nullptr;
	}

	if (lpGameStruct != nullptr) {
		delete lpGameStruct;
		lpGameStruct = nullptr;
	}

	assert(lpMetaGame != nullptr);
	if (lpMetaGame != nullptr) {
		FreeBFCInfo(lpMetaGame);
		delete lpMetaGame;
		lpMetaGame = nullptr;
	}

	if (pGamePalette != nullptr) {
		delete pGamePalette;
		pGamePalette = nullptr;
	}
}

VOID FreeBFCInfo(CBfcMgr *pBfcMgr) {
	CHodjPodj *pPlayer;
	int i, k;

	assert(pBfcMgr != nullptr);

	pBfcMgr->m_bRestoredGame = FALSE;

	// delete any Mish/Mosh items
	//
	if (pBfcMgr->m_pMishItem != nullptr) {
		delete pBfcMgr->m_pMishItem;
		pBfcMgr->m_pMishItem = nullptr;
	}
	if (pBfcMgr->m_pMoshItem != nullptr) {
		delete pBfcMgr->m_pMoshItem;
		pBfcMgr->m_pMoshItem = nullptr;
	}

	pPlayer = &pBfcMgr->m_cHodj;
	for (k = 0; k < 2; k++) {

		if (k == 1) {
			pPlayer = &pBfcMgr->m_cPodj;
		}

		if (pPlayer->m_pBlackMarket != nullptr) {
			delete pPlayer->m_pBlackMarket;
			pPlayer->m_pBlackMarket = nullptr;
		}

		if (pPlayer->m_pGenStore != nullptr) {
			delete pPlayer->m_pGenStore;
			pPlayer->m_pGenStore = nullptr;
		}

		if (pPlayer->m_pTradingPost != nullptr) {
			delete pPlayer->m_pTradingPost;
			pPlayer->m_pTradingPost = nullptr;
		}

		if (pPlayer->m_pInventory != nullptr) {
			delete pPlayer->m_pInventory;
			pPlayer->m_pInventory = nullptr;
		}

		// This deallocation MUST be after the delete pInventorys
		//
		for (i = 0; i < NUMBER_OF_CLUES; i++) {

			if (!pPlayer->m_aClueArray[i].bUsed) {

				if (pPlayer->m_aClueArray[i].pNote != nullptr) {
					delete pPlayer->m_aClueArray[i].pNote;
					pPlayer->m_aClueArray[i].pNote = nullptr;
				}
			}
		}
	}
}


VOID InitBFCInfo(CBfcMgr *pBfcMgr) {
	const CClueTable *pClueTable;
	CHodjPodj *pPlayer;
	CItem *pItem;
	int i, j, k;

	assert(pBfcMgr != nullptr);

	FreeBFCInfo(pBfcMgr);

	pPlayer = &pBfcMgr->m_cHodj;
	for (k = 0; k < 2; k++) {

		pPlayer->m_bComputer = FALSE;
		pPlayer->m_bMoving = TRUE;
		pPlayer->m_nTurns = 1;

		// Podj Specific stuff
		//
		if (k == 1) {
			pPlayer = &pBfcMgr->m_cPodj;
			pPlayer->m_bComputer = TRUE;
			pPlayer->m_bMoving = FALSE;
			pPlayer->m_nTurns = 0;
		}

		pPlayer->m_iSkillLevel = SKILLLEVEL_LOW;

		pPlayer->m_bHaveMishMosh = FALSE;

		pPlayer->m_pTradingPost = nullptr;
		pPlayer->m_pBlackMarket = nullptr;

		if ((pPlayer->m_pInventory = new CInventory(k == 0 ? "Hodj's Stuff" : "Podj's Stuff")) != nullptr) {

			pPlayer->m_pInventory->AddItem(k == 0 ? MG_OBJ_HODJ_NOTEBOOK : MG_OBJ_PODJ_NOTEBOOK, 1);
			pItem = pPlayer->m_pInventory->FindItem(k == 0 ? MG_OBJ_HODJ_NOTEBOOK : MG_OBJ_PODJ_NOTEBOOK);
			(*pItem).SetActionCode(ITEM_ACTION_NOTEBOOK);
			pPlayer->m_pInventory->AddItem(MG_OBJ_CROWN, 20);
		}

		pszTest = (CHAR *)&"Corruption Test";

		if ((pPlayer->m_pGenStore = new CInventory("General Store")) != nullptr) {
			for (i = MG_OBJ_BASE; i <= MG_OBJ_MAX; i++) {
				switch (i) {
				case MG_OBJ_HERRING:
				case MG_OBJ_MISH:
				case MG_OBJ_MOSH:
				case MG_OBJ_HODJ_NOTEBOOK:
				case MG_OBJ_PODJ_NOTEBOOK:
				case MG_OBJ_CROWN:
					break;
				default:
					pPlayer->m_pGenStore->AddItem(i, 1);
					break;
				}
			}

			if ((pPlayer->m_pBlackMarket = new CInventory("Black Market")) != nullptr) {
				for (i = 0; i < ITEMS_IN_BLACK_MARKET; i ++) {
					j = brand() % pPlayer->m_pGenStore->ItemCount();
					pItem = pPlayer->m_pGenStore->FetchItem(j);
					pPlayer->m_pGenStore->RemoveItem(pItem);

					pPlayer->m_pBlackMarket->AddItem(pItem);
				}
			}
		}

		pClueTable = CMgStatic::cHodjClueTable;
		if (k == 1)
			pClueTable = CMgStatic::cPodjClueTable;

		for (i = 0; i < NUMBER_OF_CLUES; i++) {

			pPlayer->m_aClueArray[i].pNote = new CNote(-1,
			    pClueTable[i].m_iCluePicCode - MG_CLUEPIC_BASE + NOTE_ICON_BASE,
			    pClueTable[i].m_iCluePicCount,
			    -1, -1);
			pPlayer->m_aClueArray[i].bUsed = FALSE;
		}

		pPlayer->m_iFurlongs = 0;
	}

	pBfcMgr->m_bInventories = TRUE;
	pBfcMgr->m_stGameStruct.lCrowns = 20;
	pBfcMgr->m_stGameStruct.lScore = 0;
	pBfcMgr->m_stGameStruct.nSkillLevel = SKILLLEVEL_MEDIUM;
	pBfcMgr->m_stGameStruct.bSoundEffectsEnabled = bSoundEffectsEnabled;
	pBfcMgr->m_stGameStruct.bMusicEnabled = bMusicEnabled;
	pBfcMgr->m_stGameStruct.bPlayingHodj = TRUE;
	pBfcMgr->m_bRestart = FALSE;

	pBfcMgr->m_bRestoredGame = TRUE;

	pBfcMgr->m_stGameStruct.bPlayingMetagame = TRUE;
	pBfcMgr->m_iGameTime = SHORT_GAME;
	pBfcMgr->m_bScrolling = bScrollingEnabled;
	pBfcMgr->m_bSlowCPU = bSlowCPU;
	pBfcMgr->m_bLowMemory = bLowMemory;
	pBfcMgr->m_bAnimations = ((bSlowCPU || bLowMemory) ? FALSE : bAnimationsEnabled);
	pBfcMgr->m_bChanged = FALSE;
	pBfcMgr->m_dwFreeSpaceMargin = dwFreeSpaceMargin;
	pBfcMgr->m_dwFreePhysicalMargin = dwFreePhysicalMargin;

	if (bHomeWriteLocked)
		pBfcMgr->m_nInstallationCode = INSTALL_NONE;
	else
		pBfcMgr->m_nInstallationCode = (bPathsDiffer ? INSTALL_BASIC : INSTALL_MINIMAL);
	Common::strcpy_s(pBfcMgr->m_chHomePath, chHomePath);
	Common::strcpy_s(pBfcMgr->m_chCDPath, chCDPath);
	Common::strcpy_s(pBfcMgr->m_chMiniPath, chMiniPath);

	// create Mish/Mosh items
	assert(pBfcMgr->m_pMishItem == nullptr);
	pBfcMgr->m_pMishItem = new CItem(MG_OBJ_MISH);

	assert(pBfcMgr->m_pMoshItem == nullptr);
	pBfcMgr->m_pMoshItem = new CItem(MG_OBJ_MOSH);
}


/*****************************************************************
 *
 * FlushInputEvents
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Remove all keyboard and mouse related events from the message
 *  so that they will not be sent to us for processing; i.e. this
 *  flushes keyboard type ahead and extra mouse clicks and movement.
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

void CHodjPodjWindow::FlushInputEvents(void) {
	MSG msg;

	while (TRUE) {                                      // find and remove all keyboard events
		if (!PeekMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			break;
	}

	while (TRUE) {                                      // find and remove all mouse events
		if (!PeekMessage(&msg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	}
}


void CHodjPodjWindow::HandleError(ERROR_CODE errCode) {
	//
	// Exit this application on fatal errors
	//
	if (errCode != ERR_NONE) {

		// Display Error Message to the user
		MessageBox(errList[errCode], "Fatal Error!", MB_OK | MB_ICONSTOP);

		// Force this application to terminate
		PostMessage(WM_CLOSE, 0, 0);
	}
}


/*****************************************************************
*
*  Restore
*
*  FUNCTIONAL DESCRIPTION:
*
*       Load Save/Restore DLL and restore an old game
*
*  IMPLICIT INPUT PARAMETERS:
*
*       [External data read]
*
*  RETURN VALUE:
*
*       ERROR_CODE = Error return code
*
****************************************************************/
BOOL CHodjPodjWindow::Restore() {
	BOOL bSuccess;
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

	// Assume Restore will work
	bSuccess = TRUE;

	// Validate implicit input
	assert(gpszSaveDLL != nullptr);
	assert(pGamePalette != nullptr);

	PositionAtHomePath();

	bSuccess = Saves::RestoreGame((CWnd *)this, pGamePalette, &errCode);

	lpMetaGame->m_bScrolling = bScrollingEnabled;
	lpMetaGame->m_bSlowCPU = bSlowCPU;
	lpMetaGame->m_bLowMemory = bLowMemory;
	lpMetaGame->m_bAnimations = ((bSlowCPU || bLowMemory) ? FALSE : bAnimationsEnabled);
	lpMetaGame->m_dwFreeSpaceMargin = dwFreeSpaceMargin;
	lpMetaGame->m_dwFreePhysicalMargin = dwFreePhysicalMargin;

	if (bHomeWriteLocked)
		lpMetaGame->m_nInstallationCode = INSTALL_NONE;
	else
		lpMetaGame->m_nInstallationCode = (bPathsDiffer ? INSTALL_BASIC : INSTALL_MINIMAL);

	// maintain the correct home path info
	Common::strcpy_s(lpMetaGame->m_chHomePath, chHomePath);
	Common::strcpy_s(lpMetaGame->m_chCDPath, chCDPath);
	Common::strcpy_s(lpMetaGame->m_chMiniPath, chMiniPath);

	if (errCode != ERR_NONE) {
		HandleError(errCode);
		bSuccess = FALSE;
	}

	return (bSuccess);
}


VOID CHodjPodjWindow::ShowCredits(VOID) {
	MSG msg;
	LONG lTimeElapsed, lStart;
	ULONG lGoal, lWait;
	CDC *pDC;
	INT i;

	m_bInCredits = TRUE;

	// Start theme music
	StartBackgroundMidi();

	// paint black
	//
	BlackScreen();

	m_nFlags = 0;

	if ((pDC = GetDC()) != nullptr) {

		// load first bitmap
		m_pCurrentBmp = nullptr;
		if (FileExists(stCredits[0].m_pszCelFile)) {
			m_pCurrentBmp = FetchBitmap(pDC, nullptr, stCredits[0].m_pszCelFile);
		}

		// for each credit screen
		//
		for (i = 0; i < MAX_CREDITS; i++) {

			// Paint bitmap onto sreen
			//
			if (m_pCurrentBmp != nullptr) {
				PaintBitmap(pDC, nullptr, m_pCurrentBmp, 0, 0);
				delete m_pCurrentBmp;
				m_pCurrentBmp = nullptr;
			}

			// get current time
			lStart = GetTickCount();

			// pre-load next bitmap
			//
			if (i < (MAX_CREDITS - 1)) {
				if (FileExists(stCredits[i + 1].m_pszCelFile)) {
					m_pCurrentBmp = FetchBitmap(pDC, nullptr, stCredits[i + 1].m_pszCelFile);
				} else {
					continue;
				}
			}

			// how long did it take to load that bitmap
			lTimeElapsed = GetTickCount() - lStart;

			//
			// pause for specified ammount of time less ammount used to pre-load
			// next bitmap
			//
			lWait = (ULONG)max((LONG)stCredits[i].m_nDuration - lTimeElapsed, 0L);

			lGoal = lWait + GetTickCount();
			while (lGoal > GetTickCount()) {

				// give windows some time because we need to be able to get
				// mouse clicks and keyboard hits to stop this credits screen
				//
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				if (m_nFlags)
					break;
			}
			if (m_nFlags & CR_ESCAPE)
				break;

			m_nFlags = 0;
		}
		if (m_pCurrentBmp != nullptr) {
			delete m_pCurrentBmp;
			m_pCurrentBmp = nullptr;
		}
		ReleaseDC(pDC);
	}

	// paint black
	BlackScreen();

	// stop theme music
	StopBackgroundMidi();

	m_bInCredits = FALSE;
}

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
