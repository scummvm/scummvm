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
#include "bagel/hodjnpodj/barbershop/undo.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

/*****************************************************************
 *
 * [function name]
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
CUndo::CUndo() {
	m_cUndoRect = CRect(
	                  UNDO_LEF,
	                  UNDO_TOP,
	                  UNDO_RIG,
	                  UNDO_BOT
	              );
	m_pStack    = nullptr;
	m_pCard     = nullptr;
	m_nStock    = NONE;
}

/*****************************************************************
 *
 * [function name]
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
CUndo::~CUndo() {
}

/*****************************************************************
 *
 * [function name]
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CUndo::Record(CStack *pStack, CCard *pCard) {
	m_pStack    = pStack;
	m_pCard     = pCard;
	m_nStock    = NONE;
}

/*****************************************************************
 *
 * [function name]
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
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
void CUndo::Record(int nCardsFlipped) {
	m_nStock    = nCardsFlipped;

	m_pStack    = nullptr;
	m_pCard     = nullptr;
}

/*****************************************************************
 *
 * Reset
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Resets undo parameters
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
void CUndo::Reset() {
	m_pStack    = nullptr;
	m_pCard     = nullptr;
	m_nStock    = NONE;
}

/*****************************************************************
 *
 * Undo
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Puts card back into original position.
 *      Returns true if undo was possible, else false.
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
bool CUndo::Undo(CDC *pDC, CBoard *pBoard, CPaint *pPaint) {
	CCard   *pCard;
	CPoint  cPos;
	int     i;

	if (m_nStock != NONE) {          // undo stock card flip?
		for (i = 0; i < m_nStock; i++) {
			//
			// update interal representation
			//
			pCard = pBoard->GetStack((loc) used)->Pop();
			pBoard->GetStack((loc) stock)->Push(pCard);

			//
			// update visual rep
			//
			pPaint->FlipCard(pDC, pCard);
			pPaint->Stack(pDC, pCard);
			pPaint->m_nUsedInd--;
		} // end for

		Reset();                    // clear undo
		return true;
	} // end if

	if (m_pStack == nullptr)            // basic card undo?
		return false;               // No - nothing can be undone


	m_pCard->m_pStack->Pop();       // undo move in internal rep
	m_pStack->Push(m_pCard);        // undo move in internal rep

	if (m_pStack->GetID() == used) {         // card in used stack?
		//
		// messy code to handle special visual used stacking order
		//
		if (m_pCard->m_pPrevCard == nullptr) {
			pPaint->m_nUsedInd = 0;
		} else {
			if (m_pCard->m_pPrevCard->m_cOrigin.x == m_pCard->m_pStack->m_cRect.TopLeft().x) {
				pPaint->m_nUsedInd = 1;
			} else if (m_pCard->m_pPrevCard->m_cOrigin.x == m_pCard->m_pStack->m_cRect.TopLeft().x + (1 * USED_DX)) {
				pPaint->m_nUsedInd = 2;
			} else {
				pPaint->m_nUsedInd = 0;
			} // end if
		} // end if

		pPaint->Stack(pDC, m_pCard);    // draw card apro stack
		pPaint->m_nUsedInd = 0;
	} else {
		pPaint->Stack(pDC, m_pCard);    // draw card apro stack
	} // end if

	Reset();                        // clear undo
	return true;
}

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
