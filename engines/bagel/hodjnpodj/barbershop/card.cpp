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
#include "bagel/hodjnpodj/barbershop/card.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

OSpr::OSpr() : CSprite() {
	m_cCard = nullptr;
}

CCard::CCard() {
	m_enSuit    = (suit) suit_none;
	m_nPip      = 0;
	m_pPrevCard = nullptr;
	m_pNextCard = nullptr;
	m_pSprite   = nullptr;
	m_pStack    = nullptr;
	m_bIsBack   = false;
	m_cOrigin   = CPoint(0, 0);
}

CCard::CCard(int nValue) {
	if (nValue >= SUITS * PIPS) {                    // Is this a joker card?
		m_enSuit    = manicurist;                   // Yes - assign joker suit
		m_nPip      = MANI_CARD;                    //  and joker pip
	} else {
		m_enSuit = (suit)(nValue % SUITS);          // No - assign usual suit
		m_nPip      = nValue % PIPS;
	}

	m_pPrevCard = nullptr;
	m_pNextCard = nullptr;
	m_pSprite   = nullptr;
	m_pStack    = nullptr;
	m_bIsBack   = false;
	m_cOrigin   = CPoint(0, 0);
}

CCard::CCard(suit enSuit, int nPip) {
	m_enSuit    = enSuit;
	m_nPip      = nPip;
	m_pPrevCard = nullptr;
	m_pNextCard = nullptr;
	m_pSprite   = nullptr;
	m_pStack    = nullptr;
	m_bIsBack   = false;
	m_cOrigin   = CPoint(0, 0);
}

CCard::~CCard() {
}

int CCard::GetValue() const {
	if (m_enSuit == manicurist) {    // Is Joker suit?
		return m_nPip;              // return it's special pip
	} else {
		return m_nPip + (m_enSuit * PIPS);
	}
}

CCard &CCard::operator=(const CCard& cCard) {
	m_enSuit    = cCard.m_enSuit;       // Private members
	m_nPip      = cCard.m_nPip;

	m_pPrevCard = cCard.m_pPrevCard;    // Public members
	m_pNextCard = cCard.m_pNextCard;
	m_pStack    = cCard.m_pStack;
	m_bIsBack   = cCard.m_bIsBack;
	m_pSprite   = cCard.m_pSprite;
	m_cOrigin   = cCard.m_cOrigin;

	return *this;
}

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
