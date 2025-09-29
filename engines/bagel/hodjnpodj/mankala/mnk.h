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

#ifndef HODJNPODJ_MANKALA_MNK_H
#define HODJNPODJ_MANKALA_MNK_H

#include "common/serializer.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/mankala/bgen.h"
#include "bagel/hodjnpodj/mankala/bgenut.h"
#include "bagel/hodjnpodj/mankala/btimeut.h"
#include "bagel/hodjnpodj/mankala/mnkopt.h"
#include "bagel/hodjnpodj/mankala/resource.h"

namespace Bagel {
namespace HodjNPodj {
namespace Mankala {



// DATADIR is directory for *.BMP and other data files
// #define DATADIR ".".\\"
#define DATADIR "art\\"
// const XPSTR MNKSCREEN = DATADIR "mancrab.bmp" ;  // bitmap file for screen

const int MAXMOVES = 100 ;  // max # moves is 100
const int NUMPITS = 6 ;     // # pits each player has
const int NUMPLAYERS = 2 ;  // # players
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


const int MAXTABLESTONES = 12 ; // configurations in best win table
const unsigned long MAXCONFIGS = 3000000L ; // # configurations in table


const int BESTWINBITS = 5 ; // # bits per best win table entry
const int TABLEUNDEF = (-16 + 32) ; // value of undefined table entry
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

#define IDC_SCROLL      419

#define RULES   ".\\mankala.txt"

#define RULES_NARRATION     ".\\sound\\mankala.wav"
#define IGO3                ".\\sound\\igo3.wav"
#define YOUGO3              ".\\sound\\yougo3.wav"
#define GOAGAIN             ".\\sound\\goagain.wav"
#define IGOAGAIN            ".\\sound\\igoagain.wav"
#define PICKUP              ".\\sound\\pickup.wav"
#define ONESHELL            ".\\sound\\oneshell.wav"
#define INK ".\\sound\\ink.wav"
#define BROCHURE    ".\\sound\\brochure.wav"
#define ARMCHAIR    ".\\sound\\armchair.wav"
#define FISHHOOK    ".\\sound\\fishhook.wav"
#define PAILS   ".\\sound\\pails.wav"
#define NETSHELL    ".\\sound\\netshell.wav"
#define WHEEL   ".\\sound\\wheel.wav"
#define HONK    ".\\sound\\honk.wav"
#define POP ".\\sound\\pop.wav"
#define INFLATE ".\\sound\\inflate.wav"
#define SOSORRY ".\\sound\\sosorry.wav"
#define FANFARE2    ".\\sound\\fanfare2.wav"
#define MIDI_BCKGND ".\\sound\\mankala.mid"
#define GLOBE_SPRITE ".\\art\\globe2.bmp"
#define CHAIR_SPRITE ".\\art\\chair.bmp"


//inline int OTHERPLAYER(int iPlayer) {return(NUMPLAYERS - 1 - iPlayer) ; }

#define OTHERPLAYER(iPlayer) (NUMPLAYERS - 1 - iPlayer)
#define MAXRECURSION 3
#define MAXMOVES            25

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

} MOVE, *PMOVE, *LPMOVE, *NPMOVE;

// CBmpTable -- table of bitmap filenames
class CBmpTable {
public:
	int m_iBmpType ;        // BMT_xxxx
	int m_iNumStones ;      // 0 means "many"
	int m_iNumBmps ;        // number of bmp files for this combo
	const char *m_xpszFilenameString ;  // sprintf string
	bool m_bSubNumStones ;
	bool m_bMasked ;        // mask white areas of bitmap
} ;

// CBmpObject -- bitmap object
class CBmpObject {
	friend class CMnk ;
	friend class CMnkWindow ;

	CBmpObject *m_xpcNextFree = nullptr;  // pointer to next in chain to free
	bool m_bChained = false, m_bHeap = false;  // on NextFree chain / on heap
	CBmpObject *m_xpcNext = nullptr;	// pointer to next in chain
	bool m_bDummy = false;				// dummy object -- no bitmap
	bool m_bMasked = false;				// mask white areas of bitmap
	class CPitWnd *m_xpcPit = nullptr;	// ptr to pit, for pit/bin types
	bool m_bInit = false;				// flag: object is initialized
	int m_iBmpType = 0;					// BMT_xxxx -- bitmap type
	int m_iBmpArg = 0;
	int m_iBmpNum = 0;
	int m_iStoneNum = 0;				// for type BMT_STONE
	bool m_bSprite = false;				// flag: use sprite, not DIB
	CSprite *m_xpcSprite = nullptr;		// if this is a sprite, store path
										// there rather than in DIB
	CDibDoc *m_xpDibDoc = nullptr;		// DIB document
	HDIB m_lpDib = nullptr;				// pointer to bitmap memory
	CPoint m_cPosition;					// position to paint bitmap
	CSize m_cSize;						// size of bitmap
} ;

// CMove -- position/move class
class CMove {
	friend class CMnk ;
	friend class CMnkWindow ;

	bool m_bRealMove = false;  // this is CMove object for real board position
	// number of stones in each pit, prior to sowing
	int m_iNumStones[NUMPLAYERS][NUMPITS + 2] = {};
	int m_iPlayer = 0;				// player on the move
	int m_iTotalStones = 0;			// total stones in pits (not home bins)
	long m_lConfigIndex = 0;		// configuration index for position
	int m_iValues[NUMPITS] = {};	// value of each move
	int m_iBestWinValue = 0;		// best value found
	int m_iBestMove = 0;			// best move
	class CPit *m_xpcPit = nullptr;	// ptr to pit being sowed
	bool m_bFreeTurn = false;		// player got free turn
	bool m_bCapture = false;		// capture occurred
	int m_iCapturePit = 0;			// # of opponent's pit that got captured
	bool m_bHasCapture = false;		// has a capture from here
	bool m_bHasFree = false;		// has free move from here
	int m_iNumMoves = 0;			// number of legal moves

	// methods to zero or copy CMove objects
	void Zero() {
		m_bRealMove = false;
		Common::fill(&m_iNumStones[0][0], &m_iNumStones[0][0] + NUMPLAYERS * (NUMPITS + 2), 0);
		m_iPlayer = 0;
		m_iTotalStones = 0;
		m_lConfigIndex = 0;
		Common::fill(m_iValues, m_iValues + NUMPITS, 0);
		m_iBestWinValue = 0;
		m_iBestMove = 0;
		m_xpcPit = nullptr;
		m_bFreeTurn = false;
		m_bCapture = false;
		m_iCapturePit = 0;
		m_bHasCapture = false;
		m_bHasFree = false;
		m_iNumMoves = 0;
	}
	void Copy(CMove *xpcMove) {
		*this = *xpcMove;
		m_bRealMove = false;
	}
} ;

// CPit -- class for pit (as well as home bin and hand)
class CPit {
	friend class CMnk ;
public:
	int m_iNumStones = 0, m_iDispStones = 0, m_iDispMax = 0;
	int m_iPlayer = 0, m_iPit = 0; // player #, pit #
}; // CPit

// CMnk -- Mankala game class
class CMnk {
private:
	friend class CMnkWindow ;
	CGenUtil m_cGenUtil ;					// general utility object
	CTimeUtil m_cTimeUtil ;					// time utility object

	bool m_bStartGame = false;				// just starting game
	bool m_bGameOver = false;				// game is over
	bool m_bGameOverMusicPlayed = false;	// music played when game is over
	int m_iTableStones = 0;					// number of stones in data table
	long m_lNumConfigs = 0;					// # configs in best win table
	long m_lTableSize = 0;					// size of best win table
	bool m_bInitData = false;				// flag: initialize data tables
	class CMnkData FAR *m_lpCMnkData = nullptr;	// pointer to data tables
	CSound *m_pSound = nullptr;

//    XPVOID m_cWnd ;   // mankala window
//    int m_iPlayer ;   // Next player: 0=player 1, 1=player 2
	int m_iStartStones = 0;    // #stones per pit at start
	bool m_bComputer[NUMPLAYERS] = {}; // flag: player is
	// computer (vs human)
	class CPit *m_xpcPits[NUMPLAYERS][NUMPITS + 2] = {};

	int m_iMaxDepth[NUMPLAYERS] = {}, m_iCurrentMaxDepth = 0;
	// max minimax depth
	int m_iCapDepth[NUMPLAYERS] = {}, m_iCurrentCapDepth = 0;
	// minimax capture depth
	int m_iNumberMoves = 0;						// #moves played so far
	class CMove m_cCurrentMove;					// current position/move
	class CMove m_cMoveList[MAXMOVES];			// move list
	enum enum_Level {LEV_NONE, LEV_RANDOM,
	                 LEV_LOWEST, LEV_HIGHEST, LEV_EVAL, LEV_MINIMAX
	                }
	m_eLevel[NUMPLAYERS] = {};					// level of computer play

	bool m_bDumpPopulate = false,
		m_bDumpMoves = false,
		m_bDumpTree = false;					// dump options

private:
	/**
	 * Read save file with best win table
	 * @returns		true if error, false otherwise
	 */
	bool ReadTableFile();

	/**
	 * Write out save file with best win table
	 * @returns		true if error, false otherwise
	 */
	bool WriteTableFile();

public:
// mnklog.cpp -- Mankala game logic

//- InitMankala -- initialize a new game of Mankala
private: bool InitMankala() ;
//- Move -- make a move
private: bool Move(CPit * xpcSowPit, CMove * xpcMove PDFT(nullptr)) ;
//- MoveStone -- move one stone for move
public: bool MoveStone(CMove * xpcMove,
	                       CPit * xpcFromPit, CPit * xpcToPit) ;
//- InitData -- initialize data class object
private: bool InitData(bool bInit PDFT(true)) ;
//- CountConfigurations -- set up Configurations table
	bool CountConfigurations() ;
//- PopulateTable -- compute values for best win table
private: bool PopulateTable() ;
//- MapConfiguration -- map a configuration to its integer index,
//		store configuration index into Move object
	bool MapConfiguration(CMove * xpcMove) ;
//- UnmapConfiguration -- map configuration index back
//				to configuration
	bool UnmapConfiguration(CMove * xpcMove) ;
//- SearchMove -- search for best move
private: bool SearchMove(CMove * xpcMove, int &iMove) ;
//- Minimax -- find best move from supplied configuration
	bool Minimax(CMove * xpcMove, int iDepth PDFT(0)) ;
//- StaticEvaluation -- determine static value of a position
	bool StaticEvaluation(CMove * xpcMove) ;
//- CountStones -- count total stones in configuration
	bool CountStones(CMove * xpcMove) ;
//- GetBestWinCount -- get position value in best win table
private: bool GetBestWinCount(CMove * xpcMove) ;
//- SetBestWinCount -- set value in best win table
private: bool SetBestWinCount(CMove * xpcMove) ;
//- DumpPosition -- dump contents of CMove object
	bool DumpPosition(CMove * xpcMove) ;
//- DumpBestWinTable -- dump fields of best win table
	bool DumpBestWinTable(long lLow PDFT(0),
	                      long lHigh PDFT(6 * 24 - 1)) ;
	bool AggressiveStaticEvaluation(CMove* xpcMove);

	bool DefensiveStaticEvaluation(CMove*);
	bool TreeAlgo(CMove*);
	int ExtendedStaticEvaluation(MOVE*, MOVE*, signed char, int);

} ; // CMnk

// FIVE structure -- used for addressing values in best win array
#pragma pack(1)     // byte alignment
struct FIVE {
	unsigned char   v0: 5,      // 0:0-4
	         v1a: 3, v1b: 2, // 0:5-7, 1:0-1
	         v2: 5,      // 1:2-6
	         v3a: 1, v3b: 4, // 1:7, 2:0-3
	         v4a: 4, v4b: 1, // 2:4-7, 3:0
	         v5: 5,      // 3:1-5
	         v6a: 2, v6b: 3, // 3:6-7, 4:0-2
	         v7: 5 ;     // 4:3-7
//  unsigned int v0:5, v1:5, v2:5, v3:5, v4:5, v5:5, v6:5, v7:5 ;
} ;
#pragma pack()      // revert to compiler default

// CFileHeader -- file header for data file
class CFileHeader {
	friend class CMnk;

	char m_szText[80] = {};		// descriptive text
	int m_iHeaderSize = 0;		// size of header (# bytes)
	int m_iVersion = 0;			// version number
	int m_iTableStones = 0;		// # stones in stored best win table
	long m_lTableSize = 0;		// length of stored best win table

	void sync(Common::Serializer &s);
};

// class CMnkData -- mankala data - this class used for move analysis
typedef class FAR CMnkData {
	friend class CMnk ;

	CFileHeader m_cFileHeader ; // file header for data file
	long m_NX[MAXTABLESTONES + 1][TOTALPITS + 1] = {},
		m_NA[MAXTABLESTONES + 1][TOTALPITS + 1] = {};
		// NX[s,p] contains the number of arrangements of
		// exactly s stones into p pits.  NA[s,p] contains the
		// number of arrangements of s or fewer stones into
		// p pits.

	HGLOBAL m_hBestWin = nullptr; // Windows handle for best win table
	byte *m_hpcBestWin = nullptr; // pointer to array
		// of 5 bit values.  The subscripts are integers
		// 0 <= subscript <= NUMCONFIGS, and the array element
		// specifies the number of stones (-15 to +15, with -16
		// representing an unknown value) that can be won from
		// this configuration with best play.
} class_CMnkData, *HPCMNKDATA;

#define SPRITE_COUNT    15  // maximum sprites to be displayed
#define BUMPER_COUNT    5   // number of bumper sprites to display
#define SPRITE_TIMER    50  // identifier for sprite timer
#define SPRITE_INTERVAL 10      // time interval between sprite updates
#define SPRITE_GENERATE 30    // number of updates per sprite creation

// Button Identifier codes (BIDs)
#define IDC_QUIT    500
#define IDC_OKAY    501

// Button positioning constants
#define QUIT_BUTTON_WIDTH       50
#define QUIT_BUTTON_HEIGHT      20
#define QUIT_BUTTON_OFFSET_X    10
#define QUIT_BUTTON_OFFSET_Y    10

#define OKAY_BUTTON_WIDTH       63
#define OKAY_BUTTON_HEIGHT      39
#define OKAY_BUTTON_OFFSET_X    10
#define OKAY_BUTTON_OFFSET_Y    10


// CPitWnd -- display class for pit (as well as home bin and hand)
class CPitWnd : public CPit {
	friend class CMnkWindow ;
	CBmpObject *m_xpcStoneChain = nullptr;	// chain header for stone displays
	CBmpObject m_cBmpObject ;   // current bitmap object for pit with stones
};


/////////////////////////////////////////////////////////

// CMnkWindow:

class CMnkWindow : public CFrameWnd, public CMnk {
	bool bPlayedGameOnce = false;

	bool m_bJustStarted = false;	// flag to indicate beginning of game.
	bool m_bGameWon = false;		// flag to indicate game result.
	bool m_bRulesActive = false;	// flag to indicate the unfurled status of the rules scroll.
		// ...this flag is set only when rules are invoked via the F1 key.
	bool m_bScoresDisplayed = false;	// to indicate that the Score has been displayed at the end of game,
		// ... and should pop up again.
	CText *m_pText = nullptr;

	CBmpObject m_cBmpMain ; // bitmap object for main window
	CPalette *m_xpGamePalette = nullptr, *m_xpOldPalette = nullptr;  // game pallet
	CBmpObject *m_xpFreeStoneChain = nullptr;	// chain of unused stone sprites
	CBmpObject *m_xpcBmpFreeChain = nullptr;	// chain of objects to be freed

	CBmpObject m_cBmpScroll;					// options scroll bitmap object
	bool m_bInMenu = false;						// within options menu
	bool m_bPlaySound = false;
	char m_szDataDirectory[100] = {};			// directory for data files

	CRect m_cMainRect,   // screen area spanned by the game window
	      m_cQuitRect,   // window area spanned by the QUIT button
	      m_cOkayRect ;  // window area spanned by the OKAY button

	class CPitWnd *m_xpcPits[NUMPLAYERS][NUMPITS + 2] = {};
	// pointers to pit classes for pits, home bins,
	// and hands
	CBmpObject m_cBmpCrab ; // bitmap object for crab with no sign
	CBmpObject m_cBmpSign ; // bitmap object for crab sign
	int m_iBmpSign = 0;        // SBT_xxxx -- which sign crab displays


// mnk.cpp -- Mankala game -- Windows interface

//- CMnkWindow() -- mankala window constructor function
public: CMnkWindow() ;
//- ~CMnkWindow -- Mankala window destructor function
public: ~CMnkWindow() ;


// mnkui.cpp -- Mankala game -- user interface

//- StartGame -- start a new game
private: bool StartGame() ;
//- PaintBitmapObject -- paint bitmap
private: bool PaintBitmapObject(CBmpObject * xpcBmpObject,
	                                int iBmpType PDFT(0), int iBmpArg PDFT(0)) ;
//- InitBitmapObject -- set up DibDoc in bitmap object
private: bool InitBitmapObject(CBmpObject * xpcBmpObject) ;
//- InitBitmapFilename -- set up filename bitmap object
private: bool InitBitmapFilename(CBmpObject * xpcBmpObject) ;
//- SetBitmapCoordinates -- set coordinates of bitmap
private: bool SetBitmapCoordinates(
	    CBmpObject * xpcBmpObject) ;
//- AcceptClick -- process a mouse click by user
	bool AcceptClick(CPoint cClickPoint) ;
//- MoveStoneDisplay -- move a stone from pit to another
public: bool MoveStoneDisplay(CPitWnd * xpcFromPit,
	                              CPitWnd * xpcToPit) ;
//- AdjustPitDisplay -- adjust display of pit when
//			number of stones changes
private: bool AdjustPitDisplay(CPitWnd * xpcPit,
	                               bool bForcePaint PDFT(false)) ;
//- PaintScreen -- paint screen for mankala game
private: void PaintScreen() ;
//- ProcessDc -- handle processing of device context
private: bool ProcessDc(bool bAlloc PDFT(true)) ;
//- AllocatePits -- allocate pits (including home bin/hand)
	bool AllocatePits() ;
//- SetCrabSign -- to my/your turn
private: bool SetCrabSign(bool bPaint PDFT(true)) ;
//- FreePitResources -- free (optionally delete) all pit
//		resources -- stone sprites and pit bitmaps
private: bool FreePitResources(bool bDelete PDFT(false)) ;
//- ClearBitmapObject -- release bitmap object
private: bool ClearBitmapObject(CBmpObject * xpcBmpObject) ;
//- ReleaseResources -- release all resources before term
public: void ReleaseResources() ;
//- DebugDialog -- put up debugging dialog box
private: bool DebugDialog() ;
public: void Setm_bJustStarted(bool U) {
		m_bJustStarted = U; //- UserDialog -- put up user dialog box
	}
public: bool FAR PASCAL UserDialog() ;
//- OptionsDialog -- call options dialog
private: bool OptionsDialog() ;

//private: bool CALLBACK ResetPitsDlgProc(HWND, unsigned int, WPARAM, LPARAM);



//    CMnkWindow();
//
//
//    void SplashScreen();
//
//    static    void ReleaseResources();
//    static    void FlushInputEvents();
//
//private:
//public:
//    static    void BuildSprites(CDC *pDC);
//    static    void ProcessSprites(CDC *pDC);
//    static    void MoveSprite(CDC *pDC,CSprite *pSprite);

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT OnMCINotify(WPARAM W, LPARAM L);
	virtual LRESULT OnMMIONotify(WPARAM W, LPARAM L);

	//{{AFX_MSG( CMnkWindow )
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(unsigned int, CPoint);
	afx_msg void OnLButtonUp(unsigned int, CPoint);
	afx_msg void OnLButtonDblClk(unsigned int, CPoint);
	afx_msg void OnMButtonDown(unsigned int, CPoint);
	afx_msg void OnMButtonUp(unsigned int, CPoint);
	afx_msg void OnMButtonDblClk(unsigned int, CPoint);
	afx_msg void OnRButtonDown(unsigned int, CPoint);
	afx_msg void OnRButtonUp(unsigned int, CPoint);
	afx_msg void OnRButtonDblClk(unsigned int, CPoint);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg void OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnTimer(uintptr nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
} ;

///////////////////////////////////////////////////////

// CMnkApp:
//
class CMnkApp : public CWinApp {
	CMnkWindow *m_xpcMnkWindow ;
public:
	bool InitInstance();
	int ExitInstance();
} ;

///////////////////////////////////////////////



} // namespace Mankala
} // namespace HodjNPodj
} // namespace Bagel

#endif
