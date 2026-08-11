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
#include "bagel/hodjnpodj/barbershop/stack.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

CStack::CStack(CRect cRect, int nID) {
	m_pTopCard      = nullptr;
	m_pBottomCard   = nullptr;
	m_nID           = nID;
	m_nSize         = 0;
	m_cRect         = cRect;
}

CStack::~CStack() {
}

void CStack::Reset() {
	m_pTopCard      = nullptr;
	m_pBottomCard   = nullptr;
	m_nSize         = 0;
}

void CStack::Push(CCard *pCard) {
	if (pCard == nullptr)
		return;

	/*********************************************************
	* Update card pointers aligning them with current stack. *
	*********************************************************/
	pCard->m_pPrevCard  = m_pTopCard;   // link new top prev to old top card
	pCard->m_pNextCard  = nullptr;         // new top next card now does not point anywhere

	pCard->m_pStack     = this;         // reassign proper stack location

	/*************************
	* Update stack pointers. *
	*************************/
	if (m_pTopCard != nullptr)
		m_pTopCard->m_pNextCard = pCard;    // create forward chain for next card

	m_pTopCard = pCard;                     // link old top card to new top

	if (m_pBottomCard == nullptr)
		m_pBottomCard = pCard;

	m_nSize++;
}

CCard *CStack::Pop() {
	CCard *pReturnCard;

	/**************************************************
	* Undo any links of current card to other stacks. *
	**************************************************/
	pReturnCard = m_pTopCard;

	if (m_pTopCard == nullptr) {
		m_nSize = 0;
		return nullptr;
	}

	if (m_pTopCard->m_pPrevCard != nullptr) {
		m_pTopCard->m_pPrevCard->m_pNextCard = nullptr;
		m_pTopCard = m_pTopCard->m_pPrevCard;

		if (m_pTopCard->m_pNextCard != nullptr)
			m_pTopCard->m_pNextCard->m_pPrevCard = m_pTopCard->m_pPrevCard; // make cur prev card point to prev of prev card :^) (if any)

		m_nSize--;
	} else {
		m_pTopCard      = nullptr;
		m_pBottomCard   = nullptr;
		m_nSize         = 0;
	}

	return pReturnCard;
}

void CStack::Unlink(CCard *pCard) {
	/**************************************************
	* Undo any links of current card to other stacks. *
	**************************************************/
	if (pCard->m_pNextCard != nullptr) {
		pCard->m_pNextCard->m_pPrevCard = pCard->m_pPrevCard;   // make cur prev card point to prev of prev card :^) (if any)
		pCard->m_pNextCard = nullptr;
	}

	if (pCard->m_pPrevCard != nullptr) {
		pCard->m_pPrevCard->m_pNextCard = pCard->m_pNextCard;   // make cur of prev of next pointer point to next of the next (if any)
		this->m_pTopCard = pCard->m_pPrevCard;                  // Top of stack points to the prev card
		pCard->m_pPrevCard = nullptr;                              // clear the card "prev card" pointer
	} else {
		m_pTopCard      = nullptr;
		m_pBottomCard   = nullptr;
	}

	/**************************************************
	* Card's stack pointer should not point anywhere. *
	**************************************************/
	pCard->m_pStack = nullptr;                                     // clear card "stack" pointer
}

int CStack::Size() {
	return m_nSize;
}

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
