// game.cpp : Defines the class behaviors for the application.
//           Game is a simple skeleton program which consists of a main
//           window, which contains background artwork from a .BMP file,
//			 and a Quit button.  Stub routines are present for mouse related
//			 activities and keyboard input; edit the stubs as required.
//
//
//
//
/****************************************************
 *                                                  *
 *                P E G G L E B O Z                 *
 *                                                  *
 *              By:  Christopher Lee                *
 *                                                  *
 *             Coding started: 3/3/94               *
 *                                                  *
 ****************************************************
 *
 * v.1.0:  3/16/94 - Peggleboz works!
 *                   Functions like Hi-Q game; move logic
 *					 in the form of click and drag
 *
 * v.1.1:  3/17/94 - Added Undo Move Button:  takes back
 *					 last move (until the beginning)
 *
 * v.1.2:  3/18/94 - Added Restart Button:  reloads board
 *					 and restart from the beginning
 *
 * v.1.3:  3/21/94 - Added end of game sequences, scoring,
 *					 "No Moves Left" Message, Yes/No button
 *                   for play again which restarts/quits.
 *                 - Also added neighbor checks (forgot previously)
 *
 * v.1.4:  3/22/94 - Diagonal moves on CROSS and CROSS PLUS cases
 *					 taken out (not supposed to be legal moves)
 *                   Triangle and Triangle Plus transformations
 *					 (display and move validation) completed.
 *
 * v.1.5:  3/23/94 - Click and Drag changed to click and click
 *
 * v.1.6:  3/24/94 - Fixed up some bugs and displays
 *
 * v.1.7:  3/25/94 - Bitmap "cursors" completed.  All warnings
 *					 taken out.  0 errors, 0 warnings.
 *
 * v.1.8:  3/28/94 - No total refresh, local refresh only, but a
 *					 bit buggy, can't figure out the right sequence.
 *
 * v.1.9:  3/30/94 - Added Dialog box (no commands yet)
 *
 * v.1.10: 3/31/94 - Fixed bugs with normal move sequence
 *					 As well, if you jump weirdly (wrongly),
 *					 the peg is replaced.  Undo bug removed.
 *
 * v.2.0:  4/3/94 -  All bugs removed.  Dialog box now
 *                   selects game type.
 *
 * v.3.0   5/1/94	Modularized all components, added rules and scrolls
 *
 * v.3.1   5/1/94	added scroll dialog boxes and wider hole sprite
 *
 * v.3.2   5/4/94	corrected "you won" determination; special case 1 point scores
 *
 * v.3.3   5/4/94	corrected "you won" determination one last time ...
 *
 * v.3.4   5/10/94	reset input focus to options dialog after mini-options dialog
 *
 * v.3.5   5/11/94  set input focus to Return to Game after Options/Rules
 *
 * v.3.6   5/16/94	convert art from disk based to resource based
 *
 * v.3.7   6/16/94	refresh dialog backgrounds upon dialog exits
 *
 * v.3.8   7/06/94	remove superflous sprite creation in SetupBoard
 */

#define SHOW_CURSOR	TRUE

#include "stdafx.h"
#include <time.h>

#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "misc.h"
#include "resource.h"
#include "gamedll.h"
#include "game.h"
#include "dibdoc.h"
#include "sprite.h"
#include "globals.h"
#include "game2.h"
#include "options.h"
#include "time.h"
//#include "gameover.h"
#include "rules.h"
#include "button.h"
#include "sound.h"

extern	LPGAMESTRUCT 	pGameInfo;

void setup_cursor(void);
void set_wait_cursor(void);
void reset_wait_cursor(void);

CBmpButton	*pScrollButton = NULL;
CRect		ScrollRect;

int				sprite_count = 0,
				counter = 0 ;

CPalette		*pGamePalette = NULL ;
static CSound	*pGameSound = NULL;					// Game theme song

// Board Selection stuff
BOOL bRandomBoard = FALSE;
char BoardSelected = CROSS ;
static char MaxPegs[4][2] =
	{CROSS,	CROSS_PLUS, TRIANGLE, 	TRIANGLE_PLUS,
	 32,	36,			14,		    20} ;
char fState [GRID_SIZE][GRID_SIZE];
char *BoardSpec[BOARD_COUNT] = {".\\ART\\CROSS.BMP",".\\ART\\CROSSX.BMP",".\\ART\\TRIANGLE.BMP",".\\ART\\TRIANGLX.BMP"};
char Moves [70][2]; // i.e. 2 pairs of coordinates (old, new) per
					// move, and 1 peg can be removed per move.

CSprite *pCursorSprite = NULL;

CSprite *pShotGlass = NULL;
CSprite *pTableSlot = NULL;
CSprite *pInvalidSlot = NULL;

int pegz = (((BoardSelected == TRIANGLE) * 15) +
			   ((BoardSelected == TRIANGLE_PLUS) * 21) +
			   ((BoardSelected == CROSS) * 49) +
			   ((BoardSelected == CROSS_PLUS) * 49));

static	nBoard_DX = TRI_BOARD_DX,
		nBoard_DY = TRI_BOARD_DY;

static	bIgnoreScroll = FALSE;
static	bPegMoving = FALSE;

/////////////////////////////////////////////////////////////////////////////

// theApp:
// Just creating this application object runs the whole application.
//
// CTheApp NEAR theApp;

/////////////////////////////////////////////////////////////////////////////

// CMainWindow constructor:
// Create the window with the appropriate style, size, menu, etc.;
// it will be later revealed by CTheApp::InitInstance().  Then
// create our splash screen object by opening and loading its DIB.
//
CMainWindow::CMainWindow( HWND hCallingApp )
{
CString	WndClass;
CDC		*pDC;
CPalette	*pPalOld;
CDibDoc	*pDibDoc;
CRect	MainRect;
BOOL	bSuccess;


BeginWaitCursor();
m_hCallAppWnd = hCallingApp;

// select a type!
// Define a special window class which traps double-clicks, is byte aligned
// to maximize BITBLT performance, and creates "owned" DCs rather than sharing
// the five system defined DCs which are not guaranteed to be available;
// this adds a bit to our app size but avoids hangs/freezes/lockups.
WndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC,
								NULL, NULL, NULL);

// Center our window on the screen

pDC = GetDC();
MainRect.left = (pDC->GetDeviceCaps(HORZRES) - GAME_WIDTH) >> 1;
MainRect.top = (pDC->GetDeviceCaps(VERTRES) - GAME_HEIGHT) >> 1;
MainRect.right = MainRect.left + GAME_WIDTH;
MainRect.bottom = MainRect.top + GAME_HEIGHT;
ReleaseDC(pDC);

//MainRect.SetRect(10,10,GAME_WIDTH + 10,GAME_HEIGHT + 10);

// Create the window as a POPUP so no boarders, title, or menu are present;
// this is because the game's background art will fill the entire 640x40 area.
Create( WndClass, "Boffo Games -- Peggleboz", WS_POPUP, MainRect, NULL, NULL );

SplashScreen();

setup_cursor();

// Acquire the shared palete for our game from the splash screen art
pDibDoc = new CDibDoc();
bSuccess = (*pDibDoc).OpenDocument(BoardSpec[BoardSelected - BOARD_BASE]);
ASSERT(bSuccess);

pGamePalette = (*pDibDoc).DetachPalette();
delete pDibDoc;

pDC = GetDC();
pPalOld = (*pDC).SelectPalette(pGamePalette, FALSE);

pScrollButton = new CBmpButton;				// build a bitmapped OKAY button for resetting
ASSERT(pScrollButton);                            // ... the sprites, again as an example
ScrollRect.SetRect(SCROLL_BUTTON_X,
				 SCROLL_BUTTON_Y,
				 SCROLL_BUTTON_X + SCROLL_BUTTON_DX - 1,
				 SCROLL_BUTTON_Y + SCROLL_BUTTON_DY - 1);
bSuccess = (*pScrollButton).Create(NULL,BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, ScrollRect, this, IDC_SCROLL);
ASSERT(bSuccess);
bSuccess = (*pScrollButton).LoadBitmaps("SCROLLUP","SCROLLDOWN","SCROLLFOCUS","SCROLLDISABLED");
ASSERT(bSuccess);

pShotGlass = new CSprite;

(*pShotGlass).SharePalette(pGamePalette);
bSuccess = (*pShotGlass).LoadResourceSprite(pDC,"SHOTGLASS");
ASSERT(bSuccess);
(*pShotGlass).SetMasked(TRUE);
(*pShotGlass).SetMobile(TRUE);
(*pShotGlass).SetTypeCode(SPRITE_GLASS);

pTableSlot = new CSprite;

(*pTableSlot).SharePalette(pGamePalette);
bSuccess = (*pTableSlot).LoadResourceSprite(pDC,"TABLESLOT");
ASSERT(bSuccess);
(*pTableSlot).SetMasked(TRUE);
(*pTableSlot).SetMobile(TRUE);
(*pTableSlot).SetTypeCode(SPRITE_HOLE);

pInvalidSlot = new CSprite;

(*pInvalidSlot).SharePalette(pGamePalette);
bSuccess = (*pInvalidSlot).LoadResourceSprite(pDC,"INVALIDSLOT");
ASSERT(bSuccess);
(*pInvalidSlot).SetMasked(TRUE);
(*pInvalidSlot).SetMobile(TRUE);
(*pInvalidSlot).SetTypeCode(SPRITE_INVALID);

pCursorSprite = new CSprite;

(*pCursorSprite).SharePalette(pGamePalette);
bSuccess = (*pCursorSprite).LoadResourceSprite(pDC,"SHOTGLASS");
ASSERT(bSuccess);
(*pCursorSprite).SetMasked(TRUE);
(*pCursorSprite).SetMobile(TRUE);

	//srand((unsigned)time(NULL));

	if ((*pGameInfo).bPlayingMetagame)
		bRandomBoard = TRUE;
//		BoardSelected = (brand() % BOARD_COUNT) + BOARD_BASE;

	SetUpBoard(pDC);
	
	(void) (*pDC).SelectPalette(pPalOld, FALSE);
	ReleaseDC(pDC);

	if ((*pGameInfo).bMusicEnabled) {
		if (pGameSound = new CSound( this, GAME_THEME, 
										SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END ) ) {
			(*pGameSound).midiLoopPlaySegment( 1000, 30000, 0, FMT_MILLISEC );
		} // end if pGameSound
	}
	
	if ((*pGameInfo).bPlayingMetagame == FALSE) {
		PostMessage( WM_COMMAND, IDC_SCROLL, BN_CLICKED);
	}

#ifndef SHOW_CURSOR
	::ShowCursor(FALSE);
#endif
	EndWaitCursor();
}


void CMainWindow::OnActivateApp(BOOL bActive, HTASK hTask)
{
	m_bProgramActive = bActive;
}


// OnPaint:
// This is called whenever Windows sends a WM_PAINT message.
// Note that creating a CPaintDC automatically does a BeginPaint and
// an EndPaint call is done when it is destroyed at the end of this
// function.  CPaintDC's constructor needs the window (this).
//
void CMainWindow::OnPaint()
{
PAINTSTRUCT	lpPaint;

InvalidateRect(NULL,FALSE);
BeginPaint(&lpPaint);
SplashScreen();
EndPaint(&lpPaint);
}


// Paint the background art (splash screen) in the client area;
// called by both OnPaint and InitInstance.
void CMainWindow::SplashScreen()
{
CRect	rcDest;
CRect	rcDIB;
CDC		*pDC;
CDibDoc	myDoc;
HDIB	hDIB;
LPSTR 	lpDIB;
CSprite	*pSprite;
BOOL	bSuccess;

bSuccess = myDoc.OpenDocument(BoardSpec[BoardSelected - BOARD_BASE]);
ASSERT(bSuccess);
hDIB = myDoc.GetHDIB();

pDC = GetDC();
if (pDC && hDIB) {
	GetClientRect( rcDest );
	lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
	int cxDIB = (int) ::DIBWidth(lpDIB);
	int cyDIB = (int) ::DIBHeight(lpDIB);
	::GlobalUnlock((HGLOBAL) hDIB);
	rcDIB.top = rcDIB.left = 0;
	rcDIB.right = cxDIB;
	rcDIB.bottom = cyDIB;
	::PaintDIB((*pDC).m_hDC, &rcDest, hDIB, &rcDIB, pGamePalette);}

pSprite = CSprite::GetSpriteChain();
while(pSprite) {
    (*pSprite).ClearBackground();
	bSuccess = (*pSprite).RefreshSprite(pDC);
	ASSERT(bSuccess);
	pSprite = (*pSprite).GetNextSprite();}

ReleaseDC(pDC);
}


void SetUpBoard(CDC *pDC)
{
int 	i, j;
CSprite	*pNewSprite;
CPoint	cPoint;

CSprite::EraseSprites(pDC);
CSprite::FlushSpriteChain();

if (bRandomBoard)											// want a random board selected
	BoardSelected = (brand() % BOARD_COUNT) + BOARD_BASE;

switch(BoardSelected) {
	case CROSS_PLUS:
		nBoard_DX = CROSS_PLUS_BOARD_DX;
		nBoard_DY = CROSS_PLUS_BOARD_DY;
    	for (i=0; i < GRID_SIZE; i++) {
    		for (j=0; j < GRID_SIZE; j++) {
				fState [i][j] = Board_CrossPlus [i][j] ;
		 	}}
		break ;
	case CROSS:
		nBoard_DX = CROSS_BOARD_DX;
		nBoard_DY = CROSS_BOARD_DY;
    	for (i=0; i < GRID_SIZE; i++) {
    		for (j=0; j < GRID_SIZE; j++) {
				fState [i][j] = Board_Cross [i][j] ;
		 	}}
		break ;
	case TRIANGLE:
		nBoard_DX = TRI_BOARD_DX;
		nBoard_DY = TRI_BOARD_DY;
    	for (i=0; i < GRID_SIZE; i++) {
    		for (j=0; j < GRID_SIZE; j++) {
				fState [i][j] = Board_Triangle [i][j] ;
		 	}}
		break ;
	case TRIANGLE_PLUS:
		nBoard_DX = TRI_PLUS_BOARD_DX;
		nBoard_DY = TRI_PLUS_BOARD_DY;
    	for (i=0; i < GRID_SIZE; i++) {
    		for (j=0; j < GRID_SIZE; j++) {
				fState [i][j] = Board_TrianglePlus [i][j] ;
		 	}}
		break ;
		}

for (j=0; j < GRID_SIZE; j++) {
	for (i=0; i < GRID_SIZE; i++) {
		pNewSprite = NULL;
		if (fState[i][j] == PEGGED)
			pNewSprite = (*pShotGlass).DuplicateSprite(pDC);
		else
		if (fState[i][j] == EMPTY)
			pNewSprite = (*pTableSlot).DuplicateSprite(pDC);
		else
//		if ((fState[i][j] == NO_HOLE) && ((BoardSelected == CROSS) || (BoardSelected == CROSS_PLUS)))
			pNewSprite = (*pInvalidSlot).DuplicateSprite(pDC);
		ASSERT(pNewSprite != NULL);
		cPoint = CMainWindow::GridToPoint(i,j);
		if ((*pNewSprite).GetTypeCode() == SPRITE_HOLE)
			cPoint.x -= (SPRITE_SIZE_DX >> 1);	
		(*pNewSprite).SetPosition(cPoint);
		(*pNewSprite).LinkSprite();
      	}
	}

pNewSprite = NULL;

counter = 0;

}

/////////////////////////////////////////////////////////////////////////////
//
// Process messages and controls
//
/////////////////////////////////////////////////////////////////////////////

// OnCommand
// This function is called when a WM_COMMAND message is issued,
// typically in order to process control related activities.
//

BOOL CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
CDC     	*pDC;
CPoint  	sprite_loc;
CRules		RulesDlg( (CWnd *)this, RULESSPEC, pGamePalette, 
						((*pGameInfo).bSoundEffectsEnabled ? NARRATIVESPEC : NULL) );
COptions	COptionsWind((CWnd *)this, pGamePalette, IDD_OPTIONS_DIALOG) ;


if (HIWORD(lParam) == BN_CLICKED) {

	if (counter & 0x01) {
		pDC = GetDC();
		UndoMove(pDC);
		ReleaseDC(pDC);
		}

	switch (wParam) {
	
		case IDC_RULES:
			bIgnoreScroll = TRUE;
			(*pScrollButton).SendMessage(BM_SETSTATE,TRUE,0L);
			(void) RulesDlg.DoModal();      					// invoke the help dialog box
			break;

		case IDC_NEWGAME:
			if (!(*pGameInfo).bPlayingMetagame) {
				pDC = GetDC();
				SetUpBoard(pDC);
				ReleaseDC(pDC);
				InvalidateRect(NULL,FALSE);
			}
			break;

		case IDC_SCROLL:
			if (bIgnoreScroll) {
				(*pScrollButton).SendMessage(BM_SETSTATE,TRUE,0L);
				break;}
			bIgnoreScroll = TRUE;
			(*pScrollButton).SendMessage(BM_SETSTATE,TRUE,0L);
			SendDlgItemMessage(IDC_SCROLL,BM_SETSTATE,TRUE,0L);

			switch ( COptionsWind.DoModal() ) {

				case IDC_RETURN:
					(*pScrollButton).SendMessage(BM_SETSTATE,FALSE,0L);
					bIgnoreScroll = FALSE;
					break;

				case IDC_RESTART:
				case IDC_NEWGAME:
					(*pScrollButton).SendMessage(BM_SETSTATE,FALSE,0L);
					bIgnoreScroll = FALSE;
					pDC = GetDC();
					SetUpBoard(pDC);
					ReleaseDC(pDC);
					InvalidateRect(NULL,FALSE);
		            break;

				case IDC_QUIT:
					PostMessage( WM_CLOSE,0,0 );
					return( FALSE );
					
				default:
					(*pScrollButton).SendMessage(BM_SETSTATE,FALSE,0L);
					bIgnoreScroll = FALSE;

			} //end switch(ComDlg.DoModal())

			if ((*pGameInfo).bMusicEnabled) {
				if (pGameSound == NULL) {
					pGameSound = new CSound( this, GAME_THEME, 
											SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					if (pGameSound != NULL)
						(*pGameSound).midiLoopPlaySegment( 6370, 33000, 0, FMT_MILLISEC );
				}
			} // end if pGameSound
			else { 
				if (pGameSound != NULL) {
					pGameSound->stop();
					delete pGameSound;
					pGameSound = NULL;
				}
			}
			
		} //end switch(wParam)

	}

(*this).SetFocus();						// Reset focus back to the main window
return(TRUE);
}


// OnChar and OnSysChar
// These functions are called when keyboard input generates a character.
//

void CMainWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

CFrameWnd ::OnChar(nChar, nRepCnt, nFlags);		// default action
}


void CMainWindow::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

if ((nChar == 'q') && (nFlags & 0x2000)) {		// terminate app on ALT-q
	CSprite::FlushSpriteChain();
	PostMessage(WM_CLOSE,0,0);}                   // *** remove later ***
else
	CFrameWnd ::OnChar(nChar, nRepCnt, nFlags);	// default action
}


void CMainWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ( (nChar == VK_F1) ) {								// F1 key is hit
		SendMessage( WM_COMMAND, IDC_RULES, BN_CLICKED);	// Activate the Options dialog
		(*pScrollButton).SendMessage(BM_SETSTATE,FALSE,0L);
		bIgnoreScroll = FALSE;
	}
	else
	if ( nChar == VK_F2 ) {									// F2 key is hit
		SendMessage( WM_COMMAND, IDC_SCROLL, BN_CLICKED);	// Activate the Options dialog
		(*pScrollButton).SendMessage(BM_SETSTATE,FALSE,0L);
		bIgnoreScroll = FALSE;
	}
}


// OnMouseMove and OnButtonXXX:
// These functions are called whenever the corresponding WM_ mouse
// related message occurs.
//

void CMainWindow::OnMouseMove(UINT nFlags, CPoint point)
{
CDC    *pDC;
CPoint real_loc;

if (counter % 2) {
	real_loc.x = point.x - (SPRITE_SIZE_DX / 2);
	real_loc.y = point.y - (SPRITE_SIZE_DY / 2);
	pDC = GetDC();
	(*pCursorSprite).PaintSprite(pDC, real_loc);
	ReleaseDC(pDC);}

setup_cursor();
}


void CMainWindow::OnLButtonDown(UINT nFlags,CPoint myPoint)
{
// if you want to do click and click, just check the counter % 2
// if true, then store oldx, oldy, and first click case ("click")
// if false, then do newx, newy, and second click case ("drag")
CSprite	*pSprite;
int score = 0;
int num_left ;
int moves_left = 0;
int i, j;
char score_string[40];
char *score_blurb;
CDC     *pDC;
CPoint  sprite_loc;
CPoint  grid_loc;
CSound	*pEffect = NULL;
int newx, newy ;
int oldx , oldy, neighborx, neighbory;
BOOL	bSuccess;
CRect	bottleRect,
		titleRect,
		dartRect,
		kegRect,
		stoolRect,
		netRect,
		oarRect,
		signRect,
		tableRect,
		candlenrRect,
		candlefrRect;

	bottleRect.SetRect(BOTTLE_X, BOTTLE_Y, BOTTLE_X + BOTTLE_DX, BOTTLE_Y + BOTTLE_DY);
	titleRect.SetRect(TITLE_X, TITLE_Y, TITLE_X + TITLE_DX, TITLE_Y + TITLE_DY);
	dartRect.SetRect(EE_DART_X, EE_DART_Y, EE_DART_X + EE_DART_DX, EE_DART_Y + EE_DART_DY);
	kegRect.SetRect(KEG_X, KEG_Y, KEG_X + KEG_DX, KEG_Y + KEG_DY);
	stoolRect.SetRect(STOOL_X, STOOL_Y, STOOL_X + STOOL_DX, STOOL_Y + STOOL_DY);
	netRect.SetRect(NET_X, NET_Y, NET_X + NET_DX, NET_Y + NET_DY);
	oarRect.SetRect(OAR_X, OAR_Y, OAR_X + OAR_DX, OAR_Y + OAR_DY);
	signRect.SetRect(SIGN_X, SIGN_Y, SIGN_X + SIGN_DX, SIGN_Y + SIGN_DY);
	tableRect.SetRect(TABLE_X, TABLE_Y, TABLE_X + TABLE_DX, TABLE_Y + TABLE_DY);
	candlenrRect.SetRect(CANDLENR_X, CANDLENR_Y, CANDLENR_X + CANDLENR_DX, CANDLENR_Y + CANDLENR_DY);
	candlefrRect.SetRect(CANDLEFR_X, CANDLEFR_Y, CANDLEFR_X + CANDLEFR_DX, CANDLEFR_Y + CANDLEFR_DY);


	if (((*pGameInfo).bPlayingMetagame == FALSE) && titleRect.PtInRect(myPoint)) {
		if (!(counter & 0x01))
			UndoTurn();
		else {
			pDC = GetDC();
			UndoMove(pDC);
			ReleaseDC(pDC);
		}
		SendMessage( WM_COMMAND, IDC_NEWGAME, BN_CLICKED);
	}
	else
	if (((*pGameInfo).bPlayingMetagame == FALSE) && bottleRect.PtInRect(myPoint)) {
		if (!(counter & 0x01))
			UndoTurn();
		else {
			pDC = GetDC();
			UndoMove(pDC);
			ReleaseDC(pDC);}
	}
	else
	if (dartRect.PtInRect(myPoint)) {
		CSound::waitWaveSounds();
		sndPlaySound( NULL, 0 );
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels( pDC, ".\\art\\dart.bmp", NUM_DART_CELS );
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(FALSE);
		(*pSprite).SetMobile(FALSE); 
	
		if ((*pGameInfo).bSoundEffectsEnabled) {
			pEffect = new CSound( (CWnd *)this, ".\\sound\\darts.wav", 				// Load up the sound file as a 
									SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);	//...Wave file, to delete itself
		}								
		for( i = 0; i < NUM_DART_CELS; i++ ) {
			(*pSprite).PaintSprite( pDC, DART_X, DART_Y );
			Sleep( DART_SLEEP - (i) );	// * 2
			if (( i == 0 ) && (pEffect != NULL)) {
				bSuccess = (*pEffect).Play();														//...play the narration
			   	if (!bSuccess)
			   		delete pEffect;
			}
		}
		if (pSprite != NULL)
			delete pSprite; 
		
		ReleaseDC(pDC);
	}
	else
	if (kegRect.PtInRect(myPoint)) {
		CSound::waitWaveSounds();
		sndPlaySound( NULL, 0 );
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels( pDC, ".\\art\\keg.bmp", NUM_KEG_CELS );
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(FALSE);
		(*pSprite).SetMobile(FALSE); 
	
		if ((*pGameInfo).bSoundEffectsEnabled) {
			pEffect = new CSound( (CWnd *)this, ".\\sound\\barglass.wav",			// Load up the sound file as a 
									SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);				//...Wave file, to delete itself
		}									
		if (pEffect != NULL) {
		   	bSuccess = (*pEffect).Play();
		   	if (!bSuccess)
		   		delete pEffect;
		}
		(*pSprite).SetCel( NUM_KEG_CELS - KEG_CEL_OFFSET );
		for( i = 0; i < NUM_KEG_CELS; i++ ) {
			(*pSprite).PaintSprite( pDC, KEG_X, KEG_Y );
			Sleep( KEG_SLEEP );
		}
		if (pSprite != NULL)
			delete pSprite; 
		
		ReleaseDC(pDC);
	}
	else
	if (stoolRect.PtInRect(myPoint)) {
		CSound::waitWaveSounds();
		sndPlaySound( NULL, 0 );
		pDC = GetDC();
		pSprite = new CSprite;
		(*pSprite).SharePalette(pGamePalette);
		bSuccess = (*pSprite).LoadCels( pDC, ".\\art\\stool.bmp", NUM_STOOL_CELS );
		if (!bSuccess) {
			delete pSprite;
			ReleaseDC(pDC);
			return;
		}
		(*pSprite).SetMasked(FALSE);
		(*pSprite).SetMobile(FALSE); 
	
		if ((*pGameInfo).bSoundEffectsEnabled) {
			pEffect = new CSound( (CWnd *)this, ".\\sound\\chrdance.wav",			// Load up the sound file as a 
									SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);				//...Wave file, to delete itself
		}									
		if (pEffect != NULL) {
		   	bSuccess = (*pEffect).Play();
		   	if (!bSuccess)
		   		delete pEffect;
		}
		for( i = 0; i < NUM_STOOL_CELS; i++ ) {
			(*pSprite).PaintSprite( pDC, STOOL_X, STOOL_Y );
			Sleep( STOOL_SLEEP );
		}
		if (pSprite != NULL)
			delete pSprite; 
		
		ReleaseDC(pDC);
	}
	else
	if ( oarRect.PtInRect(myPoint) && (*pGameInfo).bSoundEffectsEnabled ) {
		CSound::waitWaveSounds();
		sndPlaySound( NULL, 0 );
		pEffect = new CSound( (CWnd *)this, OAR_SOUND,								// Load up the sound file as a 
									SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);	//...Wave file, to delete itself
		if (pEffect != NULL) {
		   	bSuccess = (*pEffect).Play();
		   	if (!bSuccess)
		   		delete pEffect;
		}
	}
	else
	if (netRect.PtInRect(myPoint) && (*pGameInfo).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound( NULL, 0 );
		pEffect = new CSound( (CWnd *)this, NET_SOUND,								// Load up the sound file as a 
									SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);	//...Wave file, to delete itself
		if (pEffect != NULL) {
		   	bSuccess = (*pEffect).Play();
		   	if (!bSuccess)
		   		delete pEffect;
		}
	}
	else
	if (signRect.PtInRect(myPoint) && (*pGameInfo).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound( NULL, 0 );
		pEffect = new CSound( (CWnd *)this, SIGN_SOUND,								// Load up the sound file as a 
									SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);	//...Wave file, to delete itself
		if (pEffect != NULL) {
		   	bSuccess = (*pEffect).Play();
		   	if (!bSuccess)
		   		delete pEffect;
		}
	}
	else
	if (tableRect.PtInRect(myPoint) && (*pGameInfo).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound( NULL, 0 );
		pEffect = new CSound( (CWnd *)this, TABLE_SOUND,								// Load up the sound file as a 
									SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);	//...Wave file, to delete itself
		if (pEffect != NULL) {
		   	bSuccess = (*pEffect).Play();
		   	if (!bSuccess)
		   		delete pEffect;
		}
	}
	else
	if ((candlenrRect.PtInRect(myPoint) || candlefrRect.PtInRect(myPoint)) && (*pGameInfo).bSoundEffectsEnabled) {
		CSound::waitWaveSounds();
		sndPlaySound( NULL, 0 );
		pEffect = new CSound( (CWnd *)this, CANDLE_SOUND,								// Load up the sound file as a 
									SOUND_WAVE | SOUND_QUEUE |  SOUND_ASYNCH | SOUND_AUTODELETE);	//...Wave file, to delete itself
		if (pEffect != NULL) {
		   	bSuccess = (*pEffect).Play();
		   	if (!bSuccess)
		   		delete pEffect;
		}
	}
	else
	if (!(counter & 0x01)) {
		pSprite = CSprite::Touched(myPoint);
		if (pSprite != NULL) {
			sprite_loc = (*pSprite).GetPosition();
			grid_loc = PointToGrid(sprite_loc);
		    if (fState [grid_loc.x][grid_loc.y] == PEGGED) {
		    	counter += 1;
		    	Moves [counter][1] = grid_loc.x;
		    	Moves [counter][2] = grid_loc.y;
				pDC = GetDC();
				UpdatePegPosition(pDC,pTableSlot,grid_loc.x,grid_loc.y);
				if (counter % 2)
					(*pCursorSprite).PaintSprite(pDC, sprite_loc);
				ReleaseDC(pDC);
				bPegMoving = TRUE;
		   	}
			else {				 // illegal move
//			    MessageBeep(-1);
				if ((*pGameInfo).bSoundEffectsEnabled)
					sndPlaySound( NULL, 0 );
					sndPlaySound( WAV_NOMOVE, SND_ASYNC );
			}
		}
	}
	else { // this is the second click
		oldx = Moves [counter][1];
		oldy = Moves [counter][2];
		pSprite = CSprite::Touched(myPoint);
		if (pSprite != NULL) {
			sprite_loc = (*pSprite).GetPosition();
			if ((*pSprite).GetTypeCode() == SPRITE_HOLE)
				sprite_loc.x += (SPRITE_SIZE_DX >> 1);	
			grid_loc = PointToGrid(sprite_loc);
			newx = grid_loc.x;
			newy = grid_loc.y;
		    if (fState [newx][newy] == EMPTY) {
				neighborx = ((oldx + newx) / 2);
				neighbory = ((oldy + newy) / 2);
		    	if ((((newx == oldx - 2) && (newy == oldy + 2)) && (BoardSelected == TRIANGLE || BoardSelected == TRIANGLE_PLUS)) ||
		    	     ((newx == oldx)     && (newy == oldy + 2)) ||
		      		 ((newx == oldx + 2) && (newy == oldy))     ||
		    	     ((newx == oldx - 2) && (newy == oldy)) 	||
		    	    (((newx == oldx + 2) && (newy == oldy - 2)) && (BoardSelected == TRIANGLE || BoardSelected == TRIANGLE_PLUS)) ||
		    	     ((newx == oldx)     && (newy == oldy - 2))) {
					if (fState [neighborx][neighbory] == PEGGED) {
		    			fState [oldx][oldy] = EMPTY;
		    			fState [neighborx][neighbory] = EMPTY;
		    			fState [newx][newy] = PEGGED ;
			   			counter += 1;
			   			bPegMoving = FALSE;
		    			Moves [counter][1] = newx ;
		    			Moves [counter][2] = newy ;
						pDC = GetDC();
	
						(*pCursorSprite).EraseSprite(pDC);
	
						UpdatePegPosition(pDC,pShotGlass,newx,newy);
	
						UpdatePegPosition(pDC,pTableSlot,neighborx,neighbory);
	
						ReleaseDC(pDC);
	
						if ((*pGameInfo).bSoundEffectsEnabled) {
							sndPlaySound( NULL, 0 );
							sndPlaySound(WAV_MOVE, SND_ASYNC);
						}
							
						// check for neighbors and hole after neighbors

						for (i=0; i < GRID_SIZE; i++) {
							for (j=0; j < GRID_SIZE; j++) {
								if (fState[i][j] == PEGGED) {
									if (((fState [i+1][j] == PEGGED) && 	(fState [i+2][j] == EMPTY) && 	(i <= 4)) ||
								        ((fState [i-1][j] == PEGGED) && 	(fState [i-2][j] == EMPTY) && 	(i >= 2)) ||
								        ((fState [i][j+1] == PEGGED) &&		(fState [i][j+2] == EMPTY) &&	(j <= 4)) ||
								        ((fState [i-1][j+1] == PEGGED) &&	(fState [i-2][j+2] == EMPTY) &&	(j <= 4) &&	(i >= 2) && (BoardSelected == TRIANGLE || BoardSelected == TRIANGLE_PLUS)) ||
								        ((fState [i+1][j-1] == PEGGED) && 	(fState [i+2][j-2] == EMPTY) && (i <= 4) &&	(j >= 2) && (BoardSelected == TRIANGLE || BoardSelected == TRIANGLE_PLUS)) ||
								        ((fState [i][j-1] == PEGGED) &&		(fState [i][j-2] == EMPTY) &&	(j >= 2))) {
										    moves_left++;
										    }}}}
	
						if (moves_left == 0) {
							// check for end conditions
							// is counter == maxmoves ?   i.e. only one peg left
	
							if (((counter == 62) && (BoardSelected == CROSS)) ||
								((counter == 70) && (BoardSelected == CROSS_PLUS)) ||
								((counter == 26) && (BoardSelected == TRIANGLE)) ||
								((counter == 38) && (BoardSelected == TRIANGLE_PLUS))) {
	
			    				// if so --> is fState [center] == PEGGED?
	
			    				if ((((BoardSelected == CROSS) 		|| (BoardSelected == CROSS_PLUS)) 		&& (fState [3][3] == PEGGED)) ||
			    				 	 ((BoardSelected == TRIANGLE) && (fState [1][1] == PEGGED)) ||
								 	 ((BoardSelected == TRIANGLE_PLUS) && (fState [2][2] == PEGGED))) {
	
				    				// if both TRUE, then WIN!!!!
		                    		score = 25;
									}
									// one left but not in center
								else {
									score = 10;
									}}
			    			// else, just score...  # left = maxchips - ((counter+1)/2) .. counter starts at 0, not 1...
								num_left = (((BoardSelected == CROSS) * 64) +
								    		((BoardSelected == CROSS_PLUS) * 72) +
								    		((BoardSelected == TRIANGLE) * 28) +
								    		((BoardSelected == TRIANGLE_PLUS) * 40) -
								    		  counter ) / 2;
								if (num_left == 2) {
									score = 5;
									}
								else if (num_left == 3) {
									score = 4;
									}
								else if (num_left == 4) {
									score = 3;
								    }
								else if (num_left == 5) {
									score = 2;
									}
								else if ((num_left > 5) && (num_left < 11)) {
									score = 1;
									}
					// display the score
					// display two buttons, Quit or Again?
#ifndef SHOW_CURSOR
					::ShowCursor(TRUE);
#endif

						if (score == 25) {
							if ((*pGameInfo).bSoundEffectsEnabled) {
								sndPlaySound( NULL, 0 );
								sndPlaySound(WAV_WON, SND_ASYNC);
							}
							score_blurb = "You have won!";}
						else {
							if ((*pGameInfo).bSoundEffectsEnabled) {
								sndPlaySound( NULL, 0 );
								sndPlaySound(WAV_DONE, SND_ASYNC);
							}
							score_blurb = "Game over.";}
	
						if (score == 1)
							sprintf(score_string, "Score:  %d point.",score) ;
						else
							sprintf(score_string, "Score:  %d points.",score) ;
	
						CMessageBox GameOverDlg((CWnd *)this, pGamePalette, score_blurb, score_string);
//						UpdateWindow();
//						GameOverDlg.DoModal();
	
						if ((*pGameInfo).bPlayingMetagame) {
#ifndef SHOW_CURSOR
							::ShowCursor(FALSE);
#endif
							(*pGameInfo).lScore += score;	
							PostMessage(WM_CLOSE,0,0);
						}
						else {
							pDC = GetDC();
							SetUpBoard(pDC);
							ReleaseDC(pDC);
							InvalidateRect(NULL,FALSE);
						// if Restart --> IDC_RESTART
//		    		PostMessage(WM_COMMAND, IDC_RESTART, BN_CLICKED);
#ifndef SHOW_CURSOR
		  	  		::ShowCursor(FALSE);
#endif
			    		}}
	
					else {
						 // do nothing, it's not the last move...
						 }}
				else {	// Wrong move, ... must jump over a peg!!!!!
					pDC = GetDC();
					UndoMove(pDC);
		    		ReleaseDC(pDC);
					 }}
				else {
					pDC = GetDC();
					UndoMove(pDC);
		    		ReleaseDC(pDC);
	
					// Wrong move ... must be 2 away
					// restore hole to a peg
					}}
			else {
				pDC = GetDC();
				UndoMove(pDC);
		    	ReleaseDC(pDC);
	
			// Wrong move ... must jump to a hole
			// must restore hole [old] to a peg
			}}
		else {
			counter -= 1;
//			MessageBeep(-1);
			if ((*pGameInfo).bSoundEffectsEnabled) {
				sndPlaySound( NULL, 0 );
				sndPlaySound( WAV_NOMOVE, SND_ASYNC );
			}
			pDC = GetDC();
			(*pCursorSprite).EraseSprite(pDC);
	
			UpdatePegPosition(pDC,pShotGlass,oldx,oldy);
	
			ReleaseDC(pDC);
			}
			// Wrong move ... must jump within the board
		}
}


void CMainWindow::OnLButtonUp(UINT nFlags,CPoint point)
{
	if (bPegMoving)
		OnLButtonDown(nFlags,point);
}


void CMainWindow::OnLButtonDblClk(UINT nFlags,CPoint point)
{
//  insert mouse button processing code here
}


void CMainWindow::OnMButtonDown(UINT nFlags,CPoint point)
{
//  insert mouse button processing code here
}


void CMainWindow::OnMButtonUp(UINT nFlags,CPoint point)
{
//  insert mouse button processing code here
}

void CMainWindow::OnMButtonDblClk(UINT nFlags,CPoint point)
{
//  insert mouse button processing code here
}


void CMainWindow::OnRButtonDown(UINT nFlags,CPoint point)
{
//  insert mouse button processing code here
}


void CMainWindow::OnRButtonUp(UINT nFlags,CPoint point)
{
//  insert mouse button processing code here
}

void CMainWindow::OnRButtonDblClk(UINT nFlags,CPoint point)
{
//  insert mouse button processing code here
}

//////////// Additional Sound Notify routines //////////////

long CMainWindow::OnMCINotify( WPARAM wParam, LPARAM lParam)
{
CSound	*pSound;
	
	pSound = CSound::OnMCIStopped(wParam,lParam);
	if (pSound != NULL)
		OnSoundNotify(pSound);
    return(0L);  
}

	
long CMainWindow::OnMMIONotify( WPARAM wParam, LPARAM lParam)
{
CSound	*pSound;
	
	pSound = CSound::OnMMIOStopped(wParam,lParam);
	if (pSound != NULL)
		OnSoundNotify(pSound);
    return(0L);  
}

void CMainWindow::OnSoundNotify(CSound *pSound)
{
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}



void CMainWindow::UpdatePegPosition(CDC *pDC, CSprite *pBaseSprite, int x, int y)
{
CSprite	*pSprite;
CPoint	sprite_loc,
		hotspot_loc;

sprite_loc = GridToPoint(x,y);
hotspot_loc.x = sprite_loc.x + (SPRITE_SIZE_DX / 2);
hotspot_loc.y = sprite_loc.y + (SPRITE_SIZE_DY / 2);
pSprite = CSprite::Touched(hotspot_loc);
ASSERT(pSprite != NULL);
(*pSprite).EraseSprite(pDC);
(*pSprite).UnlinkSprite();
delete pSprite;
pSprite = (*pBaseSprite).DuplicateSprite(pDC);
ASSERT(pSprite != NULL);
(*pSprite).LinkSprite();
if ((*pSprite).GetTypeCode() == SPRITE_HOLE)
	sprite_loc.x -= (SPRITE_SIZE_DX >> 1);	
(*pSprite).PaintSprite(pDC, sprite_loc.x, sprite_loc.y);
}


void CMainWindow::UndoTurn(void)
{
CDC		*pDC;
int		newx, newy, oldx, oldy,
		neighborx, neighbory;

if (counter > 0) {
	if ((*pGameInfo).bSoundEffectsEnabled) {
		sndPlaySound( NULL, 0 );
		sndPlaySound(WAV_UNDO, SND_ASYNC);
	}
    bPegMoving = FALSE;
	newx = Moves [counter][1];
	newy = Moves [counter][2];
	counter -= 1;
	pDC = GetDC();
	(*pCursorSprite).EraseSprite(pDC);
	oldx = Moves [counter][1];
	oldy = Moves [counter][2];
	neighborx = (oldx + newx) / 2 ;
	neighbory = (oldy + newy) / 2 ;
	fState [neighborx][neighbory] = PEGGED ;
	fState [oldx][oldy] = PEGGED ;
	fState [newx][newy] = EMPTY ;
	counter -= 1;

	UpdatePegPosition(pDC,pShotGlass,neighborx,neighbory);

	UpdatePegPosition(pDC,pShotGlass,oldx,oldy);

	UpdatePegPosition(pDC,pTableSlot,newx,newy);

	ReleaseDC(pDC);
	}
}


void CMainWindow::UndoMove(CDC *pDC)
{
// CSprite	*pSprite;
CPoint	sprite_loc;
int		oldx, oldy;

oldx = Moves[counter][1];
oldy = Moves[counter][2];

counter -= 1;
bPegMoving = FALSE;

//MessageBeep(-1);
if ((*pGameInfo).bSoundEffectsEnabled)  {
	sndPlaySound( NULL, 0 );
	sndPlaySound( WAV_NOMOVE, SND_ASYNC );
}

(*pCursorSprite).EraseSprite(pDC);

UpdatePegPosition(pDC,pShotGlass,oldx,oldy);
}


CPoint CMainWindow::GridToPoint(int i,int j)
{
CPoint	sprite_loc;

if (BoardSelected == CROSS) {
	sprite_loc.x = - (CROSS_SHOTGLASS_DX * j) +
					((CROSS_SHOTGLASS_DDX + (CROSS_SHOTGLASS_DDDX * j)) * i) +
					j - 1 + i;
	sprite_loc.y = (CROSS_SHOTGLASS_DY * (j+1)) + j;
	}
else
if (BoardSelected == CROSS_PLUS) {
	sprite_loc.x = - (CROSS_PLUS_SHOTGLASS_DX * j) +
					((CROSS_PLUS_SHOTGLASS_DDX + (CROSS_PLUS_SHOTGLASS_DDDX * j)) * i) +
					j - 1 + i;
	sprite_loc.y = (CROSS_PLUS_SHOTGLASS_DY * (j+1)) + j;
	}
else
if (BoardSelected == TRIANGLE) {
	sprite_loc.x = (TRI_SHOTGLASS_DX * (i - j));
	sprite_loc.y = (TRI_SHOTGLASS_DY * (i+j+1));
	}
else
if (BoardSelected == TRIANGLE_PLUS)  {
	if ((i + j) == 1) {
		sprite_loc.x = (TRI_PLUS_SHOTGLASS_DX * (i - j));
		sprite_loc.y = TRI_PLUS_SHOTGLASS_DY;
		}
	else
	if ((i + j) == 2) {
		if ((i != 0) && (j != 0)) {
			sprite_loc.x = 0;
			sprite_loc.y = TRI_PLUS_SHOTGLASS_DY * 2;
			}
		}
	else
	if ((i + j) > 2)
		{
		sprite_loc.x = (TRI_PLUS_SHOTGLASS_DX * (i - j));
		sprite_loc.y = (TRI_PLUS_SHOTGLASS_DY * (i + j));
		}
	}

sprite_loc.x += nBoard_DX;
sprite_loc.y += nBoard_DY;

return(sprite_loc);
}


CPoint CMainWindow::PointToGrid(CPoint myPoint)
{
int		x, y;
CPoint	sprite_loc;
CPoint	point;

point.x = myPoint.x - nBoard_DX;
point.y = myPoint.y - nBoard_DY;

if (BoardSelected == CROSS) {
	sprite_loc.y = (point.y - CROSS_SHOTGLASS_DY) /
					(CROSS_SHOTGLASS_DY + 1);
	sprite_loc.x = (point.x + 1 + (sprite_loc.y * (CROSS_SHOTGLASS_DX - 1))) /
					(CROSS_SHOTGLASS_DDX + (sprite_loc.y * CROSS_SHOTGLASS_DDDX) + 1);
	}
else
if (BoardSelected == CROSS_PLUS) {
	sprite_loc.y = (point.y - CROSS_PLUS_SHOTGLASS_DY ) /
					(CROSS_PLUS_SHOTGLASS_DY + 1);
	sprite_loc.x = (point.x + 1 + (sprite_loc.y * (CROSS_PLUS_SHOTGLASS_DX - 1))) /
					(CROSS_PLUS_SHOTGLASS_DDX + (sprite_loc.y * CROSS_PLUS_SHOTGLASS_DDDX) + 1);
	}
else
if (BoardSelected == TRIANGLE) {
	x = (point.x / TRI_SHOTGLASS_DX) ; // x1 = int (i-j)
	y = ((point.y / TRI_SHOTGLASS_DY) - 1) ; // y1 = int(i+j)
	sprite_loc.x = ((x + y) / 2) ;
	sprite_loc.y = ((y - x + 1) / 2) ; // +1 because of the size of the sprite
    }
else { // Triangle Plus case
	y = (point.y / TRI_PLUS_SHOTGLASS_DY) ; // y1 = int(i+j)
	if (y == 1) {
		if ((point.x < TRI_PLUS_SHOTGLASS_DX) && (point.x >= - TRI_PLUS_SHOTGLASS_DX)) {
			sprite_loc.x = 0;
			sprite_loc.y = 1;
			}
		else
		if ((point.x < TRI_PLUS_SHOTGLASS_DX * 2) && (point.x >= TRI_PLUS_SHOTGLASS_DX)) {
			sprite_loc.x = 1;
			sprite_loc.y = 0;
			}
		else {
			sprite_loc.x = 0;
			sprite_loc.y = 0; // error case (didn't click on a shotglass)
			}
		}
	else
	if (y == 2) {
		if ((point.x >= 0) && (point.x <= TRI_PLUS_SHOTGLASS_DX * 2)) {
			sprite_loc.x = 1;
			sprite_loc.y = 1;
			}
		else {
			sprite_loc.x = 0;
			sprite_loc.y = 0; // error case (didn't click on a shotglass)
			}
		}
    else
    if (y > 2) {
		x = (point.x / TRI_PLUS_SHOTGLASS_DX) ; // int(i-j)
		sprite_loc.x = ((x + y) / 2);
		sprite_loc.y = ((y - x + 1) / 2);
		}
	}

return(sprite_loc);
}

void CMainWindow::OnDestroy()
{
//  send a message to the calling app to tell it the user has quit the game
  ::PostMessage( m_hCallAppWnd, WM_PARENTNOTIFY, WM_DESTROY, 0L );
  CFrameWnd::OnDestroy();
} 

// CMainWindow message map:
// Associate messages with member functions.
//
BEGIN_MESSAGE_MAP( CMainWindow, CFrameWnd )
	//{{AFX_MSG_MAP( CMainWindow )
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
	ON_WM_KEYDOWN()
	ON_WM_ACTIVATEAPP()
 	ON_WM_DESTROY()
    ON_MESSAGE(MM_MCINOTIFY, OnMCINotify)
    ON_MESSAGE(MM_WOM_DONE, OnMMIONotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTheApp

// InitInstance:
// When any CTheApp object is created, this member function is automatically
// called.  Any data may be set up at this point.
//
// Also, the main window of the application should be created and shown here.
// Return TRUE if the initialization is successful.
//
/*
BOOL CTheApp::InitInstance()
{
CMainWindow	*pMyMain;

TRACE( "Boffo Games\n" );

SetDialogBkColor();     	// hook gray dialogs (was default in MFC V1)

m_pMainWnd = pMyMain = new CMainWindow();
m_pMainWnd->ShowWindow( m_nCmdShow );

m_pMainWnd->UpdateWindow();

return TRUE;
}

BOOL CTheApp::ExitInstance()
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

return(TRUE);
}
*/

void CMainWindow::OnClose()
{
CDC		*pDC;
CBrush	myBrush;
CRect	myRect;

//	CTheApp::ExitInstance();

	pDC = GetDC();
	myRect.SetRect(0,0,GAME_WIDTH,GAME_HEIGHT);
	myBrush.CreateStockObject(BLACK_BRUSH);
	(*pDC).FillRect(&myRect,&myBrush);
	ReleaseDC(pDC);

	if ((*pGameInfo).bMusicEnabled) {
		CSound::clearSounds();
	}

	CFrameWnd ::OnClose();
}


void setup_cursor(void)
{
HCURSOR	hNewCursor;
CWinApp	*pMyApp;

	pMyApp = AfxGetApp();

	hNewCursor = (*pMyApp).LoadStandardCursor(IDC_ARROW);

	ASSERT(hNewCursor != NULL);
	MFC::SetCursor(hNewCursor);
}


void set_wait_cursor(void)
{
CWinApp	*pMyApp;

	pMyApp = AfxGetApp();

	(void) (*pMyApp).BeginWaitCursor();
}


void reset_wait_cursor(void)
{
CWinApp	*pMyApp;

	pMyApp = AfxGetApp();

	(void) (*pMyApp).EndWaitCursor();
}

   