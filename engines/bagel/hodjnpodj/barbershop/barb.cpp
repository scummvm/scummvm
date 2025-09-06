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
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/barbershop/barb.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

//
// global
//
extern LPGAMESTRUCT     pGameParams;

/*****************************************************************
 *
 * CBarber
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Initializes members.
 *
 * FORMAL PARAMETERS:           n/a
 *
 * IMPLICIT INPUT PARAMETERS:   n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                barbershop quintet object
 *
 ****************************************************************/
CBarber::CBarber(CDC *pDC, CSound *pSound) {
	m_cDck      = new CDeck();
	m_cPnt      = new CPaint(pDC);
	m_cBrd      = new CBoard(m_cPnt);
	m_pLogic    = new CLogic();
	m_pUndo     = new CUndo();
	m_pSound    = pSound;
	m_pCrd      = nullptr;             // remembers cur card

	/********************************************************
	* Switchs used to prevent further user updates          *
	* after timer has run out, no moves left, or is solved. *
	********************************************************/
	m_bIsGameOver   = false;        // Initialize solved switch
	m_bIsWin        = false;        // Initialize win/lose switch
}

/*****************************************************************
 *
 * ~Barber
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Destructor
 *
 * FORMAL PARAMETERS:           n/a
 *
 * IMPLICIT INPUT PARAMETERS:   n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
CBarber::~CBarber() {

	if (m_pSound != nullptr) {
		m_pSound = nullptr;            // never init with a "new" operator
	}

	if (m_pUndo != nullptr) {
		delete m_pUndo;
		m_pUndo = nullptr;
	}

	if (m_pLogic != nullptr) {
		delete m_pLogic;
		m_pLogic = nullptr;
	}

	if (m_cBrd != nullptr) {
		delete m_cBrd;
		m_cBrd = nullptr;
	}

	if (m_cPnt != nullptr) {
		delete m_cPnt;
		m_cPnt = nullptr;
	}

	if (m_cDck != nullptr) {
		delete m_cDck;
		m_cDck = nullptr;
	}
}

/*****************************************************************
 *
 * NewGame
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Resets interal state, and starts a new game.
 *
 * FORMAL PARAMETERS:
 *
 *  pDC - device context for painting images to the window.
 *
 * IMPLICIT INPUT PARAMETERS:   n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
void CBarber::NewGame(CDC *pDC) {
	m_cDck->Shuffle();
	m_cDck->Deal(m_cBrd);
	m_cPnt->Board(pDC, m_cBrd);     // paint the cards visually on the board
	m_pUndo->Reset();               // clear all undo info
	m_bIsGameOver = false;          // turn off game over switch
}

/*****************************************************************
 *
 * Refresh
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Repaints all cards on the board.
 *
 * FORMAL PARAMETERS:           n/a
 *
 * IMPLICIT INPUT PARAMETERS:   n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
void CBarber::Refresh(CDC *pDC) {
	m_cPnt->Refresh(pDC, m_cBrd);
}

/*****************************************************************
 *
 * OnLButtonDown
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Determines if user clicked on a card or on the undo area.
 *
 * FORMAL PARAMETERS:
 *
 *  pWnd - the parent window used for poping up message dialog
 *  boxes.
 *  pPalette - passed to any message box that needs to be
 *  displayed.
 *  point - point where user double clicked.
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_pCrd - will remember what card user clicked on.
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
void CBarber::OnLButtonDown(CWnd *pWnd, CPalette *pPalette, CPoint point) {
	CDC     *pDC = nullptr;        // device context for painting
	CCard   *pCard;             // for stack push and pop operations
	int     i;                  // index

	if (              // user want to undo a change?
	    m_pUndo->m_cUndoRect.PtInRect(point) == true &&
	    m_pCrd == nullptr
	) {
		pDC = pWnd->GetDC();

		if (pGameParams->bSoundEffectsEnabled != false)
			m_pSound->initialize(
			    UNDO,
			    SOUND_WAVE | SOUND_ASYNCH
			);

		if (m_pUndo->Undo(pDC, m_cBrd, m_cPnt) == true) {    // undo successful?
			if (pGameParams->bSoundEffectsEnabled != false)
				m_pSound->play();

			// undoing does not always guarantee there is a move left,
			// but it is okay to reset game over switch anyway.
			//
			m_bIsGameOver = false;                          // yes - undo successful.
		} // end if
		pWnd->ReleaseDC(pDC);
		return;
	} // end if

	if (m_bIsGameOver == true)
		return;

	if ((m_pCrd = m_cPnt->IsOnCard(point)) == nullptr) {    // get card corr to point
		/********************************************************
		* Find out if stock cards need to be recycled.  This    *
		* will occur if the stock card stack is empty and there *
		* are cards on the used stack.                          *
		********************************************************/
		if (
		    m_cBrd->GetStack((loc) stock)->m_cRect.PtInRect(point) == true &&
		    m_cBrd->GetStack((loc) stock)->IsEmpty() == true
		) {
			int nFlipSnd = 0;                                       // keeps track of when to play fwap sound
			pDC = pWnd->GetDC();

			// user wants to recycle used stack
			//
			if (pGameParams->bSoundEffectsEnabled != false)          // init sound if it is enabled
				m_pSound->initialize(
				    STOCKCARDS,
				    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
				);

			while (m_cBrd->GetStack((loc) used)->IsEmpty() != true) {
				pCard = m_cBrd->GetStack((loc) used)->Pop();        // take top card off used stack
				m_cBrd->GetStack((loc) stock)->Push(pCard);         // put it on stock stack

				// sound (if enabled)
				if (
				    pGameParams->bSoundEffectsEnabled != false &&
				    nFlipSnd % RECYCLE_SOUND_FREQ == 0
				)
					m_pSound->play();                                   // make flap sound...
				nFlipSnd++;                                         // every three cards

				m_cPnt->FlipCard(pDC, pCard);                       // repaint top card
				m_cPnt->Stack(pDC, pCard);                          // indented on approp stack
			} // end while
			pWnd->ReleaseDC(pDC);                                   // release device context!
			m_pUndo->Reset();                                       // Can't undo recycled used stack
		} // end if
		return;
	} // end if

	/*****************************************************************
	* Figure out what stack user clicked on, and behave accordingly. *
	*****************************************************************/
	switch (m_pCrd->m_pStack->GetID()) {

	case fnd: {     // foundation stack
		/************************************************
		* Not allowed to remove cards from foundation.  *
		* Tell the user his score.                      *
		************************************************/
		char buf[32];
		Common::sprintf_s(buf, "Score: %d", Score());
		CMessageBox cmsgboxScore(
		    pWnd,
		    pPalette,
		    buf
		);
		m_pCrd = nullptr;                                  // don't remember the clicked card
		return;
	}

	case stock: {   // stock stack
		/****************************************************
		* Flip cards from stock to used stack, or recycle   *
		* cards on used stack if used stack is empty.       *
		****************************************************/
		CStack *pStock, *pUsed;

		pStock  = m_cBrd->GetStack(stock);      // get stock stack for future ref
		pUsed   = m_cBrd->GetStack(used);       // get used stack too.

		// sound (if enabled)
		if (pGameParams->bSoundEffectsEnabled != false) {
			m_pSound->initialize(
			    STOCKCARDS,
			    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
			);
			m_pSound->play();
		}

		pDC = pWnd->GetDC();

		for (i = 0; i < STOCK_DRAW; i++) {           // flip over three cards onto used stack
			if ((pCard = pStock->Pop()) == nullptr) {   // Out of cards on the Stock stack?
				break;
			}

			// Move card from stock stack, to used stack.
			//
			m_cPnt->FlipCard(pDC, pCard);               // give card a flipped sprite
			pUsed->Push(pCard);                         // put stock card on used stack
			m_cPnt->Stack(pDC, pCard);                  // paint card appropriately indented
		} // end for

		pWnd->ReleaseDC(pDC);

		if (pStock->IsEmpty() == true)               // game over?
			IsGameOver(pWnd);

		m_pUndo->Record(i);                             // note stack flip for possible undoing
		m_pCrd = nullptr;                                  // don't remember the clicked card
		return;
	} // end case

	case used:  // used stack rules
		return; // have fun with these! (no special function)

	default:    // tab stack
		if (m_pCrd->m_bIsBack == true) {         // want to flip over card?
			pDC = pWnd->GetDC();                // yes
			m_cPnt->FlipCard(pDC, m_pCrd);      // flip card
			m_cPnt->UpdateCard(pDC, m_pCrd);    // paint it
			pWnd->ReleaseDC(pDC);

			m_pUndo->Reset();                   // Can't undo flipped card
			m_pCrd = nullptr;                      // don't remember the clicked card
			return;
		}  // end if
	} // end switch
}

/*****************************************************************
 *
 *  OnLButtonDblClk
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Handles undo and stock card flips only.  Will not remember
 *  card moving.
 *
 * FORMAL PARAMETERS:
 *
 *  pWnd - the parent window used for poping up message dialog
 *  boxes.
 *  pPalette - passed to any message box that needs to be
 *  displayed.
 *  point - point where user double clicked.
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_pCrd - will remember what card user clicked on.
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
void CBarber::OnLButtonDblClk(CWnd *pWnd, CPalette *pPalette, CPoint point) {
	CDC     *pDC = nullptr;
	CCard   *pCard;
	int     i;

	if (             // user want to undo a change?
	    m_pUndo->m_cUndoRect.PtInRect(point) == true &&
	    m_pCrd == nullptr
	) {
		pDC = pWnd->GetDC();
		if (pGameParams->bSoundEffectsEnabled != false)
			m_pSound->initialize(
			    UNDO,
			    SOUND_WAVE | SOUND_QUEUE
			);

		if (m_pUndo->Undo(pDC, m_cBrd, m_cPnt) == true) {    // undo successful?
			if (pGameParams->bSoundEffectsEnabled != false)
				m_pSound->play();

			m_bIsGameOver = false;                          // yes
		}
		pWnd->ReleaseDC(pDC);
		return;
	} // end if

	if (m_bIsGameOver == true)
		return;

	if ((m_pCrd = m_cPnt->IsOnCard(point)) == nullptr) {    // get card corr to point
		/********************************************************
		* Find out if stock cards need to be recycled.  This    *
		* will occur if the stock card stack is empty and there *
		* are cards on the used stack.                          *
		********************************************************/
		if (
		    m_cBrd->GetStack((loc) stock)->m_cRect.PtInRect(point) == true  &&
		    m_cBrd->GetStack((loc) stock)->IsEmpty() == true
		) {
			int nFlipSnd = 0;                                       // keeps track of when to play fwap sound

			// user wants to recycle used stack
			//
			if (pGameParams->bSoundEffectsEnabled != false)          // init sound if it is enabled
				m_pSound->initialize(
				    STOCKCARDS,
				    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
				);

			pDC = pWnd->GetDC();
			while (m_cBrd->GetStack((loc) used)->IsEmpty() != true) {
				pCard = m_cBrd->GetStack((loc) used)->Pop();        // get used card
				m_cBrd->GetStack((loc) stock)->Push(pCard);         // put used card onto stock

				// sound (if enabled)
				if (            // play fwap sound...
				    pGameParams->bSoundEffectsEnabled != false &&   // every RECYCLE_'_FREQ card
				    nFlipSnd % RECYCLE_SOUND_FREQ == 0
				)
					m_pSound->play();                                   // make flap sound...
				nFlipSnd++;                                         // update fwap counter

				m_cPnt->FlipCard(pDC, pCard);                       // change card sprite
				m_cPnt->Stack(pDC, pCard);                          // paint card properly indented on approp stack
			} // end while
			pWnd->ReleaseDC(pDC);
			m_pUndo->Reset();           // Can't undo recycled used stack

		} // end if

		return;
	} // end if

	/*****************************************************************
	* Figure out what stack user clicked on, and behave accordingly. *
	*****************************************************************/
	switch (m_pCrd->m_pStack->GetID()) {

	case fnd: {     // foundation stack rules
		/***********************************************
		* Not allowed to remove cards from foundation. *
		***********************************************/
		char buf[32];
		Common::sprintf_s(buf, "Score: %d", Score());
		CMessageBox cGameOver(
		    pWnd,
		    pPalette,
		    buf
		);
		m_pCrd = nullptr;
		return;
	}

	case stock: {   // stock stack rules
		/****************************************************
		* Flip cards from stock to used stack, or recycle   *
		* cards on used stack if used stack is empty.       *
		****************************************************/
		CStack *pStock, *pUsed;

		pStock  = m_cBrd->GetStack(stock);
		pUsed   = m_cBrd->GetStack(used);

		// sound (if enabled)
		if (pGameParams->bSoundEffectsEnabled != false) {
			m_pSound->initialize(
			    STOCKCARDS,
			    SOUND_WAVE | SOUND_ASYNCH | SOUND_QUEUE
			);
			m_pSound->play();
		} // end if

		for (i = 0; i < STOCK_DRAW; i++) {
			if ((pCard = pStock->Pop()) == nullptr) {   // Out of cards on the Stock stack?
				break;
			}
			pDC = pWnd->GetDC();
			m_cPnt->FlipCard(pDC, pCard);
			pUsed->Push(pCard);
			m_cPnt->Stack(pDC, pCard);
			pWnd->ReleaseDC(pDC);
		} // end for

		if (pStock->IsEmpty() == true)
			IsGameOver(pWnd);

		m_pUndo->Record(i);     // note stack flip for possible undoing
		m_pCrd = nullptr;
		return;
	} // end case

	default:    // tab stacks
		if (m_pCrd->m_bIsBack == true) {         // user want to flip a card?
			pDC = pWnd->GetDC();
			m_cPnt->FlipCard(pDC, m_pCrd);
			m_cPnt->UpdateCard(pDC, m_pCrd);
			pWnd->ReleaseDC(pDC);

			m_pUndo->Reset();                   // Can't undo flipped card

			m_pCrd = nullptr;
			return;
		}  // end if
	} // end switch
}

/*****************************************************************
 *
 *  OnMouseMove
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Paint the card at the mouse position if user is holding onto
 *  a card.
 *
 * FORMAL PARAMETERS:
 *
 *  pDC - device context for painting images to the window.
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_pCrd - contains the card the user is holding.
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
void CBarber::OnMouseMove(CDC *pDC, CPoint point) {
	if (m_pCrd == nullptr)          // no card selected
		return;

	m_cPnt->MoveCard(pDC, m_pCrd, point);
}

/*****************************************************************
 *
 *  OnMouseMove
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Paint the card at the mouse position if user is holding onto
 *  a card.
 *
 * FORMAL PARAMETERS:
 *
 *  pDC - Device context for painting images on the window.
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_pCrd - contains the card the user is holding.
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
void CBarber::OnLButtonUp(CWnd *pWnd) {
	CDC     *pDC = nullptr;
	bool    bSuccess = false;
	CRect   cTest;
	CStack  *pStack;
	int     i;

	if (m_pCrd == nullptr)                          // no card selected
		return;

	/************************************************************
	* Updates interal stack reps.  Flip thru all stacks.        *
	* Find card interception on any stack.  Adds card to first  *
	* intercepted stack.                                        *
	************************************************************/
	for (i = 0; i <= used; i++) {
		pStack = m_cBrd->GetStack((loc) i);     // get appropriate stack
		if (pStack->IsEmpty() == true) {         // assigns rect of interest as
			cTest = pStack->m_cRect;            //  either base rect
		} else {                                //  or card rect from top of stack
			cTest = pStack->Top()->m_pSprite->GetRect();
		} // end if

		/************************************************************
		* Handles special case of current stack is same as          *
		* stack that the card is in.  In this case, either the      *
		* the rect of previous card (if any) needs to be used as    *
		* the test region.                                          *
		************************************************************/
		if (m_pCrd->m_pStack == pStack) {            // is card on original stack?
			if (m_pCrd->m_pPrevCard != nullptr) {   // Yes - any other cards on this stack?
				cTest = pStack->Top()->m_pPrevCard->m_pSprite->GetRect();
			} else {
				cTest = pStack->m_cRect;            //  either base rect
			} // end if
		} // end if

		/********************************************************
		* find out if corner points of the card line anywhere   *
		* within a stack's rect.                                *
		********************************************************/
//b
		if (IsInRect(cTest, m_pCrd->m_pSprite->GetRect()) == false) {  //!= true ) { // is card on invalid area?
			continue;
		} // end if

		if (m_pLogic->IsMoveOk(m_pCrd, pStack) == true) {
			m_pUndo->Record(m_pCrd->m_pStack, m_pCrd);  // note move for possible undoing

			m_pCrd->m_pStack->Pop();
			pStack->Push(m_pCrd);
			bSuccess = true;
		} // end if
		continue;
//b     break;
	} // end for

	/*************************
	* Update visual display. *
	*************************/
	pDC = (*pWnd).GetDC();
	if (bSuccess != true) {                                  // card dropped over stack?
		m_cPnt->UpdateCard(pDC, m_pCrd);                    // no - redraw card over original stack
	} else {
		m_cPnt->Stack(pDC, m_pCrd);                         // draw card apro stack
	} // end if
	(*pWnd).ReleaseDC(pDC);

	m_pCrd = nullptr;                                          // clear current card
	if (m_cBrd->GetStack(stock)->IsEmpty() == true)
		IsGameOver(pWnd);                                       // game over?
}

/*****************************************************************
 *
 *  IsInRect
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Determines if card's hotspot is intercepts the stack rectangle.
 *
 * FORMAL PARAMETERS:
 *
 *  cStk - The rectangle of the stack.
 *  cCrd - The rectangle of the card.
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_pCrd - it assumes that the user is currently holding a card.
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
bool CBarber::IsInRect(CRect cStk, CRect cCrd) {
//b CPoint  point;
	CRect   inter;
	bool    bTL;
	/*
	    point.x = cCrd.TopLeft().x + m_pCrd->m_pSprite->GetHotspot().x;
	    point.y = cCrd.TopLeft().y + m_pCrd->m_pSprite->GetHotspot().y;
	*/

//b bTL = cStk.PtInRect(point);
	cStk.InflateRect(5, 5);

	bTL = inter.IntersectRect(cStk, cCrd);
	return bTL;
}

/*****************************************************************
 *
 *  IsGameOver
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Checks every possible move to determine if there are any moves
 *  left.
 *
 * FORMAL PARAMETERS:           n/a
 *
 * IMPLICIT INPUT PARAMETERS:   n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
bool CBarber::IsGameOver(CWnd *pWnd) {
	/*****************************
	* Determine if game is over. *
	*****************************/
	m_bIsGameOver = m_pLogic->IsGameOver(m_cBrd);
	if (m_bIsGameOver == false)
		return false;

	/**********************
	* Determine win/loss. *
	**********************/
	if (m_cBrd->GetStack((loc) fnd)->Size() == DECK) {
		// sound (if enabled)
		if (pGameParams->bSoundEffectsEnabled != false) {
			m_pSound->initialize(
			    WIN_SOUND,
			    SOUND_WAVE | SOUND_ASYNCH
			);
			m_pSound->play();
		}

		m_bIsWin = true;
	} else {
		// sound (if enabled)
		if (pGameParams->bSoundEffectsEnabled != false) {
			m_pSound->initialize(
			    LOSE_SOUND,
			    SOUND_WAVE | SOUND_ASYNCH
			);
			m_pSound->play();
		}

		m_bIsWin = false;
	}

	return true;
}

/*****************************************************************
 *
 *  IsNewBack
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:           n/a
 *
 * IMPLICIT INPUT PARAMETERS:   n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
bool CBarber::IsNewBack(int nCardBack) {
	return m_cPnt->IsNewBack(nCardBack);
}

/*****************************************************************
 *
 *  ChangeBack
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *
 * FORMAL PARAMETERS:           n/a
 *
 * IMPLICIT INPUT PARAMETERS:   n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
void CBarber::ChangeBack(CDC* pDC, int nCardBack) {
	m_cPnt->ChangeBack(pDC, m_cBrd, nCardBack);
}

/*****************************************************************
 *
 *  Score
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Returns number of cards on the foundation, defined as the
 *  score.
 *
 * FORMAL PARAMETERS:           n/a
 *
 * IMPLICIT INPUT PARAMETERS:   n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:  n/a
 *
 * RETURN VALUE:                n/a
 *
 ****************************************************************/
int CBarber::Score() {
	return m_cBrd->GetStack(fnd)->Size();
}

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
