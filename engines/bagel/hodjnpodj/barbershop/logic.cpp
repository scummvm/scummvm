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
#include "bagel/hodjnpodj/barbershop/logic.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

/*****************************************************************
 *
 * Logic
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Destructor
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
CLogic::CLogic() {
}

/*****************************************************************
 *
 * ~Logic
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Destructor
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
CLogic::~CLogic() {
}

/*****************************************************************
 *
 * IsMoveOk
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Destructor
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
BOOL CLogic::IsMoveOk(CCard *pCard, CStack *pStack) {
	CCard *pTopCard;

	switch (pStack->GetID()) {

	case fnd:       // foundation rules
		if (pCard->IsFace() == TRUE)
			return TRUE;
		if (pStack->IsEmpty() == TRUE)
			return FALSE;

		pTopCard = pStack->Top();
		if (pTopCard->IsManicurist() == TRUE)
			return TRUE;

		if (
		    pTopCard->IsCustomer() == TRUE &&
		    pCard->GetPip() == ONE_CARD
		)
			return TRUE;

		if (
		    pTopCard->IsBarber() == TRUE &&
		    pCard->GetPip() == TEN_CARD
		)
			return TRUE;

		if (pTopCard->GetPip() == ONE_CARD)
			if (pCard->GetPip() == TWO_CARD) {
				return TRUE;
			} else {
				return FALSE;
			}

		if (pTopCard->GetPip() == TEN_CARD)
			if (pCard->GetPip() == NINE_CARD) {
				return TRUE;
			} else {
				return FALSE;
			}

		if (
		    (
		        pTopCard->GetPip() == (pCard->GetPip() + 1) ||
		        pTopCard->GetPip() == (pCard->GetPip() - 1)
		    ) &&
		    pTopCard->IsFace() == FALSE
		)
			return TRUE;

		return FALSE;

	case stock:
		return FALSE;
	case used:
		return FALSE;
	default:        // tab
		if (pStack->IsEmpty() == TRUE) {     // empty space on tab?
			if (        // yes - source card from stack with other face cards?
			    pCard->m_pStack->GetID() >= tab &&
			    pCard->m_pStack->GetID() < stock &&
			    pCard->m_pPrevCard != nullptr &&
			    pCard->m_pPrevCard->m_bIsBack == TRUE
			)
				return TRUE;                // this is a valid move
			if (        // yes - source card from stock?
			    pCard->m_pStack->GetID() == used
			)
				return TRUE;                // this is a valid move
			return FALSE;
		}

		if (
		    pStack->IsEmpty() == TRUE &&
		    pCard->m_pStack->GetID() >= tab &&
		    pCard->m_pStack->GetID() < stock &&
		    pCard->m_pPrevCard != nullptr &&
		    pCard->m_pPrevCard->m_bIsBack == TRUE
		)                   // when there's an empty space on the tab
			return TRUE;


		pTopCard = pStack->Top();
		if (pCard == pTopCard)   // card back on it's own stack?
			return FALSE;           // yes, illegal

		if (pTopCard->m_bIsBack == TRUE)
			return FALSE;

		if (
		    pTopCard->IsManicurist() == TRUE &&
		    pCard->IsManicurist() == TRUE
		)
			return TRUE;

		if (pTopCard->GetPip() == pCard->GetPip()) {
			if (pCard->m_pStack->GetID() == used)            // Card from used stack?
				return TRUE;                                // yes - acceptable move
			if (pCard->m_pPrevCard == nullptr)              // anything under cur card?
				return TRUE;                                // no
			if (pCard->m_pPrevCard->m_bIsBack == TRUE)   // card under cur card a card back?
				return TRUE;                                // yes
			return FALSE;
		}

		return FALSE;
	}
}

BOOL CLogic::IsGameOver(CBoard *pBoard) {
	CStack  *pFnd;
	CCard   *pCard;
	int     i, j;

	pFnd = pBoard->GetStack((loc) fnd);
	if (pFnd->Size() == DECK)        // All cards on foundation?
		return TRUE;                // Yes

	/******************************************************
	* Check all combinations of tableau to tableau moves. *
	******************************************************/
	for (i = tab; i < stock; i++) {
		if ((pCard = pBoard->GetStack((loc) i)->Top()) == nullptr)
			continue;

		if (pCard->m_bIsBack == TRUE)
			return FALSE;

		for (j = tab; j < stock; j++) {
			if (pCard->m_pStack == pBoard->GetStack((loc) j))
				continue;
			if (IsMoveOk(pCard, pBoard->GetStack((loc) j)) == TRUE)
				return FALSE;
		} // end for
	} // end for

	//return FALSE; // used for debugging purposes

	/*********************************************************
	* Check all combinations of tableau to foundation moves. *
	*********************************************************/
	for (i = tab; i < stock; i++) {
		if ((pCard = pBoard->GetStack((loc) i)->Top()) == nullptr)
			continue;

		if (IsMoveOk(pCard, pFnd) == TRUE)
			return FALSE;
	} // end for

	/************************************
	* Check all combinations of         *
	* - used stack to tableau moves.    *
	* - used stack to foundation.       *
	* - stock to tableau moves.         *
	* - stock to foundation.            *
	************************************/
	pCard   = pBoard->GetStack((loc) stock)->Top();
	while (pCard != nullptr) {
		for (j = 0; j < STOCK_DRAW; j++) {
			if (pCard->m_pStack->Bottom() == pCard)      // Out of cards on the Stock stack?
				break;

			if (pCard->m_pStack->Top() == pCard)         // if top, skip a count
				j++;

			pCard = pCard->m_pPrevCard;
		} // end for

		for (j = fnd; j < stock; j++) {
			#ifdef REVEAL
			if (pBoard->GetStack((loc) j)->Top() == nullptr)
				if (tab <= j && j < stock) {     // empty slots r valid moves
					return FALSE;
				} else {
					continue;
				}
			#endif

			if (IsMoveOk(pCard, pBoard->GetStack((loc) j)) == TRUE)
				return FALSE;
		} // end for

		if (pCard->m_pStack->Bottom() == pCard)          // Out of cards on the Stock stack?
			break;
	} // end while

	pCard   = pBoard->GetStack((loc) used)->Bottom();
	while (pCard != nullptr) {
		for (j = 0; j < STOCK_DRAW; j++) {
			if (pCard->m_pStack->Top() == pCard)         // Out of cards on the Stock stack?
				break;

			if (pCard->m_pStack->Bottom() == pCard)      // if bottom, skip a count
				j++;

			pCard = pCard->m_pNextCard;
		} // end for

		for (j = fnd; j < stock; j++) {
			#ifdef REVEAL
			if (pBoard->GetStack((loc) j)->Top() == nullptr)
				continue;
			#endif

			if (IsMoveOk(pCard, pBoard->GetStack((loc) j)) == TRUE)
				return FALSE;
		} // end for

		if (pCard->m_pStack->Top() == pCard)             // Out of cards on the Stock stack?
			break;
	} // end while

	return TRUE;    // Game over, no other possible moves
}

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
