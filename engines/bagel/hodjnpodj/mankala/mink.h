// mnk.h -- header file for mankala game
// Written by John J. Xenakis, 1994, for Boffo Games

#ifndef __mnk_H__
#define __mnk_H__

#include <time.h>
#include "globals.h"
#include "resource.h"
#include "dibdoc.h"
#include "sprite.h"
#include "bitmaps.h"    
#include <memory.h>
#include "sound.h"

#include "bgen.h"
#include "bgenut.h"
#include "btimeut.h"
#include "mnkopt.h"

#include "hhead.h"

// DATADIR is directory for *.BMP and other data files
// #define DATADIR ".".\\"
#define DATADIR "art\\"
// const XPSTR MNKSCREEN = DATADIR "mancrab.bmp" ;	// bitmap file for screen

const int MAXMOVES = 100 ;	// max # moves is 100
const int NUMPITS = 6 ;		// # pits each player has
const int NUMPLAYERS = 2 ;	// # players
const int TOTALPITS = NUMPLAYERS * NUMPITS ; // total # pits
					// (not including home bins)
const int HANDINDEX = -2, HOMEINDEX = -1 ;

const int MAXDISPPIT = 8, MAXDISPHOME = 22, MAXDISPHAND = MAXPOSINT ;
	// max # of stones that can be displayed discretely

// values in user dialogs
//const int MINSTONES = 1, MAXSTONES = 12 ;
//const int MINSTRENGTH = 1, MAXSTRENGTH = 5 ;
const int MINSTONES = 3, MAXSTONES = 7 ;
const int MINSTRENGTH = 1, MAXSTRENGTH = 4 ;


const int MAXTABLESTONES = 12 ;	// configurations in best win table
const ULONG MAXCONFIGS = 3000000L ;	// # configurations in table


const int BESTWINBITS = 5 ;	// # bits per best win table entry
const int TABLEUNDEF = (-16+32) ;  // value of undefined table entry
const int BESTWINUNDEF = -1000 ;  // value of undefined minimax value

// bitmap object types
#define BMT_MAIN 200
#define BMT_STONE 201
#define BMT_PIT 202
#define BMT_LEFTBIN 203
#define BMT_RIGHTBIN 204
#define BMT_SIGN 205
#define BMT_HAND 206
#define BMT_SCROLL 207

// sign bitmap numbers
#define SBT_CRAB 1
#define SBT_MYTURN 2
#define SBT_YOURTURN 3
#define SBT_TIE 4
#define SBT_IWIN 5
#define SBT_YOUWIN 6                            

#define	IDC_SCROLL		419         

#define RULES 	".\\mankala.txt"

#define RULES_NARRATION		".\\sound\\mankala.wav"
#define IGO3				".\\sound\\igo3.wav"
#define YOUGO3				".\\sound\\yougo3.wav"
#define GOAGAIN				".\\sound\\goagain.wav"
#define IGOAGAIN			".\\sound\\igoagain.wav"
#define PICKUP				".\\sound\\pickup.wav"
#define ONESHELL			".\\sound\\oneshell.wav"
#define INK	".\\sound\\ink.wav"
#define BROCHURE	".\\sound\\brochure.wav"
#define ARMCHAIR	".\\sound\\armchair.wav"
#define FISHHOOK	".\\sound\\fishhook.wav"
#define PAILS	".\\sound\\pails.wav"
#define NETSHELL	".\\sound\\netshell.wav"
#define WHEEL	".\\sound\\wheel.wav"
#define HONK	".\\sound\\honk.wav"
#define POP	".\\sound\\pop.wav"
#define INFLATE	".\\sound\\inflate.wav"
#define SOSORRY	".\\sound\\sosorry.wav"
#define FANFARE2	".\\sound\\fanfare2.wav"
#define MIDI_BCKGND ".\\sound\\mankala.mid"   
#define GLOBE_SPRITE ".\\art\\globe2.bmp"
#define CHAIR_SPRITE ".\\art\\chair.bmp"


//inline int OTHERPLAYER(int iPlayer) {return(NUMPLAYERS - 1 - iPlayer) ; }

#define OTHERPLAYER(iPlayer) (NUMPLAYERS - 1 - iPlayer)
#define MAXRECURSION 3
#define MAXMOVES			25

typedef struct { 
	int iFree[NUMPITS][MAXRECURSION]; 
	int iCapture[NUMPITS][MAXRECURSION];
	int iRocksInHomeBin;
	int iPitSequence[MAXRECURSION];
	int iNumRocks[NUMPITS];  
	int iNumRocksOpposite[NUMPITS];
	signed char cRecursion;
	int iParentID;
	signed char cMoveID;  
	int iMoveID;  
	
}MOVE;

typedef MOVE* PMOVE;
typedef MOVE _far* LPMOVE;
typedef MOVE _near* NPMOVE;


// CBmpTable -- table of bitmap filenames
class CBmpTable {
public:
    int m_iBmpType ;		// BMT_xxxx
    int m_iNumStones ;		// 0 means "many"
    int m_iNumBmps ;		// number of bmp files for this combo
    char * m_xpszFilenameString ;	// sprintf string
    BOOL m_bSubNumStones ;
    BOOL m_bMasked ;		// mask white areas of bitmap
} ;

// CBmpObject -- bitmap object
class CBmpObject {
    friend class CMnk ;
    friend class CMnkWindow ;
    char m_cStartData ;
    CBmpObject * m_xpcNextFree ; // pointer to next in chain to free
    BOOL m_bChained, m_bHeap ;	// on NextFree chain / on heap
    CBmpObject * m_xpcNext ;	// pointer to next in chain
    BOOL m_bDummy ;		// dummy object -- no bitmap
    BOOL m_bMasked ;		// mask white areas of bitmap
    class CPitWnd * m_xpcPit ;	// ptr to pit, for pit/bin types
    BOOL m_bInit ;		// flag: object is initialized
    int m_iBmpType ;		// BMT_xxxx -- bitmap type
    int m_iBmpArg ;
    int m_iBmpNum ;
    int m_iStoneNum ;		// for type BMT_STONE
    BOOL m_bSprite ;		// flag: use sprite, not DIB
    class CSprite * m_xpcSprite ;  // if this is a sprite, store path
				// there rather than in DIB
    CDibDoc * m_xpDibDoc ;	// DIB document
    LPVOID m_lpDib ;		// pointer to bitmap memory
    CPoint m_cPosition ;	// position to paint bitmap
    CSize m_cSize ;		// size of bitmap
    char m_cEndData ;

public:
    CBmpObject() {memset(&m_cStartData, 0,
	    			&m_cEndData - &m_cStartData) ;}
} ;

// CMove -- position/move class
class CMove {
    friend class CMnk ;
    friend class CMnkWindow ;
    char m_cStartData ;
    BOOL m_bRealMove ;	// this is CMove object for real board position 
	 int m_iNumStones[NUMPLAYERS][NUMPITS+2] ;	// number of stones
		// in each pit, prior to sowing
    int m_iPlayer ;		// player on the move
	int m_iTotalStones ;	// total stones in pits (not home bins)
    long m_lConfigIndex ;	// configuration index for position
    int m_iValues[NUMPITS] ; // value of each move
    int m_iBestWinValue ;	// best value found
    int m_iBestMove ;		// best move
    class CPit * m_xpcPit ;	// ptr to pit being sowed
    BOOL m_bFreeTurn ;	// player got free turn
    BOOL m_bCapture ;	// capture occurred
    int m_iCapturePit ;	// # of opponent's pit that got captured
    BOOL m_bHasCapture ;	// has a capture from here
    BOOL m_bHasFree ;		// has free move from here
    int m_iNumMoves ;		// number of legal moves
    char m_cEndData ;

    // methods to zero or copy CMove objects
    void Zero(void)
	{memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;}
    void Copy(CMove * xpcMove) {memcpy(&m_cStartData, 
	&xpcMove->m_cStartData, &m_cEndData - &m_cStartData) ;
    		m_bRealMove = FALSE ;}                                   
    
    public:
    CMove::CMove() {Zero() ;}	// constructor
	#ifdef _DEBUG
		BOOL SetBackToOriginal(HWND);		//this is PUBLIC
		void RearrangeConfiguration(HWND, int, long, BOOL);	//THIS IS PUBLIC
		void ReConfig(HWND,  long);	//THIS IS PUBLIC
		inline void SetPlayer(int);				//THIS IS public
	#endif	
		
} ;

// CPit -- class for pit (as well as home bin and hand)
class CPit {
    friend class CMnk ;    
public:
    char m_cStartData ;
    int m_iNumStones, m_iDispStones, m_iDispMax ;
    int m_iPlayer, m_iPit ;	// player #, pit #

    char m_cEndData ;

    // constructor zeroes out all fields
    CPit() {memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;}

} ;	// CPit

// CMnk -- Mankala game class
class CMnk {
    friend class CMnkWindow ;
    CGenUtil m_cGenUtil ;		// general utility object
    CTimeUtil m_cTimeUtil ;		// time utility object
    char m_cStartData ;
    BOOL m_bStartGame ;		// just starting game
    BOOL m_bGameOver ;		// game is over
    BOOL m_bGameOverMusicPlayed ;		// music played when game is over
    int m_iTableStones ;	// number of stones in data table
    long m_lNumConfigs ;	// # configs in best win table
    long m_lTableSize ;		// size of best win table
    BOOL m_bInitData ;		// flag: initialize data tables
    class CMnkData FAR * m_lpCMnkData ;	// pointer to data tables      
    CSound* m_pSound;

//    XPVOID m_cWnd ;	// mankala window
//    int m_iPlayer ;	// Next player: 0=player 1, 1=player 2
    int m_iStartStones ;	// #stones per pit at start
    BOOL m_bComputer[NUMPLAYERS] ; // flag: player is
					// computer (vs human)
    class CPit * m_xpcPits[NUMPLAYERS][NUMPITS+2] ;
		// pointers to pit classes for pits, home bins,
    		// and hands
//    int m_iHomeStones[NUMPLAYERS] ;	// #stones in each home bin
//    int m_iPitStones[NUMPLAYERS][NUMPITS] ;	// #stones in each pit
    int m_iMaxDepth[NUMPLAYERS], m_iCurrentMaxDepth ;
					// max minimax depth
    int m_iCapDepth[NUMPLAYERS], m_iCurrentCapDepth ;
					// minimax capture depth
    int m_iNumberMoves ;	// #moves played so far
    class CMove m_cCurrentMove ;	// current position/move
    class CMove m_cMoveList[MAXMOVES] ;	// move list
    enum enum_Level{LEV_NONE, LEV_RANDOM,
    		LEV_LOWEST, LEV_HIGHEST, LEV_EVAL, LEV_MINIMAX}
	m_eLevel[NUMPLAYERS]  ;		// level of computer play

    BOOL m_bDumpPopulate, m_bDumpMoves, m_bDumpTree ; // dump options

    char m_cEndData ;


// methods
    CMnk::CMnk() {memset((char _huge*)&m_cStartData, 0,
				(size_t)((char _huge*)&m_cEndData - (char _huge*)&m_cStartData)) ;}

// mnklog.cpp -- Mankala game logic

//- CMnk::InitMankala -- initialize a new game of Mankala
PRIVATE BOOL CMnk::InitMankala(void) ;
//- CMnk::Move -- make a move
PRIVATE BOOL CMnk::Move(CPit * xpcSowPit, CMove * xpcMove PDFT(NULL)) ;
//- CMnk::MoveStone -- move one stone for move
PUBLIC BOOL CMnk::MoveStone(CMove * xpcMove,
	CPit * xpcFromPit, CPit * xpcToPit) ;
//- CMnk::InitData -- initialize data class object
PRIVATE BOOL CMnk::InitData(BOOL bInit PDFT(TRUE)) ;
//- CMnk::CountConfigurations -- set up Configurations table
BOOL CMnk::CountConfigurations(void) ;
//- CMnk::PopulateTable -- compute values for best win table
PRIVATE BOOL CMnk::PopulateTable(void) ;
//- CMnk::WriteTableFile -- write out file with best win table
PRIVATE BOOL CMnk::WriteTableFile(void) ;
//- CMnk::ReadTableFile -- read file with best win table
PRIVATE BOOL CMnk::ReadTableFile(void) ;
//- CMnk::MapConfiguration -- map a configuration to its integer index,
//		store configuration index into Move object
BOOL CMnk::MapConfiguration(CMove * xpcMove) ;
//- CMnk::UnmapConfiguration -- map configuration index back
//				to configuration
BOOL CMnk::UnmapConfiguration(CMove * xpcMove) ;
//- CMnk::SearchMove -- search for best move
PRIVATE BOOL CMnk::SearchMove(CMove * xpcMove, int& iMove) ;
//- CMnk::Minimax -- find best move from supplied configuration
BOOL CMnk::Minimax(CMove * xpcMove, int iDepth PDFT(0)) ;
//- CMnk::StaticEvaluation -- determine static value of a position
BOOL CMnk::StaticEvaluation(CMove * xpcMove) ;
//- CMnk::CountStones -- count total stones in configuration
BOOL CMnk::CountStones(CMove * xpcMove) ;
//- CMnk::GetBestWinCount -- get position value in best win table
PRIVATE BOOL CMnk::GetBestWinCount(CMove * xpcMove) ;
//- CMnk::SetBestWinCount -- set value in best win table
PRIVATE BOOL CMnk::SetBestWinCount(CMove * xpcMove) ;
//- CMnk::DumpPosition -- dump contents of CMove object
BOOL CMnk::DumpPosition(CMove * xpcMove) ;
//- CMnk::DumpBestWinTable -- dump fields of best win table
BOOL CMnk::DumpBestWinTable(long lLow PDFT(0),
				long lHigh PDFT(6*24-1)) ;       
BOOL CMnk::AggressiveStaticEvaluation(CMove* xpcMove);

BOOL CMnk::DefensiveStaticEvaluation(CMove*);
BOOL CMnk::TreeAlgo(CMove*);
int CMnk::ExtendedStaticEvaluation(MOVE*, MOVE*, signed char, int);

} ;	// CMnk

// FIVE structure -- used for addressing values in best win array
#pragma pack(1)		// byte alignment
struct FIVE {
    unsigned char   v0:5,		// 0:0-4
		    v1a:3, v1b:2,	// 0:5-7, 1:0-1
		    v2:5,		// 1:2-6
		    v3a:1, v3b:4, 	// 1:7, 2:0-3
		    v4a:4, v4b:1,	// 2:4-7, 3:0
		    v5:5,		// 3:1-5
		    v6a:2, v6b:3,	// 3:6-7, 4:0-2
		    v7:5 ;		// 4:3-7
//  unsigned int v0:5, v1:5, v2:5, v3:5, v4:5, v5:5, v6:5, v7:5 ;
} ;
#pragma pack()		// revert to compiler default

// CFileHeader -- file header for data file
class CFileHeader {
    friend class CMnk ;

    char m_szText[80] ;		// descriptive text
    int m_iHeaderSize ;		// size of header (# bytes)
    int m_iVersion ;		// version number
    int m_iTableStones ;	// # stones in stored best win table
    long m_lTableSize ;	// length of stored best win table
} ;

// class CMnkData -- mankala data - this class used for move analysis
typedef class FAR CMnkData {
    friend class CMnk ;
//    friend class CMnkWindow ;

    char m_cStartData ;
    CFileHeader m_cFileHeader ;	// file header for data file
    long m_NX[MAXTABLESTONES+1][TOTALPITS+1],
			m_NA[MAXTABLESTONES+1][TOTALPITS+1] ;
	    // NX[s,p] contains the number of arrangements of
	    // exactly s stones into p pits.  NA[s,p] contains the
	    // number of arrangements of s or fewer stones into
	    // p pits.

    HGLOBAL m_hBestWin ;	// Windows handle for best win table
    BYTE HUGE * m_hpcBestWin ;	// pointer to array
	    // of 5 bit values.  The subscripts are integers
	    // 0 <= subscript <= NUMCONFIGS, and the array element
	    // specifies the number of stones (-15 to +15, with -16
	    // representing an unknown value) that can be won from
	    // this configuration with best play.

    char m_cEndData ;
    CMnkData::CMnkData()
	{_fmemset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;}
} class_CMnkData, HUGE * HPCMNKDATA ;

#define	SPRITE_COUNT	15	// maximum sprites to be displayed
#define BUMPER_COUNT	5	// number of bumper sprites to display
#define	SPRITE_TIMER	50	// identifier for sprite timer 
#define	SPRITE_INTERVAL	10      // time interval between sprite updates
#define SPRITE_GENERATE	30    // number of updates per sprite creation

// Button Identifier codes (BIDs)
#define	IDC_QUIT	500
#define IDC_OKAY	501

// Button positioning constants
#define QUIT_BUTTON_WIDTH		50
#define QUIT_BUTTON_HEIGHT		20
#define QUIT_BUTTON_OFFSET_X	10
#define QUIT_BUTTON_OFFSET_Y	10

#define OKAY_BUTTON_WIDTH		63
#define OKAY_BUTTON_HEIGHT		39
#define OKAY_BUTTON_OFFSET_X	10
#define OKAY_BUTTON_OFFSET_Y	10


// CPitWnd -- display class for pit (as well as home bin and hand)
class CPitWnd : public CPit {
    friend class CMnkWindow ;
    char m_cStartData2 ;
    CBmpObject * m_xpcStoneChain ;  // chain header for stone displays
//    CPoint m_cStonePosition ;	// position of first stone
//    int m_iNumStones ;		// number of stones in pit
    char m_cEndData2 ;
    CBmpObject m_cBmpObject ;	// current bitmap object for pit with stones

    CPitWnd() {memset((char _huge*)&m_cStartData2, 0,
			(size_t)((char _huge*)&m_cEndData2 - (char _huge*)&m_cStartData2)) ;}

} ;


/////////////////////////////////////////////////////////

// CMnkWindow:

class CMnkWindow : public CFrameWnd, public CMnk
{
    char m_cStartData2 ;
    BOOL m_bJustStarted;	// flag to indicate beginning of game.	
	BOOL m_bGameWon;       //flag to indicate game result.
	BOOL m_bRulesActive;	//flag to indicate the unfurled status of the rules scroll.
	                                              // ...this flag is set only when rules are invoked via the F1 key.
	BOOL m_bScoresDisplayed; // to indicate that the Score has been displayed at the end of game,
												//... and should pop up again.	                                              
	CText* m_pText;
	
    CBmpObject m_cBmpMain ;	// bitmap object for main window
    CPalette *m_xpGamePalette, *m_xpOldPalette ;  // game pallet
    CBmpObject * m_xpFreeStoneChain ;    // chain of unused stone sprites
    CBmpObject * m_xpcBmpFreeChain ;	// chain of objects to be freed
//    CSprite * m_xpStoneSprite ;	// basic stone sprite (others dup'ed)
    CBmpObject m_cBmpScroll ;		// options scroll bitmap object
    BOOL m_bInMenu ;		// within options menu
	BOOL	m_bPlaySound;       
    char m_szDataDirectory[100] ;	// directory for data files

    CRect m_cMainRect,   // screen area spanned by the game window
	  m_cQuitRect,   // window area spanned by the QUIT button
	  m_cOkayRect ;  // window area spanned by the OKAY button

//    // pointers to chain of stone sprites for each pit/home/hand
//    CStone * m_xpcHomeStones[NUMPLAYERS] ;	// bin stones
//    CStone * m_xpcPitStones[NUMPLAYERS][NUMPITS] ;	// pit stones
//    CStone * m_xpcHandStones[NUMPLAYERS] ;	// stones in hand
    class CPitWnd * m_xpcPits[NUMPLAYERS][NUMPITS+2] ;
		// pointers to pit classes for pits, home bins,
    		// and hands
    CBmpObject m_cBmpCrab ;	// bitmap object for crab with no sign
    CBmpObject m_cBmpSign ;	// bitmap object for crab sign
    int m_iBmpSign ;		// SBT_xxxx -- which sign crab displays
    char m_cEndData2 ;


// mnk.cpp -- Mankala game -- Windows interface

//- CMnkWindow::CMnkWindow() -- mankala window constructor function
PUBLIC CMnkWindow::CMnkWindow(void) ;
//- CMnkWindow::~CMnkWindow -- Mankala window destructor function
PUBLIC CMnkWindow::~CMnkWindow(void) ;


// mnkui.cpp -- Mankala game -- user interface

//- CMnkWindow::StartGame -- start a new game
PRIVATE BOOL CMnkWindow::StartGame(void) ;
//- CMnkWindow::PaintBitmapObject -- paint bitmap
PRIVATE BOOL CMnkWindow::PaintBitmapObject(CBmpObject * xpcBmpObject,
		int iBmpType PDFT(0), int iBmpArg PDFT(0)) ;
//- CMnkWindow::InitBitmapObject -- set up DibDoc in bitmap object
PRIVATE BOOL CMnkWindow::InitBitmapObject(CBmpObject * xpcBmpObject) ;
//- CMnkWindow::InitBitmapFilename -- set up filename bitmap object
PRIVATE BOOL CMnkWindow::InitBitmapFilename(CBmpObject * xpcBmpObject) ;
//- CMnkWindow::SetBitmapCoordinates -- set coordinates of bitmap
PRIVATE BOOL CMnkWindow::SetBitmapCoordinates(
				CBmpObject * xpcBmpObject) ;
//- CMnkWindow::AcceptClick -- process a mouse click by user
BOOL CMnkWindow::AcceptClick(CPoint cClickPoint) ;
//- CMnkWindow::MoveStoneDisplay -- move a stone from pit to another
PUBLIC BOOL CMnkWindow::MoveStoneDisplay(CPitWnd * xpcFromPit,
		CPitWnd * xpcToPit) ;
//- CMnkWindow::AdjustPitDisplay -- adjust display of pit when
//			number of stones changes
PRIVATE BOOL CMnkWindow::AdjustPitDisplay(CPitWnd * xpcPit,
				BOOL bForcePaint PDFT(FALSE)) ;
//- CMnkWindow::PaintScreen -- paint screen for mankala game
PRIVATE VOID CMnkWindow::PaintScreen(void) ;
//- CMnkWindow::ProcessDc -- handle processing of device context
PRIVATE BOOL CMnkWindow::ProcessDc(BOOL bAlloc PDFT(TRUE)) ;
//- CMnkWindow::AllocatePits -- allocate pits (including home bin/hand)
BOOL CMnkWindow::AllocatePits(void) ;
//- CMnkWindow::SetCrabSign -- to my/your turn
PRIVATE BOOL CMnkWindow::SetCrabSign(BOOL bPaint PDFT(TRUE)) ;
//- CMnkWindow::FreePitResources -- free (optionally delete) all pit
//		resources -- stone sprites and pit bitmaps
PRIVATE BOOL CMnkWindow::FreePitResources(BOOL bDelete PDFT(FALSE)) ;
//- CMnkWindow::ClearBitmapObject -- release bitmap object
PRIVATE BOOL CMnkWindow::ClearBitmapObject(CBmpObject * xpcBmpObject) ;
//- CMnkWindow::ReleaseResources -- release all resources before term
PUBLIC void CMnkWindow::ReleaseResources(void) ;
//- CMnkWindow::DebugDialog -- put up debugging dialog box
PRIVATE BOOL CMnkWindow::DebugDialog(void) ;
PUBLIC void Setm_bJustStarted(BOOL U){
	m_bJustStarted=U;//- CMnkWindow::UserDialog -- put up user dialog box
}
PUBLIC BOOL FAR PASCAL CMnkWindow::UserDialog(void) ;
//- CMnkWindow::OptionsDialog -- call options dialog
PRIVATE BOOL CMnkWindow::OptionsDialog(void) ;

//PRIVATE BOOL CALLBACK CMnkWindow::ResetPitsDlgProc(HWND, UINT, WPARAM, LPARAM);



//    CMnkWindow();
//
//
//    void SplashScreen();
//
//    static	void ReleaseResources(void);
//    static	void FlushInputEvents(void);
//
//private:
//public:
//    static	void BuildSprites(CDC *pDC);
//    static	void ProcessSprites(CDC *pDC);
//    static	void MoveSprite(CDC *pDC,CSprite *pSprite);

protected:
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual long OnMCINotify(WPARAM W, LPARAM L);
    virtual long OnMMIONotify(WPARAM W, LPARAM L);

    //{{AFX_MSG( CMnkWindow )
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT,CPoint);
    afx_msg void OnLButtonUp(UINT,CPoint);
    afx_msg void OnLButtonDblClk(UINT,CPoint);
    afx_msg void OnMButtonDown(UINT,CPoint);
    afx_msg void OnMButtonUp(UINT,CPoint);
    afx_msg void OnMButtonDblClk(UINT,CPoint);
    afx_msg void OnRButtonDown(UINT,CPoint);
    afx_msg void OnRButtonUp(UINT,CPoint);
    afx_msg void OnRButtonDblClk(UINT,CPoint);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnClose();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
} ;

///////////////////////////////////////////////////////

// CMnkApp:
//
class CMnkApp : public CWinApp
{
    CMnkWindow * m_xpcMnkWindow ;
public:
    BOOL InitInstance();
    int ExitInstance();
} ;

///////////////////////////////////////////////

#include "htail.h"

#endif // __mnk_H__

