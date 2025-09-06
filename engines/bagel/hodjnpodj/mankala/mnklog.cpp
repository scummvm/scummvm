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
#include "bagel/hodjnpodj/mankala/mnk.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/boflib/misc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Mankala {

extern LPGAMESTRUCT pGameParams;        // declared in mnk.cpp.

bool gbTurnSoundsOff;         // used by mnkui.cpp too.
///DEFS mnk.h

static inline void DebugBreak() {
}

//* CMnk::InitMankala -- initialize a new game of Mankala
bool CMnk::InitMankala()
// returns: true if error, false otherwise
{
	JXENTER(CMnk::InitMankala) ;
	int iError = 0 ;        // error code
	int iPlayer, iPit ;     // loop variables
	CMove * xpcMove = &m_cCurrentMove ; // pointer to current
	// position move object
	CPit * xpcPit ;     // pointer to pit object

	xpcMove->Zero() ;   // clear all data fields in current move/pos object
	xpcMove->m_bRealMove = true ;   // but make this one the real one
//    m_iPlayer = m_iNumberMoves = 0 ;
	for (iPlayer = 0 ; iPlayer < NUMPLAYERS ; ++iPlayer)
		for (iPit = -2 ; iPit < NUMPITS ; ++iPit) {
			xpcPit = m_xpcPits[iPlayer][iPit + 2] ;
			// point to pit object
			if (xpcPit->m_iPlayer != iPlayer ||
			        xpcPit->m_iPit != iPit)
				// validity checking
			{
				//iError = 100 ;
				//goto cleanup ;
			}
//      xpcPit->m_iPlayer = iPlayer ;
//      xpcPit->m_iPit = iPit ;
			xpcPit->m_iNumStones =
			    xpcMove->m_iNumStones[iPlayer][iPit + 2] =
			        (iPit < 0) ? 0  // no stones in home/hand
			        : m_iStartStones ;
		}

	/*cleanup */

	JXELEAVE(CMnk::InitMankala) ;
	RETURN(iError != 0) ;
}

//* CMnk::Move -- make a move
bool CMnk::Move(CPit * xpcSowPit, CMove * xpcMove)
// xpcSowPit -- ptr to pit object whose stones are being sowed
// xpcMove -- move object for position to start from; if nullptr,
//      then use the move object for current position on
//      the board, and make the actual moves on the board
// returns: true if error, false otherwise
{
	JXENTER(CMnk::Move) ;

	int iError = 0 ;        // error code
	int iNumStones ;        // number of stones to be sowed
	int iStone ;        // loop variable
	bool bStonesFound,
	     bOtherStonesFound ;     // flag: stones found, game not over
	int iTemp ;     // temp variable for swapping players
	int iPlayer = xpcSowPit->m_iPlayer, iPit = xpcSowPit->m_iPit ;
	int iOtherPlayer = OTHERPLAYER(iPlayer) ;   // other player (0 or 1)
	int iTempPlayer;
	CPit * xpcHandPit = m_xpcPits[iPlayer][HANDINDEX + 2] ; // hand pit
	CPit * xpcHomePit = m_xpcPits[iPlayer][HOMEINDEX + 2] ; // home bin
	CPit * xpcPit ; // target pit
//	bool B;
	MSG msg;

	//_asm int 3;                    //to track GPf
	if (!xpcMove)       // if no starting position specified
		xpcMove = &m_cCurrentMove ;  // then it's a board move

	iNumStones = xpcMove->m_iNumStones
	             [xpcMove->m_iPlayer][xpcSowPit->m_iPit + 2] ;
	// get # stones being moved
	if (iNumStones <= 0) {  // if no stones
		iError = 100 ;  // attempt to sow an empty pit
		goto cleanup ;
	}

	xpcMove->m_xpcPit = xpcSowPit ; // save ptr to pit being sowed
	xpcMove->m_bFreeTurn = xpcMove->m_bCapture = false ;        //reset FreeTurn, Capture indicators.

	if (xpcMove->m_bRealMove && m_bDumpMoves)
		DumpPosition(xpcMove) ;



	/* move the stones from the bin to the hand     */

	if (pGameParams->bSoundEffectsEnabled) sndPlaySound(PICKUP, SND_ASYNC);

	for (iStone = iNumStones ; iStone-- > 0 ;)

		if (MoveStone(xpcMove, xpcSowPit, xpcHandPit)) {
			iError = 101 ;  // internal error moving stone
			goto cleanup ;
		}

	/*
	    move stones from the hands to the individual pits
	*/
	for (iStone = iNumStones ; iStone-- > 0 ;) {
		if (iPit < 0 || (iPit == 0 && iPlayer !=
		                 xpcMove->m_xpcPit->m_iPlayer))
			// if last iteration placed a stone in my home bin
			// or in my opponent's last pit
			iPit = NUMPITS - 1, iTemp = iPlayer, iPlayer = iOtherPlayer, iOtherPlayer = iTemp ;
		// new pit is last pit on other side -- swap
		// player and other player
		else
			--iPit ;    // otherwise, just decrement to get pit #

		xpcPit = m_xpcPits[iPlayer][iPit + 2] ; // pt to target pit

		if (pGameParams->bSoundEffectsEnabled) sndPlaySound(ONESHELL, SND_ASYNC);

		if (MoveStone(xpcMove, xpcHandPit, xpcPit))
			// move stone from hand to target pit, test for error
		{
			iError = 102 ;
			goto cleanup ;
		}
	}

	if (iPit < 0)   // if my last move was into my home bin
		xpcMove->m_bFreeTurn = true ;   // I get a free turn

	else if (iPlayer == xpcMove->m_xpcPit->m_iPlayer &&
	         xpcMove->m_iNumStones[iPlayer][iPit + 2] == 1) {
		// if the last stone went into a previously empty pit

		if (pGameParams->bSoundEffectsEnabled) sndPlaySound(PICKUP, SND_ASYNC);

		xpcMove->m_bCapture = true ;    // we have a capture
		xpcMove->m_iCapturePit = NUMPITS - 1 - iPit ;
		// number of opponent's pit being captured
		xpcPit = m_xpcPits[iOtherPlayer][xpcMove->m_iCapturePit + 2] ;
		// point to pit object for captured pit

		iNumStones = xpcMove->m_iNumStones[iOtherPlayer][xpcMove->m_iCapturePit + 2] ;

		while (iNumStones-- > 0)        // count stones
			MoveStone(xpcMove, xpcPit, xpcHomePit) ;
		// move stone from captured pit to my home bin pit
	}




	if (!xpcMove->m_bFreeTurn)  // if there's no free turn
		xpcMove->m_iPlayer = OTHERPLAYER(xpcMove->m_iPlayer) ;
	// then other player's turn
	iPlayer = xpcMove->m_iPlayer ;
	iOtherPlayer = OTHERPLAYER(iPlayer) ;


	// check whether there are any moves to be played now -- if not,
	// the game is over
	bStonesFound = false ;  // no stones found yet
	bOtherStonesFound = false;
	for (iPit = 0 ;  iPit < NUMPITS ; iPit++) {
		bStonesFound = bStonesFound || (xpcMove->m_iNumStones[iPlayer][iPit + 2] > 0) ;

		bOtherStonesFound = bOtherStonesFound || xpcMove->m_iNumStones[OTHERPLAYER(iPlayer)][iPit + 2];
	}


	while (MFC::PeekMessage(&msg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)); //flush out pending mouse clicks

	// test for stones in this pit
	if (pGameParams->bSoundEffectsEnabled && !gbTurnSoundsOff && bStonesFound && bOtherStonesFound) {
		if (MFC::PeekMessage(&msg, nullptr, MM_MCINOTIFY, MM_MCINOTIFY, PM_REMOVE)) {
			MFC::TranslateMessage(&msg);
			MFC::DispatchMessage(&msg);
		}

		if (!xpcMove->m_bFreeTurn)
			sndPlaySound(iPlayer ? IGO3 : YOUGO3, SND_SYNC);
		else
			sndPlaySound(iPlayer ? IGOAGAIN : GOAGAIN, SND_SYNC);


	}
	//#endif


	if ((m_bGameOver = (!bStonesFound || !bOtherStonesFound))) {    // game is over if
		// no stones found -- tally the results
		/*
		    depending upon who has no stones left,
		    for each of the other player's pits, transfer the stones
		    to the corresponding home bin
		*/
		iTempPlayer = !bStonesFound ? iOtherPlayer : iPlayer;
		xpcHomePit = m_xpcPits[ iTempPlayer][-1 + 2] ;  // home bin
		for (iPit = 0 ; iPit < NUMPITS ; iPit++) {
			xpcPit = m_xpcPits[iTempPlayer][iPit + 2] ;
			iNumStones = xpcMove->m_iNumStones[iTempPlayer][iPit + 2] ;
			// get number of stones
			while (iNumStones-- > 0)    // count stones
				MoveStone(xpcMove,
				          (CPitWnd *)xpcPit, (CPitWnd *)xpcHomePit) ;
			// move stone from pit to my home bin
		}
	}     // ... winner is the one with more stones in home bin




	if (xpcMove->m_bRealMove && m_bDumpMoves)
		DumpPosition(xpcMove) ;


#define DECENT_CODE 0
	#if DECENT_CODE                            //"THE FOLLOWING SECTION OF CODE IS RUINED"...(nish)
	CMove* xpcStoreMove;
	int iMoveIndex;

	if (xpcMove->m_bRealMove) { // if it's real move
		iMoveIndex = m_iNumberMoves++ ; // get move index, incr count
		xpcStoreMove = m_cMoveList + iMoveIndex ;
		// point to correct slot
		xpcStoreMove->Copy(xpcMove) ;   // copy
	}
	#endif

cleanup:
	JXELEAVE(CMnk::Move) ;
	RETURN(iError != 0) ;
}


//* CMnk::MoveStone -- move one stone for move
bool CMnk::MoveStone(CMove * xpcMove,
                     CPit * xpcFromPit, CPit * xpcToPit)
// xpcMove -- position/move object in which move takes place
// xpcFromPit -- source pit (where stone comes from)
// xpcToPit -- target pit (where stone goes)
// returns: true if error, false otherwise
{
	JXENTER(CMnk::MoveStone) ;
	int iError = 0 ;        // error code

	--xpcMove->m_iNumStones[xpcFromPit->m_iPlayer]
	[xpcFromPit->m_iPit + 2] ;

	if (xpcMove->m_bRealMove)   // if this is a real move
		((CMnkWindow *)this)->MoveStoneDisplay((CPitWnd *)xpcFromPit,
		                                       (CPitWnd *)xpcToPit) ;
	// move display of stone from one pit to another
	++xpcMove->m_iNumStones[xpcToPit->m_iPlayer][xpcToPit->m_iPit + 2] ;
	/*
	MSG msg;
	if(MFC::PeekMessage(&msg,nullptr,MM_MCINOTIFY, MM_MCINOTIFY,PM_REMOVE)){
	    MFC::TranslateMessage(&msg);
	    MFC::DispatchMessage(&msg);
	}
	*/
// cleanup:

	JXELEAVE(CMnk::MoveStone) ;
	RETURN(iError != 0) ;
}

//* CMnk::InitData -- initialize data class object
bool CMnk::InitData(bool bInit)
// bInit -- if false, release data
// returns: true if error, false otherwise
{
	JXENTER(CMnk::InitData) ;
	int iError = 0 ;        // error code
//    long lTableSize ;     // size of lookup table
	long lK ;       // loop variable
	int iJ ;        // loop variable
	byte *hpcTab;    // table pointer for initialization
	byte *xpcFive;        // ptr to Five structure
	struct FIVE stFive = {TABLEUNDEF,           // 0
		       TABLEUNDEF >> 2, TABLEUNDEF & 3, TABLEUNDEF, // 1-2
		       TABLEUNDEF >> 4, TABLEUNDEF & 15,       // 3
		       TABLEUNDEF >> 1, TABLEUNDEF & 1, TABLEUNDEF, // 4-5
		       TABLEUNDEF >> 3, TABLEUNDEF & 7, TABLEUNDEF
	} ;   // 6-7

	if (bInit) {    // if we're initializing
		if (m_iTableStones > MAXTABLESTONES)    // parm set too high
			m_iTableStones = MAXTABLESTONES ;

		if (!(m_lpCMnkData = new FAR CMnkData)) {
			iError = 100 ;  // can't allocate CMnkData
			goto cleanup ;
		}
		if ((iError = CountConfigurations()))
			goto cleanup ;

		m_lNumConfigs = m_lpCMnkData->m_NA[m_iTableStones][TOTALPITS] ;

//  m_lTableSize = (MAXCONFIGS/8 + 1)*5 ; // table length in bytes
		m_lTableSize = (m_lNumConfigs / 8 + 2) * 5 ; // table length in bytes

		if (!(m_lpCMnkData->m_hBestWin = GlobalAlloc(0, m_lTableSize)))
			// allocate global handle and test
		{
			iError = 110 ;  // Windows GlobalAlloc failure
			goto cleanup ;
		}
		if (!(m_lpCMnkData->m_hpcBestWin
		        = (byte *)GlobalLock(m_lpCMnkData->m_hBestWin))) {
			iError = 111 ;  // Windows GlobalLock failure
			goto cleanup ;
		}

		#if 0
		if (!(m_lpCMnkData->m_hpcBestWin =
		            new byte[m_lTableSize + 8])) {
			iError = 101 ;  // can't allocate best win array
			goto cleanup ;
		}
		#endif

		if (m_bInitData) {  // if we're initializing data tables
			// init best win table to all undefined values
			for (lK = m_lTableSize / sizeof(struct FIVE),
			        hpcTab = m_lpCMnkData->m_hpcBestWin ; lK-- > 0 ;)
				for (iJ = sizeof(struct FIVE),
				        xpcFive = (byte *)&stFive ; iJ-- > 0 ;)
					*hpcTab++ = *xpcFive++ ;

			PopulateTable() ;   // populate the best win table
			WriteTableFile() ;  // write best win table to disk
		} else {    // initialized data is already available
			//ReadTableFile() ;   // read best win table from disk
		}
	}

	else {      // we're releasing table
		if (m_lpCMnkData) { // if data table locked
			GlobalUnlock(m_lpCMnkData->m_hBestWin) ;    // unlock it
			m_lpCMnkData = nullptr ;   // clear pointer
		}
		if (m_lpCMnkData->m_hBestWin)       // if allocated
			GlobalFree(m_lpCMnkData->m_hBestWin),
			           m_lpCMnkData->m_hBestWin = nullptr ;
		// free it
		#if 0
		if (m_lpCMnkData) { // if data table allocated
			if (m_lpCMnkData->m_hpcBestWin)
				// if there's a best win table
				delete m_lpCMnkData->m_hpcBestWin,
				       m_lpCMnkData->m_hpcBestWin = nullptr ;
			delete m_lpCMnkData ;
			m_lpCMnkData = nullptr ;
		}
		#endif
	}

//delete m_lpCMnkData ;
//m_lpCMnkData = nullptr ;

cleanup:

	JXELEAVE(CMnk::InitData) ;
	RETURN(iError != 0) ;
}

//* CMnk::CountConfigurations -- set up Configurations table
bool CMnk::CountConfigurations()
// returns: true if error, false otherwise
{
	JXENTER(CMnk::CountConfigurations) ;
	int iError = 0 ;        // error code
	int iStone, iPit, iK ;  // loop variables

	for (iStone = 0 ; iStone <= m_iTableStones ; ++iStone)
		m_lpCMnkData->m_NX[iStone][1] = 1 ; // if there's only one
	// pit, then there's only one configuration

	for (iPit = 2 ; iPit <= TOTALPITS ; ++iPit)
		for (iStone = 0 ; iStone <= m_iTableStones ; ++iStone) {
			m_lpCMnkData->m_NX[iStone][iPit] = 0 ;
			for (iK = 0 ; iK <= iStone ; ++iK)
				m_lpCMnkData->m_NX[iStone][iPit] +=
				    m_lpCMnkData->m_NX[iK][iPit - 1] ;
		}

	for (iPit = 1 ; iPit <= TOTALPITS ; ++iPit)
		for (iStone = 0 ; iStone <= m_iTableStones ; ++iStone) {
			m_lpCMnkData->m_NA[iStone][iPit] = 0 ;
			for (iK = 0 ; iK <= iStone ; ++iK)
				m_lpCMnkData->m_NA[iStone][iPit] +=
				    m_lpCMnkData->m_NX[iK][iPit] ;
		}

// cleanup:

	JXELEAVE(CMnk::CountConfigurations) ;
	RETURN(iError != 0) ;
}

//* CMnk::PopulateTable -- compute values for best win table
bool CMnk::PopulateTable() {
	JXENTER(CMnk::PopulateTable) ;
	int iError = 0 ;        // error code
	long lConfigIndex ;     // loop variable
	CMove cMove ;       // move structure

	m_iCurrentMaxDepth = 3 ; // minimax depth is 3 for table populate
	m_iCurrentCapDepth = 0 ; // capture depth is 0

	for (lConfigIndex = 1 ; lConfigIndex < MAXCONFIGS - 1 ; ++lConfigIndex) {
		cMove.Zero() ;      // clear move/position object
		cMove.m_lConfigIndex = lConfigIndex ;   // store into move object
		if (UnmapConfiguration(&cMove))    // translate configuration index
			// into a configuration, test for error
		{
			iError = 100 ;  // UnmapConfiguration error
			goto cleanup ;
		}
		if (Minimax(&cMove)) {  // find move, and test for error
			iError = 101 ;  // Minimax error
			goto cleanup ;
		}
		if (SetBestWinCount(&cMove)) {
			iError = 102 ;  // SetBestWinCount error
			goto cleanup ;
		}
	}

cleanup:

	if (iError)
		iError += 100 ;

	JXELEAVE(CMnk::PopulateTable) ;
	RETURN(iError != 0) ;
}

bool CMnk::WriteTableFile() {
	int iError = 0;			// error code
	JXENTER(CMnk::WriteTableFile);

	Common::strcpy_s(m_lpCMnkData->m_cFileHeader.m_szText,
	                 "Mankala data file, version 1.0, "
	                 "May, 1994, by John J. Xenakis");
	m_lpCMnkData->m_cFileHeader.m_iHeaderSize = sizeof(CFileHeader) ;
	m_lpCMnkData->m_cFileHeader.m_iVersion = 100 ;
	m_lpCMnkData->m_cFileHeader.m_iTableStones = m_iTableStones ;
	m_lpCMnkData->m_cFileHeader.m_lTableSize = m_lTableSize ;

	Common::WriteStream *ws = g_system->getSavefileManager()->openForSaving("mankala.dat");
	if (ws) {
		Common::Serializer s(nullptr, ws);
		m_lpCMnkData->m_cFileHeader.sync(s);
	} else {
		// Can't create file
		iError = 100;
	}

	ws->finalize();
	delete ws;

	JXELEAVE(CMnk::WriteTableFile) ;
	RETURN iError != 0;
}

bool CMnk::ReadTableFile() {
	JXENTER(CMnk::ReadTableFile) ;
	int iError = 0 ;        // error code

	Common::SeekableReadStream *rs = g_system->getSavefileManager()->openForLoading("mankala.dat");
	if (rs) {
		Common::Serializer s(rs, nullptr);
		m_lpCMnkData->m_cFileHeader.sync(s);

		if (m_lpCMnkData->m_cFileHeader.m_iVersion != 100) {
			iError = 102;      // invalid file header;
		} else {
			if (m_iTableStones > m_lpCMnkData->m_cFileHeader.m_iTableStones)
				m_iTableStones = m_lpCMnkData->m_cFileHeader.m_iTableStones;

			if (m_lTableSize > m_lpCMnkData->m_cFileHeader.m_lTableSize)
				m_lTableSize = m_lpCMnkData->m_cFileHeader.m_lTableSize;

			//lTotalCount = m_lTableSize;
			// total number of bytes to be read
			//hpData = m_lpCMnkData->m_hpcBestWin;   // point to beginning of table
		}
	} else {
		iError = 100;      // can't open file
	}

	delete rs;

	JXELEAVE(CMnk::ReadTableFile);
	RETURN(iError != 0);
}

//* CMnk::MapConfiguration -- map a configuration to its integer index,
//      store configuration index into Move object
bool CMnk::MapConfiguration(CMove * xpcMove)
// xpcMove -- CMove object containing the configuration
// returns: true if error, false otherwise
{
	JXENTER(CMnk::MapConfiguration) ;
	int iError = 0 ;        // error code
	long lConfigIndex = 0 ; // return value, configuration index
	int iStones ;       // total number of stones
	int iPlayer = xpcMove->m_iPlayer ;
	int iOtherPlayer = OTHERPLAYER(iPlayer) ;
	int iPit, iPitCount;    // #pit, #stones in pit ;
	int iK ;            // loop variable

	CountStones(xpcMove) ;  // set m_iTotalStones
	iStones = xpcMove->m_iTotalStones ;

	if (iStones > m_iTableStones)       // if number of stones
		// exceeds the number we're supporting
		lConfigIndex = -1 ;
	else {
		iPit = TOTALPITS ;  // start from pit 12
		iPitCount = -1 ;    // force fetch of # stones in pit
		lConfigIndex = (iStones == 0) ? 0 :
		               m_lpCMnkData->m_NA[iStones - 1][TOTALPITS] ;
		// start from end of table as of previous # stones

		for (iK = 1 ; iK <= iStones ; ++iK) {
			while (iPitCount <= 0) { // find a stone
				if (iPitCount == 0 && --iPit < 1) {
					iError = 100 ;  // logic error in algorithm
					goto cleanup ;
				}

				// set iPitCount to the number of stones in iPit
				if (iPit <= NUMPITS)    // pit for current player
					iPitCount = xpcMove->m_iNumStones[iPlayer]
					            [iPit - 1 + 2] ;
				else        // pit for other player
					iPitCount = xpcMove->m_iNumStones
					            [iOtherPlayer]
					            [iPit - (NUMPITS + 1) + 2] ;
			}

			lConfigIndex += m_lpCMnkData->m_NX[iStones - iK + 1]
			                [iPit - 1] ;
			--iPitCount ; // one less stone to count in this pit
		}
		++lConfigIndex ;    // index begins at 1
	}

cleanup:
	xpcMove->m_lConfigIndex = lConfigIndex ;    // store result

	JXELEAVE(CMnk::MapConfiguration) ;
	RETURN(iError != 0) ;
}

//* CMnk::UnmapConfiguration -- map configuration index back
//              to configuration
bool CMnk::UnmapConfiguration(CMove * xpcMove)
// xpcMove -- CMove object containing the configuration / index
// returns: true if error, false otherwise
{
	JXENTER(CMnk::UnmapConfiguration) ;
	int iError = 0 ;        // error code
	long lConfigIndex = xpcMove->m_lConfigIndex - 1 ;  // config index
	long lConfigSave = lConfigIndex + 1 ;   // save for debugging
	int iPlayer = xpcMove->m_iPlayer ;
	int iOtherPlayer = OTHERPLAYER(iPlayer) ;
	int iStones ;       // total number of stones
	int iPit, iPitCur ;     // loop variable
	int iK ;            // loop variable
	long lValue = 0L, lValueNew ;   // table values
	char szDebugStr[200] ;  // debugging string

	if (lConfigIndex < 0 || lConfigIndex >= MAXCONFIGS) {
		iError = 100 ;  // config index is out of range
		goto cleanup ;
	}

	// find number of stones in configuration
//    xpcMove->m_iTotalStones = 0 ;
	for (iStones = 0 ; iStones <= m_iTableStones
	        && (lValueNew = m_lpCMnkData->m_NA[iStones][TOTALPITS])
	        <= lConfigIndex ; ++iStones, lValue = lValueNew)
		;   // null loop body

	xpcMove->m_iTotalStones = iStones ;

	if (iStones > m_iTableStones) {
		iError = 101 ;  // can't compute number of stones
		goto cleanup ;
	}

	lConfigIndex -= lValue ;

	for (iK = xpcMove->m_iTotalStones ; iK >= 1 ; --iK) {
		// find which pit stone # iK is in
		iPit = 1 ;
		for (iPitCur = 1, lValue = 0L ; iPitCur <= TOTALPITS
		        && (lValueNew = m_lpCMnkData->m_NX[iK][iPitCur])
		        <= lConfigIndex ;
		        lValue = lValueNew, ++iPitCur) ;
		iPit = iPitCur ;

//  if (iPit > TOTALPITS)
//  {
//      iError = 120 + iK ;     // can't compute pit #
//      goto cleanup ;
//  }

		lConfigIndex -= lValue ;

//  // ***** debugging
//  Common::sprintf_s(szDebugStr, "lConfig=%ld, #stones=%d, pit %d = %d\n",
//              lConfigSave, iStones, iK, iPit) ;
//  debug(szDebugStr) ;

		// increment stone count for appropriate pit
		if (iPit <= NUMPITS)    // pit for current player
			++xpcMove->m_iNumStones[iPlayer][iPit - 1 + 2] ;
		else        // pit for other player
			++xpcMove->m_iNumStones[iOtherPlayer]
			[iPit - (NUMPITS + 1) + 2] ;

	}

	// *** debugging
	MapConfiguration(xpcMove) ;
	if (lConfigSave != xpcMove->m_lConfigIndex) { // if unmap/map failed
		Common::sprintf_s(szDebugStr, "Config %ld changed to %ld.\n",
		                  lConfigSave, xpcMove->m_lConfigIndex) ;
		debug("%s", szDebugStr) ;
		DebugBreak();
	}

cleanup:

	JXELEAVE(CMnk::UnmapConfiguration) ;
	RETURN(iError != 0) ;
}

//* CMnk::SearchMove -- search for best move
bool CMnk::SearchMove(CMove * xpcMove, int &iMove)
// xpcMove -- pointer to move/position to find move for
// iMove (output) -- recommended pit number
// returns: true if error, false otherwise
{
	JXENTER(CMnk::SearchMove) ;

	int iError = 0 ;        // error code
	//bool bDone = false ;    // flag: evaluations done
	//int iNumMoves = 0 ;     // number of legal moves
	int iNumStones ;        // number of stones in pit
//    bool bFree[NUMPITS], bCapture[NUMPITS] ;
//    int iValue[NUMPITS] ; // value of each pit
	int iPit ;      // loop variable
	//int iLastPit = -1 ; // for finding lowest/highest # pit
	int iMaxValue = BESTWINUNDEF, iNumberAtMax = 0,
	                              tmpVal, maxtmpVal ;
	// computing maximum value
//    CPit * xpcPit ;   // pit being processed
	int iPlayer = xpcMove->m_iPlayer ;  // person on the move
//    CMove * xpcMove = &m_cCurrentMove ; // current move/position object
	bool bStonesFound;

	m_iCurrentMaxDepth = m_iMaxDepth[iPlayer] ; // minimax depth
	m_iCurrentCapDepth = m_iCapDepth[iPlayer] ; // capture depth

	for (iPit = 0 ; iPit < NUMPITS ; ++iPit)
		xpcMove->m_iValues[iPit] = BESTWINUNDEF ;   // values undefined

	switch (m_eLevel[iPlayer])  // switch based on type of
		// move generation algorithm
	{
	case LEV_RANDOM:    // handle these individually below
		break;

	case LEV_LOWEST:
		StaticEvaluation(xpcMove);
		// fall through
	case LEV_HIGHEST:
		AggressiveStaticEvaluation(xpcMove);
		break;

	case LEV_EVAL:      // evaluate statically
		DefensiveStaticEvaluation(xpcMove) ;
		break ;

	case LEV_MINIMAX:
		TreeAlgo(xpcMove);
		//Minimax(xpcMove) ;  // use minimax algorithm
		break ;

	default:
		iError = 100 ;  // invalid m_eLevel value
		goto cleanup ;
	}

	for (iPit = 0, bStonesFound = false ; iPit < NUMPITS ; ++iPit) {
		iNumStones =  xpcMove->m_iNumStones[iPlayer][iPit + 2];
		bStonesFound = bStonesFound || iNumStones;

		if ((iNumStones   > 0))
			// get # stones, test
			// if this is a possible move
			switch (m_eLevel[iPlayer])  // switch based on type of
				// move generation algorithm
			{
			case LEV_RANDOM:
				// Random choice
				xpcMove->m_iValues[iPit] = 1 ;
				break ;
			case LEV_LOWEST:
			case LEV_HIGHEST:
			case LEV_EVAL:      // these were handled above
			case LEV_MINIMAX:
				break ;

			default:
				iError = 101 ;  // invalid m_eLevel value
				goto cleanup ;
			}
	}
	/*
	if(!bStonesFound){
	     iPit=-3;               //-3 is a good choice because iPit=-1, iPit=-2 represent the hand and home bins.
	    return(false);   //let the while loop in mnkui.cpp that calls SearchMove continue, so that moving stones is automatic.
	}
	*/

	// loop thru pits, find those with maximum value
	for (iPit = 0 ; iPit < NUMPITS ; ++iPit)
		if (xpcMove->m_iValues[iPit] > iMaxValue)   // new max
			iMaxValue = xpcMove->m_iValues[iPit], iNumberAtMax = 1 ;
		else if (xpcMove->m_iValues[iPit] == iMaxValue)
			++iNumberAtMax ;

	if (iMaxValue <= BESTWINUNDEF)  // if no move found (or there
		// are no legal moves)
	{
		iError = 110 ;      // no move found
		goto cleanup ;
	}


	for (maxtmpVal = BESTWINUNDEF + 1, iPit = 0; iPit < NUMPITS ; ++iPit) {
		if (xpcMove->m_iValues[iPit] == iMaxValue)
			/*
			    Among those with equal iValues, Select the least range pit of 'em all
			*/
			if ((tmpVal = iPit - xpcMove->m_iNumStones[xpcMove->m_iPlayer][iPit + 2]) > maxtmpVal) {
				iMove = iPit;
				maxtmpVal = tmpVal;
			}
	}



cleanup:
	JXELEAVE(CMnk::SearchMove) ;
	RETURN(iError != 0) ;
}

//* CMnk::Minimax -- find best move from supplied configuration
bool CMnk::Minimax(CMove * xpcMove, int iDepth)
// xpcMove -- CMove object containing the configuration
// iDepth -- minimax tree depth so far
// returns: true if error, false otherwise
{
	JXENTER(CMnk::Minimax) ;

	int iError = 0 ;        // error code
	int iPlayer = xpcMove->m_iPlayer ;  // player moving
	CMove cMoveBase, cMove ;    // six possible moves
	int iLegalMoves = 0 ;   // number of legal moves
	CPit * xpcSowPit ;  // ptr to CPit object being sowed
	int iValue, iMaxValue = BESTWINUNDEF ;  // move value for this move
	int iBestMove = -1 ;        // best move
	bool bDone ;    // flag: processing this move is done
	int iPit, iP ;      // loop variable

	for (iPit = 0 ; iPit < NUMPITS ; ++iPit)
		xpcMove->m_iValues[iPit] = BESTWINUNDEF ;  // values undefined

	memcpy(&cMoveBase, xpcMove, sizeof(CMove)) ;
	// form a base CMove object
	cMoveBase.m_iNumStones[0][HOMEINDEX + 2]
	    = cMoveBase.m_iNumStones[1][HOMEINDEX + 2] = 0 ;
	// clear both players' home bins
	cMoveBase.m_bRealMove = false ;

	for (iPit = 0 ; iPit < NUMPITS ; ++iPit) {
		bDone = false ;         // not done yet
		iValue = BESTWINUNDEF ;     // no value yet
		if (xpcMove->m_iNumStones[iPlayer][iPit + 2] == 0)
			// if this pit has no stones in it
			bDone = true ;      // nothing to do
		if (!bDone) {
			++iLegalMoves ; // increment legal move count
			memcpy(&cMove, &cMoveBase, sizeof(CMove)) ;
			// copy the new CMove object from base object

			xpcSowPit = m_xpcPits[iPlayer][iPit + 2] ;
			Move(xpcSowPit, &cMove) ;   // make move
			// sow pit # iPit, and change iPlayer
			CountStones(&cMove) ;   // set m_iTotalStones

			if (cMove.m_iTotalStones <= m_iTableStones)
				// if we have the value of this in the
				// best win table
			{
				MapConfiguration(&cMove) ;  // map stone configuration
				// to index in best win table
				GetBestWinCount(&cMove) ;
				iValue = cMove.m_iBestWinValue ;
				// get value of this move
				if (iValue > BESTWINUNDEF)  // value was in table
					bDone = true ;
			}
			if (!bDone && (iDepth < m_iCurrentMaxDepth ||
			               (iDepth < m_iCurrentMaxDepth + m_iCurrentCapDepth
			                && (cMove.m_bCapture || cMove.m_bFreeTurn)))) {
				Minimax(&cMove, iDepth + 1) ;
				iValue = cMove.m_iBestWinValue ;
				bDone = true ;
			}

			if (!bDone) {
				iValue = 0 ;
				for (iP = 0 ; iP < NUMPITS ; ++iP)
					iValue += (cMove.m_iNumStones[cMove.m_iPlayer][iP + 2]
					           - cMove.m_iNumStones
					           [OTHERPLAYER(cMove.m_iPlayer)][iP + 2]) / 2 ;
				bDone = true ;
			}

//      if (!bDone)
//      StaticEvaluation(&cMove) ;  // do static analysis

			iValue += cMove.m_iNumStones[cMove.m_iPlayer][HOMEINDEX + 2]
			          - cMove.m_iNumStones[OTHERPLAYER(cMove.m_iPlayer)]
			          [HOMEINDEX + 2] ;

			if (cMove.m_iPlayer != iPlayer && iValue > BESTWINUNDEF)
				iValue = -iValue ;  // reverse value if other player

			if (iMaxValue < iValue) // new max?
				iMaxValue = iValue, iBestMove = iPit ;
		}
		xpcMove->m_iValues[iPit] = iValue ; // save val for this move
	}

	if ((xpcMove->m_iNumMoves = iLegalMoves) == 0)
		// if there were no legal moves
		iMaxValue = - xpcMove->m_iTotalStones ;
	// if there is no legal move, then the value of this
	// position is minus the number of stones on the
	// other side

// ... by inserting test for iLegalMoves > 1 here, can provide other
//  patterns -- eg, occasionally choosing less good move

	xpcMove->m_iBestWinValue = iMaxValue ;  // store best value
	xpcMove->m_iBestMove = iBestMove ;      // and best move

// cleanup:
	JXELEAVE(CMnk::Minimax) ;
	RETURN(iError != 0) ;
}


//* CMnk::StaticEvaluation -- determine static value of a position
bool CMnk::StaticEvaluation(CMove * xpcMove)
// xpcMove -- position to be evaluated
// returns: true if error, false otherwise
{
	JXENTER(CMnk::StaticEvaluation) ;

	int iError = 0 ;        // error code
	int iNumMoves = 0 ;     // number of legal moves
	int iNumStones ;        // number of stones in pit
	bool bFree[NUMPITS], bCapture[NUMPITS] ;
	int iPit ;      // loop variable
	//int iMaxValue = BESTWINUNDEF ;  // computing maximum value
	//CPit * xpcPit ; // pit being processed
	int iPlayer = xpcMove->m_iPlayer ;  // current player

	xpcMove->m_bHasCapture = xpcMove->m_bHasFree = false ;

	for (iPit = 0 ; iPit < NUMPITS ; ++iPit) {
		xpcMove->m_iValues[iPit] = BESTWINUNDEF ;  // no value yet
//		xpcPit = m_xpcPits[iPlayer][iPit + 2] ; // pt to pit object
		if ((iNumStones = xpcMove->m_iNumStones[iPlayer][iPit + 2]))
			// get number of stones and test
			++iNumMoves ;   // it's a legal move

		bFree[iPit] = (iNumStones &&
		               (iNumStones % (2 * NUMPITS + 1) == iPit + 1)) ;
		// test whether this pit yields a free turn

		if (iNumStones == 2 * NUMPITS + 1)
			bCapture[iPit] = true ;

		else if (iNumStones && iNumStones <= iPit)
			bCapture[iPit] = (xpcMove->m_iNumStones[iPlayer]
			                  [iPit - iNumStones + 2] == 0) ;

		else if (iNumStones > iPit + NUMPITS && iNumStones <= 2 * NUMPITS)
			bCapture[iPit] = (xpcMove->m_iNumStones[iPlayer]
			                  [iPit - iNumStones + NUMPITS + 2] == 0) ;

		else
			bCapture[iPit] = false ;

		if (bFree[iPit])
			xpcMove->m_bHasFree = true ;

		if (bCapture[iPit])
			xpcMove->m_bHasCapture = true ;

		// the following formula is really just a guess
		xpcMove->m_iValues[iPit] = iNumStones
		                           + (bCapture[iPit] ? 5 : 0) + (bFree[iPit] ? 5 : 0) ;
	}

	xpcMove->m_iNumMoves = iNumMoves ;  // store # legal moves

// cleanup:
	JXELEAVE(CMnk::StaticEvaluation) ;
	RETURN(iError != 0) ;
}


//* CMnk::AggressiveStaticEvaluation -- determine static value of a position
bool CMnk::AggressiveStaticEvaluation(CMove * xpcMove)
// xpcMove -- position to be evaluated
// returns: true if error, false otherwise

/*
    For Simplicity: defensive moves are discussed from pt of view of crab
*/
{
	JXENTER(CMnk::StaticEvaluation) ;

	int iError = 0 ;        // error code
	int iNumMoves = 0 ;     // number of legal moves
	int iNumStones ;        // number of stones in pit
	int iFree,
	    iCapture,
	    maxCapture = 0;
	//int maxFree=0;
	int iPit,
	    iCapturePit = 0,
	    iFreePit = 0;
	//int iMaxValue = BESTWINUNDEF ;  // computing maximum value
	int iPlayer = xpcMove->m_iPlayer ;  // current player

	bool bAFreePitExists,
	     bACaptureExists,
	     bDoFreePitFirst;
	//CPit * xpcPit ; // pit being processed


	/*INITIALIZE*/
	xpcMove->m_bHasCapture = xpcMove->m_bHasFree = false ;
	bDoFreePitFirst = false;
	bAFreePitExists = false;
	bACaptureExists = false;
	maxCapture = 0;

	for (iPit = 0, bAFreePitExists = false, bACaptureExists = false ; iPit < NUMPITS ; ++iPit) {
		xpcMove->m_iValues[iPit] = BESTWINUNDEF ;  // no value yet
		//xpcPit = m_xpcPits[iPlayer][iPit + 2] ; // pt to pit object


		if ((iNumStones = xpcMove->m_iNumStones[iPlayer][iPit + 2]))
			// get number of stones and test
			++iNumMoves ;   // it's a legal move


		iFree = (iNumStones && (iNumStones % (2 * NUMPITS + 1) == iPit + 1)) ? 1 : 0;
		// test whether this pit yields a free turn

		if (iNumStones == 2 * NUMPITS + 1)
			iCapture = xpcMove->m_iNumStones[OTHERPLAYER(iPlayer)][NUMPITS + 1 - iPit];

		else if (iNumStones && (iNumStones <= iPit) && !xpcMove->m_iNumStones[iPlayer][iPit - iNumStones + 2])
			iCapture = xpcMove->m_iNumStones[OTHERPLAYER(iPlayer)][NUMPITS + 1 - iPit + iNumStones] ;

		else if (iNumStones > iPit + NUMPITS && (iNumStones <= 2 * NUMPITS) &&  !xpcMove->m_iNumStones[iPlayer][2 * NUMPITS + 3 + iPit - iNumStones])
			iCapture = xpcMove->m_iNumStones[OTHERPLAYER(iPlayer)][iNumStones - iPit - NUMPITS];

		else
			iCapture = 0;

		if (iFree) {
			xpcMove->m_bHasFree = true ;
			if (!bAFreePitExists) iFreePit = iPit;                              //so that you start w/ the closest  free Pit (ie. iPit with the lowest index),
			if (iNumStones <= NUMPITS) bAFreePitExists = true; //unless there's a "WRAP AROUND" in which case the farthest pit will be selected.
		}

		/* determine which pit yields the max capture      */
		if (iCapture) {
			xpcMove->m_bHasCapture = true ;
			if (iCapture > maxCapture) {
				maxCapture = iCapture;
				iCapturePit = iPit;
			}
			bACaptureExists = true;
		}
	}//end- for

	/*  Determine whether the FreePit "interferes" with the CapturePit   */
	if (bACaptureExists && bAFreePitExists) {
		if ((xpcMove->m_iNumStones[iPlayer][iFreePit + 2] < NUMPITS - 1) &&        //i.e. if iFreePit <=3
		        (xpcMove->m_iNumStones[iPlayer][iCapturePit + 2] < NUMPITS + 1) &&  //i.e. No Wrap Around Capture
		        ((iCapturePit - xpcMove->m_iNumStones[iPlayer][iCapturePit + 2])  > iFreePit)) {   //i.e. the landing pit in the capture case, ...
			//  is  farther than the freePit.
			bDoFreePitFirst = true;
		}
	}



	if (((maxCapture < 4) && bAFreePitExists) || bDoFreePitFirst)
		xpcMove->m_iValues[iFreePit] = 5;
	else if (bACaptureExists)
		xpcMove->m_iValues[iCapturePit] = 5;
	else for (signed char i = 0; i < NUMPITS;  xpcMove->m_iValues[i] = (xpcMove->m_iNumStones[iPlayer][i + 2]) ? 5 : 0, i++);

	xpcMove->m_iNumMoves = iNumMoves ;  // store # legal moves


// cleanup:
	JXELEAVE(CMnk::AggressiveStaticEvaluation) ;
	RETURN(iError != 0) ;
}

//* CMnk::StaticEvaluation -- determine static value of a position
bool CMnk::DefensiveStaticEvaluation(CMove * xpcMove)
// xpcMove -- position to be evaluated
// returns: true if error, false otherwise

/*
    For Simplicity: defensive moves are discussed from pt of view of crab
*/
{
	JXENTER(CMnk::DefensiveStaticEvaluation) ;

	int iError = 0 ;        // error code
	int iNumMoves = 0 ;     // number of legal moves
	int iNumStones,        // number of stones in pit
	    iOtherNumStones,    //number of stones in opponent's pits.
	    iMaxStones,
	    StonesGained,
	    StonesSaved,
	    TotalBenefit,
	    maxTotalBenefit;
	int OtherPlayer;
	int iFree,
	    iCapture,
	    maxCapture,
//	    maxFree,
	    maxThreat,
	    minThreat;
	int iPit = 0,
	    iCapturePit = 0,
	    iFreePit = 0,
	    iMaxThreatPit = 0,
	    iCurrentPitUnderThreat = 0,
	    iSaved = 0,
	    iStartWithThisCapture = 0;
	//int iMaxValue = BESTWINUNDEF ;  // computing maximum value
	int iPlayer = xpcMove->m_iPlayer ;  // current player
	int ThreatCount,                                //how many pits of HUMAN are  aggressive.
	    iEmptyPitCount,
	    iOtherEmptyPitCount;
	int iPitUnderThreat[NUMPITS];         //List of pits of CRAB  under threat.
	int iEvasionNecessary[NUMPITS];     //Amount of threat to a pit on this side.
	int iOtherOffensivePit[NUMPITS]; //which pit a given pit on the opposite side is offensive to.
	int jYieldsACapture[NUMPITS];       //the pit on the opposite side that can be captured by a given pit.

	int ii,
	    u,
	    j,
	    k,
	    kk,
	    tmpValues,
	    tmpCount;

	bool bAFreePitExists,
	     bACaptureExists,
	     bAThreatExists,
	     bDoFreePitFirst,
	     bEndGamePlay,
	     bNearClustered,
	     bConclusive,
	     bSaveable,
	     bUnSaveable;
//	CPit * xpcPit ; // pit being processed


	/*INITIALIZE*/
	xpcMove->m_bHasCapture = xpcMove->m_bHasFree = false ;
	bAThreatExists = false;
	bDoFreePitFirst = false;
	bAFreePitExists = false;
	bACaptureExists = false;
	bEndGamePlay = false;
	bConclusive = false;
	bSaveable = false;
	bUnSaveable = false;
	tmpCount = 0;
	maxCapture = 0;
	maxThreat = 0;
	minThreat = 0;
	ThreatCount = 0;
	iMaxStones = 0;
//	maxFree = 0;
	maxTotalBenefit = 0;
	bNearClustered = false;

	memset((void *)iEvasionNecessary, 0x0, NUMPITS * sizeof(int));
	memset((void *)iPitUnderThreat, -1, NUMPITS * sizeof(int));
	memset((void *)iOtherOffensivePit, -1, NUMPITS * sizeof(int));


	/*
	note that in SOME OF THE following sections of code
	    the prefix "Other" may refer to the computer_crab.
	    Codewise though the terms are perfectly interchangeable.
	*/

	/*determine if this is an EndOfGame Play move */
	for (ii = 0, iEmptyPitCount = 0, iOtherEmptyPitCount = 0; ii < NUMPITS; ii++) {
		if (!xpcMove->m_iNumStones[0][ii + 2])
			iEmptyPitCount++;

		if (!xpcMove->m_iNumStones[1][ii + 2])
			iOtherEmptyPitCount++;
	}

	if (((iEmptyPitCount > 2) && (iOtherEmptyPitCount > 2)) || (bNearClustered && ((iEmptyPitCount > 1) || (iOtherEmptyPitCount > 1))))
		bEndGamePlay = true;


	/*
	    SET UP DEFENSIVE MOVES, DETERMINE OFFENDING CRAB_PITS, and
	    MY PITS UNDER ATTACK
	*/
	for (OtherPlayer = OTHERPLAYER(iPlayer), j = 0; j < NUMPITS; j++) {
		iOtherNumStones = xpcMove->m_iNumStones[OtherPlayer][j + 2];    /*Stones in the j-th HUMAN pit*/

		/*
		     if there are 13 stones in the HUMAN'S pit then the (5-j)th CRAB'S pit is threatened
		*/
		if (iOtherNumStones == 2 * NUMPITS + 1)
			iCurrentPitUnderThreat = NUMPITS - j - 1;

		else if (iOtherNumStones && (iOtherNumStones <= j) && !xpcMove->m_iNumStones[OtherPlayer][j - iOtherNumStones + 2]) {
			iCurrentPitUnderThreat = NUMPITS - 1 - j + iOtherNumStones;
		}

		else if ((iOtherNumStones > j + NUMPITS) && (iOtherNumStones <= 2 * NUMPITS)
		         && (!xpcMove->m_iNumStones[OtherPlayer][2 * NUMPITS + 3 + j - iOtherNumStones]))
			iCurrentPitUnderThreat = iOtherNumStones - j - NUMPITS - 2;
		else
			iCurrentPitUnderThreat = -1;

		/* then check to see if there are stones in iCurrentPitUnderThreat at all */
		if ((iCurrentPitUnderThreat != -1) && !xpcMove->m_iNumStones[iPlayer][iCurrentPitUnderThreat + 2])
			iCurrentPitUnderThreat = -1;

		/* if there are stones in the pit under threat then set flags */
		if (iCurrentPitUnderThreat != -1) {
			bAThreatExists = true;
			iPitUnderThreat[ThreatCount] = iCurrentPitUnderThreat;
			iEvasionNecessary[iCurrentPitUnderThreat] = xpcMove->m_iNumStones[iPlayer][iCurrentPitUnderThreat + 2]; //the extent of threat to a given CRAB pit
			iOtherOffensivePit[j] = iCurrentPitUnderThreat;                     // the j th HUMAN pit is offensive to iCurrentPitUnderThreat CRAB pit.
			ThreatCount++;                                             //the total # of CRAB pits under threat.
		}
	}//end for -j



	/*
	    Find the pit least threatened and the one most threatened
	*/
	for (ii = 0; ii < NUMPITS; ii++) {
		if (maxThreat < iEvasionNecessary[ii]) {
			maxThreat = iEvasionNecessary[ii];
			iMaxThreatPit = ii;
		}
		minThreat = min(minThreat, iEvasionNecessary[ii]);
	}



	for (iPit = 0, bAFreePitExists = false, bACaptureExists = false ; iPit < NUMPITS ; ++iPit) {
		xpcMove->m_iValues[iPit] = BESTWINUNDEF ;  // no value yet
		//xpcPit = m_xpcPits[iPlayer][iPit + 2] ; // pt to pit object


		if ((iNumStones = xpcMove->m_iNumStones[iPlayer][iPit + 2]))
			// get number of stones and test
			++iNumMoves ;   // it's a legal move

		/*
		   determine the max # of stones across all pits on one side
		*/
		if (iNumStones > iMaxStones) iMaxStones = iNumStones;

		/*
		     test whether this pit yields a free turn
		*/
		iFree = (iNumStones && (iNumStones % (2 * NUMPITS + 1) == iPit + 1)) ? 1 : 0;


		/*
		   Test for a Capture and the amount of Capture.
		   There are 3 ways you can execute a capture:
		       if you have 13 stones in a pit, then you definitely capture all stones in the opposite pit +1 (which is your own stone).
		       if you have <13 stones in a pit and you don't wrap around the other side, then you might yield a capture.
		       if you have <13 stones in a pit then you  never cross the point you started from and you might yield a capture.

		   If You happen to cross your starting point as in a loop (if you have >13 stones) you will never land up w/ a capture.
		*/

		if (iNumStones == 2 * NUMPITS + 1) {
			iCapture = xpcMove->m_iNumStones[OTHERPLAYER(iPlayer)][NUMPITS + 1 - iPit];
			jYieldsACapture[iPit] = NUMPITS - 1 - iPit;
		} else if (iNumStones && (iNumStones <= iPit) && !xpcMove->m_iNumStones[iPlayer][iPit - iNumStones + 2]) {
			iCapture = xpcMove->m_iNumStones[OTHERPLAYER(iPlayer)][NUMPITS + 1 - iPit + iNumStones] ;
			jYieldsACapture[iPit] = NUMPITS - 1 - iPit + iNumStones;
		} else if (iNumStones > iPit + NUMPITS && (iNumStones <= 2 * NUMPITS) &&  !xpcMove->m_iNumStones[iPlayer][2 * NUMPITS + 3 + iPit - iNumStones]) {
			iCapture = xpcMove->m_iNumStones[OTHERPLAYER(iPlayer)][iNumStones - iPit - NUMPITS];
			jYieldsACapture[iPit] = iNumStones - iPit - NUMPITS - 2;
		} else {
			iCapture = 0;
			jYieldsACapture[iPit] = -1;
		}

		/*
		     (OTHER THINGS BEING THE SAME),  if there're more than one FreePit
		     then start from the one closest to the home bin
		*/
		if (iFree) {
			xpcMove->m_bHasFree = true ;
			if (!bAFreePitExists) iFreePit = iPit;                              //so that you start w/ the closest  free Pit (ie. iPit with the lowest index),
			if (iNumStones <= NUMPITS) bAFreePitExists = true; //unless there's a "WRAP AROUND" in which case the farthest pit will be selected.
		}

		/*
		   Determine which pit yields the max capture,
		   (OTHER THINGS BEING THE SAME),   start with this pit
		*/
		if (iCapture) {
			xpcMove->m_bHasCapture = true ;
			if (iCapture > maxCapture) {
				maxCapture = iCapture;
				iCapturePit = iPit;
			}
			bACaptureExists = true;
		}

	}//end- for iPit

	/*
	    Determine whether the FreePit "interferes" with the CapturePit.
	    There's NO interference if doing the free pit first keeps the capture intact.
	 */


	if (bACaptureExists && bAFreePitExists) {
		if ((xpcMove->m_iNumStones[iPlayer][iFreePit + 2] < NUMPITS - 1) &&        //i.e. if iFreePit <=3
		        (xpcMove->m_iNumStones[iPlayer][iCapturePit + 2] < NUMPITS + 1) &&  //i.e. No Wrap Around Capture
		        ((iCapturePit - xpcMove->m_iNumStones[iPlayer][iCapturePit + 2])  > iFreePit)) {   //i.e. the landing pit in the capture case, ...
			//  is  farther than the freePit.
			bDoFreePitFirst = true;
		}
	}

	/*
	    Use End Game Play Strategy if more than 2 stones are empty on each side.
	    In this case ALWAYS start with the pit which has stones falling furthest away
	    from home bin. if this pit is a free  pit avoid it, unless
	    by doing otherwise you have to squander more than 3 stones to the
	    opposite side then go for the free pit.
	*/

	if (bEndGamePlay) {
		if (maxCapture > 3) {
			xpcMove->m_iValues[iCapturePit] = 5;
		} else if (bAThreatExists) {
			xpcMove->m_iValues[iMaxThreatPit] = 5;
		} else {
			for (u = 0; u < NUMPITS; u++) {
				if (!xpcMove->m_iNumStones[iPlayer][u + 2]) continue;           //proceed in the current loop only if there are stones.

				tmpValues = u + 1 - xpcMove->m_iNumStones[iPlayer][u + 2]; //highest value to pit w/ stones falling furthest away from home bin.
				if (tmpValues <= 0)         //u is a free pit or has stones falling on the opp. side.
					tmpValues = BESTWINUNDEF + 1;           //the lowest possible; add 1 to distinguish with pits without stones.
				xpcMove->m_iValues[u] = tmpValues;
			}//end for u.
		}//end  else

		goto cleanup;
	}//end if bEndGamePlay

	/*
	   if there's no end of game play: (HIGHEST PRIORITY enumerated first)
	       1) if the DoFreePitFirst flag is high then do the Free Pit first
	       2) if there's a threat more than the max capture do the threat first.
	       3) if the smallest capture's more than the highest threat do the capture first.
	       4) if you can't execute 2) or 3) then do the threat if it's more than 3 first, else do the capture.
	       5) if your capture is less than 4 stones and you have a free pit too, do the free pit.
	       6) if you can't conclude from 1)-5) assign equal values to all.

	       This is the overall condensed strategy. The actual implementation is simpler
	       if you construct a binary conditions table (TCF for threat, capture, free) and
	       then implement subconditions if two or more of these flags are high, according to the above
	       rules.
	*/

	//T C F     stands for....(threatExists, CaptureExists, FreePitExists)
	if (!bAThreatExists && !bACaptureExists && bAFreePitExists) {                                                               // 0 0 1
		xpcMove->m_iValues[iFreePit] = 5;
		bConclusive = true;
	}

	if (!bAThreatExists && bACaptureExists && !bAFreePitExists) {                                                               // 0 1 0
		xpcMove->m_iValues[iCapturePit] = 5;
		bConclusive = true;
	}

	if (!bAThreatExists && bACaptureExists && bAFreePitExists) {                                                                // 0 1 1
		if (bDoFreePitFirst || (maxCapture < 3))                                                                                                                    //If the capture is less than 3 or if there's a "non-interfering"
			xpcMove->m_iValues[iFreePit] = 5;                                                                                                                       //free pit then do free pit first.
		else
			xpcMove->m_iValues[iCapturePit] = 5;

		bConclusive = true;
	}


	if (bAThreatExists && !bAFreePitExists && !bACaptureExists) {                                                              //1 0 0
		xpcMove->m_iValues[iMaxThreatPit] = 5;
		bConclusive = true;
	}

	if (bAThreatExists && !bACaptureExists && bAFreePitExists) {                                                                //1 0 1
		xpcMove->m_iValues[iFreePit] = 5;
		bConclusive = true;                                                                           // if no capture do free pit ---> no contest from Threat.
	}
	if (bAThreatExists && bACaptureExists) {
		if (!bAFreePitExists || (bAFreePitExists && !bDoFreePitFirst)) {                                                                //1 1 0 && 1 1 1
			if (maxCapture >= maxThreat) {
				xpcMove->m_iValues[iCapturePit] = 5;
			} else {
				/*
				see if you can capture the offending pit anyway :
				    if  you have more than CapturePit  Stones (ie. a wrap around capture).
				    you take care of the threat because you fill up
				    pits on the opp.side.   If this condition is true and maxThreat>5
				    then go for the capture. If that condition is true and but maxThreat<=5
				    then go for the threat.

				    if IT'S  not a wrap around capture, and the offending pits can be
				    captured (i.e. look at all capture pits and the pit numbers captured
				    from opp. side), then see if you have more than one offending pit for
				    the same threat, in which case go for the bigger offending pit capture...

				*/
				if (xpcMove->m_iNumStones[iPlayer][iCapturePit] > iCapturePit) {
					if (maxThreat > 4)     xpcMove->m_iValues[iCapturePit] = 5;
					else                        xpcMove->m_iValues[iMaxThreatPit] = 5;
				} else {

					/*
					    determine if an offensive pit is capturable. If atleast one pit offensive
					    to the maxThreatPit is uncapturable then go for the threat pit instead of the
					    capture.
					*/
					for (kk = 0, tmpCount = 0, bUnSaveable = false; kk < ThreatCount; kk++) {
						if (iPitUnderThreat[kk] == iMaxThreatPit) tmpCount++;
						if (tmpCount > 1) {
							bUnSaveable = true;
							break;
						}
					}
					/*
					go thru every pit and see if it can capture a pit.
					    If it can't go to next pit.
					    else see if the captured pit is offensive to one of the other pits.
					        If not go to next pit.
					        if yes, find out the number of stones gained (captured) and the number of
					                    stones saved (stones no longer threatened because of capture(s))
					*/

					for (ii = 0, bSaveable = false; !bUnSaveable && ii < NUMPITS; ii++) {                         // ii for this side.
						if (jYieldsACapture[ii] == -1) continue;            //skip the following lines if ii does not yield a capture.
						if ((iSaved = iOtherOffensivePit[jYieldsACapture[ii]]) == -1) {     //iSaved lies on this side.
							continue;                           //the captured pit is not offensive.
						} else {
							bSaveable = true;
							StonesSaved = xpcMove->m_iNumStones[iPlayer][2 + iSaved];   // no. of stones saved via capture.
							StonesGained = xpcMove->m_iNumStones[OtherPlayer][2 + jYieldsACapture[ii]]; //no. of stones gained via capture.
							TotalBenefit = StonesGained + StonesSaved;
							if (maxTotalBenefit < TotalBenefit) {
								maxTotalBenefit = TotalBenefit;
								iStartWithThisCapture = ii;
							}
						}  //end if iSaved
					}//end for ii.

					if (bSaveable && !bUnSaveable)
						xpcMove->m_iValues[iStartWithThisCapture] = 5;  //optimum capture
					else
						xpcMove->m_iValues[iMaxThreatPit] = 5;       //threat pit can't be saved.

				}//endif xpcMove->...
			}//endif maxCapture>=maxThreat
		} else {                                                                             //if bdoFreePitFirst
			xpcMove->m_iValues[iFreePit] = 5;
		}
		bConclusive = true;
	} //end if (bAThreatExists&&baCaptureExists)

	if ((!bAThreatExists && !bACaptureExists && !bAFreePitExists) || !bConclusive) {
		for (k = 0; k < NUMPITS; k++) {
			xpcMove->m_iValues[k] = xpcMove->m_iNumStones[iPlayer][k + 2] ? 5 : 0;  //assign equal values to all.
		}
	}//end 0 0 0

	xpcMove->m_iNumMoves = iNumMoves ;  // store # legal moves


cleanup:
	JXELEAVE(CMnk::DefensiveStaticEvaluation) ;
	RETURN(iError != 0) ;
}
/*

*/

bool  CMnk::TreeAlgo(CMove *xpcMove) {
	MOVE* pMoveArr[MAXMOVES],
	      *pOrigMove;
	HGLOBAL hMoveArr[MAXMOVES];
//	char cFinalID;
	short int   j,
	      k;

	for (j = 0; j < MAXMOVES; j++) {
		if (!(hMoveArr[j] = GlobalAlloc(GHND, sizeof(MOVE))))
			for (k = 0; k < j; k++) {
				GlobalFree(hMoveArr[k]);
			}
		return true;          //for error.
	}

	pMoveArr[0] = (MOVE*) GlobalLock(hMoveArr[0]);

	memcpy(&(pMoveArr[0]->iNumRocks[0]), &(xpcMove->m_iNumStones[0][2]), NUMPITS * sizeof(int));
	pMoveArr[0]->iRocksInHomeBin = xpcMove->m_iNumStones[0][1];

	pOrigMove = pMoveArr[0];

	ExtendedStaticEvaluation(pMoveArr[0], pOrigMove, 0, 0);
	return false;

}


int CMnk::ExtendedStaticEvaluation(MOVE *pMove, MOVE *pParentMove,  signed char cID, int iPlayer) {
	bool bWrapsAroundOnTop,
	     bWrapsAroundBehind;
//	     bWrapsAroundAhead;

	int iError = 0,        // error code
	    iNumMoves = 0,     // number of legal moves
	    iNumStones,        // number of stones in pit
	    iPit;      // loop variable
	//int iMaxValue = BESTWINUNDEF ;  // computing maximum value

	int iFreeTurn = 0,
	    iCapture,
	    iNextID,
	    j,
	    iStopPit = 0;


	/*initialize*/
	pMove->iParentID = pParentMove->iMoveID;
	pMove->iMoveID = cID;


	for (iPit = 0 ; iPit < NUMPITS ; ++iPit) {

		if ((iNumStones = pMove->iNumRocks[iPit]))    //  if there are stones in current pit,
			++iNumMoves;                                                                               //  ...(iNumStones being the #of stones in the current pit),
		// ... it's a legal move

		/* If  this pit yields a free turn ...*/
		if ((iNumStones && (iNumStones % (2 * NUMPITS + 1) == (iPit + 1)))) {
			iFreeTurn = iNumStones;
			pMove->iFree[iPit][pMove->cRecursion] = iNumStones;
			pMove->iRocksInHomeBin  = pParentMove->iRocksInHomeBin + 1;
			pMove->iPitSequence[pMove->cRecursion] = iPit;
			pMove->iNumRocks[iPit] = 0;

			for (j = 0; j < iPit; j++, pMove->iNumRocks[j]++);

			pMove->cRecursion++;
			ExtendedStaticEvaluation(*(&pMove + 4), pMove, cID, iPlayer);
			break;
		}

		//bWrapsAroundAhead = iNumStones > (2 * NUMPITS + 1);
		bWrapsAroundOnTop = iNumStones == (2 * NUMPITS + 1);
		bWrapsAroundBehind = (iNumStones < (2 * NUMPITS + 1)) && (iNumStones > (iPit + NUMPITS));


		/*
		    if  you have (2*NUMPITS+1)(=13) pebbles in a pit, 'a' of yours, you are definitely capturing a pebble
		    on the pit 'b' EXACTLY opposite 'a';
		    a and b are related by the expression
		    a+b=(NUMPITS-1), in regular logic, OR by:
		    a+b=(NUMPITS-1)+4, when a and b, (both) are INDEXES into the m_iNumStones array, as in the case below.
		*/
		if (bWrapsAroundOnTop) {
			iCapture = pMove->iNumRocks[NUMPITS + 3 - iPit] ;
		} else {
			/*
			    you have captured pebbles from the other player's
			     pit#   (NUMPITS+1-(iPit-iNumStones+2)),
			     but REMEMBER to add 2 to this before indexing
			*/
			if (iNumStones && (iNumStones <= iPit) && !(pMove->iNumRocks[iPit - iNumStones])) {
				iStopPit = iPit - iNumStones;
				iCapture = pMove->iNumRocksOpposite[iStopPit];
			} else {
				/*
				    if you wrap around and end up with  a vacant pit
				    to the left of where you started from, you have hauled
				    a capture of  the number of stones in the other player's
				    pit# (NUMPITS+1-(2*NUMPITS+3+iPit-iNumStones))
				*/
				if (bWrapsAroundBehind && !(pMove->iNumRocks[2 * NUMPITS + 1 + iPit - iNumStones])) {
					iStopPit = 2 * NUMPITS + 1 + iPit - iNumStones;
					iCapture = pMove->iNumRocksOpposite[iNumStones - NUMPITS - iPit - 4];
				} else {
					iCapture = 0;       //no capture otherwise.
				}
			}//end else, if (iNumStones && ...)
		}   //end else, if (NumStones==...)

		if (iCapture) {
			pMove->iCapture[iPit][pMove->cRecursion] = iCapture;
			pMove->iRocksInHomeBin  = pParentMove->iRocksInHomeBin + iCapture;

			memcpy(&(pMove->iPitSequence[0]), &(pParentMove->iPitSequence[0]), pMove->cRecursion * sizeof(int));
			pMove->iPitSequence[pMove->cRecursion] = iPit;
			pMove->iNumRocks[iPit] = 0;

			for (j = iStopPit; j < iPit; j++, pMove->iNumRocks[j]++);        //executed only if StopPit<jPit i.e. if NoWrapAround.

			if (bWrapsAroundBehind) for (j = iStopPit; j < NUMPITS; j++, pMove->iNumRocks[j]++) ;


			iNextID = ((iPit == (NUMPITS - 1)) && !iFreeTurn) ? -1 : (pMove->cMoveID + 1);  //returns the Next ID.
			if (iNextID == -1) return -1;

		}


	}//end -for (iPit)

	(void)iNumMoves;

	RETURN(iError != 0) ;
}
/*

#define FREE    (0x01<<4)
#define CAPTURE (0x02<<4)
#define THREAT  (0x03<<4)
#define NEITHER (0x00<<4)
#define F FREE
#define C   CAPTURE
#define T   THREAT
#define N   NEITHER
#define MAKEWORD(L, H)          ((L)||(H<<8))
#define CreateMoveChainWord(Gain, Flag, Pit)    (MAKEWORD((Flag) ||(Pit), Gain))

uint16 Type(byte pit, CONFIG* pStoneCfg){

//returns F/C/T/N in the lowbyte    and the amount of gain/save in the high byte.
}

bool CMnk::LookAheadEvaluation(CONFIG* pStoneConfig, int iPlayer,   uint16* pwMoveChain){
    static bool bLocked;
    byte Pit;

    for(Pit=0; Pit<NUMPITS; Pit++){
        switch(LOBYTE(u=Type(iPit, pStoneConfig))){
            case FREE:
                _wordstrcat(pwMoveChain,CreateMoveChainWord(0x01,F, Pit));
                LookAheadEvaluation(pStoneConfig, iPlayer, pwMoveChain);
                break;
            case CAPTURE:
                _wordstrcat(pwMoveChain,CreateMoveChainWord(HIBYTE(u),C, Pit));
                len= _wordstrlen(pwMoveChain);
                bLocked[m_iChainID++]=true;
                ptmpMoveChain=pwMoveChain;
                pwMoveChain=m_pwMoveChainList[m_iChainID];
                _wordstrcpy(pwMoveChain, StepBackwards(ptmpMoveChain));
                LookAheadEvaluation(pStoneConfig,iPlayer, pwMoveChain);
                break;
            default:
            case NEITHER:
                break;
        }
    }
}


*/













//* CMnk::CountStones -- count total stones in configuration
bool CMnk::CountStones(CMove * xpcMove)
// xpcMove -- CMove object containing the configuration
// returns: true if error, false otherwise
{
	JXENTER(CMnk::CountStones) ;
	int iError = 0 ;        // error code
	//long lConfigIndex = 0 ; // return value, configuration index
	int iStones ;       // total number of stones
	int iPlayer, iPit ;     // loop variables

	for (iStones = 0, iPlayer = 0 ; iPlayer < NUMPLAYERS ; ++iPlayer)
		for (iPit = 0 ; iPit < NUMPITS ; ++iPit)
			iStones += xpcMove->m_iNumStones[iPlayer][iPit + 2] ;
	// count total number of stones
	xpcMove->m_iTotalStones = iStones ; // store it

// cleanup:

	JXELEAVE(CMnk::CountStones) ;
	RETURN(iError != 0) ;
}

//* CMnk::GetBestWinCount -- get position value in best win table
bool CMnk::GetBestWinCount(CMove * xpcMove)
// xpcMove -- CMove object for position to be evaluated
// returns: true if error, false otherwise
{
	JXENTER(CMnk::GetBestWinCount) ;
	int iError = 0 ;        // error code
	long lIndex = xpcMove->m_lConfigIndex ;
	int iValue = 0;        // value from table
	struct FIVE * hpFive ; // ptr to structure of 8 5-bit values

	if (lIndex < 0 || lIndex >= MAXCONFIGS) {
		iError = 100 ;      // index out of range
		goto cleanup ;
	}

	hpFive = (struct FIVE *)(m_lpCMnkData->m_hpcBestWin
	                         + (lIndex / 8) * 5) ;
	// point to group of eight numbers

	switch (lIndex % 8) {
	case 0:
		iValue = hpFive->v0 ;
		break ;

	case 1:
//      iValue = hpFive->v1 ;
		iValue = (hpFive->v1a << 2) + (hpFive->v1b) ;
		break ;

	case 2:
		iValue = hpFive->v2 ;
		break ;

	case 3:
//      iValue = hpFive->v3 ;
		iValue = (hpFive->v3a << 4) + (hpFive->v3b) ;
		break ;

	case 4:
//      iValue = hpFive->v4 ;
		iValue = (hpFive->v4a << 1) + (hpFive->v4b) ;
		break ;

	case 5:
		iValue = hpFive->v5 ;
		break ;

	case 6:
//      iValue = hpFive->v6 ;
		iValue = (hpFive->v6a << 3) + (hpFive->v6b) ;
		break ;

	case 7:
		iValue = hpFive->v7 ;
		break ;
	}

	if (iValue == TABLEUNDEF)       // undefined or unspecified
		iValue = BESTWINUNDEF ;

	else if (iValue > 15)       // negative number
		iValue -= 32 ;

	xpcMove->m_iBestWinValue = iValue ;
//
//
//  xpcMove->m_iBestWinValue = (iValue == TABLEUNDEF) ?
//          BESTWINUNDEF : iValue ;    // store value

cleanup:

	JXELEAVE(CMnk::GetBestWinCount) ;
	RETURN(iError != 0) ;
}

//* CMnk::SetBestWinCount -- set value in best win table
bool CMnk::SetBestWinCount(CMove * xpcMove)
// xpcMove -- pointer to CMove object where value is to be set
// returns: true if error, false otherwise
{
	JXENTER(CMnk::SetBestWinCount) ;
	int iError = 0 ;        // error code
	struct FIVE * hpFive ; // ptr to structure of 8 5-bit values
	long lIndex = xpcMove->m_lConfigIndex ;
	int iValue = xpcMove->m_iBestWinValue ; // value from table
	bool bTest = false ;        // debugging test

	if (lIndex < 0 || lIndex >= MAXCONFIGS) {
		iError = 100 ;      // index out of range
		goto cleanup ;
	}

	hpFive = (struct FIVE *)(m_lpCMnkData->m_hpcBestWin
	                         + (lIndex / 8) * 5) ;
	// point to group of eight numbers

	switch (lIndex % 8) {
	case 0:
		hpFive->v0 = iValue ;
		break ;

	case 1:
//      hpFive->v1 = iValue ;
		hpFive->v1a = iValue >> 2 ;
		hpFive->v1b = iValue ;
		break ;

	case 2:
		hpFive->v2 = iValue ;
		break ;

	case 3:
//      hpFive->v3 = iValue ;
		hpFive->v3a = iValue >> 4 ;
		hpFive->v3b = iValue ;
		break ;

	case 4:
//      hpFive->v4 = iValue ;
		hpFive->v4a = iValue >> 1 ;
		hpFive->v4b = iValue ;
		break ;

	case 5:
		hpFive->v5 = iValue ;
		break ;

	case 6:
//      hpFive->v6 = iValue ;
		hpFive->v6a = iValue >> 3 ;
		hpFive->v6b = iValue ;
		break ;

	case 7:
		hpFive->v7 = iValue ;
		break ;
	}

cleanup:

	if (m_bDumpPopulate && (lIndex % 10000 == 0
	                        || (bTest = iValue > xpcMove->m_iTotalStones
	                                    || -iValue > xpcMove->m_iTotalStones))) {
		DumpPosition(xpcMove) ;
		if (bTest) {
			DumpBestWinTable() ;
			DebugBreak() ;
		}
		DoPendingEvents() ;
	}

//#define THRESHHOLD 20000L
//#define BUFSIZE 10000
//
//  if (lIndex >= THRESHHOLD)
//  {
//  static byte FAR cBuffer[BUFSIZE] ;
//
//  if (lIndex == THRESHHOLD)
//  {
//      _fmemcpy(cBuffer, (char *)m_lpCMnkData->m_hpcBestWin, BUFSIZE) ;
//      DumpBestWinTable() ;
//  }
/////   else if (lIndex >= 104856   // error detected at 104858
//  else if (_fmemcmp(cBuffer, (char *)m_lpCMnkData->m_hpcBestWin,
//          BUFSIZE))   // if first BUFSIZE bytes changed
//  {
//      DumpPosition(xpcMove) ;
//      DumpBestWinTable() ;
//      DebugBreak() ;
//  }
//  }


	JXELEAVE(CMnk::SetBestWinCount) ;
	RETURN(iError != 0) ;
}

//* CMnk::DumpPosition -- dump contents of CMove object
bool CMnk::DumpPosition(CMove * xpcMove)
// xpcMove -- CMove object containing the configuration
// returns: true if error, false otherwise
{
	JXENTER(CMnk::DumpPosition) ;
	int iError = 0 ;        // error code

	if (xpcMove->m_bRealMove)
		debugN("(REAL) ");

	debug("P%d Cfg=%ld w/%d stones, %d moves, sow=%d, "
	                              "best %d for %d.",
	                  xpcMove->m_iPlayer, xpcMove->m_lConfigIndex,
	                  xpcMove->m_iTotalStones, xpcMove->m_iNumMoves,
	                  (xpcMove->m_xpcPit ? xpcMove->m_xpcPit->m_iPit : -1),
	                  xpcMove->m_iBestMove, xpcMove->m_iBestWinValue);


	debug("               [%3d] %2d %2d %2d %2d %2d %2d              H=%d",
	                  xpcMove->m_iNumStones[1][HOMEINDEX + 2],
	                  xpcMove->m_iNumStones[1][2], xpcMove->m_iNumStones[1][3],
	                  xpcMove->m_iNumStones[1][4], xpcMove->m_iNumStones[1][5],
	                  xpcMove->m_iNumStones[1][6], xpcMove->m_iNumStones[1][7],
	                  xpcMove->m_iNumStones[1][HANDINDEX + 2]);

	debug("                     %2d %2d %2d %2d %2d %2d [%3d]        H=%d\n",
	                  xpcMove->m_iNumStones[0][7], xpcMove->m_iNumStones[0][6],
	                  xpcMove->m_iNumStones[0][5], xpcMove->m_iNumStones[0][4],
	                  xpcMove->m_iNumStones[0][3], xpcMove->m_iNumStones[0][2],
	                  xpcMove->m_iNumStones[0][HOMEINDEX + 2],
	                  xpcMove->m_iNumStones[0][HANDINDEX + 2]);
	DoPendingEvents() ;

// cleanup:

	JXELEAVE(CMnk::DumpPosition) ;
	RETURN(iError != 0) ;
}

//* CMnk::DumpBestWinTable -- dump fields of best win table
bool CMnk::DumpBestWinTable(long lLow,
                            long lHigh) {
// lLow -- low end of configuration range
// lHigh -- high end of configuration range
// returns: true if error, false otherwise
	JXENTER(CMnk::DumpBestWinTable) ;
	int iError = 0 ;        // error code
	CMove cMove ;       // dummy move structure
	long lEol ;     // config index at end of line

	for (lEol = lLow + 23 ; lEol <= lHigh + 23 ; lEol += 24) {
		if (lEol > lHigh)
			lEol = lHigh ;
		debugN("Table[%ld-%ld]:", lLow, lEol) ;

		while (lLow <= lEol) {      // loop thru values
			cMove.m_lConfigIndex = lLow++ ;
			GetBestWinCount(&cMove) ;
			if (cMove.m_iBestWinValue == BESTWINUNDEF)
				debugN(" U") ;
			else
				debugN(" %d", cMove.m_iBestWinValue) ;
		}
		debugN("\n") ;
	}
	debugN("\n") ;

// cleanup:

	JXELEAVE(CMnk::DumpBestWinTable) ;
	RETURN(iError != 0) ;
}

} // namespace Mankala
} // namespace HodjNPodj
} // namespace Bagel
