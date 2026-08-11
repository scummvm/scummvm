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
#include "bagel/hodjnpodj/barbershop/deck.h"
#include "bagel/hodjnpodj/barbershop/board.h"
#include "bagel/hodjnpodj/barbershop/stack.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

CDeck::CDeck() {
	int i;

	for (i = 0; i < DECK; i++)
		m_cDeck[i] = CCard(i);          // Reset deck

	//srand((unsigned) time(nullptr));     // seed the random number generator
}

CDeck::~CDeck() {
}

void CDeck::Shuffle() {
	int     nNewDeck[DECK];
	int     nCard;
	int     i;

	for (i = 0; i < DECK; i++)
		nNewDeck[i] = NOT_USED;

	/******************************
	* Find an unused encrypt map. *
	******************************/
	for (i = 0; i < DECK; i++) {

		#ifndef REVEAL                          // change ifndef to ifdef for debugging purposes
		do {
			nCard = brand() % DECK;
		} while (nNewDeck[nCard] == USED);   // find unshuffled card

		nNewDeck[nCard] = USED;                 // mark card as shuffled

		#else
		nCard = i % 2;
		if (nCard == 1)
			nCard = CUST_CARD;
		#endif

		m_cDeck[i] = CCard(nCard);              // put card into it's new location
	}
}

void CDeck::Deal(CBoard *pBoard) {
	int     nStack, nCard, i;

	for (nStack = fnd; nStack <= used; nStack++)
		pBoard->GetStack((loc) nStack)->Reset();


	for (nStack = tab, i = 1, nCard = 0; nStack < stock; nStack++, i++)
		for (; nCard < (i * (TAB_STACK)); nCard++)
			pBoard->GetStack((loc) nStack)->Push(&m_cDeck[nCard]);

	for (; nCard < (STOCK + TABLEAU); nCard++)
		pBoard->GetStack(stock)->Push(&m_cDeck[nCard]);
}

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
