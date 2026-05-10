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
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/mankala/mnk.h"

namespace Bagel {
namespace HodjNPodj {
namespace Mankala {

extern  bool gbTurnSoundsOff;
extern LPGAMESTRUCT pGameParams;

static int gCount;     //reset to 0 in StartGame() and incremented in AcceptClick to keep track of # of Turns.



//* CMnkWindow::StartGame -- start a new game
bool CMnkWindow::StartGame()
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::StartGame) ;

	int iError = 0 ;        // error code
	gCount = 0;             // this count keeps track of the # of turns of each player so far,
	// and is used


	FreePitResources() ;    // free resources from previous game

//    ((CMnk *)this)->InitMankala() ;   // initialize mankala game
	CMnk::InitMankala() ;   // initialize mankala game logic
	Invalidate(false) ; // invalidate the entire window, so it
	// will be repainted

	m_bStartGame = m_bGameOver = false ;
	m_bGameOverMusicPlayed = false;
	m_bScoresDisplayed = false;
	SetCrabSign(false) ;
	UpdateWindow();

	if (!gbTurnSoundsOff && pGameParams->bSoundEffectsEnabled)
		if (!sndPlaySound(YOUGO3, SND_SYNC)) {
			MessageBox("Can't Play Wave Sound");
		}

	JXELEAVE(CMnkWindow::StartGame) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::PaintBitmapObject -- paint bitmap
bool CMnkWindow::PaintBitmapObject(CBmpObject * xpcBmpObject,
                                   int iBmpType, int iBmpArg)
// xpcBmpObject -- pointer to bitmap object
// iBmpType -- BMT_xxxx -- type of bitmap object
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::PaintBitmapObject) ;

	int iError = 0 ;        // error code
	bool bNew = !xpcBmpObject->m_bInit ;    // needs to be initialized
	CRect cBmpRect, cDestRect ; // bitmap/destination rectangles
	CDC *pDC ;

	pDC = GetDC();

	MSG msg;
	if (MFC::PeekMessage(&msg, nullptr, MM_MCINOTIFY, MM_MCINOTIFY, PM_REMOVE)) {
		MFC::TranslateMessage(&msg);
		MFC::DispatchMessage(&msg);
	}

	if (xpcBmpObject->m_iBmpType == 0)  // if no type yet inserted
		xpcBmpObject->m_iBmpType = iBmpType ;   // store type
	else if (iBmpType > 0 && xpcBmpObject->m_iBmpType != iBmpType)
		// if we're changing types
	{
		iError = 100 ;      // invalid type change
		goto cleanup ;
	}

	if (!xpcBmpObject->m_iBmpType) {
		iError = 101 ;      // no type specified
		goto cleanup ;
	}

	if (!xpcBmpObject->m_bChained) { // if not yet on chain to be freed
		xpcBmpObject->m_xpcNextFree = m_xpcBmpFreeChain ;
		m_xpcBmpFreeChain = xpcBmpObject ;
		xpcBmpObject->m_bChained = true ;
	}

	if (iBmpArg)
		xpcBmpObject->m_iBmpArg = iBmpArg ;

	if (bNew)   // if not initialized
		InitBitmapObject(xpcBmpObject) ;    // load bitmap

	if (bNew || xpcBmpObject->m_bSprite)
		// if this is new, or if this is a sprite,
		SetBitmapCoordinates(xpcBmpObject) ;    // set position

	if (!xpcBmpObject->m_bDummy) {  // if not a dummy bitmap
		cBmpRect = CRect(CPoint(0, 0), xpcBmpObject->m_cSize) ;
		cDestRect = CRect(xpcBmpObject->m_cPosition, xpcBmpObject->m_cSize) ;

		if (xpcBmpObject->m_bSprite) {
			if (!m_bJustStarted) {
				xpcBmpObject->m_xpcSprite->ClearBackground() ;
				// clear the save background for the stone
				// sprite, forcing RefreshSprite to fetch/save
				// the background it's currently covering
				xpcBmpObject->m_xpcSprite->
				SetPosition(xpcBmpObject->m_cPosition.x,
				            xpcBmpObject->m_cPosition.y) ;


				xpcBmpObject->m_xpcSprite->RefreshSprite(pDC) ;
				// display the stone sprite, saving
				// current background image
			}
		} else if (xpcBmpObject->m_bMasked) {
			PaintMaskedDIB(pDC, m_xpGamePalette,
			               xpcBmpObject->m_xpDibDoc,
			               xpcBmpObject->m_cPosition.x,
			               xpcBmpObject->m_cPosition.y,
			               xpcBmpObject->m_cSize.cx,
			               xpcBmpObject->m_cSize.cy) ;
		}

		else        // there's a DIB
			PaintDIB(pDC->m_hDC, &cDestRect,
			         xpcBmpObject->m_xpDibDoc->GetHDIB(),
			         &cBmpRect, m_xpGamePalette) ;
		// transfer the bitmap to the screen
	}
	xpcBmpObject->m_bInit = true ;  // object is initialized

cleanup:
	if (pDC) {
		ReleaseDC(pDC);
		pDC = nullptr;
	}

	JXELEAVE(CMnkWindow::PaintBitmapObject) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::InitBitmapObject -- set up DibDoc in bitmap object
bool CMnkWindow::InitBitmapObject(CBmpObject * xpcBmpObject)
// xpcBmpObject -- pointer to bitmap object
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::InitBitmapObject) ;

	int iError = 0 ;        // error code
//    CPitWnd * xpcPit = xpcBmpObject->m_xpcPit ;
	CSprite * xpcSprite ;
	HDIB hDib ;


	if (xpcBmpObject->m_bSprite) {
		if (!(xpcSprite = xpcBmpObject->m_xpcSprite
		                  = new CSprite)) { // alloc sprite and test
			iError = 100 ;      // can't allocate sprite
			goto cleanup ;
		}

		xpcSprite->SharePalette(m_xpGamePalette) ;
		// set its palette to be the same as the game's
		xpcBmpObject->m_xpcSprite = xpcSprite ;
		if ((iError = InitBitmapFilename(xpcBmpObject)))
			// initialize correct filename and do LoadSprite
			goto cleanup ;


		xpcSprite->SetMasked(true) ;
		// it is a masked image (white/transparent background)
		xpcSprite->SetOptimizeSpeed(false) ;
		// keep its bitmap inside a device context
		xpcSprite->SetMobile(true) ;  // it will move around the screen
//  xpcSprite->LinkSprite() ;
//              // link stone sprite into sprite chain
		xpcBmpObject->m_cSize = xpcSprite->GetSize() ;
		// now establish its "hotspot" as its center point
		xpcSprite->SetHotspot(xpcBmpObject->m_cSize.cx >> 1,
		                      xpcBmpObject->m_cSize.cy >> 1) ;

	}

	else {          // it's a DIB
		if (!xpcBmpObject->m_xpDibDoc) { // if no DIB document allocated
			if (!(xpcBmpObject->m_xpDibDoc = new CDibDoc()))
				// allocate DIB and test
			{
				iError = 100 ;      // can't allocate DIB
				goto cleanup ;
			}

			if ((iError = InitBitmapFilename(xpcBmpObject)))
				goto cleanup ;
		}

		if (!xpcBmpObject->m_bDummy && !xpcBmpObject->m_lpDib) {
			if (!(hDib = xpcBmpObject->m_xpDibDoc->GetHDIB()))
				// get DIB memory handle and test
			{
				iError = 103 ;  // no DIB memory handle
				goto cleanup ;
			}

			xpcBmpObject->m_lpDib = hDib;
			xpcBmpObject->m_cSize = CSize(
				(int)DIBWidth(xpcBmpObject->m_lpDib),
				(int)DIBHeight(xpcBmpObject->m_lpDib));
		}
	}

cleanup:
	JXELEAVE(CMnkWindow::InitBitmapObject) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::InitBitmapFilename -- set up filename bitmap object
bool CMnkWindow::InitBitmapFilename(CBmpObject * xpcBmpObject)
// xpcBmpObject -- pointer to bitmap object
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::InitBitmapFilename) ;


	CDC *pDC = GetDC();

	static class CBmpTable cBmpTable[] = {
		{BMT_MAIN, -1, 1, "mankala.bmp", false, false},
		{BMT_SCROLL, -1, 1, "scrollup.bmp", false, true},           //scrolbtn.bmp
		{BMT_STONE, -1, 4, "mans%d.bmp", false, true},
		{BMT_PIT, 0, 1, "manp%d%d.bmp", true, true},
		{BMT_PIT, 1, 4, nullptr, true, true},
		{BMT_PIT, 2, 6, nullptr, true, true},
		{BMT_PIT, 3, 8, nullptr, true, true},
		{BMT_PIT, 4, 8, nullptr, true, true},
		{BMT_PIT, 5, 5, nullptr, true, true},
		{BMT_PIT, 6, 4, nullptr, true, true},
		{BMT_PIT, 7, 4, nullptr, true, true},
		{BMT_PIT, 8, 4, nullptr, true, true},
		{BMT_PIT, -1, 2, "manpm%d.bmp", false, true},
		{BMT_LEFTBIN, 22, 1, "manl%02d.bmp", true, true},
		{BMT_LEFTBIN, -1, 1, "manlm.bmp", false, true},
		{BMT_RIGHTBIN, 22, 1, "manr%02d.bmp", true, true},
		{BMT_RIGHTBIN, -1, 1, "manrm.bmp", false, true},
		{BMT_HAND, -1, 0, nullptr, false, true},
		{BMT_SIGN, SBT_CRAB, 1, "mancrab.bmp", false, false},
		{BMT_SIGN, SBT_MYTURN, 1, "mancm.bmp", false, true},
		{BMT_SIGN, SBT_YOURTURN, 1, "mancy.bmp", false, true},
		{BMT_SIGN, SBT_TIE, 1, "manctie.bmp", false, true},
		{BMT_SIGN, SBT_IWIN, 1, "manciwin.bmp", false, true},
		{BMT_SIGN, SBT_YOUWIN, 1, "mancuwin.bmp", false, true},
		{0, 0, 0, nullptr, false, true}
	} ;

	int iError = 0 ;        // error code
	CBmpTable * xpBmpTable ;
	bool bFound = false ;
	const char *xpszFilenameString = nullptr ;
	char szPath[200] = {0} ;    // bitmap file path
	CPitWnd * xpcPit;
	int iNumStones;

	xpcPit = xpcBmpObject->m_xpcPit ;
	iNumStones = xpcPit ? xpcPit->m_iNumStones
	             : xpcBmpObject->m_iBmpArg ;
	// # stones in pit or bin

	Common::strcpy_s(szPath, m_szDataDirectory) ;
	for (xpBmpTable = cBmpTable ; !bFound &&
	        xpBmpTable->m_iBmpType ;
	        bFound || ++xpBmpTable)
		if (xpcBmpObject->m_iBmpType == xpBmpTable->m_iBmpType) {
			if (xpBmpTable->m_xpszFilenameString)
				xpszFilenameString =
				    xpBmpTable->m_xpszFilenameString ;
			if (xpBmpTable->m_iNumStones < 0 ||
			        iNumStones <= xpBmpTable->m_iNumStones)
				bFound = true ;
		}

	if (!bFound) {
		iError = 101 ;      // can't find bmp file
		goto cleanup ;
	}

	xpcBmpObject->m_bDummy = (xpBmpTable->m_iNumBmps < 1) ;
	// test if this is a dummy object
	xpcBmpObject->m_bMasked = xpBmpTable->m_bMasked ;

	if (xpBmpTable->m_iNumBmps >= 1) {
		xpcBmpObject->m_iBmpNum = m_cGenUtil.RandomInteger(1,
		                          xpBmpTable->m_iNumBmps) ;
		// choose a random file

		Common::sprintf_s(szPath + strlen(szPath),
		                  200 - strlen(szPath),
		                  xpszFilenameString,
		                  xpBmpTable->m_bSubNumStones ? iNumStones
		                  : xpcBmpObject->m_iBmpNum,
		                  xpcBmpObject->m_iBmpNum) ;

		if (xpcBmpObject->m_bSprite) {      // if this is a sprite
			if (!xpcBmpObject->m_xpcSprite->LoadSprite(pDC, szPath)) {
				iError = 102 ;  // can't load bitmap from disk
				goto cleanup ;
			}
		} else {    // DIB
			// next load in the bitmap segment and test
			if (!xpcBmpObject->m_xpDibDoc->OpenDocument(szPath)) {
				iError = 103 ;
				goto cleanup ;
			}
		}
	}

cleanup:
	if (pDC) {
		ReleaseDC(pDC);
		pDC = nullptr;
	}

	JXELEAVE(CMnkWindow::InitBitmapFilename) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::SetBitmapCoordinates -- set coordinates of bitmap
bool CMnkWindow::SetBitmapCoordinates(
    CBmpObject * xpcBmpObject)
// returns: true if error, false otherwise
{


	JXENTER(CMnkWindow::SetBitmapCoordinates) ;
	int iError = 0 ;        // error code
	CPitWnd * xpcPit = xpcBmpObject->m_xpcPit ;
	//int iStoneNum = xpcBmpObject->m_iStoneNum ; // stone number
	// for stone type
	int iBmpType ;
	int iX, iY ;

	if ((iBmpType = xpcBmpObject->m_iBmpType) == BMT_STONE && xpcPit) {
		if (xpcPit->m_iPit == HOMEINDEX)
			iBmpType = (xpcPit->m_iPlayer == 0) ? BMT_RIGHTBIN
			           : BMT_LEFTBIN ;
		else if (xpcPit->m_iPit == HANDINDEX)
			iBmpType = BMT_HAND ;

		else
			iBmpType = BMT_PIT ;
	}

	switch (iBmpType) {
	case BMT_MAIN:
		iX = iY = 0 ;
		break ;

	case BMT_SCROLL:
		iX = SCROLL_BUTTON_X, iY = SCROLL_BUTTON_Y ;
		break ;

	case BMT_LEFTBIN:
		iX = 38, iY = 253 ;
		break ;

	case BMT_RIGHTBIN:
		iX = 531, iY = 278 ;
		break ;

	case BMT_PIT:
		if (xpcPit->m_iPlayer == 0)
			iX = 464 - 72 * xpcPit->m_iPit, iY = 323 ;
		else
			iX = 114 + 68 * xpcPit->m_iPit, iY = 241 ;
		break ;

	case BMT_SIGN:
		switch (xpcBmpObject->m_iBmpArg) {
		case SBT_CRAB:
			iX = 255, iY = 148 ;
			break ;

		case SBT_MYTURN:
		case SBT_IWIN:
			iX = 307, iY = 152 ;
			break ;

		case SBT_YOURTURN:
		case SBT_YOUWIN:
		case SBT_TIE:
			iX = 255, iY = 151 ;
			break ;

		default:
			iError = 100 ;
			goto cleanup ;
			// break ;
		}
		break ;

	case BMT_HAND:
		if (xpcPit->m_iPlayer == 0)
			iX = 496 - 6 * xpcBmpObject->m_iStoneNum, iY = 430 ;
		else
			iX = 371 + 6 * xpcBmpObject->m_iStoneNum, iY = 227 ;
		break ;

	default:
		iError = 101 ;
		goto cleanup ;
		// break ;
	}

	if (xpcBmpObject->m_iBmpType == BMT_STONE && iBmpType  != BMT_HAND)
		iX += xpcBmpObject->m_xpcPit->m_cBmpObject.m_cSize.cx / 2,
		      iY += xpcBmpObject->m_xpcPit->m_cBmpObject.m_cSize.cy / 2 ;

	xpcBmpObject->m_cPosition = CPoint(iX, iY) ;

cleanup:
	JXELEAVE(CMnkWindow::SetBitmapCoordinates) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::AcceptClick -- process a mouse click by user
bool CMnkWindow::AcceptClick(CPoint cClickPoint)
// cClickPoint -- position of mouse when click (button up) occurred
// returns: true if error, false otherwise
{

	JXENTER(CMnkWindow::AcceptClick) ;



	int iError = 0 ;        // error code
	static bool bActive = false ;   // prevent recursion
	int iPlayer, iPit ;
	bool bFound = false ;       // found pit clicked on
	CBmpObject * xpcBmpObject ;     // bitmap object clicked on
	CPitWnd * xpcPit = nullptr;     // pit clicked on
	CMove *xpcMove = &m_cCurrentMove ; // current move/position
	//bool    bPlayerSwitched;
	MSG msg;


	if (bActive)        // prevent recursive call
		goto exit ;

	while (MFC::PeekMessage(&msg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)); //flush out pending mouse clicks

	bActive = true ;
	//bPlayerSwitched = false;

	if (CRect(m_cBmpScroll.m_cPosition,
	          m_cBmpScroll.m_cSize).PtInRect(cClickPoint)) {
		if (!m_bInMenu)
			OptionsDialog() ;
	} else {

		iPlayer = xpcMove->m_iPlayer ;  // only permit user to
		// click on current player's pits
		for (iPit = 0 ; !m_bGameOver &&
		        !bFound && iPit < NUMPITS ; bFound || ++iPit) {
			xpcPit = m_xpcPits[iPlayer][iPit + 2] ; // point to pit
			xpcBmpObject = &xpcPit->m_cBmpObject ;
			if (CRect(xpcBmpObject->m_cPosition, xpcBmpObject->m_cSize).PtInRect(cClickPoint))
				bFound = true ;
		}

		if (bFound) {
			if (CMnk::Move((CPit *)xpcPit))
				// if found, make the move, test for error
			{
				iError = 100 ;
				goto cleanup ;
			}
			if (xpcMove->m_iPlayer) gCount++;
			// increment count only if it's now the computer player, so it's turn based. (Move toggles turns internally)
			if (!(gCount < 5 || (gCount % 4 == 0)) && (gCount < 15)) //Play Igo, YouGo,etc. crap only ...
				// when for the first 4 turns of each player and
				//thereafter every 4th turn until it's 28 turns in all and never thereafter.
				gbTurnSoundsOff = true;     // "Just Shut up, Crab".
			else
				gbTurnSoundsOff = false;
		}
		SetCrabSign() ;


		// next loop as long as it's the computer's move
		while (!m_bGameOver && m_bComputer[xpcMove->m_iPlayer]
		        && !((CMnk *)this)->SearchMove(xpcMove, iPit))
			// loop as long as game is not over, it's the computer's
			// move, and SearchMove succeeds in finding a move
		{
			xpcPit = m_xpcPits[xpcMove->m_iPlayer][iPit + 2] ; // pt to pit object

			if (((CMnk *)this)->Move((CPit *)xpcPit))
				// make the move, test for error
			{
				iError = 110 ;
				goto cleanup ;
			}
		}

		SetCrabSign() ;

	}// end else (if CRect(m_cBmpScroll...)

cleanup:
	bActive = false ;

exit:
	JXELEAVE(CMnkWindow::AcceptClick) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::MoveStoneDisplay -- move a stone from pit to another
bool CMnkWindow::MoveStoneDisplay(CPitWnd * xpcFromPit,
                                  CPitWnd * xpcToPit)
// xpcFromPit -- source pit (where stone comes from)
// xpcToPit -- target pit (where stone goes)
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::MoveStoneDisplay) ;
	int iError = 0 ;        // error code
	CPoint cOldPosition, cNewPosition ; // old/new positions of stone
	CBmpObject * xpcStone ;     // stone being moved
//  CBmpObject cBmpObject ; // bitmap object for stone
	int iXDiff, iYDiff ;    // coordinate differences
	int iK ;        // loop variable
	CDC *pDC = GetDC();

	if ((xpcStone = xpcFromPit->m_xpcStoneChain))   // get last
		// stone from pit stone chain, if any
	{
		xpcFromPit->m_xpcStoneChain = xpcStone->m_xpcNext ;
		// remove stone from "from" pit chain
		xpcStone->m_xpcSprite->RefreshBackground(pDC) ;
		// erase the stone
//  OutputDebugString("Stone from pit chain.\n") ;
	} else if ((xpcStone = m_xpFreeStoneChain)) {
		m_xpFreeStoneChain = xpcStone->m_xpcNext ;
//  OutputDebugString("Stone from free chain.\n") ;
	}

	else if (!(xpcStone = new CBmpObject)) {
		iError = 110 ;  // stone allocation failed
		goto cleanup ;
	} else {    // stone allocation succeeded
		xpcStone->m_bHeap = true ;  // stone is on the heap
		xpcStone->m_bSprite = true ;    // it's a sprite
//  OutputDebugString("New stone allocated.\n") ;
	}

	xpcStone->m_xpcNext = nullptr ;
	xpcStone->m_xpcPit = xpcFromPit ;   // set stone's pit ptr
	xpcStone->m_iStoneNum = --xpcFromPit->m_iNumStones ;
	// store stone number

	AdjustPitDisplay(xpcFromPit, true) ;
	// adjust the "from" pit

	PaintBitmapObject(xpcStone, BMT_STONE) ;    // paint stone

	cOldPosition = xpcStone->m_cPosition ; // save stone start position


	xpcStone->m_xpcPit = xpcToPit ;  // now move to the "to" pit
	xpcStone->m_iStoneNum = xpcToPit->m_iNumStones++ ;
	// increment number of stones,
	// and make that new stone number
	SetBitmapCoordinates(xpcStone) ;    // get coordinates
	// of stone at position in new pit
	cNewPosition = xpcStone->m_cPosition ;  // get new stone position
	iXDiff = cNewPosition.x - cOldPosition.x ;
	iYDiff = cNewPosition.y - cOldPosition.y ;
	//m_cTimeUtil.DelayMs(30) ;       // delay 30 ms
	//Sleep(30);

	for (iK = 1 ; iK <= 5 ; ++iK) {
		if (!xpcStone->m_xpcSprite->PaintSprite(pDC,
		                                        cOldPosition.x + iXDiff * iK / 5, cOldPosition.y + iYDiff * iK / 5))
			// move stone sprite to new location, and test
		{
			iError = 102 ;
			goto cleanup ;
		}
		MSG msg;
		if (MFC::PeekMessage(&msg, nullptr, MM_MCINOTIFY, MM_MCINOTIFY, PM_REMOVE)) {
			MFC::TranslateMessage(&msg);
			MFC::DispatchMessage(&msg);
		}
		if (iK != 5)
			AfxGetApp()->pause();			//delay 10 ms except for the last one,
		//so that the shell does not "fly" off the board.
	}

	if (xpcToPit->m_cBmpObject.m_bDummy) {
		xpcStone->m_xpcNext = xpcToPit->m_xpcStoneChain ;
		// make it the last stone (front of the chain) for
		// the "to" pit
		xpcToPit->m_xpcStoneChain = xpcStone ;
	} else {
		xpcStone->m_xpcSprite->RefreshBackground(pDC) ;
		// erase the stone
		xpcStone->m_xpcNext = m_xpFreeStoneChain ;
		// put stone on the free chain
		m_xpFreeStoneChain = xpcStone ;

		AdjustPitDisplay(xpcToPit, true) ;
		// adjust the "to" pit
	}

cleanup:
	if (pDC) {
		ReleaseDC(pDC);
		pDC = nullptr;
	}

	JXELEAVE(CMnkWindow::MoveStoneDisplay) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::AdjustPitDisplay -- adjust display of pit when
//          number of stones changes
bool CMnkWindow::AdjustPitDisplay(CPitWnd * xpcPit,
                                  bool bForcePaint)
// xpcPit -- pit whose display is to be adjusted
// bForcePaint -- if true, then always paint
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::AdjustPitDisplay) ;
	int iError = 0 ;        // error code
	CBmpObject * xpcBmpObject = &xpcPit->m_cBmpObject ;
	int iDispStones ;       // number of stones to display
	CRect cBitmapRect ;
	MSG msg;

	if (MFC::PeekMessage(&msg, nullptr, MM_MCINOTIFY, MM_MCINOTIFY, PM_REMOVE)) {
		MFC::TranslateMessage(&msg);
		MFC::DispatchMessage(&msg);
	}

	iDispStones = xpcPit->m_iNumStones ;
	if (iDispStones > xpcPit->m_iDispMax)   // if this is more
		// than the max number of stones we can
		// display discretely
		iDispStones = xpcPit->m_iDispMax + 1 ;  // then use "many"

	if (iDispStones != xpcPit->m_iDispStones)   // if we're changing the
		// number of stones
		ClearBitmapObject(xpcBmpObject) ;   // clear
	// bitmap resources, if any

	if (bForcePaint || iDispStones != xpcPit->m_iDispStones)
		// if we have to repaint
	{
		PaintBitmapObject(xpcBmpObject) ;
		xpcPit->m_iDispStones = iDispStones ;
	}

	JXELEAVE(CMnkWindow::AdjustPitDisplay) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::PaintScreen -- paint screen for mankala game
void CMnkWindow::PaintScreen()
// returns: void
{
	JXENTER(CMnkWindow::PaintScreen) ;
	int iPlayer, iPit ;     // loop variables
	CPitWnd * xpcPit ;      // pit variable
	CBmpObject * xpcStone ; // stones in chain

	PaintBitmapObject(&m_cBmpMain, BMT_MAIN) ;
	// paint main screen bitmap

	if (!m_bInMenu /*&& !m_bJustStarted*/) { // if not in scroll menu
		m_cBmpScroll.m_bSprite = true ; // scroll is a sprite
		PaintBitmapObject(&m_cBmpScroll, BMT_SCROLL) ;
	}

	for (iPlayer = 0 ; iPlayer < NUMPLAYERS ; ++iPlayer)
		for (iPit = -2 ; iPit < NUMPITS ; ++iPit) {
			xpcPit = m_xpcPits[iPlayer][iPit + 2] ;
			// point to pit
			if (xpcPit->m_iNumStones > 0)
				AdjustPitDisplay(xpcPit, true) ;

//      if (xpcPit->m_cBmpObject.m_lpDib)
//      PaintBitmap(xpcPit->m_cBmpObject) ;

			// ****** unfortunately, this chain must be traversed
			// in reverse order ******* bug to be fixed
			for (xpcStone = xpcPit->m_xpcStoneChain ; xpcStone ;
			        xpcStone = xpcStone->m_xpcNext)
				// loop through stone chain for this pit
				PaintBitmapObject(xpcStone) ;
		}
	m_iBmpSign = 0 ;
	SetCrabSign() ;

	JXELEAVE(CMnkWindow::PaintScreen) ;
}


//* CMnkWindow::AllocatePits -- allocate pits (including home bin/hand)
bool CMnkWindow::AllocatePits()
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::AllocatePits) ;
	int iError = 0 ;        // error code
	int iPlayer, iPit ;     // loop variables
	CPitWnd * xpcPit ;      // pointer to latest pit

	for (iPlayer = 0 ; iPlayer < NUMPLAYERS ; ++iPlayer)
		for (iPit = -2 ; iPit < NUMPITS ; ++iPit) {
			if (!(xpcPit = new CPitWnd))
				// allocate a pit and test
			{
				iError = 100 ;      // new CPitWnd failed
				goto cleanup ;
			}

			xpcPit->m_iPlayer = iPlayer ;
			xpcPit->m_iPit = iPit ;
			m_xpcPits[iPlayer][iPit + 2] = xpcPit ;
			CMnk::m_xpcPits[iPlayer][iPit + 2] = (CPit *)xpcPit ;
			xpcPit->m_cBmpObject.m_xpcPit = xpcPit ;
			xpcPit->m_cBmpObject.m_iBmpType =
			    (iPit == HANDINDEX) ? BMT_HAND
			    : (iPit != HOMEINDEX) ? BMT_PIT
			    : (iPlayer == 0) ? BMT_RIGHTBIN : BMT_LEFTBIN ;
			xpcPit->m_cBmpObject.m_bDummy =
			    (xpcPit->m_cBmpObject.m_iBmpType == BMT_HAND) ;

			xpcPit->m_iDispMax = (iPit == HANDINDEX) ? MAXDISPHAND
			                     : (iPit == HOMEINDEX) ? MAXDISPHOME
			                     : MAXDISPPIT ;
		}

cleanup:
	JXELEAVE(CMnkWindow::AllocatePits) ;
	RETURN(iError != 0) ;
}

// CMnkWindow::SetCrabSign -- to my/your turn
bool CMnkWindow::SetCrabSign(bool bPaint)
// bPaint -- paint the new sign
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::SetCrabSign) ;
	int iError = 0;        // error code
	int iBmpSign;      // SBT_xxxx -- sign to display
	char *npszHumanScore;
	char *npszCrabScore;
	HLOCAL hlocHumanScore, hlocCrabScore;

	if (m_bStartGame) {
		iBmpSign = 0 ;
	} else if (m_bGameOver) {

		/*
		 equal stones in home bins; TIE: you HAVE to beat the crab to win
		 */
		if (m_cCurrentMove.m_iNumStones[0][HOMEINDEX + 2]
		        == m_cCurrentMove.m_iNumStones[1][HOMEINDEX + 2]) {
			iBmpSign = SBT_TIE ;
			if (!m_bGameOverMusicPlayed) { //  so that FANFARE2/ SOSORRY is played just once.
				m_bGameWon = false;
				if (pGameParams->bSoundEffectsEnabled)
					sndPlaySound(SOSORRY, SND_ASYNC);
				m_bGameOverMusicPlayed = true;
			}
		}

		/*
		 you win if you have more stones
		 */
		if (m_cCurrentMove.m_iNumStones[0][HOMEINDEX + 2]
		        > m_cCurrentMove.m_iNumStones[1][HOMEINDEX + 2]) {
			iBmpSign = SBT_YOUWIN ;
			if (!m_bGameOverMusicPlayed) { //  so that FANFARE2/ SOSORRY is played just once.
				m_bGameWon = true;
				if (pGameParams->bSoundEffectsEnabled)
					sndPlaySound(FANFARE2, SND_SYNC);
				m_bGameOverMusicPlayed = true;
			}
		} else {
			/*
			 you lose if you have fewer stones
			 */
			iBmpSign = SBT_IWIN ;
			if (!m_bGameOverMusicPlayed) { //  so that FANFARE2/ SOSORRY is played just once.
				m_bGameWon = false;
				if (pGameParams->bSoundEffectsEnabled)
					sndPlaySound(SOSORRY, SND_SYNC);
				m_bGameOverMusicPlayed = true;
			}
		}
	}//end if m_hbGameOver // m_bStartGame

	else if (m_cCurrentMove.m_iPlayer == 0)
		iBmpSign = SBT_YOURTURN ;

	else
		iBmpSign = SBT_MYTURN ;

	if (iBmpSign && m_cBmpSign.m_iBmpArg
	        && iBmpSign != m_cBmpSign.m_iBmpArg) {
		ClearBitmapObject(&m_cBmpSign) ;    // clear old sign bitmap
		m_cBmpSign.m_iBmpArg = 0 ;
	}

	if (m_iBmpSign && m_iBmpSign != iBmpSign) {
		PaintBitmapObject(&m_cBmpCrab, BMT_SIGN, SBT_CRAB) ;
		m_iBmpSign = 0 ;
	}

	if (bPaint && iBmpSign && iBmpSign != m_iBmpSign) {
		PaintBitmapObject(&m_cBmpSign, BMT_SIGN, iBmpSign) ;
		m_iBmpSign = iBmpSign ;
	}
	if (m_bGameOver) {
		if (!m_bScoresDisplayed) {
			/*display scores in stand alone mode*/
			hlocHumanScore = LocalAlloc(LHND, 32);
			hlocCrabScore = LocalAlloc(LHND, 32);

			if (hlocHumanScore && hlocCrabScore) {
				npszHumanScore = (char *)LocalLock(hlocHumanScore);
				npszCrabScore = (char *)LocalLock(hlocCrabScore);

				Common::sprintf_s(npszHumanScore, 32, "Your Score: %d shell%c",   m_cCurrentMove.m_iNumStones[0][HOMEINDEX + 2], (m_cCurrentMove.m_iNumStones[0][HOMEINDEX + 2] > 1) ? 's' : 0);
				Common::sprintf_s(npszCrabScore, 32, "My Score: %d shell%c",  m_cCurrentMove.m_iNumStones[1][HOMEINDEX + 2], (m_cCurrentMove.m_iNumStones[1][HOMEINDEX + 2] > 1) ? 's' : 0);

				CMessageBox(this, m_xpGamePalette, npszHumanScore, npszCrabScore);

				LocalUnlock(hlocHumanScore);
				LocalUnlock(hlocCrabScore);
			}// end if hlocHumanScore  and hlocCrabScore.

			if (hlocHumanScore) LocalFree(hlocHumanScore);
			if (hlocCrabScore) LocalFree(hlocCrabScore);

			m_bScoresDisplayed = true;
		}  // end if m_bScoredisplayed

		if (pGameParams->bPlayingMetagame) {
			/*auto exit upon Game Over, in Meta mode.*/

			//::Sleep(2000);                                                                                      // prolong the display of win /lose sign.
			//UpdateWindow();
			pGameParams->lScore = m_bGameWon;               //do the same things as in OptionsDialog() with ...
			PostMessage(WM_CLOSE, 0, 0L);                           //IDC_OPTIONS_QUIT

		}  // end if pGameParams->bPlayingMetagame.
	} // end if m_bGameOver.

	JXELEAVE(CMnkWindow::SetCrabSign) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::FreePitResources -- free (optionally delete) all pit
//      resources -- stone sprites and pit bitmaps
bool CMnkWindow::FreePitResources(bool bDelete)
// bDelete -- if true, then delete all stone sprites
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::FreePitResources) ;
	int iError = 0 ;        // error code
	int iPlayer, iPit ;     // loop variables
	CPitWnd * xpcPit ;      // pit variable
	CBmpObject * xpcLastStone, *xpcNextStone ;  // stones in chain

	for (iPlayer = 0 ; iPlayer < NUMPLAYERS ; ++iPlayer)
		for (iPit = -2 ; iPit < NUMPITS ; ++iPit) {
			xpcPit = m_xpcPits[iPlayer][iPit + 2] ;
			if ((xpcLastStone = xpcPit->m_xpcStoneChain))
				// if pit has stone chain
			{
				while ((xpcNextStone = xpcLastStone->m_xpcNext))
					// loop to end of stone chain
					xpcLastStone = xpcNextStone ;   // move to next

				xpcLastStone->m_xpcNext = m_xpFreeStoneChain ;
				// add new chain to free chain
				m_xpFreeStoneChain = xpcPit->m_xpcStoneChain ;
				xpcPit->m_xpcStoneChain = nullptr ;
				// no more chain on pit
			}

			ClearBitmapObject(&xpcPit->m_cBmpObject) ;
//		delete xpcPit;
		}

	if (bDelete)        // if we're deleting
		while ((xpcNextStone = m_xpFreeStoneChain))
			// get first/next stone sprite in chain
		{
			m_xpFreeStoneChain = xpcNextStone->m_xpcNext ;
			// unlink it from stone sprite chain
			(*(CSprite *)xpcNextStone).UnlinkSprite();                          // ... unlinking it
			// unlink it from sprite chain
			delete xpcNextStone ;       // delete stone sprite
		}

	JXELEAVE(CMnkWindow::FreePitResources) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::ClearBitmapObject -- release bitmap object
bool CMnkWindow::ClearBitmapObject(CBmpObject * xpcBmpObject)
// xpcBmpObject -- pointer to bitmap object
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::ClearBitmapObject) ;
	int iError = 0 ;        // error code
	HDIB hDib ;

	if (xpcBmpObject->m_xpcSprite) { // if there's a sprite
		delete xpcBmpObject->m_xpcSprite ;  // free it
		xpcBmpObject->m_xpcSprite = nullptr ;  // no more pointer
	}

	if (xpcBmpObject->m_xpDibDoc) { // if there's a DIB document
		if (xpcBmpObject->m_lpDib &&
		        (hDib = xpcBmpObject->m_xpDibDoc->GetHDIB())) {
			GlobalUnlock(hDib) ;      // unlock Dib memory
		}

		xpcBmpObject->m_lpDib = nullptr ;  // clear pointers
		delete xpcBmpObject->m_xpDibDoc ;
		xpcBmpObject->m_xpDibDoc = nullptr ;
	}
	xpcBmpObject->m_bInit = false ; // object no longer initialized

	JXELEAVE(CMnkWindow::ClearBitmapObject) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::ReleaseResources -- release all resources before term
void CMnkWindow::ReleaseResources() {
	CBmpObject * xpcBmpObject ;     // objects to be freed
	CPitWnd* xpcPitWnd;

	while ((xpcBmpObject = m_xpcBmpFreeChain)) {
		m_xpcBmpFreeChain = xpcBmpObject->m_xpcNextFree ;
		xpcBmpObject->m_xpcNextFree = nullptr ;    // out of habit
		ClearBitmapObject(xpcBmpObject) ;   // free resources
		if (xpcBmpObject->m_bHeap)  // if on the heap
			delete xpcBmpObject ;
	}

	for (int i = 0 ; i < NUMPLAYERS ; ++i)
		for (int j = -2 ; j < NUMPITS ; ++j) {
			if ((xpcPitWnd = m_xpcPits[i][j + 2]))
				delete xpcPitWnd;
			xpcPitWnd = nullptr;
		}

	if (m_pSound) {
		m_pSound->stop();
		delete m_pSound;
		m_pSound = nullptr;
	}
}


//* CMnkWindow::DebugDialog -- put up debugging dialog box
bool CMnkWindow::DebugDialog()
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::DebugDialog) ;
	int iError = 0 ;        // error code
	CMnkOpt cMnkOpt ;       // options dialog class object

	// initialize the options dialog values
	cMnkOpt.m_iStartStones = m_iStartStones ;
	cMnkOpt.m_iLevel0 = m_eLevel[0] - 1 ;
	cMnkOpt.m_iLevel1 = m_eLevel[1] - 1 ;
	cMnkOpt.m_iPlayer0 = m_bComputer[0] ;
	cMnkOpt.m_iPlayer1 = m_bComputer[1] ;
	cMnkOpt.m_iTableStones = m_iTableStones ;
	cMnkOpt.m_bInitData = m_bInitData ;
	cMnkOpt.m_iMaxDepth0 = m_iMaxDepth[0] ;
	cMnkOpt.m_iMaxDepth1 = m_iMaxDepth[1] ;
	cMnkOpt.m_iCapDepth0 = m_iCapDepth[0] ;
	cMnkOpt.m_iCapDepth1 = m_iCapDepth[1] ;

	cMnkOpt.m_bDumpPopulate = m_bDumpPopulate ;
	cMnkOpt.m_bDumpMoves = m_bDumpMoves ;
	cMnkOpt.m_bDumpTree = m_bDumpTree ;

	if (cMnkOpt.DoModal() == IDOK) {
		m_iStartStones = cMnkOpt.m_iStartStones ;
		m_eLevel[0] = (enum_Level)(cMnkOpt.m_iLevel0 + 1) ;
		m_eLevel[1] = (enum_Level)(cMnkOpt.m_iLevel1 + 1) ;
		m_bComputer[0] = cMnkOpt.m_iPlayer0 ;
		m_bComputer[1] = cMnkOpt.m_iPlayer1 ;
		m_iTableStones = cMnkOpt.m_iTableStones ;
		m_bInitData = cMnkOpt.m_bInitData ;
		m_iMaxDepth[0] = cMnkOpt.m_iMaxDepth0 ;
		m_iMaxDepth[1] = cMnkOpt.m_iMaxDepth1 ;
		m_iCapDepth[0] = cMnkOpt.m_iCapDepth0 ;
		m_iCapDepth[1] = cMnkOpt.m_iCapDepth1 ;
		m_bDumpPopulate = cMnkOpt.m_bDumpPopulate ;
		m_bDumpMoves = cMnkOpt.m_bDumpMoves ;
		m_bDumpTree = cMnkOpt.m_bDumpTree ;
	}


	JXELEAVE(CMnkWindow::DebugDialog) ;
	RETURN(iError != 0) ;
}

//* CMnkWindow::UserDialog -- put up user dialog box
bool FAR PASCAL CMnkWindow::UserDialog()
// returns: true if error, false otherwise
{
	JXENTER(CMnkWindow::UserDialog) ;
	int iError = 0 ;        // error code

	CMnkUsr cMnkUsr(this, m_xpGamePalette) ;
	// options dialog class object

	// initialize the options dialog values
	cMnkUsr.m_iUShells = m_iStartStones ;
	cMnkUsr.m_iUStrength = m_eLevel[0] ;

	if (cMnkUsr.DoModal() == IDOK) {
		m_iStartStones = cMnkUsr.m_iUShells ;
		m_eLevel[0] = m_eLevel[1] = (enum_Level)cMnkUsr.m_iUStrength ;
	}

	JXELEAVE(CMnkWindow::UserDialog) ;
	RETURN(iError != 0) ;
}

////* CallUserDialog --
void CALLBACK CallUserDialog(CWnd * xpcWindow) {
// returns: true if error, false otherwise
	JXENTER(CallUserDialog) ;

	CMnkWindow * xpcMnkWindow = (CMnkWindow *)xpcWindow->GetParent();
	xpcMnkWindow->UserDialog() ;

	JXELEAVE(CallUserDialog) ;
}

//* CMnkWindow::OptionsDialog -- call options dialog
bool CMnkWindow::OptionsDialog() {
// returns: true if error, false otherwise
	JXENTER(CMnkWindow::OptionsDialog) ;
	int iError = 0 ;        // error code

	CDC *pDC = nullptr ;

	if (!m_bInMenu) {   // prevent recursion
		m_bInMenu = true ;  // in the options menu now
		if ((pDC = GetDC())) {
			m_cBmpScroll.m_xpcSprite->EraseSprite(pDC) ;
			// hide the command scroll
			ReleaseDC(pDC);
			pDC = nullptr;
		}

		/*
		set up main menu:
		If in MetaGame mode disable newgame and options buttons and on start up
		disable Continue button
		*/
		CMainMenu cOptionsDlg(this, m_xpGamePalette,
		                      (pGameParams->bPlayingMetagame ? NO_NEWGAME | NO_OPTIONS : false) | (m_bStartGame ? NO_RETURN : 0X0),
		                      CallUserDialog, RULES, pGameParams->bSoundEffectsEnabled ? RULES_NARRATION : nullptr, pGameParams) ;
		switch (cOptionsDlg.DoModal()) {
		case IDC_OPTIONS_NEWGAME:
			if (!pGameParams->bPlayingMetagame) StartGame() ;
			break ;

		case IDC_OPTIONS_QUIT:
			pGameParams->lScore = (long)m_bGameWon;
			m_bGameOver = true ;
			PostMessage(WM_CLOSE, 0, 0);
			break ;

		default:
			break ;
		}

		PaintBitmapObject(&m_cBmpScroll, BMT_SCROLL) ;

		//
		// Check to see if the music state was changed and adjust to match it
		//
		if ((pGameParams->bMusicEnabled == false) && (m_pSound != nullptr)) {
			if (m_pSound->playing())
				m_pSound->stop();
		} else if (pGameParams->bMusicEnabled) {
			if (m_pSound == nullptr) {
				m_pSound = new CSound(this, MIDI_BCKGND, SOUND_MIDI | SOUND_DONT_LOOP_TO_END);
			}
			if (m_pSound != nullptr) {
				if (!m_pSound->playing())
					m_pSound->midiLoopPlaySegment(1004L, 34040L, 1004L, FMT_MILLISEC);
			}
		}

		m_bInMenu = false ;
	}

	/* cleanup */
	JXELEAVE(CMnkWindow::OptionsDialog) ;
	RETURN(iError != 0) ;
}

} // namespace Mankala
} // namespace HodjNPodj
} // namespace Bagel
