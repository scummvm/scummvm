/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * hodjpodj.cpp
 *
 * HISTORY
 *
 *  1.0 5/2/94 GTB      
 *
 * MODULE DESCRIPTION:
 *
 *
 * LOCALS:
 *
 *  SplashScreen            paint the background artwork
 *
 * GLOBALS:
 *
 *  FlushInputEvents        flush all unprocessed mouse/keyboard events
 *  ReleaseResources        release all application specific resources
 *
 * RELEVANT DOCUMENTATION:
 *
 *      n/a
 *
 * FILES USED:
 *
 *
 ****************************************************************/

#include "stdafx.h"

#include <io.h>
#include <direct.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdlib.h>


#include "bagel/boflib/misc.h"

#include "invent.h"
#include "item.h"

#include "hodjpodj.h"  
#include "mgstat.h"  
#include "bfc.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "dialogs.h"
#include "misc.h"
#include "movytmpl.h"
#include "restgame.h"
#include "sound.h"
#include "..\grandtr\gtstruct.h"
#include "c1btndlg.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"

#include "copyrite.h"                       // mandatory internal copyright notice

#define SAVEDLL         0


#define CONTROL_PHYSICAL_MEMORY		TRUE


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

typedef HWND (FAR PASCAL *FPDLLFUNCT) ( HWND, LPGAMESTRUCT);
typedef HWND (FAR PASCAL *FPZOOMFUNCT) ( HWND, BOOL );
typedef HWND (FAR PASCAL *FPGTFUNCT) ( HWND, LPGRANDTRSTRUCT );
typedef HWND (FAR PASCAL *FPMETAFUNCT) ( HWND, CBfcMgr *, BOOL );
typedef DWORD (FAR PASCAL * FPGETFREEMEMINFO) (void);

static FPGETFREEMEMINFO lpfnGetFreeMemInfo;

HINSTANCE   hExeInst = NULL;
HINSTANCE   hDllInst = NULL;
HINSTANCE   hMetaInst = NULL;

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

static  CPalette    *pGamePalette = NULL;
static  RECT        MainRect;                           // screen area spanned by the game window

int                                     nInstallCode;

BOOL                    bHomeWriteLocked = FALSE;
BOOL                    bPathsDiffer = FALSE;
static  BOOL        bHaveCDROM = FALSE;
static  char        chProfilePath[PATHSPECSIZE];
char                    chHomePath[PATHSPECSIZE];
char                    chMiniPath[PATHSPECSIZE];
char                    chCDPath[PATHSPECSIZE];
static  int         nHomeDrive = -1;
static  int         nMiniDrive = -1;
static  int         nCDDrive = -1;

static  HWND        hwndGame = NULL;
static  BOOL        bActiveWindow = FALSE;

static  BOOL        bReturnToZoom = FALSE;
static  BOOL        bReturnToMeta = FALSE;
static  BOOL        bReturnToGrandTour = FALSE;

CBfcMgr             *lpMetaGame = NULL;
CHAR                *pszTest = "Corruption Test";

LPGAMESTRUCT        lpGameStruct = NULL;
LPGRANDTRSTRUCT     lpGrandTour = NULL;

CMovieWindow    *pMovie = NULL;    

int     nChallengePhase = 0;
int     nChallengeGame = 0;
long    lChallengeScore = 0;

CSound  *pBackgroundMidi = NULL;

#define BELLCURVE   100
UBYTE     anBellCurve[BELLCURVE] =
{ 
 1,
 2, 2,
 3, 3, 3,
 4, 4, 4, 4,
 5, 5, 5, 5, 5,
 6, 6, 6, 6, 6, 6,
 7, 7, 7, 7, 7, 7, 7,
 8, 8, 8, 8, 8, 8, 8, 8,
 9, 9, 9, 9, 9, 9, 9, 9, 9,
10,10,10,10,10,10,10,10,10, 10,
11,11,11,11,11,11,11,11,11,
12,12,12,12,12,12,12,12,
13,13,13,13,13,13,13,
14,14,14,14,14,14,
15,15,15,15,15,
16,16,16,16,
17,17,17,
18,18,
19
};


#define MAX_CREDITS  12

#define CR_NEXT   0x0001
#define CR_ESCAPE 0x0002

struct CREDITS {
    CHAR *m_pszCelFile;
    UINT  m_nDuration;
};

STATIC CREDITS stCredits[MAX_CREDITS] = {
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
void LoadFloatLib(void);

/////////////////////////////////////////////////////////////////////////////

// theApp:
// Just creating this application object runs the whole application.
//
CTheApp NEAR theApp;

/////////////////////////////////////////////////////////////////////////////

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
CHodjPodjWindow::CHodjPodjWindow()
{
    BOOL    bSuccess;
    CDC     *pDC = NULL;                        // device context for the screen
    CString WndClass;
    CDibDoc *pDibDoc = NULL;            // pointer to the background art DIB
    CSize   mySize;
    BOOL    bTestDibDoc;
    CRect   rQuitRect;
    CBitmap *pBitmap;

    // hack to load the WIN87EM DLL
    LoadFloatLib();

    // Inits
    m_pCurrentBmp = NULL;
    m_nFlags = 0;
    m_bInCredits = FALSE;

    WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC, NULL, NULL, NULL);

    //srand( (unsigned)time( NULL ));

#ifdef _DEBUG
    MainRect.left = 0;
    MainRect.top = 0;
    MainRect.right = GAME_WIDTH;    // determine where to place the game window
    MainRect.bottom = GAME_HEIGHT;   // ... so it is centered on the screen
#else
    pDC = GetDC();                                  // get a device context for our window

    MainRect.left = 0;
    MainRect.top = 0;
    MainRect.right = pDC->GetDeviceCaps(HORZRES);    // determine where to place the game window
    MainRect.bottom = pDC->GetDeviceCaps(VERTRES);   // ... so it is centered on the screen
    ReleaseDC( pDC );
#endif

    bTestDibDoc = Create( WndClass, "Boffo Games - Hodj 'n' Podj", WS_POPUP, MainRect, NULL, NULL );
    ASSERT( bTestDibDoc );

    BeginWaitCursor();
    ShowWindow(SW_SHOWNORMAL);
    BlackScreen();
    EndWaitCursor();

    pDC = GetDC();                                  // get a device context for our window

    bSuccess = CheckConfig(pDC);                    // verify we are runnable
    if (!bSuccess) {                                // ... and abort if not
        ReleaseDC(pDC);
        PostMessage( WM_CLOSE);
        return;
    }

    pBitmap = FetchBitmap(pDC,&pGamePalette,MAINSCROLL);
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

    lpMetaGame = new CBfcMgr;                       // only allocate this once

    // if user passes the /N command line switch,
    //
    //
    if (StriStr(theApp.m_lpCmdLine, "-n") || StriStr(theApp.m_lpCmdLine, "/n")) {

        // ...then do not play the into movie
        PostMessage( WM_COMMAND, IDC_MAINDLG );
    } else {

        PostMessage( WM_COMMAND, IDC_PLAY_LOGO_MOVIE );
    }
}


void CHodjPodjWindow::GetProfileSettings(void)
{
    CWinApp *pMyApp;

    pMyApp = AfxGetApp();
    nInstallCode = pMyApp->GetProfileInt("Meta","InstallCode",INSTALL_NONE);
    bAnimationsEnabled = pMyApp->GetProfileInt("Meta","Animations",TRUE);
    bScrollingEnabled = pMyApp->GetProfileInt("Meta","MapScrolling",FALSE);
    bSoundEffectsEnabled = pMyApp->GetProfileInt("Meta","SoundEffects",TRUE);
    bMusicEnabled = pMyApp->GetProfileInt("Meta","Music",TRUE);
    nMidiVolume = pMyApp->GetProfileInt("Meta","MidiVolume",(VOLUME_INDEX_MAX * 3) >> 2);
    nWaveVolume = pMyApp->GetProfileInt("Meta","WaveVolume",(VOLUME_INDEX_MAX * 3) >> 2);

    bSlowCPU = (GetWinFlags() & WF_CPU386) ? TRUE : FALSE;
    
    CSound::SetVolume(nMidiVolume,nWaveVolume);
}


void CHodjPodjWindow::SaveProfileSettings(void)
{
    CWinApp *pMyApp;

    pMyApp = AfxGetApp();
    pMyApp->WriteProfileInt("Meta","Animations",bAnimationsEnabled);
    pMyApp->WriteProfileInt("Meta","MapScrolling",bScrollingEnabled);
//      pMyApp->WriteProfileInt("Meta","SoundEffects",bSoundEffectsEnabled);
//      pMyApp->WriteProfileInt("Meta","Music",bMusicEnabled);
}


void LoadFloatLib(void)
{
    double fHack = 3.1415;

    fHack = fHack / 1.4;
}


BOOL CHodjPodjWindow::CheckConfig(CDC *pDC)
{
int     nDevCaps;
BOOL    bSuccess = TRUE;

    bLowMemory = CheckLowMemory();

    (void) SystemParametersInfo(SPI_GETSCREENSAVEACTIVE,0,&bScreenSaver,0);
    (void) SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,FALSE,NULL,0);

    nDevCaps = (*pDC).GetDeviceCaps(BITSPIXEL);
    if (nDevCaps < 8) {
    ::MessageBox(NULL,"Please set your display to 256 colors\nbefore playing this game.","Display Configuration",MB_ICONEXCLAMATION);
    bSuccess = FALSE;
    }
    else {
        nDevCaps = (*pDC).GetDeviceCaps(RASTERCAPS);
        if (!(nDevCaps & RC_PALETTE))
        ::MessageBox(NULL,"It is recommended that your display\nbe set to 256 colors before playing.","Display Configuration",MB_ICONINFORMATION);
#ifdef _DEBUG
        if (!(nDevCaps & RC_BITBLT))
        ::MessageBox(NULL,"BitBLTs not Supported","Display Configuration",MB_ICONEXCLAMATION);
        if (!(nDevCaps & RC_BITMAP64))
        ::MessageBox(NULL,"64K+ bitmaps not Supported","Display Configuration",MB_ICONEXCLAMATION);
        if (!(nDevCaps & RC_DI_BITMAP))
        ::MessageBox(NULL,"SetDIBits not Supported","Display Configuration",MB_ICONEXCLAMATION);
        if (!(nDevCaps & RC_DIBTODEV))
        ::MessageBox(NULL,"SetDIBitsToDevice not Supported","DDisplay Configuration",MB_ICONEXCLAMATION);
        if (!(nDevCaps & RC_PALETTE))
        ::MessageBox(NULL,"Palettes not Supported","Display Configuration",MB_ICONEXCLAMATION);
        if (!(nDevCaps & RC_STRETCHBLT))
        ::MessageBox(NULL,"StretchBlts not Supported","Display Configuration",MB_ICONEXCLAMATION);
#endif
    }

    return(bSuccess);
}


BOOL CHodjPodjWindow::CheckLowMemory(void)
{
BOOL                    bMemoryProblem;
DWORD                   dwInfo;
WORD                    wFreePages;
long                    lFreeBytes;
DWORD                   dwFreeSpace;

    bMemoryProblem = FALSE;

    dwFreeSpace = ::GetFreeSpace(0);
    
    dwFreeSpaceMargin = dwFreeSpace >> 2;
    if (dwFreeSpaceMargin < 850000L)
        dwFreeSpaceMargin = 850000L;
    
    dwFreePhysicalMargin = dwFreeSpaceMargin;

    if (dwFreeSpace < 3145728L)
        bMemoryProblem = TRUE;
#if CONTROL_PHYSICAL_MEMORY    
    else {
        lpfnGetFreeMemInfo = (FPGETFREEMEMINFO)::GetProcAddress(GetModuleHandle("KERNEL"), "GETFREEMEMINFO");
        if (lpfnGetFreeMemInfo != NULL) {
            dwInfo = lpfnGetFreeMemInfo();
            if (dwInfo != -1L) {
                wFreePages = HIWORD(dwInfo);
                lFreeBytes = (long) wFreePages * 4096L;
                if (lFreeBytes < 1400000L) {
                    bMemoryProblem = TRUE;
                    dwFreePhysicalMargin = 200000L;
                }
                else
                    dwFreePhysicalMargin = lFreeBytes / 10;
            } 
        }
    }
#endif
    
    return(bMemoryProblem); 
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

void CHodjPodjWindow::OnPaint()
{
    CDC *pDC;

    if ( hDllInst > HINSTANCE_ERROR ) {
        PAINTSTRUCT lpPaint;

        Invalidate(FALSE);
        BeginPaint(&lpPaint);
        EndPaint(&lpPaint);

    } else {

        if (m_pCurrentBmp != NULL) {
            if ((pDC = GetDC()) != NULL) {
                PaintBitmap(pDC, NULL, m_pCurrentBmp, 0, 0);
                ReleaseDC(pDC);
            }

        } else {
            CPaintDC dc(this);
            CBrush  Brush( RGB( 0, 0, 0 ));

            dc.FillRect( &MainRect, &Brush);
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

BOOL CHodjPodjWindow::SetupNewMeta()
{                                   
    CMetaSetupDlg   cMetaSetupDlg( (CWnd *)this, pGamePalette );
    int             nMetaSetupReturn = 0;
    CItem           *pItem = NULL;

    InitBFCInfo(lpMetaGame);

    cMetaSetupDlg.SetInitialOptions( lpMetaGame );
    nMetaSetupReturn = cMetaSetupDlg.DoModal();

    if ( nMetaSetupReturn == 0 )
    return(FALSE);
    else
    return(TRUE);
}

void CHodjPodjWindow::PlayMovie(const int nMovieId, const char *pszMovie, BOOL bScroll)
{
    POINT   ptMovie;
    
    BlackScreen();

    if ( pMovie != NULL ) {
        delete pMovie;
        pMovie = NULL;
    }

    pMovie = new CMovieWindow();
    pMovie->nMovieId = nMovieId;
    
    ptMovie.x = ( MainRect.right / 2 ) - ( MOVIE_WIDTH / 2 );
    ptMovie.y = ( MainRect.bottom / 2 ) - ( MOVIE_HEIGHT / 2 );
    
    PositionAtCDPath();
        
    if (pMovie->BlowWindow((CWnd *)this, bScroll, pszMovie, ptMovie.x, ptMovie.y) == TRUE) {
        if (nMovieId != MOVIE_ID_TITLE)
            ::ShowCursor(FALSE);
    } else {
        if ( pMovie !=NULL ) {
            delete pMovie;
            pMovie = NULL;
        }

        ::ShowCursor(TRUE);

        PositionAtHomePath();
        PostMessage( WM_COMMAND, IDC_MAINDLG );
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
BOOL CHodjPodjWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // this must be before the CMainGameDlg constructor
    PositionAtHomePath();

    BOOL            bSuccess;
    CMainGameDlg    cMainDlg( (CWnd *)this, pGamePalette );
    int             nMainDlgReturn = 0;
    CDC            *pDC = NULL;
    CWnd           *pWnd = NULL;
    int                         nMovieId;

    switch ( wParam ) {

    case MOVIE_OVER:
        PositionAtHomePath();

        if ( pMovie == NULL )
        nMovieId = MOVIE_ID_NONE;
        else {
        nMovieId = pMovie->nMovieId;
        delete pMovie;
        pMovie = NULL;
        }

        pWnd = (CWnd *)lParam;
        if ( pWnd != NULL ) {
        (*pWnd).SendMessage(WM_CLOSE,0,0L);
        delete pWnd;
        }

        BlackScreen();
        
        if (nMovieId == MOVIE_ID_LOGO) {
            ::ShowCursor(TRUE);
            StartBackgroundMidi();
            ::ShowCursor(FALSE);
            PostMessage( WM_COMMAND, IDC_PLAY_TITLE_MOVIE );

        } else if (nMovieId == MOVIE_ID_ENDING) {
            ::ShowCursor(TRUE);
            ShowCredits();
            BlackScreen();
            PostMessage( WM_COMMAND, IDC_MAINDLG );

        } else {
            ::ShowCursor(TRUE);
            PostMessage( WM_COMMAND, IDC_MAINDLG );
        }
        break;

    case IDC_MAINDLG:
        StartBackgroundMidi();
        PositionAtHomePath();
        nMainDlgReturn = cMainDlg.DoModal();
    
        switch ( nMainDlgReturn ) {

        case IDC_PLAY_META:
            if ( SetupNewMeta() )
            PostMessage( WM_COMMAND, IDC_META );
            else
            PostMessage( WM_COMMAND, IDC_MAINDLG );
            
            return(TRUE);

        case IDC_PLAY_MINI:
            PostMessage( WM_COMMAND, IDC_ZOOM );
            return(TRUE);

        case IDC_RESTORE_GAME:
            StopBackgroundMidi();

            if (Restore())
            PostMessage(WM_COMMAND, IDC_META);
            else
            PostMessage(WM_COMMAND, IDC_MAINDLG);

            return(TRUE);

        case IDC_GRAND_TOUR:
            PostMessage( WM_COMMAND, IDC_GRANDTOUR );
            break;

        case IDC_RESTART_MOVIE:
            StopBackgroundMidi();
            PlayMovie(MOVIE_ID_INTRO, STARTUP_MOVIE, TRUE);
            break;

        case IDC_QUIT_GAME:
            StopBackgroundMidi();
            PostMessage( WM_CLOSE );
            return(TRUE);

        default:
            PostMessage( WM_COMMAND, IDC_MAINDLG );
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
            PostMessage( WM_COMMAND, IDC_MAINDLG );
        break;

    case IDC_GRANDTOUR:
        BlackScreen();
        
        if ( lpGrandTour != NULL ) { 
            if ( lpGameStruct != NULL )
                lpGameStruct = NULL;
            delete lpGrandTour;
            lpGrandTour = NULL;
        }
        bSuccess = LoadGrandTourDLL();
        if (!bSuccess)
            PostMessage( WM_COMMAND, IDC_MAINDLG );
        break;

    case IDC_META:
        StopBackgroundMidi();
        BlackScreen();

        bSuccess = LoadMetaDLL();
        if (!bSuccess)
            PostMessage( WM_COMMAND, IDC_MAINDLG );
        break;
    }   

    if ( pDC != NULL )
        ReleaseDC(pDC);

    return(TRUE);
}


void CHodjPodjWindow::OnMouseMove(UINT nFlags, CPoint point)
{
HCURSOR hNewCursor = NULL;
CWinApp *pMyApp;

    if (!bActiveWindow)
        return;
    
    pMyApp = AfxGetApp();

    hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);

    if (hNewCursor != NULL);
        MFC::SetCursor(hNewCursor);

    CWnd::OnMouseMove(nFlags, point);
}


void CHodjPodjWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_bInCredits) {
        m_nFlags = CR_NEXT;
    } else {
        CFrameWnd::OnLButtonDown(nFlags, point);
    }
}

void CHodjPodjWindow::OnRButtonDown(UINT nFlags, CPoint point)
{
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

BOOL CHodjPodjWindow::OnEraseBkgnd( CDC * ) {
    return TRUE;
}


void CHodjPodjWindow::BlackScreen(void)
{
CDC         *pDC;
CBrush  	Brush( RGB( 0, 0, 0 ));
CPalette	*pPalOld = NULL;

    ValidateRect(NULL);
    pDC = GetDC();

    pDC->FillRect( &MainRect, &Brush);

	if (pGamePalette != NULL) {									// map in color palette to be used
		pPalOld = (*pDC).SelectPalette(pGamePalette,FALSE);
		(void) (*pDC).RealizePalette();
	}

    pDC->FillRect( &MainRect, &Brush);

	if (pPalOld != NULL)									// relinquish the resources we built
		(void) (*pDC).SelectPalette(pPalOld,FALSE);

    ReleaseDC(pDC);
}


void CHodjPodjWindow::OnActivate( UINT nState, CWnd *, BOOL)
{
    BOOL    bUpdateNeeded;

    switch (nState) {

    case WA_INACTIVE:
        bActiveWindow = FALSE;
        break;

    case WA_ACTIVE:
    case WA_CLICKACTIVE:

        if (( pMovie != NULL ) && ( pMovie->pDum->m_hWnd != NULL )) {
            ::SetFocus( pMovie->pDum->m_hWnd );
            break;
        }
        else {
            if (( pMovie != NULL ) && ( pMovie->hWndDum != NULL )) {
                ::SetFocus( pMovie->hWndDum );
                break;
            }
        }

        if (hwndGame != NULL)
            ::SetActiveWindow( hwndGame );
        else {    
            bActiveWindow = TRUE;
            bUpdateNeeded = GetUpdateRect(NULL,FALSE);
            if (bUpdateNeeded)
                InvalidateRect(NULL,FALSE);
        }
        break;
    }
}


void CHodjPodjWindow::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if (nChar == VK_ESCAPE) {
        m_nFlags = CR_ESCAPE;
    } else {
        m_nFlags = CR_NEXT;
    }
    CWnd::OnKeyDown( nChar, nRepCnt, nFlags);
}


long CHodjPodjWindow::OnMCINotify( WPARAM wParam, LPARAM lParam)
{
    (void) CSound::OnMCIStopped(wParam,lParam);
    return(0L);  
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

void    CHodjPodjWindow::LoadNewDLL( LPARAM lParam )
{
BOOL    bSuccess = FALSE;   
int     nWhichDLL;
CDC     *pDC = NULL;
BOOL    bLoadedDLL;
CWinApp *pMyApp;

    pMyApp = AfxGetApp();

    nWhichDLL = (int)lParam;

    BlackScreen();

    FreeCurrentDLL();

    nWhichDLL-= MG_GAME_BASE;

    if (( nWhichDLL > MG_GAME_COUNT ) || (nWhichDLL < 0 )) {

        if ( bReturnToZoom ) {
            StartBackgroundMidi();
            bSuccess = LoadZoomDLL();
        }

        if ( bReturnToMeta ) {
            StopBackgroundMidi();
            lpMetaGame->m_bRestart = TRUE;
            bSuccess = LoadMetaDLL();
        }

        if ( bReturnToGrandTour ) {
            StartBackgroundMidi();
            bSuccess = LoadGrandTourDLL();
        }
    
        return;
    }

    bLoadedDLL = FALSE;
    
    if ( lParam == MG_GAME_CHALLENGE ) {
        StartBackgroundMidi();
        bSuccess = LoadZoomDLL();
        bReturnToZoom = FALSE;
        if ( bSuccess == FALSE ) {
            lpMetaGame->m_bRestart = TRUE;
            bSuccess = LoadMetaDLL();
        } else
            StartBackgroundMidi();
        return;
    }
    
    StopBackgroundMidi();

    if (( CMgStatic::cGameTable[ nWhichDLL ].m_lpszDllName !=NULL ) &&
    ( CMgStatic::cGameTable[ nWhichDLL ].m_lpszApiName !=NULL )) {
    
    FPDLLFUNCT  lpfnGame;

    strcpy(chMiniPath,CMgStatic::cGameTable[ nWhichDLL ].m_lpszDllPath);
            
    if ( bReturnToZoom ) {

        if ( lpGameStruct != NULL ) {
            delete lpGameStruct;
            lpGameStruct = NULL;
        }
        lpGameStruct = new GAMESTRUCT;
        lpGameStruct->lCrowns = 1000;
        lpGameStruct->lScore = 0;
        lpGameStruct->nSkillLevel = SKILLLEVEL_MEDIUM;
        bSoundEffectsEnabled = pMyApp->GetProfileInt("Meta","SoundEffects",TRUE);
        bMusicEnabled = pMyApp->GetProfileInt("Meta","Music",TRUE);
        lpGameStruct->bSoundEffectsEnabled = bSoundEffectsEnabled;
        lpGameStruct->bMusicEnabled = bMusicEnabled;
        lpGameStruct->bPlayingMetagame = FALSE;
        lpGameStruct->bPlayingHodj = TRUE;
    }

    if ((CMgStatic::cGameTable[ nWhichDLL ].m_bLocalDLL && PositionAtHomePath()) ||
        (PositionAtMiniPath(nWhichDLL) && FileExists(CMgStatic::cGameTable[ nWhichDLL ].m_lpszDllName))) {
        hDllInst = ::LoadLibrary( CMgStatic::cGameTable[ nWhichDLL ].m_lpszDllName );
    
        if ( hDllInst > HINSTANCE_ERROR ) {
        lpfnGame = (FPDLLFUNCT)::GetProcAddress( hDllInst, CMgStatic::cGameTable[ nWhichDLL ].m_lpszApiName );
        if ((lpfnGame != NULL) && PositionAtMiniPath(nWhichDLL)) {
            if ( bReturnToZoom ) {
                hwndGame = lpfnGame( m_hWnd, lpGameStruct );
            }
            else {
                if ( bReturnToGrandTour ) {
                    hwndGame = lpfnGame( m_hWnd, &lpGrandTour->stMiniGame );
                }
                else {
                    hwndGame = lpfnGame( m_hWnd, &lpMetaGame->m_stGameStruct );
                }
            }
            bLoadedDLL = TRUE;
        }
        }
    }

    if ( bLoadedDLL == FALSE) {
        CString     cTmp;
        
        (void) PositionAtHomePath();

        if (!bHaveCDROM) {
            MessageBox( "Unable to find the Hodj 'n' Podj CDROM!!!");
            MessageBox( "Please place the game disc in your CDROM drive.");
            (void) GetCDPath();
        }
        else {
            cTmp = "Unable to find or load the ";
            cTmp += CMgStatic::cGameTable[ nWhichDLL ].m_lpszGameName;
            cTmp += " game!!!";
            MessageBox( cTmp, "Game Launch Error" );
            MessageBox( "Verify proper game installation and try again.");
        }
        bLoadedDLL = FALSE;
        
        if ( bReturnToZoom )
        bSuccess = LoadZoomDLL();
    
        if ( bReturnToMeta ) {
        lpMetaGame->m_bRestart = TRUE;
        bSuccess = LoadMetaDLL();
        }       
    }
    }   
}


void CHodjPodjWindow::FreeCurrentDLL(void)
{
    if (strcmp(pszTest, "Corruption Test")) {
    MessageBox("Data Segment is Corrupt.  Go get Brian!", MB_OK);
    }

    if ( hDllInst > HINSTANCE_ERROR ) {
    if (hDllInst != hMetaInst)
        FreeLibrary(hDllInst);
    hDllInst = NULL;
    hwndGame = m_hWnd;
    }
}
 

BOOL CHodjPodjWindow::LoadMetaDLL(void)
{
FPMETAFUNCT lpfnMeta;

    FreeCurrentDLL();

    if (((nInstallCode <= INSTALL_MINIMAL) && PositionAtCDPath()) || PositionAtHomePath()) {
    if (bMetaLoaded)
        hDllInst = hMetaInst;
    else
    if ( FileExists( "hnpmeta.dll" ))
        hDllInst = ::LoadLibrary( "hnpmeta.dll" );
    else
        goto punt;

    if ( hDllInst > HINSTANCE_ERROR ) {
        lpfnMeta = (FPMETAFUNCT)::GetProcAddress( hDllInst, "RunMeta");
        if (lpfnMeta !=NULL) {
        hwndGame = NULL;
#if RETAIN_META_DLL
        hwndGame = lpfnMeta( m_hWnd, lpMetaGame, bMetaLoaded );
        hMetaInst = hDllInst;
        bMetaLoaded = TRUE;
#else
        hwndGame = lpfnMeta( m_hWnd, lpMetaGame, FALSE );
#endif
        bReturnToMeta = TRUE;
        return(TRUE);
        }
    }
    }

punt:

    (void) PositionAtHomePath();

    bReturnToMeta = FALSE;
    if (!bHaveCDROM) {
    MessageBox( "Unable to find the Hodj 'n' Podj CDROM!!!");
    MessageBox( "Please place the game disc in your CDROM drive.");
    (void) GetCDPath();
    }
    else {
    MessageBox( "Unable to find or load the Hodj 'n' Podj game!!!");
    MessageBox( "Verify proper game installation and try again.");
    }

    return(FALSE);
}
 

BOOL CHodjPodjWindow::LoadZoomDLL(void)
{
FPZOOMFUNCT lpfnZoom;

    FreeCurrentDLL();
    
    if ((((nInstallCode <= INSTALL_MINIMAL) && PositionAtCDPath()) || PositionAtHomePath()) &&
    FileExists( "hnpzm.dll" )) {

    hDllInst = ::LoadLibrary( "hnpzm.dll" );
    if ( hDllInst > HINSTANCE_ERROR ) {
        lpfnZoom = (FPZOOMFUNCT)::GetProcAddress( hDllInst, "RunZoomMap");
        if (lpfnZoom !=NULL) {
        hwndGame = NULL;
        
        if ( nChallengePhase == 0 )
            hwndGame = lpfnZoom( m_hWnd, TRUE );
        else
            hwndGame = lpfnZoom( m_hWnd, FALSE );
            
        bReturnToZoom = TRUE;
        return(TRUE);
        }
    }
    }

    bReturnToZoom = FALSE;
    if ((nInstallCode <= INSTALL_MINIMAL) && !bHaveCDROM) {
    MessageBox( "Unable to find the Hodj 'n' Podj CDROM!!!");
    MessageBox( "Please place the game disc in your CDROM drive.");
    (void) GetCDPath();
    }
    else {
    MessageBox( "Unable to load the game selection map!!!" );
    MessageBox( "Verify proper game installation and try again.");
    }

    return(FALSE);
}


BOOL CHodjPodjWindow::LoadGrandTourDLL(void)
{
FPGTFUNCT lpfnGT;

    FreeCurrentDLL();
    
    if ( lpGrandTour == NULL ) {
    int i;

    lpGrandTour = new GRANDTRSTRUCT;
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
    for ( i = 0; i < 18; i++ ) {
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

    if ((((nInstallCode <= INSTALL_MINIMAL) && PositionAtCDPath()) || PositionAtHomePath()) &&
    FileExists( "hnpgt.dll" )) {

    hDllInst = ::LoadLibrary( "hnpgt.dll" );
    if ( hDllInst > HINSTANCE_ERROR ) {
        lpfnGT = (FPGTFUNCT)::GetProcAddress( hDllInst, "RunGrandTour");
        if ( lpfnGT !=NULL) {
        hwndGame = NULL;
        hwndGame = lpfnGT( m_hWnd, lpGrandTour );
        bReturnToGrandTour = TRUE;
        return(TRUE);
        }
    }
    }

    bReturnToGrandTour = FALSE;
    if ((nInstallCode <= INSTALL_MINIMAL) && !bHaveCDROM) {
    MessageBox( "Unable to find the Hodj 'n' Podj CDROM!!!");
    MessageBox( "Please place the game disc in your CDROM drive.");
    (void) GetCDPath();
    }
    else {
    MessageBox( "Unable to load the Grand Tour!!!" );
    MessageBox( "Verify proper game installation and try again.");
    }

    return(FALSE);
}


void CHodjPodjWindow::UpdateDLLRouting(void)
{
int i;

    PositionAtHomePath();
    
    for (i = 0; i < MG_GAME_COUNT; i++) {
    if ((CMgStatic::cGameTable[ i ].m_lpszDllName != NULL) && 
        FileExists( CMgStatic::cGameTable[ i ].m_lpszDllName))
        CMgStatic::cGameTable[ i ].m_bLocalDLL = TRUE;
    else    
        CMgStatic::cGameTable[ i ].m_bLocalDLL = FALSE;
    }
}


LPARAM CHodjPodjWindow::UpdateChallengePhase(LPARAM lParam)
{
    LPARAM  nGameID = 0;

    nGameID = lParam;
    
    BlackScreen();

    switch ( nChallengePhase ) {
    case 1:   // goto the zoom map and determine the game
        if ( lpMetaGame->m_stGameStruct.bPlayingHodj ) {
        C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", "Podj chooses", "the game." );
                cMsgBox.DoModal();
        }
        else {
        C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", "Hodj chooses", "the game." );
                cMsgBox.DoModal();
        }
        
        nChallengePhase++;
        
        if (( lpMetaGame->m_stGameStruct.bPlayingHodj ) && ( lpMetaGame->m_cPodj.m_bComputer)){
        int nGameIndex;
        do {
            nGameIndex = brand() % ( MG_GAME_COUNT - 1 );
        } while ( CMgStatic::cGameTable[nGameIndex].m_lpszDllName == NULL );
        nGameID = (LPARAM)(nGameIndex + MG_GAME_BASE);
            {
            C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Podj chooses :", " ", CMgStatic::cGameTable[nGameIndex].m_lpszGameName );
                    cMsgBox.DoModal();
        }
        goto PHASE2;
        }
        break;

    case 2:   // send the challenger off to play the mini game and store the Game Code
PHASE2:
    // set up mini-game structure 
        lpMetaGame->m_stGameStruct.lCrowns = 50;
        if ( lpMetaGame->m_stGameStruct.bPlayingHodj ) {
        C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", " ", "Hodj goes first." );
                cMsgBox.DoModal();
            }
        else {
        C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", " ", "Podj goes first." );
                cMsgBox.DoModal();
        }
                
        nChallengeGame  =  (int)nGameID; 
        nChallengePhase++;

        if (( lpMetaGame->m_stGameStruct.bPlayingHodj == FALSE ) && ( lpMetaGame->m_cPodj.m_bComputer)){
        SetComputerScore();
        goto PHASE3;
        }
        break;

    case 3:   // check to see the challenger's score > 0, if so send the challengee, if not return to the Meta
PHASE3:
        if (( lChallengeScore = DetermineChallengeScore() ) > 0 ) {
        if ( lpMetaGame->m_stGameStruct.bPlayingHodj ) {
            C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", " ", "Podj's turn." );
                    cMsgBox.DoModal();
        }
        else {
            C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", " ", "Hodj's turn." );
                    cMsgBox.DoModal();
        }

        if ( lpMetaGame->m_stGameStruct.bPlayingHodj ) {
            lpMetaGame->m_stGameStruct.bPlayingHodj = FALSE;
            lpMetaGame->m_stGameStruct.nSkillLevel = lpMetaGame->m_cPodj.m_iSkillLevel;
            lpMetaGame->m_stGameStruct.lCrowns = 50;
        }
        else {
            lpMetaGame->m_stGameStruct.bPlayingHodj = TRUE;
            lpMetaGame->m_stGameStruct.nSkillLevel = lpMetaGame->m_cHodj.m_iSkillLevel;
            lpMetaGame->m_stGameStruct.lCrowns = 50;
        }
        lpMetaGame->m_stGameStruct.lScore = 0;
                
        nGameID  =  nChallengeGame;
        }
        else {
        lChallengeScore = 0l;
        if ( lpMetaGame->m_stGameStruct.bPlayingHodj ) {
            C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Hodj Challenges", "Podj keeps", "Mish and Mosh." );
                    cMsgBox.DoModal();
        }
        else {
            C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Podj Challenges", "Hodj keeps", "Mish and Mosh." );
                    cMsgBox.DoModal();
        }
                
        lpMetaGame->m_cHodj.m_iFurlongs = 0;
        lpMetaGame->m_cPodj.m_iFurlongs = 0;
        nGameID = (long)lpMetaGame; 
        nChallengePhase = 10;
        }
        nChallengePhase++;
        
        if (( lpMetaGame->m_stGameStruct.bPlayingHodj == FALSE ) && ( lpMetaGame->m_cPodj.m_bComputer) && ( nChallengePhase < 10 )){
        SetComputerScore();
        goto PHASE4;
        }
        break;

    case 4:   // compare the player's score, if challenger won, give him mish and mosh, and another turn
PHASE4:
        if ( lChallengeScore > DetermineChallengeScore() ) {
        // give Mish and Mosh to challengee and give the turn back to the challengee
        if ( lpMetaGame->m_cHodj.m_bHaveMishMosh ) {
        CItem   *pItem = NULL;
                    {
                C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", "Podj has won Mish,", "Mosh and a turn." );
                        cMsgBox.DoModal();
                    }
            pItem = lpMetaGame->m_cHodj.m_pInventory->FindItem( MG_OBJ_MISH );
            lpMetaGame->m_cHodj.m_pInventory->RemoveItem( pItem );
            lpMetaGame->m_cPodj.m_pInventory->AddItem( pItem );
            pItem = NULL;
            pItem = lpMetaGame->m_cHodj.m_pInventory->FindItem( MG_OBJ_MOSH );
            lpMetaGame->m_cHodj.m_pInventory->RemoveItem( pItem );
            lpMetaGame->m_cPodj.m_pInventory->AddItem( pItem );
            pItem = NULL;
            lpMetaGame->m_cHodj.m_bHaveMishMosh = FALSE;
            lpMetaGame->m_cPodj.m_bHaveMishMosh = TRUE;
            lpMetaGame->m_cHodj.m_bMoving = TRUE;
            lpMetaGame->m_cPodj.m_bMoving = FALSE;
        }
        else {
        CItem   *pItem = NULL;
                    {
                C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", "Hodj has won Mish,", "Mosh and a turn." );
                        cMsgBox.DoModal();
                    }
            pItem = lpMetaGame->m_cPodj.m_pInventory->FindItem( MG_OBJ_MISH );
            lpMetaGame->m_cPodj.m_pInventory->RemoveItem( pItem );
            lpMetaGame->m_cHodj.m_pInventory->AddItem( pItem );
            pItem = NULL;
            pItem = lpMetaGame->m_cPodj.m_pInventory->FindItem( MG_OBJ_MOSH );
            lpMetaGame->m_cPodj.m_pInventory->RemoveItem( pItem );
            lpMetaGame->m_cHodj.m_pInventory->AddItem( pItem );
            pItem = NULL;
            lpMetaGame->m_cPodj.m_bHaveMishMosh = FALSE;
            lpMetaGame->m_cHodj.m_bHaveMishMosh = TRUE;
            lpMetaGame->m_cPodj.m_bMoving = TRUE;
            lpMetaGame->m_cHodj.m_bMoving = FALSE;
        }
        }
        else {
        if ( lpMetaGame->m_stGameStruct.bPlayingHodj ) {
            C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Podj Challenges;", "Hodj keeps", "Mish and Mosh." );
                    cMsgBox.DoModal();
        }
        else {
            C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Hodj Challenges;", "Podj keeps", "Mish and Mosh." );
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
    return(nGameID);
}

void CHodjPodjWindow::SetComputerScore()
{
    int anScrambledCurve[BELLCURVE];
    int nBottomScore = 0;
    int nTopScore = 0;
    int nBellScore = 0;
    int i, j;
    BOOL    bTemp;

    for (i = 0; i < BELLCURVE; i++)
        anScrambledCurve[i] = 0;

    for ( i = 0; i < BELLCURVE; i++ ) {
        bTemp = FALSE;
        do {
            j = brand() % BELLCURVE;
            if ( anScrambledCurve[j] == 0 ) {
        anScrambledCurve[j] = (int)anBellCurve[i];
                bTemp = TRUE;
            }
        } while ( bTemp == FALSE );
    }

    switch ( nChallengeGame ) {
    
    case  MG_GAME_ARCHEROIDS: // 1 or 0
    case  MG_GAME_ARTPARTS: // 1 or 0
    case  MG_GAME_BATTLEFISH: // 1 or 0
    case  MG_GAME_MANKALA: // 1 or 0
    case  MG_GAME_MAZEODOOM: // 1 or 0
    case  MG_GAME_RIDDLES: // 1 or 0
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
                case SKILLLEVEL_LOW :
                lpMetaGame->m_stGameStruct.lScore = ProbableTrue( 80 ); 
                    break;
                case SKILLLEVEL_MEDIUM :
                lpMetaGame->m_stGameStruct.lScore = ProbableTrue( 60 ); 
                    break;
                case SKILLLEVEL_HIGH :
                lpMetaGame->m_stGameStruct.lScore = ProbableTrue( 40 ); 
                    break;
            }
        break;

    case  MG_GAME_BEACON:
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
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
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
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
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
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
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
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
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
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
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
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
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
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
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
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
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
                case SKILLLEVEL_LOW :
                    if ( ProbableTrue( 20 ) ) {
                        lpMetaGame->m_stGameStruct.lScore = 21;
                        goto ENDCASE;
                    }
                nBottomScore = 1;
                nTopScore = 15;
                    break;
                case SKILLLEVEL_MEDIUM :
                    if ( ProbableTrue( 15 ) ) {
                        lpMetaGame->m_stGameStruct.lScore = 37;
                        goto ENDCASE;
                    }
                nBottomScore = 1;
                nTopScore = 31;
                    break;
                case SKILLLEVEL_HIGH :
                    if ( ProbableTrue( 10 ) ) {
                        lpMetaGame->m_stGameStruct.lScore = 53;
                        goto ENDCASE;
                    }
                nBottomScore = 1;
                nTopScore = 47;
                    break;
            }
            break;

    case  MG_GAME_CRYPTOGRAMS : // number
            switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
                case SKILLLEVEL_LOW :
                    if ( ProbableTrue( 30 ) ) {
                        lpMetaGame->m_stGameStruct.lScore = 100;
                        goto ENDCASE;
                    }
                nBottomScore = 24;
                nTopScore = 48;
                    break;
                case SKILLLEVEL_MEDIUM :
                    if ( ProbableTrue( 25 ) ) {
                        lpMetaGame->m_stGameStruct.lScore = 100;
                        goto ENDCASE;
                    }
                nBottomScore = 20;
                nTopScore = 48;
                    break;
                case SKILLLEVEL_HIGH :
                    if ( ProbableTrue( 20 ) ) {
                        lpMetaGame->m_stGameStruct.lScore = 100;
                        goto ENDCASE;
                    }
                nBottomScore = 16;
                nTopScore = 48;
                    break;
            }

CALCSCORE:
        nBellScore = (int)anBellCurve[brand() % BELLCURVE];
        lpMetaGame->m_stGameStruct.lScore = ((((( nTopScore - nBottomScore ) * 10 ) / 19 ) * nBellScore ) / 10 ) + nBottomScore; 
ENDCASE:
        break;

    case  MG_GAME_PEGGLEBOZ : // number
        i = brand() % 100;
        lpMetaGame->m_stGameStruct.lScore = 1;

        if ( i < 5 )
            lpMetaGame->m_stGameStruct.lScore = 25;

        if (( i >= 5 ) && ( i < 13 ))
            lpMetaGame->m_stGameStruct.lScore = 10;

        if (( i >= 13 ) && ( i < 25 ))
            lpMetaGame->m_stGameStruct.lScore = 5;

        if (( i >= 25 ) && ( i < 40 ))
            lpMetaGame->m_stGameStruct.lScore = 4;

            break;
            
    }
}


// determines the challenge score on a 0-100 scale
long CHodjPodjWindow::DetermineChallengeScore()
{
    char    cTemp[32];
    long    lReturn = 0L;
    long    lTemp = 0L;

    switch ( nChallengeGame ) {
    
        case  MG_GAME_ARCHEROIDS : // 1 or 0
        case  MG_GAME_ARTPARTS : // 1 or 0
        case  MG_GAME_BATTLEFISH : // 1 or 0
        case  MG_GAME_MANKALA : // 1 or 0
        case  MG_GAME_MAZEODOOM : // 1 or 0
        case  MG_GAME_RIDDLES : // 1 or 0
            lReturn = lpMetaGame->m_stGameStruct.lScore * 100;
            break;

        case  MG_GAME_BEACON : //  %
            lReturn = min( 100, (lpMetaGame->m_stGameStruct.lScore * 2));
            break;
            
        case  MG_GAME_LIFE : // number
            lReturn = min( 100, ((lpMetaGame->m_stGameStruct.lScore * 25) / 10 ));
            break;
            
        case  MG_GAME_THGESNGGME : // %
            lReturn = min( 100, ((lpMetaGame->m_stGameStruct.lScore * 15) / 10 ));
            break;
            
        case  MG_GAME_CRYPTOGRAMS : // number
            lReturn = lpMetaGame->m_stGameStruct.lScore;
            break;                                      

        case  MG_GAME_PEGGLEBOZ : // number
        case  MG_GAME_GARFUNKEL : // number
        case  MG_GAME_WORDSEARCH : // number
            lReturn = ( lpMetaGame->m_stGameStruct.lScore * 100 ) / 25;
            break;

        case  MG_GAME_BARBERSHOP : // number of cards discarded
            lReturn = ( lpMetaGame->m_stGameStruct.lScore * 100 ) / 62;
            break;

        case  MG_GAME_NOVACANCY : // number
            lReturn = 100 - (( lpMetaGame->m_stGameStruct.lScore * 100 ) / 45 );
            break;

        case  MG_GAME_DAMFURRY : // number
            lReturn = ( lpMetaGame->m_stGameStruct.lScore * 100 ) / 60;
            break;

        case  MG_GAME_FUGE : // number
            lReturn = ( lpMetaGame->m_stGameStruct.lScore * 100 ) / 53;
            break;

    case  MG_GAME_PACRAT : // number
        lTemp = lpMetaGame->m_stGameStruct.lScore * 100;
        switch ( lpMetaGame->m_stGameStruct.nSkillLevel ) {
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

    if ( nChallengeGame == MG_GAME_VIDEOPOKER ) {
        Common::sprintf_s( cTemp, "%li", lReturn );
    }
    else {
        Common::sprintf_s( cTemp, "%li / 100", lReturn );
    }

    if ( lpMetaGame->m_stGameStruct.bPlayingHodj ) {
    C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Hodj's Score :", " ", cTemp );
        cMsgBox.DoModal();
    }
    else {
    C1ButtonDialog  cMsgBox( (CWnd *)this, pGamePalette, "&OK", "Podj's Score :", " ", cTemp );
        cMsgBox.DoModal();
    }

    return(lReturn);
}


void CHodjPodjWindow::OnParentNotify(UINT msg, LPARAM lParam)
{
BOOL        bMainDlg = TRUE;
LPARAM      nGameReturn;

    switch (msg){
    case WM_DESTROY:
    
        if (bReturnToMeta && (lpMetaGame != NULL)) {
        bSoundEffectsEnabled = (*lpMetaGame).m_stGameStruct.bSoundEffectsEnabled;
        bMusicEnabled = (*lpMetaGame).m_stGameStruct.bMusicEnabled;
        bScrollingEnabled = (*lpMetaGame).m_bScrolling;
        }
        else
        if (bReturnToGrandTour && (lpGrandTour != NULL)) {
        bSoundEffectsEnabled = (*lpGrandTour).stMiniGame.bSoundEffectsEnabled;
        bMusicEnabled = (*lpGrandTour).stMiniGame.bMusicEnabled;
        }
    
        nGameReturn = lParam;
        
        if ( hDllInst > HINSTANCE_ERROR ) {

        (void) PositionAtHomePath();

        if ( nGameReturn < 0 ) {
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
            PostMessage( WM_COMMAND, IDC_PLAY_HODJ_MOVIE );

        } else if ( nGameReturn == MG_DLLX_PODJ_WINS ) {

            bReturnToZoom = FALSE;
            bReturnToMeta = FALSE;
            bReturnToGrandTour = FALSE;
            bMainDlg = FALSE;
            nGameReturn = -1;
            nChallengePhase = 0;
            PostMessage( WM_COMMAND, IDC_PLAY_PODJ_MOVIE );

        } else {
            if ( nGameReturn == MG_GAME_CHALLENGE )
            nChallengePhase = 1;
            nGameReturn = UpdateChallengePhase(nGameReturn);
        }

        LoadNewDLL( nGameReturn );
        if ( hDllInst == 0 ) {
            UpdateWindow();
            if ( bMainDlg )
            PostMessage( WM_COMMAND, IDC_MAINDLG );
        }

        return;
        }

        UpdateWindow();
        break;
    }

    CWnd::OnParentNotify(msg, lParam);
}


void CHodjPodjWindow::GetProfilePath(void)
{
int     i;

    i = GetWindowsDirectory((LPSTR) chProfilePath,PATHSPECSIZE);
    if (i < 1)
    chProfilePath[0] = '\0';
    else
    if (i > 3)
    strcat(chProfilePath,"\\");

    strcat(chProfilePath,AfxGetApp()->m_pszProfileName);
}


void CHodjPodjWindow::GetHomePath(void)
{
int     i;

    if (GetModuleFileName(hExeInst,chHomePath,PATHSPECSIZE)) {
    for (i = strlen(chHomePath) - 1; i >= 2; i--) {
        if (i == 2) {
        chHomePath[3] = '\0';
        break;
        }
        if (chHomePath[i] == '\\') {
        chHomePath[i] = '\0';
        break;
        }
    }
    }
    else
    if (_getdcwd(_getdrive(),chHomePath,PATHSPECSIZE) == NULL) {
    strcpy(chHomePath,"c:\\");
    chHomePath[0] = _getdrive() + 'A' - 1;
    }

    (void) PositionAtHomePath();
    
    bHomeWriteLocked = DriveWriteLocked();
}


BOOL CHodjPodjWindow::PositionAtHomePath(void)
{
int     status;
struct  _stat myStats;

    status = _stat(chHomePath,&myStats);
    if (status != 0)
    return(FALSE);
    
    nHomeDrive = myStats.st_dev + 1;
    
    if (_chdrive(nHomeDrive))
    return(FALSE);
    
    if (_chdir(chHomePath))
    return(FALSE);
    
    return(TRUE);
}


BOOL CHodjPodjWindow::GetCDPath(void)
{
    strcpy(chCDPath,AfxGetApp()->GetProfileString("Meta","CDROMDrive",NULL));

    if (chCDPath[0] == '\0') {
    if (bHomeWriteLocked)
        chCDPath[0] = chHomePath[0];
    else
    if (!FindCDROM())
        return(FALSE);
    }

    strcpy(&chCDPath[1],CDROMHOME);

    if (!PositionAtCDPath()) {
    chCDPath[0] = '\0';
    if (!FindCDROM())
        return(FALSE);
    strcpy(&chCDPath[1],CDROMHOME);
    if (!PositionAtCDPath()) {
        chCDPath[0] = '\0';
        return(FALSE);
    }
    }

    if (chHomePath[0] == chCDPath[0])
    bPathsDiffer = FALSE;
    else    
    bPathsDiffer = TRUE;
    
    return(TRUE);
}


BOOL CHodjPodjWindow::PositionAtCDPath(void)
{
int     status;
struct  _stat myStats;

    bHaveCDROM = FALSE;
    
    status = _stat(chCDPath,&myStats);
    if (status != 0)
    return(FALSE);
    
    nCDDrive = myStats.st_dev + 1;
    
    if (_chdrive(nCDDrive))
    return(FALSE);
    
    if (_chdir(chCDPath))
    return(FALSE);

    bHaveCDROM = TRUE;

    return(TRUE);
}


BOOL CHodjPodjWindow::PositionAtMiniPath(int nWhichDLL)
{
int     status;
struct  _stat myStats;

    switch(nInstallCode) {
        case INSTALL_FULL:
            switch(nWhichDLL + MG_GAME_BASE) {
                case MG_GAME_ARTPARTS:
                case MG_GAME_BEACON:
                case MG_GAME_RIDDLES:
                    goto remains_on_cd;
            }
            if (!PositionAtHomePath())
            return(FALSE);
            break;
        case INSTALL_NONE:
        case INSTALL_MINIMAL:
        case INSTALL_BASIC:
        case INSTALL_EXTRA:
remains_on_cd:
            if (!bHaveCDROM &&
            !GetCDPath())
            return(FALSE);
            if (!PositionAtCDPath())
            return(FALSE);
            break;
    }

    status = _stat(chMiniPath,&myStats);
    if (status != 0)
    return(FALSE);
    
    nMiniDrive = myStats.st_dev + 1;
    
    if (_chdrive(nMiniDrive))
    return(FALSE);
    
    if (_chdir(chMiniPath))
    return(FALSE);
    
    return(TRUE);
}


BOOL CHodjPodjWindow::FindCDROM(void)
{
int     nDevID;
char    chDevPath[PATHSPECSIZE];
BOOL    bSuccess = FALSE;

    strcpy(chDevPath,"A");
    strcat(chDevPath,CDROMHOME);

    for (nDevID = 'A'; nDevID <= 'Z'; nDevID++) {
    chDevPath[0] = nDevID;
    if (!_access(chDevPath,00))
        if (!_chdrive(nDevID - 'A' + 1))
        if (!_chdir(chDevPath) &&
            !_chdir(MOVIE_PATH)) {
            chCDPath[0] = nDevID;
            AfxGetApp()->WriteProfileString("Meta","CDROMDrive",chCDPath);
            bSuccess = TRUE;
            break;
        }
    }

    (void) PositionAtHomePath();
    
    return(bSuccess);
}


BOOL CHodjPodjWindow::DriveWriteLocked(void)
{
FILE    *sfp;

    sfp = fopen(TESTFILESPEC,"w");
    if (!sfp)
    return(TRUE);

    (void) fclose(sfp);
    (void) remove(TESTFILESPEC);

    return(FALSE);
}


void CHodjPodjWindow::StartBackgroundMidi(void)
{
    CWinApp *pMyApp;

    pMyApp = AfxGetApp();

    bMusicEnabled = pMyApp->GetProfileInt("Meta","Music",TRUE);

    if (bMusicEnabled && (pBackgroundMidi == NULL)) {
        PositionAtHomePath();
        pBackgroundMidi = new CSound(this,LOGO_MIDI,SOUND_MIDI | SOUND_LOOP /* | SOUND_DONT_LOOP_TO_END */ );
        (*pBackgroundMidi).play();
    }
}


void CHodjPodjWindow::StopBackgroundMidi(void)
{
    if (pBackgroundMidi != NULL) {
        (*pBackgroundMidi).Stop();
        delete pBackgroundMidi;
        pBackgroundMidi = NULL;
    }
}


void CHodjPodjWindow::OnDestroy()
{                              
    CFrameWnd::OnDestroy();
}


void CHodjPodjWindow::OnClose()
{
    if ( hDllInst > HINSTANCE_ERROR ) {
    if (hwndGame != NULL)
        ::SetActiveWindow( hwndGame );
    return;
    }
    
    ReleaseResources();
    
    SaveProfileSettings();
    
    CFrameWnd::OnClose();
}


void CHodjPodjWindow::ReleaseResources(void)
{
    CSound::clearSounds();
    
    if (bMetaLoaded) {
    if ( hMetaInst > HINSTANCE_ERROR ) {
        FreeLibrary(hMetaInst);
        hMetaInst = NULL;
    }
    }

    if ( lpGrandTour != NULL ) { 
    delete lpGrandTour;
    lpGrandTour = NULL;
    }
    
    if ( lpGameStruct != NULL ) {
    delete lpGameStruct;
    lpGameStruct = NULL;
    }

    assert(lpMetaGame != NULL);
    if (lpMetaGame != NULL) {
    FreeBFCInfo(lpMetaGame);
    delete lpMetaGame;
    lpMetaGame = NULL;
    }

    if ( pGamePalette != NULL) {
    delete pGamePalette;    
    pGamePalette = NULL;
    }
    
    (void) SystemParametersInfo(SPI_SETSCREENSAVEACTIVE,bScreenSaver,NULL,0);
}

VOID FreeBFCInfo(CBfcMgr *pBfcMgr)
{
    CHodjPodj *pPlayer;
    int i, k;

    assert(pBfcMgr != NULL);

    pBfcMgr->m_bRestoredGame = FALSE;

    // delete any Mish/Mosh items
    //
    if (pBfcMgr->m_pMishItem != NULL) {
    delete pBfcMgr->m_pMishItem;
    pBfcMgr->m_pMishItem = NULL;
    }
    if (pBfcMgr->m_pMoshItem != NULL) {
    delete pBfcMgr->m_pMoshItem;
    pBfcMgr->m_pMoshItem = NULL;
    }

    pPlayer = &pBfcMgr->m_cHodj;
    for (k = 0; k < 2; k++) {

    if (k == 1) {
        pPlayer = &pBfcMgr->m_cPodj;
    }

    if ( pPlayer->m_pBlackMarket != NULL ) {
        delete pPlayer->m_pBlackMarket;
        pPlayer->m_pBlackMarket = NULL;
    }

    if ( pPlayer->m_pGenStore != NULL ) {
        delete pPlayer->m_pGenStore;
        pPlayer->m_pGenStore = NULL;
    }

    if ( pPlayer->m_pTradingPost != NULL ) {
        delete pPlayer->m_pTradingPost;
        pPlayer->m_pTradingPost = NULL;
    }

    if ( pPlayer->m_pInventory != NULL ) {
        delete pPlayer->m_pInventory;
        pPlayer->m_pInventory = NULL;
    }

    // This deallocation MUST be after the delete pInventorys
    //
    for ( i = 0; i < NUMBER_OF_CLUES; i++ ) {

        if (!pPlayer->m_aClueArray[i].bUsed) {

        if (pPlayer->m_aClueArray[i].pNote != NULL) {
            delete pPlayer->m_aClueArray[i].pNote;
            pPlayer->m_aClueArray[i].pNote = NULL;
        }
        }
    }
    }
}


VOID InitBFCInfo(CBfcMgr *pBfcMgr)
{
    CClueTable *pClueTable;
    CHodjPodj *pPlayer;
    CItem *pItem;
    int i, j, k;

    assert(pBfcMgr != NULL);

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

    pPlayer->m_pTradingPost = NULL;
    pPlayer->m_pBlackMarket = NULL;

    if ((pPlayer->m_pInventory = new CInventory(k == 0 ? "Hodj's Stuff" : "Podj's Stuff")) != NULL) {

        pPlayer->m_pInventory->AddItem(k == 0 ? MG_OBJ_HODJ_NOTEBOOK : MG_OBJ_PODJ_NOTEBOOK, 1);
        pItem = pPlayer->m_pInventory->FindItem(k == 0 ? MG_OBJ_HODJ_NOTEBOOK : MG_OBJ_PODJ_NOTEBOOK);
        (*pItem).SetActionCode(ITEM_ACTION_NOTEBOOK);
        pPlayer->m_pInventory->AddItem( MG_OBJ_CROWN, 20);
    }

    pszTest = (CHAR *)&"Corruption Test";

    if ((pPlayer->m_pGenStore = new CInventory("General Store")) != NULL) {
        for ( i = MG_OBJ_BASE; i <= MG_OBJ_MAX; i++ ) {
        switch ( i ) {
            case MG_OBJ_HERRING:
            case MG_OBJ_MISH:
            case MG_OBJ_MOSH:
            case MG_OBJ_HODJ_NOTEBOOK:
            case MG_OBJ_PODJ_NOTEBOOK:
            case MG_OBJ_CROWN:
            break;
            default:
            pPlayer->m_pGenStore->AddItem( i,1 );
            break;
        }
        }

        if ((pPlayer->m_pBlackMarket = new CInventory("Black Market")) != NULL) {
        for  ( i = 0; i < ITEMS_IN_BLACK_MARKET; i ++ ) {
            j = brand() % pPlayer->m_pGenStore->ItemCount();
            pItem = pPlayer->m_pGenStore->FetchItem( j );
            pPlayer->m_pGenStore->RemoveItem( pItem );

            pPlayer->m_pBlackMarket->AddItem( pItem );
        }
        }
    }

    pClueTable = CMgStatic::cHodjClueTable;
    if (k == 1)
        pClueTable = CMgStatic::cPodjClueTable;

    for ( i = 0; i < NUMBER_OF_CLUES; i++ ) {

        pPlayer->m_aClueArray[i].pNote = new CNote( -1,
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
    strcpy(pBfcMgr->m_chHomePath,chHomePath);
    strcpy(pBfcMgr->m_chCDPath,chCDPath);
    strcpy(pBfcMgr->m_chMiniPath,chMiniPath);

    // create Mish/Mosh items
    assert(pBfcMgr->m_pMishItem == NULL);
    pBfcMgr->m_pMishItem = new CItem(MG_OBJ_MISH);

    assert(pBfcMgr->m_pMoshItem == NULL);
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

void CHodjPodjWindow::FlushInputEvents(void)
{
    MSG msg;

while(TRUE) {                                       // find and remove all keyboard events
    if (!PeekMessage(&msg,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE))
    break;}

while(TRUE) {                                       // find and remove all mouse events
    if (!PeekMessage(&msg,NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE))
    break;}
}


void CHodjPodjWindow::HandleError(ERROR_CODE errCode)
{
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
BOOL CHodjPodjWindow::Restore(VOID)
{
    BOOL bSuccess;
    ERROR_CODE errCode;

    // assume no error
    errCode = ERR_NONE;

    // assume Restore will work
    bSuccess = TRUE;

    // validate implicit input
    assert(gpszSaveGameFile != NULL);
    assert(gpszSaveDLL != NULL);
    assert(lpMetaGame != NULL);
    assert(pGamePalette != NULL);

    PositionAtHomePath();

#if SAVEDLL
    LPSAVEFUNC lpfnRestore;
    HINSTANCE hInst;

    //
    // Load the HNPSAVE.DLL and invode it's RestoreGame
    //
    hInst = LoadLibrary(gpszSaveDLL);

    if (hInst > HINSTANCE_ERROR) {

    lpfnRestore = (LPSAVEFUNC)::GetProcAddress(hInst, "RestoreGame");
    if (lpfnRestore != NULL) {

        // call RestoreGame()
        bSuccess = lpfnRestore(gpszSaveGameFile, lpMetaGame, (CWnd *)this, pGamePalette, &errCode);

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
        strcpy(lpMetaGame->m_chHomePath, chHomePath);
        strcpy(lpMetaGame->m_chCDPath, chCDPath);
        strcpy(lpMetaGame->m_chMiniPath, chMiniPath);

    } else {
        errCode = ERR_UNKNOWN;
    }

    FreeLibrary(hInst);

    } else {
    errCode = ERR_FFIND;
    }

#else
    bSuccess = RestoreGame(gpszSaveGameFile, lpMetaGame, (CWnd *)this, pGamePalette, &errCode);

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
    strcpy(lpMetaGame->m_chHomePath, chHomePath);
    strcpy(lpMetaGame->m_chCDPath, chCDPath);
    strcpy(lpMetaGame->m_chMiniPath, chMiniPath);

#endif

    if (errCode != ERR_NONE) {
    HandleError(errCode);
    bSuccess = FALSE;
    }

    return(bSuccess);
}


VOID CHodjPodjWindow::ShowCredits(VOID)
{
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

    if ((pDC = GetDC()) != NULL) {

        // load first bitmap
        m_pCurrentBmp = NULL;
        if (FileExists(stCredits[0].m_pszCelFile)) {
            m_pCurrentBmp = FetchBitmap(pDC, NULL, stCredits[0].m_pszCelFile);
        }

        // for each credit screen
        //
        for (i = 0; i < MAX_CREDITS; i++) {

            // Paint bitmap onto sreen
            //
            if (m_pCurrentBmp != NULL) {
                PaintBitmap(pDC, NULL, m_pCurrentBmp, 0, 0);
                delete m_pCurrentBmp;
                m_pCurrentBmp = NULL;
            }

            // get current time
            lStart = GetTickCount();

            // pre-load next bitmap
            //
            if (i < (MAX_CREDITS - 1)) {
                if (FileExists(stCredits[i+1].m_pszCelFile)) {
                    m_pCurrentBmp = FetchBitmap(pDC, NULL, stCredits[i+1].m_pszCelFile);
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
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
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
        if (m_pCurrentBmp != NULL) {
            delete m_pCurrentBmp;
            m_pCurrentBmp = NULL;
        }
        ReleaseDC(pDC);
    }

    // paint black
    BlackScreen();

    // stop theme music
    StopBackgroundMidi();

    m_bInCredits = FALSE;
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

BOOL CTheApp::InitInstance()
{
    CString         lpszCommandLine = m_lpCmdLine;

    m_pMainWnd = new CHodjPodjWindow();
    m_pMainWnd->ShowWindow( SW_SHOWNORMAL );
    m_pMainWnd->UpdateWindow();

    return(TRUE);
}


/*****************************************************************
 *
 * ExitInstance
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This routine is automatically called when the application is
 *  being terminated. 
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
 *      int         Success (0) / Failure status
 *
 ****************************************************************/
int CTheApp::ExitInstance()
{
    if ( hDllInst > HINSTANCE_ERROR ) {
    if (hwndGame != NULL)
        ::SendMessage( hwndGame, WM_CLOSE, 0, 0L );

    FreeLibrary( hDllInst );
    hDllInst = NULL;
    }
    
    if ( hMetaInst > HINSTANCE_ERROR ) {
    if (hwndGame != NULL)
        ::SendMessage( hwndGame, WM_CLOSE, 0, 0L );

    FreeLibrary( hMetaInst );
    hMetaInst = NULL;
    }
    
    if ( m_pMainWnd != NULL )
    m_pMainWnd->SendMessage( WM_CLOSE );

    return(0);
}


// CHodjPodjWindow message map:
// Associate messages with member functions.
//                                        
BEGIN_MESSAGE_MAP( CHodjPodjWindow, CFrameWnd )
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
    ON_MESSAGE( MM_MCINOTIFY, OnMCINotify)
//    ON_MESSAGE( MM_MCINOTIFY, OnMMNotify)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
